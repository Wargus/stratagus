//       _________ __                 __
//      /   _____//  |_____________ _/  |______     ____  __ __  ______
//      \_____  \\   __\_  __ \__  \\   __\__  \   / ___\|  |  \/  ___/
//      /        \|  |  |  | \// __ \|  |  / __ \_/ /_/  >  |  /\___ |
//     /_______  /|__|  |__|  (____  /__| (____  /\___  /|____//____  >
//             \/                  \/          \//_____/            \/
//  ______________________                           ______________________
//                        T H E   W A R   B E G I N S
//         Stratagus - A free fantasy real time strategy game engine
//
/**@name font.c - The color fonts. */
//
//      (c) Copyright 1998-2004 by Lutz Sammer, Jimmy Salmon, Nehal Mistry
//
//      This program is free software; you can redistribute it and/or modify
//      it under the terms of the GNU General Public License as published by
//      the Free Software Foundation; only version 2 of the License.
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
//      $Id$

//@{

/*----------------------------------------------------------------------------
--  Includes
----------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "stratagus.h"
#include "video.h"
#include "font.h"
#include "script.h"

#include "intern_video.h"

/*----------------------------------------------------------------------------
--  Variables
----------------------------------------------------------------------------*/

#define NumFontColors 7

	/// Font color mapping
typedef struct _font_color_mapping_ {
	char* ColorName;                        /// Font color name
	SDL_Color Color[NumFontColors];         /// Array of colors
	struct _font_color_mapping_* Next;      /// Next pointer
} FontColorMapping;

local FontColorMapping* FontColor;

	/// Font color mappings
local FontColorMapping* FontColorMappings;

	/// Font mapping
typedef struct _font_mapping_ {
	char* Ident;                            /// Font name
	int Font;                               /// Ident number
	struct _font_mapping_* Next;            /// Next pointer
} FontMapping;

local FontMapping* FontMappings;

/**
**  Fonts table
**
**  Define the font files, sizes.
*/
local ColorFont Fonts[MaxFonts];

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

	/// Draw character with current video depth.
local void VideoDrawChar(const Graphic*, int, int, int, int, int, int);

#ifdef USE_OPENGL
	/// Font bitmaps
local GLubyte* FontBitmaps[MaxFonts][NumFontColors];
	/// Font bitmap widths
local int FontBitmapWidths[MaxFonts];
	/// Current font
local int CurrentFont;
#endif

/*----------------------------------------------------------------------------
--  Functions
----------------------------------------------------------------------------*/

/**
**  Draw character with current color.
**
**  @param sprite  Pointer to object
**  @param gx      X offset into object
**  @param gy      Y offset into object
**  @param w       width to display
**  @param h       height to display
**  @param x       X screen position
**  @param y       Y screen position
*/
#ifndef USE_OPENGL
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
local void VideoDrawChar(const Graphic* sprite,
	int gx, int gy, int w, int h, int x, int y)
{
	SDL_Color* c;
	int i;
	int fy;

	glDisable(GL_TEXTURE_2D);

	if (y + h >= VideoHeight) {
		h = VideoHeight - y - 1;
	}
	fy = gy / Fonts[CurrentFont].Height * Fonts[CurrentFont].Height;
	fy = fy + Fonts[CurrentFont].Height - (gy - fy) - h;
	for (i = 0; i < NumFontColors; ++i) {
		c = FontColor->Color + i;
		glColor3ub(c->r, c->g, c->b);
		glRasterPos2i(x, y + h);
		glBitmap(FontBitmapWidths[CurrentFont] * 8, h,
			0.0f, 0.0f, 0.0f, 0.0f,
			FontBitmaps[CurrentFont][i] +
				fy * FontBitmapWidths[CurrentFont]);
	}

	glEnable(GL_TEXTURE_2D);
}
#endif

/**
**  FIXME: docu
*/
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
	fprintf(stderr, "Font color mapping not found: '%s'\n", color);
	ExitFatal(1);
	return NULL;
}

/**
**  Set the default text colors.
**
**  @param normal   Normal text color.
**  @param reverse  Reverse text color.
*/
global void SetDefaultTextColors(char* normal, char* reverse)
{
	DefaultNormalColorIndex = normal;
	DefaultReverseColorIndex = reverse;
	LastTextColor = DefaultTextColor = FontColor = GetFontColorMapping(normal);
	ReverseTextColor = GetFontColorMapping(reverse);
}

/**
**  Get the default text colors.
**
**  @param normalp   Normal text color pointer.
**  @param reversep  Reverse text color pointer.
*/
global void GetDefaultTextColors(char** normalp, char** reversep)
{
	*normalp = DefaultNormalColorIndex;
	*reversep = DefaultReverseColorIndex;
}

/**
**  Returns the pixel length of a text.
**
**  @param font  Font number.
**  @param text  Text to calculate the length of.
**
**  @return      The length in pixels of the text.
*/
global int VideoTextLength(unsigned font, const unsigned char* text)
{
	int width;
	const unsigned char* s;
	const char* widths;
	int isformat;

	widths = Fonts[font].CharWidth;
	isformat = 0;
	for (width = 0, s = text; *s; ++s) {
		if (*s == '~') {
			if (!*++s) {  // bad formated string
				break;
			}
			if (*s == '<' || *s == '>' || *s == '!') {
				continue;
			}
			if (*s != '~') {				// ~~ -> ~
				isformat = !isformat;
				continue;
			}
		}
		if (!isformat) {
			width += widths[*s - 32] + 1;
		}
	}
	return width;
}

/**
**  Returns the height of the font.
**
**  @param font  Font number.
**
**  @return      The height of the font.
*/
global int VideoTextHeight(unsigned font)
{
	return Fonts[font].Height;
}

/**
**  Draw character with current color clipped into 8 bit framebuffer.
**
**  @param graphic  Pointer to object
**  @param gx       X offset into object
**  @param gy       Y offset into object
**  @param w        width to display
**  @param h        height to display
**  @param x        X screen position
**  @param y        Y screen position
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
**  Draw text with font at x,y clipped/unclipped.
**
**  ~		is special prefix.
**  ~~		is the ~ character self.
**  ~!		print next character reverse.
**  ~<		start reverse.
**  ~>		switch back to last used color.
**
**  @param x     X screen position
**  @param y     Y screen position
**  @param font  Font number
**  @param text  Text to be displayed.
**  @param clip  Flag if TRUE clip, otherwise not.
**
**  @return      The length of the printed text.
*/
local int DoDrawText(int x, int y, unsigned font, const unsigned char* text,
	int clip)
{
	int w;
	int height;
	int widths;
	const ColorFont* fp;
	FontColorMapping* rev;
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
				case '\0':  // wrong formated string.
					DebugPrint("oops, format your ~\n");
					return widths;
				case '~':
					break;
				case '!':
					rev = FontColor;
					FontColor = ReverseTextColor;

					++text;
					break;
				case '<':
					LastTextColor = FontColor;
					FontColor = ReverseTextColor;
					continue;
				case '>':
					rev = LastTextColor;  // swap last and current color
					LastTextColor = FontColor;
					FontColor = rev;
					continue;

				default:
					p = text;
					while (*p && *p!='~') {
						++p;
					}
					if (!*p) {
						DebugPrint("oops, format your ~\n");
						return widths;
					}
					color = malloc(p - text + 1);
					memcpy(color, text, p - text);
					color[p - text] = '\0';
					text = p;
					LastTextColor = FontColor;
					FontColor = GetFontColorMapping(color);
					free(color);
					continue;
			}
		}

		Assert(*text >= 32);

		if (*text - 32 >= 0 && height * (*text - 32) < fp->Graphic->Height) {
			w = fp->CharWidth[*text - 32];
			DrawChar(fp->Graphic, 0, height * (*text - 32), w, height, x + widths, y);
		} else {
			w = fp->CharWidth[0];
			DrawChar(fp->Graphic, 0, height * 0, w, height, x + widths, y);
		}
		widths += w + 1;
		if (rev) {
			FontColor = rev;
			rev = NULL;
		}
	}

	return widths;
}

/**
**  Draw text with font at x,y unclipped.
**
**  ~		is special prefix.
**  ~~		is the ~ character self.
**  ~!		print next character reverse.
**  ~n		0123456789abcdef print text in color 1-16.
**  ~<		start reverse.
**  ~>		switch back to last used color.
**
**  @param x     X screen position
**  @param y     Y screen position
**  @param font  Font number
**  @param text  Text to be displayed.
**
**  @return      The length of the printed text.
*/
global int VideoDrawText(int x, int y, unsigned font,
	const unsigned char* text)
{
	return DoDrawText(x, y, font, text, 0);
}

/**
**  Draw text with font at x,y clipped.
**
**  See VideoDrawText.
**
**  @param x     X screen position
**  @param y     Y screen position
**  @param font  Font number
**  @param text  Text to be displayed.
**
**  @return      The length of the printed text.
*/
global int VideoDrawTextClip(int x, int y, unsigned font,
	const unsigned char* text)
{
	return DoDrawText(x, y, font, text, 1);
}

/**
**  Draw reverse text with font at x,y unclipped.
**
**  @see VideoDrawText for full description.
**
**  @param x     X screen position
**  @param y     Y screen position
**  @param font  Font number
**  @param text  Text to be displayed.
**
**  @return      The length of the printed text.
*/
global int VideoDrawReverseText(int x, int y, unsigned font,
	const unsigned char* text)
{
	int w;

	FontColor = ReverseTextColor;
	w = VideoDrawText(x, y, font, text);
	FontColor = DefaultTextColor;

	return w;
}

/**
**  Draw reverse text with font at x,y clipped.
**
**  @see VideoDrawText for full description.
**
**  @param x     X screen position
**  @param y     Y screen position
**  @param font  Font number
**  @param text  Text to be displayed.
**
**  @return      The length of the printed text.
*/
global int VideoDrawReverseTextClip(int x, int y, unsigned font,
	const unsigned char* text)
{
	int w;

	FontColor = ReverseTextColor;
	w = VideoDrawTextClip(x, y, font, text);
	FontColor = DefaultTextColor;

	return w;
}

/**
**  Draw text with font at x,y centered.
**
**  @see VideoDrawText for full description.
**
**  @param x     X screen position
**  @param y     Y screen position
**  @param font  Font number
**  @param text  Text to be displayed.
**
**  @return      The length of the printed text.
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
**  Format a number using commas
**
**  @param number  Number to be formatted
**  @param buf     Buffer to save the formatted number to
*/
local void FormatNumber(int number, char* buf)
{
	char bufs[sizeof(int) * 10 + 2];
	int sl;
	int s;
	int d;

	sl = s = d = 0;
	sprintf(bufs, "%d", number);
	sl = strlen(bufs);
	do {
		if (s > 0 && s < sl && (s - (sl % 3)) % 3 == 0) {
			buf[d++] = ',';
		}
		buf[d++] = bufs[s++];
	} while (s <= sl);
}

/**
**  Draw number with font at x,y unclipped.
**
**  @param x       X screen position
**  @param y       Y screen position
**  @param font    Font number
**  @param number  Number to be displayed.
**
**  @return        The length of the printed text.
*/
global int VideoDrawNumber(int x, int y, unsigned font, int number)
{
	char buf[sizeof(int) * 10 + 2];

	FormatNumber(number, buf);
	return VideoDrawText(x, y, font, buf);
}

/**
**  Draw number with font at x,y clipped.
**
**  @param x       X screen position
**  @param y       Y screen position
**  @param font    Font number
**  @param number  Number to be displayed.
**
**  @return        The length of the printed text.
*/
global int VideoDrawNumberClip(int x, int y, unsigned font, int number)
{
	char buf[sizeof(int) * 10 + 2];

	FormatNumber(number, buf);
	return VideoDrawTextClip(x, y, font, buf);
}

/**
**  Draw reverse number with font at x,y unclipped.
**
**  @param x       X screen position
**  @param y       Y screen position
**  @param font    Font number
**  @param number  Number to be displayed.
**
**  @return        The length of the printed text.
*/
global int VideoDrawReverseNumber(int x, int y, unsigned font, int number)
{
	char buf[sizeof(int) * 10 + 2];

	FormatNumber(number, buf);
	return VideoDrawReverseText(x, y, font, buf);
}

/**
**  Draw reverse number with font at x,y clipped.
**
**  @param x       X screen position
**  @param y       Y screen position
**  @param font    Font number
**  @param number  Number to be displayed.
**
**  @return        The length of the printed text.
*/
global int VideoDrawReverseNumberClip(int x, int y, unsigned font, int number)
{
	char buf[sizeof(int) * 10 + 2];

	FormatNumber(number, buf);
	return VideoDrawReverseTextClip(x, y, font, buf);
}

/**
**  FIXME: docu
*/
local void FontMeasureWidths(ColorFont* fp)
{
	// FIXME: todo.. can this be optimized?
	int y;
	const unsigned char* sp;
	const unsigned char* lp;
	const unsigned char* gp;
	Uint32 ckey;

	memset(fp->CharWidth, 0, sizeof(fp->CharWidth));
	fp->CharWidth[0] = fp->Width / 2;  // a reasonable value for SPACE
	ckey = fp->Graphic->Surface->format->colorkey;

	SDL_LockSurface(fp->Graphic->Surface);
	for (y = 1; y < 207; ++y) {
		sp = (const unsigned char *)fp->Graphic->Surface->pixels +
			y * fp->Height * fp->Graphic->Surface->pitch - 1;
		gp = sp + fp->Graphic->Surface->pitch * fp->Height;
		// Bail out if no letters left
		if (gp >= ((const unsigned char *)fp->Graphic->Surface->pixels +
				fp->Graphic->Surface->pitch * fp->Graphic->Height)) {
			break;
		}
		while (sp < gp) {
			lp = sp + fp->Width - 1;
			for (; sp < lp; --lp) {
				if (*lp != ckey) {
					if (lp - sp > fp->CharWidth[y]) {  // max width
						fp->CharWidth[y] = lp - sp;
					}
				}
			}
			sp += fp->Graphic->Surface->pitch;
		}

	}
	SDL_UnlockSurface(fp->Graphic->Surface);
}

/**
**  Make font bitmap.
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
	const unsigned char* nextsp;
	int numfonts;
	int n;

	FontBitmapWidths[font] = (g->Width + 7) / 8;

	SDL_LockSurface(g->Surface);
	for (n = 0; n < NumFontColors; ++n) {
		if (FontBitmaps[font][n]) {
			free(FontBitmaps[font][n]);
		}
		FontBitmaps[font][n] = (GLubyte*)malloc(FontBitmapWidths[font] * g->Height);

		nextsp = (const unsigned char*)g->Surface->pixels;
		x = 0;
		numfonts = g->Height / Fonts[font].Height;
		for (k = 0; k < numfonts; ++k) {
			for (i = 0; i < Fonts[font].Height; ++i) {
				sp = nextsp;
				nextsp += g->Surface->pitch;
				c = FontBitmaps[font][n] + k * Fonts[font].Height * FontBitmapWidths[font] +
					(Fonts[font].Height - 1 - i) * FontBitmapWidths[font];
				for (j = 0; j < g->Width; ++j) {
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
	SDL_UnlockSurface(g->Surface);
}
#endif

/**
**  Load all fonts.
*/
global void LoadFonts(void)
{
	unsigned i;

	for (i = 0; i < sizeof(Fonts) / sizeof(*Fonts); ++i) {
		if (Fonts[i].File && !Fonts[i].Graphic) {
			ShowLoadProgress("Fonts %s", Fonts[i].File);
			Fonts[i].Graphic = LoadGraphic(Fonts[i].File);
			FontMeasureWidths(Fonts + i);
#ifdef USE_OPENGL
			MakeFontBitmap(Fonts[i].Graphic, i);
#endif
		}
	}
}

/**
**  Find font by identifier.
**
**  @param ident  Font identifier
**
**  @return       Integer as font identifier.
*/
global int FontByIdent(const char* ident)
{
	FontMapping* fm;

	fm = FontMappings;
	while (fm) {
		if (!strcmp(fm->Ident, ident)) {
			return fm->Font;
		}
		fm = fm->Next;
	}
	fprintf(stderr, "Font not found: '%s'", ident);
	ExitFatal(1);
	return 0;
}

/**
**  Find the name of a font.
**
**  @param font  Font identifier.
**
**  @return      Name of the font.
*/
global const char* FontName(int font)
{
	FontMapping* fm;

	fm = FontMappings;
	while (fm) {
		if (fm->Font == font) {
			return fm->Ident;
		}
		fm = fm->Next;
	}
	fprintf(stderr, "Font not found: %d", font);
	ExitFatal(1);
	return NULL;
}

/*----------------------------------------------------------------------------
--  CCL
----------------------------------------------------------------------------*/

/**
**  Define the used fonts.
**
**  @todo  make the font name functions more general, support more fonts.
*/
local int CclDefineFont(lua_State* l)
{
	const char* value;
	int i;
	int w;
	int h;
	char* file;
	FontMapping** fm;
	const char* str;

	if (lua_gettop(l) != 1 || !lua_istable(l, 1)) {
		LuaError(l, "incorrect argument");
	}
	i = -1;
	w = h = 0;
	file = NULL;
	lua_pushnil(l);
	while (lua_next(l, 1)) {
		value = LuaToString(l, -2);
		if (!strcmp(value, "Name")) {
			str = LuaToString(l, -1);
			fm = &FontMappings;
			i = 0;
			while (*fm) {
				if (!strcmp((*fm)->Ident, str)) {
					break;
				}
				fm = &(*fm)->Next;
				++i;
			}
			if (!*fm) {
				*fm = malloc(sizeof(**fm));
				(*fm)->Ident = strdup(str);
				(*fm)->Font = i;
				(*fm)->Next = NULL;
			}
		} else if (!strcmp(value, "File")) {
			file = strdup(LuaToString(l, -1));
		} else if (!strcmp(value, "Size")) {
			if (!lua_istable(l, -1) || luaL_getn(l, -1) != 2) {
				LuaError(l, "incorrect argument");
			}
			lua_rawgeti(l, -1, 1);
			w = LuaToNumber(l, -1);
			lua_pop(l, 1);
			lua_rawgeti(l, -1, 2);
			h = LuaToNumber(l, -1);
			lua_pop(l, 1);
		} else {
			LuaError(l, "Unsupported tag: %s" _C_ value);
		}
		lua_pop(l, 1);
	}
	if (i == -1 || !w || !h || !file) {
		LuaError(l, "missing argument");
	}
	free(Fonts[i].File);
	VideoSafeFree(Fonts[i].Graphic);
	Fonts[i].Graphic = NULL;
	Fonts[i].File = file;
	Fonts[i].Width = w;
	Fonts[i].Height = h;

	return 0;
}

/**
**  Define a font color.
*/
local int CclDefineFontColor(lua_State* l)
{
	char* color;
	int i;
	FontColorMapping* fcm;
	FontColorMapping** fcmp;

	if (lua_gettop(l) != 2) {
		LuaError(l, "incorrect argument");
	}
	color = strdup(LuaToString(l, 1));

	if (!FontColorMappings) {
		FontColorMappings = calloc(sizeof(*FontColorMappings), 1);
		fcm = FontColorMappings;
	} else {
		fcmp = &FontColorMappings;
		while (*fcmp) {
			if (!strcmp((*fcmp)->ColorName, color)) {
				fprintf(stderr, "Warning: Redefining color '%s'\n", color);
				free((*fcmp)->ColorName);
				fcm = *fcmp;
				break;
			}
			fcmp = &(*fcmp)->Next;
		}
		*fcmp = calloc(sizeof(*FontColorMappings), 1);
		fcm = *fcmp;
	}
	fcm->ColorName = color;
	fcm->Next = NULL;

	if (luaL_getn(l, 2) != NumFontColors * 3) {
		fprintf(stderr, "Wrong vector length\n");
	}
	for (i = 0; i < NumFontColors; ++i) {
		lua_rawgeti(l, 2, i * 3 + 1);
		fcm->Color[i].r = LuaToNumber(l, -1);
		lua_pop(l, 1);
		lua_rawgeti(l, 2, i * 3 + 2);
		fcm->Color[i].g = LuaToNumber(l, -1);
		lua_pop(l, 1);
		lua_rawgeti(l, 2, i * 3 + 3);
		fcm->Color[i].b = LuaToNumber(l, -1);
		lua_pop(l, 1);
	}

	return 0;
}

/**
**  Register CCL features for fonts.
**
**  @todo FIXME: Make the remaining functions accessable from CCL.
*/
global void FontsCclRegister(void)
{
	lua_register(Lua, "DefineFont", CclDefineFont);
	lua_register(Lua, "DefineFontColor", CclDefineFontColor);

//	lua_register(Lua, "DefaultTextColors", CclDefaultTextColors);
//	lua_register(Lua, "TextLength", CclTextLength);
//	lua_register(Lua, "DrawText", CclDrawText);
//	lua_register(Lua, "DrawReverseText", CclDrawReverseText);
//	lua_register(Lua, "DrawTextCentered", CclDrawTextCentered);
//	lua_register(Lua, "DrawReverseTextCentered", CclDrawReverseTextCentered);
//	lua_register(Lua, "DrawNumber", CclDrawNumber);
//	lua_register(Lua, "DrawReverseNumber", CclDrawReverseNumber);
}

/**
**  Cleanup the font module.
*/
global void CleanFonts(void)
{
	unsigned i;
	FontColorMapping *fcmp, *fcmpn;

	for (i = 0; i < sizeof(Fonts) / sizeof(*Fonts); ++i) {
		free(Fonts[i].File);
		VideoSafeFree(Fonts[i].Graphic);
		Fonts[i].File = NULL;
		Fonts[i].Graphic = NULL;
	}

	fcmp = FontColorMappings;
	while (fcmp) {
		fcmpn = fcmp->Next;
		free(fcmp);
		fcmp = fcmpn;
	}
	FontColorMappings = NULL;
}

/**
**  Check if font is already loaded.
**
**  @param font  Font number
**
**  @return      True if loaded, false otherwise.
*/
global int IsFontLoaded(unsigned font)
{
	return Fonts[font].Graphic != 0;
}

//@}
