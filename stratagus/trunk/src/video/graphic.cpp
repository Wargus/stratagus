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
/**@name graphic.c	-	The general graphic functions. */
/*
**	(c) Copyright 1999,2000 by Lutz Sammer
**
**	$Id$
*/

//@{

/*----------------------------------------------------------------------------
--	Includes
----------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "freecraft.h"
#include "video.h"
#include "iolib.h"

/*----------------------------------------------------------------------------
--	Declarations
----------------------------------------------------------------------------*/

/**
**	Typedef of lnode.
*/
typedef struct __lnode__ PaletteLink;

/**
**	sturct __lnode__, links all palettes together to join the same palettes.
*/
struct __lnode__ {
    PaletteLink*	Next;		/// Next palette
    VMemType*		Palette;	/// Palette in hardware format
    long		Checksum;	/// Checksum for quick lookup
};

local PaletteLink *palette_list;	/// List of all used palettes.

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

local GraphicType GraphicImage8Type;	/// image type 8bit palette
local GraphicType GraphicImage16Type;	/// image type 16bit palette

/*----------------------------------------------------------------------------
--	Local functions
----------------------------------------------------------------------------*/

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
local void VideoDrawSub8to8(
	const Graphic* graphic,int gx,int gy,unsigned w,unsigned h,
	int x,int y)
{
    const unsigned char* sp;
    const unsigned char* lp;
    const unsigned char* gp;
    VMemType8* dp;
    const VMemType8* pixels;
    int sa;
    int da;

    pixels=graphic->Pixels;
    sp=graphic->Frames+gx+gy*graphic->Width;
    gp=sp+graphic->Width*h;
    sa=graphic->Width-w;
    dp=VideoMemory8+x+y*VideoWidth;
    da=VideoWidth-w--;

    while( sp<gp ) {
	lp=sp+w;
	while( sp<lp ) {
	    *dp++=pixels[*sp++];	// unroll
	    *dp++=pixels[*sp++];
	}
	if( sp<=lp ) {
	    *dp++=pixels[*sp++];
	}
	sp+=sa;
	dp+=da;
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
local void VideoDrawSub8to16(
	const Graphic* graphic,int gx,int gy,unsigned w,unsigned h,
	int x,int y)
{
    const unsigned char* sp;
    const unsigned char* lp;
    const unsigned char* gp;
    int sa;
    const VMemType16* pixels;
    VMemType16* dp;
    int da;

    pixels=(VMemType16*)graphic->Pixels;
    sp=graphic->Frames+gx+gy*graphic->Width;
    gp=sp+graphic->Width*h;
    sa=graphic->Width-w;
    dp=VideoMemory16+x+y*VideoWidth;
    da=VideoWidth-w--;

    while( sp<gp ) {
	lp=sp+w;
	while( sp<lp ) {
	    *dp++=pixels[*sp++];	// unroll
	    *dp++=pixels[*sp++];
	}
	if( sp<=lp ) {
	    *dp++=pixels[*sp++];
	}
	sp+=sa;
	dp+=da;
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
local void VideoDrawSub8to24(
	const Graphic* graphic,int gx,int gy,unsigned w,unsigned h,
	int x,int y)
{
    const unsigned char* sp;
    const unsigned char* lp;
    const unsigned char* gp;
    int sa;
    const VMemType24* pixels;
    VMemType24* dp;
    int da;

    pixels=(VMemType24*)graphic->Pixels;
    sp=graphic->Frames+gx+gy*graphic->Width;
    gp=sp+graphic->Width*h;
    sa=graphic->Width-w;
    dp=VideoMemory24+x+y*VideoWidth;
    da=VideoWidth-w--;

    while( sp<gp ) {
	lp=sp+w;
	while( sp<lp ) {
	    *dp++=pixels[*sp++];	// unroll
	    *dp++=pixels[*sp++];
	}
	if( sp<=lp ) {
	    *dp++=pixels[*sp++];
	}
	sp+=sa;
	dp+=da;
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
local void VideoDrawSub8to32(
	const Graphic* graphic,int gx,int gy,unsigned w,unsigned h,
	int x,int y)
{
    const unsigned char* sp;
    const unsigned char* lp;
    const unsigned char* gp;
    int sa;
    const VMemType32* pixels;
    VMemType32* dp;
    int da;

    pixels=(VMemType32*)graphic->Pixels;
    sp=graphic->Frames+gx+gy*graphic->Width;
    gp=sp+graphic->Width*h;
    sa=graphic->Width-w;
    dp=VideoMemory32+x+y*VideoWidth;
    da=VideoWidth-w--;

    while( sp<gp ) {
	lp=sp+w;
	while( sp<lp ) {
	    *dp++=pixels[*sp++];	// unroll
	    *dp++=pixels[*sp++];
	}
	if( sp<=lp ) {
	    *dp++=pixels[*sp++];
	}
	sp+=sa;
	dp+=da;
    }
}

/**
**	Clip to clipping rectangle.
**
**	@param w	width to display
**	@param h	height to display
**	@param x	X screen position
**	@param y	Y screen position
*/
#define DO_CLIPPING(w,h,x,y) \
    do {						\
	int f;						\
							\
	if( x<ClipX1 ) {				\
	    f=ClipX1-x;					\
	    x=ClipX1;					\
	    if( w<f ) {			/* outside */	\
		return;					\
	    }						\
	    w-=f;					\
	}						\
	if( (x+w)>ClipX2 ) {				\
	    if( w<ClipX2-x ) {		/* outside */	\
		return;					\
	    }						\
	    w=ClipX2-x;					\
	}						\
							\
	if( y<ClipY1 ) {				\
	    f=ClipY1-y;					\
	    y=ClipY1;					\
	    if( h<f ) {			/* outside */	\
		return;					\
	    }						\
	    h-=f;					\
	}						\
	if( (y+h)>ClipY2 ) {				\
	    if( h<ClipY2-y ) {		/* outside */	\
		return;					\
	    }						\
	    h=ClipY2-y;					\
	}						\
    } while( 0 )

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
local void VideoDrawSub8to8Clip(
	const Graphic* graphic,int gx,int gy,unsigned w,unsigned h,
	int x,int y)
{
    DO_CLIPPING(w,h,x,y);
    VideoDrawSub8to8(graphic,gx,gy,w,h,x,y);
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
local void VideoDrawSub8to16Clip(
	const Graphic* graphic,int gx,int gy,unsigned w,unsigned h,
	int x,int y)
{
    DO_CLIPPING(w,h,x,y);
    VideoDrawSub8to16(graphic,gx,gy,w,h,x,y);
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
local void VideoDrawSub8to24Clip(
	const Graphic* graphic,int gx,int gy,unsigned w,unsigned h,
	int x,int y)
{
    DO_CLIPPING(w,h,x,y);
    VideoDrawSub8to24(graphic,gx,gy,w,h,x,y);
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
local void VideoDrawSub8to32Clip(
	const Graphic* graphic,int gx,int gy,unsigned w,unsigned h,
	int x,int y)
{
    DO_CLIPPING(w,h,x,y);
    VideoDrawSub8to32(graphic,gx,gy,w,h,x,y);
}

/**
**	Free graphic object.
*/
local void FreeGraphic8(Graphic* graphic)
{
    IfDebug( AllocatedGraphicMemory-=graphic->Size );
    IfDebug( AllocatedGraphicMemory-=sizeof(Graphic) );
    free(graphic->Frames);
    free(graphic);
}

// FIXME: need frame version

// FIXME: need 16 bit palette version

// FIXME: need zooming version

/*----------------------------------------------------------------------------
--	Global functions
----------------------------------------------------------------------------*/

/**
**	Make a graphic object.
**
**	@param depth	Pixel depth of the object (8,16,32)
**	@param width	Pixel width.
**	@param height	Pixel height.
**	@param data	Object data.
*/
global Graphic* MakeGraphic(
	unsigned depth,unsigned width,unsigned height,void* data,unsigned size)
{
    Graphic* graphic;

    graphic=malloc(sizeof(Graphic));
    IfDebug( AllocatedGraphicMemory+=sizeof(Graphic) );

    if( !graphic ) {
	fprintf(stderr,"Out of memory\n");
	exit(-1);
    }
    if( depth==8 ) {
	graphic->Type=&GraphicImage8Type;
    } else if( depth==16 ) {
	graphic->Type=&GraphicImage16Type;
    } else {
	fprintf(stderr,"Unsported image depth\n");
	exit(-1);
    }
    graphic->Width=width;
    graphic->Height=height;

    graphic->Pixels=NULL;
    graphic->Palette=NULL;

    graphic->NumFrames=0;
    graphic->Frames=data;
    graphic->Size=size;

    return graphic;
}

/**
**	Make a new graphic object.
**
**	@param depth	Pixel depth of the object (8,16,32)
**	@param width	Pixel width.
**	@param height	Pixel height.
*/
global Graphic* NewGraphic(
	unsigned depth,unsigned width,unsigned height)
{
    void* data;
    int size;

    size=width*height*(depth+7)/8;
    data=malloc(size);
    IfDebug( AllocatedGraphicMemory+=size );

    return MakeGraphic(depth,width,height,data,size);
}

/**
**	creates a checksum used to compare palettes.
**	JOSH: change the method if you have better ideas.
**	JOHNS: I say this also always :)
**
**	@param palette	Palette source.
**
**	@return		Calculated hash/checksum.
*/
local long GetPaletteChecksum(const Palette* palette)
{
    long retVal;
    int i;

    for(retVal = i = 0; i < 256; i++){
	//This is designed to return different values if
	// the pixels are in a different order.
	retVal = ((palette[i].r+i)&0xff)+retVal;
	retVal = ((palette[i].g+i)&0xff)+retVal;
	retVal = ((palette[i].b+i)&0xff)+retVal;
    }
    return retVal;
}

/**
**	Load graphic from file.
**
**	@param name	File name.
**
**	@return		Graphic object.
*/
global Graphic* LoadGraphic(const char* name)
{
    PaletteLink * current_link = palette_list, * prev_link = NULL;
    VMemType * pixels;
    Graphic* graphic;
    long checksum;
    char buf[1024];

    // FIXME: I want also support JPG file format!

    if( !(graphic=LoadGraphicPNG(LibraryFileName(name,buf))) ) {
	fprintf(stderr,"Can't load the graphic `%s'\n",name);
	exit(-1);
    }

    checksum = GetPaletteChecksum(graphic->Palette);
    while(current_link != NULL){
      if(current_link->Checksum == checksum)
	break;
      prev_link = current_link;
      current_link = current_link->Next;
    }
    //Palette Not found
    if(current_link == NULL){
      pixels = VideoCreateNewPalette(graphic->Palette);

      DebugLevel0("loading new palette with %s\n",name);
      if(prev_link == NULL){
	palette_list = (PaletteLink *)malloc(sizeof(PaletteLink));
	palette_list->Checksum = checksum;
	palette_list->Next = NULL;
	palette_list->Palette = pixels;
      } else {
	prev_link->Next = (PaletteLink *)malloc(sizeof(PaletteLink));
	prev_link->Next->Checksum = checksum;
	prev_link->Next->Next = NULL;
	prev_link->Next->Palette = pixels;
      }
    } else {
      pixels = current_link->Palette;
    }
    graphic->Pixels = pixels;
    free(graphic->Palette);
    graphic->Palette=NULL;		// JOHNS: why should we free this?

    return graphic;
}

/**
**	Init graphic
*/
global void InitGraphic(void)
{
    switch( VideoDepth ) {
	case 8:
	    GraphicImage8Type.DrawSub=VideoDrawSub8to8;
	    GraphicImage8Type.DrawSubClip=VideoDrawSub8to8Clip;
	    break;

	case 15:
	case 16:
	    GraphicImage8Type.DrawSub=VideoDrawSub8to16;
	    GraphicImage8Type.DrawSubClip=VideoDrawSub8to16Clip;
	    break;

	case 24:
	    GraphicImage8Type.DrawSub=VideoDrawSub8to24;
	    GraphicImage8Type.DrawSubClip=VideoDrawSub8to24Clip;
	    break;

	case 32:
	    GraphicImage8Type.DrawSub=VideoDrawSub8to32;
	    GraphicImage8Type.DrawSubClip=VideoDrawSub8to32Clip;
	    break;

	default:
	    DebugLevel0(__FUNCTION__": unsupported %d bpp\n",VideoDepth);
	    abort();
    }

    GraphicImage8Type.Free=FreeGraphic8;
}

//@}
