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
/**@name spell_teleport.cpp - The spell Teleport. */
//
//      (c) Copyright 2013 by cybermind
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

#include "spell/spell_teleport.h"

#include "game.h"
#include "map.h"
#include "script.h"
#include "unit.h"

/* virtual */ void Spell_Teleport::Parse(lua_State *l, int startIndex, int endIndex)
{
	for (int j = startIndex; j < endIndex; ++j) {
		const char *value = LuaToString(l, -1, j + 1);
		++j;
		LuaError(l, "Unsupported Teleport tag: %s" _C_ value);
	}
}

/**
**  Cast teleport.
**
**  @param caster       Unit that casts the spell
**  @param spell        Spell-type pointer
**  @param goalPos      coord of target spot to cast
**
**  @return             =!0 if spell should be repeated, 0 if not
*/
/* virtual */ int Spell_Teleport::Cast(CUnit &caster, const SpellType &spell, CUnit * /*target*/, const Vec2i &goalPos)
{
	if (Map.Info.IsPointOnMap(goalPos)) {
		unsigned int selected = caster.Selected;
		caster.Remove(NULL);
		caster.tilePos = goalPos;
		DropOutNearest(caster, goalPos, NULL);
		if (selected) {
			SelectUnit(caster);
		}
	}
	return 0;
}

//@}
