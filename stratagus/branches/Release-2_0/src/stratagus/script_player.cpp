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
/**@name ccl_player.c - The player ccl functions. */
//
//      (c) Copyright 2001-2004 by Lutz Sammer and Jimmy Salmon
//
//      This program is free software; you can redistribute it and/or modify
//      it under the terms of the GNU General Public License as published by
//      the Free Software Foundation; version 2 dated June, 1991.
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
--		Includes
----------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "stratagus.h"
#include "player.h"
#include "script.h"
#include "ai.h"
#include "actions.h"
#include "commands.h"

/*----------------------------------------------------------------------------
--		Variables
----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
--		Functions
----------------------------------------------------------------------------*/

/**
**		Get a player pointer
**
**		@param value		Player slot number.
**
**		@return				The player pointer
*/
local Player* CclGetPlayer(lua_State* l)
{
	return &Players[(int)LuaToNumber(l, -1)];
}

/**
**		Parse the player configuration.
**
**		@param list		Tagged list of all informations.
*/
local int CclPlayer(lua_State* l)
{
	const char* value;
	Player* player;
	int i;
	int args;
	int j;
	int subargs;
	int k;

	args = lua_gettop(l);
	j = 0;

	i = LuaToNumber(l, j + 1);
	++j;
	player = &Players[i];
	if (NumPlayers <= i) {
		NumPlayers = i + 1;
	}
	player->Player = i;
	if (!(player->Units = (Unit**)calloc(UnitMax, sizeof(Unit*)))) {
		DebugLevel0("Not enough memory to create player %d.\n" _C_ i);
		return 0;
	}

	//
	//		Parse the list:		(still everything could be changed!)
	//
	for (; j < args; ++j) {
		value = LuaToString(l, j + 1);
		++j;

		if (!strcmp(value, "name")) {
			player->Name = strdup(LuaToString(l, j + 1));
		} else if (!strcmp(value, "type")) {
			value = LuaToString(l, j + 1);
			if (!strcmp(value, "neutral")) {
				player->Type = PlayerNeutral;
			} else if (!strcmp(value, "nobody")) {
				player->Type = PlayerNobody;
			} else if (!strcmp(value, "computer")) {
				player->Type = PlayerComputer;
			} else if (!strcmp(value, "person")) {
				player->Type = PlayerPerson;
			} else if (!strcmp(value, "rescue-passive")) {
				player->Type = PlayerRescuePassive;
			} else if (!strcmp(value, "rescue-active")) {
				player->Type = PlayerRescueActive;
			} else {
			   lua_pushfstring(l, "Unsupported tag: %s", value);
			   lua_error(l);
			}
		} else if (!strcmp(value, "race")) {
			value = LuaToString(l, j + 1);
			for (i = 0; i < PlayerRaces.Count; ++i) {
				if (!strcmp(value, PlayerRaces.Name[i])) {
					player->RaceName = PlayerRaces.Name[i];
					player->Race = i;
					break;
				}
			}
			if (i == PlayerRaces.Count) {
				lua_pushfstring(l, "Unsupported race: %s", value);
				lua_error(l);
			}
		} else if (!strcmp(value, "ai")) {
			player->AiNum = LuaToNumber(l, j + 1);
		} else if (!strcmp(value, "team")) {
			player->Team = LuaToNumber(l, j + 1);
		} else if (!strcmp(value, "enemy")) {
			value = LuaToString(l, j + 1);
			for (i = 0; i < PlayerMax && *value; ++i, ++value) {
				if (*value == '-' || *value == '_' || *value == ' ') {
					player->Enemy &= ~(1 << i);
				} else {
					player->Enemy |= (1 << i);
				}
			}
		} else if (!strcmp(value, "allied")) {
			value = LuaToString(l, j + 1);
			for (i = 0; i < PlayerMax && *value; ++i, ++value) {
				if (*value == '-' || *value == '_' || *value == ' ') {
					player->Allied &= ~(1 << i);
				} else {
					player->Allied |= (1 << i);
				}
			}
		} else if (!strcmp(value, "shared-vision")) {
			value = LuaToString(l, j + 1);
			for (i = 0; i < PlayerMax && *value; ++i, ++value) {
				if (*value == '-' || *value == '_' || *value == ' ') {
					player->SharedVision &= ~(1 << i);
				} else {
					player->SharedVision |= (1 << i);
				}
			}
		} else if (!strcmp(value, "start")) {
			if (!lua_istable(l, j + 1) || luaL_getn(l, j + 1) != 2) {
				lua_pushstring(l, "incorrect argument");
				lua_error(l);
			}
			lua_rawgeti(l, j + 1, 1);
			player->StartX = LuaToNumber(l, -1);
			lua_pop(l, 1);
			lua_rawgeti(l, j + 1, 2);
			player->StartY = LuaToNumber(l, -1);
			lua_pop(l, 1);
		} else if (!strcmp(value, "resources")) {
			if (!lua_istable(l, j + 1)) {
				lua_pushstring(l, "incorrect argument");
				lua_error(l);
			}
			subargs = luaL_getn(l, j + 1);
			for (k = 0; k < subargs; ++k) {
				lua_rawgeti(l, j + 1, k + 1);
				value = LuaToString(l, -1);
				lua_pop(l, 1);
				++k;

				for (i = 0; i < MaxCosts; ++i) {
					if (!strcmp(value, DefaultResourceNames[i])) {
						lua_rawgeti(l, j + 1, k + 1);
						player->Resources[i] = LuaToNumber(l, -1);
						lua_pop(l, 1);
						break;
					}
				}
				if (i == MaxCosts) {
					lua_pushfstring(l, "Unsupported tag: %s", value);
					lua_error(l);
				}
			}
		} else if (!strcmp(value, "last-resources")) {
			if (!lua_istable(l, j + 1)) {
				lua_pushstring(l, "incorrect argument");
				lua_error(l);
			}
			subargs = luaL_getn(l, j + 1);
			for (k = 0; k < subargs; ++k) {
				lua_rawgeti(l, j + 1, k + 1);
				value = LuaToString(l, -1);
				lua_pop(l, 1);
				++k;

				for (i = 0; i < MaxCosts; ++i) {
					if (!strcmp(value, DefaultResourceNames[i])) {
						lua_rawgeti(l, j + 1, k + 1);
						player->LastResources[i] = LuaToNumber(l, -1);
						lua_pop(l, 1);
						break;
					}
				}
				if (i == MaxCosts) {
					lua_pushfstring(l, "Unsupported tag: %s", value);
					lua_error(l);
				}
			}
		} else if (!strcmp(value, "incomes")) {
			if (!lua_istable(l, j + 1)) {
				lua_pushstring(l, "incorrect argument");
				lua_error(l);
			}
			subargs = luaL_getn(l, j + 1);
			for (k = 0; k < subargs; ++k) {
				lua_rawgeti(l, j + 1, k + 1);
				value = LuaToString(l, -1);
				lua_pop(l, 1);
				++k;

				for (i = 0; i < MaxCosts; ++i) {
					if (!strcmp(value, DefaultResourceNames[i])) {
						lua_rawgeti(l, j + 1, k + 1);
						player->Incomes[i] = LuaToNumber(l, -1);
						lua_pop(l, 1);
						break;
					}
				}
				if (i == MaxCosts) {
					lua_pushfstring(l, "Unsupported tag: %s", value);
					lua_error(l);
				}
			}
		} else if (!strcmp(value, "revenue")) {
			if (!lua_istable(l, j + 1)) {
				lua_pushstring(l, "incorrect argument");
				lua_error(l);
			}
			subargs = luaL_getn(l, j + 1);
			for (k = 0; k < subargs; ++k) {
				lua_rawgeti(l, j + 1, k + 1);
				value = LuaToString(l, -1);
				lua_pop(l, 1);
				++k;

				for (i = 0; i < MaxCosts; ++i) {
					if (!strcmp(value, DefaultResourceNames[i])) {
						lua_rawgeti(l, j + 1, k + 1);
						player->Revenue[i] = LuaToNumber(l, -1);
						lua_pop(l, 1);
						break;
					}
				}
				if (i == MaxCosts) {
					lua_pushfstring(l, "Unsupported tag: %s", value);
					lua_error(l);
				}
			}
		} else if (!strcmp(value, "ai-enabled")) {
			player->AiEnabled = 1;
			--j;
		} else if (!strcmp(value, "ai-disabled")) {
			player->AiEnabled = 0;
			--j;
		} else if (!strcmp(value, "supply")) {
			player->Supply = LuaToNumber(l, j + 1);
		} else if (!strcmp(value, "demand")) {
			player->Demand = LuaToNumber(l, j + 1);
		} else if (!strcmp(value, "unit-limit")) {
			player->UnitLimit = LuaToNumber(l, j + 1);
		} else if (!strcmp(value, "building-limit")) {
			player->BuildingLimit = LuaToNumber(l, j + 1);
		} else if (!strcmp(value, "total-unit-limit")) {
			player->TotalUnitLimit = LuaToNumber(l, j + 1);
		} else if (!strcmp(value, "score")) {
			player->Score = LuaToNumber(l, j + 1);
		} else if (!strcmp(value, "total-units")) {
			player->TotalUnits = LuaToNumber(l, j + 1);
		} else if (!strcmp(value, "total-buildings")) {
			player->TotalBuildings = LuaToNumber(l, j + 1);
		} else if (!strcmp(value, "total-razings")) {
			player->TotalRazings = LuaToNumber(l, j + 1);
		} else if (!strcmp(value, "total-kills")) {
			player->TotalKills = LuaToNumber(l, j + 1);
		} else if (!strcmp(value, "total-resources")) {
			if (!lua_istable(l, j + 1)) {
				lua_pushstring(l, "incorrect argument");
				lua_error(l);
			}
			subargs = luaL_getn(l, j + 1);
			if (subargs != MaxCosts) {
				lua_pushfstring(l, "Wrong number of total-resources: %d", i);
				lua_error(l);
			}
			for (k = 0; k < subargs; ++k) {
				lua_rawgeti(l, j + 1, k + 1);
				player->TotalResources[k] = LuaToNumber(l, -1);
				lua_pop(l, 1);
			}
		} else if (!strcmp(value, "color")) {
			int r;
			int g;
			int b;

			if (!lua_istable(l, j + 1) || luaL_getn(l, j + 1) != 3) {
				lua_pushstring(l, "incorrect argument");
				lua_error(l);
			}
			lua_rawgeti(l, j + 1, 1);
			r = LuaToNumber(l, -1);
			lua_pop(l, 1);
			lua_rawgeti(l, j + 1, 2);
			g = LuaToNumber(l, -1);
			lua_pop(l, 1);
			lua_rawgeti(l, j + 1, 3);
			b = LuaToNumber(l, -1);
			lua_pop(l, 1);
			player->Color = SDL_MapRGB(TheScreen->format, r, g, b);
		} else if (!strcmp(value, "timers")) {
			if (!lua_istable(l, j + 1)) {
				lua_pushstring(l, "incorrect argument");
				lua_error(l);
			}
			subargs = luaL_getn(l, j + 1);
			if (subargs != UpgradeMax) {
				lua_pushfstring(l, "Wrong upgrade timer length: %d", i);
				lua_error(l);
			}
			for (k = 0; k < subargs; ++k) {
				lua_rawgeti(l, j + 1, k + 1);
				player->UpgradeTimers.Upgrades[k] = LuaToNumber(l, -1);
				lua_pop(l, 1);
			}
		} else {
		   lua_pushfstring(l, "Unsupported tag: %s", value);
		   lua_error(l);
		}
	}

	return 0;
}

/**
**		Change unit owner
**
**		@param pos1		 top left tile
**		@param pos2		 bottom right tile
**		@param oldplayer old player number
**		@param newplayer new player number
**/
local int CclChangeUnitsOwner(lua_State* l)
{
	Unit* table[UnitMax];
	int n;
	int oldp;
	int newp;
	int x1;
	int y1;
	int x2;
	int y2;

	if (lua_gettop(l) != 4 || !lua_istable(l, 1) || !lua_istable(l, 2)) {
		lua_pushstring(l, "incorrect argument");
		lua_error(l);
	}
	if (luaL_getn(l, 1) != 2) {
		lua_pushstring(l, "incorrect argument");
		lua_error(l);
	}
	lua_rawgeti(l, 1, 1);
	x1 = LuaToNumber(l, -1);
	lua_pop(l, 1);
	lua_rawgeti(l, 1, 1);
	y1 = LuaToNumber(l, -1);
	lua_pop(l, 1);

	if (luaL_getn(l, 2) != 2) {
		lua_pushstring(l, "incorrect argument");
		lua_error(l);
	}
	lua_rawgeti(l, 2, 1);
	x2 = LuaToNumber(l, -1);
	lua_pop(l, 1);
	lua_rawgeti(l, 2, 1);
	y2 = LuaToNumber(l, -1);
	lua_pop(l, 1);

	n = UnitCacheSelect(x1, y1, x2, y2, table);
	oldp = LuaToNumber(l, 3);
	newp = LuaToNumber(l, 4);
	while (n) {
		if (table[n - 1]->Player->Player == oldp) {
			ChangeUnitOwner(table[n - 1], &Players[newp]);
		}
		--n;
	}

	return 0;
}

/**
**		Get ThisPlayer.
**
**		@return				This player number.
*/
local int CclGetThisPlayer(lua_State* l)
{
	if (lua_gettop(l) != 0) {
		lua_pushstring(l, "incorrect argument");
		lua_error(l);
	}
	lua_pushnumber(l, ThisPlayer - Players);
	return 1;
}

/**
**		Set ThisPlayer.
**
**		@param plynr		This player number.
*/
local int CclSetThisPlayer(lua_State* l)
{
	int plynr;

	if (lua_gettop(l) != 1) {
		lua_pushstring(l, "incorrect argument");
		lua_error(l);
	}
	plynr = LuaToNumber(l, 1);
	ThisPlayer = &Players[plynr];

	lua_pushnumber(l, plynr);
	return 1;
}

/**
**		Set MaxSelectable
**
**		@param				Max number of selectable units.
*/
local int CclSetMaxSelectable(lua_State* l)
{
	if (lua_gettop(l) != 1) {
		lua_pushstring(l, "incorrect argument");
		lua_error(l);
	}
	MaxSelectable = LuaToNumber(l, 1);

	lua_pushnumber(l, MaxSelectable);
	return 1;
}

/**
**		Set player unit limit.
**
**		@param limit		Unit limit.
*/
local int CclSetAllPlayersUnitLimit(lua_State* l)
{
	int i;

	if (lua_gettop(l) != 1) {
		lua_pushstring(l, "incorrect argument");
		lua_error(l);
	}
	for (i = 0; i < PlayerMax; ++i) {
		Players[i].UnitLimit = LuaToNumber(l, 1);
	}

	lua_pushnumber(l, lua_tonumber(l, 1));
	return 1;
}

/**
**		Set player unit limit.
**
**		@param limit		Unit limit.
*/
local int CclSetAllPlayersBuildingLimit(lua_State* l)
{
	int i;

	if (lua_gettop(l) != 1) {
		lua_pushstring(l, "incorrect argument");
		lua_error(l);
	}
	for (i = 0; i < PlayerMax; ++i) {
		Players[i].BuildingLimit = LuaToNumber(l, 1);
	}

	lua_pushnumber(l, lua_tonumber(l, 1));
	return 1;
}

/**
**		Set player unit limit.
**
**		@param limit		Unit limit.
*/
local int CclSetAllPlayersTotalUnitLimit(lua_State* l)
{
	int i;

	if (lua_gettop(l) != 1) {
		lua_pushstring(l, "incorrect argument");
		lua_error(l);
	}
	for (i = 0; i < PlayerMax; ++i) {
		Players[i].TotalUnitLimit = LuaToNumber(l, 1);
	}

	lua_pushnumber(l, lua_tonumber(l, 1));
	return 1;
}

/**
**		Change the diplomacy from player to another player.
**
**		@param player		Player to change diplomacy.
**		@param opponent		Player number to change.
**		@param state		To which state this should be changed.
**
**		@return				FIXME: should return old state.
**
**		@todo FIXME: should return old state.
*/
local int CclSetDiplomacy(lua_State* l)
{
	int plynr;
	int base;
	const char* state;

	if (lua_gettop(l) != 3) {
		lua_pushstring(l, "incorrect argument");
		lua_error(l);
	}
	base = LuaToNumber(l, 1);
	plynr = LuaToNumber(l, 3);
	state = LuaToString(l, 2);

	if (!strcmp(state, "allied")) {
		SendCommandDiplomacy(base, DiplomacyAllied, plynr);
	} else if (!strcmp(state, "neutral")) {
		SendCommandDiplomacy(base, DiplomacyNeutral, plynr);
	} else if (!strcmp(state, "crazy")) {
		SendCommandDiplomacy(base, DiplomacyCrazy, plynr);
	} else if (!strcmp(state, "enemy")) {
		SendCommandDiplomacy(base, DiplomacyEnemy, plynr);
	}

	return 0;
}

/**
**		Change the diplomacy from ThisPlayer to another player.
**
**		@param state		To which state this should be changed.
**		@param player		Player number to change.
*/
local int CclDiplomacy(lua_State* l)
{
	lua_pushnumber(l, ThisPlayer->Player);
	lua_insert(l, 1);
	return CclSetDiplomacy(l);
}

/**
**		Change the shared vision from player to another player.
**
**		@param player		Player to change shared vision.
**		@param opponent		Player number to change.
**		@param state		To which state this should be changed.
**
**		@return				FIXME: should return old state.
**
**		@todo FIXME: should return old state.
*/
local int CclSetSharedVision(lua_State* l)
{
	int plynr;
	int base;
	int shared;

	if (lua_gettop(l) != 3) {
		lua_pushstring(l, "incorrect argument");
		lua_error(l);
	}

	base = LuaToNumber(l, 1);
	shared = LuaToBoolean(l, 2);
	plynr = LuaToNumber(l, 3);

	SendCommandSharedVision(base, shared, plynr);

	return 0;
}

/**
**		Change the shared vision from ThisPlayer to another player.
**
**		@param state		To which state this should be changed.
**		@param player		Player number to change.
*/
local int CclSharedVision(lua_State* l)
{
	lua_pushnumber(l, ThisPlayer->Player);
	lua_insert(l, 1);
	return CclSetSharedVision(l);
}

/**
**		Define race names
**
**		@param list		List of all races.
*/
local int CclDefineRaceNames(lua_State* l)
{
	int i;
	int j;
	int k;
	int args;
	int subargs;
	const char* value;

	PlayerRaces.Count = 0;
	args = lua_gettop(l);
	for (j = 0; j < args; ++j) {
		value = LuaToString(l, j + 1);
		if (!strcmp(value, "race")) {
			++j;
			if (!lua_istable(l, j + 1)) {
				lua_pushstring(l, "incorrect argument");
				lua_error(l);
			}
			subargs = luaL_getn(l, j + 1);
			i = PlayerRaces.Count++;
			PlayerRaces.Race[i] = 0;
			PlayerRaces.Name[i] = NULL;
			PlayerRaces.Display[i] = NULL;
			PlayerRaces.Visible[i] = 0;
			for (k = 0; k < subargs; ++k) {
				lua_rawgeti(l, j + 1, k + 1);
				value = LuaToString(l, -1);
				lua_pop(l, 1);
				if (!strcmp(value, "race")) {
					++k;
					lua_rawgeti(l, j + 1, k + 1);
					PlayerRaces.Race[i] = LuaToNumber(l, -1);
					lua_pop(l, 1);
				} else if (!strcmp(value, "name")) {
					++k;
					lua_rawgeti(l, j + 1, k + 1);
					PlayerRaces.Name[i] = strdup(LuaToString(l, -1));
					lua_pop(l, 1);
				} else if (!strcmp(value, "display")) {
					++k;
					lua_rawgeti(l, j + 1, k + 1);
					PlayerRaces.Display[i] = strdup(LuaToString(l, -1));
					lua_pop(l, 1);
				} else if (!strcmp(value, "visible")) {
					PlayerRaces.Visible[i] = 1;
				} else {
					lua_pushfstring(l, "Unsupported tag: %s", value);
					lua_error(l);
				}
			}
		} else {
			lua_pushfstring(l, "Unsupported tag: %s", value);
			lua_error(l);
		}
	}

	return 0;
}

/**
**		Make new player colors
*/
local int CclNewPlayerColors(lua_State* l)
{
	if (lua_gettop(l) != 0) {
		lua_pushstring(l, "incorrect argument");
		lua_error(l);
	}
	SetPlayersPalette();

	return 0;
}

// ----------------------------------------------------------------------------

/**
**		Get player resources.
**
**		@param player		Player
**		@param resource		Resource name
**
**		@return				Player resource
*/
local int CclGetPlayerResource(lua_State* l)
{
	int i;
	Player* plyr;
	const char* res;

	if (lua_gettop(l) != 2) {
		lua_pushstring(l, "incorrect argument");
		lua_error(l);
	}
	lua_pushvalue(l, 1);
	plyr = CclGetPlayer(l);
	lua_pop(l, 1);
	res = LuaToString(l, 2);

	for (i = 0; i < MaxCosts; ++i) {
		if (!strcmp(res, DefaultResourceNames[i])) {
			break;
		}
	}
	if (i == MaxCosts) {
		// FIXME: this leaves a half initialized player
		lua_pushfstring(l, "Invalid resource \"%s\"", res);
		lua_error(l);
	}
	lua_pushnumber(l, plyr->Resources[i]);
	return 1;
}

/**
**		Set player resource.
**
**		@param list		Resource list
*/
local int CclSetPlayerResource(lua_State* l)
{
	int i;
	Player* player;
	const char* value;
	int args;
	int j;

	args = lua_gettop(l);
	lua_pushvalue(l, 1);
	player = CclGetPlayer(l);
	lua_pop(l, 1);
	for (j = 1; j < args; ++j) {
		value = LuaToString(l, j + 1);
		++j;
		for (i = 0; i < MaxCosts; ++i) {
			if (!strcmp(value, DefaultResourceNames[i])) {
				break;
			}
		}
		if (i == MaxCosts) {
		   lua_pushfstring(l, "Unsupported tag: %s", value);
		   lua_error(l);
		}
		player->Resources[i] = LuaToNumber(l, j + 1);
	}
	MustRedraw |= RedrawResources;
	return 0;
}

// ----------------------------------------------------------------------------

/**
**		Register CCL features for players.
*/
global void PlayerCclRegister(void)
{
	lua_register(Lua, "Player", CclPlayer);
	lua_register(Lua, "ChangeUnitsOwner", CclChangeUnitsOwner);
	lua_register(Lua, "GetThisPlayer", CclGetThisPlayer);
	lua_register(Lua, "SetThisPlayer", CclSetThisPlayer);

	lua_register(Lua, "SetMaxSelectable", CclSetMaxSelectable);

	lua_register(Lua, "SetAllPlayersUnitLimit",
		CclSetAllPlayersUnitLimit);
	lua_register(Lua, "SetAllPlayersBuildingLimit",
		CclSetAllPlayersBuildingLimit);
	lua_register(Lua, "SetAllPlayersTotalUnitLimit",
		CclSetAllPlayersTotalUnitLimit);

	lua_register(Lua, "SetDiplomacy", CclSetDiplomacy);
	lua_register(Lua, "Diplomacy", CclDiplomacy);
	lua_register(Lua, "SetSharedVision", CclSetSharedVision);
	lua_register(Lua, "SharedVision", CclSharedVision);

	lua_register(Lua, "DefineRaceNames", CclDefineRaceNames);

	lua_register(Lua, "NewColors", CclNewPlayerColors);

	// player member access functions
	lua_register(Lua, "GetPlayerResource", CclGetPlayerResource);
	lua_register(Lua, "SetPlayerResource", CclSetPlayerResource);
}

#ifdef META_LUA

	/// Proxy type for the Player array
local ScriptProxyType ScriptProxyPlayerArray;
	/// Proxy type for Player
local ScriptProxyType ScriptProxyPlayer;
	/// Proxy type for Player->Allied
local ScriptProxyType ScriptProxyPlayerAllied;
	/// Proxy type for Player->Enemy
local ScriptProxyType ScriptProxyPlayerEnemy;
	/// Proxy type for Player->SharedVision
local ScriptProxyType ScriptProxyPlayerSharedVision;
	/// Proxy type for Player->UnitTypesCount
local ScriptProxyType ScriptProxyPlayerUnitTypesCount;

/**
**	Get function for the big Players namespace, with int index
*/
local int ScriptPlayerArrayGetInt(void* object, int index, lua_State* l)
{
	if (index < 0 || index >= PlayerMax) {
		LuaError(l, "Player index out of range");
	}
	ScriptCreateUserdata(l, Players + index, &ScriptProxyPlayer);
	return 1;
}

/**
**	Get function for a player userdata.
*/
local int ScriptPlayerGet(Player* player, const char* key, lua_State* l)
{
	META_GET_INT("Slot", player->Player);
	META_GET_STRING("Name", player->Name);

	META_GET_INT("TotalNumUnits", player->TotalNumUnits);
	META_GET_INT("NumBuildings", player->NumBuildings);
	META_GET_INT("UnitLimit", player->UnitLimit);
	META_GET_INT("BuildingLimit", player->BuildingLimit);
	META_GET_INT("TotalUnitLimit", player->TotalUnitLimit);
	META_GET_INT("Supply", player->Supply);
	META_GET_INT("Demand", player->Demand);

	META_GET_INT("StartX", player->StartX);
	META_GET_INT("StartY", player->StartY);

	META_GET_INT("Score", player->Score);
	META_GET_INT("TotalUnits", player->TotalUnits);
	META_GET_INT("TotalBuildings", player->TotalBuildings);
	META_GET_INT("TotalRazings", player->TotalRazings);
	META_GET_INT("TotalKills", player->TotalKills);

	META_GET_USERDATA("Allied", player, &ScriptProxyPlayerAllied);
	META_GET_USERDATA("Enemy", player, &ScriptProxyPlayerEnemy);
	META_GET_USERDATA("SharedVision", player, &ScriptProxyPlayerSharedVision);
	META_GET_USERDATA("UnitTypesCount", player, &ScriptProxyPlayerUnitTypesCount);
	META_GET_USERDATA("Research", player, &ScriptProxyPlayer);

	LuaError(l, "Field \"%s\" is innexistent or write-only (yes, we have those).\n" _C_ key);
}

/**
**	Set function for a player userdata.
*/
local int ScriptPlayerSet(Player* player, const char* key, lua_State* l)
{
	META_SET_STRING("Name", player->Name);

	META_SET_INT("TotalNumUnits", player->TotalNumUnits);
	META_SET_INT("NumBuildings", player->NumBuildings);
	META_SET_INT("UnitLimit", player->UnitLimit);
	META_SET_INT("BuildingLimit", player->BuildingLimit);
	META_SET_INT("TotalUnitLimit", player->TotalUnitLimit);
	META_SET_INT("Supply", player->Supply);
	META_SET_INT("Demand", player->Demand);

	META_SET_INT("StartX", player->StartX);
	META_SET_INT("StartY", player->StartY);

	META_SET_INT("Score", player->Score);
	META_SET_INT("TotalUnits", player->TotalUnits);
	META_SET_INT("TotalBuildings", player->TotalBuildings);
	META_SET_INT("TotalRazings", player->TotalRazings);
	META_SET_INT("TotalKills", player->TotalKills);

	LuaError(l, "Field \"%s\" is innexistent or read-only.\n" _C_ key);
}

/**
**	Get function for Player->Allied
*/
local int ScriptPlayerAlliedGet(Player* player, int index, lua_State* l)
{
	lua_pushboolean(l, player->Allied & (1 << index));
	return 1;
}

/**
**	Set function for Player->Allied
*/
local int ScriptPlayerAlliedSet(Player* player, int index, lua_State* l)
{
	if (LuaToBoolean(l, -1)) {
		player->Allied |= (1 << index);
	} else {
		player->Allied &= ~(1 << index);
	}
	return 1;
}

/**
**	Get function for Player->Enemy
*/
local int ScriptPlayerEnemyGet(Player* player, int index, lua_State* l)
{
	lua_pushboolean(l, player->Enemy & (1 << index));
	return 1;
}

/**
**	Set function for Player->Enemy
*/
local int ScriptPlayerEnemySet(Player* player, int index, lua_State* l)
{
	if (LuaToBoolean(l, -1)) {
		player->Enemy |= (1 << index);
	} else {
		player->Enemy &= ~(1 << index);
	}
	return 1;
}

/**
**	Get function for Player->SharedVision
*/
local int ScriptPlayerSharedVisionGet(Player* player, int index, lua_State* l)
{
	lua_pushboolean(l, player->SharedVision & (1 << index));
	return 1;
}

/**
**	Set function for Player->SharedVision
*/
local int ScriptPlayerSharedVisionSet(Player* player, int index, lua_State* l)
{
	CommandSharedVision(player->Player, LuaToBoolean(l, -1), index);
	return 1;
}

/**
**	Get function for Player->UnitTypesCount with string key
*/
local int ScriptPlayerUnitTypesCountGetStr(Player* player, const char* key, lua_State* l)
{
	UnitType* type;

	if ((type = UnitTypeByIdent(key))) {
		lua_pushnumber(l, player->UnitTypesCount[type->Slot]);
		return 1;
	}
	LuaError(l, "Unit \"%s\" not found." _C_ key);
}

/**
**	Get function for Player->UnitTypesCount with int index
*/
local int ScriptPlayerUnitTypesCountGetInt(Player* player, int index, lua_State* l)
{
	if (index < 0 || index >= NumUnitTypes) {
		LuaError(l, "Unittype index out of range.");
	}

	lua_pushnumber(l, player->UnitTypesCount[index]);
	return 1;
}

/**
**	Initialize player scripting. The main table is at -1
*/
global void ScriptPlayerInit(void)
{
	ScriptProxyTypeInitBlock(&ScriptProxyPlayerArray);
	ScriptProxyPlayerArray.GetInt = (ScriptGetSetIntFunction *)ScriptPlayerArrayGetInt;

	ScriptProxyTypeInitBlock(&ScriptProxyPlayer);
	ScriptProxyPlayer.GetStr = (ScriptGetSetStrFunction *)ScriptPlayerGet;
	ScriptProxyPlayer.SetStr = (ScriptGetSetStrFunction *)ScriptPlayerSet;

	ScriptProxyTypeInitBlock(&ScriptProxyPlayerAllied);
	ScriptProxyPlayerAllied.GetInt = (ScriptGetSetIntFunction *)ScriptPlayerAlliedGet;
	ScriptProxyPlayerAllied.SetInt = (ScriptGetSetIntFunction *)ScriptPlayerAlliedSet;

	ScriptProxyTypeInitBlock(&ScriptProxyPlayerEnemy);
	ScriptProxyPlayerEnemy.GetInt = (ScriptGetSetIntFunction *)ScriptPlayerEnemyGet;
	ScriptProxyPlayerEnemy.SetInt = (ScriptGetSetIntFunction *)ScriptPlayerEnemySet;

	ScriptProxyTypeInitBlock(&ScriptProxyPlayerSharedVision);
	ScriptProxyPlayerSharedVision.GetInt = (ScriptGetSetIntFunction *)ScriptPlayerSharedVisionGet;
	ScriptProxyPlayerSharedVision.SetInt = (ScriptGetSetIntFunction *)ScriptPlayerSharedVisionSet;

	ScriptProxyTypeInitBlock(&ScriptProxyPlayerUnitTypesCount);
	ScriptProxyPlayerUnitTypesCount.GetStr = (ScriptGetSetStrFunction *)ScriptPlayerUnitTypesCountGetStr;
	ScriptProxyPlayerUnitTypesCount.GetInt = (ScriptGetSetIntFunction *)ScriptPlayerUnitTypesCountGetInt;

	// Create Stratagus.Players namespace.
	lua_pushstring(Lua, "Players");
	ScriptCreateUserdata(Lua, 0, &ScriptProxyPlayerArray);
	lua_rawset(Lua, -3);
}

#endif

//@}
