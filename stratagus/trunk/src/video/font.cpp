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
	char* ColorName;                        ///< Font color name
	SDL_Color Color[NumFontColors];         ///< Array of colors
} FontColorMapping;

static FontColorMapping* FontColor;         ///< Current font color

static FontColorMapping* FontColorMappings; ///< Font color mappings
static int NumFontColorMappings;            ///< Number of font color mappings

	/// Font mapping
typedef struct _font_mapping_ {
	char* Ident;                            ///< Font name
	int Font;                               ///< Ident number
	struct _font_mapping_* Next;            ///< Next pointer
} FontMapping;

static FontMapping* FontMappings;           ///< Font mappings

/**
**  Fonts table
**
**  Define the font files, sizes.
*/
static ColorFont Fonts[MaxFonts];

static FontColorMapping* LastTextColor;    ///< Last text color
static FontColorMapping* DefaultTextColor; ///< Default text color
static FontColorMapping* ReverseTextColor; ///< Reverse text color
static char* DefaultNormalColorIndex;      ///< Default normal color index
static char* DefaultReverseColorIndex;     ///< Default reverse color index

	/// Draw character with current video depth.
static void VideoDrawChar(const Graphic*, int, int, int, int, int, int);

#ifdef USE_OPENGL
static Graphic** FontColorGraphics[MaxFonts];   ///< Font color graphics
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
static void VideoDrawChar(const Graphic* g,
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

	SDL_SetColors(g->Surface, FontColor->Color, 0, NumFontColors);

	SDL_BlitSurface(g->Surface, &srect, TheScreen, &drect);
}
#else
static void VideoDrawChar(const Graphic* g,
	int gx, int gy, int w, int h, int x, int y)
{
	VideoDrawSub(g, gx, gy, w, h, x, y);
}
#endif

/**
**  FIXME: docu
*/
static FontColorMapping* GetFontColorMapping(char* color)
{
	int i;

	for (i = 0; i < NumFontColorMappings; ++i) {
		if (!strcmp(FontColorMappings[i].ColorName, color)) {
			return &FontColorMappings[i];
		}
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
void SetDefaultTextColors(char* normal, char* reverse)
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
void GetDefaultTextColors(char** normalp, char** reversep)
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
int VideoTextLength(unsigned font, const unsigned char* text)
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
			if (*s == '<' || *s == '>') {
				isformat = 0;
				continue;
			}
			if (*s == '!') {
				continue;
			}
			if (*s != '~') { // ~~ -> ~
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
int VideoTextHeight(unsigned font)
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
static void VideoDrawCharClip(const Graphic* g, int gx, int gy, int w, int h,
	int x, int y)
{
	int ox;
	int oy;
	int ex;
	CLIP_RECTANGLE_OFS(x, y, w, h, ox, oy, ex);
	VideoDrawChar(g, gx + ox, gy + oy, w, h, x, y);
}

/**
**  Draw text with font at x,y clipped/unclipped.
**
**  ~    is special prefix.
**  ~~   is the ~ character self.
**  ~!   print next character reverse.
**  ~<   start reverse.
**  ~>   switch back to last used color.
**
**  @param x     X screen position
**  @param y     Y screen position
**  @param font  Font number
**  @param text  Text to be displayed.
**  @param clip  Flag if TRUE clip, otherwise not.
**
**  @return      The length of the printed text.
*/
static int DoDrawText(int x, int y, unsigned font, const unsigned char* text,
	int clip)
{
	int w;
	int widths;
	const ColorFont* fp;
	FontColorMapping* rev;
	char* color;
	const unsigned char* p;
	void (*DrawChar)(const Graphic*, int, int, int, int, int, int);
	int ipr;
	int c;
	Graphic* g;

	if (clip) {
		DrawChar = VideoDrawCharClip;
	} else {
		DrawChar = VideoDrawChar;
	}

	fp = Fonts + font;
#ifndef USE_OPENGL
	g = fp->G;
#else
	g = FontColorGraphics[font][FontColor - FontColorMappings];
#endif
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
#ifdef USE_OPENGL
					g = FontColorGraphics[font][FontColor - FontColorMappings];
#endif
					++text;
					break;
				case '<':
					LastTextColor = FontColor;
					FontColor = ReverseTextColor;
#ifdef USE_OPENGL
					g = FontColorGraphics[font][FontColor - FontColorMappings];
#endif
					continue;
				case '>':
					rev = LastTextColor;  // swap last and current color
					LastTextColor = FontColor;
					FontColor = rev;
#ifdef USE_OPENGL
					g = FontColorGraphics[font][FontColor - FontColorMappings];
#endif
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
#ifdef USE_OPENGL
					g = FontColorGraphics[font][FontColor - FontColorMappings];
#endif
					free(color);
					continue;
			}
		}

		c = *text - 32;
		Assert(c >= 0);

		ipr = fp->G->GraphicWidth / fp->Width;
		if (c >= 0 && c < ipr * fp->G->GraphicHeight / fp->Height) {
			w = fp->CharWidth[c];
			DrawChar(g, (c % ipr) * fp->Width, (c / ipr) * fp->Height,
				w, fp->Height, x + widths, y);
		} else {
			w = fp->CharWidth[0];
			DrawChar(g, 0, 0, w, fp->Height, x + widths, y);
		}
		widths += w + 1;
		if (rev) {
			FontColor = rev;
#ifdef USE_OPENGL
			g = FontColorGraphics[font][FontColor - FontColorMappings];
#endif
			rev = NULL;
		}
	}

	return widths;
}

/**
**  Draw text with font at x,y unclipped.
**
**  ~    is special prefix.
**  ~~   is the ~ character self.
**  ~!   print next character reverse.
**  ~n   0123456789abcdef print text in color 1-16.
**  ~<   start reverse.
**  ~>   switch back to last used color.
**
**  @param x     X screen position
**  @param y     Y screen position
**  @param font  Font number
**  @param text  Text to be displayed.
**
**  @return      The length of the printed text.
*/
int VideoDrawText(int x, int y, unsigned font,
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
int VideoDrawTextClip(int x, int y, unsigned font,
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
int VideoDrawReverseText(int x, int y, unsigned font,
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
int VideoDrawReverseTextClip(int x, int y, unsigned font,
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
int VideoDrawTextCentered(int x, int y, unsigned font,
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
static void FormatNumber(int number, char* buf)
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
int VideoDrawNumber(int x, int y, unsigned font, int number)
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
int VideoDrawNumberClip(int x, int y, unsigned font, int number)
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
int VideoDrawReverseNumber(int x, int y, unsigned font, int number)
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
int VideoDrawReverseNumberClip(int x, int y, unsigned font, int number)
{
	char buf[sizeof(int) * 10 + 2];

	FormatNumber(number, buf);
	return VideoDrawReverseTextClip(x, y, font, buf);
}

/**
**  Calculate the width of each character
**
**  @param fp  Font to calculate
*/
static void FontMeasureWidths(ColorFont* fp)
{
	// FIXME: todo.. can this be optimized?
	int y;
	const unsigned char* sp;
	const unsigned char* lp;
	const unsigned char* gp;
	Uint32 ckey;
	int ipr;

	memset(fp->CharWidth, 0, sizeof(fp->CharWidth));
	fp->CharWidth[0] = fp->Width / 2;  // a reasonable value for SPACE
	ckey = fp->G->Surface->format->colorkey;
	ipr = fp->G->Surface->w / fp->Width;

	SDL_LockSurface(fp->G->Surface);
	for (y = 1; y < 207; ++y) {
		sp = (const unsigned char*)fp->G->Surface->pixels +
			(y / ipr) * fp->G->Surface->pitch * fp->Height +
			(y % ipr) * fp->Width - 1;
		gp = sp + fp->G->Surface->pitch * fp->Height;
		// Bail out if no letters left
		if (gp >= ((const unsigned char*)fp->G->Surface->pixels +
				fp->G->Surface->pitch * fp->G->Height)) {
			break;
		}
		while (sp < gp) {
			lp = sp + fp->Width;
			for (; sp < lp; --lp) {
				if (*lp != ckey) {
					if (lp - sp > fp->CharWidth[y]) {  // max width
						fp->CharWidth[y] = lp - sp;
					}
				}
			}
			sp += fp->G->Surface->pitch;
		}

	}
	SDL_UnlockSurface(fp->G->Surface);
}

/**
**  Make font bitmap.
**
**  @param g     Font graphic
**  @param font  Font number
*/
#ifdef USE_OPENGL
static void MakeFontColorTextures(Graphic* g, int font)
{
	int i;
	int j;
	SDL_Surface* s;

	FontColorGraphics[font] = malloc(NumFontColorMappings * sizeof(Graphic*));
	s = g->Surface;
	for (i = 0; i < NumFontColorMappings; ++i) {
		FontColorGraphics[font][i] = calloc(1, sizeof(Graphic));
		memcpy(FontColorGraphics[font][i], g, sizeof(Graphic));
		FontColorGraphics[font][i]->Textures = NULL;
		SDL_LockSurface(s);
		for (j = 0; j < NumFontColors; ++j) {
			s->format->palette->colors[j] = FontColorMappings[i].Color[j];
		}
		SDL_UnlockSurface(s);
		MakeTexture(FontColorGraphics[font][i]);
	}
}
#endif

/**
**  Load all fonts.
*/
void LoadFonts(void)
{
	unsigned i;

	for (i = 0; i < sizeof(Fonts) / sizeof(*Fonts); ++i) {
		if (Fonts[i].G && !GraphicLoaded(Fonts[i].G)) {
			ShowLoadProgress("Fonts %s", Fonts[i].G->File);
			DebugPrint("Font %s\n" _C_ Fonts[i].G->File);
			LoadGraphic(Fonts[i].G);
			FontMeasureWidths(Fonts + i);
#ifdef USE_OPENGL
			MakeFontColorTextures(Fonts[i].Graphic, i);
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
int FontByIdent(const char* ident)
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
const char* FontName(int font)
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
*/
static int CclDefineFont(lua_State* l)
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
	Fonts[i].G = NewGraphic(file, w, h);
	Fonts[i].Width = w;
	Fonts[i].Height = h;

	return 0;
}

/**
**  Define a font color.
*/
static int CclDefineFontColor(lua_State* l)
{
	char* color;
	int i;
	FontColorMapping* fcm;

	if (lua_gettop(l) != 2) {
		LuaError(l, "incorrect argument");
	}
	color = strdup(LuaToString(l, 1));
	fcm = NULL;

	if (!NumFontColorMappings) {
		FontColorMappings = calloc(sizeof(*FontColorMappings), 1);
		fcm = FontColorMappings;
		++NumFontColorMappings;
	} else {
		for (i = 0; i < NumFontColorMappings; ++i) {
			fcm = &FontColorMappings[i];
			if (!strcmp(fcm->ColorName, color)) {
				fprintf(stderr, "Warning: Redefining color '%s'\n", color);
				free(fcm->ColorName);
				break;
			}
		}
		if (i == NumFontColorMappings) {
			++NumFontColorMappings;
			FontColorMappings = realloc(FontColorMappings,
				NumFontColorMappings * sizeof(FontColorMapping));
			fcm = &FontColorMappings[NumFontColorMappings - 1];
		}
	}
	fcm->ColorName = color;

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
*/
void FontsCclRegister(void)
{
	lua_register(Lua, "DefineFont", CclDefineFont);
	lua_register(Lua, "DefineFontColor", CclDefineFontColor);
}

/**
**  Cleanup the font module.
*/
void CleanFonts(void)
{
	int i;
#ifdef USE_OPENGL
	int j;
#endif

	for (i = 0; i < (int)(sizeof(Fonts) / sizeof(*Fonts)); ++i) {
		FreeGraphic(Fonts[i].G);
		Fonts[i].G = NULL;

#ifdef USE_OPENGL
		for (j = 0; j < NumFontColorMappings; ++j) {
			if (!FontColorGraphics[i]) {
				break;
			}
			glDeleteTextures(FontColorGraphics[i][j]->NumFrames,
				FontColorGraphics[i][j]->Textures);
			free(FontColorGraphics[i][j]->Textures);
		}
		free(FontColorGraphics[i]);
		FontColorGraphics[i] = NULL;
#endif
	}

	for (i = 0; i < NumFontColorMappings; ++i) {
		free(FontColorMappings[i].ColorName);
	}
	free(FontColorMappings);
	FontColorMappings = NULL;
	NumFontColorMappings = 0;
}

/**
**  Check if font is already loaded.
**
**  @param font  Font number
**
**  @return      True if loaded, false otherwise.
*/
int IsFontLoaded(unsigned font)
{
	return GraphicLoaded(Fonts[font].G);
}

//@}
