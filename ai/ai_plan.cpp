//   ___________		     _________		      _____  __
//   \_	  _____/______   ____   ____ \_   ___ \____________ _/ ____\/  |_
//    |    __) \_  __ \_/ __ \_/ __ \/    \  \/\_  __ \__  \\   __\\   __|
//    |     \   |  | \/\  ___/\  ___/\     \____|  | \// __ \|  |   |  |
//    \___  /   |__|    \___  >\___  >\______  /|__|  (____  /__|   |__|
//	  \/		    \/	   \/	     \/		   \/
//  ______________________                           ______________________
//			  T H E   W A R   B E G I N S
//	   FreeCraft - A free fantasy real time strategy game engine
//
/**@name ai_plan.c	-	AI planning functions. */
//
//      (c) Copyright 2002-2003 by Lutz Sammer and Jimmy Salmon
//
//	FreeCraft is free software; you can redistribute it and/or modify
//	it under the terms of the GNU General Public License as published
//	by the Free Software Foundation; only version 2 of the License.
//
//	FreeCraft is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//	GNU General Public License for more details.
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

#include "missile.h"
#include "unittype.h"
#include "map.h"
#include "pathfinder.h"
#include "actions.h"
#include "ai_local.h"

/*----------------------------------------------------------------------------
--	Variables
----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
--	Functions
----------------------------------------------------------------------------*/

/**
**	Choose enemy on map tile.
**
**	@param source	Unit which want to attack.
**	@param tx	X position on map, tile-based.
**	@param ty	Y position on map, tile-based.
**
**	@return		Returns ideal target on map tile.
*/
local Unit* EnemyOnMapTile(const Unit* source,int tx,int ty)
{
    Unit* table[UnitMax];
    Unit* unit;
    Unit* best;
    const UnitType* type;
    int n;
    int i;

    n=SelectUnitsOnTile(tx,ty,table);
    best=NoUnitP;
    for( i=0; i<n; ++i ) {
	unit=table[i];
	// unusable unit ?
	// if( UnitUnusable(unit) ) can't attack constructions
	// FIXME: did SelectUnitsOnTile already filter this?
	// Invisible and not Visible
	if( unit->Removed || unit->Invisible || !unit->HP
		|| !(unit->Visible&(1<<source->Player->Player))
		|| unit->Orders[0].Action==UnitActionDie ) {
	    continue;
	}
	type=unit->Type;
	if( tx<unit->X || tx>=unit->X+type->TileWidth
		|| ty<unit->Y || ty>=unit->Y+type->TileHeight ) {
	    continue;
	}
	if( !CanTarget(source->Type,unit->Type) ) {
	    continue;
	}
	if( !IsEnemy(source->Player,unit) ) {	// a friend or neutral
	    continue;
	}

	//
	//	Choose the best target.
	//
	if( !best || best->Type->Priority<unit->Type->Priority ) {
	    best=unit;
	}
    }
    return best;
}

/**
**	Mark all by transporter reachable water tiles.
**
**	@param unit	Transporter
**	@param matrix	Water matrix.
**
**	@note only works for water transporters!
*/
local void AiMarkWaterTransporter(const Unit* unit,unsigned char* matrix)
{
    static const int xoffset[]={  0,-1,+1, 0, -1,+1,-1,+1 };
    static const int yoffset[]={ -1, 0, 0,+1, -1,-1,+1,+1 };
    struct {
	unsigned short X;
	unsigned short Y;
    } * points;
    int size;
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

    x=unit->X;
    y=unit->Y;
    w=TheMap.Width+2;
    matrix+=w+w+2;
    if( matrix[x+y*w] ) {		// already marked
	DebugLevel0("Done\n");
	return;
    }

    points=malloc(TheMap.Width*TheMap.Height);
    size=TheMap.Width*TheMap.Height/sizeof(*points);

    //
    //	Make movement matrix.
    //
    mask=UnitMovementMask(unit);
    // Ignore all possible mobile units.
    mask&=~(MapFieldLandUnit|MapFieldAirUnit|MapFieldSeaUnit);

    points[0].X=x;
    points[0].Y=y;
    rp=0;
    matrix[x+y*w]=66;				// mark start point
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

		if( CanMoveToMask(x,y,mask) ) {	// reachable
		    /*MakeLocalMissile(MissileTypeRedCross,
			x*TileSizeX+TileSizeX/2,y*TileSizeY+TileSizeY/2,
			x*TileSizeX+TileSizeX/2,y*TileSizeY+TileSizeY/2);*/
			
		    *m=66;
		    points[wp].X=x;		// push the point
		    points[wp].Y=y;
		    if( ++wp>=size ) {		// round about
			wp=0;
		    }
		/*	Must be checked multiple
		} else {			// unreachable
		    *m=99;
		*/
		}
	    }

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
}

/**
**	Find possible targets.
**
**	@param unit	Attack.
**	@param matrix	Water matrix.
**	@param dx	Attack point X.
**	@param dy	Attack point Y.
**	@param ds	Attack state.
**
**	@return 	True if target found.
*/
local int AiFindTarget(const Unit* unit,unsigned char* matrix,
	int* dx,int* dy,int* ds)
{
    static const int xoffset[]={  0,-1,+1, 0, -1,+1,-1,+1 };
    static const int yoffset[]={ -1, 0, 0,+1, -1,-1,+1,+1 };
    struct {
	unsigned short X;
	unsigned short Y;
	unsigned char State;
    } * points;
    int size;
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
    enum { OnWater, OnLand, OnIsle } state;
    unsigned char* m;

    size=TheMap.Width*TheMap.Height/2;
    points=alloca(size*sizeof(*points));

    x=unit->X;
    y=unit->Y;

    w=TheMap.Width+2;
    mask=UnitMovementMask(unit);
    // Ignore all possible mobile units.
    mask&=~(MapFieldLandUnit|MapFieldAirUnit|MapFieldSeaUnit);

    points[0].X=x;
    points[0].Y=y;
    points[0].State=OnLand;
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
	    state=points[rp].State;
	    for( i=0; i<8; ++i ) {		// mark all neighbors
		x=rx+xoffset[i];
		y=ry+yoffset[i];
		m=matrix+x+y*w;

		if( state!=OnWater ) {
		    if( *m ) {			// already checked
			if( state==OnLand && *m==66 ) {	// tansporter?
			    DebugLevel0Fn("->Water\n");
			    *m=6;
			    points[wp].X=x;	// push the point
			    points[wp].Y=y;
			    points[wp].State=OnWater;
			    if( ++wp>=size ) {	// round about
				wp=0;
			    }
			}
			continue;
		    }

		    // Check targets on tile?
		    // FIXME: the move code didn't likes a shore building as
		    //		target
		    if( EnemyOnMapTile(unit,x,y) ) {
			DebugLevel0Fn("Target found %d,%d-%d\n"
				_C_ x _C_ y _C_ state);
			*dx=x;
			*dy=y;
			*ds=state;
			return 1;
		    }

		    if( CanMoveToMask(x,y,mask) ) {	// reachable
			/*MakeLocalMissile(MissileTypeGreenCross,
			    x*TileSizeX+TileSizeX/2,y*TileSizeY+TileSizeY/2,
			    x*TileSizeX+TileSizeX/2,y*TileSizeY+TileSizeY/2);*/
			    
			*m=1;
			points[wp].X=x;		// push the point
			points[wp].Y=y;
			points[wp].State=state;
			if( ++wp>=size ) {	// round about
			    wp=0;
			}
		    } else {			// unreachable
			*m=99;
		    }
		} else {			// On water
		    if( *m ) {			// already checked 
			if( *m==66 ) {		// tansporter?
			    *m=6;
			    points[wp].X=x;	// push the point
			    points[wp].Y=y;
			    points[wp].State=OnWater;
			    if( ++wp>=size ) {	// round about
				wp=0;
			    }
			}
			continue;
		    }
		    if( CanMoveToMask(x,y,mask) ) {	// reachable
			DebugLevel0Fn("->Land\n");
			*m=1;
			points[wp].X=x;		// push the point
			points[wp].Y=y;
			points[wp].State=OnIsle;
			if( ++wp>=size ) {	// round about
			    wp=0;
			}
		    } else {			// unreachable
			*m=99;
		    }
		}
	    }

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
    return 0;
}

/**
**	Find possible walls to target.
**
**	@param force	Attack force.
**
**	@return 	True if wall found.
*/
global int AiFindWall(AiForce* force)
{
    static const int xoffset[]={  0,-1,+1, 0, -1,+1,-1,+1 };
    static const int yoffset[]={ -1, 0, 0,+1, -1,-1,+1,+1 };
    struct {
	unsigned short X;
	unsigned short Y;
    } * points;
    int size;
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
    int destx;
    int desty;
    AiUnit* aiunit;
    Unit* unit;

    // Find a unit to use.  Best choice is a land unit with range 1.
    // Next best choice is any land unit.  Otherwise just use the first.
    aiunit=force->Units;
    unit=aiunit->Unit;
    while( aiunit ) {
	if( aiunit->Unit->Type->UnitType==UnitTypeLand ) {
	    unit=aiunit->Unit;
	    if( aiunit->Unit->Type->Missile.Missile->Range==1 ) {
		break;
	    }
	}
	aiunit=aiunit->Next;
    }

    x=unit->X;
    y=unit->Y;
    size=TheMap.Width*TheMap.Height/4;
    points=alloca(size*sizeof(*points));

    destx=-1;
    desty=-1;

    matrix=CreateMatrix();
    w=TheMap.Width+2;
    matrix+=w+w+2;

    points[0].X=x;
    points[0].Y=y;
    rp=0;
    matrix[x+y*w]=1;				// mark start point
    ep=wp=1;					// start with one point

    mask=UnitMovementMask(unit);

    //
    //	Pop a point from stack, push all neighbors which could be entered.
    //
    for( ; destx==-1; ) {
	while( rp!=ep && destx==-1 ) {
	    rx=points[rp].X;
	    ry=points[rp].Y;
	    for( i=0; i<8; ++i ) {		// mark all neighbors
		x=rx+xoffset[i];
		y=ry+yoffset[i];
		m=matrix+x+y*w;
		if( *m ) {
		    continue;
		}

		// 
		//	Check for a wall
		//
		if( WallOnMap(x,y) ) {
		    DebugLevel0Fn("Wall found %d,%d\n" _C_ x _C_ y);
		    destx=x;
		    desty=y;
		    break;
		}

		if( CanMoveToMask(x,y,mask) ) {	// reachable
		    *m=1;
		    points[wp].X=x;		// push the point
		    points[wp].Y=y;
		    if( ++wp>=size ) {		// round about
			wp=0;
		    }
		} else {			// unreachable
		    *m=99;
		}
	    }
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

    if( destx!=-1 ) {
	force->State=0;
	aiunit=force->Units;
	while( aiunit ) {
	    if( aiunit->Unit->Type->CanAttack ) {
		CommandAttack(aiunit->Unit,destx,desty,NULL,FlushCommands);
	    } else {
		CommandMove(aiunit->Unit,destx,desty,FlushCommands);
	    }
	    aiunit=aiunit->Next;
	}
	return 1;
    }

    return 0;
}

/**
**	Plan an attack with a force.
**	We know, that we must use a transporter.
**
**	@param force	Pointer on the force.
**
**	@return		True if target found, false otherwise.
**
**	@todo	Perfect planning.
**		Only works for water transporter!
*/
global int AiPlanAttack(AiForce* force)
{
    char* watermatrix;
    const AiUnit* aiunit;
    int x;
    int y;
    int i;
    int state;
    Unit* transporter;

    DebugLevel0Fn("Planning for force #%d of player #%d\n"
	_C_ force-AiPlayer->Force _C_ AiPlayer->Player->Player);

    watermatrix=CreateMatrix();

    //
    //	Transporter must be already assigned to the force.
    //	NOTE: finding free transportes was too much work for me.
    //
    aiunit=force->Units;
    state=1;
    while( aiunit ) {
	if( aiunit->Unit->Type->Transporter ) {
	    DebugLevel0Fn("Transporter #%d\n" _C_ UnitNumber(aiunit->Unit));
	    AiMarkWaterTransporter(aiunit->Unit,watermatrix);
	    state=0;
	}
	aiunit=aiunit->Next;
    }

    //
    //	No transport that belongs to the force.
    //
    transporter=NULL;
    if( state ) {
	for( i=0; i<AiPlayer->Player->TotalNumUnits; ++i ) { 
	    Unit* unit;

	    unit=AiPlayer->Player->Units[i];
	    if( unit->Type->Transporter 
		    && unit->Orders[0].Action==UnitActionStill
		    && unit->OrderCount==1 && !unit->OnBoard[0] ) {
		DebugLevel0Fn("Assign any transporter\n");
		AiMarkWaterTransporter(unit,watermatrix);
		// FIXME: can be the wrong transporter.
		transporter=unit;
		state=0;
	    }
	}
    }

    if( state ) {			// Absolute no transporter
	DebugLevel0Fn("No transporter available\n");
	// FIXME: should tell the resource manager we need a transporter!
	return 0;
    }

    //
    //	Find a land unit of the force.
    //		FIXME: if force is split over different places -> broken
    //
    aiunit=force->Units;
    while( aiunit ) {
	if( aiunit->Unit->Type->UnitType==UnitTypeLand ) {
	    DebugLevel0Fn("Landunit %d\n" _C_ UnitNumber(aiunit->Unit));
	    break;
	}
	aiunit=aiunit->Next;
    }

    if( !aiunit ) {
	DebugLevel0Fn("No land unit in force\n");
	return 0;
    }

    if( AiFindTarget(aiunit->Unit,watermatrix,&x,&y,&state) ) {
	AiUnit* aiunit;

	if( transporter ) {
	    aiunit=malloc(sizeof(*aiunit));
	    aiunit->Next=force->Units;
	    force->Units=aiunit;
	    aiunit->Unit=transporter;
	    RefsDebugCheck( transporter->Destroyed || !transporter->Refs );
	    ++transporter->Refs;
	}

	DebugLevel0Fn("Can attack\n");
	force->GoalX=x;
	force->GoalY=y;
	force->MustTransport= state==2 ;

	force->State=1;
	return 1;
    }
    return 0;
}

//@}

#endif // } NEW_AI
