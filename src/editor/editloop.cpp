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
/**@name editloop.cpp - The editor main loop. */
//
//      (c) Copyright 2002-2015 by Lutz Sammer, Jimmy Salmon and
//		Andrettin
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

//@{

/*----------------------------------------------------------------------------
--  Includes
----------------------------------------------------------------------------*/

#include <deque>
#include <stdint.h>

#include "stratagus.h"

#include "editor.h"

#include "commands.h"
#include "font.h"
#include "game.h"
#include "interface.h"
#include "iolib.h"
#include "map.h"
#include "menus.h"
#include "minimap.h"
#include "network.h"
#include "parameters.h"
#include "replay.h"
#include "script.h"
#include "settings.h"
#include "sound.h"
#include "sound_server.h"
#include "tileset.h"
#include "translate.h"
#include "ui.h"
#include "unit_find.h"
#include "unittype.h"
#include "video.h"
#include "widgets.h"

#include <guichan.h>

extern void DoScrollArea(int state, bool fast, bool isKeyboard);
extern void DrawGuichanWidgets();
extern void CleanGame();
extern void CreateGame(const fs::path &filename, CMap *map);

extern void DrawMapArea(const fieldHighlightChecker highlightChecker);
/*----------------------------------------------------------------------------
--  Defines
----------------------------------------------------------------------------*/

#define UNIT_ICON_X (IconWidth + 7)       /// Unit mode icon
#define UNIT_ICON_Y (0)                   /// Unit mode icon
#define TILE_ICON_X (IconWidth * 2 + 16)  /// Tile mode icon
#define TILE_ICON_Y (2)                   /// Tile mode icon
#define START_ICON_X (IconWidth * 3 + 16)  /// Start mode icon
#define START_ICON_Y (2)                   /// Start mode icon

/*----------------------------------------------------------------------------
--  Variables
----------------------------------------------------------------------------*/

static int IconWidth;                       /// Icon width in panels
static int IconHeight;                      /// Icon height in panels

static int ButtonPanelWidth;
static int ButtonPanelHeight;

bool TileToolNoFixup = false;     /// Allow setting every tile, no fixups
char TileToolRandom;      /// Tile tool draws random
static char TileToolDecoration;  /// Tile tool draws with decorations
static int TileCursorSize;       /// Tile cursor size 1x1 2x2 ... 4x4
static bool UnitPlacedThisPress = false;  /// Only allow one unit per press
static bool UpdateMinimap = false;        /// Update units on the minimap
static int MirrorEdit = 0;                /// Mirror editing enabled
static int VisibleUnitIcons = 0;              /// Number of icons that are visible at a time
static int VisibleTileIcons = 0;

enum class EditorActionType {
	PlaceUnit,
	RemoveUnit
};

enum EditorOverlays {
	cNone,
	cUnpassable,
	cNoBuildingAllowed,
	cElevation,
	cOpaque
};
struct EditorAction {
	EditorActionType Type;
	Vec2i tilePos;
	const CUnitType *UnitType;
	CPlayer *Player;
};

static std::deque<EditorAction> EditorUndoActions;
static std::deque<EditorAction> EditorRedoActions;

static void EditorUndoAction();
static void EditorRedoAction();
static void EditorAddUndoAction(EditorAction action);

extern std::unique_ptr<gcn::Gui> Gui;
static std::unique_ptr<gcn::Container> editorContainer;
static std::unique_ptr<gcn::Slider> editorSlider;
static std::unique_ptr<gcn::DropDown> toolDropdown;
static std::unique_ptr<gcn::DropDown> overlaysDropdown;
/*----------------------------------------------------------------------------
--  Functions
----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
--  Edit
----------------------------------------------------------------------------*/

/**
**  Edit tile.
**
**  @param pos   map tile coordinate.
**  @param tile  Tile type to edit or -1, to edit the tile under the brush according to the modifiers
*/
static void EditTile(const Vec2i &pos, int tile)
{
 	Assert(Map.Info.IsPointOnMap(pos));

	const CTileset &tileset = *Map.Tileset;

	CMapField &mf = *Map.Field(pos);

	int32_t baseTileIndex = tileset.findTileIndexByTile(tile);
	if (baseTileIndex <= 0) {
		// use the tile under the cursor and randomize *that* if it's
		// not a mix tile
		baseTileIndex = tileset.findTileIndexByTile(mf.getGraphicTile());
		const int32_t mixTerrainIdx = tileset.tiles[baseTileIndex].tileinfo.MixTerrain;
		if (mixTerrainIdx > 0) {
			return;
		}
		baseTileIndex = baseTileIndex / 16 * 16;
	}
	const tile_index tileIndex = tileset.getTileNumber(baseTileIndex, TileToolRandom, TileToolDecoration);
	mf.setTileIndex(tileset, tileIndex, 0, mf.getElevation());
	mf.playerInfo.SeenTile = mf.getGraphicTile();

	UI.Minimap.UpdateSeenXY(pos);
	UI.Minimap.UpdateXY(pos);

	EditorTileChanged(pos);
	UpdateMinimap = true;
}

/**
**  Edit tiles (internal, used by EditTiles()).
**
**  @param pos   map tile coordinate.
**  @param tile  Tile type to edit.
**  @param size  Size of rectangle
**
**  @bug  This function does not support mirror editing!
*/
static void EditTilesInternal(const Vec2i &pos, int tile, int size)
{
	Vec2i minPos = pos;
	Vec2i maxPos(pos.x + size - 1, pos.y + size - 1);

	Map.FixSelectionArea(minPos, maxPos);

	Vec2i itPos;
	for (itPos.y = minPos.y; itPos.y <= maxPos.y; ++itPos.y) {
		for (itPos.x = minPos.x; itPos.x <= maxPos.x; ++itPos.x) {
			EditTile(itPos, tile);
		}
	}
}

/**
**  Edit tiles
**
**  @param pos   map tile coordinate.
**  @param tile  Tile type to edit.
**  @param size  Size of rectangle
*/
static void EditTiles(const Vec2i &pos, int tile, int size)
{
	EditTilesInternal(pos, tile, size);

	if (!MirrorEdit) {
		return;
	}
	const Vec2i mpos(Map.Info.MapWidth - size, Map.Info.MapHeight - size);
	const Vec2i mirror = mpos - pos;
	const Vec2i mirrorv(mirror.x, pos.y);

	EditTilesInternal(mirrorv, tile, size);
	if (MirrorEdit == 1) {
		return;
	}
	const Vec2i mirrorh(pos.x, mirror.y);

	EditTilesInternal(mirrorh, tile, size);
	EditTilesInternal(mirror, tile, size);
}

/**
**  Set map tile's elevation level
**
**  @param pos   map tile coordinate.
**/
static void EditorSetElevationLevel(const Vec2i &pos, const uint8_t elevation)
{
	Map.Field(pos)->setElevation(elevation);
}
/*----------------------------------------------------------------------------
--  Actions
----------------------------------------------------------------------------*/

/**
**  Place unit.
**
**  @param pos     map tile coordinate.
**  @param type    Unit type to edit.
**  @param player  Player owning the unit.
**
**  @todo  FIXME: Check if the player has already a start-point.
**  @bug   This function does not support mirror editing!
*/
static void EditorActionPlaceUnit(const Vec2i &pos, const CUnitType &type, CPlayer *player)
{
	Assert(Map.Info.IsPointOnMap(pos));

	if (type.Neutral) {
		player = &Players[PlayerNumNeutral];
	}

	// FIXME: vladi: should check place when mirror editing is enabled...?
	CUnit *unit = MakeUnitAndPlace(pos, type, player);
	if (unit == nullptr) {
		DebugPrint("Unable to allocate Unit");
		return;
	}

	CBuildRestrictionOnTop *b = OnTopDetails(*unit, nullptr);
	if (b && b->ReplaceOnBuild) {
		auto &unitCache = Map.Field(pos)->UnitCache;
		auto it = ranges::find_if(unitCache, HasSameTypeAs(*b->Parent));

		if (it != unitCache.end()) {
			CUnit &replacedUnit = **it;
			unit->ResourcesHeld = replacedUnit.ResourcesHeld; // We capture the value of what is beneath.
			unit->Variable[GIVERESOURCE_INDEX].Value = replacedUnit.Variable[GIVERESOURCE_INDEX].Value;
			unit->Variable[GIVERESOURCE_INDEX].Max = replacedUnit.Variable[GIVERESOURCE_INDEX].Max;
			unit->Variable[GIVERESOURCE_INDEX].Enable = replacedUnit.Variable[GIVERESOURCE_INDEX].Enable;
			replacedUnit.Remove(nullptr); // Destroy building beneath
			UnitLost(replacedUnit);
			UnitClearOrders(replacedUnit);
			replacedUnit.Release();
		}
	}
	if (unit != nullptr) {
		if (type.GivesResource) {
			if (type.StartingResources != 0) {
				unit->ResourcesHeld = type.StartingResources;
				unit->Variable[GIVERESOURCE_INDEX].Value = type.StartingResources;
				unit->Variable[GIVERESOURCE_INDEX].Max = type.StartingResources;
			} else {
				unit->ResourcesHeld = DefaultResourceAmounts[type.GivesResource];
				unit->Variable[GIVERESOURCE_INDEX].Value = DefaultResourceAmounts[type.GivesResource];
				unit->Variable[GIVERESOURCE_INDEX].Max = DefaultResourceAmounts[type.GivesResource];
			}
			unit->Variable[GIVERESOURCE_INDEX].Enable = 1;
		}
	} else {
		DebugPrint("Unable to allocate Unit");
	}
	UpdateMinimap = true;
}

/**
**  Edit unit.
**
**  @param pos     map tile coordinate.
**  @param type    Unit type to edit.
**  @param player  Player owning the unit.
*/
static void EditorPlaceUnit(const Vec2i &pos, CUnitType &type, CPlayer *player)
{
	EditorAction editorAction;
	editorAction.Type = EditorActionType::PlaceUnit;
	editorAction.tilePos = pos;
	editorAction.UnitType = &type;
	editorAction.Player = player;

	EditorActionPlaceUnit(pos, type, player);
	EditorAddUndoAction(editorAction);
}

/**
**  Remove a unit
*/
static void EditorActionRemoveUnit(CUnit &unit)
{
	unit.Remove(nullptr);
	UnitLost(unit);
	UnitClearOrders(unit);
	unit.Release();
	UI.StatusLine.Set(_("Unit deleted"));
	UpdateMinimap = true;
}

/**
**  Remove a unit
*/
static void EditorRemoveUnit(CUnit &unit)
{
	EditorAction editorAction;
	editorAction.Type = EditorActionType::RemoveUnit;
	editorAction.tilePos = unit.tilePos;
	editorAction.UnitType = unit.Type;
	editorAction.Player = unit.Player;

	EditorActionRemoveUnit(unit);
	EditorAddUndoAction(editorAction);
}

/*----------------------------------------------------------------------------
--  Undo/Redo
----------------------------------------------------------------------------*/

static void EditorUndoAction()
{
	if (EditorUndoActions.empty()) {
		return;
	}

	EditorAction action = EditorUndoActions.back();
	EditorUndoActions.pop_back();

	switch (action.Type) {
		case EditorActionType::PlaceUnit: {
			CUnit *unit = UnitOnMapTile(action.tilePos, action.UnitType->UnitType);
			EditorActionRemoveUnit(*unit);
			break;
		}

		case EditorActionType::RemoveUnit:
			EditorActionPlaceUnit(action.tilePos, *action.UnitType, action.Player);
			break;
	}
	EditorRedoActions.push_back(action);
}

static void EditorRedoAction()
{
	if (EditorRedoActions.empty()) {
		return;
	}
	EditorAction action = EditorRedoActions.back();
	EditorRedoActions.pop_back();

	switch (action.Type) {
		case EditorActionType::PlaceUnit:
			EditorActionPlaceUnit(action.tilePos, *action.UnitType, action.Player);
			break;

		case EditorActionType::RemoveUnit: {
			CUnit *unit = UnitOnMapTile(action.tilePos, action.UnitType->UnitType);
			EditorActionRemoveUnit(*unit);
			break;
		}
	}
	EditorUndoActions.push_back(action);
}

static void EditorAddUndoAction(EditorAction action)
{
	EditorRedoActions.clear();
	EditorUndoActions.push_back(action);
}

/*----------------------------------------------------------------------------
--  Other
----------------------------------------------------------------------------*/

/**
**  Calculate the max height and the max width of icons,
**  and assign them to IconHeight and IconWidth
*/
static void CalculateMaxIconSize()
{
	IconWidth = 0;
	IconHeight = 0;
	for (const auto &typeStr : Editor.UnitTypes) {
		if (starts_with(typeStr, "unit-")) {
			const CUnitType &type = UnitTypeByIdent(typeStr);
			if (type.Icon.Icon) {
				const CIcon &icon = *type.Icon.Icon;

				IconWidth = std::max(IconWidth, icon.G->Width);
				IconHeight = std::max(IconHeight, icon.G->Height);
			}
		}
	}
}

/**
**  Recalculate the shown units.
*/
static void RecalculateShownUnits(size_t start = 0, size_t stop = INT_MAX)
{
	Editor.ShownUnitTypes.clear();

	for (size_t i = start; i < Editor.UnitTypes.size() && i < stop; i++) {
		if (starts_with(Editor.UnitTypes[i], "unit-")) {
			Editor.ShownUnitTypes.push_back(&UnitTypeByIdent(Editor.UnitTypes[i]));
		} else {
			Editor.ShownUnitTypes.push_back(nullptr);
		}
	}

	if (Editor.UnitIndex >= (int)Editor.ShownUnitTypes.size()) {
		Editor.UnitIndex = Editor.ShownUnitTypes.size() / VisibleUnitIcons * VisibleUnitIcons;
	}
	// Quick & dirty make them invalid
	Editor.CursorUnitIndex = -1;
	Editor.SelectedUnitIndex = -1;
}

/*----------------------------------------------------------------------------
--  Display
----------------------------------------------------------------------------*/

static int getPlayerButtonSize() {
	static int sz = GetGameFont().getHeight() + 2;
	return sz;
}

// area where normally the selected unit(s) is/are shown
static std::vector<int> getSelectionArea() {
	static bool cached = false;
	static std::vector<int> playerButtonArea;
	if (!cached) {
		if (UI.EditorSettingsAreaBottomRight.x != 0 && UI.EditorSettingsAreaBottomRight.y != 0) {
			// we have some explicit dimensions
			playerButtonArea = {UI.EditorSettingsAreaTopLeft.x, UI.EditorSettingsAreaTopLeft.y,
								UI.EditorSettingsAreaBottomRight.x, UI.EditorSettingsAreaBottomRight.y};
			cached = true;
		} else {
			int x, y, x2, y2;
			x = y = INT_MAX;
			x2 = y2 = 0;
			for (auto btn : UI.SelectedButtons) {
				x = std::min(x, btn.X);
				y = std::min(y, btn.Y);
				x2 = std::max(x2, btn.X + btn.Style->Width);
				y2 = std::max(y2, btn.Y + btn.Style->Height);
			}
			playerButtonArea = {x, y, x2, y2};
			cached = true;
		}
	}
	return playerButtonArea;
}

// area where normally the action buttons for the selected unit(s) are shown
static std::vector<int> getButtonArea() {
	static bool cached = false;
	static std::vector<int> buttonArea;
	if (!cached) {
		if (UI.EditorButtonAreaBottomRight.x != 0 && UI.EditorButtonAreaBottomRight.y != 0) {
			// we have some explicit dimensions
			buttonArea = {UI.EditorButtonAreaTopLeft.x, UI.EditorButtonAreaTopLeft.y,
						  UI.EditorButtonAreaBottomRight.x, UI.EditorButtonAreaBottomRight.y};
			cached = true;
		} else {
			int x, y;
			x = y = INT_MAX;
			int x2, y2;
			x2 = y2 = 0;
			for (auto btn : UI.ButtonPanel.Buttons) {
				x = std::min(x, btn.X);
				y = std::min(y, btn.Y);
				x2 = std::max(x2, btn.X + btn.Style->Width);
				y2 = std::max(y2, btn.Y + btn.Style->Height);
			}
			buttonArea = {x, y, x2, y2};
			cached = true;
		}
	}
	return buttonArea;
}

/**
 * Call the forEach callback with each player icon's <playerNum,x,y,w,h>. Return false to cancel iteration.
 *
 * Returns the last value returned by forEach. This can be used to detect if an early cancellation of the
 * iteration was requested.
 */
static bool forEachPlayerSelectionBoxArea(std::function<bool(int, int, int, int, int)> forEach) {
	int x = getSelectionArea()[0];
	int y = getSelectionArea()[1];
	int x2 = getSelectionArea()[2];
	int y2 = getSelectionArea()[3];
	int maxX = x2 - getPlayerButtonSize();
	int maxY = y2 - getPlayerButtonSize();

	for (int i = 0; i < PlayerMax; i++, x += getPlayerButtonSize()) {
		if (x > maxX) {
			x = getSelectionArea()[0];
			y += getPlayerButtonSize();
		}
		if (y > maxY) { /* FIXME: nothing we can do? */ }

		if (!forEach(i, x, y, getPlayerButtonSize(), getPlayerButtonSize())) {
			return false;
		}
	}
	return true;
};

/**
**  Draw a table with the players
*/
static void DrawPlayers()
{
	forEachPlayerSelectionBoxArea([](int i, int x, int y, int w, int h) {
		// draw highlight
		if (i == Editor.CursorPlayer && Map.Info.PlayerType[i] != PlayerTypes::PlayerNobody) {
			Video.DrawRectangle(ColorWhite, x, y, getPlayerButtonSize(), getPlayerButtonSize());
		}

		// draw border 1px inside highlight
		Video.DrawRectangle(
			i == Editor.CursorPlayer && Map.Info.PlayerType[i] != PlayerTypes::PlayerNobody ? ColorWhite : ColorGray,
			x + 1, y + 1, getPlayerButtonSize() - 1, getPlayerButtonSize() - 1);

		// if player exists, draw player color 2px inside highlight
		if (Map.Info.PlayerType[i] != PlayerTypes::PlayerNobody) {
			Video.FillRectangle(PlayerColorsRGB[GameSettings.Presets[i].PlayerColor][0],
			                    x + 2,
			                    y + 2,
			                    getPlayerButtonSize() - 2,
			                    getPlayerButtonSize() - 2);
		}

		// Draw green rectangle around selected player
		if (i == Editor.SelectedPlayer) {
			Video.DrawRectangle(ColorGreen, x + 1, y + 1, getPlayerButtonSize() - 1, getPlayerButtonSize() - 1);
		}

		char buf[256];
		sprintf(buf, "%d", i + 1);
		CLabel label(GetGameFont());
		label.DrawCentered(x + getPlayerButtonSize() / 2, y + 3, buf);

		return true;
	});
}

/**
 * Call the forEach callback with each unit icon's <EditorUnitIndex,ButtonStyle,x,y,w,h>. Return false to cancel iteration;
 *
 * Returns the last value returned by forEach. This can be used to detect if an early cancellation of the
 * iteration was requested.
 */
static bool forEachUnitIconArea(std::function<bool(int,ButtonStyle*,int,int,int,int)> forEach) {
	int x1 = getButtonArea()[0];
	int y1 = getButtonArea()[1];
	int x2 = getButtonArea()[2];
	int y2 = getButtonArea()[3];

	int iconW = UI.ButtonPanel.Buttons[0].Style->Width + 2;
	int iconH = UI.ButtonPanel.Buttons[0].Style->Height + 2;
	int maxX = x2 - iconW;
	int maxY = y2 - iconH;

	// initialize on the first draw how many tile icons we can actually draw
	if (VisibleUnitIcons == 0) {
		int horizCnt = (x2 - x1) / iconW;
		int vertCnt = (y2 - y1) / iconH;
		VisibleUnitIcons = horizCnt * vertCnt;
	}

	int i = Editor.UnitIndex;
	Assert(Editor.UnitIndex != -1);

	int y = y1;
	while (y < maxY) {
		if (i >= (int)Editor.ShownUnitTypes.size()) {
			break;
		}
		int x = x1;
		while (x < maxX) {
			if (i >= (int) Editor.ShownUnitTypes.size()) {
				break;
			}
			if (!forEach(i, UI.ButtonPanel.Buttons[0].Style, x, y, iconW, iconH)) {
				return false;
			}
			x += iconW;
			++i;
		}
		y += iconH;
	}

	return true;
}

/**
**  Draw unit icons into button area
*/
static void DrawUnitIcons()
{
	forEachUnitIconArea([](int i, ButtonStyle *style, int x, int y, int w, int h) {
		if (Editor.ShownUnitTypes[i] == nullptr) {
			return true;
		}
		CIcon &icon = *Editor.ShownUnitTypes[i]->Icon.Icon;
		const PixelPos pos(x, y);
		unsigned int flag = 0;
		if (i == Editor.CursorUnitIndex) {
			flag = IconActive;
			if (MouseButtons & LeftButton) {
				// Overwrite IconActive.
				flag = IconClicked;
			}
		}
		icon.DrawUnitIcon(*style, flag, pos, "", Players[Editor.SelectedPlayer].Index);
		Video.DrawRectangleClip(ColorGray, x, y, w, h);
		if (i == Editor.SelectedUnitIndex) {
			Video.DrawRectangleClip(ColorGreen, x + 1, y + 1,
									w - 2, h - 2);
		}
		if (i == Editor.CursorUnitIndex) {
			Video.DrawRectangleClip(ColorWhite, x - 1, y - 1,
									w + 2, h + 2);
			Editor.PopUpX = x;
			Editor.PopUpY = y;
		}
		return true;
	});
}

/**
 * Call the forEach callback with each tile option buttons's <is-active boolean,label string,i,x,y,w,h>.
 * Return false to cancel iteration.
 *
 * Returns the last value returned by forEach. This can be used to detect if an early cancellation of the
 * iteration was requested.
 */
static bool forEachTileOptionArea(std::function<bool(bool,std::string&,int,int,int,int,int)> forEach) {
	int x1 = getSelectionArea()[0];
	int y1 = getSelectionArea()[1];
	int x2 = getSelectionArea()[2];
	int y2 = getSelectionArea()[3];

	int labelHeight = GetGameFont().getHeight() + 1;
	int labelMaxW = x2 - x1;
	int labelX = x1;
	int labelY = y1;

	std::vector<std::pair<bool, std::string>> compactOptions = {
		{ TileCursorSize == 1, "1x1" },
		{ TileCursorSize == 2, "2x2" },
		{ TileCursorSize == 3, "3x3" },
		{ TileCursorSize == 4, "4x4" },
		{ TileCursorSize == 5, "5x5" },
		{ TileCursorSize == 10, "10x10" }
	};

	std::vector<std::pair<bool, std::string>> options = {
		{ TileToolRandom != 0, "Random" },
		{ TileToolDecoration != 0, "Filler" },
		{ TileToolNoFixup != 0, "Manual" }
	};

	int i = 0;
	int compactX = labelX;
	while ((size_t)i < compactOptions.size()) {
		auto opt = compactOptions[i];
		if (!forEach(opt.first, opt.second, i, compactX, labelY, labelMaxW / 2, labelHeight)) {
			return false;
		}
		if (i % 2 == 0) {
			compactX = labelX + labelMaxW / 2;
		} else {
			compactX = labelX;
			labelY += labelHeight;
		}
		i++;
	}
	for (auto opt : options) {
		if (!forEach(opt.first, opt.second, i, labelX, labelY, labelMaxW, labelHeight)) {
			return false;
		}
		labelY += labelHeight;
		i++;
	}

	return true;
}

static void DrawTileOptions() {
	forEachTileOptionArea([](bool active, std::string str, int i, int x, int y, int w, int h) {
		CLabel label(GetGameFont());
		if (active) {
			label.DrawReverseCentered(x + w / 2, y, str);
		} else {
			label.DrawCentered(x + w / 2, y, str);
		}

		if (ButtonUnderCursor == i + 300) {
			Video.DrawRectangle(ColorGray, x - 1, y - 1, w + 2, h + 2);
		}

		return true;
	});
}

/**
 * Call the forEach callback with each tile's <EditorTileIndex,x,y,w,h>. Return false to cancel iteration.
 *
 * Returns the last value returned by forEach. This can be used to detect if an early cancellation of the
 * iteration was requested.
 */
static bool forEachTileIconArea(std::function<bool(int,int,int,int,int)> forEach) {
	int x1 = getButtonArea()[0];
	int y1 = getButtonArea()[1];
	int x2 = getButtonArea()[2];
	int y2 = getButtonArea()[3];

	int tileW = Map.Tileset->getPixelTileSize().x + 1;
	int tileH = Map.Tileset->getPixelTileSize().y + 1;
	int maxX = x2 - tileW;
	int maxY = y2 - tileH;

	int i = Editor.TileIndex;
	Assert(Editor.TileIndex != -1);

	if (VisibleTileIcons == 0) {
		// initialize on the first draw how many tile icons we can actually draw
		int horizCnt = (x2 - x1) / tileW;
		int vertCnt = (y2 - y1) / tileH;
		VisibleTileIcons = horizCnt * vertCnt;
	}

	int y = y1;
	while (y < maxY) {
		if (i >= (int)Editor.ShownTileTypes.size()) {
			break;
		}
		int x = x1;
		while (x < maxX) {
			if (i >= (int) Editor.ShownTileTypes.size()) {
				break;
			}
			if (!forEach(i, x, y, tileW, tileH)) {
				return false;
			}
			x += tileW;
			++i;
		}
		y += tileH;
	}

	return true;
}

/**
**  Draw tile icons.
*/
static void DrawTileIcons()
{
	forEachTileIconArea([](int i, int x, int y, int w, int h) {
		const unsigned int tile = Editor.ShownTileTypes[i];

		Map.TileGraphic->DrawFrameClip(tile, x, y);
		Video.DrawRectangleClip(ColorGray, x, y, Map.Tileset->getPixelTileSize().x, Map.Tileset->getPixelTileSize().y);

		if (i == Editor.SelectedTileIndex) {
			Video.DrawRectangleClip(ColorGreen, x + 1, y + 1,
									Map.Tileset->getPixelTileSize().x - 2, Map.Tileset->getPixelTileSize().y - 2);
		}
		if (i == Editor.CursorTileIndex) {
			Video.DrawRectangleClip(ColorWhite, x - 1, y - 1,
									Map.Tileset->getPixelTileSize().x + 2, Map.Tileset->getPixelTileSize().y + 2);
			Editor.PopUpX = x;
			Editor.PopUpY = y;
		}

		return true;
	});
}

static void DrawIntoSelectionArea()
{
	switch (Editor.State) {
		case EditorStateType::SetStartLocation:
		case EditorStateType::EditUnit:
			DrawPlayers();
			break;
		case EditorStateType::EditTile:
			DrawTileOptions();
			break;
		default:
			break;
	}
}

static void DrawIntoButtonArea()
{
	switch (Editor.State) {
		case EditorStateType::EditTile:
			DrawTileIcons();
			break;
		case EditorStateType::EditUnit:
			DrawUnitIcons(); // this draws directly into the UI.ButtonPanel.Buttons
			break;
		default:
			break;
	}
}

static void DrawInfoHighlightedOverlay()
{
	if (overlaysDropdown->getSelected() == EditorOverlays::cElevation) {
		CLabel(GetGameFont())
			.Draw(overlaysDropdown->getX() + overlaysDropdown->getWidth() + 5,
		          2,
		          Editor.HighlightElevationLevel);
	}
}

/**
**  Draw the editor panels.
*/
static void DrawEditorPanel()
{
	DrawIntoSelectionArea();
	DrawIntoButtonArea();
	DrawInfoHighlightedOverlay();
}

/**
**  Draw special cursor on map.
**
**  @todo support for bigger cursors (2x2, 3x3 ...)
*/
static void DrawMapCursor()
{
	//  Affect CursorBuilding if necessary.
	//  (Menu reset CursorBuilding)
	if (!CursorBuilding) {
		switch (Editor.State) {
			case EditorStateType::Selecting:
			case EditorStateType::EditTile:
				break;
			case EditorStateType::EditUnit:
				if (Editor.SelectedUnitIndex != -1) {
					CursorBuilding = const_cast<CUnitType *>(Editor.ShownUnitTypes[Editor.SelectedUnitIndex]);
				}
				break;
			case EditorStateType::SetStartLocation:
				if (Editor.StartUnit) {
					CursorBuilding = const_cast<CUnitType *>(Editor.StartUnit);
				}
				break;
		}
	}

	// Draw map cursor
	if (UI.MouseViewport && !CursorBuilding) {
		const Vec2i tilePos = UI.MouseViewport->ScreenToTilePos(CursorScreenPos);
		const PixelPos screenPos = UI.MouseViewport->TilePosToScreen_TopLeft(tilePos);

		if (Editor.State == EditorStateType::EditTile && Editor.SelectedTileIndex != -1) {
			const unsigned short tile = Editor.ShownTileTypes[Editor.SelectedTileIndex];
			PushClipping();
			UI.MouseViewport->SetClipping();

			PixelPos screenPosIt;
			for (int j = 0; j < TileCursorSize; ++j) {
				screenPosIt.y = screenPos.y + j * Map.Tileset->getPixelTileSize().y;
				if (screenPosIt.y >= UI.MouseViewport->GetBottomRightPos().y) {
					break;
				}
				for (int i = 0; i < TileCursorSize; ++i) {
					screenPosIt.x = screenPos.x + i * Map.Tileset->getPixelTileSize().x;
					if (screenPosIt.x >= UI.MouseViewport->GetBottomRightPos().x) {
						break;
					}
					Map.TileGraphic->DrawFrameClip(tile, screenPosIt.x, screenPosIt.y);
				}
			}
			Video.DrawRectangleClip(ColorWhite, screenPos.x, screenPos.y, Map.Tileset->getPixelTileSize().x * TileCursorSize, Map.Tileset->getPixelTileSize().y * TileCursorSize);
			PopClipping();
		} else {
			PushClipping();
			UI.MouseViewport->SetClipping();
			Video.DrawRectangleClip(ColorWhite, screenPos.x, screenPos.y, Map.Tileset->getPixelTileSize().x, Map.Tileset->getPixelTileSize().y);
			if (Editor.State == EditorStateType::ElevationLevel) {
				CLabel(GetGameFont()).DrawClip(screenPos.x + 2, screenPos.y + 1, Editor.SelectedElevationLevel);
			}
			PopClipping();
		}
	}
}

static void DrawCross(const PixelPos &topleft_pos, const PixelSize &size, Uint32 color)
{
	const PixelPos lt = topleft_pos;
	const PixelPos lb(topleft_pos.x, topleft_pos.y + size.y);
	const PixelPos rt(topleft_pos.x + size.x, topleft_pos.y);
	const PixelPos rb = topleft_pos + size;

	Video.DrawLineClip(color, lt, rb);
	Video.DrawLineClip(color, lb, rt);
}

/**
**  Draw the start locations of all active players on the map
*/
static void DrawStartLocations()
{
	const CUnitType *type = Editor.StartUnit;
	for (const CViewport *vp = UI.Viewports; vp < UI.Viewports + UI.NumViewports; ++vp) {
		PushClipping();
		vp->SetClipping();

		for (int i = 0; i < PlayerMax; i++) {
			if (Map.Info.PlayerType[i] != PlayerTypes::PlayerNobody && Map.Info.PlayerType[i] != PlayerTypes::PlayerNeutral) {
				const PixelPos startScreenPos = vp->TilePosToScreen_TopLeft(Players[i].StartPos);

				if (type) {
#ifdef DYNAMIC_LOAD
					if (!type->Sprite) {
						LoadUnitTypeSprite(*const_cast<CUnitType *>(type));
					}
#endif
					DrawUnitType(*type, type->Sprite, i, 0, startScreenPos);
				} else { // Draw a cross
					DrawCross(startScreenPos, Map.Tileset->getPixelTileSize(), Players[i].Color);
				}
			}
		}
		PopClipping();
	}
}

/**
**  Draw editor info.
**
**  If cursor is on map or minimap show information about the current tile.
*/
static void DrawEditorInfo()
{
#if 1
	Vec2i pos(0, 0);

	if (UI.MouseViewport) {
		pos = UI.MouseViewport->ScreenToTilePos(CursorScreenPos);
	}

	char buf[256];
	snprintf(buf, sizeof(buf), _("Editor (%d %d)"), pos.x, pos.y);
	CLabel(GetGameFont()).Draw(UI.StatusLine.TextX, UI.StatusLine.TextY - GetGameFont().getHeight() * 3, buf);
	const CMapField &mf = *Map.Field(pos);
	//
	// Flags info
	//
	const unsigned flag = mf.getFlag();
	sprintf(buf, "elev:(%u) value:(0x%02X) | flags:(0x%04X)>[%c%c%c%c%c%c%c%c%c%c%c%c%c%c]",
			mf.getElevation(),
			mf.Value, flag,
			flag & MapFieldOpaque       ? 'o' : '-',
			flag & MapFieldUnpassable   ? 'u' : '-',
			flag & MapFieldNoBuilding   ? 'n' : '-',
			flag & MapFieldHuman        ? 'h' : '-',
			flag & MapFieldWall         ? 'w' : '-',
			flag & MapFieldRocks        ? 'r' : '-',
			flag & MapFieldForest       ? 'f' : '-',
			flag & MapFieldLandAllowed  ? 'L' : '-',
			flag & MapFieldCoastAllowed ? 'C' : '-',
			flag & MapFieldWaterAllowed ? 'W' : '-',
			flag & MapFieldLandUnit     ? 'l' : '-',
			flag & MapFieldAirUnit      ? 'a' : '-',
			flag & MapFieldSeaUnit      ? 's' : '-',
			flag & MapFieldBuilding     ? 'b' : '-');
	CLabel(GetGameFont()).Draw(UI.StatusLine.TextX, UI.StatusLine.TextY - GetGameFont().getHeight() * 2, buf);

	// Tile info
	const CTileset &tileset = *Map.Tileset;
	const int index = tileset.findTileIndexByTile(mf.getGraphicTile());
	Assert(index != -1);
	const terrain_typeIdx baseTerrainIdx = tileset.tiles[index].tileinfo.BaseTerrain;
	const char *baseTerrainStr = tileset.getTerrainName(baseTerrainIdx).c_str();
	const terrain_typeIdx mixTerrainIdx = tileset.tiles[index].tileinfo.MixTerrain;
	const char *mixTerrainStr = mixTerrainIdx ? tileset.getTerrainName(mixTerrainIdx).c_str() : "";
	snprintf(buf, sizeof(buf), "%s %s", baseTerrainStr, mixTerrainStr);
	CLabel(GetGameFont()).Draw(UI.StatusLine.TextX, UI.StatusLine.TextY - GetGameFont().getHeight(), buf);
#endif
}

/**
**  Show info about unit.
**
**  @param unit  Unit pointer.
*/
static void ShowUnitInfo(const CUnit &unit)
{
	char buf[256];

	int n = sprintf(buf, _("#%d '%s' Player:#%d %s"), UnitNumber(unit),
					unit.Type->Name.c_str(), unit.Player->Index + 1,
					unit.Active ? "active" : "passive");
	if (unit.Type->GivesResource) {
		sprintf(buf + n, _(" Amount %d"), unit.ResourcesHeld);
	}
	UI.StatusLine.Set(buf);
}

/**
**  Update editor display.
*/
void EditorUpdateDisplay()
{
	ColorCycle();

	DrawMapArea(Editor.OverlayHighlighter); // draw the map area

	DrawStartLocations();

	// Fillers
	for (auto& filler : UI.Fillers) {
		filler.G->DrawClip(filler.X, filler.Y);
	}

	if (CursorOn == ECursorOn::Map && Gui->getTop() == editorContainer.get() && !GamePaused) {
		DrawMapCursor(); // cursor on map
	}

	// Menu button
	const int flag_active = ButtonAreaUnderCursor == ButtonArea::Menu
							&& ButtonUnderCursor == ButtonUnderMenu ? MI_FLAGS_ACTIVE : 0;
	const int flag_clicked = GameMenuButtonClicked ? MI_FLAGS_CLICKED : 0;
	DrawUIButton(UI.MenuButton.Style,
				 flag_active | flag_clicked,
				 UI.MenuButton.X, UI.MenuButton.Y,
				 UI.MenuButton.Text);


	// Minimap
	if (UI.SelectedViewport) {
		UI.Minimap.Draw();
		for (std::size_t i = 0; i != UI.NumViewports; ++i) {
			UI.Minimap.DrawViewportArea(UI.Viewports[i],
			                            UI.SelectedViewport == &UI.Viewports[i] ? 255 : 128);
		}
	}
	// Info panel
	if (UI.InfoPanel.G) {
		UI.InfoPanel.G->DrawClip(UI.InfoPanel.X, UI.InfoPanel.Y);
	}
	// Button panel
	if (UI.ButtonPanel.G) {
		UI.ButtonPanel.G->DrawClip(UI.ButtonPanel.X, UI.ButtonPanel.Y);
	}
	DrawEditorPanel();

	if (CursorOn == ECursorOn::Map) {
		DrawEditorInfo();
	}

	// Status line
	UI.StatusLine.Draw();

	DrawGuichanWidgets();

	DrawCursor();

	// refresh entire screen, so no further invalidate needed
	Invalidate();
	RealizeVideoMemory();
}

/*----------------------------------------------------------------------------
--  Highlight layers
----------------------------------------------------------------------------*/
static inline bool OverlayElevation(const CMapField &mapField)
{
	return mapField.getElevation() == Editor.HighlightElevationLevel;
}

static inline bool OverlayUnpassable(const CMapField &mapField)
{
	return mapField.getFlag() & MapFieldUnpassable;
}

static inline bool OverlayNoBuildingAllowed(const CMapField &mapField)
{
	return mapField.getFlag() & MapFieldNoBuilding;
}

static inline bool OverlayOpaque(const CMapField &mapField)
{
	return mapField.isOpaque();
}

/*----------------------------------------------------------------------------
--  Input / Keyboard / Mouse
----------------------------------------------------------------------------*/

/**
**  Callback for input.
*/
static void EditorCallbackButtonUp(unsigned button)
{
	if (GameCursor == UI.Scroll.Cursor) {
		// Move map.
		GameCursor = UI.Point.Cursor; // Reset
		return;
	}

	if ((1 << button) == LeftButton && GameMenuButtonClicked) {
		GameMenuButtonClicked = false;
		if (ButtonUnderCursor == ButtonUnderMenu) {
			if (UI.MenuButton.Callback) {
				UI.MenuButton.Callback->action("");
			}
		}
	}
	if ((1 << button) == LeftButton) {
		UnitPlacedThisPress = false;
	}

	if (CursorState == CursorStates::Rectangle && !(MouseButtons & LeftButton)) { // leave select mode
		PixelPos pos0 = CursorStartMapPos;
		const PixelPos cursorMapPos = UI.MouseViewport->ScreenToMapPixelPos(CursorScreenPos);
		PixelPos pos1 = cursorMapPos;
		if (pos0.x > pos1.x) {
			std::swap(pos0.x, pos1.x);
		}
		if (pos0.y > pos1.y) {
			std::swap(pos0.y, pos1.y);
		}

		const Vec2i t0 = Map.MapPixelPosToTilePos(pos0);
		const Vec2i t1 = Map.MapPixelPosToTilePos(pos1);
		std::vector<CUnit *> table = Select(t0, t1);

		if (!(KeyModifiers & ModifierShift)) {
			UnSelectAll();
		}
		for (auto u : table) {
			SelectUnit(*u);
		}
		CursorStartScreenPos.x = 0;
		CursorStartScreenPos.y = 0;
		GameCursor = UI.Point.Cursor;
		CursorState = CursorStates::Point;
	}
}

/**
**  Called if mouse button pressed down.
**
**  @param button  Mouse button number (0 left, 1 middle, 2 right)
*/
static void EditorCallbackButtonDown(unsigned button)
{
	if (GamePaused) {
		return;
	}
	if ((button >> MouseHoldShift) != 0) {
		// Ignore repeated events when holding down a button
		return;
	}
	// Click on menu button
	if (CursorOn == ECursorOn::Button && ButtonAreaUnderCursor == ButtonArea::Menu &&
		(MouseButtons & LeftButton) && !GameMenuButtonClicked) {
		PlayGameSound(GameSounds.Click.Sound, MaxSampleVolume);
		GameMenuButtonClicked = true;
		return;
	}
	// Click on minimap
	if (CursorOn == ECursorOn::Minimap) {
		if (MouseButtons & LeftButton) { // enter move mini-mode
			const Vec2i tilePos = UI.Minimap.ScreenToTilePos(CursorScreenPos);
			UI.SelectedViewport->Center(Map.TilePosToMapPixelPos_Center(tilePos));
		}
		return;
	}
	// Click on tile area
	if (Editor.State == EditorStateType::EditTile) {
		if (CursorOn == ECursorOn::Button && ButtonUnderCursor >= 100) {
			switch (ButtonUnderCursor) {
				case 300: TileCursorSize = 1; return;
				case 301: TileCursorSize = 2; return;
				case 302: TileCursorSize = 3; return;
				case 303: TileCursorSize = 4; return;
				case 304: TileCursorSize = 5; return;
				case 305: TileCursorSize = 10; return;
				case 306: TileToolRandom ^= 1; return;
				case 307: TileToolDecoration ^= 1; return;
			    case 308: {
					TileToolNoFixup = !TileToolNoFixup;
					// switch the selected tiles
					if (TileToolNoFixup) {
						Editor.ShownTileTypes.clear();
						Editor.SelectedTileIndex = -1;

						tile_index index = 0;
						for (auto &currTile : Map.Tileset->tiles) {
							if (currTile.tile) {
								Editor.ShownTileTypes.push_back(currTile.tile);
							}
							index++;
						}
					} else {
						Editor.ShownTileTypes.clear();
						Editor.SelectedTileIndex = -1;
						Map.Tileset->fillSolidTiles(&Editor.ShownTileTypes);
					}
					return;
				}
			}
		}

		if (MouseButtons & RightButton) {
			Editor.SelectedTileIndex = -1;
			return;
		} else {
			if (Editor.CursorTileIndex != -1) {
				Editor.SelectedTileIndex = Editor.CursorTileIndex;
				return;
			}
		}
	}

	// Click on player area
	if (Editor.State == EditorStateType::EditUnit || Editor.State == EditorStateType::SetStartLocation) {
		// Cursor on player icons
		if (Editor.CursorPlayer != -1) {
			if (Map.Info.PlayerType[Editor.CursorPlayer] != PlayerTypes::PlayerNobody) {
				Editor.SelectedPlayer = Editor.CursorPlayer;
				ThisPlayer = Players + Editor.SelectedPlayer;
			}
			return;
		}
	}

	// Click on unit area
	if (Editor.State == EditorStateType::EditUnit) {
		// Cursor on unit icons
		if (Editor.CursorUnitIndex != -1) {
			if (MouseButtons & LeftButton) {
				Editor.SelectedUnitIndex = Editor.CursorUnitIndex;
				CursorBuilding = const_cast<CUnitType *>(Editor.ShownUnitTypes[Editor.CursorUnitIndex]);
				return;
			} else if (MouseButtons & RightButton) {
				lua_getglobal(Lua, "EditUnitTypeProperties"); // function to be called
				if (lua_isfunction(Lua, -1) == 1) {
					lua_pushstring(Lua, Editor.ShownUnitTypes[Editor.CursorUnitIndex]->Ident.c_str());
					LuaCall(1, 1, false);
					Editor.CursorUnitIndex = -1;
				} else {
					lua_pop(Lua, 1);
				}
				return;
			}
		}
	}

	// Right click on a resource
	if (Editor.State == EditorStateType::Selecting) {
		if ((MouseButtons & RightButton) && (UnitUnderCursor != nullptr || !Selected.empty())) {
			lua_getglobal(Lua, "EditUnitProperties");
			if (lua_isfunction(Lua, -1) == 1) {
				lua_newtable(Lua);
				int n = 0;
				if (!Selected.empty()) {
					for (auto u : Selected) {
						lua_pushnumber(Lua, UnitNumber(*u));
						lua_rawseti(Lua, -2, ++n);
					}
				} else {
					lua_pushnumber(Lua, UnitNumber(*UnitUnderCursor));
					lua_rawseti(Lua, -2, ++n);
				}
				LuaCall(1, 1, false);
			} else {
				lua_pop(Lua, 1);
			}
			return;
		}
	}

	// Click on map area
	if (CursorOn == ECursorOn::Map) {
		if (MouseButtons & RightButton) {
			if (Editor.State == EditorStateType::EditUnit && Editor.SelectedUnitIndex != -1) {
				Editor.SelectedUnitIndex = -1;
				CursorBuilding = nullptr;
				return;
			} else if (Editor.State == EditorStateType::EditTile && Editor.SelectedTileIndex != -1) {
				Editor.SelectedTileIndex = -1;
				CursorBuilding = nullptr;
				return;
			}
		}

		CViewport *vp = GetViewport(CursorScreenPos);
		Assert(vp);
		if ((MouseButtons & LeftButton) && UI.SelectedViewport != vp) {
			// viewport changed
			UI.SelectedViewport = vp;
		}
		if (MouseButtons & LeftButton) {
			const Vec2i tilePos = UI.MouseViewport->ScreenToTilePos(CursorScreenPos);

			if (Editor.State == EditorStateType::EditTile && (Editor.SelectedTileIndex != -1 || (KeyModifiers & ModifierAlt))) {
				EditTiles(tilePos, Editor.SelectedTileIndex != -1 ? Editor.ShownTileTypes[Editor.SelectedTileIndex] : -1, TileCursorSize);
			} else if (Editor.State == EditorStateType::EditUnit) {
				if (!UnitPlacedThisPress && CursorBuilding) {
					if (CanBuildUnitType(nullptr, *CursorBuilding, tilePos, 1)) {
						PlayGameSound(GameSounds.PlacementSuccess[ThisPlayer->Race].Sound,
									  MaxSampleVolume);
						EditorPlaceUnit(tilePos, *CursorBuilding, Players + Editor.SelectedPlayer);
						UnitPlacedThisPress = true;
						UI.StatusLine.Clear();
					} else {
						UI.StatusLine.Set(_("Unit cannot be placed here."));
						PlayGameSound(GameSounds.PlacementError[ThisPlayer->Race].Sound,
									  MaxSampleVolume);
					}
				}
			} else if (Editor.State == EditorStateType::SetStartLocation) {
				Players[Editor.SelectedPlayer].StartPos = tilePos;
			} else if (Editor.State == EditorStateType::Selecting) {
				CursorStartScreenPos = CursorScreenPos;
				CursorStartMapPos = UI.MouseViewport->ScreenToMapPixelPos(CursorScreenPos);
				GameCursor = UI.Cross.Cursor;
				CursorState = CursorStates::Rectangle;
			} else if (Editor.State == EditorStateType::ElevationLevel) {
				EditorSetElevationLevel(tilePos, Editor.SelectedElevationLevel);
			}
		} else if (MouseButtons & MiddleButton) {
			// enter move map mode
			CursorStartScreenPos = CursorScreenPos;
			GameCursor = UI.Scroll.Cursor;
		}
	}
}

/**
**  Handle key down.
**
**  @param key      Key scancode.
**  @param keychar  Character code.
*/
static void EditorCallbackKeyDown(unsigned key, unsigned keychar)
{
	if (HandleKeyModifiersDown(key, keychar)) {
		return;
	}

	// FIXME: don't handle unicode well. Should work on all latin keyboard.
	const char *ptr = strchr(UiGroupKeys.c_str(), key);

	if (ptr) {
		key = ((int)'0') + ptr - UiGroupKeys.c_str();
		if (key > '9') {
			key = SDLK_BACKQUOTE;
		}
	}
	switch (key) {
		case SDLK_PAGEUP:
			if ((KeyModifiers & ModifierAlt) && KeyModifiers & ModifierControl) {
				if (editorSlider->isVisible()) {
					editorSlider->keyPress(gcn::Key::K_LEFT);
				}
			}
			break;

		case SDLK_PAGEDOWN:
			if ((KeyModifiers & ModifierAlt) && KeyModifiers & ModifierControl) {
				if (editorSlider->isVisible()) {
					editorSlider->keyPress(gcn::Key::K_RIGHT);
				}
			}
			break;

		case 't':
			toolDropdown->setSelected((toolDropdown->getSelected() + 1) % (toolDropdown->getListModel()->getNumberOfElements()));
			toolDropdown->action("");
			break;

		case 'f': // ALT+F, CTRL+F toggle fullscreen
			if (!(KeyModifiers & (ModifierAlt | ModifierControl))) {
				break;
			}
			ToggleFullScreen();
			break;

		case 'v': // 'v' Viewport
			if (KeyModifiers & ModifierControl) {
				CycleViewportMode(-1);
			} else {
				CycleViewportMode(1);
			}
			break;

		// FIXME: move to lua
		case 'r': // CTRL+R Randomize map
			if (KeyModifiers & ModifierControl) {
				Editor.CreateRandomMap();
			}
			break;

		// FIXME: move to lua
		case 'm': // CTRL+M Mirror edit
			if (KeyModifiers & ModifierControl)  {
				++MirrorEdit;
				if (MirrorEdit == 3) {
					MirrorEdit = 0;
				}
				switch (MirrorEdit) {
					case 1:
						UI.StatusLine.Set(_("Mirror editing enabled: 2-side"));
						break;
					case 2:
						UI.StatusLine.Set(_("Mirror editing enabled: 4-side"));
						break;
					default:
						UI.StatusLine.Set(_("Mirror editing disabled"));
						break;
				}
			}
			break;
		case 'x': // ALT+X, CTRL+X: Exit editor
			if (!(KeyModifiers & (ModifierAlt | ModifierControl))) {
				break;
			}
			Exit(0);

		case 'z':
			if (KeyModifiers & ModifierControl) {
				EditorUndoAction();
			}
			break;
		case 'y':
			if (KeyModifiers & ModifierControl) {
				EditorRedoAction();
			}
			break;

		case SDLK_BACKSPACE:
		case SDLK_DELETE: // Delete
			if (UnitUnderCursor != nullptr) {
				EditorRemoveUnit(*UnitUnderCursor);
			}
			break;

		case SDLK_UP: // Keyboard scrolling
		case SDLK_KP_8:
			KeyScrollState |= ScrollUp;
			break;
		case SDLK_DOWN:
		case SDLK_KP_2:
			KeyScrollState |= ScrollDown;
			break;
		case SDLK_LEFT:
		case SDLK_KP_4:
			KeyScrollState |= ScrollLeft;
			break;
		case SDLK_RIGHT:
		case SDLK_KP_6:
			KeyScrollState |= ScrollRight;
			break;
		case '0':
			if (!(KeyModifiers & ModifierAlt)) {
				if (UnitUnderCursor != nullptr) {
					UnitUnderCursor->ChangeOwner(Players[PlayerNumNeutral]);
					UI.StatusLine.Set(_("Unit owner modified"));
				}
				break;
			} else {
				// fallthrough
			}
		case '1': case '2':
		case '3': case '4': case '5':
		case '6': case '7': case '8':
		case '9': {
			int pnum = (int) key - '1';
			if (KeyModifiers & ModifierAlt) {
				pnum += 10;
				if (pnum >= PlayerMax) {
					break;
				}
			}
			if (UnitUnderCursor != nullptr && Map.Info.PlayerType[pnum] != PlayerTypes::PlayerNobody) {
				UnitUnderCursor->ChangeOwner(Players[pnum]);
				UI.StatusLine.Set(_("Unit owner modified"));
				UpdateMinimap = true;
			}
			break;
		}
		case 'g': // Toggle map grid
			CViewport::EnableGrid(!CViewport::isGridEnabled());
			break;
		case '+': /// Increace brush's elevation level
		case '=':
			if (Editor.State == EditorStateType::ElevationLevel && Editor.SelectedElevationLevel < 255) {
				Editor.SelectedElevationLevel++;
			}
			break;
		case '-': /// Decreace brush's elevation level
			if (Editor.State == EditorStateType::ElevationLevel && Editor.SelectedElevationLevel > 0) {
				Editor.SelectedElevationLevel--;
			}
			break;
		case ']': /// Increace highlighted elevation level
			if (overlaysDropdown->getSelected() == EditorOverlays::cElevation
			    && Editor.HighlightElevationLevel < 255) {
				Editor.HighlightElevationLevel++;
			}
			break;
		case '[': /// Decreace highlighted elevation level
			if (overlaysDropdown->getSelected() == EditorOverlays::cElevation
			    && Editor.HighlightElevationLevel > 0) {
				Editor.HighlightElevationLevel--;
			}
			break;
		default:
			HandleCommandKey(key);
			return;
	}
}

/**
**  Handle key up.
**
**  @param key      Key scancode.
**  @param keychar  Character code.
*/
static void EditorCallbackKeyUp(unsigned key, unsigned keychar)
{
	if (HandleKeyModifiersUp(key, keychar)) {
		return;
	}

	switch (key) {
		case SDLK_UP: // Keyboard scrolling
		case SDLK_KP_8:
			KeyScrollState &= ~ScrollUp;
			break;
		case SDLK_DOWN:
		case SDLK_KP_2:
			KeyScrollState &= ~ScrollDown;
			break;
		case SDLK_LEFT:
		case SDLK_KP_4:
			KeyScrollState &= ~ScrollLeft;
			break;
		case SDLK_RIGHT:
		case SDLK_KP_6:
			KeyScrollState &= ~ScrollRight;
			break;
		default:
			break;
	}
}

/**
**  Callback for input.
*/
static void EditorCallbackKeyRepeated(unsigned key, unsigned)
{
	switch (key) {
		case 'z':
			if (KeyModifiers & ModifierControl) {
				EditorUndoAction();
			}
			break;
		case 'y':
			if (KeyModifiers & ModifierControl) {
				EditorRedoAction();
			}
			break;
	}
}

static bool EditorCallbackMouse_EditUnitArea(const PixelPos &screenPos)
{
	Assert(Editor.State == EditorStateType::EditUnit || Editor.State == EditorStateType::SetStartLocation);

	bool noHit = forEachPlayerSelectionBoxArea([screenPos](int i, int x, int y, int w, int h) {
		if (x < screenPos.x && screenPos.x < x + w && y < screenPos.y && screenPos.y < y + h) {
			if (Map.Info.PlayerType[i] != PlayerTypes::PlayerNobody) {
				char buf[256];
				snprintf(buf, sizeof(buf), _("Select player #%d"), i + 1);
				UI.StatusLine.Set(buf);
			} else {
				UI.StatusLine.Clear();
			}
			Editor.CursorPlayer = i;
			return false;
		} else {
			return true;
		}
	});

	noHit = forEachUnitIconArea([screenPos](int i, ButtonStyle*, int x, int y, int w, int h) {
		if (x < screenPos.x && screenPos.x < x + w && y < screenPos.y && screenPos.y < y + h) {
			if (Editor.ShownUnitTypes[i] == nullptr) {
				return false;
			}
			char buf[256];
			snprintf(buf, sizeof(buf), "%s \"%s\"",
					 Editor.ShownUnitTypes[i]->Ident.c_str(),
					 Editor.ShownUnitTypes[i]->Name.c_str());
			UI.StatusLine.Set(buf);
			Editor.CursorUnitIndex = i;
			return false;
		}
		return true;
	});

	return false;
}

static bool EditorCallbackMouse_EditTileArea(const PixelPos &screenPos)
{
	int bx = UI.InfoPanel.X + 4;
	int by = UI.InfoPanel.Y + 4 + IconHeight + 10;

	bool noHit = forEachTileOptionArea([screenPos](bool active, std::string &label, int i, int x, int y, int w, int h) {
		if (x < screenPos.x && screenPos.x < x + w && y < screenPos.y && screenPos.y < y + h) {
			ButtonUnderCursor = i + 300;
			CursorOn = ECursorOn::Button;
			return false;
		}
		return true;
	});

	noHit = forEachTileIconArea([screenPos](int i, int x, int y, int w, int h) {
		if (x < screenPos.x && screenPos.x < x + w && y < screenPos.y && screenPos.y < y + w) {
			const int tile = Editor.ShownTileTypes[i];
			const int32_t tileindex = Map.Tileset->findTileIndexByTile(tile);
			Assert(tileindex != -1);
			const terrain_typeIdx base = Map.Tileset->tiles[tileindex].tileinfo.BaseTerrain;
			UI.StatusLine.Set(Map.Tileset->getTerrainName(base));
			Editor.CursorTileIndex = i;
			return false;
		}
		return true;
	});

	return false;
}

/**
**  Callback for input movement of the cursor.
**
**  @param pos  Screen position.
*/
static void EditorCallbackMouse(const PixelPos &pos)
{
	static int LastMapX;
	static int LastMapY;

	PixelPos restrictPos = pos;
	HandleCursorMove(&restrictPos.x, &restrictPos.y); // Reduce to screen
	const PixelPos screenPos = pos;

	// Move map.
	if (GameCursor == UI.Scroll.Cursor) {
		Vec2i tilePos = UI.MouseViewport->MapPos;

		// FIXME: Support with CTRL for faster scrolling.
		// FIXME: code duplication, see ../ui/mouse.c
		if (UI.MouseScrollSpeedDefault < 0) {
			if (screenPos.x < CursorStartScreenPos.x) {
				tilePos.x++;
			} else if (screenPos.x > CursorStartScreenPos.x) {
				tilePos.x--;
			}
			if (screenPos.y < CursorStartScreenPos.y) {
				tilePos.y++;
			} else if (screenPos.y > CursorStartScreenPos.y) {
				tilePos.y--;
			}
		} else {
			if (screenPos.x < CursorStartScreenPos.x) {
				tilePos.x--;
			} else if (screenPos.x > CursorStartScreenPos.x) {
				tilePos.x++;
			}
			if (screenPos.y < CursorStartScreenPos.y) {
				tilePos.y--;
			} else if (screenPos.y > CursorStartScreenPos.y) {
				tilePos.y++;
			}
		}
		UI.MouseWarpPos = CursorStartScreenPos;
		UI.MouseViewport->Set(tilePos, Map.Tileset->getPixelTileSize() / 2);
		return;
	}

	// Automatically unpress when map tile has changed
	const Vec2i cursorTilePos = UI.SelectedViewport->ScreenToTilePos(CursorScreenPos);

	if (LastMapX != cursorTilePos.x || LastMapY != cursorTilePos.y) {
		LastMapX = cursorTilePos.x;
		LastMapY = cursorTilePos.y;
		UnitPlacedThisPress = false;
	}
	// Drawing tiles on map.
	if (CursorOn == ECursorOn::Map && (MouseButtons & LeftButton)
		&& (Editor.State == EditorStateType::EditTile || Editor.State == EditorStateType::EditUnit)) {
		Vec2i vpTilePos = UI.SelectedViewport->MapPos;
		// Scroll the map
		if (CursorScreenPos.x <= UI.SelectedViewport->GetTopLeftPos().x) {
			vpTilePos.x--;
			UI.SelectedViewport->Set(vpTilePos, Map.Tileset->getPixelTileSize() / 2);
		} else if (CursorScreenPos.x >= UI.SelectedViewport->GetBottomRightPos().x) {
			vpTilePos.x++;
			UI.SelectedViewport->Set(vpTilePos, Map.Tileset->getPixelTileSize() / 2);
		}

		if (CursorScreenPos.y <= UI.SelectedViewport->GetTopLeftPos().y) {
			vpTilePos.y--;
			UI.SelectedViewport->Set(vpTilePos, Map.Tileset->getPixelTileSize() / 2);
		} else if (CursorScreenPos.y >= UI.SelectedViewport->GetBottomRightPos().y) {
			vpTilePos.y++;
			UI.SelectedViewport->Set(vpTilePos, Map.Tileset->getPixelTileSize() / 2);
		}

		// Scroll the map, if cursor moves outside the viewport.
		RestrictCursorToViewport();
		const Vec2i tilePos = UI.SelectedViewport->ScreenToTilePos(CursorScreenPos);

		if (Editor.State == EditorStateType::EditTile && (Editor.SelectedTileIndex != -1 || (KeyModifiers & ModifierAlt))) {
			EditTiles(tilePos, Editor.SelectedTileIndex != -1 ? Editor.ShownTileTypes[Editor.SelectedTileIndex] : -1, TileCursorSize);
		} else if (Editor.State == EditorStateType::EditUnit && CursorBuilding) {
			if (!UnitPlacedThisPress) {
				if (CanBuildUnitType(nullptr, *CursorBuilding, tilePos, 1)) {
					EditorPlaceUnit(tilePos, *CursorBuilding, Players + Editor.SelectedPlayer);
					UnitPlacedThisPress = true;
					UI.StatusLine.Clear();
				}
			}
		}
		return;
	}

	// Minimap move viewpoint
	if (CursorOn == ECursorOn::Minimap && (MouseButtons & LeftButton)) {
		RestrictCursorToMinimap();
		const Vec2i tilePos = UI.Minimap.ScreenToTilePos(CursorScreenPos);

		UI.SelectedViewport->Center(Map.TilePosToMapPixelPos_Center(tilePos));
		return;
	}

	MouseScrollState = ScrollNone;
	GameCursor = UI.Point.Cursor;
	CursorOn = ECursorOn::Unknown;
	Editor.CursorPlayer = -1;
	Editor.CursorUnitIndex = -1;
	Editor.CursorTileIndex = -1;
	ButtonUnderCursor = -1;
	OldButtonUnderCursor = -1;

	// Minimap
	if (UI.Minimap.Contains(screenPos)) {
		CursorOn = ECursorOn::Minimap;
	}
	// Handle edit unit area
	if (Editor.State == EditorStateType::EditUnit || Editor.State == EditorStateType::SetStartLocation) {
		if (EditorCallbackMouse_EditUnitArea(screenPos) == true) {
			return;
		}
	}

	// Handle tile area
	if (Editor.State == EditorStateType::EditTile) {
		if (EditorCallbackMouse_EditTileArea(screenPos) == true) {
			return;
		}
	}

	if (UI.MenuButton.X != -1 && UI.MenuButton.Contains(screenPos)) {
		ButtonAreaUnderCursor = ButtonArea::Menu;
		ButtonUnderCursor = ButtonUnderMenu;
		CursorOn = ECursorOn::Button;
		return;
	} else {
		ButtonAreaUnderCursor.reset();
	}

	// Minimap
	if (UI.Minimap.Contains(screenPos)) {
		CursorOn = ECursorOn::Minimap;
		return;
	}

	// Map
	UnitUnderCursor = nullptr;
	if (UI.MapArea.Contains(screenPos)) {
		CViewport *vp = GetViewport(screenPos);
		Assert(vp);
		if (UI.MouseViewport != vp) { // viewport changed
			UI.MouseViewport = vp;
			DebugPrint("active viewport changed to %ld.\n",
			           static_cast<long int>(UI.Viewports - vp));
		}
		CursorOn = ECursorOn::Map;

		// Look if there is an unit under the cursor.
		const PixelPos cursorMapPos = UI.MouseViewport->ScreenToMapPixelPos(CursorScreenPos);
		UnitUnderCursor = UnitOnScreen(cursorMapPos.x, cursorMapPos.y);

		if (UnitUnderCursor != nullptr) {
			ShowUnitInfo(*UnitUnderCursor);
			return;
		}
	}
	// Scrolling Region Handling
	if (HandleMouseScrollArea(screenPos)) {
		return;
	}

	// Not reached if cursor is inside the scroll area

	UI.StatusLine.Clear();
}

/**
**  Callback for exit.
*/
static void EditorCallbackExit()
{
}

/**
**  Create editor.
*/
void CEditor::Init()
{
	// Load and evaluate the editor configuration file
	const fs::path filename = LibraryFileName(Parameters::Instance.luaEditorStartFilename.string());
	if (!fs::exists(filename)) {
		fprintf(stderr,
		        "Editor configuration file '%s' was not found\n"
		        "Specify another with '-E file.lua'\n",
		        Parameters::Instance.luaEditorStartFilename.u8string().c_str());
		ExitFatal(-1);
	}

	ShowLoadProgress(_("Script %s"), filename.u8string().c_str());
	LuaLoadFile(filename);
	LuaGarbageCollect();

	ThisPlayer = &Players[0];

	FlagRevealMap = MapRevealModes::cExplored; // editor without fog and all visible
	Map.NoFogOfWar = true;

	if (!*CurrentMapPath) { // new map!
		InitUnitTypes(1);
		//
		// Inititialize Map / Players.
		//
		InitPlayers();
		for (int i = 0; i < PlayerMax; ++i) {
			if (i == PlayerNumNeutral) {
				CreatePlayer(PlayerTypes::PlayerNeutral);
				Map.Info.PlayerType[i] = PlayerTypes::PlayerNeutral;
				Map.Info.PlayerSide[i] = Players[i].Race = 0;
			} else {
				CreatePlayer(PlayerTypes::PlayerNobody);
				Map.Info.PlayerType[i] = PlayerTypes::PlayerNobody;
			}
		}

		Map.Fields.resize(Map.Info.MapWidth * Map.Info.MapHeight);

		const int defaultTile = Map.Tileset->getDefaultTileIndex();

		for (int i = 0; i < Map.Info.MapWidth * Map.Info.MapHeight; ++i) {
			Map.Fields[i].setTileIndex(*Map.Tileset, defaultTile, 0, 0);
		}
		GameSettings.Resources = SettingsPresetMapDefault;
		CreateGame("", &Map);
	} else {
		CreateGame(CurrentMapPath, &Map);
	}

	ReplayRevealMap = true;
	FlagRevealMap = MapRevealModes::cHidden;
	Editor.SelectedPlayer = PlayerNumNeutral;

	// Place the start points, which the loader discarded.
	for (int i = 0; i < PlayerMax; ++i) {
		if (Map.Info.PlayerType[i] != PlayerTypes::PlayerNobody) {
			// Set SelectedPlayer to a valid player
			if (Editor.SelectedPlayer == PlayerNumNeutral) {
				Editor.SelectedPlayer = i;
				break;
			}
		}
	}
	if (UI.ButtonPanel.G != nullptr) {
		ButtonPanelWidth = UI.ButtonPanel.G->Width;
	} else if (UI.InfoPanel.G != nullptr) {
		ButtonPanelWidth = UI.InfoPanel.G->Width;
	} else {
		ButtonPanelWidth = 170;
	}
	ButtonPanelHeight = 160 + (Video.Height - 480);

	CalculateMaxIconSize();

	if (!StartUnitName.empty()) {
		StartUnit = &UnitTypeByIdent(StartUnitName);
	}
	Select.Icon = nullptr;
	Select.Load();
	Units.Icon = nullptr;
	Units.Load();

	Map.Tileset->fillSolidTiles(&Editor.ShownTileTypes);

	RecalculateShownUnits();

	EditorUndoActions.clear();
	EditorRedoActions.clear();

	EditorCallbacks.ButtonPressed = EditorCallbackButtonDown;
	EditorCallbacks.ButtonReleased = EditorCallbackButtonUp;
	EditorCallbacks.MouseMoved = EditorCallbackMouse;
	EditorCallbacks.MouseExit = EditorCallbackExit;
	EditorCallbacks.KeyPressed = EditorCallbackKeyDown;
	EditorCallbacks.KeyReleased = EditorCallbackKeyUp;
	EditorCallbacks.KeyRepeated = EditorCallbackKeyRepeated;
	EditorCallbacks.NetworkEvent = NetworkEvent;
}

/**
**  Save a map from editor.
**
**  @param file  Save the level to this file.
**
**  @return      0 for success, -1 for error
**
**  @todo  FIXME: Check if the map is valid, contains no failures.
**         At least two players, one human slot, every player a startpoint
**         ...
*/
int EditorSaveMap(const std::string &file)
{
	const fs::path fullName = fs::path(StratagusLibPath) / file;

	if (!SaveStratagusMap(fullName, Map, Editor.TerrainEditable)) {
		fprintf(stderr, "Cannot save map\n");
		return -1;
	}
	return 0;
}

int EditorSaveMapWithResize(std::string_view file, Vec2i newSize, Vec2i offset)
{
	const fs::path fullName = fs::path(StratagusLibPath) / file;

	if (!SaveStratagusMap(fullName, Map, Editor.TerrainEditable, newSize, offset)) {
		fprintf(stderr, "Cannot save map\n");
		return -1;
	}
	return 0;
}

/*----------------------------------------------------------------------------
--  Editor main loop
----------------------------------------------------------------------------*/

/**
**  Editor main event loop.
*/
void EditorMainLoop()
{
	bool OldCommandLogDisabled = CommandLogDisabled;
	const EventCallback *old_callbacks = GetCallbacks();
	bool first_init = true;

	CommandLogDisabled = true;
	SetCallbacks(&EditorCallbacks);

	gcn::Widget *oldTop = Gui->getTop();

	editorContainer = std::make_unique<gcn::Container>();
	editorContainer->setDimension(gcn::Rectangle(0, 0, Video.Width, Video.Height));
	editorContainer->setOpaque(false);
	Gui->setTop(editorContainer.get());

	// The slider is positioned in the bottom of the button area
	editorSlider = std::make_unique<gcn::Slider>();
	editorSlider->setStepLength(1.0 / 50);
	editorSlider->setWidth(getButtonArea()[2] - getButtonArea()[0]);
	editorSlider->setHeight(GetSmallFont().getHeight());
	editorSlider->setBaseColor(gcn::Color(38, 38, 78));
	editorSlider->setForegroundColor(gcn::Color(200, 200, 120));
	editorSlider->setBackgroundColor(gcn::Color(200, 200, 120));
	editorSlider->setVisible(false);
	auto editorSliderListener = std::make_unique<LambdaActionListener>([](const std::string&) {
		switch (Editor.State) {
			case EditorStateType::EditTile:
				{
					const int iconsPerStep = VisibleTileIcons;
					const int steps = (Editor.ShownTileTypes.size() + iconsPerStep - 1) / iconsPerStep;
					const double value = editorSlider->getValue();
					for (int i = 1; i <= steps; ++i) {
						if (value <= (double)i / steps) {
							Editor.TileIndex = iconsPerStep * (i - 1);
							break;
						}
					}
				}
				break;
			case EditorStateType::EditUnit:
				{
					const int iconsPerStep = VisibleUnitIcons;
					const int steps = (Editor.ShownUnitTypes.size() + iconsPerStep - 1) / iconsPerStep;
					const double value = editorSlider->getValue();
					for (int i = 1; i <= steps; ++i) {
						if (value <= (double)i / steps) {
							Editor.UnitIndex = iconsPerStep * (i - 1);
							break;
						}
					}
				}
				break;
			default:
				break;
		}
	});
	editorSlider->addActionListener(editorSliderListener.get());
	editorContainer->add(editorSlider.get(), getSelectionArea()[0], getSelectionArea()[3] - editorSlider->getHeight());

	// Mode selection is put into the status line
	std::vector<std::string> toolListStrings = { "Select", "Tiles", "Start Locations", "Units", "Elevation" };
	auto toolList = std::make_unique<StringListModel>(toolListStrings);
	toolDropdown = std::make_unique<gcn::DropDown>(toolList.get());
	auto toolDropdownListener = std::make_unique<LambdaActionListener>([&toolListStrings](const std::string&) {
		int selected = toolDropdown->getSelected();
		// Click on mode area
		Editor.CursorUnitIndex = Editor.CursorTileIndex = Editor.SelectedUnitIndex = Editor.SelectedTileIndex = -1;
		CursorBuilding = nullptr;
		Editor.UnitIndex = Editor.TileIndex = 0;
		switch (selected) {
			case 0:
				Editor.State = EditorStateType::Selecting;
				editorSlider->setVisible(false);
				return;
			case 1:
				Editor.State = EditorStateType::EditTile;
				editorSlider->setVisible(true);
				editorSlider->setValue(0);
				return;
			case 2:
				Editor.State = EditorStateType::SetStartLocation;
				editorSlider->setVisible(false);
				return;
			case 3:
				Editor.State = EditorStateType::EditUnit;
				RecalculateShownUnits();
				editorSlider->setVisible(true);
				editorSlider->setValue(0);
				return;
			case 4:
				Editor.State = EditorStateType::ElevationLevel;
				editorSlider->setVisible(false);
				Editor.SelectedElevationLevel = 0;
				return;
			default: {
				std::string selectedString = toolListStrings[selected];
				Editor.State = EditorStateType::EditUnit;
				size_t startIndex = 0;
				size_t endIndex = INT_MAX;
				for (size_t i = 0; i < Editor.UnitTypes.size(); i++) {
					std::string entry = Editor.UnitTypes[i];
					if (startIndex == 0) {
						if (entry.find(selectedString, 2) != std::string::npos) {
							startIndex = i + 1;
						}
					} else {
						if (entry.rfind("--", 0) != std::string::npos) {
							endIndex = i;
							break;
						}
					}
				}
				RecalculateShownUnits(startIndex, endIndex);
				editorSlider->setVisible(true);
				editorSlider->setValue(0);
				break;
			}
		}
	});
	toolDropdown->setWidth(100);
	toolDropdown->setBaseColor(gcn::Color(38, 38, 78));
	toolDropdown->setForegroundColor(gcn::Color(200, 200, 120));
	toolDropdown->setBackgroundColor(gcn::Color(200, 200, 120));
	toolDropdown->addActionListener(toolDropdownListener.get());
	// toolDropdown->setFont(GetSmallFont());
	if (UI.MenuButton.Y < toolDropdown->getHeight()) {
		// menu button is up top, move the selection tool right
		editorContainer->add(toolDropdown.get(), UI.MenuButton.X + UI.MenuButton.Style->Width + 10, 0);
	} else {
		editorContainer->add(toolDropdown.get(), 0, 0);
	}

	std::vector<std::string> overlaysListStrings = { "Layers: None", "Unpassable", "No building allowed", "Elevation", "Opaque" };
	auto overlaysList = std::make_unique<StringListModel>(overlaysListStrings);
	overlaysDropdown = std::make_unique<gcn::DropDown>(overlaysList.get());
	auto overlaysDropdownListener = std::make_unique<LambdaActionListener>([&overlaysListStrings](const std::string&) {
		const int selected = overlaysDropdown->getSelected();
		switch (selected) {
			case EditorOverlays::cNone:
				Editor.OverlayHighlighter = nullptr;
				return;
			case EditorOverlays::cUnpassable:
				Editor.OverlayHighlighter = OverlayUnpassable;
				return;
			case EditorOverlays::cNoBuildingAllowed:
				Editor.OverlayHighlighter = OverlayNoBuildingAllowed;
				return;
			case EditorOverlays::cElevation:
				Editor.HighlightElevationLevel = 1;
				Editor.OverlayHighlighter = OverlayElevation;
				return;
			case EditorOverlays::cOpaque:
				Editor.OverlayHighlighter = OverlayOpaque;
				return;
			default:
				Editor.OverlayHighlighter = nullptr;
				break;
		}

	});

	int overlaysWidth = 0;
	for (std::string &entry : overlaysListStrings) {
		toolDropdown->getFont()->getWidth(entry);
		overlaysWidth = std::max(overlaysWidth, toolDropdown->getFont()->getWidth(entry) + 20);
	}

	overlaysDropdown->setWidth(overlaysWidth);
	overlaysDropdown->getScrollArea()->setWidth(overlaysWidth);
	overlaysDropdown->getListBox()->setWidth(overlaysWidth);

	overlaysDropdown->setBaseColor(gcn::Color(38, 38, 78));
	overlaysDropdown->setForegroundColor(gcn::Color(200, 200, 120));
	overlaysDropdown->setBackgroundColor(gcn::Color(200, 200, 120));
	overlaysDropdown->addActionListener(overlaysDropdownListener.get());
	editorContainer->add(overlaysDropdown.get(), toolDropdown->getX() + toolDropdown->getWidth() + 10, 0);

	UpdateMinimap = true;

	while (1) {
		Editor.MapLoaded = false;
		Editor.Running = EditorEditing;

		Editor.Init();

		// Unit Types is now valid, update the tools
		// TODO (timfel): This is very ugly/hacky, but the entire editor is, unfortunately...
		int newW = toolDropdown->getWidth();
		for (std::string entry : Editor.UnitTypes) {
			if (entry.rfind("--", 0) != std::string::npos) {
				std::string e = entry.substr(2);
				toolListStrings.push_back(e);
				int strW = toolDropdown->getFont()->getWidth(e) + 20;
				if (newW < strW) {
					newW = strW;
				}
			}
		}
		toolDropdown->setListModel(new StringListModel(toolListStrings));
		toolDropdown->setWidth(newW);
		toolDropdown->getScrollArea()->setWidth(newW);
		toolDropdown->getListBox()->setWidth(newW);

		overlaysDropdown->setX(toolDropdown->getX() + toolDropdown->getWidth() + 10);

		//ProcessMenu("menu-editor-tips", 1);
		InterfaceState = IfaceState::Normal;

		SetVideoSync();

		GameCursor = UI.Point.Cursor;
		InterfaceState = IfaceState::Normal;
		Editor.State = EditorStateType::Selecting;
		UI.SelectedViewport = UI.Viewports;
		TileCursorSize = 1;

		bool start = true;

		while (Editor.Running) {
			if (FrameCounter % CYCLES_PER_SECOND == 0) {
				if (UpdateMinimap) {
					UI.Minimap.Update();
					UpdateMinimap = false;
				}
			}

			EditorUpdateDisplay();

			//
			// Map scrolling
			//
			if (UI.MouseScroll) {
				int tbX, tbY;
				toolDropdown->getAbsolutePosition(tbX, tbY);
				tbX += CursorScreenPos.x;
				tbY += CursorScreenPos.y;
				if (!(toolDropdown->getDimension().isPointInRect(tbX, tbY))) {
					DoScrollArea(MouseScrollState, 0, MouseScrollState == 0 && KeyScrollState > 0);
				}
			}
			if (UI.KeyScroll) {
				if (CursorOn == ECursorOn::Map) {
					DoScrollArea(KeyScrollState, (KeyModifiers & ModifierControl) != 0, MouseScrollState == 0 && KeyScrollState > 0);
				}
				if (CursorOn == ECursorOn::Map && (MouseButtons & LeftButton) &&
					(Editor.State == EditorStateType::EditTile ||
					 Editor.State == EditorStateType::EditUnit)) {
					EditorCallbackButtonDown(0);
				}
			}

			if (start) {
				start = false;
				if (UI.MenuButton.Callback) {
					UI.MenuButton.Callback->action("");
				}
			}

			WaitEventsOneFrame();
		}
		CursorBuilding = nullptr;
		if (!Editor.MapLoaded) {
			break;
		}

		CleanModules();

		LoadCcl(Parameters::Instance.luaStartFilename); // Reload the main config file

		PreMenuSetup();

		InterfaceState = IfaceState::Menu;
		GameCursor = UI.Point.Cursor;

		Video.ClearScreen();
		Invalidate();
	}

	CommandLogDisabled = OldCommandLogDisabled;
	SetCallbacks(old_callbacks);
	Gui->setTop(oldTop);

	toolDropdown.reset();
	overlaysDropdown.reset();
	editorContainer.reset();
	editorSlider.reset();
}

/**
**  Start the editor
**
**  @param filename  Map to load, nullptr to create a new map
*/
void StartEditor(const char *filename)
{
	std::string nc, rc;

	GetDefaultTextColors(nc, rc);
	if (filename) {
		if (strcpy_s(CurrentMapPath, sizeof(CurrentMapPath), filename) != 0) {
			filename = nullptr;
		}
	}
	if (!filename) {
		// new map, choose some default values
		strcpy_s(CurrentMapPath, sizeof(CurrentMapPath), "");
		// Map.Info.Description.clear();
		// Map.Info.MapWidth = 64;
		// Map.Info.MapHeight = 64;
	}

	// Run the editor.
	EditorMainLoop();

	// Clear screen
	Video.ClearScreen();
	Invalidate();

	Editor.TerrainEditable = true;

	Editor.ShownTileTypes.clear();
	CleanGame();
	CleanPlayers();

	SetDefaultTextColors(nc, rc);
}

//@}
