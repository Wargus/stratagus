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
/**@name font.c		-	The color fonts. */
//
//	(c) Copyright 1998-2003 by Lutz Sammer and Jimmy Salmon
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
#include "font.h"
#include "ccl.h"

#include "intern_video.h"

/*----------------------------------------------------------------------------
--	Variables
----------------------------------------------------------------------------*/

#define NumFontColors 7

    /// Font color mapping
typedef struct _font_color_mapping_ {
    char* Color;			/// Font color
    struct {
	int R;
	int G;
	int B;
    } RGB[NumFontColors];
    VMemType Pixels[NumFontColors];	/// Array of hardware dependent pixels
    struct _font_color_mapping_* Next;	/// Next pointer
} FontColorMapping;

local const VMemType *FontPixels;		/// Font pixels
#define FontPixels8	(&FontPixels->D8)	/// font pixels  8bpp
#define FontPixels16	(&FontPixels->D16)	/// font pixels 16bpp
#define FontPixels24	(&FontPixels->D24)	/// font pixels 24bpp
#define FontPixels32	(&FontPixels->D32)	/// font pixels 32bpp

    /// Font color mappings
local FontColorMapping* FontColorMappings;

/**
**	Fonts table
**
**	Define the font files, sizes.
*/
local ColorFont Fonts[MaxFonts];

    /// Last text color
local const VMemType* LastTextColor;
    /// Default text color
local const VMemType* DefaultTextColor;
    /// Reverse text color
local const VMemType* ReverseTextColor;
    /// Default normal color index
local char* DefaultNormalColorIndex;
    /// Default reverse color index
local char* DefaultReverseColorIndex;

    /// Draw character with current video depth.
local void (*VideoDrawChar)(const Graphic*, int, int, int, int, int, int);

#ifdef USE_OPENGL
    /// Font bitmaps
local GLubyte* FontBitmaps[MaxFonts][NumFontColors];
    /// Font bitmap widths
local int FontBitmapWidths[MaxFonts];
    /// Current font
local int CurrentFont;
#endif

/**
**	FIXME: should use the names of the real fonts.
*/
global char* FontNames[] = {
    "small",
    "game",
    "large",
    "small-title",
    "large-title",
    "user1",
    "user2",
    "user3",
    "user4",
    "user5",
};

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
    int gx, int gy, int w, int h, int x, int y)
{
    int p;
    const unsigned char* sp;
    const unsigned char* lp;
    const unsigned char* gp;
    int sa;
    VMemType8* dp;
    int da;

    sp = (const unsigned char*)sprite->Frames + gx + gy * sprite->Width - 1;
    gp = sp + sprite->Width * h;
    sa = sprite->Width - w;
    dp = VideoMemory8 + x + y * VideoWidth - 1;
    da = VideoWidth - w;
    --w;
    
#define UNROLL \
    ++dp; \
    p = *++sp; \
    if (p != 255) { \
	if (p < NumFontColors) { \
	    *dp = FontPixels8[p]; \
	} else { \
	    *dp = ((VMemType8*)sprite->Pixels)[p]; \
	} \
    }

    while (sp < gp) {
	lp = sp + w;
	while (sp < lp) {		// loop with unroll
	    UNROLL;
	    UNROLL;
	}
	if (sp <= lp) {
	    UNROLL;
	}
	sp += sa;
	dp += da;
    }
#undef UNROLL
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
    int gx, int gy, int w, int h, int x, int y)
{
    int p;
    const unsigned char* sp;
    const unsigned char* lp;
    const unsigned char* gp;
    int sa;
    VMemType16* dp;
    int da;

    sp = (const unsigned char*)sprite->Frames + gx + gy * sprite->Width - 1;
    gp = sp + sprite->Width * h;
    sa = sprite->Width - w;
    dp = VideoMemory16 + x + y * VideoWidth - 1;
    da = VideoWidth - w;
    --w;

#define UNROLL \
    ++dp; \
    p = *++sp; \
    if (p != 255) { \
	if (p < NumFontColors) { \
	    *dp = FontPixels16[p]; \
	} else { \
	    *dp = ((VMemType16*)sprite->Pixels)[p]; \
	} \
    }

    while (sp < gp) {
	lp = sp + w;
	while (sp < lp) {		// loop with unroll
	    UNROLL;
	    UNROLL;
	}
	if (sp <= lp) {
	    UNROLL;
	}
	sp += sa;
	dp += da;
    }
#undef UNROLL
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
    int gx, int gy, int w, int h, int x, int y)
{
    int p;
    const unsigned char* sp;
    const unsigned char* lp;
    const unsigned char* gp;
    int sa;
    VMemType24* dp;
    int da;

    sp = (const unsigned char*)sprite->Frames + gx + gy * sprite->Width - 1;
    gp = sp + sprite->Width * h;
    sa = sprite->Width - w;
    dp = VideoMemory24 + x + y * VideoWidth - 1;
    da = VideoWidth - w;
    --w;

#define UNROLL \
    ++dp; \
    p = *++sp; \
    if (p != 255) { \
	if (p < NumFontColors) { \
	    *dp = FontPixels24[p]; \
	} else { \
	    *dp = ((VMemType24*)sprite->Pixels)[p]; \
	} \
    }

    while (sp < gp) {
	lp = sp + w;
	while (sp < lp) {		// loop with unroll
	    UNROLL;
	    UNROLL;
	}
	if (sp <= lp) {
	    UNROLL;
	}
	sp += sa;
	dp += da;
    }
#undef UNROLL
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
    int gx, int gy, int w, int h, int x, int y)
{
    int p;
    const unsigned char* sp;
    const unsigned char* lp;
    const unsigned char* gp;
    int sa;
    VMemType32* dp;
    int da;

    sp = (const unsigned char*)sprite->Frames + gx + gy * sprite->Width - 1;
    gp = sp + sprite->Width * h;
    sa = sprite->Width - w;
    dp = VideoMemory32 + x + y * VideoWidth - 1;
    da = VideoWidth - w;
    --w;

#define UNROLL \
    ++dp; \
    p = *++sp; \
    if (p != 255) { \
	if (p < NumFontColors) { \
	    *dp = FontPixels32[p]; \
	} else { \
	    *dp = ((VMemType32*)sprite->Pixels)[p]; \
	} \
    }

    while (sp < gp) {
	lp = sp + w;
	while (sp < lp) {		// loop with unroll
	    UNROLL;
	    UNROLL;
	}
	if (sp <= lp) {
	    UNROLL;
	}
	sp += sa;
	dp += da;
    }
#undef UNROLL
}

#ifdef USE_OPENGL
/**
**	Draw character with current color.
**
**	@param sprite	Pointer to object
**	@param gx	X offset into object
**	@param gy	Y offset into object
**	@param w	width to display
**	@param h	height to display
**	@param x	X screen position
**	@param y	Y screen position
*/
local void VideoDrawCharOpenGL(const Graphic* sprite,
    int gx, int gy, int w, int h, int x, int y)
{
    Palette c;
    int i;

    glDisable(GL_TEXTURE_2D);

    for (i = 0; i < NumFontColors; ++i) {
	//c = FontPixels[i];
	memcpy(&c, FontPixels + i, sizeof(Palette));
	glColor3ub(c.r, c.g, c.b);
	glRasterPos2i(x, VideoHeight - y - h);
	glBitmap(FontBitmapWidths[CurrentFont] * 8, h,
	    0.0f, 0.0f, 0.0f, 0.0f,
	    FontBitmaps[CurrentFont][i] + (gy + Fonts[CurrentFont].Height - h) * FontBitmapWidths[CurrentFont]);
    }

    glEnable(GL_TEXTURE_2D);
}
#endif

/**
**	FIXME: docu
*/
local const VMemType* GetFontColorMapping(char* color)
{
    FontColorMapping* fcm;

    fcm = FontColorMappings;
    while (fcm) {
	if (!strcmp(fcm->Color, color)) {
	    return fcm->Pixels;
	}
	fcm = fcm->Next;
    }
    fprintf(stderr, "Font mapping not found: '%s'\n", color);
    ExitFatal(1);
    return NULL;
}

/**
**	Set the default text colors.
**
**	@param normal	Normal text color.
**	@param reverse	Reverse text color.
*/
global void SetDefaultTextColors(char* normal, char* reverse)
{
    DefaultNormalColorIndex = normal;
    DefaultReverseColorIndex = reverse;
    LastTextColor = DefaultTextColor = FontPixels = GetFontColorMapping(normal);
    ReverseTextColor = GetFontColorMapping(reverse);
}

/**
**	Get the default text colors.
**
**	@param normalp	Normal text color pointer.
**	@param reversep	Reverse text color pointer.
*/
global void GetDefaultTextColors(char** normalp, char** reversep)
{
    *normalp = DefaultNormalColorIndex;
    *reversep = DefaultReverseColorIndex;
}

/**
**	Returns the pixel length of a text.
**
**	@param font	Font number.
**	@param text	Text to calculate the length of.
**
**	@return		The length in pixels of the text.
*/
global int VideoTextLength(unsigned font, const unsigned char* text)
{
    int width;
    const unsigned char* s;
    const char* widths;

    widths = Fonts[font].CharWidth;
    for (width = 0, s = text; *s; ++s) {
	if (*s == '~') {
	    if (!*++s) {		// bad formated string
		break;
	    }
	    if (*s != '~') {		// ~~ -> ~
		continue;
	    }
	}
	width += widths[*s - 32] + 1;
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
local void VideoDrawCharClip(const Graphic* graphic, int gx, int gy, int w, int h,
    int x, int y)
{
    int ox;
    int oy;
    int ex;
    CLIP_RECTANGLE_OFS(x, y, w, h, ox, oy, ex);
    VideoDrawChar(graphic, gx + ox, gy + oy, w, h, x, y);
}

/**
**	Draw text with font at x,y clipped/unclipped.
**
**	~	is special prefix.
**	~~	is the ~ character self.
**	~!	print next character reverse.
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
local int DoDrawText(int x, int y, unsigned font, const unsigned char* text,
    int clip)
{
    int w;
    int height;
    int widths;
    const ColorFont* fp;
    const VMemType* rev;
    char* color;
    const unsigned char* p;
    void (*DrawChar)(const Graphic*, int, int, int, int, int, int);

#ifdef USE_OPENGL
    CurrentFont = font;
#endif

    if (clip) {
	DrawChar = VideoDrawCharClip;
    } else {
	DrawChar = VideoDrawChar;
    }

    fp = Fonts + font;
    height = fp->Height;
    for (rev = NULL, widths = 0; *text; ++text) {
	if (*text == '~') {
	    switch (*++text) {
		case '\0':		// wrong formated string.
		    DebugLevel0Fn("oops, format your ~\n");
		    return widths;
		case '~':
		    break;
		case '!':
		    rev=FontPixels;
		    FontPixels = ReverseTextColor;
		    ++text;
		    break;
		case '<':
		    LastTextColor = FontPixels;
		    FontPixels = ReverseTextColor;
		    continue;
		case '>':
		    rev = LastTextColor;	// swap last and current color
		    LastTextColor = FontPixels;
		    FontPixels = rev;
		    continue;

		default:
		    p = text;
		    while (*p && *p!='~') {
			++p;
		    }
		    if (!*p) {
			DebugLevel0Fn("oops, format your ~\n");
			return widths;
		    }
		    color = malloc(p - text + 1);
		    memcpy(color, text, p - text);
		    color[p - text] = '\0';
		    text = p;
		    LastTextColor = FontPixels;
		    FontPixels = GetFontColorMapping(color);
		    free(color);
		    continue;
	    }
	}

	DebugCheck(*text < 32);

	if (*text - 32 >= 0 && height * (*text - 32) < fp->Graphic->Height) {
	    w = fp->CharWidth[*text - 32];
	    DrawChar(fp->Graphic, 0, height * (*text - 32), w, height, x + widths, y);
	} else {
	    w=fp->CharWidth[0];
	    DrawChar(fp->Graphic, 0, height * 0, w, height, x + widths, y);
	}
	widths += w + 1;
	if (rev) {
	    FontPixels = rev;
	    rev = NULL;
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
global int VideoDrawText(int x, int y, unsigned font,
    const unsigned char* text)
{
    return DoDrawText(x, y, font, text, 0);
}

/**
**	Draw text with font at x,y clipped.
**
**	See VideoDrawText.
**
**	@return		The length of the printed text.
*/
global int VideoDrawTextClip(int x, int y, unsigned font,
    const unsigned char* text)
{
    return DoDrawText(x, y, font, text, 1);
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
global int VideoDrawReverseText(int x, int y, unsigned font,
    const unsigned char* text)
{
    int w;

    FontPixels = ReverseTextColor;
    w = VideoDrawText(x, y, font, text);
    FontPixels = DefaultTextColor;

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
global int VideoDrawTextCentered(int x, int y, unsigned font,
    const unsigned char* text)
{
    int dx;

    dx = VideoTextLength(font, text);
    VideoDrawText(x - dx / 2, y, font, text);

    return dx / 2;
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
global int VideoDrawNumberClip(int x, int y, unsigned font, int number)
{
    char buf[sizeof(int) * 10 + 2];

    sprintf(buf, "%d", number);
    return VideoDrawTextClip(x, y, font, buf);
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
global int VideoDrawReverseNumber(int x, int y, unsigned font, int number)
{
    char buf[sizeof(int) * 10 + 2];

    sprintf(buf, "%d", number);
    return VideoDrawReverseText(x, y, font, buf);
}

/**
**	Calculate widths table for a font.
**
// FIXME: ARI: This is runtime and fairly slow!
// FIXME: ARI: Maybe integrate into wartool and load from file!
*/
local void FontMeasureWidths(ColorFont* fp)
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
**	Make font bitmap.
*/
#ifdef USE_OPENGL
local void MakeFontBitmap(Graphic* g, int font)
{
    int i;
    int j;
    int k;
    GLubyte* c;
    GLubyte x;
    const unsigned char* sp;
    int numfonts;
    int n;

    FontBitmapWidths[font] = (g->Width + 7) / 8;

    for (n=0; n < NumFontColors; ++n) {
	if (FontBitmaps[font][n]) {
	    free(FontBitmaps[font][n]);
	}
	FontBitmaps[font][n] = (GLubyte*)malloc(FontBitmapWidths[font] * g->Height);

	sp = (const unsigned char*)g->Frames;
	x = 0;
	numfonts = g->Height / Fonts[font].Height;
	for (k = 0; k < numfonts; ++k) {
	    for (i = 0; i<Fonts[font].Height; ++i) {
		c = FontBitmaps[font][n] + k * Fonts[font].Height * FontBitmapWidths[font] +
		    (Fonts[font].Height - 1 - i) * FontBitmapWidths[font];
		for (j=0; j < g->Width; ++j) {
		    if (*sp == n) {
			x |= 0x1;
		    }
		    ++sp;
		    if ((j & 0x7) == 0x7) {
			*c++ = x;
			x = 0;
		    } else if (j == g->Width - 1) {
			x <<= 0x7 - (j & 0x7);
			*c++ = x;
			x = 0;
		    } else {
			x <<= 1;
		    }
		}
	    }
	}
    }
}
#endif

/**
**	Load all fonts.
*/
global void LoadFonts(void)
{
    unsigned i;
    FontColorMapping* fcm;
    void* pixels;

    //
    //	First select the font drawing procedure.
    //
#ifdef USE_OPENGL
    VideoDrawChar = VideoDrawCharOpenGL;
#else
    switch (VideoBpp) {
	case 8:
	    VideoDrawChar = VideoDrawChar8;
	    break;

	case 15:
	case 16:
	    VideoDrawChar = VideoDrawChar16;
	    break;

	case 24:
	    VideoDrawChar = VideoDrawChar24;
	    break;

	case 32:
	    VideoDrawChar = VideoDrawChar32;
	    break;

	default:
	    DebugLevel0Fn("unsupported %d bpp\n" _C_ VideoBpp);
	    abort();
    }
#endif

    for (i = 0; i<sizeof(Fonts) / sizeof(*Fonts); ++i) {
	if (Fonts[i].File) {
	    ShowLoadProgress("Fonts %s", Fonts[i].File);
	    Fonts[i].Graphic = LoadGraphic(Fonts[i].File);
	    FontMeasureWidths(Fonts + i);
#ifdef USE_OPENGL
	    MakeFontBitmap(Fonts[i].Graphic, i);
#endif
	}
    }

    fcm = FontColorMappings;
    while (fcm) {
	pixels = fcm->Pixels;
	for (i = 0; i < NumFontColors; ++i) {
	    VMemType c;

	    c = VideoMapRGB(fcm->RGB[i].R, fcm->RGB[i].G, fcm->RGB[i].B);

	    switch (VideoBpp) {
		case 8:
		    ((VMemType8*)pixels)[i] = c.D8;
		    break;
		case 15:
		case 16:
		    ((VMemType16*)pixels)[i] = c.D16;
		    break;
		case 24:
		    ((VMemType24*)pixels)[i] = c.D24;
		    break;
		case 32:
		    ((VMemType32*)pixels)[i] = c.D32;
		    break;
	    }
	}
	fcm = fcm->Next;
    }
}

/*----------------------------------------------------------------------------
--	CCL
----------------------------------------------------------------------------*/

/**
**	Font symbol to id.
**
**	@param type	Type of the font (game,small,...)
**
**	@return		Integer as font identifier.
*/
global int CclFontByIdentifier(SCM type)
{
    if (gh_eq_p(type, gh_symbol2scm("game"))) {
	return GameFont;
    } else if (gh_eq_p(type, gh_symbol2scm("small"))) {
	return SmallFont;
    } else if (gh_eq_p(type, gh_symbol2scm("large"))) {
	return LargeFont;
    } else if (gh_eq_p(type, gh_symbol2scm("small-title"))) {
	return SmallTitleFont;
    } else if (gh_eq_p(type, gh_symbol2scm("large-title"))) {
	return LargeTitleFont;
    } else if (gh_eq_p(type, gh_symbol2scm("user1"))) {
	return User1Font;
    } else if (gh_eq_p(type, gh_symbol2scm("user2"))) {
	return User2Font;
    } else if (gh_eq_p(type, gh_symbol2scm("user3"))) {
	return User3Font;
    } else if (gh_eq_p(type, gh_symbol2scm("user4"))) {
	return User4Font;
    } else if (gh_eq_p(type, gh_symbol2scm("user5"))) {
	return User5Font;
    } else {
	errl("Unsupported font tag", type);
    }
    return 0;
}

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
local SCM CclDefineFont(SCM type, SCM file, SCM width, SCM height)
{
    int i;

    i = CclFontByIdentifier(type);
    free(Fonts[i].File);
    VideoSaveFree(Fonts[i].Graphic);
    Fonts[i].Graphic = NULL;
    Fonts[i].File = gh_scm2newstr(file, NULL);
    Fonts[i].Width = gh_scm2int(width);
    Fonts[i].Height = gh_scm2int(height);

    return SCM_UNSPECIFIED;
}

/**
**	Define a font color.
*/
local SCM CclDefineFontColor(SCM list)
{
    SCM value;
    char* color;
    int i;
    FontColorMapping* fcm;
    FontColorMapping** fcmp;

    value = gh_car(list);
    list = gh_cdr(list);

    color = gh_scm2newstr(value,NULL);

    if (!FontColorMappings) {
	FontColorMappings = calloc(sizeof(*FontColorMappings), 1);
	fcm = FontColorMappings;
    } else {
	fcmp = &FontColorMappings;
	while (*fcmp) {
	    if (!strcmp((*fcmp)->Color, color)) {
		fprintf(stderr, "Warning: Redefining color '%s'\n", color);
		free((*fcmp)->Color);
		fcm = *fcmp;
		break;
	    }
	    fcmp = &(*fcmp)->Next;
	}
	*fcmp = calloc(sizeof(*FontColorMappings), 1);
	fcm = *fcmp;
    }
    fcm->Color = color;
    fcm->Next = NULL;

    value = gh_car(list);
    list = gh_cdr(list);

    if (gh_vector_length(value) != NumFontColors * 3) {
	fprintf(stderr, "Wrong vector length\n");
    }
    for (i = 0; i < NumFontColors; ++i) {
	fcm->RGB[i].R = gh_scm2int(gh_vector_ref(value, gh_int2scm(i * 3 + 0)));
	fcm->RGB[i].G = gh_scm2int(gh_vector_ref(value, gh_int2scm(i * 3 + 1)));
	fcm->RGB[i].B = gh_scm2int(gh_vector_ref(value, gh_int2scm(i * 3 + 2)));
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
    gh_new_procedure4_0("define-font", CclDefineFont);
    gh_new_procedureN("define-font-color", CclDefineFontColor);

    //gh_new_procedure2_0("default-text-colors", CclDefaultTextColors);
    //gh_new_procedure1_0("text-length", CclTextLength);
    //gh_new_procedure4_0("draw-text", CclDrawText);
    //gh_new_procedure4_0("draw-reverse-text", CclDrawReverseText);
    //gh_new_procedure4_0("draw-text-centered", CclDrawTextCentered);
    //gh_new_procedure4_0("draw-reverse-text-centered", CclDrawReverseTextCentered);
    //gh_new_procedure4_0("draw-number", CclDrawNumber);
    //gh_new_procedure4_0("draw-reverse-number", CclDrawReverseNumber);
}

/**
**	Cleanup the font module.
*/
global void CleanFonts(void)
{
    unsigned i;
    FontColorMapping* fcm;
    FontColorMapping* temp;

    for (i = 0; i < sizeof(Fonts) / sizeof(*Fonts); ++i) {
	free(Fonts[i].File);
	VideoSaveFree(Fonts[i].Graphic);
	Fonts[i].File = NULL;
	Fonts[i].Graphic = NULL;
    }

    fcm = FontColorMappings;
    while (fcm) {
	temp = fcm->Next;
	free(fcm->Color);
	free(fcm);
	fcm = temp;
    }
    FontColorMappings = NULL;
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
    return Fonts[font].Graphic != 0;
}

//@}
