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
/**@name query.c - Basic games query process. */
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

#include "stratagus.h"
#include "query.h"
#include "netdriver.h"
#include "cmd.h"

/*----------------------------------------------------------------------------
--  Functions
----------------------------------------------------------------------------*/

/**
**  ValidGame: determines if a user is hosting a game, compliant to checker.
**
**  @param ptr       Checker.
**  @param prospect  Prospective host.
*/
static int ValidGame(Session* ptr, Session* prospect)
{
	if (strstr(ptr->UserData.Service, prospect->UserData.Service) &&
			strstr(ptr->UserData.Version, prospect->UserData.Version) &&
			prospect->GameData.Port[0] != '\0') {
		return 1;
	}
	return 0;
}

/**
**  MatchingGames: count the number of valid games.
**
**  @param ptr  Checker.
*/
static int MatchingGames(Session* ptr)
{
	Session* tptr;
	int c;

	c = 0;
	for (tptr = Pool->First; tptr; tptr = tptr->Next) {
		if (ValidGame(ptr, tptr)) {
			++c;
		}
	}
	return c;
}

/**
**  MakeQueryString: format host string.
**
**  @param ptr  Session hosting the game.
*/
static char* MakeQueryString(Session* ptr)
{
	char* buffer;

	if (!(buffer = (char*)malloc(1024))) {
		printf("ERROR: %s\n", strerror(errno));
		return NULL;
	}
	buffer[1023] = '\0';

	snprintf(buffer, 1023, "OK\r\n%s\n%s\n%s\r\n%s\n%s\n%s\n%s\n%s\n%s\n",
		ptr->UserData.Name, ptr->UserData.Service, ptr->UserData.Version,
		ptr->AddrData.IPStr,
		ptr->GameData.Port, ptr->GameData.Name, ptr->GameData.Map,
		ptr->GameData.Slots.Max, ptr->GameData.Slots.Open);

	return buffer;
}

/**
**  StartQuery: Starts a new query for specified session.
**
**  @param ptr  Checker.
*/
int StartQuery(Session* ptr)
{
	Session* tptr;
	char query_result_string[64];
	int i;

	i = 0;

	// safety
	if (ptr->QueryData.List) {
		CleanUpQuery(ptr);
	}

	// don't allocate anything if there isn't anything to query
	if ((ptr->QueryData.Count = MatchingGames(ptr)) <= 0) {
		ptr->QueryData.Count = 0;
		i = sprintf(query_result_string, "%d\r\n%d\r\n%d\r\n1\r\n",
			(ptr->QueryData.Count + 1),
			(ptr->QueryData.Count + 1),
			(ptr->QueryData.Count + 1));
		SDLNet_TCP_Send(ptr->Socket, query_result_string, i);
		return 0;
	}

	// create array of strings
	ptr->QueryData.List = (char**)calloc(ptr->QueryData.Count, sizeof(char*));
	if (!ptr->QueryData.List) {
		return 1;
	}

	// add matching games to list.
	for (tptr = Pool->First; tptr; tptr = tptr->Next) {
		if (ValidGame(ptr, tptr)) {
			ptr->QueryData.List[i++] = MakeQueryString(tptr);
		}
	}

	i = sprintf(query_result_string, "%d\r\n%d\r\n%d\r\n1\r\n",
		(ptr->QueryData.Count + 1),
		(ptr->QueryData.Count + 1),
		(ptr->QueryData.Count + 1));

	SDLNet_TCP_Send(ptr->Socket, query_result_string, i);
	return 0;
}

/**
**  QueryGame: Send game info from session's query.
**
**  @param ptr  Checker.
**  @param n    Game number.
*/
int QueryGame(Session* ptr, int n)
{
	if (!ptr->QueryData.List || n > ptr->QueryData.Count) {
		// handle error?
	} else {
		SDLNet_TCP_Send(ptr->Socket, ptr->QueryData.List[n - 1],
			strlen(ptr->QueryData.List[n - 1]));
		if (n == ptr->QueryData.Count) {
			CleanUpQuery(ptr);
		}
	}
	return 0;
}

/**
**  CleanUpQuery: Release memory from query.
**
**  @param ptr  Session currently with query.
*/
int CleanUpQuery(Session* ptr)
{
	if (ptr->QueryData.List) {
		while (ptr->QueryData.Count-- > 0) {
			free(ptr->QueryData.List[ptr->QueryData.Count]);
		}
		free(ptr->QueryData.List);
		ptr->QueryData.List = NULL;
	}

	return 0;
}


//@}
