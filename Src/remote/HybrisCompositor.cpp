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
#include <QFile>

#include <glib.h>

#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

#include <EGL/eglhybris.h>

#include "HybrisCompositor.h"
#include "IpcServer.h"
#include "HostWindow.h"
#include "HostWindowDataOpenGLHybris.h"

static const int kMaxConnections = 100;

struct compositor_ctrl_hdr {
    uint32_t windowId;
    uint32_t command;
};

HybrisCompositorRemoteClient::HybrisCompositorRemoteClient(HybrisCompositor *parent, int socketFd)
	: QObject(parent),
	  m_parent(parent),
	  m_socketFd(socketFd)
{
	m_socketNotifier = new QSocketNotifier(m_socketFd, QSocketNotifier::Read, this);
	connect(m_socketNotifier, SIGNAL(activated(int)), this, SLOT(onIncomingData()));
}

void HybrisCompositorRemoteClient::onIncomingData()
{
    int ret;
    struct compositor_ctrl_hdr hdr;

	qDebug() << __PRETTY_FUNCTION__ << "Incoming data from client ..";

    memset(&hdr, 0, sizeof(struct compositor_ctrl_hdr));

    ret = read(m_socketFd, &hdr, sizeof(struct compositor_ctrl_hdr));
    if (ret < 0) {
        qDebug() << __PRETTY_FUNCTION__ << "Client closed connection; removing ...";
        close(m_socketFd);
        Q_EMIT disconnected();
        return;
    }

    qDebug() << __PRETTY_FUNCTION__ << "Client send update for window id =" << hdr.windowId;

    OffscreenNativeWindowBuffer *buffer = new OffscreenNativeWindowBuffer();
    buffer->readFromFd(m_socketFd);

    hybris_register_buffer_handle(buffer->getHandle());

    Window *clientWindow = IpcServer::instance()->findWindow(hdr.windowId);
    if (!clientWindow) {
        qWarning() << __PRETTY_FUNCTION__ << "Did not found corressponding window for id" << hdr.windowId;
        return;
    }

    HostWindow *clientHostWindow = static_cast<HostWindow*>(clientWindow);
    if (!clientHostWindow) {
        qWarning() << __PRETTY_FUNCTION__ << "Window with id" << hdr.windowId << "is not a host window!";
        return;
    }

    const HostWindowDataOpenGLHybris *windowData =
            static_cast<const HostWindowDataOpenGLHybris*>(clientHostWindow->hostWindowData());
    if (!windowData) {
        qWarning() << __PRETTY_FUNCTION__ << "Window with id" << hdr.windowId << "is not a hybris based window!";
        return;
    }

    qDebug() << __PRETTY_FUNCTION__ << "Forwarding buffer to host window ...";
    windowData->postBuffer(buffer);
	qDebug() << __PRETTY_FUNCTION__ << "Requesting a repaint of the host window ...";
	clientHostWindow->onUpdateFullWindow();
}

HybrisCompositor* HybrisCompositor::instance()
{
	static HybrisCompositor* s_server = 0;

	if (G_UNLIKELY(s_server == 0))
		s_server = new HybrisCompositor;

	return s_server;
}

HybrisCompositor::HybrisCompositor()
	: m_socketPath("/tmp/sysmgr_compositor"),
	  m_channel(0),
	  m_socketWatch(-1)
{
	setup();
}

HybrisCompositor::~HybrisCompositor()
{
}

void HybrisCompositor::setup()
{
	if (QFile::exists(m_socketPath))
		QFile::remove(m_socketPath);

	g_message("%s: %d Initializing buffer server ...",
			  __PRETTY_FUNCTION__, __LINE__);

	m_socketFd = ::socket(PF_LOCAL, SOCK_STREAM, 0);
	if (m_socketFd < 0) {
		g_critical("%s: %d Failed to create socket: %s",
				   __PRETTY_FUNCTION__, __LINE__, strerror(errno));
		exit(-1);
	}

	struct sockaddr_un socketAddr;
	socketAddr.sun_family = AF_LOCAL;
	::strncpy(socketAddr.sun_path, m_socketPath.toAscii().data(),
			  G_N_ELEMENTS(socketAddr.sun_path));
	socketAddr.sun_path[G_N_ELEMENTS(socketAddr.sun_path)-1] = '\0';

	if (::bind(m_socketFd, (struct sockaddr*) &socketAddr, SUN_LEN(&socketAddr)) != 0) {
		qWarning() << __PRETTY_FUNCTION__ << "Failed to bind socket";
		close(m_socketFd);
		m_socketFd = -1;
		return;
	}

	if (::listen(m_socketFd, kMaxConnections) != 0) {
		qWarning() << __PRETTY_FUNCTION__ << "Failed to listen on socket";
		close(m_socketFd);
		m_socketFd = -1;
		return;
	}

	m_channel =  g_io_channel_unix_new(m_socketFd);
	m_socketWatch = g_io_add_watch_full(m_channel, G_PRIORITY_DEFAULT, G_IO_IN,
										onNewConnectionCb, this, NULL);

	g_message("%s: %d Buffer server successfully initialized",
			  __PRETTY_FUNCTION__, __LINE__);
}

gboolean HybrisCompositor::onNewConnectionCb(GIOChannel *channel, GIOCondition condition, gpointer user_data)
{
	HybrisCompositor *compositor = reinterpret_cast<HybrisCompositor*>(user_data);
	compositor->onNewConnection();
	return TRUE;
}

void HybrisCompositor::onClientDisconnected()
{
	HybrisCompositorRemoteClient *client = qobject_cast<HybrisCompositorRemoteClient*>(sender());
	client->deleteLater();
}

void HybrisCompositor::onNewConnection()
{
	struct sockaddr_un  socketAddr;
	socklen_t socketAddrLen;
	int clientSocketFd = -1;

	qDebug() << __PRETTY_FUNCTION__ << "New buffer sharing client connected";

	memset(&socketAddr, 0, sizeof(socketAddr));
	memset(&socketAddrLen, 0, sizeof(socketAddrLen));

	clientSocketFd = ::accept(m_socketFd, (struct sockaddr*) &socketAddr, &socketAddrLen);
	if (-1 == clientSocketFd) {
		g_critical("%s: %d Failed to accept inbound connection: %s",
				   __PRETTY_FUNCTION__, __LINE__, strerror(errno));
		return;
	}

	HybrisCompositorRemoteClient *client = new HybrisCompositorRemoteClient(this, clientSocketFd);
	connect(client, SIGNAL(disconnected()), this, SLOT(onClientDisconnected()));
}
