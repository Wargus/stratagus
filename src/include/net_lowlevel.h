//   ___________		     _________		      _____  __
//   \_	  _____/______   ____   ____ \_   ___ \____________ _/ ____\/  |_
//    |    __) \_  __ \_/ __ \_/ __ \/    \  \/\_  __ \__  \\   __\\   __\ 
//    |     \   |  | \/\  ___/\  ___/\     \____|  | \// __ \|  |   |  |
//    \___  /   |__|    \___  >\___  >\______  /|__|  (____  /__|   |__|
//	  \/		    \/	   \/	     \/		   \/
//  ______________________                           ______________________
//			  T H E   W A R   B E G I N S
//	   FreeCraft - A free fantasy real time strategy game engine
//
/**@name net_lowlevel.h	-	The network low level header file. */
//
//	(c) Copyright 1998-2001 by Lutz Sammer
//
//	$Id$

#ifndef __NET_LOWLEVEL_H
#define __NET_LOWLEVEL_H

//@{

/*----------------------------------------------------------------------------
--	Includes
----------------------------------------------------------------------------*/

#include <errno.h>
#include <time.h>

// Include system network headers
#ifdef USE_SDL_NET
#include <SDLnet.h>
#else
#if defined(__WIN32__) || defined(WIN32)
#  define USE_WINSOCK
#  include <windows.h>
#else	// UNIX
#    include <sys/time.h>
#    include <unistd.h>
#  ifndef __BEOS__
#    include <arpa/inet.h>
#  endif
#  include <netinet/in.h>
#  include <netdb.h>
#  include <sys/socket.h>
#  define INVALID_SOCKET -1
#endif	// !WIN32
#endif // !USE_SDL_NET

#ifndef INADDR_NONE
#define INADDR_NONE -1
#endif

/*----------------------------------------------------------------------------
--	Defines
----------------------------------------------------------------------------*/

#define NIPQUAD(ad) \
	(int)(((ad) >> 24) & 0xff), (int)(((ad) >> 16) & 0xff), \
	(int)(((ad) >> 8) & 0xff), (int)((ad) & 0xff)

/*----------------------------------------------------------------------------
--	Declarations
----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
--	Variables
----------------------------------------------------------------------------*/

extern unsigned long NetLastHost;	/// Last host number (net format)
extern int NetLastPort;			/// Last port number (net format)

/*----------------------------------------------------------------------------
--	Functions
----------------------------------------------------------------------------*/

    /// Hardware dependend network init.
extern int NetInit(void);
    /// Hardware dependend network exit.
extern void NetExit(void);
    /// Resolve host in name or or colon dot notation.
extern unsigned long NetResolveHost(const char* host);
    /// Open an UDP Socket port.
extern int NetOpenUDP(int port);
    /// Close an UDP socket port.
extern void NetCloseUDP(int sockfd);
    /// Send through an UPD socket to a host:port.
extern int NetSendUDP(int sockfd,unsigned long host,int port
	,const void* buf,int len);
    /// Wait for socket ready.
extern int NetSocketReady(int sockfd,int timeout);
    /// Receive from an UDP socket.
extern int NetRecvUDP(int sockfd,void* buf,int len);

//@}

#endif	// !__NET_LOWLEVEL_H
