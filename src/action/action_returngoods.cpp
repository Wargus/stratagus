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
/**@name action_returngoods.c -	The return goods action. */
//
//	(c) Copyright 1998,2000,2001 by Lutz Sammer
//
//	$Id$

//@{

/*----------------------------------------------------------------------------
--      Include
----------------------------------------------------------------------------*/

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
#include "pathfinder.h"

/*----------------------------------------------------------------------------
--      Functions
----------------------------------------------------------------------------*/

/**
**	Return goods to gold/wood deposit.
**
**	FIXME: must support to move to a specified deposit.
**
**	@param unit	pointer to unit.
*/
global void HandleActionReturnGoods(Unit* unit)
{
    const UnitType* type;
    Unit* destu;

    type=unit->Type;
    //
    //	Select target to return goods.
    //
    if( type==UnitTypeHumanWorkerWithGold || type==UnitTypeOrcWorkerWithGold ) {
#ifdef NEW_ORDERS
	if( !(destu=FindGoldDeposit(unit,unit->X,unit->Y)) ) {
	    // No deposit -> can't return
	    unit->Orders[0].Action=UnitActionStill;
	    return;
	}
	ResetPath(unit->Orders[0]);
	unit->Orders[0].Goal=destu;
	RefsDebugCheck( !destu->Refs );
	++destu->Refs;
	unit->Orders[0].RangeX=unit->Orders[0].RangeY=1;
	unit->Orders[0].X=-1;
	unit->Orders[0].Y=-1;
	unit->Orders[0].Action=UnitActionMineGold;
#else
	if( !(destu=FindGoldDeposit(unit,unit->X,unit->Y)) ) {
	    // No deposit -> can't return
	    unit->Command.Action=UnitActionStill;
	    return;
	}
	ResetPath(unit->Command);
	unit->Command.Data.Move.Goal=destu;
	RefsDebugCheck( !destu->Refs );
	++destu->Refs;
	unit->Command.Data.Move.Range=1;
	unit->Command.Data.Move.DX=-1;
	unit->Command.Data.Move.DY=-1;
	unit->Command.Action=UnitActionMineGold;
#endif
	unit->SubAction=65;	// FIXME: hardcoded
	DebugLevel3("Wait: %d\n",unit->Wait);
	unit->Wait=1;
	return;
    }

    if( type==UnitTypeHumanWorkerWithWood || type==UnitTypeOrcWorkerWithWood ) {
#ifdef NEW_ORDERS
	if( !(destu=FindWoodDeposit(unit->Player,unit->X,unit->Y)) ) {
	    // No deposit -> can't return
	    unit->Orders[0].Action=UnitActionStill;
	    return;
	}
	ResetPath(unit->Orders[0]);
	unit->Orders[0].Goal=destu;
	RefsDebugCheck( !destu->Refs );
	++destu->Refs;
	unit->Orders[0].RangeX=unit->Orders[0].RangeY=1;
	unit->Orders[0].X=-1;
	unit->Orders[0].Y=-1;
	DebugLevel3("Return to %Zd=%d,%d\n"
	    ,UnitNumber(destu),unit->Orders[0].X,unit->Orders[0].Y);
	unit->Orders[0].Action=UnitActionHarvest;
#else
	if( !(destu=FindWoodDeposit(unit->Player,unit->X,unit->Y)) ) {
	    // No deposit -> can't return
	    unit->Command.Action=UnitActionStill;
	    return;
	}
	ResetPath(unit->Command);
	unit->Command.Data.Move.Goal=destu;
	RefsDebugCheck( !destu->Refs );
	++destu->Refs;
	unit->Command.Data.Move.Range=1;
	unit->Command.Data.Move.DX=-1;
	unit->Command.Data.Move.DY=-1;
	DebugLevel3("Return to %Zd=%d,%d\n"
	    ,UnitNumber(destu)
	    ,unit->Command.Data.Move.DX,unit->Command.Data.Move.DY);
	unit->Command.Action=UnitActionHarvest;
#endif
	unit->SubAction=2;		// FIXME: Hardcoded
	DebugLevel3("Wait: %d\n",unit->Wait);
	unit->Wait=1;
	return;
    }

    if( type==UnitTypeHumanTankerFull || type==UnitTypeOrcTankerFull ) {
#ifdef NEW_ORDERS
	if( !(destu=FindOilDeposit(unit->Player,unit->X,unit->Y)) ) {
	    // No deposit -> can't return
	    unit->Orders[0].Action=UnitActionStill;
	    return;
	}
	ResetPath(unit->Orders[0]);
	unit->Orders[0].Goal=destu;
	RefsDebugCheck( !destu->Refs );
	++destu->Refs;
	unit->Orders[0].RangeX=unit->Orders[0].RangeY=1;
	unit->Orders[0].X=-1;
	unit->Orders[0].Y=-1;
	DebugLevel3("Return to %Zd=%d,%d\n"
	    ,UnitNumber(destu),unit->Orders[0].X,unit->Orders[0].Y);
	unit->Orders[0].Action=UnitActionHaulOil;
#else
	if( !(destu=FindOilDeposit(unit->Player,unit->X,unit->Y)) ) {
	    // No deposit -> can't return
	    unit->Command.Action=UnitActionStill;
	    return;
	}
	ResetPath(unit->Command);
	unit->Command.Data.Move.Goal=destu;
	RefsDebugCheck( !destu->Refs );
	++destu->Refs;
	unit->Command.Data.Move.Range=1;
	unit->Command.Data.Move.DX=-1;
	unit->Command.Data.Move.DY=-1;
	DebugLevel3("Return to %Zd=%d,%d\n"
	    ,UnitNumber(destu)
	    ,unit->Command.Data.Move.DX,unit->Command.Data.Move.DY);
	unit->Command.Action=UnitActionHaulOil;
#endif
	unit->SubAction=65;		// FIXME: Hardcoded
	DebugLevel3("Wait: %d\n",unit->Wait);
	unit->Wait=1;
	return;
    }

    // FIXME: return of more resources.
    // FIXME: some general method for this?
}

//@}
