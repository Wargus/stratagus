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
#include "script_sound.h"
#include "script.h"

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
	mtype->Flip = 1;
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
		} else if (!strcmp(value, "Flip")) {
			mtype->SpriteFrames = LuaToBoolean(l, -1);
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
			free(mtype->SmokeName);
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
local int CclMissile(lua_State* l)
{
	const char* value;
	MissileType* type;
	int x;
	int y;
	int dx;
	int dy;
	int sx;
	int sy;
	Missile* missile;
	int args;
	int j;

	DebugLevel0Fn("FIXME: not finished\n");

	missile = NULL;
	type = NULL;
	x = dx = y = dy = sx = sy = -1;

	args = lua_gettop(l);
	for (j = 0; j < args; ++j) {
		value = LuaToString(l, j + 1);
		++j;

		if (!strcmp(value, "type")) {
			type = MissileTypeByIdent(LuaToString(l, j + 1));
		} else if (!strcmp(value, "pos")) {
			if (!lua_istable(l, j + 1) || luaL_getn(l, j + 1) != 2) {
				lua_pushstring(l, "incorrect argument");
				lua_error(l);
			}
			lua_rawgeti(l, j + 1, 1);
			x = LuaToNumber(l, -1);
			lua_pop(l, 1);
			lua_rawgeti(l, j + 1, 2);
			y = LuaToNumber(l, -1);
			lua_pop(l, 1);
		} else if (!strcmp(value, "origin-pos")) {
			if (!lua_istable(l, j + 1) || luaL_getn(l, j + 1) != 2) {
				lua_pushstring(l, "incorrect argument");
				lua_error(l);
			}
			lua_rawgeti(l, j + 1, 1);
			sx = LuaToNumber(l, -1);
			lua_pop(l, 1);
			lua_rawgeti(l, j + 1, 2);
			sy = LuaToNumber(l, -1);
			lua_pop(l, 1);
		} else if (!strcmp(value, "goal")) {
			if (!lua_istable(l, j + 1) || luaL_getn(l, j + 1) != 2) {
				lua_pushstring(l, "incorrect argument");
				lua_error(l);
			}
			lua_rawgeti(l, j + 1, 1);
			dx = LuaToNumber(l, -1);
			lua_pop(l, 1);
			lua_rawgeti(l, j + 1, 2);
			dy = LuaToNumber(l, -1);
			lua_pop(l, 1);
		} else if (!strcmp(value, "local")) {
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
			--j;
		} else if (!strcmp(value, "global")) {
			DebugCheck(!type);
			missile = MakeMissile(type, x, y, dx, dy);
			missile->X = x;
			missile->Y = y;
			missile->SourceX = sx;
			missile->SourceY = sy;
			missile->DX = dx;
			missile->DY = dy;
			missile->Local = 0;
			--j;
		} else if (!strcmp(value, "frame")) {
			DebugCheck(!missile);
			missile->SpriteFrame = LuaToNumber(l, j + 1);
		} else if (!strcmp(value, "state")) {
			DebugCheck(!missile);
			missile->State = LuaToNumber(l, j + 1);
		} else if (!strcmp(value, "anim-wait")) {
			DebugCheck(!missile);
			missile->AnimWait = LuaToNumber(l, j + 1);
		} else if (!strcmp(value, "wait")) {
			DebugCheck(!missile);
			missile->Wait = LuaToNumber(l, j + 1);
		} else if (!strcmp(value, "delay")) {
			DebugCheck(!missile);
			missile->Delay = LuaToNumber(l, j + 1);
		} else if (!strcmp(value, "source")) {
			DebugCheck(!missile);
			value = LuaToString(l, j + 1);
			missile->SourceUnit = UnitSlots[strtol(value + 1, 0, 16)];
			RefsIncrease(missile->SourceUnit);
		} else if (!strcmp(value, "target")) {
			DebugCheck(!missile);
			value = LuaToString(l, j + 1);
			missile->TargetUnit = UnitSlots[strtol(value + 1, 0, 16)];
			RefsIncrease(missile->TargetUnit);
		} else if (!strcmp(value, "damage")) {
			DebugCheck(!missile);
			missile->Damage = LuaToNumber(l, j + 1);
		} else if (!strcmp(value, "ttl")) {
			DebugCheck(!missile);
			missile->TTL = LuaToNumber(l, j + 1);
		} else if (!strcmp(value, "hidden")) {
			DebugCheck(!missile);
			missile->Hidden = 1;
			--j;
		} else if (!strcmp(value, "step")) {
			DebugCheck(!missile);
			if (!lua_istable(l, j + 1) || luaL_getn(l, j + 1) != 2) {
				lua_pushstring(l, "incorrect argument");
				lua_error(l);
			}
			lua_rawgeti(l, j + 1, 1);
			missile->CurrentStep = LuaToNumber(l, -1);
			lua_pop(l, 1);
			lua_rawgeti(l, j + 1, 2);
			missile->TotalStep = LuaToNumber(l, -1);
			lua_pop(l, 1);
		}
	}
	return 0;
}

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

#ifdef META_LUA

	/// Proxy type for MissileType
local ScriptProxyType ScriptProxyMissileType;
	/// Proxy type for the SpellType array
local ScriptProxyType ScriptProxyMissileTypes;

/**
**  Create a new missile Type
**
**	@param l    Lua state
*/
local int ScriptMissileTypeCreate(lua_State* l)
{
	const char* name;
	MissileType* mtype;

	if (lua_gettop(l) != 1) {
		LuaError(l, "Incorrect number of arguments");
	}
	name = LuaToString(l, 1);

	mtype = MissileTypeByIdent(name);
	if (mtype != NULL) {
		LuaError(l, "Missile allready exists");
	} else {
		mtype = NewMissileTypeSlot(strdup(name));

		//  Defaults.
		mtype->NumDirections = 1;
		mtype->Flip = 1;
		mtype->SplashFactor = 100;

		ScriptCreateUserdata(l, mtype, &ScriptProxyMissileType);
		return 1;
	}
}

/**
**	Get function for a missile type userdata.
**
**	@param missiletype	Pointer to the missile type.
**	@param key      Key string.
*/
local int ScriptMissileTypeGet(MissileType* mtype, const char* key, lua_State* l)
{
	META_GET_STRING("Ident", mtype->Ident);
	META_GET_STRING("File", mtype->File);

	META_GET_INT("Transparency", mtype->Transparency);
	META_GET_INT("Width", mtype->Width);
	META_GET_INT("Height", mtype->Height);
	META_GET_INT("DrawLevel", mtype->DrawLevel);
	META_GET_INT("SpriteFrames", mtype->SpriteFrames);
	META_GET_INT("NumDirections", mtype->NumDirections);

	META_GET_INT("NumBounces", mtype->NumBounces);
	META_GET_INT("Sleep", mtype->Sleep);
	META_GET_INT("Speed", mtype->Speed);
	META_GET_INT("Range", mtype->Range);
	META_GET_INT("SplashFactor", mtype->SplashFactor);
	META_GET_BOOL("CanHitOwner", mtype->CanHitOwner);
	META_GET_BOOL("FriendlyFire", mtype->FriendlyFire);

	META_GET_STRING("ImpactMissile", mtype->ImpactName);
	META_GET_STRING("SmokeMissile", mtype->SmokeName);

	META_GET_STRING("FiredSound", mtype->FiredSound.Name);
	META_GET_STRING("ImpactSound", mtype->ImpactSound.Name);
	META_GET_STRING("Class", MissileClassNames[mtype->Class]);

	LuaError(l, "Field \"%s\" is innexistent or write-only (yes, we have those).\n" _C_ key);
}

/**
**	Set function for a missile type userdata.
**
**	@param missiletype	Pointer to the missile type.
**	@param key      Key string.
*/
local int ScriptMissileTypeSet(MissileType* mtype, const char* key, lua_State* l)
{
	META_SET_STRING("File", mtype->File);

	META_SET_INT("Transparency", mtype->Transparency);
	META_SET_INT("Width", mtype->Width);
	META_SET_INT("Height", mtype->Height);
	META_SET_INT("DrawLevel", mtype->DrawLevel);
	META_SET_INT("SpriteFrames", mtype->SpriteFrames);
	META_SET_INT("NumDirections", mtype->NumDirections);

	META_SET_INT("NumBounces", mtype->NumBounces);
	META_SET_INT("Sleep", mtype->Sleep);
	META_SET_INT("Speed", mtype->Speed);
	META_SET_INT("Range", mtype->Range);
	META_SET_INT("SplashFactor", mtype->SplashFactor);
	META_SET_BOOL("CanHitOwner", mtype->CanHitOwner);
	META_SET_BOOL("FriendlyFire", mtype->FriendlyFire);

	META_SET_STRING("ImpactMissile", mtype->ImpactName);
	META_SET_STRING("SmokeMissile", mtype->SmokeName);

	META_SET_STRING("FiredSound", mtype->FiredSound.Name);
	META_SET_STRING("ImpactSound", mtype->ImpactSound.Name);

	if (!strcmp(key, "Class")) {
		const char* value;
		int i;

		value = LuaToString(l, -1);
		for (i = 0; MissileClassNames[i]; ++i) {
			if (!strcmp(value, MissileClassNames[i])) {
				mtype->Class = i;
				return 0;
			}
		}
		LuaError(l, "Unsupported missile class: %s" _C_ value);
	}

	LuaError(l, "Field \"%s\" is innexistent or read-only.\n" _C_ key);
}

/**
**	Get function for the big missile types namespace, with int index
*/
local int ScriptMissileTypesGetInt(void* object, int index, lua_State* l)
{
	if (index < 0 || index >= NumMissileTypes) {
		LuaError(l, "Missile type index out of range");
	}
	ScriptCreateUserdata(l, MissileTypes[index], &ScriptProxyMissileType);
	return 1;
}

/**
**	Get function for the big missile types namespace, with string key
*/
local int ScriptMissileTypesGetStr(void* object, const char* key, lua_State* l)
{
	MissileType* mtype;

	META_GET_INT("n", NumMissileTypes);
	META_GET_FUNC("Create", ScriptMissileTypeCreate);
	if ((mtype = MissileTypeByIdent(key))) {
		ScriptCreateUserdata(l, mtype, &ScriptProxyMissileType);
		return 1;
	}

	LuaError(l, "Missile type \"%s\" doesn't exist.\n" _C_ key);
}

/**
**	Initialize missile scripting. The main table is at -1
**
**	@param l   The lua state.
*/
global void ScriptMissileTypesInit(void)
{
	ScriptProxyMissileType.GetStr = (ScriptGetSetStrFunction *)ScriptMissileTypeGet;
	ScriptProxyMissileType.SetStr = (ScriptGetSetStrFunction *)ScriptMissileTypeSet;
	ScriptProxyMissileType.GetInt = ScriptGetSetIntBlock;
	ScriptProxyMissileType.SetInt = ScriptGetSetIntBlock;
	ScriptProxyMissileType.Collect = 0;

	ScriptProxyMissileTypes.GetStr = (ScriptGetSetStrFunction *)ScriptMissileTypesGetStr;
	ScriptProxyMissileTypes.SetStr = ScriptGetSetStrBlock;
	ScriptProxyMissileTypes.GetInt = (ScriptGetSetIntFunction *)ScriptMissileTypesGetInt;
	ScriptProxyMissileTypes.SetInt = ScriptGetSetIntBlock;
	ScriptProxyMissileTypes.Collect = 0;

	// Create Stratagus.MissileTypes namespace.
	lua_pushstring(Lua, "MissileTypes");
	ScriptCreateUserdata(Lua, 0, &ScriptProxyMissileTypes);
	lua_rawset(Lua, -3);
}

#endif

//@}
