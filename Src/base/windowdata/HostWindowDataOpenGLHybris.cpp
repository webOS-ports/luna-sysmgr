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
		: m_textureId(0),
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
	GLuint m_textureId;
	QPixmap *m_pixmap;
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
	  m_bufferSemaphore(0),
	  m_currentBufferTexture(0)
{
	qDebug() << __PRETTY_FUNCTION__ << "width =" << m_width << "height =" << m_height;

	if (metaDataKey > 0) {
		m_metaDataBuffer = PIpcBuffer::attach(metaDataKey);
		if (!m_metaDataBuffer) {
			g_critical("%s (%d): Failed to attach to metadata shared buffer with key: %d",
				__PRETTY_FUNCTION__, __LINE__, metaDataKey);
		}
	}

	if (eglCreateImageKHR == 0) {
		eglCreateImageKHR = (PFNEGLCREATEIMAGEKHRPROC) eglGetProcAddress("eglCreateImageKHR");
		eglDestroyImageKHR = (PFNEGLDESTROYIMAGEKHRPROC) eglGetProcAddress("eglDestroyImageKHR");
		glEGLImageTargetTexture2DOES = (PFNGLEGLIMAGETARGETTEXTURE2DOESPROC) eglGetProcAddress("glEGLImageTargetTexture2DOES");
	}

	m_bufferSemaphore = new QSystemSemaphore(QString("EGLWindow%1").arg(key));
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
	if (m_bufferQueue.size() == 0 && m_currentBufferTexture == 0)
		return &screenPixmap;

	if (NULL == m_currentBufferTexture && m_bufferQueue.size() > 0)
	{
		OffscreenNativeWindowBuffer *pNextBuffer = m_bufferQueue.dequeue();
		m_bufferSemaphore->release();
		if( pNextBuffer )
		{
			m_currentBufferTexture = new RemoteTextureBundle(pNextBuffer);
			pNextBuffer->decStrong(NULL);
		}
	}

	return m_currentBufferTexture ? m_currentBufferTexture->pixmap() : &screenPixmap;
}

void HostWindowDataOpenGLHybris::updateFromAppDirectRenderingLayer(int screenX, int screenY, int screenOrientation)
{
}

void HostWindowDataOpenGLHybris::onUpdateRegion(QPixmap& screenPixmap, int x, int y, int w, int h)
{
}

void HostWindowDataOpenGLHybris::releaseScreenPixmap()
{
	// If there is a next buffer in queue, discard the current texture cache so that the next
	// draw will pick that next buffer
	if( m_currentBufferTexture && m_bufferQueue.size() > 0 )
	{
		delete m_currentBufferTexture; m_currentBufferTexture = NULL;
	}
}

void HostWindowDataOpenGLHybris::postBuffer(OffscreenNativeWindowBuffer *buffer)
{
	m_bufferQueue.append(buffer);
	buffer->incStrong(NULL);
}

void HostWindowDataOpenGLHybris::cancelBuffer(OffscreenNativeWindowBuffer *buffer)
{
	// If we have buffers queued we have to care that we release all of them before to
	// enable the client to acquire the buffers in the correct order.
	if (m_bufferQueue.size() > 0) {
		m_bufferSemaphore->release(m_bufferQueue.size());
		m_bufferQueue.clear();
	}

	if( m_currentBufferTexture )
	{
		delete m_currentBufferTexture; m_currentBufferTexture = NULL;
	}

	m_bufferSemaphore->release();
}
