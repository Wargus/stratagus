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

#include "stratagus.h"

#include "depend.h"

#include "interface.h"
#include "player.h"
#include "script.h"
#include "translate.h"
#include "trigger.h"
#include "unit.h"
#include "unittype.h"
#include "upgrade_structs.h"
#include "upgrade.h"

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
static void AddDependency(const std::string &target, const std::string &required, int count, int or_flag)
{
	DependRule rule;

	//  Setup structure.
	if (!strncmp(target.c_str(), "unit-", 5)) {
		// target string refers to unit-xxx
		rule.Type = DependRuleUnitType;
		rule.Kind.UnitType = UnitTypeByIdent(target);
	} else if (!strncmp(target.c_str(), "upgrade-", 8)) {
		// target string refers to upgrade-XXX
		rule.Type = DependRuleUpgrade;
		rule.Kind.Upgrade = CUpgrade::Get(target);
	} else {
		DebugPrint("dependency target `%s' should be unit-type or upgrade\n" _C_ target.c_str());
		return;
	}

	int hash = (int)((intptr_t)rule.Kind.UnitType % (sizeof(DependHash) / sizeof(*DependHash)));

	//  Find correct hash slot.
	DependRule *node = DependHash[hash];

	if (node) {  // find correct entry
		while (node->Type != rule.Type || node->Kind.Upgrade != rule.Kind.Upgrade) {
			if (!node->Next) {  // end of list
				DependRule *temp = new DependRule;
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

	//  Adjust count.
	if (count < 0 || count > 255) {
		DebugPrint("wrong count `%d' range 0 .. 255\n" _C_ count);
		count = 255;
	}

	DependRule *temp = new DependRule;
	temp->Rule = NULL;
	temp->Next = NULL;
	temp->Count = count;

	//  Setup structure.
	if (!strncmp(required.c_str(), "unit-", 5)) {
		// required string refers to unit-xxx
		temp->Type = DependRuleUnitType;
		temp->Kind.UnitType = UnitTypeByIdent(required);
	} else if (!strncmp(required.c_str(), "upgrade-", 8)) {
		// required string refers to upgrade-XXX
		temp->Type = DependRuleUpgrade;
		temp->Kind.Upgrade = CUpgrade::Get(required);
	} else {
		DebugPrint("dependency required `%s' should be unit-type or upgrade\n" _C_ required.c_str());
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
	fprintf(stdout, "New rules are :");
	node = node->Rule;
	while (node) {
		temp = node;
		while (temp) {
			fprintf(stdout, "temp->Kind.UnitType=%s ", temp->Kind.UnitType->Ident.c_str());
			temp = temp->Rule;
		}
		node = node->Next;
		fprintf(stdout, "\n or ... ");
	}
	fprintf(stdout, "\n");
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
	//  Find rule
	int i = (int)((intptr_t)rule.Kind.UnitType % (sizeof(DependHash) / sizeof(*DependHash)));
	const DependRule *node = DependHash[i];

	if (node) {  // find correct entry
		while (node->Type != rule.Type || node->Kind.Upgrade != rule.Kind.Upgrade) {
			if (!node->Next) {  // end of list
				return true;
			}
			node = node->Next;
		}
	} else {
		return true;
	}

	//  Prove the rules
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
std::string PrintDependencies(const CPlayer &player, const ButtonAction &button)
{
	DependRule rule;
	std::string rules("");

	//
	//  first have to check, if target is allowed itself
	//
	if (!strncmp(button.ValueStr.c_str(), "unit-", 5)) {
		// target string refers to unit-XXX
		rule.Kind.UnitType = UnitTypeByIdent(button.ValueStr);
		rule.Type = DependRuleUnitType;
	} else if (!strncmp(button.ValueStr.c_str(), "upgrade-", 8)) {
		// target string refers to upgrade-XXX
		rule.Kind.Upgrade = CUpgrade::Get(button.ValueStr);
		rule.Type = DependRuleUpgrade;
	} else if (!strncmp(button.ValueStr.c_str(), "spell-", 6)) {
		// Special case for spells
		if (button.Allowed && IsButtonAllowed(*Selected[0], button) == false) {
			if (!strncmp(button.AllowStr.c_str(), "upgrade-", 8)) {
				rules.insert(0, _("Requirements:\n"));
				rules.append("-");
				rules.append(AllUpgrades[UpgradeIdByIdent(button.AllowStr)]->Name);
				rules.append("\n");
			}
		}
		return rules;
	} else {
		DebugPrint("target `%s' should be unit-type or upgrade\n" _C_ button.ValueStr.c_str());
		return rules;
	}

	//  Find rule
	int i = (int)((intptr_t)rule.Kind.UnitType % (sizeof(DependHash) / sizeof(*DependHash)));
	const DependRule *node = DependHash[i];

	if (node) {  // find correct entry
		while (node->Type != rule.Type || node->Kind.Upgrade != rule.Kind.Upgrade) {
			if (!node->Next) {  // end of list
				return rules;
			}
			node = node->Next;
		}
	} else {
		return rules;
	}

	//  Prove the rules
	node = node->Rule;

	while (node) {
		const DependRule *temp = node;
		std::string subrules("");
		while (temp) {
			if (temp->Type == DependRuleUnitType) {
				i = player.HaveUnitTypeByType(*temp->Kind.UnitType);
				if (temp->Count ? i < temp->Count : i) {
					subrules.append("-");
					subrules.append(temp->Kind.UnitType->Name.c_str());
					subrules.append("\n");
				}
			} else if (temp->Type == DependRuleUpgrade) {
				i = UpgradeIdAllowed(player, temp->Kind.Upgrade->ID) != 'R';
				if (temp->Count ? i : !i) {
					subrules.append("-");
					subrules.append(temp->Kind.Upgrade->Name.c_str());
					subrules.append("\n");
				}
			}
			temp = temp->Rule;
		}
		if (subrules.empty()) {
			return subrules;
		}
		rules.clear();
		rules.append(subrules);
		node = node->Next;
	}
	rules.insert(0, _("Requirements:\n"));
	return rules;
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

	rule.Kind.UnitType = &type;
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
	// Free all dependencies

	for (unsigned int u = 0; u < sizeof(DependHash) / sizeof(*DependHash); ++u) {
		DependRule *node = DependHash[u];
		while (node) {  // all hash links
			// All or cases

			DependRule *rule = node->Rule;
			while (rule) {
				if (rule) {
					DependRule *temp = rule->Rule;
					while (temp) {
						DependRule *next = temp;
						temp = temp->Rule;
						delete next;
					}
				}
				DependRule *temp = rule;
				rule = rule->Next;
				delete temp;
			}
			DependRule *temp = node;
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
	const int args = lua_gettop(l);
	const char *target = LuaToString(l, 1);

	//  All or rules.
	int or_flag = 0;
	for (int j = 1; j < args; ++j) {
		if (!lua_istable(l, j + 1)) {
			LuaError(l, "incorrect argument");
		}
		const int subargs = lua_rawlen(l, j + 1);

		for (int k = 0; k < subargs; ++k) {
			const char *required = LuaToString(l, j + 1, k + 1);
			int count = 1;
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
			const char *value = LuaToString(l, j + 1);
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
	DebugPrint("FIXME: write this %p\n" _C_(void *)l);

	return 0;
}

/**
**  Checks if dependencies are met.
**
**  @return true if the dependencies are met.
**
**  @param l  Lua state.
**  Argument 1: player
**  Argument 2: object which we want to check the dependencies of
*/
static int CclCheckDependency(lua_State *l)
{
	LuaCheckArgs(l, 2);
	const char *object = LuaToString(l, 2);
	lua_pop(l, 1);
	const int plynr = TriggerGetPlayer(l);
	if (plynr == -1) {
		LuaError(l, "bad player: %i" _C_ plynr);
	}
	CPlayer &player = Players[plynr];

	lua_pushboolean(l, CheckDependByIdent(player, object));
	return 1;
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
