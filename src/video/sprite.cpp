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
//      (c) Copyright 2000-2005 by Lutz Sammer, Stephan Rasenberg,
//                                 Nehal Mistry, and Jimmy Salmon
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
#include "player.h"
#include "iocompat.h"
#include "iolib.h"

#include "intern_video.h"

/*----------------------------------------------------------------------------
--  Functions
----------------------------------------------------------------------------*/

#ifdef USE_OPENGL
void DrawTexture(const Graphic* g, GLuint* textures, int sx, int sy,
	int ex, int ey, int x, int y, int flip)
{
	GLfloat stx, etx;
	GLfloat sty, ety;
	int texture;
	int minw, minh;
	int maxw, maxh;
	int i, j;
	int tw, th;
	int sx2, sy2;
	int ex2, ey2;
	int w, h;
	int x2, y2;
	int nextsx2, nextsy2;

	tw = ex / GLMaxTextureSize - sx / GLMaxTextureSize + 1;
	th = ey / GLMaxTextureSize - sy / GLMaxTextureSize + 1;

	x2 = x;
	y2 = y;

	sy2 = sy;
	for (j = 0; j < th; ++j) {
		minh = sy2 / GLMaxTextureSize * GLMaxTextureSize;
		maxh = minh + GLMaxTextureSize > g->GraphicHeight ?
			g->GraphicHeight : minh + GLMaxTextureSize;
		if (sy > minh) {
			h = ey - sy;
		} else {
			h = ey - minh;
		}
		if (h > maxh) {
			h = maxh;
		}

		sx2 = sx;
		for (i = 0; i < tw; ++i) {
			minw = sx2 / GLMaxTextureSize * GLMaxTextureSize;
			maxw = minw + GLMaxTextureSize > g->GraphicWidth ?
				g->GraphicWidth : minw + GLMaxTextureSize;
			if (sx > minw) {
				w = ex - sx;
			} else {
				w = ex - minw;
			}
			if (w > maxw) {
				w = maxw;
			}

			stx = (GLfloat)(sx2 - minw) / (maxw - minw);
			sty = (GLfloat)(sy2 - minh) / (maxh - minh);
			if (ex > maxw) {
				ex2 = maxw;
			} else {
				ex2 = ex;
			}
			etx = (GLfloat)(ex2 - sx2);
			if (maxw == g->GraphicWidth) {
				stx *= g->TextureWidth;
				etx = stx + etx / (maxw - minw) * g->TextureWidth;
			} else {
				etx = stx + (etx / GLMaxTextureSize);
			}
			if (ey > maxh) {
				ey2 = maxh;
			} else {
				ey2 = ey;
			}
			ety = (GLfloat)(ey2 - sy2);
			if (maxh == g->GraphicHeight) {
				sty *= g->TextureHeight;
				ety = sty + ety / (maxh - minh) * g->TextureHeight;
			} else {
				ety = sty + (ety / GLMaxTextureSize);
			}

			texture = sy2 / GLMaxTextureSize * (g->GraphicWidth / GLMaxTextureSize + 1) +
				sx2 / GLMaxTextureSize;

			glBindTexture(GL_TEXTURE_2D, textures[texture]);
			glBegin(GL_QUADS);
			if (!flip) {
				glTexCoord2f(stx, sty);
				glVertex2i(x2, y2);
				glTexCoord2f(stx, ety);
				glVertex2i(x2, y2 + h);
				glTexCoord2f(etx, ety);
				glVertex2i(x2 + w, y2 + h);
				glTexCoord2f(etx, sty);
				glVertex2i(x2 + w, y2);
			} else {
				glTexCoord2f(stx, sty);
				glVertex2i(x + (ex - sx) - (x2 - x), y2);
				glTexCoord2f(stx, ety);
				glVertex2i(x + (ex - sx) - (x2 - x), y2 + h);
				glTexCoord2f(etx, ety);
				glVertex2i(x + (ex - sx) - (x2 + w - x), y2 + h);
				glTexCoord2f(etx, sty);
				glVertex2i(x + (ex - sx) - (x2 + w - x), y2);
			}
			glEnd();

			nextsx2 = (sx2 + GLMaxTextureSize) / GLMaxTextureSize * GLMaxTextureSize;
			x2 += nextsx2 - sx2;
			sx2 = nextsx2;
		}

		nextsy2 = (sy2 + GLMaxTextureSize) / GLMaxTextureSize * GLMaxTextureSize;
		y2 += nextsy2 - sy2;
		sy2 = nextsy2;
	}
}
#endif

/**
**  Draw graphic object unclipped.
**
**  @param g       pointer to object
**  @param frame   number of frame (object index)
**  @param x       x coordinate on the screen
**  @param y       y coordinate on the screen
*/
#ifndef USE_OPENGL
void VideoDraw(const Graphic* g, unsigned frame, int x, int y)
{
	VideoDrawSub(g,
		(frame % (g->Surface->w / g->Width)) * g->Width,
		(frame / (g->Surface->w / g->Width)) * g->Height,
		g->Width, g->Height, x, y);
}
#else
void VideoDraw(const Graphic* g, unsigned frame, int x, int y)
{
	int sx, sy, ex, ey, n;

	n = g->GraphicWidth / g->Width;
	sx = (frame % n) * g->Width;
	ex = sx + g->Width;
	sy = (frame / n) * g->Height;
	ey = sy + g->Height;
	DrawTexture(g, g->Textures, sx, sy, ex, ey, x, y, 0);
}
#endif

/**
**  Draw graphic object clipped.
**
**  @param g       pointer to object
**  @param frame   number of frame (object index)
**  @param x       x coordinate on the screen
**  @param y       y coordinate on the screen
*/
#ifndef USE_OPENGL
void VideoDrawClip(const Graphic* g, unsigned frame, int x, int y)
{
	VideoDrawSubClip(g,
		(frame % (g->Surface->w / g->Width)) * g->Width,
		(frame / (g->Surface->w / g->Width)) * g->Height,
		g->Width, g->Height, x, y);
}
#else
void VideoDoDrawClip(const Graphic* g, GLuint* textures,
	unsigned frame, int x, int y)
{
	int ox;
	int oy;
	int skip;
	int w;
	int h;
	int sx, sy, ex, ey, n;

	n = g->GraphicWidth / g->Width;
	sx = (frame % n) * g->Width;
	ex = sx + g->Width;
	sy = (frame / n) * g->Height;
	ey = sy + g->Height;

	w = g->Width;
	h = g->Height;
	CLIP_RECTANGLE_OFS(x, y, w, h, ox, oy, skip);

	DrawTexture(g, textures, sx + ox, sy + oy, sx + ox + w, sy + oy + h, x, y, 0);
}
#endif

/**
**  Draw graphic object clipped and with player colors.
**
**  @param g       pointer to object
**  @param player  player number
**  @param frame   number of frame (object index)
**  @param x       x coordinate on the screen
**  @param y       y coordinate on the screen
*/
void VideoDrawPlayerColorClip(Graphic* g, int player,
	unsigned frame, int x, int y)
{
#ifndef USE_OPENGL
	GraphicPlayerPixels(&Players[player], g);
	VideoDrawClip(g, frame, x, y);
#else
	if (!g->PlayerColorTextures[player]) {
		MakePlayerColorTexture(g, player);
	}
	VideoDoDrawClip(g, g->PlayerColorTextures[player], frame, x, y);
#endif
}

/**
**  Draw graphic object clipped, flipped, and with player colors.
**
**  @param g       pointer to object
**  @param player  player number
**  @param frame   number of frame (object index)
**  @param x       x coordinate on the screen
**  @param y       y coordinate on the screen
*/
void VideoDrawPlayerColorClipX(Graphic* g, int player,
	unsigned frame, int x, int y)
{
#ifndef USE_OPENGL
	GraphicPlayerPixels(&Players[player], g);
	VideoDrawClipX(g, frame, x, y);
#else
	if (!g->PlayerColorTextures[player]) {
		MakePlayerColorTexture(g, player);
	}
	VideoDoDrawClipX(g, g->PlayerColorTextures[player], frame, x, y);
#endif
}

/**
**  Draw graphic object unclipped and flipped in X direction.
**
**  @param g       pointer to object
**  @param frame   number of frame (object index)
**  @param x       x coordinate on the screen
**  @param y       y coordinate on the screen
*/
#ifndef USE_OPENGL
void VideoDrawX(const Graphic* g, unsigned frame, int x, int y)
{
	SDL_Rect srect;
	SDL_Rect drect;

	srect.x = (g->SurfaceFlip->w - (frame % (g->SurfaceFlip->w /
			g->Width)) * g->Width) - g->Width;
	srect.y = (frame / (g->SurfaceFlip->w / g->Width)) * g->Height;
	srect.w = g->Width;
	srect.h = g->Height;

	drect.x = x;
	drect.y = y;

	SDL_BlitSurface(g->SurfaceFlip, &srect, TheScreen, &drect);
}
#else
void VideoDrawX(const Graphic* g, unsigned frame, int x, int y)
{
	int sx, sy, ex, ey, n;

	n = g->GraphicWidth / g->Width;
	sx = (frame % n) * g->Width;
	ex = sx + g->Width;
	sy = (frame / n) * g->Height;
	ey = sy + g->Height;
	DrawTexture(g, g->Textures, sx, sy, ex, ey, x, y, 1);
}
#endif

/**
**  Draw graphic object clipped and flipped in X direction.
**
**  @param g       pointer to object
**  @param frame   number of frame (object index)
**  @param x       x coordinate on the screen
**  @param y       y coordinate on the screen
*/
#ifndef USE_OPENGL
void VideoDrawClipX(const Graphic* g, unsigned frame, int x, int y)
{
	SDL_Rect srect;
	SDL_Rect drect;
	int oldx;
	int oldy;

	srect.x = (g->SurfaceFlip->w - (frame % (g->SurfaceFlip->w /
			g->Width)) * g->Width) - g->Width;
	srect.y = (frame / (g->SurfaceFlip->w / g->Width)) * g->Height;
	srect.w = g->Width;
	srect.h = g->Height;

	oldx = x;
	oldy = y;
	CLIP_RECTANGLE(x, y, srect.w, srect.h);
	srect.x += x - oldx;
	srect.y += y - oldy;

	drect.x = x;
	drect.y = y;

	SDL_BlitSurface(g->SurfaceFlip, &srect, TheScreen, &drect);
}
#else
void VideoDoDrawClipX(const Graphic* g, GLuint* textures,
	unsigned frame, int x, int y)
{
	int ox;
	int oy;
	int skip;
	int w;
	int h;
	int sx, sy, ex, ey, n;

	n = g->GraphicWidth / g->Width;
	sx = (frame % n) * g->Width;
	ex = sx + g->Width;
	sy = (frame / n) * g->Height;
	ey = sy + g->Height;

	w = g->Width;
	h = g->Height;
	CLIP_RECTANGLE_OFS(x, y, w, h, ox, oy, skip);

	if (w < g->Width) {
		if (ox == 0) {
			ox += g->Width - w;
		} else {
			ox = 0;
		}
	}

	DrawTexture(g, textures, sx + ox, sy + oy, sx + ox + w, sy + oy + h, x, y, 1);
}
#endif

#ifndef USE_OPENGL
void VideoDrawTrans(const Graphic* g, unsigned frame, int x, int y, int alpha)
{
	VideoDrawSubTrans(g,
		(frame % (g->Surface->w / g->Width)) * g->Width,
		(frame / (g->Surface->w / g->Width)) * g->Height,
		g->Width, g->Height, x, y, alpha);
}

void VideoDrawClipTrans(const Graphic* g, unsigned frame, int x, int y, int alpha)
{
	VideoDrawSubClipTrans(g,
		(frame % (g->Surface->w / g->Width)) * g->Width,
		(frame / (g->Surface->w / g->Width)) * g->Height,
		g->Width, g->Height, x, y, alpha);
}

void VideoDrawTransX(const Graphic* g, unsigned frame, int x, int y, int alpha)
{
	SDL_Rect srect;
	SDL_Rect drect;
	int oldalpha;

	srect.x = (g->SurfaceFlip->w - (frame % (g->SurfaceFlip->w /
			g->Width)) * g->Width) - g->Width;
	srect.y = (frame / (g->SurfaceFlip->w / g->Width)) * g->Height;
	srect.w = g->Width;
	srect.h = g->Height;

	drect.x = x;
	drect.y = y;

	oldalpha = g->Surface->format->alpha;
	SDL_SetAlpha(g->Surface, SDL_SRCALPHA, alpha);
	SDL_BlitSurface(g->SurfaceFlip, &srect, TheScreen, &drect);
	SDL_SetAlpha(g->Surface, SDL_SRCALPHA, oldalpha);
}

void VideoDrawClipTransX(const Graphic* g, unsigned frame, int x, int y, int alpha)
{
	SDL_Rect srect;
	SDL_Rect drect;
	int oldx;
	int oldy;
	int oldalpha;

	srect.x = (g->SurfaceFlip->w - (frame % (g->SurfaceFlip->w /
			g->Width)) * g->Width) - g->Width;
	srect.y = (frame / (g->SurfaceFlip->w / g->Width)) * g->Height;
	srect.w = g->Width;
	srect.h = g->Height;

	oldx = x;
	oldy = y;
	CLIP_RECTANGLE(x, y, srect.w, srect.h);
	srect.x += x - oldx;
	srect.y += y - oldy;

	drect.x = x;
	drect.y = y;

	oldalpha = g->Surface->format->alpha;
	SDL_SetAlpha(g->Surface, SDL_SRCALPHA, alpha);
	SDL_BlitSurface(g->SurfaceFlip, &srect, TheScreen, &drect);
	SDL_SetAlpha(g->Surface, SDL_SRCALPHA, oldalpha);
}
#else
void VideoDrawTrans(const Graphic* g, unsigned frame, int x, int y, int alpha)
{
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	glColor4ub(255, 255, 255, alpha);
	VideoDraw(g, frame, x, y);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
}
void VideoDrawClipTrans(const Graphic* g, unsigned frame, int x, int y, int alpha)
{
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	glColor4ub(255, 255, 255, alpha);
	VideoDrawClip(g, frame, x, y);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
}
void VideoDrawTransX(const Graphic* g, unsigned frame, int x, int y, int alpha)
{
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	glColor4ub(255, 255, 255, alpha);
	VideoDrawX(g, frame, x, y);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
}
void VideoDrawClipTransX(const Graphic* g, unsigned frame, int x, int y, int alpha)
{
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	glColor4ub(255, 255, 255, alpha);
	VideoDrawClipX(g, frame, x, y);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
}
#endif

/*----------------------------------------------------------------------------
--  Global functions
----------------------------------------------------------------------------*/

/**
**  Make shadow sprite
**
**  @param g  pointer to object
**
**  @todo FIXME: 32bpp
*/
void MakeShadowSprite(Graphic* g)
{
	SDL_Color colors[256];

	// Set all colors in the palette to black and use 50% alpha
	memset(colors, 0, sizeof(colors));

	SDL_SetPalette(g->Surface, SDL_LOGPAL | SDL_PHYSPAL, colors, 0, 256);
	SDL_SetAlpha(g->Surface, SDL_SRCALPHA | SDL_RLEACCEL, 128);

#ifndef USE_OPENGL
	if (g->SurfaceFlip) {
		SDL_SetPalette(g->SurfaceFlip, SDL_LOGPAL | SDL_PHYSPAL, colors, 0, 256);
		SDL_SetAlpha(g->SurfaceFlip, SDL_SRCALPHA | SDL_RLEACCEL, 128);
	}
#endif
#ifdef USE_OPENGL
	if (g->Textures) {
		glDeleteTextures(g->NumTextures, g->Textures);
		free(g->Textures);
		g->Textures = NULL;
	}
	MakeTexture(g);
#endif
}

//@}
