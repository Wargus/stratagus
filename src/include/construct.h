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
/**@name construct.h - The constructions headerfile. */
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

#ifndef __CONSTRUCT_H__
#define __CONSTRUCT_H__

//@{

/*----------------------------------------------------------------------------
--  Documentation
----------------------------------------------------------------------------*/

/**
**  @struct _construction_ construct.h
**
**  \#include "construct.h"
**
**  typedef struct _construction_ Construction;
**
**  Each building perhaps also units can have its own construction
**    frames. This construction frames are currently not animated,
**    this is planned for the future. What construction frames a
**    building has, is handled by UnitType::Construction.
**
**  The construction structure members:
**
**  Construction::OType
**
**    Object type (future extensions).
**
**  Construction::Ident
**
**    Unique identifier of the construction, used to reference it in
**    the config files and during startup. As convention they start
**    with "construction-" fe. "construction-land".
**    @note Don't use this member in game, use instead the pointer
**      to this structure. See ConstructionByIdent().
**
**  Construction::File[::TilesetMax]
**
**    Path file name of sprite files for the different tilesets.
**    @note It is planned to change this to support more and
**      better tilesets.
**
**  Construction::File[::TilesetMax]
**
**    Path file name of shadow sprite file for the different tilesets.
**
**  Construction::Nr
**
**    Slot number of the construction, used for saving. This should
**    be removed, if we use symbol identifiers.
**    @todo can now be removed
**
**  Construction::Width Construction::Height
**
**    Size of a sprite frame in pixels. All frames of a sprite have
**    the same size. Also all sprites (tilesets) must have the same
**    size.
**
**  Construction::ShadowWidth Construction::ShadowHeight
**
**    Size of a shadow sprite frame in pixels. All frames of a sprite
**    have the same size. Also all sprites (tilesets) must have the
**    same size.
**
**  Construction::Sprite
**
**    Sprite image.
**
**  Construction::ShadowSprite
**
**    Shadow sprite image.
**
**    @todo
**      Need ::TilesetByName, ...
**      Only fixed number of constructions supported, more than
**      a single construction frame is not supported, animated
**      constructions aren't supported.
*/

/*----------------------------------------------------------------------------
--  Declarations
----------------------------------------------------------------------------*/

struct _graphic_;

	/// Construction frame
typedef struct _construction_frame_ {
	int Percent;  ///< Percent complete
	enum {
		ConstructionFileConstruction,
		ConstructionFileMain,
	} File;  ///< Graphic to use
	int Frame;  ///< Frame number
	struct _construction_frame_* Next;  ///< Next pointer
} ConstructionFrame;

	/// Construction shown during construction of a building
typedef struct _construction_ {
	const void* OType;  ///< Object type (future extensions)
	char*       Ident;  ///< construction identifier
	struct {
		char* File;    ///< sprite file
		int   Width;   ///< sprite width
		int   Height;  ///< sprite height
	} File[TilesetMax], ShadowFile[TilesetMax];
	ConstructionFrame* Frames;  ///< construction frames

// --- FILLED UP ---

	struct _graphic_* Sprite;        ///< construction sprite image
	int               Width;         ///< sprite width
	int               Height;        ///< sprite height
	struct _graphic_* ShadowSprite;  ///< construction shadow sprite image
	int               ShadowWidth;   ///< shadow sprite width
	int               ShadowHeight;  ///< shadow sprite height
} Construction;

/*----------------------------------------------------------------------------
--  Macros
----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
--  Variables
----------------------------------------------------------------------------*/

extern const char ConstructionType[];  ///< Construction type

/*----------------------------------------------------------------------------
--  Functions
----------------------------------------------------------------------------*/

	/// Initialize the constructions module
extern void InitConstructions(void);
	/// Load the graphics for constructions
extern void LoadConstructions(void);
	/// Clean up the constructions module
extern void CleanConstructions(void);
	/// Get construction by wc number
extern Construction* ConstructionByWcNum(int num);
	/// Get construction by identifier
extern Construction* ConstructionByIdent(const char* ident);

	/// Register ccl features
extern void ConstructionCclRegister(void);

//@}

#endif // !__CONSTRUCT_H__
