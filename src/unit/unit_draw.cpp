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
/**@name unit_draw.c	-	The draw routines for units. */
//
//	(c) Copyright 1998-2002 by Lutz Sammer
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

#include "freecraft.h"
#include "video.h"
#include "sound_id.h"
#include "unitsound.h"
#include "unittype.h"
#include "player.h"
#include "unit.h"
#include "tileset.h"
#include "map.h"
#include "construct.h"
#include "cursor.h"
#include "interface.h"
#include "font.h"
#include "ui.h"
#include "ccl.h"

#include "etlib/generic.h"

/*----------------------------------------------------------------------------
--	Variables
----------------------------------------------------------------------------*/

global int ShowHealthBar;		/// Flag: show health bar
global int ShowHealthDot;		/// Flag: show health dot
global int ShowManaBar;			/// Flag: show mana bar
global int ShowManaDot;			/// Flag: show mana dot
global int ShowNoFull;			/// Flag: show no full health or mana
global int DecorationOnTop;		/// Flag: show health and mana on top
global int ShowSightRange;		/// Flag: show right range
global int ShowReactionRange;		/// Flag: show reaction range
global int ShowAttackRange;		/// Flag: show attack range
global int ShowOrders;			/// Flag: show orders of unit on map
global int ShowOrdersCount;		/// Show orders for some time
    /// Flag: health horizontal instead of vertical
global int ShowHealthHorizontal;
    /// Flag: health horizontal instead of vertical
global int ShowManaHorizontal;
    /// Flag: show bars and dot energy only for selected
global int ShowEnergySelectedOnly;
    /// Flag: show the health background long
global int ShowHealthBackgroundLong;
    /// Flag: show the mana background long
global int ShowManaBackgroundLong;

// FIXME: not all variables of this file are here
// FIXME: perhaps split this file into two or three parts?

/**
**	Show that units are selected.
**
**	@param unit	Pointer to the unit.
**	@param type	Type of the unit.
**	@param x	Screen X position of the unit.
**	@param y	Screen Y position of the unit.
*/
global void (*DrawSelection)(const Unit* unit,const UnitType* type,int x,int y)
	=DrawSelectionNone;

/*----------------------------------------------------------------------------
--	Functions
----------------------------------------------------------------------------*/

// FIXME: clean split screen support
// FIXME: integrate this with global versions of these functions in map.c

local int CurrentViewport;		/// FIXME: quick hack for split screen

#define Map2ScreenX	Map2ScreenXV	/// FIXME: quick hack for split screen
#define Map2ScreenY	Map2ScreenYV	/// FIXME: quick hack for split screen

/// FIXME: quick hack for split screen
local inline int Map2ScreenXV(int x)
{
    return (TheUI.VP[CurrentViewport].X + ((x) -
	    TheUI.VP[CurrentViewport].MapX) * TileSizeX);
}

/// FIXME: quick hack for split screen
local inline int Map2ScreenYV(int y)
{
    return (TheUI.VP[CurrentViewport].Y + ((y) -
	    TheUI.VP[CurrentViewport].MapY) * TileSizeY);
}

// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

/**
**	Choose color for selection.
**
**	@param unit	Pointer to the unit.
**	@param type	Type of the unit.
**
**	@return		Color for selection, or -1 if not selected.
*/
local int SelectionColor(const Unit* unit,const UnitType* type)
{
    if( unit->Selected || (unit->Blink&1) ) {
	if( unit->Player->Player==PlayerNumNeutral ) {
	    return ColorYellow;
	}
	// FIXME: better allied?
	if( unit->Player==ThisPlayer ) {
	    return ColorGreen;
	}
	if( IsEnemy(ThisPlayer,unit) ) {
	    return ColorRed;
	}
	return unit->Player->Color;
    }

    // If building mark all own buildings
    if( CursorBuilding && type->Building && unit->Player==ThisPlayer ) {
	return ColorGray;
    }
    return -1;
}

/**
**	Don't show selected units.
**
**	@param unit	Pointer to the unit.
**	@param type	Type of the unit.
**	@param x	Screen X position of the unit.
**	@param y	Screen Y position of the unit.
*/
global void DrawSelectionNone(const Unit* unit __attribute__((unused)),
	const UnitType* type __attribute__((unused)),
	int x __attribute__((unused)),int y __attribute__((unused)))
{
}

/**
**	Show selected units with circle.
**
**	@param unit	Pointer to the unit.
**	@param type	Type of the unit.
**	@param x	Screen X position of the unit.
**	@param y	Screen Y position of the unit.
*/
global void DrawSelectionCircle(const Unit* unit,const UnitType* type
	,int x,int y)
{
    int color;

    //
    //	Select color for the circle.
    //
    if( (color=SelectionColor(unit,type))<0 ) {
	return;
    }
    VideoDrawCircleClip(color
	    ,x+type->TileWidth*TileSizeX/2
	    ,y+type->TileHeight*TileSizeY/2
	    ,min(type->BoxWidth,type->BoxHeight)/2);

    VideoDrawCircleClip(color
	    ,x+type->TileWidth*TileSizeX/2
	    ,y+type->TileHeight*TileSizeY/2
	    ,min(type->BoxWidth+2,type->BoxHeight+2)/2);
}

/**
**	Show selected units with circle.
**
**	@param unit	Pointer to the unit.
**	@param type	Type of the unit.
**	@param x	Screen X position of the unit.
**	@param y	Screen Y position of the unit.
*/
global void DrawSelectionCircleWithTrans(const Unit* unit,const UnitType* type
	,int x,int y)
{
    int color;

    //
    //	Select color for the circle.
    //
    if( (color=SelectionColor(unit,type))<0 ) {
	return;
    }
    VideoDrawCircleClip(color
	    ,x+type->TileWidth*TileSizeX/2
	    ,y+type->TileHeight*TileSizeY/2
	    ,min(type->BoxWidth,type->BoxHeight)/2);

    VideoFill75TransCircleClip(color
	    ,x+type->TileWidth*TileSizeX/2
	    ,y+type->TileHeight*TileSizeY/2
	    ,min(type->BoxWidth-2,type->BoxHeight-2)/2);
}

/**
**	Draw selected rectangle around the unit.
**
**	@param unit	Pointer to the unit.
**	@param type	Type of the unit.
**	@param x	Screen X position of the unit.
**	@param y	Screen Y position of the unit.
*/
global void DrawSelectionRectangle(const Unit* unit,const UnitType* type
	,int x,int y)
{
    int color;

    //
    //	Select color for the rectangle
    //
    if( (color=SelectionColor(unit,type))<0 ) {
	return;
    }

    VideoDrawRectangleClip(color
	    ,x+(type->TileWidth*TileSizeX-type->BoxWidth)/2
	    ,y+(type->TileHeight*TileSizeY-type->BoxHeight)/2
	    ,type->BoxWidth
	    ,type->BoxHeight);
}

/**
**	Draw selected rectangle around the unit.
**
**	@param unit	Pointer to the unit.
**	@param type	Type of the unit.
**	@param x	Screen X position of the unit.
**	@param y	Screen Y position of the unit.
*/
global void DrawSelectionRectangleWithTrans(const Unit* unit
	,const UnitType* type,int x,int y)
{
    int color;

    //
    //	Select color for the rectangle
    //
    if( (color=SelectionColor(unit,type))<0 ) {
	return;
    }

    VideoDrawRectangleClip(color
	    ,x+(type->TileWidth*TileSizeX-type->BoxWidth)/2
	    ,y+(type->TileHeight*TileSizeY-type->BoxHeight)/2
	    ,type->BoxWidth
	    ,type->BoxHeight);
    VideoFill75TransRectangleClip(color
	    ,x+1+(type->TileWidth*TileSizeX-type->BoxWidth)/2
	    ,y+1+(type->TileHeight*TileSizeY-type->BoxHeight)/2
	    ,type->BoxWidth-2
	    ,type->BoxHeight-2);
}

/**
**	Draw selected corners around the unit.
**
**	@param unit	Pointer to the unit.
**	@param type	Type of the unit.
**	@param x	Screen X position of the unit.
**	@param y	Screen Y position of the unit.
*/
global void DrawSelectionCorners(const Unit* unit,const UnitType* type
	,int x,int y)
{
    int color;
#define CORNER_PIXELS 6

    //
    //	Select color for the rectangle
    //
    if( (color=SelectionColor(unit,type))<0 ) {
	return;
    }

    VideoDrawVLineClip(color
	    ,x+(type->TileWidth*TileSizeX-type->BoxWidth)/2
	    ,y+(type->TileHeight*TileSizeY-type->BoxHeight)/2
	    ,CORNER_PIXELS);
    VideoDrawHLineClip(color
	    ,x+(type->TileWidth*TileSizeX-type->BoxWidth)/2+1
	    ,y+(type->TileHeight*TileSizeY-type->BoxHeight)/2
	    ,CORNER_PIXELS-1);

    VideoDrawVLineClip(color
	    ,x+(type->TileWidth*TileSizeX-type->BoxWidth)/2+type->BoxWidth
	    ,y+(type->TileHeight*TileSizeY-type->BoxHeight)/2
	    ,CORNER_PIXELS);
    VideoDrawHLineClip(color
	    ,x+(type->TileWidth*TileSizeX-type->BoxWidth)/2+type->BoxWidth
		-CORNER_PIXELS+1
	    ,y+(type->TileHeight*TileSizeY-type->BoxHeight)/2
	    ,CORNER_PIXELS-1);

    VideoDrawVLineClip(color
	    ,x+(type->TileWidth*TileSizeX-type->BoxWidth)/2
	    ,y+(type->TileHeight*TileSizeY-type->BoxHeight)/2+type->BoxHeight
		-CORNER_PIXELS+1
	    ,CORNER_PIXELS);
    VideoDrawHLineClip(color
	    ,x+(type->TileWidth*TileSizeX-type->BoxWidth)/2+1
	    ,y+(type->TileHeight*TileSizeY-type->BoxHeight)/2+type->BoxHeight
	    ,CORNER_PIXELS-1);

    VideoDrawVLineClip(color
	    ,x+(type->TileWidth*TileSizeX-type->BoxWidth)/2+type->BoxWidth
	    ,y+(type->TileHeight*TileSizeY-type->BoxHeight)/2+type->BoxHeight
		-CORNER_PIXELS+1
	    ,CORNER_PIXELS);
    VideoDrawHLineClip(color
	    ,x+(type->TileWidth*TileSizeX-type->BoxWidth)/2+type->BoxWidth
		-CORNER_PIXELS+1
	    ,y+(type->TileHeight*TileSizeY-type->BoxHeight)/2+type->BoxHeight
	    ,CORNER_PIXELS-1);
}

/**
**	Decoration: health, mana.
*/
typedef struct _decoration_ {
    char*	File;			/// File containing the graphics data
    int		HotX;			/// X drawing position (relative)
    int		HotY;			/// Y drawing position (relative)
    int		Width;			/// width of the decoration
    int		Height;			/// height of the decoration

// --- FILLED UP ---
    Graphic*	Sprite;			/// loaded sprite images
} Decoration;

/**
**	Sprite to display the mana.
*/
global Decoration ManaSprite;

/**
**	Sprite to display the health.
*/
global Decoration HealthSprite;

/**
**	Sprite to display as the shadow of flying units.
**
**	@todo	Made this configurable with CCL.
*/
global Decoration ShadowSprite;

/**
**	Sprite to display the active spells on an unit.
*/
global Decoration SpellSprite;

/**
**	Define mana sprite.
**
**	@param file	Mana graphic file.
**	@param x	Mana X position.
**	@param y	Mana Y position.
**	@param w	Mana width.
**	@param h	Mana height.
*/
global SCM CclManaSprite(SCM file,SCM x,SCM y,SCM w,SCM h)
{
    free(ManaSprite.File);

    ManaSprite.File=gh_scm2newstr(file,NULL);
    ManaSprite.HotX=gh_scm2int(x);
    ManaSprite.HotY=gh_scm2int(y);
    ManaSprite.Width=gh_scm2int(w);
    ManaSprite.Height=gh_scm2int(h);

    return SCM_UNSPECIFIED;
}

/**
**	Define health sprite.
**
**	@param file	Health graphic file.
**	@param x	Health X position.
**	@param y	Health Y position.
**	@param w	Health width.
**	@param h	Health height.
*/
global SCM CclHealthSprite(SCM file,SCM x,SCM y,SCM w,SCM h)
{
    free(HealthSprite.File);

    HealthSprite.File=gh_scm2newstr(file,NULL);
    HealthSprite.HotX=gh_scm2int(x);
    HealthSprite.HotY=gh_scm2int(y);
    HealthSprite.Width=gh_scm2int(w);
    HealthSprite.Height=gh_scm2int(h);

    return SCM_UNSPECIFIED;
}

/**
**	Define health sprite.
**
**	@param file	Shadow graphic file.
**	@param x	Shadow X position.
**	@param y	Shadow Y position.
**	@param w	Shadow width.
**	@param h	Shadow height.
*/
global SCM CclShadowSprite(SCM file,SCM x,SCM y,SCM w,SCM h)
{
    free(ShadowSprite.File);

    ShadowSprite.File=gh_scm2newstr(file,NULL);
    ShadowSprite.HotX=gh_scm2int(x);
    ShadowSprite.HotY=gh_scm2int(y);
    ShadowSprite.Width=gh_scm2int(w);
    ShadowSprite.Height=gh_scm2int(h);

    return SCM_UNSPECIFIED;
}

/**
**	Define health sprite.
**
**	@param file	Spell graphic file.
**	@param x	Spell X position.
**	@param y	Spell Y position.
**	@param w	Spell width.
**	@param h	Spell height.
*/
global SCM CclSpellSprite(SCM file,SCM x,SCM y,SCM w,SCM h)
{
    free(SpellSprite.File);

    SpellSprite.File=gh_scm2newstr(file,NULL);
    SpellSprite.HotX=gh_scm2int(x);
    SpellSprite.HotY=gh_scm2int(y);
    SpellSprite.Width=gh_scm2int(w);
    SpellSprite.Height=gh_scm2int(h);

    return SCM_UNSPECIFIED;
}

/**
**	Enable display health as health-bar.
*/
local SCM CclShowHealthBar(void)
{
    ShowHealthBar=1;
    ShowHealthDot=0;

    return SCM_UNSPECIFIED;
}

/**
**	Enable display health as health-dot.
*/
local SCM CclShowHealthDot(void)
{
    ShowHealthBar=0;
    ShowHealthDot=1;

    return SCM_UNSPECIFIED;
}

/**
**	Enable display health as horizontal bar.
*/
local SCM CclShowHealthHorizontal(void)
{
    ShowHealthBar=1;
    ShowHealthDot=0;
    ShowHealthHorizontal=1;

    return SCM_UNSPECIFIED;
}

/**
**	Enable display health as vertical bar.
*/
local SCM CclShowHealthVertical(void)
{
    ShowHealthBar=1;
    ShowHealthDot=0;
    ShowHealthHorizontal=0;

    return SCM_UNSPECIFIED;
}

/**
**	Enable display mana as mana-bar.
*/
local SCM CclShowManaBar(void)
{
    ShowManaBar=1;
    ShowManaDot=0;

    return SCM_UNSPECIFIED;
}

/**
**	Enable display mana as mana-dot.
*/
local SCM CclShowManaDot(void)
{
    ShowManaBar=0;
    ShowManaDot=1;

    return SCM_UNSPECIFIED;
}

/**
**	Enable energy bars and dots only for selected units
*/
local SCM CclShowEnergySelected(void)
{
    ShowEnergySelectedOnly=1;

    return SCM_UNSPECIFIED;
}


/**
**	Enable display of full bars/dots.
*/
local SCM CclShowFull(void)
{
    ShowNoFull=0;

    return SCM_UNSPECIFIED;
}

/**
**	Enable display mana as horizontal bar.
*/
local SCM CclShowManaHorizontal(void)
{
    ShowManaBar=1;
    ShowManaDot=0;
    ShowManaHorizontal=1;

    return SCM_UNSPECIFIED;
}

/**
**	Enable display mana as vertical bar.
*/
local SCM CclShowManaVertical(void)
{
    ShowManaBar=1;
    ShowManaDot=0;
    ShowManaHorizontal=0;

    return SCM_UNSPECIFIED;
}

/**
**	Disable display of full bars/dots.
*/
local SCM CclShowNoFull(void)
{
    ShowNoFull=1;

    return SCM_UNSPECIFIED;
}

/**
**	Draw decorations always on top.
*/
local SCM CclDecorationOnTop(void)
{
    DecorationOnTop=1;

    return SCM_UNSPECIFIED;
}

/**
**	Register CCL features for decorations.
*/
global void DecorationCclRegister(void)
{
    gh_new_procedure5_0("mana-sprite",CclManaSprite);
    gh_new_procedure5_0("health-sprite",CclHealthSprite);
    gh_new_procedure5_0("shadow-sprite",CclShadowSprite);
    gh_new_procedure5_0("spell-sprite",CclSpellSprite);

    gh_new_procedure0_0("show-health-bar",CclShowHealthBar);
    gh_new_procedure0_0("show-health-dot",CclShowHealthDot);
// adicionado por protoman
    gh_new_procedure0_0("show-health-vertical",CclShowHealthVertical);
    gh_new_procedure0_0("show-health-horizontal",CclShowHealthHorizontal);
    gh_new_procedure0_0("show-mana-vertical",CclShowManaVertical);
    gh_new_procedure0_0("show-mana-horizontal",CclShowManaHorizontal);
// fim

    gh_new_procedure0_0("show-mana-bar",CclShowManaBar);
    gh_new_procedure0_0("show-mana-dot",CclShowManaDot);
    gh_new_procedure0_0("show-energy-selected-only",CclShowEnergySelected);
    gh_new_procedure0_0("show-full",CclShowFull);
    gh_new_procedure0_0("show-no-full",CclShowNoFull);
    gh_new_procedure0_0("decoration-on-top",CclDecorationOnTop);
}

/**
**	Load decoration.
*/
global void LoadDecorations(void)
{
    if( HealthSprite.File ) {
	ShowLoadProgress("\tDecorations `%s'\n",HealthSprite.File);
	HealthSprite.Sprite=LoadSprite(HealthSprite.File
		,HealthSprite.Width,HealthSprite.Height);
    }
    if( ManaSprite.File ) {
	ShowLoadProgress("\tDecorations `%s'\n",ManaSprite.File);
	ManaSprite.Sprite=LoadSprite(ManaSprite.File
		,ManaSprite.Width,ManaSprite.Height);
    }
    if( ShadowSprite.File ) {
	ShowLoadProgress("\tDecorations `%s'\n",ShadowSprite.File);
	ShadowSprite.Sprite=LoadSprite(ShadowSprite.File
		,ShadowSprite.Width,ShadowSprite.Height);
    }
    if( SpellSprite.File ) {
	ShowLoadProgress("\tDecorations `%s'\n",SpellSprite.File);
	SpellSprite.Sprite=LoadSprite(SpellSprite.File
		,SpellSprite.Width,SpellSprite.Height);
    }
}

/**
**	Save decorations.
*/
global void SaveDecorations(FILE* file)
{
    fprintf(file,"\n;;; -----------------------------------------\n");
    fprintf(file,";;; MODULE: decorations $Id$\n\n");

    fprintf(file,"(mana-sprite \"%s\"  %d %d  %d %d)\n",
	ManaSprite.File,ManaSprite.HotX,ManaSprite.HotY,
	ManaSprite.Width,ManaSprite.Height);
    fprintf(file,"(health-sprite \"%s\"  %d %d  %d %d)\n",
	HealthSprite.File,HealthSprite.HotX,HealthSprite.HotY,
	HealthSprite.Width,HealthSprite.Height);
    fprintf(file,"(shadow-sprite \"%s\"  %d %d  %d %d)\n",
	ShadowSprite.File,ShadowSprite.HotX,ShadowSprite.HotY,
	ShadowSprite.Width,ShadowSprite.Height);
    fprintf(file,"(spell-sprite \"%s\"  %d %d  %d %d)\n",
	SpellSprite.File,SpellSprite.HotX,SpellSprite.HotY,
	SpellSprite.Width,SpellSprite.Height);

    // This belongs to the config and not save file
    if( ShowHealthBar ) {
	fprintf(file,";(show-health-bar)\n");
    }
    if( ShowHealthDot ) {
	fprintf(file,";(show-health-dot)\n");
    }
    if( ShowHealthHorizontal ) {
	fprintf(file,";(show-health-horizontal)\n");
    } else {
	fprintf(file,";(show-health-vertical)\n");
    }
    if( ShowHealthBackgroundLong ) {
	fprintf(file,";(show-health-blackground-long)\n");
    }
    if( ShowManaBar ) {
	fprintf(file,";(show-mana-bar)\n");
    }
    if( ShowManaDot ) {
	fprintf(file,";(show-mana-dot)\n");
    }
    if( ShowManaHorizontal ) {
	fprintf(file,";(show-mana-horizontal)\n");
    } else {
	fprintf(file,";(show-mana-vertical)\n");
    }
    if( ShowManaBackgroundLong ) {
	fprintf(file,";(show-mana-blackground-long)\n");
    }
    if( ShowEnergySelectedOnly ) {
	fprintf(file,";(show-energy-selected-only)\n");
    }
    if( ShowNoFull ) {
	fprintf(file,";(show-no-full)\n");
    } else {
	fprintf(file,";(show-full)\n");
    }
    if( DecorationOnTop ) {
	fprintf(file,";(decoration-on-top)\n");
    }
}

/**
**	Clean decorations.
*/
global void CleanDecorations(void)
{
    if( HealthSprite.File ) {
	free(HealthSprite.File);
    }
    VideoSaveFree(HealthSprite.Sprite);
    HealthSprite.File=NULL;
    HealthSprite.Sprite=NULL;

    if( ManaSprite.File ) {
	free(ManaSprite.File);
    }
    VideoSaveFree(ManaSprite.Sprite);
    ManaSprite.File=NULL;
    ManaSprite.Sprite=NULL;

    if( ShadowSprite.File ) {
	free(ShadowSprite.File);
    }
    VideoSaveFree(ShadowSprite.Sprite);
    ShadowSprite.File=NULL;
    ShadowSprite.Sprite=NULL;

    if( SpellSprite.File ) {
	free(SpellSprite.File);
    }
    VideoSaveFree(SpellSprite.Sprite);
    SpellSprite.File=NULL;
    SpellSprite.Sprite=NULL;
}

/**
**	Draw mana/working sprite.
**
**	@param x	X screen pixel position
**	@param y	Y screen pixel position
**	@param type	Unit type pointer
**	@param full	Full value
**	@param ready	Ready value
*/
local void DrawManaSprite(int x,int y,const UnitType* type,int full,int ready)
{
    int n;

    n=VideoGraphicFrames(ManaSprite.Sprite)-1;
    n-=(n*ready)/full;

    DebugCheck( n<0 || n>=VideoGraphicFrames(ManaSprite.Sprite)) ;
    if( ManaSprite.HotX<0 ) {
	x+=ManaSprite.HotX
		+(type->TileWidth*TileSizeX+type->BoxWidth+1)/2;
    } else if( ManaSprite.HotX>0 ) {
	x+=1-ManaSprite.HotX
		+(type->TileWidth*TileSizeX-type->BoxWidth)/2;
    } else {
	x+=(type->TileWidth*TileSizeX-ManaSprite.Width+1)/2;
    }
    if( ManaSprite.HotY<0 ) {
	y+=ManaSprite.HotY
		+(type->TileHeight*TileSizeY+type->BoxHeight+1)/2;
    } else if( ManaSprite.HotY>0 ) {
	y+=1-ManaSprite.HotY
		+(type->TileHeight*TileSizeY-type->BoxHeight)/2;
    } else {
	y+=(type->TileHeight*TileSizeY-ManaSprite.Height+1)/2;
    }
    VideoDrawClip(ManaSprite.Sprite,n,x,y);
}

/**
**	Draw mana/working bar.
**
**	@param x	X screen pixel position
**	@param y	Y screen pixel position
**	@param type	Unit type pointer
**	@param full	Full value
**	@param ready	Ready value
*/
local void DrawManaBar(int x,int y,const UnitType* type,int full,int ready)
{
    int f, w;

    f=(100*ready)/full;
    if ( ShowManaHorizontal == 0)  {
	VideoFillRectangleClip(ColorBlue
		,x+(type->TileWidth*TileSizeX
			+type->BoxWidth)/2
		,y+(type->TileHeight*TileSizeY
			-type->BoxHeight)/2
		,2,(f*type->BoxHeight)/100);
    }  else  {
	//
	//	Draw the black rectangle in full size?
	//
	if( ShowManaBackgroundLong ) {
	    VideoFillRectangleClip(ColorBlack
		,x+((type->TileWidth*TileSizeX-type->BoxWidth)/2)
		,(y+(type->TileHeight*TileSizeY-type->BoxHeight)/2)
			+type->BoxHeight+5
		,(type->BoxHeight)+1
		,5);
	} else {
	    VideoDrawRectangleClip(ColorBlack
		,x+((type->TileWidth*TileSizeX-type->BoxWidth)/2)
		,(y+(type->TileHeight*TileSizeY-type->BoxHeight)/2)
			+type->BoxHeight+5
		,(f*type->BoxHeight)/100
		,4);
	}
        w=(f*type->BoxHeight)/100-1;
        if (  w > 0 ) // Prevents -1 turning into unsigned int
	    VideoFillRectangleClip(ColorBlue
		,x+(type->TileWidth*TileSizeX-type->BoxWidth)/2+1
		,(y+(type->TileHeight*TileSizeY-type->BoxHeight)/2)
			+type->BoxHeight+6
		,w
		,3);
    }
}

/**
**	Draw decoration (invis, for the unit.)
**
**	@param unit	Pointer to the unit.
**	@param type	Type of the unit.
**	@param x	Screen X position of the unit.
**	@param y	Screen Y position of the unit.
*/
local void DrawDecoration(const Unit* unit,const UnitType* type,int x,int y)
{
    int f;
    int color;
    int w;
    int x1;
    int y1;
    const UnitStats* stats;

#ifdef REFS_DEBUG
    //
    //	Show the number of references.
    //
    VideoDrawNumberClip(x+1,y+1,GameFont,unit->Refs);
#endif

    //
    //	Only for selected units?
    //
    if( ShowEnergySelectedOnly && !unit->Selected ) {
	return;
    }

    //
    //	Health bar on left side of unit.
    //
    stats=unit->Stats;
    if( (!type->Critter || unit->Player->Type!=PlayerRaceNeutral)
		&& ShowHealthBar ) {
	if( stats->HitPoints
		&& !(ShowNoFull && unit->HP==stats->HitPoints) ) {
	    f=(100*unit->HP)/stats->HitPoints;
	    if( f>75) {
		color=ColorDarkGreen;
	    } else if( f>50 ) {
		color=ColorYellow;
	    } else if( f>25 ) {
		color=ColorOrange;
	    } else {
		color=ColorRed;
	    }
	    if ( ShowHealthHorizontal )  {
		//
		//	Draw the black rectangle in full size?
		//
		if( ShowHealthBackgroundLong ) {
#if defined(DEBUG)
		    // Johns: I want to see fast moving.
		    //VideoFillRectangleClip(unit->Data.Move.Fast
		    // Johns: I want to see the AI active flag
		    VideoFillRectangleClip(unit->Active
			    ? ColorBlack : ColorWhite
#else
		    VideoFillRectangleClip(ColorBlack
#endif
			,x+((type->TileWidth*TileSizeX-type->BoxWidth)/2)
			,(y+(type->TileHeight*TileSizeY-type->BoxHeight)/2)
				+type->BoxHeight+1
			,type->BoxHeight+1
			,5);
		} else {
#if defined(DEBUG)
		    // Johns: I want to see fast moving.
		    VideoFillRectangleClip(unit->Data.Move.Fast
			    ? ColorBlack : ColorWhite
#else
		    VideoFillRectangleClip(ColorBlack
#endif
			,x+((type->TileWidth*TileSizeX-type->BoxWidth)/2)
			,(y+(type->TileHeight*TileSizeY-type->BoxHeight)/2)
				+type->BoxHeight+1
			,((f*type->BoxHeight)/100)+1
			,5);
		}
              w = (f*type->BoxHeight)/100-1;
              if ( w > 0 ) // Prevents -1 turning into unsigned int
		VideoFillRectangleClip(color
		    ,x+((type->TileWidth*TileSizeX-type->BoxWidth)/2)+1
		    ,(y+(type->TileHeight*TileSizeY-type->BoxHeight)/2)
			    +type->BoxHeight+2
		    ,w
		    ,3);
	    }  else  {
		VideoFillRectangleClip(color
		    ,x+(type->TileWidth*TileSizeX
			    -type->BoxWidth)/2
		    ,y+(type->TileHeight*TileSizeY
			    -type->BoxHeight)/2
		    ,2,(f*type->BoxHeight)/100);
	    }
	}
    }

    //
    //	Health dot on left side of unit.
    //
    if( (!type->Critter || unit->Player->Type!=PlayerRaceNeutral)
		&& ShowHealthDot ) {
	if( stats->HitPoints
		&& !(ShowNoFull && unit->HP==stats->HitPoints) ) {
	    int n;

	    n=VideoGraphicFrames(HealthSprite.Sprite)-1;
	    n-=(n*unit->HP)/stats->HitPoints;
#if 0
	    f=(100*unit->HP)/stats->HitPoints;
	    if( f>75) {
		n=3-((f-75)/(25/3))+ 0;
	    } else if( f>50 ) {
		n=3-((f-50)/(25/3))+ 4;
		DebugLevel3("%d - %d\n" _C_ f _C_ n);
	    } else {
		n=3-(f/(50/3))+ 8;
		DebugLevel3("%d - %d\n" _C_ f _C_ n);
	    }
#endif
	    DebugCheck( n<0 );
	    if( HealthSprite.HotX<0 ) {
		x1=x+HealthSprite.HotX
			+(type->TileWidth*TileSizeX+type->BoxWidth+1)/2;
	    } else if( HealthSprite.HotX>0 ) {
		x1=x+1-HealthSprite.HotX
			+(type->TileWidth*TileSizeX-type->BoxWidth)/2;
	    } else {
		x1=x+(type->TileWidth*TileSizeX-HealthSprite.Width+1)/2;
	    }
	    if( HealthSprite.HotY<0 ) {
		y1=y+HealthSprite.HotY
			+(type->TileHeight*TileSizeY+type->BoxHeight+1)/2;
	    } else if( HealthSprite.HotY>0 ) {
		y1=y+1-HealthSprite.HotY
			+(type->TileHeight*TileSizeY-type->BoxHeight)/2;
	    } else {
		y1=y+(type->TileHeight*TileSizeY-HealthSprite.Height+1)/2;
	    }
	    VideoDrawClip(HealthSprite.Sprite,n,x1,y1);
	}
    }

    //
    //	Mana bar on right side of unit. FIXME: combine bar and sprite
    //
    if( ShowManaBar ) {
	    /* s0m3body: mana bar should display man proportionally to unit's max mana (unit->Type->_MaxMana) */
	if( type->CanCastSpell && !(ShowNoFull && unit->Mana==unit->Type->_MaxMana) ) {
	    DrawManaBar(x,y,type,unit->Type->_MaxMana,unit->Mana);
	} else if( type->GivesOil || type->GoldMine || type->OilPatch ) {
	    DrawManaBar(x,y,type,655350,unit->Value);
	}
	//
	//	Show working of units.
	//
	if( unit->Player==ThisPlayer ) {

	    //
	    //	Building under constuction.
	    //
	    /*
	    if( unit->Orders[0].Action==UnitActionBuilded ) {
		DrawManaBar(x,y,type,stats->HitPoints,unit->HP);
	    } else
	    */

	    //
	    //	Building training units.
	    //
	    if( unit->Orders[0].Action==UnitActionTrain ) {
		DrawManaBar(x,y,type,unit->Data.Train.What[0]
			    ->Stats[unit->Player->Player].Costs[TimeCost]
			,unit->Data.Train.Ticks);

	    //
	    //	Building upgrading to better type.
	    //
	    } else if( unit->Orders[0].Action==UnitActionUpgradeTo ) {
		DrawManaBar(x,y,type,unit->Orders[0].Type
			    ->Stats[unit->Player->Player].Costs[TimeCost]
			,unit->Data.UpgradeTo.Ticks);

	    //
	    //	Building research new technologie.
	    //
	    } else if( unit->Orders[0].Action==UnitActionResearch ) {
		DrawManaBar(x,y,type,unit->Data.Research.Upgrade
			    ->Costs[TimeCost],
			unit->Player->UpgradeTimers.Upgrades[
			    unit->Data.Research.Upgrade-Upgrades]);
	    //
	    //	Transporter with units on board.
	    //
	    } else if( unit->Type->Transporter ) {
		// Count units on board.
		// FIXME: We can do this nicer?
		for( w=f=0; f<MAX_UNITS_ONBOARD; ++f ) {
		    if( unit->OnBoard[f] ) {
			++w;
		    }
		}
		DrawManaBar(x,y,type,MAX_UNITS_ONBOARD,w);
	    }
	}
    }

    //
    //	Mana dot on right side of unit.
    //
    if( ShowManaDot ) {
	    /* s0m3body: MaxMana can vary for each unit, it is stored in unit->Type->_MaxMana */	
	if( type->CanCastSpell && !(ShowNoFull && unit->Mana==unit->Type->_MaxMana) ) {
	    DrawManaSprite(x,y,type,unit->Type->_MaxMana,unit->Mana);
	} else if( type->GivesOil || type->GoldMine || type->OilPatch ) {
	    DrawManaSprite(x,y,type,655350,unit->Value);
	}
	//
	//	Show working of units.
	//
	if( unit->Player==ThisPlayer ) {

	    //
	    //	Building under constuction.
	    //
	    /*
	    if( unit->Orders[0].Action==UnitActionBuilded ) {
		DrawManaSprite(x,y,type,stats->HitPoints,unit->HP);
	    } else
	    */

	    //
	    //	Building training units.
	    //
	    if( unit->Orders[0].Action==UnitActionTrain ) {
		DrawManaSprite(x,y,type,unit->Data.Train.What[0]
			    ->Stats[unit->Player->Player].Costs[TimeCost]
			,unit->Data.Train.Ticks);

	    //
	    //	Building upgrading to better type.
	    //
	    } else if( unit->Orders[0].Action==UnitActionUpgradeTo ) {
		DrawManaSprite(x,y,type,unit->Orders[0].Type
			    ->Stats[unit->Player->Player].Costs[TimeCost]
			,unit->Data.UpgradeTo.Ticks);

	    //
	    //	Building research new technologie.
	    //
	    } else if( unit->Orders[0].Action==UnitActionResearch ) {
		DrawManaSprite(x,y,type,unit->Data.Research.Upgrade
			    ->Costs[TimeCost],
			unit->Player->UpgradeTimers.Upgrades[
			    unit->Data.Research.Upgrade-Upgrades]);
	    //
	    //	Transporter with units on board.
	    //
	    } else if( unit->Type->Transporter ) {
		// Count units on board.
		// FIXME: We can do this nicer?
		for( w=f=0; f<MAX_UNITS_ONBOARD; ++f ) {
		    if( unit->OnBoard[f] ) {
			++w;
		    }
		}
		DrawManaSprite(x,y,type,MAX_UNITS_ONBOARD,w);
	    }
	}
    }

    // FIXME: Johns there is 100% a way to remove this calculation from
    //		runtime.
    x1=x;
    y1=y;
    if( SpellSprite.HotX<0 ) {
	x1+=SpellSprite.HotX
		+(type->TileWidth*TileSizeX+type->BoxWidth+1)/2;
    } else if( SpellSprite.HotX>0 ) {
	x1+=1-SpellSprite.HotX
		+(type->TileWidth*TileSizeX-type->BoxWidth)/2;
    } else {
	x1+=(type->TileWidth*TileSizeX-SpellSprite.Width+1)/2;
    }
    if( SpellSprite.HotY<0 ) {
	y1+=SpellSprite.HotY
		+(type->TileHeight*TileSizeY+type->BoxHeight+1)/2;
    } else if( SpellSprite.HotY>0 ) {
	y1+=1-SpellSprite.HotY
		+(type->TileHeight*TileSizeY-type->BoxHeight)/2;
    } else {
	y1+=(type->TileHeight*TileSizeY-SpellSprite.Height+1)/2;
    }

    //
    // Draw spells decoration
    //
    if ( unit->Bloodlust ) {
	VideoDrawClip( SpellSprite.Sprite, 0, x1, y1 );
    }
    if ( unit->Haste ) {	// same slot as slow
	VideoDrawClip( SpellSprite.Sprite, 1, x1+16, y1 );
    }
    if ( unit->Slow ) {		// same slot as haste
	VideoDrawClip( SpellSprite.Sprite, 2, x1+16, y1 );
    }
    if ( unit->Invisible ) {
	VideoDrawClip( SpellSprite.Sprite, 3, x1+16+16, y1 );
    }
    if ( unit->UnholyArmor ) {
	VideoDrawClip( SpellSprite.Sprite, 4, x1+16+16+16, y1 );
    }

    //
    //	Draw group number
    //
    if( unit->Selected && unit->GroupId!=0 ) {
	char buf[2];
	int num;

	// FIXME: shows the smallest group number, is this what we want?
	for( num=0; !(unit->GroupId & (1<<num)); num++) ;
	buf[0]=num+'0';
	buf[1]='\0';
	f=VideoTextLength(GameFont,buf);
	x+=(type->TileWidth*TileSizeX+type->BoxWidth)/2-f;
	f=VideoTextHeight(GameFont);
	y+=(type->TileHeight*TileSizeY+type->BoxHeight)/2-f;
	VideoDrawNumberClip(x,y,GameFont,num);
    }
}

/**
**	Draw flying units shadow.
**
**	@param unit	Pointer to the unit.
**	@param x	Screen X position of the unit.
**	@param y	Screen Y position of the unit.
**
**	@todo FIXME: later units should have its own shadows with animation.
*/
local void DrawShadow(const Unit* unit, int x, int y)
{
    if( unit->Type->Building ) {
	if( unit->Orders[0].Action==UnitActionBuilded &&
	    (unit->Constructed || VideoGraphicFrames(unit->Type->Sprite)<=1) ) {
	    if( unit->Type->Construction->ShadowSprite ) {
		x-=(unit->Type->Construction->ShadowWidth-unit->Type->TileWidth*TileSizeX)/2;
		y-=(unit->Type->Construction->ShadowHeight-unit->Type->TileHeight*TileSizeY)/2;
		if( unit->Frame<0 ) {
		    VideoDrawShadowClipX(unit->Type->Construction->ShadowSprite,
			-unit->Frame, x, y);
		} else {
		    VideoDrawShadowClip(unit->Type->Construction->ShadowSprite,
			unit->Frame, x, y);
		}
	    }
	    return;
	}
	if( unit->Type->ShadowSprite ) {
	    x-=(unit->Type->ShadowWidth-unit->Type->TileWidth*TileSizeX)/2;
	    y-=(unit->Type->ShadowHeight-unit->Type->TileHeight*TileSizeY)/2;
	    if( unit->Frame<0 ) {
		VideoDrawShadowClipX(unit->Type->ShadowSprite, -unit->Frame, x, y);
	    } else {
		VideoDrawShadowClip(unit->Type->ShadowSprite, unit->Frame, x, y);
	    }
	}
    } else {
	if( unit->Type->ShadowSprite ) {
	    x-=(unit->Type->ShadowWidth-unit->Type->TileWidth*TileSizeX)/2;
	    y-=(unit->Type->ShadowHeight-unit->Type->TileHeight*TileSizeY)/2;
	    if( unit->Type->AirUnit ) {
		y+=TileSizeY;
	    }

	    if( unit->Frame<0 ) {
		VideoDrawShadowClipX(unit->Type->ShadowSprite, -unit->Frame, x, y);
	    } else {
		VideoDrawShadowClip(unit->Type->ShadowSprite, unit->Frame, x, y);
	    }
	} else {
	    int i;

	    // Shadow size depends on box-size
	    if (unit->Type->BoxHeight > 63) {
		i = 2;
	    } else if (unit->Type->BoxHeight > 32) {
		i = 1;
	    } else {
		i = 0;
	    }

	    VideoDrawClip(ShadowSprite.Sprite, i, x + ShadowSprite.HotX,
		y + ShadowSprite.HotY);
	}
    }
}

/**
**	Draw path from current postion to the destination of the move.
**
**	@param unit	Pointer to the unit.
**
**	FIXME: this is the start of the routine which shows the orders
**	FIXME: of the current selected unit.
**	FIXME: should be extend to show waypoints, which order (repair...)
*/
global void DrawPath(const Unit* unit)
{
    int x1;
    int y1;
    int x2;
    int y2;
    int d;
    int dx;
    int dy;
    int xstep;

    // initialize

    x1=unit->X;
    y1=unit->Y;
    if( unit->Orders[0].Goal ) {
	x2=unit->Orders[0].Goal->X;
	y2=unit->Orders[0].Goal->Y;
    } else {
	x2=unit->Orders[0].X;
	y2=unit->Orders[0].Y;
    }

    if( y1>y2 ) {			// exchange coordinates
	x1^=x2; x2^=x1; x1^=x2;
	y1^=y2; y2^=y1; y1^=y2;
    }
    dy=y2-y1;
    dx=x2-x1;
    if( dx<0 ) {
	dx=-dx;
	xstep=-1;
    } else {
	xstep=1;
    }

    if( dy==0 ) {		// horizontal line
	if( dx==0 ) {
	    return;
	}
	// CLIPPING
	VideoDrawRectangleClip(ColorGray
	    ,Map2ScreenX(x1)+TileSizeX/2-3,Map2ScreenY(y1)+TileSizeY/2-3
	    ,6,6);
	while( x1!=x2 ) {
	    x1+=xstep;
	    VideoDrawRectangleClip(ColorGray
		,Map2ScreenX(x1)+TileSizeX/2-3,Map2ScreenY(y1)+TileSizeY/2-3
		,6,6);
	}
	return;
    }

    if( dx==0 ) {		// vertical line
	// CLIPPING
	VideoDrawRectangleClip(ColorGray
	    ,Map2ScreenX(x1)+TileSizeX/2-3,Map2ScreenY(y1)+TileSizeY/2-3
	    ,6,6);
	while( y1!=y2 ) {
	    y1++;
	    VideoDrawRectangleClip(ColorGray
		,Map2ScreenX(x1)+TileSizeX/2-3,Map2ScreenY(y1)+TileSizeY/2-3
		,6,6);
	}
	return;
    }

    VideoDrawRectangleClip(ColorGray
	,Map2ScreenX(x1)+TileSizeX/2-3,Map2ScreenY(y1)+TileSizeY/2-3
	,6,6);

    if( dx<dy ) {		// step in vertical direction
	d=dy-1;
	dx+=dx;
	dy+=dy;
	while( y1!=y2 ) {
	    y1++;
	    d-=dx;
	    if( d<0 ) {
		d+=dy;
		x1+=xstep;
	    }
	    VideoDrawRectangleClip(ColorGray
		,Map2ScreenX(x1)+TileSizeX/2-3,Map2ScreenY(y1)+TileSizeY/2-3
		,6,6);
	}
	return;
    }

    if( dx>dy ) {		// step in horizontal direction
	d=dx-1;
	dx+=dx;
	dy+=dy;

	while( x1!=x2 ) {
	    x1+=xstep;
	    d-=dy;
	    if( d<0 ) {
		d+=dx;
		++y1;
	    }
	    VideoDrawRectangleClip(ColorGray
		,Map2ScreenX(x1)+TileSizeX/2-3,Map2ScreenY(y1)+TileSizeY/2-3
		,6,6);
	}
	return;
    }

				// diagonal line
    while( y1!=y2) {
	x1+=xstep;
	y1++;
	VideoDrawRectangleClip(ColorGray
	    ,Map2ScreenX(x1)+TileSizeX/2-3,Map2ScreenY(y1)+TileSizeY/2-3
	    ,6,6);
    }
}

/**
**	Show the order on map.
**
**	@param unit	Unit pointer.
**	@param x1	X pixel coordinate.
**	@param y1	Y pixel coordinate.
**	@param order	Order to display.
*/
local void ShowSingleOrder(const Unit* unit,int x1,int y1,const Order* order)
{
    int x2;
    int y2;
    int color;
    int e_color;
    int dest;
    const Unit* goal;

    if( (goal=order->Goal) && goal->Type ) {
	x2=Map2ScreenX(goal->X)+goal->IX+goal->Type->TileWidth*TileSizeX/2;
	y2=Map2ScreenY(goal->Y)+goal->IY+goal->Type->TileHeight*TileSizeY/2;
    } else {
	x2=Map2ScreenX(order->X+order->RangeX/2)+TileSizeX/2;
	y2=Map2ScreenY(order->Y+order->RangeY/2)+TileSizeY/2;
    }
    dest=0;
    switch( order->Action ) {
	case UnitActionNone:
	    e_color=color=ColorGray;
	    break;

	case UnitActionStill:
	    e_color=color=ColorGray;
	    break;

	case UnitActionStandGround:
	    e_color=color=ColorGreen;
	    break;

	case UnitActionFollow:
	case UnitActionMove:
	    e_color=color=ColorGreen;
	    dest=1;
	    break;

	case UnitActionPatrol:
	    VideoDrawLineClip(ColorGreen,x1,y1,x2,y2);
	    e_color=color=ColorBlue;
	    x1=Map2ScreenX(((int)order->Arg1)>>16)+TileSizeX/2;
	    y1=Map2ScreenY(((int)order->Arg1)&0xFFFF)+TileSizeY/2;
	    dest=1;
	    break;

	case UnitActionRepair:
	    e_color=color=ColorGreen;
	    dest=1;
	    break;

	case UnitActionAttackGround:
	    x2=Map2ScreenX(order->X)+TileSizeX/2;
	    y2=Map2ScreenY(order->Y)+TileSizeY/2;
	    // FALL THROUGH
	case UnitActionAttack:
	    if( unit->SubAction&2 ) {	// Show weak targets.
		e_color=ColorBlue;
	    } else {
		e_color=ColorRed;
	    }
	    color=ColorRed;
	    dest=1;
	    break;

	case UnitActionBoard:
	    e_color=color=ColorGreen;
	    dest=1;
	    break;

	case UnitActionUnload:
	    e_color=color=ColorGreen;
	    dest=1;
	    break;

	case UnitActionDie:
	    e_color=color=ColorGray;
	    break;

	case UnitActionSpellCast:
	    e_color=color=ColorBlue;
	    dest=1;
	    break;

	case UnitActionTrain:
	    e_color=color=ColorGray;
	    break;

	case UnitActionUpgradeTo:
	    e_color=color=ColorGray;
	    break;

	case UnitActionResearch:
	    e_color=color=ColorGray;
	    break;

	case UnitActionBuild:
	    e_color=color=ColorGreen;
	    dest=1;
	    break;

	case UnitActionBuilded:
	    e_color=color=ColorGray;
	    break;

	case UnitActionHarvest:
	    e_color=color=ColorYellow;
	    dest=1;
	    break;

	case UnitActionMineGold:
	    e_color=color=ColorYellow;
	    dest=1;
	    break;

	case UnitActionHaulOil:
	    e_color=color=ColorYellow;
	    dest=1;
	    break;

	case UnitActionReturnGoods:
	    e_color=color=ColorYellow;
	    dest=1;
	    break;

	case UnitActionDemolish:
	    e_color=color=ColorRed;
	    dest=1;
	    break;

	default:
	    e_color=color=ColorGray;
	    DebugLevel1Fn("Unknown action %d\n" _C_ order->Action);
	    break;
    }
    VideoFillCircleClip(color,x1,y1,2);
    if( dest ) {
	VideoDrawLineClip(color,x1,y1,x2,y2);
	VideoFillCircleClip(e_color,x2,y2,3);
    }
    //DrawPath(unit);
}

/**
**	Show the current order of an unit.
**
**	@param unit	Pointer to the unit.
*/
local void ShowOrder(const Unit* unit)
{
    int x1;
    int y1;

    if( unit->Destroyed ) {
	return;
    }
    x1=Map2ScreenX(unit->X)+unit->IX+unit->Type->TileWidth*TileSizeX/2;
    y1=Map2ScreenY(unit->Y)+unit->IY+unit->Type->TileHeight*TileSizeY/2;

    ShowSingleOrder(unit,x1,y1,unit->Orders);
    if( unit->Type->Building ) {
	ShowSingleOrder(unit,x1,y1,&unit->NewOrder);
    }
}

/**
**	Draw additional informations of an unit.
**
**	@param unit	Unit pointer of drawn unit.
**	@param type	Unit-type pointer.
**	@param x	X screen pixel position of unit.
**	@param y	Y screen pixel position of unit.
*/
local void DrawInformations(const Unit* unit,const UnitType* type,int x,int y)
{
    const UnitStats* stats;
    int r;

    stats=unit->Stats;

    //
    //	For debug draw sight, react and attack range!
    //
    if( NumSelected==1 && unit->Selected ) {
	if( ShowSightRange ) {
	    if( ShowSightRange == 1 ) {
		VideoFill75TransRectangleClip(ColorGreen
		    ,x+type->TileWidth*TileSizeX/2-stats->SightRange*TileSizeX
		    ,y+type->TileHeight*TileSizeY/2-stats->SightRange*TileSizeY
		    ,stats->SightRange*TileSizeX*2
		    ,stats->SightRange*TileSizeY*2);
	    } else if( ShowSightRange == 2 ) {
		VideoFill75TransCircleClip(ColorGreen
			,x+type->TileWidth*TileSizeX/2
			,y+type->TileHeight*TileSizeY/2
			,min(stats->SightRange*TileSizeX
			    ,stats->SightRange*TileSizeY));
	    } else {
		VideoDrawRectangleClip(ColorGreen
		    ,x+type->TileWidth*TileSizeX/2-stats->SightRange*TileSizeX
		    ,y+type->TileHeight*TileSizeY/2-stats->SightRange*TileSizeY
		    ,stats->SightRange*TileSizeX*2
		    ,stats->SightRange*TileSizeY*2);
	    }
	}
	if( type->CanAttack || type->Tower ) {
	    if( ShowReactionRange ) {
		r= (unit->Player->Type==PlayerPerson)
			? type->ReactRangePerson
			: type->ReactRangeComputer;
		if( r ) {
		    VideoDrawRectangleClip(ColorBlue
			,x+type->TileWidth*TileSizeX/2-r*TileSizeX
			,y+TileSizeY/2-r*TileSizeY
			,r*TileSizeX*2
			,r*TileSizeY*2);
		}
	    }
	    if( ShowAttackRange && stats->AttackRange ) {
		VideoDrawRectangleClip(ColorRed
		    ,x+type->TileWidth*TileSizeX/2-stats->AttackRange*TileSizeX
		    ,y+type->TileHeight*TileSizeY/2-stats->AttackRange*TileSizeY
		    ,stats->AttackRange*TileSizeX*2
		    ,stats->AttackRange*TileSizeY*2);
	    }
	}
    }

    //
    //	Show order.
    //
    if( ShowOrders || (unit->Selected
	    && (ShowOrdersCount ||(KeyModifiers&ModifierShift))) ) {
	ShowOrder(unit);
    }

    // FIXME: johns: ugly check here, should be removed!
    if( unit->Orders[0].Action!=UnitActionDie ) {
	DrawDecoration(unit,type,x,y);
    }
}

/**
**	Change current color set to units colors.
**
**	FIXME: use function pointer here.
**
**	@param unit	Pointer to unit.
**	@param sprite	Change the palette entries 208-210 in this sprite.
*/
local void GraphicUnitPixels(const Unit* unit,const Graphic* sprite)
{
    switch( VideoBpp ) {
	case 8:
	    *((struct __4pixel8__*)(((VMemType8*)sprite->Pixels)+208))
		    =unit->Colors.Depth8;
	    break;
	case 15:
	case 16:
	    *((struct __4pixel16__*)(((VMemType16*)sprite->Pixels)+208))
		    =unit->Colors.Depth16;
	    break;
	case 24:
	    *((struct __4pixel24__*)(((VMemType24*)sprite->Pixels)+208))
		    =unit->Colors.Depth24;
	    break;
	case 32:
	    *((struct __4pixel32__*)(((VMemType32*)sprite->Pixels)+208))
		    =unit->Colors.Depth32;
	    break;
    }
}

/*
**	Units on map:
**
**	1) Must draw underground/underwater units. (FUTURE extension)
**	2) Must draw buildings and corpse.
**	3) Must draw land/sea units.
**	4) Must draw decoration units. (FUTURE extension)
**	5) Must draw low air units.
**	6) Must draw middle air units. (FUTURE extension)
**	7) Must draw hight air units. (FUTURE extension)
*/

/**
**	Draw building on map.
**
**	@param unit	Pointer to the building
*/
#ifdef NEW_DECODRAW
global void DrawBuilding(Unit* unit)
#else
local void DrawBuilding(Unit* unit)
#endif
{
    int x;
    int y;
    const UnitType* type;
    int frame;
    int visible;

    visible=UnitVisibleOnMap(unit);
    // FIXME: is this the correct place? No, but now correct working.
    if( visible ) {
	frame = unit->SeenFrame = unit->Frame;
    } else {
	frame = unit->SeenFrame;
	DebugCheck( frame==UnitNotSeen );
    }

    type=unit->Type;
    x=Map2ScreenX(unit->X)+unit->IX;
    y=Map2ScreenY(unit->Y)+unit->IY;

    DrawShadow(unit,x,y);

    //
    //	Show that the unit is selected
    //
    DrawSelection(unit,type,x,y);

    //
    //	Buildings under construction/upgrade/ready.
    //
    if( unit->Orders[0].Action==UnitActionBuilded ) {
	if( unit->Constructed || VideoGraphicFrames(type->Sprite)<=1 ) {
	    GraphicUnitPixels(unit,type->Construction->Sprite);
	    DrawConstruction(type->Construction
		,frame&127
		,x+(type->TileWidth*TileSizeX)/2
		,y+(type->TileHeight*TileSizeY)/2);
	} else {
	    GraphicUnitPixels(unit,type->Sprite);
	    DrawUnitType(type,frame,x,y);
	}
    //
    //	Draw the future unit type, if upgrading to it.
    //
    } else if( unit->Orders[0].Action==UnitActionUpgradeTo ) {
	// FIXME: this frame is hardcoded!!!
	GraphicUnitPixels(unit,unit->Orders[0].Type->Sprite);
	DrawUnitType(unit->Orders[0].Type,frame<0?-1:1,x,y);
    } else {
	GraphicUnitPixels(unit,type->Sprite);
	DrawUnitType(type,frame,x,y);
    }

    // FIXME: johns: ugly check here, should be removed!
    if( visible ) {
	DrawInformations(unit,type,x,y);
    }
}

/**
**	Draw unit on map.
**
**	@param unit	Pointer to the unit.
*/
#ifdef NEW_DECODRAW
global void DrawUnit(const Unit* unit)
#else
local void DrawUnit(const Unit* unit)
#endif
{
    int x;
    int y;
    const UnitType* type;

    if ( unit->Revealer ) {		// Revealers are not drawn
	DebugLevel3Fn("Drawing revealer %d\n" _C_ UnitNumber(unit));
	return;
    }

    x=Map2ScreenX(unit->X)+unit->IX;
    y=Map2ScreenY(unit->Y)+unit->IY;

    type=unit->Type;
    if( type->UnitType==UnitTypeFly || type->ShadowSprite ) {
	DrawShadow(unit,x,y);
    }

    //
    //	Show that the unit is selected
    //
    DrawSelection(unit,type,x,y);

    GraphicUnitPixels(unit,type->Sprite);
    DrawUnitType(type,unit->Frame,x,y);

#ifndef NEW_DECODRAW
// Unit's extras not fully supported.. need to be deocrations themselves.
    DrawInformations(unit,type,x,y);
#endif
}

/**
**	Draw all units on visible map.
**	FIXME: Must use the redraw tile flags in this function
*/
#ifdef SPLIT_SCREEN_SUPPORT
global void DrawUnits (int v)
{
    Unit* unit;
    Unit* table[UnitMax];
    int n;
    int i;

    CurrentViewport = v;
    //
    //	Select all units touching the viewpoint.
    //
    n = SelectUnits(TheUI.VP[v].MapX-1, TheUI.VP[v].MapY-1,
		TheUI.VP[v].MapX+TheUI.VP[v].MapWidth+1,
		TheUI.VP[v].MapY+TheUI.VP[v].MapHeight+1,table);
#else /* SPLIT_SCREEN_SUPPORT */

global void DrawUnits(void)
{
    Unit* unit;
    Unit* table[UnitMax];
    int n;
    int i;

    //
    //	Select all units touching the viewpoint.
    //
    // FIXME: Must be here +1 on Width, Height.
    n=SelectUnits(MapX-1,MapY-1,MapX+MapWidth+1,MapY+MapHeight+1,table);
#endif /* SPLIT_SCREEN_SUPPORT */

    //
    //	2a) corpse aren't in the cache.
    //
    for( i=0; i<NumUnits; ++i ) {
	unit=Units[i];
	// FIXME: this tries to draw all corps, ohje
	if( unit->Type->Vanishes || unit->Orders[0].Action==UnitActionDie ) {
	    DrawUnit(unit);
	}
    }

    //
    //	2b) buildings
    //
    for( i=0; i<n; ++i ) {
	unit=table[i];
#ifdef SPLIT_SCREEN_SUPPORT
	if( !unit->Removed && UnitVisibleInViewport (CurrentViewport, unit) ) {
#else /* SPLIT_SCREEN_SUPPORT */
	if( !unit->Removed && UnitVisibleOnScreen(unit) ) {
#endif /* SPLIT_SCREEN_SUPPORT */
	    if( unit->Type->Building ) {
		DrawBuilding(unit);
		table[i]=NoUnitP;
	    }
	} else {
	    table[i]=NoUnitP;
	}
    }
    //
    //	3) land/sea units
    //
    for( i=0; i<n; ++i ) {
	if( !(unit=table[i]) ) {
	    continue;
	}
	if( unit->Type->UnitType!=UnitTypeFly ) {
	    DrawUnit(unit);
	    table[i]=NoUnitP;
	}
    }
    //
    //	5) flying units
    //
    for( i=0; i<n; ++i ) {
	if( !(unit=table[i]) ) {
	    continue;
	}
	DrawUnit(unit);
    }
}

//@}
