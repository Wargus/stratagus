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
/**@name pud.h - The pud headerfile. */
//
//      (c) Copyright 1998-2004 by Lutz Sammer
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

#ifndef __PUD_H__
#define __PUD_H__

//@{

/*----------------------------------------------------------------------------
--  Includes
----------------------------------------------------------------------------*/

#include "map.h"

/*----------------------------------------------------------------------------
--  Defines
----------------------------------------------------------------------------*/

//
//  Used for reading puds.
//
#define MapMoveOnlyLand 0x01  ///< only land units
#define MapMoveCoast    0x02  ///< only water transporters
#define MapMoveHuman    0x04  ///< marks human wall on map
#define MapMoveWallO    0x08  ///< orcish wall on map
// 1101
#define MapMoveWallH    0x0D  ///< human wall on map
#define MapMoveDirt     0x10  ///< no buildings allowed
// 20
#define MapMoveOnlyWater  0x40  ///< only water units
#define MapMoveUnpassable 0x80  ///< blocked

#define MapMoveForestOrRock 0x81  ///< forest or rock on map
#define MapMoveCoast2       0x82  ///< unknown
#define MapMoveWall         0x8D  ///< wall on map

#define MapMoveLandUnit     0x100  ///< land unit
#define MapMoveAirUnit      0x200  ///< air unit
#define MapMoveSeaUnit      0x400  ///< water unit
#define MapMoveBuildingUnit 0x800  ///< building unit
#define MapMoveBlocked      0xF00  ///< no unit allowed

/// @note LOW BITS: Area connected!
#define MapActionWater   0x0000  ///< water on map
#define MapActionLand    0x4000  ///< land on map
#define MapActionIsland  0xFFFA  ///< no trans, no land on map
#define MapActionWall    0xFFFB  ///< wall on map
#define MapActionRocks   0xFFFD  ///< rocks on map
#define MapActionForest  0xFFFE  ///< forest on map

/// @note These are hard-coded PUD internals (and, as such, belong here!)
/// @todo for our own maps this should become configurable
#define WC_UnitPeasant        0x02  ///< human peasant
#define WC_UnitPeon           0x03  ///< orc peon
#define WC_UnitGoldMine       0x5C  ///< goldmine
#define WC_UnitOilPatch       0x5D  ///< oilpatch
#define WC_StartLocationHuman 0x5E  ///< start location human
#define WC_StartLocationOrc   0x5F  ///< start location orc

/*----------------------------------------------------------------------------
--  Declarations
----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
--  Functions
----------------------------------------------------------------------------*/

	/// Return info for pud
extern MapInfo* GetPudInfo(const char*);

	/// Load a pud file
extern void LoadPud(const char* pud,WorldMap* map);

	/// Save a pud file
extern int SavePud(const char* pud,const WorldMap* map);
	/// Change a pud's tileset
extern void ChangeTilesetPud(int old,WorldMap* map);

	/// Clean the pud module
extern void CleanPud(void);

//@}

#endif // !__PUD_H__
