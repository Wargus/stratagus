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
#include "ai.h"

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
    const Upgrade* upgrade;

    if( !unit->SubAction ) {		// first entry
	upgrade=unit->Data.Research.Upgrade=unit->Orders[0].Arg1;
#if 0
	// FIXME: I want to support both, but with network we need this check
	//	  but if want combined upgrades this is worse

	//
	//	Check if an other building has already started?
	//
	if( unit->Player->UpgradeTimers.Upgrades[upgrade-Upgrades] ) {
	    DebugLevel0Fn("Two researches running\n");
	    PlayerAddCosts(unit->Player,upgrade->Costs);

	    unit->Reset=unit->Wait=1;
	    unit->Orders[0].Action=UnitActionStill;
	    unit->SubAction=0;
	    if( IsOnlySelected(unit) ) {
		MustRedraw|=RedrawInfoPanel;
	    }
	    return;
	}
#endif
	unit->SubAction=1;
    } else {
	upgrade=unit->Data.Research.Upgrade;
    }

    unit->Player->UpgradeTimers.Upgrades[upgrade-Upgrades]+=SpeedResearch;
    if( unit->Player->UpgradeTimers.Upgrades[upgrade-Upgrades]
		>=upgrade->Costs[TimeCost] ) {

	NotifyPlayer(unit->Player,NotifyGreen,unit->X,unit->Y,
		"%s: complete",unit->Type->Name);
	if( unit->Player->Ai ) {
	    AiResearchComplete(unit,upgrade);
	}
        UpgradeAcquire(unit->Player,upgrade);

	unit->Reset=unit->Wait=1;
	unit->Orders[0].Action=UnitActionStill;
	unit->SubAction=0;

	// Upgrade can change all
	UpdateButtonPanel();
	MustRedraw|=RedrawPanels;

	return;
    }

    if( IsOnlySelected(unit) ) {
	MustRedraw|=RedrawInfoPanel;
    }

    unit->Reset=1;
    unit->Wait=FRAMES_PER_SECOND/6;

    // FIXME: should be animations here?
}

//@}
