//   ___________		     _________		      _____  __
//   \_	  _____/______   ____   ____ \_   ___ \____________ _/ ____\/  |_
//    |    __) \_  __ \_/ __ \_/ __ \/    \  \/\_  __ \__  \\   __\\   __\ 
//    |     \   |  | \/\  ___/\  ___/\     \____|  | \// __ \|  |   |  |
//    \___  /   |__|    \___  >\___  >\______  /|__|  (____  /__|   |__|
//	  \/		    \/	   \/	     \/		   \/
//  ______________________                           ______________________
//			  T H E   W A R   B E G I N S
//	   FreeCraft - A free fantasy real time strategy game engine
//
/**@name ui.c		-	The user interface globals. */
//
//	(c) Copyright 1999-2002 by Lutz Sammer and Andreas Arens
//
//	FreeCraft is free software; you can redistribute it and/or modify
//	it under the terms of the GNU General Public License as published
//	by the Free Software Foundation; only version 2 of the License.
//
//	FreeCraft is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//	GNU General Public License for more details.
//
//	$Id$

//@{

/*----------------------------------------------------------------------------
--	Includes
----------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "freecraft.h"
#include "interface.h"
#include "map.h"
#include "ui.h"

/*----------------------------------------------------------------------------
--	Variables
----------------------------------------------------------------------------*/

global char RightButtonAttacks;		/// right button 0 move, 1 attack
global char FancyBuildings;		/// Mirror buildings 1 yes, 0 now.

    /// keyboard scroll speed
global int SpeedKeyScroll=KEY_SCROLL_SPEED;
    /// mouse scroll speed
global int SpeedMouseScroll=MOUSE_SCROLL_SPEED;

/**
**	The user interface configuration
*/
global UI TheUI;

/**
**	The available user interfaces.
*/
global UI** UI_Table;

/*----------------------------------------------------------------------------
--	Functions
----------------------------------------------------------------------------*/

/**
**	Initialize the user interface.
**
**	The function looks through ::UI_Table, to find a matching user
**	interface. It uses the race_name and the current video window sizes to
**	find it.
**
**	@param race_name	The race identifier, to select the interface.
*/
global void InitUserInterface(const char *race_name)
{
    int i;
    int best;

    // select the correct slot
    best=0;
    for( i=0; UI_Table[i]; ++i ) {
	if( !strcmp(race_name,UI_Table[i]->Name) ) {
	    // perfect
	    if( VideoWidth==UI_Table[i]->Width
		    && VideoHeight==UI_Table[i]->Height  ) {
		best=i;
		break;
	    }
	    // too big
	    if( VideoWidth<UI_Table[i]->Width
		    || VideoHeight<UI_Table[i]->Height  ) {
		continue;
	    }
	    // best smaller
	    if( UI_Table[i]->Width*UI_Table[i]->Height>=
		    UI_Table[best]->Width*UI_Table[best]->Height  ) {
		best=i;
	    }
	}
    }

    // FIXME: overwrites already set slots?
    // ARI: Yes, it does :(((
    TheUI=*UI_Table[best];

    TheUI.Offset640X = (VideoWidth - 640) / 2;
    TheUI.Offset480Y = (VideoHeight - 480) / 2;

    //
    //	Calculations
    //
#ifdef SPLIT_SCREEN_SUPPORT
    TheUI.LastClickedVP = 0;

    SetViewportMode (VIEWPORT_SINGLE);
#else /* SPLIT_SCREEN_SUPPORT */
    MapWidth=(TheUI.MapEndX-TheUI.MapX+TileSizeX)/TileSizeX;
    MapHeight=(TheUI.MapEndY-TheUI.MapY+TileSizeY)/TileSizeY;
#endif /* SPLIT_SCREEN_SUPPORT */
}

/**
**	Load the user interface graphics.
**
**	@todo	If sub images of the same graphic are used, they are loaded
**		multiple into memory. Use the IconFile code and perhaps build
**		a new layer, which supports image sharing.
*/
global void LoadUserInterface(void)
{
    int i;

    //
    //	Load graphics
    //
    if( TheUI.Filler1.File ) {
	TheUI.Filler1.Graphic=LoadGraphic(TheUI.Filler1.File);
    }
    if( TheUI.Resource.File ) {
	TheUI.Resource.Graphic=LoadGraphic(TheUI.Resource.File);
    }

    for( i=0; i<MaxCosts; ++i ) {
	// FIXME: reuse same graphics?
	if( TheUI.Resources[i].Icon.File ) {
	    TheUI.Resources[i].Icon.Graphic
		    =LoadGraphic(TheUI.Resources[i].Icon.File);
	}
    }

    // FIXME: reuse same graphics?
    if( TheUI.FoodIcon.File ) {
	TheUI.FoodIcon.Graphic=LoadGraphic(TheUI.FoodIcon.File);
    }
    // FIXME: reuse same graphics?
    if( TheUI.ScoreIcon.File ) {
	TheUI.ScoreIcon.Graphic=LoadGraphic(TheUI.ScoreIcon.File);
    }

    if( TheUI.InfoPanel.File ) {
	TheUI.InfoPanel.Graphic=LoadGraphic(TheUI.InfoPanel.File);
    }
    if( TheUI.ButtonPanel.File ) {
	TheUI.ButtonPanel.Graphic=LoadGraphic(TheUI.ButtonPanel.File);
    }
    if( TheUI.MenuButton.File ) {
	TheUI.MenuButton.Graphic=LoadGraphic(TheUI.MenuButton.File);
    }
    if( TheUI.Minimap.File ) {
	TheUI.Minimap.Graphic=LoadGraphic(TheUI.Minimap.File);
    }
    if( TheUI.StatusLine.File ) {
	TheUI.StatusLine.Graphic=LoadGraphic(TheUI.StatusLine.File);
    }

    //
    //	Resolve cursors
    //
    TheUI.Point.Cursor=CursorTypeByIdent(TheUI.Point.Name);
    TheUI.Glass.Cursor=CursorTypeByIdent(TheUI.Glass.Name);
    TheUI.Cross.Cursor=CursorTypeByIdent(TheUI.Cross.Name);
    TheUI.YellowHair.Cursor=CursorTypeByIdent(TheUI.YellowHair.Name);
    TheUI.GreenHair.Cursor=CursorTypeByIdent(TheUI.GreenHair.Name);
    TheUI.RedHair.Cursor=CursorTypeByIdent(TheUI.RedHair.Name);
    TheUI.Scroll.Cursor=CursorTypeByIdent(TheUI.Scroll.Name);

    TheUI.ArrowE.Cursor=CursorTypeByIdent(TheUI.ArrowE.Name);
    TheUI.ArrowNE.Cursor=CursorTypeByIdent(TheUI.ArrowNE.Name);
    TheUI.ArrowN.Cursor=CursorTypeByIdent(TheUI.ArrowN.Name);
    TheUI.ArrowNW.Cursor=CursorTypeByIdent(TheUI.ArrowNW.Name);
    TheUI.ArrowW.Cursor=CursorTypeByIdent(TheUI.ArrowW.Name);
    TheUI.ArrowSW.Cursor=CursorTypeByIdent(TheUI.ArrowSW.Name);
    TheUI.ArrowS.Cursor=CursorTypeByIdent(TheUI.ArrowS.Name);
    TheUI.ArrowSE.Cursor=CursorTypeByIdent(TheUI.ArrowSE.Name);

    if( TheUI.GameMenuePanel.File ) {
	TheUI.GameMenuePanel.Graphic=LoadGraphic(TheUI.GameMenuePanel.File);
    }
    if( TheUI.Menue1Panel.File ) {
	TheUI.Menue1Panel.Graphic=LoadGraphic(TheUI.Menue1Panel.File);
    }
    if( TheUI.Menue2Panel.File ) {
	TheUI.Menue2Panel.Graphic=LoadGraphic(TheUI.Menue2Panel.File);
    }
    if( TheUI.VictoryPanel.File ) {
	TheUI.VictoryPanel.Graphic=LoadGraphic(TheUI.VictoryPanel.File);
    }
    if( TheUI.ScenarioPanel.File ) {
	TheUI.ScenarioPanel.Graphic=LoadGraphic(TheUI.ScenarioPanel.File);
    }
}

/**
**	Save the user interface module.
*/
global void SaveUserInterface(FILE* file)
{
    int i;

    fprintf(file,"\n;;; -----------------------------------------\n");
    fprintf(file,";;; MODULE: ui $Id$\n\n");

    // Contrast, Brightness, Saturation
    fprintf(file,"(set-contrast! %d)\n",TheUI.Contrast);
    fprintf(file,"(set-brightness! %d)\n",TheUI.Brightness);
    fprintf(file,"(set-saturation! %d)\n\n",TheUI.Saturation);
    // Scrolling
    fprintf(file,"(set-mouse-scroll! %s)\n",TheUI.MouseScroll ? "#t" : "#f");
    fprintf(file,"(set-mouse-scroll-speed! %d)\n",SpeedMouseScroll);
    fprintf(file,"(set-key-scroll! %s)\n",TheUI.KeyScroll ? "#t" : "#f");
    fprintf(file,"(set-key-scroll-speed! %d)\n",SpeedKeyScroll);
    fprintf(file,"(set-reverse-map-move! %s)\n\n",
	    TheUI.ReverseMouseMove ? "#t" : "#f");

    fprintf(file,"(set-mouse-adjust! %d)\n",TheUI.MouseAdjust);
    fprintf(file,"(set-mouse-scale! %d)\n\n",TheUI.MouseScale);

    fprintf(file,"(set-original-resources! %s)\n\n",
	    TheUI.OriginalResources ? "#t" : "#f");

    // Save the current UI

    fprintf(file,"(define-ui '%s %d %d\t; Selector\n",
	    TheUI.Name,TheUI.Width,TheUI.Height);
    fprintf(file,"  ; Filler 1\n");
    fprintf(file,"  (list \"%s\" %d %d)\n",
	    TheUI.Filler1.File,TheUI.Filler1X,TheUI.Filler1Y);
    fprintf(file,"  ; Resource line\n");
    fprintf(file,"  (list \"%s\" %d %d)\n",
	    TheUI.Resource.File,TheUI.ResourceX,TheUI.ResourceY);

    for( i=1; i<MaxCosts; ++i ) {
	fprintf(file,"  ; Resource %s\n",DEFAULT_NAMES[i]);
	fprintf(file,"  (list \"%s\" %d\n    %d %d %d %d  %d %d)\n",
		TheUI.Resources[i].Icon.File,TheUI.Resources[i].IconRow,
		TheUI.Resources[i].IconX,TheUI.Resources[i].IconY,
		TheUI.Resources[i].IconW,TheUI.Resources[i].IconH,
		TheUI.Resources[i].TextX,TheUI.Resources[i].TextY);
    }
    fprintf(file,"  ; Food\n");
    fprintf(file,"  (list \"%s\" %d\n    %d %d %d %d  %d %d)\n",
	    TheUI.FoodIcon.File,TheUI.FoodIconRow,
	    TheUI.FoodIconX,TheUI.FoodIconY,
	    TheUI.FoodIconW,TheUI.FoodIconH,
	    TheUI.FoodTextX,TheUI.FoodTextY);
    fprintf(file,"  ; Score\n");
    fprintf(file,"  (list \"%s\" %d\n    %d %d %d %d  %d %d)\n",
	    TheUI.ScoreIcon.File,TheUI.ScoreIconRow,
	    TheUI.ScoreIconX,TheUI.ScoreIconY,
	    TheUI.ScoreIconW,TheUI.ScoreIconH,
	    TheUI.ScoreTextX,TheUI.ScoreTextY);

    fprintf(file,"  ; Info panel\n");
    fprintf(file,"  (list \"%s\" %d %d %d %d)\n",
	    TheUI.InfoPanel.File,
	    TheUI.InfoPanelX,TheUI.InfoPanelY,
	    TheUI.InfoPanelW,TheUI.InfoPanelH);

    fprintf(file,"  ; Complete bar\n");
    fprintf(file,"  (list %d %d %d %d %d)\n",
	    TheUI.CompleteBarColor,
	    TheUI.CompleteBarX,TheUI.CompleteBarY,
	    TheUI.CompleteTextX,TheUI.CompleteTextY);

    fprintf(file,"  ; Button panel\n");
    fprintf(file,"  (list \"%s\" %d %d)\n",
	    TheUI.ButtonPanel.File,TheUI.ButtonPanelX,TheUI.ButtonPanelY);

    fprintf(file,"  ; The map area\n");
    fprintf(file,"  (list %d %d %d %d)\n",
#ifdef SPLIT_SCREEN_SUPPORT
	    TheUI.MapArea.X, TheUI.MapArea.Y,
	    TheUI.MapArea.EndX+1,TheUI.MapArea.EndY+1);
#else /* SPLIT_SCREEN_SUPPORT */
	    TheUI.MapX,TheUI.MapY,TheUI.MapEndX+1,TheUI.MapEndY+1);
#endif /* SPLIT_SCREEN_SUPPORT */

    fprintf(file,"  ; Menu button background\n");
    fprintf(file,"  (list \"%s\" %d %d)\n",
	    TheUI.MenuButton.File,TheUI.MenuButtonX,TheUI.MenuButtonY);

    fprintf(file,"  ; Minimap background\n");
    fprintf(file,"  (list \"%s\" %d %d)\n",
	    TheUI.Minimap.File,TheUI.MinimapX,TheUI.MinimapY);

    fprintf(file,"  ; Status line\n");
    fprintf(file,"  (list \"%s\" %d %d)\n",
	    TheUI.StatusLine.File,TheUI.StatusLineX,TheUI.StatusLineY);

    fprintf(file,"  ; Buttons\n");
    for( i=0; i<MaxButtons; ++i ) {
	fprintf(file,"  (list %3d %3d %4d %3d)\n",
		TheUI.Buttons[i].X,TheUI.Buttons[i].Y,
		TheUI.Buttons[i].Width,TheUI.Buttons[i].Height);
    }

    fprintf(file,"  ; Buttons II\n");
    for( i=0; i<6; ++i ) {
	fprintf(file,"  (list %3d %3d %4d %3d)\n",
		TheUI.Buttons2[i].X,TheUI.Buttons2[i].Y,
		TheUI.Buttons2[i].Width,TheUI.Buttons2[i].Height);
    }

    fprintf(file,"  ; Cursors\n");
    fprintf(file,"  (list");
    fprintf(file," '%s",TheUI.Point.Name);
    fprintf(file," '%s",TheUI.Glass.Name);
    fprintf(file," '%s\n",TheUI.Cross.Name);
    fprintf(file,"    '%s",TheUI.YellowHair.Name);
    fprintf(file," '%s",TheUI.GreenHair.Name);
    fprintf(file,"    '%s\n",TheUI.RedHair.Name);
    fprintf(file,"    '%s\n",TheUI.Scroll.Name);

    fprintf(file,"    '%s",TheUI.ArrowE.Name);
    fprintf(file," '%s",TheUI.ArrowNE.Name);
    fprintf(file," '%s",TheUI.ArrowN.Name);
    fprintf(file," '%s\n",TheUI.ArrowNW.Name);
    fprintf(file,"    '%s",TheUI.ArrowW.Name);
    fprintf(file," '%s",TheUI.ArrowSW.Name);
    fprintf(file," '%s",TheUI.ArrowS.Name);
    fprintf(file," '%s)\n",TheUI.ArrowSE.Name);

    fprintf(file,"  (list \"%s\")\n",TheUI.GameMenuePanel.File);
    fprintf(file,"  (list \"%s\")\n",TheUI.Menue1Panel.File);
    fprintf(file,"  (list \"%s\")\n",TheUI.Menue2Panel.File);
    fprintf(file,"  (list \"%s\")\n",TheUI.VictoryPanel.File);
    fprintf(file,"  (list \"%s\")",TheUI.ScenarioPanel.File);

    fprintf(file," )\n\n");
}

/**
**	Clean up the user interface module.
*/
global void CleanUserInterface(void)
{
    int i;

    //
    //	Free the graphics. FIXME: if they are shared this will crash.
    //
    VideoSaveFree(TheUI.Filler1.Graphic);
    VideoSaveFree(TheUI.Resource.Graphic);

    for( i=0; i<MaxCosts; ++i ) {
	VideoSaveFree(TheUI.Resources[i].Icon.Graphic);
    }

    VideoSaveFree(TheUI.FoodIcon.Graphic);
    VideoSaveFree(TheUI.ScoreIcon.Graphic);
    VideoSaveFree(TheUI.InfoPanel.Graphic);
    VideoSaveFree(TheUI.ButtonPanel.Graphic);
    VideoSaveFree(TheUI.MenuButton.Graphic);
    VideoSaveFree(TheUI.Minimap.Graphic);
    VideoSaveFree(TheUI.StatusLine.Graphic);

    VideoSaveFree(TheUI.GameMenuePanel.Graphic);
    VideoSaveFree(TheUI.Menue1Panel.Graphic);
    VideoSaveFree(TheUI.Menue2Panel.Graphic);
    VideoSaveFree(TheUI.VictoryPanel.Graphic);
    VideoSaveFree(TheUI.ScenarioPanel.Graphic);

#if 0
    //
    //	Free the available user interfaces.
    //
    if( UI_Table ) {
	for( i=0; UI_Table[i]; ++i ) {
	    DebugLevel0Fn("FIXME: not completely written\n");
	}
	free(UI_Table);
	UI_Table=NULL;
    }
#endif

    // FIXME: Johns: Implement this correctly or we will lose memory!
    DebugLevel0Fn("FIXME: not completely written\n");

    memset(&TheUI,0,sizeof(TheUI));
}

#ifdef SPLIT_SCREEN_SUPPORT

/**
**	Takes coordinates of a pixel in freecraft's window and computes
**	the number of the map viewport which contains this pixel.
**
**	@param x	x pixel coordinate with origin at UL corner of FC window
**	@param y	y pixel coordinate with origin at UL corner of FC window
**
**	@return		viewport number (index into TheUI.VP) or -1
**			if this pixel is not inside any of the viewports.
*/
global int GetViewport (int x, int y)
{
    int i;

    for (i=0; i < TheUI.NumViewports; i++) {
	if (x >= TheUI.VP[i].X && x <= TheUI.VP[i].EndX &&
		y >= TheUI.VP[i].Y && y <= TheUI.VP[i].EndY)
	    return i;
    }
    return -1;
}

/**
**	Takes coordinates of a map tile and computes the number of the map
**	viewport (if any) inside which the tile is displayed.
**
**	@param tx	x coordinate of the map tile
**	@param ty	y coordinate of the map tile
**
**	@return		viewport number (index into TheUI.VP) or -1
**			if this map tile is not displayed in any of
**			the viewports.
**
**	@note		If the tile (tx,ty) is currently displayed in more
**			than one viewports (may well happen) this function
**			returns the first one it finds.
*/
global int MapTileGetViewport (int tx, int ty)
{
    int i;

    for (i=0; i < TheUI.NumViewports; i++) {
	Viewport *vp = &TheUI.VP[i];
	if (tx >= vp->MapX && tx < vp->MapX + vp->MapWidth &&
		ty >= vp->MapY && ty < vp->MapY + vp->MapHeight)
	    return i;
    }
    return -1;
}

/**
**	Takes an array of new Viewports which are supposed to have their
**	pixel geometry (Viewport::[XY] and Viewport::End[XY]) already
**	computed. Using this information as well as old viewport's
**	parameters fills in new viewports' Viewport::Map* parameters.
**	Then it replaces the old viewports with the new ones and finishes
**	the set-up of the new mode.
**
**	@param new_vps	The array of the new viewports
**	@param num_vps	The number of elements in the new_vps[] array.
*/
local void FinishViewportModeConfiguration (Viewport new_vps[], int num_vps)
{
    int i, active;

    /* If the number of viewports increases we need to compute what to display
     * in the newly created ones.  We need to do this before we store new
     * geometry information in the TheUI.VP field because we use the old
     * geometry information for map origin computation.
     */
    if (TheUI.NumViewports < num_vps) {
	for (i=0; i < num_vps; i++) {
	    int v = GetViewport (new_vps[i].X, new_vps[i].Y);

	    TheUI.VP[i].MapX = Viewport2MapX (v, new_vps[i].X);
	    TheUI.VP[i].MapY = Viewport2MapY (v, new_vps[i].Y);
	}
    }

    for (i=0; i < num_vps; i++) {
	TheUI.VP[i].X = new_vps[i].X;
	TheUI.VP[i].EndX = new_vps[i].EndX;
	TheUI.VP[i].Y = new_vps[i].Y;
	TheUI.VP[i].EndY = new_vps[i].EndY;
	TheUI.VP[i].MapWidth = (new_vps[i].EndX - new_vps[i].X + TileSizeX) /
		TileSizeX;
	TheUI.VP[i].MapHeight = (new_vps[i].EndY - new_vps[i].Y + TileSizeY) /
		TileSizeY;

	if (TheUI.VP[i].MapWidth + TheUI.VP[i].MapX > TheMap.Width)
	    TheUI.VP[i].MapX -= (TheUI.VP[i].MapWidth + TheUI.VP[i].MapX) -
					TheMap.Width;
	if (TheUI.VP[i].MapHeight + TheUI.VP[i].MapY > TheMap.Height)
	    TheUI.VP[i].MapY -= (TheUI.VP[i].MapHeight + TheUI.VP[i].MapY) -
					TheMap.Height;
    }
    TheUI.NumViewports = num_vps;
    active = GetViewport (CursorX, CursorY);
    if (active != -1)
	TheUI.ActiveViewport = active;
    if (TheUI.LastClickedVP >= TheUI.NumViewports)
	TheUI.LastClickedVP = TheUI.NumViewports - 1;
}

/**
**	Takes a viewport which is supposed to have its Viewport::[XY]
**	correctly filled-in and computes Viewport::End[XY] attributes
**	according to clipping information passed in other two arguments.
**
**	@param v	The viewport.
**	@param ClipX	Maximum x-coordinate of the viewport's right side
**			as dictated by current UI's geometry and ViewportMode.
**	@param ClipY	Maximum y-coordinate of the viewport's bottom side
**			as dictated by current UI's geometry and ViewportMode.
**
**	@note		It is supposed that values passed in Clip[XY] will
**			never be greater than TheUI::MapArea::End[XY].
**			However, they can be smaller according to the place
**			the viewport v takes in context of current ViewportMode.
*/
local void ClipViewport (Viewport *v, int ClipX, int ClipY)
{
    // begin with maximum possible viewport size
    v->EndX = v->X + TheMap.Width * TileSizeX - 1;
    v->EndY = v->Y + TheMap.Height * TileSizeY - 1;

    // first clip it to MapArea size if necessary
    if (v->EndX > ClipX) {
	v->EndX = ClipX;
    }
    // then clip it to the nearest lower TileSize boundary if necessary
    v->EndX -= (v->EndX - v->X + 1) % TileSizeX;

    // the same for y
    if (v->EndY > ClipY) {
	v->EndY = ClipY;
    }
    v->EndY -= (v->EndY - v->Y + 1) % TileSizeY;
}

/**
**	Compute viewport parameters for single viewport mode.
**
**	The parameters 	include viewport's width and height expressed
**	in pixels, its position with respect to FreeCraft's window
**	origin, and the corresponding map parameters expressed in map
**	tiles with origin at map origin (map tile (0,0)).
*/
local void SetViewportModeSingle (void)
{
    Viewport new_vps[MAX_NUM_VIEWPORTS];

    DebugLevel0 ("Single viewport set\n");

    new_vps[0].X = TheUI.MapArea.X;
    new_vps[0].Y = TheUI.MapArea.Y;
    ClipViewport (new_vps, TheUI.MapArea.EndX, TheUI.MapArea.EndY);

    FinishViewportModeConfiguration (new_vps, 1);
}

/**
**	Compute viewport parameters for horizontally split viewport mode.
**	This mode splits the TheUI::MapArea with a horizontal line to
**	2 (approximately) equal parts.
**
**	The parameters 	include viewport's width and height expressed
**	in pixels, its position with respect to FreeCraft's window
**	origin, and the corresponding map parameters expressed in map
**	tiles with origin at map origin (map tile (0,0)).
*/
local void SetViewportModeSplitHoriz (void)
{
    Viewport new_vps[MAX_NUM_VIEWPORTS];

    DebugLevel0 ("Two horizontal viewports set\n");

    new_vps[0].X = TheUI.MapArea.X;
    new_vps[0].Y = TheUI.MapArea.Y;
    ClipViewport (new_vps, TheUI.MapArea.EndX,
		    TheUI.MapArea.Y+(TheUI.MapArea.EndY-TheUI.MapArea.Y+1)/2);

    new_vps[1].X = TheUI.MapArea.X;
    new_vps[1].Y = new_vps[0].EndY + 1;
    ClipViewport (new_vps+1, TheUI.MapArea.EndX, TheUI.MapArea.EndY);

    FinishViewportModeConfiguration (new_vps, 2);
}

/**
**	Compute viewport parameters for horizontal 3-way split viewport mode.
**	This mode splits the TheUI::MapArea with a horizontal line to
**	2 (approximately) equal parts, then splits the bottom part vertically
**	to another 2 parts.
**
**	The parameters 	include viewport's width and height expressed
**	in pixels, its position with respect to FreeCraft's window
**	origin, and the corresponding map parameters expressed in map
**	tiles with origin at map origin (map tile (0,0)).
*/
local void SetViewportModeSplitHoriz3 (void)
{
    Viewport new_vps[MAX_NUM_VIEWPORTS];

    DebugLevel0 ("Horizontal 3-way viewport division set\n");

    new_vps[0].X = TheUI.MapArea.X;
    new_vps[0].Y = TheUI.MapArea.Y;
    ClipViewport (new_vps, TheUI.MapArea.EndX,
		    TheUI.MapArea.Y+(TheUI.MapArea.EndY-TheUI.MapArea.Y+1)/2);

    new_vps[1].X = TheUI.MapArea.X;
    new_vps[1].Y = new_vps[0].EndY + 1;
    ClipViewport (new_vps+1,
		TheUI.MapArea.X +(TheUI.MapArea.EndX-TheUI.MapArea.X+1)/2,
		TheUI.MapArea.EndY);

    new_vps[2].X = new_vps[1].EndX + 1;
    new_vps[2].Y = new_vps[0].EndY + 1;
    ClipViewport (new_vps+2, TheUI.MapArea.EndX, TheUI.MapArea.EndY);

    FinishViewportModeConfiguration (new_vps, 3);
}

/**
**	Compute viewport parameters for vertically split viewport mode.
**	This mode splits the TheUI::MapArea with a vertical line to
**	2 (approximately) equal parts.
**
**	The parameters 	include viewport's width and height expressed
**	in pixels, its position with respect to FreeCraft's window
**	origin, and the corresponding map parameters expressed in map
**	tiles with origin at map origin (map tile (0,0)).
*/
local void SetViewportModeSplitVert (void)
{
    Viewport new_vps[MAX_NUM_VIEWPORTS];

    DebugLevel0 ("Two vertical viewports set\n");

    new_vps[0].X = TheUI.MapArea.X;
    new_vps[0].Y = TheUI.MapArea.Y;
    ClipViewport (new_vps,
		TheUI.MapArea.X +(TheUI.MapArea.EndX-TheUI.MapArea.X+1)/2,
		TheUI.MapArea.EndY);

    new_vps[1].X = new_vps[0].EndX + 1;
    new_vps[1].Y = TheUI.MapArea.Y;
    ClipViewport (new_vps+1, TheUI.MapArea.EndX, TheUI.MapArea.EndY);

    FinishViewportModeConfiguration (new_vps, 2);
}

/**
**	Compute viewport parameters for 4-way split viewport mode.
**	This mode splits the TheUI::MapArea vertically *and* horizontally
**	to 4 (approximately) equal parts.
**
**	The parameters 	include viewport's width and height expressed
**	in pixels, its position with respect to FreeCraft's window
**	origin, and the corresponding map parameters expressed in map
**	tiles with origin at map origin (map tile (0,0)).
*/
local void SetViewportModeQuad (void)
{
    Viewport new_vps[MAX_NUM_VIEWPORTS];

    DebugLevel0 ("Four viewports set\n");

    new_vps[0].X = TheUI.MapArea.X;
    new_vps[0].Y = TheUI.MapArea.Y;
    ClipViewport (new_vps,
		TheUI.MapArea.X +(TheUI.MapArea.EndX-TheUI.MapArea.X+1)/2,
		TheUI.MapArea.Y +(TheUI.MapArea.EndY-TheUI.MapArea.Y+1)/2);

    new_vps[1].X = new_vps[0].EndX + 1;
    new_vps[1].Y = TheUI.MapArea.Y;
    ClipViewport (new_vps+1,
		TheUI.MapArea.EndX,
		TheUI.MapArea.Y +(TheUI.MapArea.EndY-TheUI.MapArea.Y+1)/2);

    new_vps[2].X = TheUI.MapArea.X;
    new_vps[2].Y = new_vps[0].EndY + 1;
    ClipViewport (new_vps+2,
		TheUI.MapArea.X +(TheUI.MapArea.EndX-TheUI.MapArea.X+1)/2,
		TheUI.MapArea.EndY);

    new_vps[3].X = new_vps[1].X;
    new_vps[3].Y = new_vps[2].Y;
    ClipViewport (new_vps+3, TheUI.MapArea.EndX, TheUI.MapArea.EndY);

    FinishViewportModeConfiguration (new_vps, 4);
}

/**
**	Sets up (calls geometry setup routines for) a new viewport mode.
**
**	@param new_mode		New mode's number.
*/
global void SetViewportMode (ViewportMode new_mode)
{
    TheUI.ViewportMode = new_mode;

    switch (TheUI.ViewportMode) {
    case VIEWPORT_SINGLE:
	SetViewportModeSingle ();
	break;
    case VIEWPORT_SPLIT_HORIZ:
	SetViewportModeSplitHoriz ();
	break;
    case VIEWPORT_SPLIT_HORIZ3:
	SetViewportModeSplitHoriz3 ();
	break;
    case VIEWPORT_SPLIT_VERT:
	SetViewportModeSplitVert ();
	break;
    case VIEWPORT_QUAD:
	SetViewportModeQuad ();
	break;
    default:
	DebugLevel0Fn ("trying to set an unknown mode!!\n");
	break;
    }
#if 0
    VideoLockScreen ();
    VideoFillRectangleClip (ColorBlack, TheUI.MapArea.X, TheUI.MapArea.Y,
		TheUI.MapArea.EndX - TheUI.MapArea.X + 1,
		TheUI.MapArea.EndY - TheUI.MapArea.Y + 1);
    VideoUnlockScreen ();
#endif
}

/**
**	Cycles through predefined viewport modes (geometry configurations)
**	in order defined by the ViewportMode enumerated type.
**
**	@param step	The size of step used for cycling. Values that
**			make sense are mostly 1 (next viewport mode) and
*			-1 (previous viewport mode).
*/
global void CycleViewportMode (int step)
{
    int new_mode = (int )TheUI.ViewportMode + step;

    if (new_mode >= NUM_VIEWPORT_MODES)
	new_mode = 0;
    if (new_mode < 0)
	new_mode = NUM_VIEWPORT_MODES - 1;
    SetViewportMode (new_mode);
}

#endif /* SPLIT_SCREEN_SUPPORT */

//@}
