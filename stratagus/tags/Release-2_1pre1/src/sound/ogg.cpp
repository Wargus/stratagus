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
/**@name ogg.c - ogg support */
//
//      (c) Copyright 2002 by Lutz Sammer
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

#include "stratagus.h"

#ifdef USE_OGG		// {

#include <stdlib.h>
#include <stdio.h>
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
--  Declaration
----------------------------------------------------------------------------*/

/**
**  Private ogg data structure to handle ogg streaming.
*/
typedef struct _ogg_data_ {
	OggVorbis_File VorbisFile;  /// Vorbis file handle
} OggData;

/*----------------------------------------------------------------------------
--  Functions
----------------------------------------------------------------------------*/

/**
**  OGG vorbis read callback.
**
**  @param ptr    Pointer to memory to fill.
**  @param size   Size of the element.
**  @param nmemb  Number of elements to fill.
**  @param user   User argument.
**
**  @return       The number of elements loaded.
*/
static size_t OGG_read(void* ptr, size_t size, size_t nmemb, void* user)
{
	return CLread(user, ptr, size * nmemb) / size;
}

/**
**  OGG vorbis seek callback.
**
**  @param user    User argument.
**  @param offset  Seek offset.
**  @param whence  How to seek.
**
**  @return        Seek position, -1 if non-seeking.
*/
static int OGG_seek(void* user, int64_t offset, int whence)
{
	return CLseek(user, offset, whence);
}

/**
**  OGG vorbis close callback.
**
**  @param user  User argument.
**
**  @return      Success status.
*/
static int OGG_close(void* user)
{
	return CLclose(user);
}

static long OGG_tell(void* user)
{
	return CLtell(user);
}

/**
**  Type member function to read from the ogg file
**
**  @param sample  Sample reading from
**  @param buf     Buffer to write data to
**  @param len     Length of the buffer
**
**  @return        Number of bytes read
*/
static int OggStreamRead(Sample* sample, void* buf, int len)
{
	OggData* data;
	int i;
	int n;
	int bitstream;

	data = sample->User;

	if (sample->Pos > SOUND_BUFFER_SIZE / 2) {
		memcpy(sample->Buffer, sample->Buffer + sample->Pos, sample->Len);
		sample->Pos = 0;
	}

	while (sample->Len < SOUND_BUFFER_SIZE / 4) {
		// read more data
		n = SOUND_BUFFER_SIZE - sample->Len;

#ifdef STRATAGUS_BIG_ENDIAN
		i = ov_read(&data->VorbisFile, sample->Buffer + sample->Pos +
			sample->Len, n, 1, 2, 1, &bitstream);
#else
		i = ov_read(&data->VorbisFile, sample->Buffer + sample->Pos +
			sample->Len, n, 0, 2, 1, &bitstream);
#endif
		Assert(i >= 0);

		if (!i) {
			// EOF
			break;
		}

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

/**
**  Type member function to free an ogg file
**
**  @param sample  Sample to free
*/
static void OggStreamFree(Sample* sample)
{
	OggData* data;

	data = sample->User;

	ov_clear(&data->VorbisFile);
	free(data);
	free(sample->Buffer);
	free(sample);
}

/**
**		Ogg stream type structure.
*/
static const SampleType OggStreamSampleType = {
	OggStreamRead,
	OggStreamFree,
};

/**
**  Type member function to read from the ogg file
**
**  @param sample  Sample reading from
**  @param buf     Buffer to write data to
**  @param len     Length of the buffer
**
**  @return        Number of bytes read
*/
static int OggRead(Sample* sample, void* buf, int len)
{
	if (len > sample->Len) {
		len = sample->Len;
	}

	memcpy(buf, sample->Buffer + sample->Pos, len);
	sample->Pos += len;
	sample->Len -= len;

	return len;
}

/**
**  Type member function to free an ogg file
**
**  @param sample  Sample to free
*/
static void OggFree(Sample* sample)
{
	free(sample->User);
	free(sample->Buffer);
	free(sample);
}

/**
**  Ogg object type structure.
*/
static const SampleType OggSampleType = {
	OggRead,
	OggFree,
};

/**
**  Load ogg.
**
**  @param name   File name.
**  @param flags  Load flags.
**
**  @return       Returns the loaded sample.
*/
Sample* LoadOgg(const char* name,int flags)
{
	Sample* sample;
	OggData *data;
	CLFile* f;
	unsigned int magic[1];
	vorbis_info* info;
	static const ov_callbacks vc = { OGG_read, OGG_seek, OGG_close, OGG_tell };

	if (!(f = CLopen(name, CL_OPEN_READ))) {
		fprintf(stderr, "Can't open file `%s'\n", name);
		return NULL;
	}

	CLread(f, magic, sizeof(magic));
	if (AccessLE32(magic) != 0x5367674F) {		// "OggS" in ASCII
		CLclose(f);
		return NULL;
	}

	data = malloc(sizeof(OggData));

	CLseek(f, 0, SEEK_SET);
	if (ov_open_callbacks(f, &data->VorbisFile, NULL, 0, vc)) {
		fprintf(stderr, "Can't initialize ogg decoder\n");
		free(data);
		CLclose(f);
		return NULL;
	}

	info = ov_info(&data->VorbisFile, -1);
	if (!info) {
		fprintf(stderr, "no ogg stream\n");
		free(data);
		ov_clear(&data->VorbisFile);
		return NULL;
	}

	sample = malloc(sizeof(Sample));
	sample->Channels = info->channels;
	sample->SampleSize = 16;
	sample->Frequency = info->rate;
	sample->Len = 0;
	sample->Pos = 0;
	sample->User = data;

	if (flags & PlayAudioStream) {
		sample->Buffer = malloc(SOUND_BUFFER_SIZE);
		sample->Type = &OggStreamSampleType;
	} else {
		int total;
		int i;
		int n;
		int bitstream;
		
		total = ov_pcm_total(&data->VorbisFile, -1) * 2;

		sample->Buffer = malloc(total);
		sample->Type = &OggSampleType;

		while (sample->Len < total) {
			n = total - sample->Len > SOUND_BUFFER_SIZE ? SOUND_BUFFER_SIZE : total - sample->Len;

#ifdef STRATAGUS_BIG_ENDIAN
			i = ov_read(&data->VorbisFile, sample->Buffer + sample->Pos + sample->Len, n, 1, 2, 1,
				&bitstream);
#else
			i = ov_read(&data->VorbisFile, sample->Buffer + sample->Pos + sample->Len, n, 0, 2, 1,
				&bitstream);
#endif
			Assert(i >= 0);

			if (!i) {
				// EOF
				break;
			}

			sample->Len += i;
		}

		Assert(sample->Len == total);
	}

	return sample;
}

/*----------------------------------------------------------------------------
--  Avi support
----------------------------------------------------------------------------*/

/**
**  OGG vorbis read callback.
**
**  @param ptr    Pointer to memory to fill.
**  @param size   Size of the element.
**  @param nmemb  Number of elements to fill.
**  @param user   User argument.
**
**  @return       The number of elements loaded.
*/
static size_t AVI_OGG_read(void* ptr, size_t size, size_t nmemb, void* user)
{
	AviFile* avi;
	size_t length;
	unsigned char* frame;

	avi = user;
	if (avi->AudioRemain) {				// Bytes remaining
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
static int AVI_OGG_close(void* user __attribute__((unused)))
{
	return 0;
}

/**
**		Play the ogg stream of an avi movie.
**
**		@param avi		Avi file handle
*/
void PlayAviOgg(AviFile* avi)
{
	Sample* sample;
	OggData* data;
	vorbis_info* info;
	static const ov_callbacks vc = { AVI_OGG_read, OGG_seek, AVI_OGG_close,	NULL };

	data = malloc(sizeof(OggData));

	if (ov_open_callbacks(avi, &data->VorbisFile, 0, 0, vc)) {
		fprintf(stderr, "Can't initialize ogg decoder\n");
		free(data);
		return;
	}

	info = ov_info(&data->VorbisFile, -1);
	if (!info) {
		fprintf(stderr, "no ogg stream\n");
		ov_clear(&data->VorbisFile);
		free(data);
		return;
	}

	sample = malloc(sizeof(Sample));
	sample->Channels = info->channels;
	sample->SampleSize = 16;
	sample->Frequency = info->rate;
	sample->Buffer = malloc(sizeof(SOUND_BUFFER_SIZE));
	sample->Len = 0;
	sample->Pos = 0;
	sample->Type = &OggStreamSampleType;
	sample->User = data;

	MusicSample = sample;
	PlayingMusic = 1;
}

#endif		// USE_OGG

//@}
