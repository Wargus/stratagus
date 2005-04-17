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
/**@name optargs.h - Command line parser header. */
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

#ifndef __GAMES_H__
#define __GAMES_H__

//@{

/*----------------------------------------------------------------------------
--  Defines
----------------------------------------------------------------------------*/

#define MAX_DESCRIPTION_LENGTH 64
#define MAX_MAP_LENGTH 64

/*----------------------------------------------------------------------------
--  Declarations
----------------------------------------------------------------------------*/

struct _session_;

typedef struct _game_data_
{
	char IP[16];
	char Port[6];
	char Description[MAX_DESCRIPTION_LENGTH + 1];
   	char Map[MAX_MAP_LENGTH + 1];
	int OpenSlots;
	int MaxSlots;

	char* GameName;
	char* Version;

	struct _session_* Sessions[16];
	int NumSessions;
	int ID;
	int Started;

	struct _game_data_* Next;
	struct _game_data_* Prev;
} GameData;

extern int GameID;

/*----------------------------------------------------------------------------
--  Functions
----------------------------------------------------------------------------*/

extern void CreateGame(struct _session_* session, char* description, char* map,
	char* players, char* ip, char* port);
extern int CancelGame(struct _session_* session);
extern int StartGame(struct _session_* session);
extern int JoinGame(struct _session_* session, int id);
extern int PartGame(struct _session_* session);
extern void ListGames(struct _session_* session);

//@}

#endif // __GAMES_H__
