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
/**@name savegame.cpp - Save game. */
//
//      (c) Copyright 2001-2005 by Lutz Sammer, Andreas Arens
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

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <ctype.h>
#include <string>

#include "stratagus.h"
#include "icons.h"
#include "ui.h"
#include "construct.h"
#include "unittype.h"
#include "unit.h"
#include "upgrade.h"
#include "depend.h"
#include "interface.h"
#include "missile.h"
#include "tileset.h"
#include "map.h"
#include "player.h"
#include "ai.h"
#include "campaign.h"
#include "trigger.h"
#include "settings.h"
#include "iolib.h"
#include "spells.h"
#include "commands.h"
#include "script.h"
#include "actions.h"

/*----------------------------------------------------------------------------
--  Variables
----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
--  Functions
----------------------------------------------------------------------------*/

/**
** For saving lua state (table, number, string, bool, not function).
**
** @param l lua_State to save.
** @param is_root non-null for the main call, 0 for recursif call.
**
** @return NULL if nothing could be saved.
** else a string that could be executed in lua to restore lua state
** @todo do the output prettier (adjust indentation, newline)
*/
static char *SaveGlobal(lua_State *l, int is_root)
{
	int type_key;
	int type_value;
	const char *sep;
	const char *key;
	char *value;
	char *res;
	int first;
	char *tmp;
	int b;

//	Assert(!is_root || !lua_gettop(l));
	first = 1;
	res = NULL;
	if (is_root) {
		lua_pushstring(l, "_G");// global table in lua.
		lua_gettable(l, LUA_GLOBALSINDEX);
	}
	sep = (is_root) ? "" : ", ";
	Assert(lua_istable(l, -1));
	lua_pushnil(l);
	while (lua_next(l, -2)) {
		type_key = lua_type(l, -2);
		type_value = lua_type(l, -1);
		key = (type_key == LUA_TSTRING) ? lua_tostring(l, -2) : "";
		if (!strcmp(key, "_G") || (is_root &&
				(!strcmp(key, "assert") || !strcmp(key, "gcinfo") || !strcmp(key, "getfenv") ||
				!strcmp(key, "unpack") || !strcmp(key, "tostring") || !strcmp(key, "tonumber") ||
				!strcmp(key, "setmetatable") || !strcmp(key, "require") || !strcmp(key, "pcall") ||
				!strcmp(key, "rawequal") || !strcmp(key, "collectgarbage") || !strcmp(key, "type") ||
				!strcmp(key, "getmetatable") || !strcmp(key, "next") || !strcmp(key, "print") ||
				!strcmp(key, "xpcall") || !strcmp(key, "rawset") || !strcmp(key, "setfenv") ||
				!strcmp(key, "rawget") || !strcmp(key, "newproxy") || !strcmp(key, "ipairs") ||
				!strcmp(key, "loadstring") || !strcmp(key, "dofile") || !strcmp(key, "_TRACEBACK") ||
				!strcmp(key, "_VERSION") || !strcmp(key, "pairs") || !strcmp(key, "__pow") ||
				!strcmp(key, "error") || !strcmp(key, "loadfile") || !strcmp(key, "arg") ||
				!strcmp(key, "_LOADED") || !strcmp(key, "loadlib") || !strcmp(key, "string") ||
				!strcmp(key, "os") || !strcmp(key, "io") || !strcmp(key, "debug") ||
				!strcmp(key, "coroutine")
		// other string to protected ?
		))) {
			lua_pop(l, 1); // pop the value
			continue;
		}
		switch (type_value) {
			case LUA_TNIL:
				value = new_strdup("nil");
				break;
			case LUA_TNUMBER:
				value = new_strdup(lua_tostring(l, -1)); // let lua do the conversion
				break;
			case LUA_TBOOLEAN:
				b = lua_toboolean(l, -1);
				value = new_strdup(b ? "true" : "false");
				break;
			case LUA_TSTRING:
				value = strdcat3("\"", lua_tostring(l, -1), "\"");
				break;
			case LUA_TTABLE:
				lua_pushvalue(l, -1);
				tmp = SaveGlobal(l, 0);
				value = NULL;
				if (tmp != NULL) {
					value = strdcat3("{", tmp, "}");
					delete[] tmp;
				}
				break;
			case LUA_TFUNCTION:
			// Could be done with string.dump(function)
			// and debug.getinfo(function).name (coulb be nil for anonymous function)
			// But not usefull yet.
				value = NULL;
				break;
			case LUA_TUSERDATA:
			case LUA_TTHREAD:
			case LUA_TLIGHTUSERDATA:
			case LUA_TNONE:
			default : // no other cases
				value = NULL;
				break;
		}
		lua_pop(l, 1); /* pop the value */

		// Check the validity of the key (only [a-zA-z_])
		if (type_key == LUA_TSTRING) {
			int i;

			for (i = 0; key[i]; ++i) {
				if (!isalnum(key[i]) && key[i] != '_') {
					delete[] value;
					value = NULL;
					break;
				}
			}
		}
		if (value == NULL) {
			if (!is_root) {
				lua_pop(l, 2); // pop the key and the table
				return NULL;
			}
			continue;
		}
		if (type_key == LUA_TSTRING && !strcmp(key, value)) {
			continue;
		}
		if (first) {
			first = 0;
			if (type_key == LUA_TSTRING) {
				res = strdcat3(key, "=", value);
				delete[] value;
			} else {
				res = value;
			}
		} else {
			if (type_key == LUA_TSTRING) {
				tmp = value;
				value = strdcat3(key, "=", value);
				delete[] tmp;
				tmp = res;
				res = strdcat3(res, sep, value);
				delete[] tmp;
			} else {
				res = strdcat3(res, sep, value);
			}
		}
		tmp = res;
		res = strdcat3("", res, "\n");
		delete[] tmp;
	}
	lua_pop(l, 1); // pop the table
//	Assert(!is_root || !lua_gettop(l));
	return res;
}

/**
** Get the save directory and create dirs if needed
*/
static std::string GetSaveDir()
{
	std::string dir;

#ifdef USE_WIN32
	dir = GameName;
	mkdir(dir.c_str());
	dir += "/save";
	mkdir(dir.c_str());
	dir += "/";
#else
	dir = getenv("HOME");
	dir += "/";
	dir += STRATAGUS_HOME_PATH;
	mkdir(dir.c_str(), 0777);
	dir += "/";
	dir += GameName;
	mkdir(dir.c_str(), 0777);
	dir += "/save";
	mkdir(dir.c_str(), 0777);
	dir += "/";
#endif

	return dir;
}

/**
**  Save a game to file.
**
**  @param filename  File name to be stored.
**
**  @note  Later we want to store in a more compact binary format.
*/
void SaveGame(const char *filename)
{
	time_t now;
	CFile file;
	char *s;
	char *s1;
	std::string fullpath;

	fullpath = GetSaveDir() + filename;
	if (file.open(fullpath.c_str(), CL_WRITE_GZ | CL_OPEN_WRITE) == -1) {
		fprintf(stderr, "Can't save to `%s'\n", filename);
		return;
	}

	time(&now);
	s = ctime(&now);
	if ((s1 = strchr(s, '\n'))) {
		*s1 = '\0';
	}

	//
	// Parseable header
	//
	file.printf("SavedGameInfo({\n");
	file.printf("---  \"comment\", \"Generated by Stratagus Version " VERSION "\",\n");
	file.printf("---  \"comment\", \"Visit http://Stratagus.Org for more informations\",\n");
	file.printf("---  \"comment\", \"$Id$\",\n");
	file.printf("---  \"type\",    \"%s\",\n", "single-player");
	file.printf("---  \"date\",    \"%s\",\n", s);
	file.printf("---  \"map\",     \"%s\",\n", Map.Info.Description);
	file.printf("---  \"media-version\", \"%s\"", "Undefined");
	file.printf("---  \"engine\",  {%d, %d, %d},\n",
		StratagusMajorVersion, StratagusMinorVersion, StratagusPatchLevel);
	file.printf("  SyncHash = %d, \n", SyncHash);
	file.printf("  SyncRandSeed = %d, \n", SyncRandSeed);
	file.printf("  SaveFile = \"%s\"\n", CurrentMapPath);
	file.printf("\n---  \"preview\", \"%s.pam\",\n", filename);
	file.printf("} )\n\n");

	// FIXME: probably not the right place for this
	file.printf("SetGameCycle(%lu)\n", GameCycle);

	SaveCcl(&file);
	SaveUnitTypes(&file);
	SaveUpgrades(&file);
	SavePlayers(&file);
	Map.Save(&file);
	SaveUnits(&file);
	SaveUserInterface(&file);
	SaveAi(&file);
	SaveSelections(&file);
	SaveGroups(&file);
	SaveMissiles(&file);
	SaveTriggers(&file);
	SaveCampaign(&file);
	SaveObjectives(&file);
	SaveReplayList(&file);
	// FIXME: find all state information which must be saved.
	s = SaveGlobal(Lua, 1);
	if (s != NULL) {
		file.printf("-- Lua state\n\n %s\n", s);
		delete[] s;
	}
	file.close();
}

//@}
