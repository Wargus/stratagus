//     ____                _       __               
//    / __ )____  _____   | |     / /___ ___________
//   / __  / __ \/ ___/   | | /| / / __ `/ ___/ ___/
//  / /_/ / /_/ (__  )    | |/ |/ / /_/ / /  (__  ) 
// /_____/\____/____/     |__/|__/\__,_/_/  /____/  
//                                              
//       A futuristic real-time strategy game.
//          This file is part of Bos Wars.
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

#include <stdio.h>
#include <stdlib.h>

#include "stratagus.h"
#include "unittype.h"
#include "unit.h"
#include "spells.h"
#include "actions.h"
#include "map.h"
#include "ai_local.h"
#include "player.h"

/*----------------------------------------------------------------------------
--  Functions
----------------------------------------------------------------------------*/

/**
**  Check what computer units can do with magic.
**  In fact, turn on autocast for AI.
*/
void AiCheckMagic(void)
{
	int i;
	unsigned int j;
	int n;
	CUnit **units;
	CUnit *unit;
	const CPlayer *player;
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
			for (j = 0; j < SpellTypeTable.size(); ++j) {
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
