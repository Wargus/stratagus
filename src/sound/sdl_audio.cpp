//       _________ __                 __
//      /   _____//  |_____________ _/  |______     ____  __ __  ______
//      \_____  \\   __\_  __ \__  \\   __\__  \   / ___\|  |  \/  ___/
//      /        \|  |  |  | \// __ \|  |  / __ \_/ /_/  >  |  /\___ |
//     /_______  /|__|  |__|  (____  /__| (____  /\___  /|____//____  >
//             \/                  \/          \//_____/            \/
//  ______________________                           ______________________
//			  T H E   W A R   B E G I N S
//	   Stratagus - A free fantasy real time strategy game engine
//
/**@name sdl_audio.c		-	SDL hardware support */
//
//	(c) Copyright 2002-2003 by Lutz Sammer and Fabrice Rossi
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
//	$Id$

//@{

/*----------------------------------------------------------------------------
--		Includes
----------------------------------------------------------------------------*/

#include <stdio.h>
#include "stratagus.h"

#ifdef WITH_SOUND		// {
#ifdef USE_SDLA

#include "SDL.h"
#include "sound_server.h"

/*----------------------------------------------------------------------------
--		Functions
----------------------------------------------------------------------------*/

	/// FIXME: move function to here.
extern void FillAudio(void* udata __attribute__((unused)), Uint8* stream,
	int len);

/**
**		Initialize sound card hardware part with SDL.
**
**		@param dev		Device name ("/dev/dsp").
**		@param freq		Sample frequenz (44100,22050,11025 hz).
**		@param size		Sampe size (8bit, 16bit)
**		@param wait		Flag, if true wait for sound device to open.
**
**		@return				True if failure, false if everything ok.
*/
global int InitSdlSound(const char* dev __attribute__((unused)), int freq,
	int size, int wait __attribute__((unused)))
{
	SDL_AudioSpec wanted;

	wanted.freq = freq;
	if (size == 8) {
		wanted.format = AUDIO_U8;
	} else if (size == 16) {
		wanted.format = AUDIO_S16SYS;
	} else {
		DebugLevel0Fn("Unexpected sample size %d\n" _C_ size);
		wanted.format = AUDIO_S16SYS;
	}
	wanted.channels = 2;
	wanted.samples = 2048;
	wanted.callback = FillAudio;
	wanted.userdata = NULL;

	//  Open the audio device, forcing the desired format
	if (SDL_OpenAudio(&wanted, NULL) < 0) {
		fprintf(stderr, "Couldn't open audio: %s\n", SDL_GetError());
		return -1;
	}
	SoundFildes = 0;
	SDL_PauseAudio(0);

	return 0;
}

#endif
#endif		// } WITH_SOUND

//@}
