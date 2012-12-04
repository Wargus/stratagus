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
/**@name ai_building.cpp - AI building functions. */
//
//      (c) Copyright 2001-2005 by Lutz Sammer
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

#include "map.h"
#include "pathfinder.h"
#include "player.h"
#include "unit.h"
#include "unit_find.h"
#include "unittype.h"

/*----------------------------------------------------------------------------
--  Variables
----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
--  Functions
----------------------------------------------------------------------------*/

static bool IsPosFree(const Vec2i &pos, const CUnit &exceptionUnit)
{
	if (Map.Info.IsPointOnMap(pos) == false) {
		return false;
	}
	const CMapField &mf = *Map.Field(pos);
	const CUnitCache &unitCache = mf.UnitCache;
	if (std::find(unitCache.begin(), unitCache.end(), &exceptionUnit) != unitCache.end()) {
		return true;
	}
	const unsigned int blockedFlag = (MapFieldUnpassable | MapFieldWall | MapFieldRocks | MapFieldForest | MapFieldBuilding);
	if (mf.Flags & blockedFlag) {
		return false;
	}
	const unsigned int passableFlag = (MapFieldWaterAllowed | MapFieldCoastAllowed | MapFieldLandAllowed);
	return ((mf.Flags & passableFlag) != 0);
}

/**
**  Check if the surrounding are free. Depending on the value of flag, it will check :
**  0: the building will not block any way
**  1: all surrounding is free
**
**  @param worker    Worker to build.
**  @param type      Type of building.
**  @param pos       map tile position for the building.
**  @param backupok  Location can be used as a backup
**
**  @return          True if the surrounding is free, false otherwise.
**
**  @note            Can be faster written.
*/
static bool AiCheckSurrounding(const CUnit &worker, const CUnitType &type, const Vec2i &pos, bool &backupok)
{
	const int surroundRange = type.AiAdjacentRange != -1 ? type.AiAdjacentRange : 1;
	const Vec2i pos_topLeft(pos.x - surroundRange, pos.y - surroundRange);
	const Vec2i pos_bottomRight(pos.x + type.TileWidth + surroundRange - 1,
								pos.y + type.TileHeight + surroundRange - 1);
	Vec2i it = pos_topLeft;
	const bool firstVal = IsPosFree(it, worker);
	bool lastval = firstVal;
	int obstacleCount = 0;

	for (++it.x; it.x < pos_bottomRight.x; ++it.x) {
		const bool isfree = IsPosFree(it, worker);

		if (isfree && !lastval) {
			++obstacleCount;
		}
		lastval = isfree;
	}
	for (; it.y < pos_bottomRight.y; ++it.y) {
		const bool isfree = IsPosFree(it, worker);

		if (isfree && !lastval) {
			++obstacleCount;
		}
		lastval = isfree;
	}
	for (; pos_topLeft.x < it.x; --it.x) {
		const bool isfree = IsPosFree(it, worker);

		if (isfree && !lastval) {
			++obstacleCount;
		}
		lastval = isfree;
	}
	for (; pos_topLeft.y < it.y; --it.y) {
		const bool isfree = IsPosFree(it, worker);

		if (isfree && !lastval) {
			++obstacleCount;
		}
		lastval = isfree;
	}
	if (firstVal && !lastval) {
		++obstacleCount;
	}

	if (!type.ShoreBuilding) {
		backupok = obstacleCount < 5;
	} else {
		// Shore building have at least 2 obstacles : sea->ground & ground->sea
		backupok = obstacleCount < 3;
	}
	return obstacleCount == 0;
}

class BuildingPlaceFinder
{
public:
	BuildingPlaceFinder(const CUnit &worker, const CUnitType &type, bool checkSurround, Vec2i *resultPos) :
		worker(worker), type(type),
		movemask(worker.Type->MovementMask & ~(MapFieldLandUnit | MapFieldAirUnit | MapFieldSeaUnit)),
		checkSurround(checkSurround),
		resultPos(resultPos) {
		resultPos->x = -1;
		resultPos->y = -1;
	}
	VisitResult Visit(TerrainTraversal &terrainTraversal, const Vec2i &pos, const Vec2i &from);
private:
	const CUnit &worker;
	const CUnitType &type;
	unsigned int movemask;
	bool checkSurround;
	Vec2i *resultPos;
};

VisitResult BuildingPlaceFinder::Visit(TerrainTraversal &terrainTraversal, const Vec2i &pos, const Vec2i &from)
{
#if 0
	if (!player.AiEnabled && !Map.IsFieldExplored(player, pos)) {
		return VisitResult_DeadEnd;
	}
#endif
	if (CanBuildUnitType(&worker, type, pos, 1)
		&& !AiEnemyUnitsInDistance(*worker.Player, NULL, pos, 8)) {
		bool backupok;
		if (AiCheckSurrounding(worker, type, pos, backupok) && checkSurround) {
			*resultPos = pos;
			return VisitResult_Finished;
		} else if (backupok && resultPos->x == -1) {
			*resultPos = pos;
		}
	}
	if (CanMoveToMask(pos, movemask)) { // reachable
		return VisitResult_Ok;
	} else { // unreachable
		return VisitResult_DeadEnd;
	}
}

/**
**  Find free building place. (flood fill version)
**
**  @param worker   Worker to build building.
**  @param type     Type of building.
**  @param startPos Original position to try building
**  @param checkSurround Check if the perimeter of the building is free
**  @param resultPos     OUT: Pointer for position returned.
**
**  @return  True if place found, false if no found.
*/
static bool AiFindBuildingPlace2(const CUnit &worker, const CUnitType &type, const Vec2i &startPos, const CUnit *startUnit, bool checkSurround, Vec2i *resultPos)
{
	TerrainTraversal terrainTraversal;

	terrainTraversal.SetSize(Map.Info.MapWidth, Map.Info.MapHeight);
	terrainTraversal.Init();

	if (startUnit != NULL) {
		terrainTraversal.PushUnitPosAndNeighboor(*startUnit);
	} else {
		Assert(Map.Info.IsPointOnMap(startPos));
		terrainTraversal.PushPos(startPos);
	}

	BuildingPlaceFinder buildingPlaceFinder(worker, type, checkSurround, resultPos);

	terrainTraversal.Run(buildingPlaceFinder);
	return Map.Info.IsPointOnMap(*resultPos);
}

class HallPlaceFinder
{
public:
	HallPlaceFinder(const CUnit &worker, const CUnitType &type, int resource, Vec2i *resultPos) :
		worker(worker), type(type),
		movemask(worker.Type->MovementMask & ~(MapFieldLandUnit | MapFieldAirUnit | MapFieldSeaUnit)),
		resource(resource),
		resultPos(resultPos)
	{}
	VisitResult Visit(TerrainTraversal &terrainTraversal, const Vec2i &pos, const Vec2i &from);
private:
	bool IsAUsableMine(const CUnit &mine) const;
private:
	const CUnit &worker;
	const CUnitType &type;
	const unsigned int movemask;
	const int resource;
	Vec2i *resultPos;
};

bool HallPlaceFinder::IsAUsableMine(const CUnit &mine) const
{
	// Check units around mine
	const Vec2i offset(5, 5);
	const Vec2i minpos = mine.tilePos - offset;
	const Vec2i typeSize(mine.Type->TileWidth - 1, mine.Type->TileHeight - 1);
	const Vec2i maxpos = mine.tilePos + typeSize + offset;
	std::vector<CUnit *> units;

	Select(minpos, maxpos, units);

	const size_t nunits = units.size();
	int buildings = 0;

	for (size_t j = 0; j < nunits; ++j) {
		const CUnit &unit = *units[j];
		// Enemy near mine
		if (AiPlayer->Player->IsEnemy(*unit.Player)) {
			return false;
		}
		// Town hall near mine
		if (unit.Type->CanStore[resource]) {
			return false;
		}
		// Town hall may not be near but we may be using it, check
		// for 2 buildings near it and assume it's been used
		if (unit.Type->Building && !unit.Type->GivesResource == resource) {
			++buildings;
			if (buildings == 2) {
				return false;
			}
		}
	}
	return true;
}

VisitResult HallPlaceFinder::Visit(TerrainTraversal &terrainTraversal, const Vec2i &pos, const Vec2i &from)
{
#if 0
	if (!player.AiEnabled && !Map.IsFieldExplored(player, pos)) {
		return VisitResult_DeadEnd;
	}
#endif
	CUnit *mine = ResourceOnMap(pos, resource);
	if (mine && IsAUsableMine(*mine)) {
		if (AiFindBuildingPlace2(worker, type, pos, mine, true, resultPos)) {
			return VisitResult_Finished;
		}
	}
	if (CanMoveToMask(pos, movemask)) { // reachable
		return VisitResult_Ok;
	} else { // unreachable
		return VisitResult_DeadEnd;
	}
}

/**
**  Find building place for hall. (flood fill version)
**
**  The best place:
**  1) near to resource.
**  !2) near to wood.
**  !3) near to worker and must be reachable.
**  4) no enemy near it.
**  5) no hall already near
**  !6) enough gold in mine
**
**  @param worker    Worker to build building.
**  @param type      Type of building.
**  @param startPos  Start search position (if == -1 then unit X pos used).
**  @param resource  resource to be near.
**  @param resultPos OUT: Pointer for position returned.
**
**  @return        True if place found, false if not found.
**
**  @todo          FIXME: This is slow really slow, using
**                 two flood fills, is not a perfect solution.
*/
static bool AiFindHallPlace(const CUnit &worker,
							const CUnitType &type,
							const Vec2i &startPos,
							int resource,
							Vec2i *resultPos)
{
	TerrainTraversal terrainTraversal;

	terrainTraversal.SetSize(Map.Info.MapWidth, Map.Info.MapHeight);
	terrainTraversal.Init();

	Assert(Map.Info.IsPointOnMap(startPos));
	terrainTraversal.PushPos(startPos);

	HallPlaceFinder hallPlaceFinder(worker, type, resource, resultPos);

	return terrainTraversal.Run(hallPlaceFinder);
}

class LumberMillPlaceFinder
{
public:
	LumberMillPlaceFinder(const CUnit &worker, const CUnitType &type, int resource, Vec2i *resultPos) :
		worker(worker), type(type),
		movemask(worker.Type->MovementMask & ~(MapFieldLandUnit | MapFieldAirUnit | MapFieldSeaUnit)),
		resource(resource),
		resultPos(resultPos)
	{}
	VisitResult Visit(TerrainTraversal &terrainTraversal, const Vec2i &pos, const Vec2i &from);
private:
	const CUnit &worker;
	const CUnitType &type;
	unsigned int movemask;
	int resource;
	Vec2i *resultPos;
};

VisitResult LumberMillPlaceFinder::Visit(TerrainTraversal &terrainTraversal, const Vec2i &pos, const Vec2i &from)
{
#if 0
	if (!player.AiEnabled && !Map.IsFieldExplored(player, pos)) {
		return VisitResult_DeadEnd;
	}
#endif
	if (Map.Field(pos)->IsTerrainResourceOnMap(resource)) {
		if (AiFindBuildingPlace2(worker, type, pos, NULL, true, resultPos)) {
			return VisitResult_Finished;
		}
	}
	if (CanMoveToMask(pos, movemask)) { // reachable
		return VisitResult_Ok;
	} else { // unreachable
		return VisitResult_DeadEnd;
	}
}

/**
**  Find free building place for lumber mill. (flood fill version)
**
**  @param worker    Worker to build building.
**  @param type      Type of building.
**  @param resource  resource terrain to be near.
**  @param startPos  Start search X position (if == -1 then unit X pos used).
**  @param resultPos OUT: Pointer for position returned.
**
**  @return        True if place found, false if not found.
**
**  @todo          FIXME: This is slow really slow, using two flood fills, is not a perfect solution.
*/
static bool AiFindLumberMillPlace(const CUnit &worker, const CUnitType &type, const Vec2i &startPos, int resource, Vec2i *resultPos)
{
	TerrainTraversal terrainTraversal;

	terrainTraversal.SetSize(Map.Info.MapWidth, Map.Info.MapHeight);
	terrainTraversal.Init();

	Assert(Map.Info.IsPointOnMap(startPos));
	terrainTraversal.PushPos(startPos);

	LumberMillPlaceFinder lumberMillPlaceFinder(worker, type, resource, resultPos);

	return terrainTraversal.Run(lumberMillPlaceFinder);
}

static bool AiFindMiningPlace(const CUnit &worker,
							  const CUnitType &type,
							  const Vec2i &startPos,
							  int resource,
							  Vec2i *resultPos)
{
	// look near (mine = ResourceOnMap(pos, resource, false) ?
	return AiFindBuildingPlace2(worker, type, startPos, NULL, false, resultPos);
}

/**
**  Find free building place.
**
**  @param worker     Worker to build building.
**  @param type       Type of building.
**  @param nearPos    Start search near nearPos position (or worker->X if nearPos is invalid).
**  @param resultPos  Pointer for position returned.
**
**  @return        True if place found, false if no found.
**
**  @todo          Better and faster way to find building place of oil
**                 platforms Special routines for special buildings.
*/
bool AiFindBuildingPlace(const CUnit &worker, const CUnitType &type, const Vec2i &nearPos, Vec2i *resultPos)
{
	// Find a good place for a new hall
	DebugPrint("%d: Want to build a %s(%s)\n" _C_ AiPlayer->Player->Index
			   _C_ type.Ident.c_str() _C_ type.Name.c_str());

	const Vec2i &startPos = Map.Info.IsPointOnMap(nearPos) ? nearPos : worker.tilePos;

	//Mines and Depots
	for (int i = 1; i < MaxCosts; ++i) {
		ResourceInfo *resinfo = worker.Type->ResInfo[i];
		//Depots
		if (type.CanStore[i]) {
			if (resinfo && resinfo->TerrainHarvester) {
				return AiFindLumberMillPlace(worker, type, startPos, i, resultPos);
			} else {
				return AiFindHallPlace(worker, type, startPos, i, resultPos);
			}
		} else {
			//mines
			if (type.GivesResource == i) {
				if (resinfo && resinfo->RefineryHarvester) {
					//Mine have to be build ONTOP resources
					return AiFindMiningPlace(worker, type, startPos, i, resultPos);
				} else {
					//Mine can be build without resource restrictions: solar panels, etc
					return AiFindBuildingPlace2(worker, type, startPos, NULL, true, resultPos);
				}
			}
		}
	}
	return AiFindBuildingPlace2(worker, type, startPos, NULL, true, resultPos);
}

//@}
