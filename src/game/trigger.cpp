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

#include "stratagus.h"
#include "script.h"
#include "unittype.h"
#include "player.h"
#include "trigger.h"
#include "results.h"
#include "interface.h"
#include "unit.h"
#include "iolib.h"
#include "map.h"

/*----------------------------------------------------------------------------
--  Variables
----------------------------------------------------------------------------*/

CTimer GameTimer;               /// The game timer
static int Trigger;
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
	LuaCheckArgs(l, 4);

	int plynr = LuaToNumber(l, 1);
	lua_pushvalue(l, 2);
	const CUnitType *unittype = TriggerGetUnitType(l);
	lua_pop(l, 1);

	Vec2i minPos;
	Vec2i maxPos;
	CclGetPos(l, &minPos.x, &minPos.y, 3);
	CclGetPos(l, &maxPos.x, &maxPos.y, 4);
	maxPos.x += 1;
	maxPos.y += 1;

	std::vector<CUnit*> units;

	Map.Select(minPos, maxPos, units);

	int s = 0;
	for (size_t i = 0; i != units.size(); ++i) {
		const CUnit& unit = *units[i];
		// Check unit type

		// FIXME: ALL_UNITS
		if (unittype == ANY_UNIT
			|| (unittype == ALL_FOODUNITS && !unit.Type->Building)
			|| (unittype == ALL_BUILDINGS && unit.Type->Building)
			|| (unittype == unit.Type && !unit.Constructed)) {

			// Check the player
			if (plynr == -1 || plynr == unit.Player->Index) {
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
	LuaCheckArgs(l, 5);
	lua_pushvalue(l, 1);
	const int plynr = TriggerGetPlayer(l);
	lua_pop(l, 1);
	const char* op = LuaToString(l, 2);
	const int q = LuaToNumber(l, 3);
	lua_pushvalue(l, 4);
	const CUnitType *unittype = TriggerGetUnitType(l);
	lua_pop(l, 1);
	const CUnitType *ut2 = CclGetUnitType(l);
	if (!unittype || !ut2) {
		LuaError(l, "CclIfNearUnit: not a unit-type valid");
	}
	CompareFunction compare = GetCompareFunction(op);
	if (!compare) {
		LuaError(l, "Illegal comparison operation in if-near-unit: %s" _C_ op);
	}

	//
	// Get all unit types 'near'.
	//

	std::vector<CUnit*> unitsOfType;

	FindUnitsByType(*ut2, unitsOfType);
	for (size_t i = 0; i != unitsOfType.size(); ++i) {
		const CUnit &centerUnit = *unitsOfType[i];

		std::vector<CUnit *> around;
		Map.SelectAroundUnit(centerUnit, 1, around);

		// Count the requested units
		int s = 0;
		for (size_t j = 0; j < around.size(); ++j) {
			const CUnit& unit = *around[j];

			// Check unit type
			//
			// FIXME: ALL_UNITS
			if (unittype == ANY_UNIT
				|| (unittype == ALL_FOODUNITS && !unit.Type->Building)
				|| (unittype == ALL_BUILDINGS && unit.Type->Building)
				|| (unittype == unit.Type)) {

				// Check the player
				if (plynr == -1 || plynr == unit.Player->Index) {
					++s;
				}
			}
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
	LuaCheckArgs(l, 5);

	lua_pushvalue(l, 1);
	const int plynr = TriggerGetPlayer(l);
	lua_pop(l, 1);
	const char* op = LuaToString(l, 2);
	const int q = LuaToNumber(l, 3);
	lua_pushvalue(l, 4);
	const CUnitType *unittype = TriggerGetUnitType(l);
	lua_pop(l, 1);
	const CUnitType *ut2 = CclGetUnitType(l);
	if (!unittype || !ut2) {
		LuaError(l, "CclIfRescuedNearUnit: not a unit-type valid");
	}

	CompareFunction compare = GetCompareFunction(op);
	if (!compare) {
		LuaError(l, "Illegal comparison operation in if-rescued-near-unit: %s" _C_ op);
	}

	// Get all unit types 'near'.
	std::vector<CUnit *> table;
	FindUnitsByType(*ut2, table);
	for (size_t i = 0; i != table.size(); ++i) {
		CUnit &centerUnit = *table[i];
		std::vector<CUnit *> around;

		Map.SelectAroundUnit(centerUnit, 1, around);

		// Count the requested units
		int s = 0;
		for (size_t j = 0; j != around.size(); ++j) {
			CUnit& unit = *around[j];

			if (unit.RescuedFrom) { // only rescued units
				// Check unit type

				// FIXME: ALL_UNITS
				if (unittype == ANY_UNIT
					|| (unittype == ALL_FOODUNITS && !unit.Type->Building)
					|| (unittype == ALL_BUILDINGS && unit.Type->Building)
					|| (unittype == unit.Type)) {

					// Check the player
					if (plynr == -1 || plynr == unit.Player->Index) {
						++s;
					}
				}
			}
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
		const int unitCount = Players[i].TotalNumUnits;

		// This player is our enemy and has units left.
		if ((Players[player].IsEnemy(Players[i])) || (Players[i].IsEnemy(Players[player]))) {
			// Don't count walls
			for (int j = 0; j < unitCount; ++j) {
				if (Players[i].Units[j]->Type->Wall == false) {
					++n;
					break;
				}
			}
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
	Trigger = trigger;
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
void TriggersEachCycle()
{
	int triggers;
	int base = lua_gettop(Lua);

	lua_pushstring(Lua, "_triggers_");
	lua_gettable(Lua, LUA_GLOBALSINDEX);
	triggers = lua_objlen(Lua, -1);

	if (Trigger >= triggers) {
		Trigger = 0;
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
		int currentTrigger = Trigger;
		Trigger += 2;
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
void TriggerCclRegister()
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
void SaveTriggers(CFile &file)
{
	file.printf("\n--- -----------------------------------------\n");
	file.printf("--- MODULE: triggers\n");

	file.printf("\n");
	lua_pushstring(Lua, "_triggers_");
	lua_gettable(Lua, LUA_GLOBALSINDEX);
	const int triggers = lua_objlen(Lua, -1);

	file.printf("SetActiveTriggers(");
	for (int i = 0; i < triggers; i += 2) {
		lua_rawgeti(Lua, -1, i + 1);
		if (i) {
			file.printf(", ");
		}
		if (!lua_isnil(Lua, -1)) {
			file.printf("true");
		} else {
			file.printf("false");
		}
		lua_pop(Lua, 1);
	}
	file.printf(")\n");

	file.printf("SetTrigger(%d)\n", Trigger);

	if (GameTimer.Init) {
		file.printf("ActionSetTimer(%ld, %s)\n",
			GameTimer.Cycles, (GameTimer.Increasing ? "true" : "false"));
		if (GameTimer.Running) {
			file.printf("ActionStartTimer()\n");
		}
	}

	file.printf("\n");
	file.printf("if (Triggers ~= nil) then assert(loadstring(Triggers))() end\n");
	file.printf("\n");
}

/**
**  Initialize the trigger module.
*/
void InitTriggers()
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
void CleanTriggers()
{
	lua_pushstring(Lua, "_triggers_");
	lua_pushnil(Lua);
	lua_settable(Lua, LUA_GLOBALSINDEX);

	Trigger = 0;

	delete[] ActiveTriggers;
	ActiveTriggers = NULL;

	GameTimer.Reset();
}

//@}
