//   ___________		     _________		      _____  __
//   \_	  _____/______   ____   ____ \_   ___ \____________ _/ ____\/  |_
//    |    __) \_  __ \_/ __ \_/ __ \/    \  \/\_  __ \__  \\   __\\   __\ 
//    |     \   |  | \/\  ___/\  ___/\     \____|  | \// __ \|  |   |  |
//    \___  /   |__|    \___  >\___  >\______  /|__|  (____  /__|   |__|
//	  \/		    \/	   \/	     \/		   \/
//  ______________________                           ______________________
//			  T H E   W A R   B E G I N S
//	   FreeCraft - A free fantasy real time strategy game engine
//
/**@name pud.h		-	The pud headerfile. */
//
//	(c) Copyright 1998-2001 by Lutz Sammer
//
//	FreeCraft is free software; you can redistribute it and/or modify
//	it under the terms of the GNU General Public License as published
//	by the Free Software Foundation; either version 2 of the License,
//	or (at your option) any later version.
//
//	FreeCraft is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//	GNU General Public License for more details.
//
//	$Id$

#ifndef __PUD_H__
#define __PUD_H__

//@{

/*----------------------------------------------------------------------------
--	Includes
----------------------------------------------------------------------------*/

#include "map.h"

/*----------------------------------------------------------------------------
--	Defines
----------------------------------------------------------------------------*/

//
//	Used to read puds.
//
#define MapMoveOnlyLand		0x01	/// only land units
#define MapMoveCoast		0x02	/// only water transporters
#define MapMoveHuman		0x04	/// marks human wall on map
#define MapMoveWallO		0x08	/// orcish wall on map
// 1101
#define MapMoveWallH		0x0D	/// human wall on map
#define MapMoveDirt		0x10	/// no buildings allowed
// 20
#define MapMoveOnlyWater	0x40	/// only water units
#define MapMoveUnpassable	0x80	/// blocked

#define MapMoveForestOrRock	0x81	/// Forest or Rock on map
#define MapMoveCoast2		0x82	/// unknown
#define MapMoveWall		0x8D	/// Wall on map

#define MapMoveLandUnit		0x100	/// Land unit
#define MapMoveAirUnit		0x200	/// Air unit
#define MapMoveSeaUnit		0x400	/// Water unit
#define MapMoveBuildingUnit	0x800	/// Building unit
#define MapMoveBlocked		0xF00	/// No unit allowed

// LOW BITS: Area connected!
#define MapActionWater		0x0000	/// water on map
#define MapActionLand		0x4000	/// land on map
#define MapActionIsland		0xFFFA	/// no trans, no land on map
#define MapActionWall		0xFFFB	/// wall on map
#define MapActionRocks		0xFFFD	/// rocks on map
#define MapActionForest		0xFFFE	/// forest on map

// These are hard-coded PUD internals (and, as such, belong here!)
#define WC_UnitPeasant		0x02	/// Human Peasant
#define WC_UnitPeon		0x03	/// Orc Peon
#define WC_UnitGoldMine		0x5C	/// Goldmine
#define WC_UnitOilPatch		0x5D	/// Oilpatch
#define WC_StartLocationHuman	0x5E	/// Start location Human
#define WC_StartLocationOrc	0x5F	/// Start location Orc

/*----------------------------------------------------------------------------
--	Declarations
----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
--	Functions
----------------------------------------------------------------------------*/

    /// Return info for pud.
extern MapInfo* GetPudInfo(const char*);

    /// Load a pud file.
extern void LoadPud(const char* pud,WorldMap* map);

//@}

#endif // !__PUD_H__
