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
/**@name netdriver.cpp - Session mangement (SDL_net Socket Implementation). */
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

SessionPool *Pool;
ServerStruct Server;

/*----------------------------------------------------------------------------
--  Functions
----------------------------------------------------------------------------*/

/**
**  Send a message to a session
**
**  @param session  Session to send the message to
**  @param msg      Message to send
*/
void Send(Session *session, const char *msg)
{
	NetSendTCP(session->Sock, msg, strlen(msg));
}

/**
**  Initialize the server
**
**  @param port  Defines the port to which the server will bind.
**
**  @return      0 for success, non-zero for failure
*/
int ServerInit(int port)
{
	Pool = NULL;

	if (NetInit() == -1) {
		return -1;
	}

	if ((MasterSocket = NetOpenTCP(NULL, port)) == (Socket)-1) {
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

	if (!(Pool = new SessionPool)) {
		fprintf(stderr, "Out of memory\n");
		NetCloseTCP(MasterSocket);
		NetExit();
		return -5;
	}

	if (!(Pool->Sockets = new SocketSet)) {
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
**  ServerQuit: Releases the server socket.
*/
void ServerQuit(void)
{
	NetCloseTCP(MasterSocket);
	// begin clean up of any remaining sockets
	if (Pool) {
		Session *ptr;

		while (Pool->First) {
			ptr = Pool->First;
			UNLINK(Pool->First, Pool->First, Pool->Last, Pool->Count);
			NetCloseTCP(ptr->Sock);
			delete ptr;
		}

		delete Pool->Sockets;
		delete Pool;
	}

	NetExit();
}

/**
**  Returns time (in seconds) that a session has been idle.
**
**  @param session  This is the session we are checking.
*/
static int IdleSeconds(Session *session)
{
	return (int)(time(0) - session->Idle);
}

/**
**  Destroys and cleans up session data.
**
**  @param session  Reference to the session to be killed.
*/
static int KillSession(Session *session)
{
	DebugPrint("Closing connection from '%s'\n" _C_ session->AddrData.IPStr);
	NetCloseTCP(session->Sock);
	Pool->Sockets->DelSocket(session->Sock);
	UNLINK(Pool->First, session, Pool->Last, Pool->Count);
	delete session;
	return 0;
}

/**
**  Accept new connections
*/
static void AcceptConnections(void)
{
	Session *new_session;
	Socket new_socket;

	while ((new_socket = NetAcceptTCP(MasterSocket)) != (Socket)-1) {
		// Check if we're at MaxConnections
		if (Pool->Count == Server.MaxConnections) {
			NetSendTCP(new_socket, "Server Full\n", 12);
			NetCloseTCP(new_socket);
			break;
		}

		new_session = new Session;
		if (!new_session) {
			fprintf(stderr, "ERROR: %s\n", strerror(errno));
			break;
		}

		new_session->Sock = new_socket;
		new_session->Idle = time(0);

		new_session->AddrData.Host = NetLastHost;
		sprintf(new_session->AddrData.IPStr, "%d.%d.%d.%d",
			NIPQUAD(ntohl(NetLastHost)));
		new_session->AddrData.Port = NetLastPort;
		DebugPrint("New connection from '%s'\n" _C_ new_session->AddrData.IPStr);

		LINK(Pool->First, new_session, Pool->Last, Pool->Count);
		Pool->Sockets->AddSocket(new_socket);
	}

}

/**
**  Kick idlers
*/
static void KickIdlers(void)
{
	Session *session;
	Session *next;

	for (session = Pool->First; session; ) {
		next = session->Next;
		if (IdleSeconds(session) > Server.IdleTimeout) {
			DebugPrint("Kicking idler '%s'\n" _C_ session->AddrData.IPStr);
			KillSession(session);
		}
		session = next;
	}
}

/**
**  Read data
*/
static int ReadData(void)
{
	Session *session;
	Session *next;
	int result;

	result = NetSocketSetReady(Pool->Sockets, 0);
	if (result == 0) {
		// No sockets ready
		return 0;
	}
	if (result == -1) {
		// FIXME: print error message
		return -1;
	}

	// ready sockets
	for (session = Pool->First; session; ) {
		next = session->Next;
		if (NetSocketSetSocketReady(Pool->Sockets, session->Sock)) {
			int clen;

			// socket ready
			session->Idle = time(0);
			clen = strlen(session->Buffer);
			result = NetRecvTCP(session->Sock, session->Buffer + clen,
				sizeof(session->Buffer) - clen);
			if (result < 0) {
				KillSession(session);
			} else {
				session->Buffer[clen + result] = '\0';
			}
		}
		session = next;
	}

	return 0;
}

/**
**  Accepts new connections, receives data, manages buffers,
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
