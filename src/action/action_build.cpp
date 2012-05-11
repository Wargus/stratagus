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

#include "action/action_build.h"

#include "action/action_built.h"
#include "ai.h"
#include "animation.h"
#include "iolib.h"
#include "map.h"
#include "pathfinder.h"
#include "player.h"
#include "script.h"
#include "tileset.h"
#include "ui.h"
#include "unit.h"
#include "unittype.h"

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

/* static */ COrder *COrder::NewActionBuild(const CUnit &builder, const Vec2i &pos, CUnitType &building)
{
	Assert(Map.Info.IsPointOnMap(pos));

	COrder_Build *order = new COrder_Build;

	order->goalPos = pos;
	if (building.BuilderOutside) {
		order->Range = builder.Type->RepairRange;
	} else {
		// If building inside, but be next to stop
		if (building.ShoreBuilding && builder.Type->UnitType == UnitTypeLand) {
			// Peon won't dive :-)
			order->Range = 1;
		}
	}
	order->Type = &building;
	return order;
}


/* virtual */ void COrder_Build::Save(CFile &file, const CUnit &unit) const
{
	file.printf("{\"action-build\",");

	if (this->Finished) {
		file.printf(" \"finished\", ");
	}
	file.printf(" \"range\", %d,", this->Range);
	file.printf(" \"tile\", {%d, %d},", this->goalPos.x, this->goalPos.y);

	if (this->BuildingUnit != NULL) {
		file.printf(" \"building\", \"%s\",", UnitReference(this->BuildingUnit).c_str());
	}
	file.printf(" \"type\", \"%s\",", this->Type->Ident.c_str());
	file.printf(" \"state\", %d", this->State);
	file.printf("}");
}

/* virtual */ bool COrder_Build::ParseSpecificData(lua_State *l, int &j, const char *value, const CUnit &unit)
{
	if (!strcmp(value, "building")) {
		++j;
		lua_rawgeti(l, -1, j + 1);
		this->BuildingUnit = CclGetUnitFromRef(l);
		lua_pop(l, 1);
	} else if (!strcmp(value, "range")) {
		++j;
		lua_rawgeti(l, -1, j + 1);
		this->Range = LuaToNumber(l, -1);
		lua_pop(l, 1);
	} else if (!strcmp(value, "state")) {
		++j;
		lua_rawgeti(l, -1, j + 1);
		this->State = LuaToNumber(l, -1);
		lua_pop(l, 1);
	} else if (!strcmp(value, "tile")) {
		++j;
		lua_rawgeti(l, -1, j + 1);
		CclGetPos(l, &this->goalPos.x , &this->goalPos.y);
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

/* virtual */ bool COrder_Build::IsValid() const
{
	return true;
}

/* virtual */ PixelPos COrder_Build::Show(const CViewport &vp, const PixelPos &lastScreenPos) const
{
	PixelPos targetPos = vp.TilePosToScreen_TopLeft(this->goalPos);
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

/* virtual */ void COrder_Build::UpdatePathFinderData(PathFinderInput &input)
{
	input.SetMinRange(this->Type->BuilderOutside ? 1 : 0);
	input.SetMaxRange(this->Range);

	const Vec2i tileSize = {this->Type->TileWidth, this->Type->TileHeight};
	input.SetGoal(this->goalPos, tileSize);
}

/** Called when unit is killed.
**  warn the AI module.
*/
void COrder_Build::AiUnitKilled(CUnit &unit)
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

			unit.Player->Notify(NotifyYellow, unit.tilePos, "%s", _("You cannot reach building place"));
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
		player.Notify(NotifyYellow, unit.tilePos,
					  _("Not enough resources to build %s"), type.Name.c_str());
		isOk = false;
	}

	// Check if hiting any limits for the building.
	if (player.CheckLimits(type) < 0) {
		player.Notify(NotifyYellow, unit.tilePos,
					  _("Can't build more units %s"), type.Name.c_str());
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
	bool operator()(const CUnit *const unit) const {
		return (!unit->Destroyed && unit->Type == type
				&& (worker->Player == unit->Player || worker->IsAllied(*unit)));
	}
	CUnit *Find(const CMapField *const mf) const {
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
		unit.Player->Notify(NotifyYellow, unit.tilePos,
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

		b = static_cast<CBuildRestrictionOnTop *>(OnTopDetails(unit.Type->BuildingRules, *build, ontop.Type));
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
		CUnit &goal = *const_cast<COrder_Build *>(this)->BuildingUnit;


		targetOrder.ProgressHp(goal, 100);
	}
	if (unit.Anim.Unbreakable) {
		return false;
	}
	return this->BuildingUnit->CurrentAction() != UnitActionBuilt;
}


/* virtual */ void COrder_Build::Execute(CUnit &unit)
{
	if (unit.Wait) {
		unit.Wait--;
		return ;
	}
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

		if (ontop != NULL) {
			this->StartBuilding(unit, *ontop);
		}
	}
	if (this->State == State_StartBuilding_Failed) {
		unit.Player->Notify(NotifyYellow, unit.tilePos,
							_("You cannot build %s here"), type.Name.c_str());
		if (unit.Player->AiEnabled) {
			AiCanNotBuild(unit, type);
		}
		this->Finished = true;
		return ;
	}
	if (this->State == State_BuildFromOutside) {
		this->BuildFromOutside(unit);
	}
}

//@}
