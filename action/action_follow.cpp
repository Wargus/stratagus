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
/**@name action_follow.c - The follow action. */
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
void HandleActionFollow(Unit* unit)
{
	Unit* goal;

	//
	// Reached target
	//
	if (unit->SubAction == 128) {
		goal = unit->Orders[0].Goal;
		if (!goal || !UnitVisibleAsGoal(goal, unit->Player)) {
			DebugPrint("Goal gone\n");
			if (goal) {
				RefsDecrease(goal);
			}
			unit->Orders[0].Goal = NoUnitP;
			unit->SubAction = 0;
			unit->Orders[0].Action = UnitActionStill;
			if (IsOnlySelected(unit)) { // update display for new action
				SelectedUnitChanged();
			}
			return;
		}

		// Two posibilities, both broken. maybe we should change the animation system?
		// FIXME: Unit doesn't decrease range
#if 0
		if ((goal->X == unit->Orders[0].X && goal->Y == unit->Orders[0].Y) || unit->State) {
			UnitShowAnimation(unit, unit->Type->Animations->Still);
			//
			// Sea and air units are floating up/down.
			//
			if (unit->Type->SeaUnit || unit->Type->AirUnit) {
				unit->IY = (MyRand() >> 15) & 1;
			}
			return;
		}
#else
		// FIXME: Unit doesn't animate.
		if ((goal->X == unit->Orders[0].X && goal->Y == unit->Orders[0].Y)) {
			unit->Wait = 10;
			if (unit->Orders[0].Range > 1) {
				unit->Orders[0].Range = 1;
				unit->SubAction = 0;
			}
			return;
		}
#endif
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
			if (unit->Orders[0].Range <= TheMap.Info.MapWidth ||
					unit->Orders[0].Range <= TheMap.Info.MapHeight) {
				unit->Orders[0].Range++;
				break;
			}
			// FALL THROUGH
		case PF_REACHED:
			// Handle Teleporter Units
			// FIXME: BAD HACK
			if ((goal = unit->Orders[0].Goal) &&
					goal->Type->Teleporter && goal->Goal &&
					MapDistanceBetweenUnits(unit, goal) <= 1) {
				Unit* dest;

				// Teleport the unit
				RemoveUnit(unit, NULL);
				unit->X = goal->Goal->X;
				unit->Y = goal->Goal->Y;
				DropOutOnSide(unit, unit->Direction, 1, 1);
#if 0
				// FIXME: SoundIdForName() should be called once
				PlayGameSound(SoundIdForName("invisibility"), MaxSampleVolume);
				// FIXME: MissileTypeByIdent() should be called once
				MakeMissile(MissileTypeByIdent("missile-normal-spell"),
					unit->X * TileSizeX + TileSizeX / 2,
					unit->Y * TileSizeY + TileSizeY / 2,
					unit->X * TileSizeX + TileSizeX / 2,
					unit->Y * TileSizeY + TileSizeY / 2);
#endif
				unit->SubAction = 0;
				unit->Orders[0].Action = UnitActionStill;

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
						unit->Orders->Action = UnitActionStill;
						unit->Orders->Goal = NoUnitP;
					} else {
						if (dest->NewOrder.Goal) {
							if (dest->NewOrder.Goal->Destroyed) {
								// FIXME: perhaps we should use another dest?
								DebugPrint("Destroyed unit in teleport unit\n");
								RefsDecrease(dest);
								dest->NewOrder.Goal = NoUnitP;
								dest->NewOrder.Action = UnitActionStill;
							}
						}

						unit->Orders[0] = dest->NewOrder;

						//
						// FIXME: Pending command uses any references?
						//
						if (unit->Orders[0].Goal) {
							RefsIncrease(unit->Orders->Goal);
						}
					}
				}
				return;
			}

			if (!(goal = unit->Orders[0].Goal)) { // goal has died
				unit->SubAction = 0;
				unit->Orders[0].Action = UnitActionStill;
				if (IsOnlySelected(unit)) { // update display for new action
					SelectedUnitChanged();
				}
				return;
			}
			unit->Orders[0].X = goal->X;
			unit->Orders[0].Y = goal->Y;
			unit->SubAction = 128;

			// FALL THROUGH
		default:
			break;
	}

	//
	// Target destroyed?
	//
	if ((goal = unit->Orders[0].Goal) && !UnitVisibleAsGoal(goal, unit->Player)) {
		DebugPrint("Goal gone\n");
		unit->Orders[0].X = goal->X + goal->Type->TileWidth / 2;
		unit->Orders[0].Y = goal->Y + goal->Type->TileHeight / 2;
		unit->Orders[0].Goal = NoUnitP;
		RefsDecrease(goal);
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
				(!goal || goal->Orders[0].Action == UnitActionAttack ||
					goal->Orders[0].Action == UnitActionStill)) {
			goal = AttackUnitsInReactRange(unit);
			if (goal) {
				CommandAttack(unit, goal->X, goal->Y, NULL, FlushCommands);
				// Save current command to come back.
				unit->SavedOrder = unit->Orders[0];
				// This stops the follow command and the attack is executed
				unit->Orders[0].Action = UnitActionStill;
				unit->Orders[0].Goal = NoUnitP;
				unit->SubAction = 0;
			}
		}
	}
}

//@}
