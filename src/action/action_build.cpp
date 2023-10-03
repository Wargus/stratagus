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

/*----------------------------------------------------------------------------
--  Includes
----------------------------------------------------------------------------*/

#include "stratagus.h"

#include "action/action_build.h"

#include "action/action_built.h"
#include "ai.h"
#include "animation.h"
#include "iolib.h"
#include "map.h"
#include "pathfinder.h"
#include "player.h"
#include "script.h"
#include "translate.h"
#include "ui.h"
#include "unit.h"
#include "unittype.h"
#include "video.h"

extern void AiReduceMadeInBuilt(PlayerAi &pai, const CUnitType &type);

enum {
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

/* static */ std::unique_ptr<COrder> COrder::NewActionBuild(const CUnit &builder, const Vec2i &pos, CUnitType &building)
{
	Assert(Map.Info.IsPointOnMap(pos));

	auto order = std::make_unique<COrder_Build>();

	order->goalPos = pos;
	if (building.BoolFlag[BUILDEROUTSIDE_INDEX].value) {
		order->Range = builder.Type->RepairRange;
	} else {
		// If building inside, but be next to stop
		if (building.BoolFlag[SHOREBUILDING_INDEX].value && builder.Type->UnitType == UnitTypeLand) {
			// Peon won't dive :-)
			order->Range = 1;
		}
	}
	order->Type = &building;
	return order;
}


void COrder_Build::Save(CFile &file, const CUnit &unit) const /* override */
{
	file.printf("{\"action-build\",");

	if (this->Finished) {
		file.printf(" \"finished\", ");
	}
	file.printf(" \"range\", %d,", this->Range);
	file.printf(" \"tile\", {%d, %d},", this->goalPos.x, this->goalPos.y);

	if (this->BuildingUnit != nullptr) {
		file.printf(" \"building\", \"%s\",", UnitReference(this->BuildingUnit).c_str());
	}
	file.printf(" \"type\", \"%s\",", this->Type->Ident.c_str());
	file.printf(" \"state\", %d", this->State);
	file.printf("}");
}

bool COrder_Build::ParseSpecificData(lua_State *l,
                                     int &j,
                                     std::string_view value,
                                     const CUnit &unit) /* override */
{
	if (value == "building") {
		++j;
		lua_rawgeti(l, -1, j + 1);
		this->BuildingUnit = CclGetUnitFromRef(l);
		lua_pop(l, 1);
	} else if (value == "range") {
		++j;
		this->Range = LuaToNumber(l, -1, j + 1);
	} else if (value == "state") {
		++j;
		this->State = LuaToNumber(l, -1, j + 1);
	} else if (value == "tile") {
		++j;
		lua_rawgeti(l, -1, j + 1);
		CclGetPos(l, &this->goalPos);
		lua_pop(l, 1);
	} else if (value == "type") {
		++j;
		this->Type = &UnitTypeByIdent(LuaToString(l, -1, j + 1));
	} else {
		return false;
	}
	return true;
}

bool COrder_Build::IsValid() const /* override */
{
	return true;
}

PixelPos COrder_Build::Show(const CViewport &vp, const PixelPos &lastScreenPos) const /* override */
{
	PixelPos targetPos = vp.TilePosToScreen_Center(this->goalPos);
	targetPos.x += (this->GetUnitType().TileWidth - 1) * PixelTileSize.x / 2;
	targetPos.y += (this->GetUnitType().TileHeight - 1) * PixelTileSize.y / 2;

	const int w = this->GetUnitType().BoxWidth;
	const int h = this->GetUnitType().BoxHeight;
	DrawSelection(ColorGray, targetPos.x - w / 2, targetPos.y - h / 2, targetPos.x + w / 2, targetPos.y + h / 2);
	Video.FillCircleClip(ColorGreen, lastScreenPos, 2);
	Video.DrawLineClip(ColorGreen, lastScreenPos, targetPos);
	Video.FillCircleClip(ColorGreen, targetPos, 3);
	return targetPos;
}

void COrder_Build::UpdatePathFinderData(PathFinderInput &input) /* override */
{
	input.SetMinRange(this->Type->BoolFlag[BUILDEROUTSIDE_INDEX].value && input.GetUnit()->CanMove() ? 1 : 0);
	input.SetMaxRange(this->Range);

	const Vec2i tileSize(this->Type->TileWidth, this->Type->TileHeight);
	input.SetGoal(this->goalPos, tileSize);
}

/** Called when unit is killed.
**  warn the AI module.
*/
void COrder_Build::AiUnitKilled(CUnit &unit)
{
	DebugPrint("%d: %d(%s) killed, with order %s!\n",
	           unit.Player->Index,
	           UnitNumber(unit),
	           unit.Type->Ident.c_str(),
	           this->Type->Ident.c_str());
	if (this->BuildingUnit == nullptr) {
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
		unit.pathFinderData->output.Cycles = 0; //moving counter
		this->State = 1;
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

			unit.Player->Notify(ColorYellow, unit.tilePos, "%s", _("You cannot reach building place"));
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
		player.Notify(
			ColorYellow, unit.tilePos, _("Not enough resources to build %s"), type.Name.c_str());
		isOk = false;
	}

	// Check if hiting any limits for the building.
	if (player.CheckLimits(type) < 0) {
		player.Notify(ColorYellow, unit.tilePos, _("Can't build more units %s"), type.Name.c_str());
		isOk = false;
	}
	if (isOk == false && player.AiEnabled) {
		AiCanNotBuild(unit, type);
	}
	return isOk;
}


class AlreadyBuildingFinder
{
public:
	AlreadyBuildingFinder(const CUnit &unit, const CUnitType &t) :
		worker(&unit), type(&t) {}
	bool operator()(const CUnit *const unit) const
	{
		return (!unit->Destroyed && unit->Type == type
				&& (worker->Player == unit->Player || worker->IsAllied(*unit)));
	}
	CUnit *Find(const CMapField *const mf) const
	{
		auto it = ranges::find_if(mf->UnitCache, *this);
		return it != mf->UnitCache.end() ? *it : nullptr;
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

	if (ontop != nullptr) {
		return ontop;
	}
#if 0
	/*
	 * FIXME: rb - CheckAlreadyBuilding should be somehow
	 * enabled/disable via game lua scripting
	 */
	CUnit *building = AlreadyBuildingFinder(unit, type).Find(Map.Field(pos));
	if (building != nullptr) {
		if (unit.CurrentOrder() == this) {
			DebugPrint("%d: Worker [%d] is helping build: %s [%d]\n",
					   unit.Player->Index, unit.Slot,
					   building->Type->Name.c_str(),
					   building->Slot);

			delete this; // Bad
			unit.Orders[0] = COrder::NewActionRepair(unit, *building);
			return nullptr;
		}
	}
#endif
	// Some tries to build the building.
	this->State++;
	// To keep the load low, retry each 10 cycles
	// NOTE: we can already inform the AI about this problem?
	unit.Wait = 10;
	return nullptr;
}


bool COrder_Build::StartBuilding(CUnit &unit, CUnit &ontop)
{
	const CUnitType &type = this->GetUnitType();

	unit.Player->SubUnitType(type);

	CUnit *build = MakeUnit(const_cast<CUnitType &>(type), unit.Player);

	// If unable to make unit, stop, and report message
	if (build == nullptr) {
		// FIXME: Should we retry this?
		unit.Player->Notify(
			ColorYellow, unit.tilePos, _("Unable to create building %s"), type.Name.c_str());
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

		b = static_cast<CBuildRestrictionOnTop *>(OnTopDetails(*build, ontop.Type));
		Assert(b);
		if (b->ReplaceOnBuild) {
			build->ResourcesHeld = ontop.ResourcesHeld; // We capture the value of what is beneath.
			build->Variable[GIVERESOURCE_INDEX].Value = ontop.Variable[GIVERESOURCE_INDEX].Value;
			build->Variable[GIVERESOURCE_INDEX].Max = ontop.Variable[GIVERESOURCE_INDEX].Max;
			build->Variable[GIVERESOURCE_INDEX].Enable = ontop.Variable[GIVERESOURCE_INDEX].Enable;
			ontop.Remove(nullptr); // Destroy building beneath
			UnitLost(ontop);
			UnitClearOrders(ontop);
			ontop.Release();
		}
	}

	if (type.BoolFlag[MAINFACILITY_INDEX].value && CPlayer::IsRevelationEnabled()
	    && unit.Player->LostMainFacilityTimer != 0) {

		unit.Player->LostMainFacilityTimer = 0;
		unit.Player->SetRevealed(false);
		for (int j = 0; j < NumPlayers; ++j) {
			if (unit.Player->Index != j && Players[j].Type != PlayerTypes::PlayerNobody) {
				Players[j].Notify(_("%s has rebuilt a base, and will no longer be revealed!"), unit.Player->Name.c_str());
			} else {
				Players[j].Notify("%s", _("You have rebuilt a base, and will no longer be revealed!"));
			}
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
	if (build->Active) {
		build->Player->UnitTypesAiActiveCount[type.Slot]--;
	}

	// We need somebody to work on it.
	if (!type.BoolFlag[BUILDEROUTSIDE_INDEX].value) {
		UnitShowAnimation(unit, unit.Type->Animations->Still);
		unit.Remove(build);
		this->State = State_BuildFromInside;
		if (unit.Selected) {
			SelectedUnitChanged();
		}
	} else {
		this->State = State_BuildFromOutside;
		this->BuildingUnit = build;
		UnitHeadingFromDeltaXY(unit, build->tilePos - unit.tilePos);
	}
	return true;
}

static void AnimateActionBuild(CUnit &unit)
{
	CAnimations *animations = unit.Type->Animations;

	if (animations == nullptr) {
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

	if (this->BuildingUnit == nullptr) {
		return false;
	}

	if (this->BuildingUnit->CurrentAction() == UnitAction::Built) {
		COrder_Built &targetOrder = *static_cast<COrder_Built *>(this->BuildingUnit->CurrentOrder());
		CUnit &goal = *const_cast<COrder_Build *>(this)->BuildingUnit;

		targetOrder.ProgressHp(goal, 100);
	}
	if (unit.Anim.Unbreakable) {
		return false;
	}
	return this->BuildingUnit->CurrentAction() != UnitAction::Built;
}

CUnit *COrder_Build::GetBuildingUnit() const
{
	return this->BuildingUnit;
}

void COrder_Build::UpdateUnitVariables(CUnit &unit) const /* override */
{
	if (this->State == State_BuildFromOutside && this->BuildingUnit != nullptr) {
		unit.Variable[TRAINING_INDEX].Value = this->BuildingUnit->Variable[BUILD_INDEX].Value;
		unit.Variable[TRAINING_INDEX].Max = this->BuildingUnit->Variable[BUILD_INDEX].Max;
	}
}

void COrder_Build::Execute(CUnit &unit) /* override */
{
	if (IsWaiting(unit)) {
		return;
	}
	StopWaiting(unit);
	if (this->State <= State_MoveToLocationMax) {
		if (this->MoveToLocation(unit)) {
			this->Finished = true;
			return ;
		}
	}
	const CUnitType &type = this->GetUnitType();

	if (State_NearOfLocation <= this->State && this->State < State_StartBuilding_Failed) {
		if (CheckLimit(unit, type) == false) {
			this->Finished = true;
			return ;
		}
		CUnit *ontop = this->CheckCanBuild(unit);

		if (ontop != nullptr) {
			this->StartBuilding(unit, *ontop);
		}
	}
	if (this->State == State_StartBuilding_Failed) {
		unit.Player->Notify(
			ColorYellow, unit.tilePos, _("You cannot build %s here"), type.Name.c_str());
		if (unit.Player->AiEnabled) {
			AiCanNotBuild(unit, type);
		}
		this->Finished = true;
		return ;
	}
	if (this->State == State_BuildFromOutside) {
		if (this->BuildFromOutside(unit)) {
			this->Finished = true;
		}
	}
}

void COrder_Build::Cancel(CUnit &unit) /* override */
{
	if (this->State == State_BuildFromOutside && this->BuildingUnit != nullptr && this->BuildingUnit->CurrentAction() == UnitAction::Built) {
		COrder_Built &targetOrder = *static_cast<COrder_Built *>(this->BuildingUnit->CurrentOrder());
		targetOrder.Cancel(*this->BuildingUnit);
	}
}

/**
**  Get goal position
*/
const Vec2i COrder_Build::GetGoalPos() const /* override */
{
	const Vec2i invalidPos(-1, -1);
	if (goalPos != invalidPos) {
		return goalPos;
	}
	if (this->HasGoal()) {
		return this->GetGoal()->tilePos;
	}
	return invalidPos;
}

//@}
