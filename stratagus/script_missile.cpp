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
/**@name ccl_missile.c - The missile-type ccl functions. */
//
//      (c) Copyright 2002-2003 by Lutz Sammer
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

#include "stratagus.h"
#include "video.h"
#include "tileset.h"
#include "unittype.h"
#include "missile.h"
#include "ccl_sound.h"
#include "ccl.h"

/*----------------------------------------------------------------------------
--  Variables
----------------------------------------------------------------------------*/

#ifdef DEBUG
extern int NoWarningMissileType; /// quiet ident lookup.
#endif

/*----------------------------------------------------------------------------
--  Functions
----------------------------------------------------------------------------*/

/**
**  Parse missile-type.
**
**  @param list  List describing missile-type.
*/
local int CclDefineMissileType(lua_State* l)
{
	const char* value;
	char* str;
	MissileType* mtype;
	unsigned i;

	if (lua_gettop(l) != 2 || !lua_istable(l, 2)) {
		lua_pushstring(l, "incorrect argument");
		lua_error(l);
	}

	// Slot identifier

	str = strdup(LuaToString(l, 1));
#ifdef DEBUG
	i = NoWarningMissileType;
	NoWarningMissileType = 1;
#endif
	mtype = MissileTypeByIdent(str);
#ifdef DEBUG
	NoWarningMissileType = i;
#endif
	if (mtype) {
		DebugLevel0Fn("Redefining missile-type `%s'\n" _C_ str);
		free(str);
	} else {
		mtype = NewMissileTypeSlot(str);  // str consumed!
	} 

	mtype->NumDirections = 1;
	// Ensure we don't divide by zero.
	mtype->SplashFactor = 100;

	//
	// Parse the arguments
	//
	lua_pushnil(l);
	while (lua_next(l, 2)) {
		value = LuaToString(l, -2);
		if (!strcmp(value, "File")) {
			free(mtype->File);
			mtype->File = strdup(LuaToString(l, -1));
		} else if (!strcmp(value, "Size")) {
			if (!lua_istable(l, -1) || luaL_getn(l, -1) != 2) {
				lua_pushstring(l, "incorrect argument");
				lua_error(l);
			}
			lua_rawgeti(l, -1, 1);
			mtype->Width = LuaToNumber(l, -1);
			lua_pop(l, 1);
			lua_rawgeti(l, -1, 2);
			mtype->Height = LuaToNumber(l, -1);
			lua_pop(l, 1);
		} else if (!strcmp(value, "Frames")) {
			mtype->SpriteFrames = LuaToNumber(l, -1);
		} else if (!strcmp(value, "NumDirections")) {
			mtype->NumDirections = LuaToNumber(l, -1);
		} else if (!strcmp(value, "transparency")) {
			mtype->Transparency = LuaToNumber(l, -1);
		} else if (!strcmp(value, "FiredSound")) {
			free(mtype->FiredSound.Name);
			mtype->FiredSound.Name = strdup(LuaToString(l, -1));
		} else if (!strcmp(value, "ImpactSound")) {
			free(mtype->ImpactSound.Name);
			mtype->ImpactSound.Name = strdup(LuaToString(l, -1));
		} else if (!strcmp(value, "Class")) {
			value = LuaToString(l, -1);
			for (i = 0; MissileClassNames[i]; ++i) {
				if (!strcmp(value, MissileClassNames[i])) {
					mtype->Class = i;
					break;
				}
			}
			if (!MissileClassNames[i]) {
				lua_pushfstring(l, "Unsupported class: %s", value);
				lua_error(l);
			}
		} else if (!strcmp(value, "NumBounces")) {
			mtype->NumBounces = LuaToNumber(l, -1);
		} else if (!strcmp(value, "Delay")) {
			mtype->StartDelay = LuaToNumber(l, -1);
		} else if (!strcmp(value, "Sleep")) {
			mtype->Sleep = LuaToNumber(l, -1);
		} else if (!strcmp(value, "Speed")) {
			mtype->Speed = LuaToNumber(l, -1);
		} else if (!strcmp(value, "DrawLevel")) {
			mtype->DrawLevel = LuaToNumber(l, -1);
		} else if (!strcmp(value, "Range")) {
			mtype->Range = LuaToNumber(l, -1);
		} else if (!strcmp(value, "ImpactMissile")) {
			free(mtype->ImpactName);
			mtype->ImpactName = strdup(LuaToString(l, -1));
		} else if (!strcmp(value, "SmokeMissile")) {
			free(mtype->ImpactName);
			mtype->SmokeName = strdup(LuaToString(l, -1));
		} else if (!strcmp(value, "CanHitOwner")) {
			mtype->CanHitOwner = LuaToBoolean(l, -1);
		} else if (!strcmp(value, "FriendlyFire")) {
			mtype->FriendlyFire = LuaToBoolean(l, -1);
		} else if (!strcmp(value, "SplashFactor")) {
			mtype->SplashFactor = LuaToNumber(l, -1);
		} else {
			lua_pushfstring(l, "Unsupported tag: %s", value);
			lua_error(l);
		}
		lua_pop(l, 1);
	}

	return 0;
}

/**
**  Define missile type mapping from original number to internal symbol
**
**  @param list  List of all names.
*/
local int CclDefineMissileTypeWcNames(lua_State* l)
{
	int i;
	int j;
	char** cp;

	if ((cp = MissileTypeWcNames)) {  // Free all old names
		while (*cp) {
			free(*cp++);
		}
		free(MissileTypeWcNames);
	}

	//
	// Get new table.
	//
	i = lua_gettop(l);
	MissileTypeWcNames = cp = malloc((i + 1) * sizeof(char*));
	if (!cp) {
		fprintf(stderr, "out of memory.\n");
		ExitFatal(-1);
	}

	for (j = 0; j < i; ++j) {
		*cp++ = strdup(LuaToString(l, j + 1));
	}
	*cp = NULL;

	return 0;
}

/**
**  Create a missile.
**
**  @param list  List of all names.
*/
#if defined(USE_GUILE) || defined(USE_SIOD)
local SCM CclMissile(SCM list)
{
	SCM value;
	char* str;
	MissileType* type;
	int x;
	int y;
	int dx;
	int dy;
	int sx;
	int sy;
	Missile* missile;

	DebugLevel0Fn("FIXME: not finished\n");

	missile = NULL;
	type = NULL;
	x = dx = y = dy = sx = sy = -1;

	while (!gh_null_p(list)) {
		value = gh_car(list);
		list = gh_cdr(list);

		if (gh_eq_p(value, gh_symbol2scm("type"))) {
			value = gh_car(list);
			list = gh_cdr(list);
			str = gh_scm2newstr(value, NULL);
			type = MissileTypeByIdent(str);
			free(str);
		} else if (gh_eq_p(value, gh_symbol2scm("pos"))) {
			SCM sublist;

			sublist = gh_car(list);
			list = gh_cdr(list);
			x = gh_scm2int(gh_car(sublist));
			y = gh_scm2int(gh_cadr(sublist));
		} else if (gh_eq_p(value, gh_symbol2scm("origin-pos"))) {
			SCM sublist;

			sublist = gh_car(list);
			list = gh_cdr(list);
			sx = gh_scm2int(gh_car(sublist));
			sy = gh_scm2int(gh_cadr(sublist));
		} else if (gh_eq_p(value, gh_symbol2scm("goal"))) {
			SCM sublist;

			sublist = gh_car(list);
			list = gh_cdr(list);
			dx = gh_scm2int(gh_car(sublist));
			dy = gh_scm2int(gh_cadr(sublist));
		} else if (gh_eq_p(value, gh_symbol2scm("local"))) {
			DebugCheck(!type);
			missile = MakeLocalMissile(type, x, y, dx, dy);
			// we need to reinitialize position parameters - that's because of
			// the way InitMissile() (called from MakeLocalMissile()) computes
			// them - it works for creating a missile during a game but breaks
			// loading the missile from a file.
			missile->X = x;
			missile->Y = y;
			missile->SourceX = sx;
			missile->SourceY = sy;
			missile->DX = dx;
			missile->DY = dy;
			missile->Local = 1;
		} else if (gh_eq_p(value, gh_symbol2scm("global"))) {
			DebugCheck(!type);
			missile = MakeMissile(type, x, y, dx, dy);
			missile->X = x;
			missile->Y = y;
			missile->SourceX = sx;
			missile->SourceY = sy;
			missile->DX = dx;
			missile->DY = dy;
			missile->Local = 0;
		} else if (gh_eq_p(value, gh_symbol2scm("frame"))) {
			DebugCheck(!missile);
			missile->SpriteFrame = gh_scm2int(gh_car(list));
			list = gh_cdr(list);
		} else if (gh_eq_p(value, gh_symbol2scm("state"))) {
			DebugCheck(!missile);
			missile->State = gh_scm2int(gh_car(list));
			list = gh_cdr(list);
		} else if (gh_eq_p(value, gh_symbol2scm("anim-wait"))) {
			DebugCheck(!missile);
			missile->AnimWait = gh_scm2int(gh_car(list));
			list = gh_cdr(list);
		} else if (gh_eq_p(value, gh_symbol2scm("wait"))) {
			DebugCheck(!missile);
			missile->Wait = gh_scm2int(gh_car(list));
			list = gh_cdr(list);
		} else if (gh_eq_p(value, gh_symbol2scm("delay"))) {
			DebugCheck(!missile);
			missile->Delay = gh_scm2int(gh_car(list));
			list = gh_cdr(list);
		} else if (gh_eq_p(value, gh_symbol2scm("source"))) {
			DebugCheck(!missile);
			value = gh_car(list);
			list = gh_cdr(list);
			str = gh_scm2newstr(value, NULL);
			missile->SourceUnit = UnitSlots[strtol(str + 1, 0, 16)];
			free(str);
			++missile->SourceUnit->Refs;
		} else if (gh_eq_p(value, gh_symbol2scm("target"))) {
			DebugCheck(!missile);
			value = gh_car(list);
			list = gh_cdr(list);
			str = gh_scm2newstr(value, NULL);
			missile->TargetUnit = UnitSlots[strtol(str + 1, 0, 16)];
			free(str);
			missile->TargetUnit->Refs++;
		} else if (gh_eq_p(value, gh_symbol2scm("damage"))) {
			DebugCheck(!missile);
			missile->Damage = gh_scm2int(gh_car(list));
			list = gh_cdr(list);
		} else if (gh_eq_p(value, gh_symbol2scm("ttl"))) {
			DebugCheck(!missile);
			missile->TTL = gh_scm2int(gh_car(list));
			list = gh_cdr(list);
		} else if (gh_eq_p(value, gh_symbol2scm("hidden"))) {
			DebugCheck(!missile);
			missile->Hidden = 1;
			list = gh_cdr(list);
		} else if (gh_eq_p(value, gh_symbol2scm("step"))) {
			SCM sublist;

			DebugCheck(!missile);
			sublist = gh_car(list);
			list = gh_cdr(list);
			missile->CurrentStep = gh_scm2int(gh_car(sublist));
			missile->TotalStep = gh_scm2int(gh_cadr(sublist));
		}
	}
	return SCM_UNSPECIFIED;
}
#elif defined(USE_LUA)
local int CclMissile(lua_State* l)
{
	return 0;
}
#endif

/**
**  Define burning building missiles.
**
**  @param list  FIXME: docu.
*/
local int CclDefineBurningBuilding(lua_State* l)
{
	const char* value;
	BurningBuildingFrame** frame;
	BurningBuildingFrame* ptr;
	BurningBuildingFrame* next;
	int args;
	int j;
	int subargs;
	int k;

	ptr = BurningBuildingFrames;
	while (ptr) {
		next = ptr->Next;
		free(ptr);
		ptr = next;
	}
	BurningBuildingFrames = NULL;

	frame = &BurningBuildingFrames;

	args = lua_gettop(l);
	for (j = 0; j < args; ++j) {
		if (!lua_istable(l, j + 1)) {
			lua_pushstring(l, "incorrect argument");
			lua_error(l);
		}

		*frame = calloc(1, sizeof(BurningBuildingFrame));
		subargs = luaL_getn(l, j + 1);
		for (k = 0; k < subargs; ++k) {
			lua_rawgeti(l, j + 1, k + 1);
			value = LuaToString(l, -1);
			lua_pop(l, 1);
			++k;

			if (!strcmp(value, "percent")) {
				lua_rawgeti(l, j + 1, k + 1);
				(*frame)->Percent = LuaToNumber(l, -1);
				lua_pop(l, 1);
			} else if (!strcmp(value, "missile")) {
				lua_rawgeti(l, j + 1, k + 1);
				(*frame)->Missile = MissileTypeByIdent(LuaToString(l, -1));
				lua_pop(l, 1);
			}
		}
		frame = &((*frame)->Next);
	}
	return 0;
}

/**
**  Register CCL features for missile-type.
*/
global void MissileCclRegister(void)
{
	lua_register(Lua, "DefineMissileTypeWcNames",
		CclDefineMissileTypeWcNames);
	lua_register(Lua, "DefineMissileType", CclDefineMissileType);
	lua_register(Lua, "Missile", CclMissile);
	lua_register(Lua, "DefineBurningBuilding", CclDefineBurningBuilding);
}

//@}
