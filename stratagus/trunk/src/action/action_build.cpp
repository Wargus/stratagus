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
/**@name action_build.c - The build building action. */
//
//      (c) Copyright 1998,2000-2004 by Lutz Sammer, Jimmy Salmon
//          Russell Smith
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
static void UpdateConstructionFrame(Unit* unit)
{
	ConstructionFrame* cframe;
	ConstructionFrame* tmp;
	int percent;

	percent = unit->Data.Built.Progress /
		(unit->Type->Stats[unit->Player->Player].Costs[TimeCost] * 6);
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
*/
static void MoveToLocation(Unit* unit)
{
	// First entry
	if (!unit->SubAction) {
		unit->SubAction = 1;
		NewResetPath(unit);
	}

	if (unit->Wait) {
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

			NotifyPlayer(unit->Player, NotifyYellow, unit->X, unit->Y,
				"You cannot reach building place");
			if (unit->Player->AiEnabled) {
				AiCanNotReach(unit, unit->Orders[0].Type);
			}

			unit->Orders[0].Action = UnitActionStill;
			unit->SubAction = 0;
			if (unit->Selected) { // update display for new action
				SelectedUnitChanged();
			}
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
**  Check if the unit can build
*/
static Unit* CheckCanBuild(Unit* unit)
{
	int x;
	int y;
	UnitType* type;
	Unit* ontop;

	if (unit->Wait) {
		unit->Wait--;
		return NULL;
	}

	x = unit->Orders[0].X;
	y = unit->Orders[0].Y;
	type = unit->Orders[0].Type;

	//
	// Check if the building could be built there.
	// if on NULL, really attempt to build here
	//
	if ((ontop = CanBuildUnitType(unit, type, x, y, 1)) == NULL) {
		//
		// Some tries to build the building.
		//
		if (unit->SubAction++ < 30) {
			// To keep the load low, retry each 10 cycles
			// NOTE: we can already inform the AI about this problem?
			unit->Wait = 10;
			return NULL;
		}

		NotifyPlayer(unit->Player, NotifyYellow, unit->X, unit->Y,
			"You cannot build %s here",type->Name);
		if (unit->Player->AiEnabled) {
			AiCanNotBuild(unit, type);
		}

		unit->Orders[0].Action = UnitActionStill;
		unit->SubAction = 0;
		if (unit->Selected) { // update display for new action
			SelectedUnitChanged();
		}

		return NULL;
	}

	//
	// FIXME: got bug report about unit->Type==NULL in building
	//
	Assert(unit->Type && unit->HP);
	if (!unit->Type || !unit->HP) {
		return NULL;
	}

	//
	// Check if enough resources for the building.
	//
	if (PlayerCheckUnitType(unit->Player, type)) {
		// FIXME: Better tell what is missing?
		NotifyPlayer(unit->Player, NotifyYellow, unit->X, unit->Y,
			"Not enough resources to build %s", type->Name);
		if (unit->Player->AiEnabled) {
			AiCanNotBuild(unit, type);
		}

		unit->Orders[0].Action = UnitActionStill;
		unit->SubAction = 0;
		if (unit->Selected) { // update display for new action
			SelectedUnitChanged();
		}
		return NULL;
	}

	//
	// Check if hiting any limits for the building.
	//
	if (PlayerCheckLimits(unit->Player, type) < 0) {
		NotifyPlayer(unit->Player, NotifyYellow, unit->X, unit->Y,
			"Can't build more units %s", type->Name);
		if (unit->Player->AiEnabled) {
			AiCanNotBuild(unit, type);
		}

		unit->Orders[0].Action = UnitActionStill;
		unit->SubAction = 0;
		if (unit->Selected) { // update display for new action
			SelectedUnitChanged();
		}
		return NULL;
	}

	return ontop;
}

/**
**  Start building
*/
static void StartBuilding(Unit* unit, Unit* ontop)
{
	int x;
	int y;
	UnitType* type;
	Unit* build;
	BuildRestriction* b;
	const UnitStats* stats;

	x = unit->Orders[0].X;
	y = unit->Orders[0].Y;
	type = unit->Orders[0].Type;

	PlayerSubUnitType(unit->Player, type);

	build = MakeUnit(type, unit->Player);
	build->Constructed = 1;
	build->CurrentSightRange = 0;

	// Building on top of something, may remove what is beneath it
	if (ontop != unit) {
		b = OnTopDetails(build, ontop->Type);
		Assert(b);
		if (b->Data.OnTop.ReplaceOnBuild) {
			build->ResourcesHeld = ontop->ResourcesHeld; // We capture the value of what is beneath.
			RemoveUnit(ontop, NULL); // Destroy building beneath
			UnitLost(ontop);
			UnitClearOrders(ontop);
			ReleaseUnit(ontop);
		}
	}

	// Must set action before placing, otherwise it will incorrectly mark radar
	build->Orders[0].Action = UnitActionBuilt;
	
	// Must place after previous for map flags
	PlaceUnit(build, x, y);
	if (!type->BuilderOutside) {
		build->CurrentSightRange = 1;
	}

	// HACK: the building is not ready yet
	build->Player->UnitTypesCount[type->Slot]--;

	stats = build->Stats;

	build->Wait = 1;
	// Make sure the bulding doesn't cancel itself out right away.
	build->Data.Built.Progress = 100;
	build->HP = 1;
	UpdateConstructionFrame(build);

	// We need somebody to work on it.
	build->HP = 1;
	if (!type->BuilderOutside) {
		// Place the builder inside the building
		build->Data.Built.Worker = unit;
		RemoveUnit(unit, build);
		build->CurrentSightRange = 0;
		unit->X = x;
		unit->Y = y;
		unit->Orders[0].Action = UnitActionStill;
		unit->Orders[0].Goal = NULL;
		unit->SubAction = 0;
	} else {
		unit->Orders[0].Goal = build;
		unit->Orders[0].X = unit->Orders[0].Y = -1;
		// FIXME: Should have a BuildRange?
		unit->Orders[0].Range = unit->Type->RepairRange;
		unit->SubAction = 40;
		unit->Wait = 1;
		RefsIncrease(build);
		// Mark the new building seen.
		MapMarkUnitSight(build);
	}
	UpdateConstructionFrame(build);
}

/**
**  Animate unit build
**
**  @param unit Unit, for that the build animation is played.
*/
static int AnimateActionBuild(Unit* unit)
{
	if (unit->Type->Animations) {
		int flags;

		Assert(unit->Type->Animations->Repair);
		flags = UnitShowAnimation(unit, unit->Type->Animations->Repair);
		if ((flags & AnimationSound)) {
			PlayUnitSound(unit, VoiceRepairing);
		}
	} else if (unit->Type->NewAnimations) {
		UnitShowNewAnimation(unit, unit->Type->NewAnimations->Build);
	}

	return 0;
}

/**
**  Build the building
*/
static void BuildBuilding(Unit* unit)
{
	Unit* goal;
	int hp;
	int animlength;
	Animation* anim;

	AnimateActionBuild(unit);
	if ((!unit->Type->NewAnimations && unit->Reset) ||
			(unit->Type->NewAnimations && !unit->Anim.Unbreakable)) {
		goal = unit->Orders[0].Goal;

		// hp is the current damage taken by the unit.
		hp = (goal->Data.Built.Progress * goal->Stats->HitPoints) /
			(goal->Type->Stats[goal->Player->Player].Costs[TimeCost] * 600) - goal->HP;
		//
		// Calculate the length of the attack (repair) anim.
		//
		animlength = 0;
		for (anim = unit->Type->Animations->Repair; !(anim->Flags & AnimationReset); ++anim) {
			animlength += anim->Sleep;
		}

		// FIXME: implement this below:
		// unit->Data.Built.Worker->Type->BuilderSpeedFactor;
		goal->Data.Built.Progress += 100 * animlength * SpeedBuild;
		// Keep the same level of damage while increasing HP.
		goal->HP = (goal->Data.Built.Progress * goal->Stats->HitPoints) /
			(goal->Type->Stats[goal->Player->Player].Costs[TimeCost] * 600) - hp;
		if (goal->HP > goal->Stats->HitPoints) {
			goal->HP = goal->Stats->HitPoints;
		}

		//
		// Building is gone or finished
		//
		if (!goal || goal->HP >= goal->Stats->HitPoints) {
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
	}
}

/**
**  Unit builds a building.
**
**  @param unit  Unit that builds a building.
*/
void HandleActionBuild(Unit* unit)
{
	Unit* ontop;

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
**  @param unit  Unit that is built.
*/
void HandleActionBuilt(Unit* unit)
{
	Unit* worker;
	UnitType* type;
	int n;
	int progress;

	type = unit->Type;

	// n is the current damage taken by the unit.
	n = (unit->Data.Built.Progress * unit->Stats->HitPoints) /
		(type->Stats[unit->Player->Player].Costs[TimeCost] * 600) - unit->HP;
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
	// Keep the same level of damage while increasing HP.
	unit->HP = (unit->Data.Built.Progress * unit->Stats->HitPoints) /
		(type->Stats[unit->Player->Player].Costs[TimeCost] * 600) - n;
	if (unit->HP > unit->Stats->HitPoints) {
		unit->HP = unit->Stats->HitPoints;
	}

	//
	// Check if construction should be canceled...
	//
	if (unit->Data.Built.Cancel || unit->Data.Built.Progress < 0) {
		DebugPrint("%s canceled.\n" _C_ unit->Type->Name);
		// Drop out unit
		if ((worker = unit->Data.Built.Worker)) {
			worker->Orders[0].Action = UnitActionStill;
			unit->Data.Built.Worker = NoUnitP;
			worker->Reset = worker->Wait = 1;
			worker->SubAction = 0;
			DropOutOnSide(worker, LookingW, type->TileWidth, type->TileHeight);
		}

		// Player gets back 75% of the original cost for a building.
		PlayerAddCostsFactor(unit->Player, unit->Stats->Costs,
			CancelBuildingCostsFactor);
		// Cancel building
		LetUnitDie(unit);
		return;
	}

	//
	// Check if building ready. Note we can both build and repair.
	//
	if (unit->Data.Built.Progress >= unit->Stats->Costs[TimeCost] * 600 ||
			unit->HP >= unit->Stats->HitPoints) {
		DebugPrint("Building ready.\n");
		if (unit->HP > unit->Stats->HitPoints) {
			unit->HP = unit->Stats->HitPoints;
		}
		unit->Orders[0].Action = UnitActionStill;
		// HACK: the building is ready now
		unit->Player->UnitTypesCount[type->Slot]++;
		unit->Constructed = 0;
		if (unit->Frame < 0) {
			unit->Frame = -1;
		} else {
			unit->Frame = 0;
		}
		unit->Reset = unit->Wait = 1;

		if ((worker = unit->Data.Built.Worker)) {
			// Bye bye worker.
			if (type->BuilderLost) {
				// FIXME: enough?
				LetUnitDie(worker);
			// Drop out the worker.
			} else {
				worker->Orders[0].Action = UnitActionStill;
				worker->SubAction = 0;
				worker->Reset = worker->Wait = 1;
				DropOutOnSide(worker, LookingW, type->TileWidth, type->TileHeight);
				//
				// If we can harvest from the new building, do it.
				//
				if (worker->Type->ResInfo[type->GivesResource]) {
					CommandResource(worker, unit, 0);
				}
			}
		}

		if (type->GivesResource) {
			// Set to Zero as it's part of a union
			unit->Data.Resource.Active = 0;
			// Has StartingResources, Use those
			if (type->StartingResources) {
				unit->ResourcesHeld = type->StartingResources;
			}
		}

		NotifyPlayer(unit->Player, NotifyGreen, unit->X, unit->Y,
			"New %s done", type->Name);
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
			MapSetWall(unit->X, unit->Y, unit->Type == UnitTypeHumanWall);
			RemoveUnit(unit, NULL);
			UnitLost(unit);
			UnitClearOrders(unit);
			ReleaseUnit(unit);
			return;
		}

		UpdateForNewUnit(unit, 0);

		if (IsOnlySelected(unit)) {
			SelectedUnitChanged();
		} else if (unit->Player == ThisPlayer) {
			SelectedUnitChanged();
		}
		unit->CurrentSightRange = unit->Stats->SightRange;
		MapMarkUnitSight(unit);
		return;
	}

	UpdateConstructionFrame(unit);

	unit->Wait = 1;
}

//@}
