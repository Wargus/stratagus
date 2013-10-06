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
/**@name script_missile.cpp - The missile-type ccl functions. */
//
//      (c) Copyright 2002-2005 by Lutz Sammer and Jimmy Salmon
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

#include "missile.h"

#include "luacallback.h"
#include "script.h"
#include "unittype.h"
#include "unit.h"
#include "unit_manager.h"
#include "video.h"

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
	"missile-class-land-mine",
	"missile-class-whirlwind",
	"missile-class-flame-shield",
	"missile-class-death-coil",
	"missile-class-tracer",
	"missile-class-clip-to-target",
	"missile-class-continious",
	"missile-class-straight-fly",
	NULL
};

/*----------------------------------------------------------------------------
--  Functions
----------------------------------------------------------------------------*/

void MissileType::Load(lua_State *l)
{
	this->NumDirections = 1;
	this->Flip = true;
	// Ensure we don't divide by zero.
	this->SplashFactor = 100;

	// Parse the arguments
	std::string file;
	for (lua_pushnil(l); lua_next(l, 2); lua_pop(l, 1)) {
		const char *value = LuaToString(l, -2);

		if (!strcmp(value, "File")) {
			file = LuaToString(l, -1);
		} else if (!strcmp(value, "Size")) {
			CclGetPos(l, &this->size.x, &this->size.y);
		} else if (!strcmp(value, "Frames")) {
			this->SpriteFrames = LuaToNumber(l, -1);
		} else if (!strcmp(value, "Flip")) {
			this->Flip = LuaToBoolean(l, -1);
		} else if (!strcmp(value, "NumDirections")) {
			this->NumDirections = LuaToNumber(l, -1);
		} else if (!strcmp(value, "Transparency")) {
			this->Transparency = LuaToNumber(l, -1);
		} else if (!strcmp(value, "FiredSound")) {
			this->FiredSound.Name = LuaToString(l, -1);
		} else if (!strcmp(value, "ImpactSound")) {
			this->ImpactSound.Name = LuaToString(l, -1);
		} else if (!strcmp(value, "ChangeVariable")) {
			const int index = UnitTypeVar.VariableNameLookup[LuaToString(l, -1)];// User variables
			if (index == -1) {
				fprintf(stderr, "Bad variable name '%s'\n", LuaToString(l, -1));
				Exit(1);
				return;
			}
			this->ChangeVariable = index;
		} else if (!strcmp(value, "ChangeAmount")) {
			this->ChangeAmount = LuaToNumber(l, -1);
		} else if (!strcmp(value, "ChangeMax")) {
			this->ChangeMax = LuaToBoolean(l, -1);
		} else if (!strcmp(value, "Class")) {
			const char *className = LuaToString(l, -1);
			unsigned int i = 0;
			for (; MissileClassNames[i]; ++i) {
				if (!strcmp(className, MissileClassNames[i])) {
					this->Class = i;
					break;
				}
			}
			if (!MissileClassNames[i]) {
				LuaError(l, "Unsupported class: %s" _C_ value);
			}
		} else if (!strcmp(value, "NumBounces")) {
			this->NumBounces = LuaToNumber(l, -1);
		} else if (!strcmp(value, "ParabolCoefficient")) {
			this->ParabolCoefficient = LuaToNumber(l, -1);
		} else if (!strcmp(value, "Delay")) {
			this->StartDelay = LuaToNumber(l, -1);
		} else if (!strcmp(value, "Sleep")) {
			this->Sleep = LuaToNumber(l, -1);
		} else if (!strcmp(value, "Speed")) {
			this->Speed = LuaToNumber(l, -1);
		} else if (!strcmp(value, "TTL")) {
			this->TTL = LuaToNumber(l, -1);
		} else if (!strcmp(value, "Damage")) {
			this->Damage = CclParseNumberDesc(l);
			lua_pushnil(l);
		} else if (!strcmp(value, "ReduceFactor")) {
			this->ReduceFactor = LuaToNumber(l, -1);
		} else if (!strcmp(value, "SmokePrecision")) {
			this->SmokePrecision = LuaToNumber(l, -1);
		} else if (!strcmp(value, "MissileStopFlags")) {
			this->MissileStopFlags = LuaToNumber(l, -1);
		} else if (!strcmp(value, "DrawLevel")) {
			this->DrawLevel = LuaToNumber(l, -1);
		} else if (!strcmp(value, "Range")) {
			this->Range = LuaToNumber(l, -1);
		} else if (!strcmp(value, "ImpactMissile")) {
			if (!lua_istable(l, -1)) {
				MissileConfig *mc = new MissileConfig();
				mc->Name = LuaToString(l, -1);
				this->Impact.push_back(mc);
			} else {
				const int impacts = lua_rawlen(l, -1);
				for (int i = 0; i < impacts; ++i) {
					MissileConfig *mc = new MissileConfig();
					mc->Name = LuaToString(l, -1, i + 1);
					this->Impact.push_back(mc);
				}
			}
		} else if (!strcmp(value, "SmokeMissile")) {
			this->Smoke.Name = LuaToString(l, -1);
		} else if (!strcmp(value, "ImpactParticle")) {
			this->ImpactParticle = new LuaCallback(l, -1);
		} else if (!strcmp(value, "SmokeParticle")) {
			this->SmokeParticle = new LuaCallback(l, -1);
		} else if (!strcmp(value, "OnImpact")) {
			this->OnImpact = new LuaCallback(l, -1);
		} else if (!strcmp(value, "CanHitOwner")) {
			this->CanHitOwner = LuaToBoolean(l, -1);
		} else if (!strcmp(value, "AlwaysFire")) {
			this->AlwaysFire = LuaToBoolean(l, -1);
		} else if (!strcmp(value, "Pierce")) {
			this->Pierce = LuaToBoolean(l, -1);
		} else if (!strcmp(value, "PierceOnce")) {
			this->PierceOnce = LuaToBoolean(l, -1);
		} else if (!strcmp(value, "IgnoreWalls")) {
			this->IgnoreWalls = LuaToBoolean(l, -1);
		} else if (!strcmp(value, "KillFirstUnit")) {
			this->KillFirstUnit = LuaToBoolean(l, -1);
		} else if (!strcmp(value, "FriendlyFire")) {
			this->FriendlyFire = LuaToBoolean(l, -1);
		} else if (!strcmp(value, "SplashFactor")) {
			this->SplashFactor = LuaToNumber(l, -1);
		} else if (!strcmp(value, "CorrectSphashDamage")) {
			this->CorrectSphashDamage = LuaToBoolean(l, -1);
		} else {
			LuaError(l, "Unsupported tag: %s" _C_ value);
		}
	}

	if (!this->SmokePrecision) {
		this->SmokePrecision = this->Speed;
	}
	if (!file.empty()) {
		this->G = CGraphic::New(file, this->Width(), this->Height());
	}

}

/**
**  Parse missile-type.
**
**  @param l  Lua state.
*/
static int CclDefineMissileType(lua_State *l)
{
	LuaCheckArgs(l, 2);
	if (!lua_istable(l, 2)) {
		LuaError(l, "incorrect argument");
	}

	// Slot identifier
	const char *str = LuaToString(l, 1);
	MissileType *mtype = MissileTypeByIdent(str);

	if (mtype) {
		DebugPrint("Redefining missile-type `%s'\n" _C_ str);
	} else {
		mtype = NewMissileTypeSlot(str);
	}
	mtype->Load(l);
	return 0;
}

/**
**  Create a missile.
**
**  @param l  Lua state.
*/
static int CclMissile(lua_State *l)
{
	MissileType *type = NULL;
	PixelPos position(-1, -1);
	PixelPos destination(-1, -1);
	PixelPos source(-1, -1);
	Missile *missile = NULL;

	DebugPrint("FIXME: not finished\n");

	const int args = lua_gettop(l);
	for (int j = 0; j < args; ++j) {
		const char *value = LuaToString(l, j + 1);
		++j;

		if (!strcmp(value, "type")) {
			type = MissileTypeByIdent(LuaToString(l, j + 1));
		} else if (!strcmp(value, "pos")) {
			CclGetPos(l, &position.x, &position.y, j + 1);
		} else if (!strcmp(value, "origin-pos")) {
			CclGetPos(l, &source.x, &source.y, j + 1);
		} else if (!strcmp(value, "goal")) {
			CclGetPos(l, &destination.x, &destination.y, j + 1);
		} else if (!strcmp(value, "local")) {
			Assert(type);
			missile = MakeLocalMissile(*type, position, destination);
			missile->Local = 1;
			--j;
		} else if (!strcmp(value, "global")) {
			Assert(type);
			missile = MakeMissile(*type, position, destination);
			missile->position = position;
			missile->source = source;
			missile->destination = destination;
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
			lua_pushvalue(l, j + 1);
			missile->SourceUnit = CclGetUnitFromRef(l);
			lua_pop(l, 1);
		} else if (!strcmp(value, "target")) {
			Assert(missile);
			lua_pushvalue(l, j + 1);
			missile->TargetUnit = CclGetUnitFromRef(l);
			lua_pop(l, 1);
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
			if (!lua_istable(l, j + 1) || lua_rawlen(l, j + 1) != 2) {
				LuaError(l, "incorrect argument");
			}
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
	missile->position = position;
	missile->source = source;
	missile->destination = destination;
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

	const int args = lua_gettop(l);
	for (int j = 0; j < args; ++j) {
		if (!lua_istable(l, j + 1)) {
			LuaError(l, "incorrect argument");
		}
		BurningBuildingFrame *ptr = new BurningBuildingFrame;
		const int subargs = lua_rawlen(l, j + 1);

		for (int k = 0; k < subargs; ++k) {
			const char *value = LuaToString(l, j + 1, k + 1);
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
**  Create a missile on the map
**
**  @param l  Lua state.
**
*/
static int CclCreateMissile(lua_State *l)
{
	const int arg = lua_gettop(l);
	if (arg < 6 || arg > 7) {
		LuaError(l, "incorrect argument");
	}

	const std::string name = LuaToString(l, 1);
	const MissileType *mtype = MissileTypeByIdent(name);
	if (!mtype) {
		LuaError(l, "Bad missile");
	}
	PixelPos startpos, endpos;
	CclGetPos(l, &startpos.x, &startpos.y, 2);
	CclGetPos(l, &endpos.x, &endpos.y, 3);

	const int sourceUnitId = LuaToNumber(l, 4);
	const int destUnitId = LuaToNumber(l, 5);
	const bool dealDamage = LuaToBoolean(l, 6);
	const bool mapRelative = arg == 7 ? LuaToBoolean(l, 7) : false;
	CUnit *sourceUnit = sourceUnitId != -1 ? &UnitManager.GetSlotUnit(sourceUnitId) : NULL;
	CUnit *destUnit = destUnitId != -1 ? &UnitManager.GetSlotUnit(destUnitId) : NULL;

	if (mapRelative == false) {
		if (sourceUnit != NULL) {
			startpos += sourceUnit->GetMapPixelPosTopLeft();
		}
		if (destUnit != NULL) {
			endpos += destUnit->GetMapPixelPosTopLeft();
		}
	}

	Missile *missile = MakeMissile(*mtype, startpos, endpos);
	if (!missile) {
		return 0;
	}
	if (dealDamage) {
		missile->SourceUnit = sourceUnit;
	}
	missile->TargetUnit = destUnit;
	return 0;
}

/**
**  Register CCL features for missile-type.
*/
void MissileCclRegister()
{
	lua_register(Lua, "DefineMissileType", CclDefineMissileType);
	lua_register(Lua, "Missile", CclMissile);
	lua_register(Lua, "DefineBurningBuilding", CclDefineBurningBuilding);
	lua_register(Lua, "CreateMissile", CclCreateMissile);
}

//@}
