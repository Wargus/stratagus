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
/**@name action_research.c -	The research action. */
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
#include "video.h"
#include "sound_id.h"
#include "unitsound.h"
#include "unittype.h"
#include "player.h"
#include "unit.h"
#include "actions.h"
#include "upgrade_structs.h"
#include "upgrade.h"
#include "interface.h"

/*----------------------------------------------------------------------------
--	Functions
----------------------------------------------------------------------------*/

/**
**	Unit researches!
**
**	@param unit	Pointer of researching unit.
*/
global void HandleActionResearch(Unit* unit)
{
    Upgrade* upgrade;

    DebugLevel3("Research %Zd\n",UnitNumber(unit));

#ifdef NEW_ORDERS
    if( !unit->SubAction ) {		// first entry
	unit->SubAction=1;
	unit->Data.Research.Ticks=0;
	unit->Data.Research.Upgrade=unit->Orders[0].Arg1;
    }
    upgrade=unit->Data.Research.Upgrade;
    unit->Data.Research.Ticks+=SpeedResearch;

    if( unit->Data.Research.Ticks>=upgrade->Costs[TimeCost] ) {
#else
    upgrade=unit->Command.Data.Research.What;
    unit->Command.Data.Research.Ticks+=SpeedResearch;

    if( unit->Command.Data.Research.Ticks>=upgrade->Costs[TimeCost] ) {
#endif

	// FIXME: should als speak and tell ai. generic notify/message.
	if( unit->Player==ThisPlayer ) {
	    SetMessage2(unit->X, unit->Y, "%s: Upgrade complete"
		,unit->Type->Name );
	} else {
	    // FIXME: AiUpgradeToComplete(unit,type);
	}
        UpgradeAcquire(unit->Player,upgrade);

	unit->Reset=unit->Wait=1;
#ifdef NEW_ORDERS
	unit->Orders[0].Action=UnitActionStill;
#else
	unit->Command.Action=UnitActionStill;
#endif
	unit->SubAction=0;

	// Upgrade can change all
	UpdateButtonPanel();
	MustRedraw|=RedrawPanels;

	return;
    }

    if( IsSelected(unit) ) {
	MustRedraw|=RedrawInfoPanel;
    }

    unit->Reset=1;
    unit->Wait=FRAMES_PER_SECOND/6;

    // FIXME: should be animations here?
}

//@}
