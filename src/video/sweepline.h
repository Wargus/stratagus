//   ___________		     _________		      _____  __
//   \_	  _____/______   ____   ____ \_   ___ \____________ _/ ____\/  |_
//    |    __) \_  __ \_/ __ \_/ __ \/    \  \/\_  __ \__  \\   __\\   __|
//    |     \   |  | \/\  ___/\  ___/\     \____|  | \// __ \|  |   |  |
//    \___  /   |__|    \___  >\___  >\______  /|__|  (____  /__|   |__|
//	  \/		    \/	   \/	     \/		   \/
//  ______________________                           ______________________
//			  T H E   W A R   B E G I N S
//	   Stratagus - A free fantasy real time strategy game engine
//
/**@name sweepline.h 	-	Invalidate rectangles from given marked areas */
//
//	(c) Copyright 2002 by Lutz Sammer and Stephan Rasenberg
//
//	Stratagus is free software; you can redistribute it and/or modify
//	it under the terms of the GNU General Public License as published
//	by the Free Software Foundation; only version 2 of the License.
//
//	Stratagus is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//	GNU General Public License for more details.
//
//	$Id$

#ifndef __SWEEPLINE_H__
#define __SWEEPLINE_H__

//@{

/*----------------------------------------------------------------------------
--      Declarations
----------------------------------------------------------------------------*/

/**
** 
**
**/

/*----------------------------------------------------------------------------
--      Functions
----------------------------------------------------------------------------*/

/**
**      Given horizontal segment in pixel x-coordinates leftx..rightx and
**      seen at given pixel y-coordinate will be stored (merged or not) with
**      the existing segments, so they can later be extracted as rectangles.
**      Merging is done when segments are in range (leftx-SWEEPLINE_MERGE) upto
**      (rightx+SWEEPLINE_MERGE).
**      It also remembers given y and denotes the resulting segment to be
**      invalidate at (y+SWEEPLINE_MERGE), unless another segment added later
**      is to be merged with it. This way we can get rectangles covering all
**      added horizontal segments.
**
**      @note: For this to work all segments should be added with an increasing
**      or equal y-coordinate, to make the merge possible and ensure the
**      invalidate order.
**/
extern void SweeplineAdd( int leftx, int rightx, int y );

/**
**      Invalidate all segments which exist too long (have bottomyshadow
**      greater or equal to given y-position as rectangles, removing them from
**      the existing structure.
**      @note: This leaves segments which might still be 'merged' with new ones
**      or are the last and need to be invalidated separetely with
**      SweeplineInvalidateAll
**/
extern void SweeplineInvalidate( int y );

/**
**      Invalidate all segments still available in this structure.
**/
extern void SweeplineInvalidateAll( void );



//@}

#endif	// !__SWEEPLINE_H__
