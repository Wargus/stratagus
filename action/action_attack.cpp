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
/**@name action_attack.c	-	The attack action. */
/*
**	(c) Copyright 1998-2000 by Lutz Sammer
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
#include "missile.h"
#include "actions.h"
#include "sound.h"
#include "tileset.h"
#include "map.h"

/*----------------------------------------------------------------------------
--	Functions
----------------------------------------------------------------------------*/

/**
**	Generic unit attacker.
**
**	@param unit	Unit, for that the attack animation is played.
**	@param attack	Attack animation.
*/
local void DoActionAttackGeneric(Unit* unit,const Animation* attack)
{
    int flags;

    IfDebug(
    int oframe;

    oframe=unit->Frame;
    );

    flags=UnitShowAnimation(unit,attack);

    IfDebug(
	if( (unit->Frame&127)>=unit->Type->RleSprite->NumFrames ) {
	    DebugLevel0("Oops what this %s %d,%d %d #%d\n"
		,unit->Type->Ident
		,oframe,oframe&127
		,unit->Frame&127
		,unit->Type->RleSprite->NumFrames);
	    SaveUnit(unit,stdout);
	    abort();
	    return;
	}
    );

    if( (flags&AnimationSound) ) {	
	PlayUnitSound(unit,VoiceAttacking);
    }

    if( flags&AnimationMissile ) {	// time to fire projectil
	FireMissile(unit);
    }
}

/**
**	Animate unit attack!
**
**	@param unit	Unit, for that the attack animation is played.
*/
global int AnimateActionAttack(Unit* unit)
{
    if( unit->Type->Animations ) {
	DebugCheck( !unit->Type->Animations->Attack );
	DoActionAttackGeneric(unit,unit->Type->Animations->Attack);
    }

    return 0;
}

/**
**	Unit attacks!
**
**	I added a little trick, if SubAction&2 is true the goal is a weak goal.
**	This means the unit AI (little AI) could choose a new better goal.
**
**	@param unit	Unit, for that the attack is handled.
*/
global int HandleActionAttack(Unit* unit)
{
    Unit* goal;
    int wall;
    int err;

    DebugLevel3(__FUNCTION__": Attack %Zd\n",UnitNumber(unit));

    switch( unit->SubAction ) {
	//
	//	Move near to target.
	//
	case 0:
	case 2:
	    // FIXME: RESET FIRST!!
	    err=HandleActionMove(unit); 
	    if( unit->Reset ) {
		//
		//	Target is dead, choose new one.
		//
		goal=unit->Command.Data.Move.Goal;
		if( goal && (!goal->HP
			|| goal->Command.Action==UnitActionDie) ) {
		    unit->Command.Data.Move.Goal=goal=NoUnitP;
		}

		//
		//	No goal: if meeting enemy attack it.
		//
		wall=0;
		if( !goal
			&& !(wall=WallOnMap(unit->Command.Data.Move.DX
			     ,unit->Command.Data.Move.DY)) ) {
		    goal=AttackUnitsInReactRange(unit);
		    if( goal ) {
			unit->Command.Data.Move.Goal=goal;
			unit->Command.Data.Move.Fast=1;
			unit->Command.Data.Move.DX=goal->X;
			unit->Command.Data.Move.DY=goal->Y;
			unit->SubAction|=2;
			DebugLevel3("Unit in react range %Zd\n",UnitNumber(goal));
		    }
		} else 

		//
		//	Have a weak target, try a better target.
		//
		if( goal && (unit->SubAction&2) ) {
		    Unit* temp;

		    temp=AttackUnitsInReactRange(unit);
		    if( temp && temp->Type->Priority>goal->Type->Priority ) {
			unit->Command.Data.Move.Goal=goal=temp;
			unit->Command.Data.Move.Fast=1;
			unit->Command.Data.Move.DX=temp->X;
			unit->Command.Data.Move.DY=temp->Y;
		    }
		}

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
		    unit->SubAction++;
		} else if( wall && MapDistance(unit->X,unit->Y
			    ,unit->Command.Data.Move.DX
			    ,unit->Command.Data.Move.DY)
				<=unit->Stats->AttackRange ) {
		    DebugLevel3("Attacking wall\n");
		    unit->State=0;
		    if( !unit->Type->Tower ) {
			UnitNewHeadingFromXY(unit
			    ,unit->Command.Data.Move.DX-unit->X
			    ,unit->Command.Data.Move.DY-unit->Y);
		    }
		    unit->SubAction=1;
		} else if( err ) {
		    unit->SubAction=0;
		    return 1;
		}
		DebugCheck( unit->Type->Vanishes );
		unit->Command.Action=UnitActionAttack;
	    }
	    break;

	//
	//	Attack the target.
	//
	case 1:
	case 3:
	    AnimateActionAttack(unit);
	    if( unit->Reset ) {
		goal=unit->Command.Data.Move.Goal;
		//
		//	Goal is "weak" or a wall.
		//
		if( !goal && WallOnMap(unit->Command.Data.Move.DX
			     ,unit->Command.Data.Move.DY) ) {
		    DebugLevel3("attack a wall!!!!\n");
		    break;
		}

		//
		//	Target is dead, choose new one.
		//
		if( !goal || !goal->HP
			|| goal->Command.Action==UnitActionDie ) {
		    unit->State=0;
		    goal=AttackUnitsInReactRange(unit);
		    unit->Command.Data.Move.Goal=goal;
		    if( !goal ) {
			unit->SubAction=0;
			unit->Command.Action=UnitActionStill;	// cade?
			return 1;
		    }
		    unit->SubAction|=2;
		    DebugLevel3("Unit in react range %Zd\n",UnitNumber(goal));
		    unit->Command.Data.Move.DX=goal->X;
		    unit->Command.Data.Move.DY=goal->Y;
		    if( !unit->Type->Tower ) {
			UnitNewHeadingFromXY(unit
			    ,goal->X-unit->X,goal->Y-unit->Y);
		    }
		} else

		//
		//	Have a weak target, try a better target.
		//
		if( goal && (unit->SubAction&2) ) {
		    Unit* temp;

		    temp=AttackUnitsInReactRange(unit);
		    if( temp && temp->Type->Priority>goal->Type->Priority ) {
			unit->Command.Data.Move.Goal=goal=temp;
			unit->Command.Data.Move.DX=goal->X;
			unit->Command.Data.Move.DY=goal->Y;
			if( !unit->Type->Tower ) {
			    UnitNewHeadingFromXY(unit
				,goal->X-unit->X,goal->Y-unit->Y);
			}
		    }
		}

		//
		//	Still near to target, if not goto target.
		//
		if( MapDistanceToUnit(unit->X,unit->Y,goal)
			>unit->Stats->AttackRange ) {
		    unit->Command.Data.Move.Fast=1;
		    unit->Command.Data.Move.DX=goal->X;
		    unit->Command.Data.Move.DY=goal->Y;
		    unit->Frame=0;
		    unit->State=0;
		    unit->SubAction--;
		    break;
		}
	    }
	    break;
    }

    return 0;
}

//@}
