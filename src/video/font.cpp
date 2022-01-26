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
//      (c) Copyright 1998-2007 by Lutz Sammer, Jimmy Salmon, Nehal Mistry
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

//@{

/*----------------------------------------------------------------------------
--  Includes
----------------------------------------------------------------------------*/

#include "stratagus.h"

#include "font.h"

#include "intern_video.h"
#include "video.h"

#include <vector>
#include <map>

/*----------------------------------------------------------------------------
--  Variables
----------------------------------------------------------------------------*/

typedef std::map<std::string, CFont *> FontMap;
static FontMap Fonts;  /// Font mappings

typedef std::map<std::string, CFontColor *> FontColorMap;
static FontColorMap FontColors;  /// Map of ident to font color.

static CFontColor *FontColor;                /// Current font color

static const CFontColor *LastTextColor;      /// Last text color
static CFontColor *DefaultTextColor;         /// Default text color
static CFontColor *ReverseTextColor;         /// Reverse text color
static std::string DefaultNormalColorIndex;  /// Default normal color index
static std::string DefaultReverseColorIndex; /// Default reverse color index

/**
**  Font color graphics
**  Usage: FontColorGraphics[CFont *font][CFontColor *color]
*/
typedef std::map<const CFontColor *, CGraphic *> FontColorGraphicMap;
static std::map<const CFont *, FontColorGraphicMap> FontColorGraphics;

// FIXME: remove these
static CFont *SmallFont;  /// Small font used in stats
static CFont *GameFont;   /// Normal font used in game

static int FormatNumber(int number, char *buf);

CFont &GetSmallFont()
{
	if (!SmallFont) {
		SmallFont = CFont::Get("small");
	}
	Assert(SmallFont);
	return *SmallFont;
}

bool IsGameFontReady()
{
	return GameFont != NULL || CFont::Get("game") != NULL;
}

CFont &GetGameFont()
{
	if (!GameFont) {
		GameFont = CFont::Get("game");
	}
	Assert(GameFont);
	return *GameFont;
}


/*----------------------------------------------------------------------------
--  Guichan Functions
----------------------------------------------------------------------------*/

/* virtual */ void CFont::drawString(gcn::Graphics *graphics, const std::string &txt, int x, int y, bool is_normal)
{
	DynamicLoad();
	const gcn::ClipRectangle &r = graphics->getCurrentClipArea();
	int right = std::min<int>(r.x + r.width - 1, Video.Width - 1);
	int bottom = std::min<int>(r.y + r.height - 1, Video.Height - 1);

	if (r.x > right || r.y > bottom) {
		return;
	}

	PushClipping();
	SetClipping(r.x, r.y, right, bottom);
	CLabel(*this).DrawClip(x + r.xOffset, y + r.yOffset, txt, is_normal);
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
static void VideoDrawChar(const CGraphic &g,
						  int gx, int gy, int w, int h, int x, int y, const CFontColor &fc)
{
	SDL_Rect srect = {Sint16(gx), Sint16(gy), Uint16(w), Uint16(h)};
	SDL_Rect drect = {Sint16(x), Sint16(y), 0, 0};
	std::vector<SDL_Color> sdlColors(fc.Colors, fc.Colors + MaxFontColors);
	SDL_SetPaletteColors(g.Surface->format->palette, &sdlColors[0], 0, MaxFontColors);
	SDL_BlitSurface(g.Surface, &srect, TheScreen, &drect);
}

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

static const unsigned char utf8_europe_to_cp437[] = {
    0xff, 0xad, 0x9b, 0x9c, 0x00, 0x9d, 0x00, 0x00, 0x00, 0x00, 0xa6, 0xae, 0xaa, 0x00, 0x00, 0x00, // 0xa0
    0xf8, 0xf1, 0xfd, 0x00, 0x00, 0xe6, 0x00, 0xfa, 0x00, 0x00, 0xa7, 0xaf, 0xac, 0xab, 0x00, 0xa8, // 0xb0
    0x00, 0x00, 0x00, 0x00, 0x8e, 0x8f, 0x92, 0x80, 0x00, 0x90, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 0xc0
    0x00, 0xa5, 0x00, 0x00, 0x00, 0x00, 0x99, 0x00, 0x00, 0x00, 0x00, 0x00, 0x9a, 0x00, 0x00, 0xe1, // 0xd0
    0x85, 0xa0, 0x83, 0x00, 0x84, 0x86, 0x91, 0x87, 0x8a, 0x82, 0x88, 0x89, 0x8d, 0xa1, 0x8c, 0x8b, // 0xe0
    0x00, 0xa4, 0x95, 0xa2, 0x93, 0x00, 0x94, 0xf6, 0x00, 0x97, 0xa3, 0x96, 0x81, 0x00, 0x00, 0x98, // 0xf0
};

static const unsigned char utf8_europe_to_cp1252[] = {
    0xa0, 0xa1, 0xa2, 0xa3, 0xa4, 0xa5, 0xa6, 0xa7, 0xa8, 0xa9, 0xaa, 0xab, 0xac, 0xad, 0xae, 0xaf, // 0xa0
    0xb0, 0xb1, 0xb2, 0xb3, 0xb4, 0xb5, 0xb6, 0xb7, 0xb8, 0xb9, 0xba, 0xbb, 0xbc, 0xbd, 0xbe, 0xbf, // 0xb0
    0xc0, 0xc1, 0xc2, 0xc3, 0xc4, 0xc5, 0xc6, 0xc7, 0xc8, 0xc9, 0xca, 0xcb, 0xcc, 0xcd, 0xce, 0xcf, // 0xc0
    0xd0, 0xd1, 0xd2, 0xd3, 0xd4, 0xd5, 0xd6, 0xd7, 0xd8, 0xd9, 0xda, 0xdb, 0xdc, 0xdd, 0xde, 0xdf, // 0xd0
    0xe0, 0xe1, 0xe2, 0xe3, 0xe4, 0xe5, 0xe6, 0xe7, 0xe8, 0xe9, 0xea, 0xeb, 0xec, 0xed, 0xee, 0xef, // 0xe0
    0xf0, 0xf1, 0xf2, 0xf3, 0xf4, 0xf5, 0xf6, 0xf7, 0xf8, 0xf9, 0xfa, 0xfb, 0xfc, 0xfd, 0xfe, 0xff, // 0xf0
};

static const unsigned char utf8_europe_to_ascii[] = {
    0x20, 0x21, 0x43, 0x50, 0x24, 0x59, 0x7c, 0x53, 0x22, 0x28, 0x61, 0x3c, 0x21, 0x00, 0x28, 0x2d, // 0xa0
    0x64, 0x2b, 0x32, 0x33, 0x27, 0x75, 0x50, 0x2a, 0x2c, 0x31, 0x6f, 0x3e, 0x20, 0x20, 0x20, 0x3f, // 0xb0
    0x41, 0x41, 0x41, 0x41, 0x41, 0x41, 0x41, 0x43, 0x45, 0x45, 0x45, 0x45, 0x49, 0x49, 0x49, 0x49, // 0xc0
    0x44, 0x4e, 0x4f, 0x4f, 0x4f, 0x4f, 0x4f, 0x78, 0x4f, 0x55, 0x55, 0x55, 0x55, 0x59, 0x54, 0x73, // 0xd0
    0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x63, 0x65, 0x65, 0x65, 0x65, 0x69, 0x69, 0x69, 0x69, // 0xe0
    0x64, 0x6e, 0x6f, 0x6f, 0x6f, 0x6f, 0x6f, 0x2f, 0x6f, 0x75, 0x75, 0x75, 0x75, 0x79, 0x74, 0x79, // 0xf0
};

static const unsigned char utf8_cyrillic_to_ascii[] = {
    0x49, 0x49, 0x44, 0x47, 0x49, 0x44, 0x49, 0x59, 0x4a, 0x4c, 0x4e, 0x54, 0x4b, 0x49, 0x55, 0x44, // 0x400
    0x41, 0x42, 0x56, 0x47, 0x44, 0x45, 0x5a, 0x5a, 0x49, 0x49, 0x4b, 0x4c, 0x4d, 0x4e, 0x4f, 0x50, // 0x410
    0x52, 0x53, 0x54, 0x55, 0x46, 0x4b, 0x54, 0x43, 0x53, 0x53, 0x27, 0x59, 0x27, 0x45, 0x49, 0x49, // 0x420
    0x61, 0x62, 0x76, 0x67, 0x64, 0x65, 0x7a, 0x7a, 0x69, 0x69, 0x6b, 0x6c, 0x6d, 0x6e, 0x6f, 0x70, // 0x430
    0x72, 0x73, 0x74, 0x75, 0x66, 0x6b, 0x74, 0x63, 0x73, 0x73, 0x27, 0x79, 0x27, 0x65, 0x69, 0x69, // 0x440
    0x69, 0x69, 0x64, 0x67, 0x69, 0x64, 0x69, 0x79, 0x6a, 0x6c, 0x6e, 0x74, 0x6b, 0x69, 0x75, 0x64, // 0x450
};

static const unsigned char utf8_cyrillic_to_cp866[] = {
    0x00, 0xf0, 0x00, 0x00, 0xf2, 0x00, 0x00, 0xf4, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xf6, 0x00, // 0x400
    0x80, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87, 0x88, 0x89, 0x8a, 0x8b, 0x8c, 0x8d, 0x8e, 0x8f, // 0x410
    0x90, 0x91, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97, 0x98, 0x99, 0x9a, 0x9b, 0x9c, 0x9d, 0x9e, 0x9f, // 0x420
    0xa0, 0xa1, 0xa2, 0xa3, 0xa4, 0xa5, 0xa6, 0xa7, 0xa8, 0xa9, 0xaa, 0xab, 0xac, 0xad, 0xae, 0xaf, // 0x430
    0xe0, 0xe1, 0xe2, 0xe3, 0xe4, 0xe5, 0xe6, 0xe7, 0xe8, 0xe9, 0xea, 0xeb, 0xec, 0xed, 0xee, 0xef, // 0x440
    0x00, 0xf1, 0x00, 0x00, 0xf3, 0x00, 0x00, 0xf5, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xf7, 0x00, // 0x450
};

// default to codepage 437
int FontCodePage = 437;

/**
** Convert a UTF8 char to an ASCII-extended char, based on the
** convert table to work with Stratagus char. This maps UTF8
** chars into codepages commonly used for DOS and early Windows
** games; currently CP437, CP1251, and CP866. More can be added.
** The games need to ensure that the font graphic represents the
** appropriate codepage.
**
** @param utf8 the char to convert
** @return the codepage equivalent, or '?' if char is unsuported.
*/
static int utf8_to_codepage(int utf8)
{
	Assert(utf8 >= 0x80);

	int cpChar = 0;

	switch (utf8) {
		case 0xa1: // special case: print inverted exclamation mark as normal one
			cpChar = '!';
			break;
		case 0x192: // index for this letter in cp437 is 0x9f
			cpChar = 0x9f;
			break;
		case 0x2502:
			cpChar = 0xb3;
		case 0x2591:
			cpChar = 0xb0;
			break;
		case 0x2592:
			cpChar = 0xb1;
			break;
		case 0x2593:
			cpChar = 0xb2;
			break;
		default:
			utf8 -= 0xa0;
			if (utf8 >= 0 && utf8 < sizeof(utf8_europe_to_cp437)) {
				// western european
				switch (FontCodePage) {
					case 437:
						cpChar = utf8_europe_to_cp437[utf8];
						break;
					case 1252:
						cpChar = utf8_europe_to_cp1252[utf8];
						break;
					default:
						cpChar = utf8_europe_to_ascii[utf8];
						break;
				}
			} else {
				utf8 -= (0x400 - 0xa0);
				if (utf8 >= 0 && utf8 < sizeof(utf8_cyrillic_to_cp866)) {
					// cyrillic
					switch (FontCodePage) {
						case 866:
							cpChar = utf8_cyrillic_to_cp866[utf8];
							break;
						default:
							cpChar = utf8_cyrillic_to_ascii[utf8];
							break;
					}
				}
			}
			break;
	}

	if (cpChar < 0x32) {
		fprintf(stderr, "Can't convert UTF8 char to codepage %d: '%c' d=%d (0x%04x)\r\n", FontCodePage, utf8, utf8, utf8);
		cpChar = '?';
	}
	return cpChar;
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
	} else if ((c & 0xFE) == 0xFC) {
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

	int ascii = utf8_to_codepage(utf8);
	utf8 = ascii;

	return true;
}

/**
**  Get the next utf8 character from an array of chars
*/
static bool GetUTF8(const char text[], const size_t len, size_t &pos, int &utf8)
{
	// end of string
	if (pos >= len) {
		return false;
	}

	int count;
	char c = text[pos++];

	// ascii
	if (!(c & 0x80)) {
		utf8 = c < 32 ? '?' : c;
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
	} else if ((c & 0xFE) == 0xFC) {
		utf8 = (c & 0x01);
		count = 5;
	} else {
		DebugPrint("Invalid utf8 I %c <%s> [%lu]\n" _C_ c _C_ text _C_(long) pos);
		return false;
	}

	while (count--) {
		c = text[pos++];
		if ((c & 0xC0) != 0x80) {
			DebugPrint("Invalid utf8 II\n");
			return false;
		}
		utf8 <<= 6;
		utf8 |= (c & 0x3F);
	}
	
	int ascii = utf8_to_codepage(utf8);
	utf8 = ascii;

	return true;
}


int CFont::Height() const
{
	DynamicLoad();
	return G->Height;
}

bool CFont::IsLoaded() const
{
	return G && G->IsLoaded();
}

/**
**  Returns the pixel width of text.
**
**  @param number  number to calculate the width of.
**
**  @return      The width in pixels of the text.
*/
int CFont::Width(const int number) const
{
	int width = 0;
#if 0
	bool isformat = false;
#endif
	int utf8;
	size_t pos = 0;
	char text[ sizeof(int) * 10 + 2];
	const int len = FormatNumber(number, text);

	DynamicLoad();
	while (GetUTF8(text, len, pos, utf8)) {
#if 0
		if (utf8 == '~') {
			//if (pos >= text.size()) {  // bad formatted string
			if (pos >= size) {  // bad formatted string
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
#else
		width += this->CharWidth[utf8 - 32] + 1;
#endif
	}
	return width;
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

	DynamicLoad();
	while (GetUTF8(text, pos, utf8)) {
		if (utf8 == '~') {
			if (text[pos] == '|') {
				++pos;
				continue;
			}
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

extern int convertKey(const char *key);

/**
**  Get the hot key from a string
*/
int GetHotKey(const std::string &text)
{
	int hotkey = 0;
	size_t pos = 0;

	if (text.length() > 1) {
		hotkey = convertKey(text.c_str());
	} else if (text.length() == 1) {
		GetUTF8(text, pos, hotkey);
	}

	return hotkey;
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
static void VideoDrawCharClip(const CGraphic &g, int gx, int gy, int w, int h,
							  int x, int y, const CFontColor &fc)
{
	int ox;
	int oy;
	int ex;
	CLIP_RECTANGLE_OFS(x, y, w, h, ox, oy, ex);
	UNUSED(ex);
	VideoDrawChar(g, gx + ox, gy + oy, w, h, x, y, fc);
}


template<bool CLIP>
unsigned int CFont::DrawChar(CGraphic &g, int utf8, int x, int y, const CFontColor &fc) const
{
	int c = utf8 - 32;
	Assert(c >= 0);
	const int ipr = this->G->GraphicWidth / this->G->Width;

	if (c < 0 || ipr * this->G->GraphicHeight / this->G->Height <= c) {
		c = 0;
	}
	const int w = this->CharWidth[c];
	const int gx = (c % ipr) * this->G->Width;
	const int gy = (c / ipr) * this->G->Height;

	if (CLIP) {
		VideoDrawCharClip(g, gx, gy, w, this->G->Height, x , y, fc);
	} else {
		VideoDrawChar(g, gx, gy, w, this->G->Height, x, y, fc);
	}
	return w + 1;
}

CGraphic *CFont::GetFontColorGraphic(const CFontColor &fontColor) const
{
	return this->G;
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
template <const bool CLIP>
int CLabel::DoDrawText(int x, int y,
					   const char *const text, const size_t len, const CFontColor *fc) const
{
	int widths = 0;
	int utf8;
	bool tab;
	const int tabSize = 4; // FIXME: will be removed when text system will be rewritten
	size_t pos = 0;
	const CFontColor *backup = fc;
	bool isColor = false;
	font->DynamicLoad();
	CGraphic *g = font->GetFontColorGraphic(*FontColor);

	while (GetUTF8(text, len, pos, utf8)) {
		tab = false;
		if (utf8 == '\t') {
			tab = true;
		} else if (utf8 == '~') {
			switch (text[pos]) {
				case '\0':  // wrong formatted string.
					DebugPrint("oops, format your ~\n");
					return widths;
				case '~':
					++pos;
					break;
				case '|':
					++pos;
					continue;
				case '!':
					if (fc != reverse) {
						fc = reverse;
						g = font->GetFontColorGraphic(*fc);
					}
					++pos;
					continue;
				case '<':
					LastTextColor = fc;
					if (fc != reverse) {
						isColor = true;
						fc = reverse;
						g = font->GetFontColorGraphic(*fc);
					}
					++pos;
					continue;
				case '>':
					if (fc != LastTextColor) {
						std::swap(fc, LastTextColor);
						isColor = false;
						g = font->GetFontColorGraphic(*fc);
					}
					++pos;
					continue;

				default: {
					const char *p = text + pos;
					while (*p && *p != '~') {
						++p;
					}
					if (!*p) {
						DebugPrint("oops, format your ~\n");
						return widths;
					}
					std::string color;

					color.insert(0, text + pos, p - (text + pos));
					pos = p - text + 1;
					LastTextColor = fc;
					const CFontColor *fc_tmp = CFontColor::Get(color);
					if (fc_tmp) {
						isColor = true;
						fc = fc_tmp;
						g = font->GetFontColorGraphic(*fc);
					}
					continue;
				}
			}
		}
		if (tab) {
			for (int tabs = 0; tabs < tabSize; ++tabs) {
				widths += font->DrawChar<CLIP>(*g, ' ', x + widths, y, *fc);
			}
		} else {
			widths += font->DrawChar<CLIP>(*g, utf8, x + widths, y, *fc);
		}

		if (isColor == false && fc != backup) {
			fc = backup;
			g = font->GetFontColorGraphic(*fc);
		}
	}
	return widths;
}


CLabel::CLabel(const CFont &f) :
	normal(DefaultTextColor),
	reverse(ReverseTextColor),
	font(&f)
{
}

/// Draw text/number unclipped
int CLabel::Draw(int x, int y, const char *const text) const
{
	return DoDrawText<false>(x, y, text, strlen(text), normal);
}

int CLabel::Draw(int x, int y, const std::string &text) const
{
	return DoDrawText<false>(x, y, text.c_str(), text.size(), normal);
}

int CLabel::Draw(int x, int y, int number) const
{
	char buf[sizeof(int) * 10 + 2];
	size_t len = FormatNumber(number, buf);
	return DoDrawText<false>(x, y, buf, len, normal);
}

/// Draw text/number clipped
int CLabel::DrawClip(int x, int y, const char *const text) const
{
	return DoDrawText<true>(x, y, text, strlen(text), normal);
}

int CLabel::DrawClip(int x, int y, const std::string &text, bool is_normal) const
{
	// return DoDrawText<true>(x, y, text.c_str(), text.size(), normal);
	if (is_normal) {
	    return DoDrawText<true>(x, y, text.c_str(), text.size(), normal);
	}
	else
	{
	    return DoDrawText<true>(x, y, text.c_str(), text.size(), reverse);
	}
}

int CLabel::DrawClip(int x, int y, int number) const
{
	char buf[sizeof(int) * 10 + 2];
	size_t len = FormatNumber(number, buf);
	return DoDrawText<true>(x, y, buf, len, normal);
}


/// Draw reverse text/number unclipped
int CLabel::DrawReverse(int x, int y, const char *const text) const
{
	return DoDrawText<false>(x, y, text, strlen(text), reverse);
}

int CLabel::DrawReverse(int x, int y, const std::string &text) const
{
	return DoDrawText<false>(x, y, text.c_str(), text.size(), reverse);
}

int CLabel::DrawReverse(int x, int y, int number) const
{
	char buf[sizeof(int) * 10 + 2];
	size_t len = FormatNumber(number, buf);
	return DoDrawText<false>(x, y, buf, len, reverse);
}

/// Draw reverse text/number clipped
int CLabel::DrawReverseClip(int x, int y, const char *const text) const
{
	return DoDrawText<true>(x, y, text, strlen(text), reverse);
}

int CLabel::DrawReverseClip(int x, int y, const std::string &text) const
{
	return DoDrawText<true>(x, y, text.c_str(), text.size(), reverse);
}

int CLabel::DrawReverseClip(int x, int y, int number) const
{
	char buf[sizeof(int) * 10 + 2];
	size_t len = FormatNumber(number, buf);
	return DoDrawText<true>(x, y, buf, len, reverse);
}

int CLabel::DrawCentered(int x, int y, const std::string &text) const
{
	int dx = font->Width(text);
	DoDrawText<false>(x - dx / 2, y, text.c_str(), text.size(), normal);
	return dx / 2;
}

int CLabel::DrawReverseCentered(int x, int y, const std::string &text) const
{
	int dx = font->Width(text);
	DoDrawText<false>(x - dx / 2, y, text.c_str(), text.size(), reverse);
	return dx / 2;
}


/**
**  Format a number using commas
**
**  @param number  Number to be formatted
**  @param buf     Buffer to save the formatted number to
**
**  @return      The real length of the Formated Number.
*/
static int FormatNumber(int number, char *buf)
{
	const char sep = ',';
	char bufs[sizeof(int) * 10 + 2];
	int s = 0;
	int d = number < 0 ? 1 : 0;
	const int sl = snprintf(bufs, sizeof(bufs), "%d", abs(number));

	while (s <= sl) {
		if (s > 0 && s < sl && (s - (sl % 3)) % 3 == 0) {
			buf[d++] = sep;
		}
		buf[d++] = bufs[s++];
	}
	buf[0] = number < 0 ? '-' : buf[0];
	return d - 1;
}

/**
**  Return the index of first occurrence of c in [s- s + maxlen]
**
**  @param s       original string.
**  @param c       character to find.
**  @param maxlen  size limit of the search. (0 means unlimited). (in char if font == NULL else in pixels).
**  @param font    if specified use font->Width() instead of strlen.
**
**  @return computed value.
*/
static int strchrlen(const std::string &s, char c, unsigned int maxlen, const CFont *font)
{
	if (s.empty()) {
		return 0;
	}
	int res = s.find(c);
	res = (res == -1) ? s.size() : res;

	if (!maxlen || (!font && (unsigned int) res < maxlen) || (font && (unsigned int) font->Width(s.substr(0, res)) < maxlen)) {
		return res;
	}
	if (!font) {
		res = s.rfind(' ', maxlen);
		if (res == -1) {
			// line too long
			return maxlen;
		} else {
			return res;
		}
	} else {
		res = s.rfind(' ', res);
		while (res != -1 && (unsigned int) font->Width(s.substr(0, res)) > maxlen) {
			res = s.rfind(' ', res - 1);
		}
		if (res == -1) {
			// Line too long.
			// FIXME.
		} else {
			return res;
		}
	}
	return res;
}

/**
**  Return the 'line' line of the string 's'.
**
**  @param line    line number.
**  @param s       multiline string.
**  @param maxlen  max length of the string (0 : unlimited) (in char if font == NULL else in pixels).
**  @param font    if specified use font->Width() instead of strlen.
**
**  @return computed value.
*/
std::string GetLineFont(unsigned int line, const std::string &s, unsigned int maxlen, const CFont *font)
{
	unsigned int res;
	std::string s1 = s;

	Assert(0 < line);

	for (unsigned int i = 1; i < line; ++i) {
		res = strchrlen(s1, '\n', maxlen, font);
		if (!res || res >= s1.size()) {
			return "";
		}
		s1 = s1.substr(res + 1);
	}
	res = strchrlen(s1, '\n', maxlen, font);
	return s1.substr(0, res);
}


/**
**  Calculate the width of each character
*/
void CFont::MeasureWidths()
{
	const int maxy = G->GraphicWidth / G->Width * G->GraphicHeight / G->Height;

	delete[] CharWidth;
	CharWidth = new char[maxy];
	memset(CharWidth, 0, maxy);
	CharWidth[0] = G->Width / 2;  // a reasonable value for SPACE
	Uint32 ckey = 0;
	const int ipr = G->Surface->w / G->Width; // images per row

	SDL_LockSurface(G->Surface);
	SDL_GetColorKey(G->Surface, &ckey);
	for (int y = 1; y < maxy; ++y) {
		const unsigned char *sp = (const unsigned char *)G->Surface->pixels +
								  (y / ipr) * G->Surface->pitch * G->Height +
								  (y % ipr) * G->Width - 1;
		const unsigned char *gp = sp + G->Surface->pitch * G->Height;
		// Bail out if no letters left
		if (gp >= ((const unsigned char *)G->Surface->pixels +
				   G->Surface->pitch * G->GraphicHeight)) {
			break;
		}
		while (sp < gp) {
			// Some accented glyphes are not perfectly aligned on the glyph grid
			// (like ï or î ).  So we could do -1 to not compute width on the
			// next glyph, but that breaks other fonts like the one used in
			// war1gus. TODO: figure out what to do
			const unsigned char *lp = sp + G->Width;

			for (; sp < lp; --lp) {
				if (*lp != ckey && *lp != 7) {
					CharWidth[y] = std::max<char>(CharWidth[y], lp - sp);
				}
			}
			sp += G->Surface->pitch;
		}
	}
	SDL_UnlockSurface(G->Surface);
}

void CFont::Load()
{
	if (this->IsLoaded()) {
		return;
	}

	if (this->G) {
		this->G->Load();
		this->MeasureWidths();
	}
}

void CFont::DynamicLoad() const
{
	const_cast<CFont *>(this)->Load();
	if (this->CharWidth == 0) {
		const_cast<CFont *>(this)->MeasureWidths();
	}
}


/**
**  Load all fonts.
*/
void LoadFonts()
{
	for (FontMap::iterator it = Fonts.begin(); it != Fonts.end(); ++it) {
		CFont &font = *it->second;
		font.Load();
	}

	// TODO: remove this
	SmallFont = CFont::Get("small");
	GameFont = CFont::Get("game");
}

void CFont::Reload() const
{
	if (this->G) {
		FontColorGraphicMap &fontColorGraphicMap = FontColorGraphics[this];
		for (FontColorGraphicMap::iterator it = fontColorGraphicMap.begin();
			 it != fontColorGraphicMap.end(); ++it) {
			CGraphic *g = it->second;
			delete g;
		}
		fontColorGraphicMap.clear();
	}
}


/**
**  Reload fonts
*/
void ReloadFonts()
{
	for (FontMap::iterator it = Fonts.begin(); it != Fonts.end(); ++it) {
		CFont &font = *it->second;

		font.Reload();
	}
}

/**
**  Create a new font
**
**  @param ident  Font identifier
**  @param g      Graphic
**
**  @return       New font
*/
/* static */ CFont *CFont::New(const std::string &ident, CGraphic *g)
{
	CFont *&font = Fonts[ident];
	if (font) {
		if (font->G != g) {
			CGraphic::Free(font->G);
		}
	} else {
		font = new CFont(ident);
	}
	font->G = g;
	return font;
}

/**
**  Get a font
**
**  @param ident  Font identifier
**
**  @return       The font
*/
/* static */ CFont *CFont::Get(const std::string &ident)
{
	std::map<std::string, CFont *>::iterator it = Fonts.find(ident);
	if (it == Fonts.end()) {
		DebugPrint("font not found: %s\n" _C_ ident.c_str());
		return NULL;
	}
	CFont *font = it->second;
	if (font == NULL) {
		DebugPrint("font not found: %s\n" _C_ ident.c_str());
		return NULL;
	}
	return font;
}

CFontColor::CFontColor(const std::string &ident)
{
	Ident = ident;
}

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
/* static */ CFontColor *CFontColor::New(const std::string &ident)
{
	CFontColor *&fc = FontColors[ident];

	if (fc == NULL) {
		fc = new CFontColor(ident);
	}
	return fc;
}

/**
**  Get a font color
**
**  @param ident  Font color identifier
**
**  @return       The font color
*/
/* static */ CFontColor *CFontColor::Get(const std::string &ident)
{
	CFontColor *fc = FontColors[ident];
	if (!fc) {
		DebugPrint("font color not found: %s\n" _C_ ident.c_str());
	}
	return fc;
}

void CFont::Clean()
{
}

/**
**  Clean up the font module.
*/
void CleanFonts()
{
	for (FontMap::iterator it = Fonts.begin(); it != Fonts.end(); ++it) {
		CFont *font = it->second;

		font->Clean();
		delete font;
	}
	Fonts.clear();

	for (FontColorMap::iterator it = FontColors.begin(); it != FontColors.end(); ++it) {
		delete it->second;
	}
	FontColors.clear();

	SmallFont = NULL;
	GameFont = NULL;
}

//@}
