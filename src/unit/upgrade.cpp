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
/**@name upgrade.c - The upgrade/allow functions. */
//
//      (c) Copyright 1999-2004 by Vladi Belperchinov-Shabanski and Jimmy Salmon
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
//      $Id$

//@{

/*----------------------------------------------------------------------------
--  Includes
----------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "stratagus.h"
#include "upgrade.h"
#include "player.h"
#include "depend.h"
#include "interface.h"
#include "map.h"
#include "script.h"
#include "spells.h"

#include "myendian.h"

#include "util.h"

local int AddUpgradeModifierBase(int, int, int, int, int, int, int, int, int,
	int*, const int[UnitTypeMax], const char*, const char*, UnitType*);
local int AddUpgradeModifier(int, int, int, int, int, int, int, int,
	int*, const int[UnitTypeMax], const char*, const char*);

local void AllowUnitId(Player* player, int id, int units);
local void AllowUpgradeId(Player* player, int id, char af);

/*----------------------------------------------------------------------------
--  Variables
----------------------------------------------------------------------------*/

/**
**  upgrade type definition
*/
global const char UpgradeType[] = "upgrade";

global Upgrade Upgrades[UpgradeMax];        /// The main user useable upgrades
local int NumUpgrades;                      /// Number of upgrades used

	/// How many upgrades modifiers supported
#define UPGRADE_MODIFIERS_MAX		(UpgradeMax * 4)
	/// Upgrades modifiers
local UpgradeModifier* UpgradeModifiers[UPGRADE_MODIFIERS_MAX];
	/// Number of upgrades modifiers used
local int NumUpgradeModifiers;

#ifdef DOXYGEN  // no real code, only for documentation
local Upgrade* UpgradeHash[61];             /// lookup table for upgrade names
#else
local hashtable(Upgrade*, 61) UpgradeHash;  /// lookup table for upgrade names
#endif

/**
**  Mapping of W*rCr*ft number to our internal upgrade symbol.
**  The numbers are used in puds.
*/
local char** UpgradeWcNames;

/*----------------------------------------------------------------------------
--  Functions
----------------------------------------------------------------------------*/

/**
**  Add an upgrade. If the ident didn't exist a new upgrade is created,
**  if the ident is alreay used, the old value is overwritten.
**
**  @param ident  upgrade identifier.
**  @param icon   icon displayed for this upgrade,
**                NULL for generated name (icon-<em>ident</em>).
**  @param costs  costs to upgrade.
**
**  @return       upgrade id or -1 for error
*/
local Upgrade* AddUpgrade(const char* ident, const char* icon,
	const int* costs)
{
	Upgrade* upgrade;
	Upgrade** tmp;
	int i;

	// Check for free slot.

	if (NumUpgrades == UpgradeMax) {
		DebugLevel0Fn("Upgrades limit reached.\n");
		return NULL;
	}
	// Fill upgrade structure

	if ((tmp = (Upgrade**)hash_find(UpgradeHash, (char*)ident)) && *tmp) {
		DebugLevel0Fn("Already defined upgrade `%s'\n" _C_ ident);
		upgrade = *tmp;
		free(upgrade->Icon.Name);
	} else {
		upgrade = Upgrades + NumUpgrades++;
		upgrade->OType = UpgradeType;
		upgrade->Ident = strdup(ident);
		*(Upgrade**)hash_add(UpgradeHash, upgrade->Ident) = upgrade;
	}

	if (icon) {
		upgrade->Icon.Name = strdup(icon);
	} else {  // automatically generated icon-name
		upgrade->Icon.Name = malloc(strlen(ident) + 5 - 8 + 1);
		strcpy(upgrade->Icon.Name, "icon-");
		strcpy(upgrade->Icon.Name + 5, ident + 8);
	}

	for (i = 0; i < MaxCosts; ++i) {
		upgrade->Costs[i] = costs[i];
	}

	return upgrade;
}

/**
**  Upgrade by identifier.
**
**  @param ident  The upgrade identifier.
**  @return       Upgrade pointer or NULL if not found.
*/
global Upgrade* UpgradeByIdent(const char* ident)
{
	Upgrade** upgrade;

	if ((upgrade = (Upgrade**)hash_find(UpgradeHash, (char*)ident))) {
		return *upgrade;
	}

	DebugLevel0Fn(" upgrade %s not found\n" _C_ ident);

	return NULL;
}

/**
**  Find upgrade by wc number.
**
**  @param num  The upgrade number used in f.e. puds.
**  @return     Upgrade pointer.
*/
local Upgrade* UpgradeByWcNum(unsigned num)
{
	return UpgradeByIdent(UpgradeWcNames[num]);
}


/**
**  Init upgrade/allow structures
*/
global void InitUpgrades(void)
{
	int i;

	//
	//  Resolve the icons.
	//
	for (i = 0; i < NumUpgrades; ++i) {
		Upgrades[i].Icon.Icon = IconByIdent(Upgrades[i].Icon.Name);
	}
}

/**
**  Cleanup the upgrade module.
*/
global void CleanUpgrades(void)
{
	int i;
	char** cp;

	//
	//  Free the upgrades.
	//
	for (i = 0; i < NumUpgrades; ++i) {
		hash_del(UpgradeHash, Upgrades[i].Ident);
		free(Upgrades[i].Ident);
		free(Upgrades[i].Icon.Name);
	}
	NumUpgrades = 0;

	//
	//  Free the upgrade modifiers.
	//
	for (i = 0; i < NumUpgradeModifiers; ++i) {
		free(UpgradeModifiers[i]);
	}
	NumUpgradeModifiers = 0;

	//
	//  Free mapping of original upgrade numbers in puds to our internal strings
	//
	if ((cp = UpgradeWcNames)) {
		while (*cp) {
			free(*cp++);
		}
		free(UpgradeWcNames);
		UpgradeWcNames = NULL;
	}
}

/**
**		Parse ALOW area from puds.
**
**		@param alow		Pointer to alow area.
**		@param length		length of alow area.
**
**		@note		Only included for compatibility, for new levels use
**				CCL (define-allow)
*/
global void ParsePudALOW(const char* alow, int length __attribute__((unused)))
{
	// units allow bits -> wc2num -> internal names.
	static char unit_for_bit[64] = {
		 0,  1,		// unit-footman						unit-grunt
		 2,  3,		// unit-peasant						unit-peon
		 4,  5,		// unit-ballista				unit-catapult
		 6,  7,		// unit-knight						unit-ogre
		 8,  9,		// unit-archer						unit-axethrower
		10, 11,		// unit-mage						unit-death-knight
		26, 27,		// unit-human-oil-tanker		unit-orc-oil-tanker
		30, 31,		// unit-elven-destroyer				unit-troll-destroyer
		28, 29,		// unit-human-transport				unit-orc-transport
		32, 33,		// unit-battleship				unit-ogre-juggernaught
		38, 39,		// unit-gnomish-submarine		unit-giant-turtle
		40, 41,		// unit-gnomish-flying-machine		unit-goblin-zeppelin
		42, 43,		// unit-gryphon-rider				unit-dragon
		-1, -1,		// unused
		14, 15,		// unit-dwarves						unit-goblin-sappers
		70, 71,		// unit-gryphon-aviary				unit-dragon-roost
		58, 59,		// unit-farm						unit-pig-farm
		60, 61,		// unit-human-barracks				unit-orc-barracks
		76, 77,		// unit-elven-lumber-mill		unit-troll-lumber-mill
		66, 67,		// unit-stables						unit-ogre-mound
		80, 81,		// unit-mage-tower				unit-temple-of-the-damned
		78, 79,		// unit-human-foundry				unit-orc-foundry
		84, 85,		// unit-human-refinery				unit-orc-refinery
		68, 69,		// unit-gnomish-inventor		unit-goblin-alchemist
		62, 63,		// unit-church						unit-altar-of-storms
		64, 65,		// unit-human-watch-tower		unit-orc-watch-tower
		74, 75,		// unit-town-hall				unit-great-hall
		88, 89,		// unit-keep						unit-stronghold
		90, 91,		// unit-castle						unit-fortress
		82, 83,		// unit-human-blacksmith		unit-orc-blacksmith
		72, 73,		// unit-human-shipyard				unit-orc-shipyard
		103,104,// unit-human-wall				unit-orc-wall
	};
	// spells allow bits -> wc2num -> internal names.
	static char spell_for_bit[32] = {
		34,		// upgrade-holy-vision
		35,		// upgrade-healing
		-1,		// not used
		36,		// upgrade-exorcism
		37,		// upgrade-flame-shield
		38,		// upgrade-fireball
		39,		// upgrade-slow
		40,		// upgrade-invisibility
		41,		// upgrade-polymorph
		42,		// upgrade-blizzard
		43,		// upgrade-eye-of-kilrogg
		44,		// upgrade-bloodlust
		-1,		// not used
		45,		// upgrade-raise-dead
		46,		// upgrade-death-coil
		47,		// upgrade-whirlwind
		48,		// upgrade-haste
		49,		// upgrade-unholy-armor
		50,		// upgrade-runes
		51,		// upgrade-death-and-decay
		-1,		// not used
		-1,		// not used
		-1,		// not used
		-1,		// not used
		-1,		// not used
		-1,		// not used
		-1,		// not used
		-1,		// not used
		-1,		// not used
		-1,		// not used
		-1,		// not used
		-1,		// not used
	};
	// upgrades allow bits -> wc2num -> internal names.
	static char upgrade_for_bit[64] = {
		 4, 6,		// upgrade-arrow1				upgrade-throwing-axe1
		 5, 7,		// upgrade-arrow2				upgrade-throwing-axe2
		 0, 2,		// upgrade-sword1				upgrade-battle-axe1
		 1, 3,		// upgrade-sword2				upgrade-battle-axe2
		 8,10,		// upgrade-human-shield1		upgrade-orc-shield1
		 9,11,		// upgrade-human-shield2		upgrade-orc-shield2
		12,14,		// upgrade-human-ship-cannon1		upgrade-orc-ship-cannon1
		13,15,		// upgrade-human-ship-cannon2		upgrade-orc-ship-cannon2
		16,18,		// upgrade-human-ship-armor1		upgrade-orc-ship-armor1
		17,19,		// upgrade-human-ship-armor2		upgrade-orc-ship-armor2
		-1,-1,		// unused
		-1,-1,		// unused
		20,22,		// upgrade-catapult1				upgrade-ballista1
		21,23,		// upgrade-catapult2				upgrade-ballista2
		-1,-1,		// unused
		-1,-1,		// unused
		24,28,		// upgrade-ranger				upgrade-berserker
		25,29,		// upgrade-longbow				upgrade-light-axes
		26,30,		// upgrade-ranger-scouting		upgrade-berserker-scouting
		27,31,		// upgrade-ranger-marksmanship		upgrade-berserker-regeneration
		33,32,		// upgrade-paladin				upgrade-ogre-mage
		-1,-1,		// unused
		-1,-1,		// unused
		-1,-1,		// unused
		-1,-1,		// unused
		-1,-1,		// unused
		-1,-1,		// unused
		-1,-1,		// unused
		-1,-1,		// unused
		-1,-1,		// unused
		-1,-1,		// unused
		-1,-1,		// unused
	};
	int i;
	int b;
	Player* player;

	DebugLevel0Fn(" Length %d FIXME: constant must be moved to ccl\n" _C_ length);

	//
	//		Allow units
	//
	for (player = Players; player < Players + 16; ++player) {
		for (i = 0; i < 4; ++i) {				// 4 bytes endian save
			int v;

			v = (*alow++) & 0xFF;
			for (b = 0; b < 8; ++b) {
				if (unit_for_bit[i * 16 + 0 + b * 2] >= 0) {
					if (v & (1 << b)) {
						AllowUnitId(player,
							UnitTypeByWcNum(unit_for_bit[i * 16 + 0 + b * 2])->Slot,
								UnitMax);
						AllowUnitId(player,
							UnitTypeByWcNum(unit_for_bit[i * 16 + 1 + b * 2])->Slot,
								UnitMax);
					} else {
						AllowUnitId(player,
							UnitTypeByWcNum(unit_for_bit[i * 16 + 0 + b * 2])->Slot,
								0);
						AllowUnitId(player,
							UnitTypeByWcNum(unit_for_bit[i * 16 + 1 + b * 2])->Slot,
								0);
					}
				}
			}
		}
	}

	//
	//		Spells start with
	//
	for (player = Players; player < Players + 16; ++player) {
		for (i = 0; i < 4; ++i) {
			int v;

			v = *alow++;
			for (b = 0; b < 8; ++b) {
				if (spell_for_bit[i * 8 + b] >= 0) {
					if (v & (1 << b)) {
						AllowUpgradeId(player,
							UpgradeByWcNum(spell_for_bit[i * 8 + b]) - Upgrades, 'R');
					} else {
						AllowUpgradeId(player,
							UpgradeByWcNum(spell_for_bit[i * 8 + b]) - Upgrades, 'F');
					}
				}
			}
		}
	}

	//
	//		Spells allowed
	//
	for (player = Players; player < Players + 16; ++player) {
		for (i = 0; i < 4; ++i) {
			int v;

			v = *alow++;
			for (b = 0; b < 8; ++b) {
				if (v & (1 << b)) {
					// FIXME: combine with 'R'esearched and 'F'orbidden
					if (spell_for_bit[i * 8 + b] >= 0) {
						AllowUpgradeId(player,
							UpgradeByWcNum(spell_for_bit[i * 8 + b]) - Upgrades, 'A');
					}
				}
			}
		}
	}

	//
	//		Spells researching
	//				FIXME: not useful.
	//
	for (player = Players; player < Players + 16; ++player) {
		for (i = 0; i < 4; ++i) {
			int v;

			v = *alow++;
			for (b = 0; b < 8; ++b) {
				if (v & (1 << b)) {
					// FIXME: combine with 'R'esearched and 'F'orbidden
					if (spell_for_bit[i * 8 + b] >= 0) {
						AllowUpgradeId(player,
							UpgradeByWcNum(spell_for_bit[i * 8 + b]) - Upgrades, 'U');
					}
				}
			}
		}
	}

	//
	//		Upgrades allowed
	//
	for (player = Players; player < Players + 16; ++player) {
		for (i = 0; i < 4; ++i) {
			int v;

			v = *alow++;
			for (b = 0; b < 8; ++b) {
				if (v & (1 << b)) {
					if (upgrade_for_bit[i * 16 + b * 2 + 0] >= 0) {
						AllowUpgradeId(player,
							UpgradeByWcNum(upgrade_for_bit[i * 16 + b * 2 + 0]) - Upgrades,
							'A');
						AllowUpgradeId(player,
							UpgradeByWcNum(upgrade_for_bit[i * 16 + b * 2 + 1])-Upgrades,
							'A');
					}
				}
			}
		}
	}

	//
	//		Upgrades acquired
	//
	for (player = Players; player < Players + 16; ++player) {
		for (i = 0; i < 4; ++i) {
			int v;

			v = *alow++;
			for (b = 0; b < 8; ++b) {
				if (v & (1 << b)) {
					if (upgrade_for_bit[i * 16 + b * 2 + 0] >= 0) {
						AllowUpgradeId(player,
							UpgradeByWcNum(upgrade_for_bit[i * 16 + b * 2 + 0]) - Upgrades,
							'U');
						AllowUpgradeId(player,
							UpgradeByWcNum(upgrade_for_bit[i * 16 + b * 2 + 1]) - Upgrades,
							'U');
					}
				}
			}
		}
	}
}

/**
**		Parse UGRD area from puds.
**
**		@param ugrd		Pointer to ugrd area.
**		@param length		length of ugrd area.
*/
global void ParsePudUGRD(const char* ugrd, int length __attribute__((unused)))
{
	int i;
	int time;
	int gold;
	int lumber;
	int oil;
	int icon;
	int group;
	int flags;
	int costs[MaxCosts];

	DebugLevel3Fn(" Length %d\n" _C_ length);
	Assert(length == 780);

	for (i = 0; i < 52; ++i) {
		time = ((unsigned char*)ugrd)[i];
		gold = AccessLE16(		ugrd + 52 + (i) * 2);
		lumber = AccessLE16(		ugrd + 52 + (i + 52) * 2);
		oil = AccessLE16(		ugrd + 52 + (i + 52 + 52) * 2);
		icon = AccessLE16(		ugrd + 52 + (i + 52 + 52 + 52) * 2);
		group = AccessLE16(		ugrd + 52 + (i + 52 + 52 + 52 + 52) * 2);
		flags = AccessLE16(		ugrd + 52 + (i + 52 + 52 + 52 + 52 + 52) * 2);
		DebugLevel3Fn(" (%d)%s %d,%d,%d,%d (%d)%s %d %08X\n" _C_
			i _C_ UpgradeWcNames[i] _C_ time _C_ gold _C_ lumber _C_ oil _C_
			icon _C_ IconWcNames[icon] _C_ group _C_ flags);

		memset(costs, 0, sizeof(costs));
		costs[TimeCost] = time;
		costs[GoldCost] = gold;
		costs[WoodCost] = lumber;
		costs[OilCost] = oil;
		AddUpgrade(UpgradeWcNames[i], IconWcNames[icon], costs);

		// group+flags are to mystic to be implemented
	}
}

/**
**  Save state of the dependencies to file.
**
**  @param file  Output file.
*/
global void SaveUpgrades(CLFile* file)
{
	int i;
	int p;

	CLprintf(file, "\n-- -----------------------------------------\n");
	CLprintf(file, "-- MODULE: upgrades $Id$\n\n");

	//
	//  Save the allow
	//
	for (i = 0; i < NumUnitTypes; ++i) {
		CLprintf(file, "DefineUnitAllow(\"%s\", ", UnitTypes[i]->Ident);
		for (p = 0; p < PlayerMax; ++p) {
			if (p) {
				CLprintf(file, ", ");
			}
			CLprintf(file, "%d", Players[p].Allow.Units[i]);
		}
		CLprintf(file, ")\n");
	}
	CLprintf(file, "\n");

	//
	//  Save the upgrades
	//
	for (i = 0; i < NumUpgrades; ++i) {
		CLprintf(file, "DefineAllow(\"%s\", \"", Upgrades[i].Ident);
		for (p = 0; p < PlayerMax; ++p) {
			CLprintf(file, "%c", Players[p].Allow.Upgrades[i]);
		}
		CLprintf(file, "\")\n");
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
local int CclDefineModifier(lua_State* l)
{
	const char* temp;
	const char* value;
	int uid;
	int attack_range;
	int sight_range;
	int basic_damage;
	int piercing_damage;
	int armor;
	int speed;
	int regeneration_rate;
	int hit_points;
	int costs[MaxCosts];
	int units[UnitTypeMax];
	char upgrades[UpgradeMax];
	char apply_to[UnitTypeMax];
	UnitType* convert_to;
	int args;
	int j;

	args = lua_gettop(l);
	j = 0;

	attack_range = 0;
	sight_range = 0;
	basic_damage = 0;
	piercing_damage = 0;
	armor = 0;
	speed = 0;
	hit_points = 0;
	regeneration_rate = 0;
	memset(costs, 0, sizeof(costs));
	memset(units, 0, sizeof(units));
	memset(upgrades, '?', sizeof(upgrades));
	memset(apply_to, '?', sizeof(apply_to));
	convert_to = NULL;

	value = LuaToString(l, j + 1);
	uid = UpgradeIdByIdent(value);
	++j;

	for (; j < args; ++j) {
		if (!lua_istable(l, j + 1)) {
			LuaError(l, "incorrect argument");
		}
		lua_rawgeti(l, j + 1, 1);
		temp = LuaToString(l, -1);
		lua_pop(l, 1);
		if (!strcmp(temp, "attack-range")) {
			lua_rawgeti(l, j + 1, 2);
			attack_range = LuaToNumber(l, -1);
			lua_pop(l, 1);
		} else if (!strcmp(temp, "sight-range")) {
			lua_rawgeti(l, j + 1, 2);
			sight_range = LuaToNumber(l, -1);
			lua_pop(l, 1);
		} else if (!strcmp(temp, "basic-damage")) {
			lua_rawgeti(l, j + 1, 2);
			basic_damage = LuaToNumber(l, -1);
			lua_pop(l, 1);
		} else if (!strcmp(temp, "piercing-damage")) {
			lua_rawgeti(l, j + 1, 2);
			piercing_damage = LuaToNumber(l, -1);
			lua_pop(l, 1);
		} else if (!strcmp(temp, "armor")) {
			lua_rawgeti(l, j + 1, 2);
			armor = LuaToNumber(l, -1);
			lua_pop(l, 1);
		} else if (!strcmp(temp, "speed")) {
			lua_rawgeti(l, j + 1, 2);
			speed = LuaToNumber(l, -1);
			lua_pop(l, 1);
		} else if (!strcmp(temp, "hit-points")) {
			lua_rawgeti(l, j + 1, 2);
			hit_points = LuaToNumber(l, -1);
			lua_pop(l, 1);
		} else if (!strcmp(temp, "regeneration-rate")) {
			lua_rawgeti(l, j + 1, 2);
			regeneration_rate = LuaToNumber(l, -1);
			lua_pop(l, 1);
		} else if (!strcmp(temp, "cost")) {
			int i;

			if (!lua_istable(l, j + 1) || luaL_getn(l, j + 1) != 2) {
				LuaError(l, "incorrect argument");
			}
			lua_rawgeti(l, j + 1, 1);
			value = LuaToString(l, -1);
			lua_pop(l, 1);
			for (i = 0; i < MaxCosts; ++i) {
				if (!strcmp(value, DefaultResourceNames[i])) {
					break;
				}
			}
			if (i == MaxCosts) {
				LuaError(l, "Resource not found: %s" _C_ value);
			}
			lua_rawgeti(l, j + 1, 2);
			costs[i] = LuaToNumber(l, -1);
			lua_pop(l, 1);
		} else if (!strcmp(temp, "allow-unit")) {
			lua_rawgeti(l, j + 1, 2);
			value = LuaToString(l, -1);
			lua_pop(l, 1);
			DebugLevel3Fn("%s\n" _C_ value);
			if (!strncmp(value, "unit-", 5)) {
				lua_rawgeti(l, j + 1, 3);
				units[UnitTypeIdByIdent(value)] = LuaToNumber(l, -1);
				lua_pop(l, 1);
			} else {
				LuaError(l, "unit expected");
			}
		} else if (!strcmp(temp, "allow")) {
			lua_rawgeti(l, j + 1, 2);
			value = LuaToString(l, -1);
			lua_pop(l, 1);
			DebugLevel3Fn("%s\n" _C_ value);
			if (!strncmp(value, "upgrade-", 8)) {
				lua_rawgeti(l, j + 1, 3);
				upgrades[UpgradeIdByIdent(value)] = LuaToNumber(l, -1);
				lua_pop(l, 1);
			} else {
				LuaError(l, "upgrade expected");
			}
		} else if (!strcmp(temp, "apply-to")) {
			lua_rawgeti(l, j + 1, 2);
			value = LuaToString(l, -1);
			lua_pop(l, 1);
			apply_to[UnitTypeIdByIdent(value)] = 'X';
		} else if (!strcmp(temp, "convert-to")) {
			lua_rawgeti(l, j + 1, 2);
			value = LuaToString(l, -1);
			lua_pop(l, 1);
			convert_to = UnitTypeByIdent(value);
		} else {
			LuaError(l, "wrong tag: %s" _C_ temp);
		}
	}

	AddUpgradeModifierBase(uid, attack_range, sight_range, basic_damage,
		piercing_damage, armor, speed, hit_points, regeneration_rate, costs,
		units, upgrades, apply_to,convert_to);

	return 0;
}

/**
**  Define a new upgrade.
**
**  @param l  List defining the upgrade.
*/
local int CclDefineUpgrade(lua_State* l)
{
	const char* value;
	const char* icon;
	const char* ident;
	int costs[MaxCosts];
	int n;
	int j;
	int args;
	int k;

	args = lua_gettop(l);
	k = 0;

	// Identifier
	ident = LuaToString(l, k + 1);
	++k;

	icon = NULL;
	memset(costs, 0, sizeof(costs));

	for (; k < args; ++k) {
		value = LuaToString(l, k + 1);
		++k;
		if (!strcmp(value, "icon")) {
			// Icon
			icon = LuaToString(l, k + 1);
		} else if (!strcmp(value, "costs")) {
			// Costs
			if (!lua_istable(l, k + 1)) {
				LuaError(l, "incorrect argument");
			}
			n = luaL_getn(l, k + 1);
			if (n > MaxCosts) {
				LuaError(l, "%s: Wrong vector length" _C_ ident);
			}
			for (j = 0; j < n; ++j) {
				lua_rawgeti(l, k + 1, j + 1);
				costs[j] = LuaToNumber(l, -1);
				lua_pop(l, 1);
			}
			while (j < MaxCosts) {
				costs[j++] = 0;
			}
		} else {
			LuaError(l, "%s: Wrong tag `%s'" _C_ ident _C_ value);
		}
	}

	AddUpgrade(ident, icon, costs);

	return 0;
}

/**
**  Define which units are allowed and how much.
*/
local int CclDefineUnitAllow(lua_State* l)
{
	const char* ident;
	int i;
	int args;
	int j;
	int id;

	args = lua_gettop(l);
	j = 0;
	ident = LuaToString(l, j + 1);
	++j;

	if (strncmp(ident, "unit-", 5)) {
		DebugLevel0Fn(" wrong ident %s\n" _C_ ident);
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
local int CclDefineAllow(lua_State* l)
{
	const char* ident;
	const char* ids;
	int i;
	int n;
	int args;
	int j;
	int id;

	args = lua_gettop(l);
	for (j = 0; j < args; ++j) {
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
			for (i = 0; i < n; ++i) {
				if (ids[i] == 'A') {
					AllowUnitId(&Players[i], id, UnitMax);
				} else if (ids[i] == 'F') {
					AllowUnitId(&Players[i], id, 0);
				}
			}
		} else if (!strncmp(ident, "upgrade-", 8)) {
			id = UpgradeIdByIdent(ident);
			for (i = 0; i < n; ++i) {
				AllowUpgradeId(&Players[i], id, ids[i]);
			}
		} else {
			DebugLevel0Fn(" wrong ident %s\n" _C_ ident);
		}
	}

	return 0;
}

/**
**  Define upgrade mapping from original number to internal symbol
**
**  @param l  List of all names.
*/
local int CclDefineUpgradeWcNames(lua_State* l)
{
	int i;
	int j;
	char** cp;

	if ((cp = UpgradeWcNames)) {  // Free all old names
		while (*cp) {
			free(*cp++);
		}
		free(UpgradeWcNames);
	}

	//
	//  Get new table.
	//
	i = lua_gettop(l);
	UpgradeWcNames = cp = malloc((i + 1) * sizeof(char*));
	if (!cp) {
		fprintf(stderr, "out of memory.\n");
		ExitFatal(-1);
	}

	for (j = 0; j < i; ++j) {
		*cp++ = strdup(LuaToString(l, j + 1));
	}
	*cp = NULL;

	return 0;
}

/**
**  Register CCL features for upgrades.
*/
global void UpgradesCclRegister(void)
{
	lua_register(Lua, "DefineModifier", CclDefineModifier);
	lua_register(Lua, "DefineUpgrade", CclDefineUpgrade);
	lua_register(Lua, "DefineAllow", CclDefineAllow);
	lua_register(Lua, "DefineUnitAllow", CclDefineUnitAllow);

	lua_register(Lua, "DefineUpgradeWcNames", CclDefineUpgradeWcNames);
}


/*----------------------------------------------------------------------------
--  Init/Done/Add functions
----------------------------------------------------------------------------*/

/**
**  Add a upgrade modifier.
**
**  @param uid                Upgrade identifier of the modifier.
**  @param attack_range       Attack range modification.
**  @param sight_range        Sight range modification.
**  @param basic_damage       Basic damage modification.
**  @param piercing_damage    Piercing damage modification.
**  @param armor              Armor modification.
**  @param speed              Speed modification (Currently not possible).
**  @param hit_points         Hitpoint modification.
**  @param regeneration_rate  Regenerations modification.
**  @param costs              Costs modification.
**  @param units              Changes in allowed units.
**  @param af_upgrades        Changes in allow upgrades.
**  @param apply_to           Applies to this units.
**  @param convert_to         Converts units to this unit-type.
**
**  @return                 upgrade modifier id or -1 for error
**                          (actually this id is useless, just error checking)
*/
local int AddUpgradeModifierBase(int uid, int attack_range, int sight_range,
	int basic_damage, int piercing_damage, int armor, int speed,
	int hit_points, int regeneration_rate, int* costs,
	const int units[UnitTypeMax],
	const char* af_upgrades, const char* apply_to, UnitType* convert_to)
{
	int i;
	UpgradeModifier* um;

	um = (UpgradeModifier*)malloc(sizeof(UpgradeModifier));
	if (!um) {
		return -1;
	}

	um->UpgradeId = uid;

	// get/save stats modifiers
	um->Modifier.AttackRange      = attack_range;
	um->Modifier.SightRange       = sight_range;
	um->Modifier.BasicDamage      = basic_damage;
	um->Modifier.PiercingDamage   = piercing_damage;
	um->Modifier.Armor            = armor;
	um->Modifier.Speed            = speed;
	um->Modifier.HitPoints        = hit_points;
	um->Modifier.RegenerationRate = regeneration_rate;

	for (i = 0; i < MaxCosts; ++i) {
		um->Modifier.Costs[i] = costs[i];
	}

	memcpy(um->ChangeUnits, units, sizeof(um->ChangeUnits));
	memcpy(um->ChangeUpgrades, af_upgrades, sizeof(um->ChangeUpgrades));
	memcpy(um->ApplyTo, apply_to, sizeof(um->ApplyTo));

	um->ConvertTo = convert_to;

	UpgradeModifiers[NumUpgradeModifiers] = um;

	return NumUpgradeModifiers++;
}

/**
**		returns upgrade modifier id or -1 for error (actually this id is
**		useless, just error checking)
*/
local int AddUpgradeModifier(int uid, int attack_range, int sight_range,
	int basic_damage, int piercing_damage, int armor, int speed,
	int hit_points, int* costs,
	const int units[UnitTypeMax],
	// following are comma separated list of required string id's
	const char* af_upgrades,
	const char* apply_to			// "unit-peon,unit-peasant"
	)
{
	char* s1;
	char* s2;
	int i;
	UpgradeModifier* um;

	um = (UpgradeModifier*)malloc(sizeof(UpgradeModifier));
	if (!um) {
		return -1;
	}

	um->UpgradeId = uid;

	// get/save stats modifiers
	um->Modifier.AttackRange    = attack_range;
	um->Modifier.SightRange     = sight_range;
	um->Modifier.BasicDamage    = basic_damage;
	um->Modifier.PiercingDamage = piercing_damage;
	um->Modifier.Armor          = armor;
	um->Modifier.Speed          = speed;
	um->Modifier.HitPoints      = hit_points;

	for (i = 0; i < MaxCosts; ++i) {
		um->Modifier.Costs[i] = costs[i];
	}

	// FIXME: all the thing below is sensitive to the format of the string!
	// FIXME: it will be good if things are checked for errors better!
	// FIXME: perhaps the function `strtok()' should be replaced with local one?

	memcpy(um->ChangeUnits, units, sizeof(um->ChangeUnits));
	memset(um->ChangeUpgrades, '?', sizeof(um->ChangeUpgrades));
	memset(um->ApplyTo,		'?', sizeof(um->ApplyTo));

	//
	// get allow/forbid's for upgrades
	//
	s1 = strdup(af_upgrades);
	Assert(s1);
	for (s2 = strtok(s1, ","); s2; s2 = strtok(NULL, ",")) {
		int id;
		Assert(s2[0] == 'A' || s2[0] == 'F' || s2[0] == 'R');
		Assert(s2[1] == ':');
		id = UpgradeIdByIdent(s2 + 2);
		if (id == -1) {
			continue;				// should we cancel all and return error?!
		}
		um->ChangeUpgrades[id] = s2[0];
	}
	free(s1);

	//
	// get units that are affected by this upgrade
	//
	s1 = strdup(apply_to);
	Assert(s1);
	for (s2 = strtok(s1, ","); s2; s2 = strtok(NULL, ",")) {
		int id;

		DebugLevel3Fn(" %s\n" _C_ s2);
		id = UnitTypeIdByIdent(s2);
		if (id == -1) {
			break;				// cade: should we cancel all and return error?!
		}
		um->ApplyTo[id] = 'X';		// something other than '?'
	}
	free(s1);

	UpgradeModifiers[NumUpgradeModifiers] = um;
	NumUpgradeModifiers++;

	return NumUpgradeModifiers - 1;
}

/*----------------------------------------------------------------------------
--		General/Map functions
----------------------------------------------------------------------------*/

// AllowStruct and UpgradeTimers will be static in the player so will be
// load/saved with the player struct

/**
**  UnitType ID by identifier.
**
**  @param ident  The unit-type identifier.
**  @return       Unit-type ID (int) or -1 if not found.
*/
global int UnitTypeIdByIdent(const char* ident)
{
	UnitType* type;

	if ((type = UnitTypeByIdent(ident))) {
		return type->Slot;
	}
	DebugLevel0Fn(" fix this %s\n" _C_ ident);
	return -1;
}

/**
**  Upgrade ID by identifier.
**
**  @param ident  The upgrade identifier.
**  @return       Upgrade ID (int) or -1 if not found.
*/
global int UpgradeIdByIdent(const char* ident)
{
	Upgrade* upgrade;

	upgrade = UpgradeByIdent(ident);
	if (upgrade) {
		return upgrade - Upgrades;
	}
	DebugLevel0Fn(" fix this %s\n" _C_ ident);
	return -1;
}

/*----------------------------------------------------------------------------
--		Upgrades
----------------------------------------------------------------------------*/

/**
**  Convert unit-type to.
**
**  @param player  For this player.
**  @param src     From this unit-type.
**  @param dst     To this unit-type.
*/
local void ConvertUnitTypeTo(Player* player, const UnitType* src, UnitType* dst)
{
	Unit* unit;
	int i;
	int j;

	for (i = 0; i < player->TotalNumUnits; ++i) {
		unit = player->Units[i];
		//
		//  Convert already existing units to this type.
		//
		if (unit->Type == src) {
			unit->HP += dst->Stats[player->Player].HitPoints -
				unit->Stats->HitPoints;
			// don't have such unit now
			player->UnitTypesCount[src->Slot]--;
			// UnMark the Unit sight for conversion if on map
			if ((unit->CurrentSightRange != dst->Stats[player->Player].SightRange ||
					src->TileWidth != dst->TileWidth ||
					src->TileHeight != dst->TileHeight) && !unit->Removed) {
				MapUnmarkUnitSight(unit);
			}
			unit->Type = dst;
			unit->Stats = &dst->Stats[player->Player];
			// and we have new one...

			UpdateForNewUnit(unit, 1);
			if (dst->CanCastSpell) {
				unit->Mana = MAGIC_FOR_NEW_UNITS;
				unit->AutoCastSpell = malloc(SpellTypeCount);
				memset(unit->AutoCastSpell, 0, SpellTypeCount);
			}
			if ((unit->CurrentSightRange != dst->Stats[player->Player].SightRange ||
					src->TileWidth != dst->TileWidth ||
					src->TileHeight != dst->TileHeight) && !unit->Removed) {
				unit->CurrentSightRange = dst->Stats[player->Player].SightRange;
				MapMarkUnitSight(unit);
			}
			player->UnitTypesCount[dst->Slot]++;
		//
		//  Convert trained units to this type.
		//  FIXME: what about buildings?
		//
		} else {
			if (unit->Orders[0].Action == UnitActionTrain) {
				for (j = 0; j < unit->Data.Train.Count; ++j) {
					if (unit->Data.Train.What[j] == src) {
						unit->Data.Train.What[j] = dst;
					}
				}
			}
			for (j = 1; j < unit->OrderCount; ++j) {
				if (unit->Orders[j].Action == UnitActionTrain &&
						unit->Orders[j].Type == src) {
					unit->Orders[j].Type = dst;
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
local void ApplyUpgradeModifier(Player* player, const UpgradeModifier* um)
{
	int z;
	int j;
	int pn;

	pn = player->Player;  // player number
	for (z = 0; z < UpgradeMax; ++z) {
		// allow/forbid upgrades for player.  only if upgrade is not acquired

		// FIXME: check if modify is allowed

		if (player->Allow.Upgrades[z] != 'R') {
			if (um->ChangeUpgrades[z] == 'A') {
				player->Allow.Upgrades[z] = 'A';
			}
			if (um->ChangeUpgrades[z] == 'F') {
				player->Allow.Upgrades[z] = 'F';
			}
			// we can even have upgrade acquired w/o costs
			if (um->ChangeUpgrades[z] == 'R') {
				player->Allow.Upgrades[z] = 'R';
			}
		}
	}

	for (z = 0; z < UnitTypeMax; ++z) {
		// add/remove allowed units

		// FIXME: check if modify is allowed

		player->Allow.Units[z] += um->ChangeUnits[z];

		Assert(um->ApplyTo[z] == '?' || um->ApplyTo[z] == 'X');

		// this modifier should be applied to unittype id == z
		if (um->ApplyTo[z] == 'X') {

			DebugLevel3Fn(" applied to %d\n" _C_ z);
			// upgrade stats
			UnitTypes[z]->Stats[pn].AttackRange += um->Modifier.AttackRange;
			UnitTypes[z]->Stats[pn].SightRange += um->Modifier.SightRange;
			// If Sight range is upgraded, we need to change EVERY unit
			// to the new range, otherwise the counters get confused.
			if (um->Modifier.SightRange) {
				int numunits;
				Unit* sightupgrade[UnitMax];

				numunits = FindUnitsByType(UnitTypes[z], sightupgrade);
				numunits--; // Change to 0 Start not 1 start
				while (numunits >= 0) {
					if (sightupgrade[numunits]->Player->Player == player->Player &&
								!sightupgrade[numunits]->Removed) {
						MapUnmarkUnitSight(sightupgrade[numunits]);
						sightupgrade[numunits]->CurrentSightRange =
							UnitTypes[z]->Stats[pn].SightRange;
						MapMarkUnitSight(sightupgrade[numunits]);
					}
					--numunits;
				}
			}
			UnitTypes[z]->Stats[pn].BasicDamage += um->Modifier.BasicDamage;
			UnitTypes[z]->Stats[pn].PiercingDamage += um->Modifier.PiercingDamage;
			UnitTypes[z]->Stats[pn].Armor += um->Modifier.Armor;
			UnitTypes[z]->Stats[pn].Speed += um->Modifier.Speed;
			UnitTypes[z]->Stats[pn].HitPoints += um->Modifier.HitPoints;
			UnitTypes[z]->Stats[pn].RegenerationRate += um->Modifier.RegenerationRate;

			// upgrade costs :)
			for (j = 0; j < MaxCosts; ++j) {
				UnitTypes[z]->Stats[pn].Costs[j] += um->Modifier.Costs[j];
			}

			UnitTypes[z]->Stats[pn].Level++;

			if (um->ConvertTo) {
				((UnitType*)um->ConvertTo)->Stats[pn].Level++;
				ConvertUnitTypeTo(player,UnitTypes[z], um->ConvertTo);
			}
		}
	}
}

/**
**  Handle that an upgrade was acquired.
**
**  @param player   Player researching the upgrade.
**  @param upgrade  Upgrade ready researched.
*/
global void UpgradeAcquire(Player* player, const Upgrade* upgrade)
{
	int z;
	int id;

	id = upgrade-Upgrades;
	player->UpgradeTimers.Upgrades[id] = upgrade->Costs[TimeCost];
	AllowUpgradeId(player, id, 'R');  // research done

	for (z = 0; z < NumUpgradeModifiers; ++z) {
		if (UpgradeModifiers[z]->UpgradeId == id) {
			ApplyUpgradeModifier(player, UpgradeModifiers[z]);
		}
	}

	//
	//  Upgrades could change the buttons displayed.
	//
	if (player == ThisPlayer) {
		SelectedUnitChanged();
	}
}

/**
**  for now it will be empty?
**  perhaps acquired upgrade can be lost if (for example) a building is lost
**  (lumber mill? stronghold?)
**  this function will apply all modifiers in reverse way
*/
global void UpgradeLost(Player* player, int id)
{
	return; // FIXME: remove this if implemented below

	player->UpgradeTimers.Upgrades[id] = 0;
	AllowUpgradeId(player, id, 'A'); // research is lost i.e. available
	// FIXME: here we should reverse apply upgrade...
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
local void AllowUnitId(Player* player, int id, int units)
{
	player->Allow.Units[id] = units;
}

/**
**  Change allow for an upgrade.
**
**  @param player  Player to change
**  @param id      upgrade id
**  @param af      `A'llow/`F'orbid/`R'eseached
*/
local void AllowUpgradeId(Player* player, int id, char af)
{
	Assert(af == 'A' || af == 'F' || af == 'R');
	player->Allow.Upgrades[id] = af;
}

/**
**  FIXME: docu
*/
global int UnitIdAllowed(const Player* player, int id)
{
	// JOHNS: Don't be kind, the people should code correct!
	Assert(id >= 0 && id < UnitTypeMax);
	if (id < 0 || id >= UnitTypeMax) {
		return 0;
	}
	return player->Allow.Units[id];
}

/**
**  FIXME: docu
*/
global char UpgradeIdAllowed(const Player* player, int id)
{
	// JOHNS: Don't be kind, the people should code correct!
	Assert(id >= 0 && id < UpgradeMax);
	return player->Allow.Upgrades[id];
}

// ***************by string identifiers's

/**
**  Return the allow state of an upgrade.
**
**  @param player  Check state for this player.
**  @param ident   Upgrade identifier.
**
**  @note This function shouldn't be used during runtime, it is only for setup.
*/
global char UpgradeIdentAllowed(const Player* player, const char* ident)
{
	int id;

	if ((id = UpgradeIdByIdent(ident)) != -1) {
		return UpgradeIdAllowed(player, id);
	}
	DebugLevel0Fn("Fix your code, wrong idenifier `%s'\n" _C_ ident);
	return '-';
}

/*----------------------------------------------------------------------------
--  Check availablity
----------------------------------------------------------------------------*/

/**
**  Check if upgrade (also spells) available for the player.
**
**  @param player  Player pointer.
**  @param ident   Upgrade ident.
*/
global int UpgradeIdentAvailable(const Player* player, const char* ident)
{
	int allow;

#if 0
	//
	//  Check dependencies
	//
	if (!CheckDependByIdent(player, ident)) {
		return 0;
	}
#endif
	//
	//  Allowed by level
	//
	allow = UpgradeIdentAllowed(player, ident);
	return allow == 'R' || allow == 'X';
}

//@}
