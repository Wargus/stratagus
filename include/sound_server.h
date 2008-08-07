//     ____                _       __               
//    / __ )____  _____   | |     / /___ ___________
//   / __  / __ \/ ___/   | | /| / / __ `/ ___/ ___/
//  / /_/ / /_/ (__  )    | |/ |/ / /_/ / /  (__  ) 
// /_____/\____/____/     |__/|__/\__,_/_/  /____/  
//                                              
//       A futuristic real-time strategy game.
//          This file is part of Bos Wars.
//
/**@name sound_server.h - The sound server header file. */
//
//      (c) Copyright 1998-2008 by Lutz Sammer, Fabrice Rossi, and
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
	PlayAudioLoadOnDemand = 8,  /// Load only if needed.
};

/*----------------------------------------------------------------------------
--  Functions
----------------------------------------------------------------------------*/

extern CSample *LoadWav(const std::string &name, int flags);    /// Load a wav file
extern CSample *LoadVorbis(const std::string &name, int flags); /// Load a vorbis file

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
extern int GetEffectsVolume(void);
	/// Set effects enabled
extern void SetEffectsEnabled(bool enabled);
	/// Check if effects are enabled
extern bool IsEffectsEnabled(void);

	/// Set the music finished callback
void SetMusicFinishedCallback(void (*callback)(void));
	/// Play a music file
extern int PlayMusic(CSample *sample);
	/// Play a music file
extern int PlayMusic(const std::string &file);
	/// Stop music playing
extern void StopMusic(void);
	/// Set music volume
extern void SetMusicVolume(int volume);
	/// Get music volume
extern int GetMusicVolume(void);
	/// Set music enabled
extern void SetMusicEnabled(bool enabled);
	/// Check if music is enabled
extern bool IsMusicEnabled(void);
	/// Check if music is playing
extern bool IsMusicPlaying(void);

	/// Check if sound is enabled
extern bool SoundEnabled(void);
	/// Initialize the sound card.
extern int InitSound(void);
	///  Cleanup sound.
extern void QuitSound(void);

//@}

#endif  // !__SOUND_SERVER_H__
