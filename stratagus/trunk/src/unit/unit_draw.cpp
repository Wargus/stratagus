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

// FIXME: not all variables of this file are here
// FIXME: perhaps split this file into two?

/*----------------------------------------------------------------------------
--	Functions
----------------------------------------------------------------------------*/

/**
**	Draw selected rectangle around the unit.
**
**	@param unit	Pointer to the unit.
**	@param type	Type of the unit.
**	@param x	Screen X position of the unit.
**	@param y	Screen Y position of the unit.
*/
local void DrawSelectionRectangle(Unit* unit,UnitType* type,int x,int y)
{
    int color;

    //
    //	Select color for the rectangle
    //
    if( unit->Selected || (unit->Blink&1) ) {
	if( unit->Player->Player==PlayerNumNeutral ) {
	    color=ColorYellow;
	} else if( unit->Player==ThisPlayer ) {
	    color=ColorGreen;
	} else {
	    color=ColorRed;
	}
    } else if( CursorBuilding && type->Building && unit->Player==ThisPlayer ) {
	// If building mark all buildings
	color=ColorGray;
    } else {
	return;
    }

    VideoDrawRectangle(color
	    ,x+(type->TileWidth*TileSizeX-type->BoxWidth)/2
	    ,y+(type->TileHeight*TileSizeY-type->BoxHeight)/2
	    ,type->BoxWidth
	    ,type->BoxHeight);
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

    RleSprite*	RleSprite;		/// loaded sprite images
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
    HealthSprite.RleSprite=LoadRleSprite(HealthSprite.File
		,HealthSprite.Width,HealthSprite.Height);
    ManaSprite.RleSprite=LoadRleSprite(ManaSprite.File
		,ManaSprite.Width,ManaSprite.Height);
    ShadowSprite.RleSprite=LoadRleSprite(ShadowSprite.File
		,ShadowSprite.Width,ShadowSprite.Height);
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
    //	Health bar on left side of unit.
    //
    stats=unit->Stats;
    if( ShowHealthBar ) {
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
	    VideoFillRectangle(color
		,x+(type->TileWidth*TileSizeX
			-type->BoxWidth)/2
		,y+(type->TileHeight*TileSizeY
			-type->BoxHeight)/2
		,2,(f*type->BoxHeight)/100);
	}
    }

    //
    //	Health dot on left side of unit.
    //
    if( ShowHealthDot ) {
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
	    DrawRleSpriteClipped(HealthSprite.RleSprite,n,x1,y1);
	}
    }

    //
    //	Mana bar on right side of unit.
    //
    if( ShowManaBar ) {
	if( type->CanCastSpell
		&& !(ShowNoFull && unit->Mana==255) ) {
	    f=(100*unit->Mana)/255;
	    VideoFillRectangle(ColorBlue
		,x+(type->TileWidth*TileSizeX
			+type->BoxWidth)/2
		,y+(type->TileHeight*TileSizeY
			-type->BoxHeight)/2
		,2,(f*type->BoxHeight)/100);
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
	    } else if( f && ManaSprite.Width*4<ManaSprite.RleSprite->Width ) {
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
	    DrawRleSpriteClipped(ManaSprite.RleSprite,n,x1,y1);
	}
    }

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

    DrawRleSpriteClipped(ShadowSprite.RleSprite,i
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
	VideoDrawRectangle(ColorGray
	    ,Map2ScreenX(x1)+TileSizeX/2-3,Map2ScreenY(y1)+TileSizeY/2-3
	    ,6,6);
	while( x1!=x2 ) {
	    x1+=xstep;
	    VideoDrawRectangle(ColorGray
		,Map2ScreenX(x1)+TileSizeX/2-3,Map2ScreenY(y1)+TileSizeY/2-3
		,6,6);
	}
	return;
    }

    if( dx==0 ) {		// vertical line
	// CLIPPING
	VideoDrawRectangle(ColorGray
	    ,Map2ScreenX(x1)+TileSizeX/2-3,Map2ScreenY(y1)+TileSizeY/2-3
	    ,6,6);
	while( y1!=y2 ) {
	    y1++;
	    VideoDrawRectangle(ColorGray
		,Map2ScreenX(x1)+TileSizeX/2-3,Map2ScreenY(y1)+TileSizeY/2-3
		,6,6);
	}
	return;
    }

    VideoDrawRectangle(ColorGray
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
	    VideoDrawRectangle(ColorGray
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
	    VideoDrawRectangle(ColorGray
		,Map2ScreenX(x1)+TileSizeX/2-3,Map2ScreenY(y1)+TileSizeY/2-3
		,6,6);
	}
	return;
    }

				// diagonal line
    while( y1!=y2) {
	x1+=xstep;
	y1++;
	VideoDrawRectangle(ColorGray
	    ,Map2ScreenX(x1)+TileSizeX/2-3,Map2ScreenY(y1)+TileSizeY/2-3
	    ,6,6);
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
    UnitType* type;
    int frame;
    int n_frame;

    // FIXME: This should I rewrite, without checks here!!

    type=unit->Type;
    x = unit->X;
    y = unit->Y;

    // FIXME: johns: this isn't 100% correct, building which are partly
    // FIXME: johns: under the fog are shown partly.

    // FIXME: There is already a check in the main loop UnitVisibile!
    if ( !MAPEXPLORED( x, y ) ) {
	return;
    }

    if ( !TheMap.NoFogOfWar && !MAPVISIBLE( x, y ) ) {
	frame = unit->SeenFrame;
	if (frame == 255) {
	    return;
	}
    } else {
	frame = unit->SeenFrame = unit->Frame;
    }

#if 0
    if( type->Type==UnitOilPlatformHuman || type->Type==UnitOilPlatformOrc ) {
	DebugLevel0("%d -> %d\n",unit->Frame,frame);
    }
#endif

    n_frame = 0;
    if ((frame & 128) == 0 && unit->Rs > 50) {
	n_frame = 128; // fancy buildings
    }

    PlayerPixels(unit->Player);
    x=Map2ScreenX(unit->X)+unit->IX;
    y=Map2ScreenY(unit->Y)+unit->IY;

    //
    //	Buildings under construction/upgrade/ready.
    //
    if( unit->Command.Action==UnitActionBuilded ) {
	if( unit->Constructed || type->RleSprite->NumFrames<=1 ) {
	    DrawConstruction(type->OverlapFrame
		,frame
		,x+(type->TileWidth*TileSizeX)/2
		,y+(type->TileHeight*TileSizeY)/2);
	} else {
#if 0
	    DebugLevel0("Remove this %d\n",n_frame);
	    if ( strcmp(type->Ident,"dark-portal") == 0
		||  strcmp(type->Ident,"runestone") == 0 )
	    //FIXME: dark-portal and runestone haven't reqiured frames, so we draw construction instead
	    DrawConstruction(type->OverlapFrame
		,frame
		,x+(type->TileWidth*TileSizeX)/2
		,y+(type->TileHeight*TileSizeY)/2);
	    else
#endif
	    DrawUnitType(type,frame+n_frame,x,y);
	}
    } else if( unit->Command.Action==UnitActionUpgradeTo ) {
	DrawUnitType(unit->Command.Data.UpgradeTo.What,1+n_frame,x,y);
    } else {
	DrawUnitType(type,frame+n_frame,x,y);
    }

    // FIXME: johns: ugly check here should be removed! vanish could be used
    if( unit->Command.Action!=UnitActionDie ) {
	DrawDecoration(unit,type,x,y);
	DrawSelectionRectangle(unit,type,x,y);
    }
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

    type=unit->Type;

    x=Map2ScreenX(unit->X)+unit->IX;
    y=Map2ScreenY(unit->Y)+unit->IY;

    if( type->UnitType==UnitTypeFly ) {
	DrawShadow(unit,type,x,y);
    }

    DrawSelectionRectangle(unit,type,x,y);

    PlayerPixels(unit->Player);
    DrawUnitType(type,unit->Frame,x,y);

    stats=unit->Stats;

    //
    //	For debug draw sight, react and attack range!
    //
    if( NumSelected==1 && unit->Selected ) {
	if( ShowSightRange ) {
	    VideoDrawRectangle(ColorGreen
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
		    VideoDrawRectangle(ColorBlue
			,x+TileSizeX/2-r*TileSizeX
			,y+TileSizeY/2-r*TileSizeY
			,r*TileSizeX*2
			,r*TileSizeY*2);
		}
	    }
	    if( ShowAttackRange && stats->AttackRange ) {
		VideoDrawRectangle(ColorRed
		    ,x+TileSizeX/2-stats->AttackRange*TileSizeX
		    ,y+TileSizeY/2-stats->AttackRange*TileSizeY
		    ,stats->AttackRange*TileSizeX*2
		    ,stats->AttackRange*TileSizeY*2);
	    }
	}
    }

    //
    //	For debug draw destination. FIXME: should become orders
    //
    if( ShowOrders && unit->Selected && (KeyModifiers&ModifierShift)) {
	DrawPath(unit);
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
