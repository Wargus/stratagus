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
/**@name include/font.h - The font headerfile. */
//
//      (c) Copyright 1998-2005 by Lutz Sammer and Jimmy Salmon
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

#ifndef __FONT_H__
#define __FONT_H__

//@{

/*----------------------------------------------------------------------------
--  Documentation
----------------------------------------------------------------------------*/

/**
**  @class CFont font.h
**
**  \#include "font.h"
**
**  Defines the fonts used in the Stratagus engine. We support
**  proportional multicolor fonts of 9 colors.
**  (Currently the fonts aren't packed)
**
**  CFont::CharWidth[]
**
**    The width of each font glyph in pixels. The index 0 is the
**    width of the SPACE (' ', 0x20).
**
**  CFont::G
**
**    Contains the graphics of the font, Only 9 colors are supported.
*/

/*----------------------------------------------------------------------------
--  Includes
----------------------------------------------------------------------------*/

#include <string>
#include "SDL.h"
#include "guichan/font.h"

/*----------------------------------------------------------------------------
--  Declarations
----------------------------------------------------------------------------*/
class CGraphic;
class CFontColor;

/// Font definition
class CFont : public gcn::Font
{
private:
	CFont(const std::string &ident) :
		Ident(ident),
		CharWidth(NULL),
		G(NULL)
	{}

public:
	virtual ~CFont();

	static CFont *New(const std::string &ident, CGraphic *g);
	static CFont *Get(const std::string &ident);

	int Height() const;
	int Width(const std::string &text) const;
	int Width(const int number) const;
	bool IsLoaded() const;

	virtual int getHeight() const { return Height(); }
	virtual int getWidth(const std::string &text) const { return Width(text); }
	virtual void drawString(gcn::Graphics *graphics, const std::string &text,
							int x, int y);


	void Load();
	void Reload() const;
	void FreeOpenGL();
	void Clean();

	CGraphic *GetFontColorGraphic(const CFontColor *fontColor) const;

	template<bool CLIP>
	unsigned int DrawChar(CGraphic *g, int utf8, int x, int y, const CFontColor *fc) const;


	void DynamicLoad() const;

private:
	void MakeFontColorTextures() const;
	void MeasureWidths();

private:
	std::string Ident;    /// Ident of the font.
	char *CharWidth;      /// Real font width (starting with ' ')
	CGraphic *G;          /// Graphic object used to draw
};

#define MaxFontColors 9

/// Font color definition
class CFontColor
{
public:
	CFontColor(const std::string &ident);
	~CFontColor();

	static CFontColor *New(const std::string &ident);
	static CFontColor *Get(const std::string &ident);

	std::string Ident;
	SDL_Color Colors[MaxFontColors];
};

/*----------------------------------------------------------------------------
--  Definitions
----------------------------------------------------------------------------*/

#define MaxFonts 15  /// Number of fonts supported

/**
**  FIXME: should be moved to lua
*/
#define FontRed "red"
#define FontGreen "green"
#define FontYellow "yellow"
#define FontWhite "white"
#define FontGrey "grey"

/*----------------------------------------------------------------------------
--  Variables
----------------------------------------------------------------------------*/

/**
**  Font selector for the font functions.
**  FIXME: should be moved to lua
*/
extern CFont *GetSmallFont();       /// Small font used in stats
extern CFont *GetGameFont();        /// Normal font used in game

/*----------------------------------------------------------------------------
--  Functions
----------------------------------------------------------------------------*/

/// Set the default text colors for normal and reverse text
extern void SetDefaultTextColors(const std::string &normal, const std::string &reverse);
/// Get the default text colors for normal and reverse text
extern void GetDefaultTextColors(std::string &normalp, std::string &reversep);
///  Return the 'line' line of the string 's'.
extern std::string GetLineFont(unsigned int line, const std::string &s, unsigned int maxlen, CFont *font);

/// Get the hot key from a string
extern int GetHotKey(const std::string &text);

/// Load and initialize the fonts
extern void LoadFonts();
/// Free OpenGL fonts
extern void FreeOpenGLFonts();
/// Reload OpenGL fonts
extern void ReloadFonts();
/// Cleanup the font module
extern void CleanFonts();

class CLabel
{
	const CFontColor *normal;
	const CFontColor *reverse;
	const CFont *font;

	template <const bool CLIP>
	int DoDrawText(int x, int y, const char *const text,
				   const size_t len, const CFontColor *fc) const;

public:
	CLabel(const CFont *f, const std::string &nc, const std::string &rc): font(f) {
		normal = CFontColor::Get(nc);
		reverse = CFontColor::Get(rc);
	}
	CLabel(const CFont *f);

	int Height() const { return font->Height(); }

	void SetFont(const CFont *f) { font = f; }

	void SetNormalColor(const std::string &nc) { normal = CFontColor::Get(nc); }

	/// Draw text/number unclipped
	int Draw(int x, int y, const char *const text) const;
	int Draw(int x, int y, const std::string &text) const;
	int Draw(int x, int y, int number) const;
	/// Draw text/number clipped
	int DrawClip(int x, int y, const char *const text) const;
	int DrawClip(int x, int y, const std::string &text) const;
	int DrawClip(int x, int y, int number) const;
	/// Draw reverse text/number unclipped
	int DrawReverse(int x, int y, const char *const text) const;
	int DrawReverse(int x, int y, const std::string &text) const;
	int DrawReverse(int x, int y, int number)const ;
	/// Draw reverse text/number clipped
	int DrawReverseClip(int x, int y, const char *const text) const;
	int DrawReverseClip(int x, int y, const std::string &text) const;
	int DrawReverseClip(int x, int y, int number) const;

	int DrawCentered(int x, int y, const std::string &text) const;

};

//@}

#endif // !__FONT_H__
