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
--  Includes
----------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "stratagus.h"
#include "video.h"
#include "iolib.h"
#include "iocompat.h"
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
#ifdef DEBUG
	AllocatedGraphicMemory -= sizeof(Graphic);
#endif

#ifdef USE_OPENGL
	if (graphic->NumTextureNames) {
		glDeleteTextures(graphic->NumTextureNames, graphic->TextureNames);
		free(graphic->TextureNames);
	}
#endif

	if (graphic->Surface) {
#ifdef DEBUG
		AllocatedGraphicMemory -=
			graphic->Width * graphic->Height * graphic->Surface->format->BytesPerPixel;
#endif
		VideoPaletteListRemove(graphic->Surface);
		SDL_FreeSurface(graphic->Surface);
	}
	if (graphic->SurfaceFlip) {
#ifdef DEBUG
		AllocatedGraphicMemory -=
			graphic->Width * graphic->Height * graphic->SurfaceFlip->format->BytesPerPixel;
#endif
		VideoPaletteListRemove(graphic->SurfaceFlip);
		SDL_FreeSurface(graphic->SurfaceFlip);
	}
	free(graphic->Data);
	free(graphic);
}

/*----------------------------------------------------------------------------
--  Global functions
----------------------------------------------------------------------------*/

/**
**  Make a graphic object.
**
**  @param depth   Pixel depth of the object (8,16,32)
**  @param width   Pixel width.
**  @param height  Pixel height.
**  @param data    Object data (malloced by caller, freed from object).
**  @param size    Size in bytes of the object data.
**
**  @return        New graphic object (malloced).
*/
global Graphic* MakeGraphic(unsigned depth, int width, int height,
	void* data, unsigned size)
{
	Graphic* graphic;

	graphic = malloc(sizeof(Graphic));
#ifdef DEBUG
	AllocatedGraphicMemory += sizeof(Graphic);
#endif

	if (!graphic) {
		fprintf(stderr, "Out of memory\n");
		ExitFatal(-1);
	}
	graphic->Width = width;
	graphic->Height = height;
	graphic->Data = data;

	// FIXME: endian

	graphic->Surface = SDL_CreateRGBSurfaceFrom(data, width, height, depth, width * depth / 8,
		0, 0, 0, 0);
	graphic->SurfaceFlip = NULL;
	graphic->NumFrames = 0;

#ifdef USE_OPENGL
	graphic->NumTextureNames = 0;
#endif

	return graphic;
}

/**
**  Flip graphic and store in graphic->SurfaceFlip
**
**  @param graphic  Pointer to object
*/
global void FlipGraphic(Graphic* graphic)
{
#ifdef USE_OPENGL
	return;
#else
	int i;
	int j;
	SDL_Surface* s;

	s = graphic->SurfaceFlip = SDL_ConvertSurface(graphic->Surface,
		graphic->Surface->format, SDL_SWSURFACE);
	VideoPaletteListAdd(graphic->SurfaceFlip);
#ifdef DEBUG
	AllocatedGraphicMemory +=
		graphic->Width * graphic->Height * graphic->Surface->format->BytesPerPixel;
#endif

	SDL_LockSurface(graphic->Surface);
	SDL_LockSurface(s);
	for (i = 0; i < s->h; ++i) {
		for (j = 0; j < s->w; ++j) {
			((char*)s->pixels)[j + i * s->pitch] =
				((char*)graphic->Surface->pixels)[s->w - j - 1 + i * graphic->Surface->pitch];
		}
	}
	SDL_UnlockSurface(graphic->Surface);
	SDL_UnlockSurface(s);
#endif
}

/**
**  Make a new graphic object.
**
**  @param depth   Pixel depth of the object (8,16,32)
**  @param width   Pixel width.
**  @param height  Pixel height.
*/
global Graphic* NewGraphic(unsigned depth, int width, int height)
{
	void* data;
	int size;

	size = width * height * (depth + 7) / 8;
	data = malloc(size);
#ifdef DEBUG
	AllocatedGraphicMemory += size;
#endif

	return MakeGraphic(depth, width, height, data, size);
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

	n = (graphic->Width / width) * (graphic->Height / height);
	fl = graphic->Width / width;

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
	for (x = 0; x < n; ++x) {
		glBindTexture(GL_TEXTURE_2D, graphic->TextureNames[x]);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		SDL_LockSurface(graphic->Surface);
		for (i = 0; i < height; ++i) {
			sp = (const unsigned char*)graphic->Surface->pixels + (x % fl) * width +
				((x / fl) * height + i) * graphic->Width;
			for (j = 0; j < width; ++j) {
				int c;
				SDL_Color p;

				c = i * w * 4 + j * 4;
				if (*sp == 255) {
					tex[c + 3] = 0;
				} else {
					p = graphic->Surface->format->palette->colors[*sp];
					tex[c + 0] = p.r;
					tex[c + 1] = p.g;
					tex[c + 2] = p.b;
					tex[c + 3] = 0xff;
				}
				++sp;
			}
		}
		SDL_UnlockSurface(graphic->Surface);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, tex);
#ifdef DEBUG
		i = glGetError();
		if (i) {
			DebugLevel0Fn("glTexImage2D(%x)\n" _C_ i);
		}
#endif
	}
	free(tex);
}

/**
**  Make an OpenGL texture of the player color pixels only.
**
**  FIXME: docu
*/
global void MakePlayerColorTexture(Graphic** g, Graphic* graphic, int frame,
		unsigned char* map, int maplen)
{
	int i;
	int j;
	int h;
	int w;
	int n;
	unsigned char* tex;
	const unsigned char* sp;
	int fl;

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
		sp = (const unsigned char*)graphic->Surface->pixels + (frame % fl) * graphic->Width +
			((frame / fl) * graphic->Height + i) * graphic->GraphicWidth;
		for (j = 0; j < graphic->Width; ++j) {
			int c;
			int z;
			SDL_Color p;

			c = i * w * 4 + j * 4;
			for (z = 0; z < maplen; ++z) {
				if (*sp == map[z * 2]) {
					p = TheMap.TileGraphic->Surface->format->palette->colors[map[z * 2 + 1]];
					tex[c + 0] = p.r;
					tex[c + 1] = p.g;
					tex[c + 2] = p.b;
					tex[c + 3] = 0xff;
					break;
				}
			}
			if (z == maplen) {
				tex[c + 3] = 0;
			}
			++sp;
		}
	}
	SDL_UnlockSurface(graphic->Surface);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, tex);
#ifdef DEBUG
	i = glGetError();
	if (i) {
		DebugLevel0Fn("glTexImage2D(%x)\n" _C_ i);
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
global void ResizeGraphic(Graphic *g, int w, int h)
{
	int i;
	int j;
	unsigned char* data;
	int x;
	SDL_Color pal[256];

	DebugCheck(g->Surface->format->BytesPerPixel != 1);
	if (g->Width == w && g->Height == h) {
		return;
	}

	SDL_LockSurface(g->Surface);

	data = (unsigned char*)malloc(w * h);
#ifdef DEBUG
	AllocatedGraphicMemory += w * h;
#endif
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
	VideoPaletteListRemove(g->Surface);
	SDL_FreeSurface(g->Surface);

	g->Surface = SDL_CreateRGBSurfaceFrom(data, w, h, 8, w, 0, 0, 0, 0);
	VideoPaletteListAdd(g->Surface);
	SDL_SetPalette(g->Surface, SDL_LOGPAL | SDL_PHYSPAL, pal, 0, 256);
	SDL_SetColorKey(g->Surface, SDL_SRCCOLORKEY | SDL_RLEACCEL, 255);

	g->Width = w;
	g->Height = h;
	g->Data = data;
}

/**
**  Load graphic from file.
**
**  @param name  File name.
**
**  @return      Graphic object.
*/
global Graphic* LoadGraphic(const char* name)
{
	Graphic* graphic;
	char buf[PATH_MAX];

	// TODO: More formats?
	if (!(graphic = LoadGraphicPNG(LibraryFileName(name, buf)))) {
		fprintf(stderr, "Can't load the graphic `%s'\n", name);
		ExitFatal(-1);
	}

	graphic->NumFrames = 1;
	VideoPaletteListAdd(graphic->Surface);

	return graphic;
}

/**
**  Init graphic
*/
global void InitGraphic(void)
{
}

//@}
