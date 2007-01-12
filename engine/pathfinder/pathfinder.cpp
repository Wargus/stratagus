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
//      (c) Copyright 1998-2006 by Lutz Sammer, Russell Smith
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

#include "stratagus.h"
#include "map.h"
#include "unittype.h"
#include "unit.h"
#include "pathfinder.h"

#ifndef MAX_PATH_LENGTH
#define MAX_PATH_LENGTH  9 /// Maximal path part returned.
#endif

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
static unsigned char Matrix[(MaxMapWidth + 2) * (MaxMapHeight + 3) + 2];  /// Path matrix

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

/*----------------------------------------------------------------------------
--  PATH-FINDER USE
----------------------------------------------------------------------------*/

/**
**  Can the unit 'src' reach the place x,y.
**
**  @param src       Unit for the path.
**  @param x         Map X tile position.
**  @param y         Map Y tile position.
**  @param w         Width of Goal
**  @param h         Height of Goal
**  @param minrange  min range to the tile
**  @param range     Range to the tile.
**
**  @return          Distance to place.
*/
int PlaceReachable(const CUnit *src, int x, int y, int w, int h, int minrange, int range)
{
	int i = AStarFindPath(src, x, y, w, h, minrange, range, NULL);
	switch (i) {
		case PF_FAILED:
		case PF_UNREACHABLE:
		case PF_REACHED:
			i = 0;
			break;
		case PF_WAIT:
			Assert(0);
			i = 0;
			break;
		case PF_MOVE:
			break;
		default:
			break;
	}

	return i;
}

/**
**  Can the unit 'src' reach the unit 'dst'.
**
**  @param src    Unit for the path.
**  @param dst    Unit to be reached.
**  @param range  Range to unit.
**
**  @return       Distance to place.
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
**  @return      >0 remaining path length, 0 wait for path, -1
**               reached goal, -2 can't reach the goal.
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
		// Large units may have a goal that goes outside the map, fix it here
		if (unit->Orders[0]->X + unit->Type->TileWidth - 1 >= Map.Info.MapWidth) {
			unit->Orders[0]->X = Map.Info.MapWidth - unit->Type->TileWidth;
		}
		if (unit->Orders[0]->Y + unit->Type->TileHeight - 1 >= Map.Info.MapHeight) {
			unit->Orders[0]->Y = Map.Info.MapHeight - unit->Type->TileHeight;
		}
		gx = unit->Orders[0]->X;
		gy = unit->Orders[0]->Y;
	}
	path = unit->Data.Move.Path;
	i = AStarFindPath(unit, gx, gy, gw, gh, minrange, maxrange, path);
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
