//     ____                _       __               
//    / __ )____  _____   | |     / /___ ___________
//   / __  / __ \/ ___/   | | /| / / __ `/ ___/ ___/
//  / /_/ / /_/ (__  )    | |/ |/ / /_/ / /  (__  ) 
// /_____/\____/____/     |__/|__/\__,_/_/  /____/  
//                                              
//       A futuristic real-time strategy game.
//          This file is part of Bos Wars.
//
/**@name font.cpp - The color fonts. */
//
//      (c) Copyright 1998-2006 by Lutz Sammer, Jimmy Salmon, Nehal Mistry
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

static std::vector<CFont *> AllFonts;           /// Vector of all fonts
static std::map<std::string, CFont *> Fonts;    /// Font mappings

static std::vector<CFontColor *> AllFontColors; /// Vector of all font colors.
std::map<std::string, CFontColor *> FontColors; /// Map of ident to font color.

static CFontColor *FontColor;                   /// Current font color

static CFontColor *LastTextColor;          /// Last text color
static CFontColor *DefaultTextColor;       /// Default text color
static CFontColor *ReverseTextColor;       /// Reverse text color
static std::string DefaultNormalColorIndex;     /// Default normal color index
static std::string DefaultReverseColorIndex;    /// Default reverse color index

#ifdef USE_OPENGL
/**
**  Font color graphics
**  Usage: FontColorGraphics[CFont *font][CFontColor *color]
*/
static std::map<CFont *, std::map<CFontColor *, CGraphic *> > FontColorGraphics;
#endif

// FIXME: remove these
CFont *SmallFont;       /// Small font used in stats
CFont *GameFont;        /// Normal font used in game
CFont *LargeFont;       /// Large font used in menus
CFont *SmallTitleFont;  /// Small font used in episoden titles
CFont *LargeTitleFont;  /// Large font used in episoden titles


/*----------------------------------------------------------------------------
--  Guichan Functions
----------------------------------------------------------------------------*/

void CFont::drawString(gcn::Graphics *graphics, const std::string &txt,
	int x, int y) 
{
	const gcn::ClipRectangle &r = graphics->getCurrentClipArea();
	int right = (r.x + r.width - 1 < Video.Width) ? r.x + r.width - 1 : Video.Width - 1;
	int bottom = (r.y + r.height - 1 < Video.Height) ? r.y + r.height - 1 : Video.Height - 1;

	if (r.x > right || r.y > bottom) {
		return;
	}

	PushClipping();
	SetClipping(r.x, r.y, right, bottom);
	VideoDrawTextClip(x + r.xOffset, y + r.yOffset, this, txt);
	PopClipping();
}

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

	SDL_SetColors(g->Surface, FontColor->Colors, 0, MaxFontColors);
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
**  Set the default text colors.
**
**  @param normal   Normal text color.
**  @param reverse  Reverse text color.
*/
void SetDefaultTextColors(const std::string &normal, const std::string &reverse)
{
	DefaultNormalColorIndex = normal;
	DefaultReverseColorIndex = reverse;
	LastTextColor = DefaultTextColor = FontColor = CFontColor::Get(normal);
	ReverseTextColor = CFontColor::Get(reverse);
}

/**
**  Get the default text colors.
**
**  @param normalp   Normal text color pointer.
**  @param reversep  Reverse text color pointer.
*/
void GetDefaultTextColors(std::string &normalp, std::string &reversep)
{
	normalp = DefaultNormalColorIndex;
	reversep = DefaultReverseColorIndex;
}

/**
**  Get the next utf8 character from a string
*/
static bool GetUTF8(const std::string &text, size_t &pos, int &utf8)
{
	// end of string
	if (pos >= text.size()) {
		return false;
	}

	int count;
	char c = text[pos++];

	// ascii
	if (!(c & 0x80)) {
		utf8 = c;
		return true;
	}

	if ((c & 0xE0) == 0xC0) {
		utf8 = (c & 0x1F);
		count = 1;
	} else if ((c & 0xF0) == 0xE0) {
		utf8 = (c & 0x0F);
		count = 2;
	} else if ((c & 0xF8) == 0xF0) {
		utf8 = (c & 0x07);
		count = 3;
	} else if ((c & 0xFC) == 0xF8) {
		utf8 = (c & 0x03);
		count = 4;
	} else if (( c & 0xFE) == 0xFC) {
		utf8 = (c & 0x01);
		count = 5;
	} else {
		DebugPrint("Invalid utf8\n");
		return false;
	}

	while (count--) {
		c = text[pos++];
		if ((c & 0xC0) != 0x80) {
			DebugPrint("Invalid utf8\n");
			return false;
		}
		utf8 <<= 6;
		utf8 |= (c & 0x3F);
	}

	return true;
}

/**
**  Returns the pixel width of text.
**
**  @param text  Text to calculate the width of.
**
**  @return      The width in pixels of the text.
*/
int CFont::Width(const std::string &text) const
{
	int width = 0;
	bool isformat = false;
	int utf8;
	size_t pos = 0;

	while (GetUTF8(text, pos, utf8)) {
		if (utf8 == '~') {
			if (pos >= text.size()) {  // bad formatted string
				break;
			}
			if (text[pos] == '<' || text[pos] == '>') {
				isformat = false;
				++pos;
				continue;
			}
			if (text[pos] == '!') {
				++pos;
				continue;
			}
			if (text[pos] != '~') { // ~~ -> ~
				isformat = !isformat;
				continue;
			}
		}
		if (!isformat) {
			width += this->CharWidth[utf8 - 32] + 1;
		}
	}
	return width;
}

CFont::~CFont()
{
	if (G) {
		CGraphic::Free(G);
	}
	delete[] CharWidth;
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
static int DoDrawText(int x, int y, CFont *font, const std::string &text,
	int clip)
{
	int w;
	int widths;
	CFontColor *rev;
	char *color;
	const char *p;
	void (*DrawChar)(const CGraphic *, int, int, int, int, int, int);
	int ipr;
	int c;
	CGraphic *g;
	int utf8;
	size_t pos;

	if (clip) {
		DrawChar = VideoDrawCharClip;
	} else {
		DrawChar = VideoDrawChar;
	}

#ifndef USE_OPENGL
	g = font->G;
#else
	g = FontColorGraphics[font][FontColor];
#endif
	rev = NULL;
	widths = 0;
	pos = 0;
	while (GetUTF8(text, pos, utf8)) {
		if (utf8 == '~') {
			switch (text[pos]) {
				case '\0':  // wrong formatted string.
					DebugPrint("oops, format your ~\n");
					return widths;
				case '~':
					++pos;  
					break;
				case '!':
					rev = FontColor;
					FontColor = ReverseTextColor;
#ifdef USE_OPENGL
					g = FontColorGraphics[font][FontColor];
#endif
					++pos;
					continue;
				case '<':
					LastTextColor = FontColor;
					FontColor = ReverseTextColor;
#ifdef USE_OPENGL
					g = FontColorGraphics[font][FontColor];
#endif
					++pos;
					continue;
				case '>':
					rev = LastTextColor;  // swap last and current color
					LastTextColor = FontColor;
					FontColor = rev;
#ifdef USE_OPENGL
					g = FontColorGraphics[font][FontColor];
#endif
					++pos;
					continue;

				default:
					p = text.c_str() + pos;
					while (*p && *p !='~') {
						++p;
					}
					if (!*p) {
						DebugPrint("oops, format your ~\n");
						return widths;
					}
					color = new char[p - (text.c_str() + pos) + 1];
					memcpy(color, text.c_str() + pos, p - (text.c_str() + pos));
					color[p - (text.c_str() + pos)] = '\0';
					pos = p - text.c_str() + 1;
					LastTextColor = FontColor;
					CFontColor *fc = CFontColor::Get(color);
					if (fc) {
						FontColor = fc;
#ifdef USE_OPENGL
						g = FontColorGraphics[font][fc];
#endif
					}
					delete[] color;
					continue;
			}
		}

		c = utf8 - 32;
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
			g = FontColorGraphics[font][FontColor];
#endif
			rev = NULL;
		}
	}

	return widths;
}

/**
**  Draw text with font at x,y unclipped.
**
**  @see DoDrawText
**
**  @param x     X screen position
**  @param y     Y screen position
**  @param font  Font number
**  @param text  Text to be displayed.
**
**  @return      The length of the printed text.
*/
int VideoDrawText(int x, int y, CFont *font, const std::string &text)
{
	return DoDrawText(x, y, font, text, 0);
}

/**
**  Draw text with font at x,y clipped.
**
**  @see DoDrawText.
**
**  @param x     X screen position
**  @param y     Y screen position
**  @param font  Font number
**  @param text  Text to be displayed.
**
**  @return      The length of the printed text.
*/
int VideoDrawTextClip(int x, int y, CFont *font, const std::string &text)
{
	return DoDrawText(x, y, font, text, 1);
}

/**
**  Draw reverse text with font at x,y unclipped.
**
**  @see DoDrawText for full description.
**
**  @param x     X screen position
**  @param y     Y screen position
**  @param font  Font number
**  @param text  Text to be displayed.
**
**  @return      The length of the printed text.
*/
int VideoDrawReverseText(int x, int y, CFont *font, const std::string &text)
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
**  @see DoDrawText for full description.
**
**  @param x     X screen position
**  @param y     Y screen position
**  @param font  Font number
**  @param text  Text to be displayed.
**
**  @return      The length of the printed text.
*/
int VideoDrawReverseTextClip(int x, int y, CFont *font, const std::string &text)
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
**  @see DoDrawText for full description.
**
**  @param x     X screen position
**  @param y     Y screen position
**  @param font  Font number
**  @param text  Text to be displayed.
**
**  @return      The length of the printed text.
*/
int VideoDrawTextCentered(int x, int y, CFont *font, const std::string &text)
{
	int dx;

	dx = font->Width(text);
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
**  @param font    if specified use font->Width() instead of strlen.
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
		while (font->Width(s) > maxlen) {
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
**  @param font    if specified use font->Width() instead of strlen.
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
*/
void CFont::MeasureWidths()
{
	// FIXME: todo.. can this be optimized?
	const unsigned char *sp;
	const unsigned char *lp;
	const unsigned char *gp;
	Uint32 ckey;
	int ipr;  // images per row

	int maxy = G->GraphicWidth / G->Width * G->GraphicHeight / G->Height;
	delete[] CharWidth;
	CharWidth = new char[maxy];
	memset(CharWidth, 0, maxy);
	CharWidth[0] = G->Width / 2;  // a reasonable value for SPACE
	ckey = G->Surface->format->colorkey;
	ipr = G->Surface->w / G->Width;

	SDL_LockSurface(G->Surface);
	for (int y = 1; y < maxy; ++y) {
		sp = (const unsigned char *)G->Surface->pixels +
			(y / ipr) * G->Surface->pitch * G->Height +
			(y % ipr) * G->Width - 1;
		gp = sp + G->Surface->pitch * G->Height;
		// Bail out if no letters left
		if (gp >= ((const unsigned char *)G->Surface->pixels +
				G->Surface->pitch * G->GraphicHeight)) {
			break;
		}
		while (sp < gp) {
			lp = sp + G->Width;
			for (; sp < lp; --lp) {
				if (*lp != ckey && *lp != 7) {
					if (lp - sp > CharWidth[y]) {  // max width
						CharWidth[y] = lp - sp;
					}
				}
			}
			sp += G->Surface->pitch;
		}

	}
	SDL_UnlockSurface(G->Surface);
}

/**
**  Make font bitmap.
**
**  @param font  Font number
*/
#ifdef USE_OPENGL
void MakeFontColorTextures(CFont *font)
{
	SDL_Surface *s;
	CGraphic *g;
	CGraphic *newg;

	if (!FontColorGraphics[font].empty()) {
		// already loaded
		return;
	}

	g = font->G;
	s = g->Surface;
	for (int i = 0; i < (int)AllFontColors.size(); ++i) {
		CFontColor *fc = AllFontColors[i];
		newg = FontColorGraphics[font][fc] = new CGraphic;
		newg->Width = g->Width;
		newg->Height = g->Height;
		newg->NumFrames = g->NumFrames;
		newg->GraphicWidth = g->GraphicWidth;
		newg->GraphicHeight = g->GraphicHeight;
		newg->Surface = g->Surface;

		SDL_LockSurface(s);
		for (int j = 0; j < MaxFontColors; ++j) {
			s->format->palette->colors[j] = fc->Colors[j];
		}
		SDL_UnlockSurface(s);

		MakeTexture(newg);
	}
}
#endif

/**
**  Load all fonts.
*/
void LoadFonts(void)
{
	CGraphic *g;

	for (int i = 0; i < (int)AllFonts.size(); ++i) {
		if ((g = AllFonts[i]->G)) {
			ShowLoadProgress("Fonts %s", g->File.c_str());
			g->Load();
			AllFonts[i]->MeasureWidths();
#ifdef USE_OPENGL
			MakeFontColorTextures(AllFonts[i]);
#endif
		}
	}

	// TODO: remove this
	SmallFont = CFont::Get("small");
	GameFont = CFont::Get("game");
	LargeFont = CFont::Get("large");
	SmallTitleFont = CFont::Get("small-title");
	LargeTitleFont = CFont::Get("large-title");
}

#ifdef USE_OPENGL
/**
**  Reload OpenGL fonts
*/
void ReloadFonts(void)
{
	for (int i = 0; i < (int)AllFonts.size(); ++i) {
		CFont *font = AllFonts[i];
		if (font->G) {
			for (int j = 0; j < (int)AllFontColors.size(); ++j) {
				CGraphic *g = FontColorGraphics[font][AllFontColors[j]];
				glDeleteTextures(g->NumTextures, g->Textures);
				delete[] g->Textures;
				delete g;
			}
			FontColorGraphics[font].clear();
			MakeFontColorTextures(font);
		}
	}
}
#endif

/**
**  Create a new font
**
**  @param ident  Font identifier
**  @param g      Graphic
**
**  @return       New font
*/
CFont *CFont::New(const std::string &ident, CGraphic *g)
{
	CFont *font = Fonts[ident];
	if (font) {
		if (font->G != g) {
			CGraphic::Free(font->G);
		}
		font->G = g;
	} else {
		font = new CFont(ident);
		font->G = g;
		AllFonts.push_back(font);
		Fonts[ident] = font;
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
CFont *CFont::Get(const std::string &ident)
{
	CFont *font = Fonts[ident];
	if (!font) {
		DebugPrint("font not found: %s" _C_ ident.c_str());
	}
	return font;
}

/**
**  CFontColor constructor
*/
CFontColor::CFontColor(const std::string &ident)
{
	Ident = ident;
	memset(Colors, 0, sizeof(Colors));
}

/**
**  CFontColor destructor
*/
CFontColor::~CFontColor()
{
}

/**
**  Create a new font color
**
**  @param ident  Font color identifier
**
**  @return       New font color
*/
CFontColor *CFontColor::New(const std::string &ident)
{
	CFontColor *fc = FontColors[ident];
	if (fc) {
		return fc;
	} else {
		fc = new CFontColor(ident);
		FontColors[ident] = fc;
		AllFontColors.push_back(fc);
		return fc;
	}
}

/**
**  Get a font color
**
**  @param ident  Font color identifier
**
**  @return       The font color
*/
CFontColor *CFontColor::Get(const std::string &ident)
{
	CFontColor *fc = FontColors[ident];
	if (!fc) {
		DebugPrint("font color not found: %s" _C_ ident.c_str());
	}
	return fc;
}

/**
**  Clean up the font module.
*/
void CleanFonts(void)
{
	int i;

	for (i = 0; i < (int)AllFonts.size(); ++i) {
		CFont *font = AllFonts[i];

#ifdef USE_OPENGL
		if (!FontColorGraphics[font].empty()) {
			for (int j = 0; j < (int)AllFontColors.size(); ++j) {
				CGraphic *g = FontColorGraphics[font][AllFontColors[j]];
				glDeleteTextures(g->NumTextures, g->Textures);
				delete[] g->Textures;
				delete g;
			}
			FontColorGraphics[font].clear();
		}
#endif

		delete font;
	}
#ifdef USE_OPENGL
	FontColorGraphics.clear();
#endif
	AllFonts.clear();
	Fonts.clear();

	for (i = 0; i < (int)AllFontColors.size(); ++i) {
		delete AllFontColors[i];
	}
	AllFontColors.clear();
	FontColors.clear();

	SmallFont = NULL;
	GameFont = NULL;
	LargeFont = NULL;
	SmallTitleFont = NULL;
	LargeTitleFont = NULL;
}

//@}
