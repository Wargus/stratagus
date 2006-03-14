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
/**@name graphic.cpp - The general graphic functions. */
//
//      (c) Copyright 1999-2005 by Lutz Sammer, Nehal Mistry, and Jimmy Salmon
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

#include <string>
#include <map>
#include <list>

#include "video.h"
#include "player.h"
#include "intern_video.h"
#include "iocompat.h"
#include "iolib.h"

/*----------------------------------------------------------------------------
--  Variables
----------------------------------------------------------------------------*/

static std::map<std::string, CGraphic *> GraphicHash;

/*----------------------------------------------------------------------------
--  Functions
----------------------------------------------------------------------------*/

/**
**  Video draw part of graphic.
**
**  @param gx  X offset into object
**  @param gy  Y offset into object
**  @param w   width to display
**  @param h   height to display
**  @param x   X screen position
**  @param y   Y screen position
*/
void CGraphic::DrawSub(int gx, int gy, int w, int h, int x, int y) const
{
	SDL_Rect srect = {gx, gy, w, h};
	SDL_Rect drect = {x, y};

	SDL_BlitSurface(Surface, &srect, TheScreen, &drect);
}

/**
**  Video draw part of graphic clipped.
**
**  @param gx  X offset into object
**  @param gy  Y offset into object
**  @param w   width to display
**  @param h   height to display
**  @param x   X screen position
**  @param y   Y screen position
*/
void CGraphic::DrawSubClip(int gx, int gy, int w, int h, int x, int y) const
{
	int oldx = x;
	int oldy = y;
	CLIP_RECTANGLE(x, y, w, h);
	DrawSub(gx + x - oldx, gy + y - oldy, w, h, x, y);
}

/**
**  Video draw part of graphic with alpha.
**
**  @param gx     X offset into object
**  @param gy     Y offset into object
**  @param w      width to display
**  @param h      height to display
**  @param x      X screen position
**  @param y      Y screen position
**  @param alpha  Alpha
*/
void CGraphic::DrawSubTrans(int gx, int gy, int w, int h, int x, int y,
	unsigned char alpha) const
{
	int oldalpha = Surface->format->alpha;
	SDL_SetAlpha(Surface, SDL_SRCALPHA, alpha);
	DrawSub(gx, gy, w, h, x, y);
	SDL_SetAlpha(Surface, SDL_SRCALPHA, oldalpha);
}

/**
**  Video draw part of graphic with alpha and clipped.
**
**  @param gx     X offset into object
**  @param gy     Y offset into object
**  @param w      width to display
**  @param h      height to display
**  @param x      X screen position
**  @param y      Y screen position
**  @param alpha  Alpha
*/
void CGraphic::DrawSubClipTrans(int gx, int gy, int w, int h, int x, int y,
	unsigned char alpha) const
{
	int oldx = x;
	int oldy = y;
	CLIP_RECTANGLE(x, y, w, h);
	DrawSubTrans(gx + x - oldx, gy + y - oldy, w, h, x, y, alpha);
}

/**
**  Draw graphic object unclipped.
**
**  @param frame   number of frame (object index)
**  @param x       x coordinate on the screen
**  @param y       y coordinate on the screen
*/
void CGraphic::DrawFrame(unsigned frame, int x, int y) const
{
	DrawSub((frame % (Surface->w / Width)) * Width,
		(frame / (Surface->w / Width)) * Height,
		Width, Height, x, y);
}

/**
**  Draw graphic object clipped.
**
**  @param frame   number of frame (object index)
**  @param x       x coordinate on the screen
**  @param y       y coordinate on the screen
*/
void CGraphic::DrawFrameClip(unsigned frame, int x, int y) const
{
	DrawSubClip((frame % (Surface->w / Width)) * Width,
		(frame / (Surface->w / Width)) * Height,
		Width, Height, x, y);
}

void CGraphic::DrawFrameTrans(unsigned frame, int x, int y, int alpha) const
{
	DrawSubTrans((frame % (Surface->w / Width)) * Width,
		(frame / (Surface->w / Width)) * Height,
		Width, Height, x, y, alpha);
}

void CGraphic::DrawFrameClipTrans(unsigned frame, int x, int y, int alpha) const
{
	DrawSubClipTrans((frame % (Surface->w / Width)) * Width,
		(frame / (Surface->w / Width)) * Height,
		Width, Height, x, y, alpha);
}


/**
**  Draw graphic object clipped and with player colors.
**
**  @param player  player number
**  @param frame   number of frame (object index)
**  @param x       x coordinate on the screen
**  @param y       y coordinate on the screen
*/
void CGraphic::DrawPlayerColorFrameClip(int player, unsigned frame,
	int x, int y)
{
	GraphicPlayerPixels(&Players[player], this);
	DrawFrameClip(frame, x, y);
}



/**
**  Draw graphic object unclipped and flipped in X direction.
**
**  @param frame   number of frame (object index)
**  @param x       x coordinate on the screen
**  @param y       y coordinate on the screen
*/
void CGraphic::DrawFrameX(unsigned frame, int x, int y) const
{
	SDL_Rect srect;
	SDL_Rect drect;

	srect.x = (SurfaceFlip->w - (frame % (SurfaceFlip->w /
		Width)) * Width) - Width;
	srect.y = (frame / (SurfaceFlip->w / Width)) * Height;
	srect.w = Width;
	srect.h = Height;

	drect.x = x;
	drect.y = y;

	SDL_BlitSurface(SurfaceFlip, &srect, TheScreen, &drect);
}

/**
**  Draw graphic object clipped and flipped in X direction.
**
**  @param frame   number of frame (object index)
**  @param x       x coordinate on the screen
**  @param y       y coordinate on the screen
*/
void CGraphic::DrawFrameClipX(unsigned frame, int x, int y) const
{
	SDL_Rect srect;
	SDL_Rect drect;
	int oldx;
	int oldy;

	srect.x = (SurfaceFlip->w - (frame % (SurfaceFlip->w /
			Width)) * Width) - Width;
	srect.y = (frame / (SurfaceFlip->w / Width)) * Height;
	srect.w = Width;
	srect.h = Height;

	oldx = x;
	oldy = y;
	CLIP_RECTANGLE(x, y, srect.w, srect.h);
	srect.x += x - oldx;
	srect.y += y - oldy;

	drect.x = x;
	drect.y = y;

	SDL_BlitSurface(SurfaceFlip, &srect, TheScreen, &drect);
}

void CGraphic::DrawFrameTransX(unsigned frame, int x, int y, int alpha) const
{
	SDL_Rect srect;
	SDL_Rect drect;
	int oldalpha;

	srect.x = (SurfaceFlip->w - (frame % (SurfaceFlip->w /
		Width)) * Width) - Width;
	srect.y = (frame / (SurfaceFlip->w / Width)) * Height;
	srect.w = Width;
	srect.h = Height;

	drect.x = x;
	drect.y = y;

	oldalpha = Surface->format->alpha;
	SDL_SetAlpha(Surface, SDL_SRCALPHA, alpha);
	SDL_BlitSurface(SurfaceFlip, &srect, TheScreen, &drect);
	SDL_SetAlpha(Surface, SDL_SRCALPHA, oldalpha);
}

void CGraphic::DrawFrameClipTransX(unsigned frame, int x, int y, int alpha) const
{
	SDL_Rect srect;
	SDL_Rect drect;
	int oldx;
	int oldy;
	int oldalpha;

	srect.x = (SurfaceFlip->w - (frame % (SurfaceFlip->w /
		Width)) * Width) - Width;
	srect.y = (frame / (SurfaceFlip->w / Width)) * Height;
	srect.w = Width;
	srect.h = Height;

	oldx = x;
	oldy = y;
	CLIP_RECTANGLE(x, y, srect.w, srect.h);
	srect.x += x - oldx;
	srect.y += y - oldy;

	drect.x = x;
	drect.y = y;

	oldalpha = Surface->format->alpha;
	SDL_SetAlpha(Surface, SDL_SRCALPHA, alpha);
	SDL_BlitSurface(SurfaceFlip, &srect, TheScreen, &drect);
	SDL_SetAlpha(Surface, SDL_SRCALPHA, oldalpha);
}

/**
**  Draw graphic object clipped, flipped, and with player colors.
**
**  @param player  player number
**  @param frame   number of frame (object index)
**  @param x       x coordinate on the screen
**  @param y       y coordinate on the screen
*/
void CGraphic::DrawPlayerColorFrameClipX(int player, unsigned frame,
	int x, int y)
{
	GraphicPlayerPixels(&Players[player], this);
	DrawFrameClipX(frame, x, y);
}

/*----------------------------------------------------------------------------
--  Global functions
----------------------------------------------------------------------------*/

/**
**  Make a new graphic object.
**
**  @param file  Filename
**  @param w     Width of a frame (optional)
**  @param h     Height of a frame (optional)
**
**  @return      New graphic object
*/
CGraphic *CGraphic::New(const char *file, int w, int h)
{
	if (!file) {
		return new CGraphic;
	}

	CGraphic *g = GraphicHash[file];
	if (!g) {
		g = new CGraphic;
		if (!g) {
			fprintf(stderr, "Out of memory\n");
			ExitFatal(-1);
		}
		// FIXME: use a constructor for this
		g->File = new_strdup(file);
		g->HashFile = new_strdup(g->File);
		g->Width = w;
		g->Height = h;
		GraphicHash[g->HashFile] = g;
	} else {
		++g->Refs;
		Assert((w == 0 || g->Width == w) && (g->Height == h || h == 0));
	}

	return g;
}

/**
**  Make a new graphic object.  Don't reuse a graphic from the hash table.
**
**  @param file  Filename
**  @param w     Width of a frame (optional)
**  @param h     Height of a frame (optional)
**
**  @return      New graphic object
*/
CGraphic *CGraphic::ForceNew(const char *file, int w, int h)
{
	CGraphic *g = new CGraphic;
	if (!g) {
		fprintf(stderr, "Out of memory\n");
		ExitFatal(-1);
	}
	g->File = new_strdup(file);
	g->HashFile = new char[strlen(file) + 2 * sizeof(g->File) + 3];
	sprintf(g->HashFile, "%s%p", g->File, g->File);
	g->Width = w;
	g->Height = h;
	GraphicHash[g->HashFile] = g;

	return g;
}

/**
**  Load a graphic
*/
void CGraphic::Load()
{
	if (Surface) {
		return;
	}

	// TODO: More formats?
	if (LoadGraphicPNG(this) == -1) {
		fprintf(stderr, "Can't load the graphic `%s'\n", File);
		ExitFatal(-1);
	}

	if (!Width) {
		Width = GraphicWidth;
	}
	if (!Height) {
		Height = GraphicHeight;
	}

	Assert(Width <= GraphicWidth && Height <= GraphicHeight);

	if ((GraphicWidth / Width) * Width != GraphicWidth ||
			(GraphicHeight / Height) * Height != GraphicHeight) {
		fprintf(stderr, "Invalid graphic (width, height) %s\n", File);
		fprintf(stderr, "Expected: (%d,%d)  Found: (%d,%d)\n",
			Width, Height, GraphicWidth, GraphicHeight);
		ExitFatal(1);
	}

	NumFrames = GraphicWidth / Width * GraphicHeight / Height;
}

/**
**  Free a graphic
**
**  @param g  Pointer to the graphic
*/
void CGraphic::Free(CGraphic *g)
{
	unsigned char *pixels;

	if (!g) {
		return;
	}

	Assert(g->Refs);

	--g->Refs;
	if (!g->Refs) {
		// No more uses of this graphic
		if (g->Surface) {
			if (g->Surface->flags & SDL_PREALLOC) {
				pixels = (unsigned char *)g->Surface->pixels;
			} else {
				pixels = NULL;
			}
			SDL_FreeSurface(g->Surface);
			delete[] pixels;
		}
		if (g->SurfaceFlip) {
			if (g->SurfaceFlip->flags & SDL_PREALLOC) {
				pixels = (unsigned char *)g->SurfaceFlip->pixels;
			} else {
				pixels = NULL;
			}
			SDL_FreeSurface(g->SurfaceFlip);
			delete[] pixels;
		}

		if (g->HashFile) {
			GraphicHash.erase(g->HashFile);
		}
		delete[] g->File;
		delete[] g->HashFile;
		delete g;
	}
}

/**
**  Flip graphic and store in graphic->SurfaceFlip
*/
void CGraphic::Flip()
{
	int i;
	int j;
	SDL_Surface *s;

	if (SurfaceFlip) {
		return;
	}

	s = SurfaceFlip = SDL_ConvertSurface(Surface, Surface->format, SDL_SWSURFACE);
	if (Surface->flags & SDL_SRCCOLORKEY) {
		SDL_SetColorKey(SurfaceFlip, SDL_SRCCOLORKEY | SDL_RLEACCEL,
			Surface->format->colorkey);
	}

	SDL_LockSurface(Surface);
	SDL_LockSurface(s);
	switch (s->format->BytesPerPixel) {
		case 1:
			for (i = 0; i < s->h; ++i) {
				for (j = 0; j < s->w; ++j) {
					((char *)s->pixels)[j + i * s->pitch] =
						((char *)Surface->pixels)[s->w - j - 1 + i * Surface->pitch];
				}
			}
			break;
		case 3:
			for (i = 0; i < s->h; ++i) {
				for (j = 0; j < s->w; ++j) {
					memcpy(&((char *)s->pixels)[j + i * s->pitch],
						&((char *)Surface->pixels)[(s->w - j - 1) * 3 + i * Surface->pitch], 3);
				}
			}
			break;
		case 4:
			for (i = 0; i < s->h; ++i) {
				for (j = 0; j < s->w; ++j) {
					*(Uint32 *)&((char *)s->pixels)[j * 4 + i * s->pitch] =
						*(Uint32 *)&((char *)Surface->pixels)[(s->w - j - 1) * 4 + i * Surface->pitch];
				}
			}
			break;
	}
	SDL_UnlockSurface(Surface);
	SDL_UnlockSurface(s);
}

/**
**  Resize a graphic
**
**  @param w  New width of graphic.
**  @param h  New height of graphic.
*/
void CGraphic::Resize(int w, int h)
{
	int i;
	int j;
	unsigned char *data;
	unsigned char *pixels;
	int x;
	int bpp;

	if (GraphicWidth == w && GraphicHeight == h) {
		return;
	}

	bpp = Surface->format->BytesPerPixel;
	if (bpp == 1) {
		SDL_Color pal[256];
		Uint32 ckey;
		int useckey;

		SDL_LockSurface(Surface);

		pixels = (unsigned char *)Surface->pixels;
		data = new unsigned char[w * h];
		x = 0;

		for (i = 0; i < h; ++i) {
			for (j = 0; j < w; ++j) {
				data[x] = pixels[(i * Height / h) * Surface->pitch + j * Width / w];
				++x;
			}
		}

		SDL_UnlockSurface(Surface);
		memcpy(pal, Surface->format->palette->colors, sizeof(SDL_Color) * 256);
		useckey = Surface->flags & SDL_SRCCOLORKEY;
		ckey = Surface->format->colorkey;
		SDL_FreeSurface(Surface);

		Surface = SDL_CreateRGBSurfaceFrom(data, w, h, 8, w, 0, 0, 0, 0);
		SDL_SetPalette(Surface, SDL_LOGPAL | SDL_PHYSPAL, pal, 0, 256);
		if (useckey) {
			SDL_SetColorKey(Surface, SDL_SRCCOLORKEY | SDL_RLEACCEL, ckey);
		}
	} else {
		int ix, iy;
		float fx, fy, fz;
		unsigned char *p1, *p2, *p3, *p4;

		SDL_LockSurface(Surface);

		pixels = (unsigned char *)Surface->pixels;
		data = new unsigned char[w * h * bpp];
		x = 0;

		for (i = 0; i < h; ++i) {
			fy = (float)i * Height / h;
			iy = (int)fy;
			fy -= iy;
			for (j = 0; j < w; ++j) {
				fx = (float)j * Width / w;
				ix = (int)fx;
				fx -= ix;
				fz = (fx + fy) / 2;

				p1 = &pixels[iy * Surface->pitch + ix * bpp];
				p2 = (iy != Surface->h - 1) ?
					&pixels[(iy + 1) * Surface->pitch + ix * bpp] :
					p1;
				p3 = (ix != Surface->w - 1) ?
					&pixels[iy * Surface->pitch + (ix + 1) * bpp] :
					p1;
				p4 = (iy != Surface->h - 1 && ix != Surface->w - 1) ?
					&pixels[(iy + 1) * Surface->pitch + (ix + 1) * bpp] :
					p1;

				data[x * bpp + 0] = static_cast<unsigned char>(
					(p1[0] * (1 - fy) + p2[0] * fy +
					p1[0] * (1 - fx) + p3[0] * fx +
					p1[0] * (1 - fz) + p4[0] * fz) / 3.0 + .5);
				data[x * bpp + 1] = static_cast<unsigned char>(
					(p1[1] * (1 - fy) + p2[1] * fy +
					p1[1] * (1 - fx) + p3[1] * fx +
					p1[1] * (1 - fz) + p4[1] * fz) / 3.0 + .5);
				data[x * bpp + 2] = static_cast<unsigned char>(
					(p1[2] * (1 - fy) + p2[2] * fy +
					p1[2] * (1 - fx) + p3[2] * fx +
					p1[2] * (1 - fz) + p4[2] * fz) / 3.0 + .5);
				if (bpp == 4) {
					data[x * bpp + 3] = static_cast<unsigned char>(
						(p1[3] * (1 - fy) + p2[3] * fy +
						p1[3] * (1 - fx) + p3[3] * fx +
						p1[3] * (1 - fz) + p4[3] * fz) / 3.0 + .5);
				}
				++x;
			}
		}

		SDL_UnlockSurface(Surface);
		SDL_FreeSurface(Surface);

		Surface = SDL_CreateRGBSurfaceFrom(data, w, h, 8 * bpp, w * bpp,
			RMASK, GMASK, BMASK, (bpp == 3 ? 0 : AMASK));
	}

	Width = GraphicWidth = w;
	Height = GraphicHeight = h;
}

/**
**  Check if a pixel is transparent
**
**  @param x  X coordinate
**  @param y  Y coordinate
**
**  @return   True if the pixel is transparent, False otherwise
*/
int CGraphic::TransparentPixel(int x, int y)
{
	unsigned char *p;
	int bpp;
	int ret;

	bpp = Surface->format->BytesPerPixel;
	if ((bpp == 1 && !(Surface->flags & SDL_SRCCOLORKEY)) || bpp == 3) {
		return 0;
	}

	ret = 0;
	SDL_LockSurface(Surface);
	p = (unsigned char *)Surface->pixels + y * Surface->pitch + x * bpp;
	if (bpp == 1) {
		if (*p == Surface->format->colorkey) {
			ret = 1;
		}
	} else {
		if (p[Surface->format->Ashift >> 3] == 255) {
			ret = 1;
		}
	}
	SDL_UnlockSurface(Surface);

	return ret;
}

/**
**  Make shadow sprite
**
**  @todo FIXME: 32bpp
*/
void CGraphic::MakeShadow()
{
	SDL_Color colors[256];

	// Set all colors in the palette to black and use 50% alpha
	memset(colors, 0, sizeof(colors));

	SDL_SetPalette(Surface, SDL_LOGPAL | SDL_PHYSPAL, colors, 0, 256);
	SDL_SetAlpha(Surface, SDL_SRCALPHA | SDL_RLEACCEL, 128);

	if (SurfaceFlip) {
		SDL_SetPalette(SurfaceFlip, SDL_LOGPAL | SDL_PHYSPAL, colors, 0, 256);
		SDL_SetAlpha(SurfaceFlip, SDL_SRCALPHA | SDL_RLEACCEL, 128);
	}
}

//@}
