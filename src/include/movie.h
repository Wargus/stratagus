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
/**@name movie.h	-	The movie header file. */
//
//	(c) Copyright 2002 by Lutz Sammer
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

#ifndef __MOVIE_H__
#define __MOVIE_H__

//@{

/*----------------------------------------------------------------------------
--	Documentation
----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
--	Includes
----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
--	Declarations
----------------------------------------------------------------------------*/

/**
**	Play movie flags.
*/
enum _play_movie_flags_ {
    PlayMovieFullScreen = 1,		/// Switch to full screen
    PlayMovieZoomScreen = 2,		/// Zoom to screen size
    PlayMovieKeepAspect = 4,		/// Keep the aspect while zooming
};

/**
**	Movie handle.
*/
typedef struct _movie_ {
    void*	File;			/// Demux handler
} Movie;

/*----------------------------------------------------------------------------
--	Variables
----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
--	Functions
----------------------------------------------------------------------------*/

    /// Play a movie file
extern int PlayMovie(const char* name, int flags);

    /// Initialize the video module
extern void InitMovie(void);
    /// Cleanup the video module
extern void CleanMovie(void);

//@}

#endif	// !__MOVIE_H__
