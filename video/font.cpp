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
/**@name font.cpp - The color fonts. */
//
//      (c) Copyright 1998-2005 by Lutz Sammer, Jimmy Salmon, Nehal Mistry
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

#include <string>
#include <vector>
#include <map>

#include "video.h"
#include "font.h"
#include "script.h"

#include "intern_video.h"

/*----------------------------------------------------------------------------
--  Variables
----------------------------------------------------------------------------*/

#define NumFontColors 9

	/// Font color mapping
class FontColorMapping {
public:
	FontColorMapping() : ColorName(NULL), Index(0) {}

	char *ColorName;                        /// Font color name
	SDL_Color Color[NumFontColors];         /// Array of colors
	int Index;                              /// Index in FontColorMappings
};

static FontColorMapping *FontColor;         /// Current font color

static std::vector<FontColorMapping> FontColorMappings; /// Font color mappings

static std::map<std::string, CFont *> Fonts;    /// Font mappings
static std::map<CFont *, std::string> FontNames;/// Font name mappings

/**
**  Fonts table
**
**  Define the font files, sizes.
*/
static std::vector<CFont *> AllFonts;

static FontColorMapping *LastTextColor;    /// Last text color
static FontColorMapping *DefaultTextColor; /// Default text color
static FontColorMapping *ReverseTextColor; /// Reverse text color
static char *DefaultNormalColorIndex;      /// Default normal color index
static char *DefaultReverseColorIndex;     /// Default reverse color index

#ifdef USE_OPENGL
static CGraphic **FontColorGraphics[MaxFonts];   /// Font color graphics
#endif

CFont *SmallFont;       /// Small font used in stats
CFont *GameFont;        /// Normal font used in game
CFont *LargeFont;       /// Large font used in menus
CFont *SmallTitleFont;  /// Small font used in episoden titles
CFont *LargeTitleFont;  /// Large font used in episoden titles

/*----------------------------------------------------------------------------
--  Functions
----------------------------------------------------------------------------*/

/**
**  Draw character with current color.
**
**  @param g   Pointer to object
**  @param gx  X offset into object
**  @param gy  Y offset into object
**  @param w   width to display
**  @param h   height to display
**  @param x   X screen position
**  @param y   Y screen position
*/
#ifndef USE_OPENGL
static void VideoDrawChar(const CGraphic *g,
	int gx, int gy, int w, int h, int x, int y)
{
	SDL_Rect srect = {gx, gy, w, h};
	SDL_Rect drect = {x, y};

	SDL_SetColors(g->Surface, FontColor->Color, 0, NumFontColors);
	SDL_BlitSurface(g->Surface, &srect, TheScreen, &drect);
}
#else
static void VideoDrawChar(const CGraphic *g,
	int gx, int gy, int w, int h, int x, int y)
{
	g->DrawSub(gx, gy, w, h, x, y);
}
#endif

/**
**  Find the relevant font color for a color string eg. "blue"
**
**  @param color  The text color to find "blue"
**
**  @return       Pointer to the information about this color.
*/
static FontColorMapping *GetFontColorMapping(char *color)
{
	std::vector<FontColorMapping>::iterator i;
	for (i = FontColorMappings.begin(); i != FontColorMappings.end(); ++i) {
		if (!strcmp((*i).ColorName, color)) {
			return &(*i);
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
void SetDefaultTextColors(char *normal, char *reverse)
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
void GetDefaultTextColors(char **normalp, char **reversep)
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
int VideoTextLength(CFont *font, const char *text)
{
	int width;
	const char *s;
	const char *widths;
	int isformat;

	widths = font->CharWidth;
	isformat = 0;
	for (width = 0, s = text; *s; ++s) {
		if (*s == '~') {
			if (!*++s) {  // bad formatted string
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
int VideoTextHeight(CFont *font)
{
	return font->G->Height;
}

/**
**  Draw character with current color clipped into 8 bit framebuffer.
**
**  @param g   Pointer to object
**  @param gx  X offset into object
**  @param gy  Y offset into object
**  @param w   width to display
**  @param h   height to display
**  @param x   X screen position
**  @param y   Y screen position
*/
static void VideoDrawCharClip(const CGraphic *g, int gx, int gy, int w, int h,
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
static int DoDrawText(int x, int y, CFont *font, const char *text,
	int clip)
{
	int w;
	int widths;
	FontColorMapping *rev;
	char *color;
	const char *p;
	void (*DrawChar)(const CGraphic *, int, int, int, int, int, int);
	int ipr;
	int c;
	CGraphic *g;

	if (clip) {
		DrawChar = VideoDrawCharClip;
	} else {
		DrawChar = VideoDrawChar;
	}

#ifndef USE_OPENGL
	g = font->G;
#else
	g = FontColorGraphics[font][FontColor->Index];
#endif
	for (rev = NULL, widths = 0; *text; ++text) {
		if (*text == '~') {
			switch (*++text) {
				case '\0':  // wrong formatted string.
					DebugPrint("oops, format your ~\n");
					return widths;
				case '~':
					break;
				case '!':
					rev = FontColor;
					FontColor = ReverseTextColor;
#ifdef USE_OPENGL
					g = FontColorGraphics[font][FontColor->Index];
#endif
					++text;
					break;
				case '<':
					LastTextColor = FontColor;
					FontColor = ReverseTextColor;
#ifdef USE_OPENGL
					g = FontColorGraphics[font][FontColor->Index];
#endif
					continue;
				case '>':
					rev = LastTextColor;  // swap last and current color
					LastTextColor = FontColor;
					FontColor = rev;
#ifdef USE_OPENGL
					g = FontColorGraphics[font][FontColor->Index];
#endif
					continue;

				default:
					p = text;
					while (*p && *p !='~') {
						++p;
					}
					if (!*p) {
						DebugPrint("oops, format your ~\n");
						return widths;
					}
					color = new char[p - text + 1];
					memcpy(color, text, p - text);
					color[p - text] = '\0';
					text = p;
					LastTextColor = FontColor;
					FontColor = GetFontColorMapping(color);
#ifdef USE_OPENGL
					g = FontColorGraphics[font][FontColor->Index];
#endif
					delete[] color;
					continue;
			}
		}

		c = *(unsigned char *)text - 32;
		Assert(c >= 0);

		ipr = font->G->GraphicWidth / font->G->Width;
		if (c >= 0 && c < ipr * font->G->GraphicHeight / font->G->Height) {
			w = font->CharWidth[c];
			DrawChar(g, (c % ipr) * font->G->Width, (c / ipr) * font->G->Height,
				w, font->G->Height, x + widths, y);
		} else {
			w = font->CharWidth[0];
			DrawChar(g, 0, 0, w, font->G->Height, x + widths, y);
		}
		widths += w + 1;
		if (rev) {
			FontColor = rev;
#ifdef USE_OPENGL
			g = FontColorGraphics[font][FontColor->Index];
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
int VideoDrawText(int x, int y, CFont *font, const char *text)
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
int VideoDrawTextClip(int x, int y, CFont *font, const char *text)
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
int VideoDrawReverseText(int x, int y, CFont *font, const char *text)
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
int VideoDrawReverseTextClip(int x, int y, CFont *font, const char *text)
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
int VideoDrawTextCentered(int x, int y, CFont *font, const char *text)
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
static void FormatNumber(int number, char *buf)
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
**  Return the first occurance of c in [s- s + maxlen]
**
**  @param s       original string.
**  @param c       charrater to find.
**  @param maxlen  size limit of the search. (0 means unlimited).
**  @param font    if specified use VideoTextLength instead of strlen.
**
**  @return computed value.
*/
static char *strchrlen(char *s, char c, int maxlen, CFont *font)
{
	char *res;

	Assert(s);
	Assert(0 <= maxlen);

	res = strchr(s, c);

	if (!maxlen) {
		return res;
	}
	if (!font &&
			(s + maxlen < res || (!res && strlen(s) >= (unsigned)maxlen))) {
		c = s[maxlen];
		s[maxlen] = '\0';
		res = strrchr(s, ' ');
		s[maxlen] = c;
		if (!res) {
			fprintf(stderr, "line too long: \"%s\"\n", s);
			res = s + maxlen;
		}
	} else if (font) {
		char *end;

		if (!res) {
			res = s + strlen(s);
		}
		end = res;
		c = *end;
		*end = '\0';
		while (VideoTextLength(font, s) > maxlen) {
			res = strrchr(s, ' ');
			*end = c;
			end = res;
			if (!res) {
				fprintf(stderr, "line too long: \"%s\"\n", s);
				return strchr(s, '\n');
			}
			c = *end;
			*end = '\0';
		}
		*end = c;
	}
	return res;
}

/**
**  Return the 'line' line of the string 's'.
**
**  @param line    line number.
**  @param s       multiline string.
**  @param maxlen  max length of the string.
**  @param font    if specified use VideoTextLength instead of strlen.
**
**  @return computed value.
*/
char *GetLineFont(int line, char *s, int maxlen, CFont *font)
{
	int i;
	char *res;
	char *tmp;

	Assert(0 < line);
	Assert(s);
	Assert(0 <= maxlen);

	res = s;
	for (i = 1; i < line; ++i) {
		res = strchrlen(res, '\n', maxlen, font);
		if (!res) {
			return NULL;
		}
		while (*res == '\n' || *res == ' ') {
			++res;
		}
	}
	res = new_strdup(res);
	tmp = strchrlen(res, '\n', maxlen, font);
	if (tmp) {
		*tmp = '\0';
	}
	return res;
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
int VideoDrawNumber(int x, int y, CFont *font, int number)
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
int VideoDrawNumberClip(int x, int y, CFont *font, int number)
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
int VideoDrawReverseNumber(int x, int y, CFont *font, int number)
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
int VideoDrawReverseNumberClip(int x, int y, CFont *font, int number)
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
static void FontMeasureWidths(CFont *fp)
{
	// FIXME: todo.. can this be optimized?
	int y;
	const unsigned char *sp;
	const unsigned char *lp;
	const unsigned char *gp;
	Uint32 ckey;
	int ipr;

	memset(fp->CharWidth, 0, sizeof(fp->CharWidth));
	fp->CharWidth[0] = fp->G->Width / 2;  // a reasonable value for SPACE
	ckey = fp->G->Surface->format->colorkey;
	ipr = fp->G->Surface->w / fp->G->Width;

	SDL_LockSurface(fp->G->Surface);
	for (y = 1; y < 207; ++y) {
		sp = (const unsigned char*)fp->G->Surface->pixels +
			(y / ipr) * fp->G->Surface->pitch * fp->G->Height +
			(y % ipr) * fp->G->Width - 1;
		gp = sp + fp->G->Surface->pitch * fp->G->Height;
		// Bail out if no letters left
		if (gp >= ((const unsigned char*)fp->G->Surface->pixels +
				fp->G->Surface->pitch * fp->G->GraphicHeight)) {
			break;
		}
		while (sp < gp) {
			lp = sp + fp->G->Width;
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
static void MakeFontColorTextures(CGraphic *g, int font)
{
	SDL_Surface *s;
	std::vector<FontColorMapping>::size_type i;

	if (FontColorGraphics[font]) {
		return;
	}

	FontColorGraphics[font] = new CGraphic *[FontColorMappings.size()];
	s = g->Surface;
	for (i = 0; i < (int)FontColorMappings.size(); ++i) {
		FontColorGraphics[font][i] = new CGraphic;
		memcpy(FontColorGraphics[font][i], g, sizeof(CGraphic));
		FontColorGraphics[font][i]->Textures = NULL;
		SDL_LockSurface(s);
		for (int j = 0; j < NumFontColors; ++j) {
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
	std::vector<CFont>::size_type i;
	for (i = 0; i < AllFonts.size(); ++i) {
		if (AllFonts[i]->G) {
			ShowLoadProgress("Fonts %s", AllFonts[i]->G->File);
			DebugPrint("Font %s\n" _C_ AllFonts[i]->G->File);
			AllFonts[i]->G->Load();
			FontMeasureWidths(AllFonts[i]);
#ifdef USE_OPENGL
			MakeFontColorTextures(AllFonts[i]->G, i);
#endif
		}
	}

	// TODO: remove this
	SmallFont = FontByIdent("small");
	GameFont = FontByIdent("game");
	LargeFont = FontByIdent("large");
	SmallTitleFont = FontByIdent("small-title");
	LargeTitleFont = FontByIdent("large-title");
}

#ifdef USE_OPENGL
/**
**  Reload OpenGL fonts
*/
void ReloadFonts(void)
{
	std::vector<CFont>::size_type i;
	for (i = 0; i < AllFonts.size(); ++i) {
		if (AllFonts[i]->G) {
			for (int j = 0; j < (int)FontColorMappings.size(); ++j) {
				delete FontColorGraphics[i][j];
			}
			delete[] FontColorGraphics[i];
			FontColorGraphics[i] = NULL;
			MakeFontColorTextures(AllFonts[i].G, i);
		}
	}
}
#endif

/**
**  Find font by identifier.
**
**  @param ident  Font identifier
**
**  @return       Integer as font identifier.
*/
CFont *FontByIdent(const char *ident)
{
	CFont *font = Fonts[ident];
	if (!font) {
		fprintf(stderr, "Font not found: '%s'", ident);
		ExitFatal(1);
	}
	return font;
}

/**
**  Find the name of a font.
**
**  @param font  Font identifier.
**
**  @return      Name of the font.
*/
const char *FontName(CFont *font)
{
	const char *s = FontNames[font].c_str();
	if (!*s) {
		fprintf(stderr, "Font not found.");
		ExitFatal(1);
	}

	return s;
}

/**
**  Create a new font
**
**  @param ident  Font identifier
**  @param g      Graphic
**
**  @return       New font
*/
CFont *CFont::New(const char *ident, CGraphic *g)
{
	CFont *font = Fonts[ident];
	if (font) {
		if (font->G != g) {
			CGraphic::Free(font->G);
		}
		font->G = g;
	} else {
		font = new CFont;
		font->G = g;
		AllFonts.push_back(font);
		Fonts[ident] = font;
		FontNames[font] = ident;
	}
	return font;
}

/**
**  Get a font
**
**  @param ident  Font identifier
**
**  @return       The font
*/
CFont *CFont::Get(const char *ident)
{
	CFont *font = Fonts[ident];
	if (!font) {
		DebugPrint("font not found: %s" _C_ ident);
	}
	return font;
}

/*----------------------------------------------------------------------------
--  CCL
----------------------------------------------------------------------------*/

/**
**  Define a font color.
*/
static int CclDefineFontColor(lua_State *l)
{
	char *color;
	int i;
	int args;
	FontColorMapping *fcm;

	LuaCheckArgs(l, 2);
	color = new_strdup(LuaToString(l, 1));
	fcm = NULL;

	if (!FontColorMappings.size()) {
		FontColorMapping f;
		FontColorMappings.push_back(f);
		fcm = &FontColorMappings[0];
	} else {
		std::vector<FontColorMapping>::iterator it;
		for (it = FontColorMappings.begin(); it != FontColorMappings.end(); ++it) {
			fcm = &(*it);
			if (!strcmp(fcm->ColorName, color)) {
				fprintf(stderr, "Warning: Redefining color '%s'\n", color);
				delete[] fcm->ColorName;
				break;
			}
		}
		if (it == FontColorMappings.end()) {
			FontColorMapping f;
			FontColorMappings.push_back(f);
			fcm = &FontColorMappings[FontColorMappings.size() - 1];
			fcm->Index = FontColorMappings.size() - 1;
		}
	}
	fcm->ColorName = color;

	args = luaL_getn(l, 2) / 3;
	Assert(args <= NumFontColors);
	for (i = 0; i < args; ++i) {
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
	for (; i < NumFontColors; ++i) {
		fcm->Color[i].r = fcm->Color[i].g = fcm->Color[i].b = 0;
	}

	return 0;
}

/**
**  Register CCL features for fonts.
*/
void FontsCclRegister(void)
{
	lua_register(Lua, "DefineFontColor", CclDefineFontColor);
}

/**
**  Cleanup the font module.
*/
void CleanFonts(void)
{
	int i;

	for (i = 0; i < (int)AllFonts.size(); ++i) {
		CGraphic::Free(AllFonts[i]->G);
		AllFonts[i]->G = NULL;
		delete AllFonts[i];

#ifdef USE_OPENGL
		for (int j = 0; j < (int)FontColorMappings.size(); ++j) {
			if (!FontColorGraphics[i]) {
				break;
			}
			glDeleteTextures(FontColorGraphics[i][j]->NumFrames,
				FontColorGraphics[i][j]->Textures);
			delete[] FontColorGraphics[i][j]->Textures;
		}
		delete[] FontColorGraphics[i];
		FontColorGraphics[i] = NULL;
#endif
	}
	AllFonts.clear();
	Fonts.clear();
	FontNames.clear();

	for (i = 0; i < (int)FontColorMappings.size(); ++i) {
		delete[] FontColorMappings[i].ColorName;
	}
	FontColorMappings.clear();

	SmallFont = NULL;
	GameFont = NULL;
	LargeFont = NULL;
	SmallTitleFont = NULL;
	LargeTitleFont = NULL;
}

/**
**  Check if font is already loaded.
**
**  @param font  Font number
**
**  @return      True if loaded, false otherwise.
*/
int IsFontLoaded(CFont *font)
{
	return font && font->G && font->G->Loaded();
}

//@}
