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
/**@name action_repair.c	-	The repair action. */
//
//	(c) Copyright 1999,2000 by Vladi Shabanski
//
//	$Id$

//@{

/*----------------------------------------------------------------------------
--      Include
----------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>

#include "freecraft.h"
#include "unittype.h"
#include "player.h"
#include "unit.h"
#include "missile.h"
#include "actions.h"
#include "sound.h"
#include "tileset.h"
#include "map.h"
#include "pathfinder.h"

/*----------------------------------------------------------------------------
--      Functions
----------------------------------------------------------------------------*/

/**
**	Generic unit repair.
**
**	@param unit	Unit, for that the repair animation is played.
**	@param repair	Repair animation.
*/
local void DoActionRepairGeneric(Unit* unit,const Animation* repair)
{
    int flags;

    flags=UnitShowAnimation(unit,repair);

    if( (flags&AnimationSound) ) {	
	PlayUnitSound(unit,VoiceAttacking); //FIXME: should be something else...
    }
}

/**
**	Repair an unit.
*/
local void RepairUnit(Unit* unit,Unit* goal)
{
    Player* player;
    int costs[MaxCosts];
    int i;
    int hp;
#define GIVES_HP	8
#define MUL		2
#define DIVISOR		2

    player=unit->Player;

    //	FIXME: Should substract the correct values for repair
    //
    //	Calculate the repair points
    //
    hp=((goal->Stats->Costs[TimeCost]*GIVES_HP*FRAMES_PER_SECOND/6)
	    /goal->Stats->HitPoints)*MUL;
    DebugLevel3Fn("hitpoints %d\n",hp);

    //
    //	Calculate the repair costs.
    //
    DebugCheck( !goal->Stats->HitPoints );
    
    for( i=1; i<MaxCosts; ++i ) {
	costs[i]=((goal->Stats->Costs[i]*hp)/goal->Stats->HitPoints)/DIVISOR;

	// FIXME: unit costs something but to less costs calculated
	IfDebug(
	    if( !costs[i] && goal->Stats->Costs[i] ) {
		DebugLevel0("Costs %d-%d\n",i,costs[i]);
	    }
	);
    }
    //
    //	Check if enough resources are available
    //
    for( i=1; i<MaxCosts; ++i ) {
	if( player->Resources[i]<costs[i] ) {
    // FIXME: we should show a message, we need resources!
    // FIXME: perhaps we should not animate if no resources are available.
	    return;
	}
    }
    //
    //	Repair the unit
    //
    goal->HP+=hp;
    if ( goal->HP > goal->Stats->HitPoints ) {
	goal->HP = goal->Stats->HitPoints;
    }
    //
    //	Subtract the resources
    //
    PlayerSubCosts(player,costs);

    if( UnitVisible(goal) ) {
        MustRedraw|=RedrawMaps;
    }
    if( IsSelected(goal) ) {		// Update panel if unit is selected
	MustRedraw|=RedrawInfoPanel;
    }
}

/**
**	Animate unit repair
**
**	@param unit	Unit, for that the repair animation is played.
*/
local int AnimateActionRepair(Unit* unit)
{
    if( unit->Type->Animations ) {
	DebugCheck( !unit->Type->Animations->Attack );
	// FIXME: An seperate repair animation would be nice?
	DoActionRepairGeneric(unit,unit->Type->Animations->Attack);
    }

    return 0;
}

/**
**	Unit repairs
**
**	@param unit	Unit, for that the attack is handled.
*/
global int HandleActionRepair(Unit* unit)
{
    Unit* goal;
    int err;

    DebugLevel3("Repair %d\n",unit-Units);

    switch( unit->SubAction ) {
	//
	//	Move near to target.
	//
	case 0:
	    // FIXME: RESET FIRST!! Why? (Johns)
	    err=HandleActionMove(unit); 
	    if( unit->Reset ) {
		//
		//	No goal: if meeting enemy attack it.
		//
		goal=unit->Command.Data.Move.Goal;

		//
		//	Target is dead, choose new one.
		//
		// Check if goal is correct unit.
		// FIXME: should I do a function for this?
		if( goal ) {
		    if( goal->Destroyed ) {
			DebugLevel0Fn("destroyed unit\n");
			if( !--goal->Refs ) {
			    ReleaseUnit(goal);
			}
			// FIXME: should I clear this here?
			unit->Command.Data.Move.Goal=goal=NULL;
			unit->Command.Data.Move.DX=goal->X;
			unit->Command.Data.Move.DY=goal->Y;
		    } else if( !goal->HP ||
				goal->Command.Action==UnitActionDie ) {
			// FIXME: should I clear this here?
			unit->Command.Data.Move.Goal=goal=NULL;
			unit->Command.Data.Move.DX=goal->X;
			unit->Command.Data.Move.DY=goal->Y;
		    }
		}
		//
		//	Have reached target? FIXME: could use return value
		//
		if( goal && MapDistanceToUnit(unit->X,unit->Y,goal)
			<=REPAIR_RANGE ) {
		    unit->State=0;
		    unit->SubAction=1;
		} else if( err<0 ) {
		    DebugCheck( unit->Command.Action!=UnitActionStill );
		    if( goal ) {		// release reference
			goal->Refs--;
		    }
		    return 1;
		}
		unit->Command.Action=UnitActionRepair;
	    }
	    break;

	//
	//	Repair the target.
	//
	case 1:
	    AnimateActionRepair(unit);
	    if( unit->Reset ) {
		goal=unit->Command.Data.Move.Goal;

		//
		//	Target is dead, choose new one.
		//
		// Check if goal is correct unit.
		// FIXME: should I do a function for this?
		if( goal ) {
		    if( goal->Destroyed ) {
			DebugLevel0Fn("destroyed unit\n");
			if( !--goal->Refs ) {
			    ReleaseUnit(goal);
			}
			// FIXME: should I clear this here?
			unit->Command.Data.Move.Goal=goal=NULL;
			unit->Command.Data.Move.DX=goal->X;
			unit->Command.Data.Move.DY=goal->Y;
		    } else if( !goal->HP
				|| goal->Command.Action==UnitActionDie ) {
			// FIXME: should I clear this here?
			unit->Command.Data.Move.Goal=goal=NULL;
			unit->Command.Data.Move.DX=goal->X;
			unit->Command.Data.Move.DY=goal->Y;
		    }
		}
		if( goal ) {
		    RepairUnit(unit,goal);
		}

		//
		//	Target is fine, choose new one.
		//
		if( !goal || goal->HP >= goal->Stats->HitPoints ) {
		    if( goal ) {		// release reference
			goal->Refs--;
		    }
                    unit->Command.Action=UnitActionStill;
		    unit->SubAction=0;
		    unit->State=0;
                    return 1;
		}

		// FIXME: automatic repair
	    }
	    break;
    }
    return 0;
}

//@}
