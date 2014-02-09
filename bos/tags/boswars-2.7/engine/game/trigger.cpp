//     ____                _       __               
//    / __ )____  _____   | |     / /___ ___________
//   / __  / __ \/ ___/   | | /| / / __ `/ ___/ ___/
//  / /_/ / /_/ (__  )    | |/ |/ / /_/ / /  (__  ) 
// /_____/\____/____/     |__/|__/\__,_/_/  /____/  
//                                              
//       A futuristic real-time strategy game.
//          This file is part of Bos Wars.
//
/**@name trigger.cpp - The trigger handling. */
//
//      (c) Copyright 2002-2007 by Lutz Sammer and Jimmy Salmon
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
#include <setjmp.h>

#include "stratagus.h"
#include "script.h"
#include "unittype.h"
#include "unit_cache.h"
#include "player.h"
#include "trigger.h"
#include "results.h"
#include "interface.h"
#include "unit.h"
#include "iolib.h"


/*----------------------------------------------------------------------------
--  Variables
----------------------------------------------------------------------------*/

CTimer GameTimer;               /// The game timer

/* Index of the trigger in the _trigger_ table where TriggersEachCycle will
   start at the next cycle. */
static int nextTrigger; 

/* Trigger activation flags loaded from a saved game. Only triggers defined 
   by AddTrigger which have the true flag in ActiveTriggers will be 
   effectively added. */
static bool *ActiveTriggers;

/// Some data accessible for script during the game.
TriggerDataType TriggerData;


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
int TriggerGetPlayer(lua_State *l)
{
	const char *player;
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
		return ThisPlayer->Index;
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
const CUnitType *TriggerGetUnitType(lua_State *l)
{
	const char *unit;

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

/**
**  Check whether a unit is of the specified type or in the specified
**  category.
**
**  @param unit             Unit to examine; must not be NULL.
**  @param triggerUnitType  As returned by TriggerGetUnitType.
**
**  @return   true if the unit matches the type or category, else false.
*/
bool TriggerMatchUnitType(const CUnit *unit, const CUnitType *triggerUnitType)
{
	if (triggerUnitType == ANY_UNIT) {
		return true;
	} else if (triggerUnitType == ALL_UNITS) {
		// FIXME: ALL_UNITS should be "sum of units and buildings",
		// but that currently means just ANY_UNIT.
		return true;
	} else if (triggerUnitType == ALL_FOODUNITS) {
		return !unit->Type->Building;
	} else if (triggerUnitType == ALL_BUILDINGS) {
		return unit->Type->Building;
	} else {
		return unit->Type == triggerUnitType;
	}
}

/*--------------------------------------------------------------------------
--  Conditions
--------------------------------------------------------------------------*/

static int CompareEq(int a, int b)
{
	return a == b;
}
static int CompareNEq(int a, int b)
{
	return a != b;
}
static int CompareGrEq(int a, int b)
{
	return a >= b;
}
static int CompareGr(int a, int b)
{
	return a > b;
}
static int CompareLeEq(int a, int b)
{
	return a <= b;
}
static int CompareLe(int a, int b)
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
static CompareFunction GetCompareFunction(const char *op)
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
**  Return the number of units of a giver unit-type and player at a location.
*/
static int CclGetNumUnitsAt(lua_State *l)
{
	int plynr;
	int x1;
	int y1;
	int x2;
	int y2;
	const CUnitType *unittype;
	CUnit *table[UnitMax];
	CUnit *unit;
	int an;
	int j;
	int s;

	LuaCheckArgs(l, 4);

	plynr = LuaToNumber(l, 1);
	lua_pushvalue(l, 2);
	unittype = TriggerGetUnitType(l);
	lua_pop(l, 1);
	LuaCheckTableSize(l, 3, 2);
	x1 = LuaToNumber(l, 3, 1);
	y1 = LuaToNumber(l, 3, 2);
	LuaCheckTableSize(l, 4, 2);
	x2 = LuaToNumber(l, 4, 1);
	y2 = LuaToNumber(l, 4, 2);

	//
	// Get all unit types in location.
	//
	an = UnitCache.Select(x1, y1, x2 + 1, y2 + 1, table, UnitMax);
	//
	// Count the requested units
	//
	for (j = s = 0; j < an; ++j) {
		unit = table[j];
		//
		// Check unit type
		//
		if (TriggerMatchUnitType(unit, unittype) && !unit->Constructed) {
			//
			// Check the player
			//
			if (plynr == -1 || plynr == unit->Player->Index) {
				++s;
			}
		}
	}
	lua_pushnumber(l, s);
	return 1;
}

/**
**  Player has the quantity of unit-type near to unit-type.
*/
static int CclIfNearUnit(lua_State *l)
{
	int plynr;
	int q;
	int n;
	int i;
	const CUnitType *unittype;
	const CUnitType *ut2;
	const char *op;
	CUnit *table[UnitMax];
	CompareFunction compare;

	LuaCheckArgs(l, 5);

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
	n = FindUnitsByType(ut2, table, UnitMax);
	for (i = 0; i < n; ++i) {
		CUnit *unit;
		CUnit *around[UnitMax];
		int an;
		int j;
		int s;

		unit = table[i];

		if (unit->Type->UnitType == UnitTypeLand) {
			an = UnitCache.Select(unit->X - 1, unit->Y - 1,
				unit->X + unit->Type->TileWidth + 1,
				unit->Y + unit->Type->TileHeight + 1, around, UnitMax);
		} else {
			an = UnitCache.Select(unit->X - 2, unit->Y - 2,
				unit->X + unit->Type->TileWidth + 2,
				unit->Y + unit->Type->TileHeight + 2, around, UnitMax);
		}
		//
		// Count the requested units
		//
		for (j = s = 0; j < an; ++j) {
			unit = around[j];
			//
			// Check unit type
			//
			if (TriggerMatchUnitType(unit, unittype)) {
				//
				// Check the player
				//
				if (plynr == -1 || plynr == unit->Player->Index) {
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
**  Player has the quantity of rescued unit-type near to unit-type.
*/
static int CclIfRescuedNearUnit(lua_State *l)
{
	int plynr;
	int q;
	int n;
	int i;
	const CUnitType *unittype;
	const CUnitType *ut2;
	const char *op;
	CUnit *table[UnitMax];
	CompareFunction compare;

	LuaCheckArgs(l, 5);

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
	n = FindUnitsByType(ut2, table, UnitMax);
	for (i = 0; i < n; ++i) {
		CUnit *unit;
		CUnit *around[UnitMax];
		int an;
		int j;
		int s;

		unit = table[i];

		if (unit->Type->UnitType == UnitTypeLand) {
			an = UnitCache.Select(unit->X - 1, unit->Y - 1,
				unit->X + unit->Type->TileWidth + 1,
				unit->Y + unit->Type->TileHeight + 1, around, UnitMax);
		} else {
			an = UnitCache.Select(unit->X - 2, unit->Y - 2,
				unit->X + unit->Type->TileWidth + 2,
				unit->Y + unit->Type->TileHeight + 2, around, UnitMax);
		}
		//
		// Count the requested units
		//
		for (j = s = 0; j < an; ++j) {
			unit = around[j];
			if (unit->RescuedFrom) { // only rescued units
				//
				// Check unit type
				//
				if (TriggerMatchUnitType(unit, unittype)) {
					//
					// Check the player
					//
					if (plynr == -1 || plynr == unit->Player->Index) {
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
**  Returns the number of opponents of a given player.
*/
int GetNumOpponents(int player)
{
	int n = 0;

	// Check the player opponents
	for (int i = 0; i < PlayerMax; ++i) {
		// This player is our enemy and has units left.
		if (((Players[player].Enemy & (1 << i)) || (Players[i].Enemy & (1 << player))) &&
				Players[i].TotalNumUnits) {
			++n;
		}
	}

	return n;
}

/**
**  Check the timer value
*/
int GetTimer()
{
	if (!GameTimer.Init) {
		return 0;
	}
	return GameTimer.Cycles;
}

/*---------------------------------------------------------------------------
-- Actions
---------------------------------------------------------------------------*/

/**
**  Stop the running game with a given result
*/
void StopGame(GameResults result)
{
	GameResult = result;
	GamePaused = true;
	GameRunning = false;
}

/**
**  Action condition player wins.
*/
void ActionVictory()
{
	StopGame(GameVictory);
}

/**
**  Action condition player lose.
*/
void ActionDefeat()
{
	StopGame(GameDefeat);
}

/**
**  Action condition player draw.
*/
void ActionDraw()
{
	StopGame(GameDraw);
}

/**
**  Action set timer
*/
void ActionSetTimer(int cycles, bool increasing)
{
	GameTimer.Cycles = cycles;
	GameTimer.Increasing = increasing;
	GameTimer.Init = true;
	GameTimer.LastUpdate = GameCycle;
}

/**
**  Action start timer
*/
void ActionStartTimer()
{
	GameTimer.Running = true;
	GameTimer.Init = true;
}

/**
**  Action stop timer
*/
void ActionStopTimer()
{
	GameTimer.Running = false;
}

/**
**  Add a trigger.
*/
static int CclAddTrigger(lua_State *l)
{
	int i;

	LuaCheckArgs(l, 2);
	if (!lua_isfunction(l, 1) ||
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
		DebugPrint("Trigger not set, defining trigger\n");
		lua_pop(l, 1);
		lua_pushstring(l, "_triggers_");
		lua_newtable(l);
		lua_settable(l, LUA_GLOBALSINDEX);
		lua_pushstring(l, "_triggers_");
		lua_gettable(l, LUA_GLOBALSINDEX);
	}

	i = lua_objlen(l, -1);
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
void SetTrigger(int trigger)
{
	nextTrigger = trigger;
}

/**
**  Set the active triggers
*/
static int CclSetActiveTriggers(lua_State *l)
{
	int args;

	args = lua_gettop(l);
	ActiveTriggers = new bool[args];
	for (int j = 0; j < args; ++j) {
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
static int TriggerExecuteAction(int script)
{
	int ret;
	int args;
	int j;
	int base = lua_gettop(Lua);

	ret = 0;

	lua_rawgeti(Lua, -1, script + 1);
	args = lua_objlen(Lua, -1);
	for (j = 0; j < args; ++j) {
		lua_rawgeti(Lua, -1, j + 1);
		LuaCall(0, 0);
		if (lua_gettop(Lua) > base + 1 && lua_toboolean(Lua, -1)) {
			ret = 1;
		} else {
			ret = 0;
		}
		lua_settop(Lua, base + 1);
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
static void TriggerRemoveTrigger(int trig)
{
	lua_pushnumber(Lua, -1);
	lua_rawseti(Lua, -2, trig + 1);
	lua_pushnumber(Lua, -1);
	lua_rawseti(Lua, -2, trig + 2);
}

/**
**  Check trigger each game cycle.
*/
void TriggersEachCycle(void)
{
	int triggers;
	int base = lua_gettop(Lua);

	lua_pushstring(Lua, "_triggers_");
	lua_gettable(Lua, LUA_GLOBALSINDEX);
	triggers = lua_objlen(Lua, -1);

	if (nextTrigger >= triggers) {
		nextTrigger = 0;
	}

	if (GamePaused) {
		lua_pop(Lua, 1);
		return;
	}

	// Skip to the next trigger
	while (nextTrigger < triggers) {
		lua_rawgeti(Lua, -1, nextTrigger + 1);
		if (!lua_isnumber(Lua, -1)) {
			break;
		}
		lua_pop(Lua, 1);
		nextTrigger += 2;
	}
	if (nextTrigger < triggers) {
		int currentTrigger = nextTrigger;
		nextTrigger += 2;
		LuaCall(0, 0);
		// If condition is true execute action
		if (lua_gettop(Lua) > base + 1 && lua_toboolean(Lua, -1)) {
			lua_settop(Lua, base + 1);
			if (TriggerExecuteAction(currentTrigger + 1)) {
				TriggerRemoveTrigger(currentTrigger);
			}
		}
		lua_settop(Lua, base + 1);
	}
	lua_pop(Lua, 1);
}

/**
**  Register CCL features for triggers.
*/
void TriggerCclRegister(void)
{
	lua_register(Lua, "AddTrigger", CclAddTrigger);
	lua_register(Lua, "SetActiveTriggers", CclSetActiveTriggers);
	// Conditions
	lua_register(Lua, "GetNumUnitsAt", CclGetNumUnitsAt);
	lua_register(Lua, "IfNearUnit", CclIfNearUnit);
	lua_register(Lua, "IfRescuedNearUnit", CclIfRescuedNearUnit);
}

/**
**  Save the trigger module.
**
**  @param file  Open file to print to
*/
void SaveTriggers(CFile *file)
{
	int i;
	int triggers;

	lua_pushstring(Lua, "_triggers_");
	lua_gettable(Lua, LUA_GLOBALSINDEX);
	triggers = lua_objlen(Lua, -1);

	file->printf("SetActiveTriggers(");
	for (i = 0; i < triggers; i += 2) {
		lua_rawgeti(Lua, -1, i + 1);
		if (i) {
			file->printf(", ");
		}
		if (!lua_isnil(Lua, -1)) {
			file->printf("true");
		} else {
			file->printf("false");
		}
		lua_pop(Lua, 1);
	}
	file->printf(")\n");

	file->printf("SetTrigger(%d)\n", nextTrigger);

	if (GameTimer.Init) {
		file->printf("ActionSetTimer(%ld, %s)\n",
			GameTimer.Cycles, (GameTimer.Increasing ? "true" : "false"));
		if (GameTimer.Running) {
			file->printf("ActionStartTimer()\n");
		}
	}
}

/**
**  Initialize the trigger module.
*/
void InitTriggers(void)
{
	//
	// Setup default triggers
	//
	// FIXME: choose the triggers for game type

	lua_pushstring(Lua, "_triggers_");
	lua_gettable(Lua, LUA_GLOBALSINDEX);
	if (lua_isnil(Lua, -1)) {
		lua_pushstring(Lua, "SinglePlayerTriggers");
		lua_gettable(Lua, LUA_GLOBALSINDEX);
		LuaCall(0, 1);
	}
	lua_pop(Lua, 1);
}

/**
**  Clean up the trigger module.
*/
void CleanTriggers(void)
{
	lua_pushstring(Lua, "_triggers_");
	lua_pushnil(Lua);
	lua_settable(Lua, LUA_GLOBALSINDEX);

	nextTrigger = 0;

	delete[] ActiveTriggers;
	ActiveTriggers = NULL;

	GameTimer.Reset();
}

//@}
