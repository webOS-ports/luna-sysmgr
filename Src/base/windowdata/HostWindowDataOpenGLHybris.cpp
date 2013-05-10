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
		EGLImageKHR eglImage = eglCreateImageKHR(currentDisplay, EGL_NO_CONTEXT, EGL_NATIVE_BUFFER_ANDROID,
											  clientBuffer, attrs);
		if (eglImage == EGL_NO_IMAGE_KHR) {
			EGLint error = eglGetError();
			qWarning() << __PRETTY_FUNCTION__ << "error creating EGLImage; error =" << error;
		}

		glEGLImageTargetTexture2DOES(GL_TEXTURE_2D, (GLeglImageOES) eglImage);

		eglDestroyImageKHR(currentDisplay, eglImage); eglImage = 0;
	}

	~RemoteTextureBundle()
	{
		if (m_textureId)
		{
			QGLContext* gc = (QGLContext*) QGLContext::currentContext();
			gc->deleteTexture(m_textureId);
		}

		if (m_pixmap)
			delete m_pixmap;
	}

	QPixmap* pixmap() { return m_pixmap; }

private:
	int m_index;
	GLuint m_textureId;
	QPixmap *m_pixmap;
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
				m_cache.remove(buffer->index());
			}
		}

		RemoteTextureBundle* item = new RemoteTextureBundle(buffer);
		m_cache.insert(buffer->index(), item);

		return item->pixmap();
	}

	void releaseBufferFromCache(OffscreenNativeWindowBuffer *buffer)
	{
		if(m_cache.contains(buffer->index())) {
			 // there is a match --> remove this item from the cache
			 m_cache.remove(buffer->index());
		 }
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

	m_cache = new RemoteTextureCache(5);
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
	if (m_bufferQueue.size() == 0 && m_currentBuffer == 0)
		return &screenPixmap;

	if (m_bufferQueue.size() > 0) {
		m_currentBuffer = m_bufferQueue.dequeue();
		m_bufferSemaphore->release();
	}

	return m_cache->retrievePixmapForBuffer(m_currentBuffer);
}

void HostWindowDataOpenGLHybris::updateFromAppDirectRenderingLayer(int screenX, int screenY, int screenOrientation)
{
}

void HostWindowDataOpenGLHybris::onUpdateRegion(QPixmap& screenPixmap, int x, int y, int w, int h)
{
}

void HostWindowDataOpenGLHybris::releaseScreenPixmap()
{
	if( m_currentBuffer )
	{
		m_cache->releaseBufferFromCache(m_currentBuffer);
	}
}

void HostWindowDataOpenGLHybris::postBuffer(OffscreenNativeWindowBuffer *buffer)
{
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

	m_bufferSemaphore->release();
}
