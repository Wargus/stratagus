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
//      (c) Copyright 1998-2008 by Lutz Sammer, Jimmy Salmon, Nehal Mistry
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
#include <memory>

#include "video.h"
#include "font.h"
#include "script.h"

#include "intern_video.h"

/*----------------------------------------------------------------------------
--  Types
----------------------------------------------------------------------------*/

namespace {
	/**
	** A smart pointer that maintains CGraphic::Refs.
	**
	** There is no CGraphicPtr(CGraphic *) constructor, because
	** that would too easily lead to bugs.  Instead, you can set
	** the G member directly.
	**
	** @todo Publish CGraphicPtr to Lua, instead of CGraphic itself.
	** That should make it easier to ensure that CGraphic::Refs
	** remains positive as long as references exist in Lua.
	*/
	struct CGraphicPtr {
		CGraphic *G;

		CGraphicPtr(): G(NULL)
		{
		}

		~CGraphicPtr()
		{
			CGraphic::Free(this->G);
		}

		CGraphicPtr(const CGraphicPtr &other): G(other.G)
		{
			if (this->G) {
				this->G->Refs++;
			}
		}

		CGraphicPtr &operator =(const CGraphicPtr &other)
		{
			if (other.G != this->G) {
				CGraphic::Free(this->G);
				this->G = other.G;
				if (this->G) {
					this->G->Refs++;
				}
			}
			return *this;
		}

		operator CGraphic *() const
		{
			return this->G;
		}

		CGraphic *operator ->() const
		{
			return this->G;
		}

		CGraphic *Detach()
		{
			CGraphic *ret = this->G;
			this->G = NULL;
			return ret;
		}
	};

	/**
	 ** Swaps the pointers of two CGraphicPtr objects.
	 */
	void swap(CGraphicPtr &a, CGraphicPtr &b)
	{
		std::swap(a.G, b.G);
	}
}

/**
** Two fonts that use the same textures: a markup font that recognizes
** markup in strings drawn with it, and a plain-text font that
** doesn't.
*/
class CFontFamily {
	friend class CFont;

public:
	explicit CFontFamily(const std::string &ident);
	~CFontFamily();

	void MeasureWidths();
	void MakeFontColorGraphics();
	CGraphic *FontColorGraphic(CFontColor *);
	void FreeFontColorGraphics();

	/**
	** A font that does not recognize any markup in strings.
	**
	** This should be used in text-input fields and text boxes
	** because GUIChan lets the user type directly into those
	** and does not know how to escape markup.
	*/
	CFont PlainTextFont;

	/**
	** A font that recognizes markup in strings.
	*/
	CFont MarkupFont;

	/// Real font width (starting with ' ')
	char *CharWidth;

	/// Graphic object used to draw
	CGraphicPtr G;

private:
	/// Ident of the font.
	const std::string Ident;

	/**
	** Textures for each CFontColor.  Used for OpenGL only.
	*/
	typedef std::map<CFontColor *, CGraphic *> FontColorGraphicsType;
	FontColorGraphicsType FontColorGraphics;

private: // undefined
	CFontFamily(const CFontFamily &);
	CFontFamily &operator =(const CFontFamily &);
};

/*----------------------------------------------------------------------------
--  Variables
----------------------------------------------------------------------------*/

typedef std::map<std::string, CFontFamily *> FontFamiliesType;
static FontFamiliesType FontFamilies;      /// Map of ident to font family.

static std::vector<CFontColor *> AllFontColors; /// Vector of all font colors.
static std::map<std::string, CFontColor *> FontColors; /// Map of ident to font color.

static CFontColor *FontColor;                   /// Current font color

static CFontColor *LastTextColor;          /// Last text color
static CFontColor *DefaultTextColor;       /// Default text color
static CFontColor *ReverseTextColor;       /// Reverse text color
static std::string DefaultNormalColorIndex;     /// Default normal color index
static std::string DefaultReverseColorIndex;    /// Default reverse color index

// FIXME: remove these
CFont *SmallFont;       /// Small font used in stats
CFont *GameFont;        /// Normal font used in game
CFont *LargeFont;       /// Large font used in menus


/*----------------------------------------------------------------------------
--  Guichan Functions
----------------------------------------------------------------------------*/

int CFont::Height() const
{
	return this->Family->G->Height;
}

bool CFont::IsLoaded() const
{
	return this->Family->G && this->Family->G->IsLoaded();
}

void CFont::drawString(gcn::Graphics *graphics, const std::string &txt,
	int x, int y)
{
	const gcn::ClipRectangle &r = graphics->getCurrentClipArea();
	int right = std::min(r.x + r.width - 1, Video.Width - 1);
	int bottom = std::min(r.y + r.height - 1, Video.Height - 1);

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
** CFontFamily constructor.
**
** This does not set CFontFamily::G.  That must be done separately.
*/
CFontFamily::CFontFamily(const std::string &ident)
	: Ident(ident)
{
	this->PlainTextFont.Family = this;
	this->MarkupFont.Family = this;
	this->CharWidth = NULL;
}

/**
** CFontFamily destructor.
*/
CFontFamily::~CFontFamily()
{
	this->FreeFontColorGraphics();
	delete[] this->CharWidth;
}

/**
** CFont constructor.
**
** All the hard work is done in CFont::New instead.
*/
CFont::CFont(): Family(NULL)
{
}

/**
** CFont destructor.
**
** Because CFont nowadays has just a CFontFamily pointer
** and exists only as part of the CFontFamily,
** there is nothing to do here.
*/
CFont::~CFont()
{
}

/**
** Gets the plain-text font in the same font family.
**
** The plain-text font does not recognize markup in strings
** and is therefore suitable for widgets to which the user
** can directly type text.
**
** @return The plain-text font in the same font family
** as this font.
*/
CFont *CFont::PlainText()
{
	return &this->Family->PlainTextFont;
}

/**
** Checks whether this font is a plain-text font.
**
** @return true if this font is a plain-text font;
** false if this font recognizes markup in strings.
*/
bool CFont::IsPlainText() const
{
	return this == &this->Family->PlainTextFont;
}

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
	int gx, int gy, int w, int h, int x, int y)
{
	if (!UseOpenGL) {
		SDL_Rect srect = {
			static_cast<Sint16>(gx),
			static_cast<Sint16>(gy),
			static_cast<Uint16>(w),
			static_cast<Uint16>(h)
		};
		SDL_Rect drect = {
			static_cast<Sint16>(x),
			static_cast<Sint16>(y),
			0, // SDL_BlitSurface ignores the width and height.
			0
		};

		SDL_SetColors(g->Surface, FontColor->Colors, 0, MaxFontColors);
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

	bool isPlainText = this->IsPlainText();

	while (GetUTF8(text, pos, utf8)) {
		if (utf8 == '~' && !isPlainText) {
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
			width += this->Family->CharWidth[utf8 - 32] + 1;
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
	ex = ex; // make the compiler to shut up.
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
**  @param clip  Flag to clip the text.
**
**  @return      The length of the printed text.
*/
static int DoDrawText(int x, int y, CFontFamily *fontFamily,
	bool isPlainText, const std::string &text, bool clip)
{
	int w;
	int width;
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

	if (!UseOpenGL) {
		g = fontFamily->G;
	} else {
		g = fontFamily->FontColorGraphic(FontColor);
	}

	rev = NULL;
	width = 0;
	pos = 0;
	while (GetUTF8(text, pos, utf8)) {
		if (utf8 == '~' && !isPlainText) {
			switch (text[pos]) {
				case '\0':  // wrong formatted string.
					DebugPrint("oops, format your ~\n");
					return width;
				case '~':
					++pos;
					break;
				case '!':
					rev = FontColor;
					FontColor = ReverseTextColor;
					if (UseOpenGL) {
						g = fontFamily->FontColorGraphic(FontColor);
					}
					++pos;
					continue;
				case '<':
					LastTextColor = FontColor;
					FontColor = ReverseTextColor;
					if (UseOpenGL) {
						g = fontFamily->FontColorGraphic(FontColor);
					}
					++pos;
					continue;
				case '>':
					rev = LastTextColor;  // swap last and current color
					LastTextColor = FontColor;
					FontColor = rev;
					if (UseOpenGL) {
						g = fontFamily->FontColorGraphic(FontColor);
					}
					++pos;
					continue;

				default:
					p = text.c_str() + pos;
					while (*p && *p !='~') {
						++p;
					}
					if (!*p) {
						DebugPrint("oops, format your ~\n");
						return width;
					}
					color = new char[p - (text.c_str() + pos) + 1];
					memcpy(color, text.c_str() + pos, p - (text.c_str() + pos));
					color[p - (text.c_str() + pos)] = '\0';
					pos = p - text.c_str() + 1;
					LastTextColor = FontColor;
					CFontColor *fc = CFontColor::Get(color);
					if (fc) {
						FontColor = fc;
						if (UseOpenGL) {
							g = fontFamily->FontColorGraphic(fc);
						}
					}
					delete[] color;
					continue;
			}
		}

		c = utf8 - 32;
		Assert(c >= 0);

		ipr = fontFamily->G->GraphicWidth / fontFamily->G->Width;
		if (c >= 0 && c < ipr * fontFamily->G->GraphicHeight / fontFamily->G->Height) {
			w = fontFamily->CharWidth[c];
			DrawChar(g, (c % ipr) * fontFamily->G->Width, (c / ipr) * fontFamily->G->Height,
				w, fontFamily->G->Height, x + width, y);
		} else {
			w = fontFamily->CharWidth[0];
			DrawChar(g, 0, 0, w, fontFamily->G->Height, x + width, y);
		}
		width += w + 1;
		if (rev) {
			FontColor = rev;
			if (UseOpenGL) {
				g = fontFamily->FontColorGraphic(FontColor);
			}
			rev = NULL;
		}
	}

	return width;
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
	return DoDrawText(x, y, font->FontFamily(), font->IsPlainText(),
			  text, false);
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
	return DoDrawText(x, y, font->FontFamily(), font->IsPlainText(),
			  text, true);
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
	int width;

	FontColor = ReverseTextColor;
	width = VideoDrawText(x, y, font, text);
	FontColor = DefaultTextColor;

	return width;
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
	int width;

	FontColor = ReverseTextColor;
	width = VideoDrawTextClip(x, y, font, text);
	FontColor = DefaultTextColor;

	return width;
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
	sprintf_s(bufs, sizeof(bufs), "%d", number);
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
void CFontFamily::MeasureWidths()
{
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
**  Make or retrieve a variant of the graphic, colorized for the
**  specified CFontColor.  This function is used for OpenGL only.
**
**  @param fc Which color you want.  This function saves the address
**  of the CFontColor object and uses that for checking whether a
**  suitably colorized variant already exists.
**
**  @return The colorized graphic.  The caller must not free it.
*/
CGraphic *CFontFamily::FontColorGraphic(CFontColor *fc)
{
	// Only OpenGL needs color-specific textures.
	Assert(UseOpenGL);

	FontColorGraphicsType::iterator found
		= this->FontColorGraphics.find(fc);
	if (found != this->FontColorGraphics.end()) {
		// already loaded
		return found->second;
	}

	// The CGraphic we'll create will not have its own Surface;
	// instead, it shares the Surface of this->G.  To prevent
	// CGraphic::Free from freeing the shared surface, we reset
	// newg->Surface to NULL immediately after MakeTexture.
	// We must do that even if MakeTexture throws std::bad_alloc.
	// Use a destructor rather than try...catch, for better
	// backtraces on uncaught exceptions.
	struct CSurfaceSharingGraphicPtr: CGraphicPtr
	{
		~CSurfaceSharingGraphicPtr()
		{
			if (this->G) {
				this->G->Surface = NULL;
			}
		}
	};

	CSurfaceSharingGraphicPtr newg;
	newg.G = new CGraphic;
	newg->Width = this->G->Width;
	newg->Height = this->G->Height;
	newg->NumFrames = this->G->NumFrames;
	newg->GraphicWidth = this->G->GraphicWidth;
	newg->GraphicHeight = this->G->GraphicHeight;

	SDL_Surface *s = this->G->Surface;
	SDL_LockSurface(s);
	for (int j = 0; j < MaxFontColors; ++j) {
		s->format->palette->colors[j] = fc->Colors[j];
	}
	SDL_UnlockSurface(s);

	newg->Surface = s;
	MakeTexture(newg);
	newg->Surface = NULL;

	this->FontColorGraphics.insert(
		FontColorGraphicsType::value_type(fc, newg));
	return newg.Detach();
}

/**
**  Makes colorized variants of the graphic for all known CFontColor
**  objects.  This function is used for OpenGL only.
*/
void CFontFamily::MakeFontColorGraphics()
{
	// Only OpenGL needs color-specific textures.
	Assert(UseOpenGL);

	for (int i = 0; i < (int)AllFontColors.size(); ++i) {
		CFontColor *fc = AllFontColors[i];
		this->FontColorGraphic(fc);
	}
}

/**
**  Frees all colorized variants of the graphic.  This function is
**  needed for OpenGL only, but is safe to call without OpenGL too.
*/
void CFontFamily::FreeFontColorGraphics()
{
	// Clear the map regardless of whether the CFontColor
	// objects still exist.
	for (FontColorGraphicsType::iterator iter
		     = this->FontColorGraphics.begin();
	     iter != this->FontColorGraphics.end();
	     ++iter) {
		CGraphic::Free(iter->second);
	}
	this->FontColorGraphics.clear();
}

/**
**  Load all fonts.
*/
void LoadFonts()
{
	for (FontFamiliesType::iterator iter = FontFamilies.begin();
	     iter != FontFamilies.end(); ++iter) {
		CFontFamily *fontFamily = iter->second;
		CGraphic *g = fontFamily->G;
		if (g) {
			ShowLoadProgress("Fonts %s", g->File.c_str());
			g->Load();
			fontFamily->MeasureWidths();
			if (UseOpenGL) {
				fontFamily->MakeFontColorGraphics();
			}
		}
	}

	// TODO: remove this
	SmallFont = CFont::Get("small");
	GameFont = CFont::Get("game");
	LargeFont = CFont::Get("large");
}

/**
**  Free OpenGL fonts
*/
void FreeOpenGLFonts()
{
	for (FontFamiliesType::iterator iter = FontFamilies.begin();
	     iter != FontFamilies.end(); ++iter) {
		CFontFamily *fontFamily = iter->second;
		fontFamily->FreeFontColorGraphics();
	}
}

/**
**  Reload OpenGL fonts
*/
void ReloadFonts()
{
	for (FontFamiliesType::iterator iter = FontFamilies.begin();
	     iter != FontFamilies.end(); ++iter) {
		CFontFamily *fontFamily = iter->second;
		fontFamily->FreeFontColorGraphics();
		if (fontFamily->G) {
			fontFamily->MakeFontColorGraphics();
		}
	}
}

/**
**  Create a new font
**
**  @param ident  Font identifier
**  @param g      Graphic (always consumes a reference)
**
**  @return       New font
*/
CFont *CFont::New(const std::string &ident, CGraphic *g)
{
	// Make sure we'll call CGraphic::Free(g) on any exception.
	CGraphicPtr gptr;
	gptr.G = g;

	FontFamiliesType::iterator found = FontFamilies.find(ident);
	if (found != FontFamilies.end()) {
		CFontFamily *fontFamily = found->second;
		if (fontFamily->G != g) {
			fontFamily->FreeFontColorGraphics();
		}
		swap(fontFamily->G, gptr);
		return &fontFamily->MarkupFont;
	} else {
		std::auto_ptr<CFontFamily> fontFamily(new CFontFamily(ident));
		swap(fontFamily->G, gptr);
		FontFamilies.insert(
			FontFamiliesType::value_type(ident, fontFamily.get()));
		return &fontFamily.release()->MarkupFont;
	}
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
	FontFamiliesType::iterator found = FontFamilies.find(ident);
	if (found != FontFamilies.end()) {
		return &found->second->MarkupFont;
	} else {
		DebugPrint("font not found: %s" _C_ ident.c_str());
		return NULL;
	}
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
void CleanFonts()
{
	while (FontFamilies.begin() != FontFamilies.end()) {
		delete FontFamilies.begin()->second;
		FontFamilies.erase(FontFamilies.begin());
	}

	for (int i = 0; i < (int)AllFontColors.size(); ++i) {
		delete AllFontColors[i];
	}
	AllFontColors.clear();
	FontColors.clear();

	SmallFont = NULL;
	GameFont = NULL;
	LargeFont = NULL;
}

//@}
