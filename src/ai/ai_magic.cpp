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
/**@name ai_magic.cpp - AI magic functions. */
//
//      (c) Copyright 2002-2005 by Lutz Sammer, Joris Dauphin
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
#include "unittype.h"
#include "unit.h"
#include "spells.h"
#include "actions.h"
#include "ai_local.h"

/*----------------------------------------------------------------------------
--  Functions
----------------------------------------------------------------------------*/

/**
**  Check what computer units can do with magic.
**  In fact, turn on autocast for AI.
*/
void AiCheckMagic()
{
	CPlayer &player = *AiPlayer->Player;
	const int n = player.GetUnitCount();

	for (int i = 0; i < n; ++i) {
		CUnit &unit = player.GetUnit(i);

		if (unit.Type->CanCastSpell) {
			// Check only idle magic units
			for (size_t i = 0; i != unit.Orders.size(); ++i) {
				if (unit.Orders[i]->Action == UnitActionSpellCast) {
					return;
				}
			}
			for (unsigned int j = 0; j < SpellTypeTable.size(); ++j) {
				// Check if we can cast this spell. SpellIsAvailable checks for upgrades.
				if (unit.Type->CanCastSpell[j] && SpellIsAvailable(player, j)
					&& (SpellTypeTable[j]->AutoCast || SpellTypeTable[j]->AICast)) {
					AutoCastSpell(unit, *SpellTypeTable[j]);
				}
			}
		}
	}
}

//@}
