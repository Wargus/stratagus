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
/**@name mad.c			-	mp3 support with libmad */
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

#if defined(WITH_SOUND) && defined(USE_MAD)	// {

#include <stdlib.h>
#include <string.h>
#include <memory.h>
#include "mad.h"

#include "iolib.h"
#include "sound_server.h"

/*----------------------------------------------------------------------------
--	Declaration
----------------------------------------------------------------------------*/

/**
**	My user data for mad callbacks.
*/
typedef struct _mad_user_ {
    CLFile* File;			// File handle
    Sample* Sample;			// Sample buffer
    unsigned char Buffer[4096];		// Decoded buffer
} MyUser;

/*----------------------------------------------------------------------------
--	Functions
----------------------------------------------------------------------------*/

/**
**	MAD read callback.
**
**	@param user	Our user pointer.
**	@param stream	MP3 stream.
**
**	@return		MAP_FLOW_STOP if eof, MAD_FLOW_CONTINUE otherwise.
*/
local enum mad_flow MAD_read(void *user, struct mad_stream *stream)
{
    int i;
    int l;
    CLFile *f;
    MyUser *u;

    DebugLevel3Fn("Read callback\n");

    u = (MyUser *) user;
    f = u->File;

    l = 0;
    // Copy remaining bytes over
    if (stream->next_frame) {
	memmove(u->Buffer, stream->next_frame,
	    l = &u->Buffer[sizeof(u->Buffer)] - stream->next_frame);
    }

    i = CLread(f, u->Buffer + l, sizeof(u->Buffer) - l);
    //if (!(l + i)) {
    if (!i) {
	return MAD_FLOW_STOP;
    }
    DebugLevel3Fn("%d bytes\n" _C_ l + i);
    mad_stream_buffer(stream, u->Buffer, l + i);

    return MAD_FLOW_CONTINUE;
}

/**
**	This is the output callback function. It is called after each frame of
**	MPEG audio data has been completely decoded. The purpose of this
**	callback is to output the decoded PCM audio.
**
**	@param user	User argument.
**	@param header	MAD header.
**	@param pcm	MAD pcm data struture.
*/
local enum mad_flow MAD_write(void *user,
    struct mad_header const *header __attribute__((unused)),
    struct mad_pcm *pcm)
{
    int i;
    int n;
    int c;
    int channels;
    Sample* sample;
    short* p;

    n=pcm->length;
    channels=pcm->channels;

    DebugLevel3Fn("%d channels %d samples\n" _C_ channels _C_ n);

    sample = ((MyUser *) user)->Sample;

    if( !sample->SampleSize ) {
	sample->Frequency = pcm->samplerate;
	sample->Channels = channels;
	sample->SampleSize = 16;
    }

    i = n * channels * 2;

    ((MyUser *) user)->Sample = sample =
	realloc(sample, sizeof(*sample) + sample->Length + i);
    if (!sample) {
	fprintf(stderr, "Out of memory!\n");
	CLclose(((MyUser *) user)->File);
	ExitFatal(-1);
    }
    p = (short*)(sample->Data + sample->Length);
    sample->Length += i;

    for( i=0; i<n; ++i ) {
	for( c=0; c<channels; ++c ) {
	    mad_fixed_t b;

	    b = pcm->samples[c][i];
	    // round
	    b += (1L << (MAD_F_FRACBITS - 16));
	    // clip
	    if (b >= MAD_F_ONE) {
		b = MAD_F_ONE - 1;
	    } else if (b < -MAD_F_ONE) {
		b = -MAD_F_ONE;
	    }
	    // quantize
	    *p++ = b >> (MAD_F_FRACBITS + 1 - 16);
	}
    }

    return MAD_FLOW_CONTINUE;
}

/**
**	This is the error callback function. It is called whenever a decoding
**	error occurs. The error is indicated by stream->error; the list of
**	possible MAD_ERROR_* errors can be found in the mad.h (or
**	libmad/stream.h) header file.
*/
local enum mad_flow MAD_error(void *user __attribute__((unused)),
    struct mad_stream *stream,
    struct mad_frame *frame __attribute__((unused)))
{
    fprintf(stderr, "decoding error 0x%04x (%s)\n",
	stream->error, mad_stream_errorstr(stream));

    return MAD_FLOW_BREAK;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

#if 0

/**
**	Type member function to read from the ogg file
**
**	@param sample	    Sample reading from
**	@param buf	    Buffer to write data to
**	@param len	    Length of the buffer
**
**	@return		    Number of bytes read
*/
local int Mp3ReadStream(Sample* sample, void* buf, int len)
{
    Mp3Data* data;
    int i;
    int n;
    int bitstream;

    data = (Mp3Data*) sample->User;

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
local void Mp3FreeStream(Sample* sample)
{
    Mp3Data* data;

    IfDebug( AllocatedSoundMemory -= sizeof(*sample) + OGG_BUFFER_SIZE);

    data = (Mp3Data*)sample->User;
    ov_clear(data->VorbisFile);
    free(data);
    free(sample);
}

/**
**	Mp3 object type structure.
*/
local const SampleType Mp3StreamSampleType = {
    Mp3ReadStream,
    Mp3FreeStream,
};
#endif

/**
**	Type member function to read from the ogg file
**
**	@param sample	    Sample reading from
**	@param buf	    Buffer to write data to
**	@param len	    Length of the buffer
**
**	@return		    Number of bytes read
*/
local int Mp3Read(Sample* sample, void* buf, int len)
{
    int pos;

    pos = (int)sample->User;
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
local void Mp3Free(Sample* sample)
{
    IfDebug( AllocatedSoundMemory -= sample->Length; );

    free(sample);
}

/**
**	Mp3 object type structure.
*/
local const SampleType Mp3SampleType = {
    Mp3Read,
    Mp3Free,
};

/**
**	Test code.
*/
static int run_sync(struct mad_decoder *decoder)
{
    enum mad_flow (*error_func) (void *, struct mad_stream *,
	struct mad_frame *);
    void *error_data;
    int bad_last_frame = 0;
    struct mad_stream *stream;
    struct mad_frame *frame;
    struct mad_synth *synth;
    int result = 0;

    error_func = decoder->error_func;
    error_data = decoder->cb_data;

    stream = &decoder->sync->stream;
    frame = &decoder->sync->frame;
    synth = &decoder->sync->synth;

    mad_stream_init(stream);
    mad_frame_init(frame);
    mad_synth_init(synth);

    mad_stream_options(stream, decoder->options);

    do {
	switch (decoder->input_func(decoder->cb_data, stream)) {
	    case MAD_FLOW_STOP:
		goto done;
	    case MAD_FLOW_BREAK:
		goto fail;
	    case MAD_FLOW_IGNORE:
		continue;
	    case MAD_FLOW_CONTINUE:
		break;
	}

	while (1) {
	    if (decoder->header_func) {
		if (mad_header_decode(&frame->header, stream) == -1) {
		    if (!MAD_RECOVERABLE(stream->error))
			break;

		    switch (error_func(error_data, stream, frame)) {
			case MAD_FLOW_STOP:
			    goto done;
			case MAD_FLOW_BREAK:
			    goto fail;
			case MAD_FLOW_IGNORE:
			case MAD_FLOW_CONTINUE:
			default:
			    continue;
		    }
		}

		switch (decoder->header_func(decoder->cb_data, &frame->header)) {
		    case MAD_FLOW_STOP:
			goto done;
		    case MAD_FLOW_BREAK:
			goto fail;
		    case MAD_FLOW_IGNORE:
			continue;
		    case MAD_FLOW_CONTINUE:
			break;
		}
	    }

	    if (mad_frame_decode(frame, stream) == -1) {
		if (!MAD_RECOVERABLE(stream->error))
		    break;

		switch (error_func(error_data, stream, frame)) {
		    case MAD_FLOW_STOP:
			goto done;
		    case MAD_FLOW_BREAK:
			goto fail;
		    case MAD_FLOW_IGNORE:
			break;
		    case MAD_FLOW_CONTINUE:
		    default:
			continue;
		}
	    } else
		bad_last_frame = 0;

	    mad_synth_frame(synth, frame);

	    if (decoder->output_func) {
		switch (decoder->output_func(decoder->cb_data, &frame->header,
			&synth->pcm)) {
		    case MAD_FLOW_STOP:
			goto done;
		    case MAD_FLOW_BREAK:
			goto fail;
		    case MAD_FLOW_IGNORE:
		    case MAD_FLOW_CONTINUE:
			break;
		}
	    }
	}
    } while (stream->error == MAD_ERROR_BUFLEN);

fail:
    result = -1;

done:
    mad_synth_finish(synth);
    mad_frame_finish(frame);
    mad_stream_finish(stream);

    return result;
}

/**
**	Load mp3.
**
**	@param name	File name.
**	@param flags	Load flags.
**
**	@return		Returns the loaded sample.
*/
global Sample *LoadMp3(const char* name, int flags __attribute__((unused)))
{
    MyUser user;
    CLFile* f;
    unsigned char magic[2];
    struct mad_decoder decoder;
    Sample* sample;

    if (!(f = CLopen(name))) {
	fprintf(stderr, "Can't open file `%s'\n", name);
	return NULL;
    }
    CLread(f, magic, sizeof(magic));
    // 0xFF 0xE? for mp3 stream
    if (magic[0] != 0xFF || (magic[1]&0xE0) != 0xE0 ) {
	CLclose(f);
	return NULL;
    }

    // FIXME: ugly way to rewind.
    CLclose(f);
    if (!(f = CLopen(name))) {
	fprintf(stderr, "Can't open file `%s'\n", name);
	return NULL;
    }

    DebugLevel2Fn("Have mp3 file %s\n" _C_ name);

    sample = malloc(sizeof(*sample));
    sample->Channels = 0;
    sample->SampleSize = 0;
    sample->Frequency = 0;
    sample->Length = 0;

    // configure input, output, and error functions
    user.File = f;
    user.Sample = sample;

    mad_decoder_init(&decoder, &user,
	MAD_read, 0 /* header */, 0 /* filter */, MAD_write,
	MAD_error, 0 /* message */);

    // start decoding
    // mad_decoder_run(&decoder, MAD_DECODER_MODE_SYNC);

    decoder.sync = malloc(sizeof(*decoder.sync));
    if (!decoder.sync) {
	mad_decoder_finish(&decoder);
	CLclose(f);
	return NULL;
    }

    run_sync(&decoder);

    free(decoder.sync);
    decoder.sync = 0;

    // release the decoder

    mad_decoder_finish(&decoder);
    CLclose(f);

    user.Sample->Type = &Mp3SampleType;
    user.Sample->User = 0;

    DebugLevel0Fn(" %d\n" _C_ user.Sample->Length);
    IfDebug( AllocatedSoundMemory += user.Sample->Length; );

    return user.Sample;
}

#endif	// } WITH_SOUND && USE_MAD

//@}
