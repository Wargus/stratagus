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
/**@name editor.h - The editor file. */
//
//      (c) Copyright 2002-2004 by Lutz Sammer
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

#ifndef __EDITOR_H__
#define __EDITOR_H__

//@{

/*----------------------------------------------------------------------------
--  Variables
----------------------------------------------------------------------------*/

	/// Editor is running
extern char EditorRunning;
	/// Map loaded in editor
extern char EditorMapLoaded;
	/// Current editor state type.
typedef enum _editor_state_type_ {
	EditorSelecting,  ///< Select
	EditorEditTile,   ///< Edit tiles
	EditorEditUnit,   ///< Edit units
} EditorStateType;    ///< Current editor state
	/// Current editor state.
extern EditorStateType EditorState;

	/// Editor CCL start file
extern const char* EditorStartFile;

extern char** EditorUnitTypes;  ///< Sorted editor unit-type table

extern int MaxUnitIndex;  ///< Max unit icon draw index

extern char* EditorSelectIcon;  ///< Editor's select icon
extern char* EditorUnitsIcon;   ///< Editor's units icon

/*----------------------------------------------------------------------------
--  Functions
----------------------------------------------------------------------------*/

	/// Editor main event loop
extern void EditorMainLoop(void);
	/// Update editor display
extern void EditorUpdateDisplay(void);

	/// Save a pud from editor
extern int EditorSavePud(const char *file);

	/// Register ccl features
extern void EditorCclRegister(void);

	/// Edit tile
extern void EditTile(int x, int y, int tile);
	/// Edit tiles
extern void EditTiles(int x, int y, int tile, int size);

	/// Change the view of a tile
extern void ChangeTile(int x, int y, int tile);
	/// Update surroundings for tile changes
extern void EditorTileChanged(int x, int y);
	/// Make random map
extern void EditorCreateRandomMap(void);

//@}

#endif // !__EDITOR_H__
