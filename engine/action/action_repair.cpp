//     ____                _       __               
//    / __ )____  _____   | |     / /___ ___________
//   / __  / __ \/ ___/   | | /| / / __ `/ ___/ ___/
//  / /_/ / /_/ (__  )    | |/ |/ / /_/ / /  (__  ) 
// /_____/\____/____/     |__/|__/\__,_/_/  /____/  
//                                              
//       A futuristic real-time strategy game.
//          This file is part of Bos Wars.
//
/**@name action_repair.cpp - The repair action. */
//
//      (c) Copyright 1999-2008 by Vladi Shabanski and Jimmy Salmon
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
#include "missile.h"
#include "actions.h"
#include "sound.h"
#include "map.h"
#include "pathfinder.h"
#include "interface.h"
#include "ai.h"

/*----------------------------------------------------------------------------
--  Functions
----------------------------------------------------------------------------*/

/**
**  Restore the saved order
**  FIXME: this should be moved to a more central location
**
**  @param unit  Unit to restore
**
**  @return      True if the saved order was restored
*/
static bool RestoreSavedOrder(CUnit *unit)
{
	if (unit->SavedOrder.Action != UnitActionStill) {
		unit->SubAction = 0;
		Assert(unit->Orders[0]->Goal == NoUnitP);
		*unit->Orders[0] = unit->SavedOrder;
		NewResetPath(unit);
		unit->SavedOrder.Action = UnitActionStill;

		// This isn't supported
		Assert(unit->SavedOrder.Goal == NoUnitP);
		return true;
	}
	return false;
}

/**
**  Move to build location
**
**  @param unit  Unit to move
*/
static void MoveToLocation(CUnit *unit)
{
	// First entry
	if (!unit->SubAction) {
		unit->SubAction = 1;
		NewResetPath(unit);
	}

	if (unit->Wait) {
		// FIXME: show still animation while we wait?
		unit->Wait--;
		return;
	}

	CUnit *goal = unit->Orders[0]->Goal;
	int err = 0;

	switch (DoActionMove(unit)) { // reached end-point?
		case PF_FAILED:
		case PF_UNREACHABLE:
			//
			// Some tries to reach the goal
			//
			if (unit->SubAction++ < 10) {
				// To keep the load low, retry each 10 cycles
				// NOTE: we can already inform the AI about this problem?
				unit->Wait = 10;
				return;
			}

			if (goal) { // release reference
				goal->RefsDecrease();
				unit->Orders[0]->Goal = NoUnitP;
			}
			unit->ClearAction();
			return;

		case PF_REACHED:
			break;

		default:
			// Moving...
			break;
	}

	if (unit->Anim.Unbreakable) {
		return;
	}

	//
	// Target is dead, choose new one.
	//
	// Check if goal is correct unit.
	if (goal) {
		if (!goal->IsVisibleAsGoal(unit->Player)) {
			DebugPrint("repair target gone.\n");
			unit->Orders[0]->X = goal->X;
			unit->Orders[0]->Y = goal->Y;
			goal->RefsDecrease();
			unit->Orders[0]->Goal = goal = NULL;
			unit->SubAction = 0;
		}
	} else if (unit->Player->AiEnabled) {
		// Ai players workers should stop if target is killed
		err = -1;
	}

	//
	// Have reached target?
	// FIXME: could use return value of DoActionMove
	//
	bool goalReached = goal && MapDistanceBetweenUnits(unit, goal) <= unit->Type->RepairRange;
	if (goalReached && goal->Variable[HP_INDEX].Value < goal->Variable[HP_INDEX].Max) {
		unit->State = 0;
		unit->SubAction = 20;
		unit->Data.Repair.Progress = 0;
		UnitHeadingFromDeltaXY(unit,
			goal->X + (goal->Type->TileWidth - 1) / 2 - unit->X,
			goal->Y + (goal->Type->TileHeight - 1) / 2 - unit->Y);

		int costs[MaxCosts];
		CalculateRequestedAmount(unit->Type, unit->Orders[0]->Goal->Type->ProductionCosts, costs);
		unit->Player->AddToUnitsConsumingResources(unit, costs);
	} else if (err < 0 || goalReached) {
		if (goal) { // release reference
			goal->RefsDecrease();
			unit->Orders[0]->Goal = NoUnitP;
		}
		if (!RestoreSavedOrder(unit)) {
			unit->ClearAction();
			unit->State = 0;
		}
		return;
	}

	// FIXME: Should be it already?
	Assert(unit->Orders[0]->Action == UnitActionRepair);
}

/**
**  Animate unit repair
**
**  @param unit  Unit to animate.
**  @param goal  Unit being repaired.
*/
static int AnimateActionRepair(CUnit *unit, const CUnit *goal)
{
	if (goal) {
		int dx = (goal->X - unit->X) * TileSizeX
			+ (goal->Type->TileWidth - unit->Type->TileWidth)
			  * TileSizeX / 2
			+ (goal->IX - unit->IX);
		int dy = (goal->Y - unit->Y) * TileSizeY
			+ (goal->Type->TileHeight - unit->Type->TileHeight)
			  * TileSizeY / 2
			+ (goal->IY - unit->IY);
		UnitHeadingFromDeltaXY(unit, dx, dy);
	}
	UnitShowAnimation(unit, unit->Type->Animations->Repair);
	return 0;
}

/**
**  Do the actual repair.
**
**  @param unit  unit repairing
**  @param goal  unit being repaired
**
**  @return      true if goal is healed, false otherwise
*/
static bool DoRepair(CUnit *unit, CUnit *goal)
{
	int *pcosts = goal->Type->ProductionCosts;
	int pcost = pcosts[EnergyCost] ? pcosts[EnergyCost] : pcosts[MagmaCost];
	bool healed = false;

	if (goal->Orders[0]->Action != UnitActionBuilt) {
		Assert(goal->Variable[HP_INDEX].Max);

		int *costs = unit->Player->UnitsConsumingResourcesActual[unit];
		int cost = costs[EnergyCost] ? costs[EnergyCost] : costs[MagmaCost];
		int step = pcost / goal->Variable[HP_INDEX].Max;
		int hp = 0;

		unit->Data.Repair.Progress += cost;
		while (unit->Data.Repair.Progress > step) {
			unit->Data.Repair.Progress -= step;
			++hp;
		}
		goal->Variable[HP_INDEX].Value += hp;
		if (goal->Variable[HP_INDEX].Value >= goal->Variable[HP_INDEX].Max) {
			goal->Variable[HP_INDEX].Value = goal->Variable[HP_INDEX].Max;
			healed = true;
		}
	} else {
		// hp is the current damage taken by the unit.
		int hp = (goal->Data.Built.Progress * goal->Variable[HP_INDEX].Max) / pcost - goal->Variable[HP_INDEX].Value;

		// Update build progress
		int *costs = unit->Player->UnitsConsumingResourcesActual[unit];
		int cost = costs[EnergyCost] ? costs[EnergyCost] : costs[MagmaCost];
		goal->Data.Built.Progress += cost * SpeedBuild;
		if (goal->Data.Built.Progress > pcost) {
			goal->Data.Built.Progress = pcost;
		}

		// Keep the same level of damage while increasing HP.
		goal->Variable[HP_INDEX].Value = (goal->Data.Built.Progress * goal->Stats->Variables[HP_INDEX].Max) / pcost - hp;
		if (goal->Variable[HP_INDEX].Value >= goal->Variable[HP_INDEX].Max) {
			goal->Variable[HP_INDEX].Value = goal->Variable[HP_INDEX].Max;
			healed = true;
		}
	}
	return healed;
}

/**
**  Repair unit
**
**  @param unit  Unit that's doing the repairing
*/
static void RepairUnit(CUnit *unit)
{
	CUnit *goal = unit->Orders[0]->Goal;
	bool visible = goal->IsVisibleAsGoal(unit->Player);
	bool inrange = MapDistanceBetweenUnits(unit, goal) <= unit->Type->RepairRange;
	bool healed = false;

	if (goal && visible && inrange) {
		healed = DoRepair(unit, goal);
		goal = unit->Orders[0]->Goal;
	}

	AnimateActionRepair(unit, goal);
	if (unit->Anim.Unbreakable) {
		return;
	}

	// Check if goal is gone.
	if (goal && !visible) {
		DebugPrint("repair goal is gone\n");
		unit->Orders[0]->X = goal->X;
		unit->Orders[0]->Y = goal->Y;
		goal->RefsDecrease();
		unit->Orders[0]->Goal = goal = NoUnitP;
		NewResetPath(unit);
	}

	// If goal has moved, chase after it
	if (goal && !inrange && !healed) {
		unit->Player->RemoveFromUnitsConsumingResources(unit);
		unit->State = 0;
		unit->SubAction = 0;
	}

	// Done repairing
	if (!goal || healed) {
		unit->Player->RemoveFromUnitsConsumingResources(unit);
		if (goal) { // release reference
			goal->RefsDecrease();
			unit->Orders[0]->Goal = NULL;
		}
		if (!RestoreSavedOrder(unit)) {
			unit->ClearAction();
			unit->State = 0;
		}
		return;
	}
}

/**
**  Unit repairs
**
**  @param unit  Unit that's doing the repairing
*/
void HandleActionRepair(CUnit *unit)
{
	if (unit->SubAction <= 10) {
		MoveToLocation(unit);
	}
	if (unit->SubAction == 20) {
		RepairUnit(unit);
	}
}

//@}
