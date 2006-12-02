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
//      (c) Copyright 1999-2006 by Lutz Sammer,Fabrice Rossi, Russell Smith,
//                                  Francois Beerten.
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

#include "stratagus.h"
#include "player.h"
#include "unit.h"
#include "unittype.h"
#include "pathfinder.h"

#if defined(DEBUG_ASTAR)
#define AstarDebugPrint(x) DebugPrint(x)
#else
#define AstarDebugPrint(x)
#endif

/*----------------------------------------------------------------------------
-- Declarations
----------------------------------------------------------------------------*/

struct Node {
	char Direction;     /// Direction for trace back
	char InGoal;        /// is this point in the goal
	int CostFromStart;  /// Real costs to reach this point
};

struct Open {
	int X;     /// X coordinate
	int Y;     /// Y coordinate
	int O;     /// Offset into matrix
	int Costs; /// complete costs to goal
};

/// heuristic cost fonction for a star
#define AStarCosts(sx,sy,ex,ey) (abs(sx-ex)+abs(sy-ey))
// Other heuristic functions
// #define AStarCosts(sx,sy,ex,ey) 0
// #define AStarCosts(sx,sy,ex,ey) isqrt((abs(sx-ex)*abs(sx-ex))+(abs(sy-ey)*abs(sy-ey)))
// #define AStarCosts(sx,sy,ex,ey) max(abs(sx-ex),abs(sy-ey))
/*----------------------------------------------------------------------------
-- Variables
----------------------------------------------------------------------------*/

//  Convert heading into direction.
//                      //  N NE  E SE  S SW  W NW
const int Heading2X[9] = {  0,+1,+1,+1, 0,-1,-1,-1, 0 };
const int Heading2Y[9] = { -1,-1, 0,+1,+1,+1, 0,-1, 0 };
const int XY2Heading[3][3] = { {7,6,5},{0,0,4},{1,2,3}};
/// cost matrix
static Node *AStarMatrix;
/// a list of close nodes, helps to speed up the matrix cleaning
static int *CloseSet;
static int Threshold;
static int OpenSetMaxSize;
static int AStarMatrixSize;
#define MAX_CLOSE_SET_RATIO 4
#define MAX_OPEN_SET_RATIO 8 // 10,16 to small

/// see pathfinder.h
int AStarFixedUnitCrossingCost = MaxMapWidth * MaxMapHeight;
int AStarMovingUnitCrossingCost = 5;
int AStarKnowUnknown = 0;
int AStarUnknownTerrainCost = 2;

/**
** The Open set is handled by a stored array
** the end of the array holds the item witht he smallest cost.
*/

/// The set of Open nodes
static Open *OpenSet;
/// The size of the open node set
static int OpenSetSize;

/**
** Init A* data structures
*/
void InitAStar(void)
{
	if (!AStarMatrix) {
		AStarMatrixSize = sizeof(Node) * Map.Info.MapWidth * Map.Info.MapHeight;
		AStarMatrix = new Node[Map.Info.MapWidth * Map.Info.MapHeight];
		memset(AStarMatrix, 0, AStarMatrixSize);
		Threshold = Map.Info.MapWidth * Map.Info.MapHeight / MAX_CLOSE_SET_RATIO;
		CloseSet = new int[Threshold];
		OpenSetMaxSize = Map.Info.MapWidth * Map.Info.MapHeight / MAX_OPEN_SET_RATIO;
		OpenSet = new Open[OpenSetMaxSize];
	}
}

/**
** Free A* data structure
*/
void FreeAStar(void)
{
	if (AStarMatrix) {
		delete[] AStarMatrix;
		AStarMatrix = NULL;
		delete[] CloseSet;
		delete[] OpenSet;
	}
}

/**
** Prepare path finder.
*/
static void AStarPrepare(void)
{
	memset(AStarMatrix, 0, AStarMatrixSize);
}

/**
** Clean up the AStarMatrix
*/
static void AStarCleanUp(int num_in_close)
{
	if (num_in_close >= Threshold) {
		AStarPrepare();
	} else {
		for (int i = 0; i < num_in_close; ++i) {
			AStarMatrix[CloseSet[i]].CostFromStart = 0;
			AStarMatrix[CloseSet[i]].InGoal = 0;
		}
	}
}

/**
** Find the best node in the current open node set
** Returns the position of this node in the open node set 
*/
#define AStarFindMinimum() (OpenSetSize - 1)


/**
** Remove the minimum from the open node set
*/
static void AStarRemoveMinimum(int pos)
{
	Assert(pos == OpenSetSize - 1);

	OpenSetSize--;
}

/**
** Add a new node to the open set (and update the heap structure)
** Returns Pathfinder failed
*/
static int AStarAddNode(int x, int y, int o, int costs)
{
	int bigi, smalli;
	int midcost;
	int midi;
	
	if (OpenSetSize + 1 >= OpenSetMaxSize) {
		fprintf(stderr, "A* internal error: raise Open Set Max Size "
				"(current value %d)\n", OpenSetMaxSize);
		return PF_FAILED;
	}

	
	// find where we should insert this node.
	bigi = 0;
	smalli = OpenSetSize;

	// binary search where to insert the new node
	while (bigi < smalli) {
		midi = (smalli + bigi) >> 1;
		midcost = OpenSet[midi].Costs;
		if (costs > midcost) {
			smalli = midi;
		} else if (costs < midcost ) {
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

	return 0;
}

/**
** Change the cost associated to an open node. 
** Can be further optimised knowing that the new cost MUST BE LOWER
** than the old one.
*/
static void AStarReplaceNode(int pos, int costs)
{
	Open node;

	// Remove the outdated node
	node = OpenSet[pos];
	OpenSetSize--;
	memmove(&OpenSet[pos], &OpenSet[pos+1], sizeof(Open) * (OpenSetSize-pos));

	// Re-add the node with the new cost
	AStarAddNode(node.X, node.Y, node.O, node.Costs);
}


/**
** Check if a node is already in the open set.
** Return -1 if not found and the position of the node in the table if found.
*/
static int AStarFindNode(int eo)
{
	for (int i = 0; i < OpenSetSize; ++i) {
		if (OpenSet[i].O == eo) {
			return i;
		}
	}
	return -1;
}

/**
**  Compute the cost of crossing tile (dx,dy)
**
**  @param unit          unit which move.
**  @param ex            X tile where to move.
**  @param ey            Y tile where to move.
**  @param mask          mask movement of the unit.
**
**  @return              -1 -> impossible to cross.
**                        0 -> no induced cost, except move
**                       >0 -> costly tile
*/
static int CostMoveTo(const CUnit *unit, int ex, int ey, int mask) {
	int i;     // iterator on tilesize.
	int j;     // iterator on tilesize.
	int flag;  // flag of the map.
	int cost;  // result cost.
	CUnit *goal;
	cost = 0;

	// Doesn't cost anything to move to ourselves :)
	// Used when marking goals mainly.  Could cause speed problems
	if (unit->X == ex && unit->Y == ey) {
		return 0;
	}

	// verify each tile of the unit.
	for (i = ex; i < ex + unit->Type->TileWidth; i++) {
		for (j = ey; j < ey + unit->Type->TileHeight; j++) {
			flag = Map.Fields[i + j * Map.Info.MapWidth].Flags & mask;
			if (flag && (AStarKnowUnknown || Map.IsFieldExplored(unit->Player, i, j)) ) {
				if (flag & ~(MapFieldLandUnit | MapFieldAirUnit | MapFieldSeaUnit)) {
					// we can't cross fixed units and other unpassable things
					return -1;
				}
				goal = UnitCacheOnXY(i, j, unit->Type->UnitType);
				if (!goal) {
					// Shouldn't happen, mask says there is something on this tile
					Assert(0);
					return -1;
				}
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
			if (!AStarKnowUnknown && !Map.IsFieldExplored(unit->Player, i, j) ) {
				// Tend against unknown tiles.
				cost += AStarUnknownTerrainCost;
			}
			// Add tile movement cost
			cost += Map.Fields[i + j * Map.Info.MapWidth].Cost;
		}
	}
	return cost;
}

/**
**  MarkAStarGoal
*/
static int AStarMarkGoal(const CUnit *unit, int gx, int gy, int gw, int gh, int minrange, int maxrange,
	int mask, int *num_in_close)
{
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
		if (gx + unit->Type->TileWidth >= Map.Info.MapWidth ||
				gy + unit->Type->TileHeight >= Map.Info.MapHeight) {
			return 0;
		}
		if (CostMoveTo(unit, gx, gy, mask) >= 0) {
			AStarMatrix[gx + gy * Map.Info.MapWidth].InGoal = 1;
			return 1;
		} else {
			return 0;
		}
	}

	if (minrange == 0) {
		int sx = std::max(gx, 0);
		int ex = std::min(gx + gw, Map.Info.MapWidth - unit->Type->TileWidth);
		for (x = sx; x < ex; ++x) {
			int sy = std::max(gy, 0);
			int ey = std::min(gy + gh, Map.Info.MapHeight - unit->Type->TileHeight);
			for (y = sy; y < ey; ++y) {
				if (CostMoveTo(unit, x, y, mask) >= 0) {
					AStarMatrix[y * Map.Info.MapWidth + x].InGoal = 1;
					goal_reachable = true;
				}
				if (*num_in_close < Threshold) {
					CloseSet[(*num_in_close)++] = y * Map.Info.MapWidth + x;
				}
			}
		}
	}

	if (gw) {
		gw--;
	}
	if (gh) {
		gh--;
	}

	int sx = std::max(gx, 0);
	int ex = std::min(gx + gw, Map.Info.MapWidth - unit->Type->TileWidth - 1);
	int sy = std::max(gy, 0);
	int ey = std::min(gy + gh, Map.Info.MapHeight - unit->Type->TileHeight - 1);

	// Mark top, bottom, left, right
	for (range = minrange; range <= maxrange; ++range) {
		z1 = gy - range;
		z2 = gy + range + gh;
		doz1 = z1 >= 0 && z1 < Map.Info.MapHeight - unit->Type->TileHeight;
		doz2 = z2 >= 0 && z2 < Map.Info.MapHeight - unit->Type->TileHeight;
		if (doz1 || doz2) {
			// Mark top and bottom of goal
			for (x = sx; x <= ex; ++x) {
				if (doz1 && CostMoveTo(unit, x, z1, mask) >= 0) {
					AStarMatrix[z1 * Map.Info.MapWidth + x].InGoal = 1;
					if (*num_in_close < Threshold) {
						CloseSet[(*num_in_close)++] = z1 * Map.Info.MapWidth + x;
					}
					goal_reachable = true;
				}
				if (doz2 && CostMoveTo(unit, x, z2, mask) >= 0) {
					AStarMatrix[z2 * Map.Info.MapWidth + x].InGoal = 1;
					if (*num_in_close < Threshold) {
						CloseSet[(*num_in_close)++] = z2 * Map.Info.MapWidth + x;
					}
					goal_reachable = true;
				}
			}
		}
		z1 = gx - range;
		z2 = gx + gw + range;
		doz1 = z1 >= 0 && z1 < Map.Info.MapHeight - unit->Type->TileHeight;
		doz2 = z2 >= 0 && z2 < Map.Info.MapHeight - unit->Type->TileHeight;
		if (doz1 || doz2) {
			// Mark left and right of goal
			for (y = sy; y <= ey; ++y) {
				if (doz1 && CostMoveTo(unit, z1, y, mask) >= 0) {
					AStarMatrix[y * Map.Info.MapWidth + z1].InGoal = 1;
					if (*num_in_close < Threshold) {
						CloseSet[(*num_in_close)++] = y * Map.Info.MapWidth + z1;
					}
					goal_reachable = true;
				}
				if (doz2 && CostMoveTo(unit, z2, y, mask) >= 0) {
					AStarMatrix[y * Map.Info.MapWidth + z2].InGoal = 1;
					if (*num_in_close < Threshold) {
						CloseSet[(*num_in_close)++] = y * Map.Info.MapWidth + z2;
					}
					goal_reachable = true;
				}
			}
		}
	} // Go Through the Ranges

	// Mark Goal Border in Matrix

	// Mark Edges of goal

	steps = VisionLookup[minrange];

	while (VisionTable[0][steps] <= maxrange) {
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
					for (quad = 0; quad < 4; ++quad) {
						if (quad < 2) {
							filler = 1;
						} else {
							filler = -1;
						}
						if (cx[quad] >= 0 && cx[quad] + unit->Type->TileWidth < Map.Info.MapWidth &&
								cy[quad] + filler >= 0 && cy[quad] + filler + unit->Type->TileHeight < Map.Info.MapHeight &&
								CostMoveTo(unit, cx[quad], cy[quad] + filler, mask) >= 0) {
							eo = (cy[quad] + filler) * Map.Info.MapWidth + cx[quad];
							AStarMatrix[eo].InGoal = 1;
							if (*num_in_close < Threshold) {
								CloseSet[(*num_in_close)++] = eo;
							}
							goal_reachable = true;
						}
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
				for (quad = 0; quad < 4; ++quad) {
					if (cx[quad] >= 0 && cx[quad] + unit->Type->TileWidth < Map.Info.MapWidth &&
							cy[quad] >= 0 && cy[quad] + unit->Type->TileHeight < Map.Info.MapHeight &&
							CostMoveTo(unit, cx[quad], cy[quad], mask) >= 0) {
						eo = cy[quad] * Map.Info.MapWidth + cx[quad];
						AStarMatrix[eo].InGoal = 1;
						if (*num_in_close < Threshold) {
							CloseSet[(*num_in_close)++] = eo;
						}
						goal_reachable = true;
					}
				}
			}
			++steps;
		}
	}
	return goal_reachable;
}

/**
** Find path.
*/
int AStarFindPath(const CUnit *unit, int gx, int gy, int gw, int gh, int minrange, int maxrange, char *path)
{
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
	int counter;
	int new_cost;
	int cost_to_goal;
	int path_length;
	int num_in_close;
	int mask;

	if (maxrange == 0 && abs(gx - unit->X) <= 1 && abs(gy - unit->Y) <= 1) {
		// Simplest case, move to adj cell
		if (gx == unit->X && gy == unit->Y) {
			return PF_REACHED;
		}

		if (path) {
			path[0] = XY2Heading[gx - unit->X + 1][gy - unit->Y + 1];
		}
		return 1;
	}

	OpenSetSize = 0;
	num_in_close = 0;
	mask = unit->Type->MovementMask;
	x = unit->X;
	y = unit->Y;

	// if goal is not directory reachable, punch out
	if (!AStarMarkGoal(unit, gx, gy, gw, gh, minrange, maxrange, mask, &num_in_close)) {
		AStarCleanUp(num_in_close);
		return PF_UNREACHABLE;
	}

	eo = y * Map.Info.MapWidth + x;
	// it is quite important to start from 1 rather than 0, because we use
	// 0 as a way to represent nodes that we have not visited yet.
	AStarMatrix[eo].CostFromStart=1;
	// 8 to say we are came from nowhere.
	AStarMatrix[eo].Direction=8;

	// place start point in open, it that failed, try another pathfinder
	if( AStarAddNode(x,y,eo,1+AStarCosts(x,y,gx,gy)) == PF_FAILED ) {
		AStarCleanUp(num_in_close);
		return PF_FAILED;
	}
	if( num_in_close<Threshold ) {
		CloseSet[num_in_close++]=OpenSet[0].O;
	}
	if( AStarMatrix[eo].InGoal ) {
		AStarCleanUp(num_in_close);
		return PF_REACHED;
	}

	counter=Map.Info.MapWidth*Map.Info.MapHeight;

	while( 1 ) {
		//
		// Find the best node of from the open set
		//
		shortest=AStarFindMinimum();
		x=OpenSet[shortest].X;
		y=OpenSet[shortest].Y;
		o=OpenSet[shortest].O;
		cost_to_goal=OpenSet[shortest].Costs-AStarMatrix[o].CostFromStart;

		AStarRemoveMinimum(shortest);

		//
		// If we have reached the goal, then exit.
		if( AStarMatrix[o].InGoal==1 ) {
			ex=x;
			ey=y;
			break;
		}

		//
		// If we have looked too long, then exit.
		//
		if( !counter-- ) {
			//
			// Select a "good" point from the open set.
			// Nearest point to goal.
			DebugPrint("%d way too long\n" _C_ UnitNumber(unit));
			AStarCleanUp(num_in_close);
			return PF_FAILED;
		}

		//
		// Generate successors of this node.

		// Node that this node was generated from.
		px=x-Heading2X[(int)AStarMatrix[x+Map.Info.MapWidth*y].Direction];
		py=y-Heading2Y[(int)AStarMatrix[x+Map.Info.MapWidth*y].Direction];

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
			if (ex < 0 || ex + unit->Type->TileWidth >= Map.Info.MapWidth ||
					ey < 0 || ey + unit->Type->TileHeight >= Map.Info.MapHeight) {
				continue;
			}

			// if the point is "move to"-able and
			// if we have not reached this point before,
			// or if we have a better path to it, we add it to open set
			new_cost = CostMoveTo(unit, ex, ey, mask);
			if (new_cost == -1) {
				// uncrossable tile
				continue;
			}

			// Add a cost for walking to make paths more realistic for the user.
			new_cost+=abs(Heading2X[i])+abs(Heading2Y[i])+1;
			eo=ey*Map.Info.MapWidth+ex;
			new_cost+=AStarMatrix[o].CostFromStart;
			if( AStarMatrix[eo].CostFromStart==0 ) {
				// we are sure the current node has not been already visited
				AStarMatrix[eo].CostFromStart=new_cost;
				AStarMatrix[eo].Direction=i;
				if( AStarAddNode(ex,ey,eo,AStarMatrix[eo].CostFromStart+AStarCosts(ex,ey,gx,gy)) == PF_FAILED ) {
					AStarCleanUp(num_in_close);
					return PF_FAILED;
				}
				// we add the point to the close set
				if( num_in_close<Threshold ) {
					CloseSet[num_in_close++]=eo;
				}
			} else if( new_cost<AStarMatrix[eo].CostFromStart ) {
				// Already visited node, but we have here a better path
				// I know, it's redundant (but simpler like this)
				AStarMatrix[eo].CostFromStart=new_cost;
				AStarMatrix[eo].Direction=i;
				// this point might be already in the OpenSet
				j=AStarFindNode(eo);
				if( j==-1 ) {
					if( AStarAddNode(ex,ey,eo,
								 AStarMatrix[eo].CostFromStart+
								 AStarCosts(ex,ey,gx,gy)) == PF_FAILED ) {
						AStarCleanUp(num_in_close);
						return PF_FAILED;
					}
				} else {
					AStarReplaceNode(j,AStarMatrix[eo].CostFromStart+
									 AStarCosts(ex,ey,gx,gy));
				}
				// we don't have to add this point to the close set
			}
		}
		if( OpenSetSize<=0 ) { // no new nodes generated
			AStarCleanUp(num_in_close);
			return PF_UNREACHABLE;
		}
	}
	// now we need to backtrack
	path_length=0;
	x=unit->X;
	y=unit->Y;
	gx=ex;
	gy=ey;
	i=0;
	while( ex!=x || ey!=y ) {
		eo=ey*Map.Info.MapWidth+ex;
		i=AStarMatrix[eo].Direction;
		ex-=Heading2X[i];
		ey-=Heading2Y[i];
		path_length++;
	}

	// gy = Path length to cache
	// gx = Current place in path
	ex=gx;
	ey=gy;
	gy=path_length;
	gx=path_length;
	if( gy>MAX_PATH_LENGTH ) {
		gy=MAX_PATH_LENGTH;
	}

	// Now we have the length, calculate the cached path.
	while( (ex!=x || ey!=y) && path!=NULL ) {
		eo=ey*Map.Info.MapWidth+ex;
		i=AStarMatrix[eo].Direction;
		ex-=Heading2X[i];
		ey-=Heading2Y[i];
		--gx;
		if( gx<gy ) {
			path[gy-gx-1]=i;
		}
	}

	// let's clean up the matrix now
	AStarCleanUp(num_in_close);
	if ((Map.Info.MapWidth*Map.Info.MapHeight) - counter > 500) {
		AstarDebugPrint("%s:%d Visited %d tiles, length %d tiles\n" _C_ unit->Type->Name _C_ UnitNumber(unit) 
			_C_ (Map.Info.MapWidth*Map.Info.MapHeight) - counter _C_ path_length);
	}
	return path_length;
}

/**
** Returns the next element of a path.
**
** @param unit   Unit that wants the path element.
** @param pxd    Pointer for the x direction.
** @param pyd    Pointer for the y direction.
**
** @return >0 remaining path length, 0 wait for path, -1
** reached goal, -2 can't reach the goal.
*/
int NextPathElement(CUnit *unit, int *pxd, int *pyd)
{
	int result;

	// Attempt to use path cache
	// FIXME: If there is a goal, it may have moved, ruining the cache
	*pxd = 0;
	*pyd = 0;

	// Goal has moved, need to recalculate path or no cached path
	if( unit->Data.Move.Length <= 0 ||
		( unit->Orders[0]->Goal && (unit->Orders[0]->Goal->X != unit->Orders[0]->X
			|| unit->Orders[0]->Goal->Y != unit->Orders[0]->Y)) ) {
		result=NewPath(unit);

		if( result==PF_UNREACHABLE ) {
			unit->Data.Move.Length=0;
			return result;
		}
		if( result==PF_REACHED ) {
			return result;
		}
		if( unit->Goal ) {
			// Update Orders
			unit->Orders[0]->X=unit->Goal->X;
			unit->Orders[0]->Y=unit->Goal->Y;
		}
	}

	*pxd=Heading2X[(int)unit->Data.Move.Path[(int)unit->Data.Move.Length-1]];
	*pyd=Heading2Y[(int)unit->Data.Move.Path[(int)unit->Data.Move.Length-1]];
	result=unit->Data.Move.Length;
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
			result=NewPath(unit);
			if( result>0 ) {
				*pxd=Heading2X[(int)unit->Data.Move.Path[(int)unit->Data.Move.Length-1]];
				*pyd=Heading2Y[(int)unit->Data.Move.Path[(int)unit->Data.Move.Length-1]];
				if (!UnitCanBeAt(unit, *pxd + unit->X, *pyd + unit->Y)) {
					// There may be unit in the way, Astar may allow you to walk onto it.
					result=PF_UNREACHABLE;
					*pxd=0;
					*pyd=0;
				} else {
					result=unit->Data.Move.Length;
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
