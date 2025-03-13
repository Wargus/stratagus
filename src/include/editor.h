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
#include "editor_brush_ui.h"
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
	SetStartLocation,  /// Set the start location
	EditUnit           /// Edit units
};

class CTileIconsSet
{
public:
	CTileIconsSet() = default;
	~CTileIconsSet() = default;

	void enable(bool value = true)
	{
		enabled = value;
		if (!enabled) {
			resetSelected();
		}
	}
	bool isEnabled() { return enabled; }

	bool isSelected() const { return selected != -1; }

	std::optional<tile_index> getTile(size_t iconNo) const;

	void setIconUnderCursor(size_t iconNo) { iconUnderCursor = iconNo; }
	int getIconUnderCursor() const { return iconUnderCursor; }
	void resetIconUnderCursor() { iconUnderCursor = -1; }

	std::optional<tile_index> getSelectedTile() const;
	int getSelectedIcon() const { return selected; }
	void select(const uint16_t iconNo)
	{
		selected = iconNo < icons.size() ? iconNo : selected;
	}
	void resetSelected() { selected = -1; }

	size_t numberOf() const { return icons.size(); }

	void rebuild(bool manualMode = false, bool firstOfKindOnly = true);

	void clear() { icons.clear(); }

	void updateSliderCtrl();
	void recalcDisplayed();
	void displayFrom(const uint16_t shift)
	{
		shiftToFirstDisplayed = shift < icons.size() ? shift : shiftToFirstDisplayed;
	}
	int getDisplayedFirst() { return shiftToFirstDisplayed; }
	void setDisplayedNum(const uint16_t number);
	uint16_t getDisplayedNum() const { return displayedNum; }

	void attachSliderCtrl(gcn::Slider *slider) { sliderCtrl = slider; }
	void resetSliderCtrl() { sliderCtrl = nullptr; }

private:
	bool enabled = true;
	int selected = -1; /// Selected tile icon
	int iconUnderCursor = -1;

	gcn::Slider* sliderCtrl = nullptr;
	uint16_t shiftToFirstDisplayed = 0;	/// Shift to the first icon displayed
	uint16_t displayedNum = 0;	/// The number of icons that can be displayed in the area

	std::vector<tile_index> icons; /// Set of tile icons (palette)
};

class CEditor
{
public:
	CEditor();
	~CEditor() {}

	void Init();

	void applyCurentBrush(const Vec2i &pos);

	/// Make random map
	void CreateRandomMap(bool shuffleTransitions = false);

private:
	void SetTile(const Vec2i &pos, tile_index tileIdx);
	void ChangeTile(const Vec2i &pos,
					tile_index tileIndex,
					const Vec2i &lock_pos,
					bool changeSurroundings,
					bool randomizeTile);
	/// Callback for changed tile (with locked position)
	void ChangeSurrounding(const Vec2i &pos, const Vec2i &lock_pos);
	void RandomizeTile(int tile, int count, int max_size);
	void TileFill(const Vec2i &pos, int tile, int size);
	void RandomizeTransition(int x, int y);
	void RandomizeUnit(const std::string_view unit_type,
					   int count,
					   int value,
					   int tileIndexUnderUnit);

public:
	/// Variables for random map creation
	int BaseTileIndex; /// Tile to fill the map with initially;
	std::vector<std::tuple<int, int, int>> RandomTiles; /// other tiles to fill randomly. (tile, count, area size)
	std::vector<std::tuple<std::string, int, int, int>> RandomUnits; /// neutral units to add randomly. (name, count, initial resources, tile under unit)

	std::vector<std::string> UnitTypes;             /// Sorted editor unit-type table.
	std::vector<const CUnitType *> ShownUnitTypes;  /// Shown editor unit-type table.

	bool TerrainEditable = true;	/// Is the terrain editable?
	IconConfig Select;				/// Editor's select icon.
	IconConfig Units;				/// Editor's units icon.
	std::string StartUnitName;		/// name of the Unit used to display the start location.
	const CUnitType *StartUnit = nullptr;	/// Unit used to display the start location.

	int UnitIndex = 0;				/// Unit icon draw index.
	int CursorUnitIndex = -1;		/// Unit icon under cursor.
	int SelectedUnitIndex = -1;		/// Unit type to draw.

	bool BuildingRandomMap = false;	/// Is random map generation in progress

	bool UpdateMinimap = false;		/// Update units on the minimap
	int MirrorEdit = 0;				/// Mirror editing enabled

	uint8_t	HighlightElevationLevel {0};
	uint8_t	SelectedElevationLevel {0};

	int CursorPlayer = -1;            /// Player under the cursor.
	int SelectedPlayer = PlayerNumNeutral; /// Player selected for draw.

	EditorRunningType Running = EditorNotRunning; /// Editor is running
	EditorStateType State = EditorStateType::Selecting; /// Current editor state

	fieldHighlightChecker OverlayHighlighter {nullptr};

	CBrushesSet brushes;
	CTileIconsSet tileIcons;
};

/*----------------------------------------------------------------------------
--  Variables
----------------------------------------------------------------------------*/

extern CEditor Editor;

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

//@}

#endif // !__EDITOR_H__
