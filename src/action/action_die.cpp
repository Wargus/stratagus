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
--	Variables
----------------------------------------------------------------------------*/

extern Animation ** UnitCorpse;

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
#if 0
    int type;

    type=unit->Type->Type;

    switch( type ) {
	case Unit1x1DestroyedPlace:
	case Unit2x2DestroyedPlace:
	case Unit3x3DestroyedPlace:
	case Unit4x4DestroyedPlace:
	case UnitDeadBody:
	    if( UnitCorpse[unit->SubAction] ) {
		UnitShowAnimation(unit,UnitCorpse[unit->SubAction]);
	    }
	    break;

	default:
	    if( unit->Type->Animations ) {
		UnitShowAnimation(unit,unit->Type->Animations->Die);
	    } else {
		DebugLevel0("FIXME: die animation missing\n");
		unit->Reset=1;
		unit->Wait=1;
	    }
	    break;
    }
#endif
    //
    //	Show death animation
    //
    if( unit->Type->Animations ) {
	UnitShowAnimation(unit,unit->Type->Animations->Die);
    } else {
	DebugLevel0("FIXME: die animation missing\n");
	unit->Reset=1;
	unit->Wait=1;
    }

    //
    //	Die sequence terminated, generate corpse.
    //
    if( unit->Reset ) {
	DebugLevel3("Die complete %Zd\n",UnitNumber(unit));
#if 0
	if( !UnitCorpse[type] ){
	    FreeUnitMemory(unit);
	    return 1;
	}
	unit->SubAction=type;
	unit->Type=UnitTypeByIdent("unit-dead-body");
	unit->State=0;
	unit->Reset=0;
	UnitNewHeading(unit);
#endif
	if( !unit->Type->CorpseType ) {
	    FreeUnitMemory(unit);
	    return 1;
	}

	unit->State=unit->Type->CorpseScript;
	unit->Type=unit->Type->CorpseType;
	unit->Command.Action=UnitActionDie;
	unit->SubAction=0;
	UnitNewHeading(unit);
	DebugCheck( !unit->Type->Animations || !unit->Type->Animations->Die );
	UnitShowAnimation(unit,unit->Type->Animations->Die);

	ChangeUnitOwner(unit,unit->Player,&Players[PlayerNumNeutral]);
    }

    return 0;
}

//@}
