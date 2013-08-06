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
/**@name spell_adjustvital.cpp - The spell AdjustVital. */
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

#include "spell/spell_adjustvital.h"

#include "script.h"
#include "unit.h"

/* virtual */ void Spell_AdjustVital::Parse(lua_State *l, int startIndex, int endIndex)
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
		} else if (!strcmp(value, "max-multi-cast")) {
			this->MaxMultiCast = LuaToNumber(l, -1, j + 1);
		} else {
			LuaError(l, "Unsupported adjust-vitals tag: %s" _C_ value);
		}
	}
}


/**
** Cast healing. (or exorcism)
**
**  @param caster       Unit that casts the spell
**  @param spell        Spell-type pointer
**  @param target       Target unit that spell is addressed to
**  @param goalPos      coord of target spot when/if target does not exist
**
**  @return             =!0 if spell should be repeated, 0 if not
*/
/* virtual */ int Spell_AdjustVital::Cast(CUnit &caster, const SpellType &spell, CUnit *target, const Vec2i &/*goalPos*/)
{
	if (!target) {
		return 0;
	}

	const int hp = this->HP;
	const int mana = this->Mana;
	const int shield = this->Shield;
	const int manacost = spell.ManaCost;
	int diffHP;
	int diffMana;
	int diffShield;

	//  Healing and harming
	if (hp > 0) {
		diffHP = target->Variable[HP_INDEX].Max - target->Variable[HP_INDEX].Value;
	} else {
		diffHP = target->Variable[HP_INDEX].Value;
	}
	if (mana > 0) {
		diffMana = target->Stats->Variables[MANA_INDEX].Max - target->Variable[MANA_INDEX].Value;
	} else {
		diffMana = target->Variable[MANA_INDEX].Value;
	}

	if (shield > 0) {
		diffShield = target->Stats->Variables[SHIELD_INDEX].Max - target->Variable[SHIELD_INDEX].Value;
	} else {
		diffShield = target->Variable[SHIELD_INDEX].Value;
	}

	//  When harming cast again to send the hp to negative values.
	//  Careful, a perfect 0 target hp kills too.
	//  Avoid div by 0 errors too!
	int castcount = 1;
	if (hp) {
		castcount = std::max<int>(castcount,
								  diffHP / abs(hp) + (((hp < 0) && (diffHP % (-hp) > 0)) ? 1 : 0));
	}
	if (mana) {
		castcount = std::max<int>(castcount,
								  diffMana / abs(mana) + (((mana < 0) && (diffMana % (-mana) > 0)) ? 1 : 0));
	}
	if (shield) {
		castcount = std::max<int>(castcount,
								  diffShield / abs(shield) + (((shield < 0) && (diffShield % (-shield) > 0)) ? 1 : 0));
	}
	if (manacost) {
		castcount = std::min<int>(castcount, caster.Variable[MANA_INDEX].Value / manacost);
	}
	if (this->MaxMultiCast) {
		castcount = std::min<int>(castcount, this->MaxMultiCast);
	}

	caster.Variable[MANA_INDEX].Value -= castcount * manacost;
	if (hp < 0) {
		if (&caster != target) {
			HitUnit(&caster, *target, -(castcount * hp));
		} else {
			target->Variable[HP_INDEX].Value += castcount * hp;
			target->Variable[HP_INDEX].Value = std::max(target->Variable[HP_INDEX].Value, 0);
		}
	} else {
		target->Variable[HP_INDEX].Value += castcount * hp;
		target->Variable[HP_INDEX].Value = std::min(target->Variable[HP_INDEX].Max, target->Variable[HP_INDEX].Value);
	}
	target->Variable[MANA_INDEX].Value += castcount * mana;
	clamp(&target->Variable[MANA_INDEX].Value, 0, target->Variable[MANA_INDEX].Max);
	target->Variable[SHIELD_INDEX].Value += castcount * shield;
	clamp(&target->Variable[SHIELD_INDEX].Value, 0, target->Variable[SHIELD_INDEX].Max);

	if (spell.RepeatCast) {
		return 1;
	}
	return 0;
}

//@}
