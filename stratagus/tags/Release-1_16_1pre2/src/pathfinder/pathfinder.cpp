/*
**	A clone of a famous game.
*/
/**@name pathfinder.c	-	The path finder routines. */
/*
**	I use breadth-first.
**
**	(c) Copyright 1998 by Lutz Sammer
**
**	$Id$
*/

//@{

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "clone.h"
#include "video.h"
#include "tileset.h"
#include "map.h"
#include "sound_id.h"
#include "unitsound.h"
#include "unittype.h"
#include "player.h"
#include "unit.h"
#include "pathfinder.h"

#define FC_MAX_PATH	9		/// Maximal path part returned.

//
//	The matrix is used to generated the paths.
//
//		 0:	Nothing
//		1-8:	Field on the path 1->2->3->4->5...
//		88:	Marks the possible goal fields.
//		98:	Marks map border, for faster limits checks.
//
global unsigned char Matrix[(MaxMapWidth+1)*(MaxMapHeight+1)];	/// Path matrix

IfDebug(
global unsigned PfCounterFail;
global unsigned PfCounterOk;
global unsigned PfCounterDepth;
global unsigned PfCounterNotReachable;
);

/*----------------------------------------------------------------------------
--	PATH-FINDER LOW-LEVEL
----------------------------------------------------------------------------*/

/**
**	Create empty movement matrix.
**
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

    matrix=Matrix;
    w=TheMap.Width+2;
    h=TheMap.Height+2;

    memset(matrix,98,w);
    memset(matrix+w,0,(w*h)-w*2);	// initialize matrix

    e=w*TheMap.Height;
    for( i=w; i<=e; ) {			// mark left and right border
	matrix[i]=98;
	i+=w;
	matrix[i-1]=98;
    }
    memset(matrix+i,98,w);

    return matrix;
}

/**
**	Mark place in matrix.
**
**	@param x	X position
**	@param y	Y position
**	@param w	Width
**	@param h	Height
**	@param matrix	Matrix
*/
local void MarkPlaceInMatrix(int x,int y,int w,int h,unsigned char* matrix)
{
    int xi;
    int xe;
    int mw;

    if( x<0 ) {				// reduce to map limits
	w=x;
	x=0;
    }
    if( x+w>TheMap.Width ) {
	w=TheMap.Width-x;
    }
    if( y<0 ) {
	w=y;
	y=0;
    }
    if( y+h>TheMap.Height ) {
	h=TheMap.Height-y;
    }

    DebugCheck( h==0 || w==0 );

    mw=TheMap.Width+2;
    xe=x+w;
    while( h-- ) {			// mark the rectangle
	for( xi=x; xi<xe; ++xi ) {
	    matrix[mw+1+xi+y*mw]=88;
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
*/
local void MarkGoalInMatrix(Unit* unit,int range,unsigned char* matrix)
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
**	Trace path in the matrix back to origin.
**
**	FIXME: didn't need to check the goal field again!
**	FIXME: only the last 8 points are returned!
*/
local void PathTraceBack(const unsigned char* matrix,int x,int y,int n
	,int* path)
{
    int w;
    int w2;
    const unsigned char* m;

    DebugLevel3(__FUNCTION__": %d\n",n);

    w=TheMap.Width+2;
    w2=w+w;

    path[n*2+0]=x;
    path[n*2+1]=y;

    m=matrix+1+w+y*w;

    for( ;; ) {
	DebugLevel3("%d,%d(%d)\n",x,y,n);
	n=((n-1)&0x7)+1;

	x++;					// +1, 0
	if( m[x]!=n ) {			// test all neighbors
	  x-=2;					// -1, 0
	  if( m[x]!=n ) {
	    x++;
	    y++;				//  0,+1
	    m+=w;
	    if( m[x]!=n ) {
	      y-=2;				//  0,-1
	      m-=w2;
	      if( m[x]!=n ) {
		x++;				// +1,-1
		if( m[x]!=n ) {
		  x-=2;				// -1,-1
		  if( m[x]!=n ) {
		    y+=2;			// -1,+1
		    m+=w2;
		    if( m[x]!=n ) {
		      x+=2;			// +1,+1
			if( m[x]!=n ) {
			    DebugLevel3("Start reached\n");
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
	--n;
	path[n*2+0]=x;
	path[n*2+1]=y;
    }
}

/*
**	Mark path to goal.
*/
local int MarkPathInMatrix(Unit* unit,unsigned char* matrix)
{
    static int xoffset[]={  0,-1,+1, 0, -1,+1,-1,+1 };
    static int yoffset[]={ -1, 0, 0,+1, -1,-1,+1,+1 };
    struct {
	int X;
	int Y;
    } points[TheMap.Width*TheMap.Height];
    int x;
    int y;
    int mask;
    int wp;
    int rp;
    int ep;
    int n;
    int j;
    int w;

    w=TheMap.Width+2;
    mask=UnitMovementMask(unit);
    mask&=~(MapFieldLandUnit|MapFieldAirUnit|MapFieldSeaUnit);

    points[0].X=x=unit->X;
    points[0].Y=y=unit->Y;
    matrix[w+1+x+y*w]=1;			// mark start point
    n=rp=0;
    ep=1;
    wp=1;					// start with one point

    //
    //	Pop a point from stack, push all neightbors which could be entered.
    //
    for( ;; ) {
	++n;
	while( rp!=ep ) {
	    for( j=0; j<8; ++j ) {		// mark all neighbors
		x=points[rp].X+xoffset[j];
		y=points[rp].Y+yoffset[j];
		if( matrix[w+1+x+y*w] ) {	// already reached
		    //
		    //	Check if goal reached.
		    //
		    if( matrix[w+1+x+y*w]==88 ) {
			DebugLevel3("Goal reached ");
			return 1;
		    }
		    continue;
		}
		if( CanMoveToMask(x,y,mask) ) {	// reachable
		    DebugLevel3("OK: %d,%d\n",x,y);
		    matrix[w+1+x+y*w]=n+1;
		    points[wp].X=x;
		    points[wp].Y=y;
		    wp++;
		    if( wp>=sizeof(points) ) {	// round about
			wp=0;
		    }
		} else {			// unreachable
		    DebugLevel3("NO: %d,%d\n",x,y);
		    matrix[w+1+x+y*w]=99;
		}
	    }

	    rp++;
	    if( rp>=sizeof(points) ) {	// round about
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
	n&=7;
    }
    DebugLevel3(__FUNCTION__": Can't reach point\n");

    return -1;
}

/*----------------------------------------------------------------------------
--	PATH-FINDER USE
----------------------------------------------------------------------------*/

/*
**	Can the unit 'src' reach the place x,y.
*/
global int PlaceReachable(Unit* src,int x,int y)
{
    unsigned char* matrix;

    DebugLevel3(__FUNCTION__": %p -> %d,%d\n",src,x,y);

    //
    //	Setup movement.
    //
    matrix=CreateMatrix();
    MarkPlaceInMatrix(x,y,1,1,matrix);

    if( MarkPathInMatrix(src,matrix) < 0 ) {
	DebugLevel3("Can't move to destination\n");
	return 0;
    }

    return 1;
}

/*
**	Can the unit 'src' reach the unit 'dst'.
*/
global int UnitReachable(Unit* src,Unit* dst)
{
    unsigned char* matrix;

    DebugLevel3(__FUNCTION__": %Zd(%d,%d,%s)->%Zd(%d,%d,%s) "
	,UnitNumber(src),src->X,src->Y,src->Type->Ident
	,UnitNumber(dst),dst->X,dst->Y,dst->Type->Ident);

    //
    //	Setup movement.
    //
    matrix=CreateMatrix();
    MarkGoalInMatrix(dst,1,matrix);

    if( MarkPathInMatrix(src,matrix)<0 ) {
	DebugLevel3("NO WAY\n");
	return 0;
    }
    DebugLevel3("OK\n");

    return 1;
}

/*----------------------------------------------------------------------------
--	REAL PATH-FINDER
----------------------------------------------------------------------------*/

/**
**	Fast search new path.
**
**	@param unit	Path for this unit.
**
**	@param xdp	OUT: Pointer for x direction.
**	@param xdp	OUT: Pointer for y direction.
**	@return		True, if could move to goal. False otherwise.
*/
local int FastNewPath(const Unit* unit,int *xdp,int* ydp)
{
    int x;
    int y;
    int xd;
    int yd;
    const UnitType* type;
    int mask;
    int steps;
    const Unit* goal;
    int r;

    //
    //	1) Fast check if the way is blocked in sight.
    //
    type=unit->Type;
    mask=TypeMovementMask(type);
    steps=unit->Stats->SightRange;	// only in sight range!

    x=unit->X;
    y=unit->Y;
    goal=unit->Command.Data.Move.Goal;
    if( goal ) {
	type=goal->Type;
	xd=goal->X;
	yd=goal->Y;
    } else {
	xd=unit->Command.Data.Move.DX;
	yd=unit->Command.Data.Move.DY;
    }

    xd-=x;
    if( xd<0 ) xd=-1; else if( xd>0 ) xd=1;
    *xdp=xd;
    yd-=y;
    if( yd<0 ) yd=-1; else if( yd>0 ) yd=1;
    *ydp=yd;
    DebugLevel3(__FUNCTION__": %d,%d\n",xd,yd);

    r=unit->Command.Data.Move.Range;
    while( steps-- ) {
	x+=xd;
	y+=yd;
	DebugLevel3(__FUNCTION__": Check %d,%d=%x\n",x,y,mask);

	//
	//	Now check if we can move to this field
	//
	if( !CheckedCanMoveToMask(x,y,mask) ) {	// blocked
	    DebugLevel3(__FUNCTION__": The way is blocked in sight range\n");
	    return 0;
	}

	//
	//	Check if goal is reached
	//	FIXME: This could be improved
	//
	if( goal ) {
	    DebugLevel3(__FUNCTION__": Unit %d, %d Goal %d,%d - %d,%d\n"
		,x,y
		,goal->X-r,goal->Y-r
		,goal->X+type->TileWidth+r
		,goal->Y+type->TileHeight+r);
	    if( x>=goal->X-r && x<goal->X+type->TileWidth+r
		    && y>=goal->Y-r && y<goal->Y+type->TileHeight+r ) {
		DebugLevel3(_FUNCTION__": Goal in sight\n");
		return 1;
	    }
	    xd=goal->X;
	    yd=goal->Y;
	} else {
	    if( x>=unit->Command.Data.Move.DX
		    && x<=unit->Command.Data.Move.DX+r
		    && y>=unit->Command.Data.Move.DY
		    && y<=unit->Command.Data.Move.DY+r ) {
		DebugLevel3(__FUNCTION__": Field in sight\n");
		return 1;
	    }
	    xd=unit->Command.Data.Move.DX;
	    yd=unit->Command.Data.Move.DY;
	}

	//
	//	Try next field.
	//	From now on ignore any units.
	//
	mask&=~(MapFieldLandUnit|MapFieldAirUnit|MapFieldSeaUnit);
	xd-=x;
	if( xd<0 ) xd=-1; else if( xd>0 ) xd=1;
	yd-=y;
	if( yd<0 ) yd=-1; else if( yd>0 ) yd=1;
    }
    DebugLevel3(__FUNCTION__": Nothing in sight range\n");

    return 1;
}

/**
**	Complex search new path.
**
**	@param unit	Path for this unit.
**
**	@param xdp	OUT: Pointer for x direction.
**	@param xdp	OUT: Pointer for y direction.
**
**	@return		1=Reachable 0=Reached -1=Unreachable
*/
local int ComplexNewPath(Unit* unit,int *xdp,int* ydp)
{
    static int xoffset[]={  0,-1,+1, 0, -1,+1,-1,+1 };
    static int yoffset[]={ -1, 0, 0,+1, -1,-1,+1,+1 };
    unsigned char* matrix;
    unsigned char points[TheMap.Width*TheMap.Height];
    int path[FC_MAX_PATH*2];
    Unit* goal;
    int bestx;
    int besty;
    int bestd;
    int bestn;
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
    int r;
    int f;
    int depth;

    DebugLevel3(__FUNCTION__": %s(%Zd) to %Zd=%d,%d~%d\n"
	    ,unit ? unit->Type->Ident : ""
	    ,UnitNumber(unit)
	    ,unit->Command.Data.Move.Goal
		? unit->Command.Data.Move.UnitNumber(Goal) : 0
	    ,unit->Command.Data.Move.DX
	    ,unit->Command.Data.Move.DY
	    ,unit->Command.Data.Move.Range);

    matrix=CreateMatrix();
    w=TheMap.Width+2;
    mask=UnitMovementMask(unit);

#if 0
// If reachable it is better to start at goal
// We needn't trace back the route
    points[0]=unit->Command.Data.Move.DX;		// start at goal
    points[1]=unit->Command.Data.Move.DY;
#endif

    points[0]=x=unit->X;
    points[1]=y=unit->Y;
    matrix[w+1+x+y*w]=n=1;			// mark start point
    depth=1;
    rp=0;
    ep=2;
    wp=2;					// start with one point

    bestx=x;
    besty=y;
    xd=abs(unit->Command.Data.Move.DX-x);
    yd=abs(unit->Command.Data.Move.DY-y);
    bestd=xd>yd ? xd : yd;
    bestn=1;					// needed if goal not reachable

    //
    //	Mark goal (FIXME: can't enter field?)
    //
    r=unit->Command.Data.Move.Range;
    if( unit->Command.Data.Move.Goal ) {
	goal=unit->Command.Data.Move.Goal;
	x=goal->X;
	y=goal->Y;
	j=goal->Type->Type;
	// FIXME: need only mark rectangle
	// FIXME: don't mark outside the matrix!
	i=1;
	for( yd=UnitTypes[j].TileHeight+r; yd-->-r; ) {
	    for( xd=UnitTypes[j].TileWidth+r; xd-->-r; ) {
		if( CheckedCanMoveToMask(x+xd,y+yd,mask) ) {	// reachable
		    matrix[w+1+x+xd+(y+yd)*w]=88;
		    i=0;
		}
	    }
	    if( i ) {
		DebugLevel3("Goal never reachable (%s,%d,%d+%d)\n"
			,goal->Type->Ident,x,y,r);
		IfDebug( PfCounterNotReachable++ );
	    }
	}
    } else {
	x=unit->Command.Data.Move.DX;
	y=unit->Command.Data.Move.DY;
	i=1;
	// FIXME: need only mark rectangle
	// FIXME: don't mark outside the matrix!
	for( yd=r+1; yd-->0; ) {
	    for( xd=r+1; xd-->0; ) {
		DebugLevel3("%d,%d\n",x+xd,y+yd);
		if( CheckedCanMoveToMask(x+xd,y+yd,mask) ) {	// reachable
		    matrix[w+1+x+xd+(y+yd)*w]=88;
		    i=0;
		}
	    }
	}
	if( i ) {
	    DebugLevel3("Goal never reachable (%d,%d+%d)\n",x,y,r);
	    IfDebug( PfCounterNotReachable++; );
	}
    }
    f=1;

    for( ;; ) {
	while( rp!=ep ) {
	    for( j=0; j<8; ++j ) {		// mark all neighbors
		x=points[rp]+xoffset[j];
		y=points[rp+1]+yoffset[j];
		if( matrix[w+1+x+y*w] ) {	// already reached
		    //
		    //	Check if goal reached.
		    //
		    if( matrix[w+1+x+y*w]==88 ) {
			DebugLevel3("Goal reached %d ",n);
			PathTraceBack(matrix,x,y,n,path);
			xd=path[2]-unit->X;
			yd=path[3]-unit->Y;
			DebugLevel3("%d,%d: %d,%d\n",unit->X,unit->Y,xd,yd);
			DebugCheck( xd>1 || xd<-1 );
			DebugCheck( yd>1 || yd<-1 );

			*xdp=xd;
			*ydp=yd;
			IfDebug( PfCounterOk++;
			    //PfCounterDepth+=n;
			    //PfCounterDepth/=2;
			);
			return 1;
		    }
		    continue;
		}
#if 0
		if( x<0 || y<0 || x>=128 || y>=128 ) {
		    DebugLevel1("MATRIX-ERROR: %d\n",matrix[w+1+x+y*w]);
		    DebugLevel1("MATRIX-ERROR: %d\n",matrix[w+1+x+(y+1)*w]);
		}
#endif

		//
		//	Look if destination field is blocked.
		//	Fields blocked by moving units, are considered free.
		//

		i=TheMap.Fields[x+y*TheMap.Width].Flags&mask;
		if( i ) {		// unreachable
		    // Blocked by non-moving in not first round
		    DebugLevel3("%x - %x\n",mask,i);
		    if( f || (i&~(MapFieldLandUnit
				|MapFieldAirUnit|MapFieldSeaUnit)) ) {
			DebugLevel3("NO: %d,%d\n",x,y);
			matrix[w+1+x+y*w]=99;
			continue;
		    }
		    goal=UnitCacheOnXY(x,y,unit->Type->UnitType);
		    if( !goal ) {
			DebugLevel0(__FUNCTION__
			    ": %p No goal for %d,%d on %d,%d?\n",
				    unit,unit->X,unit->Y,x,y);
			//	Clear movement flag from MovementMap
			//	for this type of unit
			//TheMap.MovementMap[x+y*TheMap.Width]&=
			//	~UnitMovement(unit);
			matrix[w+1+x+y*w]=99;
			continue;
		    }
#if 0
		    // FIXME: Have more move actions
		    if( goal->Command.Action!=UnitActionMove ) {
			matrix[w+1+x+y*w]=99;
			continue;
		    }
#endif
		    if( !goal->Moving ) {
			matrix[w+1+x+y*w]=99;
			continue;
		    }
		}

		DebugLevel3("OK: %d,%d\n",x,y);
		matrix[w+1+x+y*w]=n+1;
		points[wp]=x;
		points[wp+1]=y;
		wp+=2;
		if( wp>=sizeof(points) ) {	// round about
		    wp=0;
		}
		xd=abs(unit->Command.Data.Move.DX-x);
		yd=abs(unit->Command.Data.Move.DY-y);
		DebugLevel3("Best: %d,%d-%d - %d,%d\n"
		    ,bestx,besty,bestd
		    ,xd,yd);
		if( xd>yd && xd<bestd ) {
		    bestd=xd;
		    bestx=x;
		    besty=y;
		    bestn=n;
		}
		if( yd>xd && yd<bestd ) {
		    bestd=yd;
		    bestx=x;
		    besty=y;
		    bestn=n;
		}
	    }

	    rp+=2;
	    if( rp>=sizeof(points) ) {	// round about
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
	f=0;
#if 0
	//	FIXME: if we ignore distant units we run left-right
	//	Ignore any units more than 5 fields from start.
	if( n>6 ) {
	    mask&=~(MapMoveLandUnit|MapMoveAirUnit|MapMoveSeaUnit);
	}
#endif
	n=(n+1)&7;
	++depth;
    }

    DebugLevel3("Unreachable Best %d=%d,%d\n",bestn,bestx,besty);

    PathTraceBack(matrix,bestx,besty,bestn,path);
    xd=path[2]-unit->X;
    yd=path[3]-unit->Y;

    DebugCheck( xd>1 || xd<-1 );
    DebugCheck( yd>1 || yd<-1 );

    DebugLevel3("Move: %d,%d\n",*xdp,*ydp);

    *xdp=xd;
    *ydp=yd;

    IfDebug(
	PfCounterFail++;
	PfCounterDepth+=depth;
	PfCounterDepth/=2;
    );
    return -1;
}

/**
**	Find new path.
**
**	@param unit	Path for this unit.
**	@param xdp	Pointer for x direction return.
**	@param xdp	Pointer for y direction return.
**	@return		1 for reached -1 unreachable, 0 on the way.
*/
global int NewPath(Unit* unit,int* xdp,int* ydp)
{
    int x;
    int y;
    Unit* goal;
    int r;
    UnitType* type;

    //
    //	Check if goal reached.
    //
    x=unit->X;
    y=unit->Y;
    r=unit->Command.Data.Move.Range;
    goal=unit->Command.Data.Move.Goal;
    DebugLevel3(__FUNCTION__": Goal %p\n",unit->Command.Data.Move.Goal);

    if( goal ) {			// goal unit
	type=goal->Type;
	DebugLevel3(__FUNCTION__": Unit %d,%d Goal %d,%d - %d,%d\n"
	    ,x,y
	    ,goal->X-r,goal->Y-r
	    ,goal->X+type->TileWidth+r
	    ,goal->Y+type->TileHeight+r);
	if( x>=goal->X-r && x<goal->X+type->TileWidth+r
		&& y>=goal->Y-r && y<goal->Y+type->TileHeight+r ) {
	    DebugLevel3(__FUNCTION__": Goal reached\n");
	    *xdp=*ydp=0;
	    return 1;
	}
    } else {				// goal map field
	if( x>=unit->Command.Data.Move.DX
		&& x<=unit->Command.Data.Move.DX+r
		&& y>=unit->Command.Data.Move.DY
		&& y<=unit->Command.Data.Move.DY+r ) {
	    DebugLevel3(__FUNCTION__": Field reached\n");
	    *xdp=*ydp=0;
	    return 1;
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
		DebugLevel3(__FUNCTION__": Field unreached\n");
		*xdp=*ydp=0;
		return -1;
	    }
	}
    }

    //
    //	If possible, try fast.
    //
    if( unit->Command.Data.Move.Fast ) {
	if( FastNewPath(unit,xdp,ydp) ) {
	    DebugCheck( *xdp==0 && *ydp==0 );
	    return 0;
	}
	unit->Command.Data.Move.Fast=0;
	DebugLevel3(__FUNCTION__": Fallback to slow method\n");
    }

    //
    //	Fall back to slow complex method.
    //
    r=ComplexNewPath(unit,xdp,ydp);
    // FIXME: must be done by complex!
    if( *xdp==0 && *ydp==0 ) {
	return -1;
    }
    return 0;
}

//@}
