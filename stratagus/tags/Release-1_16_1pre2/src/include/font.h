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
/*
**	(c) Copyright 1998-2000 by Lutz Sammer
**
**	$Id$
*/

#ifndef __FONT_H__
#define __FONT_H__

//@{

/*----------------------------------------------------------------------------
--	Definitions
----------------------------------------------------------------------------*/

/**
**	Color font definition
*/
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
    GameFont,				/// Font used in game
    SmallFont,				/// Small font used in stats
    LargeFont,				/// Large font used in menus

    // ... more to come
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

// extern ColorFont Fonts[];		

/*----------------------------------------------------------------------------
--	Functions
----------------------------------------------------------------------------*/

    /// Set the default text colors for normal and reverse text.
extern void SetDefaultTextColors(int normal,int reverse);
    /// Returns the pixel length of a text.
extern int TextLength(unsigned font,const unsigned char* text);
    /// Draw text unclipped.
extern int DrawText(int x,int y,unsigned font,const unsigned char* text);
    /// Draw reverse text unclipped.
extern int DrawReverseText(int x,int y,unsigned font,const unsigned char* text);
    /// Draw text centered and unclipped.
extern int DrawTextCentered(int x,int y,unsigned font,const unsigned char* text);
    /// Draw number unclipped.
extern int DrawNumber(int x,int y,unsigned font,int number);
    /// Draw reverse number unclipped.
extern int DrawReverseNumber(int x,int y,unsigned font,int number);

    /// load and initialize the fonts
extern void LoadFonts(void);
    /// register ccl features
extern void FontsCclRegister(void);

//@}

#endif	// !__FONT_H__
