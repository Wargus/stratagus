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
#include "actions.h"
#include "pathfinder.h"

/*----------------------------------------------------------------------------
--	Functions
----------------------------------------------------------------------------*/

/**
**	Unit Patrol:
**		The unit patrols between two points.
**		Any enemy unit in reaction range is attacked.
**	@todo FIXME:
**		Should do some tries to reach the end-points.
**		Should support patrol between more points!
**		Patrol between units.
**
**	@param unit	Patroling unit pointer.
*/
global void HandleActionPatrol(Unit* unit)
{
    const Unit* goal;

    if( !unit->SubAction ) {		// first entry.
	NewResetPath(unit);
	unit->SubAction=1;
    }

    if( DoActionMove(unit)<0 ) {	// reached end-point or stop
	int tmp;

	DebugCheck( unit->Orders[0].Action!=UnitActionPatrol );

	//
	//	Swap the points.
	//
	tmp=(int)unit->Orders[0].Arg1;
	unit->Orders[0].Arg1=(void*)((unit->Orders[0].X<<16)|unit->Orders[0].Y);
	unit->Orders[0].X=tmp>>16;
	unit->Orders[0].Y=tmp&0xFFFF;

	NewResetPath(unit);
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
		Order order;

		DebugLevel3("Patrol attack %d\n",UnitNumber(goal));
		order=unit->Orders[0];
		CommandAttack(unit,goal->X,goal->Y,NULL,FlushCommands);
		// Save current command to come back.
		unit->SavedOrder=order;
		if( unit->SavedOrder.Goal ) {
		    RefsDebugCheck(!order.Goal->Refs || order.Goal->Destroyed);
		    order.Goal->Refs++;
		}
		unit->Orders[0].Action=UnitActionStill;
		unit->SubAction=0;
	    }
	}
    }
}

//@}
