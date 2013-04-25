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
/**@name spell_polymorph.cpp - The spell Polymorph. */
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

#include "spell/spell_polymorph.h"

#include "game.h"
#include "map.h"
#include "script.h"
#include "unit.h"

/* virtual */ void Spell_Polymorph::Parse(lua_State *l, int startIndex, int endIndex)
{
	for (int j = startIndex; j < endIndex; ++j) {
		const char *value = LuaToString(l, -1, j + 1);
		++j;
		if (!strcmp(value, "new-form")) {
			value = LuaToString(l, -1, j + 1);
			this->NewForm = UnitTypeByIdent(value);
			if (!this->NewForm) {
				this->NewForm = 0;
				DebugPrint("unit type \"%s\" not found for polymorph spell.\n" _C_ value);
			}
			// FIXME: temp polymorphs? hard to do.
		} else if (!strcmp(value, "player-neutral")) {
			this->PlayerNeutral = 1;
			--j;
		} else if (!strcmp(value, "player-caster")) {
			this->PlayerNeutral = 2;
			--j;
		} else {
			LuaError(l, "Unsupported polymorph tag: %s" _C_ value);
		}
	}
	// Now, checking value.
	if (this->NewForm == NULL) {
		LuaError(l, "Use a unittype for polymorph (with new-form)");
	}
}

/**
** Cast polymorph.
**
**  @param caster       Unit that casts the spell
**  @param spell        Spell-type pointer
**  @param target       Target unit that spell is addressed to
**  @param goalPos      coord of target spot when/if target does not exist
**
**  @return             =!0 if spell should be repeated, 0 if not
*/
/* virtual */ int Spell_Polymorph::Cast(CUnit &caster, const SpellType &spell, CUnit *target, const Vec2i &goalPos)
{
	if (!target) {
		return 0;
	}
	CUnitType &type = *this->NewForm;
	const Vec2i pos(goalPos - type.GetHalfTileSize());

	caster.Player->Score += target->Variable[POINTS_INDEX].Value;
	if (caster.IsEnemy(*target)) {
		if (target->Type->Building) {
			caster.Player->TotalRazings++;
		} else {
			caster.Player->TotalKills++;
		}
		if (UseHPForXp) {
			caster.Variable[XP_INDEX].Max += target->Variable[HP_INDEX].Value;
		} else {
			caster.Variable[XP_INDEX].Max += target->Variable[POINTS_INDEX].Value;
		}
		caster.Variable[XP_INDEX].Value = caster.Variable[XP_INDEX].Max;
		caster.Variable[KILL_INDEX].Value++;
		caster.Variable[KILL_INDEX].Max++;
		caster.Variable[KILL_INDEX].Enable = 1;
	}

	// as said somewhere else -- no corpses :)
	target->Remove(NULL);
	Vec2i offset;
	for (offset.x = 0; offset.x < type.TileWidth; ++offset.x) {
		for (offset.y = 0; offset.y < type.TileHeight; ++offset.y) {
			if (!UnitTypeCanBeAt(type, pos + offset)) {
				target->Place(target->tilePos);
				return 0;
			}
		}
	}
	caster.Variable[MANA_INDEX].Value -= spell.ManaCost;
	if (this->PlayerNeutral == 1) {
		MakeUnitAndPlace(pos, type, Players + PlayerNumNeutral);
	} else if (this->PlayerNeutral == 2) {
		MakeUnitAndPlace(pos, type, caster.Player);
	} else {
		MakeUnitAndPlace(pos, type, target->Player);
	}
	UnitLost(*target);
	UnitClearOrders(*target);
	target->Release();
	return 1;
}

//@}
