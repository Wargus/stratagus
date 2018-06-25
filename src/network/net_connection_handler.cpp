#include "net_connection_handler.h"

using namespace std;

bool CTCPConnectionHandler::Open(const CHost& host) {
	return _socket.Open(host);
}

int CTCPConnectionHandler::Listen() {
	return _socket.Listen();
}

shared_ptr<CTCPConnectionHandler> CTCPConnectionHandler::Accept() {
	CTCPSocket* newSocket = _socket.Accept();

	if(newSocket == nullptr) {
		return nullptr;
	}

	auto newConnectionHandler = make_shared<CTCPConnectionHandler>(*newSocket);
	delete newSocket;

	return newConnectionHandler;
}

void CTCPConnectionHandler::Close() {
	_socket.Close();
}

bool CTCPConnectionHandler::Connect(const CHost& host) {
	return _socket.Connect(host);
}

int CTCPConnectionHandler::Send(const unsigned char* buf, unsigned len) {
	const auto bufLength = new unsigned char[2];

	bufLength[0] = len >> 8 & 0xFF;
	bufLength[1] = len & 0xFF;

	_socket.Send(bufLength, 2);
	return _socket.Send(buf, len);
}

int CTCPConnectionHandler::Recv(unsigned char* buf, int len) {
	auto* bufLength = new unsigned char[2];

	const int resLen = _socket.Recv(bufLength, 2);
	if (resLen < 2)
	{
		return resLen;
	}

	int actualLen = bufLength[0];
	actualLen <<= 8;
	actualLen |= bufLength[1];

	return _socket.Recv(buf, actualLen);
}

void CTCPConnectionHandler::SetBlocking() {
	_socket.SetBlocking();
}

void CTCPConnectionHandler::SetNonBlocking() {
	_socket.SetNonBlocking();
}

int CTCPConnectionHandler::HasDataToRead(int timeout) {
	return _socket.HasDataToRead(timeout);
}

bool CTCPConnectionHandler::IsValid() const {
	return _socket.IsValid();
}

CHost CTCPConnectionHandler::GetHost() const {
	return _socket.GetHost();
}

bool CUDPConnectionHandler::Open(const CHost& host) {
	return _socket.Open(host);
}

void CUDPConnectionHandler::Close() {
	_socket.Close();
}

void CUDPConnectionHandler::Send(const CHost& host, const unsigned char* buf, unsigned len) {
	_socket.Send(host, buf, len);
}

int CUDPConnectionHandler::Recv(unsigned char* buf, int len, CHost* hostFrom) {
	return _socket.Recv(buf, len, hostFrom);
}

void CUDPConnectionHandler::SetNonBlocking() {
	_socket.SetNonBlocking();
}

int CUDPConnectionHandler::HasDataToRead(int timeout) {
	return _socket.HasDataToRead(timeout);
}

bool CUDPConnectionHandler::IsValid() const {
	return _socket.IsValid();
}

void CTCPServerConnectionHandler::Open(const CHost& host) {
	_connectionHandler.Open(host);
	_connectionHandler.SetNonBlocking();
	_connectionHandler.Listen();
}

int CTCPServerConnectionHandler::HasDataToRead(int timeout) {
	const auto newConnectionHandler = _connectionHandler.Accept();
	if (newConnectionHandler)
	{
		newConnectionHandler->SetBlocking();
		// TODO make sure new connection does not override legitimate host
		_clientConnections[newConnectionHandler->GetHost()] = newConnectionHandler;
	}

	for (auto& it : _clientConnections) {
		const int read = it.second->HasDataToRead(timeout);
		if (read > 0)
		{
			return read;
		}
	}

	return 0;
}

void CTCPServerConnectionHandler::SendToAllClients(vector<CHost> hosts, const unsigned char* buf, unsigned len) {
	for (auto& host : hosts) {
		_clientConnections[host]->Send(buf, len);
	}
}

void CTCPServerConnectionHandler::SendToClient(CHost host, const unsigned char* buf, unsigned len) {
	_clientConnections[host]->Send(buf, len);
}

int CTCPServerConnectionHandler::Recv(unsigned char* buf, int len, CHost* hostFrom) {
	// round robin across _clientSockets to avoid chatty client flooding client/server comm

	int skip = _skipClient;
	int take = _clientConnections.size();

	_skipClient = (_skipClient + 1) % _clientConnections.size();

	for (auto& it : _clientConnections) {
		if (skip-- > 0) continue;
		take--;

		const int read = it.second->HasDataToRead(0);
		if (read > 0)
		{
			*hostFrom = it.first;
			return it.second->Recv(buf, len);
		}
	}

	for (auto& it : _clientConnections) {
		if (take-- == 0) break;

		const int read = it.second->HasDataToRead(0);
		if (read > 0)
		{
			*hostFrom = it.first;
			return it.second->Recv(buf, len);
		}
	}

	return 0;
}

void CTCPServerConnectionHandler::Close() {
	for (auto& it : _clientConnections) {
		it.second->Close();
	}

	_clientConnections.clear();
	_connectionHandler.Close();
}

void CUDPServerConnectionHandler::Open(const CHost& host) {
	_connectionHandler.Open(host);
}

void CUDPServerConnectionHandler::SendToAllClients(std::vector<CHost> hosts, const unsigned char* buf, unsigned len) {
	for (auto& host : hosts) {
		_connectionHandler.Send(host, buf, len);
	}
}

void CUDPServerConnectionHandler::SendToClient(CHost host, const unsigned char* buf, unsigned len) {
	_connectionHandler.Send(host, buf, len);
}

int CUDPServerConnectionHandler::Recv(unsigned char* buf, int len, CHost* hostFrom) {
	return _connectionHandler.Recv(buf, len, hostFrom);
}

void CTCPClientConnectionHandler::Open(const CHost& host) {
	_connectionHandler.Open(host);
	_connectionHandler.SetBlocking();
	_connectionHandler.Connect(_serverHost);
}

void CTCPClientConnectionHandler::SendToServer(const unsigned char* buf, unsigned len) {
	_connectionHandler.Send(buf, len);
}

int CTCPClientConnectionHandler::Recv(unsigned char* buf, int len, CHost* hostFrom) {
	*hostFrom = _serverHost;
	return _connectionHandler.Recv(buf, len);
}

void CUDPClientConnectionHandler::Open(const CHost& host) {
	_connectionHandler.Open(CHost("localhost", 0));
}

void CUDPClientConnectionHandler::SendToServer(const unsigned char* buf, unsigned len) {
	_connectionHandler.Send(_serverHost, buf, len);
}

int CUDPClientConnectionHandler::Recv(unsigned char* buf, int len, CHost* hostFrom) {
	return _connectionHandler.Recv(buf, len, hostFrom);
}
