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
/**@name cdaudio.h		-	The cdda format header file. */
//
//	(c) Copyright 2003 by Nehal Mistry
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

#ifndef __CDAUDIO_H__
#define __CDAUDIO_H__

//@{

#if defined(USE_SDLCD) || defined(USE_LIBCDA) || defined(USE_CDDA)

#if defined(USE_SDLCD) 
#include "SDL.h" 
#elif defined(USE_LIBCDA) 
#include "libcda.h" 
#elif defined(USE_CDDA) 
#include <linux/cdrom.h> 
#include <fcntl.h> 
#include <sys/ioctl.h> 
#include "iocompat.h"
#endif 

#include "sound_server.h"

typedef enum _cd_modes_ {
  CDModeStopped = -1,                 /// Stopped
  CDModeOff,                          /// Off
  CDModeAll,                          /// All
  CDModeRandom,                       /// Random
  CDModeDefined,                      /// Defined
} CDModes;

extern CDModes CDMode;                  /// CD mode

#ifdef USE_CDDA 
extern Sample* LoadCD(const char* name,int flags);      /// Load a cd track
extern int CDDrive; 
extern struct cdrom_tochdr CDchdr; 
extern struct cdrom_tocentry CDtocentry[64]; 
extern struct cdrom_read_audio CDdata; 
#endif 

    /// Current track
extern int CDTrack;

    /// Number of tracks on CD
extern int NumCDTracks;

    /// Play CDMode 'name'
extern int PlayCDRom(int name);

    /// Play 'track'
extern int PlayCDTrack(int track);

    /// Is 'track' an audio track?
extern int IsAudioTrack(int track);

    /// Get cd volume (0-255)
extern int GetCDVolume();

    /// Set cd volume (0-255)
extern void SetCDVolume(int vol);

    /// Resume CD
extern void ResumeCD();

    /// Pause CD
extern void PauseCD();

    /// Close CD
extern void QuitCD(void);

    /// Check the cdrom status
extern int CDRomCheck(void *);

#else

#define QuitCD()                        /// Dummy macro for without cd

#endif

//@}

#endif	// !__CDAUDIO_H__
