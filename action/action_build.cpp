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
/**@name action_build.c -	The build building action. */
//
//	(c) Copyright 1998,2000-2003 by Lutz Sammer and Jimmy Salmon
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
--      Includes
----------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>

#include "stratagus.h"
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
**	Update construction frame
**
**	@param unit	Unit
*/
local void UpdateConstructionFrame(Unit* unit)
{
    ConstructionFrame* cframe;
    ConstructionFrame* tmp;
    int percent;

    percent=unit->Data.Builded.Sum*100/unit->Stats->HitPoints;
    cframe=tmp=unit->Type->Construction->Frames;
    while( tmp ) {
	if( percent<tmp->Percent ) {
	    break;
	}
	cframe=tmp;
	tmp=tmp->Next;
    }
    if( cframe!=unit->Data.Builded.Frame ) {
	unit->Data.Builded.Frame=cframe;
	unit->Frame=cframe->Frame;
	UnitMarkSeen(unit);
    }
}

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
		unit->Wait=CYCLES_PER_SECOND/4+unit->SubAction;
		return;
	    }

	    NotifyPlayer(unit->Player,NotifyYellow,unit->X,unit->Y,
		"You cannot reach building place");
	    if( unit->Player->Ai ) {
		AiCanNotReach(unit,type);
	    }

	    unit->Orders[0].Action=UnitActionStill;
	    unit->SubAction=0;
	    if( unit->Selected ) {	// update display for new action
		UpdateButtonPanel();
	    }
	    return;

	case PF_REACHED:
	    DebugLevel3Fn("reached %d,%d\n" _C_ unit->X _C_ unit->Y);
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
	    unit->Wait=CYCLES_PER_SECOND/4+unit->SubAction;
	    return;
	}

	NotifyPlayer(unit->Player,NotifyYellow,unit->X,unit->Y,
		"You cannot build %s here",type->Name);
	if( unit->Player->Ai ) {
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
    //	FIXME: got bug report about unit->Type==NULL in building
    //
    DebugCheck( !unit->Type || !unit->HP );

    if( !unit->Type || !unit->HP ) {
	return;
    }

    //
    //	Check if enough resources for the building.
    //
    if( PlayerCheckUnitType(unit->Player,type) ) {
	// FIXME: Better tell what is missing?
	NotifyPlayer(unit->Player,NotifyYellow,unit->X,unit->Y,
		"Not enough resources to build %s",type->Name);
	if( unit->Player->Ai ) {
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
    //	Check if hiting any limits for the building.
    //
    if( !PlayerCheckLimits(unit->Player,type) ) {
	NotifyPlayer(unit->Player,NotifyYellow,unit->X,unit->Y,
		"Can't build more units %s",type->Name);
	if( unit->Player->Ai ) {
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

    build=MakeUnit(type,unit->Player);
    build->Constructed=1;
    build->CurrentSightRange=0;
    PlaceUnit(build,x,y);
    build->CurrentSightRange=build->Type->TileWidth < build->Type->TileHeight
				? build->Type->TileHeight : build->Type->TileWidth;


/* Done by PlaceUnit now
#ifdef HIERARCHIC_PATHFINDER
    PfHierMapChangedCallback (build->X, build->Y,
		    build->X + build->Type->TileWidth - 1,
		    build->Y + build->Type->TileHeight - 1);
#endif
*/

    // HACK: the building is not ready yet
    build->Player->UnitTypesCount[type->Type]--;
    build->HP=0;

    stats=build->Stats;

    build->Orders[0].Action=UnitActionBuilded;
    build->Data.Builded.Sum=0;  // FIXME: Is it necessary?
    build->Data.Builded.Val=stats->HitPoints;
    n=(stats->Costs[TimeCost]*CYCLES_PER_SECOND/6)/(SpeedBuild*5);
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
	_C_ build->Data.Builded.Sum _C_ build->Data.Builded.Add
	_C_ build->Data.Builded.Val _C_ build->Data.Builded.Sub);
    build->Wait=CYCLES_PER_SECOND/6;
    UpdateConstructionFrame(build);

#if 0
    //
    //	Building oil-platform, must remove oil-patch.
    //
    if( type->GivesOil ) {
	Unit* temp;
        DebugLevel0Fn("Remove oil-patch\n");
	temp=OilPatchOnMap(x,y);
	DebugCheck( !temp );
	// FIXME: Johns: why the worker and not the construction?
	unit->Value=temp->Value;	// Let worker hold value while building
	// oil patch should NOT make sound, handled by let unit die
	LetUnitDie(temp);		// Destroy oil patch
    }
#endif
    unit->Value=build->Value;		// worker holding value while building

    RemoveUnit(unit,build);		// automaticly: CheckUnitToBeDrawn(unit)
    build->CurrentSightRange=0;
    unit->X=x;
    unit->Y=y;
    unit->Orders[0].Action=UnitActionStill;
    unit->SubAction=0;
    UnitMarkSeen(build);
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
    int n;

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

    n=(unit->Stats->Costs[TimeCost]*CYCLES_PER_SECOND/6)/(SpeedBuild*5);
    if( unit->Data.Builded.Add!=unit->Stats->HitPoints/(n?n:1) ) {
	unit->Data.Builded.Add=unit->Stats->HitPoints/(n?n:1);
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
	    DebugLevel0Fn(" =%d\n" _C_ unit->Data.Resource.Active);
	    unit->Data.Resource.Active=0;
	    unit->Value=worker->Value;	// worker holding value while building
	}
	//
	//	Building lumber mill, let worker automatic chopping wood.
	//
	if( type->CanStore[WoodCost] ) {
	    CommandHarvest(worker,unit->X+unit->Type->TileWidth/2,
		    unit->Y+unit->Type->TileHeight/2,0);
	}

	NotifyPlayer(unit->Player,NotifyGreen,unit->X,unit->Y,
	    "New %s done", type->Name);
	if( unit->Player==ThisPlayer ) {
	    PlayUnitSound(worker,VoiceWorkCompleted);
	} else if( unit->Player->Ai ) {
	    AiWorkComplete(worker,unit);
	}

	// FIXME: Vladi: this is just a hack to test wall fixing,
	// FIXME:	also not sure if the right place...
	// FIXME: Johns: hardcoded unit-type wall / more races!
	if ( unit->Type == UnitTypeOrcWall
		    || unit->Type == UnitTypeHumanWall ) {
	    MapSetWall(unit->X, unit->Y, unit->Type == UnitTypeHumanWall);
	    RemoveUnit(unit,NULL);
	    UnitLost(unit);
	    UnitClearOrders(unit);
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
	unit->CurrentSightRange=unit->Stats->SightRange;
	MapMarkSight(unit->Player,unit->X+unit->Type->TileWidth/2,
			    unit->Y+unit->Type->TileWidth/2,
			    unit->CurrentSightRange);
        CheckUnitToBeDrawn(unit);
	return;
    }

    UpdateConstructionFrame(unit);
    CheckUnitToBeDrawn(unit);

    unit->Wait=5;
    if( IsOnlySelected(unit) ) {
        MustRedraw|=RedrawInfoPanel;
    }
}

//@}
