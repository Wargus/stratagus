//     ____                _       __               
//    / __ )____  _____   | |     / /___ ___________
//   / __  / __ \/ ___/   | | /| / / __ `/ ___/ ___/
//  / /_/ / /_/ (__  )    | |/ |/ / /_/ / /  (__  ) 
// /_____/\____/____/     |__/|__/\__,_/_/  /____/  
//                                              
//       A futuristic real-time strategy game.
//          This file is part of Bos Wars.
//
/**@name ai_force.cpp - AI force functions. */
//
//      (c) Copyright 2001-2009 by Lutz Sammer and Jimmy Salmon
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
#include <string.h>

#include "stratagus.h"
#include "unittype.h"
#include "unit.h"
#include "ai_local.h"
#include "actions.h"
#include "map.h"
#include "pathfinder.h"
#include "player.h"

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
	for (int i = 0; i <= UnitTypeMax; ++i)
	{
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
	if (find < replace)
	{
		i = find;
		find = replace;
		replace = i;
	}

	// Then just find & replace in UnitTypeEquivs...
	for (i = 0; i <= UnitTypeMax; ++i)
	{
		if (UnitTypeEquivs[i] == find)
		{
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

	for (i = 0; i <= UnitTypeMax; ++i)
	{
		if (UnitTypeEquivs[i] == search)
		{
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
	int bestlevel;
	int curlevel;

	// 1 - Find equivalents
	usableTypesCount = AiFindUnitTypeEquiv(unittype,  usableTypes);

	// 2 - Sort by level
	// We won't have usableTypesCount>4, so simple sort should do it
	for (int i = 0; i < usableTypesCount - 1; ++i)
	{
		bestlevel = UnitTypes[usableTypes[i]]->Priority;
		for (int j = i + 1; j < usableTypesCount; ++j)
		{
			curlevel = UnitTypes[usableTypes[j]]->Priority;

			if (curlevel > bestlevel)
			{
				// Swap
				int tmp = usableTypes[j];
				usableTypes[j] = usableTypes[i];
				usableTypes[i] = tmp;

				bestlevel = curlevel;
			}
		}
	}

	return usableTypesCount;
}

/**
**  Clean units in a force.
**
**  @param force  Force number.
*/
static void AiCleanForce(int force)
{
	CUnit *aiunit;
	const AiUnitType *aitype;
	int counter[UnitTypeMax + 1];
	size_t i;

	//
	// Release all killed units.
	//
	i = 0;
	while (i != AiPlayer->Force[force].Units.size())
	{
		aiunit = AiPlayer->Force[force].Units[i];
		if (aiunit->Destroyed ||
			aiunit->Orders[0]->Action == UnitActionDie)
		{
			aiunit->RefsDecrease();
			AiPlayer->Force[force].Units.erase(
				AiPlayer->Force[force].Units.begin() + i);
			continue;
		}
		++i;
	}

	//
	// Count units in force.
	//
	memset(counter, 0, sizeof(counter));
	for (i = 0; i < AiPlayer->Force[force].Units.size(); ++i)
	{
		aiunit = AiPlayer->Force[force].Units[i];
		counter[UnitTypeEquivs[aiunit->Type->Slot]]++;
	}

	//
	// Look if the force is complete.
	//
	AiPlayer->Force[force].Completed = true;
	for (i = 0; i < AiPlayer->Force[force].UnitTypes.size(); ++i)
	{
		aitype = &AiPlayer->Force[force].UnitTypes[i];
		if (aitype->Want > counter[aitype->Type->Slot])
		{
			AiPlayer->Force[force].Completed = false;
		}
		counter[aitype->Type->Slot] -= aitype->Want;
	}

	//
	// Release units too much in force.
	//
	if (!AiPlayer->Force[force].Attacking)
	{
		i = 0;
		while (i != AiPlayer->Force[force].Units.size())
		{
			aiunit = AiPlayer->Force[force].Units[i];
			if (counter[aiunit->Type->Slot] > 0)
			{
				counter[UnitTypeEquivs[aiunit->Type->Slot]]--;
				aiunit->RefsDecrease();
				AiPlayer->Force[force].Units.erase(
					AiPlayer->Force[force].Units.begin() + i);
				continue;
			}
			++i;
		}
	}
}

/**
**  Cleanup units in forces.
*/
void AiCleanForces()
{
	// Release all killed units.
	for (int force = 0; force < AI_MAX_ATTACKING_FORCES; ++force)
	{
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
static int AiCheckBelongsToForce(int force, const CUnitType *type)
{
	CUnit *aiunit;
	AiUnitType *aitype;
	int counter[UnitTypeMax + 1];
	int flag;
	int i;

	memset(counter, 0, sizeof(counter));
	//
	// Count units in force.
	//
	for (i = 0; i < (int)AiPlayer->Force[force].Units.size(); ++i)
	{
		aiunit = AiPlayer->Force[force].Units[i];
		counter[UnitTypeEquivs[aiunit->Type->Slot]]++;
	}

	//
	// Look what should be in the force.
	//
	flag = 0;
	AiPlayer->Force[force].Completed = true;
	for (i = 0; i < (int)AiPlayer->Force[force].UnitTypes.size(); ++i)
	{
		aitype = &AiPlayer->Force[force].UnitTypes[i];
		if (aitype->Want > counter[aitype->Type->Slot])
		{
			if (UnitTypeEquivs[type->Slot] == aitype->Type->Slot)
			{
				if (aitype->Want - 1 > counter[aitype->Type->Slot])
				{
					AiPlayer->Force[force].Completed = false;
				}
				flag = 1;
			}
			else
			{
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
void AiAssignToForce(CUnit *unit)
{
	//
	// Check to which force it belongs
	//
	for (int force = 0; force < AI_MAX_FORCES; ++force)
	{
		// No troops for attacking force
		if (!AiPlayer->Force[force].Defending &&
			AiPlayer->Force[force].Attacking)
		{
			continue;
		}

		if (AiCheckBelongsToForce(force, unit->Type))
		{
			AiPlayer->Force[force].Units.insert(
				AiPlayer->Force[force].Units.begin(), unit);
			unit->RefsIncrease();
			break;
		}
	}
}

/**
**  Assign free units to force.
*/
void AiAssignFreeUnitsToForce()
{
	const CUnit *aiunit;
	CUnit *table[UnitMax];
	int n;
	int f;
	int i;

	AiCleanForces();

	n = AiPlayer->Player->TotalNumUnits;
	memcpy(table, AiPlayer->Player->Units, sizeof(*AiPlayer->Player->Units) * n);

	//
	// Remove all units already in forces.
	//
	for (f = 0; f < AI_MAX_ATTACKING_FORCES; ++f)
	{
		for (int j = 0; j < (int)AiPlayer->Force[f].Units.size(); ++j)
		{
			aiunit = AiPlayer->Force[f].Units[j];
			for (i = 0; i < n; ++i)
			{
				if (table[i] == aiunit)
				{
					table[i] = table[--n];
				}
			}
		}
	}

	//
	// Try to assign the remaining units.
	//
	for (i = 0; i < n; ++i)
	{
		AiAssignToForce(table[i]);
	}
}

/**
**  Attack at position with force.
**
**  @param force  Force number to attack with.
**  @param x      X tile map position to be attacked.
**  @param y      Y tile map position to be attacked.
*/
void AiAttackWithForceAt(int force, int x, int y)
{
	AiCleanForce(force);

	if (!AiPlayer->Force[force].Units.empty())
	{
		AiPlayer->Force[force].Attacking = true;

		//
		// Send all units in the force to enemy.
		//
		for (int i = 0; i < (int)AiPlayer->Force[force].Units.size(); ++i)
		{
			CUnit *aiunit = AiPlayer->Force[force].Units[i];
			aiunit->Wait = i;
			if (aiunit->Type->CanAttack)
			{
				CommandAttack(aiunit, x, y, NULL, FlushCommands);
			}
			else
			{
				CommandMove(aiunit, x, y, FlushCommands);
			}
		}
	}
}

/**
**  Attack opponent with force.
**
**  @param force  Force number to attack with.
*/
void AiAttackWithForce(int force)
{
	CUnit *aiunit;
	const CUnit *enemy;
	int x;
	int y;
	int f;
	int i;

	// Move the force to a free position so it can be used for a new
	// attacking party
	if (force < AI_MAX_FORCES)
	{
		f = AI_MAX_FORCES;
		while (AiPlayer->Force[f].Attacking)
		{
			++f;
			if (f == AI_MAX_ATTACKING_FORCES)
			{
				DebugPrint("No free attacking forces\n");
				f = force;
				break;
			}
		}
		if (f != AI_MAX_ATTACKING_FORCES)
		{
			AiPlayer->Force[f] = AiPlayer->Force[force];
			AiPlayer->Force[force].Reset();
		}

		force = f;
	}

	AiCleanForce(force);

	AiPlayer->Force[force].Attacking = false;
	if (!AiPlayer->Force[force].Units.empty())
	{
		AiPlayer->Force[force].Attacking = true;

		enemy = NULL;
		for (i = 0; !enemy && i < (int)AiPlayer->Force[force].Units.size(); ++i)
		{
			aiunit = AiPlayer->Force[force].Units[i];
			if (aiunit->Type->CanAttack)
			{
				enemy = AttackUnitsInDistance(aiunit, MaxMapWidth);
			}
		}

		if (!enemy)
		{
			DebugPrint("No enemy found\n");
			return;
		}
		AiPlayer->Force[force].State = 0;
		x = enemy->X;
		y = enemy->Y;

		//
		//  Send all units in the force to enemy.
		//
		for (i = 0; i < (int)AiPlayer->Force[force].Units.size(); ++i)
		{
			aiunit = AiPlayer->Force[force].Units[i];
			aiunit->Wait = i;
			if (aiunit->Type->CanAttack)
			{
				CommandAttack(aiunit, x, y, NULL, FlushCommands);
			}
			else
			{
				CommandMove(aiunit, x, y, FlushCommands);
			}
		}
	}
}

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
	const CUnit *unit;
	int i;

	if (force->Units.empty())
	{
		force->Attacking = false;
		return;
	}

	// Find a unit that isn't idle
	unit = NoUnitP;
	for (i = 0; i < (int)force->Units.size(); ++i)
	{
		aiunit = force->Units[i];
		if (!aiunit->IsIdle())
		{
			// Found an idle unit, use it if we find nothing better
			if (unit == NoUnitP)
			{
				unit = aiunit;
			}
			// If the unit has a goal use it
			if (aiunit->Orders[0]->Goal != NoUnitP)
			{
				unit = aiunit;
				break;
			}
		}
	}

	if (unit != NoUnitP)
	{
		// Give idle units a new goal
		// FIXME: may not be a good goal
		for (i = 0; i < (int)force->Units.size(); ++i)
		{
			aiunit = force->Units[i];
			if (aiunit->IsIdle())
			{
				if (unit->Orders[0]->Goal)
				{
					x = unit->Orders[0]->Goal->X;
					y = unit->Orders[0]->Goal->Y;
				}
				else if (unit->Orders[0]->X != -1 && unit->Orders[0]->Y != -1)
				{
					x = unit->Orders[0]->X;
					y = unit->Orders[0]->Y;
				}
				else
				{
					x = unit->X;
					y = unit->Y;
				}
				if (aiunit->Type->CanAttack)
				{
					CommandAttack(aiunit, x, y, NULL, FlushCommands);
				}
				else
				{
					CommandMove(aiunit, x, y, FlushCommands);
				}
			}
		}
	}
	else
	{
		// Everyone is idle, find a new target
		unit = NULL;
		for (i = 0; i < (int)force->Units.size(); ++i)
		{
			aiunit = force->Units[i];
			if (aiunit->Type->CanAttack)
			{
				unit = AttackUnitsInDistance(aiunit, MaxMapWidth);
				break;
			}
		}
		if (!unit)
		{
			// No enemy found, give up
			// FIXME: should the force go home or keep trying to attack?
			DebugPrint("Attack force can't find a target, giving up\n");
			force->Attacking = false;
			return;
		}
		for (i = 0; i < (int)force->Units.size(); ++i)
		{
			aiunit = force->Units[i];
			if (aiunit->Type->CanAttack)
			{
				CommandAttack(aiunit, unit->X, unit->Y, NULL, FlushCommands);
			}
			else
			{
				CommandMove(aiunit, unit->X, unit->Y, FlushCommands);
			}
		}
	}
}

/**
**  Entry point of force manager, called each second.
*/
void AiForceManager()
{
	for (int force = 0; force < AI_MAX_ATTACKING_FORCES; ++force)
	{
		// Defending
		if (AiPlayer->Force[force].Defending)
		{
			const CUnit *aiunit = NULL;

			AiCleanForce(force);

			// Check if there are still enemies in attack range.
			for (size_t i = 0; i < AiPlayer->Force[force].Units.size(); ++i)
			{
				if (AiPlayer->Force[force].Units[i]->Type->CanAttack &&
					AttackUnitsInReactRange(AiPlayer->Force[force].Units[i]))
				{
					aiunit = AiPlayer->Force[force].Units[i];
					break;
				}
			}
			if (aiunit == NULL)
			{
				// No enemies go home.
				DebugPrint("FIXME: not written, should send force home\n");
				AiPlayer->Force[force].Defending = false;
				AiPlayer->Force[force].Attacking = false;
			}
		}

		// Attacking
		if (AiPlayer->Force[force].Attacking)
		{
			AiCleanForce(force);
			AiForceAttacks(&AiPlayer->Force[force]);
		}
	}

	AiAssignFreeUnitsToForce();
}

//@}
