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

    mad_decoder_run(&decoder, MAD_DECODER_MODE_SYNC);

    // release the decoder

    mad_decoder_finish(&decoder);

    CLclose(f);

    return user.Sample;
}

#endif	// } WITH_SOUND && USE_MAD

//@}
