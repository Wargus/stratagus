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
//
//	(c) Copyright 1999-2002 by Lutz Sammer
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
#include "ccl.h"
#include "interface.h"
#include "ui.h"
#include "video.h"
#include "map.h"
#include "menus.h"
#include "font.h"
#include "etlib/hash.h"

/*----------------------------------------------------------------------------
--	Functions
----------------------------------------------------------------------------*/

/**
**	Enable/disable the global color cycling.
**
**	@param flag	True = turn on, false = off.
**	@return		The old state of color cylce all.
*/
local SCM CclSetColorCycleAll(SCM flag)
{
    int old;

    old=ColorCycleAll;
    ColorCycleAll=gh_scm2bool(flag);

    return gh_bool2scm(old);
}

/**
**	Enable/disable the reverse map move.
**
**	@param flag	True = turn on, false = off.
**	@return		The old state of scrolling.
*/
local SCM CclSetReverseMapMove(SCM flag)
{
    int old;

    old=TheUI.ReverseMouseMove;
    TheUI.ReverseMouseMove=gh_scm2bool(flag);

    return gh_bool2scm(old);
}

/**
**	Defines the SVGALIB mouse speed adjust (must be > 0)
**
**	@param adjust	mouse adjust for SVGALIB
**	@return		old value
*/
local SCM CclSetMouseAdjust(SCM adjust)
{
    SCM old;
    int i;

    old=gh_int2scm(TheUI.MouseAdjust);
    i=gh_scm2int(adjust);
    if( i>0 ) {
	TheUI.MouseAdjust=i;
    }

    return old;
}

/**
**	Defines the SVGALIB mouse scale
**
**	@param scale	mouse scale for SVGALIB
**	@return		old value
*/
local SCM CclSetMouseScale(SCM scale)
{
    SCM old;

    old=gh_int2scm(TheUI.MouseScale);
    TheUI.MouseScale=gh_scm2int(scale);

    return old;
}

/**
**	Game contrast.
**
**	@param contrast	New contrast 0 - 400.
**	@return		Old contrast.
*/
local SCM CclSetContrast(SCM contrast)
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
**
**	@param brightness	New brightness -100 - 100.
**	@return			Old brightness.
*/
local SCM CclSetBrightness(SCM brightness)
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
**
**	@param saturation	New saturation -100 - 200.
**	@return			Old saturation.
*/
local SCM CclSetSaturation(SCM saturation)
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
**	Default title-screen.
**
**	@param title	SCM title. (nil reports only)
**
**	@return		Old title screen.
*/
local SCM CclSetTitleScreen(SCM title)
{
    SCM old;

    old=NIL;
    if( TitleScreen ) {
	old=gh_str02scm(TitleScreen);
    }
    if( !gh_null_p(title) ) {
	if( TitleScreen ) {
	    free(TitleScreen);
	    TitleScreen=NULL;
	}

	TitleScreen=gh_scm2newstr(title,NULL);
    }
    return old;
}

/**
**	Default title music.
**
**	@param music	title music. (nil reports only)
**
**	@return		Old title music.
*/
local SCM CclSetTitleMusic(SCM music)
{
    SCM old;

    old=NIL;
    if( TitleMusic ) {
	old=gh_str02scm(TitleMusic);
    }
    if( !gh_null_p(music) ) {
	if( TitleMusic ) {
	    free(TitleMusic);
	    TitleMusic=NULL;
	}

	TitleMusic=gh_scm2newstr(music,NULL);
    }
    return old;
}

/**
**	Default menu background.
**
**	@param background	background. (nil reports only)
**
**	@return		Old menu background.
*/
local SCM CclSetMenuBackground(SCM background)
{
    SCM old;

    old=NIL;
    if( MenuBackground ) {
	old=gh_str02scm(MenuBackground);
    }
    if( !gh_null_p(background) ) {
	if( MenuBackground ) {
	    free(MenuBackground);
	    MenuBackground=NULL;
	}

	MenuBackground=gh_scm2newstr(background,NULL);
    }
    return old;
}

/**
**	Default menu background with title.
**
**	@param background	background. (nil reports only)
**
**	@return		Old menu background.
*/
local SCM CclSetMenuBackgroundWithTitle(SCM background)
{
    SCM old;

    old=NIL;
    if( MenuBackgroundWithTitle ) {
	old=gh_str02scm(MenuBackgroundWithTitle);
    }
    if( !gh_null_p(background) ) {
	if( MenuBackgroundWithTitle ) {
	    free(MenuBackgroundWithTitle);
	    MenuBackgroundWithTitle=NULL;
	}

	MenuBackgroundWithTitle=gh_scm2newstr(background,NULL);
    }
    return old;
}

/**
**	Default menu music.
**
**	@param music	menu music. (nil reports only)
**
**	@return		Old menu music.
*/
local SCM CclSetMenuMusic(SCM music)
{
    SCM old;

    old=NIL;
    if( MenuMusic ) {
	old=gh_str02scm(MenuMusic);
    }
    if( !gh_null_p(music) ) {
	if( MenuMusic ) {
	    free(MenuMusic);
	    MenuMusic=NULL;
	}

	MenuMusic=gh_scm2newstr(music,NULL);
    }
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
    SetClipping(0,0,VideoWidth-1,VideoHeight-1);
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
    char *mid;

    mid = gh_scm2newstr(id, NULL);
    if (hash_find(MenuHash, mid)) {
	ProcessMenu(mid, 1);
    }
    free(mid);

    return SCM_UNSPECIFIED;
}

/**
**	Enable/disable resource extension, use original resource display.
**
**	@param flag	True = turn on, false = off.
**	@return		The old state of scrolling.
*/
local SCM CclSetOriginalResources(SCM flag)
{
    int old;

    old=TheUI.OriginalResources;
    TheUI.OriginalResources=gh_scm2bool(flag);

    return gh_bool2scm(old);
}

/**
**	Define a cursor.
**
**	FIXME: need some general data structure to make this parsing easier.
*/
local SCM CclDefineCursor(SCM list)
{
    SCM value;
    char* s1;
    char* s2;
    int i;
    CursorType* ct;

    //	Get identifier
    value=gh_car(list);
    list=gh_cdr(list);
    s1=gh_scm2newstr(value,NULL);
    value=gh_car(list);
    list=gh_cdr(list);
    s2=gh_scm2newstr(value,NULL);
    if( !strcmp(s2,"any") ) {
	free(s2);
	s2=NULL;
    }

    //
    //	Look if this kind of cursor already exists.
    //
    ct=NULL;
    i=0;
    if( Cursors ) {
	for( ; Cursors[i].OType; ++i ) {
	    //
	    //	Race not same, not found.
	    //
	    if( Cursors[i].Race && s2 ) {
		if( strcmp(Cursors[i].Race,s2) ) {
		    continue;
		}
	    } else if( Cursors[i].Race!=s2 ) {
		continue;
	    }
	    if( !strcmp(Cursors[i].Ident,s1) ) {
		ct=&Cursors[i];
		break;
	    }
	}
    }
    //
    //	Not found, make a new slot.
    //
    if( ct ) {
	free(s1);
	free(s2);
    } else {
	ct=calloc(i+2,sizeof(CursorType));
	memcpy(ct,Cursors,sizeof(CursorType)*i);
	free(Cursors);
	Cursors=ct;
	ct=&Cursors[i];
	ct->OType=CursorTypeType;
	ct->Ident=s1;
	ct->Race=s2;
    }

    //
    //	Parse the arguments, already the new tagged format.
    //
    while( !gh_null_p(list) ) {
	value=gh_car(list);
	list=gh_cdr(list);
	if( gh_eq_p(value,gh_symbol2scm("image")) ) {
	    free(ct->File);
	    ct->File=gh_scm2newstr(gh_car(list),NULL);
	} else if( gh_eq_p(value,gh_symbol2scm("hot-spot")) ) {
	    value=gh_car(list);
	    ct->HotX=gh_scm2int(gh_car(value));
	    value=gh_cdr(value);
	    ct->HotY=gh_scm2int(gh_car(value));
	} else if( gh_eq_p(value,gh_symbol2scm("size")) ) {
	    value=gh_car(list);
	    ct->Width=gh_scm2int(gh_car(value));
	    value=gh_cdr(value);
	    ct->Height=gh_scm2int(gh_car(value));
	} else {
	    s1=gh_scm2newstr(value,NULL);
	    fprintf(stderr,"Unsupported tag %s\n",s1);
	    free(s1);
	}
	list=gh_cdr(list);
    }

    return SCM_UNSPECIFIED;
}

/**
**	Set the current game cursor.
**
**	@param ident	Cursor identifier.
*/
local SCM CclSetGameCursor(SCM ident)
{
    char* str;

    str=gh_scm2newstr(ident,NULL);
    GameCursor=CursorTypeByIdent(str);
    free(str);

    return SCM_UNSPECIFIED;
}

/**
**	Define the look+feel of the user interface.
**
**	FIXME: need some general data structure to make this parsing easier.
**	FIXME: use the new tagged config format.
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
    i=0;
    if( UI_Table ) {
	for( ; UI_Table[i]; ++i ) {
	    if( UI_Table[i]->Width==x && UI_Table[i]->Height==y
		    && !strcmp(UI_Table[i]->Name,str) ) {
		ui=UI_Table[i];
		break;
	    }
	}
    }
    if( !ui ) {
	ui=calloc(1,sizeof(UI));
	v=malloc(sizeof(UI*)*(i+2));
	memcpy(v,UI_Table,i*sizeof(UI*));
	free(UI_Table);
	UI_Table=v;
	UI_Table[i]=ui;
	UI_Table[i+1]=NULL;
    }

    free(ui->Name);
    ui->Name=str;
    ui->Width=x;
    ui->Height=y;

    //
    //	Some value defaults
    //

    // This save the setup values FIXME: They are set by CCL.

    ui->Contrast=TheUI.Contrast;
    ui->Brightness=TheUI.Brightness;
    ui->Saturation=TheUI.Saturation;

    ui->MouseScroll=TheUI.MouseScroll;
    ui->KeyScroll=TheUI.KeyScroll;
    ui->ReverseMouseMove=TheUI.ReverseMouseMove;

    ui->WarpX=-1;
    ui->WarpY=-1;

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

    free(ui->Filler1.File);
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

    free(ui->Resource.File);
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
	    free(ui->Resources[i].Icon.File);
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
	free(ui->Resources[i].Icon.File);
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
    free(ui->FoodIcon.File);
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
    free(ui->ScoreIcon.File);
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

    free(ui->InfoPanel.File);
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

    free(ui->ButtonPanel.File);
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
#ifdef SPLIT_SCREEN_SUPPORT
    ui->MapArea.X=x;
    ui->MapArea.Y=y;
    if ( ui->MapArea.X < 0 || ui->MapArea.Y < 0 ) {
	fprintf(stderr,"map top-left point expected\n");
	return SCM_UNSPECIFIED;
    }
    value=gh_car(temp);
    temp=gh_cdr(temp);
    x=gh_scm2int(value);
    value=gh_car(temp);
    temp=gh_cdr(temp);
    y=gh_scm2int(value);
    //StephanR: note that the bottom-right point is one pixel off
    ui->MapArea.EndX = x-1;
    ui->MapArea.EndY = y-1;
    if ( x < 1 || y < 1 || ui->MapArea.EndX < ui->MapArea.X ||
				ui->MapArea.EndY < ui->MapArea.Y ) {
	fprintf(stderr,"map bottom-right point expected\n");
	return SCM_UNSPECIFIED;
    }
#else /* SPLIT_SCREEN_SUPPORT */
    ui->MapX=x;
    ui->MapY=y;
    if ( ui->MapX < 0 || ui->MapY < 0 ) {
	fprintf(stderr,"map top-left point expected\n");
	return SCM_UNSPECIFIED;
    }
    value=gh_car(temp);
    temp=gh_cdr(temp);
    x=gh_scm2int(value);
    value=gh_car(temp);
    temp=gh_cdr(temp);
    y=gh_scm2int(value);
    //StephanR: note that the bottom-right point is one pixel off
    ui->MapEndX=x - 1;
    ui->MapEndY=y - 1;
    if ( x < 1 || y < 1 ||
	    ui->MapEndX < ui->MapX || ui->MapEndY < ui->MapY ) {
	fprintf(stderr,"map bottom-right point expected\n");
	return SCM_UNSPECIFIED;
    }
#endif /* SPLIT_SCREEN_SUPPORT */

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

    free(ui->MenuButton.File);
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

    free(ui->Minimap.File);
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

    free(ui->StatusLine.File);
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

    //
    //	Get the cursors definitions.
    //
    temp=gh_car(list);
    list=gh_cdr(list);

    value=gh_car(temp);
    temp=gh_cdr(temp);
    str=gh_scm2newstr(value,NULL);
    free(ui->Point.Name);
    ui->Point.Name=str;

    value=gh_car(temp);
    temp=gh_cdr(temp);
    str=gh_scm2newstr(value,NULL);
    free(ui->Glass.Name);
    ui->Glass.Name=str;

    value=gh_car(temp);
    temp=gh_cdr(temp);
    str=gh_scm2newstr(value,NULL);
    free(ui->Cross.Name);
    ui->Cross.Name=str;

    value=gh_car(temp);
    temp=gh_cdr(temp);
    str=gh_scm2newstr(value,NULL);
    free(ui->YellowHair.Name);
    ui->YellowHair.Name=str;

    value=gh_car(temp);
    temp=gh_cdr(temp);
    str=gh_scm2newstr(value,NULL);
    free(ui->GreenHair.Name);
    ui->GreenHair.Name=str;

    value=gh_car(temp);
    temp=gh_cdr(temp);
    str=gh_scm2newstr(value,NULL);
    free(ui->RedHair.Name);
    ui->RedHair.Name=str;

    value=gh_car(temp);
    temp=gh_cdr(temp);
    str=gh_scm2newstr(value,NULL);
    free(ui->Scroll.Name);
    ui->Scroll.Name=str;

    value=gh_car(temp);
    temp=gh_cdr(temp);
    str=gh_scm2newstr(value,NULL);
    free(ui->ArrowE.Name);
    ui->ArrowE.Name=str;

    value=gh_car(temp);
    temp=gh_cdr(temp);
    str=gh_scm2newstr(value,NULL);
    free(ui->ArrowNE.Name);
    ui->ArrowNE.Name=str;

    value=gh_car(temp);
    temp=gh_cdr(temp);
    str=gh_scm2newstr(value,NULL);
    free(ui->ArrowN.Name);
    ui->ArrowN.Name=str;

    value=gh_car(temp);
    temp=gh_cdr(temp);
    str=gh_scm2newstr(value,NULL);
    free(ui->ArrowNW.Name);
    ui->ArrowNW.Name=str;

    value=gh_car(temp);
    temp=gh_cdr(temp);
    str=gh_scm2newstr(value,NULL);
    free(ui->ArrowW.Name);
    ui->ArrowW.Name=str;

    value=gh_car(temp);
    temp=gh_cdr(temp);
    str=gh_scm2newstr(value,NULL);
    free(ui->ArrowSW.Name);
    ui->ArrowSW.Name=str;

    value=gh_car(temp);
    temp=gh_cdr(temp);
    str=gh_scm2newstr(value,NULL);
    free(ui->ArrowS.Name);
    ui->ArrowS.Name=str;

    value=gh_car(temp);
    temp=gh_cdr(temp);
    str=gh_scm2newstr(value,NULL);
    free(ui->ArrowSE.Name);
    ui->ArrowSE.Name=str;

    //
    //	Panels
    //
    temp=gh_car(list);
    list=gh_cdr(list);

    value=gh_car(temp);
    temp=gh_cdr(temp);
    str=gh_scm2newstr(value,NULL);
    free(ui->GameMenuePanel.File);
    ui->GameMenuePanel.File=str;

    temp=gh_car(list);
    list=gh_cdr(list);

    value=gh_car(temp);
    temp=gh_cdr(temp);
    str=gh_scm2newstr(value,NULL);
    free(ui->Menue1Panel.File);
    ui->Menue1Panel.File=str;

    temp=gh_car(list);
    list=gh_cdr(list);

    value=gh_car(temp);
    temp=gh_cdr(temp);
    str=gh_scm2newstr(value,NULL);
    free(ui->Menue2Panel.File);
    ui->Menue2Panel.File=str;

    temp=gh_car(list);
    list=gh_cdr(list);

    value=gh_car(temp);
    temp=gh_cdr(temp);
    str=gh_scm2newstr(value,NULL);
    free(ui->VictoryPanel.File);
    ui->VictoryPanel.File=str;

    temp=gh_car(list);
    list=gh_cdr(list);

    value=gh_car(temp);
    temp=gh_cdr(temp);
    str=gh_scm2newstr(value,NULL);
    free(ui->ScenarioPanel.File);
    ui->ScenarioPanel.File=str;

    return SCM_UNSPECIFIED;
}

/**
**	Enable/disable scrolling with the mouse.
**
**	@param flag	True = turn on, false = off.
**	@return		The old state of scrolling.
*/
local SCM CclSetMouseScroll(SCM flag)
{
    int old;

    old=TheUI.MouseScroll;
    TheUI.MouseScroll=gh_scm2bool(flag);

    return gh_bool2scm(old);
}

/**
**	Set speed of mouse scrolling
**
**	@param num	Mouse scroll speed in frames.
**	@return		old scroll speed.
*/
local SCM CclSetMouseScrollSpeed(SCM num)
{
    int speed;
    int old;

    old=SpeedMouseScroll;
    speed=gh_scm2int(num);
    if (speed < 1 || speed > FRAMES_PER_SECOND) {
	SpeedMouseScroll=MOUSE_SCROLL_SPEED;
    } else {
	SpeedMouseScroll=speed;
    }
    return gh_int2scm(old);
}

/**
**	Enable/disable grabbing the mouse.
**
**	@param flag	True = grab on, false = grab off.
**	@return		FIXME: not supported: The old state of grabbing.
*/
local SCM CclSetGrabMouse(SCM flag)
{
    if( gh_scm2bool(flag) ) {
	ToggleGrabMouse(1);
    } else {
	ToggleGrabMouse(-1);
    }

    //return gh_bool2scm(old);
    return SCM_UNSPECIFIED;
}

/**
**	Enable/disable leaving the window stops scrolling.
**
**	@param flag	True = stop on, false = stop off.
**	@return		The old state of stopping.
*/
local SCM CclSetLeaveStops(SCM flag)
{
    int old;

    old=LeaveStops;
    LeaveStops=gh_scm2bool(flag);

    return gh_bool2scm(old);
}

/**
**	Enable/disable scrolling with the keyboard.
**
**	@param flag	True = turn on, false = off.
**	@return		The old state of scrolling.
*/
local SCM CclSetKeyScroll(SCM flag)
{
    int old;

    old=TheUI.KeyScroll;
    TheUI.KeyScroll=gh_scm2bool(flag);

    return gh_bool2scm(old);
}

/**
**	Set speed of keyboard scrolling
**
**	@param num	Keyboard scroll speed in frames.
**	@return		old scroll speed.
*/
local SCM CclSetKeyScrollSpeed(SCM num)
{
    int speed;
    int old;

    old=SpeedKeyScroll;
    speed=gh_scm2int(num);
    if (speed < 1 || speed > FRAMES_PER_SECOND) {
	SpeedKeyScroll=KEY_SCROLL_SPEED;
    } else {
	SpeedKeyScroll=speed;
    }
    return gh_int2scm(old);
}

/**
**	Enable/disable display of command keys in panels.
**
**	@param flag	True = turn on, false = off.
**	@return		The old state of scrolling.
*/
local SCM CclSetShowCommandKey(SCM flag)
{
    int old;

    old=ShowCommandKey;
    ShowCommandKey=gh_scm2bool(flag);
    UpdateButtonPanel();

    return gh_bool2scm(old);
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
**	Enable/disable the fancy buildings.
**
**	@param flag	True = turn on, false = off.
**	@return		The old state of fancy buildings flag.
*/
local SCM CclSetFancyBuildings(SCM flag)
{
    int old;

    old=FancyBuildings;
    FancyBuildings=gh_scm2bool(flag);

    return gh_bool2scm(old);
}

/**
**	Define a menu
**
**	FIXME: need some general data structure to make this parsing easier.
**
**	@param list	List describing the menu.
*/
local SCM CclDefineMenu(SCM list)
{
    SCM value;
    Menu *menu, item;
    char *name = NULL;
    char *s1;
    void **func;

    DebugLevel3Fn("Define menu\n");

    //
    //	Parse the arguments, already the new tagged format.
    //
    memset(&item,0,sizeof(Menu));

    while ( !gh_null_p(list) ) {
	value=gh_car(list);
	list=gh_cdr(list);
	if( gh_eq_p(value,gh_symbol2scm("geometry")) ) {
	    value=gh_car(list);
	    list=gh_cdr(list);

	    item.x=gh_scm2int(gh_car(value));
	    value=gh_cdr(value);
	    item.y=gh_scm2int(gh_car(value));
	    value=gh_cdr(value);
	    item.xsize=gh_scm2int(gh_car(value));
	    value=gh_cdr(value);
	    item.ysize=gh_scm2int(gh_car(value));

	} else if( gh_eq_p(value,gh_symbol2scm("name")) ) {
	    value=gh_car(list);
	    list=gh_cdr(list);
	    name=gh_scm2newstr(value, NULL);
	} else if( gh_eq_p(value,gh_symbol2scm("image")) ) {
	    value=gh_car(list);
	    list=gh_cdr(list);
	    if( gh_eq_p(value,gh_symbol2scm("none")) ) {
		item.image=ImageNone;
	    } else if( gh_eq_p(value,gh_symbol2scm("panel1")) ) {
		item.image=ImagePanel1;
	    } else if( gh_eq_p(value,gh_symbol2scm("panel2")) ) {
		item.image=ImagePanel2;
	    } else if( gh_eq_p(value,gh_symbol2scm("panel3")) ) {
		item.image=ImagePanel3;
	    } else if( gh_eq_p(value,gh_symbol2scm("panel4")) ) {
		item.image=ImagePanel4;
	    } else if( gh_eq_p(value,gh_symbol2scm("panel5")) ) {
		item.image=ImagePanel5;
	    } else {
		s1=gh_scm2newstr(value, NULL);
		fprintf(stderr, "Unsupported image %s\n", s1);
		free(s1);
	    }
	} else if( gh_eq_p(value,gh_symbol2scm("default")) ) {
	    value=gh_car(list);
	    list=gh_cdr(list);
	    item.defsel=gh_scm2int(value);
/*
	} else if( gh_eq_p(value,gh_symbol2scm("nitems")) ) {
	    value=gh_car(list);
	    list=gh_cdr(list);
	    item.nitems=gh_scm2int(value);
*/
	} else if( gh_eq_p(value,gh_symbol2scm("netaction")) ) {
	    value=gh_car(list);
	    list=gh_cdr(list);
	    s1 = gh_scm2newstr(value,NULL);
	    func = (void **)hash_find(MenuFuncHash, s1);
	    if (func != NULL) {
		item.netaction=(void *)*func;
	    } else {
		fprintf(stderr,"Can't find function: %s\n", s1);
	    }
	    free(s1);
	} else {
	    s1=gh_scm2newstr(value, NULL);
	    fprintf(stderr, "Unsupported tag %s\n", s1);
	    free(s1);
	}
    }

    if (name) {
	menu = (Menu*)calloc(1,sizeof(Menu));
	memcpy(menu, &item, sizeof(Menu));
	menu->nitems = 0; // reset to zero
	//move the buttons for different resolutions..
	if (VideoWidth != 640) {
	    if (VideoWidth == 0) {
		if (DEFAULT_VIDEO_WIDTH != 640) {
		    menu->x += (DEFAULT_VIDEO_WIDTH - 640) / 2;
		}
		if (DEFAULT_VIDEO_HEIGHT != 480) {
		    menu->y += (DEFAULT_VIDEO_HEIGHT - 480) / 2;
		}
	    } else {
		//printf("VideoWidth = %d\n", VideoWidth);
		menu->x += (VideoWidth - 640) / 2;
		menu->y += (VideoHeight - 480) / 2;
	    }
	}
	//printf("Me:%s\n", name);
	*(Menu **)hash_add(MenuHash,name) = menu;
    } else {
	fprintf(stderr,"Name of menu is missed, skip definition\n");
    }

    return SCM_UNSPECIFIED;
}

/**
**	Define a menu item
**
**	FIXME: need some general data structure to make this parsing easier.
**
**	@param value	Button type.
*/
local MenuButtonId scm2buttonid(SCM value)
{
    MenuButtonId id;

    if ( gh_eq_p(value, gh_symbol2scm("main")) ) {
        id=MBUTTON_MAIN;
    } else if ( gh_eq_p(value, gh_symbol2scm("gm-half")) ) {
        id=MBUTTON_GM_HALF;
    } else if ( gh_eq_p(value, gh_symbol2scm("132")) ) {
        id=MBUTTON_132;
    } else if ( gh_eq_p(value, gh_symbol2scm("gm-full")) ) {
        id=MBUTTON_GM_FULL;
    } else if ( gh_eq_p(value, gh_symbol2scm("gem-round")) ) {
        id=MBUTTON_GEM_ROUND;
    } else if ( gh_eq_p(value, gh_symbol2scm("gem-square")) ) {
        id=MBUTTON_GEM_SQUARE;
    } else if ( gh_eq_p(value, gh_symbol2scm("up-arrow")) ) {
        id=MBUTTON_UP_ARROW;
    } else if ( gh_eq_p(value, gh_symbol2scm("down-arrow")) ) {
        id=MBUTTON_DOWN_ARROW;
    } else if ( gh_eq_p(value, gh_symbol2scm("left-arrow")) ) {
        id=MBUTTON_LEFT_ARROW;
    } else if ( gh_eq_p(value, gh_symbol2scm("right-arrow")) ) {
        id=MBUTTON_RIGHT_ARROW;
    } else if ( gh_eq_p(value, gh_symbol2scm("s-knob")) ) {
        id=MBUTTON_S_KNOB;
    } else if ( gh_eq_p(value, gh_symbol2scm("s-vcont")) ) {
        id=MBUTTON_S_VCONT;
    } else if ( gh_eq_p(value, gh_symbol2scm("s-hcont")) ) {
        id=MBUTTON_S_HCONT;
    } else if ( gh_eq_p(value, gh_symbol2scm("pulldown")) ) {
        id=MBUTTON_PULLDOWN;
    } else if ( gh_eq_p(value, gh_symbol2scm("vthin")) ) {
        id=MBUTTON_VTHIN;
    } else if ( gh_eq_p(value, gh_symbol2scm("folder")) ) {
        id=MBUTTON_FOLDER;
    } else {
	char *s1=gh_scm2newstr(value, NULL);
        fprintf(stderr, "Unsupported button %s\n", s1);
        free(s1);
	return 0;
    }
    return id;
}

local int scm2hotkey(SCM value)
{
    char *s;
    int l, key=0, f;

    s = gh_scm2newstr(value,NULL);
    l = strlen(s);

    if (l==0) {
	key=0;
    } else if (l==1) {
	key=s[0];
    } else if (!strcmp(s,"esc")) {
	key=27;
    } else if (s[0]=='f' && l>1 && l<4) {
	f = atoi(s+1);
	if (f > 0 && f < 13) {
	    key = KeyCodeF1+f-1; // if key-order in include/interface.h is linear
	} else {
	    printf("Unknow key '%s'\n", s);
	}
    } else {
	printf("Unknow key '%s'\n", s);
    }
    free(s);
    return key;
}

local SCM CclDefineMenuItem(SCM list)
{
    SCM value, sublist;
    char* s1;
    char* name;
    Menuitem *item;
    Menu **tmp, *menu;
    void **func;

    DebugLevel3Fn("Define menu-item\n");

    name = NULL;
    item = (Menuitem*)calloc(1,sizeof(Menuitem));

    //
    //	Parse the arguments, already the new tagged format.
    //
    while( !gh_null_p(list) ) {
	value=gh_car(list);
	list=gh_cdr(list);
	if( gh_eq_p(value,gh_symbol2scm("pos")) ) {
	    value=gh_car(list);
	    list=gh_cdr(list);

	    item->xofs=gh_scm2int(gh_car(value));
	    value=gh_cdr(value);
	    item->yofs=gh_scm2int(gh_car(value));

	} else if( gh_eq_p(value,gh_symbol2scm("menu")) ) {
	    value=gh_car(list);
	    list=gh_cdr(list);
	    name=gh_scm2newstr(value, NULL);
	} else if( gh_eq_p(value,gh_symbol2scm("flags")) ) {
	    sublist=gh_car(list);
	    list=gh_cdr(list);

	    while ( !gh_null_p(sublist) ) {
	    
		value = gh_car(sublist);
		sublist = gh_cdr(sublist);
	    
		if ( gh_eq_p(value,gh_symbol2scm("active")) ) {
		    item->flags|=MenuButtonActive;
		} else if ( gh_eq_p(value,gh_symbol2scm("clicked")) ) {
		    item->flags|=MenuButtonClicked;
		} else if ( gh_eq_p(value,gh_symbol2scm("selected")) ) {
		    item->flags|=MenuButtonSelected;
		} else if ( gh_eq_p(value,gh_symbol2scm("disabled")) ) {
		    item->flags|=MenuButtonDisabled;
		} else {
		    s1=gh_scm2newstr(gh_car(value),NULL);
		    fprintf(stderr,"Unknow flag %s\n",s1);
		    free(s1);
		}
	    }
	} else if( gh_eq_p(value,gh_symbol2scm("font")) ) {
	    // FIXME: should use the names of the real fonts.
	    value=gh_car(list);
	    list=gh_cdr(list);
	    if( gh_eq_p(value,gh_symbol2scm("small")) ) {
		item->font=SmallFont;
	    } else if( gh_eq_p(value,gh_symbol2scm("game")) ) {
		item->font=GameFont;
	    } else if( gh_eq_p(value,gh_symbol2scm("large")) ) {
		item->font=LargeFont;
	    } else if( gh_eq_p(value,gh_symbol2scm("small-title")) ) {
		item->font=SmallTitleFont;
	    } else if( gh_eq_p(value,gh_symbol2scm("large-title")) ) {
		item->font=LargeTitleFont;
	    } else {
		s1=gh_scm2newstr(value,NULL);
		fprintf(stderr,"Unsupported font %s\n",s1);
		free(s1);
	    }
	} else if( gh_eq_p(value,gh_symbol2scm("init")) ) {
	    value=gh_car(list);
	    list=gh_cdr(list);

	    s1 = gh_scm2newstr(value,NULL);
	    func = (void **)hash_find(MenuFuncHash,s1);
	    if (func != NULL) {
		item->initfunc=(void *)*func;
	    } else {
	        fprintf(stderr,"Can't find function: %s\n", s1);
	    }
	    free(s1);
	} else if( gh_eq_p(value,gh_symbol2scm("exit")) ) {
	    value=gh_car(list);
	    list=gh_cdr(list);

	    s1 = gh_scm2newstr(value,NULL);
	    func = (void **)hash_find(MenuFuncHash,s1);
	    if (func != NULL) {
		item->exitfunc=(void *)*func;
	    } else {
	        fprintf(stderr,"Can't find function: %s\n", s1);
	    }
	    free(s1);
/* Menu types */
	} else if( !item->mitype ) {
	    if ( gh_eq_p(value,gh_symbol2scm("text")) ) {
		value=gh_car(list);
		list=gh_cdr(list);
		item->mitype=MI_TYPE_TEXT;
		item->d.text.text = NULL;

		if ( !gh_null_p(gh_car(value)) ) {
		    item->d.text.text=gh_scm2newstr(gh_car(value), NULL);
		    // FIXME: can be removed
		    if (!strcmp(item->d.text.text, "null")) {
			free(item->d.text.text);
			item->d.text.text = NULL;
		    }
		}
		value=gh_cdr(value);
		value=gh_car(value);
		if ( gh_eq_p(value,gh_symbol2scm("center")) ) {
		    item->d.text.tflags=MI_TFLAGS_CENTERED;
		} else if ( gh_eq_p(value,gh_symbol2scm("left")) ) {
		    item->d.text.tflags=MI_TFLAGS_LALIGN;
		} else if ( gh_eq_p(value,gh_symbol2scm("right")) ) {
		    item->d.text.tflags=MI_TFLAGS_RALIGN;
		} else if ( gh_eq_p(value,gh_symbol2scm("none")) ) {
		    item->d.text.tflags=0;
		} else {
		    s1=gh_scm2newstr(gh_car(value),NULL);
		    fprintf(stderr,"Unknow flag %s\n", s1);
		    free(s1);
		}
	    } else if ( gh_eq_p(value,gh_symbol2scm("button")) ) {
		sublist=gh_car(list);
		list=gh_cdr(list);
		item->mitype=MI_TYPE_BUTTON;

		while ( !gh_null_p(sublist) ) {

		    value=gh_car(sublist);
		    sublist=gh_cdr(sublist);

		    if ( gh_eq_p(value, gh_symbol2scm("size")) ) {
			item->d.button.xsize=gh_scm2int(gh_car(gh_car(sublist)));
			value=gh_cdr(gh_car(sublist));
			item->d.button.ysize=gh_scm2int(gh_car(value));
			sublist=gh_cdr(sublist);
		    } else if ( gh_eq_p(value, gh_symbol2scm("caption")) ) {
			item->d.button.text = NULL;
			if ( !gh_null_p(gh_car(sublist)) ) {
			    item->d.button.text = gh_scm2newstr(
				gh_car(sublist),NULL);
			    // FIXME: can be removed
			    if ( !strcmp(item->d.button.text, "null") ) {
				free(item->d.button.text);
				item->d.button.text = NULL;
			    }
			}
			sublist=gh_cdr(sublist);
		    } else if ( gh_eq_p(value, gh_symbol2scm("hotkey")) ) {
			item->d.button.hotkey=scm2hotkey(gh_car(sublist));
			sublist=gh_cdr(sublist);
		    } else if ( gh_eq_p(value, gh_symbol2scm("func")) ) {
	    		s1 = gh_scm2newstr(gh_car(sublist),NULL);
			//item->d.button.handler=hash_mini_get(MenuHndlrHash, s1);
			func = (void **)hash_find(MenuFuncHash, s1);
			if (func != NULL) {
			    item->d.button.handler=(void *)*func;
			} else {
			    fprintf(stderr,"Can't find function: %s\n", s1);
			}
			free(s1);
			sublist=gh_cdr(sublist);
		    } else if ( gh_eq_p(value, gh_symbol2scm("style")) ) {
			value=gh_car(sublist);
			item->d.button.button=scm2buttonid(value);
		    } else {
			//s1=gh_scm2newstr(value, NULL);
			//fprintf(stderr, "Unsupported property %s\n", s1);
			//free(s1);
		    }
		}
	    } else if ( gh_eq_p(value,gh_symbol2scm("pulldown")) ) {
		sublist=gh_car(list);
		list=gh_cdr(list);
		item->mitype=MI_TYPE_PULLDOWN;

		while ( !gh_null_p(sublist) ) {

		    value=gh_car(sublist);
		    sublist=gh_cdr(sublist);

		    if ( gh_eq_p(value, gh_symbol2scm("size")) ) {
			item->d.pulldown.xsize=gh_scm2int(gh_car(gh_car(sublist)));
			value=gh_cdr(gh_car(sublist));
			item->d.pulldown.ysize=gh_scm2int(gh_car(value));
			sublist=gh_cdr(sublist);
		    } else if ( gh_eq_p(value, gh_symbol2scm("options")) ) {
			value=gh_car(sublist);
			if (gh_list_p(value)) {
			    int n, i;

			    n = item->d.pulldown.noptions = gh_length(value);
			    item->d.pulldown.options = (unsigned char **)malloc(sizeof(char*)*n);
			    for (i=0; i<n; i++) {
				item->d.pulldown.options[i]=gh_scm2newstr(gh_car(value),NULL);
				value = gh_cdr(value);
			    }
			}
		    } else if ( gh_eq_p(value, gh_symbol2scm("func")) ) {
	    		s1 = gh_scm2newstr(gh_car(sublist),NULL);
			func = (void **)hash_find(MenuFuncHash,s1);
			if (func != NULL) {
			    item->d.pulldown.action=(void *)*func;
			} else {
			    fprintf(stderr,"Can't find function: %s\n", s1);
			}
			free(s1);
			sublist=gh_cdr(sublist);
		    } else if ( gh_eq_p(value, gh_symbol2scm("style")) ) {
			value=gh_car(sublist);
			item->d.pulldown.button=scm2buttonid(value);
		    } else if ( gh_eq_p(value, gh_symbol2scm("state")) ) {
			value=gh_car(sublist);
			if ( gh_eq_p(value, gh_symbol2scm("passive")) ) {
			    item->d.pulldown.state=MI_PSTATE_PASSIVE;
			}
		    } else if ( gh_eq_p(value, gh_symbol2scm("default")) ) {
			item->d.pulldown.defopt=gh_scm2int(gh_car(sublist));
			sublist=gh_cdr(sublist);
		    } else if ( gh_eq_p(value, gh_symbol2scm("current")) ) {
			item->d.pulldown.curopt=gh_scm2int(gh_car(sublist));
			sublist=gh_cdr(sublist);
		    } else {
			//s1=gh_scm2newstr(value, NULL);
			//fprintf(stderr, "Unsupported property %s\n", s1);
			//free(s1);
		    }
		}
	    } else if ( gh_eq_p(value,gh_symbol2scm("listbox")) ) {
		sublist=gh_car(list);
		list=gh_cdr(list);
		item->mitype=MI_TYPE_LISTBOX;

		while ( !gh_null_p(sublist) ) {

		    value=gh_car(sublist);
		    sublist=gh_cdr(sublist);

		    if ( gh_eq_p(value, gh_symbol2scm("size")) ) {
			item->d.listbox.xsize=gh_scm2int(gh_car(gh_car(sublist)));
			value=gh_cdr(gh_car(sublist));
			item->d.listbox.ysize=gh_scm2int(gh_car(value));
			sublist=gh_cdr(sublist);
		    } else if ( gh_eq_p(value, gh_symbol2scm("func")) ) {
	    		s1 = gh_scm2newstr(gh_car(sublist),NULL);
			func = (void **)hash_find(MenuFuncHash,s1);
			if (func != NULL) {
			    item->d.listbox.action=(void *)*func;
			} else {
			    fprintf(stderr,"Can't find function: %s\n", s1);
			}
			free(s1);
			sublist=gh_cdr(sublist);
		    } else if ( gh_eq_p(value, gh_symbol2scm("handler")) ) {
	    		s1 = gh_scm2newstr(gh_car(sublist),NULL);
			func = (void **)hash_find(MenuFuncHash,s1);
			if (func != NULL) {
			    item->d.listbox.handler=(void *)*func;
			} else {
			    fprintf(stderr,"Can't find function: %s\n", s1);
			}
			free(s1);
			sublist=gh_cdr(sublist);
		    } else if ( gh_eq_p(value, gh_symbol2scm("retopt")) ) {
	    		s1 = gh_scm2newstr(gh_car(sublist),NULL);
			func = (void **)hash_find(MenuFuncHash,s1);
			if (func != NULL) {
			    item->d.listbox.retrieveopt=(void *)(*func);
			} else {
			    fprintf(stderr,"Can't find function: %s\n", s1);
			}
			free(s1);
			sublist=gh_cdr(sublist);
		    } else if ( gh_eq_p(value, gh_symbol2scm("style")) ) {
			value=gh_car(sublist);
			item->d.listbox.button=scm2buttonid(value);
		    } else if ( gh_eq_p(value, gh_symbol2scm("default")) ) {
			item->d.listbox.defopt=gh_scm2int(gh_car(sublist));
			sublist=gh_cdr(sublist);
		    } else if ( gh_eq_p(value, gh_symbol2scm("startline")) ) {
			item->d.listbox.startline=gh_scm2int(gh_car(sublist));
			sublist=gh_cdr(sublist);
		    } else if ( gh_eq_p(value, gh_symbol2scm("nlines")) ) {
			item->d.listbox.nlines=gh_scm2int(gh_car(sublist));
			sublist=gh_cdr(sublist);
		    } else if ( gh_eq_p(value, gh_symbol2scm("current")) ) {
			item->d.listbox.curopt=gh_scm2int(gh_car(sublist));
			sublist=gh_cdr(sublist);
		    } else {
			//s1=gh_scm2newstr(value, NULL);
			//fprintf(stderr, "Unsupported property %s\n", s1);
			//free(s1);
		    }
		}
	    } else if ( gh_eq_p(value,gh_symbol2scm("vslider")) ) {
		sublist=gh_car(list);
		list=gh_cdr(list);
		item->mitype=MI_TYPE_VSLIDER;
		item->d.vslider.defper=-1;

		while ( !gh_null_p(sublist) ) {

		    value=gh_car(sublist);
		    sublist=gh_cdr(sublist);

		    if ( gh_eq_p(value, gh_symbol2scm("size")) ) {
			item->d.vslider.xsize=gh_scm2int(gh_car(gh_car(sublist)));
			value=gh_cdr(gh_car(sublist));
			item->d.vslider.ysize=gh_scm2int(gh_car(value));
			sublist=gh_cdr(sublist);
		    } else if ( gh_eq_p(value, gh_symbol2scm("flags")) ) {
			SCM slist;

			slist = gh_car(sublist);
			while ( !gh_null_p(slist) ) {
	    
			    value = gh_car(slist);
			    slist = gh_cdr(slist);
	    
			    if ( gh_eq_p(value,gh_symbol2scm("up")) ) {
				item->d.vslider.cflags|=MI_CFLAGS_UP;
			    } else if ( gh_eq_p(value,gh_symbol2scm("down")) ) {
				item->d.vslider.cflags|=MI_CFLAGS_DOWN;
			    } else if ( gh_eq_p(value,gh_symbol2scm("left")) ) {
				item->d.vslider.cflags|=MI_CFLAGS_LEFT;
			    } else if ( gh_eq_p(value,gh_symbol2scm("right")) ) {
				item->d.vslider.cflags|=MI_CFLAGS_RIGHT;
			    } else if ( gh_eq_p(value,gh_symbol2scm("knob")) ) {
				item->d.vslider.cflags|=MI_CFLAGS_KNOB;
			    } else if ( gh_eq_p(value,gh_symbol2scm("cont")) ) {
				item->d.vslider.cflags|=MI_CFLAGS_CONT;
			    } else {
				s1=gh_scm2newstr(gh_car(value),NULL);
				fprintf(stderr,"Unknow flag %s\n",s1);
				free(s1);
			    }
			}
			sublist=gh_cdr(sublist);
		    } else if ( gh_eq_p(value, gh_symbol2scm("func")) ) {
	    		s1 = gh_scm2newstr(gh_car(sublist),NULL);
			func = (void **)hash_find(MenuFuncHash,s1);
			if (func != NULL) {
			    item->d.vslider.action=(void *)*func;
			} else {
			    fprintf(stderr,"Can't find function: %s\n", s1);
			}
			free(s1);
			sublist=gh_cdr(sublist);
		    } else if ( gh_eq_p(value, gh_symbol2scm("handler")) ) {
	    		s1 = gh_scm2newstr(gh_car(sublist),NULL);
			func = (void **)hash_find(MenuFuncHash,s1);
			if (func != NULL) {
			    item->d.vslider.handler=(void *)*func;
			} else {
			    fprintf(stderr,"Can't find function: %s\n", s1);
			}
			free(s1);
			sublist=gh_cdr(sublist);
		    } else if ( gh_eq_p(value, gh_symbol2scm("default")) ) {
			item->d.vslider.defper=gh_scm2int(gh_car(sublist));
			sublist=gh_cdr(sublist);
		    } else if ( gh_eq_p(value, gh_symbol2scm("current")) ) {
			item->d.vslider.percent=gh_scm2int(gh_car(sublist));
			sublist=gh_cdr(sublist);
		    } else {
			//s1=gh_scm2newstr(value, NULL);
			//fprintf(stderr, "Unsupported property %s\n", s1);
			//free(s1);
		    }
		}
	    } else if ( gh_eq_p(value,gh_symbol2scm("drawfunc")) ) {
		value=gh_car(list);
		list=gh_cdr(list);
		item->mitype=MI_TYPE_DRAWFUNC;

		s1 = gh_scm2newstr(value,NULL);
		func = (void **)hash_find(MenuFuncHash,s1);
		if (func != NULL) {
		    item->d.drawfunc.draw=(void *)*func;
		} else {
		    fprintf(stderr,"Can't find function: %s\n", s1);
		}
		free(s1);
	    } else if ( gh_eq_p(value,gh_symbol2scm("input")) ) {
		sublist=gh_car(list);
		list=gh_cdr(list);
		item->mitype=MI_TYPE_INPUT;

		while ( !gh_null_p(sublist) ) {

		    value=gh_car(sublist);
		    sublist=gh_cdr(sublist);

		    if ( gh_eq_p(value, gh_symbol2scm("size")) ) {
			item->d.input.xsize=gh_scm2int(gh_car(gh_car(sublist)));
			value=gh_cdr(gh_car(sublist));
			item->d.input.ysize=gh_scm2int(gh_car(value));
			sublist=gh_cdr(sublist);
		    } else if ( gh_eq_p(value, gh_symbol2scm("text")) ) {
			item->d.input.buffer=gh_scm2newstr(gh_car(sublist),NULL);
			item->d.input.nch=strlen(item->d.input.buffer);
			sublist=gh_cdr(sublist);
		    } else if ( gh_eq_p(value, gh_symbol2scm("func")) ) {
	    		s1 = gh_scm2newstr(gh_car(sublist),NULL);
			func = (void **)hash_find(MenuFuncHash, s1);
			if (func != NULL) {
			    item->d.input.action=(void *)*func;
			} else {
			    fprintf(stderr,"Can't find function: %s\n", s1);
			}
			free(s1);
			sublist=gh_cdr(sublist);
		    } else if ( gh_eq_p(value, gh_symbol2scm("style")) ) {
			value=gh_car(sublist);
			item->d.input.button=scm2buttonid(value);
		    } else if ( gh_eq_p(value, gh_symbol2scm("maxch")) ) {
			value=gh_car(sublist);
			item->d.input.maxch=gh_scm2int(value);
		    } else {
			//s1=gh_scm2newstr(value, NULL);
			//fprintf(stderr, "Unsupported property %s\n", s1);
			//free(s1);
		    }
		}
	    } else if ( gh_eq_p(value,gh_symbol2scm("gem")) ) {
		sublist=gh_car(list);
		list=gh_cdr(list);
		item->mitype=MI_TYPE_GEM;

		while ( !gh_null_p(sublist) ) {

		    value=gh_car(sublist);
		    sublist=gh_cdr(sublist);

		    if ( gh_eq_p(value, gh_symbol2scm("size")) ) {
			item->d.gem.xsize=gh_scm2int(gh_car(gh_car(sublist)));
			value=gh_cdr(gh_car(sublist));
			item->d.gem.ysize=gh_scm2int(gh_car(value));
			sublist=gh_cdr(sublist);
		    } else if ( gh_eq_p(value, gh_symbol2scm("state")) ) {
			value=gh_car(sublist);
			if ( gh_eq_p(value, gh_symbol2scm("unchecked")) ) {
			    item->d.gem.state=MI_GSTATE_UNCHECKED;
			} else if ( gh_eq_p(value, gh_symbol2scm("passive")) ) {
			    item->d.gem.state=MI_GSTATE_PASSIVE;
			} else if ( gh_eq_p(value, gh_symbol2scm("invisible")) ) {
			    item->d.gem.state=MI_GSTATE_INVISIBLE;
			} else if ( gh_eq_p(value, gh_symbol2scm("checked")) ) {
			    item->d.gem.state=MI_GSTATE_CHECKED;
			}
		    } else if ( gh_eq_p(value, gh_symbol2scm("func")) ) {
	    		s1 = gh_scm2newstr(gh_car(sublist),NULL);
			func = (void **)hash_find(MenuFuncHash,s1);
			if (func != NULL) {
			    item->d.gem.action=(void *)*func;
			} else {
			    fprintf(stderr,"Can't find function: %s\n", s1);
			}
			free(s1);
			sublist=gh_cdr(sublist);
		    } else if ( gh_eq_p(value, gh_symbol2scm("style")) ) {
			value=gh_car(sublist);
			item->d.gem.button=scm2buttonid(value);
		    } else {
			//s1=gh_scm2newstr(value, NULL);
			//fprintf(stderr, "Unsupported property %s\n", s1);
			//free(s1);
		    }
		}
	    } else if ( gh_eq_p(value,gh_symbol2scm("hslider")) ) {
		sublist=gh_car(list);
		list=gh_cdr(list);
		item->mitype=MI_TYPE_HSLIDER;
		item->d.hslider.defper=-1;

		while ( !gh_null_p(sublist) ) {

		    value=gh_car(sublist);
		    sublist=gh_cdr(sublist);

		    if ( gh_eq_p(value, gh_symbol2scm("size")) ) {
			item->d.hslider.xsize=gh_scm2int(gh_car(gh_car(sublist)));
			value=gh_cdr(gh_car(sublist));
			item->d.hslider.ysize=gh_scm2int(gh_car(value));
			sublist=gh_cdr(sublist);
		    } else if ( gh_eq_p(value, gh_symbol2scm("flags")) ) {
			SCM slist;

			slist = gh_car(sublist);
			while ( !gh_null_p(slist) ) {
	    
			    value = gh_car(slist);
			    slist = gh_cdr(slist);
	    
			    if ( gh_eq_p(value,gh_symbol2scm("up")) ) {
				item->d.hslider.cflags|=MI_CFLAGS_UP;
			    } else if ( gh_eq_p(value,gh_symbol2scm("down")) ) {
				item->d.hslider.cflags|=MI_CFLAGS_DOWN;
			    } else if ( gh_eq_p(value,gh_symbol2scm("left")) ) {
				item->d.hslider.cflags|=MI_CFLAGS_LEFT;
			    } else if ( gh_eq_p(value,gh_symbol2scm("right")) ) {
				item->d.hslider.cflags|=MI_CFLAGS_RIGHT;
			    } else if ( gh_eq_p(value,gh_symbol2scm("knob")) ) {
				item->d.hslider.cflags|=MI_CFLAGS_KNOB;
			    } else if ( gh_eq_p(value,gh_symbol2scm("cont")) ) {
				item->d.hslider.cflags|=MI_CFLAGS_CONT;
			    } else {
				s1=gh_scm2newstr(gh_car(value),NULL);
				fprintf(stderr,"Unknow flag %s\n",s1);
				free(s1);
			    }
			}
			sublist=gh_cdr(sublist);
		    } else if ( gh_eq_p(value, gh_symbol2scm("func")) ) {
	    		s1 = gh_scm2newstr(gh_car(sublist),NULL);
			func = (void **)hash_find(MenuFuncHash,s1);
			if (func != NULL) {
			    item->d.hslider.action=(void *)*func;
			} else {
			    fprintf(stderr,"Can't find function: %s\n", s1);
			}
			free(s1);
			sublist=gh_cdr(sublist);
		    } else if ( gh_eq_p(value, gh_symbol2scm("handler")) ) {
	    		s1 = gh_scm2newstr(gh_car(sublist),NULL);
			func = (void **)hash_find(MenuFuncHash,s1);
			if (func != NULL) {
			    item->d.hslider.handler=(void *)*func;
			} else {
			    fprintf(stderr,"Can't find function: %s\n", s1);
			}
			free(s1);
			sublist=gh_cdr(sublist);
		    } else if ( gh_eq_p(value, gh_symbol2scm("default")) ) {
			item->d.hslider.defper=gh_scm2int(gh_car(sublist));
			sublist=gh_cdr(sublist);
		    } else if ( gh_eq_p(value, gh_symbol2scm("current")) ) {
			item->d.hslider.percent=gh_scm2int(gh_car(sublist));
			sublist=gh_cdr(sublist);
		    } else {
			//s1=gh_scm2newstr(value, NULL);
			//fprintf(stderr, "Unsupported property %s\n", s1);
			//free(s1);
		    }
		}
	    }
	} else {
	    s1=gh_scm2newstr(value, NULL);
	    fprintf(stderr, "Unsupported tag %s\n", s1);
	    free(s1);
	}
    }

    if ( (tmp = (Menu **)hash_find(MenuHash,name)) ) {
	menu = *tmp;
	if (menu->items) {
	    menu->items=(Menuitem*)realloc(menu->items,sizeof(Menuitem)*(menu->nitems+1));
	} else {
	    menu->items=(Menuitem*)malloc(sizeof(Menuitem));
	}
	item->menu = menu;
	memcpy(menu->items+menu->nitems,item,sizeof(Menuitem));
	menu->nitems++;
    }
    free(name);
    free(item);

    return SCM_UNSPECIFIED;
}


/**
**	Define a button.
**
**	FIXME: need some general data structure to make this parsing easier.
**
**	@param list	List describing the button.
*/
local SCM CclDefineButton(SCM list)
{
    char buf[64];
    SCM value;
    char* s1;
    char* s2;
    ButtonAction ba;

    DebugLevel3Fn("Define button\n");

    memset(&ba,0,sizeof(ba));
    //
    //	Parse the arguments, already the new tagged format.
    //
    while( !gh_null_p(list) ) {
	value=gh_car(list);
	list=gh_cdr(list);
	if( gh_eq_p(value,gh_symbol2scm("pos")) ) {
	    value=gh_car(list);
	    list=gh_cdr(list);
	    ba.Pos=gh_scm2int(value);
	} else if( gh_eq_p(value,gh_symbol2scm("level")) ) {
	    value=gh_car(list);
	    list=gh_cdr(list);
	    ba.Level=gh_scm2int(value);
	} else if( gh_eq_p(value,gh_symbol2scm("icon")) ) {
	    value=gh_car(list);
	    list=gh_cdr(list);
	    ba.Icon.Name=gh_scm2newstr(value,NULL);
	} else if( gh_eq_p(value,gh_symbol2scm("action")) ) {
	    value=gh_car(list);
	    list=gh_cdr(list);
	    if( gh_eq_p(value,gh_symbol2scm("move")) ) {
		ba.Action=ButtonMove;
	    } else if( gh_eq_p(value,gh_symbol2scm("stop")) ) {
		ba.Action=ButtonStop;
	    } else if( gh_eq_p(value,gh_symbol2scm("attack")) ) {
		ba.Action=ButtonAttack;
	    } else if( gh_eq_p(value,gh_symbol2scm("repair")) ) {
		ba.Action=ButtonRepair;
	    } else if( gh_eq_p(value,gh_symbol2scm("harvest")) ) {
		ba.Action=ButtonHarvest;
	    } else if( gh_eq_p(value,gh_symbol2scm("button")) ) {
		ba.Action=ButtonButton;
	    } else if( gh_eq_p(value,gh_symbol2scm("build")) ) {
		ba.Action=ButtonBuild;
	    } else if( gh_eq_p(value,gh_symbol2scm("train-unit")) ) {
		ba.Action=ButtonTrain;
	    } else if( gh_eq_p(value,gh_symbol2scm("patrol")) ) {
		ba.Action=ButtonPatrol;
	    } else if( gh_eq_p(value,gh_symbol2scm("stand-ground")) ) {
		ba.Action=ButtonStandGround;
	    } else if( gh_eq_p(value,gh_symbol2scm("attack-ground")) ) {
		ba.Action=ButtonAttackGround;
	    } else if( gh_eq_p(value,gh_symbol2scm("return-goods")) ) {
		ba.Action=ButtonReturn;
	    } else if( gh_eq_p(value,gh_symbol2scm("demolish")) ) {
		ba.Action=ButtonDemolish;
	    } else if( gh_eq_p(value,gh_symbol2scm("cast-spell")) ) {
		ba.Action=ButtonSpellCast;
	    } else if( gh_eq_p(value,gh_symbol2scm("research")) ) {
		ba.Action=ButtonResearch;
	    } else if( gh_eq_p(value,gh_symbol2scm("upgrade-to")) ) {
		ba.Action=ButtonUpgradeTo;
	    } else if( gh_eq_p(value,gh_symbol2scm("unload")) ) {
		ba.Action=ButtonUnload;
	    } else if( gh_eq_p(value,gh_symbol2scm("cancel")) ) {
		ba.Action=ButtonCancel;
	    } else if( gh_eq_p(value,gh_symbol2scm("cancel-upgrade")) ) {
		ba.Action=ButtonCancelUpgrade;
	    } else if( gh_eq_p(value,gh_symbol2scm("cancel-train-unit")) ) {
		ba.Action=ButtonCancelTrain;
	    } else if( gh_eq_p(value,gh_symbol2scm("cancel-build")) ) {
		ba.Action=ButtonCancelBuild;
	    } else {
		s1=gh_scm2newstr(value,NULL);
		fprintf(stderr,"Unsupported action %s\n",s1);
		free(s1);
	    }
	} else if( gh_eq_p(value,gh_symbol2scm("value")) ) {
	    value=gh_car(list);
	    list=gh_cdr(list);
	    if( gh_exact_p(value) ) {
		sprintf(buf,"%ld",gh_scm2int(value));
		s1=strdup(buf);
	    } else {
		s1=gh_scm2newstr(value,NULL);
	    }
	    ba.ValueStr=s1;
	} else if( gh_eq_p(value,gh_symbol2scm("allowed")) ) {
	    value=gh_car(list);
	    list=gh_cdr(list);
	    if( gh_eq_p(value,gh_symbol2scm("check-true")) ) {
		ba.Allowed=ButtonCheckTrue;
	    } else if( gh_eq_p(value,gh_symbol2scm("check-false")) ) {
		ba.Allowed=ButtonCheckFalse;
	    } else if( gh_eq_p(value,gh_symbol2scm("check-upgrade")) ) {
		ba.Allowed=ButtonCheckUpgrade;
	    } else if( gh_eq_p(value,gh_symbol2scm("check-unit")) ) {
		ba.Allowed=ButtonCheckUnit;
	    } else if( gh_eq_p(value,gh_symbol2scm("check-units")) ) {
		ba.Allowed=ButtonCheckUnits;
	    } else if( gh_eq_p(value,gh_symbol2scm("check-network")) ) {
		ba.Allowed=ButtonCheckNetwork;
	    } else if( gh_eq_p(value,gh_symbol2scm("check-no-work")) ) {
		ba.Allowed=ButtonCheckNoWork;
	    } else if( gh_eq_p(value,gh_symbol2scm("check-no-research")) ) {
		ba.Allowed=ButtonCheckNoResearch;
	    } else if( gh_eq_p(value,gh_symbol2scm("check-attack")) ) {
		ba.Allowed=ButtonCheckAttack;
	    } else if( gh_eq_p(value,gh_symbol2scm("check-upgrade-to")) ) {
		ba.Allowed=ButtonCheckUpgradeTo;
	    } else if( gh_eq_p(value,gh_symbol2scm("check-research")) ) {
		ba.Allowed=ButtonCheckResearch;
	    } else if( gh_eq_p(value,gh_symbol2scm("check-single-research")) ) {
		ba.Allowed=ButtonCheckSingleResearch;
	    } else {
		s1=gh_scm2newstr(value,NULL);
		fprintf(stderr,"Unsupported action %s\n",s1);
		free(s1);
	    }
	} else if( gh_eq_p(value,gh_symbol2scm("allow-arg")) ) {
	    value=gh_car(list);
	    list=gh_cdr(list);
	    s1=strdup("");
	    while( !gh_null_p(value) ) {
		s2=gh_scm2newstr(gh_car(value),NULL);
		s1=realloc(s1,strlen(s1)+strlen(s2)+2);
		strcat(s1,s2);
		value=gh_cdr(value);
		if( !gh_null_p(value) ) {
		    strcat(s1,",");
		}
	    }
	    ba.AllowStr=s1;
	} else if( gh_eq_p(value,gh_symbol2scm("key")) ) {
	    value=gh_car(list);
	    list=gh_cdr(list);
	    s1=gh_scm2newstr(value,NULL);
	    ba.Key=*s1;
	    free(s1);
	} else if( gh_eq_p(value,gh_symbol2scm("hint")) ) {
	    value=gh_car(list);
	    list=gh_cdr(list);
	    ba.Hint=gh_scm2newstr(value,NULL);
	} else if( gh_eq_p(value,gh_symbol2scm("for-unit")) ) {
	    value=gh_car(list);
	    list=gh_cdr(list);
	    s1=strdup(",");
	    while( !gh_null_p(value) ) {
		s2=gh_scm2newstr(gh_car(value),NULL);
		s1=realloc(s1,strlen(s1)+strlen(s2)+2);
		strcat(s1,s2);
		strcat(s1,",");
		value=gh_cdr(value);
	    }
	    ba.UnitMask=s1;
	    if( !strncmp(ba.UnitMask,",*,",3) ) {
		free(ba.UnitMask);
		ba.UnitMask=strdup("*");
	    }
	} else {
	    s1=gh_scm2newstr(value,NULL);
	    fprintf(stderr,"Unsupported tag %s\n",s1);
	    free(s1);
	}
    }
    AddButton(ba.Pos,ba.Level,ba.Icon.Name,ba.Action,ba.ValueStr,
	    ba.Allowed,ba.AllowStr,ba.Key,ba.Hint,ba.UnitMask);
    if( ba.ValueStr ) {
	free(ba.ValueStr);
    }
    if( ba.AllowStr ) {
	free(ba.AllowStr);
    }
    if( ba.Hint ) {
        free(ba.Hint);
    }
    if( ba.UnitMask ) {
        free(ba.UnitMask);
    }

    return SCM_UNSPECIFIED;
}

/**
**	Set double-click delay.
**
**	@param delay	Delay in ms
**	@return		Old delay
*/
local SCM CclSetDoubleClickDelay(SCM delay)
{
    int i;

    i=DoubleClickDelay;
    DoubleClickDelay=gh_scm2int(delay);

    return gh_int2scm(i);
}

/**
**	Set hold-click delay.
**
**	@param delay	Delay in ms
**	@return		Old delay
*/
local SCM CclSetHoldClickDelay(SCM delay)
{
    int i;

    i=HoldClickDelay;
    HoldClickDelay=gh_scm2int(delay);

    return gh_int2scm(i);
}

/**
**	Set selection style.
**
**	@param style	New style
**	@return		Old style
*/
local SCM CclSetSelectionStyle(SCM style)
{
    SCM old;

    old=NIL;

    if( !gh_null_p(style) ) {
	if( gh_eq_p(style,gh_symbol2scm("rectangle")) ) {
	    DrawSelection=DrawSelectionRectangle;
	} else if( gh_eq_p(style,gh_symbol2scm("alpha-rectangle")) ) {
	    DrawSelection=DrawSelectionRectangleWithTrans;
	} else if( gh_eq_p(style,gh_symbol2scm("circle")) ) {
	    DrawSelection=DrawSelectionCircle;
	} else if( gh_eq_p(style,gh_symbol2scm("alpha-circle")) ) {
	    DrawSelection=DrawSelectionCircleWithTrans;
	} else if( gh_eq_p(style,gh_symbol2scm("corners")) ) {
	    DrawSelection=DrawSelectionCorners;
	} else {
	    errl("Unsupported selection style",style);
	}
    } else {
	DrawSelection=DrawSelectionNone;
    }
    return old;
}

/**
**	Set display of sight range.
**
**	@param flag	True = turning display of sight on, false = off.
**
**	@return		The old state of display of sight.
*/
local SCM CclSetShowSightRange(SCM flag)
{
    int old;

    old=ShowSightRange;
    if( !gh_null_p(flag) ) {
	if( gh_eq_p(flag,gh_symbol2scm("rectangle")) ) {
	    ShowSightRange=1;
	} else if( gh_eq_p(flag,gh_symbol2scm("circle")) ) {
	    ShowSightRange=2;
	} else {
	    ShowSightRange=3;
	}
    } else {
	ShowSightRange=0;
    }

    return gh_int2scm(old);
}

/**
**	Set display of reaction range.
**
**	@param flag	True = turning display of reaction on, false = off.
**
**	@return		The old state of display of reaction.
*/
local SCM CclSetShowReactionRange(SCM flag)
{
    int old;

    old=ShowReactionRange;
    if( !gh_null_p(flag) ) {
	if( gh_eq_p(flag,gh_symbol2scm("rectangle")) ) {
	    ShowReactionRange=1;
	} else if( gh_eq_p(flag,gh_symbol2scm("circle")) ) {
	    ShowReactionRange=2;
	} else {
	    ShowReactionRange=3;
	}
    } else {
	ShowReactionRange=0;
    }

    return gh_int2scm(old);
}

/**
**	Set display of attack range.
**
**	@param flag	True = turning display of attack on, false = off.
**
**	@return		The old state of display of attack.
*/
local SCM CclSetShowAttackRange(SCM flag)
{
    int old;

    old=!ShowAttackRange;
    ShowAttackRange=gh_scm2bool(flag);

    return gh_bool2scm(old);
}

/**
**	Set display of orders.
**
**	@param flag	True = turning display of orders on, false = off.
**
**	@return		The old state of display of orders.
*/
local SCM CclSetShowOrders(SCM flag)
{
    int old;

    old=!ShowOrders;
    ShowOrders=gh_scm2bool(flag);

    return gh_bool2scm(old);
}

/**
**	Add a new message.
**
**	@param message	Message to display.
*/
global SCM CclSetMessage(SCM message)
{
    const char *str;

    str = get_c_string(message);
    SetMessage(str);

    return SCM_UNSPECIFIED;
}

/**
**	Reset the keystroke help array
*/
local SCM CclResetKeystrokeHelp(void)
{
    int n;
    
    n = nKeyStrokeHelps * 2;
    while (n--) {
	free(KeyStrokeHelps[n]);
    }
    if (KeyStrokeHelps) {
	free(KeyStrokeHelps);
	KeyStrokeHelps = NULL;
    }
    nKeyStrokeHelps = 0;

    return SCM_UNSPECIFIED;
}

/**
**	Add a keystroke help
**
**	@param list	pair describing the keystroke.
*/
local SCM CclAddKeystrokeHelp(SCM list)
{
    SCM value;
    char *s1, *s2;
    int n;

    if (!gh_null_p(list)) {
	value=gh_car(list);
	list=gh_cdr(list);
	s1=gh_scm2newstr(value,NULL);
    }
    if (!gh_null_p(list)) {
	value=gh_car(list);
	list=gh_cdr(list);
	s2=gh_scm2newstr(value,NULL);

	n = nKeyStrokeHelps;
	if (!n) {
	    n = 1;
	    KeyStrokeHelps = malloc(2 * sizeof(char *));
	} else {
	    n++;
	    KeyStrokeHelps = realloc(KeyStrokeHelps, n * 2 * sizeof(char *));
	}
	if (KeyStrokeHelps) {
	    nKeyStrokeHelps = n;
	    n--;
	    KeyStrokeHelps[n * 2] = s1;
	    KeyStrokeHelps[n * 2 + 1] = s2;
	}
    }

    while( !gh_null_p(list) ) {
	list=gh_cdr(list);
    }

    return SCM_UNSPECIFIED;
}

/**
**	Register CCL features for UI.
*/
global void UserInterfaceCclRegister(void)
{

    gh_new_procedure1_0("add-message", CclSetMessage);

    gh_new_procedure1_0("set-color-cycle-all!",CclSetColorCycleAll);
    gh_new_procedure1_0("set-reverse-map-move!",CclSetReverseMapMove);

    gh_new_procedure1_0("set-mouse-adjust!",CclSetMouseAdjust);
    gh_new_procedure1_0("set-mouse-scale!",CclSetMouseScale);

    gh_new_procedure1_0("set-contrast!",CclSetContrast);
    gh_new_procedure1_0("set-brightness!",CclSetBrightness);
    gh_new_procedure1_0("set-saturation!",CclSetSaturation);

    gh_new_procedure1_0("set-title-screen!",CclSetTitleScreen);
    gh_new_procedure1_0("set-menu-background!",CclSetMenuBackground);
    gh_new_procedure1_0("set-menu-background-with-title!",
	    CclSetMenuBackgroundWithTitle);
    gh_new_procedure1_0("set-title-music!",CclSetTitleMusic);
    gh_new_procedure1_0("set-menu-music!",CclSetMenuMusic);

    gh_new_procedure1_0("display-picture",CclDisplayPicture);
    gh_new_procedure1_0("process-menu",CclProcessMenu);

    gh_new_procedure1_0("set-original-resources!",CclSetOriginalResources);

    gh_new_procedureN("define-cursor",CclDefineCursor);
    gh_new_procedure1_0("set-game-cursor!",CclSetGameCursor);
    gh_new_procedureN("define-ui",CclDefineUI);

    gh_new_procedure1_0("set-grab-mouse!", CclSetGrabMouse);
    gh_new_procedure1_0("set-leave-stops!", CclSetLeaveStops);
    gh_new_procedure1_0("set-key-scroll!", CclSetKeyScroll);
    gh_new_procedure1_0("set-key-scroll-speed!", CclSetKeyScrollSpeed);
    gh_new_procedure1_0("set-mouse-scroll!", CclSetMouseScroll);
    gh_new_procedure1_0("set-mouse-scroll-speed!", CclSetMouseScrollSpeed);

    gh_new_procedure1_0("set-show-command-key!",CclSetShowCommandKey);
    gh_new_procedure0_0("right-button-attacks",CclRightButtonAttacks);
    gh_new_procedure0_0("right-button-moves",CclRightButtonMoves);
    gh_new_procedure1_0("set-fancy-buildings!",CclSetFancyBuildings);

    gh_new_procedureN("define-button",CclDefineButton);

    gh_new_procedureN("define-menu-item",CclDefineMenuItem);
    gh_new_procedureN("define-menu",CclDefineMenu);

    //
    //	Correct named functions
    //
    gh_new_procedure1_0("set-double-click-delay!",CclSetDoubleClickDelay);
    gh_new_procedure1_0("set-hold-click-delay!",CclSetHoldClickDelay);

    //
    //	Look and feel of units
    //
    gh_new_procedure1_0("set-selection-style!",CclSetSelectionStyle);
    gh_new_procedure1_0("set-show-sight-range!",CclSetShowSightRange);
    gh_new_procedure1_0("set-show-reaction-range!",CclSetShowReactionRange);
    gh_new_procedure1_0("set-show-attack-range!",CclSetShowAttackRange);
    gh_new_procedure1_0("set-show-orders!",CclSetShowOrders);

    //
    //	Keystroke helps
    //
    gh_new_procedure0_0("reset-keystroke-help",CclResetKeystrokeHelp);
    gh_new_procedureN("add-keystroke-help",CclAddKeystrokeHelp);

    InitMenuFuncHash();
}

//@}
