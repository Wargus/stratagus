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
/**@name action_follow.c -	The follow action. */
//
//	(c) Copyright 2001 by Lutz Sammer
//
//	FreeCraft is free software; you can redistribute it and/or modify
//	it under the terms of the GNU General Public License as published
//	by the Free Software Foundation; either version 2 of the License,
//	or (at your option) any later version.
//
//	FreeCraft is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//	GNU General Public License for more details.
//
//	$Id$

//@{

/*----------------------------------------------------------------------------
--	Includes
----------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>

#include "freecraft.h"
#include "unit.h"
#include "pathfinder.h"
#include "interface.h"
#include "actions.h"

/*----------------------------------------------------------------------------
--	Variables
----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
--	Function
----------------------------------------------------------------------------*/

/**
**	Unit follow action:
**
**	@param unit	Pointer to unit.
*/
global void HandleActionFollow(Unit* unit)
{
    Unit* goal;

    DebugLevel3Fn("%d: %d %d,%d \n",UnitNumber(unit),
	    unit->Orders[0].Goal ? UnitNumber(unit->Orders[0].Goal) : -1,
	    unit->Orders[0].X,unit->Orders[0].Y);

    //
    //	Reached target
    //
    if( unit->SubAction==128 ) {
	goal=unit->Orders[0].Goal;
	if( goal->Destroyed || !goal->HP
		|| goal->Orders[0].Action==UnitActionDie ) {
	    DebugLevel0Fn("Goal dead\n");
	    RefsDebugCheck( !goal->Refs );
	    if( !--goal->Refs && goal->Destroyed ) {
		ReleaseUnit(goal);
	    }
	    unit->Orders[0].Goal=NoUnitP;
	    unit->Wait=1;
	    unit->SubAction=0;
	    unit->Orders[0].Action=UnitActionStill;
	    if( IsOnlySelected(unit) ) { // update display for new action
		UpdateButtonPanel();
	    }
	    return;
	}
	if( goal->X==unit->Orders[0].X && goal->Y==unit->Orders[0].Y ) {
	    unit->Reset=1;
	    unit->Wait=10;
	    return;
	}
	unit->SubAction=0;
    }

    if( !unit->SubAction ) {		// first entry
	unit->SubAction=1;
	NewResetPath(unit);
	DebugCheck( unit->State!=0 );
    }

    switch( DoActionMove(unit) ) {	// reached end-point?
	case PF_UNREACHABLE:
	    //
	    //	Some tries to reach the goal
	    //
	    if( unit->SubAction++<10 ) {
		//	To keep the load low, retry delayed.
		unit->Wait=FRAMES_PER_SECOND/10+unit->SubAction;
		break;
	    }
	    // FALL THROUGH
	case PF_REACHED:
	    if( !(goal=unit->Orders[0].Goal) ) {// goal has died
		unit->Wait=1;
		unit->SubAction=0;
		unit->Orders[0].Action=UnitActionStill;
		if( IsOnlySelected(unit) ) { // update display for new action
		    UpdateButtonPanel();
		}
		return;
	    }
	    unit->Orders[0].X=goal->X;
	    unit->Orders[0].Y=goal->Y;
	    unit->SubAction=128;

	    // FALL THROUGH
	default:
	    break;
    }

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

    if( unit->Reset ) {
	//
	//	If our leader is dead or stops or attacks:
	//	Attack any enemy in reaction range.
	//		If don't set the goal, the unit can than choose a
	//		better goal if moving nearer to enemy.
	//
	if( unit->Type->CanAttack && !unit->Type->Tower
		&& (!goal || goal->Orders[0].Action==UnitActionAttack
		    || goal->Orders[0].Action==UnitActionStill) ) {
	    goal=AttackUnitsInReactRange(unit);
	    if( goal ) {
		DebugLevel2Fn("Follow attack %d\n",UnitNumber(goal));
		CommandAttack(unit,goal->X,goal->Y,NULL,FlushCommands);
		// Save current command to come back.
		unit->SavedOrder=unit->Orders[0];
		// This stops the follow command and the attack is executed
		unit->Orders[0].Action=UnitActionStill;
		unit->Orders[0].Goal=NoUnitP;
		unit->SubAction=0;
		unit->Wait=1;
	    }
	}
    }
}

//@}
