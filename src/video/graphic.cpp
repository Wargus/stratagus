//       _________ __                 __                               
//      /   _____//  |_____________ _/  |______     ____  __ __  ______
//      \_____  \\   __\_  __ \__  \\   __\__  \   / ___\|  |  \/  ___/
//      /        \|  |  |  | \// __ \|  |  / __ \_/ /_/  >  |  /\___ |
//     /_______  /|__|  |__|  (____  /__| (____  /\___  /|____//____  >
//             \/                  \/          \//_____/            \/ 
//  ______________________                           ______________________
//			  T H E   W A R   B E G I N S
//	   Stratagus - A free fantasy real time strategy game engine
//
/**@name graphic.c	-	The general graphic functions. */
//
//	(c) Copyright 1999-2002 by Lutz Sammer and Nehal Mistry
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
//	$Id$

//@{

/*----------------------------------------------------------------------------
--	Includes
----------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>

#include "stratagus.h"
#include "video.h"
#include "iolib.h"
#include "intern_video.h"

#ifdef USE_SDL_SURFACE
#include <string.h>
#endif

/*----------------------------------------------------------------------------
--	Declarations
----------------------------------------------------------------------------*/

global PaletteLink* PaletteList;	/// List of all used palettes.

/*----------------------------------------------------------------------------
--	Variables
----------------------------------------------------------------------------*/

#ifdef USE_SDL_SURFACE
local GraphicType GraphicImage;
#else
local GraphicType GraphicImage8Type;	/// image type 8bit palette
local GraphicType GraphicImage16Type;	/// image type 16bit palette
#endif

/*----------------------------------------------------------------------------
--	Local functions
----------------------------------------------------------------------------*/

#ifdef USE_SDL_SURFACE
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

    CLIP_RECTANGLE(gx, gy, w, h);

    SDL_BlitSurface(graphic->Surface, &srect, TheScreen, &drect);
//    InvalidateArea(x, y, w, h);
}

global void VideoDrawSubClip(const Graphic* graphic, int gx, int gy,
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

    CLIP_RECTANGLE(gx, gy, w, h);

    SDL_BlitSurface(graphic->Surface, &srect, TheScreen, &drect);
//    InvalidateArea(x, y, w, h);
//    Invalidate();
}

#else
/**
**	Video draw part of 8bit graphic into 8 bit framebuffer.
**
**	@param graphic	Pointer to object
**	@param gx	X offset into object
**	@param gy	Y offset into object
**	@param w	width to display
**	@param h	height to display
**	@param x	X screen position
**	@param y	Y screen position
*/
local void VideoDrawSub8to8(const Graphic* graphic, int gx, int gy,
    int w, int h, int x, int y)
{
    const unsigned char* sp;
    const unsigned char* lp;
    const unsigned char* gp;
    VMemType8* dp;
    const VMemType8* pixels;
    int sa;
    int da;

    pixels = graphic->Pixels;
    sp = (const unsigned char*)graphic->Frames + gx + gy * graphic->Width;
    gp = sp + graphic->Width * h;
    sa = graphic->Width - w;
    dp = VideoMemory8 + x + y * VideoWidth;
    da = VideoWidth - w--;

    while (sp < gp) {
	lp = sp + w;
	while (sp < lp) {
	    *dp++ = pixels[*sp++];	// unroll
	    *dp++ = pixels[*sp++];
	}
	if (sp <= lp) {
	    *dp++ = pixels[*sp++];
	}
	sp += sa;
	dp += da;
    }
}

/**
**	Video draw part of 8bit graphic into 16 bit framebuffer.
**
**	@param graphic	Pointer to object
**	@param gx	X offset into object
**	@param gy	Y offset into object
**	@param w	width to display
**	@param h	height to display
**	@param x	X screen position
**	@param y	Y screen position
*/
local void VideoDrawSub8to16(const Graphic* graphic, int gx, int gy,
    int w, int h, int x,int y)
{
    const unsigned char* sp;
    const unsigned char* lp;
    const unsigned char* gp;
    int sa;
    const VMemType16* pixels;
    VMemType16* dp;
    int da;

    pixels = (VMemType16*)graphic->Pixels;
    sp = (const unsigned char*)graphic->Frames + gx + gy * graphic->Width;
    gp = sp + graphic->Width * h;
    sa = graphic->Width - w;
    dp = VideoMemory16 + x + y * VideoWidth;
    da = VideoWidth - w--;

    while (sp < gp) {
	lp = sp + w;
	while (sp < lp) {
	    *dp++ = pixels[*sp++];	// unroll
	    *dp++ = pixels[*sp++];
	}
	if (sp <= lp) {
	    *dp++ = pixels[*sp++];
	}
	sp += sa;
	dp += da;
    }
}

/**
**	Video draw part of 8bit graphic into 24 bit framebuffer.
**
**	FIXME: 24 bit blitting could be optimized.
**
**	@param graphic	Pointer to object
**	@param gx	X offset into object
**	@param gy	Y offset into object
**	@param w	width to display
**	@param h	height to display
**	@param x	X screen position
**	@param y	Y screen position
*/
local void VideoDrawSub8to24(const Graphic* graphic, int gx, int gy,
    int w, int h, int x, int y)
{
    const unsigned char* sp;
    const unsigned char* lp;
    const unsigned char* gp;
    int sa;
    const VMemType24* pixels;
    VMemType24* dp;
    int da;

    pixels = (VMemType24*)graphic->Pixels;
    sp = (const unsigned char*)graphic->Frames + gx + gy * graphic->Width;
    gp = sp + graphic->Width * h;
    sa = graphic->Width - w;
    dp = VideoMemory24 + x + y * VideoWidth;
    da = VideoWidth - w--;

    while (sp < gp) {
	lp = sp + w;
	while (sp < lp) {
	    *dp++ = pixels[*sp++];	// unroll
	    *dp++ = pixels[*sp++];
	}
	if (sp <= lp) {
	    *dp++ = pixels[*sp++];
	}
	sp += sa;
	dp += da;
    }
}

/**
**	Video draw part of 8bit graphic into 32 bit framebuffer.
**
**	@param graphic	Pointer to object
**	@param gx	X offset into object
**	@param gy	Y offset into object
**	@param w	width to display
**	@param h	height to display
**	@param x	X screen position
**	@param y	Y screen position
*/
local void VideoDrawSub8to32(const Graphic* graphic, int gx, int gy,
    int w, int h, int x, int y)
{
    const unsigned char* sp;
    const unsigned char* lp;
    const unsigned char* gp;
    int sa;
    const VMemType32* pixels;
    VMemType32* dp;
    int da;

    pixels = (VMemType32*)graphic->Pixels;
    sp = (const unsigned char*)graphic->Frames + gx + gy * graphic->Width;
    gp = sp + graphic->Width * h;
    sa = graphic->Width - w;
    dp = VideoMemory32 + x + y * VideoWidth;
    da = VideoWidth - w--;

    while (sp < gp) {
	lp = sp + w;
	while (sp < lp) {
	    *dp++ = pixels[*sp++];	// unroll
	    *dp++ = pixels[*sp++];
	}
	if (sp <= lp) {
	    *dp++ = pixels[*sp++];
	}
	sp += sa;
	dp += da;
    }
}
#endif

/**
**	Video draw part of graphic.
**
**	@param graphic	Pointer to object
**	@param gx	X offset into object
**	@param gy	Y offset into object
**	@param w	width to display
**	@param h	height to display
**	@param x	X screen position
**	@param y	Y screen position
*/
#ifdef USE_OPENGL
local void VideoDrawSubOpenGL(const Graphic* graphic, int gx, int gy,
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
    ey = VideoHeight - y;
    sy = ey - h;

    stx = (GLfloat)gx / graphic->Width * graphic->TextureWidth;
    etx = (GLfloat)(gx + w) / graphic->Width * graphic->TextureWidth;
    sty = (GLfloat)gy / graphic->Height * graphic->TextureHeight;
    ety = (GLfloat)(gy + h) / graphic->Height * graphic->TextureHeight;

    glBindTexture(GL_TEXTURE_2D, graphic->TextureNames[0]);
    glBegin(GL_QUADS);
    glTexCoord2f(stx, 1.0f - ety);
    glVertex2i(sx, sy);
    glTexCoord2f(stx, 1.0f - sty);
    glVertex2i(sx, ey);
    glTexCoord2f(etx, 1.0f - sty);
    glVertex2i(ex, ey);
    glTexCoord2f(etx, 1.0f - ety);
    glVertex2i(ex, sy);
    glEnd();
}
#endif

#ifdef USE_SDL_SURFACE
/*
local void VideoDrawSub(const Graphic* graphic, int gx, int gy,
    int w, int h, int x, int y)
{
    CLIP_RECTANGLE(x, y, w, h);
    VideoDrawSub(graphic, gx, gy, w, h, x, y);
}
*/
#else
/**
**	Video draw part of 8bit graphic clipped into 8 bit framebuffer.
**
**	@param graphic	Pointer to object
**	@param gx	X offset into object
**	@param gy	Y offset into object
**	@param w	width to display
**	@param h	height to display
**	@param x	X screen position
**	@param y	Y screen position
*/
local void VideoDrawSub8to8Clip(const Graphic* graphic, int gx, int gy,
    int w, int h, int x, int y)
{
    CLIP_RECTANGLE(x, y, w, h);
    VideoDrawSub8to8(graphic, gx, gy, w, h, x, y);
}

/**
**	Video draw part of 8bit graphic clipped into 16 bit framebuffer.
**
**	@param graphic	Pointer to object
**	@param gx	X offset into object
**	@param gy	Y offset into object
**	@param w	width to display
**	@param h	height to display
**	@param x	X screen position
**	@param y	Y screen position
*/
local void VideoDrawSub8to16Clip(const Graphic* graphic, int gx, int gy,
    int w, int h, int x, int y)
{
    CLIP_RECTANGLE(x, y, w, h);
    VideoDrawSub8to16(graphic, gx, gy, w, h, x, y);
}

/**
**	Video draw part of 8bit graphic clipped into 24 bit framebuffer.
**
**	@param graphic	Pointer to object
**	@param gx	X offset into object
**	@param gy	Y offset into object
**	@param w	width to display
**	@param h	height to display
**	@param x	X screen position
**	@param y	Y screen position
*/
local void VideoDrawSub8to24Clip(const Graphic* graphic, int gx, int gy,
    int w, int h, int x, int y)
{
    CLIP_RECTANGLE(x, y, w, h);
    VideoDrawSub8to24(graphic, gx, gy, w, h, x, y);
}

/**
**	Video draw part of 8bit graphic clipped into 32 bit framebuffer.
**
**	@param graphic	Pointer to object
**	@param gx	X offset into object
**	@param gy	Y offset into object
**	@param w	width to display
**	@param h	height to display
**	@param x	X screen position
**	@param y	Y screen position
*/
local void VideoDrawSub8to32Clip(const Graphic* graphic, int gx, int gy,
    int w, int h, int x, int y)
{
    CLIP_RECTANGLE(x, y, w, h);
    VideoDrawSub8to32(graphic, gx, gy, w, h, x, y);
}
#endif

/**
**	Video draw part of graphic clipped.
**
**	@param graphic	Pointer to object
**	@param gx	X offset into object
**	@param gy	Y offset into object
**	@param w	width to display
**	@param h	height to display
**	@param x	X screen position
**	@param y	Y screen position
*/
#ifdef USE_OPENGL
local void VideoDrawSubOpenGLClip(const Graphic* graphic, int gx, int gy,
    int w, int h, int x, int y)
{
    CLIP_RECTANGLE(x, y, w, h);
    VideoDrawSubOpenGL(graphic, gx, gy, w, h, x, y);
}
#endif

/**
**	Free graphic object.
*/
local void FreeGraphic8(Graphic* graphic)
{
#ifdef DEBUG
#ifdef USE_SDL_SURFACE
    // FIXME: todo
#else
    AllocatedGraphicMemory -= graphic->Size;
    AllocatedGraphicMemory -= sizeof(Graphic);
#endif
#endif

#ifdef USE_OPENGL
    if (graphic->NumTextureNames) {
	glDeleteTextures(graphic->NumTextureNames, graphic->TextureNames);
	free(graphic->TextureNames);
    }
#endif

#ifdef USE_SDL_SURFACE
    SDL_FreeSurface(graphic->Surface);
#else
    VideoFreeSharedPalette(graphic->Pixels);
    if (graphic->Palette) {
	free(graphic->Palette);
    }
    free(graphic->Frames);
    free(graphic);
#endif
}

// FIXME: need frame version

// FIXME: need 16 bit palette version

// FIXME: need zooming version


/*----------------------------------------------------------------------------
--	Global functions
----------------------------------------------------------------------------*/

/**
**	Video draw part of a graphic clipped and faded.
**
**	@param graphic	Pointer to object
**	@param gx	X offset into object
**	@param gy	Y offset into object
**	@param w	width to display
**	@param h	height to display
**	@param x	X screen position
**	@param y	Y screen position
**	@param fade	Amount faded, from 0 (black) to 255 (no fading)
*/
global void VideoDrawSubClipFaded(Graphic* graphic, int gx, int gy,
    int w, int h, int x, int y, unsigned char fade)
{
    VideoDrawSubClip(graphic, gx, gy, w, h, x, y);
    VideoFillTransRectangle(ColorBlack, x, y, w, h, fade);
}

/**
**	Make a graphic object.
**
**	@param depth	Pixel depth of the object (8,16,32)
**	@param width	Pixel width.
**	@param height	Pixel height.
**	@param data	Object data (malloced by caller, freed from object).
**	@param size	Size in bytes of the object data.
**
**	@return		New graphic object (malloced).
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
#ifdef USE_SDL_SURFACE
#else
    if (depth == 8) {
	graphic->Type = &GraphicImage8Type;
    } else if (depth == 16) {
	graphic->Type = &GraphicImage16Type;
    } else {
	fprintf(stderr, "Unsported image depth\n");
	ExitFatal(-1);
    }
#endif
    graphic->Width = width;
    graphic->Height = height;

#ifdef USE_SDL_SURFACE
    // FIXME: endian

//    graphic->Surface = SDL_CreateRGBSurface(SDL_SWSURFACE, width, height, depth, 0x000000ff, 0x0000ff00, 0x00ff0000, 0xff000000);
    graphic->Surface = SDL_CreateRGBSurfaceFrom(data, width, height, depth, width * depth / 8, 
//	0x000000ff, 0x0000ff00, 0x00ff0000, 0xff000000);
	0, 0, 0, 0);
//	depth * width);//, 0x000000ff, 0x0000ff00, 0x00ff0000, 0xff000000);

    graphic->NumFrames = 0;
#else
    graphic->Pixels = NULL;
    graphic->Palette = NULL;

    graphic->NumFrames = 0;
    graphic->Frames = data;
    graphic->Size = size;
#endif

#ifdef USE_OPENGL
    graphic->NumTextureNames = 0;
#endif

    return graphic;
}

/**
**	Make a new graphic object.
**
**	@param depth	Pixel depth of the object (8,16,32)
**	@param width	Pixel width.
**	@param height	Pixel height.
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
**	Make an OpenGL texture or textures out of a graphic object.
**
**	@param graphic	The graphic object.
**	@param width	Graphic width.
**	@param height	Graphic height.
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
	for (i = 0; i < height; ++i) {
	    sp = (const unsigned char*)graphic->Frames + (x % fl) * width +
		((x / fl) * height + i) * graphic->Width;
	    for (j = 0; j < width; ++j) {
		int c;
		Palette p;

		c = (h - i - 1) * w * 4 + j * 4;
		if (*sp == 255) {
		    tex[c + 3] = 0;
		} else {
		    p = graphic->Palette[*sp];
		    tex[c + 0] = p.r;
		    tex[c + 1] = p.g;
		    tex[c + 2] = p.b;
		    tex[c + 3] = 0xff;
		}
		++sp;
	    }
	}
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA,
	    GL_UNSIGNED_BYTE, tex);
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
**	Make an OpenGL texture of the player color pixels only.
**
**	FIXME: Docu
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

	(*g)->Type = graphic->Type;
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
    for (i = 0; i < graphic->Height; ++i) {
	sp = (const unsigned char*)graphic->Frames + (frame % fl) * graphic->Width +
	    ((frame / fl) * graphic->Height + i) * graphic->GraphicWidth;
	for (j = 0; j < graphic->Width; ++j) {
	    int c;
	    int z;
	    Palette p;

	    c = (h - i - 1) * w * 4 + j * 4;
	    for (z = 0; z < maplen; ++z) {
		if (*sp == map[z * 2]) {
		    p = GlobalPalette[map[z * 2 + 1]];
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
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA,
	GL_UNSIGNED_BYTE, tex);
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
**	Resize a graphic
**
**	@param g	Graphic object.
**	@param w	New width of graphic.
**	@param h	New height of graphic.
**
**	@todo	FIXME: Higher quality resizing.
**		FIXME: Works only with 8bit indexed graphic objects.
*/
global void ResizeGraphic(Graphic *g, int w, int h)
{
    int i;
    int j;
    unsigned char* data;
    int x;

#ifdef USE_SDL_SURFACE
    return;

    int bps;

    bps = g->Surface->format->BytesPerPixel;
#else
    DebugCheck(g->Type != &GraphicImage8Type);
#endif

    if (g->Width == w && g->Height == h) {
	return;
    }

    data = (unsigned char*)malloc(w * h);
#ifdef DEBUG
    AllocatedGraphicMemory += w * h;
#endif
    x = 0;

    for (i = 0; i < h; ++i) {
	for (j = 0; j < w; ++j) {
#ifdef USE_SDL_SURFACE
	    memcpy(&data[x * g->Surface->format->BytesPerPixel],
		&((char*)g->Surface->pixels)[(i * g->Height / h) * g->Width * bps
		    + bps * j * g->Width / w], bps);
#else
	    data[x] = ((unsigned char*)g->Frames)[
		(i * g->Height / h) * g->Width + j * g->Width / w];
#endif
	    ++x;
	}
    }

#ifdef USE_SDL_SURFACE
    // FIXME: todo
    SDL_FreeSurface(g->Surface);
    g->Surface = SDL_CreateRGBSurfaceFrom(data, w, h, bps, w * bps, 0, 0, 0, 0);
    g->Width = w;
    g->Height = h;
#else
    free(g->Frames);
#ifdef DEBUG
    AllocatedGraphicMemory -= g->Width * g->Height;
#endif
    g->Frames = data;
    g->Width = w;
    g->Height = h;
#endif
}

/**
**	Load graphic from file.
**
**	@param name	File name.
**
**	@return		Graphic object.
**
**	@todo		FIXME: I want also support JPG file format!
**			FIXME: I want to support our own binary format!
**			FIXME: Add support for 16bit indexed format!
*/
global Graphic* LoadGraphic(const char* name)
{
    Graphic* graphic;
    char buf[1024];

    if (!(graphic = LoadGraphicPNG(LibraryFileName(name, buf)))) {
	fprintf(stderr, "Can't load the graphic `%s'\n", name);
	ExitFatal(-1);
    }

#ifdef USE_SDL_SURFACE
//    Pixels = graphic->Surface->format->palette;
//    graphic->Surface->format->palette = 
//	VideoCreateSharedPalette(graphic->Surface->format->palette);
    graphic->NumFrames = 1;
#else
    graphic->Pixels = VideoCreateSharedPalette(graphic->Palette);
    //free(graphic->Palette);
    //graphic->Palette = NULL;		// JOHNS: why should we free this?
#endif

    return graphic;
}

/**
**	Init graphic
*/
global void InitGraphic(void)
{
#ifdef USE_SDL_SURFACE
//    GlobalPalletPixels->ncolors = 256;
//    GlobalPalette.colors = (SDL_Color*)calloc(256, sizeof(SDL_Color));
//    GlobalPalette.ncolors = 256;
//    GlobalPalette.colors = (SDL_Color*)calloc(256, sizeof(SDL_Color));
#else

#ifdef USE_OPENGL
    GraphicImage8Type.DrawSub = VideoDrawSubOpenGL;
    GraphicImage8Type.DrawSubClip = VideoDrawSubOpenGLClip;
#else

    switch( VideoBpp ) {
	case 8:
	    GraphicImage8Type.DrawSub = VideoDrawSub8to8;
	    GraphicImage8Type.DrawSubClip = VideoDrawSub8to8Clip;
	    break;

	case 15:
	case 16:
	    GraphicImage8Type.DrawSub = VideoDrawSub8to16;
	    GraphicImage8Type.DrawSubClip = VideoDrawSub8to16Clip;
	    break;

	case 24:
	    GraphicImage8Type.DrawSub = VideoDrawSub8to24;
	    GraphicImage8Type.DrawSubClip = VideoDrawSub8to24Clip;
	    break;

	case 32:
	    GraphicImage8Type.DrawSub = VideoDrawSub8to32;
	    GraphicImage8Type.DrawSubClip = VideoDrawSub8to32Clip;
	    break;

	default:
	    DebugLevel0Fn("unsupported %d bpp\n" _C_ VideoBpp);
	    abort();
    }
#endif
    GraphicImage8Type.Free = FreeGraphic8;
#endif
}

//@}
