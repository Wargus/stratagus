//   ___________		     _________		      _____  __
//   \_	  _____/______   ____   ____ \_   ___ \____________ _/ ____\/  |_
//    |    __) \_  __ \_/ __ \_/ __ \/    \  \/\_  __ \__  \\   __\\   __\ 
//    |     \   |  | \/\  ___/\  ___/\     \____|  | \// __ \|  |   |  |
//    \___  /   |__|    \___  >\___  >\______  /|__|  (____  /__|   |__|
//	  \/		    \/	   \/	     \/		   \/
//  ______________________                           ______________________
//			  T H E   W A R   B E G I N S
//	   FreeCraft - A free fantasy real time strategy game engine
//
/**@name sprite.c	-	The general sprite functions. */
/*
**	(c) Copyright 2000 by Lutz Sammer
**
**	$Id$
*/

//@{

/*----------------------------------------------------------------------------
--	Includes
----------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>

#include "freecraft.h"
#include "video.h"

/*----------------------------------------------------------------------------
--	Declarations
----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
--	Externals
----------------------------------------------------------------------------*/

extern int ClipX1;			/// current clipping top left
extern int ClipY1;			/// current clipping top left
extern int ClipX2;			/// current clipping bottom right
extern int ClipY2;			/// current clipping bottom right

#ifdef DEBUG
extern unsigned AllocatedGraphicMemory;
extern unsigned CompressedGraphicMemory;
#endif

/*----------------------------------------------------------------------------
--	Variables
----------------------------------------------------------------------------*/

local GraphicType GraphicSprite8Type;	/// sprite type 8bit palette
local GraphicType GraphicSprite16Type;	/// sprite type 16bit palette

/*----------------------------------------------------------------------------
--	Local functions
----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
--	RLE Sprites
----------------------------------------------------------------------------*/

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
**	NOTE: This macro looks nice, but is absolutly no debugable.
**
**	@param bpp	Bit depth of target framebuffer
**	@param sprite	pointer to object
**	@param frame	number of frame (object index)
**	@param x	x coordinate on the screen
**	@param y	y coordinate on the screen
*/
#define RLE_BLIT(bpp,sprite,frame,x,y)	\
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
	pixels=(VMemType##bpp*)sprite->Pixels;			\
	sp=((unsigned char**)sprite->Frames)[frame];		\
	w=sprite->Width;					\
	da=VideoWidth-w;					\
	dp=VideoMemory##bpp+x+y*VideoWidth;			\
	ep=dp+VideoWidth*sprite->Height;			\
								\
	do {							\
	    lp=dp+w;						\
	    do {			/* 1 line */		\
		dp+=*sp++;		/* transparent # */	\
		pp=dp-1+*sp++;		/* opaque # */		\
		while( dp<pp ) {	/* unrolled */		\
		    *dp++=pixels[*sp++];			\
		    *dp++=pixels[*sp++];			\
		}						\
		if( dp<=pp ) {					\
		    *dp++=pixels[*sp++];			\
		}						\
	    } while( dp<lp );					\
	    dp+=da;						\
	} while( dp<ep );		/* all lines */		\
    } while( 0 )

/**
**	Draw 8bit graphic object unclipped into 8 bit framebuffer.
**
**	@param sprite	pointer to object
**	@param frame	number of frame (object index)
**	@param x	x coordinate on the screen
**	@param y	y coordinate on the screen
*/
global void VideoDraw8to8(const Graphic* sprite,unsigned frame,int x,int y)
{
    RLE_BLIT(8,sprite,frame,x,y);
}

/**
**	Draw 8bit graphic object unclipped into 16 bit framebuffer.
**
**	@param sprite	pointer to object
**	@param frame	number of frame (object index)
**	@param x	x coordinate on the screen
**	@param y	y coordinate on the screen
*/
global void VideoDraw8to16(const Graphic* sprite,unsigned frame,int x,int y)
{
    RLE_BLIT(16,sprite,frame,x,y);
}

/**
**	Draw 8bit graphic object unclipped into 24 bit framebuffer.
**
**	@param sprite	pointer to object
**	@param frame	number of frame (object index)
**	@param x	x coordinate on the screen
**	@param y	y coordinate on the screen
*/
global void VideoDraw8to24(const Graphic* sprite,unsigned frame,int x,int y)
{
    RLE_BLIT(24,sprite,frame,x,y);
}

/**
**	Draw 8bit graphic object unclipped into 32 bit framebuffer.
**
**	@param sprite	pointer to object
**	@param frame	number of frame (object index)
**	@param x	x coordinate on the screen
**	@param y	y coordinate on the screen
*/
global void VideoDraw8to32(const Graphic* sprite,unsigned frame,int x,int y)
{
    RLE_BLIT(32,sprite,frame,x,y);
}

/**
**	Draw 8bit graphic object unclipped and flipped in X direction
**	into 8 bit framebuffer.
**
**	@param sprite	pointer to object
**	@param frame	number of frame (object index)
**	@param x	x coordinate on the screen
**	@param y	y coordinate on the screen
*/
global void VideoDraw8to8X(const Graphic* sprite,unsigned frame,int x,int y)
{
    const unsigned char* sp;
    unsigned w;
    VMemType8* dp;
    const VMemType8* lp;
    const VMemType8* ep;
    const VMemType8* pp;
    const VMemType8* pixels;
    unsigned da;

    pixels=(VMemType8*)sprite->Pixels;
    sp=((unsigned char**)sprite->Frames)[frame];
    w=sprite->Width;
    dp=VideoMemory8+x+y*VideoWidth+w;
    da=VideoWidth+w;
    ep=dp+VideoWidth*sprite->Height;

    do {
	lp=dp-w;
	do {				// 1 line
	    dp-=*sp++;			// transparent
	    pp=dp+1-*sp++;		// opaque
	    while( dp>pp ) {		// unrolled
		*dp--=pixels[*sp++];
		*dp--=pixels[*sp++];
	    }
	    if( dp>=pp ) {
		*dp--=pixels[*sp++];
	    }
	} while( dp>lp );
	dp+=da;
    } while( dp<ep );			// all lines
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
global void VideoDraw8to16X(const Graphic* sprite,unsigned frame,int x,int y)
{
    const unsigned char* sp;
    unsigned w;
    VMemType16* dp;
    const VMemType16* lp;
    const VMemType16* ep;
    const VMemType16* pp;
    const VMemType16* pixels;
    unsigned da;

    pixels=(VMemType16*)sprite->Pixels;
    sp=((unsigned char**)sprite->Frames)[frame];
    w=sprite->Width;
    dp=VideoMemory16+x+y*VideoWidth+w;
    da=VideoWidth+w;
    ep=dp+VideoWidth*sprite->Height;

    do {
	lp=dp-w;
	do {				// 1 line
	    dp-=*sp++;			// transparent
	    pp=dp+1-*sp++;		// opaque
	    while( dp>pp ) {		// unrolled
		*dp--=pixels[*sp++];
		*dp--=pixels[*sp++];
	    }
	    if( dp>=pp ) {
		*dp--=pixels[*sp++];
	    }
	} while( dp>lp );
	dp+=da;
    } while( dp<ep );			// all lines
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
global void VideoDraw8to24X(const Graphic* sprite,unsigned frame,int x,int y)
{
    const unsigned char* sp;
    unsigned w;
    VMemType24* dp;
    const VMemType24* lp;
    const VMemType24* ep;
    const VMemType24* pp;
    const VMemType24* pixels;
    unsigned da;

    pixels=(VMemType24*)sprite->Pixels;
    sp=((unsigned char**)sprite->Frames)[frame];
    w=sprite->Width;
    dp=VideoMemory24+x+y*VideoWidth+w;
    da=VideoWidth+w;
    ep=dp+VideoWidth*sprite->Height;

    do {
	lp=dp-w;
	do {				// 1 line
	    dp-=*sp++;			// transparent
	    pp=dp+1-*sp++;		// opaque
	    while( dp>pp ) {		// unrolled
		*dp--=pixels[*sp++];
		*dp--=pixels[*sp++];
	    }
	    if( dp>=pp ) {
		*dp--=pixels[*sp++];
	    }
	} while( dp>lp );
	dp+=da;
    } while( dp<ep );			// all lines
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
global void VideoDraw8to32X(const Graphic* sprite,unsigned frame,int x,int y)
{
    const unsigned char* sp;
    unsigned w;
    VMemType32* dp;
    const VMemType32* lp;
    const VMemType32* ep;
    const VMemType32* pp;
    const VMemType32* pixels;
    unsigned da;

    pixels=(VMemType32*)sprite->Pixels;
    sp=((unsigned char**)sprite->Frames)[frame];
    w=sprite->Width;
    dp=VideoMemory32+x+y*VideoWidth+w;
    da=VideoWidth+w;
    ep=dp+VideoWidth*sprite->Height;

    do {
	lp=dp-w;
	do {				// 1 line
	    dp-=*sp++;			// transparent
	    pp=dp+1-*sp++;		// opaque
	    while( dp>pp ) {		// unrolled
		*dp--=pixels[*sp++];
		*dp--=pixels[*sp++];
	    }
	    if( dp>=pp ) {
		*dp--=pixels[*sp++];
	    }
	} while( dp>lp );
	dp+=da;
    } while( dp<ep );			// all lines
}

/**
**	Draw 8bit graphic object clipped into 8 bit framebuffer.
**
**	@param sprite	pointer to object
**	@param frame	number of frame (object index)
**	@param x	x coordinate on the screen
**	@param y	y coordinate on the screen
*/
global void VideoDraw8to8Clip(const Graphic* sprite,unsigned frame,int x,int y)
{
    int ox;
    int oy;
    int w;
    int h;
    const unsigned char* sp;
    unsigned sw;
    VMemType8* dp;
    const VMemType8* lp;
    const VMemType8* ep;
    VMemType8* pp;
    const VMemType8* pixels;
    unsigned da;

    ox=oy=0;
    sw=w=sprite->Width;
    h=sprite->Height;

    if( x<ClipX1 ) {			// reduce to visible range
	ox=ClipX1-x;
	w-=ox;
	x=ClipX1;
    }
    if( x+w>ClipX2 ) {
	w=ClipX2-x;
    }

    if( y<ClipY1 ) {
	oy=ClipY1-y;
	h-=oy;
	y=ClipY1;
    }
    if( y+h>ClipY2 ) {
	h=ClipY2-y;
    }

    if( w<=0 || h<=0 ) {		// nothing to draw
	return;
    }

    //
    //	Draw the clipped sprite
    //
    pixels=(VMemType8*)sprite->Pixels;
    sp=((unsigned char**)sprite->Frames)[frame];

    //
    //	Skip top lines, if needed.
    //
    while( oy-- ) {
	da=0;
	do {
	    da+=*sp++;			// transparent
	    da+=*sp;			// opaque
	    sp+=*sp+1;
	} while( da<sw );
    }

    da=VideoWidth-sw;
    dp=VideoMemory8+x+y*VideoWidth;
    ep=dp+VideoWidth*h;

    if( w==sw ) {			// Unclipped horizontal

	do {
	    lp=dp+sw;
	    do {			// 1 line
		dp+=*sp++;		// transparent
		pp=dp-1+*sp++;		// opaque
		while( dp<pp ) {	// unroll
		    *dp++=pixels[*sp++];
		    *dp++=pixels[*sp++];
		}
		if( dp<=pp ) {
		    *dp++=pixels[*sp++];
		}
	    } while( dp<lp );
	    dp+=da;
	} while( dp<ep );		// all lines

    } else {				// Clip horizontal

	da+=ox;
	do {
	    lp=dp+w;
	    //
	    //	Clip left
	    //
	    pp=dp-ox;
	    for( ;; ) {
		pp+=*sp++;		// transparent
		if( pp>=dp ) {
		    dp=pp;
		    goto middle_trans;
		}
		pp+=*sp;		// opaque
		if( pp>=dp ) {
		    sp+=*sp-(pp-dp)+1;
		    goto middle_pixel;
		}
		sp+=*sp+1;
	    }

	    //
	    //	Draw middle
	    //
	    for( ;; ) {
		dp+=*sp++;		// transparent
middle_trans:
		if( dp>=lp ) {
		    lp+=sw-w-ox;
		    goto right_trans;
		}
		pp=dp+*sp++;		// opaque
middle_pixel:
		if( pp<lp ) {
		    while( dp<pp ) {
			*dp++=pixels[*sp++];
		    }
		    continue;
		}
		while( dp<lp ) {
		    *dp++=pixels[*sp++];
		}
		sp+=pp-dp;
		dp=pp;
		break;
	    }

	    //
	    //	Clip right
	    //
	    lp+=sw-w-ox;
	    while( dp<lp ) {
		dp+=*sp++;		// transparent
right_trans:
		dp+=*sp;		// opaque
		sp+=*sp+1;
	    }
	    dp+=da;
	} while( dp<ep );		// all lines

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
global void VideoDraw8to16Clip(const Graphic* sprite,unsigned frame,int x,int y)
{
    int ox;
    int oy;
    int w;
    int h;
    const unsigned char* sp;
    unsigned sw;
    VMemType16* dp;
    const VMemType16* lp;
    const VMemType16* ep;
    VMemType16* pp;
    const VMemType16* pixels;
    unsigned da;

    ox=oy=0;
    sw=w=sprite->Width;
    h=sprite->Height;

    if( x<ClipX1 ) {			// reduce to visible range
	ox=ClipX1-x;
	w-=ox;
	x=ClipX1;
    }
    if( x+w>ClipX2 ) {
	w=ClipX2-x;
    }

    if( y<ClipY1 ) {
	oy=ClipY1-y;
	h-=oy;
	y=ClipY1;
    }
    if( y+h>ClipY2 ) {
	h=ClipY2-y;
    }

    if( w<=0 || h<=0 ) {		// nothing to draw
	return;
    }

    //
    //	Draw the clipped sprite
    //
    pixels=(VMemType16*)sprite->Pixels;
    sp=((unsigned char**)sprite->Frames)[frame];

    //
    //	Skip top lines, if needed.
    //
    while( oy-- ) {
	da=0;
	do {
	    da+=*sp++;			// transparent
	    da+=*sp;			// opaque
	    sp+=*sp+1;
	} while( da<sw );
    }

    da=VideoWidth-sw;
    dp=VideoMemory16+x+y*VideoWidth;
    ep=dp+VideoWidth*h;

    if( w==sw ) {			// Unclipped horizontal

	do {
	    lp=dp+sw;
	    do {			// 1 line
		dp+=*sp++;		// transparent
		pp=dp-1+*sp++;		// opaque
		while( dp<pp ) {	// unroll
		    *dp++=pixels[*sp++];
		    *dp++=pixels[*sp++];
		}
		if( dp<=pp ) {
		    *dp++=pixels[*sp++];
		}
	    } while( dp<lp );
	    dp+=da;
	} while( dp<ep );		// all lines

    } else {				// Clip horizontal

	da+=ox;
	do {
	    lp=dp+w;
	    //
	    //	Clip left
	    //
	    pp=dp-ox;
	    for( ;; ) {
		pp+=*sp++;		// transparent
		if( pp>=dp ) {
		    dp=pp;
		    goto middle_trans;
		}
		pp+=*sp;		// opaque
		if( pp>=dp ) {
		    sp+=*sp-(pp-dp)+1;
		    goto middle_pixel;
		}
		sp+=*sp+1;
	    }

	    //
	    //	Draw middle
	    //
	    for( ;; ) {
		dp+=*sp++;		// transparent
middle_trans:
		if( dp>=lp ) {
		    lp+=sw-w-ox;
		    goto right_trans;
		}
		pp=dp+*sp++;		// opaque
middle_pixel:
		if( pp<lp ) {
		    while( dp<pp ) {
			*dp++=pixels[*sp++];
		    }
		    continue;
		}
		while( dp<lp ) {
		    *dp++=pixels[*sp++];
		}
		sp+=pp-dp;
		dp=pp;
		break;
	    }

	    //
	    //	Clip right
	    //
	    lp+=sw-w-ox;
	    while( dp<lp ) {
		dp+=*sp++;		// transparent
right_trans:
		dp+=*sp;		// opaque
		sp+=*sp+1;
	    }
	    dp+=da;
	} while( dp<ep );		// all lines

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
global void VideoDraw8to24Clip(const Graphic* sprite,unsigned frame,int x,int y)
{
    int ox;
    int oy;
    int w;
    int h;
    const unsigned char* sp;
    unsigned sw;
    VMemType24* dp;
    const VMemType24* lp;
    const VMemType24* ep;
    VMemType24* pp;
    const VMemType24* pixels;
    unsigned da;

    ox=oy=0;
    sw=w=sprite->Width;
    h=sprite->Height;

    if( x<ClipX1 ) {			// reduce to visible range
	ox=ClipX1-x;
	w-=ox;
	x=ClipX1;
    }
    if( x+w>ClipX2 ) {
	w=ClipX2-x;
    }

    if( y<ClipY1 ) {
	oy=ClipY1-y;
	h-=oy;
	y=ClipY1;
    }
    if( y+h>ClipY2 ) {
	h=ClipY2-y;
    }

    if( w<=0 || h<=0 ) {		// nothing to draw
	return;
    }

    //
    //	Draw the clipped sprite
    //
    pixels=(VMemType24*)sprite->Pixels;
    sp=((unsigned char**)sprite->Frames)[frame];

    //
    //	Skip top lines, if needed.
    //
    while( oy-- ) {
	da=0;
	do {
	    da+=*sp++;			// transparent
	    da+=*sp;			// opaque
	    sp+=*sp+1;
	} while( da<sw );
    }

    da=VideoWidth-sw;
    dp=VideoMemory24+x+y*VideoWidth;
    ep=dp+VideoWidth*h;

    if( w==sw ) {			// Unclipped horizontal

	do {
	    lp=dp+sw;
	    do {			// 1 line
		dp+=*sp++;		// transparent
		pp=dp-1+*sp++;		// opaque
		while( dp<pp ) {	// unroll
		    *dp++=pixels[*sp++];
		    *dp++=pixels[*sp++];
		}
		if( dp<=pp ) {
		    *dp++=pixels[*sp++];
		}
	    } while( dp<lp );
	    dp+=da;
	} while( dp<ep );		// all lines

    } else {				// Clip horizontal

	da+=ox;
	do {
	    lp=dp+w;
	    //
	    //	Clip left
	    //
	    pp=dp-ox;
	    for( ;; ) {
		pp+=*sp++;		// transparent
		if( pp>=dp ) {
		    dp=pp;
		    goto middle_trans;
		}
		pp+=*sp;		// opaque
		if( pp>=dp ) {
		    sp+=*sp-(pp-dp)+1;
		    goto middle_pixel;
		}
		sp+=*sp+1;
	    }

	    //
	    //	Draw middle
	    //
	    for( ;; ) {
		dp+=*sp++;		// transparent
middle_trans:
		if( dp>=lp ) {
		    lp+=sw-w-ox;
		    goto right_trans;
		}
		pp=dp+*sp++;		// opaque
middle_pixel:
		if( pp<lp ) {
		    while( dp<pp ) {
			*dp++=pixels[*sp++];
		    }
		    continue;
		}
		while( dp<lp ) {
		    *dp++=pixels[*sp++];
		}
		sp+=pp-dp;
		dp=pp;
		break;
	    }

	    //
	    //	Clip right
	    //
	    lp+=sw-w-ox;
	    while( dp<lp ) {
		dp+=*sp++;		// transparent
right_trans:
		dp+=*sp;		// opaque
		sp+=*sp+1;
	    }
	    dp+=da;
	} while( dp<ep );		// all lines

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
global void VideoDraw8to32Clip(const Graphic* sprite,unsigned frame,int x,int y)
{
    int ox;
    int oy;
    int w;
    int h;
    const unsigned char* sp;
    unsigned sw;
    VMemType32* dp;
    const VMemType32* lp;
    const VMemType32* ep;
    VMemType32* pp;
    const VMemType32* pixels;
    unsigned da;

    ox=oy=0;
    sw=w=sprite->Width;
    h=sprite->Height;

    if( x<ClipX1 ) {			// reduce to visible range
	ox=ClipX1-x;
	w-=ox;
	x=ClipX1;
    }
    if( x+w>ClipX2 ) {
	w=ClipX2-x;
    }

    if( y<ClipY1 ) {
	oy=ClipY1-y;
	h-=oy;
	y=ClipY1;
    }
    if( y+h>ClipY2 ) {
	h=ClipY2-y;
    }

    if( w<=0 || h<=0 ) {		// nothing to draw
	return;
    }

    //
    //	Draw the clipped sprite
    //
    pixels=(VMemType32*)sprite->Pixels;
    sp=((unsigned char**)sprite->Frames)[frame];

    //
    //	Skip top lines, if needed.
    //
    while( oy-- ) {
	da=0;
	do {
	    da+=*sp++;			// transparent
	    da+=*sp;			// opaque
	    sp+=*sp+1;
	} while( da<sw );
    }

    da=VideoWidth-sw;
    dp=VideoMemory32+x+y*VideoWidth;
    ep=dp+VideoWidth*h;

    if( w==sw ) {			// Unclipped horizontal

	do {
	    lp=dp+sw;
	    do {			// 1 line
		dp+=*sp++;		// transparent
		pp=dp-1+*sp++;		// opaque
		while( dp<pp ) {	// unroll
		    *dp++=pixels[*sp++];
		    *dp++=pixels[*sp++];
		}
		if( dp<=pp ) {
		    *dp++=pixels[*sp++];
		}
	    } while( dp<lp );
	    dp+=da;
	} while( dp<ep );		// all lines

    } else {				// Clip horizontal

	da+=ox;
	do {
	    lp=dp+w;
	    //
	    //	Clip left
	    //
	    pp=dp-ox;
	    for( ;; ) {
		pp+=*sp++;		// transparent
		if( pp>=dp ) {
		    dp=pp;
		    goto middle_trans;
		}
		pp+=*sp;		// opaque
		if( pp>=dp ) {
		    sp+=*sp-(pp-dp)+1;
		    goto middle_pixel;
		}
		sp+=*sp+1;
	    }

	    //
	    //	Draw middle
	    //
	    for( ;; ) {
		dp+=*sp++;		// transparent
middle_trans:
		if( dp>=lp ) {
		    lp+=sw-w-ox;
		    goto right_trans;
		}
		pp=dp+*sp++;		// opaque
middle_pixel:
		if( pp<lp ) {
		    while( dp<pp ) {
			*dp++=pixels[*sp++];
		    }
		    continue;
		}
		while( dp<lp ) {
		    *dp++=pixels[*sp++];
		}
		sp+=pp-dp;
		dp=pp;
		break;
	    }

	    //
	    //	Clip right
	    //
	    lp+=sw-w-ox;
	    while( dp<lp ) {
		dp+=*sp++;		// transparent
right_trans:
		dp+=*sp;		// opaque
		sp+=*sp+1;
	    }
	    dp+=da;
	} while( dp<ep );		// all lines

    }
}

/**
**	Draw 8bit graphic object clipped and flipped in X direction
**	into 8 bit framebuffer.
**
**	@param sprite	pointer to object
**	@param frame	number of frame (object index)
**	@param x	x coordinate on the screen
**	@param y	y coordinate on the screen
*/
global void VideoDraw8to8ClipX(const Graphic* sprite,unsigned frame,int x,int y)
{
    int ox;
    int oy;
    int w;
    int h;
    const unsigned char* sp;
    unsigned sw;
    VMemType8* dp;
    const VMemType8* lp;
    const VMemType8* ep;
    VMemType8* pp;
    const VMemType8* pixels;
    unsigned da;

    ox=oy=0;
    sw=w=sprite->Width;
    h=sprite->Height;

    if( x<ClipX1 ) {			// reduce to visible range
	ox=ClipX1-x;
	w-=ox;
	x=ClipX1;
    }
    if( x+w>ClipX2 ) {
	w=ClipX2-x;
    }

    if( y<ClipY1 ) {
	oy=ClipY1-y;
	h-=oy;
	y=ClipY1;
    }
    if( y+h>ClipY2 ) {
	h=ClipY2-y;
    }

    if( w<=0 || h<=0 ) {		// nothing to draw
	return;
    }

    //
    //	Draw the clipped sprite
    //
    pixels=(VMemType8*)sprite->Pixels;
    sp=((unsigned char**)sprite->Frames)[frame];

    //
    // Skip top lines
    //
    while( oy-- ) {
	da=0;
	do {
	    da+=*sp++;			// transparent
	    da+=*sp;			// opaque
	    sp+=*sp+1;
	} while( da<sw );
    }

    da=VideoWidth+sw;
    dp=VideoMemory8+x+y*VideoWidth+w;
    ep=dp+VideoWidth*h;

    if( w==sw ) {			// Unclipped horizontal

	while( dp<ep ) {		// all lines
	    lp=dp-w;
	    do {			// 1 line
		dp-=*sp++;		// transparent
		pp=dp+1-*sp++;		// opaque
		while( dp>pp ) {
		    *dp--=pixels[*sp++];
		    *dp--=pixels[*sp++];
		}
		if( dp>=pp ) {
		    *dp--=pixels[*sp++];
		}
	    } while( dp>lp );
	    dp+=da;
	}

    } else {				// Clip horizontal

	da-=sw-w-ox;
	while( dp<ep ) {		// all lines
	    lp=dp-w;
	    //
	    //	Clip right side
	    //
	    pp=dp+sw-w-ox;
	    for( ;; ) {
		pp-=*sp++;		// transparent
		if( pp<=dp ) {
		    dp=pp;
		    goto middle_trans;
		}
		pp-=*sp;		// opaque
		if( pp<=dp ) {
		    sp+=*sp-(dp-pp)+1;
		    goto middle_pixel;
		}
		sp+=*sp+1;
	    }

	    //
	    //	Draw middle
	    //
	    for( ;; ) {
		dp-=*sp++;		// transparent
middle_trans:
		if( dp<=lp ) {
		    lp-=ox;
		    goto right_trans;
		}
		pp=dp-*sp++;		// opaque
middle_pixel:
		if( pp>lp ) {
		    while( dp>pp ) {
			*dp--=pixels[*sp++];
		    }
		    continue;
		}
		while( dp>lp ) {
		    *dp--=pixels[*sp++];
		}
		sp+=dp-pp;
		dp=pp;
		break;
	    }

	    //
	    //	Clip left side
	    //
	    lp-=ox;
	    while( dp>lp ) {
		dp-=*sp++;		// transparent
right_trans:
		dp-=*sp;		// opaque
		sp+=*sp+1;
	    }
	    dp+=da;

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
global void VideoDraw8to16ClipX(const Graphic* sprite,unsigned frame
	,int x,int y)
{
    int ox;
    int oy;
    int w;
    int h;
    const unsigned char* sp;
    unsigned sw;
    VMemType16* dp;
    const VMemType16* lp;
    const VMemType16* ep;
    VMemType16* pp;
    const VMemType16* pixels;
    unsigned da;

    ox=oy=0;
    sw=w=sprite->Width;
    h=sprite->Height;

    if( x<ClipX1 ) {			// reduce to visible range
	ox=ClipX1-x;
	w-=ox;
	x=ClipX1;
    }
    if( x+w>ClipX2 ) {
	w=ClipX2-x;
    }

    if( y<ClipY1 ) {
	oy=ClipY1-y;
	h-=oy;
	y=ClipY1;
    }
    if( y+h>ClipY2 ) {
	h=ClipY2-y;
    }

    if( w<=0 || h<=0 ) {		// nothing to draw
	return;
    }

    //
    //	Draw the clipped sprite
    //
    pixels=(VMemType16*)sprite->Pixels;
    sp=((unsigned char**)sprite->Frames)[frame];

    //
    // Skip top lines
    //
    while( oy-- ) {
	da=0;
	do {
	    da+=*sp++;			// transparent
	    da+=*sp;			// opaque
	    sp+=*sp+1;
	} while( da<sw );
    }

    da=VideoWidth+sw;
    dp=VideoMemory16+x+y*VideoWidth+w;
    ep=dp+VideoWidth*h;

    if( w==sw ) {			// Unclipped horizontal

	while( dp<ep ) {		// all lines
	    lp=dp-w;
	    do {			// 1 line
		dp-=*sp++;		// transparent
		pp=dp+1-*sp++;		// opaque
		while( dp>pp ) {
		    *dp--=pixels[*sp++];
		    *dp--=pixels[*sp++];
		}
		if( dp>=pp ) {
		    *dp--=pixels[*sp++];
		}
	    } while( dp>lp );
	    dp+=da;
	}

    } else {				// Clip horizontal

	da-=sw-w-ox;
	while( dp<ep ) {		// all lines
	    lp=dp-w;
	    //
	    //	Clip right side
	    //
	    pp=dp+sw-w-ox;
	    for( ;; ) {
		pp-=*sp++;		// transparent
		if( pp<=dp ) {
		    dp=pp;
		    goto middle_trans;
		}
		pp-=*sp;		// opaque
		if( pp<=dp ) {
		    sp+=*sp-(dp-pp)+1;
		    goto middle_pixel;
		}
		sp+=*sp+1;
	    }

	    //
	    //	Draw middle
	    //
	    for( ;; ) {
		dp-=*sp++;		// transparent
middle_trans:
		if( dp<=lp ) {
		    lp-=ox;
		    goto right_trans;
		}
		pp=dp-*sp++;		// opaque
middle_pixel:
		if( pp>lp ) {
		    while( dp>pp ) {
			*dp--=pixels[*sp++];
		    }
		    continue;
		}
		while( dp>lp ) {
		    *dp--=pixels[*sp++];
		}
		sp+=dp-pp;
		dp=pp;
		break;
	    }

	    //
	    //	Clip left side
	    //
	    lp-=ox;
	    while( dp>lp ) {
		dp-=*sp++;		// transparent
right_trans:
		dp-=*sp;		// opaque
		sp+=*sp+1;
	    }
	    dp+=da;

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
global void VideoDraw8to24ClipX(const Graphic* sprite,unsigned frame
	,int x,int y)
{
    int ox;
    int oy;
    int w;
    int h;
    const unsigned char* sp;
    unsigned sw;
    VMemType24* dp;
    const VMemType24* lp;
    const VMemType24* ep;
    VMemType24* pp;
    const VMemType24* pixels;
    unsigned da;

    ox=oy=0;
    sw=w=sprite->Width;
    h=sprite->Height;

    if( x<ClipX1 ) {			// reduce to visible range
	ox=ClipX1-x;
	w-=ox;
	x=ClipX1;
    }
    if( x+w>ClipX2 ) {
	w=ClipX2-x;
    }

    if( y<ClipY1 ) {
	oy=ClipY1-y;
	h-=oy;
	y=ClipY1;
    }
    if( y+h>ClipY2 ) {
	h=ClipY2-y;
    }

    if( w<=0 || h<=0 ) {		// nothing to draw
	return;
    }

    //
    //	Draw the clipped sprite
    //
    pixels=(VMemType24*)sprite->Pixels;
    sp=((unsigned char**)sprite->Frames)[frame];

    //
    // Skip top lines
    //
    while( oy-- ) {
	da=0;
	do {
	    da+=*sp++;			// transparent
	    da+=*sp;			// opaque
	    sp+=*sp+1;
	} while( da<sw );
    }

    da=VideoWidth+sw;
    dp=VideoMemory24+x+y*VideoWidth+w;
    ep=dp+VideoWidth*h;

    if( w==sw ) {			// Unclipped horizontal

	while( dp<ep ) {		// all lines
	    lp=dp-w;
	    do {			// 1 line
		dp-=*sp++;		// transparent
		pp=dp+1-*sp++;		// opaque
		while( dp>pp ) {
		    *dp--=pixels[*sp++];
		    *dp--=pixels[*sp++];
		}
		if( dp>=pp ) {
		    *dp--=pixels[*sp++];
		}
	    } while( dp>lp );
	    dp+=da;
	}

    } else {				// Clip horizontal

	da-=sw-w-ox;
	while( dp<ep ) {		// all lines
	    lp=dp-w;
	    //
	    //	Clip right side
	    //
	    pp=dp+sw-w-ox;
	    for( ;; ) {
		pp-=*sp++;		// transparent
		if( pp<=dp ) {
		    dp=pp;
		    goto middle_trans;
		}
		pp-=*sp;		// opaque
		if( pp<=dp ) {
		    sp+=*sp-(dp-pp)+1;
		    goto middle_pixel;
		}
		sp+=*sp+1;
	    }

	    //
	    //	Draw middle
	    //
	    for( ;; ) {
		dp-=*sp++;		// transparent
middle_trans:
		if( dp<=lp ) {
		    lp-=ox;
		    goto right_trans;
		}
		pp=dp-*sp++;		// opaque
middle_pixel:
		if( pp>lp ) {
		    while( dp>pp ) {
			*dp--=pixels[*sp++];
		    }
		    continue;
		}
		while( dp>lp ) {
		    *dp--=pixels[*sp++];
		}
		sp+=dp-pp;
		dp=pp;
		break;
	    }

	    //
	    //	Clip left side
	    //
	    lp-=ox;
	    while( dp>lp ) {
		dp-=*sp++;		// transparent
right_trans:
		dp-=*sp;		// opaque
		sp+=*sp+1;
	    }
	    dp+=da;

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
global void VideoDraw8to32ClipX(const Graphic* sprite,unsigned frame
	,int x,int y)
{
    int ox;
    int oy;
    int w;
    int h;
    const unsigned char* sp;
    unsigned sw;
    VMemType32* dp;
    const VMemType32* lp;
    const VMemType32* ep;
    VMemType32* pp;
    const VMemType32* pixels;
    unsigned da;

    ox=oy=0;
    sw=w=sprite->Width;
    h=sprite->Height;

    if( x<ClipX1 ) {			// reduce to visible range
	ox=ClipX1-x;
	w-=ox;
	x=ClipX1;
    }
    if( x+w>ClipX2 ) {
	w=ClipX2-x;
    }

    if( y<ClipY1 ) {
	oy=ClipY1-y;
	h-=oy;
	y=ClipY1;
    }
    if( y+h>ClipY2 ) {
	h=ClipY2-y;
    }

    if( w<=0 || h<=0 ) {		// nothing to draw
	return;
    }

    //
    //	Draw the clipped sprite
    //
    pixels=(VMemType32*)sprite->Pixels;
    sp=((unsigned char**)sprite->Frames)[frame];

    //
    // Skip top lines
    //
    while( oy-- ) {
	da=0;
	do {
	    da+=*sp++;			// transparent
	    da+=*sp;			// opaque
	    sp+=*sp+1;
	} while( da<sw );
    }

    da=VideoWidth+sw;
    dp=VideoMemory32+x+y*VideoWidth+w;
    ep=dp+VideoWidth*h;

    if( w==sw ) {			// Unclipped horizontal

	while( dp<ep ) {		// all lines
	    lp=dp-w;
	    do {			// 1 line
		dp-=*sp++;		// transparent
		pp=dp+1-*sp++;		// opaque
		while( dp>pp ) {
		    *dp--=pixels[*sp++];
		    *dp--=pixels[*sp++];
		}
		if( dp>=pp ) {
		    *dp--=pixels[*sp++];
		}
	    } while( dp>lp );
	    dp+=da;
	}

    } else {				// Clip horizontal

	da-=sw-w-ox;
	while( dp<ep ) {		// all lines
	    lp=dp-w;
	    //
	    //	Clip right side
	    //
	    pp=dp+sw-w-ox;
	    for( ;; ) {
		pp-=*sp++;		// transparent
		if( pp<=dp ) {
		    dp=pp;
		    goto middle_trans;
		}
		pp-=*sp;		// opaque
		if( pp<=dp ) {
		    sp+=*sp-(dp-pp)+1;
		    goto middle_pixel;
		}
		sp+=*sp+1;
	    }

	    //
	    //	Draw middle
	    //
	    for( ;; ) {
		dp-=*sp++;		// transparent
middle_trans:
		if( dp<=lp ) {
		    lp-=ox;
		    goto right_trans;
		}
		pp=dp-*sp++;		// opaque
middle_pixel:
		if( pp>lp ) {
		    while( dp>pp ) {
			*dp--=pixels[*sp++];
		    }
		    continue;
		}
		while( dp>lp ) {
		    *dp--=pixels[*sp++];
		}
		sp+=dp-pp;
		dp=pp;
		break;
	    }

	    //
	    //	Clip left side
	    //
	    lp-=ox;
	    while( dp>lp ) {
		dp-=*sp++;		// transparent
right_trans:
		dp-=*sp;		// opaque
		sp+=*sp+1;
	    }
	    dp+=da;

	}
    }
}

/**
**	Free graphic object.
*/
local void FreeSprite8(Graphic* graphic)
{
    IfDebug( AllocatedGraphicMemory-=graphic->Size );
    IfDebug( AllocatedGraphicMemory-=sizeof(Graphic) );
    free(graphic->Frames);
    free(graphic);
}

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
**	@return 	A graphic object for the loaded sprite.
**
**	@see	LoadGraphic
*/
global Graphic* LoadSprite(const char* name,unsigned width,unsigned height)
{
    Graphic* sprite;
    Graphic* graphic;
    unsigned char* data;
    const unsigned char* sp;
    unsigned char* dp;
    unsigned char* cp;
    int depth;
    int fl;
    int n;
    int counter;
    int i;
    int h;
    int w;

    graphic=LoadGraphic(name);
    if( !width ) {			// FIXME: this is hack for cursors!
	width=graphic->Width;
    }
    if( !height ) {
	height=graphic->Height;
    }
    depth=8;

    // Check if width and height fits.
    DebugCheck( ((graphic->Width/width)*width!=graphic->Width)
	    || ((graphic->Height/height)*height!=graphic->Height) );

    n=(graphic->Width/width)*(graphic->Height/height);
    DebugLevel3Fn("%dx%d in %dx%d = %d frames.\n"
	    ,width,height,graphic->Width,graphic->Height,n);

    //
    //	Allocate structure
    //
    sprite=malloc(sizeof(Graphic));
    IfDebug( AllocatedGraphicMemory+=sizeof(Graphic) );

    if( !sprite ) {
	fprintf(stderr,"Out of memory\n");
	exit(-1);
    }
    if( depth==8 ) {
	sprite->Type=&GraphicSprite8Type;
    } else if( depth==16 ) {
	sprite->Type=&GraphicSprite16Type;
    } else {
	fprintf(stderr,"Unsported image depth\n");
	exit(-1);
    }
    sprite->Width=width;
    sprite->Height=height;

    sprite->NumFrames=n;

    sprite->Palette=graphic->Palette;
    sprite->Pixels=graphic->Pixels;	// WARNING: if not shared freed below!

    sprite->Size=0;
    sprite->Frames=NULL;


    // Worst case is alternating opaque and transparent pixels
    data=malloc(n*sizeof(unsigned char*)
	    +(graphic->Width/2+1)*3*graphic->Height);
    // Pixel area
    dp=(unsigned char *)data+n*sizeof(unsigned char*);

    //
    //	Compress all frames of the sprite.
    //
    fl=graphic->Width/width;
    for( i=0; i<n; ++i ) {		// each frame
	((unsigned char**)data)[i]=dp;
	for( h=0; h<height; ++h ) {	// each line
	    sp=graphic->Frames+(i%fl)*width+((i/fl)*height+h)*graphic->Width;

	    for( counter=w=0; w<width; ++w ) {
		if( *sp==255 ) {	// transparent
		    ++sp;
		    if( ++counter==256 ) {
			*dp++=255;
			*dp++=0;
			counter=1;
		    }
		    continue;
		}
		*dp++=counter;

		cp=dp++;
		counter=0;
		for( ; w<width; ++w ) {		// opaque
		    *dp++=*sp++;
		    if( ++counter==255 ) {
			*cp=255;
			*dp++=0;
			cp=dp++;
			counter=0;
		    }
		    // ARI: FIXME: wrong position
		    if( w+1!=width && *sp==255 ) {	// transparent
			break;
		    }
		}
		*cp=counter;
		counter=0;
	    }
	    if( counter ) {
		*dp++=counter;
		*dp++=0;		// 1 byte more, 1 check less!
	    }
	}
    }

    DebugLevel3Fn("\t%d => %d RLE compressed\n"
	    ,graphic->Width*graphic->Height,dp-data);

    //
    //	Update to real length
    //
    sprite->Frames=data;
    i=n*sizeof(unsigned char*)+dp-data;
    sprite->Size=i;
    dp=realloc(data,i);
    if( dp!=data ) {			// shrink only - happens rarely
	for( h=0; h<n; ++h ) {		// convert address
	    ((unsigned char**)dp)[h]+=dp-data;
	}
	sprite->Frames=dp;
    }

    IfDebug( CompressedGraphicMemory+=i; );

    VideoFree(graphic);

    return sprite;
}

/**
**	Init sprite
*/
global void InitSprite(void)
{
    switch( VideoDepth ) {
	case 8:
	    GraphicSprite8Type.Draw=VideoDraw8to8;
	    GraphicSprite8Type.DrawClip=VideoDraw8to8Clip;
	    GraphicSprite8Type.DrawX=VideoDraw8to8X;
	    GraphicSprite8Type.DrawClipX=VideoDraw8to8ClipX;
	    break;

	case 15:
	case 16:
	    GraphicSprite8Type.Draw=VideoDraw8to16;
	    GraphicSprite8Type.DrawClip=VideoDraw8to16Clip;
	    GraphicSprite8Type.DrawX=VideoDraw8to16X;
	    GraphicSprite8Type.DrawClipX=VideoDraw8to16ClipX;
	    break;

	case 24:
	    GraphicSprite8Type.Draw=VideoDraw8to24;
	    GraphicSprite8Type.DrawClip=VideoDraw8to24Clip;
	    GraphicSprite8Type.DrawX=VideoDraw8to24X;
	    GraphicSprite8Type.DrawClipX=VideoDraw8to24ClipX;
	    break;

	case 32:
	    GraphicSprite8Type.Draw=VideoDraw8to32;
	    GraphicSprite8Type.DrawClip=VideoDraw8to32Clip;
	    GraphicSprite8Type.DrawX=VideoDraw8to32X;
	    GraphicSprite8Type.DrawClipX=VideoDraw8to32ClipX;
	    break;

	default:
	    DebugLevel0Fn("Unsupported %d bpp\n",VideoDepth);
	    abort();
    }

    GraphicSprite8Type.Free=FreeSprite8;
}

//@}
