//     ____                _       __               
//    / __ )____  _____   | |     / /___ ___________
//   / __  / __ \/ ___/   | | /| / / __ `/ ___/ ___/
//  / /_/ / /_/ (__  )    | |/ |/ / /_/ / /  (__  ) 
// /_____/\____/____/     |__/|__/\__,_/_/  /____/  
//                                              
//       A futuristic real-time strategy game.
//          This file is part of Bos Wars.
//
/**@name action_follow.cpp - The follow action. */
//
//      (c) Copyright 2001-2007 by Lutz Sammer and Jimmy Salmon
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
#include <string.h>

#include "stratagus.h"
#include "unit.h"
#include "unittype.h"
#include "pathfinder.h"
#include "map.h"
#include "interface.h"
#include "actions.h"

/*----------------------------------------------------------------------------
--  Variables
----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
--  Function
----------------------------------------------------------------------------*/

/**
**  Unit follow action:
**
**  @param unit  Pointer to unit.
*/
void HandleActionFollow(CUnit *unit)
{
	CUnit *goal;

	if (unit->Wait) {
		unit->Wait--;
		return;
	}

	//
	// Reached target
	//
	if (unit->SubAction == 128) {
		goal = unit->Orders[0]->Goal;
		if (!goal || !goal->IsVisibleAsGoal(unit->Player)) {
			DebugPrint("Goal gone\n");
			if (goal) {
				goal->RefsDecrease();
			}
			unit->Orders[0]->Goal = NoUnitP;
			unit->ClearAction();
			return;
		}

		if (goal->X == unit->Orders[0]->X && goal->Y == unit->Orders[0]->Y) {
			// Move to the next order
			if (unit->OrderCount > 1) {
				goal->RefsDecrease();
				unit->Orders[0]->Goal = NoUnitP;
				unit->ClearAction();
				return;
			}

			// Reset frame to still frame while we wait
			// FIXME: Unit doesn't animate.
			unit->Frame = unit->Type->StillFrame;
			UnitUpdateHeading(unit);
			unit->Wait = 10;
			if (unit->Orders[0]->Range > 1) {
				unit->Orders[0]->Range = 1;
				unit->SubAction = 0;
			}
			return;
		}

		unit->SubAction = 0;
	}

	if (!unit->SubAction) { // first entry
		unit->SubAction = 1;
		NewResetPath(unit);
		Assert(unit->State == 0);
	}

	switch (DoActionMove(unit)) { // reached end-point?
		case PF_UNREACHABLE:
			//
			// Some tries to reach the goal
			//
			if (unit->Orders[0]->Range <= Map.Info.MapWidth ||
					unit->Orders[0]->Range <= Map.Info.MapHeight) {
				unit->Orders[0]->Range++;
				break;
			}
			// FALL THROUGH
		case PF_REACHED:
			if (!(goal = unit->Orders[0]->Goal)) { // goal has died
				unit->ClearAction();
				return;
			}
			unit->Orders[0]->X = goal->X;
			unit->Orders[0]->Y = goal->Y;
			unit->SubAction = 128;

			// FALL THROUGH
		default:
			break;
	}

	//
	// Target destroyed?
	//
	if ((goal = unit->Orders[0]->Goal) && !goal->IsVisibleAsGoal(unit->Player)) {
		DebugPrint("Goal gone\n");
		unit->Orders[0]->X = goal->X + goal->Type->TileWidth / 2;
		unit->Orders[0]->Y = goal->Y + goal->Type->TileHeight / 2;
		unit->Orders[0]->Goal = NoUnitP;
		goal->RefsDecrease();
		goal = NoUnitP;
		NewResetPath(unit);
	}

	if (!unit->Anim.Unbreakable) {
		//
		// If our leader is dead or stops or attacks:
		// Attack any enemy in reaction range.
		// If don't set the goal, the unit can than choose a
		//  better goal if moving nearer to enemy.
		//
		if (unit->Type->CanAttack &&
				(!goal || goal->Orders[0]->Action == UnitActionAttack ||
					goal->Orders[0]->Action == UnitActionStill)) {
			goal = AttackUnitsInReactRange(unit);
			if (goal) {
				CommandAttack(unit, goal->X, goal->Y, NULL, FlushCommands);
				// Save current command to come back.
				unit->SavedOrder = *unit->Orders[0];
				// This stops the follow command and the attack is executed
				unit->ClearAction();
				unit->Orders[0]->Goal = NoUnitP;
			}
		}
	}
}

//@}
