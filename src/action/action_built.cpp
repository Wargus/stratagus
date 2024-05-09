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
//      (c) Copyright 1998-2015 by Lutz Sammer, Jimmy Salmon,
//      Russell Smith and Andrettin
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

#include "stratagus.h"
#include "action/action_built.h"

#include "ai.h"
#include "commands.h"
#include "construct.h"
#include "iolib.h"
#include "luacallback.h"
#include "map.h"
#include "player.h"
#include "script.h"
#include "sound.h"
#include "translate.h"
#include "unit.h"
#include "unittype.h"

/// How many resources the player gets back if canceling building
#define CancelBuildingCostsFactor  75


extern void AiReduceMadeInBuilt(PlayerAi &pai, const CUnitType &type);

/* static */ std::unique_ptr<COrder> COrder::NewActionBuilt(CUnit &builder, CUnit &unit)
{
	auto order = std::make_unique < COrder_Built>();

	// Make sure the bulding doesn't cancel itself out right away.

	unit.Variable[HP_INDEX].Value = 1;
	if (unit.Variable[SHIELD_INDEX].Max) {
		unit.Variable[SHIELD_INDEX].Value = 1;
	}
	order->UpdateConstructionFrame(unit);

	if (unit.Type->BoolFlag[BUILDEROUTSIDE_INDEX].value == false) {
		order->Worker = &builder;
	}
	return order;
}


void COrder_Built::Save(CFile &file, const CUnit &unit) const /* override */
{
	file.printf("{\"action-built\", ");
	if (this->Finished) {
		file.printf(" \"finished\", ");
	}
	if (this->Worker != nullptr) {
		file.printf("\"worker\", \"%s\", ", UnitReference(*this->Worker).c_str());
	}
	const int frame = this->Frame;
	file.printf("\"progress\", %d, \"frame\", %d", this->ProgressCounter, frame);
	if (this->IsCancelled) {
		file.printf(", \"cancel\"");
	}
	file.printf("}");
}

bool COrder_Built::ParseSpecificData(lua_State *l,
                                     int &j,
                                     std::string_view value,
                                     const CUnit &unit) /* override */
{
	if (value == "worker") {
		++j;
		lua_rawgeti(l, -1, j + 1);
		this->Worker = CclGetUnitFromRef(l);
		lua_pop(l, 1);
	} else if (value == "progress") {
		++j;
		this->ProgressCounter = LuaToNumber(l, -1, j + 1);
	} else if (value == "cancel") {
		this->IsCancelled = true;
	} else if (value == "frame") {
		++j;
		this->Frame = LuaToNumber(l, -1, j + 1);
	} else {
		return false;
	}
	return true;
}

bool COrder_Built::IsValid() const /* override */
{
	return true;
}

PixelPos COrder_Built::Show(const CViewport &, const PixelPos &lastScreenPos) const /* override */
{
	return lastScreenPos;
}


static void CancelBuilt(COrder_Built &order, CUnit *unit)
{
	Assert(unit == nullptr || unit->CurrentOrder() == &order);
	CUnit *worker = order.GetWorkerPtr();

	// Drop out unit
	if (worker != nullptr && worker->CurrentAction() == UnitAction::Build && !worker->CurrentOrder()->Finished) {
		worker->ClearAction();

		DropOutOnSide(*worker, LookingW, unit);
	}
	if (unit != nullptr) {
		// Player gets back 75% of the original cost for a building.
		unit->Player->AddCostsFactor(unit->Stats->Costs, CancelBuildingCostsFactor);
		// Cancel building
		LetUnitDie(*unit);
	}
}

static void Finish(COrder_Built &order, CUnit &unit)
{
	const CUnitType &type = *unit.Type;
	CPlayer &player = *unit.Player;

	DebugPrint("%d: Building %s(%s) ready.\n", player.Index, type.Ident.c_str(), type.Name.c_str());

	// HACK: the building is ready now
	player.UnitTypesCount[type.Slot]++;
	if (unit.Active) {
		player.UnitTypesAiActiveCount[type.Slot]++;
	}
	unit.Constructed = 0;
	if (unit.Frame < 0) {
		unit.Frame = -1;
	} else {
		unit.Frame = 0;
	}
	CUnit *worker = order.GetWorkerPtr();

	if (worker != nullptr) {
		if (type.BoolFlag[BUILDERLOST_INDEX].value) {
			// Bye bye worker.
			LetUnitDie(*worker);
			worker = nullptr;
		} else { // Drop out the worker.
			worker->ClearAction();

			DropOutOnSide(*worker, LookingW, &unit);

			// If we can harvest from the new building, do it.
			if (worker->Type->ResInfo[type.GivesResource]) {
				CommandResource(*worker, unit, EFlushMode::Off);
			}
			// If we can reurn goods to a new depot, do it.
			if (worker->CurrentResource && worker->ResourcesHeld > 0 && type.CanStore[worker->CurrentResource]) {
				CommandReturnGoods(*worker, &unit, EFlushMode::Off);
			}
		}
	}

	if (type.GivesResource && type.StartingResources != 0) {
		// Has StartingResources, Use those
		unit.ResourcesHeld = type.StartingResources;
	}

	player.Notify(ColorGreen, unit.tilePos, _("New %s done"), type.Name.c_str());
	if (&player == ThisPlayer) {
		if (type.MapSound.Ready.Sound) {
			PlayUnitSound(unit, EUnitVoice::Ready);
		} else if (worker) {
			PlayUnitSound(*worker, EUnitVoice::WorkCompleted);
		} else {
			PlayUnitSound(unit, EUnitVoice::Building);
		}
	}

	if (player.AiEnabled) {
		/* Worker can be nullptr */
		AiWorkComplete(worker, unit);
	}

	if (unit.Type->OnReady) {
		unit.Type->OnReady(UnitNumber(unit));
	}

	// FIXME: Vladi: this is just a hack to test wall fixing,
	// FIXME:  also not sure if the right place...
	// FIXME: Johns: hardcoded unit-type wall / more races!
	if (&type == UnitTypeOrcWall || &type == UnitTypeHumanWall) {
		Map.SetWall(unit.tilePos, &type == UnitTypeHumanWall);
		order.Finished = true;
		unit.Remove(nullptr);
		UnitLost(unit);
		UnitClearOrders(unit);
		unit.Release();
		return ;
	}

	UpdateForNewUnit(unit, 0);

	// Set the direction of the building if it supports them
	if (type.NumDirections > 1 && type.BoolFlag[NORANDOMPLACING_INDEX].value == false) {
		if (type.BoolFlag[WALL_INDEX].value) { // Special logic for walls
			CorrectWallDirections(unit);
			CorrectWallNeighBours(unit);
		} else {
			unit.Direction = (SyncRand() >> 8) & 0xFF; // random heading
		}
		UnitUpdateHeading(unit);
	}

	if (IsOnlySelected(unit) || &player == ThisPlayer) {
		SelectedUnitChanged();
	}
	MapUnmarkUnitSight(unit);
	unit.CurrentSightRange = unit.Stats->Variables[SIGHTRANGE_INDEX].Max;
	MapMarkUnitSight(unit);
	order.Finished = true;
}

COrder_Built::~COrder_Built()
{
	CancelBuilt(*this, nullptr);
}

void COrder_Built::Execute(CUnit &unit) /* override */
{
	const CUnitType &type = *unit.Type;

	int amount;
	if (type.BoolFlag[BUILDEROUTSIDE_INDEX].value) {
		amount = type.AutoBuildRate;
	} else {
		// FIXME: implement this below:
		// this->Data.Worker->Type->BuilderSpeedFactor;
		amount = 100;
	}
	this->Progress(unit, amount);

	// Check if construction should be canceled...
	if (this->IsCancelled || this->ProgressCounter < 0) {
		DebugPrint("%d: %s canceled.\n", unit.Player->Index, unit.Type->Name.c_str());

		CancelBuilt(*this, &unit);
		return ;
	}

	const int maxProgress = type.Stats[unit.Player->Index].Costs[TimeCost] * 600;

	// Check if we should make some random noise
	// IMPORTANT: this is local randomization, do not use the SyncRand function!
	if (unit.Frame == 0 && unit.Player == ThisPlayer && GameCycle % 150 == 0 && (MyRand() % 3) == 0) {
		PlayUnitSound(unit, EUnitVoice::Building, true);
	}

	// Check if building ready. Note we can both build and repair.
	if (!unit.Anim.Unbreakable && this->ProgressCounter >= maxProgress) {
		Finish(*this, unit);
	}
}

void COrder_Built::Cancel(CUnit &unit) /* override */
{
	this->IsCancelled = true;
}

void COrder_Built::UpdateUnitVariables(CUnit &unit) const /* override */
{
	Assert(unit.CurrentOrder() == this);

	unit.Variable[BUILD_INDEX].Value = this->ProgressCounter;
	unit.Variable[BUILD_INDEX].Max = unit.Type->Stats[unit.Player->Index].Costs[TimeCost] * 600;

	// This should happen when building unit with several peons
	// Maybe also with only one.
	// FIXME : Should be better to fix it in action_{build,repair}.c ?
	unit.Variable[BUILD_INDEX].Value = std::min(unit.Variable[BUILD_INDEX].Max, unit.Variable[BUILD_INDEX].Value);
}

void COrder_Built::FillSeenValues(CUnit &unit) const /* override */
{
	unit.Seen.State = 1;
	unit.Seen.CFrame = this->Frame;
}

/** Called when unit is killed.
**  warn the AI module.
*/
void COrder_Built::AiUnitKilled(CUnit &unit)
{
	DebugPrint("%d: %d(%s) killed, under construction!\n",
	           unit.Player->Index,
	           UnitNumber(unit),
	           unit.Type->Ident.c_str());
	AiReduceMadeInBuilt(*unit.Player->Ai, *unit.Type);
}


static std::optional<std::size_t> FindCFramePercent(const std::vector<CConstructionFrame> &cframes, int percent)
{
	if (cframes.empty())
	{
		return {};
	}
	const auto it =
		ranges::find_if(cframes, [&](const auto &frame) { return percent < frame.Percent; });
	const std::size_t index = std::distance(cframes.begin(), it);
	return index > 0 ? index - 1 : 0;
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
	const auto index = FindCFramePercent(type.Construction->Frames, percent);

	if (index.has_value() && index.value() != this->Frame) {
		this->Frame = index.value();
		const CConstructionFrame &cframe = type.Construction->Frames[index.value()];
		unit.Frame = (unit.Frame < 0) ? -cframe.Frame - 1 : cframe.Frame;
	}
}


void COrder_Built::Progress(CUnit &unit, int amount)
{
	Boost(unit, amount, HP_INDEX);
	Boost(unit, amount, SHIELD_INDEX);

	this->ProgressCounter += std::max(1, amount * unit.Player->SpeedBuild / SPEEDUP_FACTOR);
	UpdateConstructionFrame(unit);
}

void COrder_Built::ProgressHp(CUnit &unit, int amount)
{
	Boost(unit, amount, HP_INDEX);

	this->ProgressCounter += std::max(1, amount * unit.Player->SpeedBuild / SPEEDUP_FACTOR);
	UpdateConstructionFrame(unit);
}


void COrder_Built::Boost(CUnit &building, int amount, int varIndex) const
{
	Assert(building.CurrentOrder() == this);

	const int costs = building.Stats->Costs[TimeCost] * 600;
	const int progress = this->ProgressCounter;
	const int newProgress = progress + std::max(1, amount * building.Player->SpeedBuild / SPEEDUP_FACTOR);
	const int maxValue = building.Variable[varIndex].Max;

	int &currentValue = building.Variable[varIndex].Value;

	// damageValue is the current damage taken by the unit.
	const int damageValue = (progress * maxValue) / costs - currentValue;

	// Keep the same level of damage while increasing Value.
	currentValue = (newProgress * maxValue) / costs - damageValue;
	currentValue = std::min(currentValue, maxValue);
}

//@}
