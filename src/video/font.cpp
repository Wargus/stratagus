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

/**
**  Font color graphics
**  Usage: FontColorGraphics[CFont *font][CFontColor *color]
*/
static std::map< const CFont *, std::map<const CFontColor *, CGraphic *> > FontColorGraphics;

// FIXME: remove these
static CFont *SmallFont;       /// Small font used in stats
static CFont *GameFont;        /// Normal font used in game

static int FormatNumber(int number, char *buf);


CFont *GetSmallFont()
{
	if (!SmallFont) {
		SmallFont = CFont::Get("small");
	}
	return SmallFont;
}

CFont *GetGameFont()
{
	if (!GameFont) {
		GameFont = CFont::Get("game");
	}
	return GameFont;
}


/*----------------------------------------------------------------------------
--  Guichan Functions
----------------------------------------------------------------------------*/

void CFont::drawString(gcn::Graphics *graphics, const std::string &txt, int x, int y)
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
	CLabel(this).DrawClip(x + r.xOffset, y + r.yOffset, txt);
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
static void VideoDrawChar(const CGraphic *g,
						  int gx, int gy, int w, int h, int x, int y, const CFontColor *fc)
{
	if (!UseOpenGL) {
		SDL_Rect srect = {gx, gy, w, h};
		SDL_Rect drect = {x, y, 0, 0};

		SDL_SetColors(g->Surface, (SDL_Color *)fc->Colors, 0, MaxFontColors);
		SDL_BlitSurface(g->Surface, &srect, TheScreen, &drect);
	} else {
		g->DrawSub(gx, gy, w, h, x, y);
	}
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

	return true;
}

/**
**  Get the next utf8 character from a array of chars
*/
static inline bool GetUTF8(const char text[], const size_t len, size_t &pos, int &utf8)
{
	// end of string
	if (pos >= len) {
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
**  @param text  Text to calculate the width of.
**
**  @return      The width in pixels of the text.
*/
int CFont::Width(const int number) const
{
	int width = 0;
	//bool isformat = false;
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
#endif
			width += this->CharWidth[utf8 - 32] + 1;
			//}
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
	int CFont::Width(const std::string & text) const {
		int width = 0;
		bool isformat = false;
		int utf8;
		size_t pos = 0;

		DynamicLoad();
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

	extern int convertKey(const char * key);

	/**
	**  Get the hot key from a string
	*/
	int GetHotKey(const std::string & text) {
		int hotkey = 0;
		int utf8;
		size_t pos = 0;

		while (GetUTF8(text, pos, utf8)) {
			if (utf8 == '~') {
				if (pos >= text.size()) {
					break;
				}
				if (text[pos] == '<') {
					++pos;
					size_t endpos = pos;
					while (endpos < text.size()) {
						if (text[endpos] == '~') {
							break;
						}
						++endpos;
					}
					std::string key = text.substr(pos, endpos - pos);
					hotkey = convertKey(key.c_str());
					break;
				}
				if (text[pos] == '!') {
					++pos;
					if (pos >= text.size()) {
						break;
					}
					GetUTF8(text, pos, utf8);
					hotkey = utf8;
					break;
				}
			}
		}

		return hotkey;
	}

	CFont::~CFont() {
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
	static void VideoDrawCharClip(const CGraphic * g, int gx, int gy, int w, int h,
								  int x, int y, const CFontColor * fc) {
		int ox;
		int oy;
		int ex;
		CLIP_RECTANGLE_OFS(x, y, w, h, ox, oy, ex);
		UNUSED(ex);
		VideoDrawChar(g, gx + ox, gy + oy, w, h, x, y, fc);
	}


	template<bool CLIP>
	unsigned int CFont::DrawChar(CGraphic * g, int utf8, int x, int y, const CFontColor * fc) const {
		const CFont *font = this;
		int c = utf8 - 32;
		Assert(c >= 0);
		const int ipr = font->G->GraphicWidth / font->G->Width;

		if (c < 0 || ipr * font->G->GraphicHeight / font->G->Height <= c) {
			c = 0;
		}
		const int w = font->CharWidth[c];
		const int gx = (c % ipr) * font->G->Width;
		const int gy = (c / ipr) * font->G->Height;

		if (CLIP) {
			VideoDrawCharClip(g, gx, gy, w, font->G->Height, x , y, fc);
		} else {
			VideoDrawChar(g, gx, gy, w, font->G->Height, x, y, fc);
		}
		return w + 1;
	}

	CGraphic *CFont::GetFontColorGraphic(const CFontColor * fontColor) const {
		if (!UseOpenGL) {
			return this->G;
		} else {
			return FontColorGraphics[this][fontColor];
		}
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
						   const char * const text, const size_t len, const CFontColor * fc) const {
		int widths = 0;
		std::string color;
		int utf8;
		size_t pos = 0;
		const CFontColor *backup = fc;
		font->DynamicLoad();
		CGraphic *g = font->GetFontColorGraphic(FontColor);

		while (GetUTF8(text, len, pos, utf8)) {
			if (utf8 == '~') {
				switch (text[pos]) {
					case '\0':  // wrong formatted string.
						DebugPrint("oops, format your ~\n");
						return widths;
					case '~':
						++pos;
						break;
					case '!':
						if (fc != reverse) {
							fc = reverse;
							g = font->GetFontColorGraphic(fc);
						}
						++pos;
						continue;
					case '<':
						LastTextColor = (CFontColor *)fc;
						if (fc != reverse) {
							fc = reverse;
							g = font->GetFontColorGraphic(fc);
						}
						++pos;
						continue;
					case '>':
						if (fc != LastTextColor) {
							const CFontColor *rev = LastTextColor;  // swap last and current color
							LastTextColor = (CFontColor *)fc;
							fc = rev;
							g = font->GetFontColorGraphic(fc);
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
						color.insert(0, text + pos, p - (text + pos));
						color[p - (text + pos)] = '\0';
						pos = p - text + 1;
						LastTextColor = (CFontColor *)fc;
						const CFontColor *fc_tmp = CFontColor::Get(color);
						if (fc_tmp) {
							fc = fc_tmp;
							g = font->GetFontColorGraphic(fc);
						}
						continue;
					}
				}
			}

			widths += font->DrawChar<CLIP>(g, utf8, x + widths, y, fc);

			if (fc != backup) {
				fc = backup;
				if (UseOpenGL) {
					g = FontColorGraphics[font][fc];
				}
			}
		}

		return widths;
	}


	CLabel::CLabel(const CFont * f) :
		normal(DefaultTextColor),
		reverse(ReverseTextColor),
		font(f) {
	}

	/// Draw text/number unclipped
	int CLabel::Draw(int x, int y, const char * const text) const {
		return DoDrawText<false>(x, y, text, strlen(text), normal);
	}

	int CLabel::Draw(int x, int y, const std::string & text) const {
		return DoDrawText<false>(x, y, text.c_str(), text.size(), normal);
	}

	int CLabel::Draw(int x, int y, int number) const {
		char buf[sizeof(int) * 10 + 2];
		size_t len = FormatNumber(number, buf);
		return DoDrawText<false>(x, y, buf, len, normal);
	}

	/// Draw text/number clipped
	int CLabel::DrawClip(int x, int y, const char * const text) const {
		return DoDrawText<true>(x, y, text, strlen(text), normal);
	}

	int CLabel::DrawClip(int x, int y, const std::string & text) const {
		return DoDrawText<true>(x, y, text.c_str(), text.size(), normal);
	}

	int CLabel::DrawClip(int x, int y, int number) const {
		char buf[sizeof(int) * 10 + 2];
		size_t len = FormatNumber(number, buf);
		return DoDrawText<true>(x, y, buf, len, normal);
	}


	/// Draw reverse text/number unclipped
	int CLabel::DrawReverse(int x, int y, const char * const text) const {
		return DoDrawText<false>(x, y, text, strlen(text), reverse);
	}

	int CLabel::DrawReverse(int x, int y, const std::string & text) const {
		return DoDrawText<false>(x, y, text.c_str(), text.size(), reverse);
	}

	int CLabel::DrawReverse(int x, int y, int number) const {
		char buf[sizeof(int) * 10 + 2];
		size_t len = FormatNumber(number, buf);
		return DoDrawText<false>(x, y, buf, len, reverse);
	}

	/// Draw reverse text/number clipped
	int CLabel::DrawReverseClip(int x, int y, const char * const text) const {
		return DoDrawText<true>(x, y, text, strlen(text), reverse);
	}

	int CLabel::DrawReverseClip(int x, int y, const std::string & text) const {
		return DoDrawText<true>(x, y, text.c_str(), text.size(), reverse);
	}

	int CLabel::DrawReverseClip(int x, int y, int number) const {
		char buf[sizeof(int) * 10 + 2];
		size_t len = FormatNumber(number, buf);
		return DoDrawText<true>(x, y, buf, len, reverse);
	}

	int CLabel::DrawCentered(int x, int y, const std::string & text) const {
		int dx = font->Width(text);
		DoDrawText<false>(x - dx / 2, y, text.c_str(), text.size(), normal);
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
	static int FormatNumber(int number, char * buf) {
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
	**  Return the index of first occurance of c in [s- s + maxlen]
	**
	**  @param s       original string.
	**  @param c       character to find.
	**  @param maxlen  size limit of the search. (0 means unlimited). (in char if font == NULL else in pixels).
	**  @param font    if specified use font->Width() instead of strlen.
	**
	**  @return computed value.
	*/
	static int strchrlen(const std::string & s, char c, unsigned int maxlen, CFont * font) {
		if (s.empty()) {
			return 0;
		}
		int res = s.find(c);
		res = (res == -1) ? s.size() : res - 1;

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
	std::string GetLineFont(unsigned int line, const std::string & s, unsigned int maxlen, CFont * font) {
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
	void CFont::MeasureWidths() {
		const int maxy = G->GraphicWidth / G->Width * G->GraphicHeight / G->Height;

		delete[] CharWidth;
		CharWidth = new char[maxy];
		memset(CharWidth, 0, maxy);
		CharWidth[0] = G->Width / 2;  // a reasonable value for SPACE
		const Uint32 ckey = G->Surface->format->colorkey;
		const int ipr = G->Surface->w / G->Width; // images per row

		SDL_LockSurface(G->Surface);
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
				const unsigned char *lp = sp + G->Width;

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
	*/
	void CFont::MakeFontColorTextures() const {
		if (!FontColorGraphics[this].empty()) {
			// already loaded
			return;
		}
		const CGraphic *g = this->G;
		SDL_Surface *s = g->Surface;

		for (unsigned int i = 0; i < AllFontColors.size(); ++i) {
			CFontColor *fc = AllFontColors[i];
			CGraphic *newg = FontColorGraphics[this][fc] = new CGraphic;

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

	void CFont::Load() {
		if (this->IsLoaded()) {
			return;
		}

		if (this->G) {
			//		ShowLoadProgress("Fonts %s", this->G->File.c_str());
			this->G->Load();
			this->MeasureWidths();

			if (UseOpenGL) {
				this->MakeFontColorTextures();
			}
		}
	}

	void CFont::DynamicLoad() const {
		const_cast<CFont *>(this)->Load();
		if (this->CharWidth == 0) {
			const_cast<CFont *>(this)->MeasureWidths();
		}
	}


	/**
	**  Load all fonts.
	*/
	void LoadFonts() {
		for (unsigned int i = 0; i < AllFonts.size(); ++i) {
			AllFonts[i]->Load();
		}

		// TODO: remove this
		SmallFont = CFont::Get("small");
		GameFont = CFont::Get("game");
	}

	void CFont::FreeOpenGL() {
		if (this->G) {
			for (unsigned int j = 0; j < AllFontColors.size(); ++j) {
				CGraphic *g = FontColorGraphics[this][AllFontColors[j]];
				glDeleteTextures(g->NumTextures, g->Textures);
			}
		}
	}

	/**
	**  Free OpenGL fonts
	*/
	void FreeOpenGLFonts() {
		for (unsigned int i = 0; i < AllFonts.size(); ++i) {
			CFont *font = AllFonts[i];

			font->FreeOpenGL();
		}
	}


	void CFont::Reload() const {
		if (this->G) {
			for (unsigned int j = 0; j < AllFontColors.size(); ++j) {
				//CGraphic::Free(FontColorGraphics[this][AllFontColors[j]]);
				CGraphic *g = FontColorGraphics[this][AllFontColors[j]];
				delete[] g->Textures;
				delete g;
			}
			FontColorGraphics[this].clear();
			this->MakeFontColorTextures();
		}
	}


	/**
	**  Reload OpenGL fonts
	*/
	void ReloadFonts() {
		for (unsigned int i = 0; i < AllFonts.size(); ++i) {
			const CFont *font = AllFonts[i];

			font->Reload();
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
	CFont *CFont::New(const std::string & ident, CGraphic * g) {
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
	CFont *CFont::Get(const std::string & ident) {
		CFont *font = Fonts[ident];
		if (!font) {
			DebugPrint("font not found: %s\n" _C_ ident.c_str());
		}
		return font;
	}

	/**
	**  CFontColor constructor
	*/
	CFontColor::CFontColor(const std::string & ident) {
		Ident = ident;
		memset(Colors, 0, sizeof(Colors));
	}

	/**
	**  CFontColor destructor
	*/
	CFontColor::~CFontColor() {
	}

	/**
	**  Create a new font color
	**
	**  @param ident  Font color identifier
	**
	**  @return       New font color
	*/
	CFontColor *CFontColor::New(const std::string & ident) {
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
	CFontColor *CFontColor::Get(const std::string & ident) {
		CFontColor *fc = FontColors[ident];
		if (!fc) {
			DebugPrint("font color not found: %s\n" _C_ ident.c_str());
		}
		return fc;
	}

	void CFont::Clean() {
		CFont *font = this;

		if (UseOpenGL) {
			if (!FontColorGraphics[font].empty()) {
				for (int j = 0; j < (int)AllFontColors.size(); ++j) {
					CGraphic *g = FontColorGraphics[font][AllFontColors[j]];
					glDeleteTextures(g->NumTextures, g->Textures);
					delete[] g->Textures;
					delete g;
				}
				FontColorGraphics[font].clear();
			}
		}
	}


	/**
	**  Clean up the font module.
	*/
	void CleanFonts() {
		for (unsigned int i = 0; i < AllFonts.size(); ++i) {
			CFont *font = AllFonts[i];

			font->Clean();
			delete font;
		}
		if (UseOpenGL) {
			FontColorGraphics.clear();
		}
		AllFonts.clear();
		Fonts.clear();

		for (unsigned int i = 0; i < AllFontColors.size(); ++i) {
			delete AllFontColors[i];
		}
		AllFontColors.clear();
		FontColors.clear();

		SmallFont = NULL;
		GameFont = NULL;
	}

	//@}
