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
/**@name image.h	-	The standard images headerfile. */
//
//	(c) Copyright 1998-2001 by Lutz Sammer
//
//	$Id$

#ifndef __IMAGE_H__
#define __IMAGE_H__

//@{

/*----------------------------------------------------------------------------
--	Images
----------------------------------------------------------------------------*/

/**
**	Constant graphics
**
**	@todo	FIXME: This should also become more flexible.
*/
enum __images__ {
    ImagePanel1,			/// game menue panel
    ImagePanel2,			/// menue panel (unused)
    ImagePanel3,			/// menue panel (unused)
    ImagePanel4,			/// victory menue panel
    ImagePanel5,			/// scenario menue panel
    ImageNone=(-1)			/// use no image or image not found
};

/*----------------------------------------------------------------------------
--	Variables
----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
--	Functions
----------------------------------------------------------------------------*/

    /// Draw image sub area at screen unclipped.
extern void DrawImage(int image,int row,int frame,int x,int y);
    /// Load images for a race.
extern void LoadImages(unsigned int race);

    /// Return image width
extern int ImageWidth(int image);
    /// Return image height
extern int ImageHeight(int image);

//@}

#endif	// !__IMAGE_H__
