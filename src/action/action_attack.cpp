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
//
//	(c) Copyright 1998-2001 by Lutz Sammer
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
	unit->Invisible = 0;		// unit is invisible until attacks
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

    // FIXME: is this a a-star problem ?
#ifdef NEW_ORDERS
    if( unit->Orders[0].Action==UnitActionAttackGround
	    || WallOnMap(unit->Orders[0].X,unit->Orders[0].Y) ) {
	// FIXME: workaround for pathfinder problem
	unit->Orders[0].X-=unit->Orders[0].RangeX;
	unit->Orders[0].Y-=unit->Orders[0].RangeY;
	unit->Orders[0].RangeX*=2;
	unit->Orders[0].RangeY*=2;
	wall=unit->Orders[0].Action;
	err=HandleActionMove(unit);
	unit->Orders[0].RangeX/=2;
	unit->Orders[0].RangeY/=2;
	unit->Orders[0].X+=unit->Orders[0].RangeX;
	unit->Orders[0].Y+=unit->Orders[0].RangeY;
	unit->Orders[0].Action=wall;
    } else {
	err=HandleActionMove(unit);
    }
#else
    if( unit->Command.Action==UnitActionAttackGround
	|| WallOnMap(unit->Command.Data.Move.DX,unit->Command.Data.Move.DY) ) {
	// FIXME: workaround for pathfinder problem
	unit->Command.Data.Move.DX-=unit->Command.Data.Move.Range;
	unit->Command.Data.Move.DY-=unit->Command.Data.Move.Range;
	unit->Command.Data.Move.Range*=2;
	wall=unit->Command.Action;
	err=HandleActionMove(unit);
	unit->Command.Data.Move.Range/=2;
	unit->Command.Data.Move.DX+=unit->Command.Data.Move.Range;
	unit->Command.Data.Move.DY+=unit->Command.Data.Move.Range;
	unit->Command.Action=wall;
    } else {
	err=HandleActionMove(unit);
    }
#endif

    // NEW return codes supported, FIXME: but I think not perfect.

    if( unit->Reset ) {
	//
	//	Target is dead, choose new one.
	//
#ifdef NEW_ORDERS
	if( (goal=unit->Orders[0].Goal) ) {
	    // FIXME: Should be done by Action Move???????
	    if( goal->Destroyed ) {
		DebugLevel0Fn("destroyed unit\n");
		RefsDebugCheck( !goal->Refs );
		if( !--goal->Refs ) {
		    ReleaseUnit(goal);
		}
		unit->Orders[0].X=goal->X;
		unit->Orders[0].Y=goal->Y;
		unit->Orders[0].Goal=goal=NoUnitP;
		ResetPath(unit->Orders[0]);
	    } else if( !goal->HP || goal->Orders[0].Action==UnitActionDie ) {
		RefsDebugCheck( !goal->Refs );
		--goal->Refs;
		RefsDebugCheck( !goal->Refs );
		unit->Orders[0].X=goal->X;
		unit->Orders[0].Y=goal->Y;
		unit->Orders[0].Goal=goal=NoUnitP;
		ResetPath(unit->Orders[0]);
	    }
	}
#else
	if( (goal=unit->Command.Data.Move.Goal) ) {
	    // FIXME: Should be done by Action Move???????
	    if( goal->Destroyed ) {
		DebugLevel0Fn("destroyed unit\n");
		RefsDebugCheck( !goal->Refs );
		if( !--goal->Refs ) {
		    ReleaseUnit(goal);
		}
		unit->Command.Data.Move.DX=goal->X;
		unit->Command.Data.Move.DY=goal->Y;
		unit->Command.Data.Move.Goal=goal=NoUnitP;
		ResetPath(unit->Command);
	    } else if( !goal->HP || goal->Command.Action==UnitActionDie ) {
		RefsDebugCheck( !goal->Refs );
		--goal->Refs;
		RefsDebugCheck( !goal->Refs );
		unit->Command.Data.Move.DX=goal->X;
		unit->Command.Data.Move.DY=goal->Y;
		unit->Command.Data.Move.Goal=goal=NoUnitP;
		ResetPath(unit->Command);
	    }
	}
#endif

	//
	//	No goal: if meeting enemy attack it.
	//
	wall=0;
#ifdef NEW_ORDERS
	if( !goal && !(wall=WallOnMap(unit->Orders[0].X,unit->Orders[0].Y))
		&& unit->Orders[0].Action!=UnitActionAttackGround ) {
	    goal=AttackUnitsInReactRange(unit);
	    if( goal ) {
		goal->Refs++;
		if( unit->SavedOrder.Action==UnitActionStill ) {
		    // Save current command to come back.
		    unit->SavedOrder=unit->Orders[0];
		}
		unit->Orders[0].Goal=goal;
		ResetPath(unit->Orders[0]);
		unit->Orders[0].X=-1;
		unit->Orders[0].Y=-1;
		unit->SubAction|=2;		// weak target
		DebugLevel3Fn("%Zd in react range %Zd\n"
			,UnitNumber(unit),UnitNumber(goal));
	    }
#else
	if( !goal && !(wall=WallOnMap(unit->Command.Data.Move.DX
		    ,unit->Command.Data.Move.DY))
		&& unit->Command.Action!=UnitActionAttackGround ) {
	    goal=AttackUnitsInReactRange(unit);
	    if( goal ) {
		goal->Refs++;
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
#endif

	//
	//	Have a weak target, try a better target.
	//
	} else if( goal && (unit->SubAction&2) ) {
	    Unit* temp;

	    temp=AttackUnitsInReactRange(unit);
	    if( temp && temp->Type->Priority>goal->Type->Priority ) {
		RefsDebugCheck( !goal->Refs );
		goal->Refs--;
		RefsDebugCheck( !goal->Refs );
		RefsDebugCheck( !temp->Refs );
		temp->Refs++;
#ifdef NEW_ORDERS
		if( unit->SavedOrder.Action==UnitActionStill ) {
		    // Save current command to come back.
		    unit->SavedOrder=unit->Orders[0];
		}
		unit->Orders[0].Goal=goal=temp;
		ResetPath(unit->Orders[0]);
		unit->Orders[0].X=goal->X;
		unit->Orders[0].Y=goal->Y;
#else
		if( unit->SavedCommand.Action==UnitActionStill ) {
		    // Save current command to come back.
		    unit->SavedCommand=unit->Command;
		}
		unit->Command.Data.Move.Goal=goal=temp;
		ResetPath(unit->Command);
		unit->Command.Data.Move.DX=goal->X;
		unit->Command.Data.Move.DY=goal->Y;
#endif
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
#ifdef NEW_ORDERS
	} else if( (wall || unit->Orders[0].Action==UnitActionAttackGround)
		&& MapDistance(unit->X,unit->Y
		    ,unit->Orders[0].X,unit->Orders[0].Y)
			<=unit->Stats->AttackRange ) {
	    DebugLevel3Fn("Attacking wall or ground\n");
	    unit->State=0;
	    if( !unit->Type->Tower ) {
		UnitHeadingFromDeltaXY(unit,unit->Orders[0].X-unit->X
		    ,unit->Orders[0].Y-unit->Y);
	    }
	    unit->SubAction=1;
	    return;
	} else if( err<0 ) {
#else
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
#endif
	    unit->State=0;
	    unit->SubAction=0;
	    // Return to old task?
#ifdef NEW_ORDERS
	    if( unit->Orders[0].Action==UnitActionStill ) {
		unit->Orders[0]=unit->SavedOrder;
		ResetPath(unit->Orders[0]);
		// Must finish, if saved command finishes
		unit->SavedOrder.Action=UnitActionStill;
	    }
#else
	    if( unit->Command.Action==UnitActionStill ) {
		unit->Command=unit->SavedCommand;
		ResetPath(unit->Command);
		// Must finish if saved command finishes
		unit->SavedCommand.Action=UnitActionStill;
	    }
#endif
	    return;
	}
	DebugCheck( unit->Type->Vanishes || unit->Destroyed );
#ifdef NEW_ORDERS
	unit->Orders[0].Action=UnitActionAttack;
#else
	unit->Command.Action=UnitActionAttack;
#endif
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
	//
	//	Goal is "weak" or a wall.
	//
#ifdef NEW_ORDERS
	goal=unit->Orders[0].Goal;
	if( !goal && (WallOnMap(unit->Orders[0].X,unit->Orders[0].Y)
		|| unit->Orders[0].Action==UnitActionAttackGround) ) {
	    DebugLevel3Fn("attack a wall!!!!\n");
	    return;
	}
#else
	goal=unit->Command.Data.Move.Goal;
	if( !goal && (WallOnMap(unit->Command.Data.Move.DX
		     ,unit->Command.Data.Move.DY)
		|| unit->Command.Action==UnitActionAttackGround) ) {
	    DebugLevel3Fn("attack a wall!!!!\n");
	    return;
	}
#endif

	//
	//	Target is dead, choose new one.
	//
	if( goal ) {
	    if( goal->Destroyed ) {
		DebugLevel0Fn("destroyed unit\n");
		RefsDebugCheck( !goal->Refs );
		if( !--goal->Refs ) {
		    ReleaseUnit(goal);
		}
#ifdef NEW_ORDERS
		unit->Orders[0].X=goal->X;
		unit->Orders[0].Y=goal->Y;
		unit->Orders[0].Goal=goal=NoUnitP;
		ResetPath(unit->Orders[0]);
	    } else if( !goal->HP || goal->Orders[0].Action==UnitActionDie ) {
		// FIXME: goal->Removed???
		RefsDebugCheck( !goal->Refs );
		--goal->Refs;
		RefsDebugCheck( !goal->Refs );
		unit->Orders[0].X=goal->X;
		unit->Orders[0].Y=goal->Y;
		unit->Orders[0].Goal=goal=NoUnitP;
		ResetPath(unit->Orders[0]);
#else
		unit->Command.Data.Move.DX=goal->X;
		unit->Command.Data.Move.DY=goal->Y;
		unit->Command.Data.Move.Goal=goal=NoUnitP;
		ResetPath(unit->Command);
	    } else if( !goal->HP || goal->Command.Action==UnitActionDie ) {
		// FIXME: goal->Removed???
		RefsDebugCheck( !goal->Refs );
		--goal->Refs;
		RefsDebugCheck( !goal->Refs );
		unit->Command.Data.Move.DX=goal->X;
		unit->Command.Data.Move.DY=goal->Y;
		unit->Command.Data.Move.Goal=goal=NoUnitP;
		ResetPath(unit->Command);
#endif
	    }
	}

	//
	//	No target choose one.
	//
	if( !goal ) {
	    unit->State=0;
	    goal=AttackUnitsInReactRange(unit);
#ifdef NEW_ORDERS
	    if( !goal ) {
		unit->SubAction=0;
		// Return to old task!
		unit->Orders[0]=unit->SavedOrder;
		// Must finish, if saved command finishes
		unit->SavedOrder.Action=UnitActionStill;
		ResetPath(unit->Orders[0]);
		return;
	    }
	    if( unit->SavedOrder.Action==UnitActionStill ) {
		// Save current command to come back.
		unit->SavedOrder=unit->Orders[0];
	    }
#else
	    if( !goal ) {
		unit->SubAction=0;
		// Return to old task!
		unit->Command=unit->SavedCommand;
		// Must finish, if saved command finishes
		unit->SavedCommand.Action=UnitActionStill;
		ResetPath(unit->Command);
		return;
	    }
	    if( unit->SavedCommand.Action==UnitActionStill ) {
		// Save current command to come back.
		unit->SavedCommand=unit->Command;
	    }
#endif
	    goal->Refs++;
	    DebugLevel3Fn("%Zd Unit in react range %Zd\n"
		    ,UnitNumber(unit),UnitNumber(goal));
#ifdef NEW_ORDERS
	    unit->Orders[0].Goal=goal;
	    unit->Orders[0].X=-1;
	    unit->Orders[0].Y=-1;
#else
	    unit->Command.Data.Move.Goal=goal;
	    unit->Command.Data.Move.DX=goal->X;
	    unit->Command.Data.Move.DY=goal->Y;
#endif
	    unit->SubAction|=2;
	} else

	//
	//	Have a weak target, try a better target.
	//
	if( goal && (unit->SubAction&2) ) {
	    Unit* temp;

	    temp=AttackUnitsInReactRange(unit);
	    if( temp && temp->Type->Priority>goal->Type->Priority ) {
		RefsDebugCheck( !goal->Refs );
		goal->Refs--;
		RefsDebugCheck( !goal->Refs );
		temp->Refs++;
#ifdef NEW_ORDERS
		if( unit->SavedOrder.Action==UnitActionStill ) {
		    // Save current order to come back or to continue it.
		    unit->SavedOrder=unit->Orders[0];
		}
		unit->Orders[0].Goal=goal=temp;
		ResetPath(unit->Orders[0]);
#else
		if( unit->SavedCommand.Action==UnitActionStill ) {
		    // Save current command to come back.
		    unit->SavedCommand=unit->Command;
		}
		unit->Command.Data.Move.Goal=goal=temp;
		unit->Command.Data.Move.DX=goal->X;
		unit->Command.Data.Move.DY=goal->Y;
		ResetPath(unit->Command);
#endif
	    }
	}

	//
	//	Still near to target, if not goto target.
	//
	if( MapDistanceToUnit(unit->X,unit->Y,goal)
		>unit->Stats->AttackRange ) {
#ifdef NEW_ORDERS
	    if( unit->SavedOrder.Action==UnitActionStill ) {
		// Save current order to come back or to continue it.
		unit->SavedOrder=unit->Orders[0];
	    }
	    ResetPath(unit->Orders[0]);
	    unit->Orders[0].X=goal->X;
	    unit->Orders[0].Y=goal->Y;
#else
	    if( unit->SavedCommand.Action==UnitActionStill ) {
		// Save current command to come back.
		unit->SavedCommand=unit->Command;
	    }
	    ResetPath(unit->Command);
	    unit->Command.Data.Move.DX=goal->X;
	    unit->Command.Data.Move.DY=goal->Y;
#endif
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
