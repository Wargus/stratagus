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
/**@name graphic.c - The general graphic functions. */
//
//      (c) Copyright 1999-2004 by Lutz Sammer, Nehal Mistry, and Jimmy Salmon
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
--  Includes
----------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "stratagus.h"
#include "video.h"
#include "map.h"
#include "intern_video.h"

/*----------------------------------------------------------------------------
--  Variables
----------------------------------------------------------------------------*/

global PaletteLink* PaletteList;        /// List of all used palettes.

/*----------------------------------------------------------------------------
--  Functions
----------------------------------------------------------------------------*/

/**
**  Video draw part of graphic.
**
**  @param graphic  Pointer to object
**  @param gx       X offset into object
**  @param gy       Y offset into object
**  @param w        width to display
**  @param h        height to display
**  @param x        X screen position
**  @param y        Y screen position
*/
#ifndef USE_OPENGL
global void VideoDrawSub(const Graphic* graphic, int gx, int gy,
	int w, int h, int x, int y)
{
	SDL_Rect srect;
	SDL_Rect drect;

	srect.x = gx;
	srect.y = gy;
	srect.w = w;
	srect.h = h;

	drect.x = x;
	drect.y = y;

	SDL_BlitSurface(graphic->Surface, &srect, TheScreen, &drect);
}
#else
global void VideoDrawSub(const Graphic* graphic, int gx, int gy,
	int w, int h, int x, int y)
{
	int sx;
	int ex;
	int sy;
	int ey;
	GLfloat stx;
	GLfloat etx;
	GLfloat sty;
	GLfloat ety;

	sx = x;
	ex = sx + w;
	sy = y;
	ey = y + h;

	stx = (GLfloat)gx / graphic->Width * graphic->TextureWidth;
	etx = (GLfloat)(gx + w) / graphic->Width * graphic->TextureWidth;
	sty = (GLfloat)gy / graphic->Height * graphic->TextureHeight;
	ety = (GLfloat)(gy + h) / graphic->Height * graphic->TextureHeight;

	glBindTexture(GL_TEXTURE_2D, graphic->TextureNames[0]);
	glBegin(GL_QUADS);
	glTexCoord2f(stx, sty);
	glVertex2i(sx, sy);
	glTexCoord2f(stx, ety);
	glVertex2i(sx, ey);
	glTexCoord2f(etx, ety);
	glVertex2i(ex, ey);
	glTexCoord2f(etx, sty);
	glVertex2i(ex, sy);
	glEnd();
}
#endif

/**
**  Video draw part of graphic clipped.
**
**  @param graphic  Pointer to object
**  @param gx       X offset into object
**  @param gy       Y offset into object
**  @param w        width to display
**  @param h        height to display
**  @param x        X screen position
**  @param y        Y screen position
*/
global void VideoDrawSubClip(const Graphic* graphic, int gx, int gy,
	int w, int h, int x, int y)
{
	int oldx;
	int oldy;

	oldx = x;
	oldy = y;
	CLIP_RECTANGLE(x, y, w, h);
	VideoDrawSub(graphic, gx + x - oldx, gy + y - oldy, w, h, x, y);
}

/**
**  Video draw part of graphic with alpha.
**
**  @param graphic  Pointer to object
**  @param gx       X offset into object
**  @param gy       Y offset into object
**  @param w        width to display
**  @param h        height to display
**  @param x        X screen position
**  @param y        Y screen position
**  @param alpha    Alpha
*/
#ifndef USE_OPENGL
global void VideoDrawSubTrans(const Graphic* graphic, int gx, int gy,
	int w, int h, int x, int y, unsigned char alpha)
{
	int oldalpha;

	oldalpha = graphic->Surface->format->alpha;
	SDL_SetAlpha(graphic->Surface, SDL_SRCALPHA, alpha);
	VideoDrawSub(graphic, gx, gy, w, h, x, y);
	SDL_SetAlpha(graphic->Surface, SDL_SRCALPHA, oldalpha);
}
#else
global void VideoDrawSubTrans(const Graphic* graphic, int gx, int gy,
	int w, int h, int x, int y, unsigned char alpha)
{
	// FIXME: not done
	VideoDrawSub(graphic, gx, gy, w, h, x, y);
}
#endif

/**
**  Video draw part of graphic with alpha and clipped.
**
**  @param graphic  Pointer to object
**  @param gx       X offset into object
**  @param gy       Y offset into object
**  @param w        width to display
**  @param h        height to display
**  @param x        X screen position
**  @param y        Y screen position
**  @param alpha    Alpha
*/
global void VideoDrawSubClipTrans(const Graphic* graphic, int gx, int gy,
	int w, int h, int x, int y, unsigned char alpha)
{
	int oldx;
	int oldy;

	oldx = x;
	oldy = y;
	CLIP_RECTANGLE(x, y, w, h);
	VideoDrawSubTrans(graphic, gx + x - oldx, gy + y - oldy, w, h, x, y, alpha);
}

/**
**  Free graphic object.
*/
global void VideoFree(Graphic* graphic)
{
#ifdef USE_OPENGL
	if (graphic->NumTextureNames) {
		glDeleteTextures(graphic->NumTextureNames, graphic->TextureNames);
		free(graphic->TextureNames);
	}
#endif

	if (graphic->Surface) {
		if (graphic->Surface->format->BytesPerPixel == 1) {
			VideoPaletteListRemove(graphic->Surface);
		}
		SDL_FreeSurface(graphic->Surface);
	}
	if (graphic->SurfaceFlip) {
		if (graphic->SurfaceFlip->format->BytesPerPixel == 1) {
			VideoPaletteListRemove(graphic->SurfaceFlip);
		}
		SDL_FreeSurface(graphic->SurfaceFlip);
	}
	free(graphic);
}

/*----------------------------------------------------------------------------
--  Global functions
----------------------------------------------------------------------------*/

/**
**  Make a graphic object.
**
**  @param surface FIXME:Docu
**
**  @todo docu
**  @return        New graphic object (malloced).
*/
global Graphic* MakeGraphic(SDL_Surface* surface)
{
	Graphic* graphic;

	graphic = calloc(1, sizeof(Graphic));
	if (!graphic) {
		fprintf(stderr, "Out of memory\n");
		ExitFatal(-1);
	}

	graphic->Surface = surface;
	graphic->Width = surface->w;
	graphic->Height = surface->h;
	graphic->NumFrames = 1;

	return graphic;
}

/**
**  Flip graphic and store in graphic->SurfaceFlip
**
**  @param g  Pointer to object
*/
global void FlipGraphic(Graphic* g)
{
#ifdef USE_OPENGL
	return;
#else
	int i;
	int j;
	SDL_Surface* s;

	s = g->SurfaceFlip = SDL_ConvertSurface(g->Surface,
		g->Surface->format, SDL_SWSURFACE);
	if (g->Surface->flags & SDL_SRCCOLORKEY) {
		SDL_SetColorKey(g->SurfaceFlip, SDL_SRCCOLORKEY | SDL_RLEACCEL,
			g->Surface->format->colorkey);
	}
	if (g->SurfaceFlip->format->BytesPerPixel == 1) {
		VideoPaletteListAdd(g->SurfaceFlip);
	}

	SDL_LockSurface(g->Surface);
	SDL_LockSurface(s);
	for (i = 0; i < s->h; ++i) {
		for (j = 0; j < s->w; ++j) {
			((char*)s->pixels)[j + i * s->pitch] =
				((char*)g->Surface->pixels)[s->w - j - 1 + i * g->Surface->pitch];
		}
	}
	SDL_UnlockSurface(g->Surface);
	SDL_UnlockSurface(s);
#endif
}

/**
**  Make an OpenGL texture or textures out of a graphic object.
**
**  @param graphic  The graphic object.
**  @param width    Graphic width.
**  @param height   Graphic height.
*/
#ifdef USE_OPENGL
global void MakeTexture(Graphic* graphic, int width, int height)
{
	int i;
	int j;
	int h;
	int w;
	int x;
	int n;
	unsigned char* tex;
	const unsigned char* sp;
	int fl;
	Uint32 ckey;
	int useckey;
	int bpp;
	int size;
	unsigned char alpha;

	glGetIntegerv(GL_MAX_TEXTURE_SIZE, &size);
	if (width > size || height > size) {
		DebugPrint("Image too large (%d,%d), max size: %d\n" _C_
			width _C_ height _C_ size);
		return;
	}

	n = (graphic->Width / width) * (graphic->Height / height);
	fl = graphic->Width / width;
	bpp = graphic->Surface->format->BytesPerPixel;
	useckey = graphic->Surface->flags & SDL_SRCCOLORKEY;
	ckey = graphic->Surface->format->colorkey;

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	graphic->NumTextureNames = n;
	graphic->TextureNames = (GLuint*)malloc(n * sizeof(GLuint));
	glGenTextures(n, graphic->TextureNames);
	for (i = 1; i < width; i <<= 1) {
	}
	w = i;
	for (i = 1; i < height; i <<= 1) {
	}
	h = i;
	graphic->TextureWidth = (float)width / w;
	graphic->TextureHeight = (float)height / h;
	tex = (unsigned char*)malloc(w * h * 4);
	if (graphic->Surface->flags & SDL_SRCALPHA) {
		alpha = graphic->Surface->format->alpha;
	} else {
		alpha = 0xff;
	}

	for (x = 0; x < n; ++x) {
		glBindTexture(GL_TEXTURE_2D, graphic->TextureNames[x]);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		SDL_LockSurface(graphic->Surface);
		for (i = 0; i < height; ++i) {
			sp = (const unsigned char*)graphic->Surface->pixels + (x % fl) * width * bpp +
				((x / fl) * height + i) * graphic->Surface->pitch;
			for (j = 0; j < width; ++j) {
				int c;
				SDL_Color p;

				c = i * w * 4 + j * 4;
				if (bpp == 1) {
					if (useckey && *sp == ckey) {
						tex[c + 3] = 0;
					} else {
						p = graphic->Surface->format->palette->colors[*sp];
						tex[c + 0] = p.r;
						tex[c + 1] = p.g;
						tex[c + 2] = p.b;
						tex[c + 3] = alpha;
					}
					++sp;
				} else {
					tex[c + 0] = *sp++;
					tex[c + 1] = *sp++;
					tex[c + 2] = *sp++;
					if (bpp == 4) {
						tex[c + 3] = *sp++;
					} else {
						tex[c + 3] = alpha;
					}
				}
			}
		}
		SDL_UnlockSurface(graphic->Surface);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, tex);
#ifdef DEBUG
		if ((i = glGetError())) {
			DebugPrint("glTexImage2D(%x)\n" _C_ i);
		}
#endif
	}
	free(tex);
}

/**
**  Make an OpenGL texture of the player color pixels only.
**
**  @param g        FIXME: docu
**  @param graphic  FIXME: docu
**  @param frame    FIXME: docu
**  @param map      FIXME: docu
**  @param maplen   FIXME: docu
*/
global void MakePlayerColorTexture(Graphic** g, Graphic* graphic, int frame,
	UnitColors* colors)
{
	int i;
	int j;
	int h;
	int w;
	int n;
	unsigned char* tex;
	const unsigned char* sp;
	int fl;
	int bpp;

	if (!*g) {
		*g = calloc(1, sizeof(Graphic));

		n = graphic->NumFrames;

		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
		(*g)->NumTextureNames = n;
		(*g)->TextureNames = calloc(n,sizeof(GLuint));

		(*g)->Width = graphic->Width;
		(*g)->Height = graphic->Height;
		(*g)->GraphicWidth = graphic->GraphicWidth;
		(*g)->GraphicHeight = graphic->GraphicHeight;
		(*g)->TextureWidth = graphic->TextureWidth;
		(*g)->TextureHeight = graphic->TextureHeight;
	}

	fl = graphic->GraphicWidth / graphic->Width;
	bpp = graphic->Surface->format->BytesPerPixel;
	glGenTextures(1, &(*g)->TextureNames[frame]);

	for (i = 1; i < graphic->Width; i <<= 1) {
	}
	w = i;
	for (i = 1; i < graphic->Height; i <<= 1) {
	}
	h = i;

	tex = (unsigned char*)malloc(w * h * 4);

	glBindTexture(GL_TEXTURE_2D, (*g)->TextureNames[frame]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	SDL_LockSurface(graphic->Surface);
	for (i = 0; i < graphic->Height; ++i) {
		sp = (const unsigned char*)graphic->Surface->pixels + (frame % fl) * graphic->Width * bpp +
			((frame / fl) * graphic->Height + i) * graphic->Surface->pitch;
		for (j = 0; j < graphic->Width; ++j) {
			int c;
			int z;
			SDL_Color p;

			c = i * w * 4 + j * 4;
			if (bpp == 1) {
				for (z = 0; z < 4; ++z) {
					if (*sp == 208 + z) {
						p = colors->Colors[z];
						tex[c + 0] = p.r;
						tex[c + 1] = p.g;
						tex[c + 2] = p.b;
						tex[c + 3] = 0xff;
						break;
					}
				}
				if (z == 4) {
					tex[c + 3] = 0;
				}
				++sp;
			} else {
				// FIXME: not done
				sp += bpp;
			}
		}
	}
	SDL_UnlockSurface(graphic->Surface);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, tex);
#ifdef DEBUG
	if ((i = glGetError())) {
		DebugPrint("glTexImage2D(%x)\n" _C_ i);
	}
#endif
	free(tex);
}
#endif

/**
**  Resize a graphic
**
**  @param g  Graphic object.
**  @param w  New width of graphic.
**  @param h  New height of graphic.
**
**  @todo FIXME: Higher quality resizing.
**        FIXME: Works only with 8bit indexed graphic objects.
*/
global void ResizeGraphic(Graphic* g, int w, int h)
{
	int i;
	int j;
	unsigned char* data;
	int x;
	SDL_Color pal[256];
	Uint32 ckey;
	int useckey;

	// FIXME: Support more formats
	if (g->Surface->format->BytesPerPixel != 1) {
		return;
	}

	if (g->Width == w && g->Height == h) {
		return;
	}

	SDL_LockSurface(g->Surface);

	data = (unsigned char*)malloc(w * h);
	x = 0;

	for (i = 0; i < h; ++i) {
		for (j = 0; j < w; ++j) {
			data[x] = ((unsigned char*)g->Surface->pixels)[
				(i * g->Height / h) * g->Surface->pitch + j * g->Width / w];
			++x;
		}
	}

	SDL_UnlockSurface(g->Surface);
	memcpy(pal, g->Surface->format->palette->colors, sizeof(SDL_Color) * 256);
	if (g->Surface->format->BytesPerPixel == 1) {
		VideoPaletteListRemove(g->Surface);
	}
	useckey = g->Surface->flags & SDL_SRCCOLORKEY;
	ckey = g->Surface->format->colorkey;
	SDL_FreeSurface(g->Surface);

	g->Surface = SDL_CreateRGBSurfaceFrom(data, w, h, 8, w, 0, 0, 0, 0);
	if (g->Surface->format->BytesPerPixel == 1) {
		VideoPaletteListAdd(g->Surface);
	}
	SDL_SetPalette(g->Surface, SDL_LOGPAL | SDL_PHYSPAL, pal, 0, 256);
	if (useckey) {
		SDL_SetColorKey(g->Surface, SDL_SRCCOLORKEY | SDL_RLEACCEL, ckey);
	}

	g->Width = w;
	g->Height = h;

#ifdef USE_OPENGL
	glDeleteTextures(g->NumTextureNames, g->TextureNames);
	free(g->TextureNames);
	MakeTexture(g, g->Width, g->Height);
#endif
}

//@}
