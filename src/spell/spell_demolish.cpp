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
/**@name spell_demolish.cpp - The spell demolish. */
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

#include "spell/spell_demolish.h"

#include "script.h"
#include "map.h"
#include "unit.h"
#include "unit_find.h"

void Spell_Demolish::Parse(lua_State *l, int startIndex, int endIndex) /* override */
{
	for (int j = startIndex; j < endIndex; ++j) {
		const std::string_view value = LuaToString(l, -1, j + 1);
		++j;
		if (value == "range") {
			this->Range = LuaToNumber(l, -1, j + 1);
		} else if (value == "damage") {
			this->Damage = LuaToNumber(l, -1, j + 1);
		} else {
			LuaError(l, "Unsupported demolish tag: %s", value.data());
		}
	}
}

/**
**  Cast demolish
**  @param caster       Unit that casts the spell
**  @param spell        Spell-type pointer
**  @param target       Target unit that spell is addressed to
**  @param goalPos      tilePos of target spot when/if target does not exist
**
**  @return             =!0 if spell should be repeated, 0 if not
*/
int Spell_Demolish::Cast(CUnit &caster,
                         const SpellType &,
                         CUnit *&,
                         const Vec2i &goalPos) /* override */
{
	// Allow error margins. (Lame, I know)
	const Vec2i offset(this->Range + 2, this->Range + 2);
	Vec2i minpos = goalPos - offset;
	Vec2i maxpos = goalPos + offset;

	Map.FixSelectionArea(minpos, maxpos);

	//
	// Terrain effect of the explosion
	//
	Vec2i ipos;
	for (ipos.x = minpos.x; ipos.x <= maxpos.x; ++ipos.x) {
		for (ipos.y = minpos.y; ipos.y <= maxpos.y; ++ipos.y) {
			const CMapField &mf = *Map.Field(ipos);
			if (SquareDistance(ipos, goalPos) > square(this->Range)) {
				// Not in circle range
				continue;
			} else if (mf.isAWall() || mf.RockOnMap() || mf.ForestOnMap()) {
				Map.ClearTile(ipos);
			}
		}
	}

	//
	//  Effect of the explosion on units. Don't bother if damage is 0
	//
	if (this->Damage) {
		std::vector<CUnit *> table = SelectFixed(minpos, maxpos);
		for (CUnit *unit : table) {
			if (unit->Type->UnitType != EMovement::Fly && unit->IsAlive()
			    && unit->MapDistanceTo(goalPos) <= this->Range) {
				// Don't hit flying units!
				HitUnit(&caster, *unit, this->Damage);
			}
		}
	}

	return 1;
}



//@}
