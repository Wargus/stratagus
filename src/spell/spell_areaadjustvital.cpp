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
/**@name spell_areaadjustvital.cpp - The spell AreaAdjustVital. */
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

#include "spell/spell_areaadjustvital.h"

#include "script.h"
#include "unit.h"
#include "unit_find.h"

/* virtual */ void Spell_AreaAdjustVital::Parse(lua_State *l, int startIndex, int endIndex)
{
	for (int j = startIndex; j < endIndex; ++j) {
		const char *value = LuaToString(l, -1, j + 1);
		++j;
		if (!strcmp(value, "hit-points")) {
			this->HP = LuaToNumber(l, -1, j + 1);
		} else if (!strcmp(value, "mana-points")) {
			this->Mana = LuaToNumber(l, -1, j + 1);
		} else if (!strcmp(value, "shield-points")) {
			this->Shield = LuaToNumber(l, -1, j + 1);
		} else if (!strcmp(value, "range")) {
			this->Range = LuaToNumber(l, -1, j + 1);
		} else if (!strcmp(value, "use-mana")) {
			this->UseMana = LuaToBoolean(l, -1, j + 1);
		} else {
			LuaError(l, "Unsupported area-adjust-vitals tag: %s" _C_ value);
		}
	}
}

/**
** Cast Area Adjust Vital on all valid units in range.
**
**  @param caster       Unit that casts the spell
**  @param spell        Spell-type pointer
**  @param target       Target unit that spell is addressed to
**  @param goalPos      TilePos of target spot when/if target does not exist
**
**  @return             =!0 if spell should be repeated, 0 if not
*/
/* virtual */ int Spell_AreaAdjustVital::Cast(CUnit &caster, const SpellType &spell, CUnit *target, const Vec2i &goalPos)
{
	const Vec2i range(this->Range, this->Range);
	const Vec2i typeSize(caster.Type->Width, caster.Type->Height);
	std::vector<CUnit *> units;

	// Get all the units around the unit
	Select(goalPos - range, goalPos + typeSize + range, units);
	int hp = this->HP;
	int mana = this->Mana;
	int shield = this->Shield;
	for (size_t j = 0; j != units.size(); ++j) {
		target = units[j];
		// if (!PassCondition(caster, spell, target, goalPos) {
		if (!CanCastSpell(caster, spell, target, goalPos)) {
			continue;
		}
		if (hp < 0) {
			HitUnit(&caster, *target, -hp);
		} else {
			target->Variable[HP_INDEX].Value += hp;
			target->Variable[HP_INDEX].Value = std::min(target->Variable[HP_INDEX].Max, target->Variable[HP_INDEX].Value);
		}
		target->Variable[MANA_INDEX].Value += mana;
		clamp(&target->Variable[MANA_INDEX].Value, 0, target->Variable[MANA_INDEX].Max);
		target->Variable[SHIELD_INDEX].Value += shield;
		clamp(&target->Variable[SHIELD_INDEX].Value, 0, target->Variable[SHIELD_INDEX].Max);
	}
	if (UseMana) {
		caster.Variable[MANA_INDEX].Value -= spell.ManaCost;
	}
	return 0;
}

//@}
