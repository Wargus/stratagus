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
/**@name editloop.c - The editor main loop. */
//
//      (c) Copyright 2002-2004 by Lutz Sammer and Jimmy Salmon
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
#include "editor.h"
#include "campaign.h"
#include "menus.h"
#include "sound.h"
#include "pud.h"
#include "iolib.h"
#include "iocompat.h"
#include "commands.h"

#include "script.h"

extern void DoScrollArea(enum _scroll_state_ state, int fast);

/*----------------------------------------------------------------------------
--  Defines
----------------------------------------------------------------------------*/

#define UNIT_ICON_X (IconWidth + 7)       /// Unit mode icon
#define UNIT_ICON_Y (0)                   /// Unit mode icon
#define TILE_ICON_X (IconWidth * 2 + 16)  /// Tile mode icon
#define TILE_ICON_Y (2)                   /// Tile mode icon

/*----------------------------------------------------------------------------
--  Variables
----------------------------------------------------------------------------*/

static int IconWidth;                       /// Icon width in panels
static int IconHeight;                      /// Icon height in panels


char EditorMapLoaded;  /// Map loaded in editor

EditorStateType EditorState;                /// Current editor state.
EditorRunningType EditorRunning;  /// Running State of editor.

static char TileToolRandom;      /// Tile tool draws random
static char TileToolDecoration;  /// Tile tool draws with decorations
static int TileCursorSize;       /// Tile cursor size 1x1 2x2 ... 4x4
static int TileCursor;           /// Tile type number

static int MirrorEdit = 0;           /// Mirror editing enabled
static int UnitPlacedThisPress = 0;  ///Only allow one unit per press

enum _mode_buttons_ {
	SelectButton = 201,  /// Select mode button
	UnitButton,          /// Unit mode button
	TileButton,          /// Tile mode button
};

char** EditorUnitTypes;  /// Sorted editor unit-type table
int MaxUnitIndex;        /// Max unit icon draw index

static char** ShownUnitTypes;       /// Shown editor unit-type table
static int MaxShownUnits;           /// Max unit icon draw index
static char ShowUnitsToSelect;      /// Show units in unit list
static char ShowBuildingsToSelect;  /// Show buildings in unit list
#if 0
static char ShowHeroesToSelect;     /// Show heroes in unit list
#endif
static char ShowAirToSelect;        /// Show air units in unit list
static char ShowLandToSelect;       /// Show land units in unit list
static char ShowWaterToSelect;      /// Show water units in unit list

static int UnitIndex;               /// Unit icon draw index
static int CursorUnitIndex;         /// Unit icon under cursor
static int SelectedUnitIndex;       /// Unit type to draw

static int CursorPlayer;            /// Player under the cursor
static int SelectedPlayer;          /// Player selected for draw

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
int GetTileNumber(int basic, int random, int filler)
{
	int tile;
	int i;
	int n;

	tile = 16 + basic * 16;
	if (random) {
		for (n = i = 0; i < 16; ++i) {
			if (!TheMap.Tileset->Table[tile + i]) {
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
			while (++i < 16 && !TheMap.Tileset->Table[tile + i]) {
			}
		} while (i < 16 && n--);
		Assert(i != 16);
		return tile + i;
	}
	if (filler) {
		for (i = 0; i < 16 && TheMap.Tileset->Table[tile + i]; ++i) {
		}
		for (; i < 16 && !TheMap.Tileset->Table[tile + i]; ++i) {
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
	MapField* mf;

	Assert(x >= 0 && y >= 0 && x < TheMap.Width && y < TheMap.Height);

	ChangeTile(x, y, GetTileNumber(tile, TileToolRandom, TileToolDecoration));

	//
	// Change the flags
	//
	mf = &TheMap.Fields[y * TheMap.Width + x];
	mf->Flags &= ~(MapFieldHuman | MapFieldLandAllowed | MapFieldCoastAllowed |
		MapFieldWaterAllowed | MapFieldNoBuilding | MapFieldUnpassable |
		MapFieldWall | MapFieldRocks | MapFieldForest);

	mf->Flags |= TheMap.Tileset->FlagsTable[16 + tile * 16];

	UpdateMinimapSeenXY(x, y);
	UpdateMinimapXY(x, y);

	EditorTileChanged(x, y);
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
**
*/
void EditTilesInternal(int x, int y, int tile, int size)
{
	int ex;
	int ey;
	int i;

	ex = x + size;
	if (ex > TheMap.Width) {
		ex = TheMap.Width;
	}
	ey = y + size;
	if (ey > TheMap.Height) {
		ey = TheMap.Height;
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
**
*/
void EditTiles(int x, int y, int tile, int size)
{
	int mx;
	int my;

	mx = TheMap.Width;
	my = TheMap.Height;

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

/**
**  Edit unit (internal, used by EditUnit()).
**
**  @param x       X map tile coordinate.
**  @param y       Y map tile coordinate.
**  @param type    Unit type to edit.
**  @param player  Player owning the unit.
**
**  @todo  FIXME: Check if the player has already a start-point.
**  @bug   This function does not support mirror editing!
**
*/
static void EditUnitInternal(int x, int y, UnitType* type, Player* player)
{
	Unit* unit;

	// FIXME: vladi: should check place when mirror editing is enabled...?
	unit = MakeUnitAndPlace(x, y, type, player);
	if (type->GivesResource) {
		unit->ResourcesHeld = DefaultResourceAmounts[type->GivesResource];
	}
}

/**
**  Edit unit.
**
**  @param x       X map tile coordinate.
**  @param y       Y map tile coordinate.
**  @param type    Unit type to edit.
**  @param player  Player owning the unit.
**
**  @todo  FIXME: Check if the player has already a start-point.
*/
static void EditUnit(int x, int y, UnitType* type, Player* player)
{
	int mx;
	int my;

	mx = TheMap.Width;
	my = TheMap.Height;

	EditUnitInternal(x, y, type, player);

	if (!MirrorEdit) {
		return;
	}

	EditUnitInternal(mx - x - 1, y, type, player);

	if (MirrorEdit == 1) {
		return;
	}

	EditUnitInternal(x, my - y - 1, type, player);
	EditUnitInternal(mx - x - 1, my - y - 1, type, player);
}

/**
**  Calculate the number of unit icons that can be displayed
**
**  @return  Number of unit icons that can be displayed.
*/
static int CalculateUnitIcons(void)
{
	int i;
	int x;
	int count;

	i = 0;
	count = 0;
	x = TheUI.ButtonPanelY + 24;
	while (x < TheUI.ButtonPanelY + TheUI.ButtonPanelG->Height - IconHeight) {
		++i;
		x += IconHeight + 2;
	}
	x = TheUI.ButtonPanelX + 10;
	while (x < TheUI.ButtonPanelX + TheUI.ButtonPanelG->Width - IconWidth) {
		count += i;
		x += IconWidth + 8;
	}
	return count;
}

/**
** Calculate the max height and the max widht of icons,
** and assign them to IconHeight and IconWidth
*/
static void CalculateMaxIconSize(void)
{
	int i;
	const UnitType* type;
	const Icon* icon;

	IconWidth = 0;
	IconHeight = 0;
	for (i = 0; i < MaxUnitIndex; ++i) {
		type = UnitTypeByIdent(EditorUnitTypes[i]);
		Assert(type && type->Icon.Icon);
		icon = type->Icon.Icon;
		if (IconWidth < icon->Sprite->Width) {
			IconWidth = icon->Sprite->Width;
		}
		if (IconHeight < icon->Sprite->Height) {
			IconHeight = icon->Sprite->Height;
		}
	}
}


/**
**  Recalculate the shown units.
*/
static void RecalculateShownUnits(void)
{
	int i;
	int n;
	const UnitType* type;

	if (!ShownUnitTypes) {
		ShownUnitTypes = malloc(sizeof(char*) * MaxUnitIndex);
	}

	for (n = i = 0; i < MaxUnitIndex; ++i) {
		type = UnitTypeByIdent(EditorUnitTypes[i]);

		if (type->Building && !ShowBuildingsToSelect) {
			continue;
		}
		if (!type->Building && !ShowUnitsToSelect) {
			continue;
		}
#if 0
		if (type->Hero && !ShowHeroesToSelect) {
			continue;
		}
#endif
		if (type->UnitType == UnitTypeLand && !ShowLandToSelect) {
			continue;
		}
		if (type->UnitType == UnitTypeNaval && !ShowWaterToSelect) {
			continue;
		}
		if (type->UnitType == UnitTypeFly && !ShowAirToSelect) {
			continue;
		}

		ShownUnitTypes[n++] = EditorUnitTypes[i];
	}
	MaxShownUnits = n;

	if (UnitIndex >= MaxShownUnits) {
		int count;

		count = CalculateUnitIcons();
		UnitIndex = MaxShownUnits / count * count;
	}
	// Quick & dirty make them invalid
	CursorUnitIndex = -1;
	SelectedUnitIndex = -1;
}

/*----------------------------------------------------------------------------
--  Display
----------------------------------------------------------------------------*/

#define MBUTTON_GEM_SQUARE  24

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

	x = TheUI.InfoPanelX + 46;
	y = TheUI.InfoPanelY + 4 + IconHeight + 11;

	if (CursorOn == CursorOnButton &&
			ButtonUnderCursor >= 300 && ButtonUnderCursor < 306) {
		VideoDrawRectangle(ColorGray, x - 42,
				y - 3 + (ButtonUnderCursor - 300) * 20, 100, 20);
	}

	VideoDrawTextCentered(x, y, GameFont, "1x1");
	VideoDraw(MenuButtonGfx.Sprite,
		MBUTTON_GEM_SQUARE + (TileCursorSize == 1 ? 2 : 0), x + 40, y - 3);
	y += 20;
	VideoDrawTextCentered(x, y, GameFont, "2x2");
	VideoDraw(MenuButtonGfx.Sprite,
		MBUTTON_GEM_SQUARE + (TileCursorSize == 2 ? 2 : 0), x + 40, y - 3);
	y += 20;
	VideoDrawTextCentered(x, y, GameFont, "3x3");
	VideoDraw(MenuButtonGfx.Sprite,
		MBUTTON_GEM_SQUARE + (TileCursorSize == 3 ? 2 : 0), x + 40, y - 3);
	y += 20;
	VideoDrawTextCentered(x, y, GameFont, "4x4");
	VideoDraw(MenuButtonGfx.Sprite,
		MBUTTON_GEM_SQUARE + (TileCursorSize == 4 ? 2 : 0), x + 40, y - 3);
	y += 20;
	VideoDrawTextCentered(x, y, GameFont, "Random");
	VideoDraw(MenuButtonGfx.Sprite,
		MBUTTON_GEM_SQUARE + (TileToolRandom ? 2 : 0), x + 40, y - 3);
	y += 20;
	VideoDrawTextCentered(x, y, GameFont, "Filler");
	VideoDraw(MenuButtonGfx.Sprite,
		MBUTTON_GEM_SQUARE + (TileToolDecoration ? 2 : 0), x + 40, y - 3);
	y += 20;

	y = TheUI.ButtonPanelY + 4;
	i = 0;

	while (y < TheUI.ButtonPanelY + 100) {
		x = TheUI.ButtonPanelX + 4;
		while (x < TheUI.ButtonPanelX + 144) {
			if (!TheMap.Tileset->Tiles[0x10 + i * 16].BaseTerrain) {
				y = TheUI.ButtonPanelY + 100;
				break;
			}
			VideoDrawClip(TheMap.TileGraphic, TheMap.Tileset->Table[0x10 + i * 16], x, y);
			VideoDrawRectangle(ColorGray, x, y, TileSizeX, TileSizeY);
			if (TileCursor == i) {
				VideoDrawRectangleClip(ColorGreen, x + 1, y + 1, TileSizeX-2, TileSizeY-2);

			}
			if (CursorOn == CursorOnButton && ButtonUnderCursor == i + 100) {
				VideoDrawRectangle(ColorWhite, x - 1, y - 1, TileSizeX+2, TileSizeY+2);
			}
			x += TileSizeX+2;
			++i;
		}
		y += TileSizeY+2;
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
	int j;
	int percent;
	char buf[256];
	Icon *icon;

	x = TheUI.InfoPanelX + 8;
	y = TheUI.InfoPanelY + 4 + IconHeight + 10;

	for (i = 0; i < PlayerMax; ++i) {
		if (i == PlayerMax / 2) {
			y += 20;
		}
		if (i == CursorPlayer && TheMap.Info->PlayerType[i] != PlayerNobody) {
			VideoDrawRectangle(ColorWhite, x + i % 8 * 20, y, 20, 20);
		}
		VideoDrawRectangle(
			i == CursorPlayer && TheMap.Info->PlayerType[i] != PlayerNobody ?
				ColorWhite : ColorGray,
			x + i % 8 * 20, y, 19, 19);
		if (TheMap.Info->PlayerType[i] != PlayerNobody) {
			VideoFillRectangle(Players[i].Color, x + 1 + i % 8 * 20, y + 1,
				17, 17);
		}
		if (i == SelectedPlayer) {
			VideoDrawRectangle(ColorGreen, x + 1 + i % 8 * 20, y + 1, 17, 17);
		}
		sprintf(buf, "%d", i);
		VideoDrawTextCentered(x + i % 8 * 20 + 10, y + 7, SmallFont, buf);
	}

	x = TheUI.InfoPanelX + 4;
	y += 18 * 1 + 4;
	if (SelectedPlayer != -1) {
		i = sprintf(buf,"Plyr %d %s ", SelectedPlayer,
				PlayerRaces.Name[TheMap. Info->PlayerSide[SelectedPlayer]]);
		// Players[SelectedPlayer].RaceName);

		switch (TheMap.Info->PlayerType[SelectedPlayer]) {
			case PlayerNeutral:
				strcat(buf, "Neutral");
				break;
			case PlayerNobody:
			default:
				strcat(buf, "Nobody");
				break;
			case PlayerPerson:
				strcat(buf, "Person");
				break;
			case PlayerComputer:
			case PlayerRescuePassive:
			case PlayerRescueActive:
				strcat(buf, "Computer");
				break;
		}

		VideoDrawText(x, y, GameFont, buf);
	}

	//
	// Draw the unit selection buttons.
	//
	x = TheUI.InfoPanelX + 10;
	y = TheUI.InfoPanelY + 140;

	VideoDrawText(x + 28 * 0, y, GameFont, "Un");
	VideoDraw(MenuButtonGfx.Sprite,
		MBUTTON_GEM_SQUARE + (ShowUnitsToSelect ? 2 : 0), x + 28 * 0, y + 16);
	VideoDrawText(x + 28 * 1, y, GameFont, "Bu");
	VideoDraw(MenuButtonGfx.Sprite,
		MBUTTON_GEM_SQUARE + (ShowBuildingsToSelect ? 2 : 0), x + 28 * 1,
		y + 16);
#if 0
	VideoDrawText(x + 28 * 2, y, GameFont, "He");
	VideoDraw(MenuButtonGfx.Sprite,
		MBUTTON_GEM_SQUARE + (ShowHeroesToSelect ? 2 : 0), x + 28 * 2, y + 16);
#endif
	VideoDrawText(x + 28 * 3, y, GameFont, "La");
	VideoDraw(MenuButtonGfx.Sprite,
		MBUTTON_GEM_SQUARE + (ShowLandToSelect ? 2 : 0), x + 28 * 3, y + 16);
	VideoDrawText(x + 28 * 4, y, GameFont, "Wa");
	VideoDraw(MenuButtonGfx.Sprite,
		MBUTTON_GEM_SQUARE + (ShowWaterToSelect ? 2 : 0), x + 28 * 4, y + 16);
	VideoDrawText(x + 28 * 5, y, GameFont, "Ai");
	VideoDraw(MenuButtonGfx.Sprite,
		MBUTTON_GEM_SQUARE + (ShowAirToSelect ? 2 : 0), x + 28 * 5, y + 16);

	//
	// Scroll bar for units. FIXME: drag not supported.
	//
	x = TheUI.ButtonPanelX + 4;
	y = TheUI.ButtonPanelY + 4;
	j = 176 - 8;

	PushClipping();
	SetClipping(0, 0, x + j - 20, VideoHeight - 1);
	VideoDrawClip(MenuButtonGfx.Sprite, MBUTTON_S_HCONT, x - 2, y);
	PopClipping();
	if (TheUI.ButtonPanelX + 4 < CursorX
			&& CursorX < TheUI.ButtonPanelX + 24
			&& TheUI.ButtonPanelY + 4 < CursorY
			&& CursorY < TheUI.ButtonPanelY + 24
			&& MouseButtons & LeftButton) {
		VideoDraw(MenuButtonGfx.Sprite, MBUTTON_LEFT_ARROW + 1, x - 2, y);
	} else {
		VideoDraw(MenuButtonGfx.Sprite, MBUTTON_LEFT_ARROW, x - 2, y);
	}
	if (TheUI.ButtonPanelX + 176 - 24 < CursorX
			&& CursorX < TheUI.ButtonPanelX + 176 - 4
			&& TheUI.ButtonPanelY + 4 < CursorY
			&& CursorY < TheUI.ButtonPanelY + 24
			&& MouseButtons & LeftButton) {
		VideoDraw(MenuButtonGfx.Sprite, MBUTTON_RIGHT_ARROW + 1, x + j - 20, y);
	} else {
		VideoDraw(MenuButtonGfx.Sprite, MBUTTON_RIGHT_ARROW, x + j - 20, y);
	}

	percent = UnitIndex * 100 / (MaxShownUnits ? MaxShownUnits : 1);
	i = (percent * (j - 54)) / 100;
	VideoDraw(MenuButtonGfx.Sprite, MBUTTON_S_KNOB, x + 18 + i, y + 1);

	//
	//  Draw the unit icons.
	//
	y = TheUI.ButtonPanelY + 24;
	i = UnitIndex;
	while (y < TheUI.ButtonPanelY + TheUI.ButtonPanelG->Height
			- IconHeight) {
		if (i >= MaxShownUnits) {
			break;
		}
		x = TheUI.ButtonPanelX + 10;
		while (x < TheUI.ButtonPanelX + TheUI.ButtonPanelG->Width
				- IconWidth) {
			if (i >= MaxShownUnits) {
				break;
			}
			icon = UnitTypeByIdent(ShownUnitTypes[i])->Icon.Icon;
			DrawIcon(Players + SelectedPlayer, icon, x, y);

			VideoDrawRectangleClip(ColorGray, x, y, icon->Sprite->Width, icon->Sprite->Height);
			if (i == SelectedUnitIndex) {
				VideoDrawRectangleClip(ColorGreen, x + 1, y + 1,
					icon->Sprite->Width - 2, icon->Sprite->Height - 2);
			}
			if (i == CursorUnitIndex) {
				VideoDrawRectangleClip(ColorWhite,x - 1, y - 1,
					icon->Sprite->Width + 2, icon->Sprite->Height + 2);
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
static void DrawTileIcon(unsigned tilenum,unsigned x,unsigned y,unsigned flags)
{
	Uint32 color;

	color = (flags & IconActive) ? ColorGray : ColorBlack;

	VideoDrawRectangleClip(color, x, y, TileSizeX + 7, TileSizeY + 7);
	VideoDrawRectangleClip(ColorBlack, x + 1, y + 1, TileSizeX + 5, TileSizeY + 5);

	VideoDrawVLine(ColorGray, x + TileSizeX + 4, y + 5, TileSizeY - 1); // _|
	VideoDrawVLine(ColorGray, x + TileSizeX + 5, y + 5, TileSizeY - 1);
	VideoDrawHLine(ColorGray, x + 5, y + TileSizeY + 4, TileSizeX + 1);
	VideoDrawHLine(ColorGray, x + 5, y + TileSizeY + 5, TileSizeX + 1);

	color = (flags & IconClicked) ? ColorGray : ColorWhite;
	VideoDrawHLine(color, x + 5, y + 3, TileSizeX + 1);
	VideoDrawHLine(color, x + 5, y + 4, TileSizeX + 1);
	VideoDrawVLine(color, x + 3, y + 3, TileSizeY + 3);
	VideoDrawVLine(color, x + 4, y + 3, TileSizeY + 3);

	if (flags & IconClicked) {
		++x;
		++y;
	}

	x += 4;
	y += 4;
	VideoDrawClip(TheMap.TileGraphic, TheMap.Tileset->Table[tilenum], x, y);

	if (flags & IconSelected) {
		VideoDrawRectangleClip(ColorGreen, x, y, TileSizeX, TileSizeY);
	}
}

/**
**  Draw the editor panels.
*/
static void DrawEditorPanel(void)
{
	int x;
	int y;
	Icon* icon;

	x = TheUI.InfoPanelX + 4;
	y = TheUI.InfoPanelY + 4;

	//
	// Select / Units / Tiles
	//
	icon = IconByIdent(EditorSelectIcon);
	Assert(icon);
	// FIXME: wrong button style
	DrawUnitIcon(Players, TheUI.SingleSelectedButton->Style, icon,
		(ButtonUnderCursor == SelectButton ? IconActive : 0) |
			(EditorState == EditorSelecting ? IconSelected : 0),
		x, y, NULL);
	icon = IconByIdent(EditorUnitsIcon);
	Assert(icon);
	// FIXME: wrong button style
	DrawUnitIcon(Players, TheUI.SingleSelectedButton->Style, icon,
		(ButtonUnderCursor == UnitButton ? IconActive : 0) |
			(EditorState == EditorEditUnit ? IconSelected : 0),
		x + UNIT_ICON_X, y + UNIT_ICON_Y, NULL);

	DrawTileIcon(0x10 + 4 * 16, x + TILE_ICON_X, y + TILE_ICON_Y,
		(ButtonUnderCursor == TileButton ? IconActive : 0) |
			(EditorState == EditorEditTile ? IconSelected : 0));

	switch (EditorState) {
		case EditorSelecting:
			break;
		case EditorEditTile:
			DrawTileIcons();
			break;
		case EditorEditUnit:
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
	// Draw map cursor
	//
	if (TheUI.MouseViewport && !CursorBuilding) {
		x = Viewport2MapX(TheUI.MouseViewport, CursorX);
		y = Viewport2MapY(TheUI.MouseViewport, CursorY);
		x = Map2ViewportX(TheUI.MouseViewport, x);
		y = Map2ViewportY(TheUI.MouseViewport, y);
		if (EditorState == EditorEditTile) {
			int i;
			int j;

			SetClipping(TheUI.MouseViewport->X, TheUI.MouseViewport->Y,
				TheUI.MouseViewport->EndX, TheUI.MouseViewport->EndY);
			for (j = 0; j < TileCursorSize; ++j) {
				int ty;

				ty = y + j * TileSizeY;
				if (ty >= TheUI.MouseViewport->EndY) {
					break;
				}
				for (i = 0; i < TileCursorSize; ++i) {
					int tx;

					tx = x + i * TileSizeX;
					if (tx >= TheUI.MouseViewport->EndX) {
						break;
					}
					VideoDrawClip(TheMap.TileGraphic, 
						TheMap.Tileset->Table[0x10 + TileCursor * 16], tx, ty);
				}
			}
			VideoDrawRectangleClip(ColorWhite, x, y, TileSizeX * TileCursorSize,
				TileSizeY * TileCursorSize);
			SetClipping(0, 0, VideoWidth - 1, VideoHeight - 1);
		} else {
			//
			// If there is an unit under the cursor, it's selection thing
			//  is drawn somewhere else (Check DrawUnitSelection.)
			//
			if (!UnitUnderCursor) {
				VideoDrawRectangle(ColorWhite, x, y, TileSizeX, TileSizeY);
			}
		}
	}
}

/**
**  Draw editor info.
**
**  If cursor is on map or minimap show information about the current tile.
**
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
	if (TheUI.MouseViewport) {
		x = Viewport2MapX(TheUI.MouseViewport, CursorX);
		y = Viewport2MapY(TheUI.MouseViewport, CursorY);
	}

	sprintf(buf, "Editor (%d %d)", x, y);
	VideoDrawText(TheUI.ResourceX + 2, TheUI.ResourceY + 2, GameFont, buf);

	//
	// Flags info
	//
	flags = TheMap.Fields[x + y * TheMap.Width].Flags;
	sprintf(buf, "%02X|%04X|%c%c%c%c%c%c%c%c%c%c%c%c%c",
		TheMap.Fields[x + y * TheMap.Width].Value, flags,
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
	VideoDrawText(TheUI.ResourceX + 118, TheUI.ResourceY + 2, GameFont, buf);

	//
	// Tile info
	//
	tile = TheMap.Fields[x + y * TheMap.Width].Tile;

	for (i = 0; i < TheMap.Tileset->NumTiles; ++i) {
		if (tile == TheMap.Tileset->Table[i]) {
			break;
		}
	}

	Assert(i != TheMap.Tileset->NumTiles);

	sprintf(buf, "%d %s %s", tile,
		TheMap.Tileset->SolidTerrainTypes[TheMap.Tileset->Tiles[i].BaseTerrain].TerrainName,
		TheMap.Tileset->Tiles[i].MixTerrain
			? TheMap.Tileset->SolidTerrainTypes[TheMap.Tileset->Tiles[i].MixTerrain].TerrainName
			: "");

	VideoDrawText(TheUI.ResourceX + 252, TheUI.ResourceY + 2, GameFont, buf);
#endif
}

/**
**  Show info about unit.
**
**  @param unit  Unit pointer.
*/
static void ShowUnitInfo(const Unit* unit)
{
	char buf[256];
	int i;

	i = sprintf(buf, "#%d '%s' Player:#%d %s", UnitNumber(unit),
		unit->Type->Name, unit->Player->Player,
		unit->Active ? "active" : "passive");
	if (unit->Type->GivesResource) {
		sprintf(buf + i," Amount %d", unit->ResourcesHeld);
	}
	SetStatusLine(buf);
}

/**
**  Update editor display.
*/
void EditorUpdateDisplay(void)
{
	int i;

	DrawMapArea(); // draw the map area

	//
	// Fillers
	//
	for (i = 0; i < TheUI.NumFillers; ++i) {
		VideoDrawSub(TheUI.Filler[i], 0, 0,
			TheUI.Filler[i]->Width,
			TheUI.Filler[i]->Height,
			TheUI.FillerX[i], TheUI.FillerY[i]);
	}

	if (CursorOn == CursorOnMap) {
		DrawMapCursor(); // cursor on map
	}

	//
	// Menu button
	//
	DrawMenuButton(TheUI.MenuButton.Style,
		(ButtonAreaUnderCursor == ButtonAreaMenu
			&& ButtonUnderCursor == ButtonUnderMenu ? MenuButtonActive : 0) |
		(GameMenuButtonClicked ? MenuButtonClicked : 0),
		TheUI.MenuButton.X,TheUI.MenuButton.Y,
		TheUI.MenuButton.Text);

	//
	// Minimap
	//
	if (TheUI.SelectedViewport) {
		DrawMinimap(TheUI.SelectedViewport->MapX, TheUI.SelectedViewport->MapY);
		DrawMinimapCursor(TheUI.SelectedViewport->MapX,
			TheUI.SelectedViewport->MapY);
	}
	//
	// Info panel
	//
	if (TheUI.InfoPanelG) {
		VideoDrawSub(TheUI.InfoPanelG, 0, 0,
			TheUI.InfoPanelG->Width, TheUI.InfoPanelG->Height,
			TheUI.InfoPanelX, TheUI.InfoPanelY);
	}
	//
	// Button panel
	//
	if (TheUI.ButtonPanelG) {
		VideoDrawSub(TheUI.ButtonPanelG, 0, 0,
			TheUI.ButtonPanelG->Width,
			TheUI.ButtonPanelG->Height, TheUI.ButtonPanelX,
			TheUI.ButtonPanelY);
	}
	DrawEditorPanel();

	if (CursorOn == CursorOnMap) {
		DrawEditorInfo();
	}

	//
	// Status line
	//
	DrawStatusLine();

	DrawAnyCursor();

	// FIXME: For now update everything each frame

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
	if (GameCursor == TheUI.Scroll.Cursor) {
		// Move map.
		GameCursor = TheUI.Point.Cursor; // Reset
		return;
	}

	if ((1 << button) == LeftButton && GameMenuButtonClicked == 1) {
		GameMenuButtonClicked = 0;
		if (ButtonUnderCursor == ButtonUnderMenu) {
			ProcessMenu("menu-editor", 1);
		}
	}
	if ((1 << button) == LeftButton) {
		UnitPlacedThisPress = 0;
	}
}

/**
**  Called if mouse button pressed down.
**
**  @param button  Mouse button number (0 left, 1 middle, 2 right)
*/
static void EditorCallbackButtonDown(unsigned button __attribute__ ((unused)))
{
	//
	// Click on menu button
	//
	if (CursorOn == CursorOnButton && ButtonAreaUnderCursor == ButtonAreaMenu &&
			(MouseButtons & LeftButton) && !GameMenuButtonClicked) {
		PlayGameSound(GameSounds.Click.Sound, MaxSampleVolume);
		GameMenuButtonClicked = 1;
		return;
	}
	//
	// Click on minimap
	//
	if (CursorOn == CursorOnMinimap) {
		if (MouseButtons & LeftButton) { // enter move mini-mode
			ViewportSetViewpoint(TheUI.SelectedViewport,
				ScreenMinimap2MapX(CursorX) -
					TheUI.SelectedViewport->MapWidth / 2,
				ScreenMinimap2MapY(CursorY) -
					TheUI.SelectedViewport->MapHeight / 2, TileSizeX / 2, TileSizeY / 2);
		}
		return;
	}
	//
	// Click on mode area
	//
	if (CursorOn == CursorOnButton) {
		if (ButtonUnderCursor == SelectButton) {
			CursorBuilding = NULL;
			EditorState = EditorSelecting;
			return;
		}
		if (ButtonUnderCursor == UnitButton) {
			EditorState = EditorEditUnit;
			return;
		}
		if (ButtonUnderCursor == TileButton) {
			CursorBuilding = NULL;
			EditorState = EditorEditTile;
			return;
		}
	}
	//
	// Click on tile area
	//
	if (CursorOn == CursorOnButton && ButtonUnderCursor >= 100 &&
			EditorState == EditorEditTile) {
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
		if (TheMap.Tileset->Tiles[16 + (ButtonUnderCursor - 100) * 16].BaseTerrain) {
			TileCursor = ButtonUnderCursor - 100;
		}
		return;
	}
	//
	// Click on unit area
	//
	if (EditorState == EditorEditUnit) {
		int percent;
		int j;
		int count;

		percent = UnitIndex * 100 / (MaxShownUnits ? MaxShownUnits : 1);
		j = (percent * (176 - 8 - 54)) / 100;
		count = CalculateUnitIcons();

		// Unit icons scroll left area
		if (TheUI.ButtonPanelX + 4 < CursorX &&
				CursorX < TheUI.ButtonPanelX + 4 + 18 + j &&
				TheUI.ButtonPanelY + 4 < CursorY &&
				CursorY < TheUI.ButtonPanelY + 24) {
			if (UnitIndex - count >= 0) {
				UnitIndex -= count;
			} else {
				UnitIndex = 0;
			}
			return;
		}
		// Unit icons scroll right area
		if (TheUI.ButtonPanelX + 4 + 18 + j + 18 < CursorX &&
				CursorX < TheUI.ButtonPanelX + 176 - 4 &&
				TheUI.ButtonPanelY + 4 < CursorY &&
				CursorY < TheUI.ButtonPanelY + 24) {
			if (UnitIndex + count <= MaxShownUnits) {
				UnitIndex += count;
			}
			return;
		}
		// Cursor on unit icons
		if (CursorUnitIndex != -1) {
			SelectedUnitIndex = CursorUnitIndex;
			CursorBuilding = UnitTypeByIdent(ShownUnitTypes[CursorUnitIndex]);
			return;
		}
		// Cursor on player icons
		if (CursorPlayer != -1) {
			if (TheMap.Info->PlayerType[CursorPlayer] != PlayerNobody) {
				SelectedPlayer = CursorPlayer;
				ThisPlayer = Players + SelectedPlayer;
			}
			return;
		}
		// Cursor on unit selection icons
		if (TheUI.InfoPanelX + 10 + 28 * 0 < CursorX &&
				CursorX < TheUI.InfoPanelX + 10 + 28 * 1 &&
				TheUI.InfoPanelY + 140 < CursorY &&
				CursorY < TheUI.InfoPanelY + 140 + 28) {
			ShowUnitsToSelect ^= 1;
			RecalculateShownUnits();
			return;
		}
		if (TheUI.InfoPanelX + 10 + 28 * 1 < CursorX &&
				CursorX < TheUI.InfoPanelX + 10 + 28 * 2 &&
				TheUI.InfoPanelY + 140 < CursorY &&
				CursorY < TheUI.InfoPanelY + 140 + 28) {
			ShowBuildingsToSelect ^= 1;
			RecalculateShownUnits();
			return;
		}
#if 0
		if (TheUI.InfoPanelX + 10 + 28 * 2 < CursorX &&
				CursorX < TheUI.InfoPanelX + 10 + 28 * 3 &&
				TheUI.InfoPanelY + 140 < CursorY &&
				CursorY < TheUI.InfoPanelY + 140 + 28) {
			ShowHeroesToSelect ^= 1;
			RecalculateShownUnits();
			return;
		}
#endif
		if (TheUI.InfoPanelX + 10 + 28 * 3 < CursorX &&
				CursorX < TheUI.InfoPanelX + 10 + 28 * 4 &&
				TheUI.InfoPanelY + 140 < CursorY &&
				CursorY < TheUI.InfoPanelY + 140 + 28) {
			ShowLandToSelect ^= 1;
			RecalculateShownUnits();
			return;
		}
		if (TheUI.InfoPanelX + 10 + 28 * 4 < CursorX &&
				CursorX < TheUI.InfoPanelX + 10 + 28 * 5 &&
				TheUI.InfoPanelY + 140 < CursorY &&
				CursorY < TheUI.InfoPanelY + 140 + 28) {
			ShowWaterToSelect ^= 1;
			RecalculateShownUnits();
			return;
		}
		if (TheUI.InfoPanelX + 10 + 28 * 5 < CursorX &&
				CursorX < TheUI.InfoPanelX + 10 + 28 * 6 &&
				TheUI.InfoPanelY + 140 < CursorY &&
				CursorY < TheUI.InfoPanelY + 140 + 28) {
			ShowAirToSelect ^= 1;
			RecalculateShownUnits();
			return;
		}
	}

	//
	// Right click on a resource
	//
	if (EditorState == EditorSelecting) {
		if ((MouseButtons & RightButton && UnitUnderCursor)) {
			if (UnitUnderCursor->Type->GivesResource) {
				EditorEditResource();
				return;
			}
			if (!UnitUnderCursor->Type->Building && UnitUnderCursor->HP > 0) {
				EditorEditAiProperties();
			}
		}
	}

	//
	// Click on map area
	//
	if (CursorOn == CursorOnMap) {
		Viewport* vp;

		vp = GetViewport(CursorX, CursorY);
		Assert(vp);
		if ((MouseButtons & LeftButton) && TheUI.SelectedViewport != vp) {
			// viewport changed
			TheUI.SelectedViewport = vp;
		}

		if (MouseButtons & LeftButton) {
			if (EditorState == EditorEditTile) {
				EditTiles(Viewport2MapX(TheUI.MouseViewport, CursorX),
					Viewport2MapY(TheUI.MouseViewport, CursorY), TileCursor,
					TileCursorSize);
			}
			if (!UnitPlacedThisPress) {
				if (EditorState == EditorEditUnit && CursorBuilding) {
					if (CanBuildUnitType(NULL, CursorBuilding,
							Viewport2MapX(TheUI.MouseViewport, CursorX),
							Viewport2MapY(TheUI.MouseViewport, CursorY), 1)) {
						PlayGameSound(GameSounds.PlacementSuccess.Sound,
							MaxSampleVolume);
						EditUnit(Viewport2MapX(TheUI.MouseViewport,CursorX),
							Viewport2MapY(TheUI.MouseViewport, CursorY),
							CursorBuilding, Players + SelectedPlayer);
						UnitPlacedThisPress = 1;
						ClearStatusLine();
					} else {
						SetStatusLine("Unit can't be placed here.");
						PlayGameSound(GameSounds.PlacementError.Sound,
							MaxSampleVolume);
					}
				}
			}
		} else if (MouseButtons & MiddleButton) {
			// enter move map mode
			CursorStartX = CursorX;
			CursorStartY = CursorY;
			GameCursor = TheUI.Scroll.Cursor;
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

	switch (key) {
		case 'f' & 0x1F:
		case 'f':
		case 'F': // ALT+F, CTRL+F toggle fullscreen
			if (!(KeyModifiers & (ModifierAlt | ModifierControl))) {
				break;
			}
			ToggleFullScreen();
			break;

		case 's': // ALT s F11 save pud menu
		case 'S':
		case KeyCodeF11:
			if (EditorSaveMenu() != -1) {
				SetStatusLine("Pud saved");
			}
			InterfaceState = IfaceStateNormal;
			break;

		case KeyCodeF12:
			EditorLoadMenu();
			InterfaceState = IfaceStateNormal;
			break;

		case 'v': // 'v' Viewport
			if (KeyModifiers & ModifierControl) {
				CycleViewportMode(-1);
			} else {
				CycleViewportMode(1);
			}
			break;

		case 'r':
		case 'R': // CTRL+R Randomize map
			if (KeyModifiers & ModifierControl) {
				EditorCreateRandomMap();
			}
			break;

		case 'm':
		case 'M': // CTRL+M Mirror edit
			if (KeyModifiers & ModifierControl)  {
				++MirrorEdit;
				if (MirrorEdit == 3) {
					MirrorEdit = 0;
				}
				switch (MirrorEdit) {
					case 1:
						SetStatusLine("Mirror editing enabled: 2-side");
						break;
					case 2:
						SetStatusLine("Mirror editing enabled: 4-side");
						break;
					default:
						SetStatusLine("Mirror editing disabled");
						break;
				  }
			}
			break;

		case 'x' & 0x1F:
		case 'x':
		case 'X': // ALT+X, CTRL+X: Exit editor
			if (!(KeyModifiers & (ModifierAlt | ModifierControl))) {
				break;
			}
			Exit(0);

		case KeyCodeDelete: // Delete
			if (UnitUnderCursor) {
				Unit* unit;

				RemoveUnit(unit = UnitUnderCursor, NULL);
				UnitLost(unit);
				UnitClearOrders(unit);
				ReleaseUnit(unit);
				SetStatusLine("Unit deleted");
			}
			break;

		case KeyCodeF10:
			ProcessMenu("menu-editor", 1);
			break;

		case KeyCodeUp: // Keyboard scrolling
		case KeyCodeKP8:
			KeyScrollState |= ScrollUp;
			break;
		case KeyCodeDown:
		case KeyCodeKP2:
			KeyScrollState |= ScrollDown;
			break;
		case KeyCodeLeft:
		case KeyCodeKP4:
			KeyScrollState |= ScrollLeft;
			break;
		case KeyCodeRight:
		case KeyCodeKP6:
			KeyScrollState |= ScrollRight;
			break;

		default:
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
		case KeyCodeUp: // Keyboard scrolling
		case KeyCodeKP8:
			KeyScrollState &= ~ScrollUp;
			break;
		case KeyCodeDown:
		case KeyCodeKP2:
			KeyScrollState &= ~ScrollDown;
			break;
		case KeyCodeLeft:
		case KeyCodeKP4:
			KeyScrollState &= ~ScrollLeft;
			break;
		case KeyCodeRight:
		case KeyCodeKP6:
			KeyScrollState &= ~ScrollRight;
			break;
		default:
			break;
	}
}

/**
**  Callback for input.
*/
static void EditorCallbackKey3(unsigned dummy1 __attribute__((unused)),
	unsigned dummy2 __attribute__((unused)))
{
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
	if (GameCursor == TheUI.Scroll.Cursor) {
		int xo;
		int yo;

		// FIXME: Support with CTRL for faster scrolling.
		// FIXME: code duplication, see ../ui/mouse.c
		xo = TheUI.MouseViewport->MapX;
		yo = TheUI.MouseViewport->MapY;
		if (TheUI.MouseScrollSpeedDefault < 0) {
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
		TheUI.MouseWarpX = CursorStartX;
		TheUI.MouseWarpY = CursorStartY;
		ViewportSetViewpoint(TheUI.MouseViewport, xo, yo, TileSizeX / 2, TileSizeY / 2);
		return;
	}

	// Automatically unpress when map tile has changed
	if (LastMapX != Viewport2MapX(TheUI.SelectedViewport, CursorX) ||
		LastMapY != Viewport2MapY(TheUI.SelectedViewport, CursorY)) {
		LastMapX = Viewport2MapX(TheUI.SelectedViewport, CursorX);
		LastMapY = Viewport2MapY(TheUI.SelectedViewport, CursorY);
		UnitPlacedThisPress = 0;
	}
	//
	// Drawing tiles on map.
	//
	if (CursorOn == CursorOnMap && (MouseButtons & LeftButton) &&
			(EditorState == EditorEditTile || EditorState == EditorEditUnit)) {

		//
		// Scroll the map
		//
		if (!(FrameCounter % SpeedMouseScroll)) {
			if (CursorX <= TheUI.SelectedViewport->X) {
				ViewportSetViewpoint(TheUI.SelectedViewport,
					TheUI.SelectedViewport->MapX - 1,
					TheUI.SelectedViewport->MapY, TileSizeX / 2, TileSizeY / 2);
			} else if (CursorX >= TheUI.SelectedViewport->EndX) {
				ViewportSetViewpoint(TheUI.SelectedViewport,
					TheUI.SelectedViewport->MapX + 1,
					TheUI.SelectedViewport->MapY, TileSizeX / 2, TileSizeY / 2);
			}

			if (CursorY <= TheUI.SelectedViewport->Y) {
				ViewportSetViewpoint(TheUI.SelectedViewport,
					TheUI.SelectedViewport->MapX,
					TheUI.SelectedViewport->MapY - 1, TileSizeX / 2, TileSizeY / 2);
			} else if (CursorY >= TheUI.SelectedViewport->EndY) {
				ViewportSetViewpoint(TheUI.SelectedViewport,
					TheUI.SelectedViewport->MapX,
					TheUI.SelectedViewport->MapY + 1, TileSizeX / 2, TileSizeY / 2);

			}
		}

		//
		// Scroll the map, if cursor moves outside the viewport.
		//
		RestrictCursorToViewport();

		if (EditorState == EditorEditTile) {
			EditTiles(Viewport2MapX(TheUI.SelectedViewport, CursorX),
				Viewport2MapY(TheUI.SelectedViewport, CursorY), TileCursor,
				TileCursorSize);
		} else if (EditorState == EditorEditUnit && CursorBuilding) {
			if (!UnitPlacedThisPress) {
				if (CanBuildUnitType(NULL, CursorBuilding,
					Viewport2MapX(TheUI.SelectedViewport, CursorX),
					Viewport2MapY(TheUI.SelectedViewport, CursorY), 1)) {
					EditUnit(Viewport2MapX(TheUI.SelectedViewport, CursorX),
						Viewport2MapY(TheUI.SelectedViewport, CursorY),
						CursorBuilding, Players + SelectedPlayer);
					UnitPlacedThisPress = 1;
					ClearStatusLine();
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
		ViewportSetViewpoint(TheUI.SelectedViewport,
			ScreenMinimap2MapX(CursorX)
				- TheUI.SelectedViewport->MapWidth / 2,
			ScreenMinimap2MapY(CursorY)
				- TheUI.SelectedViewport->MapHeight / 2, 0, 0);
		return;
	}

	OldCursorOn = CursorOn;

	MouseScrollState = ScrollNone;
	GameCursor = TheUI.Point.Cursor;
	CursorOn = -1;
	CursorPlayer = -1;
	CursorUnitIndex = -1;
	ButtonUnderCursor = -1;

	//
	// Minimap
	//
	if (x >= TheUI.MinimapPosX && x < TheUI.MinimapPosX + TheUI.MinimapW &&
			y >= TheUI.MinimapPosY && y < TheUI.MinimapPosY + TheUI.MinimapH) {
		CursorOn = CursorOnMinimap;
	}

	//
	// Handle edit unit area
	//
	if (EditorState == EditorEditUnit) {
		// Scrollbar
		if (TheUI.ButtonPanelX + 4 < CursorX
				&& CursorX < TheUI.ButtonPanelX + 176 - 4
				&& TheUI.ButtonPanelY + 4 < CursorY
				&& CursorY < TheUI.ButtonPanelY + 24) {
			return;
		}
		bx = TheUI.InfoPanelX + 8;
		by = TheUI.InfoPanelY + 4 + IconHeight + 10;
		for (i = 0; i < PlayerMax; ++i) {
			if (i == PlayerMax / 2) {
				bx = TheUI.InfoPanelX + 8;
				by += 20;
			}
			if (bx < x && x < bx + 20 && by < y && y < by + 20) {
				if (TheMap.Info->PlayerType[i] != PlayerNobody) {
					sprintf(buf,"Select player #%d",i);
					SetStatusLine(buf);
				} else {
					ClearStatusLine();
				}
				CursorPlayer = i;
#if 0
				ButtonUnderCursor = i + 100;
				CursorOn = CursorOnButton;
#endif
				return;
			}
			bx += 20;
		}

		i = UnitIndex;
		by = TheUI.ButtonPanelY + 24;
		while (by < TheUI.ButtonPanelY +
				TheUI.ButtonPanelG->Height - IconHeight) {
			if (i >= MaxShownUnits || !ShownUnitTypes[i]) {
				break;
			}
			bx = TheUI.ButtonPanelX + 10;
			while (bx < TheUI.ButtonPanelX + 146) {
				if (i >= MaxShownUnits || !ShownUnitTypes[i]) {
					break;
				}
				if (bx < x && x < bx + IconWidth &&
						by < y && y < by + IconHeight) {
					sprintf(buf,"%s \"%s\"",
						UnitTypeByIdent(ShownUnitTypes[i])->Ident,
						UnitTypeByIdent(ShownUnitTypes[i])->Name);
					SetStatusLine(buf);
					CursorUnitIndex = i;
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
	if (EditorState == EditorEditTile) {
		i = 0;
		bx = TheUI.InfoPanelX + 4;
		by = TheUI.InfoPanelY + 4 + IconHeight + 10;

		while (i < 6) {
			if (bx < x && x < bx + 100 && by < y && y < by + 18) {
				ButtonUnderCursor = i + 300;
				CursorOn = CursorOnButton;
				return;
			}
			++i;
			by += 20;
		}

		i = 0;
		by = TheUI.ButtonPanelY + 4;
		while (by < TheUI.ButtonPanelY + 100) {
			bx = TheUI.ButtonPanelX + 4;
			while (bx < TheUI.ButtonPanelX + 144) {
				if (!TheMap.Tileset->Tiles[0x10 + i * 16].BaseTerrain) {
					by = TheUI.ButtonPanelY + 100;
					break;
				}
				if (bx < x && x < bx + TileSizeX &&
						by < y && y < by + TileSizeY) {
					int j;

					// FIXME: i is wrong, must find the solid type
					j = TheMap.Tileset->Tiles[i * 16 + 16].BaseTerrain;
					SetStatusLine(TheMap.Tileset->SolidTerrainTypes[j].TerrainName);
					ButtonUnderCursor = i + 100;
					CursorOn = CursorOnButton;
					return;
				}
				bx += TileSizeX+2;
				++i;
			}
			by += TileSizeY+2;
		}
	}

	//
	// Handle buttons
	//
	if (TheUI.InfoPanelX + 4 < CursorX &&
			CursorX < TheUI.InfoPanelX + 4 + IconWidth + 7 &&
			TheUI.InfoPanelY + 4 < CursorY &&
			CursorY < TheUI.InfoPanelY + 4 + IconHeight + 7) {
		// FIXME: what is this button?
		ButtonAreaUnderCursor = -1;
		ButtonUnderCursor = SelectButton;
		CursorOn = CursorOnButton;
		SetStatusLine("Select mode");
		return;
	}
	if (TheUI.InfoPanelX + 4 + UNIT_ICON_X < CursorX &&
			CursorX < TheUI.InfoPanelX + 4 + UNIT_ICON_X + IconWidth + 7 &&
			TheUI.InfoPanelY + 4 + UNIT_ICON_Y < CursorY &&
			CursorY < TheUI.InfoPanelY + 4 + UNIT_ICON_Y + IconHeight + 7) {
		ButtonAreaUnderCursor = -1;
		ButtonUnderCursor = UnitButton;
		CursorOn = CursorOnButton;
		SetStatusLine("Unit mode");
		return;
	}
	if (TheUI.InfoPanelX + 4 + TILE_ICON_X < CursorX &&
			CursorX < TheUI.InfoPanelX + 4 + TILE_ICON_X + TileSizeX + 7 &&
			TheUI.InfoPanelY + 4 + TILE_ICON_Y < CursorY &&
			CursorY < TheUI.InfoPanelY + 4 + TILE_ICON_Y + TileSizeY + 7) {
		ButtonAreaUnderCursor = -1;
		ButtonUnderCursor = TileButton;
		CursorOn = CursorOnButton;
		SetStatusLine("Tile mode");
		return;
	}
	if (TheUI.MenuButton.X != -1) {
		if (x >= TheUI.MenuButton.X &&
				x <= TheUI.MenuButton.X + TheUI.MenuButton.Style->Width &&
				y > TheUI.MenuButton.Y &&
				y <= TheUI.MenuButton.Y + TheUI.MenuButton.Style->Height) {
			ButtonAreaUnderCursor = ButtonAreaMenu;
			ButtonUnderCursor = ButtonUnderMenu;
			CursorOn = CursorOnButton;
			return;
		}
	}

	//
	// Minimap
	//
	if (x >= TheUI.MinimapPosX && x < TheUI.MinimapPosX + TheUI.MinimapW &&
			y >= TheUI.MinimapPosY && y < TheUI.MinimapPosY + TheUI.MinimapH) {
		CursorOn = CursorOnMinimap;
		return;
	}

	//
	// Map
	//
	UnitUnderCursor = NULL;
	if (x >= TheUI.MapArea.X && x <= TheUI.MapArea.EndX &&
			y >= TheUI.MapArea.Y && y <= TheUI.MapArea.EndY) {
		Viewport* vp;

		vp = GetViewport(x, y);
		Assert(vp);
		if (TheUI.MouseViewport != vp) { // viewport changed
			TheUI.MouseViewport = vp;
			DebugPrint("active viewport changed to %d.\n" _C_
				TheUI.Viewports - vp);
		}
		CursorOn = CursorOnMap;

		//
		// Look if there is an unit under the cursor.
		// FIXME: use Viewport2MapX Viewport2MapY
		//
		UnitUnderCursor = UnitOnScreen(NULL,
			CursorX - TheUI.MouseViewport->X +
				TheUI.MouseViewport->MapX * TileSizeX + TheUI.MouseViewport->OffsetX,
			CursorY - TheUI.MouseViewport->Y +
				TheUI.MouseViewport->MapY * TileSizeY + TheUI.MouseViewport->OffsetY);
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

	ClearStatusLine();
}

/**
**  Callback for exit.
*/
static void EditorCallbackExit(void)
{
}

/**
**  Create editor.
*/
static void CreateEditor(void)
{
	int i;
	int n;
	char* file;
	char buf[PATH_MAX];
	CLFile* clf;

	//
	// Load and evaluate the editor configuration file
	// FIXME: the CLopen is very slow and repeats the work of LibraryFileName.
	//
	file = LibraryFileName(EditorStartFile, buf);
	if ((clf = CLopen(file, CL_OPEN_READ))) {
		CLclose(clf);
		ShowLoadProgress("Script %s", file);
		LuaLoadFile(file);
		CclGarbageCollect(0); // Cleanup memory after load
	}

	ThisPlayer = &Players[0];

	FlagRevealMap = 1; // editor without fog and all visible
	TheMap.NoFogOfWar = 1;

	if (!*CurrentMapPath) { // new map!
		InitUnitTypes(1);
		//
		// Inititialize TheMap / Players.
		//
		TheMap.Info->MapTerrainName =
			strdup(Tilesets[TheMap.Info->MapTerrain]->Ident);
		InitPlayers();
		for (i = 0; i < PlayerMax; ++i) {
			int j;

			if (i == PlayerNumNeutral) {
				CreatePlayer(PlayerNeutral);
				TheMap.Info->PlayerType[i] = PlayerNeutral;
				TheMap.Info->PlayerSide[i] = Players[i].Race = PlayerRaceNeutral;
			} else {
				CreatePlayer(PlayerNobody);
				TheMap.Info->PlayerType[i] = PlayerNobody;
			}
			for (j = 1; j < MaxCosts; ++j) {
				TheMap.Info->PlayerResources[i][j] = Players[i].Resources[j];
			}
		}

		strncpy(TheMap.Description, TheMap.Info->Description, 32);
		TheMap.Width = TheMap.Info->MapWidth;
		TheMap.Height = TheMap.Info->MapHeight;
		TheMap.Fields = calloc(TheMap.Width * TheMap.Height, sizeof(MapField));
		TheMap.Visible[0] = calloc(TheMap.Width * TheMap.Height / 8, 1);
		InitUnitCache();

		TheMap.Terrain = TheMap.Info->MapTerrain;
		TheMap.TerrainName = strdup(Tilesets[TheMap.Info->MapTerrain]->Ident);
		TheMap.Tileset = Tilesets[TheMap.Info->MapTerrain];
		LoadTileset();

		for (i = 0; i < TheMap.Width * TheMap.Height; ++i) {
			TheMap.Fields[i].Tile = TheMap.Fields[i].SeenTile = 0;
			TheMap.Fields[i].Tile = TheMap.Fields[i].SeenTile =
				TheMap.Tileset->Table[0x50];
			TheMap.Fields[i].Flags = TheMap.Tileset->FlagsTable[0x50];
		}
		GameSettings.Resources = SettingsResourcesMapDefault;
		CreateGame(NULL, &TheMap);
	} else {
		CreateGame(CurrentMapPath, &TheMap);
	}

	ReplayRevealMap = 1;
	FlagRevealMap = 0;
	SelectedPlayer = 15;

	//
	// Place the start points, which the loader discarded.
	//
	for (i = 0; i < PlayerMax; ++i) {
		if (TheMap.Info->PlayerType[i] != PlayerNobody) {
			// Set SelectedPlayer to a valid player
			if (SelectedPlayer == 15) {
				SelectedPlayer = i;
			}
#if 0
			// FIXME: must support more races
			switch (TheMap.Info->PlayerSide[i]) {
				case PlayerRaceHuman:
					MakeUnitAndPlace(Players[i].StartX, Players[i].StartY,
						UnitTypeByWcNum(WC_StartLocationHuman),
						Players + i);
					break;
				case PlayerRaceOrc:
					MakeUnitAndPlace(Players[i].StartX, Players[i].StartY,
						UnitTypeByWcNum(WC_StartLocationOrc),
						Players + i);
					break;
			}
#endif
		} else if (Players[i].StartX | Players[i].StartY) {
			DebugPrint("Player nobody has a start position\n");
		}
	}

	if (!EditorUnitTypes) {
		//
		// Build editor unit-type tables.
		//
		i = 0;
		while (UnitTypeWcNames[i]) {
			++i;
		}
		n = i + 1;
		EditorUnitTypes = malloc(sizeof(char*) * n);
		for (i = 0; i < n; ++i) {
			EditorUnitTypes[i] = UnitTypeWcNames[i];
		}
		MaxUnitIndex = n - 1;
	}
	CalculateMaxIconSize();
	ShowUnitsToSelect = 1; // Show all units as default
	ShowBuildingsToSelect = 1;
#if 0
	ShowHeroesToSelect = 1;
#endif
	ShowAirToSelect = 1;
	ShowLandToSelect = 1;
	ShowWaterToSelect = 1;

	RecalculateShownUnits();
	UpdateMinimap();

	if (1) {
		ProcessMenu("menu-editor-tips", 1);
		InterfaceState = IfaceStateNormal;
	}
}

/**
**  Save a pud from editor.
**
**  @param file  Save the level to this file.
**
**  @return      0 for success, -1 for error
**
**  @todo  FIXME: Check if the pud is valid, contains no failures.
**         At least two players, one human slot, every player a startpoint
**         ...
*/
int EditorSavePud(const char* file)
{
	int i;

	for (i = 0; i < NumUnits; ++i) {
		const UnitType* type;

		type = Units[i]->Type;
		if (type == UnitTypeByWcNum(WC_StartLocationHuman) ||
				type == UnitTypeByWcNum(WC_StartLocationOrc)) {
			// FIXME: Startpoints sets the land-unit flag.
			TheMap.Fields[Units[i]->X + Units[i]->Y * TheMap.Width].Flags &=
				~MapFieldLandUnit;
		}
	}
	if (SavePud(file, &TheMap) == -1) {
		ErrorMenu("Cannot save map");
		InterfaceState = IfaceStateNormal;
		EditorUpdateDisplay();
		InterfaceState = IfaceStateMenu;
		return -1;
	}
	for (i = 0; i < NumUnits; ++i) {
		const UnitType* type;

		type = Units[i]->Type;
		if (type == UnitTypeByWcNum(WC_StartLocationHuman) ||
				type == UnitTypeByWcNum(WC_StartLocationOrc)) {
			// FIXME: Startpoints sets the land-unit flag.
			TheMap.Fields[Units[i]->X + Units[i]->Y * TheMap.Width].Flags |=
				MapFieldLandUnit;
		}
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
	EventCallback callbacks;
	int OldCommandLogDisabled;

	OldCommandLogDisabled = CommandLogDisabled;
	CommandLogDisabled = 1;

	while (1) {
		EditorMapLoaded = 0;
		EditorRunning = EditorEditing;

		CreateEditor();

		SetVideoSync();

		callbacks.ButtonPressed = EditorCallbackButtonDown;
		callbacks.ButtonReleased = EditorCallbackButtonUp;
		callbacks.MouseMoved = EditorCallbackMouse;
		callbacks.MouseExit = EditorCallbackExit;
		callbacks.KeyPressed = EditorCallbackKeyDown;
		callbacks.KeyReleased = EditorCallbackKeyUp;
		callbacks.KeyRepeated = EditorCallbackKey3;
		callbacks.NetworkEvent = NetworkEvent;

		GameCursor = TheUI.Point.Cursor;
		InterfaceState = IfaceStateNormal;
		EditorState = EditorSelecting;
		TheUI.SelectedViewport = TheUI.Viewports;
		TileCursorSize = 1;

		while (EditorRunning) {
			PlayListAdvance();

			UpdateMinimap();

			EditorUpdateDisplay();

			//
			// Map scrolling
			//
			if (TheUI.MouseScroll && !(FrameCounter%SpeedMouseScroll)) {
				DoScrollArea(MouseScrollState, 0);
			}
			if (TheUI.KeyScroll && !(FrameCounter%SpeedKeyScroll)) {
				DoScrollArea(KeyScrollState, KeyModifiers & ModifierControl);
				if (CursorOn == CursorOnMap && (MouseButtons & LeftButton) &&
						(EditorState == EditorEditTile ||
							EditorState == EditorEditUnit)) {
					EditorCallbackButtonDown(0);
				}
			}

			if (ColorCycleAll >= 0 && !(FrameCounter % COLOR_CYCLE_SPEED)) {
				ColorCycle();
			}

			WaitEventsOneFrame(&callbacks);
		}

		if (!EditorMapLoaded) {
			break;
		}

		CleanModules();

		LoadCcl(); // Reload the main config file

		PreMenuSetup();

		InterfaceState = IfaceStateMenu;
		GameCursor = TheUI.Point.Cursor;

		VideoClearScreen();
		Invalidate();
	}

	CommandLogDisabled = OldCommandLogDisabled;
}

//@}
