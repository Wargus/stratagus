//       _________ __                 __
//      /   _____//  |_____________ _/  |______     ____  __ __  ______
//      \_____  \\   __\_  __ \__  \\   __\__  \   / ___\|  |  \/  ___/
//      /        \|  |  |  | \// __ \|  |  / __ \_/ /_/  >  |  /\___ |
//     /_______  /|__|  |__|  (____  /__| (____  /\___  /|____//____  >
//             \/                  \/          \//_____/            \/
//  ______________________                           ______________________
//                        T H E   W A R   B E G I N S
//         Stratagus - A free fantasy real time strategy game engine
//
/**@name linedraw.c - The general linedraw functions. */
//
//      (c) Copyright 2000-2004 by Lutz Sammer, Stephan Rasenberg,
//                              Jimmy Salmon, Nehal Mistry
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
//      $Id$

//@{

/*----------------------------------------------------------------------------
--		Includes
----------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "stratagus.h"
#include "video.h"

#include "intern_video.h"


/*----------------------------------------------------------------------------
--		Declarations
----------------------------------------------------------------------------*/

/**
**		Bitmask, denoting a postion left/right/above/below clip rectangle
**		(mainly used by VideoDrawLineClip)
*/
typedef enum {
   ClipCodeInside = 0,						/// Clipping inside rectangle
   ClipCodeAbove  = 1,						/// Clipping above rectangle
   ClipCodeBelow  = 2,						/// Clipping below rectangle
   ClipCodeLeft   = 4,						/// Clipping left rectangle
   ClipCodeRight  = 8						/// Clipping right rectangle
} ClipCode;

/*----------------------------------------------------------------------------
--		Variables
----------------------------------------------------------------------------*/

global void (*VideoDrawPixel)(Uint32 color, int x, int y);
local void (*VideoDoDrawPixel)(Uint32 color, int x, int y);
global void (*VideoDrawTransPixel)(Uint32 color, int x, int y, unsigned char alpha);
local void (*VideoDoDrawTransPixel)(Uint32 color, int x, int y, unsigned char alpha);
global void VideoDrawPixelClip(Uint32 color, int x, int y);
global void VideoDrawTransPixelClip(Uint32 color, int x, int y, unsigned char alpha);
global void VideoDrawVLine(Uint32 color, int x, int y, int width);
global void VideoDrawVLineClip(Uint32 color, int x, int y, int width);
global void VideoDrawTransVLine(Uint32 color, int x, int y,
	int height, unsigned char alpha);
global void VideoDrawHLine(Uint32 color, int x, int y, int width);
global void VideoDrawHLineClip(Uint32 color, int x, int y, int width);
global void VideoDrawTransHLine(Uint32 color, int x, int y,
	int width, unsigned char alpha);
global void VideoDrawLine(Uint32 color, int sx, int sy, int dx, int dy);
global void VideoDrawTransLine(Uint32 color, int sx, int sy,
	int dx, int dy, unsigned char alpha);
global void VideoDrawRectangle(Uint32 color, int x, int y,
	int w, int h);
global void VideoDrawRectangleClip(Uint32 color, int x, int y,
	int w, int h);
global void VideoDrawTransRectangle(Uint32 color, int x, int y,
	int w, int h, unsigned char alpha);
global void VideoFillRectangle(Uint32 color, int x, int y,
	int w, int h);
global void VideoFillTransRectangle(Uint32 color, int x, int y,
	int w, int h, unsigned char alpha);

// ===========================================================================
//		Pixel
// ===========================================================================

/**
**  FIXME: docu
*/
local void VideoDoDrawPixel16(Uint32 color, int x, int y)
{
	((Uint16*)TheScreen->pixels)[x + y * VideoWidth] = color;
}

/**
**  FIXME: docu
*/
global void VideoDrawPixel16(Uint32 color, int x, int y)
{
	VideoLockScreen();
	VideoDoDrawPixel16(color, x, y);
	VideoUnlockScreen();
}

/**
**  FIXME: docu
*/
local void VideoDoDrawPixel32(Uint32 color, int x, int y)
{
	((Uint32*)TheScreen->pixels)[x + y * VideoWidth] = color;
}

/**
**  FIXME: docu
*/
global void VideoDrawPixel32(Uint32 color, int x, int y)
{
	VideoLockScreen();
	VideoDoDrawPixel32(color, x, y);
	VideoUnlockScreen();
}

/**
**  FIXME: docu
*/
local void VideoDoDrawTransPixel16(Uint32 color, int x, int y, unsigned char alpha)
{
	Uint16* p;
	unsigned long dp;

	// Loses precision for speed
	alpha = (255 - alpha) >> 3;

	p = &((Uint16*)TheScreen->pixels)[x + y * VideoWidth];
	color = (((color << 16) | color) & 0x07E0F81F);
	dp = *p;
	dp = ((dp << 16) | dp) & 0x07E0F81F;
	dp = ((((dp - color) * alpha) >> 5) + color) & 0x07E0F81F;
	*p = (dp >> 16) | dp;
}

/**
**  FIXME: docu
*/
global void VideoDrawTransPixel16(Uint32 color, int x, int y, unsigned char alpha)
{
	VideoLockScreen();
	VideoDoDrawTransPixel16(color, x, y, alpha);
	VideoUnlockScreen();
}

/**
**  FIXME: docu
*/
local void VideoDoDrawTransPixel32(Uint32 color, int x, int y, unsigned char alpha)
{
	unsigned long sp2;
	unsigned long dp1;
	unsigned long dp2;
	Uint32* p;

	alpha = 255 - alpha;

	VideoLockScreen();

	p = &((Uint32*)TheScreen->pixels)[x + y * VideoWidth];

	sp2 = (color & 0xFF00FF00) >> 8;
	color &= 0x00FF00FF;

	dp1 = *p;
	dp2 = (dp1 & 0xFF00FF00) >> 8;
	dp1 &= 0x00FF00FF;

	dp1 = ((((dp1-color) * alpha) >> 8) + color) & 0x00FF00FF;
	dp2 = ((((dp2-sp2) * alpha) >> 8) + sp2) & 0x00FF00FF;
	*p = (dp1 | (dp2 << 8));

	VideoUnlockScreen();
}

/**
**  FIXME: docu
*/
global void VideoDrawTransPixel32(Uint32 color, int x, int y, unsigned char alpha)
{
	VideoLockScreen();
	VideoDoDrawTransPixel32(color, x, y, alpha);
	VideoUnlockScreen();
}

/**
**  FIXME: docu
*/
local void VideoDoDrawPixelClip(Uint32 color, int x, int y)
{
	if (x >= ClipX1 && y >= ClipY1 && x <= ClipX2 && y <= ClipY2) {
		VideoDoDrawPixel(color, x, y);
	}
}

/**
**  FIXME: docu
*/
global void VideoDrawPixelClip(Uint32 color, int x, int y)
{
	VideoLockScreen();
	VideoDoDrawPixelClip(color, x, y);
	VideoUnlockScreen();
}

/**
**  FIXME: docu
*/
local void VideoDoDrawTransPixelClip(Uint32 color, int x, int y, unsigned char alpha)
{
	if (x >= ClipX1 && y >= ClipY1 && x <= ClipX2 && y <= ClipY2) {
		VideoDoDrawTransPixel(color, x, y, alpha);
	}
}

/**
**  FIXME: docu
*/
global void VideoDrawTransPixelClip(Uint32 color, int x, int y, unsigned char alpha)
{
	VideoLockScreen();
	VideoDoDrawTransPixelClip(color, x, y, alpha);
	VideoUnlockScreen();
}

/**
**  FIXME: docu
*/
global void VideoDrawVLine(Uint32 color, int x, int y, int height)
{
	int i;

	VideoLockScreen();
	for (i = 0; i < height; ++i) {
		VideoDoDrawPixel(color, x, y + i);
	}
	VideoUnlockScreen();
}

/**
**  FIXME: docu
*/
global void VideoDrawTransVLine(Uint32 color, int x, int y,
	int height, unsigned char alpha)
{
	int i;

	VideoLockScreen();
	for (i = 0; i < height; ++i) {
		VideoDoDrawTransPixel(color, x, y + i, alpha);
	}
	VideoUnlockScreen();
}

/**
**  FIXME: docu
*/
global void VideoDrawVLineClip(Uint32 color, int x, int y, int height)
{
	int w;
	
	w = 1;
	CLIP_RECTANGLE(x, y, w, height);
	VideoDrawVLine(color, x, y, height);
}

/**
**  FIXME: docu
*/
global void VideoDrawTransVLineClip(Uint32 color, int x, int y,
	int height, unsigned char alpha)
{
	int i;

	VideoLockScreen();
	for (i = 0; i < height; ++i) {
		VideoDoDrawTransPixelClip(color, x, y + i, alpha);
	}
	VideoUnlockScreen();
}

/**
**  FIXME: docu
*/
global void VideoDrawHLine(Uint32 color, int x, int y, int width)
{
	int i;

	VideoLockScreen();
	for (i = 0; i < width; ++i) {
		VideoDoDrawPixel(color, x + i, y);
	}
	VideoUnlockScreen();
}

/**
**  FIXME: docu
*/
global void VideoDrawHLineClip(Uint32 color, int x, int y, int width)
{
	int h;
	
	h = 1;
	CLIP_RECTANGLE(x, y, width, h);
	VideoDrawHLine(color, x, y, width);
}

/**
**  FIXME: docu
*/
global void VideoDrawTransHLine(Uint32 color, int x, int y,
	int width, unsigned char alpha)
{
	int i;

	VideoLockScreen();
	for (i = 0; i < width; ++i) {
		VideoDoDrawTransPixel(color, x + i, y, alpha);
	}
	VideoUnlockScreen();
}

/**
**  FIXME: docu
*/
global void VideoDrawTransHLineClip(Uint32 color, int x, int y,
	int width, unsigned char alpha)
{
	int i;

	VideoLockScreen();
	for (i = 0; i < width; ++i) {
		VideoDoDrawTransPixelClip(color, x + i, y, alpha);
	}
	VideoUnlockScreen();
}

/**
**  FIXME: docu
*/
global void VideoDrawLine(Uint32 color, int sx, int sy, int dx, int dy)
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

		VideoLockScreen();
		for (x = sx; x < dx; ++x) {
			VideoDoDrawPixel(color, x, y);
			if (p >= 0) {
				y += incr;
				p += (ylen - xlen) << 1;
			} else {
				p += (ylen << 1);
			}
		}
		VideoUnlockScreen();
		return;
	}

	if (ylen > xlen) {
		int p;

		p = (xlen << 1) - ylen;

		VideoLockScreen();
		for (y = sy; y < dy; ++y) {
			VideoDoDrawPixel(color, x, y);
			if (p >= 0) {
				x += incr;
				p += (xlen - ylen) << 1;
			} else {
				p += (xlen << 1);
			}
		}
		VideoUnlockScreen();
		return;
	}

	// Draw a diagonal line
	if (ylen == xlen) {
		VideoLockScreen();
		while (y != dy) {
			VideoDoDrawPixel(color, x, y);
			x += incr;
			++y;
		}
		VideoUnlockScreen();
	}
}

/**
**  FIXME: docu
*/
global void VideoDrawLineClip(Uint32 color, int sx, int sy, int dx, int dy)
{
	int x;
	int y;
	int xlen;
	int ylen;
	int incr;

	if (sx == dx) {
		if (sy < dy) {
			VideoDrawVLineClip(color, sx, sy, dy - sy + 1);
		} else {
			VideoDrawVLineClip(color, dx, dy, sy - dy + 1);
		}
		return;
	}

	if (sy == dy) {
		if (sx < dx) {
			VideoDrawHLineClip(color, sx, sy, dx - sx + 1);
		} else {
			VideoDrawHLineClip(color, dx, dy, sx - dx + 1);
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

		VideoLockScreen();
		for (x = sx; x < dx; ++x) {
			VideoDoDrawPixelClip(color, x, y);
			if (p >= 0) {
				y += incr;
				p += (ylen - xlen) << 1;
			} else {
				p += (ylen << 1);
			}
		}
		VideoUnlockScreen();
		return;
	}

	if (ylen > xlen) {
		int p;

		p = (xlen << 1) - ylen;

		VideoLockScreen();
		for (y = sy; y < dy; ++y) {
			VideoDoDrawPixelClip(color, x, y);
			if (p >= 0) {
				x += incr;
				p += (xlen - ylen) << 1;
			} else {
				p += (xlen << 1);
			}
		}
		VideoUnlockScreen();
		return;
	}

	// Draw a diagonal line
	if (ylen == xlen) {
		VideoLockScreen();
		while (y != dy) {
			VideoDoDrawPixelClip(color, x, y);
			x += incr;
			++y;
		}
		VideoUnlockScreen();
	}
}

/**
**  FIXME: docu
*/
global void VideoDrawTransLine(Uint32 color, int sx, int sy,
	int dx, int dy, unsigned char alpha)
{
	// FIXME: trans
	VideoDrawLine(color, sx, sy, dx, dy);
}

/**
**  FIXME: docu
*/
global void VideoDrawRectangle(Uint32 color, int x, int y,
	int w, int h)
{
	VideoDrawHLine(color, x, y, w);
	VideoDrawHLine(color, x, y + h - 1, w);

	VideoDrawVLine(color, x, y + 1, h - 2);
	VideoDrawVLine(color, x + w - 1, y + 1, h - 2);
}

/**
**  FIXME: docu
*/
global void VideoDrawRectangleClip(Uint32 color, int x, int y,
	int w, int h)
{
	VideoDrawHLineClip(color, x, y, w);
	VideoDrawHLineClip(color, x, y + h - 1, w);

	VideoDrawVLineClip(color, x, y + 1, h - 2);
	VideoDrawVLineClip(color, x + w - 1, y + 1, h - 2);
}

/**
**  FIXME: docu
*/
global void VideoDrawTransRectangle(Uint32 color, int x, int y,
	int w, int h, unsigned char alpha)
{
	VideoDrawTransHLine(color, x, y, w, alpha);
	VideoDrawTransHLine(color, x, y + h - 1, w, alpha);

	VideoDrawTransVLine(color, x, y + 1, h - 2, alpha);
	VideoDrawTransVLine(color, x + w - 1, y + 1, h - 2, alpha);
}

/**
**  FIXME: docu
*/
global void VideoFillRectangle(Uint32 color, int x, int y,
	int w, int h)
{
	SDL_Rect drect;

	drect.x = x;
	drect.y = y;
	drect.w = w;
	drect.h = h;

	SDL_FillRect(TheScreen, &drect, color);
}

/**
**  FIXME: docu
*/
global void VideoFillRectangleClip(Uint32 color, int x, int y,
	int w, int h)
{
	SDL_Rect oldrect;
	SDL_Rect newrect;

	SDL_GetClipRect(TheScreen, &oldrect);
	newrect.x = ClipX1;
	newrect.y = ClipY1;
	newrect.w = ClipX2 + 1 - ClipX1;
	newrect.h = ClipY2 + 1 - ClipY1;

	SDL_SetClipRect(TheScreen, &newrect);
	VideoFillRectangle(color, x, y, w, h);
	SDL_SetClipRect(TheScreen, &oldrect);
}

/**
**  FIXME: docu
*/
global void VideoFillTransRectangle(Uint32 color, int x, int y,
	int w, int h, unsigned char alpha)
{
	SDL_Rect drect;
	SDL_Surface* s;
	unsigned char r;
	unsigned char g;
	unsigned char b;

	// FIXME: optimize
	s = SDL_CreateRGBSurface(SDL_SWSURFACE, w, h,
		24, RMASK, GMASK, BMASK, 0);

	SDL_GetRGB(color, TheScreen->format, &r, &g, &b);
	color = SDL_MapRGB(s->format, r, g, b);

	SDL_FillRect(s, NULL, color);

	drect.x = x;
	drect.y = y;

	SDL_SetAlpha(s, SDL_SRCALPHA | SDL_RLEACCEL, alpha);
	SDL_BlitSurface(s, NULL, TheScreen, &drect);
	SDL_FreeSurface(s);
}

/**
**  FIXME: docu
*/
global void VideoFillTransRectangleClip(Uint32 color, int x, int y,
	int w, int h, unsigned char alpha)
{
	SDL_Rect oldrect;
	SDL_Rect newrect;

	SDL_GetClipRect(TheScreen, &oldrect);
	newrect.x = ClipX1;
	newrect.y = ClipY1;
	newrect.w = ClipX2 + 1 - ClipX1;
	newrect.h = ClipY2 + 1 - ClipY1;

	SDL_SetClipRect(TheScreen, &newrect);
	VideoFillTransRectangle(color, x, y, w, h, alpha);
	SDL_SetClipRect(TheScreen, &oldrect);
}

/**
**  FIXME: docu
*/
global void VideoDrawCircle(Uint32 color, int x, int y, int r)
{
	int p;
	int px;
	int py;

	p = 1 - r;
	py = r;

	VideoLockScreen();
	for (px = 0; px <= py + 1; ++px) {
		VideoDoDrawPixel(color, x + px, y + py);
		VideoDoDrawPixel(color, x + px, y - py);
		VideoDoDrawPixel(color, x - px, y + py);
		VideoDoDrawPixel(color, x - px, y - py);

		VideoDoDrawPixel(color, x + py, y + px);
		VideoDoDrawPixel(color, x + py, y - px);
		VideoDoDrawPixel(color, x - py, y + px);
		VideoDoDrawPixel(color, x - py, y - px);

		if (p < 0) {
			p += 2 * px + 3;
		} else {
			p += 2 * (px - py) + 5;
			py -= 1;
		}
	}
	VideoUnlockScreen();
}

/**
**  FIXME: docu
*/
global void VideoDrawTransCircle(Uint32 color, int x, int y,
	int r, unsigned char alpha)
{
	int p;
	int px;
	int py;

	p = 1 - r;
	py = r;

	VideoLockScreen();
	for (px = 0; px <= py + 1; ++px) {
		VideoDoDrawTransPixel(color, x + px, y + py, alpha);
		VideoDoDrawTransPixel(color, x + px, y - py, alpha);
		VideoDoDrawTransPixel(color, x - px, y + py, alpha);
		VideoDoDrawTransPixel(color, x - px, y - py, alpha);

		VideoDoDrawTransPixel(color, x + py, y + px, alpha);
		VideoDoDrawTransPixel(color, x + py, y - px, alpha);
		VideoDoDrawTransPixel(color, x - py, y + px, alpha);
		VideoDoDrawTransPixel(color, x - py, y - px, alpha);

		if (p < 0) {
			p += 2 * px + 3;
		} else {
			p += 2 * (px - py) + 5;
			py -= 1;
		}
	}
	VideoUnlockScreen();
}

/**
**  FIXME: docu
*/
global void VideoDrawCircleClip(Uint32 color, int x, int y, int r)
{
	int p;
	int px;
	int py;

	p = 1 - r;
	py = r;

	VideoLockScreen();
	for (px = 0; px <= py + 1; ++px) {
		VideoDoDrawPixelClip(color, x + px, y + py);
		VideoDoDrawPixelClip(color, x + px, y - py);
		VideoDoDrawPixelClip(color, x - px, y + py);
		VideoDoDrawPixelClip(color, x - px, y - py);

		VideoDoDrawPixelClip(color, x + py, y + px);
		VideoDoDrawPixelClip(color, x + py, y - px);
		VideoDoDrawPixelClip(color, x - py, y + px);
		VideoDoDrawPixelClip(color, x - py, y - px);

		if (p < 0) {
			p += 2 * px + 3;
		} else {
			p += 2 * (px - py) + 5;
			py -= 1;
		}
	}
	VideoUnlockScreen();
}

/**
**  FIXME: docu
*/
global void VideoDrawTransCircleClip(Uint32 color, int x, int y,
	int r, unsigned char alpha)
{
	int p;
	int px;
	int py;

	p = 1 - r;
	py = r;

	VideoLockScreen();
	for (px = 0; px <= py + 1; ++px) {
		VideoDoDrawTransPixelClip(color, x + px, y + py, alpha);
		VideoDoDrawTransPixelClip(color, x + px, y - py, alpha);
		VideoDoDrawTransPixelClip(color, x - px, y + py, alpha);
		VideoDoDrawTransPixelClip(color, x - px, y - py, alpha);

		VideoDoDrawTransPixelClip(color, x + py, y + px, alpha);
		VideoDoDrawTransPixelClip(color, x + py, y - px, alpha);
		VideoDoDrawTransPixelClip(color, x - py, y + px, alpha);
		VideoDoDrawTransPixelClip(color, x - py, y - px, alpha);

		if (p < 0) {
			p += 2 * px + 3;
		} else {
			p += 2 * (px - py) + 5;
			py -= 1;
		}
	}
	VideoUnlockScreen();
}

/**
**  FIXME: docu
*/
global void VideoFillCircle(Uint32 color, int x, int y, int r)
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
			if (py >= px) {
				VideoDrawVLine(color, x + py + 1, y, px + 1);
				VideoDrawVLine(color, x + py + 1, y - px, px);
				VideoDrawVLine(color, x - py - 1, y, px + 1);
				VideoDrawVLine(color, x - py - 1, y - px,  px);
			}
		}
	}
}

/**
**  FIXME: docu
*/
global void VideoFillTransCircle(Uint32 color, int x, int y,
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
			if (py >= px) {
				VideoDrawTransVLine(color, x + py + 1, y, px + 1, alpha);
				VideoDrawTransVLine(color, x + py + 1, y - px, px, alpha);
				VideoDrawTransVLine(color, x - py - 1, y, px + 1, alpha);
				VideoDrawTransVLine(color, x - py - 1, y - px,  px, alpha);
			}
		}
	}
}

/**
**  FIXME: docu
*/
global void VideoFillCircleClip(Uint32 color, int x, int y, int r)
{
	int p;
	int px;
	int py;

	p = 1 - r;
	py = r;

	for (px = 0; px <= py; ++px) {

		// Fill up the middle half of the circle
		VideoDrawVLineClip(color, x + px, y, py + 1);
		VideoDrawVLineClip(color, x + px, y - py, py);
		if (px) {
			VideoDrawVLineClip(color, x - px, y, py + 1);
			VideoDrawVLineClip(color, x - px, y - py, py);
		}

		if (p < 0) {
			p += 2 * px + 3;
		} else {
			p += 2 * (px - py) + 5;
			py -= 1;

			// Fill up the left/right half of the circle
			if (py >= px) {
				VideoDrawVLineClip(color, x + py + 1, y, px + 1);
				VideoDrawVLineClip(color, x + py + 1, y - px, px);
				VideoDrawVLineClip(color, x - py - 1, y, px + 1);
				VideoDrawVLineClip(color, x - py - 1, y - px,  px);
			}
		}
	}
}

/**
**  FIXME: docu
*/
global void VideoFillTransCircleClip(Uint32 color, int x, int y,
	int r, unsigned char alpha)
{
	int p;
	int px;
	int py;

	p = 1 - r;
	py = r;

	for (px = 0; px <= py + 1; ++px) {

		// Fill up the middle half of the circle
		VideoDrawTransVLineClip(color, x + px, y, py + 1, alpha);
		VideoDrawTransVLineClip(color, x + px, y - py, py, alpha);
		if (px) {
			VideoDrawTransVLineClip(color, x - px, y, py + 1, alpha);
			VideoDrawTransVLineClip(color, x - px, y - py, py, alpha);
		}

		if (p < 0) {
			p += 2 * px + 3;
		} else {
			p += 2 * (px - py) + 5;
			py -= 1;

			// Fill up the left/right half of the circle
			if (py >= px) {
				VideoDrawTransVLineClip(color, x + py + 1, y, px + 1, alpha);
				VideoDrawTransVLineClip(color, x + py + 1, y - px, px, alpha);
				VideoDrawTransVLineClip(color, x - py - 1, y, px + 1, alpha);
				VideoDrawTransVLineClip(color, x - py - 1, y - px,  px, alpha);
			}
		}
	}
}

/**
**  FIXME: docu
*/
global void InitLineDraw()
{
	switch (VideoDepth) {
		case 16:
			VideoDrawPixel = VideoDrawPixel16;
			VideoDoDrawPixel = VideoDoDrawPixel16;
			VideoDrawTransPixel = VideoDrawTransPixel16;
			VideoDoDrawTransPixel = VideoDoDrawTransPixel16;
			break;
		case 32:
			VideoDrawPixel = VideoDrawPixel32;
			VideoDoDrawPixel = VideoDoDrawPixel32;
			VideoDrawTransPixel = VideoDrawTransPixel32;
			VideoDoDrawTransPixel = VideoDoDrawTransPixel32;
	}
}

#if 0

/**
**		Draw pixel unclipped.
**
**		@param color		color
**		@param x		x coordinate on the screen
**		@param y		y coordinate on the screen
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
**		Draw 25% translucent pixel unclipped.
**
**		@param color		color
**		@param x		x coordinate on the screen
**		@param y		y coordinate on the screen
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
**		Draw 50% translucent pixel unclipped.
**
**		@param color		color
**		@param x		x coordinate on the screen
**		@param y		y coordinate on the screen
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
**		Draw 75% translucent pixel unclipped.
**
**		@param color		color
**		@param x		x coordinate on the screen
**		@param y		y coordinate on the screen
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

/**
**		Draw translucent pixel unclipped.
**
**		@param color		color
**		@param x		x coordinate on the screen
**		@param y		y coordinate on the screen
**	  @param alpha	alpha value of pixel.
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
**		Draw pixel clipped to current clip setting.
**
**		@param color		color
**		@param x		x coordinate on the screen
**		@param y		y coordinate on the screen
*/
#ifdef USE_OPENGL
local void DrawPixelClipOpenGL(VMemType color, int x, int y)
{
	VMemType32 c;
	GLubyte r;
	GLubyte g;
	GLubyte b;

	//		Clipping:
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
**		Draw horizontal line unclipped.
**
**		@param color		color
**		@param x		x coordinate on the screen
**		@param y		y coordinate on the screen
**		@param width		width of line (0=don't draw).
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
**		Draw 25% translucent horizontal line unclipped.
**
**		@param color		color
**		@param x		x coordinate on the screen
**		@param y		y coordinate on the screen
**		@param width		width of line (0=don't draw).
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
**		Draw 50% translucent horizontal line unclipped.
**
**		@param color		color
**		@param x		x coordinate on the screen
**		@param y		y coordinate on the screen
**		@param width		width of line (0=don't draw).
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
**		Draw 75% translucent horizontal line unclipped.
**
**		@param color		color
**		@param x		x coordinate on the screen
**		@param y		y coordinate on the screen
**		@param width		width of line (0=don't draw).
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
**		Draw translucent horizontal line unclipped.
**
**		@param color		color
**		@param x		x coordinate on the screen
**		@param y		y coordinate on the screen
**		@param width		width of line (0=don't draw).
**		@param alpha		alpha value of pixels.
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
**		Draw vertical line unclipped.
**
**		@param color		color
**		@param x		x coordinate on the screen
**		@param y		y coordinate on the screen
**		@param height		height of line (0=don't draw).
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
**		Draw 25% translucent vertical line unclipped.
**
**		@param color		color
**		@param x		x coordinate on the screen
**		@param y		y coordinate on the screen
**		@param height		height of line (0=don't draw).
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
**		Draw 50% translucent vertical line unclipped.
**
**		@param color		color
**		@param x		x coordinate on the screen
**		@param y		y coordinate on the screen
**		@param height		height of line (0=don't draw).
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
**		Draw 75% translucent vertical line unclipped.
**
**		@param color		color
**		@param x		x coordinate on the screen
**		@param y		y coordinate on the screen
**		@param height		height of line (0=don't draw).
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
**		Draw translucent vertical line unclipped.
**
**		@param color		color
**		@param x		x coordinate on the screen
**		@param y		y coordinate on the screen
**		@param height		height of line (0=don't draw).
**		@param alpha		alpha value of pixels.
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
**		Draw line unclipped into 32bit framebuffer.
**
**		@param color		color
**		@param x1		Source x coordinate on the screen
**		@param y1		Source y coordinate on the screen
**		@param x2		Destination x coordinate on the screen
**		@param y2		Destination y coordinate on the screen
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
**		Draw rectangle.
**
**		@param color		color
**		@param x		x coordinate on the screen
**		@param y		y coordinate on the screen
**		@param h		height of rectangle (0=don't draw).
**		@param w		width of rectangle (0=don't draw).
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
**		Draw 25% translucent rectangle.
**
**		@param color		color
**		@param x		x coordinate on the screen
**		@param y		y coordinate on the screen
**		@param h		height of rectangle (0=don't draw).
**		@param w		width of rectangle (0=don't draw).
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
**		Draw 50% translucent rectangle into 32bpp frame buffer.
**
**		@param color		color
**		@param x		x coordinate on the screen
**		@param y		y coordinate on the screen
**		@param h		height of rectangle (0=don't draw).
**		@param w		width of rectangle (0=don't draw).
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
**		Draw 75% translucent rectangle.
**
**		@param color		color
**		@param x		x coordinate on the screen
**		@param y		y coordinate on the screen
**		@param h		height of rectangle (0=don't draw).
**		@param w		width of rectangle (0=don't draw).
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
**		Draw translucent rectangle.
**
**		@param color		color
**		@param x		x coordinate on the screen
**		@param y		y coordinate on the screen
**		@param h		height of rectangle (0=don't draw).
**		@param w		width of rectangle (0=don't draw).
**	  @param alpha	alpha value of pixel.
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
**		Fill rectangle.
**
**		@param color		color
**		@param x		x coordinate on the screen
**		@param y		y coordinate on the screen
**		@param h		height of rectangle (0=don't draw).
**		@param w		width of rectangle (0=don't draw).
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
**		Fill rectangle 25% translucent clipped.
**
**		@param color		color
**		@param x		x coordinate on the screen
**		@param y		y coordinate on the screen
**		@param h		height of rectangle (0=don't draw).
**		@param w		width of rectangle (0=don't draw).
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
**		Fill rectangle 50% translucent clipped.
**
**		@param color		color
**		@param x		x coordinate on the screen
**		@param y		y coordinate on the screen
**		@param h		height of rectangle (0=don't draw).
**		@param w		width of rectangle (0=don't draw).
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
**		Fill rectangle 75% translucent clipped.
**
**		@param color		color
**		@param x		x coordinate on the screen
**		@param y		y coordinate on the screen
**		@param h		height of rectangle (0=don't draw).
**		@param w		width of rectangle (0=don't draw).
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
**		Draw translucent rectangle.
**
**		@param color		color
**		@param x		x coordinate on the screen
**		@param y		y coordinate on the screen
**		@param h		height of rectangle (0=don't draw).
**		@param w		width of rectangle (0=don't draw).
**	  @param alpha	alpha value of pixel.
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

#endif

//@}
