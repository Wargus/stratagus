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
//      the Free Software Foundation; only version 2 of the License.
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

#ifndef USE_OPENGL
global void (*VideoDrawPixel)(Uint32 color, int x, int y);
local void (*VideoDoDrawPixel)(Uint32 color, int x, int y);
global void (*VideoDrawTransPixel)(Uint32 color, int x, int y, unsigned char alpha);
local void (*VideoDoDrawTransPixel)(Uint32 color, int x, int y, unsigned char alpha);
#endif

// ===========================================================================
//		Pixel
// ===========================================================================

#ifndef USE_OPENGL

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
	color = VideoMapRGB(s->format, r, g, b);

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

	for (px = 0; px <= py; ++px) {

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

	for (px = 0; px <= py; ++px) {

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
global void InitLineDraw(void)
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

#else

/**
**  Draw pixel unclipped.
**
**  @param color  color
**  @param x      x coordinate on the screen
**  @param y      y coordinate on the screen
*/
global void VideoDrawPixel(Uint32 color, int x, int y)
{
	GLubyte r, g, b, a;

	VideoGetRGBA(color, &r, &g, &b, &a);
	glDisable(GL_TEXTURE_2D);
	glColor4ub(r, g, b, a);
	glBegin(GL_POINTS);
	glVertex2i(x, y);
	glEnd();
	glEnable(GL_TEXTURE_2D);
}

/**
**  Draw translucent pixel unclipped.
**
**  @param color  color
**  @param x      x coordinate on the screen
**  @param y      y coordinate on the screen
**  @param alpha  alpha value of pixel.
*/
global void VideoDrawTransPixel(Uint32 color, int x, int y,
	unsigned char alpha)
{
	GLubyte r, g, b;

	VideoGetRGB(color, &r, &g, &b);
	glDisable(GL_TEXTURE_2D);
	glColor4ub(r, g, b, 255 - alpha);
	glBegin(GL_POINTS);
	glVertex2i(x, y);
	glEnd();
	glEnable(GL_TEXTURE_2D);
}

/**
**  Draw pixel clipped to current clip setting.
**
**  @param color  color
**  @param x      x coordinate on the screen
**  @param y      y coordinate on the screen
*/
global void VideoDrawPixelClip(Uint32 color, int x, int y)
{
	GLubyte r, g, b, a;

	// Clipping:
	if (x < ClipX1 || x > ClipX2 || y < ClipY1 || y > ClipY2) {
		return;
	}

	VideoGetRGBA(color, &r, &g, &b, &a);
	glDisable(GL_TEXTURE_2D);
	glColor4ub(r, g, b, a);
	glBegin(GL_POINTS);
	glVertex2i(x, y);
	glEnd();
	glEnable(GL_TEXTURE_2D);
}

/**
**  Draw horizontal line unclipped.
**
**  @param color  color
**  @param x      x coordinate on the screen
**  @param y      y coordinate on the screen
**  @param width  width of line (0=don't draw).
*/
global void VideoDrawHLine(Uint32 color, int x, int y, int width)
{
	GLubyte r, g, b, a;

	VideoGetRGBA(color, &r, &g, &b, &a);
	glDisable(GL_TEXTURE_2D);
	glColor4ub(r, g, b, a);
	glBegin(GL_LINES);
	glVertex2i(x, y);
	glVertex2i(x + width, y);
	glEnd();
	glEnable(GL_TEXTURE_2D);
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
**  Draw horizontal line clipped.
**
**  @param color  color
**  @param x      x coordinate on the screen
**  @param y      y coordinate on the screen
**  @param width  width of line (0=don't draw).
*/
global void VideoDrawHLineClip(Uint32 color, int x, int y, int width)
{
    CLIP_HLINE(x, y, width);
    VideoDrawHLine(color, x, y, width);
}

/**
**  Draw translucent horizontal line unclipped.
**
**  @param color  color
**  @param x      x coordinate on the screen
**  @param y      y coordinate on the screen
**  @param width  width of line (0=don't draw).
**  @param alpha  alpha value of pixels.
*/
global void VideoDrawTransHLine(Uint32 color, int x, int y, int width,
	unsigned char alpha)
{
	GLubyte r, g, b;

	VideoGetRGB(color, &r, &g, &b);
	glDisable(GL_TEXTURE_2D);
	glColor4ub(r, g, b, 255 - alpha);
	glBegin(GL_LINES);
	glVertex2i(x, y);
	glVertex2i(x + width, y);
	glEnd();
	glEnable(GL_TEXTURE_2D);
}

/**
**  Draw translucent horizontal line clipped.
**
**  @param color  Color index
**  @param x      X pixel coordinate on the screen
**  @param y      Y c pixeloordinate on the screen
**  @param width  Width of line (0=don't draw)
**  @param alpha  Alpha value of pixels
*/
global void VideoDrawTransHLineClip(Uint32 color, int x, int y, int width,
	unsigned char alpha)
{
	CLIP_HLINE(x, y, width);
	VideoDrawTransHLine(color, x, y, width, alpha);
}

/**
**  Draw vertical line unclipped.
**
**  @param color   color
**  @param x       x coordinate on the screen
**  @param y       y coordinate on the screen
**  @param height  height of line (0=don't draw).
*/
global void VideoDrawVLine(Uint32 color, int x, int y, int height)
{
	GLubyte r, g, b, a;

	VideoGetRGBA(color, &r, &g, &b, &a);
	glDisable(GL_TEXTURE_2D);
	glColor4ub(r, g, b, a);
	glBegin(GL_LINES);
	glVertex2i(x, y);
	glVertex2i(x, y + height);
	glEnd();
	glEnable(GL_TEXTURE_2D);
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
**  Draw vertical line clipped.
**
**  @param color   color
**  @param x       x coordinate on the screen
**  @param y       y coordinate on the screen
**  @param height  height of line (0=don't draw).
*/
global void VideoDrawVLineClip(Uint32 color, int x, int y, int height)
{
    CLIP_VLINE(x, y, height);
    VideoDrawVLine(color, x, y, height);
}

/**
**  Draw translucent vertical line unclipped.
**
**  @param color   color
**  @param x       x coordinate on the screen
**  @param y       y coordinate on the screen
**  @param height  height of line (0=don't draw).
**  @param alpha   alpha value of pixels.
*/
global void VideoDrawTransVLine(Uint32 color, int x, int y, int height,
	unsigned char alpha)
{
	GLubyte r, g, b;

	VideoGetRGB(color, &r, &g, &b);
	glDisable(GL_TEXTURE_2D);
	glColor4ub(r, g, b, 255 - alpha);
	glBegin(GL_LINES);
	glVertex2i(x, y);
	glVertex2i(x, y + height);
	glEnd();
	glEnable(GL_TEXTURE_2D);
}

/**
**  Draw translucent vertical line clipped.
**
**  @param color   color
**  @param x       x coordinate on the screen
**  @param y       y coordinate on the screen
**  @param height  height of line (0=don't draw).
**  @param alpha   alpha value of pixels.
*/
global void VideoDrawTransVLineClip(Uint32 color, int x, int y,
	int height, unsigned char alpha)
{
	CLIP_VLINE(x, y, height);
	VideoDrawTransVLine(color, x, y, height, alpha);
}

/**
**  Draw line unclipped into 32bit framebuffer.
**
**  @param color  color
**  @param x1     Source x coordinate on the screen
**  @param y1     Source y coordinate on the screen
**  @param x2     Destination x coordinate on the screen
**  @param y2     Destination y coordinate on the screen
*/
global void VideoDrawLine(Uint32 color, int x1, int y1, int x2, int y2)
{
	GLubyte r, g, b, a;

	VideoGetRGBA(color, &r, &g, &b, &a);
	glDisable(GL_TEXTURE_2D);
	glColor4ub(r, g, b, a);
	glBegin(GL_LINES);
	glVertex2i(x1, y1);
	glVertex2i(x2, y2);
	glEnd();
	glEnable(GL_TEXTURE_2D);
}

/**
**  Delivers bitmask denoting given point is left/right/above/below
**      clip rectangle, used for faster determinination of clipped position.
**
**  @param x  pixel's x position (not restricted to screen width)
**  @param y  pixel's y position (not restricted to screen height)
*/
local ClipCode ClipCodeLine(int x, int y)
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
**  Denotes entire line located at the same side outside clip rectangle
**      (point 1 and 2 are both as left/right/above/below the clip rectangle)
**
**  @param code1  ClipCode of one point of line
**  @param code2  ClipCode of second point of line
*/
local ClipCode LineIsUnclippedOnSameSide(int code1, int code2)
{
	return code1 & code2;
}

/**
**  Denotes part of (or entire) line located outside clip rectangle
**      (point 1 and/or 2 is outside clip rectangle)
**
**  @param code1  ClipCode of one point of line
**  @param code2  ClipCode of second point of line
*/
local ClipCode LineIsUnclipped(int code1, int code2)
{
	return code1 | code2;
}

/**
**  Draw line clipped.
**      Based on Sutherland-Cohen clipping technique
**      (Replaces Liang/Barksy clipping algorithm in CVS version 1.18, which
**       might be faster, but that one contained some BUGs)
**
**  @param color  color
**  @param x1     Source x coordinate on the screen
**  @param y1     Source y coordinate on the screen
**  @param x2     Destination x coordinate on the screen
**  @param y2     Destination y coordinate on the screen
*/
global void VideoDrawLineClip(Uint32 color, int x1, int y1, int x2, int y2)
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

	// Draw line based on clipped coordinates
	// FIXME: As the clipped coordinates are rounded to integers, the line's
	//        direction vector might be slightly off. Somehow, the sub-pixel
	//        position(s) on the clipped retangle should be denoted to the line
	//        drawing routine..
	DebugCheck(x1 < ClipX1 || x2 < ClipX1 || x1 > ClipX2 || x2 > ClipX2 ||
		y1 < ClipY1 || y2 < ClipY1 || y1 > ClipY2 || y2 > ClipY2);
	VideoDrawLine(color, x1, y1, x2, y2);
}

/**
**  Draw rectangle.
**
**  @param color  color
**  @param x      x coordinate on the screen
**  @param y      y coordinate on the screen
**  @param h      height of rectangle (0=don't draw).
**  @param w      width of rectangle (0=don't draw).
*/
global void VideoDrawRectangle(Uint32 color, int x, int y, int w, int h)
{
	GLubyte r, g, b, a;

	VideoGetRGBA(color, &r, &g, &b, &a);
	glDisable(GL_TEXTURE_2D);
	glColor4ub(r, g, b, a);
	glBegin(GL_LINE_LOOP);
	glVertex2i(x, y);
	glVertex2i(x + w, y);
	glVertex2i(x + w, y + h);
	glVertex2i(x, y + h);
	glEnd();
	glEnable(GL_TEXTURE_2D);
}

/**
**  Draw rectangle clipped.
**
**  @param color  color
**  @param x      x coordinate on the screen
**  @param y      y coordinate on the screen
**  @param h      height of rectangle (0=don't draw).
**  @param w      width of rectangle (0=don't draw).
*/
global void VideoDrawRectangleClip(Uint32 color, int x, int y,
    int w, int h)
{
	#define _x              x
	#define _y              y
	#define _w              w
	#define _h              h
	#define _hline(x, y,w)   VideoDrawHLine(color, x, y, w)
	#define _vline(x, y,h)   VideoDrawVLine(color, x, y, h)

	#include "_clip_rectangle"

	#undef _x
	#undef _y
	#undef _w
	#undef _h
	#undef _hline
	#undef _vline
}

/**
**  Draw translucent rectangle.
**
**  @param color  color
**  @param x      x coordinate on the screen
**  @param y      y coordinate on the screen
**  @param h      height of rectangle (0=don't draw).
**  @param w      width of rectangle (0=don't draw).
**  @param alpha  alpha value of pixel.
*/
global void VideoDrawTransRectangle(Uint32 color, int x, int y,
	int w, int h, unsigned char alpha)
{
	GLubyte r, g, b;

	VideoGetRGB(color, &r, &g, &b);
	glDisable(GL_TEXTURE_2D);
	glColor4ub(r, g, b, 255 - alpha);
	glBegin(GL_LINE_LOOP);
	glVertex2i(x, y);
	glVertex2i(x + w, y);
	glVertex2i(x + w, y + h);
	glVertex2i(x, y + h);
	glEnd();
	glEnable(GL_TEXTURE_2D);
}

/**
**  Fill rectangle.
**
**  @param color  color
**  @param x      x coordinate on the screen
**  @param y      y coordinate on the screen
**  @param h      height of rectangle (0=don't draw).
**  @param w      width of rectangle (0=don't draw).
*/
global void VideoFillRectangle(Uint32 color, int x, int y,
	int w, int h)
{
	GLubyte r, g, b, a;

	VideoGetRGBA(color, &r, &g, &b, &a);
	glDisable(GL_TEXTURE_2D);
	glColor4ub(r, g, b, a);
	glBegin(GL_TRIANGLE_STRIP);
	glVertex2i(x, y);
	glVertex2i(x + w, y);
	glVertex2i(x, y + h);
	glVertex2i(x + w, y + h);
	glEnd();
	glEnable(GL_TEXTURE_2D);
}

/**
**  Fill rectangle clipped.
**
**  @param color  color
**  @param x      x coordinate on the screen
**  @param y      y coordinate on the screen
**  @param h      height of rectangle (0=don't draw).
**  @param w      width of rectangle (0=don't draw).
*/
global void VideoFillRectangleClip(Uint32 color, int x, int y,
    int w, int h)
{
    CLIP_RECTANGLE(x, y, w, h);
    VideoFillRectangle(color, x, y, w, h);
}

/**
**  Draw translucent rectangle.
**
**  @param color  color
**  @param x      x coordinate on the screen
**  @param y      y coordinate on the screen
**  @param h      height of rectangle (0=don't draw).
**  @param w      width of rectangle (0=don't draw).
**  @param alpha  alpha value of pixel.
*/
global void VideoFillTransRectangle(Uint32 color, int x, int y,
	int w, int h, unsigned char alpha)
{
	GLubyte r, g, b;

	VideoGetRGB(color, &r, &g, &b);
	glDisable(GL_TEXTURE_2D);
	glColor4ub(r, g, b, 255 - alpha);
	glBegin(GL_TRIANGLE_STRIP);
	glVertex2i(x, y);
	glVertex2i(x + w, y);
	glVertex2i(x, y + h);
	glVertex2i(x + w, y + h);
	glEnd();
	glEnable(GL_TEXTURE_2D);
}

/**
**  Fill rectangle translucent clipped.
**
**  @param color  color
**  @param x      x coordinate on the screen
**  @param y      y coordinate on the screen
**  @param h      height of rectangle (0=don't draw).
**  @param w      width of rectangle (0=don't draw).
**  @param alpha  alpha value of pixels.
*/
global void VideoFillTransRectangleClip(Uint32 color, int x, int y,
    int w, int h, unsigned char alpha)
{
    CLIP_RECTANGLE(x, y, w, h);
    VideoFillTransRectangle(color, x, y, w, h,alpha);
}

/**
**  Draw circle clipped.
**
**  @param color  color
**  @param x      Center x coordinate on the screen
**  @param y      Center y coordinate on the screen
**  @param r      radius of circle
*/
global void VideoDrawCircleClip(Uint32 color, int x, int y, int r)
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

	// FIXME: could be much improved :)
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
			--cy;
		}
		d_e += 2;
		++cx;
	} while (cx <= cy);
}

/**
**  Fill circle clipped.
**
**  @param color  color
**  @param x      Center x coordinate on the screen
**  @param y      Center y coordinate on the screen
**  @param r      radius of circle
*/
global void VideoFillCircleClip(Uint32 color, int x, int y, int r)
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

	// FIXME: could be much improved :)
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
			--cy;
		}
		d_e += 2;
		++cx;
	} while (cx <= cy);
}

global void VideoFillTransCircle(Uint32 color, int x, int y, 
	int r, unsigned char alpha)
{
	int p;
	int px;
	int py;

	p = 1 - r;
	py = r;

	for (px = 0; px <= py; ++px) {

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

global void VideoFillTransCircleClip(Uint32 color, int x, int y,
	int r, unsigned char alpha)
{
	int p;
	int px;
	int py;

	p = 1 - r;
	py = r;

	for (px = 0; px <= py; ++px) {

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
global void InitLineDraw(void)
{
}

#endif

//@}
