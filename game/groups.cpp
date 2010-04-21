//     ____                _       __               
//    / __ )____  _____   | |     / /___ ___________
//   / __  / __ \/ ___/   | | /| / / __ `/ ___/ ___/
//  / /_/ / /_/ (__  )    | |/ |/ / /_/ / /  (__  ) 
// /_____/\____/____/     |__/|__/\__,_/_/  /____/  
//                                              
//       A futuristic real-time strategy game.
//          This file is part of Bos Wars.
//
/**@name groups.cpp - The units' groups handling. */
//
//      (c) Copyright 1999-2007 by Patrice Fortier, Lutz Sammer,
//                                 and Jimmy Salmon
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
#include "unit.h"
#include "unit_manager.h"
#include "unittype.h"
#include "script.h"
#include "player.h"
#include "iolib.h"

/*----------------------------------------------------------------------------
--  Variables
----------------------------------------------------------------------------*/

/**
**  Defines a group of units.
*/
struct CUnitGroup {
	CUnit **Units;                       /// Units in the group
	int NumUnits;                        /// How many units in the group
};                                       /// group of units

static CUnitGroup Groups[NUM_GROUPS];    /// Number of groups predefined

/*----------------------------------------------------------------------------
--  Functions
----------------------------------------------------------------------------*/

/**
**  Initialize group part.
**
**  @todo Not needed with the new unit code!
*/
void InitGroups(void)
{
	for (int i = 0; i < NUM_GROUPS; ++i) {
		if (!Groups[i].Units) {
			Groups[i].Units = new CUnit *[MaxSelectable];
		}
	}
}

/**
**  Save groups.
**
**  @param file  Output file.
*/
void SaveGroups(CFile *file)
{
	file->printf("\n--- -----------------------------------------\n");
	file->printf("--- MODULE: groups\n\n");

	for (int g = 0; g < NUM_GROUPS; ++g) {
		file->printf("Group(%d, %d, {", g, Groups[g].NumUnits);
		for (int i = 0; i < Groups[g].NumUnits; ++i) {
			file->printf("\"%s\", ", UnitReference(Groups[g].Units[i]).c_str());
		}
		file->printf("})\n");
	}
}

/**
**  Clean up group part.
*/
void CleanGroups(void)
{
	for (int i = 0; i < NUM_GROUPS; ++i) {
		delete[] Groups[i].Units;
		Groups[i].Units = NULL;
		Groups[i].NumUnits = 0;
	}
}

/**
**  Return the number of units of group num
**
**  @param num  Group number.
**
**  @return     Returns the number of units in the group.
*/
int GetNumberUnitsOfGroup(int num)
{
	return Groups[num].NumUnits;
}

/**
**  Return the units of group num
**
**  @param num  Group number.
**
**  @return     Returns an array of all units in the group.
*/
CUnit **GetUnitsOfGroup(int num)
{
	return Groups[num].Units;
}

/**
**  Clear contents of group num
**
**  @param num  Group number.
*/
void ClearGroup(int num)
{
	CUnitGroup *group;
	int i;

	group = &Groups[num];
	for (i = 0; i < group->NumUnits; ++i) {
		group->Units[i]->GroupId &= ~(1 << num);
		Assert(!group->Units[i]->Destroyed);
	}
	group->NumUnits = 0;
}

/**
**  Add units to group num contents from unit array "units"
**
**  @param units   Array of units to place into group.
**  @param nunits  Number of units in array.
**  @param num     Group number for storage.
*/
void AddToGroup(CUnit **units, int nunits, int num)
{
	CUnitGroup *group;
	int i;

	Assert(num <= NUM_GROUPS);

	group = &Groups[num];
	// Check to make sure we don't have a building as the only group member
	// If so, we are unable to add these units to the group.
	if (group->NumUnits == 1 && !group->Units[0]->Type->SelectableByRectangle) {
		return;
	}
	for (i = 0; group->NumUnits < MaxSelectable && i < nunits; ++i) {
		// Add to group only if they are on our team, and are rectangle
		// selectable.  Otherwise buildings and units can be in a group.
		// or enemy units and my units. Exceptions is when there is only
		// one unit in the group, then we can group a buildings.
		if (ThisPlayer->IsTeamed(units[i]) &&
				(units[i]->Type->SelectableByRectangle ||
					(nunits == 1 && group->NumUnits == 0))) {
			group->Units[group->NumUnits++] = units[i];
			units[i]->GroupId |= (1 << num);
		}
	}
}

/**
**  Set group num contents to unit array "units"
**
**  @param units   Array of units to place into group.
**  @param nunits  Number of units in array.
**  @param num     Group number for storage.
*/
void SetGroup(CUnit **units, int nunits, int num)
{
	Assert(num <= NUM_GROUPS && nunits <= MaxSelectable);

	ClearGroup(num);
	AddToGroup(units, nunits, num);
}

/**
**  Remove unit from its groups
**
**  @param unit  Unit to remove from group.
*/
void RemoveUnitFromGroups(CUnit *unit)
{
	CUnitGroup *group;
	int num;
	int i;

	Assert(unit->GroupId != 0);  // unit doesn't belong to a group

	for (num = 0; unit->GroupId; ++num, unit->GroupId >>= 1) {
		if ((unit->GroupId & 1) != 1) {
			continue;
		}

		group = &Groups[num];
		for (i = 0; group->Units[i] != unit; ++i) {
			;
		}

		Assert(i < group->NumUnits);  // oops not found

		// This is a clean way that will allow us to add a unit
		// to a group easily, or make an easy array walk...
		if (i < --group->NumUnits) {
			group->Units[i] = group->Units[group->NumUnits];
		}
	}
}

// ----------------------------------------------------------------------------

/**
**  Define the group.
**
**  @param l  Lua state.
*/
static int CclGroup(lua_State *l)
{
	int i;
	CUnitGroup *grp;
	int args;
	int j;

	LuaCheckArgs(l, 3);

	InitGroups();
	grp = &Groups[(int)LuaToNumber(l, 1)];
	grp->NumUnits = LuaToNumber(l, 2);
	i = 0;
	args = lua_objlen(l, 3);
	for (j = 0; j < args; ++j) {
		const char *str;

		str = LuaToString(l, 3, j + 1);
		grp->Units[i++] = UnitSlots[strtol(str + 1, NULL, 16)];
	}

	return 0;
}

/**
**  Register CCL features for groups.
*/
void GroupCclRegister(void)
{
	lua_register(Lua, "Group", CclGroup);
}

//@}
