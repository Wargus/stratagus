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
/**@name pathfinder.h - The path finder headerfile. */
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

#ifndef __PATH_FINDER_H__
#define __PATH_FINDER_H__

//@{

#ifndef STDCALL
#ifdef _MSC_VER
#define STDCALL __stdcall
#else
#define STDCALL
#endif
#endif

#if defined(DEBUG_ASTAR)
#define AstarDebugPrint(x) DebugPrint(x)
#else
#define AstarDebugPrint(x)
#endif

/*----------------------------------------------------------------------------
--  Declarations
----------------------------------------------------------------------------*/

#include <queue>
#include "vec2i.h"

class CUnit;
class CFile;
struct lua_State;

/**
**  Result codes of the pathfinder.
**
**  @todo
**    Another idea is SINT_MAX as reached, SINT_MIN as unreachable
**    stop others how far to goal.
*/
enum _move_return_ {
	PF_FAILED = -3,       /// This Pathfinder failed, try another
	PF_UNREACHABLE = -2,  /// Unreachable stop
	PF_REACHED = -1,      /// Reached goal stop
	PF_WAIT = 0,          /// Wait, no time or blocked
	PF_MOVE = 1           /// On the way moving
};

class PathFinderInput
{
public:
	PathFinderInput();
	CUnit *GetUnit() const { return unit; }
	const Vec2i &GetUnitPos() const;
	Vec2i GetUnitSize() const;
	const Vec2i &GetGoalPos() const { return goalPos; }
	const Vec2i &GetGoalSize() const { return goalSize; }
	int GetMinRange() const { return minRange; }
	int GetMaxRange() const { return maxRange; }
	bool IsRecalculateNeeded() const { return isRecalculatePathNeeded; }

	void SetUnit(CUnit &_unit);
	void SetGoal(const Vec2i &pos, const Vec2i &size);
	void SetMinRange(int range);
	void SetMaxRange(int range);

	void PathRacalculated();

	void Save(CFile &file) const;
	void Load(lua_State *l);

private:
	CUnit *unit;
	Vec2i unitSize;
	Vec2i goalPos;
	Vec2i goalSize;
	int minRange;
	int maxRange;
	bool isRecalculatePathNeeded;
};

class PathFinderOutput
{
public:
	enum {MAX_PATH_LENGTH = 28}; /// max length of precalculated path
public:
	PathFinderOutput();
	void Save(CFile &file) const;
	void Load(lua_State *l);
public:
	unsigned short int Cycles;  /// how much Cycles we move.
	char Fast;                  /// Flag fast move (one step)
	char Length;                /// stored path length
	char Path[MAX_PATH_LENGTH]; /// directions of stored path
};

class PathFinderData
{
public:
	PathFinderInput input;
	PathFinderOutput output;
};


//
//  Terrain traversal stuff.
//

enum VisitResult {
	VisitResult_Finished,
	VisitResult_DeadEnd,
	VisitResult_Ok,
	VisitResult_Cancel
};

class TerrainTraversal
{
public:
	typedef short unsigned int dataType;
public:
	void SetSize(unsigned int width, unsigned int height);
	void Init();

	void PushPos(const Vec2i &pos);
	void PushNeighboor(const Vec2i &pos);
	void PushUnitPosAndNeighboor(const CUnit &unit);

	template <typename T>
	bool Run(T &context);

	bool IsVisited(const Vec2i &pos) const;
	bool IsReached(const Vec2i &pos) const;
	bool IsInvalid(const Vec2i &pos) const;

	// Accept pos to be at one inside the real map
	dataType Get(const Vec2i &pos) const;

private:
	void Set(const Vec2i &pos, dataType value);

	struct PosNode {
		PosNode(const Vec2i &pos, const Vec2i &from) : pos(pos), from(from) {}
		Vec2i pos;
		Vec2i from;
	};

private:
	std::vector<dataType> m_values;
	std::queue<PosNode> m_queue;
	unsigned int m_extented_width;
	unsigned int m_height;
};

template <typename T>
bool TerrainTraversal::Run(T &context)
{
	for (; m_queue.empty() == false; m_queue.pop()) {
		const PosNode &posNode = m_queue.front();

		switch (context.Visit(*this, posNode.pos, posNode.from)) {
			case VisitResult_Finished: return true;
			case VisitResult_DeadEnd: Set(posNode.pos, -1); break;
			case VisitResult_Ok: PushNeighboor(posNode.pos); break;
			case VisitResult_Cancel: return false;
		}
		Assert(IsVisited(posNode.pos));
	}
	return false;
}


/*----------------------------------------------------------------------------
--  Variables
----------------------------------------------------------------------------*/

/// cost associated to move on a tile occupied by a fixed unit
extern int AStarFixedUnitCrossingCost;
/// cost associated to move on a tile occupied by a moving unit
extern int AStarMovingUnitCrossingCost;
/// Whether to have knowledge of terrain that we haven't visited yet
extern bool AStarKnowUnseenTerrain;
/// Cost of using a square we haven't seen before.
extern int AStarUnknownTerrainCost;

//
//  Convert heading into direction.
//  N NE  E SE  S SW  W NW
extern const int Heading2X[9];
extern const int Heading2Y[9];
extern const int XY2Heading[3][3];

/*----------------------------------------------------------------------------
--  Functions
----------------------------------------------------------------------------*/

/// Init the pathfinder
extern void InitPathfinder();
/// Free the pathfinder
extern void FreePathfinder();

/// Returns the next element of the path
extern int NextPathElement(CUnit &unit, short int *xdp, short int *ydp);
/// Return distance to unit.
extern int UnitReachable(const CUnit &unit, const CUnit &dst, int range);
/// Can the unit 'src' reach the place x,y
extern int PlaceReachable(const CUnit &src, const Vec2i &pos, int w, int h,
						  int minrange, int maxrange);

//
// in astar.cpp
//

extern void SetAStarFixedUnitCrossingCost(int cost);
extern int GetAStarFixedUnitCrossingCost();

extern void SetAStarMovingUnitCrossingCost(int cost);
extern int GetAStarMovingUnitCrossingCost();

extern void SetAStarUnknownTerrainCost(int cost);
extern int GetAStarUnknownTerrainCost();

extern void PathfinderCclRegister();

//@}

#endif // !__PATH_FINDER_H__
