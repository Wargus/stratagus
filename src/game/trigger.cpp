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
--  Includes
----------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <setjmp.h>

#include "stratagus.h"
#include "ccl.h"
#include "unittype.h"
#include "player.h"
#include "trigger.h"
#include "campaign.h"
#include "interface.h"

/*----------------------------------------------------------------------------
--  Declarations
----------------------------------------------------------------------------*/

	/// Get unit-type.
#if defined(USE_GUILE) || defined(USE_SIOD)
extern UnitType* CclGetUnitType(SCM ptr);
#elif defined(USE_LUA)
extern UnitType* CclGetUnitType(lua_State* l);
#endif

#define MAX_SWITCH 256 /// Maximum number of switches

/*----------------------------------------------------------------------------
--  Variables
----------------------------------------------------------------------------*/

global Timer GameTimer; /// The game timer
local unsigned long WaitFrame; /// Frame to wait for
#if defined(USE_GUILE) || defined(USE_SIOD)
local SCM Trigger;      /// Current trigger
local SCM WaitScript;   /// Script to run after wait is over
local SCM WaitTrigger;  /// Old Trigger value during wait
#elif defined(USE_LUA)
local int Trigger;
local int WaitScript;
local int WaitTrigger;
#endif
local unsigned char Switch[MAX_SWITCH]; /// Switches

/*----------------------------------------------------------------------------
--  Functions
----------------------------------------------------------------------------*/

/**
**  Get player number.
**
**  @param player  The player
**
**  @return  The player number, -1 matches any.
*/
#if defined(USE_GUILE) || defined(USE_SIOD)
global int TriggerGetPlayer(SCM player)
{
	int ret;

	if (gh_exact_p(player)) {
		ret = gh_scm2int(player);
		if (ret < 0 || ret > PlayerMax) {
			errl("bad player", player);
		}
		return ret;
	}
	if (gh_eq_p(player, gh_symbol2scm("any"))) {
		return -1;
	} else if (gh_eq_p(player, gh_symbol2scm("this"))) {
		return ThisPlayer->Player;
	}
	errl("bad player", player);

	return 0;
}
#elif defined(USE_LUA)
global int TriggerGetPlayer(lua_State* l)
{
	const char* player;
	int ret;

	if (lua_isnumber(l, -1)) {
		ret = LuaToNumber(l, -1);
		if (ret < 0 || ret > PlayerMax) {
			lua_pushfstring(l, "bad player: %d", ret);
			lua_error(l);
		}
		return ret;
	}
	player = LuaToString(l, -1);
	if (!strcmp(player, "any")) {
		return -1;
	} else if (!strcmp(player, "this")) {
		return ThisPlayer->Player;
	}
	lua_pushfstring(l, "bad player: %s", player);
	lua_error(l);

	return 0;
}
#endif

/**
**  Get the unit-type.
**
**  @param unit  The unit type.
**
**  @return      The unit-type pointer.
*/
#if defined(USE_GUILE) || defined(USE_SIOD)
global const UnitType* TriggerGetUnitType(SCM unit)
{
	if (gh_eq_p(unit, gh_symbol2scm("any"))) {
		return ANY_UNIT;
	} else if (gh_eq_p(unit, gh_symbol2scm("all"))) {
		return ALL_UNITS;
	} else if (gh_eq_p(unit, gh_symbol2scm("units"))) {
		return ALL_FOODUNITS;
	} else if (gh_eq_p(unit, gh_symbol2scm("buildings"))) {
		return ALL_BUILDINGS;
	}

	return CclGetUnitType(unit);
}
#elif defined(USE_LUA)
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
#endif

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
#if defined(USE_GUILE) || defined(USE_SIOD)
local SCM CclIfUnit(SCM player, SCM operation, SCM quantity, SCM unit)
{
	int plynr;
	int q;
	int pn;
	const UnitType* unittype;
	const char* op;
	CompareFunction compare;

	plynr = TriggerGetPlayer(player);
	op = get_c_string(operation);
	q = gh_scm2int(quantity);
	unittype = TriggerGetUnitType(unit);

	compare = GetCompareFunction(op);
	if (!compare) {
		errl("Illegal comparison operation in if-unit", operation);
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
					return SCM_BOOL_T;
				}
			}
		}
	} else if (unittype == ALL_UNITS) {
		for (; plynr < pn; ++plynr) {
			if (compare(Players[plynr].TotalNumUnits, q)) {
				return SCM_BOOL_T;
			}
		}
	} else if (unittype == ALL_FOODUNITS) {
		for (; plynr < pn; ++plynr) {
			if (compare(Players[plynr].TotalNumUnits - Players[plynr].NumBuildings, q)) {
				return SCM_BOOL_T;
			}
		}
	} else if (unittype == ALL_BUILDINGS) {
		for (; plynr < pn; ++plynr) {
			if (compare(Players[plynr].NumBuildings, q)) {
				return SCM_BOOL_T;
			}
		}
	} else {
		for (; plynr < pn; ++plynr) {
			DebugLevel3Fn("Player%d, %d == %s\n" _C_ plynr _C_ q _C_ unittype->Ident);
			if (compare(Players[plynr].UnitTypesCount[unittype->Type], q)) {
				return SCM_BOOL_T;
			}
		}
	}

	return SCM_BOOL_F;
}
#elif defined(USE_LUA)
local int CclIfUnit(lua_State* l)
{
	int plynr;
	int q;
	int pn;
	const UnitType* unittype;
	const char* op;
	CompareFunction compare;

	if (lua_gettop(l) != 4) {
		lua_pushstring(l, "incorrect argument");
		lua_error(l);
	}

	lua_pushvalue(l, 1);
	plynr = TriggerGetPlayer(l);
	lua_pop(l, 1);
	op = LuaToString(l, 2);
	q = LuaToNumber(l, 3);
	unittype = TriggerGetUnitType(l);

	compare = GetCompareFunction(op);
	if (!compare) {
		lua_pushfstring(l, "Illegal comparison operation in if-unit: %s", op);
		lua_error(l);
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
			if (compare(Players[plynr].UnitTypesCount[unittype->Type], q)) {
				lua_pushboolean(l, 1);
				return 1;
			}
		}
	}

	lua_pushboolean(l, 0);
	return 1;
}
#endif

/**
**  Player has the quantity of unit-type at a location.
**
**  (if-unit-at {player} {op} {quantity} {unit} {location} {location})
*/
#if defined(USE_GUILE) || defined(USE_SIOD)
local SCM CclIfUnitAt(SCM list)
{
	int plynr;
	int q;
	int x1;
	int y1;
	int x2;
	int y2;
	const UnitType* unittype;
	CompareFunction compare;
	Unit* table[UnitMax];
	Unit* unit;
	int an;
	int j;
	int s;

	plynr = TriggerGetPlayer(gh_car(list));
	list = gh_cdr(list);
	compare = GetCompareFunction(get_c_string(gh_car(list)));
	if (!compare) {
		errl("Illegal comparison operator in if-unit-at", gh_car(list));
	}
	list = gh_cdr(list);
	q = gh_scm2int(gh_car(list));
	list = gh_cdr(list);
	unittype = TriggerGetUnitType(gh_car(list));
	list = gh_cdr(list);
	x1 = gh_scm2int(gh_car(gh_car(list)));
	y1 = gh_scm2int(gh_car(gh_cdr(gh_car(list))));
	list = gh_cdr(list);
	x2 = gh_scm2int(gh_car(gh_car(list)));
	y2 = gh_scm2int(gh_car(gh_cdr(gh_car(list))));
	list = gh_cdr(list);

	//
	// Get all unit types in location.
	//
#ifdef UNIT_ON_MAP
	// FIXME: could be done faster?
#endif
	// FIXME: I hope SelectUnits checks bounds?
	// FIXME: Yes, but caller should check.
	// NOTE: +1 right,bottom isn't inclusive :(
	an = SelectUnits(x1, y1, x2 + 1, y2 + 1, table);
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
		return SCM_BOOL_T;
	}

	return SCM_BOOL_F;
}
#elif defined(USE_LUA)
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
		lua_pushstring(l, "incorrect argument");
		lua_error(l);
	}

	lua_pushvalue(l, 1);
	plynr = TriggerGetPlayer(l);
	lua_pop(l, 1);
	op = LuaToString(l, 2);
	compare = GetCompareFunction(op);
	if (!compare) {
		lua_pushfstring(l, "Illegal comparison operator in if-unit-at: %s", op);
		lua_error(l);
	}
	q = LuaToNumber(l, 3);
	lua_pushvalue(l, 1);
	unittype = TriggerGetUnitType(l);
	lua_pop(l, 1);
	if (!lua_istable(l, 5) || luaL_getn(l, 5) != 2) {
		lua_pushstring(l, "incorrect argument");
		lua_error(l);
	}
	lua_rawgeti(l, 5, 1);
	x1 = LuaToNumber(l, -1);
	lua_pop(l, 1);
	lua_rawgeti(l, 5, 2);
	y1 = LuaToNumber(l, -1);
	lua_pop(l, 1);
	if (!lua_istable(l, 6) || luaL_getn(l, 6) != 2) {
		lua_pushstring(l, "incorrect argument");
		lua_error(l);
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
#ifdef UNIT_ON_MAP
	// FIXME: could be done faster?
#endif
	// FIXME: I hope SelectUnits checks bounds?
	// FIXME: Yes, but caller should check.
	// NOTE: +1 right,bottom isn't inclusive :(
	an = SelectUnits(x1, y1, x2 + 1, y2 + 1, table);
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
#endif

/**
**  Player has the quantity of unit-type near to unit-type.
*/
#if defined(USE_GUILE) || defined(USE_SIOD)
local SCM CclIfNearUnit(SCM player, SCM operation, SCM quantity, SCM unit,
	SCM nearunit)
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

	plynr = TriggerGetPlayer(player);
	op = get_c_string(operation);
	q = gh_scm2int(quantity);
	unittype = TriggerGetUnitType(unit);
	ut2 = CclGetUnitType(nearunit);

	compare = GetCompareFunction(op);
	if (!compare) {
		errl("Illegal comparison operation in if-near-unit", operation);
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

#ifdef UNIT_ON_MAP
		// FIXME: could be done faster?
#endif
		// FIXME: I hope SelectUnits checks bounds?
		// FIXME: Yes, but caller should check.
		// NOTE: +1 right,bottom isn't inclusive :(
		if (unit->Type->UnitType == UnitTypeLand) {
			an = SelectUnits(unit->X - 1, unit->Y - 1,
				unit->X + unit->Type->TileWidth + 1,
				unit->Y + unit->Type->TileHeight + 1, around);
		} else {
			an = SelectUnits(unit->X - 2, unit->Y - 2,
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
			return SCM_BOOL_T;
		}
	}

	return SCM_BOOL_F;
}
#elif defined(USE_LUA)
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

	if (lua_gettop(l) != 4) {
		lua_pushstring(l, "incorrect argument");
		lua_error(l);
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
		lua_pushfstring(l, "Illegal comparison operation in if-near-unit: %s", op);
		lua_error(l);
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

#ifdef UNIT_ON_MAP
		// FIXME: could be done faster?
#endif
		// FIXME: I hope SelectUnits checks bounds?
		// FIXME: Yes, but caller should check.
		// NOTE: +1 right,bottom isn't inclusive :(
		if (unit->Type->UnitType == UnitTypeLand) {
			an = SelectUnits(unit->X - 1, unit->Y - 1,
				unit->X + unit->Type->TileWidth + 1,
				unit->Y + unit->Type->TileHeight + 1, around);
		} else {
			an = SelectUnits(unit->X - 2, unit->Y - 2,
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
#endif

/**
** Player has the quantity of rescued unit-type near to unit-type.
*/
#if defined(USE_GUILE) || defined(USE_SIOD)
local SCM CclIfRescuedNearUnit(SCM player, SCM operation, SCM quantity, SCM unit,
	SCM nearunit)
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

	plynr = TriggerGetPlayer(player);
	op = get_c_string(operation);
	q = gh_scm2int(quantity);
	unittype = TriggerGetUnitType(unit);
	ut2 = CclGetUnitType(nearunit);

	compare = GetCompareFunction(op);
	if (!compare) {
		errl("Illegal comparison operation in if-rescued-near-unit", operation);
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

#ifdef UNIT_ON_MAP
		// FIXME: could be done faster?
#endif
		// FIXME: I hope SelectUnits checks bounds?
		// FIXME: Yes, but caller should check.
		// NOTE: +1 right,bottom isn't inclusive :(
		if (unit->Type->UnitType == UnitTypeLand) {
			an = SelectUnits(unit->X - 1, unit->Y - 1,
				unit->X + unit->Type->TileWidth + 1,
				unit->Y + unit->Type->TileHeight + 1, around);
		} else {
			an = SelectUnits(unit->X - 2, unit->Y - 2,
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
			return SCM_BOOL_T;
		}
	}

	return SCM_BOOL_F;
}
#elif defined(USE_LUA)
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
		lua_pushstring(l, "incorrect argument");
		lua_error(l);
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
		lua_pushfstring(l, "Illegal comparison operation in if-rescued-near-unit: %s", op);
		lua_error(l);
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

#ifdef UNIT_ON_MAP
		// FIXME: could be done faster?
#endif
		// FIXME: I hope SelectUnits checks bounds?
		// FIXME: Yes, but caller should check.
		// NOTE: +1 right,bottom isn't inclusive :(
		if (unit->Type->UnitType == UnitTypeLand) {
			an = SelectUnits(unit->X - 1, unit->Y - 1,
				unit->X + unit->Type->TileWidth + 1,
				unit->Y + unit->Type->TileHeight + 1, around);
		} else {
			an = SelectUnits(unit->X - 2, unit->Y - 2,
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
#endif

/**
**  Player has n opponents left.
*/
#if defined(USE_GUILE) || defined(USE_SIOD)
local SCM CclIfOpponents(SCM player, SCM operation, SCM quantity)
{
	int plynr;
	int q;
	int pn;
	int n;
	const char* op;
	CompareFunction compare;

	plynr = TriggerGetPlayer(player);
	op = get_c_string(operation);
	q = gh_scm2int(quantity);

	compare = GetCompareFunction(op);
	if (!compare) {
		errl("Illegal comparison operation in if-opponents", operation);
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
			return SCM_BOOL_T;
		}
	}

	return SCM_BOOL_F;
}
#elif defined(USE_LUA)
local int CclIfOpponents(lua_State* l)
{
	int plynr;
	int q;
	int pn;
	int n;
	const char* op;
	CompareFunction compare;

	if (lua_gettop(l) != 3) {
		lua_pushstring(l, "incorrect argument");
		lua_error(l);
	}

	lua_pushvalue(l, 1);
	plynr = TriggerGetPlayer(l);
	lua_pop(l, 1);
	op = LuaToString(l, 2);
	q = LuaToNumber(l, 3);

	compare = GetCompareFunction(op);
	if (!compare) {
		lua_pushfstring(l, "Illegal comparison operation in if-opponents: %s", op);
		lua_error(l);
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
#endif

/**
**  Player has the quantity of resource.
*/
#if defined(USE_GUILE) || defined(USE_SIOD)
local SCM CclIfResource(SCM player, SCM operation, SCM quantity, SCM resource)
{
	int plynr;
	int q;
	int pn;
	const char* res;
	const char* op;
	CompareFunction compare;
	int i;

	plynr = TriggerGetPlayer(player);
	op = get_c_string(operation);
	q = gh_scm2int(quantity);
	res = get_c_string(resource);

	compare = GetCompareFunction(op);
	if (!compare) {
		errl("Illegal comparison operation in if-resource", operation);
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
					return SCM_BOOL_T;
				}
			}
			return SCM_BOOL_F;
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
			return SCM_BOOL_T;
		}
	} else if (!strcmp(res, "any")) {
		int j;

		for (; plynr < pn; ++plynr) {
			for (j = 1; j < MaxCosts; ++j) {
				if (compare(Players[plynr].Resources[j], q)) {
					return SCM_BOOL_T;
				}
			}
		}
	}

	return SCM_BOOL_F;
}
#elif defined(USE_LUA)
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
		lua_pushstring(l, "incorrect argument");
		lua_error(l);
	}

	lua_pushvalue(l, 1);
	plynr = TriggerGetPlayer(l);
	lua_pop(l, 1);
	op = LuaToString(l, 2);
	q = LuaToNumber(l, 3);
	res = LuaToString(l, 4);

	compare = GetCompareFunction(op);
	if (!compare) {
		lua_pushfstring(l, "Illegal comparison operation in if-resource: %s", op);
		lua_error(l);
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
#endif

/**
**  Player has quantity kills
*/
#if defined(USE_GUILE) || defined(USE_SIOD)
local SCM CclIfKills(SCM player, SCM operation, SCM quantity)
{
	int plynr;
	int q;
	int pn;
	int n;
	const char* op;
	CompareFunction compare;

	plynr = TriggerGetPlayer(player);
	op = get_c_string(operation);
	q = gh_scm2int(quantity);

	compare = GetCompareFunction(op);
	if (!compare) {
		errl("Illegal comparison operation in if-kills", operation);
	}

	if (plynr == -1) {
		plynr = 0;
		pn = PlayerMax;
	} else {
		pn = plynr + 1;
	}

	for (n = 0; plynr < pn; ++plynr) {
		if (compare(Players[plynr].TotalKills, q)) {
			return SCM_BOOL_T;
		}
	}

	return SCM_BOOL_F;
}
#elif defined(USE_LUA)
local int CclIfKills(lua_State* l)
{
	int plynr;
	int q;
	int pn;
	int n;
	const char* op;
	CompareFunction compare;

	if (lua_gettop(l) != 3) {
		lua_pushstring(l, "incorrect argument");
		lua_error(l);
	}

	lua_pushvalue(l, 1);
	plynr = TriggerGetPlayer(l);
	lua_pop(l, 1);
	op = LuaToString(l, 2);
	q = LuaToNumber(l, 3);

	compare = GetCompareFunction(op);
	if (!compare) {
		lua_pushfstring(l, "Illegal comparison operation in if-kills: %s", op);
		lua_error(l);
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
#endif

/**
**  Player has a certain score
*/
#if defined(USE_GUILE) || defined(USE_SIOD)
local SCM CclIfScore(SCM player, SCM operation, SCM quantity)
{
	int plynr;
	int q;
	int pn;
	int n;
	const char* op;
	CompareFunction compare;

	plynr = TriggerGetPlayer(player);
	op = get_c_string(operation);
	q = gh_scm2int(quantity);

	compare = GetCompareFunction(op);
	if (!compare) {
		errl("Illegal comparison operation in if-score", operation);
	}

	if (plynr == -1) {
		plynr = 0;
		pn = PlayerMax;
	} else {
		pn = plynr + 1;
	}

	for (n = 0; plynr < pn; ++plynr) {
		if (compare(Players[plynr].Score, q)) {
			return SCM_BOOL_T;
		}
	}

	return SCM_BOOL_F;
}
#elif defined(USE_LUA)
local int CclIfScore(lua_State* l)
{
	int plynr;
	int q;
	int pn;
	int n;
	const char* op;
	CompareFunction compare;

	if (lua_gettop(l) != 3) {
		lua_pushstring(l, "incorrect argument");
		lua_error(l);
	}

	lua_pushvalue(l, 1);
	plynr = TriggerGetPlayer(l);
	lua_pop(l, 1);
	op = LuaToString(l, 2);
	q = LuaToNumber(l, 3);

	compare = GetCompareFunction(op);
	if (!compare) {
		lua_pushfstring(l, "Illegal comparison operation in if-score: %s", op);
		lua_error(l);
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
#endif

/**
**  Number of game cycles elapsed
*/
#if defined(USE_GUILE) || defined(USE_SIOD)
local SCM CclIfElapsed(SCM operation, SCM quantity)
{
	int q;
	const char* op;
	CompareFunction compare;

	op = get_c_string(operation);
	q = gh_scm2int(quantity);

	compare = GetCompareFunction(op);
	if (!compare) {
		errl("Illegal comparison operation in if-elapsed", operation);
	}

	if (compare(GameCycle, q)) {
		return SCM_BOOL_T;
	}

	return SCM_BOOL_F;
}
#elif defined(USE_LUA)
local int CclIfElapsed(lua_State* l)
{
	int q;
	const char* op;
	CompareFunction compare;

	if (lua_gettop(l) != 2) {
		lua_pushstring(l, "incorrect argument");
		lua_error(l);
	}

	op = LuaToString(l, 1);
	q = LuaToNumber(l, 2);

	compare = GetCompareFunction(op);
	if (!compare) {
		lua_pushfstring(l, "Illegal comparison operation in if-elapsed: %s", op);
		lua_error(l);
	}

	if (compare(GameCycle, q)) {
		lua_pushboolean(l, 1);
		return 1;
	}

	lua_pushboolean(l, 0);
	return 1;
}
#endif

/**
**  Check the timer value
*/
#if defined(USE_GUILE) || defined(USE_SIOD)
local SCM CclIfTimer(SCM operation, SCM quantity)
{
	int q;
	const char* op;
	CompareFunction compare;

	if (!GameTimer.Init) {
		return SCM_BOOL_F;
	}

	op = get_c_string(operation);
	q = gh_scm2int(quantity);

	compare = GetCompareFunction(op);
	if (!compare) {
		errl("Illegal comparison operation in if-timer", operation);
	}

	if (compare(GameTimer.Cycles, q)) {
		return SCM_BOOL_T;
	}

	return SCM_BOOL_F;
}
#elif defined(USE_LUA)
local int CclIfTimer(lua_State* l)
{
	int q;
	const char* op;
	CompareFunction compare;

	if (lua_gettop(l) != 2) {
		lua_pushstring(l, "incorrect argument");
		lua_error(l);
	}

	if (!GameTimer.Init) {
		lua_pushboolean(l, 0);
		return 1;
	}

	op = LuaToString(l, 1);
	q = LuaToNumber(l, 2);

	compare = GetCompareFunction(op);
	if (!compare) {
		lua_pushfstring(l, "Illegal comparison operation in if-timer: %s", op);
		lua_error(l);
	}

	if (compare(GameTimer.Cycles, q)) {
		lua_pushboolean(l, 1);
		return 1;
	}

	lua_pushboolean(l, 0);
	return 1;
}
#endif

/**
**  Check the switch value
*/
#if defined(USE_GUILE) || defined(USE_SIOD)
local SCM CclIfSwitch(SCM number, SCM set)
{
	int i;
	unsigned char s;

	i = gh_scm2int(number);
	if (i < 0 || i >= MAX_SWITCH) {
		errl("Invalid switch number", number);
	}

	if (gh_boolean_p(set)) {
		s = gh_scm2bool(set);
	} else {
		s = gh_scm2int(set);
		if (s) {
			s = 1;
		}
	}

	if (Switch[i] == s) {
		return SCM_BOOL_T;
	}
	return SCM_BOOL_F;
}
#elif defined(USE_LUA)
local int CclIfSwitch(lua_State* l)
{
	int i;
	unsigned char s;

	if (lua_gettop(l) != 2) {
		lua_pushstring(l, "incorrect argument");
		lua_error(l);
	}

	i = LuaToNumber(l, 1);
	if (i < 0 || i >= MAX_SWITCH) {
		lua_pushfstring(l, "Invalid switch number", i);
		lua_error(l);
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
#endif

/*---------------------------------------------------------------------------
--		Actions
---------------------------------------------------------------------------*/
/**
**  Action condition player wins.
*/
#if defined(USE_GUILE) || defined(USE_SIOD)
local SCM CclActionVictory(void)
{
	GameResult = GameVictory;
	GamePaused = 1;
	GameRunning = 0;
	return SCM_UNSPECIFIED;
}
#elif defined(USE_LUA)
local int CclActionVictory(lua_State* l)
{
	if (lua_gettop(l) != 0) {
		lua_pushstring(l, "incorrect argument");
		lua_error(l);
	}

	GameResult = GameVictory;
	GamePaused = 1;
	GameRunning = 0;
	return 0;
}
#endif

/**
**  Action condition player lose.
*/
#if defined(USE_GUILE) || defined(USE_SIOD)
local SCM CclActionDefeat(void)
{
	GameResult = GameDefeat;
	GamePaused = 1;
	GameRunning = 0;
	return SCM_UNSPECIFIED;
}
#elif defined(USE_LUA)
local int CclActionDefeat(lua_State* l)
{
	if (lua_gettop(l) != 0) {
		lua_pushstring(l, "incorrect argument");
		lua_error(l);
	}

	GameResult = GameDefeat;
	GamePaused = 1;
	GameRunning = 0;
	return 0;
}
#endif

/**
**  Action condition player draw.
*/
#if defined(USE_GUILE) || defined(USE_SIOD)
local SCM CclActionDraw(void)
{
	GameResult = GameDraw;
	GamePaused = 1;
	GameRunning = 0;
	return SCM_UNSPECIFIED;
}
#elif defined(USE_LUA)
local int CclActionDraw(lua_State* l)
{
	if (lua_gettop(l) != 0) {
		lua_pushstring(l, "incorrect argument");
		lua_error(l);
	}

	GameResult = GameDraw;
	GamePaused = 1;
	GameRunning = 0;
	return 0;
}
#endif

/**
**  Action set timer
*/
#if defined(USE_GUILE) || defined(USE_SIOD)
local SCM CclActionSetTimer(SCM cycles, SCM increasing)
{
	GameTimer.Cycles = gh_scm2int(cycles);
	GameTimer.Increasing = gh_scm2int(increasing);
	GameTimer.Init = 1;
	GameTimer.LastUpdate = GameCycle;

	return SCM_UNSPECIFIED;
}
#elif defined(USE_LUA)
local int CclActionSetTimer(lua_State* l)
{
	if (lua_gettop(l) != 2) {
		lua_pushstring(l, "incorrect argument");
		lua_error(l);
	}

	GameTimer.Cycles = LuaToNumber(l, 1);
	GameTimer.Increasing = LuaToNumber(l, 2);
	GameTimer.Init = 1;
	GameTimer.LastUpdate = GameCycle;

	return 0;
}
#endif

/**
**  Action start timer
*/
#if defined(USE_GUILE) || defined(USE_SIOD)
local SCM CclActionStartTimer(void)
{
	GameTimer.Running = 1;
	GameTimer.Init = 1;
	return SCM_UNSPECIFIED;
}
#elif defined(USE_LUA)
local int CclActionStartTimer(lua_State* l)
{
	if (lua_gettop(l) != 0) {
		lua_pushstring(l, "incorrect argument");
		lua_error(l);
	}

	GameTimer.Running = 1;
	GameTimer.Init = 1;
	return 0;
}
#endif

/**
**  Action stop timer
*/
#if defined(USE_GUILE) || defined(USE_SIOD)
local SCM CclActionStopTimer(void)
{
	GameTimer.Running = 0;
	return SCM_UNSPECIFIED;
}
#elif defined(USE_LUA)
local int CclActionStopTimer(lua_State* l)
{
	if (lua_gettop(l) != 0) {
		lua_pushstring(l, "incorrect argument");
		lua_error(l);
	}

	GameTimer.Running = 0;
	return 0;
}
#endif

/**
**  Action wait
*/
#if defined(USE_GUILE) || defined(USE_SIOD)
local SCM CclActionWait(SCM ms)
{
	WaitFrame = FrameCounter +
		(FRAMES_PER_SECOND * VideoSyncSpeed / 100 * gh_scm2int(ms) + 999) / 1000;
	return SCM_UNSPECIFIED;
}
#elif defined(USE_LUA)
local int CclActionWait(lua_State* l)
{
	if (lua_gettop(l) != 1) {
		lua_pushstring(l, "incorrect argument");
		lua_error(l);
	}

	WaitFrame = FrameCounter +
		(FRAMES_PER_SECOND * VideoSyncSpeed / 100 * (int)LuaToNumber(l, 1) + 999) / 1000;
	return 0;
}
#endif

/**
**  Action stop timer
*/
#if defined(USE_GUILE) || defined(USE_SIOD)
local SCM CclActionSetSwitch(SCM number, SCM set)
{
	int i;
	unsigned char s;

	i = gh_scm2int(number);
	if (i < 0 || i >= MAX_SWITCH) {
		errl("Invalid switch number", number);
	}

	if (gh_boolean_p(set)) {
		s = gh_scm2bool(set);
	} else {
		s = gh_scm2int(set);
		if (s) {
			s = 1;
		}
	}

	Switch[i] = s;
	return set;
}
#elif defined(USE_LUA)
local int CclActionSetSwitch(lua_State* l)
{
	int i;
	unsigned char s;

	if (lua_gettop(l) != 2) {
		lua_pushstring(l, "incorrect argument");
		lua_error(l);
	}

	i = LuaToNumber(l, 1);
	if (i < 0 || i >= MAX_SWITCH) {
		lua_pushfstring(l, "Invalid switch number: %d", i);
		lua_error(l);
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
#endif

/**
**  Add a trigger.
*/
#if defined(USE_GUILE) || defined(USE_SIOD)
local SCM CclAddTrigger(SCM condition, SCM action)
{
	SCM var;

	//
	// Make a list of all triggers.
	// A trigger is a pair of condition and action
	//
	var = gh_symbol2scm("*triggers*");

	if (gh_null_p(symbol_value(var, NIL))) {
		puts("Trigger not set, defining trigger");
		setvar(var, cons(cons(condition, action), NIL), NIL);
	} else {
		// Search for the last element in the list
		var = symbol_value(var, NIL);
		while(!gh_null_p(gh_cdr(var))) {
			var = gh_cdr(var);
		}
		gh_set_cdr_x(var, cons(cons(condition, action), NIL));
	}

	return SCM_UNSPECIFIED;
}
#elif defined(USE_LUA)
local int CclAddTrigger(lua_State* l)
{
	int i;
	const char* str;
	int args;
	int j;

	if (lua_gettop(l) != 2 || !lua_isfunction(l, 1) ||
			(!lua_isfunction(l, 2) && !lua_istable(l, 2))) {
		lua_pushstring(l, "incorrect argument");
		lua_error(l);
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
	lua_pushvalue(l, 1);
	lua_rawseti(l, -2, i + 1);
	lua_newtable(l);
	if (lua_isfunction(l, 2)) {
		lua_pushvalue(l, 2);
		lua_rawseti(l, -2, 1);
	} else {
		args = luaL_getn(l, 2);
		for (j = 0; j < args; ++j) {
			lua_rawgeti(l, 2, j + 1);
			str = LuaToString(l, -1);
			lua_pop(l, 1);
			luaL_loadbuffer(l, str, strlen(str), str);
			lua_rawseti(l, -2, j + 1);
		}
	}
	lua_rawseti(l, -2, i + 2);
	lua_pop(l, 1);

	return 0;
}
#endif

/**
**  Set the current trigger number
**
**  @param number  Trigger number
*/
#if defined(USE_GUILE) || defined(USE_SIOD)
local SCM CclSetTriggerNumber(SCM number)
{
	int num;
	int i;

	num = gh_scm2int(number);
	if (num == -1) {
		CclGcProtectedAssign(&Trigger, NULL);
	} else {
		CclGcProtectedAssign(&Trigger, symbol_value(gh_symbol2scm("*triggers*"), NIL));
		if (gh_null_p(Trigger)) {
			DebugLevel0Fn("Invalid trigger number: %d out of -1\n" _C_ num);
		} else {
			for (i = 0; i < num; ++i) {
				if (gh_null_p(Trigger)) {
					DebugLevel0Fn("Invalid trigger number: %d out of %d\n" _C_
						num _C_ i - 1);
					break;
				}
				CclGcProtectedAssign(&Trigger, gh_cdr(Trigger));
			}
		}
	}

	return SCM_UNSPECIFIED;
}
#elif defined(USE_LUA)
#endif

/**
**  Execute a trigger action
**
**  @param script  Script to execute
**
**  @return        1 if the trigger should be removed
*/
#if defined(USE_GUILE) || defined(USE_SIOD)
local int TriggerExecuteAction(SCM script)
{
	SCM value;

	value = NULL;

	while (!gh_null_p(script)) {
		value = gh_eval(gh_car(script), NIL);
		script = gh_cdr(script);
		if (WaitFrame > FrameCounter) {
			CclGcProtectedAssign(&WaitScript, script);
			return 0;
		}
	}

	// If action returns false remove it
	if (gh_null_p(value)) {
		return 1;
	}
	return 0;
}
#elif defined(USE_LUA)
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
			WaitScript = script;
			lua_pop(Lua, 1);
			return 0;
		}
	}
	lua_pop(Lua, 1);

	// If action returns false remove it
	return !ret;
}
#endif

/**
**  Remove a trigger
**
**  @param trig  Current trigger
*/
#if defined(USE_GUILE) || defined(USE_SIOD)
local void TriggerRemoveTrigger(SCM trig)
{
	if (!gh_null_p(Trigger)) {
		gh_set_car_x(trig, gh_car(Trigger));
		gh_set_cdr_x(trig, gh_cdr(Trigger));
	} else {
		gh_set_car_x(trig, NIL);
		gh_set_cdr_x(trig, NIL);
	}
	CclGcProtectedAssign(&Trigger, trig);
}
#elif defined(USE_LUA)
local void TriggerRemoveTrigger(int trig)
{
	lua_pushnil(Lua);
	lua_rawseti(Lua, -2, trig);
	lua_pushnil(Lua);
	lua_rawseti(Lua, -2, trig + 1);
}
#endif

/**
**  Check trigger each game cycle.
*/
global void TriggersEachCycle(void)
{
#if defined(USE_GUILE) || defined(USE_SIOD)
	SCM pair;
	SCM trig;
	SCM value;
	SCM script;

	if (!Trigger) {
		CclGcProtectedAssign(&Trigger, symbol_value(gh_symbol2scm("*triggers*"), NIL));
	}
	trig = Trigger;

	if (WaitFrame > FrameCounter) {
		return;
	}
	if (WaitFrame && WaitFrame <= FrameCounter) {
		WaitFrame = 0;
		if (TriggerExecuteAction(WaitScript)) {
			TriggerRemoveTrigger(WaitTrigger);
		}
		return;
	}

	if (GamePaused) {
		return;
	}

	if (!gh_null_p(trig)) { // Next trigger
		pair = gh_car(trig);
		CclGcProtectedAssign(&Trigger, gh_cdr(trig));
		CclGcProtectedAssign(&WaitTrigger, trig);
		// Pair is condition action
		if (!gh_null_p(pair)) {
			script = gh_car(pair);
			value = NULL;
			while (!gh_null_p(script)) {
				value = gh_eval(gh_car(script), NIL);
				script = gh_cdr(script);
			}
			// If condition is true execute action
			if (value != SCM_BOOL_F) {
				if (TriggerExecuteAction(gh_cdr(pair))) {
					TriggerRemoveTrigger(trig);
				}
			}
		}
	} else {
		CclGcProtectedAssign(&Trigger, NULL);
	}
#elif defined(USE_LUA)
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
		if (TriggerExecuteAction(WaitScript)) {
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
		if (!lua_isnil(Lua, -1)) {
			break;
		}
		Trigger += 2;
	}
	if (Trigger < triggers) {
		int top;

		WaitTrigger = Trigger;
		Trigger += 2;
		top = lua_gettop(Lua);
		LuaCall(0, 0);
		// If condition is true execute action
		top = lua_gettop(Lua);
		if (lua_gettop(Lua) > 1 && lua_toboolean(Lua, -1)) {
			lua_settop(Lua, 1);
			if (TriggerExecuteAction(WaitTrigger + 1)) {
				TriggerRemoveTrigger(WaitTrigger);
			}
		}
		lua_settop(Lua, 1);
	}
	lua_pop(Lua, 1);
#endif
}

/**
**  Register CCL features for triggers.
*/
global void TriggerCclRegister(void)
{
#if defined(USE_GUILE) || defined(USE_SIOD)
	Trigger = NIL;
	WaitScript = NIL;
	WaitTrigger  = NIL;
	CclGcProtect(&Trigger);
	CclGcProtect(&WaitScript);
	CclGcProtect(&WaitTrigger);
	gh_new_procedure2_0("add-trigger", CclAddTrigger);
	gh_new_procedure1_0("set-trigger-number!", CclSetTriggerNumber);
	// Conditions
	gh_new_procedure4_0("if-unit", CclIfUnit);
	gh_new_procedureN("if-unit-at", CclIfUnitAt);
	gh_new_procedure5_0("if-near-unit", CclIfNearUnit);
	gh_new_procedure5_0("if-rescued-near-unit", CclIfRescuedNearUnit);
	gh_new_procedure3_0("if-opponents", CclIfOpponents);
	gh_new_procedure4_0("if-resource", CclIfResource);
	gh_new_procedure3_0("if-kills", CclIfKills);
	gh_new_procedure3_0("if-score", CclIfScore);
	gh_new_procedure2_0("if-elapsed", CclIfElapsed);
	gh_new_procedure2_0("if-timer", CclIfTimer);
	gh_new_procedure2_0("if-switch", CclIfSwitch);
	// Actions
	gh_new_procedure0_0("action-victory", CclActionVictory);
	gh_new_procedure0_0("action-defeat", CclActionDefeat);
	gh_new_procedure0_0("action-draw", CclActionDraw);
	gh_new_procedure2_0("action-set-timer", CclActionSetTimer);
	gh_new_procedure0_0("action-start-timer", CclActionStartTimer);
	gh_new_procedure0_0("action-stop-timer", CclActionStopTimer);
	gh_new_procedure1_0("action-wait", CclActionWait);
	gh_new_procedure2_0("action-set-switch", CclActionSetSwitch);

	gh_define("*triggers*", NIL);
#elif defined(USE_LUA)
	lua_register(Lua, "AddTrigger", CclAddTrigger);
#if 0
	lua_register(Lua, "SetTriggerNumber!", CclSetTriggerNumber);
#endif
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
#endif
}

/**
**  Print a trigger from a LISP object.
**  This is a modified version of lprin1g that prints
**  (lambda) instead of #&lt;CLOSURE&gt;
**
**  @param exp  Expression
**  @param f    File to print to
*/
#if defined(USE_GUILE) || defined(USE_SIOD)
local void PrintTrigger(SCM exp, CLFile* f)
{
#ifdef USE_GUILE
#else
	SCM tmp;
	long n;
#if 0
	struct user_type_hooks *p;
#endif
	extern char* subr_kind_str(long);

	STACK_CHECK(&exp);
	INTERRUPT_CHECK();
	switch TYPE(exp) {
	case tc_nil:
		CLprintf(f, "()");
		break;
	case tc_cons:
		CLprintf(f, "(");
		PrintTrigger(car(exp), f);
		for (tmp = cdr(exp); CONSP(tmp); tmp = cdr(tmp)) {
			CLprintf(f, " ");
			PrintTrigger(car(tmp), f);
		}
		if (NNULLP(tmp)) {
			CLprintf(f, " . ");
			PrintTrigger(tmp, f);
		}
		CLprintf(f, ")");
		break;
	case tc_flonum:
		n = (long)FLONM(exp);
		if (((double)n) == FLONM(exp)) {
			sprintf(tkbuffer, "%ld", n);
		} else {
			sprintf(tkbuffer, "%g", FLONM(exp));
		}
		CLprintf(f, tkbuffer);
		break;
	case tc_symbol:
		CLprintf(f, PNAME(exp));
		break;
	case tc_subr_0:
	case tc_subr_1:
	case tc_subr_2:
	case tc_subr_2n:
	case tc_subr_3:
	case tc_subr_4:
	case tc_subr_5:
	case tc_lsubr:
	case tc_fsubr:
	case tc_msubr:
		sprintf(tkbuffer, "#<%s ", subr_kind_str(TYPE(exp)));
		CLprintf(f, tkbuffer);
		CLprintf(f, (*exp).storage_as.subr.name);
		CLprintf(f, ">");
		break;
	case tc_string:
		CLprintf(f, "\"%s\"", (*exp).storage_as.string.data);
		break;
	case tc_closure:
		CLprintf(f, "(lambda ");
		if (CONSP((*exp).storage_as.closure.code)) {
			PrintTrigger(car((*exp).storage_as.closure.code), f);
			CLprintf(f, " ");
			PrintTrigger(cdr((*exp).storage_as.closure.code), f);
		} else
			PrintTrigger((*exp).storage_as.closure.code, f);
		CLprintf(f, ")");
		break;
	default:
		break;
#if 0
		p = get_user_type_hooks(TYPE(exp));
		if (p->prin1)
			(*p->prin1)(exp, f);
		else {
			sprintf(tkbuffer, "#<UNKNOWN %d %p>", TYPE(exp), exp);
			CLprintf(f, tkbuffer);
		}
#endif
	}
#endif
}
#elif defined(USE_LUA)
#endif

/**
**  Save the trigger module.
**
**  @param file  Open file to print to
*/
global void SaveTriggers(CLFile* file)
{
#if defined(USE_GUILE) || defined(USE_SIOD)
	SCM list;
	int i;
	int trigger;

	CLprintf(file, "\n;;; -----------------------------------------\n");
	CLprintf(file, ";;; MODULE: trigger $Id$\n\n");

	i = 0;
	trigger = -1;
	list = symbol_value(gh_symbol2scm("*triggers*"), NIL);
	while (!gh_null_p(list)) {
		if (gh_eq_p(Trigger, list)) {
			trigger = i;
		}
		CLprintf(file, "(add-trigger '");
		PrintTrigger(gh_car(gh_car(list)), file);
		CLprintf(file, " '");
		PrintTrigger(gh_cdr(gh_car(list)), file);
		CLprintf(file, ")\n");
		list = gh_cdr(list);
		++i;
	}
	CLprintf(file, "(set-trigger-number! %d)\n", trigger);

	if (GameTimer.Init) {
		CLprintf(file, "(action-set-timer %ld %d)\n",
			GameTimer.Cycles, GameTimer.Increasing);
		if (GameTimer.Running) {
			CLprintf(file, "(action-start-timer)\n");
		}
	}
#elif defined(USE_LUA)
#endif
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

#if defined(USE_GUILE) || defined(USE_SIOD)
	if (gh_null_p(symbol_value(gh_symbol2scm("*triggers*"), NIL))) {
		DebugLevel0Fn("Default triggers\n");
		gh_apply(symbol_value(gh_symbol2scm("single-player-triggers"), NIL), NIL);
	}
#elif defined(USE_LUA)
	lua_pushstring(Lua, "_triggers_");
	lua_gettable(Lua, LUA_GLOBALSINDEX);
	if (lua_isnil(Lua, -1)) {
		lua_pushstring(Lua, "SinglePlayerTriggers");
		lua_gettable(Lua, LUA_GLOBALSINDEX);
		LuaCall(0, 1);
	}
	lua_pop(Lua, 1);
#endif

	memset(Switch, 0, sizeof(Switch));
}

/**
**  Clean up the trigger module.
*/
global void CleanTriggers(void)
{
#if defined(USE_GUILE) || defined(USE_SIOD)
	SCM var;

	DebugLevel0Fn("FIXME: Cleaning trigger not written\n");

	var = gh_symbol2scm("*triggers*");
	setvar(var, NIL, NIL);

	CclGcProtectedAssign(&Trigger, NULL);
#elif defined(USE_LUA)
#endif

	memset(&GameTimer, 0, sizeof(GameTimer));
}

//@}
