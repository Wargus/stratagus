//       _________ __                 __                               
//      /   _____//  |_____________ _/  |______     ____  __ __  ______
//      \_____  \\   __\_  __ \__  \\   __\__  \   / ___\|  |  \/  ___/
//      /        \|  |  |  | \// __ \|  |  / __ \_/ /_/  >  |  /\___ |
//     /_______  /|__|  |__|  (____  /__| (____  /\___  /|____//____  >
//             \/                  \/          \//_____/            \/ 
//  ______________________                           ______________________
//			  T H E   W A R   B E G I N S
//	   Stratagus - A free fantasy real time strategy game engine
//
/**@name linedraw.c	-	The general linedraw functions. */
//
//	(c) Copyright 2000-2003 by Lutz Sammer, Stephan Rasenberg,
//	Jimmy Salmon, Nehal Mistry
//
//      This program is free software; you can redistribute it and/or modify
//      it under the terms of the GNU General Public License as published by
//      the Free Software Foundation; version 2 dated June, 1991.
//
//      This program is distributed in the hope that it will be useful,
//      but WITHOUT ANY WARRANTY; without even the implied warranty of
//      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//      GNU General Public License for more details.
//
//      You should have received a copy of the GNU General Public License
//      along with this program; if not, write to the Free Software
//      Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
//      02111-1307, USA.
//
//	$Id$

//@{

/*----------------------------------------------------------------------------
--	Includes
----------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "stratagus.h"
#include "video.h"

#include "intern_video.h"


/*----------------------------------------------------------------------------
--	Declarations
----------------------------------------------------------------------------*/

/**
**	Bitmask, denoting a postion left/right/above/below clip rectangle
**	(mainly used by VideoDrawLineClip)
*/
typedef enum {
   ClipCodeInside = 0,			/// Clipping inside rectangle
   ClipCodeAbove  = 1,			/// Clipping above rectangle
   ClipCodeBelow  = 2,			/// Clipping below rectangle
   ClipCodeLeft   = 4,			/// Clipping left rectangle
   ClipCodeRight  = 8			/// Clipping right rectangle
} ClipCode;

/*----------------------------------------------------------------------------
--	Variables
----------------------------------------------------------------------------*/

#ifdef USE_SDL_SURFACE
local SDL_Surface* PixelSurface;

// FIXME: comments
global void VideoDrawPixel(SDL_Color color, int x, int y);
global void VideoDrawTransPixel(SDL_Color color, int x, int y, unsigned char alpha);
global void VideoDrawPixelClip(SDL_Color color, int x, int y);
global void VideoDrawVLine(SDL_Color color, int x, int y, int width);
global void VideoDrawVLineClip(SDL_Color color, int x, int y, int width);
global void VideoDrawTransVLine(SDL_Color color, int x, int y,
    int height, unsigned char alpha);
global void VideoDrawHLine(SDL_Color color, int x, int y, int width);
global void VideoDrawHLineClip(SDL_Color color, int x, int y, int width);
global void VideoDrawTransHLine(SDL_Color color, int x, int y,
    int width, unsigned char alpha);
global void VideoDrawLine(SDL_Color color, int sx, int sy, int dx, int dy);
global void VideoDrawTransLine(SDL_Color color, int sx, int sy,
    int dx, int dy, unsigned char alpha);
global void VideoDrawRectangle(SDL_Color color, int x, int y,
    int w, int h);
global void VideoDrawRectangleClip(SDL_Color color, int x, int y,
    int w, int h);
global void VideoDrawTransRectangle(SDL_Color color, int x, int y,
    int w, int h, unsigned char alpha);
global void VideoFillRectangle(SDL_Color color, int x, int y,
    int w, int h);
global void VideoFillTransRectangle(SDL_Color color, int x, int y,
    int w, int h, unsigned char alpha);
#else
/**
**	Draw pixel unclipped.
**
**	@param color	color
**	@param x	x coordinate on the screen
**	@param y	y coordinate on the screen
*/
global void (*VideoDrawPixel)(VMemType color, int x, int y);

/**
**	Draw 25% translucent pixel (Alpha = 64) unclipped.
**
**	@param color	color
**	@param x	x coordinate on the screen
**	@param y	y coordinate on the screen
*/
global void (*VideoDraw25TransPixel)(VMemType color, int x, int y);

/**
**	Draw 50% translucent pixel (Alpha = 128) unclipped.
**
**	@param color	color
**	@param x	x coordinate on the screen
**	@param y	y coordinate on the screen
*/
global void (*VideoDraw50TransPixel)(VMemType color, int x, int y);

/**
**	Draw 75% translucent pixel (Alpha = 192) unclipped.
**
**	@param color	color
**	@param x	x coordinate on the screen
**	@param y	y coordinate on the screen
*/
global void (*VideoDraw75TransPixel)(VMemType color, int x, int y);

/**
**	Draw translucent pixel unclipped.
**
**  All upfollowing translucent routines follow:
**  newcolor = ((oldcolor-color) * alpha)/255+color
**
**  FIXME: the following delivers better rounding, but is slower and can
**         not be calculate in an unsigned char (can deliver overflow):
**         newcolor = (oldcolor*alpha+color*(255 - alpha)+127)/255
**
**	@param color	color
**	@param x	x coordinate on the screen
**	@param y	y coordinate on the screen
**	@param alpha	alpha value of pixel.
*/
global void (*VideoDrawTransPixel)(VMemType color, int x, int y,
    unsigned char alpha);

/**
**	Draw pixel clipped to current clip setting.
**
**	@param color	color
**	@param x	x coordinate on the screen
**	@param y	y coordinate on the screen
*/
global void (*VideoDrawPixelClip)(VMemType color, int x, int y);

/**
**	Draw vertical line unclipped.
**
**	@param color	color
**	@param x	x coordinate on the screen
**	@param y	y coordinate on the screen
**	@param height	height of line.
*/
global void (*VideoDrawVLine)(VMemType color, int x, int y, int height);

/**
**	Draw 25% translucent vertical line unclipped.
**
**	@param color	color
**	@param x	x coordinate on the screen
**	@param y	y coordinate on the screen
**	@param height	height of line.
*/
global void (*VideoDraw25TransVLine)(VMemType color, int x, int y, int height);

/**
**	Draw 50% translucent vertical line unclipped.
**
**	@param color	color
**	@param x	x coordinate on the screen
**	@param y	y coordinate on the screen
**	@param height	height of line.
*/
global void (*VideoDraw50TransVLine)(VMemType color, int x, int y, int height);

/**
**	Draw 75% translucent vertical line unclipped.
**
**	@param color	color
**	@param x	x coordinate on the screen
**	@param y	y coordinate on the screen
**	@param height	height of line.
*/
global void (*VideoDraw75TransVLine)(VMemType color, int x, int y, int height);

/**
**	Draw translucent vertical line unclipped.
**
**	@param color	color
**	@param x	x coordinate on the screen
**	@param y	y coordinate on the screen
**	@param height	height of line.
**	@param alpha	alpha value of pixel.
*/
global void (*VideoDrawTransVLine)(VMemType color, int x, int y,
    int height, unsigned char alpha);

/**
**	Draw horizontal line unclipped.
**
**	@param color	color
**	@param x	x coordinate on the screen
**	@param y	y coordinate on the screen
**	@param width	width of line.
*/
global void (*VideoDrawHLine)(VMemType color, int x, int y, int width);

/**
**	Draw 25% translucent horizontal line unclipped.
**
**	@param color	color
**	@param x	x coordinate on the screen
**	@param y	y coordinate on the screen
**	@param width	width of line.
*/
global void (*VideoDraw25TransHLine)(VMemType color, int x, int y, int width);

/**
**	Draw 50% translucent horizontal line unclipped.
**
**	@param color	color
**	@param x	x coordinate on the screen
**	@param y	y coordinate on the screen
**	@param width	width of line.
*/
global void (*VideoDraw50TransHLine)(VMemType color, int x, int y, int width);

/**
**	Draw 75% translucent horizontal line unclipped.
**
**	@param color	color
**	@param x	x coordinate on the screen
**	@param y	y coordinate on the screen
**	@param width	width of line.
*/
global void (*VideoDraw75TransHLine)(VMemType color, int x, int y, int width);

/**
**	Draw translucent horizontal line unclipped.
**
**	@param color	color
**	@param x	x coordinate on the screen
**	@param y	y coordinate on the screen
**	@param width	width of line.
**	@param alpha	alpha value of pixel.
*/
global void (*VideoDrawTransHLine)(VMemType color, int x, int y,
    int width, unsigned char alpha);

/**
**	Draw line unclipped.
**
**	@param color	color
**	@param sx	Source x coordinate on the screen
**	@param sy	Source y coordinate on the screen
**	@param dx	Destination x coordinate on the screen
**	@param dy	Destination y coordinate on the screen
*/
global void (*VideoDrawLine)(VMemType color, int sx, int sy, int dx, int dy);

/**
**	Draw 25% translucent line unclipped.
**
**	@param color	color
**	@param sx	Source x coordinate on the screen
**	@param sy	Source y coordinate on the screen
**	@param dx	Destination x coordinate on the screen
**	@param dy	Destination y coordinate on the screen
*/
global void (*VideoDraw25TransLine)(VMemType color, int sx, int sy,
    int dx, int dy);

/**
**	Draw 50% translucent line unclipped.
**
**	@param color	color
**	@param sx	Source x coordinate on the screen
**	@param sy	Source y coordinate on the screen
**	@param dx	Destination x coordinate on the screen
**	@param dy	Destination y coordinate on the screen
*/
global void (*VideoDraw50TransLine)(VMemType color, int sx, int sy,
    int dx, int dy);

/**
**	Draw 75% translucent line unclipped.
**
**	@param color	color
**	@param sx	Source x coordinate on the screen
**	@param sy	Source y coordinate on the screen
**	@param dx	Destination x coordinate on the screen
**	@param dy	Destination y coordinate on the screen
*/
global void (*VideoDraw75TransLine)(VMemType color, int sx, int sy,
    int dx, int dy);

/**
**	Draw translucent line unclipped.
**
**	@param color	color
**	@param sx	Source x coordinate on the screen
**	@param sy	Source y coordinate on the screen
**	@param dx	Destination x coordinate on the screen
**	@param dy	Destination y coordinate on the screen
**	@param alpha	alpha value of pixel.
*/
global void (*VideoDrawTransLine)(VMemType color, int sx, int sy,
    int dx, int dy, unsigned char alpha);

/**
**	Draw rectangle.
**
**	@param color	color
**	@param x	x coordinate on the screen
**	@param y	y coordinate on the screen
**	@param h	height of rectangle.
**	@param w	width of rectangle.
*/
global void (*VideoDrawRectangle)(VMemType color, int x, int y,
    int w, int h);

/**
**	Draw 25% translucent rectangle.
**
**	@param color	color
**	@param x	x coordinate on the screen
**	@param y	y coordinate on the screen
**	@param h	height of rectangle.
**	@param w	width of rectangle.
*/
global void (*VideoDraw25TransRectangle)(VMemType color, int x, int y,
    int w, int h);

/**
**	Draw 50% translucent rectangle.
**
**	@param color	color
**	@param x	x coordinate on the screen
**	@param y	y coordinate on the screen
**	@param h	height of rectangle.
**	@param w	width of rectangle.
*/
global void (*VideoDraw50TransRectangle)(VMemType color, int x, int y,
    int w, int h);

/**
**	Draw 75% translucent rectangle.
**
**	@param color	color
**	@param x	x coordinate on the screen
**	@param y	y coordinate on the screen
**	@param h	height of rectangle.
**	@param w	width of rectangle.
*/
global void (*VideoDraw75TransRectangle)(VMemType color, int x, int y,
    int w, int h);

/**
**	Draw translucent rectangle.
**
**	@param color	color
**	@param x	x coordinate on the screen
**	@param y	y coordinate on the screen
**	@param h	height of rectangle.
**	@param w	width of rectangle.
**	@param alpha	alpha value of pixel.
*/
global void (*VideoDrawTransRectangle)(VMemType color, int x, int y,
    int w, int h, unsigned char alpha);

/**
**	Fill rectangle.
**
**	@param color	color
**	@param x	x coordinate on the screen
**	@param y	y coordinate on the screen
**	@param h	height of rectangle.
**	@param w	width of rectangle.
*/
global void (*VideoFillRectangle)(VMemType color, int x, int y,
    int w, int h);

/**
**	Draw 25% translucent filled rectangle.
**
**	@param color	color
**	@param x	x coordinate on the screen
**	@param y	y coordinate on the screen
**	@param h	height of rectangle.
**	@param w	width of rectangle.
*/
global void (*VideoFill25TransRectangle)(VMemType color, int x, int y,
    int w, int h);

/**
**	Draw 50% translucent filled rectangle.
**
**	@param color	color
**	@param x	x coordinate on the screen
**	@param y	y coordinate on the screen
**	@param h	height of rectangle.
**	@param w	width of rectangle.
*/
global void (*VideoFill50TransRectangle)(VMemType color, int x, int y,
    int w, int h);

/**
**	Draw 75% translucent filled rectangle.
**
**	@param color	color
**	@param x	x coordinate on the screen
**	@param y	y coordinate on the screen
**	@param h	height of rectangle.
**	@param w	width of rectangle.
*/
global void (*VideoFill75TransRectangle)(VMemType color, int x, int y,
    int w, int h);

/**
**	Draw translucent filled rectangle.
**
**	@param color	color
**	@param x	x coordinate on the screen
**	@param y	y coordinate on the screen
**	@param h	height of rectangle.
**	@param w	width of rectangle.
**	@param alpha	alpha value of pixel.
*/
global void (*VideoFillTransRectangle)(VMemType color, int x, int y,
    int w, int h, unsigned char alpha);
#endif

// ===========================================================================
//	Pixel
// ===========================================================================

#ifdef USE_SDL_SURFACE
    // FIXME: BIG todo
    // FIXME: optimize all these
global void InitLineDraw()
{
    SDL_Surface* s;

    s = SDL_CreateRGBSurface(SDL_SWSURFACE, 1, 1, 32, 
	RMASK, GMASK, BMASK, AMASK);
    PixelSurface = SDL_DisplayFormatAlpha(s);
    SDL_FreeSurface(s);
}

global void VideoDrawPixel(SDL_Color color, int x, int y)
{
    int bpp;
    int ofs;
    unsigned int c;

    c = SDL_MapRGB(TheScreen->format, color.r, color.g, color.b);
    bpp = TheScreen->format->BytesPerPixel;
    ofs = TheScreen->pitch * y + x * bpp;

    SDL_LockSurface(TheScreen);
    memcpy((char*)TheScreen->pixels + ofs, &c, bpp);
    SDL_UnlockSurface(TheScreen);
}

global void VideoDrawTransPixel(SDL_Color color, int x, int y, unsigned char alpha)
{
//    int bpp;
//    int ofs;
//    unsigned int c;
    SDL_Rect drect;

    drect.x = x;
    drect.y = y;

    SDL_FillRect(PixelSurface, NULL, SDL_MapRGBA(PixelSurface->format, 
	color.r, color.g, color.b, alpha));
    SDL_BlitSurface(PixelSurface, NULL, TheScreen, &drect);
/*
    c = SDL_MapRGBA(TheScreen->format, color.r, color.g, color.b, alpha);
    bpp = TheScreen->format->BytesPerPixel;
    ofs = TheScreen->pitch * y + x * bpp;

    SDL_LockSurface(TheScreen);
    memcpy((char*)TheScreen->pixels + ofs, &c, bpp);
    SDL_UnlockSurface(TheScreen);
*/
}

global void VideoDrawPixelClip(SDL_Color color, int x, int y)
{
    int w;
    int h;

    w = h = 1;
    CLIP_RECTANGLE(x, y, w, h);
    VideoDrawPixel(color, x, y);
}

global void VideoDrawVLine(SDL_Color color, int x, int y, int height)
{
    int i;

    for (i = 0; i < height; ++i) {
	VideoDrawPixel(color, x, y + i);
    }
}

global void VideoDrawTransVLine(SDL_Color color, int x, int y,
    int height, unsigned char alpha)
{
    int i;

    for (i = 0; i < height; ++i) {
	VideoDrawTransPixel(color, x, y + i, alpha);
    }
}

global void VideoDrawVLineClip(SDL_Color color, int x, int y, int height)
{
    int w = 1;
    CLIP_RECTANGLE(x, y, w, height);
    VideoDrawVLine(color, x, y, height);
}

global void VideoDrawHLine(SDL_Color color, int x, int y, int width)
{
    int i;

    for (i = 0; i < width; ++i) {
	VideoDrawPixel(color, x + i, y);
    }
}

global void VideoDrawHLineClip(SDL_Color color, int x, int y, int width)
{
    int h = 1;
    CLIP_RECTANGLE(x, y, width, h);
    VideoDrawHLine(color, x, y, width);
}

global void VideoDrawTransHLine(SDL_Color color, int x, int y,
    int width, unsigned char alpha)
{
    int i;

    for (i = 0; i < width; ++i) {
	VideoDrawTransPixel(color, x + i, y, alpha);
    }
}

global void VideoDrawLine(SDL_Color color, int sx, int sy, int dx, int dy)
{
    int x;
    int y;
    int xlen;
    int ylen;
    int incr;

    if (sx == dx) {
	if (sy < dy) {
	    VideoDrawVLine(color, sx, sy, dy - sy + 1);
	} else {
	    VideoDrawVLine(color, dx, dy, sy - dy + 1);
	}
	return;
    }

    if (sy == dy) {
	if (sx < dx) {
	    VideoDrawHLine(color, sx, sy, dx - sx + 1);
	} else {
	    VideoDrawHLine(color, dx, dy, sx - dx + 1);
	}
	return;
    }

    // exchange coordinates
    if (sy > dy) {
	int t;
	t = dx;
	dx = sx;
	sx = t;
	t = dy;
	dy = sy;
	sy = t;
    }
    ylen = dy - sy;

    if (sx > dx) {
	xlen = sx - dx;
	incr = -1;
    } else {
	xlen = dx - sx;
	incr = 1;
    }

    y = sy;
    x = sx;

    if (xlen > ylen) {
	int p;

	if (sx > dx) {
	    int t;
	    t = sx;
	    sx = dx;
	    dx = t;
	    y = dy;
	}

	p = (ylen << 1) - xlen;

	for (x = sx; x < dx; ++x) {
	    VideoDrawPixel(color, x, y);
	    if (p >= 0) {
		y += incr;
		p += (ylen - xlen) << 1;
	    } else {
		p += (ylen << 1);
	    }
	}
	return;
    }

    if (ylen > xlen) {
	int p;

	p = (xlen << 1) - ylen;

	for (y = sy; y < dy; ++y) {
	    VideoDrawPixel(color, x, y);
	    if (p >= 0) {
		x += incr;
		p += (xlen - ylen) << 1;
	    } else {
		p += (xlen << 1);
	    }
	}
	return;
    }

    // Draw a diagonal line
    if (ylen == xlen) {
	while (y != dy) {
	    VideoDrawPixel(color, x, y);
	    x += incr;
	    ++y;
	}
    }
}

global void VideoDrawLineClip(SDL_Color color, int sx, int sy, int dx, int dy)
{
    int w;
    int h;

    // FIXME: messy
    if (dx > sx && dy > sy) {
	w = dx - sx;
	h = dy - sy;
	CLIP_RECTANGLE(sx, sy, w, h);
	dx = sx + w;
	dy = sy + h;
    } else if (dx > sx && dy < sy) {
	w = dx - sx;
	h = sy - dy;
	CLIP_RECTANGLE(sx, dy, w, h);
	dx = sx + w;
	sy = dy + h;
    } else if (dx < sx && dy > sy) {
	w = sx - dx;
	h = dy - sy;
	CLIP_RECTANGLE(dx, sy, w, h);
	sx = dx + w;
	dy = sy + h;
    } else if (dx < sx && dy < sy) {
	w = sx - dx;
	h = sy - dy;
	CLIP_RECTANGLE(dx, dy, w, h);
	sx = dx + w;
	sy = dy + h;
    }

    VideoDrawLine(color, sx, sy, dx, dy);
}

global void VideoDrawTransLine(SDL_Color color, int sx, int sy,
    int dx, int dy, unsigned char alpha)
{
    // FIXME: trans
    VideoDrawLine(color, sx, sy, dx, dy);
}

global void VideoDrawRectangle(SDL_Color color, int x, int y,
    int w, int h)
{
    VideoDrawHLine(color, x, y, w);
    VideoDrawHLine(color, x, y + h - 1, w);

    VideoDrawVLine(color, x, y + 1, h - 2);
    VideoDrawVLine(color, x + w - 1, y + 1, h - 2);
}

global void VideoDrawRectangleClip(SDL_Color color, int x, int y,
    int w, int h)
{
    CLIP_RECTANGLE(x, y, w, h);
    VideoDrawRectangle(color, x, y, w, h);
}

global void VideoDrawTransRectangle(SDL_Color color, int x, int y,
    int w, int h, unsigned char alpha)
{
    VideoDrawTransHLine(color, x, y, w, alpha);
    VideoDrawTransHLine(color, x, y + h - 1, w, alpha);

    VideoDrawTransVLine(color, x, y + 1, h - 2, alpha);
    VideoDrawTransVLine(color, x + w - 1, y + 1, h - 2, alpha);
}

global void VideoFillRectangle(SDL_Color color, int x, int y,
    int w, int h)
{
    SDL_Rect drect;
    Uint32 c = SDL_MapRGB(TheScreen->format, color.r, color.g, color.b);

    drect.x = x;
    drect.y = y;
    drect.w = w;
    drect.h = h;

    SDL_FillRect(TheScreen, &drect, c);
}

global void VideoFillRectangleClip(SDL_Color color, int x, int y,
    int w, int h)
{
    CLIP_RECTANGLE(x, y, w, h);
    VideoFillRectangle(color, x, y, w, h);
}

global void VideoFillTransRectangle(SDL_Color color, int x, int y,
    int w, int h, unsigned char alpha)
{
    SDL_Rect drect;
    SDL_Surface* s;
    Uint32 c;

    s = SDL_CreateRGBSurface(SDL_SWSURFACE, w, h,
	32, RMASK, GMASK, BMASK, AMASK);

    c = SDL_MapRGBA(s->format, color.r, color.g, color.b, alpha);

    SDL_FillRect(s, NULL, c);

    drect.x = x;
    drect.y = y;

    SDL_BlitSurface(s, NULL, TheScreen, &drect);
    SDL_FreeSurface(s);
}

global void VideoFillTransRectangleClip(SDL_Color color, int x, int y,
    int w, int h, unsigned char alpha)
{
    CLIP_RECTANGLE(x, y, w, h);
    VideoFillTransRectangle(color, x, y, w, h, alpha);
}

global void VideoDrawCircle(SDL_Color color, int x, int y, int r)
{
    int p;
    int px;
    int py;

    p = 1 - r;
    py = r;

    for (px = 0; px <= py + 1; ++px) {
	VideoDrawPixel(color, x + px, y + py);
	VideoDrawPixel(color, x + px, y - py);
	VideoDrawPixel(color, x - px, y + py);
	VideoDrawPixel(color, x - px, y - py);

	VideoDrawPixel(color, x + py, y + px);
	VideoDrawPixel(color, x + py, y - px);
	VideoDrawPixel(color, x - py, y + px);
	VideoDrawPixel(color, x - py, y - px);

	if (p < 0) {
	    p += 2 * px + 3;
	} else {
	    p += 2 * (px - py) + 5;
	    py -= 1;
	}
    }
}

global void VideoDrawCircleClip(SDL_Color color, int x, int y, int r)
{
    int w;
    int h;

    w = h = r * 2;

    CLIP_RECTANGLE(x, y, w, h);
    VideoDrawCircle(color, x, y, r);

    r = w / 2;
    h = w / 2;
}

global void VideoDrawTransCircleClip(SDL_Color color, int x, int y,
    int r, unsigned char alpha)
{
    // FIXME: clip, trans
    VideoDrawCircle(color, x, y, r);
}

global void VideoFillCircle(SDL_Color color, int x, int y, int r)
{
    int p;
    int px;
    int py;

    p = 1 - r;
    py = r;

    for (px = 0; px <= py; ++px) {

	// Fill up the middle half of the circle
	VideoDrawVLine(color, x + px, y, py + 1);
        VideoDrawVLine(color, x + px, y - py, py);
	if (px) {
	    VideoDrawVLine(color, x - px, y, py + 1);
	    VideoDrawVLine(color, x - px, y - py, py);
	}

	if (p < 0) {
	    p += 2 * px + 3;
	} else {
	    p += 2 * (px - py) + 5;
	    py -= 1;

	    // Fill up the left/right half of the circle
	    if (py > px) {
		VideoDrawVLine(color, x + py + 1, y, px + 1);
		VideoDrawVLine(color, x + py + 1, y - px, px);
		VideoDrawVLine(color, x - py - 1, y, px + 1);
		VideoDrawVLine(color, x - py - 1, y - px,  px);
	    }
	}
    }
}

global void VideoFillTransCircle(SDL_Color color, int x, int y, 
    int r, unsigned char alpha)
{
    int p;
    int px;
    int py;

    p = 1 - r;
    py = r;

    for (px = 0; px <= py + 1; ++px) {

	// Fill up the middle half of the circle
	VideoDrawTransVLine(color, x + px, y, py + 1, alpha);
        VideoDrawTransVLine(color, x + px, y - py, py, alpha);
	if (px) {
	    VideoDrawTransVLine(color, x - px, y, py + 1, alpha);
	    VideoDrawTransVLine(color, x - px, y - py, py, alpha);
	}

	if (p < 0) {
	    p += 2 * px + 3;
	} else {
	    p += 2 * (px - py) + 5;
	    py -= 1;

	    // Fill up the left/right half of the circle
	    if (py > px) {
		VideoDrawTransVLine(color, x + py + 1, y, px + 1, alpha);
		VideoDrawTransVLine(color, x + py + 1, y - px, px, alpha);
		VideoDrawTransVLine(color, x - py - 1, y, px + 1, alpha);
		VideoDrawTransVLine(color, x - py - 1, y - px,  px, alpha);
	    }
	}
    }
}

global void VideoFillCircleClip(SDL_Color color, int x, int y, int r)
{
    int w;
    int h;

    w = h = r * 2;

    CLIP_RECTANGLE(x, y, w, h);
    r = w / 2;
    VideoFillCircle(color, x, y, r);
}

global void VideoFillTransCircleClip(SDL_Color color, int x, int y,
    int r, unsigned char alpha)
{
    int w;
    int h;

    w = h = r * 2;

    CLIP_RECTANGLE(x, y, w, h);
    r = w / 2;
    VideoFillTransCircle(color, x, y, r, alpha);
}

global void DebugTestDisplayLines(void)
{
//    DebugCheck(1);
}
#else
/**
**	Draw pixel unclipped into 8bit framebuffer.
**
**	@param color	color
**	@param x	x coordinate on the screen
**	@param y	y coordinate on the screen
*/
local void DrawPixel8(VMemType color, int x, int y)
{
    VideoMemory8[x + y * VideoWidth] = color.D8;
}

/**
**	Draw pixel unclipped into 16bit framebuffer.
**
**	@param color	color
**	@param x	x coordinate on the screen
**	@param y	y coordinate on the screen
*/
local void DrawPixel16(VMemType color, int x, int y)
{
    VideoMemory16[x + y * VideoWidth] = color.D16;
}

/**
**	Draw pixel unclipped into 24bit framebuffer.
**
**	@param color	color
**	@param x	x coordinate on the screen
**	@param y	y coordinate on the screen
*/
local void DrawPixel24(VMemType color, int x, int y)
{
    VideoMemory24[x + y * VideoWidth] = color.D24;
}

/**
**	Draw pixel unclipped into 32bit framebuffer.
**
**	@param color	color
**	@param x	x coordinate on the screen
**	@param y	y coordinate on the screen
*/
local void DrawPixel32(VMemType color, int x, int y)
{
    VideoMemory32[x + y * VideoWidth] = color.D32;
}

/**
**	Draw pixel unclipped.
**
**	@param color	color
**	@param x	x coordinate on the screen
**	@param y	y coordinate on the screen
*/
#ifdef USE_OPENGL
local void DrawPixelOpenGL(VMemType color, int x, int y)
{
    VMemType32 c;
    GLubyte r;
    GLubyte g;
    GLubyte b;

    c = color.D32;
    r = (c >> 16) & 0xff;
    g = (c >> 8) & 0xff;
    b = (c >> 0) & 0xff;
    glDisable(GL_TEXTURE_2D);
    glColor3ub(r, g, b);
    glBegin(GL_POINTS);
    glVertex2i(x, VideoHeight - y);
    glEnd();
    glEnable(GL_TEXTURE_2D);
}
#endif

/**
**	Draw 25% translucent pixel unclipped into 8bit framebuffer.
**
**	@param color	color
**	@param x	x coordinate on the screen
**	@param y	y coordinate on the screen
*/
local void Draw25TransPixel8(VMemType color, int x, int y)
{
    VMemType8* p;

    p = VideoMemory8 + x + y * VideoWidth;
    *p = lookup25trans8[(color.D8 << 8) | *p];
}

/**
**	Draw 25% translucent pixel unclipped into 15bit framebuffer.
**
**	@param color	color
**	@param x	x coordinate on the screen
**	@param y	y coordinate on the screen
*/
local void Draw25TransPixel15(VMemType color, int x, int y)
{
    VMemType16* p;
    unsigned long sp;
    unsigned long dp;

    p = VideoMemory16 + x + y * VideoWidth;
    sp = color.D16;
    // FIXME: pre multiply?
    sp = (((sp << 16) | sp) & 0x03E07C1F) * 3;
    dp = *p;
    dp = ((dp << 16) | dp) & 0x03E07C1F;
    dp = ((dp + sp) >> 2) & 0x03E07C1F;
    *p = (dp >> 16) | dp;
}

/**
**	Draw 25% translucent pixel unclipped into 16bit framebuffer.
**
**	@param color	color
**	@param x	x coordinate on the screen
**	@param y	y coordinate on the screen
*/
local void Draw25TransPixel16(VMemType color, int x, int y)
{
    VMemType16* p;
    unsigned long sp;
    unsigned long dp;

    p = VideoMemory16 + x + y * VideoWidth;
    sp = color.D16;
    // FIXME: pre multiply?
    sp = (((sp << 16) | sp) & 0x07E0F81F) * 3;
    dp = *p;
    dp = ((dp << 16) | dp) & 0x07E0F81F;
    dp = ((dp + sp) >> 2) & 0x07E0F81F;
    *p = (dp >> 16) | dp;
}

/**
**	Draw 25% translucent pixel unclipped into 24bit framebuffer.
**
**	@param color	color
**	@param x	x coordinate on the screen
**	@param y	y coordinate on the screen
*/
local void Draw25TransPixel24(VMemType color, int x, int y)
{
//FIXME: does 24bpp represents R|G|B?
    VideoMemory24[x + y * VideoWidth] = color.D24;
}

/**
**	Draw 25% translucent pixel unclipped into 32bit framebuffer.
**
**	@param color	color
**	@param x	x coordinate on the screen
**	@param y	y coordinate on the screen
*/
local void Draw25TransPixel32(VMemType color, int x, int y)
{
    VMemType32* p;
    unsigned long sp1;
    unsigned long sp2;
    unsigned long dp1;
    unsigned long dp2;

    sp1 = color.D32;
    // FIXME: pre multiply?
    sp2 = ((sp1 & 0xFF00FF00) >> 8) * 3;
    sp1 = (sp1 & 0x00FF00FF) * 3;

    p = VideoMemory32 + y * VideoWidth + x;
    dp1 = *p;
    dp2 = (dp1 & 0xFF00FF00) >> 8;
    dp1 &= 0x00FF00FF;

    dp1 = ((dp1 + sp1) >> 2) & 0x00FF00FF;
    dp2 = ((dp2 + sp2) >> 2) & 0x00FF00FF;
    *p = (dp1 | (dp2 << 8));
}

/**
**	Draw 25% translucent pixel unclipped.
**
**	@param color	color
**	@param x	x coordinate on the screen
**	@param y	y coordinate on the screen
*/
#ifdef USE_OPENGL
local void Draw25TransPixelOpenGL(VMemType color, int x, int y)
{
    VMemType32 c;
    GLubyte r;
    GLubyte g;
    GLubyte b;

    c = color.D32;
    r = (c >> 16) & 0xff;
    g = (c >> 8) & 0xff;
    b = (c >> 0) & 0xff;
    glDisable(GL_TEXTURE_2D);
    glColor4ub(r, g, b, 192);
    glBegin(GL_POINTS);
    glVertex2i(x, VideoHeight - y);
    glEnd();
    glEnable(GL_TEXTURE_2D);
}
#endif

/**
**	Draw 50% translucent pixel unclipped into 8bit framebuffer.
**
**	@param color	color
**	@param x	x coordinate on the screen
**	@param y	y coordinate on the screen
*/
local void Draw50TransPixel8(VMemType color, int x, int y)
{
    VMemType8* p;

    p = VideoMemory8 + x + y * VideoWidth;
    *p = lookup50trans8[(color.D8 << 8) | *p];
}

/**
**	Draw 50% translucent pixel unclipped into 15bit framebuffer.
**
**	@param color	color
**	@param x	x coordinate on the screen
**	@param y	y coordinate on the screen
*/
local void Draw50TransPixel15(VMemType color, int x, int y)
{
    VMemType16* p;
    unsigned long sp;
    unsigned long dp;

    p = VideoMemory16 + x + y * VideoWidth;
    sp = color.D16;
    sp = ((sp << 16) | sp) & 0x03E07C1F;
    dp = *p;
    dp = ((dp << 16) | dp) & 0x03E07C1F;
    dp = ((dp + sp) >> 1) & 0x03E07C1F;
    *p = (dp >> 16) | dp;
}

/**
**	Draw 50% translucent pixel unclipped into 16bit framebuffer.
**
**	@param color	color
**	@param x	x coordinate on the screen
**	@param y	y coordinate on the screen
*/
local void Draw50TransPixel16(VMemType color, int x, int y)
{
    VMemType16* p;
    unsigned long sp;
    unsigned long dp;

    p = VideoMemory16 + x + y * VideoWidth;
    sp = color.D16;
    sp = ((sp << 16) | sp) & 0x07E0F81F;
    dp = *p;
    dp = ((dp << 16) | dp) & 0x07E0F81F;
    dp = ((dp + sp) >> 1) & 0x07E0F81F;
    *p = (dp >> 16) | dp;
}

/**
**	Draw 50% translucent pixel unclipped into 24bit framebuffer.
**
**	@param color	color
**	@param x	x coordinate on the screen
**	@param y	y coordinate on the screen
*/
local void Draw50TransPixel24(VMemType color, int x, int y)
{
//FIXME: does 24bpp represents R|G|B?
    VideoMemory24[x + y * VideoWidth] = color.D24;
}

/**
**	Draw 50% translucent pixel unclipped into 32bit framebuffer.
**
**	@param color	color
**	@param x	x coordinate on the screen
**	@param y	y coordinate on the screen
*/
local void Draw50TransPixel32(VMemType color, int x, int y)
{
    VMemType32* p;
    unsigned long sp1;
    unsigned long sp2;
    unsigned long dp1;
    unsigned long dp2;

    sp1 = color.D32;
    sp2 = (sp1 & 0xFF00FF00) >> 8;
    sp1 &= 0x00FF00FF;

    p = VideoMemory32 + y * VideoWidth + x;
    dp1 = *p;
    dp2 = (dp1 & 0xFF00FF00) >> 8;
    dp1 &= 0x00FF00FF;

    dp1 = ((dp1 + sp1) >> 1) & 0x00FF00FF;
    dp2 = ((dp2 + sp2) >> 1) & 0x00FF00FF;
    *p = (dp1 | (dp2 << 8));
}

/**
**	Draw 50% translucent pixel unclipped.
**
**	@param color	color
**	@param x	x coordinate on the screen
**	@param y	y coordinate on the screen
*/
#ifdef USE_OPENGL
local void Draw50TransPixelOpenGL(VMemType color, int x, int y)
{
    VMemType32 c;
    GLubyte r;
    GLubyte g;
    GLubyte b;

    c = color.D32;
    r = (c >> 16) & 0xff;
    g = (c >> 8) & 0xff;
    b = (c >> 0) & 0xff;
    glDisable(GL_TEXTURE_2D);
    glColor4ub(r, g, b, 128);
    glBegin(GL_POINTS);
    glVertex2i(x, VideoHeight - y);
    glEnd();
    glEnable(GL_TEXTURE_2D);
}
#endif

/**
**	Draw 75% translucent pixel unclipped into 8bit framebuffer.
**
**	@param color	color
**	@param x	x coordinate on the screen
**	@param y	y coordinate on the screen
*/
local void Draw75TransPixel8(VMemType color, int x, int y)
{
    VMemType8* p;

    p = VideoMemory8 + x + y * VideoWidth;
    *p = lookup25trans8[(*p << 8) | color.D8];
}

/**
**	Draw 75% translucent pixel unclipped into 15bit framebuffer.
**
**	@param color	color
**	@param x	x coordinate on the screen
**	@param y	y coordinate on the screen
*/
local void Draw75TransPixel15(VMemType color, int x, int y)
{
    VMemType16* p;
    unsigned long sp;
    unsigned long dp;

    p = VideoMemory16 + x + y * VideoWidth;
    sp = color.D16;
    sp = ((sp << 16) | sp) & 0x03E07C1F;
    dp = *p;
    dp = ((dp << 16) | dp) & 0x03E07C1F;
    dp = (((dp << 1) + dp + sp) >> 2) & 0x03E07C1F;
    *p = (dp >> 16) | dp;
}

/**
**	Draw 75% translucent pixel unclipped into 16bit framebuffer.
**
**	@param color	color
**	@param x	x coordinate on the screen
**	@param y	y coordinate on the screen
*/
local void Draw75TransPixel16(VMemType color, int x, int y)
{
    VMemType16* p;
    unsigned long sp;
    unsigned long dp;

    p = VideoMemory16 + x + y * VideoWidth;
    sp = color.D16;
    sp = ((sp << 16) | sp) & 0x07E0F81F;
    dp = *p;
    dp = ((dp << 16) | dp) & 0x07E0F81F;
    dp = (((dp << 1) + dp + sp) >> 2) & 0x07E0F81F;
    *p = (dp >> 16) | dp;
}

/**
**	Draw 75% translucent pixel unclipped into 24bit framebuffer.
**
**	@param color	color
**	@param x	x coordinate on the screen
**	@param y	y coordinate on the screen
*/
local void Draw75TransPixel24(VMemType color, int x, int y)
{
//FIXME: does 24bpp represents R|G|B?
    VideoMemory24[x + y * VideoWidth] = color.D24;
}

/**
**	Draw 75% translucent pixel unclipped into 32bit framebuffer.
**
**	@param color	color
**	@param x	x coordinate on the screen
**	@param y	y coordinate on the screen
*/
local void Draw75TransPixel32(VMemType color, int x, int y)
{
    VMemType32* p;
    unsigned long sp1;
    unsigned long sp2;
    unsigned long dp1;
    unsigned long dp2;

    sp1 = color.D32;
    sp2 = (sp1 & 0xFF00FF00) >> 8;
    sp1 &= 0x00FF00FF;

    p = VideoMemory32 + y * VideoWidth + x;
    dp1 = *p;
    dp2 = (dp1 & 0xFF00FF00) >> 8;
    dp1 &= 0x00FF00FF;

    dp1 = (((dp1 << 1) + dp1 + sp1) >> 2) & 0x00FF00FF;
    dp2 = (((dp2 << 1) + dp2 + sp2) >> 2) & 0x00FF00FF;
    *p = (dp1 | (dp2 << 8));
}

/**
**	Draw 75% translucent pixel unclipped.
**
**	@param color	color
**	@param x	x coordinate on the screen
**	@param y	y coordinate on the screen
*/
#ifdef USE_OPENGL
local void Draw75TransPixelOpenGL(VMemType color, int x, int y)
{
    VMemType32 c;
    GLubyte r;
    GLubyte g;
    GLubyte b;

    c = color.D32;
    r = (c >> 16) & 0xff;
    g = (c >> 8) & 0xff;
    b = (c >> 0) & 0xff;
    glDisable(GL_TEXTURE_2D);
    glColor4ub(r, g, b, 64);
    glBegin(GL_POINTS);
    glVertex2i(x, VideoHeight - y);
    glEnd();
    glEnable(GL_TEXTURE_2D);
}
#endif
#endif // ifdef USE_SDL_SURFACE

#ifdef USE_SDL_SURFACE

#else
/**
**	Draw translucent pixel unclipped into 8bit framebuffer.
**
**	@param color	color
**	@param x	x coordinate on the screen
**	@param y	y coordinate on the screen
**      @param alpha    alpha value of pixel.
*/
local void DrawTransPixel8(VMemType color, int x, int y,
    unsigned char alpha)
{
    VMemType8* p;

    p = VideoMemory8 + x + y * VideoWidth;
    switch (((unsigned int)alpha * 4) / 255) {
	case 0:
	    *p = color.D8;
	    break;
	case 1:
	    *p = lookup25trans8[(color.D8 << 8) | *p];
	    break;
	case 2:
	    *p = lookup50trans8[(*p << 8) | color.D8];
	    break;
	case 3:
	    *p = lookup25trans8[(*p << 8) | color.D8];
	    break;
	default:
	    break;
    }
}

/**
**	Draw pixel unclipped into 8bit framebuffer (ignoring alpha).
**
**	@param color	color
**	@param x	x coordinate on the screen
**	@param y	y coordinate on the screen
**      @param alpha    alpha value of pixel.
*/
local void DrawNoTransPixel8(VMemType color, int x, int y,
    unsigned char alpha __attribute__((unused)))
{
    DrawPixel8(color, x, y);
}

/**
**	Draw translucent pixel unclipped into 15bit framebuffer.
**
**	@param color	color
**	@param x	x coordinate on the screen
**	@param y	y coordinate on the screen
**      @param alpha    alpha value of pixel.
**
** For 15bit |-RRRRRGGGGGBBBBB|, we need for each 5bit offically:
**   (5bit - 5bit) * 8bit alpha = 14bit signed int
**
** But Lutz has a smarter way, all in one unsigned 32bit:
**    color = |------GGGGG-----RRRRR------BBBBB|
**  c1 - c2 = |-SSSSSGGGGGSSSSSRRRRR-SSSSSBBBBB|
**  * alpha = |-GGGGGGGGGGSRRRRRRRRR-SBBBBBBBBB|
** newcolor = |------GGGGG-----RRRRR------BBBBB|
*/
local void DrawTransPixel15(VMemType color, int x, int y,
    unsigned char alpha)
{
    VMemType16* p;
    unsigned long sp;
    unsigned long dp;

    p = VideoMemory16 + x + y * VideoWidth;
    sp = color.D16;
    sp = ((sp << 16) | sp) & 0x03E07C1F;
    alpha >>= 3; //FIXME: only 5bits available between colors
    dp = *p;
    dp = ((dp << 16) | dp) & 0x03E07C1F;
    dp = ((((dp - sp) * alpha) >> 5) + sp) & 0x03E07C1F; //FIXME: alpha==256 unreached
    *p = (dp >> 16) | dp;
}

/**
**	Draw translucent pixel unclipped into 16bit framebuffer.
**
**	@param color	color
**	@param x	x coordinate on the screen
**	@param y	y coordinate on the screen
**      @param alpha    alpha value of pixel.
**
** For 16bit |RRRRRGGGGGGBBBBB|, we need offically:
**   (5bit - 5bit) * 8bit alpha = 14bit signed int
**   (6bit - 6bit) * 8bit alpha = 15bit signed int
**
** But Lutz has a smarter way, all in one unsigned 32bit:
**    color = |-----GGGGGG-----RRRRR------BBBBB|
**  c1 - c2 = |SSSSSGGGGGGSSSSSRRRRRSSSSSSBBBBB|
**  * alpha = |SGGGGGGGGGGSRRRRRRRRRSBBBBBBBBBB|
** newcolor = |-----GGGGGG-----RRRRR------BBBBB|
*/
local void DrawTransPixel16(VMemType color, int x, int y,
    unsigned char alpha)
{
    VMemType16* p;
    unsigned long sp;
    unsigned long dp;

    p = VideoMemory16 + x + y * VideoWidth;
    sp = color.D16;
    sp = ((sp << 16) | sp) & 0x07E0F81F;
    alpha >>= 3; //FIXME: only 5bits available between colors
    dp = *p;
    dp = ((dp << 16) | dp) & 0x07E0F81F;
    dp = ((((dp - sp) * alpha) >> 5) + sp) & 0x07E0F81F; //FIXME: alpha==256 unreached
    *p = (dp >> 16) | dp;
}

/**
**	Draw translucent pixel unclipped into 24bit framebuffer.
**
**	@param color	color
**	@param x	x coordinate on the screen
**	@param y	y coordinate on the screen
**      @param alpha    alpha value of pixel.
*/
local void DrawTransPixel24(VMemType color, int x, int y,
    unsigned char alpha __attribute__((unused)))
{
//FIXME: does 24bpp represents R|G|B?
    VideoMemory24[x + y * VideoWidth] = color.D24;
}

/**
**	Draw translucent pixel unclipped into 32bit framebuffer.
**
**	@param color	color
**	@param x	x coordinate on the screen
**	@param y	y coordinate on the screen
**      @param alpha    alpha value of pixel.
**
** For 32bit |AAAAAAAARRRRRRRRGGGGGGGGBBBBBBBB|, we need offically:
**   (8bit - 8bit) * 8bit alpha = 17bit signed int
**
** a smarter way, all in two unsigned 32bit:
**  color(1)    = |--------RRRRRRRR--------BBBBBBBB|
**  c1(1)-c2(1) = |SSSSSSSSRRRRRRRRSSSSSSSSBBBBBBBB|
**    * alpha   = |SRRRRRRRRRRRRRRRSBBBBBBBBBBBBBBB|
** newcolor(1)  = |--------RRRRRRRR--------BBBBBBBB|
**
**  color(2)    = |--------AAAAAAAA--------GGGGGGGG|
**  c1(2)-c2(2) = |SSSSSSSSAAAAAAAASSSSSSSSGGGGGGGG|
**    * alpha   = |SAAAAAAAAAAAAAAASGGGGGGGGGGGGGGG|
**  newcolor(2) = |--------AAAAAAAA--------GGGGGGGG|
**
** FIXME: alpha blending the A-value of 32bit may not be needed.. always 0
*/
local void DrawTransPixel32(VMemType color, int x, int y,
    unsigned char alpha)
{
    VMemType32* p;
    unsigned long sp1;
    unsigned long sp2;
    unsigned long dp1;
    unsigned long dp2;

    sp1 = color.D32;
    sp2 = (sp1 & 0xFF00FF00) >> 8;
    sp1 &= 0x00FF00FF;

    p = VideoMemory32 + y * VideoWidth + x;
    dp1 = *p;
    dp2 = (dp1 & 0xFF00FF00) >> 8;
    dp1 &= 0x00FF00FF;

    //FIXME: alpha==256 unreached
    dp1 = ((((dp1-sp1) * alpha) >> 8) + sp1) & 0x00FF00FF;
    dp2 = ((((dp2-sp2) * alpha) >> 8) + sp2) & 0x00FF00FF;
    *p = (dp1 | (dp2 << 8));
}

/**
**	Draw translucent pixel unclipped.
**
**	@param color	color
**	@param x	x coordinate on the screen
**	@param y	y coordinate on the screen
**      @param alpha    alpha value of pixel.
*/
#ifdef USE_OPENGL
local void DrawTransPixelOpenGL(VMemType color, int x, int y,
    unsigned char alpha)
{
    VMemType32 c;
    GLubyte r;
    GLubyte g;
    GLubyte b;

    c = color.D32;
    r = (c >> 16) & 0xff;
    g = (c >> 8) & 0xff;
    b = (c >> 0) & 0xff;
    glDisable(GL_TEXTURE_2D);
    glColor4ub(r, g, b, 255 - alpha);
    glBegin(GL_POINTS);
    glVertex2i(x, VideoHeight - y);
    glEnd();
    glEnable(GL_TEXTURE_2D);
}
#endif

/**
**	Draw pixel clipped to current clip setting into 8bit framebuffer.
**
**	@param color	color
**	@param x	x coordinate on the screen
**	@param y	y coordinate on the screen
*/
local void DrawPixelClip8(VMemType color, int x, int y)
{
    //	Clipping:
    if (x < ClipX1 || x > ClipX2 || y < ClipY1 || y > ClipY2) {
	return;
    }
    VideoMemory8[x + y * VideoWidth] = color.D8;
}

/**
**	Draw pixel clipped to current clip setting into 16bit framebuffer.
**
**	@param color	color
**	@param x	x coordinate on the screen
**	@param y	y coordinate on the screen
*/
local void DrawPixelClip16(VMemType color, int x, int y)
{
    //	Clipping:
    if (x < ClipX1 || x > ClipX2 || y < ClipY1 || y > ClipY2) {
	return;
    }
    VideoMemory16[x + y * VideoWidth] = color.D16;
}

/**
**	Draw pixel clipped to current clip setting into 24bit framebuffer.
**
**	@param color	color
**	@param x	x coordinate on the screen
**	@param y	y coordinate on the screen
*/
local void DrawPixelClip24(VMemType color, int x, int y)
{
    //	Clipping:
    if (x < ClipX1 || x > ClipX2 || y < ClipY1 || y > ClipY2) {
	return;
    }
    VideoMemory24[x + y * VideoWidth] = color.D24;
}

/**
**	Draw pixel clipped to current clip setting into 32bit framebuffer.
**
**	@param color	color
**	@param x	x coordinate on the screen
**	@param y	y coordinate on the screen
*/
local void DrawPixelClip32(VMemType color, int x, int y)
{
    //	Clipping:
    if (x < ClipX1 || x > ClipX2 || y < ClipY1 || y > ClipY2) {
	return;
    }
    VideoMemory32[x + y * VideoWidth] = color.D32;
}

/**
**	Draw pixel clipped to current clip setting.
**
**	@param color	color
**	@param x	x coordinate on the screen
**	@param y	y coordinate on the screen
*/
#ifdef USE_OPENGL
local void DrawPixelClipOpenGL(VMemType color, int x, int y)
{
    VMemType32 c;
    GLubyte r;
    GLubyte g;
    GLubyte b;

    //	Clipping:
    if (x < ClipX1 || x > ClipX2 || y < ClipY1 || y > ClipY2) {
	return;
    }

    c = color.D32;
    r = (c >> 16) & 0xff;
    g = (c >> 8) & 0xff;
    b = (c >> 0) & 0xff;
    glDisable(GL_TEXTURE_2D);
    glColor3ub(r, g, b);
    glBegin(GL_POINTS);
    glVertex2i(x, VideoHeight - y);
    glEnd();
    glEnable(GL_TEXTURE_2D);
}
#endif

/**
**	Draw 25% translucent pixel clipped to current clip setting.
**
**	@param color	color
**	@param x	x coordinate on the screen
**	@param y	y coordinate on the screen
*/
global void VideoDraw25TransPixelClip(VMemType color, int x, int y)
{
    //	Clipping:
    if (x < ClipX1 || x > ClipX2 || y < ClipY1 || y > ClipY2) {
	return;
    }
    VideoDraw25TransPixel(color, x, y);
}

/**
**	Draw 50% translucent pixel clipped to current clip setting.
**
**	@param color	color
**	@param x	x coordinate on the screen
**	@param y	y coordinate on the screen
*/
global void VideoDraw50TransPixelClip(VMemType color, int x, int y)
{
    //	Clipping:
    if (x < ClipX1 || x > ClipX2 || y < ClipY1 || y > ClipY2) {
	return;
    }
    VideoDraw50TransPixel(color, x, y);
}

/**
**	Draw 75% translucent pixel clipped to current clip setting.
**
**	@param color	color
**	@param x	x coordinate on the screen
**	@param y	y coordinate on the screen
*/
global void VideoDraw75TransPixelClip(VMemType color, int x, int y)
{
    //	Clipping:
    if (x < ClipX1 || x > ClipX2 || y < ClipY1 || y > ClipY2) {
	return;
    }
    VideoDraw75TransPixel(color, x, y);
}

/**
**	Draw translucent pixel clipped to current clip setting.
**
**	@param color	color
**	@param x	x coordinate on the screen
**	@param y	y coordinate on the screen
**      @param alpha    alpha value of pixels.
*/
global void VideoDrawTransPixelClip(VMemType color, int x, int y,
    unsigned char alpha)
{
    //	Clipping:
    if (x < ClipX1 || x > ClipX2 || y < ClipY1 || y > ClipY2) {
	return;
    }
    VideoDrawTransPixel(color, x, y, alpha);
}

// ===========================================================================
//	Horizontal Line
// ===========================================================================

/**
**	Draw horizontal line unclipped into 8bit framebuffer.
**
**	@param color	color
**	@param x	x coordinate on the screen
**	@param y	y coordinate on the screen
**	@param width	width of line.
*/
local void DrawHLine8(VMemType color, int x, int y, int width)
{
    VMemType8* p;
    VMemType8* e;
    int w;
    unsigned f;

    w = VideoWidth;
    p = VideoMemory8 + y * w + x;
    e = p + width;
    f = color.D8;

    while (p < e) {			// FIXME: better!
	*p++ = f;
    }
}

/**
**	Draw horizontal line unclipped into 16bit framebuffer.
**
**	@param color	color
**	@param x	x coordinate on the screen
**	@param y	y coordinate on the screen
**	@param width	width of line (0=don't draw).
*/
local void DrawHLine16(VMemType color, int x, int y, int width)
{
    VMemType16* p;
    VMemType16* e;
    int w;
    unsigned long f;

    w = VideoWidth;
    p = VideoMemory16 + y * w + x;
    e = p + width-1;
    f = ((unsigned long)color.D16 << 16) | color.D16;

    while (p < e) {			// draw 2 pixels
	*((unsigned long*)p)++ = f;
    }

    if (p <= e) {
	*p = f;
    }
}

/**
**	Draw horizontal line unclipped into 24bit framebuffer.
**
**	@param color	color
**	@param x	x coordinate on the screen
**	@param y	y coordinate on the screen
**	@param width	width of line (0=don't draw).
*/
local void DrawHLine24(VMemType color, int x, int y, int width)
{
    VMemType24* p;
    VMemType24* e;
    int w;
    VMemType24 f;

    w = VideoWidth;
    p = VideoMemory24 + y * w + x;
    e = p + width;
    f = color.D24;

    while (p < e) {
	*p++ = f;
    }
}

/**
**	Draw horizontal line unclipped into 32bit framebuffer.
**
**	@param color	color
**	@param x	x coordinate on the screen
**	@param y	y coordinate on the screen
**	@param width	width of line (0=don't draw).
*/
local void DrawHLine32(VMemType color, int x, int y, int width)
{
    VMemType32* p;
    VMemType32* e;
    int w;
    unsigned long f;

    w = VideoWidth;
    p = VideoMemory32 + y * w + x;
    e = p + width;
    f = color.D32;

    while (p < e) {
	*p++ = f;
    }
}

/**
**	Draw horizontal line unclipped.
**
**	@param color	color
**	@param x	x coordinate on the screen
**	@param y	y coordinate on the screen
**	@param width	width of line (0=don't draw).
*/
#ifdef USE_OPENGL
local void DrawHLineOpenGL(VMemType color, int x, int y, int width)
{
    VMemType32 c;
    GLubyte r;
    GLubyte g;
    GLubyte b;

    c = color.D32;
    r = (c >> 16) & 0xff;
    g = (c >> 8) & 0xff;
    b = (c >> 0) & 0xff;
    glDisable(GL_TEXTURE_2D);
    glColor3ub(r, g, b);
    glBegin(GL_LINES);
    glVertex2i(x, VideoHeight - y);
    glVertex2i(x + width, VideoHeight - y);
    glEnd();
    glEnable(GL_TEXTURE_2D);
}
#endif

/**
**	Draw 25% translucent horizontal line unclipped into 8bit framebuffer.
**
**	@param color	color
**	@param x	x coordinate on the screen
**	@param y	y coordinate on the screen
**	@param width	width of line (0=don't draw).
*/
local void Draw25TransHLine8(VMemType color, int x, int y, int width)
{
    VMemType8* p;
    VMemType8* e;
    unsigned int c;

    p = VideoMemory8 + x + y * VideoWidth;
    e = p + width;
    c = color.D8 << 8;

    while (p < e) {
	*p = lookup25trans8[c | *p];
	++p;
    }
}

/**
**	Draw 25% translucent horizontal line unclipped into 15bit framebuffer.
**
**	@param color	color
**	@param x	x coordinate on the screen
**	@param y	y coordinate on the screen
**	@param width	width of line (0=don't draw).
*/
local void Draw25TransHLine15(VMemType color, int x, int y, int width)
{
    VMemType16* p;
    VMemType16* e;
    int w;
    unsigned long sp;

    w = VideoWidth;
    p = VideoMemory16 + y * w + x;
    e = p + width;
    sp = color.D16;
    // FIXME: pre multiply?
    sp = (((sp << 16) | sp) & 0x03E07C1F) * 3;

    while (p < e) {
	unsigned long dp;

	dp = *p;
	dp = ((dp << 16) | dp) & 0x03E07C1F;
	dp = ((dp + sp) >> 2) & 0x03E07C1F;
	*p++ = (dp >> 16) | dp;
    }
}

/**
**	Draw 25% translucent horizontal line unclipped into 16bit framebuffer.
**
**	@param color	color
**	@param x	x coordinate on the screen
**	@param y	y coordinate on the screen
**	@param width	width of line (0=don't draw).
*/
local void Draw25TransHLine16(VMemType color, int x, int y, int width)
{
    VMemType16* p;
    VMemType16* e;
    int w;
    unsigned long sp;

    w = VideoWidth;
    p = VideoMemory16 + y * w + x;
    e = p + width;
    sp = color.D16;
    // FIXME: pre multiply?
    sp = (((sp << 16) | sp) & 0x07E0F81F) * 3;

    while (p < e) {
	unsigned long dp;

	dp = *p;
	dp = ((dp << 16) | dp) & 0x07E0F81F;
	dp = ((dp + sp) >> 2) & 0x07E0F81F;
	*p++ = (dp >> 16) | dp;
    }
}

/**
**	Draw 25% translucent horizontal line unclipped into 24bit framebuffer.
**
**	@param color	color
**	@param x	x coordinate on the screen
**	@param y	y coordinate on the screen
**	@param width	width of line (0=don't draw).
*/
local void Draw25TransHLine24(VMemType color, int x, int y, int width)
{
// FIXME: does 24bpp holds R|G|B ?
    DrawHLine24(color, x, y, width); // no trans functionaility for the moment :(
}

/**
**	Draw 25% translucent horizontal line unclipped into 32bit framebuffer.
**
**	@param color	color
**	@param x	x coordinate on the screen
**	@param y	y coordinate on the screen
**	@param width	width of line (0=don't draw).
*/
local void Draw25TransHLine32(VMemType color, int x, int y, int width)
{
    VMemType32* p;
    VMemType32* e;
    unsigned long sp1;
    unsigned long sp2;
    int w;

    w = VideoWidth;
    p = VideoMemory32 + y * w + x;
    e = p + width;
    sp1 = color.D32;
    // FIXME: pre multiply?
    sp2 = ((sp1 & 0xFF00FF00) >> 8) * 3;
    sp1 = (sp1 & 0x00FF00FF) * 3;

    while (p < e) {
	unsigned long dp1;
	unsigned long dp2;

	dp1 = *p;
	dp2 = (dp1 & 0xFF00FF00) >> 8;
	dp1 &= 0x00FF00FF;

	dp1 = ((dp1 + sp1) >> 2) & 0x00FF00FF;
	dp2 = ((dp2 + sp2) >> 2) & 0x00FF00FF;
	*p++ = (dp1 | (dp2 << 8));
    }
}

/**
**	Draw 25% translucent horizontal line unclipped.
**
**	@param color	color
**	@param x	x coordinate on the screen
**	@param y	y coordinate on the screen
**	@param width	width of line (0=don't draw).
*/
#ifdef USE_OPENGL
local void Draw25TransHLineOpenGL(VMemType color, int x, int y, int width)
{
    VMemType32 c;
    GLubyte r;
    GLubyte g;
    GLubyte b;

    c = color.D32;
    r = (c >> 16) & 0xff;
    g = (c >> 8) & 0xff;
    b = (c >> 0) & 0xff;
    glDisable(GL_TEXTURE_2D);
    glColor4ub(r, g, b, 192);
    glBegin(GL_LINES);
    glVertex2i(x, VideoHeight - y);
    glVertex2i(x + width, VideoHeight - y);
    glEnd();
    glEnable(GL_TEXTURE_2D);
}
#endif

/**
**	Draw 50% translucent horizontal line unclipped into 8bit framebuffer.
**
**	@param color	color
**	@param x	x coordinate on the screen
**	@param y	y coordinate on the screen
**	@param width	width of line (0=don't draw).
*/
local void Draw50TransHLine8(VMemType color, int x, int y, int width)
{
    VMemType8* p;
    VMemType8* e;
    unsigned int c;

    p = VideoMemory8 + x + y * VideoWidth;
    e = p + width;
    c = color.D8 << 8;

    while (p < e) {
	*p = lookup50trans8[c | *p];
	++p;
    }
}

/**
**	Draw 50% translucent horizontal line unclipped into 15bit framebuffer.
**
**	@param color	color
**	@param x	x coordinate on the screen
**	@param y	y coordinate on the screen
**	@param width	width of line (0=don't draw).
*/
local void Draw50TransHLine15(VMemType color, int x, int y, int width)
{
    VMemType16* p;
    VMemType16* e;
    int w;
    unsigned long sp;

    w = VideoWidth;
    p = VideoMemory16 + y * w + x;
    e = p + width;
    sp = color.D16;
    sp = ((sp << 16) | sp) & 0x03E07C1F;

    while (p < e) {
	unsigned long dp;

	dp = *p;
	dp = ((dp << 16) | dp) & 0x03E07C1F;
	dp = ((dp + sp) >> 1) & 0x03E07C1F;
	*p++ = (dp >> 16) | dp;
    }
}

/**
**	Draw 50% translucent horizontal line unclipped into 16bit framebuffer.
**
**	@param color	color
**	@param x	x coordinate on the screen
**	@param y	y coordinate on the screen
**	@param width	width of line (0=don't draw).
*/
local void Draw50TransHLine16(VMemType color, int x, int y, int width)
{
    VMemType16* p;
    VMemType16* e;
    int w;
    unsigned long sp;

    w = VideoWidth;
    p = VideoMemory16 + y * w + x;
    e = p + width;
    sp = color.D16;
    sp = ((sp << 16) | sp) & 0x07E0F81F;

    while (p < e) {
	unsigned long dp;

	dp = *p;
	dp = ((dp << 16) | dp) & 0x07E0F81F;
	dp = ((dp + sp) >> 1) & 0x07E0F81F;
	*p++ = (dp >> 16) | dp;
    }
}

/**
**	Draw 50% translucent horizontal line unclipped into 24bit framebuffer.
**
**	@param color	color
**	@param x	x coordinate on the screen
**	@param y	y coordinate on the screen
**	@param width	width of line (0=don't draw).
*/
local void Draw50TransHLine24(VMemType color, int x, int y, int width)
{
// FIXME: does 24bpp holds R|G|B ?
    DrawHLine24(color, x, y, width); // no trans functionaility for the moment :(
}

/**
**	Draw 50% translucent horizontal line unclipped into 32bit framebuffer.
**
**	@param color	color
**	@param x	x coordinate on the screen
**	@param y	y coordinate on the screen
**	@param width	width of line (0=don't draw).
*/
local void Draw50TransHLine32(VMemType color, int x, int y, int width)
{
    VMemType32* p, *e;
    unsigned long sp1, sp2;
    int w;

    w = VideoWidth;
    p = VideoMemory32 + y * w + x;
    e = p + width;
    sp1 = color.D32;
    sp2 = (sp1 & 0xFF00FF00) >> 8;
    sp1 &= 0x00FF00FF;

    while (p < e) {
	unsigned long dp1, dp2;

	dp1 = *p;
	dp2 = (dp1 & 0xFF00FF00) >> 8;
	dp1 &= 0x00FF00FF;

	dp1 = ((dp1 + sp1) >> 1) & 0x00FF00FF;
	dp2 = ((dp2 + sp2) >> 1) & 0x00FF00FF;
	*p++ = (dp1 | (dp2 << 8));
    }
}

/**
**	Draw 50% translucent horizontal line unclipped.
**
**	@param color	color
**	@param x	x coordinate on the screen
**	@param y	y coordinate on the screen
**	@param width	width of line (0=don't draw).
*/
#ifdef USE_OPENGL
local void Draw50TransHLineOpenGL(VMemType color, int x, int y, int width)
{
    VMemType32 c;
    GLubyte r;
    GLubyte g;
    GLubyte b;

    c = color.D32;
    r = (c >> 16) & 0xff;
    g = (c >> 8) & 0xff;
    b = (c >> 0) & 0xff;
    glDisable(GL_TEXTURE_2D);
    glColor4ub(r, g, b, 128);
    glBegin(GL_LINES);
    glVertex2i(x, VideoHeight - y);
    glVertex2i(x + width, VideoHeight - y);
    glEnd();
    glEnable(GL_TEXTURE_2D);
}
#endif

/**
**	Draw 75% translucent horizontal line unclipped into 8bit framebuffer.
**
**	@param color	color
**	@param x	x coordinate on the screen
**	@param y	y coordinate on the screen
**	@param width	width of line (0=don't draw).
*/
local void Draw75TransHLine8(VMemType color, int x, int y, int width)
{
    VMemType8* p;
    VMemType8* e;
    unsigned int c;

    p = VideoMemory8 + x + y * VideoWidth;
    e = p + width;
    c = color.D8;

    while (p < e) {
	*p = lookup25trans8[(*p << 8) | c];
	++p;
    }
}

/**
**	Draw 75% translucent horizontal line unclipped into 15bit framebuffer.
**
**	@param color	color
**	@param x	x coordinate on the screen
**	@param y	y coordinate on the screen
**	@param width	width of line (0=don't draw).
*/
local void Draw75TransHLine15(VMemType color, int x, int y, int width)
{
    VMemType16* p;
    VMemType16* e;
    int w;
    unsigned long sp;

    w = VideoWidth;
    p = VideoMemory16 + y * w + x;
    e = p + width;
    sp = color.D16;
    sp = ((sp << 16) | sp) & 0x03E07C1F;

    while (p < e) {
	unsigned long dp;

	dp = *p;
	dp = ((dp << 16) | dp) & 0x03E07C1F;
	dp = (((dp << 1) + dp + sp) >> 2) & 0x03E07C1F;
	*p++ = (dp >> 16) | dp;
    }
}

/**
**	Draw 75% translucent horizontal line unclipped into 16bit framebuffer.
**
**	@param color	color
**	@param x	x coordinate on the screen
**	@param y	y coordinate on the screen
**	@param width	width of line (0=don't draw).
*/
local void Draw75TransHLine16(VMemType color, int x, int y, int width)
{
    VMemType16* p;
    VMemType16* e;
    int w;
    unsigned long sp;

    w = VideoWidth;
    p = VideoMemory16 + y * w + x;
    e = p + width;
    sp = color.D16;
    sp = ((sp << 16) | sp) & 0x07E0F81F;

    while (p < e) {
	unsigned long dp;

	dp = *p;
	dp = ((dp << 16) | dp) & 0x07E0F81F;
	dp = (((dp << 1) + dp + sp) >> 2) & 0x07E0F81F;
	*p++ = (dp >> 16) | dp;
    }
}

/**
**	Draw 75% translucent horizontal line unclipped into 24bit framebuffer.
**
**	@param color	color
**	@param x	x coordinate on the screen
**	@param y	y coordinate on the screen
**	@param width	width of line (0=don't draw).
*/
local void Draw75TransHLine24(VMemType color, int x, int y, int width)
{
// FIXME: does 24bpp holds R|G|B ?
    DrawHLine24(color, x, y, width); // no trans functionaility for the moment :(
}

/**
**	Draw 75% translucent horizontal line unclipped into 32bit framebuffer.
**
**	@param color	color
**	@param x	x coordinate on the screen
**	@param y	y coordinate on the screen
**	@param width	width of line (0=don't draw).
*/
local void Draw75TransHLine32(VMemType color, int x, int y, int width)
{
    VMemType32* p;
    VMemType32* e;
    unsigned long sp1;
    unsigned long sp2;
    int w;

    w = VideoWidth;
    p = VideoMemory32 + y * w + x;
    e = p + width;
    sp1 = color.D32;
    sp2 = (sp1 & 0xFF00FF00) >> 8;
    sp1 &= 0x00FF00FF;

    while (p < e) {
	unsigned long dp1;
	unsigned long dp2;

	dp1 = *p;
	dp2 = (dp1 & 0xFF00FF00) >> 8;
	dp1 &= 0x00FF00FF;

	dp1 = (((dp1 << 1) + dp1 + sp1) >> 2) & 0x00FF00FF;
	dp2 = (((dp2 << 1) + dp2 + sp2) >> 2) & 0x00FF00FF;
	*p++ = (dp1 | (dp2 << 8));
    }
}

/**
**	Draw 75% translucent horizontal line unclipped.
**
**	@param color	color
**	@param x	x coordinate on the screen
**	@param y	y coordinate on the screen
**	@param width	width of line (0=don't draw).
*/
#ifdef USE_OPENGL
local void Draw75TransHLineOpenGL(VMemType color, int x, int y, int width)
{
    VMemType32 c;
    GLubyte r;
    GLubyte g;
    GLubyte b;

    c = color.D32;
    r = (c >> 16) & 0xff;
    g = (c >> 8) & 0xff;
    b = (c >> 0) & 0xff;
    glDisable(GL_TEXTURE_2D);
    glColor4ub(r, g, b, 64);
    glBegin(GL_LINES);
    glVertex2i(x, VideoHeight - y);
    glVertex2i(x + width, VideoHeight - y);
    glEnd();
    glEnable(GL_TEXTURE_2D);
}
#endif

/**
**	Draw translucent horizontal line unclipped into 8bit framebuffer.
**
**	@param color	color
**	@param x	x coordinate on the screen
**	@param y	y coordinate on the screen
**	@param width	width of line (0=don't draw).
**	@param alpha	alpha value of pixels.
*/
local void DrawTransHLine8(VMemType color, int x, int y, int width,
    unsigned char alpha)
{
    VMemType8* p;
    VMemType8* e;
    unsigned c;

    p = VideoMemory8 + x + y * VideoWidth;
    e = p + width;
    c = color.D8;

    switch (((unsigned int)alpha * 4) / 255) {
	case 0:
	    while (p < e) {
		*p++ = c;
	    }
	    break;
	case 1:
	    while (p < e) {
		*p = lookup25trans8[(*p << 8) | c];
		++p;
	    }
	    break;
	case 2:
	    c <<= 8;
	    while (p < e) {
		*p = lookup50trans8[c | *p];
		++p;
	    }
	    break;
	case 3:
	    c <<= 8;
	    while (p < e) {
		*p = lookup25trans8[c | *p];
		++p;
	    }
	    break;
	default:
	    break;
    }
}

/**
**	Draw horizontal line unclipped into 8bit framebuffer (ignoring alpha).
**
**	@param color	Color index
**	@param x	x pixel coordinate on the screen
**	@param y	y pixel coordinate on the screen
**	@param width	Line width in pixel
**      @param alpha    alpha value of pixel
*/
local void DrawNoTransHLine8(VMemType color, int x, int y, int width,
    unsigned char alpha __attribute__((unused)))
{
    DrawHLine8(color, x, y, width);
}

/**
**	Draw translucent horizontal line unclipped into 15bit framebuffer.
**
**	@param color	color
**	@param x	x coordinate on the screen
**	@param y	y coordinate on the screen
**	@param width	width of line (0=don't draw).
**	@param alpha	alpha value of pixels.
*/
local void DrawTransHLine15(VMemType color, int x, int y, int width,
    unsigned char alpha)
{
    VMemType16* p;
    VMemType16* e;
    unsigned long sp;

    p = VideoMemory16 + y * VideoWidth + x;
    e = p + width;
    sp = color.D16;
    // FIXME: pre multiply?
    sp = ((sp << 16) | sp) & 0x03E07C1F;
    alpha >>= 3;				// FIXME: only 5bits

    while (p < e) {
	unsigned long dp;

	dp = *p;
	dp = ((dp << 16) | dp) & 0x03E07C1F;
	dp = ((((dp - sp) * alpha) >> 5) + sp) & 0x03E07C1F; //FIXME: alpha==256 unreached
	*p++ = (dp >> 16) | dp;
    }
}

/**
**	Draw translucent horizontal line unclipped into 16bit framebuffer.
**
**	@param color	color
**	@param x	x coordinate on the screen
**	@param y	y coordinate on the screen
**	@param width	width of line (0=don't draw).
**	@param alpha	alpha value of pixels.
*/
local void DrawTransHLine16(VMemType color, int x, int y, int width,
    unsigned char alpha)
{
    VMemType16* p;
    VMemType16* e;
    unsigned long sp;

    p = VideoMemory16 + y * VideoWidth + x;
    e = p + width;
    sp = color.D16;
    // FIXME: pre multiply?
    sp = ((sp << 16) | sp) & 0x07E0F81F;
    alpha >>= 3;				// FIXME: only 5bits

    while (p < e) {
	unsigned long dp;

	dp = *p;
	dp = ((dp << 16) | dp) & 0x07E0F81F;
	dp = ((((dp - sp) * alpha) >> 5) + sp) & 0x07E0F81F; //FIXME: alpha==256 unreached
	*p++ = (dp >> 16) | dp;
    }
}

/**
**	Draw translucent horizontal line unclipped into 24bit framebuffer.
**
**	@param color	color
**	@param x	x coordinate on the screen
**	@param y	y coordinate on the screen
**	@param width	width of line (0=don't draw).
**	@param alpha	alpha value of pixels.
*/
local void DrawTransHLine24(VMemType color, int x, int y, int width,
    unsigned char alpha)
{
    VMemType24 c;
    VMemType24* p;
    VMemType24* e;
    unsigned spR;
    unsigned spG;
    unsigned spB;

    p = VideoMemory24 + y * VideoWidth + x;
    e = p + width;
    c = color.D24;
    spR = c.a;
    spG = c.b;
    spB = c.c;
    // FIXME: untested; does 24bpp holds R|G|B ?
  //FIXME: alpha==256 unreached

    while (p < e) {
	unsigned int i;

	c = *p;

	i = c.a;
	i = (((i - spR) * alpha) >> 8) + spR;
	c.a = i & 0xFF;

	i = c.b;
	i = (((i - spG) * alpha) >> 8) + spG;
	c.b = i & 0xFF;

	i = c.c;
	i = (((i - spB) * alpha) >> 8) + spB;
	c.c = i & 0xFF;

	*p++ = c;
    }
}

/**
**	Draw translucent horizontal line unclipped into 32bit framebuffer.
**
**	@param color	color
**	@param x	x coordinate on the screen
**	@param y	y coordinate on the screen
**	@param width	width of line (0=don't draw).
**	@param alpha	alpha value of pixels.
*/
local void DrawTransHLine32(VMemType color, int x, int y, int width,
    unsigned char alpha)
{
    VMemType32* p;
    VMemType32* e;
    unsigned long sp1;
    unsigned long sp2;

    p = VideoMemory32 + y * VideoWidth + x;
    e = p + width;
    sp1 = color.D32;
    sp2 = (sp1 & 0xFF00FF00) >> 8;
    sp1 &= 0x00FF00FF;

    alpha >>= 1;

    while (p < e) {
	unsigned long dp1;
	unsigned long dp2;

	dp1 = *p;
	dp2 = (dp1 & 0xFF00FF00) >> 8;
	dp1 &= 0x00FF00FF;

	//FIXME: alpha==256 unreached
	dp1 = ((((dp1 - sp1) * alpha) >> 7) + sp1) & 0x00FF00FF;
	dp2 = ((((dp2 - sp2) * alpha) >> 7) + sp2) & 0x00FF00FF;

	*p++ = (dp1 | (dp2 << 8));
    }
}

#define CLIP_HLINE(x, y, width) { \
    if (y < ClipY1 || y > ClipY2) { \
	return; \
    } \
    if (x < ClipX1) { \
	int f = ClipX1 - x; \
	if (width <= f) { \
	    return; \
	} \
	width -= f; \
	x = ClipX1; \
    } \
    if ((x + width) > ClipX2 + 1) { \
	if (x > ClipX2) { \
	    return; \
	} \
	width = ClipX2 - x + 1; \
    } \
}

/**
**	Draw translucent horizontal line unclipped.
**
**	@param color	color
**	@param x	x coordinate on the screen
**	@param y	y coordinate on the screen
**	@param width	width of line (0=don't draw).
**	@param alpha	alpha value of pixels.
*/
#ifdef USE_OPENGL
local void DrawTransHLineOpenGL(VMemType color, int x, int y, int width,
    unsigned char alpha)
{
    VMemType32 c;
    GLubyte r;
    GLubyte g;
    GLubyte b;

    c = color.D32;
    r = (c >> 16) & 0xff;
    g = (c >> 8) & 0xff;
    b = (c >> 0) & 0xff;
    glDisable(GL_TEXTURE_2D);
    glColor4ub(r, g, b, 255 - alpha);
    glBegin(GL_LINES);
    glVertex2i(x, VideoHeight - y);
    glVertex2i(x + width, VideoHeight - y);
    glEnd();
    glEnable(GL_TEXTURE_2D);
}
#endif
/**
**	Draw horizontal line clipped.
**
**	@param color	color
**	@param x	x coordinate on the screen
**	@param y	y coordinate on the screen
**	@param width	width of line (0=don't draw).
*/
global void VideoDrawHLineClip(VMemType color, int x, int y, int width)
{
    CLIP_HLINE(x, y, width);
    VideoDrawHLine(color, x, y, width);
}

/**
**	Draw 25% translucent horizontal line clipped.
**
**	@param color	color
**	@param x	x coordinate on the screen
**	@param y	y coordinate on the screen
**	@param width	width of line (0=don't draw).
*/
global void VideoDraw25TransHLineClip(VMemType color, int x, int y, int width)
{
    CLIP_HLINE(x, y, width);
    VideoDraw25TransHLine(color, x, y, width);
}

/**
**	Draw 50% translucent horizontal line clipped.
**
**	@param color	color
**	@param x	x coordinate on the screen
**	@param y	y coordinate on the screen
**	@param width	width of line (0=don't draw).
*/
global void VideoDraw50TransHLineClip(VMemType color, int x, int y, int width)
{
    CLIP_HLINE(x, y, width);
    VideoDraw50TransHLine(color, x, y, width);
}

/**
**	Draw 75% translucent horizontal line clipped.
**
**	@param color	color
**	@param x	x coordinate on the screen
**	@param y	y coordinate on the screen
**	@param width	width of line (0=don't draw).
*/
global void VideoDraw75TransHLineClip(VMemType color, int x, int y, int width)
{
    CLIP_HLINE(x, y, width);
    VideoDraw75TransHLine(color, x, y, width);
}

/**
**	Draw translucent horizontal line clipped.
**
**	@param color	Color index
**	@param x	X pixel coordinate on the screen
**	@param y	Y c pixeloordinate on the screen
**	@param width	Width of line (0=don't draw)
**	@param alpha	Alpha value of pixels
*/
global void VideoDrawTransHLineClip(VMemType color, int x, int y, int width,
    unsigned char alpha)
{
    CLIP_HLINE(x, y, width);
    VideoDrawTransHLine(color, x, y, width,alpha);
}

// ===========================================================================
//	Vertical Line
// ===========================================================================

/**
**	Draw vertical line unclipped into 8bit framebuffer.
**
**	@param color	color
**	@param x	x coordinate on the screen
**	@param y	y coordinate on the screen
**	@param height	height of line (0=don't draw).
*/
local void DrawVLine8(VMemType color, int x, int y, int height)
{
    VMemType8* p;
    VMemType8* e;
    int w;
    int f;

    w = VideoWidth;
    p = VideoMemory8 + y * w + x;
    e = p + height * w;
    f = color.D8;
    while (p < e) {			// FIXME: better
	*p = f;
	p += w;
    }
}

/**
**	Draw vertical line unclipped into 16bit framebuffer.
**
**	@param color	color
**	@param x	x coordinate on the screen
**	@param y	y coordinate on the screen
**	@param height	height of line (0=don't draw).
*/
local void DrawVLine16(VMemType color, int x, int y, int height)
{
    VMemType16* p;
    VMemType16* e;
    int w;
    int f;

    w = VideoWidth;
    p = VideoMemory16 + y * w + x;
    e = p + height * w;
    f = color.D16;
    while (p < e) {			// FIXME: better
	*p = f;
	p += w;
    }
}

/**
**	Draw vertical line unclipped into 24bit framebuffer.
**
**	@param color	color
**	@param x	x coordinate on the screen
**	@param y	y coordinate on the screen
**	@param height	height of line (0=don't draw).
*/
local void DrawVLine24(VMemType color, int x, int y, int height)
{
    VMemType24* p;
    VMemType24* e;
    int w;
    VMemType24 f;

    w = VideoWidth;
    p = VideoMemory24 + y * w + x;
    e = p + height * w;
    f = color.D24;
    while (p < e) {			// FIXME: better
	*p = f;
	p += w;
    }
}

/**
**	Draw vertical line unclipped into 32bit framebuffer.
**
**	@param color	color
**	@param x	x coordinate on the screen
**	@param y	y coordinate on the screen
**	@param height	height of line (0=don't draw).
*/
local void DrawVLine32(VMemType color, int x, int y, int height)
{
    VMemType32* p;
    VMemType32* e;
    int w;
    int f;

    w = VideoWidth;
    p = VideoMemory32 + y * w + x;
    e = p + height * w;
    f = color.D32;
    while (p < e) {			// FIXME: better
	*p = f;
	p += w;
    }
}

/**
**	Draw vertical line unclipped.
**
**	@param color	color
**	@param x	x coordinate on the screen
**	@param y	y coordinate on the screen
**	@param height	height of line (0=don't draw).
*/
#ifdef USE_OPENGL
local void DrawVLineOpenGL(VMemType color, int x, int y, int height)
{
    VMemType32 c;
    GLubyte r;
    GLubyte g;
    GLubyte b;

    c = color.D32;
    r = (c >> 16) & 0xff;
    g = (c >> 8) & 0xff;
    b = (c >> 0) & 0xff;
    glDisable(GL_TEXTURE_2D);
    glColor3ub(r, g, b);
    glBegin(GL_LINES);
    glVertex2i(x, VideoHeight - y);
    glVertex2i(x, VideoHeight - (y + height));
    glEnd();
    glEnable(GL_TEXTURE_2D);
}
#endif

/**
**	Draw 25% translucent vertical line unclipped into 8bit framebuffer.
**
**	@param color	color
**	@param x	x coordinate on the screen
**	@param y	y coordinate on the screen
**	@param height	height of line (0=don't draw).
*/
local void Draw25TransVLine8(VMemType color, int x, int y, int height)
{
    VMemType8* p;
    unsigned int c;
    int w;

    w = VideoWidth;
    p = VideoMemory8 + x + y * w;
    c = color.D8 << 8;
    while (height--) {
	*p = lookup25trans8[c | *p];
	p += w;
    }
}

/**
**	Draw 25% translucent vertical line unclipped into 15bit framebuffer.
**
**	@param color	color
**	@param x	x coordinate on the screen
**	@param y	y coordinate on the screen
**	@param height	height of line (0=don't draw).
*/
local void Draw25TransVLine15(VMemType color, int x, int y, int height)
{
    VMemType16* p;
    unsigned long sp;
    int w;

    w = VideoWidth;
    p = VideoMemory16 + y * w + x;
    sp = color.D16;
    // FIXME: pre multiply?
    sp = (((sp << 16) | sp) & 0x03E07C1F) * 3;

    while (height--) {
	unsigned long dp;

	dp = *p;
	dp = ((dp << 16) | dp) & 0x03E07C1F;
	dp = ((dp + sp) >> 2) & 0x03E07C1F;
	*p = (dp >> 16) | dp;
	p += w;
    }
}

/**
**	Draw 25% translucent vertical line unclipped into 16bit framebuffer.
**
**	@param color	color
**	@param x	x coordinate on the screen
**	@param y	y coordinate on the screen
**	@param height	height of line (0=don't draw).
*/
local void Draw25TransVLine16(VMemType color, int x, int y, int height)
{
    VMemType16* p;
    unsigned long sp;
    int w;

    w = VideoWidth;
    p = VideoMemory16 + y * w + x;
    sp = color.D16;
    // FIXME: pre multiply?
    sp = (((sp << 16) | sp) & 0x07E0F81F) * 3;

    while (height--) {
	unsigned long dp;

	dp = *p;
	dp = ((dp << 16) | dp) & 0x07E0F81F;
	dp = ((dp + sp) >> 2) & 0x07E0F81F;
	*p = (dp >> 16) | dp;
	p += w;
    }
}

/**
**	Draw 25% translucent vertical line unclipped into 24bit framebuffer.
**
**	@param color	color
**	@param x	x coordinate on the screen
**	@param y	y coordinate on the screen
**	@param height	height of line (0=don't draw).
*/
local void Draw25TransVLine24(VMemType color, int x, int y, int height)
{
    // FIXME: does 24bpp holds R|G|B ?
    DrawVLine24(color, x, y, height); // no trans functionaility for the moment :(
}

/**
**	Draw 25% translucent vertical line unclipped into 32bit framebuffer.
**
**	@param color	color
**	@param x	x coordinate on the screen
**	@param y	y coordinate on the screen
**	@param height	height of line (0=don't draw).
*/
local void Draw25TransVLine32(VMemType color, int x, int y, int height)
{
    VMemType32* p;
    unsigned long sp1,sp2;
    int w;

    w = VideoWidth;
    p = VideoMemory32 + y * w + x;
    sp1 = color.D32;
    // FIXME: pre multiply?
    sp2 = ((sp1 & 0xFF00FF00) >> 8) * 3;
    sp1 = (sp1 & 0x00FF00FF) * 3;

    while (height--) {
	unsigned long dp1, dp2;

	dp1 = *p;
	dp2 = (dp1 & 0xFF00FF00) >> 8;
	dp1 &= 0x00FF00FF;

	dp1 = ((dp1 + sp1) >> 2) & 0x00FF00FF;
	dp2 = ((dp2 + sp2) >> 2) & 0x00FF00FF;
	*p = (dp1 | (dp2 << 8));

	p += w;
    }
}

/**
**	Draw 25% translucent vertical line unclipped.
**
**	@param color	color
**	@param x	x coordinate on the screen
**	@param y	y coordinate on the screen
**	@param height	height of line (0=don't draw).
*/
#ifdef USE_OPENGL
local void Draw25TransVLineOpenGL(VMemType color, int x, int y, int height)
{
    VMemType32 c;
    GLubyte r,g,b;

    c = color.D32;
    r = (c >> 16) & 0xff;
    g = (c >> 8) & 0xff;
    b = (c >> 0) & 0xff;
    glDisable(GL_TEXTURE_2D);
    glColor4ub(r, g, b, 192);
    glBegin(GL_LINES);
    glVertex2i(x, VideoHeight - y);
    glVertex2i(x, VideoHeight - (y + height));
    glEnd();
    glEnable(GL_TEXTURE_2D);
}
#endif

/**
**	Draw 50% translucent vertical line unclipped into 8bit framebuffer.
**
**	@param color	color
**	@param x	x coordinate on the screen
**	@param y	y coordinate on the screen
**	@param height	height of line (0=don't draw).
*/
local void Draw50TransVLine8(VMemType color, int x, int y, int height)
{
    VMemType8* p;
    unsigned int c;
    int w;

    w = VideoWidth;
    p = VideoMemory8 + x + y * w;
    c = color.D8 << 8;
    while (height--) {
	*p = lookup50trans8[c | *p];
	p += w;
    }
}

/**
**	Draw 50% translucent vertical line unclipped into 15bit framebuffer.
**
**	@param color	color
**	@param x	x coordinate on the screen
**	@param y	y coordinate on the screen
**	@param height	height of line (0=don't draw).
*/
local void Draw50TransVLine15(VMemType color, int x, int y, int height)
{
    VMemType16* p;
    unsigned long sp;
    int w;

    w = VideoWidth;
    p = VideoMemory16 + y * w + x;
    sp = color.D16;
    sp = ((sp << 16) | sp) & 0x03E07C1F;

    while (height--) {
	unsigned long dp;

	dp = *p;
	dp = ((dp << 16) | dp) & 0x03E07C1F;
	dp = ((dp + sp) >> 1) & 0x03E07C1F;
	*p = (dp >> 16) | dp;
	p += w;
    }
}

/**
**	Draw 50% translucent vertical line unclipped into 16bit framebuffer.
**
**	@param color	color
**	@param x	x coordinate on the screen
**	@param y	y coordinate on the screen
**	@param height	height of line (0=don't draw).
*/
local void Draw50TransVLine16(VMemType color, int x, int y, int height)
{
    VMemType16* p;
    unsigned long sp;
    int w;

    w = VideoWidth;
    p = VideoMemory16 + y * w + x;
    sp = color.D16;
    // FIXME: pre multiply?
    sp = ((sp << 16) | sp) & 0x07E0F81F;

    while (height--) {
	unsigned long dp;

	dp = *p;
	dp = ((dp << 16) | dp) & 0x07E0F81F;
	dp = ((dp + sp) >> 1) & 0x07E0F81F;
	*p = (dp >> 16) | dp;
	p += w;
    }
}

/**
**	Draw 50% translucent vertical line unclipped into 24bit framebuffer.
**
**	@param color	color
**	@param x	x coordinate on the screen
**	@param y	y coordinate on the screen
**	@param height	height of line (0=don't draw).
*/
local void Draw50TransVLine24(VMemType color, int x, int y, int height)
{
    // FIXME: does 24bpp holds R|G|B ?
    DrawVLine24(color, x, y, height); // no trans functionaility for the moment :(
}

/**
**	Draw 50% translucent vertical line unclipped into 32bit framebuffer.
**
**	@param color	color
**	@param x	x coordinate on the screen
**	@param y	y coordinate on the screen
**	@param height	height of line (0=don't draw).
*/
local void Draw50TransVLine32(VMemType color, int x, int y, int height)
{
    VMemType32* p;
    unsigned long sp1;
    unsigned long sp2;
    int w;

    w = VideoWidth;
    p = VideoMemory32 + y * w + x;
    sp1 = color.D32;
    sp2 = (sp1 & 0xFF00FF00) >> 8;
    sp1 &= 0x00FF00FF;

    while (height--) {
	unsigned long dp1;
	unsigned long dp2;

	dp1 = *p;
	dp2 = (dp1 & 0xFF00FF00) >> 8;
	dp1 &= 0x00FF00FF;

	dp1 = ((dp1 + sp1) >> 1) & 0x00FF00FF;
	dp2 = ((dp2 + sp2) >> 1) & 0x00FF00FF;
	*p = (dp1 | (dp2 << 8));

	p += w;
    }
}

/**
**	Draw 50% translucent vertical line unclipped.
**
**	@param color	color
**	@param x	x coordinate on the screen
**	@param y	y coordinate on the screen
**	@param height	height of line (0=don't draw).
*/
#ifdef USE_OPENGL
local void Draw50TransVLineOpenGL(VMemType color, int x, int y, int height)
{
    VMemType32 c;
    GLubyte r;
    GLubyte g;
    GLubyte b;

    c = color.D32;
    r = (c >> 16) & 0xff;
    g = (c >> 8) & 0xff;
    b = (c >> 0) & 0xff;
    glDisable(GL_TEXTURE_2D);
    glColor4ub(r, g, b, 128);
    glBegin(GL_LINES);
    glVertex2i(x, VideoHeight - y);
    glVertex2i(x, VideoHeight - (y + height));
    glEnd();
    glEnable(GL_TEXTURE_2D);
}
#endif

/**
**	Draw 75% translucent vertical line unclipped into 8bit framebuffer.
**
**	@param color	color
**	@param x	x coordinate on the screen
**	@param y	y coordinate on the screen
**	@param height	height of line (0=don't draw).
*/
local void Draw75TransVLine8(VMemType color, int x, int y, int height)
{
    VMemType8* p;
    unsigned int c;
    int w;

    w = VideoWidth;
    p = VideoMemory8 + x + y * w;
    c = color.D8;
    while (height--) {
	*p = lookup25trans8[(*p << 8) | c];
	p += w;
    }
}

/**
**	Draw 75% translucent vertical line unclipped into 15bit framebuffer.
**
**	@param color	color
**	@param x	x coordinate on the screen
**	@param y	y coordinate on the screen
**	@param height	height of line (0=don't draw).
*/
local void Draw75TransVLine15(VMemType color, int x, int y, int height)
{
    VMemType16* p;
    unsigned long sp;
    int w;

    w = VideoWidth;
    p = VideoMemory16 + y * w + x;
    sp = color.D16;
    // FIXME: pre multiply?
    sp = ((sp << 16) | sp) & 0x03E07C1F;

    while (height--) {
	unsigned long dp;

	dp = *p;
	dp = ((dp << 16) | dp) & 0x03E07C1F;
	dp = (((dp << 1) + dp + sp) >> 2) & 0x03E07C1F;
	*p = (dp >> 16) | dp;
	p += w;
    }
}

/**
**	Draw 75% translucent vertical line unclipped into 16bit framebuffer.
**
**	@param color	color
**	@param x	x coordinate on the screen
**	@param y	y coordinate on the screen
**	@param height	height of line (0=don't draw).
*/
local void Draw75TransVLine16(VMemType color, int x, int y, int height)
{
    VMemType16* p;
    unsigned long sp;
    int w;

    w = VideoWidth;
    p = VideoMemory16 + y * w + x;
    sp = color.D16;
    // FIXME: pre multiply?
    sp = ((sp << 16) | sp) & 0x07E0F81F;

    while (height--) {
	unsigned long dp;

	dp = *p;
	dp = ((dp << 16) | dp) & 0x07E0F81F;
	dp = (((dp << 1) + dp + sp) >> 2) & 0x07E0F81F;
	*p = (dp >> 16) | dp;
	p += w;
    }
}

/**
**	Draw 75% translucent vertical line unclipped into 24bit framebuffer.
**
**	@param color	color
**	@param x	x coordinate on the screen
**	@param y	y coordinate on the screen
**	@param height	height of line (0=don't draw).
*/
local void Draw75TransVLine24(VMemType color, int x, int y, int height)
{
    // FIXME: does 24bpp holds R|G|B ?
    DrawVLine24(color, x, y, height); // no trans functionaility for the moment :(
}

/**
**	Draw 75% translucent vertical line unclipped into 32bit framebuffer.
**
**	@param color	color
**	@param x	x coordinate on the screen
**	@param y	y coordinate on the screen
**	@param height	height of line (0=don't draw).
*/
local void Draw75TransVLine32(VMemType color, int x, int y, int height)
{
    VMemType32* p;
    unsigned long sp1;
    unsigned long sp2;
    int w;

    w = VideoWidth;
    p = VideoMemory32 + y * w + x;
    sp1 = color.D32;
    sp2 = (sp1 & 0xFF00FF00) >> 8;
    sp1 &= 0x00FF00FF;

    while (height--) {
	unsigned long dp1;
	unsigned long dp2;

	dp1 = *p;
	dp2 = (dp1 & 0xFF00FF00) >> 8;
	dp1 &= 0x00FF00FF;

	dp1 = (((dp1 << 1) + dp1 + sp1) >> 2) & 0x00FF00FF;
	dp2 = (((dp2 << 1) + dp2 + sp2) >> 2) & 0x00FF00FF;
	*p = (dp1 | (dp2 << 8));

	p += w;
    }
}

/**
**	Draw 75% translucent vertical line unclipped.
**
**	@param color	color
**	@param x	x coordinate on the screen
**	@param y	y coordinate on the screen
**	@param height	height of line (0=don't draw).
*/
#ifdef USE_OPENGL
local void Draw75TransVLineOpenGL(VMemType color, int x, int y, int height)
{
    VMemType32 c;
    GLubyte r;
    GLubyte g;
    GLubyte b;

    c = color.D32;
    r = (c >> 16) & 0xff;
    g = (c >> 8) & 0xff;
    b = (c >> 0) & 0xff;
    glDisable(GL_TEXTURE_2D);
    glColor4ub(r, g, b, 64);
    glBegin(GL_LINES);
    glVertex2i(x, VideoHeight - y);
    glVertex2i(x, VideoHeight - (y + height));
    glEnd();
    glEnable(GL_TEXTURE_2D);
}
#endif

/**
**	Draw translucent vertical line unclipped into 8bit framebuffer.
**
**	@param color	color
**	@param x	x coordinate on the screen
**	@param y	y coordinate on the screen
**	@param height	height of line (0=don't draw).
**	@param alpha	alpha value of pixels.
*/
local void DrawTransVLine8(VMemType color, int x, int y, int height,
    unsigned char alpha)
{
    VMemType8* p;
    unsigned int c;
    int w;

    w = VideoWidth;
    p = VideoMemory8 + x + y * w;
    c = color.D8;

    switch (((unsigned int)alpha * 4) / 255) {
	case 0:
	    while (height--) {
		*p = c;
		p += w;
	    }
	    break;
	case 1:
	    while (height--) {
		*p = lookup25trans8[(*p << 8) | c];
		p += w;
	    }
	    break;
	case 2:
	    c <<= 8;
	    while (height--) {
		*p = lookup50trans8[c | *p];
		p += w;
	    }
	    break;
	case 3:
	    c <<= 8;
	    while (height--) {
		*p = lookup25trans8[c | *p];
		p += w;
	    }
	    break;
	default:
	    break;
    }
}

/**
**	Draw vertical line unclipped into 8bit framebuffer (ignoring alpha).
**
**	@param color	color
**	@param x	x coordinate on the screen
**	@param y	y coordinate on the screen
**	@param height	Height = length of the line.
**      @param alpha    alpha value of pixel.
*/
local void DrawNoTransVLine8(VMemType color, int x, int y, int height,
    unsigned char alpha __attribute__((unused)))
{
    DrawVLine8(color, x, y, height);
}

/**
**	Draw translucent vertical line unclipped into 15bit framebuffer.
**
**	@param color	color
**	@param x	x coordinate on the screen
**	@param y	y coordinate on the screen
**	@param height	height of line (0=don't draw).
**	@param alpha	alpha value of pixels.
*/
local void DrawTransVLine15(VMemType color, int x, int y, int height,
    unsigned char alpha)
{
    VMemType16* p;
    unsigned long sp;
    int w;

    w = VideoWidth;
    p = VideoMemory16 + y * w + x;
    sp = color.D16;
    // FIXME: pre multiply?
    sp = ((sp << 16) | sp) & 0x03E07C1F;
    alpha >>= 3;				// FIXME: only 5bits

    while (height--) {
	unsigned long dp;

	dp = *p;
	dp = ((dp << 16) | dp) & 0x03E07C1F;
	dp = ((((dp - sp) * alpha) >> 5) + sp) & 0x03E07C1F; //FIXME: alpha==256 unreached
	*p = (dp >> 16) | dp;
	p += w;
    }
}

/**
**	Draw translucent vertical line unclipped into 16bit framebuffer.
**
**	@param color	color
**	@param x	x coordinate on the screen
**	@param y	y coordinate on the screen
**	@param height	height of line (0=don't draw).
**	@param alpha	alpha value of pixels.
*/
local void DrawTransVLine16(VMemType color, int x, int y, int height,
    unsigned char alpha)
{
    VMemType16* p;
    unsigned long sp;
    int w;

    w = VideoWidth;
    p = VideoMemory16 + y * w + x;
    sp = color.D16;
    // FIXME: pre multiply?
    sp = ((sp << 16) | sp) & 0x07E0F81F;
    alpha >>= 3;				// FIXME: only 5bits

    while (height--) {
	unsigned long dp;

	dp = *p;
	dp = ((dp << 16) | dp) & 0x07E0F81F;
	dp = ((((dp - sp) * alpha) >> 5) + sp) & 0x07E0F81F; //FIXME: alpha==256 unreached
	*p = (dp >> 16) | dp;
	p += w;
    }
}

/**
**	Draw translucent vertical line unclipped into 24bit framebuffer.
**
**	@param color	color
**	@param x	x coordinate on the screen
**	@param y	y coordinate on the screen
**	@param height	height of line (0=don't draw).
**	@param alpha	alpha value of pixels.
*/
local void DrawTransVLine24(VMemType color, int x, int y, int height,
    unsigned char alpha __attribute__((unused)))
{
    // FIXME: does 24bpp holds R|G|B ?
    DrawVLine24(color, x, y, height); // no trans functionaility for the moment :(
}

/**
**	Draw translucent vertical line unclipped into 32bit framebuffer.
**
**	@param color	color
**	@param x	x coordinate on the screen
**	@param y	y coordinate on the screen
**	@param height	height of line (0=don't draw).
**	@param alpha	alpha value of pixels.
*/
local void DrawTransVLine32(VMemType color, int x, int y, int height,
    unsigned char alpha)
{
    VMemType32* p;
    unsigned long sp1;
    unsigned long sp2;
    int w;

    w = VideoWidth;
    p = VideoMemory32 + y * w + x;
    sp1 = color.D32;
    sp2 = (sp1 & 0xFF00FF00) >> 8;
    sp1 &= 0x00FF00FF;

    while (height--) {
	unsigned long dp1;
	unsigned long dp2;

	dp1 = *p;
	dp2 = (dp1 & 0xFF00FF00) >> 8;
	dp1 &= 0x00FF00FF;

	//FIXME: alpha==256 unreached
	dp1 = ((((dp1-sp1) * alpha) >> 8) + sp1) & 0x00FF00FF;
	dp2 = ((((dp2-sp2) * alpha) >> 8) + sp2) & 0x00FF00FF;
	*p = (dp1 | (dp2 << 8));

	p += w;
    }
}

#define CLIP_VLINE(x, y, width) { \
    if (x < ClipX1 || x > ClipX2) { \
	return; \
    } \
    if (y < ClipY1) { \
	int f = ClipY1 - y; \
	if (height <= f) { \
	    return; \
	} \
	height -= f; \
	y = ClipY1; \
    } \
    if ((y + height) > ClipY2 + 1) { \
	if (y > ClipY2) { \
	    return; \
	} \
	height = ClipY2 - y + 1; \
    } \
}

/**
**	Draw translucent vertical line unclipped.
**
**	@param color	color
**	@param x	x coordinate on the screen
**	@param y	y coordinate on the screen
**	@param height	height of line (0=don't draw).
**	@param alpha	alpha value of pixels.
*/
#ifdef USE_OPENGL
local void DrawTransVLineOpenGL(VMemType color, int x, int y, int height,
    unsigned char alpha)
{
    VMemType32 c;
    GLubyte r;
    GLubyte g;
    GLubyte b;

    c = color.D32;
    r = (c >> 16) & 0xff;
    g = (c >> 8) & 0xff;
    b = (c >> 0) & 0xff;
    glDisable(GL_TEXTURE_2D);
    glColor4ub(r, g, b, 255 - alpha);
    glBegin(GL_LINES);
    glVertex2i(x, VideoHeight - y);
    glVertex2i(x, VideoHeight - (y + height));
    glEnd();
    glEnable(GL_TEXTURE_2D);
}
#endif

/**
**	Draw vertical line clipped.
**
**	@param color	color
**	@param x	x coordinate on the screen
**	@param y	y coordinate on the screen
**	@param height	height of line (0=don't draw).
*/
global void VideoDrawVLineClip(VMemType color, int x, int y, int height)
{
    CLIP_VLINE(x, y, height);
    VideoDrawVLine(color, x, y, height);
}

/**
**	Draw 25% translucent vertical line clipped.
**
**	@param color	color
**	@param x	x coordinate on the screen
**	@param y	y coordinate on the screen
**	@param height	height of line (0=don't draw).
*/
global void VideoDraw25TransVLineClip(VMemType color, int x, int y,
    int height)
{
    CLIP_VLINE(x, y, height);
    VideoDraw25TransVLine(color, x, y, height);
}

/**
**	Draw 50% translucent vertical line clipped.
**
**	@param color	color
**	@param x	x coordinate on the screen
**	@param y	y coordinate on the screen
**	@param height	height of line (0=don't draw).
*/
global void VideoDraw50TransVLineClip(VMemType color, int x, int y,
    int height)
{
    CLIP_VLINE(x, y, height);
    VideoDraw50TransVLine(color, x, y, height);
}

/**
**	Draw 75% translucent vertical line clipped.
**
**	@param color	color
**	@param x	x coordinate on the screen
**	@param y	y coordinate on the screen
**	@param height	height of line (0=don't draw).
*/
global void VideoDraw75TransVLineClip(VMemType color, int x, int y,
    int height)
{
    CLIP_VLINE(x, y, height);
    VideoDraw75TransVLine(color, x, y, height);
}

/**
**	Draw translucent vertical line clipped.
**
**	@param color	color
**	@param x	x coordinate on the screen
**	@param y	y coordinate on the screen
**	@param height	height of line (0=don't draw).
**	@param alpha	alpha value of pixels.
*/
global void VideoDrawTransVLineClip(VMemType color, int x, int y,
    int height, unsigned char alpha)
{
    CLIP_VLINE(x, y, height);
    VideoDrawTransVLine(color, x, y, height, alpha);
}

// ===========================================================================
//	General Line
// ===========================================================================

/**
**	Draw line unclipped into 8bit framebuffer.
**
**	@param color	color
**	@param x1	Source x coordinate on the screen
**	@param y1	Source y coordinate on the screen
**	@param x2	Destination x coordinate on the screen
**	@param y2	Destination y coordinate on the screen
*/
local void DrawLine8(VMemType color, int x1, int y1, int x2, int y2)
{
    int d;
    int dx;
    int dy;
    int xstep;
    VMemType8* p;
    int w;
    unsigned f;

    if (x1 == x2) {
	if (y1<y2) {
	    DrawVLine8(color, x1, y1, y2 - y1 + 1);
	} else {
	    DrawVLine8(color, x2, y2, y1 - y2 + 1);
	}
	return;
    }
    if (y1 == y2) {
	if (x1 < x2) {
	    DrawHLine8(color, x1, y1, x2 - x1 + 1);
	} else {
	    DrawHLine8(color, x2, y2, x1 - x2 + 1);
	}
	return;
    }

    // initialize

    w = VideoWidth;
    f = color.D8;

    if (y1 > y2) {		// exchange coordinates
	x1 ^= x2; x2 ^= x1; x1 ^= x2;
	y1 ^= y2; y2 ^= y1; y1 ^= y2;
    }
    dy = y2 - y1;
    xstep = 1;
    dx = x2 - x1;
    if (dx < 0) {
	dx = -dx;
	xstep = -1;
    }

    p = VideoMemory8 + w * y1 + x1;
    *p = f;

    if (dx < dy) {		// step in vertical direction
	d = dy - 1;
	dx += dx;
	dy += dy;
	while (y1 != y2) {
	    y1++;
	    p += w;
	    d -= dx;
	    if (d < 0) {
		d += dy;
		p += xstep;
	    }
	    *p = f;
	}
	return;
    }

    if (dx > dy) {		// step in horizontal direction
	d = dx - 1;
	dx += dx;
	dy += dy;

	while (x1 != x2) {
	    x1 += xstep;
	    p += xstep;
	    d -= dy;
	    if (d < 0) {
		d += dx;
		p += w;
	    }
	    *p = f;
	}
	return;
    }

    while (y1 != y2) {			// diagonal line
	//x1 += xstep;
	p += xstep;
	y1++;
	p += w;
	*p = f;
    }
}

/**
**	Draw line unclipped into 16bit framebuffer.
**
**	@param color	color
**	@param x1	Source x coordinate on the screen
**	@param y1	Source y coordinate on the screen
**	@param x2	Destination x coordinate on the screen
**	@param y2	Destination y coordinate on the screen
*/
local void DrawLine16(VMemType color, int x1, int y1, int x2, int y2)
{
    int d;
    int dx;
    int dy;
    int xstep;
    VMemType16* p;
    int w;
    unsigned f;

    if (x1 == x2) {
	if (y1 < y2) {
	    DrawVLine16(color, x1, y1, y2 - y1 + 1);
	} else {
	    DrawVLine16(color, x2, y2, y1 - y2 + 1);
	}
	return;
    }
    if (y1 == y2) {
	if (x1 < x2) {
	    DrawHLine16(color, x1, y1, x2 - x1 + 1);
	} else {
	    DrawHLine16(color, x2, y2, x1 - x2 + 1);
	}
	return;
    }

    // initialize

    w = VideoWidth;
    f = color.D16;

    if (y1 > y2) {		// exchange coordinates
	x1 ^= x2; x2 ^= x1; x1 ^= x2;
	y1 ^= y2; y2 ^= y1; y1 ^= y2;
    }
    dy = y2 - y1;
    xstep = 1;
    dx = x2 - x1;
    if (dx < 0) {
	dx = -dx;
	xstep = -1;
    }

    p = VideoMemory16 + w * y1 + x1;
    *p = f;

    if (dx < dy) {		// step in vertical direction
	d = dy - 1;
	dx += dx;
	dy += dy;
	while (y1 != y2) {
	    y1++;
	    p += w;
	    d -= dx;
	    if (d < 0) {
		d += dy;
		p += xstep;
	    }
	    *p = f;
	}
	return;
    }

    if (dx > dy) {		// step in horizontal direction
	d = dx - 1;
	dx += dx;
	dy += dy;

	while (x1 != x2) {
	    x1 += xstep;
	    p += xstep;
	    d -= dy;
	    if (d < 0) {
		d += dx;
		p += w;
	    }
	    *p = f;
	}
	return;
    }

    while (y1 != y2) {			// diagonal line
	//x1 += xstep;
	p += xstep;
	y1++;
	p += w;
	*p = f;
    }
}

/**
**	Draw line unclipped into 24bit framebuffer.
**
**	@param color	color
**	@param x1	Source x coordinate on the screen
**	@param y1	Source y coordinate on the screen
**	@param x2	Destination x coordinate on the screen
**	@param y2	Destination y coordinate on the screen
*/
local void DrawLine24(VMemType color, int x1, int y1, int x2, int y2)
{
    int d;
    int dx;
    int dy;
    int xstep;
    VMemType24* p;
    int w;
    VMemType24 f;

    if (x1 == x2) {
	if (y1<y2) {
	    DrawVLine24(color, x1, y1, y2 - y1 + 1);
	} else {
	    DrawVLine24(color, x2, y2, y1 - y2 + 1);
	}
	return;
    }
    if (y1 == y2) {
	if (x1 < x2) {
	    DrawHLine24(color, x1, y1, x2 - x1 + 1);
	} else {
	    DrawHLine24(color, x2, y2, x1 - x2 + 1);
	}
	return;
    }

    // initialize

    w = VideoWidth;
    f = color.D24;

    if (y1 > y2) {		// exchange coordinates
	x1 ^= x2; x2 ^= x1; x1 ^= x2;
	y1 ^= y2; y2 ^= y1; y1 ^= y2;
    }
    dy = y2 - y1;
    xstep = 1;
    dx = x2 - x1;
    if (dx < 0) {
	dx = -dx;
	xstep = -1;
    }

    p = VideoMemory24 + w * y1 + x1;
    *p = f;

    if (dx < dy) {		// step in vertical direction
	d = dy - 1;
	dx += dx;
	dy += dy;
	while (y1 != y2) {
	    y1++;
	    p += w;
	    d -= dx;
	    if (d < 0) {
		d += dy;
		p += xstep;
	    }
	    *p = f;
	}
	return;
    }

    if (dx > dy) {		// step in horizontal direction
	d = dx - 1;
	dx += dx;
	dy += dy;

	while (x1 != x2) {
	    x1 += xstep;
	    p += xstep;
	    d -= dy;
	    if (d < 0) {
		d += dx;
		p += w;
	    }
	    *p = f;
	}
	return;
    }

    while (y1 != y2) {			// diagonal line
	//x1 += xstep;
	p += xstep;
	y1++;
	p += w;
	*p = f;
    }
}

/**
**	Draw line unclipped into 32bit framebuffer.
**
**	@param color	color
**	@param x1	Source x coordinate on the screen
**	@param y1	Source y coordinate on the screen
**	@param x2	Destination x coordinate on the screen
**	@param y2	Destination y coordinate on the screen
*/
local void DrawLine32(VMemType color, int x1, int y1, int x2, int y2)
{
    int d;
    int dx;
    int dy;
    int xstep;
    VMemType32* p;
    int w;
    unsigned f;

    if (x1 == x2) {
	if (y1 < y2) {
	    DrawVLine32(color, x1, y1, y2 - y1 + 1);
	} else {
	    DrawVLine32(color, x2, y2, y1 - y2 + 1);
	}
	return;
    }
    if (y1 == y2) {
	if (x1 < x2) {
	    DrawHLine32(color, x1, y1, x2 - x1 + 1);
	} else {
	    DrawHLine32(color, x2, y2, x1 - x2 + 1);
	}
	return;
    }

    // initialize

    w = VideoWidth;
    f = color.D32;

    if (y1 > y2) {		// exchange coordinates
	x1 ^= x2; x2 ^= x1; x1 ^= x2;
	y1 ^= y2; y2 ^= y1; y1 ^= y2;
    }
    dy = y2 - y1;
    xstep = 1;
    dx = x2 - x1;
    if (dx < 0) {
	dx = -dx;
	xstep = -1;
    }

    p = VideoMemory32 + w * y1 + x1;
    *p = f;

    if (dx < dy) {		// step in vertical direction
	d = dy - 1;
	dx += dx;
	dy += dy;
	while (y1 != y2) {
	    y1++;
	    p += w;
	    d -= dx;
	    if (d < 0) {
		d += dy;
		p += xstep;
	    }
	    *p = f;
	}
	return;
    }

    if (dx > dy) {		// step in horizontal direction
	d = dx - 1;
	dx += dx;
	dy += dy;

	while (x1 != x2) {
	    x1 += xstep;
	    p += xstep;
	    d -= dy;
	    if (d < 0) {
		d += dx;
		p += w;
	    }
	    *p = f;
	}
	return;
    }

    while (y1 != y2) {			// diagonal line
	//x1 += xstep;
	p += xstep;
	y1++;
	p += w;
	*p = f;
    }
}

/**
**	Draw line unclipped into 32bit framebuffer.
**
**	@param color	color
**	@param x1	Source x coordinate on the screen
**	@param y1	Source y coordinate on the screen
**	@param x2	Destination x coordinate on the screen
**	@param y2	Destination y coordinate on the screen
*/
#ifdef USE_OPENGL
local void DrawLineOpenGL(VMemType color, int x1, int y1, int x2, int y2)
{
    VMemType32 c;
    GLubyte r;
    GLubyte g;
    GLubyte b;

    c = color.D32;
    r = (c >> 16) & 0xff;
    g = (c >> 8) & 0xff;
    b = (c >> 0) & 0xff;
    glDisable(GL_TEXTURE_2D);
    glColor3ub(r, g, b);
    glBegin(GL_LINES);
    glVertex2i(x1, VideoHeight - y1);
    glVertex2i(x2, VideoHeight - y2);
    glEnd();
    glEnable(GL_TEXTURE_2D);
}
#endif

/**
**	Delivers bitmask denoting given point is left/right/above/below
**      clip rectangle, used for faster determinination of clipped position.
**
**	@param x	pixel's x position (not restricted to screen width)
**	@param y	pixel's y position (not restricted to screen height)
*/
static ClipCode ClipCodeLine(int x, int y)
{
    ClipCode result;

    if (y < ClipY1) {
	result = ClipCodeAbove;
    } else if (y > ClipY2) {
	result = ClipCodeBelow;
    } else {
	result = ClipCodeInside;
    }

    if (x < ClipX1) {
	result |= ClipCodeLeft;
    } else if (x > ClipX2) {
	result |= ClipCodeRight;
    }

    return result;
}

/**
**	Denotes entire line located at the same side outside clip rectangle
**      (point 1 and 2 are both as left/right/above/below the clip rectangle)
**
**	@param code1	ClipCode of one point of line
**	@param code2	ClipCode of second point of line
*/
static ClipCode LineIsUnclippedOnSameSide(int code1, int code2)
{
   return code1 & code2;
}

/**
**	Denotes part of (or entire) line located outside clip rectangle
**      (point 1 and/or 2 is outside clip rectangle)
**
**	@param code1	ClipCode of one point of line
**	@param code2	ClipCode of second point of line
*/
static ClipCode LineIsUnclipped(int code1, int code2)
{
   return code1 | code2;
}

/**
**	Draw line clipped.
**      Based on Sutherland-Cohen clipping technique
**      (Replaces Liang/Barksy clipping algorithm in CVS version 1.18, which
**       might be faster, but that one contained some BUGs)
**
**	@param color	color
**	@param x1	Source x coordinate on the screen
**	@param y1	Source y coordinate on the screen
**	@param x2	Destination x coordinate on the screen
**	@param y2	Destination y coordinate on the screen
*/
global void VideoDrawLineClip(VMemType color, int x1, int y1, int x2, int y2)
/* Based on Sutherland-Cohen clipping technique */
{
    ClipCode code1;
    ClipCode code2;
    int temp;

// Make sure coordinates or on/in clipped rectangle
    while (code1 = ClipCodeLine(x1, y1), code2 = ClipCodeLine(x2, y2),
	    LineIsUnclipped(code1, code2)) {
	if (LineIsUnclippedOnSameSide(code1, code2)) {
	    return;
	}

	if (!code1) {
	    temp = x1; x1 = x2; x2 = temp;
	    temp = y1; y1 = y2; y2 = temp;
	    code1 = code2;
	}

	if (code1 & ClipCodeAbove) {
	    temp = ClipY1;
	    x1 += (int)(((long)(temp - y1) * (x2 - x1)) / (y2 - y1));
	    y1 = temp;
	} else if (code1 & ClipCodeBelow) {
	    temp = ClipY2;
	    x1 += (int)(((long)(temp - y1) * (x2 - x1)) / (y2 - y1));
	    y1 = temp;
	} else if (code1 & ClipCodeLeft) {
	    temp = ClipX1;
	    y1 += (int)(((long)(temp - x1) * (y2 - y1)) / (x2 - x1));
	    x1 = temp;
	} else {  /* code1 & ClipCodeRight */
	    temp = ClipX2;
	    y1 += (int)(((long)(temp - x1) * (y2 - y1)) / (x2 - x1));
	    x1 = temp;
	}
    }

//Draw line based on clipped coordinates
//FIXME: As the clipped coordinates are rounded to integers, the line's
//       direction vector might be slightly off. Somehow, the sub-pixel
//       position(s) on the clipped retangle should be denoted to the line
//       drawing routine..
    DebugCheck(x1 < ClipX1 || x2 < ClipX1 || x1 > ClipX2 || x2 > ClipX2 ||
	y1 < ClipY1 || y2 < ClipY1 || y1 > ClipY2 || y2 > ClipY2);
    VideoDrawLine(color, x1, y1, x2, y2);
}

// ===========================================================================
//	Rectangle
// ===========================================================================

/**
**	Draw rectangle into 8bpp frame buffer.
**
**	@param color	color
**	@param x	x coordinate on the screen
**	@param y	y coordinate on the screen
**	@param h	height of rectangle (0=don't draw).
**	@param w	width of rectangle (0=don't draw).
*/
local void DrawRectangle8(VMemType color, int x, int y, int w, int h)
{
    VMemType8* p;
    VMemType8 f;
    int swidth;
    int ofs;

    f = color.D8;
    swidth = VideoWidth;
    p = VideoMemory8 + y * swidth + x;
    if (h && (ofs = w)) {
	do {
	    *p++ = f; // Draw top horizontal line, FIXME: 4 bytes in one go
	} while (--ofs);
	if (--h) {
	    p += (swidth - w);
	    ofs = w - 1;
	    while (--h) { // Draw vertical line(s)
		p[ofs] = *p = f; //FIXME: draws pixel twice for w == 1 :(
		p += swidth;
	    }
	    do {
		*p++ = f; // Draw bottom horizontal line, FIXME: 4 bytes in one go
	    } while (ofs--);
	}
    }
}

/**
**	Draw rectangle into 16bpp frame buffer.
**
**	@param color	color
**	@param x	x coordinate on the screen
**	@param y	y coordinate on the screen
**	@param h	height of rectangle (0=don't draw).
**	@param w	width of rectangle (0=don't draw).
*/
local void DrawRectangle16(VMemType color, int x, int y, int w, int h)
{
    VMemType16* p;
    VMemType16 f;
    int swidth;
    int ofs;

    f = color.D16;
    swidth = VideoWidth;
    p = VideoMemory16 + y * swidth + x;
    if (h && (ofs = w)) {
	do {
	    *p++ = f; // Draw top horizontal line
	} while (--ofs);
	if (--h) {
	    p += (swidth - w);
	    ofs = w - 1;
	    while (--h) { // Draw vertical line(s)
		p[ofs] = *p = f; //FIXME: draws pixel twice for w == 1 :(
		p += swidth;
	    }
	    do {
		*p++ = f; // Draw bottom horizontal line
	    } while (ofs--);
	}
    }
}

/**
**	Draw rectangle into 24bpp frame buffer.
**
**	@param color	color
**	@param x	x coordinate on the screen
**	@param y	y coordinate on the screen
**	@param h	height of rectangle (0=don't draw).
**	@param w	width of rectangle (0=don't draw).
*/
local void DrawRectangle24(VMemType color, int x, int y
	, int w, int h)
{
    VMemType24* p;
    VMemType24 f;
    int swidth;
    int ofs;

    f = color.D24;
    swidth = VideoWidth;
    p = VideoMemory24 + y * swidth + x;
    if (h && (ofs = w)) {
	do {
	    *p++ = f; // Draw top horizontal line
	} while (--ofs);
	if (--h) {
	    p += (swidth - w);
	    ofs = w - 1;
	    while (--h) { // Draw vertical line(s)
		p[ofs] = *p = f; //FIXME: draws pixel twice for w == 1 :(
		p += swidth;
	    }
	    do {
		*p++ = f; // Draw bottom horizontal line
	    } while (ofs--);
	}
    }
}

/**
**	Draw rectangle into 32bpp frame buffer.
**
**	@param color	color
**	@param x	x coordinate on the screen
**	@param y	y coordinate on the screen
**	@param h	height of rectangle (0=don't draw).
**	@param w	width of rectangle (0=don't draw).
*/
local void DrawRectangle32(VMemType color, int x, int y
	, int w, int h)
{
    VMemType32* p;
    VMemType32 f;
    int swidth;
    int ofs;

    f = color.D32;
    swidth = VideoWidth;
    p = VideoMemory32 + y * swidth + x;
    if (h && (ofs = w)) {
	do {
	    *p++ = f; // Draw top horizontal line
	} while (--ofs);
	if (--h) {
	    p += (swidth - w);
	    ofs = w - 1;
	    while (--h) { // Draw vertical line(s)
		p[ofs] = *p = f; //FIXME: draws pixel twice for w == 1 :(
		p += swidth;
	    }
	    do {
		*p++ = f; // Draw bottom horizontal line
	    } while (ofs--);
	}
    }
}

/**
**	Draw rectangle.
**
**	@param color	color
**	@param x	x coordinate on the screen
**	@param y	y coordinate on the screen
**	@param h	height of rectangle (0=don't draw).
**	@param w	width of rectangle (0=don't draw).
*/
#ifdef USE_OPENGL
local void DrawRectangleOpenGL(VMemType color, int x, int y, int w, int h)
{
    VMemType32 c;
    GLubyte r;
    GLubyte g;
    GLubyte b;

    c = color.D32;
    r = (c >> 16) & 0xff;
    g = (c >> 8) & 0xff;
    b = (c >> 0) & 0xff;
    glDisable(GL_TEXTURE_2D);
    glColor3ub(r, g, b);
    glBegin(GL_LINE_LOOP);
    glVertex2i(x, VideoHeight - y);
    glVertex2i(x + w, VideoHeight - y);
    glVertex2i(x + w, VideoHeight - (y + h));
    glVertex2i(x, VideoHeight - (y + h));
    glEnd();
    glEnable(GL_TEXTURE_2D);
}
#endif

/**
**	Draw 25% translucent rectangle into 8bpp frame buffer.
**
**	@param color	color
**	@param x	x coordinate on the screen
**	@param y	y coordinate on the screen
**	@param h	height of rectangle (0=don't draw).
**	@param w	width of rectangle (0=don't draw).
*/
local void Draw25TransRectangle8(VMemType color, int x, int y, int w, int h)
{
    VMemType8* p;
    int swidth;
    int ofs;
    unsigned c;

    swidth = VideoWidth;
    p = VideoMemory8 + y * swidth + x;
    c = color.D8 << 8;
    if (h && (ofs = w)) {
	do { // Draw top horizontal line
	    *p = lookup25trans8[c | *p];
	    ++p;
	} while (--ofs);

	if (--h) {
	    p += (swidth - w);
	    if ((ofs = w - 1)) {
		while (--h) { // Draw two vertical lines
		    p[ofs] = lookup25trans8[c | p[ofs]];
		    *p = lookup25trans8[c | *p];
		    p += swidth;
		}
	    } else {
		while (--h) { // Draw one vertical line
		    *p = lookup25trans8[c | *p];
		    p += swidth;
		}
	    }

	    do { // Draw bottom horizontal line
		*p = lookup25trans8[c | *p];
		++p;
	    } while (ofs--);
	}
    }
}

/**
**	Draw 25% translucent rectangle into 15bpp frame buffer.
**
**	@param color	color
**	@param x	x coordinate on the screen
**	@param y	y coordinate on the screen
**	@param h	height of rectangle (0=don't draw).
**	@param w	width of rectangle (0=don't draw).
*/
local void Draw25TransRectangle15(VMemType color, int x, int y, int w, int h)
{
    VMemType16* p;
    unsigned long sp;
    unsigned long dp;
    int swidth;
    int ofs;

    sp = color.D16;
    // FIXME: pre multiply?
    sp = (((sp << 16) | sp) & 0x03E07C1F) * 3;
    swidth = VideoWidth;
    p = VideoMemory16 + y * swidth + x;
    if (h && (ofs = w)) {
	do { // Draw top horizontal line
	    dp = *p;
	    dp = ((dp << 16) | dp) & 0x03E07C1F;
	    dp = ((dp + sp) >> 2) & 0x03E07C1F;
	    *p++ = (dp >> 16) | dp;
	} while (--ofs);

	if (--h) {
	    p += (swidth - w);
	    if ((ofs = w - 1)) {
		while (--h) { // Draw two vertical lines
		    dp = p[ofs];
		    dp = ((dp << 16) | dp) & 0x03E07C1F;
		    dp = ((dp + sp) >> 2) & 0x03E07C1F;
		    p[ofs]=(dp >> 16) | dp;

		    dp = *p;
		    dp = ((dp << 16) | dp) & 0x03E07C1F;
		    dp = ((dp + sp) >> 2) & 0x03E07C1F;
		    *p = (dp >> 16) | dp;

		    p += swidth;
		}
	    } else {
		while (--h) { // Draw one vertical line
		    dp = *p;
		    dp = ((dp << 16) | dp) & 0x03E07C1F;
		    dp = ((dp + sp) >> 2) & 0x03E07C1F;
		    *p = (dp >> 16) | dp;
		    p += swidth;
		}
	    }

	    do { // Draw bottom horizontal line
		dp = *p;
		dp = ((dp << 16) | dp) & 0x03E07C1F;
		dp = ((dp + sp) >> 2) & 0x03E07C1F;
		*p++ = (dp >> 16) | dp;
	    } while (ofs--);
	}
    }
}

/**
**	Draw 25% translucent rectangle into 16bpp frame buffer.
**
**	@param color	color
**	@param x	x coordinate on the screen
**	@param y	y coordinate on the screen
**	@param h	height of rectangle (0=don't draw).
**	@param w	width of rectangle (0=don't draw).
*/
local void Draw25TransRectangle16(VMemType color, int x, int y, int w, int h)
{
    VMemType16* p;
    unsigned long sp;
    unsigned long dp;
    int swidth;
    int ofs;

    sp = color.D16;
    // FIXME: pre multiply?
    sp = (((sp << 16) | sp) & 0x07E0F81F) * 3;
    swidth = VideoWidth;
    p = VideoMemory16 + y * swidth + x;
    if (h && (ofs = w)) {
	do { // Draw top horizontal line
	    dp = *p;
	    dp = ((dp << 16) | dp) & 0x07E0F81F;
	    dp = ((dp + sp) >> 2) & 0x07E0F81F;
	    *p++ = (dp >> 16) | dp;
	} while (--ofs);

	if (--h) {
	    p += (swidth - w);
	    if ((ofs = w - 1)) {
		while (--h) { // Draw two vertical lines
		    dp = p[ofs];
		    dp = ((dp << 16) | dp) & 0x07E0F81F;
		    dp = ((dp + sp) >> 2) & 0x07E0F81F;
		    p[ofs]=(dp >> 16) | dp;

		    dp = *p;
		    dp = ((dp << 16) | dp) & 0x07E0F81F;
		    dp = ((dp + sp) >> 2) & 0x07E0F81F;
		    *p = (dp >> 16) | dp;

		    p += swidth;
		}
	    } else {
		while (--h) { // Draw one vertical line
		    dp = *p;
		    dp = ((dp << 16) | dp) & 0x07E0F81F;
		    dp = ((dp + sp) >> 2) & 0x07E0F81F;
		    *p = (dp >> 16) | dp;
		    p += swidth;
		}
	    }

	    do { // Draw bottom horizontal line
		dp = *p;
		dp = ((dp << 16) | dp) & 0x07E0F81F;
		dp = ((dp + sp) >> 2) & 0x07E0F81F;
		*p++ = (dp >> 16) | dp;
	    } while (ofs--);
	}
    }
}

/**
**	Draw 25% translucent rectangle into 24bpp frame buffer.
**
**	@param color	color
**	@param x	x coordinate on the screen
**	@param y	y coordinate on the screen
**	@param h	height of rectangle (0=don't draw).
**	@param w	width of rectangle (0=don't draw).
*/
local void Draw25TransRectangle24(VMemType color, int x, int y, int w, int h)
{
//FIXME: does 24bpp represents R|G|B?
    DrawRectangle24(color, x, y, w, h); // no trans functionaility for the moment :(
}

/**
**	Draw 25% translucent rectangle into 32bpp frame buffer.
**
**	@param color	color
**	@param x	x coordinate on the screen
**	@param y	y coordinate on the screen
**	@param h	height of rectangle (0=don't draw).
**	@param w	width of rectangle (0=don't draw).
*/
local void Draw25TransRectangle32(VMemType color, int x, int y, int w, int h)
{
    VMemType32* p;
    unsigned long sp1;
    unsigned long sp2;
    unsigned long dp1;
    unsigned long dp2;
    int swidth;
    int ofs;

    sp1 = color.D32;
    // FIXME: pre multiply?
    sp2 = ((sp1 & 0xFF00FF00) >> 8) * 3;
    sp1 = (sp1 & 0x00FF00FF) * 3;

    swidth = VideoWidth;
    p = VideoMemory32 + y * swidth + x;
    if (h && (ofs = w)) {
	do { // Draw top horizontal line
	    dp1 = *p;
	    dp2 = (dp1 & 0xFF00FF00) >> 8;
	    dp1 &= 0x00FF00FF;
	    dp1 = ((dp1 + sp1) >> 2) & 0x00FF00FF;
	    dp2 = ((dp2 + sp2) >> 2) & 0x00FF00FF;
	    *p++ = (dp1 | (dp2 << 8));
	} while (--ofs);

	if (--h) {
	    p += (swidth - w);
	    if ((ofs = w - 1)) {
		while (--h) { // Draw two vertical lines
		    dp1 = p[ofs];
		    dp2 = (dp1 & 0xFF00FF00) >> 8;
		    dp1 &= 0x00FF00FF;
		    dp1 = ((dp1 + sp1) >> 2) & 0x00FF00FF;
		    dp2 = ((dp2 + sp2) >> 2) & 0x00FF00FF;
		    p[ofs] = (dp1 | (dp2 << 8));

		    dp1 = *p;
		    dp2 = (dp1 & 0xFF00FF00) >> 8;
		    dp1 &= 0x00FF00FF;
		    dp1 = ((dp1 + sp1) >> 2) & 0x00FF00FF;
		    dp2 = ((dp2 + sp2) >> 2) & 0x00FF00FF;
		    *p = (dp1 | (dp2 << 8));

		    p += swidth;
		}
	    } else {
		while (--h) { // Draw one vertical line
		    dp1 = *p;
		    dp2 = (dp1 & 0xFF00FF00) >> 8;
		    dp1 &= 0x00FF00FF;
		    dp1 = ((dp1 + sp1) >> 2) & 0x00FF00FF;
		    dp2 = ((dp2 + sp2) >> 2) & 0x00FF00FF;
		    *p++ = (dp1 | (dp2 << 8));
		    p += swidth;
		}
	    }

	    do { // Draw bottom horizontal line
		dp1 = *p;
		dp2 = (dp1 & 0xFF00FF00) >> 8;
		dp1 &= 0x00FF00FF;
		dp1 = ((dp1 + sp1) >> 2) & 0x00FF00FF;
		dp2 = ((dp2 + sp2) >> 2) & 0x00FF00FF;
		*p++ = (dp1 | (dp2 << 8));
	    } while (ofs--);
	}
    }
}

/**
**	Draw 25% translucent rectangle.
**
**	@param color	color
**	@param x	x coordinate on the screen
**	@param y	y coordinate on the screen
**	@param h	height of rectangle (0=don't draw).
**	@param w	width of rectangle (0=don't draw).
*/
#ifdef USE_OPENGL
local void Draw25TransRectangleOpenGL(VMemType color, int x, int y,
    int w, int h)
{
    VMemType32 c;
    GLubyte r;
    GLubyte g;
    GLubyte b;

    c = color.D32;
    r = (c >> 16) & 0xff;
    g = (c >> 8) & 0xff;
    b = (c >> 0) & 0xff;
    glDisable(GL_TEXTURE_2D);
    glColor4ub(r, g, b, 192);
    glBegin(GL_LINE_LOOP);
    glVertex2i(x, VideoHeight - y);
    glVertex2i(x + w, VideoHeight - y);
    glVertex2i(x + w, VideoHeight - (y + h));
    glVertex2i(x, VideoHeight - (y + h));
    glEnd();
    glEnable(GL_TEXTURE_2D);
}
#endif

/**
**	Draw 50% translucent rectangle into 8bpp frame buffer.
**
**	@param color	color
**	@param x	x coordinate on the screen
**	@param y	y coordinate on the screen
**	@param h	height of rectangle (0=don't draw).
**	@param w	width of rectangle (0=don't draw).
*/
local void Draw50TransRectangle8(VMemType color, int x, int y, int w, int h)
{
    VMemType8* p;
    int swidth;
    int ofs;
    unsigned c;

    swidth = VideoWidth;
    p = VideoMemory8 + y * swidth + x;
    c = color.D8 << 8;
    if (h && (ofs = w)) {
	do { // Draw top horizontal line
	    *p = lookup50trans8[c | *p];
	    ++p;
	} while (--ofs);

	if (--h) {
	    p += (swidth - w);
	    if ((ofs = w - 1)) {
		while (--h) { // Draw two vertical lines
		    p[ofs] = lookup50trans8[c | p[ofs]];
		    *p = lookup50trans8[c | *p];
		    p += swidth;
		}
	    } else {
		while (--h) { // Draw one vertical line
		    *p = lookup50trans8[c | *p];
		    p += swidth;
		}
	    }

	    do { // Draw bottom horizontal line
		*p = lookup50trans8[c | *p];
		++p;
	    } while (ofs--);
	}
    }
}

/**
**	Draw 50% translucent rectangle into 15bpp frame buffer.
**
**	@param color	color
**	@param x	x coordinate on the screen
**	@param y	y coordinate on the screen
**	@param h	height of rectangle (0=don't draw).
**	@param w	width of rectangle (0=don't draw).
*/
local void Draw50TransRectangle15(VMemType color, int x, int y, int w, int h)
{
    VMemType16* p;
    unsigned long sp;
    unsigned long dp;
    int swidth;
    int ofs;

    sp = color.D16;
    sp = ((sp << 16) | sp) & 0x03E07C1F;
    swidth = VideoWidth;
    p = VideoMemory16 + y * swidth + x;
    if (h && (ofs = w)) {
	do { // Draw top horizontal line
	    dp = *p;
	    dp = ((dp << 16) | dp) & 0x03E07C1F;
	    dp = ((dp + sp) >> 1) & 0x03E07C1F;
	    *p++ = (dp >> 16) | dp;
	} while (--ofs);

	if (--h) {
	    p += (swidth - w);
	    if ((ofs = w - 1)) {
		while (--h) { // Draw two vertical lines
		    dp = p[ofs];
		    dp = ((dp << 16) | dp) & 0x03E07C1F;
		    dp = ((dp + sp) >> 1) & 0x03E07C1F;
		    p[ofs] = (dp >> 16) | dp;

		    dp = *p;
		    dp = ((dp << 16) | dp) & 0x03E07C1F;
		    dp = ((dp + sp) >> 1) & 0x03E07C1F;
		    *p = (dp >> 16) | dp;

		    p += swidth;
		}
	    } else {
		while (--h) { // Draw one vertical line
		    dp = *p;
		    dp = ((dp << 16) | dp) & 0x03E07C1F;
		    dp = ((dp + sp) >> 1) & 0x03E07C1F;
		    *p = (dp >> 16) | dp;
		    p += swidth;
		}
	    }

	    do { // Draw bottom horizontal line
		dp = *p;
		dp = ((dp << 16) | dp) & 0x03E07C1F;
		dp = ((dp + sp) >> 1) & 0x03E07C1F;
		*p++ = (dp >> 16) | dp;
	    } while (ofs--);
	}
    }
}

/**
**	Draw 50% translucent rectangle into 16bpp frame buffer.
**
**	@param color	color
**	@param x	x coordinate on the screen
**	@param y	y coordinate on the screen
**	@param h	height of rectangle (0=don't draw).
**	@param w	width of rectangle (0=don't draw).
*/
local void Draw50TransRectangle16(VMemType color, int x, int y, int w, int h)
{
    VMemType16* p;
    unsigned long sp;
    unsigned long dp;
    int swidth;
    int ofs;

    sp = color.D16;
    sp = ((sp << 16) | sp) & 0x07E0F81F;
    swidth = VideoWidth;
    p = VideoMemory16 + y * swidth + x;
    if (h && (ofs = w)) {
	do { // Draw top horizontal line
	    dp = *p;
	    dp = ((dp << 16) | dp) & 0x07E0F81F;
	    dp = ((dp + sp) >> 1) & 0x07E0F81F;
	    *p++ = (dp >> 16) | dp;
	} while (--ofs);

	if (--h) {
	    p += (swidth - w);
	    if ((ofs = w - 1)) {
		while (--h) { // Draw two vertical lines
		    dp = p[ofs];
		    dp = ((dp << 16) | dp) & 0x07E0F81F;
		    dp = ((dp + sp) >> 1) & 0x07E0F81F;
		    p[ofs] = (dp >> 16) | dp;

		    dp = *p;
		    dp = ((dp << 16) | dp) & 0x07E0F81F;
		    dp = ((dp + sp) >> 1) & 0x07E0F81F;
		    *p = (dp >> 16) | dp;

		    p += swidth;
		}
	    } else {
		while (--h) { // Draw one vertical line
		    dp = *p;
		    dp = ((dp << 16) | dp) & 0x07E0F81F;
		    dp = ((dp + sp) >> 1) & 0x07E0F81F;
		    *p = (dp >> 16) | dp;
		    p += swidth;
		}
	    }

	    do { // Draw bottom horizontal line
		dp = *p;
		dp = ((dp << 16) | dp) & 0x07E0F81F;
		dp = ((dp + sp) >> 1) & 0x07E0F81F;
		*p++ = (dp >> 16) | dp;
	    } while (ofs--);
	}
    }
}

/**
**	Draw 50% translucent rectangle into 24bpp frame buffer.
**
**	@param color	color
**	@param x	x coordinate on the screen
**	@param y	y coordinate on the screen
**	@param h	height of rectangle (0=don't draw).
**	@param w	width of rectangle (0=don't draw).
*/
local void Draw50TransRectangle24(VMemType color, int x, int y, int w, int h)
{
//FIXME: does 24bpp represents R|G|B?
    DrawRectangle24(color, x, y, w, h); // no trans functionaility for the moment :(
}

/**
**	Draw 50% translucent rectangle into 32bpp frame buffer.
**
**	@param color	color
**	@param x	x coordinate on the screen
**	@param y	y coordinate on the screen
**	@param h	height of rectangle (0=don't draw).
**	@param w	width of rectangle (0=don't draw).
*/
local void Draw50TransRectangle32(VMemType color, int x, int y, int w, int h)
{
    VMemType32* p;
    unsigned long sp1;
    unsigned long sp2;
    unsigned long dp1;
    unsigned long dp2;
    int swidth;
    int ofs;

    sp1 = color.D32;
    // FIXME: pre multiply?
    sp2 = (sp1 & 0xFF00FF00) >> 8;
    sp1 &= 0x00FF00FF;

    swidth = VideoWidth;
    p = VideoMemory32 + y * swidth + x;
    if (h && (ofs = w)) {
	do { // Draw top horizontal line
	    dp1 = *p;
	    dp2 = (dp1 & 0xFF00FF00) >> 8;
	    dp1 &= 0x00FF00FF;
	    dp1 = ((dp1 + sp1) >> 1) & 0x00FF00FF;
	    dp2 = ((dp2 + sp2) >> 1) & 0x00FF00FF;
	    *p++ = (dp1 | (dp2 << 8));
	} while (--ofs);

	if (--h) {
	    p += (swidth - w);
	    if ((ofs = w - 1)) {
		while (--h) { // Draw two vertical lines
		    dp1 = p[ofs];
		    dp2 = (dp1 & 0xFF00FF00) >> 8;
		    dp1 &= 0x00FF00FF;
		    dp1 = ((dp1 + sp1) >> 1) & 0x00FF00FF;
		    dp2 = ((dp2 + sp2) >> 1) & 0x00FF00FF;
		    p[ofs] = (dp1 | (dp2 << 8));

		    dp1 = *p;
		    dp2 = (dp1 & 0xFF00FF00) >> 8;
		    dp1 &= 0x00FF00FF;
		    dp1 = ((dp1 + sp1) >> 1) & 0x00FF00FF;
		    dp2 = ((dp2 + sp2) >> 1) & 0x00FF00FF;
		    *p = (dp1 | (dp2 << 8));

		    p += swidth;
		}
	    } else {
		while (--h) { // Draw one vertical line
		    dp1 = *p;
		    dp2 = (dp1 & 0xFF00FF00) >> 8;
		    dp1 &= 0x00FF00FF;
		    dp1 = ((dp1 + sp1) >> 1) & 0x00FF00FF;
		    dp2 = ((dp2 + sp2) >> 1) & 0x00FF00FF;
		    *p++ = (dp1 | (dp2 << 8));
		    p += swidth;
		}
	    }

	    do { // Draw bottom horizontal line
		dp1 = *p;
		dp2 = (dp1 & 0xFF00FF00) >> 8;
		dp1 &= 0x00FF00FF;
		dp1 = ((dp1 + sp1) >> 1) & 0x00FF00FF;
		dp2 = ((dp2 + sp2) >> 1) & 0x00FF00FF;
		*p++ = (dp1 | (dp2 << 8));
	    } while (ofs--);
	}
    }
}

/**
**	Draw 50% translucent rectangle into 32bpp frame buffer.
**
**	@param color	color
**	@param x	x coordinate on the screen
**	@param y	y coordinate on the screen
**	@param h	height of rectangle (0=don't draw).
**	@param w	width of rectangle (0=don't draw).
*/
#ifdef USE_OPENGL
local void Draw50TransRectangleOpenGL(VMemType color, int x, int y,
    int w, int h)
{
    VMemType32 c;
    GLubyte r;
    GLubyte g;
    GLubyte b;

    c = color.D32;
    r = (c >> 16) & 0xff;
    g = (c >> 8) & 0xff;
    b = (c >> 0) & 0xff;
    glDisable(GL_TEXTURE_2D);
    glColor4ub(r, g, b, 128);
    glBegin(GL_LINE_LOOP);
    glVertex2i(x, VideoHeight - y);
    glVertex2i(x + w, VideoHeight - y);
    glVertex2i(x + w, VideoHeight - (y + h));
    glVertex2i(x, VideoHeight - (y + h));
    glEnd();
    glEnable(GL_TEXTURE_2D);
}
#endif

/**
**	Draw 75% translucent rectangle into 8bpp frame buffer.
**
**	@param color	color
**	@param x	x coordinate on the screen
**	@param y	y coordinate on the screen
**	@param h	height of rectangle (0=don't draw).
**	@param w	width of rectangle (0=don't draw).
*/
local void Draw75TransRectangle8(VMemType color, int x, int y, int w, int h)
{
    VMemType8* p;
    int swidth;
    int ofs;
    unsigned c;

    swidth = VideoWidth;
    p = VideoMemory8 + y * swidth + x;
    c = color.D8;
    if (h && (ofs = w)) {
	do { // Draw top horizontal line
	    *p = lookup25trans8[(*p << 8) | c];
	    ++p;
	} while (--ofs);

	if (--h) {
	    p += (swidth - w);
	    if ((ofs = w - 1)) {
		while (--h) { // Draw two vertical lines
		    p[ofs] = lookup25trans8[(p[ofs] << 8) | c];
		    *p = lookup25trans8[(*p << 8) | c];
		    p += swidth;
		}
	    } else {
		while (--h) { // Draw one vertical line
		    *p = lookup25trans8[(*p << 8) | c];
		    p += swidth;
		}

		do { // Draw bottom horizontal line
		    *p = lookup25trans8[(*p << 8) | c];
		    ++p;
		} while (ofs--);
	    }
	}
    }
}

/**
**	Draw 75% translucent rectangle into 15bpp frame buffer.
**
**	@param color	color
**	@param x	x coordinate on the screen
**	@param y	y coordinate on the screen
**	@param h	height of rectangle (0=don't draw).
**	@param w	width of rectangle (0=don't draw).
*/
local void Draw75TransRectangle15(VMemType color, int x, int y, int w, int h)
{
    VMemType16* p;
    unsigned long sp;
    unsigned long dp;
    int swidth;
    int ofs;

    sp = color.D16;
    sp = ((sp << 16) | sp) & 0x03E07C1F;
    swidth = VideoWidth;
    p = VideoMemory16 + y * swidth + x;
    if (h && (ofs = w)) {
	do { // Draw top horizontal line
	    dp = *p;
	    dp = ((dp << 16) | dp) & 0x03E07C1F;
	    dp = (((dp << 1) + dp + sp) >> 2) & 0x03E07C1F;
	    *p++ = (dp >> 16) | dp;
	} while (--ofs);

	if (--h) {
	    p += (swidth - w);
	    if ((ofs = w - 1)) {
		while (--h) { // Draw two vertical lines
		    dp = p[ofs];
		    dp = ((dp << 16) | dp) & 0x03E07C1F;
		    dp = (((dp << 1) + dp + sp) >> 2) & 0x03E07C1F;
		    p[ofs] = (dp >> 16) | dp;

		    dp = *p;
		    dp = ((dp << 16) | dp) & 0x03E07C1F;
		    dp = (((dp << 1) + dp + sp) >> 2) & 0x03E07C1F;
		    *p = (dp >> 16) | dp;

		    p += swidth;
		}
	    } else {
		while (--h) { // Draw one vertical line
		    dp = *p;
		    dp = ((dp << 16) | dp) & 0x03E07C1F;
		    dp = (((dp << 1) + dp + sp) >> 2) & 0x03E07C1F;
		    *p = (dp >> 16) | dp;
		    p += swidth;
		}
	    }

	    do { // Draw bottom horizontal line
		dp = *p;
		dp = ((dp << 16) | dp) & 0x03E07C1F;
		dp = (((dp << 1) + dp + sp) >> 2) & 0x03E07C1F;
		*p++ = (dp >> 16) | dp;
	    } while (ofs--);
	}
    }
}

/**
**	Draw 75% translucent rectangle into 16bpp frame buffer.
**
**	@param color	color
**	@param x	x coordinate on the screen
**	@param y	y coordinate on the screen
**	@param h	height of rectangle (0=don't draw).
**	@param w	width of rectangle (0=don't draw).
*/
local void Draw75TransRectangle16(VMemType color, int x, int y, int w, int h)
{
    VMemType16* p;
    unsigned long sp;
    unsigned long dp;
    int swidth;
    int ofs;

    sp = color.D16;
    sp = ((sp << 16) | sp) & 0x07E0F81F;
    swidth = VideoWidth;
    p = VideoMemory16 + y * swidth + x;
    if (h && (ofs = w)) {
	do { // Draw top horizontal line
	    dp = *p;
	    dp = ((dp << 16) | dp) & 0x07E0F81F;
	    dp = (((dp << 1) + dp + sp) >> 2) & 0x07E0F81F;
	    *p++ = (dp >> 16) | dp;
	} while (--ofs);

	if (--h) {
	    p += (swidth - w);
	    if ((ofs = w - 1)) {
		while (--h) { // Draw two vertical lines
		    dp = p[ofs];
		    dp = ((dp << 16) | dp) & 0x07E0F81F;
		    dp = (((dp << 1) + dp + sp) >> 2) & 0x07E0F81F;
		    p[ofs] = (dp >> 16) | dp;

		    dp = *p;
		    dp = ((dp << 16) | dp) & 0x07E0F81F;
		    dp = (((dp << 1) + dp + sp) >> 2) & 0x07E0F81F;
		    *p = (dp >> 16) | dp;

		    p += swidth;
		}
	    } else {
		while (--h) { // Draw one vertical line
		    dp = *p;
		    dp = ((dp << 16) | dp) & 0x07E0F81F;
		    dp = (((dp << 1) + dp + sp) >> 2) & 0x07E0F81F;
		    *p = (dp >> 16) | dp;
		    p += swidth;
		}
	    }

	    do { // Draw bottom horizontal line
		dp = *p;
		dp = ((dp << 16) | dp) & 0x07E0F81F;
		dp = (((dp << 1) + dp + sp) >> 2) & 0x07E0F81F;
		*p++ = (dp >> 16) | dp;
	    } while (ofs--);
	}
    }
}

/**
**	Draw 75% translucent rectangle into 24bpp frame buffer.
**
**	@param color	color
**	@param x	x coordinate on the screen
**	@param y	y coordinate on the screen
**	@param h	height of rectangle (0=don't draw).
**	@param w	width of rectangle (0=don't draw).
*/
local void Draw75TransRectangle24(VMemType color, int x, int y, int w, int h)
{
//FIXME: does 24bpp represents R|G|B?
    DrawRectangle24(color, x, y, w, h); // no trans functionaility for the moment :(
}

/**
**	Draw 75% translucent rectangle into 32bpp frame buffer.
**
**	@param color	color
**	@param x	x coordinate on the screen
**	@param y	y coordinate on the screen
**	@param h	height of rectangle (0=don't draw).
**	@param w	width of rectangle (0=don't draw).
*/
local void Draw75TransRectangle32(VMemType color, int x, int y, int w, int h)
{
    VMemType32* p;
    unsigned long sp1;
    unsigned long sp2;
    unsigned long dp1;
    unsigned long dp2;
    int swidth;
    int ofs;

    sp1 = color.D32;
    // FIXME: pre multiply?
    sp2 = (sp1 & 0xFF00FF00) >> 8;
    sp1 &= 0x00FF00FF;

    swidth = VideoWidth;
    p = VideoMemory32 + y * swidth + x;
    if (h && (ofs = w)) {
	do { // Draw top horizontal line
	    dp1 = *p;
	    dp2 = (dp1 & 0xFF00FF00) >> 8;
	    dp1 &= 0x00FF00FF;
	    dp1 = (((dp1 << 1) + dp1 + sp1) >> 2) & 0x00FF00FF;
	    dp2 = (((dp2 << 1) + dp2 + sp2) >> 2) & 0x00FF00FF;
	    *p++ = (dp1 | (dp2 << 8));
	} while (--ofs);

	if (--h) {
	    p += (swidth - w);
	    if ((ofs = w - 1)) {
		while (--h) { // Draw two vertical lines
		    dp1 = p[ofs];
		    dp2 = (dp1 & 0xFF00FF00) >> 8;
		    dp1 &= 0x00FF00FF;
		    dp1 = (((dp1 << 1) + dp1 + sp1) >> 2) & 0x00FF00FF;
		    dp2 = (((dp2 << 1) + dp2 + sp2) >> 2) & 0x00FF00FF;
		    p[ofs] = (dp1 | (dp2 << 8));

		    dp1 = *p;
		    dp2 = (dp1 & 0xFF00FF00) >> 8;
		    dp1 &= 0x00FF00FF;
		    dp1 = (((dp1 << 1) + dp1 + sp1) >> 2) & 0x00FF00FF;
		    dp2 = (((dp2 << 1) + dp2 + sp2) >> 2) & 0x00FF00FF;
		    *p = (dp1 | (dp2 << 8));

		    p += swidth;
		}
	    } else {
		while (--h) { // Draw one vertical line
		    dp1 = *p;
		    dp2 = (dp1 & 0xFF00FF00) >> 8;
		    dp1 &= 0x00FF00FF;
		    dp1 = (((dp1 << 1) + dp1 + sp1) >> 2) & 0x00FF00FF;
		    dp2 = (((dp2 << 1) + dp2 + sp2) >> 2) & 0x00FF00FF;
		    *p++ = (dp1 | (dp2 << 8));
		    p += swidth;
		}
	    }

	    do { // Draw bottom horizontal line
		dp1 = *p;
		dp2 = (dp1 & 0xFF00FF00) >> 8;
		dp1 &= 0x00FF00FF;
		dp1 = (((dp1 << 1) + dp1 + sp1) >> 2) & 0x00FF00FF;
		dp2 = (((dp2 << 1) + dp2 + sp2) >> 2) & 0x00FF00FF;
		*p++ = (dp1 | (dp2 << 8));
	    } while (ofs--);
	}
    }
}

/**
**	Draw 75% translucent rectangle.
**
**	@param color	color
**	@param x	x coordinate on the screen
**	@param y	y coordinate on the screen
**	@param h	height of rectangle (0=don't draw).
**	@param w	width of rectangle (0=don't draw).
*/
#ifdef USE_OPENGL
local void Draw75TransRectangleOpenGL(VMemType color, int x, int y,
    int w, int h)
{
    VMemType32 c;
    GLubyte r;
    GLubyte g;
    GLubyte b;

    c = color.D32;
    r = (c >> 16) & 0xff;
    g = (c >> 8) & 0xff;
    b = (c >> 0) & 0xff;
    glDisable(GL_TEXTURE_2D);
    glColor4ub(r, g, b, 64);
    glBegin(GL_LINE_LOOP);
    glVertex2i(x, VideoHeight - y);
    glVertex2i(x + w, VideoHeight - y);
    glVertex2i(x + w, VideoHeight - (y + h));
    glVertex2i(x, VideoHeight - (y + h));
    glEnd();
    glEnable(GL_TEXTURE_2D);
}
#endif

/**
**	Draw translucent rectangle into 8bpp frame buffer.
**
**	@param color	color
**	@param x	x coordinate on the screen
**	@param y	y coordinate on the screen
**	@param h	height of rectangle (0=don't draw).
**	@param w	width of rectangle (0=don't draw).
**      @param alpha    alpha value of pixel.
*/
local void DrawTransRectangle8(VMemType color, int x, int y,
    int w, int h, unsigned char alpha)
{
    switch (((unsigned int)alpha * 4) / 255) {
	case 0:
	    DrawRectangle8(color, x, y, w, h);
	    break;
	case 1:
	    Draw25TransRectangle8(color, x, y, w, h);
	    break;
	case 2:
	    Draw50TransRectangle8(color, x, y, w, h);
	    break;
	case 3:
	    Draw75TransRectangle8(color, x, y, w, h);
	    break;
	default:
	    break;
    }
}

/**
**	Draw rectangle into 8bpp frame buffer (ignoring alpha).
**
**	@param color	Color index
**	@param x	X pixel coordinate on the screen
**	@param y	Y pixel coordinate on the screen
**	@param w	Width in pixel of the rectangle
**	@param h	Height in pixel of the rectangle
**      @param alpha    Alpha value of pixel
*/
local void DrawNoTransRectangle8(VMemType color, int x, int y,
    int w, int h, unsigned char alpha __attribute__((unused)))
{
    DrawRectangle8(color, x, y, w, h);
}

/**
**	Draw translucent rectangle into 15bpp frame buffer.
**
**	@param color	color
**	@param x	x coordinate on the screen
**	@param y	y coordinate on the screen
**	@param h	height of rectangle (0=don't draw).
**	@param w	width of rectangle (0=don't draw).
**      @param alpha    alpha value of pixel.
*/
local void DrawTransRectangle15(VMemType color, int x, int y,
    int w, int h, unsigned char alpha)
{
    VMemType16* p;
    unsigned long sp;
    unsigned long dp;
    int swidth;
    int ofs;

    sp = color.D16;
    sp = ((sp << 16) | sp) & 0x03E07C1F;
    swidth = VideoWidth;
    p = VideoMemory16 + y * swidth + x;
    //FIXME: alpha==256 unreached
    alpha >>= 3;                          //FIXME: only 5bits
    if (h && (ofs = w)) {
	do { // Draw top horizontal line
	    dp = *p;
	    dp = ((dp << 16) | dp) & 0x03E07C1F;
	    dp = ((((dp - sp) * alpha) >> 5) + sp) & 0x03E07C1F;
	    *p++ = (dp >> 16) | dp;
	} while (--ofs);

	if (--h) {
	    p += (swidth - w);
	    if ((ofs = w - 1)) {
		while (--h) { // Draw two vertical lines
		    dp = p[ofs];
		    dp = ((dp << 16) | dp) & 0x03E07C1F;
		    dp = ((((dp - sp) * alpha) >> 5) + sp) & 0x03E07C1F;
		    p[ofs] = (dp >> 16) | dp;

		    dp = *p;
		    dp = ((dp << 16) | dp) & 0x03E07C1F;
		    dp = ((((dp - sp) * alpha) >> 5) + sp) & 0x03E07C1F;
		    *p = (dp >> 16) | dp;

		    p += swidth;
		}
	    } else {
		while (--h) { // Draw one vertical line
		    dp = *p;
		    dp = ((dp << 16) | dp) & 0x03E07C1F;
		    dp = ((((dp - sp) * alpha) >> 5) + sp) & 0x03E07C1F;
		    *p = (dp >> 16) | dp;
		    p += swidth;
		}
	    }

	    do { // Draw bottom horizontal line
		dp = *p;
		dp = ((dp << 16) | dp) & 0x03E07C1F;
		dp = ((((dp - sp) * alpha) >> 5) + sp) & 0x03E07C1F;
		*p++ = (dp >> 16) | dp;
	    } while (ofs--);
	}
    }
}

/**
**	Draw translucent rectangle into 16bpp frame buffer.
**
**	@param color	color
**	@param x	x coordinate on the screen
**	@param y	y coordinate on the screen
**	@param h	height of rectangle (0=don't draw).
**	@param w	width of rectangle (0=don't draw).
**      @param alpha    alpha value of pixel.
*/
local void DrawTransRectangle16(VMemType color, int x, int y,
    int w, int h, unsigned char alpha)
{
    VMemType16* p;
    unsigned long sp;
    unsigned long dp;
    int swidth;
    int ofs;

    sp = color.D16;
    sp = ((sp << 16) | sp) & 0x07E0F81F;
    swidth = VideoWidth;
    p = VideoMemory16 + y * swidth + x;
    //FIXME: alpha==256 unreached
    alpha >>= 3;                          //FIXME: only 5bits
    if (h && (ofs = w)) {
	do { // Draw top horizontal line
	    dp = *p;
	    dp = ((dp << 16) | dp) & 0x07E0F81F;
	    dp = ((((dp - sp) * alpha) >> 5) + sp) & 0x07E0F81F;
	    *p++ = (dp >> 16) | dp;
	} while (--ofs);

	if (--h) {
	    p += (swidth - w);
	    if ((ofs = w - 1)) {
		while (--h) { // Draw two vertical lines
		    dp = p[ofs];
		    dp = ((dp << 16) | dp) & 0x07E0F81F;
		    dp = ((((dp - sp) * alpha) >> 5) + sp) & 0x07E0F81F;
		    p[ofs] = (dp >> 16) | dp;

		    dp = *p;
		    dp = ((dp << 16) | dp) & 0x07E0F81F;
		    dp = ((((dp - sp) * alpha) >> 5) + sp) & 0x07E0F81F;
		    *p = (dp >> 16) | dp;

		    p += swidth;
		}
	    } else {
		while (--h) { // Draw one vertical line
		    dp = *p;
		    dp = ((dp << 16) | dp) & 0x07E0F81F;
		    dp = ((((dp - sp) * alpha) >> 5) + sp) & 0x07E0F81F;
		    *p = (dp >> 16) | dp;
		    p += swidth;
		}
	    }

	    do { // Draw bottom horizontal line
		dp = *p;
		dp = ((dp << 16) | dp) & 0x07E0F81F;
		dp = ((((dp - sp) * alpha) >> 5) + sp) & 0x07E0F81F;
		*p++ = (dp >> 16) | dp;
	    } while (ofs--);
	}
    }
}

/**
**	Draw translucent rectangle into 24bpp frame buffer.
**
**	@param color	color
**	@param x	x coordinate on the screen
**	@param y	y coordinate on the screen
**	@param h	height of rectangle (0=don't draw).
**	@param w	width of rectangle (0=don't draw).
**      @param alpha    alpha value of pixel.
*/
local void DrawTransRectangle24(VMemType color, int x, int y,
    int w, int h, unsigned char alpha __attribute__((unused)))
{
//FIXME: does 24bpp represents R|G|B?
    DrawRectangle24(color, x, y, w, h); // no trans functionaility for the moment :(
}

/**
**	Draw translucent rectangle into 32bpp frame buffer.
**
**	@param color	color
**	@param x	x coordinate on the screen
**	@param y	y coordinate on the screen
**	@param h	height of rectangle (0=don't draw).
**	@param w	width of rectangle (0=don't draw).
**      @param alpha    alpha value of pixel.
*/
local void DrawTransRectangle32(VMemType color, int x, int y,
    int w, int h, unsigned char alpha)
{
    VMemType32* p;
    unsigned long sp1;
    unsigned long sp2;
    unsigned long dp1;
    unsigned long dp2;
    int swidth;
    int ofs;

    sp1 = color.D32;
    // FIXME: pre multiply?
    sp2 = (sp1 & 0xFF00FF00) >> 8;
    sp1 &= 0x00FF00FF;

    //FIXME: alpha==256 unreached
    swidth = VideoWidth;
    p = VideoMemory32 + y * swidth + x;
    if (h && (ofs = w)) {
	do { // Draw top horizontal line
	    dp1 = *p;
	    dp2 = (dp1 & 0xFF00FF00) >> 8;
	    dp1 &= 0x00FF00FF;
	    dp1 = ((((dp1-sp1) * alpha) >> 8) + sp1) & 0x00FF00FF;
	    dp2 = ((((dp2-sp2) * alpha) >> 8) + sp2) & 0x00FF00FF;
	    *p++ = (dp1 | (dp2 << 8));
	} while (--ofs);
	if (--h) {
	    p += (swidth - w);
	    if ((ofs = w - 1)) {
		while (--h) { // Draw two vertical lines
		    dp1 = p[ofs];
		    dp2 = (dp1 & 0xFF00FF00) >> 8;
		    dp1 &= 0x00FF00FF;
		    dp1 = ((((dp1-sp1) * alpha) >> 8) + sp1) & 0x00FF00FF;
		    dp2 = ((((dp2-sp2) * alpha) >> 8) + sp2) & 0x00FF00FF;
		    p[ofs] = (dp1 | (dp2 << 8));

		    dp1 = *p;
		    dp2 = (dp1 & 0xFF00FF00) >> 8;
		    dp1 &= 0x00FF00FF;
		    dp1 = ((((dp1-sp1) * alpha) >> 8) + sp1) & 0x00FF00FF;
		    dp2 = ((((dp2-sp2) * alpha) >> 8) + sp2) & 0x00FF00FF;
		    *p = (dp1 | (dp2 << 8));

		    p += swidth;
		}
	    } else {
		while (--h) { // Draw one vertical line
		    dp1 = *p;
		    dp2 = (dp1 & 0xFF00FF00) >> 8;
		    dp1 &= 0x00FF00FF;
		    dp1 = ((((dp1-sp1) * alpha) >> 8) + sp1) & 0x00FF00FF;
		    dp2 = ((((dp2-sp2) * alpha) >> 8) + sp2) & 0x00FF00FF;
		    *p = (dp1 | (dp2 << 8));
		    p += swidth;
		}
	    }

	    do { // Draw bottom horizontal line
		dp1 = *p;
		dp2 = (dp1 & 0xFF00FF00) >> 8;
		dp1 &= 0x00FF00FF;
		dp1 = ((((dp1-sp1) * alpha) >> 8) + sp1) & 0x00FF00FF;
		dp2 = ((((dp2-sp2) * alpha) >> 8) + sp2) & 0x00FF00FF;
		*p++ = (dp1 | (dp2 << 8));
	    } while (ofs--);
	}
    }
}

/**
**	Draw translucent rectangle.
**
**	@param color	color
**	@param x	x coordinate on the screen
**	@param y	y coordinate on the screen
**	@param h	height of rectangle (0=don't draw).
**	@param w	width of rectangle (0=don't draw).
**      @param alpha    alpha value of pixel.
*/
#ifdef USE_OPENGL
local void DrawTransRectangleOpenGL(VMemType color, int x, int y,
    int w, int h, unsigned char alpha)
{
    VMemType32 c;
    GLubyte r;
    GLubyte g;
    GLubyte b;

    c = color.D32;
    r = (c >> 16) & 0xff;
    g = (c >> 8) & 0xff;
    b = (c >> 0) & 0xff;
    glDisable(GL_TEXTURE_2D);
    glColor4ub(r, g, b, 255 - alpha);
    glBegin(GL_LINE_LOOP);
    glVertex2i(x, VideoHeight - y);
    glVertex2i(x + w, VideoHeight - y);
    glVertex2i(x + w, VideoHeight - (y + h));
    glVertex2i(x, VideoHeight - (y + h));
    glEnd();
    glEnable(GL_TEXTURE_2D);
}
#endif

/**
**	Draw rectangle clipped.
**
**	@param color	color
**	@param x	x coordinate on the screen
**	@param y	y coordinate on the screen
**	@param h	height of rectangle (0=don't draw).
**	@param w	width of rectangle (0=don't draw).
*/
global void VideoDrawRectangleClip(VMemType color, int x, int y,
    int w, int h)
{
    #define _x              x
    #define _y              y
    #define _w              w
    #define _h              h
    #define _hline(x, y,w)   VideoDrawHLine(color, x, y,w)
    #define _vline(x, y,h)   VideoDrawVLine(color, x, y,h)

    #include "_clip_rectangle"

    #undef _x
    #undef _y
    #undef _w
    #undef _h
    #undef _hline
    #undef _vline
}

/**
**	Draw 25% translucent rectangle clipped.
**
**	@param color	color
**	@param x	x coordinate on the screen
**	@param y	y coordinate on the screen
**	@param h	height of rectangle (0=don't draw).
**	@param w	width of rectangle (0=don't draw).
*/
global void VideoDraw25TransRectangleClip(VMemType color, int x, int y,
    int w, int h)
{
    #define _x              x
    #define _y              y
    #define _w              w
    #define _h              h
    #define _hline(x, y,w)   VideoDraw25TransHLine(color, x, y,w)
    #define _vline(x, y,h)   VideoDraw25TransVLine(color, x, y,h)

    #include "_clip_rectangle"

    #undef _x
    #undef _y
    #undef _w
    #undef _h
    #undef _hline
    #undef _vline
}

/**
**	Draw 50% translucent rectangle clipped.
**
**	@param color	color
**	@param x	x coordinate on the screen
**	@param y	y coordinate on the screen
**	@param h	height of rectangle (0=don't draw).
**	@param w	width of rectangle (0=don't draw).
*/
global void VideoDraw50TransRectangleClip(VMemType color, int x, int y,
    int w, int h)
{
    #define _x              x
    #define _y              y
    #define _w              w
    #define _h              h
    #define _hline(x, y,w)   VideoDraw50TransHLine(color, x, y,w)
    #define _vline(x, y,h)   VideoDraw50TransVLine(color, x, y,h)

    #include "_clip_rectangle"

    #undef _x
    #undef _y
    #undef _w
    #undef _h
    #undef _hline
    #undef _vline
}

/**
**	Draw 75% translucent rectangle clipped.
**
**	@param color	color
**	@param x	x coordinate on the screen
**	@param y	y coordinate on the screen
**	@param h	height of rectangle (0=don't draw).
**	@param w	width of rectangle (0=don't draw).
*/
global void VideoDraw75TransRectangleClip(VMemType color, int x, int y,
    int w, int h)
{
    #define _x              x
    #define _y              y
    #define _w              w
    #define _h              h
    #define _hline(x, y,w)   VideoDraw75TransHLine(color, x, y,w)
    #define _vline(x, y,h)   VideoDraw75TransVLine(color, x, y,h)

    #include "_clip_rectangle"

    #undef _x
    #undef _y
    #undef _w
    #undef _h
    #undef _hline
    #undef _vline
}

/**
**	Draw translucent rectangle clipped.
**
**	@param color	color
**	@param x	x coordinate on the screen
**	@param y	y coordinate on the screen
**	@param h	height of rectangle (0=don't draw).
**	@param w	width of rectangle (0=don't draw).
**      @param alpha    alpha value of pixel.
*/
global void VideoDrawTransRectangleClip(VMemType color, int x, int y,
    int w, int h, unsigned char alpha)
{
    #define _x              x
    #define _y              y
    #define _w              w
    #define _h              h
    #define _hline(x, y,w)   VideoDrawTransHLine(color, x, y,w,alpha)
    #define _vline(x, y,h)   VideoDrawTransVLine(color, x, y,h,alpha)

    #include "_clip_rectangle"

    #undef _x
    #undef _y
    #undef _w
    #undef _h
    #undef _hline
    #undef _vline
}

// ===========================================================================
//	Circle
// ===========================================================================

// FIXME: could write a general circle function?

/**
**	Draw circle.
**
**	@param color	color
**	@param x	Center x coordinate on the screen
**	@param y	Center y coordinate on the screen
**	@param r	radius of circle
*/
global void VideoDrawCircle(VMemType color, int x, int y, int r)
{
    int cx;
    int cy;
    int df;
    int d_e;
    int d_se;

    cx = 0;
    cy = r;
    df = 1 - r;
    d_e = 3;
    d_se = -2 * r + 5;

    // FIXME: could much improved :)
    do {
	if (cx == 0) {
	    VideoDrawPixel(color, x, y + cy);
	    VideoDrawPixel(color, x, y - cy);
	    VideoDrawPixel(color, x + cy, y);
	    VideoDrawPixel(color, x - cy, y);
	} else if (cx == cy) {
	    DebugCheck(cx == 0 || cy == 0);
	    VideoDrawPixel(color,x + cx, y + cy);
	    VideoDrawPixel(color,x - cx, y + cy);
	    VideoDrawPixel(color,x + cx, y - cy);
	    VideoDrawPixel(color,x - cx, y - cy);
	} else if (cx < cy) {
	    DebugCheck(cx == 0 || cy == 0);
	    VideoDrawPixel(color, x + cx, y + cy);
	    VideoDrawPixel(color, x + cx, y - cy);
	    VideoDrawPixel(color, x + cy, y + cx);
	    VideoDrawPixel(color, x + cy, y - cx);
	    VideoDrawPixel(color, x - cx, y + cy);
	    VideoDrawPixel(color, x - cx, y - cy);
	    VideoDrawPixel(color, x - cy, y + cx);
	    VideoDrawPixel(color, x - cy, y - cx);
	}
	if (df < 0) {
	    df += d_e;
	    d_se += 2;
	} else {
	    df += d_se;
	    d_se += 4;
	    cy--;
	}
	d_e += 2;
	cx++;
    } while (cx <= cy);
}

/**
**	Draw circle clipped.
**
**	@param color	color
**	@param x	Center x coordinate on the screen
**	@param y	Center y coordinate on the screen
**	@param r	radius of circle
*/
global void VideoDrawCircleClip(VMemType color, int x, int y, int r)
{
    int cx;
    int cy;
    int df;
    int d_e;
    int d_se;

    cx = 0;
    cy = r;
    df = 1 - r;
    d_e = 3;
    d_se = -2 * r + 5;

    // FIXME: could much improved :)
    do {
	if (cx == 0) {
	    VideoDrawPixelClip(color, x, y + cy);
	    VideoDrawPixelClip(color, x, y - cy);
	    VideoDrawPixelClip(color, x + cy, y);
	    VideoDrawPixelClip(color, x - cy, y);
	} else if (cx == cy) {
	    DebugCheck(cx == 0 || cy == 0);
	    VideoDrawPixelClip(color, x + cx, y + cy);
	    VideoDrawPixelClip(color, x - cx, y + cy);
	    VideoDrawPixelClip(color, x + cx, y - cy);
	    VideoDrawPixelClip(color, x - cx, y - cy);
	} else if (cx < cy) {
	    DebugCheck(cx == 0 || cy == 0);
	    VideoDrawPixelClip(color, x + cx, y + cy);
	    VideoDrawPixelClip(color, x + cx, y - cy);
	    VideoDrawPixelClip(color, x + cy, y + cx);
	    VideoDrawPixelClip(color, x + cy, y - cx);
	    VideoDrawPixelClip(color, x - cx, y + cy);
	    VideoDrawPixelClip(color, x - cx, y - cy);
	    VideoDrawPixelClip(color, x - cy, y + cx);
	    VideoDrawPixelClip(color, x - cy, y - cx);
	}
	if (df < 0) {
	    df += d_e;
	    d_se += 2;
	} else {
	    df += d_se;
	    d_se += 4;
	    cy--;
	}
	d_e += 2;
	cx++;
    } while (cx <= cy);
}

// ===========================================================================
//	Filled rectangle
// ===========================================================================

/**
**	Fill rectangle into 8bpp frame buffer.
**
**	@param color	color
**	@param x	x coordinate on the screen
**	@param y	y coordinate on the screen
**	@param h	height of rectangle (0=don't draw).
**	@param w	width of rectangle (0=don't draw).
*/
local void DrawFillRectangle8(VMemType color, int x, int y, int w, int h)
{
    VMemType8* p;
    VMemType8 f;
    int swidth;
    int i;

    f = color.D8;
    swidth = VideoWidth;
    p = VideoMemory8 + y * swidth + x;
    swidth -= w;
    if (w) {
	while (h--) {
	    i = w;
	    do {
		*p++ = f;
	    } while (--i);
	    p += swidth;
	}
    }
}

/**
**	Fill rectangle into 16bpp frame buffer.
**
**	@param color	color
**	@param x	x coordinate on the screen
**	@param y	y coordinate on the screen
**	@param h	height of rectangle (0=don't draw).
**	@param w	width of rectangle (0=don't draw).
*/
local void DrawFillRectangle16(VMemType color, int x, int y, int w, int h)
{
    VMemType16* p;
    VMemType16 f;
    int swidth;
    int i;

    f = color.D16;
    swidth = VideoWidth;
    p = VideoMemory16 + y * swidth + x;
    swidth -= w;
    if (w) {
	while (h--) {
	    i = w;
	    do {
		*p++ = f;
	    } while (--i);
	    p += swidth;
	}
    }
}

/**
**	Fill rectangle into 24bpp frame buffer.
**
**	@param color	color
**	@param x	x coordinate on the screen
**	@param y	y coordinate on the screen
**	@param h	height of rectangle (0=don't draw).
**	@param w	width of rectangle (0=don't draw).
*/
local void DrawFillRectangle24(VMemType color, int x, int y, int w, int h)
{
    VMemType24* p;
    VMemType24 f;
    int swidth;
    int i;

    f = color.D24;
    swidth = VideoWidth;
    p = VideoMemory24 + y * swidth + x;
    swidth -= w;
    if (w) {
	while (h--) {
	    i = w;
	    do {
		*p++ = f;
	    } while (--i);
	    p += swidth;
	}
    }
}

/**
**	Fill rectangle into 32bpp frame buffer.
**
**	@param color	color
**	@param x	x coordinate on the screen
**	@param y	y coordinate on the screen
**	@param h	height of rectangle (0=don't draw).
**	@param w	width of rectangle (0=don't draw).
*/
local void DrawFillRectangle32(VMemType color, int x, int y, int w, int h)
{
    VMemType32* p;
    VMemType32 f;
    int swidth;
    int i;

    f = color.D32;
    swidth = VideoWidth;
    p = VideoMemory32 + y * swidth + x;
    swidth -= w;
    if (w) {
	while (h--) {
	    i = w;
	    do {
		*p++ = f;
	    } while (--i);
	    p += swidth;
	}
    }
}

/**
**	Fill rectangle.
**
**	@param color	color
**	@param x	x coordinate on the screen
**	@param y	y coordinate on the screen
**	@param h	height of rectangle (0=don't draw).
**	@param w	width of rectangle (0=don't draw).
*/
#ifdef USE_OPENGL
local void DrawFillRectangleOpenGL(VMemType color, int x, int y,
    int w, int h)
{
    VMemType32 c;
    GLubyte r;
    GLubyte g;
    GLubyte b;

    c = color.D32;
    r = (c >> 16) & 0xff;
    g = (c >> 8) & 0xff;
    b = (c >> 0) & 0xff;
    glDisable(GL_TEXTURE_2D);
    glColor3ub(r, g, b);
    glBegin(GL_TRIANGLE_STRIP);
    glVertex2i(x, VideoHeight - y);
    glVertex2i(x + w, VideoHeight - y);
    glVertex2i(x, VideoHeight - (y + h));
    glVertex2i(x + w, VideoHeight - (y + h));
    glEnd();
    glEnable(GL_TEXTURE_2D);
}
#endif

/**
**	Fill rectangle 25% translucent clipped into 8bpp frame buffer.
**
**	@param color	color
**	@param x	x coordinate on the screen
**	@param y	y coordinate on the screen
**	@param h	height of rectangle (0=don't draw).
**	@param w	width of rectangle (0=don't draw).
*/
local void DrawFill25TransRectangle8(VMemType color, int x, int y,
    int w, int h)
{
    VMemType8* p;
    int i;
    int swidth;
    unsigned c;

    swidth = VideoWidth;
    p = VideoMemory8 + y * swidth + x;
    swidth -= w;
    c = color.D8 << 8;

    if (w) {
	while (h--) {
	    i = w;
	    do {
		*p = lookup25trans8[c | *p];
		++p;
	    } while (--i);
	    p += swidth;
	}
    }
}

/**
**	Fill rectangle 25% translucent clipped into 15bpp frame buffer.
**
**	@param color	color
**	@param x	x coordinate on the screen
**	@param y	y coordinate on the screen
**	@param h	height of rectangle (0=don't draw).
**	@param w	width of rectangle (0=don't draw).
*/
local void DrawFill25TransRectangle15(VMemType color, int x, int y,
    int w, int h)
{
    VMemType16* p;
    unsigned long sp;
    int i;
    int swidth;

    swidth = VideoWidth;
    p = VideoMemory16 + y * swidth + x;
    swidth -= w;
    sp = color.D16;
    // FIXME: pre multiply?
    sp = (((sp << 16) | sp) & 0x03E07C1F) * 3;

    if (w) {
	while (h--) {
	    i = w;
	    do {
		unsigned long dp;
		dp = *p;
		dp = ((dp << 16) | dp) & 0x03E07C1F;
		dp = ((dp + sp) >> 2) & 0x03E07C1F;
		*p++ = (dp >> 16) | dp;
	    } while (--i);
	    p += swidth;
	}
    }
}

/**
**	Fill rectangle 25% translucent clipped into 16bpp frame buffer.
**
**	@param color	color
**	@param x	x coordinate on the screen
**	@param y	y coordinate on the screen
**	@param h	height of rectangle (0=don't draw).
**	@param w	width of rectangle (0=don't draw).
*/
local void DrawFill25TransRectangle16(VMemType color, int x, int y,
    int w, int h)
{
    VMemType16* p;
    unsigned long sp;
    int i;
    int swidth;

    swidth = VideoWidth;
    p = VideoMemory16 + y * swidth + x;
    swidth -= w;
    sp = color.D16;
    // FIXME: pre multiply?
    sp = (((sp << 16) | sp) & 0x07E0F81F) * 3;

    if (w) {
	while (h--) {
	    i = w;
	    do {
		unsigned long dp;
		dp = *p;
		dp = ((dp << 16) | dp) & 0x07E0F81F;
		dp = ((dp + sp) >> 2) & 0x07E0F81F;
		*p++ = (dp >> 16) | dp;
	    } while (--i);
	    p += swidth;
	}
    }
}

/**
**	Fill rectangle 25% translucent clipped into 24bpp frame buffer.
**
**	@param color	color
**	@param x	x coordinate on the screen
**	@param y	y coordinate on the screen
**	@param h	height of rectangle (0=don't draw).
**	@param w	width of rectangle (0=don't draw).
*/
local void DrawFill25TransRectangle24(VMemType color, int x, int y,
    int w, int h)
{
// FIXME: does 24bpp holds R|G|B ?
    DrawFillRectangle24(color, x, y, w, h); // no trans functionaility :(
}

/**
**	Fill rectangle 25% translucent clipped into 32bpp frame buffer.
**
**	@param color	color
**	@param x	x coordinate on the screen
**	@param y	y coordinate on the screen
**	@param h	height of rectangle (0=don't draw).
**	@param w	width of rectangle (0=don't draw).
*/
local void DrawFill25TransRectangle32(VMemType color, int x, int y,
    int w, int h)
{
    VMemType32* p;
    unsigned long sp1;
    unsigned long sp2;
    int i;
    int swidth;

    swidth = VideoWidth;
    p = VideoMemory32 + y * swidth + x;
    swidth -= w;

    sp1 = color.D32;
    // FIXME: pre multiply?
    sp2 = ((sp1 & 0xFF00FF00) >> 8) * 3;
    sp1 = (sp1 & 0x00FF00FF) * 3;

    if (w) {
	while (h--) {
	    i = w;
	    do {
		unsigned long dp1;
		unsigned long dp2;

		dp1 = *p;
		dp2 = (dp1 & 0xFF00FF00) >> 8;
		dp1 &= 0x00FF00FF;

		dp1 = ((dp1 + sp1) >> 2) & 0x00FF00FF;
		dp2 = ((dp2 + sp2) >> 2) & 0x00FF00FF;
		*p++ = (dp1 | (dp2 << 8));
	    } while (--i);
	    p += swidth;
	}
    }
}

/**
**	Fill rectangle 25% translucent clipped.
**
**	@param color	color
**	@param x	x coordinate on the screen
**	@param y	y coordinate on the screen
**	@param h	height of rectangle (0=don't draw).
**	@param w	width of rectangle (0=don't draw).
*/
#ifdef USE_OPENGL
local void DrawFill25TransRectangleOpenGL(VMemType color, int x, int y,
    int w, int h)
{
    VMemType32 c;
    GLubyte r;
    GLubyte g;
    GLubyte b;

    c = color.D32;
    r = (c >> 16) & 0xff;
    g = (c >> 8) & 0xff;
    b = (c >> 0) & 0xff;
    glDisable(GL_TEXTURE_2D);
    glColor4ub(r, g, b, 192);
    glBegin(GL_TRIANGLE_STRIP);
    glVertex2i(x, VideoHeight - y);
    glVertex2i(x + w, VideoHeight - y);
    glVertex2i(x, VideoHeight - (y + h));
    glVertex2i(x + w, VideoHeight - (y + h));
    glEnd();
    glEnable(GL_TEXTURE_2D);
}
#endif

/**
**	Fill rectangle 50% translucent clipped into 8bpp frame buffer.
**
**	@param color	color
**	@param x	x coordinate on the screen
**	@param y	y coordinate on the screen
**	@param h	height of rectangle (0=don't draw).
**	@param w	width of rectangle (0=don't draw).
*/
local void DrawFill50TransRectangle8(VMemType color, int x, int y,
    int w, int h)
{
    VMemType8* p;
    int i;
    int swidth;
    unsigned c;

    swidth = VideoWidth;
    p = VideoMemory8 + y * swidth + x;
    swidth -= w;
    c = color.D8 << 8;

    if (w) {
	while (h--) {
	    i = w;
	    do {
		*p = lookup50trans8[c | *p];
		++p;
	    } while (--i);
	    p += swidth;
	}
    }
}

/**
**	Fill rectangle 50% translucent clipped into 15bpp frame buffer.
**
**	@param color	color
**	@param x	x coordinate on the screen
**	@param y	y coordinate on the screen
**	@param h	height of rectangle (0=don't draw).
**	@param w	width of rectangle (0=don't draw).
*/
local void DrawFill50TransRectangle15(VMemType color, int x, int y,
    int w, int h)
{
    VMemType16* p;
    unsigned long sp;
    int i;
    int swidth;

    swidth = VideoWidth;
    p = VideoMemory16 + y * swidth + x;
    swidth -= w;
    sp = color.D16;
    sp = ((sp << 16) | sp) & 0x03E07C1F;

    if (w) {
	while (h--) {
	    i = w;
	    do {
		unsigned long dp;
		dp = *p;
		dp = ((dp << 16) | dp) & 0x03E07C1F;
		dp = ((dp + sp) >> 1) & 0x03E07C1F;
		*p++ = (dp >> 16) | dp;
	    } while (--i);
	    p += swidth;
	}
    }
}

/**
**	Fill rectangle 50% translucent clipped into 16bpp frame buffer.
**
**	@param color	color
**	@param x	x coordinate on the screen
**	@param y	y coordinate on the screen
**	@param h	height of rectangle (0=don't draw).
**	@param w	width of rectangle (0=don't draw).
*/
local void DrawFill50TransRectangle16(VMemType color, int x, int y,
    int w, int h)
{
    VMemType16* p;
    unsigned long sp;
    int i;
    int swidth;

    swidth = VideoWidth;
    p = VideoMemory16 + y * swidth + x;
    swidth -= w;
    sp = color.D16;
    sp = ((sp << 16) | sp) & 0x07E0F81F;

    if (w) {
	while (h--) {
	    i = w;
	    do {
		unsigned long dp;
		dp = *p;
		dp = ((dp << 16) | dp) & 0x07E0F81F;
		dp = ((dp + sp) >> 1) & 0x07E0F81F;
		*p++ = (dp >> 16) | dp;
	    } while (--i);
	    p += swidth;
	}
    }
}

/**
**	Fill rectangle 50% translucent clipped into 24bpp frame buffer.
**
**	@param color	color
**	@param x	x coordinate on the screen
**	@param y	y coordinate on the screen
**	@param h	height of rectangle (0=don't draw).
**	@param w	width of rectangle (0=don't draw).
*/
local void DrawFill50TransRectangle24(VMemType color, int x, int y,
    int w, int h)
{
// FIXME: how does 24bpp represents RGB ?
    DrawFillRectangle24(color, x, y, w, h); // no trans functionaility :(
}

/**
**	Fill rectangle 50% translucent clipped into 32bpp frame buffer.
**
**	@param color	color
**	@param x	x coordinate on the screen
**	@param y	y coordinate on the screen
**	@param h	height of rectangle (0=don't draw).
**	@param w	width of rectangle (0=don't draw).
*/
local void DrawFill50TransRectangle32(VMemType color, int x, int y,
    int w, int h)
{
    VMemType32* p;
    unsigned long sp1;
    unsigned long sp2;
    int i;
    int swidth;

    swidth = VideoWidth;
    p = VideoMemory32 + y * swidth + x;
    swidth -= w;

    sp1 = color.D32;
    sp2 = (sp1 & 0xFF00FF00) >> 8;
    sp1 &= 0x00FF00FF;

    if (w) {
	while (h--) {
	    i = w;
	    do {
		unsigned long dp1;
		unsigned long dp2;

		dp1 = *p;
		dp2 = (dp1 & 0xFF00FF00) >> 8;
		dp1 &= 0x00FF00FF;

		dp1 = ((dp1 + sp1) >> 1) & 0x00FF00FF;
		dp2 = ((dp2 + sp2) >> 1) & 0x00FF00FF;
		*p++ = (dp1 | (dp2 << 8));
	    } while (--i);
	    p += swidth;
	}
    }
}

/**
**	Fill rectangle 50% translucent clipped.
**
**	@param color	color
**	@param x	x coordinate on the screen
**	@param y	y coordinate on the screen
**	@param h	height of rectangle (0=don't draw).
**	@param w	width of rectangle (0=don't draw).
*/
#ifdef USE_OPENGL
local void DrawFill50TransRectangleOpenGL(VMemType color, int x, int y,
    int w, int h)
{
    VMemType32 c;
    GLubyte r;
    GLubyte g;
    GLubyte b;

    c = color.D32;
    r = (c >> 16) & 0xff;
    g = (c >> 8) & 0xff;
    b = (c >> 0) & 0xff;
    glDisable(GL_TEXTURE_2D);
    glColor4ub(r, g, b, 128);
    glBegin(GL_TRIANGLE_STRIP);
    glVertex2i(x, VideoHeight - y);
    glVertex2i(x + w, VideoHeight - y);
    glVertex2i(x, VideoHeight - (y + h));
    glVertex2i(x + w, VideoHeight - (y + h));
    glEnd();
    glEnable(GL_TEXTURE_2D);
}
#endif

/**
**	Fill rectangle 75% translucent clipped into 8bpp frame buffer.
**
**	@param color	color
**	@param x	x coordinate on the screen
**	@param y	y coordinate on the screen
**	@param h	height of rectangle (0=don't draw).
**	@param w	width of rectangle (0=don't draw).
*/
local void DrawFill75TransRectangle8(VMemType color, int x, int y,
    int w, int h)
{
    VMemType8* p;
    int i;
    int swidth;
    unsigned c;

    swidth = VideoWidth;
    p = VideoMemory8 + y * swidth + x;
    swidth -= w;
    c = color.D8;

    if (w) {
	while (h--) {
	    i = w;
	    do {
		*p = lookup25trans8[(*p << 8) | c];
		++p;
	    } while (--i);
	    p += swidth;
	}
    }
}

/**
**	Fill rectangle 75% translucent clipped into 15bpp frame buffer.
**
**	@param color	color
**	@param x	x coordinate on the screen
**	@param y	y coordinate on the screen
**	@param h	height of rectangle (0=don't draw).
**	@param w	width of rectangle (0=don't draw).
*/
local void DrawFill75TransRectangle15(VMemType color, int x, int y,
    int w, int h)
{
    VMemType16* p;
    unsigned long sp;
    int i;
    int swidth;

    swidth = VideoWidth;
    p = VideoMemory16 + y * swidth + x;
    swidth -= w;
    sp = color.D16;
    sp = ((sp << 16) | sp) & 0x03E07C1F;

    if (w) {
	while (h--) {
	    i = w;
	    do {
		unsigned long dp;
		dp = *p;
		dp = ((dp << 16) | dp) & 0x03E07C1F;
		dp = (((dp << 1) + dp + sp) >> 2) & 0x03E07C1F;
		*p++ = (dp >> 16) | dp;
	    } while (--i);
	    p += swidth;
	}
    }
}

/**
**	Fill rectangle 75% translucent clipped into 16bpp frame buffer.
**
**	@param color	color
**	@param x	x coordinate on the screen
**	@param y	y coordinate on the screen
**	@param h	height of rectangle (0=don't draw).
**	@param w	width of rectangle (0=don't draw).
*/
local void DrawFill75TransRectangle16(VMemType color, int x, int y,
    int w, int h)
{
    VMemType16* p;
    unsigned long sp;
    int i;
    int swidth;

    swidth = VideoWidth;
    p = VideoMemory16 + y * swidth + x;
    swidth -= w;
    sp = color.D16;
    sp = ((sp << 16) | sp) & 0x07E0F81F;

    if (w) {
	while (h--) {
	    i = w;
	    do {
		unsigned long dp;
		dp = *p;
		dp = ((dp << 16) | dp) & 0x07E0F81F;
		dp = (((dp << 1) + dp + sp) >> 2) & 0x07E0F81F;
		*p++ = (dp >> 16) | dp;
	    } while (--i);
	    p += swidth;
	}
    }
}

/**
**	Fill rectangle 75% translucent clipped into 24bpp frame buffer.
**
**	@param color	color
**	@param x	x coordinate on the screen
**	@param y	y coordinate on the screen
**	@param h	height of rectangle (0=don't draw).
**	@param w	width of rectangle (0=don't draw).
*/
local void DrawFill75TransRectangle24(VMemType color, int x, int y,
    int w, int h)
{
// FIXME: does 24bpp holds R|G|B ?
    DrawFillRectangle24(color, x, y, w, h); // no trans functionaility :(
}

/**
**	Fill rectangle 75% translucent clipped into 32bpp frame buffer.
**
**	@param color	color
**	@param x	x coordinate on the screen
**	@param y	y coordinate on the screen
**	@param h	height of rectangle (0=don't draw).
**	@param w	width of rectangle (0=don't draw).
*/
local void DrawFill75TransRectangle32(VMemType color, int x, int y,
    int w, int h)
{
    VMemType32* p;
    unsigned long sp1;
    unsigned long sp2;
    int i;
    int swidth;

    swidth = VideoWidth;
    p = VideoMemory32 + y * swidth + x;
    swidth -= w;

    sp1 = color.D32;
    sp2 = (sp1 & 0xFF00FF00) >> 8;
    sp1 &= 0x00FF00FF;

    if (w) {
	while (h--) {
	    i = w;
	    do {
		unsigned long dp1;
		unsigned long dp2;

		dp1 = *p;
		dp2 = (dp1 & 0xFF00FF00) >> 8;
		dp1 &= 0x00FF00FF;

		dp1 = (((dp1 << 1) + dp1 + sp1) >> 2) & 0x00FF00FF;
		dp2 = (((dp2 << 1) + dp2 + sp2) >> 2) & 0x00FF00FF;
		*p++ = (dp1 | (dp2 << 8));
	    } while (--i);
	    p += swidth;
	}
    }
}

/**
**	Fill rectangle 75% translucent clipped.
**
**	@param color	color
**	@param x	x coordinate on the screen
**	@param y	y coordinate on the screen
**	@param h	height of rectangle (0=don't draw).
**	@param w	width of rectangle (0=don't draw).
*/
#ifdef USE_OPENGL
local void DrawFill75TransRectangleOpenGL(VMemType color, int x, int y,
    int w, int h)
{
    VMemType32 c;
    GLubyte r;
    GLubyte g;
    Glubyte b;

    c = color.D32;
    r = (c >> 16) & 0xff;
    g = (c >> 8) & 0xff;
    b = (c >> 0) & 0xff;
    glDisable(GL_TEXTURE_2D);
    glColor4ub(r, g, b, 64);
    glBegin(GL_TRIANGLE_STRIP);
    glVertex2i(x, VideoHeight - y);
    glVertex2i(x + w, VideoHeight - y);
    glVertex2i(x, VideoHeight - (y + h));
    glVertex2i(x + w, VideoHeight - (y + h));
    glEnd();
    glEnable(GL_TEXTURE_2D);
}
#endif

/**
**	Draw translucent rectangle into 8bpp frame buffer.
**
**	@param color	color
**	@param x	x coordinate on the screen
**	@param y	y coordinate on the screen
**	@param h	height of rectangle (0=don't draw).
**	@param w	width of rectangle (0=don't draw).
**      @param alpha    alpha value of pixel.
*/
local void DrawFillTransRectangle8(VMemType color, int x, int y,
    int w, int h, unsigned char alpha)
{
    VMemType8* p;
    int i;
    int swidth;
    unsigned c;

    swidth = VideoWidth;
    p = VideoMemory8 + y * swidth + x;
    swidth -= w;
    c = color.D8;

    if (w) {
	switch (((unsigned int)alpha * 4) / 255) {
	    case 0:
		while (h--) {
		    i = w;
		    do {
			*p = c;
			++p;
		    } while (--i);
		    p += swidth;
		}
		break;
	    case 1:
		c <<= 8;
		while (h--) {
		    i = w;
		    do {
			*p = lookup25trans8[*p | c];
			++p;
		    } while (--i);
		    p += swidth;
		}
		break;
	    case 2:
		c <<= 8;
		while (h--) {
		    i = w;
		    do {
			*p = lookup50trans8[*p | c];
			++p;
		    } while (--i);
		    p += swidth;
		}
		break;
	    case 3:
		while (h--) {
		    i = w;
		    do {
			*p = lookup25trans8[(*p << 8) | c];
			++p;
		    } while (--i);
		    p += swidth;
		}
		break;
	    default:
		break;
	}
    }
}

/**
**	Draw rectangle into 8bpp frame buffer (ignoring alpha).
**
**	@param color	color
**	@param x	x coordinate on the screen
**	@param y	y coordinate on the screen
**	@param h	height of rectangle (0=don't draw).
**	@param w	width of rectangle (0=don't draw).
**      @param alpha    alpha value of pixel.
*/
local void DrawFillNoTransRectangle8(VMemType color, int x, int y,
    int w, int h, unsigned char alpha __attribute__((unused)))
{
    DrawFillRectangle8(color, x, y, w, h);
}

/**
**	Draw translucent rectangle into 15bpp frame buffer.
**
**	@param color	color
**	@param x	x coordinate on the screen
**	@param y	y coordinate on the screen
**	@param h	height of rectangle (0=don't draw).
**	@param w	width of rectangle (0=don't draw).
**      @param alpha    alpha value of pixel.
*/
local void DrawFillTransRectangle15(VMemType color, int x, int y,
    int w, int h, unsigned char alpha)
{
    VMemType16* p;
    unsigned long sp;
    int i;
    int swidth;

    swidth = VideoWidth;
    p = VideoMemory16 + y * swidth + x;
    swidth -= w;
    sp = color.D16;
    sp = ((sp << 16) | sp) & 0x07E0F81F;
    alpha >>= 3;                          //FIXME: only 5bits

    if (w) {
	while (h--) {
	    i = w;
	    do {
		unsigned long dp;
		dp = *p;
		dp = ((dp << 16) | dp) & 0x07E0F81F;
		dp = ((((dp - sp) * alpha) >> 5) + sp) & 0x03E07C1F; //FIXME: alpha==256 unreached
		*p++ = (dp >> 16) | dp;
	    } while (--i);
	    p += swidth;
	}
    }
}

/**
**	Draw translucent rectangle into 16bpp frame buffer.
**
**	@param color	color
**	@param x	x coordinate on the screen
**	@param y	y coordinate on the screen
**	@param h	height of rectangle (0=don't draw).
**	@param w	width of rectangle (0=don't draw).
**      @param alpha    alpha value of pixel.
*/
local void DrawFillTransRectangle16(VMemType color, int x, int y,
    int w, int h, unsigned char alpha)
{
    VMemType16* p;
    unsigned long sp;
    int i;
    int swidth;

    swidth = VideoWidth;
    p = VideoMemory16 + y * swidth + x;
    swidth -= w;
    sp = color.D16;
    sp = ((sp << 16) | sp) & 0x07E0F81F;
    alpha >>= 3;                          //FIXME: only 5bits

    if (w) {
	while (h--) {
	    i = w;
	    do {
		unsigned long dp;
		dp = *p;
		dp = ((dp << 16) | dp) & 0x07E0F81F;
		dp = ((((dp - sp) * alpha) >> 5) + sp) & 0x07E0F81F; //FIXME: alpha==256 unreached
		*p++ = (dp >> 16) | dp;
	    } while (--i);
	    p += swidth;
	}
    }
}

/**
**	Draw translucent rectangle into 24bpp frame buffer.
**
**	@param color	color
**	@param x	x coordinate on the screen
**	@param y	y coordinate on the screen
**	@param h	height of rectangle (0=don't draw).
**	@param w	width of rectangle (0=don't draw).
**      @param alpha    alpha value of pixel.
*/
local void DrawFillTransRectangle24(VMemType color, int x, int y,
    int w, int h, unsigned char alpha __attribute__((unused)))
{
//FIXME: does 24bpp represents R|G|B?
    DrawFillRectangle24(color, x, y, w, h); // no trans functionaility :(
}

/**
**	Draw translucent rectangle into 32bpp frame buffer.
**
**	@param color	color
**	@param x	x coordinate on the screen
**	@param y	y coordinate on the screen
**	@param h	height of rectangle (0=don't draw).
**	@param w	width of rectangle (0=don't draw).
**      @param alpha    alpha value of pixel.
*/
local void DrawFillTransRectangle32(VMemType color, int x, int y,
    int w, int h, unsigned char alpha)
{
    VMemType32* p;
    unsigned long sp1;
    unsigned long sp2;
    int i;
    int swidth;

    swidth = VideoWidth;
    p = VideoMemory32 + y * swidth + x;
    swidth -= w;

    sp1 = color.D32;
    sp2 = (sp1 & 0xFF00FF00) >> 8;
    sp1 &= 0x00FF00FF;

    if (w) {
	while (h--) {
	    i = w;
	    do {
		unsigned long dp1;
		unsigned long dp2;

		dp1 = *p;
		dp2 = (dp1 & 0xFF00FF00) >> 8;
		dp1 &= 0x00FF00FF;

		//FIXME: alpha==256 unreached
		dp1 = ((((dp1-sp1) * alpha) >> 8) + sp1) & 0x00FF00FF;
		dp2 = ((((dp2-sp2) * alpha) >> 8) + sp2) & 0x00FF00FF;
		*p++ = (dp1 | (dp2 << 8));
	    } while (--i);
	    p += swidth;
	}
    }
}

/**
**	Draw translucent rectangle.
**
**	@param color	color
**	@param x	x coordinate on the screen
**	@param y	y coordinate on the screen
**	@param h	height of rectangle (0=don't draw).
**	@param w	width of rectangle (0=don't draw).
**      @param alpha    alpha value of pixel.
*/
#ifdef USE_OPENGL
local void DrawFillTransRectangleOpenGL(VMemType color, int x, int y,
    int w, int h, unsigned char alpha)
{
    VMemType32 c;
    GLubyte r;
    GLubyte g;
    GLubyte b;

    c = color.D32;
    r = (c >> 16) & 0xff;
    g = (c >> 8) & 0xff;
    b = (c >> 0) & 0xff;
    glDisable(GL_TEXTURE_2D);
    glColor4ub(r, g, b, 255 - alpha);
    glBegin(GL_TRIANGLE_STRIP);
    glVertex2i(x, VideoHeight - y);
    glVertex2i(x + w, VideoHeight - y);
    glVertex2i(x, VideoHeight - (y + h));
    glVertex2i(x + w, VideoHeight - (y + h));
    glEnd();
    glEnable(GL_TEXTURE_2D);
}
#endif

/**
**	Fill rectangle clipped.
**
**	@param color	color
**	@param x	x coordinate on the screen
**	@param y	y coordinate on the screen
**	@param h	height of rectangle (0=don't draw).
**	@param w	width of rectangle (0=don't draw).
*/
global void VideoFillRectangleClip(VMemType color, int x, int y,
    int w, int h)
{
    CLIP_RECTANGLE(x, y, w, h);
    VideoFillRectangle(color, x, y, w, h);
}

/**
**	Fill rectangle 25% translucent clipped.
**
**	@param color	color
**	@param x	x coordinate on the screen
**	@param y	y coordinate on the screen
**	@param h	height of rectangle (0=don't draw).
**	@param w	width of rectangle (0=don't draw).
*/
global void VideoFill25TransRectangleClip(VMemType color, int x, int y,
    int w, int h)
{
    CLIP_RECTANGLE(x, y, w, h);
    VideoFill25TransRectangle(color, x, y, w, h);
}

/**
**	Fill rectangle 50% translucent clipped.
**
**	@param color	color
**	@param x	x coordinate on the screen
**	@param y	y coordinate on the screen
**	@param h	height of rectangle (0=don't draw).
**	@param w	width of rectangle (0=don't draw).
*/
global void VideoFill50TransRectangleClip(VMemType color, int x, int y,
    int w, int h)
{
    CLIP_RECTANGLE(x, y, w, h);
    VideoFill50TransRectangle(color, x, y, w, h);
}

/**
**	Fill rectangle 75% translucent clipped.
**
**	@param color	color
**	@param x	x coordinate on the screen
**	@param y	y coordinate on the screen
**	@param h	height of rectangle (0=don't draw).
**	@param w	width of rectangle (0=don't draw).
*/
global void VideoFill75TransRectangleClip(VMemType color, int x, int y,
    int w, int h)
{
    CLIP_RECTANGLE(x, y, w, h);
    VideoFill75TransRectangle(color, x, y, w, h);
}

/**
**	Fill rectangle translucent clipped.
**
**	@param color	color
**	@param x	x coordinate on the screen
**	@param y	y coordinate on the screen
**	@param h	height of rectangle (0=don't draw).
**	@param w	width of rectangle (0=don't draw).
**	@param alpha	alpha value of pixels.
*/
global void VideoFillTransRectangleClip(VMemType color, int x, int y,
    int w, int h, unsigned char alpha)
{
    CLIP_RECTANGLE(x, y, w, h);
    VideoFillTransRectangle(color, x, y, w, h,alpha);
}

// ===========================================================================
//	Filled circle
// ===========================================================================

/**
**	Fill circle clipped.
**
**	@param color	color
**	@param x	Center x coordinate on the screen
**	@param y	Center y coordinate on the screen
**	@param r	radius of circle
*/
global void VideoFillCircleClip(VMemType color, int x, int y, int r)
{
    int cx;
    int cy;
    int df;
    int d_e;
    int d_se;

    cx = 0;
    cy = r;
    df = 1 - r;
    d_e = 3;
    d_se = -2 * r + 5;

    // FIXME: could much improved :)
    do {
	VideoDrawHLineClip(color, x - cy, y - cx, 1 + cy * 2);
	if (cx) {
	    VideoDrawHLineClip(color, x - cy, y + cx, 1 + cy * 2);
	}
	if (df < 0) {
	    df += d_e;
	    d_se += 2;
	} else {
	    if (cx != cy) {
		VideoDrawHLineClip(color, x - cx, y - cy, 1 + cx * 2);
		VideoDrawHLineClip(color, x - cx, y + cy, 1 + cx * 2);
	    }
	    df += d_se;
	    d_se += 4;
	    cy--;
	}
	d_e += 2;
	cx++;

    } while (cx <= cy);
}

/**
**	Fill circle 25% translucent clipped.
**
**	@param color	color
**	@param x	Center x coordinate on the screen
**	@param y	Center y coordinate on the screen
**	@param r	radius of circle
*/
global void VideoFill25TransCircleClip(VMemType color, int x, int y, int r)
{
    int cx;
    int cy;
    int df;
    int d_e;
    int d_se;

    cx = 0;
    cy = r;
    df = 1 - r;
    d_e = 3;
    d_se = -2 * r + 5;

    // FIXME: could much improved :)
    do {
	VideoDraw25TransHLineClip(color, x - cy, y - cx, 1 + cy * 2);
	if (cx) {
	    VideoDraw25TransHLineClip(color, x - cy, y + cx, 1 + cy * 2);
	}
	if (df < 0) {
	    df += d_e;
	    d_se += 2;
	} else {
	    if (cx!=cy) {
		VideoDraw25TransHLineClip(color, x - cx, y - cy, 1 + cx * 2);
		VideoDraw25TransHLineClip(color, x - cx, y + cy, 1 + cx * 2);
	    }
	    df += d_se;
	    d_se += 4;
	    cy--;
	}
	d_e += 2;
	cx++;

    } while (cx <= cy);
}

/**
**	Fill circle 50% translucent clipped.
**
**	@param color	color
**	@param x	Center x coordinate on the screen
**	@param y	Center y coordinate on the screen
**	@param r	radius of circle
*/
global void VideoFill50TransCircleClip(VMemType color, int x, int y, int r)
{
    int cx;
    int cy;
    int df;
    int d_e;
    int d_se;

    cx = 0;
    cy = r;
    df = 1 - r;
    d_e = 3;
    d_se = -2 * r + 5;

    // FIXME: could much improved :)
    do {
	VideoDraw50TransHLineClip(color, x - cy, y - cx, 1 + cy * 2);
	if (cx) {
	    VideoDraw50TransHLineClip(color, x - cy, y + cx, 1 + cy * 2);
	}
	if (df < 0) {
	    df += d_e;
	    d_se += 2;
	} else {
	    if (cx != cy) {
		VideoDraw50TransHLineClip(color, x - cx, y - cy, 1 + cx * 2);
		VideoDraw50TransHLineClip(color, x - cx, y + cy, 1 + cx * 2);
	    }
	    df += d_se;
	    d_se += 4;
	    cy--;
	}
	d_e += 2;
	cx++;

    } while (cx <= cy);
}

/**
**	Fill circle 75% translucent clipped.
**
**	@param color	color
**	@param x	Center x coordinate on the screen
**	@param y	Center y coordinate on the screen
**	@param r	radius of circle
*/
global void VideoFill75TransCircleClip(VMemType color, int x, int y, int r)
{
    int cx;
    int cy;
    int df;
    int d_e;
    int d_se;

    cx = 0;
    cy = r;
    df = 1 - r;
    d_e = 3;
    d_se = -2 * r + 5;

    // FIXME: could much improved :)
    do {
	VideoDraw75TransHLineClip(color, x - cy, y - cx, 1 + cy * 2);
	if (cx) {
	    VideoDraw75TransHLineClip(color, x - cy, y + cx, 1 + cy * 2);
	}
	if (df < 0) {
	    df += d_e;
	    d_se += 2;
	} else {
	    if (cx != cy) {
		VideoDraw75TransHLineClip(color, x - cx, y - cy, 1 + cx * 2);
		VideoDraw75TransHLineClip(color, x - cx, y + cy, 1 + cx * 2);
	    }
	    df += d_se;
	    d_se += 4;
	    cy--;
	}
	d_e += 2;
	cx++;

    } while (cx <= cy);
}

/*----------------------------------------------------------------------------
--	Global functions
----------------------------------------------------------------------------*/

/**
**	Init linedraw
*/
global void InitLineDraw(void)
{
#ifdef USE_OPENGL
    VideoDrawPixel            = DrawPixelOpenGL;
    VideoDraw25TransPixel     = Draw25TransPixelOpenGL;
    VideoDraw50TransPixel     = Draw50TransPixelOpenGL;
    VideoDraw75TransPixel     = Draw75TransPixelOpenGL;
    VideoDrawTransPixel       = DrawTransPixelOpenGL;
    VideoDrawPixelClip        = DrawPixelClipOpenGL;
    VideoDrawHLine            = DrawHLineOpenGL;
    VideoDraw25TransHLine     = Draw25TransHLineOpenGL;
    VideoDraw50TransHLine     = Draw50TransHLineOpenGL;
    VideoDraw75TransHLine     = Draw75TransHLineOpenGL;
    VideoDrawTransHLine       = DrawTransHLineOpenGL;
    VideoDrawVLine            = DrawVLineOpenGL;
    VideoDraw25TransVLine     = Draw25TransVLineOpenGL;
    VideoDraw50TransVLine     = Draw50TransVLineOpenGL;
    VideoDraw75TransVLine     = Draw75TransVLineOpenGL;
    VideoDrawTransVLine       = DrawTransVLineOpenGL;
    VideoDrawLine             = DrawLineOpenGL;
    VideoDrawRectangle        = DrawRectangleOpenGL;
    VideoDraw25TransRectangle = Draw25TransRectangleOpenGL;
    VideoDraw50TransRectangle = Draw50TransRectangleOpenGL;
    VideoDraw75TransRectangle = Draw75TransRectangleOpenGL;
    VideoDrawTransRectangle   = DrawTransRectangleOpenGL;
    VideoFillRectangle        = DrawFillRectangleOpenGL;
    VideoFill25TransRectangle = DrawFill25TransRectangleOpenGL;
    VideoFill50TransRectangle = DrawFill50TransRectangleOpenGL;
    VideoFill75TransRectangle = DrawFill75TransRectangleOpenGL;
    VideoFillTransRectangle   = DrawFillTransRectangleOpenGL;
#else
    switch (VideoBpp) {
	case 8:
	    if (lookup25trans8 && lookup50trans8) {
		VideoDrawPixel            = DrawPixel8;
		VideoDraw25TransPixel     = Draw25TransPixel8;
		VideoDraw50TransPixel     = Draw50TransPixel8;
		VideoDraw75TransPixel     = Draw75TransPixel8;
		VideoDrawTransPixel       = DrawTransPixel8;
		VideoDrawPixelClip        = DrawPixelClip8;
		VideoDrawHLine            = DrawHLine8;
		VideoDraw25TransHLine     = Draw25TransHLine8;
		VideoDraw50TransHLine     = Draw50TransHLine8;
		VideoDraw75TransHLine     = Draw75TransHLine8;
		VideoDrawTransHLine       = DrawTransHLine8;
		VideoDrawVLine            = DrawVLine8;
		VideoDraw25TransVLine     = Draw25TransVLine8;
		VideoDraw50TransVLine     = Draw50TransVLine8;
		VideoDraw75TransVLine     = Draw75TransVLine8;
		VideoDrawTransVLine       = DrawTransVLine8;
		VideoDrawLine             = DrawLine8;
		VideoDrawRectangle        = DrawRectangle8;
		VideoDraw25TransRectangle = Draw25TransRectangle8;
		VideoDraw50TransRectangle = Draw50TransRectangle8;
		VideoDraw75TransRectangle = Draw75TransRectangle8;
		VideoDrawTransRectangle   = DrawTransRectangle8;
		VideoFillRectangle        = DrawFillRectangle8;
		VideoFill25TransRectangle = DrawFill25TransRectangle8;
		VideoFill50TransRectangle = DrawFill50TransRectangle8;
		VideoFill75TransRectangle = DrawFill75TransRectangle8;
		VideoFillTransRectangle   = DrawFillTransRectangle8;
	    } else {
		printf("(transparency support disabled)\n");
		VideoDrawPixel            =
		VideoDraw25TransPixel     =
		VideoDraw50TransPixel     =
		VideoDraw75TransPixel     = DrawPixel8;
		VideoDrawTransPixel       = DrawNoTransPixel8;
		VideoDrawPixelClip        = DrawPixelClip8;
		VideoDrawHLine            =
		VideoDraw25TransHLine     =
		VideoDraw50TransHLine     =
		VideoDraw75TransHLine     = DrawHLine8;
		VideoDrawTransHLine       = DrawNoTransHLine8;
		VideoDrawVLine            =
		VideoDraw25TransVLine     =
		VideoDraw50TransVLine     =
		VideoDraw75TransVLine     = DrawVLine8;
		VideoDrawTransVLine       = DrawNoTransVLine8;
		VideoDrawLine             = DrawLine8;
		VideoDrawRectangle        =
		VideoDraw25TransRectangle =
		VideoDraw50TransRectangle =
		VideoDraw75TransRectangle = DrawRectangle8;
		VideoDrawTransRectangle   = DrawNoTransRectangle8;
		VideoFillRectangle        =
		VideoFill25TransRectangle =
		VideoFill50TransRectangle =
		VideoFill75TransRectangle = DrawFillRectangle8;
		VideoFillTransRectangle   = DrawFillNoTransRectangle8;
	    }
	    break;

	case 15:
	    VideoDrawPixel            = DrawPixel16;
	    VideoDraw25TransPixel     = Draw25TransPixel15;
	    VideoDraw50TransPixel     = Draw50TransPixel15;
	    VideoDraw75TransPixel     = Draw75TransPixel15;
	    VideoDrawTransPixel       = DrawTransPixel15;
	    VideoDrawPixelClip        = DrawPixelClip16;
	    VideoDrawHLine            = DrawHLine16;
	    VideoDraw25TransHLine     = Draw25TransHLine15;
	    VideoDraw50TransHLine     = Draw50TransHLine15;
	    VideoDraw75TransHLine     = Draw75TransHLine15;
	    VideoDrawTransHLine       = DrawTransHLine15;
	    VideoDrawVLine            = DrawVLine16;
	    VideoDraw25TransVLine     = Draw25TransVLine15;
	    VideoDraw50TransVLine     = Draw50TransVLine15;
	    VideoDraw75TransVLine     = Draw75TransVLine15;
	    VideoDrawTransVLine       = DrawTransVLine15;
	    VideoDrawLine             = DrawLine16;
	    VideoDrawRectangle        = DrawRectangle16;
	    VideoDraw25TransRectangle = Draw25TransRectangle15;
	    VideoDraw50TransRectangle = Draw50TransRectangle15;
	    VideoDraw75TransRectangle = Draw75TransRectangle15;
	    VideoDrawTransRectangle   = DrawTransRectangle15;
	    VideoFillRectangle        = DrawFillRectangle16;
	    VideoFill25TransRectangle = DrawFill25TransRectangle15;
	    VideoFill50TransRectangle = DrawFill50TransRectangle15;
	    VideoFill75TransRectangle = DrawFill75TransRectangle15;
	    VideoFillTransRectangle   = DrawFillTransRectangle15;
	    break;

	case 16:
	    VideoDrawPixel            = DrawPixel16;
	    VideoDraw25TransPixel     = Draw25TransPixel16;
	    VideoDraw50TransPixel     = Draw50TransPixel16;
	    VideoDraw75TransPixel     = Draw75TransPixel16;
	    VideoDrawTransPixel       = DrawTransPixel16;
	    VideoDrawPixelClip        = DrawPixelClip16;
	    VideoDrawHLine            = DrawHLine16;
	    VideoDraw25TransHLine     = Draw25TransHLine16;
	    VideoDraw50TransHLine     = Draw50TransHLine16;
	    VideoDraw75TransHLine     = Draw75TransHLine16;
	    VideoDrawTransHLine       = DrawTransHLine16;
	    VideoDrawVLine            = DrawVLine16;
	    VideoDraw25TransVLine     = Draw25TransVLine16;
	    VideoDraw50TransVLine     = Draw50TransVLine16;
	    VideoDraw75TransVLine     = Draw75TransVLine16;
	    VideoDrawTransVLine       = DrawTransVLine16;
	    VideoDrawLine             = DrawLine16;
	    VideoDrawRectangle        = DrawRectangle16;
	    VideoDraw25TransRectangle = Draw25TransRectangle16;
	    VideoDraw50TransRectangle = Draw50TransRectangle16;
	    VideoDraw75TransRectangle = Draw75TransRectangle16;
	    VideoDrawTransRectangle   = DrawTransRectangle16;
	    VideoFillRectangle        = DrawFillRectangle16;
	    VideoFill25TransRectangle = DrawFill25TransRectangle16;
	    VideoFill50TransRectangle = DrawFill50TransRectangle16;
	    VideoFill75TransRectangle = DrawFill75TransRectangle16;
	    VideoFillTransRectangle   = DrawFillTransRectangle16;
	    break;

	case 24:
	    VideoDrawPixel            = DrawPixel24;
	    VideoDraw25TransPixel     = Draw25TransPixel24;
	    VideoDraw50TransPixel     = Draw50TransPixel24;
	    VideoDraw75TransPixel     = Draw75TransPixel24;
	    VideoDrawTransPixel       = DrawTransPixel24;
	    VideoDrawPixelClip        = DrawPixelClip24;
	    VideoDrawHLine            = DrawHLine24;
	    VideoDraw25TransHLine     = Draw25TransHLine24;
	    VideoDraw50TransHLine     = Draw50TransHLine24;
	    VideoDraw75TransHLine     = Draw75TransHLine24;
	    VideoDrawTransHLine       = DrawTransHLine24;
	    VideoDrawVLine            = DrawVLine24;
	    VideoDraw25TransVLine     = Draw25TransVLine24;
	    VideoDraw50TransVLine     = Draw50TransVLine24;
	    VideoDraw75TransVLine     = Draw75TransVLine24;
	    VideoDrawTransVLine       = DrawTransVLine24;
	    VideoDrawLine             = DrawLine24;
	    VideoDrawRectangle        = DrawRectangle24;
	    VideoDraw25TransRectangle = Draw25TransRectangle24;
	    VideoDraw50TransRectangle = Draw50TransRectangle24;
	    VideoDraw75TransRectangle = Draw75TransRectangle24;
	    VideoDrawTransRectangle   = DrawTransRectangle24;
	    VideoFillRectangle        = DrawFillRectangle24;
	    VideoFill25TransRectangle = DrawFill25TransRectangle24;
	    VideoFill50TransRectangle = DrawFill50TransRectangle24;
	    VideoFill75TransRectangle = DrawFill75TransRectangle24;
	    VideoFillTransRectangle   = DrawFillTransRectangle24;
	    break;

	case 32:
	    VideoDrawPixel            = DrawPixel32;
	    VideoDraw25TransPixel     = Draw25TransPixel32;
	    VideoDraw50TransPixel     = Draw50TransPixel32;
	    VideoDraw75TransPixel     = Draw75TransPixel32;
	    VideoDrawTransPixel       = DrawTransPixel32;
	    VideoDrawPixelClip        = DrawPixelClip32;
	    VideoDrawHLine            = DrawHLine32;
	    VideoDraw25TransHLine     = Draw25TransHLine32;
	    VideoDraw50TransHLine     = Draw50TransHLine32;
	    VideoDraw75TransHLine     = Draw75TransHLine32;
	    VideoDrawTransHLine       = DrawTransHLine32;
	    VideoDrawVLine            = DrawVLine32;
	    VideoDraw25TransVLine     = Draw25TransVLine32;
	    VideoDraw50TransVLine     = Draw50TransVLine32;
	    VideoDraw75TransVLine     = Draw75TransVLine32;
	    VideoDrawTransVLine       = DrawTransVLine32;
	    VideoDrawLine             = DrawLine32;
	    VideoDrawRectangle        = DrawRectangle32;
	    VideoDraw25TransRectangle = Draw25TransRectangle32;
	    VideoDraw50TransRectangle = Draw50TransRectangle32;
	    VideoDraw75TransRectangle = Draw75TransRectangle32;
	    VideoDrawTransRectangle   = DrawTransRectangle32;
	    VideoFillRectangle        = DrawFillRectangle32;
	    VideoFill25TransRectangle = DrawFill25TransRectangle32;
	    VideoFill50TransRectangle = DrawFill50TransRectangle32;
	    VideoFill75TransRectangle = DrawFill75TransRectangle32;
	    VideoFillTransRectangle   = DrawFillTransRectangle32;
	    break;

	default:
	    DebugLevel0Fn("unsupported %d bpp\n" _C_ VideoBpp);
	    abort();
    }
#endif
}

#ifdef DEBUG	// {

// ===========================================================================
//
//      Below are functions which can test above linedraw functionaility
//
//      FOR DEBUG PURPOSE ONLY, BUT DON'T REMOVE PLEASE !!!
//
// ===========================================================================
/**
**      Will try all kinds of possible linedraw routines on current screen
**      contents (it does not clear the screen to show transparency better)
**      FIXME: all clipping assumes setgm. fault when goin outside screen
**             coordinate, it would be better to set a smaller clip rectangle
**             in the center of the screen to detect visible errors outside
**             the clip rectangle..
*/
global void DebugTestDisplayVarious(void)
{
    int x;
    int y;
    int i;
    int j;

    x = y = i = j = 0;
    //Should not show anything (segmentation fault when not properly clipped)
    VideoDrawPixelClip(ColorRed, -1, 0);
    VideoDrawPixelClip(ColorRed, 0, -1);
    VideoDrawPixelClip(ColorRed, VideoWidth, 0);
    VideoDrawPixelClip(ColorRed, 0, VideoHeight);
    VideoDraw25TransPixelClip(ColorRed, -1, 0);
    VideoDraw25TransPixelClip(ColorRed, 0, -1);
    VideoDraw25TransPixelClip(ColorRed, VideoWidth, 0);
    VideoDraw25TransPixelClip(ColorRed, 0, VideoHeight);
    VideoDraw50TransPixelClip(ColorRed, -1, 0);
    VideoDraw50TransPixelClip(ColorRed, 0, -1);
    VideoDraw50TransPixelClip(ColorRed, VideoWidth, 0);
    VideoDraw50TransPixelClip(ColorRed, 0, VideoHeight);
    VideoDraw75TransPixelClip(ColorRed, -1, 0);
    VideoDraw75TransPixelClip(ColorRed, 0, -1);
    VideoDraw75TransPixelClip(ColorRed, VideoWidth, 0);
    VideoDraw75TransPixelClip(ColorRed, 0, VideoHeight);
    VideoDrawTransPixelClip(ColorRed, -1, 0, 0);
    VideoDrawTransPixelClip(ColorRed, 0, -1, 0);
    VideoDrawTransPixelClip(ColorRed, VideoWidth, 0, 0);
    VideoDrawTransPixelClip(ColorRed, 0, VideoHeight, 0);

    //Should show blue area getting transparent from left-to-right
    for (y = 0; y < 50; ++y) {
	for (x = 0; x < 50; ++x) {
	    VideoDrawPixel(ColorBlue, x, y);
	}
    }
    for (y = 0; y < 50; ++y) {
	for (x = 50; x < 100; ++x) {
	    VideoDraw25TransPixel(ColorBlue, x, y);
	}
    }
    for (y = 0; y < 50; ++y) {
	for (x = 100; x < 150; ++x) {
	    VideoDraw50TransPixel(ColorBlue, x, y);
	}
    }
    for (y = 0; y < 50; ++y) {
	for (x = 150; x < 200; ++x) {
	    VideoDraw75TransPixel(ColorBlue, x, y);
	}
    }
    for (y = 50; y < 100; ++y) {
	for (x = 0; x < 256; ++x) {
	    VideoDrawTransPixel(ColorBlue, x, y, x);
	}
    }

    // Should show blue+red horizontal bars just below above drawpixel tests
    //getting transparent from top-to-bottom. Clipping should prevent segm.fault
    for (y = 0; y < 10; ++y) {
	VideoDrawHLine(ColorBlue, 0, y + 100, VideoWidth - 1);
	VideoDrawHLineClip(ColorRed, -100, -1, VideoWidth + 200);
	VideoDrawHLineClip(ColorRed, -100, VideoHeight, VideoWidth + 200);
	VideoDrawHLineClip(ColorRed, -100, y + 110, VideoWidth + 200);
	VideoDraw25TransHLine(ColorBlue, 0, y + 120, VideoWidth - 1);
	VideoDraw25TransHLineClip(ColorRed, -100, -1, VideoWidth + 200);
	VideoDraw25TransHLineClip(ColorRed, -100, VideoHeight, VideoWidth + 200);
	VideoDraw25TransHLineClip(ColorRed, -100, y + 130, VideoWidth + 200);
	VideoDraw50TransHLine(ColorBlue, 0, y + 140, VideoWidth - 1);
	VideoDraw50TransHLineClip(ColorRed, -100, -1, VideoWidth + 200);
	VideoDraw50TransHLineClip(ColorRed, -100, VideoHeight, VideoWidth + 200);
	VideoDraw50TransHLineClip(ColorRed, -100, y + 150, VideoWidth + 200);
	VideoDraw75TransHLine(ColorBlue, 0, y + 160, VideoWidth - 1);
	VideoDraw75TransHLineClip(ColorRed, -100, -1, VideoWidth + 200);
	VideoDraw75TransHLineClip(ColorRed, -100, VideoHeight, VideoWidth + 200);
	VideoDraw75TransHLineClip(ColorRed, -100, y + 170, VideoWidth + 200);
    }
    for (y = 0; y < 64; ++y) {
	VideoDrawTransHLine(ColorBlue, 0, y + 180, VideoWidth - 1, y * 4);
	VideoDrawTransHLineClip(ColorRed, -100, -1, VideoWidth + 200, y * 4);
	VideoDrawTransHLineClip(ColorRed, -100, VideoHeight, VideoWidth + 200, y * 4);
	VideoDrawTransHLineClip(ColorRed, -100, y + 180 + 64 + 6 + 6, VideoWidth + 200, (63 - y) * 4);
    }

    // Should show blue+red vertical bars at the right of the screen
    //getting transparent from left-to-right. Clipping should prevent segm.fault
    i = ((VideoWidth - 70 - 70 - 50) / 10) * 10; // starting grid pos for two colums
    for (x = 0; x < 64; ++x) {
	VideoDrawTransVLine(ColorBlue, i + x, 0, VideoHeight - 1, x * 4);
	VideoDrawTransVLineClip(ColorRed, -1, -100, VideoHeight + 200, x * 4);
	VideoDrawTransVLineClip(ColorRed, VideoWidth, -100, VideoHeight + 200,x * 4);
	VideoDrawTransVLineClip(ColorRed, i + 76 + x, -100, VideoHeight + 200,(63 - x) * 4);
    }
    i -= 4 * 2 * 10;
    for (x = 0; x < 10; ++x) {
	VideoDrawVLine(ColorBlue, i + x, 0, VideoHeight - 1);
	VideoDrawVLineClip(ColorRed, -1, -100, VideoHeight + 200);
	VideoDrawVLineClip(ColorRed, VideoWidth, -100, VideoHeight + 200);
	VideoDrawVLineClip(ColorRed, i + x + 10, -100, VideoHeight + 200);
	VideoDraw25TransVLine(ColorBlue, i + x + 20, 0, VideoHeight - 1);
	VideoDraw25TransVLineClip(ColorRed, -1, -100, VideoHeight + 200);
	VideoDraw25TransVLineClip(ColorRed, VideoWidth, -100, VideoHeight + 200);
	VideoDraw25TransVLineClip(ColorRed, i + x + 30, -100, VideoHeight + 200);
	VideoDraw50TransVLine(ColorBlue, i + x + 40, 0, VideoHeight-1);
	VideoDraw50TransVLineClip(ColorRed, -1, -100, VideoHeight + 200);
	VideoDraw50TransVLineClip(ColorRed, VideoWidth, -100, VideoHeight + 200);
	VideoDraw50TransVLineClip(ColorRed, i + x + 50, -100, VideoHeight + 200);
	VideoDraw75TransVLine(ColorBlue, i + x + 60,0, VideoHeight-1);
	VideoDraw75TransVLineClip(ColorRed,-1, -100, VideoHeight + 200);
	VideoDraw75TransVLineClip(ColorRed, VideoWidth, -100, VideoHeight + 200);
	VideoDraw75TransVLineClip(ColorRed, i + x + 70, -100, VideoHeight + 200);
    }

    //Should show filled rectangles in screen's top-right corners
    VideoFillRectangleClip(ColorGray, VideoWidth - 30, -20, 60, 40);
    VideoFill25TransRectangleClip(ColorBlue, VideoWidth - 29, -19, 58, 38);
    VideoFill50TransRectangleClip(ColorRed, VideoWidth - 28, -18, 56, 36);
    VideoFill75TransRectangleClip(ColorGreen, VideoWidth - 27, -17, 54, 34);
    VideoFillTransRectangleClip(ColorBlue, VideoWidth - 26, -16, 52, 32, 64);

    //Should show red area in lower-left getting transparent from left-to-right
    i = ((VideoHeight - 20) / 10) * 10; // starting grid pos for two colums
    VideoFillRectangle(ColorRed, 0, i, 50, 20);
    VideoFill25TransRectangle(ColorRed, 50, i, 50, 20);
    VideoFill50TransRectangle(ColorRed, 100, i, 50, 20);
    VideoFill75TransRectangle(ColorRed, 150, i, 50, 20);
    i -= 20;
    for (x = 0; x < 256; ++x) {
	VideoFillTransRectangle(ColorRed, x, i, 1, 20, x);
    }

    //Should show red/green/blue rectangles in lower-left transparent from
    //left-to-right
    i -= 20;
    for (x = 0; x < 10; ++x) {
	VideoDrawRectangle(ColorBlue, x, i + x, 50 - 2 * x, 20 - 2 * x);
	VideoDraw25TransRectangle(ColorBlue, 50 + x, i + x, 50 - 2 * x, 20 - 2 * x);
	VideoDraw50TransRectangle(ColorBlue, 100 + x, i + x, 50 - 2 * x, 20 - 2 * x);
	VideoDraw75TransRectangle(ColorBlue, 150 + x, i + x, 50 - 2 * x, 20 - 2 * x);
    }
    i -= 20;
    for (x = 0;x < 256; ++x) {
	VideoDrawTransRectangle(ColorGreen, x, i, 1, 20, x);
    }
    i -= 20;
    for (x = 0; x < 128; ++x) {
	VideoDrawTransRectangle(ColorRed, 2 * x, i, 2, 20, x * 2);
    }

    //Should show rectangles in screen's bottom-right corners
    VideoDrawRectangleClip(ColorGray, VideoWidth - 30, VideoHeight - 20, 60, 40);
    VideoDraw25TransRectangleClip(ColorBlue, VideoWidth - 29, VideoHeight - 19, 58, 38);
    VideoDraw50TransRectangleClip(ColorRed, VideoWidth - 28, VideoHeight - 18, 56, 36);
    VideoDraw75TransRectangleClip(ColorGreen, VideoWidth - 27, VideoHeight - 17, 54, 34);
    VideoDrawTransRectangleClip(ColorBlue, VideoWidth - 26, VideoHeight - 16, 52, 32, 64);

    //Display grid of 10x10 (to detect errors more easier)
    for (y = 0; y < VideoHeight; y += 10) {
	for (x = 0; x < VideoWidth; x += 10) {
	    VideoDrawPixel(ColorWhite, x, y);
	}
    }

    //Should show white pixel in lower-right corner (not prevented by clippingi)
    VideoDrawPixelClip(ColorWhite, VideoWidth - 1, VideoHeight - 1);
}

/**
**      Show colorcube (only for 8bpp!) and common+current palette
*/
global void DebugTestDisplayColorCube(void)
{
    int i;
    int x;
    int y;

    for (i = 0; i < 32; ++i) {
	for (y = 0; y < 32; ++y) {
	    for (x = 0; x < 32; ++x) {
		int a;
		int b;
		a = (x + (i % 10) * 32) * 2;
		b = (y + (i / 10) * 32) * 2;
		VideoMemory8[a+b*VideoWidth] =
		    VideoMemory8[a + 1 + b * VideoWidth] =
		    VideoMemory8[a + (b + 1) * VideoWidth] =
		    VideoMemory8[a + 1 + (b + 1) * VideoWidth] =
		    colorcube8[(i << 10) | (y << 5) | x];
	    }
	}
    }
    for (i = 0; i < 256; ++i) {
	VideoMemory8[i + 400 * VideoWidth] =
	    VideoMemory8[i + 401 * VideoWidth] = i;
    }
    for (i = 0; i < 256; ++i) {
	VideoMemory8[i + 403 * VideoWidth] =
	    VideoMemory8[i + 404 * VideoWidth] = Pixels8[i];
    }
}

/**
**      Try all kinds of possible lines (only one time) upon
**      current display, making the job of debugging them more eassier..
**      FIXME: This shows the BUG that 'clipped red lines' are not drawn
**             exactly upon 'unclipped blue lines'.
*/
global void DebugTestDisplayLines(void)
{
    int x;
    int y;
    int i;
    int j;

    /* ClearTheScreen */
    for (y = 0; y < VideoHeight; ++y) {
	for (x = 0; x < VideoWidth; ++x) {
	    VideoDrawPixel(ColorBlack, x, y);
	}
    }

    /* draw lines in each possible direction
    y = VideoHeight < VideoWidth ? VideoHeight : VideoWidth;
    for (x = 0; x < y; x += 10) {
	VideoDrawLine(ColorBlue, 0, x, y - x - 1, 0);
    }
    */
    i = (VideoWidth / 10) * 10;
    j = (VideoHeight / 10) * 10;
    for (x = 0; x <= i; x += 10) {
	VideoDrawLine(ColorBlue, 0, 0, x, j);
    }
    for (y = 0; y <= j; y += 10) {
	VideoDrawLine(ColorBlue, 0, 0, i, y);
    }

    SetClipping(20, 20, (VideoWidth / 10) * 10 - 20, (VideoHeight / 10) * 10 - 20);
    for (x = 0; x <= i; x += 10) {
	VideoDrawLineClip(ColorRed, 0, 0, x, j);
    }
    for (y = 0; y <= j; y += 10) {
	VideoDrawLineClip(ColorRed, 0, 0, i, y);
    }
    /*
    i = (VideoWidth / 10) * 10 + 15;
    j = (VideoHeight / 10) * 10 + 15;
    for (x = i; x >= -15; x -= 10) {
	VideoDrawLineClip(ColorRed, i, 0, x, j);
    }
    for (y = j; y >= -15; y -= 10) {
	VideoDrawLineClip(ColorRed, i, 0, -15, y);
    }
    */

    /* Display grid of 10x10 (to detect errors more easier) */
    for (y = 0; y < VideoHeight; y += 10) {
	for (x = 0; x < VideoWidth; x += 10) {
	    VideoDrawPixel(ColorWhite, x, y);
	}
    }
}
#endif // ifdef USE_SDL_SURFACE

#endif	// } DEBUG

//@}
