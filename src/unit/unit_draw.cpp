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
/*
**	(c) Copyright 1998-2000 by Lutz Sammer
**
**	$Id$
*/

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
**	Show that units are selection.
**
**	@param unit	Pointer to the unit.
**	@param type	Type of the unit.
**	@param x	Screen X position of the unit.
**	@param y	Screen Y position of the unit.
*/
local void (*DrawSelection)(const Unit*,const UnitType*,int,int)
	=DrawSelectionRectangle;

/*----------------------------------------------------------------------------
--	Functions
----------------------------------------------------------------------------*/

/**
**	Choose color for selection.
**
**	@param unit	Pointer to the unit.
**	@param type	Type of the unit.
**
**	@returns	Color for selection, or -1 if not selected.
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
global Decoration ManaSprite = {
    "mana.png",		-7,-7, 7,7
};

/**
**	Sprite to display the health.
*/
global Decoration HealthSprite = {
    "health.png",	0,-7, 7,7
};

/**
**	Sprite to display the shadow of flying units.
*/
global Decoration ShadowSprite = {
    "graphic/unit shadow.png",	0,42, 32,32
};

/**
**	Sprite to display the active spells on units.
*/
global Graphic* SpellSprites;

#if defined(USE_CCL) || defined(USE_CCL2)

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

#endif	// defined(USE_CCL) || defined(USE_CCL2)

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
    SpellSprites=LoadSprite("graphic/bloodlust,haste,slow,invis.,shield.png",16,16);		
}

/**
**	Draw decoration (invis, for the unit.)
**
**	@param unit	Pointer to the unit.
**	@param type	Type of the unit.
**	@param x	Screen X position of the unit.
**	@param y	Screen Y position of the unit.
*/
local void DrawDecoration(Unit* unit,const UnitType* type,int x,int y)
{
    int f;
    int color;
    UnitStats* stats;


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
    if( !type->Critter && ShowHealthBar ) {
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
	    if ( ShowHealthHorizontal == 0)  {
		    VideoFillRectangleClip(color
			,x+(type->TileWidth*TileSizeX
				-type->BoxWidth)/2
			,y+(type->TileHeight*TileSizeY
				-type->BoxHeight)/2
			,2,(f*type->BoxHeight)/100);
	    }  else  {
		    VideoFillRectangleClip(ColorBlack
			,x+((type->TileWidth*TileSizeX-type->BoxWidth)/2)
			,(y+(type->TileHeight*TileSizeY-type->BoxHeight)/2)
				+type->BoxHeight+1
			,((f*type->BoxHeight)/100)+1
			,5);
		    VideoFillRectangleClip(color
			,x+((type->TileWidth*TileSizeX-type->BoxWidth)/2)+1
			,(y+(type->TileHeight*TileSizeY-type->BoxHeight)/2)
				+type->BoxHeight+2
			,(f*type->BoxHeight)/100-1
			,3);
	    }
	}
    }

    //
    //	Health dot on left side of unit.
    //
    if( !type->Critter && ShowHealthDot ) {
	if( stats->HitPoints
		&& !(ShowNoFull && unit->HP==stats->HitPoints) ) {
	    int x1;
	    int y1;
	    int n;

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
	    DebugCheck( n<0 );
	    if( HealthSprite.HotX<0 ) {
		x1=x+HealthSprite.HotX
			+(type->TileWidth*TileSizeX
			+type->BoxWidth+1)/2;
	    } else {
		x1=x-HealthSprite.HotX
			+(type->TileWidth*TileSizeX
			-type->BoxWidth+1)/2;
	    }
	    if( HealthSprite.HotY<0 ) {
		y1=y+HealthSprite.HotY
			+(type->TileHeight*TileSizeY
			+type->BoxHeight+1)/2;
	    } else {
		y1=y-HealthSprite.HotY
			+(type->TileHeight*TileSizeY
			-type->BoxHeight+1)/2;
	    }
	    VideoDrawClip(HealthSprite.Sprite,n,x1,y1);
	}
    }

    //
    //	Mana bar on right side of unit.
    //
    if( ShowManaBar ) {
	if( type->CanCastSpell
		&& !(ShowNoFull && unit->Mana==255) ) {
	    if ( ShowManaHorizontal == 0)  {
	    	f=(100*unit->Mana)/255;
		    VideoFillRectangleClip(ColorBlue
			,x+(type->TileWidth*TileSizeX
				+type->BoxWidth)/2
			,y+(type->TileHeight*TileSizeY
				-type->BoxHeight)/2
			,2,(f*type->BoxHeight)/100);
	    }  else  {
		    f=(100*unit->Mana)/255;
		    VideoFillRectangleClip(ColorBlack
			,x+((type->TileWidth*TileSizeX-type->BoxWidth)/2)
			,(y+(type->TileHeight*TileSizeY-type->BoxHeight)/2)+type->BoxHeight+5
			,(type->BoxHeight)+1
			,5);
		    VideoFillRectangleClip(ColorBlue
			,x+(type->TileWidth*TileSizeX-type->BoxWidth)/2+1
			,(y+(type->TileHeight*TileSizeY-type->BoxHeight)/2)+type->BoxHeight+6
			,(f*type->BoxHeight)/100-1
			,3);
	    }

	}
    }

    //
    //	Mana dot on right side of unit.
    //
    if( ShowManaDot ) {
	if( type->CanCastSpell
		&& !(ShowNoFull && unit->Mana==255) ) {
	    int x1;
	    int y1;
	    int n;

	    f=(100*unit->Mana)/255;
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
	    if( ManaSprite.HotX<0 ) {
		x1=x+ManaSprite.HotX
			+(type->TileWidth*TileSizeX
			+type->BoxWidth+1)/2;
	    } else {
		x1=x-ManaSprite.HotX
			+(type->TileWidth*TileSizeX
			-type->BoxWidth+1)/2;
	    }
	    if( ManaSprite.HotY<0 ) {
		y1=y+ManaSprite.HotY
			+(type->TileHeight*TileSizeY
			+type->BoxHeight+1)/2;
	    } else {
		y1=y-ManaSprite.HotY
			+(type->TileHeight*TileSizeY
			-type->BoxHeight+1)/2;
	    }
	    VideoDrawClip(ManaSprite.Sprite,n,x1,y1);
	}
    }

    //
    // Draw spells decoration
    //
    if ( unit->Bloodlust > 0 )
      VideoDrawClip( SpellSprites, 0, x, y );
    if ( unit->Haste > 0 ) // same slot as slow
      VideoDrawClip( SpellSprites, 1, x+16, y );
    if ( unit->Slow > 0 ) // same slot as haste
      VideoDrawClip( SpellSprites, 2, x+16, y );
    if ( unit->Invisible > 0 )
      VideoDrawClip( SpellSprites, 3, x+16+16, y );
    if ( unit->UnholyArmor > 0 )
      VideoDrawClip( SpellSprites, 4, x+16+16+16, y );
    
    // FIXME: group number could also be shown
}

/**
**	Draw flying units shadow.
**
**	@param unit	Pointer to the unit.
**	@param type	Type of the unit.
**	@param x	Screen X position of the unit.
**	@param y	Screen Y position of the unit.
*/
local void DrawShadow(Unit* unit,UnitType* type,int x,int y)
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
global void DrawPath(Unit* unit)
{
#ifdef NEW_ORDERS
    DebugLevel0Fn("FIXME: later\n");
#else
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
    x2=unit->Command.Data.Move.DX;
    y2=unit->Command.Data.Move.DY;

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
#endif
}

/**
**	Show the current order of an unit.
**
**	@param unit	Pointer to the unit.
**
*/
local void ShowOrder(const Unit* unit)
{
#ifdef NEW_ORDERS
    DebugLevel0Fn("FIXME: later\n");
#else
    int x1;
    int y1;
    int x2;
    int y2;
    int color;
    const Unit* goal;

    if( unit->Destroyed ) {
	return;
    }
    x1=Map2ScreenX(unit->X)+unit->IX+unit->Type->TileWidth*TileSizeX/2;
    y1=Map2ScreenY(unit->Y)+unit->IY+unit->Type->TileHeight*TileSizeY/2;

    if( (goal=unit->Command.Data.Move.Goal) && goal->Type ) {
	x2=Map2ScreenX(goal->X)+goal->IX+goal->Type->TileWidth*TileSizeX/2;
	y2=Map2ScreenY(goal->Y)+goal->IY+goal->Type->TileHeight*TileSizeY/2;
    } else {
	x2=Map2ScreenX(unit->Command.Data.Move.DX)+TileSizeX/2;
	y2=Map2ScreenY(unit->Command.Data.Move.DY)+TileSizeY/2;
    }
    switch( unit->Command.Action ) {
	case UnitActionNone:
	    color=ColorGray;
	    break;

	case UnitActionStill:
	    color=ColorGray;
	    break;

	case UnitActionStandGround:
	    color=ColorGray;
	    break;

	case UnitActionFollow:
	case UnitActionMove:
	    color=ColorGreen;
	    break;

	case UnitActionPatrol:
	    VideoDrawLineClip(ColorGreen,x1,y1,x2,y2);
	    color=ColorBlue;
	    x1=Map2ScreenX(unit->Command.Data.Move.SX)+TileSizeX/2;
	    y1=Map2ScreenY(unit->Command.Data.Move.SY)+TileSizeY/2;
	    break;

	case UnitActionRepair:
	    color=ColorGreen;
	    break;

	case UnitActionAttack:
	case UnitActionAttackGround:
	    color=ColorRed;
	    break;

	case UnitActionBoard:
	    color=ColorGreen;
	    break;

	case UnitActionUnload:
	    color=ColorGreen;
	    break;

	case UnitActionDie:
	    color=ColorGray;
	    break;

	case UnitActionSpellCast:
	    color=ColorBlue;
	    break;

	case UnitActionTrain:
	    color=ColorGray;
	    break;

	case UnitActionUpgradeTo:
	    color=ColorGray;
	    break;

	case UnitActionResearch:
	    color=ColorGray;
	    break;

	case UnitActionBuild:
	    color=ColorGreen;
	    break;

	case UnitActionBuilded:
	    color=ColorGray;
	    break;

	case UnitActionHarvest:
	    color=ColorGreen;
	    break;

	case UnitActionMineGold:
	    color=ColorGreen;
	    break;

	case UnitActionHaulOil:
	    color=ColorGreen;
	    break;

	case UnitActionReturnGoods:
	    color=ColorGreen;
	    break;

	case UnitActionDemolish:
	    color=ColorRed;
	    break;

	default:
	    color=ColorGray;
	    DebugLevel1Fn("Unknown action %d\n",unit->Command.Action);
	    break;
    }
    VideoFillCircleClip(color,x1,y1,2);
    VideoDrawLineClip(color,x1,y1,x2,y2);
    VideoFillCircleClip(color,x2,y2,2);

    //DrawPath(unit);
#endif
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
    UnitType* type;
    int frame;

    // FIXME: This should I rewrite, without checks here!!

    type=unit->Type;
    x = unit->X;
    y = unit->Y;

#ifdef NEW_FOW
    if ( !IsMapFieldVisible( x, y ) ) {
	frame = unit->SeenFrame;
	if (frame == 255) {
	    return;
	}
    } else {
	// FIXME: is this the correct place?
	frame = unit->SeenFrame = unit->Frame;
    }
#else
    // FIXME: johns: this isn't 100% correct, building which are partly
    // FIXME: johns: under the fog are shown partly.

    // FIXME: There is already a check in the main loop UnitVisible!
    if ( !IsMapFieldExplored( x, y ) ) {
	return;
    }

    if ( !IsMapFieldVisible( x, y ) ) {
	frame = unit->SeenFrame;
	if (frame == 255) {
	    return;
	}
    } else {
	frame = unit->SeenFrame = unit->Frame;
    }
#endif

    GraphicPlayerPixels(unit->Player,unit->Type->Sprite);
    x=Map2ScreenX(unit->X)+unit->IX;
    y=Map2ScreenY(unit->Y)+unit->IY;

#if 0
#else
    // FIXME: Johns: don't remember why I have draw this after the unit type.
    DrawSelection(unit,type,x,y);
#endif

    //
    //	Buildings under construction/upgrade/ready.
    //
    if( unit->Command.Action==UnitActionBuilded ) {
	if( unit->Constructed || VideoGraphicFrames(type->Sprite)<=1 ) {
	    DrawConstruction(type->OverlapFrame
		,frame&127
		,x+(type->TileWidth*TileSizeX)/2
		,y+(type->TileHeight*TileSizeY)/2);
	} else {
	    DrawUnitType(type,frame,x,y);
	}
    } else if( unit->Command.Action==UnitActionUpgradeTo ) {
	DrawUnitType(unit->Command.Data.UpgradeTo.What,(frame&128)+1,x,y);
    } else {
	DrawUnitType(type,frame,x,y);
    }

#if 0
    // FIXME: johns: ugly check here should be removed!
    if( unit->Command.Action!=UnitActionDie ) {
	DrawDecoration(unit,type,x,y);
	DrawSelection(unit,type,x,y);
    }
#else
    // FIXME: johns: ugly check here should be removed!
    if( unit->Command.Action!=UnitActionDie ) {
	DrawDecoration(unit,type,x,y);
    }
#endif
}

/**
**	Draw unit on map.
**
**	@param unit	Pointer to the unit.
*/
local void DrawUnit(Unit* unit)
{
    int x;
    int y;
    int r;
    UnitType* type;
    UnitStats* stats;

    if ( unit->Revealer ) return; // Revealers are not drawn

    type=unit->Type;

    x=Map2ScreenX(unit->X)+unit->IX;
    y=Map2ScreenY(unit->Y)+unit->IY;

    if( type->UnitType==UnitTypeFly ) {
	DrawShadow(unit,type,x,y);
    }

    DrawSelection(unit,type,x,y);

    GraphicPlayerPixels(unit->Player,unit->Type->Sprite);
    DrawUnitType(type,unit->Frame,x,y);

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
	if( type->CanAttack ) {
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

    // FIXME: johns: ugly check here should be removed!
    if( unit->Command.Action!=UnitActionDie ) {
	DrawDecoration(unit,type,x,y);
    }
}

/**
**	Draw all units on visible map.
**	FIXME: Must use the redraw tile flags in this function
*/
global void DrawUnits(void)
{
    Unit* unit;
    Unit* table[MAX_UNITS];
    int n;
    int i;

    //
    //	Select all units touching the viewpoint.
    //
    n=SelectUnits(MapX,MapY,MapX+MapWidth,MapY+MapHeight,table);

    //
    //	2a) corpse aren't in the cache.
    //
    for( i=0; i<NumUnits; ++i ) {
	unit=Units[i];
	// FIXME: this tries to draw all corps, ohje
	if( unit->Type->Vanishes || unit->Command.Action==UnitActionDie ) {
	    DrawUnit(unit);
	}
    }
    //
    //	2b) buildings
    //
    for( i=0; i<n; ++i ) {
	unit=table[i];
	if( !unit->Removed && UnitVisible(unit) ) {
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
