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
/**@name ai_building.c	-	AI building functions. */
//
//      (c) Copyright 2001 by Lutz Sammer
//
//      $Id$

#ifdef NEW_AI	// {

//@{

/*----------------------------------------------------------------------------
--	Includes
----------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>

#include "freecraft.h"

#include "unit.h"
#include "map.h"
#include "pathfinder.h"
#include "ai_local.h"

/*----------------------------------------------------------------------------
--	Variables
----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
--	Functions
----------------------------------------------------------------------------*/

/**
**	Check if the surrounding are free.
**
**	@param worker	Worker to build.
**	@param type	Type of building.
**	@param x	X map tile position for the building.
**	@param y	Y map tile position for the building.
**
**	@return		True if the surrounding is free, false otherwise.
**
**	@note		Can be faster written.
*/
local int AiCheckSurrounding(const Unit* worker,const UnitType* type,
	int x, int y)
{
    int i;
    int h;
    int w;

    h=type->TileHeight+2;
    w=type->TileWidth+2;
    --x;
    --y;

    for( i=0; i<w; ++i ) {		// Top row
	if( (x+i)<0 || (x+i)>TheMap.Width ) {	// FIXME: slow, worse,...
	    continue;
	}
	if( !((x+i)==worker->X && y==worker->Y) && y>=0
		&& TheMap.Fields[x+i+y*TheMap.Width].Flags&(MapFieldUnpassable
		|MapFieldWall|MapFieldRocks|MapFieldForest|MapFieldBuilding) ) {
	    return 0;
	}				// Bot row
	if( !((x+i)==worker->X && (y+h)==worker->Y) && (y+h)<TheMap.Height
		&& TheMap.Fields[x+i+(y+h)*TheMap.Width].Flags&
		(MapFieldUnpassable|MapFieldWall|MapFieldRocks
		|MapFieldForest|MapFieldBuilding) ) {
	    return 0;
	}
    }

    ++y;
    h-=2;
    for( i=0; i<h; ++i ) {		// Left row
	if( (y+i)<0 || (y+i)>TheMap.Height ) {	// FIXME: slow, worse,...
	    continue;
	}
	if( !(x==worker->X && (y+i)==worker->Y) && x>=0
		&& TheMap.Fields[x+(y+i)*TheMap.Width].Flags&
		(MapFieldUnpassable|MapFieldWall|MapFieldRocks
		|MapFieldForest|MapFieldBuilding) ) {
	    return 0;
	}				// Right row
	if( !((x+w)==worker->X && (y+i)==worker->Y) && (x+w)<TheMap.Width
		&& TheMap.Fields[x+w+(y+i)*TheMap.Width].Flags&
		(MapFieldUnpassable|MapFieldWall|MapFieldRocks
		|MapFieldForest|MapFieldBuilding) ) {
	    return 0;
	}
    }
    return 1;
}

#if 0
/**
**      Find free building place.
**
**	@param worker	Worker to build building.
**	@param type	Type of building.
**	@param dx	Pointer for X position returned.
**	@param dy	Pointer for Y position returned.
**	@param flag	Flag if surrounding must be free.
**	@return		True if place found, false if no found.
**
**	@note	This can be done faster, use flood fill.
*/
local int AiFindBuildingPlace2(const Unit * worker, const UnitType * type,
	int *dx, int *dy,int flag)
{
    int wx, wy, x, y, addx, addy;
    int end, state;

    wx = worker->X;
    wy = worker->Y;
    x = wx;
    y = wy;
    addx = 1;
    addy = 1;

    state = 0;
    end = y + addy - 1;
    for (;;) {				// test rectangles arround the place
	switch (state) {
	case 0:
	    if (y++ == end) {
		++state;
		end = x + addx++;
	    }
	    break;
	case 1:
	    if (x++ == end) {
		++state;
		end = y - addy++;
	    }
	    break;
	case 2:
	    if (y-- == end) {
		++state;
		end = x - addx++;
	    }
	    break;
	case 3:
	    if (x-- == end) {
		state = 0;
		end = y + addy++;
		if( addx>=TheMap.Width && addy>=TheMap.Height ) {
		    return 0;
		}
	    }
	    break;
	}

	// FIXME: this check outside the map could be speeded up.
	if (y < 0 || x < 0 || y >= TheMap.Height || x >= TheMap.Width) {
	    continue;
	}
	if (CanBuildUnitType(worker, type, x, y)
		&& (!flag || AiCheckSurrounding(worker,type, x, y))
		&& PlaceReachable(worker, x, y, 1) ) {
	    *dx=x;
	    *dy=y;
	    return 1;
	}
    }
    return 0;
}

#endif

/**
**      Find free building place. (flood fill version)
**
**	@param worker	Worker to build building.
**	@param type	Type of building.
**	@param dx	Pointer for X position returned.
**	@param dy	Pointer for Y position returned.
**	@param flag	Flag if surrounding must be free.
**	@return		True if place found, false if no found.
*/
local int AiFindBuildingPlace2(const Unit * worker, const UnitType * type,
	int *dx, int *dy,int flag)
{
    static const int xoffset[]={  0,-1,+1, 0, -1,+1,-1,+1 };
    static const int yoffset[]={ -1, 0, 0,+1, -1,-1,+1,+1 };
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
    int i;
    int w;
    unsigned char* m;
    unsigned char* matrix;

    x=worker->X;
    y=worker->Y;
    //
    //	Look if we can build at current place.
    //
    if (CanBuildUnitType(worker, type, x, y)
	    && (!flag || AiCheckSurrounding(worker,type, x, y)) ) {
	*dx=x;
	*dy=y;
	return 1;
    }

    //
    //	Make movement matrix.
    //
    matrix=CreateMatrix();
    w=TheMap.Width+2;

    mask=UnitMovementMask(worker);
    // Ignore all possible mobile units.
    mask&=~(MapFieldLandUnit|MapFieldAirUnit|MapFieldSeaUnit);

    points[0].X=x;
    points[0].Y=y;
    matrix+=w+w+2;
    rp=0;
    matrix[x+y*w]=1;				// mark start point
    ep=wp=1;					// start with one point

    //
    //	Pop a point from stack, push all neightbors which could be entered.
    //
    for( ;; ) {
	while( rp!=ep ) {
	    rx=points[rp].X;
	    ry=points[rp].Y;
	    for( i=0; i<8; ++i ) {		// mark all neighbors
		x=rx+xoffset[i];
		y=ry+yoffset[i];
		m=matrix+x+y*w;
		if( *m ) {			// already checked
		    continue;
		}

		//
		//	Look if we can build here.
		//
		if (CanBuildUnitType(worker, type, x, y)
			&& (!flag || AiCheckSurrounding(worker,type, x, y)) ) {
		    *dx=x;
		    *dy=y;
		    return 1;
		}

		if( CanMoveToMask(x,y,mask) ) {	// reachable
		    *m=1;
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
    }

    return 0;
}

/**
**      Find free building place.
**
**	@param worker	Worker to build building.
**	@param type	Type of building.
**	@param dx	Pointer for X position returned.
**	@param dy	Pointer for Y position returned.
**	@return		True if place found, false if no found.
*/
global int AiFindBuildingPlace(const Unit * worker, const UnitType * type,
	int *dx, int *dy)
{
    if( AiFindBuildingPlace2(worker,type,dx,dy,1) ) {
	return 1;
    }

    // FIXME: Should do this if all units can't build better!
    //return AiFindBuildingPlace2(worker,type,dx,dy,0);

    return 0;
}

//@}

#endif // } NEW_AI
