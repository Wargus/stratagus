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
/**@name upgrade.h - The upgrades headerfile. */
//
//      (c) Copyright 1999-2006 by Vladi Belperchinov-Shabanski and Jimmy Salmon
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

#ifndef __UPGRADE_H__
#define __UPGRADE_H__

//@{

/*----------------------------------------------------------------------------
--  Declarations
----------------------------------------------------------------------------*/

class CFile;
class CPlayer;
class CUpgrade;

/*----------------------------------------------------------------------------
--  Variables
----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
--  Functions
----------------------------------------------------------------------------*/

/// init upgrade/allow structures
extern void InitUpgrades();
/// save the upgrades
extern void SaveUpgrades(CFile &file);
/// cleanup upgrade module
extern void CleanUpgrades();

/// Register CCL features for upgrades
extern void UpgradesCclRegister();

/*----------------------------------------------------------------------------
--  General/Map functions
----------------------------------------------------------------------------*/

// AllowStruct and UpgradeTimers will be static in the player so will be
// load/saved with the player struct

extern int UnitTypeIdByIdent(const std::string &sid);
extern int UpgradeIdByIdent(const std::string &sid);

/*----------------------------------------------------------------------------
--  Upgrades
----------------------------------------------------------------------------*/

/// Upgrade will be acquired
extern void UpgradeAcquire(CPlayer &player, const CUpgrade *upgrade);

/// for now it will be empty?
/// perhaps acquired upgrade can be lost if (for example) a building is lost
/// (lumber mill? stronghold?)
/// this function will apply all modifiers in reverse way
extern void UpgradeLost(CPlayer &player, int id);

/*----------------------------------------------------------------------------
--  Allow(s)
----------------------------------------------------------------------------*/

// all the following functions are just map handlers, no specific notes
// id -- unit type id, af -- `A'llow/`F'orbid

extern int UnitIdAllowed(const CPlayer &player, int id);

extern char UpgradeIdAllowed(const CPlayer &player, int id);
extern char UpgradeIdentAllowed(const CPlayer &player, const std::string &ident);

//@}

#endif  // !__UPGRADE_H__
