//       _________ __                 __                               
//      /   _____//  |_____________ _/  |______     ____  __ __  ______
//      \_____  \\   __\_  __ \__  \\   __\__  \   / ___\|  |  \/  ___/
//      /        \|  |  |  | \// __ \|  |  / __ \_/ /_/  >  |  /\___ |
//     /_______  /|__|  |__|  (____  /__| (____  /\___  /|____//____  >
//             \/                  \/          \//_____/            \/ 
//  ______________________                           ______________________
//			  T H E   W A R   B E G I N S
//	   Stratagus - A free fantasy real time strategy game engine
//
/**@name pathfinder.c	-	The path finder routines. */
//
//	I use breadth-first.
//
//	(c) Copyright 1998,2000-2003 by Lutz Sammer,Russell Smith
//
//      This program is free software; you can redistribute it and/or modify
//      it under the terms of the GNU General Public License as published by
//      the Free Software Foundation; version 2 dated June, 1991.
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
//	$Id$

//@{

/*----------------------------------------------------------------------------
--	Includes
----------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

#include "stratagus.h"
#include "video.h"
#include "tileset.h"
#include "map.h"
#include "sound_id.h"
#include "unitsound.h"
#include "unittype.h"
#include "player.h"
#include "unit.h"
#include "pathfinder.h"
#include "missile.h"
#include "ui.h"

#ifndef MAX_PATH_LENGTH
#define MAX_PATH_LENGTH		9	/// Maximal path part returned.
#endif

#define USE_BEST			/// Goto best point, don't stop.

/*----------------------------------------------------------------------------
--	Variables
----------------------------------------------------------------------------*/

/**
**	The matrix is used to generated the paths.
**
**		0:	Nothing must check if usable.
**		1-8:	Field on the path 1->2->3->4->5...
**		88:	Marks the possible goal fields.
**		98:	Marks map border, for faster limits checks.
*/
global unsigned char Matrix[(MaxMapWidth+2)*(MaxMapHeight+3)+2];	/// Path matrix
local unsigned int LocalMatrix[MaxMapWidth*MaxMapHeight];

IfDebug(
global unsigned PfCounterFail;
global unsigned PfCounterOk;
global unsigned PfCounterDepth;
global unsigned PfCounterNotReachable;
);

/*----------------------------------------------------------------------------
--	Functions
----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
--	PATH-FINDER LOW-LEVEL
----------------------------------------------------------------------------*/

/**
**	Initialize a matrix
**
**	@note	Double border for ships/flyers.
**
**		98 98 98 98 98
**		98 98 98 98 98
**		98	    98
**		98	    98
**		98 98 98 98 98
*/
local void InitMatrix(unsigned char* matrix)
{
    unsigned i;
    unsigned w;
    unsigned h;
    unsigned e;

    w=TheMap.Width+2;
    h=TheMap.Height;

    i=w+w+1;
    memset(matrix,98,i);                // +1 for ships!
    memset(matrix+i,0,w*h);             // initialize matrix

    for( e=i+w*h; i<e; ) {              // mark left and right border
	matrix[i]=98;
	i+=w;
	matrix[i-1]=98;
    }
    memset(matrix+i,98,w+1);            // +1 for ships!
}
									
local void InitLocalMatrix(void)
{
    memset(LocalMatrix,0,TheMap.Width*TheMap.Height*sizeof(int));		// initialize matrix
}

/**
**	Create empty movement matrix.
*/
global unsigned char* CreateMatrix(void)
{
    InitMatrix(Matrix);
    return Matrix;
}

/**
**	Allocate a new matrix and initialize
*/
global unsigned char* MakeMatrix(void)
{
    unsigned char* matrix;

    matrix=malloc((TheMap.Width+2)*(TheMap.Height+3)+2);
    InitMatrix(matrix);

    return matrix;
}

/**
**	Mark place in matrix.
**
**	@param x1	X1 position of target area
**	@param y1	Y1 position of target area
**	@param x2	X2 position of target area, range is [x1,x2]
**	@param y2	Y2 position of target area, range is [y1,y2]
**	@param matrix	Target area marked in matrix
**
**	@returns	depth, -1 unreachable
*/
local int CheckPlaceInMatrix(int x1,int y1,int x2,int y2,unsigned int* matrix)
{
    int x;
    int y;

    for( y=y1; y<=y2; y++) {
	if( matrix[y*TheMap.Width+x1] ) {
	    return matrix[y*TheMap.Width+x1];
	}
	if( matrix[y*TheMap.Width+x2] ) {
	    return matrix[y*TheMap.Width+x2];
	}
    }
    for( x=x1; x<=x2; ++x ) {
	if( matrix[y1*TheMap.Width+x] ) {
	    return matrix[y1*TheMap.Width+x];
	}
	if( matrix[y2*TheMap.Width+x] ) {
	    return matrix[y2*TheMap.Width+x];
	}
    }
    return -1;
}

/**
**	Make goal in matrix.
**	'range' is how near we must reach.
**
**	       rrrr
**	       rggr
**	       rggr
**	       rrrr
**
**	@param unit	Goal to reach.
**	@param range	How near we must come.
**	@param matrix	Target goal and area around is marked in matrix
*/
local int CheckGoalInMatrix(const Unit* unit,int range,unsigned int* matrix)
{
    int x;
    int y;
    int w;
    int h;
    UnitType* type;

    x=unit->X-range;
    y=unit->Y-range;
    type=unit->Type;
    w=type->TileWidth+range*2;
    h=type->TileHeight+range*2;

    return CheckPlaceInMatrix(x,y,w,h,matrix);
}

/**
**	Flood fill an area for a matrix.
**
**	This use the flood-fill algorithms.
**	@todo can be done faster, if starting from both sides.
**
**	@param unit	Path for this unit.
**	@param matrix	Matrix for calculation.
**
*/
local void FillMatrix(Unit* unit,unsigned int* matrix)
{
    struct {
	unsigned short X;
	unsigned short Y;
	int depth;
    } *points;
    int x;
    int y;
    int rx;
    int ry;
    int mask;
    int wp;
    int rp;
    int ep;
    int n;
    int j;
    int depth;
    int size;
    unsigned int* m;

    size=4*(TheMap.Width+TheMap.Height)*sizeof(*points);
    points=malloc(size);
    size=4*(TheMap.Width+TheMap.Height);

    mask=UnitMovementMask(unit);
    // Ignore all possible mobile units.
    // FIXME: bad? mask&=~(MapFieldLandUnit|MapFieldAirUnit|MapFieldSeaUnit);

    points[0].X=x=unit->X;
    points[0].Y=y=unit->Y;
    points[0].depth=1;
    rp=0;
    matrix[x+y*TheMap.Width]=depth=1;			// mark start point
    ep=wp=1;					// start with one point
    n=2;

    //
    //	Pop a point from stack, push all neightbors which could be entered.
    //
    for( ;; ) {
	while( rp!=ep ) {
	    rx=points[rp].X;
	    ry=points[rp].Y;
	    depth=points[rp].depth;
	    for( j=0; j<8; ++j ) {		// mark all neighbors
		x=rx+Heading2X[j];
		y=ry+Heading2Y[j];
		if( x < 0 || y<0 || x >= TheMap.Width || y >= TheMap.Height) {	// already checked
		    // We Have been here before
		    continue;
		}
		m=matrix+x+y*TheMap.Width;
		if( *m ) {
		    continue;
		}
		if( CanMoveToMask(x,y,mask) ) {	// reachable
		    *m=depth+1;
		    points[wp].X=x;		// push the point
		    points[wp].Y=y;
		    points[wp].depth=depth+1;
		    if( ++wp>=size ) {		// round about
			wp=0;
		    }
		} else {			// unreachable
		    *m=0;
		}
	    }

	    // Loop for 
	    if( ++rp>=size ) {			// round about
		rp=0;
	    }
	}

	//
	//	Continue with next frame.
	//
	if( rp==wp ) {			// unreachable, no more points available
	    break;
	}
	ep=wp;
    }

    free(points);
    return;
}

/*----------------------------------------------------------------------------
--	PATH-FINDER USE
----------------------------------------------------------------------------*/

/**
**	Can the unit 'src' reach the place x,y.
**
**	@param src	Unit for the path.
**	@param x	Map X tile position.
**	@param y	Map Y tile position.
**	@param range	Range to the tile.
**
**	@return		Distance to place.
*/
global int PlaceReachable(Unit* src,int x,int y,int range)
{
    int depth;
    int x1;
    int x2;
    int y1;
    int y2;
    int reachable;
    static unsigned long LastGameCycle;
    static unsigned mask;

    DebugLevel3Fn("%p -> %d,%d\n" _C_ src _C_ x _C_ y);

    //
    //	Find a path to the place.
    //
    x1 = x;
    if( x1 < 0 ) {
	x1 = 0;
    }
    x2 = x + range;
    if( x2 > TheMap.Width ) {
	x2 = TheMap.Width;
    }
    y1 = y;
    if( y1 < 0 ) {
	y1 = 0;
    }
    y2 = y+range;
    if( y2 > TheMap.Height ) {
    	y2 = TheMap.Height;
    }
    // Find a reachable target, otherwise, don't search
    reachable=0;
    for( x=x1;x<=x2;x++ ) {
	if( CheckedCanMoveToMask(x,y1,UnitMovementMask(src)) ||
	    CheckedCanMoveToMask(x,y2,UnitMovementMask(src)) ) {
	    reachable=1;
	}
    }
    for( y=y1;y<y2;y++ ) {
	if( CheckedCanMoveToMask(x1,y,UnitMovementMask(src)) ||
	    CheckedCanMoveToMask(x2,y,UnitMovementMask(src)) ) {
	    reachable=1;
	}
    }

    if( !reachable ) {
	DebugLevel1("Can't move to destination, goal unreachable\n");
	return 0;
    }

    //
    //  Setup movement.
    //
    if( src->Type->MovementMask != mask || LastGameCycle != GameCycle 
	|| LocalMatrix[src->X+src->Y*TheMap.Width] == 0 ) {
	InitLocalMatrix();
	FillMatrix(src,LocalMatrix);
	LastGameCycle = GameCycle;
	mask = src->Type->MovementMask;
    }

    //
    //  Find a path to the place.
    //
    if( (depth=CheckPlaceInMatrix(x1,y1,x2,y2,LocalMatrix)) < 0 ) {
	DebugLevel1("Can't move to destination, not route to goal\n");
	return 0;
    }

    return depth;
								
}

/**
**	Can the unit 'src' reach the unit 'dst'.
**
**	@param src	Unit for the path.
**	@param dst	Unit to be reached.
**	@param range	Range to unit.
**
**	@return		Distance to place.
*/
global int UnitReachable(Unit* src,const Unit* dst,int range)
{
    int depth;
    int realrange;

    DebugLevel3Fn("%d(%d,%d,%s)->%d(%d,%d,%s)+%d "
	_C_ UnitNumber(src) _C_ src->X _C_ src->Y _C_ src->Type->Ident
	_C_ UnitNumber(dst) _C_ dst->X _C_ dst->Y _C_ dst->Type->Ident _C_ range);

    //
    //	Find a path to the goal.
    //
    realrange = range + range + max(dst->Type->TileHeight,dst->Type->TileWidth);
    depth=PlaceReachable(src,dst->X-range,dst->Y-range,realrange);
    if( depth <= 0 ) {
	DebugLevel3("NO WAY\n");
	return 0;
    }
    DebugLevel3("OK\n");

    return depth;
}

/*----------------------------------------------------------------------------
--	REAL PATH-FINDER
----------------------------------------------------------------------------*/

/**
**	Fast search new path.
**
**	@param unit	Path for this unit.
**	@param x1
**	@param y1
**	@param x2
**	@param y2
**
**      @return		>0 remaining path length, 0 wait for path, -1
**			reached goal, -2 can't reach the goal.
*/
local int FastNewPath(Unit* unit,int x1,int y1,int x2,int y2)
{
    int x;
    int y;
    int xd;
    int yd;
    const UnitType* type;
    int mask;
    int steps;
	
    //
    //	Fast check if the way is blocked in sight.
    //
    type=unit->Type;
    mask=TypeMovementMask(type);
    steps=unit->Stats->SightRange;	// only in sight range!

    x=unit->X;
    y=unit->Y;
    xd=(x1+x2)/2;				// Center of goal
    yd=(y1+y2)/2;


    // FIXME: johns: has anybody an idea how to write this faster?
    xd-=x;
    yd-=y;
    if( xd<0 ) xd=-1; else if( xd>0 ) xd=1;
    if( yd<0 ) yd=-1; else if( yd>0 ) yd=1;
    unit->Data.Move.Path[0]=XY2Heading[xd+1][yd+1];
    unit->Data.Move.Length=1;

    while( (steps-=1)>0 ) {
	x+=xd;
	y+=yd;

	DebugLevel3Fn("Unit %d,%d Goal %d,%d - %d,%d\n"
		_C_ x _C_ y _C_ x1 _C_ y1 _C_ x2 _C_ y2);
	DebugLevel3Fn("Check %d,%d=%x\n" _C_ x _C_ y _C_ mask);

	//
	//	Now check if we can move to this field
	//	FIXME: moving units didn't block?
	//
	if( !CheckedCanMoveToMask(x,y,mask) ) {	// blocked
	    DebugLevel3Fn("The way is blocked in sight range\n");
	    return PF_UNREACHABLE;
	}

	//
	//	Check if goal is reached
	//
	if( x>=x1 && x<=x2 && y>=y1 && y<=y2 ) {
	    DebugLevel3Fn("Goal in sight\n");
	    return unit->Stats->SightRange-steps;
	}

	//
	//	Try next field.
	//	From now on ignore any units.
	//
	mask&=~(MapFieldLandUnit|MapFieldAirUnit|MapFieldSeaUnit);

	xd=(x1+x2)/2;			// Center of goal
	yd=(y1+y2)/2;

	xd-=x;
	if( xd<0 ) xd=-1; else if( xd>0 ) xd=1;
	yd-=y;
	if( yd<0 ) yd=-1; else if( yd>0 ) yd=1;
    }
    DebugLevel3Fn("Nothing in sight range\n");

    return INT_MAX;
}

/**
**	Trace path in the matrix back to origin.
**
**	@param matrix	Matrix (not real start!)
**	@param add	Step value.
**	@param x	X start point of trace back.
**	@param y	Y start point of trace back.
**	@param depth	Depth of path and marks start value (1..8)
**	@param path	OUT: here are the path directions stored.
*/
local void PathTraceBack(const unsigned char* matrix,int add,int x,int y
	,int depth,unsigned char* path)
{
    int w;
    int w2;
    const unsigned char* m;
    int d;
    int n;
    int startdepth;

    w=TheMap.Width+2;
    m=matrix+x+y*w;			// End of path in matrix.
    w*=add;
    w2=w+w;
    startdepth=depth;
    if( startdepth > MAX_PATH_LENGTH ) {
	startdepth = MAX_PATH_LENGTH;
    }

    //
    //	Find the way back, nodes are numbered ascending.
    //
    for( ;; ) {
	--depth;
	n=(depth&0x7)+1;

	//
	//	Directions stored in path:
	//		7 0 1
	//		6 . 2
	//		5 4 3
	//
	d=6;
	m+=add;				// +1, 0
	if( *m!=n ) {			// test all neighbors
	  d=2;
	  m-=add<<1;			// -1, 0
	  if( *m!=n ) {
	    d=0;
	    m+=add+w;			//  0,+1
	    if( *m!=n ) {
	      d=4;
	      m-=w2;			//  0,-1
	      if( *m!=n ) {
		d=5;
		m+=add;			// +1,-1
		if( *m!=n ) {
		  d=3;
		  m-=add<<1;		// -1,-1
		  if( *m!=n ) {
		    d=1;
		    m+=w2;		// -1,+1
		    if( *m!=n ) {
		      d=7;
		      m+=add<<1;	// +1,+1
		      if( *m!=n ) {
			  return;
		      }
		    }
		  }
		}
	      }
	    }
	  }
	}
	// found: continue
	// Mart path if it's needed
	// 
	if( depth<MAX_PATH_LENGTH && path != NULL) {
	    path[startdepth-depth-1]=d;
	}
    }
}

/**
**	Complex search new path.
**
**	@param unit	Path for this unit.
**	@param x1	
**	@param y1	
**	@param x2	
**	@param y2	
**
**	@param path	OUT: part of the path.
**
**      @return		>0 remaining path length, 0 wait for path, -1
**			reached goal, -2 can't reach the goal.
*/
local int ComplexNewPath(Unit* unit,int x1,int y1,int x2,int y2,char* path)
{
    unsigned char* matrix;
    unsigned char* m;
    unsigned short* points;
    const Unit* goal;
#ifdef USE_BEST
    int bestx;
    int besty;
    int bestd;
    int bestn;
#endif
    int x;
    int y;
    int xd;
    int yd;
    int mask;
    int wp;
    int rp;
    int ep;
    int n;
    int i;
    int j;
    int w;
    int depth;
    int add;
    int unreachable;
    int size;

    DebugLevel3Fn("%s(%d) to %d=%d,%d+%d+%d\n"
	    _C_ unit->Type->Ident _C_ UnitNumber(unit)
	    _C_ unit->Orders[0].Goal ? UnitNumber(unit->Orders[0].Goal) : 0
	    _C_ x1 _C_ y1 _C_ x2 _C_ y2);

    size=TheMap.Width*TheMap.Height;
    points=malloc(size);
    size/=sizeof(*points)*2;

    w=TheMap.Width+2;
    matrix=CreateMatrix();
    matrix+=w+w+2;
    mask=UnitMovementMask(unit);

    points[0]=x=unit->X;
    points[1]=y=unit->Y;
    rp=0;
    matrix[x+y*w]=depth=1;			// mark start point
    ep=wp=n=2;					// start with one point

#ifdef USE_BEST
    bestx=x;
    besty=y;
    xd=abs(x1+x2)/2;
    yd=abs(y1+y2)/2;
    bestd=xd>yd ? xd : yd;
    bestn=0;					// needed if goal not reachable
#endif

    //
    //	Mark goal
    //
    unreachable=1;
    // FIXME: we need only to mark the rectangle border!!
    for( x=x1; x<=x2; ++x ) {
        if( x > TheMap.Width ) {
	    continue;
	}
	for( y=y1; y<=y2; ++y ) {
	    if( y > TheMap.Height ) {
	        continue;
	    }
	    if( CanMoveToMask(x,y,mask) ) {	// reachable
		matrix[x+y*w]=88;
		unreachable=0;
	    } else {
		matrix[x+y*w]=99;
	    }
	}
    }

    // No point of the target area could be entered
    IfDebug(
	if( unreachable ) {
	    PfCounterNotReachable++;
	    MakeLocalMissile(MissileTypeGreenCross,
		unit->X*TileSizeX+TileSizeX/2,unit->Y*TileSizeY+TileSizeY/2,
		unit->X*TileSizeX+TileSizeX/2,unit->Y*TileSizeY+TileSizeY/2);
	}
    );

#ifdef NEW_SHIPS
    if( unit->Type->UnitType==UnitTypeLand ) {
	add=0;
    } else {
	add=8;
    }
#else
    add=0;
#endif

    //gx=(x1+x2)/2;
    //gy=(y1+y2)/2;

    //
    //	Push and pop points until reached.
    //
    for( ;; ) {
	while( rp!=ep ) {
	    for( j=0; j<8; ++j ) {		// mark all neighbors
		x=points[rp]  +Heading2X[j+add];
		y=points[rp+1]+Heading2Y[j+add];
		m=matrix+x+y*w;
		if( *m ) {			// already checked/goal
		    if( *m==88 ) {		// Check if goal reached.
			PathTraceBack(matrix,(add>>3)+1,x,y,depth,path);
			IfDebug(
			    PfCounterOk++;
			    PfCounterDepth+=depth;
			    PfCounterDepth/=2;
			);
			free(points);
			return depth;
		    }
		    continue;
		}

		//
		//	Look if destination field is blocked.
		//	Fields blocked by moving units, are considered free.
		//
		i=TheMap.Fields[x+y*TheMap.Width].Flags&mask;
		if( i ) {		// unreachable
		    // Blocked by non-moving in not first round
		    DebugLevel3("%x - %x\n" _C_ mask _C_ i);
		    if( depth==1 || (i&~(MapFieldLandUnit
				|MapFieldAirUnit|MapFieldSeaUnit)) ) {
			DebugLevel3("NO: %d,%d\n" _C_ x _C_ y);
			*m=99;
			continue;
		    }
		    goal=UnitCacheOnXY(x,y,unit->Type->UnitType);
		    if( !goal ) {	// Should not happen.
			DebugLevel3Fn("%d %s: No goal for %d,%d on %d,%d?\n" _C_
				UnitNumber(unit) _C_ unit->Type->Ident _C_
				unit->X _C_ unit->Y _C_ x _C_ y);
			*m=99;
			DebugCheck( 1 );
			continue;
		    }
		    if( !goal->Moving ) {
			*m=99;
			continue;
		    }
		}

		//
		//	Reachable: push point on stack.
		//
		*m=n;
		points[wp]=x;
		points[wp+1]=y;
		wp+=2;
		if( wp>=size ) {	// round about
		    wp=0;
		}

#if 0
		//
		//	Save nearest point to target.
		//
		//xd=abs(gx-x);
		//yd=abs(gy-y);
		DebugLevel3("Best: %d,%d-%d - %d,%d\n" _C_ bestx _C_ besty _C_ bestd _C_ xd _C_ yd);
		if( xd>yd && xd<bestd ) {
		    bestd=xd;
		    bestx=x;
		    besty=y;
		    bestn=depth;
		}
		if( yd>=xd && yd<bestd ) {
		    bestd=yd;
		    bestx=x;
		    besty=y;
		    bestn=depth;
		}
#endif
	    }

	    rp+=2;
	    if( rp>=size ) {	// round about
		rp=0;
	    }
	}
	DebugLevel3("%d,%d\n" _C_ rp _C_ wp);

	//
	//	Continue with next frame.
	//
	if( rp==wp ) {			// unreachable, no points available
	    break;
	}
	ep=wp;

#if 0
	//	FIXME: if we ignore distant units we run left-right
	//	Ignore any units more than 5 fields from start.
	if( depth>6 ) {
	    mask&=~(MapFieldLandUnit|MapFieldAirUnit|MapFieldSeaUnit);
	}
#endif

	++depth;
	n=(depth&0x07)+1;
    }

    //
    //	We now move to the best reachable point.
    //
    DebugLevel3Fn("Unreachable Best %d,%d -> %d=%d,%d\n" _C_
	  unit->X _C_ unit->Y _C_ bestn _C_ bestx _C_ besty);

    IfDebug(
	PfCounterFail++;
	PfCounterDepth+=depth;
	PfCounterDepth/=2;
    );

#ifdef USE_BEST
    if( bestn ) {		// can move
#if 0
	//
	//	This reduces the amount of unreachables.
	//	FIXME: should this be handled by the caller????
	//
	if( unreachable && !unit->Orders[0].Goal ) {
	    unit->Orders[0].X=bestx;
	    unit->Orders[0].Y=besty;
	}
#endif
	PathTraceBack(matrix,(add>>3)+1,bestx,besty,bestn,path);
	free(points);
	return INT_MAX;
    }
#endif
	
    free(points);
    return PF_UNREACHABLE;
}

/**
**	Find new path.
**
**	The destination could be an unit or a field.
**	Range gives how far we must reach the goal.
**
**	@note	The destination could become negative coordinates!
**
**	@param unit	Path for this unit.
**
**      @return		>0 remaining path length, 0 wait for path, -1
**			reached goal, -2 can't reach the goal.
*/
global int NewPath(Unit* unit)
{
    int x;
    int y;
    const Unit* goal;
    const UnitType* type;
    int x1;
    int y1;
    int x2;
    int y2;
    int rx;
    int ry;
    int i;
    int reachable;
    char* path;

    x=unit->X;
    y=unit->Y;
    goal=unit->Orders[0].Goal;
    x1=unit->Orders[0].X;
    y1=unit->Orders[0].Y;
    rx=unit->Orders[0].RangeX;
    ry=unit->Orders[0].RangeY;
    unit->Data.Move.Length=0;

#if 0
    DebugLevel1Fn("%d: -> %s %p | %dx%d-%dx%d\n"
	_C_ UnitNumber(unit) _C_ unit->Data.Move.Fast ? "F" : "C"
	_C_ goal _C_ x1 _C_ y1 _C_ x2 _C_ y2);
#endif
    //
    //	Check if goal is already reached.
    //
    if( goal ) {			// goal unit
	// Increase range, so can move to edge of building
	type=goal->Type;
	DebugLevel3Fn("Goal: %s, Me: %s (range %d)\n" _C_ type->Ident _C_ unit->Type->Ident _C_ rx);
	x1=goal->X-rx;
	y1=goal->Y-ry;
	x2=x1+rx+type->TileWidth;
	y2=y1+ry+type->TileHeight;
	DebugLevel3Fn("Unit %d,%d Goal %d,%d - %d,%d\n"
		_C_ x _C_ y _C_ x1 _C_ y1 _C_ x2 _C_ y2);
	if( x>=x1 && x<=x2 && y>=y1 && y<=y2 ) {
	    DebugLevel3Fn("Goal reached\n");
	    return PF_REACHED;
	}
    } else {				// goal map field
	x1=unit->Orders[0].X-rx;
	x2=unit->Orders[0].X+rx;
	y1=unit->Orders[0].Y-ry;
	y2=unit->Orders[0].Y+ry;
	DebugLevel3Fn("Location: Unit %d,%d (%d,%d,%d) Goal %d,%d - %d,%d\n"
		_C_ x _C_ y _C_ unit->Orders[0].X _C_ unit->Orders[0].Y _C_ rx _C_ x1 _C_ y1 _C_ x2 _C_ y2);
	if( x>=x1 && x<=x2 && y>=y1 && y<=y2 ) {
	    DebugLevel3Fn("Field reached\n");
	    return PF_REACHED;
	}
	// This reduces the processor use,
	// If target isn't reachable and were are beside it
	// FIXME: should be +2? for ships/flyers?
#if 0
	if( x>=x1-1 && x<=x2+1 && y>=y1-1 && y<=y2+1 ) {
	    if( !CheckedCanMoveToMask(x,y,UnitMovementMask(unit)) ) {
		// target field blocked by something
		DebugLevel1Fn("Unreachable\n");
		return PF_UNREACHABLE;
	    }
	    unit->Data.Move.Fast=1;	// this could be handled fast
	}
#endif
    }

    // Ensure the pathfinder gets something that's on the map
    if( x1 < 0 ) x1 = 0;
    if( x2 >= TheMap.Width ) x2 = TheMap.Width-1;
    if( y1 < 0 ) y1 = 0;
    if( y2 >= TheMap.Height ) y2 = TheMap.Height-1;
    // Find a reachable target
    reachable=0;
    for( x=x1;x<=x2;x++ ) {
	if( CheckedCanMoveToMask(x,y1,UnitMovementMask(unit)) ||
	    CheckedCanMoveToMask(x,y2,UnitMovementMask(unit)) ) {
	    reachable=1;
	}
    }
    for( y=y1;y<y2;y++ ) {
	if( CheckedCanMoveToMask(x1,y,UnitMovementMask(unit)) ||
	    CheckedCanMoveToMask(x2,y,UnitMovementMask(unit)) ) {
	    reachable=1;
	}
    }
    if( !reachable ) {
	return PF_UNREACHABLE;
    }
    x=unit->X;
    y=unit->Y;
    if( x>=x1 && x<=x2 && y>=y1 && y<=y2 ) {
	DebugLevel3Fn("Field reached\n");
	return PF_REACHED;
    }

    //
    //	If possible, try fast.
    //
    if( unit->Data.Move.Fast ) {
	if( (i=FastNewPath(unit,x1,y1,x2,y2))>=-1 ) {
	    // Fast works
	    // DebugCheck( *xdp==0 && *ydp==0 );
	    return i;
	}
	DebugLevel3Fn("Fallback to slow method\n");
	unit->Data.Move.Fast=0;
    }

    //
    //	Fall back to slow complex method.
    //
    path = unit->Data.Move.Path;
    if( AStarOn ) {
	i=AStarFindPath(unit,x1,y1,x2,y2,path);
	if( i == PF_FAILED ) {
	    i = PF_UNREACHABLE;
	}
    } else {
	i=ComplexNewPath(unit,x1,y1,x2,y2,path);
    }

    // Update path if it was requested. Otherwise we may only want
    // to know if there exists a path.

    if( path != NULL ) {
	if( i >= MAX_PATH_LENGTH ) {
	    unit->Data.Move.Length=MAX_PATH_LENGTH;
	} else {
	    unit->Data.Move.Length=i;
	}
	if( unit->Data.Move.Length == 0) {
	    unit->Data.Move.Length++;
	}
    }
    return i;
}

//@}
