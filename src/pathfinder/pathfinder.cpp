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
/**@name pathfinder.c	-	The path finder routines. */
//
//	I use breadth-first.
//
//	(c) Copyright 1998,2000,2001 by Lutz Sammer
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

#include "freecraft.h"
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
global unsigned char Matrix[(MaxMapWidth+2)*(MaxMapHeight+2)];	/// Path matrix

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
**	Create empty movement matrix.
**
**	NOTE: double border for ships/flyers.
**
**		98 98 98 98 98
**		98 98 98 98 98
**		98	    98
**		98	    98
**		98 98 98 98 98
*/
global unsigned char* CreateMatrix(void)
{
    unsigned char* matrix;
    unsigned i;
    unsigned w;
    unsigned h;
    unsigned e;

    w=TheMap.Width+2;
    h=TheMap.Height;
    matrix=Matrix;
    //matrix=malloc(w*(h+2));

    i=w+w+1;
    memset(matrix,98,i);		// +1 for ships!
    memset(matrix+i,0,w*h);		// initialize matrix

    for( e=i+w*h; i<e; ) {		// mark left and right border
	matrix[i]=98;
	i+=w;
	matrix[i-1]=98;
    }
    memset(matrix+i,98,w+1);		// +1 for ships!

    return matrix;
}

/**
**	Mark place in matrix.
**
**	@param x	X position of target area
**	@param y	Y position of target area
**	@param w	Width of target area
**	@param h	Height of target area
**	@param matrix	Target area marked in matrix
*/
local void MarkPlaceInMatrix(int x,int y,int w,int h,unsigned char* matrix)
{
    int xi;
    int xe;
    int mw;

    if( x<0 ) {				// reduce to map limits
	w-=x;
	x=0;
    }
    if( x+w>TheMap.Width ) {
	w=TheMap.Width-x;
    }
    if( y<0 ) {
	h-=y;
	y=0;
    }
    if( y+h>TheMap.Height ) {
	h=TheMap.Height-y;
    }

    DebugCheck( h==0 || w==0 );		// atleast one tile should be there!

    mw=TheMap.Width+2;
    matrix+=mw+mw+2;
    xe=x+w;
    while( h-- ) {			// mark the rectangle
	for( xi=x; xi<xe; ++xi ) {
	    matrix[xi+y*mw]=88;
	}
	++y;
    }
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
local void MarkGoalInMatrix(const Unit* unit,int range,unsigned char* matrix)
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

    MarkPlaceInMatrix(x,y,w,h,matrix);
}

/**
**	Mark path to goal.
**
**	This use the flood-fill algorithms.
**	@todo can be done faster, if starting from both sides.
**
**	@param unit	Path for this unit.
**	@param matrix	Matrix for calculation.
**
**	@return		1 reachable, -1 unreachable
*/
local int MarkPathInMatrix(const Unit* unit,unsigned char* matrix)
{
    static const int xoffset[]={  0,-1,+1, 0, -1,+1,-1,+1,
	     0,-2,+2, 0, -2,+2,-2,+2 };
    static const int yoffset[]={ -1, 0, 0,+1, -1,-1,+1,+1,
	    -2, 0, 0,+2, -2,-2,+2,+2 };
    struct {
	unsigned short X;
	unsigned short Y;
    } points[TheMap.Width*TheMap.Height];
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
    int w;
    int add;
    int depth;
    unsigned char* m;

    w=TheMap.Width+2;
    mask=UnitMovementMask(unit);
    // Ignore all possible mobile units.
    mask&=~(MapFieldLandUnit|MapFieldAirUnit|MapFieldSeaUnit);

#ifdef NEW_SHIPS
    if( unit->Type->UnitType==UnitTypeLand ) {
	add=0;
    } else {
	add=8;
    }
#else
    add=0;
#endif

    points[0].X=x=unit->X;
    points[0].Y=y=unit->Y;
    matrix+=w+w+2;
    rp=0;
    matrix[x+y*w]=depth=1;			// mark start point
    ep=wp=1;					// start with one point
    n=2;

    //
    //	Pop a point from stack, push all neightbors which could be entered.
    //
    for( ;; ) {
	while( rp!=ep ) {
	    rx=points[rp].X;
	    ry=points[rp].Y;
	    for( j=0; j<8; ++j ) {		// mark all neighbors
		x=rx+xoffset[j+add];
		y=ry+yoffset[j+add];
		m=matrix+x+y*w;
		if( *m ) {			// already checked
		    // Check if goal reached.
		    if( *m==88 && CanMoveToMask(x,y,mask) ) {
			return depth;
		    }
		    continue;
		}
		if( CanMoveToMask(x,y,mask) ) {	// reachable
		    *m=n;
		    points[wp].X=x;		// push the point
		    points[wp].Y=y;
		    if( ++wp>=sizeof(points) ) {// round about
			wp=0;
		    }
		} else {			// unreachable
		    *m=99;
		}
	    }

	    if( ++rp>=sizeof(points) ) {	// round about
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
	++depth;
	n=(depth&0x07)+1;
    }

    return -1;
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
global int PlaceReachable(const Unit* src,int x,int y,int range)
{
    unsigned char* matrix;
    int depth;

    DebugLevel3Fn("%p -> %d,%d\n",src,x,y);

    //
    //	Setup movement.
    //
    matrix=CreateMatrix();
    MarkPlaceInMatrix(x,y,range,range,matrix);

    //
    //	Find a path to the place.
    //
    if( (depth=MarkPathInMatrix(src,matrix)) < 0 ) {
	DebugLevel3("Can't move to destination\n");
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
global int UnitReachable(const Unit* src,const Unit* dst,int range)
{
    unsigned char* matrix;
    int depth;

    DebugLevel3Fn("%d(%d,%d,%s)->%d(%d,%d,%s)+%d "
	,UnitNumber(src),src->X,src->Y,src->Type->Ident
	,UnitNumber(dst),dst->X,dst->Y,dst->Type->Ident,range);

    //
    //	Setup movement.
    //
    matrix=CreateMatrix();
    MarkGoalInMatrix(dst,range,matrix);

    //
    //	Find a path to the goal.
    //
    if( (depth=MarkPathInMatrix(src,matrix))<0 ) {
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
**	@param gx	Goal X position.
**	@param gy	Goal Y position.
**	@param ox	Offset in X.
**	@param oy	Offset in Y.
**
**	@param xdp	OUT: Pointer for x direction.
**	@param xdp	OUT: Pointer for y direction.
**
**      @return		>0 remaining path length, 0 wait for path, -1
**			reached goal, -2 can't reach the goal.
*/
local int FastNewPath(const Unit* unit,int gx,int gy,int ox,int oy
	,int *xdp,int* ydp)
{
    int x;
    int y;
    int xd;
    int yd;
    const UnitType* type;
    int mask;
    int steps;
    int add;

    //
    //	Fast check if the way is blocked in sight.
    //
    type=unit->Type;
    mask=TypeMovementMask(type);
    steps=unit->Stats->SightRange;	// only in sight range!

    x=unit->X;
    y=unit->Y;
    xd=gx+ox/2;				// Center of goal
    yd=gy+oy/2;

#ifdef NEW_SHIPS
    if( type->UnitType==UnitTypeLand ) {
	add=1;
    } else {
	add=2;
    }
#else
    add=1;
#endif

    // FIXME: johns: has anybody an idea how to write this faster?
    xd-=x;
    if( xd<0 ) xd=-add; else if( xd>0 ) xd=add;
    *xdp=xd;
    yd-=y;
    if( yd<0 ) yd=-add; else if( yd>0 ) yd=add;
    *ydp=yd;
    DebugLevel3Fn("%d,%d\n",xd,yd);

    while( (steps-=add)>0 ) {
	x+=xd;
	y+=yd;

	DebugLevel3Fn("Unit %d,%d Goal %d,%d - %d,%d\n"
		,x,y,gx,gy,gx+ox,gy+oy);
	DebugLevel3Fn("Check %d,%d=%x\n",x,y,mask);

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
	if( x>=gx && x<gx+ox && y>=gy && y<gy+oy ) {
	    DebugLevel3Fn("Goal in sight\n");
	    return unit->Stats->SightRange-steps;
	}

	//
	//	Try next field.
	//	From now on ignore any units.
	//
	mask&=~(MapFieldLandUnit|MapFieldAirUnit|MapFieldSeaUnit);

	xd=gx+ox/2;			// Center of goal
	yd=gy+oy/2;

	xd-=x;
	if( xd<0 ) xd=-add; else if( xd>0 ) xd=add;
	yd-=y;
	if( yd<0 ) yd=-add; else if( yd>0 ) yd=add;
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

    w=TheMap.Width+2;
    m=matrix+x+y*w;			// End of path in matrix.
    w*=add;
    w2=w+w;

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
	if( depth<MAX_PATH_LENGTH ) {
	    path[depth]=d;
	}
    }
}

/**
**	Complex search new path.
**
**	@param unit	Path for this unit.
**	@param gx	Goal X position.
**	@param gy	Goal Y position.
**	@param ox	Offset in X.
**	@param oy	Offset in Y.
**
**	@param path	OUT: part of the path.
**
**      @return		>0 remaining path length, 0 wait for path, -1
**			reached goal, -2 can't reach the goal.
*/
local int ComplexNewPath(Unit* unit,int gx,int gy,int ox,int oy,char* path)
{
    static const int xoffset[]={  0,-1,+1, 0, -1,+1,-1,+1,
	     0,-2,+2, 0, -2,+2,-2,+2 };
    static const int yoffset[]={ -1, 0, 0,+1, -1,-1,+1,+1,
	    -2, 0, 0,+2, -2,-2,+2,+2 };
    unsigned char* matrix;
    unsigned char* m;
    unsigned short points[TheMap.Width*TheMap.Height];
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

    DebugLevel3Fn("%s(%d) to %d=%d,%d+%d+%d\n"
	    ,unit->Type->Ident,UnitNumber(unit)
	    ,unit->Command.Data.Move.Goal
		? UnitNumber(unit->Command.Data.Move.Goal) : 0
	    ,gx,gy,ox,oy);

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
    xd=abs(gx+ox/2-x);
    yd=abs(gy+oy/2-y);
    bestd=xd>yd ? xd : yd;
    bestn=0;					// needed if goal not reachable
#endif

    //
    //	Mark goal
    //
    xd=gx+ox;
    if( xd>TheMap.Width ) {			// limit to map
	xd=TheMap.Width;
    }
    yd=gy+oy;
    if( yd>TheMap.Height ) {
	yd=TheMap.Height;
    }

    unreachable=1;
    // FIXME: we need only to mark the rectangle border!!
    for( x=gx<0 ? 0 : gx; x<xd; ++x ) {
	for( y=gy<0 ? 0 : gy; y<yd; ++y ) {
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
	    MakeMissile(MissileTypeGreenCross,
		unit->X*TileSizeX+TileSizeX/2,
		unit->Y*TileSizeY+TileSizeY/2,0,0);
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

    gx+=ox/2;
    gy+=oy/2;

    //
    //	Push and pop points until reached.
    //
    for( ;; ) {
	while( rp!=ep ) {
	    for( j=0; j<8; ++j ) {		// mark all neighbors
		x=points[rp]  +xoffset[j+add];
		y=points[rp+1]+yoffset[j+add];
		m=matrix+x+y*w;
		if( *m ) {			// already checked/goal
		    if( *m==88 ) {		// Check if goal reached.
			PathTraceBack(matrix,(add>>3)+1,x,y,depth,path);
			IfDebug(
			    PfCounterOk++;
			    PfCounterDepth+=depth;
			    PfCounterDepth/=2;
			);
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
		    DebugLevel3("%x - %x\n",mask,i);
		    if( depth==1 || (i&~(MapFieldLandUnit
				|MapFieldAirUnit|MapFieldSeaUnit)) ) {
			DebugLevel3("NO: %d,%d\n",x,y);
			*m=99;
			continue;
		    }
		    goal=UnitCacheOnXY(x,y,unit->Type->UnitType);
		    if( !goal ) {	// Should not happen.
			DebugLevel0Fn("%d %s: No goal for %d,%d on %d,%d?\n",
				UnitNumber(unit),unit->Type->Ident,
				unit->X,unit->Y,x,y);
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
		if( wp>=sizeof(points)/sizeof(*points) ) {	// round about
		    wp=0;
		}

#ifdef USE_BEST
		//
		//	Save nearest point to target.
		//
		xd=abs(gx-x);
		yd=abs(gy-y);
		DebugLevel3("Best: %d,%d-%d - %d,%d\n",bestx,besty,bestd,xd,yd);
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
	    if( rp>=sizeof(points)/sizeof(*points) ) {	// round about
		rp=0;
	    }
	}
	DebugLevel3("%d,%d\n",rp,wp);

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
	    mask&=~(MapMoveLandUnit|MapMoveAirUnit|MapMoveSeaUnit);
	}
#endif

	++depth;
	n=(depth&0x07)+1;
    }

    //
    //	We now move to the best reachable point.
    //
    DebugLevel3Fn("Unreachable Best %d,%d -> %d=%d,%d\n",
	  unit->X,unit->Y,bestn,bestx,besty);

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
	return INT_MAX;
    }
#endif

    return PF_UNREACHABLE;
}

/**
**	Find new path.
**
**	The destination could be an unit or a field.
**	Range gives how far we must reach the goal.
**
**	NOTE: the destination could become negative coordinates!
**
**	@param unit	Path for this unit.
**	@param xdp	Pointer for x direction return.
**	@param xdp	Pointer for y direction return.
**
**      @return		>0 remaining path length, 0 wait for path, -1
**			reached goal, -2 can't reach the goal.
*/
global int NewPath(Unit* unit,int* xdp,int* ydp)
{
    int x;
    int y;
    const Unit* goal;
    const UnitType* type;
    int gx;
    int gy;
    int rx;
    int ry;
    int i;
    unsigned char path[MAX_PATH_LENGTH];

    //
    //	Convert heading into direction.
    //				  //  N NE  E SE  S SW  W NW
    local const int Heading2X[8] = {  0,+1,+1,+1, 0,-1,-1,-1 };
    local const int Heading2Y[8] = { -1,-1, 0,+1,+1,+1, 0,-1 };


    x=unit->X;
    y=unit->Y;
    goal=unit->Orders[0].Goal;
    gx=unit->Orders[0].X;
    gy=unit->Orders[0].Y;
    rx=unit->Orders[0].RangeX;
    ry=unit->Orders[0].RangeY;

    DebugLevel3Fn("%d: -> %s %p | %dx%d+%d+%d\n"
	,UnitNumber(unit),unit->Command.Data.Move.Fast ? "F" : "C"
	,goal,gx,gy,rx,ry);

#ifdef DEBUG
    //
    //	Check if the path is correct reseted.
    //	A new path should always have the fast flag set.
    //
    if( unit->Goal!=goal || unit->GoalX!=gx || unit->GoalY!=gy ) {
	if( !unit->Data.Move.Fast ) {
	    DebugLevel0Fn("ResetPath missing\n");
	    abort();
	}
	unit->Goal=goal;
	unit->GoalX=gx;
	unit->GoalY=gy;
    }
#endif

    //
    //	Check if goal is already reached.
    //
    if( goal ) {			// goal unit
	type=goal->Type;
	gx=goal->X-rx;
	gy=goal->Y-ry;
	rx+=rx+type->TileWidth;
	ry+=ry+type->TileHeight;
	DebugLevel3Fn("Unit %d,%d Goal %d,%d - %d,%d\n"
		,x,y,gx,gy,gx+rx,gy+ry);
	if( x>=gx && x<gx+rx && y>=gy && y<gy+ry ) {
	    DebugLevel3Fn("Goal reached\n");
	    *xdp=*ydp=0;
	    return PF_REACHED;
	}
    } else {				// goal map field
	// FIXME: still some wired code, if moving to point this must +1
	// FIXME: should be +2? for ships/flyers?
	++rx;
	++ry;
	if( x>=gx && x<gx+rx && y>=gy && y<gy+ry ) {
	    DebugLevel3Fn("Field reached\n");
	    *xdp=*ydp=0;
	    return PF_REACHED;
	}
	// This reduces the processor use,
	// If target isn't reachable and were beside it
	if( !rx && !ry && x>=gx-1 && x<gx+1 && y>=gy-1 && y<gy+1 ) {
	    if( !CheckedCanMoveToMask(gx,gy,UnitMovementMask(unit)) ) {
		// target field blocked by something
		*xdp=*ydp=0;
		return PF_UNREACHABLE;
	    }
	    unit->Data.Move.Fast=1;	// this could be handled fast
	}
    }

    //
    //	If possible, try fast.
    //
    if( unit->Data.Move.Fast ) {
	if( (i=FastNewPath(unit,gx,gy,rx,ry,xdp,ydp))>=-1 ) {
	    // Fast works
	    DebugCheck( *xdp==0 && *ydp==0 );
	    return i;
	}
	DebugLevel3Fn("Fallback to slow method\n");
	unit->Data.Move.Fast=0;
    }

    //
    //	Fall back to slow complex method.
    //
    i=ComplexNewPath(unit,gx,gy,rx,ry,path);

    if( i>=0 ) {
#ifdef NEW_SHIPS
	if( unit->Type->UnitType==UnitTypeLand ) {
	    *xdp=Heading2X[*path];
	    *ydp=Heading2Y[*path];
	} else {
	    *xdp=Heading2X[*path]*2;
	    *ydp=Heading2Y[*path]*2;
	}
#else
	*xdp=Heading2X[*path];
	*ydp=Heading2Y[*path];
#endif
    }

    return i;
}

//@}
