//       _________ __                 __
//      /   _____//  |_____________ _/  |______     ____  __ __  ______
//      \_____  \\   __\_  __ \__  \\   __\__  \   / ___\|  |  \/  ___/
//      /        \|  |  |  | \// __ \|  |  / __ \_/ /_/  >  |  /\___ |
//     /_______  /|__|  |__|  (____  /__| (____  /\___  /|____//____  >
//             \/                  \/          \//_____/            \/
//  ______________________                           ______________________
//                        T H E   W A R   B E G I N S
//           Stratagus - A free fantasy real time strategy game engine
//
/**@name action_returngoods.c - The return goods action. */
//
//      (c) Copyright 1998,2000-2004 by Lutz Sammer
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
--  Include
----------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>

#include "stratagus.h"
#include "video.h"
#include "sound_id.h"
#include "unitsound.h"
#include "unittype.h"
#include "player.h"
#include "unit.h"
#include "actions.h"
#include "pathfinder.h"

/*----------------------------------------------------------------------------
--  Functions
----------------------------------------------------------------------------*/

/**
**  Return goods to gold/wood deposit.
**
**  @param unit  pointer to unit.
**
**  @todo  FIXME: move this into action_resource?
*/
void HandleActionReturnGoods(Unit* unit)
{
	const UnitType* type;
	Unit* destu;

	type = unit->Type;
	//
	// Select target to return goods.
	//
	Assert(type->Harvester );
	if (!unit->CurrentResource ||
			(unit->ResourcesHeld != unit->Type->ResInfo[unit->CurrentResource]->ResourceCapacity &&
				unit->Type->ResInfo[unit->CurrentResource]->LoseResources)) {
		DebugPrint("Unit can't return resources, it doesn't carry any.\n");
		NotifyPlayer(unit->Player, NotifyYellow, unit->X, unit->Y, "No Resources to Return.");
		unit->Orders[0].Action = UnitActionStill;
		return;
	}
	if (!unit->Orders[0].Goal) {
		if (!(destu = FindDeposit(unit, unit->X, unit->Y, 1000,
				unit->CurrentResource))) {
			unit->Orders[0].Action = UnitActionStill;
			return;
		}
		unit->Orders[0].Goal = destu;
		RefsIncrease(destu);
	}

	unit->Orders[0].Action = UnitActionResource;
	// Somewhere on the way the loaded worker could have change Arg1
	// Bummer, go get the closest resource to the depot
	unit->Orders[0].Arg1 = (void*)-1;
	NewResetPath(unit);
	unit->SubAction = 70;
	unit->Wait = 1;
	return;
}

//@}
