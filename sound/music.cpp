//   ___________		     _________		      _____  __
//   \_	  _____/______   ____   ____ \_   ___ \____________ _/ ____\/  |_
//    |    __) \_  __ \_/ __ \_/ __ \/    \  \/\_  __ \__  \\   __\\   __\ 
//    |     \   |  | \/\  ___/\  ___/\     \____|  | \// __ \|  |   |  |
//    \___  /   |__|    \___  >\___  >\______  /|__|  (____  /__|   |__|
//	  \/		    \/	   \/	     \/		   \/
//  ______________________                           ______________________
//			  T H E   W A R   B E G I N S
//	   FreeCraft - A free fantasy real time strategy game engine
//
/**@name music.c		-	Background music support */
//
//	(c) Copyright 2002 by Lutz Sammer
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
#include <SDL.h>
#endif

#include "iolib.h"
#include "iocompat.h"
#include "sound.h"
#include "sound_server.h"

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

#if defined(USE_SDLCD) || defined(USE_LIBCDA) || defined(USE_CDDA)
global char *CDMode = ":off";	/// cd play mode, ":off" ":random" or ":all"
global int CDTrack = 1;			/// Current cd track
#endif

#if defined(USE_SDLCD)
global SDL_CD *CDRom;			/// SDL cdrom device
#elif defined(USE_LIBCDA)
global int NumCDTracks;			/// Number of tracks on the cd
#elif defined(USE_CDDA)
// FIXME: fill up
global int NumCDTracks;
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
local int PlayCDRom(const char* name)
{
    // Old mode off, starting cdrom play.
    if (!strcmp(CDMode, ":off")) {
	if (!strncmp(name, ":", 1)) {
	    if (SDL_Init(SDL_INIT_CDROM) < 0)
		return 1;
	    CDRom = SDL_CDOpen(0);
	    if (!SDL_CDStatus(CDRom)) {
		CDMode = ":off";
		return 1;
	    }
	}
    }
    // CDPlayer command?
    if (!strncmp(name, ":", 1)) {
	if (!CDRom) {
	    fprintf(stderr, "Couldn't open cdrom drive: %s\n", SDL_GetError());
	    CDMode = ":stopped";
	    return 1;
	}
	// if mode is play all tracks
	if (!strcmp(name, ":all")) {
	    CDMode = ":all";
	    if (SDL_CDPlayTracks(CDRom, 0, 0, 0, 0) < 0)
		CDMode = ":stopped";
	    return 1;
	}
	// if mode is play random tracks
	if (!strcmp(name, ":random")) {
	    CDMode = ":random";
	    CDTrack = MyRand() % CDRom->numtracks;
	    if (SDL_CDPlayTracks(CDRom, CDTrack, 0, 0, 0) < 0)
		CDMode = ":stopped";
	}
	return 1;
    }

    StopMusic();			// FIXME: JOHNS: why stop music here?

    // FIXME: no cdrom, must stop it now!

    return 0;
}
#endif

#ifdef USE_LIBCDA
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
    int data_cd;

    if (!strcmp(CDMode, ":off")) {
	if (!strncmp(name, ":", 1)) {
	    if (cd_init()) {
		fprintf(stderr, "Error initialising libcda \n");
		CDMode = ":off";
		return 1;
	    }
	    if (cd_get_tracks(&CDTrack, &NumCDTracks)) {
		CDMode = ":off";
		return 1;
	    }
	    data_cd = 1;
	    for (i = 1; i <= NumCDTracks; ++i) {
		if (cd_is_audio(i) > 0) {
		    data_cd = 0;
		    break;
		}
	    }
	    if (data_cd) {
		CDMode = ":off";
		return 1;
	    }
	    if (NumCDTracks == 0) {
		CDMode = ":off";
		return 1;
	    }
	    --CDTrack;
	}
    }

    StopMusic();

    if (!strncmp(name, ":", 1)) {

	if (cd_get_tracks(NULL, NULL) == -1)
	    return 1;

	// if mode is play all tracks
	if (!strcmp(name, ":all")) {
	    CDMode = ":all";
	    do {
		if (CDTrack > NumCDTracks)
		    CDTrack = 1;
	    } while (cd_is_audio(++CDTrack) < 1);
	    if (cd_play(CDTrack))
		CDMode = ":stopped";
	    return 1;
	}
	// if mode is play random tracks
	if (!strcmp(name, ":random")) {
	    CDMode = ":random";
	    do {
		CDTrack = MyRand() % NumCDTracks;
	    } while (cd_is_audio(CDTrack) < 1);
	    if (cd_play(CDTrack))
		CDMode = ":stopped";
	    return 1;
	}
	return 1;
    }
    // FIXME: no cdrom, must stop it now!

    return 0;
}
#endif

#ifdef USE_CDDA
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
    cdrom_drive *CddaDrive = NULL;
    void *buf;
//    cdrom_paranoia *Cdda;

    if (!strcmp(CDMode, ":off")) {
	if (!strncmp(name, ":", 1)) {
	    CddaDrive = cdda_find_a_cdrom(0, NULL);
	    cdda_open(CddaDrive);
//	    Cdda = paranoia_init(CddaDrive);

	    NumCDTracks = cdda_tracks(CddaDrive);

	    if (NumCDTracks == -1) {
		CDMode = ":off";
		return 1;
	    }
	    --CDTrack;
	}
    }

    StopMusic();

    if (!strncmp(name, ":", 1)) {

	// if mode is play all tracks
	if (!strcmp(name, ":all")) {
	    CDMode = ":all";
	    
	    do {
		if (CDTrack > NumCDTracks)
		    CDTrack = 1;
	    } while (cdda_track_audiop(CddaDrive, ++CDTrack) == 0);
	    
	    // temporary
	    fprintf(stderr, "AAAAAAAAAAa %d\n",CDTrack);
	    CDTrack = 3;
	    
	    buf = malloc(512*100);
	    cdda_read(CddaDrive, buf, cdda_track_firstsector(CddaDrive, CDTrack), 100);

	    free(buf);

	    return 1;
	}
	// if mode is play random tracks
	if (!strcmp(name, ":random")) {
	    CDMode = ":random";

	    do {
		CDTrack = MyRand() % NumCDTracks;
	    } while (cdda_track_audiop(CddaDrive, ++CDTrack) == 0);

	    buf = malloc(512*100);
	    cdda_read(CddaDrive, buf, cdda_track_firstsector(CddaDrive, CDTrack), 100);

	    free(buf);

	    return 1;
	}
	return 1;
    }
    // FIXME: no cdrom, must stop it now!

    return 0;

}
#endif

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

#if defined(USE_SDLCD) || defined(USE_LIBCDA) || defined(USE_CDDA)
    if (PlayCDRom(name)) {
	return;
    }
    if (strcmp(CDMode, ":off") && strcmp(CDMode, ":stopped")) {
	return;
    }
#endif

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
    PlayMusic(name);
}

#endif	// } WITH_SOUND

//@}
