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
/**@name minimap.c	-	The minimap. */
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
#include <string.h>

#include "freecraft.h"
#include "video.h"
#include "tileset.h"
#include "map.h"
#include "minimap.h"
#include "unittype.h"
#include "player.h"
#include "unit.h"
#include "ui.h"

/*----------------------------------------------------------------------------
--	Variables
----------------------------------------------------------------------------*/

local Graphic* MinimapGraphic;		/// generated minimap
local int Minimap2MapX[MINIMAP_W];	/// fast conversion table
local int Minimap2MapY[MINIMAP_H];	/// fast conversion table
local int Map2MinimapX[MaxMapWidth];	/// fast conversion table
local int Map2MinimapY[MaxMapHeight];	/// fast conversion table

//	MinimapScale:
//	32x32 64x64 96x96 128x128 256x256 512x512 ...
//	  *4    *2    *4/3   *1	     *1/2    *1/4
global int MinimapScale;		/// Minimap scale to fit into window
global int MinimapX;			/// Minimap drawing position x offset
global int MinimapY;			/// Minimap drawing position y offset

global int MinimapWithTerrain=1;	/// display minimap with terrain
global int MinimapFriendly=1;		/// switch colors of friendly units
global int MinimapShowSelected=1;	/// highlight selected units

/*----------------------------------------------------------------------------
--	Functions
----------------------------------------------------------------------------*/

/**
**	Update minimap at map position x,y. This is called when the tile image
**	of a tile changes.
**
**	@todo	FIXME: this is not correct should use SeenTile.
**
**	@param tx	Tile X position, where the map changed.
**	@param ty	Tile Y position, where the map changed.
**
**	FIXME: this can surely speeded up??
*/
global void UpdateMinimapXY(int tx,int ty)
{
    int mx;
    int my;
    int x;
    int y;
    int scale;

    if( !(scale=(MinimapScale/MINIMAP_FAC)) ) {
	scale=1;
    }
    //
    //	Pixel 7,6 7,14, 15,6 15,14 are taken for the minimap picture.
    //
    ty*=TheMap.Width;
    for( my=MinimapY; my<MINIMAP_H-MinimapY; my++ ) {
	y=Minimap2MapY[my];
	if( y<ty ) {
	    continue;
	}
	if( y>ty ) {
	    break;
	}

	for( mx=MinimapX; mx<MINIMAP_W-MinimapX; mx++ ) {
	    int tile;

	    x=Minimap2MapX[mx];
	    if( x<tx ) {
		continue;
	    }
	    if( x>tx ) {
		break;
	    }

	    tile=TheMap.Fields[x+y].Tile;
	    ((char*)MinimapGraphic->Frames)[mx+my*MINIMAP_W]=
		TheMap.Tiles[tile][7+(mx%scale)*8+(6+(my%scale)*8)*TileSizeX];
	}
    }
}

/**
**	Update a mini-map from the tiles of the map.
**
**	@todo	FIXME: this is not correct should use SeenTile.
**
**	FIXME: this can surely speeded up??
*/
global void UpdateMinimap(void)
{
    int mx;
    int my;
    int scale;

    if( !(scale=(MinimapScale/MINIMAP_FAC)) ) {
	scale=1;
    }

    //
    //	Pixel 7,6 7,14, 15,6 15,14 are taken for the minimap picture.
    //
    for( my=MinimapY; my<MINIMAP_H-MinimapY; my++ ) {
	for( mx=MinimapX; mx<MINIMAP_W-MinimapX; mx++ ) {
	    int tile;

	    tile=TheMap.Fields[Minimap2MapX[mx]+Minimap2MapY[my]].Tile;
	    ((char*)MinimapGraphic->Frames)[mx+my*MINIMAP_W]=
		TheMap.Tiles[tile][7+(mx%scale)*8+(6+(my%scale)*8)*TileSizeX];
	}
    }
}

/**
**	Create a mini-map from the tiles of the map.
**
**	@todo 	Scaling and scrolling the minmap is currently not supported.
*/
global void CreateMinimap(void)
{
    int n;

    if( TheMap.Width>TheMap.Height ) {	// Scale too biggest value.
	n=TheMap.Width;
    } else {
	n=TheMap.Height;
    }
    MinimapScale=(MINIMAP_W*MINIMAP_FAC)/n;

    MinimapX=((MINIMAP_W*MINIMAP_FAC)/MinimapScale-TheMap.Width)/2;
    MinimapY=((MINIMAP_H*MINIMAP_FAC)/MinimapScale-TheMap.Height)/2;
    MinimapX=(MINIMAP_W-(TheMap.Width*MinimapScale)/MINIMAP_FAC)/2;
    MinimapY=(MINIMAP_H-(TheMap.Height*MinimapScale)/MINIMAP_FAC)/2;

    DebugLevel0Fn("MinimapScale %d(%d), X off %d, Y off %d\n",
	    MinimapScale/MINIMAP_FAC,MinimapScale,MinimapX,MinimapY);

    //
    //	Calculate minimap fast lookup tables.
    //
    for( n=MinimapX; n<MINIMAP_W-MinimapX; ++n ) {
	Minimap2MapX[n]=((n-MinimapX)*MINIMAP_FAC)/MinimapScale;
    }
    for( n=MinimapY; n<MINIMAP_H-MinimapY; ++n ) {
	Minimap2MapY[n]=(((n-MinimapY)*MINIMAP_FAC)/MinimapScale)*TheMap.Width;
    }
    for( n=0; n<TheMap.Width; ++n ) {
	Map2MinimapX[n]=(n*MinimapScale)/MINIMAP_FAC;
    }
    for( n=0; n<TheMap.Height; ++n ) {
	Map2MinimapY[n]=(n*MinimapScale)/MINIMAP_FAC;
    }

    MinimapGraphic=NewGraphic(8,MINIMAP_W,MINIMAP_H);
    memset(MinimapGraphic->Frames,0,MINIMAP_W*MINIMAP_H);

    UpdateMinimap();
}

/**
**	Destroy mini-map.
*/
global void DestroyMinimap(void)
{
    VideoSaveFree(MinimapGraphic);
    MinimapGraphic=NULL;
}

/**
**	Draw the mini-map with current viewpoint.
**
**	@param vx	View point X position.
**	@param vy	View point Y position.
**
**	@note This one of the hot-points in the program optimize and optimize!
*/
global void DrawMinimap(int vx __attribute__((unused)),int vy __attribute__((unused)))
{
    static int red_phase;
    int new_phase;
    int mx;
    int my;
    int x;
    int y;
    UnitType* type;
    Unit** table;
    Unit* unit;
    int w;
    int h;
    int h0;
#ifdef NEW_FOW
    int bits;
    MapField* mf;
#else
    int flags;
#endif

    x=(FrameCounter/FRAMES_PER_SECOND)&1;
    if( (new_phase=red_phase-x) ) {
	red_phase=x;
    }

    x=TheUI.MinimapX+24;
    y=TheUI.MinimapY+2;

    //
    //	Draw the mini-map background.	Note draws a little too much.
    //
    VideoDrawSub(TheUI.Minimap.Graphic,24,2
	    ,TheUI.Minimap.Graphic->Width-48,TheUI.Minimap.Graphic->Height-4
	    ,x,y);

    //
    //	Draw the terrain
    //
#ifdef NEW_FOW
    bits=(1<<ThisPlayer->Player);
#endif
    if( MinimapWithTerrain ) {
	for( my=0; my<MINIMAP_H; ++my ) {
	    for( mx=0; mx<MINIMAP_W; ++mx ) {
#ifdef NEW_FOW
		mf=TheMap.Fields+Minimap2MapX[mx]+Minimap2MapY[my];
		if( (mf->Explored&bits)
			&& ( (mf->Visible&bits)
			    || ((mx&1)==(my&1)) ) ) {
		    VideoDrawPixel(((char*)MinimapGraphic->Frames)
			    [mx+my*MINIMAP_W],x+mx,y+my);
		}
#else
		flags=TheMap.Fields[Minimap2MapX[mx]+Minimap2MapY[my]].Flags;
		if( flags&MapFieldExplored &&
			( (flags&MapFieldVisible) || ((mx&1)==(my&1)) ) ) {
		    VideoDrawPixel(((char*)MinimapGraphic->Frames)
			    [mx+my*MINIMAP_W],x+mx,y+my);
		}
#endif
	    }
	}
    }

    //
    //	Draw units on map
    //	FIXME: I should rewrite this complet
    //	FIXME: make a bitmap of the units, and update it with the moves
    //	FIXME: and other changes
    //
    for( table=Units; table<Units+NumUnits; table++ ) {
	SysColors color;

	unit=*table;

	if( unit->Removed ) {		// Removed, inside another building
	    continue;
	}
	if( unit->Invisible ) {		// Can't be seen
	    continue;
	}
	if( !(unit->Visible&(1<<ThisPlayer->Player)) ) {
	    continue;			// Submarine not visible
	}


#ifdef NEW_FOW
	mf=TheMap.Fields+unit->X+unit->Y*TheMap.Width;
	// Draw only units on explored fields
	if( !(mf->Explored&bits) ) {
	    continue;
	}
	// Draw only units on visible fields
	if( !(mf->Visible&bits) ) {
	    continue;
	}
#else
	if( !UnitKnownOnMap(unit) ) {
	    continue;
	}
	unit->SeenFrame = unit->Frame;
#endif

	// FIXME: submarine not visible

	type=unit->Type;
	if( unit->Player->Player==PlayerNumNeutral ) {
	    if( type->Critter ) {
		color=ColorNPC;
	    } else if( type->OilPatch ) {
		color=ColorBlack;
	    } else {
		color=ColorYellow;
	    }
	} else if( unit->Player==ThisPlayer ) {
	    if( unit->Attacked && red_phase ) {
		color=ColorRed;
		// better to clear to fast, than to clear never :?)
		if( new_phase ) {
		    unit->Attacked--;
		}
	    } else if( MinimapShowSelected && unit->Selected ) {
		color=ColorWhite;
	    } else {
		color=ColorGreen;
	    }
	} else {
	    color=unit->Player->Color;
	}

	mx=x+1+MinimapX+Map2MinimapX[unit->X];
	my=y+1+MinimapY+Map2MinimapY[unit->Y];
	w=Map2MinimapX[type->TileWidth];
	if( mx+w>=x+MINIMAP_W ) {	// clip right side
	    w=x+MINIMAP_W-mx;
	}
	h0=Map2MinimapY[type->TileHeight];
	if( my+h0>=y+MINIMAP_H ) {	// clip bottom side
	    h0=y+MINIMAP_H-my;
	}
	while( w-->=0 ) {
	    h=h0;
	    while( h-->=0 ) {
		VideoDrawPixel(color,mx+w,my+h);
	    }
	}
    }
}

local int OldMinimapCursorX;		/// Save MinimapCursorX
local int OldMinimapCursorY;		/// Save MinimapCursorY
local int OldMinimapCursorW;		/// Save MinimapCursorW
local int OldMinimapCursorH;		/// Save MinimapCursorH
local int OldMinimapCursorSize;		/// Saved image size

local void* OldMinimapCursorImage;	/// Saved image behind cursor

/**
**	Hide minimap cursor.
*/
global void HideMinimapCursor(void)
{
    int i;
    int w;
    int h;

    if( !OldMinimapCursorImage ) {
	return;
    }

    w=OldMinimapCursorW;
    h=OldMinimapCursorH;

    // FIXME: Attention 8/16/32 bpp!
    switch( VideoBpp ) {
    case 8:
	{ VMemType8* sp;
	  VMemType8* dp;
	sp=OldMinimapCursorImage;
	dp=VideoMemory8+OldMinimapCursorY*VideoWidth+OldMinimapCursorX;
	memcpy(dp,sp,w*sizeof(VMemType8));
	sp+=w;
	for( i=0; i<h; ++i ) {
	    *dp=*sp++;
	    dp[w]=*sp++;
	    dp+=VideoWidth;
	}
	memcpy(dp,sp,(w+1)*sizeof(VMemType8)); }
	break;
    case 15:
    case 16:
	{ VMemType16* sp;
	  VMemType16* dp;
	sp=OldMinimapCursorImage;
	dp=VideoMemory16+OldMinimapCursorY*VideoWidth+OldMinimapCursorX;
	memcpy(dp,sp,w*sizeof(VMemType16));
	sp+=w;
	for( i=0; i<h; ++i ) {
	    *dp=*sp++;
	    dp[w]=*sp++;
	    dp+=VideoWidth;
	}
	memcpy(dp,sp,(w+1)*sizeof(VMemType16)); }
	break;
    case 24:
	{ VMemType24* sp;
	  VMemType24* dp;
	sp=OldMinimapCursorImage;
	dp=VideoMemory24+OldMinimapCursorY*VideoWidth+OldMinimapCursorX;
	memcpy(dp,sp,w*sizeof(VMemType24));
	sp+=w;
	for( i=0; i<h; ++i ) {
	    *dp=*sp++;
	    dp[w]=*sp++;
	    dp+=VideoWidth;
	}
	memcpy(dp,sp,(w+1)*sizeof(VMemType24)); }
	break;
    case 32:
	{ VMemType32* sp;
	  VMemType32* dp;
	sp=OldMinimapCursorImage;
	dp=VideoMemory32+OldMinimapCursorY*VideoWidth+OldMinimapCursorX;
	memcpy(dp,sp,w*sizeof(VMemType32));
	sp+=w;
	for( i=0; i<h; ++i ) {
	    *dp=*sp++;
	    dp[w]=*sp++;
	    dp+=VideoWidth;
	}
	memcpy(dp,sp,(w+1)*sizeof(VMemType32)); }
	break;
    }
}

/**
**	Draw minimap cursor.
**
**	@param vx	View point X position.
**	@param vy	View point Y position.
*/
global void DrawMinimapCursor(int vx,int vy)
{
    int x;
    int y;
    int w;
    int h;
    int i;

    OldMinimapCursorX=x=TheUI.MinimapX+24+MinimapX+
	    (vx*MinimapScale)/MINIMAP_FAC;
    OldMinimapCursorY=y=TheUI.MinimapY+2+MinimapY+
	    (vy*MinimapScale)/MINIMAP_FAC;
    OldMinimapCursorW=w=(MapWidth*MinimapScale)/MINIMAP_FAC;
    OldMinimapCursorH=h=(MapHeight*MinimapScale)/MINIMAP_FAC;

    switch( VideoBpp ) {
	case 8:
	    i=(w+1+h)*2*sizeof(VMemType8);
	    break;
	case 15:
	case 16:
	    i=(w+1+h)*2*sizeof(VMemType16);
	    break;
	case 24:
	    i=(w+1+h)*2*sizeof(VMemType24);
	    break;
	default:
	case 32:
	    i=(w+1+h)*2*sizeof(VMemType32);
	    break;
    }
    if( OldMinimapCursorSize<i ) {
	if( OldMinimapCursorImage ) {
	    OldMinimapCursorImage=realloc(OldMinimapCursorImage,i);
	} else {
	    OldMinimapCursorImage=malloc(i);
	}
	DebugLevel3("Cursor memory %d\n",i);
	OldMinimapCursorSize=i;
    }

    // FIXME: not 100% correct
    switch( VideoBpp ) {
    case 8:
	{ VMemType8* sp;
	VMemType8* dp;
	dp=OldMinimapCursorImage;
	sp=VideoMemory8+OldMinimapCursorY*VideoWidth+OldMinimapCursorX;
	memcpy(dp,sp,w*sizeof(VMemType8));
	dp+=w;
	for( i=0; i<h; ++i ) {
	    *dp++=*sp;
	    *dp++=sp[w];
	    sp+=VideoWidth;
	}
	memcpy(dp,sp,(w+1)*sizeof(VMemType8));
	break; }
    case 15:
    case 16:
	{ VMemType16* sp;
	VMemType16* dp;
	dp=OldMinimapCursorImage;
	sp=VideoMemory16+OldMinimapCursorY*VideoWidth+OldMinimapCursorX;
	memcpy(dp,sp,w*sizeof(VMemType16));
	dp+=w;
	for( i=0; i<h; ++i ) {
	    *dp++=*sp;
	    *dp++=sp[w];
	    sp+=VideoWidth;
	}
	memcpy(dp,sp,(w+1)*sizeof(VMemType16));
	break; }
    case 24:
	{ VMemType24* sp;
	VMemType24* dp;
	dp=OldMinimapCursorImage;
	sp=VideoMemory24+OldMinimapCursorY*VideoWidth+OldMinimapCursorX;
	memcpy(dp,sp,w*sizeof(VMemType24));
	dp+=w;
	for( i=0; i<h; ++i ) {
	    *dp++=*sp;
	    *dp++=sp[w];
	    sp+=VideoWidth;
	}
	memcpy(dp,sp,(w+1)*sizeof(VMemType24));
	break;
	}
    case 32:
	{ VMemType32* sp;
	VMemType32* dp;
	dp=OldMinimapCursorImage;
	sp=VideoMemory32+OldMinimapCursorY*VideoWidth+OldMinimapCursorX;
	memcpy(dp,sp,w*sizeof(VMemType32));
	dp+=w;
	for( i=0; i<h; ++i ) {
	    *dp++=*sp;
	    *dp++=sp[w];
	    sp+=VideoWidth;
	}
	memcpy(dp,sp,(w+1)*sizeof(VMemType32));
	break; }
    }

    // FIXME: The viewpoint color should be configurable
    switch( ThisPlayer->Race ) {
	case PlayerRaceHuman:
	    VideoDrawRectangleClip(ColorWhite,x,y,w,h);
	    break;
	case PlayerRaceOrc:
	    VideoDrawRectangleClip(ColorWhite,x,y,w,h);
	    break;
    }
}

/**
**	Convert minimap cursor X position to tile map coordinate.
**
**	@param x	Screen X pixel coordinate.
**	@return		Tile X coordinate.
*/
global int ScreenMinimap2MapX(int x)
{
    int tx;

    tx=((((x)-TheUI.MinimapX-24-MinimapX)*MINIMAP_FAC)/MinimapScale);
    if( tx<0 ) {
	return 0;
    }
    return tx<TheMap.Width ? tx : TheMap.Width-1;
}

/**
**	Convert minimap cursor Y position to tile map coordinate.
**
**	@param y	Screen Y pixel coordinate.
**	@return		Tile Y coordinate.
*/
global int ScreenMinimap2MapY(int y)
{
    int ty;

    ty=((((y)-TheUI.MinimapY-2-MinimapY)*MINIMAP_FAC)/MinimapScale);
    if( ty<0 ) {
	return 0;
    }
    return ty<TheMap.Height ? ty : TheMap.Height-1;
}

//@}
