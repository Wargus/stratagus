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
/**@name font.h - The font headerfile. */
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
**  CFont::Width
**
**    Maximal width of a character in pixels.
**
**  CFont::Height
**
**    Height of all characters in pixels.
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
--  Declarations
----------------------------------------------------------------------------*/

class CGraphic;

/*----------------------------------------------------------------------------
--  Definitions
----------------------------------------------------------------------------*/

	/// Color font definition
class CFont {
public:
	CFont() : G(NULL)
	{
		memset(CharWidth, 0, sizeof(CharWidth));
	}

	char CharWidth[208];  /// Real font width (starting with ' ')

// --- FILLED UP ---

	CGraphic *G;  /// Graphic object used to draw
};

/**
**  Font selector for the font functions.
**  @todo should be removed
*/
extern CFont *SmallFont;       /// Small font used in stats
extern CFont *GameFont;        /// Normal font used in game
extern CFont *LargeFont;       /// Large font used in menus
extern CFont *SmallTitleFont;  /// Small font used in episoden titles
extern CFont *LargeTitleFont;  /// Large font used in episoden titles

#define MaxFonts 15  /// Number of fonts supported

/**
**  Color selector for the font functions.
*/
#define FontRed "red"
#define FontGreen "green"
#define FontYellow "yellow"
#define FontWhite "white"
#define FontGrey "grey"

/*----------------------------------------------------------------------------
--  Variables
----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
--  Functions
----------------------------------------------------------------------------*/

	/// Set the default text colors for normal and reverse text
extern void SetDefaultTextColors(char *normal, char *reverse);
	/// Get the default text colors for normal and reverse text
extern void GetDefaultTextColors(char **normalp, char **reversep);
	/// Returns the pixel length of a text
extern int VideoTextLength(CFont *font, const char *text);
	/// Returns the height of the font
extern int VideoTextHeight(CFont *font);
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
	/// Register ccl features
extern void FontsCclRegister(void);
	/// Cleanup the font module
extern void CleanFonts(void);
	/// Check if font is loaded
extern int IsFontLoaded(CFont *font);
	/// Find font by identifier
extern CFont *FontByIdent(const char *ident);
	// Find the name of a font.
extern const char *FontName(CFont *font);

//@}

#endif // !__FONT_H__
