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
/**@name action_resource.c -	The generic resource action. */
//
//	(c) Copyright 2001 by Lutz Sammer
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
#include "unit.h"
#include "unittype.h"
#include "actions.h"
#include "pathfinder.h"
#include "interface.h"

/*----------------------------------------------------------------------------
--	Declarations
----------------------------------------------------------------------------*/

/**
**	Helper structure for getting resources.
**	FIXME: later we should make this configurable, to allow game
**	FIXME: designers to create own resources.
*/
typedef struct _resource_ {
    int	Action;				/// Unit action.
    int	Frame;				/// Frame for active resource
    Unit* (*ResourceOnMap)(int,int);	/// Get the resource on map.
    Unit* (*DepositOnMap)(int,int);	/// Get the deposit on map.
	 /// Find the source of resource
    Unit* (*FindResource)(const Player*,int,int);
	 /// Find the deposit of resource
    Unit* (*FindDeposit)(const Player*,int,int);
    int Cost;				/// How many can the unit carry.
    UnitType** Human;			/// Human worker
    UnitType** HumanWithResource;	/// Human worker with resource
    UnitType** Orc;			/// Orc worker
    UnitType** OrcWithResource;		/// Orc worker with resource

    int	GetTime;			/// Time to get the resource
    int	PutTime;			/// Time to store the resource

} Resource;

/*----------------------------------------------------------------------------
--	Functions
----------------------------------------------------------------------------*/

/**
**	Move unit to resource.
**
**	@param unit	Pointer to unit.
**	@param resouce	How to handle the resource.
**
**	@return		TRUE if reached, otherwise FALSE.
*/
local int MoveToResource(Unit* unit,const Resource* resource)
{
    Unit* goal;

    switch( DoActionMove(unit) ) {	// reached end-point?
	case PF_UNREACHABLE:
	    DebugCheck( unit->Orders[0].Action!=resource->Action );
	    return -1;
	case PF_REACHED:
	    break;
	default:
	    return 0;
    }

    goal=unit->Orders[0].Goal;

    DebugCheck( !goal );
    DebugCheck( unit->Wait!=1 );
    DebugCheck( MapDistanceToUnit(unit->X,unit->Y,goal)!=1 );

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

    DebugCheck( unit->Orders[0].Action!=resource->Action );

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
    DebugLevel3Fn("+%d\n",goal->Data.Resource.Active);

    if( !goal->Frame ) {		// show resource working
	goal->Frame=resource->Frame;
	CheckUnitToBeDrawn(goal);
    }

    //
    //	Place unit inside the resource
    //
    RemoveUnit(unit);
    unit->X=goal->X;
    unit->Y=goal->Y;

    //
    //	Time to collect the resource.
    //
    if( resource->GetTime<MAX_UNIT_WAIT ) {
	unit->Wait=resource->GetTime;
    } else {
	unit->Wait=MAX_UNIT_WAIT;
    }
    unit->Value=resource->GetTime-unit->Wait;

    return 1;
}

/**
**	Wait in resource, for collecting the resource.
**
**	@param unit	Pointer to unit.
**	@param resouce	How to handle the resource.
**
**	@return		TRUE if ready, otherwise FALSE.
*/
local int WaitInResource(Unit* unit,const Resource* resource)
{
    Unit* source;
    Unit* depot;

    DebugLevel3Fn("Waiting\n");

    if( !unit->Value ) {
	//
	//	Have the resource
	//
	source=resource->ResourceOnMap(unit->X,unit->Y);
	IfDebug(
	    DebugLevel2Fn("Found %d,%d=%d\n",unit->X,unit->Y
		,UnitNumber(source));
	    if( !source ) {
		DebugLevel0Fn("No unit? (%d,%d)\n",unit->X,unit->Y);
		abort();
	    } );

	//
	//	Update the resource.
	//	Remove what we can carry, FIXME: always this?
	//
	source->Value-=DEFAULT_INCOMES[resource->Cost];
	DebugLevel3Fn("-%d\n",source->Data.Resource.Active);
	if( !--source->Data.Resource.Active ) {
	    source->Frame=0;
	    CheckUnitToBeDrawn(source);
	}
	if( IsOnlySelected(source) ) {
	    MustRedraw|=RedrawInfoPanel;
	}

	//
	//	End of resource: destroy the resource.
	//
	if( source->Value<DEFAULT_INCOMES[resource->Cost] ) {
	    DropOutAll(source);
	    DestroyUnit(source);
	    source=NULL;
	}

	//
	//	Change unit to full state. FIXME: more races
	//
	if( unit->Type==*resource->Human ) {
	    unit->Type=*resource->HumanWithResource;
	} else if( unit->Type==*resource->Orc ) {
	    unit->Type=*resource->OrcWithResource;
	} else {
	    // FIXME: should support more races
	    DebugLevel0Fn("Wrong unit-type `%s' for resource `%s'\n"
		,unit->Type->Ident,DEFAULT_NAMES[resource->Cost]);
	}

	//
	//	Find and send to resource deposit.
	//
	if( !(depot=resource->FindDeposit(unit->Player,unit->X,unit->Y)) ) {
	    if( source ) {
		DropOutOnSide(unit,LookingW
		    ,source->Type->TileWidth,source->Type->TileHeight);
	    }
	    unit->Orders[0].Action=UnitActionStill;
	    unit->SubAction=0;
	    // should return 0, done below!
	} else {
	    if( source ) {
		DropOutNearest(unit,depot->X+depot->Type->TileWidth/2
		    ,depot->Y+depot->Type->TileHeight/2
		    ,source->Type->TileWidth,source->Type->TileHeight);
	    }
	    unit->Orders[0].Goal=depot;
	    RefsDebugCheck( !depot->Refs );
	    ++depot->Refs;
	    unit->Orders[0].RangeX=unit->Orders[0].RangeY=1;
	    unit->Orders[0].X=unit->Orders[0].Y=-1;
	    unit->Orders[0].Action=resource->Action;
	    unit->SubAction=64;
	    NewResetPath(unit);
	}

        CheckUnitToBeDrawn(unit);
	if( IsOnlySelected(unit) ) {
	    UpdateButtonPanel();
	    MustRedraw|=RedrawButtonPanel;
	}
	unit->Wait=1;
	return unit->Orders[0].Action==resource->Action;
    }

    //
    //	Continue waiting
    //
    if( unit->Value<MAX_UNIT_WAIT ) {
	unit->Wait=unit->Value;
    } else {
	unit->Wait=MAX_UNIT_WAIT;
    }
    unit->Value-=unit->Wait;
    return 0;
}

/**
**	Move to resource depot
**
**	@param unit	Pointer to unit.
**	@param resouce	How to handle the resource.
**
**	@return		TRUE if reached, otherwise FALSE.
*/
local int MoveToDepot(Unit* unit,const Resource* resource)
{
    Unit* goal;

    switch( DoActionMove(unit) ) {	// reached end-point?
	case PF_UNREACHABLE:
	    DebugCheck( unit->Orders[0].Action!=resource->Action );
	    return -1;
	case PF_REACHED:
	    break;
	default:
	    return 0;
    }

    goal=unit->Orders[0].Goal;

    DebugCheck( !goal );
    DebugCheck( unit->Wait!=1 );
    DebugCheck( MapDistanceToUnit(unit->X,unit->Y,goal)!=1 );

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

    DebugCheck( unit->Orders[0].Action!=resource->Action );

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
    //	Place unit inside the depot
    //
    RemoveUnit(unit);
    unit->X=goal->X;
    unit->Y=goal->Y;

    //
    //	Update resource.
    //
    unit->Player->Resources[resource->Cost]
	+=unit->Player->Incomes[resource->Cost];
    if( unit->Player==ThisPlayer ) {
	MustRedraw|=RedrawResources;
    }

    //
    //	Change unit to empty state. FIXME: more races
    //
    if( unit->Type==*resource->HumanWithResource ) {
	unit->Type=*resource->Human;
    } else if( unit->Type==*resource->OrcWithResource ) {
	unit->Type=*resource->Orc;
    } else {
	DebugLevel0Fn("Wrong unit-type `%s' for resource `%s'\n"
	    ,unit->Type->Ident,DEFAULT_NAMES[resource->Cost]);
    }

    //
    //	Time to store the resource.
    //
    if( resource->PutTime<MAX_UNIT_WAIT ) {
	unit->Wait=resource->PutTime;
    } else {
	unit->Wait=MAX_UNIT_WAIT;
    }
    unit->Value=resource->PutTime-unit->Wait;
    return 1;
}

/**
**	Wait in depot, for the resources stored.
**
**	@param unit	Pointer to unit.
**	@param resouce	How to handle the resource.
**
**	@return		TRUE if ready, otherwise FALSE.
*/
local int WaitInDepot(Unit* unit,const Resource* resource)
{
    Unit* depot;
    Unit* goal;

    DebugLevel3Fn("Waiting\n");
    if( !unit->Value ) {
	depot=resource->DepositOnMap(unit->X,unit->Y);
	DebugCheck( !depot );
	// Could be destroyed, but than we couldn't be in?

	// FIXME: return to last position!
	if( !(goal=resource->FindResource(unit->Player,unit->X,unit->Y)) ) {
	    DropOutOnSide(unit,LookingW
		,depot->Type->TileWidth,depot->Type->TileHeight);
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
	    unit->Orders[0].Action=resource->Action;
	    NewResetPath(unit);
	}

        CheckUnitToBeDrawn(unit);
	unit->Wait=1;
	return unit->Orders[0].Action==resource->Action;
    }

    //
    //	Continue waiting
    //
    if( unit->Value<MAX_UNIT_WAIT ) {
	unit->Wait=unit->Value;
    } else {
	unit->Wait=MAX_UNIT_WAIT;
    }
    unit->Value-=unit->Wait;
    return 0;
}

/**
**	Control the unit action: getting a resource.
**
**	This the generic function for oil, gold, ...
**
**	@param unit	Pointer to unit.
**	@param resouce	How to handle the resource.
*/
global void HandleActionResource(Unit* unit,const Resource* resource)
{
    int ret;

    DebugLevel3Fn("%s(%d) SubAction %d\n"
	,unit->Type->Ident,UnitNumber(unit),unit->SubAction);

    switch( unit->SubAction ) {
	//
	//	Move to the resource
	//
	case 0:
	    NewResetPath(unit);
	    unit->SubAction=1;
	    // FALL THROUGH
	case 1:
	case 2:
	case 3:
	case 4:
	    if( (ret=MoveToResource(unit,resource)) ) {
		if( ret==-1 ) {
		    if( ++unit->SubAction==5 ) {
			unit->Orders[0].Action=UnitActionStill;
			unit->SubAction=0;
			if( unit->Orders[0].Goal ) {
			    RefsDebugCheck( !unit->Orders[0].Goal->Refs );
			    --unit->Orders[0].Goal->Refs;
			    RefsDebugCheck( !unit->Orders[0].Goal->Refs );
			    unit->Orders[0].Goal=NoUnitP;
			}
		    } else {			// Do a little delay
			unit->Wait*=unit->SubAction;
			DebugLevel0Fn("Retring\n");
		    }
		} else {
		    unit->SubAction=64;
		}
	    }
	    break;

	//
	//	Wait for collecting the resource
	//
	case 64:
	    if( WaitInResource(unit,resource) ) {
		++unit->SubAction;
	    }
	    break;

	//
	//	Return to the resource depot
	//
	case 65:
	case 66:
	case 67:
	case 68:
	case 69:
	    if( (ret=MoveToDepot(unit,resource)) ) {
		if( ret==-1 ) {
		    if( ++unit->SubAction==70 ) {
			unit->Orders[0].Action=UnitActionStill;
			unit->SubAction=0;
			if( unit->Orders[0].Goal ) {
			    RefsDebugCheck( !unit->Orders[0].Goal->Refs );
			    --unit->Orders[0].Goal->Refs;
			    RefsDebugCheck( !unit->Orders[0].Goal->Refs );
			    unit->Orders[0].Goal=NoUnitP;
			}
		    } else {			// Do a little delay
			unit->Wait*=unit->SubAction-65;
			DebugLevel0Fn("Retring\n");
		    }
		} else {
		    unit->SubAction=128;
		}
	    }
	    break;

	//
	//	Wait for resource stored
	//
	case 128:
	    if( WaitInDepot(unit,resource) ) {
		unit->SubAction=0;
	    }
	    break;
    }
}

/*----------------------------------------------------------------------------
--	High level
----------------------------------------------------------------------------*/

/**
**	The oil resource.
*/
local Resource ResourceOil[1] = {
{
    UnitActionHaulOil,
    2,					// FIXME: hardcoded.
    PlatformOnMap,
    OilDepositOnMap,
    FindOilPlatform,
    FindOilDeposit,
    OilCost,
    // FIXME: The & could be removed.
    &UnitTypeHumanTanker,
    &UnitTypeHumanTankerFull,
    &UnitTypeOrcTanker,
    &UnitTypeOrcTankerFull,

    0,					// Must be initialized
    0,					// Must be initialized
}
};

/**
**	Control the unit action haul oil
**
**	@param unit	Pointer to unit.
*/
global void HandleActionHaulOil(Unit* unit)
{
    // FIXME: move into init function, debug could change this values.
    ResourceOil->GetTime=HAUL_FOR_OIL;
    ResourceOil->PutTime=WAIT_FOR_OIL;

    HandleActionResource(unit,ResourceOil);
}

//@}
