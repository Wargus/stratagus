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
#include "video.h"
#include "map.h"
#include "player.h"
#include "intern_video.h"
#include "util.h"
#include "iocompat.h"
#include "iolib.h"

/*----------------------------------------------------------------------------
--  Variables
----------------------------------------------------------------------------*/

static hashtable(Graphic*, 4099) GraphicHash;/// lookup table for graphic data

#ifdef USE_OPENGL
static Graphic** Graphics;
static int NumGraphics;
#endif

/*----------------------------------------------------------------------------
--  Functions
----------------------------------------------------------------------------*/

/**
**  Video draw part of graphic.
**
**  @param g   Pointer to object
**  @param gx  X offset into object
**  @param gy  Y offset into object
**  @param w   width to display
**  @param h   height to display
**  @param x   X screen position
**  @param y   Y screen position
*/
#ifndef USE_OPENGL
void VideoDrawSub(const Graphic* g, int gx, int gy,
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

	SDL_BlitSurface(g->Surface, &srect, TheScreen, &drect);
}
#else
void VideoDrawSub(const Graphic* g, int gx, int gy,
	int w, int h, int x, int y)
{
	DrawTexture(g, g->Textures, gx, gy, gx + w, gy + h, x, y, 0);
}
#endif

/**
**  Video draw part of graphic clipped.
**
**  @param g   Pointer to object
**  @param gx  X offset into object
**  @param gy  Y offset into object
**  @param w   width to display
**  @param h   height to display
**  @param x   X screen position
**  @param y   Y screen position
*/
void VideoDrawSubClip(const Graphic* g, int gx, int gy,
	int w, int h, int x, int y)
{
	int oldx;
	int oldy;

	oldx = x;
	oldy = y;
	CLIP_RECTANGLE(x, y, w, h);
	VideoDrawSub(g, gx + x - oldx, gy + y - oldy, w, h, x, y);
}

/**
**  Video draw part of graphic with alpha.
**
**  @param g      Pointer to object
**  @param gx     X offset into object
**  @param gy     Y offset into object
**  @param w      width to display
**  @param h      height to display
**  @param x      X screen position
**  @param y      Y screen position
**  @param alpha  Alpha
*/
#ifndef USE_OPENGL
void VideoDrawSubTrans(const Graphic* g, int gx, int gy,
	int w, int h, int x, int y, unsigned char alpha)
{
	int oldalpha;

	oldalpha = g->Surface->format->alpha;
	SDL_SetAlpha(g->Surface, SDL_SRCALPHA, alpha);
	VideoDrawSub(g, gx, gy, w, h, x, y);
	SDL_SetAlpha(g->Surface, SDL_SRCALPHA, oldalpha);
}
#else
void VideoDrawSubTrans(const Graphic* g, int gx, int gy,
	int w, int h, int x, int y, unsigned char alpha)
{
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	glColor4ub(255, 255, 255, alpha);
	VideoDrawSub(g, gx, gy, w, h, x, y);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
}
#endif

/**
**  Video draw part of graphic with alpha and clipped.
**
**  @param g      Pointer to object
**  @param gx     X offset into object
**  @param gy     Y offset into object
**  @param w      width to display
**  @param h      height to display
**  @param x      X screen position
**  @param y      Y screen position
**  @param alpha  Alpha
*/
void VideoDrawSubClipTrans(const Graphic* g, int gx, int gy,
	int w, int h, int x, int y, unsigned char alpha)
{
	int oldx;
	int oldy;

	oldx = x;
	oldy = y;
	CLIP_RECTANGLE(x, y, w, h);
	VideoDrawSubTrans(g, gx + x - oldx, gy + y - oldy, w, h, x, y, alpha);
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
**  @return      New graphic object (malloced).
*/
Graphic* NewGraphic(const char* file, int w, int h)
{
	Graphic* g;
	Graphic** ptr;

	ptr = (Graphic**)hash_find(GraphicHash, file);
	if (!ptr || !*ptr) {
		g = calloc(1, sizeof(Graphic));
		if (!g) {
			fprintf(stderr, "Out of memory\n");
			ExitFatal(-1);
		}
		g->File = strdup(file);
		g->HashFile = strdup(g->File);
		g->Width = w;
		g->Height = h;
		g->NumFrames = 1;
		g->Refs = 1;
		*(Graphic**)hash_add(GraphicHash, g->HashFile) = g;
	} else {
		g = *ptr;
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
**  @return      New graphic object (malloced).
*/
Graphic* ForceNewGraphic(const char* file, int w, int h)
{
	Graphic* g;

	g = calloc(1, sizeof(Graphic));
	if (!g) {
		fprintf(stderr, "Out of memory\n");
		ExitFatal(-1);
	}
	g->File = strdup(file);
	g->HashFile = malloc(strlen(file) + 2 * sizeof(g->File) + 3);
	sprintf(g->HashFile, "%s%p", g->File, g->File);
	g->Width = w;
	g->Height = h;
	g->NumFrames = 1;
	g->Refs = 1;
	*(Graphic**)hash_add(GraphicHash, g->HashFile) = g;

	return g;
}

/**
**  Load a graphic
**
**  @param g  Graphic object to load
*/
void LoadGraphic(Graphic* g)
{
	if (g->Surface) {
		return;
	}

	// TODO: More formats?
	if (LoadGraphicPNG(g) == -1) {
		fprintf(stderr, "Can't load the graphic `%s'\n", g->File);
		ExitFatal(-1);
	}

	if (!g->Width) {
		g->Width = g->GraphicWidth;
	}
	if (!g->Height) {
		g->Height = g->GraphicHeight;
	}

	Assert(g->Width <= g->GraphicWidth && g->Height <= g->GraphicHeight);

	if ((g->GraphicWidth / g->Width) * g->Width != g->GraphicWidth ||
			(g->GraphicHeight / g->Height) * g->Height != g->GraphicHeight) {
		fprintf(stderr, "Invalid graphic (width, height) %s\n", g->File);
		fprintf(stderr, "Expected: (%d,%d)  Found: (%d,%d)\n",
			g->Width, g->Height, g->GraphicWidth, g->GraphicHeight);
		ExitFatal(1);
	}

	g->NumFrames = g->GraphicWidth / g->Width * g->GraphicHeight / g->Height;

#ifdef USE_OPENGL
	MakeTexture(g);

	++NumGraphics;
	Graphics = realloc(Graphics, NumGraphics * sizeof(*Graphics));
	if (!Graphics) {
		fprintf(stderr, "Out of memory\n");
		exit(1);
	}
	Graphics[NumGraphics - 1] = g;
#endif
}

/**
**  Free a graphic
**
**  @param g  Pointer to the graphic
*/
void FreeGraphic(Graphic* g)
{
#ifdef USE_OPENGL
	int i;
#endif
	void* pixels;

	if (!g) {
		return;
	}

	Assert(g->Refs);
	
	--g->Refs;
	if (!g->Refs) {
		// No more uses of this graphic
#ifdef USE_OPENGL
		if (g->Textures) {
			glDeleteTextures(g->NumTextures, g->Textures);
			free(g->Textures);
		}
		for (i = 0; i < PlayerMax; ++i) {
			if (g->PlayerColorTextures[i]) {
				glDeleteTextures(g->NumTextures, g->PlayerColorTextures[i]);
			}
		}

		for (i = 0; i < NumGraphics; ++i) {
			if (Graphics[i] == g) {
				Graphics[i] = Graphics[--NumGraphics];
				break;
			}
		}
#endif

		if (g->Surface) {
			if (g->Surface->flags & SDL_PREALLOC) {
				pixels = g->Surface->pixels;
			} else {
				pixels = NULL;
			}
			SDL_FreeSurface(g->Surface);
			free(pixels);
		}
#ifndef USE_OPENGL
		if (g->SurfaceFlip) {
			if (g->SurfaceFlip->flags & SDL_PREALLOC) {
				pixels = g->SurfaceFlip->pixels;
			} else {
				pixels = NULL;
			}
			SDL_FreeSurface(g->SurfaceFlip);
			free(pixels);
		}
#endif
		Assert(g->File);

		hash_del(GraphicHash, g->HashFile);
		free(g->File);
		free(g->HashFile);
		free(g);
	}
}

#ifdef USE_OPENGL
/**
**  Reload OpenGL graphics
*/
void ReloadGraphics(void)
{
	int i;
	int j;

	for (i = 0; i < NumGraphics; ++i) {
		if (Graphics[i]->Textures) {
			free(Graphics[i]->Textures);
			Graphics[i]->Textures = NULL;
			MakeTexture(Graphics[i]);
		}
		for (j = 0; j < PlayerMax; ++j) {
			if (Graphics[i]->PlayerColorTextures[j]) {
				free(Graphics[i]->PlayerColorTextures[j]);
				Graphics[i]->PlayerColorTextures[j] = NULL;
				MakePlayerColorTexture(Graphics[j], j);
			}
		}
	}
}
#endif

/**
**  Flip graphic and store in graphic->SurfaceFlip
**
**  @param g  Pointer to object
*/
void FlipGraphic(Graphic* g)
{
#ifdef USE_OPENGL
	return;
#else
	int i;
	int j;
	SDL_Surface* s;

	if (g->SurfaceFlip) {
		return;
	}

	s = g->SurfaceFlip = SDL_ConvertSurface(g->Surface,
		g->Surface->format, SDL_SWSURFACE);
	if (g->Surface->flags & SDL_SRCCOLORKEY) {
		SDL_SetColorKey(g->SurfaceFlip, SDL_SRCCOLORKEY | SDL_RLEACCEL,
			g->Surface->format->colorkey);
	}

	SDL_LockSurface(g->Surface);
	SDL_LockSurface(s);
	switch (s->format->BytesPerPixel) {
		case 1:
			for (i = 0; i < s->h; ++i) {
				for (j = 0; j < s->w; ++j) {
					((char*)s->pixels)[j + i * s->pitch] =
						((char*)g->Surface->pixels)[s->w - j - 1 + i * g->Surface->pitch];
				}
			}
			break;
		case 3:
			for (i = 0; i < s->h; ++i) {
				for (j = 0; j < s->w; ++j) {
					memcpy(&((char*)s->pixels)[j + i * s->pitch],
						&((char*)g->Surface->pixels)[(s->w - j - 1) * 3 + i * g->Surface->pitch], 3);
				}
			}
			break;
		case 4:
			for (i = 0; i < s->h; ++i) {
				for (j = 0; j < s->w; ++j) {
					*(Uint32*)&((char*)s->pixels)[j * 4 + i * s->pitch] =
						*(Uint32*)&((char*)g->Surface->pixels)[(s->w - j - 1) * 4 + i * g->Surface->pitch];
				}
			}
			break;
	}
	SDL_UnlockSurface(g->Surface);
	SDL_UnlockSurface(s);
#endif
}

#ifdef USE_OPENGL
/**
**  Find the next power of 2 >= x
*/
static int PowerOf2(int x)
{
	int i;
	for (i = 1; i < x; i <<= 1) ;
	return i;
}

/**
**  Make an OpenGL texture or textures out of a graphic object.
**
**  @param g       The graphic object.
**  @param width   Graphic width.
**  @param height  Graphic height.
*/
static void MakeTextures2(Graphic* g, GLuint texture, UnitColors* colors,
	int ow, int oh)
{
	int i;
	int j;
	int h;
	int w;
	unsigned char* tex;
	unsigned char* tp;
	const unsigned char* sp;
	int fl;
	Uint32 ckey;
	int useckey;
	int bpp;
	unsigned char alpha;
	Uint32 b;
	Uint32 c;
	Uint32 pc;
	SDL_PixelFormat* f;
	int maxw;
	int maxh;

	fl = g->GraphicWidth / g->Width;
	useckey = g->Surface->flags & SDL_SRCCOLORKEY;
	f = g->Surface->format;
	bpp = f->BytesPerPixel;
	ckey = f->colorkey;

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	maxw = g->GraphicWidth - ow > GLMaxTextureSize ?
		GLMaxTextureSize : g->GraphicWidth - ow;
	maxh = g->GraphicHeight - oh > GLMaxTextureSize ?
		GLMaxTextureSize : g->GraphicHeight - oh;
	w = PowerOf2(maxw);
	h = PowerOf2(maxh);
	tex = malloc(w * h * 4);
	if (g->Surface->flags & SDL_SRCALPHA) {
		alpha = f->alpha;
	} else {
		alpha = 0xff;
	}

	SDL_LockSurface(g->Surface);
	glBindTexture(GL_TEXTURE_2D, texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	for (i = 0; i < maxh; ++i) {
		sp = (const unsigned char*)g->Surface->pixels + ow * bpp +
			(oh + i) * g->Surface->pitch;
		tp = tex + i * w * 4;
		for (j = 0; j < maxw; ++j) {
			int z;
			SDL_Color p;

			if (bpp == 1) {
				if (useckey && *sp == ckey) {
					tp[3] = 0;
				} else {
					p = f->palette->colors[*sp];
					tp[0] = p.r;
					tp[1] = p.g;
					tp[2] = p.b;
					tp[3] = alpha;
				}
				if (colors) {
					for (z = 0; z < PlayerShadeCount; ++z) {
						if (*sp == PlayerShadeStart + z) {
							p = colors->Colors[z];
							tp[0] = p.r;
							tp[1] = p.g;
							tp[2] = p.b;
							tp[3] = 0xff;
							break;
						}
					}
				}
				++sp;
			} else {
				if (bpp == 4) {
					c = *(Uint32*)sp;
				} else {
					c = (sp[f->Rshift >> 3] << f->Rshift) |
						(sp[f->Gshift >> 3] << f->Gshift) |
						(sp[f->Bshift >> 3] << f->Bshift);
					c |= ((alpha | (alpha << 8) | (alpha << 16) | (alpha << 24)) ^
						(f->Rmask | f->Gmask | f->Bmask));
				}
				*(Uint32*)tp = c;
				if (colors) {
					b = (c & f->Bmask) >> f->Bshift;
					if (b && ((c & f->Rmask) >> f->Rshift) == 0 &&
							((c & f->Gmask) >> f->Gshift) == b) {
						pc = ((colors->Colors[0].r * b / 255) << f->Rshift) |
							((colors->Colors[0].g * b / 255) << f->Gshift) |
							((colors->Colors[0].b * b / 255) << f->Bshift);
						if (bpp == 4) {
							pc |= (c & f->Amask);
						} else {
							pc |= (0xFFFFFFFF ^ (f->Rmask | f->Gmask | f->Bmask));
						}
						*(Uint32*)tp = pc;
					}
				}
				sp += bpp;
			}
			tp += 4;
		}
	}
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, tex);
#ifdef DEBUG
	if ((i = glGetError())) {
		DebugPrint("glTexImage2D(%x)\n" _C_ i);
	}
#endif
	SDL_UnlockSurface(g->Surface);
	free(tex);
}

/**
**  Make an OpenGL texture or textures out of a graphic object.
**
**  @param g       The graphic object.
**  @param width   Graphic width.
**  @param height  Graphic height.
*/
static void MakeTextures(Graphic* g, int player, UnitColors* colors)
{
	int i;
	int j;
	int tw;
	int th;
	GLuint* textures;

	tw = g->GraphicWidth / (GLMaxTextureSize + 1) + 1;
	th = g->GraphicHeight / (GLMaxTextureSize + 1) + 1;

	i = g->GraphicWidth % GLMaxTextureSize;
	g->TextureWidth = (GLfloat)i / PowerOf2(i);
	i = g->GraphicHeight % GLMaxTextureSize;
	g->TextureHeight = (GLfloat)i / PowerOf2(i);

	g->NumTextures = tw * th;
	if (g->NumTextures > 1) {
		tw = tw;
	}
	if (!colors) {
		textures = g->Textures = malloc(g->NumTextures * sizeof(*g->Textures));
		glGenTextures(g->NumTextures, g->Textures);
	} else {
		textures = g->PlayerColorTextures[player] = malloc(g->NumTextures * sizeof(GLuint));
		glGenTextures(g->NumTextures, g->PlayerColorTextures[player]);
	}

	for (j = 0; j < th; ++j) {
		for (i = 0; i < tw; ++i) {
			MakeTextures2(g, textures[j * tw + i], colors, GLMaxTextureSize * i, GLMaxTextureSize * j);
		}
	}
}

/**
**  Make an OpenGL texture or textures out of a graphic object.
**
**  @param g  The graphic object.
*/
void MakeTexture(Graphic* g)
{
	if (g->Textures) {
		return;
	}

	MakeTextures(g, 0, NULL);
}

/**
**  Make an OpenGL texture with the player colors.
**
**  @param g       The graphic to texture with player colors.
**  @param player  Player number to make textures for.
*/
void MakePlayerColorTexture(Graphic* g, int player)
{
	if (g->PlayerColorTextures[player]) {
		return;
	}

	MakeTextures(g, player, &Players[player].UnitColors);
}
#endif

/**
**  Resize a graphic
**
**  @param g  Graphic object.
**  @param w  New width of graphic.
**  @param h  New height of graphic.
*/
void ResizeGraphic(Graphic* g, int w, int h)
{
	int i;
	int j;
	unsigned char* data;
	unsigned char* pixels;
	int x;
	int bpp;

	if (g->GraphicWidth == w && g->GraphicHeight == h) {
		return;
	}

	bpp = g->Surface->format->BytesPerPixel;
	if (bpp == 1) {
		SDL_Color pal[256];
		Uint32 ckey;
		int useckey;

		SDL_LockSurface(g->Surface);

		pixels = (unsigned char*)g->Surface->pixels;
		data = malloc(w * h);
		x = 0;

		for (i = 0; i < h; ++i) {
			for (j = 0; j < w; ++j) {
				data[x] = pixels[(i * g->Height / h) * g->Surface->pitch + j * g->Width / w];
				++x;
			}
		}

		SDL_UnlockSurface(g->Surface);
		memcpy(pal, g->Surface->format->palette->colors, sizeof(SDL_Color) * 256);
		useckey = g->Surface->flags & SDL_SRCCOLORKEY;
		ckey = g->Surface->format->colorkey;
		SDL_FreeSurface(g->Surface);

		g->Surface = SDL_CreateRGBSurfaceFrom(data, w, h, 8, w, 0, 0, 0, 0);
		SDL_SetPalette(g->Surface, SDL_LOGPAL | SDL_PHYSPAL, pal, 0, 256);
		if (useckey) {
			SDL_SetColorKey(g->Surface, SDL_SRCCOLORKEY | SDL_RLEACCEL, ckey);
		}
	} else {
		int ix, iy;
		float fx, fy, fz;
		unsigned char *p1, *p2, *p3, *p4;

		SDL_LockSurface(g->Surface);

		pixels = (unsigned char*)g->Surface->pixels;
		data = malloc(w * h * bpp);
		x = 0;

		for (i = 0; i < h; ++i) {
			fy = (float)i * g->Height / h;
			iy = (int)fy;
			fy -= iy;
			for (j = 0; j < w; ++j) {
				fx = (float)j * g->Width / w;
				ix = (int)fx;
				fx -= ix;
				fz = (fx + fy) / 2;

				p1 = &pixels[iy * g->Surface->pitch + ix * bpp];
				p2 = (iy != g->Surface->h - 1) ?
					&pixels[(iy + 1) * g->Surface->pitch + ix * bpp] :
					p1;
				p3 = (ix != g->Surface->w - 1) ?
					&pixels[iy * g->Surface->pitch + (ix + 1) * bpp] :
					p1;
				p4 = (iy != g->Surface->h - 1 && ix != g->Surface->w - 1) ?
					&pixels[(iy + 1) * g->Surface->pitch + (ix + 1) * bpp] :
					p1;

				data[x * bpp + 0] = (p1[0] * (1 - fy) + p2[0] * fy +
					p1[0] * (1 - fx) + p3[0] * fx +
					p1[0] * (1 - fz) + p4[0] * fz) / 3 + .5;
				data[x * bpp + 1] = (p1[1] * (1 - fy) + p2[1] * fy +
					p1[1] * (1 - fx) + p3[1] * fx +
					p1[1] * (1 - fz) + p4[1] * fz) / 3 + .5;
				data[x * bpp + 2] = (p1[2] * (1 - fy) + p2[2] * fy +
					p1[2] * (1 - fx) + p3[2] * fx +
					p1[2] * (1 - fz) + p4[2] * fz) / 3 + .5;
				if (bpp == 4) {
					data[x * bpp + 3] = (p1[3] * (1 - fy) + p2[3] * fy +
						p1[3] * (1 - fx) + p3[3] * fx +
						p1[3] * (1 - fz) + p4[3] * fz) / 3 + .5;
				}
				++x;
			}
		}

		SDL_UnlockSurface(g->Surface);
		SDL_FreeSurface(g->Surface);

		g->Surface = SDL_CreateRGBSurfaceFrom(data, w, h, 8 * bpp, w * bpp,
			RMASK, GMASK, BMASK, (bpp == 3 ? 0 : AMASK));
	}

	g->Width = g->GraphicWidth = w;
	g->Height = g->GraphicHeight = h;

#ifdef USE_OPENGL
	glDeleteTextures(g->NumTextures, g->Textures);
	free(g->Textures);
	g->Textures = NULL;
	MakeTexture(g);
#endif
}

/**
**  Check if a pixel is transparent
**
**  @param g  Graphic to check
**  @param x  X coordinate
**  @param y  Y coordinate
**
**  @return   True if the pixel is transparent, False otherwise
*/
int GraphicTransparentPixel(Graphic* g, int x, int y)
{
	unsigned char* p;
	int bpp;
	int ret;

	bpp = g->Surface->format->BytesPerPixel;
	if ((bpp == 1 && !(g->Surface->flags & SDL_SRCCOLORKEY)) || bpp == 3) {
		return 0;
	}

	ret = 0;
	SDL_LockSurface(g->Surface);
	p = (unsigned char*)g->Surface->pixels + y * g->Surface->pitch + x * bpp;
	if (bpp == 1) {
		if (*p == g->Surface->format->colorkey) {
			ret = 1;
		}
	} else {
		if (p[g->Surface->format->Ashift >> 3] == 255) {
			ret = 1;
		}
	}
	SDL_UnlockSurface(g->Surface);

	return ret;
}

//@}
