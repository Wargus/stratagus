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
/**@name music.cpp - Background music support */
//
//      (c) Copyright 2002-2006 by Lutz Sammer, Nehal Mistry, and Jimmy Salmon
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

//@{

/*----------------------------------------------------------------------------
-- Includes
----------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "stratagus.h"


#include "SDL.h"

#include "sound_server.h"
#include "script.h"

/*----------------------------------------------------------------------------
-- Declaration
----------------------------------------------------------------------------*/

#define SoundFrequency 44100 // sample rate of dsp

/*----------------------------------------------------------------------------
-- Variables
----------------------------------------------------------------------------*/

static SDL_mutex *MusicFinishedMutex;     /// Mutex for MusicFinished
static bool MusicFinished;                /// Music ended and we need a new file

bool CallbackMusic;                       /// flag true callback ccl if stops

/*----------------------------------------------------------------------------
-- Functions
----------------------------------------------------------------------------*/

/**
**  Callback for when music has finished
**  Note: we are in the sdl audio thread
*/
static void MusicFinishedCallback()
{
	SDL_LockMutex(MusicFinishedMutex);
	MusicFinished = true;
	SDL_UnlockMutex(MusicFinishedMutex);
}

/**
**  Check if music is finished and play the next song
*/
void CheckMusicFinished(bool force)
{
	bool proceed;

	SDL_LockMutex(MusicFinishedMutex);
	proceed = MusicFinished;
	MusicFinished = false;
	SDL_UnlockMutex(MusicFinishedMutex);

	if ((proceed || force) && SoundEnabled() && IsMusicEnabled() && CallbackMusic) {
		lua_getglobal(Lua, "MusicStopped");
		if (!lua_isfunction(Lua, -1)) {
			fprintf(stderr, "No MusicStopped function in Lua\n");
			StopMusic();
		} else {
			LuaCall(0, 1);
		}
	}
}

/**
**  Init music
*/
void InitMusic()
{
	MusicFinished = false;
	MusicFinishedMutex = SDL_CreateMutex();
	SetMusicFinishedCallback(MusicFinishedCallback);
}

//@}
