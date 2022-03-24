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

#include "stratagus.h"
#include "video.h"

#include "intern_video.h"


/*----------------------------------------------------------------------------
-- Declarations
----------------------------------------------------------------------------*/

/**
** Bitmask, denoting a position left/right/above/below clip rectangle
** (mainly used by VideoDrawLineClip)
*/
#define ClipCodeInside 0 /// Clipping inside rectangle
#define ClipCodeAbove  1 /// Clipping above rectangle
#define ClipCodeBelow  2 /// Clipping below rectangle
#define ClipCodeLeft   4 /// Clipping left rectangle
#define ClipCodeRight  8 /// Clipping right rectangle


namespace linedraw_sdl
{

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
	// Loses precision for speed
	alpha = (255 - alpha) >> 3;

	Uint16 *p = &((Uint16 *)TheScreen->pixels)[x + y * Video.Width];
	color = (((color << 16) | color) & 0x07E0F81F);
	unsigned long dp = *p;
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
	alpha = 255 - alpha;

	Uint32 *p = &((Uint32 *)TheScreen->pixels)[x + y * Video.Width];

	const unsigned long sp2 = (color & 0xFF00FF00) >> 8;
	color &= 0x00FF00FF;

	unsigned long dp1 = *p;
	unsigned long dp2 = (dp1 & 0xFF00FF00) >> 8;
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
		std::swap(dx, sx);
		std::swap(dy, sy);
	}
	int xlen;
	int incr;

	int ylen = dy - sy;

	if (sx > dx) {
		xlen = sx - dx;
		incr = -1;
	} else {
		xlen = dx - sx;
		incr = 1;
	}

	int y = sy;
	int x = sx;

	if (xlen > ylen) {
		int p;

		if (sx > dx) {
			std::swap(sx, dx);
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
		int p = (xlen << 1) - ylen;

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
		std::swap(dx, sx);
		std::swap(dy, sy);
	}
	int ylen = dy - sy;
	int xlen;
	int incr;

	if (sx > dx) {
		xlen = sx - dx;
		incr = -1;
	} else {
		xlen = dx - sx;
		incr = 1;
	}

	int y = sy;
	int x = sx;

	if (xlen > ylen) {
		if (sx > dx) {
			std::swap(dx, sx);
			y = dy;
		}

		int p = (ylen << 1) - xlen;

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
		int p = (xlen << 1) - ylen;

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
	SDL_Rect drect = {Sint16(x), Sint16(y), Uint16(w), Uint16(h)};
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
	int p = 1 - r;
	int py = r;

	Video.LockScreen();
	for (int px = 0; px <= py + 1; ++px) {
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
	int p = 1 - r;
	int py = r;

	Video.LockScreen();
	for (int px = 0; px <= py + 1; ++px) {
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
	int p = 1 - r;
	int py = r;

	Video.LockScreen();
	for (int px = 0; px <= py + 1; ++px) {
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
	int p = 1 - r;
	int py = r;

	Video.LockScreen();
	for (int px = 0; px <= py + 1; ++px) {
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
**  Draw an ellipse clipped
*/
void DrawEllipseClip(Uint32 color, int xc, int yc, int rx, int ry)
{
	int rx2 = rx*rx, ry2 = ry*ry , frx2 = 4*rx2, fry2 = 4*ry2;

	int x, y, sigma;

	for (x = 0, y = ry, sigma = 2*ry2+rx2*(1-2*ry); ry2*x <= rx2*y ; x++) {
		VideoDoDrawPixelClip(color, xc + x, yc + y);
		VideoDoDrawPixelClip(color, xc - x, yc + y);
		VideoDoDrawPixelClip(color, xc + x, yc - y);
		VideoDoDrawPixelClip(color, xc - x, yc - y);

		if (sigma >= 0) {
			sigma += frx2*(1-y);
			y--;
		}
		sigma += ry2*(4*x+6);
	}

	for (x = rx, y = 0, sigma = 2*rx2+ry2*(1-2*rx); rx2*y <= ry2*x ; y++) {
		VideoDoDrawPixelClip(color, xc + x, yc + y);
		VideoDoDrawPixelClip(color, xc - x, yc + y);
		VideoDoDrawPixelClip(color, xc + x, yc - y);
		VideoDoDrawPixelClip(color, xc - x, yc - y);

		if (sigma >= 0) {
			sigma += fry2*(1-x);
			x--;
		}
		sigma += rx2*(4*y+6);
	}

	Video.UnlockScreen();
}

/**
**  Draw a filled circle
*/
void FillCircle(Uint32 color, int x, int y, int r)
{
	int p = 1 - r;
	int py = r;

	for (int px = 0; px <= py; ++px) {

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
	int p = 1 - r;
	int py = r;

	for (int px = 0; px <= py; ++px) {

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
	int p = 1 - r;
	int py = r;

	for (int px = 0; px <= py; ++px) {

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
	int p = 1 - r;
	int py = r;

	for (int px = 0; px <= py; ++px) {

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

void CVideo::DrawPixelClip(Uint32 color, int x, int y)
{
	linedraw_sdl::DrawPixelClip(color, x, y);
}
void CVideo::DrawTransPixelClip(Uint32 color, int x, int y, unsigned char alpha)
{
	linedraw_sdl::DrawTransPixelClip(color, x, y, alpha);
}

void CVideo::DrawVLine(Uint32 color, int x, int y, int height)
{
	linedraw_sdl::DrawVLine(color, x, y, height);
}
void CVideo::DrawTransVLine(Uint32 color, int x, int y, int height, unsigned char alpha)
{
	linedraw_sdl::DrawTransVLine(color, x, y, height, alpha);
}
void CVideo::DrawVLineClip(Uint32 color, int x, int y, int height)
{
	linedraw_sdl::DrawVLineClip(color, x, y, height);
}
void CVideo::DrawTransVLineClip(Uint32 color, int x, int y, int height, unsigned char alpha)
{
	linedraw_sdl::DrawTransVLineClip(color, x, y, height, alpha);
}

void CVideo::DrawHLine(Uint32 color, int x, int y, int width)
{
	linedraw_sdl::DrawHLine(color, x, y, width);
}
void CVideo::DrawTransHLine(Uint32 color, int x, int y, int width, unsigned char alpha)
{
	linedraw_sdl::DrawTransHLine(color, x, y, width, alpha);
}
void CVideo::DrawHLineClip(Uint32 color, int x, int y, int width)
{
	linedraw_sdl::DrawHLineClip(color, x, y, width);
}
void CVideo::DrawTransHLineClip(Uint32 color, int x, int y, int width, unsigned char alpha)
{
	linedraw_sdl::DrawTransHLineClip(color, x, y, width, alpha);
}

void CVideo::DrawLine(Uint32 color, int sx, int sy, int dx, int dy)
{
	linedraw_sdl::DrawLine(color, sx, sy, dx, dy);
}
void CVideo::DrawTransLine(Uint32 color, int sx, int sy, int dx, int dy, unsigned char alpha)
{
	linedraw_sdl::DrawTransLine(color, sx, sy, dx, dy, alpha);
}
void CVideo::DrawLineClip(Uint32 color, const PixelPos &pos1, const PixelPos &pos2)
{
	linedraw_sdl::DrawLineClip(color, pos1.x, pos1.y, pos2.x, pos2.y);
}
void CVideo::DrawTransLineClip(Uint32 color, int sx, int sy, int dx, int dy, unsigned char alpha)
{
	linedraw_sdl::DrawTransLineClip(color, sx, sy, dx, dy, alpha);
}

void CVideo::DrawRectangle(Uint32 color, int x, int y, int w, int h)
{
	linedraw_sdl::DrawRectangle(color, x, y, w, h);
}
void CVideo::DrawTransRectangle(Uint32 color, int x, int y, int w, int h, unsigned char alpha)
{
	linedraw_sdl::DrawTransRectangle(color, x, y, w, h, alpha);
}
void CVideo::DrawRectangleClip(Uint32 color, int x, int y, int w, int h)
{
	linedraw_sdl::DrawRectangleClip(color, x, y, w, h);
}
void CVideo::DrawTransRectangleClip(Uint32 color, int x, int y, int w, int h, unsigned char alpha)
{
	linedraw_sdl::DrawTransRectangleClip(color, x, y, w, h, alpha);
}

void CVideo::FillRectangle(Uint32 color, int x, int y, int w, int h)
{
	linedraw_sdl::FillRectangle(color, x, y, w, h);
}
void CVideo::FillTransRectangle(Uint32 color, int x, int y, int w, int h, unsigned char alpha)
{
	linedraw_sdl::FillTransRectangle(color, x, y, w, h, alpha);
}
void CVideo::FillRectangleClip(Uint32 color, int x, int y, int w, int h)
{
	linedraw_sdl::FillRectangleClip(color, x, y, w, h);
}
void CVideo::FillTransRectangleClip(Uint32 color, int x, int y, int w, int h, unsigned char alpha)
{
	linedraw_sdl::FillTransRectangleClip(color, x, y, w, h, alpha);
}

void CVideo::DrawCircle(Uint32 color, int x, int y, int r)
{
	linedraw_sdl::DrawCircle(color, x, y, r);
}
void CVideo::DrawTransCircle(Uint32 color, int x, int y, int r, unsigned char alpha)
{
	linedraw_sdl::DrawTransCircle(color, x, y, r, alpha);
}
void CVideo::DrawCircleClip(Uint32 color, int x, int y, int r)
{
	linedraw_sdl::DrawCircleClip(color, x, y, r);
}
void CVideo::DrawTransCircleClip(Uint32 color, int x, int y, int r, unsigned char alpha)
{
	linedraw_sdl::DrawTransCircleClip(color, x, y, r, alpha);
}

void CVideo::DrawEllipseClip(Uint32 color, int xc, int yc, int rx, int ry)
{
	linedraw_sdl::DrawEllipseClip(color, xc, yc, rx, ry);
}

void CVideo::FillCircle(Uint32 color, int x, int y, int r)
{
	linedraw_sdl::FillCircle(color, x, y, r);
}
void CVideo::FillTransCircle(Uint32 color, int x, int y, int r, unsigned char alpha)
{
	linedraw_sdl::FillTransCircle(color, x, y, r, alpha);
}
void CVideo::FillCircleClip(Uint32 color, const PixelPos &screenPos, int r)
{
	linedraw_sdl::FillCircleClip(color, screenPos.x, screenPos.y, r);
}
void CVideo::FillTransCircleClip(Uint32 color, int x, int y, int r, unsigned char alpha)
{
	linedraw_sdl::FillTransCircleClip(color, x, y, r, alpha);
}

void InitLineDraw()
{
	linedraw_sdl::InitLineDraw();
}

//@}
