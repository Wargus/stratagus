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
//	(c) Copyright 1998-2003 by Lutz Sammer, Jimmy Salmon, Nehal Mistry
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
--		Includes
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
--		Variables
----------------------------------------------------------------------------*/

#define NumFontColors 7

#ifdef USE_SDL_SURFACE
	/// Font color mapping
typedef struct _font_color_mapping_ {
	char* ColorName;						/// Font color name

	SDL_Color Color[NumFontColors];		/// Array of hardware dependent pixels
	struct _font_color_mapping_* Next;		/// Next pointer
} FontColorMapping;

local FontColorMapping* FontColor;
#else
	/// Font color mapping
typedef struct _font_color_mapping_ {
	char* Color;						/// Font color
	struct {
		int R;
		int G;
		int B;
	} RGB[NumFontColors];
	VMemType Pixels[NumFontColors];		/// Array of hardware dependent pixels
	struct _font_color_mapping_* Next;		/// Next pointer
} FontColorMapping;

local const VMemType* FontPixels;				/// Font pixels
#define FontPixels8		(&FontPixels->D8)		/// font pixels  8bpp
#define FontPixels16		(&FontPixels->D16)		/// font pixels 16bpp
#define FontPixels24		(&FontPixels->D24)		/// font pixels 24bpp
#define FontPixels32		(&FontPixels->D32)		/// font pixels 32bpp

#endif

	/// Font color mappings
local FontColorMapping* FontColorMappings;

/**
**		Fonts table
**
**		Define the font files, sizes.
*/
local ColorFont Fonts[MaxFonts];

#ifdef USE_SDL_SURFACE
	/// Last text color
local FontColorMapping* LastTextColor;
	/// Default text color
local FontColorMapping* DefaultTextColor;
	/// Reverse text color
local FontColorMapping* ReverseTextColor;
	/// Default normal color index
local char* DefaultNormalColorIndex;
	/// Default reverse color index
local char* DefaultReverseColorIndex;
#else
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
#endif

	/// Draw character with current video depth.
#ifdef USE_SDL_SURFACE
local void VideoDrawChar(const Graphic*, int, int, int, int, int, int);
#else
local void (*VideoDrawChar)(const Graphic*, int, int, int, int, int, int);
#endif

#ifdef USE_OPENGL
	/// Font bitmaps
local GLubyte* FontBitmaps[MaxFonts][NumFontColors];
	/// Font bitmap widths
local int FontBitmapWidths[MaxFonts];
	/// Current font
local int CurrentFont;
#endif

/**
**		FIXME: should use the names of the real fonts.
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
--		Functions
----------------------------------------------------------------------------*/

// FIXME: should use RLE encoded fonts, not color key fonts.

#ifdef USE_SDL_SURFACE
local void VideoDrawChar(const Graphic* sprite,
	int gx, int gy, int w, int h, int x, int y)
{
	SDL_Rect srect;
	SDL_Rect drect;

	srect.x = gx;
	srect.y = gy;
	srect.w = w;
	srect.h = h;

	drect.x = x;
	drect.y = y;

	SDL_SetColors(sprite->Surface, FontColor->Color, 0, NumFontColors);

	SDL_BlitSurface(sprite->Surface, &srect, TheScreen, &drect);
}

#else

/**
**		Draw character with current color into 8bit video memory.
**
**		@param sprite		Pointer to object
**		@param gx		X offset into object
**		@param gy		Y offset into object
**		@param w		width to display
**		@param h		height to display
**		@param x		X screen position
**		@param y		Y screen position
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
		while (sp < lp) {				// loop with unroll
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
**		Draw character with current color into 16bit video memory.
**
**		@param sprite		Pointer to object
**		@param gx		X offset into object
**		@param gy		Y offset into object
**		@param w		width to display
**		@param h		height to display
**		@param x		X screen position
**		@param y		Y screen position
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
		while (sp < lp) {				// loop with unroll
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
**		Draw character with current color into 24bit video memory.
**
**		@param sprite		Pointer to object
**		@param gx		X offset into object
**		@param gy		Y offset into object
**		@param w		width to display
**		@param h		height to display
**		@param x		X screen position
**		@param y		Y screen position
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
		while (sp < lp) {				// loop with unroll
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
**		Draw character with current color into 32bit video memory.
**
**		@param sprite		Pointer to object
**		@param gx		X offset into object
**		@param gy		Y offset into object
**		@param w		width to display
**		@param h		height to display
**		@param x		X screen position
**		@param y		Y screen position
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
		while (sp < lp) {				// loop with unroll
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
#endif

#ifdef USE_OPENGL
/**
**		Draw character with current color.
**
**		@param sprite		Pointer to object
**		@param gx		X offset into object
**		@param gy		Y offset into object
**		@param w		width to display
**		@param h		height to display
**		@param x		X screen position
**		@param y		Y screen position
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

#ifdef USE_SDL_SURFACE
local FontColorMapping* GetFontColorMapping(char* color)
{
	FontColorMapping *fcm;

	fcm = FontColorMappings;
	while (fcm) {
		if (!strcmp(fcm->ColorName, color)) {
			return fcm;
		}
		fcm = fcm->Next;
	}
	fprintf(stderr, "Font mapping not found: '%s'\n", color);
	ExitFatal(1);
	return NULL;
}
#else
/**
**		FIXME: docu
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
#endif

/**
**		Set the default text colors.
**
**		@param normal		Normal text color.
**		@param reverse		Reverse text color.
*/
global void SetDefaultTextColors(char* normal, char* reverse)
{
	DefaultNormalColorIndex = normal;
	DefaultReverseColorIndex = reverse;
#ifdef USE_SDL_SURFACE
	LastTextColor = DefaultTextColor = FontColor = GetFontColorMapping(normal);
#else
	LastTextColor = DefaultTextColor = FontPixels = GetFontColorMapping(normal);
#endif
	ReverseTextColor = GetFontColorMapping(reverse);
}

/**
**		Get the default text colors.
**
**		@param normalp		Normal text color pointer.
**		@param reversep		Reverse text color pointer.
*/
global void GetDefaultTextColors(char** normalp, char** reversep)
{
	*normalp = DefaultNormalColorIndex;
	*reversep = DefaultReverseColorIndex;
}

/**
**		Returns the pixel length of a text.
**
**		@param font		Font number.
**		@param text		Text to calculate the length of.
**
**		@return				The length in pixels of the text.
*/
global int VideoTextLength(unsigned font, const unsigned char* text)
{
	int width;
	const unsigned char* s;
	const char* widths;

	widths = Fonts[font].CharWidth;
	for (width = 0, s = text; *s; ++s) {
		if (*s == '~') {
			if (!*++s) {				// bad formated string
				break;
			}
			if (*s != '~') {				// ~~ -> ~
				continue;
			}
		}
		width += widths[*s - 32] + 1;
	}
	return width;
}

/**
**		Returns the height of the font.
**
**		@param font		Font number.
**
**		@return				The height of the font.
*/
global int VideoTextHeight(unsigned font)
{
	return Fonts[font].Height;
}

/**
**		Draw character with current color clipped into 8 bit framebuffer.
**
**		@param graphic		Pointer to object
**		@param gx		X offset into object
**		@param gy		Y offset into object
**		@param w		width to display
**		@param h		height to display
**		@param x		X screen position
**		@param y		Y screen position
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
**		Draw text with font at x,y clipped/unclipped.
**
**		~		is special prefix.
**		~~		is the ~ character self.
**		~!		print next character reverse.
**		~<		start reverse.
**		~>		switch back to last used color.
**
**		@param x		X screen position
**		@param y		Y screen position
**		@param font		Font number
**		@param text		Text to be displayed.
**		@param clip		Flag if TRUE clip, otherwise not.
**
**		@return				The length of the printed text.
*/
local int DoDrawText(int x, int y, unsigned font, const unsigned char* text,
	int clip)
{
	int w;
	int height;
	int widths;
	const ColorFont* fp;
#ifdef USE_SDL_SURFACE
	FontColorMapping* rev;
#else
	const VMemType* rev;
#endif
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
				case '\0':				// wrong formated string.
					DebugLevel0Fn("oops, format your ~\n");
					return widths;
				case '~':
					break;
				case '!':
#ifdef USE_SDL_SURFACE
					rev = FontColor;
					FontColor = ReverseTextColor;
#else
					rev = FontPixels;
					FontPixels = ReverseTextColor;
#endif

					++text;
					break;
				case '<':
#ifdef USE_SDL_SURFACE
					LastTextColor = FontColor;
					FontColor = ReverseTextColor;
#else
					LastTextColor = FontPixels;
					FontPixels = ReverseTextColor;
#endif
					continue;
				case '>':
					rev = LastTextColor;		// swap last and current color
#ifdef USE_SDL_SURFACE
					LastTextColor = FontColor;
					FontColor = rev;
#else
					LastTextColor = FontPixels;
					FontPixels = rev;
#endif
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
#ifdef USE_SDL_SURFACE
					printf("NBNNNNNNNNNNNNNNNNNNNNNN\n");
					LastTextColor = FontColor;
					FontColor = GetFontColorMapping(color);
#else
					LastTextColor = FontPixels;
					FontPixels = GetFontColorMapping(color);
#endif
					free(color);
					continue;
			}
		}
#ifdef USE_SDL_SURFACE

#endif

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
#ifdef USE_SDL_SURFACE
			FontColor = rev;
#else
			FontPixels = rev;
#endif
			rev = NULL;
		}
	}

	return widths;
}

/**
**		Draw text with font at x,y unclipped.
**
**		~		is special prefix.
**		~~		is the ~ character self.
**		~!		print next character reverse.
**		~n		0123456789abcdef print text in color 1-16.
**		~<		start reverse.
**		~>		switch back to last used color.
**
**		@param x		X screen position
**		@param y		Y screen position
**		@param font		Font number
**		@param text		Text to be displayed.
**
**		@return				The length of the printed text.
*/
global int VideoDrawText(int x, int y, unsigned font,
	const unsigned char* text)
{
	return DoDrawText(x, y, font, text, 0);
}

/**
**		Draw text with font at x,y clipped.
**
**		See VideoDrawText.
**
**		@return				The length of the printed text.
*/
global int VideoDrawTextClip(int x, int y, unsigned font,
	const unsigned char* text)
{
	return DoDrawText(x, y, font, text, 1);
}

/**
**		Draw reverse text with font at x,y unclipped.
**
**		@see VideoDrawText for full description.
**
**		@param x		X screen position
**		@param y		Y screen position
**		@param font		Font number
**		@param text		Text to be displayed.
**
**		@return				The length of the printed text.
*/
global int VideoDrawReverseText(int x, int y, unsigned font,
	const unsigned char* text)
{
	int w;

#ifdef USE_SDL_SURFACE
	FontColor = ReverseTextColor;
	w = VideoDrawText(x, y, font, text);
	FontColor = DefaultTextColor;
#else
	FontPixels = ReverseTextColor;
	w = VideoDrawText(x, y, font, text);
	FontPixels = DefaultTextColor;
#endif

	return w;
}

/**
**		Draw text with font at x,y centered.
**
**		@see VideoDrawText for full description.
**
**		@param x		X screen position
**		@param y		Y screen position
**		@param font		Font number
**		@param text		Text to be displayed.
**
**		@return				The length of the printed text.
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
**		Draw number with font at x,y unclipped.
**
**		@param x		X screen position
**		@param y		Y screen position
**		@param font		Font number
**		@param number		Number to be displayed.
**
**		@return				The length of the printed text.
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
**		Draw number with font at x,y clipped.
**
**		@param x		X screen position
**		@param y		Y screen position
**		@param font		Font number
**		@param number		Number to be displayed.
**
**		@return				The length of the printed text.
*/
global int VideoDrawNumberClip(int x, int y, unsigned font, int number)
{
	char buf[sizeof(int) * 10 + 2];

	sprintf(buf, "%d", number);
	return VideoDrawTextClip(x, y, font, buf);
}

/**
**		Draw reverse number with font at x,y unclipped.
**
**		@param x		X screen position
**		@param y		Y screen position
**		@param font		Font number
**		@param number		Number to be displayed.
**
**		@return				The length of the printed text.
*/
global int VideoDrawReverseNumber(int x, int y, unsigned font, int number)
{
	char buf[sizeof(int) * 10 + 2];

	sprintf(buf, "%d", number);
	return VideoDrawReverseText(x, y, font, buf);
}

#ifdef USE_SDL_SURFACE
local void FontMeasureWidths(ColorFont* fp)
{
	// FIXME: todo.. can this be optimized?
	int y;
	const unsigned char* sp;
	const unsigned char* lp;
	const unsigned char* gp;

	for (y = 1; y < 207; ++y) {
		fp->CharWidth[y] = 0;
	}

	fp->CharWidth[0] = fp->Width / 2;		// a reasonable value for SPACE

	for (y = 1; y < 207; ++y) {
		sp = (const unsigned char *)fp->Graphic->Surface->pixels +
			y * fp->Height * fp->Graphic->Width - 1;
		gp = sp + fp->Graphic->Width * fp->Height;
		// Bail out if no letters left
		if (gp >= ((const unsigned char *)fp->Graphic->Surface->pixels +
				fp->Graphic->Width * fp->Graphic->Height)) {
			break;
		}
		while (sp < gp) {
			lp = sp + fp->Graphic->Width - 1;
			for (; sp < lp; --lp) {
				if (*lp != 255) {
					if (lp - sp > fp->CharWidth[y]) {		// max width
						fp->CharWidth[y] = lp - sp;
					}
				}
			}
			sp += fp->Graphic->Width;
		}

	}
}
#else
/**
**		Calculate widths table for a font.
**
// FIXME: ARI: This is runtime and fairly slow!
// FIXME: ARI: Maybe integrate into wartool and load from file!
*/
local void FontMeasureWidths(ColorFont* fp)
{
	int y;
	const unsigned char* sp;
	const unsigned char* lp;
	const unsigned char* gp;

	for (y = 1; y < 207; ++y) {
		fp->CharWidth[y] = 0;
	}

	for (y = 1; y < 207; ++y) {
		sp = (const unsigned char *)fp->Graphic->Frames +
			y * fp->Height * fp->Graphic->Width - 1;
		gp = sp + fp->Graphic->Width * fp->Height;
		//		Bail out cause there are no letters left
		if (gp >= ((const unsigned char *)fp->Graphic->Frames +
				fp->Graphic->Width * fp->Graphic->Height)) {
			break;
		}
		while (sp < gp) {
			lp = sp + fp->Graphic->Width - 1;
			for (; sp < lp; --lp) {
				if (*lp != 255) {
					if (lp - sp > fp->CharWidth[y]) {		// max width
						fp->CharWidth[y] = lp - sp;
					}
				}
			}
			sp += fp->Graphic->Width;
		}
	}
	fp->CharWidth[0] = fp->Width / 2;		// a reasonable value for SPACE
}
#endif

/**
**		Make font bitmap.
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
**		Load all fonts.
*/
global void LoadFonts(void)
{
	unsigned i;
	FontColorMapping* fcm;
#ifdef USE_SDL_SURFACE
	SDL_Color* color;
#else
	void* pixels;
#endif

	//
	//		First select the font drawing procedure.
	//
#ifdef USE_OPENGL
	VideoDrawChar = VideoDrawCharOpenGL;
#else
#ifdef USE_SDL_SURFACE
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
#ifdef USE_SDL_SURFACE

	while (fcm) {
		color = fcm->Color;
		for (i = 0; i < NumFontColors; ++i) {
///			SDL_Color c;
			// FIXME: todo
//			c = VideoMapRGB(fcm->RGB[i].R, fcm->RGB[i].G, fcm->RGB[i].B);
///			c = VideoMapRGB(fcm->Color[i].r, fcm->Color[i].g, fcm->Color[i].b);

///			color[i] = VideoMapRGB(c.r, c.g, c.b);
			color[i].r = fcm->Color[i].r;
			color[i].g = fcm->Color[i].g;
			color[i].b = fcm->Color[i].b;
		}
		fcm = fcm->Next;
	}

#else
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
#endif
}

/*----------------------------------------------------------------------------
--		CCL
----------------------------------------------------------------------------*/

/**
**		Font symbol to id.
**
**		@param type		Type of the font (game,small,...)
**
**		@return				Integer as font identifier.
*/
#if defined(USE_GUILE) || defined(USE_SIOD)
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
#elif defined(USE_LUA)
global int CclFontByIdentifier(const char* type)
{
	if (!strcmp(type, "game")) {
		return GameFont;
	} else if (!strcmp(type, "small")) {
		return SmallFont;
	} else if (!strcmp(type, "large")) {
		return LargeFont;
	} else if (!strcmp(type, "small-title")) {
		return SmallTitleFont;
	} else if (!strcmp(type, "large-title")) {
		return LargeTitleFont;
	} else if (!strcmp(type, "user1")) {
		return User1Font;
	} else if (!strcmp(type, "user2")) {
		return User2Font;
	} else if (!strcmp(type, "user3")) {
		return User3Font;
	} else if (!strcmp(type, "user4")) {
		return User4Font;
	} else if (!strcmp(type, "user5")) {
		return User5Font;
	} else {
		fprintf(stderr, "Unsupported font tag: %s", type);
		exit(1);
	}
	return 0;
}
#endif

/**
**		Define the used fonts.
**
**		@param type		Type of the font (game,small,...)
**		@param file		File name of the graphic file
**		@param width		Font width in pixels
**		@param height		Font height in pixels
**
**		@todo		make the font name functions more general, support more fonts.
*/
#if defined(USE_GUILE) || defined(USE_SIOD)
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
#elif defined(USE_LUA)
local int CclDefineFont(lua_State* l)
{
	int i;

	if (lua_gettop(l) != 4) {
		lua_pushstring(l, "incorrect argument");
		lua_error(l);
	}
	i = CclFontByIdentifier(LuaToString(l, 1));
	free(Fonts[i].File);
	VideoSaveFree(Fonts[i].Graphic);
	Fonts[i].Graphic = NULL;
	Fonts[i].File = strdup(LuaToString(l, 2));
	Fonts[i].Width = LuaToNumber(l, 3);
	Fonts[i].Height = LuaToNumber(l, 4);

	return 0;
}
#endif

/**
**		Define a font color.
*/
#if defined(USE_GUILE) || defined(USE_SIOD)
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
#ifdef USE_SDL_SURFACE
			if (!strcmp((*fcmp)->ColorName, color)) {
				fprintf(stderr, "Warning: Redefining color '%s'\n", color);
				free((*fcmp)->ColorName);
				fcm = *fcmp;
				break;
			}
#else
			if (!strcmp((*fcmp)->Color, color)) {
				fprintf(stderr, "Warning: Redefining color '%s'\n", color);
				free((*fcmp)->Color);
				fcm = *fcmp;
				break;
			}
#endif
			fcmp = &(*fcmp)->Next;
		}
		*fcmp = calloc(sizeof(*FontColorMappings), 1);
		fcm = *fcmp;
	}
#ifdef USE_SDL_SURFACE
	fcm->ColorName = color;
#else
	fcm->Color = color;
#endif
	fcm->Next = NULL;

	value = gh_car(list);
	list = gh_cdr(list);

	if (gh_vector_length(value) != NumFontColors * 3) {
		fprintf(stderr, "Wrong vector length\n");
	}
	for (i = 0; i < NumFontColors; ++i) {
#ifdef USE_SDL_SURFACE
		fcm->Color[i].r = gh_scm2int(gh_vector_ref(value, gh_int2scm(i * 3 + 0)));
		fcm->Color[i].g = gh_scm2int(gh_vector_ref(value, gh_int2scm(i * 3 + 1)));
		fcm->Color[i].b = gh_scm2int(gh_vector_ref(value, gh_int2scm(i * 3 + 2)));
#else
		fcm->RGB[i].R = gh_scm2int(gh_vector_ref(value, gh_int2scm(i * 3 + 0)));
		fcm->RGB[i].G = gh_scm2int(gh_vector_ref(value, gh_int2scm(i * 3 + 1)));
		fcm->RGB[i].B = gh_scm2int(gh_vector_ref(value, gh_int2scm(i * 3 + 2)));
#endif
	}

	return SCM_UNSPECIFIED;
}
#elif defined(USE_LUA)
local int CclDefineFontColor(lua_State* l)
{
	char* color;
	int i;
	FontColorMapping* fcm;
	FontColorMapping** fcmp;

	if (lua_gettop(l) != 2) {
		lua_pushstring(l, "incorrect argument");
		lua_error(l);
	}
	color = strdup(LuaToString(l, 1));

	if (!FontColorMappings) {
		FontColorMappings = calloc(sizeof(*FontColorMappings), 1);
		fcm = FontColorMappings;
	} else {
		fcmp = &FontColorMappings;
		while (*fcmp) {
#ifdef USE_SDL_SURFACE
			if (!strcmp((*fcmp)->ColorName, color)) {
#else
			if (!strcmp((*fcmp)->Color, color)) {
#endif
				fprintf(stderr, "Warning: Redefining color '%s'\n", color);
#ifdef USE_SDL_SURFACE
				free((*fcmp)->ColorName);
#else
				free((*fcmp)->Color);
#endif
				fcm = *fcmp;
				break;
			}
			fcmp = &(*fcmp)->Next;
		}
		*fcmp = calloc(sizeof(*FontColorMappings), 1);
		fcm = *fcmp;
	}
#ifdef USE_SDL_SURFACE
	fcm->ColorName = color;
#else
	fcm->Color = color;
#endif
	fcm->Next = NULL;

	if (luaL_getn(l, 2) != NumFontColors * 3) {
		fprintf(stderr, "Wrong vector length\n");
	}
	for (i = 0; i < NumFontColors; ++i) {
		lua_rawgeti(l, 2, i * 3 + 1);
#ifdef USE_SDL_SURFACE
		fcm->Color[i].r = LuaToNumber(l, -1);
#else
		fcm->RGB[i].R = LuaToNumber(l, -1);
#endif
		lua_pop(l, 1);
		lua_rawgeti(l, 2, i * 3 + 2);
#ifdef USE_SDL_SURFACE
		fcm->Color[i].g = LuaToNumber(l, -1);
#else
		fcm->RGB[i].G = LuaToNumber(l, -1);
#endif
		lua_pop(l, 1);
		lua_rawgeti(l, 2, i * 3 + 3);
#ifdef USE_SDL_SURFACE
		fcm->Color[i].b = LuaToNumber(l, -1);
#else
		fcm->RGB[i].B = LuaToNumber(l, -1);
#endif
		lua_pop(l, 1);
	}

	return 0;
}
#endif

/**
**		Register CCL features for fonts.
**
**		@todo FIXME: Make the remaining functions accessable from CCL.
*/
global void FontsCclRegister(void)
{
#if defined(USE_GUILE) || defined(USE_SIOD)
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
#elif defined(USE_LUA)
	lua_register(Lua, "DefineFont", CclDefineFont);
	lua_register(Lua, "DefineFontColor", CclDefineFontColor);
#endif
}

/**
**		Cleanup the font module.
*/
global void CleanFonts(void)
{
	unsigned i;
#ifndef USE_SDL_SURFACE
	FontColorMapping* fcm;
	FontColorMapping* temp;
#endif

	for (i = 0; i < sizeof(Fonts) / sizeof(*Fonts); ++i) {
		free(Fonts[i].File);
		VideoSaveFree(Fonts[i].Graphic);
		Fonts[i].File = NULL;
		Fonts[i].Graphic = NULL;
	}

#ifndef USE_SDL_SURFACE
	fcm = FontColorMappings;
	while (fcm) {
		temp = fcm->Next;
		free(fcm->Color);
		free(fcm);
		fcm = temp;
	}
#endif
	FontColorMappings = NULL;
}

/**
**		Check if font is already loaded.
**
**		@param font		Font number
**
**		@return				True if loaded, false otherwise.
*/
global int IsFontLoaded(unsigned font)
{
	return Fonts[font].Graphic != 0;
}

//@}
