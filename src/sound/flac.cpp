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
/**@name flac.c			-	flac support */
//
//	(c) Copyright 2002 by Lutz Sammer and Fabrice Rossi
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

#if defined(WITH_SOUND) && defined(USE_FLAC)	// {

#include <stdlib.h>
#include "FLAC/stream_decoder.h"

#include "myendian.h"
#include "iolib.h"
#include "sound_server.h"

//
//	Use this if you have still an old flac version.
//
#ifdef FLAC_IDIOTIC

#define FLAC__STREAM_DECODER_READ_STATUS_END_OF_STREAM \
    FLAC__STREAM_DECODER_READ_END_OF_STREAM
#define FLAC__STREAM_DECODER_READ_STATUS_CONTINUE \
    FLAC__STREAM_DECODER_READ_CONTINUE
#define FLAC__StreamMetadata \
    FLAC__StreamMetaData
#define FLAC__STREAM_DECODER_WRITE_STATUS_CONTINUE \
    FLAC__STREAM_DECODER_WRITE_CONTINUE

#endif

/*----------------------------------------------------------------------------
--	Declaration
----------------------------------------------------------------------------*/

/**
**	My user data for flac callbacks.
*/
typedef struct _my_user_ {
    CLFile* File;			// File handle
    Sample* Sample;			// Sample buffer
} MyUser;

/*----------------------------------------------------------------------------
--	Functions
----------------------------------------------------------------------------*/

/**
**	Read callback from FLAC stream decoder.
**
**	@param stream	Decoder stream.
**	@param status	Error state.
**	@param user	User data.
*/
local void FLAC_error_callback(
	const FLAC__StreamDecoder * stream __attribute__((unused)),
	FLAC__StreamDecoderErrorStatus status __attribute__((unused)),
	void *user __attribute__((unused)))
{
    DebugLevel0Fn(" %s\n" _C_ FLAC__StreamDecoderErrorStatusString[status]);
}

/**
**	Read callback from FLAC stream decoder.
**
**	@param stream	Decoder stream.
**	@param buffer	Buffer to be filled.
**	@param bytes	Number of bytes to be filled.
**	@param user	User data.
**
**	@return		Error status.
*/
local FLAC__StreamDecoderReadStatus FLAC_read_callback(
	const FLAC__StreamDecoder * stream __attribute__((unused)),
	FLAC__byte buffer[], unsigned int *bytes, void *user)
{
    int i;
    CLFile *f;

    DebugLevel3Fn("Read callback %d\n" _C_ *bytes);

    f = ((MyUser *) user)->File;

    if ((i = CLread(f, buffer, *bytes)) != *bytes) {
	*bytes = i;
	if (!i) {
	    return FLAC__STREAM_DECODER_READ_STATUS_END_OF_STREAM;
	}
    }
    return FLAC__STREAM_DECODER_READ_STATUS_CONTINUE;
}

/**
**	Write callback from FLAC stream decoder.
**
**	@param stream	Decoder stream.
**	@param metadata	metadata block
**	@param user	User data.
*/
local void FLAC_metadata_callback(
	const FLAC__StreamDecoder * stream __attribute__((unused)),
	const FLAC__StreamMetadata * metadata, void *user)
{
    Sample *sample;

    if (metadata->type == FLAC__METADATA_TYPE_STREAMINFO) {
	sample = ((MyUser *) user)->Sample;

	sample->Channels = metadata->data.stream_info.channels;
	sample->Frequency = metadata->data.stream_info.sample_rate;
	sample->SampleSize = metadata->data.stream_info.bits_per_sample;

	DebugLevel3Fn("Stream %d Channels, %d frequency, %d bits\n" _C_
		sample->Channels _C_ sample->Frequency _C_ sample->SampleSize);
    }
}

/**
**	Write callback from FLAC stream decoder.
**
**	@param stream	Decoder stream.
**	@param frame	Frame to decode.
**	@param buffer	Buffer to be filled.
**	@param user	User data.
**
**	@return		Error status.
*/
local FLAC__StreamDecoderWriteStatus FLAC_write_callback(const
    FLAC__StreamDecoder * decoder __attribute__((unused)),
    const FLAC__Frame * frame, const FLAC__int32 * const buffer[], void *user)
{
    Sample *sample;
    int i;
    int channel;
    void *p;

    DebugLevel3Fn("Write callback %d bits, %d channel, %d bytes\n" _C_
	frame->header.bits_per_sample _C_ frame->header.channels _C_
	frame->header.blocksize);

    sample = ((MyUser *) user)->Sample;
    DebugCheck(frame->header.bits_per_sample != sample->SampleSize);

    i = frame->header.channels * frame->header.blocksize *
	frame->header.bits_per_sample / 8;

    ((MyUser *) user)->Sample = sample =
	realloc(sample, sizeof(*sample) + sample->Length + i);
    if (!sample) {
	fprintf(stderr, "Out of memory!\n");
	CLclose(((MyUser *) user)->File);
	ExitFatal(-1);
    }
    p = sample->Data + sample->Length;
    sample->Length += i;

    switch (sample->SampleSize) {
	case 8:
	    for (i = 0; i < frame->header.blocksize; i++) {
		for (channel = 0; channel < frame->header.channels; channel++) {
		    *((unsigned char *)p)++ = buffer[channel][i] + 128;
		}
	    }
	    break;
	case 16:
	    for (i = 0; i < frame->header.blocksize; i++) {
		for (channel = 0; channel < frame->header.channels; channel++) {
		    *((short *)p)++ = buffer[channel][i];
		}
	    }
	    break;
	default:
	    fprintf(stderr, "Unsupported sample depth!\n");
	    CLclose(((MyUser *) user)->File);
	    ExitFatal(-1);
    }

    return FLAC__STREAM_DECODER_WRITE_STATUS_CONTINUE;
}

/**
**	Load flac.
**
**	@param name	File name.
**
**	@return		Returns the loaded sample.
*/
global Sample* LoadFlac(const char* name)
{
    MyUser user;
    CLFile* f;
    Sample* sample;
    unsigned int magic[1];
    FLAC__StreamDecoder* stream;

    if( !(f=CLopen(name)) ) {
	fprintf(stderr,"Can't open file `%s'\n",name);
	return NULL;
    }
    CLread(f,magic,sizeof(magic));
    if( AccessLE32(magic)!=0x43614C66 ) {	// "fLaC" in ASCII
	CLclose(f);
	return NULL;
    }

    DebugLevel2Fn("Have flac file %s\n" _C_ name);

    // FIXME: ugly way to seek to start of file
    CLclose(f);
    if( !(f=CLopen(name)) ) {
	fprintf(stderr,"Can't open file `%s'\n",name);
	return NULL;
    }

    if( !(stream=FLAC__stream_decoder_new()) ) {
	fprintf(stderr,"Can't initialize flac decoder\n");
	CLclose(f);
	return NULL;
    }

    FLAC__stream_decoder_set_read_callback(stream, FLAC_read_callback);
    FLAC__stream_decoder_set_write_callback(stream, FLAC_write_callback);
    FLAC__stream_decoder_set_metadata_callback(stream, FLAC_metadata_callback);
    FLAC__stream_decoder_set_error_callback(stream, FLAC_error_callback);
    FLAC__stream_decoder_set_client_data(stream, &user);
    FLAC__stream_decoder_init(stream);

    //
    //	Read sample
    //
    user.File=f;
    user.Sample=sample=malloc(sizeof(*sample));
    sample->Channels=0;
    sample->SampleSize=0;
    sample->Frequency=0;
    sample->Length=0;

#if 0
    FLAC__stream_decoder_process_metadata(stream);
    if( !sample->Channels || !sample->SampleSize ) {
	free(sample);
	FLAC__stream_decoder_finish(stream);
	FLAC__stream_decoder_delete(stream);
	CLclose(f);
	return NULL;
    }
#endif

    FLAC__stream_decoder_process_whole_stream(stream);

    FLAC__stream_decoder_finish(stream);
    FLAC__stream_decoder_delete(stream);
    CLclose(f);

    DebugLevel3Fn(" %d\n" _C_ user.Sample->Length);
    IfDebug( AllocatedSoundMemory+=user.Sample->Length; );

    return user.Sample;
}

#endif	// } WITH_SOUND && USE_FLAC

//@}
