//       _________ __                 __
//      /   _____//  |_____________ _/  |______     ____  __ __  ______
//      \_____  \\   __\_  __ \__  \\   __\__  \   / ___\|  |  \/  ___/
//      /        \|  |  |  | \// __ \|  |  / __ \_/ /_/  >  |  /\___ |
//     /_______  /|__|  |__|  (____  /__| (____  /\___  /|____//____  >
//             \/                  \/          \//_____/            \/
//  ______________________                           ______________________
//                        T H E   W A R   B E G I N S
//         Stratagus - A free fantasy real time strategy game engine
//
/**@name action_attack.cpp - The attack action. */
//
//      (c) Copyright 1998-2005 by Lutz Sammer and Jimmy Salmon
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
void AnimateActionAttack(CUnit &unit)
{
	//  No animation.
	//  So direct fire missile.
	//  FIXME : wait a little.
	if (!unit.Type->Animations || !unit.Type->Animations->Attack) {
		FireMissile(unit);
		UnHideUnit(unit);// unit is invisible until attacks
		return;
	}
	UnitShowAnimation(unit, unit.Type->Animations->Attack);
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
static int CheckForDeadGoal(CUnit &unit)
{
	COrderPtr order = unit.CurrentOrder();
	CUnit *goal = order->GetGoal();

	// Position or valid target, it is ok.
	if (!goal || goal->IsVisibleAsGoal(*unit.Player)) {
		return 0;
	}

	// Goal could be destroyed or unseen
	// So, cannot use type.
	order->goalPos = goal->tilePos;
	order->MinRange = 0;
	order->Range = 0;
	order->ClearGoal();

	//
	// If we have a saved order continue this saved order.
	//
	if (unit.RestoreOrder()) {
		//unit.ClearAction();
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
static int CheckForTargetInRange(CUnit &unit)
{
	// Target is dead?
	if (CheckForDeadGoal(unit)) {
		return 1;
	}
	COrderPtr order = unit.CurrentOrder();

	// No goal: if meeting enemy attack it.
	if (!order->HasGoal() &&
		order->Action != UnitActionAttackGround &&
		!Map.WallOnMap(order->goalPos)) {
		CUnit *goal = AttackUnitsInReactRange(unit);

		if (goal) {
			COrder *savedOrder = COrder::NewActionAttack(unit, order->goalPos);

			if (unit.StoreOrder(savedOrder) == false) {
				delete savedOrder;
				savedOrder = NULL;
			}
			order->SetGoal(goal);
			order->MinRange = unit.Type->MinAttackRange;
			order->Range = unit.Stats->Variables[ATTACKRANGE_INDEX].Max;
			order->goalPos.x = order->goalPos.y = -1;
			unit.SubAction |= WEAK_TARGET; // weak target
			order->NewResetPath();
		}
	// Have a weak target, try a better target.
	} else if (order->HasGoal() && (unit.SubAction & WEAK_TARGET)) {
		CUnit *goal = order->GetGoal();
		CUnit *temp = AttackUnitsInReactRange(unit);

		if (temp && temp->Type->Priority > goal->Type->Priority) {
			COrder *savedOrder = order->Clone();

			if (unit.StoreOrder(savedOrder) == false) {
				delete savedOrder;
				savedOrder = NULL;
			}
			order->SetGoal(temp);
			order->goalPos.x = order->goalPos.y = -1;
			order->NewResetPath();
		}
	}

	Assert(!unit.Type->Vanishes && !unit.Destroyed && !unit.Removed);
	Assert(order->Action == UnitActionAttack || order->Action == UnitActionAttackGround);

	return 0;
}

/**
**  Controls moving a unit to its target when attacking
**
**  @param unit  Unit that is attacking and moving
*/
static void MoveToTarget(CUnit &unit)
{
	Assert(!unit.Type->Vanishes && !unit.Destroyed && !unit.Removed);
	Assert(unit.CurrentAction() == UnitActionAttack || unit.CurrentAction() == UnitActionAttackGround);
	Assert(unit.CanMove());
	Assert(unit.CurrentOrder()->HasGoal()
		 || (unit.CurrentOrder()->goalPos.x != -1 && unit.CurrentOrder()->goalPos.y != -1));

	int err = DoActionMove(unit);

	if (unit.Anim.Unbreakable) {
		return;
	}

	//
	// Look if we have reached the target.
	//
	COrderPtr order = unit.CurrentOrder();
	if (err == 0 && !order->HasGoal()) {
		// Check if we're in range when attacking a location and we are waiting
		if (unit.MapDistanceTo(order->goalPos.x, order->goalPos.y) <=
				unit.Stats->Variables[ATTACKRANGE_INDEX].Max) {
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
		CUnit *goal = order->GetGoal();
		//
		// Have reached target? FIXME: could use the new return code?
		//
		if (goal
		&& unit.MapDistanceTo(*goal) <= unit.Stats->Variables[ATTACKRANGE_INDEX].Max) {
			// Reached another unit, now attacking it
			unit.State = 0;
			const Vec2i dir = goal->tilePos + goal->Type->GetHalfTileSize() - unit.tilePos;
			UnitHeadingFromDeltaXY(unit, dir);
			unit.SubAction++;
			return;
		}
		//
		// Attacking wall or ground.
		//
		if (!goal && (Map.WallOnMap(order->goalPos) ||
					order->Action == UnitActionAttackGround) &&
				unit.MapDistanceTo(order->goalPos.x, order->goalPos.y) <=
					unit.Stats->Variables[ATTACKRANGE_INDEX].Max) {
			// Reached wall or ground, now attacking it
			unit.State = 0;
			UnitHeadingFromDeltaXY(unit, order->goalPos - unit.tilePos);
			unit.SubAction &= WEAK_TARGET;
			unit.SubAction |= ATTACK_TARGET;
			return;
		}
	}
	//
	// Unreachable.
	//
	if (err == PF_UNREACHABLE) {
		unit.State = 0;
		if (!order->HasGoal()) {
			//
			// When attack-moving we have to allow a bigger range
			//
			if (order->CheckRange()) {
				// Try again with more range
				order->Range++;
				unit.Wait = 5;
				return;
			}
		} else {
			order->ClearGoal();
		}
	}
	//
	// Return to old task?
	//
	if (!unit.RestoreOrder()) {
		unit.ClearAction();
		unit.State = 0;
	}
}

/**
**  Handle attacking the target.
**
**  @param unit  Unit, for that the attack is handled.
*/
static void AttackTarget(CUnit &unit)
{
	Assert(unit.CurrentOrder()->HasGoal() ||
		(unit.CurrentOrder()->goalPos.x != -1 && unit.CurrentOrder()->goalPos.y != -1));

	AnimateActionAttack(unit);
	if (unit.Anim.Unbreakable) {
		return;
	}
	//
	// Goal is "weak" or a wall.
	//
	COrderPtr order = unit.CurrentOrder();
	if (!order->HasGoal() && (order->Action == UnitActionAttackGround || Map.WallOnMap(order->goalPos))) {
		return;
	}

	//
	// Target is dead ? Change order ?
	//
	if (CheckForDeadGoal(unit)) {
		return;
	}
	CUnit *goal = order->GetGoal();

	//
	// No target choose one.
	//
	if (!goal) {
		unit.State = 0;
		goal = AttackUnitsInReactRange(unit);

		// No new goal, continue way to destination.
		if (!goal) {
			// Return to old task ?
			if (unit.RestoreOrder()) {
				return;
			}
			unit.SubAction = MOVE_TO_TARGET;
			return;
		}
		// Save current command to come back.
		COrder *savedOrder = order->Clone();

		if (unit.StoreOrder(savedOrder) == false) {
			delete savedOrder;
			savedOrder = NULL;
		}
		order->SetGoal(goal);
		order->goalPos.x = order->goalPos.y = -1;
		order->MinRange = unit.Type->MinAttackRange;
		order->Range = unit.Stats->Variables[ATTACKRANGE_INDEX].Max;
		order->NewResetPath();
		unit.SubAction |= WEAK_TARGET;

	//
	// Have a weak target, try a better target.
	// FIXME: if out of range also try another target quick
	//
	} else {
		if ((unit.SubAction & WEAK_TARGET)) {
			CUnit *temp = AttackUnitsInReactRange(unit);
			if (temp && temp->Type->Priority > goal->Type->Priority) {
				COrder *savedOrder = order->Clone();

				if (unit.StoreOrder(savedOrder) == false) {
					delete savedOrder;
					savedOrder = NULL;
				}
				goal = temp;
				order->SetGoal(temp);
				order->goalPos.x = order->goalPos.y = -1;
				order->MinRange = unit.Type->MinAttackRange;
				unit.SubAction = MOVE_TO_TARGET;
				order->NewResetPath();
			}
		}
	}

	//
	// Still near to target, if not goto target.
	//
	int dist = unit.MapDistanceTo(*goal);
	if (dist > unit.Stats->Variables[ATTACKRANGE_INDEX].Max) {
		COrder *savedOrder = order->Clone();

		if (unit.StoreOrder(savedOrder) == false) {
			delete savedOrder;
			savedOrder = NULL;
		}
		order->NewResetPath();
		unit.Frame = 0;
		unit.State = 0;
		unit.SubAction &= WEAK_TARGET;
		unit.SubAction |= MOVE_TO_TARGET;
	}
	if (dist < unit.Type->MinAttackRange) {
		unit.SubAction = MOVE_TO_TARGET;
	}

	//
	// Turn always to target
	//
	if (goal) {
		const Vec2i dir = goal->tilePos + goal->Type->GetHalfTileSize() - unit.tilePos;
		UnitHeadingFromDeltaXY(unit, dir);
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
void HandleActionAttack(COrder& order, CUnit &unit)
{
	Assert(order.Action == UnitActionAttackGround || order.Action == UnitActionAttack);
	Assert(order.HasGoal() || Map.Info.IsPointOnMap(order.goalPos));

	if (unit.Wait) {
		unit.Wait--;
		return;
	}

	switch (unit.SubAction) {
		case 0: // First entry
		{
			// did Order change ?
			if (CheckForTargetInRange(unit)) {
				return;
			}
			// Can we already attack ?
			if (order.HasGoal()) {
				CUnit &goal = *order.GetGoal();
				const int dist = goal.MapDistanceTo(unit);

				if (unit.Type->MinAttackRange < dist &&
					 dist <= unit.Stats->Variables[ATTACKRANGE_INDEX].Max) {
					const Vec2i dir = goal.tilePos + goal.Type->GetHalfTileSize() - unit.tilePos;

					UnitHeadingFromDeltaXY(unit, dir);
					unit.SubAction = ATTACK_TARGET;
					AttackTarget(unit);
					return;
				}
			}
			unit.SubAction = MOVE_TO_TARGET;
			order.NewResetPath();
			// FIXME: should use a reachable place to reduce pathfinder time.
			Assert(unit.State == 0);
		}
		// FALL THROUGH
		case MOVE_TO_TARGET:
		case MOVE_TO_TARGET + WEAK_TARGET:
			if (!unit.CanMove()) {
				if (!unit.RestoreOrder()) {
					unit.ClearAction();
					unit.State = 0;
				}
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
