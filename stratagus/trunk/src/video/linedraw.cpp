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
/*
**	(c) Copyright 2000 by Lutz Sammer
**
**	$Id$
*/

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
    **	Draw pixel clipped to current clip setting.
    **
    **	@param color	Color index.
    **	@param x	x coordinate on the screen
    **	@param y	y coordinate on the screen
    */
global void (*VideoDrawPixelClip)(SysColors color,int x,int y);


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
    **	Draw horizontal line clipped to current clip setting
    **
    **	@param color	Color index.
    **	@param x	x coordinate on the screen
    **	@param y	y coordinate on the screen
    **	@param width	width of line.
    */
global void (*VideoDrawHLineClip)(SysColors color,int x,int y
	,unsigned width);

/*----------------------------------------------------------------------------
--	Local functions
----------------------------------------------------------------------------*/

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
    f=(Pixels16[color]<<16)|Pixels16[color];

    while( p<e ) {			// draw 2 pixels
	*((unsigned long*)p)++=f;
    }

    if( p<e+1 ) {
	*p=f;
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

/**
**	Draw rectangle.
**
**	@param color	Color index.
**	@param x	x coordinate on the screen
**	@param y	y coordinate on the screen
**	@param h	height of rectangle.
**	@param w	width of rectangle.
*/
global void VideoDrawRectangle(SysColors color,int x,int y
	,unsigned w,unsigned h)
{
    //	FIXME: Clip here
    VideoDrawHLineClip(color,x,y,w);
    VideoDrawVLineClip(color,x,y+1,h);
    VideoDrawHLineClip(color,x+1,y+h,w);
    VideoDrawVLineClip(color,x+w,y,h);
}

/**
**	Fill rectangle.
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
    //	FIXME: Clip here
    while( h-- ) {
	VideoDrawHLineClip(color,x,y++,w);
    }
}

/*----------------------------------------------------------------------------
--	Global functions
----------------------------------------------------------------------------*/

/**
**	Init linedraw
*/
global void InitLineDraw(void)
{
    switch( VideoDepth ) {
	case 8:
	    VideoDrawPixel=DrawPixel8;
	    VideoDrawPixelClip=DrawPixelClip8;
	    VideoDrawHLine=DrawHLine8;
	    VideoDrawHLineClip=DrawHLineClip8;
	    VideoDrawVLine=DrawVLine8;
	    VideoDrawVLineClip=DrawVLineClip8;
	    break;

	case 15:
	case 16:
	    VideoDrawPixel=DrawPixel16;
	    VideoDrawPixelClip=DrawPixelClip16;
	    VideoDrawHLine=DrawHLine16;
	    VideoDrawHLineClip=DrawHLineClip16;
	    VideoDrawVLine=DrawVLine16;
	    VideoDrawVLineClip=DrawVLineClip16;
	    break;

	case 24:
	case 32:
	    VideoDrawPixel=DrawPixel32;
	    VideoDrawPixelClip=DrawPixelClip32;
	    VideoDrawHLine=DrawHLine32;
	    VideoDrawHLineClip=DrawHLineClip32;
	    VideoDrawVLine=DrawVLine32;
	    VideoDrawVLineClip=DrawVLineClip32;
	    break;

	default:
	    DebugLevel0(__FUNCTION__": unsupported %d bpp\n",VideoDepth);
	    abort();
    }
}

//@}
