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
/**@name action_hauloil.c -	The haul oil action. */
/*
**	(c) Copyright 1998-2000 by Lutz Sammer
**
**	$Id$
*/

//@{

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
#include "tileset.h"
#include "map.h"
#include "interface.h"

//	FIXME:	this could be generalized, one function for all oil+gold

/**
**	Move unit to oil platform.
**
**	@param unit	Pointer to unit.
**	@return		TRUE if reached, otherwise FALSE.
*/
local int MoveToOilWell(Unit* unit)
{
    Unit* well;
    int i;

    if( !(i=HandleActionMove(unit)) ) {	// reached end-point
	return 0;
    }
    DebugLevel3("Result: %d\n",i);
    if( i==-1 ) {
	DebugLevel2("OIL-WELL NOT REACHED\n");
	return -1;
    }

    well=unit->Command.Data.Move.Goal;

    if( !well ) {
        DebugLevel2("Invalid Oil Well\n");
	return -1;
    }

    // FIXME: the incomplete oil well didn't completed
    if( well->Command.Action==UnitActionBuilded ) {
        DebugLevel2("Invalid Oil Well\n");
	return -1;
    }

    unit->Command.Action=UnitActionHaulOil;

    DebugCheck( !well || MapDistanceToUnit(unit->X,unit->Y,well)!=1 );

    //
    // Activate oil-well
    //
    if( !well->Type->GivesOil ) {
	// OilWell destoryed.
	DebugLevel3("WAIT after oil-well destroyed %d\n",unit->Wait);
	unit->Command.Action=UnitActionStill;
	unit->SubAction=0;
	return 0;
    }
    well->Command.Data.OilWell.Active++;
    DebugLevel0(__FUNCTION__": +%d\n",well->Command.Data.OilWell.Active);
    well->Frame=2;			// FIXME: this shouldn't be hard coded!

    RemoveUnit(unit);
    unit->X=well->X;
    unit->Y=well->Y;

    if( HAUL_FOR_OIL<UNIT_MAX_WAIT ) {
	unit->Wait=HAUL_FOR_OIL;
    } else {
	unit->Wait=UNIT_MAX_WAIT;
    }
    unit->Value=HAUL_FOR_OIL-unit->Wait;

    return 1;
}

/**
**	Haul oil in oil well.
**
**	@param unit	Pointer to unit.
**	@return		TRUE if ready, otherwise FALSE.
*/
local int HaulInOilWell(Unit* unit)
{
    Unit* well;
    Unit* depot;

    DebugLevel3("Waiting\n");

    if( !unit->Value ) {
	//
	// Have oil
	//

	well=PlatformOnMap(unit->X,unit->Y);
	DebugLevel3("Found %d,%d=%Zd\n",unit->X,unit->Y,UnitNumber(well));
	IfDebug(
	    if( !well ) {
		DebugLevel0("No unit? (%d,%d)\n",unit->X,unit->Y);
		abort();
	    } );

	//
	//	Update oil-well.
	//
	well->Value-=DEFAULT_INCOMES[OilCost];	// remove oil from store
	DebugLevel0(__FUNCTION__": -%d\n",well->Command.Data.OilWell.Active);
	if( !--well->Command.Data.OilWell.Active ) {
	    well->Frame=0;
	}
	if( IsSelected(well) ) {
	    MustRedraw|=RedrawInfoPanel;
	}

	//
	//	End of oil: destroy oil-well.
	//
	if( well->Value<=0 ) {
	    // FIXME: better way to fix bug
	    unit->Removed=0;
	    DropOutAll(well);
	    DestroyUnit(well);
	    // Also remove oil-patch
#if 0
	    well=OilPatchOnMap(unit->X,unit->Y);
	    if( well ) {
		DebugLevel0(__FUNCTION__": removing oil patch\n");
		DestroyUnit(well);
	    }
#endif
	    well=NULL;
	}

	//
	//	Change unit to full state.
	//
	if( unit->Type->Type==UnitTankerHuman ) {
	    unit->Type=&UnitTypes[UnitTankerHumanFull];
	} else if( unit->Type->Type==UnitTankerOrc ) {
	    unit->Type=&UnitTypes[UnitTankerOrcFull];
	} else {
	    DebugLevel0("Wrong unit-type for haul oil %d\n"
		,unit->Type->Type);
	}

	//
	//	Find and send to oil deposit.
	//
	if( !(depot=FindOilDeposit(unit->Player,unit->X,unit->Y)) ) {
	    if( well ) {
		DropOutOnSide(unit,HeadingW
			,well->Type->TileWidth
			,well->Type->TileHeight);
	    } else {
		DropOutOnSide(unit,HeadingW,1,1);
	    }
	    unit->Command.Action=UnitActionStill;
	    unit->SubAction=0;
	    // FIXME: should return 0? done below!
	} else {
	    if( well ) {
		DropOutNearest(unit,depot->X,depot->Y
			,well->Type->TileWidth
			,well->Type->TileHeight);
	    } else {
		DropOutNearest(unit,depot->X,depot->Y,1,1);
	    }
	    unit->Command.Data.Move.Fast=1;
	    unit->Command.Data.Move.Goal=depot;
	    unit->Command.Data.Move.Range=1;
#if 1
	    NearestOfUnit(depot,unit->X,unit->Y
		,&unit->Command.Data.Move.DX
		,&unit->Command.Data.Move.DY);
#else
	    unit->Command.Data.Move.DX=depot->X;
	    unit->Command.Data.Move.DY=depot->Y;
#endif
	    unit->Command.Action=UnitActionHaulOil;
	}

	if( UnitVisible(unit) ) {
	    MustRedraw|=RedrawMap;
	}
	if( IsSelected(unit) ) {
	    UpdateButtonPanel();
	    MustRedraw|=RedrawButtonPanel;
	}
	unit->Wait=1;
	return unit->Command.Action==UnitActionHaulOil;
    }

    //
    //	Continue waiting
    //
    if( unit->Value<UNIT_MAX_WAIT ) {
	unit->Wait=unit->Value;
    } else {
	unit->Wait=UNIT_MAX_WAIT;
    }
    unit->Value-=unit->Wait;
    return 0;
}

/**
**	Move to oil-deposit.
**
**	@param unit	Pointer to unit.
**	@return		TRUE if ready, otherwise FALSE.
*/
local int MoveToOilDepot(Unit* unit)
{
    int i;
    int x;
    int y;
    Unit* depot;

    if( !(i=HandleActionMove(unit)) ) {	// reached end-point
	return 0;
    }
    DebugLevel3("Result: %d\n",i);
    if( i==-1 ) {
	DebugLevel2("OIL-DEPOT NOT REACHED\n");
	unit->Command.Action=UnitActionStill;
	unit->SubAction=0;
	return 0;
    }

    unit->Command.Action=UnitActionHaulOil;
    x=unit->Command.Data.Move.DX;
    y=unit->Command.Data.Move.DY;
    depot=OilDepositOnMap(x,y);

    DebugCheck( !depot || MapDistanceToUnit(unit->X,unit->Y,depot)!=1 );

    RemoveUnit(unit);
    unit->X=depot->X;
    unit->Y=depot->Y;

    //
    //	Update oil.
    //
    unit->Player->Resources[OilCost]+=unit->Player->Incomes[OilCost];
    if( unit->Player==ThisPlayer ) {
	MustRedraw|=RedrawResources;
    }

    if( unit->Type->Type==UnitTankerHumanFull ) {
	unit->Type=&UnitTypes[UnitTankerHuman];
    } else if( unit->Type->Type==UnitTankerOrcFull ) {
	unit->Type=&UnitTypes[UnitTankerOrc];
    } else {
	DebugLevel0("Wrong unit for returning oil %d\n"
	    ,unit->Type->Type);
    }

    if( WAIT_FOR_OIL<UNIT_MAX_WAIT ) {
	unit->Wait=WAIT_FOR_OIL;
    } else {
	unit->Wait=UNIT_MAX_WAIT;
    }
    unit->Value=WAIT_FOR_OIL-unit->Wait;
    return 1;
}

/**
**	Wait for oil to be delivered
**
**	@param unit	Pointer to unit.
**	@return		TRUE if ready, otherwise FALSE.
*/
local int WaitForOilDeliver(Unit* unit)
{
    Unit* platform;

    DebugLevel3("Waiting\n");
    if( !unit->Value ) {
	// FIXME: return to last position!
	if( !(platform=FindOilPlatform(unit->Player,unit->X,unit->Y)) ) {
	    DropOutOnSide(unit,HeadingW
		    ,UnitTypes[UnitGreatHall].TileWidth
		    ,UnitTypes[UnitGreatHall].TileHeight);
	    unit->Command.Action=UnitActionStill;
	    unit->SubAction=0;
	} else {
	    DropOutNearest(unit
		    ,platform->X
		    ,platform->Y
		    ,UnitTypes[UnitGreatHall].TileWidth
		    ,UnitTypes[UnitGreatHall].TileHeight);
	    unit->Command.Data.Move.Fast=1;
	    unit->Command.Data.Move.Goal=platform;
	    unit->Command.Data.Move.Range=1;
#if 1
	    NearestOfUnit(platform,unit->X,unit->Y
		,&unit->Command.Data.Move.DX
		,&unit->Command.Data.Move.DY);
#else
	    unit->Command.Data.Move.DX=platform->X;
	    unit->Command.Data.Move.DY=platform->Y;
#endif
	    unit->Command.Action=UnitActionHaulOil;
	    //unit->Value=UnitNumber(platform);	// unused
	}

	if( UnitVisible(unit) ) {
	    MustRedraw|=RedrawMap;
	}
	unit->Wait=1;
	unit->SubAction=0;
	return 1;
    } else {
	if( unit->Value<UNIT_MAX_WAIT ) {
	    unit->Wait=unit->Value;
	} else {
	    unit->Wait=UNIT_MAX_WAIT;
	}
	unit->Value-=unit->Wait;
    }
    return 0;
}

/**
**	Control the unit action haul oil
**
**	@param unit	Pointer to unit.
*/
global void HandleActionHaulOil(Unit* unit)
{
    int i;

    DebugLevel3(__FUNCTION__": %p (%Zd) SubAction %d\n"
	,unit,UnitNumber(unit),unit->SubAction);

    switch( unit->SubAction ) {
	//
	//	Move to oil-platform
	//
	case 0:
	    if( (i=MoveToOilWell(unit)) ) {
		if( i==-1 ) {
		    if( ++unit->SubAction==1 ) {
			unit->Command.Action=UnitActionStill;
			unit->SubAction=0;
		    }
		} else {
		    unit->SubAction=1;
		}
	    }
	    break;
	//
	//	Wait for haul oil
	//
	case 1:
	    if( HaulInOilWell(unit) ) {
		++unit->SubAction;
	    }
	    break;
	//
	//	Return to oil deposit
	//
	case 2:
	    if( MoveToOilDepot(unit) ) {
		++unit->SubAction;
	    }
	    break;
	//
	//	Wait for deliver oil
	//
	case 3:
	    if( WaitForOilDeliver(unit) ) {
		unit->SubAction=0;
	    }
	    break;
    }
}

//@}
