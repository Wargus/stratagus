//       _________ __                 __
//      /   _____//  |_____________ _/  |______     ____  __ __  ______
//      \_____  \\   __\_  __ \__  \\   __\__  \   / ___\|  |  \/  ___/
//      /        \|  |  |  | \// __ \|  |  / __ \_/ /_/  >  |  /\___ |
//     /_______  /|__|  |__|  (____  /__| (____  /\___  /|____//____  >
//             \/                  \/          \//_____/            \/
//  ______________________                           ______________________
//			  T H E   W A R   B E G I N S
//	   Stratagus - A free fantasy real time strategy game engine
//
/**@name sound_server.c		-	The sound server
**                                      (hardware layer and so on) */
//
//	(c) Copyright 1998-2004 by Lutz Sammer and Fabrice Rossi
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
//	$Id$

//@{

/*----------------------------------------------------------------------------
--		Includes
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
#include "sound_id.h"
#include "unitsound.h"
#include "tileset.h"
#include "ui.h"
#include "iolib.h"

#include "sound_server.h"
#include "sound.h"
#include "cdaudio.h"
#include "script.h"
#include "etlib/hash.h"

/*----------------------------------------------------------------------------
--		Defines
----------------------------------------------------------------------------*/

//#define SoundSampleSize		8				/// sample size of dsp in bit
#define SoundSampleSize		16				// sample size of dsp in bit
#define SoundFrequency		44100				// sample rate of dsp
//#define SoundFrequency		22050				// sample rate of dsp
//#define SoundFrequency		11025				/// sample rate of dsp
#define SoundDeviceName		"/dev/dsp"		/// dsp device

/*----------------------------------------------------------------------------
--		Variables
----------------------------------------------------------------------------*/

global int SoundFildes = -1;				/// audio file descriptor
global int PlayingMusic;				/// flag true if playing music
global int CallbackMusic;				/// flag true callback ccl if stops
global int WaitForSoundDevice;				/// Block until sound device available

#ifdef DEBUG
global unsigned AllocatedSoundMemory;		/// memory used by sound
#endif

global int GlobalVolume = 128;				/// global sound volume
global int MusicVolume = 128;				/// music volume

global int DistanceSilent;				/// silent distance

// the sound FIFO
global SoundRequest SoundRequests[MAX_SOUND_REQUESTS];
global int NextSoundRequestIn;
global int NextSoundRequestOut;

global int SoundThreadRunning;				/// FIXME: docu

local int MusicTerminated;

SDL_mutex * MusicTerminatedMutex;

/*----------------------------------------------------------------------------
--		Functions
----------------------------------------------------------------------------*/

/**
**		Check if the playlist need to be advanced,
**		and invoque music-stopped if necessary
*/
global void PlayListAdvance(void)
{
	int proceed;

	SDL_LockMutex(MusicTerminatedMutex);
	proceed = MusicTerminated;
	MusicTerminated = 0;
	SDL_UnlockMutex(MusicTerminatedMutex);

	if (proceed) {
		lua_pushstring(Lua, "MusicStopped");
		lua_gettable(Lua, LUA_GLOBALSINDEX);
		LuaCall(0, 1);
	}
}

/**
**		Mix music to stereo 32 bit.
**
**		@param buffer		Buffer for mixed samples.
**		@param size		Number of samples that fits into buffer.
**
**		@todo this functions can be called from inside the SDL audio callback,
**				which is bad, the buffer should be precalculated.
*/
local void MixMusicToStereo32(int* buffer, int size)
{
	int i;
	int n;
	int len;
	short* buf;

	if (PlayingMusic) {
		Assert(MusicSample && MusicSample->Type);

		len = size * sizeof(*buf);
		buf = alloca(len);
		n = MusicSample->Type->Read(MusicSample, buf, len);

		for (i = 0; i < n / (int)sizeof(*buf); ++i) {
			// Add to our samples
			// FIXME: why taking out '/ 2' leads to distortion
			buffer[i] += buf[i] * MusicVolume / MaxVolume / 2;
		}

		if (n < len) {				// End reached
			PlayingMusic = 0;
			SoundFree(MusicSample);
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
--		Functions
----------------------------------------------------------------------------*/

/**
**		Mix sample to buffer.
**
**		The input samples are adjusted by the local volume and resampled
**		to the output frequence.
**
**		@param sample		Input sample
**		@param index		Position into input sample
**		@param volume		Volume of the input sample
**		@param stereo		Stereo (left/right) position of sample
**		@param buffer		Output buffer
**		@param size		Size of the output buffer to be filled
**
**		@return			the number of bytes used to fill buffer
**
**		@todo			Can mix faster if signed 8 bit buffers are used.
**		@todo			Can combine stereo and volume for faster operation.
*/
local int MixSampleToStereo32(Sample* sample,int index,unsigned char volume,
	char stereo, int* buffer, int size)
{
	int local_volume;
	unsigned char left;
	unsigned char right;
	int i;

	local_volume = (int)volume * GlobalVolume / MaxVolume;
	if (local_volume < 32) {
		// minimum volume
		local_volume = 32;
	}

	if (stereo < 0) {
		left = 128;
		right = 128 + stereo;
	} else {
		left = 128 - stereo;
		right = 128;
	}

	DebugLevel3("Length %d\n" _C_ length);

	Assert(!(index & 1));

	if (size >= sample->Len / 2 - index) {
		size = sample->Len / 2 - index;
	}

	for (i = 0; i < size; i += 2) {
		// FIXME: why taking out '/ 2' leads to distortion
		buffer[i] += ((short*)(sample->Buffer))[index + i] * local_volume * left / 128 / MaxVolume / 2;
		buffer[i + 1] += ((short*)(sample->Buffer))[index + i + 1] * local_volume * right / 128 / MaxVolume / 2;
	}

	return size;
}

/**
**		Convert RAW sound data to 44100 hz, Stereo, 16 bits per channel
**
**		// FIXME
**		@param src				Source buffer
**		@param dest				Destination buffer
**		@param frequency		Frequency of source
**		@param chansize				Bitrate in bytes per channel of source
**		@param channels				Number of channels of source
**		@param bytes				Number of compressed bytes to read
**
**		@return				Number of bytes written in 'dest'
*/
global int ConvertToStereo32(const char* src, char* dest, int frequency,
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

	acvt.buf = dest;
	memcpy(dest, src, bytes);
	acvt.len = bytes;

	SDL_ConvertAudio(&acvt);

	return acvt.len_mult * bytes;
}

global SoundChannel Channels[MaxChannels];
global int NextFreeChannel;

/*
** Selection handling
*/
typedef struct _selection_handling_ {
	Origin Source;				// origin of the sound
	ServerSoundId Sound;		// last sound played by this unit
	unsigned char HowMany;		// number of sound played in this group
} SelectionHandling;

/// FIXME: docu
SelectionHandling SelectionHandler;

/**
**		Source registration
*/
/// hash table used to store unit to channel mapping
//local hashtable(int, 61) UnitToChannel;

/**
**		Distance to Volume Mapping
*/
local int ViewPointOffset;

/**
** Number of free channels
*/
local int HowManyFree(void)
{
	int channel;
	int nb;

	nb = 0;
	channel = NextFreeChannel;
	while(channel < MaxChannels) {
		++nb;
		channel = Channels[channel].Point;
	}
	return nb;
}

/**
** Check whether to discard or not a sound request
*/
local int KeepRequest(SoundRequest* sr) {
	//FIXME: take fight flag into account
	int channel;

	if (sr->Sound == NO_SOUND) {
		return 0;
	}

	const SoundChannel* theChannel;

	// slow but working solution: we look for the source in the channels
	theChannel = Channels;
	for (channel = 0; channel < MaxChannels; ++channel) {
		if ((*theChannel).Command == ChannelPlay &&
				(*theChannel).Source.Base == sr->Source.Base &&
				(*theChannel).Sound == sr->Sound &&
				(*theChannel).Source.Id == sr->Source.Id) {
			//FIXME: decision should take into account the sound
			return 0;
		}
		++theChannel;
	}

	return 1;
}

/*
** Register the source of a request in the selection handling hash table.
*/
local void RegisterSource(SoundRequest* sr, int channel)
{
	// always keep the last registered channel
	// use channel+1 to avoid inserting null values
	//FIXME: we should use here the unique identifier of the source, not only
	// the pointer
	int i;
	void* p;

	i = channel;
	p = sr;
	DebugLevel3Fn("FIXME: must write %p -> %d\n" _C_ sr _C_ channel);

	DebugLevel3("Registering %p (channel %d)\n" _C_ sr->Source.Base _C_ channel);
}

/*
** Remove the source of a channel from the selection handling hash table.
*/
local void UnRegisterSource(int channel) {
	int i;

	i = channel;
	DebugLevel3Fn("FIXME: must write %d\n" _C_ channel);

	DebugLevel3("Removing %p (channel %d)\n" _C_
		Channels[channel].Source.Base _C_ channel);
}

/**
**		Compute a suitable volume for something taking place at a given
**		distance from the current view point.
**
**		@param d		distance
**		@param range		range
**
**		@return				volume for given distance (0..??)
*/
local unsigned char VolumeForDistance(unsigned short d, unsigned char range) {
	int d_tmp;
	int range_tmp;

	//FIXME: THIS IS SLOW!!!!!!!
	if (d <= ViewPointOffset || range==INFINITE_SOUND_RANGE) {
		return MaxVolume;
	} else {
		if (range) {
			d -= ViewPointOffset;
			d_tmp = d * MAX_SOUND_RANGE;
			range_tmp = DistanceSilent * range;
			DebugLevel3("Distance: %d, Range: %d\n" _C_ d_tmp _C_ range_tmp);
			if (d_tmp > range_tmp) {
				return 0;
			} else {
				return (unsigned char)((range_tmp - d_tmp) * MAX_SOUND_RANGE / range_tmp);
			}
		} else {
			return 0;
		}
	}
}

/*
** Compute the volume associated with a request, either by clipping the Range
** parameter of this request, or by mapping this range to a volume.
*/
local unsigned char ComputeVolume(SoundRequest* sr) {
	if (sr->IsVolume) {
		if (sr->Power > MaxVolume) {
			return MaxVolume;
		} else {
			return (unsigned char)sr->Power;
		}
	} else {
		// map distance to volume
		return VolumeForDistance(sr->Power, ((ServerSoundId)(sr->Sound))->Range);
	}
}

/*
** "Randomly" choose a sample from a sound group.
*/
local Sample* SimpleChooseSample(ServerSoundId sound) {
	if (sound->Number == ONE_SOUND) {
		return sound->Sound.OneSound;
	} else {
		//FIXME: check for errors
		//FIXME: valid only in shared memory context (FrameCounter)
		return sound->Sound.OneGroup[FrameCounter % sound->Number];
	}
}

/*
** Choose a sample from a SoundRequest. Take into account selection and sound
** groups.
*/
local Sample* ChooseSample(SoundRequest* sr)
{
	ServerSoundId theSound;
	Sample* result;

	result = NO_SOUND;

	if (sr->Sound != NO_SOUND) {
		theSound = (ServerSoundId)(sr->Sound);
		if (theSound->Number == TWO_GROUPS) {
			// handle a special sound (selection)
			if (SelectionHandler.Source.Base == sr->Source.Base
				&& SelectionHandler.Source.Id == sr->Source.Id) {
				if (SelectionHandler.Sound == theSound->Sound.TwoGroups->First) {
					DebugLevel3("First group\n");
					result = SimpleChooseSample(SelectionHandler.Sound);
					SelectionHandler.HowMany++;
					if (SelectionHandler.HowMany >= 3) {
						SelectionHandler.HowMany = 0;
						SelectionHandler.Sound = (ServerSoundId)theSound->Sound.TwoGroups->Second;
						DebugLevel3("Switching to second group\n");
					}
				} else {
					//FIXME: checks for error
					DebugLevel3("Second group\n");
					// check wether the second group is really a group
					if (SelectionHandler.Sound->Number > 1) {
						result = SelectionHandler.Sound->Sound.OneGroup[SelectionHandler.HowMany];
						SelectionHandler.HowMany++;
						if(SelectionHandler.HowMany >= SelectionHandler.Sound->Number) {
							SelectionHandler.HowMany = 0;
							SelectionHandler.Sound = (ServerSoundId)theSound->Sound.TwoGroups->First;
							DebugLevel3("Switching to first group\n");
						}
					} else {
						result = SelectionHandler.Sound->Sound.OneSound;
						SelectionHandler.HowMany = 0;
						SelectionHandler.Sound = (ServerSoundId)theSound->Sound.TwoGroups->First;
						DebugLevel3("Switching to first group\n");
					}
				}
			} else {
				SelectionHandler.Source = sr->Source;
				SelectionHandler.Sound = theSound->Sound.TwoGroups->First;
				result = SimpleChooseSample(SelectionHandler.Sound);
				SelectionHandler.HowMany = 1;
			}
		} else {
			// normal sound/sound group handling
			result = SimpleChooseSample(theSound);
			if (sr->Selection) {
				SelectionHandler.Source = sr->Source;
			}
		}
	}
	return result;
}

/*
** Free a channel and unregister its source
*/
global void FreeOneChannel(int channel)
{
	Channels[channel].Command = ChannelFree;
	Channels[channel].Point = NextFreeChannel;
	NextFreeChannel = channel;
	if (Channels[channel].Source.Base) {
		// unregister only valid sources
		UnRegisterSource(channel);
	}
}

/*
** Put a sound request in the next free channel. While doing this, the
** function computes the volume of the source and chooses a sample.
*/
local int FillOneChannel(SoundRequest* sr)
{
	int next_free;
	int old_free;

	old_free = NextFreeChannel;
	if (NextFreeChannel < MaxChannels) {
		next_free = Channels[NextFreeChannel].Point;
		Channels[NextFreeChannel].Source = sr->Source;
		Channels[NextFreeChannel].Point = 0;
		Channels[NextFreeChannel].Volume = ComputeVolume(sr);
		Channels[NextFreeChannel].Command = ChannelPlay;
		Channels[NextFreeChannel].Sound = sr->Sound;
		Channels[NextFreeChannel].Sample = ChooseSample(sr);
		Channels[NextFreeChannel].Stereo = sr->Stereo;
		NextFreeChannel = next_free;
	} else {
		// should not happen
		DebugLevel0("***** NO FREE CHANNEL *****\n");
	}
	return old_free;
}

/**
** get orders from the fifo and put them into channels. This function takes
** care of registering sound sources.
//FIXME: is this the correct place to do this?
*/
local void FillChannels(int free_channels,int* discarded,int* started)
{
	int channel;
	SoundRequest* sr;

	sr = SoundRequests+NextSoundRequestOut;
	*discarded = 0;
	*started = 0;
	while (free_channels && sr->Used) {
		if (KeepRequest(sr)) {
			DebugLevel3("Source [%p]: start playing request %p at slot %d\n" _C_
				sr->Source.Base _C_ sr->Sound _C_ NextSoundRequestOut);
			channel = FillOneChannel(sr);
			if (sr->Source.Base) {
				//Register only sound with a valid origin
				RegisterSource(sr, channel);
			}
			--free_channels;
			DebugLevel3("Free channels: %d\n" _C_ free_channels);
			sr->Used = 0;
			++NextSoundRequestOut;
			(*started)++;
		} else {
		  // Discarding request (for whatever reason)
		  DebugLevel3("Discarding resquest %p from %p at slot %d\n" _C_
			  sr->Sound _C_ sr->Source.Base _C_ NextSoundRequestOut);
		  sr->Used = 0;
		  ++NextSoundRequestOut;
		  (*discarded)++;
		}
		if(NextSoundRequestOut >= MAX_SOUND_REQUESTS) {
			NextSoundRequestOut = 0;
		}
		sr = SoundRequests + NextSoundRequestOut;
	}
}

/**
**		Mix channels to stereo 32 bit.
**
**		@param buffer		Buffer for mixed samples.
**		@param size		Number of samples that fits into buffer.
**
**		@return				How many channels become free after mixing them.
*/
local int MixChannelsToStereo32(int* buffer,int size)
{
	int channel;
	int i;
	int new_free_channels;

	new_free_channels = 0;
	for (channel = 0; channel < MaxChannels; ++channel) {
		if (Channels[channel].Command == ChannelPlay &&
				Channels[channel].Sample) {
			i = MixSampleToStereo32(Channels[channel].Sample,
				Channels[channel].Point, Channels[channel].Volume,
				Channels[channel].Stereo, buffer, size);
			Channels[channel].Point += i;

//			if (Channels[channel].Point >= Channels[channel].Sample->Length){
			if (Channels[channel].Point >= Channels[channel].Sample->Len / 2) {
				// free channel as soon as possible (before playing)
				// useful in multithreading
				DebugLevel3("End playing request from %p\n" _C_
					Channels[channel].Source.Base);
				FreeOneChannel(channel);
				++new_free_channels;
			}
		}
	}

	return new_free_channels;
}

#if SoundSampleSize == 8
/**
**		Clip mix to output stereo 8 unsigned bit.
**
**		@param mix		signed 32 bit input.
**		@param size		number of samples in input.
**		@param output		clipped 8 unsigned bit output buffer.
*/
local void ClipMixToStereo8(const int* mix, int size, unsigned char* output)
{
	int s;

	while (size--) {
		s = (*mix++) / 256;
		if (s > 127) {
			*output++ = 255;
		} else if (s < -127) {
			*output++ = 0;
		} else {
			*output++ = s + 127;
		}
	}
}
#endif

#if SoundSampleSize == 16
/**
**		Clip mix to output stereo 16 signed bit.
**
**		@param mix		signed 32 bit input.
**		@param size		number of samples in input.
**		@param output		clipped 16 signed bit output buffer.
*/
local void ClipMixToStereo16(const int* mix, int size, short* output)
{
	int s;
	const int* end;

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
#endif

/*----------------------------------------------------------------------------
--		Other
----------------------------------------------------------------------------*/

/**
**		Load one sample
**
**		@param name		File name of sample (short version).
**
**		@return				General sample loaded from file into memory.
**
**		@todo 				Add streaming, cashing support.
*/
local Sample* LoadSample(const char* name)
{
	Sample* sample;
	char* buf;

	// FIXME: find a better way to detect sound not in sound dir
	if (strstr(name, "campaign")) {
		buf = strdcat3(StratagusLibPath, "/", name);
	} else {
		buf = strdcat3(StratagusLibPath, "/sounds/", name);
	}

	if ((sample = LoadWav(buf, PlayAudioLoadInMemory))) {
		free(buf);
		return sample;
	}
#ifdef USE_OGG
	if ((sample = LoadOgg(buf, PlayAudioLoadInMemory))) {
		free(buf);
		return sample;
	}
#endif
#ifdef USE_FLAC
	if ((sample = LoadFlac(buf, PlayAudioLoadInMemory))) {
		free(buf);
		return sample;
	}
#endif
#ifdef USE_MAD
	if ((sample = LoadMp3(buf, PlayAudioLoadInMemory))) {
		free(buf);
		return sample;
	}
#endif
#ifdef USE_MIKMOD
	if ((sample = LoadMikMod(buf, PlayAudioLoadInMemory))) {
		free(buf);
		return sample;
	}
#endif

	fprintf(stderr, "Can't load the sound `%s'\n", name);

	// FIXME: should support more sample formats.
	free(buf);
	return sample;
}

/**
**		Ask the sound server to register a sound (and currently to load it)
**		and to return an unique identifier for it. The unique identifier is
**		memory pointer of the server.
**
**		@param files		An array of wav files.
**		@param number		Number of files belonging together.
**
**		@return				the sound unique identifier
**
**		@todo		FIXME: Must handle the errors better.
**				FIXME: Support for more sample files (ogg/flac/mp3).
*/
global SoundId RegisterSound(const char* files[], unsigned number)
{
	unsigned i;
	ServerSoundId id;

	id = malloc(sizeof(*id));
	if (number > 1) {						// load a sound group
		id->Sound.OneGroup = malloc(sizeof(Sample*) * number);
		for (i = 0; i < number; ++i) {
			DebugLevel3("Registering `%s'\n" _C_ files[i]);
			id->Sound.OneGroup[i] = LoadSample(files[i]);
			if (!id->Sound.OneGroup[i]) {
				free(id->Sound.OneGroup);
				free(id);
				return NO_SOUND;
			}
		}
		id->Number = number;
	} else {								// load an unique sound
		DebugLevel3("Registering `%s'\n" _C_ files[0]);
		id->Sound.OneSound = LoadSample(files[0]);
		if (!id->Sound.OneSound) {
			free(id);
			return NO_SOUND;
		}
		id->Number = ONE_SOUND;
	}
	id->Range = MAX_SOUND_RANGE;
	return (SoundId)id;
}

/**
** 		Ask the sound server to put together two sounds to form a special sound.
**
**		@param first		first part of the group
**		@param second		second part of the group
**
**		@return				the special sound unique identifier
*/
global SoundId RegisterTwoGroups(SoundId first, SoundId second)
{
	ServerSoundId id;

	if (first == NO_SOUND || second == NO_SOUND) {
		return NO_SOUND;
	}
	id = (ServerSoundId)malloc(sizeof(*id));
	id->Number = TWO_GROUPS;
	id->Sound.TwoGroups = (TwoGroups*)malloc(sizeof(TwoGroups));
	id->Sound.TwoGroups->First = first;
	id->Sound.TwoGroups->Second = second;
	id->Range = MAX_SOUND_RANGE;

	return (SoundId) id;
}

/**
**		Ask the sound server to change the range of a sound.
**
**		@param sound		the id of the sound to modify.
**		@param range		the new range for this sound.
*/
global void SetSoundRange(SoundId sound, unsigned char range)
{
	if (sound != NO_SOUND) {
		((ServerSoundId) sound)->Range = range;
		DebugLevel3("Setting sound <%p> to range %u\n" _C_ sound _C_ range);
	}
}

/**
**		Mix into buffer.
**
**		@param buffer		Buffer to be filled with samples. Buffer must be big
**						enough.
**		@param samples		Number of samples.
*/
global void MixIntoBuffer(void* buffer, int samples)
{
	int* mixer_buffer;
	int free_channels;
	int dummy1;
	int dummy2;

	free_channels = HowManyFree();
	FillChannels(free_channels, &dummy1, &dummy2);

	// Create empty mixer buffer
	mixer_buffer = alloca(samples * sizeof(*mixer_buffer));
	// FIXME: can save the memset here, if first channel sets the values
	memset(mixer_buffer, 0, samples * sizeof(*mixer_buffer));

	// Add channels to mixer buffer
	MixChannelsToStereo32(mixer_buffer, samples);
	// Add music to mixer buffer
	MixMusicToStereo32(mixer_buffer, samples);

#if SoundSampleSize == 8
	ClipMixToStereo8(mixer_buffer, samples, buffer);
#endif
#if SoundSampleSize == 16
	ClipMixToStereo16(mixer_buffer, samples, buffer);
#endif
}

/**
**		Fill buffer for the sound card.
**
**		@see SDL_OpenAudio
**
**		@param udata		the pointer stored in userdata field of SDL_AudioSpec.
**		@param stream		pointer to buffer you want to fill with information.
**		@param len		is length of audio buffer in bytes.
*/
global void FillAudio(void* udata __attribute__((unused)), Uint8* stream, int len)
{
#if SoundSampleSize == 16
	len >>= 1;
#endif
	DebugLevel3Fn("%d\n" _C_ len);

	MixIntoBuffer(stream, len);
}

/**
**		Initialize sound card.
*/
global int InitSound(void)
{
	int dummy;

	MusicTerminated = 0;
	MusicTerminatedMutex = SDL_CreateMutex();

	//
	//		Open sound device, 8bit samples, stereo.
	//
	if (InitSdlSound(SoundDeviceName, SoundFrequency, SoundSampleSize,
			WaitForSoundDevice)) {
		return 1;
	}

	//		ARI:		The following must be done here to allow sound to work in
	//				pre-start menus!
	// initialize channels
	for (dummy = 0; dummy < MaxChannels; ++dummy) {
		Channels[dummy].Point = dummy + 1;
	}

	// initialize unit to channel hash table
	// WARNING: creation is only valid for a hash table using pointers as key
	DebugLevel0Fn("FIXME: must write non GLIB hash functions\n");

	return 0;
}

/**
**		Initialize sound server structures (and thread)
*/
global int InitSoundServer(void)
{
	int MapWidth;
	int MapHeight;

	MapWidth = (TheUI.MapArea.EndX - TheUI.MapArea.X + TileSizeX) / TileSizeX;
	MapHeight = (TheUI.MapArea.EndY - TheUI.MapArea.Y + TileSizeY) / TileSizeY;
	//FIXME: Valid only in shared memory context!
	DistanceSilent = 3 * max(MapWidth, MapHeight);
	DebugLevel2("Distance Silent: %d\n" _C_ DistanceSilent);
	ViewPointOffset = max(MapWidth / 2, MapHeight / 2);
	DebugLevel2("ViewPointOffset: %d\n" _C_ ViewPointOffset);

	return 0;
}

/**
**		Cleanup sound server.
*/
global void QuitSound(void)
{
	SDL_CloseAudio();
	SoundFildes = -1;
}

//@}
