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
//	(c) Copyright 1998-2000 by Lutz Sammer
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

    if( HandleActionMove(unit)>=0 ) {	// reached end-point?
	return 0;
    }
    
    // FIXME: HandleActionMove return this: reached nearly, use it!

    unit->Command.Action=UnitActionMineGold;

    destu=unit->Command.Data.Move.Goal;
    if( destu && (destu->Destroyed || !destu->HP) ) {
	DebugLevel1Fn("WAIT after goldmine destroyed %d\n",unit->Wait);
	if( !--destu->Refs ) {
	    ReleaseUnit(destu);
	}
	unit->Command.Action=UnitActionStill;
	unit->SubAction=0;
	return 0;
    }
    if( !destu || MapDistanceToUnit(unit->X,unit->Y,destu)!=1 ) {
	DebugLevel3Fn("GOLD-MINE NOT REACHED %d,%d\n",dx,dy);
	return -1;
    }

    //
    // Activate gold-mine
    //
    // FIXME: hmmm... we're in trouble here.
    // we should check if there's still some gold left in the mine instead.
    --destu->Refs;
    destu->Command.Data.GoldMine.Active++;
    destu->Frame=1;			// FIXME: should be configurable

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
	// Have gold
	//
	mine=GoldMineOnMap(unit->X,unit->Y);
	IfDebug(
	    DebugLevel3Fn("Found %d,%d=%Zd\n",unit->X,unit->Y,UnitNumber(mine));
	    if( !mine ) {
		DebugLevel0Fn("No unit? (%d,%d)\n",unit->X,unit->Y);
		abort();
	    } );

	//
	//	Update gold mine.
	//
	mine->Value-=DEFAULT_INCOMES[GoldCost];	// remove gold from store
	if( !--mine->Command.Data.GoldMine.Active ) {
	    mine->Frame=0;
	}
	if( IsSelected(mine) ) {
	    MustRedraw|=RedrawInfoPanel;
	}

	//
	//	End of gold: destroy gold-mine.
	//
	if( mine->Value<=DEFAULT_INCOMES[GoldCost] ) {
	    // FIXME: better way to fix bug
	    unit->Removed=0;
	    DropOutAll(mine);
	    DestroyUnit(mine);
	    if( mine->Value<DEFAULT_INCOMES[GoldCost] ) {
		// FIXME: should return 0 here?
		DebugLevel0Fn("Too less gold\n");
	    }
	}
	// FIXME: I use goldmine after destory!!!

	if( !(destu=FindGoldDeposit(unit,unit->X,unit->Y)) ) {
	    DropOutOnSide(unit,LookingW
		    ,mine->Type->TileWidth,mine->Type->TileHeight);
	    unit->Command.Action=UnitActionStill;
	    unit->SubAction=0;
	    DebugLevel2Fn("Mine without deposit\n");
	} else {
	    DropOutNearest(unit
		    ,destu->X,destu->Y
		    ,mine->Type->TileWidth,mine->Type->TileHeight);
	    ResetPath(unit->Command);
	    unit->Command.Data.Move.Goal=destu;
	    ++destu->Refs;
	    unit->Command.Data.Move.Range=1;
#if 1
	    // FIXME: old pathfinder didn't found the path to the nearest
	    // FIXME: point of the unit
	    NearestOfUnit(destu,unit->X,unit->Y
		,&unit->Command.Data.Move.DX
		,&unit->Command.Data.Move.DY);
#else
	    unit->Command.Data.Move.DX=destu->X;
	    unit->Command.Data.Move.DY=destu->Y;
#endif
	    unit->Command.Action=UnitActionMineGold;
	    DebugLevel3Fn("Mine with deposit %d,%d\n",destu->X,destu->Y);
	}

	if( unit->Type==UnitTypeOrcWorker ) {
	    unit->Type=UnitTypeOrcWorkerWithGold;
	} else if( unit->Type==UnitTypeHumanWorker ) {
	    unit->Type=UnitTypeHumanWorkerWithGold;
	} else {
	    DebugLevel0Fn("Wrong unit (%d,%d) for mining gold %d (%s)\n"
		,unit->X,unit->Y
		,unit->Type->Type,unit->Type->Name);
	}
	if( UnitVisible(unit) ) {
	    MustRedraw|=RedrawMap;
	}
	if( IsSelected(unit) ) {
	    UpdateButtonPanel();
	    MustRedraw|=RedrawButtonPanel;
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
    int x;
    int y;
    Unit* destu;

    if( HandleActionMove(unit)>=0 ) {	// reached end-point?
	return 0;
    }
    
    // FIXME: HandleActionMove return this: reached nearly, use it!

    unit->Command.Action=UnitActionMineGold;

    destu=unit->Command.Data.Move.Goal;

    if( destu && (destu->Destroyed || !destu->HP) ) {
	if( !--destu->Refs ) {
	    ReleaseUnit(destu);
	}
	unit->Command.Action=UnitActionStill;
	unit->SubAction=0;
	return 0;
    }

    x=unit->Command.Data.Move.DX;
    y=unit->Command.Data.Move.DY;
    DebugCheck( destu!=GoldDepositOnMap(x,y) );

    if( !destu || MapDistanceToUnit(unit->X,unit->Y,destu)!=1 ) {
	DebugLevel3Fn("GOLD-DEPOSIT NOT REACHED %Zd=%d,%d ? %d\n"
	    ,UnitNumber(destu),x,y
	    ,MapDistanceToUnit(unit->X,unit->Y,destu));
	return -1;
    }

    --destu->Refs;

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
    
    if( unit->Type==UnitTypeOrcWorkerWithGold ) {
	unit->Type=UnitTypeOrcWorker;
    } else if( unit->Type==UnitTypeHumanWorkerWithGold ) {
	unit->Type=UnitTypeHumanWorker;
    } else {
	DebugLevel0Fn("Wrong unit (%d,%d) for returning gold %d (%s)\n"
	    ,unit->X,unit->Y
	    ,unit->Type->Type,unit->Type->Name);
    }

    if( WAIT_FOR_GOLD<MAX_UNIT_WAIT ) {
	unit->Wait=WAIT_FOR_GOLD;
    } else {
	unit->Wait=MAX_UNIT_WAIT;
    }
    unit->Value=WAIT_FOR_GOLD-unit->Wait;

    return 1;
}

/**
**	Store gold in deposit.
**
**	@param unit	Pointer to worker unit.
**
**	@return		TRUE if ready, otherwise FALSE.
*/
local int StoreGoldInDeposit(Unit* unit)
{
    Unit* destu;
    Unit* depot;

    DebugLevel3Fn("Waiting\n");
    if( !unit->Value ) {
	depot=unit->Command.Data.Move.Goal;
	// Could be destroyed, but than we couldn't be in?

	// FIXME: return to last position!
	// FIXME: Ari says, don't automatic search a new mine.
	if( !(destu=FindGoldMine(unit,unit->X,unit->Y)) ) {
	    DropOutOnSide(unit,LookingW
		    ,depot->Type->TileWidth,depot->Type->TileHeight);
	    unit->Command.Action=UnitActionStill;
	    unit->SubAction=0;
	} else {
	    DropOutNearest(unit,destu->X,destu->Y
		    ,depot->Type->TileWidth,depot->Type->TileHeight);
	    ResetPath(unit->Command);
	    unit->Command.Data.Move.Goal=destu;
	    ++destu->Refs;
	    unit->Command.Data.Move.Range=1;
#if 1
	    // FIXME: old pathfinder didn't found the path to the nearest
	    // FIXME: point of the unit
	    NearestOfUnit(destu,unit->X,unit->Y
		,&unit->Command.Data.Move.DX,&unit->Command.Data.Move.DY);
#else
	    unit->Command.Data.Move.DX=destu->X;
	    unit->Command.Data.Move.DY=destu->Y;
#endif
	    unit->Command.Action=UnitActionMineGold;
	}

	if( UnitVisible(unit) ) {
	    MustRedraw|=RedrawMap;
	}
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
	//	Move to gold-mine
	//
	case 0:
	case 1:
	case 2:
	case 3:
	case 4:					// 4 tries to reach gold-mine
	    if( (ret=MoveToGoldMine(unit)) ) {
		if( ret==-1 ) {
		    if( ++unit->SubAction==5 ) {
			unit->Command.Action=UnitActionStill;
			unit->SubAction=0;
			if( unit->Command.Data.Move.Goal ) {
			    --unit->Command.Data.Move.Goal->Refs;
			}
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
	//	Return to gold deposit
	//
	case 65:
	case 66:
	case 67:
	case 68:				// 4 tries to reach depot
	    if( (ret=MoveToGoldDeposit(unit)) ) {
		if( ret==-1 ) {
		    if( ++unit->SubAction==69 ) {
			unit->Command.Action=UnitActionStill;
			unit->SubAction=0;
			if( unit->Command.Data.Move.Goal ) {
			    --unit->Command.Data.Move.Goal->Refs;
			}
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
