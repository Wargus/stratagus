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
/**@name action_die.c	-	The die action. */
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

/*----------------------------------------------------------------------------
--	Functions
----------------------------------------------------------------------------*/

/**
**	Unit dies!
**
**	@param unit	The unit which dies.
*/
global void HandleActionDie(Unit* unit)
{
    //
    //	Show death animation
    //
    if( unit->Type->Animations && unit->Type->Animations->Die ) {
	UnitShowAnimation(unit,unit->Type->Animations->Die);
    } else {
	// some units has no death animation
	unit->Reset=unit->Wait=1;
    }

    //
    //	Die sequence terminated, generate corpse.
    //
    if( unit->Reset ) {
	DebugLevel3("Die complete %d\n",UnitNumber(unit));
	if( !unit->Type->CorpseType ) {
	    ReleaseUnit(unit);
	    return;
	}

	unit->State=unit->Type->CorpseScript;
	unit->Type=unit->Type->CorpseType;
	CommandStopUnit(unit);		// This clears all order queues
	IfDebug(
	    if( unit->Orders[0].Action!=UnitActionDie ) {
		DebugLevel0Fn("Reset to die is really needed\n");
	    }
	);
	unit->Orders[0].Action=UnitActionDie;
	--unit->OrderCount;		// remove the stop command
	unit->SubAction=0;
	unit->Frame=0;
	UnitUpdateHeading(unit);
	UnitShowAnimation(unit,unit->Type->Animations->Die);

	// FIXME: perhaps later or never is better
	//ChangeUnitOwner(unit,unit->Player,&Players[PlayerNumNeutral]);
    }
}

//@}
