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
/**@name minimap.h	-	The minimap headerfile. */
//
//	(c) Copyright 1998,2000,2001 by Lutz Sammer
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

#ifndef __MINIMAP_H__
#define __MINIMAP_H__

//@{

/*----------------------------------------------------------------------------
--	Declarations
----------------------------------------------------------------------------*/

#define MINIMAP_FAC	(16*3)		/// integer scale factor


    /// Update seen tile change in minimap
#define UpdateMinimapSeenXY(tx,ty)

/*----------------------------------------------------------------------------
--	Variables
----------------------------------------------------------------------------*/

extern int MinimapScale;		/// Minimap scale to fit into window
extern int MinimapX;			/// Minimap drawing position x offset.
extern int MinimapY;			/// Minimap drawing position y offset.

extern int MinimapWithTerrain;		/// display minimap with terrain
extern int MinimapFriendly;		/// switch colors of friendly units
extern int MinimapShowSelected;		/// highlight selected units

/*----------------------------------------------------------------------------
--	Functions
----------------------------------------------------------------------------*/

    /// Update tile change in minimap
extern void UpdateMinimapXY(int tx,int ty);
    /// Update complete minimap
extern void UpdateMinimap(void);
    /// Create new minimap
extern void CreateMinimap(void);
    /// Draw minimap with viewpoint
extern void DrawMinimap(int vx,int vy);
    /// Hide minimap cursor
extern void HideMinimapCursor(void);
    /// Draw minimap viewpoint cursor
extern void DrawMinimapCursor(int vx,int vy);

    ///	Convert minimap cursor X position to tile map coordinate
extern int ScreenMinimap2MapX(int);
    ///	Convert minimap cursor Y position to tile map coordinate
extern int ScreenMinimap2MapY(int);

//@}

#endif	// !__MINIMAP_H__
