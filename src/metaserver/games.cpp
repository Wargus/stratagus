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
/**@name games.cpp - Basic games query process. */
//
//      (c) Copyright 2005 by Jimmy Salmon
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "stratagus.h"
#include "games.h"
#include "netdriver.h"

/*----------------------------------------------------------------------------
--  Variables
----------------------------------------------------------------------------*/

static GameData *Games;
int GameID;

/*----------------------------------------------------------------------------
--  Functions
----------------------------------------------------------------------------*/

/**
**  Create a game
*/
void CreateGame(Session *session, char *description, char *map,
	char *players, char *ip, char *port, char *password)
{
	GameData *game;

	game = new GameData;

	strcpy(game->IP, ip);
	strcpy(game->Port, port);
	strcpy(game->Description, description);
	strcpy(game->Map, map);
	game->MaxSlots = atoi(players);
	game->OpenSlots = game->MaxSlots - 1;
	if (password) {
		strcpy(game->Password, password);
	} else {
		game->Password[0] = '\0';
	}

	game->NumSessions = 1;
	game->Sessions[0] = session;
	game->ID = GameID++;
	game->Started = 0;

	game->GameName = session->UserData.GameName;
	game->Version = session->UserData.Version;

	if (Games) {
		Games->Prev = game;
	}
	game->Next = Games;
	game->Prev = NULL;
	Games = game;

	session->Game = game;
}

/**
**  Cancel a game
*/
int CancelGame(Session *session)
{
	GameData *game;
	int i;

	game = session->Game;

	if (game->Sessions[0] != session) {
		return -1; // Not the host
	}

	if (game->Next) {
		game->Next->Prev = game->Prev;
	}
	if (game->Prev) {
		game->Prev->Next = game->Next;
	}
	if (Games == game) {
		Games = game->Next;
	}

	for (i = 0; i < game->NumSessions; ++i) {
		game->Sessions[i]->Game = NULL;
	}

	delete game;
	return 0;
}

/**
**  Start a game
*/
int StartGame(Session *session)
{
	if (session->Game->Sessions[0] != session) {
		return -1; // Not the host
	}

	session->Game->Started = 1;
	return 0;
}

/**
**  Join a game
*/
int JoinGame(Session *session, int id, char *password)
{
	GameData *game;

	if (session->Game) {
		return -1; // Already in a game
	}

	game = Games;
	while (game) {
		if (game->ID == id) {
			break;
		}
		game = game->Next;
	}
	if (!game) {
		return -2; // ID not found
	}

	if (game->Password[0]) {
		if (!password || strcmp(game->Password, password)) {
			return -3; // Wrong password
		}
	}
	if (!game->OpenSlots) {
		return -4; // Game full
	}
	game->Sessions[game->NumSessions++] = session;
	session->Game = game;

	return 0;
}

/**
**  Leave a game
*/
int PartGame(Session *session)
{
	GameData *game;
	int i;

	game = session->Game;

	if (!game) {
		return -1; // Not in a game
	}
	if (game->Started) {
		return -2;
	}

	if (game->Sessions[0] == session) {
		// The host left, cancel the game
		CancelGame(session);
		return 0;
	}

	for (i = 1; i < game->NumSessions; ++i) {
		if (game->Sessions[i] == session) {
			for (; i < game->NumSessions - 1; ++i) {
				game->Sessions[i] = game->Sessions[i + 1];
			}
			game->NumSessions--;
			break;
		}
	}

	session->Game = NULL;

	return 0;
}

static int MatchGameType(Session *session, GameData *game)
{
	return (!*game->GameName || !strcmp(session->UserData.GameName, game->GameName)) &&
		(!*game->Version || !strcmp(session->UserData.Version, game->Version));
}

/**
**  List games
*/
void ListGames(Session *session)
{
	GameData *game;
	char buf[1024];

	game = Games;
	while (game) {
		if (!game->Started && MatchGameType(session, game)) {
			sprintf(buf, "LISTGAMES %d \"%s\" \"%s\" %d %d %s %s\n",
				game->ID, game->Description, game->Map,
				game->OpenSlots, game->MaxSlots, game->IP, game->Port);
			Send(session, buf);
		}
		game = game->Next;
	}
}
