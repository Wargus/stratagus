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
/**@name master.cpp - The master server. */
//
//      (c) Copyright 2003-2007 by Tom Zickel and Jimmy Salmon
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

#ifndef __NET_CONNECTION_HANDLER_H__
#define __NET_CONNECTION_HANDLER_H__

#include "network/netsockets.h"

#include <memory>
#include <map>
#include <vector>

class IConnectionHandler
{
public:
	virtual ~IConnectionHandler() = default;
	virtual void Open(const CHost &host) = 0;
	virtual bool IsValid() = 0;
	virtual int HasDataToRead(int timeout) = 0;
	virtual int Recv(unsigned char *buf, int len, CHost *hostFrom) = 0;
	virtual void Close() = 0;
};

class IServerConnectionHandler : public IConnectionHandler
{
public:
	virtual ~IServerConnectionHandler() = default;
	virtual void SendToAllClients(std::vector<CHost> hosts, const unsigned char *buf, unsigned int len) = 0;
	virtual void SendToClient(CHost host, const unsigned char *buf, unsigned int len) = 0;
};

class IClientConnectionHandler : public IConnectionHandler
{
public:
	virtual ~IClientConnectionHandler() = default;
	virtual void SendToServer(const unsigned char *buf, unsigned int len) = 0;
};

class CTCPConnectionHandler
{
public:
	CTCPConnectionHandler() = default;
	CTCPConnectionHandler(CTCPSocket socket) : _socket(socket) {}
	bool Open(const CHost& host);
	int Listen();
	std::shared_ptr<CTCPConnectionHandler> Accept();
	void Close();
	bool Connect(const CHost& host);
	int Send(const unsigned char* buf, unsigned int len);
	int Recv(unsigned char* buf, int len);
	void SetBlocking();
	void SetNonBlocking();
	//
	int HasDataToRead(int timeout);
	bool IsValid() const;
	CHost GetHost() const;

private:
	CTCPSocket _socket;
};

class CUDPConnectionHandler
{
public:
	bool Open(const CHost& host);
	void Close();
	void Send(const CHost& host, const unsigned char* buf, unsigned int len);
	int Recv(unsigned char* buf, int len, CHost* hostFrom);
	void SetNonBlocking();
	//
	int HasDataToRead(int timeout);
	bool IsValid() const;

private:
	CUDPSocket _socket;
};

class CTCPServerConnectionHandler : public IServerConnectionHandler
{
public:
	virtual ~CTCPServerConnectionHandler() = default;
	void Open(const CHost& host) override;
	bool IsValid() override { return _connectionHandler.IsValid(); }
	int HasDataToRead(int timeout) override;
	void SendToAllClients(std::vector<CHost> hosts, const unsigned char* buf, unsigned int len) override;
	void SendToClient(CHost host, const unsigned char* buf, unsigned int len) override;
	int Recv(unsigned char* buf, int len, CHost* hostFrom) override;
	void Close() override;

private:
	int _skipClient = 0;
	CTCPConnectionHandler _connectionHandler;
	std::map<CHost, std::shared_ptr<CTCPConnectionHandler>> _clientConnections;
};

class CUDPServerConnectionHandler : public IServerConnectionHandler
{
public:
	virtual ~CUDPServerConnectionHandler() = default;
	void Open(const CHost& host) override;
	int HasDataToRead(int timeout) override { return _connectionHandler.HasDataToRead(timeout); }
	void SendToAllClients(std::vector<CHost> hosts, const unsigned char* buf, unsigned int len) override;
	void SendToClient(CHost host, const unsigned char* buf, unsigned int len) override;
	int Recv(unsigned char* buf, int len, CHost* hostFrom) override;
	bool IsValid() override { return _connectionHandler.IsValid(); }
	void Close() override { _connectionHandler.Close(); }

private:
	CUDPConnectionHandler _connectionHandler;
};

class CTCPClientConnectionHandler : public IClientConnectionHandler
{
public:
	CTCPClientConnectionHandler(CHost serverHost)
		: _serverHost(serverHost) {}

	virtual ~CTCPClientConnectionHandler() = default;
	void Open(const CHost &host) override;
	int HasDataToRead(int timeout) override { return _connectionHandler.HasDataToRead(timeout); }
	void SendToServer(const unsigned char *buf, unsigned int len) override;
	int Recv(unsigned char *buf, int len, CHost *hostFrom) override;
	bool IsValid() override { return _connectionHandler.IsValid(); }
	void Close() override { _connectionHandler.Close(); }

private:
	CHost _serverHost;
	CTCPConnectionHandler _connectionHandler;
};

class CUDPClientConnectionHandler : public IClientConnectionHandler
{
public:
	CUDPClientConnectionHandler(CHost serverHost)
		: _serverHost(serverHost) {}

	virtual ~CUDPClientConnectionHandler() = default;
	void Open(const CHost &host) override;
	int HasDataToRead(int timeout) override { return _connectionHandler.HasDataToRead(timeout); }
	void SendToServer(const unsigned char *buf, unsigned int len) override;
	int Recv(unsigned char *buf, int len, CHost *hostFrom) override;
	bool IsValid() override { return _connectionHandler.IsValid(); }
	void Close() override { _connectionHandler.Close(); }

private:
	CHost _serverHost;
	CUDPConnectionHandler _connectionHandler;
};

#endif // !__NET_CONNECTION_HANDLER_H__