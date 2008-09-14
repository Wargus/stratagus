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
	char Direction;     /// Direction for trace back
	char InGoal;        /// is this point in the goal
	int CostFromStart;  /// Real costs to reach this point
	int CostToGoal;     /// Estimated cost to goal
};

struct Open {
	short int X;     /// X coordinate
	short int Y;     /// Y coordinate
	short int Costs; /// complete costs to goal
	unsigned short int O;     /// Offset into matrix
};

/// heuristic cost function for a*
#define AStarCosts(sx,sy,ex,ey) std::max(MyAbs(sx - ex), MyAbs(sy - ey))

/*----------------------------------------------------------------------------
--  Variables
----------------------------------------------------------------------------*/

//  Convert heading into direction.
//                      //  N NE  E SE  S SW  W NW
const int Heading2X[9] = {  0,+1,+1,+1, 0,-1,-1,-1, 0 };
const int Heading2Y[9] = { -1,-1, 0,+1,+1,+1, 0,-1, 0 };
int Heading2O[9];//heading to offset
const int XY2Heading[3][3] = { {7,6,5},{0,0,4},{1,2,3}};

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

static unsigned char *ASVisionTable[3];
static int *ASVisionLookup;

static int (STDCALL *CostMoveToCallback)(unsigned int index, void *data);
static int *CostMoveToCache;
static const int CacheNotSet = -5;

/*----------------------------------------------------------------------------
--  Profile
----------------------------------------------------------------------------*/

#ifdef ASTAR_PROFILE

#include <map>
#include <windows.h>
#undef max
#undef min
static std::map<std::string, LARGE_INTEGER> functionTimerMap;
struct ProfileData {
	unsigned long Calls;
	unsigned long TotalTime;
};
static std::map<std::string, ProfileData> functionProfiles;

inline void ProfileInit()
{
	functionTimerMap.clear();
	functionProfiles.clear();
}

inline void ProfileBegin(const std::string &function)
{
	LARGE_INTEGER counter;
	if (!QueryPerformanceCounter(&counter)) {
		return;
	}
	functionTimerMap[function] = counter;
}

inline void ProfileEnd(const std::string &function)
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

inline void ProfilePrint()
{
	LARGE_INTEGER frequency;
	if (!QueryPerformanceFrequency(&frequency)) {
		return;
	}

	std::map<std::string, ProfileData>::iterator i;

	FILE *fd = fopen("profile.txt", "wb");
	fprintf(fd, "    total\t    calls\t      per\tname\n");

	for (i = functionProfiles.begin(); i != functionProfiles.end(); ++i) {
		ProfileData *data = &i->second;
		fprintf(fd, "%9.3f\t%9lu\t%9.3f\t%s\n",
			(double)data->TotalTime / frequency.QuadPart * 1000.0,
			data->Calls,
			(double)data->TotalTime / frequency.QuadPart * 1000.0 / data->Calls,
			i->first.c_str());
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

// FIXME: this is duplicated from map_fog.cpp
static void ASInitVisionTable(void)
{
	ProfileBegin("InitVisionTable");

	int *visionlist;
	int maxsize;
	int sizex;
	int sizey;
	int maxsearchsize;
	int i;
	int VisionTablePosition;
	int marker;
	int direction;
	int right;
	int up;
	int repeat;

	// Initialize Visiontable to large size, can't be more entries than tiles.
	ASVisionTable[0] = new unsigned char[AStarMapWidth * AStarMapWidth];
	ASVisionTable[1] = new unsigned char[AStarMapWidth * AStarMapWidth];
	ASVisionTable[2] = new unsigned char[AStarMapWidth * AStarMapWidth];

	ASVisionLookup = new int[AStarMapWidth + 2];

	visionlist = new int[AStarMapWidth * AStarMapWidth];
	//*2 as diagonal distance is longer

	maxsize = AStarMapWidth;
	maxsearchsize = AStarMapWidth;
	// Fill in table of map size
	for (sizex = 0; sizex < maxsize; ++sizex) {
		for (sizey = 0; sizey < maxsize; ++sizey) {
			visionlist[sizey * maxsize + sizex] = isqrt(sizex * sizex + sizey * sizey);
		}
	}

	ASVisionLookup[0] = 0;
	i = 1;
	VisionTablePosition = 0;
	while (i < maxsearchsize) {
		// Set Lookup Table
		ASVisionLookup[i] = VisionTablePosition;
		// Put in Null Marker
		ASVisionTable[0][VisionTablePosition] = i;
		ASVisionTable[1][VisionTablePosition] = 0;
		ASVisionTable[2][VisionTablePosition] = 0;
		++VisionTablePosition;


		// find i in left column
		marker = maxsize * i;
		direction = 0;
		right = 0;
		up = 0;

		// If not on top row, continue
		do {
			repeat = 0;
			do {
				// search for repeating
				// Test Right
				if ((repeat == 0 || direction == 1) && visionlist[marker + 1] == i) {
					right = 1;
					up = 0;
					++repeat;
					direction = 1;
					++marker;
				} else if ((repeat == 0 || direction == 2) && visionlist[marker - maxsize] == i) {
					up = 1;
					right = 0;
					++repeat;
					direction = 2;
					marker = marker - maxsize;
				} else if ((repeat == 0 || direction == 3) && visionlist[marker + 1 - maxsize] == i &&
						visionlist[marker - maxsize] != i && visionlist[marker + 1] != i) {
					up = 1;
					right = 1;
					++repeat;
					direction = 3;
					marker = marker + 1 - maxsize;
				} else {
					direction = 0;
					break;
				}

			   // search right
			   // search up - store as down.
			   // search diagonal
			}  while (direction && marker > (maxsize * 2));
			if (right || up) {
				ASVisionTable[0][VisionTablePosition] = repeat;
				ASVisionTable[1][VisionTablePosition] = right;
				ASVisionTable[2][VisionTablePosition] = up;
				++VisionTablePosition;
			}
		} while (marker > (maxsize * 2));
		++i;
	}

	delete[] visionlist;

	ProfileEnd("InitVisionTable");
}

/**
**  Init A* data structures
*/
void InitAStar(int mapWidth, int mapHeight,
		 int (STDCALL *costMoveTo)(unsigned int index, void *data))
{
	// Should only be called once
	Assert(!AStarMatrix);

	AStarMapWidth = mapWidth;
	AStarMapHeight = mapHeight;
	CostMoveToCallback = costMoveTo;

	AStarMatrixSize = sizeof(Node) * AStarMapWidth * AStarMapHeight;
	AStarMatrix = new Node[AStarMapWidth * AStarMapHeight];
	memset(AStarMatrix, 0, AStarMatrixSize);

	Threshold = AStarMapWidth * AStarMapHeight / MAX_CLOSE_SET_RATIO;
	CloseSet = new int[Threshold];

	OpenSetMaxSize = AStarMapWidth * AStarMapHeight / MAX_OPEN_SET_RATIO;
	OpenSet = new Open[OpenSetMaxSize];

	CostMoveToCache = new int[AStarMapWidth * AStarMapHeight];

	for(int i = 0; i < 9; ++i)
		Heading2O[i] = Heading2Y[i] * AStarMapWidth;

	ProfileInit();
	ASInitVisionTable();
}

/**
**  Free A* data structure
*/
void FreeAStar(void)
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

	delete[] ASVisionLookup;
	ASVisionLookup = NULL;
	delete[] ASVisionTable[0];
	ASVisionTable[0] = NULL;
	delete[] ASVisionTable[1];
	ASVisionTable[1] = NULL;
	delete[] ASVisionTable[2];
	ASVisionTable[2] = NULL;

	ProfilePrint();
}

/**
**  Prepare pathfinder.
*/
static void AStarPrepare(void)
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

static void CostMoveToCacheCleanUp(void)
{
	ProfileBegin("CostMoveToCacheCleanUp");
	int AStarMapMax =  AStarMapWidth * AStarMapHeight;
#if 1
	int *ptr = CostMoveToCache;
#ifdef __x86_64__
	union {
		long long int d;
		int i[2];
	} conv;
	conv.i[0] = CacheNotSet;
	conv.i[1] = CacheNotSet;

	if(((uintptr_t)ptr) & 4) {
		*ptr++ = CacheNotSet;
		--AStarMapMax;
	}
#endif
	while (AStarMapMax > 3) {
#ifdef __x86_64__
		*((long long int*)ptr) = conv.d;
		*((long long int*)(ptr + 2)) = conv.d;
		ptr += 4;
#else
		*ptr++ = CacheNotSet;
		*ptr++ = CacheNotSet;
		*ptr++ = CacheNotSet;
		*ptr++ = CacheNotSet;
#endif
		AStarMapMax-=4;
	};
	while(AStarMapMax) {
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
static int AStarAddNode(int x, int y, int o, int costs)
{
	ProfileBegin("AStarAddNode");

	int bigi, smalli;
	int midcost;
	int midi;
	int costToGoal;
	int midCostToGoal;
	int dist;
	int midDist;
	
	if (OpenSetSize + 1 >= OpenSetMaxSize) {
		fprintf(stderr, "A* internal error: raise Open Set Max Size "
				"(current value %d)\n", OpenSetMaxSize);
		ProfileEnd("AStarAddNode");
		return PF_FAILED;
	}

	costToGoal = AStarMatrix[o].CostToGoal;
	dist = MyAbs(x - AStarGoalX) + MyAbs(y - AStarGoalY);

	// find where we should insert this node.
	bigi = 0;
	smalli = OpenSetSize;

	// binary search where to insert the new node
	while (bigi < smalli) {
		midi = (smalli + bigi) >> 1;
		midcost = OpenSet[midi].Costs;
		midCostToGoal = AStarMatrix[OpenSet[midi].O].CostToGoal;
		midDist = MyAbs(OpenSet[midi].X - AStarGoalX) +
					 MyAbs(OpenSet[midi].Y - AStarGoalY);
		if (costs > midcost || (costs == midcost &&
				(costToGoal > midCostToGoal || (costToGoal == midCostToGoal &&
					dist > midDist)))) {
			smalli = midi;
		} else if (costs < midcost || (costs == midcost &&
				(costToGoal < midCostToGoal || (costToGoal == midCostToGoal &&
					dist < midDist)))) {
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
		memmove(&OpenSet[bigi+1], &OpenSet[bigi], (OpenSetSize - bigi) * sizeof(Open));
	}

	// fill our new node
	OpenSet[bigi].X = x;
	OpenSet[bigi].Y = y;
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
static void AStarReplaceNode(int pos, int costs)
{
	ProfileBegin("AStarReplaceNode");

	Open node;

	// Remove the outdated node
	node = OpenSet[pos];
	OpenSetSize--;
	memmove(&OpenSet[pos], &OpenSet[pos+1], sizeof(Open) * (OpenSetSize-pos));

	// Re-add the node with the new cost
	AStarAddNode(node.X, node.Y, node.O, node.Costs);
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

#define GetIndex(x,y) \
	(x) + (y) * AStarMapWidth	

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
static inline int CostMoveTo(unsigned int index, void *data)
{
	int *c = &CostMoveToCache[index];
	if (*c != CacheNotSet) {
		return *c;
	}
	if(!CostMoveToCallback) {
		/* build-in costmoveto code */
		const CUnit *const unit = (const CUnit *)data;
/*		
		
		// Doesn't cost anything to move to ourselves :)
		// Used when marking goals mainly.  Could cause speed problems
		if (unit->X == x && unit->Y == y) {
			*c = 0;
			return 0;
		}
*/
		CUnit *goal;
		const CMapField *mf;
		int i, flag, cost = 0;
		const int mask = unit->Type->MovementMask;
		const CUnitTypeFinder unit_finder((UnitTypeType)unit->Type->UnitType);
		const unsigned int player_index = unit->Player->Index;
		
		// verify each tile of the unit.
		int h = unit->Type->TileHeight;
		const int w = unit->Type->TileWidth;
		do {
			mf = Map.Field(index);
			i = w;
			do {
				flag = mf->Flags & mask;
				if (flag && (AStarKnowUnseenTerrain || mf->IsExplored(player_index))) {
					if (flag & ~(MapFieldLandUnit | MapFieldAirUnit | MapFieldSeaUnit)) {
						// we can't cross fixed units and other unpassable things
						*c = -1;
						return -1;
					}
					goal = unit_finder.Find(mf);
					if (!goal) {
						// Shouldn't happen, mask says there is something on this tile
						Assert(0);
						*c = -1;
						return -1;
					}
					if (goal->Moving)  {
						// moving unit are crossable
						cost += AStarMovingUnitCrossingCost;
					} else {
						// for non moving unit Always Fail
						// FIXME: Need support for moving a fixed unit to add cost
						*c = -1;
						return -1;
						//cost += AStarFixedUnitCrossingCost;
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
			} while(--i);
			index += AStarMapWidth;
		} while(--h);
		*c = cost;
		return cost;
	}	
	return (*c = CostMoveToCallback(index, data));
}

/**
**  MarkAStarGoal
*/
static int AStarMarkGoal(int gx, int gy, int gw, int gh,
	int tilesizex, int tilesizey, int minrange, int maxrange, void *data)
{
	ProfileBegin("AStarMarkGoal");

	int cx[4];
	int cy[4];
	int steps;
	int cycle;
	int x;
	int y;
	bool goal_reachable;
	int quad;
	int eo;
	int filler;
	int range;
	int z1, z2;
	bool doz1, doz2;

	goal_reachable = false;

	if (minrange == 0 && maxrange == 0 && gw == 0 && gh == 0) {
		if (gx + tilesizex >= AStarMapWidth ||
				gy + tilesizey >= AStarMapHeight) {
			ProfileEnd("AStarMarkGoal");
			return 0;
		}
		unsigned int offset = GetIndex(gx,gy);
		if (CostMoveTo(offset, data) >= 0) {
			AStarMatrix[offset].InGoal = 1;
			ProfileEnd("AStarMarkGoal");
			return 1;
		} else {
			ProfileEnd("AStarMarkGoal");
			return 0;
		}
	}

	if (minrange == 0) {
		int sx = std::max(gx, 0);
		int sy = std::max(gy, 0);
		int ey = std::min(gy + gh, AStarMapHeight - tilesizey);
		int ex = std::min(gx + gw, AStarMapWidth - tilesizex);
		unsigned int offset = sy * AStarMapWidth;// + x; 		
		for (y = sy; y < ey; ++y) {
			for (x = sx; x < ex; ++x) {
				if (CostMoveTo(offset + x, data) >= 0) {
					AStarMatrix[offset + x].InGoal = 1;
					goal_reachable = true;
				}
				AStarAddToClose(offset + x);
			}
			offset += AStarMapWidth;
		}
	}

	if (gw) {
		gw--;
	}
	if (gh) {
		gh--;
	}

	int sx = std::max(gx, 0);
	int ex = std::min(gx + gw, AStarMapWidth - tilesizex);
	int sy = std::max(gy, 0);
	int ey = std::min(gy + gh, AStarMapHeight - tilesizey);

	// Mark top, bottom, left, right
	for (range = minrange; range <= maxrange; ++range) {
		z1 = gy - range;
		z2 = gy + range + gh;
		doz1 = z1 >= 0 && z1 + tilesizex - 1 < AStarMapHeight;
		doz2 = z2 >= 0 && z2 + tilesizey - 1 < AStarMapHeight;
		if (doz1) {
			unsigned int offset = z1 * AStarMapWidth;
			// Mark top of goal
			for (x = sx; x <= ex; ++x) {
				if (CostMoveTo(offset + x, data) >= 0) {
					AStarMatrix[offset + x].InGoal = 1;
					AStarAddToClose(offset + x);
					goal_reachable = true;
				}
			}
		}
		if (doz2) {
			unsigned int offset = z2 * AStarMapWidth;
			// Mark bottom of goal
			for (x = sx; x <= ex; ++x) {
				if (CostMoveTo(offset + x, data) >= 0) {
					AStarMatrix[offset + x].InGoal = 1;
					AStarAddToClose(offset + x);
					goal_reachable = true;
				}
			}
		}
		z1 = gx - range;
		z2 = gx + gw + range;
		doz1 = z1 >= 0 && z1 + tilesizex - 1 < AStarMapWidth;
		doz2 = z2 >= 0 && z2 + tilesizex - 1 < AStarMapWidth;
		if (doz1 || doz2) {
			// Mark left and right of goal
			unsigned int offset = sy * AStarMapWidth;
			for (y = sy; y <= ey; ++y) {
				if (doz1 && CostMoveTo(offset + z1, data) >= 0) {
					AStarMatrix[offset + z1].InGoal = 1;
					AStarAddToClose(offset + z1);
					goal_reachable = true;
				}
				if (doz2 && CostMoveTo(offset + z2, data) >= 0) {
					AStarMatrix[offset + z2].InGoal = 1;
					AStarAddToClose(offset + z2);
					goal_reachable = true;
				}
				offset +=  AStarMapWidth;
			}
		}
	} // Go Through the Ranges

	// Mark Goal Border in Matrix

	// Mark Edges of goal

	steps = ASVisionLookup[minrange];

	while (ASVisionTable[0][steps] <= maxrange) {
		// 0 - Top right Quadrant
		cx[0] = gx + gw;
		cy[0] = gy - ASVisionTable[0][steps];
		// 1 - Top left Quadrant
		cx[1] = gx;
		cy[1] = gy - ASVisionTable[0][steps];
		// 2 - Bottom Left Quadrant
		cx[2] = gx;
		cy[2] = gy + ASVisionTable[0][steps]+gh;
		// 3 - Bottom Right Quadrant
		cx[3] = gx + gw;
		cy[3] = gy + ASVisionTable[0][steps]+gh;

		++steps;  // Move past blank marker
		while (ASVisionTable[1][steps] != 0 || ASVisionTable[2][steps] != 0) {
			// Loop through for repeat cycle
			cycle = 0;
			while (cycle++ < ASVisionTable[0][steps]) {
				// If we travelled on an angle, mark down as well.
				if (ASVisionTable[1][steps] == ASVisionTable[2][steps]) {
					// do down
					for (quad = 0; quad < 4; ++quad) {
						if (quad < 2) {
							filler = 1;
						} else {
							filler = -1;
						}
						if (cx[quad] >= 0 && cx[quad] + tilesizex - 1 < AStarMapWidth &&
								cy[quad] + filler >= 0 && cy[quad] + filler + tilesizey - 1 < AStarMapHeight) {
							eo = GetIndex(cx[quad], cy[quad] + filler);
							if(CostMoveTo(eo, data) >= 0) {
								AStarMatrix[eo].InGoal = 1;
								AStarAddToClose(eo);
								goal_reachable = true;
							}
						}
					}
				}

				cx[0] += ASVisionTable[1][steps];
				cy[0] += ASVisionTable[2][steps];
				cx[1] -= ASVisionTable[1][steps];
				cy[1] += ASVisionTable[2][steps];
				cx[2] -= ASVisionTable[1][steps];
				cy[2] -= ASVisionTable[2][steps];
				cx[3] += ASVisionTable[1][steps];
				cy[3] -= ASVisionTable[2][steps];

				// Mark Actually Goal curve change
				for (quad = 0; quad < 4; ++quad) {
					if (cx[quad] >= 0 && cx[quad] + tilesizex - 1 < AStarMapWidth &&
							cy[quad] >= 0 && cy[quad] + tilesizey - 1 < AStarMapHeight) {
						eo = GetIndex(cx[quad], cy[quad]);
						if(CostMoveTo(eo, data) >= 0) {
							AStarMatrix[eo].InGoal = 1;
							AStarAddToClose(eo);
							goal_reachable = true;
						}
					}
				}
			}
			++steps;
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
static int AStarSavePath(int startX, int startY, int endX, int endY, char *path, int pathLen)
{
	ProfileBegin("AStarSavePath");

	int currX, currY, currO;
	int fullPathLength;
	int pathPos;
	int direction;

	// Figure out the full path length
	fullPathLength = 0;
	currX = endX;
	currY = endY;
	currO = currY * AStarMapWidth;
	while (currX != startX || currY != startY) {
		direction = AStarMatrix[currO + currX].Direction;
		currX -= Heading2X[direction];
		currY -= Heading2Y[direction];
		currO -= Heading2O[direction];
		fullPathLength++;
	}

	// Save as much of the path as we can
	if (path) {
		pathLen = std::min(fullPathLength, pathLen);
		pathPos = fullPathLength;
		currX = endX;
		currY = endY;
		currO = currY * AStarMapWidth;
		while ((currX != startX || currY != startY) && path != NULL) {
			direction = AStarMatrix[currO + currX].Direction;
			currX -= Heading2X[direction];
			currY -= Heading2Y[direction];
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
static int AStarFindSimplePath(int sx, int sy, int gx, int gy, int gw, int gh,
	int tilesizex, int tilesizey, int minrange, int maxrange,
	 char *path, int pathlen, void *data)
{
	ProfileBegin("AStarFindSimplePath");
	// At exact destination point already
	if (gx == sx && gy == sy && minrange == 0) {
		ProfileEnd("AStarFindSimplePath");
		return PF_REACHED;
	}

	// Don't allow unit inside destination area
	if (gx <= sx && sx <= gx + gw - 1 &&
			gy <= sy && sy <= gy + gh - 1) {
		return PF_FAILED;
	}

	int dx = MyAbs(gx - sx);
	int dy = MyAbs(gy - sy);
	int distance = isqrt(dx * dx + dy * dy);

	// Within range of destination
	if (minrange <= distance && distance <= maxrange) {
		ProfileEnd("AStarFindSimplePath");
		return PF_REACHED;
	}

	if (dx <= 1 && dy <= 1) {
		// Move to adjacent cell
		if (CostMoveTo(GetIndex(gx, gy), data) == -1) {
			ProfileEnd("AStarFindSimplePath");
			return PF_UNREACHABLE;
		}

		if (path) {
			path[0] = XY2Heading[gx - sx + 1][gy - sy + 1];
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
int AStarFindPath(int sx, int sy, int gx, int gy, int gw, int gh,
	int tilesizex, int tilesizey, int minrange, int maxrange,
	char *path, int pathlen, void *data)
{
	ProfileBegin("AStarFindPath");

	int i;
	int j;
	int o;
	int ex;
	int ey;
	int eo;
	int x;
	int y;
	int px;
	int py;
	int shortest;
//	int counter;
	int new_cost;
	int costToGoal;
	int path_length;
	int ret = PF_FAILED;

	AStarGoalX = gx;
	AStarGoalY = gy;

	//
	//  Check for simple cases first
	//
	i = AStarFindSimplePath(sx, sy, gx, gy, gw, gh, tilesizex, tilesizey,
			minrange, maxrange, path, pathlen, data);
	if (i != PF_FAILED) {
		ret = i;
		goto Cleanup;
	}

	//
	//  Initialize
	//
	AStarCleanUp();
	CostMoveToCacheCleanUp();

	OpenSetSize = 0;
	CloseSetSize = 0;
	x = sx;
	y = sy;

	if (!AStarMarkGoal(gx, gy, gw, gh, tilesizex, tilesizey,
			minrange, maxrange, data)) {
		// goal is not reachable
		ret = PF_UNREACHABLE;
		goto Cleanup;
	}

	eo = y * AStarMapWidth + x;
	// it is quite important to start from 1 rather than 0, because we use
	// 0 as a way to represent nodes that we have not visited yet.
	AStarMatrix[eo].CostFromStart = 1;
	// 8 to say we are came from nowhere.
	AStarMatrix[eo].Direction = 8;

	// place start point in open, it that failed, try another pathfinder
	costToGoal = AStarCosts(x, y, gx, gy);
	AStarMatrix[eo].CostToGoal = costToGoal;
	if (AStarAddNode(x, y, eo, 1 + costToGoal) == PF_FAILED) {
		ret = PF_FAILED;
		goto Cleanup;
	}
	AStarAddToClose(OpenSet[0].O);
	if (AStarMatrix[eo].InGoal) {
		ret = PF_REACHED;
		goto Cleanup;
	}

//	counter = AStarMapWidth * AStarMapHeight;

	//
	//  Begin search
	//
	while (1) {
		//
		// Find the best node of from the open set
		//
		shortest = AStarFindMinimum();
		x = OpenSet[shortest].X;
		y = OpenSet[shortest].Y;
		o = OpenSet[shortest].O;

		AStarRemoveMinimum(shortest);

		//
		// If we have reached the goal, then exit.
		if (AStarMatrix[o].InGoal == 1) {
			ex = x;
			ey = y;
			break;
		}

#if 0
		//
		// If we have looked too long, then exit.
		//
		if (!counter--) {
			//
			// FIXME: Select a "good" point from the open set.
			// Nearest point to goal.
			AstarDebugPrint("way too long\n");
			ret = PF_FAILED;
			goto Cleanup;
		}
#endif

		//
		// Generate successors of this node.

		// Node that this node was generated from.
		px = x - Heading2X[(int)AStarMatrix[o].Direction];
		py = y - Heading2Y[(int)AStarMatrix[o].Direction];

		for (i = 0; i < 8; ++i) {
			ex = x + Heading2X[i];
			ey = y + Heading2Y[i];

			// Don't check the tile we came from, it's not going to be better
			// Should reduce load on A*

			if (ex == px && ey == py) {
				continue;
			}
			//
			// Outside the map or can't be entered.
			//
			if (ex < 0 || ex + tilesizex - 1 >= AStarMapWidth ||
					ey < 0 || ey + tilesizey - 1 >= AStarMapHeight) {
				continue;
			}
			
			//eo = GetIndex(ex, ey);
			eo = ex + (o-x) + Heading2O[i];
			
			// if the point is "move to"-able and
			// if we have not reached this point before,
			// or if we have a better path to it, we add it to open set
			new_cost = CostMoveTo(eo, data);
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
				costToGoal = AStarCosts(ex, ey, gx, gy);
				AStarMatrix[eo].CostToGoal = costToGoal;
				if (AStarAddNode(ex, ey, eo, AStarMatrix[eo].CostFromStart + costToGoal) == PF_FAILED) {
					ret = PF_FAILED;
					goto Cleanup;
				}
				// we add the point to the close set
				AStarAddToClose(eo);
			} else if (new_cost < AStarMatrix[eo].CostFromStart) {
				// Already visited node, but we have here a better path
				// I know, it's redundant (but simpler like this)
				AStarMatrix[eo].CostFromStart = new_cost;
				AStarMatrix[eo].Direction = i;
				// this point might be already in the OpenSet
				j = AStarFindNode(eo);
				if (j == -1) {
					costToGoal = AStarCosts(ex, ey, gx, gy);
					AStarMatrix[eo].CostToGoal = costToGoal;
					if (AStarAddNode(ex, ey, eo,
							AStarMatrix[eo].CostFromStart + costToGoal) == PF_FAILED) {
						ret = PF_FAILED;
						goto Cleanup;
					}
				} else {
					costToGoal = AStarCosts(ex, ey, gx, gy);
					AStarMatrix[eo].CostToGoal = costToGoal;
					AStarReplaceNode(j, AStarMatrix[eo].CostFromStart + costToGoal);
				}
				// we don't have to add this point to the close set
			}
		}
		if (OpenSetSize <= 0) { // no new nodes generated
			ret = PF_UNREACHABLE;
			goto Cleanup;
		}
	}

	path_length = AStarSavePath(sx, sy, ex, ey, path, pathlen);

	ret = path_length;

Cleanup:
	ProfileEnd("AStarFindPath");
	return ret;
}

struct StatsNode
{
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
void SetAStarFixedUnitCrossingCost(int cost) {
	if (cost <= 3) {
		fprintf(stderr, "AStarFixedUnitCrossingCost must be greater than 3\n");
	}
}
int GetAStarFixedUnitCrossingCost() {
	return AStarFixedUnitCrossingCost;
}

// AStarMovingUnitCrossingCost
void SetAStarMovingUnitCrossingCost(int cost) {
	if (cost <= 3) {
		fprintf(stderr, "AStarMovingUnitCrossingCost must be greater than 3\n");
	}
}
int GetAStarMovingUnitCrossingCost() {
	return AStarMovingUnitCrossingCost;
}

// AStarUnknownTerrainCost
void SetAStarUnknownTerrainCost(int cost) {
	if (cost < 0) {
		fprintf(stderr, "AStarUnknownTerrainCost must be non-negative\n");
		return;
	}
	AStarUnknownTerrainCost = cost;
}
int GetAStarUnknownTerrainCost() {
	return AStarUnknownTerrainCost;
}

//@}
