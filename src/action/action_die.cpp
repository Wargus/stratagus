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
/**@name action_die.cpp - The die action. */
//
//      (c) Copyright 1998-2005 by Lutz Sammer and Jimmy Salmon
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
#include "animation.h"
#include "player.h"
#include "unit.h"
#include "actions.h"
#include "map.h"

/*----------------------------------------------------------------------------
--  Functions
----------------------------------------------------------------------------*/

/**
**  Unit dies!
**
**  @param unit  The unit which dies.
*/
void HandleActionDie(COrder& order, CUnit &unit)
{
	Assert(order.Action == UnitActionDie);

	// Show death animation
	if (unit.Type->Animations && unit.Type->Animations->Death[unit.DamagedType]) {
		UnitShowAnimation(unit, unit.Type->Animations->Death[unit.DamagedType]);
	}
	else if (unit.Type->Animations && unit.Type->Animations->Death[ANIMATIONS_DEATHTYPES]) {
		UnitShowAnimation(unit, unit.Type->Animations->Death[ANIMATIONS_DEATHTYPES]);
	} else {
		// some units has no death animation
		unit.Anim.Unbreakable = 0;
	}

	if (unit.Anim.Unbreakable) {
		return;
	}
	// Die sequence terminated, generate corpse.
	if (!unit.Type->CorpseType) {
		// We may be in the cache if we just finished out death animation
		// even though there is no corpse.
		// (unit.Type->Animations && unit.Type->Animations->Death)
		// Remove us from the map to be safe
		unit.Remove(NULL);
		unit.Release();
		return;
	}

	Assert(unit.Type->TileWidth >= unit.Type->CorpseType->TileWidth &&
		unit.Type->TileHeight >= unit.Type->CorpseType->TileHeight);

	// Update sight for new corpse
	// We have to unmark BEFORE changing the type.
	// Always do that, since types can have different vision properties.

	unit.Remove(NULL);
	unit.Type = unit.Type->CorpseType;
	unit.Stats = &unit.Type->Stats[unit.Player->Index];
	unit.Place(unit.tilePos);

	unit.SubAction = 0;
	unit.Frame = 0;
	UnitUpdateHeading(unit);
	if (unit.Type->Animations && unit.Type->Animations->Death[ANIMATIONS_DEATHTYPES]) {
		UnitShowAnimation(unit, unit.Type->Animations->Death[ANIMATIONS_DEATHTYPES]);
	}
}

//@}
