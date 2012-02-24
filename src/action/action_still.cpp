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
#include "iolib.h"
#include "script.h"

enum {
	SUB_STILL_INIT = 0,
	SUB_STILL_STANDBY = 1,
	SUB_STILL_ATTACK = 2
};

/* virtual */ void COrder_Still::Save(CFile &file, const CUnit &unit) const
{
	if (this->Action == UnitActionStill) {
		file.printf("{\"action-still\"");
	} else {
		file.printf("{\"action-stand-ground\",");
	}
	if (this->Finished) {
		file.printf(" \"finished\", ");
	}
	if (this->HasGoal()) {
		CUnit &goal = *this->GetGoal();
		if (goal.Destroyed) {
			/* this unit is destroyed so it's not in the global unit
			 * array - this means it won't be saved!!! */
			printf ("FIXME: storing destroyed Goal - loading will fail.\n");
		}
		file.printf(" \"goal\", \"%s\",", UnitReference(goal).c_str());
	}

	file.printf(", \"state\", %d", this->State);
	file.printf("}");
}

/* virtual */ bool COrder_Still::ParseSpecificData(lua_State *l, int &j, const char *value, const CUnit &unit)
{
	if (!strcmp("state", value)) {
		++j;
		lua_rawgeti(l, -1, j + 1);
		this->State = LuaToNumber(l, -1);
		lua_pop(l, 1);
	} else {
		return false;
	}
	return true;
}


/*----------------------------------------------------------------------------
--  Functions
----------------------------------------------------------------------------*/

void UnHideUnit(CUnit &unit)
{
	unit.Variable[INVISIBLE_INDEX].Value = 0;
}

/**
**  Move in a random direction
**
**  @return  true if the unit moves, false otherwise
*/
static bool MoveRandomly(CUnit &unit)
{
	if (unit.Type->RandomMovementProbability == false
		|| ((SyncRand() % 100) > unit.Type->RandomMovementProbability)) {
		return false;
	}
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
			MarkUnitFieldFlags(unit);
			CommandMove(unit, pos, FlushCommands);
			return true;
		}
		MarkUnitFieldFlags(unit);
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
			if (unit.AutoCastSpell[i]
				&& (SpellTypeTable[i]->AutoCast || SpellTypeTable[i]->AICast)
				&& AutoCastSpell(unit, SpellTypeTable[i])) {
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
	return NULL;
}

/**
**  Auto repair a unit if possible
**
**  @return  true if the unit is repairing, false otherwise
*/
bool AutoRepair(CUnit &unit)
{
	const int repairRange = unit.Type->Variable[AUTOREPAIRRANGE_INDEX].Value;

	if (unit.AutoRepair == false || repairRange == 0) {
		return false;
	}
	CUnit *repairedUnit = UnitToRepairInRange(unit, repairRange);

	if (repairedUnit == NULL) {
		return false;
	}
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

bool COrder_Still::AutoAttackStand(CUnit &unit)
{
	if (unit.Type->CanAttack == false) {
		return false;
	}
	// Removed units can only attack in AttackRange, from bunker
	CUnit *goal = AttackUnitsInRange(unit);

	if (goal == NULL) {
		return false;
	}
	this->SetGoal(goal);
	this->State = SUB_STILL_ATTACK; // Mark attacking.
	UnitHeadingFromDeltaXY(unit, goal->tilePos + goal->Type->GetHalfTileSize() - unit.tilePos);
	return true;
}


/**
**  Auto attack nearby units if possible
*/
bool AutoAttack(CUnit &unit)
{
	if (unit.Type->CanAttack == false) {
		return false;
	}
	// Normal units react in reaction range.
	CUnit *goal = AttackUnitsInReactRange(unit);

	if (goal == NULL) {
		return false;
	}
	COrder *savedOrder;

	if (unit.SavedOrder != NULL) {
		savedOrder = unit.SavedOrder->Clone();
	} else if (unit.CurrentAction() == UnitActionStill) {
		savedOrder = COrder::NewActionAttack(unit, unit.tilePos);
	} else {
		savedOrder = unit.CurrentOrder()->Clone();
	}
	// Weak goal, can choose other unit, come back after attack
	CommandAttack(unit, goal->tilePos, NULL, FlushCommands);

	if (unit.StoreOrder(savedOrder) == false) {
		delete savedOrder;
	}
	return true;
}


/* virtual */ void COrder_Still::Execute(CUnit &unit)
{
	// If unit is not bunkered and removed, wait
	if (unit.Removed
		&& (unit.Container == NULL || unit.Container->Type->AttackFromTransporter == false)) {
		return ;
	}
	this->Finished = false;

	switch (this->State) {
		case SUB_STILL_INIT: //first entry
			this->State = SUB_STILL_STANDBY;
			// no break : follow
		case SUB_STILL_STANDBY:
			UnitShowAnimation(unit, unit.Type->Animations->Still);
		break;
		case SUB_STILL_ATTACK: // attacking unit in attack range.
			AnimateActionAttack(unit);
		break;
	}
	if (unit.Anim.Unbreakable) { // animation can't be aborted here
		return ;
	}
	this->State = SUB_STILL_STANDBY;
	this->Finished = (this->Action == UnitActionStill);
	if (this->Action == UnitActionStandGround || unit.Removed || unit.CanMove() == false) {
		if (unit.IsAgressive()) {
			this->AutoAttackStand(unit);
		}
	} else {
		if ((unit.IsAgressive() && AutoAttack(unit))
			|| AutoCast(unit)
			|| AutoRepair(unit)
			|| MoveRandomly(unit)) {
		}
	}
}


//@}
