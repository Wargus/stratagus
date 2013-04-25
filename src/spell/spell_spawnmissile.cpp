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
/**@name spell_spawnmissile.cpp - The spell SpawnMissile. */
//
//      (c) Copyright 1998-2012 by Vladi Belperchinov-Shabanski, Lutz Sammer,
//                                 Jimmy Salmon, and Joris DAUPHIN
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

#include "stratagus.h"

#include "spell/spell_spawnmissile.h"

#include "map.h"
#include "missile.h"
#include "script.h"
#include "unit.h"

/**
**  Parse the missile location description for a spell action.
**
**  @param l         Lua state.
**  @param location  Pointer to missile location description.
**
**  @note This is only here to avoid code duplication. You don't have
**        any reason to USE this:)
*/
static void CclSpellMissileLocation(lua_State *l, SpellActionMissileLocation *location)
{
	Assert(location != NULL);

	if (!lua_istable(l, -1)) {
		LuaError(l, "incorrect argument");
	}
	const int args = lua_rawlen(l, -1);
	for (int j = 0; j < args; ++j) {
		const char *value = LuaToString(l, -1, j + 1);
		++j;
		if (!strcmp(value, "base")) {
			value = LuaToString(l, -1, j + 1);
			if (!strcmp(value, "caster")) {
				location->Base = LocBaseCaster;
			} else if (!strcmp(value, "target")) {
				location->Base = LocBaseTarget;
			} else {
				LuaError(l, "Unsupported missile location base flag: %s" _C_ value);
			}
		} else if (!strcmp(value, "add-x")) {
			location->AddX = LuaToNumber(l, -1, j + 1);
		} else if (!strcmp(value, "add-y")) {
			location->AddY = LuaToNumber(l, -1, j + 1);
		} else if (!strcmp(value, "add-rand-x")) {
			location->AddRandX = LuaToNumber(l, -1, j + 1);
		} else if (!strcmp(value, "add-rand-y")) {
			location->AddRandY = LuaToNumber(l, -1, j + 1);
		} else {
			LuaError(l, "Unsupported missile location description flag: %s" _C_ value);
		}
	}
}

/* virtual */ void Spell_SpawnMissile::Parse(lua_State *l, int startIndex, int endIndex)
{
	for (int j = startIndex; j < endIndex; ++j) {
		const char *value = LuaToString(l, -1, j + 1);
		++j;
		if (!strcmp(value, "damage")) {
			this->Damage = LuaToNumber(l, -1, j + 1);
		} else if (!strcmp(value, "use-unit-var")) {
			this->UseUnitVar = true;
			--j;
		} else if (!strcmp(value, "delay")) {
			this->Delay = LuaToNumber(l, -1, j + 1);
		} else if (!strcmp(value, "ttl")) {
			this->TTL = LuaToNumber(l, -1, j + 1);
		} else if (!strcmp(value, "start-point")) {
			lua_rawgeti(l, -1, j + 1);
			CclSpellMissileLocation(l, &this->StartPoint);
			lua_pop(l, 1);
		} else if (!strcmp(value, "end-point")) {
			lua_rawgeti(l, -1, j + 1);
			CclSpellMissileLocation(l, &this->EndPoint);
			lua_pop(l, 1);
		} else if (!strcmp(value, "missile")) {
			value = LuaToString(l, -1, j + 1);
			this->Missile = MissileTypeByIdent(value);
			if (this->Missile == NULL) {
				DebugPrint("in spawn-missile : missile %s does not exist\n" _C_ value);
			}
		} else {
			LuaError(l, "Unsupported spawn-missile tag: %s" _C_ value);
		}
	}
	// Now, checking value.
	if (this->Missile == NULL) {
		LuaError(l, "Use a missile for spawn-missile (with missile)");
	}
}

/**
** Evaluate missile location description.
**
** @param location     Parameters for location.
** @param caster       Unit that casts the spell
** @param target       Target unit that spell is addressed to
** @param goalPos      TilePos of target spot when/if target does not exist
** @param res          pointer to PixelPos of the result
*/
static void EvaluateMissileLocation(const SpellActionMissileLocation &location,
									CUnit &caster, CUnit *target, const Vec2i &goalPos, PixelPos *res)
{
	if (location.Base == LocBaseCaster) {
		*res = caster.GetMapPixelPosCenter();
	} else {
		if (target) {
			*res = target->GetMapPixelPosCenter();
		} else {
			*res = Map.TilePosToMapPixelPos_Center(goalPos);
		}
	}
	res->x += location.AddX;
	if (location.AddRandX) {
		res->x += SyncRand() % location.AddRandX;
	}
	res->y += location.AddY;
	if (location.AddRandY) {
		res->y += SyncRand() % location.AddRandY;
	}
}

/**
** Cast spawn missile.
**
**  @param caster       Unit that casts the spell
**  @param spell        Spell-type pointer
**  @param target       Target unit that spell is addressed to
**  @param goalPos      TilePos of target spot when/if target does not exist
**
**  @return             =!0 if spell should be repeated, 0 if not
*/
/* virtual */ int Spell_SpawnMissile::Cast(CUnit &caster, const SpellType &, CUnit *target, const Vec2i &goalPos)
{
	PixelPos startPos;
	PixelPos endPos;

	EvaluateMissileLocation(this->StartPoint, caster, target, goalPos, &startPos);
	EvaluateMissileLocation(this->EndPoint, caster, target, goalPos, &endPos);

	::Missile *missile = MakeMissile(*this->Missile, startPos, endPos);
	missile->TTL = this->TTL;
	missile->Delay = this->Delay;
	missile->Damage = this->Damage;
	if (this->UseUnitVar) {
		missile->Damage = 0;
		missile->SourceUnit = &caster;
	} else if (missile->Damage != 0) {
		missile->SourceUnit = &caster;
	}
	missile->TargetUnit = target;
	return 1;
}


//@}
