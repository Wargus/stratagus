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
/**@name map_fog.c	-	The map fog of war handling. */
//
//	(c) Copyright 1999-2002 by Lutz Sammer and Vladi Shabanski
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
#include "player.h"
#include "unittype.h"
#include "unit.h"
#include "map.h"
#include "minimap.h"
#include "ui.h"

#if defined(DEBUG) && defined(TIMEIT)
#include "rdtsc.h"
#endif

/*----------------------------------------------------------------------------
--	Declarations
----------------------------------------------------------------------------*/

#ifdef DEBUG
#define noTIMEIT			/// defined time function
#endif

    /// True if this is the fog color
#define COLOR_FOG_P(x) ((x) == 239)
    /// Solid fog color number in global palette
#define COLOR_FOG (0)

/**
**	Do unroll 8x
**
**	@param x	Index passed to UNROLL2 incremented by 2.
*/
#define UNROLL8(x)	\
    UNROLL2((x)+0);	\
    UNROLL2((x)+2);	\
    UNROLL2((x)+4);	\
    UNROLL2((x)+6)

/**
**	Do unroll 16x
**
**	@param x	Index passed to UNROLL8 incremented by 8.
*/
#define UNROLL16(x)	\
    UNROLL8((x)+ 0);	\
    UNROLL8((x)+ 8)

/**
**	Do unroll 32x
**
**	@param x	Index passed to UNROLL8 incremented by 8.
*/
#define UNROLL32(x)	\
    UNROLL8((x)+ 0);	\
    UNROLL8((x)+ 8);	\
    UNROLL8((x)+16);	\
    UNROLL8((x)+24)

/*----------------------------------------------------------------------------
--	Variables
----------------------------------------------------------------------------*/

global int OriginalFogOfWar;		/// Use original style fog of war
global int FogOfWarContrast=50;		/// Contrast of fog of war
global int FogOfWarBrightness=10;	/// Brightness of fog of war
global int FogOfWarSaturation=40;	/// Saturation of fog of war

#define MapFieldCompletelyVisible   0x0001  /// Field completely visible
#define MapFieldPartiallyVisible    0x0002  /// Field partially visible

/**
**	Mapping for fog of war tiles.
*/
local const int FogTable[16] = {
     0,11,10, 2,  13, 6, 0, 3,  12, 0, 4, 1,  8, 9, 7, 0,
};

/**
**	Draw unexplored area function pointer. (display and video mode independ)
*/
local void (*VideoDrawUnexplored)(const GraphicData*,int,int);

/**
**	Draw fog of war function pointer. (display and video mode independ)
*/
local void (*VideoDrawFog)(const GraphicData*,int,int);

/**
**	Draw only fog of war function pointer. (display and video mode independ)
*/
local void (*VideoDrawOnlyFog)(const GraphicData*,int x,int y);

/**
**	Precalculated fog of war alpha table.
*/
local void* FogOfWarAlphaTable;

/*----------------------------------------------------------------------------
--	Functions
----------------------------------------------------------------------------*/

#ifdef NEW_FOW
/**
**	Mask
*/
local int MapExploredMask(void)
{
    return 8;
}

/**
**	Mask
*/
local int MapVisibleMask(void)
{
    return 8;
}

/**
**	Mark the sight of unit. (Explore and make visible.)
**
**	@param player	Player to mark sight.
**	@param tx	X center position.
**	@param ty	Y center position.
**	@param range	Radius to mark.
*/
global void MapMarkSight(const Player* player,int tx,int ty,int range)
{
    int i;
    int x;
    int y;
    int height;
    int width;
    int v;
    int p;

    if( !range ) {			// zero sight range is zero sight range
	DebugLevel0Fn("Zero sight range\n");
	return;
    }

    x=tx-range;
    y=ty-range;
    width=height=range+range;

    //	Clipping
    if( y<0 ) {
	height+=y;
	y=0;
    }
    if( x<0 ) {
	width+=x;
	x=0;
    }
    if( y+height>=TheMap.Height ) {
	height=TheMap.Height-y-1;
    }
    if( x+width>=TheMap.Width ) {
	width=TheMap.Width-x-1;
    }

    p=player->Player;
    ++range;
    // FIXME: Can be speed optimized, no * += ...
    while( height-->=0 ) {
	for( i=x; i<=x+width; ++i ) {
	    // FIXME: Can use quadrat table!
	    if( ((i-tx)*(i-tx)+(y-ty)*(y-ty))<=range*range ) {
		v=TheMap.Fields[i+y*TheMap.Width].Visible[p];
		switch( v ) {
		    case 0:		// Unexplored
		    case 1:		// Unseen
			if( player==ThisPlayer ) {
			    MapMarkSeenTile(i,y);
			}
			// FIXME: mark for screen update
			TheMap.Fields[i+y*TheMap.Width].Visible[p]=2;
			break;
		    case 255:		// Overflow
			DebugLevel0Fn("Visible overflow\n");
			break;

		    default:		// seen -> seen
			TheMap.Fields[i+y*TheMap.Width].Visible[p]=v+1;
			break;
		}
	    }
	}
	++y;
    }
}

/**
**	Mark the new sight of unit. (Explore and make visible.)
**
**	@param player	Player to mark sight.
**	@param tx	X center position.
**	@param ty	Y center position.
**	@param range	Radius to mark.
*/
global void MapMarkNewSight(const Player* player,int tx,int ty,int range
	,int dx __attribute__((unused)),int dy __attribute__((unused)))
{
    // FIXME: must write this
    MapMarkSight(player,tx,ty,range);
}

#else

/**
**	Mark the sight of unit. (Explore and make visible.)
**
**	@param tx	X center position.
**	@param ty	Y center position.
**	@param range	Radius to mark.
*/
global void MapMarkSight(int tx,int ty,int range)
{
    int i;
    int x;
    int y;
    int height;
    int width;

    if( !range ) {			// zero sight range is zero sight range
	DebugLevel0Fn("Zero sight range\n");
	return;
    }

    x=tx-range;
    y=ty-range;
    width=height=range+range;

    //	Clipping
    if( y<0 ) {
	height+=y;
	y=0;
    }
    if( x<0 ) {
	width+=x;
	x=0;
    }
    if( y+height>=TheMap.Height ) {
	height=TheMap.Height-y-1;
    }
    if( x+width>=TheMap.Width ) {
	width=TheMap.Width-x-1;
    }

    ++range;
    while( height-->=0 ) {
	for( i=x; i<=x+width; ++i ) {
	    // FIXME: Can use quadrat table!
	    if( ((i-tx)*(i-tx)+(y-ty)*(y-ty))<=range*range ) {
		// FIXME: can combine more bits
		if( !IsMapFieldVisible(i,y) ) {
		    TheMap.Fields[i+y*TheMap.Width].Flags |= MapFieldExplored;
		    TheMap.Visible[0][(i+y*TheMap.Width)/32]
			    |= 1<<((i+y*TheMap.Width)%32);
		    MapMarkSeenTile(i,y);
		    //MustRedrawRow[y]=NEW_MAPDRAW;
		    //MustRedrawTile[y*MapWidth+i]=NEW_MAPDRAW;
		}
	    }
	}
	++y;
    }
}

/**
**	Mark the new sight of unit. (Explore and make visible.)
**
**	@param tx	X map tile position of center.
**	@param ty	Y map tile position of center.
**	@param range	Radius to mark.
**	@param dx	Unused: Delta in tiles in X direction.
**	@param dy	Unused: Delta in tiles in Y direction.
*/
global void MapMarkNewSight(int tx,int ty,int range
	,int dx __attribute__((unused)),int dy __attribute__((unused)))
{
    // FIXME: must write this
    MapMarkSight(tx,ty,range);
}

#endif

/**
**	Update the fog of war, for the view point. Called from UpdateDisplay.
**
**	@param x	Viewpoint X map tile position
**	@param y	Viewpoint Y map tile position
**
**	@todo	FIXME: should be handled complete in UpdateDisplay.
*/
global void MapUpdateFogOfWar(int x,int y)
{
    x=y=0;
#if 0
    Unit* unit;
    Unit* table[UnitMax];
    char *redraw_row;
    char *redraw_tile;
    int n;
    int i;
    int sx,sy,ex,ey,dx,dy;
#ifndef NEW_FOW
    int vis;
    int last;
#endif

    // Tiles not visible last frame but are this frame must be redrawn.
    redraw_row=MustRedrawRow;
    redraw_tile=MustRedrawTile;

    ex=TheUI.MapEndX;
    sy=y*TheMap.Width;
    dy=TheUI.MapY;
    ey=TheUI.MapEndY;

    while( dy<=ey ) {
	sx=x+sy;
	dx=TheUI.MapX;
#ifdef NEW_FOW
	while( dx<=ex ) {
#ifdef NEW_MAPDRAW
	    *redraw_row=NEW_MAPDRAW;
	    *redraw_tile=NEW_MAPDRAW;
#else
	    *redraw_row=*redraw_tile=1;
#endif

	    ++redraw_tile;
	    ++sx;
	    dx+=TileSizeX;
	}
#else
	while( dx<=ex ) {
	    last=TheMap.Fields[sx].VisibleLastFrame;
#ifdef NEW_FOW
	    vis=TheMap.Fields[sx].Visible&(1<<ThisPlayer->Player);
#else
	    vis=TheMap.Fields[sx].Flags&MapFieldVisible;
#endif
	    if( vis && (!last || last&MapFieldPartiallyVisible) ) {
#ifdef NEW_MAPDRAW
		*redraw_row=NEW_MAPDRAW;
		*redraw_tile=NEW_MAPDRAW;
#else
		*redraw_row=*redraw_tile=1;
#endif
	    }

	    ++redraw_tile;
	    ++sx;
	    dx+=TileSizeX;
	}
#endif
	++redraw_row;
	sy+=TheMap.Width;
	dy+=TileSizeY;
    }

    // Buildings under fog of war must be redrawn
    n=SelectUnits(MapX,MapY,MapX+MapWidth,MapY+MapHeight,table);

    for( i=0; i<n; ++i ) {
	unit=table[i];
	if( unit->Type->Building && !unit->Removed
		&& UnitVisibleOnScreen(unit) ) {
	    CheckUnitToBeDrawn(unit);
	}
    }
#endif
}

/**
**	Update fog of war.
*/
global void UpdateFogOfWarChange(void)
{
    int x;
    int y;
    int w;

    //
    //	Mark all explored fields as visible.
    //
    if ( TheMap.NoFogOfWar ) {
	w=TheMap.Width;
	for( y=0; y<TheMap.Height; y++ ) {
	    for( x=0; x<TheMap.Width; ++x ) {
		if ( IsMapFieldExplored(x,y) ) {
#ifdef NEW_FOW
		    int p;

		    for( p=0; p<PlayerMax; ++p ) {
			if( TheMap.Fields[x+y*w].Visible[p] ) {
			    DebugLevel0Fn("Don't work look\n");
			}
			TheMap.Fields[x+y*w].Visible[p]=2;
		    }
#else
		    TheMap.Visible[0][(x+y*w)/32] |= 1<<((x+y*w)%32);
#endif
		    MapMarkSeenTile( x,y );
		}
	    }
	}
    }

    MarkDrawEntireMap();
}

/**
**	Update visible of the map.
**
**	@todo	This function could be improved in speed and functionality.
*/
global void MapUpdateVisible(void)
{
    int x;
    int y;
    Unit* unit;
    Unit** units;
    Unit* mine;
    int nunits,i;
#ifdef DEBUG
    unsigned long t;
#endif

    // No fog - only update revealers for holy vision
    if ( TheMap.NoFogOfWar ) {
	nunits=ThisPlayer->TotalNumUnits;
	units=ThisPlayer->Units;
	for( i=0; i<nunits; ++i ) {
	    unit=units[i];
	    x=unit->X+unit->Type->TileWidth/2;
	    y=unit->Y+unit->Type->TileHeight/2;
	    if( unit->Removed && unit->Revealer ) {
#ifdef NEW_FOW
		MapMarkSight(unit->Player,x,y,10);
#else
		MapMarkSight(x,y,10);
#endif
	    }
	}
	return;
    }

#ifdef NEW_FOW
    return;
#endif

    // FIXME: rewrite this function, faster and better
#ifdef DEBUG
    t=GetTicks();
#endif

    //
    //	Clear all visible flags.
    //
    memset(TheMap.Visible[0],0,(TheMap.Width*TheMap.Height)/8);

    DebugLevel3Fn("Ticks Clear %lu\n" _C_ GetTicks()-t);

    MarkDrawEntireMap();

    DebugLevel3Fn("Ticks Mark  %lu\n" _C_ GetTicks()-t);

    //
    //	Mark all units visible range.
    //
    nunits=ThisPlayer->TotalNumUnits;
    units=ThisPlayer->Units;
    for( i=0; i<nunits; i++ ) {
	unit=units[i];
	x=unit->X+unit->Type->TileWidth/2;
	y=unit->Y+unit->Type->TileHeight/2;
	if( unit->Removed ) {
	    if( unit->Revealer ) {
#ifdef NEW_FOW
		MapMarkSight(unit->Player,x,y,10);
#else
		MapMarkSight(x,y,10);
#endif
		continue;
	    }
	    //
	    //	If peon is in the mine, the mine has a sight range too.
	    //  This is quite dirty code...
	    //  This is not a big deal as far as only mines are
	    //  concerned, but for more units (like parasited ones
	    //  in *craft), maybe we should create a dedicated queue...
	    if( unit->Orders[0].Action==UnitActionMineGold ) {
	        mine=GoldMineOnMap(unit->X,unit->Y);
		if( mine ) {  // Somtimes, the peon is at home :).
#ifdef NEW_FOW
		    MapMarkSight(unit->Player,mine->X+mine->Type->TileWidth/2
			    ,mine->Y+mine->Type->TileHeight/2
			    ,mine->Stats->SightRange);
#else
		    MapMarkSight(mine->X+mine->Type->TileWidth/2
				 ,mine->Y+mine->Type->TileHeight/2
				 ,mine->Stats->SightRange);
#endif
		}
	    } else {
	        continue;
	    }
	}

	if( unit->Orders[0].Action==UnitActionBuilded ) {
#ifdef NEW_FOW
	    MapMarkSight(unit->Player,x,y,3);
#else
	    MapMarkSight(x,y,3);
#endif
	} else {
#ifdef NEW_FOW
	    MapMarkSight(unit->Player,x,y,unit->Stats->SightRange);
#else
	    MapMarkSight(x,y,unit->Stats->SightRange);
#endif
	}
    }

    DebugLevel3Fn("Ticks Total %lu\n" _C_ GetTicks()-t);
}

/*----------------------------------------------------------------------------
--	Draw fog solid
----------------------------------------------------------------------------*/

// Routines for 8 bit displays .. --------------------------------------------

/**
**	Fast draw solid fog of war 32x32 tile for 8 bpp video modes.
**
**	@param data	pointer to tile graphic data.
**	@param x	X position into video memory.
**	@param y	Y position into video memory.
*/
global void VideoDraw8Fog32Solid(const GraphicData* data,int x,int y)
{
    const unsigned char* sp;
    const unsigned char* gp;
    VMemType8* dp;
    int da;

    sp=data;
    gp=sp+TileSizeY*TileSizeX;
    dp=VideoMemory8+x+y*VideoWidth;
    da=VideoWidth;

    while( sp<gp ) {
#undef UNROLL1
#define UNROLL1(x)	\
	if(COLOR_FOG_P(sp[x])) {		\
	    dp[x]=((VMemType8*)TheMap.TileData->Pixels)[COLOR_FOG];	\
	}

#undef UNROLL2
#define UNROLL2(x)	\
	UNROLL1(x+0);

	UNROLL32(0);

	sp+=TileSizeX;
	dp+=da;

#undef UNROLL2
#define UNROLL2(x)	\
	UNROLL1(x+1);

	UNROLL32(0);

	sp+=TileSizeX;
	dp+=da;
    }
}

/**
**	Fast draw solid 100% fog of war 32x32 tile for 8 bpp video modes.
**
**	100% fog of war -- i.e. raster	10101.
**					01010 etc...
**
**	@param data	pointer to tile graphic data.
**	@param x	X position into video memory
**	@param y	Y position into video memory
*/
global void VideoDraw8OnlyFog32Solid(const GraphicData* data __attribute__((unused)),int x,int y)
{
    const VMemType8* gp;
    VMemType8* dp;
    int da;

    dp=VideoMemory8+x+y*VideoWidth;
    gp=dp+VideoWidth*TileSizeX;
    da=VideoWidth;
    while( dp<gp ) {
#undef UNROLL2
#define UNROLL2(x)		\
	dp[x+0]=((VMemType8*)TheMap.TileData->Pixels)[COLOR_FOG];
	UNROLL32(0);
	dp+=da;

#undef UNROLL2
#define UNROLL2(x)		\
	dp[x+1]=((VMemType8*)TheMap.TileData->Pixels)[COLOR_FOG];
	UNROLL32(0);
	dp+=da;
    }
}

/**
**	Fast draw solid unexplored 32x32 tile for 8 bpp video modes.
**
**	@param data	pointer to tile graphic data
**	@param x	X position into video memory
**	@param y	Y position into video memory
*/
global void VideoDraw8Unexplored32Solid(const GraphicData* data,int x,int y)
{
    const unsigned char* sp;
    const unsigned char* gp;
    VMemType8* dp;
    int da;

    sp=data;
    gp=sp+TileSizeY*TileSizeX;
    dp=VideoMemory8+x+y*VideoWidth;
    da=VideoWidth;

    while( sp<gp ) {
#undef UNROLL1
#define UNROLL1(x)	\
	if(COLOR_FOG_P(sp[x]) ) {		\
	    dp[x]=((VMemType8*)TheMap.TileData->Pixels)[COLOR_FOG];	\
	}

#undef UNROLL2
#define UNROLL2(x)	\
	UNROLL1(x+0);	\
	UNROLL1(x+1);

	UNROLL32(0);
	sp+=TileSizeX;
	dp+=da;

	UNROLL32(0);
	sp+=TileSizeX;
	dp+=da;

    }
}

// Routines for 16 bit displays .. -------------------------------------------

/**
**	Fast draw solid fog of war 32x32 tile for 16 bpp video modes.
**
**	@param data	pointer to tile graphic data.
**	@param x	X position into video memory.
**	@param y	Y position into video memory.
*/
global void VideoDraw16Fog32Solid(const GraphicData* data,int x,int y)
{
    const unsigned char* sp;
    const unsigned char* gp;
    VMemType16* dp;
    int da;

    sp=data;
    gp=sp+TileSizeY*TileSizeX;
    dp=VideoMemory16+x+y*VideoWidth;
    da=VideoWidth;

    while( sp<gp ) {
#undef UNROLL1
#define UNROLL1(x)	\
	if(COLOR_FOG_P(sp[x])) {		\
	    dp[x]=((VMemType16*)TheMap.TileData->Pixels)[COLOR_FOG];	\
	}

#undef UNROLL2
#define UNROLL2(x)	\
	UNROLL1(x+0);

	UNROLL32(0);

	sp+=TileSizeX;
	dp+=da;

#undef UNROLL2
#define UNROLL2(x)	\
	UNROLL1(x+1);

	UNROLL32(0);

	sp+=TileSizeX;
	dp+=da;
    }
}

/**
**	Fast draw solid 100% fog of war 32x32 tile for 16 bpp video modes.
**
**	100% fog of war -- i.e. raster	10101.
**					01010 etc...
**
**	@param data	pointer to tile graphic data.
**	@param x	X position into video memory
**	@param y	Y position into video memory
*/
global void VideoDraw16OnlyFog32Solid(const GraphicData* data __attribute__((unused)),int x,int y)
{
    const VMemType16* gp;
    VMemType16* dp;
    int da;

    dp=VideoMemory16+x+y*VideoWidth;
    gp=dp+VideoWidth*TileSizeX;
    da=VideoWidth;
    while( dp<gp ) {
#undef UNROLL2
#define UNROLL2(x)		\
	dp[x+0]=((VMemType16*)TheMap.TileData->Pixels)[COLOR_FOG];
	UNROLL32(0);
	dp+=da;

#undef UNROLL2
#define UNROLL2(x)		\
	dp[x+1]=((VMemType16*)TheMap.TileData->Pixels)[COLOR_FOG];
	UNROLL32(0);
	dp+=da;
    }
}

/**
**	Fast draw solid unexplored 32x32 tile for 16 bpp video modes.
**
**	@param data	pointer to tile graphic data
**	@param x	X position into video memory
**	@param y	Y position into video memory
*/
global void VideoDraw16Unexplored32Solid(const GraphicData* data,int x,int y)
{
    const unsigned char* sp;
    const unsigned char* gp;
    VMemType16* dp;
    int da;

    sp=data;
    gp=sp+TileSizeY*TileSizeX;
    dp=VideoMemory16+x+y*VideoWidth;
    da=VideoWidth;

    while( sp<gp ) {
#undef UNROLL1
#define UNROLL1(x)	\
	if(COLOR_FOG_P(sp[x]) ) {		\
	    dp[x]=((VMemType16*)TheMap.TileData->Pixels)[COLOR_FOG];	\
	}

#undef UNROLL2
#define UNROLL2(x)	\
	UNROLL1(x+0);	\
	UNROLL1(x+1);

	UNROLL32(0);
	sp+=TileSizeX;
	dp+=da;

	UNROLL32(0);
	sp+=TileSizeX;
	dp+=da;
    }
}

// Routines for 24 bit displays .. -------------------------------------------

/**
**	Fast draw solid fog of war 32x32 tile for 24 bpp video modes.
**
**	@param data	pointer to tile graphic data.
**	@param x	X position into video memory.
**	@param y	Y position into video memory.
*/
global void VideoDraw24Fog32Solid(const GraphicData* data,int x,int y)
{
    const unsigned char* sp;
    const unsigned char* gp;
    VMemType24* dp;
    int da;

    sp=data;
    gp=sp+TileSizeY*TileSizeX;
    dp=VideoMemory24+x+y*VideoWidth;
    da=VideoWidth;

    while( sp<gp ) {
#undef UNROLL1
#define UNROLL1(x)	\
	if(COLOR_FOG_P(sp[x])) {	\
	    dp[x]=((VMemType24*)TheMap.TileData->Pixels)[COLOR_FOG];	\
	}

#undef UNROLL2
#define UNROLL2(x)	\
	UNROLL1(x+0);

	UNROLL32(0);

	sp+=TileSizeX;
	dp+=da;

#undef UNROLL2
#define UNROLL2(x)	\
	UNROLL1(x+1);

	UNROLL32(0);

	sp+=TileSizeX;
	dp+=da;
    }
}

/**
**	Fast draw solid 100% fog of war 32x32 tile for 24 bpp video modes.
**
**	100% fog of war -- i.e. raster	10101.
**					01010 etc...
**
**	@param data	pointer to tile graphic data.
**	@param x	X position into video memory
**	@param y	Y position into video memory
*/
global void VideoDraw24OnlyFog32Solid(const GraphicData* data __attribute__((unused)),int x,int y)
{
    const VMemType24* gp;
    VMemType24* dp;
    int da;

    dp=VideoMemory24+x+y*VideoWidth;
    gp=dp+VideoWidth*TileSizeX;
    da=VideoWidth;
    while( dp<gp ) {
#undef UNROLL2
#define UNROLL2(x)		\
	dp[x+0]=((VMemType24*)TheMap.TileData->Pixels)[COLOR_FOG];
	UNROLL32(0);
	dp+=da;

#undef UNROLL2
#define UNROLL2(x)		\
	dp[x+1]=((VMemType24*)TheMap.TileData->Pixels)[COLOR_FOG];
	UNROLL32(0);
	dp+=da;
    }
}

/**
**	Fast draw solid unexplored 32x32 tile for 24 bpp video modes.
**
**	@param data	pointer to tile graphic data
**	@param x	X position into video memory
**	@param y	Y position into video memory
*/
global void VideoDraw24Unexplored32Solid(const GraphicData* data,int x,int y)
{
    const unsigned char* sp;
    const unsigned char* gp;
    VMemType24* dp;
    int da;

    sp=data;
    gp=sp+TileSizeY*TileSizeX;
    dp=VideoMemory24+x+y*VideoWidth;
    da=VideoWidth;

    while( sp<gp ) {
#undef UNROLL1
#define UNROLL1(x)	\
	if(COLOR_FOG_P(sp[x])) {		\
	    dp[x]=((VMemType24*)TheMap.TileData->Pixels)[COLOR_FOG];	\
	}

#undef UNROLL2
#define UNROLL2(x)	\
	UNROLL1(x+0);	\
	UNROLL1(x+1);

	UNROLL32(0);
	sp+=TileSizeX;
	dp+=da;

	UNROLL32(0);
	sp+=TileSizeX;
	dp+=da;
    }
}

// Routines for 32 bit displays .. -------------------------------------------

/**
**	Fast draw solid fog of war 32x32 tile for 32 bpp video modes.
**
**	@param data	pointer to tile graphic data.
**	@param x	X position into video memory.
**	@param y	Y position into video memory.
*/
global void VideoDraw32Fog32Solid(const GraphicData* data,int x,int y)
{
    const unsigned char* sp;
    const unsigned char* gp;
    VMemType32* dp;
    int da;

    sp=data;
    gp=sp+TileSizeY*TileSizeX;
    dp=VideoMemory32+x+y*VideoWidth;
    da=VideoWidth;

    while( sp<gp ) {
#undef UNROLL1
#define UNROLL1(x)	\
	if(COLOR_FOG_P(sp[x])) {	\
	    dp[x]=((VMemType16*)TheMap.TileData->Pixels)[COLOR_FOG];	\
	}

#undef UNROLL2
#define UNROLL2(x)	\
	UNROLL1(x+0);

	UNROLL32(0);

	sp+=TileSizeX;
	dp+=da;

#undef UNROLL2
#define UNROLL2(x)	\
	UNROLL1(x+1);

	UNROLL32(0);

	sp+=TileSizeX;
	dp+=da;
    }
}

/**
**	Fast draw solid 100% fog of war 32x32 tile for 32 bpp video modes.
**
**	100% fog of war -- i.e. raster	10101.
**					01010 etc...
**
**	@param data	pointer to tile graphic data.
**	@param x	X position into video memory
**	@param y	Y position into video memory
*/
global void VideoDraw32OnlyFog32Solid(const GraphicData* data __attribute__((unused)),int x,int y)
{
    const VMemType32* gp;
    VMemType32* dp;
    int da;

    dp=VideoMemory32+x+y*VideoWidth;
    gp=dp+VideoWidth*TileSizeX;
    da=VideoWidth;
    while( dp<gp ) {
#undef UNROLL2
#define UNROLL2(x)		\
	dp[x+0]=((VMemType32*)TheMap.TileData->Pixels)[COLOR_FOG];
	UNROLL32(0);
	dp+=da;

#undef UNROLL2
#define UNROLL2(x)		\
	dp[x+1]=((VMemType32*)TheMap.TileData->Pixels)[COLOR_FOG];
	UNROLL32(0);
	dp+=da;
    }
}

/**
**	Fast draw solid unexplored 32x32 tile for 32 bpp video modes.
**
**	@param data	pointer to tile graphic data
**	@param x	X position into video memory
**	@param y	Y position into video memory
*/
global void VideoDraw32Unexplored32Solid(const GraphicData* data,int x,int y)
{
    const unsigned char* sp;
    const unsigned char* gp;
    VMemType32* dp;
    int da;

    sp=data;
    gp=sp+TileSizeY*TileSizeX;
    dp=VideoMemory32+x+y*VideoWidth;
    da=VideoWidth;

    while( sp<gp ) {
#undef UNROLL1
#define UNROLL1(x)	\
	if(COLOR_FOG_P(sp[x])) {		\
	    dp[x]=((VMemType32*)TheMap.TileData->Pixels)[COLOR_FOG];	\
	}

#undef UNROLL2
#define UNROLL2(x)	\
	UNROLL1(x+0);	\
	UNROLL1(x+1);

	UNROLL32(0);
	sp+=TileSizeX;
	dp+=da;

	UNROLL32(0);
	sp+=TileSizeX;
	dp+=da;
    }
}

/**
**	Fast draw solid unexplored tile.
**
**	@param data	pointer to tile graphic data
**	@param x	X position into video memory
**	@param y	Y position into video memory
*/
#ifdef USE_OPENGL
global void VideoDrawUnexploredSolidOpenGL(
    const GraphicData* data __attribute__((unused)),
    int x __attribute__((unused)),int y __attribute__((unused)))
{
}
#endif

/*----------------------------------------------------------------------------
--	Draw real fog :-)
----------------------------------------------------------------------------*/

// Routines for 8 bit displays .. --------------------------------------------

/**
**	Fast draw alpha fog of war 32x32 tile for 8 bpp video modes.
**
**	@param data	pointer to tile graphic data.
**	@param x	X position into video memory
**	@param y	Y position into video memory
**
**	The random effect is commented out
*/
global void VideoDraw8Fog32Alpha(const GraphicData* data,int x,int y)
{
    const unsigned char* sp;
    const unsigned char* gp;
    VMemType8* dp;
    VMemType8 fog;
    int da;

    sp=data;
    gp=sp+TileSizeY*TileSizeX;
    dp=VideoMemory8+x+y*VideoWidth;
    da=VideoWidth;
    fog=((VMemType8*)TheMap.TileData->Pixels)[COLOR_FOG];

    while( sp<gp ) {
#undef UNROLL1
#define UNROLL1(x)	\
	if( COLOR_FOG_P(sp[x]) ) {	\
	    if (dp[x] != fog) { \
		dp[x]=((VMemType8*)FogOfWarAlphaTable)[dp[x]]; \
	    } \
	} \

#undef UNROLL2
#define UNROLL2(x)	\
	UNROLL1(x+0);	\
	UNROLL1(x+1);	\

	UNROLL32(0);
	sp+=TileSizeX;
	dp+=da;

	UNROLL32(0);
	sp+=TileSizeX;
	dp+=da;
    }
}

/**
**	Fast draw 100% fog of war 32x32 tile for 8 bpp video modes.
**
**	100% fog of war -- i.e. raster	10101.
**					01010 etc...
**
**	@param data	pointer to tile graphic data.
**	@param x	X position into video memory
**	@param y	Y position into video memory
**
**	The random effect is commented out
*/
global void VideoDraw8OnlyFog32Alpha(const GraphicData* data __attribute__((unused)),int x,int y)
{
    const VMemType8* gp;
    VMemType8* dp;
    VMemType8 fog;
    int da;

    dp=VideoMemory8+x+y*VideoWidth;
    gp=dp+VideoWidth*TileSizeY;
    da=VideoWidth;
    fog=((VMemType8*)TheMap.TileData->Pixels)[COLOR_FOG];

    while( dp<gp ) {
#undef UNROLL1
#define UNROLL1(x)	\
	if (dp[x] != fog) { \
	    dp[x]=((VMemType8*)FogOfWarAlphaTable)[dp[x]];	\
	} \

#undef UNROLL2
#define UNROLL2(x)	\
	UNROLL1(x+0);	\
	UNROLL1(x+1);	\

	UNROLL32(0);
	dp+=da;

	UNROLL32(0);
	dp+=da;
    }
}

// Routines for 16 bit displays .. -------------------------------------------

/**
**	Fast draw alpha fog of war 32x32 tile for 16 bpp video modes.
**
**	@param data	pointer to tile graphic data.
**	@param x	X position into video memory
**	@param y	Y position into video memory
**
**	The random effect is commented out
*/
global void VideoDraw16Fog32Alpha(const GraphicData* data,int x,int y)
{
    const unsigned char* sp;
    const unsigned char* gp;
    VMemType16* dp;
    VMemType16 fog;
    int da;
    //int o;

    sp=data;
    gp=sp+TileSizeY*TileSizeX;
    dp=VideoMemory16+x+y*VideoWidth;
    da=VideoWidth;
    fog=((VMemType16*)TheMap.TileData->Pixels)[COLOR_FOG];

    while( sp<gp ) {
	//static int a=1234567;
	//o=rand();

#undef UNROLL1
#define UNROLL1(x)	\
	if (dp[x] != fog) { \
	    /* o=a=a*(123456*4+1)+1; */ \
	    if( COLOR_FOG_P(sp[x]) ) {	\
		dp[x]=((VMemType16*)FogOfWarAlphaTable)[dp[x]/*^((o>>20)&4)*/]; \
	    } \
	} \

#undef UNROLL2
#define UNROLL2(x)	\
	UNROLL1(x+0);	\
	UNROLL1(x+1);	\
	//o=(o>>1)|((o&1)<<31);

	UNROLL32(0);
	sp+=TileSizeX;
	dp+=da;
    }
}

/**
**	Fast draw 100% fog of war 32x32 tile for 16 bpp video modes.
**
**	100% fog of war -- i.e. raster	10101.
**					01010 etc...
**
**	@param data	pointer to tile graphic data.
**	@param x	X position into video memory
**	@param y	Y position into video memory
**
**	The random effect is commented out
*/
global void VideoDraw16OnlyFog32Alpha(const GraphicData* data __attribute__((unused)),int x,int y)
{
    const VMemType16* gp;
    VMemType16* dp;
    VMemType16 fog;
    int da;
    //int o;

    dp=VideoMemory16+x+y*VideoWidth;
    gp=dp+VideoWidth*TileSizeY;
    da=VideoWidth;
    fog=((VMemType16*)TheMap.TileData->Pixels)[COLOR_FOG];

    while( dp<gp ) {
	//static int a=1234567;
	//o=rand();

#undef UNROLL1
#define UNROLL1(x)	\
	if (dp[x] != fog) { \
	    /* o=a=a*(123456*4+1)+1; */ \
	    dp[x]=((VMemType16*)FogOfWarAlphaTable)[dp[x]/*^((o>>20)&4)*/]; \
	} \

#undef UNROLL2
#define UNROLL2(x)	\
	UNROLL1(x+0);	\
	UNROLL1(x+1);	\
	//o=(o>>1)|((o&1)<<31);

	UNROLL32(0);
	dp+=da;
    }
}

// Routines for 24 bit displays .. -------------------------------------------

/**
**	Fast draw alpha fog of war 32x32 tile for 24 bpp video modes.
**
**	@param data	pointer to tile graphic data.
**	@param x	X position into video memory
**	@param y	Y position into video memory
*/
global void VideoDraw24Fog32Alpha(const GraphicData* data,int x,int y)
{
    const unsigned char* sp;
    const unsigned char* gp;
    VMemType24* dp;
    VMemType24 fog;
    int da;
    int r, g, b, v ;

    sp=data;
    gp=sp+TileSizeY*TileSizeX;
    dp=VideoMemory24+x+y*VideoWidth;
    da=VideoWidth;
    fog=((VMemType24*)TheMap.TileData->Pixels)[COLOR_FOG];

    while( sp<gp ) {
#undef FOG_SCALE
#define FOG_SCALE(x) \
	    (((((x*3-v)*FogOfWarSaturation + v*100) \
		*FogOfWarContrast) \
		+FogOfWarBrightness*25600*3)/30000)

#undef UNROLL1
#define UNROLL1(x)      \
	if (COLOR_FOG_P(sp[x])) { \
	    if (dp[x].a != fog.a || dp[x].b != fog.b || dp[x].c != fog.c) { \
		r=dp[x].a & 0xff; \
		g=dp[x].b & 0xff; \
		b=dp[x].c & 0xff; \
		v=r+g+b; \
\
		r = FOG_SCALE(r); \
		g = FOG_SCALE(g); \
		b = FOG_SCALE(b); \
\
		r= r<0 ? 0 : r>255 ? 255 : r; \
		g= g<0 ? 0 : g>255 ? 255 : g; \
		b= b<0 ? 0 : b>255 ? 255 : b; \
		dp[x].a = r; \
		dp[x].b = g; \
		dp[x].c = b; \
	    } \
	} \

#undef UNROLL2
#define UNROLL2(x)	\
	UNROLL1(x+0);	\
	UNROLL1(x+1);	\

	UNROLL32(0);
	sp+=TileSizeX;
	dp+=da;
    }
}

/**
**	Fast draw 100% fog of war 32x32 tile for 24 bpp video modes.
**
**	100% fog of war -- i.e. raster	10101.
**					01010 etc...
**
**	@param data	pointer to tile graphic data.
**	@param x	X position into video memory
**	@param y	Y position into video memory
*/
global void VideoDraw24OnlyFog32Alpha(const GraphicData* data __attribute__((unused)),int x,int y)
{
    const VMemType24* gp;
    VMemType24* dp;
    VMemType24 fog;
    int da;
    int r, g, b, v;

    dp=VideoMemory24+x+y*VideoWidth;
    gp=dp+VideoWidth*TileSizeY;
    da=VideoWidth;
    fog=((VMemType24*)TheMap.TileData->Pixels)[COLOR_FOG];

    while( dp<gp ) {

#undef FOG_SCALE
#define FOG_SCALE(x) \
	    (((((x*3-v)*FogOfWarSaturation + v*100) \
		*FogOfWarContrast) \
		+FogOfWarBrightness*25600*3)/30000)

#undef UNROLL1
#define UNROLL1(x)	\
	if (dp[x].a != fog.a || dp[x].b != fog.b || dp[x].c != fog.c) { \
	    r=dp[x].a & 0xff; \
	    g=dp[x].b & 0xff; \
	    b=dp[x].c & 0xff; \
	    v=r+g+b; \
\
	    r = FOG_SCALE(r); \
	    g = FOG_SCALE(g); \
	    b = FOG_SCALE(b); \
\
	    r= r<0 ? 0 : r>255 ? 255 : r; \
	    g= g<0 ? 0 : g>255 ? 255 : g; \
	    b= b<0 ? 0 : b>255 ? 255 : b; \
	    dp[x].a = r; \
	    dp[x].b = g; \
	    dp[x].c = b; \
	} \

#undef UNROLL2
#define UNROLL2(x)	\
	UNROLL1(x+0);	\
	UNROLL1(x+1);	\

	UNROLL32(0);
	dp+=da;
    }
}

// Routines for 32 bit displays .. -------------------------------------------

/**
**	Fast draw alpha fog of war 32x32 tile for 32 bpp video modes.
**
**	@param data	pointer to tile graphic data.
**	@param x	X position into video memory
**	@param y	Y position into video memory
*/
global void VideoDraw32Fog32Alpha(const GraphicData* data,int x,int y)
{
    const unsigned char* sp;
    const unsigned char* gp;
    VMemType32* dp;
    VMemType32 fog;
    int da;
    int r, g, b, v ;
    VMemType32 i;
    VMemType32 lasti;
    VMemType32 lastrgb;

    sp=data;
    gp=sp+TileSizeY*TileSizeX;
    dp=VideoMemory32+x+y*VideoWidth;
    da=VideoWidth;
    lastrgb=lasti=0;
    fog=((VMemType32*)TheMap.TileData->Pixels)[COLOR_FOG];

    while( sp<gp ) {
#undef FOG_SCALE
#define FOG_SCALE(x) \
	    (((((x*3-v)*FogOfWarSaturation + v*100) \
		*FogOfWarContrast) \
		+FogOfWarBrightness*25600*3)/30000)

#undef UNROLL1
#define UNROLL1(x)      \
	if (COLOR_FOG_P(sp[x])) { \
	    i = dp[x]; \
	    if (i != fog) { \
		if (i == lasti) { \
		    dp[x] = lastrgb; \
		} else { \
		    lasti = i; \
		    r=i       & 0xff; \
		    g=(i>>8 ) & 0xff; \
		    b=(i>>16) & 0xff; \
		    v=r+g+b; \
 \
		    r = FOG_SCALE(r); \
		    g = FOG_SCALE(g); \
		    b = FOG_SCALE(b); \
 \
		    r= r<0 ? 0 : r>255 ? 255 : r; \
		    g= g<0 ? 0 : g>255 ? 255 : g; \
		    b= b<0 ? 0 : b>255 ? 255 : b; \
		    dp[x]= (r | (g << 8) | (b << 16)); \
		    lastrgb = dp[x]; \
		} \
	    } \
	} \

#undef UNROLL2
#define UNROLL2(x)	\
	UNROLL1(x+0);	\
	UNROLL1(x+1);	\

	UNROLL32(0);
	sp+=TileSizeX;
	dp+=da;
    }
}

/**
**	Fast draw 100% fog of war 32x32 tile for 32 bpp video modes.
**
**	100% fog of war -- i.e. raster	10101.
**					01010 etc...
**
**	@param data	pointer to tile graphic data.
**	@param x	X position into video memory
**	@param y	Y position into video memory
*/
global void VideoDraw32OnlyFog32Alpha(const GraphicData* data __attribute__((unused)),int x,int y)
{
    const VMemType32* gp;
    VMemType32* dp;
    VMemType32 fog;
    int da;
    int r, g, b, v;
    VMemType32 i;
    VMemType32 lasti;
    VMemType32 lastrgb;

    dp=VideoMemory32+x+y*VideoWidth;
    gp=dp+VideoWidth*TileSizeY;
    da=VideoWidth;
    lastrgb=lasti=0;
    fog=((VMemType32*)TheMap.TileData->Pixels)[COLOR_FOG];

    while( dp<gp ) {

#undef FOG_SCALE
#define FOG_SCALE(x) \
	    (((((x*3-v)*FogOfWarSaturation + v*100) \
		*FogOfWarContrast) \
		+FogOfWarBrightness*25600*3)/30000)

#undef UNROLL1
#define UNROLL1(x)	\
	i = dp[x]; \
	if (i != fog) { \
	    if (i == lasti) { \
		dp[x] = lastrgb; \
	    } else { \
		lasti = i; \
		r=i       & 0xff; \
		g=(i>>8 ) & 0xff; \
		b=(i>>16) & 0xff; \
		v=r+g+b; \
 \
		r = FOG_SCALE(r); \
		g = FOG_SCALE(g); \
		b = FOG_SCALE(b); \
 \
		r= r<0 ? 0 : r>255 ? 255 : r; \
		g= g<0 ? 0 : g>255 ? 255 : g; \
		b= b<0 ? 0 : b>255 ? 255 : b; \
		dp[x]= r | (g << 8) | (b << 16); \
		lastrgb = dp[x]; \
	    } \
	} \

#undef UNROLL2
#define UNROLL2(x)	\
	UNROLL1(x+0);	\
	UNROLL1(x+1);	\

	UNROLL32(0);
	dp+=da;
    }
}

// Routines for OpenGL .. -------------------------------------------

/**
**	Fast draw alpha fog of war 32x32 tile for 32 bpp video modes.
**
**	@param data	pointer to tile graphic data.
**	@param x	X position into video memory
**	@param y	Y position into video memory
*/
#ifdef USE_OPENGL
global void VideoDrawFogAlphaOpenGL(
    const GraphicData* data __attribute__((unused)),
    int x __attribute__((unused)),int y __attribute__((unused)))
{
}
#endif

/**
**	Fast draw 100% fog of war 32x32 tile for 32 bpp video modes.
**
**	100% fog of war -- i.e. raster	10101.
**					01010 etc...
**
**	@param data	pointer to tile graphic data.
**	@param x	X position into video memory
**	@param y	Y position into video memory
*/
#ifdef USE_OPENGL
global void VideoDrawOnlyFogAlphaOpenGL(
    const GraphicData* data __attribute__((unused)),
    int x __attribute__((unused)),int y __attribute__((unused)))
{
}
#endif

/*----------------------------------------------------------------------------
--	Old version correct working but not 100% original
----------------------------------------------------------------------------*/

/**
**	Draw fog of war tile.
**
**	@param sx	Offset into fields to current tile.
**	@param sy	Start of the current row.
**	@param dx	X position into video memory.
**	@param dy	Y position into video memory.
*/
local void DrawFogOfWarTile(int sx,int sy,int dx,int dy)
{
#ifdef NEW_FOW
    sx=sy=dx=dy=0;
#if 0
    int m;
    MapField* mf;

    m=1<<ThisPlayer->Player;
    mf=TheMap.Fields+sx;

    //
    //	Draw unexplored area
    //
    if( mf->Explored && mf->ExploredMask ) {
	VideoDrawUnexplored(TheMap.Tiles[mf->ExploredMask],dx,dy);
	// Don't need to draw the same tiles.
	if( mf->Visible && mf->ExploredMask==mf->VisibleMask ) {
	    return;
	}
    }
    if( mf->Visible ) {
	if( mf->VisibleMask ) {
	    VideoDrawFog(TheMap.Tiles[mf->VisibleMask],dx,dy);
	}
    } else {
	VideoDrawOnlyFog(TheMap.Tiles[UNEXPLORED_TILE],dx,dy);
    }
#endif
#else
    int w;
    int tile;
    int tile2;

    w=TheMap.Width;
    tile=tile2=0;
    //
    //	Check each field around it
    //
    if( sy ) {
	if( sx!=sy ) {
	    if( !(TheMap.Fields[sx-w-1].Flags&MapFieldExplored) ) {
		tile2|=2;
		tile|=2;
	    } else if( !(TheMap.Visible[0][(sx-w-1)/32]&(1<<((sx-w-1)%32)) ) ) {
		tile|=2;
	    }
	}
	if( !(TheMap.Fields[sx-w].Flags&MapFieldExplored) ) {
	    tile2|=3;
	    tile|=3;
	} else if( !(TheMap.Visible[0][(sx-w)/32]&(1<<((sx-w)%32)) ) ) {
	    tile|=3;
	}
	if( sx!=sy+w-1 ) {
	    if( !(TheMap.Fields[sx-w+1].Flags&MapFieldExplored) ) {
		tile2|=1;
		tile|=1;
	    } else if( !(TheMap.Visible[0][(sx-w+1)/32]&(1<<((sx-w+1)%32)) ) ) {
		tile|=1;
	    }
	}
    }

    if( sx!=sy ) {
	if( !(TheMap.Fields[sx-1].Flags&MapFieldExplored) ) {
	    tile2|=10;
	    tile|=10;
	} else if( !(TheMap.Visible[0][(sx-1)/32]&(1<<((sx-1)%32)) ) ) {
	    tile|=10;
	}
    }
    if( sx!=sy+w-1 ) {
	if( !(TheMap.Fields[sx+1].Flags&MapFieldExplored) ) {
	    tile2|=5;
	    tile|=5;
	} else if( !(TheMap.Visible[0][(sx+1)/32]&(1<<((sx+1)%32)) ) ) {
	    tile|=5;
	}
    }

    if( sy+w<TheMap.Height*w ) {
	if( sx!=sy ) {
	    if( !(TheMap.Fields[sx+w-1].Flags&MapFieldExplored) ) {
		tile2|=8;
		tile|=8;
	    } else if( !(TheMap.Visible[0][(sx+w-1)/32]&(1<<((sx+w-1)%32)) ) ) {
		tile|=8;
	    }
	}
	if( !(TheMap.Fields[sx+w].Flags&MapFieldExplored) ) {
	    tile2|=12;
	    tile|=12;
	} else if( !(TheMap.Visible[0][(sx+w)/32]&(1<<((sx+w)%32)) ) ) {
	    tile|=12;
	}
	if( sx!=sy+w-1 ) {
	    if( !(TheMap.Fields[sx+w+1].Flags&MapFieldExplored) ) {
		tile2|=4;
		tile|=4;
	    } else if( !(TheMap.Visible[0][(sx+w+1)/32]&(1<<((sx+w+1)%32))) ) {
		tile|=4;
	    }
	}
    }

    //TheMap.Fields[sx].VisibleLastFrame=0;
    //
    //	Draw unexplored area
    //	If only partly or total invisible draw fog of war.
    //
    tile=FogTable[tile];
    tile2=FogTable[tile2];
    if( tile2) {
	VideoDrawUnexplored(TheMap.Tiles[tile2],dx,dy);
	if( tile2==tile ) {		// no same fog over unexplored
//	    if( tile != 0xf ) {
//		TheMap.Fields[sx].VisibleLastFrame|=MapFieldPartiallyVisible;
//	    }
	    tile=0;
	}
    }
    if( TheMap.Visible[0][sx/32]&(1<<(sx%32)) ) {
	if( tile ) {
	    VideoDrawFog(TheMap.Tiles[tile],dx,dy);
//	    TheMap.Fields[sx].VisibleLastFrame|=MapFieldPartiallyVisible;
//	} else {
//	    TheMap.Fields[sx].VisibleLastFrame|=MapFieldCompletelyVisible;
	}
    } else {
	VideoDrawOnlyFog(TheMap.Tiles[UNEXPLORED_TILE],dx,dy);
    }
#endif
}

#ifdef HIERARCHIC_PATHFINDER
#include "pathfinder.h"
// hack
#include "../pathfinder/region_set.h"
#endif	// HIERARCHIC_PATHFINDER

/**
**	Draw the map fog of war.
**
**	@param v	Viewport.
**	@param x	Map viewpoint x position.
**	@param y	Map viewpoint y position.
*/
global void DrawMapFogOfWar(int v, int x,int y)
{
    Viewport* vp;
    int sx;
    int sy;
    int dx;
    int ex;
    int dy;
    int ey;
    char* redraw_row;
    char* redraw_tile;
#ifdef NEW_FOW
    int p;
#endif
#ifdef TIMEIT
    u_int64_t sv=rdtsc();
    u_int64_t ev;
    static long mv=9999999;
#endif

    vp = &TheUI.VP[v];
    redraw_row=MustRedrawRow;		// flags must redraw or not
    redraw_tile=MustRedrawTile;

#ifdef NEW_FOW
    p=ThisPlayer->Player;
#endif

    ex = vp->EndX;
    sy = y*TheMap.Width;
    dy = vp->Y;
    ey = vp->EndY;

    while( dy<ey ) {
	if( *redraw_row ) {		// row must be redrawn
#if NEW_MAPDRAW > 1
	    (*redraw_row)--;
#else
	    *redraw_row=0;
#endif
	    sx=x+sy;
	    dx = vp->X;
	    while( dx<ex ) {
		if( *redraw_tile ) {
#if NEW_MAPDRAW > 1
                  (*redraw_tile)--;
#else
                  *redraw_tile=0;
#ifdef NEW_FOW
		    if( TheMap.Fields[sx].Visible[p] ) {
			DrawFogOfWarTile(sx,sy,dx,dy);
		    } else {
#ifdef USE_OPENGL
			MapDrawTile(UNEXPLORED_TILE,dx,dy);
#else
			VideoDrawTile(TheMap.Tiles[UNEXPLORED_TILE],dx,dy);
#endif
		    }
#else
		    if( TheMap.Fields[sx].Flags&MapFieldExplored ) {
			DrawFogOfWarTile(sx,sy,dx,dy);
		    } else {
#ifdef USE_OPENGL
			MapDrawTile(UNEXPLORED_TILE,dx,dy);
#else
			VideoDrawTile(TheMap.Tiles[UNEXPLORED_TILE],dx,dy);
#endif
		    }
#endif
#endif

#if defined(HIERARCHIC_PATHFINDER) && defined(DEBUG)
		    {
			char regidstr[8];
			char groupstr[8];
			int regid;
extern int VideoDrawText(int x,int y,unsigned font,const unsigned char* text);
#define GameFont 1

			if (PfHierShowRegIds || PfHierShowGroupIds) {
			    regid =
				MapFieldGetRegId (
					    (dx-vp->X)/TileSizeX + vp->MapX,
					    (dy-vp->Y)/TileSizeY + vp->MapY);
			    if (regid) {
				Region *r = RegionSetFind (regid);
				if (PfHierShowRegIds) {
				    snprintf (regidstr, 8, "%d", regid);
				    VideoDrawText (dx, dy, GameFont, regidstr);
				}
				if (PfHierShowGroupIds) {
				    snprintf (groupstr, 8, "%d", r->GroupId);
				    VideoDrawText (dx, dy+19, GameFont, groupstr);
				}
			    }
			}
		    }
#endif	// HIERARCHIC_PATHFINDER
		}
                ++redraw_tile;
		++sx;
		dx+=TileSizeX;
	    }
	} else {
	    redraw_tile += vp->MapWidth;
	}
        ++redraw_row;
	sy+=TheMap.Width;
	dy+=TileSizeY;
    }

#ifdef TIMEIT
    ev=rdtsc();
    sx=(ev-sv);
    if( sx<mv ) {
	mv=sx;
    }

    DebugLevel1("%ld %ld %3ld\n" _C_ (long)sx _C_ mv _C_ (sx*100)/mv);
#endif
}

/**
**	Initialise the fog of war.
**	Build tables, setup functions.
*/
global void InitMapFogOfWar(void)
{
#ifdef USE_OPENGL
    VideoDrawFog=VideoDrawFogAlphaOpenGL;
    VideoDrawOnlyFog=VideoDrawOnlyFogAlphaOpenGL;
    VideoDrawUnexplored=VideoDrawUnexploredSolidOpenGL;
#else
    if( !OriginalFogOfWar ) {
	int i;
	int n;
	int v;
	int r,g,b;
	int rmask,gmask,bmask;
	int rshft,gshft,bshft;
	int rloss,gloss,bloss;


	switch( VideoDepth ) {
	    case 8:
		n=1<<(sizeof(VMemType8)*8);
		if( !FogOfWarAlphaTable ) {
		    FogOfWarAlphaTable=malloc(n*sizeof(VMemType8));
		}
		if ( lookup25trans8 ) { // if enabled, make use of it in 8bpp ;)
		    unsigned int trans_color;
		    int j;

		    trans_color=Pixels8[ColorBlack];
		    trans_color<<=8;

		    //FIXME: determine which lookup table to use based on
		    //FIXME: FogOfWarSaturation,FogOfWarContrast and
		    //FIXME: FogOfWarBrightness
		    for( j=0; j<n; ++j ) {
			((VMemType8*)FogOfWarAlphaTable)[j] =
			    lookup50trans8[ trans_color | j ];
		    }
		} else {
		    for( i=0; i<n; ++i ) {
			int j;
			int l;
			int d;

			r=GlobalPalette[i].r;
			g=GlobalPalette[i].g;
			b=GlobalPalette[i].b;
			DebugLevel3("%d,%d,%d\n" _C_ r _C_ g _C_ b);
			v=r+g+b;

			r= ((((r*3-v)*FogOfWarSaturation + v*100)
			    *FogOfWarContrast)
			    +FogOfWarBrightness*25600*3)/30000;
			g= ((((g*3-v)*FogOfWarSaturation + v*100)
			    *FogOfWarContrast)
			    +FogOfWarBrightness*25600*3)/30000;
			b= ((((b*3-v)*FogOfWarSaturation + v*100)
			    *FogOfWarContrast)
			    +FogOfWarBrightness*25600*3)/30000;

			// Boundings
			r= r<0 ? 0 : r>255 ? 255 : r;
			g= g<0 ? 0 : g>255 ? 255 : g;
			b= b<0 ? 0 : b>255 ? 255 : b;

			//
			//	Find the best matching color
			//
			l=i;
			d=256*3+256;
			for( j=0; j<256; ++j ) {
			    // simple color distance
			    v=(abs(GlobalPalette[j].r-r)
				+abs(GlobalPalette[j].g-g)
				+abs(GlobalPalette[j].b-b))*3
				// light
				+abs(GlobalPalette[j].r
				    +GlobalPalette[j].g
				    +GlobalPalette[j].b-(r+g+b))*1;
			    if( v<d ) {
				d=v;
				l=j;
			    }
			}
			DebugLevel3("%d,%d,%d -> %d,%d,%d\n" _C_ r _C_ g _C_ b
				_C_ GlobalPalette[l].r _C_ GlobalPalette[l].g
				_C_ GlobalPalette[l].b);
			((VMemType8*)FogOfWarAlphaTable)[i]=l;
		    }
		}

		VideoDrawFog=VideoDraw8Fog32Alpha;
		VideoDrawOnlyFog=VideoDraw8OnlyFog32Alpha;
		VideoDrawUnexplored=VideoDraw8Unexplored32Solid;
		break;

	    case 15:			// 15 bpp 555 video depth
		rshft=( 0);
		gshft=( 5);
		bshft=(10);
		rmask=(0x1F<<rshft);
		gmask=(0x1F<<gshft);
		bmask=(0x1F<<bshft);
		rloss=( 3);
		gloss=( 3);
		bloss=( 3);
		goto build_table;

	    case 16:			// 16 bpp 565 video depth
		rshft=( 0);
		gshft=( 5);
		bshft=(11);
		rmask=(0x1F<<rshft);
		gmask=(0x3F<<gshft);
		bmask=(0x1F<<bshft);
		rloss=( 3);
		gloss=( 2);
		bloss=( 3);

build_table:
		n=1<<(sizeof(VMemType16)*8);
		if( !FogOfWarAlphaTable ) {
		    FogOfWarAlphaTable=malloc(n*sizeof(VMemType16));
		}
		for( i=0; i<n; ++i ) {
		    r=(i&rmask)>>rshft<<rloss;
		    g=(i&gmask)>>gshft<<gloss;
		    b=(i&bmask)>>bshft<<bloss;
		    v=r+g+b;

		    r= ((((r*3-v)*FogOfWarSaturation + v*100)
			*FogOfWarContrast)
			+FogOfWarBrightness*25600*3)/30000;
		    g= ((((g*3-v)*FogOfWarSaturation + v*100)
			*FogOfWarContrast)
			+FogOfWarBrightness*25600*3)/30000;
		    b= ((((b*3-v)*FogOfWarSaturation + v*100)
			*FogOfWarContrast)
			+FogOfWarBrightness*25600*3)/30000;

		    // Boundings
		    r= r<0 ? 0 : r>255 ? 255 : r;
		    g= g<0 ? 0 : g>255 ? 255 : g;
		    b= b<0 ? 0 : b>255 ? 255 : b;
		    ((VMemType16*)FogOfWarAlphaTable)[i]=((r>>rloss)<<rshft)
			    |((g>>gloss)<<gshft)
			    |((b>>bloss)<<bshft);
		}
		VideoDrawFog=VideoDraw16Fog32Alpha;
		VideoDrawOnlyFog=VideoDraw16OnlyFog32Alpha;
		VideoDrawUnexplored=VideoDraw16Unexplored32Solid;
		break;

	    case 24:
		if( VideoBpp==24 ) {
		    VideoDrawFog=VideoDraw24Fog32Alpha;
		    VideoDrawOnlyFog=VideoDraw24OnlyFog32Alpha;
		    VideoDrawUnexplored=VideoDraw24Unexplored32Solid;
		    break;
		}
		// FALL THROUGH
	    case 32:
		VideoDrawFog=VideoDraw32Fog32Alpha;
		VideoDrawOnlyFog=VideoDraw32OnlyFog32Alpha;
		VideoDrawUnexplored=VideoDraw32Unexplored32Solid;
		break;

	    default:
		DebugLevel0Fn("Depth unsupported %d\n" _C_ VideoDepth);
		break;
	}
    } else {
	switch( VideoDepth ) {
	    case  8:			//  8 bpp video depth
		VideoDrawFog=VideoDraw8Fog32Solid;
		VideoDrawOnlyFog=VideoDraw8OnlyFog32Solid;
		VideoDrawUnexplored=VideoDraw8Unexplored32Solid;
		break;

	    case 15:			// 15 bpp video depth
	    case 16:			// 16 bpp video depth
		VideoDrawFog=VideoDraw16Fog32Solid;
		VideoDrawOnlyFog=VideoDraw16OnlyFog32Solid;
		VideoDrawUnexplored=VideoDraw16Unexplored32Solid;
		break;
	    case 24:			// 24 bpp video depth
		if( VideoBpp==24 ) {
		    VideoDrawFog=VideoDraw24Fog32Solid;
		    VideoDrawOnlyFog=VideoDraw24OnlyFog32Solid;
		    VideoDrawUnexplored=VideoDraw24Unexplored32Solid;
		    break;
		}
		// FALL THROUGH
	    case 32:			// 32 bpp video depth
		VideoDrawFog=VideoDraw32Fog32Solid;
		VideoDrawOnlyFog=VideoDraw32OnlyFog32Solid;
		VideoDrawUnexplored=VideoDraw32Unexplored32Solid;
		break;

	    default:
		DebugLevel0Fn("Depth unsupported %d\n" _C_ VideoDepth);
		break;
	}
    }
#endif
}

/**
**	Cleanup the fog of war.
*/
global void CleanMapFogOfWar(void)
{
    if( FogOfWarAlphaTable ) {
	free(FogOfWarAlphaTable);
	FogOfWarAlphaTable=NULL;
    }
}

//@}
