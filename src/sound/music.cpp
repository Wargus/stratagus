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

#ifdef USE_LIBCDA
#include "libcda.h"
#endif

#ifdef USE_SDLCD
#include <SDL.h>
#endif

#include "iolib.h"
#include "sound.h"
#include "sound_server.h"

/*----------------------------------------------------------------------------
--	Declaration
----------------------------------------------------------------------------*/

#define SoundFrequency	44100		// sample rate of dsp

/*----------------------------------------------------------------------------
--	Variables
----------------------------------------------------------------------------*/

#ifdef USE_LIBMODPLUG
global ModPlugFile* ModFile;		/// Mod file loaded into memory
#endif
#if defined(USE_OGG) || defined(USE_FLAC) || defined(USE_MAD)
global Sample* MusicSample;		/// Music samples
global int MusicIndex;			/// Music sample index
#endif
#if defined(USE_SDLCD) || defined(USE_LIBCDA)
global char *CDMode = ":off";	/// cd play mode, ":off" ":random" or ":all"
global int CDTrack;			/// Current cd track
#endif
#ifdef USE_SDLCD
global SDL_CD *CDRom;			/// SDL cdrom device
#endif
#ifdef USE_LIBCDA
global int NumCDTracks;			/// Number of tracks on the cd
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
#ifdef USE_LIBMODPLUG
	if (ModFile) {
	    ModPlug_Unload(ModFile);
	    ModFile = NULL;
	    return;
	}
#endif
#if defined(USE_OGG) || defined(USE_FLAC) || defined(USE_MAD)
	if (MusicSample) {
	    if (MusicSample->Type) {
		MusicSample->Type->Free(MusicSample);
		free(MusicSample->Type);
	    }
	    free(MusicSample);
	    MusicSample = NULL;
	    MusicIndex = 0;
	    return;
	}
#endif
    }
}

#ifdef USE_LIBMODPLUG
/**
**	Load a mod file.
*/
local int LoadMod(const char* name)
{
    ModPlug_Settings settings;
    CLFile* f;
    char* buffer;
    int size;
    int i;
    
    ModPlug_GetSettings(&settings);
    settings.mFrequency=SoundFrequency;
#ifdef USE_LIBMODPLUG32
    settings.mBits=32;
#else
    settings.mBits=16;
#endif
    settings.mLoopCount=0;		// Disable looping
    ModPlug_SetSettings(&settings);

    buffer=malloc(8192);
    DebugLevel2Fn("Loading `%s'\n" _C_ name);

    if( !(f=CLopen(name)) ) {
	printf("Can't open file `%s'\n",name);
	return 0;
    }

    size=0;
    while( (i=CLread(f,buffer+size,8192))==8192 ) {
	size+=8192;
	buffer=realloc(buffer,size+8192);
    }
    size+=i;
    buffer=realloc(buffer,size);
    DebugLevel3Fn("%d\n" _C_ size);

    StopMusic();			// stop music before new music

    ModFile=ModPlug_Load(buffer,size);

    free(buffer);

    if( ModFile ) {
	DebugLevel0Fn("Started\n");
	PlayingMusic=1;
	return 1;
    }

    return 0;
}
#endif

#ifdef USE_SDLCD
/**
**	Play music from cdrom.
**
**	:all :random :off
**
**	@param name	Name starting with ":".
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
    
    StopMusic();

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
*/
local int PlayCDRom(const char* name)
{
    int i, DataCd;

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
	    DataCd = 1;
	    for (i=1; i <= NumCDTracks; ++i) {
		if (cd_is_audio(i) > 0) {
		    DataCd = 0;
		    break;
		}
	    }
	    if (DataCd) {
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

/**
**	Play a music file.
**	Currently supported are .mod, .it, .s3m, .wav, .xm.
**	Optional .ogg, mp3 and cdrom.
**
**	Some quick hacks for mods.
**
**	@param name	Name of sound file, format is automatic detected.
**			Names starting with ':' control the cdrom.
*/
global void PlayMusic(const char* name)
{
    char buffer[1024];
#if defined(USE_OGG) || defined(USE_FLAC) || defined(USE_MAD)
    Sample *sample;
#endif

#ifdef USE_SDLCD
    if (PlayCDRom(name)) {
	return;
    }
#endif
#ifdef USE_LIBCDA
    if (PlayCDRom(name)) {
	return;
    }
#endif
    name = LibraryFileName(name, buffer);

#if defined(USE_SDLCD) || defined(USE_LIBCDA)
    if (strcmp(CDMode,":off") && strcmp(CDMode,":stopped")) {
	return;
    }
#endif

#ifdef USE_OGG
    if ((sample = LoadOggStreaming(name))) {
	if( sample->Channels!=2
		|| sample->SampleSize!=16
		|| sample->Frequency!=SoundFrequency ) {
	    DebugLevel0Fn("Not supported music format\n");
	    free(sample);
	    return;
	}
	StopMusic();
	MusicSample = sample;
	MusicIndex = 0;
	PlayingMusic = 1;
	return;
    }
#endif
#ifdef USE_MAD
    if ((sample = LoadMp3(name))) {
	if( sample->Channels!=2
		|| sample->SampleSize!=16
		|| sample->Frequency!=SoundFrequency ) {
	    DebugLevel0Fn("Not supported music format\n");
	    free(sample);
	    return;
	}
	StopMusic();
	MusicSample = sample;
	MusicIndex = 0;
	PlayingMusic = 1;
	return;
    }
#endif
#ifdef USE_FLAC
    if ((sample = LoadFlac(name))) {
	if( sample->Channels!=2
		|| sample->SampleSize!=16
		|| sample->Frequency!=SoundFrequency ) {
	    DebugLevel0Fn("Not supported music format\n");
	    free(sample);
	    return;
	}
	StopMusic();
	MusicSample = sample;
	MusicIndex = 0;
	PlayingMusic = 1;
	return;
    }
#endif
#ifdef USE_LIBMODPLUG
    if (LoadMod(name)) {
	return;
    }
#endif
}

global void PlayFile(const char* name)
{
    char buffer[1024];
#if defined(USE_OGG) || defined(USE_FLAC) || defined(USE_MAD)
    Sample *sample;
#endif
    name = LibraryFileName(name, buffer);
#ifdef USE_OGG
    if ((sample = LoadOgg(name))) {
	if( sample->Channels!=2
		|| sample->SampleSize!=16
		|| sample->Frequency!=SoundFrequency ) {
	    DebugLevel0Fn("Not supported music format\n");
	    free(sample);
	    return;
	}
	StopMusic();
	MusicSample = sample;
	MusicIndex = 0;
	PlayingMusic = 1;
	return;
    }
#endif
#ifdef USE_MAD
    if ((sample = LoadMp3(name))) {
	if( sample->Channels!=2
		|| sample->SampleSize!=16
		|| sample->Frequency!=SoundFrequency ) {
	    DebugLevel0Fn("Not supported music format\n");
	    free(sample);
	    return;
	}
	StopMusic();
	MusicSample = sample;
	MusicIndex = 0;
	PlayingMusic = 1;
	return;
    }
#endif
#ifdef USE_FLAC
    if ((sample = LoadFlac(name))) {
	if( sample->Channels!=2
		|| sample->SampleSize!=16
		|| sample->Frequency!=SoundFrequency ) {
	    DebugLevel0Fn("Not supported music format\n");
	    free(sample);
	    return;
	}
	StopMusic();
	MusicSample = sample;
	MusicIndex = 0;
	PlayingMusic = 1;
	return;
    }
#endif
#ifdef USE_LIBMODPLUG
    if (LoadMod(name)) {
	return;
    }
#endif
}

#endif	// } WITH_SOUND

//@}
