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
//      (c) Copyright 2002-2015 by Lutz Sammer, Jimmy Salmon and Andrettin
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

#include "stratagus.h"

#include "trigger.h"

#include "interface.h"
#include "iolib.h"
#include "map.h"
#include "player.h"
#include "results.h"
#include "script.h"
#include "unit.h"
#include "unit_find.h"
#include "unittype.h"

#include <vector>

/*----------------------------------------------------------------------------
--  Variables
----------------------------------------------------------------------------*/

CTimer GameTimer;               /// The game timer
static std::vector<bool> ActiveTriggers;

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
**  @return   The unit-player validator.
*/
std::function<bool(const CUnit &)> TriggerGetPlayer(lua_State *l)
{
	if (lua_isnumber(l, -1)) {
		const int plynr = LuaToNumber(l, -1);
		if (plynr < 0 || plynr > PlayerMax) {
			LuaError(l, "bad player: %d", plynr);
		}
		return [plynr](const CUnit &unit) { return plynr == unit.Player->Index; };
	}
	const std::string_view player = LuaToString(l, -1);
	if (player == "any") {
		return [](const CUnit &) { return true; };
	} else if (player == "this") {
		Assert(ThisPlayer);
		return [plynr = ThisPlayer->Index](const CUnit &unit) { return plynr == unit.Player->Index; };
	}
	LuaError(l, "bad player: %s", player.data());
	ExitFatal(0);
}

/**
**  Get the unit-type.
**
**  @param l  Lua state.
**
**  @return   The unit-type validator.
*/
std::function<bool(const CUnit &)> TriggerGetUnitType(lua_State *l)
{
	const std::string_view unit = LuaToString(l, -1);

	if (unit == "any") {
		return [](const CUnit &) { return true; };
	} else if (unit == "units") {
		return [](const CUnit &unit) { return !unit.Type->Building; };
	} else if (unit == "buildings") {
		return [](const CUnit &unit) { return unit.Type->Building; };
	}
	const CUnitType *expectedType = CclGetUnitType(l);
	return [=](const CUnit &unit) { return expectedType == unit.Type && !unit.Constructed; };
}

/*--------------------------------------------------------------------------
--  Conditions
--------------------------------------------------------------------------*/
static int CompareEq(int a, int b) { return a == b; }
static int CompareNEq(int a, int b) { return a != b; }
static int CompareGrEq(int a, int b) { return a >= b; }
static int CompareGr(int a, int b) { return a > b; }
static int CompareLeEq(int a, int b) { return a <= b; }
static int CompareLe(int a, int b) { return a < b; }

using CompareFunction = int (*)(int, int);

/**
**  Returns a function pointer to the comparison function
**
**  @param op  The operation
**
**  @return    Function pointer to the compare function
*/
static CompareFunction GetCompareFunction(std::string_view op)
{
	if (op == "==" || op == "=") {
		return &CompareEq;
	} else if (op == ">=") {
		return &CompareGrEq;
	} else if (op == ">") {
		return &CompareGr;
	} else if (op == "<=") {
		return &CompareLeEq;
	} else if (op == "<") {
		return &CompareLe;
	} else if (op == "!=") {
		return &CompareNEq;
	}
	return nullptr;
}

/**
** <b>Description</b>
**
**  Return the number of units of a given unit-type and player at a location.
**
** Example:
**
** <div class="example"><code>-- Get the number of knights from player 1 from position 0,0 to 20,15
**			num_units = <strong>GetNumUnitsAt</strong>(1,"unit-knight",{0,0},{20,15})
**			print(num_units)</code></div>
*/
static int CclGetNumUnitsAt(lua_State *l)
{
	LuaCheckArgs(l, 4);

	lua_pushvalue(l, 1);
	const auto unitPlayerValidator = TriggerGetPlayer(l);
	lua_pushvalue(l, 2);
	auto unitValidator = TriggerGetUnitType(l);
	lua_pop(l, 1);

	Vec2i minPos;
	Vec2i maxPos;
	CclGetPos(l, &minPos, 3);
	CclGetPos(l, &maxPos, 4);

	if (minPos.x > maxPos.x) {
		std::swap(minPos.x, maxPos.x);
	}
	if (minPos.y > maxPos.y) {
		std::swap(minPos.y, maxPos.y);
	}

	std::vector<CUnit *> units = Select(minPos, maxPos);

	const int s = ranges::count_if(units, [&](const CUnit *unit) {
		return unitValidator(*unit) && unitPlayerValidator(*unit) && unit->IsAlive();
	});
	lua_pushnumber(l, s);
	return 1;
}

/**
** <b>Description</b>
**
**  Player has the quantity of unit-type near to unit-type.
**
** Example:
**
** <div class="example"><code>AddTrigger(
**    function() return <strong>IfNearUnit</strong>(0,">",1,"unit-peasant","unit-town-hall") end,
**    function()
**        AddMessage("Player 0 has more than 1 peasant near the Town Hall")
**        return false end
**	)</code></div>
*/
static int CclIfNearUnit(lua_State *l)
{
	LuaCheckArgs(l, 5);
	lua_pushvalue(l, 1);
	const auto unitPlayerValidator = TriggerGetPlayer(l);
	lua_pop(l, 1);
	const std::string_view op = LuaToString(l, 2);
	const int q = LuaToNumber(l, 3);
	lua_pushvalue(l, 4);
	const auto unitValidator = TriggerGetUnitType(l);
	lua_pop(l, 1);
	const CUnitType *ut2 = CclGetUnitType(l);
	if (!ut2) {
		LuaError(l, "CclIfNearUnit: not a unit-type valid");
	}
	CompareFunction compare = GetCompareFunction(op);
	if (!compare) {
		LuaError(l, "Illegal comparison operation in if-near-unit: %s", op.data());
	}

	//
	// Get all unit types 'near'.
	//

	std::vector<CUnit *> unitsOfType = FindUnitsByType(*ut2);
	for (const CUnit *centerUnit : unitsOfType) {
		std::vector<CUnit *> around = SelectAroundUnit(*centerUnit, 1);

		// Count the requested units
		const int s = ranges::count_if(around, [&](const CUnit *unit) {
			return unitValidator(*unit) && unitPlayerValidator(*unit);
		});
		if (compare(s, q)) {
			lua_pushboolean(l, 1);
			return 1;
		}
	}
	lua_pushboolean(l, 0);
	return 1;
}

/**
** <b>Description</b>
**
**  Player has the quantity of rescued unit-type near to unit-type.
**
** Example:
**
** <div class="example"><code><strong>IfRescuedNearUnit</strong>("this", ">=", 1, "unit-archer", "unit-circle-of-power")</code></div>
*/
static int CclIfRescuedNearUnit(lua_State *l)
{
	LuaCheckArgs(l, 5);

	lua_pushvalue(l, 1);
	const auto unitPlayerValidator = TriggerGetPlayer(l);
	lua_pop(l, 1);
	const std::string_view op = LuaToString(l, 2);
	const int q = LuaToNumber(l, 3);
	lua_pushvalue(l, 4);
	const auto unitValidator = TriggerGetUnitType(l);
	lua_pop(l, 1);
	const CUnitType *ut2 = CclGetUnitType(l);
	if (!ut2) {
		LuaError(l, "CclIfRescuedNearUnit: not a unit-type valid");
	}

	CompareFunction compare = GetCompareFunction(op);
	if (!compare) {
		LuaError(l, "Illegal comparison operation in if-rescued-near-unit: %s", op.data());
	}

	// Get all unit types 'near'.
	std::vector<CUnit *> table = FindUnitsByType(*ut2);
	for (CUnit *centerUnit : table) {
		std::vector<CUnit *> around = SelectAroundUnit(*centerUnit, 1);
		// Count the requested units
		const int s = ranges::count_if(around, [&](const CUnit *unit) {
			return unit->RescuedFrom && unitValidator(*unit) && unitPlayerValidator(*unit);
		});
		if (compare(s, q)) {
			lua_pushboolean(l, 1);
			return 1;
		}
	}
	lua_pushboolean(l, 0);
	return 1;
}

/**
** <b>Description</b>
**
**  Returns the number of opponents of a given player.
*/
int GetNumOpponents(int player)
{
	return ranges::count_if(Players, [&](const CPlayer &p) {
		// Check the player opponents
		// This player is our enemy and has units left.
		// Don't count walls
		return (p.IsEnemy(Players[player]) || Players[player].IsEnemy(p))
		    && ranges::any_of(p.GetUnits(), [](const CUnit *unit) {
				   return unit->Type->BoolFlag[WALL_INDEX].value == false;
			   });
	});
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
**  Remove a trigger
**
**  @param l     Lua state, top of stack is trigger table
**
**  @param trig  Index of trigger to be removed
*/
static void TriggerRemoveTrigger(lua_State *l, int trig)
{
	lua_pushnumber(l, -1);
	lua_rawseti(l, -2, trig * 2 + 1);
	lua_pushnumber(l, -1);
	lua_rawseti(l, -2, trig * 2 + 2);
}

/**
** <b>Description</b>
**
**  Add a trigger. A trigger is a set of two functions. The first is run every
**  cycle and checks a condition. Once it returns <code>true</code>, the second
**  function runs. If the second function returns <code>false</code>, the
**  trigger is removed and never runs again. Otherwise, it is kept and may run
**  again next cycle.
**
** Example:
**
** <div class="example"><code><strong>AddTrigger</strong>(
**			function() return (GetPlayerData(1,"UnitTypesCount","unit-farm") >= 4) end,
**			function() return ActionVictory() end
**		)</code></div>
*/
static int CclAddTrigger(lua_State *l)
{
	LuaCheckArgs(l, 2);
	if (!lua_isfunction(l, 1)
		|| (!lua_isfunction(l, 2) && !lua_istable(l, 2))) {
		LuaError(l, "incorrect argument");
	}

	// Make a list of all triggers.
	// A trigger is a pair of condition and action
	lua_getglobal(l, "_triggers_");

	if (lua_isnil(l, -1)) {
		LuaDebugPrint(l, "Trigger not set, defining trigger\n");
		lua_pop(l, 1);
		lua_newtable(l);
		lua_setglobal(l, "_triggers_");
		lua_getglobal(l, "_triggers_");
	}

	const int i = lua_rawlen(l, -1);
	if (i / 2 < ActiveTriggers.size() && !ActiveTriggers[i / 2]) {
		lua_pushnumber(l, -1);
		lua_rawseti(l, -2, i + 1);
		lua_pushnumber(l, -1);
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
void SetTrigger(int /*trigger*/)
{
	// legacy function
}

/**
**  Set the active triggers
*/
static int CclSetActiveTriggers(lua_State *l)
{
	const int args = lua_gettop(l);

	lua_getglobal(l, "_triggers_");
	const int triggerCount = lua_rawlen(l, -1) / 2;

	ActiveTriggers.resize(args);
	for (int j = 0; j < args; ++j) {
		ActiveTriggers[j] = LuaToBoolean(l, j + 1);
		if (j < triggerCount && !ActiveTriggers[j])
		{
			TriggerRemoveTrigger(l, j);
		}
	}

	lua_pop(l, 1);

	return 0;
}

/**
**  Execute a trigger action
**
**  @param l       Lua state, top of stack is trigger table
**
**  @param trig    Index of trigger to execute action from
**
**  @return        true if the trigger should be removed
*/
static bool TriggerExecuteAction(lua_State *l, int trig)
{
	const int base = lua_gettop(l);
	bool ret = false;

	lua_rawgeti(l, -1, trig * 2 + 2);
	const int args = lua_rawlen(l, -1);
	for (int j = 0; j < args; ++j) {
		lua_rawgeti(l, -1, j + 1);
		LuaCall(0, 0);
		ret = lua_gettop(l) > base + 1 && lua_toboolean(l, -1);
		lua_settop(l, base + 1);
	}
	lua_pop(l, 1);

	// If action returns false remove it
	return !ret;
}

/**
**  Check trigger each game cycle.
*/
void TriggersEachCycle()
{
	const int base = lua_gettop(Lua);

	lua_getglobal(Lua, "_triggers_");
	const int triggerSize = lua_rawlen(Lua, -1) / 2;

	for (int trigger = 0; trigger < triggerSize; ++trigger) {
		if (GamePaused) {
			break;
		}

		lua_rawgeti(Lua, -1, trigger * 2 + 1);
		if (!lua_isnumber(Lua, -1)) {
			LuaCall(0, 0);
			// If condition is true execute action
			if (lua_gettop(Lua) > base + 1 && lua_toboolean(Lua, -1)) {
				lua_settop(Lua, base + 1);
				if (TriggerExecuteAction(Lua, trigger)) {
					TriggerRemoveTrigger(Lua, trigger);
				}
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
	lua_getglobal(Lua, "_triggers_");
	const int triggers = lua_rawlen(Lua, -1);

	file.printf("SetActiveTriggers(");
	for (int i = 0; i < triggers; i += 2) {
		lua_rawgeti(Lua, -1, i + 1);
		if (i) {
			file.printf(", ");
		}
		if (!lua_isnumber(Lua, -1)) {
			file.printf("true");
		} else {
			file.printf("false");
		}
		lua_pop(Lua, 1);
	}
	file.printf(")\n");

	if (GameTimer.Init) {
		file.printf("ActionSetTimer(%ld, %s)\n",
					GameTimer.Cycles, (GameTimer.Increasing ? "true" : "false"));
		if (GameTimer.Running) {
			file.printf("ActionStartTimer()\n");
		}
	}

	file.printf("\n");
}

/**
**  Initialize the trigger module.
*/
void InitTriggers()
{
	// Setup default triggers

	// FIXME: choose the triggers for game type

	lua_getglobal(Lua, "_triggers_");
	if (lua_isnil(Lua, -1)) {
		lua_getglobal(Lua, "SinglePlayerTriggers");
		LuaCall(0, 1);
	}
	lua_pop(Lua, 1);
}

/**
**  Clean up the trigger module.
*/
void CleanTriggers()
{
	lua_pushnil(Lua);
	lua_setglobal(Lua, "_triggers_");

	lua_pushnil(Lua);
	lua_setglobal(Lua, "Triggers");

	ActiveTriggers.clear();

	GameTimer.Reset();
}

//@}
