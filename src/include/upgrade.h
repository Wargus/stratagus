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
//      (c) Copyright 1999-2005 by Vladi Belperchinov-Shabanski and Jimmy Salmon
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

#ifndef __UPGRADE_H__
#define __UPGRADE_H__

//@{

/*----------------------------------------------------------------------------
--  Includes
----------------------------------------------------------------------------*/

#include "upgrade_structs.h"

/*----------------------------------------------------------------------------
--  Declarations
----------------------------------------------------------------------------*/

class CLFile;

/*----------------------------------------------------------------------------
--  Variables
----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
--  Functions
----------------------------------------------------------------------------*/

	 /// upgrade of identifier
extern Upgrade* UpgradeByIdent(const char* ident);

	/// init upgrade/allow structures
extern void InitUpgrades(void);
	/// save the upgrades
extern void SaveUpgrades(CLFile *);
	/// cleanup upgrade module
extern void CleanUpgrades(void);

	/// Register CCL features for upgrades
extern void UpgradesCclRegister(void);

/*----------------------------------------------------------------------------
--  General/Map functions
----------------------------------------------------------------------------*/

// AllowStruct and UpgradeTimers will be static in the player so will be
// load/saved with the player struct

extern int UnitTypeIdByIdent(const char* sid);
extern int UpgradeIdByIdent(const char* sid);

/*----------------------------------------------------------------------------
--  Upgrades
----------------------------------------------------------------------------*/

	/// Upgrade will be acquired
extern void UpgradeAcquire(Player* player, const Upgrade* upgrade);

/// for now it will be empty?
/// perhaps acquired upgrade can be lost if (for example) a building is lost
/// (lumber mill? stronghold?)
/// this function will apply all modifiers in reverse way
extern void UpgradeLost(Player* player, int id);

/*----------------------------------------------------------------------------
--  Allow(s)
----------------------------------------------------------------------------*/

// all the following functions are just map handlers, no specific notes
// id -- unit type id, af -- `A'llow/`F'orbid

extern int UnitIdAllowed(const Player* player, int id);

extern char UpgradeIdAllowed(const Player* player, int id);
extern char UpgradeIdentAllowed(const Player* player, const char* sid);

	/// Check if the upgrade is researched.
extern int UpgradeIdentAvailable(const Player* player, const char* ident);

//@}

#endif  // !__UPGRADE_H__
