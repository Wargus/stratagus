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
/**@name action_harvest.c -	The harvest action. */
//
//	(c) Copyright 1998-2001 by Lutz Sammer
//
//	$Id$

//@{

/*----------------------------------------------------------------------------
--	Includes
----------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>

#include "freecraft.h"
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
	    unit->Orders[0].RangeX=unit->Orders[0].RangeY=2;
	    unit->Orders[0].X--;
	    unit->Orders[0].Y--;
	    NewResetPath(unit);
	    return 0;
	}

	// FIXME: Should do more tries
	unit->Orders[0].Action=UnitActionStill;
	unit->SubAction=0;
	return 0;
    }

    x=unit->Orders[0].X+1;
    y=unit->Orders[0].Y+1;

    if( !CheckedForestOnMap(x,y) ) {
	//
	//	Check surrounding for forest
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
	    goto newtry;
	}
    }

    // FIXME: don't chop the same wood!

    unit->Orders[0].X=x;
    unit->Orders[0].Y=y;
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

    if( (flags&AnimationSound) ) {
	PlayUnitSound(unit,VoiceTreeChopping);
    }

    if( unit->Reset ) {

	DebugCheck( unit->Wait!=1 );

	//
	//	This a work around the bug: "lumber bug"
	//		We give a worker a new command and in the next cycle
	//		the worker is ready chopping.
	//
#if 0
	// FIXME: johns+cade: this didn't work with the current code
	if( unit->NextCommand[0].Action==UnitActionHarvest
		 || unit->NextCommand[0].Action==UnitActionMineGold ) {
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
		unit->Orders[0].RangeX=unit->Orders[0].RangeY=2;
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
	    if( unit->Type==UnitTypeOrcWorker ) {
		unit->Type=UnitTypeOrcWorkerWithWood;
	    } else if( unit->Type==UnitTypeHumanWorker ) {
		unit->Type=UnitTypeHumanWorkerWithWood;
	    } else {
		// FIXME: support more races!
		DebugLevel0Fn("Wrong unit for chopping wood %d\n"
			,unit->Type->Type);
	    }

	    //
	    //	Update the display.
	    //
            CheckUnitToBeDrawn(unit);
	    if( unit->Selected ) {
		UpdateButtonPanel();
	    }

	    //
	    //	Update the map.
	    //
	    MapRemoveWood(unit->Orders[0].X,unit->Orders[0].Y);

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
		unit->Orders[0].RangeX=unit->Orders[0].RangeY=1;
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

    if( (i=DoActionMove(unit))>=0 ) {	// reached end-point?
	return 0;
    }

    DebugCheck( unit->Wait!=1 );

    destu=unit->Orders[0].Goal;

    DebugCheck( !destu );

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
	DebugLevel2Fn("WOOD-DEPOSIT NOT REACHED %Zd=%d,%d ? %d\n"
		  ,UnitNumber(destu),destu->X,destu->Y
		  ,MapDistanceToUnit(unit->X,unit->Y,destu));
	unit->Orders[0].Action=UnitActionStill;
	unit->SubAction=0;
	return 0;
    }

    RemoveUnit(unit);
    unit->X=destu->X;
    unit->Y=destu->Y;

    //
    //	Update wood.
    //
    unit->Player->Resources[WoodCost]+=unit->Player->Incomes[WoodCost];
    if( unit->Player==ThisPlayer ) {
	MustRedraw|=RedrawResources;
    }

    if( unit->Type==UnitTypeOrcWorkerWithWood ) {
	unit->Type=UnitTypeOrcWorker;
    } else if( unit->Type==UnitTypeHumanWorkerWithWood ) {
	unit->Type=UnitTypeHumanWorker;
    } else {
	// FIXME: must support more races.
	DebugLevel0Fn("Wrong unit for returning wood %d\n",unit->Type->Type);
    }

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
