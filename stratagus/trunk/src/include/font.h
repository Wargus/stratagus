//   ___________		     _________		      _____  __
//   \_	  _____/______   ____   ____ \_   ___ \____________ _/ ____\/  |_
//    |    __) \_  __ \_/ __ \_/ __ \/    \  \/\_  __ \__  \\   __\\   __\ 
//    |     \   |  | \/\  ___/\  ___/\     \____|  | \// __ \|  |   |  |
//    \___  /   |__|    \___  >\___  >\______  /|__|  (____  /__|   |__|
//	  \/		    \/	   \/	     \/		   \/
//  ______________________                           ______________________
//			  T H E   W A R   B E G I N S
//	   FreeCraft - A free fantasy real time strategy game engine
//
/**@name font.h		-	The font headerfile. */
//
//	(c) Copyright 1998-2001 by Lutz Sammer
//
//	FreeCraft is free software; you can redistribute it and/or modify
//	it under the terms of the GNU General Public License as published
//	by the Free Software Foundation; either version 2 of the License,
//	or (at your option) any later version.
//
//	FreeCraft is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//	GNU General Public License for more details.
//
//	$Id$

#ifndef __FONT_H__
#define __FONT_H__

//@{

/*----------------------------------------------------------------------------
--	Documentation
----------------------------------------------------------------------------*/

/**
**	@struct _color_font_ font.h
**
**	\#include "font.h"
**
**	typedef struct _color_font_ ColorFont;
**
**		Defines the fonts used in the FreeCraft engine. We support
**		proportional multicolor fonts of 7 colors. The eighth color is
**		transparent. (Currently the fonts aren't packed)
**
**	ColorFont::File
**
**		File containing the graphics for the font.
**
**	ColorFont::Width
**
**		Maximal width of a character in pixels.
**
**	ColorFont::Height
**
**		Height of all characters in pixels.
**
**	ColorFont::CharWidth[]
**
**		The width of each font glyph in pixels. The index 0 is the
**		width of the SPACE (' ', 0x20).
**
**	ColorFont::Graphic
**
**              Contains the graphics of the font, loaded from ColorFont::File.
*/

/*----------------------------------------------------------------------------
--	Definitions
----------------------------------------------------------------------------*/

    ///	Color font definition
typedef struct _color_font_ {
    char*	File;			/// file containing font data

    int		Width;			/// max width of characters in file
    int		Height;			/// max height of characters in file

    char	CharWidth[208];		/// real font width (starting with ' ')

// --- FILLED UP ---

    Graphic*	Graphic;		/// graphic object used to draw
} ColorFont;

/**
**	Font selector for the font functions.
*/
enum _game_font_ {
    SmallFont,				/// small font used in stats
    GameFont,				/// normal font used in game
    LargeFont,				/// large font used in menus
    SmallTitleFont,			/// small font used in episoden titles
    LargeTitleFont,			/// large font used in episoden titles
    // ... more to come or not
    MaxFonts,				/// number of fonts supported
};

/**
**	Color selector for the font functions.
*/
enum _font_color {
    FontBlack,				/// black font color
    FontRed,				/// red font color
    FontGreen,				/// green font color
    FontYellow,				/// yellow font color
    FontBlue,				/// blue font color
    FontMagenta,			/// magenta font color
    FontCyan,				/// cyan font color
    FontWhite,				/// white font color
    FontBrightBlack,			/// bright black font color
    FontGrey=FontBrightBlack,		/// grey font color
    FontBrightRed,			/// bright red font color
    FontBrightGreen,			/// bright green font color
    FontBrightYellow,			/// bright yellow font color
    FontBrightBlue,			/// bright blue font color
    FontBrightMagenta,			/// bright magenta font color
    FontBrightCyan,			/// bright cyan font color
    FontBrightWhite,			/// bright white font color
};

/*----------------------------------------------------------------------------
--	Variables
----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
--	Functions
----------------------------------------------------------------------------*/

    /// Set the default text colors for normal and reverse text
extern void SetDefaultTextColors(int normal,int reverse);
    /// Get the default text colors for normal and reverse text
extern void GetDefaultTextColors(int *normalp,int *reversep);
    /// Returns the pixel length of a text
extern int TextLength(unsigned font,const unsigned char* text);
    /// Draw text unclipped
extern int DrawText(int x,int y,unsigned font,const unsigned char* text);
    /// Draw reverse text unclipped
extern int DrawReverseText(int x,int y,unsigned font,const unsigned char* text);
    /// Draw text centered and unclipped
extern int DrawTextCentered(int x,int y,unsigned font,const unsigned char* text);
    /// Draw number unclipped
extern int DrawNumber(int x,int y,unsigned font,int number);
    /// Draw reverse number unclipped
extern int DrawReverseNumber(int x,int y,unsigned font,int number);

    /// Load and initialize the fonts
extern void LoadFonts(void);
    /// Register ccl features
extern void FontsCclRegister(void);

//@}

#endif	// !__FONT_H__
