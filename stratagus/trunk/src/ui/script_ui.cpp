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
//	(c) Copyright 1999-2001 by Lutz Sammer
//
//	FreeCraft is free software; you can redistribute it and/or modify
//	it under the terms of the GNU General Public License as published
//	by the Free Software Foundation; either version 2 of the License,
//	or (at your option) any later version.
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
	CclFree(s1);
	CclFree(s2);
    } else {
	ct=calloc(i+2,sizeof(CursorType));
	memcpy(ct,Cursors,sizeof(CursorType)*i);
	CclFree(Cursors);
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
	    CclFree(ct->File);
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
local SCM CclGameCursor(SCM ident)
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
#if 1
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
#else
    ui->Contrast=100;
    ui->Brightness=0;
    ui->Saturation=100;

    ui->MouseScroll=1;
    ui->ReverseMouseMove=0;

    ui->WarpX=-1;
    ui->WarpY=-1;

    ui->MouseAdjust=MOUSEADJUST;
    ui->MouseScale=MOUSESCALE;

    ui->OriginalResources=0;
#endif

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

    //
    //	Get the cursors definitions.
    //
    temp=gh_car(list);
    list=gh_cdr(list);

    value=gh_car(temp);
    temp=gh_cdr(temp);
    str=gh_scm2newstr(value,NULL);
    CclFree(ui->Point.Name);
    ui->Point.Name=str;

    value=gh_car(temp);
    temp=gh_cdr(temp);
    str=gh_scm2newstr(value,NULL);
    CclFree(ui->Glass.Name);
    ui->Glass.Name=str;

    value=gh_car(temp);
    temp=gh_cdr(temp);
    str=gh_scm2newstr(value,NULL);
    CclFree(ui->Cross.Name);
    ui->Cross.Name=str;

    value=gh_car(temp);
    temp=gh_cdr(temp);
    str=gh_scm2newstr(value,NULL);
    CclFree(ui->YellowHair.Name);
    ui->YellowHair.Name=str;

    value=gh_car(temp);
    temp=gh_cdr(temp);
    str=gh_scm2newstr(value,NULL);
    CclFree(ui->GreenHair.Name);
    ui->GreenHair.Name=str;

    value=gh_car(temp);
    temp=gh_cdr(temp);
    str=gh_scm2newstr(value,NULL);
    CclFree(ui->RedHair.Name);
    ui->RedHair.Name=str;

    value=gh_car(temp);
    temp=gh_cdr(temp);
    str=gh_scm2newstr(value,NULL);
    CclFree(ui->Scroll.Name);
    ui->Scroll.Name=str;

    value=gh_car(temp);
    temp=gh_cdr(temp);
    str=gh_scm2newstr(value,NULL);
    CclFree(ui->ArrowE.Name);
    ui->ArrowE.Name=str;

    value=gh_car(temp);
    temp=gh_cdr(temp);
    str=gh_scm2newstr(value,NULL);
    CclFree(ui->ArrowNE.Name);
    ui->ArrowNE.Name=str;

    value=gh_car(temp);
    temp=gh_cdr(temp);
    str=gh_scm2newstr(value,NULL);
    CclFree(ui->ArrowN.Name);
    ui->ArrowN.Name=str;

    value=gh_car(temp);
    temp=gh_cdr(temp);
    str=gh_scm2newstr(value,NULL);
    CclFree(ui->ArrowNW.Name);
    ui->ArrowNW.Name=str;

    value=gh_car(temp);
    temp=gh_cdr(temp);
    str=gh_scm2newstr(value,NULL);
    CclFree(ui->ArrowW.Name);
    ui->ArrowW.Name=str;

    value=gh_car(temp);
    temp=gh_cdr(temp);
    str=gh_scm2newstr(value,NULL);
    CclFree(ui->ArrowSW.Name);
    ui->ArrowSW.Name=str;

    value=gh_car(temp);
    temp=gh_cdr(temp);
    str=gh_scm2newstr(value,NULL);
    CclFree(ui->ArrowS.Name);
    ui->ArrowS.Name=str;

    value=gh_car(temp);
    temp=gh_cdr(temp);
    str=gh_scm2newstr(value,NULL);
    CclFree(ui->ArrowSE.Name);
    ui->ArrowSE.Name=str;

    //
    //	Panels
    //
    temp=gh_car(list);
    list=gh_cdr(list);

    value=gh_car(temp);
    temp=gh_cdr(temp);
    str=gh_scm2newstr(value,NULL);
    CclFree(ui->GameMenuePanel.File);
    ui->GameMenuePanel.File=str;

    temp=gh_car(list);
    list=gh_cdr(list);

    value=gh_car(temp);
    temp=gh_cdr(temp);
    str=gh_scm2newstr(value,NULL);
    CclFree(ui->Menue1Panel.File);
    ui->Menue1Panel.File=str;

    temp=gh_car(list);
    list=gh_cdr(list);

    value=gh_car(temp);
    temp=gh_cdr(temp);
    str=gh_scm2newstr(value,NULL);
    CclFree(ui->Menue2Panel.File);
    ui->Menue2Panel.File=str;

    temp=gh_car(list);
    list=gh_cdr(list);

    value=gh_car(temp);
    temp=gh_cdr(temp);
    str=gh_scm2newstr(value,NULL);
    CclFree(ui->VictoryPanel.File);
    ui->VictoryPanel.File=str;

    temp=gh_car(list);
    list=gh_cdr(list);

    value=gh_car(temp);
    temp=gh_cdr(temp);
    str=gh_scm2newstr(value,NULL);
    CclFree(ui->ScenarioPanel.File);
    ui->ScenarioPanel.File=str;

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

    gh_new_procedureN("define-cursor",CclDefineCursor);
    gh_new_procedure1_0("game-cursor",CclGameCursor);
    gh_new_procedureN("define-ui",CclDefineUI);

    gh_new_procedure1_0("key-scroll-speed", CclKeyScrollSpeed);
    gh_new_procedure1_0("mouse-scroll-speed", CclMouseScrollSpeed);

    gh_new_procedure0_0("show-command-key",CclShowCommandKey);
    gh_new_procedure0_0("right-button-attacks",CclRightButtonAttacks);
    gh_new_procedure0_0("right-button-moves",CclRightButtonMoves);
    gh_new_procedure0_0("fancy-buildings",CclFancyBuildings);

    gh_new_procedureN("define-button",CclDefineButton);

    //
    //	Correct named functions
    //
    gh_new_procedure1_0("set-double-click-delay!",CclSetDoubleClickDelay);
    gh_new_procedure1_0("set-hold-click-delay!",CclSetHoldClickDelay);
}

//@}
