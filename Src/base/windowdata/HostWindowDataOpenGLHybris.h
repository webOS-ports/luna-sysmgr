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

#ifndef HOSTWINDOWDATAOPENGLHYBRIS_H
#define HOSTWINDOWDATAOPENGLHYBRIS_H

#include "Common.h"

#include "HostWindowData.h"

#include <QSystemSemaphore>
#include <QPixmap>
#include <QQueue>
#include <PIpcBuffer.h>
#include <EGL/eglhybris.h>
#include <EGL/eglext.h>
#include <OffscreenNativeWindow.h>

class RemoteTextureCache;

class HostWindowDataOpenGLHybris : public HostWindowData
{
public:

	HostWindowDataOpenGLHybris(int key, int metaDataKey, int width, int height, bool hasAlpha);
	virtual ~HostWindowDataOpenGLHybris();

	virtual bool isValid() const { return true; }
	virtual int key() const { return m_key; }
	virtual int width() const { return m_width; }
	virtual int height() const { return m_height; }
	virtual bool hasAlpha() const { return m_hasAlpha; }
	virtual void flip();
	virtual PIpcBuffer* metaDataBuffer() const { return m_metaDataBuffer; }
	virtual void initializePixmap(QPixmap& screenPixmap);
	virtual QPixmap* acquirePixmap(QPixmap& screenPixmap);
	virtual void allowUpdates(bool allow) {}
	virtual void onUpdateRegion(QPixmap& screenPixmap, int x, int y, int w, int h);
	virtual void updateFromAppDirectRenderingLayer(int screenX, int screenY, int screenOrientation);
	virtual void onUpdateWindowRequest() { }
	virtual void onAboutToSendSyncMessage() {}
	virtual void postBuffer(OffscreenNativeWindowBuffer *buffer);
	virtual void cancelBuffer(OffscreenNativeWindowBuffer *buffer);

protected:
	int m_key;
	int m_metaDataKey;
	int m_width;
	int m_height;
	bool m_hasAlpha;
	bool m_updatedAllowed;
	PIpcBuffer *m_metaDataBuffer;
	QQueue<OffscreenNativeWindowBuffer*> m_bufferQueue;
	unsigned int m_textureId;
	EGLImageKHR m_image;
	EGLDisplay m_eglDisplay;
	QPixmap m_pixmap;
	QSystemSemaphore *m_bufferSemaphore;
	RemoteTextureCache *m_cache;
	OffscreenNativeWindowBuffer *m_currentBuffer;

private:
	HostWindowDataOpenGLHybris(const HostWindowDataOpenGLHybris&);
	HostWindowDataOpenGLHybris& operator=(const HostWindowDataOpenGLHybris&);
};

#endif /* HOSTWINDOWDATAOPENGLHYBRIS_H */
