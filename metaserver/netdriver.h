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
/**@name netdriver.h - Net driver header. */
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

#ifndef __NETDRIVER_H__
#define __NETDRIVER_H__

//@{

/*----------------------------------------------------------------------------
--  Includes
----------------------------------------------------------------------------*/

#include <time.h>
#include "net_lowlevel.h"

/*----------------------------------------------------------------------------
--  Defines
----------------------------------------------------------------------------*/

#define DEFAULT_PORT			7775			// Server port
#define DEFAULT_MAX_CONN		500			// Max Connections
#define DEFAULT_SESSION_TIMEOUT		900			// 15 miniutes
#define DEFAULT_POLLING_DELAY		250			// MS (1000 = 1s)

#define MAX_USERNAME_LENGTH 32
#define MAX_PASSWORD_LENGTH 32

#define MAX_GAMENAME_LENGTH 32
#define MAX_VERSION_LENGTH 8

/*----------------------------------------------------------------------------
--  Declarations
----------------------------------------------------------------------------*/

struct _game_data_;

/**
** Global server variables.
*/
typedef struct _server_struct_ {
	int Port;
 	int MaxConnections;
	int IdleTimeout;
	int PollingDelay;
} ServerStruct;

extern ServerStruct Server;

/**
**  Session data
**
**  One per connection.
*/
typedef struct _session_ {
	struct _session_* Next;
	struct _session_* Prev;

	char Buffer[1024];
	time_t Idle;

	Socket Socket;

	struct {
		unsigned long Host;
		char IPStr[16];
		int Port;
	} AddrData;               /// Remote address data.

	struct {
		char Name[MAX_USERNAME_LENGTH + 1];
		char GameName[MAX_GAMENAME_LENGTH + 1];
		char Version[MAX_VERSION_LENGTH + 1];
		int LoggedIn;
	} UserData;               /// Specific user data.

	struct _game_data_* GameData;
} Session;

/**
**  Global session tracking.
*/
typedef struct _session_pool_ {
	Session* First;
	Session* Last;
	int Count;

	SocketSet* SocketSet;
} SessionPool;

	/// external reference to session tracking.
extern SessionPool* Pool;

/*----------------------------------------------------------------------------
--  Functions
----------------------------------------------------------------------------*/

extern void Send(Session* session, char* msg);

extern int ServerInit(int port);
extern void ServerQuit(void);
extern int UpdateSessions(void);

//@}

#endif // __NETDRIVER_H__
