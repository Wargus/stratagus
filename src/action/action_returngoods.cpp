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
/**@name action_returngoods.cpp - The return goods action. */
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
--  Include
----------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "stratagus.h"
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
void HandleActionReturnGoods(COrder& order, CUnit &unit)
{
	Assert(unit.Type->Harvester);

	// Select target to return goods.
	if (!unit.CurrentResource || unit.ResourcesHeld == 0 ||
			(unit.ResourcesHeld != unit.Type->ResInfo[unit.CurrentResource]->ResourceCapacity &&
				unit.Type->ResInfo[unit.CurrentResource]->LoseResources)) {
		DebugPrint("Unit can't return resources, it doesn't carry any.\n");
		unit.Player->Notify(NotifyYellow, unit.tilePos.x, unit.tilePos.y, _("No Resources to Return."));

		ResourceGiveUp(unit);
		return;
	}

	// If depot was destroyed search for another one.
	if (!order.HasGoal()) {
		CUnit *destu;

		if (!(destu = FindDeposit(unit, 1000, unit.CurrentResource))) {
			ResourceGiveUp(unit);
			return;
		}
		order.SetGoal(destu);
	}

	order.Action = UnitActionResource;
	// Somewhere on the way the loaded worker could have change Arg1
	// Bummer, go get the closest resource to the depot
	//FIXME!!!!!!!!!!!!!!!!!!!!
	//unit.CurrentOrder()->Arg1.ResourcePos = -1;

	order.NewResetPath();
	unit.SubAction = /* SUB_MOVE_TO_DEPOT */ 70; // FIXME : Define value.
}

//@}
