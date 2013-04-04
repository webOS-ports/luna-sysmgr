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

#include <HybrisCompositorRemoteClient.h>

class HybrisCompositorRemoteClientLuna : public HybrisCompositorRemoteClient
{
	Q_OBJECT
public:
	HybrisCompositorRemoteClientLuna(HybrisCompositor *parent, int socketFd);

protected:
	virtual void handleIncomingBuffer(int windowId, OffscreenNativeWindowBuffer *buffer);
};

class HybrisCompositorRemoteClientLunaFactory : public HybrisCompositorRemoteClientFactory
{
public:
	virtual HybrisCompositorRemoteClient *create(HybrisCompositor *parent, int socketFd)
	{
		return new HybrisCompositorRemoteClientLuna(parent, socketFd);
	}
};

#endif
