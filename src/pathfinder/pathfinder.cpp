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
			/* since most of this function usage check return value as bool
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

PathFinderInput::PathFinderInput() : unit(NULL), minRange(0), maxRange(0),
	isRecalculatePathNeeded(true)
{
	unitSize.x = 0;
	unitSize.y = 0;
	goalPos.x = -1;
	goalPos.y = -1;
	goalSize.x = 0;
	goalSize.y = 0;
}

const Vec2i& PathFinderInput::GetUnitPos() const { return unit->tilePos; }
Vec2i PathFinderInput::GetUnitSize() const {
	const Vec2i tileSize = {unit->Type->TileWidth, unit->Type->TileHeight};
	return tileSize;
}

void PathFinderInput::SetUnit(CUnit &_unit) {
	unit = &_unit;

	isRecalculatePathNeeded = true;
}


void PathFinderInput::SetGoal(const Vec2i& pos, const Vec2i& size) {
	Vec2i newPos = pos;
	// Large units may have a goal that goes outside the map, fix it here
	if (newPos.x + unit->Type->TileWidth - 1 >= Map.Info.MapWidth) {
		newPos.x = Map.Info.MapWidth - unit->Type->TileWidth;
	}
	if (newPos.y + unit->Type->TileHeight - 1 >= Map.Info.MapHeight) {
		newPos.y = Map.Info.MapHeight - unit->Type->TileHeight;
	}
	if (goalPos != newPos || goalSize != size) {
		isRecalculatePathNeeded = true;
	}
	goalPos = newPos;
	goalSize = size;
}

void PathFinderInput::SetMinRange(int range) {
	if (minRange != range) {
		minRange = range;
		isRecalculatePathNeeded = true;
	}
}

void PathFinderInput::SetMaxRange(int range) {
	if (maxRange != range) {
		maxRange = range;
		isRecalculatePathNeeded = true;
	}
}

void PathFinderInput::PathRacalculated() {
	unitSize.x = unit->Type->TileWidth;
	unitSize.y = unit->Type->TileHeight;

	isRecalculatePathNeeded = false;
}


PathFinderOutput::PathFinderOutput()
{
	memset(this, 0, sizeof (*this));
}

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
static int NewPath(PathFinderInput& input, PathFinderOutput& output)
{
	char *path = output.Path;
	int i = AStarFindPath(input.GetUnitPos().x, input.GetUnitPos().y,
						input.GetGoalPos().x, input.GetGoalPos().y,
						input.GetGoalSize().x, input.GetGoalSize().y,
						input.GetUnitSize().x, input.GetUnitSize().y,
						input.GetMinRange(), input.GetMaxRange(),
						path, PathFinderOutput::MAX_PATH_LENGTH,
						input.GetUnit());
	input.PathRacalculated();
	if (i == PF_FAILED) {
		i = PF_UNREACHABLE;
	}

	// Update path if it was requested. Otherwise we may only want
	// to know if there exists a path.
	if (path != NULL) {
		output.Length = std::min<int>(i, PathFinderOutput::MAX_PATH_LENGTH);
		if (output.Length == 0) {
			++output.Length;
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
	PathFinderInput& input = unit.pathFinderData->input;
	PathFinderOutput& output = unit.pathFinderData->output;

	unit.CurrentOrder()->UpdatePathFinderData(input);
	// Attempt to use path cache
	// FIXME: If there is a goal, it may have moved, ruining the cache
	*pxd = 0;
	*pyd = 0;

	// Goal has moved, need to recalculate path or no cached path
	if (output.Length <= 0 || input.IsRecalculateNeeded()) {
		result = NewPath(input, output);

		if (result == PF_UNREACHABLE) {
			output.Length = 0;
			return result;
		}
		if (result == PF_REACHED) {
			return result;
		}
	}

	*pxd = Heading2X[(int)output.Path[(int)output.Length - 1]];
	*pyd = Heading2Y[(int)output.Path[(int)output.Length - 1]];
	const Vec2i dir = {*pxd, *pyd};
	result = output.Length;
	output.Length--;
	if (!UnitCanBeAt(unit, unit.tilePos + dir)) {
		// If obstructing unit is moving, wait for a bit.
		if (output.Fast) {
			output.Fast--;
			AstarDebugPrint("WAIT at %d\n" _C_ output.Fast);
			result = PF_WAIT;
		} else {
			output.Fast = 10;
			AstarDebugPrint("SET WAIT to 10\n");
			result = PF_WAIT;
		}
		if (output.Fast == 0 && result != 0) {
			AstarDebugPrint("WAIT expired\n");
			result = NewPath(input, output);
			if (result > 0) {
				*pxd = Heading2X[(int)output.Path[(int)output.Length - 1]];
				*pyd = Heading2Y[(int)output.Path[(int)output.Length - 1]];
				if (!UnitCanBeAt(unit, unit.tilePos + dir)) {
					// There may be unit in the way, Astar may allow you to walk onto it.
					result = PF_UNREACHABLE;
					*pxd = 0;
					*pyd = 0;
				} else {
					result = output.Length;
					output.Length--;
				}
			}
		}
	}
	if (result != PF_WAIT) {
		output.Fast = 0;
	}
	return result;
}

//@}
