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
/*
**	(c) Copyright 1999,2000 by Vladi Shabanski
**
**	$Id$
*/


/*

  This is a quick hack: repair action, done with attack action reversing
  :)
  
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

/*----------------------------------------------------------------------------
--      Repair
----------------------------------------------------------------------------*/

//	Peon, Peasant, Attacking Peon, Attacking Peasant.
local Animation PeonRepair[] = {
    { 0, 0, 3, 25},{ 0, 0, 3,  5},{ 0, 0, 3,  5},{12, 0, 5,  5},{ 0, 0, 3,  5},
    { 0, 0, 7,-20},{ 3, 0, 1,  0}
};

/**
**	Generic unit repair.
**
**	@param unit	Unit, for that the repair animation is played.
**	@param repair	Repair animation.
*/
local void DoActionRepairGeneric(Unit* unit,Animation* repair)
{
    Unit* goal;
    int flags;
    Player* player;

    flags=UnitShowAnimation(unit,repair);

    if( (flags&AnimationSound) ) {	
	PlayUnitSound(unit,VoiceAttacking); //FIXME: should be something else...
    }

    goal=unit->Command.Data.Move.Goal;
    if (!goal) {
	// FIXME: Should abort the repair
	return;
    }

    // FIXME: Should substract the correct values for repair

    //	Check if enough resources are available

    player=unit->Player;
    if( !player->Resources[GoldCost] || !player->Resources[WoodCost] ) {
	// FIXME: perhaps we should not animate if no resources are available.
	return;
    }

    //	Repair the unit

    goal->HP++;
    if ( goal->HP > goal->Stats->HitPoints ) {
	goal->HP = goal->Stats->HitPoints;
    }

    //	Subtract the resources

    player->Resources[GoldCost]--;		// FIXME: correct sources?
    player->Resources[WoodCost]--;

    //	Must update panel if unit is selected
    if( IsSelected(goal) ) {
	MustRedraw|=RedrawInfoPanel;
    }

    if( player==ThisPlayer ) {
	MustRedraw|=RedrawResources;
    }
}

/**
**	Animate unit repair
**
**	@param unit	Unit, for that the repair animation is played.
*/
local int AnimateActionRepair(Unit* unit)
{
    int type;

    type=unit->Type->Type;
    if( type<UnitTypeInternalMax ) {
	DoActionRepairGeneric(unit,PeonRepair);
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
	    // FIXME: RESET FIRST!!
	    err=HandleActionMove(unit); 
	    if( unit->Reset ) {
		//
		//	No goal: if meeting enemy attack it.
		//
		goal=unit->Command.Data.Move.Goal;

		//
		//	Target is dead, choose new one.
		//

		//
		//	Have reached target?
		//
		if( goal && MapDistanceToUnit(unit->X,unit->Y,goal)
			<=unit->Stats->AttackRange ) {
		    unit->State=0;
		    if( !unit->Type->Tower ) {
			UnitNewHeadingFromXY(unit
			    ,goal->X-unit->X,goal->Y-unit->Y);
		    }
		    unit->SubAction=1;
		} else if( err ) {
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
		//	Target is fine, choose new one.
		//
                if ( !goal || goal->HP >= goal->Stats->HitPoints ) {
                    unit->Command.Action=UnitActionStill;
		    unit->SubAction=0;
		    unit->State=0;
                    return 1;
		}
#if 0
		//
		//	Still near to target, if not goto target.
		//
		if( MapDistanceToUnit(unit->X,unit->Y,goal)
			>unit->Type->AttackRange ) {
		    unit->Command.Data.Move.Fast=1;
		    unit->Command.Data.Move.DX=goal->X;
		    unit->Command.Data.Move.DY=goal->Y;
		    unit->Frame=0;
		    unit->State=0;
		    unit->SubAction=0;
		    break;
		}
#endif
	    }
	    break;
    }

    return 0;
}

//@}
