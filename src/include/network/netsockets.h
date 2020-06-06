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
/**@name netsockets.h - TCP and UDP sockets. */
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

#ifndef NETSOCKETS_H
#define NETSOCKETS_H

#include <string>

//@{

class CHost
{
public:
	CHost() : ip(0), port(0) {}
	CHost(const char *name, int port);
	CHost(unsigned long ip, int port) : ip(ip), port(port) {}
	unsigned long getIp() const { return ip; }
	int getPort() const { return port; }
	std::string toString() const;
	bool isValid() const;

	bool operator == (const CHost &rhs) const { return ip == rhs.ip && port == rhs.port; }
	bool operator != (const CHost &rhs) const { return !(*this == rhs); }
	bool operator < (const CHost &rhs) const { return ip < rhs.ip || ip == rhs.ip && port < rhs.port; }
private:
	unsigned long ip;
	int port;
};

class CUDPSocket_Impl;
class CTCPSocket_Impl;

class CUDPSocket
{
public:
	CUDPSocket();
	~CUDPSocket();
	bool Open(const CHost &host);
	void Close();
	void Send(const CHost &host, const unsigned char *buf, unsigned int len);
	int Recv(unsigned char *buf, int len, CHost *hostFrom);
	void SetNonBlocking();
	//
	int HasDataToRead(int timeout);
	bool IsValid() const;
	int GetSocketAddresses(unsigned long *ips, int maxAddr);

#ifdef DEBUG

	class CStatistic
	{
		friend class CUDPSocket;
	public:
		CStatistic();
		void clear();
	public:
		unsigned int sentPacketsCount;
		unsigned int receivedPacketsCount;
		unsigned int sentBytesCount;
		unsigned int receivedBytesCount;
		unsigned int receivedErrorCount;
		unsigned int receivedBytesExpectedCount;
		unsigned int biggestSentPacketSize;
		unsigned int biggestReceivedPacketSize;
	};

	void clearStatistic() { m_statistic.clear(); }
	const CStatistic &getStatistic() const { return m_statistic; }
private:
	CStatistic m_statistic;
#endif

private:
	CUDPSocket_Impl *m_impl;
};

// Class representing TCP socket used in communication
class CTCPSocket
{
public:
	CTCPSocket();
	bool Open(const CHost &host);
	int Listen();
	CTCPSocket* Accept();
	void Close();
	bool Connect(const CHost &host);
	int Send(const unsigned char *buf, unsigned int len);
	int Recv(unsigned char *buf, int len);
	void SetBlocking();
	void SetNonBlocking();
	//
	int HasDataToRead(int timeout);
	bool IsValid() const;
	CHost GetHost() const;
private:
	CTCPSocket(CTCPSocket_Impl* impl) : m_impl(impl) {};
	CTCPSocket_Impl* m_impl;
};

//@}

#endif // !NETSOCKETS_H
