//       _________ __                 __                               
//      /   _____//  |_____________ _/  |______     ____  __ __  ______
//      \_____  \\   __\_  __ \__  \\   __\__  \   / ___\|  |  \/  ___/
//      /        \|  |  |  | \// __ \|  |  / __ \_/ /_/  >  |  /\___ \ 
//     /_______  /|__|  |__|  (____  /__| (____  /\___  /|____//____  >
//             \/                  \/          \//_____/            \/ 
//  ______________________                           ______________________
//			  T H E   W A R   B E G I N S
//	   Stratagus - A free fantasy real time strategy game engine
//
/**@name ui.c		-	The user interface globals. */
//
//	(c) Copyright 1999-2003 by Lutz Sammer, Andreas Arens, and
//	                           Jimmy Salmon
//
//      This program is free software; you can redistribute it and/or modify
//      it under the terms of the GNU General Public License as published by
//      the Free Software Foundation; version 2 dated June, 1991.
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
//	$Id$

//@{

/*----------------------------------------------------------------------------
--	Includes
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

local void ClipViewport(Viewport* vp, int ClipX, int ClipY);
local void FinishViewportModeConfiguration(Viewport new_vps[], int num_vps);


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
    int num_vps;
    int vp_mode;
    Viewport vps[MAX_NUM_VIEWPORTS];

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

    num_vps=TheUI.NumViewports;
    vp_mode=TheUI.ViewportMode;
    for( i=0; i<num_vps; ++i ) {
	vps[i].MapX=TheUI.Viewports[i].MapX;
	vps[i].MapY=TheUI.Viewports[i].MapY;
    }

    // FIXME: overwrites already set slots?
    // ARI: Yes, it does :(((
    TheUI=*UI_Table[best];

    TheUI.Offset640X=(VideoWidth-640)/2;
    TheUI.Offset480Y=(VideoHeight-480)/2;

    //
    //	Calculations
    //
    if( TheUI.MapArea.EndX > TheMap.Width*TileSizeX-1 ) {
	TheUI.MapArea.EndX = TheMap.Width*TileSizeX-1;
    }
    if( TheUI.MapArea.EndY > TheMap.Height*TileSizeY-1 ) {
	TheUI.MapArea.EndY = TheMap.Height*TileSizeY-1;
    }

    TheUI.SelectedViewport=TheUI.Viewports;

    if( num_vps ) {
	SetViewportMode(vp_mode);
	for( i=0; i<num_vps; ++i ) {
	    TheUI.Viewports[i].MapX=vps[i].MapX;
	    TheUI.Viewports[i].MapY=vps[i].MapY;
	}
	FinishViewportModeConfiguration(TheUI.Viewports,num_vps);
    } else {
	SetViewportMode(VIEWPORT_SINGLE);
    }

    TheUI.ViewportCursorColor=ColorWhite;
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
    MenuPanel* menupanel;

    //
    //	Load graphics
    //
    for( i=0; i<TheUI.NumFillers; ++i ) {
	if( TheUI.Filler[i].File ) {
	    TheUI.Filler[i].Graphic=LoadGraphic(TheUI.Filler[i].File);
#ifdef USE_OPENGL
	    MakeTexture(TheUI.Filler[i].Graphic,TheUI.Filler[i].Graphic->Width,
		    TheUI.Filler[i].Graphic->Height);
#endif
	}
    }
    if( TheUI.Resource.File ) {
	TheUI.Resource.Graphic=LoadGraphic(TheUI.Resource.File);
#ifdef USE_OPENGL
	MakeTexture(TheUI.Resource.Graphic,TheUI.Resource.Graphic->Width,
		TheUI.Resource.Graphic->Height);
#endif
    }

    for( i=0; i<MaxCosts; ++i ) {
	// FIXME: reuse same graphics?
	if( TheUI.Resources[i].Icon.File ) {
	    TheUI.Resources[i].Icon.Graphic
		    =LoadGraphic(TheUI.Resources[i].Icon.File);
#ifdef USE_OPENGL
	    MakeTexture(TheUI.Resources[i].Icon.Graphic,
		    TheUI.Resources[i].Icon.Graphic->Width,
		    TheUI.Resources[i].Icon.Graphic->Height);
#endif
	}
    }

    // FIXME: reuse same graphics?
    if( TheUI.Resources[FoodCost].Icon.File ) {
	TheUI.Resources[FoodCost].Icon.Graphic=LoadGraphic(TheUI.Resources[FoodCost].Icon.File);
#ifdef USE_OPENGL
	MakeTexture(TheUI.Resources[FoodCost].Icon.Graphic,
		TheUI.Resources[FoodCost].Icon.Graphic->Width,
		TheUI.Resources[FoodCost].Icon.Graphic->Height);
#endif
    }
    // FIXME: reuse same graphics?
    if( TheUI.Resources[ScoreCost].Icon.File ) {
	TheUI.Resources[ScoreCost].Icon.Graphic=LoadGraphic(TheUI.Resources[ScoreCost].Icon.File);
#ifdef USE_OPENGL
	MakeTexture(TheUI.Resources[ScoreCost].Icon.Graphic,
		TheUI.Resources[ScoreCost].Icon.Graphic->Width,
		TheUI.Resources[ScoreCost].Icon.Graphic->Height);
#endif
    }

    if( TheUI.InfoPanel.File ) {
	TheUI.InfoPanel.Graphic=LoadGraphic(TheUI.InfoPanel.File);
#ifdef USE_OPENGL
	MakeTexture(TheUI.InfoPanel.Graphic,
		TheUI.InfoPanel.Graphic->Width,
		TheUI.InfoPanel.Graphic->Height);
#endif
    }
    if( TheUI.ButtonPanel.File ) {
	TheUI.ButtonPanel.Graphic=LoadGraphic(TheUI.ButtonPanel.File);
#ifdef USE_OPENGL
	MakeTexture(TheUI.ButtonPanel.Graphic,
		TheUI.ButtonPanel.Graphic->Width,
		TheUI.ButtonPanel.Graphic->Height);
#endif
    }
    if( TheUI.MenuButtonGraphic.File ) {
	TheUI.MenuButtonGraphic.Graphic=LoadGraphic(TheUI.MenuButtonGraphic.File);
#ifdef USE_OPENGL
	MakeTexture(TheUI.MenuButtonGraphic.Graphic,
		TheUI.MenuButtonGraphic.Graphic->Width,
		TheUI.MenuButtonGraphic.Graphic->Height);
#endif
    }
    if( TheUI.Minimap.File ) {
	TheUI.Minimap.Graphic=LoadGraphic(TheUI.Minimap.File);
#ifdef USE_OPENGL
	MakeTexture(TheUI.Minimap.Graphic,TheUI.Minimap.Graphic->Width,
		TheUI.Minimap.Graphic->Height);
#endif
    }
    if( TheUI.StatusLine.File ) {
	TheUI.StatusLine.Graphic=LoadGraphic(TheUI.StatusLine.File);
#ifdef USE_OPENGL
	MakeTexture(TheUI.StatusLine.Graphic,TheUI.StatusLine.Graphic->Width,
		TheUI.StatusLine.Graphic->Height);
#endif
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

    menupanel=TheUI.MenuPanels;
    while( menupanel ) {
	if( menupanel->Panel.File ) {
	    menupanel->Panel.Graphic=LoadGraphic(menupanel->Panel.File);
#ifdef USE_OPENGL
	    MakeTexture(menupanel->Panel.Graphic,
		    menupanel->Panel.Graphic->Width,
		    menupanel->Panel.Graphic->Height);
#endif
	}
	menupanel=menupanel->Next;
    }
}

/**
**	Save the UI structure.
**
**	@param file	Save file handle
**	@param ui	User interface to save
*/
local void SaveUi(FILE* file,const UI* ui)
{
    int i;
    MenuPanel* menupanel;

    fprintf(file,"(define-ui '%s %d %d\t; Selector",
	    ui->Name,ui->Width,ui->Height);

    fprintf(file,"\n  'normal-font-color '%s"
                 "\n  'reverse-font-color '%s",
	    ui->NormalFontColor, ui->ReverseFontColor);
    fprintf(file,"\n");

    fprintf(file,"\n  'filler (list");
    fprintf(file,"\n    'file \"%s\"",ui->Filler[0].File);
    fprintf(file,"\n    'pos '(%3d %3d)",ui->FillerX[0],ui->FillerY[0]);
    fprintf(file,")\n");

    fprintf(file,"\n  ; Resource line");
    fprintf(file,"\n  (list \"%s\" %d %d)",
	    ui->Resource.File,ui->ResourceX,ui->ResourceY);

    fprintf(file,"\n  'resources (list");
    for( i=1; i<MaxCosts+2; ++i ) {
	if( !ui->Resources[i].Icon.File ) {
	    continue;
	}
	fprintf(file,"\n    '%s",
		i<MaxCosts ? DefaultResourceNames[i] :
		    i==FoodCost ? "food" : "score");
	fprintf(file," (list 'file \"%s\" 'row %d\n"
	             "      'pos '(%d %d) 'size '(%d %d) 'text-pos '(%d %d))",
		ui->Resources[i].Icon.File,ui->Resources[i].IconRow,
		ui->Resources[i].IconX,ui->Resources[i].IconY,
		ui->Resources[i].IconW,ui->Resources[i].IconH,
		ui->Resources[i].TextX,ui->Resources[i].TextY);
    }
    fprintf(file,")\n");

    fprintf(file,"  ; Info panel\n");
    fprintf(file,"  (list \"%s\" %d %d %d %d)\n",
	    ui->InfoPanel.File,
	    ui->InfoPanelX,ui->InfoPanelY,
	    ui->InfoPanelW,ui->InfoPanelH);

    fprintf(file,"\n  'completed-bar '(");
    fprintf(file,"\n    color %d",ui->CompleteBarColor);
    fprintf(file,"\n    pos (%3d %3d)",ui->CompleteBarX,ui->CompleteBarY);
    fprintf(file,"\n    size (%d %d)",ui->CompleteBarW,ui->CompleteBarH);
    fprintf(file,"\n    text \"%s\"",ui->CompleteBarText);
    fprintf(file,"\n    font %s",FontNames[ui->CompleteBarFont]);
    fprintf(file,"\n    text-pos (%3d %3d)",
	    ui->CompleteTextX,ui->CompleteTextY);
    fprintf(file,")\n\n");

    fprintf(file,"  ; Button panel\n");
    fprintf(file,"  (list \"%s\" %d %d)\n",
	    ui->ButtonPanel.File,ui->ButtonPanelX,ui->ButtonPanelY);

    fprintf(file,"\n  'map-area (list");
    fprintf(file,"\n    'pos '(%3d %3d)",
	    ui->MapArea.X, ui->MapArea.Y);
    fprintf(file,"\n    'size '(%d %d)",
	    ui->MapArea.EndX-ui->MapArea.X+1,
	    ui->MapArea.EndY-ui->MapArea.Y+1);
    fprintf(file,")\n\n");

    fprintf(file,"  ; Menu button background\n");
    fprintf(file,"  (list \"%s\" %d %d)\n",
	    ui->MenuButtonGraphic.File,ui->MenuButtonGraphicX,
	    ui->MenuButtonGraphicY);

    fprintf(file,"  ; Minimap background\n");
    fprintf(file,"  (list \"%s\" %d %d)\n",
	    ui->Minimap.File,ui->MinimapX,ui->MinimapY);

    fprintf(file,"\n  'status-line '(");
    fprintf(file,"\n    file \"%s\"",ui->StatusLine.File);
    fprintf(file,"\n    pos (%d %d)",ui->StatusLineX,ui->StatusLineY);
    fprintf(file,"\n    text-pos (%d %d)",
	    ui->StatusLineTextX,ui->StatusLineTextY);
    fprintf(file,"\n    font %s",FontNames[ui->StatusLineFont]);
    fprintf(file,")\n");

    fprintf(file,"\n  'menu-button '(");
    fprintf(file,"\n    pos (%d %d)",
	    ui->MenuButton.X,ui->MenuButton.Y);
    fprintf(file,"\n    size (%d %d)",
	    ui->MenuButton.Width,ui->MenuButton.Height);
    fprintf(file,"\n    caption \"%s\"",
	    ui->MenuButton.Text);
    fprintf(file,"\n    style %s",
	    MenuButtonStyle(ui->MenuButton.Button));
    fprintf(file,")");

    fprintf(file,"\n  'network-menu-button '(");
    fprintf(file,"\n    pos (%d %d)",
	    ui->NetworkMenuButton.X,ui->NetworkMenuButton.Y);
    fprintf(file,"\n    size (%d %d)",
	    ui->NetworkMenuButton.Width,ui->NetworkMenuButton.Height);
    fprintf(file,"\n    caption \"%s\"",
	    ui->NetworkMenuButton.Text);
    fprintf(file,"\n    style %s",
	    MenuButtonStyle(ui->NetworkMenuButton.Button));
    fprintf(file,")");

    fprintf(file,"\n  'network-diplomacy-button '(");
    fprintf(file,"\n    pos (%d %d)",
	    ui->NetworkDiplomacyButton.X,ui->NetworkDiplomacyButton.Y);
    fprintf(file,"\n    size (%d %d)",
	    ui->NetworkDiplomacyButton.Width,ui->NetworkDiplomacyButton.Height);
    fprintf(file,"\n    caption \"%s\"",
	    ui->NetworkDiplomacyButton.Text);
    fprintf(file,"\n    style %s",
	    MenuButtonStyle(ui->NetworkDiplomacyButton.Button));
    fprintf(file,")");

    fprintf(file,"\n\n  'info-buttons '(");
    for( i=0; i<ui->NumInfoButtons; ++i ) {
	fprintf(file,"\n    (pos (%3d %3d) size (%d %d))",
		ui->InfoButtons[i].X,ui->InfoButtons[i].Y,
		ui->InfoButtons[i].Width,ui->InfoButtons[i].Height);
    }
    fprintf(file,")");
    fprintf(file,"\n  'training-buttons '(");
    for( i=0; i<ui->NumTrainingButtons; ++i ) {
	fprintf(file,"\n    (pos (%3d %3d) size (%d %d))",
		ui->TrainingButtons[i].X,ui->TrainingButtons[i].Y,
		ui->TrainingButtons[i].Width,ui->TrainingButtons[i].Height);
    }
    fprintf(file,")");
    fprintf(file,"\n  'button-buttons '(");
    for( i=0; i<ui->NumButtonButtons; ++i ) {
	fprintf(file,"\n    (pos (%3d %3d) size (%d %d))",
		ui->ButtonButtons[i].X,ui->ButtonButtons[i].Y,
		ui->ButtonButtons[i].Width,ui->ButtonButtons[i].Height);
    }
    fprintf(file,")");

    fprintf(file,"\n\n  'cursors '(");
    fprintf(file,"\n    point %s", ui->Point.Name);
    fprintf(file,"\n    glass %s", ui->Glass.Name);
    fprintf(file,"\n    cross %s", ui->Cross.Name);
    fprintf(file,"\n    yellow %s", ui->YellowHair.Name);
    fprintf(file,"\n    green %s", ui->GreenHair.Name);
    fprintf(file,"\n    red %s", ui->RedHair.Name);
    fprintf(file,"\n    scroll %s", ui->Scroll.Name);

    fprintf(file,"\n    arrow-e %s", ui->ArrowE.Name);
    fprintf(file,"\n    arrow-ne %s", ui->ArrowNE.Name);
    fprintf(file,"\n    arrow-n %s", ui->ArrowN.Name);
    fprintf(file,"\n    arrow-nw %s", ui->ArrowNW.Name);
    fprintf(file,"\n    arrow-w %s", ui->ArrowW.Name);
    fprintf(file,"\n    arrow-sw %s", ui->ArrowSW.Name);
    fprintf(file,"\n    arrow-s %s", ui->ArrowS.Name);
    fprintf(file,"\n    arrow-se %s", ui->ArrowSE.Name);
    fprintf(file,")\n");

    fprintf(file,"\n  'menu-panels '(");
    menupanel=ui->MenuPanels;
    while( menupanel ) {
	fprintf(file,"\n    %s \"%s\"",
		menupanel->Ident,menupanel->Panel.File);
	menupanel=menupanel->Next;
    }
    fprintf(file,")\n");

    fprintf(file,"\n  'victory-background \"%s\"",
	    ui->VictoryBackground.File);
    fprintf(file,"\n  'defeat-background \"%s\"",
	    ui->DefeatBackground.File);

    fprintf(file," )\n\n");
}

/**
**	Save the viewports.
**
**	@param file	Save file handle
**	@param ui	User interface to save
*/
local void SaveViewports(FILE* file,const UI* ui)
{
    int i;
    const Viewport* vp;

    fprintf(file, "(define-viewports 'mode %d",ui->ViewportMode);
    for (i = 0; i < ui->NumViewports; ++i) {
	vp = &ui->Viewports[i];
	fprintf(file, "\n  'viewport '(%d %d)",vp->MapX,vp->MapY);
    }
    fprintf(file, ")\n\n");
}

/**
**	Save the user interface module.
**
**	@param file	Save file handle
*/
global void SaveUserInterface(FILE* file)
{
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
    SaveUi(file,&TheUI);
    SaveViewports(file,&TheUI);
}

/**
**	Clean up a user interface.
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
    for( i=0; i<ui->NumFillers; ++i ) {
	free(ui->Filler[i].File);
    }
    free(ui->FillerX);
    free(ui->FillerY);
    free(ui->Filler);

    // Resource
    free(ui->Resource.File);

    // Resource Icons
    for( i=0; i<MaxCosts+2; ++i ) {
	free(ui->Resources[i].Icon.File);
    }

    // Info Panel
    free(ui->InfoPanel.File);

    // Completed Bar
    free(ui->CompleteBarText);

    // Button Panel
    free(ui->ButtonPanel.File);

    // Menu Button
    free(ui->MenuButtonGraphic.File);

    // Minimap
    free(ui->Minimap.File);

    // Status Line
    free(ui->StatusLine.File);

    // Buttons
    free(ui->MenuButton.Text);
    free(ui->NetworkMenuButton.Text);
    free(ui->NetworkDiplomacyButton.Text);
    free(ui->InfoButtons);
    free(ui->TrainingButtons);
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
    menupanel=ui->MenuPanels;
    while( menupanel ) {
	tmp=menupanel;
	menupanel=menupanel->Next;
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
**	Clean up the user interface module.
*/
global void CleanUserInterface(void)
{
    int i;
    MenuPanel* menupanel;

    //
    //	Free the graphics. FIXME: if they are shared this will crash.
    //
    for( i=0; i<TheUI.NumFillers; ++i ) {
	VideoSaveFree(TheUI.Filler[i].Graphic);
    }
    VideoSaveFree(TheUI.Resource.Graphic);

    for( i=0; i<MaxCosts+2; ++i ) {
	VideoSaveFree(TheUI.Resources[i].Icon.Graphic);
    }

    VideoSaveFree(TheUI.InfoPanel.Graphic);
    VideoSaveFree(TheUI.ButtonPanel.Graphic);
    VideoSaveFree(TheUI.MenuButtonGraphic.Graphic);
    VideoSaveFree(TheUI.Minimap.Graphic);
    VideoSaveFree(TheUI.StatusLine.Graphic);

    menupanel=TheUI.MenuPanels;
    while( menupanel ) {
	VideoSaveFree(menupanel->Panel.Graphic);
	menupanel=menupanel->Next;
    }

    //
    //	Free the available user interfaces.
    //
    if( UI_Table ) {
	for( i=0; UI_Table[i]; ++i ) {
	    CleanUI(UI_Table[i]);
	}
	free(UI_Table);
	UI_Table=NULL;
    }

    // FIXME: Johns: Implement this correctly or we will lose memory!
    DebugLevel0Fn("FIXME: not completely written\n");

    memset(&TheUI,0,sizeof(TheUI));
}

/**
**	Takes coordinates of a pixel in stratagus's window and computes
**	the map viewport which contains this pixel.
**
**	@param x	x pixel coordinate with origin at UL corner of screen
**	@param y	y pixel coordinate with origin at UL corner of screen
**
**	@return		viewport pointer or NULL if this pixel is not inside
**			any of the viewports.
**
**	@note	This functions only works with rectangular viewports, when
**		we support shaped map window, this must be rewritten.
*/
global Viewport* GetViewport(int x, int y)
{
    Viewport* vp;

    for (vp = TheUI.Viewports; vp < TheUI.Viewports + TheUI.NumViewports;
	    vp++) {
	if (x >= vp->X && x <= vp->EndX && y >= vp->Y && y <= vp->EndY) {
	    return vp;
	}
    }
    return NULL;
}

/**
**	Takes coordinates of a map tile and computes the number of the map
**	viewport (if any) inside which the tile is displayed.
**
**	@param tx	x coordinate of the map tile
**	@param ty	y coordinate of the map tile
**
**	@return		viewport pointer (index into TheUI.Viewports) or NULL
**			if this map tile is not displayed in any of
**			the viewports.
**
**	@note		If the tile (tx,ty) is currently displayed in more
**			than one viewports (may well happen) this function
**			returns the first one it finds.
*/
global Viewport* MapTileGetViewport(int tx, int ty)
{
    Viewport* vp;

    for (vp = TheUI.Viewports; vp < TheUI.Viewports + TheUI.NumViewports;
		vp++) {
	if (tx >= vp->MapX && tx < vp->MapX + vp->MapWidth
		&& ty >= vp->MapY && ty < vp->MapY + vp->MapHeight) {
	    return vp;
	}
    }
    return NULL;
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
local void FinishViewportModeConfiguration(Viewport new_vps[], int num_vps)
{
    int i;

    // If the number of viewports increases we need to compute what to display
    // in the newly created ones.  We need to do this before we store new
    // geometry information in the TheUI.Viewports field because we use the old
    // geometry information for map origin computation.
    if (TheUI.NumViewports < num_vps) {
	for (i = 0; i < num_vps; i++) {
	    const Viewport* vp;

	    vp = GetViewport(new_vps[i].X, new_vps[i].Y);
	    if (vp) {
		TheUI.Viewports[i].MapX = Viewport2MapX(vp, new_vps[i].X);
		TheUI.Viewports[i].MapY = Viewport2MapY(vp, new_vps[i].Y);
	    } else {
		TheUI.Viewports[i].MapX = 0;
		TheUI.Viewports[i].MapY = 0;
	    }
	}
    }

    for (i = 0; i < num_vps; i++) {
	TheUI.Viewports[i].X = new_vps[i].X;
	TheUI.Viewports[i].EndX = new_vps[i].EndX;
	TheUI.Viewports[i].Y = new_vps[i].Y;
	TheUI.Viewports[i].EndY = new_vps[i].EndY;
	TheUI.Viewports[i].MapWidth =
	    (new_vps[i].EndX - new_vps[i].X + TileSizeX) / TileSizeX;
	TheUI.Viewports[i].MapHeight =
	    (new_vps[i].EndY - new_vps[i].Y + TileSizeY) / TileSizeY;

	if (TheUI.Viewports[i].MapWidth + TheUI.Viewports[i].MapX >
	    TheMap.Width) {
	    TheUI.Viewports[i].MapX -=
		(TheUI.Viewports[i].MapWidth + TheUI.Viewports[i].MapX) -
		TheMap.Width;
	}
	if (TheUI.Viewports[i].MapHeight + TheUI.Viewports[i].MapY >
	    TheMap.Height) {
	    TheUI.Viewports[i].MapY -=
		(TheUI.Viewports[i].MapHeight + TheUI.Viewports[i].MapY) -
		TheMap.Height;
	}
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
**	Takes a viewport which is supposed to have its Viewport::[XY]
**	correctly filled-in and computes Viewport::End[XY] attributes
**	according to clipping information passed in other two arguments.
**
**	@param vp	The viewport.
**	@param ClipX	Maximum x-coordinate of the viewport's right side
**			as dictated by current UI's geometry and ViewportMode.
**	@param ClipY	Maximum y-coordinate of the viewport's bottom side
**			as dictated by current UI's geometry and ViewportMode.
**
**	@note		It is supposed that values passed in Clip[XY] will
**			never be greater than TheUI::MapArea::End[XY].
**			However, they can be smaller according to the place
**			the viewport vp takes in context of current ViewportMode.
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
    // then clip it to the nearest lower TileSize boundary if necessary
    vp->EndX -= (vp->EndX - vp->X + 1) % TileSizeX;

    // the same for y
    if (vp->EndY > ClipY) {
	vp->EndY = ClipY;
    }
    vp->EndY -= (vp->EndY - vp->Y + 1) % TileSizeY;
}

/**
**	Compute viewport parameters for single viewport mode.
**
**	The parameters 	include viewport's width and height expressed
**	in pixels, its position with respect to Stratagus's window
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
**	in pixels, its position with respect to Stratagus's window
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
**	in pixels, its position with respect to Stratagus's window
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
**	in pixels, its position with respect to Stratagus's window
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
**	in pixels, its position with respect to Stratagus's window
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
	    DebugLevel0Fn("trying to set an unknown mode!!\n");
	    break;
    }
}

/**
**	Cycles through predefined viewport modes (geometry configurations)
**	in order defined by the ViewportMode enumerated type.
**
**	@param step	The size of step used for cycling. Values that
**			make sense are mostly 1 (next viewport mode) and
*			-1 (previous viewport mode).
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
