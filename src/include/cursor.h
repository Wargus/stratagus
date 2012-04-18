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
/**@name cursor.h - The cursors header file. */
//
//      (c) Copyright 1998-2005 by Lutz Sammer
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

#ifndef __CURSOR_H__
#define __CURSOR_H__

//@{

/*----------------------------------------------------------------------------
--  Documentation
----------------------------------------------------------------------------*/

/**
**  @class CCursor cursor.h
**
**  \#include "cursor.h"
**
**  This structure contains all informations about a cursor.
**  The cursor changes depending of the current user input state.
**  A cursor can have transparent areas and color cycle animated.
**
**  The cursor-type structure members:
**
**  CCursor::Ident
**
**    Unique identifier of the cursor, used to reference it in config
**    files and during startup. Don't use this in game, use instead
**    the pointer to this structure.
**
**  CCursor::Race
**
**    Owning Race of this cursor ("human", "orc", "alliance",
**    "mythical", ...). If NULL, this cursor could be used by any
**    race.
**
**  CCursor::HotX CCursor::HotY
**
**    Hot spot of the cursor in pixels. Relative to the sprite origin
**    (0,0). The hot spot of a cursor is the point to which Stratagus
**    refers in tracking the cursor's position.
**
**  CCursor::SpriteFrame
**
**    Current displayed cursor frame.
**    From 0 to CCursor::G::NumFrames.
**
**  CCursor::FrameRate
**
**    Rate of changing the frames. The "rate" tells the engine how
**    many milliseconds to hold each frame of the animation.
**
**    @note  This is the first time that for timing ms are used! I would
**           change it to display frames.
**
**  CCursor::G
**
**    Contains the sprite of the cursor, loaded from CCursor::File.
**    This can be a multicolor image with alpha or transparency.
*/

/**
**  @class CursorConfig cursor.h
**
**  \#include "cursor.h"
**
**  This structure contains all information to reference/use a cursor.
**  It is normally used in other config structures.
**
**  CursorConfig::Name
**
**    Name to reference this cursor-type. Used while initialization.
**    (See CCursor::Ident)
**
**  CursorConfig::Cursor
**
**    Pointer to this cursor-type. Used while runtime.
*/

/*----------------------------------------------------------------------------
--  Includes
----------------------------------------------------------------------------*/

#include <vector>

/*----------------------------------------------------------------------------
--  Declarations
----------------------------------------------------------------------------*/

class CGraphic;
class CUnitType;

/*----------------------------------------------------------------------------
--  Definitions
----------------------------------------------------------------------------*/

/// Private type which specifies the cursor-type
class CCursor
{
public:
	CCursor() : HotX(0), HotY(0),
		SpriteFrame(0), FrameRate(0), G(NULL) {}

	std::string Ident;  /// Identifier to reference it
	std::string Race;   /// Race name

	int HotX;     /// Hot point x
	int HotY;     /// Hot point y

	int SpriteFrame;  /// Current displayed cursor frame
	int FrameRate;    /// Rate of changing the frames

	// --- FILLED UP ---

	CGraphic *G; /// Cursor sprite image
};

/// Cursor config reference
class CursorConfig
{
public:
	CursorConfig() : Cursor(NULL) {}

	void Load();

	std::string Name; /// Config cursor-type name
	CCursor *Cursor;  /// Cursor-type pointer
};

/// Cursor state
enum CursorStates {
	CursorStatePoint,      /// Normal cursor
	CursorStateSelect,     /// Select position
	CursorStateRectangle,  /// Rectangle selecting
	CursorStatePieMenu     /// Displaying Pie Menu
};

/*----------------------------------------------------------------------------
--  Variables
----------------------------------------------------------------------------*/

extern std::vector<CCursor *> AllCursors; /// cursor-types description

extern CursorStates CursorState;  /// current cursor state (point,...)
extern int CursorAction;          /// action for selection
extern int CursorValue;           /// value for action (spell type f.e.)
extern CUnitType *CursorBuilding; /// building cursor
extern std::string CustomCursor;             /// custom cursor for button

extern CCursor *GameCursor;     /// cursor-type
extern int CursorX;             /// cursor position on screen X
extern int CursorY;             /// cursor position on screen Y
extern int CursorStartX;        /// rectangle started on screen X
extern int CursorStartY;        /// rectangle started on screen Y
extern int CursorStartScrMapX;  /// the same in screen map coordinate system
extern int CursorStartScrMapY;  /// the same in screen map coordinate system

/*----------------------------------------------------------------------------
--  Functions
----------------------------------------------------------------------------*/

/// Load all cursors
extern void LoadCursors(const std::string &racename);

/// Cursor by identifier
extern CCursor *CursorByIdent(const std::string &ident);

/// Draw any cursor
extern void DrawCursor();
/// Hide the cursor
extern void HideCursor();
/// Animate the cursor
extern void CursorAnimate(unsigned ticks);

/// Initialize the cursor module
extern void InitVideoCursors();
/// Cleanup the cursor module
extern void CleanCursors();

//@}

#endif // !__CURSOR_H__
