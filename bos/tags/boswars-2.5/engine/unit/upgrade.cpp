//     ____                _       __               
//    / __ )____  _____   | |     / /___ ___________
//   / __  / __ \/ ___/   | | /| / / __ `/ ___/ ___/
//  / /_/ / /_/ (__  )    | |/ |/ / /_/ / /  (__  ) 
// /_____/\____/____/     |__/|__/\__,_/_/  /____/  
//                                              
//       A futuristic real-time strategy game.
//          This file is part of Bos Wars.
//
/**@name upgrade.cpp - The upgrade/allow functions. */
//
//      (c) Copyright 1999-2008 by Vladi Belperchinov-Shabanski and Jimmy Salmon
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <vector>
#include <map>

#include "upgrade.h"
#include "player.h"
#include "interface.h"
#include "map.h"
#include "script.h"
#include "spells.h"
#include "unit.h"
#include "unittype.h"
#include "actions.h"
#include "iolib.h"

/*----------------------------------------------------------------------------
--  Declarations
----------------------------------------------------------------------------*/

static void AllowUnitId(CPlayer *player, int id, int units);

/*----------------------------------------------------------------------------
--  Functions
----------------------------------------------------------------------------*/

/**
**  Save state of the dependencies to file.
**
**  @param file  Output file.
*/
void SaveUpgrades(CFile *file)
{
	file->printf("\n-- -----------------------------------------\n");
	file->printf("-- MODULE: upgrades\n\n");

	//
	//  Save the allow
	//
	for (size_t i = 0; i < UnitTypes.size(); ++i) {
		file->printf("DefineUnitAllow(\"%s\", ", UnitTypes[i]->Ident.c_str());
		for (int p = 0; p < PlayerMax; ++p) {
			if (p) {
				file->printf(", ");
			}
			file->printf("%d", Players[p].Allow.Units[i]);
		}
		file->printf(")\n");
	}
	file->printf("\n");
}

/*----------------------------------------------------------------------------
--  Ccl part of upgrades
----------------------------------------------------------------------------*/

/**
**  Define which units are allowed and how much.
*/
static int CclDefineUnitAllow(lua_State *l)
{
	const char *ident;
	int i;
	int args;
	int j;
	int id;

	args = lua_gettop(l);
	j = 0;
	ident = LuaToString(l, j + 1);
	++j;

	if (strncmp(ident, "unit-", 5)) {
		DebugPrint(" wrong ident %s\n" _C_ ident);
		return 0;
	}
	id = UnitTypeIdByIdent(ident);

	i = 0;
	for (; j < args && i < PlayerMax; ++j) {
		AllowUnitId(&Players[i], id, LuaToNumber(l, j + 1));
		++i;
	}

	return 0;
}

/**
**  Define which units/upgrades are allowed.
*/
static int CclDefineAllow(lua_State *l)
{
	const char *ident;
	const char *ids;
	int n;
	int args;
	int id;

	args = lua_gettop(l);
	for (int j = 0; j < args; ++j) {
		ident = LuaToString(l, j + 1);
		++j;
		ids = LuaToString(l, j + 1);

		n = strlen(ids);
		if (n > PlayerMax) {
			fprintf(stderr, "%s: Allow string too long %d\n", ident, n);
			n = PlayerMax;
		}

		if (!strncmp(ident, "unit-", 5)) {
			id = UnitTypeIdByIdent(ident);
			for (int i = 0; i < n; ++i) {
				if (ids[i] == 'A') {
					AllowUnitId(&Players[i], id, UnitMax);
				} else if (ids[i] == 'F') {
					AllowUnitId(&Players[i], id, 0);
				}
			}
		} else {
			DebugPrint(" wrong ident %s\n" _C_ ident);
		}
	}

	return 0;
}

/**
**  Register CCL features for upgrades.
*/
void UpgradesCclRegister(void)
{
	lua_register(Lua, "DefineAllow", CclDefineAllow);
	lua_register(Lua, "DefineUnitAllow", CclDefineUnitAllow);
}

/*----------------------------------------------------------------------------
-- General/Map functions
----------------------------------------------------------------------------*/

// AllowStruct will be static in the player so will be
// load/saved with the player struct

/**
**  UnitType ID by identifier.
**
**  @param ident  The unit-type identifier.
**  @return       Unit-type ID (int) or -1 if not found.
*/
int UnitTypeIdByIdent(const std::string &ident)
{
	const CUnitType *type;

	if ((type = UnitTypeByIdent(ident))) {
		return type->Slot;
	}
	DebugPrint(" fix this %s\n" _C_ ident.c_str());
	Assert(0);
	return -1;
}

/*----------------------------------------------------------------------------
--  Allow(s)
----------------------------------------------------------------------------*/

// all the following functions are just map handlers, no specific notes

/**
**  Change allow for a unit-type.
**
**  @param player  Player to change
**  @param id      unit type id
**  @param units   maximum amount of units allowed
*/
static void AllowUnitId(CPlayer *player, int id, int units)
{
	player->Allow.Units[id] = units;
}

/**
**  Return the allow state of the unit.
**
**  @param player   Check state of this player.
**  @param id       Unit identifier.
**
**  @return the allow state of the unit.
*/
int UnitIdAllowed(const CPlayer *player, int id)
{
	Assert(id >= 0 && id < UnitTypeMax);
	return player->Allow.Units[id];
}

//@}
