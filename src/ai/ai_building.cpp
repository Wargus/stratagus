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
//      $Id$

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
static int AiCheckSurrounding(const CUnit *worker,
		 const CUnitType *type, int x, int y, bool &backupok)
{
	static int dirs[4][3] = {{1,0,0},{0,1,Map.Info.MapWidth},
								{-1,0,0},{0,-1,-Map.Info.MapWidth}};
	int surrounding[1024]; // Max criconference for building
	int surroundingnb;
	int x0, y0, x1, y1;
	int i;
	int lastval;
	int dir;
	int obstacle;
	int y_offset;
	
	x0 = x - 1;
	y0 = y - 1;
	x1 = x0 + type->TileWidth + 1;
	y1 = y0 + type->TileWidth + 1;


	x = x0;
	y = y0;
	y_offset = y * Map.Info.MapWidth;
	dir = -1;
	surroundingnb = 0;
	while (dir < 4) {
		if ((unsigned)x < (unsigned)Map.Info.MapWidth &&
						 (unsigned)y < (unsigned)Map.Info.MapHeight) {
			if (worker && x == worker->X && y == worker->Y) {
				surrounding[surroundingnb++] = 1;
			} else if (	Map.CheckMask(x + y_offset, 
						(MapFieldUnpassable | MapFieldWall | MapFieldRocks |
						MapFieldForest | MapFieldBuilding))
						) {
				surrounding[surroundingnb++] = 0;
			} else{
				// Can pass there
				surrounding[surroundingnb++] = Map.CheckMask(x + y_offset, 
						(MapFieldWaterAllowed + 
								MapFieldCoastAllowed + MapFieldLandAllowed));
						
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
	for (i = 0 ; i < surroundingnb; ++i) {
		if (lastval && !surrounding[i]) {
			++obstacle;
		}
		lastval = surrounding[i];
	}

	if (obstacle == 0) {
		obstacle = !surrounding[0];
	}

	if (!type->ShoreBuilding) {
		backupok = obstacle < 5;
	} else {
		// Shore building have at least 2 obstacles : sea->ground & ground->sea
		backupok = obstacle < 3;
	}
	return obstacle == 0;
}

/**
**  Find free building place. (flood fill version)
**
**  @param worker  Worker to build building.
**  @param type    Type of building.
**  @param ox      Original X position to try building
**  @param oy      Original Y position to try building
**  @param dx      Pointer for X position returned.
**  @param dy      Pointer for Y position returned.
**
**  @return        True if place found, false if no found.
*/
static int AiFindBuildingPlace2(const CUnit *worker, const CUnitType *type,
	int ox, int oy, int *dx, int *dy)
{
	static const int xoffset[] = { 0, -1, +1, 0, -1, +1, -1, +1 };
	static const int yoffset[] = { -1, 0, 0, +1, -1, -1, +1, +1 };
	struct p {
		unsigned short X;
		unsigned short Y;
	} *points;
	int size;
	int x;
	int y;
	int rx;
	int ry;
	int mask;
	int wp;
	int rp;
	int ep;
	int i;
	unsigned char *m;
	int backupx = -1;
	int backupy = -1;
	bool backupok;

	x = ox;
	y = oy;
	//
	// Look if we can build at current place.
	//
	if (CanBuildUnitType(worker, type, x, y, 1) &&
		!AiEnemyUnitsInDistance(worker->Player, NULL, x, y, 8)) {
		if (AiCheckSurrounding(worker, type, x, y, backupok)) {
			*dx = x;
			*dy = y;
			return 1;
		} else if (backupok) {
			backupx = x;
			backupy = y;
		}
	}
	
	size = Map.Info.MapWidth * Map.Info.MapHeight / 4;
	points = new p[size];
	
	//
	//  Make movement matrix.
	//
	unsigned char *matrix = CreateMatrix();
	const int w = Map.Info.MapWidth + 2;

	mask = worker->Type->MovementMask;
	// Ignore all possible mobile units.
	mask &= ~(MapFieldLandUnit | MapFieldAirUnit | MapFieldSeaUnit);

	points[0].X = x;
	points[0].Y = y;
	// also use the bottom right
	if ((type->TileWidth > 1 || type->TileHeight > 1) &&
			x + type->TileWidth - 1 < Map.Info.MapWidth &&
			y + type->TileHeight - 1 < Map.Info.MapHeight) {
		points[1].X = x + type->TileWidth - 1;
		points[1].Y = y + type->TileHeight - 1;
		ep = wp = 2; // start with two points
	} else {
		ep = wp = 1; // start with one point
	}
	matrix += w + w + 2;
	rp = 0;
	matrix[x + y * w] = 1; // mark start point

	//
	// Pop a point from stack, push all neighbours which could be entered.
	//
	for (;;) {
		while (rp != ep) {
			rx = points[rp].X;
			ry = points[rp].Y;
			for (i = 0; i < 8; ++i) { // mark all neighbors
				x = rx + xoffset[i];
				y = ry + yoffset[i];
				m = matrix + x + y * w;
				if (*m) { // already checked
					continue;
				}

				//
				// Look if we can build here and no enemies nearby.
				//
				if (CanBuildUnitType(worker, type, x, y, 1) &&
						!AiEnemyUnitsInDistance(worker->Player, NULL, x, y, 8)) {
					if (AiCheckSurrounding(worker, type, x, y, backupok)) {
						*dx = x;
						*dy = y;
						delete[] points;
						return 1;
					} else if (backupok && backupx == -1) {
						backupx = x;
						backupy = y;
					}
				}

				if (CanMoveToMask(x, y, mask)) { // reachable
					*m = 1;
					points[wp].X = x; // push the point
					points[wp].Y = y;
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

	delete[] points;

	if (backupx != -1) {
		*dx = backupx;
		*dy = backupy;
		return 1;
	}
	return 0;
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
**  @param worker  Worker to build building.
**  @param type    Type of building.
**  @param nx      Start search X position (if == -1 then unit X pos used).
**  @param ny      Start search X position (if == -1 then unit X pos used).
**  @param dx      Pointer for X position returned.
**  @param dy      Pointer for Y position returned.
**
**  @return        True if place found, false if not found.
**
**  @todo          FIXME: This is slow really slow, using
**                 two flood fills, is not a perfect solution.
*/
static int AiFindHallPlace(const CUnit *worker,
						 const CUnitType *type,
						 int nx, int ny, 
						 int *dx, int *dy, 
						 int resource = GoldCost)
{
	static const int xoffset[] = { 0, -1, +1, 0, -1, +1, -1, +1 };
	static const int yoffset[] = { -1, 0, 0, +1, -1, -1, +1, +1 };
	struct p {
		unsigned short X;
		unsigned short Y;
	} *points;
	int size;
	int x;
	int y;
	int rx;
	int ry;
	int mask;
	int wp;
	int rp;
	int ep;
	int i;
	int w;
	unsigned char *m;
	unsigned char *morg;
	unsigned char *matrix;
	CUnit *mine;
	int destx;
	int desty;

	destx = x = (nx != -1 ? nx : worker->X);
	desty = y = (ny != -1 ? ny : worker->Y);
	size = Map.Info.MapWidth * Map.Info.MapHeight / 4;
	points = new p[size];

	//
	// Make movement matrix. FIXME: can create smaller matrix.
	//
	morg = MakeMatrix();
	w = Map.Info.MapWidth + 2;
	matrix = morg + w + w + 2;

	points[0].X = x;
	points[0].Y = y;
	rp = 0;
	matrix[x + y * w] = 1; // mark start point
	ep = wp = 1; // start with one point

	mask = worker->Type->MovementMask;

	//
	// Pop a point from stack, push all neighbors which could be entered.
	//
	for (;;) {
		while (rp != ep) {
			rx = points[rp].X;
			ry = points[rp].Y;
			for (i = 0; i < 8; ++i) { // mark all neighbors
				x = rx + xoffset[i];
				y = ry + yoffset[i];
				m = matrix + x + y * w;
				if (*m) { // already checked
					continue;
				}
				//
				// Look if there is a mine
				//
				if ((mine = ResourceOnMap(x, y, resource))) {
					int buildings;
					int j;
					int minx;
					int maxx;
					int miny;
					int maxy;
					int nunits;
					CUnit *units[UnitMax];

					buildings = 0;

					//
					// Check units around mine
					//
					minx = mine->X - 5;
					miny = mine->Y - 5;
					maxx = mine->X + mine->Type->TileWidth + 5; 
					maxy = mine->Y + mine->Type->TileHeight + 5;
					Map.FixSelectionArea(minx, miny, maxx, maxy);
					nunits = Map.SelectFixed(minx, miny, maxx, maxy, units);
					for (j = 0; j < nunits; ++j) {
						// Enemy near mine
						if (AiPlayer->Player->Enemy &
									 (1 << units[j]->Player->Index)) {
							break;
						}
						// Town hall near mine
						if (units[j]->Type->CanStore[resource]) {
							break;
						}
						// Town hall may not be near but we may be using it, check
						// for 2 buildings near it and assume it's been used
						if (units[j]->Type->Building &&
							!units[j]->Type->GivesResource == resource) {
							++buildings;
							if (buildings == 2) {
								break;
							}
						}
					}
					if (j == nunits) {
						if (AiFindBuildingPlace2(worker, type, x, y, dx, dy)) {
							delete[] morg;
							delete[] points;
							return 1;
						}
					}
				}

				if (CanMoveToMask(x, y, mask)) { // reachable
					*m = 1;
					points[wp].X = x; // push the point
					points[wp].Y = y;
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
**  Find free building place for lumber mill. (flood fill version)
**
**  @param worker  Worker to build building.
**  @param type    Type of building.
**  @param nx      Start search X position (if == -1 then unit X pos used).
**  @param ny      Start search X position (if == -1 then unit X pos used).
**  @param dx      Pointer for X position returned.
**  @param dy      Pointer for Y position returned.
**
**  @return        True if place found, false if not found.
**
**  @todo          FIXME: This is slow really slow, using two flood
**                 fills, is not a perfect solution.
*/
static int AiFindLumberMillPlace(const CUnit *worker, const CUnitType *type, 
	int nx,int ny, int *dx, int *dy)
{
	static const int xoffset[] = { 0, -1, +1, 0, -1, +1, -1, +1 };
	static const int yoffset[] = { -1, 0, 0, +1, -1, -1, +1, +1 };
	struct p {
		unsigned short X;
		unsigned short Y;
	} *points;
	int size;
	int x;
	int y;
	int rx;
	int ry;
	int mask;
	int wp;
	int rp;
	int ep;
	int i;
	int w;
	unsigned char *m;
	unsigned char *morg;
	unsigned char *matrix;

	x = nx != -1 ? nx : worker->X;
	y = ny != -1 ? ny : worker->Y;
	size = Map.Info.MapWidth * Map.Info.MapHeight / 4;
	points = new p[size];

	//
	// Make movement matrix.
	//
	morg = MakeMatrix();
	w = Map.Info.MapWidth + 2;
	matrix = morg + w + w + 2;

	points[0].X = x;
	points[0].Y = y;
	rp = 0;
	matrix[x + y * w] = 1; // mark start point
	ep = wp = 1; // start with one point

	mask = worker->Type->MovementMask;

	//
	// Pop a point from stack, push all neightbors which could be entered.
	//
	for (;;) {
		while (rp != ep) {
			rx = points[rp].X;
			ry = points[rp].Y;
			for (i = 0; i < 8; ++i) { // mark all neighbors
				x = rx + xoffset[i];
				y = ry + yoffset[i];
				m = matrix + x + y * w;
				if (*m) { // already checked
					continue;
				}
				//
				// Look if there is wood
				//
				if (Map.ForestOnMap(x, y)) {
					if (AiFindBuildingPlace2(worker, type, x, y, dx, dy)) {
						delete[] morg;
						delete[] points;
						return 1;
					}
				}

				if (CanMoveToMask(x, y, mask)) { // reachable
					*m = 1;
					points[wp].X = x; // push the point
					points[wp].Y = y;
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

static int AiFindMiningPlace(const CUnit *worker,
						 const CUnitType *type,
						 int nx, int ny, 
						 int *dx, int *dy, 
						 int resource)
{
	static const int xoffset[] = { 0, -1, +1, 0, -1, +1, -1, +1 };
	static const int yoffset[] = { -1, 0, 0, +1, -1, -1, +1, +1 };
	struct p {
		unsigned short X;
		unsigned short Y;
	} *points;
	int size;
	int x;
	int y;
	int rx;
	int ry;
	int mask;
	int wp;
	int rp;
	int ep;
	int i;
	int w;
	unsigned char *m;
	unsigned char *morg;
	unsigned char *matrix;
	CUnit *mine;
	int destx;
	int desty;

	destx = x = (nx != -1 ? nx : worker->X);
	desty = y = (ny != -1 ? ny : worker->Y);
	size = Map.Info.MapWidth * Map.Info.MapHeight / 4;
	points = new p[size];

	//
	// Make movement matrix. FIXME: can create smaller matrix.
	//
	morg = MakeMatrix();
	w = Map.Info.MapWidth + 2;
	matrix = morg + w + w + 2;

	points[0].X = x;
	points[0].Y = y;
	rp = 0;
	//if(worker->X == x && worker->Y == y)
		matrix[x + y * w] = 1; // mark start point
	ep = wp = 1; // start with one point

	mask = worker->Type->MovementMask;

	//
	// Pop a point from stack, push all neighbors which could be entered.
	//
	for (;;) {
		while (rp != ep) {
			rx = points[rp].X;
			ry = points[rp].Y;
			for (i = 0; i < 8; ++i) { // mark all neighbors
				x = rx + xoffset[i];
				y = ry + yoffset[i];
				m = matrix + x + y * w;
				if (*m) { // already checked
					continue;
				}
				//
				// Look if there is a mine area
				//
				if ((mine = ResourceOnMap(x, y, resource, false)) &&
						 AiFindBuildingPlace2(worker, type, mine->X, mine->Y, dx, dy)) {
							delete[] morg;
							delete[] points;
							return 1;
				}

				if (CanMoveToMask(x, y, mask)) { // reachable
					*m = 1;
					points[wp].X = x; // push the point
					points[wp].Y = y;
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
**  @param nx      Start search near X position (or worker->X if nx == -1).
**  @param ny      Start search near Y position (or worker->Y if ny == -1).
**  @param dx      Pointer for X position returned.
**  @param dy      Pointer for Y position returned.
**
**  @return        True if place found, false if no found.
**
**  @todo          Better and faster way to find building place of oil
**                 platforms Special routines for special buildings.
*/
int AiFindBuildingPlace(const CUnit *worker, const CUnitType *type, 
		int nx, int ny, int *dx, int *dy)
{
	//
	// Find a good place for a new hall
	//
	DebugPrint("%d: Want to build a %s(%s)\n" _C_ AiPlayer->Player->Index 
		_C_ type->Ident.c_str() _C_ type->Name.c_str());

	//Mines and Depots
	for (int i = 0; i < MaxCosts; ++i) {
		ResourceInfo *resinfo= worker->Type->ResInfo[i];
		//Depots
		if(type->CanStore[i]) {
			if (resinfo && resinfo->TerrainHarvester) {
				return AiFindLumberMillPlace(worker, type, 	nx, ny,dx, dy);
			} else {
				return AiFindHallPlace(worker, type, nx, ny, dx, dy, i);
			}
		} else
			//mines
			if(type->GivesResource == i) {
				if (resinfo && resinfo->RefineryHarvester) {
					//Mine have to be build ONTOP resources
					return AiFindMiningPlace(worker, type, nx, ny,  dx, dy, i);
				} else {
					//Mine can be build without resource restrictions: solar panels, etc
					return AiFindBuildingPlace2(worker, type, 
							(nx != -1 ? nx : worker->X),
							(ny != -1 ? ny : worker->Y), dx, dy);
				}
			}
	}
	
	return AiFindBuildingPlace2(worker, type, 
			(nx != -1 ? nx : worker->X), (ny != -1 ? ny : worker->Y), dx, dy);
}

//@}
