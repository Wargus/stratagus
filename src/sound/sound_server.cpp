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
//      (c) Copyright 1998-2006 by Lutz Sammer, Fabrice Rossi, and
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


//@{

/*----------------------------------------------------------------------------
--  Includes
----------------------------------------------------------------------------*/

#include "stratagus.h"

#include "sound_server.h"

#include "iocompat.h"
#include "iolib.h"

#include "SDL.h"

/*----------------------------------------------------------------------------
--  Variables
----------------------------------------------------------------------------*/

static bool SoundInitialized;    /// is sound initialized
static bool MusicPlaying;        /// flag true if playing music

static int EffectsVolume = 128;  /// effects sound volume
static int MusicVolume = 128;    /// music volume

static bool MusicEnabled = true;
static bool EffectsEnabled = true;

/// Channels for sound effects and unit speech
struct SoundChannel {
	CSample *Sample;       /// sample to play
	Origin *Unit;          /// pointer to unit, who plays the sound, if any
	unsigned char Volume;  /// Volume of this channel
	signed char Stereo;    /// stereo location of sound (-128 left, 0 center, 127 right)

	bool Playing;          /// channel is currently playing
	int Point;             /// point in sample if playing or next free channel

	void (*FinishedCallback)(int channel); /// Callback for when a sample finishes playing
};

#define MaxChannels 32     /// How many channels are supported

static SoundChannel Channels[MaxChannels];
static int NextFreeChannel;

static struct {
	CSample *Sample;       /// Music sample
	void (*FinishedCallback)(); /// Callback for when music finishes playing
} MusicChannel;

static void ChannelFinished(int channel);
static int *MixerBuffer;
static int MixerBufferSize;


/*----------------------------------------------------------------------------
--  Mixers
----------------------------------------------------------------------------*/

/**
**  Convert RAW sound data to 44100 hz, Stereo, 16 bits per channel
**
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
		format = AUDIO_S16SYS;
	}
	SDL_BuildAudioCVT(&acvt, format, channels, frequency, AUDIO_S16SYS, 2, 44100);

	acvt.buf = (Uint8 *)dest;
	memcpy(dest, src, bytes);
	acvt.len = bytes;

	SDL_ConvertAudio(&acvt);

	return acvt.len_mult * bytes;
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
	if (MusicPlaying) {
		Assert(MusicChannel.Sample);

		short *buf = new short[size];
		int len = size * sizeof(short);
		char *tmp = new char[len];

		int div = 176400 / (MusicChannel.Sample->Frequency * (MusicChannel.Sample->SampleSize / 8) * MusicChannel.Sample->Channels);

		size = MusicChannel.Sample->Read(tmp, len / div);

		int n = ConvertToStereo32(tmp, (char *)buf, MusicChannel.Sample->Frequency,
								  MusicChannel.Sample->SampleSize / 8, MusicChannel.Sample->Channels, size);

		for (int i = 0; i < n / (int)sizeof(*buf); ++i) {
			// Add to our samples
			// FIXME: why taking out '/ 2' leads to distortion
			buffer[i] += buf[i] * MusicVolume / MaxVolume / 2;
		}

		delete[] tmp;
		delete[] buf;

		if (n < len) { // End reached
			MusicPlaying = false;
			delete MusicChannel.Sample;
			MusicChannel.Sample = NULL;

			if (MusicChannel.FinishedCallback) {
				MusicChannel.FinishedCallback();
			}
		}
	}
}

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
	static int buf[SOUND_BUFFER_SIZE / 2];
	unsigned char left;
	unsigned char right;

	int div = 176400 / (sample->Frequency * (sample->SampleSize / 8) * sample->Channels);
	int local_volume = (int)volume * EffectsVolume / MaxVolume;

	if (stereo < 0) {
		left = 128;
		right = 128 + stereo;
	} else {
		left = 128 - stereo;
		right = 128;
	}

	Assert(!(index & 1));

	size = std::min((sample->Len - index) * div / 2, size);

	size = ConvertToStereo32((char *)(sample->Buffer + index), (char *)buf, sample->Frequency,
							 sample->SampleSize / 8, sample->Channels,
							 size * 2 / div);

	size /= 2;
	for (int i = 0; i < size; i += 2) {
		// FIXME: why taking out '/ 2' leads to distortion
		buffer[i] += ((short *)buf)[i] * local_volume * left / 128 / MaxVolume / 2;
		buffer[i + 1] += ((short *)buf)[i + 1] * local_volume * right / 128 / MaxVolume / 2;
	}

	return 2 * size / div;
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
	int new_free_channels = 0;

	for (int channel = 0; channel < MaxChannels; ++channel) {
		if (Channels[channel].Playing && Channels[channel].Sample) {
			int i = MixSampleToStereo32(Channels[channel].Sample,
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
	const int *end = mix + size;

	while (mix < end) {
		int s = (*mix++);
		clamp(&s, SHRT_MIN, SHRT_MAX);
		*output++ = s;
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
	if (samples > MixerBufferSize) {
		delete[] MixerBuffer;
		MixerBuffer = new int[samples];
		MixerBufferSize = samples;
	}

	// FIXME: can save the memset here, if first channel sets the values
	memset(MixerBuffer, 0, samples * sizeof(*MixerBuffer));

	if (EffectsEnabled) {
		// Add channels to mixer buffer
		MixChannelsToStereo32(MixerBuffer, samples);
	}
	if (MusicEnabled) {
		// Add music to mixer buffer
		MixMusicToStereo32(MixerBuffer, samples);
	}
	ClipMixToStereo16(MixerBuffer, samples, (short *)buffer);
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
static void FillAudio(void *, Uint8 *stream, int len)
{
	len >>= 1;
	MixIntoBuffer(stream, len);
}

/*----------------------------------------------------------------------------
--  Effects
----------------------------------------------------------------------------*/

/**
**  Check if this sound is already playing
*/
bool SampleIsPlaying(CSample *sample)
{
	for (int i = 0; i < MaxChannels; ++i) {
		if (Channels[i].Sample == sample && Channels[i].Playing) {
			return true;
		}
	}
	return false;
}

bool UnitSoundIsPlaying(Origin *origin)
{
	for (int i = 0; i < MaxChannels; ++i) {
		if (origin && Channels[i].Unit && origin->Id && Channels[i].Unit->Id
			&& origin->Id == Channels[i].Unit->Id && Channels[i].Playing) {
			return true;
		}
	}
	return false;
}

/**
**  A channel is finished playing
*/
static void ChannelFinished(int channel)
{
	if (Channels[channel].FinishedCallback) {
		Channels[channel].FinishedCallback(channel);
	}

	delete Channels[channel].Unit;
	Channels[channel].Unit = NULL;

	Channels[channel].Playing = false;
	Channels[channel].Point = NextFreeChannel;
	NextFreeChannel = channel;
}

/**
**  Put a sound request in the next free channel.
*/
static int FillChannel(CSample *sample, unsigned char volume, char stereo, Origin *origin)
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
	if (origin && origin->Base) {
		Origin *source = new Origin;
		source->Base = origin->Base;
		source->Id = origin->Id;
		Channels[NextFreeChannel].Unit = source;
	}
	NextFreeChannel = next_free;

	return old_free;
}

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

		volume = std::min(MaxVolume, volume);
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

/**
**  Get the sample playing on a channel
*/
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
**  Stop all channels
*/
void StopAllChannels()
{
	SDL_LockAudio();
	for (int i = 0; i < MaxChannels; ++i) {
		if (Channels[i].Playing) {
			ChannelFinished(i);
		}
	}
	SDL_UnlockAudio();
}

static CSample *LoadSample(const char *name, enum _play_audio_flags_ flag)
{
	CSample *sampleWav = LoadWav(name, flag);

	if (sampleWav) {
		return sampleWav;
	}
#ifdef USE_VORBIS
	CSample *sampleVorbis = LoadVorbis(name, flag);
	if (sampleVorbis) {
		return sampleVorbis;
	}
#endif
#ifdef USE_MIKMOD
	CSample *sampleMikMod = LoadMikMod(name, flag);
	if (sampleMikMod) {
		return sampleMikMod;
	}
#endif
	return NULL;
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
CSample *LoadSample(const std::string &name)
{
	const std::string filename = LibraryFileName(name.c_str());
	CSample *sample = LoadSample(filename.c_str(), PlayAudioLoadInMemory);

	if (sample == NULL) {
		fprintf(stderr, "Can't load the sound `%s'\n", name.c_str());
	}
	return sample;
}

/**
**  Play a sound sample
**
**  @param sample  Sample to play
**
**  @return        Channel number, -1 for error
*/
int PlaySample(CSample *sample, Origin *origin)
{
	int channel = -1;

	SDL_LockAudio();
	if (SoundEnabled() && EffectsEnabled && sample && NextFreeChannel != MaxChannels) {
		channel = FillChannel(sample, EffectsVolume, 0, origin);
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
int PlaySoundFile(const std::string &name)
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
**  @param volume  the sound volume 0-255
*/
void SetEffectsVolume(int volume)
{
	clamp(&volume, 0, MaxVolume);
	EffectsVolume = volume;
}

/**
**  Get effects volume
*/
int GetEffectsVolume()
{
	return EffectsVolume;
}

/**
**  Set effects enabled
*/
void SetEffectsEnabled(bool enabled)
{
	EffectsEnabled = enabled;
}

/**
**  Check if effects are enabled
*/
bool IsEffectsEnabled()
{
	return EffectsEnabled;
}

/*----------------------------------------------------------------------------
--  Music
----------------------------------------------------------------------------*/

/**
**  Set the music finished callback
*/
void SetMusicFinishedCallback(void (*callback)())
{
	MusicChannel.FinishedCallback = callback;
}

/**
**  Play a music file.
**
**  @param sample  Music sample.
**
**  @return        0 if music is playing, -1 if not.
*/
int PlayMusic(CSample *sample)
{
	if (sample) {
		StopMusic();
		MusicChannel.Sample = sample;
		MusicPlaying = true;
		return 0;
	} else {
		DebugPrint("Could not play sample\n");
		return -1;
	}
}

/**
**  Play a music file.
**
**  @param file  Name of music file, format is automatically detected.
**
**  @return      0 if music is playing, -1 if not.
*/
int PlayMusic(const std::string &file)
{
	if (!SoundEnabled() || !IsMusicEnabled()) {
		return -1;
	}
	const std::string name = LibraryFileName(file.c_str());
	DebugPrint("play music %s\n" _C_ name.c_str());
	CSample *sample = LoadSample(name.c_str(), PlayAudioStream);

	if (sample) {
		StopMusic();
		MusicChannel.Sample = sample;
		MusicPlaying = true;
		return 0;
	} else {
		DebugPrint("Could not play %s\n" _C_ file.c_str());
		return -1;
	}
}

/**
**  Stop the current playing music.
*/
void StopMusic()
{
	if (MusicPlaying) {
		MusicPlaying = false;
		if (MusicChannel.Sample) {
			SDL_LockAudio();
			delete MusicChannel.Sample;
			MusicChannel.Sample = NULL;
			SDL_UnlockAudio();
		}
	}
}

/**
**  Set the music volume.
**
**  @param volume  the music volume 0-255
*/
void SetMusicVolume(int volume)
{
	clamp(&volume, 0, MaxVolume);
	MusicVolume = volume;
}

/**
**  Get music volume
*/
int GetMusicVolume()
{
	return MusicVolume;
}

/**
**  Set music enabled
*/
void SetMusicEnabled(bool enabled)
{
	if (enabled) {
		MusicEnabled = true;
	} else {
		MusicEnabled = false;
		StopMusic();
	}
}

/**
**  Check if music is enabled
*/
bool IsMusicEnabled()
{
	return MusicEnabled;
}

/**
**  Check if music is playing
*/
bool IsMusicPlaying()
{
	return MusicPlaying;
}

/*----------------------------------------------------------------------------
--  Init
----------------------------------------------------------------------------*/

/**
**  Check if sound is enabled
*/
bool SoundEnabled()
{
	return SoundInitialized;
}

/**
**  Initialize sound card hardware part with SDL.
**
**  @param freq  Sample frequency (44100,22050,11025 hz).
**  @param size  Sample size (8bit, 16bit)
**
**  @return      True if failure, false if everything ok.
*/
static int InitSdlSound(int freq, int size)
{
	SDL_AudioSpec wanted;

	wanted.freq = freq;
	if (size == 8) {
		wanted.format = AUDIO_U8;
	} else if (size == 16) {
		wanted.format = AUDIO_S16SYS;
	} else {
		DebugPrint("Unexpected sample size %d\n" _C_ size);
		wanted.format = AUDIO_S16SYS;
	}
	wanted.channels = 2;
	wanted.samples = 4096;
	wanted.callback = FillAudio;
	wanted.userdata = NULL;

	//  Open the audio device, forcing the desired format
	if (SDL_OpenAudio(&wanted, NULL) < 0) {
		fprintf(stderr, "Couldn't open audio: %s\n", SDL_GetError());
		return -1;
	}
	SDL_PauseAudio(0);
	return 0;
}

/**
**  Initialize sound card.
**
**  @return  True if failure, false if everything ok.
*/
int InitSound()
{
	//
	// Open sound device, 8bit samples, stereo.
	//
	if (InitSdlSound(44100, 16)) {
		SoundInitialized = false;
		return 1;
	}
	SoundInitialized = true;

	// ARI: The following must be done here to allow sound to work in
	// pre-start menus!
	// initialize channels
	for (int i = 0; i < MaxChannels; ++i) {
		Channels[i].Point = i + 1;
	}
	return 0;
}

/**
**  Cleanup sound server.
*/
void QuitSound()
{
	SDL_CloseAudio();
	SoundInitialized = false;
	delete[] MixerBuffer;
	MixerBuffer = NULL;
}

//@}
