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
/**@name video.c	-	The video. */
/*
**	(c) Copyright 1998,2000 by Lutz Sammer
**
**	$Id$
*/

//@{

#include <stdio.h>
#include <stdlib.h>

#include "clone.h"
#include "video.h"

//	FIXME: this functions only supports 16 bit displays!!!!

#ifdef DEBUG
global unsigned AllocatedGraphicMemory;
global unsigned CompressedGraphicMemory;
#endif


/*----------------------------------------------------------------------------
--	Clipping
----------------------------------------------------------------------------*/

global int ClipX1;			/// current clipping top left
global int ClipY1;			/// current clipping top left
global int ClipX2;			/// current clipping bottom right
global int ClipY2;			/// current clipping bottom right

/*
**	Set clipping for sprite/line routines.
*/
global void SetClipping(int left,int top,int right,int bottom)
{
    if( left>right ) { left^=right; right^=left; left^=right; }
    if( top>bottom ) { top^=bottom; bottom^=top; top^=bottom; }
    
    if( left<0 )    left=0;
    if( top<0 )	    top=0;
    if( right<0 )   right=0;
    if( bottom<0 )  bottom=0;

    if( left>=VideoWidth )	left=VideoWidth-1;
    if( right>=VideoWidth )	right=VideoWidth-1;
    if( bottom>=VideoHeight ) bottom=VideoHeight-1;
    if( top>=VideoHeight )	top=VideoHeight-1;
    
    ClipX1=left;
    ClipY1=top;
    ClipX2=right;
    ClipY2=bottom;
}

/*----------------------------------------------------------------------------
--	RLE Sprites
----------------------------------------------------------------------------*/

//	FIXME: can also compress same bytes
//	Count:	transparent

/*----------------------------------------------------------------------------
--	RLE Sprites 8bit
----------------------------------------------------------------------------*/

/*
**	Draw rle compressed sprite.
*/
global void DrawRleSprite8(RleSprite* sprite,unsigned frame,int x,int y)
{
    const unsigned char* sp;
    unsigned w;
    VMemType8* dp;
    VMemType8* lp;
    VMemType8* ep;
    VMemType8* pp;
    unsigned da;

    sp=sprite->Frames[frame];
    w=sprite->Width;
    da=VideoWidth-w;
    dp=VideoMemory8+x+y*VideoWidth;
    ep=dp+VideoWidth*sprite->Height;

    while( dp<ep ) {			// all lines
	lp=dp+w;
	do {				// 1 line
	    dp+=*sp++;			// transparent
	    if( dp>=lp ) {
		break;
	    }
	    pp=dp-1+*sp++;		// non-transparent
	    while( dp<pp ) {
		*dp++=Pixels8[*sp++];
		*dp++=Pixels8[*sp++];
	    }
	    if( dp<=pp ) {
		*dp++=Pixels8[*sp++];
	    }
	} while( dp<lp );
	IfDebug( 
	    if( dp!=lp )
		printf(__FUNCTION__": ERROR\n");
	)
	dp+=da;
    }
}

/*
**	Draw rle compressed sprite with clipping.
*/
global void DrawRleSpriteClipped8(RleSprite* sprite,unsigned frame,int x,int y)
{
    int ox;
    int oy;
    int w;
    int h;
    const unsigned char* sp;
    unsigned sw;
    VMemType8* dp;
    VMemType8* lp;
    VMemType8* ep;
    VMemType8* pp;
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
    sp=sprite->Frames[frame];

    //
    // Skip top lines
    //
    while( oy-- ) {
	da=0;
	do {
	    da+=*sp++;			// transparent
	    if( da>=sw ) {
		break;
	    }
	    da+=*sp;			// non-transparent
	    sp+=*sp+1;
	} while( da<sw );
    }

    da=VideoWidth-sw;
    dp=VideoMemory8+x+y*VideoWidth;
    ep=dp+VideoWidth*h;

    if( w==sw ) {			// Unclipped horizontal

	while( dp<ep ) {		// all lines
	    lp=dp+sw;
	    do {			// 1 line
		dp+=*sp++;		// transparent
		if( dp>=lp ) {
		    break;
		}
		pp=dp-1+*sp++;		// non-transparent
		while( dp<pp ) {
		    *dp++=Pixels8[*sp++];
		    *dp++=Pixels8[*sp++];
		}
		if( dp<=pp ) {
		    *dp++=Pixels8[*sp++];
		}
	    } while( dp<lp );
	    IfDebug( 
		if( dp!=lp )
		    printf(__FUNCTION__": ERROR\n");
	    )
	    dp+=da;
	}

    } else {				// Clip horizontal
	//printf("CLIPPING ox %d w %d\n",ox,w);

	da+=ox;
	while( dp<ep ) {		// all lines
	    lp=dp+w;
	    //
	    //	Clip left
	    //
	    pp=dp-ox;
	    for( ;; ) {
		pp+=*sp++;		// transparent
		//printf("T%d-",sp[-1]);
		if( pp>=dp ) {
		    dp=pp;
		    //printf("C");
		    goto middle_trans;
		}
		pp+=*sp;		// non-transparent
		//printf("P%d-",sp[0]);
		if( pp>=dp ) {
		    sp+=*sp-(pp-dp)+1;
		    //printf("C");
		    goto middle_pixel;
		}
		sp+=*sp+1;
	    }

	    //
	    //	Draw middle
	    //
	    for( ;; ) {
		dp+=*sp++;		// transparent
		//printf("T%d-",sp[-1]);
middle_trans:
		if( dp>=lp ) {
		    lp+=sw-w-ox;
		    //printf("C");
		    goto right_trans;
		}
		pp=dp+*sp++;		// non-transparent
		//printf("P%d-",sp[-1]);
middle_pixel:
		//printf("%p, %p, %p\n",dp,pp,lp);
		if( pp<lp ) {
		    while( dp<pp ) {
			*dp++=Pixels8[*sp++];
		    }
		    continue;
		}
		while( dp<lp ) {
		    *dp++=Pixels8[*sp++];
		}
		sp+=pp-dp;
		dp=pp;
		//printf("C");
		break;
	    }

	    //
	    //	Clip right
	    //
	    lp+=sw-w-ox;
	    while( dp<lp ) {
		dp+=*sp++;		// transparent
right_trans:
		if( dp>=lp ) {
		    break;
		}
		dp+=*sp;		// non-transparent
		sp+=*sp+1;
	    }
	    //printf("\n");
	    IfDebug( 
		if( dp!=lp )
		    printf(__FUNCTION__": ERROR\n");
	    )
	    dp+=da;
	}
    }
}

/*
**	Draw rle compressed sprite, flipped in X.
*/
global void DrawRleSpriteX8(RleSprite* sprite,unsigned frame,int x,int y)
{
    const unsigned char* sp;
    unsigned w;
    VMemType8* dp;
    VMemType8* lp;
    VMemType8* ep;
    VMemType8* pp;
    unsigned da;

    sp=sprite->Frames[frame];
    w=sprite->Width;
    dp=VideoMemory8+x+y*VideoWidth+w;
    da=VideoWidth+w;
    ep=dp+VideoWidth*sprite->Height;

    while( dp<ep ) {			// all lines
	lp=dp-w;
	do {				// 1 line
	    dp-=*sp++;			// transparent
	    if( dp<=lp ) {
		break;
	    }
	    pp=dp+1-*sp++;		// non-transparent
	    while( dp>pp ) {
		*dp--=Pixels8[*sp++];
		*dp--=Pixels8[*sp++];
	    }
	    if( dp>=pp ) {
		*dp--=Pixels8[*sp++];
	    }
	} while( dp>lp );
	IfDebug( 
	    if( dp!=lp )
		printf(__FUNCTION__": ERROR\n");
	)
	dp+=da;
    }
}

/*
**	Draw rle compressed sprite with clipping, flipped in X.
*/
global void DrawRleSpriteClippedX8(RleSprite* sprite,unsigned frame,int x,int y)
{
    int ox;
    int oy;
    int w;
    int h;
    const unsigned char* sp;
    unsigned sw;
    VMemType8* dp;
    VMemType8* lp;
    VMemType8* ep;
    VMemType8* pp;
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
    sp=sprite->Frames[frame];

    //
    // Skip top lines
    //
    while( oy-- ) {
	da=0;
	do {
	    da+=*sp++;			// transparent
	    if( da>=sw ) {
		break;
	    }
	    da+=*sp;			// non-transparent
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
		if( dp<=lp ) {
		    break;
		}
		pp=dp+1-*sp++;		// non-transparent
		while( dp>pp ) {
		    *dp--=Pixels8[*sp++];
		    *dp--=Pixels8[*sp++];
		}
		if( dp>=pp ) {
		    *dp--=Pixels8[*sp++];
		}
	    } while( dp>lp );
	    IfDebug( 
		if( dp!=lp )
		    printf(__FUNCTION__": ERROR\n");
	    )
	    dp+=da;
	}

    } else {				// Clip horizontal
	//printf("CLIPPING %d %d\n",ox,w);

	da-=sw-w-ox;
	while( dp<ep ) {		// all lines
	    lp=dp-w;
	    //
	    //	Clip right side
	    //
	    pp=dp+sw-w-ox;
	    for( ;; ) {
		pp-=*sp++;		// transparent
		//printf("T%d ",sp[-1]);
		if( pp<=dp ) {
		    dp=pp;
		    goto middle_trans;
		}
		pp-=*sp;		// non-transparent
		//printf("P%d ",sp[0]);
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
		//printf("T%d ",sp[-1]);
middle_trans:
		if( dp<=lp ) {
		    //printf("CLIP TRANS\n");
		    lp-=ox;
		    goto right_trans;
		}
		pp=dp-*sp++;		// non-transparent
		//printf("P%d ",sp[-1]);
middle_pixel:
		if( pp>lp ) {
		    while( dp>pp ) {
			*dp--=Pixels8[*sp++];
		    }
		    continue;
		}
		//printf("%d ",sp[-1]);
		while( dp>lp ) {
		    *dp--=Pixels8[*sp++];
		}
		//printf("%d: ",dp-pp);
		sp+=dp-pp;
		//printf("CLIP PIXEL %d,%d,%d\n",*sp,sp[-1],sp[1]);
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
		if( dp<=lp ) {
		    break;
		}
		dp-=*sp;		// non-transparent
		sp+=*sp+1;
	    }
	    IfDebug( 
		if( dp!=lp )
		    printf(__FUNCTION__": ERROR\n");
	    )
	    dp+=da;
	}
    }
}

/*----------------------------------------------------------------------------
--	RLE Sprites 16bit
----------------------------------------------------------------------------*/

/*
**	Draw rle compressed sprite.
*/
global void DrawRleSprite16(RleSprite* sprite,unsigned frame,int x,int y)
{
    const unsigned char* sp;
    unsigned w;
    VMemType16* dp;
    VMemType16* lp;
    VMemType16* ep;
    VMemType16* pp;
    unsigned da;

    sp=sprite->Frames[frame];
    w=sprite->Width;
    da=VideoWidth-w;
    dp=VideoMemory16+x+y*VideoWidth;
    ep=dp+VideoWidth*sprite->Height;

    while( dp<ep ) {			// all lines
	lp=dp+w;
	do {				// 1 line
	    dp+=*sp++;			// transparent
	    if( dp>=lp ) {
		break;
	    }
	    pp=dp-1+*sp++;		// non-transparent
	    while( dp<pp ) {
		*dp++=Pixels16[*sp++];
		*dp++=Pixels16[*sp++];
	    }
	    if( dp<=pp ) {
		*dp++=Pixels16[*sp++];
	    }
	} while( dp<lp );
	IfDebug( 
	    if( dp!=lp )
		printf(__FUNCTION__": ERROR\n");
	)
	dp+=da;
    }
}

/*
**	Draw rle compressed sprite with clipping.
*/
global void DrawRleSpriteClipped16(RleSprite* sprite,unsigned frame,int x,int y)
{
    int ox;
    int oy;
    int w;
    int h;
    const unsigned char* sp;
    unsigned sw;
    VMemType16* dp;
    VMemType16* lp;
    VMemType16* ep;
    VMemType16* pp;
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
    sp=sprite->Frames[frame];

    //
    // Skip top lines
    //
    while( oy-- ) {
	da=0;
	do {
	    da+=*sp++;			// transparent
	    if( da>=sw ) {
		break;
	    }
	    da+=*sp;			// non-transparent
	    sp+=*sp+1;
	} while( da<sw );
    }

    da=VideoWidth-sw;
    dp=VideoMemory16+x+y*VideoWidth;
    ep=dp+VideoWidth*h;

    if( w==sw ) {			// Unclipped horizontal

	while( dp<ep ) {		// all lines
	    lp=dp+sw;
	    do {			// 1 line
		dp+=*sp++;		// transparent
		if( dp>=lp ) {
		    break;
		}
		pp=dp-1+*sp++;		// non-transparent
		while( dp<pp ) {
		    *dp++=Pixels16[*sp++];
		    *dp++=Pixels16[*sp++];
		}
		if( dp<=pp ) {
		    *dp++=Pixels16[*sp++];
		}
	    } while( dp<lp );
	    IfDebug( 
		if( dp!=lp )
		    printf(__FUNCTION__": ERROR\n");
	    )
	    dp+=da;
	}

    } else {				// Clip horizontal
	//printf("CLIPPING ox %d w %d\n",ox,w);

	da+=ox;
	while( dp<ep ) {		// all lines
	    lp=dp+w;
	    //
	    //	Clip left
	    //
	    pp=dp-ox;
	    for( ;; ) {
		pp+=*sp++;		// transparent
		//printf("T%d-",sp[-1]);
		if( pp>=dp ) {
		    dp=pp;
		    //printf("C");
		    goto middle_trans;
		}
		pp+=*sp;		// non-transparent
		//printf("P%d-",sp[0]);
		if( pp>=dp ) {
		    sp+=*sp-(pp-dp)+1;
		    //printf("C");
		    goto middle_pixel;
		}
		sp+=*sp+1;
	    }

	    //
	    //	Draw middle
	    //
	    for( ;; ) {
		dp+=*sp++;		// transparent
		//printf("T%d-",sp[-1]);
middle_trans:
		if( dp>=lp ) {
		    lp+=sw-w-ox;
		    //printf("C");
		    goto right_trans;
		}
		pp=dp+*sp++;		// non-transparent
		//printf("P%d-",sp[-1]);
middle_pixel:
		//printf("%p, %p, %p\n",dp,pp,lp);
		if( pp<lp ) {
		    while( dp<pp ) {
			*dp++=Pixels16[*sp++];
		    }
		    continue;
		}
		while( dp<lp ) {
		    *dp++=Pixels16[*sp++];
		}
		sp+=pp-dp;
		dp=pp;
		//printf("C");
		break;
	    }

	    //
	    //	Clip right
	    //
	    lp+=sw-w-ox;
	    while( dp<lp ) {
		dp+=*sp++;		// transparent
right_trans:
		if( dp>=lp ) {
		    break;
		}
		dp+=*sp;		// non-transparent
		sp+=*sp+1;
	    }
	    //printf("\n");
	    IfDebug( 
		if( dp!=lp )
		    printf(__FUNCTION__": ERROR\n");
	    )
	    dp+=da;
	}
    }
}

/*
**	Draw rle compressed sprite, flipped in X.
*/
global void DrawRleSpriteX16(RleSprite* sprite,unsigned frame,int x,int y)
{
    const unsigned char* sp;
    unsigned w;
    VMemType16* dp;
    VMemType16* lp;
    VMemType16* ep;
    VMemType16* pp;
    unsigned da;

    sp=sprite->Frames[frame];
    w=sprite->Width;
    dp=VideoMemory16+x+y*VideoWidth+w;
    da=VideoWidth+w;
    ep=dp+VideoWidth*sprite->Height;

    while( dp<ep ) {			// all lines
	lp=dp-w;
	do {				// 1 line
	    dp-=*sp++;			// transparent
	    if( dp<=lp ) {
		break;
	    }
	    pp=dp+1-*sp++;		// non-transparent
	    while( dp>pp ) {
		*dp--=Pixels16[*sp++];
		*dp--=Pixels16[*sp++];
	    }
	    if( dp>=pp ) {
		*dp--=Pixels16[*sp++];
	    }
	} while( dp>lp );
	IfDebug( 
	    if( dp!=lp )
		printf(__FUNCTION__": ERROR\n");
	)
	dp+=da;
    }
}

/*
**	Draw rle compressed sprite with clipping, flipped in X.
*/
global void DrawRleSpriteClippedX16(RleSprite* sprite,unsigned frame,int x,int y)
{
    int ox;
    int oy;
    int w;
    int h;
    const unsigned char* sp;
    unsigned sw;
    VMemType16* dp;
    VMemType16* lp;
    VMemType16* ep;
    VMemType16* pp;
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
    sp=sprite->Frames[frame];

    //
    // Skip top lines
    //
    while( oy-- ) {
	da=0;
	do {
	    da+=*sp++;			// transparent
	    if( da>=sw ) {
		break;
	    }
	    da+=*sp;			// non-transparent
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
		if( dp<=lp ) {
		    break;
		}
		pp=dp+1-*sp++;		// non-transparent
		while( dp>pp ) {
		    *dp--=Pixels16[*sp++];
		    *dp--=Pixels16[*sp++];
		}
		if( dp>=pp ) {
		    *dp--=Pixels16[*sp++];
		}
	    } while( dp>lp );
	    IfDebug( 
		if( dp!=lp )
		    printf(__FUNCTION__": ERROR\n");
	    )
	    dp+=da;
	}

    } else {				// Clip horizontal
	//printf("CLIPPING %d %d\n",ox,w);

	da-=sw-w-ox;
	while( dp<ep ) {		// all lines
	    lp=dp-w;
	    //
	    //	Clip right side
	    //
	    pp=dp+sw-w-ox;
	    for( ;; ) {
		pp-=*sp++;		// transparent
		//printf("T%d ",sp[-1]);
		if( pp<=dp ) {
		    dp=pp;
		    goto middle_trans;
		}
		pp-=*sp;		// non-transparent
		//printf("P%d ",sp[0]);
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
		//printf("T%d ",sp[-1]);
middle_trans:
		if( dp<=lp ) {
		    //printf("CLIP TRANS\n");
		    lp-=ox;
		    goto right_trans;
		}
		pp=dp-*sp++;		// non-transparent
		//printf("P%d ",sp[-1]);
middle_pixel:
		if( pp>lp ) {
		    while( dp>pp ) {
			*dp--=Pixels16[*sp++];
		    }
		    continue;
		}
		//printf("%d ",sp[-1]);
		while( dp>lp ) {
		    *dp--=Pixels16[*sp++];
		}
		//printf("%d: ",dp-pp);
		sp+=dp-pp;
		//printf("CLIP PIXEL %d,%d,%d\n",*sp,sp[-1],sp[1]);
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
		if( dp<=lp ) {
		    break;
		}
		dp-=*sp;		// non-transparent
		sp+=*sp+1;
	    }
	    IfDebug( 
		if( dp!=lp )
		    printf(__FUNCTION__": ERROR\n");
	    )
	    dp+=da;
	}
    }
}

/*----------------------------------------------------------------------------
--	RLE Sprites 32bit
----------------------------------------------------------------------------*/

/*
**	Draw rle compressed sprite.
*/
global void DrawRleSprite32(RleSprite* sprite,unsigned frame,int x,int y)
{
    const unsigned char* sp;
    unsigned w;
    VMemType32* dp;
    VMemType32* lp;
    VMemType32* ep;
    VMemType32* pp;
    unsigned da;

    sp=sprite->Frames[frame];
    w=sprite->Width;
    da=VideoWidth-w;
    dp=VideoMemory32+x+y*VideoWidth;
    ep=dp+VideoWidth*sprite->Height;

    while( dp<ep ) {			// all lines
	lp=dp+w;
	do {				// 1 line
	    dp+=*sp++;			// transparent
	    if( dp>=lp ) {
		break;
	    }
	    pp=dp-1+*sp++;		// non-transparent
	    while( dp<pp ) {
		*dp++=Pixels32[*sp++];
		*dp++=Pixels32[*sp++];
	    }
	    if( dp<=pp ) {
		*dp++=Pixels32[*sp++];
	    }
	} while( dp<lp );
	IfDebug( 
	    if( dp!=lp )
		printf(__FUNCTION__": ERROR\n");
	)
	dp+=da;
    }
}

/*
**	Draw rle compressed sprite with clipping.
*/
global void DrawRleSpriteClipped32(RleSprite* sprite,unsigned frame,int x,int y)
{
    int ox;
    int oy;
    int w;
    int h;
    const unsigned char* sp;
    unsigned sw;
    VMemType32* dp;
    VMemType32* lp;
    VMemType32* ep;
    VMemType32* pp;
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
    sp=sprite->Frames[frame];

    //
    // Skip top lines
    //
    while( oy-- ) {
	da=0;
	do {
	    da+=*sp++;			// transparent
	    if( da>=sw ) {
		break;
	    }
	    da+=*sp;			// non-transparent
	    sp+=*sp+1;
	} while( da<sw );
    }

    da=VideoWidth-sw;
    dp=VideoMemory32+x+y*VideoWidth;
    ep=dp+VideoWidth*h;

    if( w==sw ) {			// Unclipped horizontal

	while( dp<ep ) {		// all lines
	    lp=dp+sw;
	    do {			// 1 line
		dp+=*sp++;		// transparent
		if( dp>=lp ) {
		    break;
		}
		pp=dp-1+*sp++;		// non-transparent
		while( dp<pp ) {
		    *dp++=Pixels32[*sp++];
		    *dp++=Pixels32[*sp++];
		}
		if( dp<=pp ) {
		    *dp++=Pixels32[*sp++];
		}
	    } while( dp<lp );
	    IfDebug( 
		if( dp!=lp )
		    printf(__FUNCTION__": ERROR\n");
	    )
	    dp+=da;
	}

    } else {				// Clip horizontal
	//printf("CLIPPING ox %d w %d\n",ox,w);

	da+=ox;
	while( dp<ep ) {		// all lines
	    lp=dp+w;
	    //
	    //	Clip left
	    //
	    pp=dp-ox;
	    for( ;; ) {
		pp+=*sp++;		// transparent
		//printf("T%d-",sp[-1]);
		if( pp>=dp ) {
		    dp=pp;
		    //printf("C");
		    goto middle_trans;
		}
		pp+=*sp;		// non-transparent
		//printf("P%d-",sp[0]);
		if( pp>=dp ) {
		    sp+=*sp-(pp-dp)+1;
		    //printf("C");
		    goto middle_pixel;
		}
		sp+=*sp+1;
	    }

	    //
	    //	Draw middle
	    //
	    for( ;; ) {
		dp+=*sp++;		// transparent
		//printf("T%d-",sp[-1]);
middle_trans:
		if( dp>=lp ) {
		    lp+=sw-w-ox;
		    //printf("C");
		    goto right_trans;
		}
		pp=dp+*sp++;		// non-transparent
		//printf("P%d-",sp[-1]);
middle_pixel:
		//printf("%p, %p, %p\n",dp,pp,lp);
		if( pp<lp ) {
		    while( dp<pp ) {
			*dp++=Pixels32[*sp++];
		    }
		    continue;
		}
		while( dp<lp ) {
		    *dp++=Pixels32[*sp++];
		}
		sp+=pp-dp;
		dp=pp;
		//printf("C");
		break;
	    }

	    //
	    //	Clip right
	    //
	    lp+=sw-w-ox;
	    while( dp<lp ) {
		dp+=*sp++;		// transparent
right_trans:
		if( dp>=lp ) {
		    break;
		}
		dp+=*sp;		// non-transparent
		sp+=*sp+1;
	    }
	    //printf("\n");
	    IfDebug( 
		if( dp!=lp )
		    printf(__FUNCTION__": ERROR\n");
	    )
	    dp+=da;
	}
    }
}

/*
**	Draw rle compressed sprite, flipped in X.
*/
global void DrawRleSpriteX32(RleSprite* sprite,unsigned frame,int x,int y)
{
    const unsigned char* sp;
    unsigned w;
    VMemType32* dp;
    VMemType32* lp;
    VMemType32* ep;
    VMemType32* pp;
    unsigned da;

    sp=sprite->Frames[frame];
    w=sprite->Width;
    dp=VideoMemory32+x+y*VideoWidth+w;
    da=VideoWidth+w;
    ep=dp+VideoWidth*sprite->Height;

    while( dp<ep ) {			// all lines
	lp=dp-w;
	do {				// 1 line
	    dp-=*sp++;			// transparent
	    if( dp<=lp ) {
		break;
	    }
	    pp=dp+1-*sp++;		// non-transparent
	    while( dp>pp ) {
		*dp--=Pixels32[*sp++];
		*dp--=Pixels32[*sp++];
	    }
	    if( dp>=pp ) {
		*dp--=Pixels32[*sp++];
	    }
	} while( dp>lp );
	IfDebug( 
	    if( dp!=lp )
		printf(__FUNCTION__": ERROR\n");
	)
	dp+=da;
    }
}

/*
**	Draw rle compressed sprite with clipping, flipped in X.
*/
global void DrawRleSpriteClippedX32(RleSprite* sprite,unsigned frame,int x,int y)
{
    int ox;
    int oy;
    int w;
    int h;
    const unsigned char* sp;
    unsigned sw;
    VMemType32* dp;
    VMemType32* lp;
    VMemType32* ep;
    VMemType32* pp;
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
    sp=sprite->Frames[frame];

    //
    // Skip top lines
    //
    while( oy-- ) {
	da=0;
	do {
	    da+=*sp++;			// transparent
	    if( da>=sw ) {
		break;
	    }
	    da+=*sp;			// non-transparent
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
		if( dp<=lp ) {
		    break;
		}
		pp=dp+1-*sp++;		// non-transparent
		while( dp>pp ) {
		    *dp--=Pixels32[*sp++];
		    *dp--=Pixels32[*sp++];
		}
		if( dp>=pp ) {
		    *dp--=Pixels32[*sp++];
		}
	    } while( dp>lp );
	    IfDebug( 
		if( dp!=lp )
		    printf(__FUNCTION__": ERROR\n");
	    )
	    dp+=da;
	}

    } else {				// Clip horizontal
	//printf("CLIPPING %d %d\n",ox,w);

	da-=sw-w-ox;
	while( dp<ep ) {		// all lines
	    lp=dp-w;
	    //
	    //	Clip right side
	    //
	    pp=dp+sw-w-ox;
	    for( ;; ) {
		pp-=*sp++;		// transparent
		//printf("T%d ",sp[-1]);
		if( pp<=dp ) {
		    dp=pp;
		    goto middle_trans;
		}
		pp-=*sp;		// non-transparent
		//printf("P%d ",sp[0]);
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
		//printf("T%d ",sp[-1]);
middle_trans:
		if( dp<=lp ) {
		    //printf("CLIP TRANS\n");
		    lp-=ox;
		    goto right_trans;
		}
		pp=dp-*sp++;		// non-transparent
		//printf("P%d ",sp[-1]);
middle_pixel:
		if( pp>lp ) {
		    while( dp>pp ) {
			*dp--=Pixels32[*sp++];
		    }
		    continue;
		}
		//printf("%d ",sp[-1]);
		while( dp>lp ) {
		    *dp--=Pixels32[*sp++];
		}
		//printf("%d: ",dp-pp);
		sp+=dp-pp;
		//printf("CLIP PIXEL %d,%d,%d\n",*sp,sp[-1],sp[1]);
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
		if( dp<=lp ) {
		    break;
		}
		dp-=*sp;		// non-transparent
		sp+=*sp+1;
	    }
	    IfDebug( 
		if( dp!=lp )
		    printf(__FUNCTION__": ERROR\n");
	    )
	    dp+=da;
	}
    }
}

global void DrawRleSprite(RleSprite* sprite,unsigned frame,int x,int y)
{
    // FIXME: function pointer and move to new code structure
    switch( VideoDepth ) {
	case 8:
	    DrawRleSprite8(sprite,frame,x,y);
	    break;
	case 15:
	case 16:
	    DrawRleSprite16(sprite,frame,x,y);
	    break;
	case 24:
	case 32:
	    DrawRleSprite32(sprite,frame,x,y);
	    break;
    }
}

global void DrawRleSpriteClipped(RleSprite* sprite,unsigned frame,int x,int y)
{
    // FIXME: function pointer and move to new code structure
    switch( VideoDepth ) {
	case 8:
	    DrawRleSpriteClipped8(sprite,frame,x,y);
	    break;
	case 15:
	case 16:
	    DrawRleSpriteClipped16(sprite,frame,x,y);
	    break;
	case 24:
	case 32:
	    DrawRleSpriteClipped32(sprite,frame,x,y);
	    break;
    }
}

global void DrawRleSpriteX(RleSprite* sprite,unsigned frame,int x,int y)
{
    // FIXME: function pointer and move to new code structure
    switch( VideoDepth ) {
	case 8:
	    DrawRleSpriteX8(sprite,frame,x,y);
	    break;
	case 15:
	case 16:
	    DrawRleSpriteX16(sprite,frame,x,y);
	    break;
	case 24:
	case 32:
	    DrawRleSpriteX32(sprite,frame,x,y);
	    break;
    }
}

global void DrawRleSpriteClippedX(RleSprite* sprite,unsigned frame,int x,int y)
{
    // FIXME: function pointer and move to new code structure
    switch( VideoDepth ) {
	case 8:
	    DrawRleSpriteClippedX8(sprite,frame,x,y);
	    break;
	case 15:
	case 16:
	    DrawRleSpriteClippedX16(sprite,frame,x,y);
	    break;
	case 24:
	case 32:
	    DrawRleSpriteClippedX32(sprite,frame,x,y);
	    break;
    }
}

/*
**	Load rle sprite from file.
*/
global RleSprite* LoadRleSprite(const char* name,unsigned width,unsigned height)
{
    RleSprite* sprite;
    Graphic* graphic;
    unsigned char* data;
    unsigned char* sp;
    unsigned char* dp;
    unsigned char* cp;
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

    n=(graphic->Width/width)*(graphic->Height/height);
    DebugLevel3(__FUNCTION__": %dx%d in %dx%d = %d frames.\n"
	    ,width,height
	    ,graphic->Width,graphic->Height,n);

    // FIXME: new internal compressed sprite format!
    sprite=malloc(sizeof(RleSprite)+n*sizeof(unsigned char*)+(graphic->Width*graphic->Height*2));
    // FIXME: ARI: * 2 == very passive!
    data=(unsigned char *)sprite;
    dp=(unsigned char *)&sprite->Frames[n];

    //
    //	Compress all frames of the sprite.
    //
    fl=graphic->Width/width;
    for( i=0; i<n; ++i ) {
	sprite->Frames[i]=dp;
	for( h=0; h<height; ++h ) {
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
#if 1
		for( ; w<width; ++w ) {	// non-transparent
		    *dp++=*sp++;
		    if( ++counter==256 ) {
			*cp=255;
			*dp++=0;
			cp=dp++;
			counter=1;
		    }
		    if( w+1!=width && *sp==255 ) {	// transparent
			break;
		    }
		}
#else
		for( ; w<width && *sp!=255 ; ++w ) {	// non-transparent
		    *dp++=*sp++;
		    if( ++counter==256 ) {
			*cp=255;
			*dp++=0;
			cp=dp++;
			counter=1;
		    }
		}
#endif
		*cp=counter;
		counter=0;
	    }
	    if( counter ) {
		*dp++=counter;
	    }
	}
    }

    DebugLevel3("\t%d => %d RLE compressed\n"
	    ,graphic->Width*graphic->Height,dp-data);

    i=sizeof(*sprite)+n*sizeof(unsigned char*)+dp-data;
    sprite=realloc(sprite,i);
    if( (unsigned char*)sprite!=data ) {	// shrink only - happens rarely
	for( h=0; h<n; ++h ) {			// convert address
	    sprite->Frames[h]=sprite->Frames[h]-data+(unsigned char*)sprite;
	}
    }

    IfDebug(CompressedGraphicMemory+=i;
	     sprite->ByteSize=i; )

    sprite->Width=width;
    sprite->Height=height;
    sprite->Pixels=NULL;		// FIXME: future extensions
    sprite->NumFrames=n;

    VideoFree(graphic);

    return sprite;
}

/*
**	Free rle sprite.
*/
global void FreeRleSprite(RleSprite* sprite)
{
    IfDebug (CompressedGraphicMemory-=sprite->ByteSize; )

    free(sprite);
}

/**
**	Load a RGB palette.
**
**	FIXME: Should or shouldn't this be removed???
*/
global void LoadRGB(Palette *pal, const char *name)
{
    FILE *fp;
    int i;
    
    if((fp=fopen(name,"rb")) == NULL) {
	printf("Can't load palette %s\n", name);
	exit(-1);
    }

    for(i=0;i<256;i++){
	pal[i].r=fgetc(fp);
	pal[i].g=fgetc(fp);
	pal[i].b=fgetc(fp);
    }
    
    fclose(fp);
}

//@}
