/*
**	A clone of a famous game.
*/
/**@name astar.c	-	The a* path finder routines. */
/*
**	(c) Copyright 1999 by Lutz Sammer
**
**	$Id$
*/

//@{

#include <stdio.h>

#include "clone.h"
#include "player.h"
#include "unit.h"

#include "pathfinder.h"

typedef struct _node_ {
    int		Direction;	/// Direction for trace back
    int		CostFromStart;	/// Real costs to reach this point
    int		CostToGoal;	/// Aproximated costs until goal
} Node;

typedef struct _open_ {
    int		X;		/// X coordinate
    int		Y;		/// Y coordinate
    int		O;		/// Offset into matrix
    int		Costs;		/// complete costs to goal
} Open;

#define AStarCosts(sx,sy,ex,ey)	max(abs(sx-ex),abs(sy-ey))

local Node AStarMatrix[(MaxMapWidth+1)*(MaxMapHeight+1)];

/**
**	Prepare path finder.
*/
local void AStarPrepare(void)
{
    memset(AStarMatrix,0,sizeof(AStarMatrix));
}

/**
**	Find path.	
*/
local int AStarFindPath(Unit* unit,int* pxd,int* pyd)
{
    int i;
    int o;
    int x;
    int y;
    int ex;
    int ey;
    int num_in_open;
    int shortest;
    int counter;
    Open OpenSet[MaxMapWidth*MaxMapHeight/64];
    static int xoffset[]={  0,-1,+1, 0, -1,+1,-1,+1 };
    static int yoffset[]={ -1, 0, 0,+1, -1,-1,+1,+1 };

    DebugLevel0(__FUNCTION__": %Zd %d,%d->%d,%d\n",
	    UnitNumber(unit),
	    unit->X,unit->Y,
	    unit->Command.Data.Move.DX,unit->Command.Data.Move.DY);

    AStarPrepare();
    x=unit->X;
    y=unit->Y;
    ex=unit->Command.Data.Move.DX;
    ey=unit->Command.Data.Move.DY;

    OpenSet[0].X=x;			// place start point in open
    OpenSet[0].Y=y;
    OpenSet[0].O=x*TheMap.Width+y;
    OpenSet[0].Costs=AStarCosts(x,y,ex,ey);

    AStarMatrix[OpenSet[0].O].CostFromStart=0;	// mark in matrix
    AStarMatrix[OpenSet[0].O].CostToGoal=OpenSet[0].Costs;

    num_in_open=1;
    counter=MaxMapWidth*MaxMapHeight;	// how many tries

    for( ;; ) {
	//
	//	Find the best node of from the open set
	//
	for( i=shortest=0; i<num_in_open; ++i ) {
	    if( OpenSet[i].Costs<OpenSet[shortest].Costs ) {
		shortest=i;
	    }
	}
	x=OpenSet[shortest].X;
	y=OpenSet[shortest].Y;
	o=OpenSet[shortest].O;

	// remove by inserting the last
	OpenSet[shortest]=OpenSet[num_in_open--];

	//
	//	If we have reached the goal, then exit.
	//
	if( x==ex && y==ey ) {
	    break;
	}

	//
	//	If we have looked too long, then exit.
	//
	if( !counter-- ) {
	    //
	    //	Select a "good" point from the open set.
	    //		Nearest point to goal.
	    /*
	    for( i=shortest=0; i<num_in_open; ++i ) {
		if( OpenSet[i].Costs<OpenSet[shortest].Costs ) {
		    shortest=i;
		}
	    }
	    */
	    DebugLevel0(__FUNCTION__": %Zd way too long\n",UnitNumber(unit));
	    return 0;
	}

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
	}

	if( !num_in_open ) {		// no new nodes generated
	    DebugLevel0(__FUNCTION__": %Zd unreachable\n",UnitNumber(unit));
	    return 0;
	}
    }

    DebugLevel0(__FUNCTION__": %Zd\n",UnitNumber(unit));
    return 0;
}

/**
**	Returns the next element of a path with astar algo.
**
**	@param unit	Unit that wants the path element. 
**	@param pxd	Pointer for the x direction.
**	@param pyd	Pointer for the y direction.
**
**	@returns	>0 remaining path length, 0 wait for path, -1
**			reached goal, -2 can't reach the goal.
*/
global int AStarNextPathElement(Unit* unit,int* pxd,int *pyd)
{
    // FIXME: Cache for often used pathes, like peons to goldmine.
    AStarFindPath(unit,pxd,pyd);

    switch( NewPath(unit,pxd,pyd) ) {
	case 0:
	    return 999;
	case 1:
	    return -1;
	default:
	    return -2;
    }
}

/**
**	Returns the next element of a path.
**
**	@param unit	Unit that wants the path element. 
**	@param pxd	Pointer for the x direction.
**	@param pyd	Pointer for the y direction.
**
**	@returns	>0 remaining path length, 0 wait for path, -1
**			reached goal, -2 can't reach the goal.
*/
global int NextPathElement(Unit* unit,int* pxd,int *pyd)
{
    // Convert old version to new version
    if( 1 || unit!=Selected[0] ) {
	switch( NewPath(unit,pxd,pyd) ) {
	    case 0:
		return 999;
	    case 1:
		return -1;
	    default:
		return -2;
	}
    }

    DebugLevel0(__FUNCTION__": %Zd#%s\n",UnitNumber(unit),unit->Type->Ident);

    return AStarNextPathElement(unit,pxd,pyd);
}

//@}
