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
/**@name action_move.c	-	The move action. */
//
//	(c) Copyright 1998,2000 by Lutz Sammer
//
//	$Id$

//@{

#include <stdio.h>
#include <stdlib.h>

#include "freecraft.h"
#include "video.h"
#include "sound_id.h"
#include "unitsound.h"
#include "unittype.h"
#include "player.h"
#include "unit.h"
#include "tileset.h"
#include "map.h"
#include "actions.h"
#include "pathfinder.h"
#include "missile.h"
#include "sound.h"

//
//	Convert heading into direction.
//
			//  N NE  E SE  S SW  W NW
local int Heading2X[8] = {  0,+1,+1,+1, 0,-1,-1,-1 };
local int Heading2Y[8] = { -1,-1, 0,+1,+1,+1, 0,-1 };

/**
**	Generic unit mover.
**
**	@param unit	Unit that moves.
**	@param move	Animation script for unit.
**
**	@returns	>0 remaining path length, 0 wait for path, -1
**			reached goal, -2 can't reach the goal.
*/
local int ActionMoveGeneric(Unit* unit,const Animation* move)
{
    int xd;
    int yd;
    int state;
    Unit* goal;
    int d;
    int i;

    // FIXME: state 0?, should be wrong, should be Reset.
    if( !(state=unit->State) ) {

	//
	//	Target killed?
	//
	goal=unit->Command.Data.Move.Goal;
	if( goal ) {
	    // FIXME: should this be handled here?
	    // FIXME: Can't choose a better target here!
	    if( goal->Destroyed ) {
		DebugLevel0Fn("destroyed unit\n");
		DebugCheck( !goal->Refs );
		if( !--goal->Refs ) {
		    ReleaseUnit(goal);
		}
		unit->Command.Data.Move.Goal=goal=NoUnitP;
	    } else if( goal->Removed ||
		    !goal->HP || goal->Command.Action==UnitActionDie ) {
		DebugLevel0Fn("killed unit\n");
		DebugCheck( !goal->Refs );
		--goal->Refs;
		DebugCheck( !goal->Refs );
		unit->Command.Data.Move.Goal=goal=NoUnitP;
	    }
	}

	switch( d=NextPathElement(unit,&xd,&yd) ) {
	    case PF_UNREACHABLE:	// Can't reach, stop
		unit->Reset=unit->Wait=1;
		unit->Moving=0;
		unit->Command.Action=UnitActionStill;
		return d;
	    case PF_REACHED:		// Reached goal, stop
		unit->Reset=unit->Wait=1;
		unit->Moving=0;
		unit->Command.Action=UnitActionStill;
		return d;
	    case PF_WAIT:		// No path, wait
		unit->Reset=unit->Wait=1;
		unit->Moving=0;
		return d;
	    default:			// On the way moving
		unit->Moving=1;
		break;
	}

	//
	//	Transporter (un)docking?
	//
	if( unit->Type->Transporter
		&& ( (WaterOnMap(unit->X,unit->Y)
		    && CoastOnMap(unit->X+xd,unit->Y+yd))
		|| (CoastOnMap(unit->X,unit->Y)
			&& WaterOnMap(unit->X+xd,unit->Y+yd)) ) ) {
	    PlayUnitSound(unit,VoiceDocking);
	}

	//
	//	Update movement map.
	//
	i=UnitFieldFlags(unit);
	TheMap.Fields[unit->X+unit->Y*TheMap.Width].Flags&=~i;

	UnitCacheRemove(unit);
	unit->X+=xd;
	unit->Y+=yd;
	UnitCacheInsert(unit);

	TheMap.Fields[unit->X+unit->Y*TheMap.Width].Flags|=i;

	MustRedraw|=RedrawMinimap;
	//
	//	Update visible area.
	//
#ifdef NEW_FOW
	MapMarkNewSight(unit->Player
		,unit->X,unit->Y,unit->Stats->SightRange,xd,yd);
#else
	if( unit->Player==ThisPlayer ) {
	    MapMarkNewSight(unit->X,unit->Y,unit->Stats->SightRange,xd,yd);
	}
#endif

	unit->IX=-xd*TileSizeX;
	unit->IY=-yd*TileSizeY;
	unit->Frame=0;
	UnitHeadingFromDeltaXY(unit,xd,yd);
    } else {
	xd=Heading2X[unit->Direction/NextDirection];
	yd=Heading2Y[unit->Direction/NextDirection];
	d=0;
    }

    DebugLevel3Fn(": %d,%d State %2d ",xd,yd,unit->State);
    DebugLevel3("Walk %d Frame %2d Wait %3d Heading %d %d,%d\n"
	    ,move[state].Pixel
	    ,move[state].Frame
	    ,move[state].Sleep
	    ,unit->Heading
	    ,unit->IX,unit->IY);

    unit->IX+=xd*move[state].Pixel;
    unit->IY+=yd*move[state].Pixel;
    unit->Frame+=move[state].Frame;
    unit->Wait=move[state].Sleep;

    if( (move[state].Pixel || move[state].Frame) && UnitVisible(unit) ) {
	// FIXME: Must do better flags.
	MustRedraw|=RedrawMap;
    }

    if( move[state].Flags&AnimationReset ) {
	unit->Reset=1;
    }
    if( move[state].Flags&AnimationRestart ) {
	unit->State=0;
    } else {
	++unit->State;
    }

    return d;
}

/**
**	Unit moves!
**
**	@param unit	Pointer to unit.
**
**	@returns	>0 remaining path length, 0 wait for path, -1
**			reached goal, -2 can't reach the goal.
*/
global int HandleActionMove(Unit* unit)
{
    if( unit->Type->Animations ) {
	DebugLevel3("%s: %p\n",unit->Type->Ident,unit->Type->Animations );
	return ActionMoveGeneric(unit,unit->Type->Animations->Move);
    }

    return PF_UNREACHABLE;
}

//@}
