//       _________ __                 __
//      /   _____//  |_____________ _/  |______     ____  __ __  ______
//      \_____  \\   __\_  __ \__  \\   __\__  \   / ___\|  |  \/  ___/
//      /        \|  |  |  | \// __ \|  |  / __ \_/ /_/  >  |  /\___ |
//     /_______  /|__|  |__|  (____  /__| (____  /\___  /|____//____  >
//             \/                  \/          \//_____/            \/
//  ______________________                           ______________________
//			  T H E   W A R   B E G I N S
//      Stratagus - A free fantasy real time strategy game engine
//
/**@name mad.c - mp3 support with libmad */
//
//      (c) Copyright 2002-2004 by Lutz Sammer
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
--  Declaration
----------------------------------------------------------------------------*/

#define MAD_INBUF_SIZE 65536

/**
**  Private mp3 data structure to handle mp3 streaming.
*/
typedef struct _mp3_data_ {
	struct mad_decoder MadDecoder;           /// Mad decoder handle
	CLFile* MadFile;                         /// File handle
	unsigned char Buffer[MAD_INBUF_SIZE];    /// Input buffer
	int BufferLen;                           /// Length of filled buffer
} MadData;

/*----------------------------------------------------------------------------
--		Functions
----------------------------------------------------------------------------*/

/**
**  MAD read callback.
**
**  @param user    Our user pointer.
**  @param stream  MP3 stream.
**
**  @return        MAP_FLOW_STOP if eof, MAD_FLOW_CONTINUE otherwise.
*/
static enum mad_flow MAD_read(void* user, struct mad_stream* stream)
{
	Sample *sample;
	MadData *data;
	int i;

	sample = user;
	data = sample->User;

	if (stream->next_frame) {
		memmove(data->Buffer, stream->next_frame, data->BufferLen =
			&data->Buffer[data->BufferLen] - stream->next_frame);
	}

	i = CLread(data->MadFile, data->Buffer + data->BufferLen, MAD_INBUF_SIZE - data->BufferLen);
	if (!i) {
		return MAD_FLOW_STOP;
	}

	data->BufferLen += i;
	mad_stream_buffer(stream, data->Buffer, data->BufferLen);

	return MAD_FLOW_CONTINUE;
}

/**
**  This is the output callback function. It is called after each frame of
**  MPEG audio data has been completely decoded. The purpose of this
**  callback is to output the decoded PCM audio.
**
**  @param user    User argument.
**  @param header  MAD header.
**  @param pcm     MAD pcm data struture.
*/
static enum mad_flow MAD_write(void* user,
	struct mad_header const* header,
	struct mad_pcm* pcm)
{
	Sample* sample;
	int i;
	int j;
	int n;
	short *buf;
	int s;
	int comp;

	sample = user;

	n = pcm->length;

	if (!sample->SampleSize) {
		sample->Frequency = pcm->samplerate;
		sample->Channels = pcm->channels;
		sample->SampleSize = 16;
	}

	comp = n * pcm->channels * 2;

	buf = (short*)(sample->Buffer + sample->Pos + sample->Len);

        for (i = 0; i < n; ++i) {
                for (j = 0; j < sample->Channels; ++j) {
			s = pcm->samples[j][i];
			// round
			s += (1L << (MAD_F_FRACBITS - 16));
			// clip
			if (s >= MAD_F_ONE) {
				s = MAD_F_ONE - 1;
			} else if (s < -MAD_F_ONE) {
				s = -MAD_F_ONE;
			}
			// quantize
			s >>= (MAD_F_FRACBITS + 1 - 16);
                        buf[i * sample->Channels + j] = s;
                }
        }

	sample->Len += comp;

	return MAD_FLOW_CONTINUE;
}

/**
**  This is the error callback function. It is called whenever a decoding
**  error occurs. The error is indicated by stream->error; the list of
**  possible MAD_ERROR_* errors can be found in the mad.h (or
**  libmad/stream.h) header file.
*/
static enum mad_flow MAD_error(void* user __attribute__((unused)),
	struct mad_stream* stream,
	struct mad_frame* frame __attribute__((unused)))
{
	fprintf(stderr, "decoding error 0x%04x (%s)\n",
		stream->error, mad_stream_errorstr(stream));

	return MAD_FLOW_BREAK;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

/**
**  Read one frame from mad decoder.
**
**  @param sample  Sample
**  @param buf     Buffer to write data to
**  @param len     Length of the buffer
**
**  @return        Number of bytes read
*/
static int MadRead(Sample *sample, unsigned char* buf, int len)
{
	struct mad_decoder *decoder;
	struct mad_stream* stream;
	struct mad_frame* frame;
	struct mad_synth* synth;

	decoder = &((MadData*)sample->User)->MadDecoder;

	DebugPrint("%p %p %d\n" _C_ decoder _C_ buf _C_ len);

	stream = &decoder->sync->stream;
	frame = &decoder->sync->frame;
	synth = &decoder->sync->synth;
	DebugPrint("Error: %d\n" _C_ stream->error);

	MAD_read(sample, stream);

	if (mad_frame_decode(frame, stream) == -1) {
		Assert(0);
	}
	mad_synth_frame (synth, frame);
	
	decoder->output_func(decoder->cb_data, &frame->header, &synth->pcm);


	return 0;

	do {
		DebugPrint("Read stream\n");
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
**  Type member function to read from the mp3 file
**
**  @param sample  Sample reading from
**  @param buf     Buffer to write data to
**  @param len     Length of the buffer
**
**  @return        Number of bytes read
*/
static int Mp3ReadStream(Sample* sample, void* buf, int len)
{
	MadData* data;
	int i;
	int n;
	int divide;
	char sndbuf[SOUND_BUFFER_SIZE];

	DebugPrint("%p %d\n" _C_ buf _C_ len);

	data = sample->User;

        if (sample->Pos > SOUND_BUFFER_SIZE / 2) {
                memcpy(sample->Buffer, sample->Buffer + sample->Pos, sample->Len);
                sample->Pos = 0;
        }

	divide = 176400 / (sample->Frequency * 2 * sample->Channels);

	while (sample->Len < SOUND_BUFFER_SIZE / 4) {
		// not enough in buffer, read more
		n = (SOUND_BUFFER_SIZE - sample->Len) / divide;

		i = MadRead(sample, sndbuf, n);
		if (i <= 0) {
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
**  Type member function to free an mp3 file
**
**  @param sample  Sample to free
*/
static void Mp3FreeStream(Sample* sample)
{
	MadData* data;

	data = sample->User;

	// release the decoder
	mad_synth_finish(data->MadDecoder.sync->synth);
	mad_frame_finish(&data->MadDecoder.sync->frame);
	mad_stream_finish(&data->MadDecoder.sync->stream);

//	free(data->MadDecoder.sync);
	mad_decoder_finish(&data->MadDecoder);

	CLclose(data->MadFile);

	free(data->Buffer);
	free(data);
	free(sample);
}

/**
**  Mp3 object type structure.
*/
static const SampleType Mp3StreamSampleType = {
	Mp3ReadStream,
	Mp3FreeStream,
};

/**
**  Type member function to read from the mp3 file
**
**  @param sample  Sample reading from
**  @param buf     Buffer to write data to
**  @param len     Length of the buffer
**
**  @return        Number of bytes read
*/
static int Mp3Read(Sample* sample, void* buf, int len)
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
**  Type member function to free an mp3 file
**
**  @param sample  Sample to free
*/
static void Mp3Free(Sample* sample)
{
	free(sample->User);
	free(sample->Buffer);
	free(sample);
}

/**
**  Mp3 object type structure.
*/
static const SampleType Mp3SampleType = {
	Mp3Read,
	Mp3Free,
};

/**
**  Load mp3.
**
**  @param name   File name.
**  @param flags  Load flags.
**
**  @return       Returns the loaded sample.
*/
Sample* LoadMp3(const char* name, int flags)
{
	CLFile* f;
	unsigned char magic[2];
	Sample* sample;
	MadData *data;

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

	CLseek(f, 0, SEEK_SET);

	data = malloc(sizeof(MadData));
	data->MadFile = f;
	data->BufferLen = 0;

	sample = malloc(sizeof(Sample));
	sample->User = data;
	sample->Len = 0;
	sample->Pos = 0;
	sample->SampleSize = 0;

	// streaming currently broken
	if (0 && flags & PlayAudioStream) {
		sample->SampleSize = 0;

		sample->Type = &Mp3StreamSampleType;

		// configure input, output, and error functions
		mad_decoder_init(&data->MadDecoder, sample,
			MAD_read, NULL /* header */, NULL /* filter */, MAD_write,
			MAD_error, NULL /* message */);

		data->MadDecoder.sync = malloc(sizeof(*data->MadDecoder.sync));

		mad_stream_init(&data->MadDecoder.sync->stream);
		mad_frame_init(&data->MadDecoder.sync->frame);
		mad_synth_init(&data->MadDecoder.sync->synth);
		mad_stream_options(&data->MadDecoder.sync->stream,
			data->MadDecoder.options);

		MadRead(sample, sample->Buffer, SOUND_BUFFER_SIZE);

	} else {
		sample->Buffer = malloc(55000000);
		Assert(sample->Buffer);

		// configure input, output, and error functions

		mad_decoder_init(&data->MadDecoder, sample,
			MAD_read, NULL /* header */, NULL /* filter */, MAD_write,
			MAD_error, NULL /* message */);

		mad_decoder_run(&data->MadDecoder, MAD_DECODER_MODE_SYNC);

		// release the decoder
		mad_decoder_finish(&data->MadDecoder);
		CLclose(f);

		sample->Type = &Mp3SampleType;

		DebugPrint(" %d\n" _C_ sample->Len);
	}

	return sample;
}

#endif		// USE_MAD

//@}
