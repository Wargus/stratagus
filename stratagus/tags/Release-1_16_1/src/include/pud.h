/*
**	A clone of a famous game.
*/
/**@name pud.h		-	The pud headerfile. */
/*
**	(c) Copyright 1998,1999 by Lutz Sammer
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
--	Functions
----------------------------------------------------------------------------*/

    /// Load a pud file.
extern void LoadPud(const char* pud,WorldMap* map);

//@}

#endif // !__PUD_H__
