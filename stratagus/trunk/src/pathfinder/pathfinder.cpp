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
/**@name pathfinder.cpp - The path finder routines. */
//
//      I use breadth-first.
//
//      (c) Copyright 1998-2005 by Lutz Sammer, Russell Smith
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
-- Includes
----------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

#include "stratagus.h"
#include "video.h"
#include "tileset.h"
#include "map.h"
#include "unittype.h"
#include "player.h"
#include "unit.h"
#include "pathfinder.h"
#include "missile.h"
#include "ui.h"

#ifndef MAX_PATH_LENGTH
#define MAX_PATH_LENGTH  9 /// Maximal path part returned.
#endif

#define USE_BEST  /// Goto best point, don't stop.

/*----------------------------------------------------------------------------
--  Variables
----------------------------------------------------------------------------*/

/**
**  The matrix is used to generated the paths.
**
**  0:      Nothing must check if usable.
**  1-8:    Field on the path 1->2->3->4->5...
**  88:     Marks the possible goal fields.
**  98:     Marks map border, for faster limits checks.
*/
unsigned char Matrix[(MaxMapWidth + 2) * (MaxMapHeight + 3) + 2];  /// Path matrix
static unsigned int LocalMatrix[MaxMapWidth * MaxMapHeight];

/*----------------------------------------------------------------------------
--  Functions
----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
--  PATH-FINDER LOW-LEVEL
----------------------------------------------------------------------------*/

/**
**  Initialize a matrix
**
**  @note  Double border for ships/flyers.
**
**    98 98 98 98 98
**    98 98 98 98 98
**    98          98
**    98          98
**    98 98 98 98 98
*/
static void InitMatrix(unsigned char *matrix)
{
	unsigned i;
	unsigned w;
	unsigned h;
	unsigned e;

	w = Map.Info.MapWidth + 2;
	h = Map.Info.MapHeight;

	i = w + w + 1;
	memset(matrix, 98, i);          // +1 for ships!
	memset(matrix + i, 0, w * h);   // initialize matrix

	for (e = i + w * h; i < e;) {   // mark left and right border
		matrix[i] = 98;
		i += w;
		matrix[i - 1] = 98;
	}
	memset(matrix + i, 98, w + 1);  // +1 for ships!
}

static void InitLocalMatrix(void)
{
	memset(LocalMatrix, 0, Map.Info.MapWidth * Map.Info.MapHeight * sizeof(int)); // initialize matrix
}

/**
**  Create empty movement matrix.
*/
unsigned char *CreateMatrix(void)
{
	InitMatrix(Matrix);
	return Matrix;
}

/**
**  Allocate a new matrix and initialize
*/
unsigned char *MakeMatrix(void)
{
	unsigned char *matrix;

	matrix = new unsigned char[(Map.Info.MapWidth + 2) * (Map.Info.MapHeight + 3) + 2];
	InitMatrix(matrix);

	return matrix;
}

/**
**  Mark place in matrix.
**
**  @param gx       X position of target area
**  @param gy       Y position of target area
**  @param gw       Width of target area
**  @param gh       Height of target area
**  @param range    Range to search at
**  @param matrix   Target area marked in matrix
**
**  @returns        depth, -1 unreachable
*/
static int CheckPlaceInMatrix(int gx, int gy, int gw, int gh, int range, unsigned int *matrix)
{
	int cx[4];
	int cy[4];
	int steps;
	int cycle;
	int x;
	int y;
	int quad;
	int filler;

	if (range == 0 && gw == 0 && gh == 0) {
		return matrix[gx + gy * Map.Info.MapWidth];
	}

	// Mark top, bottom, left, right

	// Mark Top and Bottom of Goal
	for (x = gx; x <= gx + gw; ++x) {
		if (x >= 0 && x < Map.Info.MapWidth) {
			if ( gy - range >= 0 && matrix[(gy - range) * Map.Info.MapWidth + x]) {
				return 1;
			}
			if (gy + range + gh < Map.Info.MapHeight && matrix[(gy + range + gh) * Map.Info.MapWidth + x]) {
				return 1;
			}
		}
	}

	for (y = gy; y <= gy + gh; ++y) {
		if (y >= 0 && y < Map.Info.MapHeight) {
			if (gx - range >= 0 && matrix[y * Map.Info.MapWidth + gx - range]) {
				return 1;
			}
			if (gx + gw + range < Map.Info.MapWidth && matrix[y * Map.Info.MapWidth + gx + gw + range]) {
				return 1;
			}
		}
	}

	// Mark Goal Border in Matrix

	// Mark Edges of goal

	steps = 0;
	// Find place to start. (for marking curves)
	while(VisionTable[0][steps] != range && VisionTable[1][steps] == 0 && VisionTable[2][steps] == 0) {
		steps++;
	}
	// 0 - Top right Quadrant
	cx[0] = gx + gw;
	cy[0] = gy - VisionTable[0][steps];
	// 1 - Top left Quadrant
	cx[1] = gx;
	cy[1] = gy - VisionTable[0][steps];
	// 2 - Bottom Left Quadrant
	cx[2] = gx;
	cy[2] = gy + VisionTable[0][steps]+gh;
	// 3 - Bottom Right Quadrant
	cx[3] = gx + gw;
	cy[3] = gy + VisionTable[0][steps]+gh;

	++steps;  // Move past blank marker
	while (VisionTable[1][steps] != 0 || VisionTable[2][steps] != 0) {
		// Loop through for repeat cycle
		cycle = 0;
		while (cycle++ < VisionTable[0][steps]) {
			// If we travelled on an angle, mark down as well.
			if (VisionTable[1][steps] == VisionTable[2][steps]) {
				// do down
				quad = 0;
				while (quad < 4) {
					if (quad < 2) {
						filler = 1;
					} else {
						filler = -1;
					}
					if (cx[quad] >= 0 && cx[quad] < Map.Info.MapWidth && cy[quad] + filler >= 0 &&
						cy[quad] + filler < Map.Info.MapHeight && matrix[(cy[quad] + filler) * Map.Info.MapWidth + cx[quad]]) {
						return 1;
					}
					++quad;
				}
			}

			cx[0] += VisionTable[1][steps];
			cy[0] += VisionTable[2][steps];
			cx[1] -= VisionTable[1][steps];
			cy[1] += VisionTable[2][steps];
			cx[2] -= VisionTable[1][steps];
			cy[2] -= VisionTable[2][steps];
			cx[3] += VisionTable[1][steps];
			cy[3] -= VisionTable[2][steps];

			// Mark Actually Goal curve change
			quad = 0;
			while (quad < 4) {
				if (cx[quad] >= 0 && cx[quad] < Map.Info.MapWidth && cy[quad] >= 0 &&
					cy[quad] < Map.Info.MapHeight &&
					matrix[cy[quad] * Map.Info.MapWidth + cx[quad]] ) {
					return 1;
				}
				++quad;
			}
		}
		++steps;
	}
	return 0;
}

/**
**  Flood fill an area for a matrix.
**
**  This use the flood-fill algorithms.
**  @todo can be done faster, if starting from both sides.
**
**  @param unit     Path for this unit.
**  @param matrix   Matrix for calculation.
**
*/
static void FillMatrix(const CUnit *unit, unsigned int *matrix)
{
	struct p {
		unsigned short X;
		unsigned short Y;
		int depth;
	} *points;
	int x;
	int y;
	int rx;
	int ry;
	int mask;
	int wp;
	int rp;
	int ep;
	int n;
	int j;
	int depth;
	int size;
	unsigned int *m;

	size = 4 * (Map.Info.MapWidth + Map.Info.MapHeight);
	points = new p[size];

	mask = unit->Type->MovementMask;
	// Ignore all possible mobile units.
	// FIXME: bad? mask&=~(MapFieldLandUnit|MapFieldAirUnit|MapFieldSeaUnit);

	points[0].X = x = unit->X;
	points[0].Y = y = unit->Y;
	points[0].depth = 1;
	rp = 0;
	matrix[x + y * Map.Info.MapWidth] = depth = 1;   // mark start point
	ep = wp = 1;                                // start with one point
	n = 2;

	//
	//  Pop a point from stack, push all neightbors which could be entered.
	//
	for (;;) {
		while (rp != ep) {
			rx = points[rp].X;
			ry = points[rp].Y;
			depth = points[rp].depth;
			for (j = 0; j < 8; ++j) {       // mark all neighbors
				x = rx + Heading2X[j];
				y = ry + Heading2Y[j];
				if (x < 0 || y < 0 || x >= Map.Info.MapWidth || y >= Map.Info.MapHeight) {
					// Outside the map
					continue;
				}
				m = matrix + x + y * Map.Info.MapWidth;
				if (*m) {
					continue;
				}
				if (CanMoveToMask(x, y, mask)) {    // reachable
					*m = depth + 1;
					points[wp].X = x;   // push the point
					points[wp].Y = y;
					points[wp].depth = depth + 1;
					if (++wp >= size) {             // round about
						wp = 0;
					}
				} else {                             // unreachable
					*m = 0;
				}
			}

			// Loop for
			if (++rp >= size) { // round about
				rp = 0;
			}
		}

		//
		//  Continue with next frame.
		//
		if (rp == wp) {  // unreachable, no more points available
			break;
		}
		ep = wp;
	}

	delete[] points;
}

/*----------------------------------------------------------------------------
--  PATH-FINDER USE
----------------------------------------------------------------------------*/

/**
**  Can the unit 'src' reach the place x,y.
**
**  @param src          Unit for the path.
**  @param x            Map X tile position.
**  @param y            Map Y tile position.
**  @param w            Width of Goal
**  @param h            Height of Goal
**  @param minrange     min range to the tile
**  @param range        Range to the tile.
**
**  @return         Distance to place.
*/
int PlaceReachable(const CUnit *src, int x, int y, int w, int h, int minrange, int range)
{
	int depth;
	static unsigned long LastGameCycle;
	static unsigned mask;

	//
	//  Setup movement.
	//
	if (src->Type->MovementMask != mask || LastGameCycle != GameCycle
		|| LocalMatrix[src->X + src->Y * Map.Info.MapWidth] == 0) {
		InitLocalMatrix();
		FillMatrix(src, LocalMatrix);
		LastGameCycle = GameCycle;
		mask = src->Type->MovementMask;
	}

	//
	//  Find a path to the place.
	//
	if ((depth = CheckPlaceInMatrix(x, y, w, h, range, LocalMatrix)) < 0) {
		DebugPrint("Can't move to destination, not route to goal\n");
		return 0;
	}

	return depth;
}

/**
**  Can the unit 'src' reach the unit 'dst'.
**
**  @param src      Unit for the path.
**  @param dst      Unit to be reached.
**  @param range    Range to unit.
**
**  @return  Distance to place.
*/
int UnitReachable(const CUnit *src, const CUnit *dst, int range)
{
	int depth;

	//
	//  Find a path to the goal.
	//
	depth = PlaceReachable(src, dst->X, dst->Y, dst->Type->TileWidth, dst->Type->TileHeight, 0, range);
	if (depth <= 0) {
		return 0;
	}

	return depth;
}

/*----------------------------------------------------------------------------
--  REAL PATH-FINDER
----------------------------------------------------------------------------*/

/**
**  Find new path.
**
**  The destination could be an unit or a field.
**  Range gives how far we must reach the goal.
**
**  @note  The destination could become negative coordinates!
**
**  @param unit  Path for this unit.
**
**  @return         >0 remaining path length, 0 wait for path, -1
**                  reached goal, -2 can't reach the goal.
*/
int NewPath(CUnit *unit)
{
	int i;
	int gw;
	int gh;
	int gx;
	int gy;
	int minrange;
	int maxrange;
	char *path;

	if (unit->Orders[0]->Goal) {
		gw = unit->Orders[0]->Goal->Type->TileWidth;
		gh = unit->Orders[0]->Goal->Type->TileHeight;
		gx = unit->Orders[0]->Goal->X;
		gy = unit->Orders[0]->Goal->Y;
		maxrange = unit->Orders[0]->Range;
		minrange = unit->Orders[0]->MinRange;
	} else {
		// Take care of non square goals :)
		// If goal is non square, range states a non-existant goal rather
		// than a tile.
		gw = unit->Orders[0]->Width;
		gh = unit->Orders[0]->Height;
		maxrange = unit->Orders[0]->Range;
		minrange = unit->Orders[0]->MinRange;
		gx = unit->Orders[0]->X;
		gy = unit->Orders[0]->Y;
	}
	path = unit->Data.Move.Path;
	i = AStarFindPath(unit,gx,gy,gw,gh,minrange,maxrange,path);
	if (i == PF_FAILED) {
		i = PF_UNREACHABLE;
	}

	// Update path if it was requested. Otherwise we may only want
	// to know if there exists a path.

	if (path != NULL) {
		if (i >= MAX_PATH_LENGTH) {
			unit->Data.Move.Length = MAX_PATH_LENGTH;
		} else {
			unit->Data.Move.Length = i;
		}
		if (unit->Data.Move.Length == 0) {
			++unit->Data.Move.Length;
		}
	}
	return i;
}

//@}
