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
#include <time.h>

#include "freecraft.h"
#include "video.h"
#include "unittype.h"
#include "player.h"
#include "unit.h"
#include "actions.h"
#include "interface.h"
#include "map.h"

/*----------------------------------------------------------------------------
--	Variables
----------------------------------------------------------------------------*/

global unsigned SyncHash;	/// Hash calculated to find sync failures

/*----------------------------------------------------------------------------
--	Functions
----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
--	Animation
----------------------------------------------------------------------------*/

/**
**	Show unit animation.
**		Returns animation flags.
**
**	@param unit		Unit of the animation.
**	@param animation	Animation script to handle.
**
**	@return			The flags of the current script step.
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
    DebugLevel3("Heading %d +%d,%d\n",unit->Direction,unit->IX,unit->IY);

    unit->Frame+=animation[state].Frame;
    unit->IX+=animation[state].Pixel;
    unit->IY+=animation[state].Pixel;
    unit->Wait=animation[state].Sleep;
    if( unit->Slow ) {			// unit is slowed down
	unit->Wait<<=1;
    }
    if( unit->Haste && unit->Wait>1 ) {	// unit is accelerated
	unit->Wait>>=1;
    }

    // Anything changed the display?
    if( (animation[state].Frame || animation[state].Pixel) ) {
        CheckUnitToBeDrawn(unit);
    }

    flags=animation[state].Flags;
    if( flags&AnimationReset ) {	// Reset can check for other actions
	unit->Reset=1;
    }
    if( flags&AnimationRestart ) {	// Restart animation script
	unit->State=0;
    } else {
	++unit->State;			// Advance to next script
    }

    return flags;
}

/*----------------------------------------------------------------------------
--	Actions
----------------------------------------------------------------------------*/

/**
**	Unit does nothing!
**
**	@param unit	Unit pointer for none action.
*/
local void HandleActionNone(Unit* unit)
{
    DebugLevel1Fn("FIXME: Should not happen!\n");
    DebugLevel1Fn("FIXME: Unit (%Zd) %s has action none.!\n",
	    UnitNumber(unit),unit->Type->Ident);
}

/**
**	Unit has notwritten function.
**
**	@param unit	Unit pointer for notwritten action.
*/
local void HandleActionNotWritten(Unit* unit)
{
    DebugLevel1Fn("FIXME: Not written!\n");
    DebugLevel1Fn("FIXME: Unit (%Zd) %s has action %d.!\n",
	    UnitNumber(unit),unit->Type->Ident,unit->Orders[0].Action);
}

/**
**	Jump table for actions.
**
**	@note can move function into unit structure.
*/
local void (*HandleActionTable[256])(Unit*) = {
    HandleActionNone,
    HandleActionStill,
    HandleActionStandGround,
    HandleActionFollow,
    HandleActionMove,
    HandleActionAttack,
    HandleActionAttack,		// HandleActionAttackGround,
    HandleActionDie,
    HandleActionSpellCast,
    HandleActionTrain,
    HandleActionUpgradeTo,
    HandleActionResearch,
    HandleActionBuilded,
    HandleActionBoard,
    HandleActionUnload,
    HandleActionPatrol,
    HandleActionBuild,
    HandleActionRepair,
    HandleActionHarvest,
    HandleActionMineGold,
    HandleActionNotWritten,	// HandleActionMineOre,
    HandleActionNotWritten,	// HandleActionMineCoal,
    HandleActionNotWritten,	// HandleActionQuarryStone,
    HandleActionHaulOil,
    HandleActionReturnGoods,
    HandleActionDemolish,

    // Enough for the future ?
    HandleActionNotWritten, HandleActionNotWritten,
    HandleActionNotWritten, HandleActionNotWritten, HandleActionNotWritten,
    HandleActionNotWritten, HandleActionNotWritten, HandleActionNotWritten,
    HandleActionNotWritten, HandleActionNotWritten, HandleActionNotWritten,
    HandleActionNotWritten, HandleActionNotWritten, HandleActionNotWritten,
    HandleActionNotWritten, HandleActionNotWritten, HandleActionNotWritten,
    HandleActionNotWritten, HandleActionNotWritten, HandleActionNotWritten,
    HandleActionNotWritten, HandleActionNotWritten, HandleActionNotWritten,
    HandleActionNotWritten, HandleActionNotWritten, HandleActionNotWritten,
    HandleActionNotWritten, HandleActionNotWritten, HandleActionNotWritten,
    HandleActionNotWritten, HandleActionNotWritten, HandleActionNotWritten,
    HandleActionNotWritten, HandleActionNotWritten, HandleActionNotWritten,
    HandleActionNotWritten, HandleActionNotWritten, HandleActionNotWritten,
    HandleActionNotWritten, HandleActionNotWritten, HandleActionNotWritten,
    HandleActionNotWritten, HandleActionNotWritten, HandleActionNotWritten,
    HandleActionNotWritten, HandleActionNotWritten, HandleActionNotWritten,
    HandleActionNotWritten, HandleActionNotWritten, HandleActionNotWritten,
    HandleActionNotWritten, HandleActionNotWritten, HandleActionNotWritten,
    HandleActionNotWritten, HandleActionNotWritten, HandleActionNotWritten,
    HandleActionNotWritten, HandleActionNotWritten, HandleActionNotWritten,
    HandleActionNotWritten, HandleActionNotWritten, HandleActionNotWritten,
    HandleActionNotWritten, HandleActionNotWritten, HandleActionNotWritten,
    HandleActionNotWritten, HandleActionNotWritten, HandleActionNotWritten,
    HandleActionNotWritten, HandleActionNotWritten, HandleActionNotWritten,
    HandleActionNotWritten, HandleActionNotWritten, HandleActionNotWritten,
    HandleActionNotWritten, HandleActionNotWritten, HandleActionNotWritten,
    HandleActionNotWritten, HandleActionNotWritten, HandleActionNotWritten,
    HandleActionNotWritten, HandleActionNotWritten, HandleActionNotWritten,
    HandleActionNotWritten, HandleActionNotWritten, HandleActionNotWritten,
    HandleActionNotWritten, HandleActionNotWritten, HandleActionNotWritten,
    HandleActionNotWritten, HandleActionNotWritten, HandleActionNotWritten,
    HandleActionNotWritten, HandleActionNotWritten, HandleActionNotWritten,
    HandleActionNotWritten, HandleActionNotWritten, HandleActionNotWritten,
    HandleActionNotWritten, HandleActionNotWritten, HandleActionNotWritten,
    HandleActionNotWritten, HandleActionNotWritten, HandleActionNotWritten,
    HandleActionNotWritten, HandleActionNotWritten, HandleActionNotWritten,
    HandleActionNotWritten, HandleActionNotWritten, HandleActionNotWritten,
    HandleActionNotWritten, HandleActionNotWritten, HandleActionNotWritten,
    HandleActionNotWritten, HandleActionNotWritten, HandleActionNotWritten,
    HandleActionNotWritten, HandleActionNotWritten, HandleActionNotWritten,
    HandleActionNotWritten, HandleActionNotWritten, HandleActionNotWritten,
    HandleActionNotWritten, HandleActionNotWritten, HandleActionNotWritten,
    HandleActionNotWritten, HandleActionNotWritten, HandleActionNotWritten,
    HandleActionNotWritten, HandleActionNotWritten, HandleActionNotWritten,
    HandleActionNotWritten, HandleActionNotWritten, HandleActionNotWritten,
    HandleActionNotWritten, HandleActionNotWritten, HandleActionNotWritten,
    HandleActionNotWritten, HandleActionNotWritten, HandleActionNotWritten,
    HandleActionNotWritten, HandleActionNotWritten, HandleActionNotWritten,
    HandleActionNotWritten, HandleActionNotWritten, HandleActionNotWritten,
    HandleActionNotWritten, HandleActionNotWritten, HandleActionNotWritten,
    HandleActionNotWritten, HandleActionNotWritten, HandleActionNotWritten,
    HandleActionNotWritten, HandleActionNotWritten, HandleActionNotWritten,
    HandleActionNotWritten, HandleActionNotWritten, HandleActionNotWritten,
    HandleActionNotWritten, HandleActionNotWritten, HandleActionNotWritten,
    HandleActionNotWritten, HandleActionNotWritten, HandleActionNotWritten,
    HandleActionNotWritten, HandleActionNotWritten, HandleActionNotWritten,
    HandleActionNotWritten, HandleActionNotWritten, HandleActionNotWritten,
    HandleActionNotWritten, HandleActionNotWritten, HandleActionNotWritten,
    HandleActionNotWritten, HandleActionNotWritten, HandleActionNotWritten,
    HandleActionNotWritten, HandleActionNotWritten, HandleActionNotWritten,
    HandleActionNotWritten, HandleActionNotWritten, HandleActionNotWritten,
    HandleActionNotWritten, HandleActionNotWritten, HandleActionNotWritten,
    HandleActionNotWritten, HandleActionNotWritten, HandleActionNotWritten,
    HandleActionNotWritten, HandleActionNotWritten, HandleActionNotWritten,
    HandleActionNotWritten, HandleActionNotWritten, HandleActionNotWritten,
    HandleActionNotWritten, HandleActionNotWritten, HandleActionNotWritten,
    HandleActionNotWritten, HandleActionNotWritten, HandleActionNotWritten,
    HandleActionNotWritten, HandleActionNotWritten, HandleActionNotWritten,
    HandleActionNotWritten, HandleActionNotWritten, HandleActionNotWritten,
    HandleActionNotWritten, HandleActionNotWritten, HandleActionNotWritten,
    HandleActionNotWritten, HandleActionNotWritten, HandleActionNotWritten,
    HandleActionNotWritten, HandleActionNotWritten, HandleActionNotWritten,
    HandleActionNotWritten, HandleActionNotWritten, HandleActionNotWritten,
    HandleActionNotWritten, HandleActionNotWritten, HandleActionNotWritten,
    HandleActionNotWritten, HandleActionNotWritten, HandleActionNotWritten,
    HandleActionNotWritten, HandleActionNotWritten, HandleActionNotWritten,
    HandleActionNotWritten, HandleActionNotWritten, HandleActionNotWritten,
};

/**
**	Handle the action of an unit.
**
**	@param unit	Pointer to handled unit.
*/
local void HandleUnitAction(Unit* unit)
{
    int z;

    //
    //	If current action is breakable proceed with next one.
    //
    if( unit->Reset ) {
	unit->Reset=0;
	//
	//	o Look if we have a new order and old finished.
	//	o Or the order queue should be flushed.
	//
	if( unit->OrderCount>1
		&& (unit->Orders[0].Action==UnitActionStill || unit->OrderFlush)
		) {

	    if( unit->Removed ) {	// FIXME: johns I see this as an error
		DebugLevel0Fn("Flushing removed unit\n");
		// This happens, if building with ALT+SHIFT.
		return;
	    }

	    //
	    //	Release pending references.
	    //
	    if( unit->Orders[0].Goal ) {
		// Still shouldn't have a reference
		DebugCheck( unit->Orders[0].Action==UnitActionStill );
		RefsDebugCheck( !unit->Orders[0].Goal->Refs );
		if( !--unit->Orders[0].Goal->Refs ) {
		    ReleaseUnit(unit->Orders[0].Goal);
		}
	    }

	    //
	    //	Shift queue with structure assignment.
	    //
	    unit->OrderCount--;
	    unit->OrderFlush=0;
	    for ( z = 0; z < unit->OrderCount; z++ ) {
		unit->Orders[z] = unit->Orders[z+1];
	    }

	    //
	    //	Note subaction 0 should reset.
	    //
	    unit->SubAction=unit->State=0;
	    unit->Wait=1;

	    if( IsSelected(unit) ) {	// update display for new action
		UpdateButtonPanel();
		MustRedraw|=RedrawInfoPanel;
	    }
	}
    }

    // FIXME: fire handling should be moved to here.

    //
    //	Select action. FIXME: should us function pointers in unit structure.
    //
    HandleActionTable[unit->Orders[0].Action](unit);
}

/**
**	Update the actions of all units each frame.
**
**	IDEA:	to improve the preformance use slots for waiting.
*/
global void UnitActions(void)
{
    Unit** table;
    Unit** tend;
    Unit* unit;

    //
    //	Must copy table, units could be removed.
    //
    table=alloca(sizeof(Unit)*NumUnits);
    tend=table+NumUnits;
    memcpy(table,Units,sizeof(Unit)*NumUnits);

    //
    //	Do all actions
    //
    for( ; table<tend; table++ ) {
	unit=*table;

#if defined(UNIT_ON_MAP) && 0		// debug unit store
	 { const Unit* list;

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
			&& !unit->Orders[0].Action==UnitActionDie) ) {
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
	} }
#endif

	if( --unit->Wait ) {		// Wait until counter reached
	    continue;
	}
	HandleUnitAction(unit);
	DebugCheck( *table!=unit );	// Removed is evil.

#ifdef no_DEBUG
	//
	//	Dump the unit to find the network unsyncron bug.
	//
	{
	static FILE* logf;

	if( !logf ) {
	    time_t now;
	    char buf[256];

	    sprintf(buf,"log_of_fc_%d.log",ThisPlayer->Player);
	    logf=fopen(buf,"wb");
	    if( !logf ) {
		return;
	    }
	    fprintf(logf,";;; Log file generated by FreeCraft Version "
		    VERSION "\n");
	    time(&now);
	    fprintf(logf,";;;\tDate: %s",ctime(&now));
	    fprintf(logf,";;;\tMap: %s\n\n",TheMap.Description);
	}

	fprintf(logf,"%d: ",FrameCounter);
	fprintf(logf,"%Zd %s S%d/%d-%d P%d Refs %d\n",
	    UnitNumber(unit),unit->Type ? unit->Type->Ident : "unit-killed",
		unit->State,unit->SubAction,
		unit->Orders[0].Action,
		unit->Player->Player,unit->Refs);
		
	// SaveUnit(unit,logf);
	fflush(NULL);
	}
#endif
	//
	//	Calculate some hash.
	//
	SyncHash=(SyncHash<<5)|(SyncHash>>27);
	SyncHash^=unit->Orders[0].Action<<18;
	SyncHash^=unit->State<<12;
	SyncHash^=unit->SubAction<<6;
	SyncHash^=unit->Refs<<3;
    }
}

//@}
