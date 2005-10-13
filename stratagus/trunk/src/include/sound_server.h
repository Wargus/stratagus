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
--  Includes
----------------------------------------------------------------------------*/

#include "sound_id.h"

/*----------------------------------------------------------------------------
--  Definitons
----------------------------------------------------------------------------*/

	/**
	**  Maximal volume value
	*/
#define MaxVolume 255

#define SOUND_BUFFER_SIZE 65536

class CSound;

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
**  Sound double group: a sound that groups two sounds, used to implement
**  the annoyed/selected sound system of WC
*/
typedef struct _two_groups_ {
	CSound *First;                /// first group: selected sound
	CSound *Second;               /// second group: annoyed sound
} TwoGroups;

/**
** A possible value for Number in the Sound struct: means a simple sound
*/
#define ONE_SOUND 0
/**
** A possible value for Number in the Sound struct: means a double group (for
** selection/annoyed sounds)
*/
#define TWO_GROUPS 1

/**
** the range value that makes a sound volume distance independent
*/
#define INFINITE_SOUND_RANGE 255
/**
** the maximum range value
*/
#define MAX_SOUND_RANGE 254

/**
**  Sound definition.
*/
class CSound {
public:
	/**
	**  Range is a multiplier for ::DistanceSilent.
	**  255 means infinite range of the sound.
	*/
	unsigned char Range;        /// Range is a multiplier for DistanceSilent
	unsigned char Number;       /// single, group, or table of sounds.
	union {
		CSample *OneSound;       /// if it's only a simple sound
		CSample **OneGroup;      /// when it's a simple group
		struct _two_groups_* TwoGroups; /// when it's a double group
	} Sound;
};

/**
**  Origin of a sound
*/
typedef struct _origin_ {
	const void *Base;   /// pointer on a Unit
	unsigned Id;        /// unique identifier (if the pointer has been shared)
} Origin;

/**
**  sound request FIFO
*/
typedef struct _sound_request {
	Origin Source;          /// origin of sound
	unsigned short Power;   /// Volume or Distance
	CSound *Sound;          /// which sound
	unsigned Used : 1;      /// flag for used/unused
	unsigned Selection : 1; /// is it a selection sound?
	unsigned IsVolume : 1;  /// how to interpret power (as a
							///volume or as a distance?)
	char Stereo;            /// stereo location of sound (
							///-128 left, 0 center, 127 right)
} SoundRequest;

#define MAX_SOUND_REQUESTS 64  /// maximal number of sound requests

#define MaxChannels 32  /// How many channels are supported

	/// Channels for sound effects and unit speach
typedef struct _sound_channel_
{
	unsigned char Command; /// channel command
	int Point;             /// point into sample
	CSample *Sample;       /// sample to play
	Origin Source;         /// unit playing
	unsigned char Volume;  /// Volume of this channel
	CSound *Sound;         /// The sound currently played
	signed char Stereo;    /// stereo location of sound (-128 left, 0 center, 127 right)
} SoundChannel;

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

	/// global range control (max cut off distance for sound)
extern int DistanceSilent;

	/// FIFO for sound requests
extern SoundRequest SoundRequests[MAX_SOUND_REQUESTS];
	/// FIFO index in
extern int NextSoundRequestIn;
	/// FIFO index out
extern int NextSoundRequestOut;

#define ChannelFree 0   /// channel is free
#define ChannelPlay 3   /// channel is playing

	/// All possible sound channels
extern SoundChannel Channels[MaxChannels];
	/// Next free channel
extern int NextFreeChannel;

#ifdef DEBUG
	/// allocated memory for sound samples
extern unsigned AllocatedSoundMemory;
#endif

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

	/// Register a sound (can be a simple sound or a group)
extern CSound *RegisterSound(const char *file[], unsigned number);

	/**
	**  @brief Create a special sound group with two sounds
	**
	**  Ask the sound server to put together two sounds to form a special sound.
	**  @param first    first part of the group
	**  @param second   second part of the group
	**  @return         the special sound unique identifier
	*/
extern CSound *RegisterTwoGroups(CSound *first, CSound *second);

	/// Modify the range of a given sound.
extern void SetSoundRange(CSound *sound, unsigned char range);

	/// Free a channel and unregister its source
extern void FreeOneChannel(int channel);

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

/** Ask the sound layer to write the content of its buffer to the sound
**  device. To be used only in the unthreaded version.
*/
extern void WriteSound(void);

	///  Cleanup sound.
extern void QuitSound(void);

//@}

#endif  // !__SOUND_SERVER_H__
