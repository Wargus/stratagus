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
/**@name action_resource.c -	The generic resource action. */
//
//	(c) Copyright 2001-2003 by Lutz Sammer
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
#include "player.h"
#include "unit.h"
#include "unittype.h"
#include "actions.h"
#include "pathfinder.h"
#include "interface.h"

/*----------------------------------------------------------------------------
--	Declarations
----------------------------------------------------------------------------*/

#define SUB_START_RESOURCE 0
#define SUB_MOVE_TO_RESOURCE 1
#define SUB_UNREACHABLE_RESOURCE 31
#define SUB_GATHER_RESOURCE 60
#define SUB_MOVE_TO_DEPOT 70
#define SUB_UNREACHABLE_DEPOT 100
#define SUB_RETURN_RESOURCE 120

/*----------------------------------------------------------------------------
--	Functions
----------------------------------------------------------------------------*/

/**
**	Move unit to resource.
**
**	@param unit	Pointer to unit.
**
**	@return		TRUE if reached, otherwise FALSE.
*/
local int MoveToResource(Unit* unit)
{
    Unit* goal;

    goal=unit->Orders[0].Goal;
    DebugCheck( !goal );

    switch( DoActionMove(unit) ) {	// reached end-point?
	case PF_UNREACHABLE:
	    return -1;
	case PF_REACHED:
	    break;
	default:
	    if( !unit->Reset || !(goal->Destroyed || goal->Removed
		    || !goal->HP || goal->Orders[0].Action==UnitActionDie) ) {
		return 0;
	    }
	    break;
    }

    //
    //	Target is dead, stop getting resources.
    //
    if( goal->Destroyed ) {
	DebugLevel0Fn("Destroyed resource goal, stop gathering.\n");
	RefsDebugCheck( !goal->Refs );
	if( !--goal->Refs ) {
	    ReleaseUnit(goal);
	}
	unit->Orders[0].Goal=NoUnitP;
	// FIXME: perhaps we should choose an alternative
	unit->Orders[0].Action=UnitActionStill;
	unit->SubAction=0;
	return 0;
    } else if( goal->Removed || !goal->HP
	    || goal->Orders[0].Action==UnitActionDie ) {
	RefsDebugCheck( !goal->Refs );
	--goal->Refs;
	RefsDebugCheck( !goal->Refs );
	unit->Orders[0].Goal=NoUnitP;
	// FIXME: perhaps we should choose an alternative
	unit->Orders[0].Action=UnitActionStill;
	unit->SubAction=0;
	return 0;
    }

    // FIXME: 0 can happen, if to near placed by map designer.
    DebugCheck( MapDistanceToUnit(unit->X,unit->Y,goal)>1 );

    DebugCheck( unit->Wait!=1 );

    //
    //	If resource is still under construction, wait!
    //
    if( goal->Orders[0].Action==UnitActionBuilded ) {
        DebugLevel2Fn("Invalid resource\n");
	return 0;
    }

    RefsDebugCheck( !goal->Refs );
    --goal->Refs;
    RefsDebugCheck( !goal->Refs );
    unit->Orders[0].Goal=NoUnitP;

    //
    //	Activate the resource
    //
    goal->Data.Resource.Active++;
    DebugLevel3Fn("+%d\n" _C_ goal->Data.Resource.Active);

    if( !goal->Frame ) {		// show resource working
	goal->Frame=2;
	CheckUnitToBeDrawn(goal);
    }
    UnitMarkSeen(goal);
    //
    //	Place unit inside the resource
    //
    RemoveUnit(unit,goal);
    unit->X=goal->X;
    unit->Y=goal->Y;

    if (unit->Type->WaitAtResource) {
	unit->Wait=unit->Type->WaitAtResource;
    } else {
	unit->Wait=1;
    }

    return 1;
}

/**
**	Wait in resource, for collecting the resource.
**
**	@param unit	Pointer to unit.
**
**	@return		TRUE if ready, otherwise FALSE.
*/
local int WaitInResource(Unit* unit)
{
    Unit* source;
    Unit* depot;

    //
    //	Have the resource
    //
    source=unit->Container;

    DebugCheck( !source );
    DebugCheck( source->Value>655350 );

    //
    //	Update the resource. FIXME: depleted resources.
    //
    if (source->Value<unit->Type->ResourceCapacity) {
	// Uhh-oh, depleted.
	unit->Value=source->Value;
	source->Value=0;
    } else {
	unit->Value=unit->Type->ResourceCapacity;
	source->Value-=unit->Type->ResourceCapacity;
    }
    
    if( !--source->Data.Resource.Active ) {
	source->Frame=0;
	CheckUnitToBeDrawn(source);
    }
    
    UnitMarkSeen(source);
    if( IsOnlySelected(source) ) {
	MustRedraw|=RedrawInfoPanel;
    }

    //
    //	Change unit to full state.
    //
    if( unit->Type->TransformWhenLoaded ) {
	unit->Player->UnitTypesCount[unit->Type->Type]--;
	unit->Type=unit->Type->TransformWhenLoaded;
    	unit->Player->UnitTypesCount[unit->Type->Type]++;
    }

    //	Store resource position.
    unit->Orders[0].Arg1=(void*)((unit->X<<16)|unit->Y);
    
    //
    //	Find and send to resource deposit.
    //
    if( !(depot=FindDeposit(unit->Player,unit->X,unit->Y,unit->Type->ResourceHarvested)) ) {
	DropOutOnSide(unit,LookingW
		,source->Type->TileWidth,source->Type->TileHeight);
	DebugLevel0Fn("Can't find a resource deposit.\n");
	unit->Orders[0].Action=UnitActionStill;
	unit->SubAction=0;
	// should return 0, done below!
    } else {
	DropOutNearest(unit,depot->X+depot->Type->TileWidth/2
		,depot->Y+depot->Type->TileHeight/2
		,source->Type->TileWidth,source->Type->TileHeight);
	unit->Orders[0].Goal=depot;
	RefsDebugCheck( !depot->Refs );
	++depot->Refs;
	unit->Orders[0].RangeX=unit->Orders[0].RangeY=1;
	unit->Orders[0].X=unit->Orders[0].Y=-1;
	unit->Orders[0].X=unit->Orders[0].Y=-1;
	unit->SubAction=SUB_MOVE_TO_DEPOT;
	NewResetPath(unit);
    }
    
    //
    //	End of resource: destroy the resource.
    //
    if( source->Value==0 ) {
	DebugLevel0Fn("Resource is destroyed\n");
	DropOutAll(source);
	LetUnitDie(source);
	source=NULL;
    }

    CheckUnitToBeDrawn(unit);
    if( IsOnlySelected(unit) ) {
	UpdateButtonPanel();
	SelectedUnitChanged();
	// FIXME: redundant?
	MustRedraw|=RedrawButtonPanel;
    }
    unit->Wait=1;
    return unit->Orders[0].Action!=UnitActionStill;
}

/**
**	Move to resource depot
**
**	@param unit	Pointer to unit.
**
**	@return		TRUE if reached, otherwise FALSE.
*/
local int MoveToDepot(Unit* unit)
{
    Unit* goal;

    goal=unit->Orders[0].Goal;
    DebugCheck( !goal );

    switch( DoActionMove(unit) ) {	// reached end-point?
	case PF_UNREACHABLE:
	    return -1;
	case PF_REACHED:
	    break;
	default:
	    if( !unit->Reset || !(goal->Destroyed || goal->Removed
		    || !goal->HP || goal->Orders[0].Action==UnitActionDie) ) {
		return 0;
	    }
	    break;
    }

    //
    //	Target is dead, stop getting resources.
    //
    if( goal->Destroyed ) {
	DebugLevel0Fn("Destroyed unit\n");
	RefsDebugCheck( !goal->Refs );
	if( !--goal->Refs ) {
	    ReleaseUnit(goal);
	}
	unit->Orders[0].Goal=NoUnitP;
	// FIXME: perhaps we should choose an alternative
	unit->Orders[0].Action=UnitActionStill;
	unit->SubAction=0;
	return 0;
    } else if( goal->Removed || !goal->HP
	    || goal->Orders[0].Action==UnitActionDie ) {
	RefsDebugCheck( !goal->Refs );
	--goal->Refs;
	RefsDebugCheck( !goal->Refs );
	unit->Orders[0].Goal=NoUnitP;
	// FIXME: perhaps we should choose an alternative
	unit->Orders[0].Action=UnitActionStill;
	unit->SubAction=0;
	return 0;
    }

    DebugCheck( MapDistanceToUnit(unit->X,unit->Y,goal)!=1 );

    DebugCheck( unit->Wait!=1 );

    //
    //	If resource depot is still under construction, wait!
    //
    if( goal->Orders[0].Action==UnitActionBuilded ) {
        DebugLevel2Fn("Invalid resource depot. FIXME:WAIT!!! \n");
	return 0;
    }

    RefsDebugCheck( !goal->Refs );
    --goal->Refs;
    RefsDebugCheck( !goal->Refs );
    unit->Orders[0].Goal=NoUnitP;

    //
    //	Place unit inside the depot
    //
    RemoveUnit(unit,goal);
    unit->X=goal->X;
    unit->Y=goal->Y;

    //
    //	Update resource.
    //
    unit->Player->Resources[unit->Type->ResourceHarvested]+=
	(unit->Value*unit->Player->Incomes[unit->Type->ResourceHarvested])/100;
    unit->Player->TotalResources[unit->Type->ResourceHarvested]+=
	(unit->Value*unit->Player->Incomes[unit->Type->ResourceHarvested])/100;
    if( unit->Player==ThisPlayer ) {
	MustRedraw|=RedrawResources;
    }

    //
    //	Change unit to empty state.
    //
    if( unit->Type->TransformWhenEmpty ) {
    	unit->Player->UnitTypesCount[unit->Type->Type]--;
	unit->Type=unit->Type->TransformWhenEmpty;
    	unit->Player->UnitTypesCount[unit->Type->Type]++;
    }

    if (unit->Type->WaitAtDepot) {
	unit->Wait=unit->Type->WaitAtDepot;
    } else {
	unit->Wait=1;
    }
    return 1;
}

/**
**	Wait in depot, for the resources stored.
**
**	@param unit	Pointer to unit.
**
**	@return		TRUE if ready, otherwise FALSE.
*/
local int WaitInDepot(Unit* unit)
{
    const Unit* depot;
    Unit* goal;
    int x;
    int y;

    depot=ResourceDepositOnMap(unit->X,unit->Y,unit->Type->ResourceHarvested);
    DebugCheck( !depot );
    // Could be destroyed, but then we couldn't be in?

    if( unit->Orders[0].Arg1==(void*)-1 ) {
	x=unit->X;
	y=unit->Y;
    } else {
	x=(int)unit->Orders[0].Arg1>>16;
	y=(int)unit->Orders[0].Arg1&0xFFFF;
    }
    if( !(goal=FindResource(unit->Player,x,y,unit->Type->ResourceHarvested)) ) {
	DropOutOnSide(unit,LookingW,
		depot->Type->TileWidth,depot->Type->TileHeight);
	unit->Orders[0].Action=UnitActionStill;
	unit->SubAction=0;
    } else {
	DropOutNearest(unit,goal->X+goal->Type->TileWidth/2
		,goal->Y+goal->Type->TileHeight/2
		,depot->Type->TileWidth,depot->Type->TileHeight);
	unit->Orders[0].Goal=goal;
	RefsDebugCheck( !goal->Refs );
	++goal->Refs;
	unit->Orders[0].RangeX=unit->Orders[0].RangeY=1;
	unit->Orders[0].X=unit->Orders[0].Y=-1;
	NewResetPath(unit);
    }

    CheckUnitToBeDrawn(unit);
    unit->Wait=1;
    return unit->Orders[0].Action!=UnitActionStill;
}

/**
** 	Give up on gathering.
**
**	@param unit	Pointer to unit.
*/
void ResourceGiveUp(Unit* unit)
{
    unit->Orders[0].Action=UnitActionStill;
    unit->Wait=1;
    unit->SubAction=0;
    if( unit->Orders[0].Goal ) {
	RefsDebugCheck( !unit->Orders[0].Goal->Refs );
	--unit->Orders[0].Goal->Refs;
	RefsDebugCheck( !unit->Orders[0].Goal->Refs );
	unit->Orders[0].Goal=NoUnitP;
    }
}

/**
**	Control the unit action: getting a resource.
**
**	This the generic function for oil, gold, ...
**
**	@param unit	Pointer to unit.
*/
global void HandleActionResource(Unit* unit)
{
    int ret;

    DebugLevel3Fn("%s(%d) SubAction %d\n"
	_C_ unit->Type->Ident _C_ UnitNumber(unit) _C_ unit->SubAction);

    if ( unit->SubAction==SUB_START_RESOURCE ) {
	NewResetPath(unit);
	unit->SubAction=1;
    }
    
    if ( unit->SubAction>=SUB_MOVE_TO_RESOURCE && unit->SubAction<SUB_UNREACHABLE_RESOURCE ) {
	// -1 failure, 0 not yet reached, 1 reached
	if( (ret=MoveToResource(unit)) ) {
	    if( ret==-1 ) {
		// Can't Reach
		unit->SubAction++;
		unit->Wait=10;
	    } else {
		unit->SubAction=SUB_GATHER_RESOURCE;
	    }
	}
	return;
    }

    if (unit->SubAction==SUB_UNREACHABLE_RESOURCE) {
	return;
	ResourceGiveUp(unit);
    }

    if (unit->SubAction==SUB_GATHER_RESOURCE) {
	if( WaitInResource(unit) ) {
	    unit->SubAction=SUB_MOVE_TO_DEPOT;
	}
    }

    if (unit->SubAction>=SUB_MOVE_TO_DEPOT&&unit->SubAction<SUB_UNREACHABLE_DEPOT) {
	// -1 failure, 0 not yet reached, 1 reached
	if( (ret=MoveToDepot(unit)) ) {
	    if( ret==-1 ) {
		// Can't Reach
		unit->SubAction++;
		unit->Wait=10;
	    } else {
		unit->SubAction=SUB_RETURN_RESOURCE;
	    }
	}
	return;
    }

    if (unit->SubAction==SUB_UNREACHABLE_DEPOT) {
	ResourceGiveUp(unit);
	return;
    }

    if (unit->SubAction==SUB_RETURN_RESOURCE) {
	if( WaitInDepot(unit) ) {
	    unit->SubAction=SUB_START_RESOURCE;
	}
	return;
    }
}

//@}
