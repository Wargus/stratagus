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
/**@name cmd.c - Client/Server Command Interpreter. */
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

//@{

/*----------------------------------------------------------------------------
--  Includes
----------------------------------------------------------------------------*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "stratagus.h"
#include "cmd.h"
#include "netdriver.h"
#include "query.h"

/*----------------------------------------------------------------------------
--  Functions
----------------------------------------------------------------------------*/

/**
**  ParseBuffer: Handler client/server interaction.
**
**  @param ptr  Current session.
*/
static int ParseBuffer(Session* ptr)
{
	static char cmd[128];
	static char dump[128];

	if (!ptr) {
		return -1;
	}

	if (ptr->Buffer[0] == '\0') {
		return 0;
	}

	//
	// Read message header.
	//
	sscanf(ptr->Buffer, "<Stratagus>\n%s\n%s\n%s\n%s\n",
		ptr->UserData.Name, ptr->UserData.Service,
	    ptr->UserData.Version, cmd);

	//
	// Take command-specific action.
	//
	if (!strcmp(cmd, "Login")) {
		SDLNet_TCP_Send(ptr->Socket, "OK\r\n", 4);
		return 0;
	}

	if (!strcmp(cmd, "AbandonGame")) {
		printf("Cancel game: [%s:%d] [%s(%s):%s] [%s] [%s/%s]\n",
			ptr->AddrData.IPStr, atoi(ptr->GameData.Port),
			ptr->UserData.Service, ptr->UserData.Version, ptr->GameData.Name,
			ptr->GameData.Map,
			ptr->GameData.Slots.Open, ptr->GameData.Slots.Max);

		memset(&ptr->GameData, 0, sizeof(ptr->GameData));
		SDLNet_TCP_Send(ptr->Socket, "OK\r\n", 4);
		return 0;
	}

	if (!strcmp(cmd, "AddGame")) {
		sscanf(ptr->Buffer, "<Stratagus>\n%s\n%s\n%s\n%s\nIP\n%s\n%s\n%s\n%s\n%s\n",
			dump, dump, dump, dump,
			ptr->GameData.Port, ptr->GameData.Name, ptr->GameData.Map,
			ptr->GameData.Slots.Max, ptr->GameData.Slots.Open);


		// creation note
		printf("New game: [%s:%d] [%s(%s):%s] [%s] [%s/%s]\n",
			ptr->AddrData.IPStr, atoi(ptr->GameData.Port),
			ptr->UserData.Service, ptr->UserData.Version, ptr->GameData.Name,
			ptr->GameData.Map,
			ptr->GameData.Slots.Open, ptr->GameData.Slots.Max);

		SDLNet_TCP_Send(ptr->Socket, "OK\r\n", 4);
		return 0;
	}

	if (!strcmp(cmd, "StartGame")) {
		printf("Start game: [%s:%d] [%s(%s):%s] [%s] [%s/%s]\n",
			ptr->AddrData.IPStr, atoi(ptr->GameData.Port),
			ptr->UserData.Service, ptr->UserData.Version, ptr->GameData.Name,
			ptr->GameData.Map,
			ptr->GameData.Slots.Open, ptr->GameData.Slots.Max);

		memset(&ptr->GameData, 0, sizeof(ptr->GameData));
		SDLNet_TCP_Send(ptr->Socket, "OK\r\n", 4);
		return 0;
	}

	if (!strcmp(cmd, "ChangeGame")) {
		sscanf(ptr->Buffer, "<Stratagus>\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s",
			dump, dump, dump, dump,
			ptr->GameData.Name, ptr->GameData.Map,
			ptr->GameData.Slots.Max, ptr->GameData.Slots.Open);

		printf("Change game: [%s:%d] [%s(%s):%s] [%s] [%s/%s]\n",
			ptr->AddrData.IPStr, atoi(ptr->GameData.Port),
			ptr->UserData.Service, ptr->UserData.Version, ptr->GameData.Name,
			ptr->GameData.Map,
			ptr->GameData.Slots.Open, ptr->GameData.Slots.Max);

		SDLNet_TCP_Send(ptr->Socket, "OK\r\n", 4);
		return 0;
	}

	if (!strcmp(cmd, "NumberOfGames")) {
		StartQuery(ptr);
		return 0;
	}

	if (!strcmp(cmd, "GameNumber")) {
		char val[8];

		memset(val, '\0', sizeof(val));

		sscanf(ptr->Buffer, "<Stratagus>\n%s\n%s\n%s\n%s\n%s",
			dump, dump, dump, dump,	val);

		QueryGame(ptr, atoi(val));
		return 0;
	}

	//
	// Default Action?
	//
	printf("Unknown Command: [%s]\n", cmd);

	return 0;
}

/**
**  UpdateParser: iterate sessions, push to parser.
*/
int UpdateParser(void)
{
	Session* ptr;
	int len;
	char* next;

	if (!Pool || !Pool->First) {
		return 0;
	}

	//
	// iterations.
	//
	for (ptr = Pool->First; ptr; ptr = ptr->Next) {
		//
		// Confirm full message.
		//
		if ((next = strstr(ptr->Buffer, "\n\n"))) {
			//
			// Parse message.
			//
			ParseBuffer(ptr);

			//
			// Terminate message.
			//
			len = next - ptr->Buffer + 2;
			memmove(ptr->Buffer, ptr->Buffer + len, sizeof(ptr->Buffer) - len);
			memset(ptr->Buffer + (sizeof(ptr->Buffer) - len), 0, len);
		}

	}
	return 0;
}

//@}
