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
/**@name action_build.c -	The build building action. */
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
--      Includes
----------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>

#include "freecraft.h"
#include "unittype.h"
#include "player.h"
#include "unit.h"
#include "sound.h"
#include "actions.h"
#include "map.h"
#include "ai.h"
#include "interface.h"
#include "pathfinder.h"

/*----------------------------------------------------------------------------
--      Functions
----------------------------------------------------------------------------*/

/**
**	Unit builds a building.
**
**	@param unit	Unit that builds a building.
*/
global void HandleActionBuild(Unit* unit)
{
    int x;
    int y;
    int n;
    UnitType* type;
    const UnitStats* stats;
    Unit* build;
    Unit* temp;

    if( !unit->SubAction ) {		// first entry
	unit->SubAction=1;
	NewResetPath(unit);
    }

    type=unit->Orders[0].Type;

    switch( DoActionMove(unit) ) {	// reached end-point?
	case PF_UNREACHABLE:
	    //
	    //	Some tries to reach the goal
	    //
	    if( unit->SubAction++<10 ) {
		//	To keep the load low, retry each 1/4 second.
		// NOTE: we can already inform the AI about this problem?
		unit->Wait=FRAMES_PER_SECOND/4+unit->SubAction;
		return;
	    }

	    // FIXME: use general notify/messages
	    // NotifyPlayer("",unit,type);
	    if( unit->Player==ThisPlayer ) {
		SetMessage("You cannot reach building place.");
	    } else if( unit->Player->Ai ) {
		AiCanNotReach(unit,type);
	    }

	    unit->Orders[0].Action=UnitActionStill;
	    unit->SubAction=0;
	    if( unit->Selected ) {	// update display for new action
		UpdateButtonPanel();
	    }
	    return;

	case PF_REACHED:
	    DebugLevel3Fn("reached %d,%d\n",unit->X,unit->Y);
	    break;

	default:
	    return;
    }

    //
    //	Building place must be reached!
    //
    x=unit->Orders[0].X;
    y=unit->Orders[0].Y;
    if( type->ShoreBuilding ) {		// correct coordinates.
	++x;
	++y;
    }

    //
    //	Check if the building could be build there.
    //
    if( !CanBuildUnitType(unit,type,x,y) ) {
	//
	//	Some tries to build the building.
	//
	if( unit->SubAction++<10 ) {
	    //	To keep the load low, retry each 1/4 second.
	    // NOTE: we can already inform the AI about this problem?
	    unit->Wait=FRAMES_PER_SECOND/4+unit->SubAction;
	    return;
	}

	// FIXME: use general notify/messages
	// NotifyPlayer("",unit,type);
        if( unit->Player==ThisPlayer ) {
	    SetMessage("You cannot build %s here.", type->Name);
	} else if( unit->Player->Ai ) {
	    AiCanNotBuild(unit,type);
	}

	unit->Orders[0].Action=UnitActionStill;
	unit->SubAction=0;
	if( unit->Selected ) {	// update display for new action
	    UpdateButtonPanel();
	}

	return;
    }

    //
    //	Check if enough resources for the building.
    //
    if( PlayerCheckUnitType(unit->Player,type) ) {
	// FIXME: use general notify/messages
	// NotifyPlayer("",unit,type);
        if( unit->Player==ThisPlayer ) {
	    SetMessage("Not enough resources to build %s.", type->Name);
	} else if( unit->Player->Ai ) {
	    AiCanNotBuild(unit,type);
	}

	unit->Orders[0].Action=UnitActionStill;
	unit->SubAction=0;
	if( unit->Selected ) {	// update display for new action
	    UpdateButtonPanel();
	}
	return;
    }
    PlayerSubUnitType(unit->Player,type);

    build=MakeUnitAndPlace(x,y,type,unit->Player);
    stats=build->Stats;
    // HACK: the building is not ready yet
    build->Player->UnitTypesCount[type->Type]--;
    build->Constructed=1;
    build->HP=0;

    build->Orders[0].Action=UnitActionBuilded;
    build->Data.Builded.Sum=0;  // FIXME: Is it necessary?
    build->Data.Builded.Val=stats->HitPoints;
    n=(stats->Costs[TimeCost]*FRAMES_PER_SECOND/6)/(SpeedBuild*5);
    if( n ) {
	build->Data.Builded.Add=stats->HitPoints/n;
    } else {				// No build time pops-up?
	build->Data.Builded.Add=stats->HitPoints;
	// This checks if the value fits in the data type
	DebugCheck( build->Data.Builded.Add!=stats->HitPoints );
    }
    build->Data.Builded.Sub=n;
    build->Data.Builded.Cancel=0; // FIXME: Is it necessary?
    build->Data.Builded.Worker=unit;
    DebugLevel3Fn("Build Sum %d, Add %d, Val %d, Sub %d\n"
	,build->Data.Builded.Sum,build->Data.Builded.Add
	,build->Data.Builded.Val,build->Data.Builded.Sub);
    build->Wait=FRAMES_PER_SECOND/6;

    //
    //	Building oil-platform, must remove oil-patch.
    //
    if( type->GivesOil ) {
        DebugLevel0Fn("Remove oil-patch\n");
	temp=OilPatchOnMap(x,y);
	DebugCheck( !temp );
	unit->Value=temp->Value;	// Let worker hold value while building
	// oil patch should NOT make sound, handled by let unit die
	LetUnitDie(temp);		// Destroy oil patch
    }

    RemoveUnit(unit);	// automaticly: CheckUnitToBeDrawn(unit)

    unit->X=x;
    unit->Y=y;
    unit->Orders[0].Action=UnitActionStill;
    unit->SubAction=0;

    CheckUnitToBeDrawn(build);
    MustRedraw|=RedrawMinimap;
}

/**
**	Unit under Construction
**
**	@param unit	Unit that is builded.
*/
global void HandleActionBuilded(Unit* unit)
{
    Unit* worker;
    UnitType* type;

    type=unit->Type;

    //
    // Check if construction should be canceled...
    //
    if( unit->Data.Builded.Cancel ) {
	// Drop out unit
	worker=unit->Data.Builded.Worker;
	worker->Orders[0].Action=UnitActionStill;
	unit->Data.Builded.Worker=NoUnitP;
	worker->Reset=worker->Wait=1;
	worker->SubAction=0;

	unit->Value=worker->Value;	// worker holding value while building
	DropOutOnSide(worker,LookingW,type->TileWidth,type->TileHeight);

	// Player gets back 75% of the original cost for a building.
	PlayerAddCostsFactor(unit->Player,unit->Stats->Costs,
		CancelBuildingCostsFactor);
	// Cancel building
	LetUnitDie(unit);
	return;
    }

    //
    //	Fixed point HP calculation
    //
    unit->Data.Builded.Val-=unit->Data.Builded.Sub;
    if( unit->Data.Builded.Val<0 ) {
	unit->Data.Builded.Val+=unit->Stats->HitPoints;
	unit->HP++;
	unit->Data.Builded.Sum++;
    }
    unit->HP+=unit->Data.Builded.Add;
    unit->Data.Builded.Sum+=unit->Data.Builded.Add;

    //
    //	Check if building ready. Note we can build and repair.
    //
    if( unit->Data.Builded.Sum>=unit->Stats->HitPoints
		|| unit->HP>=unit->Stats->HitPoints ) {
	if( unit->HP>unit->Stats->HitPoints ) {
	    unit->HP=unit->Stats->HitPoints;
	}
	unit->Orders[0].Action=UnitActionStill;
	// HACK: the building is ready now
	unit->Player->UnitTypesCount[type->Type]++;
	unit->Constructed=0;
	unit->Frame=0;
	unit->Reset=unit->Wait=1;

	worker=unit->Data.Builded.Worker;
	worker->Orders[0].Action=UnitActionStill;
	worker->SubAction=0;
	worker->Reset=worker->Wait=1;
	DropOutOnSide(worker,LookingW,type->TileWidth,type->TileHeight);

	//
	//	Building oil-platform, must update oil.
	//
	if( type->GivesOil ) {
	    CommandHaulOil(worker,unit,0);	// Let the unit haul oil
	    DebugLevel0Fn("Update oil-platform\n");
	    DebugLevel0Fn(" =%d\n",unit->Data.Resource.Active);
	    unit->Data.Resource.Active=0;
	    unit->Value=worker->Value;	// worker holding value while building
	}
	//
	//	Building lumber mill, let worker automatic chopping wood.
	//
	if( type->StoresWood ) {
	    CommandHarvest(worker,unit->X+unit->Type->TileWidth/2,
		    unit->Y+unit->Type->TileHeight/2,0);
	}

	// FIXME: General message system
	// NotifyPlayer("",unit,type);
	if( unit->Player==ThisPlayer ) {
	    SetMessage2( unit->X, unit->Y, "New %s done", type->Name );
	    PlayUnitSound(worker,VoiceWorkCompleted);
	} else if( unit->Player->Ai ) {
	    AiWorkComplete(worker,unit);
	}

	// FIXME: Vladi: this is just a hack to test wall fixing,
	// FIXME:	also not sure if the right place...
	if ( unit->Type == UnitTypeOrcWall
		    || unit->Type == UnitTypeHumanWall ) {
	    MapSetWall(unit->X, unit->Y, unit->Type == UnitTypeHumanWall);
            RemoveUnit(unit);
	    UnitLost(unit);
	    ReleaseUnit(unit);
	    return;
        }

	UpdateForNewUnit(unit,0);

	if( IsOnlySelected(unit) ) {
	    UpdateButtonPanel();
	    MustRedraw|=RedrawPanels;
	} else if( unit->Player==ThisPlayer ) {
	    UpdateButtonPanel();
	}
        CheckUnitToBeDrawn(unit);
	return;
    }

    //
    //	Update building states
    //
    if( unit->Data.Builded.Sum*2>=unit->Stats->HitPoints ) {
        if( (unit->Frame!=1 || unit->Constructed) ) {
	  CheckUnitToBeDrawn(unit);
	}
	unit->Constructed=0;
	unit->Frame=1;
    } else if( unit->Data.Builded.Sum*4>=unit->Stats->HitPoints ) {
        if( unit->Frame!=1 ) {
	  CheckUnitToBeDrawn(unit);
	}
	unit->Frame=1;
    }

    unit->Wait=5;
    if( IsOnlySelected(unit) ) {
        MustRedraw|=RedrawInfoPanel;
    }
}

//@}
