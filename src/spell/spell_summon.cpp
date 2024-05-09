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
/**@name spell_summon.cpp - The spell Summon. */
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

#include "action/action_defend.h"

#include "spell/spell_summon.h"

#include "../ai/ai_local.h"

#include "actions.h"
#include "commands.h"
#include "script.h"
#include "unit.h"
#include "unit_find.h"

void Spell_Summon::Parse(lua_State *l, int startIndex, int endIndex) /* override */
{
	for (int j = startIndex; j < endIndex; ++j) {
		std::string_view value = LuaToString(l, -1, j + 1);
		++j;
		if (value == "unit-type") {
			value = LuaToString(l, -1, j + 1);
			this->UnitType = &UnitTypeByIdent(value);
		} else if (value == "time-to-live") {
			this->TTL = LuaToNumber(l, -1, j + 1);
		} else if (value == "require-corpse") {
			this->RequireCorpse = true;
			--j;
		} else if (value == "join-to-ai-force") {
			this->JoinToAiForce = true;
			--j;
		} else {
			LuaError(l, "Unsupported summon tag: %s", value.data());
		}
	}
	// Now, checking value.
	if (this->UnitType == nullptr) {
		LuaError(l, "Use a unittype for summon (with unit-type)");
	}
}

/**
**  Cast summon spell.
**
**  @param caster       Unit that casts the spell
**  @param spell        Spell-type pointer
**  @param target       Target unit that spell is addressed to
**  @param goalPos      coord of target spot when/if target does not exist
**
**  @return             =!0 if spell should be repeated, 0 if not
*/
int Spell_Summon::Cast(CUnit &caster,
                       const SpellType &spell,
                       CUnit *&target,
                       const Vec2i &goalPos) /* override */
{
	Vec2i pos = goalPos;
	bool cansummon;
	CUnitType &unittype = *this->UnitType;
	int ttl = this->TTL;

	if (this->RequireCorpse) {
		const Vec2i offset(1, 1);
		const Vec2i minPos = pos - offset;
		const Vec2i maxPos = pos + offset;

		CUnit *unit = FindUnit_If(minPos, maxPos, [](const CUnit* unit) {
			return unit->CurrentAction() == UnitAction::Die && !unit->Type->Building;
		});
		cansummon = false;

		if (unit != nullptr) { //  Found a corpse. eliminate it and proceed to summoning.
			pos = unit->tilePos;
			unit->Remove(nullptr);
			unit->Release();
			cansummon = true;
		}
	} else {
		cansummon = true;
	}

	if (cansummon) {
		DebugPrint("Summoning a %s\n", unittype.Name.c_str());

		//
		// Create units.
		// FIXME: do summoned units count on food?
		//
		target = MakeUnit(unittype, caster.Player);
		if (target != nullptr) {
			target->tilePos = pos;
			DropOutOnSide(*target, LookingW, nullptr);
			// To avoid defending summoned unit for AI
			// we also use this value to store when this
			// unit was summoned
			target->Summoned = GameCycle + 1;
			//
			//  set life span. ttl=0 results in a permanent unit.
			//
			if (ttl) {
				target->TTL = GameCycle + ttl;
			}

			// Insert summoned unit to AI force so it will help them in battle
			if (this->JoinToAiForce && caster.Player->AiEnabled) {
				if (auto force = caster.Player->Ai->Force.GetForce(caster)) {
					caster.Player->Ai->Force[*force].Insert(*target);
					target->GroupId = caster.GroupId;
					CommandDefend(*target, caster, EFlushMode::On);
				}
			}

			caster.Variable[MANA_INDEX].Value -= spell.ManaCost;
		} else {
			DebugPrint("Unable to allocate Unit");
		}
		return 1;
	}
	return 0;
}

//@}
