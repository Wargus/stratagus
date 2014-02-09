//     ____                _       __               
//    / __ )____  _____   | |     / /___ ___________
//   / __  / __ \/ ___/   | | /| / / __ `/ ___/ ___/
//  / /_/ / /_/ (__  )    | |/ |/ / /_/ / /  (__  ) 
// /_____/\____/____/     |__/|__/\__,_/_/  /____/  
//                                              
//       A futuristic real-time strategy game.
//          This file is part of Bos Wars.
//
/**@name action_resource.cpp - The generic resource action. */
//
//      (c) Copyright 2001-2008 by Crestez Leonard and Jimmy Salmon
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
#include "player.h"
#include "unit.h"
#include "unittype.h"
#include "animation.h"
#include "actions.h"
#include "pathfinder.h"
#include "interface.h"
#include "sound.h"

/*----------------------------------------------------------------------------
--  Declarations
----------------------------------------------------------------------------*/

#define SUB_START_RESOURCE 0
#define SUB_MOVE_TO_RESOURCE 5
#define SUB_UNREACHABLE_RESOURCE 31
#define SUB_START_GATHERING 55
#define SUB_GATHER_RESOURCE 60

int AlliedUnitRecyclingEfficiency[MaxCosts] = {0, 50};
int EnemyUnitRecyclingEfficiency[MaxCosts] = {0, 40};

/*----------------------------------------------------------------------------
--  Functions
----------------------------------------------------------------------------*/

/**
**  Move unit to resource.
**
**  @param unit  Pointer to unit.
**
**  @return      -1 if unreachable, 0 if moving, 1 if reached.
*/
static int MoveToResource(CUnit *unit)
{
	switch (DoActionMove(unit)) { // reached end-point?
		case PF_UNREACHABLE:
			return -1;
		case PF_REACHED:
			break;
		default:
			// Goal gone or something.
			if (unit->Anim.Unbreakable || unit->Orders[0]->Goal->IsVisibleAsGoal(unit->Player)) {
				return 0;
			}
			break;
	}
	return 1;
}

/**
**  Start harvesting the resource.
**
**  @param unit  Pointer to unit.
**
**  @return      TRUE if ready, otherwise FALSE.
*/
static bool StartGathering(CUnit *unit)
{
	CUnit *goal = unit->Orders[0]->Goal;

	Assert(!unit->IX && !unit->IY);

	//
	// Target is gone, stop getting resources.
	//
	if (!goal->IsVisibleAsGoal(unit->Player)) {
		goal->RefsDecrease();
		// Find an alternative, but don't look too far.
		unit->Orders[0]->X = unit->Orders[0]->Y = -1;
		if ((goal = UnitFindResource(unit, unit->X, unit->Y, 10))) {
			unit->SubAction = SUB_START_RESOURCE;
			unit->Orders[0]->Goal = goal;
			goal->RefsIncrease();
		} else {
			unit->ClearAction();
			unit->Orders[0]->Goal = NoUnitP;
		}
		return false;
	}

	// 0 can happen, for example with flying units.
	Assert(MapDistanceBetweenUnits(unit, goal) <= 1);

	//
	// Update the heading of a harvesting unit to look straight at the resource.
	//
	UnitHeadingFromDeltaXY(unit,
		goal->X + (goal->Type->TileWidth - 1) / 2 - unit->X,
		goal->Y + (goal->Type->TileHeight - 1) / 2 - unit->Y);

	return true;
}

/**
**  Animate a unit that is harvesting
**
**  @param unit  Unit to animate
*/
static void AnimateActionHarvest(CUnit *unit)
{
	Assert(unit->Type->Animations->Harvest);
	UnitShowAnimation(unit, unit->Type->Animations->Harvest);
}

/**
**  Check which type of resource was harvested from a unit,
**  so that the caller can look for more of the same.
**
**  @param exhausted  Unit from which resources were harvested.
**
**  @return           Either enum CostType, or -1 if the resource
**                    type could not be identified.
*/
static int HarvestedResourceType(const CUnit *exhausted)
{
	int found = -1;
	for (int res = 0; res < MaxCosts; res++) {
		// exhausted->ResourcesHeld[res] is presumably zero
		// already so check the unit type instead.
		if (exhausted->Type->ProductionCosts[res] > 0) {
			if (found == -1) {
				found = res;
			} else {
				// It contains more than one type of
				// resource.  We could find the
				// largest number but the amounts are
				// not really comparable.
				return -1;
			}
		}
	}

	// The unit type contains just one type of resource, or none
	// at all.  In the latter case, found == -1 still.
	return found;
}

/**
**  Find something else to do when the resource is exhausted.
**  This is called from GatherResource when the resource is empty.
**
**  @param unit  Harvester unit
*/
static void FindNewResource(CUnit *unit)
{
	const int res = HarvestedResourceType(unit->Orders[0]->Goal);
	unit->Orders[0]->Goal->RefsDecrease();
	unit->Orders[0]->Goal = NoUnitP;

	unit->Orders[0]->X = unit->Orders[0]->Y = -1;
	if ((unit->Orders[0]->Goal = UnitFindResource(unit, unit->X, unit->Y, 10, res))) {
		DebugPrint("Unit %d found another resource.\n" _C_ unit->Slot);
		unit->SubAction = SUB_START_RESOURCE;
		unit->State = 0;
		unit->Orders[0]->Goal->RefsIncrease();
	} else {
		DebugPrint("Unit %d did not find another resource.\n" _C_ unit->Slot);
		unit->ClearAction();
		unit->State = 0;
	}
}

/**
**  Gather the resource
**
**  @param unit  Harvester unit
*/
static void GatherResource(CUnit *unit)
{
	CUnit *source = unit->Orders[0]->Goal;
	int amount[MaxCosts] = {1, 1};
	bool visible = source->IsAliveOnMap();
	int i;
	const int totalEfficiency[MaxCosts] = {100, 100};
	const int *efficiency = totalEfficiency;

	AnimateActionHarvest(unit);

	// Calculate how much we can harvest.
	if (visible && UnitHoldsResources(source)) {
		// Determine efficiency of recycling buildings
		if (unit->Player == source->Player || unit->Player->IsAllied(source)) {
			efficiency = AlliedUnitRecyclingEfficiency;
		} else if (unit->Player->IsEnemy(source)) {
			efficiency = EnemyUnitRecyclingEfficiency;
		}

		// Update ResourcesHeld based on HP
		// Source might have been healed or attacked
		// Is this the best place for this?
		if (efficiency != totalEfficiency) {
			int hp = source->Variable[HP_INDEX].Max *
				source->ResourcesHeld[1] / source->Type->ProductionCosts[1];
			if (hp != 0 && hp != source->Variable[HP_INDEX].Value) {
				for (i = 0; i < MaxCosts; ++i) {
					source->ResourcesHeld[i] = source->Type->ProductionCosts[i] *
						source->Variable[HP_INDEX].Value / source->Variable[HP_INDEX].Max;
				}
			}
		}

		// Calculate new resource amounts
		CalculateRequestedAmount(unit->Type, source->ResourcesHeld, amount);
		for (i = 0; i < MaxCosts; ++i) {
			unit->Player->ProductionRate[i] -= unit->Data.Harvest.CurrentProduction[i];
			unit->Data.Harvest.CurrentProduction[i] = amount[i] * efficiency[i] / 100;
			unit->Player->ProductionRate[i] += unit->Data.Harvest.CurrentProduction[i];
			source->ResourcesHeld[i] -= amount[i];
		}

		// Recycling loses hit points
		if (efficiency != totalEfficiency) {
			source->Variable[HP_INDEX].Value = source->Variable[HP_INDEX].Max *
				source->ResourcesHeld[1] / source->Type->ProductionCosts[1];
			if (source->Variable[HP_INDEX].Value == 0 && source->ResourcesHeld[1] != 0) {
				source->Variable[HP_INDEX].Value = 1;
			}
		}
	}

	//
	// End of resource: destroy the resource.
	//
	if (!visible || !UnitHoldsResources(source)) {
		if (unit->Anim.Unbreakable) {
			// Wait until the animation finishes
			return;
		}

		DebugPrint("Resource is destroyed for unit %d\n" _C_ unit->Slot);

		for (i = 0; i < MaxCosts; ++i) {
			unit->Player->ProductionRate[i] -= unit->Data.Harvest.CurrentProduction[i];
			unit->Data.Harvest.CurrentProduction[i] = 0;
		}

		if (unit->OrderCount == 1) {
			// Find a new resource to harvest
			FindNewResource(unit);
		} else {
			unit->Orders[0]->Goal->RefsDecrease();
			unit->Orders[0]->Goal = NoUnitP;
			unit->ClearAction();
		}

		// Don't destroy the resource twice.
		// This only happens when it's empty.
		if (visible) {
			LetUnitDie(source);
		}
	}
}

/**
**  Give up on gathering.
**
**  @param unit  Pointer to unit.
*/
static void ResourceGiveUp(CUnit *unit)
{
	DebugPrint("Unit %d gave up on resource gathering.\n" _C_ unit->Slot);
	if (unit->Orders[0]->Goal) {
		unit->Orders[0]->Goal->RefsDecrease();
		unit->Orders[0]->Goal = NULL;
	}
	unit->ClearAction();
}

/**
**  Control the unit action: getting a resource.
**
**  This is the generic function for harvesting resources
**
**  @param unit  Pointer to unit.
*/
void HandleActionResource(CUnit *unit)
{
	if (unit->Wait) {
		unit->Wait--;
		return;
	}

	// Let's start mining.
	if (unit->SubAction == SUB_START_RESOURCE) {
		if (unit->Orders[0]->Goal->Type->CanHarvestFrom) {
			NewResetPath(unit);
			unit->SubAction = SUB_MOVE_TO_RESOURCE;
		} else {
			ResourceGiveUp(unit);
			return;
		}
	}

	// Move to the resource location.
	if (unit->SubAction >= SUB_MOVE_TO_RESOURCE &&
			unit->SubAction < SUB_UNREACHABLE_RESOURCE) {
		int ret = MoveToResource(unit);
		// -1 failure, 0 not yet reached, 1 reached
		if (ret == -1) {
			// Can't Reach
			unit->SubAction++;
			unit->Wait = 10;
			return;
		} else if (ret == 1) {
			// Reached
			unit->SubAction = SUB_START_GATHERING;
			memset(unit->Data.Harvest.CurrentProduction, 0, sizeof(unit->Data.Harvest.CurrentProduction));
		} else {
			// Move along.
			return;
		}
	}

	// Resource seems to be unreachable
	if (unit->SubAction == SUB_UNREACHABLE_RESOURCE) {
		ResourceGiveUp(unit);
		return;
	}

	// Start gathering the resource
	if (unit->SubAction == SUB_START_GATHERING) {
		if (StartGathering(unit)) {
			unit->SubAction = SUB_GATHER_RESOURCE;
		} else {
			return;
		}
	}

	// Gather the resource.
	if (unit->SubAction == SUB_GATHER_RESOURCE) {
		GatherResource(unit);
		return;
	}
}

//@}
