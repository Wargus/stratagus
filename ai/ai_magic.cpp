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
/**@name ai_magic.c - AI magic functions. */
//
//      (c) Copyright 2002-2004 by Lutz Sammer, Joris Dauphin
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
//      $Id$

//@{

/*----------------------------------------------------------------------------
--  Includes
----------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>

#include "stratagus.h"
#include "unittype.h"
#include "unit.h"
#include "spells.h"
#include "actions.h"
#include "map.h"
#include "ai_local.h"

/*----------------------------------------------------------------------------
--  Functions
----------------------------------------------------------------------------*/

/**
**  Check what computer units can do with magic.
**  In fact, turn on autocast for AI.
*/
global void AiCheckMagic(void)
{
	int i;
	int j;
	int n;
	Unit** units;
	Unit* unit;
	const Player *player;
#ifdef DEBUG
	int success;
#endif

	n = AiPlayer->Player->TotalNumUnits;
	units = AiPlayer->Player->Units;
	player = AiPlayer->Player; /*units[0]->Player */
	for (i = 0; i < n; ++i) {
		unit = units[i];
		// Check only magic units
		if (unit->Type->CanCastSpell) {
			for (j = 0; j < SpellTypeCount; ++j) {
				// Check if we can cast this spell. SpellIsAvailable checks for upgrades.
				if (unit->Type->CanCastSpell[j] && SpellIsAvailable(player, j) &&
					(SpellTypeTable[j]->AutoCast || SpellTypeTable[j]->AICast)) {
#ifdef DEBUG
					success =  // Follow on next line (AutoCastSpell).
#endif
						AutoCastSpell(unit, SpellTypeTable[j]);
				}
			}
		}
	}
}

//@}
