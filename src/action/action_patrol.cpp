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
/**@name action_patrol.c	-	The patrol action. */
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
#include "unit.h"
#include "actions.h"
#include "pathfinder.h"

/*----------------------------------------------------------------------------
--	Functions
----------------------------------------------------------------------------*/

/**
**	Unit Patrol:
**		The unit patrols between two points.
**		Any enemy unit in reaction range is attacked.
**	FIXME:
**		Should do some tries to reach the end-points.
**		Should support patrol between more points!
**		Patrol between units.
**
**	@param unit	Patroling unit pointer.
*/
global void HandleActionPatrol(Unit* unit)
{
    const Unit* goal;

    if( HandleActionMove(unit)<0 ) {	// reached end-point or stop

#ifdef NEW_ORDERS
	Order order;

	unit->Orders[0].Action=UnitActionPatrol;

	//
	//	Swap the points.
	//
	order=unit->Orders[0];

	DebugLevel0Fn("FIXME: patrol not written\n");

	ResetPath(unit->Orders[0]);
#else
	unit->Command.Action=UnitActionPatrol;

	//	Swap the points:
	unit->Command.Data.Move.SX^=unit->Command.Data.Move.DX;
	unit->Command.Data.Move.DX^=unit->Command.Data.Move.SX;
	unit->Command.Data.Move.SX^=unit->Command.Data.Move.DX;

	unit->Command.Data.Move.SY^=unit->Command.Data.Move.DY;
	unit->Command.Data.Move.DY^=unit->Command.Data.Move.SY;
	unit->Command.Data.Move.SY^=unit->Command.Data.Move.DY;

	ResetPath(unit->Command);
#endif
    }

    if( unit->Reset ) {
	//
	//	Attack any enemy in reaction range.
	//		If don't set the goal, the unit can than choose a
	//		better goal if moving nearer to enemy.
	//
	if( unit->Type->CanAttack && !unit->Type->Tower ) {
	    goal=AttackUnitsInReactRange(unit);
	    if( goal ) {
		DebugLevel0("Patrol attack %Zd\n",UnitNumber(goal));
		CommandAttack(unit,goal->X,goal->Y,NULL,FlushCommands);
		// Save current command to come back.
#ifdef NEW_ORDERS
		unit->SavedOrder=unit->Orders[0];
		unit->SavedOrder.Action=UnitActionPatrol;
#else
		unit->SavedCommand=unit->Command;
		unit->SavedCommand.Action=UnitActionPatrol;
#endif
	    }
	}
    }
}

//@}
