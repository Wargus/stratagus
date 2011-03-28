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
/**@name depend.cpp - The units/upgrade dependencies */
//
//      (c) Copyright 2000-2011 by Vladi Belperchinov-Shabanski, Lutz Sammer,
//                                 Jimmy Salmon and Pali Rohár
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
#include "upgrade_structs.h"
#include "upgrade.h"
#include "depend.h"
#include "player.h"
#include "script.h"
#include "unittype.h"

/*----------------------------------------------------------------------------
--  Variables
----------------------------------------------------------------------------*/

	/// All dependencies hash
static DependRule *DependHash[101];

/*----------------------------------------------------------------------------
--  Functions
----------------------------------------------------------------------------*/

/**
**  Add a new dependency. If already exits append to and rule.
**
**  @param target    Target of the dependency.
**  @param required  Requirement of the dependency.
**  @param count     Amount of the required needed.
**  @param or_flag   Start of or rule.
*/
static void AddDependency(const std::string &target, const std::string required, int count,
	int or_flag)
{
	DependRule rule;
	DependRule *node;
	DependRule *temp;
	int hash;

	//
	//  Setup structure.
	//
	if (!strncmp(target.c_str(), "unit-", 5)) {
		// target string refers to unit-xxx
		rule.Type = DependRuleUnitType;
		rule.Kind.UnitType = UnitTypeByIdent(target);
	} else if (!strncmp(target.c_str(), "upgrade-", 8)) {
		// target string refers to upgrade-XXX
		rule.Type = DependRuleUpgrade;
		rule.Kind.Upgrade = CUpgrade::Get(target);
	} else {
		DebugPrint("dependency target `%s' should be unit-type or upgrade\n" _C_
			target.c_str());
		return;
	}

	hash = (int)((intptr_t)rule.Kind.UnitType % (sizeof(DependHash) / sizeof(*DependHash)));

	//
	//  Find correct hash slot.
	//
	if ((node = DependHash[hash])) {  // find correct entry
		while (node->Type != rule.Type ||
				node->Kind.Upgrade != rule.Kind.Upgrade) {
			if (!node->Next) {  // end of list
				temp = new DependRule;
				temp->Next = NULL;
				temp->Rule = NULL;
				temp->Type = rule.Type;
				temp->Kind = rule.Kind;
				node->Next = temp;
				node = temp;
				break;
			}
			node = node->Next;
		}
	} else {  // create new slow
		node = new DependRule;
		node->Next = NULL;
		node->Rule = NULL;
		node->Type = rule.Type;
		node->Kind = rule.Kind;
		DependHash[hash] = node;
	}

	//
	//  Adjust count.
	//
	if (count < 0 || count > 255) {
		DebugPrint("wrong count `%d' range 0 .. 255\n" _C_ count);
		count = 255;
	}

	temp = new DependRule;
	temp->Rule = NULL;
	temp->Next = NULL;
	temp->Count = count;
	//
	//  Setup structure.
	//
	if (!strncmp(required.c_str(), "unit-", 5)) {
		// required string refers to unit-xxx
		temp->Type = DependRuleUnitType;
		temp->Kind.UnitType = UnitTypeByIdent(required);
	} else if (!strncmp(required.c_str(), "upgrade-", 8)) {
		// required string refers to upgrade-XXX
		temp->Type = DependRuleUpgrade;
		temp->Kind.Upgrade = CUpgrade::Get(required);
	} else {
		DebugPrint("dependency required `%s' should be unit-type or upgrade\n" _C_
			required.c_str());
		delete temp;
		return;
	}

	if (or_flag) {
		// move rule to temp->next
		temp->Next = node->Rule;  // insert rule
		node->Rule = temp;
	} else {
		// move rule to temp->rule
		temp->Rule = node->Rule;  // insert rule

		// also Link temp to old "or" list
		if (node->Rule) {
			temp->Next = node->Rule->Next;
		}

		node->Rule = temp;
	}

#ifdef neverDEBUG
	printf("New rules are :");
	node = node->Rule;
	while (node) {
		temp = node;
		while (temp) {
			printf("temp->Kind.UnitType=%p ", temp->Kind.UnitType);
			temp = temp->Rule;
		}
		node = node->Next;
		printf("\n or ... ");
	}
	printf("\n");
#endif
}

/**
**  Check if this upgrade or unit is available.
**
**  @param player  For this player available.
**  @param rule  .
**
**  @return        True if available, false otherwise.
*/
static bool CheckDependByRule(const CPlayer &player, DependRule &rule)
{
	const DependRule *node;

	//  Find rule
	int i = (int)((intptr_t)rule.Kind.UnitType % (sizeof(DependHash) / sizeof(*DependHash)));

	if ((node = DependHash[i])) {  // find correct entry
		while (node->Type != rule.Type || node->Kind.Upgrade != rule.Kind.Upgrade) {
			if (!node->Next) {  // end of list
				return true;
			}
			node = node->Next;
		}
	} else {
		return true;
	}

	//
	//  Prove the rules
	//
	node = node->Rule;

	while (node) {
		const DependRule *temp = node;
		while (temp) {
			switch (temp->Type) {
				case DependRuleUnitType:
					i = player.HaveUnitTypeByType(*temp->Kind.UnitType);
					if (temp->Count ? i < temp->Count : i) {
						goto try_or;
					}
					break;
				case DependRuleUpgrade:
					i = UpgradeIdAllowed(player, temp->Kind.Upgrade->ID) != 'R';
					if (temp->Count ? i : !i) {
						goto try_or;
					}
					break;
			}
			temp = temp->Rule;
		}
		return true;  // all rules matches.

try_or:
		node = node->Next;
	}
	return false;  // no rule matches
}

/**
**  Check if this upgrade or unit is available.
**
**  @param player  For this player available.
**  @param target  Unit or Upgrade.
**
**  @return        True if available, false otherwise.
*/
bool CheckDependByIdent(const CPlayer &player, const std::string &target)
{
	DependRule rule;

	//
	//  first have to check, if target is allowed itself
	//
	if (!strncmp(target.c_str(), "unit-", 5)) {
		// target string refers to unit-XXX
		rule.Kind.UnitType = UnitTypeByIdent(target);
		if (UnitIdAllowed(player, rule.Kind.UnitType->Slot) == 0) {
			return false;
		}
		rule.Type = DependRuleUnitType;
	} else if (!strncmp(target.c_str(), "upgrade-", 8)) {
		// target string refers to upgrade-XXX
		rule.Kind.Upgrade = CUpgrade::Get(target);
		if (UpgradeIdAllowed(player, rule.Kind.Upgrade->ID) != 'A') {
			return false;
		}
		rule.Type = DependRuleUpgrade;
	} else {
		DebugPrint("target `%s' should be unit-type or upgrade\n" _C_ target.c_str());
		return false;
	}

	return CheckDependByRule(player, rule);
}


/**
**  Check if this upgrade or unit is available.
**
**  @param player  For this player available.
**  @param target  Unit or Upgrade.
**
**  @return        True if available, false otherwise.
*/
bool CheckDependByType(const CPlayer &player, const CUnitType &type)
{
	if (UnitIdAllowed(player, type.Slot) == 0) {
		return false;
	}
	DependRule rule;

	rule.Kind.UnitType = const_cast<CUnitType *>(&type);
	rule.Type = DependRuleUnitType;
	return CheckDependByRule(player, rule);
}


/**
**  Initialize unit and upgrade dependencies.
*/
void InitDependencies()
{
}

/**
**  Clean up unit and upgrade dependencies.
*/
void CleanDependencies()
{
	unsigned u;
	DependRule *node;
	DependRule *rule;
	DependRule *temp;
	DependRule *next;

	// Free all dependencies

	for (u = 0; u < sizeof(DependHash) / sizeof(*DependHash); ++u) {
		node = DependHash[u];
		while (node) {  // all hash links
			// All or cases

			rule = node->Rule;
			while (rule) {
				if (rule) {
					temp = rule->Rule;
					while (temp) {
						next = temp;
						temp = temp->Rule;
						delete next;
					}
				}
				temp = rule;
				rule = rule->Next;
				delete temp;
			}
			temp = node;
			node = node->Next;
			delete temp;
		}
		DependHash[u] = NULL;
	}
}

/*----------------------------------------------------------------------------
--  Ccl part of dependencies
----------------------------------------------------------------------------*/

/**
**  Define a new dependency.
**
**  @param l  Lua state.
*/
static int CclDefineDependency(lua_State *l)
{
	const char *target;
	const char *required;
	int count;
	const char *value;
	int or_flag;
	int args;
	int j;
	int subargs;
	int k;

	args = lua_gettop(l);
	j = 0;

	target = LuaToString(l, j + 1);
	++j;

	//
	//  All or rules.
	//
	or_flag = 0;
	for (; j < args; ++j) {
		if (!lua_istable(l, j + 1)) {
			LuaError(l, "incorrect argument");
		}
		subargs = lua_objlen(l, j + 1);

		for (k = 0; k < subargs; ++k) {
			lua_rawgeti(l, j + 1, k + 1);
			required = LuaToString(l, -1);
			lua_pop(l, 1);
			count = 1;
			if (k + 1 < subargs) {
				lua_rawgeti(l, j + 1, k + 2);
				if (lua_isnumber(l, -1)) {
					count = LuaToNumber(l, -1);
					++k;
				}
				lua_pop(l, 1);
			}

			AddDependency(target, required, count, or_flag);
			or_flag = 0;
		}
		if (j + 1 < args) {
			++j;
			value = LuaToString(l, j + 1);
			if (strcmp(value, "or")) {
				LuaError(l, "not or symbol: %s" _C_ value);
				return 0;
			}
			or_flag = 1;
		}
	}

	return 0;
}

/**
**  Get the dependency.
**
**  @todo not written.
**
**  @param l  Lua state.
*/
static int CclGetDependency(lua_State *l)
{
	DebugPrint("FIXME: write this %p\n" _C_ (void*)l);

	return 0;
}

/**
**  Check the dependency.
**
**  @todo not written.
**
**  @param l  Lua state.
*/
static int CclCheckDependency(lua_State *l)
{
	DebugPrint("FIXME: write this %p\n" _C_ (void *)l);

	return 0;
}

/**
**  Register CCL features for dependencies.
*/
void DependenciesCclRegister()
{
	lua_register(Lua, "DefineDependency", CclDefineDependency);
	lua_register(Lua, "GetDependency", CclGetDependency);
	lua_register(Lua, "CheckDependency", CclCheckDependency);
}

//@}
