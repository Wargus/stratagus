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
void HandleActionFollow(COrder& order, CUnit &unit)
{
	if (unit.Wait) {
		unit.Wait--;
		return;
	}
	CUnit *goal = order.GetGoal();

	// Reached target
	if (unit.SubAction == 128) {

		if (!goal || !goal->IsVisibleAsGoal(*unit.Player)) {
			DebugPrint("Goal gone\n");
			order.ClearGoal();
			unit.ClearAction();
			return;
		}

		if (goal->tilePos == order.goalPos) {
			// Move to the next order
			if (unit.Orders.size() > 1) {
				order.ClearGoal();
				unit.ClearAction();
				return;
			}

			// Reset frame to still frame while we wait
			// FIXME: Unit doesn't animate.
			unit.Frame = unit.Type->StillFrame;
			UnitUpdateHeading(unit);
			unit.Wait = 10;
			if (order.Range > 1) {
				order.Range = 1;
				unit.SubAction = 0;
			}
			return;
		}
		unit.SubAction = 0;
	}
	if (!unit.SubAction) { // first entry
		unit.SubAction = 1;
		order.NewResetPath();
		Assert(unit.State == 0);
	}
	switch (DoActionMove(unit)) { // reached end-point?
		case PF_UNREACHABLE:
			// Some tries to reach the goal
			if (order.CheckRange()) {
				order.Range++;
				break;
			}
			// FALL THROUGH
		case PF_REACHED:
		{
			if (!goal) { // goal has died
				unit.ClearAction();
				return;
			}
			// Handle Teleporter Units
			// FIXME: BAD HACK
			if (goal->Type->Teleporter && goal->Goal && unit.MapDistanceTo(*goal) <= 1) {
				// Teleport the unit
				unit.Remove(NULL);
				unit.tilePos = goal->Goal->tilePos;
				DropOutOnSide(unit, unit.Direction, NULL);
#if 0
				// FIXME: SoundForName() should be called once
				PlayGameSound(SoundForName("invisibility"), MaxSampleVolume);
				// FIXME: MissileTypeByIdent() should be called once
				MakeMissile(MissileTypeByIdent("missile-normal-spell"),
					unit.tilePos.x * PixelTileSize.x + PixelTileSize.x / 2,
					unit.tilePos.y * PixelTileSize.y + PixelTileSize.y / 2,
					unit.tilePos.x * PixelTileSize.x + PixelTileSize.x / 2,
					unit.tilePos.y * PixelTileSize.y + PixelTileSize.y / 2);
#endif
				unit.ClearAction();

				// FIXME: we must check if the units supports the new order.
				CUnit &dest = *goal->Goal;

				if (dest.NewOrder == NULL
					|| (dest.NewOrder->Action == UnitActionResource && !unit.Type->Harvester)
					|| (dest.NewOrder->Action == UnitActionAttack && !unit.Type->CanAttack)
					|| (dest.NewOrder->Action == UnitActionBoard && unit.Type->UnitType != UnitTypeLand)) {
					unit.ClearAction();
					unit.CurrentOrder()->ClearGoal();
				} else {
					if (dest.NewOrder->HasGoal()) {
						if (dest.NewOrder->GetGoal()->Destroyed) {
							// FIXME: perhaps we should use another dest?
							DebugPrint("Destroyed unit in teleport unit\n");
							dest.NewOrder->ClearGoal();
							dest.NewOrder->Action = UnitActionStill;
						}
					}

					delete unit.CurrentOrder();
					unit.Orders[0] = dest.NewOrder->Clone();
					unit.CurrentResource = dest.CurrentResource;
				}
				return;
			}
			order.goalPos = goal->tilePos;
			unit.SubAction = 128;
		}
			// FALL THROUGH
		default:
			break;
	}

	// Target destroyed?
	if (goal && !goal->IsVisibleAsGoal(*unit.Player)) {
		DebugPrint("Goal gone\n");
		order.goalPos = goal->tilePos + goal->Type->GetHalfTileSize();
		order.ClearGoal();
		goal = NoUnitP;
		order.NewResetPath();
	}

	if (!unit.Anim.Unbreakable) {
		// If our leader is dead or stops or attacks:
		// Attack any enemy in reaction range.
		// If don't set the goal, the unit can than choose a
		//  better goal if moving nearer to enemy.
		if (unit.Type->CanAttack
			&& (!goal || goal->CurrentAction() == UnitActionAttack || goal->CurrentAction() == UnitActionStill)) {
			goal = AttackUnitsInReactRange(unit);
			if (goal) {
				// Save current command to come back.
				COrder *savedOrder = order.Clone();

				CommandAttack(unit, goal->tilePos, NULL, FlushCommands);

				if (unit.StoreOrder(savedOrder) == false) {
					delete savedOrder;
					savedOrder = NULL;
				}
				// This stops the follow command and the attack is executed
				unit.CurrentOrder()->ClearGoal();
				unit.ClearAction();
			}
		}
	}
}

//@}
