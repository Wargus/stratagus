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
/*
**	(c) Copyright 1998,2000 by Lutz Sammer
**
**	$Id$
*/

//@{

/*----------------------------------------------------------------------------
--	Includes
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
#include "tileset.h"
#include "map.h"
#include "upgrade.h"

/*----------------------------------------------------------------------------
--	Functions
----------------------------------------------------------------------------*/

/**
**	Get next command.
**
**	@param unit	pointer to unit.
**	@param flush	if true, flush command queue.
**
**	@returns	Pointer to next free command.
*/
local Command* GetNextCommand(Unit* unit,int flush)
{
    if( flush ) {			// empty command queue
	unit->NextCount=0;
	unit->NextFlush=1;
    } else if( unit->NextCount==MAX_COMMANDS ) {
	// FIXME: johns: wrong place for an error message.
	// FIXME: johns: should be checked by AI or the user interface
	if( unit->Player==ThisPlayer ) {
            SetMessage( "Unit action list is full" );
	}
	return NULL;
    }

    return &unit->NextCommand[unit->NextCount++];
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
    unit->NextFlush=1;
    unit->NextCount=1;
    unit->NextCommand[0].Action=UnitActionStill;

    unit->PendCommand=unit->NextCommand[0];

    unit->SavedCommand.Action=UnitActionStill;	// clear saved action
}

/**
**	Stand ground.
**
**	@param unit	pointer to unit.
**	@param flush	if true, flush command queue.
*/
global void CommandStandGround(Unit* unit,int flush)
{
    Command* command;

    if( !(command=GetNextCommand(unit,flush)) ) {
	return;
    }

    command->Action=UnitActionStandGround;

    unit->SavedCommand.Action=UnitActionStill;	// clear saved action
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
    Command* command;

    if( !(command=GetNextCommand(unit,flush)) ) {
	return;
    }

    command->Action=UnitActionFollow;
    command->Data.Move.Fast=1;
    command->Data.Move.Goal=dest;
#ifdef NEW_UNIT
    dest->Refs++;
#endif
    command->Data.Move.Range=1;
    command->Data.Move.SX=unit->X;
    command->Data.Move.SY=unit->Y;

    unit->SavedCommand.Action=UnitActionStill;	// clear saved action
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
    Command* command;

    IfDebug(
	if( x<0 || y<0 || x>=TheMap.Width || y>=TheMap.Height ) {
	    DebugLevel0("Internal movement error\n");
	    return;
	}
    );

    if( unit->Type->Building ) {
	// FIXME: should find a better way for pending commands.
	command=&unit->PendCommand;
    } else if( !(command=GetNextCommand(unit,flush)) ) {
	return;
    }

    command->Action=UnitActionMove;
    command->Data.Move.Fast=1;
    command->Data.Move.Goal=NoUnitP;
    command->Data.Move.Range=0;
    command->Data.Move.SX=unit->X;
    command->Data.Move.SY=unit->Y;
    command->Data.Move.DX=x;
    command->Data.Move.DY=y;

    unit->SavedCommand.Action=UnitActionStill;	// clear saved action
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
    Command* command;

    IfDebug(
	if( x<0 || y<0 || x>=TheMap.Width || y>=TheMap.Height ) {
	    DebugLevel0("Internal movement error\n");
	    return;
	}
	// FIXME: dest until now not supported
	if( dest ) {
	    DebugLevel0(__FUNCTION__": not used %p\n",dest);
	}
    );


    if( unit->Type->Building ) {
	// FIXME: should find a better way for pending commands.
	command=&unit->PendCommand;
    } else if( !(command=GetNextCommand(unit,flush)) ) {
	return;
    }

    command->Action=UnitActionRepair;
    command->Data.Move.Fast=1;
    command->Data.Move.Goal=RepairableOnMapTile(x,y);
#ifdef NEW_UNIT
    command->Data.Move.Goal->Refs++;
#endif
    command->Data.Move.Range=REPAIR_RANGE;
    command->Data.Move.SX=unit->X;
    command->Data.Move.SY=unit->Y;
    command->Data.Move.DX=x;
    command->Data.Move.DY=y;

    unit->SavedCommand.Action=UnitActionStill;	// clear saved action
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
    Command* command;

    IfDebug(
	if( x<0 || y<0 || x>=TheMap.Width || y>=TheMap.Height ) {
	    DebugLevel0("Internal movement error\n");
	    return;
	}
	if( unit->Type->Vanishes ) {
	    DebugLevel0("Internal error\n");
	    abort();
	}
    );

    DebugLevel3(__FUNCTION__": %Zd attacks %Zd\n"
	,UnitNumber(unit),attack ? UnitNumber(attack) : 0);

    if( unit->Type->Building ) {
	// FIXME: should find a better way for pending commands.
	command=&unit->PendCommand;
    } else if( !(command=GetNextCommand(unit,flush)) ) {
	return;
    }

    command->Action=UnitActionAttack;
    command->Data.Move.Fast=1;
    // choose goal and good attack range
    if( attack ) {
	command->Data.Move.Goal=attack;
#ifdef NEW_UNIT
	attack->Refs++;
#endif
	command->Data.Move.Range=unit->Stats->AttackRange;
    } else {
	command->Data.Move.Goal=NoUnitP;
	if( WallOnMap(x,y) ) {
	    command->Data.Move.Range=unit->Stats->AttackRange;
	} else {
	    command->Data.Move.Range=0;
	}
    }
    command->Data.Move.SX=unit->X;
    command->Data.Move.SY=unit->Y;
    command->Data.Move.DX=x;
    command->Data.Move.DY=y;

    unit->SavedCommand.Action=UnitActionStill;	// clear saved action
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
    Command* command;

    IfDebug(
	if( x<0 || y<0 || x>=TheMap.Width || y>=TheMap.Height ) {
	    DebugLevel0("Internal movement error\n");
	    return;
	}
	DebugCheck( unit->Type->Vanishes );
    );

    if( !(command=GetNextCommand(unit,flush)) ) {
	return;
    }

    command->Action=UnitActionAttackGround;
    command->Data.Move.Fast=1;
    command->Data.Move.Goal=NoUnitP;
    command->Data.Move.Range=unit->Stats->AttackRange;
    command->Data.Move.SX=unit->X;
    command->Data.Move.SY=unit->Y;
    command->Data.Move.DX=x;
    command->Data.Move.DY=y;
    // FIXME: pathfinder didn't support this kind of target

    unit->SavedCommand.Action=UnitActionStill;	// clear saved action
}

/**
**	Let an unit patrol from current to new position
**
**	@param unit	pointer to unit.
**	@param x	X map position to patrol between.
**	@param y	Y map position to patrol between.
**	@param flush	if true, flush command queue.
*/
global void CommandPatrolUnit(Unit* unit,int x,int y,int flush)
{
    Command* command;

    IfDebug(
	if( x<0 || y<0 || x>=TheMap.Width || y>=TheMap.Height ) {
	    DebugLevel0("Internal movement error\n");
	    return;
	}
    );

    if( !(command=GetNextCommand(unit,flush)) ) {
	return;
    }

    command->Action=UnitActionPatrol;
    command->Data.Move.Fast=1;
    command->Data.Move.Goal=NoUnitP;
    command->Data.Move.Range=0;
    command->Data.Move.SX=unit->X;
    command->Data.Move.SY=unit->Y;
    command->Data.Move.DX=x;
    command->Data.Move.DY=y;

    unit->SavedCommand.Action=UnitActionStill;	// clear saved action
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
    Command* command;

    if( !(command=GetNextCommand(unit,flush)) ) {
	return;
    }

    command->Action=UnitActionBoard;
    command->Data.Move.Fast=1;
    command->Data.Move.Goal=dest;
#ifdef NEW_UNIT
    dest->Refs++;
#endif
    command->Data.Move.Range=1;
    command->Data.Move.SX=unit->X;
    command->Data.Move.SY=unit->Y;
    command->Data.Move.DX=dest->X;
    command->Data.Move.DY=dest->Y;

    unit->SavedCommand.Action=UnitActionStill;	// clear saved action
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
    Command* command;

    IfDebug(
	if( x<0 || y<0 || x>=TheMap.Width || y>=TheMap.Height ) {
	    DebugLevel0("Internal movement error\n");
	    return;
	}
    );

    if( !(command=GetNextCommand(unit,flush)) ) {
	return;
    }

    command->Action=UnitActionUnload;
    command->Data.Move.Fast=1;
    command->Data.Move.Goal=what;
#ifdef NEW_UNIT
    if( what ) {
	what->Refs++;
    }
#endif
    command->Data.Move.Range=0;
    command->Data.Move.SX=unit->X;
    command->Data.Move.SY=unit->Y;
    command->Data.Move.DX=x;
    command->Data.Move.DY=y;

    unit->SavedCommand.Action=UnitActionStill;	// clear saved action
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
    Command* command;

    if( !(command=GetNextCommand(unit,flush)) ) {
	return;
    }

    command->Action=UnitActionBuild;
    command->Data.Move.Fast=1;
    command->Data.Move.Goal=NoUnitP;
    // FIXME: only quadratic buildings supported!!!
    if( what->ShoreBuilding ) {
	command->Data.Move.Range=what->TileWidth+1;
	// FIXME: this hack didn't work correct on map border
	command->Data.Move.DX=x ? x-1 : x;
	command->Data.Move.DY=y ? y-1 : y;
    } else {
	command->Data.Move.Range=what->TileWidth-1;
	command->Data.Move.DX=x;
	command->Data.Move.DY=y;
    }
    // FIXME: must change movement for not build goal!

    command->Data.Move.SX=unit->X;
    command->Data.Move.SY=unit->Y;

    command->Data.Build.BuildThis=what;

    unit->SavedCommand.Action=UnitActionStill;	// clear saved action
}

/**
**	Cancel the building construction.
**
**	@param unit	pointer to unit.
**	@param worker	pointer to unit.
*/
global void CommandCancelBuilding(Unit* unit,Unit* worker)
{
    unit->NextCount=1;
    unit->NextFlush=1;

    unit->NextCommand[0].Action=UnitActionBuilded;
    unit->NextCommand[0].Data.Builded.Cancel=1;
    unit->NextCommand[0].Data.Builded.Worker=worker;

    unit->Wait=1;
    unit->Reset=1;

    unit->SavedCommand.Action=UnitActionStill;	// clear saved action
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
    Command* command;

    if( unit->Type->Building ) {
	// FIXME: should find a better way for pending commands.
	command=&unit->PendCommand;
    } else if( !(command=GetNextCommand(unit,flush)) ) {
	return;
    }

#if 0
    command->Action=UnitActionHarvest;
    command->Data.Move.Fast=1;
    command->Data.Move.Goal=NoUnitP;
    command->Data.Move.Range=1;
    command->Data.Move.SX=unit->X;
    command->Data.Move.SY=unit->Y;
    command->Data.Move.DX=x;
    command->Data.Move.DY=y;
#endif
    command->Action=UnitActionHarvest;
    command->Data.Move.Fast=1;
    command->Data.Move.Goal=NoUnitP;
    command->Data.Move.Range=2;
    command->Data.Move.SX=unit->X;
    command->Data.Move.SY=unit->Y;
    // FIXME: this hack didn't work correct on map border
    command->Data.Move.DX=x ? x-1 : x;
    command->Data.Move.DY=y ? y-1 : y;

    unit->SavedCommand.Action=UnitActionStill;	// clear saved action
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
    Command* command;

    if( unit->Type->Building ) {
	// FIXME: should find a better way for pending commands.
	command=&unit->PendCommand;
    } else if( !(command=GetNextCommand(unit,flush)) ) {
	return;
    }

    command->Action=UnitActionMineGold;
    command->Data.Move.Fast=1;
    command->Data.Move.Goal=dest;
#ifdef NEW_UNIT
    dest->Refs++;
#endif
    command->Data.Move.Range=1;
    command->Data.Move.SX=unit->X;
    command->Data.Move.SY=unit->Y;
#if 1
    // FIXME: move to any point of gold mine.
    NearestOfUnit(dest,unit->X,unit->Y
	    ,&command->Data.Move.DX
	    ,&command->Data.Move.DY);
#else
    command->Data.Move.DX=dest->X;
    command->Data.Move.DY=dest->Y;
#endif

    unit->SavedCommand.Action=UnitActionStill;	// clear saved action
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
    Command* command;

    if( unit->Type->Building ) {
	// FIXME: should find a better way for pending commands.
	command=&unit->PendCommand;
    } else if( !(command=GetNextCommand(unit,flush)) ) {
	return;
    }

    command->Action=UnitActionHaulOil;
    command->Data.Move.Fast=1;
    command->Data.Move.Goal=dest;
#ifdef NEW_UNIT
    dest->Refs++;
#endif
    command->Data.Move.Range=1;
    command->Data.Move.SX=unit->X;
    command->Data.Move.SY=unit->Y;
#if 1
    // FIXME: move to any point of gold mine.
    NearestOfUnit(dest,unit->X,unit->Y
	    ,&command->Data.Move.DX
	    ,&command->Data.Move.DY);
#else
    command->Data.Move.DX=dest->X;
    command->Data.Move.DY=dest->Y;
#endif

    unit->SavedCommand.Action=UnitActionStill;	// clear saved action
}

/**
**	Let unit returning goods.
**
**	@param unit	pointer to unit.
**	@param flush	if true, flush command queue.
*/
global void CommandReturnGoods(Unit* unit,int flush)
{
    // FIXME: flush, and command que not supported!
    unit->NextCount=1;
    unit->NextFlush=1;

    unit->NextCommand[0].Action=UnitActionReturnGoods;
    unit->NextCommand[0].Data.Move.Fast=1;
    unit->NextCommand[0].Data.Move.Goal=NoUnitP;
    unit->NextCommand[0].Data.Move.Range=1;
    unit->NextCommand[0].Data.Move.SX=unit->X;
    unit->NextCommand[0].Data.Move.SY=unit->Y;

    unit->SavedCommand.Action=UnitActionStill;	// clear saved action
}

/**
**	Building starts train.
**
**	@param unit	pointer to unit.
**	@param what	unit type to train.
**	@param flush	if true, flush command queue.
*/
global void CommandTrainUnit(Unit* unit,UnitType* what,int flush)
{
#if 0
    unit->NextCount=1;
    unit->NextFlush=1;

    if( unit->Command.Action!=UnitActionTrain ) {
	unit->Command.Action=UnitActionTrain;
	unit->Command.Data.Train.Count=0;
	unit->Command.Data.Train.Ticks=0;
    }

    if( unit->Command.Data.Train.Count==MAX_UNIT_TRAIN ) {
	// FIXME: johns: wrong place for an error message.
	// FIXME: johns: should be checked by AI or user interface
	SetMessage( "Unit queue is full" );
	return;
    }

    unit->Command.Data.Train.What[unit->Command.Data.Train.Count++]=what;

    unit->Wait=1;			// FIXME: correct this 
    unit->Reset=1;
#endif

    if( unit->Command.Action!=UnitActionTrain ) {
	unit->NextCommand[0].Action=UnitActionTrain;
	unit->NextCommand[0].Data.Train.What[0]=what;
	unit->NextCommand[0].Data.Train.Ticks=0;
	unit->NextCommand[0].Data.Train.Count=1;
	unit->NextCount=1;
	unit->NextFlush=1;
    } else {
	if( unit->Command.Data.Train.Count==MAX_UNIT_TRAIN ) {
	    // FIXME: johns: wrong place for an error message.
	    // FIXME: johns: should be checked by AI or user interface
	    SetMessage( "Unit queue is full" );
	    return;
	}
	unit->Command.Data.Train.What[unit->Command.Data.Train.Count++]=what;
    }

    unit->Wait=1;			// FIXME: correct this 
    unit->Reset=1;

    unit->SavedCommand.Action=UnitActionStill;	// clear saved action
}

/**
**	Cancel the training an unit.
**
**	@param unit	pointer to unit.
*/
global void CommandCancelTraining(Unit* unit,int slot)
{
    int i;

    DebugCheck( slot );

    if ( --unit->Command.Data.Train.Count ) {
	for( i = 0; i < MAX_UNIT_TRAIN-1; i++ ) {
	    unit->Command.Data.Train.What[i] =
	    unit->Command.Data.Train.What[i+1];
	}
	unit->Command.Data.Train.Ticks=0;
    } else {
	unit->Command.Action=UnitActionStill;
    }

    unit->Wait=1;
    unit->Reset=1;

    unit->SavedCommand.Action=UnitActionStill;	// clear saved action
}

/**
**	Building starts upgrading to.
**
**	@param unit	pointer to unit.
**	@param what	upgrade to what
**	@param flush	if true, flush command queue.
*/
global void CommandUpgradeTo(Unit* unit,UnitType* what,int flush)
{
    unit->NextCount=1;
    unit->NextFlush=1;

    unit->NextCommand[0].Action=UnitActionUpgradeTo;
    unit->NextCommand[0].Data.UpgradeTo.Ticks=0;
    unit->NextCommand[0].Data.UpgradeTo.What=what;

    unit->Wait=1;			// FIXME: correct this
    unit->Reset=1;

    unit->SavedCommand.Action=UnitActionStill;	// clear saved action
}

/**
**	Cancel Building upgrading to.
**
**	@param unit	pointer to unit.
**	@param flush	if true, flush command queue.
*/
global void CommandCancelUpgradeTo(Unit* unit)
{
    unit->Command.Action=UnitActionStill;

    unit->Wait=1;			// FIXME: correct this
    unit->Reset=1;

    unit->SavedCommand.Action=UnitActionStill;	// clear saved action
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
    unit->NextCount=1;
    unit->NextFlush=1;

    DebugLevel0(__FUNCTION__": FIXME: must support command queing!!");
    unit->NextCommand[0].Action=UnitActionResearch;
    unit->NextCommand[0].Data.Research.Ticks=0;
    unit->NextCommand[0].Data.Research.What=what;

    unit->Wait=1;			// FIXME: correct this
    unit->Reset=1;

    unit->SavedCommand.Action=UnitActionStill;	// clear saved action
}

/**
**	Cancel Building researching.
**
**	@param unit	pointer to unit.
**	@param flush	if true, flush command queue.
*/
global void CommandCancelResearch(Unit* unit)
{
    unit->Command.Action=UnitActionStill;

    unit->Wait=1;			// FIXME: correct this
    unit->Reset=1;

    unit->SavedCommand.Action=UnitActionStill;	// clear saved action
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
    Command* command;

    IfDebug(
	if( x<0 || y<0 || x>=TheMap.Width || y>=TheMap.Height ) {
	    DebugLevel0("Internal movement error\n");
	    return;
	}
    );

    if( unit->Type->Building ) {
	// FIXME: should find a better way for pending commands.
	command=&unit->PendCommand;
    } else if( !(command=GetNextCommand(unit,flush)) ) {
	return;
    }

    command->Action=UnitActionDemolish;
    command->Data.Move.Fast=1;
    // choose goal and good attack range
    if( dest ) {
	command->Data.Move.Goal=dest;
#ifdef NEW_UNIT
	dest->Refs++;
#endif
	command->Data.Move.Range=1;
    } else {
	command->Data.Move.Goal=NoUnitP;
	if( WallOnMap(x,y) ) {
	    command->Data.Move.Range=1;
	} else {
	    command->Data.Move.Range=0;
	}
    }
    command->Data.Move.SX=unit->X;
    command->Data.Move.SY=unit->Y;
    command->Data.Move.DX=x;
    command->Data.Move.DY=y;

    unit->SavedCommand.Action=UnitActionStill;	// clear saved action
}

//@}
