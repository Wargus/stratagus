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
/**@name action_resource.cpp - The generic resource action. */
//
//      (c) Copyright 2001-2005 by Crestez Leonard and Jimmy Salmon
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

#include "action/action_resource.h"

#include "ai.h"
#include "animation.h"
#include "interface.h"
#include "iolib.h"
#include "map.h"
#include "pathfinder.h"
#include "player.h"
#include "script.h"
#include "sound.h"
#include "tileset.h"
#include "translate.h"
#include "ui.h"
#include "unit.h"
#include "unit_find.h"
#include "unittype.h"
#include "video.h"

#include "../ai/ai_local.h"

/*----------------------------------------------------------------------------
--  Declarations
----------------------------------------------------------------------------*/

#define SUB_START_RESOURCE 0
#define SUB_MOVE_TO_RESOURCE 5
#define SUB_UNREACHABLE_RESOURCE 31
#define SUB_START_GATHERING 55
#define SUB_GATHER_RESOURCE 60
#define SUB_STOP_GATHERING 65
#define SUB_MOVE_TO_DEPOT 70
#define SUB_UNREACHABLE_DEPOT 100
#define SUB_RETURN_RESOURCE 120

/*----------------------------------------------------------------------------
--  Functions
----------------------------------------------------------------------------*/

class NearReachableTerrainFinder
{
public:
	friend TerrainTraversal;

	static std::optional<Vec2i> find(
		int movemask, int resmask, int range, const CPlayer &player, const Vec2i &startPos)
	{
		TerrainTraversal terrainTraversal;

		terrainTraversal.SetSize(Map.Info.MapWidth, Map.Info.MapHeight);
		terrainTraversal.Init();

		terrainTraversal.PushPos(startPos);

		NearReachableTerrainFinder nearReachableTerrainFinder(player, range, movemask, resmask);

		return terrainTraversal.Run(nearReachableTerrainFinder)
		         ? std::make_optional(nearReachableTerrainFinder.resPos)
		         : std::nullopt;
	}

private:
	NearReachableTerrainFinder(const CPlayer &player, int maxDist, int movemask, int resmask) :
		player(player),
		maxDist(maxDist),
		movemask(movemask),
		resmask(resmask)
	{}
	VisitResult Visit(TerrainTraversal &terrainTraversal, const Vec2i &pos, const Vec2i &from);
private:
	const CPlayer &player;
	int maxDist;
	unsigned int movemask;
	unsigned int resmask;
	Vec2i resPos{-1, -1};
};

VisitResult NearReachableTerrainFinder::Visit(TerrainTraversal &terrainTraversal, const Vec2i &pos, const Vec2i &from)
{
	if (!player.AiEnabled && !Map.Field(pos)->playerInfo.IsExplored(player)) {
		return VisitResult::DeadEnd;
	}
	// Look if found what was required.
	if (CanMoveToMask(pos, movemask)) {
		resPos = from;
		return VisitResult::Finished;
	}
	if (Map.Field(pos)->CheckMask(resmask)) { // reachable
		if (terrainTraversal.Get(pos) <= maxDist) {
			return VisitResult::Ok;
		} else {
			return VisitResult::DeadEnd;
		}
	} else { // unreachable
		return VisitResult::DeadEnd;
	}
}

/* static */ std::unique_ptr<COrder> COrder::NewActionResource(CUnit &harvester, const Vec2i &pos)
{
	auto order = std::make_unique<COrder_Resource>(harvester);

	int targetFlag;
	CMapField *mf = Map.Field(pos);
	if (mf->Cost4OnMap()) {
		order->CurrentResource = Cost4;
		targetFlag = MapFieldCost4;
	} else if (mf->Cost5OnMap()) {
		order->CurrentResource = Cost5;
		targetFlag = MapFieldCost5;
	} else if (mf->Cost6OnMap()) {
		order->CurrentResource = Cost6;
		targetFlag = MapFieldCost6;
	} else {
		order->CurrentResource = WoodCost;
		targetFlag = MapFieldForest;
	}

	//  Find the closest piece of wood next to a tile where the unit can move
	if (auto resourceLoc = NearReachableTerrainFinder::find(
			harvester.Type->MovementMask, targetFlag, 20, *harvester.Player, pos)) {
		order->goalPos = *resourceLoc;
	} else {
		DebugPrint("FIXME: Give up???\n");
		order->goalPos = pos;
	}
	return order;
}

/* static */ std::unique_ptr<COrder> COrder::NewActionResource(CUnit &harvester, CUnit &mine)
{
	auto order = std::make_unique<COrder_Resource>(harvester);

	order->SetGoal(&mine);
	order->Resource.Mine = &mine;
	order->Resource.Pos = mine.tilePos + mine.Type->GetHalfTileSize();
	order->CurrentResource = mine.Type->GivesResource;
	return order;
}

/* static */ std::unique_ptr<COrder> COrder::NewActionReturnGoods(CUnit &harvester, CUnit *depot)
{
	auto order = std::make_unique<COrder_Resource>(harvester);

	// Destination could be killed. NETWORK!
	if (depot && depot->Destroyed) {
		depot = nullptr;
	}
	// clicking on an allied depot still doesn't allow you to deposit there depending on preference
	if (depot && !GameSettings.AllyDepositsAllowed && depot->Player != harvester.Player) {
		depot = nullptr;
	}
	order->CurrentResource = harvester.CurrentResource;
	order->DoneHarvesting = true;

	if (depot == nullptr) {
		depot = FindDeposit(harvester, 1000, harvester.CurrentResource);
	}
	if (depot) {
		order->Depot = depot;
		order->UnitGotoGoal(harvester, depot, SUB_MOVE_TO_DEPOT);
	} else {
		order->State = SUB_UNREACHABLE_DEPOT;
		order->goalPos = harvester.tilePos;
	}
	return order;
}


Vec2i COrder_Resource::GetHarvestLocation() const
{
	if (this->Resource.Mine != nullptr) {
		return this->Resource.Mine->tilePos;
	} else {
		return this->Resource.Pos;
	}
}

bool COrder_Resource::IsGatheringStarted() const
{
	return this->State > SUB_START_GATHERING;
}

bool COrder_Resource::IsGatheringFinished() const
{
	return this->State >= SUB_STOP_GATHERING;
}

bool COrder_Resource::IsGatheringWaiting() const
{
	return this->State == SUB_START_GATHERING && this->worker->Wait != 0;
}

COrder_Resource::~COrder_Resource()
{
	CUnit *mine = this->Resource.Mine;
	this->Resource.Mine = nullptr;
	if (mine && mine->IsAlive()) {
		worker->DeAssignWorkerFromMine(*mine);
	}

	Depot = nullptr;

	CUnit *goal = this->GetGoal();
	if (goal) {
		// If mining decrease the active count on the resource.
		if (this->State == SUB_GATHER_RESOURCE) {

			goal->Resource.Active--;
			Assert(goal->Resource.Active >= 0);
		}
	}
}

void COrder_Resource::Save(CFile &file, const CUnit &unit) const /* override */
{
	file.printf("{\"action-resource\",");
	if (this->Finished) {
		file.printf(" \"finished\",");
	}
	if (this->HasGoal()) {
		file.printf(" \"goal\", \"%s\",", UnitReference(*this->GetGoal()).c_str());
	}
	file.printf(" \"tile\", {%d, %d},", this->goalPos.x, this->goalPos.y);

	Assert(this->worker != nullptr && worker->IsAlive());
	file.printf(" \"worker\", \"%s\",", UnitReference(*worker).c_str());
	file.printf(" \"current-res\", %d,", this->CurrentResource);

	file.printf(" \"res-pos\", {%d, %d},", this->Resource.Pos.x, this->Resource.Pos.y);
	if (this->Resource.Mine != nullptr) {
		file.printf(" \"res-mine\", \"%s\",", UnitReference(*this->Resource.Mine).c_str());
	}
	if (this->Depot != nullptr) {
		file.printf(" \"res-depot\", \"%s\",", UnitReference(*this->Depot).c_str());
	}
	if (this->DoneHarvesting) {
		file.printf(" \"done-harvesting\",");
	}
	file.printf(" \"timetoharvest\", %d,", this->TimeToHarvest);
	file.printf(" \"state\", %d", this->State);
	file.printf("}");
}

bool COrder_Resource::ParseSpecificData(lua_State *l,
                                        int &j,
                                        std::string_view value,
                                        const CUnit &unit) /* override */
{
	if (value == "current-res") {
		++j;
		this->CurrentResource = LuaToNumber(l, -1, j + 1);
	} else if (value == "done-harvesting") {
		this->DoneHarvesting = true;
	} else if (value == "res-depot") {
		++j;
		lua_rawgeti(l, -1, j + 1);
		this->Depot = CclGetUnitFromRef(l);
		lua_pop(l, 1);
	} else if (value == "res-mine") {
		++j;
		lua_rawgeti(l, -1, j + 1);
		this->Resource.Mine = CclGetUnitFromRef(l);
		lua_pop(l, 1);
	} else if (value == "res-pos") {
		++j;
		lua_rawgeti(l, -1, j + 1);
		CclGetPos(l, &this->Resource.Pos);
		lua_pop(l, 1);
	} else if (value == "state") {
		++j;
		this->State = LuaToNumber(l, -1, j + 1);
	} else if (value == "timetoharvest") {
		++j;
		this->TimeToHarvest = LuaToNumber(l, -1, j + 1);
	} else if (value == "worker") {
		++j;
		lua_rawgeti(l, -1, j + 1);
		this->worker = CclGetUnitFromRef(l);
		lua_pop(l, 1);
	} else if (value == "tile") {
		++j;
		lua_rawgeti(l, -1, j + 1);
		CclGetPos(l, &this->goalPos);
		lua_pop(l, 1);
	} else {
		return false;
	}
	return true;
}

bool COrder_Resource::IsValid() const /* override */
{
	return true;
}

PixelPos COrder_Resource::Show(const CViewport &vp,
                               const PixelPos &lastScreenPos) const /* override */
{
	PixelPos targetPos;

	if (this->HasGoal()) {
		targetPos = vp.MapToScreenPixelPos(this->GetGoal()->GetMapPixelPosCenter());
	} else {
		targetPos = vp.TilePosToScreen_Center(this->goalPos);
	}
	Video.FillCircleClip(ColorYellow, lastScreenPos, 2);
	Video.DrawLineClip(ColorYellow, lastScreenPos, targetPos);
	Video.FillCircleClip(ColorYellow, targetPos, 3);
	return targetPos;
}

void COrder_Resource::UpdatePathFinderData(PathFinderInput &input) /* override */
{
	input.SetMinRange(0);
	input.SetMaxRange(1);

	Vec2i tileSize;
	if (this->HasGoal()) {
		CUnit *goal = this->GetGoal();
		tileSize.x = goal->Type->TileWidth;
		tileSize.y = goal->Type->TileHeight;
		input.SetGoal(goal->tilePos, tileSize);
	} else {
		tileSize.x = 0;
		tileSize.y = 0;
		input.SetGoal(this->goalPos, tileSize);
	}
}


bool COrder_Resource::OnAiHitUnit(CUnit &unit, CUnit *attacker, int /* damage*/) /* override */
{
	if (this->IsGatheringFinished()) {
		// Normal return to depot
		return true;
	}
	if (this->IsGatheringStarted()  && unit.ResourcesHeld > 0) {
		// escape to Depot with what you have
		const ResourceInfo &resinfo = *unit.Type->ResInfo[this->CurrentResource];
		if (resinfo.TerrainHarvester && unit.ResourcesHeld < resinfo.ResourceCapacity) {
			// We don't have anything yet.
			return false;
		}
		this->DoneHarvesting = true;
		return true;
	}
	return false;
}



/**
**  Move unit to terrain.
**
**  @return      1 if reached, -1 if unreacheable, 0 if on the way.
*/
int COrder_Resource::MoveToResource_Terrain(CUnit &unit)
{
	// Wood gone, look somewhere else.
	if ((Map.Info.IsPointOnMap(this->goalPos) == false
	     || Map.Field(this->goalPos)->IsTerrainResourceOnMap(CurrentResource) == false)
		&& (!unit.IX) && (!unit.IY)) {
		if (auto pos = FindTerrainType(unit.Type->MovementMask, MapFieldForest, 16, *unit.Player, this->goalPos)) {
			this->goalPos = *pos;
		} else {
			// no wood in range
			return -1;
		}
	}
	switch (DoActionMove(unit)) {
		case PF_UNREACHABLE:
			unit.Wait = 10;
			if (unit.Player->AiEnabled) {
				this->Range++;
				if (this->Range >= 5) {
					this->Range = 0;
					AiCanNotMove(unit);
				}
			}
			if (auto pos = FindTerrainType(
					unit.Type->MovementMask, MapFieldForest, 9999, *unit.Player, unit.tilePos)) {
				this->goalPos = *pos;
				DebugPrint("Found a better place to harvest %d,%d\n", pos->x, pos->y);
				// FIXME: can't this overflow? It really shouldn't,
				// since x and y are really supposed to be reachable,
				// checked through a flood fill.
				// I don't know, sometime stuff happens.
				return 0;
			}
			return -1;
		case PF_REACHED:
			return 1;
		case PF_WAIT:
			if (unit.Player->AiEnabled) {
				this->Range++;
				if (this->Range >= 5) {
					this->Range = 0;
					AiCanNotMove(unit);
				}
			}
		default:
			return 0;
	}
}

/**
**  Move unit to unit resource.
**
**  @return      1 if reached, -1 if unreacheable, 0 if on the way.
*/
int COrder_Resource::MoveToResource_Unit(CUnit &unit)
{
	const CUnit *goal = this->GetGoal();
	Assert(goal);

	switch (DoActionMove(unit)) { // reached end-point?
		case PF_UNREACHABLE:
			return -1;
		case PF_REACHED:
			break;
		case PF_WAIT:
			if (unit.Player->AiEnabled) {
				this->Range++;
				if (this->Range >= 5) {
					this->Range = 0;
					AiCanNotMove(unit);
				}
			}
		default:
			// Goal gone or something.
			if (unit.Anim.Unbreakable || goal->IsVisibleAsGoal(*unit.Player)) {
				return 0;
			}
			break;
	}
	return 1;
}

/**
**  Move unit to resource.
**
**  @param unit  Pointer to unit.
**
**  @return      1 if reached, -1 if unreacheable, 0 if on the way.
*/
int COrder_Resource::MoveToResource(CUnit &unit)
{
	const ResourceInfo &resinfo = *unit.Type->ResInfo[this->CurrentResource];

	if (resinfo.TerrainHarvester) {
		return MoveToResource_Terrain(unit);
	} else {
		return MoveToResource_Unit(unit);
	}
}

void COrder_Resource::UnitGotoGoal(CUnit &unit, CUnit *const goal, int state)
{
	if (this->GetGoal() != goal) {
		this->SetGoal(goal);
		if (goal) {
			this->goalPos.x = this->goalPos.y = -1;
		}
	}
	this->State = state;
	if (state == SUB_MOVE_TO_DEPOT || state == SUB_MOVE_TO_RESOURCE) {
		unit.pathFinderData->output.Cycles = 0; //moving counter
	}
}

/**
**  Start harvesting the resource.
**
**  @param unit  Pointer to unit.
**
**  @return      TRUE if ready, otherwise FALSE.
*/
int COrder_Resource::StartGathering(CUnit &unit)
{
	CUnit *goal;
	const ResourceInfo &resinfo = *unit.Type->ResInfo[this->CurrentResource];
	Assert(!unit.IX);
	Assert(!unit.IY);

	if (resinfo.TerrainHarvester) {
		// This shouldn't happened?
#if 0
		if (!Map.IsTerrainResourceOnMap(unit.Orders->goalPos, this->CurrentResource)) {
			DebugPrint("Wood gone, just like that?\n");
			return 0;
		}
#endif
		UnitHeadingFromDeltaXY(unit, this->goalPos - unit.tilePos);
		if (resinfo.WaitAtResource) {
			this->TimeToHarvest = std::max<int>(1, resinfo.WaitAtResource * SPEEDUP_FACTOR / unit.Player->SpeedResourcesHarvest[resinfo.ResourceId]);
		} else {
			this->TimeToHarvest = 1;
		}
		this->DoneHarvesting = 0;
		if (this->CurrentResource != unit.CurrentResource) {
			DropResource(unit);
			unit.CurrentResource = this->CurrentResource;
		}
		return 1;
	}

	goal = this->GetGoal();

	// Target is dead, stop getting resources.
	if (!goal || goal->IsVisibleAsGoal(*unit.Player) == false) {
		// Find an alternative, but don't look too far.
		this->goalPos.x = -1;
		this->goalPos.y = -1;
		if ((goal = UnitFindResource(unit, unit, 15, this->CurrentResource, unit.Player->AiEnabled))) {
			this->State = SUB_START_RESOURCE;
			this->SetGoal(goal);
		} else {
			this->ClearGoal();
			this->Finished = true;
		}
		return 0;
	}

	// FIXME: 0 can happen, if to near placed by map designer.
	Assert(unit.MapDistanceTo(*goal) <= 1);

	// Update the heading of a harvesting unit to looks straight at the resource.
	UnitHeadingFromDeltaXY(unit, goal->tilePos - unit.tilePos + goal->Type->GetHalfTileSize());

	// If resource is still under construction, wait!
	if ((goal->Type->MaxOnBoard && goal->Resource.Active >= goal->Type->MaxOnBoard)
		|| goal->CurrentAction() == UnitAction::Built) {
		// FIXME: Determine somehow when the resource will be free to use
		// FIXME: Could we somehow find another resource? Think minerals
		// FIXME: We should add a flag for that, and a limited range.
		// FIXME: Think minerals in st*rcr*ft!!
		// However the CPU usage is really low (no pathfinding stuff).
		unit.Wait = 10;
		return 0;
	}

	// Place unit inside the resource
	if (!resinfo.HarvestFromOutside) {
		if (goal->Variable[MAXHARVESTERS_INDEX].Value == 0 || goal->Variable[MAXHARVESTERS_INDEX].Value > goal->InsideUnits.size()) {
			this->ClearGoal();
			int selected = unit.Selected;
			unit.Remove(goal);
			if (selected && !Preference.DeselectInMine) {
				unit.Removed = 0;
				SelectUnit(unit);
				SelectionChanged();
				unit.Removed = 1;
			}
		} else if (goal->Variable[MAXHARVESTERS_INDEX].Value <= goal->InsideUnits.size()) {
			//Resource is full, wait
			unit.Wait = 10;
			return 0;
		}
	}

	if (this->CurrentResource != unit.CurrentResource) {
		DropResource(unit);
		unit.CurrentResource = this->CurrentResource;
	}

	// Activate the resource
	goal->Resource.Active++;

	if (resinfo.WaitAtResource) {
		this->TimeToHarvest = std::max<int>(1, resinfo.WaitAtResource * SPEEDUP_FACTOR / unit.Player->SpeedResourcesHarvest[resinfo.ResourceId]);
	} else {
		this->TimeToHarvest = 1;
	}
	this->DoneHarvesting = 0;
	return 1;
}

/**
**  Animate a unit that is harvesting
**
**  @param unit  Unit to animate
*/
static void AnimateActionHarvest(CUnit &unit)
{
	Assert(!unit.Type->Animations->Harvest[unit.CurrentResource].empty());
	UnitShowAnimation(unit, &unit.Type->Animations->Harvest[unit.CurrentResource]);
}

/**
**  Find something else to do when the resource is exhausted.
**  This is called from GatherResource when the resource is empty.
**
**  @param unit    pointer to harvester unit.
**  @param source  pointer to resource unit.
*/
void COrder_Resource::LoseResource(CUnit &unit, CUnit &source)
{
	CUnit *depot;
	const ResourceInfo &resinfo = *unit.Type->ResInfo[this->CurrentResource];

	Assert((unit.Container == &source && !resinfo.HarvestFromOutside)
		   || (!unit.Container && resinfo.HarvestFromOutside));

	if (resinfo.HarvestFromOutside) {
		this->ClearGoal();
		--source.Resource.Active;
	}

	// Continue to harvest if we aren't fully loaded
	if (resinfo.HarvestFromOutside && unit.ResourcesHeld < resinfo.ResourceCapacity) {
		CUnit *goal = UnitFindResource(unit, unit, 15, this->CurrentResource, 1);

		if (goal) {
			this->goalPos.x = -1;
			this->goalPos.y = -1;
			this->State = SUB_START_RESOURCE;
			this->SetGoal(goal);
			return;
		}
	}

	// If we are fully loaded first search for a depot.
	if (unit.ResourcesHeld && (depot = FindDeposit(unit, 1000, unit.CurrentResource))) {
		if (unit.Container) {
			DropOutNearest(unit, depot->tilePos + depot->Type->GetHalfTileSize(), &source);
		}
		// Remember where it mined, so it can look around for another resource.
		//
		//FIXME!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
		//unit.CurrentOrder()->Arg1.ResourcePos = (unit.X << 16) | unit.Y;
		this->DoneHarvesting = true;
		UnitGotoGoal(unit, depot, SUB_MOVE_TO_DEPOT);
		DebugPrint("%d: Worker %d report: Resource is exhausted, Going to depot\n",
		           unit.Player->Index,
		           UnitNumber(unit));
		return;
	}
	// No depot found, or harvester empty
	// Dump the unit outside and look for something to do.
	if (unit.Container) {
		Assert(!resinfo.HarvestFromOutside);
		DropOutOnSide(unit, LookingW, &source);
	}
	this->goalPos.x = -1;
	this->goalPos.y = -1;
	//use depot as goal
	depot = UnitFindResource(unit, unit, 15, this->CurrentResource, unit.Player->AiEnabled);
	if (depot) {
		DebugPrint("%d: Worker %d report: Resource is exhausted, Found another resource.\n",
		           unit.Player->Index,
		           UnitNumber(unit));
		this->State = SUB_START_RESOURCE;
		this->SetGoal(depot);
	} else {
		DebugPrint("%d: Worker %d report: Resource is exhausted, Just sits around confused.\n",
		           unit.Player->Index,
		           UnitNumber(unit));
		this->Finished = true;
	}
}



/**
**  Gather the resource
**
**  @param unit  Pointer to unit.
**
**  @return      non-zero if ready, otherwise zero.
*/
int COrder_Resource::GatherResource(CUnit &unit)
{
	CUnit *source = 0;
	const ResourceInfo &resinfo = *unit.Type->ResInfo[this->CurrentResource];
	int addload;

	if (resinfo.HarvestFromOutside || resinfo.TerrainHarvester) {
		AnimateActionHarvest(unit);
	} else {
		unit.Anim.CurrAnim = nullptr;
	}

	this->TimeToHarvest--;

	if (this->DoneHarvesting) {
		Assert(resinfo.HarvestFromOutside || resinfo.TerrainHarvester);
		return !unit.Anim.Unbreakable;
	}

	// Target gone?
	if (resinfo.TerrainHarvester && !Map.Field(this->goalPos)->IsTerrainResourceOnMap(this->CurrentResource)) {
		if (!unit.Anim.Unbreakable) {
			// Action now breakable, move to resource again.
			this->State = SUB_MOVE_TO_RESOURCE;
			// Give it some reasonable look while searching.
			// FIXME: which frame?
			unit.Frame = 0;
		}
		return 0;
		// No wood? Freeze!!!
	}

	while (!this->DoneHarvesting && this->TimeToHarvest < 0) {
		//FIXME: rb - how should it look for WaitAtResource == 0
		if (resinfo.WaitAtResource) {
			this->TimeToHarvest += std::max<int>(1, resinfo.WaitAtResource * SPEEDUP_FACTOR / unit.Player->SpeedResourcesHarvest[resinfo.ResourceId]);
		} else {
			this->TimeToHarvest += 1;
		}

		// Calculate how much we can load.
		if (resinfo.ResourceStep) {
			addload = resinfo.ResourceStep;
		} else {
			addload = resinfo.ResourceCapacity;
		}
		// Make sure we don't bite more than we can chew.
		if (unit.ResourcesHeld + addload > resinfo.ResourceCapacity) {
			addload = resinfo.ResourceCapacity - unit.ResourcesHeld;
		}

		if (resinfo.TerrainHarvester) {
			CMapField *mf = Map.Field(this->goalPos);
			addload = std::min((int)mf->Value, addload);
			unit.ResourcesHeld += addload;
			mf->Value -= addload;
			if (mf->Value == 0) {
				Map.ClearTile(this->goalPos);
				this->DoneHarvesting = true;
			} else if (unit.ResourcesHeld == resinfo.ResourceCapacity) {
				// Mark as complete.
				this->DoneHarvesting = true;
			}
			return 0;
		} else {
			if (resinfo.HarvestFromOutside) {
				source = this->GetGoal();
			} else {
				source = unit.Container;
			}

			Assert(source);
			Assert(source->ResourcesHeld <= 655350);
			bool is_visible = source->IsVisibleAsGoal(*unit.Player);
			// Target is not dead, getting resources.
			if (is_visible) {
				// Don't load more that there is.
				addload = std::min(source->ResourcesHeld, addload);
				unit.ResourcesHeld += addload;
				source->ResourcesHeld -= addload;
			}

			// End of resource: destroy the resource.
			// FIXME: implement depleted resources.
			if ((!is_visible) || (source->ResourcesHeld == 0)) {
				if (unit.Anim.Unbreakable) {
					return 0;
				}
				DebugPrint("%d: Worker %d report: Resource is destroyed\n",
				           unit.Player->Index,
				           UnitNumber(unit));
				bool dead = source->IsAlive() == false;

				// Improved version of DropOutAll that makes workers go to the depot.
				LoseResource(unit, *source);
				for (CUnit *uins = source->Resource.Workers;
					 uins; uins = uins->NextWorker) {
					if (uins != &unit && uins->CurrentOrder()->Action == UnitAction::Resource) {
						COrder_Resource &order = *static_cast<COrder_Resource *>(uins->CurrentOrder());
						if (!uins->Anim.Unbreakable && order.State == SUB_GATHER_RESOURCE) {
							order.LoseResource(*uins, *source);
						}
					}
				}
				// Don't destroy the resource twice.
				// This only happens when it's empty.
				if (!dead) {
					if (Preference.MineNotifications && unit.Player->Index == ThisPlayer->Index
					    && source->Variable[GIVERESOURCE_INDEX].Max > DefaultIncomes[this->CurrentResource]) {
						unit.Player->Notify(ColorYellow,
						                    source->tilePos,
						                    _("%s has collapsed!"),
						                    source->Type->Name.c_str());
					}
					LetUnitDie(*source);
					// FIXME: make the workers inside look for a new resource.
				}
				source = nullptr;
				return 0;
			}
			if (resinfo.HarvestFromOutside) {
				if ((unit.ResourcesHeld == resinfo.ResourceCapacity) || (source == nullptr)) {
					// Mark as complete.
					this->DoneHarvesting = true;
				}
				return 0;
			} else {
				return unit.ResourcesHeld == resinfo.ResourceCapacity && source;
			}
		}
	}
	return 0;
}

int GetNumWaitingWorkers(const CUnit &mine)
{
	int ret = 0;
	CUnit *worker = mine.Resource.Workers;

	for (int i = 0; nullptr != worker; worker = worker->NextWorker, ++i) {
		Assert(worker->CurrentAction() == UnitAction::Resource);
		COrder_Resource &order = *static_cast<COrder_Resource *>(worker->CurrentOrder());

		if (order.IsGatheringWaiting()) {
			ret++;
		}
		Assert(i <= mine.Resource.Assigned);
	}
	return ret;
}

/**
**  Stop gathering from the resource, go home.
**
**  @param unit  Poiner to unit.
**
**  @return      TRUE if ready, otherwise FALSE.
*/
int COrder_Resource::StopGathering(CUnit &unit)
{
	CUnit *source = 0;
	const ResourceInfo &resinfo = *unit.Type->ResInfo[this->CurrentResource];

	if (!resinfo.TerrainHarvester) {
		if (resinfo.HarvestFromOutside) {
			source = this->GetGoal();
			this->ClearGoal();
		} else {
			source = unit.Container;
		}
		source->Resource.Active--;
		Assert(source->Resource.Active >= 0);
		//Store resource position.
		this->Resource.Mine = source;

		if (Preference.MineNotifications && unit.Player->Index == ThisPlayer->Index
			&& source->IsAlive()
			&& !source->MineLow
			&& source->ResourcesHeld * 100 / source->Variable[GIVERESOURCE_INDEX].Max <= 10
			&& source->Variable[GIVERESOURCE_INDEX].Max > DefaultIncomes[this->CurrentResource]) {
			unit.Player->Notify(
				ColorYellow, source->tilePos, _("%s is running low!"), source->Type->Name.c_str());
			source->MineLow = 1;
		}

		if (source->Type->MaxOnBoard) {
			int count = 0;
			CUnit *worker = source->Resource.Workers;
			CUnit *next = nullptr;
			for (; nullptr != worker; worker = worker->NextWorker) {
				Assert(worker->CurrentAction() == UnitAction::Resource);
				COrder_Resource &order = *static_cast<COrder_Resource *>(worker->CurrentOrder());
				if (worker != &unit && order.IsGatheringWaiting()) {
					count++;
					if (next) {
						if (next->Wait > worker->Wait) {
							next = worker;
						}
					} else {
						next = worker;
					}
				}
			}
			if (next) {
				if (!unit.Player->AiEnabled) {
					DebugPrint("%d: Worker %d report: Unfreeze resource gathering of %d <Wait %d> "
					           "on %d [Assigned: %d Waiting %d].\n",
					           unit.Player->Index,
					           UnitNumber(unit),
					           UnitNumber(*next),
					           next->Wait,
					           UnitNumber(*source),
					           source->Resource.Assigned,
					           count);
				}
				next->Wait = 0;
				//source->Data.Resource.Waiting = count - 1;
				//Assert(source->Data.Resource.Assigned >= source->Data.Resource.Waiting);
				//StartGathering(next);
			}
		}
	} else {
		// Store resource position.
		this->Resource.Pos = unit.tilePos;
		Assert(this->Resource.Mine == nullptr);
	}

#ifdef DEBUG
	if (!unit.ResourcesHeld) {
		DebugPrint("Unit %d is empty???\n", UnitNumber(unit));
	}
#endif

	// Find and send to resource deposit.
	CUnit *depot = FindDeposit(unit, 1000, unit.CurrentResource);
	// There's a bug in the traversal that leads to workers "sometimes" not finding their way to the old depot.
	// timfel: of course, maybe it's actually nice that workers drop out towards their last depot...
	if (!depot && (!(resinfo.HarvestFromOutside || resinfo.TerrainHarvester)) && Depot && Depot->IsAlive()) {
		Assert(unit.Container);
		DropOutNearest(unit, Depot->tilePos + Depot->Type->GetHalfTileSize(), source);
	}
	Depot = depot;
	if (!depot || !unit.ResourcesHeld || this->Finished) {
		if (!(resinfo.HarvestFromOutside || resinfo.TerrainHarvester)) {
			if (unit.Container) {
				DropOutOnSide(unit, LookingW, source);
			}
		}
		CUnit *mine = this->Resource.Mine;

		if (mine) {
			unit.DeAssignWorkerFromMine(*mine);
			this->Resource.Mine = nullptr;
		}

		DebugPrint("%d: Worker %d report: Can't find a resource [%d] deposit.\n",
		           unit.Player->Index,
		           UnitNumber(unit),
		           unit.CurrentResource);
		this->Finished = true;
		return 0;
	} else {
		if (!(resinfo.HarvestFromOutside || resinfo.TerrainHarvester)) {
			if (unit.Container) {
				// may have dropped out above
				DropOutNearest(unit, depot->tilePos + depot->Type->GetHalfTileSize(), source);
			}
		}
		UnitGotoGoal(unit, depot, SUB_MOVE_TO_DEPOT);
	}
	if (IsOnlySelected(unit)) {
		SelectedUnitChanged();
	}
#if 1
	return 1;
#endif
}

/**
**  Move to resource depot
**
**  @param unit  Pointer to unit.
**
**  @return      TRUE if reached, otherwise FALSE.
*/
int COrder_Resource::MoveToDepot(CUnit &unit)
{
	const ResourceInfo &resinfo = *unit.Type->ResInfo[this->CurrentResource];
	Assert(this->GetGoal());
	CUnit &goal = *this->GetGoal();
	CPlayer &player = *unit.Player;

	switch (DoActionMove(unit)) { // reached end-point?
		case PF_UNREACHABLE:
			return -1;
		case PF_REACHED:
			break;
		case PF_WAIT:
			if (unit.Player->AiEnabled) {
				this->Range++;
				if (this->Range >= 5) {
					this->Range = 0;
					AiCanNotMove(unit);
				}
			}
		default:
			if (unit.Anim.Unbreakable || goal.IsVisibleAsGoal(player)) {
				return 0;
			}
			break;
	}

	//
	// Target is dead, stop getting resources.
	//
	if (!goal.IsVisibleAsGoal(player)) {
		DebugPrint("%d: Worker %d report: Destroyed depot\n", player.Index, UnitNumber(unit));

		unit.CurrentOrder()->ClearGoal();

		CUnit *depot = FindDeposit(unit, 1000, unit.CurrentResource);

		if (depot) {
			UnitGotoGoal(unit, depot, SUB_MOVE_TO_DEPOT);
			DebugPrint(
				"%d: Worker %d report: Going to new deposit.\n", player.Index, UnitNumber(unit));
		} else {
			DebugPrint("%d: Worker %d report: Can't find a new resource deposit.\n",
			           player.Index,
			           UnitNumber(unit));

			// FIXME: perhaps we should choose an alternative
			this->Finished = true;
		}
		return 0;
	}

	// If resource depot is still under construction, wait!
	if (goal.CurrentAction() == UnitAction::Built) {
		unit.Wait = 10;
		return 0;
	}

	this->ClearGoal();
	unit.Wait = resinfo.WaitAtDepot;

	// Place unit inside the depot
	if (unit.Wait) {
		int selected = unit.Selected;
		unit.Remove(&goal);
		if (selected && !Preference.DeselectInMine) {
			unit.Removed = 0;
			SelectUnit(unit);
			SelectionChanged();
			unit.Removed = 1;
		}
		unit.Anim.CurrAnim = nullptr;
	}

	// Update resource.
	const int rindex = resinfo.FinalResource;
	player.ChangeResource(rindex, (unit.ResourcesHeld * player.Incomes[rindex]) / 100, true);
	player.TotalResources[rindex] += (unit.ResourcesHeld * player.Incomes[rindex]) / 100;
	unit.ResourcesHeld = 0;
	unit.CurrentResource = 0;

	if (unit.Wait) {
		unit.Wait /= std::max(1, unit.Player->SpeedResourcesReturn[resinfo.ResourceId] / SPEEDUP_FACTOR);
		if (unit.Wait) {
			unit.Wait--;
		}
	}
	return 1;
}

/**
**  Wait in depot, for the resources stored.
**
**  @param unit  Pointer to unit.
**
**  @return      TRUE if ready, otherwise FALSE.
*/
bool COrder_Resource::WaitInDepot(CUnit &unit)
{
	const ResourceInfo &resinfo = *unit.Type->ResInfo[this->CurrentResource];
	const CUnit *depot = ResourceDepositOnMap(unit.tilePos, resinfo.ResourceId);

	// Range hardcoded. don't stray too far though
	if (resinfo.TerrainHarvester) {
		if (auto pos = FindTerrainType(unit.Type->MovementMask, MapFieldForest, 10, *unit.Player, this->Resource.Pos)) {
			if (depot) {
				DropOutNearest(unit, *pos, depot);
			}
			this->goalPos = *pos;
			return true;
		} else {
			if (depot) {
				DropOutOnSide(unit, LookingW, depot);
			}
			this->Finished = true;
			return false;
		}
	} else {
		/// FIXME: Make it customizable
		const unsigned int tooManyWorkers = 15;

		CUnit *mine = this->Resource.Mine;
		CUnit *newdepot = nullptr;
		CUnit *newMine = nullptr;
		const bool longWay = unit.pathFinderData->output.Cycles > 500;

		if (unit.Player->AiEnabled && AiPlayer && AiPlayer->BuildDepots) {
			// If the depot is overused, we need first to try to switch into another depot
			// Use depot's ref counter for that
			if (longWay || !mine || (depot->Refs > tooManyWorkers)) {
				std::tie(newdepot, newMine) = AiGetSuitableDepot(unit, *depot);
				if (newdepot == nullptr && longWay) {
					// We need a new depot
					AiNewDepotRequest(unit);
				}
			}
		}

		// If newMine is not nullptr, then we got it in AiGetSuitableDepot
		if (!newMine) {
			if (mine != nullptr && mine->IsAlive()) {
				newMine = mine;
			} else {
				const CUnit *start_unit = nullptr;
				if (newdepot != nullptr) {
					start_unit = newdepot;
				} else if (depot != nullptr) {
					start_unit = depot;
				}
				newMine = UnitFindResource(unit,
				                           (start_unit ? *start_unit : unit),
				                           1000,
				                           this->CurrentResource,
				                           unit.Player->AiEnabled,
				                           (newdepot ? newdepot : depot));
			}
		}

		if (newMine) {
			if (depot) {
				DropOutNearest(unit, newMine->tilePos + newMine->Type->GetHalfTileSize(), depot);
			}

			if (newMine != mine) {
				if (mine) {
					unit.DeAssignWorkerFromMine(*mine);
				}
				unit.AssignWorkerToMine(*newMine);
				this->Resource.Mine = newMine;
			}
			this->SetGoal(newMine);
			this->goalPos.x = this->goalPos.y = -1;
		} else {
#ifdef DEBUG
			const Vec2i &pos = mine ? mine->tilePos : unit.tilePos;
			DebugPrint("%d: Worker %d report: [%d,%d] Resource gone near [%d,%d]. Sit "
			           "and play dumb.\n",
			           unit.Player->Index,
			           UnitNumber(unit),
			           unit.tilePos.x,
			           unit.tilePos.y,
			           pos.x,
			           pos.y);
#endif // DEBUG
			if (depot) {
				DropOutOnSide(unit, LookingW, depot);
			}
			if (mine) {
				unit.DeAssignWorkerFromMine(*mine);
				this->Resource.Mine = nullptr;
			}
			this->Finished = true;
			return false;
		}
	}
	return true;
}

void COrder_Resource::DropResource(CUnit &unit)
{
	if (unit.CurrentResource) {
		const ResourceInfo &resinfo = *unit.Type->ResInfo[unit.CurrentResource];

		if (!resinfo.TerrainHarvester) {
			CUnit *mine = this->Resource.Mine;
			if (mine) {
				unit.DeAssignWorkerFromMine(*mine);
			}
		}
		//fast clean both resource data: pos and mine
		this->Resource.Mine = nullptr;
		unit.CurrentResource = 0;
		unit.ResourcesHeld = 0;
	}
}

/**
**  Give up on gathering.
**
**  @param unit  Pointer to unit.
*/
void COrder_Resource::ResourceGiveUp(CUnit &unit)
{
	DebugPrint("%d: Worker %d report: Gave up on resource gathering.\n",
	           unit.Player->Index,
	           UnitNumber(unit));
	if (this->HasGoal()) {
		DropResource(unit);
		this->ClearGoal();
	}
	this->Finished = true;
}

/**
**  Try to find another resource before give up
**
**  return false if failed, true otherwise.
*/

bool COrder_Resource::FindAnotherResource(CUnit &unit)
{
	if (!this->CurrentResource) {
		return false;
	}
	const auto &resinfo = unit.Type->ResInfo[this->CurrentResource];
	if (!resinfo) {
		return false;
	}
	if (resinfo->TerrainHarvester) {
		if (auto resPos = FindTerrainType(unit.Type->MovementMask, MapFieldForest, 8, *unit.Player, unit.tilePos)) {
			this->goalPos = *resPos;
			this->State = SUB_MOVE_TO_RESOURCE;
			DebugPrint("Found a better place to harvest %d,%d\n", resPos->x, resPos->y);
			return true;
		}
		return false;
	} else {
		CUnit *newGoal = UnitFindResource(unit, this->Resource.Mine ? *this->Resource.Mine : unit, 8, this->CurrentResource, 1);

		if (newGoal) {
			CUnit *mine = this->Resource.Mine;
			if (mine) {
				unit.DeAssignWorkerFromMine(*mine);
			}
			unit.AssignWorkerToMine(*newGoal);
			this->Resource.Mine = newGoal;
			this->goalPos.x = -1;
			this->goalPos.y = -1;
			this->State = SUB_MOVE_TO_RESOURCE;
			this->SetGoal(newGoal);
			return true;
		}
	}
	return false;
}


/**
**  Initialize
**
**  return false if action is canceled, true otherwise.
*/
bool COrder_Resource::ActionResourceInit(CUnit &unit)
{
	Assert(this->State == SUB_START_RESOURCE);

	this->Range = 0;
	CUnit *const goal = this->GetGoal();
	CUnit *mine = this->Resource.Mine;

	if (mine) {
		unit.DeAssignWorkerFromMine(*mine);
		this->Resource.Mine = nullptr;
	}
	if (goal && goal->IsAlive() == false) {
		return false;
	}
	if (goal && goal->CurrentAction() != UnitAction::Built) {
		unit.AssignWorkerToMine(*goal);
		this->Resource.Mine = goal;
	}

	UnitGotoGoal(unit, goal, SUB_MOVE_TO_RESOURCE);
	return true;
}

/**
**  Control the unit action: getting a resource.
**
**  This the generic function for oil, gold, ...
**
**  @param unit  Pointer to unit.
*/
void COrder_Resource::Execute(CUnit &unit)
{
	// can be different by Cloning (trained unit)...
	this->worker = &unit;

	if (IsWaiting(unit)) {
		return;
	}
	StopWaiting(unit);

	// Let's start mining.
	if (this->State == SUB_START_RESOURCE) {
		if (ActionResourceInit(unit) == false) {
			ResourceGiveUp(unit);
			return;
		}
	}

	// Move to the resource location.
	if (SUB_MOVE_TO_RESOURCE <= this->State && this->State < SUB_UNREACHABLE_RESOURCE) {
		const int ret = MoveToResource(unit);

		switch (ret) {
			case -1: { // Can't Reach
				this->State++;
				unit.Wait = 5;
				return;
			}
			case 1: { // Reached
				this->State = SUB_START_GATHERING;
				break;
			}
			case 0: // Move along.
				return;
			default: {
				Assert(0);
				break;
			}
		}
	}

	// Resource seems to be unreachable
	if (this->State == SUB_UNREACHABLE_RESOURCE) {
		if (this->FindAnotherResource(unit) == false) {
			ResourceGiveUp(unit);
			return;
		}
	}

	// Start gathering the resource
	if (this->State == SUB_START_GATHERING) {
		if (StartGathering(unit)) {
			this->State = SUB_GATHER_RESOURCE;
		} else {
			return;
		}
	}

	// Gather the resource.
	if (this->State == SUB_GATHER_RESOURCE) {
		if (GatherResource(unit)) {
			this->State = SUB_STOP_GATHERING;
		} else {
			return;
		}
	}

	// Stop gathering the resource.
	if (this->State == SUB_STOP_GATHERING) {
		if (StopGathering(unit)) {
			this->State = SUB_MOVE_TO_DEPOT;
			unit.pathFinderData->output.Cycles = 0; //moving counter
		} else {
			return;
		}
	}

	// Move back home.
	if (SUB_MOVE_TO_DEPOT <= this->State && this->State < SUB_UNREACHABLE_DEPOT) {
		const int ret = MoveToDepot(unit);

		switch (ret) {
			case -1: { // Can't Reach
				this->State++;
				unit.Wait = 5;
				return;
			}
			case 1: { // Reached
				this->State = SUB_RETURN_RESOURCE;
				return;
			}
			case 0: // Move along.
				return;
			default: {
				Assert(0);
				return;
			}
		}
	}

	// Depot seems to be unreachable
	if (this->State == SUB_UNREACHABLE_DEPOT) {
		ResourceGiveUp(unit);
		return;
	}

	// Unload resources at the depot.
	if (this->State == SUB_RETURN_RESOURCE) {
		if (WaitInDepot(unit)) {
			this->State = SUB_START_RESOURCE;

			// It's posible, though very rare that the unit's goal blows up
			// this cycle, but after this unit. Thus, next frame the unit
			// will start mining a destroyed site. If, on the otherhand we
			// are already in SUB_MOVE_TO_RESOURCE then we can handle it.
			// So, we pass through SUB_START_RESOURCE the very instant it
			// goes out of the depot.
			//HandleActionResource(order, unit);
		}
	}
}

/**
**  Get goal position
*/
const Vec2i COrder_Resource::GetGoalPos() const /* override */
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
