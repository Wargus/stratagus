//       _________ __                 __                               
//      /   _____//  |_____________ _/  |______     ____  __ __  ______
//      \_____  \\   __\_  __ \__  \\   __\__  \   / ___\|  |  \/  ___/
//      /        \|  |  |  | \// __ \|  |  / __ \_/ /_/  >  |  /\___ |
//     /_______  /|__|  |__|  (____  /__| (____  /\___  /|____//____  >
//             \/                  \/          \//_____/            \/ 
//  ______________________                           ______________________
//			  T H E   W A R   B E G I N S
//	   Stratagus - A free fantasy real time strategy game engine
//
/**@name action_train.c -	The building train action. */
//
//	(c) Copyright 1998,2000-2003 by Lutz Sammer
//
//      This program is free software; you can redistribute it and/or modify
//      it under the terms of the GNU General Public License as published by
//      the Free Software Foundation; version 2 dated June, 1991.
//
//      This program is distributed in the hope that it will be useful,
//      but WITHOUT ANY WARRANTY; without even the implied warranty of
//      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//      GNU General Public License for more details.
//
//      You should have received a copy of the GNU General Public License
//      along with this program; if not, write to the Free Software
//      Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
//      02111-1307, USA.
//
//	$Id$

//@{

/*----------------------------------------------------------------------------
--	Includes
----------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>

#include "stratagus.h"
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
    const UnitType* type;
    Player* player;
    int food;

    player=unit->Player;
    //
    //	First entry
    //
    if( !unit->SubAction ) {
	unit->Data.Train.Ticks=0;
	unit->Data.Train.What[0]=unit->Orders[0].Type;
	unit->Data.Train.Count=1;
	unit->SubAction=1;
    }
    unit->Data.Train.Ticks+=SpeedTrain;
    // FIXME: Should count down
    if( unit->Data.Train.Ticks
	    >=unit->Data.Train.What[0]
		->Stats[player->Player].Costs[TimeCost] ) {
	//
	//	Check if there are still unit slots.
	//
	if( NumUnits>=UnitMax  ) {
	    unit->Data.Train.Ticks=unit->Data.Train.What[0]
		    ->Stats[player->Player].Costs[TimeCost];
	    unit->Reset=1;
	    unit->Wait=CYCLES_PER_SECOND/6;
	    return;
	}

	//
	//	Check if enough food available.
	//
	if( (food=!PlayerCheckFood(player,unit->Data.Train.What[0]))
		|| !PlayerCheckLimits(player,unit->Data.Train.What[0]) ) {
	    if( food && unit->Player->Ai ) {
		AiNeedMoreFarms(unit,unit->Orders[0].Type);
	    }

	    unit->Data.Train.Ticks=unit->Data.Train.What[0]
		    ->Stats[player->Player].Costs[TimeCost];
	    unit->Reset=1;
	    unit->Wait=CYCLES_PER_SECOND/6;
	    return;
	}

	nunit=MakeUnit(unit->Data.Train.What[0],player);
	nunit->X=unit->X;
	nunit->Y=unit->Y;
	type=unit->Type;

	//  Some guy made DropOutOnSide set unit to belong to the building
	//  training it. This was an ugly hack, setting X and Y is enough,
	//  no need to add the unit only to be removed.
	nunit->X=unit->X;
	nunit->Y=unit->Y;

	DropOutOnSide(nunit,LookingW,type->TileWidth,type->TileHeight);

	// set life span
	if( type->DecayRate ) {
	    nunit->TTL=GameCycle+type->DecayRate*6*CYCLES_PER_SECOND;
	}

	NotifyPlayer(player,NotifyYellow,nunit->X,nunit->Y,
	    "New %s ready",nunit->Type->Name);
	if( player==ThisPlayer ) {
	    PlayUnitSound(nunit,VoiceReady);
	}
	if( unit->Player->Ai ) {
	    AiTrainingComplete(unit,nunit);
	}

	unit->Reset=unit->Wait=1;

	if ( --unit->Data.Train.Count ) {
	    int z;
	    for( z = 0; z < unit->Data.Train.Count ; z++ ) {
		unit->Data.Train.What[z]=unit->Data.Train.What[z+1];
	    }
	    unit->Data.Train.Ticks=0;
	} else {
	    unit->Orders[0].Action=UnitActionStill;
	    unit->SubAction=0;
	}

	//
	//	FIXME: we must check if the units supports the new order.
	//
	if( (unit->NewOrder.Action==UnitActionHaulOil
	        && !nunit->Type->Tanker)
    	    || (unit->NewOrder.Action==UnitActionAttack
	        && !nunit->Type->CanAttack)
            || ((unit->NewOrder.Action==UnitActionMineGold
                || unit->NewOrder.Action==UnitActionHarvest)
		&& nunit->Type!=UnitTypeOrcWorker 
		&& nunit->Type!=UnitTypeHumanWorker )
	    || (unit->NewOrder.Action==UnitActionBoard
	        && nunit->Type->UnitType!=UnitTypeLand) ) {
	    DebugLevel0Fn("Wrong order for unit\n");
            
	    //nunit->Orders[0].Action=UnitActionStandStill;
            
            // Tell the unit to move instead of trying to harvest
	    nunit->Orders[0]=unit->NewOrder;
	    nunit->Orders[0].Action=UnitActionMove;
	    if( nunit->Orders[0].Goal ) {
		RefsDebugCheck( !nunit->Orders[0].Goal->Refs );
		nunit->Orders[0].Goal->Refs++;
	    }
            
	} else {
	    if( unit->NewOrder.Goal ) {
		if( unit->NewOrder.Goal->Destroyed ) {
		    // FIXME: perhaps we should use another goal?
		    DebugLevel0Fn("Destroyed unit in train unit\n");
		    RefsDebugCheck( !unit->NewOrder.Goal->Refs );
		    if( !--unit->NewOrder.Goal->Refs ) {
			ReleaseUnit(unit->NewOrder.Goal);
		    }
		    unit->NewOrder.Goal=NoUnitP;
		    unit->NewOrder.Action=UnitActionStill;
		}
	    }

	    nunit->Orders[0]=unit->NewOrder;

	    //
	    // FIXME: Pending command uses any references?
	    //
	    if( nunit->Orders[0].Goal ) {
		RefsDebugCheck( !nunit->Orders[0].Goal->Refs );
		nunit->Orders[0].Goal->Refs++;
	    }
	}

	if( IsOnlySelected(unit) ) {
#ifndef NEW_UI
	    UpdateButtonPanel();
	    MustRedraw|=RedrawPanels;
#else
	    SelectedUnitChanged();
	    MustRedraw|=RedrawInfoPanel;
#endif
	}

	return;
    }

    if( IsOnlySelected(unit) ) {
	MustRedraw|=RedrawInfoPanel;
    }

    unit->Reset=1;
    unit->Wait=CYCLES_PER_SECOND/6;
}

//@}
