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
#include "results.h"
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
**  For saving lua state (table, number, string, bool, not function).
**
**  @param l        lua_State to save.
**  @param is_root  true for the main call, 0 for recursif call.
**
**  @return  "" if nothing could be saved.
**           else a string that could be executed in lua to restore lua state
**  @todo    do the output prettier (adjust indentation, newline)
*/
std::string SaveGlobal(lua_State *l, bool is_root)
{
	int type_key;
	int type_value;
	std::string value;
	int first;
	std::string res;
	std::string tmp;
	int b;

//	Assert(!is_root || !lua_gettop(l));
	first = 1;
	if (is_root) {
		lua_pushstring(l, "_G");// global table in lua.
		lua_gettable(l, LUA_GLOBALSINDEX);
	}
	const std::string sep = is_root ? "" : ", ";
	Assert(lua_istable(l, -1));
	lua_pushnil(l);
	while (lua_next(l, -2)) {
		type_key = lua_type(l, -2);
		type_value = lua_type(l, -1);
		const std::string key = (type_key == LUA_TSTRING) ? lua_tostring(l, -2) : "";
		if ((key == "_G") || (is_root &&
				(key == "assert") || (key == "gcinfo") || (key == "getfenv") ||
				(key == "unpack") || (key == "tostring") || (key == "tonumber") ||
				(key == "setmetatable") || (key == "require") || (key == "pcall") ||
				(key == "rawequal") || (key == "collectgarbage") || (key == "type") ||
				(key == "getmetatable") || (key == "next") || (key == "print") ||
				(key == "xpcall") || (key == "rawset") || (key == "setfenv") ||
				(key == "rawget") || (key == "newproxy") || (key == "ipairs") ||
				(key == "loadstring") || (key == "dofile") || (key == "_TRACEBACK") ||
				(key == "_VERSION") || (key == "pairs") || (key == "__pow") ||
				(key == "error") || (key == "loadfile") || (key == "arg") ||
				(key == "_LOADED") || (key == "loadlib") || (key == "string") ||
				(key == "os") || (key == "io") || (key == "debug") ||
				(key == "coroutine") || (key == "Icons") || (key == "Upgrades") ||
				(key == "Fonts") || (key == "FontColors") || (key == "expansion")
				// other string to protected ?
				)) {
			lua_pop(l, 1); // pop the value
			continue;
		}
		switch (type_value) {
			case LUA_TNIL:
				value = "nil";
				break;
			case LUA_TNUMBER:
				value = lua_tostring(l, -1); // let lua do the conversion
				break;
			case LUA_TBOOLEAN:
				b = lua_toboolean(l, -1);
				value = b ? "true" : "false";
				break;
			case LUA_TSTRING:
				value = std::string("\"") + lua_tostring(l, -1) + "\"";
				break;
			case LUA_TTABLE:
				lua_pushvalue(l, -1);
				tmp = SaveGlobal(l, false);
				value = "";
				if (!tmp.empty()) {
					value = "{" + tmp + "}";
				}
				break;
			case LUA_TFUNCTION:
			// Could be done with string.dump(function)
			// and debug.getinfo(function).name (could be nil for anonymous function)
			// But not usefull yet.
				value = "";
				break;
			case LUA_TUSERDATA:
			case LUA_TTHREAD:
			case LUA_TLIGHTUSERDATA:
			case LUA_TNONE:
			default : // no other cases
				value = "";
				break;
		}
		lua_pop(l, 1); /* pop the value */

		// Check the validity of the key (only [a-zA-z_])
		if (type_key == LUA_TSTRING) {
			for (unsigned int i = 0; key[i]; ++i) {
				if (!isalnum(key[i]) && key[i] != '_') {
					value.clear();
					break;
				}
			}
		}
		if (value.empty()) {
			if (!is_root) {
				lua_pop(l, 2); // pop the key and the table
				return "";
			}
			continue;
		}
		if (type_key == LUA_TSTRING && key == value.c_str()) {
			continue;
		}
		if (first) {
			first = 0;
			if (type_key == LUA_TSTRING) {
				res = key + "=" + value;
			} else {
				res = value;
			}
		} else {
			if (type_key == LUA_TSTRING) {
				tmp = value;
				value = key + "=" + value;
				tmp = res;
				res = res + sep + value;
			} else {
				res = res + sep + value;
			}
		}
		tmp = res;
		res += "\n";
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
void SaveGame(const std::string &filename)
{
	time_t now;
	CFile file;
	std::string s;
	char *s1;
	std::string fullpath;

	fullpath = GetSaveDir() + filename;
	if (file.open(fullpath.c_str(), CL_WRITE_GZ | CL_OPEN_WRITE) == -1) {
		fprintf(stderr, "Can't save to `%s'\n", filename.c_str());
		return;
	}

	time(&now);
	s = ctime(&now);
	if ((s1 = strchr(s.c_str(), '\n'))) {
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
	file.printf("---  \"date\",    \"%s\",\n", s.c_str());
	file.printf("---  \"map\",     \"%s\",\n", Map.Info.Description.c_str());
	file.printf("---  \"media-version\", \"%s\"", "Undefined");
	file.printf("---  \"engine\",  {%d, %d, %d},\n",
		StratagusMajorVersion, StratagusMinorVersion, StratagusPatchLevel);
	file.printf("  SyncHash = %d, \n", SyncHash);
	file.printf("  SyncRandSeed = %d, \n", SyncRandSeed);
	file.printf("  SaveFile = \"%s\"\n", CurrentMapPath);
	file.printf("\n---  \"preview\", \"%s.pam\",\n", filename.c_str());
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
	SaveReplayList(&file);
	// FIXME: find all state information which must be saved.
	s = SaveGlobal(Lua, true);
	if (!s.empty()) {
		file.printf("-- Lua state\n\n %s\n", s.c_str());
	}
	file.close();
}

/**
**  Delete save game
**
**  @param filename  Name of file to delete
*/
void DeleteSaveGame(const std::string &filename)
{
	// Security check
	if (filename.find_first_of("/\\") != std::string::npos) {
		return;
	}

	std::string fullpath = GetSaveDir() + filename;
	if (unlink(fullpath.c_str()) == -1) {
		fprintf(stderr, "delete failed for %s", fullpath.c_str());
	}
}

//@}
