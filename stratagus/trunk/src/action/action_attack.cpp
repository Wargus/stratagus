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
--	Defines
----------------------------------------------------------------------------*/

#define WEAK_TARGET	2		/// Weak target, could be changed


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

    flags=UnitShowAnimation(unit,attack);

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
global void AnimateActionAttack(Unit* unit)
{
    if( unit->Type->Animations ) {
	DebugCheck( !unit->Type->Animations->Attack );
	DoActionAttackGeneric(unit,unit->Type->Animations->Attack);
    }
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

#ifdef NEW_ORDERS
    if( unit->Orders[0].Action==UnitActionAttackGround
	    || (!unit->Orders[0].Goal &&
		WallOnMap(unit->Orders[0].X,unit->Orders[0].Y)) ) {
	// FIXME: workaround for pathfinder problem
	DebugLevel0Fn("Johns remove this for new orders.\n");
	unit->Orders[0].X-=unit->Orders[0].RangeX;
	unit->Orders[0].Y-=unit->Orders[0].RangeY;
	unit->Orders[0].RangeX*=2;
	unit->Orders[0].RangeY*=2;
	wall=unit->Orders[0].Action;
#ifdef DEBUG
	// This fixes the bug: if wall is gone, debug code fails.
	unit->Goal=unit->Orders[0].Goal;
	unit->GoalX=unit->Orders[0].X;
	unit->GoalY=unit->Orders[0].Y;
#endif
	err=DoActionMove(unit);
	unit->Orders[0].RangeX/=2;
	unit->Orders[0].RangeY/=2;
	unit->Orders[0].X+=unit->Orders[0].RangeX;
	unit->Orders[0].Y+=unit->Orders[0].RangeY;
	unit->Orders[0].Action=wall;
#ifdef DEBUG
	// This fixes the bug: if wall is gone, debug code fails.
	unit->Goal=unit->Orders[0].Goal;
	unit->GoalX=unit->Orders[0].X;
	unit->GoalY=unit->Orders[0].Y;
#endif
    } else {
	err=DoActionMove(unit);
    }
#else
    if( unit->Command.Action==UnitActionAttackGround
	|| (!unit->Command.Data.Move.Goal
	    && WallOnMap(unit->Command.Data.Move.DX
		,unit->Command.Data.Move.DY)) ) {
	// FIXME: workaround for pathfinder problem
	unit->Command.Data.Move.DX-=unit->Command.Data.Move.Range;
	unit->Command.Data.Move.DY-=unit->Command.Data.Move.Range;
	unit->Command.Data.Move.Range*=2;
	wall=unit->Command.Action;
#ifdef DEBUG
	// This fixes the bug: if wall is gone, debug code fails.
	unit->Goal=unit->Command.Data.Move.Goal;
	unit->GoalX=unit->Command.Data.Move.DX;
	unit->GoalY=unit->Command.Data.Move.DY;
#endif
	err=DoActionMove(unit);
	unit->Command.Data.Move.Range/=2;
	unit->Command.Data.Move.DX+=unit->Command.Data.Move.Range;
	unit->Command.Data.Move.DY+=unit->Command.Data.Move.Range;
	unit->Command.Action=wall;
#ifdef DEBUG
	// This fixes the bug: if wall is gone, debug code fails.
	unit->Goal=unit->Command.Data.Move.Goal;
	unit->GoalX=unit->Command.Data.Move.DX;
	unit->GoalY=unit->Command.Data.Move.DY;
#endif
    } else {
	err=DoActionMove(unit);
    }
#endif

    // NEW return codes supported, FIXME: but johns thinks not perfect.

    if( unit->Reset ) {
	//
	//	Target is dead, choose new one.
	//
	// FIXME: should I make a general function for this?
#ifdef NEW_ORDERS
	if( (goal=unit->Orders[0].Goal) ) {
	    //
	    //	Could be handled here better. (not in GenericMove)
	    //
	    //	If we have a saved order, continue with it.
	    //
	    if( goal->Destroyed ) {
		unit->Orders[0].X=goal->X+goal->Type->TileWidth/2;
		unit->Orders[0].Y=goal->Y+goal->Type->TileHeight/2;
		DebugLevel0Fn("destroyed unit %Zd\n",UnitNumber(goal));
		RefsDebugCheck( !goal->Refs );
		if( !--goal->Refs ) {
		    ReleaseUnit(goal);
		}
		unit->Orders[0].Goal=goal=NoUnitP;
		if( unit->SavedOrder.Action!=UnitActionStill ) {
		    unit->Orders[0]=unit->SavedOrder;
		    unit->SavedOrder.Action=UnitActionStill;
		    DebugCheck( unit->SavedOrder.Goal!=NoUnitP );
		    // This is not supported
		}
		NewResetPath(unit);
	    } else if( !goal->HP
		    || goal->Orders[0].Action==UnitActionDie
		    || goal->Removed ) {
		RefsDebugCheck( !goal->Refs );
		--goal->Refs;
		RefsDebugCheck( !goal->Refs );
		unit->Orders[0].X=goal->X+goal->Type->TileWidth/2;
		unit->Orders[0].Y=goal->Y+goal->Type->TileHeight/2;
		unit->Orders[0].Goal=goal=NoUnitP;
		if( unit->SavedOrder.Action!=UnitActionStill ) {
		    unit->Orders[0]=unit->SavedOrder;
		    unit->SavedOrder.Action=UnitActionStill;
		    DebugCheck( unit->SavedOrder.Goal!=NoUnitP );
		    // This is not supported
		}
		NewResetPath(unit);
	    }
	}
#else
	if( (goal=unit->Command.Data.Move.Goal) ) {
	    // FIXME: Should be done by Action Move???????
	    if( goal->Destroyed ) {
		DebugLevel0Fn("destroyed unit %Zd\n",UnitNumber(goal));
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
	    } else if( goal->Removed ) {
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
		if( unit->SavedOrder.Action==UnitActionStill ) {
		    // Save current command to come back.
		    unit->SavedOrder=unit->Orders[0];
		}
		RefsDebugCheck( goal->Destroyed || !goal->Refs );
		goal->Refs++;
		unit->Orders[0].Goal=goal;
		unit->Orders[0].X=-1;
		unit->Orders[0].Y=-1;
		unit->SubAction|=WEAK_TARGET;		// weak target
		NewResetPath(unit);
		DebugLevel3Fn("%Zd in react range %Zd\n"
			,UnitNumber(unit),UnitNumber(goal));
	    }
#else
	if( !goal && !(wall=WallOnMap(unit->Command.Data.Move.DX
		    ,unit->Command.Data.Move.DY))
		&& unit->Command.Action!=UnitActionAttackGround ) {
	    goal=AttackUnitsInReactRange(unit);
	    if( goal ) {
		RefsDebugCheck( goal->Destroyed || !goal->Refs );
		goal->Refs++;
		if( unit->SavedCommand.Action==UnitActionStill ) {
		    // Save current command to come back.
		    unit->SavedCommand=unit->Command;
		}
		unit->Command.Data.Move.Goal=goal;
		ResetPath(unit->Command);
		unit->SubAction|=WEAK_TARGET;		// weak target
		DebugLevel3Fn("%Zd in react range %Zd\n"
			,UnitNumber(unit),UnitNumber(goal));
	    }
#endif

	//
	//	Have a weak target, try a better target.
	//
	} else if( goal && (unit->SubAction&WEAK_TARGET) ) {
	    Unit* temp;

	    temp=AttackUnitsInReactRange(unit);
	    if( temp && temp->Type->Priority>goal->Type->Priority ) {
		RefsDebugCheck( !goal->Refs );
		goal->Refs--;
		RefsDebugCheck( !goal->Refs );
		RefsDebugCheck( temp->Destroyed || !temp->Refs );
		temp->Refs++;
#ifdef NEW_ORDERS
		if( unit->SavedOrder.Action==UnitActionStill ) {
		    // Save current command to come back.
		    unit->SavedOrder=unit->Orders[0];
		}
		unit->Orders[0].Goal=goal=temp;
		unit->Orders[0].X=-1;
		unit->Orders[0].Y=-1;
		NewResetPath(unit);
#else
		if( unit->SavedCommand.Action==UnitActionStill ) {
		    // Save current command to come back.
		    unit->SavedCommand=unit->Command;
		}
		unit->Command.Data.Move.Goal=goal=temp;
		ResetPath(unit->Command);
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
		CheckUnitToBeDrawn(unit);
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
	    unit->SubAction=4;
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
	    unit->SubAction=4;
	    return;
	} else if( err<0 ) {
#endif
	    unit->State=unit->SubAction=0;
	    // Return to old task?
#ifdef NEW_ORDERS
	    if( unit->Orders[0].Action==UnitActionStill ) {
		unit->Orders[0]=unit->SavedOrder;
		NewResetPath(unit);
		// Must finish, if saved command finishes
		unit->SavedOrder.Action=UnitActionStill;
		DebugCheck( !unit->SavedOrder.Goal );
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
	DebugCheck( unit->Type->Vanishes || unit->Destroyed || unit->Removed );
#ifdef NEW_ORDERS
	DebugCheck( unit->Orders[0].Action!=UnitActionAttack 
		&& unit->Orders[0].Action!=UnitActionAttackGround );
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
#ifdef NEW_ORDERS
		DebugLevel0Fn("destroyed unit %Zd\n",UnitNumber(goal));
		unit->Orders[0].X=goal->X+goal->Type->TileWidth/2;
		unit->Orders[0].Y=goal->Y+goal->Type->TileHeight/2;
		RefsDebugCheck( !goal->Refs );
		if( !--goal->Refs ) {
		    ReleaseUnit(goal);
		}
		unit->Orders[0].Goal=goal=NoUnitP;
		if( unit->SavedOrder.Action!=UnitActionStill ) {
		    unit->Orders[0]=unit->SavedOrder;
		    unit->SavedOrder.Action=UnitActionStill;
		    DebugCheck( unit->SavedOrder.Goal!=NoUnitP );
		    // This is not supported
		}
		NewResetPath(unit);
	    } else if( !goal->HP || goal->Orders[0].Action==UnitActionDie
		    || goal->Removed ) {
		RefsDebugCheck( !goal->Refs );
		--goal->Refs;
		RefsDebugCheck( !goal->Refs );
		unit->Orders[0].X=goal->X+goal->Type->TileWidth/2;
		unit->Orders[0].Y=goal->Y+goal->Type->TileHeight/2;
		unit->Orders[0].Goal=goal=NoUnitP;
		if( unit->SavedOrder.Action!=UnitActionStill ) {
		    unit->Orders[0]=unit->SavedOrder;
		    unit->SavedOrder.Action=UnitActionStill;
		    DebugCheck( unit->SavedOrder.Goal!=NoUnitP );
		    // This is not supported
		}
		NewResetPath(unit);
	    }
#else
		DebugLevel0Fn("destroyed unit %Zd\n",UnitNumber(goal));
		RefsDebugCheck( !goal->Refs );
		if( !--goal->Refs ) {
		    ReleaseUnit(goal);
		}
		unit->Command.Data.Move.DX=goal->X;
		unit->Command.Data.Move.DY=goal->Y;
		unit->Command.Data.Move.Goal=goal=NoUnitP;
		ResetPath(unit->Command);
	    } else if( !goal->HP || goal->Command.Action==UnitActionDie
		    || goal->Removed ) {
		RefsDebugCheck( !goal->Refs );
		--goal->Refs;
		RefsDebugCheck( !goal->Refs );
		unit->Command.Data.Move.DX=goal->X;
		unit->Command.Data.Move.DY=goal->Y;
		unit->Command.Data.Move.Goal=goal=NoUnitP;
		ResetPath(unit->Command);
	    }
#endif
	}

	//
	//	No target choose one.
	//
	if( !goal ) {
	    unit->State=0;
	    goal=AttackUnitsInReactRange(unit);
#ifdef NEW_ORDERS
	    //
	    //	No new goal, continue way to destination.
	    //
	    if( !goal ) {
		unit->SubAction=4;
		// Return to old task!
		if( unit->SavedOrder.Action!=UnitActionStill ) {
		    unit->SubAction=0;
		    unit->Orders[0]=unit->SavedOrder;
		    NewResetPath(unit);
		    // Must finish, if saved command finishes
		    unit->SavedOrder.Action=UnitActionStill;
		    DebugCheck( !unit->SavedOrder.Goal );
		}
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
	    RefsDebugCheck( goal->Destroyed || !goal->Refs );
	    goal->Refs++;
	    DebugLevel3Fn("%Zd Unit in react range %Zd\n"
		    ,UnitNumber(unit),UnitNumber(goal));
#ifdef NEW_ORDERS
	    unit->Orders[0].Goal=goal;
	    unit->Orders[0].X=-1;
	    unit->Orders[0].Y=-1;
	    NewResetPath(unit);
#else
	    unit->Command.Data.Move.Goal=goal;
	    ResetPath(unit->Command);
#endif
	    unit->SubAction|=WEAK_TARGET;
	} else

	//
	//	Have a weak target, try a better target.
	//
	if( goal && (unit->SubAction&WEAK_TARGET) ) {
	    Unit* temp;

	    temp=AttackUnitsInReactRange(unit);
	    if( temp && temp->Type->Priority>goal->Type->Priority ) {
		RefsDebugCheck( !goal->Refs );
		goal->Refs--;
		RefsDebugCheck( !goal->Refs );
		RefsDebugCheck( temp->Destroyed || !temp->Refs );
		temp->Refs++;
#ifdef NEW_ORDERS
		if( unit->SavedOrder.Action==UnitActionStill ) {
		    // Save current order to come back or to continue it.
		    unit->SavedOrder=unit->Orders[0];
		}
		unit->Orders[0].Goal=goal=temp;
		unit->Orders[0].X=-1;
		unit->Orders[0].Y=-1;
		NewResetPath(unit);
#else
		if( unit->SavedCommand.Action==UnitActionStill ) {
		    // Save current command to come back.
		    unit->SavedCommand=unit->Command;
		}
		unit->Command.Data.Move.Goal=goal=temp;
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
	    NewResetPath(unit);
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
**	I added a little trick, if SubAction&WEAK_TARGET is true the goal is
**	a weak goal.  This means the unit AI (little AI) could choose a new
**	better goal.
**
**	@param unit	Unit, for that the attack is handled.
*/
global void HandleActionAttack(Unit* unit)
{
    DebugLevel3Fn("Attack %Zd\n",UnitNumber(unit));

    switch( unit->SubAction ) {
	//
	//	First entry
	//
	case 0:
	    unit->SubAction=4;
#ifdef NEW_ORDERS
	    NewResetPath(unit);
	    //
	    //	FIXME: should use a reachable place to reduce pathfinder time.
	    //
#endif
	    DebugCheck( unit->State!=0 );

	    // FALL THROUGH
	//
	//	Move near to the target.
	//
	case 4:
	case 4+WEAK_TARGET:
	    MoveToTarget(unit);
	    break;

	//
	//	Attack the target.
	//
	case 5:
	case 5+WEAK_TARGET:
	    AttackTarget(unit);
	    break;
    }
}

//@}
