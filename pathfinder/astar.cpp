//   ___________		     _________		      _____  __
//   \_	  _____/______   ____   ____ \_   ___ \____________ _/ ____\/  |_
//    |    __) \_  __ \_/ __ \_/ __ \/    \  \/\_  __ \__  \\   __\\   __\ 
//    |     \   |  | \/\  ___/\  ___/\     \____|  | \// __ \|  |   |  |
//    \___  /   |__|    \___  >\___  >\______  /|__|  (____  /__|   |__|
//	  \/		    \/	   \/	     \/		   \/
//  ______________________                           ______________________
//			  T H E   W A R   B E G I N S
//	   FreeCraft - A free fantasy real time strategy game engine
//
/**@name astar.c	-	The a* path finder routines. */
//
//	(c) Copyright 1999-2001 by Lutz Sammer and Fabrice Rossi
//
//	$Id$

//@{

/*----------------------------------------------------------------------------
--	Includes
----------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "freecraft.h"
#include "player.h"
#include "unit.h"

#include "pathfinder.h"

/*----------------------------------------------------------------------------
--	Declarations
----------------------------------------------------------------------------*/

typedef struct _node_ {
    short	Direction;	/// Direction for trace back
    short       InGoal;         /// is this point in the goal
    int		CostFromStart;	/// Real costs to reach this point
} Node;

typedef struct _open_ {
    int		X;		/// X coordinate
    int		Y;		/// Y coordinate
    int		O;		/// Offset into matrix
    int		Costs;		/// complete costs to goal
} Open;

/// heuristic cost fonction for a star
#define AStarCosts(sx,sy,ex,ey)	max(abs(sx-ex),abs(sy-ey))

/*----------------------------------------------------------------------------
--	Variables
----------------------------------------------------------------------------*/

/// cost matrix
local Node *AStarMatrix;
/// a list of close nodes, helps to speed up the matrix cleaning
local int *CloseSet;
local int Threshold;
local int OpenSetMaxSize;
local int AStarMatrixSize;
#define MAX_CLOSE_SET_RATIO 4
#define MAX_OPEN_SET_RATIO 8	// 10,16 to small

/// see pathfinder.h
global int AStarFixedUnitCrossingCost=MaxMapWidth*MaxMapHeight;
global int AStarMovingUnitCrossingCost=2;
global int AStarOn=0;

/**
 ** The Open set is handled by a Heap stored in a table
 ** 0 is the root
 ** node i left son is at 2*i+1 and right son is at 2*i+2
 */

/// The set of Open nodes
local Open *OpenSet;
/// The size of the open node set
local int OpenSetSize;

/**
 ** Init A* data structures
 */
global void InitAStar(void)
{
    if(AStarOn) {
	if(!AStarMatrix) {
	    AStarMatrixSize=sizeof(Node)*TheMap.Width*TheMap.Height;
	    AStarMatrix=(Node *)malloc(AStarMatrixSize);
	    Threshold=TheMap.Width*TheMap.Height/MAX_CLOSE_SET_RATIO;
	    CloseSet=(int *)malloc(sizeof(int)*Threshold);
	    OpenSetMaxSize=TheMap.Width*TheMap.Height/MAX_OPEN_SET_RATIO;
	    OpenSet=(Open *)malloc(sizeof(Open)*OpenSetMaxSize);
	}
    }
}

/**
 ** Free A* data structure
 */
global void FreeAStar(void)
{
    if(AStarMatrix) {
	free(AStarMatrix);
	AStarMatrix=0;
	free(CloseSet);
	free(OpenSet);
    }
}

/**
**	Prepare path finder.
*/
local void AStarPrepare(void)
{
    memset(AStarMatrix,0,AStarMatrixSize);
}

/**
 ** Clean up the AStarMatrix
 */
local void AStarCleanUp(int num_in_close)
{
    int i;
    if(num_in_close>=Threshold) {
	AStarPrepare();
    } else {
	for(i=0;i<num_in_close;i++) {
	  AStarMatrix[CloseSet[i]].CostFromStart=0;
	  AStarMatrix[CloseSet[i]].InGoal=0;
	}
    }
}

/**
 ** Find the best node in the current open node set
 ** Returns the position of this node in the open node set (always 0 in the
 ** current heap based implementation)
 */
local int AStarFindMinimum()
{
    return 0;
}

/**
 ** Remove the minimum from the open node set (and update the heap)
 ** pos is the position of the minimum (0 in the heap based implementation)
 */
local void AStarRemoveMinimum(int pos)
{
    int i,j,end;
    Open swap;
    if(--OpenSetSize) {
	OpenSet[pos]=OpenSet[OpenSetSize];
	// now we exchange the new root with its smallest child until the
	// order is correct
	i=0;
	end=(OpenSetSize>>1)-1;
	while(i<=end) {
	    j=(i<<1)+1;
	    if(j<OpenSetSize-1 && OpenSet[j].Costs>=OpenSet[j+1].Costs)
		j++;
	    if(OpenSet[i].Costs>OpenSet[j].Costs) {
		swap=OpenSet[i];
		OpenSet[i]=OpenSet[j];
		OpenSet[j]=swap;
		i=j;
	    } else {
		break;
	    }
	}
    }
}

/**
 ** Add a new node to the open set (and update the heap structure)
 */
local void AStarAddNode(int x,int y,int o,int costs)
{
    int i=OpenSetSize;
    int j;
    Open swap;

    if(OpenSetSize>=OpenSetMaxSize) {
	fprintf(stderr, "A* internal error: raise Open Set Max Size "
		"(current value %d)\n",OpenSetMaxSize);
	Exit(-1);
    }
    OpenSet[i].X=x;
    OpenSet[i].Y=y;
    OpenSet[i].O=o;
    OpenSet[i].Costs=costs;
    OpenSetSize++;
    while(i>0) {
	j=(i-1)>>1;
	if(OpenSet[i].Costs<OpenSet[j].Costs) {
	    swap=OpenSet[i];
	    OpenSet[i]=OpenSet[j];
	    OpenSet[j]=swap;
	    i=j;
	} else {
	    break;
	}
    }
}

/**
 ** Change the cost associated to an open node. The new cost MUST BE LOWER
 ** than the old one in the current heap based implementation.
 */
local void AStarReplaceNode(int pos,int costs)
{
    int i=pos;
    int j;
    Open swap;
    OpenSet[pos].Costs=costs;
    // we need to go up, as the cost can only decrease
    while(i>0) {
	j=(i-1)>>1;
	if(OpenSet[i].Costs<OpenSet[j].Costs) {
	    swap=OpenSet[i];
	    OpenSet[i]=OpenSet[j];
	    OpenSet[j]=swap;
	    i=j;
	} else {
	    break;
	}
    }
}


/**
 ** Check if a node is already in the open set.
 ** Return -1 if not found and the position of the node in the table if found.
 */
local int AStarFindNode(int eo)
{
    int i;
    for( i=0; i<OpenSetSize; ++i ) {
	if( OpenSet[i].O==eo ) {
	    return i;
	}
    }
    return -1;
}

/**
 ** Compute the cost of crossing tile (dx,dy)
 ** -1 -> impossible to cross
 **  0 -> no induced cost, except move
 ** >0 -> costly tile
 */
local int CostMoveTo(int ex,int ey,int mask,int current_cost) {
    int j;
    Unit* goal;

    j=TheMap.Fields[ex+ey*TheMap.Width].Flags&mask;
    if (j) {
	if(j&~(MapFieldLandUnit|MapFieldAirUnit|MapFieldSeaUnit)) {
	    // we can't cross fixed units and other unpassable things
	    return -1;
	}
	if(current_cost>=AStarFixedUnitCrossingCost) {
	    // we are already crossing a fixed unit. We don't need details
	    return AStarMovingUnitCrossingCost;
	} else {
	    // FIXME: johns: must choose the correct unit, only units
	    // FIXME: which block the moving unit should be tested
	    // FIXME: my UnitOnMapTile returns random units.
	    // FIXME: this means a land unit could be blocked by an air unit
	    goal=UnitOnMapTile(ex,ey);
	    if( !goal ) {
		return -1;//FIXME: is this a bug?
	    }
	    if( goal->Moving ) {
		// moving unit are crossable
		return AStarMovingUnitCrossingCost;
	    }
	    // for non moving unit
	    return AStarFixedUnitCrossingCost;
	}
    }
    // empty tile
    return 0;
}

/**
**	Find path.
*/
local int AStarFindPath(Unit* unit,int* pxd,int* pyd)
{
    int i,j;
    int o;
    int x;
    int y;
    int ex;
    int ey;
    int dx,dy;
    int eo,gx,gy,cx,cy,sx;
    int best_x,best_y,best_cost_to_goal,best_cost_from_start;
    int r;
    int shortest;
    int counter;
    int new_cost;
    int cost_to_goal;
    //int last_dir;
    int path_length;
    int num_in_close=0;
    int mask=UnitMovementMask(unit);
    int goal_reachable;
    //int base_mask=mask&~(MapFieldLandUnit|MapFieldAirUnit|MapFieldSeaUnit);
    Unit* goal;
    static int xoffset[]={  0,-1,+1, 0, -1,+1,-1,+1 };
    static int yoffset[]={ -1, 0, 0,+1, -1,-1,+1,+1 };

    DebugLevel3Fn("%Zd %d,%d->%d,%d\n",
	    UnitNumber(unit),
	    unit->X,unit->Y,
	    unit->Command.Data.Move.DX,unit->Command.Data.Move.DY);

    OpenSetSize=0;
   /*    AStarPrepare();*/
    x=unit->X;
    y=unit->Y;
    r=unit->Command.Data.Move.Range;
    goal_reachable=0;
    // Let's first mark goal
    if(unit->Command.Data.Move.Goal) {
	goal=unit->Command.Data.Move.Goal;
	j=goal->Type->Type;
	cx=goal->X;
	cy=goal->Y;
	ey=UnitTypes[j].TileHeight+r-1;
	sx=UnitTypes[j].TileWidth+r-1;
	// approximate goal for A*
	gx=goal->X+UnitTypes[j].TileHeight/2;
	gy=goal->Y+UnitTypes[j].TileWidth/2;
    } else {
	cx=gx=unit->Command.Data.Move.DX;
	cy=gy=unit->Command.Data.Move.DY;
	ey=r;
	sx=r;
	r=0;
    }
    for(;ey>=-r;ey--){
	dy=cy+ey;
	if( dy<0 || dy>=TheMap.Height ) {
	    continue;
	}
	for(ex=sx;ex>=-r;ex--) {
	    dx=cx+ex;
	    if( dx<0 || dx>=TheMap.Width ) {
		continue;
	    }
	    if(CostMoveTo(dx,dy,mask,AStarFixedUnitCrossingCost)>=0) {
		eo=dx*TheMap.Width+dy;
		AStarMatrix[eo].InGoal=1;
		if(num_in_close<Threshold) {
		    CloseSet[num_in_close++]=eo;
		}
		goal_reachable=1;
	    }
	}
    }
    // the following test is removed, we path find even if the goal is not
    // directly reachable.
    //    if(goal_reachable) {
    eo=x*TheMap.Width+y;
    // it is quite important to start from 1 rather than 0, because we use
    // 0 as a way to represent nodes that we have not visited yet.
    AStarMatrix[eo].CostFromStart=1;
    best_cost_from_start=1;
    best_cost_to_goal=AStarCosts(x,y,gx,gy);
    best_x=x;
    best_y=y;
    // place start point in open
    AStarAddNode(x,y,eo,best_cost_to_goal+1);
    if(num_in_close<Threshold) {
	CloseSet[num_in_close++]=OpenSet[0].O;
    }
    //    } else {
    //	AStarCleanUp(num_in_close);
    //	return -2;
    //    }

    counter=TheMap.Width*TheMap.Height;	// how many tries

    for( ;; ) {
	//
	//	Find the best node of from the open set
	//
	shortest=AStarFindMinimum();
	x=OpenSet[shortest].X;
	y=OpenSet[shortest].Y;
	o=OpenSet[shortest].O;
	cost_to_goal=OpenSet[shortest].Costs-AStarMatrix[o].CostFromStart;

	AStarRemoveMinimum(shortest);

	//
	//	If we have reached the goal, then exit.
	if( AStarMatrix[o].InGoal==1 ) {
	    ex=x;
	    ey=y;
	    DebugLevel3Fn("a star goal reached\n");
	    break;
	}

	//
	//	If we have looked too long, then exit.
	//
	if( !counter-- ) {
	    //
	    //	Select a "good" point from the open set.
	    //		Nearest point to goal.
	    DebugLevel0Fn("%Zd way too long\n",UnitNumber(unit));
	    ex=best_x;
	    ey=best_y;
	}

	// update best point so far
	if(cost_to_goal<best_cost_to_goal
	   || (cost_to_goal==best_cost_to_goal
	       && AStarMatrix[o].CostFromStart<best_cost_from_start)) {
	    best_cost_to_goal=cost_to_goal;
	    best_cost_from_start=AStarMatrix[o].CostFromStart;
	    best_x=x;
	    best_y=y;
	}
	DebugLevel3("Best point in Open Set: %d %d (%d)\n",x,y,OpenSetSize);
	//
	//	Generate successors of this node.
	//
	for( i=0; i<8; ++i ) {
	    ex=x+xoffset[i];
	    ey=y+yoffset[i];

	    //
	    //	Outside the map or can't be entered.
	    //
	    if( ex<0 || ex>=TheMap.Width ) {
		continue;
	    }
	    if( ey<0 || ey>=TheMap.Height ) {
		continue;
	    }
	    // if the point is "move to"-able an
	    // if we have not reached this point before,
	    // or if we have a better path to it, we add it to open set
	    new_cost=CostMoveTo(ex,ey,mask,AStarMatrix[o].CostFromStart);
	    if(new_cost==-1) {
		// uncrossable tile
		continue;
	    }
	    eo=ex*TheMap.Width+ey;
	    new_cost+=AStarMatrix[o].CostFromStart+1;
	    if(AStarMatrix[eo].CostFromStart==0) {
		// we are sure the current node has not been already visited
		AStarMatrix[eo].CostFromStart=new_cost;
		AStarMatrix[eo].Direction=i;
		AStarAddNode(ex,ey,eo,
			     AStarMatrix[eo].CostFromStart+AStarCosts(ex,ey,gx,gy));
		// we add the point to the close set
		if(num_in_close<Threshold) {
		    CloseSet[num_in_close++]=eo;
		}
	    } else if(new_cost<AStarMatrix[eo].CostFromStart) {
		// Already visited node, but we have here a better path
		// I know, it's redundant (but simpler like this)
		AStarMatrix[eo].CostFromStart=new_cost;
		AStarMatrix[eo].Direction=i;
		// this point might be already in the OpenSet
		j=AStarFindNode(eo);
		if(j==-1) {
		    AStarAddNode(ex,ey,eo,
				 AStarMatrix[eo].CostFromStart+
				 AStarCosts(ex,ey,gx,gy));
		} else {
		    AStarReplaceNode(j,AStarMatrix[eo].CostFromStart+
				     AStarCosts(ex,ey,gx,gy));
		}
		// we don't have to add this point to the close set
	    }
	}
	if( OpenSetSize<=0 ) {		// no new nodes generated
	    // we go to the best node
	    ex=best_x;
	    ey=best_y;
	    if(ex==unit->X && ey==unit->Y) {
		DebugLevel3Fn("%Zd unreachable\n",UnitNumber(unit));
		AStarCleanUp(num_in_close);
		return -2;
	    }
	    DebugLevel3Fn("%Zd unreachable: going to closest\n",UnitNumber(unit));
	    break;
	}
    }
    // if the goal was not reachable, we replace it by the best point
    // this will speed up next path finding
    if(!goal_reachable) {
	unit->Command.Data.Move.Goal=0;
	unit->Command.Data.Move.DX=ex;
	unit->Command.Data.Move.DY=ey;
	ResetPath(unit->Command);
    }
    // now we need to backtrack
    path_length=0;
    x=unit->X;
    y=unit->Y;
    i=0;
    while(ex!=x||ey!=y) {
	DebugLevel3("%d %d %d %d\n",x,y,ex,ey);
	eo=ex*TheMap.Width+ey;
	i=AStarMatrix[eo].Direction;
	ex-=xoffset[i];
	ey-=yoffset[i];
	path_length++;
    }
    *pxd=xoffset[i];
    *pyd=yoffset[i];
    j=CostMoveTo(ex+*pxd,ey+*pyd,mask,0);
    if(j!=0) {
	if(j==AStarMovingUnitCrossingCost) {
	    // we should wait, we are blocked by a moving unit
	    //FIXME: this might lead to a deadlock, or something similar
	    DebugLevel3("Unit %Zd waiting. Proposed move: %d %d\n",
			UnitNumber(unit),*pxd,*pyd);
	    path_length=0;
	} else {
	    // j==-1 is a bug, so we should have only
	    // j==AStarFixedUnitCrossingCost, which means
	    // the way is blocked by a non moving unit. Waiting is here
	    // pointless.
	    path_length=-2;
	}
    }
    // let's clean up the matrix now
    AStarCleanUp(num_in_close);
    DebugLevel3Fn("%Zd\n",UnitNumber(unit));
    DebugLevel3Fn("proposed move: %d %d (%d)\n",*pxd,*pyd,path_length);
    return path_length;
}

/**
**	Returns the next element of a path with astar algo.
**
**	@param unit	Unit that wants the path element.
**	@param pxd	Pointer for the x direction.
**	@param pyd	Pointer for the y direction.
**
**	@return		>0 remaining path length, 0 wait for path, -1
**			reached goal, -2 can't reach the goal.
*/
global int AStarNextPathElement(Unit* unit,int* pxd,int *pyd)
{
    // FIXME: Cache for often used pathes, like peons to goldmine.
    // FIXME: (fabrice) I've copied here the code from NewPath. Is it really
    // needed?
    int x;
    int y;
    int r=unit->Command.Data.Move.Range;
    Unit* goal=unit->Command.Data.Move.Goal;
    UnitType* type;

    x=unit->X;
    y=unit->Y;
    if( goal ) {			// goal unit
	type=goal->Type;
	DebugLevel3Fn("Unit %d,%d Goal %d,%d - %d,%d\n"
	    ,x,y
	    ,goal->X-r,goal->Y-r
	    ,goal->X+type->TileWidth+r
	    ,goal->Y+type->TileHeight+r);
	if( x>=goal->X-r && x<goal->X+type->TileWidth+r
		&& y>=goal->Y-r && y<goal->Y+type->TileHeight+r ) {
	    DebugLevel3Fn("Goal reached\n");
	    *pxd=*pyd=0;
	    return -1;
	}
    } else {				// goal map field
	if( x>=unit->Command.Data.Move.DX
		&& x<=unit->Command.Data.Move.DX+r
		&& y>=unit->Command.Data.Move.DY
		&& y<=unit->Command.Data.Move.DY+r ) {
	    DebugLevel3Fn("Field reached\n");
	    *pxd=*pyd=0;
	    return -1;
	}
	// This reduces the processor use,
	// If target isn't reachable and were beside it
	if( r==0 && x>=unit->Command.Data.Move.DX-1
		&& x<=unit->Command.Data.Move.DX+1
		&& y>=unit->Command.Data.Move.DY-1
		&& y<=unit->Command.Data.Move.DY+1 ) {
	    if( !CheckedCanMoveToMask(unit->Command.Data.Move.DX
		    ,unit->Command.Data.Move.DY
		    ,UnitMovementMask(unit)) ) {	// blocked
		DebugLevel3Fn("Field unreached\n");
		*pxd=*pyd=0;
		return -2;
	    }
	}
    }

    return AStarFindPath(unit,pxd,pyd);
}

/**
**	Returns the next element of a path.
**
**	@param unit	Unit that wants the path element.
**	@param pxd	Pointer for the x direction.
**	@param pyd	Pointer for the y direction.
**
**	@return		>0 remaining path length, 0 wait for path, -1
**			reached goal, -2 can't reach the goal.
*/
global int NextPathElement(Unit* unit,int* pxd,int *pyd)
{
    // Convert old version to new version
    if(AStarOn) {
	return AStarNextPathElement(unit,pxd,pyd);
    } else {
	return NewPath(unit,pxd,pyd);
    }
}

//@}
