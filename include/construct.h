//   ___________		     _________		      _____  __
//   \_	  _____/______   ____   ____ \_   ___ \____________ _/ ____\/  |_
//    |    __) \_  __ \_/ __ \_/ __ \/    \  \/\_  __ \__  \\   __\\   __|
//    |     \   |  | \/\  ___/\  ___/\     \____|  | \// __ \|  |   |  |
//    \___  /   |__|    \___  >\___  >\______  /|__|  (____  /__|   |__|
//	  \/		    \/	   \/	     \/		   \/
//  ______________________                           ______________________
//			  T H E   W A R   B E G I N S
//	   FreeCraft - A free fantasy real time strategy game engine
//
/**@name construct.h	-	The constructions headerfile. */
//
//	(c) Copyright 1998-2003 by Lutz Sammer
//
//	FreeCraft is free software; you can redistribute it and/or modify
//	it under the terms of the GNU General Public License as published
//	by the Free Software Foundation; only version 2 of the License.
//
//	FreeCraft is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//	GNU General Public License for more details.
//
//	$Id$

#ifndef __CONSTRUCT_H__
#define __CONSTRUCT_H__

//@{

/*----------------------------------------------------------------------------
--	Documentation
----------------------------------------------------------------------------*/

/**
**	@struct _construction_ construct.h
**
**	\#include "construct.h"
**
**	typedef struct _construction_ Construction;
**
**		Each building perhaps also units can have its own construction
**		frames. This construction frames are currently not animated,
**		this is planned for the future. What construction frames a
**		building has, is handled by UnitType::Construction.
**
**	The construction structure members:
**
**	Construction::OType
**
**		Object type (future extensions).
**
**	Construction::Ident
**
**		Unique identifier of the construction, used to reference it in
**		the config files and during startup. As convention they start
**		with "construction-" fe. "construction-land".
**		@note Don't use this member in game, use instead the pointer
**		to this structure. See ConstructionByIdent().
**
**	Construction::File[::TilesetMax]
**
**		Path file name of sprite files for the different tilesets.
**		@note It is planned to change this to support more and
**		better tilesets.
**
**	Construction::File[::TilesetMax]
**
**		Path file name of shadow sprite file for the different tilesets.
**
**	Construction::Nr
**
**		Slot number of the construction, used for saving. This should
**		be removed, if we use symbol identifiers.
**		@todo can now be removed
**
**	Construction::Width Construction::Height
**
**		Size of a sprite frame in pixels. All frames of a sprite have
**		the same size. Also all sprites (tilesets) must have the same
**		size.
**
**	Construction::ShadowWidth Construction::ShadowHeight
**
**		Size of a shadow sprite frame in pixels. All frames of a sprite
**		have the same size. Also all sprites (tilesets) must have the
**		same size.
**
**	Construction::Sprite
**
**		Sprite image.
**
**	Construction::ShadowSprite
**
**		Shadow sprite image.
**
**	@todo
**		Need ::TilesetByName, ...
**		Only fixed number of constructions supported, more than
**		a single construction frame is not supported, animated
**		constructions aren't supported.
*/

/*----------------------------------------------------------------------------
--	Includes
----------------------------------------------------------------------------*/

#include "tileset.h"
#include "video.h"

/*----------------------------------------------------------------------------
--	Declarations
----------------------------------------------------------------------------*/

    /// Construction shown during construction of a building
typedef struct _construction_ {
    const void* OType;			/// Object type (future extensions)

    char*	Ident;			/// construction identifier
    struct {
	char*	File;			/// sprite file
	int	Width;			/// sprite width
	int	Height;			/// sprite height
    } File[TilesetMax], ShadowFile[TilesetMax];

// --- FILLED UP ---

    Graphic*	Sprite;			/// construction sprite image
    int	Width;				/// sprite width
    int	Height;				/// sprite height
    Graphic*	ShadowSprite;		/// construction shadow sprite image
    int	ShadowWidth;			/// shadow sprite width
    int	ShadowHeight;			/// shadow sprite height
} Construction;

/*----------------------------------------------------------------------------
--	Macros
----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
--	Variables
----------------------------------------------------------------------------*/

extern const char ConstructionType[];	/// Construction type

/*----------------------------------------------------------------------------
--	Functions
----------------------------------------------------------------------------*/

    ///	Initialize the constructions module
extern void InitConstructions(void);
    ///	Load the graphics for constructions
extern void LoadConstructions(void);
    /// Save current construction state
extern void SaveConstructions(FILE* file);
    ///	Clean up the constructions module
extern void CleanConstructions(void);
    ///	Draw a construction
extern void DrawConstruction(const Construction*,int image,int x,int y);
    /// Get construction by wc number
extern Construction* ConstructionByWcNum(int num);
    /// Get construction by identifier
extern Construction* ConstructionByIdent(const char* ident);

    /// Register ccl features
extern void ConstructionCclRegister(void);

//@}

#endif	// !__CONSTRUCT_H__
