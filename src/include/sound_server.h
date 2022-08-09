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
/**@name sound_server.h - The sound server header file. */
//
//      (c) Copyright 1998-2005 by Lutz Sammer, Fabrice Rossi, and
//                                 Jimmy Salmon
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

#ifndef __SOUND_SERVER_H__
#define __SOUND_SERVER_H__

//@{

/*----------------------------------------------------------------------------
--  Includes
----------------------------------------------------------------------------*/

#include "sound.h"

/*----------------------------------------------------------------------------
--  Definitons
----------------------------------------------------------------------------*/

#define MaxVolume 255
#define SOUND_BUFFER_SIZE 65536

/*----------------------------------------------------------------------------
--  Functions
----------------------------------------------------------------------------*/

/// Set the channel volume
extern int SetChannelVolume(int channel, int volume);
/// Set the channel stereo
extern void SetChannelStereo(int channel, int stereo);
/// Get the sample playing on a channel
extern Mix_Chunk *GetChannelSample(int channel);
/// Stop a channel
extern void StopChannel(int channel);
/// Stop all channels
extern void StopAllChannels();

/// Check if this unit plays some sound
extern bool UnitSoundIsPlaying(Origin *origin);
/// Check, if this sample is already playing
extern bool SampleIsPlaying(Mix_Chunk *sample);
/// Load music
extern Mix_Music *LoadMusic(const std::string &name);
/// Load a sample
extern Mix_Chunk *LoadSample(const std::string &name);
extern void FreeSample(Mix_Chunk *sample);
/// Play a sample
extern int PlaySample(Mix_Chunk *sample, Origin *origin = NULL);
/// Play a sample, registering a "finished" callback
extern int PlaySample(Mix_Chunk *sample, void (*callback)(int channel));
/// Play a sound file
extern int PlaySoundFile(const std::string &name);

/// Set effects volume
extern void SetEffectsVolume(int volume);
/// Get effects volume
extern int GetEffectsVolume();
/// Set effects enabled
extern void SetEffectsEnabled(bool enabled);
/// Check if effects are enabled
extern bool IsEffectsEnabled();

/// Set the music finished callback
void SetMusicFinishedCallback(void (*callback)());
/// Play a music file
extern int PlayMusic(const std::string &file);
/// Stop music playing
extern void StopMusic();
/// Set music volume
extern void SetMusicVolume(int volume);
/// Get music volume
extern int GetMusicVolume();
/// Set music enabled
extern void SetMusicEnabled(bool enabled);
/// Check if music is enabled
extern bool IsMusicEnabled();
/// Check if music is playing
extern bool IsMusicPlaying();

/// Check if sound is enabled
extern bool SoundEnabled();
/// Initialize the sound card.
extern int InitSound();
///  Cleanup sound.
extern void QuitSound();

extern uint32_t SDL_SOUND_FINISHED;
extern void HandleSoundEvent(SDL_Event &event);

//@}

#endif  // !__SOUND_SERVER_H__
