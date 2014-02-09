//     ____                _       __               
//    / __ )____  _____   | |     / /___ ___________
//   / __  / __ \/ ___/   | | /| / / __ `/ ___/ ___/
//  / /_/ / /_/ (__  )    | |/ |/ / /_/ / /  (__  ) 
// /_____/\____/____/     |__/|__/\__,_/_/  /____/  
//                                              
//       A futuristic real-time strategy game.
//          This file is part of Bos Wars.
//
/**@name action_build.cpp - The build building action. */
//
//      (c) Copyright 1998-2008 by Lutz Sammer, Jimmy Salmon, and
//                                 Russell Smith
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
#include "unit_cache.h"
#include "sound.h"
#include "actions.h"
#include "map.h"
#include "ai.h"
#include "interface.h"
#include "pathfinder.h"
#include "construct.h"

/*----------------------------------------------------------------------------
--  Functions
----------------------------------------------------------------------------*/

/**
**  Get the production cost of a unit type
**  Use the energy cost if it's not 0, otherwise use magma cost
*/
static int GetProductionCost(CUnitType *type)
{
	int *costs = type->ProductionCosts;
	return costs[EnergyCost] ? costs[EnergyCost] : costs[MagmaCost];
}

/**
**  Update construction frame
**
**  @param unit  The building under construction.
*/
static void UpdateConstructionFrame(CUnit *unit)
{
	CConstructionFrame *cframe;
	CConstructionFrame *tmp;
	int percent = unit->Data.Built.Progress * 100 / GetProductionCost(unit->Type);

	// Find which construction frame to use
	cframe = tmp = unit->Type->Construction->Frames;
	while (tmp) {
		if (percent < tmp->Percent) {
			break;
		}
		cframe = tmp;
		tmp = tmp->Next;
	}

	// Update the frame
	if (cframe != unit->Data.Built.Frame) {
		unit->Data.Built.Frame = cframe;
		if (unit->Frame < 0) {
			unit->Frame = -cframe->Frame - 1;
		} else {
			unit->Frame = cframe->Frame;
		}
	}
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

	switch (DoActionMove(unit)) { // reached end-point?
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

			unit->Player->Notify(NotifyYellow, unit->X, unit->Y,
				_("You cannot reach building place"));
			if (unit->Player->AiEnabled) {
				AiCanNotReach(unit, unit->Orders[0]->Type);
			}

			unit->ClearAction();
			return;

		case PF_REACHED:
			unit->SubAction = 20;
			return;

		default:
			// Moving...
			return;
	}
}

/**
**  Check if we're already building this type at this location
*/
static CUnit *CheckAlreadyBuilding(CUnit *unit, CUnitType *type, int x, int y)
{
	CUnit *table[UnitMax];
	int n;

	n = UnitCache.Select(x, y, table, UnitMax);
	for (int i = 0; i < n; ++i) {
		if (!table[i]->Destroyed && table[i]->Type == type &&
				(table[i]->Player == unit->Player || unit->IsAllied(table[i]))) {
			return table[i];
		}
	}
	return NULL;
}

/**
**  Check if the unit can build
**
**  @param unit  Unit to check
*/
static CUnit *CheckCanBuild(CUnit *unit)
{
	int x;
	int y;
	CUnitType *type;
	CUnit *ontop;

	if (unit->Wait) {
		// FIXME: show still animation while we wait?
		unit->Wait--;
		return NULL;
	}

	x = unit->Orders[0]->X;
	y = unit->Orders[0]->Y;
	type = unit->Orders[0]->Type;

	//
	// Check if the building could be built there.
	//
	if ((ontop = CanBuildUnitType(unit, type, x, y, 1)) == NULL) {
		if ((ontop = CheckAlreadyBuilding(unit, type, x, y)) != NULL) {
			unit->Orders[0]->Init();
			unit->Orders[0]->Action = UnitActionRepair;
			unit->Orders[0]->Goal = ontop;
			unit->Orders[0]->Goal->RefsIncrease();
			unit->Orders[0]->Range = unit->Type->RepairRange;
			unit->SubAction = 0;
			return NULL;
		}

		//
		// Some tries to build the building.
		//
		if (unit->SubAction++ < 30) {
			// To keep the load low, retry each 10 cycles
			// NOTE: we can already inform the AI about this problem?
			unit->Wait = 10;
			return NULL;
		}

		unit->Player->Notify(NotifyYellow, unit->X, unit->Y,
			_("You cannot build %s here"), type->Name.c_str());
		if (unit->Player->AiEnabled) {
			AiCanNotBuild(unit, type);
		}

		unit->ClearAction();
		return NULL;
	}

	//
	// Check if hiting any limits for the building.
	//
	if (unit->Player->CheckLimits(type) < 0) {
		unit->Player->Notify(NotifyYellow, unit->X, unit->Y,
			_("Can't build more units %s"), type->Name.c_str());
		if (unit->Player->AiEnabled) {
			AiCanNotBuild(unit, type);
		}

		unit->ClearAction();
		return NULL;
	}

	return ontop;
}

/**
**  Start building
*/
static void StartBuilding(CUnit *unit, CUnit *ontop)
{
	int x;
	int y;
	CUnitType *type;
	CUnit *build;

	x = unit->Orders[0]->X;
	y = unit->Orders[0]->Y;
	type = unit->Orders[0]->Type;

	build = MakeUnit(type, unit->Player);
	
	// If unable to make unit, stop, and report message
	if (build == NoUnitP) {
		// FIXME: Should we retry this?
		unit->Player->Notify(NotifyYellow, unit->X, unit->Y,
			_("Unable to create building %s"), type->Name.c_str());
		if (unit->Player->AiEnabled) {
			AiCanNotBuild(unit, type);
		}

		unit->ClearAction();
		return;
	}

	build->Constructed = 1;
	build->CurrentSightRange = 0;

	// Building on top of something, may remove what is beneath it
	if (ontop != unit) {
		CBuildRestrictionOnTop *b;

		build->ProductionEfficiency = ontop->Type->ProductionEfficiency;
		b = static_cast<CBuildRestrictionOnTop *>(OnTopDetails(build, ontop->Type));
		Assert(b);
		if (b->ReplaceOnBuild) {
			// We capture the value of what is beneath.
			memcpy(build->ResourcesHeld, ontop->ResourcesHeld, sizeof(build->ResourcesHeld));
			ontop->Remove(NULL); // Destroy building beneath
			UnitLost(ontop);
			UnitClearOrders(ontop);
			ontop->Release();
		}
	}

	// Must set action before placing, otherwise it will incorrectly mark radar
	build->Orders[0]->Action = UnitActionBuilt;
	
	// Must place after previous for map flags
	build->Place(x, y);

	// HACK: the building is not ready yet
	build->Player->UnitTypesCount[type->Slot]--;

	// Make sure the building doesn't cancel itself out right away.
	build->Data.Built.Progress = 0;
	build->Variable[HP_INDEX].Value = 1;
	UpdateConstructionFrame(build);

	// We need somebody to work on it.
	// Use repair to do the building
	unit->Orders[0]->Action = UnitActionRepair;
	unit->Orders[0]->Goal = build;
	unit->Orders[0]->X = unit->Orders[0]->Y = -1;
	unit->Orders[0]->Range = unit->Type->RepairRange;
	unit->SubAction = 0;
	unit->Direction = DirectionToHeading(x - unit->X, y - unit->Y);
	UnitUpdateHeading(unit);
	build->RefsIncrease();
	// Mark the new building seen.
	MapMarkUnitSight(build);

	UpdateConstructionFrame(build);
}

/**
**  Unit builds a building.
**
**  @param unit  Unit that builds a building
*/
void HandleActionBuild(CUnit *unit)
{
	CUnit *ontop;

	if (unit->SubAction <= 10) {
		MoveToLocation(unit);
	}
	if (20 <= unit->SubAction && unit->SubAction <= 30) {
		if ((ontop = CheckCanBuild(unit))) {
			StartBuilding(unit, ontop);
		}
	}
}


/**
**  Unit under Construction
**
**  @param unit  Unit that is being built
*/
void HandleActionBuilt(CUnit *unit)
{
	int pcost = GetProductionCost(unit->Type);

	//
	// Check if construction should be canceled
	//
	if (unit->Data.Built.Cancel || unit->Data.Built.Progress < 0) {
		DebugPrint("%s canceled.\n" _C_ unit->Type->Name.c_str());

		// Cancel building
		LetUnitDie(unit);
		return;
	}

	//
	// Check if building ready. Note we can both build and repair.
	//
	if (unit->Data.Built.Progress >= pcost ||
			unit->Variable[HP_INDEX].Value >= unit->Stats->Variables[HP_INDEX].Max) {
		DebugPrint("Building ready.\n");
		if (unit->Variable[HP_INDEX].Value > unit->Stats->Variables[HP_INDEX].Max) {
			unit->Variable[HP_INDEX].Value = unit->Stats->Variables[HP_INDEX].Max;
		}
		unit->ClearAction();
		// HACK: the building is ready now
		unit->Player->UnitTypesCount[unit->Type->Slot]++;
		unit->Constructed = 0;
		if (unit->Frame < 0) {
			unit->Frame = -unit->Type->StillFrame - 1;
		} else {
			unit->Frame = unit->Type->StillFrame;
		}

		unit->Player->Notify(NotifyGreen, unit->X, unit->Y,
			_("New %s done"), unit->Type->Name.c_str());
		if (unit->Player == ThisPlayer) {
			if (unit->Type->Sound.Ready.Sound) {
				PlayUnitSound(unit, VoiceReady);
			} else {
				PlayUnitSound(unit, VoiceBuilding);
			}
		}
		if (unit->Player->AiEnabled) {
			AiWorkComplete(NULL, unit);
		}

		UpdateForNewUnit(unit, 0);

		// Set the direction of the building if it supports them
		if (unit->Type->NumDirections > 1) {
			unit->Direction = (MyRand() >> 3) & 0xFF; // random heading
			UnitUpdateHeading(unit);
		}

		if (IsOnlySelected(unit) || unit->Player == ThisPlayer) {
			SelectedUnitChanged();
		}
		unit->CurrentSightRange = unit->Stats->Variables[SIGHTRANGE_INDEX].Max;
		MapMarkUnitSight(unit);
		return;
	}

	UpdateConstructionFrame(unit);
}

//@}
