//     ____                _       __               
//    / __ )____  _____   | |     / /___ ___________
//   / __  / __ \/ ___/   | | /| / / __ `/ ___/ ___/
//  / /_/ / /_/ (__  )    | |/ |/ / /_/ / /  (__  ) 
// /_____/\____/____/     |__/|__/\__,_/_/  /____/  
//                                              
//       A futuristic real-time strategy game.
//          This file is part of Bos Wars.
//
/**@name script_missile.cpp - The missile-type ccl functions. */
//
//      (c) Copyright 2002-2008 by Lutz Sammer and Jimmy Salmon
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
#include "video.h"
#include "missile.h"
#include "script.h"
#include "unit.h"
#include "unit_manager.h"
#include "particle.h"
#include "luacallback.h"

/*----------------------------------------------------------------------------
--  Variables
----------------------------------------------------------------------------*/

/**
**  Missile class names, used to load/save the missiles.
*/
static const char *MissileClassNames[] = {
	"missile-class-none",
	"missile-class-point-to-point",
	"missile-class-point-to-point-with-hit",
	"missile-class-point-to-point-cycle-once",
	"missile-class-point-to-point-bounce",
	"missile-class-stay",
	"missile-class-cycle-once",
	"missile-class-fire",
	"missile-class-hit",
	"missile-class-parabolic",
	NULL
};

/*----------------------------------------------------------------------------
--  Functions
----------------------------------------------------------------------------*/

/**
**  Parse missile-type.
**
**  @param l  Lua state.
*/
static int CclDefineMissileType(lua_State *l)
{
	const char *value;
	const char *str;
	MissileType *mtype;
	unsigned i;
	std::string file;

	LuaCheckArgs(l, 2);
	LuaCheckTable(l, 2);

	// Slot identifier

	str = LuaToString(l, 1);
	mtype = MissileTypeByIdent(str);

	if (mtype) {
		DebugPrint("Redefining missile-type `%s'\n" _C_ str);
	} else {
		mtype = NewMissileTypeSlot(str);
	}

	mtype->NumDirections = 1;
	mtype->Flip = true;
	// Ensure we don't divide by zero.
	mtype->SplashFactor = 100;

	//
	// Parse the arguments
	//
	for (lua_pushnil(l); lua_next(l, 2); lua_pop(l, 1)) {
		value = LuaToString(l, -2);
		if (!strcmp(value, "File")) {
			file = LuaToString(l, -1);
		} else if (!strcmp(value, "Size")) {
			LuaCheckTableSize(l, -1, 2);
			mtype->Width = LuaToNumber(l, -1, 1);
			mtype->Height = LuaToNumber(l, -1, 2);
		} else if (!strcmp(value, "Frames")) {
			mtype->SpriteFrames = LuaToNumber(l, -1);
		} else if (!strcmp(value, "Flip")) {
			mtype->Flip = LuaToBoolean(l, -1);
		} else if (!strcmp(value, "NumDirections")) {
			mtype->NumDirections = LuaToNumber(l, -1);
		} else if (!strcmp(value, "transparency")) {
			mtype->Transparency = LuaToNumber(l, -1);
		} else if (!strcmp(value, "FiredSound")) {
			mtype->FiredSound.Name = LuaToString(l, -1);
		} else if (!strcmp(value, "ImpactSound")) {
			mtype->ImpactSound.Name = LuaToString(l, -1);
		} else if (!strcmp(value, "Class")) {
			value = LuaToString(l, -1);
			for (i = 0; MissileClassNames[i]; ++i) {
				if (!strcmp(value, MissileClassNames[i])) {
					mtype->Class = i;
					break;
				}
			}
			if (!MissileClassNames[i]) {
				LuaError(l, "Unsupported class: %s" _C_ value);
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
			mtype->ImpactName = LuaToString(l, -1);
		} else if (!strcmp(value, "SmokeMissile")) {
			mtype->SmokeName = LuaToString(l, -1);
		} else if (!strcmp(value, "ImpactParticle")) {
			mtype->ImpactParticle = new LuaCallback(l, -1);
		} else if (!strcmp(value, "CanHitOwner")) {
			mtype->CanHitOwner = LuaToBoolean(l, -1);
		} else if (!strcmp(value, "FriendlyFire")) {
			mtype->FriendlyFire = LuaToBoolean(l, -1);
		} else if (!strcmp(value, "SplashFactor")) {
			mtype->SplashFactor = LuaToNumber(l, -1);
		} else {
			LuaError(l, "Unsupported tag: %s" _C_ value);
		}
	}

	if (!file.empty()) {
		mtype->G = CGraphic::New(file, mtype->Width, mtype->Height);
	}

	return 0;
}

/**
**  Create a missile.
**
**  @param l  Lua state.
*/
static int CclMissile(lua_State *l)
{
	const char *value;
	MissileType *type;
	int x;
	int y;
	int dx;
	int dy;
	int sx;
	int sy;
	Missile *missile;
	int args;
	int j;

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
			LuaCheckTableSize(l, j + 1, 2);
			x = LuaToNumber(l, j + 1, 1);
			y = LuaToNumber(l, j + 1, 2);
		} else if (!strcmp(value, "origin-pos")) {
			LuaCheckTableSize(l, j + 1, 2);
			sx = LuaToNumber(l, j + 1, 1);
			sy = LuaToNumber(l, j + 1, 2);
		} else if (!strcmp(value, "goal")) {
			LuaCheckTableSize(l, j + 1, 2);
			dx = LuaToNumber(l, j + 1, 1);
			dy = LuaToNumber(l, j + 1, 2);
		} else if (!strcmp(value, "local")) {
			Assert(type);
			missile = MakeLocalMissile(type, x, y, dx, dy);
			missile->Local = 1;
			--j;
		} else if (!strcmp(value, "global")) {
			Assert(type);
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
			Assert(missile);
			missile->SpriteFrame = LuaToNumber(l, j + 1);
		} else if (!strcmp(value, "state")) {
			Assert(missile);
			missile->State = LuaToNumber(l, j + 1);
		} else if (!strcmp(value, "anim-wait")) {
			Assert(missile);
			missile->AnimWait = LuaToNumber(l, j + 1);
		} else if (!strcmp(value, "wait")) {
			Assert(missile);
			missile->Wait = LuaToNumber(l, j + 1);
		} else if (!strcmp(value, "delay")) {
			Assert(missile);
			missile->Delay = LuaToNumber(l, j + 1);
		} else if (!strcmp(value, "source")) {
			Assert(missile);
			value = LuaToString(l, j + 1);
			missile->SourceUnit = UnitSlots[strtol(value + 1, 0, 16)];
			missile->SourceUnit->RefsIncrease();
		} else if (!strcmp(value, "target")) {
			Assert(missile);
			value = LuaToString(l, j + 1);
			missile->TargetUnit = UnitSlots[strtol(value + 1, 0, 16)];
			missile->TargetUnit->RefsIncrease();
		} else if (!strcmp(value, "damage")) {
			Assert(missile);
			missile->Damage = LuaToNumber(l, j + 1);
		} else if (!strcmp(value, "ttl")) {
			Assert(missile);
			missile->TTL = LuaToNumber(l, j + 1);
		} else if (!strcmp(value, "hidden")) {
			Assert(missile);
			missile->Hidden = 1;
			--j;
		} else if (!strcmp(value, "step")) {
			Assert(missile);
			LuaCheckTableSize(l, j + 1, 2);
			missile->CurrentStep = LuaToNumber(l, j + 1, 1);
			missile->TotalStep = LuaToNumber(l, j + 1, 2);
		} else {
			LuaError(l, "Unsupported tag: %s" _C_ value);
		}
	}

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
	return 0;
}

/**
**  Define burning building missiles.
**
**  @param l  Lua state.
*/
static int CclDefineBurningBuilding(lua_State *l)
{
	for (std::vector<BurningBuildingFrame *>::iterator i = BurningBuildingFrames.begin();
			i != BurningBuildingFrames.end(); ++i) {
		delete *i;
	}
	BurningBuildingFrames.clear();

	int args = lua_gettop(l);
	for (int j = 0; j < args; ++j) {
		LuaCheckTable(l, j + 1);

		const char *value;
		BurningBuildingFrame *ptr = new BurningBuildingFrame;
		int subargs = lua_objlen(l, j + 1);

		for (int k = 0; k < subargs; ++k) {
			value = LuaToString(l, j + 1, k + 1);
			++k;

			if (!strcmp(value, "percent")) {
				ptr->Percent = LuaToNumber(l, j + 1, k + 1);
			} else if (!strcmp(value, "missile")) {
				ptr->Missile = MissileTypeByIdent(LuaToString(l, j + 1, k + 1));
			}
		}
		BurningBuildingFrames.insert(BurningBuildingFrames.begin(), ptr);
	}
	return 0;
}

/**
**  Register CCL features for missile-type.
*/
void MissileCclRegister(void)
{
	lua_register(Lua, "DefineMissileType", CclDefineMissileType);
	lua_register(Lua, "Missile", CclMissile);
	lua_register(Lua, "DefineBurningBuilding", CclDefineBurningBuilding);
}

//@}
