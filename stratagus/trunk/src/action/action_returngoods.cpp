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
/*
**	(c) Copyright 1998,2000 by Lutz Sammer
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

/**
**	Return goods to gold/wood deposit.
**
**	@param unit	pointer to unit.
*/
global void HandleActionReturnGoods(Unit* unit)
{
    int type;
    Unit* destu;

    //
    //	Select target to return goods.
    //
    type=unit->Type->Type;
    if( type==UnitPeonWithGold || type==UnitPeasantWithGold ) {
	if( !(destu=FindGoldDeposit(unit->Player,unit->X,unit->Y)) ) {
	    unit->Command.Action=UnitActionStill;
	    return;
	}
	unit->Command.Data.Move.Fast=1;
	unit->Command.Data.Move.Goal=destu;
	unit->Command.Data.Move.Range=1;
#if 1
	NearestOfUnit(destu,unit->X,unit->Y
	    ,&unit->Command.Data.Move.DX
	    ,&unit->Command.Data.Move.DY);
#else
	unit->Command.Data.Move.DX=destu->X;
	unit->Command.Data.Move.DY=destu->Y;
#endif
	unit->Command.Action=UnitActionMineGold;
	unit->SubAction=65;
	DebugLevel3("Wait: %d\n",unit->Wait);
	unit->Wait=1;
	return;
    }

    if( type==UnitPeonWithWood || type==UnitPeasantWithWood ) {
	if( !(destu=FindWoodDeposit(unit->Player,unit->X,unit->Y)) ) {
	    unit->Command.Action=UnitActionStill;
	    return;
	}
	unit->Command.Data.Move.Fast=1;
	unit->Command.Data.Move.Goal=destu;
	unit->Command.Data.Move.Range=1;
#if 1
	NearestOfUnit(destu,unit->X,unit->Y
		,&unit->Command.Data.Move.DX
		,&unit->Command.Data.Move.DY);
#else
	unit->Command.Data.Move.DX=destu->X;
	unit->Command.Data.Move.DY=destu->Y;
#endif
	DebugLevel3("Return to %Zd=%d,%d\n"
	    ,UnitNumber(destu)
	    ,unit->Command.Data.Move.DX
	    ,unit->Command.Data.Move.DY);
	unit->Command.Action=UnitActionHarvest;
	unit->SubAction=2;
	DebugLevel3("Wait: %d\n",unit->Wait);
	unit->Wait=1;
	return;
    }

    if( type==UnitTankerHumanFull || type==UnitTankerOrcFull ) {
	if( !(destu=FindOilDeposit(unit->Player,unit->X,unit->Y)) ) {
	    unit->Command.Action=UnitActionStill;
	    return;
	}
	unit->Command.Data.Move.Fast=1;
	unit->Command.Data.Move.Goal=destu;
	unit->Command.Data.Move.Range=1;
#if 1
	NearestOfUnit(destu,unit->X,unit->Y
		,&unit->Command.Data.Move.DX
		,&unit->Command.Data.Move.DY);
#else
	unit->Command.Data.Move.DX=destu->X;
	unit->Command.Data.Move.DY=destu->Y;
#endif
	DebugLevel3("Return to %Zd=%d,%d\n"
	    ,UnitNumber(destu)
	    ,unit->Command.Data.Move.DX
	    ,unit->Command.Data.Move.DY);
	unit->Command.Action=UnitActionHaulOil;
	unit->SubAction=2;
	DebugLevel3("Wait: %d\n",unit->Wait);
	unit->Wait=1;
	return;
    }
}

//@}
