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
//      (c) Copyright 1998-2007 by Lutz Sammer and Russell Smith
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

#include "pathfinder.h"

#include "actions.h"
#include "map.h"
#include "unittype.h"
#include "unit.h"

//astar.cpp

/// Init the a* data structures
extern void InitAStar(int mapWidth, int mapHeight);

/// free the a* data structures
extern void FreeAStar();

/// Find and a* path for a unit
extern int AStarFindPath(const Vec2i &startPos, const Vec2i &goalPos, int gw, int gh,
						 int tilesizex, int tilesizey, int minrange,
						 int maxrange, char *path, int pathlen, const CUnit &unit);

/*----------------------------------------------------------------------------
--  Variables
----------------------------------------------------------------------------*/

void TerrainTraversal::SetSize(unsigned int width, unsigned int height)
{
	m_values.resize((width + 2) * (height + 2));
	m_extented_width = width + 2;
	m_height = height;
}

void TerrainTraversal::Init()
{
	const unsigned int height = m_height;
	const unsigned int width = m_extented_width - 2;
	const unsigned int width_ext = m_extented_width;

	memset(&m_values[0], '\xFF', width_ext * sizeof(dataType));
	for (unsigned i = 1; i < 1 + height; ++i) {
		m_values[i * width_ext] = -1;
		memset(&m_values[i * width_ext + 1], '\0', width * sizeof(dataType));
		m_values[i * width_ext + width + 1] = -1;
	}
	memset(&m_values[(height + 1) * width_ext], '\xFF', width_ext * sizeof(dataType));
}

void TerrainTraversal::PushPos(const Vec2i &pos)
{
	if (IsVisited(pos) == false) {
		m_queue.push(PosNode(pos, pos));
		Set(pos, 1);
	}
}

void TerrainTraversal::PushNeighboor(const Vec2i &pos)
{
	const Vec2i offsets[] = {Vec2i(0, -1), Vec2i(-1, 0), Vec2i(1, 0), Vec2i(0, 1),
							 Vec2i(-1, -1), Vec2i(1, -1), Vec2i(-1, 1), Vec2i(1, 1)
							};

	for (int i = 0; i != 8; ++i) {
		const Vec2i newPos = pos + offsets[i];

		if (IsVisited(newPos) == false) {
			m_queue.push(PosNode(newPos, pos));
			Set(newPos, Get(pos) + 1);
		}
	}
}

void TerrainTraversal::PushUnitPosAndNeighboor(const CUnit &unit)
{
	const CUnit *startUnit = GetFirstContainer(unit);
	const Vec2i offset(1, 1);
	const Vec2i extraTileSize(startUnit->Type->TileWidth - 1, startUnit->Type->TileHeight - 1);
	const Vec2i start = startUnit->tilePos - offset;
	const Vec2i end = startUnit->tilePos + extraTileSize + offset;

	for (Vec2i it = start; it.y != end.y; ++it.y) {
		for (it.x = start.x; it.x != end.x; ++it.x) {
			PushPos(it);
		}
	}
}

bool TerrainTraversal::IsVisited(const Vec2i &pos) const
{
	return Get(pos) != 0;
}

bool TerrainTraversal::IsReached(const Vec2i &pos) const
{
	return Get(pos) != 0 && Get(pos) != -1;
}

bool TerrainTraversal::IsInvalid(const Vec2i &pos) const
{
	return Get(pos) != -1;
}

TerrainTraversal::dataType TerrainTraversal::Get(const Vec2i &pos) const
{
	return m_values[m_extented_width + 1 + pos.y * m_extented_width + pos.x];
}

void TerrainTraversal::Set(const Vec2i &pos, TerrainTraversal::dataType value)
{
	m_values[m_extented_width + 1 + pos.y * m_extented_width + pos.x] = value;
}

/*----------------------------------------------------------------------------
--  Functions
----------------------------------------------------------------------------*/

/**
**  Init the pathfinder
*/
void InitPathfinder()
{
	InitAStar(Map.Info.MapWidth, Map.Info.MapHeight);
}

/**
**  Free the pathfinder
*/
void FreePathfinder()
{
	FreeAStar();
}

/*----------------------------------------------------------------------------
--  PATH-FINDER USE
----------------------------------------------------------------------------*/

/**
**  Can the unit 'src' reach the place goalPos.
**
**  @param src       Unit for the path.
**  @param goalPos   Map tile position.
**  @param w         Width of Goal
**  @param h         Height of Goal
**  @param minrange  min range to the tile
**  @param range     Range to the tile.
**
**  @return          Distance to place.
*/
int PlaceReachable(const CUnit &src, const Vec2i &goalPos, int w, int h, int minrange, int range, bool from_outside_container)
{
	SetAStarFixedEnemyUnitsUnpassable(true); /// change Path Finder setting to don't count tiles with enemy units as passable
	int i;
	Vec2i srcTilePos = src.tilePos;
	int srcTW = src.Type->TileWidth;
	int srcTH = src.Type->TileHeight;
	if (!from_outside_container || !src.Container) {
		i = AStarFindPath(srcTilePos, goalPos, w, h,
						  srcTW, srcTH,
						  minrange, range, nullptr, 0, src);
	} else {
		const CUnit *first_container = GetFirstContainer(src);

        const Vec2i offset(1, 1);
		int containerW = first_container->Type->TileWidth;
		int containerH = first_container->Type->TileHeight;
		Vec2i containerTilePos = first_container->tilePos;
		// check top and bottom rows and left and right columns around the container
		for (int x = -1; x <= containerW; x++) {
			for (int y = -1; y <= containerH; y++) {
				if (x >= 0 && x < containerW && y >= 0 && y < containerH) {
					// inside the container, no need to check
					continue;
				}
				Vec2i tile_pos = containerTilePos + Vec2i(x, y);
				if (!Map.Info.IsPointOnMap(tile_pos)) {
					continue;
				}
				if (!CanMoveToMask(tile_pos, src.Type->MovementMask)) {
					//ignore tiles to which the unit cannot be dropped from its container
					continue;
				}

				i = AStarFindPath(tile_pos, goalPos, w, h,
					srcTW, srcTH,
					minrange, range, nullptr, 0, src);

				switch (i) {
					case PF_FAILED:
					case PF_UNREACHABLE:
					case PF_WAIT:
						continue;
				}
				goto finished;
			}
		}
	}
finished:
	SetAStarFixedEnemyUnitsUnpassable(false); /// restore Path Finder setting
	switch (i) {
		case PF_FAILED:
		case PF_UNREACHABLE:
		case PF_WAIT:
			return 0;
		case PF_REACHED:
			/* since most of this function usage check return value as bool
			* then reached state should be track as true value */
			return std::max(i, 1);
		default:
			return i;
	}
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
int UnitReachable(const CUnit &src, const CUnit &dst, int range, bool from_outside_container)
{
	//  Find a path to the goal.
	if (src.Type->Building) {
		return 0;
	}
	const int depth = PlaceReachable(src, dst.tilePos,
									 dst.Type->TileWidth, dst.Type->TileHeight, 0, range, from_outside_container);
	if (depth <= 0) {
		return 0;
	}
	return depth;
}

/**
**  Calc path length for the unit 'src' to reach the unit 'dst'.
**
**  @param src     	 Unit for the path.
**  @param dst    	 Unit to be reached.
**  @param minrange  min range to the tile
**  @param range     Range to the tile.
**
**  @return       path length to dst or error code
*/

int CalcPathLengthToUnit(const CUnit &src, const CUnit &dst, const int minrange, const int range)
{
	SetAStarFixedEnemyUnitsUnpassable(true); /// change Path Finder setting to don't count tiles with enemy units as passable
	int length = AStarFindPath(src.tilePos, dst.tilePos,
							   dst.Type->TileWidth, dst.Type->TileHeight,
							   src.Type->TileWidth, src.Type->TileHeight,
							   minrange, range,
							   NULL, 0, src);
	SetAStarFixedEnemyUnitsUnpassable(false); /// restore Path Finder setting
	switch (length) {
		case PF_FAILED:
		case PF_UNREACHABLE:
		case PF_WAIT:
			return -1;
			break;
		case PF_REACHED:
			return 0;
	}
	return length;
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

const Vec2i &PathFinderInput::GetUnitPos() const { return unit->tilePos; }
Vec2i PathFinderInput::GetUnitSize() const
{
	const Vec2i tileSize(unit->Type->TileWidth, unit->Type->TileHeight);
	return tileSize;
}

void PathFinderInput::SetUnit(CUnit &_unit)
{
	unit = &_unit;

	isRecalculatePathNeeded = true;
}


void PathFinderInput::SetGoal(const Vec2i &pos, const Vec2i &size)
{
	Assert(Map.Info.IsPointOnMap(pos));
	Assert(unit);
	Assert(unit->IsAliveOnMap());
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

void PathFinderInput::SetMinRange(int range)
{
	if (minRange != range) {
		minRange = range;
		isRecalculatePathNeeded = true;
	}
}

void PathFinderInput::SetMaxRange(int range)
{
	if (maxRange != range) {
		maxRange = range;
		isRecalculatePathNeeded = true;
	}
}

void PathFinderInput::PathRacalculated()
{
	unitSize.x = unit->Type->TileWidth;
	unitSize.y = unit->Type->TileHeight;

	isRecalculatePathNeeded = false;
}


PathFinderOutput::PathFinderOutput()
{
	memset(this, 0, sizeof(*this));
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
static int NewPath(PathFinderInput &input, PathFinderOutput &output)
{
	char *path = output.Path;
	int i = AStarFindPath(input.GetUnitPos(),
						  input.GetGoalPos(),
						  input.GetGoalSize().x, input.GetGoalSize().y,
						  input.GetUnitSize().x, input.GetUnitSize().y,
						  input.GetMinRange(), input.GetMaxRange(),
						  path, PathFinderOutput::MAX_PATH_LENGTH,
						  *input.GetUnit());
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
	PathFinderInput &input = unit.pathFinderData->input;
	PathFinderOutput &output = unit.pathFinderData->output;

	unit.CurrentOrder()->UpdatePathFinderData(input);
	// Attempt to use path cache
	// FIXME: If there is a goal, it may have moved, ruining the cache
	*pxd = 0;
	*pyd = 0;

	// Goal has moved, need to recalculate path or no cached path
	if (output.Length <= 0 || input.IsRecalculateNeeded()) {
		const int result = NewPath(input, output);

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
	const Vec2i dir(*pxd, *pyd);
	int result = output.Length;
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
