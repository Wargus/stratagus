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
/**@name netdriver.c - Session mangement (SDL_net Socket Implementation). */
//
//      (c) Copyright 2005 by Edward Haase and Jimmy Salmon
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
//      $Id$

//@{

/*----------------------------------------------------------------------------
--  Includes
----------------------------------------------------------------------------*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#ifndef _MSC_VER
#include <errno.h>
#endif

#include "stratagus.h"
#include "netdriver.h"
#include "net_lowlevel.h"

/*----------------------------------------------------------------------------
--  Defines
----------------------------------------------------------------------------*/

/**
**  LINK
**
**  Adds an item to a linked list.
*/
#define LINK(first, item, last, count) { \
	if (!first)                     \
		first = item;               \
	if (!last) {                    \
		last = item;                \
	} else {                        \
		item->Next = last->Next;    \
		last->Next = item;          \
		item->Prev = last;          \
		last = item;                \
		if (!item->Prev->Next)      \
			item->Prev->Next = item;\
	}                               \
	++count;                        \
}

/**
**  UNLINK
**
**  Removes an item from the linked list.
*/
#define UNLINK(first, item, last, count) { \
	if (item->Prev)                   \
		item->Prev->Next = item->Next;\
	if (item->Next)                   \
		item->Next->Prev = item->Prev;\
	if (item == last)                 \
		last = item->Prev;            \
	if (item == first)                \
		first = item->Next;           \
	--count;                          \
}

/*----------------------------------------------------------------------------
--  Variables
----------------------------------------------------------------------------*/

static Socket MasterSocket;

SessionPool* Pool;
ServerStruct Server;

/*----------------------------------------------------------------------------
--  Functions
----------------------------------------------------------------------------*/

/**
**  InitServer: Starts the SDL_Net process, allowing the gaming to commence.
**
**  @param port  Defines the port to which the server will bind.
*/
int InitServer(int port)
{
	Pool = NULL;

	if (NetInit() == -1) {
		return -1;
	}

	if ((MasterSocket = NetOpenTCP(port)) == -1) {
   		fprintf(stderr, "NetOpenTCP failed\n");
   		return -2;
	}

	if (NetSetNonBlocking(MasterSocket) == -1) {
		fprintf(stderr, "NetSetNonBlocking failed\n");
		NetCloseTCP(MasterSocket);
		NetExit();
		return -3;
	}

	if (NetListenTCP(MasterSocket) == -1) {
   		fprintf(stderr, "NetListenTCP failed\n");
		NetCloseTCP(MasterSocket);
		NetExit();
   		return -4;
	}

	if (!(Pool = (SessionPool*)calloc(1, sizeof(SessionPool)))) {
		fprintf(stderr, "Out of memory\n");
		NetCloseTCP(MasterSocket);
		NetExit();
		return -5;
	}

	if (!(Pool->SocketSet = NetAllocSocketSet())) {
		NetCloseTCP(MasterSocket);
		NetExit();
		return -6;
	}

	Pool->First = NULL;
	Pool->Last = NULL;
	Pool->Count = 0;

	return 0;
}

/**
**  TermServer: Releases the server socket.
*/
int TermServer(void)
{
	NetCloseTCP(MasterSocket);
	// begin clean up of any remaining sockets
	if (Pool) {
		Session *ptr;

		while (Pool->First) {
			ptr = Pool->First;
			UNLINK(Pool->First, Pool->First, Pool->Last, Pool->Count);
			NetCloseTCP(ptr->Socket);
			free(ptr);
		}

		NetFreeSocketSet(Pool->SocketSet);
		free(Pool);
	}

	NetExit();
	return 0;
}

/**
**  IdleSeconds: Returns time (in seconds) that a session has been idle.
**
**  @param ptr  This is the session we are checking.
*/
static int IdleSeconds(Session* ptr)
{
	return (int)(time(0) - ptr->Idle);
}

/**
**  KillSession: Destroys and cleans up session data.
**
**  @param ptr  Reference to the session to be killed.
*/
static int KillSession(Session* ptr)
{
	DebugPrint("Closing connection from '%s'\n" _C_ ptr->AddrData.IPStr);
	NetCloseTCP(ptr->Socket);
	NetDelSocket(Pool->SocketSet, ptr->Socket);
	UNLINK(Pool->First, ptr, Pool->Last, Pool->Count);
	free(ptr);
	return 0;
}

/**
**  Accept new connections
*/
static void AcceptConnections(void)
{
	Session* new_session;
	Socket new_socket;

	while ((new_socket = NetAcceptTCP(MasterSocket)) != -1) {
		// Check if we're at MaxConnections
		if (Pool->Count == Server.MaxConnections) {
			NetSendTCP(new_socket, "Server Full\n", 12);
			NetCloseTCP(new_socket);
			break;
		}

		new_session = (Session*)calloc(1, sizeof(Session));
		if (!new_session) {
			fprintf(stderr, "ERROR: %s\n", strerror(errno));
			break;
		}

		new_session->Socket = new_socket;
		new_session->Idle = time(0);

		new_session->AddrData.Host = NetLastHost;
		sprintf(new_session->AddrData.IPStr, "%d.%d.%d.%d",
			NIPQUAD(ntohl(NetLastHost)));
		new_session->AddrData.Port = NetLastPort;
		DebugPrint("New connection from '%s'\n" _C_ new_session->AddrData.IPStr);

		LINK(Pool->First, new_session, Pool->Last, Pool->Count);
		NetAddSocket(Pool->SocketSet, new_socket);
	}

}

/**
**  Kick idlers
*/
static void KickIdlers(void)
{
	Session* ptr;
	Session* next;

	for (ptr = Pool->First; ptr; ) {
		next = ptr->Next;
		if (IdleSeconds(ptr) > Server.IdleTimeout) {
			DebugPrint("Kicking idler '%s'\n" _C_ ptr->AddrData.IPStr);
			KillSession(ptr);
		}
		ptr = next;
	}
}

/**
**  Read data
*/
static int ReadData(void)
{
	Session* ptr;
	Session* next;
	int result;

	result = NetSocketSetReady(Pool->SocketSet, 0);
	if (result == 0) {
		// No sockets ready
		return 0;
	}
	if (result == -1) {
		// FIXME: print error message
		return -1;
	}

	// ready sockets
	for (ptr = Pool->First; ptr; ) {
		next = ptr->Next;
		if (NetSocketSetSocketReady(Pool->SocketSet, ptr->Socket)) {
			int clen;

			// socket ready
			ptr->Idle = time(0);
			clen = strlen(ptr->Buffer);
			result = NetRecvTCP(ptr->Socket, ptr->Buffer + clen,
				sizeof(ptr->Buffer) - clen);
			if (result <= 0) {
				KillSession(ptr);
			} else {
				ptr->Buffer[clen + result] = '\0';
			}
		}
		ptr = next;
	}

	return 0;
}

/**
**  UpdateSessions: Accepts new connections, receives data, manages buffers,
*/
int UpdateSessions(void)
{
	AcceptConnections();

	if (!Pool->First) {
		// No connections
		return 0;
	}

	KickIdlers();

	return ReadData();
}

//@}
