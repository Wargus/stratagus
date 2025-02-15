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
/**@name ai_plan.cpp - AI planning functions. */
//
//      (c) Copyright 2002-2005 by Lutz Sammer and Jimmy Salmon
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

#include "ai_local.h"

#include "actions.h"
#include "commands.h"
#include "map.h"
#include "missile.h"
#include "pathfinder.h"
#include "tileset.h"
#include "unit.h"
#include "unit_find.h"
#include "unittype.h"

/*----------------------------------------------------------------------------
--  Variables
----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
--  Functions
----------------------------------------------------------------------------*/

namespace {

/**
**  Choose enemy on map tile.
**
**  @param source  Unit which want to attack.
**  @param pos     position on map, tile-based.
**
**  @return        Returns ideal target on map tile.
*/
CUnit *EnemyOnMapTile(const CUnit &source, const Vec2i &pos)
{
	auto units = Map.Field(pos)->UnitCache;
	ranges::erase_if(units, [&](const CUnit *unit) {
		const CUnitType &type = *unit->Type;
		// unusable unit ?
		// if (unit->IsUnusable()) can't attack constructions
		// FIXME: did SelectUnitsOnTile already filter this?
		// Invisible and not Visible
		return (unit->Removed
		        || unit->Variable[INVISIBLE_INDEX].Value
		        // || (!UnitVisible(unit, source->Player))
		        || unit->CurrentAction() == UnitAction::Die
		        || (unit->Type->MoveType == EMovement::Fly && unit->IsAggressive() == false)
		        || unit->MapDistanceTo(pos) != 0 || !CanTarget(*source.Type, type)
		        || !source.Player->IsEnemy(*unit) // a friend or neutral
		);
	});
	auto it = ranges::max_element(units, std::less<>{}, [](const CUnit *unit) {
		return unit->Variable[PRIORITY_INDEX].Value;
	});
	return it != units.end() ? *it : nullptr;
}

class WallFinder
{
public:
	friend TerrainTraversal;

	static std::optional<Vec2i> find(const CUnit &unit, int range)
	{
		TerrainTraversal terrainTraversal;

		terrainTraversal.SetSize(Map.Info.MapWidth, Map.Info.MapHeight);
		terrainTraversal.Init();

		terrainTraversal.PushUnitPosAndNeighboor(unit);

		WallFinder wallFinder(unit, range);

		return terrainTraversal.Run(wallFinder) ? std::make_optional(wallFinder.resultPos)
		                                        : std::nullopt;
	}

private:
	WallFinder(const CUnit &unit, int maxDist) :
		maxDist(maxDist),
		movemask(unit.Type->MovementMask & ~(MapFieldLandUnit | MapFieldAirUnit | MapFieldSeaUnit))
	{}
	VisitResult Visit(TerrainTraversal &terrainTraversal, const Vec2i &pos, const Vec2i &from);
private:
	int maxDist;
	unsigned int movemask;
	Vec2i resultPos{-1, -1};
};

VisitResult WallFinder::Visit(TerrainTraversal &terrainTraversal, const Vec2i &pos, const Vec2i &from)
{
#if 0
	if (!unit.Player->AiEnabled && !Map.IsFieldExplored(*unit.Player, pos)) {
		return VisitResult_DeadEnd;
	}
#endif
	// Look if found what was required.
	if (Map.WallOnMap(pos)) {
		DebugPrint("Wall found %d, %d\n", pos.x, pos.y);
		resultPos = from;
		return VisitResult::Finished;
	}
	if (Map.Field(pos)->CheckMask(movemask)) { // reachable
		if (terrainTraversal.Get(pos) <= maxDist) {
			return VisitResult::Ok;
		} else {
			return VisitResult::DeadEnd;
		}
	} else { // unreachable
		return VisitResult::DeadEnd;
	}
}

}

/**
**  Find possible walls to target.
**
**  @param force  Attack force.
**
**  @return       True if wall found.
*/
bool AiFindWall(AiForce &force)
{
	// Find a unit to use.  Best choice is a land unit with range 1.
	// Next best choice is any land unit.  Otherwise just use the first.
	CUnit *unit = force.Units[0];
	for (CUnit *aiunit : force.Units) {
		if (aiunit->Type->MoveType == EMovement::Land) {
			unit = aiunit;
			if (aiunit->Type->Missile.Missile->Range == 1) {
				break;
			}
		}
	}
	const int maxRange = 1000;
	if (auto wallPos = WallFinder::find(*unit, maxRange)) {
		force.State = AiForceAttackingState::Waiting;
		for (CUnit *aiunit : force.Units) {
			if (aiunit->Type->CanAttack) {
				CommandAttack(*aiunit, *wallPos, nullptr, EFlushMode::On);
			} else {
				CommandMove(*aiunit, *wallPos, EFlushMode::On);
			}
		}
		return true;
	}
	return false;
}

namespace
{

class ReachableTerrainMarker
{
public:
	ReachableTerrainMarker(const CUnit &unit) :
		movemask(unit.Type->MovementMask & ~(MapFieldLandUnit | MapFieldAirUnit | MapFieldSeaUnit))
	{}
	VisitResult Visit(TerrainTraversal &terrainTraversal, const Vec2i &pos, const Vec2i &from);

private:
	unsigned int movemask;
};


VisitResult ReachableTerrainMarker::Visit(TerrainTraversal &terrainTraversal, const Vec2i &pos, const Vec2i &from)
{
#if 0
	if (!player.AiEnabled && !Map.IsFieldExplored(player, pos)) {
		return VisitResult::DeadEnd;
	}
#endif
	if (CanMoveToMask(pos, movemask)) { // reachable
		return VisitResult::Ok;
	} else { // unreachable
		return VisitResult::DeadEnd;
	}
}

} // namespace

static void MarkReacheableTerrainType(const CUnit &unit, TerrainTraversal &terrainTraversal)
{
	terrainTraversal.PushUnitPosAndNeighboor(unit);

	ReachableTerrainMarker reachableTerrainMarker(unit);

	terrainTraversal.Run(reachableTerrainMarker);
}

class EnemyFinderWithTransporter
{
public:
	friend TerrainTraversal;

	static std::optional<Vec2i> find(const CUnit &unit, const TerrainTraversal &terrainTransporter)
	{
		TerrainTraversal terrainTraversal;

		terrainTraversal.SetSize(Map.Info.MapWidth, Map.Info.MapHeight);
		terrainTraversal.Init();

		terrainTraversal.PushUnitPosAndNeighboor(unit);

		EnemyFinderWithTransporter enemyFinderWithTransporter(unit, terrainTransporter);

		return terrainTraversal.Run(enemyFinderWithTransporter)
		         ? std::make_optional(enemyFinderWithTransporter.resultPos)
		         : std::nullopt;
	}

private:
	EnemyFinderWithTransporter(const CUnit &unit, const TerrainTraversal &terrainTransporter) :
		unit(unit),
		terrainTransporter(terrainTransporter),
		movemask(unit.Type->MovementMask & ~(MapFieldLandUnit | MapFieldAirUnit | MapFieldSeaUnit))
	{}
	VisitResult Visit(TerrainTraversal &terrainTraversal, const Vec2i &pos, const Vec2i &from);
private:
	bool IsAccessibleForTransporter(const Vec2i &pos) const {return terrainTransporter.IsReached(pos);}

private:
	const CUnit &unit;
	const TerrainTraversal &terrainTransporter;
	unsigned int movemask;
	Vec2i resultPos{-1, -1};
};

VisitResult EnemyFinderWithTransporter::Visit(TerrainTraversal &terrainTraversal, const Vec2i &pos, const Vec2i &from)
{
#if 0
	if (!player.AiEnabled && !Map.IsFieldExplored(player, pos)) {
		return VisitResult::DeadEnd;
	}
#endif
	if (EnemyOnMapTile(unit, pos) && CanMoveToMask(from, movemask)) {
		DebugPrint("Target found %d,%d\n", pos.x, pos.y);
		resultPos = pos;
		return VisitResult::Finished;
	}
	if (CanMoveToMask(pos, movemask) || IsAccessibleForTransporter(pos)) { // reachable
		return VisitResult::Ok;
	} else { // unreachable
		return VisitResult::DeadEnd;
	}
}

class IsAFreeTransporter
{
public:
	bool operator()(const CUnit *unit) const
	{
		return unit->Type->CanMove() && unit->BoardCount < unit->Type->MaxOnBoard;
	}
};

template <typename ITERATOR>
int GetTotalBoardCapacity(ITERATOR begin, ITERATOR end)
{
	int totalBoardCapacity = 0;
	IsAFreeTransporter isAFreeTransporter;

	for (ITERATOR it = begin; it != end; ++it) {
		const CUnit *unit = *it;

		if (isAFreeTransporter(unit)) {
			totalBoardCapacity += unit->Type->MaxOnBoard - unit->BoardCount;
		}
	}
	return totalBoardCapacity;
}

/**
**  Plan an attack with a force.
**  We know, that we must use a transporter.
**
**  @return       True if target found, false otherwise.
**
**  @todo         Perfect planning.
**                Only works for water transporter!
**  @todo transporter are more selective now (flag with unittypeland).
**         We must manage it.
*/
bool AiForce::PlanAttack()
{
	CPlayer &player = *AiPlayer->Player;
	DebugPrint("%d: Planning for force #%lu of player #%d\n",
	           player.Index,
	           (long unsigned int) (this - &(AiPlayer->Force[0])),
	           player.Index);

	TerrainTraversal transporterTerrainTraversal;

	transporterTerrainTraversal.SetSize(Map.Info.MapWidth, Map.Info.MapHeight);
	transporterTerrainTraversal.Init();

	CUnit *transporter = nullptr;

	if (auto transporterIt = ranges::find_if(Units, IsAFreeTransporter());
	    transporterIt != Units.end()) {
		transporter = *transporterIt;
		DebugPrint("%d: Transporter #%d\n", player.Index, UnitNumber(*transporter));
		MarkReacheableTerrainType(*transporter, transporterTerrainTraversal);
	} else {
		auto it = ranges::find_if(player.GetUnits(), IsAFreeTransporter());
		if (it != player.GetUnits().end()) {
			transporter = *it;
			MarkReacheableTerrainType(*transporter, transporterTerrainTraversal);
		} else {
			DebugPrint("%d: No transporter available\n", player.Index);
			return false;
		}
	}

	// Find a land unit of the force.
	// FIXME: if force is split over different places -> broken
	CUnit *landUnit = nullptr;
	if (auto it = ranges::find_if(Units, CUnitTypeFinder(EMovement::Land)); it == Units.end()) {
		DebugPrint("%d: No land unit in force\n", player.Index);
		return false;
	} else {
		landUnit = *it;
	}

	if (auto pos = EnemyFinderWithTransporter::find(*landUnit, transporterTerrainTraversal)) {
		const unsigned int forceIndex = AiPlayer->Force.getIndex(*this) + 1;

		if (transporter->GroupId != forceIndex) {
			DebugPrint("%d: Assign any transporter #%d\n", player.Index, UnitNumber(*transporter));

			if (transporter->GroupId) {
				transporter->Player->Ai->Force[transporter->GroupId - 1].Remove(*transporter);
			}
			Insert(*transporter);
			transporter->GroupId = forceIndex;
		}

		int totalBoardCapacity = GetTotalBoardCapacity(Units.begin(), Units.end());

		// Verify we have enough transporter.
		// @note: Minimal check for unitType (flyers...)
		for (const CUnit *unit : Units) {
			if (CanTransport(*transporter, *unit)) {
				totalBoardCapacity -= unit->Type->BoardSize;
			}
		}
		if (totalBoardCapacity < 0) { // Not enough transporter.
			IsAFreeTransporter isAFreeTransporter;
			// Add all other idle transporter.
			for (CUnit *unit : player.GetUnits()) {
				if (isAFreeTransporter(unit) && unit->GroupId == 0 && unit->IsIdle()) {
					DebugPrint("%d: Assign any transporter #%d\n", player.Index, UnitNumber(*unit));
					Insert(*unit);
					unit->GroupId = forceIndex;
					totalBoardCapacity += unit->Type->MaxOnBoard - unit->BoardCount;
					if (totalBoardCapacity >= 0) {
						break;
					}
				}
			}
		}
		DebugPrint("%d: Can attack\n", player.Index);
		GoalPos = *pos;
		State = AiForceAttackingState::Boarding;
		return true;
	}
	return false;
}

static std::optional<Vec2i> ChooseRandomUnexploredPositionNear(const Vec2i &center)
{
	int ray = 3;
	const int maxTryCount = 8;
	for (int i = 0; i != maxTryCount; ++i) {
		Vec2i pos;
		pos.x = center.x + SyncRand() % (2 * ray + 1) - ray;
		pos.y = center.y + SyncRand() % (2 * ray + 1) - ray;

		if (Map.Info.IsPointOnMap(pos)
			&& Map.Field(pos)->playerInfo.IsExplored(*AiPlayer->Player) == false) {
			return pos;
		}
		ray = 3 * ray / 2;
	}
	return std::nullopt;
}

static std::pair<CUnit *, Vec2i> GetBestExplorer(const AiExplorationRequest &request)
{
	// Choose a target, "near"
	const Vec2i &center = request.pos;
	auto pos = ChooseRandomUnexploredPositionNear(center);
	if (!pos) {
		return {nullptr, {-1, -1}};
	}
	// We have an unexplored tile in sight (pos)

	CUnit *bestunit = nullptr;
	// Find an idle unit, responding to the mask
	bool flyeronly = false;
	int bestSquareDistance = -1;
	for (CUnit *unit : AiPlayer->Player->GetUnits()) {
		if (!unit->IsIdle()) {
			continue;
		}
		if (Map.Info.IsPointOnMap(unit->tilePos) == false) {
			continue;
		}
		if (unit->CanMove() == false) {
			continue;
		}
		const CUnitType &type = *unit->Type;

		if (type.MoveType != EMovement::Fly) {
			if (flyeronly) {
				continue;
			}
			if ((request.Mask & MapFieldLandUnit) && type.MoveType != EMovement::Land) {
				continue;
			}
			if ((request.Mask & MapFieldSeaUnit) && type.MoveType != EMovement::Naval) {
				continue;
			}
		} else {
			flyeronly = true;
		}

		const int sqDistance = SquareDistance(unit->tilePos, *pos);
		if (bestSquareDistance == -1 || sqDistance <= bestSquareDistance
		    || (bestunit->Type->MoveType != EMovement::Fly && type.MoveType == EMovement::Fly)) {
			bestSquareDistance = sqDistance;
			bestunit = unit;
		}
	}
	return {bestunit, *pos};
}


/**
**  Respond to ExplorationRequests
*/
void AiSendExplorers()
{
	// Count requests...
	const int requestcount = AiPlayer->FirstExplorationRequest.size();

	// Nothing => abort
	if (!requestcount) {
		return;
	}
	const int maxTryCount = 5;
	for (int i = 0; i != maxTryCount; ++i) {
		// Choose a request
		const int requestid = SyncRand() % requestcount;
		const AiExplorationRequest &request = AiPlayer->FirstExplorationRequest[requestid];

		const auto& [bestunit, pos] = GetBestExplorer(request);
		if (bestunit != nullptr) {
			CommandMove(*bestunit, pos, EFlushMode::On);
			AiPlayer->LastExplorationGameCycle = GameCycle;
			break;
		}
	}
	// Remove all requests
	AiPlayer->FirstExplorationRequest.clear();
}

//@}
