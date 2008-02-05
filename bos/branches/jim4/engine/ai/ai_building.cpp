//     ____                _       __               
//    / __ )____  _____   | |     / /___ ___________
//   / __  / __ \/ ___/   | | /| / / __ `/ ___/ ___/
//  / /_/ / /_/ (__  )    | |/ |/ / /_/ / /  (__  ) 
// /_____/\____/____/     |__/|__/\__,_/_/  /____/  
//                                              
//       A futuristic real-time strategy game.
//          This file is part of Bos Wars.
//
/**@name ai_building.cpp - AI building functions. */
//
//      (c) Copyright 2001-2007 by Lutz Sammer
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
static int AiCheckSurrounding(const CUnit *worker, const CUnitType *type, int x, int y, bool &backupok)
{
	static int dirs[5][2] = {{1,0},{0,1},{-1,0},{0,-1},{0,0}};
	int surrounding[1024]; // Max criconference for building
	int surroundingnb;
	int x0, y0, x1, y1;
	int i;
	int lastval;
	int dir;
	int obstacle;

	x0 = x - 1;
	y0 = y - 1;
	x1 = x0 + type->TileWidth + 1;
	y1 = y0 + type->TileWidth + 1;


	x = x0;
	y = y0;
	dir = -1;
	surroundingnb = 0;
	while (dir < 4) {
		if ((unsigned)x < (unsigned)Map.Info.MapWidth && (unsigned)y < (unsigned)Map.Info.MapHeight) {
			if (worker && x == worker->X && y == worker->Y) {
				surrounding[surroundingnb++] = 1;
			} else if (Map.Field(x, y)->Flags & (MapFieldUnpassable | MapFieldBuilding)) {
				surrounding[surroundingnb++] = 0;
			} else{
				// Can pass there
				surrounding[surroundingnb++] = (Map.Field(x, y)->Flags &
					(MapFieldWaterAllowed + MapFieldCoastAllowed + MapFieldLandAllowed)) != 0;;
			}
		} else {
			surrounding[surroundingnb++] = 0;
		}

		if ((x == x0 || x == x1) && (y == y0 || y == y1)) {
			dir++;
		}

		x += dirs[dir][0];
		y += dirs[dir][1];
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
	int w;
	unsigned char *m;
	unsigned char *matrix;
	int backupx = -1;
	int backupy = -1;
	bool backupok;

	size = Map.Info.MapWidth * Map.Info.MapHeight / 4;
	points = new p[size];

	x = ox;
	y = oy;
	//
	// Look if we can build at current place.
	//
	if (CanBuildUnitType(worker, type, x, y, 1)) {
		if (AiCheckSurrounding(worker, type, x, y, backupok)) {
			*dx = x;
			*dy = y;
			delete[] points;
			return 1;
		} else if (backupok) {
			backupx = x;
			backupy = y;
		}
	}
	//
	//  Make movement matrix.
	//
	matrix = CreateMatrix();
	w = Map.Info.MapWidth + 2;

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
**  Find free building place.
**
**  @param worker  Worker to build building.
**  @param type    Type of building.
**  @param dx      Pointer for X position returned.
**  @param dy      Pointer for Y position returned.
**
**  @return        True if place found, false if no found.
**
**  @todo          Better and faster way to find building place of oil
**                 platforms Special routines for special buildings.
*/
int AiFindBuildingPlace(const CUnit *worker, const CUnitType *type, int *dx, int *dy)
{
	DebugPrint("Want to build a %s(%s)\n" _C_ type->Ident.c_str() _C_ type->Name.c_str());

	return AiFindBuildingPlace2(worker, type, worker->X, worker->Y, dx, dy);
}

//@}
