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
/**@name command.c	-	Give units a command. */
//
//	(c) Copyright 1998,2000,2001 by Lutz Sammer
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
#include "upgrade.h"
#include "pathfinder.h"
#include "spells.h"
#include "interface.h"

/*----------------------------------------------------------------------------
--	Functions
----------------------------------------------------------------------------*/

/**
**	Release an order.
**
**	@param order	Pointer to order.
*/
local void ReleaseOrder(Order* order)
{
    if( order->Goal ) {
	RefsDebugCheck( !order->Goal->Refs );
	if( !--order->Goal->Refs ) {
	    DebugCheck( !order->Goal->Destroyed );
	    ReleaseUnit(order->Goal);
	}
	order->Goal=NoUnitP;
    }
}

/**
**	Release all orders of an unit.
**
**	@param unit	Pointer to unit.
*/
local void ReleaseOrders(Unit* unit)
{
    int n;

    if( (n=unit->OrderCount)>1 ) {
	while( --n ) {
	    ReleaseOrder(&unit->Orders[n]);
	}
	unit->OrderCount=1;
    }
    unit->OrderFlush=1;
    // Order 0 must be stopped in the action loop.
}

/**
**	Get next free order slot.
**
**	@param unit	pointer to unit.
**	@param flush	if true, flush order queue.
**
**	@return		Pointer to next free order slot.
*/
local Order* GetNextOrder(Unit* unit,int flush)
{
    if( flush ) {			// empty command queue
	ReleaseOrders(unit);
    } else if( unit->OrderCount==MAX_ORDERS ) {
	// FIXME: johns: wrong place for an error message.
	// FIXME: johns: should be checked by AI or the user interface
	// NOTE: But must still be checked here.
	if( unit->Player==ThisPlayer ) {
	    // FIXME: use a general notify call
            SetMessage( "Unit order list is full" );
	}
	return NULL;
    }

    return &unit->Orders[(int)unit->OrderCount++];
}

/**
**	Clear the saved action.
**
**	@param unit	Unit pointer, that get the saved action cleared.
**
**	If we make an new order, we must clear any saved actions.
**	Internal functions, must protect it, if needed.
*/
local void ClearSavedAction(Unit* unit)
{
    ReleaseOrder(&unit->SavedOrder);

    unit->SavedOrder.Action=UnitActionStill;	// clear saved action
    unit->SavedOrder.X=unit->SavedOrder.Y=-1;
    unit->SavedOrder.Type=NULL;
    unit->SavedOrder.Arg1=NULL;
}

/*----------------------------------------------------------------------------
--	Commands
----------------------------------------------------------------------------*/

/**
**	Stop unit.
**
**	@param unit	pointer to unit.
*/
global void CommandStopUnit(Unit* unit)
{
    Order* order;

    // Ignore that the unit could be removed.

    order=GetNextOrder(unit,FlushCommands);	// Flush them.
    order->Action=UnitActionStill;
    order->X=order->Y=-1;
    order->Goal=NoUnitP;
    order->Type=NULL;
    order->Arg1=NULL;
    ReleaseOrder(&unit->SavedOrder);
    ReleaseOrder(&unit->NewOrder);
    unit->SavedOrder=unit->NewOrder=*order;
}

/**
**	Stand ground.
**
**	@param unit	pointer to unit.
**	@param flush	if true, flush command queue.
*/
global void CommandStandGround(Unit* unit,int flush)
{
    Order* order;

    // Ignore that the unit could be removed.

    if( unit->Type->Building ) {
	// FIXME: should find a better way for pending orders.
	order=&unit->NewOrder;
	ReleaseOrder(order);
    } else if( !(order=GetNextOrder(unit,flush)) ) {
	return;
    }
    order->Action=UnitActionStandGround;
    order->X=order->Y=-1;
    order->Goal=NoUnitP;
    order->Type=NULL;
    order->Arg1=NULL;
    ClearSavedAction(unit);
}

/**
**	Follow unit to new position
**
**	@param unit	pointer to unit.
**	@param dest	unit to be followed
**	@param flush	if true, flush command queue.
*/
global void CommandFollow(Unit* unit,Unit* dest,int flush)
{
    Order* order;

    //
    //	Check if unit is still valid? (NETWORK!)
    //
    if( !unit->Removed && unit->Orders[0].Action!=UnitActionDie ) {
	if( unit->Type->Building ) {
	    // FIXME: should find a better way for pending orders.
	    order=&unit->NewOrder;
	    ReleaseOrder(order);
	} else if( !(order=GetNextOrder(unit,flush)) ) {
	    return;
	}

	order->Action=UnitActionFollow;
	//
	//	Destination could be killed.
	//	Should be handled in action, but is not possible!
	//		Unit::Refs is used as timeout counter.
	//
	if( dest->Destroyed ) {
	    order->X=dest->X+dest->Type->TileWidth/2;
	    order->Y=dest->Y+dest->Type->TileHeight/2;
	    order->Goal=NoUnitP;
	    order->RangeX=order->RangeY=0;
	} else {
	    order->X=order->Y=-1;
	    order->Goal=dest;
	    RefsDebugCheck( !dest->Refs );
	    dest->Refs++;
	    order->RangeX=order->RangeY=1;
	}
	order->Type=NULL;
	order->Arg1=NULL;
    }
    ClearSavedAction(unit);
}

/**
**	Move unit to new position
**
**	@param unit	pointer to unit.
**	@param x	X map position to move to.
**	@param y	Y map position to move to.
**	@param flush	if true, flush command queue.
*/
global void CommandMove(Unit* unit,int x,int y,int flush)
{
    Order* order;

    IfDebug(
	if( x<0 || y<0 || x>=TheMap.Width || y>=TheMap.Height ) {
	    DebugLevel0Fn("Internal movement error\n");
	    return;
	}
    );

    //
    //	Check if unit is still valid? (NETWORK!)
    //
    if( !unit->Removed && unit->Orders[0].Action!=UnitActionDie ) {
	if( unit->Type->Building ) {
	    // FIXME: should find a better way for pending orders.
	    order=&unit->NewOrder;
	    ReleaseOrder(order);
	} else if( !(order=GetNextOrder(unit,flush)) ) {
	    return;
	}

	order->Action=UnitActionMove;
	order->Goal=NoUnitP;
	order->X=x;
	order->Y=y;
	order->RangeX=order->RangeY=0;
	order->Type=NULL;
	order->Arg1=NULL;
    }
    ClearSavedAction(unit);
}

/**
**	Repair unit
**
**	@param unit	pointer to unit.
**	@param x	X map position to repair.
**	@param y	Y map position to repair.
**	@param dest	or unit to be repaired. FIXME: not supported
**	@param flush	if true, flush command queue.
*/
global void CommandRepair(Unit* unit,int x,int y,Unit* dest,int flush)
{
    Order* order;

    //
    //	Check if unit is still valid? (NETWORK!)
    //
    if( !unit->Removed && unit->Orders[0].Action!=UnitActionDie ) {
	if( unit->Type->Building ) {
	    // FIXME: should find a better way for pending orders.
	    order=&unit->NewOrder;
	    ReleaseOrder(order);
	} else if( !(order=GetNextOrder(unit,flush)) ) {
	    return;
	}

	order->Action=UnitActionRepair;
	//
	//	Destination could be killed.
	//	Should be handled in action, but is not possible!
	//		Unit::Refs is used as timeout counter.
	//
	if( dest ) {
	    if( dest->Destroyed ) {
		order->X=dest->X+dest->Type->TileWidth/2;
		order->Y=dest->Y+dest->Type->TileHeight/2;
		order->Goal=NoUnitP;
		order->RangeX=order->RangeY=0;
	    } else {
		order->X=order->Y=-1;
		order->Goal=dest;
		RefsDebugCheck( !dest->Refs );
		dest->Refs++;
		order->RangeX=order->RangeY=REPAIR_RANGE;
	    }
	} else {
	    order->X=x;
	    order->Y=y;
	    order->Goal=NoUnitP;
	    order->RangeX=order->RangeY=REPAIR_RANGE;
	}
	order->Type=NULL;
	order->Arg1=NULL;
    }
    ClearSavedAction(unit);
}

/**
**	Attack with unit at new position
**
**	@param unit	pointer to unit.
**	@param x	X map position to attack.
**	@param y	Y map position to attack.
**	@param attack	or unit to be attacked.
**	@param flush	if true, flush command queue.
*/
global void CommandAttack(Unit* unit,int x,int y,Unit* attack,int flush)
{
    Order* order;

    IfDebug(
	if( x<0 || y<0 || x>=TheMap.Width || y>=TheMap.Height ) {
	    DebugLevel0Fn("Internal movement error\n");
	    return;
	}
    );

    //
    //	Check if unit is still valid? (NETWORK!)
    //
    if( !unit->Removed && unit->Orders[0].Action!=UnitActionDie ) {
	if( unit->Type->Building ) {
	    // FIXME: should find a better way for pending orders.
	    order=&unit->NewOrder;
	    ReleaseOrder(order);
	} else if( !(order=GetNextOrder(unit,flush)) ) {
	    return;
	}

	order->Action=UnitActionAttack;
	if( attack ) {
	    //
	    //	Destination could be killed.
	    //	Should be handled in action, but is not possible!
	    //		Unit::Refs is used as timeout counter.
	    //
	    if( attack->Destroyed ) {
		order->X=attack->X+attack->Type->TileWidth/2;
		order->Y=attack->Y+attack->Type->TileHeight/2;
		order->Goal=NoUnitP;
		order->RangeX=order->RangeY=0;
	    } else {
		// Removed, Dying handled by action routine.
		order->X=order->Y=-1;
		order->Goal=attack;
		RefsDebugCheck( !attack->Refs );
		attack->Refs++;
		order->RangeX=order->RangeY=unit->Stats->AttackRange;
	    }
	} else if( WallOnMap(x,y) ) {
	    // FIXME: look into action_attack.c about this ugly problem
	    order->X=x;
	    order->Y=y;
	    order->RangeX=order->RangeY=unit->Stats->AttackRange;
	    order->Goal=NoUnitP;
	} else {
	    order->X=x;
	    order->Y=y;
	    order->RangeX=order->RangeY=0;
	    order->Goal=NoUnitP;
	}
	order->Type=NULL;
	order->Arg1=NULL;
    }
    ClearSavedAction(unit);
}

/**
**	Attack ground with unit.
**
**	@param unit	pointer to unit.
**	@param x	X map position to fire on.
**	@param y	Y map position to fire on.
**	@param flush	if true, flush command queue.
*/
global void CommandAttackGround(Unit* unit,int x,int y,int flush)
{
    Order* order;

    IfDebug(
	if( x<0 || y<0 || x>=TheMap.Width || y>=TheMap.Height ) {
	    DebugLevel0Fn("Internal movement error\n");
	    return;
	}
    );

    //
    //	Check if unit is still valid? (NETWORK!)
    //
    if( !unit->Removed && unit->Orders[0].Action!=UnitActionDie ) {
	if( unit->Type->Building ) {
	    // FIXME: should find a better way for pending orders.
	    order=&unit->NewOrder;
	    ReleaseOrder(order);
	} else if( !(order=GetNextOrder(unit,flush)) ) {
	    return;
	}

	order->Action=UnitActionAttackGround;
	order->X=x;
	order->Y=y;
	order->RangeX=order->RangeY=unit->Stats->AttackRange;
	order->Goal=NoUnitP;
	order->Type=NULL;
	order->Arg1=NULL;

	DebugLevel0("FIXME this next\n");
    }
    ClearSavedAction(unit);
}

/**
**	Let an unit patrol from current to new position
**
**	FIXME: want to support patroling between units.
**
**	@param unit	pointer to unit.
**	@param x	X map position to patrol between.
**	@param y	Y map position to patrol between.
**	@param flush	if true, flush command queue.
*/
global void CommandPatrolUnit(Unit* unit,int x,int y,int flush)
{
    Order* order;

    IfDebug(
	if( x<0 || y<0 || x>=TheMap.Width || y>=TheMap.Height ) {
	    DebugLevel0Fn("Internal movement error\n");
	    return;
	}
    );

    //
    //	Check if unit is still valid? (NETWORK!)
    //
    if( !unit->Removed && unit->Orders[0].Action!=UnitActionDie ) {
	if( unit->Type->Building ) {
	    // FIXME: should find a better way for pending orders.
	    order=&unit->NewOrder;
	    ReleaseOrder(order);
	} else if( !(order=GetNextOrder(unit,flush)) ) {
	    return;
	}

	order->Action=UnitActionPatrol;
	order->Goal=NoUnitP;
	order->X=x;
	order->Y=y;
	order->RangeX=order->RangeY=0;
	order->Type=NULL;
	DebugCheck( unit->X&~0xFFFF || unit->Y&~0xFFFF );
	// BUG-ALERT: encode source into arg1 as two 16 bit values!
	order->Arg1=(void*)((unit->X<<16)|unit->Y);
    }
    ClearSavedAction(unit);
}

/**
**	Board a transporter with unit.
**
**	@param unit	pointer to unit.
**	@param dest	unit to be boarded.
**	@param flush	if true, flush command queue.
*/
global void CommandBoard(Unit* unit,Unit* dest,int flush)
{
    Order* order;

    //
    //	Check if unit is still valid? (NETWORK!)
    //
    if( !unit->Removed && unit->Orders[0].Action!=UnitActionDie ) {
	//
	//	Destination could be killed.
	//	Should be handled in action, but is not possible!
	//		Unit::Refs is used as timeout counter.
	//
	if( dest->Destroyed ) {
	    return;
	}

	if( unit->Type->Building ) {
	    // FIXME: should find a better way for pending orders.
	    order=&unit->NewOrder;
	    ReleaseOrder(order);
	} else if( !(order=GetNextOrder(unit,flush)) ) {
	    return;
	}

	order->Action=UnitActionBoard;
	order->X=order->Y=-1;
	order->Goal=dest;
	RefsDebugCheck( !dest->Refs );
	dest->Refs++;
	order->RangeX=order->RangeY=1;
	order->Type=NULL;
	order->Arg1=NULL;
    }
    ClearSavedAction(unit);
}

/**
**	Unload a transporter.
**
**	@param unit	pointer to unit.
**	@param x	X map position to unload.
**	@param y	Y map position to unload.
**	@param what	unit to be unloaded, NoUnitP all.
**	@param flush	if true, flush command queue.
*/
global void CommandUnload(Unit* unit,int x,int y,Unit* what,int flush)
{
    Order* order;

    //
    //	Check if unit is still valid? (NETWORK!)
    //
    if( !unit->Removed && unit->Orders[0].Action!=UnitActionDie ) {
	if( unit->Type->Building ) {
	    // FIXME: should find a better way for pending orders.
	    order=&unit->NewOrder;
	    ReleaseOrder(order);
	} else if( !(order=GetNextOrder(unit,flush)) ) {
	    return;
	}

	order->Action=UnitActionUnload;
	order->X=x;
	order->Y=y;
	//
	//	Destination could be killed.
	//	Should be handled in action, but is not possible!
	//		Unit::Refs is used as timeout counter.
	//
	order->Goal=NoUnitP;
	if( what && !what->Destroyed ) {
	    order->Goal=what;
	    RefsDebugCheck( !what->Refs );
	    what->Refs++;
	}
	order->RangeX=order->RangeY=0;
	order->Type=NULL;
	order->Arg1=NULL;
    }
    ClearSavedAction(unit);
}

/**
**	Send a unit building
**
**	@param unit	pointer to unit.
**	@param x	X map position to build.
**	@param y	Y map position to build.
**	@param what	Unit type to build.
**	@param flush	if true, flush command queue.
*/
global void CommandBuildBuilding(Unit* unit,int x,int y
	,UnitType* what,int flush)
{
    Order* order;

    //
    //	Check if unit is still valid? (NETWORK!)
    //
    if( !unit->Removed && unit->Orders[0].Action!=UnitActionDie ) {
	if( unit->Type->Building ) {
	    // FIXME: should find a better way for pending orders.
	    order=&unit->NewOrder;
	    ReleaseOrder(order);
	} else if( !(order=GetNextOrder(unit,flush)) ) {
	    return;
	}

	order->Action=UnitActionBuild;
	order->Goal=NoUnitP;
	if( what->ShoreBuilding ) {
	    order->RangeX=what->TileWidth+1;
	    order->RangeY=what->TileHeight+1;
	    order->X=x-1;
	    order->Y=y-1;
	} else {
	    // FIXME: -1 read note in pathfinder, some old wired code.
	    order->RangeX=what->TileWidth-1;
	    order->RangeY=what->TileHeight-1;
	    order->X=x;
	    order->Y=y;
	}
	order->Type=what;
	order->Arg1=NULL;
    }
    ClearSavedAction(unit);
}

/**
**	Cancel the building construction.
**
**	@param unit	pointer to unit.
**	@param worker	pointer to unit.
*/
global void CommandCancelBuilding(Unit* unit,Unit* worker)
{
    //
    //	Check if building is still under construction? (NETWORK!)
    //
    if( unit->Orders[0].Action==UnitActionBuilded ) {
	unit->Data.Builded.Cancel=1;
    }
    ClearSavedAction(unit);
}

/**
**	Send unit harvest
**
**	@param unit	pointer to unit.
**	@param x	X map position for harvest.
**	@param y	Y map position for harvest.
**	@param flush	if true, flush command queue.
*/
global void CommandHarvest(Unit* unit,int x,int y,int flush)
{
    Order* order;

    //
    //	Check if unit is still valid? (NETWORK!)
    //
    if( !unit->Removed && unit->Orders[0].Action!=UnitActionDie ) {
	if( unit->Type->Building ) {
	    // FIXME: should find a better way for pending orders.
	    order=&unit->NewOrder;
	    ReleaseOrder(order);
	} else if( !(order=GetNextOrder(unit,flush)) ) {
	    return;
	}

	order->Action=UnitActionHarvest;
	order->X=x-1;
	order->Y=y-1;
	order->RangeX=order->RangeY=2;
	order->Goal=NoUnitP;
	order->Type=NULL;
	order->Arg1=NULL;
    }
    ClearSavedAction(unit);
}

/**
**	Send unit mine gold.
**
**	@param unit	pointer to unit.
**	@param dest	destination unit.
**	@param flush	if true, flush command queue.
*/
global void CommandMineGold(Unit* unit,Unit* dest,int flush)
{
    Order* order;

    //
    //	Check if unit is still valid and Goal still alive? (NETWORK!)
    //
    if( !unit->Removed && unit->Orders[0].Action!=UnitActionDie
	     && !dest->Destroyed ) {
	// FIXME: if low-level supports searching, pass NoUnitP down.

	if( unit->Type->Building ) {
	    // FIXME: should find a better way for pending orders.
	    order=&unit->NewOrder;
	    ReleaseOrder(order);
	} else if( !(order=GetNextOrder(unit,flush)) ) {
	    return;
	}

	order->Action=UnitActionMineGold;
	order->X=order->Y=-1;
	order->Goal=dest;
	RefsDebugCheck( !dest->Refs );
	dest->Refs++;
	order->RangeX=order->RangeY=1;
	order->Type=NULL;
	order->Arg1=NULL;
    }
    ClearSavedAction(unit);
}

/**
**	Send unit haul oil.
**
**	@param unit	pointer to unit.
**	@param dest	destination unit.
**	@param flush	if true, flush command queue.
*/
global void CommandHaulOil(Unit* unit,Unit* dest,int flush)
{
    Order* order;

    //
    //	Check if unit is still valid and Goal still alive? (NETWORK!)
    //
    if( !unit->Removed && unit->Orders[0].Action!=UnitActionDie
	     && !dest->Destroyed ) {
	// FIXME: if low-level supports searching, pass NoUnitP down.

	if( unit->Type->Building ) {
	    // FIXME: should find a better way for pending orders.
	    order=&unit->NewOrder;
	    ReleaseOrder(order);
	} else if( !(order=GetNextOrder(unit,flush)) ) {
	    return;
	}

	order->Action=UnitActionHaulOil;
	order->X=order->Y=-1;
	order->Goal=dest;
	RefsDebugCheck( !dest->Refs );
	dest->Refs++;
	order->RangeX=order->RangeY=1;
	order->Type=NULL;
	order->Arg1=NULL;
    }
    ClearSavedAction(unit);
}

/**
**	Let unit returning goods.
**
**	@param unit	pointer to unit.
**	@param goal	bring goods to this depot.
**	@param flush	if true, flush command queue.
*/
global void CommandReturnGoods(Unit* unit,Unit* goal,int flush)
{
    Order* order;

    //
    //	Check if unit is still valid and Goal still alive? (NETWORK!)
    //
    if( !unit->Removed && unit->Orders[0].Action!=UnitActionDie ) {
	if( unit->Type->Building ) {
	    // FIXME: should find a better way for pending orders.
	    order=&unit->NewOrder;
	    ReleaseOrder(order);
	} else if( !(order=GetNextOrder(unit,flush)) ) {
	    return;
	}

	order->Action=UnitActionReturnGoods;
	order->X=order->Y=-1;
	order->Goal=NoUnitP;
	//
	//	Destination could be killed. NETWORK!
	//
	if( goal && !goal->Destroyed ) {
	    order->Goal=goal;
	    RefsDebugCheck( !goal->Refs );
	    goal->Refs++;
	}
	order->RangeX=order->RangeY=1;
	order->Type=NULL;
	order->Arg1=NULL;
    }
    ClearSavedAction(unit);
}

/**
**	Building starts training an unit.
**
**	@param unit	pointer to unit.
**	@param type	unit type to train.
**	@param flush	if true, flush command queue.
*/
global void CommandTrainUnit(Unit* unit,UnitType* type,int flush)
{
    //
    //	Check if enough resources remains? (NETWORK!)
    //
    if( !PlayerCheckFood(unit->Player,type)
	    || PlayerCheckUnitType(unit->Player,type) ) {
	return;
    }

    //
    //	Not already training?
    //
    if( unit->Orders[0].Action!=UnitActionTrain ) {

	DebugCheck( unit->Wait>6 );

	if( unit->OrderCount==2 && unit->Orders[1].Action==UnitActionTrain ) {
	    DebugLevel0Fn("FIXME: not supported. Unit queue full!\n");
	    return;
	} else {
	    ReleaseOrders(unit);
	    unit->Orders[1].Action=UnitActionTrain;
	}
	DebugCheck( unit->OrderCount!=1 || unit->OrderFlush!=1 );

	unit->OrderCount=2;
	unit->Orders[1].Type=type;
	unit->Orders[1].X=unit->Orders[1].Y=-1;
	unit->Orders[1].Goal=NoUnitP;
	unit->Orders[1].Arg1=NULL;
    } else {
	//
	//	Update interface.
	//
	if( unit->Player==ThisPlayer && unit->Selected ) {
	    MustRedraw|=RedrawInfoPanel;
	}

	//
	//	Training slots are all already full. (NETWORK!)
	//
	if( unit->Data.Train.Count>=MAX_UNIT_TRAIN ) {
	    DebugLevel0Fn("Unit queue full!\n");
	    return;
	}

	unit->Data.Train.What[unit->Data.Train.Count++]=type;
    }
    // FIXME: if you give quick an other order, the resources are lost!
    PlayerSubUnitType(unit->Player,type);

    ClearSavedAction(unit);
}

/**
**	Cancel the training of an unit.
**
**	@param unit	pointer to unit.
**	@param slot	slot number to cancel.
*/
global void CommandCancelTraining(Unit* unit,int slot)
{
    int i;
    int n;

    // FIXME: over network we could cancel the wrong slot.

    //
    //	Check if unit is still training 'slot'? (NETWORK!)
    //
    if( unit->Orders[0].Action==UnitActionTrain
	    && slot<(n=unit->Data.Train.Count) ) {

	PlayerAddCostsFactor(unit->Player,
		unit->Data.Train.What[slot]->Stats[unit->Player->Player].Costs,
		CancelTrainingCostsFactor);

	if ( --n ) {
	    for( i = slot; i < n; i++ ) {
		unit->Data.Train.What[i] = unit->Data.Train.What[i+1];
	    }
	    if( !slot ) {
		unit->Data.Train.Ticks=0;
	    }
	    unit->Data.Train.Count=n;
	} else {
	    DebugLevel0Fn("Last slot\n");
	    unit->Orders[0].Action=UnitActionStill;
	    unit->SubAction=0;
	}

	//
	//	Update interface.
	//
	if( unit->Player==ThisPlayer && unit->Selected ) {
	    UpdateButtonPanel();
	    MustRedraw|=RedrawPanels;
	}

	unit->Wait=unit->Reset=1;	// immediately start next training
    }
    ClearSavedAction(unit);
}

/**
**	Building starts upgrading to.
**
**	@param unit	pointer to unit.
**	@param type	upgrade to type
**	@param flush	if true, flush command queue.
*/
global void CommandUpgradeTo(Unit* unit,UnitType* type,int flush)
{
    Order* order;

    //
    //	Check if unit is still valid and Goal still alive? (NETWORK!)
    //
    if( !unit->Removed && unit->Orders[0].Action!=UnitActionDie ) {
	//
	//	Check if enough resources remains? (NETWORK!)
	//
	if( PlayerCheckUnitType(unit->Player,type) ) {
	    return;
	}

	if( !flush ) {
	    DebugLevel0Fn("FIXME: must support order queing!!");
	}
	if( !(order=GetNextOrder(unit,flush)) ) {
	    return;
	}

	// FIXME: if you give quick an other order, the resources are lost!
	PlayerSubUnitType(unit->Player,type);

	order->Action=UnitActionUpgradeTo;
	order->X=order->Y=-1;
	order->Goal=NoUnitP;
	order->Type=type;
	order->Arg1=NULL;
    }
    ClearSavedAction(unit);
}

/**
**	Cancel building upgrading to.
**
**	@param unit	pointer to unit.
*/
global void CommandCancelUpgradeTo(Unit* unit)
{
    ReleaseOrders(unit);		// empty command queue

    //
    //	Check if unit is still upgrading? (NETWORK!)
    //
    if( unit->Orders[0].Action == UnitActionUpgradeTo ) {

	PlayerAddCostsFactor(unit->Player,
		unit->Orders[0].Type->Stats->Costs,
		CancelUpgradeCostsFactor);

	unit->Orders[0].Action=UnitActionStill;
	unit->Orders[0].X=unit->Orders[0].Y=-1;
	unit->Orders[0].Goal=NoUnitP;
	unit->Orders[0].Type=NULL;
	unit->Orders[0].Arg1=NULL;

	unit->SubAction=0;

	//
	//	Update interface.
	//
	if( unit->Player==ThisPlayer && unit->Selected ) {
	    UpdateButtonPanel();
	    MustRedraw|=RedrawPanels;
	}

	unit->Wait=unit->Reset=1;	// immediately start next command.
    }
    ClearSavedAction(unit);
}

/**
**	Building starts researching.
**
**	@param unit	pointer to unit.
**	@param what	what to research.
**	@param flush	if true, flush command queue.
*/
global void CommandResearch(Unit* unit,Upgrade* what,int flush)
{
    Order* order;

    //
    //	Check if unit is still valid and Goal still alive? (NETWORK!)
    //
    if( !unit->Removed && unit->Orders[0].Action!=UnitActionDie ) {
	//
	//	Check if enough resources remains? (NETWORK!)
	//
	if( PlayerCheckCosts(unit->Player,what->Costs) ) {
	    return;
	}

	if( !flush ) {
	    DebugLevel0Fn("FIXME: must support order queing!!");
	}
	if( !(order=GetNextOrder(unit,flush)) ) {
	    return;
	}

	// FIXME: if you give quick an other order, the resources are lost!
	PlayerSubCosts(unit->Player,what->Costs);

	order->Action=UnitActionResearch;
	order->X=order->Y=-1;
	order->Goal=NoUnitP;
	order->Type=NULL;
	order->Arg1=what;
    }
    ClearSavedAction(unit);
}

/**
**	Cancel Building researching.
**
**	@param unit	pointer to unit.
*/
global void CommandCancelResearch(Unit* unit)
{
    ReleaseOrders(unit);		// empty command queue

    //
    //	Check if unit is still researching? (NETWORK!)
    //
    if( unit->Orders[0].Action == UnitActionResearch ) {

	PlayerAddCostsFactor(unit->Player,
		unit->Data.Research.Upgrade->Costs,
		CancelResearchCostsFactor);

	unit->Orders[0].Action=UnitActionStill;
	unit->Orders[0].X=unit->Orders[0].Y=-1;
	unit->Orders[0].Goal=NoUnitP;
	unit->Orders[0].Type=NULL;
	unit->Orders[0].Arg1=NULL;

	unit->SubAction=0;

	//
	//	Update interface.
	//
	if( unit->Player==ThisPlayer && unit->Selected ) {
	    UpdateButtonPanel();
	    MustRedraw|=RedrawPanels;
	}

	unit->Wait=unit->Reset=1;	// immediately start next command.
    }
    ClearSavedAction(unit);
}

/**
**	Demolish at position
**
**	@param unit	pointer to unit.
**	@param x	X map position to move to.
**	@param y	Y map position to move to.
**	@param dest	if != NULL, pointer to unit to destroy.
**	@param flush	if true, flush command queue.
*/
global void CommandDemolish(Unit* unit,int x,int y,Unit* dest,int flush)
{
    Order* order;

    IfDebug(
	if( x<0 || y<0 || x>=TheMap.Width || y>=TheMap.Height ) {
	    DebugLevel0Fn("Internal movement error\n");
	    return;
	}
    );

    //
    //	Check if unit is still valid? (NETWORK!)
    //
    if( !unit->Removed && unit->Orders[0].Action!=UnitActionDie ) {
	if( unit->Type->Building ) {
	    // FIXME: should find a better way for pending orders.
	    order=&unit->NewOrder;
	    ReleaseOrder(order);
	} else if( !(order=GetNextOrder(unit,flush)) ) {
	    return;
	}

	order->Action=UnitActionDemolish;
	if( dest ) {
	    //
	    //	Destination could be killed.
	    //	Should be handled in action, but is not possible!
	    //		Unit::Refs is used as timeout counter.
	    //
	    if( dest->Destroyed ) {
		order->X=dest->X+dest->Type->TileWidth/2;
		order->Y=dest->Y+dest->Type->TileHeight/2;
		order->Goal=NoUnitP;
		order->RangeX=order->RangeY=0;
	    } else {
		order->X=order->Y=-1;
		order->Goal=dest;
		RefsDebugCheck( !dest->Refs );
		dest->Refs++;
		order->RangeX=order->RangeY=1;
	    }
	} else if( WallOnMap(x,y) || ForestOnMap(x,y) || RockOnMap(x,y) ) {
	    order->X=x;
	    order->Y=y;
	    order->RangeX=order->RangeY=1;
	    order->Goal=NoUnitP;
	} else {
	    order->X=x;
	    order->Y=y;
	    order->RangeX=order->RangeY=0;
	    order->Goal=NoUnitP;
	}
	order->Type=NULL;
	order->Arg1=NULL;
    }
    ClearSavedAction(unit);
}

/**
**	Cast a spell at position or unit.
**
**	@param unit	pointer to unit.
**	@param x	X map position to spell cast on.
**	@param y	Y map position to spell cast on.
**	@param dest	Spell cast on unit (if exist).
**	@param spell	Spell type pointer.
**	@param flush	if true, flush command queue.
*/
global void CommandSpellCast(Unit* unit,int x,int y,Unit* dest
	,SpellType* spell,int flush)
{
    Order* order;

    IfDebug(
	if( x<0 || y<0 || x>=TheMap.Width || y>=TheMap.Height ) {
	    DebugLevel0("Internal movement error\n");
	    return;
	}
    );

    DebugLevel3Fn(": %Zd spell-casts on %Zd\n"
	,UnitNumber(unit),dest ? UnitNumber(dest) : 0);

    //
    //	Check if unit is still valid? (NETWORK!)
    //
    if( !unit->Removed && unit->Orders[0].Action!=UnitActionDie ) {
	// FIXME: should I check here, if there is still enough mana?

	if( unit->Type->Building ) {
	    // FIXME: should find a better way for pending orders.
	    order=&unit->NewOrder;
	    ReleaseOrder(order);
	} else if( !(order=GetNextOrder(unit,flush)) ) {
	    return;
	}

	order->Action=UnitActionSpellCast;
	order->RangeX=order->RangeY=spell->Range;
	if( dest ) {
	    //
	    //	Destination could be killed.
	    //	Should be handled in action, but is not possible!
	    //		Unit::Refs is used as timeout counter.
	    //
	    if( dest->Destroyed ) {
		// FIXME: where check if spell needs an unit as destination?
		order->X=dest->X+dest->Type->TileWidth/2-order->RangeX;
		order->Y=dest->Y+dest->Type->TileHeight/2-order->RangeY;
		order->Goal=NoUnitP;
		order->RangeX<<=1;
		order->RangeY<<=1;
	    } else {
		order->X=order->Y=-1;
		order->Goal=dest;
		RefsDebugCheck( !dest->Refs );
		dest->Refs++;
	    }
	} else {
	    order->X=x-order->RangeX;
	    order->Y=y-order->RangeY;
	    order->Goal=NoUnitP;
	    order->RangeX<<=1;
	    order->RangeY<<=1;
	}
	order->Type=NULL;
	order->Arg1=spell;
    }
    ClearSavedAction(unit);
}

//@}
