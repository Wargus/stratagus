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

#include <stdio.h>
#include <stdlib.h>

#include "stratagus.h"
#include "unit.h"
#include "unittype.h"
#include "map.h"
#include "pathfinder.h"
#include "ai_local.h"
#include "player.h"

/*----------------------------------------------------------------------------
--  Variables
----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
--  Functions
----------------------------------------------------------------------------*/

/**
**  Check if the surrounding are free. Depending on the value of flag, it will check :
**  0: the building will not block any way
**  1: all surrounding is free
**
**  @param worker    Worker to build.
**  @param type      Type of building.
**  @param x         X map tile position for the building.
**  @param y         Y map tile position for the building.
**  @param backupok  Location can be used as a backup
**
**  @return          True if the surrounding is free, false otherwise.
**
**  @note            Can be faster written.
*/
static int AiCheckSurrounding(const CUnit &worker,
							  const CUnitType &type, int x, int y, bool &backupok)
{
	static int dirs[4][3] = {{1, 0, 0}, {0, 1, Map.Info.MapWidth},
		{ -1, 0, 0}, {0, -1, -Map.Info.MapWidth}
	};
	int surrounding[1024]; // Max circonference for building
	int surroundingnb;
	int lastval;
	int dir;
	int obstacle;
	int y_offset;

	int x0 = x - 1;
	int y0 = y - 1;
	int x1 = x0 + type.TileWidth + 1;
	int y1 = y0 + type.TileWidth + 1;

	x = x0;
	y = y0;
	y_offset = y * Map.Info.MapWidth;
	dir = -1;
	surroundingnb = 0;
	while (dir < 4) {
		if (Map.Info.IsPointOnMap(x, y)) {
			if (x == worker.tilePos.x && y == worker.tilePos.y) {
				surrounding[surroundingnb++] = 1;
			} else if (Map.CheckMask(x + y_offset,
									 (MapFieldUnpassable | MapFieldWall | MapFieldRocks
									  | MapFieldForest | MapFieldBuilding))) {
				surrounding[surroundingnb++] = 0;
			} else {
				// Can pass there
				surrounding[surroundingnb++] = Map.CheckMask(x + y_offset, (MapFieldWaterAllowed | MapFieldCoastAllowed | MapFieldLandAllowed));
			}
		} else {
			surrounding[surroundingnb++] = 0;
		}

		if ((x == x0 || x == x1) && (y == y0 || y == y1)) {
			dir++;
		}

		x += dirs[dir][0];
		y += dirs[dir][1];
		y_offset += dirs[dir][2];
	}

	lastval = surrounding[surroundingnb - 1];
	obstacle = 0;
	for (int i = 0 ; i < surroundingnb; ++i) {
		if (lastval && !surrounding[i]) {
			++obstacle;
		}
		lastval = surrounding[i];
	}

	if (obstacle == 0) {
		obstacle = !surrounding[0];
	}

	if (!type.ShoreBuilding) {
		backupok = obstacle < 5;
	} else {
		// Shore building have at least 2 obstacles : sea->ground & ground->sea
		backupok = obstacle < 3;
	}
	return obstacle == 0;
}

class BuildingPlaceFinder
{
public:
	BuildingPlaceFinder(const CUnit &worker, const CUnitType &type, bool checkSurround, Vec2i* resultPos) :
		worker(worker), type(type),
		movemask(worker.Type->MovementMask & ~(MapFieldLandUnit | MapFieldAirUnit | MapFieldSeaUnit)),
		checkSurround(checkSurround),
		resultPos(resultPos)
	{
		resultPos->x = -1;
		resultPos->y = -1;
	}
	VisitResult Visit(TerrainTraversal &terrainTraversal, const Vec2i &pos, const Vec2i &from);
private:
	const CUnit &worker;
	const CUnitType &type;
	unsigned int movemask;
	bool checkSurround;
	Vec2i* resultPos;
};

VisitResult BuildingPlaceFinder::Visit(TerrainTraversal &terrainTraversal, const Vec2i &pos, const Vec2i &from)
{
#if 0
	if (!player.AiEnabled && !Map.IsFieldExplored(player, pos)) {
		terrainTraversal.Get(pos) = -1;
		return VisitResult_DeadEnd;
	}
#endif
	if (CanBuildUnitType(&worker, type, pos, 1)
		&& !AiEnemyUnitsInDistance(*worker.Player, NULL, pos, 8)) {
		bool backupok;
		if (AiCheckSurrounding(worker, type, pos.x, pos.y, backupok) && checkSurround) {
			*resultPos = pos;
			return VisitResult_Finished;
		} else if (backupok && resultPos->x == -1) {
			*resultPos = pos;
		}
	}
	if (CanMoveToMask(pos, movemask)) { // reachable
		terrainTraversal.Get(pos) = 1;
		return VisitResult_Ok;
	} else { // unreachable
		terrainTraversal.Get(pos) = -1;
		return VisitResult_DeadEnd;
	}
}

/**
**  Find free building place. (flood fill version)
**
**  @param worker   Worker to build building.
**  @param type     Type of building.
**  @param startPos Original position to try building
**  @param dpos     OUT: Pointer for position returned.
**  @param checkSurround
**
**  @return        True if place found, false if no found.
*/
static int AiFindBuildingPlace2(const CUnit &worker, const CUnitType &type, const Vec2i &startPos, const CUnit *startUnit, Vec2i *dpos, bool checkSurround)
{
	TerrainTraversal terrainTraversal;

	terrainTraversal.SetSize(Map.Info.MapWidth, Map.Info.MapHeight);
	terrainTraversal.Init(-1);

	if (startUnit != NULL) {
		const Vec2i offset = {1, 1};
		const Vec2i extraTileSize = {startUnit->Type->TileWidth - 1, startUnit->Type->TileHeight - 1};
		const Vec2i start = startUnit->tilePos - offset;
		const Vec2i end = startUnit->tilePos + extraTileSize + offset;

		for (Vec2i it = start; it.y != end.y; ++it.y) {
			for (it.x = start.x; it.x != end.x; ++it.x) {
				terrainTraversal.PushPos(it);
			}
		}
	} else {
		terrainTraversal.PushPos(startPos);
	}

	BuildingPlaceFinder buildingPlaceFinder(worker, type, checkSurround, dpos);

	terrainTraversal.Run(buildingPlaceFinder);
	return Map.Info.IsPointOnMap(*dpos);
}

class HallPlaceFinder
{
public:
	HallPlaceFinder(const CUnit &worker, const CUnitType &type, int resource, Vec2i* resultPos) :
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
	Vec2i* resultPos;
};

bool HallPlaceFinder::IsAUsableMine(const CUnit &mine) const
{
	// Check units around mine
	const Vec2i offset = {5, 5};
	const Vec2i minpos = mine.tilePos - offset;
	const Vec2i typeSize = {mine.Type->TileWidth - 1, mine.Type->TileHeight - 1};
	const Vec2i maxpos = mine.tilePos + typeSize + offset;
	std::vector<CUnit *> units;

	Map.Select(minpos, maxpos, units);

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
		terrainTraversal.Get(pos) = -1;
		return VisitResult_DeadEnd;
	}
#endif
	CUnit *mine = ResourceOnMap(pos, resource);
	if (mine && IsAUsableMine(*mine)) {
		if (AiFindBuildingPlace2(worker, type, pos, mine, resultPos, true)) {
			return VisitResult_Finished;
		}
	}
	if (CanMoveToMask(pos, movemask)) { // reachable
		terrainTraversal.Get(pos) = 1;
		return VisitResult_Ok;
	} else { // unreachable
		terrainTraversal.Get(pos) = -1;
		return VisitResult_DeadEnd;
	}
}

/**
**  Find building place for hall. (flood fill version)
**
**  The best place:
**  1) near to goldmine.
**  !2) near to wood.
**  !3) near to worker and must be reachable.
**  4) no enemy near it.
**  5) no hall already near
**  !6) enough gold in mine
**
**  @param worker    Worker to build building.
**  @param type      Type of building.
**  @param startPos  Start search position (if == -1 then unit X pos used).
**  @param resultPos OUT: Pointer for position returned.
**
**  @return        True if place found, false if not found.
**
**  @todo          FIXME: This is slow really slow, using
**                 two flood fills, is not a perfect solution.
*/
static int AiFindHallPlace(const CUnit &worker,
						   const CUnitType &type,
						   const Vec2i &startPos,
						   Vec2i *resultPos,
						   int resource = GoldCost)
{
	TerrainTraversal terrainTraversal;

	terrainTraversal.SetSize(Map.Info.MapWidth, Map.Info.MapHeight);
	terrainTraversal.Init(-1);

	if (Map.Info.IsPointOnMap(startPos)) {
		terrainTraversal.PushPos(startPos);
	} else {
		terrainTraversal.PushPos(worker.tilePos);
	}

	HallPlaceFinder hallPlaceFinder(worker, type, resource, resultPos);

	return terrainTraversal.Run(hallPlaceFinder);
}

class LumberMillPlaceFinder
{
public:
	LumberMillPlaceFinder(const CUnit &worker, const CUnitType &type, Vec2i* resultPos) :
		worker(worker), type(type),
		movemask(worker.Type->MovementMask & ~(MapFieldLandUnit | MapFieldAirUnit | MapFieldSeaUnit)),
		resultPos(resultPos)
	{}
	VisitResult Visit(TerrainTraversal &terrainTraversal, const Vec2i &pos, const Vec2i &from);
private:
	const CUnit &worker;
	const CUnitType &type;
	unsigned int movemask;
	Vec2i* resultPos;
};

VisitResult LumberMillPlaceFinder::Visit(TerrainTraversal &terrainTraversal, const Vec2i &pos, const Vec2i &from)
{
#if 0
	if (!player.AiEnabled && !Map.IsFieldExplored(player, pos)) {
		terrainTraversal.Get(pos) = -1;
		return VisitResult_DeadEnd;
	}
#endif
	if (Map.ForestOnMap(pos)) {
		if (AiFindBuildingPlace2(worker, type, pos, NULL, resultPos, true)) {
			return VisitResult_Finished;
		}
	}
	if (CanMoveToMask(pos, movemask)) { // reachable
		terrainTraversal.Get(pos) = 1;
		return VisitResult_Ok;
	} else { // unreachable
		terrainTraversal.Get(pos) = -1;
		return VisitResult_DeadEnd;
	}
}

/**
**  Find free building place for lumber mill. (flood fill version)
**
**  @param worker   Worker to build building.
**  @param type     Type of building.
**  @param startPos Start search X position (if == -1 then unit X pos used).
**  @param dpos     OUT: Pointer for position returned.
**
**  @return        True if place found, false if not found.
**
**  @todo          FIXME: This is slow really slow, using two flood fills, is not a perfect solution.
*/
static bool AiFindLumberMillPlace(const CUnit &worker, const CUnitType &type, const Vec2i &startPos, Vec2i *dpos)
{
	TerrainTraversal terrainTraversal;

	terrainTraversal.SetSize(Map.Info.MapWidth, Map.Info.MapHeight);
	terrainTraversal.Init(-1);

	if (Map.Info.IsPointOnMap(startPos)) {
		terrainTraversal.PushPos(startPos);
	} else {
		terrainTraversal.PushPos(worker.tilePos);
	}

	LumberMillPlaceFinder lumberMillPlaceFinder(worker, type, dpos);

	return terrainTraversal.Run(lumberMillPlaceFinder);
}

static int AiFindMiningPlace(const CUnit &worker,
							 const CUnitType &type,
							 int nx, int ny,
							 Vec2i *dpos,
							 int resource)
{
	const Vec2i offset[] = {{0, -1}, { -1, 0}, {1, 0}, {0, 1}, { -1, -1}, {1, -1}, { -1, 1}, {1, 1}};
	Vec2i pos;
	Vec2i rpos;
	int wp;
	int ep;
	unsigned char *m;
	CUnit *mine;

	pos.x = (nx != -1 ? nx : worker.tilePos.x);
	pos.y = (ny != -1 ? ny : worker.tilePos.y);
	int size = Map.Info.MapWidth * Map.Info.MapHeight / 4;
	Vec2i *points = new Vec2i[size];

	//
	// Make movement matrix. FIXME: can create smaller matrix.
	//
	unsigned char *morg = MakeMatrix();
	int w = Map.Info.MapWidth + 2;
	unsigned char *matrix = morg + w + w + 1;

	points[0] = pos;
	int rp = 0;
	//if(worker->tilePos == pos)
	matrix[pos.x + pos.y * w] = 1; // mark start point
	ep = wp = 1; // start with one point

	int mask = worker.Type->MovementMask;

	//
	// Pop a point from stack, push all neighbors which could be entered.
	//
	for (;;) {
		while (rp != ep) {
			rpos = points[rp];
			for (int i = 0; i < 8; ++i) { // mark all neighbors
				pos = rpos + offset[i];
				m = matrix + pos.x + pos.y * w;
				if (*m) { // already checked
					continue;
				}
				//
				// Look if there is a mine area
				//
				if ((mine = ResourceOnMap(pos, resource, false)) &&
					AiFindBuildingPlace2(worker, type, mine->tilePos, mine, dpos, false)) {
					delete[] morg;
					delete[] points;
					return 1;
				}

				if (CanMoveToMask(pos, mask)) { // reachable
					*m = 1;
					points[wp] = pos; // push the point
					if (++wp >= size) { // round about
						wp = 0;
					}
				} else { // unreachable
					*m = 99;
				}
			}
			if (++rp >= size) { // round about
				rp = 0;
			}
		}

		//
		// Continue with next frame.
		//
		if (rp == wp) { // unreachable, no more points available
			break;
		}
		ep = wp;
	}

	delete[] morg;
	delete[] points;
	return 0;
}



/**
**  Find free building place.
**
**  @param worker  Worker to build building.
**  @param type    Type of building.
**  @param nearPos Start search near nearPos position (or worker->X if nearPos is invalid).
**  @param dpos    Pointer for position returned.
**
**  @return        True if place found, false if no found.
**
**  @todo          Better and faster way to find building place of oil
**                 platforms Special routines for special buildings.
*/
int AiFindBuildingPlace(const CUnit &worker, const CUnitType &type, const Vec2i &nearPos, Vec2i *dpos)
{
	// Find a good place for a new hall
	DebugPrint("%d: Want to build a %s(%s)\n" _C_ AiPlayer->Player->Index
			   _C_ type.Ident.c_str() _C_ type.Name.c_str());

	//Mines and Depots
	for (int i = 1; i < MaxCosts; ++i) {
		ResourceInfo *resinfo = worker.Type->ResInfo[i];
		//Depots
		if (type.CanStore[i]) {
			if (resinfo && resinfo->TerrainHarvester) {
				return AiFindLumberMillPlace(worker, type, nearPos, dpos);
			} else {
				return AiFindHallPlace(worker, type, nearPos, dpos, i);
			}
		} else
			//mines
			if (type.GivesResource == i) {
				if (resinfo && resinfo->RefineryHarvester) {
					//Mine have to be build ONTOP resources
					return AiFindMiningPlace(worker, type, nearPos.x, nearPos.y,  dpos, i);
				} else {
					//Mine can be build without resource restrictions: solar panels, etc
					const Vec2i &startPos = Map.Info.IsPointOnMap(nearPos) ? nearPos : worker.tilePos;
					return AiFindBuildingPlace2(worker, type, startPos, NULL, dpos, true);
				}
			}
	}
	const Vec2i &startPos = Map.Info.IsPointOnMap(nearPos) ? nearPos : worker.tilePos;
	return AiFindBuildingPlace2(worker, type, startPos, NULL, dpos, true);
}

//@}
