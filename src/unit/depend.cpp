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
#include "unit.h"
#include "unittype.h"
#include "upgrade.h"

namespace
{
class DependRule
{
public:
	DependRule(std::string_view name, std::size_t count) :
		typeVar([&]() -> std::variant<const CUnitType *, const CUpgrade *> {
			if (starts_with(name, "unit-")) {
				return &UnitTypeByIdent(name);
			} else if (starts_with(name, "upgrade-")) {
				const auto *upgrade = CUpgrade::Get(name);
				if (!upgrade) {
					fprintf(stderr, "upgrade not found: %s\n", name.data());
					ExitFatal(-1);
				}
				return upgrade;
			} else {
				fprintf(
					stderr, "dependency target '%s' should be unit-type or upgrade\n", name.data());
				ExitFatal(-1);
			}
		}()),
		expected(count)
	{}

	bool isValid(const CPlayer &player) const
	{
		return std::visit([&](const auto *type) { return isValid(player, *type, expected); },
		                  typeVar);
	}

	const std::string &getDisplayName() const
	{
		return std::visit(
			[](const auto *type) -> const std::string & { return DependRule::getName(*type); },
			typeVar);
	}

private:
	static bool isValid(const CPlayer &player, const CUnitType &unitType, std::size_t expected)
	{
		const std::size_t count = player.HaveUnitTypeByType(unitType);
		return (expected != 0 ? count >= expected : (count != 0));
	}

	static bool isValid(const CPlayer &player, const CUpgrade &upgrade, std::size_t expected)
	{
		const bool allowed = UpgradeIdAllowed(player, upgrade.ID) == 'R';
		return expected ? allowed : !allowed;
	}

	static const std::string &getName(const CUnitType &unitType) { return unitType.Name; }
	static const std::string &getName(const CUpgrade &upgrade) { return upgrade.Name; }

public:
	std::variant<const CUnitType *, const CUpgrade *> typeVar;
	std::size_t expected = 0;
};

class DependAndRule
{
public:
	bool isValid(const CPlayer &player) const
	{
		return ranges::all_of(rules, [&](const auto &rule) { return rule.isValid(player); });
	}

	std::string getRequirementString(const CPlayer &player) const
	{
		std::string res;
		for (const auto &rule : rules) {
			if (!rule.isValid(player)) {
				res += "-";
				res += rule.getDisplayName();
				res += "\n";
			}
		}
		return res;
	}

public:
	std::vector<DependRule> rules;
};

class DependOrRule
{
public:
	bool isValid(const CPlayer &player) const
	{
		return ranges::any_of(rules, [&](const auto &rule) { return rule.isValid(player); });
	}

	std::string getRequirementString(const CPlayer &player) const
	{
		if (isValid(player)) {
			return "";
		}
		auto it = ranges::find_if(rules, [&](const auto &rule) { return !rule.isValid(player); });
		return it != rules.end() ? _("Requirements:\n") + it->getRequirementString(player) : "";
	}

public:
	std::vector<DependAndRule> rules;
};
} // namespace
/*----------------------------------------------------------------------------
--  Variables
----------------------------------------------------------------------------*/

/// All dependencies hash
static std::map<std::string, DependOrRule, std::less<>> Depends;

/*----------------------------------------------------------------------------
--  Functions
----------------------------------------------------------------------------*/

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
	// Special case for spells
	if (starts_with(button.ValueStr, "spell-")) {
		std::string rules("");
		if (button.Allowed && IsButtonAllowed(*Selected[0], button) == false) {
			if (starts_with(button.AllowStr, "upgrade-")) {
				rules.insert(0, _("Requirements:\n"));
				rules.append("-");
				rules.append(AllUpgrades[UpgradeIdByIdent(button.AllowStr)]->Name);
				rules.append("\n");
			}
		}
		return rules;
	}
	if (!starts_with(button.ValueStr, "unit-") && !starts_with(button.ValueStr, "upgrade-")) {
		DebugPrint("target '%s' should be unit-type or upgrade\n", button.ValueStr.c_str());
		return "";
	}
	auto it = Depends.find(button.ValueStr);
	if (it == Depends.end()) { // No rules
		return "";
	}
	return it->second.getRequirementString(player);
}

/**
**  Check if this upgrade or unit is available.
**
**  @param player  For this player available.
**  @param target  Unit or Upgrade.
**
**  @return        True if available, false otherwise.
*/
bool CheckDependByIdent(const CPlayer &player, std::string_view target)
{
	// first have to check, if target is allowed itself
	if (starts_with(target, "unit-")) {
		if (UnitIdAllowed(player, UnitTypeByIdent(target).Slot) == 0) {
			return false;
		}
	} else if (starts_with(target, "upgrade-")) {
		if (UpgradeIdAllowed(player, CUpgrade::Get(target)->ID) != 'A') {
			return false;
		}
	} else {
		DebugPrint("target '%s' should be unit-type or upgrade\n", target.data());
		return false;
	}
	auto it = Depends.find(target);
	if (it == Depends.end()) { // No rules
		return true;
	}
	return it->second.isValid(player);
}

/**
**  Check if this unit is available.
**
**  @param player  For this player available.
**  @param type    Unit.
**
**  @return        True if available, false otherwise.
*/
bool CheckDependByType(const CPlayer &player, const CUnitType &type)
{
	return CheckDependByIdent(player, type.Ident);
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
	Depends.clear();
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
	const std::string_view target = LuaToString(l, 1);

	if (starts_with(target, "unit-")) {
		// Check unit exists
		UnitTypeByIdent(target);
	} else if (starts_with(target, "upgrade-")) {
		// Check upgrade exists
		if (auto* upgrade = CUpgrade::Get(target); !upgrade) {
			fprintf(stderr, "upgrade not found: %s\n", target.data());
			ExitFatal(-1);
		}
	} else {
		fprintf(stderr, "dependency target '%s' should be unit-type or upgrade\n", target.data());
		ExitFatal(-1);
	}

	auto &or_rule = Depends[std::string(target)];
	//  All or-rules.
	for (int j = 1; j < args; ++j) {
		if (!lua_istable(l, j + 1)) {
			LuaError(l, "incorrect argument");
		}
		or_rule.rules.emplace_back();
		auto &and_rule = or_rule.rules.back();
		const int subargs = lua_rawlen(l, j + 1);

		//  All and-rules.
		for (int k = 0; k < subargs; ++k) {
			const std::string_view required = LuaToString(l, j + 1, k + 1);
			int count = 1;
			if (k + 1 < subargs) {
				lua_rawgeti(l, j + 1, k + 2);
				if (lua_isnumber(l, -1)) {
					count = LuaToNumber(l, -1);
					++k;
				}
				lua_pop(l, 1);
			}
			and_rule.rules.emplace_back(required, count);
		}
		if (j + 1 < args) {
			++j;
			const std::string_view value = LuaToString(l, j + 1);
			if (value != "or") {
				LuaError(l, "not 'or' symbol: %s", value.data());
				return 0;
			}
		}
	}
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
	const std::string_view object = LuaToString(l, 2);
	lua_pop(l, 1);
	const CPlayer *player = CclGetPlayer(l);
	if (player == nullptr) {
		LuaError(l, "bad player: %s", lua_tostring(l, 1));
	}

	lua_pushboolean(l, CheckDependByIdent(*player, object));
	return 1;
}

/**
**  Register CCL features for dependencies.
*/
void DependenciesCclRegister()
{
	lua_register(Lua, "DefineDependency", CclDefineDependency);
	lua_register(Lua, "CheckDependency", CclCheckDependency);
}

//@}
