//   ___________		     _________		      _____  __
//   \_	  _____/______	 ____	____ \_	  ___ \____________ _/ ____\/  |_
//    |	   __) \_  __ \_/ __ \_/ __ \/	  \  \/\_  __ \__  \\	__\\   __\ 
//    |	    \	|  | \/\  ___/\	 ___/\	   \____|  | \// __ \|	|   |  |
//    \___  /	|__|	\___  >\___  >\______  /|__|  (____  /__|   |__|
//	  \/		    \/	   \/	     \/		   \/
//  ______________________			     ______________________
//			  T H E	  W A R	  B E G I N S
//	   FreeCraft - A free fantasy real time strategy game engine
//
/**@name cursor.c	-	The cursors. */
//
//	(c) Copyright 1998,2000,2001 by Lutz Sammer
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
#include "unittype.h"
#include "player.h"
#include "unit.h"
#include "cursor.h"
#include "tileset.h"
#include "map.h"
#include "interface.h"
#include "ui.h"

#include "intern_video.h"

/*----------------------------------------------------------------------------
--	Variables
----------------------------------------------------------------------------*/

/**
**	Number of bytes needed for current video-mode
*/
local int memsize;

/**
**	Cursor-type type definition
*/
global const char CursorTypeType[] = "cursor-type";

/**
**	Define cursor-types.
**
**	@todo FIXME: Should this be move to ui part?
*/
global CursorType* Cursors;

global CursorStates CursorState;/// current cursor state (point,...)
global int CursorAction;	/// action for selection
global int CursorValue;		/// value for CursorAction (spell type f.e.)

	//Event changed mouse position, can alter at any moment
global int CursorX;		/// cursor position on screen X
global int CursorY;		/// cursor position on screen Y

global int CursorStartX;	/// rectangle started on screen X
global int CursorStartY;	/// rectangle started on screen Y


/*--- DRAW BUILDING  CURSOR ------------------------------------------------*/
local int BuildingCursor=0;		/// Flag (0/1): last cursor was building

// area of tiles covered by building cursor (SX,SY;EX,EY)
local int BuildingCursorSX;		/// FIXME: docu
local int BuildingCursorSY;		/// FIXME: docu
local int BuildingCursorEX;		/// FIXME: docu
local int BuildingCursorEY;		/// FIXME: docu

global UnitType* CursorBuilding;/// building cursor


/*--- DRAW SPRITE CURSOR ---------------------------------------------------*/
	// Saved area after draw cursor, needed later to hide it again
	// (OldCursorW!=0 denotes it's defined)
local int OldCursorInvalidate=0;/// flag (0/1): if cursor need invalidate
local int OldCursorX;		/// saved cursor position on screen X
local int OldCursorY;		/// saved cursor position on screen Y
local int OldCursorW=0;		/// saved cursor width in pixel
local int OldCursorH;		/// saved cursor height in pixel
global CursorType* GameCursor;	/// current shown cursor-type

	// Area which is already hidden, but needed for invalidate
	// (HiddenCursorW!=0 denotes it's defined)
local int HiddenCursorX;	/// saved cursor position on screen X
local int HiddenCursorY;	/// saved cursor position on screen Y
local int HiddenCursorW=0;	/// saved cursor width in pixel
local int HiddenCursorH;	/// saved cursor height in pixel

        /// Memory re-use, so can be defined although no save present!
local unsigned int OldCursorSize;	/// size of saved cursor image
local void* OldCursorImage;		/// background saved behind cursor

	/// Function pointer: Save background behind cursor
/**
**	Function pointer: Save 2D image behind sprite cursor
**
**	@param x	Screen X pixels coordinate for left-top corner.
**	@param y	Screen Y pixels coordinate for left-top corner.
**	@param w	Width in pixels for image starting at left-top.
**	@param h	Height in pixels for image starting at left-top.
**
**	@note the complete image should be in Screen (no clipping) and
**	non-empty
**     ( x>=0,y>=0,w>0,h>0,(x+w-1)<=VideoWidth,(y+h-1)<=VideoHeight )
*/
local void (*SaveCursorBackground)(int x,int y,int w,int h);
	/// Function pointer: Load background behind cursor
local void (*LoadCursorBackground)(int x,int y,int w,int h);


/*--- DRAW RECTANGLE CURSOR ------------------------------------------------*/
	// Saved area after draw rectangle, needed later to hide it again
	// (OldCursorRectangleW!=0 denotes it's defined)
local int OldCursorRectangleInvalidate=0;/// flag (0/1): ..need invalidate
local int OldCursorRectangleX;		/// saved cursor position on screen X
local int OldCursorRectangleY;		/// saved cursor position on screen Y
local int OldCursorRectangleW=0;	/// saved cursor width in pixel
local int OldCursorRectangleH;		/// saved cursor height in pixel
local void* OldCursorRectangle;		/// background saved behind rectangle

	// Area which is already hidden, but needed for invalidate
	// (HiddenCursorRectangleW!=0 denotes it's defined)
local int HiddenCursorRectangleX;	/// saved cursor position on screen X
local int HiddenCursorRectangleY;	/// saved cursor position on screen Y
local int HiddenCursorRectangleW=0;	/// saved cursor width in pixel
local int HiddenCursorRectangleH;	/// saved cursor height in pixel

/**
**	Function pointer: Save rectangle behind cursor
**
**	@param x	Screen X pixels coordinate for left-top corner.
**	@param y	Screen Y pixels coordinate for left-top corner.
**	@param w	Width in pixels for rectangle starting at left-top.
**	@param h	Height in pixels for rectangle starting at left-top.
**
**	@note the complete rectangle should be in Screen (no clipping) and
**	non-empty
**     ( x>=0,y>=0,w>0,h>0,(x+w-1)<=VideoWidth,(y+h-1)<=VideoHeight )
*/
local void (*SaveCursorRectangle)(int x,int y,int w,int h);

/**
**	Function pointer: Load rectangle behind cursor
**
**	@param x	Screen X pixels coordinate.
**	@param y	Screen Y pixels coordinate.
**	@param w	Width in pixels.
**	@param h	Height in pixels.
**
**	@note rectangle previously saved with SaveCursorRectangle(x,y,w,h)
*/
local void (*LoadCursorRectangle)(int x,int y,int w,int h);

/*----------------------------------------------------------------------------
--	Functions
----------------------------------------------------------------------------*/
/**
**	Load all cursor sprites.
**
**	@param race	Cursor graphics of this race to load.
*/
global void LoadCursors(const char* race)
{
    int i;
    const char* file;

    //
    //	Free old cursor sprites.
    //
    for( i=0; Cursors[i].OType; ++i ) {
	VideoSaveFree(Cursors[i].Sprite);
	Cursors[i].Sprite = NULL;
    }

    //
    //	Load the graphics
    //
    for( i=0; Cursors[i].OType; ++i ) {
	//
	//	Only load cursors of this race or universal cursors.
	//
	if( Cursors[i].Race && strcmp(Cursors[i].Race,race) ) {
	    continue;
	}

	file=Cursors[i].File;
	if( file ) {
	    char* buf;

	    buf=alloca(strlen(file)+9+1);
	    file=strcat(strcpy(buf,"graphics/"),file);
	    ShowLoadProgress("\tCursor %s\n",file);
	    Cursors[i].Sprite=LoadSprite(file,
		    Cursors[i].Width,Cursors[i].Height);
	}
    }
}

/**
**	Find the cursor-type of with this identifier.
**
**	@param ident	Identifier for the cursor (from config files).
**
**	@return		Returns the matching cursor-type.
**
**	@note If we have more cursors, we should add hash to find them faster.
*/
global CursorType* CursorTypeByIdent(const char* ident)
{
    CursorType* cursortype;

    for( cursortype=Cursors; cursortype->OType; ++cursortype ) {
	if( strcmp(cursortype->Ident,ident) ) {
	    continue;
	}
	if( !cursortype->Race || cursortype->Sprite ) {
	    return cursortype;
	}
    }
    DebugLevel0Fn("Cursor `%s' not found, please check your code.\n",ident);
    return NULL;
}

/*----------------------------------------------------------------------------
--	DRAW RECTANGLE CURSOR
----------------------------------------------------------------------------*/
/**
**	Puts stored 'image' from SAVECURSORRECTANGLE back on the screen.
**      Note w and h are both > 0
**
**	FIXME: this kind of macros are hard to single step with gdb.
**	FIXME: inline functions should have the same speed and are debugable.
*/
#define LOADCURSORRECTANGLE(video,memtype,x,y,w,h)  { \
    const memtype* sp; \
    memtype* dp; \
    sp=OldCursorRectangle; \
    dp=video+y*VideoWidth+x; \
    memcpy(dp,sp,w*sizeof(memtype)); \
    if ( --h ) { \
      sp+=w; \
      dp+=VideoWidth; \
      while( --h ) { \
	*dp	= *sp++; \
	dp[w-1] = *sp++; \
	dp+=VideoWidth; \
      } \
      memcpy(dp,sp,w*sizeof(memtype)); \
    } \
}

/**
**	Saves 'image' of screen overlapped by rectangle cursor, to be able to
**      restore it later using LOADCURSORRECTANGLE.
**      Note w and h > 0
**
**	FIXME: this kind of macros are hard to single step with gdb.
**	FIXME: inline functions should have the same speed and are debugable.
*/
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
local void LoadCursorRectangle8(int x,int y,int w,int h)
{
	LOADCURSORRECTANGLE(VideoMemory8,VMemType8,x,y,w,h);
}

/** Restore cursor rectangle for 16bpp frame buffer.
**   (See description function pointer LoadCursorRectangle)
**	@see LoadCursorRectangle
*/
local void LoadCursorRectangle16(int x,int y,int w,int h)
{
	LOADCURSORRECTANGLE(VideoMemory16,VMemType16,x,y,w,h);
}

/** Restore cursor rectangle for 24bpp frame buffer.
**   (See description function pointer LoadCursorRectangle)
**	@see LoadCursorRectangle
*/
local void LoadCursorRectangle24(int x,int y,int w,int h)
{
	LOADCURSORRECTANGLE(VideoMemory24,VMemType24,x,y,w,h);
}

/** Restore cursor rectangle for 32bpp frame buffer.
**   (See description function pointer LoadCursorRectangle)
**	@see LoadCursorRectangle
*/
local void LoadCursorRectangle32(int x,int y,int w,int h)
{
	LOADCURSORRECTANGLE(VideoMemory32,VMemType32,x,y,w,h);
}

/** Save cursor rectangle for 8bpp frame buffer.
**   (See description function pointer SaveCursorRectangle)
**	@see SaveCursorRectangle
*/
local void SaveCursorRectangle8(int x,int y,int w,int h)
{
	SAVECURSORRECTANGLE(VideoMemory8,VMemType8,x,y,w,h);
}

/** Save cursor rectangle for 16bpp frame buffer.
**   (See description function pointer SaveCursorRectangle)
**	@see SaveCursorRectangle
*/
local void SaveCursorRectangle16(int x,int y,int w,int h)
{
	SAVECURSORRECTANGLE(VideoMemory16,VMemType16,x,y,w,h);
}

/** Save cursor rectangle for 24bpp frame buffer.
**   (See description function pointer SaveCursorRectangle)
**	@see SaveCursorRectangle
*/
local void SaveCursorRectangle24(int x,int y,int w,int h)
{
	SAVECURSORRECTANGLE(VideoMemory24,VMemType24,x,y,w,h);
}

/** Save cursor rectangle for 32bpp frame buffer.
**   (See description function pointer SaveCursorRectangle)
**	@see SaveCursorRectangle
*/
local void SaveCursorRectangle32(int x,int y,int w,int h)
{
	SAVECURSORRECTANGLE(VideoMemory32,VMemType32,x,y,w,h);
}

/**
**	Draw rectangle cursor when visible, defined by
**      OldCursorRectangleW (!=0),.. 
**      Pre: for this to work OldCursorRectangleW should be 0 upfront
*/
local void DrawVisibleRectangleCursor(int x,int y,int x1,int y1)
{
    int w;
    int h;

    //
    //	Clip to map window.
    // FIXME: should re-use CLIP_RECTANGLE in some way from linedraw.c ?
    //
    if( x1<TheUI.MapX ) {
	x1=TheUI.MapX;
    } else if( x1>TheUI.MapEndX ) {
	x1=TheUI.MapEndX;
    }
    if( y1<TheUI.MapY ) {
	y1=TheUI.MapY;
    } else if( y1>TheUI.MapEndY ) {
	y1=TheUI.MapEndY;
    }

    if( x>x1 ) {
	x=x1;
	w=CursorStartX-x+1;
    } else {
	w=x1-x+1;
    }
    if( y>y1 ) {
	y=y1;
	h=CursorStartY-y+1;
    } else {
	h=y1-y+1;
    }

    if ( w && h )
    {
      SaveCursorRectangle(OldCursorRectangleX=x,OldCursorRectangleY=y,
  			  OldCursorRectangleW=w,OldCursorRectangleH=h);
      VideoDrawRectangleClip(ColorGreen,x,y,w,h);
      OldCursorRectangleInvalidate=1;
    }
}


/*----------------------------------------------------------------------------
--	DRAW SPRITE CURSOR
----------------------------------------------------------------------------*/
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
    VMemType8* dp;
    VMemType8* sp;

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
    VMemType16* dp;
    const VMemType16* sp;

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
    VMemType24* dp;
    const VMemType24* sp;

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
    VMemType32* dp;
    const VMemType32* sp;

    dp=OldCursorImage;
    sp=VideoMemory32+y*VideoWidth+x;
    while( h-- ) {
	memcpy(dp,sp,w*sizeof(VMemType32));
	dp+=w;
	sp+=VideoWidth;
    }
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
**	Draw (sprite) cursor when visible, defined by
**      OldCursorW (!=0),.. 
**      Pre: for this to work OldCursorW should be 0 upfront
**
**	@param type	Cursor-type of the cursor to draw.
**	@param x	Screen x pixel position.
**	@param y	Screen y pixel position.
**	@param frame	Animation frame # of the cursor.
*/
local void DrawCursor(const CursorType* type,int x,int y,int frame)
{
    unsigned int size,w,h;
    int spritex,spritey;

    //
    //	Save cursor position and size, for faster cursor redraw.
    //
    spritex=(x-=type->HotX);
    spritey=(y-=type->HotY);
    w=VideoGraphicWidth(type->Sprite);
    h=VideoGraphicHeight(type->Sprite);

    //Reserve enough memory for background of sprite (also for future calls)
    size=(unsigned int)w*(unsigned int)h*memsize;
    if( OldCursorSize<size ) {
	if( OldCursorImage ) {
	    OldCursorImage=realloc(OldCursorImage,size);
	} else {
	    OldCursorImage=malloc(size);
	}
	OldCursorSize=size;
    }

    //Save (seen) area behind sprite
    CLIP_RECTANGLE(x,y,w,h);
    SaveCursorBackground(OldCursorX=x,OldCursorY=y,OldCursorW=w,OldCursorH=h);

    //Draw sprite (using its own clipping)  FIXME: prevent clipping twice
    VideoDrawClip(type->Sprite,frame,spritex,spritey);
    OldCursorInvalidate=1;
}

/*----------------------------------------------------------------------------
--	DRAW BUILDING CURSOR
----------------------------------------------------------------------------*/
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
	    // FIXME: The field is covered by fog of war!
	    if( f && CanBuildOn(mx+w,my+h,mask & 
		    ((Selected[0]->X==mx+w && Selected[0]->Y==my+h)
			? ~(MapFieldLandUnit|MapFieldSeaUnit) : -1))
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


/*----------------------------------------------------------------------------
--	DRAW/HIDE CURSOR (interface for the outside world)
----------------------------------------------------------------------------*/
/**
**	Draw the cursor and prepare tobe restored by HideAnyCursor again.
**      Note: This function can be called, without calling HideAnyCursor first,
**            which means that this function should re-use/free memory of the
**            last call.
**            When calling multiple times, the old cursor is expected to be
**            overdrawn by something else (else HideAnyCursor is needed!)
**            Also the cursors are not invalidated (refresh on real screen)
**            here, but this is done by InvalidateCursorAreas.
**
**  FIXME: event handler should be temporary stopped while copying
**         CursorX,CursorY,.. because between two copy commands another
**         event can occure, which let invalid mouse position be delivered.
*/
global void DrawAnyCursor(void)
{
    // Disable any previous drawn cursor
    OldCursorInvalidate=OldCursorW=
      OldCursorRectangleInvalidate=OldCursorRectangleW=
      BuildingCursor=0;

    //
    //	First, Selecting rectangle
    //
    if( CursorState==CursorStateRectangle
	     && (CursorStartX!=CursorX || CursorStartY!=CursorY) ) {
	DrawVisibleRectangleCursor(CursorStartX,CursorStartY,CursorX,CursorY);
    }
    //
    //	Or Selecting position for building
    //
    else if( CursorBuilding && CursorOn==CursorOnMap ) {
	DrawBuildingCursor();
	BuildingCursor=1;
    }

    //
    //	Last, Normal cursor.
    //  This will also save (part of) drawn rectangle cursor, but that's ok.
    //
    DrawCursor(GameCursor,CursorX,CursorY,0);
}

/**
**	Remove old cursor from display.
**      (in the opposite direction of DrawAnyCursor)
**      Note: this function can be called, without calling DrawAnyCursor first,
**            which means that cursors shouldn't be restored twice.
**            As cursors are, like DrawAnyCursor,  not invalidated here, it
**            still needs to be done by InvalidateCursorAreas.
*/
global void HideAnyCursor(void)
{
    //
    //	First, Normal cursor (might restore part of rectangle cursor also).
    //
    if( OldCursorW && OldCursorImage) {
        // restore area of visible cursor
        LoadCursorBackground(OldCursorX,OldCursorY,OldCursorW,OldCursorH);

        // save hidden area to be invalidated
        HiddenCursorX=OldCursorX;
        HiddenCursorY=OldCursorY;
        HiddenCursorW=OldCursorW;
        HiddenCursorH=OldCursorH;

        // Denote cursor no longer visible
        OldCursorW=0;
    }

    //
    //	Last, Selecting rectangle
    //
    if( OldCursorRectangleW ) {
        //  restore area of visible cursor
	LoadCursorRectangle(OldCursorRectangleX,OldCursorRectangleY,
			    OldCursorRectangleW,OldCursorRectangleH);

        // save hidden area to be invalidated
        HiddenCursorRectangleX=OldCursorRectangleX;
        HiddenCursorRectangleY=OldCursorRectangleY;
        HiddenCursorRectangleW=OldCursorRectangleW;
        HiddenCursorRectangleH=OldCursorRectangleH;

        // Denote cursor no longer visible
        OldCursorRectangleW=0;
    }
    //
    //	Or Selecting position for building
    //
    else if( BuildingCursor ) {
    //NOTE: this will restore tiles themselves later in next video update
	MarkDrawAreaMap(BuildingCursorSX,BuildingCursorSY,
		BuildingCursorEX,BuildingCursorEY);
        BuildingCursor=0;
    }
}

/**
**      Let an area be invalidated, but remembering if cursor is automaticly 
**      invalidated with this area.
**      Note: building-cursor is already invalidated by redraw-map
**
**	@param x	left-top x-position of area on screen
**	@param y	left-top y-position of area on screen
**	@param w	width of area on screen
**	@param h	height of area on screen
*/
global void InvalidateAreaAndCheckCursor( int x, int y, int w, int h )
{
  int dx,dy;

  //Invalidate area
  InvalidateArea(x,y,w,h);

  //Now check if cursor sprite is inside it, then no need for invalidate
  if ( OldCursorInvalidate )
  {
    dx = OldCursorX-x;
    dy = OldCursorY-y;
    if ( dx >= 0 && dy >= 0 && (w-dx) >= OldCursorW && (h-dy) >= OldCursorH )
      OldCursorInvalidate = 0;
  }

  //Now check if previously hidden cursor sprite is inside it..
  if ( HiddenCursorW )
  {
    dx = HiddenCursorX-x;
    dy = HiddenCursorY-y;
    if ( dx >= 0 && dy >= 0 &&
         (w-dx) >= HiddenCursorW && (h-dy) >= HiddenCursorH )
      HiddenCursorW = 0;
  }

  //Now check if cursor rectangle is inside it..
  if ( OldCursorRectangleInvalidate )
  {
    dx = OldCursorRectangleX-x;
    dy = OldCursorRectangleY-y;
    if ( dx >= 0 && dy >= 0 &&
         (w-dx) >= OldCursorRectangleW && (h-dy) >= OldCursorRectangleH )
      OldCursorRectangleInvalidate = 0;
  }

  //Now check if previously hidden cursor rectangle is inside it..
  if ( HiddenCursorRectangleW )
  {
    dx = HiddenCursorRectangleX-x;
    dy = HiddenCursorRectangleY-y;
    if ( dx >= 0 && dy >= 0 &&
         (w-dx) >= HiddenCursorRectangleW && (h-dy) >= HiddenCursorRectangleH )
      HiddenCursorRectangleW = 0;
  }
}

/**
**      Invalidate only the sides of a given rectangle (not its contents)
**
**	@param x	left-top x-position of rectangle on screen
**	@param y	left-top y-position of rectangle on screen
**	@param w	width of rectangle on screen
**	@param h	height of rectangle on screen
*/
local void InvalidateRectangle(int x, int y, int w, int h)
{
    InvalidateArea(x,y,w,1); // top side
    if ( --h > 0 )
    {
      InvalidateArea(x,y+h,w,1); // bottom side
      if ( --h > 0 )
      {
        InvalidateArea(x,++y,1,h); // left side
        if ( --w > 0 )
          InvalidateArea(x+w,y,1,h); // right side
      } 
    }
}

/**
**      Let the (remaining) areas taken by the cursors, as determined by
**      DrawAnyCursor and InvalidateAreaAndCheckcursor,  be invalidated.
**      Note: building-cursor is already invalidated by redraw-map
*/
global void InvalidateCursorAreas(void)
{
  //Invalidate cursor sprite
  if ( OldCursorInvalidate )
  {
    InvalidateArea(OldCursorX,OldCursorY,OldCursorW,OldCursorH);
    OldCursorInvalidate=0;
  }

  //Invalidate hidden cursor sprite
  if ( HiddenCursorW )
  {
    InvalidateArea(HiddenCursorX,HiddenCursorY,HiddenCursorW,HiddenCursorH);
    HiddenCursorW=0;
  }

  //Invalidate cursor rectangle
  if ( OldCursorRectangleInvalidate )
  {
    InvalidateRectangle(OldCursorRectangleX,OldCursorRectangleY,
                        OldCursorRectangleW,OldCursorRectangleH);
    OldCursorRectangleInvalidate=0;
  }

  //Invalidate hidden cursor rectangle
  if ( HiddenCursorRectangleW )
  {
    InvalidateRectangle(HiddenCursorRectangleX,HiddenCursorRectangleY,
                        HiddenCursorRectangleW,HiddenCursorRectangleH);
    HiddenCursorRectangleW=0;
  }
}

/**
**	Setup the cursor part.
**
**	@todo	FIXME: Now max possible memory for OldCursorRectangle,
**		to be limited to Map?
*/
global void InitCursors(void)
{
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

/**
**	Save cursor state.
*/
global void SaveCursors(FILE* file)
{
    int i;

    fprintf(file,"\n;;; -----------------------------------------\n");
    fprintf(file,";;; MODULE: cursors $Id$\n\n");

    for( i=0; Cursors[i].OType; ++i ) {
	fprintf(file,"(define-cursor '%s '%s\n",
		Cursors[i].Ident, Cursors[i].Race ? Cursors[i].Race : "any");
	fprintf(file,"  'image \"%s\"\n",Cursors[i].File);
	fprintf(file,"  'hot-spot '(%d %d) ",Cursors[i].HotX,Cursors[i].HotY);
	fprintf(file,"'size '(%d %d) ",Cursors[i].Width,Cursors[i].Height);
	fprintf(file,")\n\n");
    }

    // Not ready:
    fprintf(file,";;(game-cursor '%s)\n",GameCursor->Ident);
    // FIXME: what about the other variables???
    switch( CursorState ) {
	case CursorStatePoint:
	    fprintf(file,";;(cursor-state 'point)\n");
	    break;
	case CursorStateSelect:
	    fprintf(file,";;(cursor-state 'select)\n");
	    break;
	case CursorStateRectangle:
	    fprintf(file,";;(cursor-state 'rectangle)\n");
	    break;
    }
    fprintf(file,";;(cursor-action %d)\n",CursorAction);
    fprintf(file,";;(cursor-value %d)\n",CursorValue);
    fprintf(file,";;(cursor-building '%s)\n",
	    CursorBuilding ? CursorBuilding->Ident : "()");
    fprintf(file,";;(cursor-position '(%d %d)\n",CursorX,CursorY);
    fprintf(file,";;(cursor-start '(%d %d)\n",CursorStartX,CursorStartY);
}

/**
**	Cleanup cursor module
*/
global void CleanCursors(void)
{
    int i;

    for( i=0; Cursors[i].OType; ++i ) {
	free(Cursors[i].Ident);
	free(Cursors[i].Race);
	free(Cursors[i].File);
	VideoSaveFree(Cursors[i].Sprite);
    }
    free(Cursors);
    Cursors=NULL;

    free( OldCursorRectangle );
    OldCursorRectangle=0;

    DestroyCursorBackground();

    CursorBuilding=0;
    GameCursor=0;
}

//@}
