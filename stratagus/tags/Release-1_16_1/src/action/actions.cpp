/*
**	A clone of a famous game.
*/
/**@name actions.c	-	The actions. */
/*
**	(c) Copyright 1998,2000 by Lutz Sammer
**
**	$Id$
*/

//@{

#include <stdio.h>
#include <stdlib.h>

#include "clone.h"
#include "video.h"
#include "sound_id.h"
#include "unitsound.h"
#include "unittype.h"
#include "player.h"
#include "unit.h"
#include "actions.h"
#include "interface.h"

/*----------------------------------------------------------------------------
--	Animation
----------------------------------------------------------------------------*/

/**
**	Show unit animation.
**		Returns animation flags.
*/
global int UnitShowAnimation(Unit* unit,Animation* animation)
{
    int state;
    int flags;

    if( !(state=unit->State) ) {
	UnitNewHeading(unit);		// FIXME: remove this!!
    }

    DebugLevel3(__FUNCTION__": State %2d ",state);
    DebugLevel3("Flags %2d Pixel %2d Frame %2d Wait %3d "
	    ,animation[state].Flags,animation[state].Pixel
	    ,animation[state].Frame,animation[state].Sleep);
    DebugLevel3("Heading %d +%d,%d\n",unit->Heading,unit->IX,unit->IY);

    //unit->Frame= (unit->Frame&128) + (unit->Frame%5);
    unit->Frame+=animation[state].Frame;
    unit->IX+=animation[state].Pixel;
    unit->IY+=animation[state].Pixel;
    unit->Wait=animation[state].Sleep;

    if( (animation[state].Frame || animation[state].Pixel)
	    && UnitVisible(unit) ) {
	MustRedraw|=RedrawMap;
    }

    flags=animation[state].Flags;
    if( flags&AnimationReset ) {
	unit->Reset=1;
    }
    if( flags&AnimationRestart ) {
	unit->State=0;
    } else {
	++unit->State;
    }

    return flags;
}

/*----------------------------------------------------------------------------
--	Globals
----------------------------------------------------------------------------*/

/**
**	Handle the action of an unit.
*/
local void HandleUnitAction(Unit* unit)
{
    int reset;

    //
    //	If current action is breakable proceed with next one.
    //
    if( (reset=unit->Reset) ) {
	unit->Reset=0;

	//
	//	New command and forced or old ready
	//
	if( unit->NextCount
		&& (unit->Command.Action == UnitActionStill || unit->NextFlush)
		&& !unit->Removed ) {
	    int z;

	    unit->NextFlush = 0;
	    //	Structure assign
	    unit->Command=unit->NextCommand[0];
	    //Next line shouldn't affect human players, but needed for AI player
	    unit->NextCommand[0].Action = UnitActionStill;
	    // cade: shift queue
	    unit->NextCount--;
	    for ( z = 0; z < unit->NextCount; z++ ) {
		unit->NextCommand[z] = unit->NextCommand[z+1];
	    }

	    unit->SubAction=0;
	    unit->State=0;

	    unit->Wait=1;

	    if( IsSelected(unit) ) {	// update display for new action
		UpdateButtonPanel();
		MustRedraw|=RedrawButtonPanel;
	    }
	}
    }

    //
    //	Select action.
    //
    switch( unit->Command.Action ) {
	case UnitActionNone:
	    DebugLevel1("FIXME: Should not happen!\n");
	    break;

	case UnitActionStill:
	    HandleActionStill(unit);
	    break;

	case UnitActionStandGround:
	    HandleActionStandGround(unit);
	    break;

	case UnitActionMove:		// THE HARD ONE
	    HandleActionMove(unit);
	    break;

	case UnitActionPatrol:
	    HandleActionPatrol(unit);
	    break;

	case UnitActionRepair:
	    HandleActionRepair(unit);
	    break;

	case UnitActionAttack:
	    HandleActionAttack(unit);
	    break;

	case UnitActionBoard:
	    HandleActionBoard(unit);
	    break;

	case UnitActionUnload:
	    HandleActionUnload(unit);
	    break;

	case UnitActionDie:
	    HandleActionDie(unit);
	    break;

	case UnitActionTrain:
	    HandleActionTrain(unit);
	    break;

	case UnitActionUpgradeTo:
	    HandleActionUpgradeTo(unit);
	    break;

	case UnitActionResearch:
	    HandleActionResearch(unit);
	    break;

	case UnitActionBuild:
	    HandleActionBuild(unit);
	    break;

	case UnitActionBuilded:
	    HandleActionBuilded(unit);
	    break;

	case UnitActionHarvest:
	    HandleActionHarvest(unit);
	    break;

	case UnitActionMineGold:
	    HandleActionMineGold(unit);
	    break;

	case UnitActionHaulOil:
	    HandleActionHaulOil(unit);
	    break;

	case UnitActionReturnGoods:
	    HandleActionReturnGoods(unit);
	    break;

	case UnitActionDemolish:
	    HandleActionDemolish(unit);
	    break;

	default:
	    DebugLevel1(__FUNCTION__": Unknown action %d\n"
		    ,unit->Command.Action);
	    break;
    }
}

/**
**	Update the actions of all units each frame.
**	IDEA:	to improve the preformance use slots for waiting.
*/
global void UnitActions(void)
{
    Unit* unit;
    int i;

    UnitConflicts();			// start attacking

    //
    // Do all actions
    //
    for( i=0; i<NumUnits; i++ ) {
	unit=Units[i];
	if( --unit->Wait ) {		// Wait until counter reached
	    continue;
	}
	HandleUnitAction(unit);
    }
}

//@}
