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

#include "action/action_still.h"

#include "animation.h"
#include "iolib.h"
#include "map.h"
#include "missile.h"
#include "player.h"
#include "script.h"
#include "spells.h"
#include "tileset.h"
#include "unit.h"
#include "unittype.h"

enum {
	SUB_STILL_STANDBY = 0,
	SUB_STILL_ATTACK
};

/* static */ COrder *COrder::NewActionStandGround()
{
	return new COrder_Still(true);
}

/* static */ COrder *COrder::NewActionStill()
{
	return new COrder_Still(false);
}


/* virtual */ void COrder_Still::Save(CFile &file, const CUnit &unit) const
{
	if (this->Action == UnitActionStill) {
		file.printf("{\"action-still\",");
	} else {
		file.printf("{\"action-stand-ground\",");
	}
	if (this->Finished) {
		file.printf(" \"finished\", ");
	}
	if (this->AutoTarget != NULL) {
		file.printf(" \"auto-target\", \"%s\",", UnitReference(AutoTarget).c_str());
	}
	if (this->State != 0) { // useless to write default value
		file.printf("\"state\", %d", this->State);
	}
	file.printf("}");
}

/* virtual */ bool COrder_Still::ParseSpecificData(lua_State *l, int &j, const char *value, const CUnit &unit)
{
	if (!strcmp("state", value)) {
		++j;
		lua_rawgeti(l, -1, j + 1);
		this->State = LuaToNumber(l, -1);
		lua_pop(l, 1);
	} else if (!strcmp("auto-target", value)) {
		++j;
		lua_rawgeti(l, -1, j + 1);
		this->AutoTarget = CclGetUnitFromRef(l);
		lua_pop(l, 1);
	} else {
		return false;
	}
	return true;
}

/* virtual */ bool COrder_Still::IsValid() const
{
	return true;
}

/* virtual */ PixelPos COrder_Still::Show(const CViewport & , const PixelPos &lastScreenPos) const
{
	if (this->Action == UnitActionStandGround) {
		Video.FillCircleClip(ColorBlack, lastScreenPos, 2);
	} else {
		Video.FillCircleClip(ColorGray, lastScreenPos, 2);
	}
	return lastScreenPos;
}

class IsTargetInRange
{
public:
	explicit IsTargetInRange(const CUnit &_attacker) : attacker(&_attacker) {}

	bool operator()(const CUnit *unit) const {
		return unit->IsVisibleAsGoal(*attacker->Player)
			   && IsDistanceCorrect(attacker->MapDistanceTo(*unit));
	}
private:
	bool IsDistanceCorrect(int distance) const {
		return attacker->Type->MinAttackRange <= distance
			   && distance <= attacker->Stats->Variables[ATTACKRANGE_INDEX].Max;
	}
private:
	const CUnit *attacker;
};


/* virtual */ void COrder_Still::OnAnimationAttack(CUnit &unit)
{
	if (this->AutoTarget == NULL) {
		return;
	}
	if (IsTargetInRange(unit)(this->AutoTarget) == false) {
		this->AutoTarget = NULL;
		return;
	}
	const Vec2i invalidPos = { -1, -1};

	FireMissile(unit, AutoTarget, invalidPos);
	UnHideUnit(unit);
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
	Map.Clamp(pos);

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

class IsAReparableUnitBy
{
public:
	explicit IsAReparableUnitBy(const CUnit &_worker) : worker(&_worker) {}
	bool operator()(CUnit *unit) const {
		return (unit->IsTeamed(*worker)
				&& unit->Type->RepairHP
				&& unit->Variable[HP_INDEX].Value < unit->Variable[HP_INDEX].Max
				&& unit->IsVisibleAsGoal(*worker->Player));
	}
private:
	const CUnit *worker;
};


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
	const Vec2i offset = {range, range};

	return Map.Find_If(unit.tilePos - offset, unit.tilePos + offset, IsAReparableUnitBy(unit));
}

/**
**  Auto repair a unit if possible
**
**  @return  true if the unit is repairing, false otherwise
*/
bool AutoRepair(CUnit &unit)
{
	const int repairRange = unit.Type->DefaultStat.Variables[AUTOREPAIRRANGE_INDEX].Value;

	if (unit.AutoRepair == false || repairRange == 0) {
		return false;
	}
	CUnit *repairedUnit = UnitToRepairInRange(unit, repairRange);

	if (repairedUnit == NULL) {
		return false;
	}
	const Vec2i invalidPos = { -1, -1};
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
	this->AutoTarget = AttackUnitsInRange(unit);

	if (this->AutoTarget == NULL) {
		return false;
	}
	this->State = SUB_STILL_ATTACK; // Mark attacking.
	this->SetGoal(this->AutoTarget);
	UnitHeadingFromDeltaXY(unit, this->AutoTarget->tilePos + this->AutoTarget->Type->GetHalfTileSize() - unit.tilePos);
	return true;
}

bool COrder_Still::AutoCastStand(CUnit &unit)
{
	if (!unit.Removed) { // Removed units can't cast any spells, from bunker)
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
		case SUB_STILL_STANDBY:
			UnitShowAnimation(unit, unit.Type->Animations->Still);
			break;
		case SUB_STILL_ATTACK: // attacking unit in attack range.
			AnimateActionAttack(unit, *this);
			break;
	}
	if (unit.Anim.Unbreakable) { // animation can't be aborted here
		return;
	}
	this->State = SUB_STILL_STANDBY;
	this->Finished = (this->Action == UnitActionStill);
	if (this->Action == UnitActionStandGround || unit.Removed || unit.CanMove() == false) {
		if (unit.AutoCastSpell) {
			this->AutoCastStand(unit);
		}
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
