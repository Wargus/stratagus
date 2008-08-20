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
//      (c) Copyright 2000-2006 by Lutz Sammer, Stephan Rasenberg,
//                                 Jimmy Salmon, and Nehal Mistry
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

/*----------------------------------------------------------------------------
-- Variables
----------------------------------------------------------------------------*/

#ifndef USE_OPENGL
#include "renderer.h"

static Primitive16_555_t render16_555;
static Primitive16_565_t render16_565;
static Primitive32_t render32;
static CPrimitives *Renderer = NULL;

#endif

// ===========================================================================
// Pixel
// ===========================================================================

#ifndef USE_OPENGL

/**
**  FIXME: docu
*/
void CVideo::DrawPixelClip(Uint32 color, int x, int y)
{
	if (x >= ClipX1 && y >= ClipY1 && x <= ClipX2 && y <= ClipY2) {
		LockScreen();
		Renderer->DrawPixel(color, x, y);
		UnlockScreen();
	}
}

/**
**  FIXME: docu
*/
void CVideo::DrawTransPixelClip(Uint32 color, int x, int y, unsigned char alpha)
{
	if (x >= ClipX1 && y >= ClipY1 && x <= ClipX2 && y <= ClipY2) {
		LockScreen();
		Renderer->DrawTransPixel(color, x, y, alpha);
		UnlockScreen();
	}
}

/**
**  FIXME: docu
*/
void CVideo::DrawVLine(Uint32 color, int x, int y, int height)
{
	LockScreen();
	Renderer->DrawLine(color, x, y, x, (y + height));
	UnlockScreen();
}

/**
**  FIXME: docu
*/
void CVideo::DrawTransVLine(Uint32 color, int x, int y,
	int height, unsigned char alpha)
{
	LockScreen();
	Renderer->DrawTransLine(color, x, y, x, y + height, alpha);
	UnlockScreen();
}

/**
**  FIXME: docu
*/
void CVideo::DrawVLineClip(Uint32 color, int x, int y, int height)
{
	int w = 1;
	CLIP_RECTANGLE(x, y, w, height);
	DrawVLine(color, x, y, height);
}

/**
**  FIXME: docu
*/
void CVideo::DrawTransVLineClip(Uint32 color, int x, int y,
	int height, unsigned char alpha)
{
	if (x < ClipX1 || x > ClipX2 ||
	 	y > ClipY2 || y + height < ClipY1) return;
	
	if(y < ClipY1) y = ClipY1;
	if(y + height > ClipY2) height = ClipY2 - y;
	
	LockScreen();
	Renderer->DrawTransLine(color, x, y, x, y + height, alpha);
	UnlockScreen();
}

/**
**  FIXME: docu
*/
void CVideo::DrawHLine(Uint32 color, int x, int y, int width)
{
	LockScreen();
	Renderer->DrawLine(color, x, y, x + width, y);
	UnlockScreen();
}

/**
**  FIXME: docu
*/
void CVideo::DrawHLineClip(Uint32 color, int x, int y, int width)
{
	int h = 1;
	CLIP_RECTANGLE(x, y, width, h);
	DrawHLine(color, x, y, width);
}

/**
**  FIXME: docu
*/
void CVideo::DrawTransHLine(Uint32 color, int x, int y,
	int width, unsigned char alpha)
{
	LockScreen();
	Renderer->DrawTransLine(color, x, y, x + width, y, alpha);
	UnlockScreen();
}

/**
**  FIXME: docu
*/
void CVideo::DrawTransHLineClip(Uint32 color, int x, int y,
	int width, unsigned char alpha)
{
	if (y < ClipY1 || y > ClipY2 ||
		x >ClipX2 || x + width < ClipX1) return;	
	
	if(x < ClipX1) x = ClipX1;
	if(x + width > ClipX2) width = ClipX2 - x;
	
	LockScreen();
	Renderer->DrawTransLine(color, x, y, x + width, y, alpha);
	UnlockScreen();
}

/** Check to see if a line intersects another */
static inline
bool LineIntersectLine( const int v1[2], const int v2[2], 
				const int v3[2], const int v4[2] )
{
#define X 0
#define Y 1
    float denom = ((v4[Y] - v3[Y]) * (v2[X] - v1[X])) - ((v4[X] - v3[X]) * (v2[Y] - v1[Y]));

    if ( denom < 0.000000001f && denom > -0.000000001f )
    {
//        if ( numerator == 0.0f && numerator2 == 0.0f )
//        {
 //           return false;//COINCIDENT;
 //       }
        return false;// PARALLEL;
    }
    
    float numerator = ((v4[X] - v3[X]) * (v1[Y] - v3[Y])) - ((v4[Y] - v3[Y]) * (v1[X] - v3[X]));
    float ua = numerator / denom;
    
    if(ua < 0.0f || ua > 1.0f) return false;
    
    float numerator2 = ((v2[X] - v1[X]) * (v1[Y] - v3[Y])) - ((v2[Y] - v1[Y]) * (v1[X] - v3[X]));
    float ub = numerator2/ denom;

	return (ub >= 0.0f && ub <= 1.0f);
    //return (ua >= 0.0f && ua <= 1.0f && ub >= 0.0f && ub <= 1.0f);
#undef X
#undef Y
}

/**
**  FIXME: docu
*/
void CVideo::DrawLine(Uint32 color, int sx, int sy, int dx, int dy)
{
	LockScreen();
	Renderer->DrawLine(color, sx, sy, dx, dy);
	UnlockScreen();
}

/**
**  FIXME: docu
*/
void CVideo::DrawLineClip(Uint32 color, int sx, int sy, int dx, int dy)
{
	if(dy > sy)
		if(dy < ClipY1 || sy > ClipY2) return;
	else
		if(sy < ClipY1 || dy > ClipY2) return;

	if(dx > sx)
		if(dx < ClipX1 || sx > ClipX2) return;
	else
		if(sx < ClipX1 || dx > ClipX2) return;
	
	bool s_is_on_screen = sx >= ClipX1 && sx <= ClipX2 &&
							sy >= ClipY1 && sy <= ClipY2;
	bool d_is_on_screen = dx >= ClipX1 && dx <= ClipX2 &&
							dy >= ClipY1 && dy <= ClipY2;

	if(!(s_is_on_screen && d_is_on_screen)) {
	

		if(!s_is_on_screen && d_is_on_screen) {
			// exchange coordinates
			int t = dx;
			dx = sx;
			sx = t;
			t = dy;
			dy = sy;
			sy = t;
			s_is_on_screen = true; 
			d_is_on_screen = false;
		}

			const int v1[] = {sx, sy};
			const int v2[] = {dx, dy};
			const int v3[] = {ClipX1, ClipY1};
			const int v4[] = {ClipX1, ClipY2};
			const int v5[] = {ClipX2, ClipY1};
			const int v6[] = {ClipX2, ClipY2};

	
		if(s_is_on_screen && !d_is_on_screen) {
						
			//Thanks for good geometric Mr. Talles :)
			
			if(LineIntersectLine(v1, v2, v3, v4) ||
				LineIntersectLine(v1, v2, v5, v6))
			{
				const int x_max = dx <= ClipX1 ? ClipX1 : ClipX2;
				float tmp = (dy - sy);
				tmp /= (dx - sx);
				tmp *= (x_max - sx);
				tmp += sy;
		 		dy = tmp;
		 		dx = x_max;
		 	} else {
		 		const int y_max = dy <= ClipY1 ? ClipY1 : ClipY2;
		 		float tmp = (dx - sx);
		 		tmp /= (dy - sy);
		 		tmp *= (y_max - sy);
		 		tmp += sx;
		 		dx = tmp;
		 		dy = y_max;
		 	}
		} else {
			const int v7[] = {ClipX1, ClipY1};
			const int v8[] = {ClipX2, ClipY1};
			const int v9[] = {ClipX1, ClipY2};
			const int v0[] = {ClipX2, ClipY2};
					
		 	//both are out of screen we have to check Intersection		
			if(!LineIntersectLine(v1, v2, v3, v4) &&
				!LineIntersectLine(v1, v2, v5, v6) &&
				!LineIntersectLine(v1, v2, v7, v8) &&
				!LineIntersectLine(v1, v2, v9, v0))
			{
				return;
			}
			DebugPrint("FIXME: Corrupted Line coordinates: [%d,%d]: [%d,%d]\n" 
						_C_ sx _C_ sy _C_ dx _C_ dy);

			return;//FIXME: add cliping code insteed of return
		}				
	}
		
	LockScreen();
	Renderer->DrawLine(color, sx, sy, dx, dy);
	UnlockScreen();
}

/**
**  FIXME: docu
*/
void CVideo::DrawTransLine(Uint32 color, int sx, int sy,
	int dx, int dy, unsigned char alpha)
{
	LockScreen();
	Renderer->DrawTransLine(color, sx, sy, dx, dy,alpha);
	UnlockScreen();
}

/**
**  FIXME: docu
*/
void CVideo::DrawRectangle(Uint32 color, int x, int y, int w, int h)
{
	LockScreen();
	Renderer->DrawRectangle(color, x, y, w, h);
	UnlockScreen();
}

/**
**  FIXME: docu
*/
void CVideo::DrawRectangleClip(Uint32 color, int x, int y, int w, int h)
{
	DrawTransPixelClip(color, x, y, 128);
	DrawHLineClip(color, x + 1, y, w - 2);
	DrawTransPixelClip(color, x + w - 1, y, 128);
	
	DrawTransPixelClip(color, x, y + h - 1, 128);
	DrawHLineClip(color, x + 1, y + h - 1, w - 2);
	DrawTransPixelClip(color, x + w - 1, y + h - 1, 128);
	
	DrawVLineClip(color, x, y + 1, h - 2);
	DrawVLineClip(color, x + w - 1, y + 1, h - 2);
}

/**
**  FIXME: docu
*/
void CVideo::DrawTransRectangle(Uint32 color, int x, int y,
	int w, int h, unsigned char alpha)
{
	LockScreen();
	Renderer->DrawTransRectangle(color, x, y, w, h, alpha);
	UnlockScreen();
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
void CVideo::DrawTransRectangleClip(Uint32 color, int x, int y,
	int w, int h, unsigned char alpha)
{
	DrawTransPixelClip(color, x, y, alpha / 2);
	DrawTransHLineClip(color, x + 1, y, w - 2, alpha);
	DrawTransPixelClip(color, x + w - 1, y, alpha / 2);
	
	DrawTransPixelClip(color, x, y + h - 1, alpha / 2);
	DrawTransHLineClip(color, x + 1, y + h - 1, w - 2, alpha);
	DrawTransPixelClip(color, x + w - 1, y + h - 1, alpha / 2);
	
	DrawTransVLineClip(color, x, y + 1, h - 2, alpha);
	DrawTransVLineClip(color, x + w - 1, y + 1, h - 2, alpha);
}

/**
**  FIXME: docu
*/
void CVideo::FillRectangle(Uint32 color, int x, int y, int w, int h)
{
	SDL_Rect drect = {x, y, w, h};
	SDL_FillRect(TheScreen, &drect, color);
}

/**
**  FIXME: docu
*/
void CVideo::FillRectangleClip(Uint32 color, int x, int y,
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
**  FIXME: docu
*/
void CVideo::FillTransRectangle(Uint32 color, int x, int y,
	int w, int h, unsigned char alpha)
{
	LockScreen();
	Renderer->FillTransRectangle(color, x, y, w, h, alpha);
	UnlockScreen();
}

/**
**  FIXME: docu
*/
void CVideo::FillTransRectangleClip(Uint32 color, int x, int y,
	int w, int h, unsigned char alpha)
{
	CLIP_RECTANGLE(x, y, w, h);
	FillTransRectangle(color, x, y, w, h, alpha);
}

/**
**  FIXME: docu
*/
void CVideo::DrawCircle(Uint32 color, int x, int y, int r)
{
	LockScreen();
	Renderer->DrawCircle(color, x, y, r);
	UnlockScreen();
}

/**
**  FIXME: docu
*/
void CVideo::DrawTransCircle(Uint32 color, int x, int y,
	int r, unsigned char alpha)
{
	LockScreen();
	Renderer->DrawTransCircle(color, x, y, r, alpha);
	UnlockScreen();
}

/**
**  FIXME: docu
*/
void CVideo::DrawCircleClip(Uint32 color, int x, int y, int r)
{
	int p;
	int px;
	int py;

	p = 1 - r;
	py = r;

#define DoDrawPixelClip(color,x,y) \
	if (x >= ClipX1 && y >= ClipY1 && x <= ClipX2 && y <= ClipY2) { \
		Renderer->DrawPixel(color, x, y); \
	}

	LockScreen();
	for (px = 0; px <= py + 1; ++px) {
		DoDrawPixelClip(color, x + px, y + py);
		DoDrawPixelClip(color, x + px, y - py);
		DoDrawPixelClip(color, x - px, y + py);
		DoDrawPixelClip(color, x - px, y - py);

		DoDrawPixelClip(color, x + py, y + px);
		DoDrawPixelClip(color, x + py, y - px);
		DoDrawPixelClip(color, x - py, y + px);
		DoDrawPixelClip(color, x - py, y - px);

		if (p < 0) {
			p += 2 * px + 3;
		} else {
			p += 2 * (px - py) + 5;
			py -= 1;
		}
	}
	UnlockScreen();
#undef DoDrawPixelClip
}

/**
**  FIXME: docu
*/
void CVideo::DrawTransCircleClip(Uint32 color, int x, int y,
	int r, unsigned char alpha)
{
	int p;
	int px;
	int py;

	p = 1 - r;
	py = r;

#define DoDrawTransPixel(color,x,y, alpha) \
	if (x >= ClipX1 && y >= ClipY1 && x <= ClipX2 && y <= ClipY2) { \
		Renderer->DrawTransPixel(color, x, y, alpha); \
	}


	LockScreen();
	for (px = 0; px <= py + 1; ++px) {

		DoDrawTransPixel(color, x + px, y + py, alpha);
		DoDrawTransPixel(color, x + px, y - py, alpha);
		DoDrawTransPixel(color, x - px, y + py, alpha);
		DoDrawTransPixel(color, x - px, y - py, alpha);

		DoDrawTransPixel(color, x + py, y + px, alpha);
		DoDrawTransPixel(color, x + py, y - px, alpha);
		DoDrawTransPixel(color, x - py, y + px, alpha);
		DoDrawTransPixel(color, x - py, y - px, alpha);

		if (p < 0) {
			p += 2 * px + 3;
		} else {
			p += 2 * (px - py) + 5;
			py -= 1;
		}
	}
	UnlockScreen();
#undef DoDrawTransPixel
}

/**
**  FIXME: docu
*/
void CVideo::FillCircle(Uint32 color, int x, int y, int r)
{
	LockScreen();
	Renderer->FillCircle(color, x, y, r);
	UnlockScreen();
}

/**
**  FIXME: docu
*/
void CVideo::FillTransCircle(Uint32 color, int x, int y,
	int r, unsigned char alpha)
{
	LockScreen();
	Renderer->FillTransCircle(color, x, y, r, alpha);
	UnlockScreen();
}

/**
**  FIXME: docu
*/
void CVideo::FillCircleClip(Uint32 color, int x, int y, int r)
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
**  FIXME: docu
*/
void CVideo::FillTransCircleClip(Uint32 color, int x, int y,
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
**  FIXME: docu
*/
void InitLineDraw(void)
{
	switch (Video.Depth) {
		case 16:
			DebugPrint("Init 16bit Software Renderer\n");
			if(TheScreen->format->Gmask == 0x7e0)
			{
				Renderer = &render16_565;
			} else {
				if(TheScreen->format->Gmask == 0x3e0) {
					Renderer = &render16_555;
				} else {
					assert(0);
				}
			}
			break;
		case 32:
			DebugPrint("Init 32bit Software Renderer\n");
			Renderer = &render32;
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
void VideoDrawPixel(Uint32 color, int x, int y)
{
	GLubyte r, g, b, a;

	Video.GetRGBA(color, &r, &g, &b, &a);
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
void VideoDrawTransPixel(Uint32 color, int x, int y,
	unsigned char alpha)
{
	GLubyte r, g, b;

	Video.GetRGB(color, &r, &g, &b);
	color = Video.MapRGBA(0, r, g, b, alpha);
	VideoDrawPixel(color, x, y);
}

/**
**  Draw pixel clipped to current clip setting.
**
**  @param color  color
**  @param x      x coordinate on the screen
**  @param y      y coordinate on the screen
*/
void CVideo::DrawPixelClip(Uint32 color, int x, int y)
{
	if (x < ClipX1 || x > ClipX2 || y < ClipY1 || y > ClipY2) {
		return;
	}
	VideoDrawPixel(color, x, y);
}

/**
**  Draw translucent pixel clipped to current clip setting.
**
**  @param color  color
**  @param x      x coordinate on the screen
**  @param y      y coordinate on the screen
**  @param alpha  alpha value of pixel.
*/
void CVideo::DrawTransPixelClip(Uint32 color, int x, int y,
	unsigned char alpha)
{
	GLubyte r, g, b;

	GetRGB(color, &r, &g, &b);
	color = MapRGBA(0, r, g, b, alpha);
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
void CVideo::DrawHLine(Uint32 color, int x, int y, int width)
{
	GLubyte r, g, b, a;

	GetRGBA(color, &r, &g, &b, &a);
	glDisable(GL_TEXTURE_2D);
	glColor4ub(r, g, b, a);
	glBegin(GL_LINES);
	glVertex2i(x, y);
	glVertex2i(x + width, y);
	glEnd();
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
void CVideo::DrawTransHLine(Uint32 color, int x, int y, int width,
	unsigned char alpha)
{
	GLubyte r, g, b;

	GetRGB(color, &r, &g, &b);
	color = MapRGBA(0, r, g, b, alpha);
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
void CVideo::DrawHLineClip(Uint32 color, int x, int y, int width)
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
void CVideo::DrawTransHLineClip(Uint32 color, int x, int y, int width,
	unsigned char alpha)
{
	GLubyte r, g, b;

	GetRGB(color, &r, &g, &b);
	color = MapRGBA(0, r, g, b, alpha);
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
void CVideo::DrawVLine(Uint32 color, int x, int y, int height)
{
	GLubyte r, g, b, a;

	GetRGBA(color, &r, &g, &b, &a);
	glDisable(GL_TEXTURE_2D);
	glColor4ub(r, g, b, a);
	glBegin(GL_LINES);
	glVertex2i(x, y);
	glVertex2i(x, y + height);
	glEnd();
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
void CVideo::DrawTransVLine(Uint32 color, int x, int y, int height,
	unsigned char alpha)
{
	GLubyte r, g, b;

	GetRGB(color, &r, &g, &b);
	color = MapRGBA(0, r, g, b, alpha);
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
void CVideo::DrawVLineClip(Uint32 color, int x, int y, int height)
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
void CVideo::DrawTransVLineClip(Uint32 color, int x, int y,
	int height, unsigned char alpha)
{
	GLubyte r, g, b;

	GetRGB(color, &r, &g, &b);
	color = MapRGBA(0, r, g, b, alpha);
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
void CVideo::DrawLine(Uint32 color, int x1, int y1, int x2, int y2)
{
	float xx1, yy1, xx2, yy2;
	GLubyte r, g, b, a;

	xx1 = x1; xx2 = x2; yy1 = y1; yy2 = y2;
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

	GetRGBA(color, &r, &g, &b, &a);
	glDisable(GL_TEXTURE_2D);
	glColor4ub(r, g, b, a);
	glBegin(GL_LINES);
	glVertex2f(xx1, yy1);
	glVertex2f(xx2, yy2);
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
void CVideo::DrawLineClip(Uint32 color, int x1, int y1, int x2, int y2)
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
**  Draw rectangle.
**
**  @param color  color
**  @param x      x coordinate on the screen
**  @param y      y coordinate on the screen
**  @param h      height of rectangle (0=don't draw).
**  @param w      width of rectangle (0=don't draw).
*/
void CVideo::DrawRectangle(Uint32 color, int x, int y, int w, int h)
{
	GLubyte r, g, b, a;

	GetRGBA(color, &r, &g, &b, &a);
	glDisable(GL_TEXTURE_2D);
	glColor4ub(r, g, b, a);
	glBegin(GL_LINES);
	glVertex2i(x, y);
	glVertex2i(x + w, y);

	glVertex2i(x + w - 1, y);
	glVertex2i(x + w - 1, y + h);

	glVertex2i(x + w, y + h - 1);
	glVertex2i(x, y + h - 1);

	glVertex2i(x, y + h);
	glVertex2i(x, y);
	glEnd();
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
void CVideo::DrawTransRectangle(Uint32 color, int x, int y,
	int w, int h, unsigned char alpha)
{
	GLubyte r, g, b;

	GetRGB(color, &r, &g, &b);
	color = MapRGBA(0, r, g, b, alpha);
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
void CVideo::DrawRectangleClip(Uint32 color, int x, int y,
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
void CVideo::DrawTransRectangleClip(Uint32 color, int x, int y,
	int w, int h, unsigned char alpha)
{
	GLubyte r, g, b;

	GetRGB(color, &r, &g, &b);
	color = MapRGBA(0, r, g, b, alpha);
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
void CVideo::FillRectangle(Uint32 color, int x, int y,
	int w, int h)
{
	GLubyte r, g, b, a;

	GetRGBA(color, &r, &g, &b, &a);
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
**  Draw translucent rectangle.
**
**  @param color  color
**  @param x      x coordinate on the screen
**  @param y      y coordinate on the screen
**  @param h      height of rectangle (0=don't draw).
**  @param w      width of rectangle (0=don't draw).
**  @param alpha  alpha value of pixel.
*/
void CVideo::FillTransRectangle(Uint32 color, int x, int y,
	int w, int h, unsigned char alpha)
{
	GLubyte r, g, b;

	GetRGB(color, &r, &g, &b);
	color = MapRGBA(0, r, g, b, alpha);
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
void CVideo::FillRectangleClip(Uint32 color, int x, int y,
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
void CVideo::FillTransRectangleClip(Uint32 color, int x, int y,
	int w, int h, unsigned char alpha)
{
	GLubyte r, g, b;

	GetRGB(color, &r, &g, &b);
	color = MapRGBA(0, r, g, b, alpha);
	FillRectangleClip(color, x, y, w, h);
}

/**
**  Draw circle clipped.
**
**  @param color   color
**  @param x       Center x coordinate on the screen
**  @param y       Center y coordinate on the screen
**  @param radius  radius of circle
*/
void CVideo::DrawCircleClip(Uint32 color, int x, int y, int radius)
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
**  Draw circle clipped.
**
**  @param color   color
**  @param x       Center x coordinate on the screen
**  @param y       Center y coordinate on the screen
**  @param radius  radius of circle
**  @param alpha   alpha value of pixels.
*/
void CVideo::DrawTransCircleClip(Uint32 color, int x, int y, int radius,
	unsigned char alpha)
{
	GLubyte r, g, b;
	
	GetRGB(color, &r, &g, &b);
	color = MapRGBA(0, r, g, b, alpha);
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
void CVideo::FillCircle(Uint32 color, int x, int y, int radius)
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
void CVideo::FillTransCircle(Uint32 color, int x, int y,
	int radius, unsigned char alpha)
{
	GLubyte r, g, b;
	
	GetRGB(color, &r, &g, &b);
	color = MapRGBA(0, r, g, b, alpha);
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
void CVideo::FillCircleClip(Uint32 color, int x, int y, int radius)
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
**  Fill circle clipped.
**
**  @param color   color
**  @param x       Center x coordinate on the screen
**  @param y       Center y coordinate on the screen
**  @param radius  radius of circle
**  @param alpha   alpha value of pixels.
*/
void CVideo::FillTransCircleClip(Uint32 color, int x, int y,
	int radius, unsigned char alpha)
{
	GLubyte r, g, b;
	
	GetRGB(color, &r, &g, &b);
	color = MapRGBA(0, r, g, b, alpha);
	FillCircleClip(color, x, y, radius);
}

/**
**  FIXME: docu
*/
void InitLineDraw(void)
{
}

#endif

//@}
