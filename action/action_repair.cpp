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
//	(c) Copyright 1999-2001 by Vladi Shabanski
//
//	$Id$

//@{

/*----------------------------------------------------------------------------
--      Includes
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
#if 0
    hp=((goal->Stats->Costs[TimeCost]*GIVES_HP*FRAMES_PER_SECOND/6)
	    /goal->Stats->HitPoints)*MUL;
#endif
    hp=GIVES_HP*MUL;
    DebugLevel2Fn("hitpoints %d\n",hp);

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
	  SetMessage("We need resources!");
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

    if ( CheckUnitToBeDrawn(goal) ) {
	MustRedraw|=RedrawMinimap;
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
	// FIXME: A seperate repair animation would be nice?
	DoActionRepairGeneric(unit,unit->Type->Animations->Attack);
    }

    return 0;
}

/**
**	Unit repairs
**
**	@param unit	Unit, for that the attack is handled.
*/
global void HandleActionRepair(Unit* unit)
{
    Unit* goal;
    int err;

    DebugLevel3("Repair %Zd\n",UnitNumber(unit));

    switch( unit->SubAction ) {
	case 0:
	    NewResetPath(unit);
	    unit->SubAction=1;
	    // FALL THROUGH
	//
	//	Move near to target.
	//
	case 1:
	    // FIXME: RESET FIRST!! Why? (Johns)
	    err=DoActionMove(unit);
	    if( unit->Reset ) {
		//
		//	No goal: if meeting damaged building repair it.
		//
		goal=unit->Orders[0].Goal;

		//
		//	Target is dead, choose new one.
		//
		// Check if goal is correct unit.
		// FIXME: should I do a function for this?
		if( goal ) {
		    if( goal->Destroyed ) {
			DebugLevel0Fn("destroyed unit\n");
			unit->Orders[0].X=goal->X;
			unit->Orders[0].Y=goal->Y;
			RefsDebugCheck( !goal->Refs );
			if( !--goal->Refs ) {
			    ReleaseUnit(goal);
			}
			// FIXME: should I clear this here?
			unit->Orders[0].Goal=goal=NULL;
			NewResetPath(unit);
		    } else if( !goal->HP ||
				goal->Orders[0].Action==UnitActionDie ) {
			unit->Orders[0].X=goal->X;
			unit->Orders[0].Y=goal->Y;
			RefsDebugCheck( !goal->Refs );
			--goal->Refs;
			RefsDebugCheck( !goal->Refs );
			unit->Orders[0].Goal=goal=NULL;
			// FIXME: should I clear this here?
			NewResetPath(unit);
		    }
		}
		//
		//	Have reached target? FIXME: could use return value
		//
		if( goal && MapDistanceToUnit(unit->X,unit->Y,goal)
			<=REPAIR_RANGE ) {
		    unit->State=0;
		    unit->SubAction=2;
		    UnitHeadingFromDeltaXY(unit,
			    goal->X-unit->X,goal->Y-unit->Y);
		    // FIXME: only if heading changes
		    CheckUnitToBeDrawn(unit);
		} else if( err<0 ) {
		    if( goal ) {		// release reference
			RefsDebugCheck( !goal->Refs );
			goal->Refs--;
			RefsDebugCheck( !goal->Refs );
			unit->Orders[0].Goal=NoUnitP;
		    }
		    unit->Orders[0].Action=UnitActionStill;
		    unit->State=0;
		    unit->SubAction=0;
		    return;
		}
		// FIXME: Should be it already?
		DebugCheck( unit->Orders[0].Action!=UnitActionRepair );
	    }
	    break;

	//
	//	Repair the target.
	//
	case 2:
	    AnimateActionRepair(unit);
	    if( unit->Reset ) {
		goal=unit->Orders[0].Goal;

		//
		//	Target is dead, choose new one.
		//
		// Check if goal is correct unit.
		// FIXME: should I do a function for this?
		if( goal ) {
		    if( goal->Destroyed ) {
			DebugLevel0Fn("destroyed unit\n");
			unit->Orders[0].X=goal->X;
			unit->Orders[0].Y=goal->Y;
			RefsDebugCheck( !goal->Refs );
			if( !--goal->Refs ) {
			    ReleaseUnit(goal);
			}
			// FIXME: should I clear this here?
			unit->Orders[0].Goal=goal=NULL;
			NewResetPath(unit);
		    } else if( !goal->HP
				|| goal->Orders[0].Action==UnitActionDie ) {
			// FIXME: should I clear this here?
			unit->Orders[0].X=goal->X;
			unit->Orders[0].Y=goal->Y;
			unit->Orders[0].Goal=goal=NULL;
			NewResetPath(unit);
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
			RefsDebugCheck( !goal->Refs );
			goal->Refs--;
			RefsDebugCheck( !goal->Refs );
			unit->Orders[0].Goal=NULL;
		    }
                    unit->Orders[0].Action=UnitActionStill;
		    unit->SubAction=0;
		    unit->State=0;
                    return;
		}

		// FIXME: automatic repair
	    }
	    break;
    }
}

//@}
