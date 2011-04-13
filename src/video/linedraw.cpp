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
/**@name linedraw.cpp - The general linedraw functions. */
//
//      (c) Copyright 2000-2011 by Lutz Sammer, Stephan Rasenberg,
//                                 Jimmy Salmon, Nehal Mistry and Pali Rohár
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

//@{

/*----------------------------------------------------------------------------
-- Includes
----------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "stratagus.h"
#include "video.h"

#include "intern_video.h"


/*----------------------------------------------------------------------------
-- Declarations
----------------------------------------------------------------------------*/

/**
** Bitmask, denoting a postion left/right/above/below clip rectangle
** (mainly used by VideoDrawLineClip)
*/
#define ClipCodeInside 0 /// Clipping inside rectangle
#define ClipCodeAbove  1 /// Clipping above rectangle
#define ClipCodeBelow  2 /// Clipping below rectangle
#define ClipCodeLeft   4 /// Clipping left rectangle
#define ClipCodeRight  8 /// Clipping right rectangle


namespace linedraw_sdl {

void (*VideoDrawPixel)(Uint32 color, int x, int y);
static void (*VideoDoDrawPixel)(Uint32 color, int x, int y);
void (*VideoDrawTransPixel)(Uint32 color, int x, int y, unsigned char alpha);
static void (*VideoDoDrawTransPixel)(Uint32 color, int x, int y, unsigned char alpha);

/**
**  Draw a 16-bit pixel
*/
static void VideoDoDrawPixel16(Uint32 color, int x, int y)
{
	((Uint16 *)TheScreen->pixels)[x + y * Video.Width] = color;
}

/**
**  Draw a 16-bit pixel
*/
void VideoDrawPixel16(Uint32 color, int x, int y)
{
	Video.LockScreen();
	VideoDoDrawPixel16(color, x, y);
	Video.UnlockScreen();
}

/**
**  Draw a 32-bit pixel
*/
static void VideoDoDrawPixel32(Uint32 color, int x, int y)
{
	((Uint32 *)TheScreen->pixels)[x + y * Video.Width] = color;
}

/**
**  Draw a 32-bit pixel
*/
void VideoDrawPixel32(Uint32 color, int x, int y)
{
	Video.LockScreen();
	VideoDoDrawPixel32(color, x, y);
	Video.UnlockScreen();
}

/**
**  Draw a transparent 16-bit pixel
*/
static void VideoDoDrawTransPixel16(Uint32 color, int x, int y, unsigned char alpha)
{
	Uint16 *p;
	unsigned long dp;

	// Loses precision for speed
	alpha = (255 - alpha) >> 3;

	p = &((Uint16 *)TheScreen->pixels)[x + y * Video.Width];
	color = (((color << 16) | color) & 0x07E0F81F);
	dp = *p;
	dp = ((dp << 16) | dp) & 0x07E0F81F;
	dp = ((((dp - color) * alpha) >> 5) + color) & 0x07E0F81F;
	*p = (Uint16)((dp >> 16) | dp);
}

/**
**  Draw a transparent 16-bit pixel
*/
void VideoDrawTransPixel16(Uint32 color, int x, int y, unsigned char alpha)
{
	Video.LockScreen();
	VideoDoDrawTransPixel16(color, x, y, alpha);
	Video.UnlockScreen();
}

/**
**  Draw a transparent 32-bit pixel
*/
static void VideoDoDrawTransPixel32(Uint32 color, int x, int y, unsigned char alpha)
{
	unsigned long sp2;
	unsigned long dp1;
	unsigned long dp2;
	Uint32 *p;

	alpha = 255 - alpha;

	p = &((Uint32*)TheScreen->pixels)[x + y * Video.Width];

	sp2 = (color & 0xFF00FF00) >> 8;
	color &= 0x00FF00FF;

	dp1 = *p;
	dp2 = (dp1 & 0xFF00FF00) >> 8;
	dp1 &= 0x00FF00FF;

	dp1 = ((((dp1 - color) * alpha) >> 8) + color) & 0x00FF00FF;
	dp2 = ((((dp2 - sp2) * alpha) >> 8) + sp2) & 0x00FF00FF;
	*p = (dp1 | (dp2 << 8));
}

/**
**  Draw a transparent 32-bit pixel
*/
void VideoDrawTransPixel32(Uint32 color, int x, int y, unsigned char alpha)
{
	Video.LockScreen();
	VideoDoDrawTransPixel32(color, x, y, alpha);
	Video.UnlockScreen();
}

/**
**  Draw a clipped pixel
*/
static void VideoDoDrawPixelClip(Uint32 color, int x, int y)
{
	if (x >= ClipX1 && y >= ClipY1 && x <= ClipX2 && y <= ClipY2) {
		VideoDoDrawPixel(color, x, y);
	}
}

/**
**  Draw a clipped pixel
*/
void DrawPixelClip(Uint32 color, int x, int y)
{
	Video.LockScreen();
	VideoDoDrawPixelClip(color, x, y);
	Video.UnlockScreen();
}

/**
**  Draw a transparent clipped pixel
*/
static void VideoDoDrawTransPixelClip(Uint32 color, int x, int y, unsigned char alpha)
{
	if (x >= ClipX1 && y >= ClipY1 && x <= ClipX2 && y <= ClipY2) {
		VideoDoDrawTransPixel(color, x, y, alpha);
	}
}

/**
**  Draw a transparent clipped pixel
*/
void DrawTransPixelClip(Uint32 color, int x, int y, unsigned char alpha)
{
	Video.LockScreen();
	VideoDoDrawTransPixelClip(color, x, y, alpha);
	Video.UnlockScreen();
}

/**
**  Draw a vertical line
*/
void DrawVLine(Uint32 color, int x, int y, int height)
{
	Video.LockScreen();
	for (int i = 0; i < height; ++i) {
		VideoDoDrawPixel(color, x, y + i);
	}
	Video.UnlockScreen();
}

/**
**  Draw a transparent vertical line
*/
void DrawTransVLine(Uint32 color, int x, int y,
	int height, unsigned char alpha)
{
	Video.LockScreen();
	for (int i = 0; i < height; ++i) {
		VideoDoDrawTransPixel(color, x, y + i, alpha);
	}
	Video.UnlockScreen();
}

/**
**  Draw a vertical line clipped
*/
void DrawVLineClip(Uint32 color, int x, int y, int height)
{
	int w = 1;
	CLIP_RECTANGLE(x, y, w, height);
	DrawVLine(color, x, y, height);
}

/**
**  Draw a transparent vertical line clipped
*/
void DrawTransVLineClip(Uint32 color, int x, int y,
	int height, unsigned char alpha)
{
	Video.LockScreen();
	for (int i = 0; i < height; ++i) {
		VideoDoDrawTransPixelClip(color, x, y + i, alpha);
	}
	Video.UnlockScreen();
}

/**
**  Draw a horizontal line
*/
void DrawHLine(Uint32 color, int x, int y, int width)
{
	Video.LockScreen();
	for (int i = 0; i < width; ++i) {
		VideoDoDrawPixel(color, x + i, y);
	}
	Video.UnlockScreen();
}

/**
**  Draw a horizontal line clipped
*/
void DrawHLineClip(Uint32 color, int x, int y, int width)
{
	int h = 1;
	CLIP_RECTANGLE(x, y, width, h);
	DrawHLine(color, x, y, width);
}

/**
**  Draw a transparent horizontal line
*/
void DrawTransHLine(Uint32 color, int x, int y,
	int width, unsigned char alpha)
{
	Video.LockScreen();
	for (int i = 0; i < width; ++i) {
		VideoDoDrawTransPixel(color, x + i, y, alpha);
	}
	Video.UnlockScreen();
}

/**
**  Draw a transparent horizontal line clipped
*/
void DrawTransHLineClip(Uint32 color, int x, int y,
	int width, unsigned char alpha)
{
	Video.LockScreen();
	for (int i = 0; i < width; ++i) {
		VideoDoDrawTransPixelClip(color, x + i, y, alpha);
	}
	Video.UnlockScreen();
}

/**
**  Draw a line
*/
void DrawLine(Uint32 color, int sx, int sy, int dx, int dy)
{
	int x;
	int y;
	int xlen;
	int ylen;
	int incr;

	if (sx == dx) {
		if (sy < dy) {
			DrawVLine(color, sx, sy, dy - sy + 1);
		} else {
			DrawVLine(color, dx, dy, sy - dy + 1);
		}
		return;
	}

	if (sy == dy) {
		if (sx < dx) {
			DrawHLine(color, sx, sy, dx - sx + 1);
		} else {
			DrawHLine(color, dx, dy, sx - dx + 1);
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

		Video.LockScreen();
		for (x = sx; x < dx; ++x) {
			VideoDoDrawPixel(color, x, y);
			if (p >= 0) {
				y += incr;
				p += (ylen - xlen) << 1;
			} else {
				p += (ylen << 1);
			}
		}
		Video.UnlockScreen();
		return;
	}

	if (ylen > xlen) {
		int p;

		p = (xlen << 1) - ylen;

		Video.LockScreen();
		for (y = sy; y < dy; ++y) {
			VideoDoDrawPixel(color, x, y);
			if (p >= 0) {
				x += incr;
				p += (xlen - ylen) << 1;
			} else {
				p += (xlen << 1);
			}
		}
		Video.UnlockScreen();
		return;
	}

	// Draw a diagonal line
	if (ylen == xlen) {
		Video.LockScreen();
		while (y != dy) {
			VideoDoDrawPixel(color, x, y);
			x += incr;
			++y;
		}
		Video.UnlockScreen();
	}
}

/**
**  Draw a line clipped
*/
void DrawLineClip(Uint32 color, int sx, int sy, int dx, int dy)
{
	int x;
	int y;
	int xlen;
	int ylen;
	int incr;

	if (sx == dx) {
		if (sy < dy) {
			DrawVLineClip(color, sx, sy, dy - sy + 1);
		} else {
			DrawVLineClip(color, dx, dy, sy - dy + 1);
		}
		return;
	}

	if (sy == dy) {
		if (sx < dx) {
			DrawHLineClip(color, sx, sy, dx - sx + 1);
		} else {
			DrawHLineClip(color, dx, dy, sx - dx + 1);
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

		Video.LockScreen();
		for (x = sx; x < dx; ++x) {
			VideoDoDrawPixelClip(color, x, y);
			if (p >= 0) {
				y += incr;
				p += (ylen - xlen) << 1;
			} else {
				p += (ylen << 1);
			}
		}
		Video.UnlockScreen();
		return;
	}

	if (ylen > xlen) {
		int p;

		p = (xlen << 1) - ylen;

		Video.LockScreen();
		for (y = sy; y < dy; ++y) {
			VideoDoDrawPixelClip(color, x, y);
			if (p >= 0) {
				x += incr;
				p += (xlen - ylen) << 1;
			} else {
				p += (xlen << 1);
			}
		}
		Video.UnlockScreen();
		return;
	}

	// Draw a diagonal line
	if (ylen == xlen) {
		Video.LockScreen();
		while (y != dy) {
			VideoDoDrawPixelClip(color, x, y);
			x += incr;
			++y;
		}
		Video.UnlockScreen();
	}
}

/**
**  Draw a transparent line
*/
void DrawTransLine(Uint32 color, int sx, int sy,
	int dx, int dy, unsigned char)
{
	// FIXME: trans
	DrawLine(color, sx, sy, dx, dy);
}

/**
**  Draw a transparent line clipped
*/
void DrawTransLineClip(Uint32 color, int sx, int sy,
	int dx, int dy, unsigned char)
{
	// FIXME: trans
	DrawLineClip(color, sx, sy, dx, dy);
}

/**
**  Draw a rectangle
*/
void DrawRectangle(Uint32 color, int x, int y, int w, int h)
{
	DrawHLine(color, x, y, w);
	DrawHLine(color, x, y + h - 1, w);

	DrawVLine(color, x, y + 1, h - 2);
	DrawVLine(color, x + w - 1, y + 1, h - 2);
}

/**
**  Draw a rectangle clipped
*/
void DrawRectangleClip(Uint32 color, int x, int y, int w, int h)
{
	DrawHLineClip(color, x, y, w);
	DrawHLineClip(color, x, y + h - 1, w);

	DrawVLineClip(color, x, y + 1, h - 2);
	DrawVLineClip(color, x + w - 1, y + 1, h - 2);
}

/**
**  Draw a transparent rectangle
*/
void DrawTransRectangle(Uint32 color, int x, int y,
	int w, int h, unsigned char alpha)
{
	DrawTransHLine(color, x, y, w, alpha);
	DrawTransHLine(color, x, y + h - 1, w, alpha);

	DrawTransVLine(color, x, y + 1, h - 2, alpha);
	DrawTransVLine(color, x + w - 1, y + 1, h - 2, alpha);
}

/**
**  Draw a transparent rectangle clipped.
**
**  @param color  color
**  @param x      x coordinate on the screen
**  @param y      y coordinate on the screen
**  @param h      height of rectangle (0=don't draw).
**  @param w      width of rectangle (0=don't draw).
**  @param alpha  alpha value of pixels.
*/
void DrawTransRectangleClip(Uint32 color, int x, int y,
	int w, int h, unsigned char alpha)
{
	DrawTransHLineClip(color, x, y, w, alpha);
	DrawTransHLineClip(color, x, y + h - 1, w, alpha);

	DrawTransVLineClip(color, x, y + 1, h - 2, alpha);
	DrawTransVLineClip(color, x + w - 1, y + 1, h - 2, alpha);
}

/**
**  Draw a filled rectangle
*/
void FillRectangle(Uint32 color, int x, int y, int w, int h)
{
	SDL_Rect drect = {x, y, w, h};
	SDL_FillRect(TheScreen, &drect, color);
}

/**
**  Draw a filled rectangle clipped
*/
void FillRectangleClip(Uint32 color, int x, int y,
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
	FillRectangle(color, x, y, w, h);
	SDL_SetClipRect(TheScreen, &oldrect);
}

/**
**  Draw a filled transparent rectangle
*/
void FillTransRectangle(Uint32 color, int x, int y,
	int w, int h, unsigned char alpha)
{
	int ex = x + w;
	int ey = y + h;
	int sx = x;

	Video.LockScreen();
	for (; y < ey; ++y) {
		for (x = sx; x < ex; ++x) {
			VideoDoDrawTransPixel(color, x, y, alpha);
		}
	}
	Video.UnlockScreen();
}

/**
**  Draw a filled transparent rectangle clipped
*/
void FillTransRectangleClip(Uint32 color, int x, int y,
	int w, int h, unsigned char alpha)
{
	CLIP_RECTANGLE(x, y, w, h);
	FillTransRectangle(color, x, y, w, h, alpha);
}

/**
**  Draw a circle
*/
void DrawCircle(Uint32 color, int x, int y, int r)
{
	int p;
	int px;
	int py;

	p = 1 - r;
	py = r;

	Video.LockScreen();
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
	Video.UnlockScreen();
}

/**
**  Draw a transparent circle
*/
void DrawTransCircle(Uint32 color, int x, int y,
	int r, unsigned char alpha)
{
	int p;
	int px;
	int py;

	p = 1 - r;
	py = r;

	Video.LockScreen();
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
	Video.UnlockScreen();
}

/**
**  Draw a circle clipped
*/
void DrawCircleClip(Uint32 color, int x, int y, int r)
{
	int p;
	int px;
	int py;

	p = 1 - r;
	py = r;

	Video.LockScreen();
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
	Video.UnlockScreen();
}

/**
**  Draw a transparent circle clipped
*/
void DrawTransCircleClip(Uint32 color, int x, int y,
	int r, unsigned char alpha)
{
	int p;
	int px;
	int py;

	p = 1 - r;
	py = r;

	Video.LockScreen();
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
	Video.UnlockScreen();
}

/**
**  Draw a filled circle
*/
void FillCircle(Uint32 color, int x, int y, int r)
{
	int p;
	int px;
	int py;

	p = 1 - r;
	py = r;

	for (px = 0; px <= py; ++px) {

		// Fill up the middle half of the circle
		DrawVLine(color, x + px, y, py + 1);
		DrawVLine(color, x + px, y - py, py);
		if (px) {
			DrawVLine(color, x - px, y, py + 1);
			DrawVLine(color, x - px, y - py, py);
		}

		if (p < 0) {
			p += 2 * px + 3;
		} else {
			p += 2 * (px - py) + 5;
			py -= 1;
			// Fill up the left/right half of the circle
			if (py >= px) {
				DrawVLine(color, x + py + 1, y, px + 1);
				DrawVLine(color, x + py + 1, y - px, px);
				DrawVLine(color, x - py - 1, y, px + 1);
				DrawVLine(color, x - py - 1, y - px,  px);
			}
		}
	}
}

/**
**  Draw a filled transparent circle
*/
void FillTransCircle(Uint32 color, int x, int y,
	int r, unsigned char alpha)
{
	int p;
	int px;
	int py;

	p = 1 - r;
	py = r;

	for (px = 0; px <= py; ++px) {

		// Fill up the middle half of the circle
		DrawTransVLine(color, x + px, y, py + 1, alpha);
		DrawTransVLine(color, x + px, y - py, py, alpha);
		if (px) {
			DrawTransVLine(color, x - px, y, py + 1, alpha);
			DrawTransVLine(color, x - px, y - py, py, alpha);
		}

		if (p < 0) {
			p += 2 * px + 3;
		} else {
			p += 2 * (px - py) + 5;
			py -= 1;
			// Fill up the left/right half of the circle
			if (py >= px) {
				DrawTransVLine(color, x + py + 1, y, px + 1, alpha);
				DrawTransVLine(color, x + py + 1, y - px, px, alpha);
				DrawTransVLine(color, x - py - 1, y, px + 1, alpha);
				DrawTransVLine(color, x - py - 1, y - px,  px, alpha);
			}
		}
	}
}

/**
**  Draw a filled circle clipped
*/
void FillCircleClip(Uint32 color, int x, int y, int r)
{
	int p;
	int px;
	int py;

	p = 1 - r;
	py = r;

	for (px = 0; px <= py; ++px) {

		// Fill up the middle half of the circle
		DrawVLineClip(color, x + px, y, py + 1);
		DrawVLineClip(color, x + px, y - py, py);
		if (px) {
			DrawVLineClip(color, x - px, y, py + 1);
			DrawVLineClip(color, x - px, y - py, py);
		}

		if (p < 0) {
			p += 2 * px + 3;
		} else {
			p += 2 * (px - py) + 5;
			py -= 1;
			// Fill up the left/right half of the circle
			if (py >= px) {
				DrawVLineClip(color, x + py + 1, y, px + 1);
				DrawVLineClip(color, x + py + 1, y - px, px);
				DrawVLineClip(color, x - py - 1, y, px + 1);
				DrawVLineClip(color, x - py - 1, y - px,  px);
			}
		}
	}
}

/**
**  Draw a filled transparent circle clipped
*/
void FillTransCircleClip(Uint32 color, int x, int y,
	int r, unsigned char alpha)
{
	int p;
	int px;
	int py;

	p = 1 - r;
	py = r;

	for (px = 0; px <= py; ++px) {

		// Fill up the middle half of the circle
		DrawTransVLineClip(color, x + px, y, py + 1, alpha);
		DrawTransVLineClip(color, x + px, y - py, py, alpha);
		if (px) {
			DrawTransVLineClip(color, x - px, y, py + 1, alpha);
			DrawTransVLineClip(color, x - px, y - py, py, alpha);
		}

		if (p < 0) {
			p += 2 * px + 3;
		} else {
			p += 2 * (px - py) + 5;
			py -= 1;
			// Fill up the left/right half of the circle
			if (py >= px) {
				DrawTransVLineClip(color, x + py + 1, y, px + 1, alpha);
				DrawTransVLineClip(color, x + py + 1, y - px, px, alpha);
				DrawTransVLineClip(color, x - py - 1, y, px + 1, alpha);
				DrawTransVLineClip(color, x - py - 1, y - px,  px, alpha);
			}
		}
	}
}

/**
**  Initialize line draw
*/
void InitLineDraw()
{
	switch (Video.Depth) {
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

}
namespace linedraw_gl {

/**
**  Draw pixel unclipped.
**
**  @param color  color
**  @param x      x coordinate on the screen
**  @param y      y coordinate on the screen
*/
void DrawPixel(Uint32 color, int x, int y)
{
	GLubyte r, g, b, a;

	Video.GetRGBA(color, NULL, &r, &g, &b, &a);
	glDisable(GL_TEXTURE_2D);
	glColor4ub(r, g, b, a);
#ifdef USE_GLES
	float vertex[] = {
		2.0f/(GLfloat)Video.Width*x-1.0f, -2.0f/(GLfloat)Video.Height*y+1.0f
	};

	glEnableClientState(GL_VERTEX_ARRAY);

	glVertexPointer(2, GL_FLOAT, 0, vertex);
	glDrawArrays(GL_POINTS, 0, 1);

	glDisableClientState(GL_VERTEX_ARRAY);
#else
	glBegin(GL_POINTS);
	glVertex2i(x, y);
	glEnd();
#endif
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
void DrawTransPixel(Uint32 color, int x, int y,
	unsigned char alpha)
{
	GLubyte r, g, b;

	Video.GetRGB(color, NULL, &r, &g, &b);
	color = Video.MapRGBA(0, r, g, b, alpha);
	DrawPixel(color, x, y);
}

/**
**  Draw pixel clipped to current clip setting.
**
**  @param color  color
**  @param x      x coordinate on the screen
**  @param y      y coordinate on the screen
*/
void DrawPixelClip(Uint32 color, int x, int y)
{
	if (x < ClipX1 || x > ClipX2 || y < ClipY1 || y > ClipY2) {
		return;
	}
	DrawPixel(color, x, y);
}

/**
**  Draw translucent pixel clipped to current clip setting.
**
**  @param color  color
**  @param x      x coordinate on the screen
**  @param y      y coordinate on the screen
**  @param alpha  alpha value of pixel.
*/
void DrawTransPixelClip(Uint32 color, int x, int y,
	unsigned char alpha)
{
	GLubyte r, g, b;

	Video.GetRGB(color, NULL, &r, &g, &b);
	color = Video.MapRGBA(0, r, g, b, alpha);
	DrawPixelClip(color, x, y);
}

/**
**  Draw horizontal line unclipped.
**
**  @param color  color
**  @param x      x coordinate on the screen
**  @param y      y coordinate on the screen
**  @param width  width of line (0=don't draw).
*/
void DrawHLine(Uint32 color, int x, int y, int width)
{
	GLubyte r, g, b, a;

	Video.GetRGBA(color, NULL, &r, &g, &b, &a);
	glDisable(GL_TEXTURE_2D);
	glColor4ub(r, g, b, a);
#ifdef USE_GLES
	float vertex[] = {
		2.0f/(GLfloat)Video.Width*x-1.0f, -2.0f/(GLfloat)Video.Height*y+1.0f,
		2.0f/(GLfloat)Video.Width*(x+width)-1.0f, -2.0f/(GLfloat)Video.Height*y+1.0f
	};

	glEnableClientState(GL_VERTEX_ARRAY);

	glVertexPointer(2, GL_FLOAT, 0, vertex);
	glDrawArrays(GL_LINES, 0, 2);

	glDisableClientState(GL_VERTEX_ARRAY);
#else
	glBegin(GL_LINES);
	glVertex2i(x, y);
	glVertex2i(x + width, y);
	glEnd();
#endif
	glEnable(GL_TEXTURE_2D);
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
void DrawTransHLine(Uint32 color, int x, int y, int width,
	unsigned char alpha)
{
	GLubyte r, g, b;

	Video.GetRGB(color, NULL, &r, &g, &b);
	color = Video.MapRGBA(0, r, g, b, alpha);
	DrawHLine(color, x, y, width);
}

/**
**  Draw horizontal line clipped.
**
**  @param color  color
**  @param x      x coordinate on the screen
**  @param y      y coordinate on the screen
**  @param width  width of line (0=don't draw).
*/
void DrawHLineClip(Uint32 color, int x, int y, int width)
{
	if (y < ClipY1 || y > ClipY2) {
		return;
	}
	if (x < ClipX1) {
		int f = ClipX1 - x;
		if (width <= f) {
			return;
		}
		width -= f;
		x = ClipX1;
	}
	if ((x + width) > ClipX2 + 1) {
		if (x > ClipX2) {
			return;
		}
		width = ClipX2 - x + 1;
	}
	DrawHLine(color, x, y, width);
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
void DrawTransHLineClip(Uint32 color, int x, int y, int width,
	unsigned char alpha)
{
	GLubyte r, g, b;

	Video.GetRGB(color, NULL, &r, &g, &b);
	color = Video.MapRGBA(0, r, g, b, alpha);
	DrawHLineClip(color, x, y, width);
}

/**
**  Draw vertical line unclipped.
**
**  @param color   color
**  @param x       x coordinate on the screen
**  @param y       y coordinate on the screen
**  @param height  height of line (0=don't draw).
*/
void DrawVLine(Uint32 color, int x, int y, int height)
{
	GLubyte r, g, b, a;

	Video.GetRGBA(color, NULL, &r, &g, &b, &a);
	glDisable(GL_TEXTURE_2D);
	glColor4ub(r, g, b, a);
#ifdef USE_GLES
	float vertex[] = {
		2.0f/(GLfloat)Video.Width*x-1.0f, -2.0f/(GLfloat)Video.Height*y+1.0f,
		2.0f/(GLfloat)Video.Width*x-1.0f, -2.0f/(GLfloat)Video.Height*(y+height)+1.0f
	};

	glEnableClientState(GL_VERTEX_ARRAY);

	glVertexPointer(2, GL_FLOAT, 0, vertex);
	glDrawArrays(GL_LINES, 0, 2);

	glDisableClientState(GL_VERTEX_ARRAY);
#else
	glBegin(GL_LINES);
	glVertex2i(x, y);
	glVertex2i(x, y + height);
	glEnd();
#endif
	glEnable(GL_TEXTURE_2D);
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
void DrawTransVLine(Uint32 color, int x, int y, int height,
	unsigned char alpha)
{
	GLubyte r, g, b;

	Video.GetRGB(color, NULL, &r, &g, &b);
	color = Video.MapRGBA(0, r, g, b, alpha);
	DrawVLine(color, x, y, height);
}

/**
**  Draw vertical line clipped.
**
**  @param color   color
**  @param x       x coordinate on the screen
**  @param y       y coordinate on the screen
**  @param height  height of line (0=don't draw).
*/
void DrawVLineClip(Uint32 color, int x, int y, int height)
{
	if (x < ClipX1 || x > ClipX2) {
		return;
	}
	if (y < ClipY1) {
		int f = ClipY1 - y;
		if (height <= f) {
			return;
		}
		height -= f;
		y = ClipY1;
	}
	if ((y + height) > ClipY2 + 1) {
		if (y > ClipY2) {
			return;
		}
		height = ClipY2 - y + 1;
	}
	DrawVLine(color, x, y, height);
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
void DrawTransVLineClip(Uint32 color, int x, int y,
	int height, unsigned char alpha)
{
	GLubyte r, g, b;

	Video.GetRGB(color, NULL, &r, &g, &b);
	color = Video.MapRGBA(0, r, g, b, alpha);
	DrawVLineClip(color, x, y, height);
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
void DrawLine(Uint32 color, int x1, int y1, int x2, int y2)
{
	float xx1, yy1, xx2, yy2;
	GLubyte r, g, b, a;

	xx1 = (float)x1;
	xx2 = (float)x2;
	yy1 = (float)y1;
	yy2 = (float)y2;

	if (xx1 <= xx2) {
		xx2 += 0.5f;
	} else {
		xx1 += 0.5f;
	}
	if (yy1 <= yy2) {
		yy2 += 0.5f;
	} else {
		yy1 += 0.5f;
	}

	Video.GetRGBA(color, NULL, &r, &g, &b, &a);
	glDisable(GL_TEXTURE_2D);
	glColor4ub(r, g, b, a);
#ifdef USE_GLES
	float vertex[] = {
		2.0f/(GLfloat)Video.Width*xx1-1.0f, -2.0f/(GLfloat)Video.Height*yy1+1.0f,
		2.0f/(GLfloat)Video.Width*xx2-1.0f, -2.0f/(GLfloat)Video.Height*yy2+1.0f
	};

	glEnableClientState(GL_VERTEX_ARRAY);

	glVertexPointer(2, GL_FLOAT, 0, vertex);
	glDrawArrays(GL_LINES, 0, 2);

	glDisableClientState(GL_VERTEX_ARRAY);
#else
	glBegin(GL_LINES);
	glVertex2f(xx1, yy1);
	glVertex2f(xx2, yy2);
	glEnd();
#endif
	glEnable(GL_TEXTURE_2D);
}

/**
**  Delivers bitmask denoting given point is left/right/above/below
**      clip rectangle, used for faster determinination of clipped position.
**
**  @param x  pixel's x position (not restricted to screen width)
**  @param y  pixel's y position (not restricted to screen height)
*/
static int ClipCodeLine(int x, int y)
{
	int result;

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
static int LineIsUnclippedOnSameSide(int code1, int code2)
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
static int LineIsUnclipped(int code1, int code2)
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
void DrawLineClip(Uint32 color, int x1, int y1, int x2, int y2)
{
	int code1;
	int code2;
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
	Assert(x1 >= ClipX1 && x2 >= ClipX1 && x1 <= ClipX2 && x2 <= ClipX2 &&
		y1 >= ClipY1 && y2 >= ClipY1 && y1 <= ClipY2 && y2 <= ClipY2);
	DrawLine(color, x1, y1, x2, y2);
}

/**
**  Draw a transparent line
*/
void DrawTransLine(Uint32 color, int sx, int sy,
	int dx, int dy, unsigned char)
{
	// FIXME: trans
	DrawLine(color, sx, sy, dx, dy);
}

/**
**  Draw a transparent line clipped
*/
void DrawTransLineClip(Uint32 color, int sx, int sy,
	int dx, int dy, unsigned char)
{
	// FIXME: trans
	DrawLineClip(color, sx, sy, dx, dy);
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
void DrawRectangle(Uint32 color, int x, int y, int w, int h)
{
	GLubyte r, g, b, a;

	Video.GetRGBA(color, NULL, &r, &g, &b, &a);
	glDisable(GL_TEXTURE_2D);
	glColor4ub(r, g, b, a);
#ifdef USE_GLES
	float vertex[] = {
		2.0f/(GLfloat)Video.Width*x-1.0f, -2.0f/(GLfloat)Video.Height*y+1.0f,
		2.0f/(GLfloat)Video.Width*(x+w)-1.0f, -2.0f/(GLfloat)Video.Height*y+1.0f,
		2.0f/(GLfloat)Video.Width*(x+w-1)-1.0f, -2.0f/(GLfloat)Video.Height*(y+1)+1.0f,
		2.0f/(GLfloat)Video.Width*(x+w-1)-1.0f, -2.0f/(GLfloat)Video.Height*(y+h)+1.0f,
		2.0f/(GLfloat)Video.Width*(x+w-1)-1.0f, -2.0f/(GLfloat)Video.Height*(y+h-1)+1.0f,
		2.0f/(GLfloat)Video.Width*x-1.0f, -2.0f/(GLfloat)Video.Height*(y+h-1)+1.0f,
		2.0f/(GLfloat)Video.Width*x-1.0f, -2.0f/(GLfloat)Video.Height*(y+h-1)+1.0f,
		2.0f/(GLfloat)Video.Width*x-1.0f, -2.0f/(GLfloat)Video.Height*(y+1)+1.0f
	};

	glEnableClientState(GL_VERTEX_ARRAY);

	glVertexPointer(2, GL_FLOAT, 0, vertex);
	glDrawArrays(GL_LINES, 0, 8);

	glDisableClientState(GL_VERTEX_ARRAY);
#else
	glBegin(GL_LINES);
	glVertex2i(x, y);
	glVertex2i(x + w, y);

	glVertex2i(x + w - 1, y + 1);
	glVertex2i(x + w - 1, y + h);

	glVertex2i(x + w - 1, y + h - 1);
	glVertex2i(x, y + h - 1);

	glVertex2i(x, y + h - 1);
	glVertex2i(x, y + 1);
	glEnd();
#endif
	glEnable(GL_TEXTURE_2D);
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
void DrawTransRectangle(Uint32 color, int x, int y,
	int w, int h, unsigned char alpha)
{
	GLubyte r, g, b;

	Video.GetRGB(color, NULL, &r, &g, &b);
	color = Video.MapRGBA(0, r, g, b, alpha);
	DrawRectangle(color, x, y, w, h);
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
void DrawRectangleClip(Uint32 color, int x, int y,
	int w, int h)
{
	int f;
	int left;
	int right;
	int top;
	int bottom;

	// Ensure non-empty rectangle
	if (!w || !h) {
		// rectangle is `void'
		return;
	}

	// Clip rectangle boundary
	left = right = top = bottom = 1;

	if (x < ClipX1) {            // no left side
		f = ClipX1 - x;
		if (w <= f) {
			return;                    // entire rectangle left --> not visible
		}
		w -= f;
		x = ClipX1;
		left = 0;
	}
	if ((x + w) > ClipX2 + 1) {     // no right side
		if (x > ClipX2) {
			return;                    // entire rectangle right --> not visible
		}
		w = ClipX2 - x + 1;
		right = 0;
	}
	if (y < ClipY1) {               // no top
		f = ClipY1 - y;
		if (h <= f) {
			return;                    // entire rectangle above --> not visible
		}
		h -= f;
		y = ClipY1;
		top = 0;
	}
	if ((y + h) > ClipY2 + 1) {    // no bottom
		if (y > ClipY2) {
			return;                  // entire rectangle below --> not visible
		}
		h = ClipY2 - y + 1;
		bottom = 0;
	}

	// Draw (part of) rectangle sides
	// Note: _hline and _vline should be able to handle zero width/height
	if (top) {
		DrawHLine(color, x, y, w);
		if (!--h) {
			return;                    // rectangle as horizontal line
		}
		++y;
	}
	if (bottom) {
		DrawHLine(color, x, y + h - 1, w);
		--h;
	}
	if (left) {
		DrawVLine(color, x, y, h);
		if (!--w) {
			return;                    // rectangle as vertical line
		}
		++x;
	}
	if (right) {
		DrawVLine(color, x + w - 1, y, h);
	}
}

/**
**  Draw translucent rectangle clipped.
**
**  @param color  color
**  @param x      x coordinate on the screen
**  @param y      y coordinate on the screen
**  @param h      height of rectangle (0=don't draw).
**  @param w      width of rectangle (0=don't draw).
**  @param alpha  alpha value of pixels.
*/
void DrawTransRectangleClip(Uint32 color, int x, int y,
	int w, int h, unsigned char alpha)
{
	GLubyte r, g, b;

	Video.GetRGB(color, NULL, &r, &g, &b);
	color = Video.MapRGBA(0, r, g, b, alpha);
	DrawRectangleClip(color, x, y, w, h);
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
void FillRectangle(Uint32 color, int x, int y,
	int w, int h)
{
	GLubyte r, g, b, a;

	Video.GetRGBA(color, NULL, &r, &g, &b, &a);
	glDisable(GL_TEXTURE_2D);
	glColor4ub(r, g, b, a);
#ifdef USE_GLES
	float vertex[] = {
		2.0f/(GLfloat)Video.Width*x-1.0f, -2.0f/(GLfloat)Video.Height*y+1.0f,
		2.0f/(GLfloat)Video.Width*(x+w)-1.0f, -2.0f/(GLfloat)Video.Height*y+1.0f,
		2.0f/(GLfloat)Video.Width*x-1.0f, -2.0f/(GLfloat)Video.Height*(y+h)+1.0f,
		2.0f/(GLfloat)Video.Width*(x+w)-1.0f, -2.0f/(GLfloat)Video.Height*(y+h)+1.0f
	};

	glEnableClientState(GL_VERTEX_ARRAY);

	glVertexPointer(2, GL_FLOAT, 0, vertex);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	glDisableClientState(GL_VERTEX_ARRAY);
#else
	glBegin(GL_TRIANGLE_STRIP);
	glVertex2i(x, y);
	glVertex2i(x + w, y);
	glVertex2i(x, y + h);
	glVertex2i(x + w, y + h);
	glEnd();
#endif
	glEnable(GL_TEXTURE_2D);
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
void FillTransRectangle(Uint32 color, int x, int y,
	int w, int h, unsigned char alpha)
{
	GLubyte r, g, b;

	Video.GetRGB(color, NULL, &r, &g, &b);
	color = Video.MapRGBA(0, r, g, b, alpha);
	FillRectangle(color, x, y, w, h);
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
void FillRectangleClip(Uint32 color, int x, int y,
	int w, int h)
{
	CLIP_RECTANGLE(x, y, w, h);
	FillRectangle(color, x, y, w, h);
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
void FillTransRectangleClip(Uint32 color, int x, int y,
	int w, int h, unsigned char alpha)
{
	GLubyte r, g, b;

	Video.GetRGB(color, NULL, &r, &g, &b);
	color = Video.MapRGBA(0, r, g, b, alpha);
	FillRectangleClip(color, x, y, w, h);
}

/**
**  Draw circle.
**
**  @param color   color
**  @param x       Center x coordinate on the screen
**  @param y       Center y coordinate on the screen
**  @param radius  radius of circle
*/
void DrawCircle(Uint32 color, int x, int y, int radius)
{
	int cx;
	int cy;
	int df;
	int d_e;
	int d_se;

	cx = 0;
	cy = radius;
	df = 1 - radius;
	d_e = 3;
	d_se = -2 * radius + 5;

	// FIXME: could be much improved :)
	do {
		if (cx == 0) {
			DrawPixel(color, x, y + cy);
			DrawPixel(color, x, y - cy);
			DrawPixel(color, x + cy, y);
			DrawPixel(color, x - cy, y);
		} else if (cx == cy) {
			Assert(cx != 0 && cy != 0);
			DrawPixel(color, x + cx, y + cy);
			DrawPixel(color, x - cx, y + cy);
			DrawPixel(color, x + cx, y - cy);
			DrawPixel(color, x - cx, y - cy);
		} else if (cx < cy) {
			Assert(cx != 0 && cy != 0);
			DrawPixel(color, x + cx, y + cy);
			DrawPixel(color, x + cx, y - cy);
			DrawPixel(color, x + cy, y + cx);
			DrawPixel(color, x + cy, y - cx);
			DrawPixel(color, x - cx, y + cy);
			DrawPixel(color, x - cx, y - cy);
			DrawPixel(color, x - cy, y + cx);
			DrawPixel(color, x - cy, y - cx);
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
**  Draw circle clipped.
**
**  @param color   color
**  @param x       Center x coordinate on the screen
**  @param y       Center y coordinate on the screen
**  @param radius  radius of circle
*/
void DrawCircleClip(Uint32 color, int x, int y, int radius)
{
	int cx;
	int cy;
	int df;
	int d_e;
	int d_se;

	cx = 0;
	cy = radius;
	df = 1 - radius;
	d_e = 3;
	d_se = -2 * radius + 5;

	// FIXME: could be much improved :)
	do {
		if (cx == 0) {
			DrawPixelClip(color, x, y + cy);
			DrawPixelClip(color, x, y - cy);
			DrawPixelClip(color, x + cy, y);
			DrawPixelClip(color, x - cy, y);
		} else if (cx == cy) {
			Assert(cx != 0 && cy != 0);
			DrawPixelClip(color, x + cx, y + cy);
			DrawPixelClip(color, x - cx, y + cy);
			DrawPixelClip(color, x + cx, y - cy);
			DrawPixelClip(color, x - cx, y - cy);
		} else if (cx < cy) {
			Assert(cx != 0 && cy != 0);
			DrawPixelClip(color, x + cx, y + cy);
			DrawPixelClip(color, x + cx, y - cy);
			DrawPixelClip(color, x + cy, y + cx);
			DrawPixelClip(color, x + cy, y - cx);
			DrawPixelClip(color, x - cx, y + cy);
			DrawPixelClip(color, x - cx, y - cy);
			DrawPixelClip(color, x - cy, y + cx);
			DrawPixelClip(color, x - cy, y - cx);
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
**  Draw translucent circle.
**
**  @param color   color
**  @param x       Center x coordinate on the screen
**  @param y       Center y coordinate on the screen
**  @param radius  radius of circle
**  @param alpha   alpha value of pixels.
*/
void DrawTransCircle(Uint32 color, int x, int y, int radius,
	unsigned char alpha)
{
	GLubyte r, g, b;

	Video.GetRGB(color, NULL, &r, &g, &b);
	color = Video.MapRGBA(0, r, g, b, alpha);
	DrawCircle(color, x, y, radius);
}

/**
**  Draw translucent circle clipped.
**
**  @param color   color
**  @param x       Center x coordinate on the screen
**  @param y       Center y coordinate on the screen
**  @param radius  radius of circle
**  @param alpha   alpha value of pixels.
*/
void DrawTransCircleClip(Uint32 color, int x, int y, int radius,
	unsigned char alpha)
{
	GLubyte r, g, b;

	Video.GetRGB(color, NULL, &r, &g, &b);
	color = Video.MapRGBA(0, r, g, b, alpha);
	DrawCircleClip(color, x, y, radius);
}

/**
**  Fill circle.
**
**  @param color   color
**  @param x       Center x coordinate on the screen
**  @param y       Center y coordinate on the screen
**  @param radius  radius of circle
*/
void FillCircle(Uint32 color, int x, int y, int radius)
{
	int p;
	int px;
	int py;

	p = 1 - radius;
	py = radius;

	for (px = 0; px <= py; ++px) {
		// Fill up the middle half of the circle
		DrawVLine(color, x + px, y, py + 1);
		DrawVLine(color, x + px, y - py, py);
		if (px) {
			DrawVLine(color, x - px, y, py + 1);
			DrawVLine(color, x - px, y - py, py);
		}

		if (p < 0) {
			p += 2 * px + 3;
		} else {
			p += 2 * (px - py) + 5;
			py -= 1;
			// Fill up the left/right half of the circle
			if (py >= px) {
				DrawVLine(color, x + py + 1, y, px + 1);
				DrawVLine(color, x + py + 1, y - px, px);
				DrawVLine(color, x - py - 1, y, px + 1);
				DrawVLine(color, x - py - 1, y - px,  px);
			}
		}
	}
}

/**
**  Fill translucent circle clipped.
**
**  @param color   color
**  @param x       Center x coordinate on the screen
**  @param y       Center y coordinate on the screen
**  @param radius  radius of circle
**  @param alpha   alpha value of pixels.
*/
void FillTransCircle(Uint32 color, int x, int y,
	int radius, unsigned char alpha)
{
	GLubyte r, g, b;

	Video.GetRGB(color, NULL, &r, &g, &b);
	color = Video.MapRGBA(0, r, g, b, alpha);
	FillCircle(color, x, y, radius);
}

/**
**  Fill circle clipped.
**
**  @param color   color
**  @param x       Center x coordinate on the screen
**  @param y       Center y coordinate on the screen
**  @param radius  radius of circle
*/
void FillCircleClip(Uint32 color, int x, int y, int radius)
{
	int cx;
	int cy;
	int df;
	int d_e;
	int d_se;

	cx = 0;
	cy = radius;
	df = 1 - radius;
	d_e = 3;
	d_se = -2 * radius + 5;

	// FIXME: could be much improved :)
	do {
		DrawHLineClip(color, x - cy, y - cx, 1 + cy * 2);
		if (cx) {
			DrawHLineClip(color, x - cy, y + cx, 1 + cy * 2);
		}
		if (df < 0) {
			df += d_e;
			d_se += 2;
		} else {
			if (cx != cy) {
				DrawHLineClip(color, x - cx, y - cy, 1 + cx * 2);
				DrawHLineClip(color, x - cx, y + cy, 1 + cx * 2);
			}
			df += d_se;
			d_se += 4;
			--cy;
		}
		d_e += 2;
		++cx;
	} while (cx <= cy);
}

/**
**  Fill translucent circle clipped.
**
**  @param color   color
**  @param x       Center x coordinate on the screen
**  @param y       Center y coordinate on the screen
**  @param radius  radius of circle
**  @param alpha   alpha value of pixels.
*/
void FillTransCircleClip(Uint32 color, int x, int y,
	int radius, unsigned char alpha)
{
	GLubyte r, g, b;

	Video.GetRGB(color, NULL, &r, &g, &b);
	color = Video.MapRGBA(0, r, g, b, alpha);
	FillCircleClip(color, x, y, radius);
}

/**
**  Initialize line draw
*/
void InitLineDraw()
{
}

}

void CVideo::DrawPixelClip(Uint32 color, int x, int y)
{
	if (UseOpenGL) {
		linedraw_gl::DrawPixelClip(color, x, y);
	} else {
		linedraw_sdl::DrawPixelClip(color, x, y);
	}
}
void CVideo::DrawTransPixelClip(Uint32 color, int x, int y, unsigned char alpha)
{
	if (UseOpenGL) {
		linedraw_gl::DrawTransPixelClip(color, x, y, alpha);
	} else {
		linedraw_sdl::DrawTransPixelClip(color, x, y, alpha);
	}
}

void CVideo::DrawVLine(Uint32 color, int x, int y, int height)
{
	if (UseOpenGL) {
		linedraw_gl::DrawVLine(color, x, y, height);
	} else {
		linedraw_sdl::DrawVLine(color, x, y, height);
	}
}
void CVideo::DrawTransVLine(Uint32 color, int x, int y, int height, unsigned char alpha)
{
	if (UseOpenGL) {
		linedraw_gl::DrawTransVLine(color, x, y, height, alpha);
	} else {
		linedraw_sdl::DrawTransVLine(color, x, y, height, alpha);
	}
}
void CVideo::DrawVLineClip(Uint32 color, int x, int y, int height)
{
	if (UseOpenGL) {
		linedraw_gl::DrawVLineClip(color, x, y, height);
	} else {
		linedraw_sdl::DrawVLineClip(color, x, y, height);
	}
}
void CVideo::DrawTransVLineClip(Uint32 color, int x, int y, int height, unsigned char alpha)
{
	if (UseOpenGL) {
		linedraw_gl::DrawTransVLineClip(color, x, y, height, alpha);
	} else {
		linedraw_sdl::DrawTransVLineClip(color, x, y, height, alpha);
	}
}

void CVideo::DrawHLine(Uint32 color, int x, int y, int width)
{
	if (UseOpenGL) {
		linedraw_gl::DrawHLine(color, x, y, width);
	} else {
		linedraw_sdl::DrawHLine(color, x, y, width);
	}
}
void CVideo::DrawTransHLine(Uint32 color, int x, int y, int width, unsigned char alpha)
{
	if (UseOpenGL) {
		linedraw_gl::DrawTransHLine(color, x, y, width, alpha);
	} else {
		linedraw_sdl::DrawTransHLine(color, x, y, width, alpha);
	}
}
void CVideo::DrawHLineClip(Uint32 color, int x, int y, int width)
{
	if (UseOpenGL) {
		linedraw_gl::DrawHLineClip(color, x, y, width);
	} else {
		linedraw_sdl::DrawHLineClip(color, x, y, width);
	}
}
void CVideo::DrawTransHLineClip(Uint32 color, int x, int y, int width, unsigned char alpha)
{
	if (UseOpenGL) {
		linedraw_gl::DrawTransHLineClip(color, x, y, width, alpha);
	} else {
		linedraw_sdl::DrawTransHLineClip(color, x, y, width, alpha);
	}
}

void CVideo::DrawLine(Uint32 color, int sx, int sy, int dx, int dy)
{
	if (UseOpenGL) {
		linedraw_gl::DrawLine(color, sx, sy, dx, dy);
	} else {
		linedraw_sdl::DrawLine(color, sx, sy, dx, dy);
	}
}
void CVideo::DrawTransLine(Uint32 color, int sx, int sy, int dx, int dy, unsigned char alpha)
{
	if (UseOpenGL) {
		linedraw_gl::DrawTransLine(color, sx, sy, dx, dy, alpha);
	} else {
		linedraw_sdl::DrawTransLine(color, sx, sy, dx, dy, alpha);
	}
}
void CVideo::DrawLineClip(Uint32 color, int sx, int sy, int dx, int dy)
{
	if (UseOpenGL) {
		linedraw_gl::DrawLineClip(color, sx, sy, dx, dy);
	} else {
		linedraw_sdl::DrawLineClip(color, sx, sy, dx, dy);
	}
}
void CVideo::DrawTransLineClip(Uint32 color, int sx, int sy, int dx, int dy, unsigned char alpha)
{
	if (UseOpenGL) {
		linedraw_gl::DrawTransLineClip(color, sx, sy, dx, dy, alpha);
	} else {
		linedraw_sdl::DrawTransLineClip(color, sx, sy, dx, dy, alpha);
	}
}

void CVideo::DrawRectangle(Uint32 color, int x, int y, int w, int h)
{
	if (UseOpenGL) {
		linedraw_gl::DrawRectangle(color, x, y, w, h);
	} else {
		linedraw_sdl::DrawRectangle(color, x, y, w, h);
	}
}
void CVideo::DrawTransRectangle(Uint32 color, int x, int y, int w, int h, unsigned char alpha)
{
	if (UseOpenGL) {
		linedraw_gl::DrawTransRectangle(color, x, y, w, h, alpha);
	} else {
		linedraw_sdl::DrawTransRectangle(color, x, y, w, h, alpha);
	}
}
void CVideo::DrawRectangleClip(Uint32 color, int x, int y, int w, int h)
{
	if (UseOpenGL) {
		linedraw_gl::DrawRectangleClip(color, x, y, w, h);
	} else {
		linedraw_sdl::DrawRectangleClip(color, x, y, w, h);
	}
}
void CVideo::DrawTransRectangleClip(Uint32 color, int x, int y, int w, int h, unsigned char alpha)
{
	if (UseOpenGL) {
		linedraw_gl::DrawTransRectangleClip(color, x, y, w, h, alpha);
	} else {
		linedraw_sdl::DrawTransRectangleClip(color, x, y, w, h, alpha);
	}
}

void CVideo::FillRectangle(Uint32 color, int x, int y, int w, int h)
{
	if (UseOpenGL) {
		linedraw_gl::FillRectangle(color, x, y, w, h);
	} else {
		linedraw_sdl::FillRectangle(color, x, y, w, h);
	}
}
void CVideo::FillTransRectangle(Uint32 color, int x, int y, int w, int h, unsigned char alpha)
{
	if (UseOpenGL) {
		linedraw_gl::FillTransRectangle(color, x, y, w, h, alpha);
	} else {
		linedraw_sdl::FillTransRectangle(color, x, y, w, h, alpha);
	}
}
void CVideo::FillRectangleClip(Uint32 color, int x, int y, int w, int h)
{
	if (UseOpenGL) {
		linedraw_gl::FillRectangleClip(color, x, y, w, h);
	} else {
		linedraw_sdl::FillRectangleClip(color, x, y, w, h);
	}
}
void CVideo::FillTransRectangleClip(Uint32 color, int x, int y, int w, int h, unsigned char alpha)
{
	if (UseOpenGL) {
		linedraw_gl::FillTransRectangleClip(color, x, y, w, h, alpha);
	} else {
		linedraw_sdl::FillTransRectangleClip(color, x, y, w, h, alpha);
	}
}

void CVideo::DrawCircle(Uint32 color, int x, int y, int r)
{
	if (UseOpenGL) {
		linedraw_gl::DrawCircle(color, x, y, r);
	} else {
		linedraw_sdl::DrawCircle(color, x, y, r);
	}
}
void CVideo::DrawTransCircle(Uint32 color, int x, int y, int r, unsigned char alpha)
{
	if (UseOpenGL) {
		linedraw_gl::DrawTransCircle(color, x, y, r, alpha);
	} else {
		linedraw_sdl::DrawTransCircle(color, x, y, r, alpha);
	}
}
void CVideo::DrawCircleClip(Uint32 color, int x, int y, int r)
{
	if (UseOpenGL) {
		linedraw_gl::DrawCircleClip(color, x, y, r);
	} else {
		linedraw_sdl::DrawCircleClip(color, x, y, r);
	}
}
void CVideo::DrawTransCircleClip(Uint32 color, int x, int y, int r, unsigned char alpha)
{
	if (UseOpenGL) {
		linedraw_gl::DrawTransCircleClip(color, x, y, r, alpha);
	} else {
		linedraw_sdl::DrawTransCircleClip(color, x, y, r, alpha);
	}
}

void CVideo::FillCircle(Uint32 color, int x, int y, int r)
{
	if (UseOpenGL) {
		linedraw_gl::FillCircle(color, x, y, r);
	} else {
		linedraw_sdl::FillCircle(color, x, y, r);
	}
}
void CVideo::FillTransCircle(Uint32 color, int x, int y, int r, unsigned char alpha)
{
	if (UseOpenGL) {
		linedraw_gl::FillTransCircle(color, x, y, r, alpha);
	} else {
		linedraw_sdl::FillTransCircle(color, x, y, r, alpha);
	}
}
void CVideo::FillCircleClip(Uint32 color, int x, int y, int r)
{
	if (UseOpenGL) {
		linedraw_gl::FillCircleClip(color, x, y, r);
	} else {
		linedraw_sdl::FillCircleClip(color, x, y, r);
	}
}
void CVideo::FillTransCircleClip(Uint32 color, int x, int y, int r, unsigned char alpha)
{
	if (UseOpenGL) {
		linedraw_gl::FillTransCircleClip(color, x, y, r, alpha);
	} else {
		linedraw_sdl::FillTransCircleClip(color, x, y, r, alpha);
	}
}

void InitLineDraw()
{
	if (UseOpenGL) {
		linedraw_gl::InitLineDraw();
	} else {
		linedraw_sdl::InitLineDraw();
	}
}

//@}
