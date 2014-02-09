//     ____                _       __               
//    / __ )____  _____   | |     / /___ ___________
//   / __  / __ \/ ___/   | | /| / / __ `/ ___/ ___/
//  / /_/ / /_/ (__  )    | |/ |/ / /_/ / /  (__  ) 
// /_____/\____/____/     |__/|__/\__,_/_/  /____/  
//                                              
//       A futuristic real-time strategy game.
//          This file is part of Bos Wars.
//
/**@name action_still.cpp - The stand still action. */
//
//      (c) Copyright 1998-2008 by Lutz Sammer and Jimmy Salmon
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
#include "actions.h"
#include "unit.h"
#include "unit_cache.h"
#include "map.h"
#include "pathfinder.h"
#include "spells.h"
#include "player.h"

/*----------------------------------------------------------------------------
--  Functions
----------------------------------------------------------------------------*/

/**
**  Auto cast a spell if possible
**
**  @return  true if a spell was auto cast, false otherwise
*/
static bool AutoCast(CUnit *unit)
{
	if (unit->AutoCastSpell) {
		for (int i = 0; i < (int)SpellTypeTable.size(); ++i) {
			if (unit->AutoCastSpell[i] && AutoCastSpell(unit, SpellTypeTable[i])) {
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
static CUnit *UnitToRepairInRange(CUnit *unit, int range)
{
	CUnit *table[UnitMax];
	int n;

	n = UnitCache.Select(unit->X - range, unit->Y - range,
		unit->X + unit->Type->TileWidth + range,
		unit->Y + unit->Type->TileHeight + range,
		table, UnitMax);
	for (int i = 0; i < n; ++i) {
		if (table[i]->IsTeamed(unit) &&
				table[i]->Type->RepairHP &&
				table[i]->Variable[HP_INDEX].Value < table[i]->Variable[HP_INDEX].Max &&
				table[i]->IsVisibleAsGoal(unit->Player)) {
			return table[i];
		}
	}
	return NoUnitP;
}

/**
**  Auto repair a unit if possible
**
**  @return  true if the unit is repairing, false otherwise
*/
bool AutoRepair(CUnit *unit)
{
	if (unit->AutoRepair && unit->Type->Variable[AUTOREPAIRRANGE_INDEX].Value) {
		CUnit *repairedUnit = UnitToRepairInRange(unit,
			unit->Type->Variable[AUTOREPAIRRANGE_INDEX].Value);
		if (repairedUnit != NoUnitP) {
			COrder order = *unit->Orders[0];
			CommandRepair(unit, -1, -1, repairedUnit, FlushCommands);
			unit->SavedOrder = order;
			return true;
		}
	}
	return false;
}

/**
**  Auto attack nearby units if possible
*/
static void AutoAttack(CUnit *unit, bool stand_ground)
{
	CUnit *temp;
	CUnit *goal;

	if (unit->Wait) {
		unit->Wait--;
		return;
	}

	// Cowards don't attack unless ordered.
	if (unit->Type->CanAttack && !unit->Type->Coward) {
		// Normal units react in reaction range.
		if (CanMove(unit) && !unit->Removed && !stand_ground) {
			if ((goal = AttackUnitsInReactRange(unit))) {
				// Weak goal, can choose other unit, come back after attack
				CommandAttack(unit, goal->X, goal->Y, NULL, FlushCommands);
				Assert(unit->SavedOrder.Action == UnitActionStill);
				Assert(!unit->SavedOrder.Goal);
				unit->SavedOrder.Action = UnitActionAttack;
				unit->SavedOrder.Range = 0;
				unit->SavedOrder.X = unit->X;
				unit->SavedOrder.Y = unit->Y;
				unit->SavedOrder.Goal = NoUnitP;
			} else {
				unit->Wait = 15;
			}
		// Removed units can only attack in AttackRange, from bunker
		} else if ((goal = AttackUnitsInRange(unit))) {
			temp = unit->Orders[0]->Goal;
			if (temp && temp->Orders[0]->Action == UnitActionDie) {
				temp->RefsDecrease();
				unit->Orders[0]->Goal = temp = NoUnitP;
			}
			if (!unit->SubAction || temp != goal) {
				// New target.
				if (temp) {
					temp->RefsDecrease();
				}
				unit->Orders[0]->Goal = goal;
				goal->RefsIncrease();
				unit->State = 0;
				unit->SubAction = 1; // Mark attacking.
				UnitHeadingFromDeltaXY(unit,
					goal->X + (goal->Type->TileWidth - 1) / 2 - unit->X,
					goal->Y + (goal->Type->TileHeight - 1) / 2 - unit->Y);
			}
			return;
		}
	} else {
		unit->Wait = 15;
	}

	if (unit->SubAction) { // was attacking.
		if ((temp = unit->Orders[0]->Goal)) {
			temp->RefsDecrease();
			unit->Orders[0]->Goal = NoUnitP;
		}
		unit->SubAction = unit->State = 0; // No attacking, restart
	}
	Assert(!unit->Orders[0]->Goal);
}

/**
**  Unit stands still or stand ground.
**
**  @param unit          Unit pointer for action.
**  @param stand_ground  true if unit is standing ground.
*/
void ActionStillGeneric(CUnit *unit, bool stand_ground)
{
	// If unit is not bunkered and removed, wait
	if (unit->Removed && (!unit->Container ||
			!unit->Container->Type->CanTransport ||
			!unit->Container->Type->AttackFromTransporter ||
			unit->Type->Missile.Missile->Class == MissileClassNone)) {
		// If unit is in building or transporter it is removed.
		return;
	}

	// Animations
	if (unit->SubAction) { // attacking unit in attack range.
		AnimateActionAttack(unit);
	} else {
		UnitShowAnimation(unit, unit->Type->Animations->Still);
	}

	if (unit->Anim.Unbreakable) { // animation can't be aborted here
		return;
	}

	if (AutoCast(unit) || AutoRepair(unit)) {
		return;
	}

	AutoAttack(unit, stand_ground);
}

/**
**  Unit stands still!
**
**  @param unit  Unit pointer for still action.
*/
void HandleActionStill(CUnit *unit)
{
	ActionStillGeneric(unit, false);
}

//@}
