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
/**@name construct.h	-	The constructions headerfile. */
//
//	(c) Copyright 1998-2001 by Lutz Sammer
//
//	$Id$

#ifndef __CONSTRUCT_H__
#define __CONSTRUCT_H__

//@{

// FIXME: constructions must be configurable, referenced by indenifiers...
// FIXME: need support for more races.

/*----------------------------------------------------------------------------
--	Includes
----------------------------------------------------------------------------*/

#include "tileset.h"
#include "video.h"

/*----------------------------------------------------------------------------
--	Declarations
----------------------------------------------------------------------------*/

/**
**	Constructions: shown during construction of a building.
*/
typedef struct _construction_ {
    char*	Ident;			/// construction identifier
    char*	File[TilesetMax];	/// sprite file

    int		Width;			/// " width
    int		Height;			/// " height

// --- FILLED UP ---

    Graphic*	Sprite;			/// construction sprite image
} Construction;

/*----------------------------------------------------------------------------
--	Macros
----------------------------------------------------------------------------*/

#define ConstructionWall	15	/// ident nr for wall under construction

/*----------------------------------------------------------------------------
--	Functions
----------------------------------------------------------------------------*/

    ///	Load the graphics for constructions
extern void LoadConstructions(void);
    ///	Draw a construction
extern void DrawConstruction(int type,int image,int x,int y);

//@}

#endif	// !__CONSTRUCT_H__
