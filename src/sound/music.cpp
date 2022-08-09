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

#include <atomic>

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

/// flag is set when a MusicFinishedCallback was enqueued in the event loop and should be handled, and unset when the handler has run.
static std::atomic_flag MusicFinishedEventQueued = ATOMIC_FLAG_INIT;
static volatile bool IsCallbackEnabled = false;

/*----------------------------------------------------------------------------
-- Functions
----------------------------------------------------------------------------*/

/**
**  Check if music is finished and play the next song
*/
static void CheckMusicFinished()
{
	if (SoundEnabled() && IsMusicEnabled()) {
		lua_getglobal(Lua, "MusicStopped");
		if (!lua_isfunction(Lua, -1)) {
			fprintf(stderr, "No MusicStopped function in Lua\n");
		} else {
			DebugPrint("Calling MusicStopped callback at %ul\n" _C_ SDL_GetTicks());
			LuaCall(0, 1);
		}
	}
	// clear the flag after handling the event, so the next event can be enqueued
	MusicFinishedEventQueued.clear();
}

/**
**  Callback for when music has finished
**  Note: we are in the sdl audio thread, so dispatch an event to the main event loop
*/
static void MusicFinishedCallback()
{
	if (!IsCallbackEnabled) {
		return;
	}
	if (MusicFinishedEventQueued.test_and_set()) {
		// don't queue more than one of these events at a time
		return;
	}
	SDL_Event event;
	SDL_zero(event);
	event.type = SDL_SOUND_FINISHED;
	event.user.code = 1;
	event.user.data1 = (void*) CheckMusicFinished;
	if (SDL_PeepEvents(&event, 1, SDL_ADDEVENT, SDL_FIRSTEVENT, SDL_LASTEVENT) <= 0) {
		// if we failed to enqueue the event, clear the flag again
		MusicFinishedEventQueued.clear();
	}
}

/**
**  Init music
*/
void InitMusic()
{
	SetMusicFinishedCallback(MusicFinishedCallback);
	CallbackMusicEnable();
#ifdef USE_FLUIDSYNTH
	InitFluidSynth();
#endif
}

void CallbackMusicEnable() {
	IsCallbackEnabled = true;
}

void CallbackMusicDisable() {
	IsCallbackEnabled = false;
}

void CallbackMusicTrigger() {
	MusicFinishedCallback();
}

//@}
