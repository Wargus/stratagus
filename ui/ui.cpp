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
//      (c) Copyright 1999-2005 by Lutz Sammer, Andreas Arens, and
//                                 Jimmy Salmon
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

/*----------------------------------------------------------------------------
-- Variables
----------------------------------------------------------------------------*/

char RightButtonAttacks;                   /// right button 0 move, 1 attack
bool FancyBuildings;                       /// Mirror buildings 1 yes, 0 now.


int SpeedKeyScroll = KEY_SCROLL_SPEED;     /// keyboard scroll speed
int SpeedMouseScroll = MOUSE_SCROLL_SPEED; /// mouse scroll speed

/**
**  The user interface configuration
*/
CUserInterface UI;

/**
**  The available user interfaces.
*/
CUserInterface **UI_Table;

/*----------------------------------------------------------------------------
-- Functions
----------------------------------------------------------------------------*/

static void ClipViewport(CViewport *vp, int ClipX, int ClipY);
static void FinishViewportModeConfiguration(CViewport new_vps[], int num_vps);


/**
**  Initialize the user interface.
**
**  The function looks through ::UI_Table, to find a matching user
**  interface. It uses the race_name and the current video window sizes to
**  find it.
**
**  @param race_name  The race identifier, to select the interface.
*/
void InitUserInterface(const char *race_name)
{
	int i;
	int best;
	int num_vps;
	ViewportModeType vp_mode;
	CViewport vps[MAX_NUM_VIEWPORTS];
	bool show_command_key;

	// select the correct slot
	best = 0;
	for (i = 0; UI_Table[i]; ++i) {
		if (!strcmp(race_name, UI_Table[i]->Name)) {
			// perfect
			if (Video.Width == UI_Table[i]->Width &&
					Video.Height == UI_Table[i]->Height) {
				best = i;
				break;
			}
			// too big
			if (Video.Width < UI_Table[i]->Width ||
					Video.Height < UI_Table[i]->Height) {
				continue;
			}
			// best smaller
			if (UI_Table[i]->Width * UI_Table[i]->Height >=
					UI_Table[best]->Width * UI_Table[best]->Height) {
				best = i;
			}
		}
	}

	num_vps = UI.NumViewports;
	vp_mode = UI.ViewportMode;
	for (i = 0; i < num_vps; ++i) {
		vps[i].MapX = UI.Viewports[i].MapX;
		vps[i].MapY = UI.Viewports[i].MapY;
	}
	show_command_key = UI.ButtonPanel.ShowCommandKey;

	UI = *UI_Table[best];

	UI.Offset640X = (Video.Width - 640) / 2;
	UI.Offset480Y = (Video.Height - 480) / 2;

	//
	// Calculations
	//
	if (UI.MapArea.EndX > TheMap.Info.MapWidth * TileSizeX - 1) {
		UI.MapArea.EndX = TheMap.Info.MapWidth * TileSizeX - 1;
	}
	if (UI.MapArea.EndY > TheMap.Info.MapHeight * TileSizeY - 1) {
		UI.MapArea.EndY = TheMap.Info.MapHeight * TileSizeY - 1;
	}

	UI.SelectedViewport = UI.Viewports;

	if (num_vps) {
		SetViewportMode(vp_mode);
		for (i = 0; i < num_vps; ++i) {
			UI.Viewports[i].MapX = vps[i].MapX;
			UI.Viewports[i].MapY = vps[i].MapY;
		}
		FinishViewportModeConfiguration(UI.Viewports, num_vps);
	} else {
		SetViewportMode(VIEWPORT_SINGLE);
	}
	UI.ButtonPanel.ShowCommandKey = show_command_key;

	UI.CompletedBarColor = Video.MapRGB(TheScreen->format,
		UI.CompletedBarColorRGB.r,
		UI.CompletedBarColorRGB.g,
		UI.CompletedBarColorRGB.b);
	UI.ViewportCursorColor = ColorWhite;
}

/**
**  Load the user interface graphics.
*/
void LoadUserInterface(void)
{
	int i;
	MenuPanel *menupanel;

	//
	//  Load graphics
	//
	for (i = 0; i < (int)UI.Fillers.size(); ++i) {
		UI.Fillers[i].G->Load();
	}

	for (i = 0; i <= ScoreCost; ++i) {
		if (UI.Resources[i].G) {
			UI.Resources[i].G->Load();
		}
	}

	if (UI.InfoPanel.G) {
		UI.InfoPanel.G->Load();
	}
	if (UI.ButtonPanel.G) {
		UI.ButtonPanel.G->Load();
	}
	if (UI.PieMenuBackgroundG) {
		UI.PieMenuBackgroundG->Load();
	}

	//
	//  Resolve cursors
	//
	UI.Point.Cursor = CursorByIdent(UI.Point.Name);
	UI.Glass.Cursor = CursorByIdent(UI.Glass.Name);
	UI.Cross.Cursor = CursorByIdent(UI.Cross.Name);
	UI.YellowHair.Cursor = CursorByIdent(UI.YellowHair.Name);
	UI.GreenHair.Cursor = CursorByIdent(UI.GreenHair.Name);
	UI.RedHair.Cursor = CursorByIdent(UI.RedHair.Name);
	UI.Scroll.Cursor = CursorByIdent(UI.Scroll.Name);

	UI.ArrowE.Cursor = CursorByIdent(UI.ArrowE.Name);
	UI.ArrowNE.Cursor = CursorByIdent(UI.ArrowNE.Name);
	UI.ArrowN.Cursor = CursorByIdent(UI.ArrowN.Name);
	UI.ArrowNW.Cursor = CursorByIdent(UI.ArrowNW.Name);
	UI.ArrowW.Cursor = CursorByIdent(UI.ArrowW.Name);
	UI.ArrowSW.Cursor = CursorByIdent(UI.ArrowSW.Name);
	UI.ArrowS.Cursor = CursorByIdent(UI.ArrowS.Name);
	UI.ArrowSE.Cursor = CursorByIdent(UI.ArrowSE.Name);

	menupanel = UI.MenuPanels;
	while (menupanel) {
		menupanel->G->Load();
		menupanel = menupanel->Next;
	}
}

/**
**  Save the viewports.
**
**  @param file  Save file handle
**  @param ui    User interface to save
*/
static void SaveViewports(CFile *file, const CUserInterface *ui)
{
	int i;
	const CViewport *vp;

	// FIXME: don't save the number
	file->printf("DefineViewports(\"mode\", %d", ui->ViewportMode);
	for (i = 0; i < ui->NumViewports; ++i) {
		vp = &ui->Viewports[i];
		file->printf(",\n  \"viewport\", {%d, %d, %d}", vp->MapX, vp->MapY, 
			vp->Unit ? UnitNumber(vp->Unit) : -1);
	}
	file->printf(")\n\n");
}

/**
**  Save the user interface module.
**
**  @param file  Save file handle
*/
void SaveUserInterface(CFile *file)
{
	SaveViewports(file, &UI);
}

/**
**  Clean up a user interface.
*/
void CleanUI(CUserInterface *ui)
{
	int i;
	MenuPanel *menupanel;
	MenuPanel *tmp;

	delete[] ui->Name;
	delete[] ui->NormalFontColor;
	delete[] ui->ReverseFontColor;

	// Filler
	for (i = 0; i < (int)ui->Fillers.size(); ++i) {
		CGraphic::Free(ui->Fillers[i].G);
	}

	// Resource Icons
	for (i = 0; i < MaxCosts + 2; ++i) {
		CGraphic::Free(ui->Resources[i].G);
	}

	// Info Panel
	CGraphic::Free(ui->InfoPanel.G);
	delete[] ui->PanelIndex;
	delete ui->SingleSelectedButton;
	delete ui->SingleTrainingButton;
	delete ui->UpgradingButton;
	delete ui->ResearchingButton;

	// Button Panel
	CGraphic::Free(ui->ButtonPanel.G);

	// Pie Menu
	CGraphic::Free(ui->PieMenuBackgroundG);

	// Menu Panels
	menupanel = ui->MenuPanels;
	while (menupanel) {
		tmp = menupanel;
		menupanel = menupanel->Next;
		CGraphic::Free(tmp->G);
		delete[] tmp->Ident;
		delete tmp;
	}

	// Backgrounds
	CGraphic::Free(ui->VictoryBackgroundG);
	CGraphic::Free(ui->DefeatBackgroundG);

	free(ui);
}

/**
**  Clean up the user interface module.
*/
void CleanUserInterface(void)
{
	int i;
	int j;

	//
	// Free the available user interfaces.
	//
	if (UI_Table) {
		for (i = 0; UI_Table[i]; ++i) {
			CleanUI(UI_Table[i]);
		}
		delete[] UI_Table;
		UI_Table = NULL;
	}

	for (std::vector<CUnitInfoPanel *>::iterator panel = AllPanels.begin();
		panel != AllPanels.end(); ++panel) {
		delete *panel;
	}
	AllPanels.clear();

	// Free Title screen.
	if (TitleScreens) {
		for (i = 0; TitleScreens[i]; ++i) {
			delete[] TitleScreens[i]->File;
			delete[] TitleScreens[i]->Music;
			if (TitleScreens[i]->Labels) {
				for (j = 0; TitleScreens[i]->Labels[j]; ++j) {
					delete[] TitleScreens[i]->Labels[j]->Text;
					delete TitleScreens[i]->Labels[j];
				}
				delete[] TitleScreens[i]->Labels;
			}
			delete TitleScreens[i];
		}
		delete[] TitleScreens;
		TitleScreens = NULL;
	}

	memset(&UI, 0, sizeof(UI));
}

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
	CViewport *vp;

	for (vp = UI.Viewports; vp < UI.Viewports + UI.NumViewports; ++vp) {
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
	int i;

	if (UI.NumViewports < num_vps) {
		//  Compute location of the viewport using oldviewport
		for (i = 0; i < num_vps; ++i) {
			const CViewport *vp;

			new_vps[i].MapX = 0;
			new_vps[i].MapY = 0;
			vp = GetViewport(new_vps[i].X, new_vps[i].Y);
			if (vp) {
				new_vps[i].OffsetX = new_vps[i].X - vp->X + vp->MapX * TileSizeX + vp->OffsetX;
				new_vps[i].OffsetY = new_vps[i].Y - vp->Y + vp->MapY * TileSizeY + vp->OffsetY;
			} else {
				new_vps[i].OffsetX = 0;
				new_vps[i].OffsetY = 0;
			}
		}
	} else {
		for (i = 0; i < num_vps; ++i) {
			new_vps[i].MapX = UI.Viewports[i].MapX;
			new_vps[i].MapY = UI.Viewports[i].MapY;
			new_vps[i].OffsetX = UI.Viewports[i].OffsetX;
			new_vps[i].OffsetY = UI.Viewports[i].OffsetY;
		}
	}

	// Affect the old viewport.
	for (i = 0; i < num_vps; ++i) {
		CViewport *vp;

		vp = UI.Viewports + i;
		vp->X = new_vps[i].X;
		vp->EndX = new_vps[i].EndX;
		vp->Y = new_vps[i].Y;
		vp->EndY = new_vps[i].EndY;
		vp->Set(new_vps[i].MapX, new_vps[i].MapY, new_vps[i].OffsetX, new_vps[i].OffsetY);
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
static void ClipViewport(CViewport *vp, int ClipX, int ClipY)
{
	// begin with maximum possible viewport size
	vp->EndX = vp->X + TheMap.Info.MapWidth * TileSizeX - 1;
	vp->EndY = vp->Y + TheMap.Info.MapHeight * TileSizeY - 1;

	// first clip it to MapArea size if necessary
	if (vp->EndX > ClipX) {
		vp->EndX = ClipX;
	}

	// the same for y
	if (vp->EndY > ClipY) {
		vp->EndY = ClipY;
	}

	Assert(vp->EndX <= UI.MapArea.EndX);
	Assert(vp->EndY <= UI.MapArea.EndY);
}

/**
**  Compute viewport parameters for single viewport mode.
**
**  The parameters include viewport's width and height expressed
**  in pixels, its position with respect to Stratagus's window
**  origin, and the corresponding map parameters expressed in map
**  tiles with origin at map origin (map tile (0,0)).
*/
static void SetViewportModeSingle(void)
{
	CViewport new_vps[MAX_NUM_VIEWPORTS];

	DebugPrint("Single viewport set\n");

	new_vps[0].X = UI.MapArea.X;
	new_vps[0].Y = UI.MapArea.Y;
	ClipViewport(new_vps, UI.MapArea.EndX, UI.MapArea.EndY);

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
static void SetViewportModeSplitHoriz(void)
{
	CViewport new_vps[MAX_NUM_VIEWPORTS];

	DebugPrint("Two horizontal viewports set\n");

	new_vps[0].X = UI.MapArea.X;
	new_vps[0].Y = UI.MapArea.Y;
	ClipViewport(new_vps, UI.MapArea.EndX,
		UI.MapArea.Y + (UI.MapArea.EndY - UI.MapArea.Y + 1) / 2);

	new_vps[1].X = UI.MapArea.X;
	new_vps[1].Y = new_vps[0].EndY + 1;
	ClipViewport(new_vps + 1, UI.MapArea.EndX, UI.MapArea.EndY);

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
static void SetViewportModeSplitHoriz3(void)
{
	CViewport new_vps[MAX_NUM_VIEWPORTS];

	DebugPrint("Horizontal 3-way viewport division set\n");

	new_vps[0].X = UI.MapArea.X;
	new_vps[0].Y = UI.MapArea.Y;
	ClipViewport(new_vps, UI.MapArea.EndX,
		UI.MapArea.Y + (UI.MapArea.EndY - UI.MapArea.Y + 1) / 2);

	new_vps[1].X = UI.MapArea.X;
	new_vps[1].Y = new_vps[0].EndY + 1;
	ClipViewport(new_vps + 1,
		UI.MapArea.X + (UI.MapArea.EndX - UI.MapArea.X + 1) / 2,
		UI.MapArea.EndY);

	new_vps[2].X = new_vps[1].EndX + 1;
	new_vps[2].Y = new_vps[0].EndY + 1;
	ClipViewport(new_vps + 2, UI.MapArea.EndX, UI.MapArea.EndY);

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
static void SetViewportModeSplitVert(void)
{
	CViewport new_vps[MAX_NUM_VIEWPORTS];

	DebugPrint("Two vertical viewports set\n");

	new_vps[0].X = UI.MapArea.X;
	new_vps[0].Y = UI.MapArea.Y;
	ClipViewport(new_vps,
		UI.MapArea.X + (UI.MapArea.EndX - UI.MapArea.X + 1) / 2,
		UI.MapArea.EndY);

	new_vps[1].X = new_vps[0].EndX + 1;
	new_vps[1].Y = UI.MapArea.Y;
	ClipViewport(new_vps + 1, UI.MapArea.EndX, UI.MapArea.EndY);

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
static void SetViewportModeQuad(void)
{
	CViewport new_vps[MAX_NUM_VIEWPORTS];

	DebugPrint("Four viewports set\n");

	new_vps[0].X = UI.MapArea.X;
	new_vps[0].Y = UI.MapArea.Y;
	ClipViewport(new_vps,
		UI.MapArea.X + (UI.MapArea.EndX - UI.MapArea.X + 1) / 2,
		UI.MapArea.Y + (UI.MapArea.EndY - UI.MapArea.Y + 1) / 2);

	new_vps[1].X = new_vps[0].EndX + 1;
	new_vps[1].Y = UI.MapArea.Y;
	ClipViewport(new_vps + 1,
		UI.MapArea.EndX,
		UI.MapArea.Y + (UI.MapArea.EndY - UI.MapArea.Y + 1) / 2);

	new_vps[2].X = UI.MapArea.X;
	new_vps[2].Y = new_vps[0].EndY + 1;
	ClipViewport(new_vps + 2,
		UI.MapArea.X + (UI.MapArea.EndX - UI.MapArea.X + 1) / 2,
		UI.MapArea.EndY);

	new_vps[3].X = new_vps[1].X;
	new_vps[3].Y = new_vps[2].Y;
	ClipViewport(new_vps + 3, UI.MapArea.EndX, UI.MapArea.EndY);

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
	int new_mode;

	new_mode = UI.ViewportMode + step;
	if (new_mode >= NUM_VIEWPORT_MODES) {
		new_mode = 0;
	}
	if (new_mode < 0) {
		new_mode = NUM_VIEWPORT_MODES - 1;
	}
	SetViewportMode((ViewportModeType)new_mode);
}

//@}
