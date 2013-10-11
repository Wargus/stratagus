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
//      (c) Copyright 2002-2008 by Lutz Sammer and Jimmy Salmon
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

#include "stratagus.h"

#include "editor.h"

#include "commands.h"
#include "font.h"
#include "game.h"
#include "guichan.h"
#include "interface.h"
#include "iocompat.h"
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


extern void DoScrollArea(int state, bool fast);
extern void DrawGuichanWidgets();
extern void CleanGame();

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

static char TileToolRandom;      /// Tile tool draws random
static char TileToolDecoration;  /// Tile tool draws with decorations
static int TileCursorSize;       /// Tile cursor size 1x1 2x2 ... 4x4
static bool UnitPlacedThisPress = false;  /// Only allow one unit per press
static bool UpdateMinimap = false;        /// Update units on the minimap
static int MirrorEdit = 0;                /// Mirror editing enabled
static int VisibleUnitIcons;              /// Number of icons that are visible at a time
static int VisibleTileIcons;

enum _mode_buttons_ {
	SelectButton = 201,  /// Select mode button
	UnitButton,          /// Unit mode button
	TileButton,          /// Tile mode button
	StartButton
};

enum EditorActionType {
	EditorActionTypePlaceUnit,
	EditorActionTypeRemoveUnit
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

extern gcn::Gui *Gui;
static gcn::Container *editorContainer;
static gcn::Slider *editorUnitSlider;
static gcn::Slider *editorSlider;

class EditorUnitSliderListener : public gcn::ActionListener
{
public:
	virtual void action(const std::string &) {
		const int iconsPerStep = VisibleUnitIcons;
		const int steps = (Editor.ShownUnitTypes.size() + iconsPerStep - 1) / iconsPerStep;
		const double value = editorUnitSlider->getValue();
		for (int i = 1; i <= steps; ++i) {
			if (value <= (double)i / steps) {
				Editor.UnitIndex = iconsPerStep * (i - 1);
				break;
			}
		}
	}
};

static EditorUnitSliderListener *editorUnitSliderListener;

class EditorSliderListener : public gcn::ActionListener
{
public:
	virtual void action(const std::string &) {
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
};

static EditorSliderListener *editorSliderListener;

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
**  @param tile  Tile type to edit.
*/
static void EditTile(const Vec2i &pos, int tile)
{
	Assert(Map.Info.IsPointOnMap(pos));

	const CTileset &tileset = *Map.Tileset;
	const int baseTileIndex = tileset.findTileIndexByTile(tile);
	const int tileIndex = tileset.getTileNumber(baseTileIndex, TileToolRandom, TileToolDecoration);
	CMapField &mf = *Map.Field(pos);
	mf.setTileIndex(tileset, tileIndex, 0);
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
	if (unit == NULL) {
		DebugPrint("Unable to allocate Unit");
		return;
	}

	CBuildRestrictionOnTop *b = OnTopDetails(*unit, NULL);
	if (b && b->ReplaceOnBuild) {
		CUnitCache &unitCache = Map.Field(pos)->UnitCache;
		CUnitCache::iterator it = std::find_if(unitCache.begin(), unitCache.end(), HasSameTypeAs(*b->Parent));

		if (it != unitCache.end()) {
			CUnit &replacedUnit = **it;
			unit->ResourcesHeld = replacedUnit.ResourcesHeld; // We capture the value of what is beneath.
			replacedUnit.Remove(NULL); // Destroy building beneath
			UnitLost(replacedUnit);
			UnitClearOrders(replacedUnit);
			replacedUnit.Release();
		}
	}
	if (unit != NULL) {
		if (type.GivesResource) {
			unit->ResourcesHeld = DefaultResourceAmounts[type.GivesResource];
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
	editorAction.Type = EditorActionTypePlaceUnit;
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
	unit.Remove(NULL);
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
	editorAction.Type = EditorActionTypeRemoveUnit;
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
		case EditorActionTypePlaceUnit: {
			CUnit *unit = UnitOnMapTile(action.tilePos, action.UnitType->UnitType);
			EditorActionRemoveUnit(*unit);
			break;
		}

		case EditorActionTypeRemoveUnit:
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
		case EditorActionTypePlaceUnit:
			EditorActionPlaceUnit(action.tilePos, *action.UnitType, action.Player);
			break;

		case EditorActionTypeRemoveUnit: {
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
**  Calculate the number of icons that can be displayed
**
**  @return  Number of icons that can be displayed.
*/
static int CalculateVisibleIcons(bool tiles = false)
{
#if 0
	int w;
	int h;

	if (tiles) {
		w = PixelTileSize.x;//+2,
		h = PixelTileSize.y;//+2
	} else {
		w = IconWidth;
		h = IconHeight;
	}

	const int i = (ButtonPanelHeight - h - 24) / (h + 2);
	const int count = i * (ButtonPanelWidth - w - 10) / (w + 8);
	return count;
#endif
	return UI.ButtonPanel.Buttons.size();
}

/**
**  Calculate the max height and the max width of icons,
**  and assign them to IconHeight and IconWidth
*/
static void CalculateMaxIconSize()
{
	IconWidth = 0;
	IconHeight = 0;
	for (unsigned int i = 0; i < Editor.UnitTypes.size(); ++i) {
		const CUnitType *type = UnitTypeByIdent(Editor.UnitTypes[i].c_str());
		Assert(type && type->Icon.Icon);
		const CIcon &icon = *type->Icon.Icon;

		IconWidth = std::max(IconWidth, icon.G->Width);
		IconHeight = std::max(IconHeight, icon.G->Height);
	}
}

/**
**  Recalculate the shown units.
*/
static void RecalculateShownUnits()
{
	Editor.ShownUnitTypes.clear();

	for (size_t i = 0; i != Editor.UnitTypes.size(); ++i) {
		const CUnitType *type = UnitTypeByIdent(Editor.UnitTypes[i].c_str());
		Editor.ShownUnitTypes.push_back(type);
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

/**
**  Draw a table with the players
*/
static void DrawPlayers()
{
	char buf[256];
	CLabel label(GetSmallFont());

	int x = UI.InfoPanel.X + 8;
	int y = UI.InfoPanel.Y + 4 + IconHeight + 10;

	for (int i = 0; i < PlayerMax; ++i) {
		if (i == PlayerMax / 2) {
			y += 20;
		}
		if (i == Editor.CursorPlayer && Map.Info.PlayerType[i] != PlayerNobody) {
			Video.DrawRectangle(ColorWhite, x + i % 8 * 20, y, 20, 20);
		}
		Video.DrawRectangle(
			i == Editor.CursorPlayer && Map.Info.PlayerType[i] != PlayerNobody ? ColorWhite : ColorGray,
			x + i % 8 * 20, y, 19, 19);
		if (Map.Info.PlayerType[i] != PlayerNobody) {
			Video.FillRectangle(Players[i].Color, x + 1 + i % 8 * 20, y + 1, 17, 17);
		}
		if (i == Editor.SelectedPlayer) {
			Video.DrawRectangle(ColorGreen, x + 1 + i % 8 * 20, y + 1, 17, 17);
		}
		sprintf(buf, "%d", i);
		label.DrawCentered(x + i % 8 * 20 + 10, y + 7, buf);
	}

	x = UI.InfoPanel.X + 4;
	y += 18 * 1 + 4;
	if (Editor.SelectedPlayer != -1) {
		snprintf(buf, sizeof(buf), "Plyr %d %s ", Editor.SelectedPlayer,
				 PlayerRaces.Name[Map.Info.PlayerSide[Editor.SelectedPlayer]].c_str());
		// Players[SelectedPlayer].RaceName);

		switch (Map.Info.PlayerType[Editor.SelectedPlayer]) {
			case PlayerNeutral:
				strcat_s(buf, sizeof(buf), "Neutral");
				break;
			case PlayerNobody:
			default:
				strcat_s(buf, sizeof(buf), "Nobody");
				break;
			case PlayerPerson:
				strcat_s(buf, sizeof(buf), "Person");
				break;
			case PlayerComputer:
			case PlayerRescuePassive:
			case PlayerRescueActive:
				strcat_s(buf, sizeof(buf), "Computer");
				break;
		}
		label.SetFont(GetGameFont());
		label.Draw(x, y, buf);
	}
}

#if 0
extern void DrawPopupUnitInfo(const CUnitType *type,
							  int player_index, CFont *font,
							  Uint32 backgroundColor, int buttonX, int buttonY);

static void DrawPopup()
{
	if (Editor.State == EditorEditUnit && Editor.CursorUnitIndex != -1) {
		std::string nc, rc;
		GetDefaultTextColors(nc, rc);
		DrawPopupUnitInfo(Editor.ShownUnitTypes[Editor.CursorUnitIndex],
						  Editor.SelectedPlayer, GetSmallFont(),
						  Video.MapRGB(TheScreen->format, 38, 38, 78),
						  Editor.PopUpX, Editor.PopUpY);
		SetDefaultTextColors(nc, rc);
	}
}
#endif

/**
**  Draw unit icons.
*/
static void DrawUnitIcons()
{
	int i = Editor.UnitIndex;

	for (size_t j = 0; j < UI.ButtonPanel.Buttons.size(); ++j) {
		const int x = UI.ButtonPanel.Buttons[j].X;
		const int y = UI.ButtonPanel.Buttons[j].Y;
		if (i >= (int) Editor.ShownUnitTypes.size()) {
			return;
		}
		CIcon &icon = *Editor.ShownUnitTypes[i]->Icon.Icon;
		const PixelPos pos(x, y);
		icon.DrawIcon(Players[Editor.SelectedPlayer], pos);

		Video.DrawRectangleClip(ColorGray, x, y, icon.G->Width, icon.G->Height);
		if (i == Editor.SelectedUnitIndex) {
			Video.DrawRectangleClip(ColorGreen, x + 1, y + 1,
									icon.G->Width - 2, icon.G->Height - 2);
		}
		if (i == Editor.CursorUnitIndex) {
			Video.DrawRectangleClip(ColorWhite, x - 1, y - 1,
									icon.G->Width + 2, icon.G->Height + 2);
			Editor.PopUpX = x;
			Editor.PopUpY = y;
		}
		++i;
	}
}

/**
**  Draw a tile icon
**
**  @param tilenum  Tile number to display
**  @param x        X display position
**  @param y        Y display position
**  @param flags    State of the icon (::IconActive,::IconClicked,...)
*/
static void DrawTileIcon(unsigned tilenum, unsigned x, unsigned y, unsigned flags)
{
	Uint32 color = (flags & IconActive) ? ColorGray : ColorBlack;

	Video.DrawRectangleClip(color, x, y, PixelTileSize.x + 7, PixelTileSize.y + 7);
	Video.DrawRectangleClip(ColorBlack, x + 1, y + 1, PixelTileSize.x + 5, PixelTileSize.y + 5);

	Video.DrawVLine(ColorGray, x + PixelTileSize.x + 4, y + 5, PixelTileSize.y - 1); // _|
	Video.DrawVLine(ColorGray, x + PixelTileSize.x + 5, y + 5, PixelTileSize.y - 1);
	Video.DrawHLine(ColorGray, x + 5, y + PixelTileSize.y + 4, PixelTileSize.x + 1);
	Video.DrawHLine(ColorGray, x + 5, y + PixelTileSize.y + 5, PixelTileSize.x + 1);

	color = (flags & IconClicked) ? ColorGray : ColorWhite;
	Video.DrawHLine(color, x + 5, y + 3, PixelTileSize.x + 1);
	Video.DrawHLine(color, x + 5, y + 4, PixelTileSize.x + 1);
	Video.DrawVLine(color, x + 3, y + 3, PixelTileSize.y + 3);
	Video.DrawVLine(color, x + 4, y + 3, PixelTileSize.y + 3);

	if (flags & IconClicked) {
		++x;
		++y;
	}

	x += 4;
	y += 4;
	Map.TileGraphic->DrawFrameClip(Map.Tileset->tiles[tilenum].tile, x, y);

	if (flags & IconSelected) {
		Video.DrawRectangleClip(ColorGreen, x, y, PixelTileSize.x, PixelTileSize.y);
	}
}

/**
**  Draw tile icons.
**
**  @todo for the start the solid tiles are hardcoded
**        If we have more solid tiles, than they fit into the panel, we need
**        some new ideas.
*/
static void DrawTileIcons()
{
	CLabel label(GetGameFont());
	int x = UI.InfoPanel.X + 46;
	int y = UI.InfoPanel.Y + 4 + IconHeight + 11;

	if (CursorOn == CursorOnButton && 300 <= ButtonUnderCursor && ButtonUnderCursor < 306) {
		Video.DrawRectangle(ColorGray, x - 42, y - 3 + (ButtonUnderCursor - 300) * 20, 100, 20);
	}

	if (TileCursorSize == 1) {
		label.DrawReverseCentered(x, y, "1x1");
	} else {
		label.DrawCentered(x, y, "1x1");
	}
	y += 20;
	if (TileCursorSize == 2) {
		label.DrawReverseCentered(x, y, "2x2");
	} else {
		label.DrawCentered(x, y, "2x2");
	}
	y += 20;
	if (TileCursorSize == 3) {
		label.DrawReverseCentered(x, y, "3x3");
	} else {
		label.DrawCentered(x, y, "3x3");
	}
	y += 20;
	if (TileCursorSize == 4) {
		label.DrawReverseCentered(x, y, "4x4");
	} else {
		label.DrawCentered(x, y, "4x4");
	}
	y += 20;
	if (TileToolRandom) {
		label.DrawReverseCentered(x, y, "Random");
	} else {
		label.DrawCentered(x, y, "Random");
	}
	y += 20;
	if (TileToolDecoration) {
		label.DrawReverseCentered(x, y, "Filler");
	} else {
		label.DrawCentered(x, y, "Filler");
	}
	y += 20;

	int i = Editor.TileIndex;
	Assert(Editor.TileIndex != -1);
	y = UI.ButtonPanel.Y + 24;
	while (y < UI.ButtonPanel.Y + ButtonPanelHeight - PixelTileSize.y) {
		if (i >= (int)Editor.ShownTileTypes.size()) {
			break;
		}
		x = UI.ButtonPanel.X + 10;
		while (x < UI.ButtonPanel.X + ButtonPanelWidth - PixelTileSize.x) {
			if (i >= (int) Editor.ShownTileTypes.size()) {
				break;
			}
			const unsigned int tile = Editor.ShownTileTypes[i];

			Map.TileGraphic->DrawFrameClip(tile, x, y);
			Video.DrawRectangleClip(ColorGray, x, y, PixelTileSize.x, PixelTileSize.y);

			if (i == Editor.SelectedTileIndex) {
				Video.DrawRectangleClip(ColorGreen, x + 1, y + 1,
										PixelTileSize.x - 2, PixelTileSize.y - 2);
			}
			if (i == Editor.CursorTileIndex) {
				Video.DrawRectangleClip(ColorWhite, x - 1, y - 1,
										PixelTileSize.x + 2, PixelTileSize.y + 2);
				Editor.PopUpX = x;
				Editor.PopUpY = y;
			}

			x += PixelTileSize.x + 8;
			++i;
		}
		y += PixelTileSize.y + 2;
	}
}

static void DrawEditorPanel_SelectIcon()
{
	const PixelPos pos(UI.InfoPanel.X + 4, UI.InfoPanel.Y + 4);
	CIcon *icon = Editor.Select.Icon;
	Assert(icon);
	unsigned int flag = (ButtonUnderCursor == SelectButton ? IconActive : 0)
						| (Editor.State == EditorSelecting ? IconSelected : 0);
	// FIXME: wrong button style
	icon->DrawUnitIcon(*UI.SingleSelectedButton->Style, flag, pos, "");
}

static void DrawEditorPanel_UnitsIcon()
{
	const PixelPos pos(UI.InfoPanel.X + 4 + UNIT_ICON_X, UI.InfoPanel.Y + 4 + UNIT_ICON_Y);
	CIcon *icon = Editor.Units.Icon;
	Assert(icon);
	unsigned int flag = (ButtonUnderCursor == UnitButton ? IconActive : 0)
						| (Editor.State == EditorEditUnit ? IconSelected : 0);
	// FIXME: wrong button style
	icon->DrawUnitIcon(*UI.SingleSelectedButton->Style, flag, pos, "");
}

static void DrawEditorPanel_StartIcon()
{
	int x = UI.InfoPanel.X + 4;
	int y = UI.InfoPanel.Y + 4;

	if (Editor.StartUnit) {
		CIcon *icon = Editor.StartUnit->Icon.Icon;
		Assert(icon);
		const PixelPos pos(x + START_ICON_X, y + START_ICON_Y);
		int flag = (ButtonUnderCursor == StartButton ? IconActive : 0)
				   | (Editor.State == EditorSetStartLocation ? IconSelected : 0);

		icon->DrawUnitIcon(*UI.SingleSelectedButton->Style, flag, pos, "");
	} else {
		//  No unit specified : draw a cross.
		//  Todo : FIXME Should we just warn user to define Start unit ?
		PushClipping();
		x += START_ICON_X + 1;
		y += START_ICON_Y + 1;
		if (ButtonUnderCursor == StartButton) {
			Video.DrawRectangleClip(ColorGray, x - 1, y - 1, IconHeight, IconHeight);
		}
		Video.FillRectangleClip(ColorBlack, x, y, IconHeight - 2, IconHeight - 2);

		const PixelPos lt(x, y);
		const PixelPos lb(x, y + IconHeight - 2);
		const PixelPos rt(x + IconHeight - 2, y);
		const PixelPos rb(x + IconHeight - 2, y + IconHeight - 2);
		const Uint32 color = PlayerColors[Editor.SelectedPlayer][0];

		Video.DrawLineClip(color, lt, rb);
		Video.DrawLineClip(color, rt, lb);
		PopClipping();
	}
}

/**
**  Draw the editor panels.
*/
static void DrawEditorPanel()
{
	DrawEditorPanel_SelectIcon();
	DrawEditorPanel_UnitsIcon();

	if (Editor.TerrainEditable) {
		const int x = UI.InfoPanel.X + TILE_ICON_X + 4;
		const int y = UI.InfoPanel.Y + TILE_ICON_Y + 4;

		DrawTileIcon(0x10 + 4 * 16, x, y,
					 (ButtonUnderCursor == TileButton ? IconActive : 0) |
					 (Editor.State == EditorEditTile ? IconSelected : 0));
	}
	DrawEditorPanel_StartIcon();

	switch (Editor.State) {
		case EditorSelecting:
			break;
		case EditorEditTile:
			DrawTileIcons();
			break;
		case EditorSetStartLocation:
			DrawPlayers();
			break;
		case EditorEditUnit:
			DrawPlayers();
			DrawUnitIcons();
			break;
	}
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
			case EditorSelecting:
			case EditorEditTile:
				break;
			case EditorEditUnit:
				if (Editor.SelectedUnitIndex != -1) {
					CursorBuilding = const_cast<CUnitType *>(Editor.ShownUnitTypes[Editor.SelectedUnitIndex]);
				}
				break;
			case EditorSetStartLocation:
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

		if (Editor.State == EditorEditTile && Editor.SelectedTileIndex != -1) {
			const unsigned short tile = Editor.ShownTileTypes[Editor.SelectedTileIndex];
			PushClipping();
			UI.MouseViewport->SetClipping();

			PixelPos screenPosIt;
			for (int j = 0; j < TileCursorSize; ++j) {
				screenPosIt.y = screenPos.y + j * PixelTileSize.y;
				if (screenPosIt.y >= UI.MouseViewport->GetBottomRightPos().y) {
					break;
				}
				for (int i = 0; i < TileCursorSize; ++i) {
					screenPosIt.x = screenPos.x + i * PixelTileSize.x;
					if (screenPosIt.x >= UI.MouseViewport->GetBottomRightPos().x) {
						break;
					}
					Map.TileGraphic->DrawFrameClip(tile, screenPosIt.x, screenPosIt.y);
				}
			}
			Video.DrawRectangleClip(ColorWhite, screenPos.x, screenPos.y, PixelTileSize.x * TileCursorSize, PixelTileSize.y * TileCursorSize);
			PopClipping();
		} else {
			// If there is an unit under the cursor, it's selection thing
			//  is drawn somewhere else (Check DrawUnitSelection.)
			if (UnitUnderCursor != NULL) {
				PushClipping();
				UI.MouseViewport->SetClipping();
				Video.DrawRectangleClip(ColorWhite, screenPos.x, screenPos.y, PixelTileSize.x, PixelTileSize.y);
				PopClipping();
			}
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
			if (Map.Info.PlayerType[i] != PlayerNobody && Map.Info.PlayerType[i] != PlayerNeutral) {
				const PixelPos startScreenPos = vp->TilePosToScreen_TopLeft(Players[i].StartPos);

				if (type) {
					DrawUnitType(*type, type->Sprite, i, 0, startScreenPos);
				} else { // Draw a cross
					DrawCross(startScreenPos, PixelTileSize, PlayerColors[i][0]);
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
	CLabel(GetGameFont()).Draw(UI.StatusLine.TextX + 2, UI.StatusLine.TextY, buf);
	const CMapField &mf = *Map.Field(pos);
	//
	// Flags info
	//
	const unsigned flag = mf.getFlag();
	sprintf(buf, "%02X|%04X|%c%c%c%c%c%c%c%c%c%c%c%c%c",
			mf.Value, flag,
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
	CLabel(GetGameFont()).Draw(UI.StatusLine.TextX + 118, UI.StatusLine.TextY, buf);

	// Tile info
	const CTileset &tileset = *Map.Tileset;
	const int index = tileset.findTileIndexByTile(mf.getGraphicTile());
	Assert(index != -1);
	const int baseTerrainIdx = tileset.tiles[index].tileinfo.BaseTerrain;
	const char *baseTerrainStr = tileset.getTerrainName(baseTerrainIdx).c_str();
	const int mixTerrainIdx = tileset.tiles[index].tileinfo.MixTerrain;
	const char *mixTerrainStr = mixTerrainIdx ? tileset.getTerrainName(mixTerrainIdx).c_str() : "";
	snprintf(buf, sizeof(buf), "%s %s", baseTerrainStr, mixTerrainStr);
	CLabel(GetGameFont()).Draw(UI.StatusLine.TextX + 250, UI.StatusLine.TextY, buf);
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
					unit.Type->Name.c_str(), unit.Player->Index,
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

	DrawMapArea(); // draw the map area

	DrawStartLocations();

	// Fillers
	for (size_t i = 0; i != UI.Fillers.size(); ++i) {
		UI.Fillers[i].G->DrawClip(UI.Fillers[i].X, UI.Fillers[i].Y);
	}

	if (CursorOn == CursorOnMap && Gui->getTop() == editorContainer && !GamePaused) {
		DrawMapCursor(); // cursor on map
	}

	// Menu button
	const int flag_active = ButtonAreaUnderCursor == ButtonAreaMenu
							&& ButtonUnderCursor == ButtonUnderMenu ? MI_FLAGS_ACTIVE : 0;
	const int flag_clicked = GameMenuButtonClicked ? MI_FLAGS_CLICKED : 0;
	DrawUIButton(UI.MenuButton.Style,
				 flag_active | flag_clicked,
				 UI.MenuButton.X, UI.MenuButton.Y,
				 UI.MenuButton.Text);


	// Minimap
	if (UI.SelectedViewport) {
		UI.Minimap.Draw();
		UI.Minimap.DrawViewportArea(*UI.SelectedViewport);
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

	if (CursorOn == CursorOnMap) {
		DrawEditorInfo();
	}

	// Status line
	UI.StatusLine.Draw();

	DrawGuichanWidgets();

	// DrawPopup();

	DrawCursor();

	// refresh entire screen, so no further invalidate needed
	Invalidate();
	RealizeVideoMemory();
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
	if (CursorOn == CursorOnButton && ButtonAreaUnderCursor == ButtonAreaMenu &&
		(MouseButtons & LeftButton) && !GameMenuButtonClicked) {
		PlayGameSound(GameSounds.Click.Sound, MaxSampleVolume);
		GameMenuButtonClicked = true;
		return;
	}
	// Click on minimap
	if (CursorOn == CursorOnMinimap) {
		if (MouseButtons & LeftButton) { // enter move mini-mode
			const Vec2i tilePos = UI.Minimap.ScreenToTilePos(CursorScreenPos);
			UI.SelectedViewport->Center(Map.TilePosToMapPixelPos_Center(tilePos));
		}
		return;
	}
	// Click on mode area
	if (CursorOn == CursorOnButton) {
		CursorBuilding = NULL;
		switch (ButtonUnderCursor) {
			case SelectButton :
				Editor.State = EditorSelecting;
				editorUnitSlider->setVisible(false);
				editorSlider->setVisible(false);
				return;
			case UnitButton:
				Editor.State = EditorEditUnit;
				if (VisibleUnitIcons < (int)Editor.ShownUnitTypes.size()) {
					editorUnitSlider->setVisible(true);
				}
				editorSlider->setVisible(false);
				return;
			case TileButton :
				if (EditorEditTile) {
					Editor.State = EditorEditTile;
				}
				editorUnitSlider->setVisible(false);
				if (VisibleTileIcons < (int)Editor.ShownTileTypes.size()) {
					editorSlider->setVisible(true);
				}
				return;
			case StartButton:
				Editor.State = EditorSetStartLocation;
				editorUnitSlider->setVisible(false);
				editorSlider->setVisible(false);
				return;
			default:
				break;
		}
	}
	// Click on tile area
	if (Editor.State == EditorEditTile) {
		if (CursorOn == CursorOnButton && ButtonUnderCursor >= 100) {
			switch (ButtonUnderCursor) {
				case 300: TileCursorSize = 1; return;
				case 301: TileCursorSize = 2; return;
				case 302: TileCursorSize = 3; return;
				case 303: TileCursorSize = 4; return;
				case 304: TileToolRandom ^= 1; return;
				case 305: TileToolDecoration ^= 1; return;
			}
		}
		if (Editor.CursorTileIndex != -1) {
			Editor.SelectedTileIndex = Editor.CursorTileIndex;
			return;
		}
	}

	// Click on player area
	if (Editor.State == EditorEditUnit || Editor.State == EditorSetStartLocation) {
		// Cursor on player icons
		if (Editor.CursorPlayer != -1) {
			if (Map.Info.PlayerType[Editor.CursorPlayer] != PlayerNobody) {
				Editor.SelectedPlayer = Editor.CursorPlayer;
				ThisPlayer = Players + Editor.SelectedPlayer;
			}
			return;
		}
	}

	// Click on unit area
	if (Editor.State == EditorEditUnit) {
		// Cursor on unit icons
		if (Editor.CursorUnitIndex != -1) {
			Editor.SelectedUnitIndex = Editor.CursorUnitIndex;
			CursorBuilding = const_cast<CUnitType *>(Editor.ShownUnitTypes[Editor.CursorUnitIndex]);
			return;
		}
	}

	// Right click on a resource
	if (Editor.State == EditorSelecting) {
		if ((MouseButtons & RightButton) && UnitUnderCursor != NULL) {
			CclCommand("if (EditUnitProperties ~= nil) then EditUnitProperties() end;");
			return;
		}
	}

	// Click on map area
	if (CursorOn == CursorOnMap) {
		if (MouseButtons & RightButton) {
			if (Editor.State == EditorEditUnit && Editor.SelectedUnitIndex != -1) {
				Editor.SelectedUnitIndex = -1;
				CursorBuilding = NULL;
				return;
			} else if (Editor.State == EditorEditTile && Editor.SelectedTileIndex != -1) {
				Editor.SelectedTileIndex = -1;
				CursorBuilding = NULL;
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

			if (Editor.State == EditorEditTile &&
				Editor.SelectedTileIndex != -1) {
				EditTiles(tilePos, Editor.ShownTileTypes[Editor.SelectedTileIndex], TileCursorSize);
			} else if (Editor.State == EditorEditUnit) {
				if (!UnitPlacedThisPress && CursorBuilding) {
					if (CanBuildUnitType(NULL, *CursorBuilding, tilePos, 1)) {
						PlayGameSound(GameSounds.PlacementSuccess[ThisPlayer->Race].Sound,
									  MaxSampleVolume);
						EditorPlaceUnit(tilePos, *CursorBuilding, Players + Editor.SelectedPlayer);
						UnitPlacedThisPress = true;
						UI.StatusLine.Clear();
					} else {
						UI.StatusLine.Set(_("Unit can't be placed here."));
						PlayGameSound(GameSounds.PlacementError[ThisPlayer->Race].Sound,
									  MaxSampleVolume);
					}
				}
			} else if (Editor.State == EditorSetStartLocation) {
				Players[Editor.SelectedPlayer].StartPos = tilePos;
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
		key = '0' + ptr - UiGroupKeys.c_str();
		if (key > '9') {
			key = SDLK_BACKQUOTE;
		}
	}
	switch (key) {
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
			if (UnitUnderCursor != NULL) {
				EditorRemoveUnit(*UnitUnderCursor);
			}
			break;

		case SDLK_UP: // Keyboard scrolling
		case SDLK_KP8:
			KeyScrollState |= ScrollUp;
			break;
		case SDLK_DOWN:
		case SDLK_KP2:
			KeyScrollState |= ScrollDown;
			break;
		case SDLK_LEFT:
		case SDLK_KP4:
			KeyScrollState |= ScrollLeft;
			break;
		case SDLK_RIGHT:
		case SDLK_KP6:
			KeyScrollState |= ScrollRight;
			break;
		case '0':
			if (UnitUnderCursor != NULL) {
				UnitUnderCursor->ChangeOwner(Players[PlayerNumNeutral]);
				UI.StatusLine.Set(_("Unit owner modified"));
			}
			break;
		case '1': case '2':
		case '3': case '4': case '5':
		case '6': case '7': case '8':
		case '9':
			if (UnitUnderCursor != NULL && Map.Info.PlayerType[(int) key - '1'] != PlayerNobody) {
				UnitUnderCursor->ChangeOwner(Players[(int) key - '1']);
				UI.StatusLine.Set(_("Unit owner modified"));
				UpdateMinimap = true;
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
		case SDLK_KP8:
			KeyScrollState &= ~ScrollUp;
			break;
		case SDLK_DOWN:
		case SDLK_KP2:
			KeyScrollState &= ~ScrollDown;
			break;
		case SDLK_LEFT:
		case SDLK_KP4:
			KeyScrollState &= ~ScrollLeft;
			break;
		case SDLK_RIGHT:
		case SDLK_KP6:
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
	Assert(Editor.State == EditorEditUnit || Editor.State == EditorSetStartLocation);

	// Scrollbar
	if (UI.ButtonPanel.X + 4 < CursorScreenPos.x
		&& CursorScreenPos.x < UI.ButtonPanel.X + 176 - 4
		&& UI.ButtonPanel.Y + 4 < CursorScreenPos.y
		&& CursorScreenPos.y < UI.ButtonPanel.Y + 24) {
		return true;
	}
	int bx = UI.InfoPanel.X + 8;
	int by = UI.InfoPanel.Y + 4 + IconHeight + 10;
	for (int i = 0; i < PlayerMax; ++i) {
		if (i == PlayerMax / 2) {
			bx = UI.InfoPanel.X + 8;
			by += 20;
		}
		if (bx < screenPos.x && screenPos.x < bx + 20 && by < screenPos.y && screenPos.y < by + 20) {
			if (Map.Info.PlayerType[i] != PlayerNobody) {
				char buf[256];
				snprintf(buf, sizeof(buf), _("Select player #%d"), i);
				UI.StatusLine.Set(buf);
			} else {
				UI.StatusLine.Clear();
			}
			Editor.CursorPlayer = i;
#if 0
			ButtonUnderCursor = i + 100;
			CursorOn = CursorOnButton;
#endif
			return true;
		}
		bx += 20;
	}

	int i = Editor.UnitIndex;
	by = UI.ButtonPanel.Y + 24;
	for (size_t j = 0; j < UI.ButtonPanel.Buttons.size(); ++j) {
		const int x = UI.ButtonPanel.Buttons[j].X;
		const int y = UI.ButtonPanel.Buttons[j].Y;
		if (i >= (int) Editor.ShownUnitTypes.size()) {
			break;
		}
		if (x < screenPos.x && screenPos.x < x + IconWidth
			&& y < screenPos.y && screenPos.y < y + IconHeight) {
			char buf[256];
			snprintf(buf, sizeof(buf), "%s \"%s\"",
					 Editor.ShownUnitTypes[i]->Ident.c_str(),
					 Editor.ShownUnitTypes[i]->Name.c_str());
			UI.StatusLine.Set(buf);
			Editor.CursorUnitIndex = i;
			return true;
		}
		++i;
	}
	return false;
}

static bool EditorCallbackMouse_EditTileArea(const PixelPos &screenPos)
{
	int bx = UI.InfoPanel.X + 4;
	int by = UI.InfoPanel.Y + 4 + IconHeight + 10;

	for (int i = 0; i < 6; ++i) {
		if (bx < screenPos.x && screenPos.x < bx + 100 && by < screenPos.y && screenPos.y < by + 18) {
			ButtonUnderCursor = i + 300;
			CursorOn = CursorOnButton;
			return true;
		}
		by += 20;
	}

	int i = Editor.TileIndex;
	by = UI.ButtonPanel.Y + 24;
	while (by < UI.ButtonPanel.Y + ButtonPanelHeight - PixelTileSize.y) {
		if (i >= (int)Editor.ShownTileTypes.size()) {
			break;
		}
		bx = UI.ButtonPanel.X + 10;
		while (bx < UI.ButtonPanel.X + ButtonPanelWidth - PixelTileSize.x) {
			if (i >= (int)Editor.ShownTileTypes.size()) {
				break;
			}
			if (bx < screenPos.x && screenPos.x < bx + PixelTileSize.x
				&& by < screenPos.y && screenPos.y < by + PixelTileSize.y) {
				const int tile = Editor.ShownTileTypes[i];
				const int tileindex = Map.Tileset->findTileIndexByTile(tile);
				const int base = Map.Tileset->tiles[tileindex].tileinfo.BaseTerrain;
				UI.StatusLine.Set(Map.Tileset->getTerrainName(base));
				Editor.CursorTileIndex = i;
				return true;
			}
			bx += PixelTileSize.x + 8;
			i++;
		}
		by += PixelTileSize.y + 2;
	}
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
		UI.MouseViewport->Set(tilePos, PixelTileSize / 2);
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
	if (CursorOn == CursorOnMap && (MouseButtons & LeftButton)
		&& (Editor.State == EditorEditTile || Editor.State == EditorEditUnit)) {
		Vec2i vpTilePos = UI.SelectedViewport->MapPos;
		// Scroll the map
		if (CursorScreenPos.x <= UI.SelectedViewport->GetTopLeftPos().x) {
			vpTilePos.x--;
			UI.SelectedViewport->Set(vpTilePos, PixelTileSize / 2);
		} else if (CursorScreenPos.x >= UI.SelectedViewport->GetBottomRightPos().x) {
			vpTilePos.x++;
			UI.SelectedViewport->Set(vpTilePos, PixelTileSize / 2);
		}

		if (CursorScreenPos.y <= UI.SelectedViewport->GetTopLeftPos().y) {
			vpTilePos.y--;
			UI.SelectedViewport->Set(vpTilePos, PixelTileSize / 2);
		} else if (CursorScreenPos.y >= UI.SelectedViewport->GetBottomRightPos().y) {
			vpTilePos.y++;
			UI.SelectedViewport->Set(vpTilePos, PixelTileSize / 2);
		}

		// Scroll the map, if cursor moves outside the viewport.
		RestrictCursorToViewport();
		const Vec2i tilePos = UI.SelectedViewport->ScreenToTilePos(CursorScreenPos);

		if (Editor.State == EditorEditTile && Editor.SelectedTileIndex != -1) {
			EditTiles(tilePos, Editor.ShownTileTypes[Editor.SelectedTileIndex], TileCursorSize);
		} else if (Editor.State == EditorEditUnit && CursorBuilding) {
			if (!UnitPlacedThisPress) {
				if (CanBuildUnitType(NULL, *CursorBuilding, tilePos, 1)) {
					EditorPlaceUnit(tilePos, *CursorBuilding, Players + Editor.SelectedPlayer);
					UnitPlacedThisPress = true;
					UI.StatusLine.Clear();
				}
			}
		}
		return;
	}

	// Minimap move viewpoint
	if (CursorOn == CursorOnMinimap && (MouseButtons & LeftButton)) {
		RestrictCursorToMinimap();
		const Vec2i tilePos = UI.Minimap.ScreenToTilePos(CursorScreenPos);

		UI.SelectedViewport->Center(Map.TilePosToMapPixelPos_Center(tilePos));
		return;
	}

	MouseScrollState = ScrollNone;
	GameCursor = UI.Point.Cursor;
	CursorOn = CursorOnUnknown;
	Editor.CursorPlayer = -1;
	Editor.CursorUnitIndex = -1;
	Editor.CursorTileIndex = -1;
	ButtonUnderCursor = -1;

	// Minimap
	if (UI.Minimap.Contains(screenPos)) {
		CursorOn = CursorOnMinimap;
	}
	// Handle edit unit area
	if (Editor.State == EditorEditUnit || Editor.State == EditorSetStartLocation) {
		if (EditorCallbackMouse_EditUnitArea(screenPos) == true) {
			return;
		}
	}

	// Handle tile area
	if (Editor.State == EditorEditTile) {
		if (EditorCallbackMouse_EditTileArea(screenPos) == true) {
			return;
		}
	}

	// Handle buttons
	if (UI.InfoPanel.X + 4 < CursorScreenPos.x
		&& CursorScreenPos.x < UI.InfoPanel.X + 4 + Editor.Select.Icon->G->Width
		&& UI.InfoPanel.Y + 4 < CursorScreenPos.y
		&& CursorScreenPos.y < UI.InfoPanel.Y + 4 + Editor.Select.Icon->G->Width) {
		// FIXME: what is this button?
		ButtonAreaUnderCursor = -1;
		ButtonUnderCursor = SelectButton;
		CursorOn = CursorOnButton;
		UI.StatusLine.Set(_("Select mode"));
		return;
	}
	if (UI.InfoPanel.X + 4 + UNIT_ICON_X < CursorScreenPos.x
		&& CursorScreenPos.x < UI.InfoPanel.X + 4 + UNIT_ICON_X + Editor.Units.Icon->G->Width
		&& UI.InfoPanel.Y + 4 + UNIT_ICON_Y < CursorScreenPos.y
		&& CursorScreenPos.y < UI.InfoPanel.Y + 4 + UNIT_ICON_Y + Editor.Units.Icon->G->Height) {
		ButtonAreaUnderCursor = -1;
		ButtonUnderCursor = UnitButton;
		CursorOn = CursorOnButton;
		UI.StatusLine.Set(_("Unit mode"));
		return;
	}
	if (Editor.TerrainEditable) {
		if (UI.InfoPanel.X + 4 + TILE_ICON_X < CursorScreenPos.x
			&& CursorScreenPos.x < UI.InfoPanel.X + 4 + TILE_ICON_X + PixelTileSize.x + 7
			&& UI.InfoPanel.Y + 4 + TILE_ICON_Y < CursorScreenPos.y
			&& CursorScreenPos.y < UI.InfoPanel.Y + 4 + TILE_ICON_Y + PixelTileSize.y + 7) {
			ButtonAreaUnderCursor = -1;
			ButtonUnderCursor = TileButton;
			CursorOn = CursorOnButton;
			UI.StatusLine.Set(_("Tile mode"));
			return;
		}
	}

	int StartUnitWidth = Editor.StartUnit ?
						 Editor.StartUnit->Icon.Icon->G->Width : PixelTileSize.x + 7;
	int StartUnitHeight = Editor.StartUnit ?
						  Editor.StartUnit->Icon.Icon->G->Height : PixelTileSize.y + 7;
	if (UI.InfoPanel.X + 4 + START_ICON_X < CursorScreenPos.x
		&& CursorScreenPos.x < UI.InfoPanel.X + 4 + START_ICON_X + StartUnitWidth
		&& UI.InfoPanel.Y + 4 + START_ICON_Y < CursorScreenPos.y
		&& CursorScreenPos.y < UI.InfoPanel.Y + 4 + START_ICON_Y + StartUnitHeight) {
		ButtonAreaUnderCursor = -1;
		ButtonUnderCursor = StartButton;
		CursorOn = CursorOnButton;
		UI.StatusLine.Set(_("Set start location mode"));
		return;
	}
	if (UI.MenuButton.X != -1 && UI.MenuButton.Contains(screenPos)) {
		ButtonAreaUnderCursor = ButtonAreaMenu;
		ButtonUnderCursor = ButtonUnderMenu;
		CursorOn = CursorOnButton;
		return;
	}

	// Minimap
	if (UI.Minimap.Contains(screenPos)) {
		CursorOn = CursorOnMinimap;
		return;
	}

	// Map
	UnitUnderCursor = NULL;
	if (UI.MapArea.Contains(screenPos)) {
		CViewport *vp = GetViewport(screenPos);
		Assert(vp);
		if (UI.MouseViewport != vp) { // viewport changed
			UI.MouseViewport = vp;
			DebugPrint("active viewport changed to %ld.\n" _C_
					   static_cast<long int>(UI.Viewports - vp));
		}
		CursorOn = CursorOnMap;

		// Look if there is an unit under the cursor.
		const PixelPos cursorMapPos = UI.MouseViewport->ScreenToMapPixelPos(CursorScreenPos);
		UnitUnderCursor = UnitOnScreen(cursorMapPos.x, cursorMapPos.y);

		if (UnitUnderCursor != NULL) {
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
	const std::string filename = LibraryFileName(Parameters::Instance.luaEditorStartFilename.c_str());
	if (access(filename.c_str(), R_OK)) {
		fprintf(stderr, "Editor configuration file '%s' was not found\n"
				"Specify another with '-E file.lua'\n",
				Parameters::Instance.luaEditorStartFilename.c_str());
		ExitFatal(-1);
	}

	ShowLoadProgress(_("Script %s"), filename.c_str());
	LuaLoadFile(filename);
	LuaGarbageCollect();

	ThisPlayer = &Players[0];

	FlagRevealMap = 1; // editor without fog and all visible
	Map.NoFogOfWar = true;

	if (!*CurrentMapPath) { // new map!
		InitUnitTypes(1);
		//
		// Inititialize Map / Players.
		//
		InitPlayers();
		for (int i = 0; i < PlayerMax; ++i) {
			if (i == PlayerNumNeutral) {
				CreatePlayer(PlayerNeutral);
				Map.Info.PlayerType[i] = PlayerNeutral;
				Map.Info.PlayerSide[i] = Players[i].Race = 0;
			} else {
				CreatePlayer(PlayerNobody);
				Map.Info.PlayerType[i] = PlayerNobody;
			}
		}

		Map.Fields = new CMapField[Map.Info.MapWidth * Map.Info.MapHeight];

		const int defaultTile = Map.Tileset->getDefaultTileIndex();

		for (int i = 0; i < Map.Info.MapWidth * Map.Info.MapHeight; ++i) {
			Map.Fields[i].setTileIndex(*Map.Tileset, defaultTile, 0);
		}
		GameSettings.Resources = SettingsPresetMapDefault;
		CreateGame("", &Map);
	} else {
		CreateGame(CurrentMapPath, &Map);
	}

	ReplayRevealMap = 1;
	FlagRevealMap = 0;
	Editor.SelectedPlayer = PlayerNumNeutral;

	// Place the start points, which the loader discarded.
	for (int i = 0; i < PlayerMax; ++i) {
		if (Map.Info.PlayerType[i] != PlayerNobody) {
			// Set SelectedPlayer to a valid player
			if (Editor.SelectedPlayer == PlayerNumNeutral) {
				Editor.SelectedPlayer = i;
				break;
			}
		}
	}
	ButtonPanelWidth = 170;//200;
	ButtonPanelHeight = 160 + (Video.Height - 480);

	CalculateMaxIconSize();
	VisibleUnitIcons = CalculateVisibleIcons();

	if (!StartUnitName.empty()) {
		StartUnit = UnitTypeByIdent(StartUnitName);
	}
	Select.Icon = NULL;
	Select.Load();
	Units.Icon = NULL;
	Units.Load();

	Map.Tileset->fillSolidTiles(&Editor.ShownTileTypes);
	VisibleTileIcons = CalculateVisibleIcons(true);

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
	std::string fullName;
	fullName = StratagusLibPath + "/" + file;
	if (SaveStratagusMap(fullName, Map, Editor.TerrainEditable) == -1) {
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

	editorContainer = new gcn::Container();
	editorContainer->setDimension(gcn::Rectangle(0, 0, Video.Width, Video.Height));
	editorContainer->setOpaque(false);
	Gui->setTop(editorContainer);

	editorUnitSliderListener = new EditorUnitSliderListener();
	editorSliderListener = new EditorSliderListener();

	editorUnitSlider = new gcn::Slider();
	editorUnitSlider->setBaseColor(gcn::Color(38, 38, 78));
	editorUnitSlider->setForegroundColor(gcn::Color(200, 200, 120));
	editorUnitSlider->setBackgroundColor(gcn::Color(200, 200, 120));
	editorUnitSlider->setVisible(false);
	editorUnitSlider->addActionListener(editorUnitSliderListener);

	editorSlider = new gcn::Slider();
	editorSlider->setBaseColor(gcn::Color(38, 38, 78));
	editorSlider->setForegroundColor(gcn::Color(200, 200, 120));
	editorSlider->setBackgroundColor(gcn::Color(200, 200, 120));
	editorSlider->setVisible(false);
	editorSlider->addActionListener(editorSliderListener);

	UpdateMinimap = true;

	while (1) {
		Editor.MapLoaded = false;
		Editor.Running = EditorEditing;

		Editor.Init();

		if (first_init) {
			first_init = false;
			editorUnitSlider->setSize(ButtonPanelWidth/*176*/, 16);
			editorSlider->setSize(ButtonPanelWidth/*176*/, 16);
			editorContainer->add(editorUnitSlider, UI.ButtonPanel.X + 2, UI.ButtonPanel.Y - 16);
			editorContainer->add(editorSlider, UI.ButtonPanel.X + 2, UI.ButtonPanel.Y - 16);
		}
		//ProcessMenu("menu-editor-tips", 1);
		InterfaceState = IfaceStateNormal;

		SetVideoSync();

		GameCursor = UI.Point.Cursor;
		InterfaceState = IfaceStateNormal;
		Editor.State = EditorSelecting;
		UI.SelectedViewport = UI.Viewports;
		TileCursorSize = 1;

		while (Editor.Running) {
			CheckMusicFinished();

			if (FrameCounter % FRAMES_PER_SECOND == 0) {
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
				DoScrollArea(MouseScrollState, 0);
			}
			if (UI.KeyScroll) {
				DoScrollArea(KeyScrollState, (KeyModifiers & ModifierControl) != 0);
				if (CursorOn == CursorOnMap && (MouseButtons & LeftButton) &&
					(Editor.State == EditorEditTile ||
					 Editor.State == EditorEditUnit)) {
					EditorCallbackButtonDown(0);
				}
			}

			WaitEventsOneFrame();
		}
		CursorBuilding = NULL;
		if (!Editor.MapLoaded) {
			break;
		}

		CleanModules();

		LoadCcl(Parameters::Instance.luaStartFilename); // Reload the main config file

		PreMenuSetup();

		InterfaceState = IfaceStateMenu;
		GameCursor = UI.Point.Cursor;

		Video.ClearScreen();
		Invalidate();
	}

	CommandLogDisabled = OldCommandLogDisabled;
	SetCallbacks(old_callbacks);
	Gui->setTop(oldTop);
	delete editorContainer;
	delete editorUnitSliderListener;
	delete editorSliderListener;
	delete editorUnitSlider;
	delete editorSlider;
}

/**
**  Start the editor
**
**  @param filename  Map to load, NULL to create a new map
*/
void StartEditor(const char *filename)
{
	std::string nc, rc;

	GetDefaultTextColors(nc, rc);
	if (filename) {
		if (strcpy_s(CurrentMapPath, sizeof(CurrentMapPath), filename) != 0) {
			filename = NULL;
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
