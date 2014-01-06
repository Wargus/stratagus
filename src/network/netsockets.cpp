//       _________ __                 __
//      /   _____//  |_____________ _/  |______     ____  __ __  ______
//      \_____  \\   __\_  __ \__  \\   __\__  \   / ___\|  |  \/  ___/
//      /        \|  |  |  | \// __ \|  |  / __ \_/ /_/  >  |  /\___ |
//     /_______  /|__|  |__|  (____  /__| (____  /\___  /|____//____  >
//             \/                  \/          \//_____/            \/
//  ______________________                           ______________________
//                        T H E   W A R   B E G I N S
//         Stratagus - A free fantasy real time strategy game engine
//
/**@name netsockets.cpp - TCP and UDP sockets. */
//
//      (c) Copyright 2013 by Joris Dauphin and cybermind
//
//      This program is free software; you can redistribute it and/or modify
//      it under the terms of the GNU General Public License as published by
//      the Free Software Foundation; only version 2 of the License.
//
//      This program is distributed in the hope that it will be useful,
//      but WITHOUT ANY WARRANTY; without even the implied warranty of
//      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//      GNU General Public License for more details.
//
//      You should have received a copy of the GNU General Public License
//      along with this program; if not, write to the Free Software
//      Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
//      02111-1307, USA.
//

//@{

//----------------------------------------------------------------------------
//  Includes
//----------------------------------------------------------------------------

#include "stratagus.h"

#include "network/netsockets.h"
#include "net_lowlevel.h"

#include <stdio.h>

//
// CHost
//

CHost::CHost(const char *name, int port)
{
	this->ip = name ? NetResolveHost(name) : INADDR_ANY;
	this->port = htons(port);
}

std::string CHost::toString() const
{
	char buf[24]; // 127.255.255.255:65555
	sprintf(buf, "%d.%d.%d.%d:%d", NIPQUAD(ntohl(ip)), ntohs(port));
	return buf;
}

bool CHost::isValid() const
{
	return ip != 0 && port != 0;
}

//
// CUDPSocket_Impl
//

class CUDPSocket_Impl
{
public:
	CUDPSocket_Impl() : socket(Socket(-1)) {}
	~CUDPSocket_Impl() { if (IsValid()) { Close(); } }
	bool Open(const CHost &host) { socket = NetOpenUDP(host.getIp(), host.getPort()); return socket != INVALID_SOCKET; }
	void Close() { NetCloseUDP(socket); socket = Socket(-1); }
	void Send(const CHost &host, const void *buf, unsigned int len) { NetSendUDP(socket, host.getIp(), host.getPort(), buf, len); }
	int Recv(void *buf, int len, CHost *hostFrom)
	{
		unsigned long ip;
		int port;
		int res = NetRecvUDP(socket, buf, len, &ip, &port);
		*hostFrom = CHost(ip, port);
		return res;
	}
	void SetNonBlocking() { NetSetNonBlocking(socket); }
	int HasDataToRead(int timeout) { return NetSocketReady(socket, timeout); }
	bool IsValid() const { return socket != Socket(-1); }
private:
	Socket socket;
};

//
// CUDPSocket
//

#ifdef DEBUG

CUDPSocket::CStatistic::CStatistic()
{
	clear();
}

void CUDPSocket::CStatistic::clear()
{
	receivedBytesCount = 0;
	receivedBytesExpectedCount = 0;
	receivedErrorCount = 0;
	receivedPacketsCount = 0;
	sentBytesCount = 0;
	sentPacketsCount = 0;
	biggestReceivedPacketSize = 0;
	biggestSentPacketSize = 0;
}

#endif

CUDPSocket::CUDPSocket()
{
	m_impl = new CUDPSocket_Impl();
}

CUDPSocket::~CUDPSocket()
{
	delete m_impl;
}

bool CUDPSocket::Open(const CHost &host)
{
	return m_impl->Open(host);
}

void CUDPSocket::Close()
{
	m_impl->Close();
}

void CUDPSocket::Send(const CHost &host, const void *buf, unsigned int len)
{
#ifdef DEBUG
	++m_statistic.sentPacketsCount;
	m_statistic.sentBytesCount += len;
	m_statistic.biggestSentPacketSize = std::max(m_statistic.biggestSentPacketSize, len);
#endif
	m_impl->Send(host, buf, len);
}

int CUDPSocket::Recv(void *buf, int len, CHost *hostFrom)
{
	const int res = m_impl->Recv(buf, len, hostFrom);
#ifdef DEBUG
	m_statistic.receivedBytesExpectedCount += len;
	if (res == -1) {
		++m_statistic.receivedErrorCount;
	} else {
		++m_statistic.receivedPacketsCount;
		m_statistic.receivedBytesCount += res;
		m_statistic.biggestReceivedPacketSize = std::max(m_statistic.biggestReceivedPacketSize, (unsigned int)res);
	}
#endif
	return res;
}

void CUDPSocket::SetNonBlocking()
{
	m_impl->SetNonBlocking();
}

int CUDPSocket::HasDataToRead(int timeout)
{
	return m_impl->HasDataToRead(timeout);
}

bool CUDPSocket::IsValid() const
{
	return m_impl->IsValid();
}

//
// CTCPSocket_Impl
//

class CTCPSocket_Impl
{
public:
	CTCPSocket_Impl() : socket(Socket(-1)) {}
	~CTCPSocket_Impl() { if (IsValid()) { Close(); } }
	bool Open(const CHost &host);
	void Close() { NetCloseTCP(socket); socket = Socket(-1); }
	bool Connect(const CHost &host) { return NetConnectTCP(socket, host.getIp(), host.getPort()) != -1; }
	int Send(const void *buf, unsigned int len) { return NetSendTCP(socket, buf, len); }
	int Recv(void *buf, int len)
	{
		int res = NetRecvTCP(socket, buf, len);
		return res;
	}
	void SetNonBlocking() { NetSetNonBlocking(socket); }
	int HasDataToRead(int timeout) { return NetSocketReady(socket, timeout); }
	bool IsValid() const { return socket != Socket(-1); }
private:
	Socket socket;
};

bool CTCPSocket_Impl::Open(const CHost &host)
{
	char ip[24]; // 127.255.255.255:65555
	memset(&ip, 0, sizeof(ip));
	sprintf(ip, "%d.%d.%d.%d", NIPQUAD(ntohl(host.getIp())));
	this->socket = NetOpenTCP(ip, host.getPort());
	return this->socket != INVALID_SOCKET;
}

//
// CTCPSocket
//

CTCPSocket::CTCPSocket()
{
	m_impl = new CTCPSocket_Impl();
}

CTCPSocket::~CTCPSocket()
{
	delete m_impl;
}

bool CTCPSocket::Open(const CHost &host)
{
	return m_impl->Open(host);
}

void CTCPSocket::Close()
{
	m_impl->Close();
}


bool CTCPSocket::Connect(const CHost &host)
{
	return m_impl->Connect(host);
}

int CTCPSocket::Send(const void *buf, unsigned int len)
{
	return m_impl->Send(buf, len);
}

int CTCPSocket::Recv(void *buf, int len)
{
	const int res = m_impl->Recv(buf, len);
	return res;
}

void CTCPSocket::SetNonBlocking()
{
	m_impl->SetNonBlocking();
}

int CTCPSocket::HasDataToRead(int timeout)
{
	return m_impl->HasDataToRead(timeout);
}

bool CTCPSocket::IsValid() const
{
	return m_impl->IsValid();
}

//@}