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
/**@name music.c		-	Background music support */
//
//	(c) Copyright 2002-2003 by Lutz Sammer
//
//      This program is free software; you can redistribute it and/or modify
//      it under the terms of the GNU General Public License as published by
//      the Free Software Foundation; version 2 dated June, 1991.
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

#if defined(WITH_SOUND)		// {

#include <stdlib.h>
#include <string.h>

#ifdef USE_LIBMODPLUG
#include "../libmodplug/modplug.h"
#endif

#ifdef USE_SDL
#include "SDL.h"
#endif

#include "iolib.h"
#include "iocompat.h"
#include "sound.h"
#include "sound_server.h"
#include "interface.h"
#include "campaign.h"

#include "cdaudio.h"

/*----------------------------------------------------------------------------
--		Declaration
----------------------------------------------------------------------------*/

#define SoundFrequency		44100				// sample rate of dsp

/*----------------------------------------------------------------------------
--		Variables
----------------------------------------------------------------------------*/

#if defined(USE_OGG) || defined(USE_FLAC) || defined(USE_MAD) || defined(USE_LIBMODPLUG)
global Sample* MusicSample;			 /// Music samples
#endif

global char* CurrentMusicFile;

global PlaySection* PlaySections;				// Play Sections
global int NumPlaySections;						// Number of Play Sections
global PlaySectionType CurrentPlaySection;		// Current Play Section

/*----------------------------------------------------------------------------
--		Functions
----------------------------------------------------------------------------*/

/**
**		Stop the current playing music.
**
**		@todo 		FIXME: Stop the CD-PLAYER.
*/
global void StopMusic(void)
{
	if (PlayingMusic) {
		PlayingMusic = 0;				// Callback!
#if defined(USE_OGG) || defined(USE_FLAC) || defined(USE_MAD) || defined(USE_LIBMODPLUG)
		if (MusicSample) {
#ifdef USE_SDL
			SDL_LockAudio();
#endif
			SoundFree(MusicSample);
			MusicSample = NULL;
#ifdef USE_SDL
			SDL_UnlockAudio();
#endif
			return;
		}
#endif
	}
}

#ifdef USE_LIBMODPLUG
/**
**	  Read next samples from libmodplug object.
**
**	  @param o		pointer to object.
**	  @param buf	  buffer to fill.
**	  @param len	  length of buffer in bytes.
**
**	  @return		 Number of bytes filled.
*/
local int ModRead(Sample* o, void* buf, int len)
{
	return ModPlug_Read(o->User, buf, len);
}

/**
**	  Free the sample of libmodplug object.
**
**	  @param o		pointer to object.
*/
local void ModFree(Sample* o)
{
	ModPlug_Unload(o->User);
	free(o);
}

/**
**		Libmodplug object type structure.
*/
local const SampleType ModSampleType = {
	ModRead,
	ModFree,
};

/**
**		Load a mod file.
**
**		@param name		A possible mod file.
**		@param flags		Load flags.
**
**		@return				Sample to mix the mod, if the file is a mod.
**
**		@todo				If CL supports file size query, loading can be done
**						faster, perhaps we can rewrite modplug to support
**						streaming.
*/
local Sample* LoadMod(const char* name,int flags __attribute__((unused)))
{
	ModPlug_Settings settings;
	ModPlugFile* modfile;
	Sample* sample;
	CLFile* f;
	char* buffer;
	int size;
	int i;
	int ticks;
	int n;

	ticks = GetTicks();
	DebugLevel0Fn("Trying `%s'\n" _C_ name);
	if (!(f = CLopen(name,CL_OPEN_READ))) {
		printf("Can't open file `%s'\n", name);
		return NULL;
	}

	// Load complete file into memory, with realloc = slow
	size = 0;
	n = 16384;
	buffer = malloc(n);
	while ((i = CLread(f, buffer + size, n)) == n) {
		size += n;
		if (n < 1024 * 1024) {
			n <<= 1;
		} else {
			n = 2 * 1024 * 1024;
		}
		buffer = realloc(buffer, size + n);
	}
	size += i;
	buffer = realloc(buffer, size);

	CLclose(f);

	StopMusic();						// stop music before new music

	ModPlug_GetSettings(&settings);		// Conversion settings
	settings.mFrequency = SoundFrequency;
#ifdef USE_LIBMODPLUG32
	settings.mBits = 32;
#else
	settings.mBits = 16;
#endif
	settings.mLoopCount = 0;				// Disable looping
	ModPlug_SetSettings(&settings);

	modfile = ModPlug_Load(buffer, size);

	free(buffer);

	if (modfile) {
		DebugLevel0Fn("Started ticks %ld\n" _C_ GetTicks() - ticks);
		sample = malloc(sizeof(*sample));
		sample->Type = &ModSampleType;
		sample->User = modfile;
		sample->Channels = 2;
#ifdef USE_LIBMODPLUG32
		sample->SampleSize = 32;
#else
		sample->SampleSize = 16;
#endif
		sample->Frequency = SoundFrequency;
		sample->Length = 0;
		return sample;
	}

	return NULL;
}
#endif

/**
**		FIXME: docu
*/
global void PlaySectionMusic(PlaySectionType section)
{
#ifdef USE_CDAUDIO
	int track;
	int newtrack;
#endif
	int i;
	int j;
	int found;
	int numfiles;

	if (NumPlaySections == 0) {
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

	for (i = 0; i < NumPlaySections; ++i) {
		if (PlaySections[i].Type == section && (!PlaySections[i].Race ||
				!(strcmp(PlaySections[i].Race, ThisPlayer->RaceName)))) {
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
		if (!found) {
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
**		Play a music file.
**
**		Currently supported are .mod, .it, .s3m, .wav, .xm.
**		Optional .ogg, .mp3, .flac and cdrom.
**
**		@param name		Name of sound file, format is automatic detected.
**						Names starting with ':' control the cdrom.
**
**		@return				1 if music is playing, 0 if not.
*/
global int PlayMusic(const char* name)
{
	char buffer[PATH_MAX];
#if defined(USE_OGG) || defined(USE_FLAC) || defined(USE_MAD) || defined(USE_LIBMODPLUG)
	Sample* sample;
#endif

	if (MusicOff) {
		return 0;
	}

	if (CurrentMusicFile) {
		free(CurrentMusicFile);
	}
	CurrentMusicFile = strdup(name);

	name = LibraryFileName(name, buffer);

	if ((sample = LoadWav(name, PlayAudioStream))) {
		StopMusic();
		MusicSample = sample;
		PlayingMusic = 1;
		return 1;
	}

#ifdef USE_OGG
	if ((sample = LoadOgg(name, PlayAudioStream))) {
		if ((sample->Channels != 1 && sample->Channels != 2) ||
				sample->SampleSize != 16) {
			DebugLevel0Fn("Not supported music format\n");
			SoundFree(sample);
			return 0;
		}
		StopMusic();
		MusicSample = sample;
		PlayingMusic = 1;
		return 1;
	}
#endif
#ifdef USE_MAD
	if ((sample = LoadMp3(name, PlayAudioStream))) {
//		if (sample->Channels != 2 || sample->SampleSize != 16
//			|| sample->Frequency != SoundFrequency) {
//			DebugLevel0Fn("Not supported music format\n");
//			SoundFree(sample);
//			return;
//		}
		StopMusic();
		MusicSample = sample;
		PlayingMusic = 1;
		return 1;
	}
#endif
#ifdef USE_FLAC
	if ((sample = LoadFlac(name, PlayAudioStream))) {
/*
		if (sample->Channels != 2 || sample->SampleSize != 16
			|| sample->Frequency != SoundFrequency) {
			DebugLevel0Fn("Not supported music format\n");
			SoundFree(sample);
			return;
		}
*/
		StopMusic();
		MusicSample = sample;
		PlayingMusic = 1;
		return 1;
	}
#endif
#ifdef USE_LIBMODPLUG
	if ((sample = LoadMod(name, PlayAudioStream))) {
		MusicSample = sample;
		PlayingMusic = 1;
		return 1;
	}
#endif
	return 0;
}

/**
**		Play a sound file.
**
**		@param name		Name of sound file
*/
global void PlaySoundFile(const char* name)
{
	SoundId id;
	if (SoundIdForName("dynamic-sound")) {
		id = RegisterSound(&name, 1);
	} else {
		id = MakeSound("dynamic-sound", &name, 1);
	}
	PlayGameSound(id, GlobalVolume);
}

#endif		// } WITH_SOUND

//@}
