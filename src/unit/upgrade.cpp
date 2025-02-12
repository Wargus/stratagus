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
/**@name upgrade.cpp - The upgrade/allow functions. */
//
//      (c) Copyright 1999-2015 by Vladi Belperchinov-Shabanski, Jimmy Salmon
//		and Andrettin
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

#include <string>
#include <vector>
#include <map>

#include "stratagus.h"

#include "upgrade.h"

#include "action/action_train.h"
#include "commands.h"
#include "depend.h"
#include "interface.h"
#include "iolib.h"
#include "map.h"
#include "player.h"
#include "script.h"
#include "unit.h"
#include "unit_find.h"
#include "unittype.h"
#include "util.h"

/*----------------------------------------------------------------------------
--  Declarations
----------------------------------------------------------------------------*/

static void AllowUnitId(CPlayer &player, int id, int units);

/*----------------------------------------------------------------------------
--  Variables
----------------------------------------------------------------------------*/

std::vector<std::unique_ptr<CUpgrade>> AllUpgrades;           /// The main user useable upgrades

/// Upgrades modifiers
std::unique_ptr<CUpgradeModifier> UpgradeModifiers[UPGRADE_MODIFIERS_MAX];
/// Number of upgrades modifiers used
int NumUpgradeModifiers;

static std::map<std::string, CUpgrade *, std::less<>> Upgrades;

/*----------------------------------------------------------------------------
--  Functions
----------------------------------------------------------------------------*/

bool CUnitStats::operator == (const CUnitStats &rhs) const
{
	for (int i = 0; i != MaxCosts; ++i) {
		if (this->Costs[i] != rhs.Costs[i]) {
			return false;
		}
		if (this->Storing[i] != rhs.Storing[i]) {
			return false;
		}
		if (this->ImproveIncomes[i] != rhs.ImproveIncomes[i]) {
			return false;
		}
	}
	if (this->Variables != rhs.Variables) {
		return false;
	}
	return true;
}

bool CUnitStats::operator != (const CUnitStats &rhs) const
{
	return !(*this == rhs);
}

CUpgrade::CUpgrade(std::string ident) :
	Ident(std::move(ident))
{
}

/**
**  Create a new upgrade
**
**  @param ident  Upgrade identifier
*/
/* static */ CUpgrade *CUpgrade::New(std::string ident)
{
	CUpgrade *&upgrade = Upgrades[ident];
	if (upgrade) {
		return upgrade;
	} else {
		auto ptr = std::make_unique<CUpgrade>(ident);
		upgrade = ptr.get();
		upgrade->ID = AllUpgrades.size();
		AllUpgrades.push_back(std::move(ptr));
		return upgrade;
	}
}

/**
**  Get an upgrade
**
**  @param ident  Upgrade identifier
**
**  @return       Upgrade pointer or nullptr if not found.
*/
/* static */ CUpgrade *CUpgrade::Get(std::string_view ident)
{
	auto it = Upgrades.find(ident);
	if (it == Upgrades.end()) {
		ErrorPrint("upgrade not found: '%s'\n", ident.data());
		ExitFatal(-1);
	}
	return it->second;
}

/**
**  Init upgrade/allow structures
*/
void InitUpgrades()
{
}

/**
**  Cleanup the upgrade module.
*/
void CleanUpgrades()
{
	Upgrades.clear();

	//
	//  Free the upgrade modifiers.
	//
	for (auto &p : UpgradeModifiers) {
		p.reset();
	}
	NumUpgradeModifiers = 0;
}

/**
**  Save state of the dependencies to file.
**
**  @param file  Output file.
*/
void SaveUpgrades(CFile &file)
{
	file.printf("\n-- -----------------------------------------\n");
	file.printf("-- MODULE: upgrades\n\n");

	//
	//  Save the allow
	//
	for (std::vector<CUnitType *>::size_type i = 0; i < getUnitTypes().size(); ++i) {
		file.printf("DefineUnitAllow(\"%s\", ", getUnitTypes()[i]->Ident.c_str());
		for (int p = 0; p < PlayerMax; ++p) {
			if (p) {
				file.printf(", ");
			}
			file.printf("%d", Players[p].Allow.Units[i]);
		}
		file.printf(")\n");
	}
	file.printf("\n");

	//
	//  Save the upgrades
	//
	for (std::vector<CUpgrade *>::size_type j = 0; j < AllUpgrades.size(); ++j) {
		file.printf("DefineAllow(\"%s\", \"", AllUpgrades[j]->Ident.c_str());
		for (int p = 0; p < PlayerMax; ++p) {
			file.printf("%c", Players[p].Allow.Upgrades[j]);
		}
		file.printf("\")\n");
	}
}

/*----------------------------------------------------------------------------
--  Ccl part of upgrades
----------------------------------------------------------------------------*/

/**
**  Define a new upgrade modifier.
**
**  @param l  List of modifiers.
*/
static int CclDefineModifier(lua_State *l)
{
	const int args = lua_gettop(l);

	auto um = std::make_unique<CUpgradeModifier>();

	ranges::fill(um->ChangeUpgrades, '?');
	ranges::fill(um->ApplyTo, '?');
	um->Modifier.Variables.resize(UnitTypeVar.GetNumberVariable());
	um->ModifyPercent.resize(UnitTypeVar.GetNumberVariable());

	std::string_view upgrade_ident = LuaToString(l, 1);
	um->UpgradeId = UpgradeIdByIdent(upgrade_ident);
	if (um->UpgradeId == -1) {
		LuaError(l, "Error when defining upgrade modifier: upgrade \"%s\" doesn't exist.", upgrade_ident.data());
	}

	for (int j = 1; j < args; ++j) {
		if (!lua_istable(l, j + 1)) {
			LuaError(l, "incorrect argument");
		}
		const std::string_view key = LuaToString(l, j + 1, 1);
#if 0 // To be removed. must modify lua file.
		if (key == "attack-range") {
			key = "AttackRange";
		} else if (key == "sight-range") {
			key = "SightRange";
		} else if (key == "basic-damage") {
			key = "BasicDamage";
		} else if (key == "piercing-damage") {
			key = "PiercingDamage";
		} else if (key == "armor") {
			key = "Armor";
		} else if (key == "hit-points") {
			key = "HitPoints";
		}
#endif
		if (key == "regeneration-rate") {
			um->Modifier.Variables[HP_INDEX].Increase = LuaToNumber(l, j + 1, 2);
		} else if (key == "regeneration-frequency") {
			um->Modifier.Variables[HP_INDEX].IncreaseFrequency = LuaToNumber(l, j + 1, 2);
		} else if (key == "cost") {
			if (!lua_istable(l, j + 1) || lua_rawlen(l, j + 1) != 2) {
				LuaError(l, "incorrect argument");
			}
			const std::string_view value = LuaToString(l, j + 1, 1);
			const int resId = GetResourceIdByName(l, value);
			um->Modifier.Costs[resId] = LuaToNumber(l, j + 1, 2);
		} else if (key == "storing") {
			if (!lua_istable(l, j + 1) || lua_rawlen(l, j + 1) != 2) {
				LuaError(l, "incorrect argument");
			}
			const std::string_view value = LuaToString(l, j + 1, 1);
			const int resId = GetResourceIdByName(l, value);
			um->Modifier.Storing[resId] = LuaToNumber(l, j + 1, 2);
		} else if (key == "improve-production") {
			const std::string_view value = LuaToString(l, j + 1, 2);
			const int resId = GetResourceIdByName(l, value);
			um->Modifier.ImproveIncomes[resId] = LuaToNumber(l, j + 1, 3);
		} else if (key == "allow-unit") {
			const std::string_view value = LuaToString(l, j + 1, 2);

			if (starts_with(value, "unit-")) {
				um->ChangeUnits[UnitTypeByIdent(value).Slot] = LuaToNumber(l, j + 1, 3);
			} else {
				LuaError(l, "unit expected");
			}
		} else if (key == "allow") {
			const std::string_view value = LuaToString(l, j + 1, 2);
			if (starts_with(value, "upgrade-")) {
				um->ChangeUpgrades[UpgradeIdByIdent(value)] = LuaToNumber(l, j + 1, 3);
			} else {
				LuaError(l, "upgrade expected");
			}
		} else if (key == "apply-to") {
			const std::string_view value = LuaToString(l, j + 1, 2);
			um->ApplyTo[UnitTypeByIdent(value).Slot] = 'X';
		} else if (key == "convert-to") {
			const std::string_view value = LuaToString(l, j + 1, 2);
			um->ConvertTo = &UnitTypeByIdent(value);
		} else if (key == "research-speed") {
			um->SpeedResearch = LuaToNumber(l, j + 1, 2);
		} else {
			int index = UnitTypeVar.VariableNameLookup[key]; // variable index;
			if (index != -1) {
				if (lua_rawlen(l, j + 1) == 3) {
					const std::string_view value = LuaToString(l, j + 1, 3);
					if (value == "Percent") {
						um->ModifyPercent[index] = LuaToNumber(l, j + 1, 2);
					}
				} else {
					lua_rawgeti(l, j + 1, 2);
					if (lua_istable(l, -1)) {
						DefineVariableField(l, &um->Modifier.Variables[index], -1);
					} else if (lua_isnumber(l, -1)) {
						um->Modifier.Variables[index].Enable = 1;
						um->Modifier.Variables[index].Value = LuaToNumber(l, -1);
						um->Modifier.Variables[index].Max = LuaToNumber(l, -1);
					} else {
						LuaError(l, "bad argument type for '%s'\n", key.data());
					}
					lua_pop(l, 1);
				}
			} else {
				LuaError(l, "wrong tag: %s", key.data());
			}
		}
	}

	UpgradeModifiers[NumUpgradeModifiers++] = std::move(um);

	return 0;
}

/**
**  Define which units are allowed and how much.
*/
static int CclDefineUnitAllow(lua_State *l)
{
	const int args = lua_gettop(l);

	const std::string_view ident = LuaToString(l, 0 + 1);

	if (!starts_with(ident, "unit-")) {
		LuaDebugPrint(l, "wrong ident %s\n", ident.data());
		return 0;
	}
	int id = UnitTypeByIdent(ident).Slot;

	int i = 0;
	for (int j = 1; j < args && i < PlayerMax; ++j) {
		AllowUnitId(Players[i], id, LuaToNumber(l, j + 1));
		++i;
	}
	return 0;
}

/**
** <b>Description</b>
**
**  Define which units/upgrades are allowed.
**
** Example:
**
** <div class="example"><code><strong>DefineAllow</strong>("unit-town-hall","AAAAAAAAAAAAAAAA") -- Available for everybody
**		<strong>DefineAllow</strong>("unit-stables","FFFFFFFFFFFFFFFF") -- Not available
**		<strong>DefineAllow</strong>("upgrade-sword1","RRRRRRRRRRRRRRRR") -- Upgrade already researched.</code></div>
**
*/
static int CclDefineAllow(lua_State *l)
{
	const int UnitMax = 65536; /// How many units supported
	const int args = lua_gettop(l);

	for (int j = 0; j < args; ++j) {
		const std::string_view ident = LuaToString(l, j + 1);
		++j;
		const std::string_view ids = LuaToString(l, j + 1);

		int n = ids.size();
		if (n > PlayerMax) {
			LuaError(l, "'%s': Allow string too long %d\n", ident.data(), n);
			n = PlayerMax;
		}

		if (starts_with(ident, "unit-")) {
			int id = UnitTypeByIdent(ident).Slot;
			for (int i = 0; i < n; ++i) {
				if (ids[i] == 'A') {
					AllowUnitId(Players[i], id, UnitMax);
				} else if (ids[i] == 'F') {
					AllowUnitId(Players[i], id, 0);
				}
			}
		} else if (starts_with(ident, "upgrade-")) {
			int id = UpgradeIdByIdent(ident);
			for (int i = 0; i < n; ++i) {
				AllowUpgradeId(Players[i], id, ids[i]);
			}
		} else {
			LuaError(l, " wrong ident %s\n", ident.data());
		}
	}
	return 0;
}

/**
**  Register CCL features for upgrades.
*/
void UpgradesCclRegister()
{
	lua_register(Lua, "DefineModifier", CclDefineModifier);
	lua_register(Lua, "DefineAllow", CclDefineAllow);
	lua_register(Lua, "DefineUnitAllow", CclDefineUnitAllow);
}

/*----------------------------------------------------------------------------
-- General/Map functions
----------------------------------------------------------------------------*/

// AllowStruct and UpgradeTimers will be static in the player so will be
// load/saved with the player struct

/**
**  Upgrade ID by identifier.
**
**  @param ident  The upgrade identifier.
**  @return       Upgrade ID (int) or -1 if not found.
*/
int UpgradeIdByIdent(std::string_view ident)
{
	const CUpgrade *upgrade = CUpgrade::Get(ident);

	if (upgrade) {
		return upgrade->ID;
	}
	DebugPrint(" fix this %s\n", ident.data());
	return -1;
}

/*----------------------------------------------------------------------------
-- Upgrades
----------------------------------------------------------------------------*/

/**
**  Convert unit-type to.
**
**  @param player  For this player.
**  @param src     From this unit-type.
**  @param dst     To this unit-type.
*/
static void ConvertUnitTypeTo(CPlayer &player, const CUnitType &src, CUnitType &dst)
{
	for (CUnit *unit : player.GetUnits()) {
		//  Convert already existing units to this type.
		if (unit->Type == &src) {
			CommandTransformIntoType(*unit, dst);
			//  Convert trained units to this type.
			//  FIXME: what about buildings?
		} else {
			for (auto &order : unit->Orders) {
				if (order->Action == UnitAction::Train) {
					COrder_Train &order_train = static_cast<COrder_Train &>(*order);

					if (&order_train.GetUnitType() == &src) {
						order_train.ConvertUnitType(*unit, dst);
					}
				}
			}
		}
	}
}

/**
**  Apply the modifiers of an upgrade.
**
**  This function will mark upgrade done and do all required modifications
**  to unit types and will modify allow/forbid maps
**
**  @param player  Player that get all the upgrades.
**  @param um      Upgrade modifier that do the effects
*/
static void ApplyUpgradeModifier(CPlayer &player, const CUpgradeModifier &um)
{
	int pn = player.Index;

	for (int z = 0; z < UpgradeMax; ++z) {
		// allow/forbid upgrades for player.  only if upgrade is not acquired

		// FIXME: check if modify is allowed

		if (player.Allow.Upgrades[z] != 'R') {
			if (um.ChangeUpgrades[z] == 'A') {
				player.Allow.Upgrades[z] = 'A';
			}
			if (um.ChangeUpgrades[z] == 'F') {
				player.Allow.Upgrades[z] = 'F';
			}
			// we can even have upgrade acquired w/o costs
			if (um.ChangeUpgrades[z] == 'R') {
				player.Allow.Upgrades[z] = 'R';
			}
		}
	}

	for (size_t z = 0; z < getUnitTypes().size(); ++z) {
		CUnitStats &stat = getUnitTypes()[z]->Stats[pn];
		// add/remove allowed units

		// FIXME: check if modify is allowed

		player.Allow.Units[z] += um.ChangeUnits[z];

		Assert(um.ApplyTo[z] == '?' || um.ApplyTo[z] == 'X');

		// this modifier should be applied to unittype id == z
		if (um.ApplyTo[z] == 'X') {

			// If Sight range is upgraded, we need to change EVERY unit
			// to the new range, otherwise the counters get confused.
			if (um.Modifier.Variables[SIGHTRANGE_INDEX].Value) {
				std::vector<CUnit *> unitupgrade = FindUnitsByType(*getUnitTypes()[z]);

				for (CUnit *unit : unitupgrade) {
					if (unit->Player->Index == pn && !unit->Removed) {
						MapUnmarkUnitSight(*unit);
						unit->CurrentSightRange = stat.Variables[SIGHTRANGE_INDEX].Max
						                        + um.Modifier.Variables[SIGHTRANGE_INDEX].Value;
						MapMarkUnitSight(*unit);
					}
				}
			}

			// if a unit type's supply is changed, we need to update the player's supply accordingly
			if (um.Modifier.Variables[SUPPLY_INDEX].Value) {
				std::vector<CUnit *> unitupgrade = FindUnitsByType(*getUnitTypes()[z]);

				for (CUnit *unit : unitupgrade) {
					if (unit->Player->Index == pn && unit->IsAlive()) {
						unit->Player->Supply += um.Modifier.Variables[SUPPLY_INDEX].Value;
					}
				}
			}

			// if a unit type's demand is changed, we need to update the player's demand accordingly
			if (um.Modifier.Variables[DEMAND_INDEX].Value) {
				std::vector<CUnit *> unitupgrade = FindUnitsByType(*getUnitTypes()[z]);

				for (CUnit *unit : unitupgrade) {
					if (unit->Player->Index == pn && unit->IsAlive()) {
						unit->Player->Demand += um.Modifier.Variables[DEMAND_INDEX].Value;
					}
				}
			}

			// upgrade costs :)
			for (unsigned int j = 0; j < MaxCosts; ++j) {
				stat.Costs[j] += um.Modifier.Costs[j];
				stat.Storing[j] += um.Modifier.Storing[j];
				if (um.Modifier.ImproveIncomes[j]) {
					if (!stat.ImproveIncomes[j]) {
						stat.ImproveIncomes[j] += DefaultIncomes[j] + um.Modifier.ImproveIncomes[j];
					} else {
						stat.ImproveIncomes[j] += um.Modifier.ImproveIncomes[j];
					}
					//update player's income
					std::vector<CUnit *> unitupgrade = FindUnitsByType(*getUnitTypes()[z]);
					if (unitupgrade.size() > 0) {
						player.Incomes[j] = std::max(player.Incomes[j], stat.ImproveIncomes[j]);
					}
				}
			}

			bool varModified = false;
			for (unsigned int j = 0; j < UnitTypeVar.GetNumberVariable(); j++) {
				varModified |= (um.Modifier.Variables[j].Value != 0)
							   | (um.Modifier.Variables[j].Max != 0)
							   | (um.Modifier.Variables[j].Increase != 0)
							   | (um.Modifier.Variables[j].IncreaseFrequency != 0)
							   | um.Modifier.Variables[j].Enable
							   | (um.ModifyPercent[j] != 0);
				stat.Variables[j].Enable |= um.Modifier.Variables[j].Enable;
				if (um.ModifyPercent[j]) {
					stat.Variables[j].Value += stat.Variables[j].Value * um.ModifyPercent[j] / 100;
					stat.Variables[j].Max += stat.Variables[j].Max * um.ModifyPercent[j] / 100;
				} else {
					stat.Variables[j].Value += um.Modifier.Variables[j].Value;
					stat.Variables[j].Max += um.Modifier.Variables[j].Max;
					stat.Variables[j].Increase += um.Modifier.Variables[j].Increase;
					stat.Variables[j].IncreaseFrequency += um.Modifier.Variables[j].IncreaseFrequency;
				}

				stat.Variables[j].Max = std::max(stat.Variables[j].Max, 0);
				clamp(&stat.Variables[j].Value, 0, stat.Variables[j].Max);
			}

			// And now modify ingame units
			if (varModified) {
				std::vector<CUnit *> unitupgrade = FindUnitsByType(*getUnitTypes()[z], true);

				for (CUnit *unitPtr : unitupgrade) {
					CUnit &unit = *unitPtr;

					if (unit.Player->Index != player.Index) {
						continue;
					}
					for (unsigned int j = 0; j < UnitTypeVar.GetNumberVariable(); j++) {
						unit.Variable[j].Enable |= um.Modifier.Variables[j].Enable;
						if (um.ModifyPercent[j]) {
							unit.Variable[j].Value += unit.Variable[j].Value * um.ModifyPercent[j] / 100;
							unit.Variable[j].Max += unit.Variable[j].Max * um.ModifyPercent[j] / 100;
						} else {
							unit.Variable[j].Value += um.Modifier.Variables[j].Value;
							unit.Variable[j].Increase += um.Modifier.Variables[j].Increase;
							unit.Variable[j].IncreaseFrequency += um.Modifier.Variables[j].IncreaseFrequency;
						}

						unit.Variable[j].Max += um.Modifier.Variables[j].Max;
						unit.Variable[j].Max = std::max(unit.Variable[j].Max, 0);
						if (unit.Variable[j].Max > 0) {
							clamp(&unit.Variable[j].Value, 0, unit.Variable[j].Max);
						}
					}
				}
			}
			if (um.ConvertTo) {
				ConvertUnitTypeTo(player, *getUnitTypes()[z], *um.ConvertTo);
			}
		}
	}
}

/**
**  Remove the modifiers of an upgrade.
**
**  This function will unmark upgrade as done and undo all required modifications
**  to unit types and will modify allow/forbid maps back
**
**  @param player  Player that get all the upgrades.
**  @param um      Upgrade modifier that do the effects
*/
static void RemoveUpgradeModifier(CPlayer &player, const CUpgradeModifier &um)
{
	int pn = player.Index;

	if (um.SpeedResearch != 0) {
		player.SpeedResearch -= um.SpeedResearch;
	}

	for (int z = 0; z < UpgradeMax; ++z) {
		// allow/forbid upgrades for player.  only if upgrade is not acquired

		// FIXME: check if modify is allowed

		if (player.Allow.Upgrades[z] != 'R') {
			if (um.ChangeUpgrades[z] == 'A') {
				player.Allow.Upgrades[z] = 'F';
			}
			if (um.ChangeUpgrades[z] == 'F') {
				player.Allow.Upgrades[z] = 'A';
			}
			// we can even have upgrade acquired w/o costs
			if (um.ChangeUpgrades[z] == 'R') {
				player.Allow.Upgrades[z] = 'A';
			}
		}
	}

	for (size_t z = 0; z < getUnitTypes().size(); ++z) {
		CUnitStats &stat = getUnitTypes()[z]->Stats[pn];
		// add/remove allowed units

		// FIXME: check if modify is allowed

		player.Allow.Units[z] -= um.ChangeUnits[z];

		Assert(um.ApplyTo[z] == '?' || um.ApplyTo[z] == 'X');

		// this modifier should be applied to unittype id == z
		if (um.ApplyTo[z] == 'X') {

			// If Sight range is upgraded, we need to change EVERY unit
			// to the new range, otherwise the counters get confused.
			if (um.Modifier.Variables[SIGHTRANGE_INDEX].Value) {
				std::vector<CUnit *> unitupgrade = FindUnitsByType(*getUnitTypes()[z]);

				for (CUnit *unit : unitupgrade) {
					if (unit->Player->Index == pn && !unit->Removed) {
						MapUnmarkUnitSight(*unit);
						unit->CurrentSightRange = stat.Variables[SIGHTRANGE_INDEX].Max
						                        - um.Modifier.Variables[SIGHTRANGE_INDEX].Value;
						MapMarkUnitSight(*unit);
					}
				}
			}

			// if a unit type's supply is changed, we need to update the player's supply accordingly
			if (um.Modifier.Variables[SUPPLY_INDEX].Value) {
				std::vector<CUnit *> unitupgrade = FindUnitsByType(*getUnitTypes()[z]);

				for (CUnit *unit : unitupgrade) {
					if (unit->Player->Index == pn && unit->IsAlive()) {
						unit->Player->Supply -= um.Modifier.Variables[SUPPLY_INDEX].Value;
					}
				}
			}

			// if a unit type's demand is changed, we need to update the player's demand accordingly
			if (um.Modifier.Variables[DEMAND_INDEX].Value) {
				std::vector<CUnit *> unitupgrade = FindUnitsByType(*getUnitTypes()[z]);

				for (CUnit *unit : unitupgrade) {
					if (unit->Player->Index == pn && unit->IsAlive()) {
						unit->Player->Demand -= um.Modifier.Variables[DEMAND_INDEX].Value;
					}
				}
			}

			// upgrade costs :)
			for (unsigned int j = 0; j < MaxCosts; ++j) {
				stat.Costs[j] -= um.Modifier.Costs[j];
				stat.Storing[j] -= um.Modifier.Storing[j];
				stat.ImproveIncomes[j] -= um.Modifier.ImproveIncomes[j];
				//if this was the highest improve income, search for another
				if (player.Incomes[j] && (stat.ImproveIncomes[j] + um.Modifier.ImproveIncomes[j]) == player.Incomes[j]) {
					int m = DefaultIncomes[j];

					for (const CUnit* unit : player.GetUnits()) {
						m = std::max(m, unit->Type->Stats[player.Index].ImproveIncomes[j]);
					}
					player.Incomes[j] = m;
				}
			}

			bool varModified = false;
			for (unsigned int j = 0; j < UnitTypeVar.GetNumberVariable(); j++) {
				varModified |= (um.Modifier.Variables[j].Value != 0)
					| (um.Modifier.Variables[j].Max != 0)
					| (um.Modifier.Variables[j].Increase != 0)
					| um.Modifier.Variables[j].Enable
					| (um.ModifyPercent[j] != 0);
				stat.Variables[j].Enable |= um.Modifier.Variables[j].Enable;
				if (um.ModifyPercent[j]) {
					stat.Variables[j].Value = stat.Variables[j].Value * 100 / (100 + um.ModifyPercent[j]);
					stat.Variables[j].Max = stat.Variables[j].Max * 100 / (100 + um.ModifyPercent[j]);
				} else {
					stat.Variables[j].Value -= um.Modifier.Variables[j].Value;
					stat.Variables[j].Max -= um.Modifier.Variables[j].Max;
					stat.Variables[j].Increase -= um.Modifier.Variables[j].Increase;
				}

				stat.Variables[j].Max = std::max(stat.Variables[j].Max, 0);
				clamp(&stat.Variables[j].Value, 0, stat.Variables[j].Max);
			}

			// And now modify ingame units
			if (varModified) {
				std::vector<CUnit *> unitupgrade = FindUnitsByType(*getUnitTypes()[z], true);

				for (CUnit *unitPtr : unitupgrade) {
					CUnit &unit = *unitPtr;

					if (unit.Player->Index != player.Index) {
						continue;
					}
					for (unsigned int j = 0; j < UnitTypeVar.GetNumberVariable(); j++) {
						unit.Variable[j].Enable |= um.Modifier.Variables[j].Enable;
						if (um.ModifyPercent[j]) {
							unit.Variable[j].Value = unit.Variable[j].Value * 100 / (100 + um.ModifyPercent[j]);
							unit.Variable[j].Max = unit.Variable[j].Max * 100 / (100 + um.ModifyPercent[j]);
						} else {
							unit.Variable[j].Value -= um.Modifier.Variables[j].Value;
							unit.Variable[j].Increase -= um.Modifier.Variables[j].Increase;
						}

						unit.Variable[j].Max -= um.Modifier.Variables[j].Max;
						unit.Variable[j].Max = std::max(unit.Variable[j].Max, 0);

						clamp(&unit.Variable[j].Value, 0, unit.Variable[j].Max);
					}
				}
			}
			if (um.ConvertTo) {
				ConvertUnitTypeTo(player, *um.ConvertTo, *getUnitTypes()[z]);
			}
		}
	}
}

/**
**  Apply the modifiers of an individual upgrade.
**
**  @param unit    Unit that will get the modifier applied
**  @param um      Upgrade modifier that does the effects
*/
void ApplyIndividualUpgradeModifier(CUnit &unit, const CUpgradeModifier &um)
{
	if (um.Modifier.Variables[SIGHTRANGE_INDEX].Value) {
		if (!unit.Removed) {
			MapUnmarkUnitSight(unit);
			unit.CurrentSightRange = unit.Variable[SIGHTRANGE_INDEX].Value
			                       + um.Modifier.Variables[SIGHTRANGE_INDEX].Value;
			UpdateUnitSightRange(unit);
			MapMarkUnitSight(unit);
		}
	}

	for (unsigned int j = 0; j < UnitTypeVar.GetNumberVariable(); j++) {
		unit.Variable[j].Enable |= um.Modifier.Variables[j].Enable;
		if (um.ModifyPercent[j]) {
			unit.Variable[j].Value += unit.Variable[j].Value * um.ModifyPercent[j] / 100;
			unit.Variable[j].Max += unit.Variable[j].Max * um.ModifyPercent[j] / 100;
		} else {
			unit.Variable[j].Value += um.Modifier.Variables[j].Value;
			unit.Variable[j].Increase += um.Modifier.Variables[j].Increase;
			unit.Variable[j].IncreaseFrequency += um.Modifier.Variables[j].IncreaseFrequency;
		}
		unit.Variable[j].Max += um.Modifier.Variables[j].Max;
		unit.Variable[j].Max = std::max(unit.Variable[j].Max, 0);
		if (unit.Variable[j].Max > 0) {
			clamp(&unit.Variable[j].Value, 0, unit.Variable[j].Max);
		}
	}
	if (um.ConvertTo) {
		CommandTransformIntoType(unit, *um.ConvertTo);
	}
}

static void RemoveIndividualUpgradeModifier(CUnit &unit, const CUpgradeModifier &um)
{
	if (um.Modifier.Variables[SIGHTRANGE_INDEX].Value) {
		if (!unit.Removed) {
			MapUnmarkUnitSight(unit);
			unit.CurrentSightRange = unit.Variable[SIGHTRANGE_INDEX].Value
			                       - um.Modifier.Variables[SIGHTRANGE_INDEX].Value;
			UpdateUnitSightRange(unit);
			MapMarkUnitSight(unit);
		}
	}

	for (unsigned int j = 0; j < UnitTypeVar.GetNumberVariable(); j++) {
		unit.Variable[j].Enable |= um.Modifier.Variables[j].Enable;
		if (um.ModifyPercent[j]) {
			unit.Variable[j].Value = unit.Variable[j].Value * 100 / (100 + um.ModifyPercent[j]);
			unit.Variable[j].Max = unit.Variable[j].Max * 100 / (100 + um.ModifyPercent[j]);
		} else {
			unit.Variable[j].Value -= um.Modifier.Variables[j].Value;
			unit.Variable[j].Increase -= um.Modifier.Variables[j].Increase;
		}
		unit.Variable[j].Max -= um.Modifier.Variables[j].Max;
		unit.Variable[j].Max = std::max(unit.Variable[j].Max, 0);
		if (unit.Variable[j].Max > 0) {
			clamp(&unit.Variable[j].Value, 0, unit.Variable[j].Max);
		}
	}
}

/**
**  Handle that an upgrade was acquired.
**
**  @param player   Player researching the upgrade.
**  @param upgrade  Upgrade ready researched.
*/
void UpgradeAcquire(CPlayer &player, const CUpgrade *upgrade)
{
	int id = upgrade->ID;
	player.UpgradeTimers.Upgrades[id] = upgrade->Costs[TimeCost];
	AllowUpgradeId(player, id, 'R');  // research done

	for (int z = 0; z < NumUpgradeModifiers; ++z) {
		if (UpgradeModifiers[z]->UpgradeId == id) {
			ApplyUpgradeModifier(player, *UpgradeModifiers[z]);
		}
	}

	//
	//  Upgrades could change the buttons displayed.
	//
	if (&player == ThisPlayer) {
		SelectedUnitChanged();
	}
}

/**
**  Upgrade will be lost
**
**  @param player   Player researching the upgrade.
**  @param id       Upgrade to be lost.
**
*/
void UpgradeLost(CPlayer &player, int id)
{
	player.UpgradeTimers.Upgrades[id] = 0;

	for (int z = 0; z < NumUpgradeModifiers; ++z) {
		if (UpgradeModifiers[z]->UpgradeId == id) {
			RemoveUpgradeModifier(player, *UpgradeModifiers[z]);
		}
	}

	//
	//  Upgrades could change the buttons displayed.
	//
	if (&player == ThisPlayer) {
		SelectedUnitChanged();
	}
}

/**
**  Apply researched upgrades when map is loading
**
**  @return:   void
*/
void ApplyUpgrades()
{
	for (std::vector<CUpgrade *>::size_type j = 0; j < AllUpgrades.size(); ++j) {
		auto &upgrade = AllUpgrades[j];
		if (upgrade) {
			for (int p = 0; p < PlayerMax; ++p) {
				if (Players[p].Allow.Upgrades[j] == 'R') {
					int id = upgrade->ID;
					Players[p].UpgradeTimers.Upgrades[id] = upgrade->Costs[TimeCost];
					AllowUpgradeId(Players[p], id, 'R');  // research done

					for (int z = 0; z < NumUpgradeModifiers; ++z) {
						if (UpgradeModifiers[z]->UpgradeId == id) {
							ApplyUpgradeModifier(Players[p], *UpgradeModifiers[z]);
						}
					}
				}
			}
		}
	}
}

void IndividualUpgradeAcquire(CUnit &unit, const CUpgrade *upgrade)
{
	int id = upgrade->ID;
	unit.Player->UpgradeTimers.Upgrades[id] = upgrade->Costs[TimeCost];
	unit.IndividualUpgrades[id] = true;

	for (int z = 0; z < NumUpgradeModifiers; ++z) {
		if (UpgradeModifiers[z]->UpgradeId == id) {
			ApplyIndividualUpgradeModifier(unit, *UpgradeModifiers[z]);
		}
	}

	//
	//  Upgrades could change the buttons displayed.
	//
	if (unit.Player == ThisPlayer) {
		SelectedUnitChanged();
	}
}

void IndividualUpgradeLost(CUnit &unit, const CUpgrade *upgrade)
{
	int id = upgrade->ID;
	unit.Player->UpgradeTimers.Upgrades[id] = 0;
	unit.IndividualUpgrades[id] = false;

	for (int z = 0; z < NumUpgradeModifiers; ++z) {
		if (UpgradeModifiers[z]->UpgradeId == id) {
			RemoveIndividualUpgradeModifier(unit, *UpgradeModifiers[z]);
		}
	}

	//
	//  Upgrades could change the buttons displayed.
	//
	if (unit.Player == ThisPlayer) {
		SelectedUnitChanged();
	}
}

/*----------------------------------------------------------------------------
--  Allow(s)
----------------------------------------------------------------------------*/

// all the following functions are just map handlers, no specific notes

/**
**  Change allow for an unit-type.
**
**  @param player  Player to change
**  @param id      unit type id
**  @param units   maximum amount of units allowed
*/
static void AllowUnitId(CPlayer &player, int id, int units)
{
	player.Allow.Units[id] = units;
}

/**
**  Change allow for an upgrade.
**
**  @param player  Player to change
**  @param id      upgrade id
**  @param af      `A'llow/`F'orbid/`R'eseached
*/
void AllowUpgradeId(CPlayer &player, int id, char af)
{
	Assert(af == 'A' || af == 'F' || af == 'R');
	player.Allow.Upgrades[id] = af;
}

/**
**  Return the allow state of the unit.
**
**  @param player   Check state of this player.
**  @param id       Unit identifier.
**
**  @return the allow state of the unit.
*/
int UnitIdAllowed(const CPlayer &player, int id)
{
	Assert(id >= 0 && id < UnitTypeMax);
	return player.Allow.Units[id];
}

/**
**  Return the allow state of an upgrade.
**
**  @param player  Check state for this player.
**  @param id      Upgrade identifier.
**
**  @return the allow state of the upgrade.
*/
char UpgradeIdAllowed(const CPlayer &player, int id)
{
	Assert(id >= 0 && id < UpgradeMax);
	return player.Allow.Upgrades[id];
}

// ***************by string identifiers

/**
**  Return the allow state of an upgrade.
**
**  @param player  Check state for this player.
**  @param ident   Upgrade identifier.
**
**  @note This function shouldn't be used during runtime, it is only for setup.
*/
char UpgradeIdentAllowed(const CPlayer &player, std::string_view ident)
{
	int id = UpgradeIdByIdent(ident);

	if (id != -1) {
		return UpgradeIdAllowed(player, id);
	}
	DebugPrint("Fix your code, wrong identifier '%s'\n", ident.data());
	return '-';
}

//@}
