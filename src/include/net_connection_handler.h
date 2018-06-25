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
	virtual void SendToAllClients(std::vector<CHost> hosts, const unsigned char *buf, unsigned int len) = 0;
	virtual void SendToClient(CHost host, const unsigned char *buf, unsigned int len) = 0;
};

class IClientConnectionHandler : public IConnectionHandler
{
public:
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