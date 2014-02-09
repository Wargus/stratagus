//     ____                _       __               
//    / __ )____  _____   | |     / /___ ___________
//   / __  / __ \/ ___/   | | /| / / __ `/ ___/ ___/
//  / /_/ / /_/ (__  )    | |/ |/ / /_/ / /  (__  ) 
// /_____/\____/____/     |__/|__/\__,_/_/  /____/  
//                                              
//       A futuristic real-time strategy game.
//          This file is part of Bos Wars.
//
/**@name action_attack.cpp - The attack action. */
//
//      (c) Copyright 1998-2007 by Lutz Sammer and Jimmy Salmon
//
//      This program is free software; you can redistribute it and/or modify
//      it under the terms of the GNU General Public License as published by
//      the Free Software Foundation; only version 2 of the License.
//
//      This program is distributed in the hope that it will be useful,
//      but WITHOUT ANY WARRANTY; without even the implied warranty of
//      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//      GNU General Public License for more details.
//
//      You should have received a copy of the GNU General Public License
//      along with this program; if not, write to the Free Software
//      Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
//      02111-1307, USA.
//

//@{

/**
**  @todo FIXME: I should rewrite this action, if only the
**               new orders are supported.
*/

/*----------------------------------------------------------------------------
--  Includes
----------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "stratagus.h"
#include "unittype.h"
#include "animation.h"
#include "player.h"
#include "unit.h"
#include "missile.h"
#include "actions.h"
#include "sound.h"
#include "map.h"
#include "pathfinder.h"

/*----------------------------------------------------------------------------
--  Defines
----------------------------------------------------------------------------*/

#define WEAK_TARGET      2  /// Weak target, could be changed
#define MOVE_TO_TARGET   4  /// Move to target state
#define ATTACK_TARGET    5  /// Attack target state

/*----------------------------------------------------------------------------
--  Functions
----------------------------------------------------------------------------*/

/**
**  Animate unit attack!
**
**  @param unit  Unit, for that the attack animation is played.
**
**  @todo manage correctly unit with no animation attack.
*/
void AnimateActionAttack(CUnit *unit)
{
	//  No animation.
	//  So direct fire missile.
	//  FIXME : wait a little.
	if (!unit->Type->Animations || !unit->Type->Animations->Attack) {
		FireMissile(unit);
		return;
	}
	UnitShowAnimation(unit, unit->Type->Animations->Attack);
}

/**
**  Check for dead goal.
**
**  @warning  The caller must check, if he likes the restored SavedOrder!
**
**  @todo     If a unit enters an building, than the attack choose an
**            other goal, perhaps it is better to wait for the goal?
**
**  @param unit  Unit using the goal.
**
**  @return      1 if order have changed, 0 else.
*/
static int CheckForDeadGoal(CUnit *unit)
{
	CUnit *goal; // unit->Order[0].Goal

	goal = unit->Orders[0]->Goal;
	// Position or valid target, it is ok.
	if (!goal || goal->IsVisibleAsGoal(unit->Player)) {
		return 0;
	}
	// Goal could be destroyed or unseen
	// So, cannot use type.
	unit->Orders[0]->X = goal->X;
	unit->Orders[0]->Y = goal->Y;
	unit->Orders[0]->MinRange = 0;
	unit->Orders[0]->Range = 0;

	goal->RefsDecrease();
	unit->Orders[0]->Goal = NoUnitP;
	unit->SubAction &= ~WEAK_TARGET; // No target at all, so not weak

	//
	// If we have a saved order continue this saved order.
	//
	if (unit->SavedOrder.Action != UnitActionStill) {
		unit->ClearAction();
		*unit->Orders[0] = unit->SavedOrder;
		unit->SavedOrder.Action = UnitActionStill;
		unit->SavedOrder.Goal = NoUnitP;
		// Restart order state.
		unit->State = 0;
		NewResetPath(unit);
		return 1;
	}
//	NewResetPath(unit); // Should be useless.
	return 0;
}

/**
**  Change invalid target for new target in range.
**
**  @param unit  Unit to check if goal is in range
**
**  @return      1 if order(action) have changed, 0 else (if goal change retrun 0).
*/
static int CheckForTargetInRange(CUnit *unit)
{
	CUnit *goal;
	CUnit *temp;

	//
	// Target is dead?
	//
	if (CheckForDeadGoal(unit)) {
		return 1;
	}
	goal = unit->Orders[0]->Goal;

	//
	// No goal: if meeting enemy attack it.
	//
	if (!goal && unit->Orders[0]->Action != UnitActionAttackGround) {
		goal = AttackUnitsInReactRange(unit);
		if (goal) {
			if (unit->SavedOrder.Action == UnitActionStill) {
				// Save current command to continue it later.
				Assert(!unit->Orders[0]->Goal);
				unit->SavedOrder = *unit->Orders[0];
			}
			goal->RefsIncrease();
			unit->Orders[0]->Goal = goal;
			unit->Orders[0]->MinRange = unit->Type->MinAttackRange;
			unit->Orders[0]->Range = unit->Stats->Variables[ATTACKRANGE_INDEX].Max;
			unit->Orders[0]->X = unit->Orders[0]->Y = -1;
			unit->SubAction |= WEAK_TARGET; // weak target
			NewResetPath(unit);
		}

	//
	// Have a weak target, try a better target.
	//
	} else if (goal && (unit->SubAction & WEAK_TARGET)) {
		temp = AttackUnitsInReactRange(unit);
		if (temp && temp->Type->Priority > goal->Type->Priority) {
			goal->RefsDecrease();
			temp->RefsIncrease();
			if (unit->SavedOrder.Action == UnitActionStill) {
				// Save current command to come back.
				unit->SavedOrder = *unit->Orders[0];
				if ((goal = unit->SavedOrder.Goal)) {
					unit->SavedOrder.X = goal->X + goal->Type->TileWidth / 2;
					unit->SavedOrder.Y = goal->Y + goal->Type->TileHeight / 2;
					unit->SavedOrder.MinRange = 0;
					unit->SavedOrder.Range = 0;
					unit->SavedOrder.Goal = NoUnitP;
				}
			}
			unit->Orders[0]->Goal = goal = temp;
			unit->Orders[0]->X = unit->Orders[0]->Y = -1;
			NewResetPath(unit);
		}
	}

	Assert(!unit->Type->Vanishes && !unit->Destroyed && !unit->Removed);
	Assert(unit->Orders[0]->Action == UnitActionAttack ||
		unit->Orders[0]->Action == UnitActionAttackGround);

	return 0;
}

/**
**  Controls moving a unit to its target when attacking
**
**  @param unit  Unit that is attacking and moving
*/
static void MoveToTarget(CUnit *unit)
{
	CUnit *goal;
	int err;

	Assert(unit);
	Assert(!unit->Type->Vanishes && !unit->Destroyed && !unit->Removed);
	Assert(unit->Orders[0]->Action == UnitActionAttack ||
		unit->Orders[0]->Action == UnitActionAttackGround);
	Assert(CanMove(unit));
	Assert(unit->Orders[0]->Goal || (unit->Orders[0]->X != -1 && unit->Orders[0]->Y != -1));

	err = DoActionMove(unit);

	if (unit->Anim.Unbreakable) {
		return;
	}
	//
	// Look if we have reached the target.
	//
	goal = unit->Orders[0]->Goal;
	if (err == 0 && !goal) {
		// Check if we're in range when attacking a location and we are waiting
		if (MapDistanceToUnit(unit->Orders[0]->X, unit->Orders[0]->Y, unit) <
				unit->Stats->Variables[ATTACKRANGE_INDEX].Max) {
			err = PF_REACHED;
		}
	}
	if (err >= 0) {
		if (CheckForTargetInRange(unit)) {
			return;
		}
		return;
	}
	if (err == PF_REACHED) {
		//
		// Have reached target? FIXME: could use the new return code?
		//
		if (goal && MapDistanceBetweenUnits(unit, goal) <=
				unit->Stats->Variables[ATTACKRANGE_INDEX].Max) {
			// Reached another unit, now attacking it
			unit->State = 0;
			UnitHeadingFromDeltaXY(unit,
				goal->X + (goal->Type->TileWidth - 1) / 2 - unit->X,
				goal->Y + (goal->Type->TileHeight - 1) / 2 - unit->Y);
			unit->SubAction++;
			return;
		}
		//
		// Attacking ground.
		//
		if (!goal && unit->Orders[0]->Action == UnitActionAttackGround &&
				MapDistanceToUnit(unit->Orders[0]->X, unit->Orders[0]->Y, unit) <=
					unit->Stats->Variables[ATTACKRANGE_INDEX].Max) {
			// Reached ground, now attacking it
			unit->State = 0;
			UnitHeadingFromDeltaXY(unit, unit->Orders[0]->X - unit->X,
				unit->Orders[0]->Y - unit->Y);
			unit->SubAction &= WEAK_TARGET;
			unit->SubAction |= ATTACK_TARGET;
			return;
		}
	}
	//
	// Unreachable.
	//
	if (err == PF_UNREACHABLE) {
		unit->State = 0;
		if (!goal && unit->Orders[0]->Range == 0) {
			// Try again with unit's max attack range
			unit->Orders[0]->Range = unit->Stats->Variables[ATTACKRANGE_INDEX].Max;
			unit->Wait = 15;
			return;
		}
	}
	//
	// Return to old task?
	//
	unit->State = unit->SubAction = 0;
	if (unit->Orders[0]->Goal) {
		unit->Orders[0]->Goal->RefsDecrease();
	}
	*unit->Orders[0] = unit->SavedOrder;
	NewResetPath(unit);

	// Must finish, if saved command finishes
	unit->SavedOrder.Action = UnitActionStill;
	unit->SavedOrder.Goal = NoUnitP;
}

/**
**  Handle attacking the target.
**
**  @param unit  Unit, for that the attack is handled.
*/
static void AttackTarget(CUnit *unit)
{
	CUnit *goal; // unit->Order[0].Goal
	CUnit *temp;

	Assert(unit);
	Assert(unit->Orders[0]->Goal || (unit->Orders[0]->X != -1 && unit->Orders[0]->Y != -1));

	AnimateActionAttack(unit);
	if (unit->Anim.Unbreakable) {
		return;
	}
	//
	// Goal is "weak".
	//
	goal = unit->Orders[0]->Goal;
	if (!goal && unit->Orders[0]->Action == UnitActionAttackGround) {
		return;
	}

	//
	// Target is dead ? Change order ?
	//
	if (CheckForDeadGoal(unit)) {
		return;
	}
	goal = unit->Orders[0]->Goal;

	//
	// No target choose one.
	//
	if (!goal) {
		unit->State = 0;
		goal = AttackUnitsInReactRange(unit);
		//
		// No new goal, continue way to destination.
		//
		if (!goal) {
			// Return to old task ?
			if (unit->SavedOrder.Action != UnitActionStill) {
				unit->SubAction = 0;
				Assert(unit->Orders[0]->Goal == NoUnitP);
				*unit->Orders[0] = unit->SavedOrder;
				NewResetPath(unit);
				// Must finish, if saved command finishes
				unit->SavedOrder.Action = UnitActionStill;

				// This isn't supported
				Assert(unit->SavedOrder.Goal == NoUnitP);
				return;
			}
			// There's no goal, so the goal can't be weak.
			Assert(!(unit->SubAction & WEAK_TARGET));
			unit->SubAction = MOVE_TO_TARGET;
			return;
		}

		//
		// Save current command to come back.
		//
		if (unit->SavedOrder.Action == UnitActionStill) {
			Assert(unit->Orders[0]->Goal == NULL);
			unit->SavedOrder = *unit->Orders[0];
		}

		goal->RefsIncrease();
		unit->Orders[0]->Goal = goal;
		unit->Orders[0]->X = unit->Orders[0]->Y = -1;
		unit->Orders[0]->MinRange = unit->Type->MinAttackRange;
		unit->Orders[0]->Range = unit->Stats->Variables[ATTACKRANGE_INDEX].Max;
		NewResetPath(unit);
		unit->SubAction |= WEAK_TARGET;

	//
	// Have a weak target, try a better target.
	// FIXME: if out of range also try another target quick
	//
	} else if (goal && (unit->SubAction & WEAK_TARGET)) {
		temp = AttackUnitsInReactRange(unit);
		if (temp && temp->Type->Priority > goal->Type->Priority) {
			goal->RefsDecrease();
			temp->RefsIncrease();

			if (unit->SavedOrder.Action == UnitActionStill) {
				// Save current order to come back or to continue it.
				unit->SavedOrder = *unit->Orders[0];
				if ((goal = unit->SavedOrder.Goal)) {
					DebugPrint("Have goal to come back %d\n" _C_
						UnitNumber(goal));
					unit->SavedOrder.X = goal->X + goal->Type->TileWidth / 2;
					unit->SavedOrder.Y = goal->Y + goal->Type->TileHeight / 2;
					unit->SavedOrder.MinRange = 0;
					unit->SavedOrder.Range = 0;
					unit->SavedOrder.Goal = NoUnitP;
				}
			}
			unit->Orders[0]->Goal = goal = temp;
			unit->Orders[0]->X = unit->Orders[0]->Y = -1;
			unit->Orders[0]->MinRange = unit->Type->MinAttackRange;
			unit->SubAction = MOVE_TO_TARGET | WEAK_TARGET;
			NewResetPath(unit);
		}
	}

	//
	// Still near to target, if not goto target.
	//
	if (MapDistanceBetweenUnits(unit, goal) > unit->Stats->Variables[ATTACKRANGE_INDEX].Max) {
		if (unit->SavedOrder.Action == UnitActionStill) {
			// Save current order to come back or to continue it.
			unit->SavedOrder = *unit->Orders[0];
			if ((temp = unit->SavedOrder.Goal)) {
				DebugPrint("Have goal to come back %d\n" _C_
					UnitNumber(temp));
				unit->SavedOrder.X = temp->X + temp->Type->TileWidth / 2;
				unit->SavedOrder.Y = temp->Y + temp->Type->TileHeight / 2;
				unit->SavedOrder.MinRange = 0;
				unit->SavedOrder.Range = 0;
				unit->SavedOrder.Goal = NoUnitP;
			}
		}
		NewResetPath(unit);
		unit->Frame = 0;
		unit->State = 0;
		unit->SubAction &= WEAK_TARGET;
		unit->SubAction |= MOVE_TO_TARGET;
	}
	if (MapDistanceBetweenUnits(unit, goal) < unit->Type->MinAttackRange) {
		unit->SubAction &= WEAK_TARGET;
		unit->SubAction |= MOVE_TO_TARGET;
	}

	//
	// Turn always to target
	//
	if (goal) {
		UnitHeadingFromDeltaXY(unit,
			goal->X + (goal->Type->TileWidth - 1) / 2 - unit->X,
			goal->Y + (goal->Type->TileHeight - 1) / 2 - unit->Y);
	}
}

/**
**  Unit attacks!
**
**  I added a little trick, if SubAction&WEAK_TARGET is true the goal is
**  a weak goal.  This means the unit AI (little AI) could choose a new
**  better goal.
**
**  @todo  Lets do some tries to reach the target.
**         If target place is not reachable, choose better goal to reduce
**         the pathfinder load.
**
**  @param unit  Unit, for that the attack is handled.
*/
void HandleActionAttack(CUnit *unit)
{
	int dist;          // dist between unit and unit->Orders[0]->Goal.

	Assert(unit->Orders[0]->Action == UnitActionAttackGround ||
		unit->Orders[0]->Action == UnitActionAttack);
	Assert(unit->Orders[0]->Goal || (unit->Orders[0]->X != -1 && unit->Orders[0]->Y != -1));

	if (unit->Wait) {
		unit->Wait--;
		return;
	}

	switch (unit->SubAction) {
		//
		// First entry
		//
		case 0:
			// did Order change ?
			if (CheckForTargetInRange(unit)) {
				return;
			}
			// CheckForTargetInRange may have set
			// SubAction |= WEAK_TARGET.

			// Can we already attack ?
			if (unit->Orders[0]->Goal) {
				dist = MapDistanceBetweenUnits(unit, unit->Orders[0]->Goal);
				if (unit->Type->MinAttackRange < dist && dist <= unit->Stats->Variables[ATTACKRANGE_INDEX].Max) {
					UnitHeadingFromDeltaXY(unit,
						unit->Orders[0]->Goal->X + (unit->Orders[0]->Goal->Type->TileWidth - 1) / 2 - unit->X,
						unit->Orders[0]->Goal->Y + (unit->Orders[0]->Goal->Type->TileHeight - 1) / 2 - unit->Y);
					unit->SubAction &= WEAK_TARGET;
					unit->SubAction |= ATTACK_TARGET;
					AttackTarget(unit);
					return;
				}
			}
			unit->SubAction &= WEAK_TARGET;
			unit->SubAction |= MOVE_TO_TARGET;
			NewResetPath(unit);
			//
			// FIXME: should use a reachable place to reduce pathfinder time.
			//
			Assert(unit->State == 0);
			// FALL THROUGH
		//
		// Move near to the target.
		//
		case MOVE_TO_TARGET:
		case MOVE_TO_TARGET + WEAK_TARGET:
			if (!CanMove(unit)) {
				// Release order.
				if (unit->Orders[0]->Goal) {
					unit->Orders[0]->Goal->RefsDecrease();
				}
				*unit->Orders[0] = unit->SavedOrder;
				unit->SavedOrder.Action = UnitActionStill;
				unit->SavedOrder.Goal = NoUnitP;
				unit->State = 0;
				unit->SubAction = 0;
				NewResetPath(unit);
				return;
			}
			MoveToTarget(unit);
			break;

		//
		// Attack the target.
		//
		case ATTACK_TARGET:
		case ATTACK_TARGET + WEAK_TARGET:
			AttackTarget(unit);
			break;

		case WEAK_TARGET:
			DebugPrint("FIXME: wrong entry.\n");
			break;
	}
}

//@}
