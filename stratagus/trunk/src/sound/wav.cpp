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
/**@name wav.c			-	wav support */
//
//	(c) Copyright 2003 by Lutz Sammer, Fabrice Rossi and Nehal Mistry
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

//@{

/*----------------------------------------------------------------------------
--	Includes
----------------------------------------------------------------------------*/

#include <stdio.h>
#include "freecraft.h"

#if defined(WITH_SOUND) // {

#include <stdlib.h>
#include <string.h>

#include "myendian.h"

#include "iolib.h"
#include "sound_server.h"
#include "wav.h"

/*----------------------------------------------------------------------------
--	Declaration
----------------------------------------------------------------------------*/

/** 
**      Private wav data structure to handle wav streaming. 
*/
typedef struct _wav_data_ {
    char* PointerInBuffer;	/// Pointer into buffer
    CLFile* WavFile;		/// Vorbis file handle
} WavData;

#define WAV_BUFFER_SIZE  (12 * 1024)            /// Buffer size to fill 

/*----------------------------------------------------------------------------
--	Functions
----------------------------------------------------------------------------*/

local int WavReadStream(Sample *sample, void *buf, int len)
{
    WavData* data;
    char sndbuf[WAV_BUFFER_SIZE];
    int n;
    int i;
    int x;
    int y;
    int rate;

    data = (WavData*) sample->User;

    if (data->PointerInBuffer - sample->Data + len > sample->Length) {
	// need to read new data
	sample->Length -= data->PointerInBuffer - sample->Data;
	memcpy(sample->Data, data->PointerInBuffer, sample->Length);
	data->PointerInBuffer = sample->Data;

	n = WAV_BUFFER_SIZE - sample->Length;

	rate = 44100 / sample->Frequency;
	i = CLread(data->WavFile, sndbuf, n/rate);

	for (x = 0; x < i*rate; x += 4) {
	    for (y = 0; y < 4; ++y) {
		DebugCheck(x/rate+(y&1) >= i);
		data->PointerInBuffer[sample->Length + x + y] = sndbuf[x/rate+(y&1)];
	    }
	}

	sample->Length += i*rate;

        if (sample->Length < len) {
            len = sample->Length;
        }
    }

    memcpy(buf, data->PointerInBuffer, len);
    data->PointerInBuffer += len;
    return len;
}

local void WavFreeStream(Sample *sample)
{
    WavData* data;
    
    IfDebug( AllocatedSoundMemory -= sizeof(*sample) + WAV_BUFFER_SIZE);
	
    data = (WavData*)sample->User;

    CLclose(data->WavFile);
    free(data);
    free(sample);
}

/** 
**      wav object type structure. 
*/
local const SampleType WavStreamSampleType = {
    WavReadStream,
    WavFreeStream,
};

/**
**	Load wav.
**
**	@param name	File name.
**	@param flags	Load flags.
**
**	@return		Returns the loaded sample.
**
**	@note	A second wav loader is in libmodplug!
**
**	@todo	Add ADPCM loading support!
*/
global Sample* LoadWav(const char* name, int flags __attribute__((unused)))
{
    CLFile* f;
    WavChunk chunk;
    WavFMT wavfmt;
    unsigned int t;
    int i;
    Sample* sample;

    DebugLevel3("Loading `%s'\n" _C_ name);

    if (!(f = CLopen(name))) {
	printf("Can't open file `%s'\n", name);
	return NULL;
    }
    CLread(f, &chunk, sizeof(chunk));

    // Convert to native format

    chunk.Magic = ConvertLE32(chunk.Magic);
    chunk.Length = ConvertLE32(chunk.Length);

    DebugLevel3("Magic: $%x\n" _C_ chunk.Magic);
    DebugLevel3("Length: %d\n" _C_ chunk.Length);
    if (chunk.Magic != RIFF) {
	printf("Wrong magic %x (not %x)\n", chunk.Magic, RIFF);
	CLclose(f);
	return NULL;
    }

    CLread(f, &t, sizeof(t));
    t = ConvertLE32(t);
    DebugLevel3("Magic: $%lx\n" _C_ t);
    if (t != WAVE) {
	printf("Wrong magic %x (not %x)\n", t, WAVE);
	CLclose(f);
	ExitFatal(-1);
    }

    CLread(f, &wavfmt, sizeof(wavfmt));

    // Convert to native format

    wavfmt.FMTchunk = ConvertLE32(wavfmt.FMTchunk);
    wavfmt.FMTlength = ConvertLE32(wavfmt.FMTlength);
    wavfmt.Encoding = ConvertLE16(wavfmt.Encoding);
    wavfmt.Channels = ConvertLE16(wavfmt.Channels);
    wavfmt.Frequency = ConvertLE32(wavfmt.Frequency);
    wavfmt.ByteRate = ConvertLE32(wavfmt.ByteRate);
    wavfmt.SampleSize = ConvertLE16(wavfmt.SampleSize);
    wavfmt.BitsPerSample = ConvertLE16(wavfmt.BitsPerSample);

    DebugLevel3("Magic: $%x\n" _C_ wavfmt.FMTchunk);
    DebugLevel3("Length: %d\n" _C_ wavfmt.FMTlength);
    if (wavfmt.FMTchunk != FMT) {
	printf("Wrong magic %x (not %x)\n", wavfmt.FMTchunk, FMT);
	CLclose(f);
	ExitFatal(-1);
    }
    if (wavfmt.FMTlength != 16 && wavfmt.FMTlength != 18) {
	DebugLevel2("Encoding\t%d\t" _C_ wavfmt.Encoding);
	DebugLevel2("Channels\t%d\t" _C_ wavfmt.Channels);
	DebugLevel2("Frequency\t%d\n" _C_ wavfmt.Frequency);
	DebugLevel2("Byterate\t%d\t" _C_ wavfmt.ByteRate);
	DebugLevel2("SampleSize\t%d\t" _C_ wavfmt.SampleSize);
	DebugLevel2("BitsPerSample\t%d\n" _C_ wavfmt.BitsPerSample);

	printf("Wrong length %d (not %d)\n", wavfmt.FMTlength, 16);
	CLclose(f);
	ExitFatal(-1);
    }

    if (wavfmt.FMTlength == 18) {
	if (CLread(f, &chunk, 2) != 2) {
	    abort();
	}
    }
    DebugLevel3("Encoding\t%d\t" _C_ wavfmt.Encoding);
    DebugLevel3("Channels\t%d\t" _C_ wavfmt.Channels);
    DebugLevel3("Frequency\t%d\n" _C_ wavfmt.Frequency);
    DebugLevel3("Byterate\t%d\t" _C_ wavfmt.ByteRate);
    DebugLevel3("SampleSize\t%d\t" _C_ wavfmt.SampleSize);
    DebugLevel3("BitsPerSample\t%d\n" _C_ wavfmt.BitsPerSample);

    //
    //  Check if supported
    //
    if (wavfmt.Encoding != WAV_PCM_CODE) {
	printf("Unsupported encoding %d\n", wavfmt.Encoding);
	CLclose(f);
	ExitFatal(-1);
    }
    if (wavfmt.Channels != WAV_MONO && wavfmt.Channels != WAV_STEREO) {
	printf("Unsupported channels %d\n", wavfmt.Channels);
	CLclose(f);
	ExitFatal(-1);
    }
    if (wavfmt.SampleSize != 1 && wavfmt.SampleSize != 2 && wavfmt.SampleSize != 4) {
	printf("Unsupported sample size %d\n", wavfmt.SampleSize);
	CLclose(f);
	ExitFatal(-1);
    }
    if (wavfmt.BitsPerSample != 8 && wavfmt.BitsPerSample != 16) {
	printf("Unsupported bits per sample %d\n", wavfmt.BitsPerSample);
	CLclose(f);
	ExitFatal(-1);
    }
    // FIXME: should check it more. Sample frequence

    //
    //  Read sample
    //
    sample = malloc(sizeof(*sample) + WAV_BUFFER_SIZE*wavfmt.Channels);
    sample->Channels = wavfmt.Channels;
    sample->SampleSize = wavfmt.SampleSize * 8;
    sample->Frequency = wavfmt.Frequency;
    sample->Length = 0;


    if (flags & PlayAudioStream) {
	WavData* data;
	data = malloc(sizeof(WavData));

	data->WavFile = f;
	data->PointerInBuffer = sample->Data;

	sample->Type = &WavStreamSampleType;
	sample->User = data;

	CLread(f, &chunk, sizeof(chunk));

	DebugLevel0Fn(" %d\n" _C_ sizeof(*sample) + WAV_BUFFER_SIZE);
	IfDebug( AllocatedSoundMemory += sizeof(*sample) + WAV_BUFFER_SIZE);
    } else {
	for (;;) {
	    if ((i = CLread(f, &chunk, sizeof(chunk))) != sizeof(chunk)) {
		// FIXME: have 1 byte remaining, wrong wav or wrong code?
		// if( i ) printf("Rest: %d\n",i);
	        break;
	    }
	    chunk.Magic = ConvertLE32(chunk.Magic);
	    chunk.Length = ConvertLE32(chunk.Length);

	    DebugLevel3("Magic: $%x\n" _C_ chunk.Magic);
	    DebugLevel3("Length: %d\n" _C_ chunk.Length);
	    if (chunk.Magic != DATA) {
		// FIXME: cleanup the wav files, remove this junk, and don't support
		// FIXME: this!!
		DebugLevel3("Wrong magic %x (not %x)\n" _C_ chunk.Magic _C_ DATA);
		DebugLevel3("Junk at end of file\n");
		break;
	    }

	    i = chunk.Length;
	    sample = realloc(sample, sizeof(*sample) + sample->Length + i);
	    if (!sample) {
		printf("Out of memory!\n");
		CLclose(f);
		ExitFatal(-1);
	    }

	    if (CLread(f, sample->Data + sample->Length, i) != i) {
		printf("Unexpected end of file!\n");
		CLclose(f);
		free(sample);
		ExitFatal(-1);
	    }
	    sample->Length += i;
	}

	CLclose(f);

	IfDebug( AllocatedSoundMemory += sample->Length; );
    }

    return sample;
}

#endif	// } WITH_SOUND

//@}
