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
//	FreeCraft is free software; you can redistribute it and/or modify
//	it under the terms of the GNU General Public License as published
//	by the Free Software Foundation; either version 2 of the License,
//	or (at your option) any later version.
//
//	FreeCraft is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//	GNU General Public License for more details.
//
//	$Id$

//@{

/**
**	@todo FIXME:	I should rewrite this action, if only the
**			new orders are supported.
*/

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
#define MOVE_TO_TARGET	4		/// Move to target state
#define ATTACK_TARGET	5		/// Attack target state

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
**	Check for dead goal.
**
**	@warning
**		The caller must check, if he likes the restored SavedOrder!
**
**	@todo
**		If an unit enters an building, than the attack choose an
**		other goal, perhaps it is better to wait for the goal?
**
**	@param unit	Unit using the goal.
**
**	@return		A valid goal, if available.
*/
local Unit* CheckForDeadGoal(Unit* unit)
{
    Unit* goal;

    //
    //	Do we have a goal?
    //
    if( (goal=unit->Orders[0].Goal) ) {
	if( goal->Destroyed ) {
	    //
	    //	Goal is destroyed
	    //
	    unit->Orders[0].X=goal->X+goal->Type->TileWidth/2;
	    unit->Orders[0].Y=goal->Y+goal->Type->TileHeight/2;
	    unit->Orders[0].RangeX=unit->Orders[0].RangeY=0;

	    DebugLevel0Fn("destroyed unit %d\n",UnitNumber(goal));
	    RefsDebugCheck( !goal->Refs );
	    if( !--goal->Refs ) {
		ReleaseUnit(goal);
	    }

	    unit->Orders[0].Goal=goal=NoUnitP;
	} else if( !goal->HP || goal->Orders[0].Action==UnitActionDie
		|| goal->Removed ) {
	    //
	    //	Goal is unusable, dies or has entered a building.
	    //
	    unit->Orders[0].X=goal->X+goal->Type->TileWidth/2;
	    unit->Orders[0].Y=goal->Y+goal->Type->TileHeight/2;
	    unit->Orders[0].RangeX=unit->Orders[0].RangeY=0;

	    RefsDebugCheck( !goal->Refs );
	    --goal->Refs;
	    RefsDebugCheck( !goal->Refs );

	    unit->Orders[0].Goal=goal=NoUnitP;
	}
	if( !goal ) {
	    //
	    //	If we have a saved order continue this saved order.
	    //
	    if( unit->SavedOrder.Action!=UnitActionStill ) {
		unit->Orders[0]=unit->SavedOrder;
		unit->SavedOrder.Action=UnitActionStill;

		// This is not supported
		DebugCheck( unit->SavedOrder.Goal!=NoUnitP );

		if( unit->Selected && unit->Player==ThisPlayer ) {
		    MustRedraw|=RedrawButtonPanel;
		}
		
	    }
	    NewResetPath(unit);
	}
    }
    return goal;
}

/**
**	Handle moving to the target.
**
**	@param unit	Unit, for that the attack is handled.
*/
local void MoveToTarget(Unit* unit)
{
    Unit* goal;
    Unit* temp;
    int wall;
    int err;

    if( !unit->Orders[0].Goal ) {
	if( unit->Orders[0].X==-1 || unit->Orders[0].Y==-1 ) {
	    DebugLevel0Fn("FIXME: Wrong goal position, check where set!\n");
	    unit->Orders[0].X=unit->Orders[0].Y=0;
	}
    }

    if( unit->Orders[0].Action==UnitActionAttackGround
	    || (!unit->Orders[0].Goal &&
		WallOnMap(unit->Orders[0].X,unit->Orders[0].Y)) ) {
	// FIXME: workaround for pathfinder problem
	DebugLevel3Fn("FIXME: Johns remove this for new orders.\n");
	unit->Orders[0].X-=unit->Orders[0].RangeX;
	unit->Orders[0].Y-=unit->Orders[0].RangeY;
	unit->Orders[0].RangeX*=2;
	unit->Orders[0].RangeY*=2;
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
#ifdef DEBUG
	// This fixes the bug: if wall is gone, debug code fails.
	unit->Goal=unit->Orders[0].Goal;
	unit->GoalX=unit->Orders[0].X;
	unit->GoalY=unit->Orders[0].Y;
#endif
    } else {
	err=DoActionMove(unit);
    }

    // NEW return codes supported, FIXME: but johns thinks not perfect.

    if( unit->Reset ) {
	//
	//	Target is dead?
	//
	goal=CheckForDeadGoal(unit);
	// Fall back to last order.
	if( unit->Orders[0].Action!=UnitActionAttackGround
		&& unit->Orders[0].Action!=UnitActionAttack ) {
	    unit->State=unit->SubAction=0;
	    return;
	}

	//
	//	No goal: if meeting enemy attack it.
	//
	wall=0;
	if( !goal && !(wall=WallOnMap(unit->Orders[0].X,unit->Orders[0].Y))
		&& unit->Orders[0].Action!=UnitActionAttackGround ) {
	    goal=AttackUnitsInReactRange(unit);
	    if( goal ) {
		if( unit->SavedOrder.Action==UnitActionStill ) {
		    // Save current command to continue it later.
		    DebugCheck( unit->Orders[0].Goal );
		    unit->SavedOrder=unit->Orders[0];
		}
		RefsDebugCheck( goal->Destroyed || !goal->Refs );
		goal->Refs++;
		unit->Orders[0].Goal=goal;
		unit->Orders[0].RangeX=unit->Orders[0].RangeY=
			unit->Stats->AttackRange;
		unit->Orders[0].X=unit->Orders[0].Y=-1;
		unit->SubAction|=WEAK_TARGET;		// weak target
		NewResetPath(unit);
		DebugLevel3Fn("%d in react range %d\n"
			,UnitNumber(unit),UnitNumber(goal));
	    }

	//
	//	Have a weak target, try a better target.
	//
	} else if( goal && (unit->SubAction&WEAK_TARGET) ) {
	    temp=AttackUnitsInReactRange(unit);
	    if( temp && temp->Type->Priority>goal->Type->Priority ) {
		RefsDebugCheck( !goal->Refs );
		goal->Refs--;
		RefsDebugCheck( !goal->Refs );
		RefsDebugCheck( temp->Destroyed || !temp->Refs );
		temp->Refs++;
		if( unit->SavedOrder.Action==UnitActionStill ) {
		    // Save current command to come back.
		    unit->SavedOrder=unit->Orders[0];
		    if( (goal=unit->SavedOrder.Goal) ) {
			DebugLevel0Fn("Have goal to come back %d\n",
				UnitNumber(goal));
			unit->SavedOrder.X=goal->X+goal->Type->TileWidth/2;
			unit->SavedOrder.Y=goal->Y+goal->Type->TileHeight/2;
			unit->SavedOrder.RangeX=unit->SavedOrder.RangeY=0;
			unit->SavedOrder.Goal=NoUnitP;
		    }
		}
		unit->Orders[0].Goal=goal=temp;
		unit->Orders[0].X=unit->Orders[0].Y=-1;
		NewResetPath(unit);
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
		// FIXME: only if heading changes
		CheckUnitToBeDrawn(unit);
	    }
	    unit->SubAction++;
	} else if( (wall || unit->Orders[0].Action==UnitActionAttackGround)
		&& MapDistance(unit->X,unit->Y
		    ,unit->Orders[0].X,unit->Orders[0].Y)
			<=unit->Stats->AttackRange ) {
	    DebugLevel2Fn("Attacking wall or ground\n");
	    unit->State=0;
	    if( !unit->Type->Tower ) {
		UnitHeadingFromDeltaXY(unit,unit->Orders[0].X-unit->X
		    ,unit->Orders[0].Y-unit->Y);
		// FIXME: only if heading changes
		CheckUnitToBeDrawn(unit);
	    }
	    unit->SubAction&=WEAK_TARGET;
	    unit->SubAction|=ATTACK_TARGET;
	    return;
	} else if( err<0 ) {
	    unit->State=unit->SubAction=0;
	    // Return to old task?
	    if( err==-2 ) {
		DebugLevel0Fn("Target not reachable\n");
	    }
	    if( unit->Orders[0].Goal ) {
		RefsDebugCheck( !unit->Orders[0].Goal->Refs );
		unit->Orders[0].Goal->Refs--;
		RefsDebugCheck( !unit->Orders[0].Goal->Refs );
	    }
	    unit->Orders[0]=unit->SavedOrder;
	    NewResetPath(unit);

	    // Must finish, if saved command finishes
	    unit->SavedOrder.Action=UnitActionStill;
	    DebugCheck( unit->SavedOrder.Goal!=NoUnitP );
	    if( unit->Selected && unit->Player==ThisPlayer ) {
		MustRedraw|=RedrawButtonPanel;
	    }
	    return;
	}
	DebugCheck( unit->Type->Vanishes || unit->Destroyed || unit->Removed );
	DebugCheck( unit->Orders[0].Action!=UnitActionAttack 
		&& unit->Orders[0].Action!=UnitActionAttackGround );
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
    Unit* temp;

    if( !unit->Orders[0].Goal ) {
	if( unit->Orders[0].X==-1 || unit->Orders[0].Y==-1 ) {
	    DebugLevel0Fn("FIXME: Wrong goal position, check where set!\n");
	    unit->Orders[0].X=unit->Orders[0].Y=0;
	}
    }

    AnimateActionAttack(unit);
    if( unit->Reset ) {
	//
	//	Goal is "weak" or a wall.
	//
	goal=unit->Orders[0].Goal;
	if( !goal && (WallOnMap(unit->Orders[0].X,unit->Orders[0].Y)
		|| unit->Orders[0].Action==UnitActionAttackGround) ) {
	    DebugLevel3Fn("attack a wall or ground!!!!\n");
	    return;
	}

	//
	//	Target is dead?
	//
	goal=CheckForDeadGoal(unit);
	// Fall back to last order.
	if( unit->Orders[0].Action!=UnitActionAttackGround
		&& unit->Orders[0].Action!=UnitActionAttack ) {
	    unit->State=unit->SubAction=0;
	    return;
	}

	//
	//	No target choose one.
	//
	if( !goal ) {
	    unit->State=0;
	    goal=AttackUnitsInReactRange(unit);
	    //
	    //	No new goal, continue way to destination.
	    //
	    if( !goal ) {
		unit->SubAction=MOVE_TO_TARGET;
		// Return to old task?
		if( unit->SavedOrder.Action!=UnitActionStill ) {
		    unit->SubAction=0;
		    DebugCheck( unit->Orders[0].Goal!=NoUnitP );
		    unit->Orders[0]=unit->SavedOrder;
		    NewResetPath(unit);
		    // Must finish, if saved command finishes
		    unit->SavedOrder.Action=UnitActionStill;

		    // This isn't supported
		    DebugCheck( unit->SavedOrder.Goal!=NoUnitP );

		    if( unit->Selected && unit->Player==ThisPlayer ) {
			MustRedraw|=RedrawButtonPanel;
		    }
		}
		return;
	    }

	    //
	    //	Save current command to come back.
	    //
	    if( unit->SavedOrder.Action==UnitActionStill ) {
		unit->SavedOrder=unit->Orders[0];
		if( (temp=unit->SavedOrder.Goal) ) {
		    DebugLevel0Fn("Have unit to come back %d?\n",
			    UnitNumber(temp));
		    unit->SavedOrder.X=temp->X+temp->Type->TileWidth/2;
		    unit->SavedOrder.Y=temp->Y+temp->Type->TileHeight/2;
		    unit->SavedOrder.RangeX=unit->SavedOrder.RangeY=0;
		    unit->SavedOrder.Goal=NoUnitP;
		}
	    }

	    RefsDebugCheck( goal->Destroyed || !goal->Refs );
	    goal->Refs++;
	    DebugLevel3Fn("%d Unit in react range %d\n"
		    ,UnitNumber(unit),UnitNumber(goal));
	    unit->Orders[0].Goal=goal;
	    unit->Orders[0].X=unit->Orders[0].Y=-1;
	    unit->Orders[0].RangeX=unit->Orders[0].RangeY=
		    unit->Stats->AttackRange;
	    NewResetPath(unit);
	    unit->SubAction|=WEAK_TARGET;

	//
	//	Have a weak target, try a better target.
	//
	} else if( goal && (unit->SubAction&WEAK_TARGET) ) {
	    temp=AttackUnitsInReactRange(unit);
	    if( temp && temp->Type->Priority>goal->Type->Priority ) {
		RefsDebugCheck( !goal->Refs );
		goal->Refs--;
		RefsDebugCheck( !goal->Refs );
		RefsDebugCheck( temp->Destroyed || !temp->Refs );
		temp->Refs++;

		if( unit->SavedOrder.Action==UnitActionStill ) {
		    // Save current order to come back or to continue it.
		    unit->SavedOrder=unit->Orders[0];
		    if( (goal=unit->SavedOrder.Goal) ) {
			DebugLevel0Fn("Have goal to come back %d\n",
				UnitNumber(goal));
			unit->SavedOrder.X=goal->X+goal->Type->TileWidth/2;
			unit->SavedOrder.Y=goal->Y+goal->Type->TileHeight/2;
			unit->SavedOrder.RangeX=unit->SavedOrder.RangeY=0;
			unit->SavedOrder.Goal=NoUnitP;
		    }
		}
		unit->Orders[0].Goal=goal=temp;
		unit->Orders[0].X=unit->Orders[0].Y=-1;
		NewResetPath(unit);
	    }
	}

	//
	//	Still near to target, if not goto target.
	//
	if( MapDistanceToUnit(unit->X,unit->Y,goal)
		>unit->Stats->AttackRange ) {
	    if( unit->SavedOrder.Action==UnitActionStill ) {
		// Save current order to come back or to continue it.
		unit->SavedOrder=unit->Orders[0];
		if( (temp=unit->SavedOrder.Goal) ) {
		    DebugLevel0Fn("Have goal to come back %d\n",
			    UnitNumber(temp));
		    unit->SavedOrder.X=temp->X+temp->Type->TileWidth/2;
		    unit->SavedOrder.Y=temp->Y+temp->Type->TileHeight/2;
		    unit->SavedOrder.RangeX=unit->SavedOrder.RangeY=0;
		    unit->SavedOrder.Goal=NoUnitP;
		}
	    }
	    NewResetPath(unit);
	    unit->Frame=0;
	    unit->State=0;
	    unit->SubAction&=WEAK_TARGET;
	    unit->SubAction|=MOVE_TO_TARGET;
	}

	//
	//	Turn always to target
	//
	if( !unit->Type->Tower && goal ) {
	    UnitHeadingFromDeltaXY(unit,goal->X-unit->X,goal->Y-unit->Y);
	    // FIXME: only if heading changes
	    CheckUnitToBeDrawn(unit);
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
**	@todo
**		Lets do some tries to reach the target.
**		If target place is not reachable, choose better goal to reduce
**		the pathfinder load.
**
**	@param unit	Unit, for that the attack is handled.
*/
global void HandleActionAttack(Unit* unit)
{
    DebugLevel3Fn("Attack %d\n",UnitNumber(unit));

    switch( unit->SubAction ) {
	//
	//	First entry
	//
	case 0:
	    unit->SubAction=MOVE_TO_TARGET;
	    NewResetPath(unit);
	    //
	    //	FIXME: should use a reachable place to reduce pathfinder time.
	    //
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

	case WEAK_TARGET:
	    DebugLevel0("FIXME: wrong entry.\n");
	    break;
    }
}

//@}
