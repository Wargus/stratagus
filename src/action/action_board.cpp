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
**	@returns	>0 remaining path length, 0 wait for path, -1
**			reached goal, -2 can't reach the goal.
*/
local int MoveToTransporter(Unit* unit)
{
    int i;

    i=HandleActionMove(unit);
#ifdef NEW_ORDERS
    unit->Orders[0].Action=UnitActionBoard;
#else
    unit->Command.Action=UnitActionBoard;
#endif
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

#ifdef NEW_ORDERS
    trans=unit->Orders[0].Goal;
#else
    trans=unit->Command.Data.Move.Goal;
#endif
    // FIXME: destination destroyed??
    if( !trans || !trans->Type->Transporter ) {
	DebugLevel3Fn("TRANSPORTER NOT REACHED %d,%d\n",unit->X,unit->Y);
	return 0;
    }
    if( trans->Destroyed ) {
	DebugLevel0Fn("Destroyed unit\n");
#ifdef REFS_DEBUG
	DebugCheck( !trans->Refs );
#endif
	if( !--trans->Refs ) {
	    ReleaseUnit(trans);
	}
#ifdef NEW_ORDERS
	unit->Orders[0].Goal=trans=NoUnitP;
#else
	unit->Command.Data.Move.Goal=trans=NoUnitP;
#endif
	return 0;
    } else if( trans->Removed ||
#ifdef NEW_ORDERS
	    !trans->HP || trans->Orders[0].Action==UnitActionDie ) {
#else
	    !trans->HP || trans->Command.Action==UnitActionDie ) {
#endif
#ifdef REFS_DEBUG
	DebugCheck( !trans->Refs );
#endif
	--trans->Refs;
#ifdef REFS_DEBUG
	DebugCheck( !trans->Refs );
#endif
#ifdef NEW_ORDERS
	unit->Orders[0].Goal=trans=NoUnitP;
#else
	unit->Command.Data.Move.Goal=trans=NoUnitP;
#endif
	return 0;
    }

    if( MapDistanceToUnit(unit->X,unit->Y,trans)==1 ) {
	DebugLevel3Fn("Enter transporter\n");
	return 1;
    }

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
#ifdef NEW_ORDERS
    unit->Orders[0].Action=UnitActionStill;
#else
    unit->Command.Action=UnitActionStill;
#endif
    unit->SubAction=0;

#ifdef NEW_ORDERS
    transporter=unit->Orders[0].Goal;
#else
    transporter=unit->Command.Data.Move.Goal;
#endif
    if( transporter->Destroyed ) {
	DebugLevel0Fn("Destroyed unit\n");
#ifdef REFS_DEBUG
	DebugCheck( !transporter->Refs );
#endif
	if( !--transporter->Refs ) {
	    ReleaseUnit(transporter);
	}
#ifdef NEW_ORDERS
	unit->Orders[0].Goal=NoUnitP;
#else
	unit->Command.Data.Move.Goal=NoUnitP;
#endif
	return;
    } else if( transporter->Removed ||
#ifdef NEW_ORDERS
	    !transporter->HP || transporter->Orders[0].Action==UnitActionDie ) {
#else
	    !transporter->HP || transporter->Command.Action==UnitActionDie ) {
#endif
#ifdef REFS_DEBUG
	DebugCheck( !transporter->Refs );
#endif
	--transporter->Refs;
#ifdef REFS_DEBUG
	DebugCheck( !transporter->Refs );
#endif
#ifdef NEW_ORDERS
	unit->Orders[0].Goal=NoUnitP;
#else
	unit->Command.Data.Move.Goal=NoUnitP;
#endif
	return;
    }
#ifdef REFS_DEBUG
    DebugCheck( !transporter->Refs );
#endif
    --transporter->Refs;
#ifdef REFS_DEBUG
    DebugCheck( !transporter->Refs );
#endif
#ifdef NEW_ORDERS
    unit->Orders[0].Goal=NoUnitP;
#else
    unit->Command.Data.Move.Goal=NoUnitP;
#endif

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
**	FIXME: the transporter must drive to the meating point.
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
        default:
	    if( unit->SubAction<=200 ) {
		// FIXME: if near transporter wait for enter
		if( (i=MoveToTransporter(unit)) ) {
		    if( i==PF_UNREACHABLE ) {
			if( ++unit->SubAction==200 ) {
#ifdef NEW_ORDERS
			    unit->Orders[0].Action=UnitActionStill;
			    if( unit->Orders[0].Goal ) {

#ifdef REFS_DEBUG
				DebugCheck(!unit->Orders[0].Goal->Refs);
#endif
				--unit->Orders[0].Goal->Refs;
#ifdef REFS_DEBUG
				DebugCheck(!unit->Orders[0].Goal->Refs);
#endif
				unit->Orders[0].Goal=NoUnitP;
			    }
#else
			    unit->Command.Action=UnitActionStill;
			    if( unit->Command.Data.Move.Goal ) {

#ifdef REFS_DEBUG
				DebugCheck(!unit->Command.Data.Move.Goal->Refs);
#endif
				--unit->Command.Data.Move.Goal->Refs;
#ifdef REFS_DEBUG
				DebugCheck(!unit->Command.Data.Move.Goal->Refs);
#endif
				unit->Command.Data.Move.Goal=NoUnitP;
			    }
#endif
			    unit->SubAction=0;
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
