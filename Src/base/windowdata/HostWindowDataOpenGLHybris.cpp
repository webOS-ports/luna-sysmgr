/* @@@LICENSE
*
*      Copyright (c) 2010-2012 Hewlett-Packard Development Company, L.P.
*                (c) 2013 Simon Busch <morphis@gravedo.de>
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
* http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*
* LICENSE@@@ */

#include "Common.h"

#include "HostWindowDataOpenGLHybris.h"

#include <QDebug>
#include <QImage>
#include <QGLContext>
#include <QApplication>
#include <QPlatformNativeInterface>
#include <QCache>

#include <PIpcBuffer.h>

#define MESSAGES_INTERNAL_FILE "SysMgrMessagesInternal.h"
#include <PIpcMessageMacros.h>

#include "Logging.h"
#include "WindowMetaData.h"
#include "WebAppMgrProxy.h"
#include "HostBase.h"

#include <EGL/egl.h>
#include <EGL/eglext.h>

#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>

#ifndef GL_BGRA
#define GL_BGRA	0x80E1
#endif

PFNEGLCREATEIMAGEKHRPROC eglCreateImageKHR = 0;
PFNEGLDESTROYIMAGEKHRPROC eglDestroyImageKHR = 0;
PFNGLEGLIMAGETARGETTEXTURE2DOESPROC glEGLImageTargetTexture2DOES = 0;

class RemoteTextureBundle
{
public:
	RemoteTextureBundle(OffscreenNativeWindowBuffer *buffer)
		: m_index(buffer->index()),
		  m_textureId(0),
		  m_pixmap(0)
	{
		QGLContext* gc = (QGLContext*) QGLContext::currentContext();

		m_pixmap = new QPixmap(buffer->width, buffer->height);

		m_textureId = gc->bindTexture(*m_pixmap, GL_TEXTURE_2D, GL_BGRA,
				QGLContext::PremultipliedAlphaBindOption);

		EGLClientBuffer clientBuffer = (EGLClientBuffer) buffer;
		EGLint attrs[] = {
			EGL_IMAGE_PRESERVED_KHR, EGL_TRUE,
			EGL_NONE,
		};

		EGLDisplay currentDisplay = eglGetCurrentDisplay();
		m_image = eglCreateImageKHR(currentDisplay, EGL_NO_CONTEXT, EGL_NATIVE_BUFFER_ANDROID,
											  clientBuffer, attrs);
		if (m_image == EGL_NO_IMAGE_KHR) {
			EGLint error = eglGetError();
			qWarning() << __PRETTY_FUNCTION__ << "error creating EGLImage; error =" << error;
		}

		glEGLImageTargetTexture2DOES(GL_TEXTURE_2D, (GLeglImageOES) m_image);
	}

	~RemoteTextureBundle()
	{
		QGLContext* gc = (QGLContext*) QGLContext::currentContext();
		EGLDisplay currentDisplay = eglGetCurrentDisplay();

		if (m_image)
			eglDestroyImageKHR(currentDisplay, m_image);

		if (m_textureId)
			gc->deleteTexture(m_textureId);

		if (m_pixmap)
			delete m_pixmap;
	}

	QPixmap* pixmap() { return m_pixmap; }

private:
	int m_index;
	unsigned int m_textureId;
	QPixmap *m_pixmap;
	EGLImageKHR m_image;
};

class RemoteTextureCache
{
public:
	RemoteTextureCache(int size)
	{
		m_cache.setMaxCost(size);
	}

	QPixmap* retrievePixmapForBuffer(OffscreenNativeWindowBuffer *buffer)
	{
		if(m_cache.contains(buffer->index())) {
			QPixmap* bufferPixmap = m_cache[buffer->index()]->pixmap();
			if(bufferPixmap->width() == buffer->width &&
			   bufferPixmap->height() == buffer->height) {
				return bufferPixmap;
			}
			else {
				// sizes mismatch -> remove buffer from cache and create new cache item
				qDebug() << __PRETTY_FUNCTION__ << "Removing buffer from cache cause of size mismatch";
				m_cache.remove(buffer->index());
			}
		}

		qDebug() << __PRETTY_FUNCTION__ << "Buffer" << buffer->index()
				 << "is not yet cached; creating new cache item ...";
		RemoteTextureBundle* item = new RemoteTextureBundle(buffer);
		m_cache.insert(buffer->index(), item);

		return item->pixmap();
	}

private:
	QCache<unsigned int, RemoteTextureBundle> m_cache;
	int m_maxSize;
};


HostWindowDataOpenGLHybris::HostWindowDataOpenGLHybris(int key, int metaDataKey, int width,
													   int height, bool hasAlpha)
	: m_key(key),
	  m_metaDataKey(metaDataKey),
	  m_width(width),
	  m_height(height),
	  m_hasAlpha(hasAlpha),
	  m_updatedAllowed(true),
	  m_metaDataBuffer(0),
	  m_textureId(0),
	  m_bufferSemaphore(0),
	  m_currentBuffer(0)
{
	qDebug() << __PRETTY_FUNCTION__ << "width =" << m_width << "height =" << m_height;

	if (metaDataKey > 0) {
		m_metaDataBuffer = PIpcBuffer::attach(metaDataKey);
		if (!m_metaDataBuffer) {
			g_critical("%s (%d): Failed to attach to metadata shared buffer with key: %d",
				__PRETTY_FUNCTION__, __LINE__, metaDataKey);
		}
	}

	m_eglDisplay = eglGetDisplay(EGL_DEFAULT_DISPLAY);

	if (eglCreateImageKHR == 0) {
		eglCreateImageKHR = (PFNEGLCREATEIMAGEKHRPROC) eglGetProcAddress("eglCreateImageKHR");
		eglDestroyImageKHR = (PFNEGLDESTROYIMAGEKHRPROC) eglGetProcAddress("eglDestroyImageKHR");
		glEGLImageTargetTexture2DOES = (PFNGLEGLIMAGETARGETTEXTURE2DOESPROC) eglGetProcAddress("glEGLImageTargetTexture2DOES");
	}

	m_bufferSemaphore = new QSystemSemaphore(QString("EGLWindow%1").arg(key));

	m_cache = new RemoteTextureCache(OffscreenNativeWindow::bufferCount());
}

HostWindowDataOpenGLHybris::~HostWindowDataOpenGLHybris()
{
}

void HostWindowDataOpenGLHybris::flip()
{
	int width = m_width;
	m_width = m_height;
	m_height = width;
}

void HostWindowDataOpenGLHybris::initializePixmap(QPixmap &screenPixmap)
{
	screenPixmap = QPixmap();
}

QPixmap* HostWindowDataOpenGLHybris::acquirePixmap(QPixmap& screenPixmap)
{
	qDebug() << __PRETTY_FUNCTION__;

	if (m_bufferQueue.size() == 0 && m_currentBuffer == 0) {
		qDebug() << __PRETTY_FUNCTION__ << "We don't have a buffer in queue and no current buffer for rendering!";
		return &screenPixmap;
	}

	if (m_bufferQueue.size() > 0) {
		qDebug() << __PRETTY_FUNCTION__ << "Taking next buffer from queue for rendering";
		m_currentBuffer = m_bufferQueue.dequeue();
		m_bufferSemaphore->release();
	}

	qDebug() << __PRETTY_FUNCTION__ << "Getting buffer from cache (index =" << m_currentBuffer->index() << ") ...";
	QPixmap *pixmap = m_cache->retrievePixmapForBuffer(m_currentBuffer);

	return pixmap;
}

void HostWindowDataOpenGLHybris::updateFromAppDirectRenderingLayer(int screenX, int screenY, int screenOrientation)
{
}

void HostWindowDataOpenGLHybris::onUpdateRegion(QPixmap& screenPixmap, int x, int y, int w, int h)
{
}

void HostWindowDataOpenGLHybris::postBuffer(OffscreenNativeWindowBuffer *buffer)
{
	qDebug() << "Got buffer for rendering from client (key =" << m_key << ", index =" << buffer->index() << ") ...";
	m_bufferQueue.append(buffer);
}

void HostWindowDataOpenGLHybris::cancelBuffer(OffscreenNativeWindowBuffer *buffer)
{
	// If we have buffers queued we have to care that we release all of them before to
	// enable the client to acquire the buffers in the correct order.
	if (m_bufferQueue.size() > 0) {
		m_bufferSemaphore->release(m_bufferQueue.size());
		m_bufferQueue.clear();
		m_currentBuffer = 0;
	}

	qDebug() << "Canceling buffer index =" << buffer->index();
	m_bufferSemaphore->release();
}
