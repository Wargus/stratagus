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
/**@name wav.c			-	wav support */
//
//	(c) Copyright 2003-2004 by Lutz Sammer, Fabrice Rossi and Nehal Mistry
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

#include "stratagus.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "myendian.h"

#include "iolib.h"
#include "sound_server.h"
#include "wav.h"

/*----------------------------------------------------------------------------
--  Declaration
----------------------------------------------------------------------------*/

/**
**  Private wav data structure to handle wav streaming.
*/
typedef struct _wav_data_ {
	CLFile* WavFile;				/// Vorbis file handle
} WavData;

/*----------------------------------------------------------------------------
--  Functions
----------------------------------------------------------------------------*/

local int WavStreamRead(Sample* sample, void* buf, int len)
{
	WavData* data;
	WavChunk chunk;
	char sndbuf[SOUND_BUFFER_SIZE];
	int comp;		// number of compressed bytes actually read
	int divide;
	int i;
	int n;

	data = sample->User;

	if (sample->Pos > SOUND_BUFFER_SIZE / 2) {
		memcpy(sample->Buffer, sample->Buffer + sample->Pos, sample->Len);
		sample->Pos = 0;
	}

	divide = 176400 / (sample->Frequency * (sample->SampleSize/8) * sample->Channels);

	while (sample->Len < SOUND_BUFFER_SIZE / 4) {
		// read more data
		comp = CLread(data->WavFile, &chunk, sizeof(chunk));

		if (!comp) {
			// EOF
			break;
		}

		chunk.Magic = ConvertLE32(chunk.Magic);
		chunk.Length = ConvertLE32(chunk.Length);
		if (chunk.Magic != DATA) {
			CLseek(data->WavFile, chunk.Length, SEEK_CUR);
			continue;
		}
		n = chunk.Length;

		comp = CLread(data->WavFile, sndbuf, n);

		if (!comp) {
			break;
		}

		if (sample->BitsPerSample == 16) {
			for (i = 0; i < n >> 1; ++i) {
				((unsigned short*)sndbuf)[i] = ConvertLE16(((unsigned short*)sndbuf)[i]);
			}
		}

		i = ConvertToStereo32(sndbuf, sample->Buffer + sample->Pos,
			sample->Frequency, sample->SampleSize / 8,
			sample->Channels, comp);

		sample->Len += i;
	}

        if (sample->Len < len) {
                len = sample->Len;
        }

	memcpy(buf, sample->Buffer + sample->Pos, len);
	sample->Pos += len;
	sample->Len -= len;

	return len;
}

local void WavStreamFree(Sample* sample)
{
	WavData* data;

	data = sample->User;

	CLclose(data->WavFile);
	free(data);
	free(sample);
}

/**
**  wav stream type structure.
*/
local const SampleType WavStreamSampleType = {
	WavStreamRead,
	WavStreamFree,
};

local int WavRead(Sample* sample, void* buf, int len)
{
        if (len > sample->Len) {
                len = sample->Len;
        }

        memcpy(buf, sample->Buffer + sample->Pos, len);
        sample->Pos += len;
        sample->Len -= len;

        return len;
}

local void WavFree(Sample* sample)
{
        free(sample->User);
        free(sample->Buffer);
        free(sample);
}

/**
**	  wav stream type structure.
*/
local const SampleType WavSampleType = {
	WavRead,
	WavFree,
};


/**
**  Load wav.
**
**  @param name   File name.
**  @param flags  Load flags.
**
**  @return       Returns the loaded sample.
**
**  @todo         Add ADPCM loading support!
*/
global Sample* LoadWav(const char* name, int flags)
{
	Sample* sample;
	WavData* data;
	CLFile* f;
	WavChunk chunk;
	WavFMT wavfmt;
	unsigned int t;

	if (!(f = CLopen(name,CL_OPEN_READ))) {
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
		CLclose(f);
		return NULL;
	}

	DebugLevel3Fn("Loading wav file: %s\n" _C_ name);

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
	DebugCheck(wavfmt.Frequency != 44100 && wavfmt.Frequency != 22050 &&
		wavfmt.Frequency != 11025);

	data = malloc(sizeof(WavData));
	data->WavFile = f;

	//
	//  Read sample
	//
	sample = malloc(sizeof(Sample));
	sample->Channels = wavfmt.Channels;
	sample->SampleSize = wavfmt.SampleSize * 8 / sample->Channels;
	sample->Frequency = wavfmt.Frequency;
	sample->BitsPerSample = wavfmt.BitsPerSample;
	sample->Len = 0;
	sample->Pos = 0;
	sample->User = data;

	if (flags & PlayAudioStream) {
		sample->Buffer = malloc(SOUND_BUFFER_SIZE);
		sample->Type = &WavStreamSampleType;
	} else {
		int comp;		// number of compressed bytes actually read
		int divide;
		int i;
		int n;
		char sndbuf[SOUND_BUFFER_SIZE];

		sample->Type = &WavSampleType;

		divide = 176400 / (sample->Frequency * (sample->SampleSize/8) * sample->Channels);

		sample->Buffer = NULL;
		while (1) {
			// read more data
			comp = CLread(f, &chunk, sizeof(chunk));

			if (!comp) {
				// EOF
				break;
			}

			chunk.Magic = ConvertLE32(chunk.Magic);
			chunk.Length = ConvertLE32(chunk.Length);
			if (chunk.Magic != DATA) {
				CLseek(f, chunk.Length, SEEK_CUR);
				continue;
			}
			n = chunk.Length;

			sample->Buffer = realloc(sample->Buffer, sample->Len + n * divide);
			DebugCheck(!sample->Buffer);

			comp = CLread(data->WavFile, sndbuf, n);

			if (!comp) {
				break;
			}

			if (sample->BitsPerSample == 16) {
				for (i = 0; i < n >> 1; ++i) {
					((unsigned short*)sndbuf)[i] = ConvertLE16(((unsigned short*)sndbuf)[i]);
				}
			}

			i = ConvertToStereo32(sndbuf, sample->Buffer + sample->Pos,
				sample->Frequency, sample->SampleSize / 8,
				sample->Channels, comp);

			sample->Len += i;
		}
	}

	return sample;
}

//@}
