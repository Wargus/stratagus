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
/**@name action_upgradeto.c -	The unit upgrading to new action. */
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
#include "player.h"
#include "unittype.h"
#include "unit.h"
#include "actions.h"
#include "ai.h"
#include "interface.h"

/*----------------------------------------------------------------------------
--	Functions
----------------------------------------------------------------------------*/

/**
**	Unit upgrades unit!
**
**	@param unit	Pointer to unit.
*/
global void HandleActionUpgradeTo(Unit* unit)
{
    Player* player;
    UnitType* type;
    const UnitStats* stats;

    DebugLevel3Fn(" %d\n",UnitNumber(unit));

    player=unit->Player;
    if( !unit->SubAction ) {		// first entry
	unit->Data.UpgradeTo.Ticks=0;
	unit->SubAction=1;
    }
    type=unit->Orders[0].Type;
    stats=&type->Stats[player->Player];

    // FIXME: Should count down here
    unit->Data.UpgradeTo.Ticks+=SpeedUpgrade;
    if( unit->Data.UpgradeTo.Ticks>=stats->Costs[TimeCost] ) {

	unit->HP+=stats->HitPoints-unit->Type->Stats[player->Player].HitPoints;
	// don't have such unit now
	player->UnitTypesCount[unit->Type->Type]--;
	unit->Type=type;
	unit->Stats=(UnitStats*)stats;
	// and we have new one...
	player->UnitTypesCount[unit->Type->Type]++;
	UpdateForNewUnit(unit,1);

	// FIXME: SendNotify("upgrade-complete");
	if( player==ThisPlayer ) {
	    SetMessage2( unit->X, unit->Y, "Upgrade to %s complete",
		    unit->Type->Name );
	} else if( unit->Player->Ai ) {
	    AiUpgradeToComplete(unit,type);
	}
	unit->Reset=unit->Wait=1;
	unit->Orders[0].Action=UnitActionStill;
	unit->SubAction=0;

	//
	//	Update possible changed buttons.
	//
	if( IsOnlySelected(unit) ) {
	    UpdateButtonPanel();
	    MustRedraw|=RedrawPanels;
	} else if( player==ThisPlayer ) {
	    UpdateButtonPanel();
	    MustRedraw|=RedrawInfoPanel;
	}

	return;
    }

    if( IsOnlySelected(unit) ) {
	MustRedraw|=RedrawInfoPanel;
    }

    unit->Reset=1;
    unit->Wait=FRAMES_PER_SECOND/6;
}

//@}
