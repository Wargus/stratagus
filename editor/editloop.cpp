//     ____                _       __               
//    / __ )____  _____   | |     / /___ ___________
//   / __  / __ \/ ___/   | | /| / / __ `/ ___/ ___/
//  / /_/ / /_/ (__  )    | |/ |/ / /_/ / /  (__  ) 
// /_____/\____/____/     |__/|__/\__,_/_/  /____/  
//                                              
//       A futuristic real-time strategy game.
//          This file is part of Bos Wars.
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

//----------------------------------------------------------------------------
//  Documentation
//----------------------------------------------------------------------------

/**
**  @page EditorModule Module - Editor
**
**  This is a very simple editor for the Stratagus engine.
**
**  @section Missing Missing features
**
**    @li Edit allow section
**    @li Good keyboard bindings
**    @li Script support
**    @li Commandline support
**    @li Cut&Paste
**    @li More random map functions.
*/

/*----------------------------------------------------------------------------
--  Includes
----------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <sstream>
#include <deque>

#include "stratagus.h"
#include "unittype.h"
#include "unit_cache.h"
#include "video.h"
#include "map.h"
#include "patch.h"
#include "patch_type.h"
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
#include "replay.h"
#include "guichan.h"

#include "script.h"

extern void DoScrollArea(int state, bool fast);
extern void DrawGuichanWidgets();
extern void CleanGame();

/*----------------------------------------------------------------------------
--  Defines
----------------------------------------------------------------------------*/

#define IconSpacing (IconWidth + 2)
#define UNIT_ICON_X (IconSpacing)          /// Unit mode icon
#define UNIT_ICON_Y (0)                    /// Unit mode icon
#define PATCH_ICON_X (IconSpacing * 2)     /// Tile mode icon
#define PATCH_ICON_Y (0)                   /// Tile mode icon
#define START_ICON_X (IconSpacing * 3)     /// Start mode icon
#define START_ICON_Y (0)                   /// Start mode icon

/*----------------------------------------------------------------------------
--  Variables
----------------------------------------------------------------------------*/

CEditor Editor;

const char *EditorStartFile;  /// Editor CCL start file

static int IconWidth;                       /// Icon width in panels
static int IconHeight;                      /// Icon height in panels

static int ButtonPanelWidth;
static int ButtonPanelHeight;

static CPatch *PatchUnderCursor;            /// Patch under cursor
static int PatchOffsetX;
static int PatchOffsetY;
static bool UnitPlacedThisPress = false;    /// Only allow one unit per press
static bool UpdateMinimap = false;          /// Update units on the minimap
static bool UpdateMinimapTerrain = false;   /// Terrain has changed, minimap needs updating
static bool DraggingPatch = false;          /// The user is currently dragging a patch
static int VisibleIcons;                    /// Number of icons that are visible at a time

enum _mode_buttons_ {
	SelectButton = 201,  /// Select mode button
	UnitButton,          /// Unit mode button
	PatchButton,         /// Patch mode button
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
static gcn::Slider *editorPatchSlider;

class EditorUnitSliderListener : public gcn::ActionListener
{
public:
	virtual void action(const std::string &eventId) {
		int iconsPerStep = VisibleIcons;
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

class EditorPatchSliderListener : public gcn::ActionListener
{
public:
	virtual void action(const std::string &eventId) {
		int iconsPerStep = VisibleIcons;
		int steps = (Editor.ShownPatchTypes.size() + iconsPerStep - 1) / iconsPerStep;
		double value = editorPatchSlider->getValue();
		for (int i = 1; i <= steps; ++i) {
			if (value <= (double)i / steps) {
				Editor.PatchIndex = iconsPerStep * (i - 1);
				break;
			}
		}
	}
};

static EditorPatchSliderListener *editorPatchSliderListener;

/*----------------------------------------------------------------------------
--  Functions
----------------------------------------------------------------------------*/

extern void InitDefinedVariables();

/**
**  Set the editor's select icon
**
**  @param icon  The icon to use.
*/
void SetEditorSelectIcon(const std::string &icon)
{
	Editor.Select.Name = icon;
}

/**
**  Set the editor's units icon
**
**  @param icon  The icon to use.
*/
void SetEditorUnitsIcon(const std::string &icon)
{
	Editor.Units.Name = icon;
}

/**
**  Set the editor's patch icon
**
**  @param icon  The icon to use.
*/
void SetEditorPatchIcon(const std::string &icon)
{
	Editor.Patch.Name = icon;
}

/**
**  Set the editor's start location unit
**
**  @param name  The name of the unit to use.
*/
void SetEditorStartUnit(const std::string &name)
{
	Editor.StartUnitName = name;
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
*/
static void EditorActionPlaceUnit(int x, int y, CUnitType *type, CPlayer *player)
{
	CUnit *unit;
	CBuildRestrictionOnTop *b;

	if (type->Neutral) {
		player = &Players[PlayerNumNeutral];
	}

	unit = MakeUnitAndPlace(x, y, type, player);
	if (unit == NoUnitP) {
		DebugPrint("Unable to allocate Unit");
		return;
	}

	b = OnTopDetails(unit, NULL);
	if (b && b->ReplaceOnBuild) {
		int n;
		CUnit *table[UnitMax];

		n = UnitCache.Select(x, y, table, UnitMax);
		while (n--) {
			if (table[n]->Type == b->Parent) {
				// We capture the value of what is beneath.
				memcpy(unit->ResourcesHeld, table[n]->ResourcesHeld, sizeof(unit->ResourcesHeld));
				table[n]->Remove(NULL); // Destroy building beneath
				UnitLost(table[n]);
				UnitClearOrders(table[n]);
				table[n]->Release();
				break;
			}
		}
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
static int CalculateVisibleIcons()
{
	int i;
	int x;
	int count;

	i = 0;
	count = 0;
	x = UI.ButtonPanel.Y + 24;
	while (x < UI.ButtonPanel.Y + ButtonPanelHeight - IconHeight) {
		++i;
		x += IconHeight + 2;
	}
	x = UI.ButtonPanel.X + 10;
	while (x < UI.ButtonPanel.X + ButtonPanelWidth - IconWidth) {
		count += i;
		x += IconWidth + 8;
	}
	return count;
}

/**
**  Calculate the max height and the max width of icons,
**  and assign them to IconHeight and IconWidth
*/
static void CalculateMaxIconSize()
{
	const CUnitType *type;
	const CIcon *icon;

	IconWidth = 0;
	IconHeight = 0;
	for (int i = 0; i < (int)Editor.UnitTypes.size(); ++i) {
		type = UnitTypeByIdent(Editor.UnitTypes[i]);
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
static void RecalculateShownUnits()
{
	const CUnitType *type;

	Editor.ShownUnitTypes.clear();

	for (int i = 0; i < (int)Editor.UnitTypes.size(); ++i) {
		type = UnitTypeByIdent(Editor.UnitTypes[i]);
		Editor.ShownUnitTypes.push_back(type);
	}

	if (Editor.UnitIndex >= (int)Editor.ShownUnitTypes.size()) {
		Editor.UnitIndex = Editor.ShownUnitTypes.size() / VisibleIcons * VisibleIcons;
	}
	// Quick & dirty make them invalid
	Editor.CursorUnitIndex = -1;
	Editor.SelectedUnitIndex = -1;
}


static MenuScreen *editAiMenu;
static gcn::Label *editAiLabel;
static gcn::CheckBox *editAiCheckBox;
static gcn::Button *editAiOKButton;
static gcn::Button *editAiCancelButton;

static void CleanEditAi()
{
	delete editAiMenu;
	editAiMenu = NULL;
	delete editAiLabel;
	editAiLabel = NULL;
	delete editAiCheckBox;
	editAiCheckBox = NULL;
	delete editAiOKButton;
	editAiOKButton = NULL;
	delete editAiCancelButton;
	editAiCancelButton = NULL;
}

/*----------------------------------------------------------------------------
--  Display
----------------------------------------------------------------------------*/

/**
**  Draw a table with the players
*/
static void DrawPlayers() 
{
	int x = UI.InfoPanel.X + 8;
	int y = UI.InfoPanel.Y + 4 + IconHeight + 10;

	for (int i = 0; i < PlayerMax; ++i) {
		if (i == Editor.CursorPlayer && Map.Info.PlayerType[i] != PlayerNobody) {
			Video.DrawRectangle(ColorWhite, x + i * 20, y, 20, 20);
		}
		Video.DrawRectangle(
			i == Editor.CursorPlayer && Map.Info.PlayerType[i] != PlayerNobody ?
				ColorWhite : ColorGray,
			x + i * 20, y, 19, 19);
		if (Map.Info.PlayerType[i] != PlayerNobody) {
			Video.FillRectangle(Players[i].Color, x + 1 + i * 20, y + 1,
				17, 17);
		}
		if (i == Editor.SelectedPlayer) {
			Video.DrawRectangle(ColorGreen, x + 1 + i * 20, y + 1, 17, 17);
		}
		std::ostringstream o;
		o << i;
		VideoDrawTextCentered(x + i * 20 + 9, y + 3, SmallFont, o.str());
	}
	
	x = UI.InfoPanel.X + 4;
	y += 18 * 1 + 4;
	if (Editor.SelectedPlayer != -1) {
		std::ostringstream o;
		o << _("Player") << " " << Editor.SelectedPlayer << " ";

		switch (Map.Info.PlayerType[Editor.SelectedPlayer]) {
			case PlayerNeutral:
				o << _("Neutral");
				break;
			case PlayerNobody:
			default:
				o << _("Nobody");
				break;
			case PlayerPerson:
				o << _("Person");
				break;
			case PlayerComputer:
			case PlayerRescuePassive:
			case PlayerRescueActive:
				o << _("Computer");
				break;
		}

		VideoDrawText(x, y, GameFont, o.str());
	}
}


/**
**  Draw unit icons.
*/
static void DrawUnitIcons()
{
	int x;
	int y;
	int i;
	CIcon *icon;

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
			}

			x += IconWidth + 8;
			++i;
		}
		y += IconHeight + 2;
	}
}

/**
**  Draw patch icons.
*/
static void DrawPatchIcons()
{
	int x;
	int y;
	int i;
	CGraphic *g;

	x = UI.InfoPanel.X + 10;
	y = UI.InfoPanel.Y + 140;

	//
	//  Draw the patch icons.
	//
	y = UI.ButtonPanel.Y + 24;
	i = Editor.PatchIndex;
	while (y < UI.ButtonPanel.Y + ButtonPanelHeight - IconHeight) {
		if (i >= (int)Editor.ShownPatchTypes.size()) {
			break;
		}
		x = UI.ButtonPanel.X + 10;
		while (x < UI.ButtonPanel.X + ButtonPanelWidth - IconWidth) {
			if (i >= (int) Editor.ShownPatchTypes.size()) {
				break;
			}
			g = Editor.ShownPatchTypes[i].G;
			g->DrawClip(x, y);

			Video.DrawRectangleClip(ColorGray, x, y, IconWidth, IconHeight);
			if (i == Editor.SelectedPatchIndex) {
				Video.DrawRectangleClip(ColorGreen, x + 1, y + 1,
					IconWidth - 2, IconHeight - 2);
			}
			if (i == Editor.CursorPatchIndex) {
				Video.DrawRectangleClip(ColorWhite, x - 1, y - 1,
					IconWidth + 2, IconHeight + 2);
			}

			x += IconWidth + 8;
			++i;
		}
		y += IconHeight + 2;
	}
}

/**
**  Draw the editor panels.
*/
static void DrawEditorPanel()
{
	int x;
	int y;
	CIcon *icon;

	x = UI.InfoPanel.X + 4;
	y = UI.InfoPanel.Y + 4;

	//
	// Select / Units / Patch / Start
	// FIXME: wrong button style
	//
	icon = Editor.Select.Icon;
	icon->DrawUnitIcon(Players, UI.SingleSelectedButton->Style,
		(ButtonUnderCursor == SelectButton ? IconActive : 0) |
			(Editor.State == EditorSelecting ? IconSelected : 0),
		x, y, "");

	icon = Editor.Units.Icon;
	icon->DrawUnitIcon(Players, UI.SingleSelectedButton->Style,
		(ButtonUnderCursor == UnitButton ? IconActive : 0) |
			(Editor.State == EditorEditUnit ? IconSelected : 0),
		x + UNIT_ICON_X, y + UNIT_ICON_Y, "");

	icon = Editor.Patch.Icon;
	icon->DrawUnitIcon(Players, UI.SingleSelectedButton->Style,
		(ButtonUnderCursor == PatchButton ? IconActive : 0) |
			(Editor.State == EditorEditPatch ? IconSelected : 0),
		x + PATCH_ICON_X, y + PATCH_ICON_Y, "");

	icon = Editor.StartUnit->Icon.Icon;
	icon->DrawUnitIcon(Players, UI.SingleSelectedButton->Style,
		(ButtonUnderCursor == StartButton ? IconActive : 0) |
			(Editor.State == EditorSetStartLocation ? IconSelected : 0),
		x + START_ICON_X, y + START_ICON_Y, "");

	switch (Editor.State) {
		case EditorSelecting:
			break;
		case EditorEditPatch:
			DrawPatchIcons();
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
	int x;
	int y;

	//
	//  Affect CursorBuilding if necessary.
	//  (Menu reset CursorBuilding)
	//
	if (!CursorBuilding) {
		switch (Editor.State) {
			case EditorSelecting:
			case EditorEditPatch:
				break;
			case EditorEditUnit:
				if (Editor.SelectedUnitIndex != -1) {
					CursorBuilding = const_cast<CUnitType *> (Editor.ShownUnitTypes[Editor.SelectedUnitIndex]);
				}
				break;
			case EditorSetStartLocation:
				CursorBuilding = const_cast<CUnitType *>(Editor.StartUnit);
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
		if (Editor.State == EditorEditPatch) {
			PushClipping();
			SetClipping(UI.MouseViewport->X, UI.MouseViewport->Y,
				UI.MouseViewport->EndX, UI.MouseViewport->EndY);
			// FIXME: PATCHES
			PopClipping();
		} else {
			//
			//  If there is a unit under the cursor, its selection thing
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
static void DrawStartLocations()
{
	for (const CViewport *vp = UI.Viewports; vp < UI.Viewports + UI.NumViewports; ++vp) {
		PushClipping();
		SetClipping(vp->X, vp->Y, vp->EndX, vp->EndY);

		for (int i = 0; i < PlayerMax; i++) {
			if (Map.Info.PlayerType[i] != PlayerNobody && Map.Info.PlayerType[i] != PlayerNeutral) {
				int x = vp->Map2ViewportX(Players[i].StartX);
				int y = vp->Map2ViewportY(Players[i].StartY);

				DrawUnitType(Editor.StartUnit, Editor.StartUnit->Sprite, i, 0, x, y);
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

	sprintf_s(buf, sizeof(buf), "Editor (%d %d)", x, y);
	VideoDrawText(UI.ResourceX + 2, UI.ResourceY + 2, GameFont, buf);

	//
	// Flags info
	//
	flags = Map.Field(x, y)->Flags;
	sprintf_s(buf, sizeof(buf), "%02X|%04X|%c%c%c%c%c%c%c%c%c%c%c%c%c",
		Map.Field(x, y)->Value, flags,
		flags & MapFieldUnpassable   ? 'u' : '-',
		flags & MapFieldNoBuilding   ? 'n' : '-',
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
	tile = Map.Field(x, y)->Tile;

	for (i = 0; i < Map.Tileset.NumTiles; ++i) {
		if (tile == Map.Tileset.Table[i]) {
			break;
		}
	}

	Assert(i != Map.Tileset.NumTiles);

	sprintf_s(buf, sizeof(buf), "%d %s %s", tile,
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
	std::ostringstream o;

	o << "#" << UnitNumber(unit) << " '" << unit->Type->Name
	  << "' - " << _("Player") << ": " << unit->Player->Index;

	if (unit->Type->CanHarvestFrom) {
		int res = unit->Type->ProductionCosts[0] ? 0 : 1;
		o << " - " << _("Amount of") << " " << DefaultResourceNames[res] << ": "
		  << unit->ResourcesHeld[res] / CYCLES_PER_SECOND;
	}

	UI.StatusLine.Set(o.str());
}

/**
**  Show info about a patch.
**
**  @param patch  Patch pointer.
*/
static void ShowPatchInfo(const CPatch *patch)
{
	std::ostringstream o;

	o << _("Patch") << ": " << patch->getType()->getName() << " - ("
	  << patch->getX() << ", " << patch->getY() << ")";

	UI.StatusLine.Set(o.str());
}

/**
**  Update editor display.
*/
static void EditorUpdateDisplay()
{
	DrawMapArea();

	DrawStartLocations();

	//
	// Fillers
	//
	for (int i = 0; i < (int)UI.Fillers.size(); ++i) {
		UI.Fillers[i].G->DrawClip(UI.Fillers[i].X, UI.Fillers[i].Y);
	}

	if (CursorOn == CursorOnMap && Gui->getTop() == editorContainer) {
		DrawMapCursor();
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
		DraggingPatch = false;
	}

	Editor.SelectedPatchIndex = -1;
}

/**
**  Called if mouse button pressed down.
**
**  @param button  Mouse button number (0 left, 1 middle, 2 right)
*/
static void EditorCallbackButtonDown(unsigned button)
{
	if ((button >> MouseHoldShift) != 0) {
		// Ignore repeated events when holding down a button
		return;
	}

	Editor.SelectedPatchIndex = -1;

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
				Editor.ShowPatchOutlines = false;
				editorUnitSlider->setVisible(false);
				editorPatchSlider->setVisible(false);
				return;
			case UnitButton:
				Editor.State = EditorEditUnit;
				Editor.ShowPatchOutlines = false;
				editorUnitSlider->setVisible(true);
				editorPatchSlider->setVisible(false);
				return;
			case PatchButton :
				if (EditorEditPatch) {
					Editor.State = EditorEditPatch;
					Editor.ShowPatchOutlines = true;
				}
				editorUnitSlider->setVisible(false);
				editorPatchSlider->setVisible(true);
				return;
			case StartButton:
				Editor.State = EditorSetStartLocation;
				Editor.ShowPatchOutlines = false;
				editorUnitSlider->setVisible(false);
				editorPatchSlider->setVisible(false);
				return;
			default:
				break;
		}
	}
	
	//
	// Click on player area
	//
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
	// Click on patch area
	//
	if (Editor.State == EditorEditPatch) {
		// Cursor on patch icons
		if (Editor.CursorPatchIndex != -1) {
			Editor.SelectedPatchIndex = Editor.CursorPatchIndex;
			return;
		}
	}

	//
	// Click on map area
	//
	if (CursorOn == CursorOnMap) {
		CViewport *vp;

		vp = GetViewport(CursorX, CursorY);
		Assert(vp);
		if ((MouseButtons & LeftButton) && UI.SelectedViewport != vp) {
			// viewport changed
			UI.SelectedViewport = vp;
		}

		if (MouseButtons & LeftButton) {
			if (Editor.State == EditorEditPatch) {
				if (PatchUnderCursor) {
					int tileX = UI.MouseViewport->Viewport2MapX(CursorX);
					int tileY = UI.MouseViewport->Viewport2MapY(CursorY);
					PatchOffsetX = tileX - PatchUnderCursor->getX();
					PatchOffsetY = tileY - PatchUnderCursor->getY();
					DraggingPatch = true;
				}
			} else if (Editor.State == EditorEditUnit) {
				if (!UnitPlacedThisPress && CursorBuilding) {
					if (CanBuildUnitType(NULL, CursorBuilding,
							UI.MouseViewport->Viewport2MapX(CursorX),
							UI.MouseViewport->Viewport2MapY(CursorY), 1)) {
						PlayGameSound(GameSounds.PlacementSuccess.Sound,
							MaxSampleVolume);
						EditorPlaceUnit(UI.MouseViewport->Viewport2MapX(CursorX),
							UI.MouseViewport->Viewport2MapY(CursorY),
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
				Players[Editor.SelectedPlayer].StartX = UI.MouseViewport->Viewport2MapX(CursorX);
				Players[Editor.SelectedPlayer].StartY = UI.MouseViewport->Viewport2MapY(CursorY);
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
	if (HandleKeyModifiersDown(key, keychar)) {
		return;
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
			if (PatchUnderCursor) {
				Map.PatchManager.remove(PatchUnderCursor);
				PatchUnderCursor = NULL;
				UI.StatusLine.Set(_("Patch deleted"));
				UpdateMinimapTerrain = true;
			} else if (UnitUnderCursor) {
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

		case '0': case '1': case '2': // 0-8 change player owner
		case '3': case '4': case '5':
		case '6': case '7': case '8':
			if (UnitUnderCursor && Map.Info.PlayerType[(int)key - '0'] != PlayerNobody) {
				UnitUnderCursor->ChangeOwner(&Players[(int)key - '0']);
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

	HandleCursorMove(&x, &y); // Reduce to screen

	//
	// Move map.
	//
	if (GameCursor == UI.Scroll.Cursor) {
		MouseScrollMap(x, y);
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
	// Dragging patches or units on map.
	//
	if (CursorOn == CursorOnMap && (MouseButtons & LeftButton) &&
			(Editor.State == EditorEditPatch || Editor.State == EditorEditUnit)) {

		//
		// Scroll the map
		//
		if (CursorX <= UI.SelectedViewport->X) {
			UI.SelectedViewport->Set(
				UI.SelectedViewport->MapX - 1,
				UI.SelectedViewport->MapY,
				UI.SelectedViewport->OffsetX,
				UI.SelectedViewport->OffsetY);
		} else if (CursorX >= UI.SelectedViewport->EndX) {
			UI.SelectedViewport->Set(
				UI.SelectedViewport->MapX + 1,
				UI.SelectedViewport->MapY,
				UI.SelectedViewport->OffsetX,
				UI.SelectedViewport->OffsetY);
		}

		if (CursorY <= UI.SelectedViewport->Y) {
			UI.SelectedViewport->Set(
				UI.SelectedViewport->MapX,
				UI.SelectedViewport->MapY - 1,
				UI.SelectedViewport->OffsetX,
				UI.SelectedViewport->OffsetY);
		} else if (CursorY >= UI.SelectedViewport->EndY) {
			UI.SelectedViewport->Set(
				UI.SelectedViewport->MapX,
				UI.SelectedViewport->MapY + 1,
				UI.SelectedViewport->OffsetX,
				UI.SelectedViewport->OffsetY);
		}

		//
		// Scroll the map, if cursor moves outside the viewport.
		//
		RestrictCursorToViewport();

		int tileX = UI.SelectedViewport->Viewport2MapX(CursorX);
		int tileY = UI.SelectedViewport->Viewport2MapY(CursorY);

		if (Editor.State == EditorEditPatch && PatchUnderCursor != NULL) {
			// Drag patch under cursor
			Map.PatchManager.move(PatchUnderCursor, tileX - PatchOffsetX, tileY - PatchOffsetY);
			ShowPatchInfo(PatchUnderCursor);
			UpdateMinimapTerrain = true;
		} else if (Editor.State == EditorEditUnit && CursorBuilding) {
			if (!UnitPlacedThisPress) {
				if (CanBuildUnitType(NULL, CursorBuilding, tileX, tileY, 1)) {
					EditorPlaceUnit(tileX, tileY, CursorBuilding, Players + Editor.SelectedPlayer);
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
	Editor.CursorPatchIndex = -1;
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
			if (bx < x && x < bx + 20 && by < y && y < by + 20) {
				if (Map.Info.PlayerType[i] != PlayerNobody) {
					std::ostringstream o;
					o << _("Select player #") << i;
					UI.StatusLine.Set(o.str());
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
					std::ostringstream o;
					o << Editor.ShownUnitTypes[i]->Ident << " \""
					  << Editor.ShownUnitTypes[i]->Name << "\"";
					UI.StatusLine.Set(o.str());
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
	// Handle patch area
	//
	if (Editor.State == EditorEditPatch) {
		i = Editor.PatchIndex;
		by = UI.ButtonPanel.Y + 24;
		while (by < UI.ButtonPanel.Y + ButtonPanelHeight - IconHeight) {
			if (i >= (int)Editor.ShownPatchTypes.size()) {
				break;
			}
			bx = UI.ButtonPanel.X + 10;
			while (bx < UI.ButtonPanel.X + 146) {
				if (i >= (int)Editor.ShownPatchTypes.size()) {
					break;
				}
				if (bx < x && x < bx + IconWidth &&
						by < y && y < by + IconHeight) {
					CPatchType *patchType = Editor.ShownPatchTypes[i].PatchType;
					std::ostringstream ostr;
					ostr << patchType->getName() << " ("
					     << patchType->getTileWidth() << "x"
						 << patchType->getTileHeight() << ")";
					UI.StatusLine.Set(ostr.str());
					Editor.CursorPatchIndex = i;
					PatchUnderCursor = NULL;
					return;
				}
				bx += IconWidth + 8;
				i++;
			}
			by += IconHeight + 2;
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
	if (UI.InfoPanel.X + 4 + PATCH_ICON_X < CursorX &&
			CursorX < UI.InfoPanel.X + 4 + PATCH_ICON_X + Editor.Patch.Icon->G->Width &&
			UI.InfoPanel.Y + 4 + PATCH_ICON_Y < CursorY &&
			CursorY < UI.InfoPanel.Y + 4 + PATCH_ICON_Y + Editor.Patch.Icon->G->Height) {
		ButtonAreaUnderCursor = -1;
		ButtonUnderCursor = PatchButton;
		CursorOn = CursorOnButton;
		UI.StatusLine.Set(_("Patch mode"));
		return;
	}
	if (UI.InfoPanel.X + 4 + START_ICON_X < CursorX &&
			CursorX < UI.InfoPanel.X + 4 + START_ICON_X + Editor.StartUnit->Icon.Icon->G->Width &&
			UI.InfoPanel.Y + 4 + START_ICON_Y < CursorY &&
			CursorY < UI.InfoPanel.Y + 4 + START_ICON_Y + Editor.StartUnit->Icon.Icon->G->Height) {
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
	PatchUnderCursor = NULL;
	UnitUnderCursor = NULL;
	if (x >= UI.MapArea.X && x <= UI.MapArea.EndX &&
			y >= UI.MapArea.Y && y <= UI.MapArea.EndY) {
		CViewport *vp = GetViewport(x, y);
		if (UI.MouseViewport != vp) { // viewport changed
			UI.MouseViewport = vp;
		}
		CursorOn = CursorOnMap;

		if (Editor.State == EditorEditPatch) {
			int tileX = UI.MouseViewport->Viewport2MapX(CursorX);
			int tileY = UI.MouseViewport->Viewport2MapY(CursorY);

			if (Editor.SelectedPatchIndex != -1) {
				// Create a new patch
				PatchUnderCursor = Map.PatchManager.add(Editor.ShownPatchTypes[Editor.SelectedPatchIndex].PatchType->getName(), tileX, tileY);
				PatchOffsetX = 0;
				PatchOffsetY = 0;
				UpdateMinimapTerrain = true;
			} else {
				// Show what patch is under the cursor
				PatchUnderCursor = Map.PatchManager.getPatch(tileX, tileY);
			}
			if (PatchUnderCursor) {
				ShowPatchInfo(PatchUnderCursor);
				return;
			}
		} else {
			// See if there is a unit under the cursor.
			UnitUnderCursor = UnitOnScreen(
				CursorX - UI.MouseViewport->X +
					UI.MouseViewport->MapX * TileSizeX + UI.MouseViewport->OffsetX,
				CursorY - UI.MouseViewport->Y +
					UI.MouseViewport->MapY * TileSizeY + UI.MouseViewport->OffsetY);

			if (UnitUnderCursor) {
				ShowUnitInfo(UnitUnderCursor);
				return;
			}
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
static void EditorCallbackExit()
{
}

/**
**  Create the patch icons
*/
static void CreatePatchIcons()
{
	std::vector<std::string> patchTypeNames;
	std::vector<std::string>::iterator i;

	// Load all of the patch types
	Map.PatchManager.loadAll();

	// Create icons out of the patch graphics
	patchTypeNames = Map.PatchManager.getPatchTypeNames();
	for (i = patchTypeNames.begin(); i != patchTypeNames.end(); ++i) {
		CPatchType *patchType = Map.PatchManager.getPatchType(*i);
		CGraphic *g = patchType->getGraphic()->Clone();

		g->Resize(IconWidth, IconHeight);

		CPatchIcon patchIcon(patchType, g);
		Editor.ShownPatchTypes.push_back(patchIcon);
	}
}

/**
**  Clean up the patch icons
*/
static void CleanPatchIcons()
{
	for (size_t i = 0; i < Editor.ShownPatchTypes.size(); ++i) {
		CGraphic::Free(Editor.ShownPatchTypes[i].G);
	}
	Editor.ShownPatchTypes.clear();
}

/**
**  Create editor.
*/
void CEditor::Init()
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
				Map.Info.PlayerSide[i] = 0;
			} else {
				CreatePlayer(PlayerNobody);
				Map.Info.PlayerType[i] = PlayerNobody;
			}
		}

		Map.Fields = new CMapField[Map.Info.MapWidth * Map.Info.MapHeight];
		Map.Visible[0] = new unsigned[Map.Info.MapWidth * Map.Info.MapHeight / 2];
		memset(Map.Visible[0], 0, Map.Info.MapWidth * Map.Info.MapHeight / 2 * sizeof(unsigned));
		UnitCache.Init(Map.Info.MapWidth, Map.Info.MapHeight);

#if 0
		for (i = 0; i < Map.Info.MapWidth * Map.Info.MapHeight; ++i) {
			Map.Fields[i].Flags = Map.Tileset.FlagsTable[0x50];
		}
#endif
		GameSettings.Resources = SettingsPresetMapDefault;
		CreateGame("", &Map);
	} else {
		CreateGame(CurrentMapPath, &Map);
	}

	ReplayRevealMap = 1;
	FlagRevealMap = 0;
	Editor.SelectedPlayer = PlayerNumNeutral;
	Editor.PatchOutlineColor = ColorBlack;
	Editor.SelectedPatchIndex = -1;

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

	ButtonPanelWidth = 200;
	ButtonPanelHeight = 160 + (Video.Height - 480);

	CalculateMaxIconSize();
	VisibleIcons = CalculateVisibleIcons();

	if (!StartUnitName.empty()) {
		StartUnit = UnitTypeByIdent(StartUnitName);
	}
	Select.Icon = NULL;
	Select.Load();
	Units.Icon = NULL;
	Units.Load();
	Patch.Icon = NULL;
	Patch.Load();

	CreatePatchIcons();

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
*/
int EditorSaveMap(const std::string &file)
{
	std::string fullName;

	fullName = StratagusLibPath + "/" + file;
	if (SaveStratagusMap(fullName, &Map) == -1) {
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
static void EditorMainLoop()
{
	bool OldCommandLogDisabled = CommandLogDisabled;
	const EventCallback *old_callbacks = GetCallbacks();

	CommandLogDisabled = true;
	SetCallbacks(&EditorCallbacks);

	gcn::Widget *oldTop = Gui->getTop();

	editorContainer = new gcn::Container();
	editorContainer->setDimension(gcn::Rectangle(0, 0, Video.Width, Video.Height));
	editorContainer->setOpaque(false);
	Gui->setTop(editorContainer);

	editorUnitSliderListener = new EditorUnitSliderListener();
	editorPatchSliderListener = new EditorPatchSliderListener();

	editorUnitSlider = new gcn::Slider();
	editorUnitSlider->setBaseColor(gcn::Color(38, 38, 78));
	editorUnitSlider->setForegroundColor(gcn::Color(200, 200, 120));
	editorUnitSlider->setBackgroundColor(gcn::Color(200, 200, 120));
	editorUnitSlider->setSize(176, 16);
	editorUnitSlider->setVisible(false);
	editorUnitSlider->addActionListener(editorUnitSliderListener);

	editorPatchSlider = new gcn::Slider();
	editorPatchSlider->setBaseColor(gcn::Color(38, 38, 78));
	editorPatchSlider->setForegroundColor(gcn::Color(200, 200, 120));
	editorPatchSlider->setBackgroundColor(gcn::Color(200, 200, 120));
	editorPatchSlider->setSize(176, 16);
	editorPatchSlider->setVisible(false);
	editorPatchSlider->addActionListener(editorPatchSliderListener);

	editorContainer->add(editorUnitSlider, UI.ButtonPanel.X + 2, UI.ButtonPanel.Y + 4);
	editorContainer->add(editorPatchSlider, UI.ButtonPanel.X + 2, UI.ButtonPanel.Y + 4);

	UpdateMinimap = true;

	while (1) {
		Editor.MapLoaded = false;
		Editor.Running = EditorEditing;

		Editor.Init();

		//ProcessMenu("menu-editor-tips", 1);
		InterfaceState = IfaceStateNormal;

		SetVideoSync();

		GameCursor = UI.Point.Cursor;
		InterfaceState = IfaceStateNormal;
		Editor.State = EditorSelecting;
		UI.SelectedViewport = UI.Viewports;

		while (Editor.Running) {
			CheckMusicFinished();

			if (FrameCounter % FRAMES_PER_SECOND == 0) {
				if (UpdateMinimapTerrain && !DraggingPatch) {
					UI.Minimap.UpdateTerrain();
					UpdateMinimapTerrain = false;
					UpdateMinimap = true;
				}
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
						(Editor.State == EditorEditPatch ||
							Editor.State == EditorEditUnit)) {
					EditorCallbackButtonDown(0);
				}
			}

			WaitEventsOneFrame();
		}

		if (!Editor.MapLoaded) {
			break;
		}

		CleanModules();
		InitDefinedVariables();

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
	delete editorPatchSliderListener;
	delete editorUnitSlider;
	delete editorPatchSlider;
}

/**
**  Start the editor
**
**  @param filename  Map to load, empty string to create a new map
*/
void StartEditor(const std::string &filename)
{
	std::string nc, rc;
	bool newMap;

	GetDefaultTextColors(nc, rc);

	DebugPrint("StartEditor - %s\n" _C_ !filename.empty() ? filename.c_str() : "new map");

	newMap = filename.empty();
	if (!newMap) {
		if (strcpy_s(CurrentMapPath, sizeof(CurrentMapPath), filename.c_str()) != 0) {
			newMap = true;
		}
	}

	if (newMap) {
		// new map, choose some default values
		strcpy_s(CurrentMapPath, sizeof(CurrentMapPath), "");
		Map.Info.Description.clear();
		// Make sure we have good values
		if (Map.Info.MapWidth < 32) {
			fprintf(stderr, "Invalid map width, using default value\n");
			Map.Info.MapWidth = 32;
		} else if (Map.Info.MapWidth > MaxMapWidth) {
			fprintf(stderr, "Invalid map width, using default value\n");
			Map.Info.MapWidth = MaxMapWidth;
		}
		if (Map.Info.MapHeight < 32) {
			fprintf(stderr, "Invalid map height, using default value\n");
			Map.Info.MapHeight = 32;
		} else if (Map.Info.MapHeight > MaxMapHeight) {
			fprintf(stderr, "Invalid map height, using default value\n");
			Map.Info.MapHeight = MaxMapHeight;
		}
	}
	
	// Run the editor.
	EditorMainLoop();

	// Clear screen
	Video.ClearScreen();
	Invalidate();

	CleanPatchIcons();
	CleanGame();
	CleanPlayers();
	CleanEditAi();

	SetDefaultTextColors(nc, rc);
}

//@}
