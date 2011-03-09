//     ____                _       __               
//    / __ )____  _____   | |     / /___ ___________
//   / __  / __ \/ ___/   | | /| / / __ `/ ___/ ___/
//  / /_/ / /_/ (__  )    | |/ |/ / /_/ / /  (__  ) 
// /_____/\____/____/     |__/|__/\__,_/_/  /____/  
//                                              
//       A futuristic real-time strategy game.
//          This file is part of Bos Wars.
//
/**@name script_player.cpp - The player ccl functions. */
//
//      (c) Copyright 2001-2007 by Lutz Sammer and Jimmy Salmon
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
#include "unittype.h"
#include "unit_cache.h"
#include "player.h"
#include "script.h"
#include "ai.h"
#include "actions.h"
#include "commands.h"
#include "trigger.h"

/*----------------------------------------------------------------------------
--  Variables
----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
--  Functions
----------------------------------------------------------------------------*/

/**
**  Parse the player configuration.
**
**  @param l  Lua state.
*/
static int CclPlayer(lua_State *l)
{
	const char *value;
	CPlayer *player;
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
	player->Index = i;
	memset(player->Units, 0, sizeof(player->Units));

	//
	//  Parse the list: (still everything could be changed!)
	//
	for (; j < args; ++j) {
		value = LuaToString(l, j + 1);
		++j;

		if (!strcmp(value, "name")) {
			player->SetName(LuaToString(l, j + 1));
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
				LuaError(l, "Unsupported tag: %s" _C_ value);
			}
		} else if (!strcmp(value, "ai-name")) {
			player->AiName = LuaToString(l, j + 1);
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
			LuaCheckTableSize(l, j + 1, 2);
			player->StartX = LuaToNumber(l, j + 1, 1);
			player->StartY = LuaToNumber(l, j + 1, 2);
		} else if (!strcmp(value, "production-rate")) {
			LuaCheckTable(l, j + 1);
			subargs = lua_objlen(l, j + 1);
			for (k = 0; k < subargs; ++k) {
				value = LuaToString(l, j + 1, k + 1);
				++k;

				for (i = 0; i < MaxCosts; ++i) {
					if (value == DefaultResourceNames[i]) {
						player->ProductionRate[i] = LuaToNumber(l, j + 1, k + 1);
						break;
					}
				}
				if (i == MaxCosts) {
					LuaError(l, "Unsupported tag: %s" _C_ value);
				}
			}
		} else if (!strcmp(value, "stored-resources")) {
			LuaCheckTable(l, j + 1);
			subargs = lua_objlen(l, j + 1);
			for (k = 0; k < subargs; ++k) {
				value = LuaToString(l, j + 1, k + 1);
				++k;

				for (i = 0; i < MaxCosts; ++i) {
					if (value == DefaultResourceNames[i]) {
						player->StoredResources[i] = LuaToNumber(l, j + 1, k + 1);
						break;
					}
				}
				if (i == MaxCosts) {
					LuaError(l, "Unsupported tag: %s" _C_ value);
				}
			}
		} else if (!strcmp(value, "storage-capacity")) {
			LuaCheckTable(l, j + 1);
			subargs = lua_objlen(l, j + 1);
			for (k = 0; k < subargs; ++k) {
				value = LuaToString(l, j + 1, k + 1);
				++k;

				for (i = 0; i < MaxCosts; ++i) {
					if (value == DefaultResourceNames[i]) {
						player->StorageCapacity[i] = LuaToNumber(l, j + 1, k + 1);
						break;
					}
				}
				if (i == MaxCosts) {
					LuaError(l, "Unsupported tag: %s" _C_ value);
				}
			}
		} else if (!strcmp(value, "ai-enabled")) {
			player->AiEnabled = 1;
			--j;
		} else if (!strcmp(value, "ai-disabled")) {
			player->AiEnabled = 0;
			--j;
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
			LuaCheckTable(l, j + 1);
			subargs = lua_objlen(l, j + 1);
			if (subargs != MaxCosts) {
				LuaError(l, "Wrong number of total-resources: %d" _C_ i);
			}
			for (k = 0; k < subargs; ++k) {
				player->TotalResources[k] = LuaToNumber(l, j + 1, k + 1);
			}
		} else if (!strcmp(value, "color")) {
			int r;
			int g;
			int b;

			LuaCheckTableSize(l, j + 1, 3);
			r = LuaToNumber(l, j + 1, 1);
			g = LuaToNumber(l, j + 1, 2);
			b = LuaToNumber(l, j + 1, 3);
			player->Color = Video.MapRGB(TheScreen->format, r, g, b);
		} else {
		   LuaError(l, "Unsupported tag: %s" _C_ value);
		}
	}

	return 0;
}

/**
**  Change unit owner
**
**  @param l  Lua state.
*/
void CclChangeUnitsOwner(
	const int topLeft[2],
	const int bottomRight[2],
	int oldPlayer,
	int newPlayer,
	lua_Object unitTypeLua,
	lua_State *l)
{
	const CUnitType *triggerUnitType;

	if (oldPlayer < 0 || oldPlayer >= PlayerMax) {
		LuaError(l, "Player number is out of range: %d" _C_ oldPlayer);
	}

	if (newPlayer < 0 || newPlayer >= PlayerMax) {
		LuaError(l, "Player number is out of range: %d" _C_ newPlayer);
	}

	// tolua++ 1.0.93 claims to define TOLUA_NIL, but doesn't,
	// so we use 0 instead.
	//
	// If the argument is nil, behave as if it had been omitted.
	// This feature is not documented in game.html because
	// script authors should use "any" rather than nil.
	if (unitTypeLua == 0 /* TOLUA_NIL */ || lua_isnil(l, unitTypeLua)) {
		triggerUnitType = ANY_UNIT;
	} else {
		lua_pushvalue(l, unitTypeLua);
		triggerUnitType = TriggerGetUnitType(l);
		lua_pop(l, 1);
	}
	
	// Okay, now Lua won't longjmp any more.

	CUnit *units[UnitMax];
	int n;
	const int x1 = topLeft[0];
	const int y1 = topLeft[1];
	const int x2 = bottomRight[0];
	const int y2 = bottomRight[1];

	n = UnitCache.Select(x1, y1, x2 + 1, y2 + 1, units, UnitMax);
	while (n) {
		CUnit *const unit = units[n - 1];
		if (unit->Player->Index == oldPlayer
		    && TriggerMatchUnitType(unit, triggerUnitType)) {
			unit->ChangeOwner(&Players[newPlayer]);
		}
		--n;
	}
}

/**
**  Set MaxSelectable
**
**  @param l  Lua state.
*/
static int CclSetMaxSelectable(lua_State *l)
{
	LuaCheckArgs(l, 1);
	MaxSelectable = LuaToNumber(l, 1);

	lua_pushnumber(l, MaxSelectable);
	return 1;
}

/**
**  Set player unit limit.
**
**  @param l  Lua state.
*/
static int CclSetAllPlayersUnitLimit(lua_State *l)
{
	int i;

	LuaCheckArgs(l, 1);
	for (i = 0; i < PlayerMax; ++i) {
		Players[i].UnitLimit = LuaToNumber(l, 1);
	}

	lua_pushnumber(l, lua_tonumber(l, 1));
	return 1;
}

/**
**  Set player unit limit.
**
**  @param l  Lua state.
*/
static int CclSetAllPlayersBuildingLimit(lua_State *l)
{
	int i;

	LuaCheckArgs(l, 1);
	for (i = 0; i < PlayerMax; ++i) {
		Players[i].BuildingLimit = LuaToNumber(l, 1);
	}

	lua_pushnumber(l, lua_tonumber(l, 1));
	return 1;
}

/**
**  Set player unit limit.
**
**  @param l  Lua state.
*/
static int CclSetAllPlayersTotalUnitLimit(lua_State *l)
{
	int i;

	LuaCheckArgs(l, 1);
	for (i = 0; i < PlayerMax; ++i) {
		Players[i].TotalUnitLimit = LuaToNumber(l, 1);
	}

	lua_pushnumber(l, lua_tonumber(l, 1));
	return 1;
}

/**
**  Change the diplomacy from player to another player.
**
**  @param l  Lua state.
*/
static int CclSetDiplomacy(lua_State *l)
{
	int plynr;
	int base;
	const char *state;

	LuaCheckArgs(l, 3);
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
**  Change the diplomacy from ThisPlayer to another player.
**
**  @param l  Lua state.
*/
static int CclDiplomacy(lua_State *l)
{
	lua_pushnumber(l, ThisPlayer->Index);
	lua_insert(l, 1);
	return CclSetDiplomacy(l);
}

/**
**  Change the shared vision from player to another player.
**
**  @param l  Lua state.
*/
static int CclSetSharedVision(lua_State *l)
{
	int plynr;
	int base;
	bool shared;

	LuaCheckArgs(l, 3);

	base = LuaToNumber(l, 1);
	shared = LuaToBoolean(l, 2);
	plynr = LuaToNumber(l, 3);

	SendCommandSharedVision(base, shared, plynr);

	return 0;
}

/**
**  Change the shared vision from ThisPlayer to another player.
**
**  @param l  Lua state.
*/
static int CclSharedVision(lua_State *l)
{
	lua_pushnumber(l, ThisPlayer->Index);
	lua_insert(l, 1);
	return CclSetSharedVision(l);
}

/**
**  Define player colors
**
**  @param l  Lua state.
*/
static int CclDefinePlayerColors(lua_State *l)
{
	int i;
	int args;
	int j;
	int numcolors;

	LuaCheckArgs(l, 1);
	LuaCheckTable(l, 1);

	args = lua_objlen(l, 1);
	for (i = 0; i < args; ++i) {
		PlayerColorNames[i / 2] = LuaToString(l, 1, i + 1);
		++i;
		lua_rawgeti(l, 1, i + 1);
		LuaCheckTable(l, -1);
		numcolors = lua_objlen(l, -1);
		if (numcolors != PlayerColorIndexCount) {
			LuaError(l, "You should use %d colors (See DefinePlayerColorIndex())" _C_ PlayerColorIndexCount);
		}
		for (j = 0; j < numcolors; ++j) {
			lua_rawgeti(l, -1, j + 1);
			LuaCheckTableSize(l, -1, 3);
			PlayerColorsRGB[i / 2][j].r = LuaToNumber(l, -1, 1);
			PlayerColorsRGB[i / 2][j].g = LuaToNumber(l, -1, 2);
			PlayerColorsRGB[i / 2][j].b = LuaToNumber(l, -1, 3);
			lua_pop(l, 1);
		}
	}

	return 0;
}

/**
**  Make new player colors
**
**  @param l  Lua state.
*/
static int CclNewPlayerColors(lua_State *l)
{
	LuaCheckArgs(l, 0);
	SetPlayersPalette();

	return 0;
}

/**
**  Define player color indexes
**
**  @param l  Lua state.
*/
static int CclDefinePlayerColorIndex(lua_State *l)
{
	int i;

	LuaCheckArgs(l, 2);
	PlayerColorIndexStart = LuaToNumber(l, 1);
	PlayerColorIndexCount = LuaToNumber(l, 2);

	for (i = 0; i < PlayerMax; ++i) {
		delete[] PlayerColorsRGB[i];
		PlayerColorsRGB[i] = new SDL_Color[PlayerColorIndexCount];
		memset(PlayerColorsRGB[i], 0, PlayerColorIndexCount * sizeof(SDL_Color));
		delete[] PlayerColors[i];
		PlayerColors[i] = new Uint32[PlayerColorIndexCount];
		memset(PlayerColorsRGB[i], 0, PlayerColorIndexCount * sizeof(Uint32));
	}
	return 0;
}

// ----------------------------------------------------------------------------

/**
**  Register CCL features for players.
*/
void PlayerCclRegister(void)
{
	lua_register(Lua, "Player", CclPlayer);

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

	lua_register(Lua, "DefinePlayerColors", CclDefinePlayerColors);
	lua_register(Lua, "DefinePlayerColorIndex", CclDefinePlayerColorIndex);

	lua_register(Lua, "NewColors", CclNewPlayerColors);
}

//@}
