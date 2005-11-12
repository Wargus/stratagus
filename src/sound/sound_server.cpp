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
/**@name sound_server.cpp - The sound server (hardware layer and so on) */
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


//@{

/*----------------------------------------------------------------------------
--  Includes
----------------------------------------------------------------------------*/

#include <stdio.h>
#include "stratagus.h"

#include <stdlib.h>
#include <string.h>
#ifndef _MSC_VER
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#endif
#include <limits.h>

#include "SDL.h"

#include "video.h"
#include "sound.h"
#include "sound_server.h"
#include "unitsound.h"
#include "tileset.h"
#include "ui.h"
#include "iolib.h"
#include "iocompat.h"
#include "cdaudio.h"
#include "script.h"

#include "util.h"

/*----------------------------------------------------------------------------
--  Defines
----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
--  Variables
----------------------------------------------------------------------------*/

static int SoundInitialized;     /// audio file descriptor
int PlayingMusic;                /// flag true if playing music
int CallbackMusic;               /// flag true callback ccl if stops

int GlobalVolume = 128;          /// global sound volume
int MusicVolume = 128;           /// music volume

static int MusicTerminated;

SDL_mutex *MusicTerminatedMutex;


	/// Channels for sound effects and unit speach
struct SoundChannel {
	CSample *Sample;       /// sample to play
	unsigned char Volume;  /// Volume of this channel
	signed char Stereo;    /// stereo location of sound (-128 left, 0 center, 127 right)

	bool Playing;          /// channel is currently playing
	int Point;             /// point in sample if playing or next free channel

	void (*FinishedCallback)(int channel); /// Callback for when a sample finishes playing
};

#define MaxChannels 32     /// How many channels are supported

SoundChannel Channels[MaxChannels];
int NextFreeChannel;

/*----------------------------------------------------------------------------
--  Functions
----------------------------------------------------------------------------*/

/**
**  Check if the playlist need to be advanced,
**  and invoke music-stopped if necessary
*/
void PlayListAdvance(void)
{
	int proceed;

	SDL_LockMutex(MusicTerminatedMutex);
	proceed = MusicTerminated;
	MusicTerminated = 0;
	SDL_UnlockMutex(MusicTerminatedMutex);

	if (proceed) {
		lua_pushstring(Lua, "MusicStopped");
		lua_gettable(Lua, LUA_GLOBALSINDEX);
		if (!lua_isfunction(Lua, -1)) {
			fprintf(stderr, "No MusicStopped function in Lua\n");
			MusicOff = 1;
			StopMusic();
		} else {
			LuaCall(0, 1);
		}
	}
}

/**
**  Mix music to stereo 32 bit.
**
**  @param buffer  Buffer for mixed samples.
**  @param size    Number of samples that fits into buffer.
**
**  @todo this functions can be called from inside the SDL audio callback,
**  which is bad, the buffer should be precalculated.
*/
static void MixMusicToStereo32(int *buffer, int size)
{
	int i;
	int n;
	int len;
	short *buf;
	char *tmp;
	int div;

	if (PlayingMusic) {
		Assert(MusicSample);

		len = size * sizeof(*buf);
		tmp = new char[len];
		buf = new short[len];

		div = 176400 / (MusicSample->Frequency * (MusicSample->SampleSize / 8)
				* MusicSample->Channels);

		size = MusicSample->Read(tmp, len / div);

		n = ConvertToStereo32((char *)(tmp), (char *)buf, MusicSample->Frequency,
			MusicSample->SampleSize / 8, MusicSample->Channels, size);

		for (i = 0; i < n / (int)sizeof(*buf); ++i) {
			// Add to our samples
			// FIXME: why taking out '/ 2' leads to distortion
			buffer[i] += buf[i] * MusicVolume / MaxVolume / 2;
		}

		delete[] tmp;
		delete[] buf;

		if (n < len) { // End reached
			PlayingMusic = 0;
			delete MusicSample;
			MusicSample = NULL;

			// we are inside the SDL callback!
			if (CallbackMusic) {
				SDL_LockMutex(MusicTerminatedMutex);
				MusicTerminated = 1;
				SDL_UnlockMutex(MusicTerminatedMutex);
			}
		}
	}
}

/*----------------------------------------------------------------------------
-- Functions
----------------------------------------------------------------------------*/

/**
**  Mix sample to buffer.
**
**  The input samples are adjusted by the local volume and resampled
**  to the output frequence.
**
**  @param sample  Input sample
**  @param index   Position into input sample
**  @param volume  Volume of the input sample
**  @param stereo  Stereo (left/right) position of sample
**  @param buffer  Output buffer
**  @param size    Size of output buffer (in samples per channel)
**
**  @return        The number of bytes used to fill buffer
**
**  @todo          Can mix faster if signed 8 bit buffers are used.
*/
static int MixSampleToStereo32(CSample *sample, int index, unsigned char volume,
	char stereo, int *buffer, int size)
{
	int local_volume;
	unsigned char left;
	unsigned char right;
	int i;
	static int buf[SOUND_BUFFER_SIZE/2];
	int div;

	div = 176400 / (sample->Frequency * (sample->SampleSize / 8)
			* sample->Channels);

	local_volume = (int)volume * GlobalVolume / MaxVolume;

	if (stereo < 0) {
		left = 128;
		right = 128 + stereo;
	} else {
		left = 128 - stereo;
		right = 128;
	}

	Assert(!(index & 1));

	if (size >= (sample->Len - index) * div / 2) {
		size = (sample->Len - index) * div / 2;
	}

	size = ConvertToStereo32((char *)(sample->Buffer + index), (char *)buf, sample->Frequency,
			sample->SampleSize / 8, sample->Channels,
			size * 2 / div);

	size /= 2;
	for (i = 0; i < size; i += 2) {
		// FIXME: why taking out '/ 2' leads to distortion
		buffer[i] += ((short *)buf)[i] * local_volume * left / 128 / MaxVolume / 2;
		buffer[i + 1] += ((short *)buf)[i + 1] * local_volume * right / 128 / MaxVolume / 2;
	}

	return 2 * size / div;
}

/**
**  Convert RAW sound data to 44100 hz, Stereo, 16 bits per channel
**
**  // FIXME
**  @param src        Source buffer
**  @param dest       Destination buffer
**  @param frequency  Frequency of source
**  @param chansize   Bitrate in bytes per channel of source
**  @param channels   Number of channels of source
**  @param bytes      Number of compressed bytes to read
**
**  @return           Number of bytes written in 'dest'
*/
static int ConvertToStereo32(const char *src, char *dest, int frequency,
	int chansize, int channels, int bytes)
{
	SDL_AudioCVT acvt;
	Uint16 format;

	if (chansize == 1) {
		format = AUDIO_U8;
	} else {
		format = AUDIO_S16;
	}
	SDL_BuildAudioCVT(&acvt, format, channels, frequency, AUDIO_S16,
		2, 44100);

	acvt.buf = (unsigned char *)dest;
	memcpy(dest, src, bytes);
	acvt.len = bytes;

	SDL_ConvertAudio(&acvt);

	return acvt.len_mult * bytes;
}

/**
**  Number of free channels
*/
static int NumFreeChannels(void)
{
	int num = 0;
	int channel = NextFreeChannel;

	while (channel < MaxChannels) {
		++num;
		channel = Channels[channel].Point;
	}

	return num;
}

/**
**  Put a sound request in the next free channel.
*/
static int FillChannel(CSample *sample, unsigned char volume, char stereo)
{
	Assert(NextFreeChannel < MaxChannels);

	int old_free = NextFreeChannel;
	int next_free = Channels[NextFreeChannel].Point;

	Channels[NextFreeChannel].Volume = volume;
	Channels[NextFreeChannel].Point = 0;
	Channels[NextFreeChannel].Playing = true;
	Channels[NextFreeChannel].Sample = sample;
	Channels[NextFreeChannel].Stereo = stereo;
	Channels[NextFreeChannel].FinishedCallback = NULL;

	NextFreeChannel = next_free;

	return old_free;
}

/**
**  A channel is finished playing
*/
static void ChannelFinished(int channel)
{
	if (Channels[channel].FinishedCallback) {
		Channels[channel].FinishedCallback(channel);
	}

	Channels[channel].Playing = false;
	Channels[channel].Point = NextFreeChannel;
	NextFreeChannel = channel;
}

/**
**  Mix channels to stereo 32 bit.
**
**  @param buffer  Buffer for mixed samples.
**  @param size    Number of samples that fits into buffer.
**
**  @return        How many channels become free after mixing them.
*/
static int MixChannelsToStereo32(int *buffer, int size)
{
	int channel;
	int i;
	int new_free_channels;

	new_free_channels = 0;
	for (channel = 0; channel < MaxChannels; ++channel) {
		if (Channels[channel].Playing && Channels[channel].Sample) {
			i = MixSampleToStereo32(Channels[channel].Sample,
				Channels[channel].Point, Channels[channel].Volume,
				Channels[channel].Stereo, buffer, size);
			Channels[channel].Point += i;
			Assert(Channels[channel].Point <= Channels[channel].Sample->Len);

			if (Channels[channel].Point == Channels[channel].Sample->Len) {
				ChannelFinished(channel);
				++new_free_channels;
			}
		}
	}

	return new_free_channels;
}

/**
**  Clip mix to output stereo 16 signed bit.
**
**  @param mix     signed 32 bit input.
**  @param size    number of samples in input.
**  @param output  clipped 16 signed bit output buffer.
*/
static void ClipMixToStereo16(const int *mix, int size, short *output)
{
	int s;
	const int *end;

	end = mix + size;
	while (mix < end) {
		s = (*mix++);
		if (s > SHRT_MAX) {
			*output++ = SHRT_MAX;
		} else if (s < SHRT_MIN) {
			*output++ = SHRT_MIN;
		} else {
			*output++ = s;
		}
	}
}

/*----------------------------------------------------------------------------
--  Other
----------------------------------------------------------------------------*/

/**
**  Set the channel volume
**
**  @param channel  Channel to set
**  @param volume   New volume, <0 will not set the volume
**
**  @return         Current volume of the channel, -1 for error
*/
int SetChannelVolume(int channel, int volume)
{
	if (channel < 0 || channel >= MaxChannels) {
		return -1;
	}

	if (volume < 0) {
		volume = Channels[channel].Volume;
	} else {
		SDL_LockAudio();

		if (volume > MaxVolume) {
			volume = MaxVolume;
		}
		Channels[channel].Volume = volume;

		SDL_UnlockAudio();
	}

	return volume;
}

/**
**  Set the channel stereo
**
**  @param channel  Channel to set
**  @param stereo   -128 to 127, out of range will not set the stereo
**
**  @return         Current stereo of the channel, -1 for error
*/
int SetChannelStereo(int channel, int stereo)
{
	if (channel < 0 || channel >= MaxChannels) {
		return -1;
	}

	if (stereo < -128 || stereo > 127) {
		stereo = Channels[channel].Stereo;
	} else {
		SDL_LockAudio();

		if (stereo > 127) {
			stereo = 127;
		} else if (stereo < -128) {
			stereo = -128;
		}
		Channels[channel].Stereo = stereo;

		SDL_UnlockAudio();
	}

	return stereo;
}

/**
**  Set the channel's callback for when a sound finishes playing
**
**  @param channel   Channel to set
**  @param callback  Callback to call when the sound finishes
*/
void SetChannelFinishedCallback(int channel, void (*callback)(int channel))
{
	if (channel < 0 || channel >= MaxChannels) {
		return;
	}

	Channels[channel].FinishedCallback = callback;
}

CSample *GetChannelSample(int channel)
{
	if (channel < 0 || channel >= MaxChannels) {
		return NULL;
	}

	return Channels[channel].Sample;
}

/**
**  Stop a channel
**
**  @param channel  Channel to stop
*/
void StopChannel(int channel)
{
	SDL_LockAudio();

	if (channel >= 0 && channel < MaxChannels) {
		if (Channels[channel].Playing) {
			ChannelFinished(channel);
		}
	}

	SDL_UnlockAudio();
}

/**
**  Load a sample
**
**  @param name  File name of sample (short version).
**
**  @return      General sample loaded from file into memory.
**
**  @todo  Add streaming, caching support.
*/
CSample *LoadSample(const char *name)
{
	CSample *sample;
	char buf[PATH_MAX];

	LibraryFileName(name, buf);

	if ((sample = LoadWav(buf, PlayAudioLoadInMemory))) {
		return sample;
	}
#ifdef USE_VORBIS
	if ((sample = LoadVorbis(buf, PlayAudioLoadInMemory))) {
		return sample;
	}
#endif
#ifdef USE_FLAC
	if ((sample = LoadFlac(buf, PlayAudioLoadInMemory))) {
		return sample;
	}
#endif
#ifdef USE_MAD
	if ((sample = LoadMp3(buf, PlayAudioLoadInMemory))) {
		return sample;
	}
#endif
#ifdef USE_MIKMOD
	if ((sample = LoadMikMod(buf, PlayAudioLoadInMemory))) {
		return sample;
	}
#endif

	fprintf(stderr, "Can't load the sound `%s'\n", name);

	return sample;
}

/**
**  Play a sound sample
**
**  @param sample  Sample to play
**
**  @return        Channel number, -1 for error
*/
int PlaySample(CSample *sample)
{
	int channel = -1;

	SDL_LockAudio();

	if (!SoundOff && sample && NextFreeChannel != MaxChannels) {
		channel = FillChannel(sample, GlobalVolume, 0);
	}

	SDL_UnlockAudio();

	return channel;
}

/**
**  Play a sound file
**
**  @param name  Filename of a sound to play
**
**  @return      Channel number the sound is playing on, -1 for error
*/
int PlaySoundFile(const char *name)
{
	CSample *sample = LoadSample(name);
	if (sample) {
		return PlaySample(sample);
	}
	return -1;
}


/**
**  Set the global sound volume.
**
**  @param volume  the sound volume (positive number) 0-255
*/
void SetGlobalVolume(int volume)
{
	if (volume < 0) {
		GlobalVolume = 0;
	} else if (volume > MaxVolume) {
		GlobalVolume = MaxVolume;
	} else {
		GlobalVolume = volume;
	}
}

/**
**  Set the music volume.
**
**  @param volume  the music volume (positive number) 0-255
*/
void SetMusicVolume(int volume)
{
	if (volume < 0) {
		MusicVolume = 0;
	} else if (volume > MaxVolume) {
		MusicVolume = MaxVolume;
	} else {
		MusicVolume = volume;
	}
}


/**
**  Mix into buffer.
**
**  @param buffer   Buffer to be filled with samples. Buffer must be big enough.
**  @param samples  Number of samples.
*/
static void MixIntoBuffer(void *buffer, int samples)
{
	int *mixer_buffer;

	// Create empty mixer buffer
	mixer_buffer = new int[samples];
	// FIXME: can save the memset here, if first channel sets the values
	memset(mixer_buffer, 0, samples * sizeof(*mixer_buffer));

	// Add channels to mixer buffer
	MixChannelsToStereo32(mixer_buffer, samples);
	// Add music to mixer buffer
	MixMusicToStereo32(mixer_buffer, samples);

	ClipMixToStereo16(mixer_buffer, samples, (short *)buffer);

	delete[] mixer_buffer;
}

/**
**  Fill buffer for the sound card.
**
**  @see SDL_OpenAudio
**
**  @param udata   the pointer stored in userdata field of SDL_AudioSpec.
**  @param stream  pointer to buffer you want to fill with information.
**  @param len     is length of audio buffer in bytes.
*/
void FillAudio(void *udata, Uint8 *stream, int len)
{
	if (SoundOff) {
		return;
	}
	len >>= 1;
	MixIntoBuffer(stream, len);
}

/**
**  Initialize sound card.
*/
int InitSound(void)
{
	int dummy;

	MusicTerminated = 0;
	MusicTerminatedMutex = SDL_CreateMutex();

	//
	// Open sound device, 8bit samples, stereo.
	//
	if (InitSdlSound(44100, 16)) {
		SoundInitialized = 0;
		return 1;
	}
	SoundInitialized = 1;

	// ARI: The following must be done here to allow sound to work in
	// pre-start menus!
	// initialize channels
	for (dummy = 0; dummy < MaxChannels; ++dummy) {
		Channels[dummy].Point = dummy + 1;
	}

	return 0;
}

/**
**  Check if sound is enabled
*/
int SoundEnabled(void)
{
	return SoundInitialized;
}

/**
**  Initialize sound server structures (and thread)
*/
int InitSoundServer(void)
{
	int MapWidth;
	int MapHeight;

	MapWidth = (UI.MapArea.EndX - UI.MapArea.X + TileSizeX) / TileSizeX;
	MapHeight = (UI.MapArea.EndY - UI.MapArea.Y + TileSizeY) / TileSizeY;
	// FIXME: Valid only in shared memory context!
	DistanceSilent = 3 * ((MapWidth > MapHeight) ? MapWidth : MapHeight);
	ViewPointOffset = ((MapWidth / 2 > MapHeight / 2) ? MapWidth / 2 : MapHeight / 2);

	return 0;
}

/**
**  Cleanup sound server.
*/
void QuitSound(void)
{
	SDL_CloseAudio();
	SoundInitialized = 0;
}

//@}
