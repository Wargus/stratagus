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
//      (c) Copyright 2001-2007 by Crestez Leonard and Jimmy Salmon
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

/*----------------------------------------------------------------------------
--  Functions
----------------------------------------------------------------------------*/

/**
**  Move unit to resource.
**
**  @param unit  Pointer to unit.
**
**  @return      TRUE if reached, otherwise FALSE.
*/
static int MoveToResource(CUnit *unit)
{
	CUnit *goal;

	goal = unit->Orders[0]->Goal;
	Assert(goal);
	switch (DoActionMove(unit)) { // reached end-point?
		case PF_UNREACHABLE:
			return -1;
		case PF_REACHED:
			break;
		default:
			// Goal gone or something.
			if (unit->Anim.Unbreakable || goal->IsVisibleAsGoal(unit->Player)) {
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
static int StartGathering(CUnit *unit)
{
	CUnit *goal;
	ResourceInfo *resinfo;

	Assert(!unit->IX && !unit->IY);
	resinfo = unit->Type->ResInfo[unit->CurrentResource];
	goal = unit->Orders[0]->Goal;

	//
	// Target is gone, stop getting resources.
	//
	if (!goal->IsVisibleAsGoal(unit->Player)) {
		goal->RefsDecrease();
		// Find an alternative, but don't look too far.
		unit->Orders[0]->X = unit->Orders[0]->Y = -1;
		if ((goal = UnitFindResource(unit, unit->X, unit->Y, 10, unit->CurrentResource))) {
			unit->SubAction = SUB_START_RESOURCE;
			unit->Orders[0]->Goal = goal;
			goal->RefsIncrease();
		} else {
			unit->Orders[0]->Action = UnitActionStill;
			unit->Orders[0]->Goal = NoUnitP;
			unit->SubAction = 0;
		}
		return 0;
	}

	// FIXME: 0 can happen, if placed too near by map maker.
	Assert(MapDistanceBetweenUnits(unit, goal) <= 1);

	//
	// Update the heading of a harvesting unit to looks straight at the resource.
	//
	if (goal) {
		UnitHeadingFromDeltaXY(unit,
			2 * (goal->X - unit->X) + goal->Type->TileWidth,
			2 * (goal->Y - unit->Y) + goal->Type->TileHeight);
	}

	//
	// If resource is still under construction, wait!
	//
	if ((goal->Type->MaxOnBoard && goal->Data.Resource.Active >= goal->Type->MaxOnBoard) ||
			goal->Orders[0]->Action == UnitActionBuilt) {
		// FIXME: Determine somehow when the resource will be free to use
		// FIXME: Could we somehow find another resource?
		// However the CPU usage is really low (no pathfinding stuff).
		unit->Wait = 10;
		return 0;
	}

	// Activate the resource
	goal->Data.Resource.Active++;

	//
	// Place unit inside the resource
	//
	if (!resinfo->HarvestFromOutside) {
		goal->RefsDecrease();
		unit->Orders[0]->Goal = NoUnitP;

		unit->Remove(goal);
	}

	unit->Data.ResWorker.TimeToHarvest = 0;

	return 1;
}

/**
**  Animate a unit that is harvesting
**
**  @param unit  Unit to animate
*/
static void AnimateActionHarvest(CUnit *unit)
{
	Assert(unit->Type->Animations->Harvest[unit->CurrentResource]);
	UnitShowAnimation(unit, unit->Type->Animations->Harvest[unit->CurrentResource]);
}

/**
**  Find something else to do when the resource is exhausted.
**  This is called from GatherResorce when the resource is empty.
**
**  @param unit    pointer to harvester unit.
**  @param source  pointer to resource unit.
*/
static void LoseResource(CUnit *unit, const CUnit *source)
{
	ResourceInfo *resinfo = unit->Type->ResInfo[unit->CurrentResource];

	Assert((unit->Container && !resinfo->HarvestFromOutside) ||
		(!unit->Container && resinfo->HarvestFromOutside));

	if (resinfo->HarvestFromOutside) {
		unit->Orders[0]->Goal->RefsDecrease();
		unit->Orders[0]->Goal = NoUnitP;
	}

	//
	// Dump the unit outside and look for something to do.
	//
	if (unit->Container) {
		DropOutOnSide(unit, LookingW, source->Type->TileWidth,
			source->Type->TileHeight);
	}
	unit->Orders[0]->X = unit->Orders[0]->Y = -1;
	if ((unit->Orders[0]->Goal = UnitFindResource(unit, unit->X, unit->Y,
			10, unit->CurrentResource))) {
		DebugPrint("Unit %d found another resource.\n" _C_ unit->Slot);
		unit->SubAction = SUB_START_RESOURCE;
		unit->State = 0;
		unit->Orders[0]->Goal->RefsIncrease();
	} else {
		DebugPrint("Unit %d did not find another resource.\n" _C_ unit->Slot);
		unit->Orders[0]->Action = UnitActionStill;
		unit->SubAction = 0;
		unit->State = 0;
	}
}

/**
**  Gather the resource
**
**  @param unit  Pointer to unit.
*/
static void GatherResource(CUnit *unit)
{
	CUnit *source = NoUnitP;
	ResourceInfo *resinfo = unit->Type->ResInfo[unit->CurrentResource];
	int addload = 1;
	int visible = 0;

	if (resinfo->HarvestFromOutside) {
		AnimateActionHarvest(unit);
	} else {
		unit->Anim.CurrAnim = NULL;
	}

	unit->Data.ResWorker.TimeToHarvest--;

	while (unit->Data.ResWorker.TimeToHarvest < 0) {
		unit->Data.ResWorker.TimeToHarvest += 1; // / SpeedResourcesHarvest[resinfo->ResourceId];

		//
		// Calculate how much we can load.
		//
		if (resinfo->ResourceStep) {
			addload = resinfo->ResourceStep;
		} else {
			addload = source->ResourcesHeld;
		}

		if (resinfo->HarvestFromOutside) {
			source = unit->Orders[0]->Goal;
		} else {
			source = unit->Container;
		}
		Assert(source);

		//
		// Target is not dead, getting resources.
		//
		visible = source->IsVisibleAsGoal(unit->Player);
		if (visible) {
			// Don't load more than there is.
			if (addload > source->ResourcesHeld) {
				addload = source->ResourcesHeld;
			}
			source->ResourcesHeld -= addload;
		}

		//
		// End of resource: destroy the resource.
		//
		if (!visible || source->ResourcesHeld == 0) {
			if (resinfo->HarvestFromOutside && unit->Anim.Unbreakable) {
				// Continue until the animation is breakable
				unit->Data.ResWorker.TimeToHarvest = 0;
				return;
			}

			DebugPrint("Resource is destroyed for unit %d\n" _C_ unit->Slot);

			//
			// Improved version of DropOutAll.
			//
			LoseResource(unit, source);
			CUnit *uins = source->UnitInside;
			for (int i = source->InsideCount; i; --i, uins = uins->NextContained) {
				if (uins->Orders[0]->Action == UnitActionResource) {
					LoseResource(uins, source);
				}
			}

			// Don't destroy the resource twice.
			// This only happens when it's empty.
			if (visible) {
				LetUnitDie(source);
			}
		}
	}
}

/**
**  Give up on gathering.
**
**  @param unit  Pointer to unit.
*/
void ResourceGiveUp(CUnit *unit)
{
	DebugPrint("Unit %d gave up on resource gathering.\n" _C_ unit->Slot);
	if (unit->Orders[0]->Goal) {
		unit->Orders[0]->Goal->RefsDecrease();
		unit->Orders[0]->Goal = NULL;
	}
	unit->Orders[0]->Init();
	unit->Orders[0]->Action = UnitActionStill;
	unit->SubAction = 0;
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
	int ret;

	if (unit->Wait) {
		// FIXME: show idle animation while we wait?
		unit->Wait--;
		return;
	}

	// Let's start mining.
	if (unit->SubAction == SUB_START_RESOURCE) {
		unit->CurrentResource = unit->Orders[0]->Goal->Type->GivesResource;
		if (unit->CurrentResource) {
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
		// -1 failure, 0 not yet reached, 1 reached
		if ((ret = MoveToResource(unit))) {
			if (ret == -1) {
				// Can't Reach
				unit->SubAction++;
				unit->Wait = 10;
				return;
			} else {
				// Reached
				unit->SubAction = SUB_START_GATHERING;
			}
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
