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
//	(c) Copyright 1998-2001 by Lutz Sammer
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
global int ShowHealthDot=1;		/// Flag: show health dot
global int ShowManaBar;			/// Flag: show mana bar
global int ShowManaDot=1;		/// Flag: show mana dot
global int ShowNoFull=1;		/// Flag: show no full health or mana
global int DecorationOnTop=1;		/// Flag: show health and mana on top
global int ShowSightRange;		/// Flag: show right range
global int ShowReactRange;		/// Flag: show react range
global int ShowAttackRange;		/// Flag: show attack range
global int ShowOrders;			/// Flag: show orders of unit on map
    /// Flag: health horizontal instead of vertical
global int ShowHealthHorizontal=1;
    /// Flag: health horizontal instead of vertical
global int ShowManaHorizontal=1;
    /// Flag: show bars and dot energy only for selected
global int ShowEnergySelectedOnly;
    /// Flag: show the health background long
global int ShowHealthBackgroundLong=1;
    /// Flag: show the mana background long
global int ShowManaBackgroundLong=1;

// FIXME: not all variables of this file are here
// FIXME: perhaps split this file into two?

local void DrawSelectionCircle(const Unit* unit,const UnitType* type
	,int x,int y);
local void DrawSelectionCircleWithTrans(const Unit* unit,const UnitType* type
	,int x,int y);
local void DrawSelectionRectangle(const Unit* unit,const UnitType* type
	,int x,int y);
local void DrawSelectionRectangleWithTrans(const Unit* unit,const UnitType* type
	,int x,int y);

/**
**	Show that units are selected.
**
**	@param unit	Pointer to the unit.
**	@param type	Type of the unit.
**	@param x	Screen X position of the unit.
**	@param y	Screen Y position of the unit.
*/
local void (*DrawSelection)(const Unit*,const UnitType*,int,int)
	=DrawSelectionRectangleWithTrans;

/*----------------------------------------------------------------------------
--	Functions
----------------------------------------------------------------------------*/

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
**	Show selected units with circle.
**
**	@param unit	Pointer to the unit.
**	@param type	Type of the unit.
**	@param x	Screen X position of the unit.
**	@param y	Screen Y position of the unit.
*/
local void DrawSelectionCircle(const Unit* unit,const UnitType* type
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
local void DrawSelectionCircleWithTrans(const Unit* unit,const UnitType* type
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
local void DrawSelectionRectangle(const Unit* unit,const UnitType* type
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
local void DrawSelectionRectangleWithTrans(const Unit* unit,const UnitType* type
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
    VideoFill75TransRectangleClip(color
	    ,x+1+(type->TileWidth*TileSizeX-type->BoxWidth)/2
	    ,y+1+(type->TileHeight*TileSizeY-type->BoxHeight)/2
	    ,type->BoxWidth-2
	    ,type->BoxHeight-2);
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
global Decoration ManaSprite
#ifndef USE_CCL
    = { "mana.png",		-7,-7, 7,7 }
#endif
    ;

/**
**	Sprite to display the health.
*/
global Decoration HealthSprite
#ifndef USE_CCL
    = { "health.png",		0,-7, 7,7 }
#endif
    ;

/**
**	Sprite to display the shadow of flying units.
**
**	@todo	Made this configurable with CCL.
*/
global Decoration ShadowSprite
#ifndef laterUSE_CCL
#ifdef NEW_NAMES
     = { "graphics/missiles/unit shadow.png",	0,42, 32,32 };
#else
     = { "graphic/unit shadow.png",	0,42, 32,32 };
#endif
#endif
    ;

/**
**	Sprite to display the active spells on units.
*/
global Graphic* SpellSprites;

#if defined(USE_CCL)

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
    CclFree(ManaSprite.File);
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
    CclFree(HealthSprite.File);
    HealthSprite.File=gh_scm2newstr(file,NULL);
    HealthSprite.HotX=gh_scm2int(x);
    HealthSprite.HotY=gh_scm2int(y);
    HealthSprite.Width=gh_scm2int(w);
    HealthSprite.Height=gh_scm2int(h);

    return SCM_UNSPECIFIED;
}

#endif	// defined(USE_CCL)

/**
**	Load decoration.
*/
global void LoadDecorations(void)
{
    HealthSprite.Sprite=LoadSprite(HealthSprite.File
		,HealthSprite.Width,HealthSprite.Height);
    ManaSprite.Sprite=LoadSprite(ManaSprite.File
		,ManaSprite.Width,ManaSprite.Height);
    ShadowSprite.Sprite=LoadSprite(ShadowSprite.File
		,ShadowSprite.Width,ShadowSprite.Height);
    // FIXME: make this configurable
#ifdef NEW_NAMES
    SpellSprites=LoadSprite("graphics/ui/bloodlust,haste,slow,invisible,shield.png"
	,16,16);
#else
    SpellSprites=LoadSprite("graphic/bloodlust,haste,slow,invis.,shield.png",16,16);
#endif
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
#if 0
    f=(100*ready)/full;
    if( f>75) {
	n=0;
    } else if( f>50 ) {
	n=1;
    } else if( f>25 ) {
	n=2;
		// FIXME: v--- compatibility hack
    } else if( f
	&& ManaSprite.Width*4<VideoGraphicWidth(ManaSprite.Sprite) ) {
	n=3;
    } else {
	n=4;
    }
#endif
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
    int color, w;
    const UnitStats* stats;

#ifdef REFS_DEBUG
    //
    //	Show the number of references.
    //
    if( x>TheUI.MapX && x<TheUI.MapEndX &&
		y+8>TheUI.MapY && y+8<TheUI.MapEndY ) {
	DrawNumber(x+1,y+1,GameFont,unit->Refs);
    }
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
	    int x1;
	    int y1;
	    int n;

	    n=VideoGraphicFrames(HealthSprite.Sprite)-1;
	    n-=(n*unit->HP)/stats->HitPoints;
#if 0
	    f=(100*unit->HP)/stats->HitPoints;
	    if( f>75) {
		n=3-((f-75)/(25/3))+ 0;
	    } else if( f>50 ) {
		n=3-((f-50)/(25/3))+ 4;
		DebugLevel3("%d - %d\n",f,n);
	    } else {
		n=3-(f/(50/3))+ 8;
		DebugLevel3("%d - %d\n",f,n);
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
	if( type->CanCastSpell && !(ShowNoFull && unit->Mana==255) ) {
	    DrawManaBar(x,y,type,255,unit->Mana);
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
			    ->Costs[TimeCost]
			,unit->Data.Research.Ticks);
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
	if( type->CanCastSpell
		&& !(ShowNoFull && unit->Mana==255) ) {
	    DrawManaSprite(x,y,type,255,unit->Mana);
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
			    ->Costs[TimeCost]
			,unit->Data.Research.Ticks);
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

    x+=(type->TileWidth*TileSizeX-type->BoxWidth)/2;
    y+=(type->TileHeight*TileSizeY-type->BoxHeight)/2;
    //
    // Draw spells decoration
    //
    if ( unit->Bloodlust ) {
	VideoDrawClip( SpellSprites, 0, x, y );
    }
    if ( unit->Haste ) {	// same slot as slow
	VideoDrawClip( SpellSprites, 1, x+16, y );
    }
    if ( unit->Slow ) {		// same slot as haste
	VideoDrawClip( SpellSprites, 2, x+16, y );
    }
    if ( unit->Invisible ) {
	VideoDrawClip( SpellSprites, 3, x+16+16, y );
    }
    if ( unit->UnholyArmor ) {
	VideoDrawClip( SpellSprites, 4, x+16+16+16, y );
    }
    x-=(type->TileWidth*TileSizeX-type->BoxWidth)/2;
    y-=(type->TileHeight*TileSizeY-type->BoxHeight)/2;

    //
    //	Draw group number
    //
    if( unit->Selected && unit->GroupId!=-1 ) {
	char buf[2];

	buf[0]=unit->GroupId+'0';
	buf[1]='\0';
	f=TextLength(GameFont,buf);
	x+=(type->TileWidth*TileSizeX+type->BoxWidth)/2-f;
	y+=(type->TileHeight*TileSizeY+type->BoxHeight)/2-14;
	// FIXME: should use FontHeight(GameFont);
	if( x>TheUI.MapX && x+f<TheUI.MapEndX && y>TheUI.MapY
		&& y+14<TheUI.MapEndY ) {
	    DrawNumber(x,y,GameFont,unit->GroupId);
	}
    }
}

/**
**	Draw flying units shadow.
**
**	@param unit	Pointer to the unit.
**	@param type	Type of the unit.
**	@param x	Screen X position of the unit.
**	@param y	Screen Y position of the unit.
*/
local void DrawShadow(const Unit* unit,const UnitType* type,int x,int y)
{
    int i;

    // FIXME: later units should have its own shadows with animation

    // Shadow size depends on box-size
    if( type->BoxHeight>63 ) {
	i=2;
    } else if( type->BoxHeight>32 ) {
	i=1;
    } else {
	i=0;
    }
    DebugLevel3("Box-height %d\n",type->BoxHeight);

    VideoDrawClip(ShadowSprite.Sprite,i
	    ,x+ShadowSprite.HotX,y+ShadowSprite.HotY);
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

	case UnitActionAttack:
	case UnitActionAttackGround:
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
	    DebugLevel1Fn("Unknown action %d\n",order->Action);
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
**	@param X	X screen pixel position of unit.
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
	    VideoDrawRectangleClip(ColorGreen
		,x+TileSizeX/2-stats->SightRange*TileSizeX
		,y+TileSizeY/2-stats->SightRange*TileSizeY
		,stats->SightRange*TileSizeX*2
		,stats->SightRange*TileSizeY*2);
	}
	if( type->CanAttack || type->Tower ) {
	    if( ShowReactRange ) {
		r= (unit->Player->Type==PlayerHuman)
			? type->ReactRangeHuman
			: type->ReactRangeComputer;
		if( r ) {
		    VideoDrawRectangleClip(ColorBlue
			,x+TileSizeX/2-r*TileSizeX
			,y+TileSizeY/2-r*TileSizeY
			,r*TileSizeX*2
			,r*TileSizeY*2);
		}
	    }
	    if( ShowAttackRange && stats->AttackRange ) {
		VideoDrawRectangleClip(ColorRed
		    ,x+TileSizeX/2-stats->AttackRange*TileSizeX
		    ,y+TileSizeY/2-stats->AttackRange*TileSizeY
		    ,stats->AttackRange*TileSizeX*2
		    ,stats->AttackRange*TileSizeY*2);
	    }
	}
    }

    //
    //	Show order.
    //
    if( ShowOrders /*&& unit->Selected && (KeyModifiers&ModifierShift) */) {
	ShowOrder(unit);
    }

    // FIXME: johns: ugly check here, should be removed!
    if( unit->Orders[0].Action!=UnitActionDie ) {
	DrawDecoration(unit,type,x,y);
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
local void DrawBuilding(Unit* unit)
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
	DebugCheck( frame==-1 || frame==0xFF );
    }

    type=unit->Type;
    GraphicPlayerPixels(unit->Player,type->Sprite);
    x=Map2ScreenX(unit->X)+unit->IX;
    y=Map2ScreenY(unit->Y)+unit->IY;

    DrawSelection(unit,type,x,y);

    //
    //	Buildings under construction/upgrade/ready.
    //
    if( unit->Orders[0].Action==UnitActionBuilded ) {
	if( unit->Constructed || VideoGraphicFrames(type->Sprite)<=1 ) {
	    DrawConstruction(type->OverlapFrame
		,frame&127
		,x+(type->TileWidth*TileSizeX)/2
		,y+(type->TileHeight*TileSizeY)/2);
	} else {
	    DrawUnitType(type,frame,x,y);
	}
    //
    //	Draw the future unit type, if upgrading to it.
    //
    } else if( unit->Orders[0].Action==UnitActionUpgradeTo ) {
	// FIXME: this frame is hardcoded!!!
	DrawUnitType(unit->Orders[0].Type,(frame&128)+1,x,y);
    } else {
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
local void DrawUnit(const Unit* unit)
{
    int x;
    int y;
    const UnitType* type;

    if ( unit->Revealer ) {		// Revealers are not drawn
	DebugLevel3Fn("Drawing revealer %d\n",UnitNumber(unit));
	return;
    }

    x=Map2ScreenX(unit->X)+unit->IX;
    y=Map2ScreenY(unit->Y)+unit->IY;

    type=unit->Type;
    if( type->UnitType==UnitTypeFly ) {
	DrawShadow(unit,type,x,y);
    }

    DrawSelection(unit,type,x,y);

    GraphicPlayerPixels(unit->Player,type->Sprite);
    DrawUnitType(type,unit->Frame,x,y);

    DrawInformations(unit,type,x,y);
}

/**
**	Draw all units on visible map.
**	FIXME: Must use the redraw tile flags in this function
*/
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
	if( !unit->Removed && UnitVisibleOnScreen(unit) ) {
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
