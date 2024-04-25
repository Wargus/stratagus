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
/**@name upgrade_structs.h - The upgrade/allow headerfile. */
//
//      (c) Copyright 1999-2015 by Vladi Belperchinov-Shabanski,
//		Jimmy Salmon and Andrettin
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

#ifndef __UPGRADE_STRUCTS_H__
#define __UPGRADE_STRUCTS_H__

//@{

/*----------------------------------------------------------------------------
--  Includes
----------------------------------------------------------------------------*/

#include <memory>
#include <vector>

/*----------------------------------------------------------------------------
--  Defines
----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
--  Declarations
----------------------------------------------------------------------------*/

class CUnitType;
class CVariable;
class CIcon;
struct lua_State;

/**
**  Indices into costs/resource/income array.
*/
enum CostType {
	TimeCost,                               /// time in game cycles

	// standard
	GoldCost,                               /// gold  resource
	WoodCost,                               /// wood  resource
	OilCost,                                /// oil   resource
	// extensions
	Cost4,                                  /// resource 4
	Cost5,                                  /// resource 5
	Cost6,                                  /// resource 6

	MaxCosts                                /// how many different costs
};

#define FoodCost MaxCosts
#define ScoreCost (MaxCosts + 1)
#define ManaResCost (MaxCosts + 2)
#define FreeWorkersCount (MaxCosts + 3)

/**
**  Default incomes for a new player.
*/
extern int DefaultIncomes[MaxCosts];

/**
**  Default action for the resources.
*/
extern std::string DefaultActions[MaxCosts];

/**
**  Default names for the resources.
*/
extern std::string DefaultResourceNames[MaxCosts];

/**
**  Default amounts for the resources.
*/
extern int DefaultResourceAmounts[MaxCosts];

/**
**  Default max amounts for the resources.
*/
extern int DefaultResourceMaxAmounts[MaxCosts];

extern int GetResourceIdByName(std::string_view resourceName);
extern int GetResourceIdByName(lua_State *l, std::string_view resourceName);

/**
**  These are the current stats of a unit. Upgraded or downgraded.
*/
class CUnitStats
{
public:
	CUnitStats() = default;
	~CUnitStats() = default;

	CUnitStats &operator=(const CUnitStats &) = default;

	bool operator == (const CUnitStats &rhs) const;
	bool operator != (const CUnitStats &rhs) const;
public:
	std::vector<CVariable> Variables; /// user defined variable.
	int Costs[MaxCosts]{};          /// current costs of the unit
	int Storing[MaxCosts]{};        /// storage increasing
	int ImproveIncomes[MaxCosts]{}; /// Gives player an improved income
};

/**
**  The main useable upgrades.
*/
class CUpgrade
{
public:
	explicit CUpgrade(std::string ident);

	static CUpgrade *New(std::string ident);
	static CUpgrade *Get(std::string_view ident);

	std::string Ident;                /// identifier
	std::string Name;                 /// upgrade label
	int ID = 0;            /// numerical id
	int Costs[MaxCosts]{}; /// costs for the upgrade
	// TODO: not used by buttons
	CIcon *Icon = nullptr; /// icon to display to the user
};

/*----------------------------------------------------------------------------
--  upgrades and modifiers
----------------------------------------------------------------------------*/

/**
**  This is the modifier of an upgrade.
**  This do the real action of an upgrade, an upgrade can have multiple
**  modifiers.
*/
class CUpgradeModifier
{
public:
	CUpgradeModifier() = default;

	int UpgradeId = 0;            /// used to filter required modifier

	CUnitStats Modifier;          /// modifier of unit stats.
	std::vector<int> ModifyPercent; /// use for percent modifiers
	int SpeedResearch = 0;          /// speed factor for researching
	int ImproveIncomes[MaxCosts]{}; /// improve incomes

	// allow/forbid bitmaps -- used as chars for example:
	// `?' -- leave as is, `F' -- forbid, `A' -- allow
	// TODO: see below allow more semantics?
	// TODO: pointers or ids would be faster and less memory use
	int ChangeUnits[UnitTypeMax]{};    /// add/remove allowed units
	char ChangeUpgrades[UpgradeMax]{}; /// allow/forbid upgrades
	char ApplyTo[UnitTypeMax]{};       /// which unit types are affected

	CUnitType *ConvertTo = nullptr;    /// convert to this unit-type.
};

/**
**  Allow what a player can do. Every #CPlayer has an own allow struct.
**
**  This could allow/disallow units, actions or upgrades.
**
**  Values are:
**    @li `A' -- allowed,
**    @li `F' -- forbidden,
**    @li `R' -- acquired, perhaps other values
**    @li `Q' -- acquired but forbidden (does it make sense?:))
**    @li `E' -- enabled, allowed by level but currently forbidden
**    @li `X' -- fixed, acquired can't be disabled
*/
class CAllow
{
public:
	CAllow() = default;

	void Clear()
	{
		memset(Units, 0, sizeof(Units));
		memset(Upgrades, 0, sizeof(Upgrades));
	}

	int Units[UnitTypeMax]{}; /// maximum amount of units allowed
	char Upgrades[UpgradeMax]{}; /// upgrades allowed/disallowed
};

/**
**  Upgrade timer used in the player structure.
**  Every player has an own UpgradeTimers struct.
*/
class CUpgradeTimers
{
public:
	CUpgradeTimers() = default;

	void Clear() { memset(Upgrades, 0, sizeof(Upgrades)); }

	/**
	**  all 0 at the beginning, all upgrade actions do increment values in
	**  this struct.
	*/
	int Upgrades[UpgradeMax]{}; /// counter for each upgrade
};

/*----------------------------------------------------------------------------
--  Variables
----------------------------------------------------------------------------*/

extern std::vector<std::unique_ptr<CUpgrade>> AllUpgrades;  /// the main user usable upgrades

//@}

#endif // !__UPGRADE_STRUCTS_H__
