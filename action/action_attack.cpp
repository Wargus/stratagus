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
#include "unittype.h"
#include "player.h"
#include "unit.h"
#include "missile.h"
#include "actions.h"
#include "sound.h"
#include "map.h"
#include "pathfinder.h"

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
	if( (unit->Frame&127)>=VideoGraphicFrames(unit->Type->Sprite) ) {
	    DebugLevel0Fn("Oops what this %s %d,%d %d #%d\n"
		,unit->Type->Ident
		,oframe,oframe&127
		,unit->Frame&127
		,VideoGraphicFrames(unit->Type->Sprite));
	    SaveUnit(unit,stdout);
	    abort();
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
**	Handle moving to the target.
**
**	@param unit	Unit, for that the attack is handled.
*/
local void MoveToTarget(Unit* unit)
{
    Unit* goal;
    int wall;
    int err;

    if( unit->Command.Action==UnitActionAttackGround
	|| WallOnMap(unit->Command.Data.Move.DX,unit->Command.Data.Move.DY) ) { 
	// FIXME: workaround for pathfinder problem
	unit->Command.Data.Move.DX-=unit->Command.Data.Move.Range;
	unit->Command.Data.Move.DY-=unit->Command.Data.Move.Range;
	unit->Command.Data.Move.Range*=2;
	err=HandleActionMove(unit);
	unit->Command.Data.Move.Range/=2;
	unit->Command.Data.Move.DX+=unit->Command.Data.Move.Range;
	unit->Command.Data.Move.DY+=unit->Command.Data.Move.Range;
	unit->Command.Action=UnitActionAttackGround;
    } else {
	err=HandleActionMove(unit);
    }

    // NEW return codes supported, FIXME: but I think not perfect.

    if( unit->Reset ) {
	//
	//	Target is dead, choose new one.
	//
#ifdef NEW_UNIT
	if( (goal=unit->Command.Data.Move.Goal) ) {
	    // FIXME: Should be done by Action Move???????
	    if( goal->Destroyed ) {
		DebugLevel0Fn("destroyed unit\n");
		if( !--goal->Refs ) {
		    ReleaseUnit(goal);
		}
		unit->Command.Data.Move.Goal=goal=NoUnitP;
	    } else if( !goal->HP || goal->Command.Action==UnitActionDie ) {
		--goal->Refs;
		unit->Command.Data.Move.Goal=goal=NoUnitP;
	    }
	}
#else
	goal=unit->Command.Data.Move.Goal;
	if( goal && (!goal->HP || goal->Command.Action==UnitActionDie) ) {
	    unit->Command.Data.Move.Goal=goal=NoUnitP;
	}
#endif

	//
	//	No goal: if meeting enemy attack it.
	//
	wall=0;
	if( !goal && !(wall=WallOnMap(unit->Command.Data.Move.DX
		     ,unit->Command.Data.Move.DY))
		&& unit->Command.Action!=UnitActionAttackGround ) {
	    goal=AttackUnitsInReactRange(unit);
	    if( goal ) {
#ifdef NEW_UNIT
		goal->Refs++;
#endif
		if( unit->SavedCommand.Action==UnitActionStill ) {
		    // Save current command to come back.
		    unit->SavedCommand=unit->Command;
		}
		unit->Command.Data.Move.Goal=goal;
		ResetPath(unit->Command);
		unit->Command.Data.Move.DX=goal->X;
		unit->Command.Data.Move.DY=goal->Y;
		unit->SubAction|=2;		// weak target
		DebugLevel3Fn("%Zd in react range %Zd\n"
			,UnitNumber(unit),UnitNumber(goal));
	    }

	//
	//	Have a weak target, try a better target.
	//
	} else if( goal && (unit->SubAction&2) ) {
	    Unit* temp;

	    temp=AttackUnitsInReactRange(unit);
	    if( temp && temp->Type->Priority>goal->Type->Priority ) {
#ifdef NEW_UNIT
		goal->Refs--;
		temp->Refs++;
#endif
		if( unit->SavedCommand.Action==UnitActionStill ) {
		    // Save current command to come back.
		    unit->SavedCommand=unit->Command;
		}
		unit->Command.Data.Move.Goal=goal=temp;
		ResetPath(unit->Command);
		unit->Command.Data.Move.DX=goal->X;
		unit->Command.Data.Move.DY=goal->Y;
	    }
	}

	//
	//	Have reached target? FIXME: could use the new return code?
	//
	if( goal && MapDistanceToUnit(unit->X,unit->Y,goal)
		<=unit->Stats->AttackRange ) {
	    unit->State=0;
	    if( !unit->Type->Tower ) {
		UnitHeadingFromDeltaXY(unit,goal->X-unit->X,goal->Y-unit->Y);
	    }
	    unit->SubAction++;
	} else if( (wall || unit->Command.Action==UnitActionAttackGround) 
		&& MapDistance(unit->X,unit->Y
		    ,unit->Command.Data.Move.DX,unit->Command.Data.Move.DY)
			<=unit->Stats->AttackRange ) {
	    DebugLevel3Fn("Attacking wall or ground\n");
	    unit->State=0;
	    if( !unit->Type->Tower ) {
		UnitHeadingFromDeltaXY(unit,unit->Command.Data.Move.DX-unit->X
		    ,unit->Command.Data.Move.DY-unit->Y);
	    }
	    unit->SubAction=1;
	    return;
	} else if( err<0 ) {
	    unit->State=0;
	    unit->SubAction=0;
	    // Return to old task?
	    if( unit->Command.Action==UnitActionStill ) {
		unit->Command=unit->SavedCommand;
		// Must finish if saved command finishes
		unit->SavedCommand.Action=UnitActionStill;
	    }
	    return;
	}
	DebugCheck( unit->Type->Vanishes || unit->Destroyed );
	unit->Command.Action=UnitActionAttack;
    }
}

/**
**	Handle attacking the target.
**
**	@param unit	Unit, for that the attack is handled.
*/
local void AttackTarget(Unit* unit)
{
    Unit* goal;

    AnimateActionAttack(unit);
    if( unit->Reset ) {
	goal=unit->Command.Data.Move.Goal;
	//
	//	Goal is "weak" or a wall.
	//
	if( !goal && (WallOnMap(unit->Command.Data.Move.DX
		     ,unit->Command.Data.Move.DY)
		|| unit->Command.Action==UnitActionAttackGround) ) {
	    DebugLevel3Fn("attack a wall!!!!\n");
	    return;
	}

	//
	//	Target is dead, choose new one.
	//
#ifdef NEW_UNIT
	if( goal ) {
	    if( goal->Destroyed ) {
		DebugLevel0Fn("destroyed unit\n");
		if( !--goal->Refs ) {
		    ReleaseUnit(goal);
		}
		unit->Command.Data.Move.Goal=goal=NoUnitP;
	    } else if( !goal->HP || goal->Command.Action==UnitActionDie ) {
		// FIXME: goal->Removed???
		--goal->Refs;
		unit->Command.Data.Move.Goal=goal=NoUnitP;
	    }
	}
	//
	//	No target choose one.
	//
	if( !goal ) {
	    unit->State=0;
	    goal=AttackUnitsInReactRange(unit);
	    if( !goal ) {
		unit->SubAction=0;
		// Return to old task!
		unit->Command=unit->SavedCommand;
		// Must finish if saved command finishes
		unit->SavedCommand.Action=UnitActionStill;
		return;
	    }
	    if( unit->SavedCommand.Action==UnitActionStill ) {
		// Save current command to come back.
		unit->SavedCommand=unit->Command;
	    }
#ifdef NEW_UNIT
	    goal->Refs++;
#endif
	    DebugLevel3Fn("%Zd Unit in react range %Zd\n"
		    ,UnitNumber(unit),UnitNumber(goal));
	    unit->Command.Data.Move.Goal=goal;
	    unit->Command.Data.Move.DX=goal->X;
	    unit->Command.Data.Move.DY=goal->Y;
	    unit->SubAction|=2;
	} else
#else
	if( !goal || !goal->HP || goal->Command.Action==UnitActionDie ) {
	    // FIXME: goal->Removed???
	    unit->State=0;
	    goal=AttackUnitsInReactRange(unit);
	    unit->Command.Data.Move.Goal=goal;
	    if( !goal ) {
		unit->SubAction=0;
		unit->Command.Action=UnitActionStill;	// cade?
		return;
	    }
	    unit->SubAction|=2;
	    DebugLevel3Fn("Unit in react range %Zd\n",UnitNumber(goal));
	    unit->Command.Data.Move.DX=goal->X;
	    unit->Command.Data.Move.DY=goal->Y;
	} else
#endif

	//
	//	Have a weak target, try a better target.
	//
	if( goal && (unit->SubAction&2) ) {
	    Unit* temp;

	    temp=AttackUnitsInReactRange(unit);
	    if( temp && temp->Type->Priority>goal->Type->Priority ) {
#ifdef NEW_UNIT
		goal->Refs--;
		temp->Refs++;
#endif
		if( unit->SavedCommand.Action==UnitActionStill ) {
		    // Save current command to come back.
		    unit->SavedCommand=unit->Command;
		}
		unit->Command.Data.Move.Goal=goal=temp;
		unit->Command.Data.Move.DX=goal->X;
		unit->Command.Data.Move.DY=goal->Y;
	    }
	}

	//
	//	Still near to target, if not goto target.
	//
	if( MapDistanceToUnit(unit->X,unit->Y,goal)
		>unit->Stats->AttackRange ) {
	    if( unit->SavedCommand.Action==UnitActionStill ) {
		// Save current command to come back.
		unit->SavedCommand=unit->Command;
	    }
	    ResetPath(unit->Command);
	    unit->Command.Data.Move.DX=goal->X;
	    unit->Command.Data.Move.DY=goal->Y;
	    unit->Frame=0;
	    unit->State=0;
	    unit->SubAction--;
	}

	//
	//	Turn always to target
	//
	if( !unit->Type->Tower && goal ) {
	    UnitHeadingFromDeltaXY(unit,goal->X-unit->X,goal->Y-unit->Y);
	}
    }
}

/**
**	Unit attacks!
**
**	I added a little trick, if SubAction&2 is true the goal is a weak goal.
**	This means the unit AI (little AI) could choose a new better goal.
**
**	@param unit	Unit, for that the attack is handled.
*/
global void HandleActionAttack(Unit* unit)
{
    DebugLevel3Fn("Attack %Zd\n",UnitNumber(unit));

    switch( unit->SubAction ) {
	//
	//	Move near to the target.
	//
	case 0:
	case 2:
	    MoveToTarget(unit);
	    break;

	//
	//	Attack the target.
	//
	case 1:
	case 3:
	    AttackTarget(unit);
	    break;
    }
}

//@}
