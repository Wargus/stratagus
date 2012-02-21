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
//      (c) Copyright 1998-2007 by Lutz Sammer, Russell Smith
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
#include "map.h"
#include "unittype.h"
#include "unit.h"
#include "pathfinder.h"
#include "actions.h"

//astar.cpp

/// Init the a* data structures
extern void InitAStar(int mapWidth, int mapHeight,
	 int (STDCALL *costMoveTo)(unsigned int index, void *data));

/// free the a* data structures
extern void FreeAStar();

/// Find and a* path for a unit
extern int AStarFindPath(int sx, int sy, int gx, int gy, int gw, int gh,
	int tilesizex, int tilesizey, int minrange,
	 int maxrange, char *path, int pathlen, void *data);

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

/**
**  Init the pathfinder
*/
void InitPathfinder()
{
	InitAStar(Map.Info.MapWidth, Map.Info.MapHeight, NULL);
}

/**
**  Free the pathfinder
*/
void FreePathfinder()
{
	FreeAStar();
}

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
unsigned char *CreateMatrix()
{
	InitMatrix(Matrix);
	return Matrix;
}

/**
**  Allocate a new matrix and initialize
*/
unsigned char *MakeMatrix()
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
int PlaceReachable(const CUnit &src, int x, int y, int w, int h,
	 int minrange, int range)
{
	int i = AStarFindPath(src.tilePos.x, src.tilePos.y, x, y, w, h,
		src.Type->TileWidth, src.Type->TileHeight,
		 minrange, range, NULL, 0, const_cast<CUnit*>(&src));

	switch (i) {
		case PF_FAILED:
		case PF_UNREACHABLE:
			i = 0;
			break;
		case PF_REACHED:
			/* sice most of this function usage check return value as bool
			 * then reached state should be track as true value */
			i = 1;
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
int UnitReachable(const CUnit &src, const CUnit &dst, int range)
{
	//  Find a path to the goal.
	if (src.Type->Building)
		return 0;
	const int depth = PlaceReachable(src, dst.tilePos.x, dst.tilePos.y,
		dst.Type->TileWidth, dst.Type->TileHeight, 0, range);
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
**  The destination could be a unit or a field.
**  Range gives how far we must reach the goal.
**
**  @note  The destination could become negative coordinates!
**
**  @param unit  Path for this unit.
**
**  @return      >0 remaining path length, 0 wait for path, -1
**               reached goal, -2 can't reach the goal.
*/
int NewPath(CUnit &unit)
{
	int i;
	int gw;
	int gh;
	int gx;
	int gy;
	COrderPtr order = unit.CurrentOrder();
	int minrange = order->MinRange;
	int maxrange = order->Range;
	char *path;

	if (order->HasGoal()) {
		CUnit *goal = order->GetGoal();
		gw = goal->Type->TileWidth;
		gh = goal->Type->TileHeight;
		gx = goal->tilePos.x;
		gy = goal->tilePos.y;
	} else {
		// Take care of non square goals :)
		// If goal is non square, range states a non-existant goal rather
		// than a tile.
		gw = order->Width;
		gh = order->Height;
		// Large units may have a goal that goes outside the map, fix it here
		if (order->goalPos.x + unit.Type->TileWidth - 1 >= Map.Info.MapWidth) {
			order->goalPos.x = Map.Info.MapWidth - unit.Type->TileWidth;
		}
		if (order->goalPos.y + unit.Type->TileHeight - 1 >= Map.Info.MapHeight) {
			order->goalPos.y = Map.Info.MapHeight - unit.Type->TileHeight;
		}
		gx = order->goalPos.x;
		gy = order->goalPos.y;
	}
	path = unit.CurrentOrder()->Data.Move.Path;
	i = AStarFindPath(unit.tilePos.x, unit.tilePos.y, gx, gy, gw, gh,
		unit.Type->TileWidth, unit.Type->TileHeight,
		 minrange, maxrange, path, MAX_PATH_LENGTH, &unit);
	if (i == PF_FAILED) {
		i = PF_UNREACHABLE;
	}

	// Update path if it was requested. Otherwise we may only want
	// to know if there exists a path.

	if (path != NULL) {
		if (i >= MAX_PATH_LENGTH) {
			unit.CurrentOrder()->Data.Move.Length = MAX_PATH_LENGTH;
		} else {
			unit.CurrentOrder()->Data.Move.Length = i;
		}
		if (unit.CurrentOrder()->Data.Move.Length == 0) {
			++unit.CurrentOrder()->Data.Move.Length;
		}
	}
	return i;
}

/**
**  Returns the next element of a path.
**
**  @param unit  Unit that wants the path element.
**  @param pxd   Pointer for the x direction.
**  @param pyd   Pointer for the y direction.
**
**  @return >0 remaining path length, 0 wait for path, -1
**  reached goal, -2 can't reach the goal.
*/
int NextPathElement(CUnit &unit, short int *pxd, short int *pyd)
{
	int result;
	COrderPtr order = unit.CurrentOrder();
	CUnit *goal;
	// Attempt to use path cache
	// FIXME: If there is a goal, it may have moved, ruining the cache
	*pxd = 0;
	*pyd = 0;

	// Goal has moved, need to recalculate path or no cached path
	if (unit.CurrentOrder()->Data.Move.Length <= 0 ||
		((goal = order->GetGoal()) && goal->tilePos != order->goalPos)) {
		result = NewPath(unit);

		if (result == PF_UNREACHABLE) {
			unit.CurrentOrder()->Data.Move.Length = 0;
			return result;
		}
		if (result == PF_REACHED) {
			return result;
		}
		if (unit.Goal) {
			// Update Orders
			order->goalPos = unit.Goal->tilePos;
		}
	}

	*pxd = Heading2X[(int)unit.CurrentOrder()->Data.Move.Path[(int)unit.CurrentOrder()->Data.Move.Length - 1]];
	*pyd = Heading2Y[(int)unit.CurrentOrder()->Data.Move.Path[(int)unit.CurrentOrder()->Data.Move.Length - 1]];
	const Vec2i dir = {*pxd, *pyd};
	result = unit.CurrentOrder()->Data.Move.Length;
	unit.CurrentOrder()->Data.Move.Length--;
	if (!UnitCanBeAt(unit, unit.tilePos + dir)) {
		// If obstructing unit is moving, wait for a bit.
		if (unit.CurrentOrder()->Data.Move.Fast) {
			unit.CurrentOrder()->Data.Move.Fast--;
			AstarDebugPrint("WAIT at %d\n" _C_ unit.CurrentOrder()->Data.Move.Fast);
			result = PF_WAIT;
		} else {
			unit.CurrentOrder()->Data.Move.Fast = 10;
			AstarDebugPrint("SET WAIT to 10\n");
			result = PF_WAIT;
		}
		if (unit.CurrentOrder()->Data.Move.Fast == 0 && result != 0) {
			AstarDebugPrint("WAIT expired\n");
			result = NewPath(unit);
			if (result > 0) {
				*pxd = Heading2X[(int)unit.CurrentOrder()->Data.Move.Path[(int)unit.CurrentOrder()->Data.Move.Length - 1]];
				*pyd = Heading2Y[(int)unit.CurrentOrder()->Data.Move.Path[(int)unit.CurrentOrder()->Data.Move.Length - 1]];
				if (!UnitCanBeAt(unit, unit.tilePos + dir)) {
					// There may be unit in the way, Astar may allow you to walk onto it.
					result = PF_UNREACHABLE;
					*pxd = 0;
					*pyd = 0;
				} else {
					result = unit.CurrentOrder()->Data.Move.Length;
					unit.CurrentOrder()->Data.Move.Length--;
				}
			}
		}
	}
	if (result != PF_WAIT) {
		unit.CurrentOrder()->Data.Move.Fast = 0;
	}
	return result;
}

//@}
