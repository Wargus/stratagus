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
/*
**	(c) Copyright 1998,2000 by Lutz Sammer
**
**	$Id$
*/

//@{

/*----------------------------------------------------------------------------
--	Includes
----------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>

#include "freecraft.h"
#include "video.h"
#include "sound_id.h"
#include "unitsound.h"
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
**
**	@return		True the unit has died.
*/
global int HandleActionDie(Unit* unit)
{
    //
    //	Show death animation
    //
    if( unit->Type->Animations && unit->Type->Animations->Die ) {
	UnitShowAnimation(unit,unit->Type->Animations->Die);
    } else {
	// some units has no death animation
	unit->Reset=1;
	unit->Wait=1;
    }

    //
    //	Die sequence terminated, generate corpse.
    //
    if( unit->Reset ) {
	DebugLevel3("Die complete %Zd\n",UnitNumber(unit));
	if( !unit->Type->CorpseType ) {
	    ReleaseUnit(unit);
	    return 1;
	}

	unit->State=unit->Type->CorpseScript;
	unit->Type=unit->Type->CorpseType;
	unit->Command.Action=UnitActionDie;
	if( unit->NextCount ) {
	    DebugLevel0(__FUNCTION__": NextCount = %d\n",unit->NextCount);
	}
	unit->NextCount=0;
	unit->SubAction=0;
	unit->Frame=0;
	UnitUpdateHeading(unit);
	DebugCheck( !unit->Type->Animations || !unit->Type->Animations->Die );
	UnitShowAnimation(unit,unit->Type->Animations->Die);

	// FIXME: perhaps later or never is better
	//ChangeUnitOwner(unit,unit->Player,&Players[PlayerNumNeutral]);
    }

    return 0;
}

//@}
