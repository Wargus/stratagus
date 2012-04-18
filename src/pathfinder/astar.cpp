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
/**@name astar.cpp - The a* path finder routines. */
//
//      (c) Copyright 1999-2008 by Lutz Sammer, Fabrice Rossi, Russell Smith,
//                                 Francois Beerten, Jimmy Salmon.
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

#include <stdlib.h>

#include "map.h"
#include "unit.h"

#include "pathfinder.h"

/*----------------------------------------------------------------------------
--  Declarations
----------------------------------------------------------------------------*/

struct Node {
	int CostFromStart;  /// Real costs to reach this point
	short int CostToGoal;     /// Estimated cost to goal
	char InGoal;        /// is this point in the goal
	char Direction;     /// Direction for trace back
};

struct Open {
	Vec2i pos;
	short int Costs; /// complete costs to goal
	unsigned short int O;     /// Offset into matrix
};

/// heuristic cost function for a*
static inline int AStarCosts(const Vec2i &pos, const Vec2i &goalPos)
{
	const Vec2i diff = pos - goalPos;
	return std::max<int>(MyAbs(diff.x), MyAbs(diff.y));
}

/*----------------------------------------------------------------------------
--  Variables
----------------------------------------------------------------------------*/

//  Convert heading into direction.
//                      //  N NE  E SE  S SW  W NW
const int Heading2X[9] = {  0, +1, +1, +1, 0, -1, -1, -1, 0 };
const int Heading2Y[9] = { -1, -1, 0, +1, +1, +1, 0, -1, 0 };
int Heading2O[9];//heading to offset
const int XY2Heading[3][3] = { {7, 6, 5}, {0, 0, 4}, {1, 2, 3}};

/// cost matrix
static Node *AStarMatrix;

/// a list of close nodes, helps to speed up the matrix cleaning
static int *CloseSet;
static int CloseSetSize;
static int Threshold;
static int OpenSetMaxSize;
static int AStarMatrixSize;
#define MAX_CLOSE_SET_RATIO 4
#define MAX_OPEN_SET_RATIO 8 // 10,16 to small

/// see pathfinder.h
int AStarFixedUnitCrossingCost;// = MaxMapWidth * MaxMapHeight;
int AStarMovingUnitCrossingCost = 5;
bool AStarKnowUnseenTerrain = false;
int AStarUnknownTerrainCost = 2;

static int AStarMapWidth;
static int AStarMapHeight;

static int AStarGoalX;
static int AStarGoalY;

/**
**  The Open set is handled by a stored array
**  the end of the array holds the item with the smallest cost.
*/

/// The set of Open nodes
static Open *OpenSet;
/// The size of the open node set
static int OpenSetSize;

static int *CostMoveToCache;
static const int CacheNotSet = -5;

/*----------------------------------------------------------------------------
--  Profile
----------------------------------------------------------------------------*/

#ifdef ASTAR_PROFILE

#include <map>
#ifndef __unix
#include <windows.h>
#else

union LARGE_INTEGER {
	uint64_t QuadPart;
	uint32_t DoublePart[2];
};
inline int QueryPerformanceCounter(LARGE_INTEGER *ptr)
{
	unsigned int lo, hi;
	__asm__ __volatile__(       // serialize
		"xorl %%eax,%%eax \n        cpuid"
		::: "%rax", "%rbx", "%rcx", "%rdx");
	/* We cannot use "=A", since this would use %rax on x86_64 */
	__asm__ __volatile__("rdtsc" : "=a"(lo), "=d"(hi));
	ptr->DoublePart[0] = lo;
	ptr->DoublePart[1] = hi;
	return 1;
};

inline int QueryPerformanceFrequency(LARGE_INTEGER *ptr)
{
	ptr->QuadPart = 1000;
	return 1;
}

#endif

#undef max
#undef min
static std::map<const char *const, LARGE_INTEGER> functionTimerMap;
struct ProfileData {
	unsigned long Calls;
	unsigned long TotalTime;
};
static std::map<const char *const, ProfileData> functionProfiles;

inline void ProfileInit()
{
	functionTimerMap.clear();
	functionProfiles.clear();
}

inline void ProfileBegin(const char *const function)
{
	LARGE_INTEGER counter;
	if (!QueryPerformanceCounter(&counter)) {
		return;
	}
	functionTimerMap[function] = counter;
}

inline void ProfileEnd(const char *const function)
{
	LARGE_INTEGER counter;
	if (!QueryPerformanceCounter(&counter)) {
		return;
	}
	unsigned long time = (unsigned long)(counter.QuadPart - functionTimerMap[function].QuadPart);
	ProfileData *data = &functionProfiles[function];
	data->Calls++;
	data->TotalTime += time;
}

static bool compProfileData(const ProfileData *lhs, const ProfileData *rhs)
{
	return (lhs->TotalTime > rhs->TotalTime);
}


inline void ProfilePrint()
{
	LARGE_INTEGER frequency;
	if (!QueryPerformanceFrequency(&frequency)) {
		return;
	}
	std::vector<ProfileData *> prof;
	for (std::map<const char *const, ProfileData>::iterator i = functionProfiles.begin();
		 i != functionProfiles.end(); ++i) {
		ProfileData *data = &i->second;
		prof.insert(std::upper_bound(prof.begin(), prof.end(), data, compProfileData), data);
	}

	FILE *fd = fopen("profile.txt", "wb");
	fprintf(fd, "    total\t    calls\t      per\tname\n");

	for (std::vector<ProfileData *>::iterator i = prof.begin(); i != prof.end(); ++i) {
		ProfileData *data = (*i);
		fprintf(fd, "%9.3f\t%9lu\t%9.3f\t",
				(double)data->TotalTime / frequency.QuadPart * 1000.0,
				data->Calls,
				(double)data->TotalTime / frequency.QuadPart * 1000.0 / data->Calls);
		for (std::map<const char *const, ProfileData>::iterator j =
				 functionProfiles.begin(); j != functionProfiles.end(); ++j) {
			ProfileData *data2 = &j->second;
			if (data == data2) {
				fprintf(fd, "%s\n", j->first);
			}
		}

	}

	fclose(fd);
}

#else
#define ProfileInit()
#define ProfileBegin(f)
#define ProfileEnd(f)
#define ProfilePrint()
#endif

/*----------------------------------------------------------------------------
--  Functions
----------------------------------------------------------------------------*/

/**
**  Init A* data structures
*/
void InitAStar(int mapWidth, int mapHeight)
{
	// Should only be called once
	Assert(!AStarMatrix);

	AStarMapWidth = mapWidth;
	AStarMapHeight = mapHeight;

	AStarMatrixSize = sizeof(Node) * AStarMapWidth * AStarMapHeight;
	AStarMatrix = new Node[AStarMapWidth * AStarMapHeight];
	memset(AStarMatrix, 0, AStarMatrixSize);

	Threshold = AStarMapWidth * AStarMapHeight / MAX_CLOSE_SET_RATIO;
	CloseSet = new int[Threshold];

	OpenSetMaxSize = AStarMapWidth * AStarMapHeight / MAX_OPEN_SET_RATIO;
	OpenSet = new Open[OpenSetMaxSize];

	CostMoveToCache = new int[AStarMapWidth * AStarMapHeight];

	for (int i = 0; i < 9; ++i) {
		Heading2O[i] = Heading2Y[i] * AStarMapWidth;
	}

	ProfileInit();
}

/**
**  Free A* data structure
*/
void FreeAStar()
{
	delete[] AStarMatrix;
	AStarMatrix = NULL;
	delete[] CloseSet;
	CloseSet = NULL;
	CloseSetSize = 0;
	delete[] OpenSet;
	OpenSet = NULL;
	OpenSetSize = 0;
	delete[] CostMoveToCache;
	CostMoveToCache = NULL;

	ProfilePrint();
}

/**
**  Prepare pathfinder.
*/
static void AStarPrepare()
{
	memset(AStarMatrix, 0, AStarMatrixSize);
}

/**
**  Clean up A*
*/
static void AStarCleanUp()
{
	ProfileBegin("AStarCleanUp");

	if (CloseSetSize >= Threshold) {
		AStarPrepare();
	} else {
		for (int i = 0; i < CloseSetSize; ++i) {
			AStarMatrix[CloseSet[i]].CostFromStart = 0;
			AStarMatrix[CloseSet[i]].InGoal = 0;
		}
	}
	ProfileEnd("AStarCleanUp");
}

static void CostMoveToCacheCleanUp()
{
	ProfileBegin("CostMoveToCacheCleanUp");
	int AStarMapMax =  AStarMapWidth * AStarMapHeight;
#if 1
	int *ptr = CostMoveToCache;
#ifdef __x86_64__
	union {
		intptr_t d;
		int i[2];
	} conv;
	conv.i[0] = CacheNotSet;
	conv.i[1] = CacheNotSet;

	if (((uintptr_t)ptr) & 4) {
		*ptr++ = CacheNotSet;
		--AStarMapMax;
	}
#endif
	while (AStarMapMax > 3) {
#ifdef __x86_64__
		*((intptr_t *)ptr) = conv.d;
		*((intptr_t *)(ptr + 2)) = conv.d;
		ptr += 4;
#else
		*ptr++ = CacheNotSet;
		*ptr++ = CacheNotSet;
		*ptr++ = CacheNotSet;
		*ptr++ = CacheNotSet;
#endif
		AStarMapMax -= 4;
	};
	while (AStarMapMax) {
		*ptr++ = CacheNotSet;
		--AStarMapMax;
	}
#else
	for (int i = 0; i < AStarMapMax; ++i) {
		CostMoveToCache[i] = CacheNotSet;
	}
#endif
	ProfileEnd("CostMoveToCacheCleanUp");
}

/**
**  Find the best node in the current open node set
**  Returns the position of this node in the open node set
*/
#define AStarFindMinimum() (OpenSetSize - 1)


/**
**  Remove the minimum from the open node set
*/
static void AStarRemoveMinimum(int pos)
{
	Assert(pos == OpenSetSize - 1);

	OpenSetSize--;
}

/**
**  Add a new node to the open set (and update the heap structure)
**
**  @return  0 or PF_FAILED
*/
static inline int AStarAddNode(const Vec2i &pos, int o, int costs)
{
	ProfileBegin("AStarAddNode");

	int bigi = 0, smalli = OpenSetSize;
	int midcost;
	int midi;
	int midCostToGoal;
	int midDist;
	const Open *open;

	if (OpenSetSize + 1 >= OpenSetMaxSize) {
		fprintf(stderr, "A* internal error: raise Open Set Max Size "
				"(current value %d)\n", OpenSetMaxSize);
		ProfileEnd("AStarAddNode");
		return PF_FAILED;
	}

	const int costToGoal = AStarMatrix[o].CostToGoal;
	const int dist = MyAbs(pos.x - AStarGoalX) + MyAbs(pos.y - AStarGoalY);

	// find where we should insert this node.
	// binary search where to insert the new node
	while (bigi < smalli) {
		midi = (smalli + bigi) >> 1;
		open = &OpenSet[midi];
		midcost = open->Costs;
		midCostToGoal = AStarMatrix[open->O].CostToGoal;
		midDist = MyAbs(open->pos.x - AStarGoalX) + MyAbs(open->pos.y - AStarGoalY);
		if (costs > midcost || (costs == midcost
								&& (costToGoal > midCostToGoal || (costToGoal == midCostToGoal
																   && dist > midDist)))) {
			smalli = midi;
		} else if (costs < midcost || (costs == midcost
									   && (costToGoal < midCostToGoal || (costToGoal == midCostToGoal
											   && dist < midDist)))) {
			if (bigi == midi) {
				bigi++;
			} else {
				bigi = midi;
			}
		} else {
			bigi = midi;
			smalli = midi;
		}
	}

	if (OpenSetSize > bigi) {
		// free a the slot for our node
		memmove(&OpenSet[bigi + 1], &OpenSet[bigi], (OpenSetSize - bigi) * sizeof(Open));
	}

	// fill our new node
	OpenSet[bigi].pos = pos;
	OpenSet[bigi].O = o;
	OpenSet[bigi].Costs = costs;
	++OpenSetSize;

	ProfileEnd("AStarAddNode");

	return 0;
}

/**
**  Change the cost associated to an open node.
**  Can be further optimised knowing that the new cost MUST BE LOWER
**  than the old one.
*/
static void AStarReplaceNode(int pos, int)
{
	ProfileBegin("AStarReplaceNode");

	Open node;

	// Remove the outdated node
	node = OpenSet[pos];
	OpenSetSize--;
	//memmove(&OpenSet[pos], &OpenSet[pos+1], sizeof(Open) * (OpenSetSize-pos));
	memcpy(&OpenSet[pos], &OpenSet[pos + 1], sizeof(Open) * (OpenSetSize - pos));

	// Re-add the node with the new cost
	AStarAddNode(node.pos, node.O, node.Costs);
	ProfileEnd("AStarReplaceNode");
}


/**
**  Check if a node is already in the open set.
**
**  @return  -1 if not found and the position of the node in the table if found.
*/
static int AStarFindNode(int eo)
{
	ProfileBegin("AStarFindNode");

	for (int i = 0; i < OpenSetSize; ++i) {
		if (OpenSet[i].O == eo) {
			ProfileEnd("AStarFindNode");
			return i;
		}
	}
	ProfileEnd("AStarFindNode");
	return -1;
}

/**
**  Add a node to the closed set
*/
static void AStarAddToClose(int node)
{
	if (CloseSetSize < Threshold) {
		CloseSet[CloseSetSize++] = node;
	}
}

#define GetIndex(x, y) (x) + (y) * AStarMapWidth

/* build-in costmoveto code */
static int CostMoveToCallBack_Default(unsigned int index, const CUnit &unit)
{
#ifdef DEBUG
	{
		Vec2i pos;
		pos.y = index / Map.Info.MapWidth;
		pos.x = index - pos.y * Map.Info.MapWidth;
		Assert(Map.Info.IsPointOnMap(pos));
	}
#endif
	int cost = 0;
	const int mask = unit.Type->MovementMask;
	const CUnitTypeFinder unit_finder((UnitTypeType)unit.Type->UnitType);
	const unsigned int player_index = unit.Player->Index;

	// verify each tile of the unit.
	int h = unit.Type->TileHeight;
	const int w = unit.Type->TileWidth;
	do {
		const CMapField *mf = Map.Field(index);
		int i = w;
		do {
			const int flag = mf->Flags & mask;
			if (flag && (AStarKnowUnseenTerrain || mf->IsExplored(player_index))) {
				if (flag & ~(MapFieldLandUnit | MapFieldAirUnit | MapFieldSeaUnit)) {
					// we can't cross fixed units and other unpassable things
					return -1;
				}
				CUnit *goal = unit_finder.Find(mf);
				if (!goal) {
					// Shouldn't happen, mask says there is something on this tile
					Assert(0);
					return -1;
				}
				if (goal->Moving)  {
					// moving unit are crossable
					cost += AStarMovingUnitCrossingCost;
				} else {
					// for non moving unit Always Fail unless goal is unit
					if (&unit != goal) {
						// FIXME: Need support for moving a fixed unit to add cost
						return -1;
						//cost += AStarFixedUnitCrossingCost;
					}
				}
			}
			// Add cost of crossing unknown tiles if required
			if (!AStarKnowUnseenTerrain && !mf->IsExplored(player_index)) {
				// Tend against unknown tiles.
				cost += AStarUnknownTerrainCost;
			}
			// Add tile movement cost
			cost += mf->Cost;
			++mf;
		} while (--i);
		index += AStarMapWidth;
	} while (--h);
	return cost;
}


/**
**  Compute the cost of crossing tile (x,y)
**
**  @param x     X tile where to move.
**  @param y     Y tile where to move.
**  @param data  user data.
**
**  @return      -1 -> impossible to cross.
**                0 -> no induced cost, except move
**               >0 -> costly tile
*/
static inline int CostMoveTo(unsigned int index, const CUnit &unit)
{
	int *c = &CostMoveToCache[index];
	if (*c != CacheNotSet) {
		return *c;
	}
	*c = CostMoveToCallBack_Default(index, unit);
	return *c;
}

inline int square(int v)
{
	return v * v;
}

/**
**  MarkAStarGoal
*/
static int AStarMarkGoal(const Vec2i &goal, int gw, int gh,
						 int tilesizex, int tilesizey, int minrange, int maxrange, const CUnit &unit)
{
	ProfileBegin("AStarMarkGoal");

	if (minrange == 0 && maxrange == 0 && gw == 0 && gh == 0) {
		if (goal.x + tilesizex > AStarMapWidth || goal.y + tilesizey > AStarMapHeight) {
			ProfileEnd("AStarMarkGoal");
			return 0;
		}
		unsigned int offset = GetIndex(goal.x, goal.y);
		if (CostMoveTo(offset, unit) >= 0) {
			AStarMatrix[offset].InGoal = 1;
			ProfileEnd("AStarMarkGoal");
			return 1;
		} else {
			ProfileEnd("AStarMarkGoal");
			return 0;
		}
	}

	gw = std::max(gw, 1);
	gh = std::max(gh, 1);

	bool goal_reachable = false;

	const Vec2i extratilesize = {tilesizex - 1, tilesizey - 1};

	// top hemi cycle
	{
		const int miny = std::max(-maxrange - extratilesize.y, 0 - goal.y);
		for (int offsety = miny; offsety < -minrange - extratilesize.y; ++offsety) {
			const int offsetx = isqrt(square(maxrange + 1) - square(-offsety) - 1);
			const int minx = std::max(0, goal.x - offsetx - extratilesize.x);
			const int maxx = std::min(Map.Info.MapWidth - extratilesize.x, goal.x + gw + offsetx);
			Vec2i mpos = {minx, goal.y + offsety};
			const unsigned int offset = mpos.y * Map.Info.MapWidth;

			for (mpos.x = minx; mpos.x < maxx; ++mpos.x) {
				if (CostMoveTo(offset + mpos.x, unit) >= 0) {
					AStarMatrix[offset + mpos.x].InGoal = 1;
					goal_reachable = true;
				}
				AStarAddToClose(offset + mpos.x);
			}
		}
	}
	if (minrange == 0) {
		// center
		for (int offsety = -extratilesize.y; offsety < gh; ++offsety) {
			const int minx = std::max(0, goal.x - maxrange - extratilesize.x);
			const int maxx = std::min(Map.Info.MapWidth - extratilesize.x, goal.x + gw + maxrange);
			Vec2i mpos = {minx, goal.y + offsety};
			const unsigned int offset = mpos.y * Map.Info.MapWidth;

			for (mpos.x = minx; mpos.x < maxx; ++mpos.x) {
				if (CostMoveTo(offset + mpos.x, unit) >= 0) {
					AStarMatrix[offset + mpos.x].InGoal = 1;
					goal_reachable = true;
				}
				AStarAddToClose(offset + mpos.x);
			}
		}
	} else {
		{
			// top hemi cycle excluding minRange
			const int miny = std::max(-minrange - extratilesize.y, 0 - goal.y);
			const int maxy = std::min(0, Map.Info.MapHeight - goal.y - extratilesize.y);
			for (int offsety = miny; offsety < maxy; ++offsety) {
				const int offsetx1 = isqrt(square(maxrange + 1) - square(-offsety) - 1);
				const int offsetx2 = isqrt(square(minrange + 1) - square(-offsety) - 1);
				const int minxs[2] = {std::max(0, goal.x - offsetx1 - extratilesize.x), std::min(Map.Info.MapWidth - extratilesize.x, goal.x + gw + offsetx2)};
				const int maxxs[2] = {std::max(0, goal.x - offsetx2 - extratilesize.x), std::min(Map.Info.MapWidth - extratilesize.x, goal.x + gw + offsetx1)};

				for (int i = 0; i < 2; ++i) {
					const int minx = minxs[i];
					const int maxx = maxxs[i];
					Vec2i mpos = {minx, goal.y + offsety};
					const unsigned int offset = mpos.y * Map.Info.MapWidth;

					for (mpos.x = minx; mpos.x < maxx; ++mpos.x) {
						if (CostMoveTo(offset + mpos.x, unit) >= 0) {
							AStarMatrix[offset + mpos.x].InGoal = 1;
							goal_reachable = true;
						}
						AStarAddToClose(offset + mpos.x);
					}
				}
			}
		}
		{
			// center
			const int mincenters[] = {std::max(0, goal.x - maxrange - extratilesize.x),
									  std::min(Map.Info.MapWidth - extratilesize.x, goal.x + gw + minrange)
									 };
			const int maxcenters[] = {std::max(0, goal.x - minrange - extratilesize.x),
									  std::min(Map.Info.MapWidth - extratilesize.x, goal.x + gw + maxrange)
									 };

			const int miny = std::max(0 - goal.y, -extratilesize.y);
			const int maxy = std::min(Map.Info.MapHeight - goal.y - extratilesize.y, gh);

			for (int i = 0; i < 2; ++i) {
				for (int offsety = miny; offsety < maxy; ++offsety) {
					const int minx = mincenters[i];
					const int maxx = maxcenters[i];
					Vec2i mpos = {minx, goal.y + offsety};
					const unsigned int offset = mpos.y * Map.Info.MapWidth;

					for (mpos.x = minx; mpos.x < maxx; ++mpos.x) {
						if (CostMoveTo(offset + mpos.x, unit) >= 0) {
							AStarMatrix[offset + mpos.x].InGoal = 1;
							goal_reachable = true;
						}
						AStarAddToClose(offset + mpos.x);
					}
				}
			}
		}
		{
			// bottom hemi cycle
			const int maxy = std::min(minrange, Map.Info.MapHeight - goal.y - gh);
			for (int offsety = 0; offsety < maxy; ++offsety) {
				const int offsetx1 = isqrt(square(maxrange + 1) - square(offsety) - 1);
				const int offsetx2 = isqrt(square(minrange + 1) - square(offsety) - 1);
				const int minxs[2] = {std::max(0, goal.x - offsetx1) - extratilesize.x, std::min(Map.Info.MapWidth - extratilesize.x, goal.x + gw + offsetx2)};
				const int maxxs[2] = {std::max(0, goal.x - offsetx2) - extratilesize.x, std::min(Map.Info.MapWidth - extratilesize.x, goal.x + gw + offsetx1)};

				for (int i = 0; i < 2; ++i) {
					const int minx = minxs[i];
					const int maxx = maxxs[i];
					Vec2i mpos = {minx, goal.y + offsety};
					const unsigned int offset = mpos.y * Map.Info.MapWidth;

					for (mpos.x = minx; mpos.x < maxx; ++mpos.x) {
						if (CostMoveTo(offset + mpos.x, unit) >= 0) {
							AStarMatrix[offset + mpos.x].InGoal = 1;
							goal_reachable = true;
						}
						AStarAddToClose(offset + mpos.x);
					}
				}
			}
		}
	}
	{
		// bottom hemi-cycle
		const int maxy = std::min(maxrange, Map.Info.MapHeight - goal.y - gh - extratilesize.y);
		for (int offsety = minrange; offsety < maxy; ++offsety) {
			const int offsetx = isqrt(square(maxrange + 1) - square(offsety + 1) - 1);
			const int minx = std::max(0, goal.x - offsetx - extratilesize.x);
			const int maxx = std::min(Map.Info.MapWidth - extratilesize.x, goal.x + gw + offsetx);
			Vec2i mpos = {minx, goal.y + gh + offsety};
			const unsigned int offset = mpos.y * Map.Info.MapWidth;

			for (mpos.x = minx; mpos.x < maxx; ++mpos.x) {
				if (CostMoveTo(offset + mpos.x, unit) >= 0) {
					AStarMatrix[offset + mpos.x].InGoal = 1;
					goal_reachable = true;
				}
				AStarAddToClose(offset + mpos.x);
			}
		}
	}
	ProfileEnd("AStarMarkGoal");
	return goal_reachable;
}

/**
**  Save the path
**
**  @return  The length of the path
*/
static int AStarSavePath(const Vec2i &startPos, const Vec2i &endPos, char *path, int pathLen)
{
	ProfileBegin("AStarSavePath");

	int fullPathLength;
	int pathPos;
	int direction;

	// Figure out the full path length
	fullPathLength = 0;
	Vec2i curr = endPos;
	int currO = curr.y * AStarMapWidth;
	while (curr != startPos) {
		direction = AStarMatrix[currO + curr.x].Direction;
		curr.x -= Heading2X[direction];
		curr.y -= Heading2Y[direction];
		currO -= Heading2O[direction];
		fullPathLength++;
	}

	// Save as much of the path as we can
	if (path) {
		pathLen = std::min<int>(fullPathLength, pathLen);
		pathPos = fullPathLength;
		curr = endPos;
		currO = curr.y * AStarMapWidth;
		while (curr != startPos) {
			direction = AStarMatrix[currO + curr.x].Direction;
			curr.x -= Heading2X[direction];
			curr.y -= Heading2Y[direction];
			currO -= Heading2O[direction];
			--pathPos;
			if (pathPos < pathLen) {
				path[pathLen - pathPos - 1] = direction;
			}
		}
	}

	ProfileEnd("AStarSavePath");
	return fullPathLength;
}

/**
**  Optimization to find a simple path
**  Check if we're at the goal or if it's 1 tile away
*/
static int AStarFindSimplePath(const Vec2i &startPos, const Vec2i &goal, int gw, int gh,
							   int, int, int minrange, int maxrange,
							   char *path, int, const CUnit &unit)
{
	ProfileBegin("AStarFindSimplePath");
	// At exact destination point already
	if (goal == startPos && minrange == 0) {
		ProfileEnd("AStarFindSimplePath");
		return PF_REACHED;
	}

	// Don't allow unit inside destination area
	if (goal.x <= startPos.x && startPos.x <= goal.x + gw - 1
		&& goal.y <= startPos.y && startPos.y <= goal.y + gh - 1) {
		return PF_FAILED;
	}

	const Vec2i diff = goal - startPos;
	const int distance = isqrt(square(diff.x) + square(diff.y));

	// Within range of destination
	if (minrange <= distance && distance <= maxrange) {
		ProfileEnd("AStarFindSimplePath");
		return PF_REACHED;
	}

	if (MyAbs(diff.x) <= 1 && MyAbs(diff.y) <= 1) {
		// Move to adjacent cell
		if (CostMoveTo(GetIndex(goal.x, goal.y), unit) == -1) {
			ProfileEnd("AStarFindSimplePath");
			return PF_UNREACHABLE;
		}

		if (path) {
			path[0] = XY2Heading[diff.x + 1][diff.y + 1];
		}
		ProfileEnd("AStarFindSimplePath");
		return 1;
	}

	ProfileEnd("AStarFindSimplePath");
	return PF_FAILED;
}

/**
**  Find path.
*/
int AStarFindPath(const Vec2i &startPos, const Vec2i &goalPos, int gw, int gh,
				  int tilesizex, int tilesizey, int minrange, int maxrange,
				  char *path, int pathlen, const CUnit &unit)
{
	Assert(Map.Info.IsPointOnMap(startPos));

	ProfileBegin("AStarFindPath");

	AStarGoalX = goalPos.x;
	AStarGoalY = goalPos.y;

	//  Check for simple cases first
	int ret = AStarFindSimplePath(startPos, goalPos, gw, gh, tilesizex, tilesizey,
								  minrange, maxrange, path, pathlen, unit);
	if (ret != PF_FAILED) {
		ProfileEnd("AStarFindPath");
		return ret;
	}

	//  Initialize
	AStarCleanUp();
	CostMoveToCacheCleanUp();

	OpenSetSize = 0;
	CloseSetSize = 0;

	if (!AStarMarkGoal(goalPos, gw, gh, tilesizex, tilesizey, minrange, maxrange, unit)) {
		// goal is not reachable
		ret = PF_UNREACHABLE;
		ProfileEnd("AStarFindPath");
		return ret;
	}

	int eo = startPos.y * AStarMapWidth + startPos.x;
	// it is quite important to start from 1 rather than 0, because we use
	// 0 as a way to represent nodes that we have not visited yet.
	AStarMatrix[eo].CostFromStart = 1;
	// 8 to say we are came from nowhere.
	AStarMatrix[eo].Direction = 8;

	// place start point in open, it that failed, try another pathfinder
	int costToGoal = AStarCosts(startPos, goalPos);
	AStarMatrix[eo].CostToGoal = costToGoal;
	if (AStarAddNode(startPos, eo, 1 + costToGoal) == PF_FAILED) {
		ret = PF_FAILED;
		ProfileEnd("AStarFindPath");
		return ret;
	}
	AStarAddToClose(OpenSet[0].O);
	if (AStarMatrix[eo].InGoal) {
		ret = PF_REACHED;
		ProfileEnd("AStarFindPath");
		return ret;
	}
	Vec2i endPos;

	//  Begin search
	while (1) {
		// Find the best node of from the open set
		const int shortest = AStarFindMinimum();
		const int x = OpenSet[shortest].pos.x;
		const int y = OpenSet[shortest].pos.y;
		const int o = OpenSet[shortest].O;

		AStarRemoveMinimum(shortest);

		// If we have reached the goal, then exit.
		if (AStarMatrix[o].InGoal == 1) {
			endPos.x = x;
			endPos.y = y;
			break;
		}

#if 0
		// If we have looked too long, then exit.
		if (!counter--) {
			// FIXME: Select a "good" point from the open set.
			// Nearest point to goal.
			AstarDebugPrint("way too long\n");
			ret = PF_FAILED;
			ProfileEnd("AStarFindPath");
			return ret;
		}
#endif

		// Generate successors of this node.

		// Node that this node was generated from.
		const int px = x - Heading2X[(int)AStarMatrix[o].Direction];
		const int py = y - Heading2Y[(int)AStarMatrix[o].Direction];

		for (int i = 0; i < 8; ++i) {
			endPos.x = x + Heading2X[i];
			endPos.y = y + Heading2Y[i];

			// Don't check the tile we came from, it's not going to be better
			// Should reduce load on A*

			if (endPos.x == px && endPos.y == py) {
				continue;
			}

			// Outside the map or can't be entered.
			if (endPos.x < 0 || endPos.x + tilesizex - 1 >= AStarMapWidth
				|| endPos.y < 0 || endPos.y + tilesizey - 1 >= AStarMapHeight) {
				continue;
			}

			//eo = GetIndex(ex, ey);
			eo = endPos.x + (o - x) + Heading2O[i];

			// if the point is "move to"-able and
			// if we have not reached this point before,
			// or if we have a better path to it, we add it to open set
			int new_cost = CostMoveTo(eo, unit);
			if (new_cost == -1) {
				// uncrossable tile
				continue;
			}

			// Add a cost for walking to make paths more realistic for the user.
			new_cost++;
			new_cost += AStarMatrix[o].CostFromStart;
			if (AStarMatrix[eo].CostFromStart == 0) {
				// we are sure the current node has not been already visited
				AStarMatrix[eo].CostFromStart = new_cost;
				AStarMatrix[eo].Direction = i;
				costToGoal = AStarCosts(endPos, goalPos);
				AStarMatrix[eo].CostToGoal = costToGoal;
				if (AStarAddNode(endPos, eo, AStarMatrix[eo].CostFromStart + costToGoal) == PF_FAILED) {
					ret = PF_FAILED;
					ProfileEnd("AStarFindPath");
					return ret;
				}
				// we add the point to the close set
				AStarAddToClose(eo);
			} else if (new_cost < AStarMatrix[eo].CostFromStart) {
				// Already visited node, but we have here a better path
				// I know, it's redundant (but simpler like this)
				AStarMatrix[eo].CostFromStart = new_cost;
				AStarMatrix[eo].Direction = i;
				// this point might be already in the OpenSet
				const int j = AStarFindNode(eo);
				if (j == -1) {
					costToGoal = AStarCosts(endPos, goalPos);
					AStarMatrix[eo].CostToGoal = costToGoal;
					if (AStarAddNode(endPos, eo, AStarMatrix[eo].CostFromStart + costToGoal) == PF_FAILED) {
						ret = PF_FAILED;
						ProfileEnd("AStarFindPath");
						return ret;
					}
				} else {
					costToGoal = AStarCosts(endPos, goalPos);
					AStarMatrix[eo].CostToGoal = costToGoal;
					AStarReplaceNode(j, AStarMatrix[eo].CostFromStart + costToGoal);
				}
				// we don't have to add this point to the close set
			}
		}
		if (OpenSetSize <= 0) { // no new nodes generated
			ret = PF_UNREACHABLE;
			ProfileEnd("AStarFindPath");
			return ret;
		}
	}

	const int path_length = AStarSavePath(startPos, endPos, path, pathlen);

	ret = path_length;

	ProfileEnd("AStarFindPath");
	return ret;
}

struct StatsNode {
	StatsNode() : Direction(0), InGoal(0), CostFromStart(0), Costs(0) {}

	int Direction;
	int InGoal;
	int CostFromStart;
	int Costs;
	int CostToGoal;
};

StatsNode *AStarGetStats()
{
	StatsNode *stats = new StatsNode[AStarMapWidth * AStarMapHeight];
	StatsNode *s = stats;
	Node *m = AStarMatrix;

	for (int j = 0; j < AStarMapHeight; ++j) {
		for (int i = 0; i < AStarMapWidth; ++i) {
			s->Direction = m->Direction;
			s->InGoal = m->InGoal;
			s->CostFromStart = m->CostFromStart;
			s->CostToGoal = m->CostToGoal;
			++s;
			++m;
		}
	}

	for (int i = 0; i < OpenSetSize; ++i) {
		stats[OpenSet[i].O].Costs = OpenSet[i].Costs;
	}
	return stats;
}

void AStarFreeStats(StatsNode *stats)
{
	delete[] stats;
}

/*----------------------------------------------------------------------------
--  Configurable costs
----------------------------------------------------------------------------*/

// AStarFixedUnitCrossingCost
void SetAStarFixedUnitCrossingCost(int cost)
{
	if (cost <= 3) {
		fprintf(stderr, "AStarFixedUnitCrossingCost must be greater than 3\n");
	}
}
int GetAStarFixedUnitCrossingCost()
{
	return AStarFixedUnitCrossingCost;
}

// AStarMovingUnitCrossingCost
void SetAStarMovingUnitCrossingCost(int cost)
{
	if (cost <= 3) {
		fprintf(stderr, "AStarMovingUnitCrossingCost must be greater than 3\n");
	}
}
int GetAStarMovingUnitCrossingCost()
{
	return AStarMovingUnitCrossingCost;
}

// AStarUnknownTerrainCost
void SetAStarUnknownTerrainCost(int cost)
{
	if (cost < 0) {
		fprintf(stderr, "AStarUnknownTerrainCost must be non-negative\n");
		return;
	}
	AStarUnknownTerrainCost = cost;
}
int GetAStarUnknownTerrainCost()
{
	return AStarUnknownTerrainCost;
}

//@}
