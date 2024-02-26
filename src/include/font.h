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

#include "color.h"

#include <array>
#include <guichan/font.hpp>
#include <string>
#include <SDL.h>

/*----------------------------------------------------------------------------
--  Declarations
----------------------------------------------------------------------------*/
class CGraphic;
class CFontColor;

/// Font definition
class CFont : public gcn::Font
{
private:
	explicit CFont(std::string ident) : Ident(std::move(ident)) {}

public:
	~CFont() override;

	static CFont *New(const std::string &ident, CGraphic *g);
	static CFont *Get(std::string_view ident);

	int Height() const;
	int Width(const std::string &text) const;
	int Width(const int number) const;
	bool IsLoaded() const;

	int getHeight() const override { return Height(); }
	int getWidth(const std::string &text) const override { return Width(text); }
	void drawString(gcn::Graphics *graphics, const std::string &text, int x, int y) override;
	int getStringIndexAt(const std::string &text, int x) const override;

	// Set to false to reverse color when drawing string
	void setIsNormal(bool value) { is_normal = value; }

	void Load();
	void Reload() const;

	CGraphic *GetFontColorGraphic(const CFontColor &fontColor) const;

	template<bool CLIP>
	unsigned int DrawChar(CGraphic &g, int utf8, int x, int y, const CFontColor &fc) const;

	void DynamicLoad() const;

private:
	void MeasureWidths();

private:
	std::string Ident;    /// Ident of the font.
	std::vector<char> CharWidth; /// Real font width (starting with ' ')
	CGraphic *G = nullptr;       /// Graphic object used to draw
	bool is_normal = true;
};

#define MaxFontColors 9

/// Font color definition
class CFontColor
{
public:
	explicit CFontColor(std::string ident);

	static CFontColor *New(const std::string &ident);
	static CFontColor *Get(std::string_view ident);

	std::string Ident;
	std::array<SDL_Color, MaxFontColors> Colors{};
};

/*----------------------------------------------------------------------------
--  Definitions
----------------------------------------------------------------------------*/

/**
**  FIXME: should be moved to lua
*/
#define FontRed "red"
#define FontGreen "green"
#define FontYellow "yellow"
#define FontWhite "white"
#define FontGrey "grey"

/*----------------------------------------------------------------------------
--  Functions
----------------------------------------------------------------------------*/

/**
**  Font selector for the font functions.
**  FIXME: should be moved to lua
*/
extern CFont &GetSmallFont();  /// Small font used in stats
extern CFont &GetGameFont();   /// Normal font used in game
extern bool IsGameFontReady(); /// true when GameFont is provided

extern int FontCodePage;

/// Set the default text colors for normal and reverse text
extern void SetDefaultTextColors(const std::string &normal, const std::string &reverse);
/// Get the default text colors for normal and reverse text
extern void GetDefaultTextColors(std::string &normalp, std::string &reversep);
///  Return the 'line' line of the string 's'.
extern std::string GetLineFont(unsigned int line, const std::string &s, unsigned int maxlen, const CFont *font);

/// Get the hot key from a string
extern int GetHotKey(const std::string &text);

/// Load and initialize the fonts
extern void LoadFonts();

/// Cleanup the font module
extern void CleanFonts();

class CLabel
{
public:
	CLabel(const CFont &f, std::string_view nc, std::string_view rc): font(&f)
	{
		normal = CFontColor::Get(nc);
		reverse = CFontColor::Get(rc);
	}
	explicit CLabel(const CFont &f);

	int Height() const { return font->Height(); }

	void SetFont(const CFont &f) { font = &f; }

	void SetNormalColor(std::string_view nc) { normal = CFontColor::Get(nc); }

	/// Draw text/number unclipped
	int Draw(int x, int y, std::string_view text) const;
	int Draw(int x, int y, int number) const;
	/// Draw text/number clipped
	int DrawClip(int x, int y, std::string_view text, bool is_normal = true) const;
	int DrawClip(int x, int y, int number) const;
	/// Draw reverse text/number unclipped
	int DrawReverse(int x, int y, std::string_view text) const;
	int DrawReverse(int x, int y, int number) const ;
	/// Draw reverse text/number clipped
	int DrawReverseClip(int x, int y, std::string_view text) const;
	int DrawReverseClip(int x, int y, int number) const;

	int DrawCentered(int x, int y, const std::string &text) const;
	int DrawReverseCentered(int x, int y, const std::string &text) const;
private:
	template <const bool CLIP>
	int DoDrawText(int x, int y, std::string_view text, const CFontColor *fc) const;
private:
	const CFontColor *normal;
	const CFontColor *reverse;
	const CFont *font;
};

//@}

#endif // !__FONT_H__
