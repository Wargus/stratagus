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
//      (c) Copyright 1998-2015 by Lutz Sammer, Jimmy Salmon and Andrettin
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

#include "stratagus.h"

#include "action/action_attack.h"

#include "animation.h"
#include "iolib.h"
#include "map.h"
#include "missile.h"
#include "pathfinder.h"
#include "player.h"
#include "script.h"
#include "settings.h"
#include "sound.h"
#include "spells.h"
#include "tileset.h"
#include "ui.h"
#include "unit.h"
#include "unit_find.h"
#include "unittype.h"
#include "video.h"

/*----------------------------------------------------------------------------
--  Defines
----------------------------------------------------------------------------*/

#define FIRST_ENTRY		 	0
#define AUTO_TARGETING   	2  /// Targets will be selected by small (unit's) AI
#define MOVE_TO_TARGET   	4  /// Move to target state
#define ATTACK_TARGET    	5  /// Attack target state
#define MOVE_TO_ATTACKPOS	8  /// Move to position for attack if target is too close

#define RESTORE_ONLY false  /// Do not finish this order, only restore saved

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
	if (unit.Type->Animations && unit.Type->Animations->RangedAttack && unit.IsAttackRanged(order.GetGoal(), order.GetGoalPos())) {
		UnitShowAnimation(unit, unit.Type->Animations->RangedAttack);
	} else {
		if (!unit.Type->Animations || !unit.Type->Animations->Attack) {
			order.OnAnimationAttack(unit);
			return;
		}
		UnitShowAnimation(unit, unit.Type->Animations->Attack);
	}
}

/* static */ COrder *COrder::NewActionAttack(const CUnit &attacker, CUnit &target)
{
	COrder_Attack *order = new COrder_Attack(false);

	order->goalPos = target.tilePos + target.Type->GetHalfTileSize();
	// Removed, Dying handled by action routine.
	order->SetGoal(&target);
	order->Range = attacker.Stats->Variables[ATTACKRANGE_INDEX].Max;
	order->MinRange = attacker.Type->MinAttackRange;
	if (attacker.Type->BoolFlag[SKIRMISHER_INDEX].value) {
		order->SkirmishRange = order->Range;
	}

	return order;
}

/* static */ COrder *COrder::NewActionAttack(const CUnit &attacker, const Vec2i &dest)
{
	Assert(Map.Info.IsPointOnMap(dest));

	COrder_Attack *order = new COrder_Attack(false);

	if (Map.WallOnMap(dest) && Map.Field(dest)->playerInfo.IsExplored(*attacker.Player)) {
		// FIXME: look into action_attack.cpp about this ugly problem
		order->goalPos = dest;
		order->Range = attacker.Stats->Variables[ATTACKRANGE_INDEX].Max;
		order->MinRange = attacker.Type->MinAttackRange;
	} else {
		order->goalPos = dest;
		order->attackMovePos = dest;
		order->State = AUTO_TARGETING;
	}
	if (attacker.Type->BoolFlag[SKIRMISHER_INDEX].value) {
		order->SkirmishRange = attacker.Stats->Variables[ATTACKRANGE_INDEX].Max;
	}

	return order;
}

/* static */ COrder *COrder::NewActionAttackGround(const CUnit &attacker, const Vec2i &dest)
{
	COrder_Attack *order = new COrder_Attack(true);

	order->goalPos = dest;
	order->Range = attacker.Stats->Variables[ATTACKRANGE_INDEX].Max;
	order->MinRange = attacker.Type->MinAttackRange;
	if (attacker.Type->BoolFlag[SKIRMISHER_INDEX].value) {
		order->SkirmishRange = order->Range;
	}

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
	file.printf(" \"amove-tile\", {%d, %d},", this->attackMovePos.x, this->attackMovePos.y);
	file.printf(" \"state\", %d", this->State);
	file.printf("}");
}


/* virtual */ bool COrder_Attack::ParseSpecificData(lua_State *l, int &j, const char *value, const CUnit &unit)
{
	if (!strcmp(value, "state")) {
		++j;
		this->State = LuaToNumber(l, -1, j + 1);
	} else if (!strcmp(value, "min-range")) {
		++j;
		this->MinRange = LuaToNumber(l, -1, j + 1);
	} else if (!strcmp(value, "range")) {
		++j;
		this->Range = LuaToNumber(l, -1, j + 1);
		if (unit.Type->BoolFlag[SKIRMISHER_INDEX].value) {
			this->SkirmishRange = this->Range;
		}
	} else if (!strcmp(value, "tile")) {
		++j;
		lua_rawgeti(l, -1, j + 1);
		CclGetPos(l, &this->goalPos.x, &this->goalPos.y);
		lua_pop(l, 1);

	} else if (!strcmp(value, "amove-tile")) {
		++j;
		lua_rawgeti(l, -1, j + 1);
		CclGetPos(l, &this->attackMovePos.x, &this->attackMovePos.y);
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
	PixelPos orderedPos;
	bool isAttackMove = IsAutoTargeting() ? true : false;

	targetPos = this->HasGoal() ? vp.MapToScreenPixelPos(this->GetGoal()->GetMapPixelPosCenter())
								: vp.TilePosToScreen_Center(this->goalPos);

	orderedPos = isAttackMove ? vp.TilePosToScreen_Center(this->attackMovePos)
				 			  : targetPos;

	Uint32 color = isAttackMove ? ColorOrange : ColorRed;
	Video.FillCircleClip(color, lastScreenPos, 2);
	Video.DrawLineClip(ColorRed, lastScreenPos, orderedPos);
	Video.FillCircleClip(color, orderedPos, 3);

	if (isAttackMove && this->HasGoal()) {
		Video.DrawLineClip(ColorOrange, lastScreenPos, targetPos);
		Video.FillCircleClip(ColorOrange, targetPos, 3);
	}

	return isAttackMove ? orderedPos : targetPos;
}

/* virtual */ void COrder_Attack::UpdatePathFinderData(PathFinderInput &input)
{
	Vec2i tileSize;
	if (this->HasGoal() && !(this->State & MOVE_TO_ATTACKPOS)) {
		CUnit *goal = this->GetGoal();
		tileSize.x = goal->Type->TileWidth;
		tileSize.y = goal->Type->TileHeight;
		input.SetGoal(goal->tilePos, tileSize);
	} else {
		tileSize.x = 0;
		tileSize.y = 0;
		input.SetGoal(this->goalPos, tileSize);
	}

	int distance = this->Range;
	if (GameSettings.Inside) {
		CheckObstaclesBetweenTiles(input.GetUnitPos(), this->HasGoal() ? this->GetGoal()->tilePos : this->goalPos, MapFieldRocks | MapFieldForest, &distance);
	}
	input.SetMaxRange(distance);
	if (!this->SkirmishRange || Distance(input.GetUnitPos(), input.GetGoalPos()) < this->SkirmishRange) {
		input.SetMinRange(this->MinRange);
	} else {
		input.SetMinRange(std::max<int>(this->SkirmishRange, this->MinRange));
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


inline bool COrder_Attack::IsWeakTargetSelected() const
{
	return (this->State & AUTO_TARGETING) != 0;
}
inline bool COrder_Attack::IsAutoTargeting() const
{
	return (this->State & AUTO_TARGETING) != 0;
}

/**
**  Check for dead/valid goal.
**
**
**  @todo     If a unit enters an building, than the attack choose an
**            other goal, perhaps it is better to wait for the goal?
**
**  @param unit  Unit using the goal.
**
**  @return      true if target is valid, false else.
*/
bool COrder_Attack::CheckIfGoalValid(CUnit &unit)
{
	CUnit *goal = this->GetGoal();

	// Position or valid target, it is ok.
	if (!goal || goal->IsVisibleAsGoal(*unit.Player)) {
		return true;
	}

	// Goal could be destroyed or unseen
	// So, cannot use type.
	this->goalPos = goal->tilePos;
	this->MinRange = 0;
	this->Range = 0;
	this->ClearGoal();
	return false;
}

/**
**  Turn unit to Target or position on map for attack.
**
**  @param unit    Unit to turn.
**  @param target  Turn to this Target. If NULL then turn to goalPos.
**
*/
void COrder_Attack::TurnToTarget(CUnit &unit, const CUnit *target)
{
	const Vec2i dir = target ? (target->tilePos + target->Type->GetHalfTileSize() - unit.tilePos)
					  		 : (this->goalPos - unit.tilePos);
	const unsigned char oldDir = unit.Direction;

	UnitHeadingFromDeltaXY(unit, dir);
	if (unit.Type->BoolFlag[SIDEATTACK_INDEX].value) {
		unsigned char leftTurn = (unit.Direction - 2 * NextDirection) % (NextDirection * 8);
		unsigned char rightTurn = (unit.Direction + 2 * NextDirection) % (NextDirection * 8);
		if (abs(leftTurn - oldDir) < abs(rightTurn - oldDir)) {
			unit.Direction = leftTurn;
		} else {
			unit.Direction = rightTurn;
		}
		UnitUpdateHeading(unit);
	}
}

/**
**  Set target for attack in auto-attack mode.
**  Also if there is no active target Attack-Move action will be saved.
**
**  @param unit    Attacker.
**  @param target  Turn to this Target. If NULL then turn to goalPos.
**
*/
void COrder_Attack::SetAutoTarget(CUnit &unit, CUnit *target)
{
	if (this->HasGoal()) {
		this->ClearGoal();
	}
	this->SetGoal(target);
	this->goalPos 			= target->tilePos;
	this->Range 			= unit.Stats->Variables[ATTACKRANGE_INDEX].Max;
	this->MinRange 			= unit.Type->MinAttackRange;
	if (unit.Type->BoolFlag[SKIRMISHER_INDEX].value) {
		this->SkirmishRange = this->Range;
	}
}

/**
** Select target in auto attack mode
**
** return true if we have a target, false if can't find any
**/
bool COrder_Attack::AutoSelectTarget(CUnit &unit)
{
	// if unit can't attack, or if unit is not bunkered and removed - exit, no targets
	if (unit.Type->CanAttack == false
		|| (unit.Removed
			&& (unit.Container == NULL || unit.Container->Type->BoolFlag[ATTACKFROMTRANSPORTER_INDEX].value == false))) {
		return false;
	}
	CUnit *goal = this->GetGoal();
	CUnit *newTarget = NULL;

	/// if attacker cant't move (stand_ground, building, in a bunker or transport)
	const bool immobile = (this->Action == UnitActionStandGround || unit.Removed || !unit.CanMove()) ? true : false;

	if (immobile) {
		newTarget = AttackUnitsInRange(unit); // search for enemies only in attack range
	} else {
		newTarget = AttackUnitsInReactRange(unit); // search for enemies in reaction range
	}

	if (goal /// if goal is Valid
		&& goal->IsVisibleAsGoal(*unit.Player)
		&& CanTarget(*unit.Type, *goal->Type)
		&& (immobile ? InAttackRange(unit, *goal) : InReactRange(unit, *goal))) {

		if (newTarget && newTarget != goal) {
			if (Preference.SimplifiedAutoTargeting) {
				const int goal_priority			= TargetPriorityCalculate(&unit, goal);
				const int newTarget_priority 	= TargetPriorityCalculate(&unit, newTarget);

				if ((newTarget_priority & AT_PRIORITY_MASK_HI) > (goal_priority & AT_PRIORITY_MASK_HI)) {
					SetAutoTarget(unit, newTarget);
				} else if (!immobile
						   && (!InAttackRange(unit, *goal) && newTarget_priority > goal_priority)) {
					SetAutoTarget(unit, newTarget);
				}
			} else {
				if (ThreatCalculate(unit, *newTarget) < ThreatCalculate(unit, *goal)) {
					SetAutoTarget(unit, newTarget);
				}
			}
		}
	} else {
		if (goal) {
			this->ClearGoal();
		}
		if (newTarget) {
			SetAutoTarget(unit, newTarget);
		} else {
			return false;
		}
	}
	return true;
}

/**
**  Restore action/order when current action is finished
**
**  @param unit
**  @param canBeFinished    False if ony restore order/action needed
**
**  @return      			false if order/action restored, true else (if order finished).
*/
bool COrder_Attack::EndActionAttack(CUnit &unit, const bool canBeFinished = true)
{
	if (!unit.RestoreOrder()) {
		if (IsAutoTargeting() && this->goalPos != this->attackMovePos) {
			this->goalPos 	= this->attackMovePos;
			this->Range 	= 0;
			this->MinRange 	= 0;
			this->State 	= AUTO_TARGETING;
			return false;
		}
		this->Finished 		= canBeFinished ? true : false;
		return true;
	}
	return false;
}

/**
**  Move unit to randomly selected position in MinAttackRange away from current goal.
**
**  @param unit  Unit ot move
*/
void COrder_Attack::MoveToBetterPos(CUnit &unit)
{
	Assert(this->HasGoal());

	CUnit *goal 	= this->GetGoal();
	this->goalPos 	= PosToRetreat(unit, *goal, unit.Type->MinAttackRange + 1);
	this->Range		= 0;
	this->MinRange 	= 0;
	unit.Frame 	  	= 0;
	this->State  	&= AUTO_TARGETING;
	this->State  	|= MOVE_TO_ATTACKPOS;
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

	if (!this->HasGoal() && (this->Action == UnitActionAttackGround || Map.WallOnMap(this->goalPos))) {
		return false;
	}

	if (!CheckIfGoalValid(unit)) {
		EndActionAttack(unit);
		return true;
	}

	if (IsAutoTargeting() || unit.Player->AiEnabled) {
		const bool hadGoal = this->HasGoal();
		if (!AutoSelectTarget(unit) && hadGoal) {
			EndActionAttack(unit, RESTORE_ONLY);
			return true;
		}
	}

	Assert(!unit.Type->BoolFlag[VANISHES_INDEX].value && !unit.Destroyed && !unit.Removed);
	return false;
}

/**
**  Controls moving a unit to its target when attacking
**
**	@todo FIXME: add move to better pos for Attack_Ground when target tile is too close
**
**  @param unit  Unit that is attacking and moving
*/
void COrder_Attack::MoveToTarget(CUnit &unit)
{
	Assert(!unit.Type->BoolFlag[VANISHES_INDEX].value && !unit.Destroyed && !unit.Removed);
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
		if (InAttackRange(unit, this->goalPos)) {
			err = PF_REACHED;
		}
	}

	CUnit *goal = this->GetGoal();
	const int distance 	= this->HasGoal() ? unit.MapDistanceTo(*goal) : 0;
	const bool tooClose = (distance && (distance < unit.Type->MinAttackRange)) ? true : false;

	// Waiting or on the way
	if (err >= 0) {
		bool targetChanged = false;
		if (!CheckForTargetInRange(unit)) {

			CUnit *currGoal = this->GetGoal();
			if (currGoal && goal != currGoal) {
				targetChanged = true;
			}

			if ((IsAutoTargeting() && targetChanged) || (this->State & MOVE_TO_ATTACKPOS)) {

				if (InAttackRange(unit, *currGoal)) {
					this->goalPos = currGoal->tilePos; // We have to restore in case of MOVE_TO_ATTACKPOS
					TurnToTarget(unit, currGoal);
					this->State &= AUTO_TARGETING;
					this->State |= ATTACK_TARGET;
				} else if (targetChanged) {
					unit.Frame	= 0;
					this->State &= AUTO_TARGETING;
					this->State |= MOVE_TO_TARGET;
				}
			}
		}
		return;
	}
	if (err == PF_REACHED) {
		// Have reached target? FIXME: could use the new return code?
		if (goal && InAttackRange(unit, *goal)) {
			// Reached another unit, now attacking it
			TurnToTarget(unit, goal);
			this->State &= AUTO_TARGETING;
			this->State |= ATTACK_TARGET;
			return;
		} else if (tooClose) {
			MoveToBetterPos(unit);
			return;
		}
		// Attacking wall or ground.
		if (((goal && goal->Type && goal->Type->BoolFlag[WALL_INDEX].value)
			 || (!goal && (Map.WallOnMap(this->goalPos) || this->Action == UnitActionAttackGround)))
			&& InAttackRange(unit, this->goalPos)) {

			// Reached wall or ground, now attacking it
			TurnToTarget(unit, NULL);
			this->State &= AUTO_TARGETING;
			this->State |= ATTACK_TARGET;
			return;
		}
	}
	// Unreachable.
	// FIXME: look at this
	if (err == PF_UNREACHABLE) {
		if (!this->HasGoal()) {
			// When attack-moving we have to allow a bigger range (PF)
			this->Range++;
			unit.Wait = 5;
			return;
		} else if (tooClose) {
			MoveToBetterPos(unit);
			return;
		}
		this->ClearGoal();
	}
	EndActionAttack(unit);
	return;
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
	/// Order may be set as finished by outside code while playing attack animation.
	/// In this case we must not execute code of AttackTarget
	if (unit.Anim.Unbreakable || this->Finished) {
		return;
	}
	if (!this->HasGoal() && (this->Action == UnitActionAttackGround || Map.WallOnMap(this->goalPos))) {
		return;
	}

	if (!CheckIfGoalValid(unit)) {
		EndActionAttack(unit);
		return;
	}

	if (IsAutoTargeting() || unit.Player->AiEnabled) {
		if (!AutoSelectTarget(unit)) {
			EndActionAttack(unit, RESTORE_ONLY);
			return;
		}
	}
	CUnit *goal = this->GetGoal();
	if (!InAttackRange(unit, *goal)) {
		unit.Frame 	= 0;
		this->State &= AUTO_TARGETING;
		this->State |= MOVE_TO_TARGET;
	}
	TurnToTarget(unit, goal);
}

/**
**  Unit attacks!
**
**  if (SubAction & AUTO_TARGETING) is true the goal is a weak goal.
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
		if (!unit.Waiting) {
			unit.Waiting = 1;
			unit.WaitBackup = unit.Anim;
		}
		UnitShowAnimation(unit, unit.Type->Animations->Still);
		unit.Wait--;
		return;
	}
	if (unit.Waiting) {
		unit.Anim = unit.WaitBackup;
		unit.Waiting = 0;
	}

	if (this->State != ATTACK_TARGET && unit.CanStoreOrder(this) && AutoCast(unit)) {
		this->Finished = true;
		return;
	}

	switch (this->State) {
		case FIRST_ENTRY:
		case AUTO_TARGETING:

			if (CheckForTargetInRange(unit)) {
				return;
			}
			// Can we already attack ?
			if (this->GetGoal()) {
				CUnit *goal = this->GetGoal();
				if (InAttackRange(unit, *goal)) {
					TurnToTarget(unit, goal);
					this->State |= ATTACK_TARGET;
					AttackTarget(unit);
					return;
				}
			}
			this->State |= MOVE_TO_TARGET;
		// FIXME: should use a reachable place to reduce pathfinder time.

		// FALL THROUGH
		case MOVE_TO_TARGET:
		case MOVE_TO_TARGET + AUTO_TARGETING:
		case MOVE_TO_ATTACKPOS:
		case MOVE_TO_ATTACKPOS + AUTO_TARGETING:
			if (!unit.CanMove()) {
				this->Finished = true;
				return;
			}
			MoveToTarget(unit);
			break;

		case ATTACK_TARGET:
		case ATTACK_TARGET + AUTO_TARGETING:
			AttackTarget(unit);
			break;
	}
}

//@}
