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
/**@name linedraw.c	-	The general linedraw functions. */
//
//	(c) Copyright 2000,2001 by Lutz Sammer
//
//	$Id$

//@{

/*----------------------------------------------------------------------------
--	Includes
----------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "freecraft.h"
#include "video.h"

/*----------------------------------------------------------------------------
--	Declarations
----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
--	Externals
----------------------------------------------------------------------------*/

extern int ClipX1;			/// current clipping top left
extern int ClipY1;			/// current clipping top left
extern int ClipX2;			/// current clipping bottom right
extern int ClipY2;			/// current clipping bottom right

/*----------------------------------------------------------------------------
--	Variables
----------------------------------------------------------------------------*/

    /**
    **	Draw pixel unclipped.
    **
    **	@param color	Color index.
    **	@param x	x coordinate on the screen
    **	@param y	y coordinate on the screen
    */
global void (*VideoDrawPixel)(SysColors color,int x,int y);

    /**
    **	Draw 25% translucent pixel (Alpha=64) unclipped.
    **
    **	@param color	Color index.
    **	@param x	x coordinate on the screen
    **	@param y	y coordinate on the screen
    */
global void (*VideoDraw25TransPixel)(SysColors color,int x,int y);

    /**
    **	Draw 50% translucent pixel (Alpha=128) unclipped.
    **
    **	@param color	Color index.
    **	@param x	x coordinate on the screen
    **	@param y	y coordinate on the screen
    */
global void (*VideoDraw50TransPixel)(SysColors color,int x,int y);

    /**
    **	Draw 75% translucent pixel (Alpha=192) unclipped.
    **
    **	@param color	Color index.
    **	@param x	x coordinate on the screen
    **	@param y	y coordinate on the screen
    */
global void (*VideoDraw75TransPixel)(SysColors color,int x,int y);

    /**
    **	Draw translucent pixel unclipped.
    **
    **	@param color	Color index.
    **	@param x	x coordinate on the screen
    **	@param y	y coordinate on the screen
    **	@param alpha	alpha value of pixel.
    */
global void (*VideoDrawTransPixel)(SysColors color,int x,int y,int alpha);

    /**
    **	Draw pixel clipped to current clip setting.
    **
    **	@param color	Color index.
    **	@param x	x coordinate on the screen
    **	@param y	y coordinate on the screen
    */
global void (*VideoDrawPixelClip)(SysColors color,int x,int y);

    /**
    **	Draw 25% translucent pixel clipped to current clip setting.
    **
    **	@param color	Color index.
    **	@param x	x coordinate on the screen
    **	@param y	y coordinate on the screen
    */
global void (*VideoDraw25TransPixelClip)(SysColors color,int x,int y);

    /**
    **	Draw 50% translucent pixel clipped to current clip setting.
    **
    **	@param color	Color index.
    **	@param x	x coordinate on the screen
    **	@param y	y coordinate on the screen
    */
global void (*VideoDraw50TransPixelClip)(SysColors color,int x,int y);

    /**
    **	Draw 75% translucent pixel clipped to current clip setting.
    **
    **	@param color	Color index.
    **	@param x	x coordinate on the screen
    **	@param y	y coordinate on the screen
    */
global void (*VideoDraw75TransPixelClip)(SysColors color,int x,int y);

    /**
    **	Draw translucent pixel clipped to current clip setting.
    **
    **	@param color	Color index.
    **	@param x	x coordinate on the screen
    **	@param y	y coordinate on the screen
    **	@param alpha	alpha value of pixel.
    */
global void (*VideoDrawTransPixelClip)(SysColors color,int x,int y,int alpha);

    /**
    **	Draw vertical line unclipped.
    **
    **	@param color	Color index.
    **	@param x	x coordinate on the screen
    **	@param y	y coordinate on the screen
    **	@param height	height of line.
    */
global void (*VideoDrawVLine)(SysColors color,int x,int y
	,unsigned height);

    /**
    **	Draw 25% translucent vertical line unclipped.
    **
    **	@param color	Color index.
    **	@param x	x coordinate on the screen
    **	@param y	y coordinate on the screen
    **	@param height	height of line.
    */
global void (*VideoDraw25TransVLine)(SysColors color,int x,int y
	,unsigned height);

    /**
    **	Draw 50% translucent vertical line unclipped.
    **
    **	@param color	Color index.
    **	@param x	x coordinate on the screen
    **	@param y	y coordinate on the screen
    **	@param height	height of line.
    */
global void (*VideoDraw50TransVLine)(SysColors color,int x,int y
	,unsigned height);

    /**
    **	Draw 75% translucent vertical line unclipped.
    **
    **	@param color	Color index.
    **	@param x	x coordinate on the screen
    **	@param y	y coordinate on the screen
    **	@param height	height of line.
    */
global void (*VideoDraw75TransVLine)(SysColors color,int x,int y
	,unsigned height);

    /**
    **	Draw translucent vertical line unclipped.
    **
    **	@param color	Color index.
    **	@param x	x coordinate on the screen
    **	@param y	y coordinate on the screen
    **	@param height	height of line.
    **	@param alpha	alpha value of pixel.
    */
global void (*VideoDrawTransVLine)(SysColors color,int x,int y
	,unsigned height,int alpha);

    /**
    **	Draw vertical line clipped to current clip setting
    **
    **	@param color	Color index.
    **	@param x	x coordinate on the screen
    **	@param y	y coordinate on the screen
    **	@param height	height of line.
    */
global void (*VideoDrawVLineClip)(SysColors color,int x,int y
	,unsigned height);

    /**
    **	Draw 25% translucent vertical line clipped to current clip setting
    **
    **	@param color	Color index.
    **	@param x	x coordinate on the screen
    **	@param y	y coordinate on the screen
    **	@param height	height of line.
    */
global void (*VideoDraw25TransVLineClip)(SysColors color,int x,int y
	,unsigned height);

    /**
    **	Draw 50% translucent vertical line clipped to current clip setting
    **
    **	@param color	Color index.
    **	@param x	x coordinate on the screen
    **	@param y	y coordinate on the screen
    **	@param height	height of line.
    */
global void (*VideoDraw50TransVLineClip)(SysColors color,int x,int y
	,unsigned height);

    /**
    **	Draw 75% translucent vertical line clipped to current clip setting
    **
    **	@param color	Color index.
    **	@param x	x coordinate on the screen
    **	@param y	y coordinate on the screen
    **	@param height	height of line.
    */
global void (*VideoDraw75TransVLineClip)(SysColors color,int x,int y
	,unsigned height);

    /**
    **	Draw translucent vertical line clipped to current clip setting
    **
    **	@param color	Color index.
    **	@param x	x coordinate on the screen
    **	@param y	y coordinate on the screen
    **	@param height	height of line.
    **	@param alpha	alpha value of pixel.
    */
global void (*VideoDrawTransVLineClip)(SysColors color,int x,int y
	,unsigned height,int alpha);

    /**
    **	Draw horizontal line unclipped.
    **
    **	@param color	Color index.
    **	@param x	x coordinate on the screen
    **	@param y	y coordinate on the screen
    **	@param width	width of line.
    */
global void (*VideoDrawHLine)(SysColors color,int x,int y
	,unsigned width);

    /**
    **	Draw 25% translucent horizontal line unclipped.
    **
    **	@param color	Color index.
    **	@param x	x coordinate on the screen
    **	@param y	y coordinate on the screen
    **	@param width	width of line.
    */
global void (*VideoDraw25TransHLine)(SysColors color,int x,int y
	,unsigned width);

    /**
    **	Draw 50% translucent horizontal line unclipped.
    **
    **	@param color	Color index.
    **	@param x	x coordinate on the screen
    **	@param y	y coordinate on the screen
    **	@param width	width of line.
    */
global void (*VideoDraw50TransHLine)(SysColors color,int x,int y
	,unsigned width);

    /**
    **	Draw 75% translucent horizontal line unclipped.
    **
    **	@param color	Color index.
    **	@param x	x coordinate on the screen
    **	@param y	y coordinate on the screen
    **	@param width	width of line.
    */
global void (*VideoDraw75TransHLine)(SysColors color,int x,int y
	,unsigned width);

    /**
    **	Draw translucent horizontal line unclipped.
    **
    **	@param color	Color index.
    **	@param x	x coordinate on the screen
    **	@param y	y coordinate on the screen
    **	@param width	width of line.
    **	@param alpha	alpha value of pixel.
    */
global void (*VideoDrawTransHLine)(SysColors color,int x,int y
	,unsigned width,int alpha);

    /**
    **	Draw horizontal line clipped to current clip setting
    **
    **	@param color	Color index.
    **	@param x	x coordinate on the screen
    **	@param y	y coordinate on the screen
    **	@param width	width of line.
    */
global void (*VideoDrawHLineClip)(SysColors color,int x,int y
	,unsigned width);

    /**
    **	Draw 25% translucent horizontal line clipped to current clip setting
    **
    **	@param color	Color index.
    **	@param x	x coordinate on the screen
    **	@param y	y coordinate on the screen
    **	@param width	width of line.
    */
global void (*VideoDraw25TransHLineClip)(SysColors color,int x,int y
	,unsigned width);

    /**
    **	Draw 50% translucent horizontal line clipped to current clip setting
    **
    **	@param color	Color index.
    **	@param x	x coordinate on the screen
    **	@param y	y coordinate on the screen
    **	@param width	width of line.
    */
global void (*VideoDraw50TransHLineClip)(SysColors color,int x,int y
	,unsigned width);

    /**
    **	Draw 75% translucent horizontal line clipped to current clip setting
    **
    **	@param color	Color index.
    **	@param x	x coordinate on the screen
    **	@param y	y coordinate on the screen
    **	@param width	width of line.
    */
global void (*VideoDraw75TransHLineClip)(SysColors color,int x,int y
	,unsigned width);

    /**
    **	Draw translucent horizontal line clipped to current clip setting
    **
    **	@param color	Color index.
    **	@param x	x coordinate on the screen
    **	@param y	y coordinate on the screen
    **	@param width	width of line.
    **	@param alpha	alpha value of pixel.
    */
global void (*VideoDrawTransHLineClip)(SysColors color,int x,int y
	,unsigned width,int alpha);

    /**
    **	Draw line unclipped.
    **
    **	@param color	Color index.
    **	@param sx	Source x coordinate on the screen
    **	@param sy	Source y coordinate on the screen
    **	@param dx	Destination x coordinate on the screen
    **	@param dy	Destination y coordinate on the screen
    */
global void (*VideoDrawLine)(SysColors color,int sx,int sy,int dx,int dy);

    /**
    **	Draw 25% translucent line unclipped.
    **
    **	@param color	Color index.
    **	@param sx	Source x coordinate on the screen
    **	@param sy	Source y coordinate on the screen
    **	@param dx	Destination x coordinate on the screen
    **	@param dy	Destination y coordinate on the screen
    */
global void (*VideoDraw25TransLine)(SysColors color,int sx,int sy
	,int dx,int dy);

    /**
    **	Draw 50% translucent line unclipped.
    **
    **	@param color	Color index.
    **	@param sx	Source x coordinate on the screen
    **	@param sy	Source y coordinate on the screen
    **	@param dx	Destination x coordinate on the screen
    **	@param dy	Destination y coordinate on the screen
    */
global void (*VideoDraw50TransLine)(SysColors color,int sx,int sy
	,int dx,int dy);

    /**
    **	Draw 75% translucent line unclipped.
    **
    **	@param color	Color index.
    **	@param sx	Source x coordinate on the screen
    **	@param sy	Source y coordinate on the screen
    **	@param dx	Destination x coordinate on the screen
    **	@param dy	Destination y coordinate on the screen
    */
global void (*VideoDraw75TransLine)(SysColors color,int sx,int sy
	,int dx,int dy);

    /**
    **	Draw translucent line unclipped.
    **
    **	@param color	Color index.
    **	@param sx	Source x coordinate on the screen
    **	@param sy	Source y coordinate on the screen
    **	@param dx	Destination x coordinate on the screen
    **	@param dy	Destination y coordinate on the screen
    **	@param alpha	alpha value of pixel.
    */
global void (*VideoDrawTransLine)(SysColors color,int sx,int sy,int dx,int dy
	,int alpha);

    /**
    **	Draw line clipped to current clip setting
    **
    **	@param color	Color index.
    **	@param sx	Source x coordinate on the screen
    **	@param sy	Source y coordinate on the screen
    **	@param dx	Destination x coordinate on the screen
    **	@param dy	Destination y coordinate on the screen
    */
global void (*VideoDrawLineClip)(SysColors color,int sx,int sy,int dx,int dy);

    /**
    **	Draw 25% translucent line clipped to current clip setting
    **
    **	@param color	Color index.
    **	@param sx	Source x coordinate on the screen
    **	@param sy	Source y coordinate on the screen
    **	@param dx	Destination x coordinate on the screen
    **	@param dy	Destination y coordinate on the screen
    */
global void (*VideoDraw25TransLineClip)(SysColors color,int sx,int sy
	,int dx,int dy);

    /**
    **	Draw 50% translucent line clipped to current clip setting
    **
    **	@param color	Color index.
    **	@param sx	Source x coordinate on the screen
    **	@param sy	Source y coordinate on the screen
    **	@param dx	Destination x coordinate on the screen
    **	@param dy	Destination y coordinate on the screen
    */
global void (*VideoDraw50TransLineClip)(SysColors color,int sx,int sy,
	int dx,int dy);

    /**
    **	Draw 75% translucent line clipped to current clip setting
    **
    **	@param color	Color index.
    **	@param sx	Source x coordinate on the screen
    **	@param sy	Source y coordinate on the screen
    **	@param dx	Destination x coordinate on the screen
    **	@param dy	Destination y coordinate on the screen
    */
global void (*VideoDraw75TransLineClip)(SysColors color,int sx,int sy
	,int dx,int dy);

    /**
    **	Draw translucent line clipped to current clip setting
    **
    **	@param color	Color index.
    **	@param sx	Source x coordinate on the screen
    **	@param sy	Source y coordinate on the screen
    **	@param dx	Destination x coordinate on the screen
    **	@param dy	Destination y coordinate on the screen
    **	@param alpha	alpha value of pixel.
    */
global void (*VideoDrawTransLineClip)(SysColors color,int sx,int sy
	,int dx,int dy,int alpha);

    /**
    **	Draw rectangle.
    **
    **	@param color	Color index.
    **	@param x	x coordinate on the screen
    **	@param y	y coordinate on the screen
    **	@param h	height of rectangle.
    **	@param w	width of rectangle.
    */
global void (*VideoDrawRectangle)(SysColors color,int x,int y
	,unsigned w,unsigned h);

    /**
    **	Draw 25% translucent rectangle.
    **
    **	@param color	Color index.
    **	@param x	x coordinate on the screen
    **	@param y	y coordinate on the screen
    **	@param h	height of rectangle.
    **	@param w	width of rectangle.
    */
global void (*VideoDraw25TransRectangle)(SysColors color,int x,int y
	,unsigned w,unsigned h);

    /**
    **	Draw 50% translucent rectangle.
    **
    **	@param color	Color index.
    **	@param x	x coordinate on the screen
    **	@param y	y coordinate on the screen
    **	@param h	height of rectangle.
    **	@param w	width of rectangle.
    */
global void (*VideoDraw50TransRectangle)(SysColors color,int x,int y
	,unsigned w,unsigned h);

    /**
    **	Draw 75% translucent rectangle.
    **
    **	@param color	Color index.
    **	@param x	x coordinate on the screen
    **	@param y	y coordinate on the screen
    **	@param h	height of rectangle.
    **	@param w	width of rectangle.
    */
global void (*VideoDraw75TransRectangle)(SysColors color,int x,int y
	,unsigned w,unsigned h);

    /**
    **	Draw translucent rectangle.
    **
    **	@param color	Color index.
    **	@param x	x coordinate on the screen
    **	@param y	y coordinate on the screen
    **	@param h	height of rectangle.
    **	@param w	width of rectangle.
    **	@param alpha	alpha value of pixel.
    */
global void (*VideoDrawTransRectangle)(SysColors color,int x,int y
	,unsigned w,unsigned h,int alpha);

    /**
    **	Draw rectangle clipped.
    **
    **	@param color	Color index.
    **	@param x	x coordinate on the screen
    **	@param y	y coordinate on the screen
    **	@param h	height of rectangle.
    **	@param w	width of rectangle.
    */
global void (*VideoDrawRectangleClip)(SysColors color,int x,int y
	,unsigned w,unsigned h);

    /**
    **	Draw 25% translucent rectangle clipped.
    **
    **	@param color	Color index.
    **	@param x	x coordinate on the screen
    **	@param y	y coordinate on the screen
    **	@param h	height of rectangle.
    **	@param w	width of rectangle.
    */
global void (*VideoDraw25TransRectangleClip)(SysColors color,int x,int y
	,unsigned w,unsigned h);

    /**
    **	Draw 50% translucent rectangle clipped.
    **
    **	@param color	Color index.
    **	@param x	x coordinate on the screen
    **	@param y	y coordinate on the screen
    **	@param h	height of rectangle.
    **	@param w	width of rectangle.
    */
global void (*VideoDraw50TransRectangleClip)(SysColors color,int x,int y
	,unsigned w,unsigned h);

    /**
    **	Draw 75% translucent rectangle clipped.
    **
    **	@param color	Color index.
    **	@param x	x coordinate on the screen
    **	@param y	y coordinate on the screen
    **	@param h	height of rectangle.
    **	@param w	width of rectangle.
    */
global void (*VideoDraw75TransRectangleClip)(SysColors color,int x,int y
	,unsigned w,unsigned h);

    /**
    **	Draw translucent rectangle clipped.
    **
    **	@param color	Color index.
    **	@param x	x coordinate on the screen
    **	@param y	y coordinate on the screen
    **	@param h	height of rectangle.
    **	@param w	width of rectangle.
    **	@param alpha	alpha value of pixel.
    */
global void (*VideoDrawTransRectangleClip)(SysColors color,int x,int y
	,unsigned w,unsigned h,int alpha);

/*----------------------------------------------------------------------------
--	Local functions
----------------------------------------------------------------------------*/

// ===========================================================================
//	Pixel
// ===========================================================================

/**
**	Draw pixel unclipped into 8bit framebuffer.
**
**	@param color	Color index.
**	@param x	x coordinate on the screen
**	@param y	y coordinate on the screen
*/
local void DrawPixel8(SysColors color,int x,int y)
{
    VideoMemory8[x+y*VideoWidth]=Pixels8[color];
}

/**
**	Draw pixel unclipped into 16bit framebuffer.
**
**	@param color	Color index.
**	@param x	x coordinate on the screen
**	@param y	y coordinate on the screen
*/
local void DrawPixel16(SysColors color,int x,int y)
{
    VideoMemory16[x+y*VideoWidth]=Pixels16[color];
}

/**
**	Draw pixel unclipped into 24bit framebuffer.
**
**	@param color	Color index.
**	@param x	x coordinate on the screen
**	@param y	y coordinate on the screen
*/
local void DrawPixel24(SysColors color,int x,int y)
{
    VideoMemory24[x+y*VideoWidth]=Pixels24[color];
}

/**
**	Draw pixel unclipped into 32bit framebuffer.
**
**	@param color	Color index.
**	@param x	x coordinate on the screen
**	@param y	y coordinate on the screen
*/
local void DrawPixel32(SysColors color,int x,int y)
{
    VideoMemory32[x+y*VideoWidth]=Pixels32[color];
}

/**
**	Draw pixel clipped to current clip setting into 8bit framebuffer.
**
**	@param color	Color index.
**	@param x	x coordinate on the screen
**	@param y	y coordinate on the screen
*/
local void DrawPixelClip8(SysColors color,int x,int y)
{
    //	Clipping:
    if( x<ClipX1 || x>=ClipX2 || y<ClipY1 || y>=ClipY2 ) {
	return;
    }
    VideoMemory8[x+y*VideoWidth]=Pixels8[color];
}

/**
**	Draw pixel clipped to current clip setting into 16bit framebuffer.
**
**	@param color	Color index.
**	@param x	x coordinate on the screen
**	@param y	y coordinate on the screen
*/
local void DrawPixelClip16(SysColors color,int x,int y)
{
    //	Clipping:
    if( x<ClipX1 || x>=ClipX2 || y<ClipY1 || y>=ClipY2 ) {
	return;
    }
    VideoMemory16[x+y*VideoWidth]=Pixels16[color];
}

/**
**	Draw pixel clipped to current clip setting into 24bit framebuffer.
**
**	@param color	Color index.
**	@param x	x coordinate on the screen
**	@param y	y coordinate on the screen
*/
local void DrawPixelClip24(SysColors color,int x,int y)
{
    //	Clipping:
    if( x<ClipX1 || x>=ClipX2 || y<ClipY1 || y>=ClipY2 ) {
	return;
    }
    VideoMemory24[x+y*VideoWidth]=Pixels24[color];
}

/**
**	Draw pixel clipped to current clip setting into 32bit framebuffer.
**
**	@param color	Color index.
**	@param x	x coordinate on the screen
**	@param y	y coordinate on the screen
*/
local void DrawPixelClip32(SysColors color,int x,int y)
{
    //	Clipping:
    if( x<ClipX1 || x>=ClipX2 || y<ClipY1 || y>=ClipY2 ) {
	return;
    }
    VideoMemory32[x+y*VideoWidth]=Pixels32[color];
}

/**
**	Draw horizontal line unclipped into 8bit framebuffer.
**
**	@param color	Color index.
**	@param x	x coordinate on the screen
**	@param y	y coordinate on the screen
**	@param width	width of line.
*/
local void DrawHLine8(SysColors color,int x,int y,unsigned width)
{
    VMemType8* p;
    VMemType8* e;
    int w;
    unsigned f;

    w=VideoWidth;
    p=VideoMemory8+y*w+x;
    e=p+width;
    f=Pixels8[color];

    while( p<e ) {			// FIXME: better!
	*p++=f;
    }
}

// ===========================================================================
//	Horizontal Line
// ===========================================================================

/**
**	Draw horizontal line unclipped into 16bit framebuffer.
**
**	@param color	Color index.
**	@param x	x coordinate on the screen
**	@param y	y coordinate on the screen
**	@param width	width of line.
*/
local void DrawHLine16(SysColors color,int x,int y,unsigned width)
{
    VMemType16* p;
    VMemType16* e;
    int w;
    unsigned long f;

    w=VideoWidth;
    p=VideoMemory16+y*w+x;
    e=p+width-1;
    f=((unsigned long)Pixels16[color]<<16)|Pixels16[color];

    while( p<e ) {			// draw 2 pixels
	*((unsigned long*)p)++=f;
    }

    if( p<=e ) {
	*p=f;
    }
}

/**
**	Draw horizontal line unclipped into 24bit framebuffer.
**
**	@param color	Color index.
**	@param x	x coordinate on the screen
**	@param y	y coordinate on the screen
**	@param width	width of line.
*/
local void DrawHLine24(SysColors color,int x,int y,unsigned width)
{
    VMemType24* p;
    VMemType24* e;
    int w;
    VMemType24 f;

    w=VideoWidth;
    p=VideoMemory24+y*w+x;
    e=p+width;
    f=Pixels24[color];

    while( p<e ) {
	*p++=f;
    }
}

/**
**	Draw horizontal line unclipped into 32bit framebuffer.
**
**	@param color	Color index.
**	@param x	x coordinate on the screen
**	@param y	y coordinate on the screen
**	@param width	width of line.
*/
local void DrawHLine32(SysColors color,int x,int y,unsigned width)
{
    VMemType32* p;
    VMemType32* e;
    int w;
    unsigned long f;

    w=VideoWidth;
    p=VideoMemory32+y*w+x;
    e=p+width;
    f=Pixels32[color];

    while( p<e ) {
	*p++=f;
    }
}

/**
**	Draw 25% translucent horizontal line unclipped into 15bit framebuffer.
**
**	@param color	Color index.
**	@param x	x coordinate on the screen
**	@param y	y coordinate on the screen
**	@param width	width of line.
*/
local void Draw25TransHLine15(SysColors color,int x,int y,unsigned width)
{
    VMemType16* p;
    VMemType16* e;
    int w;
    unsigned long sp;

    w=VideoWidth;
    p=VideoMemory16+y*w+x;
    e=p+width;
    sp=Pixels16[color];
    sp=(((sp<<16)|sp)&0x03E07C1F)*3;

    while( p<e ) {
	unsigned dp;

	dp=*p;
	dp=((dp<<16)|dp)&0x03E07C1F;
	dp=((dp+sp)>>2)&0x03E07C1F;
	*p++=(dp>>16)|dp;
    }
}

/**
**	Draw 25% translucent horizontal line unclipped into 16bit framebuffer.
**
**	@param color	Color index.
**	@param x	x coordinate on the screen
**	@param y	y coordinate on the screen
**	@param width	width of line.
*/
local void Draw25TransHLine16(SysColors color,int x,int y,unsigned width)
{
    VMemType16* p;
    VMemType16* e;
    int w;
    unsigned long sp;

    w=VideoWidth;
    p=VideoMemory16+y*w+x;
    e=p+width;
    sp=Pixels16[color];
    sp=(((sp<<16)|sp)&0x07E0F81F)*3;

    while( p<e ) {
	unsigned dp;

	dp=*p;
	dp=((dp<<16)|dp)&0x07E0F81F;
	dp=((dp+sp)>>1)&0x07E0F81F;
	*p++=(dp>>16)|dp;
    }
}

/**
**	Draw 50% translucent horizontal line unclipped into 15bit framebuffer.
**
**	@param color	Color index.
**	@param x	x coordinate on the screen
**	@param y	y coordinate on the screen
**	@param width	width of line.
*/
local void Draw50TransHLine15(SysColors color,int x,int y,unsigned width)
{
    VMemType16* p;
    VMemType16* e;
    int w;
    unsigned long sp;

    w=VideoWidth;
    p=VideoMemory16+y*w+x;
    e=p+width;
    sp=Pixels16[color];
    sp=((sp<<16)|sp)&0x03E07C1F;

    while( p<e ) {
	unsigned dp;

	dp=*p;
	dp=((dp<<16)|dp)&0x03E07C1F;
	dp=((dp+sp)>>1)&0x03E07C1F;
	*p++=(dp>>16)|dp;
    }
}

/**
**	Draw 50% translucent horizontal line unclipped into 16bit framebuffer.
**
**	@param color	Color index.
**	@param x	x coordinate on the screen
**	@param y	y coordinate on the screen
**	@param width	width of line.
*/
local void Draw50TransHLine16(SysColors color,int x,int y,unsigned width)
{
    VMemType16* p;
    VMemType16* e;
    int w;
    unsigned long sp;

    w=VideoWidth;
    p=VideoMemory16+y*w+x;
    e=p+width;
    sp=Pixels16[color];
    sp=((sp<<16)|sp)&0x07E0F81F;

    while( p<e ) {
	unsigned dp;

	dp=*p;
	dp=((dp<<16)|dp)&0x07E0F81F;
	dp=((dp+sp)>>1)&0x07E0F81F;
	*p++=(dp>>16)|dp;
    }
}

/**
**	Draw 75% translucent horizontal line unclipped into 15bit framebuffer.
**
**	@param color	Color index.
**	@param x	x coordinate on the screen
**	@param y	y coordinate on the screen
**	@param width	width of line.
*/
local void Draw75TransHLine15(SysColors color,int x,int y,unsigned width)
{
    VMemType16* p;
    VMemType16* e;
    int w;
    unsigned long sp;

    w=VideoWidth;
    p=VideoMemory16+y*w+x;
    e=p+width;
    sp=Pixels16[color];
    sp=((sp<<16)|sp)&0x03E07C1F;

    while( p<e ) {
	unsigned dp;

	dp=*p;
	dp=((dp<<16)|dp)&0x03E07C1F;
	dp=(((dp<<1)+dp+sp)>>2)&0x03E07C1F;
	*p++=(dp>>16)|dp;
    }
}

/**
**	Draw 75% translucent horizontal line unclipped into 16bit framebuffer.
**
**	@param color	Color index.
**	@param x	x coordinate on the screen
**	@param y	y coordinate on the screen
**	@param width	width of line.
*/
local void Draw75TransHLine16(SysColors color,int x,int y,unsigned width)
{
    VMemType16* p;
    VMemType16* e;
    int w;
    unsigned long sp;

    w=VideoWidth;
    p=VideoMemory16+y*w+x;
    e=p+width;
    sp=Pixels16[color];
    sp=((sp<<16)|sp)&0x07E0F81F;

    while( p<e ) {
	unsigned dp;

	dp=*p;
	dp=((dp<<16)|dp)&0x07E0F81F;
	dp=(((dp<<1)+dp+sp)>>2)&0x07E0F81F;
	*p++=(dp>>16)|dp;
    }
}

/**
**	Draw translucent horizontal line unclipped into 15bit framebuffer.
**
**	@param color	Color index.
**	@param x	x coordinate on the screen
**	@param y	y coordinate on the screen
**	@param width	width of line.
**	@param alpha	alpha value of pixels.
*/
local void DrawTransHLine15(SysColors color,int x,int y,unsigned width
	,int alpha)
{
    VMemType16* p;
    VMemType16* e;
    int w;
    unsigned long sp;

    w=VideoWidth;
    p=VideoMemory16+y*w+x;
    e=p+width;
    sp=Pixels16[color];
    // FIXME: pre multiply?
    sp=((sp<<16)|sp)&0x03E07C1F;
    alpha>>=3;				// only 5bits

    while( p<e ) {
	unsigned dp;

	dp=*p;
	dp=((dp<<16)|dp)&0x03E07C1F;
	dp=((((dp-sp)*alpha)>>5)+sp)&0x03E07C1F;
	*p++=(dp>>16)|dp;
    }
}

/**
**	Draw translucent horizontal line unclipped into 16bit framebuffer.
**
**	@param color	Color index.
**	@param x	x coordinate on the screen
**	@param y	y coordinate on the screen
**	@param width	width of line.
**	@param alpha	alpha value of pixels.
*/
local void DrawTransHLine16(SysColors color,int x,int y,unsigned width
	,int alpha)
{
    VMemType16* p;
    VMemType16* e;
    int w;
    unsigned long sp;

    w=VideoWidth;
    p=VideoMemory16+y*w+x;
    e=p+width;
    sp=Pixels16[color];
    // FIXME: pre multiply?
    sp=((sp<<16)|sp)&0x07E0F81F;
    alpha>>=3;				// only 5bits

    while( p<e ) {
	unsigned dp;

	dp=*p;
	dp=((dp<<16)|dp)&0x07E0F81F;
	dp=((((dp-sp)*alpha)>>5)+sp)&0x07E0F81F;
	*p++=(dp>>16)|dp;
    }
}

/**
**	Draw horizontal line clipped into 8bit framebuffer.
**
**	@param color	Color index.
**	@param x	x coordinate on the screen
**	@param y	y coordinate on the screen
**	@param width	width of line.
*/
local void DrawHLineClip8(SysColors color,int x,int y,unsigned width)
{
    int f;

    if( y<ClipY1 || y>=ClipY2 ) {	//	Clipping:
	return;
    }
    if( x<ClipX1 ) {
	f=ClipX1-x;
	x=ClipX1;
	if( width<f ) {
	    return;
	}
	width-=f;
    }
    if( (x+width)>ClipX2 ) {
	if( width<ClipX2-x ) {
	    return;
	}
	width=ClipX2-x;
    }

    DrawHLine8(color,x,y,width);
}

/**
**	Draw horizontal line clipped into 16bit framebuffer.
**
**	@param color	Color index.
**	@param x	x coordinate on the screen
**	@param y	y coordinate on the screen
**	@param width	width of line.
*/
local void DrawHLineClip16(SysColors color,int x,int y,unsigned width)
{
    int f;

    if( y<ClipY1 || y>=ClipY2 ) {	//	Clipping:
	return;
    }
    if( x<ClipX1 ) {
	f=ClipX1-x;
	x=ClipX1;
	if( width<f ) {
	    return;
	}
	width-=f;
    }
    if( (x+width)>ClipX2 ) {
	if( width<ClipX2-x ) {
	    return;
	}
	width=ClipX2-x;
    }

    DrawHLine16(color,x,y,width);
}

/**
**	Draw horizontal line clipped into 24bit framebuffer.
**
**	@param color	Color index.
**	@param x	x coordinate on the screen
**	@param y	y coordinate on the screen
**	@param width	width of line.
*/
local void DrawHLineClip24(SysColors color,int x,int y,unsigned width)
{
    int f;

    if( y<ClipY1 || y>=ClipY2 ) {	//	Clipping:
	return;
    }
    if( x<ClipX1 ) {
	f=ClipX1-x;
	x=ClipX1;
	if( width<f ) {
	    return;
	}
	width-=f;
    }
    if( (x+width)>ClipX2 ) {
	if( width<ClipX2-x ) {
	    return;
	}
	width=ClipX2-x;
    }

    DrawHLine24(color,x,y,width);
}

/**
**	Draw horizontal line clipped into 32bit framebuffer.
**
**	@param color	Color index.
**	@param x	x coordinate on the screen
**	@param y	y coordinate on the screen
**	@param width	width of line.
*/
local void DrawHLineClip32(SysColors color,int x,int y,unsigned width)
{
    int f;

    if( y<ClipY1 || y>=ClipY2 ) {	//	Clipping:
	return;
    }
    if( x<ClipX1 ) {
	f=ClipX1-x;
	x=ClipX1;
	if( width<f ) {
	    return;
	}
	width-=f;
    }
    if( (x+width)>ClipX2 ) {
	if( width<ClipX2-x ) {
	    return;
	}
	width=ClipX2-x;
    }

    DrawHLine32(color,x,y,width);
}

/**
**	Draw 25% translucent horizontal line clipped into 15bit framebuffer.
**
**	@param color	Color index.
**	@param x	x coordinate on the screen
**	@param y	y coordinate on the screen
**	@param width	width of line.
*/
local void Draw25TransHLineClip15(SysColors color,int x,int y,unsigned width)
{
    int f;

    if( y<ClipY1 || y>=ClipY2 ) {	//	Clipping:
	return;
    }
    if( x<ClipX1 ) {
	f=ClipX1-x;
	x=ClipX1;
	if( width<f ) {
	    return;
	}
	width-=f;
    }
    if( (x+width)>ClipX2 ) {
	if( width<ClipX2-x ) {
	    return;
	}
	width=ClipX2-x;
    }

    Draw25TransHLine15(color,x,y,width);
}

/**
**	Draw 25% translucent horizontal line clipped into 16bit framebuffer.
**
**	@param color	Color index.
**	@param x	x coordinate on the screen
**	@param y	y coordinate on the screen
**	@param width	width of line.
*/
local void Draw25TransHLineClip16(SysColors color,int x,int y,unsigned width)
{
    int f;

    if( y<ClipY1 || y>=ClipY2 ) {	//	Clipping:
	return;
    }
    if( x<ClipX1 ) {
	f=ClipX1-x;
	x=ClipX1;
	if( width<f ) {
	    return;
	}
	width-=f;
    }
    if( (x+width)>ClipX2 ) {
	if( width<ClipX2-x ) {
	    return;
	}
	width=ClipX2-x;
    }

    Draw25TransHLine16(color,x,y,width);
}

/**
**	Draw 50% translucent horizontal line clipped into 15bit framebuffer.
**
**	@param color	Color index.
**	@param x	x coordinate on the screen
**	@param y	y coordinate on the screen
**	@param width	width of line.
*/
local void Draw50TransHLineClip15(SysColors color,int x,int y,unsigned width)
{
    int f;

    if( y<ClipY1 || y>=ClipY2 ) {	//	Clipping:
	return;
    }
    if( x<ClipX1 ) {
	f=ClipX1-x;
	x=ClipX1;
	if( width<f ) {
	    return;
	}
	width-=f;
    }
    if( (x+width)>ClipX2 ) {
	if( width<ClipX2-x ) {
	    return;
	}
	width=ClipX2-x;
    }

    Draw50TransHLine15(color,x,y,width);
}

/**
**	Draw 50% translucent horizontal line clipped into 16bit framebuffer.
**
**	@param color	Color index.
**	@param x	x coordinate on the screen
**	@param y	y coordinate on the screen
**	@param width	width of line.
*/
local void Draw50TransHLineClip16(SysColors color,int x,int y,unsigned width)
{
    int f;

    if( y<ClipY1 || y>=ClipY2 ) {	//	Clipping:
	return;
    }
    if( x<ClipX1 ) {
	f=ClipX1-x;
	x=ClipX1;
	if( width<f ) {
	    return;
	}
	width-=f;
    }
    if( (x+width)>ClipX2 ) {
	if( width<ClipX2-x ) {
	    return;
	}
	width=ClipX2-x;
    }

    Draw50TransHLine16(color,x,y,width);
}

/**
**	Draw 75% translucent horizontal line clipped into 15bit framebuffer.
**
**	@param color	Color index.
**	@param x	x coordinate on the screen
**	@param y	y coordinate on the screen
**	@param width	width of line.
*/
local void Draw75TransHLineClip15(SysColors color,int x,int y,unsigned width)
{
    int f;

    if( y<ClipY1 || y>=ClipY2 ) {	//	Clipping:
	return;
    }
    if( x<ClipX1 ) {
	f=ClipX1-x;
	x=ClipX1;
	if( width<f ) {
	    return;
	}
	width-=f;
    }
    if( (x+width)>ClipX2 ) {
	if( width<ClipX2-x ) {
	    return;
	}
	width=ClipX2-x;
    }

    Draw75TransHLine15(color,x,y,width);
}

/**
**	Draw 75% translucent horizontal line clipped into 16bit framebuffer.
**
**	@param color	Color index.
**	@param x	x coordinate on the screen
**	@param y	y coordinate on the screen
**	@param width	width of line.
*/
local void Draw75TransHLineClip16(SysColors color,int x,int y,unsigned width)
{
    int f;

    if( y<ClipY1 || y>=ClipY2 ) {	//	Clipping:
	return;
    }
    if( x<ClipX1 ) {
	f=ClipX1-x;
	x=ClipX1;
	if( width<f ) {
	    return;
	}
	width-=f;
    }
    if( (x+width)>ClipX2 ) {
	if( width<ClipX2-x ) {
	    return;
	}
	width=ClipX2-x;
    }

    Draw75TransHLine16(color,x,y,width);
}

// ===========================================================================
//	Vertical Line
// ===========================================================================

/**
**	Draw vertical line unclipped into 8bit framebuffer.
**
**	@param color	Color index.
**	@param x	x coordinate on the screen
**	@param y	y coordinate on the screen
**	@param height	height of line.
*/
local void DrawVLine8(SysColors color,int x,int y,unsigned height)
{
    VMemType8* p;
    VMemType8* e;
    int w;
    int f;

    w=VideoWidth;
    p=VideoMemory8+y*w+x;
    e=p+height*w;
    f=Pixels8[color];
    while( p<e ) {			// FIXME: better
	*p=f;
	p+=w;
    }
}

/**
**	Draw vertical line unclipped into 16bit framebuffer.
**
**	@param color	Color index.
**	@param x	x coordinate on the screen
**	@param y	y coordinate on the screen
**	@param height	height of line.
*/
local void DrawVLine16(SysColors color,int x,int y,unsigned height)
{
    VMemType16* p;
    VMemType16* e;
    int w;
    int f;

    w=VideoWidth;
    p=VideoMemory16+y*w+x;
    e=p+height*w;
    f=Pixels16[color];
    while( p<e ) {			// FIXME: better
	*p=f;
	p+=w;
    }
}

/**
**	Draw vertical line unclipped into 24bit framebuffer.
**
**	@param color	Color index.
**	@param x	x coordinate on the screen
**	@param y	y coordinate on the screen
**	@param height	height of line.
*/
local void DrawVLine24(SysColors color,int x,int y,unsigned height)
{
    VMemType24* p;
    VMemType24* e;
    int w;
    VMemType24 f;

    w=VideoWidth;
    p=VideoMemory24+y*w+x;
    e=p+height*w;
    f=Pixels24[color];
    while( p<e ) {			// FIXME: better
	*p=f;
	p+=w;
    }
}

/**
**	Draw vertical line unclipped into 32bit framebuffer.
**
**	@param color	Color index.
**	@param x	x coordinate on the screen
**	@param y	y coordinate on the screen
**	@param height	height of line.
*/
local void DrawVLine32(SysColors color,int x,int y,unsigned height)
{
    VMemType32* p;
    VMemType32* e;
    int w;
    int f;

    w=VideoWidth;
    p=VideoMemory32+y*w+x;
    e=p+height*w;
    f=Pixels32[color];
    while( p<e ) {			// FIXME: better
	*p=f;
	p+=w;
    }
}

/**
**	Draw vertical line clipped into 8bit framebuffer.
**
**	@param color	Color index.
**	@param x	x coordinate on the screen
**	@param y	y coordinate on the screen
**	@param height	height of line.
*/
local void DrawVLineClip8(SysColors color,int x,int y,unsigned height)
{
    VMemType8* p;
    VMemType8* e;
    int w;
    int f;

    //	Clipping:
    if( x<ClipX1 || x>=ClipX2 ) {
	return;
    }
    if( y<ClipY1 ) {
	f=ClipY1-y;
	y=ClipY1;
	if( height<f ) {
	    return;
	}
	height-=f;
    }
    if( (y+height)>ClipY2 ) {
	if( height<ClipY2-y ) {
	    return;
	}
	height=ClipY2-y;
    }

    w=VideoWidth;
    p=VideoMemory8+y*w+x;
    e=p+height*w;
    f=Pixels8[color];
    while( p<e ) {
	*p=f;
	p+=w;
    }
}

/**
**	Draw vertical line clipped into 16bit framebuffer.
**
**	@param color	Color index.
**	@param x	x coordinate on the screen
**	@param y	y coordinate on the screen
**	@param height	height of line.
*/
local void DrawVLineClip16(SysColors color,int x,int y,unsigned height)
{
    VMemType16* p;
    VMemType16* e;
    int w;
    int f;

    //	Clipping:
    if( x<ClipX1 || x>=ClipX2 ) {
	return;
    }
    if( y<ClipY1 ) {
	f=ClipY1-y;
	y=ClipY1;
	if( height<f ) {
	    return;
	}
	height-=f;
    }
    if( (y+height)>ClipY2 ) {
	if( height<ClipY2-y ) {
	    return;
	}
	height=ClipY2-y;
    }
    if( height>640 )
	abort();

    w=VideoWidth;
    p=VideoMemory16+y*w+x;
    e=p+height*w;
    f=Pixels16[color];
    while( p<e ) {
	*p=f;
	p+=w;
    }
}

/**
**	Draw vertical line clipped into 24bit framebuffer.
**
**	@param color	Color index.
**	@param x	x coordinate on the screen
**	@param y	y coordinate on the screen
**	@param height	height of line.
*/
local void DrawVLineClip24(SysColors color,int x,int y,unsigned height)
{
    VMemType24* p;
    VMemType24* e;
    int w;
    int t;
    VMemType24 f;

    //	Clipping:
    if( x<ClipX1 || x>=ClipX2 ) {
	return;
    }
    if( y<ClipY1 ) {
	t=ClipY1-y;
	y=ClipY1;
	if( height<t ) {
	    return;
	}
	height-=t;
    }
    if( (y+height)>ClipY2 ) {
	if( height<ClipY2-y ) {
	    return;
	}
	height=ClipY2-y;
    }
    if( height>640 )
	abort();

    w=VideoWidth;
    p=VideoMemory24+y*w+x;
    e=p+height*w;
    f=Pixels24[color];
    while( p<e ) {
	*p=f;
	p+=w;
    }
}

/**
**	Draw vertical line clipped into 32bit framebuffer.
**
**	@param color	Color index.
**	@param x	x coordinate on the screen
**	@param y	y coordinate on the screen
**	@param height	height of line.
*/
local void DrawVLineClip32(SysColors color,int x,int y,unsigned height)
{
    VMemType32* p;
    VMemType32* e;
    int w;
    int f;

    //	Clipping:
    if( x<ClipX1 || x>=ClipX2 ) {
	return;
    }
    if( y<ClipY1 ) {
	f=ClipY1-y;
	y=ClipY1;
	if( height<f ) {
	    return;
	}
	height-=f;
    }
    if( (y+height)>ClipY2 ) {
	if( height<ClipY2-y ) {
	    return;
	}
	height=ClipY2-y;
    }

    w=VideoWidth;
    p=VideoMemory32+y*w+x;
    e=p+height*w;
    f=Pixels32[color];
    while( p<e ) {
	*p=f;
	p+=w;
    }
}

// ===========================================================================
//	General Line
// ===========================================================================

/**
**	Draw line unclipped into 8bit framebuffer.
**
**	@param color	Color index.
**	@param x1	Source x coordinate on the screen
**	@param y1	Source y coordinate on the screen
**	@param x2	Destination x coordinate on the screen
**	@param y2	Destination y coordinate on the screen
*/
global void DrawLine8(SysColors color,int x1,int y1,int x2,int y2)
{
    int d;
    int dx;
    int dy;
    int xstep;
    VMemType8* p;
    int w;
    unsigned f;

    if( x1==x2 ) {
	if( y1<y2 ) {
	    DrawVLine8(color,x1,y1,y2-y1);
	} else {
	    DrawVLine8(color,x2,y2,y1-y2);
	}
	return;
    }
    if( y1==y2 ) {
	if( x1<x2 ) {
	    DrawHLine8(color,x1,y1,x2-x1);
	} else {
	    DrawHLine8(color,x2,y2,x1-x2);
	}
	return;
    }

    // initialize

    w=VideoWidth;
    f=Pixels8[color];

    if( y1>y2 ) {		// exchange coordinates
	x1^=x2; x2^=x1; x1^=x2;
	y1^=y2; y2^=y1; y1^=y2;
    }
    dy=y2-y1;
    xstep=1;
    dx=x2-x1;
    if( dx<0 ) {
	dx=-dx;
	xstep=-1;
    }

    p=VideoMemory8+w*y1+x1;
    *p=f;

    if( dx<dy ) {		// step in vertical direction
	d=dy-1;
	dx+=dx;
	dy+=dy;
	while( y1!=y2 ) {
	    y1++;
	    p+=w;
	    d-=dx;
	    if( d<0 ) {
		d+=dy;
		p+=xstep;
	    }
	    *p=f;
	}
	return;
    }

    if( dx>dy ) {		// step in horizontal direction
	d=dx-1;
	dx+=dx;
	dy+=dy;

	while( x1!=x2 ) {
	    x1+=xstep;
	    p+=xstep;
	    d-=dy;
	    if( d<0 ) {
		d+=dx;
		p+=w;
	    }
	    *p=f;
	}
	return;
    }

    while( y1!=y2) {			// diagonal line
	x1+=xstep;
	p+=xstep;
	y1++;
	p+=w;
	*p=f;
    }
}

/**
**	Draw line unclipped into 16bit framebuffer.
**
**	@param color	Color index.
**	@param x1	Source x coordinate on the screen
**	@param y1	Source y coordinate on the screen
**	@param x2	Destination x coordinate on the screen
**	@param y2	Destination y coordinate on the screen
*/
global void DrawLine16(SysColors color,int x1,int y1,int x2,int y2)
{
    int d;
    int dx;
    int dy;
    int xstep;
    VMemType16* p;
    int w;
    unsigned f;

    if( x1==x2 ) {
	if( y1<y2 ) {
	    DrawVLine16(color,x1,y1,y2-y1);
	} else {
	    DrawVLine16(color,x2,y2,y1-y2);
	}
	return;
    }
    if( y1==y2 ) {
	if( x1<x2 ) {
	    DrawHLine16(color,x1,y1,x2-x1);
	} else {
	    DrawHLine16(color,x2,y2,x1-x2);
	}
	return;
    }

    // initialize

    w=VideoWidth;
    f=Pixels16[color];

    if( y1>y2 ) {		// exchange coordinates
	x1^=x2; x2^=x1; x1^=x2;
	y1^=y2; y2^=y1; y1^=y2;
    }
    dy=y2-y1;
    xstep=1;
    dx=x2-x1;
    if( dx<0 ) {
	dx=-dx;
	xstep=-1;
    }

    p=VideoMemory16+w*y1+x1;
    *p=f;

    if( dx<dy ) {		// step in vertical direction
	d=dy-1;
	dx+=dx;
	dy+=dy;
	while( y1!=y2 ) {
	    y1++;
	    p+=w;
	    d-=dx;
	    if( d<0 ) {
		d+=dy;
		p+=xstep;
	    }
	    *p=f;
	}
	return;
    }

    if( dx>dy ) {		// step in horizontal direction
	d=dx-1;
	dx+=dx;
	dy+=dy;

	while( x1!=x2 ) {
	    x1+=xstep;
	    p+=xstep;
	    d-=dy;
	    if( d<0 ) {
		d+=dx;
		p+=w;
	    }
	    *p=f;
	}
	return;
    }

    while( y1!=y2) {			// diagonal line
	x1+=xstep;
	p+=xstep;
	y1++;
	p+=w;
	*p=f;
    }
}

/**
**	Draw line unclipped into 24bit framebuffer.
**
**	@param color	Color index.
**	@param x1	Source x coordinate on the screen
**	@param y1	Source y coordinate on the screen
**	@param x2	Destination x coordinate on the screen
**	@param y2	Destination y coordinate on the screen
*/
global void DrawLine24(SysColors color,int x1,int y1,int x2,int y2)
{
    int d;
    int dx;
    int dy;
    int xstep;
    VMemType24* p;
    int w;
    VMemType24 f;

    if( x1==x2 ) {
	if( y1<y2 ) {
	    DrawVLine24(color,x1,y1,y2-y1);
	} else {
	    DrawVLine24(color,x2,y2,y1-y2);
	}
	return;
    }
    if( y1==y2 ) {
	if( x1<x2 ) {
	    DrawHLine24(color,x1,y1,x2-x1);
	} else {
	    DrawHLine24(color,x2,y2,x1-x2);
	}
	return;
    }

    // initialize

    w=VideoWidth;
    f=Pixels24[color];

    if( y1>y2 ) {		// exchange coordinates
	x1^=x2; x2^=x1; x1^=x2;
	y1^=y2; y2^=y1; y1^=y2;
    }
    dy=y2-y1;
    xstep=1;
    dx=x2-x1;
    if( dx<0 ) {
	dx=-dx;
	xstep=-1;
    }

    p=VideoMemory24+w*y1+x1;
    *p=f;

    if( dx<dy ) {		// step in vertical direction
	d=dy-1;
	dx+=dx;
	dy+=dy;
	while( y1!=y2 ) {
	    y1++;
	    p+=w;
	    d-=dx;
	    if( d<0 ) {
		d+=dy;
		p+=xstep;
	    }
	    *p=f;
	}
	return;
    }

    if( dx>dy ) {		// step in horizontal direction
	d=dx-1;
	dx+=dx;
	dy+=dy;

	while( x1!=x2 ) {
	    x1+=xstep;
	    p+=xstep;
	    d-=dy;
	    if( d<0 ) {
		d+=dx;
		p+=w;
	    }
	    *p=f;
	}
	return;
    }

    while( y1!=y2) {			// diagonal line
	x1+=xstep;
	p+=xstep;
	y1++;
	p+=w;
	*p=f;
    }
}

/**
**	Draw line unclipped into 32bit framebuffer.
**
**	@param color	Color index.
**	@param x1	Source x coordinate on the screen
**	@param y1	Source y coordinate on the screen
**	@param x2	Destination x coordinate on the screen
**	@param y2	Destination y coordinate on the screen
*/
global void DrawLine32(SysColors color,int x1,int y1,int x2,int y2)
{
    int d;
    int dx;
    int dy;
    int xstep;
    VMemType32* p;
    int w;
    unsigned f;

    if( x1==x2 ) {
	if( y1<y2 ) {
	    DrawVLine32(color,x1,y1,y2-y1);
	} else {
	    DrawVLine32(color,x2,y2,y1-y2);
	}
	return;
    }
    if( y1==y2 ) {
	if( x1<x2 ) {
	    DrawHLine32(color,x1,y1,x2-x1);
	} else {
	    DrawHLine32(color,x2,y2,x1-x2);
	}
	return;
    }

    // initialize

    w=VideoWidth;
    f=Pixels32[color];

    if( y1>y2 ) {		// exchange coordinates
	x1^=x2; x2^=x1; x1^=x2;
	y1^=y2; y2^=y1; y1^=y2;
    }
    dy=y2-y1;
    xstep=1;
    dx=x2-x1;
    if( dx<0 ) {
	dx=-dx;
	xstep=-1;
    }

    p=VideoMemory32+w*y1+x1;
    *p=f;

    if( dx<dy ) {		// step in vertical direction
	d=dy-1;
	dx+=dx;
	dy+=dy;
	while( y1!=y2 ) {
	    y1++;
	    p+=w;
	    d-=dx;
	    if( d<0 ) {
		d+=dy;
		p+=xstep;
	    }
	    *p=f;
	}
	return;
    }

    if( dx>dy ) {		// step in horizontal direction
	d=dx-1;
	dx+=dx;
	dy+=dy;

	while( x1!=x2 ) {
	    x1+=xstep;
	    p+=xstep;
	    d-=dy;
	    if( d<0 ) {
		d+=dx;
		p+=w;
	    }
	    *p=f;
	}
	return;
    }

    while( y1!=y2) {			// diagonal line
	x1+=xstep;
	p+=xstep;
	y1++;
	p+=w;
	*p=f;
    }
}

/**
**	Liang/Barksy clipping algorithm.
**
**	FIXME: could remove floating point, but on my cpu floating point
**	FIXME: is faster than integer.
*/
local int ClipTest(double p,double q,double *u1,double *u2)
{
    double r;

    r = q/p;
    if (p < 0) {
	if (r > *u2) {
	    return 0;
	}
	if (r > *u1) {
	    *u1 = r;
	}
	return 1;
    }
    if (p > 0) {
	if (r < *u1) {
	    return 0;
	}
	if (r < *u2) {
	    *u2 = r;
	}
	return 1;
    }
    if (q < 0) {
	return 0;
    }
    return 1;
}

/**
**	Liang/Barksy clipping algorithm.
*/
local void LineClip(int x1,int y1,int x2,int y2)
{
    double u1;
    double u2;
    int dx;
    int dy;

    u1 = 0.0;
    u2 = 1.0;
    dx = x2-x1;
    if (ClipTest(-dx,x1-ClipX1, &u1, &u2)
	    && ClipTest(dx,ClipX2-x1, &u1, &u2)) {
	dy = y2-y1;
	if (ClipTest(-dy, y1-ClipY1, &u1, &u2)
		&& ClipTest(dy,ClipY2-y1, &u1, &u2)) {
	    if (u1 > 0) {
		x1 = (x1 + u1*dx) + 0.5;
		y1 = (y1 + u1*dy) + 0.5;
	    }
	    if (u2 < 1) {
		x2 = (x1 + u2*dx) + 0.5;
		y2 = (y1 + u2*dy) + 0.5;
	    }
	}
    }
}

/**
**	Draw line clipped into 8bit framebuffer.
**
**	@param color	Color index.
**	@param x1	Source x coordinate on the screen
**	@param y1	Source y coordinate on the screen
**	@param x2	Destination x coordinate on the screen
**	@param y2	Destination y coordinate on the screen
*/
global void DrawLineClip8(SysColors color,int x1,int y1,int x2,int y2)
{
    double u1;
    double u2;
    int dx;
    int dy;

    u1 = 0.0;
    u2 = 1.0;
    dx = x2-x1;
    if (ClipTest(-dx,x1-ClipX1, &u1, &u2)
	    && ClipTest(dx,ClipX2-x1, &u1, &u2)) {
	dy = y2-y1;
	if (ClipTest(-dy, y1-ClipY1, &u1, &u2)
		&& ClipTest(dy,ClipY2-y1, &u1, &u2)) {
	    if (u1 > 0) {
		x1 = (x1 + u1*dx) + 0.5;
		y1 = (y1 + u1*dy) + 0.5;
	    }
	    if (u2 < 1) {
		x2 = (x1 + u2*dx) + 0.5;
		y2 = (y1 + u2*dy) + 0.5;
	    }

	    DrawLine8(color,x1,y1,x2,y2);
	}
    }
}

/**
**	Draw line clipped into 16bit framebuffer.
**
**	@param color	Color index.
**	@param x1	Source x coordinate on the screen
**	@param y1	Source y coordinate on the screen
**	@param x2	Destination x coordinate on the screen
**	@param y2	Destination y coordinate on the screen
*/
global void DrawLineClip16(SysColors color,int x1,int y1,int x2,int y2)
{
    double u1;
    double u2;
    int dx;
    int dy;

    u1 = 0.0;
    u2 = 1.0;
    dx = x2-x1;
    if (ClipTest(-dx,x1-ClipX1, &u1, &u2)
	    && ClipTest(dx,ClipX2-x1, &u1, &u2)) {
	dy = y2-y1;
	if (ClipTest(-dy, y1-ClipY1, &u1, &u2)
		&& ClipTest(dy,ClipY2-y1, &u1, &u2)) {
	    if (u1 > 0) {
		x1 = (x1 + u1*dx) + 0.5;
		y1 = (y1 + u1*dy) + 0.5;
	    }
	    if (u2 < 1) {
		x2 = (x1 + u2*dx) + 0.5;
		y2 = (y1 + u2*dy) + 0.5;
	    }

	    DrawLine16(color,x1,y1,x2,y2);
	}
    }
}

/**
**	Draw line clipped into 24bit framebuffer.
**
**	@param color	Color index.
**	@param x1	Source x coordinate on the screen
**	@param y1	Source y coordinate on the screen
**	@param x2	Destination x coordinate on the screen
**	@param y2	Destination y coordinate on the screen
*/
global void DrawLineClip24(SysColors color,int x1,int y1,int x2,int y2)
{
    double u1;
    double u2;
    int dx;
    int dy;

    u1 = 0.0;
    u2 = 1.0;
    dx = x2-x1;
    if (ClipTest(-dx,x1-ClipX1, &u1, &u2)
	    && ClipTest(dx,ClipX2-x1, &u1, &u2)) {
	dy = y2-y1;
	if (ClipTest(-dy, y1-ClipY1, &u1, &u2)
		&& ClipTest(dy,ClipY2-y1, &u1, &u2)) {
	    if (u1 > 0) {
		x1 = (x1 + u1*dx) + 0.5;
		y1 = (y1 + u1*dy) + 0.5;
	    }
	    if (u2 < 1) {
		x2 = (x1 + u2*dx) + 0.5;
		y2 = (y1 + u2*dy) + 0.5;
	    }

	    DrawLine24(color,x1,y1,x2,y2);
	}
    }
}

/**
**	Draw line clipped into 32bit framebuffer.
**
**	@param color	Color index.
**	@param x1	Source x coordinate on the screen
**	@param y1	Source y coordinate on the screen
**	@param x2	Destination x coordinate on the screen
**	@param y2	Destination y coordinate on the screen
*/
global void DrawLineClip32(SysColors color,int x1,int y1,int x2,int y2)
{
    double u1;
    double u2;
    int dx;
    int dy;

    u1 = 0.0;
    u2 = 1.0;
    dx = x2-x1;
    if (ClipTest(-dx,x1-ClipX1, &u1, &u2)
	    && ClipTest(dx,ClipX2-x1, &u1, &u2)) {
	dy = y2-y1;
	if (ClipTest(-dy, y1-ClipY1, &u1, &u2)
		&& ClipTest(dy,ClipY2-y1, &u1, &u2)) {
	    if (u1 > 0) {
		x1 = (x1 + u1*dx) + 0.5;
		y1 = (y1 + u1*dy) + 0.5;
	    }
	    if (u2 < 1) {
		x2 = (x1 + u2*dx) + 0.5;
		y2 = (y1 + u2*dy) + 0.5;
	    }

	    DrawLine32(color,x1,y1,x2,y2);
	}
    }
}

// ===========================================================================
//	Rectangle
// ===========================================================================

/**
**	Draw rectangle into 8bpp frame buffer.
**
**	@param color	Color index.
**	@param x	x coordinate on the screen
**	@param y	y coordinate on the screen
**	@param h	height of rectangle.
**	@param w	width of rectangle.
*/
global void DrawRectangle8(SysColors color,int x,int y
	,unsigned w,unsigned h)
{
    DrawHLine8(color,x,y,w);
    DrawVLine8(color,x,y+1,h);
    DrawHLine8(color,x+1,y+h,w);
    DrawVLine8(color,x+w,y,h);
}

/**
**	Draw rectangle into 16bpp frame buffer.
**
**	@param color	Color index.
**	@param x	x coordinate on the screen
**	@param y	y coordinate on the screen
**	@param h	height of rectangle.
**	@param w	width of rectangle.
*/
global void DrawRectangle16(SysColors color,int x,int y
	,unsigned w,unsigned h)
{
    DrawHLine16(color,x,y,w);
    DrawVLine16(color,x,y+1,h);
    DrawHLine16(color,x+1,y+h,w);
    DrawVLine16(color,x+w,y,h);
}

/**
**	Draw rectangle into 24bpp frame buffer.
**
**	@param color	Color index.
**	@param x	x coordinate on the screen
**	@param y	y coordinate on the screen
**	@param h	height of rectangle.
**	@param w	width of rectangle.
*/
global void DrawRectangle24(SysColors color,int x,int y
	,unsigned w,unsigned h)
{
    DrawHLine24(color,x,y,w);
    DrawVLine24(color,x,y+1,h);
    DrawHLine24(color,x+1,y+h,w);
    DrawVLine24(color,x+w,y,h);
}

/**
**	Draw rectangle into 32bpp frame buffer.
**
**	@param color	Color index.
**	@param x	x coordinate on the screen
**	@param y	y coordinate on the screen
**	@param h	height of rectangle.
**	@param w	width of rectangle.
*/
global void DrawRectangle32(SysColors color,int x,int y
	,unsigned w,unsigned h)
{
    DrawHLine32(color,x,y,w);
    DrawVLine32(color,x,y+1,h);
    DrawHLine32(color,x+1,y+h,w);
    DrawVLine32(color,x+w,y,h);
}

/**
**	Draw rectangle clipped into 8bpp frame buffer.
**
**	@param color	Color index.
**	@param x	x coordinate on the screen
**	@param y	y coordinate on the screen
**	@param h	height of rectangle.
**	@param w	width of rectangle.
*/
global void DrawRectangleClip8(SysColors color,int x,int y
	,unsigned w,unsigned h)
{
    //	FIXME: Clip here
    DrawHLineClip8(color,x,y,w);
    DrawVLineClip8(color,x,y+1,h);
    DrawHLineClip8(color,x+1,y+h,w);
    DrawVLineClip8(color,x+w,y,h);
}

/**
**	Draw rectangle clipped into 16bpp frame buffer.
**
**	@param color	Color index.
**	@param x	x coordinate on the screen
**	@param y	y coordinate on the screen
**	@param h	height of rectangle.
**	@param w	width of rectangle.
*/
global void DrawRectangleClip16(SysColors color,int x,int y
	,unsigned w,unsigned h)
{
    //	FIXME: Clip here
    DrawHLineClip16(color,x,y,w);
    DrawVLineClip16(color,x,y+1,h);
    DrawHLineClip16(color,x+1,y+h,w);
    DrawVLineClip16(color,x+w,y,h);
}

/**
**	Draw rectangle clipped into 24bpp frame buffer.
**
**	@param color	Color index.
**	@param x	x coordinate on the screen
**	@param y	y coordinate on the screen
**	@param h	height of rectangle.
**	@param w	width of rectangle.
*/
global void DrawRectangleClip24(SysColors color,int x,int y
	,unsigned w,unsigned h)
{
    //	FIXME: Clip here
    DrawHLineClip24(color,x,y,w);
    DrawVLineClip24(color,x,y+1,h);
    DrawHLineClip24(color,x+1,y+h,w);
    DrawVLineClip24(color,x+w,y,h);
}

/**
**	Draw rectangle clipped into 32bpp frame buffer.
**
**	@param color	Color index.
**	@param x	x coordinate on the screen
**	@param y	y coordinate on the screen
**	@param h	height of rectangle.
**	@param w	width of rectangle.
*/
global void DrawRectangleClip32(SysColors color,int x,int y
	,unsigned w,unsigned h)
{
    //	FIXME: Clip here
    DrawHLineClip32(color,x,y,w);
    DrawVLineClip32(color,x,y+1,h);
    DrawHLineClip32(color,x+1,y+h,w);
    DrawVLineClip32(color,x+w,y,h);
}

// ===========================================================================
//	Circle
// ===========================================================================

// FIXME: could write a general circle function?

/**
**	Draw circle.
**
**	@param color	Color index.
**	@param x	Center x coordinate on the screen
**	@param y	Center y coordinate on the screen
**	@param r	radius of circle
*/
global void VideoDrawCircle(SysColors color,int x,int y,unsigned r)
{
    int cx;
    int cy;
    int df;
    int d_e;
    int d_se;

    cx=0;
    cy=r;
    df=1-r;
    d_e=3;
    d_se=-2*r+5;

    // FIXME: could much improved :)
    do {
	if( cx==0 ) {
	    VideoDrawPixel(color,x,y+cy);
	    VideoDrawPixel(color,x,y-cy);
	    VideoDrawPixel(color,x+cy,y);
	    VideoDrawPixel(color,x-cy,y);
	} else if ( cx==cy ) {
	    DebugCheck( cx==0 || cy==0 );
	    VideoDrawPixel(color,x+cx,y+cy);
	    VideoDrawPixel(color,x-cx,y+cy);
	    VideoDrawPixel(color,x+cx,y-cy);
	    VideoDrawPixel(color,x-cx,y-cy);
	} else if ( cx<cy ) {
	    DebugCheck( cx==0 || cy==0 );
	    VideoDrawPixel(color,x+cx,y+cy);
	    VideoDrawPixel(color,x+cx,y-cy);
	    VideoDrawPixel(color,x+cy,y+cx);
	    VideoDrawPixel(color,x+cy,y-cx);
	    VideoDrawPixel(color,x-cx,y+cy);
	    VideoDrawPixel(color,x-cx,y-cy);
	    VideoDrawPixel(color,x-cy,y+cx);
	    VideoDrawPixel(color,x-cy,y-cx);
	}
	if( df<0 ) {
	    df+=d_e;
	    d_se+=2;
	} else {
	    df+=d_se;
	    d_se+=4;
	    cy--;
	}
	d_e+=2;
	cx++;

    } while( cx <= cy );
}

/**
**	Draw circle clipped.
**
**	@param color	Color index.
**	@param x	Center x coordinate on the screen
**	@param y	Center y coordinate on the screen
**	@param r	radius of circle
*/
global void VideoDrawCircleClip(SysColors color,int x,int y,unsigned r)
{
    int cx;
    int cy;
    int df;
    int d_e;
    int d_se;

    cx=0;
    cy=r;
    df=1-r;
    d_e=3;
    d_se=-2*r+5;

    // FIXME: could much improved :)
    do {
	if( cx==0 ) {
	    VideoDrawPixelClip(color,x,y+cy);
	    VideoDrawPixelClip(color,x,y-cy);
	    VideoDrawPixelClip(color,x+cy,y);
	    VideoDrawPixelClip(color,x-cy,y);
	} else if ( cx==cy ) {
	    DebugCheck( cx==0 || cy==0 );
	    VideoDrawPixelClip(color,x+cx,y+cy);
	    VideoDrawPixelClip(color,x-cx,y+cy);
	    VideoDrawPixelClip(color,x+cx,y-cy);
	    VideoDrawPixelClip(color,x-cx,y-cy);
	} else if ( cx<cy ) {
	    DebugCheck( cx==0 || cy==0 );
	    VideoDrawPixelClip(color,x+cx,y+cy);
	    VideoDrawPixelClip(color,x+cx,y-cy);
	    VideoDrawPixelClip(color,x+cy,y+cx);
	    VideoDrawPixelClip(color,x+cy,y-cx);
	    VideoDrawPixelClip(color,x-cx,y+cy);
	    VideoDrawPixelClip(color,x-cx,y-cy);
	    VideoDrawPixelClip(color,x-cy,y+cx);
	    VideoDrawPixelClip(color,x-cy,y-cx);
	}
	if( df<0 ) {
	    df+=d_e;
	    d_se+=2;
	} else {
	    df+=d_se;
	    d_se+=4;
	    cy--;
	}
	d_e+=2;
	cx++;
    } while( cx <= cy );
}

// ===========================================================================
//	Filled rectangle
// ===========================================================================

/**
**	Fill rectangle clipped.
**
**	@param color	Color index.
**	@param x	x coordinate on the screen
**	@param y	y coordinate on the screen
**	@param h	height of rectangle.
**	@param w	width of rectangle.
*/
global void VideoFillRectangle(SysColors color,int x,int y
	,unsigned w,unsigned h)
{
    while( h-- ) {
	VideoDrawHLine(color,x,y++,w);
    }
}

/**
**	Fill rectangle 25% translucent clipped.
**
**	@param color	Color index.
**	@param x	x coordinate on the screen
**	@param y	y coordinate on the screen
**	@param h	height of rectangle.
**	@param w	width of rectangle.
*/
global void VideoFill25TransRectangle(SysColors color,int x,int y
	,unsigned w,unsigned h)
{
    while( h-- ) {
	VideoDraw25TransHLine(color,x,y++,w);
    }
}

/**
**	Fill rectangle 50% translucent clipped.
**
**	@param color	Color index.
**	@param x	x coordinate on the screen
**	@param y	y coordinate on the screen
**	@param h	height of rectangle.
**	@param w	width of rectangle.
*/
global void VideoFill50TransRectangle(SysColors color,int x,int y
	,unsigned w,unsigned h)
{
    while( h-- ) {
	VideoDraw50TransHLine(color,x,y++,w);
    }
}

/**
**	Fill rectangle 75% translucent clipped.
**
**	@param color	Color index.
**	@param x	x coordinate on the screen
**	@param y	y coordinate on the screen
**	@param h	height of rectangle.
**	@param w	width of rectangle.
*/
global void VideoFill75TransRectangle(SysColors color,int x,int y
	,unsigned w,unsigned h)
{
    while( h-- ) {
	VideoDraw75TransHLine(color,x,y++,w);
    }
}

/**
**	Fill rectangle clipped.
**
**	@param color	Color index.
**	@param x	x coordinate on the screen
**	@param y	y coordinate on the screen
**	@param h	height of rectangle.
**	@param w	width of rectangle.
*/
global void VideoFillRectangleClip(SysColors color,int x,int y
	,unsigned w,unsigned h)
{
    //	FIXME: Clip here
    while( h-- ) {
	VideoDrawHLineClip(color,x,y++,w);
    }
}

/**
**	Fill rectangle 25% translucent clipped.
**
**	@param color	Color index.
**	@param x	x coordinate on the screen
**	@param y	y coordinate on the screen
**	@param h	height of rectangle.
**	@param w	width of rectangle.
*/
global void VideoFill25TransRectangleClip(SysColors color,int x,int y
	,unsigned w,unsigned h)
{
    //	FIXME: Clip here
    while( h-- ) {
	VideoDraw25TransHLineClip(color,x,y++,w);
    }
}

/**
**	Fill rectangle 50% translucent clipped.
**
**	@param color	Color index.
**	@param x	x coordinate on the screen
**	@param y	y coordinate on the screen
**	@param h	height of rectangle.
**	@param w	width of rectangle.
*/
global void VideoFill50TransRectangleClip(SysColors color,int x,int y
	,unsigned w,unsigned h)
{
    //	FIXME: Clip here
    while( h-- ) {
	VideoDraw50TransHLineClip(color,x,y++,w);
    }
}

/**
**	Fill rectangle 75% translucent clipped.
**
**	@param color	Color index.
**	@param x	x coordinate on the screen
**	@param y	y coordinate on the screen
**	@param h	height of rectangle.
**	@param w	width of rectangle.
*/
global void VideoFill75TransRectangleClip(SysColors color,int x,int y
	,unsigned w,unsigned h)
{
    //	FIXME: Clip here
    while( h-- ) {
	VideoDraw75TransHLineClip(color,x,y++,w);
    }
}

/**
**	Fill rectangle translucent clipped.
**
**	@param color	Color index.
**	@param x	x coordinate on the screen
**	@param y	y coordinate on the screen
**	@param h	height of rectangle.
**	@param w	width of rectangle.
**	@param alpha	alpha value of pixels.
*/
global void VideoFillTransRectangleClip(SysColors color,int x,int y
	,unsigned w,unsigned h,int alpha)
{
    //	FIXME: Clip here
    while( h-- ) {
	VideoDrawTransHLineClip(color,x,y++,w,alpha);
    }
}

/**
**	Draw translucent horizontal line clipped into 15bit framebuffer.
**
**	@param color	Color index.
**	@param x	x coordinate on the screen
**	@param y	y coordinate on the screen
**	@param width	width of line.
**	@param alpha	alpha value of pixels.
*/
local void DrawTransHLineClip15(SysColors color,int x,int y,unsigned width
	,int alpha)
{
    int f;

    if( y<ClipY1 || y>=ClipY2 ) {	//	Clipping:
	return;
    }
    if( x<ClipX1 ) {
	f=ClipX1-x;
	x=ClipX1;
	if( width<f ) {
	    return;
	}
	width-=f;
    }
    if( (x+width)>ClipX2 ) {
	if( width<ClipX2-x ) {
	    return;
	}
	width=ClipX2-x;
    }

    DrawTransHLine15(color,x,y,width,alpha);
}

/**
**	Draw translucent horizontal line clipped into 16bit framebuffer.
**
**	@param color	Color index.
**	@param x	x coordinate on the screen
**	@param y	y coordinate on the screen
**	@param width	width of line.
**	@param alpha	alpha value of pixels.
*/
local void DrawTransHLineClip16(SysColors color,int x,int y,unsigned width
	,int alpha)
{
    int f;

    if( y<ClipY1 || y>=ClipY2 ) {	//	Clipping:
	return;
    }
    if( x<ClipX1 ) {
	f=ClipX1-x;
	x=ClipX1;
	if( width<f ) {
	    return;
	}
	width-=f;
    }
    if( (x+width)>ClipX2 ) {
	if( width<ClipX2-x ) {
	    return;
	}
	width=ClipX2-x;
    }

    DrawTransHLine16(color,x,y,width,alpha);
}

// ===========================================================================
//	Filled circle
// ===========================================================================

/**
**	Fill circle clipped.
**
**	@param color	Color index.
**	@param x	Center x coordinate on the screen
**	@param y	Center y coordinate on the screen
**	@param r	radius of circle
*/
global void VideoFillCircleClip(SysColors color,int x,int y,unsigned r)
{
    int cx;
    int cy;
    int df;
    int d_e;
    int d_se;

    cx=0;
    cy=r;
    df=1-r;
    d_e=3;
    d_se=-2*r+5;

    // FIXME: could much improved :)
    do {
	VideoDrawHLineClip(color,x-cy,y-cx,1+cy*2);
	if( cx ) {
	    VideoDrawHLineClip(color,x-cy,y+cx,1+cy*2);
	}
	if( df<0 ) {
	    df+=d_e;
	    d_se+=2;
	} else {
	    if( cx!=cy ) {
		VideoDrawHLineClip(color,x-cx,y-cy,1+cx*2);
		VideoDrawHLineClip(color,x-cx,y+cy,1+cx*2);
	    }
	    df+=d_se;
	    d_se+=4;
	    cy--;
	}
	d_e+=2;
	cx++;

    } while( cx <= cy );
}

/**
**	Fill circle 25% translucent clipped.
**
**	@param color	Color index.
**	@param x	Center x coordinate on the screen
**	@param y	Center y coordinate on the screen
**	@param r	radius of circle
*/
global void VideoFill25TransCircleClip(SysColors color,int x,int y,unsigned r)
{
    int cx;
    int cy;
    int df;
    int d_e;
    int d_se;

    cx=0;
    cy=r;
    df=1-r;
    d_e=3;
    d_se=-2*r+5;

    // FIXME: could much improved :)
    do {
	VideoDraw25TransHLineClip(color,x-cy,y-cx,1+cy*2);
	if( cx ) {
	    VideoDraw25TransHLineClip(color,x-cy,y+cx,1+cy*2);
	}
	if( df<0 ) {
	    df+=d_e;
	    d_se+=2;
	} else {
	    if( cx!=cy ) {
		VideoDraw25TransHLineClip(color,x-cx,y-cy,1+cx*2);
		VideoDraw25TransHLineClip(color,x-cx,y+cy,1+cx*2);
	    }
	    df+=d_se;
	    d_se+=4;
	    cy--;
	}
	d_e+=2;
	cx++;

    } while( cx <= cy );
}

/**
**	Fill circle 50% translucent clipped.
**
**	@param color	Color index.
**	@param x	Center x coordinate on the screen
**	@param y	Center y coordinate on the screen
**	@param r	radius of circle
*/
global void VideoFill50TransCircleClip(SysColors color,int x,int y,unsigned r)
{
    int cx;
    int cy;
    int df;
    int d_e;
    int d_se;

    cx=0;
    cy=r;
    df=1-r;
    d_e=3;
    d_se=-2*r+5;

    // FIXME: could much improved :)
    do {
	VideoDraw50TransHLineClip(color,x-cy,y-cx,1+cy*2);
	if( cx ) {
	    VideoDraw50TransHLineClip(color,x-cy,y+cx,1+cy*2);
	}
	if( df<0 ) {
	    df+=d_e;
	    d_se+=2;
	} else {
	    if( cx!=cy ) {
		VideoDraw50TransHLineClip(color,x-cx,y-cy,1+cx*2);
		VideoDraw50TransHLineClip(color,x-cx,y+cy,1+cx*2);
	    }
	    df+=d_se;
	    d_se+=4;
	    cy--;
	}
	d_e+=2;
	cx++;

    } while( cx <= cy );
}

/**
**	Fill circle 75% translucent clipped.
**
**	@param color	Color index.
**	@param x	Center x coordinate on the screen
**	@param y	Center y coordinate on the screen
**	@param r	radius of circle
*/
global void VideoFill75TransCircleClip(SysColors color,int x,int y,unsigned r)
{
    int cx;
    int cy;
    int df;
    int d_e;
    int d_se;

    cx=0;
    cy=r;
    df=1-r;
    d_e=3;
    d_se=-2*r+5;

    // FIXME: could much improved :)
    do {
	VideoDraw75TransHLineClip(color,x-cy,y-cx,1+cy*2);
	if( cx ) {
	    VideoDraw75TransHLineClip(color,x-cy,y+cx,1+cy*2);
	}
	if( df<0 ) {
	    df+=d_e;
	    d_se+=2;
	} else {
	    if( cx!=cy ) {
		VideoDraw75TransHLineClip(color,x-cx,y-cy,1+cx*2);
		VideoDraw75TransHLineClip(color,x-cx,y+cy,1+cx*2);
	    }
	    df+=d_se;
	    d_se+=4;
	    cy--;
	}
	d_e+=2;
	cx++;

    } while( cx <= cy );
}

/*----------------------------------------------------------------------------
--	Global functions
----------------------------------------------------------------------------*/

/**
**	Init linedraw
*/
global void InitLineDraw(void)
{
    switch( VideoBpp ) {
	case 8:
	    VideoDrawPixel=DrawPixel8;
	    VideoDrawPixelClip=DrawPixelClip8;
	    VideoDrawHLine=DrawHLine8;
	    VideoDrawHLineClip=DrawHLineClip8;
	    VideoDrawVLine=DrawVLine8;
	    VideoDrawVLineClip=DrawVLineClip8;
	    VideoDrawLine=DrawLine8;
	    VideoDrawLineClip=DrawLineClip8;
	    VideoDrawRectangle=DrawRectangle8;
	    VideoDrawRectangleClip=DrawRectangleClip8;
	    break;

	case 15:
	    VideoDrawPixel=DrawPixel16;
	    VideoDrawPixelClip=DrawPixelClip16;
	    VideoDrawHLine=DrawHLine16;
	    VideoDrawHLineClip=DrawHLineClip16;
	    VideoDraw25TransHLine=Draw25TransHLine15;
	    VideoDraw25TransHLineClip=Draw25TransHLineClip15;
	    VideoDraw50TransHLine=Draw50TransHLine15;
	    VideoDraw50TransHLineClip=Draw50TransHLineClip15;
	    VideoDraw75TransHLine=Draw75TransHLine15;
	    VideoDraw75TransHLineClip=Draw75TransHLineClip15;
	    VideoDrawTransHLine=DrawTransHLine15;
	    VideoDrawTransHLineClip=DrawTransHLineClip15;
	    VideoDrawVLine=DrawVLine16;
	    VideoDrawVLineClip=DrawVLineClip16;
	    VideoDrawLine=DrawLine16;
	    VideoDrawLineClip=DrawLineClip16;
	    VideoDrawRectangle=DrawRectangle16;
	    VideoDrawRectangleClip=DrawRectangleClip16;
	    break;

	case 16:
	    VideoDrawPixel=DrawPixel16;
	    VideoDrawPixelClip=DrawPixelClip16;
	    VideoDrawHLine=DrawHLine16;
	    VideoDrawHLineClip=DrawHLineClip16;
	    VideoDraw25TransHLine=Draw25TransHLine16;
	    VideoDraw25TransHLineClip=Draw25TransHLineClip16;
	    VideoDraw50TransHLine=Draw50TransHLine16;
	    VideoDraw50TransHLineClip=Draw50TransHLineClip16;
	    VideoDraw75TransHLine=Draw75TransHLine16;
	    VideoDraw75TransHLineClip=Draw75TransHLineClip16;
	    VideoDrawTransHLine=DrawTransHLine16;
	    VideoDrawTransHLineClip=DrawTransHLineClip16;
	    VideoDrawVLine=DrawVLine16;
	    VideoDrawVLineClip=DrawVLineClip16;
	    VideoDrawLine=DrawLine16;
	    VideoDrawLineClip=DrawLineClip16;
	    VideoDrawRectangle=DrawRectangle16;
	    VideoDrawRectangleClip=DrawRectangleClip16;
	    break;

	case 24:
	    VideoDrawPixel=DrawPixel24;
	    VideoDrawPixelClip=DrawPixelClip24;
	    VideoDrawHLine=DrawHLine24;
	    VideoDrawHLineClip=DrawHLineClip24;
	    VideoDrawVLine=DrawVLine24;
	    VideoDrawVLineClip=DrawVLineClip24;
	    VideoDrawLine=DrawLine24;
	    VideoDrawLineClip=DrawLineClip24;
	    VideoDrawRectangle=DrawRectangle24;
	    VideoDrawRectangleClip=DrawRectangleClip24;
	    break;

	case 32:
	    VideoDrawPixel=DrawPixel32;
	    VideoDrawPixelClip=DrawPixelClip32;
	    VideoDrawHLine=DrawHLine32;
	    VideoDrawHLineClip=DrawHLineClip32;
	    VideoDrawVLine=DrawVLine32;
	    VideoDrawVLineClip=DrawVLineClip32;
	    VideoDrawLine=DrawLine32;
	    VideoDrawLineClip=DrawLineClip32;
	    VideoDrawRectangle=DrawRectangle32;
	    VideoDrawRectangleClip=DrawRectangleClip32;
	    break;

	default:
	    DebugLevel0Fn("unsupported %d bpp\n",VideoBpp);
	    abort();
    }
}

//@}
