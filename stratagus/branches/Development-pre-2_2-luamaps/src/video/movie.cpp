//       _________ __                 __
//      /   _____//  |_____________ _/  |______     ____  __ __  ______
//      \_____  \\   __\_  __ \__  \\   __\__  \   / ___\|  |  \/  ___/
//      /        \|  |  |  | \// __ \|  |  / __ \_/ /_/  >  |  /\___ |
//     /_______  /|__|  |__|  (____  /__| (____  /\___  /|____//____  >
//             \/                  \/          \//_____/            \/
//  ______________________                           ______________________
// T H E   W A R   B E G I N S
// Stratagus - A free fantasy real time strategy game engine
//
/**@name movie.c - Movie playback functions. */
//
// (c) Copyright 2002-2004 by Lutz Sammer
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
// $Id$

//@{

/*----------------------------------------------------------------------------
-- Includes
----------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>

#include "stratagus.h"
#include "video.h"
#include "network.h"
#include "sound.h"
#include "sound_server.h"
#include "avi.h"
#include "movie.h"
#include "iocompat.h"
#include "iolib.h"

#ifdef USE_SDL
#include "SDL.h"
#endif



/**
**  Play a video file.
**
**  @param file   Filename of movie file.
**  @param flags  Flags for movie playback.
**
**  @return       True if file isn't a supported movie.
**
**  @todo Support full screen and resolution changes.
*/
int PlayMovie(const char* file, int flags)
{
	printf("FIXME: PlayMovie(\"%s\",%x) not supported.\n", file, flags);

	if (strcasestr(file, ".avi\0")) {
		return 0;
	}
	return 1;
}

/**
**  Initialize the movie module.
*/
void InitMovie(void)
{
}

/**
**  Cleanup the movie module.
*/
void CleanMovie(void)
{
}

//@}
