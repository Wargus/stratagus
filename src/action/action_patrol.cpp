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
/**@name action_patrol.cpp - The patrol action. */
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
#include "unit.h"
#include "unittype.h"
#include "actions.h"
#include "pathfinder.h"
#include "map.h"

/*----------------------------------------------------------------------------
--  Functions
----------------------------------------------------------------------------*/

extern bool AutoRepair(CUnit &unit);
extern bool AutoCast(CUnit &unit);

/**
**  Swap the patrol points.
*/
static void SwapPatrolPoints(CUnit &unit)
{
	COrderPtr order = unit.CurrentOrder();

	std::swap(order->Arg1.Patrol.x, order->goalPos.x);
	std::swap(order->Arg1.Patrol.y, order->goalPos.y);

	unit.CurrentOrder()->Data.Move.Cycles = 0; //moving counter
	unit.CurrentOrder()->NewResetPath();
}

/**
**  Unit Patrol:
**    The unit patrols between two points.
**    Any enemy unit in reaction range is attacked.
**  @todo FIXME:
**    Should do some tries to reach the end-points.
**    Should support patrol between more points!
**    Patrol between units.
**
**  @param unit  Patroling unit pointer.
*/
void HandleActionPatrol(COrder& order, CUnit &unit)
{
	if (unit.Wait) {
		unit.Wait--;
		return;
	}

	if (!unit.SubAction) { // first entry.
		order.Data.Move.Cycles = 0; //moving counter
		order.NewResetPath();
		unit.SubAction = 1;
	}

	switch (DoActionMove(unit)) {
		case PF_FAILED:
			unit.SubAction = 1;
			break;
		case PF_UNREACHABLE:
			// Increase range and try again
			unit.SubAction = 1;
			if (order.CheckRange()) {
				order.Range++;
				break;
			}
			// FALL THROUGH
		case PF_REACHED:
			unit.SubAction = 1;
			order.Range = 0;
			SwapPatrolPoints(unit);
			break;
		case PF_WAIT:
			// Wait for a while then give up
			unit.SubAction++;
			if (unit.SubAction == 5) {
				unit.SubAction = 1;
				order.Range = 0;
				SwapPatrolPoints(unit);
			}
			break;
		default: // moving
			unit.SubAction = 1;
			break;
	}

	if (!unit.Anim.Unbreakable) {
		//
		// Attack any enemy in reaction range.
		//  If don't set the goal, the unit can then choose a
		//  better goal if moving nearer to enemy.
		//
		if (unit.Type->CanAttack) {
			const CUnit *goal = AttackUnitsInReactRange(unit);
			if (goal) {
				// Save current command to come back.
				COrder *savedOrder = order.Clone();

				DebugPrint("Patrol attack %d\n" _C_ UnitNumber(*goal));
				CommandAttack(unit, goal->tilePos, NULL, FlushCommands);

				if (unit.StoreOrder(savedOrder) == false) {
					delete savedOrder;
					savedOrder = NULL;
				}
				unit.ClearAction();
				return;
			}
		}

		// Look for something to auto repair or auto cast
		if (AutoRepair(unit) || AutoCast(unit)) {
			return;
		}
	}
}

//@}
