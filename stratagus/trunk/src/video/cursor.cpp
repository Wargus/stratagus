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
/**@name cursor.c	-	The cursors. */
//
//	(c) Copyright 1998,2000,2001 by Lutz Sammer
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
#include "sound_id.h"
#include "unitsound.h"
#include "unittype.h"
#include "player.h"
#include "unit.h"
#include "cursor.h"
#include "tileset.h"
#include "map.h"
#include "interface.h"
#include "ui.h"

/*----------------------------------------------------------------------------
--	Variables
----------------------------------------------------------------------------*/

#ifdef noNEW_VIDEO

/**
**	Define cursor-types.
**
**	FIXME: Should move this to ui part.
*/
global CursorType Cursors[CursorMax] = {
#ifdef NEW_NAMES
    { "human",	"cursor-point",
	"human/cursors/human gauntlet.png",	 3, 2,	28,32 },
    { "orc",	"cursor-point",
	"orc/cursors/orcish claw.png",		 3, 2,	26,32 },
#else
    { "human",	"cursor-point",
	"human gauntlet.png",			 3, 2,	28,32 },
    { "orc",	"cursor-point",
	"orcish claw.png",			 3, 2,	26,32 },
    { "any",	"cursor-glass",
	"magnifying glass.png",			11,11,	34,35 },
#endif
};

#else

/**
**	Define cursor-types.
**
**	FIXME: Should make this better configurable.
*/
global CursorType Cursors[CursorMax] = {
    { { "human gauntlet.png"
	,"orcish claw.png" }
	, 3, 2,	28,32 },
    { { "magnifying glass.png"
	,NULL }
	,11,11,	34,35 },
    { { "small green cross.png"
	,NULL }
	, 8, 8,	18,18 },
    { { "yellow eagle.png"
	,"yellow crosshairs.png" }
	,15,15,	32,32 },
    { { "green eagle.png"
	,"green crosshairs.png" }
	,15,15,	32,32 },
    { { "red eagle.png"
	,"red crosshairs.png" }
	,15,15,	32,32 },
    { { "cross.png"
	,NULL }
	,15,15,	32,32 },

    { { "arrow E.png"
	,NULL }
	,22,10,	32,24 },
    { { "arrow N.png"
	,NULL }
	,12, 2,	32,24 },
    { { "arrow NE.png"
	,NULL }
	,20, 2,	32,24 },
    { { "arrow NW.png"
	,NULL }
	, 2, 2,	32,24 },
    { { "arrow S.png"
	,NULL }
	,12,22,	32,24 },
    { { "arrow SE.png"
	,NULL }
	,20,18,	32,24 },
    { { "arrow SW.png"
	,NULL }
	, 2,18,	32,24 },
    { { "arrow W.png"
	,NULL }
	, 4,10,	32,24 },
};

#endif

global enum CursorState_e CursorState;	/// cursor state
global int CursorAction;		/// action for selection
global int CursorValue;			/// value for CursorAction (spell type f.e.)
global UnitType* CursorBuilding;	/// building cursor

global CursorType* GameCursor;		/// cursor type
global int CursorX;			/// cursor position on screen X
global int CursorY;			/// cursor position on screen Y
global int CursorStartX;		/// rectangle started on screen X
global int CursorStartY;		/// rectangle started on screen Y

global int OldCursorX;			/// saved cursor position on screen X
global int OldCursorY;			/// saved cursor position on screen Y
global int OldCursorW;			/// saved cursor width in pixel
global int OldCursorH;			/// saved cursor height in pixel
global int OldCursorSize;		/// size of saved cursor image
global void* OldCursorImage;		/// background saved behind cursor

global int OldCursorRectangleX;		/// saved cursor position on screen X
global int OldCursorRectangleY;		/// saved cursor position on screen Y
global int OldCursorRectangleW;		/// saved cursor width in pixel
global int OldCursorRectangleH;		/// saved cursor height in pixel
global void* OldCursorRectangle=NULL;	/// background saved behind rectangle

	/// Function pointer: Save background behind cursor
local void (*SaveCursorBackground)(int,int,int,int);
	/// Function pointer: Load background behind cursor
local void (*LoadCursorBackground)(int,int,int,int);

/** Function pointer: Save rectangle behind cursor
**	@param x	Screen X pixels coordinate for left-top corner.
**	@param y	Screen Y pixels coordinate for left-top corner.
**	@param w	Width in pixels for rectangle starting at left-top.
**	@param h	Height in pixels for rectangle starting at left-top.
**Pre: the complete rectangle should be in Screen (no clipping) and non-empty
**     ( x>=0,y>=0,w>0,h>0,(x+w-1)<=VideoWidth,(y+h-1)<=VideoHeight )
*/
local void (*SaveCursorRectangle)(int x,int y,int w,int h);

/** Function pointer: Load rectangle behind cursor
**	@param x	Screen X pixels coordinate.
**	@param y	Screen Y pixels coordinate.
**	@param w	Width in pixels.
**	@param h	Height in pixels.
**Pre: rectangle previously saved with SaveCursorRectangle(x,y,w,h)
*/
local void (*LoadCursorRectangle)(int x,int y,int w,int h);

/*----------------------------------------------------------------------------
--	Functions
----------------------------------------------------------------------------*/
#define LOADCURSORRECTANGLE(video,memtype,x,y,w,h)  { \
    const memtype* sp; \
    memtype* dp; \
    sp=OldCursorRectangle; \
    dp=video+y*VideoWidth+x; \
    memcpy(dp,sp,w*sizeof(memtype)); \
    if ( h ) { \
      sp+=w; \
      dp+=VideoWidth; \
      while( --h ) { \
        *dp     = *sp++; \
        dp[w-1] = *sp++; \
        dp+=VideoWidth; \
      } \
      memcpy(dp,sp,w*sizeof(memtype)); \
    } \
}
#define SAVECURSORRECTANGLE(video,memtype,x,y,w,h)  { \
    const memtype* sp; \
    memtype* dp; \
    sp=video+y*VideoWidth+x; \
    dp=OldCursorRectangle; \
    memcpy(dp,sp,w*sizeof(memtype)); \
    if ( --h ) { \
      dp+=w; \
      sp+=VideoWidth; \
      while( --h ) { \
       *dp++ = *sp; \
       *dp++ = sp[w-1]; \
       sp+=VideoWidth; \
      } \
      memcpy(dp,sp,w*sizeof(memtype)); \
    } \
}

/**  Restore cursor rectangle for 8bpp frame buffer.
**   (See description function pointer LoadCursorRectangle)
*/
local void LoadCursorRectangle8(int x,int y,int w,int h) {
    if( w && h ) {
	LOADCURSORRECTANGLE(VideoMemory8,VMemType8,x,y,w,h);
    }
}

/** Restore cursor rectangle for 16bpp frame buffer.
**   (See description function pointer LoadCursorRectangle)
*/
local void LoadCursorRectangle16(int x,int y,int w,int h) {
    if( w && h ) {
	LOADCURSORRECTANGLE(VideoMemory16,VMemType16,x,y,w,h);
    }
}

/** Restore cursor rectangle for 24bpp frame buffer.
**   (See description function pointer LoadCursorRectangle)
*/
local void LoadCursorRectangle24(int x,int y,int w,int h) {
    if( w && h ) {
	LOADCURSORRECTANGLE(VideoMemory24,VMemType24,x,y,w,h);
    }
}

/** Restore cursor rectangle for 32bpp frame buffer.
**   (See description function pointer LoadCursorRectangle)
*/
local void LoadCursorRectangle32(int x,int y,int w,int h) {
    if( w && h ) {
	LOADCURSORRECTANGLE(VideoMemory32,VMemType32,x,y,w,h);
    }
}

/** Save cursor rectangle for 8bpp frame buffer.
**   (See description function pointer SaveCursorRectangle)
*/
local void SaveCursorRectangle8(int x,int y,int w,int h) {
    if( w && h ) {
	SAVECURSORRECTANGLE(VideoMemory8,VMemType8,x,y,w,h);
    }
}

/** Save cursor rectangle for 16bpp frame buffer.
**   (See description function pointer SaveCursorRectangle)
*/
local void SaveCursorRectangle16(int x,int y,int w,int h) {
    if( w && h ) {
	SAVECURSORRECTANGLE(VideoMemory16,VMemType16,x,y,w,h);
    }
}

/** Save cursor rectangle for 24bpp frame buffer.
**   (See description function pointer SaveCursorRectangle)
*/
local void SaveCursorRectangle24(int x,int y,int w,int h) {
    if( w && h ) {
	SAVECURSORRECTANGLE(VideoMemory24,VMemType24,x,y,w,h);
    }
}

/** Save cursor rectangle for 32bpp frame buffer.
**   (See description function pointer SaveCursorRectangle)
*/
local void SaveCursorRectangle32(int x,int y,int w,int h) {
    if( w && h ) {
	SAVECURSORRECTANGLE(VideoMemory32,VMemType32,x,y,w,h);
    }
}

/**
**	Restore cursor background for 8bpp frame buffer.
**
**	@param x	Screen X pixels coordinate.
**	@param y	Screen Y pixels coordinate.
**	@param w	Width in pixels.
**	@param h	Height in pixels.
*/
local void LoadCursorBackground8(int x,int y,int w,int h)
{
    const VMemType8* sp;
    VMemType8* dp;

    sp=OldCursorImage;
    dp=VideoMemory8+y*VideoWidth+x;
    while( h-- ) {
	memcpy(dp,sp,w*sizeof(VMemType8));
	sp+=w;
	dp+=VideoWidth;
    }
}

/**
**	Restore cursor background for 16bpp frame buffer.
**
**	@param x	Screen X pixels coordinate.
**	@param y	Screen Y pixels coordinate.
**	@param w	Width in pixels.
**	@param h	Height in pixels.
*/
local void LoadCursorBackground16(int x,int y,int w,int h)
{
    const VMemType16* sp;
    VMemType16* dp;

    sp=OldCursorImage;
    dp=VideoMemory16+y*VideoWidth+x;
    while( h-- ) {
	memcpy(dp,sp,w*sizeof(VMemType16));
	sp+=w;
	dp+=VideoWidth;
    }
}

/**
**	Restore cursor background for 24bpp frame buffer.
**
**	@param x	Screen X pixels coordinate.
**	@param y	Screen Y pixels coordinate.
**	@param w	Width in pixels.
**	@param h	Height in pixels.
*/
local void LoadCursorBackground24(int x,int y,int w,int h)
{
    const VMemType24* sp;
    VMemType24* dp;

    sp=OldCursorImage;
    dp=VideoMemory24+y*VideoWidth+x;
    while( h-- ) {
	memcpy(dp,sp,w*sizeof(VMemType24));
	sp+=w;
	dp+=VideoWidth;
    }
}

/**
**	Restore cursor background for 32bpp frame buffer.
**
**	@param x	Screen X pixels coordinate.
**	@param y	Screen Y pixels coordinate.
**	@param w	Width in pixels.
**	@param h	Height in pixels.
*/
local void LoadCursorBackground32(int x,int y,int w,int h)
{
    const VMemType32* sp;
    VMemType32* dp;

    sp=OldCursorImage;
    dp=VideoMemory32+y*VideoWidth+x;
    while( h-- ) {
	memcpy(dp,sp,w*sizeof(VMemType32));
	sp+=w;
	dp+=VideoWidth;
    }
}

/**
**	Save cursor background for 8bpp frame buffer.
**
**	@param x	Screen X pixels coordinate.
**	@param y	Screen Y pixels coordinate.
**	@param w	Width in pixels.
**	@param h	Height in pixels.
*/
local void SaveCursorBackground8(int x,int y,int w,int h)
{
    int i;
    VMemType8* dp;
    VMemType8* sp;

    i=w*h*sizeof(VMemType8);
    if( OldCursorSize<i ) {
	if( OldCursorImage ) {
	    OldCursorImage=realloc(OldCursorImage,i);
	} else {
	    OldCursorImage=malloc(i);
	}
	OldCursorSize=i;
    }
    dp=OldCursorImage;
    sp=VideoMemory8+y*VideoWidth+x;
    while( h-- ) {
	memcpy(dp,sp,w*sizeof(VMemType8));
	dp+=w;
	sp+=VideoWidth;
    }
}

/**
**	Save cursor background for 16bpp frame buffer.
**
**	@param x	Screen X pixels coordinate.
**	@param y	Screen Y pixels coordinate.
**	@param w	Width in pixels.
**	@param h	Height in pixels.
*/
local void SaveCursorBackground16(int x,int y,int w,int h)
{
    int i;
    VMemType16* dp;
    const VMemType16* sp;

    i=w*h*sizeof(VMemType16);
    if( OldCursorSize<i ) {
	if( OldCursorImage ) {
	    OldCursorImage=realloc(OldCursorImage,i);
	} else {
	    OldCursorImage=malloc(i);
	}
	OldCursorSize=i;
    }
    dp=OldCursorImage;
    sp=VideoMemory16+y*VideoWidth+x;
    while( h-- ) {
	memcpy(dp,sp,w*sizeof(VMemType16));
	dp+=w;
	sp+=VideoWidth;
    }
}

/**
**	Save cursor background for 24bpp frame buffer.
**
**	@param x	Screen X pixels coordinate.
**	@param y	Screen Y pixels coordinate.
**	@param w	Width in pixels.
**	@param h	Height in pixels.
*/
local void SaveCursorBackground24(int x,int y,int w,int h)
{
    int i;
    VMemType24* dp;
    const VMemType24* sp;

    i=w*h*sizeof(VMemType24);
    if( OldCursorSize<i ) {
	if( OldCursorImage ) {
	    OldCursorImage=realloc(OldCursorImage,i);
	} else {
	    OldCursorImage=malloc(i);
	}
	OldCursorSize=i;
    }
    dp=OldCursorImage;
    sp=VideoMemory24+y*VideoWidth+x;
    while( h-- ) {
	memcpy(dp,sp,w*sizeof(VMemType24));
	dp+=w;
	sp+=VideoWidth;
    }
}

/**
**	Save cursor background for 32bpp frame buffer.
**
**	@param x	Screen X pixels coordinate.
**	@param y	Screen Y pixels coordinate.
**	@param w	Width in pixels.
**	@param h	Height in pixels.
*/
local void SaveCursorBackground32(int x,int y,int w,int h)
{
    int i;
    VMemType32* dp;
    const VMemType32* sp;

    i=w*h*sizeof(VMemType32);
    if( OldCursorSize<i ) {
	if( OldCursorImage ) {
	    OldCursorImage=realloc(OldCursorImage,i);
	} else {
	    OldCursorImage=malloc(i);
	}
	OldCursorSize=i;
    }
    dp=OldCursorImage;
    sp=VideoMemory32+y*VideoWidth+x;
    while( h-- ) {
	memcpy(dp,sp,w*sizeof(VMemType32));
	dp+=w;
	sp+=VideoWidth;
    }
}

/**
**	Load all cursor sprites.
**
**	@param race	Cursor graphics of this race to load.
*/
global void LoadCursors(unsigned int race)
{
    int i;
    const char* file;
    static int last_race = -1;

    if (race == last_race) {	// same race? already loaded!
	return;
    }

    if (last_race != -1) {	// free previous sprites for different race
	for( i=0; i<sizeof(Cursors)/sizeof(*Cursors); ++i ) {
	    VideoSaveFree(Cursors[i].Sprite);
	    Cursors[i].Sprite = NULL;
	}
    }
    last_race = race;

    //
    //	Load the graphics
    //
    for( i=0; i<sizeof(Cursors)/sizeof(*Cursors); ++i ) {
	if( !(file=Cursors[i].File[race]) ) {
	    file=Cursors[i].File[0];	// default one, no race specific
	}
	// FIXME: size and hot-point extra!
	if( file ) {
	    char* buf;

	    buf=alloca(strlen(file)+9+1);
	    file=strcat(strcpy(buf,"graphic/"),file);
	    ShowLoadProgress("\tCursor %s\n",file);
	    // FIXME: real size?
	    Cursors[i].Sprite=LoadSprite(file,0,0);
	}
    }
}

/**
**	Save image behind cursor.
*/
local void SaveCursor(void)
{
    int w;
    int h;
    int x;
    int y;

    x=OldCursorX;
    w=OldCursorW;
    if( x<0 ) {
	w-=x;
	x=0;
    }
    if( w>VideoWidth-x) {	// normalize width
	w=VideoWidth-x;
    }
    if( !w ) {
	return;
    }

    y=OldCursorY;
    h=OldCursorH;
    if( y<0 ) {
	w-=y;
	y=0;
    }
    if( h>VideoHeight-y ) {	// normalize height
	h=VideoHeight-y;
    }
    if( !h ) {
	return;
    }

    SaveCursorBackground(x,y,w,h);
}

/**
**	Destroy image behind cursor.
*/
global void DestroyCursorBackground(void)
{
    if (OldCursorImage) {
	free(OldCursorImage);
	OldCursorImage = NULL;
    }
    OldCursorSize = 0;
}

/**
**	Restore image behind cursor.
*/
local void RestoreCursor(void)
{
    int w;
    int h;
    int x;
    int y;

    if( !OldCursorImage ) {		// no cursor saved
	return;
    }

    // FIXME: I should better store the correct values on save.
    x=OldCursorX;
    w=OldCursorW;
    if( x<0 ) {
	w-=x;
	x=0;
    }
    if( w>VideoWidth-x) {	// normalize width
	w=VideoWidth-x;
    }
    if( !w ) {
	return;
    }

    y=OldCursorY;
    h=OldCursorH;
    if( y<0 ) {
	w-=y;
	y=0;
    }
    if( h>VideoHeight-y ) {	// normalize height
	h=VideoHeight-y;
    }
    if( !h ) {
	return;
    }

    LoadCursorBackground(x,y,w,h);
}

/**
**	Draw cursor.
**
**	@param type	Cursor type.
**	FIXME: docu
*/
global void DrawCursor(CursorType* type,int x,int y,int frame)
{
    OldCursorX=x-=type->HotX;
    OldCursorY=y-=type->HotY;
    OldCursorW=VideoGraphicWidth(type->Sprite);
    OldCursorH=VideoGraphicHeight(type->Sprite);

/* FIXME: SaveCusor done before rectangle
    SaveCursor();
*/
    VideoDrawClip(type->Sprite,frame,x,y);
}

/**
**	Hide cursor.
*/
global void HideCursor(void)
{
    RestoreCursor();
}

/*----------------------------------------------------------------------------
--	DRAW CURSOR
----------------------------------------------------------------------------*/

local int RectangleCursor;		/// Flag: last cursor was rectangle
local int BuildingCursor;		/// Flag: last cursor was building

// area of tiles covered by building cursor (SX,SY;EX,EY)
local int BuildingCursorSX;
local int BuildingCursorSY;
local int BuildingCursorEX;
local int BuildingCursorEY;

/**
**	Draw cursor for selecting building position.
*/
local void DrawBuildingCursor(void)
{
    int x;
    int y;
    int x1;
    int y1;
    int mx;
    int my;
    int color;
    int f;
    int w;
    int w0;
    int h;
    int mask;

    // Align to grid
    x=CursorX-(CursorX-TheUI.MapX)%TileSizeX;
    y=CursorY-(CursorY-TheUI.MapY)%TileSizeY;
    BuildingCursorSX=mx=Screen2MapX(x);
    BuildingCursorSY=my=Screen2MapY(y);

    //
    //	Draw building
    //
    PushClipping();
    SetClipping(TheUI.MapX,TheUI.MapY,TheUI.MapEndX,TheUI.MapEndY);
    GraphicPlayerPixels(ThisPlayer,CursorBuilding->Sprite);
    DrawUnitType(CursorBuilding,0,x,y);
    PopClipping();

    //
    //	Draw the allow overlay
    //
    f=CanBuildHere(CursorBuilding,mx,my);
    // FIXME: Should be moved into unittype structure, and allow more types.
    if( CursorBuilding->ShoreBuilding ) {
	mask=MapFieldLandUnit
		| MapFieldSeaUnit
		| MapFieldBuilding	// already occuppied
		| MapFieldWall
		| MapFieldRocks
		| MapFieldForest	// wall,rock,forest not 100% clear?
		| MapFieldLandAllowed	// can't build on this
		//| MapFieldUnpassable	// FIXME: I think shouldn't be used
		| MapFieldNoBuilding;
    } else switch( CursorBuilding->UnitType ) {
	case UnitTypeLand:
	    mask=MapFieldLandUnit
		| MapFieldBuilding	// already occuppied
		| MapFieldWall
		| MapFieldRocks
		| MapFieldForest	// wall,rock,forest not 100% clear?
		| MapFieldCoastAllowed
		| MapFieldWaterAllowed	// can't build on this
		| MapFieldUnpassable	// FIXME: I think shouldn't be used
		| MapFieldNoBuilding;
	    break;
	case UnitTypeNaval:
	    mask=MapFieldSeaUnit
		| MapFieldBuilding	// already occuppied
		| MapFieldCoastAllowed
		| MapFieldLandAllowed	// can't build on this
		| MapFieldUnpassable	// FIXME: I think shouldn't be used
		| MapFieldNoBuilding;
	    break;
	case UnitTypeFly:
	default:
	    DebugLevel1Fn("Were moves this unit?\n");
	    return;
    }

    h=CursorBuilding->TileHeight;
    BuildingCursorEY=my+h-1;
    if( my+h>MapY+MapHeight ) {		// reduce to view limits
	h=MapY+MapHeight-my;
    }
    w0=CursorBuilding->TileWidth;	// reduce to view limits
    BuildingCursorEX=mx+w0-1;
    if( mx+w0>MapX+MapWidth ) {
	w0=MapX+MapWidth-mx;
    }
    while( h-- ) {
	w=w0;
	while( w-- ) {
	    if( f && (CanBuildOn(mx+w,my+h,mask) ||
		    (Selected[0]->X==mx+w && Selected[0]->Y==my+h))
                  && IsMapFieldExplored(mx+w,my+h) ) {
		color=ColorGreen;
	    } else {
		color=ColorRed;
	    }
	    // FIXME: I could do this faster+better
	    for( y1=0; y1<TileSizeY; ++y1 ) {
		for( x1=y1&1; x1<TileSizeX; x1+=2 ) {
		    VideoDrawPixel(color,x+w*TileSizeX+x1,y+h*TileSizeY+y1);
		}
	    }
	}
    }
}

/**
**	Draw rectangle cursor.
*/
global void DrawRectangleCursor(void)
{
    int x;
    int y;
    int w;
    int h;
    int x1;
    int y1;

    //
    //	Clip to map window.
    //
    x1=CursorX;
    if( x1<TheUI.MapX ) {
	x1=TheUI.MapX;
    } else if( x1>TheUI.MapEndX ) {
	x1=TheUI.MapEndX;
    }
    y1=CursorY;
    if( y1<TheUI.MapY ) {
	y1=TheUI.MapY;
    } else if( y1>TheUI.MapEndY ) {
	y1=TheUI.MapEndY;
    }

    x=CursorStartX;
    if( x>x1 ) {
	x=x1;
	w=CursorStartX-x+1;
    } else {
	w=x1-x+1;
    }
    y=CursorStartY;
    if( y>y1 ) {
	y=y1;
	h=CursorStartY-y+1;
    } else {
	h=y1-y+1;
    }

#ifdef NEW_MAPDRAW
    SaveCursorRectangle(OldCursorRectangleX=x,OldCursorRectangleY=y,
                        OldCursorRectangleW=w,OldCursorRectangleH=h);
    VideoDrawHLine(ColorGreen,x,y,w);
    VideoDrawVLine(ColorGreen,x,y+1,--h);
    VideoDrawHLine(ColorGreen,x+1,y+h,--w);
    VideoDrawVLine(ColorGreen,x+w,y,h);
#else
    //FIXME VideoDrawRectangleClip uses different definition for w and h
    VideoDrawRectangleClip(ColorGreen,x,y,w-1,h-1);
#endif
}

/**
**	Draw the cursor
*/
global void DrawAnyCursor(void)
{
    RectangleCursor=BuildingCursor=0;

/*FIXME: temporary, as SaveCursor must happen before rectangle
*/
    OldCursorX=CursorX-GameCursor->HotX;
    OldCursorY=CursorY-GameCursor->HotY;
    OldCursorW=VideoGraphicWidth(GameCursor->Sprite);
    OldCursorH=VideoGraphicHeight(GameCursor->Sprite);
    SaveCursor();

    //
    //	Selecting rectangle
    //
    if( CursorState==CursorStateRectangle
	     && (CursorStartX!=CursorX || CursorStartY!=CursorY) ) {
	DrawRectangleCursor();
	RectangleCursor=1;
    } else

    //
    //	Selecting position for building
    //
    if( CursorBuilding && CursorOn==CursorOnMap ) {
	DrawBuildingCursor();
	BuildingCursor=1;
    }

    //
    //	Normal cursor.
    //
    DrawCursor(GameCursor,CursorX,CursorY,0);
}

/**
**	Remove old cursor from display.
*/
global int HideAnyCursor(void)
{
   if( RectangleCursor ) {
     LoadCursorRectangle(OldCursorRectangleX,OldCursorRectangleY,
                         OldCursorRectangleW,OldCursorRectangleH);
    }
    if( BuildingCursor ) {
        //NOTE: this will restore tiles themselves later in next video update
        MarkDrawAreaMap(BuildingCursorSX,BuildingCursorSY,
                        BuildingCursorEX,BuildingCursorEY);
    }

    //
    //	Cursor complete on map and map must be redrawn, no restore.
    //StephanR: but prevented when not entire map is redrawn ;)
    //
    #ifndef NEW_MAPDRAW
    if( OldCursorX>=TheUI.MapX
	    && OldCursorX+OldCursorW-1<=TheUI.MapEndX
	    && OldCursorY>=TheUI.MapY
	    && OldCursorY+OldCursorH-1<=TheUI.MapEndY
	    && (MustRedraw&RedrawMap)
	    && (InterfaceState != IfaceStateMenu) ) {
	return 0;
    }
    #endif
    HideCursor();
    return 1;
}

/**
**	Setup the cursor part.
**FIXME: Now max possible memory for OldCursorRectangle, to be limited to Map?
*/
global void InitCursor(void)
{
    int memsize;

    if( OldCursorRectangle ) {	// memory of possible previous video-setting?
	free( OldCursorRectangle );
    }

    switch( VideoBpp ) {
	case 8:
	    SaveCursorBackground=SaveCursorBackground8;
	    LoadCursorBackground=LoadCursorBackground8;
            memsize=sizeof(VMemType8);
	    SaveCursorRectangle=SaveCursorRectangle8;
	    LoadCursorRectangle=LoadCursorRectangle8;
	    break;
	case 15:
	case 16:
	    SaveCursorBackground=SaveCursorBackground16;
	    LoadCursorBackground=LoadCursorBackground16;
            memsize=sizeof(VMemType16);
	    SaveCursorRectangle=SaveCursorRectangle16;
	    LoadCursorRectangle=LoadCursorRectangle16;
	    break;
	case 24:
	    SaveCursorBackground=SaveCursorBackground24;
	    LoadCursorBackground=LoadCursorBackground24;
            memsize=sizeof(VMemType24);
	    SaveCursorRectangle=SaveCursorRectangle24;
	    LoadCursorRectangle=LoadCursorRectangle24;
	    break;
	case 32:
	    SaveCursorBackground=SaveCursorBackground32;
	    LoadCursorBackground=LoadCursorBackground32;
            memsize=sizeof(VMemType32);
	    SaveCursorRectangle=SaveCursorRectangle32;
	    LoadCursorRectangle=LoadCursorRectangle32;
	    break;
	default:
	    DebugLevel0Fn("unsupported %d bpp\n",VideoBpp);
	    abort();
    }
    OldCursorRectangle=malloc((2*VideoWidth+2*(VideoHeight-2))*memsize);
}

//@}
