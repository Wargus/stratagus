//     ____                _       __               
//    / __ )____  _____   | |     / /___ ___________
//   / __  / __ \/ ___/   | | /| / / __ `/ ___/ ___/
//  / /_/ / /_/ (__  )    | |/ |/ / /_/ / /  (__  ) 
// /_____/\____/____/     |__/|__/\__,_/_/  /____/  
//                                              
//       A futuristic real-time strategy game.
//          This file is part of Bos Wars.
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
void HandleActionDie(CUnit *unit)
{
	//
	// Show death animation
	//
	if (unit->Type->Animations && unit->Type->Animations->Death) {
		UnitShowAnimation(unit, unit->Type->Animations->Death);
	} else {
		// some units has no death animation
		unit->Anim.Unbreakable = 0;
	}

	//
	// Die sequence terminated, generate corpse.
	//
	if (!unit->Anim.Unbreakable) {
		if (!unit->Type->CorpseType) {
			// We may be in the cache if we just finished out death animation
			// even though there is no corpse.
			// (unit->Type->Animations && unit->Type->Animations->Death)
			// Remove us from the map to be safe
			unit->Remove(NULL);
			unit->Release();
			return;
		}

		Assert(unit->Type->TileWidth == unit->Type->CorpseType->TileWidth &&
			unit->Type->TileHeight == unit->Type->CorpseType->TileHeight);

		// Update sight for new corpse
		// We have to unmark BEFORE changing the type.
		// Always do that, since types can have different vision properties.
		MapUnmarkUnitSight(unit);
		unit->Type = unit->Type->CorpseType;
		unit->CurrentSightRange = unit->Type->Stats[unit->Player->Index].Variables[SIGHTRANGE_INDEX].Max;
		MapMarkUnitSight(unit);

		// We must be dead to get here, it we aren't we need to know why
		// This assert replaces and old DEBUG message "Reset to die is really needed"
		Assert(unit->Orders[0]->Action == UnitActionDie);

		unit->SubAction = 0;
		unit->Frame = 0;
		UnitUpdateHeading(unit);
		if (unit->Type->Animations && unit->Type->Animations->Death) {
			UnitShowAnimation(unit, unit->Type->Animations->Death);
		}
	}
}

//@}
