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
//
//	(c) Copyright 1999-2002 by Lutz Sammer
//
//	FreeCraft is free software; you can redistribute it and/or modify
//	it under the terms of the GNU General Public License as published
//	by the Free Software Foundation; only version 2 of the License.
//
//	FreeCraft is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//	GNU General Public License for more details.
//
//	$Id$

//@{

/*----------------------------------------------------------------------------
--	Includes
----------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>

#include "freecraft.h"
#include "video.h"
#include "iolib.h"
#include "intern_video.h"

/*----------------------------------------------------------------------------
--	Declarations
----------------------------------------------------------------------------*/

global PaletteLink *PaletteList;	/// List of all used palettes.

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
	const Graphic* graphic,int gx,int gy,int w,int h,
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
    sp=(const unsigned char*)graphic->Frames+gx+gy*graphic->Width;
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
	const Graphic* graphic,int gx,int gy,int w,int h,
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
    sp=(const unsigned char*)graphic->Frames+gx+gy*graphic->Width;
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
	const Graphic* graphic,int gx,int gy,int w,int h,
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
    sp=(const unsigned char*)graphic->Frames+gx+gy*graphic->Width;
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
	const Graphic* graphic,int gx,int gy,int w,int h,
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
    sp=(const unsigned char*)graphic->Frames+gx+gy*graphic->Width;
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
local void VideoDrawSubOpenGL(
	const Graphic* graphic,int gx,int gy,int w,int h,
	int x,int y)
{
    GLfloat sx,ex,sy,ey;
    GLfloat stx,etx,sty,ety;

    sx=(GLfloat)x/VideoWidth;
    ex=sx+(GLfloat)w/VideoWidth;
    ey=1.0f-(GLfloat)y/VideoHeight;
    sy=ey-(GLfloat)h/VideoHeight;

    stx=(GLfloat)gx/graphic->Width*graphic->TextureWidth;
    etx=(GLfloat)(gx+w)/graphic->Width*graphic->TextureWidth;
    sty=(GLfloat)gy/graphic->Height*graphic->TextureHeight;
    ety=(GLfloat)(gy+h)/graphic->Height*graphic->TextureHeight;

    glBindTexture(GL_TEXTURE_2D, graphic->TextureNames[0]);
    glBegin(GL_QUADS);
    glTexCoord2f(stx, 1.0f-ety);
    glVertex3f(sx, sy, 0.0f);
    glTexCoord2f(stx, 1.0f-sty);
    glVertex3f(sx, ey, 0.0f);
    glTexCoord2f(etx, 1.0f-sty);
    glVertex3f(ex, ey, 0.0f);
    glTexCoord2f(etx, 1.0f-ety);
    glVertex3f(ex, sy, 0.0f);
    glEnd();
}
#endif

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
	const Graphic* graphic,int gx,int gy,int w,int h,
	int x,int y)
{
    CLIP_RECTANGLE(x,y,w,h);
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
	const Graphic* graphic,int gx,int gy,int w,int h,
	int x,int y)
{
    CLIP_RECTANGLE(x,y,w,h);
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
	const Graphic* graphic,int gx,int gy,int w,int h,
	int x,int y)
{
    CLIP_RECTANGLE(x,y,w,h);
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
	const Graphic* graphic,int gx,int gy,int w,int h,
	int x,int y)
{
    CLIP_RECTANGLE(x,y,w,h);
    VideoDrawSub8to32(graphic,gx,gy,w,h,x,y);
}

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
local void VideoDrawSubOpenGLClip(
	const Graphic* graphic,int gx,int gy,int w,int h,
	int x,int y)
{
    CLIP_RECTANGLE(x,y,w,h);
    VideoDrawSubOpenGL(graphic,gx,gy,w,h,x,y);
}
#endif

/**
**	Free graphic object.
*/
local void FreeGraphic8(Graphic* graphic)
{
    IfDebug(AllocatedGraphicMemory -= graphic->Size);
    IfDebug(AllocatedGraphicMemory -= sizeof(Graphic));

#ifdef USE_OPENGL
    if( graphic->NumTextureNames ) {
	glDeleteTextures(graphic->NumTextureNames, graphic->TextureNames);
	free(graphic->TextureNames);
    }
#endif
    VideoFreeSharedPalette(graphic->Pixels);
    if (graphic->Palette) {
	free(graphic->Palette);
    }
    free(graphic->Frames);
    free(graphic);
}

// FIXME: need frame version

// FIXME: need 16 bit palette version

// FIXME: need zooming version

/**
**	Allocates a new palette and fades
*/
local void* FadePixels(void* pixels,int fade)
{
    void* p;
    int i;

    p=NULL;

    if( VideoBpp==8 ) {
	VMemType8 c;

	p=malloc(256*sizeof(VMemType8));
	for( i=0; i<256; i++ ) {
	    c=((VMemType8*)pixels)[i];
	    // FIXME: doesn't work for 8bpp yet
	    ((VMemType8*)p)[i]=c;
	}
    }
    else if( VideoBpp==15 ) {
	VMemType16 c;
	VMemType16 bl;

	bl=Pixels16[ColorBlack];
	p=malloc(256*sizeof(VMemType16));
	for( i=0; i<256; i++ ) {
	    int r,g,b;
	    c=((VMemType16*)pixels)[i];
	    r=(((c>>10)&0x1f)-((bl>>10)&0x1f)) * fade/256 + ((bl>>10)&0x1f);
	    g=(((c>>5)&0x1f)-((bl>>5)&0x1f)) * fade/256 + ((bl>>5)&0x1f);
	    b=((c&0x1f)-(bl&0x1f)) * fade/256 + (bl&0x1f);
	    ((VMemType16*)p)[i] = (r<<10) | (g<<5) | b;
	}
    }
    else if( VideoBpp==16 ) {
	VMemType16 c;
	VMemType16 bl;

	bl=Pixels16[ColorBlack];
	p=malloc(256*sizeof(VMemType16));
	for( i=0; i<256; i++ ) {
	    int r,g,b;
	    c=((VMemType16*)pixels)[i];
	    r=(((c>>11)&0x1f)-((bl>>11)&0x1f)) * fade/256 + ((bl>>11)&0x1f);
	    g=(((c>>5)&0x3f)-((bl>>5)&0x3f)) * fade/256 + ((bl>>5)&0x3f);
	    b=((c&0x1f)-(bl&0x1f)) * fade/256 + (bl&0x1f);
	    ((VMemType16*)p)[i] = (r<<11) | (g<<5) | b;
	}
    }
    else if( VideoBpp==24 ) {
	VMemType24 c;
	VMemType24 bl;

	bl=Pixels24[ColorBlack];
	p=malloc(256*sizeof(VMemType24));
	for( i=0; i<256; i++ ) {
	    c=((VMemType24*)pixels)[i];
	    ((VMemType24*)p)[i].a=((unsigned char)c.a-bl.a) * fade/256 + bl.a;
	    ((VMemType24*)p)[i].b=((unsigned char)c.b-bl.b) * fade/256 + bl.b;
	    ((VMemType24*)p)[i].c=((unsigned char)c.c-bl.c) * fade/256 + bl.c;
	}
    }
    else if( VideoBpp==32 ) {
	VMemType32 c;
	unsigned char* x;
	VMemType32 bl;

	bl=Pixels32[ColorBlack];
	p=malloc(256*sizeof(VMemType32));
	for( i=0; i<256; i++ ) {
	    c=((VMemType32*)pixels)[i];
	    x=(char*)&((VMemType32*)p)[i];
	    x[2]=(((c>>16)&0xff)-((bl>>16)&0xff)) * fade/256 + ((bl>>16)&0xff);
	    x[1]=(((c>>8)&0xff)-((bl>>8)&0xff)) * fade/256 + ((bl>>8)&0xff);
	    x[0]=((c&0xff)-(bl&0xff)) * fade/256 + (bl&0xff);
	}
    }

    return p;
}

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
**	@param fade	Amount faded, from 0 (black) to 256 (no fading)
*/
global void VideoDrawSubClipFaded(
	Graphic* graphic,int gx,int gy,int w,int h,
	int x,int y,int fade)
{
    void* oldpixels;

    oldpixels=graphic->Pixels;

    graphic->Pixels=FadePixels(graphic->Pixels,fade);
    VideoDrawSubClip(graphic,gx,gy,w,h,x,y);

    free(graphic->Pixels);
    graphic->Pixels=oldpixels;
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
global Graphic* MakeGraphic(
	unsigned depth,int width,int height,void* data,unsigned size)
{
    Graphic* graphic;

    graphic=malloc(sizeof(Graphic));
    IfDebug( AllocatedGraphicMemory+=sizeof(Graphic) );

    if( !graphic ) {
	fprintf(stderr,"Out of memory\n");
	ExitFatal(-1);
    }
    if( depth==8 ) {
	graphic->Type=&GraphicImage8Type;
    } else if( depth==16 ) {
	graphic->Type=&GraphicImage16Type;
    } else {
	fprintf(stderr,"Unsported image depth\n");
	ExitFatal(-1);
    }
    graphic->Width=width;
    graphic->Height=height;

    graphic->Pixels=NULL;
    graphic->Palette=NULL;

    graphic->NumFrames=0;
    graphic->Frames=data;
    graphic->Size=size;
#ifdef USE_OPENGL
    graphic->NumTextureNames=0;
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
global Graphic* NewGraphic(
	unsigned depth,int width,int height)
{
    void* data;
    int size;

    size=width*height*(depth+7)/8;
    data=malloc(size);
    IfDebug( AllocatedGraphicMemory+=size );

    return MakeGraphic(depth,width,height,data,size);
}

/**
**	Make an OpenGL texture or textures out of a graphic object.
**
**	@param graphic	The graphic object.
**	@param width	Graphic width.
**	@param height	Graphic height.
*/
#ifdef USE_OPENGL
global void MakeTexture(Graphic* graphic,int width,int height)
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

    n=(graphic->Width/width)*(graphic->Height/height);
    fl=graphic->Width/width;

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    graphic->NumTextureNames = n;
    graphic->TextureNames = (GLuint*)malloc(n*sizeof(GLuint));
    glGenTextures(n, graphic->TextureNames);
    for( i=1; i<width; i<<=1 ) {
    }
    w=i;
    for( i=1; i<height; i<<=1 ) {
    }
    h=i;
    graphic->TextureWidth = (float)width/w;
    graphic->TextureHeight = (float)height/h;
    tex = (unsigned char*)malloc(w*h*4);
    for( x=0; x<n; ++x ) {
	glBindTexture(GL_TEXTURE_2D, graphic->TextureNames[x]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	for( i=0; i<height; ++i ) {
	    sp=(const unsigned char*)graphic->Frames+(x%fl)*width+((x/fl)*height+i)*graphic->Width;
	    for( j=0; j<width; ++j ) {
		int c;
		Palette p;

		c = (h-i-1)*w*4+j*4;
		if( *sp==255 ) {
		    tex[c+3] = 0;
		} else {
		    p = graphic->Palette[*sp];
		    tex[c+0] = p.r;
		    tex[c+1] = p.g;
		    tex[c+2] = p.b;
		    tex[c+3] = 0xff;
		}
		++sp;
	    }
	}
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA,
	    GL_UNSIGNED_BYTE, tex);
	IfDebug(
	    i = glGetError();
	    if (i) {
		DebugLevel0Fn("glTexImage2D(%x)\n" _C_ i);
	    }
	);
    }
    free(tex);
}

/**
**	Make an OpenGL texture of the player color pixels only.
*/
global void MakePlayerColorTexture(Graphic** g,Graphic* graphic,int frame,unsigned char *map,int maplen)
{
    int i;
    int j;
    int h;
    int w;
    int n;
    unsigned char* tex;
    const unsigned char* sp;
    int fl;

    if( !*g ) {
	*g = calloc(1,sizeof(Graphic));

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

    fl = graphic->GraphicWidth/graphic->Width;
    glGenTextures(1, &(*g)->TextureNames[frame]);

    for( i=1; i<graphic->Width; i<<=1 ) {
    }
    w=i;
    for( i=1; i<graphic->Height; i<<=1 ) {
    }
    h=i;

    tex = (unsigned char*)malloc(w*h*4);

    glBindTexture(GL_TEXTURE_2D, (*g)->TextureNames[frame]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    for( i=0; i<graphic->Height; ++i ) {
	sp=(const unsigned char*)graphic->Frames + (frame%fl)*graphic->Width + ((frame/fl)*graphic->Height+i)*graphic->GraphicWidth;
	for( j=0; j<graphic->Width; ++j ) {
	    int c;
	    int z;
	    Palette p;

	    c=(h-i-1)*w*4+j*4;
	    for( z=0; z<maplen; ++z ) {
		if( *sp==map[z*2] ) {
		    p=GlobalPalette[map[z*2+1]];
		    tex[c+0] = p.r;
		    tex[c+1] = p.g;
		    tex[c+2] = p.b;
		    tex[c+3] = 0xff;
		    break;
		}
	    }
	    if( z==maplen ) {
		tex[c+3] = 0;
	    }
	    ++sp;
	}
    }
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA,
	GL_UNSIGNED_BYTE, tex);
    IfDebug(
	i = glGetError();
	if (i) {
	    DebugLevel0Fn("glTexImage2D(%x)\n" _C_ i);
	}
    );
    free(tex);
}
#endif

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
global Graphic* LoadGraphic(const char *name)
{
    Graphic* graphic;
    char buf[1024];

    if (!(graphic = LoadGraphicPNG(LibraryFileName(name, buf)))) {
	fprintf(stderr, "Can't load the graphic `%s'\n", name);
	ExitFatal(-1);
    }

    graphic->Pixels = VideoCreateSharedPalette(graphic->Palette);
    //free(graphic->Palette);
    //graphic->Palette = NULL;		// JOHNS: why should we free this?

    return graphic;
}

/**
**	Init graphic
*/
global void InitGraphic(void)
{
#ifdef USE_OPENGL
    GraphicImage8Type.DrawSub=VideoDrawSubOpenGL;
    GraphicImage8Type.DrawSubClip=VideoDrawSubOpenGLClip;
#else
    switch( VideoBpp ) {
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
	    DebugLevel0Fn("unsupported %d bpp\n" _C_ VideoBpp);
	    abort();
    }
#endif
    GraphicImage8Type.Free=FreeGraphic8;
}

//@}
