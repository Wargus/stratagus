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
/*
**	(c) Copyright 1998-2000 by Lutz Sammer
**
**	$Id$
*/

#ifndef __IMAGE_H__
#define __IMAGE_H__

//@{

/*----------------------------------------------------------------------------
--	Images
----------------------------------------------------------------------------*/

/*
**	Constant graphics
**	FIXME: This should also become more flexible.
*/
#define ImageNone		(-1)

enum __images__ {
    ImagePanel1,
    ImagePanel2,
    ImagePanel3,
    ImagePanel4,
    ImagePanel5,
};

/*----------------------------------------------------------------------------
--	Variables
----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
--	Functions
----------------------------------------------------------------------------*/

extern void DrawImage(int image,int row,int frame,int x,int y);
extern void LoadImages(unsigned int race);

    /// Image width
extern int ImageWidth(int image);
    /// Image height
extern int ImageHeight(int image);

//@}

#endif	// !__IMAGE_H__
