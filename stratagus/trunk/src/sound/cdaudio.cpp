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
/**@name cdaudio.c - cd audio */
//
//      (c) Copyright 2003-2005 by Nehal Mistry
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
#include "cdaudio.h"
#include "util.h"

#ifdef USE_CDAUDIO

#include <stdlib.h>

#include "sound.h"
//#include "sound_server.h"


#if defined(USE_SDLCD)
#include "SDL.h"
#elif defined(USE_LIBCDA)
#include "libcda.h"
#elif defined(USE_CDDA)
#include "iocompat.h"
#endif

/*----------------------------------------------------------------------------
-- Declaration
----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
-- Variables
----------------------------------------------------------------------------*/

int CDTrack;      /// Current cd track
int NumCDTracks;  /// Number of tracks on the cd

#if defined(USE_SDLCD)
static SDL_CD* CDRom;                  /// SDL cdrom device
#elif defined(USE_CDDA)
int CDDrive;                           /// CDRom device
struct cdrom_tocentry CDtocentry[64];  /// TOC track header struct
static struct cdrom_tochdr CDchdr;     /// TOC header struct
static struct cdrom_read_audio CDdata; /// struct for reading data
#endif

CDModes CDMode; /// CD mode

/*----------------------------------------------------------------------------
-- Functions
----------------------------------------------------------------------------*/

#if defined (USE_SDLCD)
/**
** FIXME: docu
*/
static int InitCD(void)
{
	if (SDL_Init(SDL_INIT_CDROM)) {
		return 1;
	}
	CDRom = SDL_CDOpen(0);
	if (!SDL_CDStatus(CDRom)) {
		CDMode = CDModeOff;
		return 1;
	}
	NumCDTracks = CDRom->numtracks;
	return 0;
}

/**
** FIXME: docu
*/
int PlayCDTrack(int track)
{
	CDTrack = track;
	return SDL_CDPlayTracks(CDRom, track - 1, 0, 0, 0);
}

/**
** FIXME: docu
*/
void ResumeCD(void)
{
#ifdef USE_WIN32
	SDL_CDResume(CDRom);
#endif
	PlayCDRom(CDModeDefined);
}

/**
** FIXME: docu
*/
void PauseCD(void)
{
	SDL_CDPause(CDRom);
	CDTrack = 0;
	CDMode = CDModeStopped;
}

/**
** FIXME: docu
*/
int IsAudioTrack(int track)
{
	// FIXME: what is proper way?
	return 1;
}

/**
** FIXME: docu
*/
int IsCDPlaying(void)
{
	if (SDL_CDStatus(CDRom) == CD_PLAYING) {
		return 1;
	} else {
		return 0;
	}
}

/**
** FIXME: docu
*/
int GetCDVolume(void)
{
	return 0;
}

/**
** FIXME: docu
*/
void SetCDVolume(int vol)
{
	return;
}

/**
** FIXME: docu
*/
void QuitCD(void)
{
	if (CDMode != CDModeOff && CDMode != CDModeStopped) {
		SDL_CDStop(CDRom);
		CDMode = CDModeStopped;
	}
	if (CDMode != CDModeStopped) {
		SDL_CDClose(CDRom);
		CDMode = CDModeOff;
	}
}
#elif defined(USE_LIBCDA)
/**
** FIXME: docu
*/
static int InitCD(void)
{
	if (cd_init()) {
		return -1;
	} else {
		if (cd_get_tracks(NULL, &NumCDTracks)) {
			return -1;
		}
		return 0;
	}
}

/**
** FIXME: docu
*/
int PlayCDTrack(int track)
{
	CDTrack = track;
	return cd_play(track);
}

/**
** FIXME: docu
*/
void ResumeCD(void)
{
	PlayCDRom(CDModeDefined);
}

/**
** FIXME: docu
*/
void PauseCD(void)
{
	cd_pause();
	CDTrack = 0;
	CDMode = CDModeStopped;
}

/**
** FIXME: docu
*/
int IsAudioTrack(int track)
{
	return cd_is_audio(track);
}

/**
** FIXME: docu
*/
int IsCDPlaying(void)
{
	if (cd_current_track()) {
		return 1;
	} else {
		return 0;
	}
}

/**
** FIXME: docu
*/
int GetCDVolume(void)
{
	int vol;

	cd_get_volume(&vol, &vol);
	return vol;
}

/**
** FIXME: docu
*/
void SetCDVolume(int vol)
{
	cd_set_volume(vol, vol);
}

/**
** FIXME: docu
*/
void QuitCD(void)
{
	if (CDMode != CDModeOff && CDMode != CDModeStopped) {
		cd_stop();
		CDMode = CDModeStopped;
	}
	if (CDMode == CDModeStopped) {
		cd_close();
		cd_exit();
		CDMode = CDModeOff;
	}
}
#elif defined(USE_CDDA)
/**
** FIXME: docu
*/
static int InitCD(void)
{
	int i;

	CDDrive = open("/dev/cdrom", O_RDONLY | O_NONBLOCK);
	ioctl(CDDrive, CDROMRESET);
	ioctl(CDDrive, CDROMREADTOCHDR, &CDchdr);
	for (i = CDchdr.cdth_trk0; i <= CDchdr.cdth_trk1; ++i) {
		CDtocentry[i].cdte_format = CDROM_LBA;
		CDtocentry[i].cdte_track = i;
		ioctl(CDDrive, CDROMREADTOCENTRY, &CDtocentry[i]);
	}
	NumCDTracks = i - 1;

	if (NumCDTracks == 0) {
		CDMode = CDModeOff;
		return -1;
	}
	return 0;
}

/**
** FIXME: docu
*/
int PlayCDTrack(int track)
{
	Sample* sample;

	sample = LoadCD(NULL, track);
	CDTrack = track;
	MusicSample = sample;
	PlayingMusic = 1;
	return 0;
}

/**
** FIXME: docu
*/
void ResumeCD(void)
{
	PlayCDRom(CDModeDefined);
}

/**
** FIXME: docu
*/
void PauseCD(void)
{
	StopMusic();
	CDTrack = 0;
	CDMode = CDModeStopped;
}

/**
** FIXME: docu
*/
int IsAudioTrack(track)
{
	return !(CDtocentry[track].cdte_ctrl & CDROM_DATA_TRACK);
}

/**
** FIXME: docu
*/
int IsCDPlaying(void)
{
	return PlayingMusic;
}

/**
** FIXME: docu
*/
int GetCDVolume(void)
{
	return MusicVolume;
}

/**
** FIXME: docu
*/
void SetCDVolume(int vol)
{
	MusicVolume = vol;
}

/**
** FIXME: docu
*/
void QuitCD(void)
{
	close(CDDrive);
}
#endif

/**
**   Check cdrom.
**
**   Perodic called from the main loop.
*/
int CDRomCheck(void* unused __attribute__ ((unused)))
{
	if (CDMode != CDModeOff && CDMode != CDModeStopped &&
			!IsCDPlaying() && CDMode != CDModeDefined) {
		DebugPrint("Playing new track\n");
		PlayCDRom(CDMode);
	}
	return 0;
}

/**
** Play CDRom
**
** @param name name of play mode, CDModeAll, CDModeRandom, CDModeDefined
*/
int PlayCDRom(int name)
{
	int i;
	int datacd;

	if (name == CDModeOff) {
		CDMode = CDModeOff;
		return 1;
	}

	if (CDMode == CDModeOff) {
		if (InitCD()) {
			fprintf(stderr, "Error initializing cdrom\n");
			CDMode = CDModeOff;
			return 1;
		}
		datacd = 1;
		for (i = 1; i <= NumCDTracks; ++i) {
			if (IsAudioTrack(i) > 0) {
				datacd = 0;
				break;
			}
		}
		if (datacd || NumCDTracks <= 0) {
			CDMode = CDModeOff;
			fprintf(stderr, "Not an audio cd\n");
			return 1;
		}
	}

	StopMusic();

	// FIXME: when would this happen
	if (NumCDTracks <= 0) {
		return 1;
	}

	// if mode is play all tracks
	if (name == CDModeAll) {
		CDMode = CDModeAll;
		CDTrack = 0;
		do {
			if (CDTrack >= NumCDTracks) {
				CDTrack = 0;
			}
		} while (!IsAudioTrack(++CDTrack));
		if (PlayCDTrack(CDTrack)) {
			CDMode = CDModeStopped;
		}
		return 0;
	}

	// if mode is play random tracks
	if (name == CDModeRandom) {
		CDMode = CDModeRandom;
		CDTrack = 0;
		do {
			CDTrack = MyRand() % NumCDTracks;
		} while (!IsAudioTrack(CDTrack));
		if (PlayCDTrack(CDTrack)) {
			CDMode = CDModeStopped;
		}
		return 0;
	}

	if (name == CDModeDefined) {
		CDMode = CDModeDefined;
		return 0;
	}

	return 1;
}


#endif // } USE_CDAUDIO

//@}
