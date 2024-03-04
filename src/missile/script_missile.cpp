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

#include <map>
#include <string_view>

/*----------------------------------------------------------------------------
--  Variables
----------------------------------------------------------------------------*/

/**
**  Missile class names, used to load/save the missiles.
*/
static const std::map<std::string_view, MissileClass> MissileClassNames = {
	{"missile-class-clip-to-target", MissileClass::ClipToTarget},
	{"missile-class-continious", MissileClass::Continuous},
	{"missile-class-cycle-once", MissileClass::CycleOnce},
	{"missile-class-death-coil", MissileClass::DeathCoil},
	{"missile-class-fire", MissileClass::Fire},
	{"missile-class-flame-shield", MissileClass::FlameShield},
	{"missile-class-hit", MissileClass::Hit},
	{"missile-class-land-mine", MissileClass::LandMine},
	{"missile-class-none", MissileClass::Nothing},
	{"missile-class-parabolic", MissileClass::Parabolic},
	{"missile-class-point-to-point", MissileClass::PointToPoint},
	{"missile-class-point-to-point-bounce", MissileClass::PointToPointBounce},
	{"missile-class-point-to-point-cycle-once", MissileClass::PointToPointCycleOnce},
	{"missile-class-point-to-point-with-hit", MissileClass::PointToPointWithHit},
	{"missile-class-stay", MissileClass::Stay},
	{"missile-class-straight-fly",MissileClass::StraightFly},
	{"missile-class-tracer", MissileClass::Tracer},
	{"missile-class-whirlwind", MissileClass::Whirlwind}
};

/*----------------------------------------------------------------------------
--  Functions
----------------------------------------------------------------------------*/

void MissileType::Load(lua_State *l)
{
	if (this->G) {
		// reload, just ensure no div by zero
		this->NumDirections = this->NumDirections ? this->NumDirections : 1;
		this->SplashFactor = this->SplashFactor ? this->SplashFactor : 100;
	} else {
		this->NumDirections = 1;
		this->Flip = true;
		this->SplashFactor = 100;
	}

	// Parse the arguments
	std::string file;
	for (lua_pushnil(l); lua_next(l, 2); lua_pop(l, 1)) {
		const std::string_view value = LuaToString(l, -2);

		if (value == "File") {
			file = LuaToString(l, -1);
		} else if (value == "Size") {
			CclGetPos(l, &this->size);
		} else if (value == "Frames") {
			this->SpriteFrames = LuaToNumber(l, -1);
		} else if (value == "Flip") {
			this->Flip = LuaToBoolean(l, -1);
		} else if (value == "NumDirections") {
			this->NumDirections = LuaToNumber(l, -1);
		} else if (value == "Transparency") {
			this->Transparency = LuaToNumber(l, -1);
		} else if (value == "FiredSound") {
			this->FiredSound.Name = LuaToString(l, -1);
		} else if (value == "ImpactSound") {
			this->ImpactSound.Name = LuaToString(l, -1);
		} else if (value == "ChangeVariable") {
			const int index = UnitTypeVar.VariableNameLookup[LuaToString(l, -1)];// User variables
			if (index == -1) {
				ErrorPrint("Bad variable name '%s'\n", LuaToString(l, -1).data());
				Exit(1);
				return;
			}
			this->ChangeVariable = index;
		} else if (value == "ChangeAmount") {
			this->ChangeAmount = LuaToNumber(l, -1);
		} else if (value == "ChangeMax") {
			this->ChangeMax = LuaToBoolean(l, -1);
		} else if (value == "Class") {
			const std::string_view className = LuaToString(l, -1);
			const auto it = MissileClassNames.find(className);
			if (it != MissileClassNames.end()) {
				this->Class = it->second;
			} else {
				LuaError(l, "Unsupported class: %s", className.data());
			}
		} else if (value == "NumBounces") {
			this->NumBounces = LuaToNumber(l, -1);
		} else if (value == "ParabolCoefficient") {
			this->ParabolCoefficient = LuaToNumber(l, -1);
		} else if (value == "Delay") {
			this->StartDelay = LuaToNumber(l, -1);
		} else if (value == "Sleep") {
			this->Sleep = LuaToNumber(l, -1);
		} else if (value == "Speed") {
			this->Speed = LuaToNumber(l, -1);
		} else if (value == "BlizzardSpeed") {
			this->BlizzardSpeed = LuaToNumber(l, -1);
		} else if (value == "TTL") {
			this->TTL = LuaToNumber(l, -1);
		} else if (value == "Damage") {
			this->Damage = CclParseNumberDesc(l);
			lua_pushnil(l);
		} else if (value == "ReduceFactor") {
			this->ReduceFactor = LuaToNumber(l, -1);
		} else if (value == "SmokePrecision") {
			this->SmokePrecision = LuaToNumber(l, -1);
		} else if (value == "MissileStopFlags") {
			this->MissileStopFlags = LuaToNumber(l, -1);
		} else if (value == "DrawLevel") {
			this->DrawLevel = LuaToNumber(l, -1);
		} else if (value == "Range") {
			this->Range = LuaToNumber(l, -1);
		} else if (value == "ImpactMissile") {
			if (!lua_istable(l, -1)) {
				MissileConfig mc{};
				mc.Name = LuaToString(l, -1);
				this->Impact.push_back(std::move(mc));
			} else {
				const int impacts = lua_rawlen(l, -1);
				for (int i = 0; i < impacts; ++i) {
					MissileConfig mc{};
					mc.Name = LuaToString(l, -1, i + 1);
					this->Impact.push_back(std::move(mc));
				}
			}
		} else if (value == "SmokeMissile") {
			this->Smoke.Name = LuaToString(l, -1);
		} else if (value == "ImpactParticle") {
			this->ImpactParticle = LuaCallback(l, -1);
		} else if (value == "SmokeParticle") {
			this->SmokeParticle = LuaCallback(l, -1);
		} else if (value == "OnImpact") {
			this->OnImpact = LuaCallback(l, -1);
		} else if (value == "CanHitOwner") {
			this->CanHitOwner = LuaToBoolean(l, -1);
		} else if (value == "AlwaysFire") {
			this->AlwaysFire = LuaToBoolean(l, -1);
		} else if (value == "Pierce") {
			this->Pierce = LuaToBoolean(l, -1);
		} else if (value == "PierceOnce") {
			this->PierceOnce = LuaToBoolean(l, -1);
		} else if (value == "IgnoreWalls") {
			this->IgnoreWalls = LuaToBoolean(l, -1);
		} else if (value == "KillFirstUnit") {
			this->KillFirstUnit = LuaToBoolean(l, -1);
		} else if (value == "FriendlyFire") {
			this->FriendlyFire = LuaToBoolean(l, -1);
		} else if (value == "SplashFactor") {
			this->SplashFactor = LuaToNumber(l, -1);
		} else if (value == "CorrectSphashDamage") {
			this->CorrectSphashDamage = LuaToBoolean(l, -1);
		} else {
			LuaError(l, "Unsupported tag: %s", value.data());
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
	const std::string str = std::string{LuaToString(l, 1)};
	MissileType *mtype = NewMissileTypeSlot(str);

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
	MissileType *type = nullptr;
	PixelPos position(-1, -1);
	PixelPos destination(-1, -1);
	PixelPos source(-1, -1);
	Missile *missile = nullptr;

	DebugPrint("FIXME: not finished\n");

	const int args = lua_gettop(l);
	for (int j = 0; j < args; ++j) {
		const std::string_view value = LuaToString(l, j + 1);
		++j;

		if (value == "type") {
			type = &MissileTypeByIdent(LuaToString(l, j + 1));
		} else if (value == "pos") {
			CclGetPos(l, &position, j + 1);
		} else if (value == "origin-pos") {
			CclGetPos(l, &source, j + 1);
		} else if (value == "goal") {
			CclGetPos(l, &destination, j + 1);
		} else if (value == "local") {
			Assert(type);
			missile = MakeLocalMissile(*type, position, destination);
			missile->Local = 1;
			--j;
		} else if (value == "global") {
			Assert(type);
			missile = MakeMissile(*type, position, destination);
			missile->position = position;
			missile->source = source;
			missile->destination = destination;
			missile->Local = 0;
			--j;
		} else if (value == "frame") {
			Assert(missile);
			missile->SpriteFrame = LuaToNumber(l, j + 1);
		} else if (value == "state") {
			Assert(missile);
			missile->State = LuaToNumber(l, j + 1);
		} else if (value == "anim-wait") {
			Assert(missile);
			missile->AnimWait = LuaToNumber(l, j + 1);
		} else if (value == "wait") {
			Assert(missile);
			missile->Wait = LuaToNumber(l, j + 1);
		} else if (value == "delay") {
			Assert(missile);
			missile->Delay = LuaToNumber(l, j + 1);
		} else if (value == "source") {
			Assert(missile);
			lua_pushvalue(l, j + 1);
			missile->SourceUnit = CclGetUnitFromRef(l);
			lua_pop(l, 1);
		} else if (value == "target") {
			Assert(missile);
			lua_pushvalue(l, j + 1);
			missile->TargetUnit = CclGetUnitFromRef(l);
			lua_pop(l, 1);
		} else if (value == "damage") {
			Assert(missile);
			missile->Damage = LuaToNumber(l, j + 1);
		} else if (value == "ttl") {
			Assert(missile);
			missile->TTL = LuaToNumber(l, j + 1);
		} else if (value == "hidden") {
			Assert(missile);
			missile->Hidden = 1;
			--j;
		} else if (value == "step") {
			Assert(missile);
			if (!lua_istable(l, j + 1) || lua_rawlen(l, j + 1) != 2) {
				LuaError(l, "incorrect argument");
			}
			missile->CurrentStep = LuaToNumber(l, j + 1, 1);
			missile->TotalStep = LuaToNumber(l, j + 1, 2);
		} else {
			LuaError(l, "Unsupported tag: %s", value.data());
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
	BurningBuildingFrames.clear();

	const int args = lua_gettop(l);
	for (int j = 0; j < args; ++j) {
		if (!lua_istable(l, j + 1)) {
			LuaError(l, "incorrect argument");
		}
		auto ptr = std::make_unique<BurningBuildingFrame>();
		const int subargs = lua_rawlen(l, j + 1);

		for (int k = 0; k < subargs; ++k) {
			const std::string_view value = LuaToString(l, j + 1, k + 1);
			++k;

			if (value == "percent") {
				ptr->Percent = LuaToNumber(l, j + 1, k + 1);
			} else if (value == "missile") {
				ptr->Missile = &MissileTypeByIdent(LuaToString(l, j + 1, k + 1));
			}
		}
		BurningBuildingFrames.insert(BurningBuildingFrames.begin(), std::move(ptr));
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

	const std::string_view name = LuaToString(l, 1);
	const MissileType &mtype = MissileTypeByIdent(name);
	PixelPos startpos, endpos;
	CclGetPos(l, &startpos, 2);
	CclGetPos(l, &endpos, 3);

	const int sourceUnitId = LuaToNumber(l, 4);
	const int destUnitId = LuaToNumber(l, 5);
	const bool dealDamage = LuaToBoolean(l, 6);
	const bool mapRelative = arg == 7 ? LuaToBoolean(l, 7) : false;
	CUnit *sourceUnit = sourceUnitId != -1 ? &UnitManager->GetSlotUnit(sourceUnitId) : nullptr;
	CUnit *destUnit = destUnitId != -1 ? &UnitManager->GetSlotUnit(destUnitId) : nullptr;

	if (mapRelative == false) {
		if (sourceUnit != nullptr) {
			startpos += sourceUnit->GetMapPixelPosTopLeft();
		}
		if (destUnit != nullptr) {
			endpos += destUnit->GetMapPixelPosTopLeft();
		}
	}

	Missile *missile = MakeMissile(mtype, startpos, endpos);
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
