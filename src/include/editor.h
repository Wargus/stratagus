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
//      (c) Copyright 2002-2006 by Lutz Sammer
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

#ifndef __EDITOR_H__
#define __EDITOR_H__

//@{

#include <vector>
#include <string>
#include <tuple>
#include "icons.h"
#include "viewport.h"
#include "editor_brush.h"
#ifndef __VEC2I_H__
#include "vec2i.h"
#endif
/*----------------------------------------------------------------------------
--  Declarations
----------------------------------------------------------------------------*/

class CUnitType;


enum EditorRunningType {
	EditorNotRunning = 0,    /// Not Running
	EditorStarted = 1,       /// Editor Enabled at all
	EditorCommandLine = 2,   /// Called from Command Line
	EditorEditing = 4        /// Editor is fully running
};

enum class EditorStateType {
	Selecting,         /// Select
	EditTile,          /// Edit tiles
	ElevationLevel,    /// Edit elevation levels
	EditRamps,         /// Place and edit ramps
	SetStartLocation,  /// Set the start location
	EditUnit           /// Edit units
};

class CEditor
{
public:
	CEditor();
	~CEditor() {}

	void Init();

	void LoadBrushes();
	CBrush &getCurrentBrush() { return curentBrush == -1 ? defaultBrush : brushes[curentBrush]; };

	/// Make random map
	void CreateRandomMap(bool shuffleTransitions = false) const;

public:
	/// Variables for random map creation
	int BaseTileIndex; /// Tile to fill the map with initially;
	std::vector<std::tuple<int, int, int>> RandomTiles; /// other tiles to fill randomly. (tile, count, area size)
	std::vector<std::tuple<std::string, int, int, int>> RandomUnits; /// neutral units to add randomly. (name, count, initial resources, tile under unit)

	std::vector<std::string> UnitTypes;             /// Sorted editor unit-type table.
	std::vector<const CUnitType *> ShownUnitTypes;  /// Shown editor unit-type table.
	std::vector<tile_index> ShownTileTypes;			/// Shown editor tile-type table.

	bool TerrainEditable = true; /// Is the terrain editable?
	IconConfig Select;           /// Editor's select icon.
	IconConfig Units;            /// Editor's units icon.
	std::string StartUnitName;   /// name of the Unit used to display the start location.
	const CUnitType *StartUnit = nullptr;  /// Unit used to display the start location.

	int UnitIndex = 0;               /// Unit icon draw index.
	int CursorUnitIndex = -1;        /// Unit icon under cursor.
	int SelectedUnitIndex = -1;      /// Unit type to draw.

	int TileIndex = 0;              /// tile icon draw index.
	int CursorTileIndex = -1;       /// tile icon under cursor.
	int SelectedTileIndex = -1;     /// tile type to draw.

	uint8_t	HighlightElevationLevel {0};
	uint8_t	SelectedElevationLevel {0};

	int CursorPlayer = -1;            /// Player under the cursor.
	int SelectedPlayer = PlayerNumNeutral; /// Player selected for draw.

	EditorRunningType Running = EditorNotRunning; /// Editor is running
	EditorStateType State = EditorStateType::Selecting; /// Current editor state

	fieldHighlightChecker OverlayHighlighter {nullptr};

private:
	CBrush defaultBrush{CBrush::BrushTypes::SingleTile, 1, 1};
	std::vector<CBrush> brushes;
	int16_t curentBrush = -1;
	std::string BrushesSrc = "scripts/editor/brushes.lua";
};

/*----------------------------------------------------------------------------
--  Variables
----------------------------------------------------------------------------*/

extern CEditor Editor;

extern bool TileToolNoFixup;
extern char TileToolRandom;

/*----------------------------------------------------------------------------
--  Functions
----------------------------------------------------------------------------*/

/// Start the editor
extern void StartEditor(const char *filename);

/// Editor main event loop
extern void EditorMainLoop();
/// Update editor display
extern void EditorUpdateDisplay();

/// Save a map from editor
extern int EditorSaveMap(const std::string &file);
extern int EditorSaveMapWithResize(std::string_view file, Vec2i sz = {0, 0}, Vec2i off = {0, 0});

/// Register ccl features
extern void EditorCclRegister();

/// Update surroundings for tile changes
extern void EditorTileChanged(const Vec2i &pos);

//@}

#endif // !__EDITOR_H__
