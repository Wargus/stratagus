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
/**@name ogg.c			-	ogg support */
//
//	(c) Copyright 2002 by Lutz Sammer
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

#if defined(WITH_SOUND) && defined(USE_OGG)		// {

#include <stdlib.h>
#include <string.h>
#ifdef BSD
#include <inttypes.h>
#else
#ifdef _MSC_VER
#include <windows.h>
#endif
#include <stdint.h>
#endif // BSD

#include <vorbis/codec.h>
#include <vorbis/vorbisfile.h>

#include "myendian.h"
#include "iolib.h"
#include "sound.h"
#include "sound_server.h"
#include "video.h"
#include "avi.h"

/*----------------------------------------------------------------------------
--		Declaration
----------------------------------------------------------------------------*/

/**
**		Private ogg data structure to handle ogg streaming.
*/
typedef struct _ogg_data_ {
	char*				PointerInBuffer;		/// Pointer into buffer
	OggVorbis_File		VorbisFile[1];				/// Vorbis file handle
} OggData;

#define OGG_BUFFER_SIZE  (12 * 1024)				/// Buffer size to fill

/*----------------------------------------------------------------------------
--		Functions
----------------------------------------------------------------------------*/

/**
**		OGG vorbis read callback.
**
**		@param ptr				Pointer to memory to fill.
**		@param size				Size of the element.
**		@param nmemb				Number of elements to fill.
**		@param user				User argument.
**
**		@return						The number of elements loaded.
*/
local size_t OGG_read(void* ptr, size_t size, size_t nmemb, void* user)
{
	return CLread(user, ptr, size * nmemb) / size;
}

/**
**		OGG vorbis seek callback.
**
**		@param user				User argument.
**		@param offset				Seek offset.
**		@param whence				How to seek.
**
**		@return						Seek position, -1 if failure.
*/
local int OGG_seek(void* user __attribute__((unused)),
	int64_t offset __attribute__((unused)),
	int whence __attribute__((unused)))
{
	return -1;
}

/**
**		OGG vorbis tell callback.
**
**		@param user				User argument.
**
**		@return						Current seek postition.
local long OGG_tell(void* user __attribute__((unused)))
{
	return -1;
}
*/

/**
**		OGG vorbis close callback.
**
**		@param user				User argument.
**
**		@return						Success status.
*/
local int OGG_close(void* user)
{
	return CLclose(user);
}

/**
**		Type member function to read from the ogg file
**
**		@param sample			Sample reading from
**		@param buf			Buffer to write data to
**		@param len			Length of the buffer
**
**		@return					Number of bytes read
*/
local int OggReadStream(Sample* sample, void* buf, int len)
{
	OggData* data;
	int i;
	int n;
	int bitstream;

	int freqratio;
	int chanratio;
	int brratio;
	int divide;
	char sndbuf[OGG_BUFFER_SIZE];

	data = (OggData*)sample->User;

	// see if we have enough read already
	if (data->PointerInBuffer - sample->Data + len > sample->Length) {
		// not enough in buffer, read more
		sample->Length -= (data->PointerInBuffer - sample->Data);
		memcpy(sample->Data, data->PointerInBuffer, sample->Length);
		data->PointerInBuffer = sample->Data;

		n = OGG_BUFFER_SIZE - sample->Length;

		freqratio = 44100 / sample->Frequency;
		brratio = 4 / ((sample->SampleSize/8) * sample->Channels);
		chanratio = 2 / sample->Channels;
		divide = freqratio * brratio / chanratio;

		for (;;) {
#ifdef STRATAGUS_BIG_ENDIAN
			i = ov_read(data->VorbisFile, sndbuf, n / divide, 1, 2, 1,
				&bitstream);
#else
			i = ov_read(data->VorbisFile, sndbuf, n / divide, 0, 2, 1,
				&bitstream);
#endif
			if (i <= 0) {
				break;
			}

			i = ConvertToStereo32(sndbuf, &data->PointerInBuffer[sample->Length],
				sample->Frequency, sample->SampleSize / 8, sample->Channels, i);

			sample->Length += i;
			n -= i;
			if (n < 4096) {
				break;
			}
		}
		if (sample->Length < len) {
			len = sample->Length;
		}
	}

	memcpy(buf, data->PointerInBuffer, len);
	data->PointerInBuffer += len;
	return len;
}

/**
**		Type member function to free an ogg file
**
**		@param sample			Sample to free
*/
local void OggFreeStream(Sample* sample)
{
	OggData* data;

#ifdef DEBUG
	AllocatedSoundMemory -= sizeof(*sample) + OGG_BUFFER_SIZE;
#endif

	data = (OggData*)sample->User;
	ov_clear(data->VorbisFile);
	free(data);
	free(sample);
}

/**
**		Ogg object type structure.
*/
local const SampleType OggStreamSampleType = {
	OggReadStream,
	OggFreeStream,
};

/**
**		Type member function to read from the ogg file
**
**		@param sample			Sample reading from
**		@param buf			Buffer to write data to
**		@param len			Length of the buffer
**
**		@return					Number of bytes read
*/
local int OggRead(Sample* sample, void* buf, int len)
{
	int pos;

	pos = (int)sample->User;
	if (pos + len > sample->Length) {				// Not enough data?
		len = sample->Length - pos;
	}
	memcpy(buf, sample->Data + pos, len);

	sample->User = (void*)(pos + len);

	return len;
}

/**
**		Type member function to free an ogg file
**
**		@param sample			Sample to free
*/
local void OggFree(Sample* sample)
{
#ifdef DEBUG
	AllocatedSoundMemory -= sample->Length;
#endif

	free(sample);
}

/**
**		Ogg object type structure.
*/
local const SampleType OggSampleType = {
	OggRead,
	OggFree,
};

/**
**		Load ogg.
**
**		@param name		File name.
**		@param flags		Load flags.
**
**		@return				Returns the loaded sample.
**
**		@todo				Support more flags, LoadOnDemand.
*/
global Sample* LoadOgg(const char* name,int flags)
{
	static const ov_callbacks vc = { OGG_read, OGG_seek, OGG_close, NULL };
	CLFile* f;
	Sample* sample;
	OggVorbis_File vf[1];
	unsigned int magic[1];
	vorbis_info* info;

	if (!(f = CLopen(name,CL_OPEN_READ))) {
		fprintf(stderr, "Can't open file `%s'\n", name);
		return NULL;
	}
	CLread(f, magic, sizeof(magic));
	if (AccessLE32(magic) != 0x5367674F) {		// "OggS" in ASCII
		CLclose(f);
		return NULL;
	}

	DebugLevel2Fn("Loading ogg file: %s\n" _C_ name);

	if (ov_open_callbacks(f, vf, (char*)&magic, sizeof(magic), vc)) {
		fprintf(stderr, "Can't initialize ogg decoder\n");
		CLclose(f);
		return NULL;
	}
	/* JOHNS: ov_test_callbacks didn't worked for me 1.0 RC3
	if (ov_test_open(vf)) {
		ov_clear(vf);
		return NULL;
	}
	*/
	info = ov_info(vf, -1);
	if (!info) {
		fprintf(stderr, "no ogg stream\n");
		ov_clear(vf);
		return NULL;
	}

	//
	//		We have now a correct OGG stream
	//

	sample = malloc(sizeof(*sample) + OGG_BUFFER_SIZE);
	if (!sample) {
		fprintf(stderr, "Out of memory\n");
		ov_clear(vf);
		return NULL;
	}
	sample->Channels = info->channels;
	sample->SampleSize = 16;
	sample->Frequency = info->rate;
	sample->Length = 0;

	if (flags & PlayAudioStream) {
		OggData* data;

		data = malloc(sizeof(OggData));
		if (!data) {
			fprintf(stderr, "Out of memory\n");
			free(sample);
			ov_clear(vf);
			return NULL;
		}
		data->VorbisFile[0] = vf[0];
		data->PointerInBuffer = sample->Data;

		sample->Type = &OggStreamSampleType;
		sample->User = data;

		DebugLevel0Fn(" %d\n" _C_ sizeof(*sample) + OGG_BUFFER_SIZE);
#ifdef DEBUG
		AllocatedSoundMemory += sizeof(*sample) + OGG_BUFFER_SIZE;
#endif
	} else {
		int n;
		char* p;

		sample->Type = &OggSampleType;
		sample->User = 0;

		n = OGG_BUFFER_SIZE;
		p = sample->Data;

		// CLread is not seekable and ov_pcm_total(vf,-1) not supported :(

		for (;;) {
			int bitstream;
			int i;

			if (n < 4096) {
				Sample* s;

				if( sample->Length < 1024 * 1024 ) {
					n = sample->Length << 1;
				} else {
					n = 2 * 1024 * 1024;		// Big junks needed for windows
				}
				s = realloc(sample, sizeof(*sample) + sample->Length + n);
				if (!s) {
					fprintf(stderr, "out of memory\n");
					free(sample);
					ov_clear(vf);
					return NULL;
				}
				sample = s;
				p = sample->Data + sample->Length;
			}

			#ifdef STRATAGUS_BIG_ENDIAN
			i = ov_read(vf, p, 4096, 1, 2, 1, &bitstream);
			#else
			i = ov_read(vf, p, 4096, 0, 2, 1, &bitstream);
			#endif
			if (i <= 0) {
				break;
			}
			p += i;
			sample->Length += i;
			n -= i;
		}
		// Shrink to real size
		sample = realloc(sample, sizeof(*sample) + sample->Length);

		ov_clear(vf);

		DebugLevel0Fn(" %d\n" _C_ sample->Length);
#ifdef DEBUG
		AllocatedSoundMemory += sample->Length;
#endif
	}

	return sample;
}

/*----------------------------------------------------------------------------
--		Avi support
----------------------------------------------------------------------------*/

/**
**		OGG vorbis read callback.
**
**		@param ptr				Pointer to memory to fill.
**		@param size				Size of the element.
**		@param nmemb				Number of elements to fill.
**		@param user				User argument.
**
**		@return						The number of elements loaded.
*/
local size_t AVI_OGG_read(void* ptr, size_t size, size_t nmemb, void* user)
{
	AviFile* avi;
	size_t length;
	unsigned char* frame;

	DebugLevel3Fn("%p: %p %d*%d\n" _C_ user _C_ ptr _C_ size _C_ nmemb);

	avi = user;
	if (avi->AudioRemain) {				// Bytes remaining
		DebugLevel3Fn("Remain %d %d\n" _C_ avi->AudioRemain _C_
				avi->AudioBuffer->Length - avi->AudioRemain);
		length = avi->AudioRemain;
		if (length > nmemb * size) {
			length = nmemb * size;
		}
		memcpy(ptr,
			avi->AudioBuffer->Data + avi->AudioBuffer->Length -
				avi->AudioRemain, length);
		avi->AudioRemain -= length;
		return length / size;
	}

	length = AviReadNextAudioFrame(avi, &frame);
	DebugLevel3Fn("Bytes %d - %d\n" _C_ length _C_ avi->AudioBuffer->Length);
	if ((int)length < 0) {
		return 0;
	}
	if (length > nmemb * size) {
		avi->AudioRemain = length - nmemb * size;
		length = nmemb * size;
	}
	memcpy(ptr, frame, length);

	return length / size;
}

/**
**		OGG vorbis close callback.
**
**		@param user				User argument.
**
**		@return						Success status.
*/
local int AVI_OGG_close(void* user __attribute__((unused)))
{
	return 0;
}

/**
**		Play the ogg stream of an avi movie.
**
**		@param avi		Avi file handle
*/
global void PlayAviOgg(AviFile* avi)
{
	static const ov_callbacks vc = { AVI_OGG_read, OGG_seek, AVI_OGG_close,
		NULL };
	Sample* sample;
	OggVorbis_File vf[1];
	vorbis_info* info;
	OggData* data;

	if (ov_open_callbacks(avi, vf, 0, 0, vc)) {
		fprintf(stderr, "Can't initialize ogg decoder\n");
		return;
	}
	info = ov_info(vf, -1);
	if (!info) {
		fprintf(stderr, "no ogg stream\n");
		ov_clear(vf);
		return;
	}

	//
	//		We have now a correct OGG stream
	//

	sample = malloc(sizeof(*sample) + OGG_BUFFER_SIZE);
	if (!sample) {
		fprintf(stderr, "Out of memory\n");
		ov_clear(vf);
		return;
	}
	sample->Channels = info->channels;
	sample->SampleSize = 16;
	sample->Frequency = info->rate;
	sample->Length = 0;

	data = malloc(sizeof(OggData));
	if (!data) {
		fprintf(stderr, "Out of memory\n");
		free(sample);
		ov_clear(vf);
		return;
	}
	data->VorbisFile[0] = vf[0];
	data->PointerInBuffer = sample->Data;

	sample->Type = &OggStreamSampleType;
	sample->User = data;

	MusicSample = sample;
	PlayingMusic = 1;
}

#endif		// } WITH_SOUND && USE_OGG

//@}
