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
//      $Id$

//@{

/*----------------------------------------------------------------------------
--  Includes
----------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "stratagus.h"
#include "unittype.h"
#include "unit.h"
#include "ai_local.h"
#include "actions.h"
#include "map.h"
#include "depend.h"
#include "pathfinder.h"
#include "player.h"

/*----------------------------------------------------------------------------
--  Types
----------------------------------------------------------------------------*/

struct AiForceUnitsCounter {
	unsigned int data[UnitTypeMax + 1];

	inline void operator() (CUnit *const unit) {
		data[UnitTypeEquivs[unit->Type->Slot]]++;
	}	

	inline void count(AiForce *force)
	{
		memset(data, 0, sizeof(data));
		force->Units.for_each(*this);
	}

	AiForceUnitsCounter(int force) {
		count(&AiPlayer->Force[force]);
	}
	
	AiForceUnitsCounter(AiForce *force) {
		count(force);
	}
};

template <const bool IN_REACT_RANGE>
struct AiForceEnemyFinder {
	const CUnit *enemy;
	
	inline bool found(void) {
		return enemy != NULL;
	}
	
	inline bool operator() (const CUnit *const unit) 
	{
		if(IN_REACT_RANGE) {
			if (unit->Type->CanAttack)
				enemy = AttackUnitsInReactRange(unit);
		} else {
			if (unit->Type->CanAttack)
				enemy = AttackUnitsInDistance(unit, MaxMapWidth);
		}
		return enemy == NULL;
	}	

	AiForceEnemyFinder(int force): enemy(NULL) {
		AiPlayer->Force[force].Units.for_each_if(*this);
	}
	
	AiForceEnemyFinder(AiForce *force):	enemy(NULL) {
		force->Units.for_each_if(*this);
	}
	
};

struct AiForceAttackSender {
	int goalX;
	int goalY;
	int delta;
	
	inline void operator() (CUnit *const unit) {
		// To avoid lot of CPU consuption, send them with a small time difference.	
		unit->Wait = delta;
		++delta;
		if (unit->Type->CanAttack) {
			CommandAttack(unit, goalX, goalY, NULL, FlushCommands);
		} else {
			CommandMove(unit, goalX, goalY, FlushCommands);
		}
	}	

	//
	//  Send all units in the force to enemy at x,y.
	//
	AiForceAttackSender(int force, int x, int y): goalX(x), goalY(y), delta(0) {
		DebugPrint("%d: Attacking with force #%d\n" _C_ AiPlayer->Player->Index _C_ force);
		AiForce *fptr = &AiPlayer->Force[force];
		fptr->Attacking = true;
		fptr->State = AI_FORCE_STATE_ATTACKING;
		fptr->Units.for_each(*this);
	}

	AiForceAttackSender(AiForce *force, int x, int y):
		 goalX(x), goalY(y), delta(0) {
		 
		DebugPrint("%d: Attacking with force #%lu\n" _C_ AiPlayer->Player->Index
			 _C_ (force  - &(AiPlayer->Force[0])));
		force->Attacking = true;
		force->State = AI_FORCE_STATE_ATTACKING;
		force->Units.for_each(*this);
	}

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
void AiResetUnitTypeEquiv(void)
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
void AiNewUnitTypeEquiv(CUnitType *a, CUnitType *b)
{
	int find;
	int replace;
	int i;

	find = UnitTypeEquivs[a->Slot];
	replace = UnitTypeEquivs[b->Slot];

	// Always record equivalences with the lowest unittype.
	if (find < replace) {
		i = find;
		find = replace;
		replace = i;
	}

	// Then just find & replace in UnitTypeEquivs...
	for (i = 0; i <= UnitTypeMax; ++i) {
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
int AiFindUnitTypeEquiv(const CUnitType *unittype, int *result)
{
	int i;
	int search;
	int count;

	search = UnitTypeEquivs[unittype->Slot];
	count = 0;

	for (i = 0; i < UnitTypeMax + 1; ++i) {
		if (UnitTypeEquivs[i] == search) {
			// Found one
			result[count] = i;
			count++;
		}
	}

	return count;
}

/**
**  Find All unittypes equivalent to a given one, and which are available
**  UnitType are returned in the prefered order ( ie palladin >> knight... )
**
**  @param unittype     The unittype to find equivalence for
**  @param usableTypes  int array which will hold the result. (Size UnitTypeMax+1)
**
**  @return             the number of unittype found
*/
int AiFindAvailableUnitTypeEquiv(const CUnitType *unittype, int *usableTypes)
{
	int usableTypesCount;
	int i;
	int j;
	int tmp;
	int playerid;
	int bestlevel;
	int curlevel;

	// 1 - Find equivalents
	usableTypesCount = AiFindUnitTypeEquiv(unittype,  usableTypes);

	// 2 - Remove unavailable unittypes
	for (i = 0; i < usableTypesCount; ) {
		if (!CheckDependByIdent(AiPlayer->Player, UnitTypes[usableTypes[i]]->Ident)) {
			// Not available, remove it
			usableTypes[i] = usableTypes[usableTypesCount - 1];
			--usableTypesCount;
		} else {
			++i;
		}
	}

	// 3 - Sort by level
	playerid = AiPlayer->Player->Index;

	// We won't have usableTypesCount>4, so simple sort should do it
	for (i = 0; i < usableTypesCount - 1; ++i) {
		bestlevel = UnitTypes[usableTypes[i]]->Priority;
		for (j = i + 1; j < usableTypesCount; ++j) {
			curlevel = UnitTypes[usableTypes[j]]->Priority;

			if (curlevel > bestlevel) {
				// Swap
				tmp = usableTypes[j];
				usableTypes[j] = usableTypes[i];
				usableTypes[i] = tmp;

				bestlevel = curlevel;
			}
		}
	}

	return usableTypesCount;
}

/**
**  Ai clean units in a force.
**
**  @param force  Force number.
*/
static void AiCleanForce(const unsigned int force)
{
	CUnit *aiunit;
	const AiUnitType *aitype;
	unsigned int i;
	CUnitCache &units = AiPlayer->Force[force].Units;

	//
	// Release all killed units.
	//
	i = 0;
	while (i != units.size()) {
		aiunit = units[i];
		if (aiunit->Destroyed || aiunit->CurrentAction() == UnitActionDie) {
			aiunit->RefsDecrease();
			aiunit->GroupId &= ~(1 << force);
			units.Remove(i);
			continue;
		}
		++i;
	}

	//
	// Count units in force.
	//
	AiForceUnitsCounter counter(force);

	//
	// Look if the force is complete.
	//
	AiPlayer->Force[force].Completed = true;
	for (i = 0; i < AiPlayer->Force[force].UnitTypes.size(); ++i) {
		aitype = &AiPlayer->Force[force].UnitTypes[i];
		if (aitype->Want > counter.data[aitype->Type->Slot]) {
			AiPlayer->Force[force].Completed = false;
		}
		counter.data[aitype->Type->Slot] -= aitype->Want;
	}

	//
	// Release units too much in force.
	//
	if (!AiPlayer->Force[force].Attacking) {
		i = 0;
		while (i != units.size()) {
			aiunit = units[i];
			
			if (counter.data[aiunit->Type->Slot] > 0) {
				counter.data[UnitTypeEquivs[aiunit->Type->Slot]]--;
				aiunit->RefsDecrease();
				aiunit->GroupId &= ~(1 << force);
				units.Remove(i);
				continue;
			}
			++i;
		}
	}
}

/**
**  Cleanup units in forces.
*/
void AiCleanForces(void)
{
	//
	// Release all killed units.
	//
	for (int force = 0; force < AI_MAX_ATTACKING_FORCES; ++force) {
		AiCleanForce(force);
	}
}

/**
**  Check if the units belongs to the force.
**
**  @param force  Force to be checked.
**  @param type   Type to check.
**
**  @return       True if it fits, false otherwise.
*/
static bool AiCheckBelongsToForce(unsigned int force, const CUnitType *type)
{
	AiUnitType *aitype;
	bool flag;
	unsigned int i;
	int slot;

	//
	// Count units in force.
	//
	AiForceUnitsCounter counter(force);

	//
	// Look what should be in the force.
	//
	flag = false;
	AiPlayer->Force[force].Completed = true;
	for (i = 0; i < AiPlayer->Force[force].UnitTypes.size(); ++i) {
		aitype = &AiPlayer->Force[force].UnitTypes[i];
		slot = aitype->Type->Slot;
		if (aitype->Want > counter.data[slot]) {
			if (UnitTypeEquivs[type->Slot] == slot) {
				if (aitype->Want - 1 > counter.data[slot]) {
					AiPlayer->Force[force].Completed = false;
				}
				flag = true;
			} else {
				AiPlayer->Force[force].Completed = false;
			}
		}
	}
	return flag;
}

/**
**  Ai assign unit to force.
**
**  @param unit  Unit to assign to force.
*/
bool AiAssignToForce(CUnit *unit)
{
	//
	// Check to which force it belongs
	//
	for (unsigned int force = 0; force < AI_MAX_ATTACKING_FORCES; ++force) {
		// No troops for attacking force
		if (!AiPlayer->Force[force].Defending &&
				AiPlayer->Force[force].Attacking) {
			continue;
		}

		if (unit->GroupId == 0 && AiCheckBelongsToForce(force, unit->Type)) {
			AiPlayer->Force[force].Units.Insert(unit);
			unit->RefsIncrease();
			unit->GroupId |= (1 << force);//like groups
			return true;
		}
	}
	return false;
}

/**
**  Assign free units to force.
*/
void AiAssignFreeUnitsToForce(void)
{
	int i = 0;
	int n = AiPlayer->Player->TotalNumUnits;
	CUnit **table = AiPlayer->Player->Units;
	AiCleanForces();
	for (; i < n; ++i) {
		CUnit *unit = table[i];
		if(unit->Active && unit->GroupId == 0)
		{
			AiAssignToForce(table[i]);
		}
	}
}

/**
**
*/
static void AiAttack(unsigned int &force, int goalX, int goalY)
{
	Assert(force < AI_MAX_FORCES);

	// Move the force to a free position so it can be used for a new
	// attacking party
	if (!AiPlayer->Force[force].Defending && force < AI_MAX_FORCES) {
		unsigned int f = AI_MAX_FORCES;

		while (AiPlayer->Force[f].Attacking) {
			++f;
			if (f == AI_MAX_ATTACKING_FORCES) {
				DebugPrint("No free attacking forces\n");
				f = force;
				break;
			}
		}
		if (f != AI_MAX_ATTACKING_FORCES) {
			AiPlayer->Force[f] = AiPlayer->Force[force];
			for (unsigned int j = 0; j < AiPlayer->Force[force].Units.size(); ++j) {
				CUnit *aiunit = AiPlayer->Force[force].Units[j];
				aiunit->GroupId &= ~(1 << force);
				aiunit->GroupId |= (1 << f);
			}
			Assert(AiPlayer->Force[f].Size() == 
					AiPlayer->Force[force].Size());
			AiPlayer->Force[force].Reset();
		}
		force = f;
	}

	AiCleanForce(force);

	AiPlayer->Force[force].Attacking = false;
	if (AiPlayer->Force[force].Units.size() > 0) {
		AiForce *ForcePtr = &AiPlayer->Force[force];
		ForcePtr->Attacking = true;

		if(goalX == -1 || goalY == -1) {
			/* Search in entire map */
			const CUnit *enemy = AiForceEnemyFinder<false>(force).enemy;			
			if (enemy) {
				goalX = enemy->X;
				goalY = enemy->Y;
			}
		}
		if(goalX == -1 || goalY == -1) {
			DebugPrint("%d: Need to plan an attack with transporter\n" _C_ AiPlayer->Player->Index);
			if (!ForcePtr->State &&	!AiPlanAttack(ForcePtr)) {
				DebugPrint("%d: Can't transport\n" _C_ AiPlayer->Player->Index);
				ForcePtr->Attacking = false;
			}
			return;
		}

		ForcePtr->GoalX = goalX;
		ForcePtr->GoalY = goalY;

		//
		//  Send all units in the force to enemy.
		//
		AiForceAttackSender(force, goalX, goalY);
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
	if (!(force < AI_MAX_FORCES)) {
		DebugPrint("Force out of range: %d" _C_ force);
		return ;
	}
	
	if (!Map.Info.IsPointOnMap(x,y)) {
		DebugPrint("(%d, %d) not in the map(%d, %d)" _C_ x _C_ y
			_C_ Map.Info.MapWidth _C_ Map.Info.MapHeight);
		return ;
	}
	AiAttack(force, x, y);
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
	AiAttack(force, -1, -1);
}

//FIXME: rb - I don't know if AI can use transport now
#if 0
/**
**  Load all unit before attack.
**
**  @param aiForce force to group.
*/
static void AiGroupAttackerForTransport(AiForce &aiForce)
{
	Assert(aiForce.State == AI_FORCE_STATE_BOARDING);

	unsigned int nbToTransport = 0;
	unsigned int transporterIndex = 0;
	bool goNext = true;

	for (; transporterIndex < aiForce.Units.size(); ++transporterIndex) {
		const CUnit &unit = *aiForce.Units[transporterIndex];

		if (unit.Type->CanTransport && unit.Type->MaxOnBoard - unit.BoardCount > 0) {
			nbToTransport = unit.Type->MaxOnBoard - unit.BoardCount;
			break;
		}
	}
	if (transporterIndex == aiForce.Units.size()) {
		aiForce.State++;
		return ;
	}
	for (unsigned int i = 0; i < aiForce.Units.size(); ++i) {
		const CUnit &unit = *aiForce.Units[i];
		const CUnit &transporter = *aiForce.Units[transporterIndex];

		if (CanTransport(&transporter, &unit) && unit.Container == NULL) {
			goNext = false;
		}
	}
	if (goNext == true) {
		aiForce.State++;
		return ;
	}
	for (unsigned int i = 0; i < aiForce.Units.size(); ++i) {
		CUnit &unit = *aiForce.Units[i];
		CUnit &transporter = *aiForce.Units[transporterIndex];

		if (transporter.IsIdle() && unit.Orders[0]->Goal == &transporter) {
			CommandFollow(&transporter, &unit, 0);
		}
		if (CanTransport(&transporter, &unit) && unit.IsIdle() && unit.Container == NULL) {
			CommandBoard(&unit, &transporter, FlushCommands);
			CommandFollow(&transporter, &unit, 0);
			if (--nbToTransport == 0) { // full : nxt transporter.
				for (++transporterIndex; transporterIndex < aiForce.Units.size(); ++transporterIndex) {
					const CUnit &transporter = *aiForce.Units[transporterIndex];

					if (transporter.Type->CanTransport) {
						nbToTransport = transporter.Type->MaxOnBoard - transporter.BoardCount;
						break ;
					}
				}
				if (transporterIndex == aiForce.Units.size()) { // No more transporter.
					break ;
				}
			}
		}
	}
}
#endif

/**
** Force on attack ride. We attack until there is no unit or enemy left.
**
** @param force Force pointer.
*/
static void AiForceAttacks(AiForce *force)
{
	CUnit *aiunit;
	int x;
	int y;
	const CUnit *unit = NULL;
	unsigned int i;

	if (force->Size() == 0) {
		force->Attacking = false;
		if (!force->Defending && force->State) {
			DebugPrint("%d: Attack force #%lu was destroyed, giving up\n"
				_C_ AiPlayer->Player->Index _C_ (force  - &(AiPlayer->Force[0])));		
			force->Reset(true);
		}	
		return;
	}
#if 0
	force->Attacking = false;
	for (i = 0; i < force->Units.size(); ++i) {
		aiunit = force->Units[i];
		if (aiunit->Type->CanAttack) {
			force->Attacking = true;
			break;
		}
	}
	if (force->Attacking == false) {
		return ;
	}

	if (force->State == AI_FORCE_STATE_WAITING) {
		if (!AiPlanAttack(force)) {
			DebugPrint("Can't transport, look for walls\n");
			if (!AiFindWall(force)) {
				force->Attacking = false;
				return ;
			}
		}
		force->State = AI_FORCE_STATE_BOARDING;
	}
	if (force->State == AI_FORCE_STATE_BOARDING) {
		AiGroupAttackerForTransport(*force);
		return ;
	}
#endif

	// Find a unit that isn't idle
	unit = NoUnitP;
	if (force->State == AI_FORCE_STATE_ATTACKING) {
		for (i = 0; i < force->Size(); ++i) {
			aiunit = force->Units[i];
			if (!aiunit->IsIdle()) {
				// Found an idle unit, use it if we find nothing better
				if (unit == NoUnitP) {
					unit = aiunit;
				}
				// If the unit has a goal use it
				if (aiunit->CurrentOrder()->HasGoal()) {
					unit = aiunit;
					break;
				}
			}
		}
	}
	if (unit != NoUnitP) {
		// Give idle units a new goal
		// FIXME: may not be a good goal
		COrderPtr order = unit->CurrentOrder();
		if (order->HasGoal()) {
			x = order->GetGoal()->X;
			y = order->GetGoal()->Y;
		} else if (order->X != -1 && order->Y != -1) {
			x = order->X;
			y = order->Y;
		} else {
			x = force->GoalX;
			y = force->GoalY;
		}
		for (i = 0; i < force->Size(); ++i) {
			aiunit = force->Units[i];
			if (!aiunit->IsIdle()) {
				continue;
			}
			if (aiunit->Type->CanAttack) {
				CommandAttack(aiunit, x, y, NULL, FlushCommands);
			} else if (aiunit->Type->CanTransport()) {
				// FIXME : Retrieve unit blocked (transport previously full)
				CommandMove(aiunit, aiunit->Player->StartX,
					 aiunit->Player->StartY, FlushCommands);
			} else {
				CommandMove(aiunit, x, y, FlushCommands);
			}
		}
	} else { // Everyone is idle, find a new target
//FIXME: rb - I don't know if AI can use transport now
#if 0
		if (force->State == AI_FORCE_STATE_ATTACKING) {
			unit = NULL;

			for (i = 0; i < force->Units.size(); ++i) {
				aiunit = force->Units[i];
				if (aiunit->Type->CanAttack) {
					unit = AttackUnitsInDistance(aiunit, MaxMapWidth);
					break;
				}
			}
			if (!unit) {
				// No enemy found, give up
				// FIXME: should the force go home or keep trying to attack?
				DebugPrint("Attack force can't find a target, giving up\n");
				force->Attacking = false;
				return;
			}
			x = unit->X;
			y = unit->Y;
		} else {
			x = force->GoalX;
			y = force->GoalY;
		}
		for (i = 0; i < force->Units.size(); ++i) {
			aiunit = force->Units[i];

			if (aiunit->Type->CanTransport && aiunit->BoardCount > 0) {
				CommandUnload(aiunit, x, y, NULL, FlushCommands);
			} else if (aiunit->Type->CanAttack) {
				CommandAttack(aiunit, x, y, NULL, FlushCommands);
			} else if (force->State == 2) {
				CommandMove(aiunit, x, y, FlushCommands);
			}
		}
		force->State = 3;
#else
		if (force->State == AI_FORCE_STATE_ATTACKING) {

			unit = AiForceEnemyFinder<false>(force).enemy;

			if (!unit) {
				// No enemy found, give up
				// FIXME: should the force go home or keep trying to attack?
				DebugPrint("%d: Attack force #%lu can't find a target, giving up\n"
					_C_ AiPlayer->Player->Index _C_ (force  - &(AiPlayer->Force[0])));
				force->Attacking = false;
				return;
			}
			x = unit->X;
			y = unit->Y;
		} else {
			x = force->GoalX;
			y = force->GoalY;
		}
		AiForceAttackSender(force, x, y);
#endif
	}
}

/**
**  Entry point of force manager, perodic called.
**
** @todo FIXME: is this really needed anymore
*/
void AiForceManager(void)
{
	//
	//  Look if our defenders still have enemies in range.
	//
	for (int force = 0; force < AI_MAX_ATTACKING_FORCES; ++force) {
		if (AiPlayer->Force[force].Defending) {
			AiCleanForce(force);
			//
			//  Look if still enemies in attack range.
			//
			if(!AiForceEnemyFinder<true>(force).found()) {
				DebugPrint("%d:FIXME: not written, should send force #%d home\n" _C_ AiPlayer->Player->Index _C_ force);
				AiPlayer->Force[force].Defending = false;
				AiPlayer->Force[force].Attacking = false;
			}
		}
		if (AiPlayer->Force[force].Attacking) {
			AiCleanForce(force);
			AiForceAttacks(&AiPlayer->Force[force]);
		}
	}
	AiAssignFreeUnitsToForce();
}

//@}
