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
/**@name sprite.c	-	The general sprite functions. */
//
//	(c) Copyright 2000-2002 by Lutz Sammer, Stephan Rasenberg, 
//	Nehal Mistry
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
#include <string.h>

#include "stratagus.h"
#include "video.h"

#include "intern_video.h"

/*----------------------------------------------------------------------------
--	Declarations
----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
--	Variables
----------------------------------------------------------------------------*/

local GraphicType GraphicSprite8Type;	/// sprite type 8bit palette
local GraphicType GraphicSprite16Type;	/// sprite type 16bit palette

#ifdef USE_SDL_SURFACE
global void VideoDrawRawClip(SDL_Surface *surface,
    int x, int y, int w, int h);
#else
global void (*VideoDrawRawClip)(VMemType *pixels, const unsigned char *data,
    int x, int y, int w, int h);
#endif

/*----------------------------------------------------------------------------
--	Local functions
----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
--	RLE Sprites
----------------------------------------------------------------------------*/

#ifdef USE_SDL_SURFACE
local void VideoDraw(const Graphic* sprite, unsigned frame, int x, int y)
{
    SDL_Rect srect;
    SDL_Rect drect;

    srect.x = (frame % (sprite->Surface->w / sprite->Width)) * sprite->Width;
    srect.y = (frame / (sprite->Surface->w / sprite->Width)) * sprite->Height;
    srect.w = sprite->Width;
    srect.h = sprite->Height;

    drect.x = x;
    drect.y = y;

    SDL_BlitSurface(sprite->Surface, &srect, TheScreen, &drect);
}

global void VideoDrawClip(const Graphic* sprite, unsigned frame, int x, int y)
{
    SDL_Rect srect;
    SDL_Rect drect;
    int oldx;
    int oldy;

    srect.x = (frame % (sprite->Surface->w / sprite->Width)) * sprite->Width;
    srect.y = (frame / (sprite->Surface->w / sprite->Width)) * sprite->Height;
    srect.w = sprite->Width;
    srect.h = sprite->Height;

    oldx = x;
    oldy = y;
    CLIP_RECTANGLE(x, y, srect.w, srect.h);
    srect.x += x - oldx;
    srect.y += y - oldy;

    drect.x = x;
    drect.y = y;

    SDL_BlitSurface(sprite->Surface, &srect, TheScreen, &drect);
}

local void VideoDrawX(const Graphic* sprite, unsigned frame, int x, int y)
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

local void VideoDrawClipX(const Graphic* sprite, unsigned frame, int x, int y)
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

local void VideoDrawShadowClip(const Graphic* sprite, unsigned frame,
    int x, int y)
{
    SDL_Rect srect;
    SDL_Rect drect;
    int oldx;
    int oldy;
    unsigned char alpha;

    srect.x = (frame % (sprite->Surface->w / sprite->Width)) * sprite->Width;
    srect.y = (frame / (sprite->Surface->w / sprite->Width)) * sprite->Height;
    srect.w = sprite->Width;
    srect.h = sprite->Height;

    oldx = x;
    oldy = y;
    CLIP_RECTANGLE(x, y, srect.w, srect.h);
    srect.x += x - oldx;
    srect.y += y - oldy;

    drect.x = x;
    drect.y = y;

    alpha = sprite->Surface->format->alpha;
    SDL_SetAlpha(sprite->Surface, SDL_SRCALPHA | SDL_RLEACCEL, 128);
    SDL_BlitSurface(sprite->Surface, &srect, TheScreen, &drect);
    SDL_SetAlpha(sprite->Surface, SDL_SRCALPHA | SDL_RLEACCEL, alpha);
}

local void VideoDrawShadowClipX(const Graphic* sprite, unsigned frame,
    int x, int y)
{
    SDL_Rect srect;
    SDL_Rect drect;
    int oldx;
    int oldy;
    unsigned char alpha;

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

    alpha = sprite->Surface->format->alpha;
    SDL_SetAlpha(sprite->Surface, SDL_SRCALPHA | SDL_RLEACCEL, 128);
    SDL_BlitSurface(sprite->SurfaceFlip, &srect, TheScreen, &drect);
    SDL_SetAlpha(sprite->Surface, SDL_SRCALPHA | SDL_RLEACCEL, alpha);
}
#else
//
//	The current implementation uses RLE encoded sprites.
//	If you know something better, write it.
//
//	The encoding translates the sprite data to a stream of segments
//	of the form:
//
//	<skip> <run> <data>
//
//	where	<skip> is the number of transparent pixels to skip,
//		<run>  is the number of opaque pixels to blit,
//	 and	<data> are the pixels themselves.
//
//	<skip> and <run> are unsigned 8 bit integers.
//	If more than 255 transparent pixels are needed 255 0 <n> <run> is
//	used. <run> is always stored, even 0 at the end of line.
//	This makes the pixel data aligned at all times.
//	Segments never wrap around from one scan line to the next.
//

/**
**	Draw a RLE encoded graphic object unclipped into framebuffer.
**
**	@note	This macro looks nice, but is absolutly no debugable.
**	@todo	Make this an inline function.
**
**	@param bpp	Bit depth of target framebuffer
**	@param sprite	pointer to object
**	@param frame	number of frame (object index)
**	@param x	x coordinate on the screen
**	@param y	y coordinate on the screen
*/
#define RLE_BLIT(bpp, sprite, frame, x, y)	\
    do {							\
	const unsigned char* sp;				\
	unsigned w;						\
	VMemType##bpp* dp;					\
	const VMemType##bpp* lp;				\
	const VMemType##bpp* ep;				\
	const VMemType##bpp* pixels;				\
	const VMemType##bpp* pp;				\
	unsigned da;						\
								\
	pixels = (VMemType##bpp*)sprite->Pixels;		        \
	sp = ((unsigned char**)sprite->Frames)[frame];		\
	w = sprite->Width;					\
	da = VideoWidth-w;					\
	dp = VideoMemory##bpp + x + y * VideoWidth;			\
	ep = dp + VideoWidth * sprite->Height;			\
								\
	do {							\
	    lp = dp + w;						\
	    do {			/* 1 line */		\
		dp += *sp++;		/* transparent # */	\
		pp = dp - 1 + *sp++;		/* opaque # */		\
		while (dp < pp) {	/* unrolled */		\
		    *dp++ = pixels[*sp++];			\
		    *dp++ = pixels[*sp++];			\
		}						\
		if (dp <= pp) {					\
		    *dp++ = pixels[*sp++];			\
		}						\
	    } while (dp < lp);					\
	    dp += da;						\
	} while (dp < ep);		/* all lines */		\
    } while (0)

/**
**	Draw 8bit graphic object unclipped into 8 bit framebuffer.
**
**	@param sprite	pointer to object
**	@param frame	number of frame (object index)
**	@param x	x coordinate on the screen
**	@param y	y coordinate on the screen
*/
local void VideoDraw8to8(const Graphic* sprite, unsigned frame, int x, int y)
{
    RLE_BLIT(8, sprite, frame, x, y);
}

/**
**	Draw 8bit graphic object unclipped into 16 bit framebuffer.
**
**	@param sprite	pointer to object
**	@param frame	number of frame (object index)
**	@param x	x coordinate on the screen
**	@param y	y coordinate on the screen
*/
local void VideoDraw8to16(const Graphic* sprite, unsigned frame, int x, int y)
{
    RLE_BLIT(16, sprite, frame, x, y);
}

/**
**	Draw 8bit graphic object unclipped into 24 bit framebuffer.
**
**	@param sprite	pointer to object
**	@param frame	number of frame (object index)
**	@param x	x coordinate on the screen
**	@param y	y coordinate on the screen
*/
local void VideoDraw8to24(const Graphic* sprite, unsigned frame, int x, int y)
{
    RLE_BLIT(24, sprite, frame, x, y);
}

/**
**	Draw 8bit graphic object unclipped into 32 bit framebuffer.
**
**	@param sprite	pointer to object
**	@param frame	number of frame (object index)
**	@param x	x coordinate on the screen
**	@param y	y coordinate on the screen
*/
local void VideoDraw8to32(const Graphic* sprite, unsigned frame, int x, int y)
{
    RLE_BLIT(32, sprite, frame, x, y);
}
#endif

/**
**	Draw graphic object unclipped.
**
**	@param sprite	pointer to object
**	@param frame	number of frame (object index)
**	@param x	x coordinate on the screen
**	@param y	y coordinate on the screen
*/
#ifdef USE_OPENGL
local void VideoDrawOpenGL(const Graphic* sprite, unsigned frame, int x, int y)
{
    GLint sx;
    GLint ex;
    GLint sy;
    GLint ey;

    sx = x;
    ex = sx + sprite->Width;
    ey = VideoHeight - y;
    sy = ey - sprite->Height;

    glBindTexture(GL_TEXTURE_2D, sprite->TextureNames[frame]);
    glBegin(GL_QUADS);
    glTexCoord2f(0.0f, 1.0f - sprite->TextureHeight);
    glVertex2i(sx, sy);
    glTexCoord2f(0.0f, 1.0f);
    glVertex2i(sx, ey);
    glTexCoord2f(sprite->TextureWidth, 1.0f);
    glVertex2i(ex, ey);
    glTexCoord2f(sprite->TextureWidth, 1.0f - sprite->TextureHeight);
    glVertex2i(ex, sy);
    glEnd();
}
#endif

#ifndef USE_SDL_SURFACE
/**
**	Draw 8bit graphic object unclipped and flipped in X direction
**	into 8 bit framebuffer.
**
**	@param sprite	pointer to object
**	@param frame	number of frame (object index)
**	@param x	x coordinate on the screen
**	@param y	y coordinate on the screen
*/
local void VideoDraw8to8X(const Graphic* sprite, unsigned frame, int x, int y)
{
    const unsigned char* sp;
    unsigned w;
    VMemType8* dp;
    const VMemType8* lp;
    const VMemType8* ep;
    const VMemType8* pp;
    const VMemType8* pixels;
    unsigned da;

    pixels = (VMemType8*)sprite->Pixels;
    sp = ((unsigned char**)sprite->Frames)[frame];
    w = sprite->Width;
    dp = VideoMemory8 + x + y * VideoWidth + w - 1;
    da = VideoWidth + w;
    ep = dp + VideoWidth * sprite->Height;
    do {
	lp = dp - w;
	do {				// 1 line
	    dp -= *sp++;			// transparent
	    pp = dp + 1 - *sp++;		// opaque
	    while (dp > pp) {		// unrolled
		*dp-- = pixels[*sp++];
		*dp-- = pixels[*sp++];
	    }
	    if (dp >= pp) {
		*dp-- = pixels[*sp++];
	    }
	} while (dp > lp);
	dp += da;
    } while (dp < ep);			// all lines
}

/**
**	Draw 8bit graphic object unclipped and flipped in X direction
**	into 16 bit framebuffer.
**
**	@param sprite	pointer to object
**	@param frame	number of frame (object index)
**	@param x	x coordinate on the screen
**	@param y	y coordinate on the screen
*/
local void VideoDraw8to16X(const Graphic* sprite, unsigned frame, int x, int y)
{
    const unsigned char* sp;
    unsigned w;
    VMemType16* dp;
    const VMemType16* lp;
    const VMemType16* ep;
    const VMemType16* pp;
    const VMemType16* pixels;
    unsigned da;

    pixels = (VMemType16*)sprite->Pixels;
    sp = ((unsigned char**)sprite->Frames)[frame];
    w = sprite->Width;
    dp = VideoMemory16 + x + y * VideoWidth + w - 1;
    da = VideoWidth + w;
    ep = dp + VideoWidth * sprite->Height;

    do {
	lp = dp - w;
	do {				// 1 line
	    dp -= *sp++;			// transparent
	    pp = dp + 1 - *sp++;		// opaque
	    while (dp > pp) {		// unrolled
		*dp-- = pixels[*sp++];
		*dp-- = pixels[*sp++];
	    }
	    if (dp >= pp) {
		*dp-- = pixels[*sp++];
	    }
	} while (dp > lp);
	dp += da;
    } while (dp < ep);			// all lines
}

/**
**	Draw 8bit graphic object unclipped and flipped in X direction
**	into 24 bit framebuffer.
**
**	@param sprite	pointer to object
**	@param frame	number of frame (object index)
**	@param x	x coordinate on the screen
**	@param y	y coordinate on the screen
*/
local void VideoDraw8to24X(const Graphic* sprite, unsigned frame, int x, int y)
{
    const unsigned char* sp;
    unsigned w;
    VMemType24* dp;
    const VMemType24* lp;
    const VMemType24* ep;
    const VMemType24* pp;
    const VMemType24* pixels;
    unsigned da;

    pixels = (VMemType24*)sprite->Pixels;
    sp = ((unsigned char**)sprite->Frames)[frame];
    w = sprite->Width;
    dp = VideoMemory24 + x + y * VideoWidth + w - 1;
    da = VideoWidth + w;
    ep = dp + VideoWidth * sprite->Height;

    do {
	lp = dp - w;
	do {				// 1 line
	    dp -= *sp++;			// transparent
	    pp = dp + 1 - *sp++;		// opaque
	    while (dp > pp) {		// unrolled
		*dp-- = pixels[*sp++];
		*dp-- = pixels[*sp++];
	    }
	    if (dp >= pp) {
		*dp-- = pixels[*sp++];
	    }
	} while (dp > lp);
	dp += da;
    } while (dp < ep);			// all lines
}

/**
**	Draw 8bit graphic object unclipped and flipped in X direction
**	into 32 bit framebuffer.
**
**	@param sprite	pointer to object
**	@param frame	number of frame (object index)
**	@param x	x coordinate on the screen
**	@param y	y coordinate on the screen
*/
local void VideoDraw8to32X(const Graphic* sprite, unsigned frame, int x, int y)
{
    const unsigned char* sp;
    unsigned w;
    VMemType32* dp;
    const VMemType32* lp;
    const VMemType32* ep;
    const VMemType32* pp;
    const VMemType32* pixels;
    unsigned da;

    pixels = (VMemType32*)sprite->Pixels;
    sp = ((unsigned char**)sprite->Frames)[frame];
    w = sprite->Width;
    dp = VideoMemory32 + x + y * VideoWidth + w - 1;
    da = VideoWidth + w;
    ep = dp + VideoWidth * sprite->Height;

    do {
	lp = dp - w;
	do {				// 1 line
	    dp -= *sp++;			// transparent
	    pp = dp + 1 - *sp++;		// opaque
	    while (dp > pp) {		// unrolled
		*dp-- = pixels[*sp++];
		*dp-- = pixels[*sp++];
	    }
	    if (dp >= pp) {
		*dp-- = pixels[*sp++];
	    }
	} while (dp > lp);
	dp += da;
    } while (dp < ep);			// all lines
}
#endif

/**
**	Draw graphic object unclipped and flipped in X direction.
**
**	@param sprite	pointer to object
**	@param frame	number of frame (object index)
**	@param x	x coordinate on the screen
**	@param y	y coordinate on the screen
*/
#ifdef USE_OPENGL
local void VideoDrawXOpenGL(const Graphic* sprite, unsigned frame, int x, int y)
{
    GLint sx;
    GLint ex;
    GLint sy;
    GLint ey;

    sx = x;
    ex = sx + sprite->Width;
    ey = VideoHeight - y;
    sy = ey - sprite->Height;

    glBindTexture(GL_TEXTURE_2D, sprite->TextureNames[frame]);
    glBegin(GL_QUADS);
    glTexCoord2f(0.0f, 1.0f - sprite->TextureHeight);
    glVertex2i(sx, sy);
    glTexCoord2f(0.0f, 1.0f);
    glVertex2i(sx, ey);
    glTexCoord2f(sprite->TextureWidth, 1.0f);
    glVertex2i(ex, ey);
    glTexCoord2f(sprite->TextureWidth, 1.0f - sprite->TextureHeight);
    glVertex2i(ex, sy);
    glEnd();
}
#endif

#ifndef USE_SDL_SURFACE
/**
**	Draw 8bit graphic object clipped into 8 bit framebuffer.
**
**	@param sprite	pointer to object
**	@param frame	number of frame (object index)
**	@param x	x coordinate on the screen
**	@param y	y coordinate on the screen
*/
local void VideoDraw8to8Clip(const Graphic* sprite, unsigned frame, int x, int y)
{
    int ox;
    int ex;
    int oy;
    int w;
    int h;
    const unsigned char* sp;
    int sw;
    VMemType8* dp;
    const VMemType8* lp;
    const VMemType8* ep;
    VMemType8* pp;
    const VMemType8* pixels;
    int da;


    //
    // reduce to visible range
    //
    sw = w = sprite->Width;
    h = sprite->Height;
    CLIP_RECTANGLE_OFS(x, y, w, h, ox, oy, ex);

    //
    //	Draw the clipped sprite
    //
    pixels = (VMemType8*)sprite->Pixels;
    sp = ((unsigned char**)sprite->Frames)[frame];

    //
    //	Skip top lines, if needed.
    //
    while (oy--) {
	da = 0;
	do {
	    da += *sp++;			// transparent
	    da += *sp;			// opaque
	    sp += *sp + 1;
	} while (da < sw);
    }

    da = VideoWidth - sw;
    dp = VideoMemory8 + x + y * VideoWidth;
    ep = dp + VideoWidth * h;

    if (w == sw) {			// Unclipped horizontal

	do {
	    lp = dp + sw;
	    do {			// 1 line
		dp += *sp++;		// transparent
		pp = dp - 1 + *sp++;		// opaque
		while (dp < pp) {	// unroll
		    *dp++ = pixels[*sp++];
		    *dp++ = pixels[*sp++];
		}
		if (dp <= pp) {
		    *dp++ = pixels[*sp++];
		}
	    } while (dp < lp);
	    dp += da;
	} while (dp < ep);		// all lines

    } else {				// Clip horizontal

	da += ox;
	do {
	    lp = dp + w;
	    //
	    //	Clip left
	    //
	    pp = dp - ox;
	    for (;;) {
		pp += *sp++;		// transparent
		if (pp >= dp) {
		    dp = pp;
		    goto middle_trans;
		}
		pp += *sp;		// opaque
		if (pp >= dp) {
		    sp += *sp - (pp - dp) + 1;
		    goto middle_pixel;
		}
		sp += *sp + 1;
	    }

	    //
	    //	Draw middle
	    //
	    for (;;) {
		dp += *sp++;		// transparent
middle_trans:
		if (dp >= lp) {
		    lp += sw - w - ox;
		    goto right_trans;
		}
		pp = dp + *sp++;		// opaque
middle_pixel:
		if (pp < lp) {
		    while (dp < pp) {
			*dp++ = pixels[*sp++];
		    }
		    continue;
		}
		while (dp < lp) {
		    *dp++ = pixels[*sp++];
		}
		sp += pp - dp;
		dp = pp;
		break;
	    }

	    //
	    //	Clip right
	    //
	    lp += sw - w - ox;
	    while (dp < lp) {
		dp += *sp++;		// transparent
right_trans:
		dp += *sp;		// opaque
		sp += *sp + 1;
	    }
	    dp += da;
	} while (dp < ep);		// all lines

    }
}

/**
**	Draw 8bit graphic object clipped into 16 bit framebuffer.
**
**	@param sprite	pointer to object
**	@param frame	number of frame (object index)
**	@param x	x coordinate on the screen
**	@param y	y coordinate on the screen
*/
local void VideoDraw8to16Clip(const Graphic* sprite, unsigned frame, int x, int y)
{
    int ox;
    int ex;
    int oy;
    int w;
    int h;
    const unsigned char* sp;
    int sw;
    VMemType16* dp;
    const VMemType16* lp;
    const VMemType16* ep;
    VMemType16* pp;
    const VMemType16* pixels;
    int da;

    //
    // reduce to visible range
    //
    sw = w = sprite->Width;
    h = sprite->Height;
    CLIP_RECTANGLE_OFS(x, y, w, h, ox, oy, ex);

    //
    //	Draw the clipped sprite
    //
    pixels = (VMemType16*)sprite->Pixels;
    sp = ((unsigned char**)sprite->Frames)[frame];

    //
    //	Skip top lines, if needed.
    //
    while (oy--) {
	da = 0;
	do {
	    da += *sp++;			// transparent
	    da += *sp;			// opaque
	    sp += *sp + 1;
	} while (da < sw);
    }

    da = VideoWidth - sw;
    dp = VideoMemory16 + x + y * VideoWidth;
    ep = dp + VideoWidth * h;

    if (w == sw) {			// Unclipped horizontal

	do {
	    lp = dp + sw;
	    do {			// 1 line
		dp += *sp++;		// transparent
		pp = dp - 1 + *sp++;		// opaque
		while (dp < pp) {	// unroll
		    *dp++ = pixels[*sp++];
		    *dp++ = pixels[*sp++];
		}
		if (dp <= pp) {
		    *dp++ = pixels[*sp++];
		}
	    } while (dp < lp);
	    dp += da;
	} while (dp < ep);		// all lines

    } else {				// Clip horizontal

	da += ox;
	do {
	    lp = dp + w;
	    //
	    //	Clip left
	    //
	    pp = dp - ox;
	    for (;;) {
		pp += *sp++;		// transparent
		if (pp >= dp) {
		    dp = pp;
		    goto middle_trans;
		}
		pp += *sp;		// opaque
		if (pp >= dp) {
		    sp += *sp - (pp - dp) + 1;
		    goto middle_pixel;
		}
		sp += *sp + 1;
	    }

	    //
	    //	Draw middle
	    //
	    for (;;) {
		dp += *sp++;		// transparent
middle_trans:
		if (dp >= lp) {
		    lp += sw - w - ox;
		    goto right_trans;
		}
		pp = dp + *sp++;		// opaque
middle_pixel:
		if (pp < lp) {
		    while (dp < pp) {
			*dp++ = pixels[*sp++];
		    }
		    continue;
		}
		while (dp < lp) {
		    *dp++ = pixels[*sp++];
		}
		sp += pp - dp;
		dp = pp;
		break;
	    }

	    //
	    //	Clip right
	    //
	    lp += sw - w - ox;
	    while (dp < lp) {
		dp += *sp++;		// transparent
right_trans:
		dp += *sp;		// opaque
		sp += *sp + 1;
	    }
	    dp += da;
	} while (dp < ep);		// all lines

    }
}

/**
**	Draw 8bit graphic object clipped into 24 bit framebuffer.
**
**	@param sprite	pointer to object
**	@param frame	number of frame (object index)
**	@param x	x coordinate on the screen
**	@param y	y coordinate on the screen
*/
local void VideoDraw8to24Clip(const Graphic* sprite, unsigned frame, int x, int y)
{
    int ox;
    int ex;
    int oy;
    int w;
    int h;
    const unsigned char* sp;
    int sw;
    VMemType24* dp;
    const VMemType24* lp;
    const VMemType24* ep;
    VMemType24* pp;
    const VMemType24* pixels;
    int da;

    //
    // reduce to visible range
    //
    sw = w = sprite->Width;
    h = sprite->Height;
    CLIP_RECTANGLE_OFS(x, y, w, h, ox, oy, ex);

    //
    //	Draw the clipped sprite
    //
    pixels = (VMemType24*)sprite->Pixels;
    sp = ((unsigned char**)sprite->Frames)[frame];

    //
    //	Skip top lines, if needed.
    //
    while (oy--) {
	da = 0;
	do {
	    da += *sp++;			// transparent
	    da += *sp;			// opaque
	    sp += *sp + 1;
	} while (da < sw);
    }

    da = VideoWidth - sw;
    dp = VideoMemory24 + x + y * VideoWidth;
    ep = dp + VideoWidth * h;

    if (w == sw) {			// Unclipped horizontal

	do {
	    lp = dp + sw;
	    do {			// 1 line
		dp += *sp++;		// transparent
		pp = dp - 1 + *sp++;		// opaque
		while (dp < pp) {	// unroll
		    *dp++ = pixels[*sp++];
		    *dp++ = pixels[*sp++];
		}
		if (dp <= pp) {
		    *dp++ = pixels[*sp++];
		}
	    } while (dp < lp);
	    dp += da;
	} while (dp < ep);		// all lines

    } else {				// Clip horizontal

	da += ox;
	do {
	    lp = dp + w;
	    //
	    //	Clip left
	    //
	    pp = dp - ox;
	    for (;;) {
		pp += *sp++;		// transparent
		if (pp >= dp) {
		    dp = pp;
		    goto middle_trans;
		}
		pp += *sp;		// opaque
		if (pp >= dp) {
		    sp += *sp - (pp - dp) + 1;
		    goto middle_pixel;
		}
		sp += *sp + 1;
	    }

	    //
	    //	Draw middle
	    //
	    for (;;) {
		dp += *sp++;		// transparent
middle_trans:
		if (dp >= lp) {
		    lp += sw - w - ox;
		    goto right_trans;
		}
		pp = dp + *sp++;		// opaque
middle_pixel:
		if (pp < lp) {
		    while (dp < pp) {
			*dp++ = pixels[*sp++];
		    }
		    continue;
		}
		while (dp < lp) {
		    *dp++ = pixels[*sp++];
		}
		sp += pp - dp;
		dp = pp;
		break;
	    }

	    //
	    //	Clip right
	    //
	    lp += sw - w - ox;
	    while (dp < lp) {
		dp += *sp++;		// transparent
right_trans:
		dp += *sp;		// opaque
		sp += *sp + 1;
	    }
	    dp += da;
	} while (dp < ep);		// all lines

    }
}

/**
**	Draw 8bit graphic object clipped into 32 bit framebuffer.
**
**	@param sprite	pointer to object
**	@param frame	number of frame (object index)
**	@param x	x coordinate on the screen
**	@param y	y coordinate on the screen
*/
local void VideoDraw8to32Clip(const Graphic* sprite, unsigned frame, int x, int y)
{
    int ox;
    int ex;
    int oy;
    int w;
    int h;
    const unsigned char* sp;
    int sw;
    VMemType32* dp;
    const VMemType32* lp;
    const VMemType32* ep;
    VMemType32* pp;
    const VMemType32* pixels;
    int da;

    //
    // reduce to visible range
    //
    sw = w = sprite->Width;
    h = sprite->Height;
    CLIP_RECTANGLE_OFS(x, y, w, h, ox, oy, ex);

    //
    //	Draw the clipped sprite
    //
    pixels = (VMemType32*)sprite->Pixels;
    sp = ((unsigned char**)sprite->Frames)[frame];

    //
    //	Skip top lines, if needed.
    //
    while (oy--) {
	da = 0;
	do {
	    da += *sp++;			// transparent
	    da += *sp;			// opaque
	    sp += *sp + 1;
	} while (da < sw);
    }

    da = VideoWidth - sw;
    dp = VideoMemory32 + x + y * VideoWidth;
    ep = dp + VideoWidth * h;

    if (w == sw) {			// Unclipped horizontal

	do {
	    lp = dp + sw;
	    do {			// 1 line
		dp += *sp++;		// transparent
		pp = dp - 1 + *sp++;		// opaque
		while (dp < pp) {	// unroll
		    *dp++ = pixels[*sp++];
		    *dp++ = pixels[*sp++];
		}
		if (dp <= pp) {
		    *dp++ = pixels[*sp++];
		}
	    } while (dp < lp);
	    dp += da;
	} while (dp < ep);		// all lines

    } else {				// Clip horizontal

	da += ox;
	do {
	    lp = dp + w;
	    //
	    //	Clip left
	    //
	    pp = dp - ox;
	    for (;;) {
		pp += *sp++;		// transparent
		if (pp >= dp) {
		    dp = pp;
		    goto middle_trans;
		}
		pp += *sp;		// opaque
		if (pp >= dp) {
		    sp += *sp - (pp - dp) + 1;
		    goto middle_pixel;
		}
		sp += *sp + 1;
	    }

	    //
	    //	Draw middle
	    //
	    for (;;) {
		dp += *sp++;		// transparent
middle_trans:
		if (dp >= lp) {
		    lp += sw - w - ox;
		    goto right_trans;
		}
		pp = dp + *sp++;		// opaque
middle_pixel:
		if (pp < lp) {
		    while (dp < pp) {
			*dp++ = pixels[*sp++];
		    }
		    continue;
		}
		while (dp < lp) {
		    *dp++ = pixels[*sp++];
		}
		sp += pp - dp;
		dp = pp;
		break;
	    }

	    //
	    //	Clip right
	    //
	    lp += sw - w - ox;
	    while (dp < lp) {
		dp += *sp++;		// transparent
right_trans:
		dp += *sp;		// opaque
		sp += *sp + 1;
	    }
	    dp += da;
	} while (dp < ep);		// all lines

    }
}
#endif

/**
**	Draw graphic object clipped.
**
**	@param sprite	pointer to object
**	@param frame	number of frame (object index)
**	@param x	x coordinate on the screen
**	@param y	y coordinate on the screen
*/
#ifdef USE_OPENGL
local void VideoDrawClipOpenGL(const Graphic* sprite, unsigned frame, int x, int y)
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
    evy = VideoHeight - y;
    svy = evy - h;

    stx = (GLfloat)ox / sprite->Width * sprite->TextureWidth;
    etx = (GLfloat)(ox + w) / sprite->Width * sprite->TextureWidth;
    ety = 1.0f - (GLfloat)oy / sprite->Height * sprite->TextureHeight;
    sty = 1.0f - (GLfloat)(oy + h) / sprite->Height * sprite->TextureHeight;

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

#ifdef USE_SDL_SURFACE

#else
/**
**	Draw 8bit graphic object clipped and flipped in X direction
**	into 8 bit framebuffer.
**
**	@param sprite	pointer to object
**	@param frame	number of frame (object index)
**	@param x	x coordinate on the screen
**	@param y	y coordinate on the screen
*/
local void VideoDraw8to8ClipX(const Graphic* sprite, unsigned frame, int x, int y)
{
    int ox;
    int ex;
    int oy;
    int w;
    int h;
    const unsigned char* sp;
    int sw;
    VMemType8* dp;
    const VMemType8* lp;
    const VMemType8* ep;
    VMemType8* pp;
    const VMemType8* pixels;
    int da;

    //
    // reduce to visible range
    //
    sw = w = sprite->Width;
    h = sprite->Height;
    CLIP_RECTANGLE_OFS(x, y, w, h, ox, oy, ex);

    //
    //	Draw the clipped sprite
    //
    pixels = (VMemType8*)sprite->Pixels;
    sp = ((unsigned char**)sprite->Frames)[frame];

    //
    // Skip top lines
    //
    while (oy--) {
	da = 0;
	do {
	    da += *sp++;			// transparent
	    da += *sp;			// opaque
	    sp += *sp + 1;
	} while (da < sw);
    }

    da = VideoWidth + sw;
    dp = VideoMemory8 + x + y * VideoWidth + w - 1;
    ep = dp + VideoWidth * h;

    if (w == sw) {			// Unclipped horizontal

	while (dp < ep) {		// all lines
	    lp = dp - w;
	    do {			// 1 line
		dp -= *sp++;		// transparent
		pp = dp + 1 - *sp++;		// opaque
		while (dp > pp) {
		    *dp-- = pixels[*sp++];
		    *dp-- = pixels[*sp++];
		}
		if (dp >= pp) {
		    *dp-- = pixels[*sp++];
		}
	    } while (dp > lp);
	    dp += da;
	}

    } else {				// Clip horizontal

	da -= sw - w - ox;
	while (dp < ep) {		// all lines
	    lp = dp - w;
	    //
	    //	Clip right side
	    //
	    pp = dp + sw - w - ox;
	    for (;;) {
		pp -= *sp++;		// transparent
		if (pp <= dp) {
		    dp = pp;
		    goto middle_trans;
		}
		pp -= *sp;		// opaque
		if (pp <= dp) {
		    sp += *sp - (dp - pp) + 1;
		    goto middle_pixel;
		}
		sp += *sp + 1;
	    }

	    //
	    //	Draw middle
	    //
	    for (;;) {
		dp -= *sp++;		// transparent
middle_trans:
		if (dp <= lp) {
		    lp -= ox;
		    goto right_trans;
		}
		pp = dp - *sp++;		// opaque
middle_pixel:
		if (pp > lp) {
		    while (dp > pp) {
			*dp-- = pixels[*sp++];
		    }
		    continue;
		}
		while (dp > lp) {
		    *dp-- = pixels[*sp++];
		}
		sp += dp - pp;
		dp = pp;
		break;
	    }

	    //
	    //	Clip left side
	    //
	    lp -= ox;
	    while (dp > lp) {
		dp -= *sp++;		// transparent
right_trans:
		dp -= *sp;		// opaque
		sp += *sp + 1;
	    }
	    dp += da;

	}
    }
}

/**
**	Draw 8bit graphic object clipped and flipped in X direction
**	into 16 bit framebuffer.
**
**	@param sprite	pointer to object
**	@param frame	number of frame (object index)
**	@param x	x coordinate on the screen
**	@param y	y coordinate on the screen
*/
local void VideoDraw8to16ClipX(const Graphic* sprite, unsigned frame,
    int x, int y)
{
    int ox;
    int ex;
    int oy;
    int w;
    int h;
    const unsigned char* sp;
    int sw;
    VMemType16* dp;
    const VMemType16* lp;
    const VMemType16* ep;
    VMemType16* pp;
    const VMemType16* pixels;
    int da;


    //
    // reduce to visible range
    //
    sw = w = sprite->Width;
    h = sprite->Height;
    CLIP_RECTANGLE_OFS(x, y, w, h, ox, oy, ex);

    //
    //	Draw the clipped sprite
    //
    pixels = (VMemType16*)sprite->Pixels;
    sp = ((unsigned char**)sprite->Frames)[frame];

    //
    // Skip top lines
    //
    while (oy--) {
	da = 0;
	do {
	    da += *sp++;			// transparent
	    da += *sp;			// opaque
	    sp += *sp + 1;
	} while (da < sw);
    }

    da = VideoWidth + sw;
    dp = VideoMemory16 + x + y * VideoWidth + w - 1;
    ep = dp + VideoWidth * h;

    if (w == sw) {			// Unclipped horizontal

	while (dp < ep) {		// all lines
	    lp = dp - w;
	    do {			// 1 line
		dp -= *sp++;		// transparent
		pp = dp + 1 - *sp++;		// opaque
		while (dp > pp) {
		    *dp-- = pixels[*sp++];
		    *dp-- = pixels[*sp++];
		}
		if (dp >= pp) {
		    *dp-- = pixels[*sp++];
		}
	    } while (dp > lp);
	    dp += da;
	}

    } else {				// Clip horizontal

	da -= sw - w - ox;
	while (dp < ep) {		// all lines
	    lp = dp - w;
	    //
	    //	Clip right side
	    //
	    pp = dp + sw - w - ox;
	    for (;;) {
		pp -= *sp++;		// transparent
		if (pp <= dp) {
		    dp = pp;
		    goto middle_trans;
		}
		pp -= *sp;		// opaque
		if (pp <= dp) {
		    sp += *sp - (dp - pp) + 1;
		    goto middle_pixel;
		}
		sp += *sp + 1;
	    }

	    //
	    //	Draw middle
	    //
	    for (;;) {
		dp -= *sp++;		// transparent
middle_trans:
		if (dp <= lp) {
		    lp -= ox;
		    goto right_trans;
		}
		pp = dp - *sp++;		// opaque
middle_pixel:
		if (pp > lp) {
		    while (dp > pp) {
			*dp-- = pixels[*sp++];
		    }
		    continue;
		}
		while (dp > lp) {
		    *dp-- = pixels[*sp++];
		}
		sp += dp - pp;
		dp = pp;
		break;
	    }

	    //
	    //	Clip left side
	    //
	    lp -= ox;
	    while (dp > lp) {
		dp -= *sp++;		// transparent
right_trans:
		dp -= *sp;		// opaque
		sp += *sp + 1;
	    }
	    dp += da;

	}
    }
}

/**
**	Draw 8bit graphic object clipped and flipped in X direction
**	into 24bit framebuffer.
**
**	@param sprite	pointer to object
**	@param frame	number of frame (object index)
**	@param x	x coordinate on the screen
**	@param y	y coordinate on the screen
*/
local void VideoDraw8to24ClipX(const Graphic* sprite, unsigned frame,
    int x, int y)
{
    int ox;
    int ex;
    int oy;
    int w;
    int h;
    const unsigned char* sp;
    int sw;
    VMemType24* dp;
    const VMemType24* lp;
    const VMemType24* ep;
    VMemType24* pp;
    const VMemType24* pixels;
    int da;

    //
    // reduce to visible range
    //
    sw = w = sprite->Width;
    h = sprite->Height;
    CLIP_RECTANGLE_OFS(x, y, w, h, ox, oy, ex);

    //
    //	Draw the clipped sprite
    //
    pixels = (VMemType24*)sprite->Pixels;
    sp = ((unsigned char**)sprite->Frames)[frame];

    //
    // Skip top lines
    //
    while (oy--) {
	da = 0;
	do {
	    da += *sp++;			// transparent
	    da += *sp;			// opaque
	    sp += *sp + 1;
	} while (da < sw);
    }

    da = VideoWidth + sw;
    dp = VideoMemory24 + x + y * VideoWidth + w - 1;
    ep = dp + VideoWidth * h;

    if (w == sw) {			// Unclipped horizontal

	while (dp < ep) {		// all lines
	    lp = dp - w;
	    do {			// 1 line
		dp -= *sp++;		// transparent
		pp = dp + 1 - *sp++;		// opaque
		while (dp > pp) {
		    *dp-- = pixels[*sp++];
		    *dp-- = pixels[*sp++];
		}
		if (dp >= pp) {
		    *dp-- = pixels[*sp++];
		}
	    } while (dp > lp);
	    dp += da;
	}

    } else {				// Clip horizontal

	da -= sw - w - ox;
	while (dp < ep) {		// all lines
	    lp = dp - w;
	    //
	    //	Clip right side
	    //
	    pp = dp + sw - w - ox;
	    for (;;) {
		pp -= *sp++;		// transparent
		if (pp <= dp) {
		    dp = pp;
		    goto middle_trans;
		}
		pp -= *sp;		// opaque
		if (pp <= dp) {
		    sp += *sp - (dp - pp) + 1;
		    goto middle_pixel;
		}
		sp += *sp + 1;
	    }

	    //
	    //	Draw middle
	    //
	    for (;;) {
		dp -= *sp++;		// transparent
middle_trans:
		if (dp <= lp) {
		    lp -= ox;
		    goto right_trans;
		}
		pp = dp - *sp++;		// opaque
middle_pixel:
		if (pp > lp) {
		    while (dp > pp) {
			*dp-- = pixels[*sp++];
		    }
		    continue;
		}
		while (dp > lp) {
		    *dp-- = pixels[*sp++];
		}
		sp += dp - pp;
		dp = pp;
		break;
	    }

	    //
	    //	Clip left side
	    //
	    lp -= ox;
	    while (dp > lp) {
		dp -= *sp++;		// transparent
right_trans:
		dp -= *sp;		// opaque
		sp += *sp + 1;
	    }
	    dp += da;

	}
    }
}

/**
**	Draw 8bit graphic object clipped and flipped in X direction
**	into 32bit framebuffer.
**
**	@param sprite	pointer to object
**	@param frame	number of frame (object index)
**	@param x	x coordinate on the screen
**	@param y	y coordinate on the screen
*/
local void VideoDraw8to32ClipX(const Graphic* sprite, unsigned frame
	, int x, int y)
{
    int ex;
    int ox;
    int oy;
    int w;
    int h;
    const unsigned char* sp;
    int sw;
    VMemType32* dp;
    const VMemType32* lp;
    const VMemType32* ep;
    VMemType32* pp;
    const VMemType32* pixels;
    int da;

    //
    // reduce to visible range
    //
    sw = w = sprite->Width;
    h = sprite->Height;
    CLIP_RECTANGLE_OFS(x, y, w, h, ox, oy, ex);

    //
    //	Draw the clipped sprite
    //
    pixels = (VMemType32*)sprite->Pixels;
    sp = ((unsigned char**)sprite->Frames)[frame];

    //
    // Skip top lines
    //
    while (oy--) {
	da = 0;
	do {
	    da += *sp++;			// transparent
	    da += *sp;			// opaque
	    sp += *sp + 1;
	} while (da < sw);
    }

    da = VideoWidth + sw;
    dp = VideoMemory32 + x + y * VideoWidth + w - 1;
    ep = dp + VideoWidth * h;

    if (w == sw) {			// Unclipped horizontal

	while (dp < ep) {		// all lines
	    lp = dp - w;
	    do {			// 1 line
		dp -= *sp++;		// transparent
		pp = dp + 1 - *sp++;		// opaque
		while (dp > pp) {
		    *dp-- = pixels[*sp++];
		    *dp-- = pixels[*sp++];
		}
		if (dp >= pp) {
		    *dp-- = pixels[*sp++];
		}
	    } while (dp > lp);
	    dp += da;
	}

    } else {				// Clip horizontal

	da -= ex;
	while (dp < ep) {		// all lines
	    lp = dp - w;
	    //
	    //	Clip right side
	    //
	    pp = dp + ex;
	    for (;;) {
		pp -= *sp++;		// transparent
		if (pp <= dp) {
		    dp = pp;
		    goto middle_trans;
		}
		pp -= *sp;		// opaque
		if (pp <= dp) {
		    sp += *sp - (dp - pp) + 1;
		    goto middle_pixel;
		}
		sp += *sp + 1;
	    }

	    //
	    //	Draw middle
	    //
	    for (;;) {
		dp -= *sp++;		// transparent
middle_trans:
		if (dp <= lp) {
		    lp -= ox;
		    goto right_trans;
		}
		pp = dp - *sp++;		// opaque
middle_pixel:
		if (pp > lp) {
		    while (dp > pp) {
			*dp-- = pixels[*sp++];
		    }
		    continue;
		}
		while (dp > lp) {
		    *dp-- = pixels[*sp++];
		}
		sp += dp - pp;
		dp = pp;
		break;
	    }

	    //
	    //	Clip left side
	    //
	    lp -= ox;
	    while (dp > lp) {
		dp -= *sp++;		// transparent
right_trans:
		dp -= *sp;		// opaque
		sp += *sp + 1;
	    }
	    dp += da;

	}
    }
}
#endif

/**
**	Draw graphic object clipped and flipped in X direction.
**
**	@param sprite	pointer to object
**	@param frame	number of frame (object index)
**	@param x	x coordinate on the screen
**	@param y	y coordinate on the screen
*/
#ifdef USE_OPENGL
local void VideoDrawClipXOpenGL(const Graphic* sprite, unsigned frame,
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
    evy = VideoHeight - y;
    svy = evy - h;

    if (w < sprite->Width) {
	if (ox == 0) {
	    ox += sprite->Width - w;
	} else {
	    ox = 0;
	}
    }
    stx = (GLfloat)ox / sprite->Width * sprite->TextureWidth;
    etx = (GLfloat)(ox + w) / sprite->Width * sprite->TextureWidth;
    ety = 1.0f - (GLfloat)oy / sprite->Height * sprite->TextureHeight;
    sty = 1.0f - (GLfloat)(oy + h) / sprite->Height * sprite->TextureHeight;

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

#ifdef USE_SDL_SURFACE

#else
/**
**	Draw 8bit shadow graphic object clipped into 8 bit framebuffer.
**
**	@param sprite	pointer to object
**	@param frame	number of frame (object index)
**	@param x	x coordinate on the screen
**	@param y	y coordinate on the screen
*/
local void VideoDraw8to8ShadowClip(const Graphic* sprite, unsigned frame, int x, int y)
{
    int ox;
    int ex;
    int oy;
    int w;
    int h;
    const unsigned char* sp;
    int sw;
    VMemType8* dp;
    const VMemType8* lp;
    const VMemType8* ep;
    VMemType8* pp;
    const VMemType8* pixels;
    int da;

    //
    // reduce to visible range
    //
    sw = w = sprite->Width;
    h = sprite->Height;
    CLIP_RECTANGLE_OFS(x, y, w, h, ox, oy, ex);

    //
    //	Draw the clipped sprite
    //
    pixels = (VMemType8*)sprite->Pixels;
    sp = ((unsigned char**)sprite->Frames)[frame];

    //
    //	Skip top lines, if needed.
    //
    while (oy--) {
	da = 0;
	do {
	    da += *sp++;			// transparent
	    da += *sp;			// opaque
	    sp += *sp + 1;
	} while (da < sw);
    }

    da = VideoWidth - sw;
    dp = VideoMemory8 + x + y * VideoWidth;
    ep = dp + VideoWidth * h;

    if (w == sw) {			// Unclipped horizontal

	do {
	    lp = dp + sw;
	    do {			// 1 line
		dp += *sp++;		// transparent
		pp = dp - 1 + *sp++;		// opaque
		while (dp < pp) {	// unroll
		    *dp = lookup50trans8[(ColorBlack.D8 >> 8) | *dp];
		    ++dp;
		    ++sp;
		    *dp = lookup50trans8[(ColorBlack.D8 >> 8) | *dp];
		    ++dp;
		    ++sp;
		}
		if (dp <= pp) {
		    *dp = lookup50trans8[(ColorBlack.D8 >> 8) | *dp];
		    ++dp;
		    ++sp;
		}
	    } while (dp < lp);
	    dp += da;
	} while (dp < ep);		// all lines

    } else {				// Clip horizontal

	da += ox;
	do {
	    lp = dp + w;
	    //
	    //	Clip left
	    //
	    pp = dp - ox;
	    for (;;) {
		pp += *sp++;		// transparent
		if (pp >= dp) {
		    dp = pp;
		    goto middle_trans;
		}
		pp += *sp;		// opaque
		if (pp >= dp) {
		    sp += *sp - (pp - dp) + 1;
		    goto middle_pixel;
		}
		sp += *sp + 1;
	    }

	    //
	    //	Draw middle
	    //
	    for (;;) {
		dp += *sp++;		// transparent
middle_trans:
		if (dp >= lp) {
		    lp += sw - w - ox;
		    goto right_trans;
		}
		pp = dp + *sp++;		// opaque
middle_pixel:
		if (pp < lp) {
		    while (dp < pp) {
			*dp = lookup50trans8[(ColorBlack.D8 >> 8) | *dp];
			++dp;
			++sp;
		    }
		    continue;
		}
		while (dp < lp) {
		    *dp = lookup50trans8[(ColorBlack.D8 >> 8) | *dp];
		    ++dp;
		    ++sp;
		}
		sp += pp - dp;
		dp = pp;
		break;
	    }

	    //
	    //	Clip right
	    //
	    lp += sw - w - ox;
	    while (dp < lp) {
		dp += *sp++;		// transparent
right_trans:
		dp += *sp;		// opaque
		sp += *sp + 1;
	    }
	    dp += da;
	} while (dp < ep);		// all lines

    }
}

/**
**	Draw 8bit shadow graphic object clipped into 16 bit framebuffer.
**
**	@param sprite	pointer to object
**	@param frame	number of frame (object index)
**	@param x	x coordinate on the screen
**	@param y	y coordinate on the screen
*/
local void VideoDraw8to16ShadowClip(const Graphic* sprite, unsigned frame, int x, int y)
{
    int ox;
    int ex;
    int oy;
    int w;
    int h;
    const unsigned char* sp;
    int sw;
    VMemType16* dp;
    const VMemType16* lp;
    const VMemType16* ep;
    VMemType16* pp;
    const VMemType16* pixels;
    int da;
    unsigned long dp1;
    unsigned long mask;

    //
    // reduce to visible range
    //
    sw = w = sprite->Width;
    h = sprite->Height;
    CLIP_RECTANGLE_OFS(x, y, w, h, ox, oy, ex);

    //
    //	Draw the clipped sprite
    //
    pixels = (VMemType16*)sprite->Pixels;
    sp = ((unsigned char**)sprite->Frames)[frame];

    //
    //	Skip top lines, if needed.
    //
    while (oy--) {
	da = 0;
	do {
	    da += *sp++;			// transparent
	    da += *sp;			// opaque
	    sp += *sp + 1;
	} while (da < sw);
    }

    da = VideoWidth - sw;
    dp = VideoMemory16 + x + y * VideoWidth;
    ep = dp + VideoWidth * h;

    if (VideoBpp == 15) {
	mask = 0x03E07C1F;
    } else {
	mask = 0x07E0F81F;
    }

    if (w == sw) {			// Unclipped horizontal

	do {
	    lp = dp + sw;
	    do {			// 1 line
		dp += *sp++;		// transparent
		pp = dp - 1 + *sp++;		// opaque
		while (dp < pp) {	// unroll
		    dp1 = *dp;
		    dp1 = ((((dp1 << 16) | dp1) & mask) >> 1) & mask;
		    *dp++ = (dp1 >> 16) | dp1;
		    ++sp;
		    dp1 = *dp;
		    dp1 = ((((dp1 << 16) | dp1) & mask) >> 1) & mask;
		    *dp++ = (dp1 >> 16) | dp1;
		    ++sp;
		}
		if (dp <= pp) {
		    dp1 = *dp;
		    dp1 = ((((dp1 << 16) | dp1) & mask) >> 1) & mask;
		    *dp++ = (dp1 >> 16) | dp1;
		    ++sp;
		}
	    } while (dp < lp);
	    dp += da;
	} while (dp < ep);		// all lines

    } else {				// Clip horizontal

	da += ox;
	do {
	    lp = dp + w;
	    //
	    //	Clip left
	    //
	    pp = dp - ox;
	    for (;;) {
		pp += *sp++;		// transparent
		if (pp >= dp) {
		    dp = pp;
		    goto middle_trans;
		}
		pp += *sp;		// opaque
		if (pp >= dp) {
		    sp += *sp - (pp - dp) + 1;
		    goto middle_pixel;
		}
		sp += *sp + 1;
	    }

	    //
	    //	Draw middle
	    //
	    for (;;) {
		dp += *sp++;		// transparent
middle_trans:
		if (dp >= lp) {
		    lp += sw - w - ox;
		    goto right_trans;
		}
		pp = dp + *sp++;		// opaque
middle_pixel:
		if (pp < lp) {
		    while (dp < pp) {
			dp1 = *dp;
			dp1 = ((((dp1 << 16) | dp1) & mask) >> 1) & mask;
			*dp++ = (dp1 >> 16) | dp1;
			++sp;
		    }
		    continue;
		}
		while (dp < lp) {
		    dp1 = *dp;
		    dp1 = ((((dp1 << 16) | dp1) & mask) >> 1) & mask;
		    *dp++ = (dp1 >> 16) | dp1;
		    ++sp;
		}
		sp += pp - dp;
		dp = pp;
		break;
	    }

	    //
	    //	Clip right
	    //
	    lp += sw - w - ox;
	    while (dp < lp) {
		dp += *sp++;		// transparent
right_trans:
		dp += *sp;		// opaque
		sp += *sp + 1;
	    }
	    dp += da;
	} while (dp < ep);		// all lines

    }
}

/**
**	Draw 8bit shadow graphic object clipped into 24 bit framebuffer.
**
**	@param sprite	pointer to object
**	@param frame	number of frame (object index)
**	@param x	x coordinate on the screen
**	@param y	y coordinate on the screen
*/
local void VideoDraw8to24ShadowClip(const Graphic* sprite, unsigned frame, int x, int y)
{
    int ox;
    int ex;
    int oy;
    int w;
    int h;
    const unsigned char* sp;
    int sw;
    VMemType24* dp;
    const VMemType24* lp;
    const VMemType24* ep;
    VMemType24* pp;
    const VMemType24* pixels;
    int da;

    //
    // reduce to visible range
    //
    sw = w = sprite->Width;
    h = sprite->Height;
    CLIP_RECTANGLE_OFS(x, y, w, h, ox, oy, ex);

    //
    //	Draw the clipped sprite
    //
    pixels = (VMemType24*)sprite->Pixels;
    sp = ((unsigned char**)sprite->Frames)[frame];

    //
    //	Skip top lines, if needed.
    //
    while (oy--) {
	da = 0;
	do {
	    da += *sp++;			// transparent
	    da += *sp;			// opaque
	    sp += *sp + 1;
	} while (da < sw);
    }

    da = VideoWidth - sw;
    dp = VideoMemory24 + x + y * VideoWidth;
    ep = dp + VideoWidth * h;

    if (w == sw) {			// Unclipped horizontal

	do {
	    lp = dp + sw;
	    do {			// 1 line
		dp += *sp++;		// transparent
		pp = dp - 1 + *sp++;		// opaque
		while (dp < pp) {	// unroll
		    *dp++ = pixels[*sp++];
		    *dp++ = pixels[*sp++];
		}
		if (dp <= pp) {
		    *dp++ = pixels[*sp++];
		}
	    } while (dp < lp);
	    dp += da;
	} while (dp < ep);		// all lines

    } else {				// Clip horizontal

	da += ox;
	do {
	    lp = dp + w;
	    //
	    //	Clip left
	    //
	    pp = dp - ox;
	    for (;;) {
		pp += *sp++;		// transparent
		if (pp >= dp) {
		    dp = pp;
		    goto middle_trans;
		}
		pp += *sp;		// opaque
		if (pp >= dp) {
		    sp += *sp - (pp - dp) + 1;
		    goto middle_pixel;
		}
		sp += *sp + 1;
	    }

	    //
	    //	Draw middle
	    //
	    for (;;) {
		dp += *sp++;		// transparent
middle_trans:
		if (dp >= lp) {
		    lp += sw - w - ox;
		    goto right_trans;
		}
		pp = dp + *sp++;		// opaque
middle_pixel:
		if (pp < lp) {
		    while (dp < pp) {
			*dp++ = pixels[*sp++];
		    }
		    continue;
		}
		while (dp < lp) {
		    *dp++ = pixels[*sp++];
		}
		sp += pp - dp;
		dp = pp;
		break;
	    }

	    //
	    //	Clip right
	    //
	    lp += sw - w - ox;
	    while (dp < lp) {
		dp += *sp++;		// transparent
right_trans:
		dp += *sp;		// opaque
		sp += *sp + 1;
	    }
	    dp += da;
	} while (dp < ep);		// all lines

    }
}

/**
**	Draw 8bit shadow graphic object clipped into 32 bit framebuffer.
**
**	@param sprite	pointer to object
**	@param frame	number of frame (object index)
**	@param x	x coordinate on the screen
**	@param y	y coordinate on the screen
*/
local void VideoDraw8to32ShadowClip(const Graphic* sprite, unsigned frame, int x, int y)
{
    int ox;
    int ex;
    int oy;
    int w;
    int h;
    const unsigned char* sp;
    int sw;
    VMemType32* dp;
    const VMemType32* lp;
    const VMemType32* ep;
    VMemType32* pp;
    const VMemType32* pixels;
    int da;
    unsigned long dp1;
    unsigned long dp2;

    //
    // reduce to visible range
    //
    sw = w = sprite->Width;
    h = sprite->Height;
    CLIP_RECTANGLE_OFS(x, y, w, h, ox, oy, ex);

    //
    //	Draw the clipped sprite
    //
    pixels = (VMemType32*)sprite->Pixels;
    sp = ((unsigned char**)sprite->Frames)[frame];

    //
    //	Skip top lines, if needed.
    //
    while (oy--) {
	da = 0;
	do {
	    da += *sp++;			// transparent
	    da += *sp;			// opaque
	    sp += *sp + 1;
	} while (da < sw);
    }

    da = VideoWidth - sw;
    dp = VideoMemory32 + x + y * VideoWidth;
    ep = dp + VideoWidth * h;

    if (w == sw) {			// Unclipped horizontal

	do {
	    lp = dp + sw;
	    do {			// 1 line
		dp += *sp++;		// transparent
		pp = dp - 1 + *sp++;		// opaque
		while (dp < pp) {	// unroll
		    dp1 = *dp;
		    dp2 = (dp1 & 0xFF00FF00) >> 8;
		    dp1 &= 0x00FF00FF;
		    dp1 = (dp1 >> 1) & 0x00FF00FF;
		    dp2 = (dp2 >> 1) & 0x00FF00FF;
		    *dp++ = (dp1 | (dp2 << 8));
		    ++sp;
		    dp1 = *dp;
		    dp2 = (dp1 & 0xFF00FF00) >> 8;
		    dp1 &= 0x00FF00FF;
		    dp1 = (dp1 >> 1) & 0x00FF00FF;
		    dp2 = (dp2 >> 1) & 0x00FF00FF;
		    *dp++ = (dp1 | (dp2 << 8));
		    ++sp;
		}
		if (dp <= pp) {
		    dp1 = *dp;
		    dp2 = (dp1 & 0xFF00FF00) >> 8;
		    dp1 &= 0x00FF00FF;
		    dp1 = (dp1 >> 1) & 0x00FF00FF;
		    dp2 = (dp2 >> 1) & 0x00FF00FF;
		    *dp++ = (dp1 | (dp2 << 8));
		    ++sp;
		}
	    } while (dp < lp);
	    dp += da;
	} while (dp < ep);		// all lines

    } else {				// Clip horizontal

	da += ox;
	do {
	    lp = dp + w;
	    //
	    //	Clip left
	    //
	    pp = dp - ox;
	    for (;;) {
		pp += *sp++;		// transparent
		if (pp >= dp) {
		    dp = pp;
		    goto middle_trans;
		}
		pp += *sp;		// opaque
		if (pp >= dp) {
		    sp += *sp - (pp - dp) + 1;
		    goto middle_pixel;
		}
		sp += *sp + 1;
	    }

	    //
	    //	Draw middle
	    //
	    for (;;) {
		dp += *sp++;		// transparent
middle_trans:
		if (dp >= lp) {
		    lp += sw - w - ox;
		    goto right_trans;
		}
		pp = dp + *sp++;		// opaque
middle_pixel:
		if (pp < lp) {
		    while (dp < pp) {
			dp1 = *dp;
			dp2 = (dp1 & 0xFF00FF00) >> 8;
			dp1 &= 0x00FF00FF;
			dp1 = (dp1 >> 1) & 0x00FF00FF;
			dp2 = (dp2 >> 1) & 0x00FF00FF;
			*dp++ = (dp1 | (dp2 << 8));
			++sp;
		    }
		    continue;
		}
		while (dp < lp) {
		    dp1 = *dp;
		    dp2 = (dp1 & 0xFF00FF00) >> 8;
		    dp1 &= 0x00FF00FF;
		    dp1 = (dp1 >> 1) & 0x00FF00FF;
		    dp2 = (dp2 >> 1) & 0x00FF00FF;
		    *dp++ = (dp1 | (dp2 << 8));
		    ++sp;
		}
		sp += pp - dp;
		dp = pp;
		break;
	    }

	    //
	    //	Clip right
	    //
	    lp += sw - w - ox;
	    while (dp < lp) {
		dp += *sp++;		// transparent
right_trans:
		dp += *sp;		// opaque
		sp += *sp + 1;
	    }
	    dp += da;
	} while (dp < ep);		// all lines

    }
}
#endif

#ifdef USE_SDL_SURFACE
    // FIXME: todo
#else
/**
**	Draw 8bit shadow graphic object clipped and flipped in X direction
**	into 8bit framebuffer.
**
**	@param sprite	pointer to object
**	@param frame	number of frame (object index)
**	@param x	x coordinate on the screen
**	@param y	y coordinate on the screen
*/
local void VideoDraw8to8ShadowClipX(const Graphic* sprite, unsigned frame,
    int x, int y)
{
    int ex;
    int ox;
    int oy;
    int w;
    int h;
    const unsigned char* sp;
    int sw;
    VMemType8* dp;
    const VMemType8* lp;
    const VMemType8* ep;
    VMemType8* pp;
    const VMemType8* pixels;
    int da;

    //
    // reduce to visible range
    //
    sw = w = sprite->Width;
    h = sprite->Height;
    CLIP_RECTANGLE_OFS(x, y, w, h, ox, oy, ex);

    //
    //	Draw the clipped sprite
    //
    pixels = (VMemType8*)sprite->Pixels;
    sp = ((unsigned char**)sprite->Frames)[frame];

    //
    // Skip top lines
    //
    while (oy--) {
	da = 0;
	do {
	    da += *sp++;			// transparent
	    da += *sp;			// opaque
	    sp += *sp + 1;
	} while (da < sw);
    }

    da = VideoWidth + sw;
    dp = VideoMemory8 + x + y * VideoWidth + w - 1;
    ep = dp + VideoWidth * h;

    if (w == sw) {			// Unclipped horizontal
	while (dp < ep) {		// all lines
	    lp = dp - w;
	    do {			// 1 line
		dp -= *sp++;		// transparent
		pp = dp + 1 - *sp++;		// opaque
		while (dp > pp) {
		    *dp = lookup50trans8[(ColorBlack.D8 << 8) | *dp];
		    --dp;
		    ++sp;
		    *dp = lookup50trans8[(ColorBlack.D8 << 8) | *dp];
		    --dp;
		    ++sp;
		}
		if (dp >= pp) {
		    *dp = lookup50trans8[(ColorBlack.D8 << 8) | *dp];
		    --dp;
		    ++sp;
		}
	    } while (dp > lp);
	    dp += da;
	}

    } else {				// Clip horizontal

	da -= sw - w - ox;
	while (dp < ep) {		// all lines
	    lp = dp - w;
	    //
	    //	Clip right side
	    //
	    pp = dp + sw - w - ox;
	    for (;;) {
		pp -= *sp++;		// transparent
		if (pp <= dp) {
		    dp = pp;
		    goto middle_trans;
		}
		pp -= *sp;		// opaque
		if (pp <= dp) {
		    sp += *sp - (dp - pp) + 1;
		    goto middle_pixel;
		}
		sp += *sp + 1;
	    }

	    //
	    //	Draw middle
	    //
	    for (;;) {
		dp -= *sp++;		// transparent
middle_trans:
		if (dp <= lp) {
		    lp -= ox;
		    goto right_trans;
		}
		pp = dp - *sp++;		// opaque
middle_pixel:
		if (pp > lp) {
		    while (dp > pp) {
			*dp = lookup50trans8[(ColorBlack.D8 << 8) | *dp];
			--dp;
			++sp;
		    }
		    continue;
		}
		while (dp > lp) {
		    *dp = lookup50trans8[(ColorBlack.D8 << 8) | *dp];
		    --dp;
		    ++sp;
		}
		sp += dp - pp;
		dp = pp;
		break;
	    }

	    //
	    //	Clip left side
	    //
	    lp -= ox;
	    while (dp > lp) {
		dp -= *sp++;		// transparent
right_trans:
		dp -= *sp;		// opaque
		sp += *sp + 1;
	    }
	    dp += da;

	}
    }
}

/**
**	Draw 8bit shadow graphic object clipped and flipped in X direction
**	into 16bit framebuffer.
**
**	@param sprite	pointer to object
**	@param frame	number of frame (object index)
**	@param x	x coordinate on the screen
**	@param y	y coordinate on the screen
*/
local void VideoDraw8to16ShadowClipX(const Graphic* sprite, unsigned frame,
    int x, int y)
{
    int ex;
    int ox;
    int oy;
    int w;
    int h;
    const unsigned char* sp;
    int sw;
    VMemType16* dp;
    const VMemType16* lp;
    const VMemType16* ep;
    VMemType16* pp;
    const VMemType16* pixels;
    int da;
    unsigned long dp1;
    unsigned long mask;

    //
    // reduce to visible range
    //
    sw = w = sprite->Width;
    h = sprite->Height;
    CLIP_RECTANGLE_OFS(x, y, w, h, ox, oy, ex);

    //
    //	Draw the clipped sprite
    //
    pixels = (VMemType16*)sprite->Pixels;
    sp = ((unsigned char**)sprite->Frames)[frame];

    //
    // Skip top lines
    //
    while (oy--) {
	da = 0;
	do {
	    da += *sp++;			// transparent
	    da += *sp;			// opaque
	    sp += *sp + 1;
	} while (da < sw);
    }

    da = VideoWidth + sw;
    dp = VideoMemory16 + x + y * VideoWidth + w - 1;
    ep = dp + VideoWidth * h;

    if (VideoBpp == 15) {
	mask = 0x03E07C1F;
    } else {
	mask = 0x07E0F81F;
    }

    if (w == sw) {			// Unclipped horizontal

	while (dp < ep) {		// all lines
	    lp = dp - w;
	    do {			// 1 line
		dp -= *sp++;		// transparent
		pp = dp + 1 - *sp++;		// opaque
		while (dp > pp) {
		    dp1 = *dp;
		    dp1 = ((((dp1 << 16) | dp1) & mask) >> 1) & mask;
		    *dp-- = (dp1 >> 16) | dp1;
		    ++sp;
		    dp1 = *dp;
		    dp1 = ((((dp1 << 16) | dp1) & mask) >> 1) & mask;
		    *dp-- = (dp1 >> 16) | dp1;
		    ++sp;
		}
		if (dp >= pp) {
		    dp1 = *dp;
		    dp1 = ((((dp1 << 16) | dp1) & mask) >> 1) & mask;
		    *dp-- = (dp1 >> 16) | dp1;
		    ++sp;
		}
	    } while (dp > lp);
	    dp += da;
	}

    } else {				// Clip horizontal

	da -= sw - w - ox;
	while (dp < ep) {		// all lines
	    lp = dp - w;
	    //
	    //	Clip right side
	    //
	    pp = dp + sw - w - ox;
	    for (;;) {
		pp -= *sp++;		// transparent
		if (pp <= dp) {
		    dp = pp;
		    goto middle_trans;
		}
		pp -= *sp;		// opaque
		if (pp <= dp) {
		    sp += *sp - (dp - pp) + 1;
		    goto middle_pixel;
		}
		sp += *sp + 1;
	    }

	    //
	    //	Draw middle
	    //
	    for (;;) {
		dp -= *sp++;		// transparent
middle_trans:
		if (dp <= lp) {
		    lp -= ox;
		    goto right_trans;
		}
		pp = dp - *sp++;		// opaque
middle_pixel:
		if (pp > lp) {
		    while (dp > pp) {
			dp1 = *dp;
			dp1 = ((((dp1 << 16) | dp1) & mask) >> 1) & mask;
			*dp-- = (dp1 >> 16) | dp1;
			++sp;
		    }
		    continue;
		}
		while (dp > lp) {
		    dp1 = *dp;
		    dp1 = ((((dp1 << 16) | dp1) & mask) >> 1) & mask;
		    *dp-- = (dp1 >> 16) | dp1;
		    ++sp;
		}
		sp += dp - pp;
		dp = pp;
		break;
	    }

	    //
	    //	Clip left side
	    //
	    lp -= ox;
	    while (dp > lp) {
		dp -= *sp++;		// transparent
right_trans:
		dp -= *sp;		// opaque
		sp += *sp + 1;
	    }
	    dp += da;

	}
    }
}

/**
**	Draw 8bit shadow graphic object clipped and flipped in X direction
**	into 24bit framebuffer.
**
**	@param sprite	pointer to object
**	@param frame	number of frame (object index)
**	@param x	x coordinate on the screen
**	@param y	y coordinate on the screen
*/
local void VideoDraw8to24ShadowClipX(const Graphic* sprite, unsigned frame,
    int x, int y)
{
    int ex;
    int ox;
    int oy;
    int w;
    int h;
    const unsigned char* sp;
    int sw;
    VMemType24* dp;
    const VMemType24* lp;
    const VMemType24* ep;
    VMemType24* pp;
    const VMemType24* pixels;
    int da;

    //
    // reduce to visible range
    //
    sw = w = sprite->Width;
    h = sprite->Height;
    CLIP_RECTANGLE_OFS(x, y, w, h, ox, oy, ex);

    //
    //	Draw the clipped sprite
    //
    pixels = (VMemType24*)sprite->Pixels;
    sp = ((unsigned char**)sprite->Frames)[frame];

    //
    // Skip top lines
    //
    while (oy--) {
	da = 0;
	do {
	    da += *sp++;			// transparent
	    da += *sp;			// opaque
	    sp += *sp + 1;
	} while (da < sw);
    }

    da = VideoWidth + sw;
    dp = VideoMemory24 + x + y * VideoWidth + w - 1;
    ep = dp + VideoWidth * h;

    if (w == sw) {			// Unclipped horizontal

	while (dp < ep) {		// all lines
	    lp = dp - w;
	    do {			// 1 line
		dp -= *sp++;		// transparent
		pp = dp + 1 - *sp++;		// opaque
		while (dp > pp) {
		    *dp-- = pixels[*sp++];
		    *dp-- = pixels[*sp++];
		}
		if (dp >= pp) {
		    *dp-- = pixels[*sp++];
		}
	    } while (dp > lp);
	    dp += da;
	}

    } else {				// Clip horizontal

	da -= sw - w - ox;
	while (dp < ep) {		// all lines
	    lp = dp - w;
	    //
	    //	Clip right side
	    //
	    pp = dp + sw - w - ox;
	    for (;;) {
		pp -= *sp++;		// transparent
		if (pp <= dp) {
		    dp = pp;
		    goto middle_trans;
		}
		pp -= *sp;		// opaque
		if (pp <= dp) {
		    sp += *sp - (dp - pp) + 1;
		    goto middle_pixel;
		}
		sp += *sp + 1;
	    }

	    //
	    //	Draw middle
	    //
	    for (;;) {
		dp -= *sp++;		// transparent
middle_trans:
		if (dp <= lp) {
		    lp -= ox;
		    goto right_trans;
		}
		pp = dp - *sp++;		// opaque
middle_pixel:
		if (pp > lp) {
		    while (dp > pp) {
			*dp-- = pixels[*sp++];
		    }
		    continue;
		}
		while (dp > lp) {
		    *dp-- = pixels[*sp++];
		}
		sp += dp - pp;
		dp = pp;
		break;
	    }

	    //
	    //	Clip left side
	    //
	    lp -= ox;
	    while (dp > lp) {
		dp -= *sp++;		// transparent
right_trans:
		dp -= *sp;		// opaque
		sp += *sp + 1;
	    }
	    dp += da;

	}
    }
}

/**
**	Draw 8bit shadow graphic object clipped and flipped in X direction
**	into 32bit framebuffer.
**
**	@param sprite	pointer to object
**	@param frame	number of frame (object index)
**	@param x	x coordinate on the screen
**	@param y	y coordinate on the screen
*/
local void VideoDraw8to32ShadowClipX(const Graphic* sprite, unsigned frame,
    int x, int y)
{
    int ex;
    int ox;
    int oy;
    int w;
    int h;
    const unsigned char* sp;
    int sw;
    VMemType32* dp;
    const VMemType32* lp;
    const VMemType32* ep;
    VMemType32* pp;
    const VMemType32* pixels;
    int da;
    unsigned long dp1;
    unsigned long dp2;

    //
    // reduce to visible range
    //
    sw = w = sprite->Width;
    h = sprite->Height;
    CLIP_RECTANGLE_OFS(x, y, w, h, ox, oy, ex);

    //
    //	Draw the clipped sprite
    //
    pixels = (VMemType32*)sprite->Pixels;
    sp = ((unsigned char**)sprite->Frames)[frame];

    //
    // Skip top lines
    //
    while (oy--) {
	da = 0;
	do {
	    da += *sp++;			// transparent
	    da += *sp;			// opaque
	    sp += *sp + 1;
	} while (da < sw);
    }

    da = VideoWidth + sw;
    dp = VideoMemory32 + x + y * VideoWidth + w - 1;
    ep = dp + VideoWidth * h;

    if (w == sw) {			// Unclipped horizontal

	while (dp < ep) {		// all lines
	    lp = dp - w;
	    do {			// 1 line
		dp -= *sp++;		// transparent
		pp = dp + 1 - *sp++;		// opaque
		while (dp > pp) {
		    dp1 = *dp;
		    dp2 = (dp1 & 0xFF00FF00) >> 8;
		    dp1 &= 0x00FF00FF;
		    dp1 = (dp1 >> 1) & 0x00FF00FF;
		    dp2 = (dp2 >> 1) & 0x00FF00FF;
		    *dp-- = (dp1 | (dp2 << 8));
		    ++sp;
		    dp1 = *dp;
		    dp2 = (dp1 & 0xFF00FF00) >> 8;
		    dp1 &= 0x00FF00FF;
		    dp1 = (dp1 >> 1) & 0x00FF00FF;
		    dp2 = (dp2 >> 1) & 0x00FF00FF;
		    *dp-- = (dp1 | (dp2 << 8));
		    ++sp;
		}
		if (dp >= pp) {
		    dp1 = *dp;
		    dp2 = (dp1 & 0xFF00FF00) >> 8;
		    dp1 &= 0x00FF00FF;
		    dp1 = (dp1 >> 1) & 0x00FF00FF;
		    dp2 = (dp2 >> 1) & 0x00FF00FF;
		    *dp-- = (dp1 | (dp2 << 8));
		    ++sp;
		}
	    } while (dp > lp);
	    dp += da;
	}

    } else {				// Clip horizontal

	da -= ex;
	while (dp < ep) {		// all lines
	    lp = dp - w;
	    //
	    //	Clip right side
	    //
	    pp = dp + ex;
	    for (;;) {
		pp -= *sp++;		// transparent
		if (pp <= dp) {
		    dp = pp;
		    goto middle_trans;
		}
		pp -= *sp;		// opaque
		if (pp <= dp) {
		    sp += *sp - (dp - pp) + 1;
		    goto middle_pixel;
		}
		sp += *sp + 1;
	    }

	    //
	    //	Draw middle
	    //
	    for (;;) {
		dp -= *sp++;		// transparent
middle_trans:
		if (dp <= lp) {
		    lp -= ox;
		    goto right_trans;
		}
		pp = dp - *sp++;		// opaque
middle_pixel:
		if (pp > lp) {
		    while (dp > pp) {
			dp1 = *dp;
			dp2 = (dp1 & 0xFF00FF00) >> 8;
			dp1 &= 0x00FF00FF;
			dp1 = (dp1 >> 1) & 0x00FF00FF;
			dp2 = (dp2 >> 1) & 0x00FF00FF;
			*dp-- = (dp1 | (dp2 << 8));
			++sp;
		    }
		    continue;
		}
		while (dp > lp) {
		    dp1 = *dp;
		    dp2 = (dp1 & 0xFF00FF00) >> 8;
		    dp1 &= 0x00FF00FF;
		    dp1 = (dp1 >> 1) & 0x00FF00FF;
		    dp2 = (dp2 >> 1) & 0x00FF00FF;
		    *dp-- = (dp1 | (dp2 << 8));
		    ++sp;
		}
		sp += dp - pp;
		dp = pp;
		break;
	    }

	    //
	    //	Clip left side
	    //
	    lp -= ox;
	    while (dp > lp) {
		dp -= *sp++;		// transparent
right_trans:
		dp -= *sp;		// opaque
		sp += *sp + 1;
	    }
	    dp += da;

	}
    }
}

/**
**	Draw 8bit raw graphic data clipped, using given pixel pallette
**	of a given color-depth in bytes: 8=1, 16=2, 24=3, 32=4
**
**	@param pixels	VMemType 256 color palette to translate given data
**	@param data	raw graphic data in 8bit color indexes of above palette
**	@param x	left-top corner x coordinate in pixels on the screen
**	@param y	left-top corner y coordinate in pixels on the screen
**	@param w	width of above graphic data in pixels
**	@param h	height of above graphic data in pixels
**	@param bytes	color-depth of given palette
**
**	FIXME: make this faster..
*/
global void VideoDrawRawXXClip(char *pixels, const unsigned char *data,
    int x, int y, int w, int h, char bytes)
{
    char *dest;
    int ofsx;
    int ofsy;
    int skipx;
    int nextline;

    // Clip given rectangle area, keeping track of start- and end-offsets
    nextline = w;
    CLIP_RECTANGLE_OFS(x, y, w, h, ofsx, ofsy, skipx);
    data += (ofsy * nextline) + ofsx;
    skipx += ofsx;

    // Draw the raw data, through the given palette
    dest = (char*)VideoMemory + (y * VideoWidth + x) * bytes;
    nextline = (VideoWidth - w) * bytes;

    do {
	int w2;
	w2 = w;

	do {
	    memcpy(dest, pixels + *data++ * bytes, bytes);
	    dest += bytes;
	} while (--w2 > 0);

	data += skipx;
	dest += nextline;
    } while (--h > 0);
}
#endif

#ifdef USE_SDL_SURFACE
    // FIXME: todo
#else
/**
**	Draw 8bit raw graphic data clipped, using given pixel pallette
**	into 8bit framebuffer.
**
**	@param pixels	VMemType8 256 color palette to translate given data
**	@param data	raw graphic data in 8bit color indexes of above palette
**	@param x	left-top corner x coordinate in pixels on the screen
**	@param y	left-top corner y coordinate in pixels on the screen
**	@param w	width of above graphic data in pixels
**	@param h	height of above graphic data in pixels
*/
local void VideoDrawRaw8Clip(VMemType *pixels, const unsigned char *data,
    int x, int y, int w, int h)
{
    VideoDrawRawXXClip((char*)pixels, data, x, y, w, h, sizeof(VMemType8));
}

/**
**	Draw 8bit raw graphic data clipped, using given pixel pallette
**	into 16bit framebuffer.
**
**	@param pixels	VMemType16 256 color palette to translate given data
**	@param data	raw graphic data in 8bit color indexes of above palette
**	@param x	left-top corner x coordinate in pixels on the screen
**	@param y	left-top corner y coordinate in pixels on the screen
**	@param w	width of above graphic data in pixels
**	@param h	height of above graphic data in pixels
*/
local void VideoDrawRaw16Clip(VMemType *pixels, const unsigned char *data,
    int x, int y, int w, int h)
{
    VideoDrawRawXXClip((char*)pixels, data, x, y, w, h, sizeof(VMemType16));
}

/**
**	Draw 8bit raw graphic data clipped, using given pixel pallette
**	into 24bit framebuffer.
**
**	@param pixels	VMemType24 256 color palette to translate given data
**	@param data	raw graphic data in 8bit color indexes of above palette
**	@param x	left-top corner x coordinate in pixels on the screen
**	@param y	left-top corner y coordinate in pixels on the screen
**	@param w	width of above graphic data in pixels
**	@param h	height of above graphic data in pixels
*/
local void VideoDrawRaw24Clip(VMemType *pixels, const unsigned char *data,
    int x, int y, int w, int h)
{
    VideoDrawRawXXClip((char*)pixels, data, x, y, w, h, sizeof(VMemType24));
}

/**
**	Draw 8bit raw graphic data clipped, using given pixel pallette
**	into 32bit framebuffer.
**
**	@param pixels	VMemType32 256 color palette to translate given data
**	@param data	raw graphic data in 8bit color indexes of above palette
**	@param x	left-top corner x coordinate in pixels on the screen
**	@param y	left-top corner y coordinate in pixels on the screen
**	@param w	width of above graphic data in pixels
**	@param h	height of above graphic data in pixels
*/
local void VideoDrawRaw32Clip(VMemType *pixels, const unsigned char *data,
    int x, int y, int w, int h)
{
    VideoDrawRawXXClip((char*)pixels, data, x, y, w, h, sizeof(VMemType32));
}
#endif

#ifdef USE_SDL_SURFACE
local void FreeSprite(Graphic* graphic)
{
    int i;
#ifdef DEBUG_TODO
    AllocatedGraphicMemory -= graphic->Size;
    AllocatedGraphicMemory -= sizeof(Graphic);
#endif
    for (i = 0; i < graphic->NumFrames; ++i) {
	SDL_FreeSurface(&graphic->Surface[i]);
    }
}
#else
/**
**	Free graphic object.
*/
local void FreeSprite8(Graphic* graphic)
{
#ifdef DEBUG
    AllocatedGraphicMemory -= graphic->Size;
    AllocatedGraphicMemory -= sizeof(Graphic);
#endif

#ifdef USE_OPENGL
    if (graphic->NumTextureNames) {
	glDeleteTextures(graphic->NumTextureNames, graphic->TextureNames);
	free(graphic->TextureNames);
    }
#endif
    VideoFreeSharedPalette(graphic->Pixels);
    free(graphic->Frames);
    free(graphic);
}
#endif

// FIXME: need 16 bit palette version
// FIXME: need alpha blending version
// FIXME: need zooming version

/*----------------------------------------------------------------------------
--	Global functions
----------------------------------------------------------------------------*/

/**
**	Load sprite from file.
**
**	Compress the file as RLE (run-length-encoded) sprite.
**
**	@param	name	File name of sprite to load.
**	@param	width	Width of a single frame.
**	@param	height	Height of a single frame.
**
**	@return		A graphic object for the loaded sprite.
**
**	@see	LoadGraphic
*/
global Graphic* LoadSprite(const char* name, int width, int height)
{
#ifdef USE_SDL_SURFACE
    Graphic * g;
    int nframes;

    g = LoadGraphic(name);

    nframes = g->Width / width * g->Height / height;

    g->NumFrames = nframes;
    g->Width = width;
    g->Height = height;

    return g;
#else
    Graphic* graphic;
#ifndef USE_OPENGL
    Graphic* sprite;
    unsigned char* data;
    const unsigned char* sp;
    unsigned char* dp;
    unsigned char* cp;
    int fl;
    int n;
    int counter;
    int i;
    int h;
    int w;
#endif
    int depth;

    graphic = LoadGraphic(name);
    if (!width) {
	width = graphic->Width;
    }
    if (!height) {
	height = graphic->Height;
    }

    DebugCheck(width > graphic->Width || height > graphic->Height);

    depth = 8;

    if (((graphic->Width / width) * width != graphic->Width) ||
	    ((graphic->Height / height) * height != graphic->Height)) {
	fprintf(stderr, "Invalid graphic (width, height) %s\n", name);
	fprintf(stderr, "Expected: (%d,%d)  Found: (%d,%d)\n",
	    width, height, graphic->Width, graphic->Height);
    }
       
    // Check if width and height fits.
    DebugCheck(((graphic->Width / width) * width != graphic->Width) ||
	((graphic->Height / height) * height != graphic->Height));

#ifdef USE_OPENGL
    MakeTexture(graphic, width, height);
    graphic->NumFrames = (graphic->Width / width) * (graphic->Height / height);
    graphic->GraphicWidth = graphic->Width;
    graphic->GraphicHeight = graphic->Height;
    graphic->Width = width;
    graphic->Height = height;

    if (depth == 8) {
	graphic->Type = &GraphicSprite8Type;
    } else if (depth == 16) {
	graphic->Type = &GraphicSprite16Type;
    } else {
	fprintf(stderr, "Unsported image depth\n");
	ExitFatal(-1);
    }

    return graphic;
#else
    n = (graphic->Width / width) * (graphic->Height / height);
    DebugLevel3Fn("%dx%d in %dx%d = %d frames.\n" _C_ width _C_ height _C_
	graphic->Width _C_ graphic->Height _C_ n);

    //
    //	Allocate structure
    //
    sprite = malloc(sizeof(Graphic));
#ifdef DEBUG
    AllocatedGraphicMemory += sizeof(Graphic);
#endif

    if (!sprite) {
	fprintf(stderr, "Out of memory\n");
	ExitFatal(-1);
    }
    if (depth == 8) {
	sprite->Type = &GraphicSprite8Type;
    } else if (depth == 16) {
	sprite->Type = &GraphicSprite16Type;
    } else {
	fprintf(stderr, "Unsported image depth\n");
	ExitFatal(-1);
    }

    sprite->Width = width;
    sprite->Height = height;

    sprite->NumFrames = n;

    sprite->Palette = graphic->Palette;
    sprite->Pixels = graphic->Pixels;	// WARNING: if not shared freed below!

    sprite->Size = 0;
    sprite->Frames = NULL;

    // Worst case is alternating opaque and transparent pixels
    data = malloc(n * sizeof(unsigned char*) + 
	(graphic->Width / 2 + 1) * 3 * graphic->Height);
    // Pixel area
    dp = (unsigned char*)data + n * sizeof(unsigned char*);

    //
    //	Compress all frames of the sprite.
    //
    fl = graphic->Width / width;
    for (i = 0; i < n; ++i) {		// each frame
	((unsigned char**)data)[i] = dp;
	for (h = 0; h < height; ++h) {	// each line
	    sp = (const unsigned char*)graphic->Frames + (i % fl) * width +
		((i / fl) * height + h) * graphic->Width;

	    for (counter = w = 0; w < width; ++w) {
		if (*sp == 255 || *sp == 0) {			// start transparency
		    ++sp;
		    if (++counter == 256) {
			*dp++ = 255;
			*dp++ = 0;
			counter = 1;
		    }
		    continue;
		}
		*dp++ = counter;

		cp = dp++;
		counter = 0;
		for(; w < width; ++w) {			// opaque
		    *dp++ = *sp++;
		    if (++counter == 255) {
			*cp = 255;
			*dp++ = 0;
			cp = dp++;
			counter = 0;
		    }
		    // ARI: FIXME: wrong position
		    if (w + 1 != width && (*sp == 255 || *sp == 0)) {	// end transparency
			break;
		    }
		}
		*cp = counter;
		counter = 0;
	    }
	    if (counter) {
		*dp++ = counter;
		*dp++ = 0;		// 1 byte more, 1 check less! (phantom end transparency)
	    }
	}
    }

    DebugLevel3Fn("\t%d => %d RLE compressed\n" _C_
	graphic->Width * graphic->Height _C_ dp - data);

    //
    //	Update to real length
    //
    sprite->Frames = data;
    i = n * sizeof(unsigned char*) + dp - data;
    sprite->Size = i;
    dp = realloc(data, i);
    if (dp != data) {			// shrink only - happens rarely
	for (h = 0; h < n; ++h) {	// convert address
	    ((unsigned char**)dp)[h] += dp - data;
	}
	sprite->Frames = dp;
    }

#ifdef DEBUG
    CompressedGraphicMemory += i;
#endif

    graphic->Pixels = NULL;		// We own now the shared pixels
    VideoFree(graphic);

    return sprite;
#endif
#endif
}

/**
**	Init sprite
*/
global void InitSprite(void)
{
#ifdef USE_SDL_SURFACE

#else

#ifdef USE_OPENGL
    GraphicSprite8Type.Draw = VideoDrawOpenGL;
    GraphicSprite8Type.DrawClip = VideoDrawClipOpenGL;
    GraphicSprite8Type.DrawShadowClip = VideoDraw8to32ShadowClip;
    GraphicSprite8Type.DrawX = VideoDrawXOpenGL;
    GraphicSprite8Type.DrawClipX = VideoDrawClipXOpenGL;
    GraphicSprite8Type.DrawShadowClipX = VideoDraw8to32ShadowClipX;
    VideoDrawRawClip = VideoDrawRaw32Clip;
#else

    switch (VideoBpp) {
	case 8:
	    GraphicSprite8Type.Draw = VideoDraw8to8;
	    GraphicSprite8Type.DrawClip = VideoDraw8to8Clip;
	    GraphicSprite8Type.DrawShadowClip = VideoDraw8to8ShadowClip;
	    GraphicSprite8Type.DrawX = VideoDraw8to8X;
	    GraphicSprite8Type.DrawClipX = VideoDraw8to8ClipX;
	    GraphicSprite8Type.DrawShadowClipX = VideoDraw8to8ShadowClipX;
            VideoDrawRawClip = VideoDrawRaw8Clip;
	    break;

	case 15:
	case 16:
	    GraphicSprite8Type.Draw = VideoDraw8to16;
	    GraphicSprite8Type.DrawClip = VideoDraw8to16Clip;
	    GraphicSprite8Type.DrawShadowClip = VideoDraw8to16ShadowClip;
	    GraphicSprite8Type.DrawX = VideoDraw8to16X;
	    GraphicSprite8Type.DrawClipX = VideoDraw8to16ClipX;
	    GraphicSprite8Type.DrawShadowClipX = VideoDraw8to16ShadowClipX;
            VideoDrawRawClip = VideoDrawRaw16Clip;
	    break;

	case 24:
	    GraphicSprite8Type.Draw = VideoDraw8to24;
	    GraphicSprite8Type.DrawClip = VideoDraw8to24Clip;
	    GraphicSprite8Type.DrawShadowClip = VideoDraw8to24ShadowClip;
	    GraphicSprite8Type.DrawX = VideoDraw8to24X;
	    GraphicSprite8Type.DrawClipX = VideoDraw8to24ClipX;
	    GraphicSprite8Type.DrawShadowClipX = VideoDraw8to24ShadowClipX;
            VideoDrawRawClip = VideoDrawRaw24Clip;
	    break;

	case 32:
	    GraphicSprite8Type.Draw = VideoDraw8to32;
	    GraphicSprite8Type.DrawClip = VideoDraw8to32Clip;
	    GraphicSprite8Type.DrawShadowClip = VideoDraw8to32ShadowClip;
	    GraphicSprite8Type.DrawX = VideoDraw8to32X;
	    GraphicSprite8Type.DrawClipX = VideoDraw8to32ClipX;
	    GraphicSprite8Type.DrawShadowClipX = VideoDraw8to32ShadowClipX;
            VideoDrawRawClip = VideoDrawRaw32Clip;
	    break;

	default:
	    DebugLevel0Fn("Unsupported %d bpp\n" _C_ VideoBpp);
	    abort();
    }
#endif

    GraphicSprite8Type.Free = FreeSprite8;
#endif
}

//@}
