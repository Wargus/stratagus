//   ___________		     _________		      _____  __
//   \_	  _____/______   ____   ____ \_   ___ \____________ _/ ____\/  |_
//    |    __) \_  __ \_/ __ \_/ __ \/    \  \/\_  __ \__  \\   __\\   __|
//    |     \   |  | \/\  ___/\  ___/\     \____|  | \// __ \|  |   |  |
//    \___  /   |__|    \___  >\___  >\______  /|__|  (____  /__|   |__|
//	  \/		    \/	   \/	     \/		   \/
//  ______________________                           ______________________
//			  T H E   W A R   B E G I N S
//	   Stratagus - A free fantasy real time strategy game engine
//
/**@name action_harvest.c -	The harvest action. */
//
//	(c) Copyright 1998-2001,2003 by Lutz Sammer
//
//	Stratagus is free software; you can redistribute it and/or modify
//	it under the terms of the GNU General Public License as published
//	by the Free Software Foundation; only version 2 of the License.
//
//	Stratagus is distributed in the hope that it will be useful,
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

#include "stratagus.h"
#include "unittype.h"
#include "player.h"
#include "unit.h"
#include "actions.h"
#include "sound.h"
#include "tileset.h"
#include "map.h"
#include "interface.h"
#include "pathfinder.h"

// FIXME: Should combine all the resource functions
// FIXME: Should update buttons if the action changes?

/*----------------------------------------------------------------------------
--	Functions
----------------------------------------------------------------------------*/

/**
**	Move to forest.
**
**	@param unit	Pointer to worker unit.
**
**	@return		TRUE if reached, otherwise FALSE.
*/
local int MoveToWood(Unit* unit)
{
    int i;
    int x;
    int y;

    if( (i=DoActionMove(unit))>=0 ) {	// reached end-point?
	return 0;
    }

    if( i==PF_UNREACHABLE ) {
newtry:
	if( FindWoodInSight(unit,&unit->Orders[0].X,&unit->Orders[0].Y) ) {

	    // Move to new wood position
	    unit->Orders[0].Goal=NoUnitP;
	    unit->Orders[0].RangeX=unit->Orders[0].RangeY=1;
	    NewResetPath(unit);
	    return 0;
	}

	// FIXME: Should do more tries
	unit->Orders[0].Action=UnitActionStill;
	unit->SubAction=0;
	return 0;
    }

    x=unit->Orders[0].X;
    y=unit->Orders[0].Y;

    if( !CheckedForestOnMap(x,y) ) {
	//
	//	Fast check surrounding for forest
	//
	x=unit->X;
	y=unit->Y;
	if( CheckedForestOnMap(x-1,y-1) ) {
	    --x;
	    --y;
	} else if( CheckedForestOnMap(x-1,y+0) ) {
	    --x;
	} else if( CheckedForestOnMap(x-1,y+1) ) {
	    --x;
	    ++y;
	} else if( CheckedForestOnMap(x+0,y-1) ) {
	    --y;
	} else if( CheckedForestOnMap(x+0,y+1) ) {
	    ++y;
	} else if( CheckedForestOnMap(x+1,y-1) ) {
	    ++x;
	    --y;
	} else if( CheckedForestOnMap(x+1,y+0) ) {
	    ++x;
	} else if( CheckedForestOnMap(x+1,y+1) ) {
	    ++x;
	    ++y;
	} else {
	    DebugLevel3Fn("No Wood, Trying a better spot\n");
	    goto newtry;
	}
    }

    // FIXME: don't chop the same wood!

    unit->Orders[0].X=x;
    unit->Orders[0].Y=y;
    unit->Orders[0].RangeX=unit->Orders[0].RangeY=0;
    DebugCheck( unit->Orders[0].Action!=UnitActionHarvest );
    // turn to wood
    UnitHeadingFromDeltaXY(unit,x-unit->X,y-unit->Y);
    //unit->Value=unit->Data.Harvest.WoodToHarvest;

    DebugCheck( unit->Wait!=1 );

    return 1;
}

/**
**	Chop the wood.
**
**	@param unit	Pointer to worker unit.
**
**	@return		TRUE if ready, otherwise FALSE.
*/
local int ChopWood(Unit* unit)
{
    Unit* destu;
    int flags;

    flags=UnitShowAnimation(unit,unit->Type->Animations->Attack);

    if( (flags&AnimationSound) && (UnitVisibleOnMap(unit) || ReplayRevealMap) ) {
	PlayUnitSound(unit,VoiceTreeChopping);
    }

    if( unit->Reset ) {

	DebugCheck( unit->Wait!=1 );

	//
	//	This a work around the bug: "lumber bug"
	//		We give a worker a new command and in the next cycle
	//		the worker is ready chopping.
	//
#if 1
	if( unit->Orders[1].Action==UnitActionHarvest
		 || unit->Orders[1].Action==UnitActionMineGold ) {
	    unit->Orders[0].Action=UnitActionStill;
	    unit->SubAction=0;
	    return 0;
	}
#endif

	//
	//	Wood gone while chopping?
	//
	if( !ForestOnMap(unit->Orders[0].X,unit->Orders[0].Y) ) {
	    if( FindWoodInSight(unit,&unit->Orders[0].X,&unit->Orders[0].Y) ) {
		unit->Orders[0].Goal=NoUnitP;
		unit->Orders[0].RangeX=unit->Orders[0].RangeY=0;
		DebugCheck( unit->Orders[0].Action!=UnitActionHarvest );
		unit->Orders[0].X--;
		unit->Orders[0].Y--;
		NewResetPath(unit);
		unit->SubAction=1;
	    } else {
		unit->Orders[0].Action=UnitActionStill;
		unit->SubAction=0;
		DebugLevel3Fn("NO-WOOD in sight range\n");
	    }
	    return 0;
	}

	//
	//	Ready chopping wood?
	//
	if( !--unit->Value ) {
	    // Have wood
	    unit->Player->UnitTypesCount[unit->Type->Type]--;
	    if( unit->Type==UnitTypeOrcWorker ) {
		unit->Type=UnitTypeOrcWorkerWithWood;
	    } else if( unit->Type==UnitTypeHumanWorker ) {
		unit->Type=UnitTypeHumanWorkerWithWood;
	    } else {
		// FIXME: support more races!
		DebugLevel0Fn("Wrong unit for chopping wood %d\n"
			_C_ unit->Type->Type);
	    }
	    unit->Player->UnitTypesCount[unit->Type->Type]++;

	    //
	    //	Update the display.
	    //
            CheckUnitToBeDrawn(unit);
	    if( IsOnlySelected(unit) ) {	// update display
		UpdateButtonPanel();
	    }

	    //
	    //	Update the map.
	    //
	    MapRemoveWood(unit->Orders[0].X,unit->Orders[0].Y);
#if 0
	    /*
	     * This call turned out not to be sufficient because MapRemoveWood()
	     * may actually remove more that one wood tile (x,y) due to
	     * "wood fixing" code, see action_harvest.c . So the pathfinder
	     * callback had to moved directly into MapRemoveWood().
	     */
#ifdef HIERARCHIC_PATHFINDER
	    PfHierMapChangedCallback (unit->Orders[0].X, unit->Orders[0].Y,
			unit->Orders[0].X, unit->Orders[0].Y);
#endif
#endif

	    //
	    //	Find place to return wood.
	    //
	    // NOTE: unit->Orders[0].X && unit->Orders[0].Y holds return place.
	    unit->Orders[0].X=unit->X;
	    unit->Orders[0].Y=unit->Y;
	    if( !(destu=FindWoodDeposit(unit->Player,unit->X,unit->Y)) ) {
		unit->Orders[0].Action=UnitActionStill;
		unit->SubAction=0;
	    } else {
		unit->Orders[0].RangeX=unit->Orders[0].RangeY=0;
		unit->Orders[0].Goal=destu;
		RefsDebugCheck( !destu->Refs );
		destu->Refs++;
		DebugCheck( unit->Orders[0].Action!=UnitActionHarvest );
		NewResetPath(unit);
		return 1;
	    }
	}
    }
    return 0;
}

/**
**	Return with the wood.
**
**	@param unit	unit pointer.
**
**	@return		TRUE if reached, otherwise FALSE.
*/
local int ReturnWithWood(Unit* unit)
{
    Unit* destu;
    int i;

    destu=unit->Orders[0].Goal;
    DebugCheck( !destu );

    i=DoActionMove(unit);
    if( i>=0 && (!unit->Reset || !(destu->Destroyed || destu->Removed
	    || !destu->HP || destu->Orders[0].Action==UnitActionDie)) ) {
	return 0;
    }

    if( !destu ) {
	unit->Orders[0].Action=UnitActionStill;
	unit->SubAction=0;
	return 0;
    }

    //
    //	Target is dead, stop harvest
    //
    if( destu->Destroyed ) {
	DebugLevel0Fn("Destroyed unit\n");
	RefsDebugCheck( !destu->Refs );
	if( !--destu->Refs ) {
	    ReleaseUnit(destu);
	}
	unit->Orders[0].Goal=NoUnitP;
	// FIXME: perhaps we should choose an alternative
	unit->Orders[0].Action=UnitActionStill;
	unit->SubAction=0;
	return 0;
    } else if( destu->Removed || !destu->HP
	    || destu->Orders[0].Action==UnitActionDie ) {
	RefsDebugCheck( !destu->Refs );
	--destu->Refs;
	RefsDebugCheck( !destu->Refs );
	unit->Orders[0].Goal=NoUnitP;
	// FIXME: perhaps we should choose an alternative
	unit->Orders[0].Action=UnitActionStill;
	unit->SubAction=0;
	return 0;
    }

    DebugCheck( unit->Wait!=1 );
    DebugCheck( unit->Orders[0].Action!=UnitActionHarvest );

    //
    //	If depot is still under construction, wait!
    //
    if( destu->Orders[0].Action==UnitActionBuilded ) {
        DebugLevel2Fn("Invalid depot\n");
	return 0;
    }

    unit->Orders[0].Goal=NoUnitP;

    RefsDebugCheck( !destu->Refs );
    --destu->Refs;
    RefsDebugCheck( !destu->Refs );

    if( i==PF_UNREACHABLE ) {
	// FIXME: could try another depot, or retry later.
	DebugLevel2Fn("WOOD-DEPOSIT NOT REACHED %d=%d,%d ? %d\n"
		  _C_ UnitNumber(destu) _C_ destu->X _C_ destu->Y
		  _C_ MapDistanceToUnit(unit->X,unit->Y,destu));
	unit->Orders[0].Action=UnitActionStill;
	unit->SubAction=0;
	return 0;
    }

    RemoveUnit(unit,destu);
#if 0
    // FIXME: this breaks the drop out code.
    // FIXME: this is a hack, but solves the problem, a better solution is
    // FIXME: still wanted.

    // Place unit where pathfinder is more likely to work
    if (unit->X < destu->X) {
	PlaceUnit(unit,destu->X,unit->Y);
 	RemoveUnit(unit,NULL);		// Unit removal necessary to free map tiles
    }
    if (unit->X > destu->X+destu->Type->TileWidth-1) {
	PlaceUnit(unit,destu->X+destu->Type->TileWidth-1,unit->Y);
	RemoveUnit(unit,NULL);
    }
    if (unit->Y < destu->Y) {
	PlaceUnit(unit,unit->X,destu->Y);
	RemoveUnit(unit,NULL);
    }
    if (unit->Y > destu->Y+destu->Type->TileHeight-1) {
	PlaceUnit(unit,unit->X,destu->Y+destu->Type->TileHeight-1);
	RemoveUnit(unit,NULL);
    }
#else
    unit->X=destu->X;
    unit->Y=destu->Y;
#endif

    //
    //	Update wood.
    //
    unit->Player->Resources[WoodCost]+=unit->Player->Incomes[WoodCost];
    unit->Player->TotalResources[WoodCost]+=unit->Player->Incomes[WoodCost];
    if( unit->Player==ThisPlayer ) {
	MustRedraw|=RedrawResources;
    }

    unit->Player->UnitTypesCount[unit->Type->Type]--;
    if( unit->Type==UnitTypeOrcWorkerWithWood ) {
	unit->Type=UnitTypeOrcWorker;
    } else if( unit->Type==UnitTypeHumanWorkerWithWood ) {
	unit->Type=UnitTypeHumanWorker;
    } else {
	// FIXME: must support more races.
	DebugLevel0Fn("Wrong unit for returning wood %d\n" _C_ unit->Type->Type);
    }
    unit->Player->UnitTypesCount[unit->Type->Type]++;

    if( WAIT_FOR_WOOD<MAX_UNIT_WAIT ) {
	unit->Wait=WAIT_FOR_WOOD;
    } else {
	unit->Wait=MAX_UNIT_WAIT;
    }
    unit->Value=WAIT_FOR_WOOD-unit->Wait;

    return 1;
}

/**
**	Wait in wood deposit.
**
**	@param unit	Pointer to worker unit.
*/
local int WaitInWoodDeposit(Unit* unit)
{
    Unit* destu;

    DebugLevel3Fn("Waiting\n");

    if( !unit->Value ) {
	//
	//	Drop out unit at nearest point to target.
	//
	destu=WoodDepositOnMap(unit->X,unit->Y);
	DebugCheck( !destu );		// there must be a depot!

	DropOutNearest(unit
		,unit->Orders[0].X,unit->Orders[0].Y
		,destu->Type->TileWidth,destu->Type->TileHeight);

	//
	//	Return to chop point.
	//
	DebugCheck( unit->Orders[0].Action!=UnitActionHarvest );
	unit->Orders[0].Goal=NoUnitP;
	unit->Orders[0].RangeX=unit->Orders[0].RangeY=0;
	// NOTE: unit->Orders[0].X && unit->Orders[0].Y holds return place.
	NewResetPath(unit);

        CheckUnitToBeDrawn(unit);
	unit->Wait=1;
	//unit->Data.Harvest.WoodToHarvest=CHOP_FOR_WOOD;
	unit->Value=CHOP_FOR_WOOD;
	return 1;
    }

    if( unit->Value<MAX_UNIT_WAIT ) {
	unit->Wait=unit->Value;
    } else {
	unit->Wait=MAX_UNIT_WAIT;
    }
    unit->Value-=unit->Wait;

    return 0;
}

/**
**	Unit Harvest:
**		Move into forest.
**		Chop the tree.
**		Return to wood store.
**		Deliver wood.
**		Restart from beginning.
**
**	FIXME: we should write a generic resource function
**
**	@param unit	Pointer to worker unit.
*/
global void HandleActionHarvest(Unit* unit)
{
    switch( unit->SubAction ) {
	case 0:
	    NewResetPath(unit);
	    unit->Value=CHOP_FOR_WOOD;
	    unit->SubAction=1;
	case 1:
	    if( MoveToWood(unit) ) {
		unit->SubAction=64;
	    }
	    break;

	case 64:
	    if( ChopWood(unit) ) {
		unit->SubAction=128;
	    }
	    break;

	case 128:
	    if( ReturnWithWood(unit) ) {
		unit->SubAction=192;
	    }
	    break;

	case 192:
	    if( WaitInWoodDeposit(unit) ) {
		unit->SubAction=0;
	    }
	    break;
    }
}

//@}
