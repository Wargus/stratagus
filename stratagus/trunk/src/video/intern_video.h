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
/**@name intern_video.h	- The video headerfile for video sources only. */
//
//	(c) Copyright 1999-2001 by Lutz Sammer
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

#ifndef __INTERN_VIDEO_H__
#define __INTERN_VIDEO_H__

//@{


/*----------------------------------------------------------------------------
--	Documentation
----------------------------------------------------------------------------*/
/**
**  THIS HEADER FILE SHOULD ONLY BE INCLUDED BY SOURCE-FILES IN VIDEO !!!
**
**  This delivers an interface to basic video operations in video.c, while 
**  keeping these detailed operations out of the main include-file video.h
**
**  FIXME: Currently some interfaces listed in video.h should be moved in
**         here, this includes possible "extern" declarations in source-files
**         themselves.
**
*/

/*----------------------------------------------------------------------------
--	Includes
----------------------------------------------------------------------------*/


/*----------------------------------------------------------------------------
--	Declarations
----------------------------------------------------------------------------*/


/*----------------------------------------------------------------------------
--	Variables
----------------------------------------------------------------------------*/
    /// Direct acces to clipping rectangle for macro CLIP_RECTANGLE
extern int ClipX1;                      /// current clipping top left
extern int ClipY1;                      /// current clipping top left
extern int ClipX2;                      /// current clipping bottom right
extern int ClipY2;                      /// current clipping bottom right


/*----------------------------------------------------------------------------
--	Macros
----------------------------------------------------------------------------*/
/**
**      Clip to clipping rectangle.
**      FIXME: not easy to debug, but making it a function needs:
**             - pointers to be able to alter given arguments
**             - special return value to denote 'outside' clipping region
**               (which is now handled by a simple return in caller function)
**
**      @param w        unsigned int width to display
**      @param h        unsigned int height to display
**      @param x        int X screen position
**      @param y        int Y screen position
*/
#define CLIP_RECTANGLE(x,y,width,height) { \
  unsigned int f;                          \
  if( x<ClipX1 ) {                         \
    f=ClipX1-x;                            \
    if( width<=f ) {                       \
       return;                             \
    }                                      \
    width-=f;                              \
    x=ClipX1;                              \
  }                                        \
  if( (x+width)>ClipX2+1 ) {               \
    if( x>ClipX2 ) {                       \
      return;                              \
    }                                      \
    width=ClipX2-x+1;                      \
  }                                        \
  if( y<ClipY1 ) {                         \
    f=ClipY1-y;                            \
    if( height<=f ) {                      \
      return;                              \
    }                                      \
    height-=f;                             \
    y=ClipY1;                              \
  }                                        \
  if( (y+height)>ClipY2+1 ) {              \
    if( y>ClipY2 ) {                       \
        return;                            \
    }                                      \
    height=ClipY2-y+1;                     \
  }                                        \
}


/*----------------------------------------------------------------------------
--	Functions
----------------------------------------------------------------------------*/


//@}

#endif	// !__INTERN_VIDEO_H__
