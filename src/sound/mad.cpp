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
/**@name mad.c			-	mp3 support with libmad */
//
//	(c) Copyright 2002-2003 by Lutz Sammer
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

//	FIXME: JOHNS: MP3 streaming did not yet work.

//@{

/*----------------------------------------------------------------------------
--		Includes
----------------------------------------------------------------------------*/

#include <stdio.h>
#include "stratagus.h"

#ifdef USE_MAD		// {

#include <stdlib.h>
#include <string.h>
#include <memory.h>
#include "mad.h"

#include "iolib.h"
#include "sound_server.h"

/*----------------------------------------------------------------------------
--		Declaration
----------------------------------------------------------------------------*/

/**
**		My user data for mad callbacks.
*/
typedef struct _mad_user_ {
	CLFile* File;						// File handle
	Sample* Sample;						// Sample buffer
	unsigned char Buffer[4096];				// Decoded buffer
} MyUser;

/**
**		Private mp3 data structure to handle mp3 streaming.
*/
typedef struct _mp3_data_ {
	char*				PointerInBuffer;		/// Pointer into buffer
	struct mad_decoder		Decoder[1];				/// Mad decoder handle
	MyUser				User[1];				/// Decoder user data
} Mp3Data;

#define MP3_BUFFER_SIZE  (12 * 1024)				/// Buffer size to fill

/*----------------------------------------------------------------------------
--		Functions
----------------------------------------------------------------------------*/

/**
**		MAD read callback.
**
**		@param user		Our user pointer.
**		@param stream		MP3 stream.
**
**		@return				MAP_FLOW_STOP if eof, MAD_FLOW_CONTINUE otherwise.
*/
local enum mad_flow MAD_read(void* user, struct mad_stream* stream)
{
	int i;
	int l;
	CLFile* f;
	MyUser* u;

	DebugLevel3Fn("Read callback\n");

	u = (MyUser*) user;
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
**		This is the output callback function. It is called after each frame of
**		MPEG audio data has been completely decoded. The purpose of this
**		callback is to output the decoded PCM audio.
**
**		@param user		User argument.
**		@param header		MAD header.
**		@param pcm		MAD pcm data struture.
*/
local enum mad_flow MAD_write(void* user,
	struct mad_header const* header __attribute__((unused)),
	struct mad_pcm* pcm)
{
	int i;
	int n;
	int c;
	int channels;
	Sample* sample;
	short* p;

	n = pcm->length;
	channels = pcm->channels;

	DebugLevel3Fn("%d channels %d samples\n" _C_ channels _C_ n);

	sample = ((MyUser*)user)->Sample;

	if (!sample->SampleSize) {
		sample->Frequency = pcm->samplerate;
		sample->Channels = channels;
		sample->SampleSize = 16;
	}

	i = n * channels * 2;

	((MyUser*)user)->Sample = sample =
		realloc(sample, sizeof(*sample) + sample->Length + i);
	if (!sample) {
		fprintf(stderr, "Out of memory!\n");
		CLclose(((MyUser*) user)->File);
		ExitFatal(-1);
	}
	p = (short*)(sample->Data + sample->Length);
	sample->Length += i;

	for (i = 0; i < n; ++i) {
		for (c = 0; c < channels; ++c) {
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
**		This is the error callback function. It is called whenever a decoding
**		error occurs. The error is indicated by stream->error; the list of
**		possible MAD_ERROR_* errors can be found in the mad.h (or
**		libmad/stream.h) header file.
*/
local enum mad_flow MAD_error(void* user __attribute__((unused)),
	struct mad_stream* stream,
	struct mad_frame* frame __attribute__((unused)))
{
	fprintf(stderr, "decoding error 0x%04x (%s)\n",
		stream->error, mad_stream_errorstr(stream));

	return MAD_FLOW_BREAK;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

/**
**		Read one frame from mad decoder.
**
**		@param decoder		Decoder
**		@param buf		Buffer to write data to
**		@param len		Length of the buffer
**
**		@return				Number of bytes read
*/
local int MadRead(struct mad_decoder* decoder, unsigned char* buf, int len)
{
	struct mad_stream* stream;
	struct mad_frame* frame;
	struct mad_synth* synth;

	DebugLevel0Fn("%p %p %d\n" _C_ decoder _C_ buf _C_ len);

	stream = &decoder->sync->stream;
	frame = &decoder->sync->frame;
	synth = &decoder->sync->synth;
	DebugLevel0Fn("Error: %d\n" _C_ stream->error);
	do {
		DebugLevel0Fn("Read stream\n");
		switch (MAD_read(decoder->cb_data, stream)) {
			case MAD_FLOW_STOP:
				return 0;
			case MAD_FLOW_BREAK:
				return -1;
			case MAD_FLOW_IGNORE:
				continue;
			case MAD_FLOW_CONTINUE:
				break;
		}

		while (1) {
			if (mad_frame_decode(frame, stream) == -1) {
				if (!MAD_RECOVERABLE(stream->error)) {
					break;
				}

				switch (MAD_error(decoder->cb_data, stream, frame)) {
					case MAD_FLOW_STOP:
						return 0;
					case MAD_FLOW_BREAK:
						return -1;
					case MAD_FLOW_IGNORE:
						break;
					case MAD_FLOW_CONTINUE:
					default:
						continue;
				}
			}

			mad_synth_frame(synth, frame);

#if 0
			// FIXME: write out the frame buffer!
			switch (decoder->output_func(decoder->cb_data, &frame->header,
					&synth->pcm)) {
				case MAD_FLOW_STOP:
					return 0;
				case MAD_FLOW_BREAK:
					return -1;
				case MAD_FLOW_IGNORE:
				case MAD_FLOW_CONTINUE:
					break;
			}
#endif

		}
		// Should stop here!
	} while (stream->error == MAD_ERROR_BUFLEN);

	return -1;
}

/**
**		Type member function to read from the mp3 file
**
**		@param sample		Sample reading from
**		@param buf		Buffer to write data to
**		@param len		Length of the buffer
**
**		@return				Number of bytes read
*/
local int Mp3ReadStream(Sample* sample, void* buf, int len)
{
	Mp3Data* data;
	int i;
	int n;

	DebugLevel0Fn("%p %d\n" _C_ buf _C_ len);

	data = sample->User;

	// see if we have enough read already
	if (data->PointerInBuffer - sample->Data + len > sample->Length) {
		// not enough in buffer, read more
		n = sample->Length - (data->PointerInBuffer - sample->Data);
		memcpy(sample->Data, data->PointerInBuffer, n);
		sample->Length = n;
		data->PointerInBuffer = sample->Data;

		n = MP3_BUFFER_SIZE - n;
		for (;;) {
			i = MadRead(data->Decoder, data->PointerInBuffer + sample->Length,
					n);
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
**		Type member function to free an mp3 file
**
**		@param sample		Sample to free
*/
local void Mp3FreeStream(Sample* sample)
{
	Mp3Data* data;

#ifdef DEBUG
	AllocatedSoundMemory -= sizeof(*sample) + MP3_BUFFER_SIZE;
#endif

	data = sample->User;

	// release the decoder

	mad_synth_finish(data->Decoder->sync->synth);
	mad_frame_finish(&data->Decoder->sync->frame);
	mad_stream_finish(&data->Decoder->sync->stream);

	free(data->Decoder->sync);
	data->Decoder->sync = NULL;
	mad_decoder_finish(data->Decoder);

	CLclose(data->User->File);

	free(data);
	free(sample);
}

/**
**		Mp3 object type structure.
*/
local const SampleType Mp3StreamSampleType = {
	Mp3ReadStream,
	Mp3FreeStream,
};

/**
**		Type member function to read from the mp3 file
**
**		@param sample		Sample reading from
**		@param buf		Buffer to write data to
**		@param len		Length of the buffer
**
**		@return				Number of bytes read
*/
local int Mp3Read(Sample* sample, void* buf, int len)
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
**		Type member function to free an mp3 file
**
**		@param sample		Sample to free
*/
local void Mp3Free(Sample* sample)
{
#ifdef DEBUG
	AllocatedSoundMemory -= sample->Length;
#endif

	free(sample);
}

/**
**		Mp3 object type structure.
*/
local const SampleType Mp3SampleType = {
	Mp3Read,
	Mp3Free,
};

/**
**		Load mp3.
**
**		@param name		File name.
**		@param flags		Load flags.
**
**		@return				Returns the loaded sample.
**
**		@todo				Support more flags, LoadOnDemand.
*/
global Sample* LoadMp3(const char* name, int flags)
{
	CLFile* f;
	unsigned char magic[2];
	Sample* sample;

	if (!(f = CLopen(name,CL_OPEN_READ))) {
		fprintf(stderr, "Can't open file `%s'\n", name);
		return NULL;
	}
	CLread(f, magic, sizeof(magic));
	// 0xFF 0xE? for mp3 stream
	if (magic[0] != 0xFF || (magic[1]&0xE0) != 0xE0) {
		CLclose(f);
		return NULL;
	}

	// FIXME: ugly way to rewind.
	CLclose(f);
	if (!(f = CLopen(name,CL_OPEN_READ))) {
		fprintf(stderr, "Can't open file `%s'\n", name);
		return NULL;
	}

	DebugLevel2Fn("Have mp3 file %s\n" _C_ name);

#ifdef MP3_STREAM_WORKS
	if (flags & PlayAudioStream)
#else
	if (0 && (flags & PlayAudioStream))
#endif
	{
		Mp3Data* data;

		sample = malloc(sizeof(*sample) + MP3_BUFFER_SIZE);
		if (!sample) {
			fprintf(stderr, "Out of memory\n");
			CLclose(f);
			return NULL;
		}
		data = malloc(sizeof(*data));
		if (!data) {
			fprintf(stderr, "Out of memory\n");
			free(sample);
			CLclose(f);
			return NULL;
		}
		sample->User = data;
		sample->Channels = 0;
		sample->SampleSize = 0;
		sample->Frequency = 0;
		sample->Length = 0;
		sample->Type = &Mp3StreamSampleType;

		data->User->File = f;
		data->User->Sample = sample;

		// configure input, output, and error functions

		mad_decoder_init(data->Decoder, data->User,
			MAD_read, NULL /* header */, NULL /* filter */, MAD_write,
			MAD_error, NULL /* message */);

		data->Decoder->sync = malloc(sizeof(*data->Decoder->sync));
		if (!data->Decoder->sync) {
			fprintf(stderr, "Out of memory\n");
			mad_decoder_finish(data->Decoder);
			free(data);
			free(sample);
			CLclose(f);
			return NULL;
		}

		mad_stream_init(&data->Decoder->sync->stream);
		mad_frame_init(&data->Decoder->sync->frame);
		mad_synth_init(&data->Decoder->sync->synth);
		mad_stream_options(&data->Decoder->sync->stream,
			data->Decoder->options);

		// Read first frame for channels, ...
		data->PointerInBuffer = sample->Data;
		sample->Length = MadRead(data->Decoder, sample->Data, MP3_BUFFER_SIZE);

		DebugLevel0Fn(" %d\n" _C_ sizeof(*sample) + MP3_BUFFER_SIZE);
#ifdef DEBUG
		AllocatedSoundMemory += sizeof(*sample) + MP3_BUFFER_SIZE;
#endif

		return sample;
	} else {
		MyUser user;
		struct mad_decoder decoder;

		sample = calloc(1, sizeof(*sample));
		user.File = f;
		user.Sample = sample;

		// configure input, output, and error functions

		mad_decoder_init(&decoder, &user,
			MAD_read, NULL /* header */, NULL /* filter */, MAD_write,
			MAD_error, NULL /* message */);

		mad_decoder_run(&decoder, MAD_DECODER_MODE_SYNC);

		// release the decoder
		mad_decoder_finish(&decoder);
		CLclose(f);

		user.Sample->Type = &Mp3SampleType;
		user.Sample->User = 0;

		DebugLevel0Fn(" %d\n" _C_ user.Sample->Length);
#ifdef DEBUG
		AllocatedSoundMemory += user.Sample->Length;
#endif

		return user.Sample;
	}
}

#endif		// USE_MAD

//@}
