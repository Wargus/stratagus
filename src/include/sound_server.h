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
--  Definitons
----------------------------------------------------------------------------*/

#define MaxVolume 255
#define SOUND_BUFFER_SIZE 65536

/**
**  RAW samples.
*/
class CSample {
public:
	CSample() : Channels(0), SampleSize(0), Frequency(0), BitsPerSample(0),
		Buffer(NULL), Pos(0), Len(0) {}
	virtual ~CSample() {}

	virtual int Read(void *buf, int len) = 0;

	unsigned char Channels;       /// mono or stereo
	unsigned char SampleSize;     /// sample size in bits
	unsigned int Frequency;       /// frequency in hz
	unsigned short BitsPerSample; /// bits in a sample 8/16/32

	unsigned char *Buffer;        /// sample buffer
	int Pos;                      /// buffer position
	int Len;                      /// length of filled buffer
};

/**
**  Play audio flags.
*/
enum _play_audio_flags_ {
	PlayAudioStream = 1,        /// Stream the file from medium
	PlayAudioPreLoad = 2,       /// Load compressed in memory
	PlayAudioLoadInMemory = 4,  /// Preload file into memory
	PlayAudioLoadOnDemand = 8   /// Load only if needed.
};

/*----------------------------------------------------------------------------
--  Functions
----------------------------------------------------------------------------*/

extern CSample *LoadWav(const char *name, int flags);    /// Load a wav file
extern CSample *LoadVorbis(const char *name, int flags); /// Load a vorbis file
extern CSample *LoadMikMod(const char *name, int flags); /// Load a module file

	/// Set the channel volume
extern int SetChannelVolume(int channel, int volume);
	/// Set the channel stereo
extern int SetChannelStereo(int channel, int stereo);
	/// Set the channel's callback for when a sound finishes playing
extern void SetChannelFinishedCallback(int channel, void (*callback)(int channel));
	/// Get the sample playing on a channel
extern CSample *GetChannelSample(int channel);
	/// Stop a channel
extern void StopChannel(int channel);
	/// Stop all channels
extern void StopAllChannels();

	/// Load a sample
extern CSample *LoadSample(const std::string &name);
	/// Play a sample
extern int PlaySample(CSample *sample);
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
extern int PlayMusic(CSample *sample);
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

//@}

#endif  // !__SOUND_SERVER_H__
