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
/*
**	(c) Copyright 1998-2000 by Lutz Sammer
**
**	$Id$
*/

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

/*----------------------------------------------------------------------------
--	Declarations
----------------------------------------------------------------------------*/

/**
**	Get info for a pud.
*/
typedef struct _pud_info_ {
    char*	Description;		/// Map description
    int		MapTerrain;		/// Map terrain
    int		MapWidth;		/// Map width
    int		MapHeight;		/// Map height
    int		PlayerType[16];		/// Same player->Type
    int		PlayerSide[16];		/// Same player->Side
    int		PlayerGold[16];		/// Same player->Gold
    int		PlayerWood[16];		/// Same player->Wood
    int		PlayerOil[16];		/// Same player->Oil
    int		PlayerAi[16];		/// Same player->Ai
} PudInfo;

/*----------------------------------------------------------------------------
--	Functions
----------------------------------------------------------------------------*/

    /// Return info for pud.
extern PudInfo* GetPudInfo(const char*);

    /// Release info for pud.
extern void FreePudInfo(PudInfo*);

    /// Load a pud file.
extern void LoadPud(const char* pud,WorldMap* map);

//@}

#endif // !__PUD_H__
