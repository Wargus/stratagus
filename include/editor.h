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

#include <vector>
#include "player.h"

class CUnitType;
class IconConfig;

/*----------------------------------------------------------------------------
--  Variables
----------------------------------------------------------------------------*/

	/// Editor is running
typedef enum _editor_running_state_ {
	EditorNotRunning = 0,   /// Not Running
	EditorStarted = 1,      /// Editor Enabled at all
	EditorCommandLine = 2,  /// Called from Command Line
	EditorEditing = 4       /// Editor is fully running
} EditorRunningType;

extern EditorRunningType EditorRunning;

extern char EditorMapLoaded;  /// Map loaded in editor
extern int EditorWriteCompressedMaps;

	/// Current editor state type.
typedef enum _editor_state_type_ {
	EditorSelecting,  ///< Select
	EditorEditTile,   ///< Edit tiles
	EditorEditUnit,   ///< Edit units
	EditorSetStartLocation ///< Set the start location
} EditorStateType;    ///< Current editor state
	/// Current editor state.
extern EditorStateType EditorState;

extern const char *EditorStartFile;  /// Editor CCL start file

class CEditor {
public:
	CEditor() : TerrainEditable(true),
				StartUnitName(NULL), StartUnit(NULL),
				ShowUnitsToSelect(true), ShowBuildingsToSelect(true),
				ShowAirToSelect(true), ShowLandToSelect(true), ShowWaterToSelect(true),

				UnitIndex(0), CursorUnitIndex(-1), SelectedUnitIndex(-1),
				CursorPlayer(-1), SelectedPlayer(PlayerNumNeutral)

				 {};
	~CEditor() {
		delete [] StartUnitName;
	};

	void Init();
	/// Make random map
	void CreateRandomMap() const;


	std::vector<char*> UnitTypes;                   /// Sorted editor unit-type table.
	std::vector<const CUnitType *> ShownUnitTypes;  /// Shown editor unit-type table.

	bool TerrainEditable;        /// Is the terrain editable ?
	IconConfig Select;           /// Editor's select icon.
	IconConfig Units;            /// Editor's units icon.
	char* StartUnitName;         /// name of the Unit used to display the start location.
	const CUnitType *StartUnit;  /// Unit used to display the start location.

	bool ShowUnitsToSelect;      /// Show units in unit list.
	bool ShowBuildingsToSelect;  /// Show buildings in unit list.
#if 0
	bool ShowHeroesToSelect;     /// Show heroes in unit list.
#endif
	bool ShowAirToSelect;        /// Show air units in unit list.
	bool ShowLandToSelect;       /// Show land units in unit list.
	bool ShowWaterToSelect;      /// Show water units in unit list.

	int UnitIndex;               /// Unit icon draw index.
	int CursorUnitIndex;         /// Unit icon under cursor.
	int SelectedUnitIndex;       /// Unit type to draw.

	int CursorPlayer;            /// Player under the cursor.
	int SelectedPlayer;          /// Player selected for draw.

};

extern CEditor Editor;


/*----------------------------------------------------------------------------
--  Functions
----------------------------------------------------------------------------*/

	/// Editor main event loop
extern void EditorMainLoop(void);
	/// Update editor display
extern void EditorUpdateDisplay(void);

	/// Save a map from editor
extern int EditorSaveMap(const char *file);

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


//@}

#endif // !__EDITOR_H__
