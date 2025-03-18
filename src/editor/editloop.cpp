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

#include <cstdint>
#include <deque>

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

#include <guisan.hpp>

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

static bool UnitPlacedThisPress = false;  /// Only allow one unit per press
static int VisibleUnitIcons = 0;              /// Number of icons that are visible at a time

enum class EditorActionType {
	PlaceUnit,
	RemoveUnit
};

enum EditorOverlays {
	NoOverlays,
	Unpassable,
	NoBuildingAllowed,
	Elevation,
	Opaque
};

enum CornerPosition {
	cUpperLeftX = 0,
	cUpperLeftY,
	cBottomRightX,
	cBottomRightY
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
static constexpr double sliderDefaultScale = 1.0;
static constexpr double sliderDefaultStepLength = 1.0 / 50;

static std::unique_ptr<gcn::DropDown> toolDropdown;
static std::unique_ptr<gcn::DropDown> overlaysDropdown;

static std::unique_ptr<CBrushControlsUI> brushesCtrlUI;

/*----------------------------------------------------------------------------
--  Functions
----------------------------------------------------------------------------*/
/**
**  Release the memory allocated for the editor's controls
**  in the correct order
*/
static void EditorClearControls()
{
	brushesCtrlUI.reset();
	toolDropdown.reset();
	overlaysDropdown.reset();
	editorContainer.reset();
	Editor.tileIcons.resetSliderCtrl();
	editorSlider.reset();
}

/*----------------------------------------------------------------------------
--  Edit
----------------------------------------------------------------------------*/

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
	Editor.UpdateMinimap = true;
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
	Editor.UpdateMinimap = true;
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
			CUnit *unit = UnitOnMapTile(action.tilePos, action.UnitType->MoveType);
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
			CUnit *unit = UnitOnMapTile(action.tilePos, action.UnitType->MoveType);
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

static std::string getTileInfo(const tile_index index)
{
	// Tile info
	const CTileset &tileset = Map.Tileset;

	const auto baseTerrainIdx = tileset.tiles[index].tileinfo.BaseTerrain;
	const auto baseTerrain = tileset.getTerrainName(baseTerrainIdx).c_str();
	const auto mixTerrainIdx = tileset.tiles[index].tileinfo.MixTerrain;
	const auto mixTerrain = mixTerrainIdx ? tileset.getTerrainName(mixTerrainIdx).c_str() : "";

	constexpr size_t buffSize = 256;
	char buf[buffSize]{};
	snprintf(buf,
			 buffSize,
			 "[0x%04X] %s%s%s",
			 index,
			 baseTerrain,
			 mixTerrainIdx ? " <> " : "",
			 mixTerrain);
	return std::string(buf);
}

/**
 * Call the forEach callback with each player icon's <playerNum,x,y,w,h>. Return false to cancel iteration.
 *
 * Returns the last value returned by forEach. This can be used to detect if an early cancellation of the
 * iteration was requested.
 */
static bool forEachPlayerSelectionBoxArea(std::function<bool(int, int, int, int, int)> forEach) {
	int x = getSelectionArea()[cUpperLeftX];
	int y = getSelectionArea()[cUpperLeftY];
	int x2 = getSelectionArea()[cBottomRightX];
	int y2 = getSelectionArea()[cBottomRightY];
	int maxX = x2 - getPlayerButtonSize();
	int maxY = y2 - getPlayerButtonSize();

	for (int i = 0; i < PlayerMax; i++, x += getPlayerButtonSize()) {
		if (x > maxX) {
			x = getSelectionArea()[cUpperLeftX];
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

		CLabel label(GetGameFont());
		label.DrawCentered(x + getPlayerButtonSize() / 2, y + 3, std::to_string(i + 1));

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
	int x1 = getButtonArea()[cUpperLeftX];
	int y1 = getButtonArea()[cUpperLeftY];
	int x2 = getButtonArea()[cBottomRightX];
	int y2 = getButtonArea()[cBottomRightY];

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
static bool forEachTileIconArea(std::function<bool(int,int,int,int,int)> forEach)
{
	int x1 = getButtonArea()[cUpperLeftX] + 1;
	int y1 = getButtonArea()[cUpperLeftY] + 2;
	int x2 = getButtonArea()[cBottomRightX];
	int y2 = getButtonArea()[cBottomRightY];

	int tileW = Map.Tileset.getPixelTileSize().x + 1;
	int tileH = Map.Tileset.getPixelTileSize().y + 1;
	int maxX = x2 - tileW;
	int maxY = y2 - tileH;

	int i = Editor.tileIcons.getDisplayedFirst();

	if (Editor.tileIcons.getDisplayedNum() == 0) {
		// initialize on the first draw how many tile icons we can actually draw
		int horizCnt = (x2 - x1) / tileW;
		int vertCnt = (y2 - y1) / tileH;
		Editor.tileIcons.setDisplayedNum(horizCnt * vertCnt);
	}

	int y = y1;
	while (y < maxY) {
		if (i >= Editor.tileIcons.numberOf()) {
			break;
		}
		int x = x1;
		while (x < maxX) {
			if (i >= Editor.tileIcons.numberOf()) {
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
	if (Editor.tileIcons.isEnabled() == false) {
		return;
	}
	forEachTileIconArea([](int i, int x, int y, int w, int h) {

		if (const auto tile = Editor.tileIcons.getTile(i)) {

			const auto iconWidth = Map.Tileset.getPixelTileSize().x;
			const auto iconHeight = Map.Tileset.getPixelTileSize().y;

			Map.TileGraphic->DrawFrameClip(Map.Tileset.getGraphicTileFor(*tile), x, y);
			Video.DrawRectangleClip(ColorGray, x, y, iconWidth, iconHeight);

			if (Editor.tileIcons.isSelected() && i == Editor.tileIcons.getSelectedIcon()) {
				Video.DrawRectangleClip(ColorGreen, x + 1, y + 1, iconWidth - 2, iconHeight - 2);
			}
			if (i == Editor.tileIcons.getIconUnderCursor()) {
				Video.DrawRectangleClip(ColorWhite, x - 1, y - 1, iconWidth + 2, iconHeight + 2);
			}
			return true;
		}
		return false;
	});
}

static void DrawIntoSelectionArea()
{
	switch (Editor.State) {
		case EditorStateType::SetStartLocation:
		case EditorStateType::EditUnit:
			DrawPlayers();
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
	if (overlaysDropdown->getSelected() == EditorOverlays::Elevation) {
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

static void DrawCursorBorder(TilePos tilePos, PixelPos screenPos)
{
	const PixelSize tileSize(Map.Tileset.getPixelTileSize().x, Map.Tileset.getPixelTileSize().y);
	const auto &brush = Editor.brushes.getCurrentBrush();

	if (Editor.State == EditorStateType::EditTile) {
		const PixelPos offset = {tileSize.x * brush.getAlignOffset().x,
								 tileSize.y * brush.getAlignOffset().y};

		if (brush.isRound() && brush.getWidth() > 1) {
			Video.DrawCircleClip(ColorWhite,
								 screenPos.x + tileSize.x / 2,
								 screenPos.y + tileSize.y / 2,
								 tileSize.x * brush.getWidth() / 2 - 1);
		} else {
			Video.DrawRectangleClip(ColorWhite,
									screenPos.x + offset.x,
									screenPos.y + offset.y,
									tileSize.x * brush.getWidth(),
									tileSize.y * brush.getHeight());
		}
		if (Map.Info.IsHighgroundsEnabled()) {
			auto xPos = screenPos.x + offset.x;
			xPos += ((brush.isRound() && brush.getWidth() > 2) ? 4 : 2);
			auto yPos = screenPos.y + 1;
			yPos += (brush.isRound() ? 0 : offset.y);

			CLabel(GetGameFont()).DrawClip(xPos, yPos, Editor.SelectedElevationLevel);
		}
	} else if (Map.Info.IsHighgroundsEnabled() && Editor.State == EditorStateType::ElevationLevel) {

		CLabel(GetGameFont()).DrawClip(screenPos.x + 2,
									   screenPos.y + 1,
									   Editor.SelectedElevationLevel);
	} else {
		Video.DrawRectangleClip(ColorWhite, screenPos.x, screenPos.y, tileSize.x, tileSize.y);
	}
}

static void DrawMapCursor(TilePos tilePos, PixelPos screenPos, const CBrush &brush)
{

	PushClipping();
	UI.MouseViewport->SetClipping();
	const PixelSize tileSize(Map.Tileset.getPixelTileSize().x,
							 Map.Tileset.getPixelTileSize().y);

	auto drawBrushTile = [&screenPos, &tileSize](const TilePos &tileOffset,
												 tile_index tileIdx,
												 bool,
												 bool) -> void
	{
		const PixelPos screenPosIt(screenPos.x + tileOffset.x * tileSize.x,
								   screenPos.y + tileOffset.y * tileSize.y);

		if (!UI.MouseViewport->IsAreaVisibleInViewport(screenPosIt, tileSize)) {
			return;
		}
		Map.TileGraphic->DrawFrameClip(Map.Tileset.getGraphicTileFor(tileIdx),
									   screenPosIt.x,
									   screenPosIt.y);
	};
	brush.applyAt(tilePos, drawBrushTile, true);

	DrawCursorBorder(tilePos, screenPos);

	PopClipping();
}

/**
**  Draw special cursor on map.
**
**  @todo support for bigger cursors (2x2, 3x3 ...)
*/
static void UpdateMapCursor()
{
	//  Affect CursorBuilding if necessary.
	//  (Menu reset CursorBuilding)
	if (!CursorBuilding) {
		switch (Editor.State) {
			case EditorStateType::EditUnit:
				if (Editor.SelectedUnitIndex != -1) {
					CursorBuilding =
						const_cast<CUnitType *>(Editor.ShownUnitTypes[Editor.SelectedUnitIndex]);
				}
				break;
			case EditorStateType::SetStartLocation:
				if (Editor.StartUnit) {
					CursorBuilding = const_cast<CUnitType *>(Editor.StartUnit);
				}
				break;
			default:
				break;
		}
	}

	// Draw map cursor
	if (UI.MouseViewport && !CursorBuilding) {
		const TilePos tilePos = UI.MouseViewport->ScreenToTilePos(CursorScreenPos);
		const PixelPos screenPos = UI.MouseViewport->TilePosToScreen_TopLeft(tilePos);

		if (Editor.State == EditorStateType::EditTile && Editor.tileIcons.isSelected()) {
			DrawMapCursor(tilePos,
						  screenPos,
						  Editor.brushes.getCurrentBrush());

		} else if (Editor.State == EditorStateType::EditTile
					&& Editor.brushes.getCurrentBrush().getType() == CBrush::EBrushTypes::Decoration) {
			DrawMapCursor(tilePos,
						  screenPos,
						  Editor.brushes.getCurrentBrush());
		} else {
			PushClipping();
			UI.MouseViewport->SetClipping();

			DrawCursorBorder(tilePos, screenPos);

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
					DrawUnitType(*type, type->Sprite.get(), i, 0, startScreenPos);
				} else { // Draw a cross
					DrawCross(startScreenPos, Map.Tileset.getPixelTileSize(), Players[i].Color);
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
	const Vec2i pos =
		UI.MouseViewport ? UI.MouseViewport->ScreenToTilePos(CursorScreenPos) : Vec2i(0, 0);

	char buf[256];
	snprintf(buf, sizeof(buf), _("Editor (%d %d)"), pos.x, pos.y);
	CLabel(GetGameFont()).Draw(UI.StatusLine.TextX,
							   UI.StatusLine.TextY - GetGameFont().getHeight() * 3,
							   buf);
	const CMapField &mf = *Map.Field(pos);
	//
	// Flags info
	//
	const unsigned flag = mf.getFlags();
	snprintf(buf,
			 sizeof(buf),
			 "elev:(%u) value:(0x%02X) | flags:(0x%04X)>[%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c]",
			 mf.getElevation(),
			 mf.Value,
			 flag,
			 flag & MapFieldOpaque ? 'o' : '-',
			 flag & MapFieldUnpassable ? 'u' : '-',
			 flag & MapFieldNoBuilding ? 'n' : '-',
			 flag & MapFieldHuman ? 'h' : '-',
			 flag & MapFieldWall ? 'w' : '-',
			 flag & MapFieldRocks ? 'r' : '-',
			 flag & MapFieldForest ? 'f' : '-',
			 flag & MapFieldLandAllowed ? 'L' : '-',
			 flag & MapFieldCoastAllowed ? 'C' : '-',
			 flag & MapFieldWaterAllowed ? 'W' : '-',
			 flag & MapFieldLandUnit ? 'l' : '-',
			 flag & MapFieldAirUnit ? 'a' : '-',
			 flag & MapFieldSeaUnit ? 's' : '-',
			 flag & MapFieldBuilding ? 'b' : '-',
			 flag & MapFieldDecorative ? 'd' : '-',
			 flag & MapFieldNonMixing  ? 'X' : '-');
	CLabel(GetGameFont()).Draw(UI.StatusLine.TextX,
							   UI.StatusLine.TextY - GetGameFont().getHeight() * 2,
							   buf);

	// Display tile info
	const tile_index index = mf.getTileIndex();
	CLabel(GetGameFont()).Draw(UI.StatusLine.TextX,
							   UI.StatusLine.TextY - GetGameFont().getHeight(),
							   getTileInfo(index).c_str());
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

	int n = snprintf(buf,
					 sizeof(buf),
					 _("#%d '%s' Player:#%d %s"),
					 UnitNumber(unit),
					 unit.Type->Name.c_str(),
					 unit.Player->Index + 1,
					 unit.Active ? "active" : "passive");
	if (unit.Type->GivesResource) {
		snprintf(buf + n, sizeof(buf) - n, _(" Amount %d"), unit.ResourcesHeld);
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
		UpdateMapCursor(); // cursor on map
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
static bool OverlayElevation(const CMapField &mapField)
{
	return mapField.getElevation() != Editor.HighlightElevationLevel;
}

static bool OverlayUnpassable(const CMapField &mapField)
{
	return mapField.isFlag(MapFieldUnpassable);
}

static bool OverlayNoBuildingAllowed(const CMapField &mapField)
{
	return mapField.isFlag(MapFieldNoBuilding);
}

static bool OverlayOpaque(const CMapField &mapField)
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
				UI.MenuButton.Callback->action(gcn::ActionEvent{nullptr, ""});
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
		PlayGameSound(GameSounds.Click.Sound.get(), MaxSampleVolume);
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

		if (MouseButtons & RightButton) {
			Editor.tileIcons.resetSelected();
			return;
		} else {
			if (Editor.tileIcons.isEnabled() && Editor.tileIcons.getIconUnderCursor() != -1) {
				Editor.tileIcons.select(Editor.tileIcons.getIconUnderCursor());
				if (const auto selectedTile = Editor.tileIcons.getSelectedTile()) {
					if (Editor.brushes.getCurrentBrush().getType() == CBrush::EBrushTypes::SingleTile) {
						Editor.brushes.getCurrentBrush().setTile(*selectedTile);
					}
				}
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
			} else if (Editor.State == EditorStateType::EditTile
					   && Editor.tileIcons.isSelected()) {

				Editor.tileIcons.resetSelected();
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

			if (Editor.State == EditorStateType::EditTile
				&& (Editor.tileIcons.isSelected()
					|| Editor.brushes.getCurrentBrush().getType() == CBrush::EBrushTypes::Decoration)) {

				Editor.applyCurentBrush(tilePos);

			} else if (Editor.State == EditorStateType::EditUnit) {
				if (!UnitPlacedThisPress && CursorBuilding) {
					if (CanBuildUnitType(nullptr, *CursorBuilding, tilePos, 1)) {
						PlayGameSound(GameSounds.PlacementSuccess[ThisPlayer->Race].Sound.get(),
									  MaxSampleVolume);
						EditorPlaceUnit(tilePos, *CursorBuilding, Players + Editor.SelectedPlayer);
						UnitPlacedThisPress = true;
						UI.StatusLine.Clear();
					} else {
						UI.StatusLine.Set(_("Unit cannot be placed here."));
						PlayGameSound(GameSounds.PlacementError[ThisPlayer->Race].Sound.get(),
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
					gcn::KeyEvent keyEvent{
						nullptr, nullptr, false, false, false, false, 0, false, gcn::Key::Left};
					editorSlider->keyPressed(keyEvent);
				}
			}
			break;

		case SDLK_PAGEDOWN:
			if ((KeyModifiers & ModifierAlt) && KeyModifiers & ModifierControl) {
				if (editorSlider->isVisible()) {
					gcn::KeyEvent keyEvent{
						nullptr, nullptr, false, false, false, false, 0, false, gcn::Key::Right};
					editorSlider->keyPressed(keyEvent);
				}
			}
			break;

		case 't':
			toolDropdown->setSelected((toolDropdown->getSelected() + 1) % (toolDropdown->getListModel()->getNumberOfElements()));
			toolDropdown->action(gcn::ActionEvent{nullptr, ""});
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
				++Editor.MirrorEdit;
				if (Editor.MirrorEdit >= 3) {
					Editor.MirrorEdit = 0;
				}
				switch (Editor.MirrorEdit) {
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
			EditorClearControls();
			Exit(0);
			break;

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
				[[fallthrough]];
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
				Editor.UpdateMinimap = true;
			}
			break;
		}
		case 'g': // Toggle map grid
			CViewport::EnableGrid(!CViewport::isGridEnabled());
			break;
		case '+': /// Increase brush's elevation level
		case '=':
			if (Editor.SelectedElevationLevel < 255
				&& (Editor.State == EditorStateType::ElevationLevel
					|| (Map.Info.IsHighgroundsEnabled()
						&& Editor.State == EditorStateType::EditTile))) {

				Editor.SelectedElevationLevel++;
			}
			break;
		case '-': /// Decreace brush's elevation level
			if (Editor.SelectedElevationLevel > 0
				&& (Editor.State == EditorStateType::ElevationLevel
					|| (Map.Info.IsHighgroundsEnabled()
						&& Editor.State == EditorStateType::EditTile))) {

				Editor.SelectedElevationLevel--;
			}
			break;
		case ']': /// Increace brush size
			if (Editor.State == EditorStateType::EditTile) {
				brushesCtrlUI->resizeByStepUp();
			}
			break;
		case '[': /// Decreace brush size
			if (Editor.State == EditorStateType::EditTile) {
				brushesCtrlUI->resizeByStepDown();
			}
			break;
		case '.': /// Increace highlighted elevation level
		case '>':
			if (overlaysDropdown->getSelected() == EditorOverlays::Elevation
				&& Editor.HighlightElevationLevel < 255) {
				Editor.HighlightElevationLevel++;
			}
			break;
		case ',':	/// Decreace highlighted elevation level
		case '<':
			if (overlaysDropdown->getSelected() == EditorOverlays::Elevation
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
	if (Editor.tileIcons.isEnabled() == false) {
		return false;
	}
	bool noHit = forEachTileIconArea([screenPos](int i, int x, int y, int w, int h) {
		if (x < screenPos.x && screenPos.x < x + w && y < screenPos.y && screenPos.y < y + h) {

			if (i >= Editor.tileIcons.numberOf()) {
				return true;
			}
			if (const auto tileIndex = Editor.tileIcons.getTile(i)) {
				UI.StatusLine.Set(getTileInfo(*tileIndex));
				Editor.tileIcons.setIconUnderCursor(i);
			}
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

	UI.StatusLine.Clear();

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
		UI.MouseViewport->Set(tilePos, Map.Tileset.getPixelTileSize() / 2);
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
			UI.SelectedViewport->Set(vpTilePos, Map.Tileset.getPixelTileSize() / 2);
		} else if (CursorScreenPos.x >= UI.SelectedViewport->GetBottomRightPos().x) {
			vpTilePos.x++;
			UI.SelectedViewport->Set(vpTilePos, Map.Tileset.getPixelTileSize() / 2);
		}

		if (CursorScreenPos.y <= UI.SelectedViewport->GetTopLeftPos().y) {
			vpTilePos.y--;
			UI.SelectedViewport->Set(vpTilePos, Map.Tileset.getPixelTileSize() / 2);
		} else if (CursorScreenPos.y >= UI.SelectedViewport->GetBottomRightPos().y) {
			vpTilePos.y++;
			UI.SelectedViewport->Set(vpTilePos, Map.Tileset.getPixelTileSize() / 2);
		}

		// Scroll the map, if cursor moves outside the viewport.
		RestrictCursorToViewport();
		const Vec2i tilePos = UI.SelectedViewport->ScreenToTilePos(CursorScreenPos);

		if (Editor.State == EditorStateType::EditTile
			&& (Editor.tileIcons.isSelected() || (KeyModifiers & ModifierAlt))) {

			Editor.applyCurentBrush(tilePos);

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
	Editor.tileIcons.resetIconUnderCursor();
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
		ErrorPrint("Editor configuration file '%s' was not found\n"
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
		// Initialize Map / Players.
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

		const int defaultTile = Map.Tileset.getDefaultTileIndex();

		for (int i = 0; i < Map.Info.MapWidth * Map.Info.MapHeight; ++i) {
			Map.Fields[i].setTileIndex(Map.Tileset, defaultTile, 0, 0);
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

	Editor.tileIcons.rebuild();

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
**         At least two players, one human slot, every player have a start point
**         ...
*/
int EditorSaveMap(const std::string &file)
{
	const fs::path fullName = fs::path(StratagusLibPath) / file;

	if (!SaveStratagusMap(fullName, Map, Editor.TerrainEditable)) {
		ErrorPrint("Cannot save map\n");
		return -1;
	}
	return 0;
}

int EditorSaveMapWithResize(std::string_view file, Vec2i newSize, Vec2i offset)
{
	const fs::path fullName = fs::path(StratagusLibPath) / file;

	if (!SaveStratagusMap(fullName, Map, Editor.TerrainEditable, newSize, offset)) {
		ErrorPrint("Cannot save map\n");
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

	CommandLogDisabled = true;
	SetCallbacks(&EditorCallbacks);

	gcn::Widget *oldTop = Gui->getTop();

	editorContainer = std::make_unique<gcn::Container>();
	editorContainer->setDimension(gcn::Rectangle(0, 0, Video.Width, Video.Height));
	editorContainer->setOpaque(false);
	Gui->setTop(editorContainer.get());

	const auto rectangle = gcn::Rectangle(getSelectionArea()[cUpperLeftX],
										  getSelectionArea()[cUpperLeftY],
										  getSelectionArea()[cBottomRightX] - getSelectionArea()[cUpperLeftX],
										  getSelectionArea()[cBottomRightY] - getSelectionArea()[cUpperLeftY]);

	Editor.brushes.loadBrushes("scripts/editor/brushes.lua");
	brushesCtrlUI = std::make_unique<CBrushControlsUI>(editorContainer.get(), rectangle);
	brushesCtrlUI->hide();

	// The slider is positioned in the bottom of the button area
	editorSlider = std::make_unique<gcn::Slider>();
	editorSlider->setFocusable(false);
	editorSlider->setScale(0, sliderDefaultScale);
	editorSlider->setStepLength(sliderDefaultStepLength);
	editorSlider->setWidth(getButtonArea()[cBottomRightX] - getButtonArea()[cUpperLeftX] - 1);
	editorSlider->setHeight(GetSmallFont().getHeight());
	editorSlider->setBaseColor(gcn::Color(38, 38, 78));
	editorSlider->setForegroundColor(gcn::Color(200, 200, 120));
	editorSlider->setBackgroundColor(gcn::Color(200, 200, 120));
	editorSlider->setVisible(false);
	auto editorSliderListener = std::make_unique<LambdaActionListener>([](const std::string&) {
		switch (Editor.State) {
			case EditorStateType::EditTile:
				{
					Editor.tileIcons.recalcDisplayed();
				} break;
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
	editorContainer->add(editorSlider.get(),
						 getSelectionArea()[cUpperLeftX],
						 getSelectionArea()[cBottomRightY] - editorSlider->getHeight());
	Editor.tileIcons.attachSliderCtrl(editorSlider.get());

	// Mode selection is put into the status line
	std::vector<std::string> toolListStrings = { "Select", "Tiles", "Start Locations", "Units" };

	if (Map.Info.IsHighgroundsEnabled()) {
		std::vector<std::string> highgroundsTools = { "Elevation" };

		toolListStrings.insert(ranges::find(toolListStrings, "Start Locations"),
							   highgroundsTools.begin(),
							   highgroundsTools.end());
	}
	auto toolList = std::make_unique<StringListModel>(toolListStrings);
	toolDropdown = std::make_unique<gcn::DropDown>(toolList.get());
	auto toolDropdownListener = std::make_unique<LambdaActionListener>([&toolListStrings](const std::string&) {

		const std::string_view selectedItem = toolListStrings[toolDropdown->getSelected()];

		// Click on mode area
		Editor.CursorUnitIndex = Editor.SelectedUnitIndex = -1;
		Editor.tileIcons.resetSelected();
		Editor.tileIcons.resetIconUnderCursor();
		Editor.tileIcons.displayFrom(0);
		editorSlider->setScale(0, sliderDefaultScale);
		editorSlider->setStepLength(sliderDefaultStepLength);

		CursorBuilding = nullptr;
		Editor.UnitIndex = 0;

		brushesCtrlUI->hide();

		if (selectedItem == "Select") {
			Editor.State = EditorStateType::Selecting;
			editorSlider->setVisible(false);

		} else if (selectedItem == "Tiles") {
			Editor.State = EditorStateType::EditTile;
			editorSlider->setVisible(true);
			editorSlider->setValue(0);
			Editor.tileIcons.updateSliderCtrl();
			brushesCtrlUI->show();

		} else if (selectedItem == "Start Locations") {
			Editor.State = EditorStateType::SetStartLocation;
			editorSlider->setVisible(false);

		} else if (selectedItem == "Elevation") {
			Editor.State = EditorStateType::ElevationLevel;
			editorSlider->setVisible(false);
			Editor.SelectedElevationLevel = 0;

		} else if (selectedItem == "Units") {
			Editor.State = EditorStateType::EditUnit;
			RecalculateShownUnits();
			editorSlider->setVisible(true);
			editorSlider->setValue(0);

		} else { // Units by categories
			Editor.State = EditorStateType::EditUnit;
			size_t startIndex = 0;
			size_t endIndex = INT_MAX;
			for (size_t i = 0; i < Editor.UnitTypes.size(); i++) {
				std::string_view entry = Editor.UnitTypes[i];
				if (startIndex == 0) {
					if (entry.find(selectedItem, 2) != std::string::npos) {
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
		}
	});
	toolDropdown->setFocusable(false);
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

	std::vector<std::string> overlaysListStrings = { "Overlays: None", "Unpassable", "No building allowed", "Elevation", "Opaque" };
	auto overlaysList = std::make_unique<StringListModel>(overlaysListStrings);
	overlaysDropdown = std::make_unique<gcn::DropDown>(overlaysList.get());
	auto overlaysDropdownListener = std::make_unique<LambdaActionListener>([](const std::string&) {
		const int selected = overlaysDropdown->getSelected();
		switch (selected) {
			case EditorOverlays::NoOverlays:
				Editor.OverlayHighlighter = nullptr;
				return;
			case EditorOverlays::Unpassable:
				Editor.OverlayHighlighter = OverlayUnpassable;
				return;
			case EditorOverlays::NoBuildingAllowed:
				Editor.OverlayHighlighter = OverlayNoBuildingAllowed;
				return;
			case EditorOverlays::Elevation:
				Editor.HighlightElevationLevel = 1;
				Editor.OverlayHighlighter = OverlayElevation;
				return;
			case EditorOverlays::Opaque:
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
	overlaysDropdown->setFocusable(false);
	overlaysDropdown->setWidth(overlaysWidth);
	overlaysDropdown->setBaseColor(gcn::Color(38, 38, 78));
	overlaysDropdown->setForegroundColor(gcn::Color(200, 200, 120));
	overlaysDropdown->setBackgroundColor(gcn::Color(200, 200, 120));
	overlaysDropdown->addActionListener(overlaysDropdownListener.get());
	editorContainer->add(overlaysDropdown.get(), toolDropdown->getX() + toolDropdown->getWidth() + 10, 0);

	Editor.UpdateMinimap = true;

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

	overlaysDropdown->setX(toolDropdown->getX() + toolDropdown->getWidth() + 10);

	//ProcessMenu("menu-editor-tips", 1);
	InterfaceState = IfaceState::Normal;

	SetVideoSync();

	GameCursor = UI.Point.Cursor;
	InterfaceState = IfaceState::Normal;
	Editor.State = EditorStateType::Selecting;
	UI.SelectedViewport = UI.Viewports;

	bool start = true;

	while (Editor.Running) {
		if (FrameCounter % CYCLES_PER_SECOND == 0) {
			if (Editor.UpdateMinimap) {
				UI.Minimap.Update();
				Editor.UpdateMinimap = false;
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
			if (!(toolDropdown->getDimension().isContaining(tbX, tbY))) {
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
				UI.MenuButton.Callback->action(gcn::ActionEvent(nullptr, ""));
			}
		}

		WaitEventsOneFrame();
	}
	CursorBuilding = nullptr;

	CommandLogDisabled = OldCommandLogDisabled;
	SetCallbacks(old_callbacks);
	Gui->setTop(oldTop);

	EditorClearControls();
}

/**
**  Start the editor
**
**  @param filename  Map to load, nullptr to create a new map
*/
void StartEditor(const char *filename)
{
	const auto [nc, rc] = GetDefaultTextColors();
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

	Editor.tileIcons.clear();
	CleanGame();
	CleanPlayers();

	SetDefaultTextColors(nc, rc);
}

//@}
