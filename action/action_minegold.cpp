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
/**@name action_minegold.c -	The mine gold action. */
//
//	(c) Copyright 1998-2001 by Lutz Sammer
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
--	Includes
----------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>

#include "freecraft.h"
#include "unittype.h"
#include "player.h"
#include "unit.h"
#include "actions.h"
#include "tileset.h"
#include "map.h"
#include "interface.h"
#include "pathfinder.h"

/*----------------------------------------------------------------------------
--	Functions
----------------------------------------------------------------------------*/

/**
**	Move to goldmine.
**
**	@param unit	Pointer to worker unit.
**
**	@return		TRUE if reached, otherwise FALSE.
*/
local int MoveToGoldMine(Unit* unit)
{
    Unit* destu;
    int i;

    if( (i=DoActionMove(unit))>=0 ) {	// reached end-point?
	return 0;
    }

    destu=unit->Orders[0].Goal;

    DebugCheck( !destu );
    DebugCheck( unit->Wait!=1 );

    //
    //	Target is dead, stop mining.
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

    DebugCheck( unit->Orders[0].Action!=UnitActionMineGold );

    //
    //	Check if gold-mine could be reached.
    //
    if( i==PF_UNREACHABLE ) {
	// FIXME: could try another mine, or retry later.
	DebugLevel3Fn("GOLD-MINE NOT REACHED %d=%d,%d ? %d\n"
	      ,UnitNumber(destu),destu->X,destu->Y,
	      MapDistanceToUnit(unit->X,unit->Y,destu));
	return -1;
    }

    RefsDebugCheck( !destu->Refs );
    --destu->Refs;
    RefsDebugCheck( !destu->Refs );
    unit->Orders[0].Goal=NoUnitP;

    //
    //	Activate gold-mine
    //
    // FIXME: hmmm... we're in trouble here.
    // we should check if there's still some gold left in the mine instead.

    destu->Data.Resource.Active++;

    if( !destu->Frame ) {
	destu->Frame=1;			// FIXME: should be configurable
	CheckUnitToBeDrawn(destu);
    }

    RemoveUnit(unit);
    unit->X=destu->X;
    unit->Y=destu->Y;

    if( MINE_FOR_GOLD<MAX_UNIT_WAIT ) {
	unit->Wait=MINE_FOR_GOLD;
    } else {
	unit->Wait=MAX_UNIT_WAIT;
    }
    unit->Value=MINE_FOR_GOLD-unit->Wait;

    return 1;
}

/**
**	Mine gold in goldmine.
**
**	@param unit	Pointer to worker unit.
**
**	@return		TRUE if ready, otherwise FALSE.
*/
local int MineInGoldmine(Unit* unit)
{
    Unit* mine;
    Unit* destu;

    DebugLevel3Fn("Waiting\n");
    if( !unit->Value ) {
	//
	//	Have gold
	//
	mine=GoldMineOnMap(unit->X,unit->Y);
	IfDebug(
	    DebugLevel3Fn("Found %d,%d=%d\n",unit->X,unit->Y,UnitNumber(mine));
	    if( !mine ) {
		DebugLevel0Fn("No unit? (%d,%d)\n",unit->X,unit->Y);
		abort();
	    } );

	//
	//	Update gold mine.
	//
	mine->Value-=DEFAULT_INCOMES[GoldCost];	// remove gold from store
	if( !--mine->Data.Resource.Active ) {
	    mine->Frame=0;
	    CheckUnitToBeDrawn(mine);
	}
	if( IsOnlySelected(mine) ) {
	    MustRedraw|=RedrawInfoPanel;
	}

	//
	//	End of gold: destroy gold-mine.
	//
	if( mine->Value<DEFAULT_INCOMES[GoldCost] ) {
	    DebugLevel0Fn("Mine destroyed %d,%d\n",mine->X,mine->Y);
	    DropOutAll(mine);
	    LetUnitDie(mine);
	    mine=NULL;
	}

	//	Store gold mine position
	unit->Orders[0].Arg1=(void*)((unit->X<<16)|unit->Y);

	//
	//	Find gold depot
	//
	if( !(destu=FindGoldDeposit(unit,unit->X,unit->Y)) ) {
	    if( mine ) {
		DropOutOnSide(unit,LookingW
			,mine->Type->TileWidth,mine->Type->TileHeight);
	    }
	    unit->Orders[0].Action=UnitActionStill;
	    unit->SubAction=0;
	    DebugLevel2Fn("Mine without deposit\n");
	} else {
	    if( mine ) {
		DropOutNearest(unit,destu->X+destu->Type->TileWidth/2
			,destu->Y+destu->Type->TileHeight/2
			,mine->Type->TileWidth,mine->Type->TileHeight);
	    }
	    unit->Orders[0].Goal=destu;
	    NewResetPath(unit);
	    RefsDebugCheck( destu->Destroyed || !destu->Refs );
	    ++destu->Refs;
	    unit->Orders[0].RangeX=unit->Orders[0].RangeY=1;
	    unit->Orders[0].X=unit->Orders[0].Y=-1;
	    unit->Orders[0].Action=UnitActionMineGold;
	    unit->SubAction=64;
	    DebugLevel3Fn("Mine with deposit %d,%d\n",destu->X,destu->Y);
	}

	//
	//	Change unit outfit. (Unit type is used for this.)
	//
	unit->Player->UnitTypesCount[unit->Type->Type]--;
	if( unit->Type==UnitTypeOrcWorker ) {
	    unit->Type=UnitTypeOrcWorkerWithGold;
	} else if( unit->Type==UnitTypeHumanWorker ) {
	    unit->Type=UnitTypeHumanWorkerWithGold;
	} else {
	    // FIXME: support workers for more races.
	    DebugLevel0Fn("Wrong unit (%d,%d) for mining gold %d (%s)\n"
		,unit->X,unit->Y,unit->Type->Type,unit->Type->Name);
	}
	unit->Player->UnitTypesCount[unit->Type->Type]++;
        CheckUnitToBeDrawn(unit);
	if( unit->Selected ) {
	    UpdateButtonPanel();
	}
	unit->Wait=1;
	return unit->SubAction;
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
**	Move to gold deposit.
**
**	@param unit	Pointer to worker unit.
**
**	@return		TRUE if ready, otherwise FALSE.
*/
local int MoveToGoldDeposit(Unit* unit)
{
    int i;
    Unit* destu;

    if( (i=DoActionMove(unit))>=0 ) {	// reached end-point?
	return 0;
    }

    destu=unit->Orders[0].Goal;

    DebugCheck( !destu );
    DebugCheck( unit->Wait!=1 );

    //
    //	Target is dead, stop mining.
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

    DebugCheck( unit->Orders[0].Action!=UnitActionMineGold );

    //
    //	If depot is still under construction, wait!
    //
    if( destu->Orders[0].Action==UnitActionBuilded ) {
        DebugLevel2Fn("Invalid depot\n");
	return 0;
    }

    //
    //	Check if depot could be reached.
    //
    if( i==PF_UNREACHABLE ) {
	// FIXME: could try another depot, or retry later.
	DebugLevel3Fn("GOLD-DEPOT NOT REACHED %d=%d,%d ? %d\n"
	      ,UnitNumber(destu),destu->X,destu->Y
	      ,MapDistanceToUnit(unit->X,unit->Y,destu));
	return -1;
    }

    RefsDebugCheck( !destu->Refs );
    --destu->Refs;
    RefsDebugCheck( !destu->Refs );
    unit->Orders[0].Goal=NoUnitP;

    RemoveUnit(unit);
    unit->X=destu->X;
    unit->Y=destu->Y;

    //
    //	Update gold.
    //
    unit->Player->Resources[GoldCost]+=unit->Player->Incomes[GoldCost];
    if( unit->Player==ThisPlayer ) {
	MustRedraw|=RedrawResources;
    }

    unit->Player->UnitTypesCount[unit->Type->Type]--;
    if( unit->Type==UnitTypeOrcWorkerWithGold ) {
	unit->Type=UnitTypeOrcWorker;
    } else if( unit->Type==UnitTypeHumanWorkerWithGold ) {
	unit->Type=UnitTypeHumanWorker;
    } else {
	// FIXME: support workers for more races.
	DebugLevel0Fn("Wrong unit (%d,%d) for returning gold %d (%s)\n"
	    ,unit->X,unit->Y,unit->Type->Type,unit->Type->Name);
    }
    unit->Player->UnitTypesCount[unit->Type->Type]++;

    if( WAIT_FOR_GOLD<MAX_UNIT_WAIT ) {
	unit->Wait=WAIT_FOR_GOLD;
    } else {
	unit->Wait=MAX_UNIT_WAIT;
    }
    unit->Value=WAIT_FOR_GOLD-unit->Wait;

    return 1;
}

/**
**	Store the gold in the gold depot.
**
**	@param unit	Pointer to worker unit.
**
**	@return		TRUE if ready, otherwise FALSE.
*/
local int StoreGoldInDeposit(Unit* unit)
{
    Unit* destu;
    Unit* depot;
    int x;
    int y;

    DebugLevel3Fn("Waiting\n");
    if( !unit->Value ) {
	depot=GoldDepositOnMap(unit->X,unit->Y);
	DebugCheck( !depot );
	// Could be destroyed, but than we couldn't be in?

	// FIXME: Ari says, don't automatic search a new mine.
	// Return to last position
	if( unit->Orders[0].Arg1==(void*)-1 ) {
	    x=unit->X;
	    y=unit->Y;
	} else {
	    x=(int)unit->Orders[0].Arg1>>16;
	    y=(int)unit->Orders[0].Arg1&0xFFFF;
	}
	if( !(destu=FindGoldMine(unit,x,y)) ) {
	    DropOutOnSide(unit,LookingW
		    ,depot->Type->TileWidth,depot->Type->TileHeight);
	    unit->Orders[0].Action=UnitActionStill;
	    unit->SubAction=0;
	} else {
	    DropOutNearest(unit,destu->X+destu->Type->TileWidth/2
		    ,destu->Y+destu->Type->TileHeight/2
		    ,depot->Type->TileWidth,depot->Type->TileHeight);
	    unit->Orders[0].Goal=destu;
	    RefsDebugCheck( destu->Destroyed || !destu->Refs );
	    ++destu->Refs;
	    unit->Orders[0].RangeX=unit->Orders[0].RangeY=1;
	    unit->Orders[0].X=-1;
	    unit->Orders[0].Y=-1;
	    unit->Orders[0].Action=UnitActionMineGold;
	}

        CheckUnitToBeDrawn(unit);
	unit->Wait=1;
	unit->SubAction=0;
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
**	Unit mines gold!
**
**	FIXME: must move to center of gold-mine.
**	XXX
**	XOX
**	XXX	X are move animated!
*/
global void HandleActionMineGold(Unit* unit)
{
    int ret;

    switch( unit->SubAction ) {
	//
	//	Move to gold-mine, 10 tries to reach gold-mine
	//
	case 0:
	    NewResetPath(unit);
	    unit->SubAction=1;
	    // FALL THROUGH
	case 1: case 2: case 3: case 4: case 5:
	case 6: case 7: case 8: case 9: case 10:
	    if( (ret=MoveToGoldMine(unit)) ) {
		if( ret==-1 ) {
		    if( ++unit->SubAction==11 ) {
			unit->Orders[0].Action=UnitActionStill;
			unit->SubAction=0;
			if( unit->Orders[0].Goal ) {
			    RefsDebugCheck( !unit->Orders[0].Goal->Refs );
			    --unit->Orders[0].Goal->Refs;
			    RefsDebugCheck( !unit->Orders[0].Goal->Refs );
			    unit->Orders[0].Goal=NoUnitP;
			}
		    } else {
			//	To keep the load low, retry each 1/4 second.
			unit->Wait=unit->SubAction+FRAMES_PER_SECOND/4;
		    }
		} else {
		    unit->SubAction=64;
		}
	    }
	    break;

	//
	//	Wait for mine gold
	//
	case 64:
	    if( MineInGoldmine(unit) ) {
		++unit->SubAction;
	    }
	    break;

	//
	//	Return to gold deposit, 10 tries to reach gold-depot
	//
	case 65: case 66: case 67: case 68: case 69:
	case 70: case 71: case 72: case 73: case 74:
	    if( (ret=MoveToGoldDeposit(unit)) ) {
		if( ret==-1 ) {
		    if( ++unit->SubAction==75 ) {
			unit->Orders[0].Action=UnitActionStill;
			unit->SubAction=0;
			if( unit->Orders[0].Goal ) {
			    RefsDebugCheck( !unit->Orders[0].Goal->Refs );
			    --unit->Orders[0].Goal->Refs;
			    RefsDebugCheck( !unit->Orders[0].Goal->Refs );
			    unit->Orders[0].Goal=NoUnitP;
			}
		    } else {
			//	To keep the load low, retry each 1/4 second.
			unit->Wait=unit->SubAction-64+FRAMES_PER_SECOND/4;
		    }
		} else {
		    unit->SubAction=128;
		}
	    }
	    break;

	//
	//	Wait for gold stored.
	//
	case 128:
	    if( StoreGoldInDeposit(unit) ) {
		unit->SubAction=0;
	    }
	    break;
    }
}

//@}
