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
/*
**	(c) Copyright 1998,2000 by Lutz Sammer
**
**	$Id$
*/

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
    int dx;
    int dy;
    int n;
    UnitType* type;
    const UnitStats* stats;
    Unit* build;
    Unit* temp;

    type=unit->Command.Data.Build.BuildThis;
    switch( HandleActionMove(unit) ) {	// reached end-point?
	case PF_UNREACHABLE:
	    // FIXME: use general notify/messages
	    if( unit->Player==ThisPlayer ) {
		SetMessage("You cannot reach building place.");
	    } else {
		AiCanNotReach(unit,type);
	    }
	    return;
	case PF_REACHED:
	    DebugLevel3Fn("reached %d,%d\n",unit->X,unit->Y);
	    break;
	default:
	    return;
    }

    // Must be reached!
    IfDebug(
	// Check internal code, if really reached?
	x=unit->X;
	y=unit->Y;
	dx=unit->Command.Data.Move.DX;
	dy=unit->Command.Data.Move.DY;
	if( type->ShoreBuilding ) {
	    ++dx;
	    ++dy;
	}
	n=type->ShoreBuilding&1;
	//
	//	Check if building place reached.
	//
	DebugLevel3Fn("`%s' %d,%d - %d,%d (%d)\n"
		,type->Name
		,dx-n,dy-n
		,dx+type->TileWidth+n
		,dy+type->TileHeight+n
		,n);

	if( x<dx-n || x>=dx+type->TileWidth+n
	        || y<dy-n || y>=dy+type->TileHeight+n ) {
	    DebugLevel0Fn("Internal error\n");
	    abort();
	}
    );

    x=unit->Command.Data.Move.DX;
    y=unit->Command.Data.Move.DY;
    if( type->ShoreBuilding ) {		// correct coordinates.
	++x;
	++y;
    }
    
    //
    //	Check if the building could be build there.
    //
    if( !CanBuildUnitType(unit,type,x,y) ) {
	// FIXME: use general notify/messages
        if( unit->Player==ThisPlayer ) {
	    SetMessage("You cannot build here.");
	} else {
	    AiCanNotBuild(unit,type);
	}
	return;
    }

    //
    //	Check if enough resources for the building.
    //
    if( PlayerCheckUnitType(unit->Player,type) ) {
	if( unit->Player!=ThisPlayer ) {
	    AiCanNotBuild(unit,type);
	}
	// Comment: For the usual player the resources are substracted
	// when the build icon is pressed.
	return;
    }
    PlayerSubUnitType(unit->Player,type);

    build=MakeUnitAndPlace(x,y,type,unit->Player);
    stats=build->Stats;
    // HACK: the building is not ready yet
    build->Player->UnitTypesCount[type->Type]--;
    build->Constructed=1;
    build->HP=0;
    build->Command.Action=UnitActionBuilded;
    build->Command.Data.Builded.Sum=0;  // FIXME: Is it necessary?
    build->Command.Data.Builded.Val=stats->HitPoints;
    n=(stats->Costs[TimeCost]*FRAMES_PER_SECOND/6)/(SpeedBuild*5);
    build->Command.Data.Builded.Add=stats->HitPoints/n;
    build->Command.Data.Builded.Sub=n;
    build->Command.Data.Builded.Cancel=0; // FIXME: Is it necessary?
    build->Command.Data.Builded.Worker=unit;
    DebugLevel3Fn("Build Sum %d, Add %d, Val %d, Sub %d\n"
		,build->Command.Data.Builded.Sum
		,build->Command.Data.Builded.Add
		,build->Command.Data.Builded.Val
		,build->Command.Data.Builded.Sub);
    build->Wait=5;

    //
    //	Building oil-platform, must remove oilpatch.
    //
    if( type->GivesOil ) {
        DebugLevel0Fn("Remove oil-patch\n");
	temp=OilPatchOnMap(x,y);
	DebugCheck( !temp );
	unit->Value=temp->Value;	// Let peon hold value while building
	// FIXME: oil patch should NOT make sound
	DestroyUnit(temp);		// Destroy oil patch
    }

    RemoveUnit(unit);
    unit->X=x;
    unit->Y=y;
    unit->Command.Action=UnitActionStill;

    if( UnitVisible(build) ) {
        MustRedraw|=RedrawMaps;
    }
}

/**
**	Unit under Construction
**
**	@param unit	Unit that is builded.
*/
global void HandleActionBuilded(Unit* unit)
{
    Unit* peon;
    UnitType* type;

    type=unit->Type;

    //
    // Check if construction should be canceled...
    //
    if( unit->Command.Data.Builded.Cancel ) {
	// Drop out unit
	peon=unit->Command.Data.Builded.Worker;
	peon->Reset=1;
	peon->Wait=1;
	peon->Command.Action=UnitActionStill;
	unit->Command.Data.Builded.Worker=NULL;
	DropOutOnSide(peon,LookingW,type->TileWidth,type->TileHeight);
	// Cancel building
	DestroyUnit(unit);
	return;
    }

    // FIXME: if attacked subtract hit points!!

    unit->Command.Data.Builded.Val-=unit->Command.Data.Builded.Sub;
    if( unit->Command.Data.Builded.Val<0 ) {
	unit->Command.Data.Builded.Val+=unit->Stats->HitPoints;
	unit->HP++;
	unit->Command.Data.Builded.Sum++;
    }
    unit->HP+=unit->Command.Data.Builded.Add;
    unit->Command.Data.Builded.Sum+=unit->Command.Data.Builded.Add;

    //
    //	Check if building ready.
    //
    if( unit->Command.Data.Builded.Sum>=unit->Stats->HitPoints 
		|| unit->HP>=unit->Stats->HitPoints ) {
	if( unit->HP>unit->Stats->HitPoints ) {
	    unit->HP=unit->Stats->HitPoints;
	}
	unit->Command.Action=UnitActionStill;
	// HACK: the building is ready now
	unit->Player->UnitTypesCount[type->Type]++;
	unit->Constructed=0;
	unit->Frame=0;
	unit->Reset=1;
	unit->Wait=1;

	peon=unit->Command.Data.Builded.Worker;
	peon->Command.Action=UnitActionStill;
	peon->Reset=1;
	peon->Wait=1;
	DropOutOnSide(peon,LookingW,type->TileWidth,type->TileHeight);

	//
	//	Building oil-platform, must update oil.
	//
	if( type->GivesOil ) {
	    CommandHaulOil(peon,unit,0);	// Let the unit haul oil
	    DebugLevel0Fn("Update oil-platform\n");
	    DebugLevel0Fn(" =%d\n",unit->Command.Data.OilWell.Active);
	    unit->Command.Data.OilWell.Active=0;
	    unit->Value=peon->Value;		// peon was holding value while building
	}

	// FIXME: General message system
	if( unit->Player==ThisPlayer ) {
	    SetMessage("Work complete");
	    PlayUnitSound(peon,VoiceWorkCompleted);
	} else {
	    AiWorkComplete(peon,unit);
	}

	// FIXME: Vladi: this is just a hack to test wall fixing,
	// FIXME: 	also not sure if the right place...
	// FIXME: Johns: and now this is also slow
	if ( unit->Type == UnitTypeByIdent("unit-orc-wall")
		    || unit->Type == UnitTypeByIdent("unit-human-wall")) {
	    MapSetWall(unit->X, unit->Y,
		    unit->Type == UnitTypeByIdent("unit-human-wall"));
	    if( UnitVisible(unit) ) {
		MustRedraw|=RedrawMap;
	    }
	    RemoveUnit( unit );
	    UnitLost(unit);
	    ReleaseUnit(unit);
	    return;
        }

	UpdateForNewUnit(unit,0);

	if( IsSelected(unit) ) {
	    UpdateButtonPanel();
	    MustRedraw|=RedrawPanels;
	}
	if( UnitVisible(unit) ) {
	    MustRedraw|=RedrawMap;
	}
	return;
    }

    //
    //	Update building states
    //
    if( unit->Command.Data.Builded.Sum*2>=unit->Stats->HitPoints ) {
        if( (unit->Frame!=1 || unit->Constructed)
	        && UnitVisible(unit) ) {
	    MustRedraw|=RedrawMap;
	}
	unit->Constructed=0;
	unit->Frame=1;
    } else if( unit->Command.Data.Builded.Sum*4>=unit->Stats->HitPoints ) {
        if( unit->Frame!=1 && UnitVisible(unit) ) {
	    MustRedraw|=RedrawMap;
	}
	unit->Frame=1;
    }
    
    unit->Wait=5;
    if( IsSelected(unit) ) {
        MustRedraw|=RedrawInfoPanel;
    }
}

//@}
