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
//	(c) Copyright 1998,2001 by Lutz Sammer
//
//	$Id$

//@{

/*----------------------------------------------------------------------------
--	Includes
----------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>

#include "freecraft.h"
#include "video.h"
#include "unittype.h"
#include "player.h"
#include "unit.h"
#include "tileset.h"
#include "map.h"
#include "actions.h"
#include "pathfinder.h"
#include "sound.h"
#include "interface.h"

/*----------------------------------------------------------------------------
--	Variables
----------------------------------------------------------------------------*/

//
//	Convert heading into direction.
//
			//  N NE  E SE  S SW  W NW
local int Heading2X[8] = {  0,+1,+1,+1, 0,-1,-1,-1 };
local int Heading2Y[8] = { -1,-1, 0,+1,+1,+1, 0,-1 };

/*----------------------------------------------------------------------------
--	Function
----------------------------------------------------------------------------*/

/**
**	Generic unit mover.
**
**	@param unit	Unit that moves.
**	@param anim	Animation script for unit.
**
**	@return		>0 remaining path length, 0 wait for path, -1
**			reached goal, -2 can't reach the goal.
*/
local int ActionMoveGeneric(Unit* unit,const Animation* anim)
{
    int xd;
    int yd;
    int state;
    int d;
    int i;

    // FIXME: state 0?, should be wrong, should be Reset.
    // FIXME: Reset flag is cleared by HandleUnitAction.
    if( !(state=unit->State) ) {

	//
	//	Target killed?
	//
#ifdef NEW_ORDERS
#else
	Unit* goal;

	if( (goal=unit->Command.Data.Move.Goal) ) {
	    // FIXME: should this be handled here? JOHNS: No see NEW_ORDERS
	    // FIXME: Can't choose a better target here!
	    if( goal->Destroyed ) {
		DebugLevel0Fn("destroyed unit\n");
		unit->Command.Data.Move.DX=goal->X+goal->Type->TileWidth/2;
		unit->Command.Data.Move.DY=goal->Y+goal->Type->TileHeight/2;
		RefsDebugCheck( !goal->Refs );
		if( !--goal->Refs ) {
		    ReleaseUnit(goal);
		}
		unit->Command.Data.Move.Goal=NoUnitP;
		ResetPath(unit->Command);
	    } else if( goal->Removed ||
		    !goal->HP || goal->Command.Action==UnitActionDie ) {
		DebugLevel0Fn("killed unit\n");
		RefsDebugCheck( !goal->Refs );
		--goal->Refs;
		RefsDebugCheck( !goal->Refs );
		unit->Command.Data.Move.DX=goal->X;
		unit->Command.Data.Move.DY=goal->Y;
		unit->Command.Data.Move.Goal=NoUnitP;
		ResetPath(unit->Command);
	    }
	}
#endif

	switch( d=NextPathElement(unit,&xd,&yd) ) {
	    case PF_UNREACHABLE:	// Can't reach, stop
		unit->Reset=unit->Wait=1;
		unit->Moving=0;
#ifndef NEW_ORDERS
		unit->Command.Action=UnitActionStill;
#endif
		return d;
	    case PF_REACHED:		// Reached goal, stop
		unit->Reset=unit->Wait=1;
		unit->Moving=0;
#ifndef NEW_ORDERS
		unit->Command.Action=UnitActionStill;
#endif
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
#ifdef NEW_SHIPS
	if( unit->Type->UnitType==UnitTypeLand ) {
	    xd=Heading2X[unit->Direction/NextDirection];
	    yd=Heading2Y[unit->Direction/NextDirection];
	} else {
	    xd=Heading2X[unit->Direction/NextDirection]*2;
	    yd=Heading2Y[unit->Direction/NextDirection]*2;
	}
	d=0;
#else
	xd=Heading2X[unit->Direction/NextDirection];
	yd=Heading2Y[unit->Direction/NextDirection];
	d=0;
#endif
    }

    DebugLevel3Fn(": %d,%d State %2d ",xd,yd,unit->State);
    DebugLevel3("Walk %d Frame %2d Wait %3d Heading %d %d,%d\n"
	    ,anim[state].Pixel
	    ,anim[state].Frame
	    ,anim[state].Sleep
	    ,unit->Direction
	    ,unit->IX,unit->IY);

    //
    //	Next animation.
    //
    unit->IX+=xd*anim[state].Pixel;
    unit->IY+=yd*anim[state].Pixel;
    unit->Frame+=anim[state].Frame;
    unit->Wait=anim[state].Sleep;

    //
    //	Any graphic change?
    //
    if( !state || anim[state].Pixel || anim[state].Frame ) {
        CheckUnitToBeDrawn(unit);
    }

    //
    //	Handle the flags.
    //
    if( anim[state].Flags&AnimationReset ) {
	unit->Reset=1;
    }
    if( anim[state].Flags&AnimationRestart ) {
	unit->State=0;
    } else {
	++unit->State;
    }

    return d;
}

/**
**	Unit moves! Generic function called from other actions.
**
**	@param unit	Pointer to unit.
**
**	@return		>0 remaining path length, 0 wait for path, -1
**			reached goal, -2 can't reach the goal.
*/
global int DoActionMove(Unit* unit)
{
    if( unit->Type->Animations ) {
	DebugLevel3("%s: %p\n",unit->Type->Ident,unit->Type->Animations );
	return ActionMoveGeneric(unit,unit->Type->Animations->Move);
    }

    return PF_UNREACHABLE;
}

/**
**	Unit move action:
**
**	Move to a place or to an unit (can move).
**	Tries 10x to reach the target, note this are the complete tries.
**	If the target entered another unit, move to it's position.
**	If the target unit is destroyed, continue to move to it's last position.
**
**	@param unit	Pointer to unit.
*/
global void HandleActionMove(Unit* unit)
{
#ifdef NEW_ORDERS
    Unit* goal;
#endif

    DebugLevel3Fn("%Zd: %Zd %d,%d \n",UnitNumber(unit),
	    unit->Orders[0].Goal ? UnitNumber(unit->Orders[0].Goal) : -1,
	    unit->Orders[0].X,unit->Orders[0].Y);

    if( !unit->SubAction ) {		// first entry
	unit->SubAction=1;
#ifdef NEW_ORDERS
	NewResetPath(unit);
	//
	//	FIXME: should use a reachable place to reduce pathfinder time.
	//
	IfDebug(
	if( !PlaceReachable(unit,unit->Orders[0].X,unit->Orders[0].Y,1) ) {
	    DebugLevel0Fn("FIXME: should use other goal.\n");
	});
#endif
	DebugCheck( unit->State!=0 );
    }

    switch( DoActionMove(unit) ) {	// reached end-point?
	case PF_UNREACHABLE:
	    //
	    //	Some tries to reach the goal
	    //
	    if( unit->SubAction++<10 ) {
#ifndef NEW_ORDERS
		unit->Command.Action=UnitActionMove;
#endif
		//	To keep the load low, retry delayed.
		unit->Wait=FRAMES_PER_SECOND/10+unit->SubAction;
		// FIXME: Now the units didn't defend themself :(((((((
		break;
	    }
	    // FALL THROUGH
	case PF_REACHED:
	    unit->SubAction=0;
#ifdef NEW_ORDERS
	    // Release target, if any.
	    if( (goal=unit->Orders[0].Goal) ) {
		RefsDebugCheck( !goal->Refs );
		if( !--goal->Refs ) {
		    DebugCheck( !goal->Destroyed );
		    ReleaseUnit(goal);
		}
		unit->Orders[0].Goal=NoUnitP;
	    }
	    unit->Orders[0].Action=UnitActionStill;
	    if( IsSelected(unit) ) {	// update display for new action
		UpdateButtonPanel();
	    }
	    return;
#else
	    if( IsSelected(unit) ) {	// update display for new action
		UpdateButtonPanel();
	    }
#endif
	default:
	    break;
    }

#ifdef NEW_ORDERS
    //
    //	Target destroyed?
    //
    if( (goal=unit->Orders[0].Goal) && goal->Destroyed ) {
	DebugLevel0Fn("Goal dead\n");
	unit->Orders[0].X=goal->X+goal->Type->TileWidth/2;
	unit->Orders[0].Y=goal->Y+goal->Type->TileHeight/2;
	unit->Orders[0].Goal=NoUnitP;
	RefsDebugCheck( !goal->Refs );
	if( !--goal->Refs ) {
	    ReleaseUnit(goal);
	}
	NewResetPath(unit);
    }
#endif
}

//@}
