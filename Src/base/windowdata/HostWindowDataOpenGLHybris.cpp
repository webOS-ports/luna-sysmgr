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

HostWindowDataOpenGLHybris::HostWindowDataOpenGLHybris(int key, int metaDataKey, int width,
													   int height, bool hasAlpha)
	: m_key(key),
	  m_metaDataKey(metaDataKey),
	  m_width(width),
	  m_height(height),
	  m_hasAlpha(hasAlpha),
	  m_updatedAllowed(true),
	  m_metaDataBuffer(0),
	  m_textureId(0)
{
	qDebug() << __PRETTY_FUNCTION__ << "width =" << m_width << "height =" << m_height;

	m_metaDataBuffer = PIpcBuffer::attach(metaDataKey);
	if (!m_metaDataBuffer) {
		g_critical("%s (%d): Failed to attach to metadata shared buffer with key: %d",
				   __PRETTY_FUNCTION__, __LINE__, metaDataKey);
	}

	QPlatformNativeInterface *nativeInterface = QApplication::platformNativeInterface();
	if (nativeInterface) {
		m_eglDisplay = nativeInterface->nativeResourceForWidget("display", 0);
	}

	qDebug() << __PRETTY_FUNCTION__ << "m_eglDisplay = " << m_eglDisplay;

	if (eglCreateImageKHR == 0) {
		eglCreateImageKHR = (PFNEGLCREATEIMAGEKHRPROC) eglGetProcAddress("eglCreateImageKHR");
		eglDestroyImageKHR = (PFNEGLDESTROYIMAGEKHRPROC) eglGetProcAddress("eglDestroyImageKHR");
		glEGLImageTargetTexture2DOES = (PFNGLEGLIMAGETARGETTEXTURE2DOESPROC) eglGetProcAddress("glEGLImageTargetTexture2DOES");
	}
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
	qDebug() << __PRETTY_FUNCTION__ << "Initializing screen pixmap width ="
			 << m_width << "height =" << m_height;

	screenPixmap = QPixmap(m_width, m_height);
	screenPixmap.fill(QColor(255, 0, 0, 255));

	m_pixmap = QPixmap(m_width, m_height);
	m_pixmap.fill(QColor(0, 255, 0, 255));
}

QPixmap* HostWindowDataOpenGLHybris::acquirePixmap(QPixmap& screenPixmap)
{
	OffscreenNativeWindowBuffer *buffer = 0;

	qDebug() << __PRETTY_FUNCTION__;

	if (m_bufferQueue.size() > 0) {
		qDebug() << __PRETTY_FUNCTION__ << "Rendering remote buffer";
		buffer = m_bufferQueue.dequeue();

		QGLContext* gc = (QGLContext*) QGLContext::currentContext();
		if (gc) {
			if (m_image)
				eglDestroyImageKHR(m_eglDisplay, m_image);

			if (m_textureId)
				gc->deleteTexture(m_textureId);

			m_textureId = gc->bindTexture(m_pixmap, GL_TEXTURE_2D, GL_BGRA,
										  QGLContext::PremultipliedAlphaBindOption);

			EGLClientBuffer clientBuffer = (EGLClientBuffer) buffer;
			EGLint attrs[] = {
				EGL_IMAGE_PRESERVED_KHR,    EGL_TRUE,
				EGL_NONE,
			};

			m_image = eglCreateImageKHR(m_eglDisplay, EGL_NO_CONTEXT, EGL_NATIVE_BUFFER_ANDROID,
												  clientBuffer, attrs);
			if (m_image == EGL_NO_IMAGE_KHR) {
				EGLint error = eglGetError();
				qWarning() << __PRETTY_FUNCTION__ << "error creating EGLImage";
			}

			glEGLImageTargetTexture2DOES(GL_TEXTURE_2D, (GLeglImageOES) m_image);
		}
	}

	return &m_pixmap;
}

void HostWindowDataOpenGLHybris::updateFromAppDirectRenderingLayer(int screenX, int screenY, int screenOrientation)
{
	qDebug() << __PRETTY_FUNCTION__;
}

void HostWindowDataOpenGLHybris::onUpdateRegion(QPixmap& screenPixmap, int x, int y, int w, int h)
{
}

void HostWindowDataOpenGLHybris::postBuffer(OffscreenNativeWindowBuffer *buffer)
{
	qDebug() << __PRETTY_FUNCTION__ << "Received buffer for rendering";

	m_bufferQueue.append(buffer);
}
