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
//      (c) Copyright 1998-2006 by Lutz Sammer and Jimmy Salmon
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

#ifndef __CONSTRUCT_H__
#define __CONSTRUCT_H__

//@{

/*----------------------------------------------------------------------------
--  Documentation
----------------------------------------------------------------------------*/

/**
**  @class CConstruction construct.h
**
**  \#include "construct.h"
**
**  Each building perhaps also units can have its own construction
**    frames. This construction frames are currently not animated,
**    this is planned for the future. What construction frames a
**    building has, is handled by UnitType::Construction.
**
**  The construction structure members:
**
**  CConstruction::Ident
**
**    Unique identifier of the construction, used to reference it in
**    the config files and during startup. As convention they start
**    with "construction-" fe. "construction-land".
**    @note Don't use this member in game, use instead the pointer
**      to this structure. See ConstructionByIdent().
**
**  CConstruction::File
**
**    Path file name of the sprite file.
**
**  CConstruction::ShadowFile
**
**    Path file name of shadow sprite file.
**
**  CConstruction::Frames
**
**    Frames of the construction sequence.
**
**  CConstruction::Sprite
**
**    Sprite image.
**
**  CConstruction::Width CConstruction::Height
**
**    Size of a sprite frame in pixels. All frames of a sprite have
**    the same size. Also all sprites (tilesets) must have the same
**    size.
**
**  CConstruction::ShadowSprite
**
**    Shadow sprite image.
**
**  CConstruction::ShadowWidth CConstruction::ShadowHeight
**
**    Size of a shadow sprite frame in pixels. All frames of a sprite
**    have the same size. Also all sprites (tilesets) must have the
**    same size.
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

class CGraphic;
class CPlayerColorGraphic;


enum ConstructionFileType {
	ConstructionFileConstruction,
	ConstructionFileMain
};

	/// Construction frame
class CConstructionFrame {
public:
	CConstructionFrame() : Percent(0), File(ConstructionFileConstruction),
		Frame(0), Next(NULL) {}

	int Percent;                    /// Percent complete
	ConstructionFileType File;      /// Graphic to use
	int Frame;                      /// Frame number
	CConstructionFrame *Next;       /// Next pointer
};

	/// Construction shown during construction of a building
class CConstruction {
public:
	CConstruction() : Frames(NULL), Sprite(NULL), Width(0),
		Height(0), ShadowSprite(NULL), ShadowWidth(0), ShadowHeight(0)
	{
		File.Width = 0;
		File.Height = 0;
		ShadowFile.Width = 0;
		ShadowFile.Height = 0;
	}

	std::string Ident;   /// construction identifier
	struct {
		std::string File;/// sprite file
		int Width;       /// sprite width
		int Height;      /// sprite height
	} File, ShadowFile;
	CConstructionFrame *Frames;  /// construction frames

// --- FILLED UP ---

	CPlayerColorGraphic *Sprite;/// construction sprite image
	int      Width;         /// sprite width
	int      Height;        /// sprite height
	CGraphic *ShadowSprite; /// construction shadow sprite image
	int      ShadowWidth;   /// shadow sprite width
	int      ShadowHeight;  /// shadow sprite height
};

/*----------------------------------------------------------------------------
--  Macros
----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
--  Variables
----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
--  Functions
----------------------------------------------------------------------------*/

	/// Initialize the constructions module
extern void InitConstructions();
	/// Load the graphics for constructions
extern void LoadConstructions();
	/// Clean up the constructions module
extern void CleanConstructions();
	/// Get construction by identifier
extern CConstruction *ConstructionByIdent(const std::string &ident);

	/// Register ccl features
extern void ConstructionCclRegister();

//@}

#endif // !__CONSTRUCT_H__
