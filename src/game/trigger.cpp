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
/**@name trigger.c - The trigger handling. */
//
//      (c) Copyright 2002-2004 by Lutz Sammer and Jimmy Salmon
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <setjmp.h>

#include "stratagus.h"
#include "script.h"
#include "unittype.h"
#include "player.h"
#include "trigger.h"
#include "campaign.h"
#include "interface.h"

/*----------------------------------------------------------------------------
--  Declarations
----------------------------------------------------------------------------*/

#define MAX_SWITCH 256 /// Maximum number of switches

/*----------------------------------------------------------------------------
--  Variables
----------------------------------------------------------------------------*/

global Timer GameTimer; /// The game timer
local unsigned long WaitFrame; /// Frame to wait for
local int Trigger;
local int WaitTrigger;
local unsigned char Switch[MAX_SWITCH]; /// Switches
local int* ActiveTriggers;

/*----------------------------------------------------------------------------
--  Functions
----------------------------------------------------------------------------*/

/**
**  Get player number.
**
**  @param l  Lua state.
**
**  @return   The player number, -1 matches any.
*/
global int TriggerGetPlayer(lua_State* l)
{
	const char* player;
	int ret;

	if (lua_isnumber(l, -1)) {
		ret = LuaToNumber(l, -1);
		if (ret < 0 || ret > PlayerMax) {
			LuaError(l, "bad player: %d" _C_ ret);
		}
		return ret;
	}
	player = LuaToString(l, -1);
	if (!strcmp(player, "any")) {
		return -1;
	} else if (!strcmp(player, "this")) {
		return ThisPlayer->Player;
	}
	LuaError(l, "bad player: %s" _C_ player);

	return 0;
}

/**
**  Get the unit-type.
**
**  @param l  Lua state.
**
**  @return   The unit-type pointer.
*/
global const UnitType* TriggerGetUnitType(lua_State* l)
{
	const char* unit;

	unit = LuaToString(l, -1);
	if (!strcmp(unit, "any")) {
		return ANY_UNIT;
	} else if (!strcmp(unit, "all")) {
		return ALL_UNITS;
	} else if (!strcmp(unit, "units")) {
		return ALL_FOODUNITS;
	} else if (!strcmp(unit, "buildings")) {
		return ALL_BUILDINGS;
	}

	return CclGetUnitType(l);
}

/*--------------------------------------------------------------------------
--  Conditions
--------------------------------------------------------------------------*/
local int CompareEq(int a, int b)
{
	return a == b;
}
local int CompareNEq(int a, int b)
{
	return a != b;
}
local int CompareGrEq(int a, int b)
{
	return a >= b;
}
local int CompareGr(int a, int b)
{
	return a > b;
}
local int CompareLeEq(int a, int b)
{
	return a <= b;
}
local int CompareLe(int a, int b)
{
	return a < b;
}

typedef int (*CompareFunction)(int, int);

/**
**  Returns a function pointer to the comparison function
**
**  @param op  The operation
**
**  @return    Function pointer to the compare function
*/
local CompareFunction GetCompareFunction(const char* op)
{
	if (op[0] == '=') {
		if ((op[1] == '=' && op[2] == '\0') || (op[1] == '\0')) {
			return &CompareEq;
		}
	} else if (op[0] == '>') {
		if (op[1] == '=' && op[2] == '\0') {
			return &CompareGrEq;
		} else if (op[1] == '\0') {
			return &CompareGr;
		}
	} else if (op[0] == '<') {
		if (op[1] == '=' && op[2] == '\0') {
			return &CompareLeEq;
		} else if (op[1] == '\0') {
			return &CompareLe;
		}
	} else if (op[0] == '!' && op[1] == '=' && op[2] == '\0') {
		return &CompareNEq;
	}
	return NULL;
}

/**
**  Player has the quantity of unit-type.
*/
local int CclIfUnit(lua_State* l)
{
	int plynr;
	int q;
	int pn;
	const UnitType* unittype;
	const char* op;
	CompareFunction compare;

	if (lua_gettop(l) != 4) {
		LuaError(l, "incorrect argument");
	}

	lua_pushvalue(l, 1);
	plynr = TriggerGetPlayer(l);
	lua_pop(l, 1);
	op = LuaToString(l, 2);
	q = LuaToNumber(l, 3);
	unittype = TriggerGetUnitType(l);

	compare = GetCompareFunction(op);
	if (!compare) {
		LuaError(l, "Illegal comparison operation in if-unit: %s" _C_ op);
	}

	if (plynr == -1) {
		plynr = 0;
		pn = PlayerMax;
	} else {
		pn = plynr + 1;
	}

	if (unittype == ANY_UNIT) {
		for (; plynr < pn; ++plynr) {
			int j;

			for (j = 0; j < NumUnitTypes; ++j) {
				if (compare(Players[plynr].UnitTypesCount[j], q)) {
					lua_pushboolean(l, 1);
					return 1;
				}
			}
		}
	} else if (unittype == ALL_UNITS) {
		for (; plynr < pn; ++plynr) {
			if (compare(Players[plynr].TotalNumUnits, q)) {
				lua_pushboolean(l, 1);
				return 1;
			}
		}
	} else if (unittype == ALL_FOODUNITS) {
		for (; plynr < pn; ++plynr) {
			if (compare(Players[plynr].TotalNumUnits - Players[plynr].NumBuildings, q)) {
				lua_pushboolean(l, 1);
				return 1;
			}
		}
	} else if (unittype == ALL_BUILDINGS) {
		for (; plynr < pn; ++plynr) {
			if (compare(Players[plynr].NumBuildings, q)) {
				lua_pushboolean(l, 1);
				return 1;
			}
		}
	} else {
		for (; plynr < pn; ++plynr) {
			DebugLevel3Fn("Player%d, %d == %s\n" _C_ plynr _C_ q _C_ unittype->Ident);
			if (compare(Players[plynr].UnitTypesCount[unittype->Slot], q)) {
				lua_pushboolean(l, 1);
				return 1;
			}
		}
	}

	lua_pushboolean(l, 0);
	return 1;
}

/**
**  Player has the quantity of unit-type at a location.
**
**  (if-unit-at {player} {op} {quantity} {unit} {location} {location})
*/
local int CclIfUnitAt(lua_State* l)
{
	int plynr;
	int q;
	int x1;
	int y1;
	int x2;
	int y2;
	const UnitType* unittype;
	const char* op;
	CompareFunction compare;
	Unit* table[UnitMax];
	Unit* unit;
	int an;
	int j;
	int s;

	if (lua_gettop(l) != 6) {
		LuaError(l, "incorrect argument");
	}

	lua_pushvalue(l, 1);
	plynr = TriggerGetPlayer(l);
	lua_pop(l, 1);
	op = LuaToString(l, 2);
	compare = GetCompareFunction(op);
	if (!compare) {
		LuaError(l, "Illegal comparison operator in if-unit-at: %s" _C_ op);
	}
	q = LuaToNumber(l, 3);
	lua_pushvalue(l, 1);
	unittype = TriggerGetUnitType(l);
	lua_pop(l, 1);
	if (!lua_istable(l, 5) || luaL_getn(l, 5) != 2) {
		LuaError(l, "incorrect argument");
	}
	lua_rawgeti(l, 5, 1);
	x1 = LuaToNumber(l, -1);
	lua_pop(l, 1);
	lua_rawgeti(l, 5, 2);
	y1 = LuaToNumber(l, -1);
	lua_pop(l, 1);
	if (!lua_istable(l, 6) || luaL_getn(l, 6) != 2) {
		LuaError(l, "incorrect argument");
	}
	lua_rawgeti(l, 6, 1);
	x2 = LuaToNumber(l, -1);
	lua_pop(l, 1);
	lua_rawgeti(l, 6, 2);
	y2 = LuaToNumber(l, -1);
	lua_pop(l, 1);

	//
	// Get all unit types in location.
	//
	// FIXME: I hope SelectUnits checks bounds?
	// FIXME: Yes, but caller should check.
	// NOTE: +1 right,bottom isn't inclusive :(
	an = UnitCacheSelect(x1, y1, x2 + 1, y2 + 1, table);
	//
	// Count the requested units
	//
	for (j = s = 0; j < an; ++j) {
		unit = table[j];
		//
		// Check unit type
		//
		// FIXME: ALL_UNITS
		if (unittype == ANY_UNIT ||
				(unittype == ALL_FOODUNITS && !unit->Type->Building) ||
				(unittype == ALL_BUILDINGS && unit->Type->Building) ||
				(unittype == unit->Type)) {
			//
			// Check the player
			//
			if (plynr == -1 || plynr == unit->Player->Player) {
				++s;
			}
		}
	}
	if (compare(s, q)) {
		lua_pushboolean(l, 1);
		return 1;
	}

	lua_pushboolean(l, 0);
	return 1;
}

/**
**  Player has the quantity of unit-type near to unit-type.
*/
local int CclIfNearUnit(lua_State* l)
{
	int plynr;
	int q;
	int n;
	int i;
	const UnitType* unittype;
	const UnitType* ut2;
	const char* op;
	Unit* table[UnitMax];
	CompareFunction compare;

	if (lua_gettop(l) != 5) {
		LuaError(l, "incorrect argument");
	}

	lua_pushvalue(l, 1);
	plynr = TriggerGetPlayer(l);
	lua_pop(l, 1);
	op = LuaToString(l, 2);
	q = LuaToNumber(l, 3);
	lua_pushvalue(l, 4);
	unittype = TriggerGetUnitType(l);
	lua_pop(l, 1);
	ut2 = CclGetUnitType(l);

	compare = GetCompareFunction(op);
	if (!compare) {
		LuaError(l, "Illegal comparison operation in if-near-unit: %s" _C_ op);
	}

	//
	// Get all unit types 'near'.
	//
	n = FindUnitsByType(ut2, table);
	DebugLevel3Fn("%s: %d\n" _C_ ut2->Ident _C_ n);
	for (i = 0; i < n; ++i) {
		Unit* unit;
		Unit* around[UnitMax];
		int an;
		int j;
		int s;

		unit = table[i];

		// FIXME: I hope SelectUnits checks bounds?
		// FIXME: Yes, but caller should check.
		// NOTE: +1 right,bottom isn't inclusive :(
		if (unit->Type->UnitType == UnitTypeLand) {
			an = UnitCacheSelect(unit->X - 1, unit->Y - 1,
				unit->X + unit->Type->TileWidth + 1,
				unit->Y + unit->Type->TileHeight + 1, around);
		} else {
			an = UnitCacheSelect(unit->X - 2, unit->Y - 2,
				unit->X + unit->Type->TileWidth + 2,
				unit->Y + unit->Type->TileHeight + 2, around);
		}
		DebugLevel3Fn("Units around %d: %d\n" _C_ UnitNumber(unit) _C_ an);
		//
		// Count the requested units
		//
		for (j = s = 0; j < an; ++j) {
			unit = around[j];
			//
			// Check unit type
			//
			// FIXME: ALL_UNITS
			if (unittype == ANY_UNIT ||
					(unittype == ALL_FOODUNITS && !unit->Type->Building) ||
					(unittype == ALL_BUILDINGS && unit->Type->Building) ||
					(unittype == unit->Type)) {
				//
				// Check the player
				//
				if (plynr == -1 || plynr == unit->Player->Player) {
					++s;
				}
			}
		}
		// Check if we counted the unit near itself
		if (unittype == ANY_UNIT ||
				(unittype == ALL_FOODUNITS && ut2->Building) ||
				(unittype == ALL_BUILDINGS && ut2->Building)) {
			--s;
		}
		if (compare(s, q)) {
			lua_pushboolean(l, 1);
			return 1;
		}
	}

	lua_pushboolean(l, 0);
	return 1;
}

/**
** Player has the quantity of rescued unit-type near to unit-type.
*/
local int CclIfRescuedNearUnit(lua_State* l)
{
	int plynr;
	int q;
	int n;
	int i;
	const UnitType* unittype;
	const UnitType* ut2;
	const char* op;
	Unit* table[UnitMax];
	CompareFunction compare;

	if (lua_gettop(l) != 5) {
		LuaError(l, "incorrect argument");
	}

	lua_pushvalue(l, 1);
	plynr = TriggerGetPlayer(l);
	lua_pop(l, 1);
	op = LuaToString(l, 2);
	q = LuaToNumber(l, 3);
	lua_pushvalue(l, 4);
	unittype = TriggerGetUnitType(l);
	lua_pop(l, 1);
	ut2 = CclGetUnitType(l);

	compare = GetCompareFunction(op);
	if (!compare) {
		LuaError(l, "Illegal comparison operation in if-rescued-near-unit: %s" _C_ op);
	}

	//
	// Get all unit types 'near'.
	//
	n = FindUnitsByType(ut2, table);
	DebugLevel3Fn("%s: %d\n" _C_ ut2->Ident _C_ n);
	for (i = 0; i < n; ++i) {
		Unit* unit;
		Unit* around[UnitMax];
		int an;
		int j;
		int s;

		unit = table[i];

		// FIXME: I hope SelectUnits checks bounds?
		// FIXME: Yes, but caller should check.
		// NOTE: +1 right,bottom isn't inclusive :(
		if (unit->Type->UnitType == UnitTypeLand) {
			an = UnitCacheSelect(unit->X - 1, unit->Y - 1,
				unit->X + unit->Type->TileWidth + 1,
				unit->Y + unit->Type->TileHeight + 1, around);
		} else {
			an = UnitCacheSelect(unit->X - 2, unit->Y - 2,
				unit->X + unit->Type->TileWidth + 2,
				unit->Y + unit->Type->TileHeight + 2, around);
		}
		DebugLevel3Fn("Units around %d: %d\n" _C_ UnitNumber(unit) _C_ an);
		//
		// Count the requested units
		//
		for (j = s = 0; j < an; ++j) {
			unit = around[j];
			if (unit->RescuedFrom) { // only rescued units
				//
				// Check unit type
				//
				// FIXME: ALL_UNITS
				if (unittype == ANY_UNIT ||
						(unittype == ALL_FOODUNITS && !unit->Type->Building) ||
						(unittype == ALL_BUILDINGS && unit->Type->Building) ||
						(unittype == unit->Type)) {
					//
					// Check the player
					//
					if (plynr == -1 || plynr == unit->Player->Player) {
						++s;
					}
				}
			}
		}
		// Check if we counted the unit near itself
		if (unittype == ANY_UNIT ||
				(unittype == ALL_FOODUNITS && ut2->Building) ||
				(unittype == ALL_BUILDINGS && ut2->Building)) {
			--s;
		}
		if (compare(s, q)) {
			lua_pushboolean(l, 1);
			return 1;
		}
	}

	lua_pushboolean(l, 0);
	return 1;
}

/**
**  Player has n opponents left.
*/
local int CclIfOpponents(lua_State* l)
{
	int plynr;
	int q;
	int pn;
	int n;
	const char* op;
	CompareFunction compare;

	if (lua_gettop(l) != 3) {
		LuaError(l, "incorrect argument");
	}

	lua_pushvalue(l, 1);
	plynr = TriggerGetPlayer(l);
	lua_pop(l, 1);
	op = LuaToString(l, 2);
	q = LuaToNumber(l, 3);

	compare = GetCompareFunction(op);
	if (!compare) {
		LuaError(l, "Illegal comparison operation in if-opponents: %s" _C_ op);
	}

	if (plynr == -1) {
		plynr = 0;
		pn = PlayerMax;
	} else {
		pn = plynr + 1;
	}

	//
	// Check the player opponents
	//
	for (n = 0; plynr < pn; ++plynr) {
		int i;

		for (i = 0; i < PlayerMax; ++i) {
			//
			// This player is our enemy and has units left.
			//
			if ((Players[i].Enemy & (1 << plynr)) && Players[i].TotalNumUnits) {
				++n;
			}
		}
		DebugLevel3Fn("Opponents of %d = %d\n" _C_ plynr _C_ n);
		if (compare(n, q)) {
			lua_pushboolean(l, 1);
			return 1;
		}
	}

	lua_pushboolean(l, 0);
	return 1;
}

/**
**  Player has the quantity of resource.
*/
local int CclIfResource(lua_State* l)
{
	int plynr;
	int q;
	int pn;
	const char* res;
	const char* op;
	CompareFunction compare;
	int i;

	if (lua_gettop(l) != 4) {
		LuaError(l, "incorrect argument");
	}

	lua_pushvalue(l, 1);
	plynr = TriggerGetPlayer(l);
	lua_pop(l, 1);
	op = LuaToString(l, 2);
	q = LuaToNumber(l, 3);
	res = LuaToString(l, 4);

	compare = GetCompareFunction(op);
	if (!compare) {
		LuaError(l, "Illegal comparison operation in if-resource: %s" _C_ op);
	}

	if (plynr == -1) {
		plynr = 0;
		pn = PlayerMax;
	} else {
		pn = plynr + 1;
	}

	for (i = 0; i < MaxCosts; ++i) {
		if (!strcmp(res, DefaultResourceNames[i])) {
			for (; plynr < pn; ++plynr) {
				if (compare(Players[plynr].Resources[i], q)) {
					lua_pushboolean(l, 1);
					return 1;
				}
			}
			lua_pushboolean(l, 0);
			return 1;
		}
	}
	if (!strcmp(res, "all")) {
		int j;
		int sum;

		sum = 0;
		for (; plynr < pn; ++plynr) {
			for (j = 1; j < MaxCosts; ++j) {
				sum += Players[plynr].Resources[j];
			}
		}
		if (compare(sum, q)) {
			lua_pushboolean(l, 1);
			return 1;
		}
	} else if (!strcmp(res, "any")) {
		int j;

		for (; plynr < pn; ++plynr) {
			for (j = 1; j < MaxCosts; ++j) {
				if (compare(Players[plynr].Resources[j], q)) {
					lua_pushboolean(l, 1);
					return 1;
				}
			}
		}
	}

	lua_pushboolean(l, 0);
	return 1;
}

/**
**  Player has quantity kills
*/
local int CclIfKills(lua_State* l)
{
	int plynr;
	int q;
	int pn;
	int n;
	const char* op;
	CompareFunction compare;

	if (lua_gettop(l) != 3) {
		LuaError(l, "incorrect argument");
	}

	lua_pushvalue(l, 1);
	plynr = TriggerGetPlayer(l);
	lua_pop(l, 1);
	op = LuaToString(l, 2);
	q = LuaToNumber(l, 3);

	compare = GetCompareFunction(op);
	if (!compare) {
		LuaError(l, "Illegal comparison operation in if-kills: %s" _C_ op);
	}

	if (plynr == -1) {
		plynr = 0;
		pn = PlayerMax;
	} else {
		pn = plynr + 1;
	}

	for (n = 0; plynr < pn; ++plynr) {
		if (compare(Players[plynr].TotalKills, q)) {
			lua_pushboolean(l, 1);
			return 1;
		}
	}

	lua_pushboolean(l, 0);
	return 1;
}

/**
**  Player has a certain score
*/
local int CclIfScore(lua_State* l)
{
	int plynr;
	int q;
	int pn;
	int n;
	const char* op;
	CompareFunction compare;

	if (lua_gettop(l) != 3) {
		LuaError(l, "incorrect argument");
	}

	lua_pushvalue(l, 1);
	plynr = TriggerGetPlayer(l);
	lua_pop(l, 1);
	op = LuaToString(l, 2);
	q = LuaToNumber(l, 3);

	compare = GetCompareFunction(op);
	if (!compare) {
		LuaError(l, "Illegal comparison operation in if-score: %s" _C_ op);
	}

	if (plynr == -1) {
		plynr = 0;
		pn = PlayerMax;
	} else {
		pn = plynr + 1;
	}

	for (n = 0; plynr < pn; ++plynr) {
		if (compare(Players[plynr].Score, q)) {
			lua_pushboolean(l, 1);
			return 1;
		}
	}

	lua_pushboolean(l, 0);
	return 1;
}

/**
**  Number of game cycles elapsed
*/
local int CclIfElapsed(lua_State* l)
{
	int q;
	const char* op;
	CompareFunction compare;

	if (lua_gettop(l) != 2) {
		LuaError(l, "incorrect argument");
	}

	op = LuaToString(l, 1);
	q = LuaToNumber(l, 2);

	compare = GetCompareFunction(op);
	if (!compare) {
		LuaError(l, "Illegal comparison operation in if-elapsed: %s" _C_ op);
	}

	if (compare(GameCycle, q)) {
		lua_pushboolean(l, 1);
		return 1;
	}

	lua_pushboolean(l, 0);
	return 1;
}

/**
**  Check the timer value
*/
local int CclIfTimer(lua_State* l)
{
	int q;
	const char* op;
	CompareFunction compare;

	if (lua_gettop(l) != 2) {
		LuaError(l, "incorrect argument");
	}

	if (!GameTimer.Init) {
		lua_pushboolean(l, 0);
		return 1;
	}

	op = LuaToString(l, 1);
	q = LuaToNumber(l, 2);

	compare = GetCompareFunction(op);
	if (!compare) {
		LuaError(l, "Illegal comparison operation in if-timer: %s" _C_ op);
	}

	if (compare(GameTimer.Cycles, q)) {
		lua_pushboolean(l, 1);
		return 1;
	}

	lua_pushboolean(l, 0);
	return 1;
}

/**
**  Check the switch value
*/
local int CclIfSwitch(lua_State* l)
{
	int i;
	unsigned char s;

	if (lua_gettop(l) != 2) {
		LuaError(l, "incorrect argument");
	}

	i = LuaToNumber(l, 1);
	if (i < 0 || i >= MAX_SWITCH) {
		LuaError(l, "Invalid switch number %i" _C_ i);
	}

	if (lua_isboolean(l, 2)) {
		s = LuaToBoolean(l, 2);
	} else {
		s = LuaToNumber(l, 2);
		if (s) {
			s = 1;
		}
	}

	if (Switch[i] == s) {
		lua_pushboolean(l, 1);
		return 1;
	}
	lua_pushboolean(l, 0);
	return 1;
}

/*---------------------------------------------------------------------------
--		Actions
---------------------------------------------------------------------------*/
/**
**  Action condition player wins.
*/
local int CclActionVictory(lua_State* l)
{
	if (lua_gettop(l) != 0) {
		LuaError(l, "incorrect argument");
	}

	GameResult = GameVictory;
	GamePaused = 1;
	GameRunning = 0;
	return 0;
}

/**
**  Action condition player lose.
*/
local int CclActionDefeat(lua_State* l)
{
	if (lua_gettop(l) != 0) {
		LuaError(l, "incorrect argument");
	}

	GameResult = GameDefeat;
	GamePaused = 1;
	GameRunning = 0;
	return 0;
}

/**
**  Action condition player draw.
*/
local int CclActionDraw(lua_State* l)
{
	if (lua_gettop(l) != 0) {
		LuaError(l, "incorrect argument");
	}

	GameResult = GameDraw;
	GamePaused = 1;
	GameRunning = 0;
	return 0;
}

/**
**  Action set timer
*/
local int CclActionSetTimer(lua_State* l)
{
	if (lua_gettop(l) != 2) {
		LuaError(l, "incorrect argument");
	}

	GameTimer.Cycles = LuaToNumber(l, 1);
	GameTimer.Increasing = LuaToNumber(l, 2);
	GameTimer.Init = 1;
	GameTimer.LastUpdate = GameCycle;

	return 0;
}

/**
**  Action start timer
*/
local int CclActionStartTimer(lua_State* l)
{
	if (lua_gettop(l) != 0) {
		LuaError(l, "incorrect argument");
	}

	GameTimer.Running = 1;
	GameTimer.Init = 1;
	return 0;
}

/**
**  Action stop timer
*/
local int CclActionStopTimer(lua_State* l)
{
	if (lua_gettop(l) != 0) {
		LuaError(l, "incorrect argument");
	}

	GameTimer.Running = 0;
	return 0;
}

/**
**  Action wait
*/
local int CclActionWait(lua_State* l)
{
	if (lua_gettop(l) != 1) {
		LuaError(l, "incorrect argument");
	}

	WaitFrame = FrameCounter +
		(FRAMES_PER_SECOND * VideoSyncSpeed / 100 * (int)LuaToNumber(l, 1) + 999) / 1000;
	return 0;
}

/**
**  Action stop timer
*/
local int CclActionSetSwitch(lua_State* l)
{
	int i;
	unsigned char s;

	if (lua_gettop(l) != 2) {
		LuaError(l, "incorrect argument");
	}

	i = LuaToNumber(l, 1);
	if (i < 0 || i >= MAX_SWITCH) {
		LuaError(l, "Invalid switch number: %d" _C_ i);
	}

	if (lua_isboolean(l, 2)) {
		s = LuaToBoolean(l, 2);
	} else {
		s = LuaToNumber(l, 2);
		if (s) {
			s = 1;
		}
	}

	Switch[i] = s;
	lua_pushvalue(l, 2);
	return 1;
}

/**
**  Add a trigger.
*/
local int CclAddTrigger(lua_State* l)
{
	int i;

	if (lua_gettop(l) != 2 || !lua_isfunction(l, 1) ||
			(!lua_isfunction(l, 2) && !lua_istable(l, 2))) {
		LuaError(l, "incorrect argument");
	}

	//
	// Make a list of all triggers.
	// A trigger is a pair of condition and action
	//
	lua_pushstring(l, "_triggers_");
	lua_gettable(l, LUA_GLOBALSINDEX);

	if (lua_isnil(l, -1)) {
		puts("Trigger not set, defining trigger");
		lua_pop(l, 1);
		lua_pushstring(l, "_triggers_");
		lua_newtable(l);
		lua_settable(l, LUA_GLOBALSINDEX);
		lua_pushstring(l, "_triggers_");
		lua_gettable(l, LUA_GLOBALSINDEX);
	}

	i = luaL_getn(l, -1);
	if (ActiveTriggers && !ActiveTriggers[i / 2]) {
		lua_pushnil(l);
		lua_rawseti(l, -2, i + 1);
		lua_pushnil(l);
		lua_rawseti(l, -2, i + 2);
	} else {
		lua_pushvalue(l, 1);
		lua_rawseti(l, -2, i + 1);
		lua_newtable(l);
		lua_pushvalue(l, 2);
		lua_rawseti(l, -2, 1);
		lua_rawseti(l, -2, i + 2);
	}
	lua_pop(l, 1);

	return 0;
}

/**
**  Set the trigger values
*/
local int CclSetTriggers(lua_State* l)
{
	if (lua_gettop(l) != 3) {
		LuaError(l, "incorrect argument");
	}
	Trigger = LuaToNumber(l, 1);
	WaitTrigger = LuaToNumber(l, 2);
	WaitFrame = LuaToNumber(l, 3);

	return 0;
}

/**
**  Set the active triggers
*/
local int CclSetActiveTriggers(lua_State* l)
{
	int args;
	int j;

	args = lua_gettop(l);
	ActiveTriggers = malloc(args * sizeof(*ActiveTriggers));
	for (j = 0; j < args; ++j) {
		ActiveTriggers[j] = LuaToBoolean(l, j + 1);
	}

	return 0;
}

/**
**  Execute a trigger action
**
**  @param script  Script to execute
**
**  @return        1 if the trigger should be removed
*/
local int TriggerExecuteAction(int script)
{
	int ret;
	int args;
	int j;

	ret = 0;

	lua_rawgeti(Lua, -1, script + 1);
	args = luaL_getn(Lua, -1);
	for (j = 0; j < args; ++j) {
		lua_rawgeti(Lua, -1, j + 1);
		LuaCall(0, 0);
		if (lua_gettop(Lua) > 2 && lua_toboolean(Lua, -1)) {
			ret = 1;
		} else {
			ret = 0;
		}
		lua_settop(Lua, 2);
		if (WaitFrame > FrameCounter) {
			lua_pop(Lua, 1);
			return 0;
		}
	}
	lua_pop(Lua, 1);

	// If action returns false remove it
	return !ret;
}

/**
**  Remove a trigger
**
**  @param trig  Current trigger
*/
local void TriggerRemoveTrigger(int trig)
{
	lua_pushnumber(Lua, -1);
	lua_rawseti(Lua, -2, trig);
	lua_pushnumber(Lua, -1);
	lua_rawseti(Lua, -2, trig + 1);
}

/**
**  Check trigger each game cycle.
*/
global void TriggersEachCycle(void)
{
	int triggers;

	lua_pushstring(Lua, "_triggers_");
	lua_gettable(Lua, LUA_GLOBALSINDEX);
	triggers = luaL_getn(Lua, -1);

	if (Trigger >= triggers) {
		Trigger = 0;
	}

	if (WaitFrame > FrameCounter) {
		lua_pop(Lua, 1);
		return;
	}
	if (WaitFrame && WaitFrame <= FrameCounter) {
		WaitFrame = 0;
		if (TriggerExecuteAction(WaitTrigger + 1)) {
			TriggerRemoveTrigger(WaitTrigger);
		}
		lua_pop(Lua, 1);
		return;
	}

	if (GamePaused) {
		lua_pop(Lua, 1);
		return;
	}

	// Skip to the next trigger
	while (Trigger < triggers) {
		lua_rawgeti(Lua, -1, Trigger + 1);
		if (!lua_isnumber(Lua, -1)) {
			break;
		}
		lua_pop(Lua, 1);
		Trigger += 2;
	}
	if (Trigger < triggers) {
		WaitTrigger = Trigger;
		Trigger += 2;
		LuaCall(0, 0);
		// If condition is true execute action
		if (lua_gettop(Lua) > 1 && lua_toboolean(Lua, -1)) {
			lua_settop(Lua, 1);
			if (TriggerExecuteAction(WaitTrigger + 1)) {
				TriggerRemoveTrigger(WaitTrigger);
			}
		}
		lua_settop(Lua, 1);
	}
	lua_pop(Lua, 1);
}

/**
**  Register CCL features for triggers.
*/
global void TriggerCclRegister(void)
{
	lua_register(Lua, "AddTrigger", CclAddTrigger);
	lua_register(Lua, "SetTriggers", CclSetTriggers);
	lua_register(Lua, "SetActiveTriggers", CclSetActiveTriggers);
	// Conditions
	lua_register(Lua, "IfUnit", CclIfUnit);
	lua_register(Lua, "IfUnitAt", CclIfUnitAt);
	lua_register(Lua, "IfNearUnit", CclIfNearUnit);
	lua_register(Lua, "IfRescuedNearUnit", CclIfRescuedNearUnit);
	lua_register(Lua, "IfOpponents", CclIfOpponents);
	lua_register(Lua, "IfResource", CclIfResource);
	lua_register(Lua, "IfKills", CclIfKills);
	lua_register(Lua, "IfScore", CclIfScore);
	lua_register(Lua, "IfElapsed", CclIfElapsed);
	lua_register(Lua, "IfTimer", CclIfTimer);
	lua_register(Lua, "IfSwitch", CclIfSwitch);
	// Actions
	lua_register(Lua, "ActionVictory", CclActionVictory);
	lua_register(Lua, "ActionDefeat", CclActionDefeat);
	lua_register(Lua, "ActionDraw", CclActionDraw);
	lua_register(Lua, "ActionSetTimer", CclActionSetTimer);
	lua_register(Lua, "ActionStartTimer", CclActionStartTimer);
	lua_register(Lua, "ActionStopTimer", CclActionStopTimer);
	lua_register(Lua, "ActionWait", CclActionWait);
	lua_register(Lua, "ActionSetSwitch", CclActionSetSwitch);
}

/**
**  Save the trigger module.
**
**  @param file  Open file to print to
*/
global void SaveTriggers(CLFile* file)
{
	int i;
	int triggers;

	lua_pushstring(Lua, "_triggers_");
	lua_gettable(Lua, LUA_GLOBALSINDEX);
	triggers = luaL_getn(Lua, -1);

	CLprintf(file, "SetActiveTriggers(");
	for (i = 0; i < triggers; i += 2) {
		lua_rawgeti(Lua, -1, i + 1);
		if (i) {
			CLprintf(file, ", ");
		}
		if (!lua_isnil(Lua, -1)) {
			CLprintf(file, "true");
		} else {
			CLprintf(file, "false");
		}
		lua_pop(Lua, 1);
	}
	CLprintf(file, ")\n");

	CLprintf(file, "SetTriggers(%d, %d, %d)\n", Trigger, WaitTrigger, WaitFrame);

	if (GameTimer.Init) {
		CLprintf(file, "ActionSetTimer(%ld, %d)\n",
			GameTimer.Cycles, GameTimer.Increasing);
		if (GameTimer.Running) {
			CLprintf(file, "ActionStartTimer()\n");
		}
	}
}

/**
**  Initialize the trigger module.
*/
global void InitTriggers(void)
{
	//
	// Setup default triggers
	//
	WaitFrame = 0;

	// FIXME: choose the triggers for game type

	lua_pushstring(Lua, "_triggers_");
	lua_gettable(Lua, LUA_GLOBALSINDEX);
	if (lua_isnil(Lua, -1)) {
		lua_pushstring(Lua, "SinglePlayerTriggers");
		lua_gettable(Lua, LUA_GLOBALSINDEX);
		LuaCall(0, 1);
	}
	lua_pop(Lua, 1);

	memset(Switch, 0, sizeof(Switch));
}

/**
**  Clean up the trigger module.
*/
global void CleanTriggers(void)
{
	lua_pushstring(Lua, "_triggers_");
	lua_pushnil(Lua);
	lua_settable(Lua, LUA_GLOBALSINDEX);

	Trigger = 0;

	free(ActiveTriggers);
	ActiveTriggers = NULL;

	memset(&GameTimer, 0, sizeof(GameTimer));
}

//@}
