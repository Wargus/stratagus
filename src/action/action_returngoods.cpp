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
    //	Select target to return goods. FIXME: more races support
    //
    if( type==UnitTypeHumanWorkerWithGold || type==UnitTypeOrcWorkerWithGold ) {
	if( !unit->Orders[0].Goal ) {
	    if( !(destu=FindGoldDeposit(unit,unit->X,unit->Y)) ) {
		// No deposit -> can't return
		unit->Orders[0].Action=UnitActionStill;
		return;
	    }
	    unit->Orders[0].Goal=destu;
	    RefsDebugCheck( !destu->Refs );
	    ++destu->Refs;
	}
	unit->Orders[0].Action=UnitActionMineGold;
	unit->Orders[0].Arg1=(void*)-1;
	NewResetPath(unit);
	unit->SubAction=65;	// FIXME: hardcoded
	DebugLevel3("Wait: %d\n",unit->Wait);
	unit->Wait=1;
	return;
    }

    if( type==UnitTypeHumanWorkerWithWood || type==UnitTypeOrcWorkerWithWood ) {
	if( !unit->Orders[0].Goal ) {
	    if( !(destu=FindWoodDeposit(unit->Player,unit->X,unit->Y)) ) {
		// No deposit -> can't return
		unit->Orders[0].Action=UnitActionStill;
		return;
	    }
	    unit->Orders[0].Goal=destu;
	    RefsDebugCheck( !destu->Refs );
	    ++destu->Refs;
	}
	unit->Orders[0].X=unit->X;
	unit->Orders[0].Y=unit->Y;	// Return point to continue.
	DebugLevel3("Return to %d=%d,%d\n"
		,UnitNumber(destu),unit->Orders[0].X,unit->Orders[0].Y);
	unit->Orders[0].Action=UnitActionHarvest;
	unit->Orders[0].Arg1=(void*)-1;
	NewResetPath(unit);
	unit->SubAction=128;		// FIXME: Hardcoded
	DebugLevel3("Wait: %d\n",unit->Wait);
	unit->Wait=1;
	return;
    }

    if( type==UnitTypeHumanTankerFull || type==UnitTypeOrcTankerFull ) {
	if( !unit->Orders[0].Goal ) {
	    if( !(destu=FindOilDeposit(unit,unit->X,unit->Y)) ) {
		// No deposit -> can't return
		unit->Orders[0].Action=UnitActionStill;
		return;
	    }
	    unit->Orders[0].Goal=destu;
	    RefsDebugCheck( !destu->Refs );
	    ++destu->Refs;
	}
	DebugLevel3("Return to %d=%d,%d\n"
	    ,UnitNumber(destu),unit->Orders[0].X,unit->Orders[0].Y);
	unit->Orders[0].Action=UnitActionHaulOil;
	unit->Orders[0].Arg1=(void*)-1;
	NewResetPath(unit);
	unit->SubAction=65;		// FIXME: Hardcoded
	DebugLevel3("Wait: %d\n",unit->Wait);
	unit->Wait=1;
	return;
    }

    // FIXME: return of more resources.
    // FIXME: some general method for this?
}

//@}
