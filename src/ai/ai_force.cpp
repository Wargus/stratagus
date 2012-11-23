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
//      (c) Copyright 2001-2005 by Lutz Sammer and Jimmy Salmon
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
#include "unit.h"
#include "unit_find.h"
#include "unittype.h"

/*----------------------------------------------------------------------------
--  Types
----------------------------------------------------------------------------*/
#define AIATTACK_RANGE 0
#define AIATTACK_ALLMAP 1
#define AIATTACK_BUILDING 2

template <const int FIND_TYPE>
class AiForceEnemyFinder
{
public:
	AiForceEnemyFinder(int force, const CUnit **enemy) : enemy(enemy) {
		Assert(enemy != NULL);
		*enemy = NULL;
		AiPlayer->Force[force].Units.for_each_if(*this);
	}

	AiForceEnemyFinder(AiForce &force, const CUnit **enemy) : enemy(enemy) {
		Assert(enemy != NULL);
		*enemy = NULL;
		force.Units.for_each_if(*this);
	}

	bool found() const { return *enemy != NULL; }

	bool operator()(const CUnit *const unit) const {
		if (unit->Type->CanAttack == false) {
			return *enemy == NULL;
		}
		if (FIND_TYPE == AIATTACK_RANGE) {
			*enemy = AttackUnitsInReactRange(*unit);
		} else if (FIND_TYPE == AIATTACK_ALLMAP) {
			*enemy = AttackUnitsInDistance(*unit, MaxMapWidth);
		} else if (FIND_TYPE == AIATTACK_BUILDING) {
			*enemy = AttackUnitsInDistance(*unit, MaxMapWidth, true);
			Assert(!*enemy || (*enemy)->Type->Building);
			if (*enemy == NULL) {
				*enemy = AttackUnitsInDistance(*unit, MaxMapWidth);
			}
		}
		return *enemy == NULL;
	}
private:
	const CUnit **enemy;
};

class IsAnAlliedUnitOf
{
public:
	explicit IsAnAlliedUnitOf(const CPlayer &_player) : player(&_player) {}
	bool operator()(const CUnit *unit) const {
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
	for (int i = 0; i <= UnitTypeMax; ++i) {
		UnitTypeEquivs[i] = i;
	}
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
	for (unsigned int i = 0; i <= UnitTypeMax; ++i) {
		if (UnitTypeEquivs[i] == find) {
			UnitTypeEquivs[i] = replace;
		}
	}
}


/**
**  Find All unittypes equivalent to a given one
**
**  @param unittype  the unittype to find equivalence for
**  @param result    int array which will hold the result. (Size UnitTypeMax+1)
**
**  @return          the number of unittype found
*/
int AiFindUnitTypeEquiv(const CUnitType &unittype, int *result)
{
	const int search = UnitTypeEquivs[unittype.Slot];
	int count = 0;

	for (int i = 0; i < UnitTypeMax + 1; ++i) {
		if (UnitTypeEquivs[i] == search) {
			// Found one
			result[count] = i;
			++count;
		}
	}
	return count;
}

class UnitTypePrioritySorter_Decreasing
{
public:
	bool operator()(int lhs, int rhs) const {
		return UnitTypes[lhs]->Priority > UnitTypes[rhs]->Priority;
	}
};

/**
**  Find All unittypes equivalent to a given one, and which are available
**  UnitType are returned in the prefered order ( ie palladin >> knight... )
**
**  @param unittype     The unittype to find equivalence for
**  @param usableTypes  int array which will hold the result. (Size UnitTypeMax+1)
**
**  @return             the number of unittype found
*/
int AiFindAvailableUnitTypeEquiv(const CUnitType &unittype, int *usableTypes)
{
	// 1 - Find equivalents
	int usableTypesCount = AiFindUnitTypeEquiv(unittype, usableTypes);
	// 2 - Remove unavailable unittypes
	for (int i = 0; i < usableTypesCount;) {
		if (!CheckDependByIdent(*AiPlayer->Player, UnitTypes[usableTypes[i]]->Ident)) {
			// Not available, remove it
			usableTypes[i] = usableTypes[usableTypesCount - 1];
			--usableTypesCount;
		} else {
			++i;
		}
	}
	// 3 - Sort by level
	std::sort(usableTypes, usableTypes + usableTypesCount, UnitTypePrioritySorter_Decreasing());
	return usableTypesCount;
}

/* =========================== FORCES ========================== */

class AiForceCounter
{
public:
	AiForceCounter(CUnitCache &units, unsigned int *d, const size_t len) : data(d) {
		memset(data, 0, len);
		units.for_each(*this);
	}
	inline void operator()(const CUnit *const unit) const {
		data[UnitTypeEquivs[unit->Type->Slot]]++;
	}
private:
	unsigned int *data;//[UnitTypeMax + 1];
};

void AiForce::CountTypes(unsigned int *counter, const size_t len)
{
	AiForceCounter(Units, counter, len);
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
	for (unsigned int i = 0; i < UnitTypes.size(); ++i) {
		const AiUnitType &aitype = UnitTypes[i];
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
	Units.Insert(&unit);
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
	for (unsigned int i = 0; i != Units.size();) {
		CUnit &aiunit = *Units[i];

		if (!aiunit.IsAlive()) {
			InternalRemoveUnit(&aiunit);
			Units.Remove(i);
			continue;
		}
		++i;
	}
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
	if (AiEnemyUnitsInDistance(*startUnit.Player, NULL, pos, minDist) == false
		&& Distance(pos, startPos) <= abs(distance - minDist)) {
		*resultPos = pos;
		return VisitResult_Finished;
	}
	if (CanMoveToMask(pos, movemask)) { // reachable
		return VisitResult_Ok;
	} else { // unreachable
		return VisitResult_DeadEnd;
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
		this->State = AiForceAttackingState_Waiting;
		return;
	}
	if (!this->Attacking) {
		// Remember the original force position so we can return there after attack
		if (this->Role == AiForceRoleDefend
			|| (this->Role == AiForceRoleAttack && this->State == AiForceAttackingState_Waiting)) {
			this->HomePos = this->Units[this->Units.size() - 1]->tilePos;
		}
		this->Attacking = true;
	}
	Vec2i goalPos(pos);

	if (Map.Info.IsPointOnMap(goalPos) == false) {
		/* Search in entire map */
		const CUnit *enemy = NULL;
		AiForceEnemyFinder<AIATTACK_BUILDING>(*this, &enemy);
		if (enemy) {
			goalPos = enemy->tilePos;
		}
	} else {
		isDefenceForce = true;
	}
	if (Map.Info.IsPointOnMap(goalPos) == false) {
		DebugPrint("%d: Need to plan an attack with transporter\n" _C_ AiPlayer->Player->Index);
		if (State == AiForceAttackingState_Waiting && !PlanAttack()) {
			DebugPrint("%d: Can't transport\n" _C_ AiPlayer->Player->Index);
			Attacking = false;
		}
		return;
	}
	if (this->State == AiForceAttackingState_Waiting && isDefenceForce == false) {
		Vec2i resultPos;
		NewRallyPoint(goalPos, &resultPos);
		this->GoalPos = resultPos;
		this->State = AiForceAttackingState_GoingToRallyPoint;
	} else {
		this->GoalPos = goalPos;
		this->State = AiForceAttackingState_Attacking;
	}
	//  Send all units in the force to enemy.

	for (size_t i = 0; i != this->Units.size(); ++i) {
		CUnit *const unit = this->Units[i];

		if (unit->Container == NULL) {
			const int delay = i / 5; // To avoid lot of CPU consuption, send them with a small time difference.

			unit->Wait = delay;
			if (unit->Type->CanAttack) {
				CommandAttack(*unit, this->GoalPos,  NULL, FlushCommands);
			} else {
				CommandMove(*unit, this->GoalPos, FlushCommands);
			}
		}
	}
}

void AiForce::ReturnToHome()
{
	if (Map.Info.IsPointOnMap(this->HomePos)) {
		for (size_t i = 0; i != this->Units.size(); ++i) {
			CUnit &unit = *this->Units[i];
			CommandMove(unit, this->HomePos, FlushCommands);
		}
	}
	const Vec2i invalidPos(-1, -1);

	this->HomePos = invalidPos;
	this->GoalPos = invalidPos;
	this->Defending = false;
	this->Attacking = false;
	this->State = AiForceAttackingState_Waiting;
}

AiForceManager::AiForceManager()
{
	forces.resize(3);
	memset(script, -1, AI_MAX_FORCES * sizeof(char));
}

unsigned int AiForceManager::FindFreeForce(AiForceRole role)
{
	/* find free force */
	unsigned int f = 0;
	while (f < forces.size() && (forces[f].State > AiForceAttackingState_Free)) {
		++f;
	};
	if (f == forces.size()) {
		forces.resize(f + 1);
	}
	forces[f].State = AiForceAttackingState_Waiting;
	forces[f].Role = role;
	return f;
}

/**
**  Cleanup units in forces.
*/
void AiForceManager::RemoveDeadUnit()
{
	for (unsigned int i = 0; i < forces.size(); ++i) {
		forces[i].RemoveDeadUnit();
	}
}

/**
**  Ai assign unit to force.
**
**  @param unit  Unit to assign to force.
*/
bool AiForceManager::Assign(CUnit &unit)
{
	if (unit.GroupId != 0) {
		return false;
	}
	// Check to which force it belongs
	for (unsigned int i = 0; i < forces.size(); ++i) {
		AiForce &force = forces[i];
		// No troops for attacking force
		if (force.IsAttacking()) {
			continue;
		}
		if (force.IsBelongsTo(*unit.Type)) {
			force.Insert(unit);
			unit.GroupId = i + 1;
			return true;
		}
	}
	return false;
}

void AiForceManager::CheckUnits(int *counter)
{
	int attacking[UnitTypeMax];
	const int *unit_types_count = AiPlayer->Player->UnitTypesCount;

	memset(attacking, 0, sizeof(attacking));

	// Look through the forces what is missing.
	for (unsigned int i = 0; i < forces.size(); ++i) {
		const AiForce &force = forces[i];

		if (force.State > AiForceAttackingState_Free && force.IsAttacking()) {
			for (unsigned int j = 0; j < force.Size(); ++j) {
				const CUnit *unit = force.Units[j];
				attacking[unit->Type->Slot]++;
			}
		}
	}
	// create missing units
	for (unsigned int i = 0; i < forces.size(); ++i) {
		AiForce &force = forces[i];

		// No troops for attacking force
		if (force.State == AiForceAttackingState_Free || force.IsAttacking()) {
			continue;
		}
		for (unsigned int j = 0; j < force.UnitTypes.size(); ++j) {
			const AiUnitType &aiut = force.UnitTypes[j];
			const int t = aiut.Type->Slot;
			const int wantedCount = aiut.Want;
			const int requested = wantedCount - (unit_types_count[t] + counter[t] - attacking[t]);

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
void AiAssignFreeUnitsToForce()
{
	const int n = AiPlayer->Player->GetUnitCount();

	AiRemoveDeadUnitInForces();
	for (int i = 0; i < n; ++i) {
		CUnit &unit = AiPlayer->Player->GetUnit(i);

		if (unit.Active && unit.GroupId == 0) {
			AiPlayer->Force.Assign(unit);
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

	if (!(force < AI_MAX_FORCES)) {
		DebugPrint("Force out of range: %d" _C_ force);
		return ;
	}

	if (!Map.Info.IsPointOnMap(pos)) {
		DebugPrint("(%d, %d) not in the map(%d, %d)" _C_ pos.x _C_ pos.y
				   _C_ Map.Info.MapWidth _C_ Map.Info.MapHeight);
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
	if (!(force < AI_MAX_FORCES)) {
		DebugPrint("Force out of range: %d" _C_ force);
		return ;
	}

	// The AI finds the first unassigned force, moves all data to it and cleans
	// the first force, so we can reuse it
	if (!AiPlayer->Force[force].Defending) {
		unsigned int top;
		unsigned int f = AiPlayer->Force.FindFreeForce();
		AiPlayer->Force[f].Reset();

		AiPlayer->Force[f].Role = AiPlayer->Force[force].Role;

		while (AiPlayer->Force[force].Size()) {
			CUnit &aiunit = *AiPlayer->Force[force].Units[AiPlayer->Force[force].Size() - 1];
			aiunit.GroupId = f + 1;
			AiPlayer->Force[force].Units.Remove(&aiunit);
			AiPlayer->Force[f].Units.Insert(&aiunit);
		}

		while (AiPlayer->Force[force].UnitTypes.size()) {
			top = AiPlayer->Force[force].UnitTypes.size() - 1;
			AiPlayer->Force[f].UnitTypes.push_back(AiPlayer->Force[force].UnitTypes[top]);
			AiPlayer->Force[force].UnitTypes.pop_back();
		}
		AiPlayer->Force[force].Reset();
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
	unsigned int top;
	unsigned int f = AiPlayer->Force.FindFreeForce();

	AiPlayer->Force[f].Reset();

	for (int i = 0; forces[i] != -1; ++i) {
		int force = forces[i];

		if (!AiPlayer->Force[force].Defending) {
			found = true;

			AiPlayer->Force[f].Role = AiPlayer->Force[force].Role;

			while (AiPlayer->Force[force].Size()) {
				CUnit &aiunit = *AiPlayer->Force[force].Units[AiPlayer->Force[force].Size() - 1];
				aiunit.GroupId = f + 1;
				AiPlayer->Force[force].Units.Remove(&aiunit);
				AiPlayer->Force[f].Units.Insert(&aiunit);
			}
			while (AiPlayer->Force[force].UnitTypes.size()) {
				top = AiPlayer->Force[force].UnitTypes.size() - 1;
				AiPlayer->Force[f].UnitTypes.push_back(AiPlayer->Force[force].UnitTypes[top]);
				AiPlayer->Force[force].UnitTypes.pop_back();
			}
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
	Assert(aiForce.State == AiForceAttackingState_Boarding);

	unsigned int nbToTransport = 0;
	unsigned int transporterIndex = 0;
	bool forceIsReady = true;

	for (; transporterIndex < aiForce.Size(); ++transporterIndex) {
		const CUnit &unit = *aiForce.Units[transporterIndex];

		if (unit.Type->CanTransport() && unit.Type->MaxOnBoard - unit.BoardCount > 0) {
			nbToTransport = unit.Type->MaxOnBoard - unit.BoardCount;
			break;
		}
	}
	if (transporterIndex == aiForce.Size()) {
		aiForce.State = AiForceAttackingState_AttackingWithTransporter;
		return ;
	}
	for (unsigned int i = 0; i < aiForce.Size(); ++i) {
		const CUnit &unit = *aiForce.Units[i];
		const CUnit &transporter = *aiForce.Units[transporterIndex];

		if (CanTransport(transporter, unit) && unit.Container == NULL) {
			forceIsReady = false;
			break;
		}
	}
	if (forceIsReady == true) {
		aiForce.State = AiForceAttackingState_AttackingWithTransporter;
		return ;
	}
	for (unsigned int i = 0; i < aiForce.Size(); ++i) {
		CUnit &unit = *aiForce.Units[i];
		CUnit &transporter = *aiForce.Units[transporterIndex];

		if (transporter.IsIdle()
			&& unit.CurrentAction() == UnitActionBoard
			&& static_cast<COrder_Board *>(unit.CurrentOrder())->GetGoal() == &transporter) {
			CommandFollow(transporter, unit, 0);
		}
		if (CanTransport(transporter, unit) && unit.IsIdle() && unit.Container == NULL) {
			CommandBoard(unit, transporter, FlushCommands);
			CommandFollow(transporter, unit, 0);
			if (--nbToTransport == 0) { // full : next transporter.
				for (++transporterIndex; transporterIndex < aiForce.Size(); ++transporterIndex) {
					const CUnit &nextTransporter = *aiForce.Units[transporterIndex];

					if (nextTransporter.Type->CanTransport()) {
						nbToTransport = nextTransporter.Type->MaxOnBoard - nextTransporter.BoardCount;
						break ;
					}
				}
				if (transporterIndex == aiForce.Size()) { // No more transporter.
					break ;
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
		if (!Defending && State > AiForceAttackingState_Waiting) {
			DebugPrint("%d: Attack force #%lu was destroyed, giving up\n"
					   _C_ AiPlayer->Player->Index _C_(long unsigned int)(this  - & (AiPlayer->Force[0])));
			Reset(true);
		}
		return;
	}
	Attacking = false;
	for (unsigned int i = 0; i < Size(); ++i) {
		CUnit *aiunit = Units[i];

		if (aiunit->Type->CanAttack) {
			Attacking = true;
			break;
		}
	}
	if (Attacking == false) {
		if (!Defending && State > AiForceAttackingState_Waiting) {
			DebugPrint("%d: Attack force #%lu has lost all agresive units, giving up\n"
					   _C_ AiPlayer->Player->Index _C_(long unsigned int)(this  - & (AiPlayer->Force[0])));
			Reset(true);
		}
		return ;
	}
#if 0
	if (State == AiForceAttackingState_Waiting) {
		if (!this->PlanAttack()) {
			DebugPrint("Can't transport, look for walls\n");
			if (!AiFindWall(this)) {
				Attacking = false;
				return ;
			}
		}
		State = AiForceAttackingState_Boarding;
	}
#endif
	if (State == AiForceAttackingState_Boarding) {
		AiGroupAttackerForTransport(*this);
		return ;
	}

	std::vector<CUnit *> idleUnits;
	for (unsigned int i = 0; i != Size(); ++i) {
		CUnit &aiunit = *Units[i];

		if (aiunit.IsIdle() && aiunit.IsAliveOnMap()) {
			idleUnits.push_back(&aiunit);
		}
	}

	if (idleUnits.empty()) {
		return;
	}

	const int thresholdDist = 5; // Hard coded value
	Assert(Map.Info.IsPointOnMap(GoalPos));
	if (State == AiForceAttackingState_GoingToRallyPoint) {
		// Check if we are near the goalpos
		int minDist = Units[0]->MapDistanceTo(this->GoalPos);
		int maxDist = minDist;

		for (size_t i = 0; i != Size(); ++i) {
			const int distance = Units[i]->MapDistanceTo(this->GoalPos);
			minDist = std::min(minDist, distance);
			maxDist = std::max(maxDist, distance);
		}

		if (WaitOnRallyPoint > 0 && minDist <= thresholdDist) {
			--WaitOnRallyPoint;
		}
		if (maxDist <= thresholdDist || !WaitOnRallyPoint) {
			const CUnit *unit = NULL;

			AiForceEnemyFinder<AIATTACK_BUILDING>(*this, &unit);
			if (!unit) {
				AiForceEnemyFinder<AIATTACK_ALLMAP>(*this, &unit);
				if (!unit) {
					// No enemy found, give up
					// FIXME: should the force go home or keep trying to attack?
					DebugPrint("%d: Attack force #%lu can't find a target, giving up\n"
							   _C_ AiPlayer->Player->Index _C_(long unsigned int)(this - & (AiPlayer->Force[0])));
					Attacking = false;
					State = AiForceAttackingState_Waiting;
					return;
				}
			}
			this->GoalPos = unit->tilePos;
			State = AiForceAttackingState_Attacking;
		}
	}

	for (size_t i = 0; i != idleUnits.size(); ++i) {
		CUnit &aiunit = *idleUnits[i];
		const int delay = i / 5; // To avoid lot of CPU consuption, send them with a small time difference.

		aiunit.Wait = delay;
		if (aiunit.Type->CanAttack) {
			CommandAttack(aiunit, this->GoalPos, NULL, FlushCommands);
		} else if (aiunit.Type->CanTransport()) {
			if (aiunit.BoardCount != 0) {
				CommandUnload(aiunit, this->GoalPos, NULL, FlushCommands);
			} else {
				// FIXME : Retrieve unit blocked (transport previously full)
				CommandMove(aiunit, aiunit.Player->StartPos, FlushCommands);
				this->Remove(aiunit);
			}
		} else {
			CommandMove(aiunit, this->GoalPos, FlushCommands);
		}
	}

	if (State == AiForceAttackingState_Attacking) {
		int maxDist = 0;

		for (size_t i = 0; i != Size(); ++i) {
			maxDist = std::max(maxDist, Units[i]->MapDistanceTo(this->GoalPos));
		}
		if (maxDist < thresholdDist) {
			const CUnit *unit = NULL;

			AiForceEnemyFinder<AIATTACK_BUILDING>(*this, &unit);
			if (!unit) {
				// No enemy found, give up
				// FIXME: should the force go home or keep trying to attack?
				DebugPrint("%d: Attack force #%lu can't find a target, giving up\n"
						   _C_ AiPlayer->Player->Index _C_(long unsigned int)(this - & (AiPlayer->Force[0])));
				Attacking = false;
				State = AiForceAttackingState_Waiting;
				return;
			} else {
				Vec2i resultPos;
				NewRallyPoint(unit->tilePos, &resultPos);
				this->GoalPos = resultPos;
				this->State = AiForceAttackingState_GoingToRallyPoint;
			}
		}
	}
}

void AiForceManager::Update()
{
	for (unsigned int f = 0; f < forces.size(); ++f) {
		AiForce &force = forces[f];
		//  Look if our defenders still have enemies in range.

		if (force.Defending) {
			force.RemoveDeadUnit();

			if (force.Size() == 0) {
				force.Attacking = false;
				force.Defending = false;
				force.State = AiForceAttackingState_Waiting;
				continue;
			}
			const int nearDist = 5;

			if (Map.Info.IsPointOnMap(force.GoalPos) == false) {
				force.ReturnToHome();
				//  Check if some unit from force reached goal point
			} else if (force.Units[0]->MapDistanceTo(force.GoalPos) <= nearDist) {
				//  Look if still enemies in attack range.
				const CUnit *dummy = NULL;
				if (!AiForceEnemyFinder<AIATTACK_RANGE>(force, &dummy).found()) {
					force.ReturnToHome();
				}
			} else { // Find idle units and order them to defend
				// Don't attack if there aren't our units near goal point
				std::vector<CUnit *> nearGoal;
				const Vec2i offset(15, 15);
				Select(force.GoalPos - offset, force.GoalPos + offset, nearGoal,
					   IsAnAlliedUnitOf(*force.Units[0]->Player));
				if (nearGoal.empty()) {
					force.ReturnToHome();
				} else {
					std::vector<CUnit *> idleUnits;
					for (unsigned int i = 0; i != force.Size(); ++i) {
						CUnit &aiunit = *force.Units[i];

						if (aiunit.IsIdle() && aiunit.IsAliveOnMap()) {
							idleUnits.push_back(&aiunit);
						}
					}
					for (unsigned int i = 0; i != idleUnits.size(); ++i) {
						CUnit *const unit = idleUnits[i];

						if (unit->Container == NULL) {
							const int delay = i / 5; // To avoid lot of CPU consuption, send them with a small time difference.

							unit->Wait = delay;
							if (unit->Type->CanAttack) {
								CommandAttack(*unit, force.GoalPos, NULL, FlushCommands);
							} else {
								CommandMove(*unit, force.GoalPos, FlushCommands);
							}
						}
					}
				}
			}
		} else if (force.Attacking) {
			force.RemoveDeadUnit();
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
