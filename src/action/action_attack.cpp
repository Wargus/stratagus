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
#include "action/action_attack.h"

#include "animation.h"
#include "iolib.h"
#include "map.h"
#include "missile.h"
#include "pathfinder.h"
#include "player.h"
#include "script.h"
#include "sound.h"
#include "spells.h"
#include "ui.h"
#include "unit.h"
#include "unittype.h"

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
void AnimateActionAttack(CUnit &unit, COrder &order)
{
	//  No animation.
	//  So direct fire missile.
	//  FIXME : wait a little.
	if (!unit.Type->Animations || !unit.Type->Animations->Attack) {
		order.OnAnimationAttack(unit);
		return;
	}
	UnitShowAnimation(unit, unit.Type->Animations->Attack);
}

/* static */ COrder *COrder::NewActionAttack(const CUnit &attacker, CUnit &target)
{
	COrder_Attack *order = new COrder_Attack(false);

	if (target.Destroyed) {
		order->goalPos = target.tilePos + target.Type->GetHalfTileSize();
	} else {
		// Removed, Dying handled by action routine.
		order->SetGoal(&target);
		order->Range = attacker.Stats->Variables[ATTACKRANGE_INDEX].Max;
		order->MinRange = attacker.Type->MinAttackRange;
	}
	return order;
}

/* static */ COrder *COrder::NewActionAttack(const CUnit &attacker, const Vec2i &dest)
{
	Assert(Map.Info.IsPointOnMap(dest));

	COrder_Attack *order = new COrder_Attack(false);

	if (Map.WallOnMap(dest) && Map.IsFieldExplored(*attacker.Player, dest)) {
		// FIXME: look into action_attack.cpp about this ugly problem
		order->goalPos = dest;
		order->Range = attacker.Stats->Variables[ATTACKRANGE_INDEX].Max;
		order->MinRange = attacker.Type->MinAttackRange;
	} else {
		order->goalPos = dest;
	}
	return order;
}

/* static */ COrder *COrder::NewActionAttackGround(const CUnit &attacker, const Vec2i &dest)
{
	COrder_Attack *order = new COrder_Attack(true);

	order->goalPos = dest;
	order->Range = attacker.Stats->Variables[ATTACKRANGE_INDEX].Max;
	order->MinRange = attacker.Type->MinAttackRange;

	return order;
}


/* virtual */ void COrder_Attack::Save(CFile &file, const CUnit &unit) const
{
	Assert(Action == UnitActionAttack || Action == UnitActionAttackGround);

	if (Action == UnitActionAttack) {
		file.printf("{\"action-attack\",");
	} else {
		file.printf("{\"action-attack-ground\",");
	}
	file.printf(" \"range\", %d,", this->Range);
	file.printf(" \"min-range\", %d,", this->MinRange);

	if (this->Finished) {
		file.printf(" \"finished\", ");
	}
	if (this->HasGoal()) {
		file.printf(" \"goal\", \"%s\",", UnitReference(this->GetGoal()).c_str());
	}
	file.printf(" \"tile\", {%d, %d},", this->goalPos.x, this->goalPos.y);

	file.printf(" \"state\", %d", this->State);
	file.printf("}");
}


/* virtual */ bool COrder_Attack::ParseSpecificData(lua_State *l, int &j, const char *value, const CUnit &unit)
{
	if (!strcmp(value, "state")) {
		++j;
		lua_rawgeti(l, -1, j + 1);
		this->State = LuaToNumber(l, -1);
		lua_pop(l, 1);
	} else if (!strcmp(value, "min-range")) {
		++j;
		lua_rawgeti(l, -1, j + 1);
		this->MinRange = LuaToNumber(l, -1);
		lua_pop(l, 1);
	} else if (!strcmp(value, "range")) {
		++j;
		lua_rawgeti(l, -1, j + 1);
		this->Range = LuaToNumber(l, -1);
		lua_pop(l, 1);
	} else if (!strcmp(value, "tile")) {
		++j;
		lua_rawgeti(l, -1, j + 1);
		CclGetPos(l, &this->goalPos.x , &this->goalPos.y);
		lua_pop(l, 1);
	} else {
		return false;
	}
	return true;
}

/* virtual */ bool COrder_Attack::IsValid() const
{
	if (Action == UnitActionAttack) {
		if (this->HasGoal()) {
			return this->GetGoal()->IsAliveOnMap();
		} else {
			return Map.Info.IsPointOnMap(this->goalPos);
		}
	} else {
		Assert(Action == UnitActionAttackGround);
		return Map.Info.IsPointOnMap(this->goalPos);
	}
}

/* virtual */ PixelPos COrder_Attack::Show(const CViewport &vp, const PixelPos &lastScreenPos) const
{
	PixelPos targetPos;

	if (this->HasGoal()) {
		targetPos = vp.MapToScreenPixelPos(this->GetGoal()->GetMapPixelPosCenter());
	} else {
		targetPos = vp.TilePosToScreen_Center(this->goalPos);
	}
	Video.FillCircleClip(ColorRed, lastScreenPos, 2);
	Video.DrawLineClip(ColorRed, lastScreenPos, targetPos);
	Video.FillCircleClip(IsWeakTargetSelected() ? ColorBlue : ColorRed, targetPos, 3);
	return targetPos;
}

/* virtual */ void COrder_Attack::UpdatePathFinderData(PathFinderInput &input)
{
	input.SetMinRange(this->MinRange);
	input.SetMaxRange(this->Range);

	Vec2i tileSize;
	if (this->HasGoal()) {
		CUnit *goal = this->GetGoal();
		tileSize.x = goal->Type->TileWidth;
		tileSize.y = goal->Type->TileHeight;
		input.SetGoal(goal->tilePos, tileSize);
	} else {
		tileSize.x = 0;
		tileSize.y = 0;
		input.SetGoal(this->goalPos, tileSize);
	}
}

/* virtual */ void COrder_Attack::OnAnimationAttack(CUnit &unit)
{
	Assert(unit.Type->CanAttack);

	FireMissile(unit, this->GetGoal(), this->goalPos);
	UnHideUnit(unit); // unit is invisible until attacks
}

/* virtual */ bool COrder_Attack::OnAiHitUnit(CUnit &unit, CUnit *attacker, int /*damage*/)
{
	CUnit *goal = this->GetGoal();

	if (goal) {
		if (goal->IsAlive() == false) {
			this->ClearGoal();
			this->goalPos = goal->tilePos;
			return false;
		}
		if (goal == attacker) {
			return true;
		}
		if (goal->CurrentAction() == UnitActionAttack) {
			const COrder_Attack &order = *static_cast<COrder_Attack *>(goal->CurrentOrder());
			if (order.GetGoal() == &unit) {
				//we already fight with one of attackers;
				return true;
			}
		}
	}
	return false;
}



bool COrder_Attack::IsWeakTargetSelected() const
{
	return (this->State & WEAK_TARGET) != 0;
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
**  @return      true if order have changed, false else.
*/
bool COrder_Attack::CheckForDeadGoal(CUnit &unit)
{
	CUnit *goal = this->GetGoal();

	// Position or valid target, it is ok.
	if (!goal || goal->IsVisibleAsGoal(*unit.Player)) {
		return false;
	}

	// Goal could be destroyed or unseen
	// So, cannot use type.
	this->goalPos = goal->tilePos;
	this->MinRange = 0;
	this->Range = 0;
	this->ClearGoal();

	// If we have a saved order continue this saved order.
	if (unit.RestoreOrder()) {
		return true;
	}
	return false;
}

/**
**  Change invalid target for new target in range.
**
**  @param unit  Unit to check if goal is in range
**
**  @return      true if order(action) have changed, false else (if goal change return false).
*/
bool COrder_Attack::CheckForTargetInRange(CUnit &unit)
{
	// Target is dead?
	if (CheckForDeadGoal(unit)) {
		return true;
	}

	// No goal: if meeting enemy attack it.
	if (!this->HasGoal()
		&& this->Action != UnitActionAttackGround
		&& !Map.WallOnMap(this->goalPos)) {
		CUnit *goal = AttackUnitsInReactRange(unit);

		if (goal) {
			COrder *savedOrder = COrder::NewActionAttack(unit, this->goalPos);

			if (unit.StoreOrder(savedOrder) == false) {
				delete savedOrder;
				savedOrder = NULL;
			}
			this->SetGoal(goal);
			this->MinRange = unit.Type->MinAttackRange;
			this->Range = unit.Stats->Variables[ATTACKRANGE_INDEX].Max;
			this->goalPos = goal->tilePos;
			this->State |= WEAK_TARGET; // weak target
		}
		// Have a weak target, try a better target.
	} else if (this->HasGoal() && (this->State & WEAK_TARGET || unit.Player->AiEnabled)) {
		CUnit *goal = this->GetGoal();
		CUnit *newTarget = AttackUnitsInReactRange(unit);

		if (newTarget && newTarget->IsAgressive()
			&& ThreatCalculate(unit, *newTarget) < ThreatCalculate(unit, *goal)) {
			COrder *savedOrder = this->Clone();

			if (unit.StoreOrder(savedOrder) == false) {
				delete savedOrder;
				savedOrder = NULL;
			}
			this->SetGoal(newTarget);
			this->goalPos = newTarget->tilePos;
		}
	}

	Assert(!unit.Type->Vanishes && !unit.Destroyed && !unit.Removed);
	return false;
}

/**
**  Controls moving a unit to its target when attacking
**
**  @param unit  Unit that is attacking and moving
*/
void COrder_Attack::MoveToTarget(CUnit &unit)
{
	Assert(!unit.Type->Vanishes && !unit.Destroyed && !unit.Removed);
	Assert(unit.CurrentOrder() == this);
	Assert(unit.CanMove());
	Assert(this->HasGoal() || Map.Info.IsPointOnMap(this->goalPos));

	int err = DoActionMove(unit);

	if (unit.Anim.Unbreakable) {
		return;
	}

	// Look if we have reached the target.
	if (err == 0 && !this->HasGoal()) {
		// Check if we're in range when attacking a location and we are waiting
		if (unit.MapDistanceTo(this->goalPos) <= unit.Stats->Variables[ATTACKRANGE_INDEX].Max) {
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
		CUnit *goal = this->GetGoal();
		// Have reached target? FIXME: could use the new return code?
		if (goal
			&& unit.MapDistanceTo(*goal) <= unit.Stats->Variables[ATTACKRANGE_INDEX].Max) {
			// Reached another unit, now attacking it
			unit.State = 0;
			const Vec2i dir = goal->tilePos + goal->Type->GetHalfTileSize() - unit.tilePos;
			UnitHeadingFromDeltaXY(unit, dir);
			this->State++;
			return;
		}
		// Attacking wall or ground.
		if (((goal && goal->Type && goal->Type->Wall)
			 || (!goal && (Map.WallOnMap(this->goalPos) || this->Action == UnitActionAttackGround)))
			&& unit.MapDistanceTo(this->goalPos) <= unit.Stats->Variables[ATTACKRANGE_INDEX].Max) {
			// Reached wall or ground, now attacking it
			unit.State = 0;
			UnitHeadingFromDeltaXY(unit, this->goalPos - unit.tilePos);
			this->State &= WEAK_TARGET;
			this->State |= ATTACK_TARGET;
			return;
		}
	}
	// Unreachable.

	if (err == PF_UNREACHABLE) {
		unit.State = 0;
		if (!this->HasGoal()) {
			// When attack-moving we have to allow a bigger range
			this->Range++;
			unit.Wait = 5;
			return;
		} else {
			this->ClearGoal();
		}
	}

	// Return to old task?
	if (!unit.RestoreOrder()) {
		this->Finished = true;
		unit.State = 0;
	}
}

/**
**  Handle attacking the target.
**
**  @param unit  Unit, for that the attack is handled.
*/
void COrder_Attack::AttackTarget(CUnit &unit)
{
	Assert(this->HasGoal() || Map.Info.IsPointOnMap(this->goalPos));

	AnimateActionAttack(unit, *this);
	if (unit.Anim.Unbreakable) {
		return;
	}

	if (!this->HasGoal() && (this->Action == UnitActionAttackGround || Map.WallOnMap(this->goalPos))) {
		return;
	}

	// Target is dead ? Change order ?
	if (CheckForDeadGoal(unit)) {
		return;
	}
	CUnit *goal = this->GetGoal();

	// No target choose one.
	if (!goal) {
		unit.State = 0;
		goal = AttackUnitsInReactRange(unit);

		// No new goal, continue way to destination.
		if (!goal) {
			// Return to old task ?
			if (unit.RestoreOrder()) {
				return;
			}
			this->State = MOVE_TO_TARGET;
			return;
		}
		// Save current command to come back.
		COrder *savedOrder = this->Clone();

		if (unit.StoreOrder(savedOrder) == false) {
			delete savedOrder;
			savedOrder = NULL;
		}
		this->SetGoal(goal);
		this->goalPos = goal->tilePos;
		this->MinRange = unit.Type->MinAttackRange;
		this->Range = unit.Stats->Variables[ATTACKRANGE_INDEX].Max;
		this->State |= WEAK_TARGET;

		// Have a weak target, try a better target.
		// FIXME: if out of range also try another target quick
	} else {
		if ((this->State & WEAK_TARGET)) {
			CUnit *newTarget = AttackUnitsInReactRange(unit);
			if (newTarget && newTarget->IsAgressive()
				&& ThreatCalculate(unit, *newTarget) < ThreatCalculate(unit, *goal)) {
				COrder *savedOrder = this->Clone();

				if (unit.StoreOrder(savedOrder) == false) {
					delete savedOrder;
					savedOrder = NULL;
				}
				goal = newTarget;
				this->SetGoal(newTarget);
				this->goalPos = newTarget->tilePos;
				this->MinRange = unit.Type->MinAttackRange;
				this->State = MOVE_TO_TARGET;
			}
		}
	}

	// Still near to target, if not goto target.
	const int dist = unit.MapDistanceTo(*goal);
	if (dist > unit.Stats->Variables[ATTACKRANGE_INDEX].Max) {
		// towers don't chase after goal
		if (unit.CanMove()) {
			COrder *savedOrder = this->Clone();
			if (unit.StoreOrder(savedOrder) == false) {
				delete savedOrder;
				savedOrder = NULL;
			}
		}
		unit.Frame = 0;
		unit.State = 0;
		this->State &= WEAK_TARGET;
		this->State |= MOVE_TO_TARGET;
	}
	if (dist < unit.Type->MinAttackRange) {
		this->State = MOVE_TO_TARGET;
	}

	// Turn always to target
	if (goal) {
		const Vec2i dir = goal->tilePos + goal->Type->GetHalfTileSize() - unit.tilePos;
		UnitHeadingFromDeltaXY(unit, dir);
	}
}

/**
**  Unit attacks!
**
**  if (SubAction & WEAK_TARGET) is true the goal is a weak goal.
**  This means the unit AI (little AI) could choose a new better goal.
**
**  @todo  Lets do some tries to reach the target.
**         If target place is not reachable, choose better goal to reduce
**         the pathfinder load.
**
**  @param unit  Unit, for that the attack is handled.
*/
/* virtual */ void COrder_Attack::Execute(CUnit &unit)
{
	Assert(this->HasGoal() || Map.Info.IsPointOnMap(this->goalPos));

	if (unit.Wait) {
		unit.Wait--;
		return;
	}

	switch (this->State) {
		case 0: { // First entry
			// did Order change ?
			if (CheckForTargetInRange(unit)) {
				return;
			}
			// Can we already attack ?
			if (this->HasGoal()) {
				CUnit &goal = *this->GetGoal();
				const int dist = goal.MapDistanceTo(unit);

				if (unit.Type->MinAttackRange < dist &&
					dist <= unit.Stats->Variables[ATTACKRANGE_INDEX].Max) {
					const Vec2i dir = goal.tilePos + goal.Type->GetHalfTileSize() - unit.tilePos;

					UnitHeadingFromDeltaXY(unit, dir);
					this->State = ATTACK_TARGET;
					AttackTarget(unit);
					return;
				}
			}
			this->State = MOVE_TO_TARGET;
			// FIXME: should use a reachable place to reduce pathfinder time.
			Assert(unit.State == 0);
		}
		// FALL THROUGH
		case MOVE_TO_TARGET:
		case MOVE_TO_TARGET + WEAK_TARGET:
			if (!unit.CanMove()) {
				this->Finished = true;
				unit.State = 0;
				return;
			}
			MoveToTarget(unit);
			break;

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
