/* @@@LICENSE
*
* Copyright (c) 2013 Simon Busch <morphis@gravedo.de>
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

#include <QDebug>

#include "WebosSurfaceManagerRemoteClientLuna.h"

#include "IpcServer.h"
#include "HostWindow.h"
#include "HostWindowDataOpenGLHybris.h"

#include "SystemUiController.h"

WebosSurfaceManagerRemoteClientLuna::WebosSurfaceManagerRemoteClientLuna(WebosSurfaceManager *parent, int socketFd)
	: WebosSurfaceManagerRemoteClient(parent, socketFd)
{
}

void WebosSurfaceManagerRemoteClientLuna::handleIncomingBuffer(int windowId, OffscreenNativeWindowBuffer *buffer)
{
	Window *clientWindow = IpcServer::instance()->findWindow(windowId);
	if (!clientWindow) {
		qWarning() << __PRETTY_FUNCTION__ << "Did not found corressponding window for id" << windowId;
		return;
	}

	HostWindow *clientHostWindow = static_cast<HostWindow*>(clientWindow);
	if (!clientHostWindow) {
		qWarning() << __PRETTY_FUNCTION__ << "Window with id" << windowId << "is not a host window!";
		return;
	}

	const HostWindowDataOpenGLHybris *windowData =
		static_cast<const HostWindowDataOpenGLHybris*>(clientHostWindow->hostWindowData());
	if (!windowData) {
		qWarning() << __PRETTY_FUNCTION__ << "Window with id" << windowId << "is not a hybris based window!";
		return;
	}

	if (!SystemUiController::instance()->bootFinished() ||
		!clientHostWindow->isVisible() ||
		clientHostWindow->parentItem() == 0 ||
		!clientHostWindow->parentItem()->isVisible()) {
		windowData->cancelBuffer(buffer);
		return;
	}

	windowData->postBuffer(buffer);
	clientHostWindow->onUpdateFullWindow();
}
