//       _________ __                 __                               
//      /   _____//  |_____________ _/  |______     ____  __ __  ______
//      \_____  \\   __\_  __ \__  \\   __\__  \   / ___\|  |  \/  ___/
//      /        \|  |  |  | \// __ \|  |  / __ \_/ /_/  >  |  /\___ |
//     /_______  /|__|  |__|  (____  /__| (____  /\___  /|____//____  >
//             \/                  \/          \//_____/            \/ 
//  ______________________                           ______________________
//			  T H E   W A R   B E G I N S
//	   Stratagus - A free fantasy real time strategy game engine
//
/**@name master.c	-	The master server. */
//
//	(c) Copyright 2003 by Tom Zickel and Jimmy Salmon
//
//      This program is free software; you can redistribute it and/or modify
//      it under the terms of the GNU General Public License as published by
//      the Free Software Foundation; version 2 dated June, 1991.
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
//	$Id$

//@{

/*----------------------------------------------------------------------------
--	Includes
----------------------------------------------------------------------------*/

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <ctype.h>
#include <limits.h>

#ifndef _MSC_VER
#include <fcntl.h>
#endif

#include "stratagus.h"

#include "iocompat.h"

#include "network.h"
#include "netconnect.h"
#include "ccl.h"
#include "master.h"

// FIXME: jim4: why is this defined?
#define USE_WINSOCK

#include "net_lowlevel.h"

#ifdef USE_SDLA
#include "SDL.h"
#endif

/*----------------------------------------------------------------------------
--	Variables
----------------------------------------------------------------------------*/

//###### For Magnant META SERVER
local int sockfd;  // This is a TCP socket. 
global int MetaServerInUse;

/*----------------------------------------------------------------------------
--	Functions
----------------------------------------------------------------------------*/


/**
**	Initialize the TCP connection to the Meta Server.
**
**	@return	-1 fail, 0 success.
*/
global int MetaInit(void)
{
    int TCPConnectStatus; // = 0 if not successful, -1 if not.
    int i; 
    char** reply;

    reply = NULL;
    sockfd = NetworkFildes;
    for (i = 1234; i < 1244; ++i) {
	sockfd = NetOpenTCP(i);	//FIXME: need to make a dynamic port allocation there...if (!sockfd) {...}
	if (sockfd != -1) {
	    break;
	}
    }
    
    // FIXME: Configurable Meta Server
    TCPConnectStatus = NetConnectTCP(sockfd, NetResolveHost(MASTER_HOST), MASTER_PORT);

    if (TCPConnectStatus == -1) {
	//TODO: Notify player that connection was aborted...
	return -1; 
    }
	
    if (SendMetaCommand("Login","") == -1) {
	//TODO: Notify player that connection was aborted...
	return -1;
    }

    if (RecvMetaReply(reply) == -1) {
	//TODO: Notify player that connection was aborted...
	return -1;
    } else {
	if (MetaServerOK(reply)) {
	    free(*reply);
	    return 0;
	} else {
	    free(*reply);
	    return -1;
	}
    }

    return 0;
}

/**	Close Connection to Master Server
**
**	@return	nothing
*/
global int MetaClose(void)
{
    NetCloseTCP(sockfd);
    return 0;
}

/**
**	Checks if a Message was OK or ERR
**
**	@return 1 OK, 0 Error.
*/
global int MetaServerOK(char** reply)
{
    return !strcmp("OK\r\n",*reply) || !strcmp("OK\n",*reply);
}

/**
**	Send a command to the meta server
**
**	@param command	command to send
**	@param format	format of parameters
**	@param ...	parameters
**
**	@returns	-1 fail, length of command
*/
global int SendMetaCommand(char* command, char* format, ...)
{
    int n;
    int size;
    int ret;
    char* p;
    char* s;
    va_list ap;

    size = strlen(GameName) + strlen(LocalPlayerName) + strlen(command) + 100;
    ret = -1;
    if ((p = malloc(size)) == NULL) {
	return -1;
    }
    if ((s = malloc(size)) == NULL) {
	return -1;
    }

    // Message Structure
    // <Stratagus> if for Magnant Compatibility, it may be removed
    // Player Name, Game Name, VERSION, Command, **Paramaters**
    sprintf(s, "<Stratagus>\n%s\n%s\n%s\n%s\n", LocalPlayerName, GameName, VERSION, command);

    // Commands
    // Login - password
    // Logout - 0
    // AddGame - IP,Port,Description,Map,Players,FreeSpots
    // JoinGame - Nick of Hoster
    // ChangeGame - Description,Map,Players,FreeSpots
    // GameList - 0
    // NextGameInList - 0
    // StartGame - 0
    // PlayerScore - Player,Score,Win (Add razings...)
    // EndGame - Called after PlayerScore.
    // AbandonGame - 0
    while (1) {
	/* Try to print in the allocated space. */
	va_start(ap, format);
	n = vsnprintf(p, size, format, ap);
	va_end(ap);
	/* If that worked, string was processed. */
	if (n > -1 && n < size) {
	    break;
	}
	/* Else try again with more space. */
	if (n > -1) { /* glibc 2.1 */
	    size = n + 1; /* precisely what is needed */
	} else {           /* glibc 2.0 */
	    size *= 2;  /* twice the old size */
	}
	if ((p = realloc(p, size)) == NULL) {
	    return -1;
	}
    }
    // Allocate the correct size
    if ((s = realloc(s, size + strlen(s))) == NULL ) {
	free(p);
	return -1;
    }
    strcat(s, p);
    size = strlen(s);
    ret = NetSendTCP(sockfd, s, size);
    free(p);
    free(s);
    return ret;
}

/**
**	Receive reply from Meta Server
**
**	@param	reply	Text of the reply
**	@return	error or number of bytes
*/
global int RecvMetaReply(char** reply)
{
    int n;
    int size;
    char *p;
    char buf[1024];

    if (NetSocketReady(sockfd, 5000) == -1) {
	return -1;
    }
   
    size = 1;
    p = NULL;
    while ((n = NetRecvTCP(sockfd, &buf, 1024))) {
	size += n;
	if ((p = realloc(p, size)) == NULL) {
	    return -1;
	}
	strcat(p, buf);
    }

    reply = &p;
    return size;
}
