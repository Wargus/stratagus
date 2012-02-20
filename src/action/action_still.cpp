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
/**@name action_still.cpp - The stand still action. */
//
//      (c) Copyright 1998-2006 by Lutz Sammer and Jimmy Salmon
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

/*----------------------------------------------------------------------------
--  Includes
----------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>

#include "stratagus.h"
#include "missile.h"
#include "unittype.h"
#include "animation.h"
#include "unit.h"
#include "actions.h"
#include "tileset.h"
#include "map.h"
#include "pathfinder.h"
#include "spells.h"
#include "player.h"

#define SUB_STILL_INIT		0
#define SUB_STILL_STANDBY	1
#define SUB_STILL_ATTACK	2

/*----------------------------------------------------------------------------
--  Functions
----------------------------------------------------------------------------*/

static void MapMarkTileGuard(const CPlayer &player, const unsigned int index)
{
	++Map.Field(index)->Guard[player.Index];
}


static void MapUnmarkTileGuard(const CPlayer &player, const unsigned int index)
{
	Assert(Map.Field(index)->Guard[player.Index] > 0);
	--Map.Field(index)->Guard[player.Index];
}

void MapMarkUnitGuard(CUnit &unit)
{
	if (unit.IsAgressive() && !unit.GuardLock) {
		if (!unit.Removed) {
			unit.GuardLock = 1;
			MapSight(*unit.Player, unit.tilePos,
				unit.Type->TileWidth, unit.Type->TileHeight,
				unit.GetReactRange(),
				MapMarkTileGuard);
		} else {
			CUnit *c = unit.Container;
			if (c && c->Type->AttackFromTransporter) {
				unit.GuardLock = 1;
				MapSight(*unit.Player, c->tilePos,
					c->Type->TileWidth, c->Type->TileHeight,
					unit.GetReactRange(), MapMarkTileGuard);
			}
		}
	}
}

void MapUnmarkUnitGuard(CUnit &unit)
{
	if (unit.IsAgressive() && unit.GuardLock) {
		if (!unit.Removed) {
			unit.GuardLock = 0;
			MapSight(*unit.Player, unit.tilePos,
				unit.Type->TileWidth, unit.Type->TileHeight,
				unit.GetReactRange(), MapUnmarkTileGuard);
		} else {
			CUnit *c = unit.Container;
			if (c && c->Type->AttackFromTransporter) {
				unit.GuardLock = 0;
				MapSight(*unit.Player, c->tilePos,
					c->Type->TileWidth, c->Type->TileHeight,
					unit.GetReactRange(), MapUnmarkTileGuard);
			}
		}
	}
}

void UnHideUnit(CUnit &unit)
{
	const int action = unit.CurrentAction();
	const bool mark_guard = (action == UnitActionStill ||
						action == UnitActionStandGround) &&
						unit.Variable[INVISIBLE_INDEX].Value > 0;
	unit.Variable[INVISIBLE_INDEX].Value = 0;
	if (mark_guard)
	{
		MapMarkUnitGuard(unit);
	}
}

/**
**  Move in a random direction
**
**  @return  true if the unit moves, false otherwise
*/
static bool MoveRandomly(CUnit &unit)
{
	if (unit.Type->RandomMovementProbability &&
			((SyncRand() % 100) <= unit.Type->RandomMovementProbability)) {
		// pick random location
		Vec2i pos = unit.tilePos;

		switch ((SyncRand() >> 12) & 15) {
			case 0: pos.x++; break;
			case 1: pos.y++; break;
			case 2: pos.x--; break;
			case 3: pos.y--; break;
			case 4: pos.x++; pos.y++; break;
			case 5: pos.x--; pos.y++; break;
			case 6: pos.y--; pos.x++; break;
			case 7: pos.x--; pos.y--; break;
			default:
				break;
		}

		// restrict to map
		if (pos.x < 0) {
			pos.x = 0;
		} else if (pos.x >= Map.Info.MapWidth) {
			pos.x = Map.Info.MapWidth - 1;
		}
		if (pos.y < 0) {
			pos.y = 0;
		} else if (pos.y >= Map.Info.MapHeight) {
			pos.y = Map.Info.MapHeight - 1;
		}

		// move if possible
		if (pos != unit.tilePos) {
			UnmarkUnitFieldFlags(unit);
			if (UnitCanBeAt(unit, pos)) {
				COrderPtr order = unit.CurrentOrder();
				// FIXME: Don't use pathfinder for this, costs too much cpu.
				order->Action = UnitActionMove;
				Assert(!order->HasGoal());
				order->ClearGoal();
				order->Range = 0;
				order->goalPos = pos;
				unit.State = 0;
				MapUnmarkUnitGuard(unit);
				//return true;//TESTME: new localization
			}
			MarkUnitFieldFlags(unit);
		}
		return true;//TESTME: old localization
	}
	return false;
}

/**
**  Auto cast a spell if possible
**
**  @return  true if a spell was auto cast, false otherwise
*/
bool AutoCast(CUnit &unit)
{
	if (unit.AutoCastSpell && !unit.Removed) { // Removed units can't cast any spells, from bunker)
		for (unsigned int i = 0; i < SpellTypeTable.size(); ++i) {
			if (unit.AutoCastSpell[i] &&
				(SpellTypeTable[i]->AutoCast || SpellTypeTable[i]->AICast) &&
					AutoCastSpell(unit, SpellTypeTable[i])) {
				return true;
			}
		}
	}
	return false;
}

/**
**  Try to find a repairable unit around and return it.
**
**  @param unit   unit which can repair.
**  @param range  range to find a repairable unit.
**
**  @return       unit to repair if found, NoUnitP otherwise
**
**  @todo         FIXME: find the best unit (most damaged, ...).
*/
static CUnit *UnitToRepairInRange(const CUnit &unit, int range)
{
	CUnit *table[UnitMax];
	const int n = Map.Select(unit.tilePos.x - range, unit.tilePos.y - range,
		unit.tilePos.x + unit.Type->TileWidth + range,
		unit.tilePos.y + unit.Type->TileHeight + range,
		table);
	for (int i = 0; i < n; ++i) {
		CUnit &candidate = *table[i];

		if (candidate.IsTeamed(unit)
			&& candidate.Type->RepairHP
			&& candidate.Variable[HP_INDEX].Value < candidate.Variable[HP_INDEX].Max
			&& candidate.IsVisibleAsGoal(*unit.Player)) {
			return &candidate;
		}
	}
	return NoUnitP;
}

/**
**  Auto repair a unit if possible
**
**  @return  true if the unit is repairing, false otherwise
*/
bool AutoRepair(CUnit &unit)
{
	const int repairRange = unit.Type->Variable[AUTOREPAIRRANGE_INDEX].Value;

	if (unit.AutoRepair && repairRange) {
		CUnit *repairedUnit = UnitToRepairInRange(unit, repairRange);

		if (repairedUnit != NoUnitP) {
			const Vec2i invalidPos = {-1, -1};
			COrder *savedOrder = unit.CurrentOrder()->Clone();

			//Command* will clear unit.SavedOrder
			CommandRepair(unit, invalidPos, repairedUnit, FlushCommands);
			if (unit.StoreOrder(savedOrder) == false) {
				delete savedOrder;
				savedOrder = NULL;
			}
			return true;
		}
	}
	return false;
}

/**
**  Auto attack nearby units if possible
*/
static bool AutoAttack(CUnit &unit, bool stand_ground)
{
	CUnit *goal;

	// Cowards and invisible units don't attack unless ordered.
	if (unit.IsAgressive()) {
		// Normal units react in reaction range.
		if (!stand_ground && !unit.Removed && unit.CanMove()) {
			if ((goal = AttackUnitsInReactRange(unit))) {
				// Weak goal, can choose other unit, come back after attack
				CommandAttack(unit, goal->tilePos, NULL, FlushCommands);
				COrder *savedOrder = COrder::NewActionAttack(unit, unit.tilePos);

				if (unit.StoreOrder(savedOrder) == false) {
					delete savedOrder;
				}
				return true;
			}
		// Removed units can only attack in AttackRange, from bunker
		} else {
			if ((goal = AttackUnitsInRange(unit))) {
				CUnit *temp = unit.CurrentOrder()->GetGoal();
				if (temp && temp->CurrentAction() == UnitActionDie) {
					unit.CurrentOrder()->ClearGoal();
					temp = NoUnitP;
				}
				if (unit.SubAction < SUB_STILL_ATTACK || temp != goal) {
					// New target.
					unit.CurrentOrder()->SetGoal(goal);
					unit.State = 0;
					unit.SubAction = SUB_STILL_ATTACK; // Mark attacking.
					UnitHeadingFromDeltaXY(unit, goal->tilePos + goal->Type->GetHalfTileSize() - unit.tilePos);
				}
				return true;
			}
		}
	}
	return false;
}

void AutoAttack(CUnit &unit, CUnitCache &targets, bool stand_ground)
{
	// Cowards and invisible units don't attack unless ordered.
	if (unit.IsAgressive()) {
		// Normal units react in reaction range.
		if (!stand_ground && !unit.Removed && unit.CanMove()) {
			CUnit *goal = AutoAttackUnitsInDistance(unit, unit.GetReactRange(), targets);

			if (goal) {
				// Weak goal, can choose other unit, come back after attack
				CommandAttack(unit, goal->tilePos, NULL, FlushCommands);
				COrder *savedOrder = COrder::NewActionAttack(unit, unit.tilePos);

				if (unit.StoreOrder(savedOrder) == false) {
					delete savedOrder;
				}
			}
		// Removed units can only attack in AttackRange, from bunker
		} else {
			CUnit *goal = AutoAttackUnitsInDistance(unit, unit.Stats->Variables[ATTACKRANGE_INDEX].Max, targets);

			if (goal) {
				CUnit *temp = unit.CurrentOrder()->GetGoal();
				if (temp && temp->CurrentAction() == UnitActionDie) {
					unit.CurrentOrder()->ClearGoal();
					temp = NoUnitP;
				}
				if (unit.SubAction < SUB_STILL_ATTACK || temp != goal) {
					// New target.
					unit.CurrentOrder()->SetGoal(goal);
					unit.State = 0;
					unit.SubAction = SUB_STILL_ATTACK; // Mark attacking.
					UnitHeadingFromDeltaXY(unit, goal->tilePos + goal->Type->GetHalfTileSize() - unit.tilePos);
				}
			}
		}
	}
}



/**
**  Unit stands still or stand ground.
**
**  @param unit          Unit pointer for action.
**  @param stand_ground  true if unit is standing ground.
*/
void ActionStillGeneric(CUnit &unit, bool stand_ground)
{
	// If unit is not bunkered and removed, wait
	if (unit.Removed
		&& (!unit.Container
			|| !unit.Container->Type->CanTransport()
			|| !unit.Container->Type->AttackFromTransporter
			|| unit.Type->Missile.Missile->Class == MissileClassNone)) {
		// If unit is in building or transporter it is removed.
		return;
	}

	switch (unit.SubAction)
	{
		case SUB_STILL_INIT: //first entry
			MapMarkUnitGuard(unit);
			unit.SubAction = SUB_STILL_STANDBY;
			// no break : follow
		case SUB_STILL_STANDBY:
			UnitShowAnimation(unit, unit.Type->Animations->Still);
		break;
		case SUB_STILL_ATTACK: // attacking unit in attack range.
			AnimateActionAttack(unit);
		break;
	}

	if (unit.Anim.Unbreakable) { // animation can't be aborted here
		return;
	}

	if (AutoAttack(unit, stand_ground)
		|| AutoCast(unit)
		|| AutoRepair(unit)
		|| MoveRandomly(unit)) {
		return;
	}
}

/**
**  Unit stands still!
**
**  @param unit  Unit pointer for still action.
*/
void HandleActionStill(COrder& /*order*/, CUnit &unit)
{
	ActionStillGeneric(unit, false);
}

//@}
