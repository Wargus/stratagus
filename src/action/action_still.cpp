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
	if (unit->IsAgressive()) {
		MapSight(unit->Player, unit->X, unit->Y,
		 	unit->Type->TileWidth, unit->Type->TileHeight,
		 	unit->GetReactRange(),
		 	MapMarkTileGuard);
	}
}

void MapUnmarkUnitGuard(CUnit *unit)
{
	if (unit->IsAgressive()) {
		MapSight(unit->Player, unit->X, unit->Y,
		 	unit->Type->TileWidth, unit->Type->TileHeight,
		 	unit->GetReactRange(),
		 	MapUnmarkTileGuard);
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
				// FIXME: Don't use pathfinder for this, costs too much cpu.
				unit->Orders[0]->Action = UnitActionMove;
				Assert(!unit->Orders[0]->Goal);
				unit->Orders[0]->Goal = NoUnitP;
				unit->Orders[0]->Range = 0;
				unit->Orders[0]->X = x;
				unit->Orders[0]->Y = y;
				unit->State = 0;
				MapUnmarkUnitGuard(unit);
				return true;//TESTME: new localization
			}
			MarkUnitFieldFlags(unit);
		}
		//return true;//TESTME: old localization
	}
	return false;
}

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
	int n = Map.Select(unit->X - range, unit->Y - range,
		unit->X + unit->Type->TileWidth + range,
		unit->Y + unit->Type->TileHeight + range,
		table);
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
/*
	if (unit->Wait) {
		unit->Wait--;
		return;
	}
*/
	// Cowards and invisible units don't attack unless ordered.
	if (unit->IsAgressive()) {
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
			} //else {
				//unit->Wait = 15;
			//}
		// Removed units can only attack in AttackRange, from bunker
		} else  {
			if ((goal = AttackUnitsInRange(unit))) {
				temp = unit->Orders[0]->Goal;
				if (temp && temp->Orders[0]->Action == UnitActionDie) {
					temp->RefsDecrease();
					unit->Orders[0]->Goal = temp = NoUnitP;
				}
				if (unit->SubAction < 2|| temp != goal) {
					// New target.
					if (temp) {
						temp->RefsDecrease();
					}
					unit->Orders[0]->Goal = goal;
					goal->RefsIncrease();
					unit->State = 0;
					unit->SubAction = 2; // Mark attacking.
					UnitHeadingFromDeltaXY(unit,
						goal->X + (goal->Type->TileWidth - 1) / 2 - unit->X,
						goal->Y + (goal->Type->TileHeight - 1) / 2 - unit->Y);
				}
				return;
			}
		}
	} //else {
		//unit->Wait = 15;
	//}

	if (unit->SubAction > 1) { // was attacking.
		if ((temp = unit->Orders[0]->Goal)) {
			temp->RefsDecrease();
			unit->Orders[0]->Goal = NoUnitP;
		}
		unit->State = 0;
		unit->SubAction = 1; // No attacking, restart
	}
	Assert(!unit->Orders[0]->Goal);
}

void AutoAttack(CUnit *unit, CUnitCache &targets, bool stand_ground)
{
	CUnit *temp;
	CUnit *goal;
/*
	if (unit->Wait) {
		unit->Wait--;
		return;
	}
*/
	// Cowards and invisible units don't attack unless ordered.
	if (unit->IsAgressive()) {
		// Normal units react in reaction range.
		if (CanMove(unit) && !unit->Removed && !stand_ground) {
			if ((goal = AutoAttackUnitsInDistance(unit,
			   	 unit->GetReactRange(), targets))) {
				// Weak goal, can choose other unit, come back after attack
				CommandAttack(unit, goal->X, goal->Y, NULL, FlushCommands);
				Assert(unit->SavedOrder.Action == UnitActionStill);
				Assert(!unit->SavedOrder.Goal);
				unit->SavedOrder.Action = UnitActionAttack;
				unit->SavedOrder.Range = 0;
				unit->SavedOrder.X = unit->X;
				unit->SavedOrder.Y = unit->Y;
				unit->SavedOrder.Goal = NoUnitP;
			} //else {
				//unit->Wait = 15;
			//}
		// Removed units can only attack in AttackRange, from bunker
		} else  {
			if ((goal = AutoAttackUnitsInDistance(unit, 
					unit->Stats->Variables[ATTACKRANGE_INDEX].Max, targets))) {
				temp = unit->Orders[0]->Goal;
				if (temp && temp->Orders[0]->Action == UnitActionDie) {
					temp->RefsDecrease();
					unit->Orders[0]->Goal = temp = NoUnitP;
				}
				if (unit->SubAction < 2|| temp != goal) {
					// New target.
					if (temp) {
						temp->RefsDecrease();
					}
					unit->Orders[0]->Goal = goal;
					goal->RefsIncrease();
					unit->State = 0;
					unit->SubAction = 2; // Mark attacking.
					UnitHeadingFromDeltaXY(unit,
						goal->X + (goal->Type->TileWidth - 1) / 2 - unit->X,
						goal->Y + (goal->Type->TileHeight - 1) / 2 - unit->Y);
				}
				return;
			}
		}
	} //else {
		//unit->Wait = 15;
	//}
/*
	if (unit->SubAction > 1) { // was attacking.
		if ((temp = unit->Orders[0]->Goal)) {
			temp->RefsDecrease();
			unit->Orders[0]->Goal = NoUnitP;
		}
		unit->State = 0;
		unit->SubAction = 1; // No attacking, restart
	}
	Assert(!unit->Orders[0]->Goal);
	*/
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
	bool first_entrly = false;
//	if (unit->Anim.Unbreakable) { // animation can't be aborted here
//		return;
//	}

	switch(unit->SubAction)
	{
		case 0:
			//first entry
			MapMarkUnitGuard(unit);
			unit->SubAction = 1;
			first_entrly = true;
		case 1:	
			UnitShowAnimation(unit, unit->Type->Animations->Still);
		break;
		case 2: // attacking unit in attack range.
			AnimateActionAttack(unit);
		break;
	}

	if (unit->Anim.Unbreakable) { // animation can't be aborted here
		return;
	}

	if (MoveRandomly(unit) || AutoCast(unit) || AutoRepair(unit)) {
		return;
	}

	if (first_entrly) { 
		// during first entry make autoattack test 
		//and attack units in attack range.
		AutoAttack(unit, stand_ground);
	}
	
	if (unit->Wait) {
		unit->Wait--;
		return;
	}

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
