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
//      $Id$

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

#include "video.h"

/*----------------------------------------------------------------------------
--  Declarations
----------------------------------------------------------------------------*/

	/// Font definition
class CFont : public gcn::Font {
private:
	CFont(const std::string &ident) : Ident(ident), CharWidth(NULL),
		G(NULL) {}

public:
	virtual ~CFont();

	static CFont *New(const std::string &ident, CGraphic *g);
	static CFont *Get(const std::string &ident);

	inline int Height() const { return G->Height; }
	int Width(const char *text) const;
	inline bool IsLoaded() { return G && G->IsLoaded(); }

	virtual int getHeight() const { return Height(); }
	virtual int getWidth(const std::string &text) const
		{ return Width(text.c_str()); }
	virtual void drawString(gcn::Graphics *graphics, const std::string &text, 
		int x, int y);
	
	void MeasureWidths();

	std::string Ident;    /// Ident of the font.
	char *CharWidth;      /// Real font width (starting with ' ')
	CGraphic *G;          /// Graphic object used to draw
};

#define MaxFontColors 9

	/// Font color definition
class CFontColor {
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
extern CFont *SmallFont;       /// Small font used in stats
extern CFont *GameFont;        /// Normal font used in game
extern CFont *LargeFont;       /// Large font used in menus
extern CFont *SmallTitleFont;  /// Small font used in episoden titles
extern CFont *LargeTitleFont;  /// Large font used in episoden titles

/*----------------------------------------------------------------------------
--  Functions
----------------------------------------------------------------------------*/

	/// Set the default text colors for normal and reverse text
extern void SetDefaultTextColors(const std::string &normal, const std::string &reverse);
	/// Get the default text colors for normal and reverse text
extern void GetDefaultTextColors(std::string &normalp, std::string &reversep);
	///  Return the 'line' line of the string 's'.
extern char *GetLineFont(int line, char *s, int maxlen, CFont *font);
	/// Draw text unclipped
extern int VideoDrawText(int x, int y, CFont *font, const char *text);
	/// Draw text unclipped
extern int VideoDrawTextClip(int x, int y, CFont *font, const char *text);
	/// Draw reverse text unclipped
extern int VideoDrawReverseText(int x, int y, CFont *font, const char *text);
	/// Draw reverse text clipped
extern int VideoDrawReverseTextClip(int x, int y, CFont *font, const char *text);
	/// Draw text centered and unclipped
extern int VideoDrawTextCentered(int x, int y, CFont *font, const char *text);
	/// Draw number unclipped
extern int VideoDrawNumber(int x, int y, CFont *font, int number);
	/// Draw number clipped
extern int VideoDrawNumberClip(int x, int y, CFont *font, int number);
	/// Draw reverse number unclipped
extern int VideoDrawReverseNumber(int x, int y, CFont *font, int number);
	/// Draw reverse number clipped
extern int VideoDrawReverseNumberClip(int x, int y, CFont *font, int number);

	/// Load and initialize the fonts
extern void LoadFonts(void);
#ifdef USE_OPENGL
	/// Reload OpenGL fonts
extern void ReloadFonts(void);
#endif
	/// Cleanup the font module
extern void CleanFonts(void);

//@}

#endif // !__FONT_H__
