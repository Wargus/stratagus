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

#include "stratagus.h"

#include "unit.h"

#include "unit_manager.h"
#include "unittype.h"
#include "script.h"
#include "iolib.h"

/*----------------------------------------------------------------------------
--  Variables
----------------------------------------------------------------------------*/

/**
**  How many groups supported
*/
#define NUM_GROUPS 10

/**
**  Defines a group of units.
*/
struct CUnitGroup {
	CUnit **Units;  /// Units in the group
	int NumUnits;   /// How many units in the group
	bool tainted;    /// Group hold unit which can't be SelectableByRectangle
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

bool IsGroupTainted(int num)
{
	Assert(num < NUM_GROUPS);
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
	Assert(num < NUM_GROUPS);
	if (mode != SELECT_ALL && Groups[num].tainted && Groups[num].NumUnits) {
		int count = 0;
		for (int i = 0; i < Groups[num].NumUnits; ++i) {
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
	Assert(num < NUM_GROUPS);
	return Groups[num].Units;
}

/**
**  Clear contents of group num
**
**  @param num  Group number.
*/
void ClearGroup(int num)
{
	Assert(num < NUM_GROUPS);
	CUnitGroup &group = Groups[num];

	for (int i = 0; i < group.NumUnits; ++i) {
		group.Units[i]->GroupId &= ~(1 << num);
		Assert(!group.Units[i]->Destroyed);
	}
	group.NumUnits = 0;
	group.tainted = 0;
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

	CUnitGroup &group = Groups[num];
	for (int i = 0; group.NumUnits < MaxSelectable && i < nunits; ++i) {
		// Add to group only if they are on our team
		// Buildings can be in group but it "taint" the group.
		// Taited groups normaly select only SelectableByRectangle units but
		// you can force selection to show hiden members (with buildings) by
		// selecting ALT-(SHIFT)-#
		if (ThisPlayer->IsTeamed(*units[i])) {
			if (!group.tainted) {
				group.tainted = units[i]->Type->SelectableByRectangle != true;
			}
			group.Units[group.NumUnits++] = units[i];
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
	Assert(unit.GroupId != 0);  // unit doesn't belong to a group

	for (int num = 0; unit.GroupId; ++num, unit.GroupId >>= 1) {
		if ((unit.GroupId & 1) != 1) {
			continue;
		}
		CUnitGroup &group = Groups[num];

		int ind;
		for (ind = 0; group.Units[ind] != &unit; ++ind) {
			Assert(ind < group.NumUnits); // oops not found
		}
		if (ind < --group.NumUnits) {
			group.Units[ind] = group.Units[group.NumUnits];
		}

		// Update tainted flag.
		if (group.tainted && !unit.Type->SelectableByRectangle) {
			group.tainted = false;
			for (int i = 0; i < group.NumUnits; ++i) {
				if (group.Units[i]->Type && !group.Units[i]->Type->SelectableByRectangle) {
					group.tainted = true;
				}
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
	LuaCheckArgs(l, 3);

	InitGroups();
	const int grpNum = LuaToNumber(l, 1);
	if (NUM_GROUPS <= grpNum) {
		LuaError(l, "grpIndex out of bound");
	}
	CUnitGroup &grp = Groups[grpNum];
	grp.NumUnits = LuaToNumber(l, 2);
	int i = 0;
	const int args = lua_rawlen(l, 3);
	for (int j = 0; j < args; ++j) {
		lua_rawgeti(l, 3, j + 1);
		const char *str = LuaToString(l, -1);
		lua_pop(l, 1);
		grp.Units[i++] = UnitSlots[strtol(str + 1, NULL, 16)];
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
