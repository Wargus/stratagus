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
/**@name font.c		-	The color fonts. */
//
//	(c) Copyright 1998-2002 by Lutz Sammer
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
#include <string.h>

#include "freecraft.h"
#include "video.h"
#include "font.h"

#include "intern_video.h"

/*----------------------------------------------------------------------------
--	Variables
----------------------------------------------------------------------------*/

/**
**	Fonts table
**
**	Define the font files, sizes.
*/
local ColorFont Fonts[MaxFonts];

/**
**	Font color table.
*/
global unsigned char FontColors[16][8];

    /// Current color
local const unsigned char* TextColor;
    /// Last text color
local const unsigned char* LastTextColor;
    /// Default text color
local const unsigned char* DefaultTextColor;
    /// Reverse text color
local const unsigned char* ReverseTextColor;
    /// Default normal color index
local int DefaultNormalColorIndex;
    /// Default reverse color index
local int DefaultReverseColorIndex;

    /// Draw character with current video depth.
local void (*VideoDrawChar)(const Graphic*,int,int,int,int,int,int);

/*----------------------------------------------------------------------------
--	Functions
----------------------------------------------------------------------------*/

// FIXME: should use RLE encoded fonts, not color key fonts.

/**
**	Draw character with current color into 8bit video memory.
**
**	@param sprite	Pointer to object
**	@param gx	X offset into object
**	@param gy	Y offset into object
**	@param w	width to display
**	@param h	height to display
**	@param x	X screen position
**	@param y	Y screen position
*/
local void VideoDrawChar8(const Graphic* sprite,
	int gx,int gy,int w,int h,int x,int y)
{
    int p;
    const unsigned char* sp;
    const unsigned char* lp;
    const unsigned char* gp;
    int sa;
    VMemType8* dp;
    int da;

    sp=(const unsigned char*)sprite->Frames+gx+gy*sprite->Width-1;
    gp=sp+sprite->Width*h;
    sa=sprite->Width-w;
    dp=VideoMemory8+x+y*VideoWidth-1;
    da=VideoWidth-w;
    --w;

    while( sp<gp ) {
	lp=sp+w;
	while( sp<lp ) {		// loop with unroll
	    ++dp;
	    p=*++sp;
	    if( p!=255 ) {
		*dp=Pixels8[TextColor[p]];
	    }
	    ++dp;
	    p=*++sp;
	    if( p!=255 ) {
		*dp=Pixels8[TextColor[p]];
	    }
	}
	if( sp<=lp ) {
	    ++dp;
	    p=*++sp;
	    if( p!=255 ) {
		*dp=Pixels8[TextColor[p]];
	    }
	}
	sp+=sa;
	dp+=da;
    }
}

/**
**	Draw character with current color into 16bit video memory.
**
**	@param sprite	Pointer to object
**	@param gx	X offset into object
**	@param gy	Y offset into object
**	@param w	width to display
**	@param h	height to display
**	@param x	X screen position
**	@param y	Y screen position
*/
local void VideoDrawChar16(const Graphic* sprite,
	int gx,int gy,int w,int h,int x,int y)
{
    int p;
    const unsigned char* sp;
    const unsigned char* lp;
    const unsigned char* gp;
    int sa;
    VMemType16* dp;
    int da;

    sp=(const unsigned char*)sprite->Frames+gx+gy*sprite->Width-1;
    gp=sp+sprite->Width*h;
    sa=sprite->Width-w;
    dp=VideoMemory16+x+y*VideoWidth-1;
    da=VideoWidth-w;
    --w;

    while( sp<gp ) {
	lp=sp+w;
	while( sp<lp ) {		// loop with unroll
	    ++dp;
	    p=*++sp;
	    if( p!=255 ) {
		*dp=Pixels16[TextColor[p]];
	    }
	    ++dp;
	    p=*++sp;
	    if( p!=255 ) {
		*dp=Pixels16[TextColor[p]];
	    }
	}
	if( sp<=lp ) {
	    ++dp;
	    p=*++sp;
	    if( p!=255 ) {
		*dp=Pixels16[TextColor[p]];
	    }
	}
	sp+=sa;
	dp+=da;
    }
}

/**
**	Draw character with current color into 24bit video memory.
**
**	@param sprite	Pointer to object
**	@param gx	X offset into object
**	@param gy	Y offset into object
**	@param w	width to display
**	@param h	height to display
**	@param x	X screen position
**	@param y	Y screen position
*/
local void VideoDrawChar24(const Graphic* sprite,
	int gx,int gy,int w,int h,int x,int y)
{
    int p;
    const unsigned char* sp;
    const unsigned char* lp;
    const unsigned char* gp;
    int sa;
    VMemType24* dp;
    int da;

    sp=(const unsigned char*)sprite->Frames+gx+gy*sprite->Width-1;
    gp=sp+sprite->Width*h;
    sa=sprite->Width-w;
    dp=VideoMemory24+x+y*VideoWidth-1;
    da=VideoWidth-w;
    --w;

    while( sp<gp ) {
	lp=sp+w;
	while( sp<lp ) {		// loop with unroll
	    ++dp;
	    p=*++sp;
	    if( p!=255 ) {
		*dp=Pixels24[TextColor[p]];
	    }
	    ++dp;
	    p=*++sp;
	    if( p!=255 ) {
		*dp=Pixels24[TextColor[p]];
	    }
	}
	if( sp<=lp ) {
	    ++dp;
	    p=*++sp;
	    if( p!=255 ) {
		*dp=Pixels24[TextColor[p]];
	    }
	}
	sp+=sa;
	dp+=da;
    }
}

/**
**	Draw character with current color into 32bit video memory.
**
**	@param sprite	Pointer to object
**	@param gx	X offset into object
**	@param gy	Y offset into object
**	@param w	width to display
**	@param h	height to display
**	@param x	X screen position
**	@param y	Y screen position
*/
local void VideoDrawChar32(const Graphic* sprite,
	int gx,int gy,int w,int h,int x,int y)
{
    int p;
    const unsigned char* sp;
    const unsigned char* lp;
    const unsigned char* gp;
    int sa;
    VMemType32* dp;
    int da;

    sp=(const unsigned char*)sprite->Frames+gx+gy*sprite->Width-1;
    gp=sp+sprite->Width*h;
    sa=sprite->Width-w;
    dp=VideoMemory32+x+y*VideoWidth-1;
    da=VideoWidth-w;
    --w;

    while( sp<gp ) {
	lp=sp+w;
	while( sp<lp ) {		// loop with unroll
	    ++dp;
	    p=*++sp;
	    if( p!=255 ) {
		*dp=Pixels32[TextColor[p]];
	    }
	    ++dp;
	    p=*++sp;
	    if( p!=255 ) {
		*dp=Pixels32[TextColor[p]];
	    }
	}
	if( sp<=lp ) {
	    ++dp;
	    p=*++sp;
	    if( p!=255 ) {
		*dp=Pixels32[TextColor[p]];
	    }
	}
	sp+=sa;
	dp+=da;
    }
}

/**
**	Set the default text colors.
**
**	@param normal	Normal text color.
**	@param reverse	Reverse text color.
*/
global void SetDefaultTextColors(int normal,int reverse)
{
    DefaultNormalColorIndex=normal;
    DefaultReverseColorIndex=reverse;
    LastTextColor=TextColor=DefaultTextColor=FontColors[normal];
    ReverseTextColor=FontColors[reverse];
}

/**
**	Get the default text colors.
**
**	@param normalp	Normal text color pointer.
**	@param reversep	Reverse text color pointer.
*/
global void GetDefaultTextColors(int *normalp,int *reversep)
{
    *normalp=DefaultNormalColorIndex;
    *reversep=DefaultReverseColorIndex;
}

/**
**	Returns the pixel length of a text.
**
**	@param font	Font number.
**	@param text	Text to calculate the length of.
**
**	@return		The length in pixels of the text.
*/
global int VideoTextLength(unsigned font,const unsigned char* text)
{
    int width;
    const unsigned char* s;
    const char* widths;

    widths=Fonts[font].CharWidth;
    for( width=0,s=text; *s; ++s ) {
	if( *s=='~' )  {
	    if( !*++s ) {		// bad formated string
		break;
	    }
	    if( *s!='~' ) {		// ~~ -> ~
		continue;
	    }
	}
	width+=widths[*s-32]+1;
    }
    return width;
}

/**
**	Returns the height of the font.
**
**	@param font	Font number.
**
**	@return		The height of the font.
*/
global int VideoTextHeight(unsigned font)
{
    return Fonts[font].Height;
}

/**
**	Draw character with current color clipped into 8 bit framebuffer.
**
**	@param graphic	Pointer to object
**	@param gx	X offset into object
**	@param gy	Y offset into object
**	@param w	width to display
**	@param h	height to display
**	@param x	X screen position
**	@param y	Y screen position
*/
local void VideoDrawCharClip(const Graphic* graphic,int gx,int gy,int w,int h,
	int x,int y)
{
    int ox,oy,ex;
    CLIP_RECTANGLE_OFS(x,y,w,h,ox,oy,ex);
    VideoDrawChar(graphic,gx+ox,gy+oy,w,h,x,y);
}

/**
**	Draw text with font at x,y clipped/unclipped.
**
**	~	is special prefix.
**	~~	is the ~ character self.
**	~!	print next character reverse.
**	~n	0123456789abcdef print text in color 1-16.
**	~<	start reverse.
**	~>	switch back to last used color.
**
**	@param x	X screen position
**	@param y	Y screen position
**	@param font	Font number
**	@param text	Text to be displayed.
**	@param clip	Flag if TRUE clip, otherwise not.
**
**	@return		The length of the printed text.
*/
local int DoDrawText(int x,int y,unsigned font,const unsigned char* text,
	int clip)
{
    int w;
    int height;
    int widths;
    const ColorFont* fp;
    const unsigned char* rev;
    void (*DrawChar)(const Graphic*,int,int,int,int,int,int);

    if( clip ) {
	DrawChar=VideoDrawCharClip;
    } else {
	DrawChar=VideoDrawChar;
    }

    fp=Fonts+font;
    height=fp->Height;
    for( rev=NULL,widths=0; *text; ++text ) {
	if( *text=='~' ) {
	    switch( *++text ) {
		case '\0':		// wrong formated string.
		    DebugLevel0Fn("oops, format your ~\n");
		    return widths;
		case '~':
		    break;
		case '0': case '1': case '2': case '3': case '4':
		case '5': case '6': case '7': case '8': case '9':
		    LastTextColor=TextColor;
		    TextColor=FontColors[*text-'0'];
		    continue;
		case 'a': case 'b': case 'c': case 'd': case 'e': case 'f':
		    LastTextColor=TextColor;
		    TextColor=FontColors[*text+10-'a'];
		    continue;
		case 'A': case 'B': case 'C': case 'D': case 'E': case 'F':
		    LastTextColor=TextColor;
		    TextColor=FontColors[*text+10-'A'];
		    continue;
		case '!':
		    rev=TextColor;
		    TextColor=ReverseTextColor;
		    ++text;
		    break;
		case '<':
		    LastTextColor=TextColor;
		    TextColor=ReverseTextColor;
		    continue;
		case '>':
		    rev=LastTextColor;	// swap last and current color
		    LastTextColor=TextColor;
		    TextColor=rev;
		    continue;

		default:
		    DebugLevel0Fn("oops, format your ~\n");
		    continue;
	    }
	}

	if( height*(*text-32)<fp->Graphic->Height ) {
	    w=fp->CharWidth[*text-32];
	    DrawChar(fp->Graphic,0,height*(*text-32),w,height,x+widths,y);
	} else {
	    w=fp->CharWidth[0];
	    DrawChar(fp->Graphic,0,height*0,w,height,x+widths,y);
	}
	widths+=w+1;
	if( rev ) {
	    TextColor=rev;
	    rev=NULL;
	}
    }

    return widths;
}

/**
**	Draw text with font at x,y unclipped.
**
**	~	is special prefix.
**	~~	is the ~ character self.
**	~!	print next character reverse.
**	~n	0123456789abcdef print text in color 1-16.
**	~<	start reverse.
**	~>	switch back to last used color.
**
**	@param x	X screen position
**	@param y	Y screen position
**	@param font	Font number
**	@param text	Text to be displayed.
**
**	@return		The length of the printed text.
*/
global int VideoDrawText(int x,int y,unsigned font,const unsigned char* text)
{
    return DoDrawText(x,y,font,text,0);
}

/**
**	Draw text with font at x,y clipped.
**
**	See VideoDrawText.
**
**	@return		The length of the printed text.
*/
global int VideoDrawTextClip(int x,int y,unsigned font,
			     const unsigned char* text)
{
    return DoDrawText(x,y,font,text,1);
}

/**
**	Draw reverse text with font at x,y unclipped.
**
**	@see VideoDrawText for full description.
**
**	@param x	X screen position
**	@param y	Y screen position
**	@param font	Font number
**	@param text	Text to be displayed.
**
**	@return		The length of the printed text.
*/
global int VideoDrawReverseText(int x,int y,unsigned font,
	const unsigned char* text)
{
    int w;

    TextColor=ReverseTextColor;
    w=VideoDrawText(x,y,font,text);
    TextColor=DefaultTextColor;

    return w;
}

/**
**	Draw text with font at x,y centered.
**
**	@see VideoDrawText for full description.
**
**	@param x	X screen position
**	@param y	Y screen position
**	@param font	Font number
**	@param text	Text to be displayed.
**
**	@return		The length of the printed text.
*/
global int VideoDrawTextCentered(int x,int y,unsigned font,
	const unsigned char* text)
{
    int dx;

    dx=VideoTextLength(font,text);
    VideoDrawText(x-dx/2,y,font,text);

    return dx/2;
}

/**
**	Draw number with font at x,y unclipped.
**
**	@param x	X screen position
**	@param y	Y screen position
**	@param font	Font number
**	@param number	Number to be displayed.
**
**	@return		The length of the printed text.
*/
global int VideoDrawNumber(int x, int y, unsigned font, int number)
{
    char bufs[sizeof(int) * 10 + 2];
    char bufd[sizeof(int) * 10 + 2];
    int sl;
    int s;
    int d;

    sl = s = d = 0;
    sprintf(bufs, "%d", number);
    sl = strlen(bufs);
    do {
	if (s > 0 && s < sl && (s - (sl % 3)) % 3 == 0) {
	    bufd[d++] = ',';
	}
	bufd[d++] = bufs[s++];
    } while (s <= sl);
    return VideoDrawText(x, y, font, bufd);
}

/**
**	Draw number with font at x,y clipped.
**
**	@param x	X screen position
**	@param y	Y screen position
**	@param font	Font number
**	@param number	Number to be displayed.
**
**	@return		The length of the printed text.
*/
global int VideoDrawNumberClip(int x,int y,unsigned font,int number)
{
    char buf[sizeof(int)*10+2];

    sprintf(buf,"%d",number);
    return VideoDrawTextClip(x,y,font,buf);
}

/**
**	Draw reverse number with font at x,y unclipped.
**
**	@param x	X screen position
**	@param y	Y screen position
**	@param font	Font number
**	@param number	Number to be displayed.
**
**	@return		The length of the printed text.
*/
global int VideoDrawReverseNumber(int x,int y,unsigned font,int number)
{
    char buf[sizeof(int)*10+2];

    sprintf(buf,"%d",number);
    return VideoDrawReverseText(x,y,font,buf);
}

/**
**	Calculate widths table for a font.
**
// FIXME: ARI: This is runtime and fairly slow!
// FIXME: ARI: Maybe integrate into wartool and load from file!
*/
local void FontMeasureWidths(ColorFont * fp)
{
    int y;
    const unsigned char *sp;
    const unsigned char *lp;
    const unsigned char *gp;

    for (y = 1; y < 207; y++) {
	fp->CharWidth[y] = 0;
    }

    for (y = 1; y < 207; y++) {
	sp = (const unsigned char *)fp->Graphic->Frames +
	    y * fp->Height * fp->Graphic->Width - 1;
	gp = sp + fp->Graphic->Width * fp->Height;
	//	Bail out cause there are no letters left
	if (gp >= ((const unsigned char *)fp->Graphic->Frames +
		fp->Graphic->Width * fp->Graphic->Height)) {
	    break;
	}
	while (sp < gp) {
	    lp = sp + fp->Graphic->Width - 1;
	    for (; sp < lp; --lp) {
		if (*lp != 255) {
		    if (lp - sp > fp->CharWidth[y]) {	// max width
			fp->CharWidth[y] = lp - sp;
		    }
		}
	    }
	    sp += fp->Graphic->Width;
	}
    }
    fp->CharWidth[0] = fp->Width / 2;	// a reasonable value for SPACE
}

/**
**	Load all fonts.
*/
global void LoadFonts(void)
{
    unsigned i;

    switch( VideoBpp ) {
	case 8:
	    VideoDrawChar=VideoDrawChar8;
	    break;

	case 15:
	case 16:
	    VideoDrawChar=VideoDrawChar16;
	    break;

	case 24:
	    VideoDrawChar=VideoDrawChar24;
	    break;

	case 32:
	    VideoDrawChar=VideoDrawChar32;
	    break;

	default:
	    DebugLevel0Fn("unsupported %d bpp\n" _C_ VideoBpp);
	    abort();
    }

    for( i=0; i<sizeof(Fonts)/sizeof(*Fonts); ++i ) {
	ShowLoadProgress("\tFonts %s\n",Fonts[i].File);
	Fonts[i].Graphic=LoadGraphic(Fonts[i].File);
	FontMeasureWidths(Fonts+i);
    }
}

/*----------------------------------------------------------------------------
--	CCL
----------------------------------------------------------------------------*/

#include "ccl.h"

/**
**	Define the used fonts.
**
**	@param type	Type of the font (game,small,...)
**	@param file	File name of the graphic file
**	@param width	Font width in pixels
**	@param height	Font height in pixels
**
**	@todo	make the font name functions more general, support more fonts.
*/
local SCM CclDefineFont(SCM type,SCM file,SCM width,SCM height)
{
    int i;

    if( gh_eq_p(type,gh_symbol2scm("game")) ) {
	i=GameFont;
    } else if( gh_eq_p(type,gh_symbol2scm("small")) ) {
	i=SmallFont;
    } else if( gh_eq_p(type,gh_symbol2scm("large")) ) {
	i=LargeFont;
    } else if( gh_eq_p(type,gh_symbol2scm("small-title")) ) {
	i=SmallTitleFont;
    } else if( gh_eq_p(type,gh_symbol2scm("large-title")) ) {
	i=LargeTitleFont;
    } else if( gh_eq_p(type,gh_symbol2scm("user1")) ) {
	i=User1Font;
    } else if( gh_eq_p(type,gh_symbol2scm("user2")) ) {
	i=User2Font;
    } else if( gh_eq_p(type,gh_symbol2scm("user3")) ) {
	i=User3Font;
    } else if( gh_eq_p(type,gh_symbol2scm("user4")) ) {
	i=User4Font;
    } else if( gh_eq_p(type,gh_symbol2scm("user5")) ) {
	i=User5Font;
    } else {
	fprintf(stderr,"unsupported font type\n");
	return SCM_UNSPECIFIED;
    }

    free(Fonts[i].File);
    Fonts[i].File=gh_scm2newstr(file,NULL);
    Fonts[i].Width=gh_scm2int(width);
    Fonts[i].Height=gh_scm2int(height);

    return SCM_UNSPECIFIED;
}

/**
**	Define the used font colors.
*/
local SCM CclDefineFontColors(SCM list)
{
    int i;
    int j;
    SCM value;
    SCM temp;

    for( i=0; i<16; ++i ) {
	value=gh_car(list);
	list=gh_cdr(list);

	if( gh_vector_length(value)!=7 ) {
	    fprintf(stderr,"Wrong vector length\n");
	}
	for( j=0; j<7; ++j ) {
	    temp=gh_vector_ref(value,gh_int2scm(j));
	    FontColors[i][j]=gh_scm2int(temp);
	}
    }

    return SCM_UNSPECIFIED;
}

/**
**	Register CCL features for fonts.
**
**	@todo FIXME: Make the remaining functions accessable from CCL.
*/
global void FontsCclRegister(void)
{
    gh_new_procedure4_0("define-font",CclDefineFont);
    gh_new_procedureN("define-font-colors",CclDefineFontColors);

    //gh_new_procedure2_0("default-text-colors",CclDefaultTextColors);
    //gh_new_procedure1_0("text-length",CclTextLength);
    //gh_new_procedure4_0("draw-text",CclDrawText);
    //gh_new_procedure4_0("draw-reverse-text",CclDrawReverseText);
    //gh_new_procedure4_0("draw-text-centered",CclDrawTextCentered);
    //gh_new_procedure4_0("draw-reverse-text-centered",CclDrawReverseTextCentered);
    //gh_new_procedure4_0("draw-number",CclDrawNumber);
    //gh_new_procedure4_0("draw-reverse-number",CclDrawReverseNumber);
}

/**
**	Cleanup the font module.
*/
global void CleanFonts(void)
{
    unsigned i;

    for( i=0; i<sizeof(Fonts)/sizeof(*Fonts); ++i ) {
	free(Fonts[i].File);
	VideoFree(Fonts[i].Graphic);
	Fonts[i].File=NULL;
	Fonts[i].Graphic=NULL;
    }

}

/**
**	Check if font is already loaded.
**
**	@param font	Font number
**
**	@return		True if loaded, false otherwise.
*/
global int IsFontLoaded(unsigned font)
{
    return Fonts[font].Graphic!=0;
}

//@}
