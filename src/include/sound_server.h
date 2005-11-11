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
//      $Id$

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
--  Variables
----------------------------------------------------------------------------*/

	/// sound volume (from 0 to MaxVolume, acts as a multiplier)
extern int GlobalVolume;
	/// music volume (from 0 to MaxVolume, acts as a multiplier)
extern int MusicVolume;

	/// Distance to Volume Mapping
extern int ViewPointOffset;
	/// global range control (max cut off distance for sound)
extern int DistanceSilent;

extern CSample *MusicSample;  /// Music samples

/*----------------------------------------------------------------------------
--  Functions
----------------------------------------------------------------------------*/

extern CSample *LoadFlac(const char *name, int flags);   /// Load a flac file
extern CSample *LoadWav(const char *name, int flags);    /// Load a wav file
extern CSample *LoadVorbis(const char *name, int flags); /// Load a vorbis file
extern CSample *LoadMp3(const char *name, int flags);    /// Load a mp3 file
extern CSample *LoadMikMod(const char *name, int flags); /// Load a module file

extern int ConvertToStereo32(const char *in, char *out, int frequency,
	int bitrate, int channels, int bytes);

	/// Set global volume
extern void SetGlobalVolume(int volume);
	/// Set music volume
extern void SetMusicVolume(int volume);

extern int SetChannelVolume(int channel, int volume);
extern int SetChannelStereo(int channel, int stereo);
extern void SetChannelFinishedCallback(int channel, void (*callback)(int channel));
extern CSample *GetChannelSample(int channel);
extern void StopChannel(int channel);

	/// Load a sample
extern CSample *LoadSample(const char *name);

	/// Play a sample
extern int PlaySample(CSample *sample);
	/// Play a sound file
extern int PlaySoundFile(const char *name);

	/// Initialize the sound card.
extern int InitSound(void);
	/// Initialize the sound card with SDL support.
extern int InitSdlSound(int freq, int size);

	/// Check if sound is enabled
extern int SoundEnabled(void);

	/// Initialize the sound server.
extern int InitSoundServer(void);
	/// Start next song if necessary
extern void PlayListAdvance(void);

	///  Cleanup sound.
extern void QuitSound(void);

//@}

#endif  // !__SOUND_SERVER_H__
