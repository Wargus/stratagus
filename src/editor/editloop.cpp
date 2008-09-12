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
//      $Id$

//@{

/*----------------------------------------------------------------------------
--  Includes
----------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <sstream>
#include <deque>

#include "stratagus.h"
#include "unittype.h"
#include "video.h"
#include "map.h"
#include "tileset.h"
#include "minimap.h"
#include "settings.h"
#include "network.h"
#include "sound_server.h"
#include "ui.h"
#include "interface.h"
#include "font.h"
#include "widgets.h"
#include "editor.h"
#include "results.h"
#include "menus.h"
#include "sound.h"
#include "iolib.h"
#include "iocompat.h"
#include "commands.h"
#include "guichan.h"
#include "replay.h"

#include "script.h"

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
static bool UnitPlacedThisPress = false;    /// Only allow one unit per press
static bool UpdateMinimap = false;          /// Update units on the minimap
static int MirrorEdit = 0;                /// Mirror editing enabled
static int VisibleUnitIcons;                    /// Number of icons that are visible at a time
static int VisibleTileIcons;

enum _mode_buttons_ {
	SelectButton = 201,  /// Select mode button
	UnitButton,          /// Unit mode button
	TileButton,          /// Tile mode button
	StartButton
};

enum EditorActionType
{
	EditorActionTypePlaceUnit,
	EditorActionTypeRemoveUnit,
};

struct EditorAction
{
	EditorActionType Type;
	int X;
	int Y;
	CUnitType *UnitType;
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
	virtual void action(const std::string &eventId) {
		int iconsPerStep = VisibleUnitIcons;
		int steps = (Editor.ShownUnitTypes.size() + iconsPerStep - 1) / iconsPerStep;
		double value = editorUnitSlider->getValue();
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
	virtual void action(const std::string &eventId) {
		int iconsPerStep = VisibleTileIcons;
		int steps = (Editor.ShownTileTypes.size() + iconsPerStep - 1) / iconsPerStep;
		double value = editorSlider->getValue();
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
**  Get tile number.
**
**  @param basic   Basic tile number
**  @param random  Return random tile
**  @param filler  Get a decorated tile.
**
**  @return        Tile number used in pud.
**
**  @todo  FIXME: Solid tiles are here still hardcoded.
*/
static int GetTileNumber(int basic, int random, int filler)
{
	int tile = basic;
	if (random) {
		int i,n;
		for (n = i = 0; i < 16; ++i) {
			if (!Map.Tileset.Table[tile + i]) {
				if (!filler) {
					break;
				}
			} else {
				++n;
			}
		}
		n = MyRand() % n;
		i = -1;
		do {
			while (++i < 16 && !Map.Tileset.Table[tile + i]) {
			}
		} while (i < 16 && n--);
		Assert(i != 16);
		return tile + i;
	}
	if (filler) {
		int i = 0;
		for (; i < 16 && Map.Tileset.Table[tile + i]; ++i) {
		}
		for (; i < 16 && !Map.Tileset.Table[tile + i]; ++i) {
		}
		if (i != 16) {
			return tile + i;
		}
	}
	return tile;
}

/**
**  Edit tile.
**
**  @param x     X map tile coordinate.
**  @param y     Y map tile coordinate.
**  @param tile  Tile type to edit.
*/
void EditTile(int x, int y, int tile)
{
	CMapField *mf;

	Assert(x >= 0 && y >= 0 && x < Map.Info.MapWidth && y < Map.Info.MapHeight);

	ChangeTile(x, y, GetTileNumber(tile, TileToolRandom, TileToolDecoration));

	//
	// Change the flags
	//
	mf = Map.Field(x, y);
	mf->Flags &= ~(MapFieldHuman | MapFieldLandAllowed | MapFieldCoastAllowed |
		MapFieldWaterAllowed | MapFieldNoBuilding | MapFieldUnpassable |
		MapFieldWall | MapFieldRocks | MapFieldForest);

	mf->Flags |= Map.Tileset.FlagsTable[GetTileNumber(tile, 0, 0)];

	UI.Minimap.UpdateSeenXY(x, y);
	UI.Minimap.UpdateXY(x, y);

	EditorTileChanged(x, y);
	UpdateMinimap = true;
}

/**
**  Edit tiles (internal, used by EditTiles()).
**
**  @param x     X map tile coordinate.
**  @param y     Y map tile coordinate.
**  @param tile  Tile type to edit.
**  @param size  Size of rectangle
**
**  @bug  This function does not support mirror editing!
*/
void EditTilesInternal(int x, int y, int tile, int size)
{
	int ex;
	int ey;
	int i;

	ex = x + size;
	if (x < 0) {
		x = 0;
	}
	if (ex > Map.Info.MapWidth) {
		ex = Map.Info.MapWidth;
	}
	ey = y + size;
	if (y < 0) {
		y = 0;
	}
	if (ey > Map.Info.MapHeight) {
		ey = Map.Info.MapHeight;
	}
	while (y < ey) {
		for (i = x; i < ex; ++i) {
			EditTile(i, y, tile);
		}
		++y;
	}
}

/**
**  Edit tiles
**
**  @param x     X map tile coordinate.
**  @param y     Y map tile coordinate.
**  @param tile  Tile type to edit.
**  @param size  Size of rectangle
*/
void EditTiles(int x, int y, int tile, int size)
{
	int mx;
	int my;

	mx = Map.Info.MapWidth;
	my = Map.Info.MapHeight;

	EditTilesInternal(x, y, tile, size);

	if (!MirrorEdit) {
		return;
	}

	EditTilesInternal(mx - x - size, y, tile, size);

	if (MirrorEdit == 1) {
		return;
	}

	EditTilesInternal(x, my - y - size, tile, size);
	EditTilesInternal(mx - x - size, my - y - size, tile, size);
}

/*----------------------------------------------------------------------------
--  Actions
----------------------------------------------------------------------------*/

/**
**  Place unit.
**
**  @param x       X map tile coordinate.
**  @param y       Y map tile coordinate.
**  @param type    Unit type to edit.
**  @param player  Player owning the unit.
**
**  @todo  FIXME: Check if the player has already a start-point.
**  @bug   This function does not support mirror editing!
*/
static void EditorActionPlaceUnit(int x, int y, CUnitType *type, CPlayer *player)
{
	CUnit *unit;
	CBuildRestrictionOnTop *b;

	if (type->Neutral) {
		player = &Players[PlayerNumNeutral];
	}

	// FIXME: vladi: should check place when mirror editing is enabled...?
	unit = MakeUnitAndPlace(x, y, type, player);
	if (unit == NoUnitP) {
		DebugPrint("Unable to allocate Unit");
		return;
	}

	b = OnTopDetails(unit, NULL);
	if (b && b->ReplaceOnBuild) {
		int n;
		CUnit *table[UnitMax];

		//FIXME: rb: use tile functor find here.
		n = Map.Select(x, y, table);
		while (n--) {
			if (table[n]->Type == b->Parent) {
				unit->ResourcesHeld = table[n]->ResourcesHeld; // We capture the value of what is beneath.
				table[n]->Remove(NULL); // Destroy building beneath
				UnitLost(table[n]);
				UnitClearOrders(table[n]);
				table[n]->Release();
				break;
			}
		}

	}
	if (unit != NoUnitP) {
		if (type->GivesResource) {
			unit->ResourcesHeld = DefaultResourceAmounts[type->GivesResource];
		}
	} else {
		DebugPrint("Unable to allocate Unit");
	}
	UpdateMinimap = true;
}

/**
**  Edit unit.
**
**  @param x       X map tile coordinate.
**  @param y       Y map tile coordinate.
**  @param type    Unit type to edit.
**  @param player  Player owning the unit.
*/
static void EditorPlaceUnit(int x, int y, CUnitType *type, CPlayer *player)
{
	EditorAction editorAction;
	editorAction.Type = EditorActionTypePlaceUnit;
	editorAction.X = x;
	editorAction.Y = y;
	editorAction.UnitType = type;
	editorAction.Player = player;

	EditorActionPlaceUnit(x, y, type, player);
	EditorAddUndoAction(editorAction);
}

/**
**  Remove a unit
*/
static void EditorActionRemoveUnit(CUnit *unit)
{
	unit->Remove(NULL);
	UnitLost(unit);
	UnitClearOrders(unit);
	unit->Release();
	UI.StatusLine.Set(_("Unit deleted"));
	UpdateMinimap = true;
}

/**
**  Remove a unit
*/
static void EditorRemoveUnit(CUnit *unit)
{
	EditorAction editorAction;
	editorAction.Type = EditorActionTypeRemoveUnit;
	editorAction.X = unit->X;
	editorAction.Y = unit->Y;
	editorAction.UnitType = unit->Type;
	editorAction.Player = unit->Player;

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

	switch (action.Type)
	{
		case EditorActionTypePlaceUnit:
		{
			CUnit *unit = UnitOnMapTile(action.X, action.Y, action.UnitType->UnitType);
			EditorActionRemoveUnit(unit);
			break;
		}

		case EditorActionTypeRemoveUnit:
			EditorActionPlaceUnit(action.X, action.Y, action.UnitType, action.Player);
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

	switch (action.Type)
	{
		case EditorActionTypePlaceUnit:
			EditorActionPlaceUnit(action.X, action.Y, action.UnitType, action.Player);
			break;

		case EditorActionTypeRemoveUnit:
		{
			CUnit *unit = UnitOnMapTile(action.X, action.Y, action.UnitType->UnitType);
			EditorActionRemoveUnit(unit);
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
	int i,w,h;
	int x;
	int count;

	if (tiles) {
		w = TileSizeX;//+2,
		h = TileSizeY;//+2
	} else {
		w = IconWidth;
		h = IconHeight;
	}

	i = 0;
	count = 0;
	x = UI.ButtonPanel.Y + 24;
	while (x < UI.ButtonPanel.Y + ButtonPanelHeight - h) {
		++i;
		x += h + 2;
	}
	x = UI.ButtonPanel.X + 10;
	while (x < UI.ButtonPanel.X + ButtonPanelWidth - w) {
		count += i;
		x += w + 8;
	}
	return count;
}

/**
**  Calculate the max height and the max width of icons,
**  and assign them to IconHeight and IconWidth
*/
static void CalculateMaxIconSize(void)
{
	const CUnitType *type;
	const CIcon *icon;

	IconWidth = 0;
	IconHeight = 0;
	for (int i = 0; i < (int)Editor.UnitTypes.size(); ++i) {
		type = UnitTypeByIdent(Editor.UnitTypes[i].c_str());
		Assert(type && type->Icon.Icon);
		icon = type->Icon.Icon;
		if (IconWidth < icon->G->Width) {
			IconWidth = icon->G->Width;
		}
		if (IconHeight < icon->G->Height) {
			IconHeight = icon->G->Height;
		}
	}
}


/**
**  Recalculate the shown units.
*/
static void RecalculateShownUnits(void)
{
	const CUnitType *type;

	Editor.ShownUnitTypes.clear();

	for (int i = 0; i < (int)Editor.UnitTypes.size(); ++i) {
		type = UnitTypeByIdent(Editor.UnitTypes[i].c_str());
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
static void DrawPlayers(void)
{
	int i;
	char buf[256];

	int x = UI.InfoPanel.X + 8;
	int y = UI.InfoPanel.Y + 4 + IconHeight + 10;

	for (i = 0; i < PlayerMax; ++i) {
		if (i == PlayerMax / 2) {
			y += 20;
		}
		if (i == Editor.CursorPlayer && Map.Info.PlayerType[i] != PlayerNobody) {
			Video.DrawRectangle(ColorWhite, x + i % 8 * 20, y, 20, 20);
		}
		Video.DrawRectangle(
			i == Editor.CursorPlayer && Map.Info.PlayerType[i] != PlayerNobody ?
				ColorWhite : ColorGray,
			x + i % 8 * 20, y, 19, 19);
		if (Map.Info.PlayerType[i] != PlayerNobody) {
			Video.FillRectangle(Players[i].Color, x + 1 + i % 8 * 20, y + 1,
				17, 17);
		}
		if (i == Editor.SelectedPlayer) {
			Video.DrawRectangle(ColorGreen, x + 1 + i % 8 * 20, y + 1, 17, 17);
		}
		sprintf(buf, "%d", i);
		VideoDrawTextCentered(x + i % 8 * 20 + 10, y + 7, SmallFont, buf);
	}

	x = UI.InfoPanel.X + 4;
	y += 18 * 1 + 4;
	if (Editor.SelectedPlayer != -1) {
		i = sprintf(buf, "Plyr %d %s ", Editor.SelectedPlayer,
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

		VideoDrawText(x, y, GameFont, buf);
	}
}

extern void DrawPopupUnitInfo(const CUnitType *type, 
		int player_index, CFont *font,
		Uint32 backgroundColor, int buttonX, int buttonY);

static void DrawPopup(void) {
	if (Editor.State == EditorEditUnit && Editor.CursorUnitIndex != -1) {
		std::string nc, rc;
		GetDefaultTextColors(nc, rc);
		DrawPopupUnitInfo(Editor.ShownUnitTypes[Editor.CursorUnitIndex], 
				Editor.SelectedPlayer, SmallFont, 
				Video.MapRGB(TheScreen->format, 38, 38, 78),
				Editor.PopUpX, Editor.PopUpY);
		SetDefaultTextColors(nc, rc);
	}
}

/**
**  Draw unit icons.
*/
static void DrawUnitIcons(void)
{
	int x;
	int y;
	int i;
	CIcon *icon;

	//
	// Draw the unit selection buttons.
	//
	x = UI.InfoPanel.X + 10;
	y = UI.InfoPanel.Y + 140;

	//
	//  Draw the unit icons.
	//
	y = UI.ButtonPanel.Y + 24;
	i = Editor.UnitIndex;
	while (y < UI.ButtonPanel.Y + ButtonPanelHeight - IconHeight) {
		if (i >= (int)Editor.ShownUnitTypes.size()) {
			break;
		}
		x = UI.ButtonPanel.X + 10;
		while (x < UI.ButtonPanel.X + ButtonPanelWidth - IconWidth) {
			if (i >= (int) Editor.ShownUnitTypes.size()) {
				break;
			}
			icon = Editor.ShownUnitTypes[i]->Icon.Icon;
			icon->DrawIcon(Players + Editor.SelectedPlayer, x, y);

			Video.DrawRectangleClip(ColorGray, x, y, icon->G->Width, icon->G->Height);
			if (i == Editor.SelectedUnitIndex) {
				Video.DrawRectangleClip(ColorGreen, x + 1, y + 1,
					icon->G->Width - 2, icon->G->Height - 2);
			}
			if (i == Editor.CursorUnitIndex) {
				Video.DrawRectangleClip(ColorWhite, x - 1, y - 1,
					icon->G->Width + 2, icon->G->Height + 2);
				Editor.PopUpX = x;
				Editor.PopUpY = y;				
			}

			x += IconWidth + 8;
			++i;
		}
		y += IconHeight + 2;
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
	Uint32 color;

	color = (flags & IconActive) ? ColorGray : ColorBlack;

	Video.DrawRectangleClip(color, x, y, TileSizeX + 7, TileSizeY + 7);
	Video.DrawRectangleClip(ColorBlack, x + 1, y + 1, TileSizeX + 5, TileSizeY + 5);

	Video.DrawVLine(ColorGray, x + TileSizeX + 4, y + 5, TileSizeY - 1); // _|
	Video.DrawVLine(ColorGray, x + TileSizeX + 5, y + 5, TileSizeY - 1);
	Video.DrawHLine(ColorGray, x + 5, y + TileSizeY + 4, TileSizeX + 1);
	Video.DrawHLine(ColorGray, x + 5, y + TileSizeY + 5, TileSizeX + 1);

	color = (flags & IconClicked) ? ColorGray : ColorWhite;
	Video.DrawHLine(color, x + 5, y + 3, TileSizeX + 1);
	Video.DrawHLine(color, x + 5, y + 4, TileSizeX + 1);
	Video.DrawVLine(color, x + 3, y + 3, TileSizeY + 3);
	Video.DrawVLine(color, x + 4, y + 3, TileSizeY + 3);

	if (flags & IconClicked) {
		++x;
		++y;
	}

	x += 4;
	y += 4;
	Map.TileGraphic->DrawFrameClip(Map.Tileset.Table[tilenum], x, y);

	if (flags & IconSelected) {
		Video.DrawRectangleClip(ColorGreen, x, y, TileSizeX, TileSizeY);
	}
}

/**
**  Draw tile icons.
**
**  @todo for the start the solid tiles are hardcoded
**        If we have more solid tiles, than they fit into the panel, we need
**        some new ideas.
*/
static void DrawTileIcons(void)
{
	int x;
	int y;
	int i;

	x = UI.InfoPanel.X + 46;
	y = UI.InfoPanel.Y + 4 + IconHeight + 11;

	if (CursorOn == CursorOnButton &&
			ButtonUnderCursor >= 300 && ButtonUnderCursor < 306) {
		Video.DrawRectangle(ColorGray, x - 42,
				y - 3 + (ButtonUnderCursor - 300) * 20, 100, 20);
	}

	VideoDrawTextCentered(x, y, GameFont, "1x1");
	//MenuButtonG->DrawFrame(MBUTTON_GEM_SQUARE + (TileCursorSize == 1 ? 2 : 0), x + 40, y - 3);
	y += 20;
	VideoDrawTextCentered(x, y, GameFont, "2x2");
	//MenuButtonG->DrawFrame(MBUTTON_GEM_SQUARE + (TileCursorSize == 2 ? 2 : 0), x + 40, y - 3);
	y += 20;
	VideoDrawTextCentered(x, y, GameFont, "3x3");
	//MenuButtonG->DrawFrame(MBUTTON_GEM_SQUARE + (TileCursorSize == 3 ? 2 : 0), x + 40, y - 3);
	y += 20;
	VideoDrawTextCentered(x, y, GameFont, "4x4");
	//MenuButtonG->DrawFrame(MBUTTON_GEM_SQUARE + (TileCursorSize == 4 ? 2 : 0), x + 40, y - 3);
	y += 20;
	VideoDrawTextCentered(x, y, GameFont, "Random");
	//MenuButtonG->DrawFrame(MBUTTON_GEM_SQUARE + (TileToolRandom ? 2 : 0), x + 40, y - 3);
	y += 20;
	VideoDrawTextCentered(x, y, GameFont, "Filler");
	//MenuButtonG->DrawFrame(MBUTTON_GEM_SQUARE + (TileToolDecoration ? 2 : 0), x + 40, y - 3);
	y += 20;

	unsigned int tile;
	i = Editor.TileIndex;
	Assert(Editor.TileIndex != -1);
	y = UI.ButtonPanel.Y + 24;
	while (y < UI.ButtonPanel.Y + ButtonPanelHeight - TileSizeY) {
		if (i >= (int)Editor.ShownTileTypes.size()) {
			break;
		}
		x = UI.ButtonPanel.X + 10;
		while (x < UI.ButtonPanel.X + ButtonPanelWidth - TileSizeX) {
			if (i >= (int) Editor.ShownTileTypes.size()) {
				break;
			}
			tile = Editor.ShownTileTypes[i];

			Map.TileGraphic->DrawFrameClip(Map.Tileset.Table[tile], x, y);
			Video.DrawRectangleClip(ColorGray, x, y, TileSizeX, TileSizeY);

			if (i == Editor.SelectedTileIndex) {
				Video.DrawRectangleClip(ColorGreen, x + 1, y + 1,
					TileSizeX-2, TileSizeY-2);
			}
			if (i == Editor.CursorTileIndex) {
				Video.DrawRectangleClip(ColorWhite, x - 1, y - 1, 
					TileSizeX+2, TileSizeY+2);
				Editor.PopUpX = x;
				Editor.PopUpY = y;	
			}

			x += TileSizeX + 8;
			++i;
		}
		y += TileSizeY + 2;
	}
}

/**
**  Draw the editor panels.
*/
static void DrawEditorPanel(void)
{
	int x;
	int y;
	CIcon *icon;

	x = UI.InfoPanel.X + 4;
	y = UI.InfoPanel.Y + 4;

	//
	// Select / Units / Tiles / Start
	//
	icon = Editor.Select.Icon;
	Assert(icon);
	// FIXME: wrong button style
	icon->DrawUnitIcon(Players, UI.SingleSelectedButton->Style,
		(ButtonUnderCursor == SelectButton ? IconActive : 0) |
			(Editor.State == EditorSelecting ? IconSelected : 0),
		x, y, "");
	icon = Editor.Units.Icon;
	Assert(icon);
	// FIXME: wrong button style
	icon->DrawUnitIcon(Players, UI.SingleSelectedButton->Style,
		(ButtonUnderCursor == UnitButton ? IconActive : 0) |
			(Editor.State == EditorEditUnit ? IconSelected : 0),
		x + UNIT_ICON_X, y + UNIT_ICON_Y, "");

	if (Editor.TerrainEditable) {
		DrawTileIcon(0x10 + 4 * 16, x + TILE_ICON_X, y + TILE_ICON_Y,
			(ButtonUnderCursor == TileButton ? IconActive : 0) |
				(Editor.State == EditorEditTile ? IconSelected : 0));
	}

	if (Editor.StartUnit) {
		icon = Editor.StartUnit->Icon.Icon;
		Assert(icon);
		icon->DrawUnitIcon(Players, UI.SingleSelectedButton->Style,
			(ButtonUnderCursor == StartButton ? IconActive : 0) |
				(Editor.State == EditorSetStartLocation ? IconSelected : 0),
			x + START_ICON_X, y + START_ICON_Y, "");
	} else {
		//  No unit specified.
		//  Todo : FIXME Should we just warn user to define Start unit ?
		PushClipping();
		x += START_ICON_X + 1;
		y += START_ICON_Y + 1;
		if (ButtonUnderCursor == StartButton) {
			Video.DrawRectangleClip(ColorGray, x - 1, y - 1, IconHeight, IconHeight);
		}
		Video.FillRectangleClip(ColorBlack, x, y, IconHeight - 2, IconHeight - 2);
		Video.DrawLineClip(PlayerColors[Editor.SelectedPlayer][0], x, y, x + IconHeight - 2, y + IconHeight - 2);
		Video.DrawLineClip(PlayerColors[Editor.SelectedPlayer][0], x, y + IconHeight - 2, x + IconHeight - 2, y);
		PopClipping();
	}

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
static void DrawMapCursor(void)
{
	int x;
	int y;

	//
	//  Affect CursorBuilding if necessary.
	//  (Menu reset CursorBuilding)
	//
	if (!CursorBuilding) {
		switch (Editor.State) {
			case EditorSelecting:
			case EditorEditTile:
				break;
			case EditorEditUnit:
				if (Editor.SelectedUnitIndex != -1) {
					CursorBuilding = const_cast<CUnitType *> (Editor.ShownUnitTypes[Editor.SelectedUnitIndex]);
				}
				break;
			case EditorSetStartLocation:
				if (Editor.StartUnit) {
					CursorBuilding = const_cast<CUnitType *> (Editor.StartUnit);
				}
				break;
		}
	}

	//
	// Draw map cursor
	//
	if (UI.MouseViewport && !CursorBuilding) {
		x = UI.MouseViewport->Viewport2MapX(CursorX);
		y = UI.MouseViewport->Viewport2MapY(CursorY);
		x = UI.MouseViewport->Map2ViewportX(x);
		y = UI.MouseViewport->Map2ViewportY(y);
		if (Editor.State == EditorEditTile) {
			int i;
			int j;

			PushClipping();
			SetClipping(UI.MouseViewport->X, UI.MouseViewport->Y,
				UI.MouseViewport->EndX, UI.MouseViewport->EndY);
			for (j = 0; j < TileCursorSize; ++j) {
				int ty;

				ty = y + j * TileSizeY;
				if (ty >= UI.MouseViewport->EndY) {
					break;
				}
				for (i = 0; i < TileCursorSize; ++i) {
					int tx;

					tx = x + i * TileSizeX;
					if (tx >= UI.MouseViewport->EndX) {
						break;
					}
					Map.TileGraphic->DrawFrameClip(
						Map.Tileset.Table[Editor.ShownTileTypes[Editor.SelectedTileIndex]]
						, tx, ty);
				}
			}
			Video.DrawRectangleClip(ColorWhite, x, y, TileSizeX * TileCursorSize,
				TileSizeY * TileCursorSize);
			PopClipping();
		} else {
			//
			// If there is an unit under the cursor, it's selection thing
			//  is drawn somewhere else (Check DrawUnitSelection.)
			//
			if (!UnitUnderCursor) {
				PushClipping();
				SetClipping(UI.MouseViewport->X, UI.MouseViewport->Y,
					UI.MouseViewport->EndX, UI.MouseViewport->EndY);
				Video.DrawRectangleClip(ColorWhite, x, y, TileSizeX, TileSizeY);
				PopClipping();
			}
		}
	}
}

/**
**  Draw the start locations of all active players on the map
*/
static void DrawStartLocations(void)
{
	const CUnitType *type = Editor.StartUnit;
	for (const CViewport *vp = UI.Viewports; vp < UI.Viewports + UI.NumViewports; ++vp) {
		PushClipping();
		SetClipping(vp->X, vp->Y, vp->EndX, vp->EndY);

		for (int i = 0; i < PlayerMax; i++) {
			if (Map.Info.PlayerType[i] != PlayerNobody && Map.Info.PlayerType[i] != PlayerNeutral) {
				int x = vp->Map2ViewportX(Players[i].StartX);
				int y = vp->Map2ViewportY(Players[i].StartY);

				if (type) {
					DrawUnitType(type, type->Sprite, i, 0, x, y);
				} else {
					Video.DrawLineClip(PlayerColors[i][0], x, y, x + TileSizeX, y + TileSizeY);
					Video.DrawLineClip(PlayerColors[i][0], x, y + TileSizeY, x + TileSizeX, y);
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
static void DrawEditorInfo(void)
{
#if 0
	int tile;
	int i;
	int x;
	int y;
	unsigned flags;
	char buf[256];

	x = y = 0;
	if (UI.MouseViewport) {
		x = Viewport2MapX(UI.MouseViewport, CursorX);
		y = Viewport2MapY(UI.MouseViewport, CursorY);
	}

	snprintf(buf, sizeof(buf),"Editor (%d %d)", x, y);
	VideoDrawText(UI.ResourceX + 2, UI.ResourceY + 2, GameFont, buf);
	CMapField *mf = Map.Field(x, y);
	//
	// Flags info
	//
	flags = mf->Flags;
	sprintf(buf, "%02X|%04X|%c%c%c%c%c%c%c%c%c%c%c%c%c",
		mf->Value, flags,
		flags & MapFieldUnpassable   ? 'u' : '-',
		flags & MapFieldNoBuilding   ? 'n' : '-',
		flags & MapFieldHuman        ? 'h' : '-',
		flags & MapFieldWall         ? 'w' : '-',
		flags & MapFieldRocks        ? 'r' : '-',
		flags & MapFieldForest       ? 'f' : '-',
		flags & MapFieldLandAllowed  ? 'L' : '-',
		flags & MapFieldCoastAllowed ? 'C' : '-',
		flags & MapFieldWaterAllowed ? 'W' : '-',
		flags & MapFieldLandUnit     ? 'l' : '-',
		flags & MapFieldAirUnit      ? 'a' : '-',
		flags & MapFieldSeaUnit      ? 's' : '-',
		flags & MapFieldBuilding     ? 'b' : '-');
	VideoDrawText(UI.ResourceX + 118, UI.ResourceY + 2, GameFont, buf);

	//
	// Tile info
	//
	tile = mf->Tile;

	for (i = 0; i < Map.Tileset.NumTiles; ++i) {
		if (tile == Map.Tileset.Table[i]) {
			break;
		}
	}

	Assert(i != Map.Tileset.NumTiles);

	snprintf(buf, sizeof(buf),"%d %s %s", tile,
		Map.Tileset.SolidTerrainTypes[Map.Tileset.Tiles[i].BaseTerrain].TerrainName,
		Map.Tileset.Tiles[i].MixTerrain
			? Map.Tileset.SolidTerrainTypes[Map.Tileset.Tiles[i].MixTerrain].TerrainName
			: "");

	VideoDrawText(UI.ResourceX + 252, UI.ResourceY + 2, GameFont, buf);
#endif
}

/**
**  Show info about unit.
**
**  @param unit  Unit pointer.
*/
static void ShowUnitInfo(const CUnit *unit)
{
	char buf[256];
	int i;

	i = sprintf(buf, "#%d '%s' Player:#%d %s", UnitNumber(unit),
		unit->Type->Name.c_str(), unit->Player->Index,
		unit->Active ? "active" : "passive");
	if (unit->Type->GivesResource) {
		sprintf(buf + i," Amount %d", unit->ResourcesHeld);
	}
	UI.StatusLine.Set(buf);
}

/**
**  Update editor display.
*/
void EditorUpdateDisplay(void)
{
	DrawMapArea(); // draw the map area

	DrawStartLocations();

	//
	// Fillers
	//
	for (int i = 0; i < (int)UI.Fillers.size(); ++i) {
		UI.Fillers[i].G->DrawClip(UI.Fillers[i].X, UI.Fillers[i].Y);
	}

	if (CursorOn == CursorOnMap && Gui->getTop() == editorContainer && !GamePaused) {
		DrawMapCursor(); // cursor on map
	}

	//
	// Menu button
	//
	DrawMenuButton(UI.MenuButton.Style,
		(ButtonAreaUnderCursor == ButtonAreaMenu
			&& ButtonUnderCursor == ButtonUnderMenu ? MI_FLAGS_ACTIVE : 0) |
		(GameMenuButtonClicked ? MI_FLAGS_CLICKED : 0),
		UI.MenuButton.X, UI.MenuButton.Y,
		UI.MenuButton.Text);

	//
	// Minimap
	//
	if (UI.SelectedViewport) {
		UI.Minimap.Draw(UI.SelectedViewport->MapX, UI.SelectedViewport->MapY);
		UI.Minimap.DrawCursor(UI.SelectedViewport->MapX,
			UI.SelectedViewport->MapY);
	}
	//
	// Info panel
	//
	if (UI.InfoPanel.G) {
		UI.InfoPanel.G->DrawClip(UI.InfoPanel.X, UI.InfoPanel.Y);
	}
	//
	// Button panel
	//
	if (UI.ButtonPanel.G) {
		UI.ButtonPanel.G->DrawClip(UI.ButtonPanel.X, UI.ButtonPanel.Y);
	}
	DrawEditorPanel();

	if (CursorOn == CursorOnMap) {
		DrawEditorInfo();
	}

	//
	// Status line
	//
	UI.StatusLine.Draw();

	DrawGuichanWidgets();

	DrawPopup();

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
	//
	// Click on menu button
	//
	if (CursorOn == CursorOnButton && ButtonAreaUnderCursor == ButtonAreaMenu &&
			(MouseButtons & LeftButton) && !GameMenuButtonClicked) {
		PlayGameSound(GameSounds.Click.Sound, MaxSampleVolume);
		GameMenuButtonClicked = true;
		return;
	}
	//
	// Click on minimap
	//
	if (CursorOn == CursorOnMinimap) {
		if (MouseButtons & LeftButton) { // enter move mini-mode
			UI.SelectedViewport->Set(
				UI.Minimap.Screen2MapX(CursorX) -
					UI.SelectedViewport->MapWidth / 2,
				UI.Minimap.Screen2MapY(CursorY) -
					UI.SelectedViewport->MapHeight / 2, TileSizeX / 2, TileSizeY / 2);
		}
		return;
	}
	//
	// Click on mode area
	//
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
	//
	// Click on tile area
	//
	if (Editor.State == EditorEditTile) {
		if(CursorOn == CursorOnButton && ButtonUnderCursor >= 100) {
			switch (ButtonUnderCursor) {
				case 300:
					TileCursorSize = 1;
					return;
				case 301:
					TileCursorSize = 2;
					return;
				case 302:
					TileCursorSize = 3;
					return;
				case 303:
					TileCursorSize = 4;
					return;
				case 304:
					TileToolRandom ^= 1;
					return;
				case 305:
					TileToolDecoration ^= 1;
					return;
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

	//
	// Click on unit area
	//
	if (Editor.State == EditorEditUnit) {
		// Cursor on unit icons
		if (Editor.CursorUnitIndex != -1) {
			Editor.SelectedUnitIndex = Editor.CursorUnitIndex;
			CursorBuilding = const_cast<CUnitType *>(Editor.ShownUnitTypes[Editor.CursorUnitIndex]);
			return;
		}
	}

	//
	// Right click on a resource
	//
	if (Editor.State == EditorSelecting) {
		if ((MouseButtons & RightButton && UnitUnderCursor)) {
			CclCommand("if (EditUnitProperties ~= nil) then EditUnitProperties() end;");
			return;
		}
	}

	//
	// Click on map area
	//
	if (CursorOn == CursorOnMap) {
	
		if (MouseButtons & RightButton) {
			if (Editor.State == EditorEditUnit &&
				 Editor.SelectedUnitIndex != -1) {
				 Editor.SelectedUnitIndex = -1;
				 CursorBuilding = NULL;
				 return;
			} else if (Editor.State == EditorEditTile &&
					 Editor.SelectedTileIndex != -1) {
				 	Editor.SelectedTileIndex = -1;
				 	CursorBuilding = NULL;
				 	return;
				}
		}	
	
		CViewport *vp = GetViewport(CursorX, CursorY);
		Assert(vp);
		if ((MouseButtons & LeftButton) && UI.SelectedViewport != vp) {
			// viewport changed
			UI.SelectedViewport = vp;
		}

		if (MouseButtons & LeftButton) {
			int tileX = UI.MouseViewport->Viewport2MapX(CursorX);
			int tileY = UI.MouseViewport->Viewport2MapY(CursorY);				

			if (Editor.State == EditorEditTile &&
				Editor.SelectedTileIndex != -1) {
				EditTiles(tileX, tileY, 
					Editor.ShownTileTypes[Editor.SelectedTileIndex],
					TileCursorSize);
			} else if (Editor.State == EditorEditUnit) {
				if (!UnitPlacedThisPress && CursorBuilding) {
					if (CanBuildUnitType(NULL, CursorBuilding,
							tileX, tileY, 1)) {
						PlayGameSound(GameSounds.PlacementSuccess.Sound,
							MaxSampleVolume);
						EditorPlaceUnit(tileX, tileY,
							CursorBuilding, Players + Editor.SelectedPlayer);
						UnitPlacedThisPress = true;
						UI.StatusLine.Clear();
					} else {
						UI.StatusLine.Set(_("Unit can't be placed here."));
						PlayGameSound(GameSounds.PlacementError.Sound,
							MaxSampleVolume);
					}
				}
			} else if (Editor.State == EditorSetStartLocation) {
				Players[Editor.SelectedPlayer].StartX = tileX;
				Players[Editor.SelectedPlayer].StartY = tileY;
			}
		} else if (MouseButtons & MiddleButton) {
			// enter move map mode
			CursorStartX = CursorX;
			CursorStartY = CursorY;
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
	char *ptr;

	if (HandleKeyModifiersDown(key, keychar)) {
		return;
	}

	// FIXME: don't handle unicode well. Should work on all latin keyboard.
	if ((ptr = strchr(UiGroupKeys, key))) {
		key = '0' + ptr - UiGroupKeys;
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

		case SDLK_DELETE: // Delete
			if (UnitUnderCursor) {
				EditorRemoveUnit(UnitUnderCursor);
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
			if (UnitUnderCursor) {
				UnitUnderCursor->ChangeOwner(&Players[PlayerNumNeutral]);
				UI.StatusLine.Set(_("Unit owner modified"));
			}
			break;
		case '1': case '2':
		case '3': case '4': case '5':
		case '6': case '7': case '8':
		case '9':
			if (UnitUnderCursor && Map.Info.PlayerType[(int) key - '1'] != PlayerNobody) {
				UnitUnderCursor->ChangeOwner(&Players[(int) key - '1']);
				UI.StatusLine.Set(_("Unit owner modified"));
				UpdateMinimap = true;
			}
			break;

		default:
			HandleCommandKey(key);
			return;
	}
	return;
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
static void EditorCallbackKeyRepeated(unsigned key, unsigned keychar)
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

/**
**  Callback for input movement of the cursor.
**
**  @param x  Screen X position.
**  @param y  Screen Y position.
*/
static void EditorCallbackMouse(int x, int y)
{
	int i;
	int bx;
	int by;
	static int LastMapX;
	static int LastMapY;
	enum _cursor_on_ OldCursorOn;
	char buf[256];

	HandleCursorMove(&x, &y); // Reduce to screen

	//
	// Move map.
	//
	if (GameCursor == UI.Scroll.Cursor) {
		int xo;
		int yo;

		// FIXME: Support with CTRL for faster scrolling.
		// FIXME: code duplication, see ../ui/mouse.c
		xo = UI.MouseViewport->MapX;
		yo = UI.MouseViewport->MapY;
		if (UI.MouseScrollSpeedDefault < 0) {
			if (x < CursorStartX) {
				xo++;
			} else if (x > CursorStartX) {
				xo--;
			}
			if (y < CursorStartY) {
				yo++;
			} else if (y > CursorStartY) {
				yo--;
			}
		} else {
			if (x < CursorStartX) {
				xo--;
			} else if (x > CursorStartX) {
				xo++;
			}
			if (y < CursorStartY) {
				yo--;
			} else if (y > CursorStartY) {
				yo++;
			}
		}
		UI.MouseWarpX = CursorStartX;
		UI.MouseWarpY = CursorStartY;
		UI.MouseViewport->Set(xo, yo, TileSizeX / 2, TileSizeY / 2);
		return;
	}

	// Automatically unpress when map tile has changed
	if (LastMapX != UI.SelectedViewport->Viewport2MapX(CursorX) ||
			LastMapY != UI.SelectedViewport->Viewport2MapY(CursorY)) {
		LastMapX = UI.SelectedViewport->Viewport2MapX(CursorX);
		LastMapY = UI.SelectedViewport->Viewport2MapY(CursorY);
		UnitPlacedThisPress = false;
	}
	//
	// Drawing tiles on map.
	//
	if (CursorOn == CursorOnMap && (MouseButtons & LeftButton) &&
			(Editor.State == EditorEditTile || Editor.State == EditorEditUnit)) {

		//
		// Scroll the map
		//
		if (CursorX <= UI.SelectedViewport->X) {
			UI.SelectedViewport->Set(
				UI.SelectedViewport->MapX - 1,
				UI.SelectedViewport->MapY, TileSizeX / 2, TileSizeY / 2);
		} else if (CursorX >= UI.SelectedViewport->EndX) {
			UI.SelectedViewport->Set(
				UI.SelectedViewport->MapX + 1,
				UI.SelectedViewport->MapY, TileSizeX / 2, TileSizeY / 2);
		}

		if (CursorY <= UI.SelectedViewport->Y) {
			UI.SelectedViewport->Set(
				UI.SelectedViewport->MapX,
				UI.SelectedViewport->MapY - 1, TileSizeX / 2, TileSizeY / 2);
		} else if (CursorY >= UI.SelectedViewport->EndY) {
			UI.SelectedViewport->Set(
				UI.SelectedViewport->MapX,
				UI.SelectedViewport->MapY + 1, TileSizeX / 2, TileSizeY / 2);

		}

		//
		// Scroll the map, if cursor moves outside the viewport.
		//
		RestrictCursorToViewport();

		int tileX = UI.SelectedViewport->Viewport2MapX(CursorX);
		int tileY = UI.SelectedViewport->Viewport2MapY(CursorY);

		if (Editor.State == EditorEditTile) {
			EditTiles(tileX, tileY,
				Editor.ShownTileTypes[Editor.SelectedTileIndex],
				TileCursorSize);
		} else if (Editor.State == EditorEditUnit && CursorBuilding) {
			if (!UnitPlacedThisPress) {
				if (CanBuildUnitType(NULL, CursorBuilding, tileX, tileY, 1)) {
					EditorPlaceUnit(tileX, tileY, CursorBuilding,
					Players + Editor.SelectedPlayer);
					UnitPlacedThisPress = true;
					UI.StatusLine.Clear();
				}
			}
		}

		return;
	}

	//
	// Minimap move viewpoint
	//
	if (CursorOn == CursorOnMinimap && (MouseButtons & LeftButton)) {
		RestrictCursorToMinimap();
		UI.SelectedViewport->Set(
			UI.Minimap.Screen2MapX(CursorX)
				- UI.SelectedViewport->MapWidth / 2,
			UI.Minimap.Screen2MapY(CursorY)
				- UI.SelectedViewport->MapHeight / 2, 0, 0);
		return;
	}

	OldCursorOn = CursorOn;

	MouseScrollState = ScrollNone;
	GameCursor = UI.Point.Cursor;
	CursorOn = CursorOnUnknown;
	Editor.CursorPlayer = -1;
	Editor.CursorUnitIndex = -1;
	Editor.CursorTileIndex = -1;
	ButtonUnderCursor = -1;

	//
	// Minimap
	//
	if (x >= UI.Minimap.X && x < UI.Minimap.X + UI.Minimap.W &&
			y >= UI.Minimap.Y && y < UI.Minimap.Y + UI.Minimap.H) {
		CursorOn = CursorOnMinimap;
	}

	//
	// Handle edit unit area
	//
	if (Editor.State == EditorEditUnit || Editor.State == EditorSetStartLocation) {
		// Scrollbar
		if (UI.ButtonPanel.X + 4 < CursorX
				&& CursorX < UI.ButtonPanel.X + 176 - 4
				&& UI.ButtonPanel.Y + 4 < CursorY
				&& CursorY < UI.ButtonPanel.Y + 24) {
			return;
		}
		bx = UI.InfoPanel.X + 8;
		by = UI.InfoPanel.Y + 4 + IconHeight + 10;
		for (i = 0; i < PlayerMax; ++i) {
			if (i == PlayerMax / 2) {
				bx = UI.InfoPanel.X + 8;
				by += 20;
			}
			if (bx < x && x < bx + 20 && by < y && y < by + 20) {
				if (Map.Info.PlayerType[i] != PlayerNobody) {
					snprintf(buf,sizeof(buf),"Select player #%d",i);
					UI.StatusLine.Set(buf);
				} else {
					UI.StatusLine.Clear();
				}
				Editor.CursorPlayer = i;
#if 0
				ButtonUnderCursor = i + 100;
				CursorOn = CursorOnButton;
#endif
				return;
			}
			bx += 20;
		}

		i = Editor.UnitIndex;
		by = UI.ButtonPanel.Y + 24;
		while (by < UI.ButtonPanel.Y + ButtonPanelHeight - IconHeight) {
			if (i >= (int)Editor.ShownUnitTypes.size()) {
				break;
			}
			bx = UI.ButtonPanel.X + 10;
			while (bx < UI.ButtonPanel.X + 146) {
				if (i >= (int)Editor.ShownUnitTypes.size()) {
					break;
				}
				if (bx < x && x < bx + IconWidth &&
						by < y && y < by + IconHeight) {
					snprintf(buf,sizeof(buf),"%s \"%s\"",
						Editor.ShownUnitTypes[i]->Ident.c_str(),
						Editor.ShownUnitTypes[i]->Name.c_str());
					UI.StatusLine.Set(buf);
					Editor.CursorUnitIndex = i;
#if 0
					ButtonUnderCursor = i + 100;
					CursorOn = CursorOnButton;
#endif
					return;
				}
				bx += IconWidth + 8;
				i++;
			}
			by += IconHeight + 2;
		}
	}

	//
	// Handle tile area
	//
	if (Editor.State == EditorEditTile) {
		i = 0;
		bx = UI.InfoPanel.X + 4;
		by = UI.InfoPanel.Y + 4 + IconHeight + 10;

		while (i < 6) {
			if (bx < x && x < bx + 100 && by < y && y < by + 18) {
				ButtonUnderCursor = i + 300;
				CursorOn = CursorOnButton;
				return;
			}
			++i;
			by += 20;
		}	
	
		i = Editor.TileIndex;
		by = UI.ButtonPanel.Y + 24;
		while (by < UI.ButtonPanel.Y + ButtonPanelHeight - TileSizeY) {
			if (i >= (int)Editor.ShownTileTypes.size()) {
				break;
			}
			bx = UI.ButtonPanel.X + 10;
			while (bx < UI.ButtonPanel.X + ButtonPanelWidth - TileSizeX) {
			//while (bx < UI.ButtonPanel.X + 144) {
				if (i >= (int)Editor.ShownTileTypes.size()) {
					break;
				}
				if (bx < x && x < bx + TileSizeX &&
						by < y && y < by + TileSizeY) {
					int base = Map.Tileset.Tiles[Editor.ShownTileTypes[i]].BaseTerrain;
					UI.StatusLine.Set(Map.Tileset.SolidTerrainTypes[base].TerrainName);
					Editor.CursorTileIndex = i;
					return;
				}
				bx += TileSizeX + 8;
				i++;
			}
			by += TileSizeY + 2;
		}
	}

	//
	// Handle buttons
	//
	if (UI.InfoPanel.X + 4 < CursorX &&
			CursorX < UI.InfoPanel.X + 4 + Editor.Select.Icon->G->Width &&
			UI.InfoPanel.Y + 4 < CursorY &&
			CursorY < UI.InfoPanel.Y + 4 + Editor.Select.Icon->G->Width) {
		// FIXME: what is this button?
		ButtonAreaUnderCursor = -1;
		ButtonUnderCursor = SelectButton;
		CursorOn = CursorOnButton;
		UI.StatusLine.Set(_("Select mode"));
		return;
	}
	if (UI.InfoPanel.X + 4 + UNIT_ICON_X < CursorX &&
			CursorX < UI.InfoPanel.X + 4 + UNIT_ICON_X + Editor.Units.Icon->G->Width &&
			UI.InfoPanel.Y + 4 + UNIT_ICON_Y < CursorY &&
			CursorY < UI.InfoPanel.Y + 4 + UNIT_ICON_Y + Editor.Units.Icon->G->Height) {
		ButtonAreaUnderCursor = -1;
		ButtonUnderCursor = UnitButton;
		CursorOn = CursorOnButton;
		UI.StatusLine.Set(_("Unit mode"));
		return;
	}
	if (Editor.TerrainEditable) {
		if (UI.InfoPanel.X + 4 + TILE_ICON_X < CursorX &&
				CursorX < UI.InfoPanel.X + 4 + TILE_ICON_X + TileSizeX + 7 &&
				UI.InfoPanel.Y + 4 + TILE_ICON_Y < CursorY &&
				CursorY < UI.InfoPanel.Y + 4 + TILE_ICON_Y + TileSizeY + 7) {
			ButtonAreaUnderCursor = -1;
			ButtonUnderCursor = TileButton;
			CursorOn = CursorOnButton;
			UI.StatusLine.Set(_("Tile mode"));
			return;
		}
	}
	
	int StartUnitWidth = Editor.StartUnit ? 
			Editor.StartUnit->Icon.Icon->G->Width : TileSizeX + 7;
	int StartUnitHeight = Editor.StartUnit ? 
			Editor.StartUnit->Icon.Icon->G->Height : TileSizeY + 7;
	if (UI.InfoPanel.X + 4 + START_ICON_X < CursorX &&
			CursorX < UI.InfoPanel.X + 4 + START_ICON_X + StartUnitWidth &&
			UI.InfoPanel.Y + 4 + START_ICON_Y < CursorY &&
			CursorY < UI.InfoPanel.Y + 4 + START_ICON_Y + StartUnitHeight) {
		ButtonAreaUnderCursor = -1;
		ButtonUnderCursor = StartButton;
		CursorOn = CursorOnButton;
		UI.StatusLine.Set(_("Set start location mode"));
		return;
	}
	if (UI.MenuButton.X != -1) {
		if (x >= UI.MenuButton.X &&
				x <= UI.MenuButton.X + UI.MenuButton.Style->Width &&
				y > UI.MenuButton.Y &&
				y <= UI.MenuButton.Y + UI.MenuButton.Style->Height) {
			ButtonAreaUnderCursor = ButtonAreaMenu;
			ButtonUnderCursor = ButtonUnderMenu;
			CursorOn = CursorOnButton;
			return;
		}
	}

	//
	// Minimap
	//
	if (x >= UI.Minimap.X && x < UI.Minimap.X + UI.Minimap.W &&
			y >= UI.Minimap.Y && y < UI.Minimap.Y + UI.Minimap.H) {
		CursorOn = CursorOnMinimap;
		return;
	}

	//
	// Map
	//
	UnitUnderCursor = NULL;
	if (x >= UI.MapArea.X && x <= UI.MapArea.EndX &&
			y >= UI.MapArea.Y && y <= UI.MapArea.EndY) {
		CViewport *vp = GetViewport(x, y);
		Assert(vp);
		if (UI.MouseViewport != vp) { // viewport changed
			UI.MouseViewport = vp;
			DebugPrint("active viewport changed to %ld.\n" _C_
				static_cast<long int>(UI.Viewports - vp));
		}
		CursorOn = CursorOnMap;

		//
		// Look if there is an unit under the cursor.
		// FIXME: use Viewport2MapX Viewport2MapY
		//
		UnitUnderCursor = UnitOnScreen(NULL,
			CursorX - UI.MouseViewport->X +
				UI.MouseViewport->MapX * TileSizeX + UI.MouseViewport->OffsetX,
			CursorY - UI.MouseViewport->Y +
				UI.MouseViewport->MapY * TileSizeY + UI.MouseViewport->OffsetY);
		if (UnitUnderCursor) {
			ShowUnitInfo(UnitUnderCursor);
			return;
		}
	}
	//
	// Scrolling Region Handling
	//
	if (HandleMouseScrollArea(x, y)) {
		return;
	}

	// Not reached if cursor is inside the scroll area

	UI.StatusLine.Clear();
}

/**
**  Callback for exit.
*/
static void EditorCallbackExit(void)
{
}

/**
**  Create the tile icons
*/
static void CreateTileIcons(void)
{
	for(int i = 0; 0x10 + i < Map.Tileset.NumTiles; i+=16) {
		TileInfo *info = &Map.Tileset.Tiles[0x10 + i];
		if (info->BaseTerrain && !info->MixTerrain) {
			Editor.ShownTileTypes.push_back(0x10 + i);			
		}
	}	
}

/**
**  Clean up the tile icons
*/
static void CleanTileIcons(void)
{
	Editor.ShownTileTypes.clear();
}

/**
**  Create editor.
*/
void CEditor::Init(void)
{
	int i;
	char *file;
	char buf[PATH_MAX];
	CFile clf;

	//
	// Load and evaluate the editor configuration file
	// FIXME: the CLopen is very slow and repeats the work of LibraryFileName.
	//
	file = LibraryFileName(EditorStartFile, buf, sizeof(buf));
	if (clf.open(file, CL_OPEN_READ) != -1) {
		clf.close();
		ShowLoadProgress("Script %s", file);
		LuaLoadFile(file);
		CclGarbageCollect(0); // Cleanup memory after load
	}

	ThisPlayer = &Players[0];

	FlagRevealMap = 1; // editor without fog and all visible
	Map.NoFogOfWar = true;

	if (!*CurrentMapPath) { // new map!
		InitUnitTypes(1);
		//
		// Inititialize Map / Players.
		//
		InitPlayers();
		for (i = 0; i < PlayerMax; ++i) {
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
		Map.Visible[0] = new unsigned[Map.Info.MapWidth * Map.Info.MapHeight / 2];
		memset(Map.Visible[0], 0, Map.Info.MapWidth * Map.Info.MapHeight / 2 * sizeof(unsigned));

		for (i = 0; i < Map.Info.MapWidth * Map.Info.MapHeight; ++i) {
			Map.Fields[i].Tile = Map.Fields[i].SeenTile = 0;
			Map.Fields[i].Tile = Map.Fields[i].SeenTile =
				Map.Tileset.Table[0x50];
			Map.Fields[i].Flags = Map.Tileset.FlagsTable[0x50];
		}
		GameSettings.Resources = SettingsPresetMapDefault;
		CreateGame("", &Map);
	} else {
		CreateGame(CurrentMapPath, &Map);
	}

	ReplayRevealMap = 1;
	FlagRevealMap = 0;
	Editor.SelectedPlayer = PlayerNumNeutral;

	//
	// Place the start points, which the loader discarded.
	//
	for (i = 0; i < PlayerMax; ++i) {
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

	CreateTileIcons();
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
	if (SaveStratagusMap(fullName, &Map, Editor.TerrainEditable) == -1) {
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
void EditorMainLoop(void)
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

		if(first_init) {
			first_init = false;
			editorUnitSlider->setSize(ButtonPanelWidth/*176*/, 16);
			editorSlider->setSize(ButtonPanelWidth/*176*/, 16);
			editorContainer->add(editorUnitSlider, UI.ButtonPanel.X + 2, UI.ButtonPanel.Y + 4);
			editorContainer->add(editorSlider, UI.ButtonPanel.X + 2, UI.ButtonPanel.Y + 4);			
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

		LoadCcl(); // Reload the main config file

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
//		Map.Info.Description.clear();
//		Map.Info.MapWidth = 64;
//		Map.Info.MapHeight = 64;
	}

	// Run the editor.
	EditorMainLoop();

	// Clear screen
	Video.ClearScreen();
	Invalidate();

	Editor.TerrainEditable = true;

	CleanTileIcons();
	CleanGame();
	CleanPlayers();

	SetDefaultTextColors(nc, rc);
}

//@}
