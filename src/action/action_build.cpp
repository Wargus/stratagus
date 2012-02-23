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
#include "pathfinder.h"
#include "construct.h"
#include "iolib.h"
#include "script.h"

extern void AiReduceMadeInBuilt(PlayerAi &pai, const CUnitType &type);


enum
{
	State_Start = 0,
	State_MoveToLocationMax = 10, // Range from prev
	State_NearOfLocation = 11, // Range to next
	State_StartBuilding_Failed = 20,
	State_BuildFromInside = 21,
	State_BuildFromOutside = 22
};



/*----------------------------------------------------------------------------
--  Functions
----------------------------------------------------------------------------*/


/* virtual */ void COrder_Build::Save(CFile &file, const CUnit &unit) const
{
	file.printf("{\"action-build\",");
	file.printf(" \"range\", %d,", this->Range);
#if 1 // Fixed with Type
	file.printf(" \"width\", %d,", this->Width);
	file.printf(" \"height\", %d,", this->Height);
	file.printf(" \"min-range\", %d,", this->MinRange);
#endif
	file.printf(" \"tile\", {%d, %d},", this->goalPos.x, this->goalPos.y);

	if (this->BuildingUnit != NULL) {
		if (this->BuildingUnit->Destroyed) {
			/* this unit is destroyed so it's not in the global unit
			 * array - this means it won't be saved!!! */
			printf ("FIXME: storing destroyed Goal - loading will fail.\n");
		}
		file.printf(" \"building\", \"%s\",", UnitReference(this->BuildingUnit).c_str());
	}
	file.printf(" \"type\", \"%s\",", this->Type->Ident.c_str());
	file.printf(" \"state\", %d,\n  ", this->State);
	SaveDataMove(file);
	file.printf("}");
}

/* virtual */ bool COrder_Build::ParseSpecificData(lua_State *l, int &j, const char *value, const CUnit &unit)
{
	if (ParseMoveData(l, j, value)) {
		return true;
	} else if (!strcmp(value, "building")) {
		++j;
		lua_rawgeti(l, -1, j + 1);
		this->BuildingUnit = CclGetUnitFromRef(l);
		lua_pop(l, 1);
	} else if (!strcmp(value, "state")) {
		++j;
		lua_rawgeti(l, -1, j + 1);
		this->State = LuaToNumber(l, -1);
		lua_pop(l, 1);
	} else if (!strcmp(value, "type")) {
		++j;
		lua_rawgeti(l, -1, j + 1);
		this->Type = UnitTypeByIdent(LuaToString(l, -1));
		lua_pop(l, 1);
	} else {
		return false;
	}
	return true;
}

/** Called when unit is killed.
**  warn the AI module.
*/
void COrder_Build::AiUnitKilled(CUnit& unit)
{
	DebugPrint("%d: %d(%s) killed, with order %s!\n" _C_
		unit.Player->Index _C_ UnitNumber(unit) _C_
		unit.Type->Ident.c_str() _C_ this->Type->Ident.c_str());
	if (this->BuildingUnit == NULL) {
		AiReduceMadeInBuilt(*unit.Player->Ai, *this->Type);
	}
}



/**
**  Move to build location
**
**  @param unit  Unit to move
*/
bool COrder_Build::MoveToLocation(CUnit &unit)
{
	// First entry
	if (this->State == 0) {
		this->Data.Move.Cycles = 0; //moving counter
		this->State = 1;
		this->NewResetPath();
	}
	switch (DoActionMove(unit)) { // reached end-point?
		case PF_UNREACHABLE: {
			// Some tries to reach the goal
			if (this->State++ < 10) {
				// To keep the load low, retry each 1/4 second.
				// NOTE: we can already inform the AI about this problem?
				unit.Wait = CYCLES_PER_SECOND / 4;
				return false;
			}

			unit.Player->Notify(NotifyYellow, unit.tilePos.x, unit.tilePos.y,
				_("You cannot reach building place"));
			if (unit.Player->AiEnabled) {
				AiCanNotReach(unit, this->GetUnitType());
			}
			return true;
		}
		case PF_REACHED:
			this->State = State_NearOfLocation;
			return false;

		default:
			// Moving...
			return false;
	}
}

static bool CheckLimit(const CUnit &unit, const CUnitType &type)
{
	const CPlayer &player = *unit.Player;
	bool isOk = true;

	// Check if enough resources for the building.
	if (player.CheckUnitType(type)) {
		// FIXME: Better tell what is missing?
		player.Notify(NotifyYellow, unit.tilePos.x, unit.tilePos.y,
			_("Not enough resources to build %s"), type.Name.c_str());
		isOk = false;
	}

	// Check if hiting any limits for the building.
	if (player.CheckLimits(type) < 0) {
		player.Notify(NotifyYellow, unit.tilePos.x, unit.tilePos.y,
			_("Can't build more units %s"), type.Name.c_str());
		isOk = false;
	}
	if (isOk == false && player.AiEnabled) {
		AiCanNotBuild(unit, type);
	}
	return isOk;
}


class AlreadyBuildingFinder {
public:
	AlreadyBuildingFinder(const CUnit &unit, const CUnitType &t) :
		worker(&unit), type(&t) {}
	bool operator() (const CUnit *const unit) const
	{
		return (!unit->Destroyed && unit->Type == type
				&& (worker->Player == unit->Player || worker->IsAllied(*unit)));
	}
	CUnit *Find(const CMapField *const mf) const
	{
		return mf->UnitCache.find(*this);
	}
private:
	const CUnit *worker;
	const CUnitType *type;
};

/**
**  Check if the unit can build
**
**  @param unit  Unit to check
*/
CUnit *COrder_Build::CheckCanBuild(CUnit &unit)
{
	const Vec2i pos = this->goalPos;
	const CUnitType &type = this->GetUnitType();

	// Check if the building could be built there.

	CUnit *ontop = CanBuildUnitType(&unit, type, pos, 1);

	if (ontop != NULL) {
		return ontop;
	}
#if 0
	/*
	 * FIXME: rb - CheckAlreadyBuilding should be somehow
	 * enabled/disable via game lua scripting
	 */
	CUnit *building = AlreadyBuildingFinder(unit, type).Find(Map.Field(pos));
	if (building != NULL) {
		if (unit.CurrentOrder() == this) {
			DebugPrint("%d: Worker [%d] is helping build: %s [%d]\n"
					_C_ unit.Player->Index _C_ unit.Slot
					_C_ building->Type->Name.c_str()
					_C_ building->Slot);

			delete this; // Bad
			unit.Orders[0] = COrder::NewActionRepair(unit, *building);
			return NULL;
		}
	}
#endif
	// Some tries to build the building.
	this->State++;
	// To keep the load low, retry each 10 cycles
	// NOTE: we can already inform the AI about this problem?
	unit.Wait = 10;
	return NULL;
}


bool COrder_Build::StartBuilding(CUnit &unit, CUnit &ontop)
{
	const CUnitType &type = this->GetUnitType();

	unit.Player->SubUnitType(type);

	CUnit *build = MakeUnit(const_cast<CUnitType &>(type), unit.Player);

	// If unable to make unit, stop, and report message
	if (build == NoUnitP) {
		// FIXME: Should we retry this?
		unit.Player->Notify(NotifyYellow, unit.tilePos.x, unit.tilePos.y,
			_("Unable to create building %s"), type.Name.c_str());
		if (unit.Player->AiEnabled) {
			AiCanNotBuild(unit, type);
		}
		return false;
	}
	build->Constructed = 1;
	build->CurrentSightRange = 0;

	// Building on top of something, may remove what is beneath it
	if (&ontop != &unit) {
		CBuildRestrictionOnTop *b;

		b = static_cast<CBuildRestrictionOnTop *> (OnTopDetails(*build, ontop.Type));
		Assert(b);
		if (b->ReplaceOnBuild) {
			build->ResourcesHeld = ontop.ResourcesHeld; // We capture the value of what is beneath.
			ontop.Remove(NULL); // Destroy building beneath
			UnitLost(ontop);
			UnitClearOrders(ontop);
			ontop.Release();
		}
	}

	// Must set action before placing, otherwise it will incorrectly mark radar
	delete build->CurrentOrder();
	build->Orders[0] = COrder::NewActionBuilt(unit, *build);

	UpdateUnitSightRange(*build);
	// Must place after previous for map flags
	build->Place(this->goalPos);

	// HACK: the building is not ready yet
	build->Player->UnitTypesCount[type.Slot]--;

	// We need somebody to work on it.
	if (!type.BuilderOutside) {
		UnitShowAnimation(unit, unit.Type->Animations->Still);
		unit.Remove(build);
		this->State = State_BuildFromInside;
		if (unit.Selected) {
			SelectedUnitChanged();
		}
	} else {
		this->State = State_BuildFromOutside;
		this->BuildingUnit = build;
		unit.Direction = DirectionToHeading(build->tilePos - unit.tilePos);
		UnitUpdateHeading(unit);
	}
	return true;
}

static void AnimateActionBuild(CUnit &unit)
{
	CAnimations *animations = unit.Type->Animations;

	if (animations == NULL) {
		return ;
	}
	if (animations->Build) {
		UnitShowAnimation(unit, animations->Build);
	} else if (animations->Repair) {
		UnitShowAnimation(unit, animations->Repair);
	}
}


/**
**  Build the building
**
**  @param unit  worker which build.
*/
bool COrder_Build::BuildFromOutside(CUnit &unit) const
{
	AnimateActionBuild(unit);

	if (this->BuildingUnit == NULL) {
		return false;
	}

	if (this->BuildingUnit->CurrentAction() == UnitActionBuilt) {
		COrder_Built &targetOrder = *static_cast<COrder_Built *>(this->BuildingUnit->CurrentOrder());
		CUnit& goal = *const_cast<COrder_Build*>(this)->BuildingUnit;


		targetOrder.ProgressHp(goal, 100);
	}
	if (unit.Anim.Unbreakable) {
		return false;
	}
	return this->BuildingUnit->CurrentAction() != UnitActionBuilt;
}


/* virtual */ bool COrder_Build::Execute(CUnit &unit)
{
	if (unit.Wait) {
		unit.Wait--;
		return false;
	}
	if (this->State <= State_MoveToLocationMax) {
		if (this->MoveToLocation(unit)) {
			return true;
		}
	}
	const CUnitType &type = this->GetUnitType();

	if (State_NearOfLocation <= this->State && this->State < State_StartBuilding_Failed) {
		if (CheckLimit(unit, type) == false) {
			return true;
		}
		CUnit *ontop = this->CheckCanBuild(unit);

		if (ontop != NULL) {
			this->StartBuilding(unit, *ontop);
		}
	}
	if (this->State == State_StartBuilding_Failed) {
		unit.Player->Notify(NotifyYellow, unit.tilePos.x, unit.tilePos.y,
			_("You cannot build %s here"), type.Name.c_str());
		if (unit.Player->AiEnabled) {
			AiCanNotBuild(unit, type);
		}
		return true;
	}
	if (this->State == State_BuildFromOutside) {
		this->BuildFromOutside(unit);
	}

	return false;
}

/**
**  Unit builds a building.
**
**  @param unit  Unit that builds a building
*/
void HandleActionBuild(COrder& order, CUnit &unit)
{
	Assert(order.Action == UnitActionBuild);

	if (order.Execute(unit)) {
		unit.ClearAction();
	}
}




///////////////////////////
// Action_built
//////////////////////////

/* virtual */ void COrder_Built::Save(CFile &file, const CUnit &unit) const
{
	file.printf("{\"action-built\", ");

	CConstructionFrame *cframe = unit.Type->Construction->Frames;
	int frame = 0;
	while (cframe != this->Frame) {
		cframe = cframe->Next;
		++frame;
	}
	if (this->Worker == NULL) {
		file.printf("\"worker\", \"%s\", ", UnitReference(this->Worker).c_str());
	}
	file.printf("\"progress\", %d, \"frame\", %d", this->ProgressCounter, frame);
	if (this->IsCancelled) {
		file.printf(", \"cancel\"");
	}
	file.printf("}");
}

/* virtual */ bool COrder_Built::ParseSpecificData(lua_State *l, int &j, const char *value, const CUnit &unit)
{
	if (!strcmp(value, "worker")) {
		++j;
		lua_rawgeti(l, -1, j + 1);
		this->Worker = CclGetUnitFromRef(l);
		lua_pop(l, 1);
	} else if (!strcmp(value, "progress")) {
		++j;
		lua_rawgeti(l, -1, j + 1);
		this->ProgressCounter = LuaToNumber(l, -1);
		lua_pop(l, 1);
	} else if (!strcmp(value, "cancel")) {
		this->IsCancelled = true;
	} else if (!strcmp(value, "frame")) {
		++j;
		lua_rawgeti(l, -1, j + 1);
		int frame = LuaToNumber(l, -1);
		lua_pop(l, 1);
		CConstructionFrame *cframe = unit.Type->Construction->Frames;
		while (frame--) {
			cframe = cframe->Next;
		}
		this->Frame = cframe;
	} else {
		return false;
	}
	return true;
}


static void CancelBuilt(COrder_Built &order, CUnit &unit)
{
	Assert(unit.CurrentOrder() == &order);
	CUnit *worker = order.GetWorkerPtr();

	// Drop out unit
	if (worker != NULL) {
		worker->ClearAction();

		// HACK: make sure the sight is updated correctly
//		unit.CurrentSightRange = 1;
		DropOutOnSide(*worker, LookingW, &unit);
//		unit.CurrentSightRange = 0;
	}
	// Player gets back 75% of the original cost for a building.
	unit.Player->AddCostsFactor(unit.Stats->Costs, CancelBuildingCostsFactor);
	// Cancel building
	LetUnitDie(unit);
}

static bool Finish(COrder_Built &order, CUnit& unit)
{
	const CUnitType &type = *unit.Type;
	CPlayer &player = *unit.Player;

	DebugPrint("%d: Building %s(%s) ready.\n" _C_ player.Index _C_ type.Ident.c_str() _C_ type.Name.c_str() );

	// HACK: the building is ready now
	player.UnitTypesCount[type.Slot]++;
	unit.Constructed = 0;
	if (unit.Frame < 0) {
		unit.Frame = -1;
	} else {
		unit.Frame = 0;
	}
	CUnit *worker = order.GetWorkerPtr();

	if (worker != NULL) {
		if (type.BuilderLost) {
			// Bye bye worker.
			LetUnitDie(*worker);
			worker = NULL;
		} else { // Drop out the worker.
			worker->ClearAction();

			DropOutOnSide(*worker, LookingW, &unit);

			// If we can harvest from the new building, do it.
			if (worker->Type->ResInfo[type.GivesResource]) {
				CommandResource(*worker, unit, 0);
			}
		}
	}

	if (type.GivesResource && type.StartingResources != 0) {
		// Has StartingResources, Use those
		unit.ResourcesHeld = type.StartingResources;
	}

	player.Notify(NotifyGreen, unit.tilePos.x, unit.tilePos.y, _("New %s done"), type.Name.c_str());
	if (&player == ThisPlayer) {
		if (type.Sound.Ready.Sound) {
			PlayUnitSound(unit, VoiceReady);
		} else if (worker) {
			PlayUnitSound(*worker, VoiceWorkCompleted);
		} else {
			PlayUnitSound(unit, VoiceBuilding);
		}
	}

	if (player.AiEnabled) {
		/* Worker can be NULL */
		AiWorkComplete(worker, unit);
	}

	// FIXME: Vladi: this is just a hack to test wall fixing,
	// FIXME:  also not sure if the right place...
	// FIXME: Johns: hardcoded unit-type wall / more races!
	if (&type == UnitTypeOrcWall || &type == UnitTypeHumanWall) {
		Map.SetWall(unit.tilePos, &type == UnitTypeHumanWall);
		unit.Remove(NULL);
		UnitLost(unit);
		UnitClearOrders(unit);
		unit.Release();
		return false;
	}

	UpdateForNewUnit(unit, 0);

	// Set the direction of the building if it supports them
	if (type.NumDirections > 1) {
		if (type.Wall) { // Special logic for walls
			CorrectWallDirections(unit);
			CorrectWallNeighBours(unit);
		} else {
			unit.Direction = (MyRand() >> 8) & 0xFF; // random heading
		}
		UnitUpdateHeading(unit);
	}

	if (IsOnlySelected(unit) || &player == ThisPlayer) {
		SelectedUnitChanged();
	}
	MapUnmarkUnitSight(unit);
	unit.CurrentSightRange = unit.Stats->Variables[SIGHTRANGE_INDEX].Max;
	MapMarkUnitSight(unit);
	return true;
}


/* virtual */ bool COrder_Built::Execute(CUnit &unit)
{
	const CUnitType &type = *unit.Type;

	int amount;
	if (type.BuilderOutside) {
		amount = type.AutoBuildRate;
	} else {
		// FIXME: implement this below:
		// this->Data.Worker->Type->BuilderSpeedFactor;
		amount = 100;
	}
	this->Progress(unit, amount);

	// Check if construction should be canceled...
	if (this->IsCancelled || this->ProgressCounter < 0) {
		DebugPrint("%d: %s canceled.\n" _C_ unit.Player->Index _C_ unit.Type->Name.c_str());

		CancelBuilt(*this, unit);
		return false;
	}

	const int maxProgress = type.Stats[unit.Player->Index].Costs[TimeCost] * 600;

	// Check if building ready. Note we can both build and repair.
	if (!unit.Anim.Unbreakable && this->ProgressCounter >= maxProgress) {
		return Finish(*this, unit);
	}
	return false;
}

/* virtual */ void COrder_Built::Cancel(CUnit &unit)
{
	this->IsCancelled = true;
}

/* virtual */ void COrder_Built::UpdateUnitVariables(CUnit &unit) const
{
	Assert(unit.CurrentOrder() == this);

	unit.Variable[BUILD_INDEX].Value = this->ProgressCounter;
	unit.Variable[BUILD_INDEX].Max = unit.Type->Stats[unit.Player->Index].Costs[TimeCost] * 600;

	// This should happen when building unit with several peons
	// Maybe also with only one.
	// FIXME : Should be better to fix it in action_{build,repair}.c ?
	if (unit.Variable[BUILD_INDEX].Value > unit.Variable[BUILD_INDEX].Max) {
		// assume value is wrong.
		unit.Variable[BUILD_INDEX].Value = unit.Variable[BUILD_INDEX].Max;
	}
}

/* virtual */ void COrder_Built::FillSeenValues(CUnit &unit) const
{
	unit.Seen.State = 1;
	unit.Seen.CFrame = this->Frame;
}

/** Called when unit is killed.
**  warn the AI module.
*/
void COrder_Built::AiUnitKilled(CUnit& unit)
{
	DebugPrint("%d: %d(%s) killed, under construction!\n" _C_
		unit.Player->Index _C_ UnitNumber(unit) _C_ unit.Type->Ident.c_str());
	AiReduceMadeInBuilt(*unit.Player->Ai, *unit.Type);
}


static const CConstructionFrame *FindCFramePercent(const CConstructionFrame &cframe, int percent)
{
	const CConstructionFrame *prev = &cframe;

	for (const CConstructionFrame *it = cframe.Next; it; it = it->Next) {
		if (percent < it->Percent) {
			return prev;
		}
		prev = it;
	}
	return prev;
}

/**
**  Update construction frame
**
**  @param unit  The building under construction.
*/
void COrder_Built::UpdateConstructionFrame(CUnit &unit)
{
	const CUnitType &type = *unit.Type;
	const int percent = this->ProgressCounter / (type.Stats[unit.Player->Index].Costs[TimeCost] * 6);
	const CConstructionFrame *cframe = FindCFramePercent(*type.Construction->Frames, percent);

	Assert(cframe != NULL);

	if (cframe != this->Frame) {
		this->Frame = cframe;
		if (unit.Frame < 0) {
			unit.Frame = -cframe->Frame - 1;
		} else {
			unit.Frame = cframe->Frame;
		}
	}
}


void COrder_Built::Progress(CUnit &unit, int amount)
{
	Boost(unit, amount, HP_INDEX);
	Boost(unit, amount, SHIELD_INDEX);

	this->ProgressCounter += amount * SpeedBuild;
	UpdateConstructionFrame(unit);
}

void COrder_Built::ProgressHp(CUnit &unit, int amount)
{
	Boost(unit, amount, HP_INDEX);

	this->ProgressCounter += amount * SpeedBuild;
	UpdateConstructionFrame(unit);
}


void COrder_Built::Boost(CUnit &building, int amount, int varIndex) const
{
	Assert(building.CurrentOrder() == this);

	const int costs = building.Stats->Costs[TimeCost] * 600;
	const int progress = this->ProgressCounter;
	const int newProgress = progress + amount * SpeedBuild;
	const int maxValue = building.Variable[varIndex].Max;

	int &currentValue = building.Variable[varIndex].Value;

	// damageValue is the current damage taken by the unit.
	const int damageValue = (progress * maxValue) / costs - currentValue;
	
	// Keep the same level of damage while increasing Value.
	currentValue = (newProgress * maxValue) / costs - damageValue;
	currentValue = std::min(currentValue, maxValue);
}



/**
**  Unit under Construction
**
**  @param unit  Unit that is being built
*/
void HandleActionBuilt(COrder& order, CUnit &unit)
{
	Assert(order.Action == UnitActionBuilt);

	if (order.Execute(unit)) {
		order.ClearGoal();
		unit.ClearAction();
	}
}

//@}
