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

#include "cdaudio.h"

/*----------------------------------------------------------------------------
-- Declaration
----------------------------------------------------------------------------*/

#define SoundFrequency 44100 // sample rate of dsp

/*----------------------------------------------------------------------------
-- Variables
----------------------------------------------------------------------------*/

CSample *MusicSample;  /// Music samples

char *CurrentMusicFile;

std::vector<PlaySection> PlaySections; /// Play Sections
PlaySectionType CurrentPlaySection;    /// Current Play Section

/*----------------------------------------------------------------------------
-- Functions
----------------------------------------------------------------------------*/

/**
**  Stop the current playing music.
*/
void StopMusic(void)
{
	if (PlayingMusic) {
		PlayingMusic = false; // Callback!
		if (MusicSample) {
			SDL_LockAudio();
			delete MusicSample;
			MusicSample = NULL;
			SDL_UnlockAudio();
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
**  Play a music file.
**
**  @param name  Name of music file, format is automatically detected.
**
**  @return      0 if music is playing, -1 if not.
*/
int PlayMusic(const char *name)
{
	char buffer[PATH_MAX];
	CSample *sample;

	if (!IsMusicEnabled()) {
		return -1;
	}

	delete[] CurrentMusicFile;
	CurrentMusicFile = NULL;

	name = LibraryFileName(name, buffer);

	DebugPrint("play music %s\n" _C_ name);

	sample = LoadWav(name, PlayAudioStream);

#ifdef USE_VORBIS
	if (!sample) {
		sample = LoadVorbis(name, PlayAudioStream);
	}
#endif
#ifdef USE_MAD
	if (!sample) {
		sample = LoadMp3(name, PlayAudioStream);
	}
#endif
#ifdef USE_FLAC
	if (!sample) {
		sample = LoadFlac(name, PlayAudioStream);
	}
#endif
#ifdef USE_MIKMOD
	if (!sample) {
		sample = LoadMikMod(name, PlayAudioStream);
	}
#endif

	if (sample) {
		StopMusic();
		MusicSample = sample;
		PlayingMusic = true;
		CurrentMusicFile = new_strdup(name);
		return 0;
	} else {
		DebugPrint("Could not play %s\n" _C_ name);
		return -1;
	}
}

//@}
