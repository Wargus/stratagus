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

/*----------------------------------------------------------------------------
--	Declarations
----------------------------------------------------------------------------*/

struct lnode *PalettePointer;

struct lnode {
  struct lnode * next;
  GraphicData * Palette;
  long checksum;
};

typedef struct lnode PaletteLink;

PaletteLink * palette_list = NULL;


/*----------------------------------------------------------------------------
--	Externals
----------------------------------------------------------------------------*/

extern int ClipX1;			/// current clipping top left
extern int ClipY1;			/// current clipping top left
extern int ClipX2;			/// current clipping bottom right
extern int ClipY2;			/// current clipping bottom right

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
	Graphic* graphic,int gx,int gy,unsigned w,unsigned h,
	int x,int y)
{
    unsigned char* sp;
    unsigned char* lp;
    unsigned char* gp;
    int sa;
    VMemType8* dp;
    int da;

    sp=graphic->Frames+gx+gy*graphic->Width;
    gp=sp+graphic->Width*h;
    sa=graphic->Width-w;
    dp=VideoMemory8+x+y*VideoWidth;
    da=VideoWidth-w;
    --w;

    while( sp<gp ) {
	lp=sp+w;
	while( sp<lp ) {
	    *dp++=Pixels8[*sp++];	// unroll
	    *dp++=Pixels8[*sp++];
	}
	if( sp<=lp ) {
	    *dp++=Pixels8[*sp++];
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
	Graphic* graphic,int gx,int gy,unsigned w,unsigned h,
	int x,int y)
{
    unsigned char* sp;
    unsigned char* lp;
    unsigned char* gp;
    int sa;
    VMemType16* dp;
    int da;

    sp=graphic->Frames+gx+gy*graphic->Width;
    gp=sp+graphic->Width*h;
    sa=graphic->Width-w;
    dp=VideoMemory16+x+y*VideoWidth;
    da=VideoWidth-w;
    --w;

    while( sp<gp ) {
	lp=sp+w;
	while( sp<lp ) {
	    *dp++=Pixels16[*sp++];	// unroll
	    *dp++=Pixels16[*sp++];
	}
	if( sp<=lp ) {
	    *dp++=Pixels16[*sp++];
	}
	sp+=sa;
	dp+=da;
    }
}

/**
**	Free graphic object.
*/
local void FreeGraphic8(Graphic* graphic)
{
    free(graphic->Frames);
    free(graphic);
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
	Graphic* graphic,int gx,int gy,unsigned w,unsigned h,
	int x,int y)
{
    unsigned char* sp;
    unsigned char* lp;
    unsigned char* gp;
    int sa;
    VMemType32* dp;
    int da;

    sp=graphic->Frames+gx+gy*graphic->Width;
    gp=sp+graphic->Width*h;
    sa=graphic->Width-w;
    dp=VideoMemory32+x+y*VideoWidth;
    da=VideoWidth-w;
    --w;

    while( sp<gp ) {
	lp=sp+w;
	while( sp<lp ) {
	    *dp++=Pixels32[*sp++];	// unroll
	    *dp++=Pixels32[*sp++];
	}
	if( sp<=lp ) {
	    *dp++=Pixels32[*sp++];
	}
	sp+=sa;
	dp+=da;
    }
}

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
	Graphic* graphic,int gx,int gy,unsigned w,unsigned h,
	int x,int y)
{
    int f;

    if( x<ClipX1 ) {
	f=ClipX1-x;
	x=ClipX1;
	if( w<f ) {			// outside
	    return;
	}
	w-=f;
    }
    if( (x+w)>ClipX2 ) {
	if( w<ClipX2-x ) {		// outside
	    return;
	}
	w=ClipX2-x;
    }

    if( y<ClipY1 ) {
	f=ClipY1-y;
	y=ClipY1;
	if( h<f ) {			// outside
	    return;
	}
	h-=f;
    }
    if( (y+h)>ClipY2 ) {
	if( h<ClipY2-y ) {		// outside
	    return;
	}
	h=ClipY2-y;
    }

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
	Graphic* graphic,int gx,int gy,unsigned w,unsigned h,
	int x,int y)
{
    int f;

    if( x<ClipX1 ) {
	f=ClipX1-x;
	x=ClipX1;
	if( w<f ) {			// outside
	    return;
	}
	w-=f;
    }
    if( (x+w)>ClipX2 ) {
	if( w<ClipX2-x ) {		// outside
	    return;
	}
	w=ClipX2-x;
    }

    if( y<ClipY1 ) {
	f=ClipY1-y;
	y=ClipY1;
	if( h<f ) {			// outside
	    return;
	}
	h-=f;
    }
    if( (y+h)>ClipY2 ) {
	if( h<ClipY2-y ) {		// outside
	    return;
	}
	h=ClipY2-y;
    }

    VideoDrawSub8to16(graphic,gx,gy,w,h,x,y);
}

/* FIXME: It would be nice to combine all the clips somehow, there is much
   duplicated code. */
local void VideoDrawSub8to32Clip(
	Graphic* graphic,int gx,int gy,unsigned w,unsigned h,
	int x,int y)
{
    int f;

    if( x<ClipX1 ) {
	f=ClipX1-x;
	x=ClipX1;
	if( w<f ) {			// outside
	    return;
	}
	w-=f;
    }
    if( (x+w)>ClipX2 ) {
	if( w<ClipX2-x ) {		// outside
	    return;
	}
	w=ClipX2-x;
    }

    if( y<ClipY1 ) {
	f=ClipY1-y;
	y=ClipY1;
	if( h<f ) {			// outside
	    return;
	}
	h-=f;
    }
    if( (y+h)>ClipY2 ) {
	if( h<ClipY2-y ) {		// outside
	    return;
	}
	h=ClipY2-y;
    }

    VideoDrawSub8to32(graphic,gx,gy,w,h,x,y);
}

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
	unsigned depth,unsigned width,unsigned height,void* data)
{
    Graphic* graphic;

    graphic=malloc(sizeof(Graphic));
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

    graphic->NumFrames=0;
    graphic->Frames=data;

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

    data=malloc(width*height*(depth+7)/8);

    return MakeGraphic(depth,width,height,data);
}

/**
**	Generate a filename into library.
**
**	Try current directory, user home directory, global directory.
**
**	FIXME: I want also support files stored into tar/zip archives.
**
**	@param file	Filename to open.
**	@param buffer	Allocated buffer for generated filename.
**
**	@return		Pointer to buffer.
*/
global char* LibraryFileName(const char* file,char* buffer)
{
    //
    //	Absolute path or in current directory.
    //
    strcpy(buffer,file);
    if( *buffer=='/' || !access(buffer,R_OK) ) {
	return buffer;
    }
#ifdef USE_ZLIB		// gzip or bzip2 in current directory
    sprintf(buffer,"%s.gz",file);
    if( !access(buffer,R_OK) ) {
	return buffer;
    }
#endif
#ifdef USE_BZ2LIB
    sprintf(buffer,"%s.bz2",file);
    if( !access(buffer,R_OK) ) {
	return buffer;
    }
#endif

    //
    //	In user home directory
    //
    sprintf(buffer,"%s/%s/%s",getenv("HOME"),FREECRAFT_HOME_PATH,file);
    if( !access(buffer,R_OK) ) {
	return buffer;
    }
#ifdef USE_ZLIB		// gzip or bzip2 in user home directory
    strcat(buffer,".gz");
    if( !access(buffer,R_OK) ) {
	return buffer;
    }
#endif
#ifdef USE_BZ2LIB
#ifndef USE_ZLIB
    strcat(buffer,".bz2");
#else
    sprintf(buffer,"%s/%s/%s.bz2",getenv("HOME"),FREECRAFT_HOME_PATH,file);
#endif
    if( !access(buffer,R_OK) ) {
	return buffer;
    }
#endif

    //
    //	In global shared directory
    //
    sprintf(buffer,"%s/%s",FreeCraftLibPath,file);
    if( !access(buffer,R_OK) ) {
	return buffer;
    }
#ifdef USE_ZLIB		// gzip or bzip2 in global shared directory
    strcat(buffer,".gz");
    if( !access(buffer,R_OK) ) {
	return buffer;
    }
#endif
#ifdef USE_BZ2LIB
#ifndef USE_ZLIB
    strcat(buffer,".bz2");
#else
    sprintf(buffer,"%s/%s.bz2",FreeCraftLibPath,file);
#endif
    if( !access(buffer,R_OK) ) {
	return buffer;
    }
#endif
    DebugLevel0(__FUNCTION__": File `%s' not found\n",file);

    strcpy(buffer,file);
    return buffer;
}

/*
**  creates a checksum used to compare palettes.
**  change the method if you have better ideas.
*/
local long GetPaletteChecksum(Palette * palette){
  long retVal = 0;
  int i;
  for(i = 0; i < 256; i++){
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
    GraphicData * Pixels;
    Graphic* graphic;
    long checksum;
    char buf[1024];

    // FIXME: I want also support JPG file format!

    if( !(graphic=LoadGraphicPNG(LibraryFileName(name,buf))) ) {
	printf("Can't load the graphic `%s'\n",name);
	exit(-1);
    }

    checksum = GetPaletteChecksum(graphic->Palette);
    while(current_link != NULL){
      if(current_link->checksum == checksum)
	break;
      prev_link = current_link;
      current_link = current_link->next;
    }
    //Palette Not found
    if(current_link == NULL){
      Pixels = VideoCreateNewPalette(graphic->Palette);
      printf("loading new palette with %s\n",name);
      if(prev_link == NULL){
	palette_list = (PaletteLink *)malloc(sizeof(PaletteLink));
	palette_list->checksum = checksum;
	palette_list->next = NULL;
	palette_list->Palette = Pixels;
      } else {
	prev_link->next = (PaletteLink *)malloc(sizeof(PaletteLink));
	prev_link->next->checksum = checksum;
	prev_link->next->next = NULL;
	prev_link->next->Palette = Pixels;
      }
    } else {
      Pixels = current_link->Palette;
    }
    graphic->Pixels = Pixels;
    free(graphic->Palette);
    
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
