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
/**@name action_die.c - The die action. */
//
//      (c) Copyright 1998-2004 by Lutz Sammer
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
void HandleActionDie(Unit* unit)
{
	//
	// Show death animation
	//
	if (unit->Type->Animations && unit->Type->Animations->Die) {
		UnitShowAnimation(unit, unit->Type->Animations->Die);
	} else {
		// some units has no death animation
		unit->Reset = unit->Wait = 1;
	}

	//
	// Die sequence terminated, generate corpse.
	//
	if (unit->Reset) {
		if (!unit->Type->CorpseType) {
			ReleaseUnit(unit);
			return;
		}

		unit->State = unit->Type->CorpseScript;

		Assert(unit->Type->TileWidth == unit->Type->CorpseType->TileWidth &&
				unit->Type->TileHeight == unit->Type->CorpseType->TileHeight);

		// Update sight for new corpse
		// We have to unmark BEFORE changing the type.
		// Always do that, since types can have different vision properties.
		MapUnmarkUnitSight(unit);
		unit->Type = unit->Type->CorpseType;
		unit->CurrentSightRange = unit->Type->Stats[unit->Player->Player].SightRange;
		MapMarkUnitSight(unit);

		CommandStopUnit(unit); // This clears all order queues
#ifdef DEBUG
		if (unit->Orders[0].Action != UnitActionDie) {
			DebugPrint("Reset to die is really needed\n");
		}
#endif
		unit->Orders[0].Action = UnitActionDie;
		--unit->OrderCount; // remove the stop command
		unit->SubAction = 0;
		unit->Frame = 0;
		UnitUpdateHeading(unit);
		UnitShowAnimation(unit, unit->Type->Animations->Die);

		// FIXME: perhaps later or never is better
#if 0
		ChangeUnitOwner(unit, &Players[PlayerNumNeutral]);
#endif
	}
}

//@}
