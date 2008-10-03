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
//      $Id$

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

static void MapMarkTileGuard(const CPlayer *player,
	 const unsigned int index)
{
	++Map.Field(index)->Guard[player->Index];
}


static void MapUnmarkTileGuard(const CPlayer *player,
	 const unsigned int index)
{
	Assert(Map.Field(index)->Guard[player->Index] > 0);
	--Map.Field(index)->Guard[player->Index];
}

static void MapMarkUnitGuard(CUnit *unit)
{
	if (unit->IsAgressive() && !unit->GuardLock) {
		if (!unit->Removed) {
			unit->GuardLock = 1;
			MapSight(unit->Player, unit->X, unit->Y,
			 	unit->Type->TileWidth, unit->Type->TileHeight,
			 	unit->GetReactRange(),
			 	MapMarkTileGuard);		
		} else {
			CUnit *c = unit->Container;
			if (c && c->Type->AttackFromTransporter) {
				unit->GuardLock = 1;
				MapSight(unit->Player, c->X, c->Y,
			 		c->Type->TileWidth, c->Type->TileHeight,
			 		unit->GetReactRange(),	MapMarkTileGuard);			
			}
		}	 	
	}
}

void MapUnmarkUnitGuard(CUnit *unit)
{
	if (unit->IsAgressive() && unit->GuardLock) {
		if (!unit->Removed) {
			unit->GuardLock = 0;
			MapSight(unit->Player, unit->X, unit->Y,
			 	unit->Type->TileWidth, unit->Type->TileHeight,
			 	unit->GetReactRange(), MapUnmarkTileGuard);		
		} else {
			CUnit *c = unit->Container;
			if (c && c->Type->AttackFromTransporter) {
				unit->GuardLock = 0;
				MapSight(unit->Player, c->X, c->Y,
			 		c->Type->TileWidth, c->Type->TileHeight,
			 		unit->GetReactRange(), MapUnmarkTileGuard);			
			}
		}
	}
}

void UnHideUnit(CUnit *unit)
{
	const int action = unit->CurrentAction();
	const bool mark_guard = (action == UnitActionStill || 
						action == UnitActionStandGround) && 
						unit->Variable[INVISIBLE_INDEX].Value > 0;
	unit->Variable[INVISIBLE_INDEX].Value = 0;
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
static bool MoveRandomly(CUnit *unit)
{
	if (unit->Type->RandomMovementProbability &&
			((SyncRand() % 100) <= unit->Type->RandomMovementProbability)) {
		// pick random location
		int x = unit->X;
		int y = unit->Y;
		switch ((SyncRand() >> 12) & 15) {
			case 0: x++; break;
			case 1: y++; break;
			case 2: x--; break;
			case 3: y--; break;
			case 4: x++; y++; break;
			case 5: x--; y++; break;
			case 6: y--; x++; break;
			case 7: x--; y--; break;
			default:
				break;
		}

		// restrict to map
		if (x < 0) {
			x = 0;
		} else if (x >= Map.Info.MapWidth) {
			x = Map.Info.MapWidth - 1;
		}
		if (y < 0) {
			y = 0;
		} else if (y >= Map.Info.MapHeight) {
			y = Map.Info.MapHeight - 1;
		}

		// move if possible
		if (x != unit->X || y != unit->Y) {
			UnmarkUnitFieldFlags(unit);
			if (UnitCanBeAt(unit, x, y)) {
				COrderPtr order = unit->CurrentOrder();
				// FIXME: Don't use pathfinder for this, costs too much cpu.
				order->Action = UnitActionMove;
				Assert(!order->HasGoal());
				order->ClearGoal();
				order->Range = 0;
				order->X = x;
				order->Y = y;
				unit->State = 0;
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
bool AutoCast(CUnit *unit)
{
	if (unit->AutoCastSpell && !unit->Removed) { // Removed units can't cast any spells, from bunker)
		for (int i = 0; i < (int)SpellTypeTable.size(); ++i) {
			if (unit->AutoCastSpell[i] && 
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
static CUnit *UnitToRepairInRange(CUnit *unit, int range)
{
	CUnit *table[UnitMax];
	int n = Map.Select(unit->X - range, unit->Y - range,
		unit->X + unit->Type->TileWidth + range,
		unit->Y + unit->Type->TileHeight + range,
		table);
	for (int i = 0; i < n; ++i) {
		CUnit *candidate = table[i];
		if (candidate->IsTeamed(unit) &&
				candidate->Type->RepairHP &&
				candidate->Variable[HP_INDEX].Value < candidate->Variable[HP_INDEX].Max &&
				candidate->IsVisibleAsGoal(unit->Player)) {
			return candidate;
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
			CUnit::COrder order;
			order = *unit->CurrentOrder();
			//Command* will clear unit->SavedOrder
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
static bool AutoAttack(CUnit *unit, bool stand_ground)
{
	CUnit *goal;

	// Cowards and invisible units don't attack unless ordered.
	if (unit->IsAgressive()) {
		// Normal units react in reaction range.
		if (!stand_ground && !unit->Removed && unit->CanMove()) {
			if ((goal = AttackUnitsInReactRange(unit))) {
				// Weak goal, can choose other unit, come back after attack
				CommandAttack(unit, goal->X, goal->Y, NULL, FlushCommands);
				Assert(unit->SavedOrder.Action == UnitActionStill);
				Assert(!unit->SavedOrder.HasGoal());
				unit->SavedOrder.Action = UnitActionAttack;
				unit->SavedOrder.Range = 0;
				unit->SavedOrder.X = unit->X;
				unit->SavedOrder.Y = unit->Y;
				unit->SavedOrder.ClearGoal();
				return true;
			} //else {
				//unit->Wait = 15;
			//}
		// Removed units can only attack in AttackRange, from bunker
		} else  {
			if ((goal = AttackUnitsInRange(unit))) {
				CUnit *temp = unit->CurrentOrder()->GetGoal();
				if (temp && temp->CurrentAction() == UnitActionDie) {
					unit->CurrentOrder()->ClearGoal();
					temp = NoUnitP;
				}
				if (unit->SubAction < SUB_STILL_ATTACK || temp != goal) {
					// New target.
					unit->CurrentOrder()->SetGoal(goal);
					unit->State = 0;
					unit->SubAction = SUB_STILL_ATTACK; // Mark attacking.
					UnitHeadingFromDeltaXY(unit,
						goal->X + (goal->Type->TileWidth - 1) / 2 - unit->X,
						goal->Y + (goal->Type->TileHeight - 1) / 2 - unit->Y);
				}
				return true;
			}
		}
	} //else {
		//unit->Wait = 15;
	//}
	return false;
}

void AutoAttack(CUnit *unit, CUnitCache &targets, bool stand_ground)
{
	CUnit *goal;

	// Cowards and invisible units don't attack unless ordered.
	if (unit->IsAgressive()) {
		// Normal units react in reaction range.
		if (!stand_ground && !unit->Removed && unit->CanMove()) {
			if ((goal = AutoAttackUnitsInDistance(unit,
			   	 unit->GetReactRange(), targets))) {
				// Weak goal, can choose other unit, come back after attack
				CommandAttack(unit, goal->X, goal->Y, NULL, FlushCommands);
				Assert(unit->SavedOrder.Action == UnitActionStill);
				Assert(!unit->SavedOrder.HasGoal());
				unit->SavedOrder.Action = UnitActionAttack;
				unit->SavedOrder.Range = 0;
				unit->SavedOrder.X = unit->X;
				unit->SavedOrder.Y = unit->Y;
				unit->SavedOrder.ClearGoal();
			} //else {
				//unit->Wait = 15;
			//}
		// Removed units can only attack in AttackRange, from bunker
		} else  {
			if ((goal = AutoAttackUnitsInDistance(unit, 
					unit->Stats->Variables[ATTACKRANGE_INDEX].Max, targets))) {
				CUnit *temp = unit->CurrentOrder()->GetGoal();
				if (temp && temp->CurrentAction() == UnitActionDie) {
					unit->CurrentOrder()->ClearGoal();
					temp = NoUnitP;
				}
				if (unit->SubAction < SUB_STILL_ATTACK || temp != goal) {
					// New target.
					unit->CurrentOrder()->SetGoal(goal);
					unit->State = 0;
					unit->SubAction = SUB_STILL_ATTACK; // Mark attacking.
					UnitHeadingFromDeltaXY(unit,
						goal->X + (goal->Type->TileWidth - 1) / 2 - unit->X,
						goal->Y + (goal->Type->TileHeight - 1) / 2 - unit->Y);
				}
				//return;
			}
		}
	} //else {
		//unit->Wait = 15;
	//}

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
			!unit->Container->Type->CanTransport() ||
			!unit->Container->Type->AttackFromTransporter ||
			unit->Type->Missile.Missile->Class == MissileClassNone)) {
		// If unit is in building or transporter it is removed.
		return;
	}
	bool first_entrly = false;
//	if (unit->Anim.Unbreakable) { // animation can't be aborted here
//		return;
//	}

	switch(unit->SubAction)
	{
		case SUB_STILL_INIT:
			//first entry
			MapMarkUnitGuard(unit);
			unit->SubAction = SUB_STILL_STANDBY;
			first_entrly = true;
		case SUB_STILL_STANDBY:	
			UnitShowAnimation(unit, unit->Type->Animations->Still);
		break;
		case SUB_STILL_ATTACK: // attacking unit in attack range.
			AnimateActionAttack(unit);
		break;
	}

	if (unit->Anim.Unbreakable) { // animation can't be aborted here
		return;
	}

	if (first_entrly && AutoAttack(unit, stand_ground)) { 
		// during first entry make autoattack test 
		//and attack units in attack range.
		return;
	} else {
		if (unit->SubAction > SUB_STILL_STANDBY) { // is attacking.
			CUnit *temp = unit->CurrentOrder()->GetGoal();
			//check if we still can fighting
			if (temp && temp->IsAliveOnMap() && 
				unit->MapDistanceTo(temp) <= 
					unit->Stats->Variables[ATTACKRANGE_INDEX].Max) {
				AutoCast(unit);//for combat spells
				return;
			}
			unit->CurrentOrder()->ClearGoal();
			unit->State = 0;
			unit->SubAction = SUB_STILL_STANDBY; // No attacking, restart
		}
	}

	if (MoveRandomly(unit) || AutoCast(unit) || AutoRepair(unit)) {
		return;
	}

	
	//rb - do we need this ?
	if (unit->Wait) {
		unit->Wait--;
		return;
	}

	//Assert(!unit->CurrentOrder()->HasGoal());

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
