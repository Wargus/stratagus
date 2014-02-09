//     ____                _       __               
//    / __ )____  _____   | |     / /___ ___________
//   / __  / __ \/ ___/   | | /| / / __ `/ ___/ ___/
//  / /_/ / /_/ (__  )    | |/ |/ / /_/ / /  (__  ) 
// /_____/\____/____/     |__/|__/\__,_/_/  /____/  
//                                              
//       A futuristic real-time strategy game.
//          This file is part of Bos Wars.
//
/**@name upgrade_structs.h - The upgrade/allow headerfile. */
//
//      (c) Copyright 1999-2007 by Vladi Belperchinov-Shabanski and
//                                 Jimmy Salmon
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

#ifndef __UPGRADE_STRUCTS_H__
#define __UPGRADE_STRUCTS_H__

//@{

/*----------------------------------------------------------------------------
--  Declarations
----------------------------------------------------------------------------*/

class CUnitType;
class CVariable;
class CIcon;

/**
**  Indices into costs/resource/income array.
*/
enum CostType {
	EnergyCost,                             /// energy resource
	MagmaCost,                              /// magma resource

	MaxCosts                                /// how many different costs
};

/**
**  Default resources for a new player.
*/
extern int DefaultResources[MaxCosts];

/**
**  Default resources for a new player with low resources.
*/
extern int DefaultResourcesLow[MaxCosts];

/**
**  Default resources for a new player with mid resources.
*/
extern int DefaultResourcesMedium[MaxCosts];

/**
**  Default resources for a new player with high resources.
*/
extern int DefaultResourcesHigh[MaxCosts];

/**
**  Default names for the resources.
*/
extern std::string DefaultResourceNames[MaxCosts];

/**
**  Default names for the resources used for display (localized).
*/
extern std::string DefaultDisplayResourceNames[MaxCosts];

/**
**  These are the current stats of a unit. Upgraded or downgraded.
*/
class CUnitStats {
public:
	CUnitStats() : Variables(NULL) {}

	CVariable *Variables;           /// user defined variable.
};

/*----------------------------------------------------------------------------
--  upgrades and modifiers
----------------------------------------------------------------------------*/

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
class CAllow {
public:
	CAllow() { this->Clear(); }

	void Clear() {
		memset(Units, 0, sizeof(Units));
	}

	int Units[UnitTypeMax];        /// maximum amount of units allowed
};

//@}

#endif // !__UPGRADE_STRUCTS_H__
