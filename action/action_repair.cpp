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
/**@name action_repair.c - The repair action. */
//
//      (c) Copyright 1999-2005 by Vladi Shabanski and Jimmy Salmon
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
#include "animation.h"
#include "player.h"
#include "unit.h"
#include "missile.h"
#include "actions.h"
#include "sound.h"
#include "tileset.h"
#include "map.h"
#include "pathfinder.h"
#include "interface.h"

/*----------------------------------------------------------------------------
--  Functions
----------------------------------------------------------------------------*/

/**
**  Repair a unit.
**
**  @param unit  unit repairing
**  @param goal  unit being repaired
*/
static void RepairUnit(Unit* unit, Unit* goal)
{
	Player* player;
	int i;
	int animlength;
	int hp;
	char buf[100];

	player = unit->Player;

	if (goal->Orders[0].Action != UnitActionBuilt) {
		//
		// Calculate the repair costs.
		//
		Assert(goal->Stats->Variables[HP_INDEX].Max);

		//
		// Check if enough resources are available
		//
		for (i = 1; i < MaxCosts; ++i) {
			if (player->Resources[i] < goal->Type->RepairCosts[i]) {
				snprintf(buf, 100, "We need more %s for repair!",
					DefaultResourceNames[i]);
				NotifyPlayer(player, NotifyYellow, unit->X, unit->Y, buf);
				if (player->AiEnabled) {
					// FIXME: call back to AI?
					RefsDecrease(goal);
					unit->Orders[0].Goal = NULL;
					unit->Orders[0].Action = UnitActionStill;
					unit->State = unit->SubAction = 0;
					if (unit->Selected) { // update display for new action
						SelectedUnitChanged();
					}
				}
				// FIXME: We shouldn't animate if no resources are available.
				return;
			}
		}
		//
		// Subtract the resources
		//
		PlayerSubCosts(player, goal->Type->RepairCosts);

		goal->Variable[HP_INDEX].Value += goal->Type->RepairHP;
		if (goal->Variable[HP_INDEX].Value > goal->Variable[HP_INDEX].Max) {
			goal->Variable[HP_INDEX].Value = goal->Variable[HP_INDEX].Max;
		}
	} else {
		// hp is the current damage taken by the unit.
		hp = (goal->Data.Built.Progress * goal->Variable[HP_INDEX].Max) /
			(goal->Stats->Costs[TimeCost] * 600) - goal->Variable[HP_INDEX].Value;
		//
		// Calculate the length of the attack (repair) anim.
		//
		animlength = unit->Data.Repair.Cycles;
		unit->Data.Repair.Cycles = 0;

		// FIXME: implement this below:
		//unit->Data.Built.Worker->Type->BuilderSpeedFactor;
		goal->Data.Built.Progress += 100 * animlength * SpeedBuild;
		// Keep the same level of damage while increasing HP.
		goal->Variable[HP_INDEX].Value = (goal->Data.Built.Progress * goal->Stats->Variables[HP_INDEX].Max) /
			(goal->Stats->Costs[TimeCost] * 600) - hp;
		if (goal->Variable[HP_INDEX].Value > goal->Variable[HP_INDEX].Max) {
			goal->Variable[HP_INDEX].Value = goal->Variable[HP_INDEX].Max;
		}
	}
}

/**
**  Animate unit repair
**
**  @param unit  Unit, for that the repair animation is played.
*/
static int AnimateActionRepair(Unit* unit)
{
	UnitShowAnimation(unit, unit->Type->Animations->Repair);
	return 0;
}

/**
**  Unit repairs
**
**  @param unit  Unit, for that the attack is handled.
*/
void HandleActionRepair(Unit* unit)
{
	Unit* goal;
	int err;

	switch (unit->SubAction) {
		case 0:
			NewResetPath(unit);
			unit->SubAction = 1;
			// FALL THROUGH
		//
		// Move near to target.
		//
		case 1:
			// FIXME: RESET FIRST!! Why? We move first and than check if
			// something is in sight.
			err = DoActionMove(unit);
			if (!unit->Anim.Unbreakable) {
				//
				// No goal: if meeting damaged building repair it.
				//
				goal = unit->Orders[0].Goal;

				//
				// Target is dead, choose new one.
				//
				// Check if goal is correct unit.
				if (goal) {
					if (!UnitVisibleAsGoal(goal, unit->Player)) {
						DebugPrint("repair target gone.\n");
						unit->Orders[0].X = goal->X;
						unit->Orders[0].Y = goal->Y;
						RefsDecrease(goal);
						// FIXME: should I clear this here?
						unit->Orders[0].Goal = goal = NULL;
						NewResetPath(unit);
					}
				} else if (unit->Player->AiEnabled) {
					// Ai players workers should stop if target is killed
					err = -1;
				}

				//
				// Have reached target? FIXME: could use return value
				//
				if (goal && MapDistanceBetweenUnits(unit, goal) <= unit->Type->RepairRange &&
						goal->Variable[HP_INDEX].Value < goal->Variable[HP_INDEX].Max) {
					unit->State = 0;
					unit->SubAction = 2;
					unit->Data.Repair.Cycles = 0;
					UnitHeadingFromDeltaXY(unit,
						goal->X + (goal->Type->TileWidth - 1) / 2 - unit->X,
						goal->Y + (goal->Type->TileHeight - 1) / 2 - unit->Y);
				} else if (err < 0) {
					if (goal) { // release reference
						RefsDecrease(goal);
						unit->Orders[0].Goal = NoUnitP;
					}
					unit->Orders[0].Action = UnitActionStill;
					unit->State = unit->SubAction = 0;
					if (unit->Selected) { // update display for new action
						SelectedUnitChanged();
					}
					return;
				}

				// FIXME: Should be it already?
				Assert(unit->Orders[0].Action == UnitActionRepair);
			}
			break;

		//
		// Repair the target.
		//
		case 2:
			AnimateActionRepair(unit);
			unit->Data.Repair.Cycles++;
			if (!unit->Anim.Unbreakable) {
				goal = unit->Orders[0].Goal;

				//
				// Target is dead, choose new one.
				//
				// Check if goal is correct unit.
				// FIXME: should I do a function for this?
				if (goal) {
					if (!UnitVisibleAsGoal(goal, unit->Player)) {
						DebugPrint("repair goal is gone\n");
						unit->Orders[0].X = goal->X;
						unit->Orders[0].Y = goal->Y;
						RefsDecrease(goal);
						// FIXME: should I clear this here?
						unit->Orders[0].Goal = goal = NULL;
						NewResetPath(unit);
					}
				}
				if (goal && MapDistanceBetweenUnits(unit, goal) <= unit->Type->RepairRange) {
					RepairUnit(unit, goal);
					goal = unit->Orders[0].Goal;
				} else if (goal && MapDistanceBetweenUnits(unit, goal) > unit->Type->RepairRange) {
					// If goal has move, chase after it
					unit->State = 0;
					unit->SubAction = 0;
				}


				//
				// Target is fine, choose new one.
				//
				if (!goal || goal->Variable[HP_INDEX].Value >= goal->Variable[HP_INDEX].Max) {
					if (goal) { // release reference
						RefsDecrease(goal);
						unit->Orders[0].Goal = NULL;
					}
					unit->Orders[0].Action = UnitActionStill;
					unit->SubAction = unit->State = 0;
					if (unit->Selected) { // update display for new action
						SelectedUnitChanged();
					}
					return;
				}

				// FIXME: automatic repair
			}
			break;
	}
}

//@}
