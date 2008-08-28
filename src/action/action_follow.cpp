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
/**@name action_follow.cpp - The follow action. */
//
//      (c) Copyright 2001-2005 by Lutz Sammer and Jimmy Salmon
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
		COrderPtr order = unit->CurrentOrder();
		goal = order->GetGoal();

		if (!goal || !goal->IsVisibleAsGoal(unit->Player)) {
			DebugPrint("Goal gone\n");
			order->ClearGoal();
			unit->ClearAction();
			return;
		}

		if (goal->X == order->X && goal->Y == order->Y) {

			// Move to the next order
			if (unit->OrderCount > 1) {
				order->ClearGoal();
				unit->ClearAction();
				return;
			}

			// Reset frame to still frame while we wait
			// FIXME: Unit doesn't animate.
			unit->Frame = unit->Type->StillFrame;
			UnitUpdateHeading(unit);
			unit->Wait = 10;
			if (order->Range > 1) {
				order->Range = 1;
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
			if (unit->CurrentOrder()->CheckRange()) {
				unit->CurrentOrder()->Range++;
				break;
			}
			// FALL THROUGH
		case PF_REACHED:
			// Handle Teleporter Units
			// FIXME: BAD HACK
			if ((goal = unit->CurrentOrder()->GetGoal()) &&
					goal->Type->Teleporter && goal->Goal &&
					unit->MapDistanceTo(goal) <= 1) {
				CUnit *dest;

				// Teleport the unit
				unit->Remove(NULL);
				unit->X = goal->Goal->X;
				unit->Y = goal->Goal->Y;
				DropOutOnSide(unit, unit->Direction, 1, 1);
#if 0
				// FIXME: SoundForName() should be called once
				PlayGameSound(SoundForName("invisibility"), MaxSampleVolume);
				// FIXME: MissileTypeByIdent() should be called once
				MakeMissile(MissileTypeByIdent("missile-normal-spell"),
					unit->X * TileSizeX + TileSizeX / 2,
					unit->Y * TileSizeY + TileSizeY / 2,
					unit->X * TileSizeX + TileSizeX / 2,
					unit->Y * TileSizeY + TileSizeY / 2);
#endif
				unit->ClearAction();

				//
				// FIXME: we must check if the units supports the new order.
				//
				dest = goal->Goal;

				if (dest) {
					if ((dest->NewOrder.Action == UnitActionResource &&
								!unit->Type->Harvester) ||
							(dest->NewOrder.Action == UnitActionAttack &&
								!unit->Type->CanAttack) ||
							(dest->NewOrder.Action == UnitActionBoard &&
								unit->Type->UnitType != UnitTypeLand)) {
						DebugPrint("Wrong order for unit\n");
						unit->ClearAction();
						unit->CurrentOrder()->ClearGoal();
					} else {
						if (dest->NewOrder.HasGoal()) {
							if (dest->NewOrder.GetGoal()->Destroyed) {
								// FIXME: perhaps we should use another dest?
								DebugPrint("Destroyed unit in teleport unit\n");
								dest->RefsDecrease();///???????
								dest->NewOrder.ClearGoal();
								dest->NewOrder.Action = UnitActionStill;
							}
						}

						*(unit->CurrentOrder()) = dest->NewOrder;
						unit->CurrentResource = dest->CurrentResource;
						
					}
				}
				return;
			}

			goal = unit->CurrentOrder()->GetGoal();
			if (!goal) { // goal has died
				unit->ClearAction();
				return;
			}
			unit->CurrentOrder()->X = goal->X;
			unit->CurrentOrder()->Y = goal->Y;
			unit->SubAction = 128;

			// FALL THROUGH
		default:
			break;
	}

	//
	// Target destroyed?
	//
	goal = unit->CurrentOrder()->GetGoal();
	if (goal && !goal->IsVisibleAsGoal(unit->Player)) {
		DebugPrint("Goal gone\n");
		unit->CurrentOrder()->X = goal->X + goal->Type->TileWidth / 2;
		unit->CurrentOrder()->Y = goal->Y + goal->Type->TileHeight / 2;
		unit->CurrentOrder()->ClearGoal();
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
				(!goal || goal->CurrentAction() == UnitActionAttack ||
					goal->CurrentAction() == UnitActionStill)) {
			goal = AttackUnitsInReactRange(unit);
			if (goal) {
				CommandAttack(unit, goal->X, goal->Y, NULL, FlushCommands);
				// Save current command to come back.
				unit->SavedOrder = *(unit->CurrentOrder());
				// This stops the follow command and the attack is executed
				unit->ClearAction();
				unit->CurrentOrder()->ClearGoal();
			}
		}
	}
}

//@}
