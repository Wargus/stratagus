//   ___________		     _________		      _____  __
//   \_	  _____/______   ____   ____ \_   ___ \____________ _/ ____\/  |_
//    |    __) \_  __ \_/ __ \_/ __ \/    \  \/\_  __ \__  \\   __\\   __|
//    |     \   |  | \/\  ___/\  ___/\     \____|  | \// __ \|  |   |  |
//    \___  /   |__|    \___  >\___  >\______  /|__|  (____  /__|   |__|
//	  \/		    \/	   \/	     \/		   \/
//  ______________________                           ______________________
//			  T H E   W A R   B E G I N S
//	   FreeCraft - A free fantasy real time strategy game engine
//
/**@name music.c		-	Background music support */
//
//	(c) Copyright 2002-2003 by Lutz Sammer
//
//	FreeCraft is free software; you can redistribute it and/or modify
//	it under the terms of the GNU General Public License as published
//	by the Free Software Foundation; only version 2 of the License.
//
//	FreeCraft is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//	GNU General Public License for more details.
//
//	$Id$

//@{

/*----------------------------------------------------------------------------
--	Includes
----------------------------------------------------------------------------*/

#include <stdio.h>
#include "freecraft.h"

#if defined(WITH_SOUND)	// {

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

/*----------------------------------------------------------------------------
--	Declaration
----------------------------------------------------------------------------*/

#define SoundFrequency	44100		// sample rate of dsp

/*----------------------------------------------------------------------------
--	Variables
----------------------------------------------------------------------------*/

#if defined(USE_OGG) || defined(USE_FLAC) || defined(USE_MAD) || defined(USE_LIBMODPLUG)
global Sample* MusicSample;		/// Music samples
#endif

global char* CurrentMusicFile = NULL;

#if defined(USE_SDLCD) || defined(USE_LIBCDA) || defined(USE_CDDA)
global int CDTrack = 0;			/// Current cd track
#endif

#if defined(USE_SDLCD)
global SDL_CD *CDRom;			/// SDL cdrom device
#elif defined(USE_LIBCDA)
global int NumCDTracks;			/// Number of tracks on the cd
#elif defined(USE_CDDA)
global int NumCDTracks;
global int CDDrive;
global struct cdrom_tochdr CDchdr;
global struct cdrom_tocentry CDtocentry[64];
global struct cdrom_read_audio CDdata;
#endif

/*----------------------------------------------------------------------------
--	Functions
----------------------------------------------------------------------------*/

/**
**	Stop the current playing music.
**
**	@todo 	FIXME: Stop the CD-PLAYER.
*/
global void StopMusic(void)
{
    if (PlayingMusic) {
	PlayingMusic = 0;		// Callback!
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
**      Read next samples from libmodplug object.
**
**      @param o        pointer to object.
**      @param buf      buffer to fill.
**      @param len      length of buffer in bytes.
**
**      @return         Number of bytes filled.
*/
local int ModRead(Sample* o, void* buf, int len)
{
    return ModPlug_Read(o->User, buf, len);
}

/**
**      Free the sample of libmodplug object.
**
**      @param o        pointer to object.
*/
local void ModFree(Sample* o)
{
    ModPlug_Unload(o->User);
    free(o);
}

/**
**	Libmodplug object type structure.
*/
local const SampleType ModSampleType = {
    ModRead,
    ModFree,
};

/**
**	Load a mod file.
**
**	@param name	A possible mod file.
**	@param flags	Load flags.
**
**	@return		Sample to mix the mod, if the file is a mod.
**
**	@todo		If CL supports file size query, loading can be done
**			faster, perhaps we can rewrite modplug to support
**			streaming.
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

    ticks=GetTicks();
    DebugLevel0Fn("Trying `%s'\n" _C_ name);
    if (!(f = CLopen(name))) {
	printf("Can't open file `%s'\n", name);
	return NULL;
    }

    // Load complete file into memory, with realloc = slow
    size = 0;
    n = 16384;
    buffer = malloc(n);
    while ((i = CLread(f, buffer + size, n)) == n) {
	size += n;
	if (n < 1024*1024) {
	    n <<= 1;
	} else {
	    n = 2*1024*1024;
	}
	buffer = realloc(buffer, size + n);
    }
    size += i;
    buffer = realloc(buffer, size);

    CLclose(f);

    StopMusic();			// stop music before new music

    ModPlug_GetSettings(&settings);	// Conversion settings
    settings.mFrequency = SoundFrequency;
#ifdef USE_LIBMODPLUG32
    settings.mBits = 32;
#else
    settings.mBits = 16;
#endif
    settings.mLoopCount = 0;		// Disable looping
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

#ifdef USE_SDLCD
/**
**	Play music from cdrom.
**
**	:all :random :off
**
**	@param name	Name starting with ":".
**
**	@return		True if name is handled by the cdrom module.
*/
global int PlayCDRom(int name)
{
    // Old mode off, starting cdrom play.
    if (CDMode == CDModeOff) {
	if (!strncmp(name, ":", 1)) {
	    if (SDL_Init(SDL_INIT_CDROM) < 0)
		return 1;
	    CDRom = SDL_CDOpen(0);
	    if (!SDL_CDStatus(CDRom)) {
		CDMode = CDModeOff;
		return 1;
	    }
	}
    }

    // CDPlayer command?
    if (!strncmp(name, ":", 1)) {

	StopMusic();

	if (!CDRom) {
	    fprintf(stderr, "Couldn't open cdrom drive: %s\n", SDL_GetError());
	    CDMode = CDModeStopped;
	    return 1;
	}
	// if mode is play all tracks
	if (!strcmp(name, ":all")) {
	    CDMode = CDModeAll;
	    if (SDL_CDPlayTracks(CDRom, 0, 0, 0, 0) < 0)
		CDMode = CDModeStopped;
	    return 1;
	}
	// if mode is play random tracks
	if (!strcmp(name, ":random")) {
	    CDMode = CDModeRandom;
	    CDTrack = MyRand() % CDRom->numtracks;
	    if (SDL_CDPlayTracks(CDRom, CDTrack, 0, 0, 0) < 0)
		CDMode = CDModeStopped;
	}
	return 1;
    }

    return 0;
}
#elif defined(USE_LIBCDA)
/**
**	Play music from cdrom.
**
**	FIXME: remove :defined
**	:all :random :off :defined
**
**	@param name	Name starting with ":".
**
**	@return		True if name is handled by the cdrom module.
*/
global int PlayCDRom(int name)
{
    int i;
    int data_cd;
    int track;

    if (CDMode == CDModeOff) {
        if (cd_init()) {
	    fprintf(stderr, "Error initialising libcda \n");
	    CDMode = CDModeOff;
	    return 1;
	}
	if (cd_get_tracks(&CDTrack, &NumCDTracks)) {
	    CDMode = CDModeOff;
	    return 1;
	}
	data_cd = 1;
	for (i = 1; i <= NumCDTracks; ++i) {
	    if (cd_is_audio(i) > 0) {
	        data_cd = 0;
	        break;
	    }
	}
	if (data_cd || !NumCDTracks) {
	    CDMode = CDModeOff;
	    return 1;
	}
    }
    --CDTrack;

    StopMusic();

    if (cd_get_tracks(NULL, NULL) == -1)
        return 1;

    // if mode is play all tracks
    if (name == CDModeAll) {
        CDMode = CDModeAll;
	do {
	    if (CDTrack >= NumCDTracks)
	        CDTrack = 0;
	} while (cd_is_audio(++CDTrack) < 1);
	if (cd_play(CDTrack))
	    CDMode = CDModeStopped;
	return 1;
    }
    // if mode is play random tracks
    if (name == CDModeRandom) {
        CDMode = CDModeRandom;
        do {
	    CDTrack = MyRand() % NumCDTracks;
	} while (cd_is_audio(CDTrack) < 1);
	if (cd_play(CDTrack))
	    CDMode = CDModeStopped;
	return 1;
    }

    if (name == CDModeDefined) {
        CDMode = CDModeDefined;
	return 0;
        track = cd_current_track();
        if (CurrentPlaySection == PlaySectionStats) {
	    if (GameResult == GameVictory) {
		if (!ThisPlayer->Race && track != 8) {
		    cd_play(8);
		} else if (ThisPlayer->Race && track != 16) {
		    cd_play(16);
		}
	    } else {
		if (!ThisPlayer->Race && track != 9) {
		    cd_play(9);
		} else if (ThisPlayer->Race && track != 17) {
		    cd_play(17);
		}
	    }
	} else if (CurrentPlaySection == PlaySectionBriefing) {
	    if (!ThisPlayer->Race && track != 7) {
	        cd_play(7);
	    } else if (ThisPlayer->Race && track != 15) {
	        cd_play(15);
	    }
	} else if (CurrentPlaySection == PlaySectionMainMenu && track != 15) {
	    cd_play(15);
	} else if (CurrentPlaySection == PlaySectionGame && 
		    !ThisPlayer->Race && (track < 3 || track > 6)) {
	    do CDTrack = (MyRand() % NumCDTracks) + 3;
	    while (CDTrack < 3 || CDTrack > 7); 
	    cd_play(CDTrack);
	} else if (CurrentPlaySection == PlaySectionGame && 
		    ThisPlayer->Race && (track < 10 || track > 14)) {
	    do CDTrack = (MyRand() % NumCDTracks) + 9;
	    while (CDTrack < 11 || CDTrack > 14); 
	    cd_play(CDTrack);
	}
    }

    return 0;
}
#elif defined(USE_CDDA)
/**
**	Play music from cdrom.
**
**	:all :random :off
**
**	@param name	Name starting with ":".
**
**	@return		True if name is handled by the cdrom module.
*/
local int PlayCDRom(const char* name)
{
    int i;
    Sample *sample;

    if (CDMode == CDModeOff) {
	if (!strncmp(name, ":", 1)) {
	    CDDrive = open("/dev/cdrom", O_RDONLY | O_NONBLOCK);
	    ioctl(CDDrive, CDROMRESET);

	    ioctl(CDDrive, CDROMREADTOCHDR, &CDchdr);

	    for (i = CDchdr.cdth_trk0; i <= CDchdr.cdth_trk1; ++i){
		CDtocentry[i].cdte_format = CDROM_LBA;
		CDtocentry[i].cdte_track = i;
		ioctl(CDDrive, CDROMREADTOCENTRY, &CDtocentry[i]);
	    }
	    NumCDTracks = i - 1;

	    if (NumCDTracks == 0) {
		CDMode = CDModeOff;
		return 1;
	    }
	}
    }

    if (!strncmp(name, ":", 1)) {

	StopMusic();

	// if mode is play all tracks
	if (!strcmp(name, ":all")) {
	    CDMode = CDModeAll;
	    do {
		if (CDTrack >= NumCDTracks)
		    CDTrack = 0;
	    } while (CDtocentry[++CDTrack].cdte_ctrl&CDROM_DATA_TRACK);
	}
	// if mode is play random tracks
	if (!strcmp(name, ":random")) {
	    CDMode = CDModeRanom;
	    do {
		CDTrack = MyRand() % NumCDTracks;
	    } while (CDtocentry[CDTrack].cdte_ctrl&CDROM_DATA_TRACK);
	}

	sample = LoadCD(NULL, CDTrack);
	MusicSample = sample;
	PlayingMusic = 1;
	return 1;
    }

    return 0;

}
#endif

/**
**	FIXME: docu
*/
global void PlaySectionMusic(PlaySectionType section)
{
    int track;
    int newtrack;
    int i;
    int j;
    int found;
    int numfiles;

    if (NumPlaySections == 0) {
	return;
    }

    newtrack = 0;
    j = 0;
    track = cd_current_track();

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

    if (CDMode == CDModeDefined) {
	if ( (1 << track) & PlaySections[i].CDTracks ) {
	    newtrack = 0;
	} else {
	    if (PlaySections[i].CDOrder == PlaySectionOrderAll) {
		for (j = 1; j < 32; ++j) {
		    if ( (1 << j) & PlaySections[i].CDTracks ) {
			newtrack = j;
			break;
		    }
		}
	    } else if (PlaySections[i].CDOrder == PlaySectionOrderRandom) {
    		do {
		    newtrack = MyRand() % NumCDTracks;
		    printf("%d\n", newtrack);
		} while ( !((1 << newtrack) & PlaySections[i].CDTracks) || 
		    (cd_is_audio(newtrack) < 1) );
	    }
	}
	if (newtrack) {
	    cd_play(newtrack);
	}
    } else if (PlaySections[i].Files && (CDMode == CDModeOff || CDMode == CDModeStopped)) {
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
**	Play a music file.
**
**	Currently supported are .mod, .it, .s3m, .wav, .xm.
**	Optional .ogg, .mp3, .flac and cdrom.
**
**	@param name	Name of sound file, format is automatic detected.
**			Names starting with ':' control the cdrom.
*/
global void PlayMusic(const char* name)
{
    char buffer[PATH_MAX];
#if defined(USE_OGG) || defined(USE_FLAC) || defined(USE_MAD) || defined(USE_LIBMODPLUG)
    Sample* sample;
#endif

    if (MusicOff) {
	return;
    }

    if (CurrentMusicFile) {
	free(CurrentMusicFile);
    }
    CurrentMusicFile = strdup(name);

    name = LibraryFileName(name, buffer);

#ifdef USE_OGG
    if ((sample = LoadOgg(name, PlayAudioStream))) {
	if ((sample->Channels != 1 && sample->Channels != 2)
		|| sample->SampleSize != 16
		|| sample->Frequency != SoundFrequency) {
	    DebugLevel0Fn("Not supported music format\n");
	    SoundFree(sample);
	    return;
	}
	StopMusic();
	MusicSample = sample;
	PlayingMusic = 1;
	return;
    }
#endif
#ifdef USE_MAD
    if ((sample = LoadMp3(name, PlayAudioStream))) {
	if (sample->Channels != 2 || sample->SampleSize != 16
	    || sample->Frequency != SoundFrequency) {
	    DebugLevel0Fn("Not supported music format\n");
	    SoundFree(sample);
	    return;
	}
	StopMusic();
	MusicSample = sample;
	PlayingMusic = 1;
	return;
    }
#endif
#ifdef USE_FLAC
    if ((sample = LoadFlac(name, PlayAudioStream))) {
	if (sample->Channels != 2 || sample->SampleSize != 16
	    || sample->Frequency != SoundFrequency) {
	    DebugLevel0Fn("Not supported music format\n");
	    SoundFree(sample);
	    return;
	}
	StopMusic();
	MusicSample = sample;
	PlayingMusic = 1;
	return;
    }
#endif
#ifdef USE_LIBMODPLUG
    if ((sample = LoadMod(name, PlayAudioStream))) {
	MusicSample = sample;
	PlayingMusic = 1;
	return;
    }
#endif
}

/**
**	Play a sound file.
**
**	Currenly a synomy for PlayMusi
**
**	@param name	Name of sound file, format is automatic detected.
**			Names starting with ':' control the cdrom.
*/
global void PlayFile(const char* name)
{
    MusicOff = 0;
    PlayMusic(name);
}

#endif	// } WITH_SOUND

//@}
