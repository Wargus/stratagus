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
/**@name ccl_ui.c	-	The ui ccl functions. */
/*
**	(c) Copyright 1999,2000 by Lutz Sammer
**
**	$Id$
*/

//@{

/*----------------------------------------------------------------------------
--	Includes
----------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "freecraft.h"

#if defined(USE_CCL) || defined(USE_CCL2)	// {

#include "ccl.h"
#include "interface.h"
#include "ui.h"
#include "video.h"
#include "menus.h"
#include "image.h"

/*----------------------------------------------------------------------------
--	Functions
----------------------------------------------------------------------------*/

/**
**	Enable the reverse map move.
*/
local SCM CclReverseMouseMove(void)
{
    TheUI.ReverseMouseMove=1;

    return SCM_UNSPECIFIED;
}

/**
**	Disable the reverse map move.
*/
local SCM CclNoReverseMouseMove(void)
{
    TheUI.ReverseMouseMove=0;

    return SCM_UNSPECIFIED;
}

/**
**	Defines the SVGALIB mouse speed adjust (must be > 0)
*/
local SCM CclMouseAdjust(SCM adjust)
{
    SCM old;

    old=gh_int2scm(TheUI.MouseAdjust);
    TheUI.MouseAdjust=gh_scm2int(adjust);

    return old;
}

/**
**	Defines the SVGALIB mouse scale
*/
local SCM CclMouseScale(SCM scale)
{
    SCM old;

    old=gh_int2scm(TheUI.MouseScale);
    TheUI.MouseScale=gh_scm2int(scale);

    return old;
}

/**
**	Game contrast.
*/
local SCM CclContrast(SCM contrast)
{
    int i;
    SCM old;

    old=gh_int2scm(TheUI.Contrast);
    i=gh_scm2int(contrast);
    if( i<0 || i>400 ) {
	fprintf(stderr,__FUNCTION__": contrast should be 0-400\n");
	i=100;
    }
    TheUI.Contrast=i;
    VideoCreatePalette(GlobalPalette);	// rebuild palette
    MustRedraw=RedrawEverything;

    return old;
}

/**
**	Game brightness.
*/
local SCM CclBrightness(SCM brightness)
{
    int i;
    SCM old;

    old=gh_int2scm(TheUI.Brightness);
    i=gh_scm2int(brightness);
    if( i<-100 || i>100 ) {
	fprintf(stderr,__FUNCTION__": brightness should be -100-100\n");
	i=0;
    }
    TheUI.Brightness=i;
    VideoCreatePalette(GlobalPalette);	// rebuild palette
    MustRedraw=RedrawEverything;

    return old;
}

/**
**	Game saturation.
*/
local SCM CclSaturation(SCM saturation)
{
    int i;
    SCM old;

    old=gh_int2scm(TheUI.Saturation);

    i=gh_scm2int(saturation);
    if( i<-100 || i>200 ) {
	fprintf(stderr,__FUNCTION__": saturation should be -100-200\n");
	i=0;
    }
    TheUI.Saturation=i;
    VideoCreatePalette(GlobalPalette);	// rebuild palette
    MustRedraw=RedrawEverything;

    return old;
}

/**
**	Display a picture.
**
**	@param file	filename of picture.
**
**	@return		Nothing.
*/
local SCM CclDisplayPicture(SCM file)
{
    char* name;

    name=gh_scm2newstr(file,NULL);
    DisplayPicture(name);
    Invalidate();
    free(name);

    return SCM_UNSPECIFIED;
}

/**
**	Process a menu.
**
**	@param id	of menu.
**
**	@return		Nothing.
*/
local SCM CclProcessMenu(SCM id)
{
    int mid;

    mid=gh_scm2int(id);
    if (mid >= 0 && mid <= MENU_MAX)
	ProcessMenu(mid, 1);

    return SCM_UNSPECIFIED;
}

/**
**	Disable resource extension, use original resource display.
*/
local SCM CclOriginalResources(void)
{
    TheUI.OriginalResources=1;
    return SCM_UNSPECIFIED;
}

/**
**	Add to race for user interface
**
**	@param id	look and feel.
*/
local SCM CclRaceAdd(SCM id)
{
    int mid;
    extern int RaceAdd;

    mid=gh_scm2int(id);
    RaceAdd=mid;

    return SCM_UNSPECIFIED;
}

/**
**	Define the look+feel of the user interface.
*/
local SCM CclDefineUI(SCM list)
{
    SCM value;
    SCM temp;
    char* str;
    int	x;
    int	y;
    int i;
    UI* ui;
    void* v;

    //	Get identifier
    value=gh_car(list);
    list=gh_cdr(list);
    str=gh_scm2newstr(value,NULL);
    value=gh_car(list);
    list=gh_cdr(list);
    x=gh_scm2int(value);
    value=gh_car(list);
    list=gh_cdr(list);
    y=gh_scm2int(value);

    // Find slot: new or redefinition
    ui=NULL;
    for( i=0; UI_Table[i]; ++i ) {
	if( UI_Table[i]->Width==x && UI_Table[i]->Height==y
		&& !strcmp(UI_Table[i]->Name,str) ) {
	    ui=UI_Table[i];
	    break;
	}
    }
    if( !ui ) {
	ui=calloc(1,sizeof(UI));
	v=malloc(sizeof(UI*)*(i+2));
	memcpy(v,UI_Table,i*sizeof(UI*));
	CclFree(UI_Table);
	UI_Table=v;
	UI_Table[i]=ui;
	UI_Table[i+1]=NULL;
    }

    CclFree(ui->Name);
    ui->Name=str;
    ui->Width=x;
    ui->Height=y;

    //
    //	Some value defaults
    //
    ui->Contrast=TheUI.Contrast;
    ui->Brightness=TheUI.Brightness;
    ui->Saturation=TheUI.Saturation;

    ui->MouseScroll=TheUI.MouseScroll;
    ui->ReverseMouseMove=TheUI.ReverseMouseMove;

    ui->WarpX=TheUI.WarpX;
    ui->WarpY=TheUI.WarpY;

    ui->MouseAdjust=TheUI.MouseAdjust;
    ui->MouseScale=TheUI.MouseScale;

    ui->OriginalResources=TheUI.OriginalResources;

    //
    //	Now the real values.
    //

    //	Filler 1
    temp=gh_car(list);
    list=gh_cdr(list);

    if( !gh_list_p(temp) ) {
	fprintf(stderr,"list expected\n");
	return SCM_UNSPECIFIED;
    }

    value=gh_car(temp);
    temp=gh_cdr(temp);
    str=gh_scm2newstr(value,NULL);
    value=gh_car(temp);
    temp=gh_cdr(temp);
    x=gh_scm2int(value);
    value=gh_car(temp);
    temp=gh_cdr(temp);
    y=gh_scm2int(value);

    CclFree(ui->Filler1.File);
    ui->Filler1.File=str;
    ui->Filler1X=x;
    ui->Filler1Y=y;

    //	Resource
    temp=gh_car(list);
    list=gh_cdr(list);

    if( !gh_list_p(temp) ) {
	fprintf(stderr,"list expected\n");
	return SCM_UNSPECIFIED;
    }

    value=gh_car(temp);
    temp=gh_cdr(temp);
    str=gh_scm2newstr(value,NULL);
    value=gh_car(temp);
    temp=gh_cdr(temp);
    x=gh_scm2int(value);
    value=gh_car(temp);
    temp=gh_cdr(temp);
    y=gh_scm2int(value);

    CclFree(ui->Resource.File);
    ui->Resource.File=str;
    ui->ResourceX=x;
    ui->ResourceY=y;

    //
    //	Parse icons
    //
    for( i=1; i<MaxCosts; ++i ) {
	// icon
	temp=gh_car(list);
	list=gh_cdr(list);

	if( gh_null_p(temp) ) {
	    CclFree(ui->Resources[i].Icon.File);
	    ui->Resources[i].Icon.File=NULL;
	    ui->Resources[i].Icon.Graphic=NULL;
	    ui->Resources[i].IconRow=0;
	    ui->Resources[i].IconX=0;
	    ui->Resources[i].IconY=0;
	    ui->Resources[i].IconW=0;
	    ui->Resources[i].IconH=0;
	    ui->Resources[i].TextX=0;
	    ui->Resources[i].TextY=0;
	    continue;
	}

	if( !gh_list_p(temp) ) {
	    fprintf(stderr,"list expected\n");
	    return SCM_UNSPECIFIED;
	}

	value=gh_car(temp);
	temp=gh_cdr(temp);
	str=gh_scm2newstr(value,NULL);
	CclFree(ui->Resources[i].Icon.File);
	ui->Resources[i].Icon.File=str;

	value=gh_car(temp);
	temp=gh_cdr(temp);
	x=gh_scm2int(value);
	ui->Resources[i].IconRow=x;

	value=gh_car(temp);
	temp=gh_cdr(temp);
	x=gh_scm2int(value);
	ui->Resources[i].IconX=x;

	value=gh_car(temp);
	temp=gh_cdr(temp);
	y=gh_scm2int(value);
	ui->Resources[i].IconY=y;

	value=gh_car(temp);
	temp=gh_cdr(temp);
	x=gh_scm2int(value);
	ui->Resources[i].IconW=x;

	value=gh_car(temp);
	temp=gh_cdr(temp);
	y=gh_scm2int(value);
	ui->Resources[i].IconH=y;

	value=gh_car(temp);
	temp=gh_cdr(temp);
	x=gh_scm2int(value);
	ui->Resources[i].TextX=x;

	value=gh_car(temp);
	temp=gh_cdr(temp);
	y=gh_scm2int(value);
	ui->Resources[i].TextY=y;
    }

    //	Food icon
    temp=gh_car(list);
    list=gh_cdr(list);

    if( !gh_list_p(temp) ) {
	fprintf(stderr,"list expected\n");
	return SCM_UNSPECIFIED;
    }

    value=gh_car(temp);
    temp=gh_cdr(temp);
    str=gh_scm2newstr(value,NULL);
    CclFree(ui->FoodIcon.File);
    ui->FoodIcon.File=str;

    value=gh_car(temp);
    temp=gh_cdr(temp);
    i=gh_scm2int(value);
    ui->FoodIconRow=i;

    value=gh_car(temp);
    temp=gh_cdr(temp);
    x=gh_scm2int(value);
    ui->FoodIconX=x;

    value=gh_car(temp);
    temp=gh_cdr(temp);
    y=gh_scm2int(value);
    ui->FoodIconY=y;

    value=gh_car(temp);
    temp=gh_cdr(temp);
    x=gh_scm2int(value);
    ui->FoodIconW=x;

    value=gh_car(temp);
    temp=gh_cdr(temp);
    y=gh_scm2int(value);
    ui->FoodIconH=y;

    value=gh_car(temp);
    temp=gh_cdr(temp);
    x=gh_scm2int(value);
    ui->FoodTextX=x;

    value=gh_car(temp);
    temp=gh_cdr(temp);
    y=gh_scm2int(value);
    ui->FoodTextY=y;

    //	Score icon
    temp=gh_car(list);
    list=gh_cdr(list);

    if( !gh_list_p(temp) ) {
	fprintf(stderr,"list expected\n");
	return SCM_UNSPECIFIED;
    }

    value=gh_car(temp);
    temp=gh_cdr(temp);
    str=gh_scm2newstr(value,NULL);
    CclFree(ui->ScoreIcon.File);
    ui->ScoreIcon.File=str;

    value=gh_car(temp);
    temp=gh_cdr(temp);
    i=gh_scm2int(value);
    ui->ScoreIconRow=i;

    value=gh_car(temp);
    temp=gh_cdr(temp);
    x=gh_scm2int(value);
    ui->ScoreIconX=x;

    value=gh_car(temp);
    temp=gh_cdr(temp);
    y=gh_scm2int(value);
    ui->ScoreIconY=y;

    value=gh_car(temp);
    temp=gh_cdr(temp);
    x=gh_scm2int(value);
    ui->ScoreIconW=x;

    value=gh_car(temp);
    temp=gh_cdr(temp);
    y=gh_scm2int(value);
    ui->ScoreIconH=y;

    value=gh_car(temp);
    temp=gh_cdr(temp);
    x=gh_scm2int(value);
    ui->ScoreTextX=x;

    value=gh_car(temp);
    temp=gh_cdr(temp);
    y=gh_scm2int(value);
    ui->ScoreTextY=y;

    //	InfoPanel
    temp=gh_car(list);
    list=gh_cdr(list);

    if( !gh_list_p(temp) ) {
	fprintf(stderr,"list expected\n");
	return SCM_UNSPECIFIED;
    }

    value=gh_car(temp);
    temp=gh_cdr(temp);
    str=gh_scm2newstr(value,NULL);
    value=gh_car(temp);
    temp=gh_cdr(temp);
    x=gh_scm2int(value);
    value=gh_car(temp);
    temp=gh_cdr(temp);
    y=gh_scm2int(value);

    CclFree(ui->InfoPanel.File);
    ui->InfoPanel.File=str;
    ui->InfoPanelX=x;
    ui->InfoPanelY=y;

    value=gh_car(temp);
    temp=gh_cdr(temp);
    x=gh_scm2int(value);
    value=gh_car(temp);
    temp=gh_cdr(temp);
    y=gh_scm2int(value);
    ui->InfoPanelW=x;
    ui->InfoPanelH=y;

    // Completed bar
    temp=gh_car(list);
    list=gh_cdr(list);

    if( !gh_list_p(temp) ) {
	fprintf(stderr,"list expected\n");
	return SCM_UNSPECIFIED;
    }
    value=gh_car(temp);
    temp=gh_cdr(temp);
    i=gh_scm2int(value);
    ui->CompleteBarColor=i;

    value=gh_car(temp);
    temp=gh_cdr(temp);
    x=gh_scm2int(value);
    value=gh_car(temp);
    temp=gh_cdr(temp);
    y=gh_scm2int(value);
    ui->CompleteBarX=x;
    ui->CompleteBarY=y;

    value=gh_car(temp);
    temp=gh_cdr(temp);
    x=gh_scm2int(value);
    value=gh_car(temp);
    temp=gh_cdr(temp);
    y=gh_scm2int(value);
    ui->CompleteTextX=x;
    ui->CompleteTextY=y;

    //	ButtonPanel
    temp=gh_car(list);
    list=gh_cdr(list);

    if( !gh_list_p(temp) ) {
	fprintf(stderr,"list expected\n");
	return SCM_UNSPECIFIED;
    }

    value=gh_car(temp);
    temp=gh_cdr(temp);
    str=gh_scm2newstr(value,NULL);
    value=gh_car(temp);
    temp=gh_cdr(temp);
    x=gh_scm2int(value);
    value=gh_car(temp);
    temp=gh_cdr(temp);
    y=gh_scm2int(value);

    CclFree(ui->ButtonPanel.File);
    ui->ButtonPanel.File=str;
    ui->ButtonPanelX=x;
    ui->ButtonPanelY=y;

    // The map
    temp=gh_car(list);
    list=gh_cdr(list);

    if( !gh_list_p(temp) ) {
	fprintf(stderr,"list expected\n");
	return SCM_UNSPECIFIED;
    }

    value=gh_car(temp);
    temp=gh_cdr(temp);
    x=gh_scm2int(value);
    value=gh_car(temp);
    temp=gh_cdr(temp);
    y=gh_scm2int(value);
    ui->MapX=x;
    ui->MapY=y;
    value=gh_car(temp);
    temp=gh_cdr(temp);
    x=gh_scm2int(value);
    value=gh_car(temp);
    temp=gh_cdr(temp);
    y=gh_scm2int(value);
    ui->MapWidth=x;
    ui->MapHeight=y;

    //	MenuButton
    temp=gh_car(list);
    list=gh_cdr(list);

    if( !gh_list_p(temp) ) {
	fprintf(stderr,"list expected\n");
	return SCM_UNSPECIFIED;
    }

    value=gh_car(temp);
    temp=gh_cdr(temp);
    str=gh_scm2newstr(value,NULL);
    value=gh_car(temp);
    temp=gh_cdr(temp);
    x=gh_scm2int(value);
    value=gh_car(temp);
    temp=gh_cdr(temp);
    y=gh_scm2int(value);

    CclFree(ui->MenuButton.File);
    ui->MenuButton.File=str;
    ui->MenuButtonX=x;
    ui->MenuButtonY=y;

    //	Minimap
    temp=gh_car(list);
    list=gh_cdr(list);

    if( !gh_list_p(temp) ) {
	fprintf(stderr,"list expected\n");
	return SCM_UNSPECIFIED;
    }

    value=gh_car(temp);
    temp=gh_cdr(temp);
    str=gh_scm2newstr(value,NULL);
    value=gh_car(temp);
    temp=gh_cdr(temp);
    x=gh_scm2int(value);
    value=gh_car(temp);
    temp=gh_cdr(temp);
    y=gh_scm2int(value);

    CclFree(ui->Minimap.File);
    ui->Minimap.File=str;
    ui->MinimapX=x;
    ui->MinimapY=y;

    //	StatusLine
    temp=gh_car(list);
    list=gh_cdr(list);

    if( !gh_list_p(temp) ) {
	fprintf(stderr,"list expected\n");
	return SCM_UNSPECIFIED;
    }

    value=gh_car(temp);
    temp=gh_cdr(temp);
    str=gh_scm2newstr(value,NULL);
    value=gh_car(temp);
    temp=gh_cdr(temp);
    x=gh_scm2int(value);
    value=gh_car(temp);
    temp=gh_cdr(temp);
    y=gh_scm2int(value);

    CclFree(ui->StatusLine.File);
    ui->StatusLine.File=str;
    ui->StatusLineX=x;
    ui->StatusLineY=y;

    // Buttons
    for( i=0; i<MaxButtons; ++i ) {
	temp=gh_car(list);
	list=gh_cdr(list);

	if( !gh_list_p(temp) ) {
	    fprintf(stderr,"list expected\n");
	    return SCM_UNSPECIFIED;
	}
	value=gh_car(temp);
	temp=gh_cdr(temp);
	x=gh_scm2int(value);
	ui->Buttons[i].X=x;

	value=gh_car(temp);
	temp=gh_cdr(temp);
	y=gh_scm2int(value);
	ui->Buttons[i].Y=y;

	value=gh_car(temp);
	temp=gh_cdr(temp);
	x=gh_scm2int(value);
	ui->Buttons[i].Width=x;

	value=gh_car(temp);
	temp=gh_cdr(temp);
	y=gh_scm2int(value);
	ui->Buttons[i].Height=y;
    }
    for( i=0; i<6; ++i ) {
	temp=gh_car(list);
	list=gh_cdr(list);

	if( !gh_list_p(temp) ) {
	    fprintf(stderr,"list expected\n");
	    return SCM_UNSPECIFIED;
	}
	value=gh_car(temp);
	temp=gh_cdr(temp);
	x=gh_scm2int(value);
	ui->Buttons2[i].X=x;

	value=gh_car(temp);
	temp=gh_cdr(temp);
	y=gh_scm2int(value);
	ui->Buttons2[i].Y=y;

	value=gh_car(temp);
	temp=gh_cdr(temp);
	x=gh_scm2int(value);
	ui->Buttons2[i].Width=x;

	value=gh_car(temp);
	temp=gh_cdr(temp);
	y=gh_scm2int(value);
	ui->Buttons2[i].Height=y;
    }
    
    return SCM_UNSPECIFIED;
}

/**
 **	Set Speed of Mouse Scrolling
 */
local SCM CclMouseScrollSpeed(SCM num)
{
	int speed;
	
	speed=gh_scm2int(num);
	if (speed < 1 || speed > FRAMES_PER_SECOND) {
		SpeedMouseScroll=MOUSE_SCROLL_SPEED;
	} else {
		SpeedMouseScroll=speed;
	}
	return num;
}
/**
 **	Set Speed of Key Scrolling
 */
local SCM CclKeyScrollSpeed(SCM num)
{
	int speed;
	
	speed=gh_scm2int(num);
	if (speed < 1 || speed > FRAMES_PER_SECOND) {
		SpeedKeyScroll=KEY_SCROLL_SPEED;
	} else {
		SpeedKeyScroll=speed;
	}
	return num;
}

/**
**	Enable display of command keys in panels.
*/
local SCM CclShowCommandKey(void)
{
    ShowCommandKey=1;

    return SCM_UNSPECIFIED;
}

/**
**	Fighter right button attacks as default.
*/
local SCM CclRightButtonAttacks(void)
{
    RightButtonAttacks=1;

    return SCM_UNSPECIFIED;
}

/**
**	Fighter right button moves as default.
*/
local SCM CclRightButtonMoves(void)
{
    RightButtonAttacks=0;

    return SCM_UNSPECIFIED;
}

/**
**	Enable fancy buildings
*/
local SCM CclFancyBuildings(void)
{
    FancyBuildings=1;

    return SCM_UNSPECIFIED;
}

/**
**	Register CCL features for UI.
*/
global void UserInterfaceCclRegister(void)
{
    gh_new_procedure0_0("reverse-map-move",CclReverseMouseMove);
    gh_new_procedure0_0("no-reverse-map-move",CclNoReverseMouseMove);
    gh_new_procedure1_0("mouse-adjust",CclMouseAdjust);
    gh_new_procedure1_0("mouse-scale",CclMouseScale);

    gh_new_procedure1_0("contrast",CclContrast);
    gh_new_procedure1_0("brightness",CclBrightness);
    gh_new_procedure1_0("saturation",CclSaturation);

    gh_new_procedure1_0("display-picture",CclDisplayPicture);
    gh_new_procedure1_0("process-menu",CclProcessMenu);

    gh_new_procedure0_0("original-resources",CclOriginalResources);

    gh_new_procedure1_0("race-add",CclRaceAdd);
    gh_new_procedureN("define-ui",CclDefineUI);

    gh_new_procedure1_0("key-scroll-speed", CclKeyScrollSpeed);
    gh_new_procedure1_0("mouse-scroll-speed", CclMouseScrollSpeed);

    gh_new_procedure0_0("show-command-key",CclShowCommandKey);
    gh_new_procedure0_0("right-button-attacks",CclRightButtonAttacks);
    gh_new_procedure0_0("right-button-moves",CclRightButtonMoves);
    gh_new_procedure0_0("fancy-buildings",CclFancyBuildings);
 
}

#endif	// } USE_CCL

//@}
