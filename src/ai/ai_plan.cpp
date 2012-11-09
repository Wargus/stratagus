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
#include "unit.h"
#include "unit_find.h"
#include "unittype.h"

/*----------------------------------------------------------------------------
--  Variables
----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
--  Functions
----------------------------------------------------------------------------*/

class _EnemyOnMapTile
{
public:
	_EnemyOnMapTile(const CUnit &unit, const Vec2i _pos, CUnit **enemy) :
		source(&unit) , pos(_pos), best(enemy) {
	}

	void operator()(CUnit *const unit) const {
		const CUnitType &type = *unit->Type;
		// unusable unit ?
		// if (unit->IsUnusable()) can't attack constructions
		// FIXME: did SelectUnitsOnTile already filter this?
		// Invisible and not Visible
		if (unit->Removed || unit->Variable[INVISIBLE_INDEX].Value
			// || (!UnitVisible(unit, source->Player))
			|| unit->CurrentAction() == UnitActionDie) {
			return;
		}
		if (pos.x < unit->tilePos.x || pos.x >= unit->tilePos.x + type.TileWidth
			|| pos.y < unit->tilePos.y || pos.y >= unit->tilePos.y + type.TileHeight) {
			return;
		}
		if (!CanTarget(*source->Type, type)) {
			return;
		}
		if (!source->Player->IsEnemy(*unit)) { // a friend or neutral
			return;
		}
		// Choose the best target.
		if (!*best || (*best)->Type->Priority < type.Priority) {
			*best = unit;
		}
	}

private:
	const CUnit *const source;
	const Vec2i pos;
	CUnit **best;
};

/**
**  Choose enemy on map tile.
**
**  @param source  Unit which want to attack.
**  @param pos     position on map, tile-based.
**
**  @return        Returns ideal target on map tile.
*/
static CUnit *EnemyOnMapTile(const CUnit &source, const Vec2i &pos)
{
	CUnit *enemy = NULL;

	_EnemyOnMapTile filter(source, pos, &enemy);
	Map.Field(pos)->UnitCache.for_each(filter);
	return enemy;
}

class WallFinder
{
public:
	WallFinder(const CUnit &unit, int maxDist, Vec2i *resultPos) :
		unit(unit),
		maxDist(maxDist),
		movemask(unit.Type->MovementMask & ~(MapFieldLandUnit | MapFieldAirUnit | MapFieldSeaUnit)),
		resultPos(resultPos)
	{}
	VisitResult Visit(TerrainTraversal &terrainTraversal, const Vec2i &pos, const Vec2i &from);
private:
	const CUnit &unit;
	int maxDist;
	int movemask;
	Vec2i *resultPos;
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
		DebugPrint("Wall found %d, %d\n" _C_ pos.x _C_ pos.y);
		if (resultPos) {
			*resultPos = from;
		}
		return VisitResult_Finished;
	}
	if (Map.Field(pos)->CheckMask(movemask)) { // reachable
		if (terrainTraversal.Get(pos) <= maxDist) {
			return VisitResult_Ok;
		} else {
			return VisitResult_DeadEnd;
		}
	} else { // unreachable
		return VisitResult_DeadEnd;
	}
}

static bool FindWall(const CUnit &unit, int range, Vec2i *wallPos)
{
	TerrainTraversal terrainTraversal;

	terrainTraversal.SetSize(Map.Info.MapWidth, Map.Info.MapHeight);
	terrainTraversal.Init();

	terrainTraversal.PushUnitPosAndNeighboor(unit);

	WallFinder wallFinder(unit, range, wallPos);

	return terrainTraversal.Run(wallFinder);
}

/**
**  Find possible walls to target.
**
**  @param force  Attack force.
**
**  @return       True if wall found.
*/
int AiFindWall(AiForce *force)
{
	// Find a unit to use.  Best choice is a land unit with range 1.
	// Next best choice is any land unit.  Otherwise just use the first.
	CUnit *unit = force->Units[0];
	for (unsigned int i = 0; i < force->Units.size(); ++i) {
		CUnit *aiunit = force->Units[i];
		if (aiunit->Type->UnitType == UnitTypeLand) {
			unit = aiunit;
			if (aiunit->Type->Missile.Missile->Range == 1) {
				break;
			}
		}
	}
	const int maxRange = 1000;
	Vec2i wallPos;

	if (FindWall(*unit, maxRange, &wallPos)) {
		force->State = AiForceAttackingState_Waiting;
		for (unsigned int i = 0; i < force->Units.size(); ++i) {
			CUnit &aiunit = *force->Units[i];
			if (aiunit.Type->CanAttack) {
				CommandAttack(aiunit, wallPos, NULL, FlushCommands);
			} else {
				CommandMove(aiunit, wallPos, FlushCommands);
			}
		}
		return 1;
	}
	return 0;
}


class ReachableTerrainMarker
{
public:
	ReachableTerrainMarker(const CUnit &unit) :
		unit(unit),
		movemask(unit.Type->MovementMask & ~(MapFieldLandUnit | MapFieldAirUnit | MapFieldSeaUnit))
	{}
	VisitResult Visit(TerrainTraversal &terrainTraversal, const Vec2i &pos, const Vec2i &from);
private:
	const CUnit &unit;
	int movemask;
};

VisitResult ReachableTerrainMarker::Visit(TerrainTraversal &terrainTraversal, const Vec2i &pos, const Vec2i &from)
{
#if 0
	if (!player.AiEnabled && !Map.IsFieldExplored(player, pos)) {
		return VisitResult_DeadEnd;
	}
#endif
	if (CanMoveToMask(pos, movemask)) { // reachable
		return VisitResult_Ok;
	} else { // unreachable
		return VisitResult_DeadEnd;
	}
}

static void MarkReacheableTerrainType(const CUnit &unit, TerrainTraversal *terrainTraversal)
{
	terrainTraversal->PushUnitPosAndNeighboor(unit);

	ReachableTerrainMarker reachableTerrainMarker(unit);

	terrainTraversal->Run(reachableTerrainMarker);
}

class EnemyFinderWithTransporter
{
public:
	EnemyFinderWithTransporter(const CUnit &unit, const TerrainTraversal &terrainTransporter, Vec2i *resultPos) :
		unit(unit),
		terrainTransporter(terrainTransporter),
		movemask(unit.Type->MovementMask & ~(MapFieldLandUnit | MapFieldAirUnit | MapFieldSeaUnit)),
		resultPos(resultPos)
	{}
	VisitResult Visit(TerrainTraversal &terrainTraversal, const Vec2i &pos, const Vec2i &from);
private:
	bool IsAccessibleForTransporter(const Vec2i &pos) const;
private:
	const CUnit &unit;
	const TerrainTraversal &terrainTransporter;
	int movemask;
	Vec2i *resultPos;
};

bool EnemyFinderWithTransporter::IsAccessibleForTransporter(const Vec2i &pos) const
{
	return terrainTransporter.IsReached(pos);
}

VisitResult EnemyFinderWithTransporter::Visit(TerrainTraversal &terrainTraversal, const Vec2i &pos, const Vec2i &from)
{
#if 0
	if (!player.AiEnabled && !Map.IsFieldExplored(player, pos)) {
		return VisitResult_DeadEnd;
	}
#endif
	if (EnemyOnMapTile(unit, pos) && CanMoveToMask(from, movemask)) {
		DebugPrint("Target found %d,%d\n" _C_ pos.x _C_ pos.y);
		*resultPos = pos;
		return VisitResult_Finished;
	}
	if (CanMoveToMask(pos, movemask) || IsAccessibleForTransporter(pos)) { // reachable
		return VisitResult_Ok;
	} else { // unreachable
		return VisitResult_DeadEnd;
	}
}

static bool AiFindTarget(const CUnit &unit, const TerrainTraversal &terrainTransporter, Vec2i *resultPos)
{
	TerrainTraversal terrainTraversal;

	terrainTraversal.SetSize(Map.Info.MapWidth, Map.Info.MapHeight);
	terrainTraversal.Init();

	terrainTraversal.PushUnitPosAndNeighboor(unit);

	EnemyFinderWithTransporter enemyFinderWithTransporter(unit, terrainTransporter, resultPos);

	return terrainTraversal.Run(enemyFinderWithTransporter);
}

class IsAFreeTransporter
{
public:
	bool operator()(const CUnit *unit) const {
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
int AiForce::PlanAttack()
{
	CPlayer &player = *AiPlayer->Player;
	DebugPrint("%d: Planning for force #%lu of player #%d\n"_C_ player.Index
			   _C_(long unsigned int)(this - & (AiPlayer->Force[0])) _C_ player.Index);

	TerrainTraversal transporterTerrainTraversal;

	transporterTerrainTraversal.SetSize(Map.Info.MapWidth, Map.Info.MapHeight);
	transporterTerrainTraversal.Init();

	CUnit *transporter = Units.find(IsAFreeTransporter());

	if (transporter != NULL) {
		DebugPrint("%d: Transporter #%d\n" _C_ player.Index _C_ UnitNumber(*transporter));
		MarkReacheableTerrainType(*transporter, &transporterTerrainTraversal);
	} else {
		std::vector<CUnit *>::iterator it = std::find_if(player.UnitBegin(), player.UnitEnd(), IsAFreeTransporter());
		if (it != player.UnitEnd()) {
			transporter = *it;
			MarkReacheableTerrainType(*transporter, &transporterTerrainTraversal);
		} else {
			DebugPrint("%d: No transporter available\n" _C_ player.Index);
			return 0;
		}
	}

	// Find a land unit of the force.
	// FIXME: if force is split over different places -> broken
	CUnit *landUnit = Units.find(CUnitTypeFinder(UnitTypeLand));
	if (landUnit == NULL) {
		DebugPrint("%d: No land unit in force\n" _C_ player.Index);
		return 0;
	}

	Vec2i pos = this->GoalPos;

	if (AiFindTarget(*landUnit, transporterTerrainTraversal, &pos)) {
		const int forceIndex = AiPlayer->Force.getIndex(this) + 1;

		if (transporter->GroupId != forceIndex) {
			DebugPrint("%d: Assign any transporter #%d\n" _C_ player.Index _C_ UnitNumber(*transporter));

			if (transporter->GroupId) {
				transporter->Player->Ai->Force[transporter->GroupId - 1].Remove(*transporter);
			}
			Insert(*transporter);
			transporter->GroupId = forceIndex;
		}

		int totalBoardCapacity = GetTotalBoardCapacity(Units.begin(), Units.end());

		// Verify we have enough transporter.
		// @note: Minimal check for unitType (flyers...)
		for (unsigned int i = 0; i < Size(); ++i) {
			CUnit &unit = *Units[i];

			if (CanTransport(*transporter, unit)) {
				totalBoardCapacity--;
			}
		}
		if (totalBoardCapacity < 0) { // Not enough transporter.
			IsAFreeTransporter isAFreeTransporter;
			// Add all other idle transporter.
			for (int i = 0; i < player.GetUnitCount(); ++i) {
				CUnit &unit = player.GetUnit(i);

				if (isAFreeTransporter(&unit) && unit.GroupId == 0 && unit.IsIdle()) {
					DebugPrint("%d: Assign any transporter #%d\n" _C_ player.Index _C_ UnitNumber(unit));
					Insert(unit);
					unit.GroupId = forceIndex;
					totalBoardCapacity += unit.Type->MaxOnBoard - unit.BoardCount;
					if (totalBoardCapacity >= 0) {
						break;
					}
				}
			}
		}
		DebugPrint("%d: Can attack\n" _C_ player.Index);
		GoalPos = pos;
		State = AiForceAttackingState_Boarding;
		return 1;
	}
	return 0;
}

static bool ChooseRandomUnexploredPositionNear(const Vec2i &center, Vec2i *pos)
{
	Assert(pos != NULL);

	int ray = 3;
	const int maxTryCount = 8;
	for (int i = 0; i != maxTryCount; ++i) {
		pos->x = center.x + SyncRand() % (2 * ray + 1) - ray;
		pos->y = center.y + SyncRand() % (2 * ray + 1) - ray;

		if (Map.Info.IsPointOnMap(*pos)
			&& Map.Field(*pos)->playerInfo.IsExplored(*AiPlayer->Player) == false) {
			return true;
		}
		ray = 3 * ray / 2;
	}
	return false;
}

static CUnit *GetBestExplorer(const AiExplorationRequest &request, Vec2i *pos)
{
	// Choose a target, "near"
	const Vec2i &center = request.pos;
	if (ChooseRandomUnexploredPositionNear(center, pos) == false) {
		return NULL;
	}
	// We have an unexplored tile in sight (pos)

	CUnit *bestunit = NULL;
	// Find an idle unit, responding to the mask
	bool flyeronly = false;
	int bestSquareDistance = -1;
	for (int i = 0; i != AiPlayer->Player->GetUnitCount(); ++i) {
		CUnit &unit = AiPlayer->Player->GetUnit(i);

		if (!unit.IsIdle()) {
			continue;
		}
		if (Map.Info.IsPointOnMap(unit.tilePos) == false) {
			continue;
		}
		if (unit.CanMove() == false) {
			continue;
		}
		const CUnitType &type = *unit.Type;

		if (type.UnitType != UnitTypeFly) {
			if (flyeronly) {
				continue;
			}
			if ((request.Mask & MapFieldLandUnit) && type.UnitType != UnitTypeLand) {
				continue;
			}
			if ((request.Mask & MapFieldSeaUnit) && type.UnitType != UnitTypeNaval) {
				continue;
			}
		} else {
			flyeronly = true;
		}

		const int sqDistance = SquareDistance(unit.tilePos, *pos);
		if (bestSquareDistance == -1 || sqDistance <= bestSquareDistance
			|| (bestunit->Type->UnitType != UnitTypeFly && type.UnitType == UnitTypeFly)) {
			bestSquareDistance = sqDistance;
			bestunit = &unit;
		}
	}
	return bestunit;
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

		Vec2i pos;
		CUnit *bestunit = GetBestExplorer(request, &pos);
		if (bestunit != NULL) {
			CommandMove(*bestunit, pos, FlushCommands);
			AiPlayer->LastExplorationGameCycle = GameCycle;
			break;
		}
	}
	// Remove all requests
	AiPlayer->FirstExplorationRequest.clear();
}

//@}
