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
/**@name action_train.c -	The building train action. */
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
#include "missile.h"
#include "sound.h"
#include "ai.h"
#include "interface.h"

/*----------------------------------------------------------------------------
--	Functions
----------------------------------------------------------------------------*/

/**
**	Unit trains unit!
**
**	@param unit	Unit that trains.
*/
global void HandleActionTrain(Unit* unit)
{
    Unit* nunit;
    UnitType* type;
    Player* player;

#if 0
    // JOHNS: should be checked by the user-interface
    if( &Players[unit->Player]==ThisPlayer ) {
	// FIXME: If so used you get millions of messages.
	if( ThisPlayer->Food<=ThisPlayer->Units
		&& unit->Command.Data.Train.Ticks ) {
	    SetMessage( "You need more farms!" );
	} else {
	    AiNeedMoreFarms(unit);
	}
    }
#endif

    player=unit->Player;
    unit->Command.Data.Train.Ticks+=SpeedTrain;
    // FIXME: Should count down
    if( unit->Command.Data.Train.Ticks
	    >=unit->Command.Data.Train.What[0]
		->Stats[player->Player].Costs[TimeCost] ) {

	//
	//	Check if enough food available.
	//
	if( player->Food<=player->NumFoodUnits ) {

	    // FIXME: GameMessage
	    if( player==ThisPlayer ) {
		// FIXME: PlayVoice :), see task.txt
		SetMessage("You need more farms!");
	    } else {
		// FIXME: Callback for AI!
		// AiNeedMoreFarms(unit);
	    }

	    unit->Command.Data.Train.Ticks-=SpeedTrain;
	    unit->Reset=1;
	    unit->Wait=FRAMES_PER_SECOND/6;
	    return;
	}

	nunit=MakeUnit(&UnitTypes[unit->Command.Data.Train.What[0]->Type]
		,player);
	nunit->X=unit->X;
	nunit->Y=unit->Y;
	type=unit->Type;
#ifdef NEW_HEADING
	DropOutOnSide(nunit,LookingW,type->TileWidth,type->TileHeight);
#else
	DropOutOnSide(nunit,HeadingW,type->TileWidth,type->TileHeight);
#endif

	// FIXME: GameMessage
	if( player==ThisPlayer ) {
	    SetMessage("Training complete");
	    PlayUnitSound(nunit,VoiceReady);
	} else {
	    AiTrainingComplete(unit,nunit);
	}

	unit->Reset=1;
	unit->Wait=1;
        
	if ( --unit->Command.Data.Train.Count ) {
	    int z;
	    for( z = 0; z < unit->Command.Data.Train.Count ; z++ ) {
		unit->Command.Data.Train.What[z] =
			unit->Command.Data.Train.What[z+1];
	    }
	    unit->Command.Data.Train.Ticks=0;
	} else {
	    unit->Command.Action=UnitActionStill;
	}

	nunit->Command=unit->PendCommand;

	if( IsSelected(unit) ) {
	    UpdateButtonPanel();
	    MustRedraw|=RedrawPanels;
	}

	return;
    }

    if( IsSelected(unit) ) {
	MustRedraw|=RedrawInfoPanel;
    }

    unit->Reset=1;
    unit->Wait=FRAMES_PER_SECOND/6;
}

//@}
