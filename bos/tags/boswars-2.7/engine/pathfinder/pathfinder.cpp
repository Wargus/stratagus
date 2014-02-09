//     ____                _       __               
//    / __ )____  _____   | |     / /___ ___________
//   / __  / __ \/ ___/   | | /| / / __ `/ ___/ ___/
//  / /_/ / /_/ (__  )    | |/ |/ / /_/ / /  (__  ) 
// /_____/\____/____/     |__/|__/\__,_/_/  /____/  
//                                              
//       A futuristic real-time strategy game.
//          This file is part of Bos Wars.
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

static int STDCALL CostMoveTo(int x, int y, void *data);

/*----------------------------------------------------------------------------
--  Functions
----------------------------------------------------------------------------*/

/**
**  Init the pathfinder
*/
void InitPathfinder()
{
	InitAStar(Map.Info.MapWidth, Map.Info.MapHeight, CostMoveTo);
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
**  @pre             The unit's field flags must have been marked
**                   on the map; see MarkUnitFieldFlags.
**
**  @return          Distance to place.
*/
int PlaceReachable(const CUnit *src, int x, int y, int w, int h, int minrange, int range)
{
	// Please do not use UnmarkUnitFieldFlags and MarkUnitFieldFlags
	// around PlaceReachable calls.
	Assert((Map.Field(src->X, src->Y)->Flags & src->Type->FieldFlags) != 0);

	UnmarkUnitFieldFlags(src);
	int i = AStarFindPath(src->X, src->Y, x, y, w, h,
		src->Type->TileWidth, src->Type->TileHeight, minrange, range, NULL, 0, (void *)src);
	MarkUnitFieldFlags(src);

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
**  @pre          The unit's field flags must have been marked
**                on the map; see MarkUnitFieldFlags.
**
**  @return       Distance to place.
*/
int UnitReachable(const CUnit *src, const CUnit *dst, int range)
{
	int depth;

	// Please do not use UnmarkUnitFieldFlags and MarkUnitFieldFlags
	// around UnitReachable calls.
	Assert((Map.Field(src->X, src->Y)->Flags & src->Type->FieldFlags) != 0);

	//
	//  Find a path to the goal.
	//
	depth = PlaceReachable(src, dst->X, dst->Y, dst->Type->TileWidth, dst->Type->TileHeight, 0, range);
	if (depth <= 0) {
		return 0;
	}

	return depth;
}

/**
**  Compute the cost of crossing tile (dx,dy)
**
**  @param data  The CUnit * that wants to cross the tile.  This is
**               passed in as void * so that the low-level A* code
**               need not know about CUnit.
**  @param x     X tile where to move.
**  @param y     Y tile where to move.
**
**  @pre         The unit's field flags must have been unmarked
**               on the map; see UnmarkUnitFieldFlags.
**
**  @return      -1 -> impossible to cross.
**                0 -> no induced cost, except move
**               >0 -> costly tile
*/
static int STDCALL CostMoveTo(int x, int y, void *data)
{
	int i;
	int j;
	int flag;
	int cost = 0;
	CUnit *goal;
	const CUnit *unit = (const CUnit *)data;
	int mask = unit->Type->MovementMask;

	cost = 0;

	// Doesn't cost anything to move to ourselves :)
	// Used when marking goals mainly.  Could cause speed problems
	if (unit->X == x && unit->Y == y) {
		return 0;
	}

	// verify each tile of the unit.
	for (i = x; i < x + unit->Type->TileWidth; ++i) {
		for (j = y; j < y + unit->Type->TileHeight; ++j) {
			flag = Map.Field(i, j)->Flags & mask;
			if (flag && (AStarKnowUnseenTerrain || Map.IsFieldExplored(unit->Player, i, j))) {
				if (flag & ~(MapFieldLandUnit | MapFieldAirUnit | MapFieldSeaUnit)) {
					// we can't cross fixed units and other unpassable things
					return -1;
				}
				goal = UnitOnMapTile(i, j, unit->Type->UnitType);
				if (!goal) {
					// Shouldn't happen, mask says there is something on this tile
					Assert(0);
					return -1;
				}

				// The unit must not be blocked by itself.
				// Please use UnmarkUnitFieldFlags and
				// MarkUnitFieldFlags around pathfinder calls.
				Assert(goal != unit);

				if (goal->Moving)  {
					// moving unit are crossable
					cost += AStarMovingUnitCrossingCost;
				} else {
					// for non moving unit Always Fail
					// FIXME: Need support for moving a fixed unit to add cost
					return -1;
					//cost += AStarFixedUnitCrossingCost;
				}
			}
			// Add cost of crossing unknown tiles if required
			if (!AStarKnowUnseenTerrain && !Map.IsFieldExplored(unit->Player, i, j)) {
				// Tend against unknown tiles.
				cost += AStarUnknownTerrainCost;
			}
			if (unit->Type->UnitType != UnitTypeFly) {
				// Add tile movement cost
				cost += Map.Field(i, j)->Cost;
			}
		}
	}
	return cost;
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
**  @pre         The unit's field flags must have been unmarked
**               on the map; see UnmarkUnitFieldFlags.
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

	// Please use UnmarkUnitFieldFlags and MarkUnitFieldFlags
	// around NewPath calls.
	Assert((Map.Field(unit->X, unit->Y)->Flags & unit->Type->FieldFlags) == 0);

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
	i = AStarFindPath(unit->X, unit->Y, gx, gy, gw, gh,
		unit->Type->TileWidth, unit->Type->TileHeight, minrange, maxrange, path, MAX_PATH_LENGTH, unit);
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

/**
**  Returns the next element of a path.
**
**  @param unit  Unit that wants the path element.
**  @param pxd   Pointer for the x direction.
**  @param pyd   Pointer for the y direction.
**
**  @pre         The unit's field flags must have been unmarked
**               on the map; see UnmarkUnitFieldFlags.
**
**  @return >0 remaining path length, 0 wait for path, -1
**  reached goal, -2 can't reach the goal.
*/
int NextPathElement(CUnit *unit, int *pxd, int *pyd)
{
	int result;

	// Please use UnmarkUnitFieldFlags and MarkUnitFieldFlags
	// around NextPathElement calls.
	Assert((Map.Field(unit->X, unit->Y)->Flags & unit->Type->FieldFlags) == 0);

	// Attempt to use path cache
	// FIXME: If there is a goal, it may have moved, ruining the cache
	*pxd = 0;
	*pyd = 0;

	// Goal has moved, need to recalculate path or no cached path
	if (unit->Data.Move.Length <= 0 ||
		(unit->Orders[0]->Goal && (unit->Orders[0]->Goal->X != unit->Orders[0]->X ||
			unit->Orders[0]->Goal->Y != unit->Orders[0]->Y))) {
		result = NewPath(unit);

		if (result == PF_UNREACHABLE) {
			unit->Data.Move.Length = 0;
			return result;
		}
		if (result == PF_REACHED) {
			return result;
		}
		if (unit->Goal) {
			// Update Orders
			unit->Orders[0]->X = unit->Goal->X;
			unit->Orders[0]->Y = unit->Goal->Y;
		}
	}

	*pxd = Heading2X[(int)unit->Data.Move.Path[(int)unit->Data.Move.Length - 1]];
	*pyd = Heading2Y[(int)unit->Data.Move.Path[(int)unit->Data.Move.Length - 1]];
	result = unit->Data.Move.Length;
	unit->Data.Move.Length--;
	if (!UnitCanBeAt(unit, *pxd + unit->X, *pyd + unit->Y)) {
		// If obstructing unit is moving, wait for a bit.
		if (unit->Data.Move.Fast) {
			unit->Data.Move.Fast--;
			AstarDebugPrint("WAIT at %d\n" _C_ unit->Data.Move.Fast);
			result = PF_WAIT;
		} else {
			unit->Data.Move.Fast = 10;
			AstarDebugPrint("SET WAIT to 10\n");
			result = PF_WAIT;
		}
		if (unit->Data.Move.Fast == 0 && result != 0) {
			AstarDebugPrint("WAIT expired\n");
			result = NewPath(unit);
			if (result > 0) {
				*pxd = Heading2X[(int)unit->Data.Move.Path[(int)unit->Data.Move.Length - 1]];
				*pyd = Heading2Y[(int)unit->Data.Move.Path[(int)unit->Data.Move.Length - 1]];
				if (!UnitCanBeAt(unit, *pxd + unit->X, *pyd + unit->Y)) {
					// There may be unit in the way, Astar may allow you to walk onto it.
					result = PF_UNREACHABLE;
					*pxd = 0;
					*pyd = 0;
				} else {
					result = unit->Data.Move.Length;
					unit->Data.Move.Length--;
				}
			}
		}
	}
	if (result != PF_WAIT) {
		unit->Data.Move.Fast = 0;
	}
	return result;
}

//@}
