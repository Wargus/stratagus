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
/**@name sprite.c - The general sprite functions. */
//
//      (c) Copyright 2000-2004 by Lutz Sammer, Stephan Rasenberg,
//                                 Nehal Mistry, and Jimmy Salmon
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

#include "intern_video.h"

/*----------------------------------------------------------------------------
--  Functions
----------------------------------------------------------------------------*/

/**
**  Draw graphic object unclipped.
**
**  @param sprite  pointer to object
**  @param frame   number of frame (object index)
**  @param x       x coordinate on the screen
**  @param y       y coordinate on the screen
*/
#ifndef USE_OPENGL
global void VideoDraw(const Graphic* sprite, unsigned frame, int x, int y)
{
	VideoDrawSub(sprite,
		(frame % (sprite->Surface->w / sprite->Width)) * sprite->Width,
		(frame / (sprite->Surface->w / sprite->Width)) * sprite->Height,
		sprite->Width, sprite->Height, x, y);
}
#else
global void VideoDraw(const Graphic* sprite, unsigned frame, int x, int y)
{
	GLint sx;
	GLint ex;
	GLint sy;
	GLint ey;

	sx = x;
	ex = sx + sprite->Width;
	sy = y;
	ey = sy + sprite->Height;

	glBindTexture(GL_TEXTURE_2D, sprite->TextureNames[frame]);
	glBegin(GL_QUADS);
	glTexCoord2f(0.0f, 0.0f);
	glVertex2i(sx, sy);
	glTexCoord2f(0.0f, sprite->TextureHeight);
	glVertex2i(sx, ey);
	glTexCoord2f(sprite->TextureWidth, sprite->TextureHeight);
	glVertex2i(ex, ey);
	glTexCoord2f(sprite->TextureWidth, 0.0f);
	glVertex2i(ex, sy);
	glEnd();
}
#endif

/**
**  Draw graphic object clipped.
**
**  @param sprite  pointer to object
**  @param frame   number of frame (object index)
**  @param x       x coordinate on the screen
**  @param y       y coordinate on the screen
*/
#ifndef USE_OPENGL
global void VideoDrawClip(const Graphic* sprite, unsigned frame, int x, int y)
{
	VideoDrawSubClip(sprite,
		(frame % (sprite->Surface->w / sprite->Width)) * sprite->Width,
		(frame / (sprite->Surface->w / sprite->Width)) * sprite->Height,
		sprite->Width, sprite->Height, x, y);
}
#else
global void VideoDrawClip(const Graphic* sprite, unsigned frame, int x, int y)
{
	GLint svx;
	GLint evx;
	GLint svy;
	GLint evy;
	GLfloat stx;
	GLfloat etx;
	GLfloat sty;
	GLfloat ety;
	int ox;
	int oy;
	int ex;
	int w;
	int h;

	w = sprite->Width;
	h = sprite->Height;
	CLIP_RECTANGLE_OFS(x, y, w, h, ox, oy, ex);

	svx = x;
	evx = svx + w;
	svy = y;
	evy = svy + h;

	stx = (GLfloat)ox / sprite->Width * sprite->TextureWidth;
	etx = (GLfloat)(ox + w) / sprite->Width * sprite->TextureWidth;
	sty = (GLfloat)oy / sprite->Height * sprite->TextureHeight;
	ety = (GLfloat)(oy + h) / sprite->Height * sprite->TextureHeight;

	glBindTexture(GL_TEXTURE_2D, sprite->TextureNames[frame]);
	glBegin(GL_QUADS);
	glTexCoord2f(stx, sty);
	glVertex3f(svx, svy, 0.0f);
	glTexCoord2f(stx, ety);
	glVertex3f(svx, evy, 0.0f);
	glTexCoord2f(etx, ety);
	glVertex3f(evx, evy, 0.0f);
	glTexCoord2f(etx, sty);
	glVertex3f(evx, svy, 0.0f);
	glEnd();
}
#endif

/**
**  Draw graphic object unclipped and flipped in X direction.
**
**  @param sprite  pointer to object
**  @param frame   number of frame (object index)
**  @param x       x coordinate on the screen
**  @param y       y coordinate on the screen
*/
#ifndef USE_OPENGL
global void VideoDrawX(const Graphic* sprite, unsigned frame, int x, int y)
{
	SDL_Rect srect;
	SDL_Rect drect;

	srect.x = (sprite->SurfaceFlip->w - (frame % (sprite->SurfaceFlip->w /
			sprite->Width)) * sprite->Width) - sprite->Width;
	srect.y = (frame / (sprite->SurfaceFlip->w / sprite->Width)) * sprite->Height;
	srect.w = sprite->Width;
	srect.h = sprite->Height;

	drect.x = x;
	drect.y = y;

	SDL_BlitSurface(sprite->SurfaceFlip, &srect, TheScreen, &drect);
}
#else
global void VideoDrawX(const Graphic* sprite, unsigned frame, int x, int y)
{
	GLint sx;
	GLint ex;
	GLint sy;
	GLint ey;

	sx = x;
	ex = sx + sprite->Width;
	sy = y;
	ey = sy + sprite->Height;

	glBindTexture(GL_TEXTURE_2D, sprite->TextureNames[frame]);
	glBegin(GL_QUADS);
	glTexCoord2f(0.0f, 0.0f);
	glVertex2i(sx, sy);
	glTexCoord2f(0.0f, sprite->TextureHeight);
	glVertex2i(sx, ey);
	glTexCoord2f(sprite->TextureWidth, sprite->TextureHeight);
	glVertex2i(ex, ey);
	glTexCoord2f(sprite->TextureWidth, 0.0f);
	glVertex2i(ex, sy);
	glEnd();
}
#endif

/**
**  Draw graphic object clipped and flipped in X direction.
**
**  @param sprite  pointer to object
**  @param frame   number of frame (object index)
**  @param x       x coordinate on the screen
**  @param y       y coordinate on the screen
*/
#ifndef USE_OPENGL
global void VideoDrawClipX(const Graphic* sprite, unsigned frame, int x, int y)
{
	SDL_Rect srect;
	SDL_Rect drect;
	int oldx;
	int oldy;

	srect.x = (sprite->SurfaceFlip->w - (frame % (sprite->SurfaceFlip->w /
			sprite->Width)) * sprite->Width) - sprite->Width;
	srect.y = (frame / (sprite->SurfaceFlip->w / sprite->Width)) * sprite->Height;
	srect.w = sprite->Width;
	srect.h = sprite->Height;

	oldx = x;
	oldy = y;
	CLIP_RECTANGLE(x, y, srect.w, srect.h);
	srect.x += x - oldx;
	srect.y += y - oldy;

	drect.x = x;
	drect.y = y;

	SDL_BlitSurface(sprite->SurfaceFlip, &srect, TheScreen, &drect);
}
#else
global void VideoDrawClipX(const Graphic* sprite, unsigned frame,
	int x, int y)
{
	GLint svx;
	GLint evx;
	GLint svy;
	GLint evy;
	GLfloat stx;
	GLfloat etx;
	GLfloat sty;
	GLfloat ety;
	int ox;
	int oy;
	int ex;
	int w;
	int h;

	w = sprite->Width;
	h = sprite->Height;
	CLIP_RECTANGLE_OFS(x, y, w, h, ox, oy, ex);

	svx = x;
	evx = svx + w;
	svy = y;
	evy = svy + h;

	if (w < sprite->Width) {
		if (ox == 0) {
			ox += sprite->Width - w;
		} else {
			ox = 0;
		}
	}
	stx = (GLfloat)ox / sprite->Width * sprite->TextureWidth;
	etx = (GLfloat)(ox + w) / sprite->Width * sprite->TextureWidth;
	sty = (GLfloat)oy / sprite->Height * sprite->TextureHeight;
	ety = (GLfloat)(oy + h) / sprite->Height * sprite->TextureHeight;

	glBindTexture(GL_TEXTURE_2D, sprite->TextureNames[frame]);
	glBegin(GL_QUADS);
	glTexCoord2f(stx, sty);
	glVertex2i(evx, svy);
	glTexCoord2f(stx, ety);
	glVertex2i(evx, evy);
	glTexCoord2f(etx, ety);
	glVertex2i(svx, evy);
	glTexCoord2f(etx, sty);
	glVertex2i(svx, svy);
	glEnd();
}
#endif

#ifndef USE_OPENGL
global void VideoDrawTrans(const Graphic* sprite, unsigned frame, int x, int y, int alpha)
{
	VideoDrawSubTrans(sprite,
		(frame % (sprite->Surface->w / sprite->Width)) * sprite->Width,
		(frame / (sprite->Surface->w / sprite->Width)) * sprite->Height,
		sprite->Width, sprite->Height, x, y, alpha);
}

global void VideoDrawClipTrans(const Graphic* sprite, unsigned frame, int x, int y, int alpha)
{
	VideoDrawSubClipTrans(sprite,
		(frame % (sprite->Surface->w / sprite->Width)) * sprite->Width,
		(frame / (sprite->Surface->w / sprite->Width)) * sprite->Height,
		sprite->Width, sprite->Height, x, y, alpha);
}

global void VideoDrawTransX(const Graphic* sprite, unsigned frame, int x, int y, int alpha)
{
	SDL_Rect srect;
	SDL_Rect drect;
	int oldalpha;

	srect.x = (sprite->SurfaceFlip->w - (frame % (sprite->SurfaceFlip->w /
			sprite->Width)) * sprite->Width) - sprite->Width;
	srect.y = (frame / (sprite->SurfaceFlip->w / sprite->Width)) * sprite->Height;
	srect.w = sprite->Width;
	srect.h = sprite->Height;

	drect.x = x;
	drect.y = y;

	oldalpha = sprite->Surface->format->alpha;
	SDL_SetAlpha(sprite->Surface, SDL_SRCALPHA, alpha);
	SDL_BlitSurface(sprite->SurfaceFlip, &srect, TheScreen, &drect);
	SDL_SetAlpha(sprite->Surface, SDL_SRCALPHA, oldalpha);
}

global void VideoDrawClipTransX(const Graphic* sprite, unsigned frame, int x, int y, int alpha)
{
	SDL_Rect srect;
	SDL_Rect drect;
	int oldx;
	int oldy;
	int oldalpha;

	srect.x = (sprite->SurfaceFlip->w - (frame % (sprite->SurfaceFlip->w /
			sprite->Width)) * sprite->Width) - sprite->Width;
	srect.y = (frame / (sprite->SurfaceFlip->w / sprite->Width)) * sprite->Height;
	srect.w = sprite->Width;
	srect.h = sprite->Height;

	oldx = x;
	oldy = y;
	CLIP_RECTANGLE(x, y, srect.w, srect.h);
	srect.x += x - oldx;
	srect.y += y - oldy;

	drect.x = x;
	drect.y = y;

	oldalpha = sprite->Surface->format->alpha;
	SDL_SetAlpha(sprite->Surface, SDL_SRCALPHA, alpha);
	SDL_BlitSurface(sprite->SurfaceFlip, &srect, TheScreen, &drect);
	SDL_SetAlpha(sprite->Surface, SDL_SRCALPHA, oldalpha);
}
#else
global void VideoDrawTrans(const Graphic* sprite, unsigned frame, int x, int y, int alpha)
{
}
global void VideoDrawClipTrans(const Graphic* sprite, unsigned frame, int x, int y, int alpha)
{
}
global void VideoDrawTransX(const Graphic* sprite, unsigned frame, int x, int y, int alpha)
{
}
global void VideoDrawClipTransX(const Graphic* sprite, unsigned frame, int x, int y, int alpha)
{
}
#endif

local void FreeSprite(Graphic* graphic)
{
	int i;

#ifdef DEBUG_TODO
	AllocatedGraphicMemory -= graphic->Size;
	AllocatedGraphicMemory -= sizeof(Graphic);
#endif
#ifdef USE_OPENGL
	if (graphic->NumTextureNames) {
		glDeleteTextures(graphic->NumTextureNames, graphic->TextureNames);
		free(graphic->TextureNames);
	}
#endif
	for (i = 0; i < graphic->NumFrames; ++i) {
		SDL_FreeSurface(&graphic->Surface[i]);
	}
}

/*----------------------------------------------------------------------------
--  Global functions
----------------------------------------------------------------------------*/

/**
**  Load sprite from file.
**
**  @param name    File name of sprite to load.
**  @param width   Width of a single frame.
**  @param height  Height of a single frame.
**
**  @return        A graphic object for the loaded sprite.
**
**  @see LoadGraphic
*/
global Graphic* LoadSprite(const char* name, int width, int height)
{
	Graphic* g;

	g = LoadGraphic(name);

	if (!width) {
		width = g->Width;
	}
	if (!height) {
		height = g->Height;
	}

	DebugCheck(width > g->Width || height > g->Height);

	if ((g->Width / width) * width != g->Width ||
			(g->Height / height) * height != g->Height) {
		fprintf(stderr, "Invalid graphic (width, height) %s\n", name);
		fprintf(stderr, "Expected: (%d,%d)  Found: (%d,%d)\n",
			width, height, g->Width, g->Height);
		ExitFatal(1);
	}

#ifdef USE_OPENGL
	MakeTexture(g, width, height);
	g->GraphicWidth = g->Width;
	g->GraphicHeight = g->Height;
#endif

	g->NumFrames = g->Width / width * g->Height / height;;
	g->Width = width;
	g->Height = height;

	return g;
}

/**
**  Make shadow sprite
**
**  @param graphic  pointer to object
*/
global void MakeShadowSprite(Graphic* graphic)
{
	SDL_Color colors[256];

	// Set all colors in the palette to black and use 50% alpha
	memset(colors, 0, sizeof(colors));

	SDL_SetPalette(graphic->Surface, SDL_LOGPAL | SDL_PHYSPAL, colors, 0, 256);
	SDL_SetAlpha(graphic->Surface, SDL_SRCALPHA | SDL_RLEACCEL, 128);

	if (graphic->SurfaceFlip) {
		SDL_SetPalette(graphic->SurfaceFlip, SDL_LOGPAL | SDL_PHYSPAL, colors, 0, 256);
		SDL_SetAlpha(graphic->SurfaceFlip, SDL_SRCALPHA | SDL_RLEACCEL, 128);
	}
}

/**
**  Init sprite
*/
global void InitSprite(void)
{
}

//@}
