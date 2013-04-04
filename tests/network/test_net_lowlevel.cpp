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
/**@name test_net_lowlevel.cpp - The test file for net_lowlevel.cpp. */
//
//      (c) Copyright 2013 by Joris Dauphin
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

#include <UnitTest++.h>

#include "stratagus.h"
#include "net_lowlevel.h"
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>

class AutoNetwork
{
public:
	AutoNetwork() { NetInit(); }
	~AutoNetwork() { NetExit(); }
};

TEST_FIXTURE(AutoNetwork, NetResolveHost)
{
	const unsigned long localhost = htonl(0x7F000001); // 127.0.0.1

	CHECK_EQUAL(localhost, NetResolveHost("127.0.0.1"));
	CHECK_EQUAL(localhost, NetResolveHost("localhost"));
}

class Job
{
public:
	virtual ~Job() {}

	void Run() { pthread_create(&tid, NULL, Job::ThreadRun, this); }
	void Wait() { pthread_join(tid, NULL); }

private:
	virtual void DoJob() = 0;

	static void *ThreadRun(void *data)
	{
		Job *that = reinterpret_cast<Job*>(data);
		that->DoJob();
		return NULL;
	}

private:
	pthread_t tid;
};

static unsigned long GetMyIP()
{
	char buf[128];

	gethostname(buf, sizeof(buf));
	//DebugPrint("%s\n" _C_ buf);
	return NetResolveHost(buf);
}

template<typename T>
void TCPWrite(Socket socket, const T &obj)
{
	const char *buf = reinterpret_cast<const char *>(&obj);
	size_t s = 0;
	while (s != sizeof(T)) {
		s += NetSendTCP(socket, buf + s, sizeof(T) - s);
	}
}

template<typename T>
void TCPRead(Socket socket, T *obj)
{
	char *buf = reinterpret_cast<char *>(obj);
	size_t s = 0;
	while (s != sizeof(T)) {
		s += NetRecvTCP(socket, buf + s, sizeof(T) - s);
	}
}

class ServerTCP
{
public:
	explicit ServerTCP(int port) { socket = NetOpenTCP(NULL, port); }
	~ServerTCP() { NetCloseTCP(socket); }

	bool Listen() { return NetListenTCP(socket) != -1; }
	void Accept() { clientSocket = NetAcceptTCP(socket, &clientHost, &clientPort); }

	template <typename T>
	void Write(const T &obj) { TCPWrite(clientSocket, obj); }

	unsigned long GetClientHost() const { return clientHost; }
	int GetClientPort() const { return clientPort; }

private:
	Socket socket;
	Socket clientSocket;
	unsigned long clientHost;
	int clientPort;
};

class ClientTCP
{
public:
	explicit ClientTCP(int port) { socket = NetOpenTCP(NULL, port); }
	~ClientTCP() { NetCloseTCP(socket); }

	bool Connect(const char *host, int port) { return NetConnectTCP(socket, NetResolveHost(host), port) != -1; }

	template <typename T>
	void Read(T *obj) { TCPRead(socket, obj); }

private:
	Socket socket;
};

class Foo
{
public:
	Foo() { memset(&data, 0, sizeof(data)); }

	void Fill()
	{
		for (int i = 0; i != 42; ++i) {
			data[i] = i;
		}
	}

	bool Check() const
	{
		for (int i = 0; i != 42; ++i) {
			if (data[i] != i) {
				return false;
			}
		}
		return true;
	}
public:
	char data[42];
};

class ReceiverTCPJob : public Job
{
public:
	explicit ReceiverTCPJob(ClientTCP& client) : client(&client), check(false) {}

	bool Check() const { return check; }
private:

	virtual void DoJob()
	{
		Foo foo;

		client->Read(&foo);
		check = foo.Check();
	}
private:
	ClientTCP *client;
	bool check;
};

class SenderTCPJob : public Job
{
public:
	explicit SenderTCPJob(ServerTCP& server) : server(&server) {}

private:
	virtual void DoJob()
	{
		server->Listen();
		server->Accept();

		Foo foo;

		foo.Fill();
		server->Write(foo);
	}
private:
	ServerTCP *server;
};

TEST_FIXTURE(AutoNetwork, ExchangeTCP)
{
	const int serverPort = 6500;
	const int clientPort = 6501;

	ServerTCP server(serverPort);
	SenderTCPJob sender(server);
	sender.Run();

	ClientTCP client(clientPort);
	client.Connect("localhost", serverPort);
	ReceiverTCPJob receiver(client);
	receiver.Run();

	receiver.Wait();
	sender.Wait();
	CHECK(receiver.Check());
	CHECK_EQUAL(clientPort, htons(server.GetClientPort()));
	const unsigned long localhost = 0x7F000001; // 127.0.0.1

	CHECK(GetMyIP() == ntohl(server.GetClientHost())
		|| localhost == ntohl(server.GetClientHost()));
}

template<typename T>
void UDPWrite(Socket socket, const char *hostname, int port, const T &obj)
{
	const long host = NetResolveHost(hostname);
	const char *buf = reinterpret_cast<const char *>(&obj);
	port = htons(port);
	NetSendUDP(socket, host, port, buf, sizeof(T));
}

template<typename T>
void UDPRead(Socket socket, T *obj, unsigned long *hostFrom, int *portFrom)
{
	char *buf = reinterpret_cast<char *>(obj);
	size_t s = 0;
	while (s != sizeof(T)) {
		s += NetRecvUDP(socket, buf + s, sizeof(T) - s, hostFrom, portFrom);
	}
}

class ClientUDP
{
public:
	explicit ClientUDP(int port) { socket = NetOpenUDP(htonl(0x7F000001), htons(port)); }
	~ClientUDP() { NetCloseUDP(socket); }

	template <typename T>
	void Read(T *obj) { UDPRead(socket, obj, &hostFrom, &portFrom); }

	template <typename T>
	void Write(const char *hostname, long port, const T &obj) { UDPWrite(socket, hostname, port, obj); }

	unsigned long GetHostFrom() const { return hostFrom; }
	int GetPortFrom() const { return portFrom; }

private:
	Socket socket;
	unsigned long hostFrom;
	int portFrom;
};

class ReceiverUDPJob : public Job
{
public:
	explicit ReceiverUDPJob(ClientUDP& client) : client(&client), check(false) {}

	bool Check() const { return check; }
private:

	virtual void DoJob()
	{
		Foo foo;

		client->Read(&foo);
		check = foo.Check();
	}
private:
	ClientUDP *client;
	bool check;
};

class SenderUDPJob : public Job
{
public:
	explicit SenderUDPJob(ClientUDP& client, const char *hostname, int port) :
		client(&client),
		hostname(hostname),
		port(port)
	{}

private:
	virtual void DoJob()
	{
		Foo foo;

		foo.Fill();
		client->Write(hostname, port, foo);
	}
private:
	ClientUDP *client;
	const char *hostname;
	long port;
};

TEST_FIXTURE(AutoNetwork, ExchangeUDP)
{
	const int receiverPort = 6501;
	const int senderPort = 6500;

	ClientUDP client(receiverPort);
	ReceiverUDPJob receiver(client);
	receiver.Run();

	ClientUDP server(senderPort);
	SenderUDPJob sender(server, "localhost", receiverPort);
	sender.Run();

	receiver.Wait();
	sender.Wait();
	CHECK(receiver.Check());
	CHECK_EQUAL(senderPort, htons(client.GetPortFrom()));
	const unsigned long localhost = 0x7F000001; // 127.0.0.1

	CHECK(GetMyIP() == ntohl(client.GetHostFrom())
		|| localhost == ntohl(client.GetHostFrom()));
}
