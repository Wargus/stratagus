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
/**@name action_patrol.c - The patrol action. */
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
--  Includes
----------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>

#include "stratagus.h"
#include "unit.h"
#include "actions.h"
#include "pathfinder.h"

/*----------------------------------------------------------------------------
--  Functions
----------------------------------------------------------------------------*/

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
void HandleActionPatrol(Unit* unit)
{
	const Unit* goal;

	if (!unit->SubAction) { // first entry.
		NewResetPath(unit);
		unit->SubAction = 1;
	}

	if (DoActionMove(unit) < 0) { // reached end-point or stop
		int tmp;

		Assert(unit->Orders[0].Action == UnitActionPatrol);

		//
		// Swap the points.
		//
		tmp = (int)unit->Orders[0].Arg1;
		unit->Orders[0].Arg1 = (void*)((unit->Orders[0].X << 16) | unit->Orders[0].Y);
		unit->Orders[0].X = tmp >> 16;
		unit->Orders[0].Y = tmp & 0xFFFF;

		NewResetPath(unit);
	}

	if (unit->Reset) {
		//
		// Attack any enemy in reaction range.
		//  If don't set the goal, the unit can than choose a
		//  better goal if moving nearer to enemy.
		//
		if (unit->Type->CanAttack && unit->Stats->Speed) {
			goal = AttackUnitsInReactRange(unit);
			if (goal) {
				DebugPrint("Patrol attack %d\n" _C_ UnitNumber(goal));
				CommandAttack(unit, goal->X, goal->Y, NULL, FlushCommands);
				// Save current command to come back.
				unit->SavedOrder = unit->Orders[0];
				unit->Orders[0].Action = UnitActionStill;
				unit->Orders[0].Goal = NoUnitP;
				unit->SubAction = 0;
				DebugPrint("Wait %d\n" _C_ unit->Wait);
			}
		}
	}
}

//@}
