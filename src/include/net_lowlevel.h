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
/**@name net_lowlevel.h - The network low level header file. */
//
//      (c) Copyright 1998-2007 by Lutz Sammer and Jimmy Salmon
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

#ifndef __NET_LOWLEVEL_H
#define __NET_LOWLEVEL_H

//@{

/*----------------------------------------------------------------------------
--  Includes
----------------------------------------------------------------------------*/

#include <vector>

// Include system network headers
#ifdef USE_WIN32

# define USE_WINSOCK

# include <winsock2.h>

#else // UNIX
# include <sys/time.h>
# include <unistd.h>
# include <netinet/in.h>
# include <netdb.h>
# include <sys/socket.h>
# include <sys/ioctl.h>
#if __has_include(<ifaddrs.h>)
# include <ifaddrs.h>
# ifndef __BEOS__
#  include <net/if.h>
#  include <arpa/inet.h>
# endif
#elif __has_include(<proto/socket.h>)
# include <proto/socket.h>
#endif
# define INVALID_SOCKET -1

#endif // !WIN32

#ifndef INADDR_NONE
# define INADDR_NONE -1
#endif

/*----------------------------------------------------------------------------
--  Defines
----------------------------------------------------------------------------*/

#define NIPQUAD(ad) \
	(int)(((ad) >> 24) & 0xff), (int)(((ad) >> 16) & 0xff), \
	(int)(((ad) >> 8) & 0xff), (int)((ad) & 0xff)

#ifdef USE_WINSOCK
typedef SOCKET SocketFD;
#else
typedef int SocketFD;
#endif

/*----------------------------------------------------------------------------
--  Declarations
----------------------------------------------------------------------------*/

class SocketSet
{
public:
	SocketSet() : MaxSockFD(0) {}

	void AddSocket(SocketFD socket);
	void DelSocket(SocketFD socket);

	/// Wait for socket set ready.
	int Select(int timeout);
	/// Check if a socket in a socket set is ready.
	int HasDataToRead(SocketFD socket) const;

private:
	std::vector<SocketFD> Sockets;
	std::vector<int> SocketReady;
	SocketFD MaxSockFD;
};

/*----------------------------------------------------------------------------
--  Functions
----------------------------------------------------------------------------*/

/// Hardware dependend network init.
extern int NetInit();
/// Hardware dependend network exit.
extern void NetExit();

/// Resolve host in name or or colon dot notation.
extern unsigned long NetResolveHost(const std::string &host);
/// Get local IPs
extern int NetSocketAddr(unsigned long *ips, int maxAddr);
/// Get local hostname
extern std::string NetGetHostname();

/// Open a UDP Socket port. (param in network format)
extern SocketFD NetOpenUDP(unsigned long ip, int port);
/// Close a UDP socket port.
extern void NetCloseUDP(SocketFD sockfd);
/// Send through a UPD socket to a host:port.
extern int NetSendUDP(SocketFD sockfd, unsigned long host, int port, const void *buf, int len);
/// Receive from a UDP socket.
extern int NetRecvUDP(SocketFD sockfd, void *buf, int len, unsigned long *hostFrom, int *portFrom);


/// Open a TCP Socket port.
extern SocketFD NetOpenTCP(const char *addr, int port);
/// Close a TCP socket port.
extern void NetCloseTCP(SocketFD sockfd);
/// Open a TCP connection.
extern int NetConnectTCP(SocketFD sockfd, unsigned long addr, int port);
/// Send through a TCP socket
extern int NetSendTCP(SocketFD sockfd, const void *buf, int len);
/// Receive from a TCP socket.
extern int NetRecvTCP(SocketFD sockfd, void *buf, int len);
/// Listen for connections on a TCP socket
extern int NetListenTCP(SocketFD sockfd);
/// Accept a connection on a TCP socket
extern SocketFD NetAcceptTCP(SocketFD sockfd, unsigned long *clientHost, int *clientPort);


/// Set socket to non-blocking
extern int NetSetNonBlocking(SocketFD sockfd);
/// Wait for socket ready.
extern int NetSocketReady(SocketFD sockfd, int timeout);

//@}

#endif // !__NET_LOWLEVEL_H
