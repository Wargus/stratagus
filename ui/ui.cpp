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
//      (c) Copyright 1999-2004 by Lutz Sammer, Andreas Arens, and
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
--		Includes
----------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "stratagus.h"
#include "video.h"
#include "font.h"
#include "interface.h"
#include "map.h"
#include "ui.h"
#include "menus.h"

/*----------------------------------------------------------------------------
--		Variables
----------------------------------------------------------------------------*/

global char RightButtonAttacks; /// right button 0 move, 1 attack
global char FancyBuildings;     /// Mirror buildings 1 yes, 0 now.

/// keyboard scroll speed
global int SpeedKeyScroll = KEY_SCROLL_SPEED;
/// mouse scroll speed
global int SpeedMouseScroll = MOUSE_SCROLL_SPEED;

/**
**  The user interface configuration
*/
global UI TheUI;

/**
**  The available user interfaces.
*/
global UI** UI_Table;

/*----------------------------------------------------------------------------
--		Functions
----------------------------------------------------------------------------*/

local void ClipViewport(Viewport* vp, int ClipX, int ClipY);
local void FinishViewportModeConfiguration(Viewport new_vps[], int num_vps);


/**
**  Clean the user interface graphics
*/
local void CleanUIGraphics(UI* ui)
{
	MenuPanel* menupanel;
	int i;

	for (i = 0; i < ui->NumFillers; ++i) {
		VideoSafeFree(ui->Filler[i].Graphic);
	}
	VideoSafeFree(ui->Resource.Graphic);

	for (i = 0; i < MaxCosts + 2; ++i) {
		VideoSafeFree(ui->Resources[i].Icon.Graphic);
	}

	VideoSafeFree(ui->InfoPanel.Graphic);
	VideoSafeFree(ui->ButtonPanel.Graphic);
	VideoSafeFree(ui->MenuPanel.Graphic);
	VideoSafeFree(ui->MinimapPanel.Graphic);
	VideoSafeFree(ui->StatusLine.Graphic);
	VideoSafeFree(ui->PieMenuBackground.Graphic);

	menupanel = ui->MenuPanels;
	while (menupanel) {
		VideoSafeFree(menupanel->Panel.Graphic);
		menupanel = menupanel->Next;
	}
}

/**
**  Initialize the user interface.
**
**  The function looks through ::UI_Table, to find a matching user
**  interface. It uses the race_name and the current video window sizes to
**  find it.
**
**  @param race_name  The race identifier, to select the interface.
*/
global void InitUserInterface(const char* race_name)
{
	int i;
	int best;
	int num_vps;
	int vp_mode;
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

	num_vps = TheUI.NumViewports;
	vp_mode = TheUI.ViewportMode;
	for (i = 0; i < num_vps; ++i) {
		vps[i].MapX = TheUI.Viewports[i].MapX;
		vps[i].MapY = TheUI.Viewports[i].MapY;
	}

	CleanUIGraphics(&TheUI);
	TheUI = *UI_Table[best];

	TheUI.Offset640X = (VideoWidth - 640) / 2;
	TheUI.Offset480Y = (VideoHeight - 480) / 2;

	//
	// Calculations
	//
	if (TheUI.MapArea.EndX > TheMap.Width * TileSizeX - 1) {
		TheUI.MapArea.EndX = TheMap.Width * TileSizeX - 1;
	}
	if (TheUI.MapArea.EndY > TheMap.Height * TileSizeY - 1) {
		TheUI.MapArea.EndY = TheMap.Height * TileSizeY - 1;
	}

	TheUI.SelectedViewport = TheUI.Viewports;

	if (num_vps) {
		SetViewportMode(vp_mode);
		for (i = 0; i < num_vps; ++i) {
			TheUI.Viewports[i].MapX = vps[i].MapX;
			TheUI.Viewports[i].MapY = vps[i].MapY;
		}
		FinishViewportModeConfiguration(TheUI.Viewports, num_vps);
	} else {
		SetViewportMode(VIEWPORT_SINGLE);
	}

	TheUI.CompletedBarColor = VideoMapRGB(TheScreen->format,
		TheUI.CompletedBarColorRGB.r,
		TheUI.CompletedBarColorRGB.g,
		TheUI.CompletedBarColorRGB.b);
	TheUI.ViewportCursorColor = ColorWhite;
}

/**
**  Load the user interface graphics.
**
**  @todo  If sub images of the same graphic are used, they are loaded
**  multiple into memory. Use the IconFile code and perhaps build
**  a new layer, which supports image sharing.
*/
global void LoadUserInterface(void)
{
	int i;
	MenuPanel* menupanel;

	//
	//  Load graphics
	//
	for (i = 0; i < TheUI.NumFillers; ++i) {
		if (TheUI.Filler[i].File) {
			TheUI.Filler[i].Graphic = LoadGraphic(TheUI.Filler[i].File);
		}
	}
	if (TheUI.Resource.File && *TheUI.Resource.File) {
		TheUI.Resource.Graphic = LoadGraphic(TheUI.Resource.File);
	}

	for (i = 0; i < MaxCosts; ++i) {
		// FIXME: reuse same graphics?
		if (TheUI.Resources[i].Icon.File) {
			TheUI.Resources[i].Icon.Graphic =
				LoadSprite(TheUI.Resources[i].Icon.File,
					TheUI.Resources[i].IconW, TheUI.Resources[i].IconH);
		}
	}

	// FIXME: reuse same graphics?
	if (TheUI.Resources[FoodCost].Icon.File) {
		TheUI.Resources[FoodCost].Icon.Graphic = LoadGraphic(TheUI.Resources[FoodCost].Icon.File);
	}
	// FIXME: reuse same graphics?
	if (TheUI.Resources[ScoreCost].Icon.File) {
		TheUI.Resources[ScoreCost].Icon.Graphic = LoadGraphic(TheUI.Resources[ScoreCost].Icon.File);
	}

	if (TheUI.InfoPanel.File) {
		TheUI.InfoPanel.Graphic = LoadGraphic(TheUI.InfoPanel.File);
	}
	if (TheUI.ButtonPanel.File) {
		TheUI.ButtonPanel.Graphic = LoadGraphic(TheUI.ButtonPanel.File);
	}
	if (TheUI.PieMenuBackground.File) {
		TheUI.PieMenuBackground.Graphic =
			LoadGraphic(TheUI.PieMenuBackground.File);
	}
	if (TheUI.MenuPanel.File) {
		TheUI.MenuPanel.Graphic = LoadGraphic(TheUI.MenuPanel.File);
	}
	if (TheUI.MinimapPanel.File) {
		TheUI.MinimapPanel.Graphic = LoadGraphic(TheUI.MinimapPanel.File);
	}
	if (TheUI.StatusLine.File) {
		TheUI.StatusLine.Graphic = LoadGraphic(TheUI.StatusLine.File);
	}

	//
	//  Resolve cursors
	//
	TheUI.Point.Cursor = CursorTypeByIdent(TheUI.Point.Name);
	TheUI.Glass.Cursor = CursorTypeByIdent(TheUI.Glass.Name);
	TheUI.Cross.Cursor = CursorTypeByIdent(TheUI.Cross.Name);
	TheUI.YellowHair.Cursor = CursorTypeByIdent(TheUI.YellowHair.Name);
	TheUI.GreenHair.Cursor = CursorTypeByIdent(TheUI.GreenHair.Name);
	TheUI.RedHair.Cursor = CursorTypeByIdent(TheUI.RedHair.Name);
	TheUI.Scroll.Cursor = CursorTypeByIdent(TheUI.Scroll.Name);

	TheUI.ArrowE.Cursor = CursorTypeByIdent(TheUI.ArrowE.Name);
	TheUI.ArrowNE.Cursor = CursorTypeByIdent(TheUI.ArrowNE.Name);
	TheUI.ArrowN.Cursor = CursorTypeByIdent(TheUI.ArrowN.Name);
	TheUI.ArrowNW.Cursor = CursorTypeByIdent(TheUI.ArrowNW.Name);
	TheUI.ArrowW.Cursor = CursorTypeByIdent(TheUI.ArrowW.Name);
	TheUI.ArrowSW.Cursor = CursorTypeByIdent(TheUI.ArrowSW.Name);
	TheUI.ArrowS.Cursor = CursorTypeByIdent(TheUI.ArrowS.Name);
	TheUI.ArrowSE.Cursor = CursorTypeByIdent(TheUI.ArrowSE.Name);

	menupanel = TheUI.MenuPanels;
	while (menupanel) {
		if (menupanel->Panel.File) {
			menupanel->Panel.Graphic = LoadGraphic(menupanel->Panel.File);
		}
		menupanel = menupanel->Next;
	}
}

/**
**  Save the viewports.
**
**  @param file  Save file handle
**  @param ui    User interface to save
*/
local void SaveViewports(CLFile* file, const UI* ui)
{
	int i;
	const Viewport* vp;

	// FIXME: don't save the number
	CLprintf(file, "DefineViewports(\"mode\", %d", ui->ViewportMode);
	for (i = 0; i < ui->NumViewports; ++i) {
		vp = &ui->Viewports[i];
		CLprintf(file, ",\n  \"viewport\", {%d, %d}", vp->MapX, vp->MapY);
	}
	CLprintf(file, ")\n\n");
}

/**
**  Save the user interface module.
**
**  @param file  Save file handle
*/
global void SaveUserInterface(CLFile* file)
{
	SaveViewports(file, &TheUI);
}

/**
**  Clean up a user interface.
*/
global void CleanUI(UI* ui)
{
	int i;
	MenuPanel* menupanel;
	MenuPanel* tmp;

	free(ui->Name);
	free(ui->NormalFontColor);
	free(ui->ReverseFontColor);

	// Filler
	for (i = 0; i < ui->NumFillers; ++i) {
		free(ui->Filler[i].File);
	}
	free(ui->FillerX);
	free(ui->FillerY);
	free(ui->Filler);

	// Resource
	free(ui->Resource.File);
	free(ui->Resource.Graphic);

	// Resource Icons
	for (i = 0; i < MaxCosts + 2; ++i) {
		free(ui->Resources[i].Icon.File);
		free(ui->Resources[i].Icon.Graphic);
	}

	// Info Panel
	free(ui->InfoPanel.File);
	free(ui->SingleSelectedButton);
	free(ui->SingleSelectedText);
	free(ui->SelectedButtons);
	free(ui->SelectedText);
	free(ui->SingleTrainingButton);
	free(ui->SingleTrainingText);
	free(ui->TrainingButtons);
	free(ui->TrainingText);
	free(ui->UpgradingButton);
	free(ui->UpgradingText);
	free(ui->ResearchingButton);
	free(ui->ResearchingText);
	free(ui->TransportingButtons);
	free(ui->TransportingText);
	free(ui->CompletedBarText);

	// Button Panel
	free(ui->ButtonPanel.File);

	// Menu Button
	free(ui->MenuPanel.File);

	// Pie Menu
	free(ui->PieMenuBackground.File);

	// Minimap
	free(ui->MinimapPanel.File);

	// Status Line
	free(ui->StatusLine.File);

	// Buttons
	free(ui->MenuButton.Text);
	free(ui->NetworkMenuButton.Text);
	free(ui->NetworkDiplomacyButton.Text);
	free(ui->ButtonButtons);

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
		free(tmp->Panel.File);
		free(tmp->Ident);
		free(tmp);
	}

	// Backgrounds
	free(ui->VictoryBackground.File);
	free(ui->DefeatBackground.File);

	free(ui);
}

/**
**  Clean up the user interface module.
*/
global void CleanUserInterface(void)
{
	int i;
	int j;

	CleanUIGraphics(&TheUI);

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

	memset(&TheUI, 0, sizeof(TheUI));
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
global Viewport* GetViewport(int x, int y)
{
	Viewport* vp;

	for (vp = TheUI.Viewports; vp < TheUI.Viewports + TheUI.NumViewports; ++vp) {
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
**  @return viewport pointer (index into TheUI.Viewports) or NULL
**   if this map tile is not displayed in any of
**   the viewports.
**
**  @note If the tile (tx,ty) is currently displayed in more
**  than one viewports (may well happen) this function
**  returns the first one it finds.
*/
global Viewport* MapTileGetViewport(int tx, int ty)
{
	Viewport* vp;

	for (vp = TheUI.Viewports; vp < TheUI.Viewports + TheUI.NumViewports; ++vp) {
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
local void FinishViewportModeConfiguration(Viewport new_vps[], int num_vps)
{
	int i;

	if (TheUI.NumViewports < num_vps) {
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
			new_vps[i].MapX = TheUI.Viewports[i].MapX;
			new_vps[i].MapY = TheUI.Viewports[i].MapY;
			new_vps[i].OffsetX = TheUI.Viewports[i].OffsetX;
			new_vps[i].OffsetY = TheUI.Viewports[i].OffsetY;
		}
	}

	// Affect the old viewport.
	for (i = 0; i < num_vps; ++i) {
		Viewport* vp;

		vp = TheUI.Viewports + i;
		vp->X = new_vps[i].X;
		vp->EndX = new_vps[i].EndX;
		vp->Y = new_vps[i].Y;
		vp->EndY = new_vps[i].EndY;
		ViewportSetViewpoint(vp, new_vps[i].MapX, new_vps[i].MapY, new_vps[i].OffsetX, new_vps[i].OffsetY);
	}
	TheUI.NumViewports = num_vps;

	//
	//  Update the viewport pointers
	//
	TheUI.MouseViewport = GetViewport(CursorX, CursorY);
	if (TheUI.SelectedViewport > TheUI.Viewports + TheUI.NumViewports - 1) {
		TheUI.SelectedViewport = TheUI.Viewports + TheUI.NumViewports - 1;
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
**  never be greater than TheUI::MapArea::End[XY].
**  However, they can be smaller according to the place
**  the viewport vp takes in context of current ViewportMode.
*/
local void ClipViewport(Viewport* vp, int ClipX, int ClipY)
{
	// begin with maximum possible viewport size
	vp->EndX = vp->X + TheMap.Width * TileSizeX - 1;
	vp->EndY = vp->Y + TheMap.Height * TileSizeY - 1;

	// first clip it to MapArea size if necessary
	if (vp->EndX > ClipX) {
		vp->EndX = ClipX;
	}

	// the same for y
	if (vp->EndY > ClipY) {
		vp->EndY = ClipY;
	}

	Assert(vp->EndX <= TheUI.MapArea.EndX);
	Assert(vp->EndY <= TheUI.MapArea.EndY);
}

/**
**  Compute viewport parameters for single viewport mode.
**
**  The parameters include viewport's width and height expressed
**  in pixels, its position with respect to Stratagus's window
**  origin, and the corresponding map parameters expressed in map
**  tiles with origin at map origin (map tile (0,0)).
*/
local void SetViewportModeSingle(void)
{
	Viewport new_vps[MAX_NUM_VIEWPORTS];

	DebugPrint("Single viewport set\n");

	new_vps[0].X = TheUI.MapArea.X;
	new_vps[0].Y = TheUI.MapArea.Y;
	ClipViewport(new_vps, TheUI.MapArea.EndX, TheUI.MapArea.EndY);

	FinishViewportModeConfiguration(new_vps, 1);
}

/**
**  Compute viewport parameters for horizontally split viewport mode.
**  This mode splits the TheUI::MapArea with a horizontal line to
**  2 (approximately) equal parts.
**
**  The parameters include viewport's width and height expressed
**  in pixels, its position with respect to Stratagus's window
**  origin, and the corresponding map parameters expressed in map
**  tiles with origin at map origin (map tile (0,0)).
*/
local void SetViewportModeSplitHoriz(void)
{
	Viewport new_vps[MAX_NUM_VIEWPORTS];

	DebugPrint("Two horizontal viewports set\n");

	new_vps[0].X = TheUI.MapArea.X;
	new_vps[0].Y = TheUI.MapArea.Y;
	ClipViewport(new_vps, TheUI.MapArea.EndX,
		TheUI.MapArea.Y + (TheUI.MapArea.EndY - TheUI.MapArea.Y + 1) / 2);

	new_vps[1].X = TheUI.MapArea.X;
	new_vps[1].Y = new_vps[0].EndY + 1;
	ClipViewport(new_vps + 1, TheUI.MapArea.EndX, TheUI.MapArea.EndY);

	FinishViewportModeConfiguration(new_vps, 2);
}

/**
**  Compute viewport parameters for horizontal 3-way split viewport mode.
**  This mode splits the TheUI::MapArea with a horizontal line to
**  2 (approximately) equal parts, then splits the bottom part vertically
**  to another 2 parts.
**
**  The parameters include viewport's width and height expressed
**  in pixels, its position with respect to Stratagus's window
**  origin, and the corresponding map parameters expressed in map
**  tiles with origin at map origin (map tile (0,0)).
*/
local void SetViewportModeSplitHoriz3(void)
{
	Viewport new_vps[MAX_NUM_VIEWPORTS];

	DebugPrint("Horizontal 3-way viewport division set\n");

	new_vps[0].X = TheUI.MapArea.X;
	new_vps[0].Y = TheUI.MapArea.Y;
	ClipViewport(new_vps, TheUI.MapArea.EndX,
		TheUI.MapArea.Y + (TheUI.MapArea.EndY - TheUI.MapArea.Y + 1) / 2);

	new_vps[1].X = TheUI.MapArea.X;
	new_vps[1].Y = new_vps[0].EndY + 1;
	ClipViewport(new_vps + 1,
		TheUI.MapArea.X + (TheUI.MapArea.EndX - TheUI.MapArea.X + 1) / 2,
		TheUI.MapArea.EndY);

	new_vps[2].X = new_vps[1].EndX + 1;
	new_vps[2].Y = new_vps[0].EndY + 1;
	ClipViewport(new_vps + 2, TheUI.MapArea.EndX, TheUI.MapArea.EndY);

	FinishViewportModeConfiguration(new_vps, 3);
}

/**
**  Compute viewport parameters for vertically split viewport mode.
**  This mode splits the TheUI::MapArea with a vertical line to
**  2 (approximately) equal parts.
**
**  The parameters  include viewport's width and height expressed
**  in pixels, its position with respect to Stratagus's window
**  origin, and the corresponding map parameters expressed in map
**  tiles with origin at map origin (map tile (0,0)).
*/
local void SetViewportModeSplitVert(void)
{
	Viewport new_vps[MAX_NUM_VIEWPORTS];

	DebugPrint("Two vertical viewports set\n");

	new_vps[0].X = TheUI.MapArea.X;
	new_vps[0].Y = TheUI.MapArea.Y;
	ClipViewport(new_vps,
		TheUI.MapArea.X + (TheUI.MapArea.EndX - TheUI.MapArea.X + 1) / 2,
		TheUI.MapArea.EndY);

	new_vps[1].X = new_vps[0].EndX + 1;
	new_vps[1].Y = TheUI.MapArea.Y;
	ClipViewport(new_vps + 1, TheUI.MapArea.EndX, TheUI.MapArea.EndY);

	FinishViewportModeConfiguration(new_vps, 2);
}

/**
**  Compute viewport parameters for 4-way split viewport mode.
**  This mode splits the TheUI::MapArea vertically *and* horizontally
**  to 4 (approximately) equal parts.
**
**  The parameters  include viewport's width and height expressed
**  in pixels, its position with respect to Stratagus's window
**  origin, and the corresponding map parameters expressed in map
**  tiles with origin at map origin (map tile (0,0)).
*/
local void SetViewportModeQuad(void)
{
	Viewport new_vps[MAX_NUM_VIEWPORTS];

	DebugPrint("Four viewports set\n");

	new_vps[0].X = TheUI.MapArea.X;
	new_vps[0].Y = TheUI.MapArea.Y;
	ClipViewport(new_vps,
		TheUI.MapArea.X + (TheUI.MapArea.EndX - TheUI.MapArea.X + 1) / 2,
		TheUI.MapArea.Y + (TheUI.MapArea.EndY - TheUI.MapArea.Y + 1) / 2);

	new_vps[1].X = new_vps[0].EndX + 1;
	new_vps[1].Y = TheUI.MapArea.Y;
	ClipViewport(new_vps + 1,
		TheUI.MapArea.EndX,
		TheUI.MapArea.Y + (TheUI.MapArea.EndY - TheUI.MapArea.Y + 1) / 2);

	new_vps[2].X = TheUI.MapArea.X;
	new_vps[2].Y = new_vps[0].EndY + 1;
	ClipViewport(new_vps + 2,
		TheUI.MapArea.X + (TheUI.MapArea.EndX - TheUI.MapArea.X + 1) / 2,
		TheUI.MapArea.EndY);

	new_vps[3].X = new_vps[1].X;
	new_vps[3].Y = new_vps[2].Y;
	ClipViewport(new_vps + 3, TheUI.MapArea.EndX, TheUI.MapArea.EndY);

	FinishViewportModeConfiguration(new_vps, 4);
}

/**
**  Sets up (calls geometry setup routines for) a new viewport mode.
**
**  @param new_mode  New mode's number.
*/
global void SetViewportMode(ViewportMode new_mode)
{
	switch (TheUI.ViewportMode = new_mode) {
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
**  @param step	  The size of step used for cycling. Values that
**               make sense are mostly 1 (next viewport mode) and
**               -1 (previous viewport mode).
*/
global void CycleViewportMode(int step)
{
	int new_mode;

	new_mode = TheUI.ViewportMode + step;
	if (new_mode >= NUM_VIEWPORT_MODES) {
		new_mode = 0;
	}
	if (new_mode < 0) {
		new_mode = NUM_VIEWPORT_MODES - 1;
	}
	SetViewportMode(new_mode);
}

//@}
