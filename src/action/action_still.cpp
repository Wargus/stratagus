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

/*----------------------------------------------------------------------------
--  Includes
----------------------------------------------------------------------------*/

#include "stratagus.h"

#include "action/action_still.h"

#include "animation.h"
#include "commands.h"
#include "iolib.h"
#include "map.h"
#include "missile.h"
#include "player.h"
#include "script.h"
#include "settings.h"
#include "spells.h"
#include "tileset.h"
#include "unit.h"
#include "unit_find.h"
#include "unittype.h"
#include "video.h"

enum {
	SUB_STILL_STANDBY = 0,
	SUB_STILL_ATTACK
};

/* static */ std::unique_ptr<COrder> COrder::NewActionStandGround()
{
	return std::make_unique<COrder_Still>(true);
}

/* static */ std::unique_ptr<COrder> COrder::NewActionStill()
{
	return std::make_unique<COrder_Still>(false);
}


void COrder_Still::Save(CFile &file, const CUnit &unit) const /* override */
{
	if (this->Action == UnitAction::Still) {
		file.printf("{\"action-still\",");
	} else {
		file.printf("{\"action-stand-ground\",");
	}
	if (this->Finished) {
		file.printf(" \"finished\", ");
	}
	if (this->State != 0) { // useless to write default value
		file.printf("\"state\", %d", this->State);
	}
	file.printf("}");
}

bool COrder_Still::ParseSpecificData(lua_State *l, int &j, std::string_view value, const CUnit &unit)
{
	if (value == "state") {
		++j;
		this->State = LuaToNumber(l, -1, j + 1);
	} else {
		return false;
	}
	return true;
}

bool COrder_Still::IsValid() const /* override */
{
	return true;
}

PixelPos COrder_Still::Show(const CViewport &, const PixelPos &lastScreenPos) const /* override */
{
	if (this->Action == UnitAction::StandGround) {
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

	bool operator()(const CUnit *unit) const
	{
		return unit->IsVisibleAsGoal(*attacker->Player)
			   && IsDistanceCorrect(attacker->MapDistanceTo(*unit));
	}
private:
	bool IsDistanceCorrect(int distance) const
	{
		return attacker->Type->MinAttackRange < distance
			   && distance <= attacker->Stats->Variables[ATTACKRANGE_INDEX].Max;
	}
private:
	const CUnit *attacker;
};


void COrder_Still::OnAnimationAttack(CUnit &unit) /* override */
{
	CUnit *goal = this->GetGoal();
	if (goal == nullptr) {
		return;
	}
	if (IsTargetInRange(unit)(goal) == false) {
		this->ClearGoal();
		return;
	}

	FireMissile(unit, goal, goal->tilePos);
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
	if (unit.Type->RandomMovementProbability == false) {
		if (!unit.JustMoved) {
			return false;
		}
		int shift = Map.Tileset.getLogicalToGraphicalTileSizeShift();
		if (!shift) {
			return false;
		}
		auto pos = unit.tilePos;
		auto w = unit.Type->PersonalSpaceWidth;
		auto h = unit.Type->PersonalSpaceHeight;
		if (w || h) {
			std::vector<CUnit *> around = SelectAroundUnit(unit, (w + h) / 2, IsSameMovementType(unit));
			Vec2i vec(0, 0);
			for (auto u : around) {
				if (u != &unit) {
					vec += pos - u->tilePos;
				}
			}
			if (vec.x || vec.y) {
				auto newPos = pos + Vec2i(std::clamp(vec.x, (short)-1, (short)1), std::clamp(vec.y, (short)-1, (short)1));
				Map.Clamp(newPos);
				if (newPos.x || newPos.y) {
					CommandMove(unit, newPos, EFlushMode::On);
					return true;
				}
			}
		}
		return false;
	}
	if ((SyncRand() % 100) > unit.Type->RandomMovementProbability) {
		return false;
	}

	// pick random location
	Vec2i pos = unit.tilePos;

	pos.x += SyncRand(unit.Type->RandomMovementDistance * 2 + 1) - unit.Type->RandomMovementDistance;
	pos.y += SyncRand(unit.Type->RandomMovementDistance * 2 + 1) - unit.Type->RandomMovementDistance;

	// restrict to map
	Map.Clamp(pos);

	// move if possible
	if (pos != unit.tilePos) {
		UnmarkUnitFieldFlags(unit);
		if (UnitCanBeAt(unit, pos)) {
			MarkUnitFieldFlags(unit);
			CommandMove(unit, pos, EFlushMode::On);
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
	if (!unit.AutoCastSpell.empty() && !unit.Removed) { // Removed units can't cast any spells, from bunker)
		for (unsigned int i = 0; i < SpellTypeTable.size(); ++i) {
			if (unit.AutoCastSpell[i]
				&& (SpellTypeTable[i]->AutoCast || SpellTypeTable[i]->AICast)
				&& AutoCastSpell(unit, *SpellTypeTable[i])) {
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
	bool operator()(CUnit *unit) const
	{
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
**  @return       unit to repair if found, nullptr otherwise
**
**  @todo         FIXME: find the best unit (most damaged, ...).
*/
static CUnit *UnitToRepairInRange(const CUnit &unit, int range)
{
	const Vec2i offset(range, range);

	return FindUnit_If(unit.tilePos - offset, unit.tilePos + offset, IsAReparableUnitBy(unit));
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

	if (repairedUnit == nullptr) {
		return false;
	}
	const Vec2i invalidPos(-1, -1);
	std::unique_ptr<COrder> savedOrder;
	if (unit.CanStoreOrder(unit.CurrentOrder())) {
		savedOrder = unit.CurrentOrder()->Clone();
	}

	//Command* will clear unit.SavedOrder
	CommandRepair(unit, invalidPos, repairedUnit, EFlushMode::On);
	if (savedOrder != nullptr) {
		unit.SavedOrder = std::move(savedOrder);
	}
	return true;
}

bool COrder_Still::AutoAttackStand(CUnit &unit)
{
	if (unit.Type->CanAttack == false) {
		return false;
	}
	//  FIXME: if bunkers can increase attack range - count it in the distance calculations and target selection.

	// Removed units can only attack in AttackRange, from bunker
	CUnit *autoAttackUnit = AttackUnitsInRange(unit);

	if (autoAttackUnit == nullptr) {
		return false;
	}

	/*
		FIXME: To Calc and Set GoalPos (if target is closer than MinAttackRange)
		Only for units which can attack-ground && has a splash attack && can hit current target with splash
		Else do not attack at all
	*/
	// If unit is removed, use containers x and y
	const CUnit *firstContainer = unit.Container ? unit.Container : &unit;
	const int dist = firstContainer->MapDistanceTo(*autoAttackUnit);
	if (dist > unit.Stats->Variables[ATTACKRANGE_INDEX].Max	|| dist < unit.Type->MinAttackRange) {
		return false;
	}
	if (GameSettings.Inside && CheckObstaclesBetweenTiles(unit.tilePos, autoAttackUnit->tilePos, MapFieldRocks | MapFieldForest) == false) {
		return false;
	}
	this->State = SUB_STILL_ATTACK; // Mark attacking.
	this->SetGoal(autoAttackUnit);
	UnitHeadingFromDeltaXY(unit, autoAttackUnit->tilePos + autoAttackUnit->Type->GetHalfTileSize() - unit.tilePos);
	return true;
}

bool COrder_Still::AutoCastStand(CUnit &unit)
{
	if (!unit.Removed) { // Removed units can't cast any spells, from bunker)
		for (unsigned int i = 0; i < SpellTypeTable.size(); ++i) {
			if (unit.AutoCastSpell[i]
				&& (SpellTypeTable[i]->AutoCast || SpellTypeTable[i]->AICast)
				&& AutoCastSpell(unit, *SpellTypeTable[i])) {
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

	if (goal == nullptr) {
		return false;
	}
	std::unique_ptr<COrder> savedOrder;

	if (unit.CurrentAction() == UnitAction::Still) {
		savedOrder = COrder::NewActionAttack(unit, unit.tilePos);
	} else if (unit.CanStoreOrder(unit.CurrentOrder())) {
		savedOrder = unit.CurrentOrder()->Clone();
	}
	// Weak goal, can choose other unit, come back after attack
	CommandAttack(unit, goal->tilePos, nullptr, EFlushMode::On);

	if (savedOrder != nullptr) {
		unit.SavedOrder = std::move(savedOrder);
	}
	return true;
}


void COrder_Still::Execute(CUnit &unit) /* override */
{
	// If unit is not bunkered and removed, wait
	if (unit.Removed
	    && (unit.Container == nullptr
	        || unit.Container->Type->BoolFlag[ATTACKFROMTRANSPORTER_INDEX].value == false)) {
		return;
	}
	this->Finished = false;

	switch (this->State) {
		case SUB_STILL_STANDBY:
			UnitShowAnimation(unit, &unit.Type->Animations->Still);
			break;
		case SUB_STILL_ATTACK: // attacking unit in attack range.
			AnimateActionAttack(unit, *this);
			break;
	}
	if (unit.Anim.Unbreakable) { // animation can't be aborted here
		return;
	}
	this->State = SUB_STILL_STANDBY;
	this->Finished = (this->Action == UnitAction::Still);

	if (this->Sleep > 0) {
		this->Sleep -= 1;
		return;
	}
	// sleep some time before trying again for automatic actions
	this->Sleep = CYCLES_PER_SECOND / 2;

	if (this->Action == UnitAction::StandGround || unit.Removed || unit.CanMove() == false) {
		if (!unit.AutoCastSpell.empty()) {
			this->AutoCastStand(unit);
		}
		if (unit.IsAggressive()) {
			this->AutoAttackStand(unit);
		}
	} else {
		if (unit.JustMoved) --unit.JustMoved;
		if (AutoCast(unit) || (unit.IsAggressive() && AutoAttack(unit))
			|| AutoRepair(unit)
			|| MoveRandomly(unit)) {
		}
	}
}


//@}
