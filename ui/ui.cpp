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
*/
global void InitUserInterface(const char *RaceName)
{
    int i;
    int best;

    // select the correct slot
    best=0;
    for( i=0; UI_Table[i]; ++i ) {
	if( !strcmp(RaceName,UI_Table[i]->Name) ) {
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
    TheUI=*UI_Table[best];

    //
    //	Calculations
    //
    MapWidth=(TheUI.MapEndX-TheUI.MapX+TileSizeX)/TileSizeX;
    MapHeight=(TheUI.MapEndY-TheUI.MapY+TileSizeY)/TileSizeY;

#ifdef SPLIT_SCREEN_SUPPORT
    TheUI.MapArea.X = TheUI.MapX;
    TheUI.MapArea.EndX = TheUI.MapEndX;
    TheUI.MapArea.Y = TheUI.MapY;
    TheUI.MapArea.EndY = TheUI.MapEndY;

    TheUI.LastClickedVP = 0;

    TheUI.ViewportMode = VIEWPORT_SINGLE;
    SetViewportMode ();
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
	    TheUI.MapX,TheUI.MapY,TheUI.MapEndX+1,TheUI.MapEndY+1);

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
    //	Free the graphics. FIXME: if shared this will crash.
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

local void SetViewportModeSingle (void)
{
    DebugLevel0 ("Single viewport set\n");

    TheUI.NumViewports = 1;

    TheUI.VP[0].X = TheUI.MapArea.X;
    TheUI.VP[0].EndX = TheUI.MapArea.EndX;
    TheUI.VP[0].Y = TheUI.MapArea.Y;
    TheUI.VP[0].EndY = TheUI.MapArea.EndY;

    //TheUI.VP[0].MapX = 0;
    //TheUI.VP[0].MapY = 0;
    TheUI.VP[0].MapWidth = (TheUI.VP[0].EndX - TheUI.VP[0].X + TileSizeX) /
		TileSizeX;
    TheUI.VP[0].MapHeight = (TheUI.VP[0].EndY - TheUI.VP[0].Y + TileSizeY) /
		TileSizeY;

    TheUI.ActiveViewport = 0;
}

local void SetViewportModeSplitHoriz (void)
{
    int i, active;
    Viewport new_vps[MAX_NUM_VIEWPORTS];

    DebugLevel0 ("Two horizontal viewports set\n");

    new_vps[0].X = TheUI.MapArea.X;
    new_vps[0].EndX = TheUI.MapArea.EndX;
    new_vps[0].Y = TheUI.MapArea.Y;
    new_vps[0].EndY = TheUI.MapArea.Y + (TheUI.MapArea.EndY-TheUI.MapArea.Y)/2;

    new_vps[1].X = TheUI.MapArea.X;
    new_vps[1].EndX = TheUI.MapArea.EndX;
    new_vps[1].Y = TheUI.MapArea.Y + (TheUI.MapArea.EndY-TheUI.MapArea.Y)/2 +1;
    new_vps[1].EndY = TheUI.MapArea.EndY;

    if (TheUI.NumViewports < 2) {
	for (i=0; i < 2; i++) {
	    int v = GetViewport (new_vps[i].X, new_vps[i].Y);

	    TheUI.VP[i].MapX = Viewport2MapX (v, new_vps[i].X);
	    TheUI.VP[i].MapY = Viewport2MapY (v, new_vps[i].Y);
	}
    }

    for (i=0; i < 2; i++) {
	TheUI.VP[i].X = new_vps[i].X;
	TheUI.VP[i].EndX = new_vps[i].EndX;
	TheUI.VP[i].Y = new_vps[i].Y;
	TheUI.VP[i].EndY = new_vps[i].EndY;
	TheUI.VP[i].MapWidth = (new_vps[i].EndX - new_vps[i].X + TileSizeX) /
		TileSizeX;
	TheUI.VP[i].MapHeight = (new_vps[i].EndY - new_vps[i].Y + TileSizeY) /
		TileSizeY;

	if (TheUI.VP[i].MapWidth + TheUI.VP[i].MapX > TheMap.Width-1)
	    TheUI.VP[i].MapX -= (TheUI.VP[i].MapWidth + TheUI.VP[i].MapX) -
					(TheMap.Width-1);
	if (TheUI.VP[i].MapHeight + TheUI.VP[i].MapY > TheMap.Height-1)
	    TheUI.VP[i].MapY -= (TheUI.VP[i].MapHeight + TheUI.VP[i].MapY) -
					(TheMap.Height-1);
    }
    TheUI.NumViewports = 2;
    active = GetViewport (CursorX, CursorY);
    if (active != -1)
	TheUI.ActiveViewport = active;
}

local void SetViewportModeSplitHoriz3 (void)
{
    int i, active;
    Viewport new_vps[MAX_NUM_VIEWPORTS];

    DebugLevel0 ("Horizontal 3-way viewport division set\n");

    new_vps[0].X = TheUI.MapArea.X;
    new_vps[0].EndX = TheUI.MapArea.EndX;
    new_vps[0].Y = TheUI.MapArea.Y;
    new_vps[0].EndY = TheUI.MapArea.Y + (TheUI.MapArea.EndY-TheUI.MapArea.Y)/2;

    new_vps[1].X = TheUI.MapArea.X;
    new_vps[1].EndX = TheUI.MapArea.X + (TheUI.MapArea.EndX-TheUI.MapArea.X)/2;
    new_vps[1].Y = TheUI.MapArea.Y + (TheUI.MapArea.EndY-TheUI.MapArea.Y)/2 +1;
    new_vps[1].EndY = TheUI.MapArea.EndY;

    new_vps[2].X = TheUI.MapArea.X + (TheUI.MapArea.EndX-TheUI.MapArea.X)/2 +1;
    new_vps[2].EndX = TheUI.MapArea.EndX;
    new_vps[2].Y = TheUI.MapArea.Y + (TheUI.MapArea.EndY-TheUI.MapArea.Y)/2 +1;
    new_vps[2].EndY = TheUI.MapArea.EndY;

    if (TheUI.NumViewports < 3) {
	for (i=0; i < 3; i++) {
	    int v = GetViewport (new_vps[i].X, new_vps[i].Y);

	    TheUI.VP[i].MapX = Viewport2MapX (v, new_vps[i].X);
	    TheUI.VP[i].MapY = Viewport2MapY (v, new_vps[i].Y);
	}
    }

    for (i=0; i < 3; i++) {
	TheUI.VP[i].X = new_vps[i].X;
	TheUI.VP[i].EndX = new_vps[i].EndX;
	TheUI.VP[i].Y = new_vps[i].Y;
	TheUI.VP[i].EndY = new_vps[i].EndY;
	TheUI.VP[i].MapWidth = (new_vps[i].EndX - new_vps[i].X + TileSizeX) /
		TileSizeX;
	TheUI.VP[i].MapHeight = (new_vps[i].EndY - new_vps[i].Y + TileSizeY) /
		TileSizeY;

	if (TheUI.VP[i].MapWidth + TheUI.VP[i].MapX > TheMap.Width-1)
	    TheUI.VP[i].MapX -= (TheUI.VP[i].MapWidth + TheUI.VP[i].MapX) -
					(TheMap.Width-1);
	if (TheUI.VP[i].MapHeight + TheUI.VP[i].MapY > TheMap.Height-1)
	    TheUI.VP[i].MapY -= (TheUI.VP[i].MapHeight + TheUI.VP[i].MapY) -
					(TheMap.Height-1);
    }
    TheUI.NumViewports = 3;
    active = GetViewport (CursorX, CursorY);
    if (active != -1)
	TheUI.ActiveViewport = active;
}

local void SetViewportModeSplitVert (void)
{
    int i, active;
    Viewport new_vps[MAX_NUM_VIEWPORTS];

    DebugLevel0 ("Two vertical viewports set\n");

    new_vps[0].X = TheUI.MapArea.X;
    new_vps[0].EndX = TheUI.MapArea.X + (TheUI.MapArea.EndX-TheUI.MapArea.X)/2;
    new_vps[0].Y = TheUI.MapArea.Y;
    new_vps[0].EndY = TheUI.MapArea.EndY;

    new_vps[1].X = TheUI.MapArea.X + (TheUI.MapArea.EndX-TheUI.MapArea.X)/2 +1;
    new_vps[1].EndX = TheUI.MapArea.EndX;
    new_vps[1].Y = TheUI.MapArea.Y;
    new_vps[1].EndY = TheUI.MapArea.EndY;

    /* If the number of viewports increases we need to compute what to display
     * in the newly created ones.  We need to do this before we store new
     * geometry information in the TheUI.VP field because we use the old
     * geometry information for map origin computation.
     */
    /* FIXME: much of the following code is the same for all the viewport
     * configurations -> find a way how to share it. */
    if (TheUI.NumViewports < 2) {
	for (i=0; i < 2; i++) {
	    int v = GetViewport (new_vps[i].X, new_vps[i].Y);

	    TheUI.VP[i].MapX = Viewport2MapX (v, new_vps[i].X);
	    TheUI.VP[i].MapY = Viewport2MapY (v, new_vps[i].Y);
	}
    }

    for (i=0; i < 2; i++) {
	TheUI.VP[i].X = new_vps[i].X;
	TheUI.VP[i].EndX = new_vps[i].EndX;
	TheUI.VP[i].Y = new_vps[i].Y;
	TheUI.VP[i].EndY = new_vps[i].EndY;
	TheUI.VP[i].MapWidth = (new_vps[i].EndX - new_vps[i].X + TileSizeX) /
		TileSizeX;
	TheUI.VP[i].MapHeight = (new_vps[i].EndY - new_vps[i].Y + TileSizeY) /
		TileSizeY;

	if (TheUI.VP[i].MapWidth + TheUI.VP[i].MapX > TheMap.Width-1)
	    TheUI.VP[i].MapX -= (TheUI.VP[i].MapWidth + TheUI.VP[i].MapX) -
					(TheMap.Width-1);
	if (TheUI.VP[i].MapHeight + TheUI.VP[i].MapY > TheMap.Height-1)
	    TheUI.VP[i].MapY -= (TheUI.VP[i].MapHeight + TheUI.VP[i].MapY) -
					(TheMap.Height-1);
    }

    //memcpy (TheUI.VP, new_vps, sizeof (TheUI.VP));

    TheUI.NumViewports = 2;
    active = GetViewport (CursorX, CursorY);
    if (active != -1)
	TheUI.ActiveViewport = active;
}

local void SetViewportModeQuad (void)
{
    int i, active;
    Viewport new_vps[MAX_NUM_VIEWPORTS];

    DebugLevel0 ("Four viewports set\n");

    new_vps[0].X = TheUI.MapArea.X;
    new_vps[0].EndX = TheUI.MapArea.X + (TheUI.MapArea.EndX-TheUI.MapArea.X)/2;
    new_vps[0].Y = TheUI.MapArea.Y;
    new_vps[0].EndY = TheUI.MapArea.Y + (TheUI.MapArea.EndY-TheUI.MapArea.Y)/2;

    new_vps[1].X = TheUI.MapArea.X + (TheUI.MapArea.EndX-TheUI.MapArea.X)/2 +1;
    new_vps[1].EndX = TheUI.MapArea.EndX;
    new_vps[1].Y = TheUI.MapArea.Y;
    new_vps[1].EndY = TheUI.MapArea.Y + (TheUI.MapArea.EndY-TheUI.MapArea.Y)/2;

    new_vps[2].X = TheUI.MapArea.X;
    new_vps[2].EndX = TheUI.MapArea.X + (TheUI.MapArea.EndX-TheUI.MapArea.X)/2;
    new_vps[2].Y = TheUI.MapArea.Y + (TheUI.MapArea.EndY-TheUI.MapArea.Y)/2 +1;
    new_vps[2].EndY = TheUI.MapArea.EndY;

    new_vps[3].X = TheUI.MapArea.X + (TheUI.MapArea.EndX-TheUI.MapArea.X)/2 +1;
    new_vps[3].EndX = TheUI.MapArea.EndX;
    new_vps[3].Y = TheUI.MapArea.Y + (TheUI.MapArea.EndY-TheUI.MapArea.Y)/2 +1;
    new_vps[3].EndY = TheUI.MapArea.EndY;

    if (TheUI.NumViewports < 4) {
	for (i=0; i < 4; i++) {
	    int v = GetViewport (new_vps[i].X, new_vps[i].Y);

	    TheUI.VP[i].MapX = Viewport2MapX (v, new_vps[i].X);
	    TheUI.VP[i].MapY = Viewport2MapY (v, new_vps[i].Y);
	}
    }

    for (i=0; i < 4; i++) {
	TheUI.VP[i].X = new_vps[i].X;
	TheUI.VP[i].EndX = new_vps[i].EndX;
	TheUI.VP[i].Y = new_vps[i].Y;
	TheUI.VP[i].EndY = new_vps[i].EndY;
	TheUI.VP[i].MapWidth = (new_vps[i].EndX - new_vps[i].X + TileSizeX) /
		TileSizeX;
	TheUI.VP[i].MapHeight = (new_vps[i].EndY - new_vps[i].Y + TileSizeY) /
		TileSizeY;

	if (TheUI.VP[i].MapWidth + TheUI.VP[i].MapX > TheMap.Width-1)
	    TheUI.VP[i].MapX -= (TheUI.VP[i].MapWidth + TheUI.VP[i].MapX) -
					(TheMap.Width-1);
	if (TheUI.VP[i].MapHeight + TheUI.VP[i].MapY > TheMap.Height-1)
	    TheUI.VP[i].MapY -= (TheUI.VP[i].MapHeight + TheUI.VP[i].MapY) -
					(TheMap.Height-1);
    }

    TheUI.NumViewports = 4;
    active = GetViewport (CursorX, CursorY);
    if (active != -1)
	TheUI.ActiveViewport = active;
}

global void SetViewportMode (void)
{
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
}

global void CycleViewportMode (int direction)
{
    TheUI.ViewportMode = (TheUI.ViewportMode + direction) % NUM_VIEWPORT_MODES;
    SetViewportMode ();
}

#endif /* SPLIT_SCREEN_SUPPORT */

//@}
