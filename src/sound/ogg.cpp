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
#include "video.h"

/*----------------------------------------------------------------------------
--	Declaration
----------------------------------------------------------------------------*/

/**
**	Private ogg data structure to handle ogg streaming.
*/
typedef struct _ogg_data_ {
    char*		PointerInBuffer;	/// Pointer into buffer
    OggVorbis_File	VorbisFile[1];		/// Vorbis file handle
} OggData;

#define OGG_BUFFER_SIZE  (12 * 1024)		/// Buffer size to fill

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
local long OGG_tell(void* user __attribute__((unused)))
{
    return -1;
}
*/

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
**	Type member function to read from the ogg file
**
**	@param sample	    Sample reading from
**	@param buf	    Buffer to write data to
**	@param len	    Length of the buffer
**
**	@return		    Number of bytes read
*/
local int OggReadStream(Sample* sample, void* buf, int len)
{
    OggData* data;
    int i;
    int n;
    int bitstream;

    data = (OggData*) sample->User;

    // see if we have enough read already
    if (data->PointerInBuffer + len - sample->Data > sample->Length) {
	// not enough in buffer, read more
	n = sample->Length - (data->PointerInBuffer - sample->Data);
	memcpy(sample->Data, data->PointerInBuffer, n);
	sample->Length = n;
	data->PointerInBuffer = sample->Data;

	n = OGG_BUFFER_SIZE - n;
	for (;;) {
	    i = ov_read(data->VorbisFile,
		    data->PointerInBuffer + sample->Length, n, 0, 2, 1,
		    &bitstream);
	    if (i <= 0) {
		break;
	    }
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
**	Type member function to free an ogg file
**
**	@param sample	    Sample to free
*/
local void OggFreeStream(Sample* sample)
{
    OggData* data;

    IfDebug( AllocatedSoundMemory -= sizeof(*sample) + OGG_BUFFER_SIZE);

    data = (OggData*)sample->User;
    ov_clear(data->VorbisFile);
    free(data);
    free(sample);
}

/**
**	Ogg object type structure.
*/
local const SampleType OggStreamSampleType = {
    OggReadStream,
    OggFreeStream,
};

/**
**	Type member function to read from the ogg file
**
**	@param sample	    Sample reading from
**	@param buf	    Buffer to write data to
**	@param len	    Length of the buffer
**
**	@return		    Number of bytes read
*/
local int OggRead(Sample* sample, void *buf, int len)
{
    unsigned pos;

    pos = (unsigned)sample->User;
    if (pos + len > sample->Length) {		// Not enough data?
	len = sample->Length - pos;
    }
    memcpy(buf, sample->Data + pos, len);

    sample->User = (void*)(pos + len);

    return len;
}

/**
**	Type member function to free an ogg file
**
**	@param sample	    Sample to free
*/
local void OggFree(Sample* sample)
{
    IfDebug( AllocatedSoundMemory -= sample->Length; );

    free(sample);
}

/**
**	Ogg object type structure.
*/
local const SampleType OggSampleType = {
    OggRead,
    OggFree,
};

/**
**	Load ogg.
**
**	@param name	File name.
**	@param flags	Load flags.
**
**	@return		Returns the loaded sample.
**
**	@todo		Support more flags, LoadOnDemand.
*/
global Sample* LoadOgg(const char* name,int flags)
{
    static const ov_callbacks vc = { OGG_read, OGG_seek, OGG_close, NULL };
    CLFile* f;
    Sample* sample;
    OggVorbis_File vf[1];
    unsigned int magic[1];
    vorbis_info* info;

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
    /* JOHNS: ov_test_callbacks didn't worked for me 1.0 RC3
    if (ov_test_open(vf)) {
	ov_clear(vf);
	return NULL;
    }
    */
    info = ov_info(vf, -1);
    if( !info ) {
	fprintf(stderr, "no ogg stream\n");
	ov_clear(vf);
	return NULL;
    }

    //
    //	We have now a correct OGG stream
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

    if (flags&PlayAudioStream&0) {
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
	IfDebug( AllocatedSoundMemory += sizeof(*sample) + OGG_BUFFER_SIZE);
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

		if( sample->Length < 1024*1024 ) {
		    n = sample->Length << 1;
		} else {
		    n = 2 * 1024 * 1024;	// Big junks needed for windows
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

	    i = ov_read(vf, p, 4096, 0, 2, 1, &bitstream);
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
    }

    return sample;
}

#endif	// } WITH_SOUND && USE_OGG

//@}
