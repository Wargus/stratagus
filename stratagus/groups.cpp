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
/**@name groups.c - The units' groups handling. */
//
//      (c) Copyright 1999-2004 by Patrice Fortier, Lutz Sammer,
//                                 and Jimmy Salmon
//
//      This program is free software; you can redistribute it and/or modify
//      it under the terms of the GNU General Public License as published by
//      the Free Software Foundation; version 2 dated June, 1991.
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
#include "unit.h"
#include "script.h"

/*----------------------------------------------------------------------------
--  Variables
----------------------------------------------------------------------------*/

/**
**  Defines a group of units.
*/
typedef struct _unit_group_ {
	Unit** Units;                       /// Units in the group
	int    NumUnits;                    /// How many units in the group
} UnitGroup;                            /// group of units

global UnitGroup Groups[NUM_GROUPS];    /// Number of groups predefined

/*----------------------------------------------------------------------------
--  Functions
----------------------------------------------------------------------------*/

/**
**  Initialize group part.
**
**  @todo Not needed with the new unit code!
*/
global void InitGroups(void)
{
	int i;

	for (i = 0; i < NUM_GROUPS; ++i) {
		if (!Groups[i].Units) {
			Groups[i].Units = malloc(MaxSelectable * sizeof(Unit*));
		}
	}
}

/**
**  Save groups.
**
**  @param file  Output file.
*/
global void SaveGroups(CLFile* file)
{
	int i;
	int g;
	char* ref;

	CLprintf(file, "\n--- -----------------------------------------\n");
	CLprintf(file, "--- MODULE: groups $Id$\n\n");

	for (g = 0; g < NUM_GROUPS; ++g) {
		CLprintf(file, "Group(%d, %d, {", g, Groups[g].NumUnits);
		for (i = 0; i < Groups[g].NumUnits; ++i) {
			ref = UnitReference(Groups[g].Units[i]);
			CLprintf(file, "\"%s\", ", ref);
			free(ref);
		}
		CLprintf(file, "})\n");
	}
}

/**
**  Clean up group part.
*/
global void CleanGroups(void)
{
	int i;

	for (i = 0; i < NUM_GROUPS; ++i) {
		if (Groups[i].Units) {
			free(Groups[i].Units);
		}
		memset(&Groups[i], 0, sizeof(Groups[i]));
	}
}

/**
**  Return the number of units of group #num
**
**  @param num  Group number.
**
**  @return     Returns the number of units in the group.
*/
global int GetNumberUnitsOfGroup(int num)
{
	return Groups[num].NumUnits;
}

/**
**  Return the units of group #num
**
**  @param num  Group number.
**
**  @return     Returns an array of all units in the group.
*/
global Unit** GetUnitsOfGroup(int num)
{
	return Groups[num].Units;
}

/**
**  Clear contents of group #num
**
**  @param num  Group number.
*/
global void ClearGroup(int num)
{
	UnitGroup* group;
	int i;

	group = &Groups[num];
	for (i = 0; i < group->NumUnits; ++i) {
		group->Units[i]->GroupId &= ~(1 << num);
		DebugCheck(group->Units[i]->Destroyed);
	}
	group->NumUnits = 0;
}

/**
**  Add units to group #num contents from unit array "units"
**
**  @param units   Array of units to place into group.
**  @param nunits  Number of units in array.
**  @param num     Group number for storage.
*/
global void AddToGroup(Unit** units, int nunits, int num)
{
	UnitGroup* group;
	int i;

	DebugCheck(num > NUM_GROUPS);

	group = &Groups[num];
	for (i = 0; group->NumUnits < MaxSelectable && i < nunits; ++i) {
		group->Units[group->NumUnits++] = units[i];
		units[i]->GroupId |= (1 << num);
	}
}

/**
**  Set group #num contents to unit array "units"
**
**  @param units   Array of units to place into group.
**  @param nunits  Number of units in array.
**  @param num     Group number for storage.
*/
global void SetGroup(Unit** units, int nunits, int num)
{
	DebugCheck(num > NUM_GROUPS || nunits > MaxSelectable);

	ClearGroup(num);
	AddToGroup(units, nunits, num);
}

/**
**  Remove unit from its groups
**
**  @param unit  Unit to remove from group.
*/
global void RemoveUnitFromGroups(Unit* unit)
{
	UnitGroup* group;
	int num;
	int i;

	DebugCheck(unit->GroupId == 0);  // unit doesn't belong to a group

	for (num = 0; unit->GroupId; ++num, unit->GroupId >>= 1) {
		if ((unit->GroupId & 1) != 1) {
			continue;
		}

		group = &Groups[num];
		for (i = 0; group->Units[i] != unit; ++i) {
			;
		}

		DebugCheck(i >= group->NumUnits);  // oops not found

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
**  @param group  Group number
**  @param num    Number of units in group
**  @param units  Units in group
*/
local int CclGroup(lua_State* l)
{
	int i;
	UnitGroup* grp;
	int args;
	int j;

	if (lua_gettop(l) != 3) {
		LuaError(l, "incorrect argument");
	}

	InitGroups();
	grp = &Groups[(int)LuaToNumber(l, 1)];
	grp->NumUnits = LuaToNumber(l, 2);
	i = 0;
	args = luaL_getn(l, 3);
	for (j = 0; j < args; ++j) {
		const char* str;

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
global void GroupCclRegister(void)
{
	lua_register(Lua, "Group", CclGroup);
}

//@}
