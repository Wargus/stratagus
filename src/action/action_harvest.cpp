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
/*
**	(c) Copyright 1998-2000 by Lutz Sammer
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
#include "missile.h"
#include "actions.h"
#include "sound.h"
#include "tileset.h"
#include "map.h"
#include "interface.h"

/**
**	Move to forest.
**
**	@param unit	Pointer to worker unit.
**
**	@return		TRUE if reached, otherwise FALSE.
*/
local int MoveToWood(Unit* unit)
{
    int x;
    int y;
    int dx;
    int dy;

    if( !HandleActionMove(unit) ) {	// reached end-point
	return 0;
    }
    // FIXME: reached nearly must be returned by HandleActionMove!

    //
    //	reached nearly? and is there wood?
    //
    if( unit->Command.Data.Move.Range ) {
	// FIXME: not correct on map border :(
	++unit->Command.Data.Move.DX;
	++unit->Command.Data.Move.DY;
    }
    x=unit->Command.Data.Move.DX;
    y=unit->Command.Data.Move.DY;
    dx=x-unit->X;
    dy=y-unit->Y;
    DebugLevel3("Why %d,%d = %d\n",dx,dy,ForestOnMap(x,y));
    if( dx<-1 || dx>1 || dy<-1 || dy>1 || !ForestOnMap(x,y) ) {
	if( FindWoodInSight(unit
		,&unit->Command.Data.Move.DX
		,&unit->Command.Data.Move.DY) ) {

	    // Move to new wood position
	    unit->Command.Data.Move.Fast=1;
	    unit->Command.Data.Move.Goal=NoUnitP;
	    unit->Command.Data.Move.Range=2;
	    unit->Command.Action=UnitActionHarvest;
	    if( unit->Command.Data.Move.DX ) {
		unit->Command.Data.Move.DX--;
	    }
	    if( unit->Command.Data.Move.DY ) {
		unit->Command.Data.Move.DY--;
	    }
	}
	return 0;
    }
    unit->Command.Action=UnitActionHarvest;

    // FIXME: don't chop the same wood!
    // turn to wood
    UnitNewHeadingFromXY(unit,dx,dy);
    if(unit -> WoodToHarvest != CHOP_FOR_WOOD) {
	unit -> Value = unit -> WoodToHarvest;
    } else {
	unit -> Value=CHOP_FOR_WOOD;
    }

    DebugCheck( unit->Wait!=1 );

    return 1;
}

/**
**	Chop the wood.
**
**	@param unit	Unit pointer chopping wood.
**
**
**	Return TRUE if ready, otherwise FALSE.
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
	if( !ForestOnMap(unit->Command.Data.Move.DX
		,unit->Command.Data.Move.DY) ) {
	    if( FindWoodInSight(unit
		    ,&unit->Command.Data.Move.DX
		    ,&unit->Command.Data.Move.DY) ) {
		unit->Command.Data.Move.Fast=1;
		unit->Command.Data.Move.Goal=NoUnitP;
		unit->Command.Data.Move.Range=0;
		// FIXME: shouldn't it be range=1 ??
		DebugCheck( unit->Command.Action!=UnitActionHarvest );
		unit->SubAction=0;
	    } else {
		unit->Command.Action=UnitActionStill;
		unit->SubAction=0;
		DebugLevel3("NO-WOOD in sight range\n");
	    }
	    return 0;
	}

	//
	//	Ready chopping wood?
	//
	if( !(unit->WoodToHarvest = --unit->Value) ) {

	    // Have wood
	    if( unit->Type==UnitTypeOrcWorker ) {
		unit->Type=UnitTypeOrcWorkerWithWood;
	    } else if( unit->Type==UnitTypeHumanWorker ) {
		unit->Type=UnitTypeHumanWorkerWithWood;
	    } else {
		DebugLevel0("Wrong unit for chopping wood %d\n"
			,unit->Type->Type);
	    }

	    //
	    //	Update the display.
	    //
	    if( UnitVisible(unit) ) {
		MustRedraw|=RedrawMap;
	    }
	    if( IsSelected(unit) ) {
		UpdateButtonPanel();
		MustRedraw|=RedrawButtonPanel;
	    }

	    //
	    //	Update the map.
	    //
	    MapRemoveWood(unit->Command.Data.Move.DX
		,unit->Command.Data.Move.DY);

	    //
	    //	Find place to return wood.
	    //
	    unit->Command.Data.Move.SX=unit->X;
	    unit->Command.Data.Move.SY=unit->Y;
	    if( !(destu=FindWoodDeposit(unit->Player,unit->X,unit->Y)) ) {
		unit->Command.Action=UnitActionStill;
		unit->SubAction=0;
	    } else {
		unit->Command.Data.Move.Fast=1;
		unit->Command.Data.Move.Range=1;
		unit->Command.Data.Move.Goal=destu;
#if 1
		// Fast movement need this??
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
		DebugCheck( unit->Command.Action!=UnitActionHarvest );
		return 1;
	    }

	}
    }
    return 0;
}

/*
**	Return with the wood.
**	Return TRUE if reached, otherwise FALSE.
*/
local int ReturnWithWood(Unit* unit)
{
    int x;
    int y;
    //int dx;
    //int dy;
    Unit* destu;

    if( !HandleActionMove(unit) ) {	// reached end-point
	return 0;
    }

    DebugCheck( unit->Wait!=1 );

#if 0
    // reached nearly? and is there an wood deposit?

    x=unit->Command.Data.Move.DX;
    y=unit->Command.Data.Move.DY;
    dx=unit->X-x;
    dy=unit->Y-y;
    destu=WoodDepositOnMap(x,y);

    DebugLevel3("Near unit %d,%d =%Zd\n",x,y,UnitNumber(destu));

    if( !destu || dx<-1 || dx>destu->Type->TileWidth
	    || dy<-1 || dy>destu->Type->TileHeight ) {
      	DebugLevel2("WOOD-DEPOSIT NOT REACHED %d=%d,%Zd\n"
		    ,UnitNumber(destu),dx,dy);
	unit->Command.Action=UnitActionStill;
	unit->SubAction=0;
	return 0;
    }
#endif
    x=unit->Command.Data.Move.DX;
    y=unit->Command.Data.Move.DY;
    destu=WoodDepositOnMap(x,y);
    if( !destu || MapDistanceToUnit(unit->X,unit->Y,destu)!=1 ) {
      DebugLevel2("WOOD-DEPOSIT NOT REACHED %Zd=%d,%d ? %d\n"
		  ,UnitNumber(destu),x,y
		  ,MapDistanceToUnit(unit->X,unit->Y,destu));
	unit->Command.Action=UnitActionStill;
	unit->SubAction=0;
	return 0;
    }

    unit->Command.Action=UnitActionHarvest;

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
	DebugLevel0("Wrong unit for returning wood %d\n"
	    ,unit->Type->Type);
    }

    if( WAIT_FOR_WOOD<UNIT_MAX_WAIT ) {
	unit->Wait=WAIT_FOR_WOOD;
    } else {
	unit->Wait=UNIT_MAX_WAIT;
    }
    unit->Value=WAIT_FOR_WOOD-unit->Wait;
    
    return 1;
}

/*
**	Wait in wood deposit.
*/
local int WaitInWoodDeposit(Unit* unit)
{
    Unit* destu;

    DebugLevel3("Waiting\n");

    if( !unit->Value ) {
	//
	//	Drop out unit at nearest point to target.
	//
	destu=WoodDepositOnMap(unit->X,unit->Y);
	DropOutNearest(unit
		,unit->Command.Data.Move.SX
		,unit->Command.Data.Move.SY
		,destu->Type->TileWidth
		,destu->Type->TileHeight);

	//
	//	Return to chop point.
	//
	unit->Command.Action=UnitActionHarvest;
	unit->Command.Data.Move.Fast=1;
	unit->Command.Data.Move.Goal=NoUnitP;
	unit->Command.Data.Move.Range=0;
	unit->Command.Data.Move.DX=unit->Command.Data.Move.SX;
	unit->Command.Data.Move.DY=unit->Command.Data.Move.SY;

	if( UnitVisible(unit) ) {
	    MustRedraw|=RedrawMap;
	}
	unit->Wait=1;
	unit->WoodToHarvest=CHOP_FOR_WOOD;
	return 1;
    }

    if( unit->Value<UNIT_MAX_WAIT ) {
	unit->Wait=unit->Value;
    } else {
	unit->Wait=UNIT_MAX_WAIT;
    }
    unit->Value-=unit->Wait;

    return 0;
}

/*
**	Unit Harvest:
**		Move into forest.
**		Chop the tree.
**		Return to wood store.
**		Deliver wood.
**		Restart from beginning.
*/
global void HandleActionHarvest(Unit* unit)
{
    switch( unit->SubAction ) {
	case 0:
	    if( MoveToWood(unit) ) {
		++unit->SubAction;
	    }
	    break;

	case 1:
	    if( ChopWood(unit) ) {
		++unit->SubAction;
	    }
	    break;

	case 2:
	    if( ReturnWithWood(unit) ) {
		++unit->SubAction;
	    }
	    break;

	case 3:
	    if( WaitInWoodDeposit(unit) ) {
		unit->SubAction=0;
	    }
	    break;
    }
}

//@}
