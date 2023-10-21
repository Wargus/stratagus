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
/**@name ai_force.cpp - AI force functions. */
//
//      (c) Copyright 2001-2015 by Lutz Sammer, Jimmy Salmon and Andrettin
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

#include "ai_local.h"

#include "actions.h"
#include "action/action_attack.h"
#include "action/action_board.h"
#include "commands.h"
#include "depend.h"
#include "map.h"
#include "pathfinder.h"
#include "tileset.h"
#include "unit.h"
#include "unit_find.h"
#include "unittype.h"

/*----------------------------------------------------------------------------
--  Types
----------------------------------------------------------------------------*/
#define AIATTACK_RANGE 0
#define AIATTACK_ALLMAP 1
#define AIATTACK_BUILDING 2
#define AIATTACK_AGRESSIVE 3

class EnemyUnitFinder
{
public:
	EnemyUnitFinder(const CUnit &unit, CUnit **result_unit, int find_type) :
		//Wyrmgus end
		unit(unit),
		movemask(unit.Type->MovementMask & ~(MapFieldLandUnit | MapFieldAirUnit | MapFieldSeaUnit)),
		attackrange(unit.Stats->Variables[ATTACKRANGE_INDEX].Max),
		find_type(find_type),
		result_unit(result_unit)
	{
		*result_unit = nullptr;
	}
	VisitResult Visit(TerrainTraversal &terrainTraversal, const Vec2i &pos, const Vec2i &from);
private:
	const CUnit &unit;
	unsigned int movemask;
	const int attackrange;
	const int find_type;
	CUnit **result_unit;
};

VisitResult EnemyUnitFinder::Visit(TerrainTraversal &terrainTraversal, const Vec2i &pos, const Vec2i &from)
{
	if (!CanMoveToMask(pos, movemask)) { // unreachable
		return VisitResult::DeadEnd;
	}

	Vec2i minpos = pos - Vec2i(attackrange, attackrange);
	Vec2i maxpos = pos + Vec2i(unit.Type->TileWidth - 1 + attackrange, unit.Type->TileHeight - 1 + attackrange);
	std::vector<CUnit *> table = Select(minpos, maxpos, HasNotSamePlayerAs(Players[PlayerNumNeutral]));
	for (CUnit *dest : table) {
		const CUnitType &dtype = *dest->Type;

		if (!unit.IsEnemy(*dest) // a friend or neutral
		    || !CanTarget(*unit.Type, dtype)) {
			continue;
		}

		// Don't attack invulnerable units
		if (dtype.BoolFlag[INDESTRUCTIBLE_INDEX].value || dest->Variable[UNHOLYARMOR_INDEX].Value) {
			continue;
		}

		if ((find_type != AIATTACK_BUILDING || dtype.BoolFlag[BUILDING_INDEX].value) && (find_type != AIATTACK_AGRESSIVE || dest->IsAgressive())) {
			*result_unit = dest;
			return VisitResult::Finished;
		} else if (*result_unit == nullptr) { // if trying to search for buildings or aggressive units specifically, still put the first found unit (even if it doesn't fit those parameters) as the result unit, so that it can be returned if no unit with the specified parameters is found
			*result_unit = dest;
		}
	}
	return VisitResult::Ok;
}

template <int FIND_TYPE>
class AiForceEnemyFinder
{
public:
	AiForceEnemyFinder(int force, const CUnit **enemy) : enemy(enemy)
	{
		Assert(enemy != nullptr);
		*enemy = nullptr;
		for (const CUnit *unit : AiPlayer->Force[force].Units) {
			if (!(*this)(unit)) {
				break;
			}
		}
	}

	AiForceEnemyFinder(AiForce &force, const CUnit **enemy) : enemy(enemy)
	{
		Assert(enemy != nullptr);
		*enemy = nullptr;
		for (const CUnit* unit : force.Units) {
			if (!(*this)(unit)) {
				break;
			}
		}
	}

	bool found() const { return *enemy != nullptr; }

	bool operator()(const CUnit *const unit) const
	{
		if (unit->Type->CanAttack == false) {
			return *enemy == nullptr;
		}
		if constexpr (FIND_TYPE == AIATTACK_RANGE) {
			*enemy = AttackUnitsInReactRange(*unit);
		} else {
			// Terrain traversal by Andrettin
			TerrainTraversal terrainTraversal;
			terrainTraversal.SetSize(Map.Info.MapWidth, Map.Info.MapHeight);
			terrainTraversal.Init();
			terrainTraversal.PushUnitPosAndNeighboor(*unit);
			CUnit *result_unit = nullptr;
			EnemyUnitFinder enemyUnitFinder(*unit, &result_unit, FIND_TYPE);
			terrainTraversal.Run(enemyUnitFinder);
			*enemy = result_unit;
		}
		// Previous attack finding code before we added TerrainTraversal for all
		// of these. Here for prosperity

		// } else if (FIND_TYPE == AIATTACK_ALLMAP) {
		// 	*enemy = AttackUnitsInDistance(*unit, MaxMapWidth);
		// } else if (FIND_TYPE == AIATTACK_BUILDING) {
		// 	*enemy = AttackUnitsInDistance(*unit, MaxMapWidth, IsBuildingType());
		// 	Assert(!*enemy);
		// 	if (*enemy == nullptr || !(*enemy)->Type->Building) {
		// 		*enemy = AttackUnitsInDistance(*unit, MaxMapWidth);
		// 	}
		// } else if (FIND_TYPE == AIATTACK_AGRESSIVE) {
		// 	*enemy = AttackUnitsInDistance(*unit, MaxMapWidth, IsAggresiveUnit());
		// 	Assert(!*enemy || (*enemy)->IsAgressive());
		// 	if (*enemy == nullptr) {
		// 		*enemy = AttackUnitsInDistance(*unit, MaxMapWidth);
		// 	}
		// }
		return *enemy == nullptr;
	}
private:
	const CUnit **enemy;
};

class IsAnAlliedUnitOf
{
public:
	explicit IsAnAlliedUnitOf(const CPlayer &_player) : player(&_player) {}
	bool operator()(const CUnit *unit) const
	{
		return unit->IsVisibleAsGoal(*player) && (unit->Player->Index == player->Index
												  || unit->IsAllied(*player));
	}
private:
	const CPlayer *player;
};

/*----------------------------------------------------------------------------
--  Variables
----------------------------------------------------------------------------*/

int UnitTypeEquivs[UnitTypeMax + 1]; /// equivalence between unittypes

/*----------------------------------------------------------------------------
--  Functions
----------------------------------------------------------------------------*/

/**
**  Remove any equivalence between unittypes
*/
void AiResetUnitTypeEquiv()
{
	ranges::iota(UnitTypeEquivs, 0);
}

/**
**  Make two unittypes equivalents from the AI's point of vue
**
**  @param a  the first unittype
**  @param b  the second unittype
*/
void AiNewUnitTypeEquiv(const CUnitType &a, const CUnitType &b)
{
	int find = UnitTypeEquivs[a.Slot];
	int replace = UnitTypeEquivs[b.Slot];

	// Always record equivalences with the lowest unittype.
	if (find < replace) {
		std::swap(find, replace);
	}

	// Then just find & replace in UnitTypeEquivs...
	for (auto &equiv : UnitTypeEquivs) {
		if (equiv == find) {
			equiv = replace;
		}
	}
}


/**
**  Find All unittypes equivalent to a given one
**
**  @param unittype  the unittype to find equivalence for
**
**  @return          the unittypes found
*/
std::vector<int> AiFindUnitTypeEquiv(const CUnitType &unittype)
{
	const int search = UnitTypeEquivs[unittype.Slot];
	std::vector<int> result;

	for (int i = 0; i < UnitTypeMax + 1; ++i) {
		if (UnitTypeEquivs[i] == search) {
			// Found one
			result.push_back(i);
		}
	}
	return result;
}

class UnitTypePrioritySorter_Decreasing
{
public:
	bool operator()(int lhs, int rhs) const
	{
		return UnitTypes[lhs]->MapDefaultStat.Variables[PRIORITY_INDEX].Value > UnitTypes[rhs]->MapDefaultStat.Variables[PRIORITY_INDEX].Value;
	}
};

/**
**  Find All unittypes equivalent to a given one, and which are available
**  UnitType are returned in the preferred order (ie paladin >> knight...)
**
**  @param unittype     The unittype to find equivalence for
**
**  @return             the unittypes found
*/
std::vector<int> AiFindAvailableUnitTypeEquiv(const CUnitType &unittype)
{
	// 1 - Find equivalents
	auto usableTypes = AiFindUnitTypeEquiv(unittype);
	// 2 - Remove unavailable unittypes
	ranges::erase_if(usableTypes, [&](int typeIndex) {
		return !CheckDependByIdent(*AiPlayer->Player, UnitTypes[typeIndex]->Ident);
	});
	// 3 - Sort by level
	ranges::sort(usableTypes, UnitTypePrioritySorter_Decreasing());
	return usableTypes;
}

/* =========================== FORCES ========================== */

void AiForce::CountTypes(unsigned int *counter, const size_t len)
{
	memset(counter, 0, len);
	for (CUnit* unit : Units)
	{
		counter[UnitTypeEquivs[unit->Type->Slot]]++;
	}
}

/**
**  Check if the units belongs to the force/base.
**
**  @param type   Type to check.
**
**  @return       True if it fits, false otherwise.
*/
bool AiForce::IsBelongsTo(const CUnitType &type)
{
	bool flag = false;
	unsigned int counter[UnitTypeMax + 1];

	// Count units in force.
	CountTypes(counter, sizeof(counter));

	// Look what should be in the force.
	Completed = true;
	for (const AiUnitType &aitype : UnitTypes) {
		const int slot = aitype.Type->Slot;

		if (counter[slot] < aitype.Want) {
			if (UnitTypeEquivs[type.Slot] == slot) {
				if (counter[slot] < aitype.Want - 1) {
					Completed = false;
				}
				flag = true;
			} else {
				Completed = false;
			}
		}
	}
	return flag;
}

void AiForce::Insert(CUnit &unit)
{
	Units.push_back(&unit);
	unit.RefsIncrease();
}

/* static */ void AiForce::InternalRemoveUnit(CUnit *unit)
{
	unit->GroupId = 0;
	unit->RefsDecrease();
}


/**
**  Ai clean units in a force.
*/
void AiForce::RemoveDeadUnit()
{
	// Release all killed units.
	auto end = ranges::partition(Units, [](const CUnit *unit) { return unit->IsAlive(); });
	for (auto it = end; it != std::end(Units); ++it) {
		InternalRemoveUnit(*it);
	}
	Units.erase(end, std::end(Units));
}

class AiForceRallyPointFinder
{
public:
	AiForceRallyPointFinder(const CUnit &startUnit, int distance, const Vec2i &startPos, Vec2i *resultPos) :
		startUnit(startUnit), distance(distance), startPos(startPos),
		movemask(startUnit.Type->MovementMask & ~(MapFieldLandUnit | MapFieldAirUnit | MapFieldSeaUnit | MapFieldBuilding)),
		resultPos(resultPos)
	{}
	VisitResult Visit(TerrainTraversal &terrainTraversal, const Vec2i &pos, const Vec2i &from);
private:
	const CUnit &startUnit;
	const int distance;
	const Vec2i startPos;
	const int movemask;
	Vec2i *resultPos;
};

VisitResult AiForceRallyPointFinder::Visit(TerrainTraversal &terrainTraversal, const Vec2i &pos, const Vec2i &from)
{
	const int minDist = 15;
	if (AiEnemyUnitsInDistance(*startUnit.Player, nullptr, pos, minDist) == false
		&& Distance(pos, startPos) <= abs(distance - minDist)) {
		*resultPos = pos;
		return VisitResult::Finished;
	}
	if (CanMoveToMask(pos, movemask)) { // reachable
		return VisitResult::Ok;
	} else { // unreachable
		return VisitResult::DeadEnd;
	}
}

bool AiForce::NewRallyPoint(const Vec2i &startPos, Vec2i *resultPos)
{
	Assert(this->Units.size() > 0);
	const CUnit &leader = *(this->Units[0]);
	const int distance = leader.MapDistanceTo(startPos);

	WaitOnRallyPoint = AI_WAIT_ON_RALLY_POINT;

	TerrainTraversal terrainTraversal;

	terrainTraversal.SetSize(Map.Info.MapWidth, Map.Info.MapHeight);
	terrainTraversal.Init();

	Assert(Map.Info.IsPointOnMap(startPos));
	terrainTraversal.PushPos(startPos);

	AiForceRallyPointFinder aiForceRallyPointFinder(leader, distance, leader.tilePos, resultPos);

	return terrainTraversal.Run(aiForceRallyPointFinder);
}

void AiForce::Attack(const Vec2i &pos)
{
	bool isDefenceForce = false;
	RemoveDeadUnit();

	if (Units.size() == 0) {
		this->Attacking = false;
		this->State = AiForceAttackingState::Waiting;
		return;
	}
	if (!this->Attacking) {
		// Remember the original force position so we can return there after attack
		if (this->Role == AiForceRole::Defend
			|| (this->Role == AiForceRole::Attack && this->State == AiForceAttackingState::Waiting)) {
			this->HomePos = this->Units[this->Units.size() - 1]->tilePos;
		}
		this->Attacking = true;
	}
	Vec2i goalPos(pos);

	const bool isNaval = ranges::any_of(this->Units, [](const CUnit *unit) {
		return unit->Type->MoveType == EMovement::Naval && unit->Type->CanAttack;
	});
	const bool isTransporter = ranges::any_of(this->Units, [](const CUnit *unit) {
		return unit->Type->CanTransport() && unit->IsAgressive() == false;
	});
	if (Map.Info.IsPointOnMap(goalPos) == false) {
		/* Search in entire map */
		const CUnit *enemy = nullptr;
		if (isTransporter) {
			AiForceEnemyFinder<AIATTACK_AGRESSIVE>(*this, &enemy);
		} else if (isNaval) {
			AiForceEnemyFinder<AIATTACK_ALLMAP>(*this, &enemy);
		} else {
			AiForceEnemyFinder<AIATTACK_BUILDING>(*this, &enemy);
		}
		if (enemy) {
			goalPos = enemy->tilePos;
		}
	} else {
		isDefenceForce = true;
	}
	if (Map.Info.IsPointOnMap(goalPos) == false || isTransporter) {
		DebugPrint("%d: Need to plan an attack with transporter\n", AiPlayer->Player->Index);
		if (State == AiForceAttackingState::Waiting && !PlanAttack()) {
			DebugPrint("%d: Can't transport\n", AiPlayer->Player->Index);
			Attacking = false;
		}
		return;
	}
	if (this->State == AiForceAttackingState::Waiting && isDefenceForce == false) {
		Vec2i resultPos {-1, -1};
		NewRallyPoint(goalPos, &resultPos);
		if (resultPos.x != -1 && resultPos.y != -1) {
			this->GoalPos = resultPos;
			this->State = AiForceAttackingState::GoingToRallyPoint;
		} else {
			this->GoalPos = goalPos;
			this->State = AiForceAttackingState::Attacking;
		}
	} else {
		this->GoalPos = goalPos;
		this->State = AiForceAttackingState::Attacking;
	}
	//  Send all units in the force to enemy.

	CUnit *leader = nullptr;
	for (CUnit *unit : this->Units) {
		if (unit->IsAgressive()) {
			leader = unit;
			break;
		}
	}
	for (size_t i = 0; i != this->Units.size(); ++i) {
		CUnit *const unit = this->Units[i];

		if (unit->Container == nullptr) {
			const int delay = i / 5; // To avoid lot of CPU consuption, send them with a small time difference.

			unit->Wait = delay;
			if (unit->IsAgressive()) {
				CommandAttack(*unit, this->GoalPos,  nullptr, FlushCommands);
			} else {
				if (leader) {
					CommandDefend(*unit, *leader, FlushCommands);
				} else {
					CommandMove(*unit, this->GoalPos, FlushCommands);
				}
			}
		}
	}
}

void AiForce::ReturnToHome()
{
	if (Map.Info.IsPointOnMap(this->HomePos)) {
		for (CUnit *unit : this->Units) {
			CommandMove(*unit, this->HomePos, FlushCommands);
		}
	}
	const Vec2i invalidPos(-1, -1);

	this->HomePos = invalidPos;
	this->GoalPos = invalidPos;
	this->Defending = false;
	this->Attacking = false;
	this->State = AiForceAttackingState::Waiting;
}

AiForceManager::AiForceManager()
{
	forces.resize(AI_MAX_FORCES);
	memset(script, -1, AI_MAX_FORCES * sizeof(char));
}

unsigned int AiForceManager::FindFreeForce(AiForceRole role, int begin)
{
	/* find free force */
	unsigned int f = begin;
	while (f < forces.size() && (forces[f].State > AiForceAttackingState::Free)) {
		++f;
	}
	if (f == forces.size()) {
		forces.resize(f + 1);
	}
	forces[f].State = AiForceAttackingState::Waiting;
	forces[f].Role = role;
	return f;
}

/**
**  Find unit in force
**
**  @param    unit  Unit to search for.
**
**  @return   Force number, or std::nullopt if not found
*/
std::optional<int> AiForceManager::GetForce(const CUnit &unit)
{
	for (unsigned int i = 0; i < forces.size(); ++i) {
		AiForce &force = forces[i];

		if (ranges::any_of(force.Units, [&](const CUnit *aiunit) {
				return UnitNumber(unit) == UnitNumber(*aiunit);
			})) {
			return i;
		}
	}
	return std::nullopt;
}

/**
**  Cleanup units in forces.
*/
void AiForceManager::RemoveDeadUnit()
{
	for (auto &force : forces) {
		force.RemoveDeadUnit();
	}
}

/**
**  Ai assign unit to force.
**
**  @param unit  Unit to assign to force.
*/
bool AiForceManager::Assign(CUnit &unit, int force)
{
	if (unit.GroupId != 0) {
		return false;
	}
	if (force != -1) {
		AiForce &f = forces[AiPlayer->Force.getScriptForce(force)];
		if (f.IsBelongsTo(*unit.Type)) {
			f.Insert(unit);
			unit.GroupId = force + 1;
			return true;
		}
	} else {
		// Check to which force it belongs
		for (unsigned int i = 0; i < forces.size(); ++i) {
			AiForce &f = forces[i];
			// No troops for attacking force
			if (f.IsAttacking()) {
				continue;
			}
			if (f.IsBelongsTo(*unit.Type)) {
				f.Insert(unit);
				unit.GroupId = i + 1;
				return true;
			}
		}
	}
	return false;
}

void AiForceManager::CheckUnits(std::array<int, UnitTypeMax> &counter)
{
	int attacking[UnitTypeMax]{};
	const int *unit_types_count = AiPlayer->Player->UnitTypesAiActiveCount;

	// Look through the forces what is missing.
	for (const AiForce &force : forces) {
		if (force.State > AiForceAttackingState::Free && force.IsAttacking()) {
			for (unsigned int j = 0; j < force.Size(); ++j) {
				const CUnit *unit = force.Units[j];
				attacking[unit->Type->Slot]++;
			}
		}
	}
	// create missing units
	for (AiForce &force : forces) {
		// No troops for attacking force
		if (force.State == AiForceAttackingState::Free || force.IsAttacking()) {
			continue;
		}
		for (const AiUnitType &aiut : force.UnitTypes) {
			const unsigned int t = aiut.Type->Slot;
			const int wantedCount = aiut.Want;
			int e = unit_types_count[t];
			if (t < AiHelpers.Equiv().size()) {
				for (unsigned int j = 0; j < AiHelpers.Equiv()[t].size(); ++j) {
					e += unit_types_count[AiHelpers.Equiv()[t][j]->Slot];
				}
			}
			const int requested = wantedCount - (e + counter[t] - attacking[t]);

			if (requested > 0) {  // Request it.
				AiAddUnitTypeRequest(*aiut.Type, requested);
				counter[t] += requested;
				force.Completed = false;
			}
			counter[t] -= wantedCount;
		}
	}
}

/**
**  Cleanup units in forces.
*/
void AiRemoveDeadUnitInForces()
{
	AiPlayer->Force.RemoveDeadUnit();
}

/**
**  Ai assign unit to force.
**
**  @param unit  Unit to assign to force.
*/
bool AiAssignToForce(CUnit &unit)
{
	return AiPlayer->Force.Assign(unit);
}

/**
**  Assign free units to force.
*/
void AiAssignFreeUnitsToForce(int force)
{
	AiRemoveDeadUnitInForces();
	for (CUnit *unit : AiPlayer->Player->GetUnits()) {
		if (unit->Active && unit->GroupId == 0) {
			AiPlayer->Force.Assign(*unit, force);
		}
	}
}

/**
**  Attack at position with force.
**
**  @param force  Force number to attack with.
**  @param x      X tile map position to be attacked.
**  @param y      Y tile map position to be attacked.
*/
void AiAttackWithForceAt(unsigned int force, int x, int y)
{
	const Vec2i pos(x, y);

	if (!(force < AI_MAX_FORCE_INTERNAL)) {
		DebugPrint("Force out of range: %d", force);
		return ;
	}

	if (!Map.Info.IsPointOnMap(pos)) {
		DebugPrint(
			"(%d, %d) not in the map(%d, %d)", pos.x, pos.y, Map.Info.MapWidth, Map.Info.MapHeight);
		return ;
	}
	AiPlayer->Force[force].Attack(pos);
}

/**
**  Attack opponent with force.
**
**  @param force  Force number to attack with.
*/
void AiAttackWithForce(unsigned int force)
{
	if (!(force < AI_MAX_FORCE_INTERNAL)) {
		DebugPrint("Force out of range: %d", force);
		return ;
	}

	unsigned int intForce = AiPlayer->Force.getScriptForce(force);
	// The AI finds the first unassigned force, moves all data to it and cleans
	// the first force, so we can reuse it
	if (!AiPlayer->Force[intForce].Defending) {
		unsigned int f = AiPlayer->Force.FindFreeForce(AiForceRole::Default, AI_MAX_FORCE_INTERNAL);
		AiPlayer->Force[f].Reset();
		AiPlayer->Force[f].FormerForce = force;
		AiPlayer->Force[f].Role = AiPlayer->Force[intForce].Role;

		for (CUnit* aiunit : AiPlayer->Force[intForce].Units) {
			aiunit->GroupId = f + 1;
			AiPlayer->Force[f].Units.push_back(aiunit);
		}
		AiPlayer->Force[intForce].Units.clear();

		for (auto aitype : AiPlayer->Force[intForce].UnitTypes) {
			AiPlayer->Force[f].UnitTypes.push_back(aitype);
		}
		AiPlayer->Force[intForce].UnitTypes.clear();
		AiPlayer->Force[intForce].Reset();
		AiPlayer->Force[f].Completed = true;
		force = f;
	}

	const Vec2i invalidPos(-1, -1);
	AiPlayer->Force[force].Attack(invalidPos);
}

/**
**  Attack opponent with forces.
**  Merge forces in array into one attack force and attack with it
**  Merge is make because units in one force help each other during attack
**
**  @param forces  Array with Force numbers to attack with (array should be finished with -1).
*/
void AiAttackWithForces(int *forces)
{
	const Vec2i invalidPos(-1, -1);
	bool found = false;
	unsigned int f = AiPlayer->Force.FindFreeForce(AiForceRole::Default, AI_MAX_FORCE_INTERNAL);

	AiPlayer->Force[f].Reset();

	for (int i = 0; forces[i] != -1; ++i) {
		int force = forces[i];

		if (!AiPlayer->Force[force].Defending) {
			found = true;

			AiPlayer->Force[f].Role = AiPlayer->Force[force].Role;

			for (CUnit *aiunit : AiPlayer->Force[force].Units) {
				aiunit->GroupId = f + 1;
				AiPlayer->Force[f].Units.push_back(aiunit);
			}
			AiPlayer->Force[force].Units.clear();

			for (auto aitype : AiPlayer->Force[force].UnitTypes) {
				AiPlayer->Force[f].UnitTypes.push_back(aitype);
			}
			AiPlayer->Force[force].UnitTypes.clear();

			AiPlayer->Force[force].Reset();
		} else {
			AiPlayer->Force[force].Attack(invalidPos);
		}
	}
	if (found) {
		AiPlayer->Force[f].Completed = true;
		AiPlayer->Force[f].Attack(invalidPos);
	} else {
		AiPlayer->Force[f].Reset(true);
	}
}


/**
**  Load all unit before attack.
**
**  @param aiForce force to group.
*/
static void AiGroupAttackerForTransport(AiForce &aiForce)
{
	Assert(aiForce.State == AiForceAttackingState::Boarding);

	unsigned int nbToTransport = 0;
	unsigned int transporterIndex = 0;

	for (; transporterIndex < aiForce.Size(); ++transporterIndex) {
		const CUnit &unit = *aiForce.Units[transporterIndex];

		if (unit.Type->CanTransport() && unit.Type->MaxOnBoard - unit.BoardCount > 0) {
			nbToTransport = unit.Type->MaxOnBoard - unit.BoardCount;
			break;
		}
	}
	if (transporterIndex == aiForce.Size()) {
		aiForce.State = AiForceAttackingState::AttackingWithTransporter;
		return ;
	}
	const bool forceIsReady = ranges::none_of(aiForce.Units, [&](const CUnit *unit) {
		const CUnit &transporter = *aiForce.Units[transporterIndex];

		return CanTransport(transporter, *unit) && unit->Container == nullptr;
	});
	if (forceIsReady == true) {
		aiForce.State = AiForceAttackingState::AttackingWithTransporter;
		return ;
	}
	for (CUnit *unitPtr : aiForce.Units) {
		CUnit &unit = *unitPtr;
		CUnit &transporter = *aiForce.Units[transporterIndex];

		if (unit.CurrentAction() == UnitAction::Board
			&& static_cast<COrder_Board *>(unit.CurrentOrder())->GetGoal() == &transporter) {
			CommandFollow(transporter, unit, 0);
		}
		if (CanTransport(transporter, unit)
		    && (unit.IsIdle()
		        || (unit.CurrentAction() == UnitAction::Board && !unit.Moving
		            && static_cast<COrder_Board *>(unit.CurrentOrder())->GetGoal() != &transporter))
		    && unit.Container == nullptr) {
			CommandBoard(unit, transporter, FlushCommands);
			CommandFollow(transporter, unit, 0);
			if (--nbToTransport == 0) { // full : next transporter.
				for (++transporterIndex; transporterIndex < aiForce.Size(); ++transporterIndex) {
					const CUnit &nextTransporter = *aiForce.Units[transporterIndex];

					if (nextTransporter.Type->CanTransport()) {
						nbToTransport =
							nextTransporter.Type->MaxOnBoard - nextTransporter.BoardCount;
						break;
					}
				}
				if (transporterIndex == aiForce.Size()) { // No more transporter.
					break;
				}
			}
		}
	}
}

/**
** Force on attack ride. We attack until there is no unit or enemy left.
**
** @param force Force pointer.
*/
void AiForce::Update()
{
	Assert(Defending == false);
	if (Size() == 0) {
		Attacking = false;
		if (!Defending && State > AiForceAttackingState::Waiting) {
				DebugPrint("%d: Attack force #%lu was destroyed, giving up\n",
			               AiPlayer->Player->Index,
			               (long unsigned int) (this - &(AiPlayer->Force[0])));
			Reset(true);
		}
		return;
	}
	Attacking = ranges::any_of(Units, [](const CUnit *aiunit) { return aiunit->Type->CanAttack; });
	if (Attacking == false) {
		if (!Defending && State > AiForceAttackingState::Waiting) {
			DebugPrint("%d: Attack force #%lu has lost all agresive units, giving up\n",
			           AiPlayer->Player->Index,
			           (long unsigned int) (this - &(AiPlayer->Force[0])));
			Reset(true);
		}
		return ;
	}
#if 0
	if (State == AiForceAttackingState::Waiting) {
		if (!this->PlanAttack()) {
			DebugPrint("Can't transport, look for walls\n");
			if (!AiFindWall(this)) {
				Attacking = false;
				return ;
			}
		}
		State = AiForceAttackingState::Boarding;
	}
#endif
	if (State == AiForceAttackingState::Boarding) {
		AiGroupAttackerForTransport(*this);
		return ;
	}
	if (State == AiForceAttackingState::AttackingWithTransporter) {
		// Move transporters to goalpos
		std::vector<CUnit *> transporters;
		bool emptyTrans = true;
		for (CUnit *aiunit : Units) {
			if (aiunit->CanMove() && aiunit->Type->MaxOnBoard) {
				transporters.push_back(aiunit);
				if (aiunit->BoardCount > 0) {
					emptyTrans = false;
				}
			}
		}
		if (transporters.empty()) {
			// Our transporters have been destroyed
			DebugPrint("%d: Attack force #%lu has lost all agresive units, giving up\n",
			           AiPlayer->Player->Index,
			           (long unsigned int) (this - &(AiPlayer->Force[0])));
			Reset(true);
		} else if (emptyTrans) {
			// We have emptied our transporters, go go go
			State = AiForceAttackingState::GoingToRallyPoint;
		} else {
			for (size_t i = 0; i != transporters.size(); ++i) {
				CUnit &trans = *transporters[i];
				const int delay = i / 5; // To avoid lot of CPU consuption, send them with a small time difference.

				trans.Wait = delay;
				CommandUnload(trans, this->GoalPos, nullptr, FlushCommands);
			}
		}
		return;
	}
	CUnit *leader = nullptr;
	if (auto it = ranges::find_if(Units, &CUnit::IsAgressive); it != Units.end()) {
		leader = *it;
	}

	const int thresholdDist = 5; // Hard coded value
	Assert(Map.Info.IsPointOnMap(GoalPos));
	if (State == AiForceAttackingState::GoingToRallyPoint) {
		// Check if we are near the goalpos
		int minDist = Units[0]->MapDistanceTo(this->GoalPos);
		int maxDist = minDist;

		for (const CUnit *unit : Units) {
			const int distance = unit->MapDistanceTo(this->GoalPos);
			minDist = std::min(minDist, distance);
			maxDist = std::max(maxDist, distance);
		}

		if (WaitOnRallyPoint > 0 && minDist <= thresholdDist) {
			--WaitOnRallyPoint;
		}
		if (maxDist <= thresholdDist || !WaitOnRallyPoint) {
			const CUnit *unit = nullptr;

			AiForceEnemyFinder<AIATTACK_BUILDING>(*this, &unit);
			if (!unit) {
				AiForceEnemyFinder<AIATTACK_ALLMAP>(*this, &unit);
				if (!unit) {
					// No enemy found, give up
					// FIXME: should the force go home or keep trying to attack?
					DebugPrint("%d: Attack force #%lu can't find a target, giving up\n",
					           AiPlayer->Player->Index,
					           (long unsigned int) (this - &(AiPlayer->Force[0])));
					Attacking = false;
					State = AiForceAttackingState::Waiting;
					return;
				}
			}
			this->GoalPos = unit->tilePos;
			State = AiForceAttackingState::Attacking;
			for (size_t i = 0; i != this->Size(); ++i) {
				CUnit &aiunit = *this->Units[i];
				const int delay = i / 5; // To avoid lot of CPU consuption, send them with a small time difference.

				aiunit.Wait = delay;
				if (aiunit.IsAgressive()) {
					CommandAttack(aiunit, this->GoalPos, nullptr, FlushCommands);
				} else {
					if (leader) {
						CommandDefend(aiunit, *leader, FlushCommands);
					} else {
						CommandMove(aiunit, this->GoalPos, FlushCommands);
					}
				}
			}
		}
	}

	std::vector<CUnit *> idleUnits;
	ranges::copy_if(Units, std::back_inserter(idleUnits), &CUnit::IsIdle);

	if (idleUnits.empty()) {
		return;
	}

	if (State == AiForceAttackingState::Attacking && idleUnits.size() == this->Size()) {
		const bool isNaval = ranges::any_of(this->Units, [](const CUnit *unit) {
			return unit->Type->MoveType == EMovement::Naval && unit->Type->CanAttack;
		});
		const CUnit *unit = nullptr;
		if (isNaval) {
			AiForceEnemyFinder<AIATTACK_ALLMAP>(*this, &unit);
		} else {
			AiForceEnemyFinder<AIATTACK_BUILDING>(*this, &unit);
		}
		if (!unit) {
			// No enemy found, give up
			// FIXME: should the force go home or keep trying to attack?
			DebugPrint("%d: Attack force #%lu can't find a target, giving up\n",
			           AiPlayer->Player->Index,
			           (long unsigned int) (this - &(AiPlayer->Force[0])));
			Attacking = false;
			State = AiForceAttackingState::Waiting;
			return;
		} else {
			Vec2i resultPos {-1, -1};
			NewRallyPoint(unit->tilePos, &resultPos);
			if (resultPos.x != -1 && resultPos.y != -1) {
				this->GoalPos = resultPos;
				this->State = AiForceAttackingState::GoingToRallyPoint;
			} else {
				this->GoalPos = unit->tilePos;
				this->State = AiForceAttackingState::Attacking;
			}
		}
	}
	for (size_t i = 0; i != idleUnits.size(); ++i) {
		CUnit &aiunit = *idleUnits[i];
		const int delay = i / 5; // To avoid lot of CPU consuption, send them with a small time difference.

		aiunit.Wait = delay;
		if (leader) {
			if (aiunit.IsAgressive()) {
				if (State == AiForceAttackingState::Attacking) {
					CommandAttack(aiunit, leader->tilePos, nullptr, FlushCommands);
				} else {
					CommandAttack(aiunit, this->GoalPos, nullptr, FlushCommands);
				}
			} else {
				CommandDefend(aiunit, *leader, FlushCommands);
			}
		} else {
			if (aiunit.IsAgressive()) {
				CommandAttack(aiunit, this->GoalPos, nullptr, FlushCommands);
			} else {
				CommandMove(aiunit, this->GoalPos, FlushCommands);
			}
		}
	}
}

void AiForceManager::Update()
{
	int maxPathing = 2; // reduce load by stopping after issuing a few map searches
	for (AiForce &force : forces) {
		if (maxPathing < 0) {
			return;
		}

		//  Look if our defenders still have enemies in range.

		if (force.Defending) {
			force.RemoveDeadUnit();

			if (force.Size() == 0) {
				force.Attacking = false;
				force.Defending = false;
				force.State = AiForceAttackingState::Waiting;
				continue;
			}
			const int nearDist = 5;

			if (Map.Info.IsPointOnMap(force.GoalPos) == false) {
				force.ReturnToHome();
			} else {
				//  Check if some unit from force reached goal point
				for (const CUnit *aiunit : force.Units) {
					if (aiunit->MapDistanceTo(force.GoalPos) <= nearDist) {
						//  Look if still enemies in attack range.
						const CUnit *dummy = nullptr;
						maxPathing--;
						if (!AiForceEnemyFinder<AIATTACK_RANGE>(force, &dummy).found()) {
							force.ReturnToHome();
						}
					}
				}

				if (force.Defending == false) {
					// force is no longer defending
					return;
				}

				// Find idle units and order them to defend
				// Don't attack if there aren't our units near goal point
				const Vec2i offset(15, 15);
				maxPathing--;
				std::vector<CUnit *> nearGoal = Select(force.GoalPos - offset,
				                                       force.GoalPos + offset,
				                                       IsAnAlliedUnitOf(*force.Units[0]->Player));
				if (nearGoal.empty()) {
					force.ReturnToHome();
				} else {
					std::vector<CUnit *> idleUnits;
					ranges::copy_if(
						force.Units, std::back_inserter(idleUnits), [](const CUnit *aiunit) {
							return aiunit->IsIdle() && aiunit->IsAliveOnMap();
						});
					for (unsigned int i = 0; i != idleUnits.size(); ++i) {
						CUnit *const unit = idleUnits[i];

						if (unit->Container == nullptr) {
							const int delay = i / 5; // To avoid lot of CPU consuption, send them with a small time difference.

							unit->Wait = delay;
							if (unit->Type->CanAttack) {
								CommandAttack(*unit, force.GoalPos, nullptr, FlushCommands);
							} else {
								CommandMove(*unit, force.GoalPos, FlushCommands);
							}
						}
					}
				}
			}
		} else if (force.Attacking) {
			force.RemoveDeadUnit();
			maxPathing--;
			force.Update();
		}
	}
}

/**
**  Entry point of force manager, periodic called.
*/
void AiForceManager()
{
	AiPlayer->Force.Update();
	AiAssignFreeUnitsToForce();
}

//@}
