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
//      (c) Copyright 2005 by Edward Haase
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

#include "stratagus.h"
#include "netdriver.h"

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

static IPaddress IP;
static TCPsocket MasterSocket;

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

	if (SDLNet_Init() == -1) {
		return -1;
	}

	if (SDLNet_ResolveHost(&IP, NULL, (unsigned short)port) == -1) {
   		fprintf(stderr, "SDLNet_ResolveHost: %s\n", SDLNet_GetError());
   		return -2;
	}

	if (!(MasterSocket = SDLNet_TCP_Open(&IP))) {
   		fprintf(stderr, "SDLNet_TCP_Open: %s\n", SDLNet_GetError());
   		return -3;
	}

	if (!(Pool = (SessionPool*)calloc(1, sizeof(SessionPool)))) {
		SDLNet_TCP_Close(MasterSocket);
		SDLNet_Quit();
		return -4;
	}

	if (!(Pool->StatusQue = SDLNet_AllocSocketSet(Server.MaxConnections))) {
		SDLNet_TCP_Close(MasterSocket);
		SDLNet_Quit();
		return -5;
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
	SDLNet_TCP_Close(MasterSocket);
	// begin clean up of any remaining sockets
	if (Pool) {
		Session *ptr;

		while (Pool->First) {
			ptr = Pool->First;
			UNLINK(Pool->First, Pool->First, Pool->Last, Pool->Count);
			SDLNet_TCP_Close(ptr->Socket);
			free(ptr);
		}

		SDLNet_FreeSocketSet(Pool->StatusQue);
		free(Pool);
	}

	SDLNet_Quit();
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
	SDLNet_TCP_Close(ptr->Socket);
	SDLNet_TCP_DelSocket(Pool->StatusQue, ptr->Socket);
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
	TCPsocket new_socket;

	while ((new_socket = SDLNet_TCP_Accept(MasterSocket))) {
		// Check if we're at MaxConnections
		if (Pool->Count == Server.MaxConnections) {
			SDLNet_TCP_Send(new_socket, "Server Full\n", 12);
			SDLNet_TCP_Close(new_socket);
			break;
		}

		new_session = (Session*)calloc(1, sizeof(Session));
		if (!new_session) {
			fprintf(stderr, "ERROR: %s\n", strerror(errno));
			break;
		}

		new_session->Socket = new_socket;
		new_session->Idle = time(0);

		if (!(new_session->AddrData.Address = SDLNet_TCP_GetPeerAddress(new_socket))) {
			printf("SDLNet_TCP_GetPeerAddress: %s\n", SDLNet_GetError());
		} else {
			Uint32 tipaddr;

			new_session->AddrData.Port = new_session->AddrData.Address->port;

			tipaddr = SDL_SwapBE32(new_session->AddrData.Address->host);

			// build useable string
			sprintf(new_session->AddrData.IPStr, "%d.%d.%d.%d",
				tipaddr >> 24, (tipaddr >> 16) & 0xff,
				(tipaddr >> 8) & 0xff, tipaddr & 0xff);
			DebugPrint("New connection from '%s'\n" _C_ new_session->AddrData.IPStr);
		}

		LINK(Pool->First, new_session, Pool->Last, Pool->Count);
		SDLNet_TCP_AddSocket(Pool->StatusQue, new_socket);
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

	result = SDLNet_CheckSockets(Pool->StatusQue, 0);
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
		if (SDLNet_SocketReady(ptr->Socket)) {
			int clen;

			// socket ready
			ptr->Idle = time(0);
			clen = strlen(ptr->Buffer);
			result = SDLNet_TCP_Recv(ptr->Socket, ptr->Buffer + clen,
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
