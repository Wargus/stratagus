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
//      (c) Copyright 1999-2015 by Patrice Fortier, Lutz Sammer,
//      and Jimmy Salmon
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

#include "iolib.h"
#include "script.h"
#include "unit_manager.h"
#include "unittype.h"

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
class CUnitGroup
{
public:
	CUnitGroup() : tainted(false) {}

	void init()
	{
		units.clear();
		tainted = false;
	}

	bool isTainted() const { return tainted; }
	const std::vector<CUnit *> &getUnits() const { return units; }

	void add(CUnit &unit, unsigned int num)
	{
		if (ThisPlayer->IsTeamed(unit)) {
			if (!tainted) {
				tainted = unit.Type->BoolFlag[SELECTABLEBYRECTANGLE_INDEX].value != true;
			}
			units.push_back(&unit);
			unit.GroupId |= (1 << num);
		}
	}

	void remove(CUnit &unit)
	{
		std::vector<CUnit *>::iterator it = find(units.begin(), units.end(), &unit);

		Assert(it != units.end());
		*it = units.back();
		units.pop_back();

		// Update tainted flag.
		if (tainted && !unit.Type->BoolFlag[SELECTABLEBYRECTANGLE_INDEX].value) {
			updateTainted();
		}
	}

private:
	void updateTainted()
	{
		tainted = false;
		for (size_t i = 0; i != units.size(); ++i) {
			if (units[i]->Type && !units[i]->Type->BoolFlag[SELECTABLEBYRECTANGLE_INDEX].value) {
				tainted = true;
			}
		}
	}

private:
	std::vector<CUnit *> units;  /// Units in the group
	bool tainted;    /// Group hold unit which can't be SelectableByRectangle
};                                       /// group of units

static CUnitGroup Groups[NUM_GROUPS];    /// Number of groups predefined

/*----------------------------------------------------------------------------
--  Functions
----------------------------------------------------------------------------*/

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
		file.printf("Group(%d, %lu, {", g, (long unsigned int)Groups[g].getUnits().size());
		for (size_t i = 0; i < Groups[g].getUnits().size(); ++i) {
			file.printf("\"%s\", ", UnitReference(*Groups[g].getUnits()[i]).c_str());
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
		Groups[i].init();
	}
}

bool IsGroupTainted(int num)
{
	Assert(num < NUM_GROUPS);
	return Groups[num].isTainted();
}

/**
**  Return the units of group num
**
**  @param num  Group number.
**
**  @return     Returns an array of all units in the group.
*/
const std::vector<CUnit *> &GetUnitsOfGroup(int num)
{
	Assert(num < NUM_GROUPS);
	return Groups[num].getUnits();
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

	for (size_t i = 0; i != group.getUnits().size(); ++i) {
		CUnit &unit = *group.getUnits()[i];
		unit.GroupId &= ~(1 << num);
		Assert(!unit.Destroyed);
	}
	group.init();
}

/**
**  Add units to group num contents from unit array "units"
**
**  @param units   Array of units to place into group.
**  @param nunits  Number of units in array.
**  @param num     Group number for storage.
*/
void AddToGroup(CUnit **units, unsigned int nunits, int num)
{
	Assert(num <= NUM_GROUPS);

	CUnitGroup &group = Groups[num];
	for (size_t i = 0; group.getUnits().size() < MaxSelectable && i < nunits; ++i) {
		// Add to group only if they are on our team
		// Buildings can be in group but it "taint" the group.
		// Taited groups normaly select only SelectableByRectangle units but
		// you can force selection to show hiden members (with buildings) by
		// selecting ALT-(SHIFT)-#
		CUnit &unit = *units[i];
		group.add(unit, num);
	}
}

/**
**  Set group num contents to unit array "units"
**
**  @param units   Array of units to place into group.
**  @param nunits  Number of units in array.
**  @param num     Group number for storage.
*/
void SetGroup(CUnit **units, unsigned int nunits, int num)
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

		group.remove(unit);
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

	const int grpNum = LuaToNumber(l, 1);
	if (NUM_GROUPS <= grpNum) {
		LuaError(l, "grpIndex out of bound");
	}
	CUnitGroup &grp = Groups[grpNum];
	grp.init();
	//grp.Units.size() = LuaToNumber(l, 2);
	const int args = lua_rawlen(l, 3);
	for (int j = 0; j < args; ++j) {
		const char *str = LuaToString(l, 3, j + 1);
		grp.add(UnitManager->GetSlotUnit(strtol(str + 1, NULL, 16)), grpNum);
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
