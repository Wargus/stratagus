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
/**@name actions.c	-	The actions. */
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

#include "freecraft.h"
#include "video.h"
#include "unitsound.h"
#include "unittype.h"
#include "player.h"
#include "unit.h"
#include "actions.h"
#include "interface.h"
#include "map.h"

/*----------------------------------------------------------------------------
--	Functions
----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
--	Animation
----------------------------------------------------------------------------*/

/**
**	Show unit animation.
**		Returns animation flags.
*/
global int UnitShowAnimation(Unit* unit,const Animation* animation)
{
    int state;
    int flags;

    if( !(state=unit->State) ) {
	unit->Frame=0;
	UnitUpdateHeading(unit);		// FIXME: remove this!!
    }

    DebugLevel3Fn("State %2d ",state);
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
--	Actions
----------------------------------------------------------------------------*/

/**
**	Handle the action of an unit.
*/
local void HandleUnitAction(Unit* unit)
{
    // FIXME: Johns: What should this do?
    if ( unit->Revealer ) {
#ifdef NEW_ORDERS
	unit->Orders[0].Action = UnitActionDie;
#else
	unit->Command.Action = UnitActionDie;
#endif
    }
    //
    //	If current action is breakable proceed with next one.
    //
    if( unit->Reset ) {
	unit->Reset=0;

#ifdef NEW_ORDERS

	//
	//	o Look if we have a new order and old finished.
	//	o Or the order queue should be flushed.
	//
	if( unit->OrderCount>1
		&& (unit->Orders[0].Action==UnitActionStill || unit->OrderFlush)
		) {
	    int z;

	    if( unit->Removed ) {	// FIXME: johns I see this as an error
		DebugLevel0Fn("Flushing removed unit\n");
	    }

	    //
	    //	Release pending references.
	    //
	    if( unit->Orders[0].Goal ) {
#ifdef REFS_DEBUG
		DebugCheck( !unit->Orders[0].Goal->Refs );
#endif
		if( !--unit->Orders[0].Goal->Refs ) {
		    ReleaseUnit(unit->Orders[0].Goal);
		}
		IfDebug( unit->Orders[0].Goal=NoUnitP; )
	    }

	    //
	    //	Old order has a saved order
	    //
	    if( unit->SavedOrder.Action!=UnitActionStill ) {
		//
		//	Release pending references.
		//
		if( unit->SavedOrder.Goal ) {
#ifdef REFS_DEBUG
		    DebugCheck( !unit->SavedOrder.Goal->Refs );
#endif
		    if( !--unit->SavedOrder.Goal->Refs ) {
			ReleaseUnit(unit->SavedOrder.Goal);
		    }
		}
		unit->SavedOrder.Action=UnitActionStill;
		unit->SavedOrder.Goal=NoUnitP;
	    }

	    //
	    //	Shift queue with structure assignment
	    //
	    unit->OrderCount--;
	    unit->OrderFlush=0;
	    for ( z = 0; z < unit->OrderCount; z++ ) {
		unit->Orders[z] = unit->Orders[z+1];
	    }

	    //
	    //	Must reset the new order.
	    //
	    // FIXME: ResetPath(&unit->Orders[0]);

	    unit->SubAction=0;
	    unit->State=0;
	    unit->Wait=1;

	    if( IsSelected(unit) ) {	// update display for new action
		UpdateButtonPanel();
		MustRedraw|=RedrawButtonPanel;
	    }
	}

#else

	//
	//	New command and forced or old ready (= still)
	//	FIXME: how should we deal with saved commands?
	//
	if( unit->NextCount
		&& (unit->Command.Action == UnitActionStill || unit->NextFlush)
		&& !unit->Removed ) {
	    int z;

#if 0
	    // FIXME: memory loose, can't fix now.
	    //
	    //	Release pending references.
	    //
	    if( unit->Command.Data.Move.Goal ) {
#ifdef REFS_DEBUG
		DebugCheck( !unit->Command.Data.Move.Goal->Refs-- );
#endif
		if !--unit->Command.Data.Move.Goal->Refs ) {
		    ReleaseUnit(unit->Command.Data.Move.Goal);
		}
	    }
#endif

	    //	Structure assign
	    unit->Command=unit->NextCommand[0];
	    // Next line shouldn't affect human players,
	    //	but needed for AI player
	    unit->NextCommand[0].Action=UnitActionStill;
	    // cade: shift queue
	    unit->NextCount--;
	    for ( z = 0; z < unit->NextCount; z++ ) {
		unit->NextCommand[z] = unit->NextCommand[z+1];
	    }

	    unit->NextFlush=0;

	    // Reset for new order
	    unit->SubAction=0;
	    unit->State=0;
	    unit->Wait=1;

	    if( IsSelected(unit) ) {	// update display for new action
		UpdateButtonPanel();
		MustRedraw|=RedrawButtonPanel;
	    }
	}
#endif
    }

    // FIXME: fire handling should be moved to here.

    //
    //	Select action.
    //
#ifdef NEW_ORDERS
    switch( unit->Orders[0].Action ) {
#else
    switch( unit->Command.Action ) {
#endif
	case UnitActionNone:
	    DebugLevel1Fn("FIXME: Should not happen!\n");
	    break;

	case UnitActionStill:
	    HandleActionStill(unit);
	    break;

	case UnitActionStandGround:
	    HandleActionStandGround(unit);
	    break;

	case UnitActionFollow:		// FIXME: not written
	    //HandleActionFollow(unit);
	    //break;

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
	case UnitActionAttackGround:
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

	case UnitActionSpellCast:
	    HandleActionSpellCast(unit);
	    break;

	default:
#ifdef NEW_ORDERS
	    DebugLevel1Fn("Unknown action %d\n",unit->Orders[0].Action);
#else
	    DebugLevel1Fn("Unknown action %d\n",unit->Command.Action);
#endif
	    break;
    }
}

/**
**	Update the actions of all units each frame.
**
**	IDEA:	to improve the preformance use slots for waiting.
*/
global void UnitActions(void)
{
    Unit** table;

    //
    // Do all actions
    //
    for( table=Units; table<Units+NumUnits; table++ ) {
#if defined(UNIT_ON_MAP) && 0		// debug unit store
	Unit* list;
	Unit* unit;

	unit=*table;
	list=TheMap.Fields[unit->Y*TheMap.Width+unit->X].Here.Units;
	while( list ) {				// find the unit
	    if( list==unit ) {
		break;
	    }
	    list=list->Next;
	}
	if( !unit->Removed ) {
	    if( !list
		    && (!unit->Type->Vanishes
			&& !unit->Command.Action==UnitActionDie) ) {
		DebugLevel0Fn("!removed not on map %Zd\n",UnitNumber(unit));
		abort();
	    }
	} else if( list ) {
	    DebugLevel0Fn("remove on map %Zd\n",UnitNumber(unit));
	    abort();
	}
	list=unit->Next;
	while( list ) {
	    if( list->X!=unit->X || list->Y!=unit->Y ) {
		DebugLevel0Fn("Wrong X,Y %Zd %d,%d\n",UnitNumber(list)
			,list->X,list->Y);
		abort();
	    }
	    list=list->Next;
	}
#endif
	if( --(*table)->Wait ) {	// Wait until counter reached
	    continue;
	}
	HandleUnitAction(*table);
    }
}

//@}
