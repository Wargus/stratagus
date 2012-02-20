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
/**@name ui.cpp - The user interface globals. */
//
//      (c) Copyright 1999-2011 by Lutz Sammer, Andreas Arens,
//                                 Jimmy Salmon and Pali Rohár
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "stratagus.h"
#include "video.h"
#include "font.h"
#include "interface.h"
#include "map.h"
#include "tileset.h"
#include "ui.h"
#include "menus.h"
#include "iolib.h"
#include "unit.h"
#include "title.h"

/*----------------------------------------------------------------------------
-- Variables
----------------------------------------------------------------------------*/

bool RightButtonAttacks;                   /// right button attacks

static ViewportModeType NewViewportMode = VIEWPORT_SINGLE;

bool FancyBuildings;                       /// Mirror buildings 1 yes, 0 now.

/**
**  The user interface configuration
*/
CUserInterface UI;

/*----------------------------------------------------------------------------
-- Functions
----------------------------------------------------------------------------*/

CUserInterface::CUserInterface() :
	MouseScroll(false), KeyScroll(false), MouseScrollSpeed(8),
	MouseScrollSpeedDefault(0), MouseScrollSpeedControl(0),
	MouseWarpX(0), MouseWarpY(0),
	SingleSelectedButton(NULL),
	MaxSelectedFont(NULL), MaxSelectedTextX(0), MaxSelectedTextY(0),
	SingleTrainingButton(NULL),
	SingleTrainingFont(NULL), SingleTrainingTextX(0), SingleTrainingTextY(0),
	TrainingFont(NULL), TrainingTextX(0), TrainingTextY(0),
	CompletedBarColor(0), CompletedBarShadow(0),
	ViewportMode(VIEWPORT_SINGLE), MouseViewport(NULL),
	SelectedViewport(NULL), NumViewports(0),
	MessageFont(NULL), MessageScrollSpeed(5),
	ViewportCursorColor(0), Offset640X(0), Offset480Y(0),
	VictoryBackgroundG(NULL), DefeatBackgroundG(NULL)
{
	memset(&CompletedBarColorRGB, 0, sizeof(CompletedBarColorRGB));

	Point.Name = "cursor-point";
	Glass.Name = "cursor-glass";
	Cross.Name = "cursor-cross";
	YellowHair.Name = "cursor-yellow-hair";
	GreenHair.Name = "cursor-green-hair";
	RedHair.Name = "cursor-red-hair";
	Scroll.Name = "cursor-scroll";

	ArrowE.Name = "cursor-arrow-e";
	ArrowNE.Name = "cursor-arrow-ne";
	ArrowN.Name = "cursor-arrow-n";
	ArrowNW.Name = "cursor-arrow-nw";
	ArrowW.Name = "cursor-arrow-w";
	ArrowSW.Name = "cursor-arrow-sw";
	ArrowS.Name = "cursor-arrow-s";
	ArrowSE.Name = "cursor-arrow-se";

	NormalFontColor = "light-blue";
	ReverseFontColor = "yellow";
}


/**
**  Initialize the user interface.
*/
void InitUserInterface()
{
	UI.Offset640X = (Video.Width - 640) / 2;
	UI.Offset480Y = (Video.Height - 480) / 2;

	//
	// Calculations
	//
	if (Map.Info.MapWidth) {
		UI.MapArea.EndX = std::min<int>(UI.MapArea.EndX, Map.Info.MapWidth * PixelTileSize.x - 1);
		UI.MapArea.EndY = std::min<int>(UI.MapArea.EndY, Map.Info.MapHeight * PixelTileSize.y - 1);
	}

	UI.SelectedViewport = UI.Viewports;

	SetViewportMode(VIEWPORT_SINGLE);

	UI.CompletedBarColor = Video.MapRGB(TheScreen->format,
		UI.CompletedBarColorRGB.r,
		UI.CompletedBarColorRGB.g,
		UI.CompletedBarColorRGB.b);
	UI.ViewportCursorColor = ColorWhite;
}

/**
**  Load Cursor.
*/
void CursorConfig::Load()
{
	Assert(!Name.empty());
	Cursor = CursorByIdent(Name);
	if (Cursor == NULL) {
		return ;
	}
	Assert(Name == Cursor->Ident);
}

/**
**  Load the user interface graphics.
*/
void CUserInterface::Load()
{
	//  Load graphics
	const int size = (int)Fillers.size();
	for (int i = 0; i < size; ++i) {
		Fillers[i].Load();
	}

	for (int i = 0; i <= ScoreCost; ++i) {
		if (Resources[i].G) {
			Resources[i].G->Load();
			Resources[i].G->UseDisplayFormat();
		}
	}

	if (InfoPanel.G) {
		InfoPanel.G->Load();
		InfoPanel.G->UseDisplayFormat();
	}
	if (ButtonPanel.G) {
		ButtonPanel.G->Load();
		ButtonPanel.G->UseDisplayFormat();
	}
	if (PieMenu.G) {
		PieMenu.G->Load();
		PieMenu.G->UseDisplayFormat();
	}

	//  Resolve cursors
	Point.Load();
	Glass.Load();
	Cross.Load();
	YellowHair.Load();
	GreenHair.Load();
	RedHair.Load();
	Scroll.Load();

	ArrowE.Load();
	ArrowNE.Load();
	ArrowN.Load();
	ArrowNW.Load();
	ArrowW.Load();
	ArrowSW.Load();
	ArrowS.Load();
	ArrowSE.Load();
}

/**
**  Save the viewports.
**
**  @param file  Save file handle
**  @param ui    User interface to save
*/
static void SaveViewports(CFile &file, const CUserInterface &ui)
{
	// FIXME: don't save the number
	file.printf("DefineViewports(\"mode\", %d", ui.ViewportMode);
	for (int i = 0; i < ui.NumViewports; ++i) {
		const CViewport &vp = ui.Viewports[i];
		file.printf(",\n  \"viewport\", {%d, %d, %d}", vp.MapX, vp.MapY,
			vp.Unit ? UnitNumber(*vp.Unit) : -1);
	}
	file.printf(")\n\n");
}

/**
**  Save the user interface module.
**
**  @param file  Save file handle
*/
void SaveUserInterface(CFile *file)
{
	SaveViewports(*file, UI);
}

/**
**  Clean up a user interface.
*/
CUserInterface::~CUserInterface()
{
}

/**
**  Clean up the user interface module.
*/
void CleanUserInterface()
{
	// Filler
	for (int i = 0; i < (int)UI.Fillers.size(); ++i) {
		CGraphic::Free(UI.Fillers[i].G);
	}
	UI.Fillers.clear();

	// Resource Icons
	for (int i = 0; i <= ScoreCost; ++i) {
		CGraphic::Free(UI.Resources[i].G);
	}

	// Info Panel
	CGraphic::Free(UI.InfoPanel.G);
	for (std::vector<CUnitInfoPanel *>::iterator panel = UI.InfoPanelContents.begin();
			panel != UI.InfoPanelContents.end(); ++panel) {
		delete *panel;
	}
	UI.InfoPanelContents.clear();

	delete UI.SingleSelectedButton;
	UI.SelectedButtons.clear();
	delete UI.SingleTrainingButton;
	UI.SingleTrainingText.clear();
	UI.TrainingButtons.clear();
	UI.TrainingText.clear();
	delete UI.UpgradingButton;
	delete UI.ResearchingButton;
	UI.TransportingButtons.clear();

	// Button Panel
	CGraphic::Free(UI.ButtonPanel.G);

	// Pie Menu
	CGraphic::Free(UI.PieMenu.G);

	// Backgrounds
	CGraphic::Free(UI.VictoryBackgroundG);
	CGraphic::Free(UI.DefeatBackgroundG);

	// Title Screens
	if (TitleScreens) {
		for (int i = 0; TitleScreens[i]; ++i) {
			delete TitleScreens[i];
		}
		delete[] TitleScreens;
		TitleScreens = NULL;
	}
}

#ifdef DEBUG
void FreeButtonStyles()
{
	std::map<std::string, ButtonStyle *>::iterator i;
	for (i = ButtonStyleHash.begin(); i != ButtonStyleHash.end(); ++i) {
		delete (*i).second;
	}
	ButtonStyleHash.clear();
}
#endif

/**
**  Takes coordinates of a pixel in stratagus's window and computes
**  the map viewport which contains this pixel.
**
**  @param x  x pixel coordinate with origin at UL corner of screen
**  @param y  y pixel coordinate with origin at UL corner of screen
**
**  @return viewport pointer or NULL if this pixel is not inside
**  any of the viewports.
**
**  @note This functions only works with rectangular viewports, when
**  we support shaped map window, this must be rewritten.
*/
CViewport *GetViewport(int x, int y)
{
	for (CViewport *vp = UI.Viewports; vp < UI.Viewports + UI.NumViewports; ++vp) {
		if (x >= vp->X && x <= vp->EndX && y >= vp->Y && y <= vp->EndY) {
			return vp;
		}
	}
	return NULL;
}

/**
**  Takes an array of new Viewports which are supposed to have their
**  pixel geometry (CViewport::[XY] and CViewport::End[XY]) already
**  computed. Using this information as well as old viewport's
**  parameters fills in new viewports' CViewport::Map* parameters.
**  Then it replaces the old viewports with the new ones and finishes
**  the set-up of the new mode.
**
**  @param new_vps  The array of the new viewports
**  @param num_vps  The number of elements in the new_vps[] array.
*/
static void FinishViewportModeConfiguration(CViewport new_vps[], int num_vps)
{
	if (UI.NumViewports < num_vps) {
		//  Compute location of the viewport using oldviewport
		for (int i = 0; i < num_vps; ++i) {
			new_vps[i].MapX = 0;
			new_vps[i].MapY = 0;
			const CViewport *vp = GetViewport(new_vps[i].X, new_vps[i].Y);
			if (vp) {
				new_vps[i].OffsetX = new_vps[i].X - vp->X + vp->MapX * PixelTileSize.x + vp->OffsetX;
				new_vps[i].OffsetY = new_vps[i].Y - vp->Y + vp->MapY * PixelTileSize.y + vp->OffsetY;
			} else {
				new_vps[i].OffsetX = 0;
				new_vps[i].OffsetY = 0;
			}
		}
	} else {
		for (int i = 0; i < num_vps; ++i) {
			new_vps[i].MapX = UI.Viewports[i].MapX;
			new_vps[i].MapY = UI.Viewports[i].MapY;
			new_vps[i].OffsetX = UI.Viewports[i].OffsetX;
			new_vps[i].OffsetY = UI.Viewports[i].OffsetY;
		}
	}

	// Affect the old viewport.
	for (int i = 0; i < num_vps; ++i) {
		CViewport &vp = UI.Viewports[i];

		vp.X = new_vps[i].X;
		vp.EndX = new_vps[i].EndX;
		vp.Y = new_vps[i].Y;
		vp.EndY = new_vps[i].EndY;
		const Vec2i vpTilePos = {new_vps[i].MapX, new_vps[i].MapY};
		const PixelDiff offset = {new_vps[i].OffsetX, new_vps[i].OffsetY};
		vp.Set(vpTilePos, offset);
	}
	UI.NumViewports = num_vps;

	//
	//  Update the viewport pointers
	//
	UI.MouseViewport = GetViewport(CursorX, CursorY);
	if (UI.SelectedViewport > UI.Viewports + UI.NumViewports - 1) {
		UI.SelectedViewport = UI.Viewports + UI.NumViewports - 1;
	}
}

/**
**  Takes a viewport which is supposed to have its CViewport::[XY]
**  correctly filled-in and computes CViewport::End[XY] attributes
**  according to clipping information passed in other two arguments.
**
**  @param vp     The viewport.
**  @param ClipX  Maximum x-coordinate of the viewport's right side
**                as dictated by current UI's geometry and ViewportMode.
**  @param ClipY  Maximum y-coordinate of the viewport's bottom side
**                as dictated by current UI's geometry and ViewportMode.
**
**  @note It is supposed that values passed in Clip[XY] will
**  never be greater than UI::MapArea::End[XY].
**  However, they can be smaller according to the place
**  the viewport vp takes in context of current ViewportMode.
*/
static void ClipViewport(CViewport &vp, int ClipX, int ClipY)
{
	// begin with maximum possible viewport size
	vp.EndX = vp.X + Map.Info.MapWidth * PixelTileSize.x - 1;
	vp.EndY = vp.Y + Map.Info.MapHeight * PixelTileSize.y - 1;

	// first clip it to MapArea size if necessary
	vp.EndX = std::min<int>(vp.EndX, ClipX);
	vp.EndY = std::min<int>(vp.EndY, ClipY);

	Assert(vp.EndX <= UI.MapArea.EndX);
	Assert(vp.EndY <= UI.MapArea.EndY);
}

/**
**  Compute viewport parameters for single viewport mode.
**
**  The parameters include viewport's width and height expressed
**  in pixels, its position with respect to Stratagus's window
**  origin, and the corresponding map parameters expressed in map
**  tiles with origin at map origin (map tile (0,0)).
*/
static void SetViewportModeSingle()
{
	CViewport new_vps[MAX_NUM_VIEWPORTS];

	DebugPrint("Single viewport set\n");

	new_vps[0].X = UI.MapArea.X;
	new_vps[0].Y = UI.MapArea.Y;
	ClipViewport(new_vps[0], UI.MapArea.EndX, UI.MapArea.EndY);

	FinishViewportModeConfiguration(new_vps, 1);
}

/**
**  Compute viewport parameters for horizontally split viewport mode.
**  This mode splits the UI::MapArea with a horizontal line to
**  2 (approximately) equal parts.
**
**  The parameters include viewport's width and height expressed
**  in pixels, its position with respect to Stratagus's window
**  origin, and the corresponding map parameters expressed in map
**  tiles with origin at map origin (map tile (0,0)).
*/
static void SetViewportModeSplitHoriz()
{
	CViewport new_vps[MAX_NUM_VIEWPORTS];

	DebugPrint("Two horizontal viewports set\n");

	new_vps[0].X = UI.MapArea.X;
	new_vps[0].Y = UI.MapArea.Y;
	ClipViewport(new_vps[0], UI.MapArea.EndX,
		UI.MapArea.Y + (UI.MapArea.EndY - UI.MapArea.Y + 1) / 2);

	new_vps[1].X = UI.MapArea.X;
	new_vps[1].Y = new_vps[0].EndY + 1;
	ClipViewport(new_vps[1], UI.MapArea.EndX, UI.MapArea.EndY);

	FinishViewportModeConfiguration(new_vps, 2);
}

/**
**  Compute viewport parameters for horizontal 3-way split viewport mode.
**  This mode splits the UI::MapArea with a horizontal line to
**  2 (approximately) equal parts, then splits the bottom part vertically
**  to another 2 parts.
**
**  The parameters include viewport's width and height expressed
**  in pixels, its position with respect to Stratagus's window
**  origin, and the corresponding map parameters expressed in map
**  tiles with origin at map origin (map tile (0,0)).
*/
static void SetViewportModeSplitHoriz3()
{
	CViewport new_vps[MAX_NUM_VIEWPORTS];

	DebugPrint("Horizontal 3-way viewport division set\n");

	new_vps[0].X = UI.MapArea.X;
	new_vps[0].Y = UI.MapArea.Y;
	ClipViewport(new_vps[0], UI.MapArea.EndX,
		UI.MapArea.Y + (UI.MapArea.EndY - UI.MapArea.Y + 1) / 2);

	new_vps[1].X = UI.MapArea.X;
	new_vps[1].Y = new_vps[0].EndY + 1;
	ClipViewport(new_vps[1],
		UI.MapArea.X + (UI.MapArea.EndX - UI.MapArea.X + 1) / 2,
		UI.MapArea.EndY);

	new_vps[2].X = new_vps[1].EndX + 1;
	new_vps[2].Y = new_vps[0].EndY + 1;
	ClipViewport(new_vps[2], UI.MapArea.EndX, UI.MapArea.EndY);

	FinishViewportModeConfiguration(new_vps, 3);
}

/**
**  Compute viewport parameters for vertically split viewport mode.
**  This mode splits the UI::MapArea with a vertical line to
**  2 (approximately) equal parts.
**
**  The parameters  include viewport's width and height expressed
**  in pixels, its position with respect to Stratagus's window
**  origin, and the corresponding map parameters expressed in map
**  tiles with origin at map origin (map tile (0,0)).
*/
static void SetViewportModeSplitVert()
{
	CViewport new_vps[MAX_NUM_VIEWPORTS];

	DebugPrint("Two vertical viewports set\n");

	new_vps[0].X = UI.MapArea.X;
	new_vps[0].Y = UI.MapArea.Y;
	ClipViewport(new_vps[0],
		UI.MapArea.X + (UI.MapArea.EndX - UI.MapArea.X + 1) / 2,
		UI.MapArea.EndY);

	new_vps[1].X = new_vps[0].EndX + 1;
	new_vps[1].Y = UI.MapArea.Y;
	ClipViewport(new_vps[1], UI.MapArea.EndX, UI.MapArea.EndY);

	FinishViewportModeConfiguration(new_vps, 2);
}

/**
**  Compute viewport parameters for 4-way split viewport mode.
**  This mode splits the UI::MapArea vertically *and* horizontally
**  to 4 (approximately) equal parts.
**
**  The parameters  include viewport's width and height expressed
**  in pixels, its position with respect to Stratagus's window
**  origin, and the corresponding map parameters expressed in map
**  tiles with origin at map origin (map tile (0,0)).
*/
static void SetViewportModeQuad()
{
	CViewport new_vps[MAX_NUM_VIEWPORTS];

	DebugPrint("Four viewports set\n");

	new_vps[0].X = UI.MapArea.X;
	new_vps[0].Y = UI.MapArea.Y;
	ClipViewport(new_vps[0],
		UI.MapArea.X + (UI.MapArea.EndX - UI.MapArea.X + 1) / 2,
		UI.MapArea.Y + (UI.MapArea.EndY - UI.MapArea.Y + 1) / 2);

	new_vps[1].X = new_vps[0].EndX + 1;
	new_vps[1].Y = UI.MapArea.Y;
	ClipViewport(new_vps[1],
		UI.MapArea.EndX,
		UI.MapArea.Y + (UI.MapArea.EndY - UI.MapArea.Y + 1) / 2);

	new_vps[2].X = UI.MapArea.X;
	new_vps[2].Y = new_vps[0].EndY + 1;
	ClipViewport(new_vps[2],
		UI.MapArea.X + (UI.MapArea.EndX - UI.MapArea.X + 1) / 2,
		UI.MapArea.EndY);

	new_vps[3].X = new_vps[1].X;
	new_vps[3].Y = new_vps[2].Y;
	ClipViewport(new_vps[3], UI.MapArea.EndX, UI.MapArea.EndY);

	FinishViewportModeConfiguration(new_vps, 4);
}

/**
**  Sets up (calls geometry setup routines for) a new viewport mode.
**
**  @param new_mode  New mode's number.
*/
void SetViewportMode(ViewportModeType new_mode)
{
	switch (UI.ViewportMode = new_mode) {
		case VIEWPORT_SINGLE:
			SetViewportModeSingle();
			break;
		case VIEWPORT_SPLIT_HORIZ:
			SetViewportModeSplitHoriz();
			break;
		case VIEWPORT_SPLIT_HORIZ3:
			SetViewportModeSplitHoriz3();
			break;
		case VIEWPORT_SPLIT_VERT:
			SetViewportModeSplitVert();
			break;
		case VIEWPORT_QUAD:
			SetViewportModeQuad();
			break;
		default:
			DebugPrint("trying to set an unknown mode!!\n");
			break;
	}
}

/**
**  Cycles through predefined viewport modes (geometry configurations)
**  in order defined by the ViewportMode enumerated type.
**
**  @param step   The size of step used for cycling. Values that
**               make sense are mostly 1 (next viewport mode) and
**               -1 (previous viewport mode).
*/
void CycleViewportMode(int step)
{
	NewViewportMode = (ViewportModeType)(UI.ViewportMode + step);
	if (NewViewportMode >= NUM_VIEWPORT_MODES) {
		NewViewportMode = VIEWPORT_SINGLE;
	}
	if (NewViewportMode < 0) {
		NewViewportMode = (ViewportModeType)(NUM_VIEWPORT_MODES - 1);
	}
}

void CheckViewportMode()
{
	if (NewViewportMode != UI.ViewportMode) {
		SetViewportMode(NewViewportMode);
	}
}

void UpdateViewports()
{
	for (CViewport *vp = UI.Viewports; vp < UI.Viewports + UI.NumViewports; ++vp) {
		vp->UpdateUnits();
	}
}

/**
**  Check if mouse scrolling is enabled
*/
bool GetMouseScroll()
{
	return UI.MouseScroll;
}

/**
**  Enable/disable scrolling with the mouse
**
**  @param enabled  True to enable mouse scrolling, false to disable
*/
void SetMouseScroll(bool enabled)
{
	UI.MouseScroll = enabled;
}

/**
**  Check if keyboard scrolling is enabled
*/
bool GetKeyScroll()
{
	return UI.KeyScroll;
}

/**
**  Enable/disable scrolling with the keyboard
**
**  @param enabled  True to enable keyboard scrolling, false to disable
*/
void SetKeyScroll(bool enabled)
{
	UI.KeyScroll = enabled;
}

/**
**  Check if mouse grabbing is enabled
*/
bool GetGrabMouse()
{
	return SdlGetGrabMouse();
}

/**
**  Enable/disable grabbing the mouse
**
**  @param enabled  True to enable mouse grabbing, false to disable
*/
void SetGrabMouse(bool enabled)
{
	ToggleGrabMouse(enabled ? 1 : -1);
}

/**
**  Check if scrolling stops when leaving the window
*/
bool GetLeaveStops()
{
	return LeaveStops;
}

/**
**  Enable/disable leaving the window stops scrolling
**
**  @param enabled  True to stop scrolling, false to disable
*/
void SetLeaveStops(bool enabled)
{
	LeaveStops = enabled;
}

//@}
