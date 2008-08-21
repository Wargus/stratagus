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
/**@name action_build.cpp - The build building action. */
//
//      (c) Copyright 1998-2005 by Lutz Sammer, Jimmy Salmon, and
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
**  Update construction frame
**
**  @param unit  The building under construction.
*/
static void UpdateConstructionFrame(CUnit *unit)
{
	CConstructionFrame *cframe;
	CConstructionFrame *tmp;
	int percent;

	percent = unit->Data.Built.Progress /
		(unit->Type->Stats[unit->Player->Index].Costs[TimeCost] * 6);
	cframe = tmp = unit->Type->Construction->Frames;
	while (tmp) {
		if (percent < tmp->Percent) {
			break;
		}
		cframe = tmp;
		tmp = tmp->Next;
	}
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
				// To keep the load low, retry each 1/4 second.
				// NOTE: we can already inform the AI about this problem?
				unit->Wait = CYCLES_PER_SECOND / 4 + unit->SubAction;
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

struct CheckAlreadyBuilding {
	const CUnit *const worker;
	const CUnitType *const type;
	CheckAlreadyBuilding(const CUnit *unit,	const CUnitType *t):
			 worker(unit), type(t) {}
	inline bool operator() (CUnit *const unit) {
		return (!unit->Destroyed && unit->Type == type &&
				(worker->Player == unit->Player || worker->IsAllied(unit)));
	}
};

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
		CheckAlreadyBuilding filter(unit, type);
		/*
		 *	FIXME: rb - CheckAlreadyBuilding should be somehow
		 *	ebabled/disable via game lua scripting
		 */
		if ((ontop = Map.Field(x,y)->UnitCache.find(filter)) != NULL) {
			DebugPrint("%d: Worker [%d] is helping build: %s [%d]\n" 
					_C_ unit->Player->Index _C_ unit->Slot 
					_C_ ontop->Type->Name.c_str()
					_C_ ontop->Slot);
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
	// Check if enough resources for the building.
	//
	if (unit->Player->CheckUnitType(type)) {
		// FIXME: Better tell what is missing?
		unit->Player->Notify(NotifyYellow, unit->X, unit->Y,
			_("Not enough resources to build %s"), type->Name.c_str());
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
	const CUnitStats *stats;

	x = unit->Orders[0]->X;
	y = unit->Orders[0]->Y;
	type = unit->Orders[0]->Type;

	unit->Player->SubUnitType(type);

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

		b = static_cast<CBuildRestrictionOnTop *> (OnTopDetails(build, ontop->Type));
		Assert(b);
		if (b->ReplaceOnBuild) {
			build->ResourcesHeld = ontop->ResourcesHeld; // We capture the value of what is beneath.
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
	if (!type->BuilderOutside) {
		build->CurrentSightRange = 1;
	}

	// HACK: the building is not ready yet
	build->Player->UnitTypesCount[type->Slot]--;

	stats = build->Stats;

	// Make sure the bulding doesn't cancel itself out right away.
	build->Data.Built.Progress = 0;//FIXME ? 100 : 0
	build->Variable[HP_INDEX].Value = 1;
	UpdateConstructionFrame(build);

	// We need somebody to work on it.
	if (!type->BuilderOutside) {
		//FIXME: cancel buld gen crash
		// Place the builder inside the building
		build->Data.Built.Worker = unit;
		// HACK: allows the unit to be removed
		build->CurrentSightRange = 1;
		//HACK: reset anim
		UnitShowAnimation(unit, unit->Type->Animations->Still);
		unit->Remove(build);
		build->CurrentSightRange = 0;
		unit->X = x;
		unit->Y = y;
		unit->Orders[0]->Action = UnitActionBuild;
		unit->Data.Build.Cycles = 0;
		unit->SubAction = 40;
		unit->Orders[0]->Goal = build;
		build->RefsIncrease();
		if (unit->Selected) {
			SelectedUnitChanged();
		}
	} else {
		// Use repair to do the building
		unit->Orders[0]->Action = UnitActionRepair;
		unit->Orders[0]->Goal = build;
		unit->Orders[0]->X = unit->Orders[0]->Y = -1;
		// FIXME: Should have a BuildRange?
		unit->Orders[0]->Range = unit->Type->RepairRange;
		unit->SubAction = 0;
		unit->Direction = DirectionToHeading(x - unit->X, y - unit->Y);
		UnitUpdateHeading(unit);
		build->RefsIncrease();
		// Mark the new building seen.
		MapMarkUnitSight(build);
	}
	UpdateConstructionFrame(build);
}

/**
**  Build the building
**
**  @param unit  worker which build.
*/
static void BuildBuilding(CUnit *unit)
{

	UnitShowAnimation(unit, unit->Type->Animations->Build);
	unit->Data.Build.Cycles++;
	if (unit->Anim.Unbreakable) {
		return ;
	}
	
	//
	// Calculate the length of the attack (repair) anim.
	//
	//int animlength = unit->Data.Build.Cycles;
	unit->Data.Build.Cycles = 0;
#if 0
	CUnit *goal;
	int hp;
	
	//goal hp are mod by HandleActionBuilt
	//and outsid builder use repair now.		
	goal = unit->Orders[0]->Goal;
	//Assert(goal);
	if(!goal) {
		return;
	}

	if (goal->Orders[0]->Action == UnitActionDie) {
		goal->RefsDecrease();
		unit->Orders[0]->Goal = NULL;
		unit->State = 0;
		unit->ClearAction();
		return;
	}

	// hp is the current damage taken by the unit.
	hp = (goal->Data.Built.Progress * goal->Variable[HP_INDEX].Max) /
		(goal->Stats->Costs[TimeCost] * 600) - goal->Variable[HP_INDEX].Value;

	// FIXME: implement this below:
	// unit->Data.Built.Worker->Type->BuilderSpeedFactor;
	goal->Data.Built.Progress += 100 * animlength * SpeedBuild;
	// Keep the same level of damage while increasing HP.
	goal->Variable[HP_INDEX].Value = (goal->Data.Built.Progress * goal->Variable[HP_INDEX].Max) /
		(goal->Stats->Costs[TimeCost] * 600) - hp;
	if (goal->Variable[HP_INDEX].Value > goal->Variable[HP_INDEX].Max) {
		goal->Variable[HP_INDEX].Value = goal->Variable[HP_INDEX].Max;
	}

	//
	// Building is gone or finished
	//
	if (goal->Variable[HP_INDEX].Value == goal->Variable[HP_INDEX].Max) {
		goal->RefsDecrease();
		unit->Orders[0]->Goal = NULL;
		unit->State = 0;
		unit->ClearAction();
	}
#endif
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
	if (unit->SubAction == 40) {
		BuildBuilding(unit);
	}
}

/**
**  Unit under Construction
**
**  @param unit  Unit that is being built
*/
void HandleActionBuilt(CUnit *unit)
{
	CUnit *worker;
	CUnitType *type;
	int n, mod;
	int progress;

	type = unit->Type;

	// mod is use for round to upper
	mod = (unit->Stats->Costs[TimeCost] * 600) - unit->Variable[HP_INDEX].Value;
	
	// n is the current damage taken by the unit.
	n = (unit->Data.Built.Progress * unit->Variable[HP_INDEX].Max + (mod - 1)) / mod;
	
	// This below is most often 0
	if (type->BuilderOutside) {
		progress = unit->Type->AutoBuildRate;
	} else {
		progress = 100;
		// FIXME: implement this below:
		// unit->Data.Built.Worker->Type->BuilderSpeedFactor;
	}
	// Building speeds increase or decrease.
	progress *= SpeedBuild;
	unit->Data.Built.Progress += progress;
	// mod is use for round to upper and use it as cache
	mod = type->Stats[unit->Player->Index].Costs[TimeCost] * 600;
	
	// Keep the same level of damage while increasing HP.
	unit->Variable[HP_INDEX].Value =
	 (unit->Data.Built.Progress * unit->Variable[HP_INDEX].Max + (mod - n - 1)) /
		(mod - n);
	if (unit->Variable[HP_INDEX].Value > unit->Stats->Variables[HP_INDEX].Max) {
		unit->Variable[HP_INDEX].Value = unit->Stats->Variables[HP_INDEX].Max;
	}

	//
	// Check if construction should be canceled...
	//
	if (unit->Data.Built.Cancel || unit->Data.Built.Progress < 0) {
		DebugPrint("%d: %s canceled.\n" _C_ unit->Player->Index 
				_C_ unit->Type->Name.c_str());
		// Drop out unit
		if ((worker = unit->Data.Built.Worker)) {
		
			if(worker->Orders[0]->Goal) {
				worker->Orders[0]->Goal->RefsDecrease();
				worker->Orders[0]->Goal = NULL;
			}
		
			worker->ClearAction();
			//worker->State = 0;
			
			unit->Data.Built.Worker = NoUnitP;
			// HACK: make sure the sight is updated correctly
			unit->CurrentSightRange = 1;
			DropOutOnSide(worker, LookingW, type->TileWidth, type->TileHeight);
			unit->CurrentSightRange = 0;
		}

		// Player gets back 75% of the original cost for a building.
		unit->Player->AddCostsFactor(unit->Stats->Costs, CancelBuildingCostsFactor);
		// Cancel building
		LetUnitDie(unit);
		return;
	}

	//
	// Check if building ready. Note we can both build and repair.
	//
	//if (unit->Data.Built.Progress >= unit->Stats->Costs[TimeCost] * 600 ||
	if (unit->Data.Built.Progress >= mod ||
			unit->Variable[HP_INDEX].Value >= unit->Stats->Variables[HP_INDEX].Max) {
		DebugPrint("%d: Building %s(%s) ready.\n" _C_ unit->Player->Index
		_C_ unit->Type->Ident.c_str() _C_ unit->Type->Name.c_str() );
		if (unit->Variable[HP_INDEX].Value > unit->Stats->Variables[HP_INDEX].Max) {
			unit->Variable[HP_INDEX].Value = unit->Stats->Variables[HP_INDEX].Max;
		}
		unit->ClearAction();
		// HACK: the building is ready now
		unit->Player->UnitTypesCount[type->Slot]++;
		unit->Constructed = 0;
		if (unit->Frame < 0) {
			unit->Frame = -1;
		} else {
			unit->Frame = 0;
		}

		if (type->GivesResource) {
			// Set to Zero as it's part of a union
			//unit->Data.Resource.Active = 0;
			memset(&unit->Data, 0, sizeof(unit->Data));
			// Has StartingResources, Use those
			if (type->StartingResources) {
				unit->ResourcesHeld = type->StartingResources;
			}
		}

		if ((worker = unit->Data.Built.Worker)) {
			// Bye bye worker.
			if (type->BuilderLost) {
				// FIXME: enough?
				LetUnitDie(worker);
				worker = NULL;
			// Drop out the worker.
			} else {
				worker->ClearAction();
				worker->SubAction = 0;//may be 40
				// HACK: make sure the sight is updated correctly
				unit->CurrentSightRange = 1;
				DropOutOnSide(worker, LookingW, type->TileWidth, type->TileHeight);
				
				CUnit *goal = worker->Orders[0]->Goal;
				if(goal) {
					goal->RefsDecrease();
					worker->Orders[0]->Goal = NULL;				
				}
				
				//
				// If we can harvest from the new building, do it.
				//
				if (worker->Type->ResInfo[type->GivesResource]) {
					CommandResource(worker, unit, 0);
				}
			}
		}

		unit->Player->Notify(NotifyGreen, unit->X, unit->Y,
			_("New %s done"), type->Name.c_str());
		if (unit->Player == ThisPlayer) {
			if (unit->Type->Sound.Ready.Sound) {
				PlayUnitSound(unit, VoiceReady);
			} else if (worker) {
				PlayUnitSound(worker, VoiceWorkCompleted);
			} else {
				PlayUnitSound(unit, VoiceBuilding);
			}
		}
		if (unit->Player->AiEnabled) {
			AiWorkComplete(worker, unit);
		}

		// FIXME: Vladi: this is just a hack to test wall fixing,
		// FIXME:  also not sure if the right place...
		// FIXME: Johns: hardcoded unit-type wall / more races!
		if (unit->Type == UnitTypeOrcWall ||
				unit->Type == UnitTypeHumanWall) {
			Map.SetWall(unit->X, unit->Y, unit->Type == UnitTypeHumanWall);
			unit->Remove(NULL);
			UnitLost(unit);
			UnitClearOrders(unit);
			unit->Release();
			return;
		}

		UpdateForNewUnit(unit, 0);

		// Set the direction of the building if it supports them
		if (unit->Type->NumDirections > 1) {
			unit->Direction = (MyRand() >> 8) & 0xFF; // random heading
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
