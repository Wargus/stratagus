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
/**@name ui.c - The user interface globals. */
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
char FancyBuildings;                       /// Mirror buildings 1 yes, 0 now.


int SpeedKeyScroll = KEY_SCROLL_SPEED;     /// keyboard scroll speed
int SpeedMouseScroll = MOUSE_SCROLL_SPEED; /// mouse scroll speed

/**
**  The user interface configuration
*/
CUserInterface UI;

/**
**  The available user interfaces.
*/
CUserInterface** UI_Table;

/*----------------------------------------------------------------------------
-- Functions
----------------------------------------------------------------------------*/

static void ClipViewport(Viewport* vp, int ClipX, int ClipY);
static void FinishViewportModeConfiguration(Viewport new_vps[], int num_vps);


/**
**  Initialize the user interface.
**
**  The function looks through ::UI_Table, to find a matching user
**  interface. It uses the race_name and the current video window sizes to
**  find it.
**
**  @param race_name  The race identifier, to select the interface.
*/
void InitUserInterface(const char* race_name)
{
	int i;
	int best;
	int num_vps;
	ViewportModeType vp_mode;
	Viewport vps[MAX_NUM_VIEWPORTS];

	// select the correct slot
	best = 0;
	for (i = 0; UI_Table[i]; ++i) {
		if (!strcmp(race_name, UI_Table[i]->Name)) {
			// perfect
			if (VideoWidth == UI_Table[i]->Width &&
					VideoHeight == UI_Table[i]->Height) {
				best = i;
				break;
			}
			// too big
			if (VideoWidth < UI_Table[i]->Width ||
					VideoHeight < UI_Table[i]->Height) {
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

	UI = *UI_Table[best];

	UI.Offset640X = (VideoWidth - 640) / 2;
	UI.Offset480Y = (VideoHeight - 480) / 2;

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

	UI.CompletedBarColor = VideoMapRGB(TheScreen->format,
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
	MenuPanel* menupanel;

	//
	//  Load graphics
	//
	for (i = 0; i < UI.NumFillers; ++i) {
		UI.Filler[i]->Load();
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
	UI.Point.Cursor = CursorTypeByIdent(UI.Point.Name);
	UI.Glass.Cursor = CursorTypeByIdent(UI.Glass.Name);
	UI.Cross.Cursor = CursorTypeByIdent(UI.Cross.Name);
	UI.YellowHair.Cursor = CursorTypeByIdent(UI.YellowHair.Name);
	UI.GreenHair.Cursor = CursorTypeByIdent(UI.GreenHair.Name);
	UI.RedHair.Cursor = CursorTypeByIdent(UI.RedHair.Name);
	UI.Scroll.Cursor = CursorTypeByIdent(UI.Scroll.Name);

	UI.ArrowE.Cursor = CursorTypeByIdent(UI.ArrowE.Name);
	UI.ArrowNE.Cursor = CursorTypeByIdent(UI.ArrowNE.Name);
	UI.ArrowN.Cursor = CursorTypeByIdent(UI.ArrowN.Name);
	UI.ArrowNW.Cursor = CursorTypeByIdent(UI.ArrowNW.Name);
	UI.ArrowW.Cursor = CursorTypeByIdent(UI.ArrowW.Name);
	UI.ArrowSW.Cursor = CursorTypeByIdent(UI.ArrowSW.Name);
	UI.ArrowS.Cursor = CursorTypeByIdent(UI.ArrowS.Name);
	UI.ArrowSE.Cursor = CursorTypeByIdent(UI.ArrowSE.Name);

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
static void SaveViewports(CLFile* file, const CUserInterface* ui)
{
	int i;
	const Viewport* vp;

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
void SaveUserInterface(CLFile* file)
{
	SaveViewports(file, &UI);
}

/**
**  Clean Condition Panel.
**
**  @param condition condition panel to free.
*/
static void CleanConditionPanel(ConditionPanel* condition)
{
	if (!condition) {
		return;
	}
	free(condition->BoolFlags);
	free(condition->Variables);
	free(condition);
}

/**
**  Clean Condition Panel.
**
**  @param content  content panel to free.
*/
static void CleanContent(ContentType* content)
{
	if (!content) {
		return;
	}
	CleanConditionPanel(content->Condition);
	if (content->DrawData == DrawSimpleText) {
		FreeStringDesc(content->Data.SimpleText.Text);
		free(content->Data.SimpleText.Text);
	} else if (content->DrawData == DrawFormattedText) {
		free(content->Data.FormattedText.Format);
	} else if (content->DrawData == DrawFormattedText2) {
		free(content->Data.FormattedText2.Format);
	}
}

/**
**  Clean Panel.
**
**  @param panel  panel to free.
*/
void CleanPanel(InfoPanel* panel)
{
	int i; // iterator.

	if (!panel) {
		return;
	}
	free(panel->Name);
	CleanConditionPanel(panel->Condition);
	for (i = 0; i < panel->NContents; i++) {
		CleanContent(&panel->Contents[i]);
	}
	free(panel->Contents);
}

/**
**  Clean up a user interface.
*/
void CleanUI(CUserInterface* ui)
{
	int i;
	MenuPanel* menupanel;
	MenuPanel* tmp;

	free(ui->Name);
	free(ui->NormalFontColor);
	free(ui->ReverseFontColor);

	// Filler
	for (i = 0; i < ui->NumFillers; ++i) {
		FreeGraphic(ui->Filler[i]);
	}
	free(ui->FillerX);
	free(ui->FillerY);
	free(ui->Filler);

	// Resource Icons
	for (i = 0; i < MaxCosts + 2; ++i) {
		FreeGraphic(ui->Resources[i].G);
	}

	// Info Panel
	FreeGraphic(ui->InfoPanel.G);
	free(ui->PanelIndex);
	free(ui->SingleSelectedButton);
	free(ui->SelectedButtons);
	free(ui->SingleTrainingButton);
	free(ui->TrainingButtons);
	free(ui->UpgradingButton);
	free(ui->ResearchingButton);
	free(ui->TransportingButtons);

	// Button Panel
	FreeGraphic(ui->ButtonPanel.G);

	// Pie Menu
	FreeGraphic(ui->PieMenuBackgroundG);

	// Buttons
	free(ui->MenuButton.Text);
	free(ui->NetworkMenuButton.Text);
	free(ui->NetworkDiplomacyButton.Text);
	free(ui->ButtonPanel.Buttons);

	// Cursors
	free(ui->Point.Name);
	free(ui->Glass.Name);
	free(ui->Cross.Name);
	free(ui->YellowHair.Name);
	free(ui->GreenHair.Name);
	free(ui->RedHair.Name);
	free(ui->Scroll.Name);
	free(ui->ArrowE.Name);
	free(ui->ArrowNE.Name);
	free(ui->ArrowN.Name);
	free(ui->ArrowNW.Name);
	free(ui->ArrowW.Name);
	free(ui->ArrowSW.Name);
	free(ui->ArrowS.Name);
	free(ui->ArrowSE.Name);

	// Menu Panels
	menupanel = ui->MenuPanels;
	while (menupanel) {
		tmp = menupanel;
		menupanel = menupanel->Next;
		FreeGraphic(tmp->G);
		free(tmp->Ident);
		free(tmp);
	}

	// Backgrounds
	FreeGraphic(ui->VictoryBackgroundG);
	FreeGraphic(ui->DefeatBackgroundG);

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
		free(UI_Table);
		UI_Table = NULL;
	}

	for (i = 0; i < NbAllPanels; i++) {
		CleanPanel(&AllPanels[i]);
	}
	free(AllPanels);
	AllPanels = NULL;
	NbAllPanels = 0;

	// Free Title screen.
	if (TitleScreens) {
		for (i = 0; TitleScreens[i]; ++i) {
			free(TitleScreens[i]->File);
			free(TitleScreens[i]->Music);
			if (TitleScreens[i]->Labels) {
				for (j = 0; TitleScreens[i]->Labels[j]; ++j) {
					free(TitleScreens[i]->Labels[j]->Text);
					free (TitleScreens[i]->Labels[j]);
				}
				free(TitleScreens[i]->Labels);
			}
			free(TitleScreens[i]);
		}
		free(TitleScreens);
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
Viewport* GetViewport(int x, int y)
{
	Viewport* vp;

	for (vp = UI.Viewports; vp < UI.Viewports + UI.NumViewports; ++vp) {
		if (x >= vp->X && x <= vp->EndX && y >= vp->Y && y <= vp->EndY) {
			return vp;
		}
	}
	return NULL;
}

/**
**  Takes coordinates of a map tile and computes the number of the map
**  viewport (if any) inside which the tile is displayed.
**
**  @param tx  x coordinate of the map tile
**  @param ty  y coordinate of the map tile
**
**  @return viewport pointer (index into UI.Viewports) or NULL
**   if this map tile is not displayed in any of
**   the viewports.
**
**  @note If the tile (tx,ty) is currently displayed in more
**  than one viewports (may well happen) this function
**  returns the first one it finds.
*/
Viewport* MapTileGetViewport(int tx, int ty)
{
	Viewport* vp;

	for (vp = UI.Viewports; vp < UI.Viewports + UI.NumViewports; ++vp) {
		if (tx >= vp->MapX && tx < vp->MapX + vp->MapWidth &&
				ty >= vp->MapY && ty < vp->MapY + vp->MapHeight) {
			return vp;
		}
	}
	return NULL;
}

/**
**  Takes an array of new Viewports which are supposed to have their
**  pixel geometry (Viewport::[XY] and Viewport::End[XY]) already
**  computed. Using this information as well as old viewport's
**  parameters fills in new viewports' Viewport::Map* parameters.
**  Then it replaces the old viewports with the new ones and finishes
**  the set-up of the new mode.
**
**  @param new_vps  The array of the new viewports
**  @param num_vps  The number of elements in the new_vps[] array.
*/
static void FinishViewportModeConfiguration(Viewport new_vps[], int num_vps)
{
	int i;

	if (UI.NumViewports < num_vps) {
		//  Compute location of the viewport using oldviewport
		for (i = 0; i < num_vps; ++i) {
			const Viewport* vp;

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
		Viewport* vp;

		vp = UI.Viewports + i;
		vp->X = new_vps[i].X;
		vp->EndX = new_vps[i].EndX;
		vp->Y = new_vps[i].Y;
		vp->EndY = new_vps[i].EndY;
		ViewportSetViewpoint(vp, new_vps[i].MapX, new_vps[i].MapY, new_vps[i].OffsetX, new_vps[i].OffsetY);
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
**  Takes a viewport which is supposed to have its Viewport::[XY]
**  correctly filled-in and computes Viewport::End[XY] attributes
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
static void ClipViewport(Viewport* vp, int ClipX, int ClipY)
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
	Viewport new_vps[MAX_NUM_VIEWPORTS];

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
	Viewport new_vps[MAX_NUM_VIEWPORTS];

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
	Viewport new_vps[MAX_NUM_VIEWPORTS];

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
	Viewport new_vps[MAX_NUM_VIEWPORTS];

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
	Viewport new_vps[MAX_NUM_VIEWPORTS];

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
