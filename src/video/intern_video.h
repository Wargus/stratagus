//       _________ __                 __
//      /   _____//  |_____________ _/  |______     ____  __ __  ______
//      \_____  \\   __\_  __ \__  \\   __\__  \   / ___\|  |  \/  ___/
//      /        \|  |  |  | \// __ \|  |  / __ \_/ /_/  >  |  /\___ |
//     /_______  /|__|  |__|  (____  /__| (____  /\___  /|____//____  >
//             \/                  \/          \//_____/            \/
//  ______________________      ______________________
//   T H E   W A R   B E G I N S
//    Stratagus - A free fantasy real time strategy game engine
//
//*@file intern_video.h @brief The video headerfile for video sources only. */
//
// (c) Copyright 1999-2002 by Stephan Rasenbergver.
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

#ifndef __INTERN_VIDEO_H__
#define __INTERN_VIDEO_H__

//@{

/*----------------------------------------------------------------------------
-- Documentation
----------------------------------------------------------------------------*/

/**
** @file intern_video.h
**
** THIS HEADER FILE SHOULD ONLY BE INCLUDED BY SOURCE-FILES IN VIDEO !!!
**
** This delivers an interface to basic video operations in video.c, while
** keeping these detailed operations out of the main include-file video.h
**
** @todo FIXME: Currently some interfaces listed in video.h should be
** moved in here, this includes possible "extern" declarations in
** source-files themselves.
** The doxygen docs looks not good, but now contains all info.
*/

/*----------------------------------------------------------------------------
-- Includes
----------------------------------------------------------------------------*/


/*----------------------------------------------------------------------------
-- Declarations
----------------------------------------------------------------------------*/


/*----------------------------------------------------------------------------
-- Variables
----------------------------------------------------------------------------*/

	// Direct acces to clipping rectangle for macro CLIP_RECTANGLE
extern int ClipX1; /// current clipping top left
extern int ClipY1; /// current clipping top left
extern int ClipX2; /// current clipping bottom right
extern int ClipY2; /// current clipping bottom right

/*----------------------------------------------------------------------------
-- Macros
----------------------------------------------------------------------------*/

/**
** Clip rectangle area to clipping rectangle.
** This means given arguments can be changed to take the clipping
** rectangle into account.
**
** @todo FIXME: not easy to debug, but making it a function needs:
** - pointers to be able to alter given arguments
** - special return value to denote 'outside' clipping region
** (which is now handled by a simple return in caller function)
**
** @param x int X screen pixel position
** @param y int Y screen pixel position
** (return value of X and Y can be made larger)
** @param width unsigned int width to display
** @param height unsigned int height to display
** (return value of width and height can be made smaller)
*/
#define CLIP_RECTANGLE(x, y, width, height) do { \
	int f; \
	if (x < ClipX1) { \
		f = ClipX1 - x; \
		if (width <= f) { \
			return; \
		} \
		width -= f; \
		x = ClipX1; \
	} \
	if ((x + width) > ClipX2 + 1) { \
		if (x > ClipX2) { \
			return; \
		} \
		width = ClipX2 - x + 1; \
	} \
	if (y < ClipY1) { \
		f = ClipY1 - y; \
		if (height <= f) { \
			return; \
		} \
		height -= f; \
		y = ClipY1; \
	} \
	if ((y + height) > ClipY2 + 1) { \
		if (y > ClipY2) { \
			return; \
		} \
		height = ClipY2 - y + 1; \
	} \
} while(0)

/**
** Clip rectangle area (just like CLIP_RECTANGLE), but also return offsets
** (these offsets can be used to skip data when used for sprites and such)
**
** @todo FIXME: not easy to debug, but making it a function needs:
** - pointers to be able to alter given arguments
** - special return value to denote 'outside' clipping region
** (which is now handled by a simple return in caller function)
**
** @param x int X screen position
** @param y int Y screen position
** (return value of X and Y can be made larger)
** @param width int width to display
** @param height int height to display
** (return value of width and height can be made smaller)
**  returns:
** @param ofsx int offset X from start of sprite data
** @param ofsy int offset Y from start of sprite data
** @param endx int offset to skip the remaining data at the
** end of each horizontal line of the sprite.
**
** @note there was no need for 'endy', as it isn't used to draw sprites..
*/
#define CLIP_RECTANGLE_OFS(x, y, width, height, ofsx, ofsy, endx) do { \
	if (y < ClipY1) { \
		ofsy = ClipY1 - y; \
		if (height <= ofsy) { \
			return; \
		} \
		height -= ofsy; \
		y = ClipY1; \
	} else { \
		ofsy = 0; \
	} \
	if ((y + height) > ClipY2 + 1) { \
		if (y > ClipY2) { \
			return; \
		} \
		height = ClipY2 - y + 1; \
	} \
	if (x < ClipX1) { \
		ofsx = ClipX1 - x; \
		if (width <= ofsx) { \
			return; \
		} \
		width -= ofsx; \
		x = ClipX1; \
	} else { \
		ofsx = 0; \
	} \
	if ((x + width) > ClipX2 + 1) { \
		if (x > ClipX2) { \
			return; \
		} \
		endx = (x + width) - (ClipX2 + 1); \
		width = ClipX2 - x + 1; \
	} else { \
		endx = 0; \
	} \
} while(0)


/*----------------------------------------------------------------------------
-- Functions
----------------------------------------------------------------------------*/


//@}

#endif // !__INTERN_VIDEO_H__
