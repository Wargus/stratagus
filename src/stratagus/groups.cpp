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
/**@name groups.cpp - The units' groups handling. */
//
//      (c) Copyright 1999-2005 by Patrice Fortier, Lutz Sammer,
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
#include "actions.h"

/*----------------------------------------------------------------------------
--  Variables
----------------------------------------------------------------------------*/

/**
**  Defines a group of units.
*/
struct CUnitGroup {
	CUnit **Units;                       /// Units in the group
	int NumUnits;                        /// How many units in the group
	int tainted;						/// Group hold unit which can't be SelectableByRectangle
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
void InitGroups()
{
	for (int i = 0; i < NUM_GROUPS; ++i) {
		if (!Groups[i].Units) {
			Groups[i].Units = new CUnit *[MaxSelectable];
		}
		Groups[i].tainted = 0;
	}
}

/**
**  Save groups.
**
**  @param file  Output file.
*/
void SaveGroups(CFile &file)
{
	file.printf("\n--- -----------------------------------------\n");
	file.printf("--- MODULE: groups\n\n");

	for (int g = 0; g < NUM_GROUPS; ++g) {
		file.printf("Group(%d, %d, {", g, Groups[g].NumUnits);
		for (int i = 0; i < Groups[g].NumUnits; ++i) {
			file.printf("\"%s\", ", UnitReference(*Groups[g].Units[i]).c_str());
		}
		file.printf("})\n");
	}
}

/**
**  Clean up group part.
*/
void CleanGroups()
{
	for (int i = 0; i < NUM_GROUPS; ++i) {
		delete[] Groups[i].Units;
		Groups[i].Units = NULL;
		Groups[i].NumUnits = 0;
		Groups[i].tainted = 0;
	}
}

int IsGroupTainted(int num) {
	return Groups[num].tainted;
}

/**
**  Return the number of units of group num
**
**  @param num  Group number.
**
**  @return     Returns the number of units in the group.
*/
int GetNumberUnitsOfGroup(int num, GroupSelectionMode mode)
{
	if (mode != SELECT_ALL && Groups[num].tainted && Groups[num].NumUnits) {
		int count = 0;
		for(int i = 0; i < Groups[num].NumUnits; ++i)
		{
			const CUnitType *type = Groups[num].Units[i]->Type;
			if (type && type->CanSelect(mode)) {
				count++;
			}
		}
		return count;
	}
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

	group = &Groups[num];
	for (int i = 0; i < group->NumUnits; ++i) {
		group->Units[i]->GroupId &= ~(1 << num);
		Assert(!group->Units[i]->Destroyed);
	}
	group->NumUnits = 0;
	group->tainted = 0;
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
	Assert(num <= NUM_GROUPS);

	CUnitGroup *group = &Groups[num];
	for (int i = 0; group->NumUnits < MaxSelectable && i < nunits; ++i) {
		// Add to group only if they are on our team
		// Buildings can be in group but it "taint" the group.
		// Taited groups normaly select only SelectableByRectangle units but
		// you can force selection to show hiden members (with buildings) by
		// seletcing ALT-(SHIFT)-#
		if (ThisPlayer->IsTeamed(*units[i])) {
			if (!group->tainted) {
				group->tainted = units[i]->Type->SelectableByRectangle != true;
			}
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
void RemoveUnitFromGroups(CUnit &unit)
{
	CUnitGroup *group;
	int num;
	int i;

	Assert(unit.GroupId != 0);  // unit doesn't belong to a group

	for (num = 0; unit.GroupId; ++num, unit.GroupId >>= 1) {
		if ((unit.GroupId & 1) != 1) {
			continue;
		}

		group = &Groups[num];
		for (i = 0; group->Units[i] != &unit; ++i) {
			;
		}

		Assert(i < group->NumUnits);  // oops not found

		// This is a clean way that will allow us to add a unit
		// to a group easily, or make an easy array walk...
		if (i < --group->NumUnits) {
			group->Units[i] = group->Units[group->NumUnits];
		}

		if (group->tainted && !unit.Type->SelectableByRectangle) {
			for (i = 0; i < group->NumUnits; ++i) {
				if (group->Units[i]->Type && !group->Units[i]->Type->SelectableByRectangle) {
					break;
				}
			}
			if (i == group->NumUnits) {
				group->tainted = 0;
			}
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

		lua_rawgeti(l, 3, j + 1);
		str = LuaToString(l, -1);
		lua_pop(l, 1);
		grp->Units[i++] = UnitSlots[strtol(str + 1, NULL, 16)];
	}

	return 0;
}

/**
**  Register CCL features for groups.
*/
void GroupCclRegister()
{
	lua_register(Lua, "Group", CclGroup);
}

//@}
