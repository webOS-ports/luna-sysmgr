/**
 *  Copyright (c) 2013 Simon Busch <morphis@gravedo.de>
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */


#ifndef HYBRISCOMPOSITORREMOTECLIENTLUNA_H
#define HYBRISCOMPOSITORREMOTECLIENTLUNA_H

#include <WebosSurfaceManagerRemoteClient.h>

class HostWindow;
class HostWindowDataOpenGLHybris;

class WebosSurfaceManagerRemoteClientLuna : public WebosSurfaceManagerRemoteClient
{
public:
	WebosSurfaceManagerRemoteClientLuna(WebosSurfaceManager *parent, int socketFd);

protected:
	virtual void handleIdentify(unsigned int windowId);
	virtual void handleIncomingBuffer(OffscreenNativeWindowBuffer *buffer);

private:
	HostWindow *m_clientHostWindow;
	HostWindowDataOpenGLHybris *m_windowData;
};

class WebosSurfaceManagerRemoteClientLunaFactory : public WebosSurfaceManagerRemoteClientFactory
{
public:
	virtual WebosSurfaceManagerRemoteClient *create(WebosSurfaceManager *parent, int socketFd)
	{
		return new WebosSurfaceManagerRemoteClientLuna(parent, socketFd);
	}
};

#endif
