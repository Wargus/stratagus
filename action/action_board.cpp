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
/**@name action_board.c	-	The board action. */
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
#include "unittype.h"
#include "player.h"
#include "unit.h"
#include "actions.h"
#include "interface.h"
#include "pathfinder.h"

/*----------------------------------------------------------------------------
--	Functions
----------------------------------------------------------------------------*/

/**
**	Move to transporter.
**
**	@param unit	Pointer to unit, moving to transporter.
**
**	@return		>0 remaining path length, 0 wait for path, -1
**			reached goal, -2 can't reach the goal.
*/
local int MoveToTransporter(Unit* unit)
{
    int i;

    i=DoActionMove(unit);
    // New code has this as default.
    DebugCheck( unit->Orders[0].Action!=UnitActionBoard );
    return i;
}

/**
**	Wait for transporter.
**
**	@param unit	Pointer to unit.
**	@return		True if ship arrived/present, False otherwise.
*/
local int WaitForTransporter(Unit* unit)
{
    Unit* trans;

    unit->Wait=6;
    unit->Reset=1;

    trans=unit->Orders[0].Goal;

    if( !trans || !trans->Type->Transporter ) {
	// FIXME: destination destroyed??
	DebugLevel2Fn("TRANSPORTER NOT REACHED %d,%d\n",unit->X,unit->Y);
	return 0;
    }

    if( trans->Destroyed ) {
	DebugLevel0Fn("Destroyed transporter\n");
	RefsDebugCheck( !trans->Refs );
	if( !--trans->Refs ) {
	    ReleaseUnit(trans);
	}
	unit->Orders[0].Goal=NoUnitP;
	return 0;
    } else if( trans->Removed ||
	    !trans->HP || trans->Orders[0].Action==UnitActionDie ) {
	DebugLevel0Fn("Unusable transporter\n");
	RefsDebugCheck( !trans->Refs );
	--trans->Refs;
	RefsDebugCheck( !trans->Refs );
	unit->Orders[0].Goal=NoUnitP;
	return 0;
    }

    if( MapDistanceToUnit(unit->X,unit->Y,trans)==1 ) {
	DebugLevel3Fn("Enter transporter\n");
	return 1;
    }

    //
    //	FIXME: any enemies in range attack them, while waiting.
    //

    DebugLevel2Fn("TRANSPORTER NOT REACHED %d,%d\n",unit->X,unit->Y);

    return 0;
}

/**
**	Enter the transporter.
**
**	@param unit	Pointer to unit.
*/
local void EnterTransporter(Unit* unit)
{
    Unit* transporter;
    int i;

    unit->Wait=1;
    unit->Orders[0].Action=UnitActionStill;
    unit->SubAction=0;

    transporter=unit->Orders[0].Goal;
    if( transporter->Destroyed ) {
	DebugLevel0Fn("Destroyed transporter\n");
	RefsDebugCheck( !transporter->Refs );
	if( !--transporter->Refs ) {
	    ReleaseUnit(transporter);
	}
	unit->Orders[0].Goal=NoUnitP;
	return;
    } else if( transporter->Removed ||
	    !transporter->HP || transporter->Orders[0].Action==UnitActionDie ) {
	DebugLevel0Fn("Unuseable transporter\n");
	RefsDebugCheck( !transporter->Refs );
	--transporter->Refs;
	RefsDebugCheck( !transporter->Refs );
	unit->Orders[0].Goal=NoUnitP;
	return;
    }

    RefsDebugCheck( !transporter->Refs );
    --transporter->Refs;
    RefsDebugCheck( !transporter->Refs );
    unit->Orders[0].Goal=NoUnitP;

    //
    //	Find free slot in transporter.
    //
    for( i=0; i<sizeof(unit->OnBoard)/sizeof(*unit->OnBoard); ++i ) {
	if( transporter->OnBoard[i]==NoUnitP ) {
	    transporter->OnBoard[i]=unit;
	    transporter->Value++;
	    RemoveUnit(unit);
	    if( IsSelected(transporter) ) {
		UpdateButtonPanel();
		MustRedraw|=RedrawPanels;
	    }
	    return;
	}
    }
    DebugLevel0Fn("No free slot in transporter\n");
}

/**
**	The unit boards a transporter.
**
**	@todo FIXME: the transporter must drive to the meating point.
**		While waiting for the transporter the units must defend
**		themselfs.
**
**	@param unit	Pointer to unit.
*/
global void HandleActionBoard(Unit* unit)
{
    int i;

    DebugLevel3Fn("%p(%Zd) SubAction %d\n"
	    ,unit,UnitNumber(unit),unit->SubAction);

    switch( unit->SubAction ) {
	//
	//	Wait for transporter
	//
	case 201:
	    // FIXME: show still animations
	    DebugLevel3Fn("Waiting\n");
	    if( WaitForTransporter(unit) ) {
		unit->SubAction=202;
	    }
	    break;
	//
	//	Enter transporter
	//
	case 202:
	    EnterTransporter(unit);
	    break;
	//
	//	Move to transporter
	//
	case 0:
		NewResetPath(unit);
		unit->SubAction=1;
		// FALL THROUGH
        default:
	    if( unit->SubAction<=200 ) {
		// FIXME: if near transporter wait for enter
		if( (i=MoveToTransporter(unit)) ) {
		    if( i==PF_UNREACHABLE ) {
			if( ++unit->SubAction==200 ) {
			    unit->Orders[0].Action=UnitActionStill;
			    if( unit->Orders[0].Goal ) {

				RefsDebugCheck(!unit->Orders[0].Goal->Refs);
				--unit->Orders[0].Goal->Refs;
				RefsDebugCheck(!unit->Orders[0].Goal->Refs);
				unit->Orders[0].Goal=NoUnitP;
			    }
			    unit->SubAction=0;
			} else {
			    unit->Wait=unit->SubAction;
			}
		    } else if( i==PF_REACHED ) {
			unit->SubAction=201;
		    }
		}
	    }
	    break;
    }
}

//@}
