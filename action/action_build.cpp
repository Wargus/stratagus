//       _________ __                 __
//      /   _____//  |_____________ _/  |______     ____  __ __  ______
//      \_____  \\   __\_  __ \__  \\   __\__  \   / ___\|  |  \/  ___/
//      /        \|  |  |  | \// __ \|  |  / __ \_/ /_/  >  |  /\___ |
//     /_______  /|__|  |__|  (____  /__| (____  /\___  /|____//____  >
//             \/                  \/          \//_____/            \/
//  ______________________                           ______________________
//                        T H E   W A R   B E G I N S
//        Stratagus - A free fantasy real time strategy game engine
//
/**@name action_build.c - The build building action. */
//
//      (c) Copyright 1998,2000-2004 by Lutz Sammer, Jimmy Salmon
//          Russell Smith
//
//      This program is free software; you can redistribute it and/or modify
//      it under the terms of the GNU General Public License as published by
//      the Free Software Foundation; version 2 dated June, 1991.
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

/*----------------------------------------------------------------------------
--  Functions
----------------------------------------------------------------------------*/

/**
**  Update construction frame
**
**  @param unit  The building under construction.
*/
local void UpdateConstructionFrame(Unit* unit)
{
	ConstructionFrame* cframe;
	ConstructionFrame* tmp;
	int percent;

	percent = unit->Data.Builded.Progress /
		(unit->Type->Stats[unit->Player->Player].Costs[TimeCost] * 6);
	cframe = tmp = unit->Type->Construction->Frames;
	while (tmp) {
		if (percent < tmp->Percent) {
			break;
		}
		cframe = tmp;
		tmp = tmp->Next;
	}
	if (cframe != unit->Data.Builded.Frame) {
		unit->Data.Builded.Frame = cframe;
		if (unit->Frame < 0) {
			unit->Frame = -cframe->Frame - 1;
		} else {
			unit->Frame = cframe->Frame;
		}
		CheckUnitToBeDrawn(unit);
	}
}

/**
**  Unit builds a building.
**
**  @param unit  Unit that builds a building.
*/
global void HandleActionBuild(Unit* unit)
{
	int x;
	int y;
	UnitType* type;
	const UnitStats* stats;
	Unit* build;

	if (!unit->SubAction) {				// first entry
		unit->SubAction = 1;
		NewResetPath(unit);
	}

	type = unit->Orders[0].Type;

	switch (DoActionMove(unit)) {		// reached end-point?
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
				AiCanNotReach(unit,type);
			}

			unit->Orders[0].Action = UnitActionStill;
			unit->SubAction = 0;
			if (unit->Selected) {		// update display for new action
				SelectedUnitChanged();
			}
			return;

		case PF_REACHED:
			DebugLevel3Fn("reached %d,%d\n" _C_ unit->X _C_ unit->Y);
			break;

		default:
			return;
	}

	x = unit->Orders[0].X;
	y = unit->Orders[0].Y;

	//
	// Check if the building could be build there.
	//
	if (!CanBuildUnitType(unit, type, x, y)) {
		//
		// Some tries to build the building.
		//
		if (unit->SubAction++ < 10) {
			// To keep the load low, retry each 10 cycles
			// NOTE: we can already inform the AI about this problem?
			unit->Wait = 10;
			return;
		}

		NotifyPlayer(unit->Player, NotifyYellow, unit->X, unit->Y,
			"You cannot build %s here",type->Name);
		if (unit->Player->AiEnabled) {
			AiCanNotBuild(unit, type);
		}

		unit->Orders[0].Action = UnitActionStill;
		unit->SubAction = 0;
		if (unit->Selected) {		// update display for new action
			SelectedUnitChanged();
		}

		return;
	}

	//
	// FIXME: got bug report about unit->Type==NULL in building
	//
	DebugCheck(!unit->Type || !unit->HP);

	if (!unit->Type || !unit->HP) {
		return;
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
		if (unit->Selected) {		// update display for new action
			SelectedUnitChanged();
		}
		return;
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
		if (unit->Selected) {		// update display for new action
			SelectedUnitChanged();
		}
		return;
	}
	PlayerSubUnitType(unit->Player, type);

	build = MakeUnit(type, unit->Player);
	build->Constructed = 1;
	build->CurrentSightRange = 0;
	PlaceUnit(build, x, y);
	if (!type->BuilderOutside) {
		build->CurrentSightRange = 1;
	}

	// Building on top of something, must remove what is beneath it
	if (type->MustBuildOnTop) {
		Unit* temp;
		if ((temp = UnitTypeOnMap(x, y, type->MustBuildOnTop))) {
			build->Value = temp->Value;	// We capture the value of what is beneath.
			RemoveUnit(temp, NULL);		// Destroy building beneath
			UnitLost(temp);
			UnitClearOrders(temp);
			ReleaseUnit(temp);
		} else {
			DebugCheck(1);
		}
	}

	// HACK: the building is not ready yet
	build->Player->UnitTypesCount[type->Slot]--;

	stats = build->Stats;

	build->Wait = 1;
	// Make sure the bulding doesn't cancel itself out right away.
	build->Data.Builded.Progress = 100;
	build->Orders[0].Action = UnitActionBuilded;
	build->HP = 1;
	UpdateConstructionFrame(build);

	// We need somebody to work on it.
	build->HP = 1;
	if (!type->BuilderOutside) {
		// Place the builder inside the building
		build->Data.Builded.Worker = unit;
		RemoveUnit(unit, build);
		build->CurrentSightRange = 0;
		unit->X = x;
		unit->Y = y;
		unit->Orders[0].Action = UnitActionStill;
		unit->Orders[0].Goal = NULL;
		unit->SubAction = 0;
	} else {
		// Make the builder repair the newly spawned building.
		unit->Orders[0].Action = UnitActionRepair;
		unit->Orders[0].Goal = build;
		unit->Orders[0].X = unit->Orders[0].Y = -1;
		unit->Orders[0].Range = unit->Type->RepairRange;
		unit->SubAction = 0;
		unit->Wait = 1;
		RefsIncrease(build);
		// Mark the new building seen.
		MapMarkUnitSight(build);
	}
	UpdateConstructionFrame(build);
}

/**
**  Unit under Construction
**
**  @param unit  Unit that is builded.
*/
global void HandleActionBuilded(Unit* unit)
{
	Unit* worker;
	UnitType* type;
	int n;
	int progress;

	type = unit->Type;

	// n is the current damage taken by the unit.
	n = (unit->Data.Builded.Progress * unit->Stats->HitPoints) /
		(type->Stats[unit->Player->Player].Costs[TimeCost] * 600) - unit->HP;
	// This below is most often 0
	if (type->BuilderOutside) {
		progress = unit->Type->AutoBuildRate;
	} else {
		progress = 100;
		// FIXME: implement this below:
		// unit->Data.Builded.Worker->Type->BuilderSpeedFactor;
	}
	// Building speeds increase or decrease.
	progress *= SpeedBuild;
	unit->Data.Builded.Progress += progress;
	// Keep the same level of damage while increasing HP.
	unit->HP = (unit->Data.Builded.Progress * unit->Stats->HitPoints) /
		(type->Stats[unit->Player->Player].Costs[TimeCost] * 600) - n;
	if (unit->HP > unit->Stats->HitPoints) {
		unit->HP = unit->Stats->HitPoints;
	}

	//
	// Check if construction should be canceled...
	//
	if (unit->Data.Builded.Cancel || unit->Data.Builded.Progress < 0) {
		DebugLevel0Fn("%s canceled.\n" _C_ unit->Type->Name);
		// Drop out unit
		if ((worker = unit->Data.Builded.Worker)) {
			worker->Orders[0].Action = UnitActionStill;
			unit->Data.Builded.Worker = NoUnitP;
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
	if (unit->Data.Builded.Progress >= unit->Stats->Costs[TimeCost] * 600 ||
			unit->HP >= unit->Stats->HitPoints) {
		DebugLevel0Fn("Building ready.\n");
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

		if ((worker = unit->Data.Builded.Worker)) {
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
			MustRedraw |= RedrawInfoPanel;
		} else if (unit->Player == ThisPlayer) {
			SelectedUnitChanged();
		}
		unit->CurrentSightRange = unit->Stats->SightRange;
		MapMarkUnitSight(unit);
		CheckUnitToBeDrawn(unit);
		return;
	}

	UpdateConstructionFrame(unit);

	unit->Wait = 1;
	if (IsOnlySelected(unit)) {
		MustRedraw |= RedrawInfoPanel;
	}
}

//@}
