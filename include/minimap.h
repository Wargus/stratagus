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
/*
**	(c) Copyright 1998,2000 by Lutz Sammer
**
**	$Id$
*/

#ifndef __MINIMAP_H__
#define __MINIMAP_H__

//@{

/*----------------------------------------------------------------------------
--	Minimap
----------------------------------------------------------------------------*/

#define MINIMAP_FAC	(16*3)		/// integer scale factor

/**
**	Convert minimap cursor X position to map coordinate.
*/
#define Minimap2MapX(x)	\
    ((((x)-TheUI.MinimapX-24-MinimapX)*MINIMAP_FAC)/MinimapScale)

/**
**	Convert minimap cursor Y position to map coordinate.
*/
#define Minimap2MapY(y)	\
    ((((y)-TheUI.MinimapY-2-MinimapY)*MINIMAP_FAC)/MinimapScale)

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

extern void UpdateMinimapXY(int tx,int ty);	///
extern void UpdateMinimap(void);
extern void CreateMinimap(void);
extern void DrawMinimap(int vx,int vy);
extern void HideMinimapCursor(void);
extern void DrawMinimapCursor(int vx,int vy);

//@}

#endif	// !__MINIMAP_H__
