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
//      (c) Copyright 2002-2005 by Lutz Sammer, Nehal Mistry
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
//      $Id$

//@{

/*----------------------------------------------------------------------------
-- Includes
----------------------------------------------------------------------------*/

#include <stdio.h>
#include "stratagus.h"

#include <stdlib.h>
#include <string.h>

#include "SDL.h"

#include "iolib.h"
#include "iocompat.h"
#include "sound.h"
#include "sound_server.h"
#include "interface.h"
#include "campaign.h"
#include "util.h"
#include "script.h"

#include "cdaudio.h"

/*----------------------------------------------------------------------------
-- Declaration
----------------------------------------------------------------------------*/

#define SoundFrequency 44100 // sample rate of dsp

/*----------------------------------------------------------------------------
-- Variables
----------------------------------------------------------------------------*/

std::vector<PlaySection> PlaySections; /// Play Sections
PlaySectionType CurrentPlaySection;    /// Current Play Section

SDL_mutex *MusicFinishedMutex;         /// Mutex for MusicFinished
static bool MusicFinished;             /// Music ended and we need a new file

bool CallbackMusic;                    /// flag true callback ccl if stops

/*----------------------------------------------------------------------------
-- Functions
----------------------------------------------------------------------------*/

/**
**  Callback for when music has finished
**  Note: we are in the sdl audio thread
*/
static void MusicFinishedCallback(void)
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

	if ((proceed || force) && IsMusicEnabled() && CallbackMusic) {
		lua_pushstring(Lua, "MusicStopped");
		lua_gettable(Lua, LUA_GLOBALSINDEX);
		if (!lua_isfunction(Lua, -1)) {
			fprintf(stderr, "No MusicStopped function in Lua\n");
			StopMusic();
		} else {
			LuaCall(0, 1);
		}
	}
}

/**
**  FIXME: docu
*/
void PlaySectionMusic(PlaySectionType section)
{
#ifdef USE_CDAUDIO
	int track;
	int newtrack;
#endif
	int i;
	int j;
	int found;
	int numfiles;

	if (PlaySections.empty()) {
		return;
	}

	if (section == PlaySectionUnknown) {
		section = CurrentPlaySection;
	}

	if (section == PlaySectionStats) {
		if (GameResult == GameVictory) {
			section = PlaySectionStatsVictory;
		} else {
			section = PlaySectionStatsDefeat;
		}
	}

	for (i = 0; i < (int)PlaySections.size(); ++i) {
		if (PlaySections[i].Type == section && (!PlaySections[i].Race ||
				!(strcmp(PlaySections[i].Race, PlayerRaces.Name[ThisPlayer->Race])))) {
			break;
		}
	}
	CurrentPlaySection = PlaySections[i].Type;

#ifdef USE_CDAUDIO
	if (CDMode == CDModeDefined) {
		track = CDTrack;
		newtrack = 0;
		if ((1 << track) & PlaySections[i].CDTracks) {
			newtrack = 0;
		} else {
			if (!((1 << CDTrack) & PlaySections[i].CDTracks)) {
				CDTrack = 0;
			}
			if (PlaySections[i].CDOrder == PlaySectionOrderAll) {
				for (j = CDTrack + 1; j != CDTrack; ++j) {
					if ((1 << j) & PlaySections[i].CDTracks) {
						newtrack = j;
						break;
					} else if (j == 31) {
						j = 0;
					}
				}
			} else if (PlaySections[i].CDOrder == PlaySectionOrderRandom) {
					do {
					newtrack = MyRand() % NumCDTracks;
				} while (!((1 << newtrack) & PlaySections[i].CDTracks) ||
					(!IsAudioTrack(newtrack)));
			}
		}
		if (newtrack) {
			PlayCDTrack(newtrack);
			CDTrack = newtrack;
		}
	} else if (PlaySections[i].Files && (CDMode == CDModeOff || CDMode == CDModeStopped)) {
#else
	if (PlaySections[i].Files) {
#endif
		found = 0;
		numfiles = 0;
		for (j = 0; PlaySections[i].Files[j] && !found; ++j) {
			if (!strcmp(PlaySections[i].Files[j], CurrentMusicFile)) {
				found = 1;
				++numfiles;
			}
		}
		if (found) {
			if (PlaySections[i].FileOrder == PlaySectionOrderAll) {
				PlayMusic(PlaySections[i].Files[0]);
			} else if (PlaySections[i].FileOrder == PlaySectionOrderRandom) {
				j = MyRand() % numfiles;
				PlayMusic(PlaySections[i].Files[j]);
			}
		}
	}
}

/**
**  Init music
*/
void InitMusic(void)
{
	MusicFinished = false;
	MusicFinishedMutex = SDL_CreateMutex();
	SetMusicFinishedCallback(MusicFinishedCallback);
}

//@}
