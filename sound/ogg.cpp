//   ___________		     _________		      _____  __
//   \_	  _____/______   ____   ____ \_   ___ \____________ _/ ____\/  |_
//    |    __) \_  __ \_/ __ \_/ __ \/    \  \/\_  __ \__  \\   __\\   __\ 
//    |     \   |  | \/\  ___/\  ___/\     \____|  | \// __ \|  |   |  |
//    \___  /   |__|    \___  >\___  >\______  /|__|  (____  /__|   |__|
//	  \/		    \/	   \/	     \/		   \/
//  ______________________                           ______________________
//			  T H E   W A R   B E G I N S
//	   FreeCraft - A free fantasy real time strategy game engine
//
/**@name ogg.c			-	ogg support */
//
//	(c) Copyright 2002 by Lutz Sammer
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

#if defined(WITH_SOUND) && defined(USE_OGG)	// {

#include <stdlib.h>
#ifdef BSD
#include <inttypes.h>
#else
#if defined(_MSC_VER) && !defined(_WIN32_WCE)
#include "windows.h"
#endif
#include <stdint.h>
#endif // BSD

#include "vorbis/codec.h"
#include "vorbis/vorbisfile.h"

#include "myendian.h"
#include "iolib.h"
#include "sound_server.h"

/*----------------------------------------------------------------------------
--	Declaration
----------------------------------------------------------------------------*/

typedef struct _ogg_data_ {
    OggVorbis_File vf[1];
    char* p;
} OggData;

#define OGG_BUFFER_SIZE  64*1024

/*----------------------------------------------------------------------------
--	Functions
----------------------------------------------------------------------------*/

/**
**	OGG vorbis read callback.
**
**	@param ptr		Pointer to memory to fill.
**	@param size		Size of the element.
**	@param nmemb		Number of elements to fill.
**	@param user		User argument.
**
**	@return			The number of elements loaded.
*/
local size_t OGG_read(void *ptr, size_t size, size_t nmemb, void *user)
{
    return CLread(user, ptr, size * nmemb) / size;
}

/**
**	OGG vorbis seek callback.
**
**	@param user		User argument.
**	@param offset		Seek offset.
**	@param whence		How to seek.
**
**	@return			Seek position, -1 if failure.
*/
local int OGG_seek(void* user __attribute__((unused)),
	int64_t offset __attribute__((unused)),
	int whence __attribute__((unused)))
{
    return -1;
}

/**
**	OGG vorbis tell callback.
**
**	@param user		User argument.
**
**	@return			Current seek postition.
*/
local long OGG_tell(void* user __attribute__((unused)))
{
    return -1;
}

/**
**	OGG vorbis close callback.
**
**	@param user		User argument.
**
**	@return			Success status.
*/
local int OGG_close(void* user)
{
    return CLclose(user);
}

/**
**	Callback function to read from the ogg file
**
**	@param sample	    Sample reading from
**	@param buf	    Buffer to write data to
**	@param len	    Length of the buffer
**
**	@return		    Number of bytes read
*/
local int ReadOgg(Sample* sample, void* buf, int len)
{
    OggData* data;
    int i;
    int n;
    int bitstream;

    data = (OggData*)sample->User;

    // see if we have enough read already
    if (data->p + len - sample->Data <= sample->Length) {
	memcpy(buf, data->p, len);
	data->p += len;
	return len;
    }

    // not enough in buffer, read more
    n = sample->Length - (data->p - sample->Data);
    memcpy(sample->Data, data->p, n);
    sample->Length = n;
    data->p = sample->Data;
    n = OGG_BUFFER_SIZE - n;
    for (;;) {
	i = ov_read(data->vf, data->p + sample->Length, n, 0, 2, 1, &bitstream);
	if (i <= 0) {
	    break;
	}
	sample->Length += i;
	n -= i;
	if (n <= 4096) {
	    break;
	}
    }
    if (sample->Length < len) {
	len = sample->Length;
    }
    memcpy(buf, data->p, len);
    data->p += len;
    return len;
}

/**
**	Callback function to free an ogg file
**
**	@param sample	    Sample to free
*/
local void FreeOgg(Sample* sample)
{
    OggData* data;

    data = (OggData*)sample->User;
    ov_clear(data->vf);
    free(data);
}

/**
**	Load ogg.
**
**	@param name	File name.
**
**	@return		Returns the loaded sample.
**
**	@todo		FIXME: Should rewrite loop and the sample structure.
*/
global Sample *LoadOgg(const char* name)
{
    CLFile* f;
    Sample* sample;
    OggVorbis_File vf[1];
    unsigned int magic[1];
    vorbis_info* info;
    static const ov_callbacks vc = { OGG_read, OGG_seek, OGG_close, OGG_tell };
    int n;
    char *p;

    if (!(f = CLopen(name))) {
	fprintf(stderr, "Can't open file `%s'\n", name);
	return NULL;
    }
    CLread(f, magic, sizeof(magic));
    if (AccessLE32(magic) != 0x5367674F) {	// "OggS" in ASCII
	CLclose(f);
	return NULL;
    }

    DebugLevel2Fn("Have ogg file %s\n" _C_ name);

    if (ov_open_callbacks(f, vf, (char *)&magic, sizeof(magic), vc)) {
	fprintf(stderr, "Can't initialize ogg decoder\n");
	CLclose(f);
	return NULL;
    }

    info = ov_info(vf, -1);
    if( !info ) {
	fprintf(stderr, "no ogg stream\n");
	CLclose(f);
	return NULL;
    }

    sample = malloc(sizeof(*sample));
    sample->Channels = info->channels;
    sample->SampleSize = 16;
    sample->Frequency = info->rate;
    sample->Type = NULL;
    n = sample->Length = 0;
    p = sample->Data;

    for (;;) {
	int bitstream;
	int i;

	if (n < 8192) {
	    Sample* s;

	    n = 8192 * 64;
	    s = realloc(sample, sizeof(*sample) + sample->Length + n);
	    if (!s) {
		free(sample);
		fprintf(stderr, "out of memory\n");
		ov_clear(vf);
		return NULL;
	    }
	    sample = s;
	    p = sample->Data + sample->Length;
	}

	i = ov_read(vf, p, 8192, 0, 2, 1, &bitstream);
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
    IfDebug( AllocatedSoundMemory += sample->Length; );

    return sample;
}

/**
**	Load ogg streaming.
**
**	@param name	File name.
**
**	@return		Returns the sample.
*/
global Sample *LoadOggStreaming(const char* name)
{
    CLFile* f;
    Sample* sample;
    OggData* data;
    unsigned int magic[1];
    vorbis_info* info;
    static const ov_callbacks vc = { OGG_read, OGG_seek, OGG_close, OGG_tell };

    data = (OggData*)malloc(sizeof(OggData));
    if (!data) {
	fprintf(stderr, "Out of memory\n");
	return NULL;
    }

    if (!(f = CLopen(name))) {
	fprintf(stderr, "Can't open file `%s'\n", name);
	return NULL;
    }
    CLread(f, magic, sizeof(magic));
    if (AccessLE32(magic) != 0x5367674F) {	// "OggS" in ASCII
	CLclose(f);
	return NULL;
    }

    DebugLevel2Fn("Have ogg file %s\n" _C_ name);

    if (ov_open_callbacks(f, data->vf, (char *)&magic, sizeof(magic), vc)) {
	fprintf(stderr, "Can't initialize ogg decoder\n");
	CLclose(f);
	return NULL;
    }

    info = ov_info(data->vf, -1);
    if( !info ) {
	fprintf(stderr, "no ogg stream\n");
	CLclose(f);
	return NULL;
    }

    sample = malloc(sizeof(*sample) + OGG_BUFFER_SIZE);
    if (!sample) {
	fprintf(stderr, "Out of memory\n");
	CLclose(f);
	free(data);
	return NULL;
    }

    sample->Channels = info->channels;
    sample->SampleSize = 16;
    sample->Frequency = info->rate;
    sample->User = data;
    sample->Length = 0;
    sample->Type = (SampleType*)malloc(sizeof(SampleType));
    if (!sample->Type) {
	fprintf(stderr, "Out of memory\n");
	CLclose(f);
	free(data);
	free(sample);
	return NULL;
    }
    sample->Type->Read = &ReadOgg;
    sample->Type->Free = &FreeOgg;
    data->p = sample->Data;

    return sample;
}

#endif	// } WITH_SOUND && USE_OGG

//@}
