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

    if( (i=HandleActionMove(unit))>=0 ) {	// reached end-point?
	return 0;
    }

#ifdef NEW_ORDERS
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

    unit->Orders[0].Action=UnitActionMineGold;

    //
    //	Check if mine could be reached.
    /
    if( i==PF_UNREACHABLE ) {
	// FIXME: could try another mine, or retry later.
	DebugLevel3Fn("GOLD-MINE NOT REACHED %Zd=%d,%d ? %d\n"
	      ,UnitNumber(destu),x,y,MapDistanceToUnit(unit->X,unit->Y,destu));
	return -1;
    }

    RefsDebugCheck( !destu->Refs );
    --destu->Refs;
    RefsDebugCheck( !destu->Refs );
    unit->Orders[0].Goal=NoUnitP;

    //
    // Activate gold-mine
    //
    // FIXME: hmmm... we're in trouble here.
    // we should check if there's still some gold left in the mine instead.

    destu->Data.Resource.Active++;
#else
    destu=unit->Command.Data.Move.Goal;

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
	unit->Command.Data.Move.Goal=NoUnitP;
	// FIXME: perhaps we should choose an alternative
	unit->Command.Action=UnitActionStill;
	unit->SubAction=0;
	return 0;
    } else if( destu->Removed || !destu->HP
	    || destu->Command.Action==UnitActionDie ) {
	RefsDebugCheck( !destu->Refs );
	--destu->Refs;
	RefsDebugCheck( !destu->Refs );
	unit->Command.Data.Move.Goal=NoUnitP;
	// FIXME: perhaps we should choose an alternative
	unit->Command.Action=UnitActionStill;
	unit->SubAction=0;
	return 0;
    }

    unit->Command.Action=UnitActionMineGold;

    //
    //	Check if mine could be reached.
    //
    if( i==PF_UNREACHABLE ) {
	// FIXME: could try another mine, or retry later.
	DebugLevel3Fn("GOLD-MINE NOT REACHED %Zd=%d,%d ? %d\n"
	      ,UnitNumber(destu),destu->X,destu->Y
	      ,MapDistanceToUnit(unit->X,unit->Y,destu));
	return -1;
    }

    RefsDebugCheck( !destu->Refs );
    --destu->Refs;
    RefsDebugCheck( !destu->Refs );
    unit->Command.Data.Move.Goal=NoUnitP;

    //
    // Activate gold-mine
    //
    // FIXME: hmmm... we're in trouble here.
    // we should check if there's still some gold left in the mine instead.

    destu->Command.Data.GoldMine.Active++;
#endif
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
#ifdef NEW_ORDERS
	if( !--mine->Data.Resource.Active ) {
	    mine->Frame=0;
	    CheckUnitToBeDrawn(mine);
	}
#else
	if( !--mine->Command.Data.GoldMine.Active ) {
	    mine->Frame=0;
	    CheckUnitToBeDrawn(mine);
	}
#endif
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

	//
	//	Find gold depot
	//
#ifdef NEW_ORDERS
	if( !(destu=FindGoldDeposit(unit,unit->X,unit->Y)) ) {
	    DropOutOnSide(unit,LookingW
		    ,mine->Type->TileWidth,mine->Type->TileHeight);
	    unit->Orders[0].Action=UnitActionStill;
	    unit->SubAction=0;
	    DebugLevel2Fn("Mine without deposit\n");
	} else {
	    DropOutNearest(unit
		    ,destu->X,destu->Y
		    ,mine->Type->TileWidth,mine->Type->TileHeight);
	    ResetPath(unit->Orders[0]);
	    unit->Orders[0].Goal=destu;
	    RefsDebugCheck( !destu->Refs );
	    ++destu->Refs;
	    unit->Orders[0].RangeX=unit->Orders[0].RangeY=1;
	    unit->Orders[0].X=-1;
	    unit->Orders[0].Y=-1;
	    unit->Orders[0].Action=UnitActionMineGold;
	    DebugLevel3Fn("Mine with deposit %d,%d\n",destu->X,destu->Y);
	}
#else
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
	    RefsDebugCheck( !destu->Refs );
	    ++destu->Refs;
	    unit->Command.Data.Move.Range=1;
	    unit->Command.Data.Move.DX=-1;
	    unit->Command.Data.Move.DY=-1;
	    unit->Command.Action=UnitActionMineGold;
	    DebugLevel3Fn("Mine with deposit %d,%d\n",destu->X,destu->Y);
	}
#endif

	//
	//	Change unit outfit. (Unit type is used for this.)
	//
	if( unit->Type==UnitTypeOrcWorker ) {
	    unit->Type=UnitTypeOrcWorkerWithGold;
	} else if( unit->Type==UnitTypeHumanWorker ) {
	    unit->Type=UnitTypeHumanWorkerWithGold;
	} else {
	    DebugLevel0Fn("Wrong unit (%d,%d) for mining gold %d (%s)\n"
		,unit->X,unit->Y,unit->Type->Type,unit->Type->Name);
	}
        CheckUnitToBeDrawn(unit);
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
    int i;
    Unit* destu;

    if( (i=HandleActionMove(unit))>=0 ) {	// reached end-point?
	return 0;
    }

#ifdef NEW_ORDERS
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

    unit->Orders[0].Action=UnitActionMineGold;

    //
    //	Check if depot could be reached.
    /
    if( i==PF_UNREACHABLE ) {
	// FIXME: could try another mine, or retry later.
	DebugLevel3Fn("GOLD-DEPOT NOT REACHED %Zd=%d,%d ? %d\n"
	      ,UnitNumber(destu),x,y,MapDistanceToUnit(unit->X,unit->Y,destu));
	return -1;
    }

    RefsDebugCheck( !destu->Refs );
    --destu->Refs;
    RefsDebugCheck( !destu->Refs );
    unit->Orders[0].Goal=NoUnitP;

#else

    destu=unit->Command.Data.Move.Goal;

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
	unit->Command.Data.Move.Goal=NoUnitP;
	// FIXME: perhaps we should choose an alternative
	unit->Command.Action=UnitActionStill;
	unit->SubAction=0;
	return 0;
    } else if( destu->Removed || !destu->HP
	    || destu->Command.Action==UnitActionDie ) {
	RefsDebugCheck( !destu->Refs );
	--destu->Refs;
	RefsDebugCheck( !destu->Refs );
	unit->Command.Data.Move.Goal=NoUnitP;
	// FIXME: perhaps we should choose an alternative
	unit->Command.Action=UnitActionStill;
	unit->SubAction=0;
	return 0;
    }

    unit->Command.Action=UnitActionMineGold;

    //
    //	Check if mine could be reached.
    //
    if( i==PF_UNREACHABLE ) {
	// FIXME: could try another mine, or retry later.
	DebugLevel3Fn("GOLD-MINE NOT REACHED %Zd=%d,%d ? %d\n"
	      ,UnitNumber(destu),destu->X,destu->Y
	      ,MapDistanceToUnit(unit->X,unit->Y,destu));
	return -1;
    }

    RefsDebugCheck( !destu->Refs );
    --destu->Refs;
    RefsDebugCheck( !destu->Refs );
    unit->Command.Data.Move.Goal=NoUnitP;

#endif

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
	    ,unit->X,unit->Y,unit->Type->Type,unit->Type->Name);
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

    DebugLevel3Fn("Waiting\n");
    if( !unit->Value ) {
#ifdef NEW_ORDERS
	depot=GoldDepositOnMap(unit->X,unit->Y);
	DebugCheck( !depot );
	// Could be destroyed, but than we couldn't be in?

	// FIXME: return to last position!
	// FIXME: Ari says, don't automatic search a new mine.
	if( !(destu=FindGoldMine(unit,unit->X,unit->Y)) ) {
	    DropOutOnSide(unit,LookingW
		    ,depot->Type->TileWidth,depot->Type->TileHeight);
	    unit->Orders[0].Action=UnitActionStill;
	    unit->SubAction=0;
	} else {
	    DropOutNearest(unit,destu->X,destu->Y
		    ,depot->Type->TileWidth,depot->Type->TileHeight);
	    ResetPath(unit->Command);
	    unit->Orders[0].Goal=destu;
	    RefsDebugCheck( !destu->Refs );
	    ++destu->Refs;
	    unit->Orders[0].RangeX=unit->Orders[0].RangeY=1;
	    unit->Orders[0].X=-1;
	    unit->Orders[0].Y=-1;
	    unit->Orders[0].Action=UnitActionMineGold;
#else
	depot=GoldDepositOnMap(unit->X,unit->Y);
	DebugCheck( !depot );
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
	    RefsDebugCheck( !destu->Refs );
	    ++destu->Refs;
	    unit->Command.Data.Move.Range=1;
	    unit->Command.Data.Move.DX=-1;
	    unit->Command.Data.Move.DY=-1;
	    unit->Command.Action=UnitActionMineGold;
#endif
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
#ifdef NEW_ORDERS
			unit->Orders[0].Action=UnitActionStill;
			unit->SubAction=0;
			if( unit->Orders[0].Goal ) {
			    RefsDebugCheck( !unit->Orders[0].Goal->Refs );
			    --unit->Orders[0].Goal->Refs;
			    RefsDebugCheck( !unit->Orders[0].Goal->Refs );
			    unit->Orders[0].Goal=NoUnitP;
			}
#else
			unit->Command.Action=UnitActionStill;
			unit->SubAction=0;
			if( unit->Command.Data.Move.Goal ) {
			    RefsDebugCheck(!unit->Command.Data.Move.Goal->Refs);
			    --unit->Command.Data.Move.Goal->Refs;
			    RefsDebugCheck(!unit->Command.Data.Move.Goal->Refs);
			    unit->Command.Data.Move.Goal=NoUnitP;
			}
#endif
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
	case 68:
	case 69:				// 4 tries to reach depot
	    if( (ret=MoveToGoldDeposit(unit)) ) {
		if( ret==-1 ) {
		    if( ++unit->SubAction==70 ) {
#ifdef NEW_ORDERS
			unit->Orders[0].Action=UnitActionStill;
			unit->SubAction=0;
			if( unit->Orders[0].Goal ) {
			    RefsDebugCheck( !unit->Orders[0].Goal->Refs );
			    --unit->Orders[0].Goal->Refs;
			    RefsDebugCheck( !unit->Orders[0].Goal->Refs );
			    unit->Orders[0].Goal=NoUnitP;
			}
#else
			unit->Command.Action=UnitActionStill;
			unit->SubAction=0;
			if( unit->Command.Data.Move.Goal ) {
			    RefsDebugCheck(!unit->Command.Data.Move.Goal->Refs);
			    --unit->Command.Data.Move.Goal->Refs;
			    RefsDebugCheck(!unit->Command.Data.Move.Goal->Refs);
			    unit->Command.Data.Move.Goal=NoUnitP;
			}
#endif
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
