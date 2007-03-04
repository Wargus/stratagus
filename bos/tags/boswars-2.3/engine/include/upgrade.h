//     ____                _       __               
//    / __ )____  _____   | |     / /___ ___________
//   / __  / __ \/ ___/   | | /| / / __ `/ ___/ ___/
//  / /_/ / /_/ (__  )    | |/ |/ / /_/ / /  (__  ) 
// /_____/\____/____/     |__/|__/\__,_/_/  /____/  
//                                              
//       A futuristic real-time strategy game.
//          This file is part of Bos Wars.
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
//      $Id$

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
extern void InitUpgrades(void);
	/// save the upgrades
extern void SaveUpgrades(CFile *file);
	/// cleanup upgrade module
extern void CleanUpgrades(void);

	/// Register CCL features for upgrades
extern void UpgradesCclRegister(void);

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
extern void UpgradeAcquire(CPlayer *player, const CUpgrade *upgrade);

/// for now it will be empty?
/// perhaps acquired upgrade can be lost if (for example) a building is lost
/// (lumber mill? stronghold?)
/// this function will apply all modifiers in reverse way
extern void UpgradeLost(CPlayer *player, int id);

/*----------------------------------------------------------------------------
--  Allow(s)
----------------------------------------------------------------------------*/

// all the following functions are just map handlers, no specific notes
// id -- unit type id, af -- `A'llow/`F'orbid

extern int UnitIdAllowed(const CPlayer *player, int id);

extern char UpgradeIdAllowed(const CPlayer *player, int id);
extern char UpgradeIdentAllowed(const CPlayer *player, const std::string &ident);

	/// Check if the upgrade is researched.
extern int UpgradeIdentAvailable(const CPlayer *player, const std::string &ident);

//@}

#endif  // !__UPGRADE_H__
