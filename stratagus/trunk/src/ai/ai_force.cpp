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
/**@name ai_force.c - AI force functions. */
//
//      (c) Copyright 2001-2004 by Lutz Sammer
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

/*----------------------------------------------------------------------------
--  Variables
----------------------------------------------------------------------------*/

global int UnitTypeEquivs[UnitTypeMax + 1]; /// equivalence between unittypes

/*----------------------------------------------------------------------------
--  Functions
----------------------------------------------------------------------------*/


/**
**  Remove any equivalence between unittypes
*/
global void AiResetUnitTypeEquiv(void)
{
	int i;

	for (i = 0; i <= UnitTypeMax; ++i) {
		UnitTypeEquivs[i] = i;
	}
}

/**
**  Make two unittypes equivalents from the AI's point of vue
**
**  @param a  the first unittype
**  @param b  the second unittype
*/
global void AiNewUnitTypeEquiv(UnitType* a, UnitType* b)
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
global int AiFindUnitTypeEquiv(const UnitType* unittype, int* result)
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
global int AiFindAvailableUnitTypeEquiv(const UnitType* unittype, int* usableTypes)
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
	playerid = AiPlayer->Player->Player;

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
local void AiCleanForce(int force)
{
	AiUnit** prev;
	AiUnit* aiunit;
	const AiUnitType* aitype;
	int counter[UnitTypeMax + 1];

	//
	// Release all killed units.
	//
	prev = &AiPlayer->Force[force].Units;
	while ((aiunit = *prev)) {
		if (aiunit->Unit->Destroyed) {
			RefsDecrease(aiunit->Unit);
			*prev = aiunit->Next;
			free(aiunit);
			continue;
		} else if (!aiunit->Unit->HP ||
				aiunit->Unit->Orders[0].Action == UnitActionDie) {
			RefsDecrease(aiunit->Unit);
			*prev = aiunit->Next;
			free(aiunit);
			continue;
		}
		prev = &aiunit->Next;
	}

	//
	// Count units in force.
	//
	memset(counter, 0, sizeof(counter));
	aiunit = AiPlayer->Force[force].Units;
	while (aiunit) {
		// FIXME: Should I use equivalent unit types?
		counter[UnitTypeEquivs[aiunit->Unit->Type->Slot]]++;
		aiunit = aiunit->Next;
	}

	//
	// Look if the force is complete.
	//
	AiPlayer->Force[force].Completed = 1;
	aitype = AiPlayer->Force[force].UnitTypes;
	while (aitype) {
		if (aitype->Want > counter[aitype->Type->Slot]) {
			AiPlayer->Force[force].Completed = 0;
		}
		counter[aitype->Type->Slot] -= aitype->Want;
		aitype = aitype->Next;
	}

	//
	// Release units too much in force.
	//
	if (!AiPlayer->Force[force].Attacking) {
		prev = &AiPlayer->Force[force].Units;
		while ((aiunit = *prev)) {
			if (counter[aiunit->Unit->Type->Slot] > 0) {
				counter[UnitTypeEquivs[aiunit->Unit->Type->Slot]]--;
				RefsDecrease(aiunit->Unit);
				*prev = aiunit->Next;
				free(aiunit);
				continue;
			}
			prev = &aiunit->Next;
		}
	}
}

/**
**  Cleanup units in forces.
*/
global void AiCleanForces(void)
{
	int force;

	//
	// Release all killed units.
	//
	for (force = 0; force < AI_MAX_ATTACKING_FORCES; ++force) {
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
local int AiCheckBelongsToForce(int force, const UnitType* type)
{
	AiUnit* aiunit;
	AiUnitType* aitype;
	int counter[UnitTypeMax + 1];
	int flag;

	memset(counter, 0, sizeof(counter));
	//
	// Count units in force.
	//
	aiunit = AiPlayer->Force[force].Units;
	while (aiunit) {
		counter[UnitTypeEquivs[aiunit->Unit->Type->Slot]]++;
		aiunit = aiunit->Next;
	}

	//
	// Look what should be in the force.
	//
	flag = 0;
	AiPlayer->Force[force].Completed = 1;
	aitype = AiPlayer->Force[force].UnitTypes;
	while (aitype) {
		if (aitype->Want > counter[aitype->Type->Slot]) {
			if (UnitTypeEquivs[type->Slot] == aitype->Type->Slot) {
				if (aitype->Want - 1 > counter[aitype->Type->Slot]) {
					AiPlayer->Force[force].Completed = 0;
				}
				flag = 1;
			} else {
				AiPlayer->Force[force].Completed = 0;
			}
		}
		aitype = aitype->Next;
	}
	return flag;
}

/**
**  Ai assign unit to force.
**
**  @param unit  Unit to assign to force.
*/
global void AiAssignToForce(Unit* unit)
{
	int force;

	//
	// Check to which force it belongs
	//
	for (force = 0; force < AI_MAX_FORCES; ++force) {
		// No troops for attacking force
		if (!AiPlayer->Force[force].Defending &&
				AiPlayer->Force[force].Attacking) {
			continue;
		}

		if (AiCheckBelongsToForce(force, unit->Type)) {
			AiUnit* aiunit;

			aiunit = malloc(sizeof (*aiunit));
			aiunit->Next = AiPlayer->Force[force].Units;
			AiPlayer->Force[force].Units = aiunit;
			aiunit->Unit = unit;
			RefsIncrease(unit);
			break;
		}
	}
}

/**
**  Assign free units to force.
*/
global void AiAssignFreeUnitsToForce(void)
{
	const AiUnit* aiunit;
	Unit* table[UnitMax];
	Unit* unit;
	int n;
	int f;
	int i;

	AiCleanForces();

	n = AiPlayer->Player->TotalNumUnits;
	memcpy(table, AiPlayer->Player->Units, sizeof(*AiPlayer->Player->Units) * n);

	//
	// Remove all units already in forces.
	//
	for (f = 0; f < AI_MAX_ATTACKING_FORCES; ++f) {
		aiunit = AiPlayer->Force[f].Units;
		while (aiunit) {
			unit = aiunit->Unit;
			for (i = 0; i < n; ++i) {
				if (table[i] == unit) {
					table[i] = table[--n];
				}
			}
			aiunit = aiunit->Next;
		}
	}

	//
	// Try to assign the remaining units.
	//
	for (i = 0; i < n; ++i) {
		if (table[i]->Active) {
			AiAssignToForce(table[i]);
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
global void AiAttackWithForceAt(int force, int x, int y)
{
	const AiUnit* aiunit;

	AiCleanForce(force);

	if ((aiunit = AiPlayer->Force[force].Units)) {
		AiPlayer->Force[force].Attacking = 1;

		//
		// Send all units in the force to enemy.
		//
		while (aiunit) {
			if (aiunit->Unit->Type->CanAttack) {
				CommandAttack(aiunit->Unit, x, y, NULL, FlushCommands);
			} else {
				CommandMove(aiunit->Unit, x, y, FlushCommands);
			}
			aiunit = aiunit->Next;
		}
	}
}

/**
**  Attack opponent with force.
**
**  @param force  Force number to attack with.
*/
global void AiAttackWithForce(int force)
{
	const AiUnit* aiunit;
	const Unit* enemy;
	int x;
	int y;
	int f;

	// Move the force to a free position so it can be used for a new
	// attacking party
	if (force < AI_MAX_FORCES) {
		AiUnitType* aiut;
		AiUnitType* temp;
		AiUnitType** aiut2;

		f = AI_MAX_FORCES;
		while (AiPlayer->Force[f].Attacking) {
			++f;
			if (f == AI_MAX_ATTACKING_FORCES) {
				DebugPrint("No free attacking forces\n");
				f = force;
				break;
			}
		}
		if (f != AI_MAX_ATTACKING_FORCES) {
			for (aiut = AiPlayer->Force[f].UnitTypes; aiut; aiut = temp) {
				temp = aiut->Next;
				free(aiut);
			}

			AiPlayer->Force[f] = AiPlayer->Force[force];
			memset(&AiPlayer->Force[force], 0, sizeof(AiForce));
			aiut = AiPlayer->Force[force].UnitTypes;
			aiut2 = &AiPlayer->Force[force].UnitTypes;
			while (aiut) {
				*aiut2 = malloc(sizeof(**aiut2));
				(*aiut2)->Next = NULL;
				(*aiut2)->Want = aiut->Want;
				(*aiut2)->Type = aiut->Type;
				aiut = aiut->Next;
				aiut2 = &(*aiut2)->Next;
			}
		}

		force = f;
	}

	AiCleanForce(force);

	AiPlayer->Force[force].Attacking = 0;
	if ((aiunit = AiPlayer->Force[force].Units)) {
		AiPlayer->Force[force].Attacking = 1;

		enemy = NoUnitP;
		while (aiunit && !enemy) {
			if (aiunit->Unit->Type->CanAttack) {
				enemy = AttackUnitsInDistance(aiunit->Unit, MaxMapWidth);
			}
			aiunit = aiunit->Next;
		}

		if (!enemy) {
			DebugPrint("Need to plan an attack with transporter\n");
			if (!AiPlayer->Force[force].State &&
					!AiPlanAttack(&AiPlayer->Force[force])) {
				DebugPrint("Can't transport, look for walls\n");
				if (!AiFindWall(&AiPlayer->Force[force])) {
					AiPlayer->Force[force].Attacking = 0;
				}
			}
			return;
		}
		AiPlayer->Force[force].State = 0;
		x = enemy->X;
		y = enemy->Y;

		//
		//  Send all units in the force to enemy.
		//
		aiunit = AiPlayer->Force[force].Units;
		while (aiunit) {
			if (aiunit->Unit->Type->CanAttack) {
				CommandAttack(aiunit->Unit, x, y, NULL, FlushCommands);
			} else {
				CommandMove(aiunit->Unit, x, y, FlushCommands);
			}
			aiunit = aiunit->Next;
		}
	}
}

/**
**	Force on attack ride. We attack until there is no unit or enemy left.
**
**	@param force	Force pointer.
*/
local void AiForceAttacks(AiForce* force)
{
	const AiUnit* aiunit;

	if ((aiunit = force->Units)) {
		while (aiunit) {
			// Still some action
			if (!UnitIdle(aiunit->Unit)) {
				break;
			}
			aiunit = aiunit->Next;
		}
		// Must mark the attack as terminated
		if (!aiunit) {
			force->Attacking = 0;
			// AiAttackWithForce(force-AiPlayer->Force);
		}
	} else {
		force->Attacking = 0;
	}
}

/**
**  Entry point of force manager, perodic called.
**
** @todo FIXME: is this really needed anymore
*/
global void AiForceManager(void)
{
	int force;

	//
	//  Look if our defenders still have enemies in range.
	//
	for (force = 0; force < AI_MAX_ATTACKING_FORCES; ++force) {
		if (AiPlayer->Force[force].Defending) {
			const AiUnit* aiunit;

			AiCleanForce(force);
			//
			//  Look if still enemies in attack range.
			//
			aiunit = AiPlayer->Force[force].Units;
			while (aiunit) {
				if (aiunit->Unit->Type->CanAttack &&
						AttackUnitsInReactRange(aiunit->Unit)) {
					break;
				}
				aiunit = aiunit->Next;
			}
			if (!aiunit) {		// No enemies go home.
				DebugPrint("FIXME: not written, should send force home\n");
				AiPlayer->Force[force].Defending = 0;
				AiPlayer->Force[force].Attacking = 0;
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
