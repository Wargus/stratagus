/*
**	A clone of a famous game.
*/
/**@name action_patrol.c	-	The patrol action. */
/*
**	(c) Copyright 1998 by Lutz Sammer
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

/*
**	Unit Patrol:
**		The unit patrols between two points.
**		Any enemy unit in reaction range is attacked.
**	FIXME:
**		If the attack is completed, resume patrol!
**  		Should do some tries to reach the end-points.
**		Should support patrol between more points!
*/
global void HandleActionPatrol(Unit* unit)
{
    int t;
    const Unit* goal;

    if( HandleActionMove(unit) ) {	// reached end-point

	//	Swap the points:

	unit->Command.Action=UnitActionPatrol;
	unit->Command.Data.Move.Fast=1;
	t=unit->Command.Data.Move.SX;
	unit->Command.Data.Move.SX=unit->Command.Data.Move.DX;
	unit->Command.Data.Move.DX=t;

	t=unit->Command.Data.Move.SY;
	unit->Command.Data.Move.SY=unit->Command.Data.Move.DY;
	unit->Command.Data.Move.DY=t;
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
		CommandAttack(unit,goal->X,goal->Y,NULL,1);
	    }
	}
    }
}

//@}
