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
//	(c) Copyright 1998-2001 by Lutz Sammer
//
//	$Id$

//@{

/*----------------------------------------------------------------------------
--	Includes
----------------------------------------------------------------------------*/

#include <stdio.h>

#include "freecraft.h"
#include "video.h"
#include "font.h"

/*----------------------------------------------------------------------------
--	Variables
----------------------------------------------------------------------------*/

/**
**	Fonts table
**
**	Define the font files, sizes.
*/
global ColorFont Fonts[] = {
#ifdef NEW_NAMES
    { "graphic/ui/fonts/game.png",	13,14 },
    { "graphic/ui/fonts/small.png",	 7, 6 },
    { "graphic/ui/fonts/large.png",	17,17 },
#else
    { "graphic/small font.png",		13,14 },
    { "graphic/game font.png",		 7, 6 },
    { "graphic/large font.png",		17,17 },
#endif
};

/**
**	Font color table.
**
**	FIXME: only yellow, grey and white are correct.
*/
global unsigned char FontColors[16][8] = {
    // 0 black
    {   0, 228, 228, 228, 228, 239,   0 },
    // 1 red
    {   0, 208, 209, 210, 211, 239,   0 },
    // 2 green
    {   0, 216, 216, 216, 216, 239,   0 },
    // 3 yellow
    { 246, 200, 199, 197, 192, 239, 104 },
    // 4 blue
    {   0,   1,   1,   1,   1, 239,   0 },
    // 5 magenta
    {   0, 220, 220, 220, 220, 239,   0 },
    // 6 cyan
    {   0, 224, 224, 224, 224, 239,   0 },
    // 7 white
    {   0, 246, 246, 246, 104, 239,   0 },
    // 8 grey
    {   0, 111, 110, 109, 104, 239,   0 },
    // 9 light-red
    {   0, 208, 208, 208, 208, 239,   0 },
    // a light-green
    {   0, 216, 216, 216, 216, 239,   0 },
    // b light-yellow
    { 246, 200, 199, 197, 192, 239, 104 },
    // c light-blue
    {   0,   1,   1,   1,   1, 239,   0 },
    // d light-magenta
    {   0, 220, 220, 220, 220, 239,   0 },
    // e light-cyan
    {   0, 224, 224, 224, 224, 239,   0 },
    // f light grey
    {   0, 111, 110, 109, 104, 239,   0 },
};

    /// Current color
local const unsigned char* TextColor;
    /// Last text color
local const unsigned char* LastTextColor;
    /// Default text color
local const unsigned char* DefaultTextColor;
    /// Reverse text color
local const unsigned char* ReverseTextColor;
    /// Default normal color index
local int nc_font_idx;
    /// Default reverse color index
local int rc_font_idx;

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

    sp=sprite->Frames+gx+gy*sprite->Width-1;
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

    sp=sprite->Frames+gx+gy*sprite->Width-1;
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

    sp=sprite->Frames+gx+gy*sprite->Width-1;
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

    sp=sprite->Frames+gx+gy*sprite->Width-1;
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
    nc_font_idx=normal;
    rc_font_idx=reverse;
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
    *normalp=nc_font_idx;
    *reversep=rc_font_idx;
}

/**
**	Returns the pixel length of a text.
**
**	@param font	Font number.
**	@param text	Text to calculate the length of.
**
**	@return		The length in pixels of the text.
*/
global int TextLength(unsigned font,const unsigned char* text)
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
**	@return		The lenght of the printed text.
*/
global int DrawText(int x,int y,unsigned font,const unsigned char* text)
{
    int w;
    int height;
    int widths;
    const ColorFont* fp;
    const unsigned char* rev;

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

	w=fp->CharWidth[*text-32];
	VideoDrawChar(fp->Graphic,0,height*(*text-32),w,height,x+widths,y);
	widths+=w+1;
	if( rev ) {
	    TextColor=rev;
	    rev=NULL;
	}
    }

    return widths;
}

/**
**	Draw reverse text with font at x,y unclipped.
**
**	@see DrawText for full description.
**
**	@param x	X screen position
**	@param y	Y screen position
**	@param font	Font number
**	@param text	Text to be displayed.
**
**	@return		The lenght of the printed text.
*/
global int DrawReverseText(int x,int y,unsigned font,const unsigned char* text)
{
    int w;

    TextColor=ReverseTextColor;
    w=DrawText(x,y,font,text);
    TextColor=DefaultTextColor;

    return w;
}

/**
**	Draw text with font at x,y centered.
**
**	@see DrawText for full description.
**
**	@param x	X screen position
**	@param y	Y screen position
**	@param font	Font number
**	@param text	Text to be displayed.
**
**	@return		The lenght of the printed text.
*/
global int DrawTextCentered(int x,int y,unsigned font,const unsigned char* text)
{
    int dx;

    dx=TextLength(font,text);
    DrawText(x-dx/2,y,font,text);

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
**	@return		The lenght of the printed text.
*/
global int DrawNumber(int x,int y,unsigned font,int number)
{
    char buf[sizeof(int)*10+2];

    sprintf(buf,"%d",number);
    return DrawText(x,y,font,buf);
}

/**
**	Draw reverse number with font at x,y unclipped.
**
**	@param x	X screen position
**	@param y	Y screen position
**	@param font	Font number
**	@param number	Number to be displayed.
**
**	@return		The lenght of the printed text.
*/
global int DrawReverseNumber(int x,int y,unsigned font,int number)
{
    char buf[sizeof(int)*10+2];

    sprintf(buf,"%d",number);
    return DrawReverseText(x,y,font,buf);
}

/**
**	Calculate widths table for a font.
**
// FIXME: ARI: This is runtime and fairly slow!
// FIXME: ARI: Maybe integrate into wartool and load from file!
*/
local void FontMeasureWidths(ColorFont* fp)
{
    int i, x, y, h, w;
    const unsigned char* sp;
    const unsigned char* lp;
    const unsigned char* gp;
    const Graphic *sprite;

    sprite = fp->Graphic;
    w = sprite->Width;
    h = fp->Height;
    for (y = 1; y < 207; y++) {
	sp = sprite->Frames + y * h * w - 1;
	gp = sp + w * h;
	x = 0;
	while (sp < gp) {
	    lp = sp + w;
	    while (sp < lp) {
		if (*++sp != 255) {
		    i = w - (lp - sp);
		    if (i > x) {	// max width
			x = i;
		    }
		}
	    }
	}
	fp->CharWidth[y] = x;
    }
    fp->CharWidth[0] = fp->Width / 2; // a reasonable value for SPACE
}

/**
**	Load all fonts.
*/
global void LoadFonts(void)
{
    unsigned i;

    for( i=0; i<sizeof(Fonts)/sizeof(*Fonts); ++i ) {
	ShowLoadProgress("\tFonts %s\n",Fonts[i].File);
	Fonts[i].Graphic=LoadGraphic(Fonts[i].File);
	FontMeasureWidths(Fonts+i);
    }

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
	    DebugLevel0Fn("unsupported %d bpp\n",VideoBpp);
	    abort();
    }
}

/*----------------------------------------------------------------------------
--	CCL
----------------------------------------------------------------------------*/

#if defined(USE_CCL) || defined(USE_CCL2)	// {

#include "ccl.h"

/**
**	Define the used fonts.
*/
local SCM CclDefineFont(SCM type,SCM file,SCM width,SCM height)
{
    int i;

    if( gh_eq_p(type,gh_symbol2scm("game")) ) {
	i=0;
    } else if( gh_eq_p(type,gh_symbol2scm("small")) ) {
	i=1;
    } else if( gh_eq_p(type,gh_symbol2scm("large")) ) {
	i=2;
    } else {
	fprintf(stderr,"unsupported font type\n");
	return SCM_UNSPECIFIED;
    }

    CclFree(Fonts[i].File);
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

#endif	// } USE_CCL or USE_CCL2

//@}
