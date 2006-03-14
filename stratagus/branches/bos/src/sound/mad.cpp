//       _________ __                 __
//      /   _____//  |_____________ _/  |______     ____  __ __  ______
//      \_____  \\   __\_  __ \__  \\   __\__  \   / ___\|  |  \/  ___/
//      /        \|  |  |  | \// __ \|  |  / __ \_/ /_/  >  |  /\___ |
//     /_______  /|__|  |__|  (____  /__| (____  /\___  /|____//____  >
//             \/                  \/          \//_____/            \/
//  ______________________                           ______________________
//                        T H E   W A R   B E G I N S
//         Stratagus - A free fantasy real time strategy game engine
//
/**@name mad.cpp - mp3 support with libmad */
//
//      (c) Copyright 2002-2005 by Lutz Sammer
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

#ifdef USE_MAD // {

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
struct MadData {
	struct mad_decoder MadDecoder;           /// Mad decoder handle
	CFile *MadFile;                          /// File handle
	unsigned char Buffer[MAD_INBUF_SIZE];    /// Input buffer
	int BufferLen;                           /// Length of filled buffer
};

class CSampleMad : public CSample
{
public:
	~CSampleMad();
	int Read(void *buf, int len);

	MadData Data;
};

class CSampleMadStream : public CSample
{
public:
	~CSampleMadStream();
	int Read(void *buf, int len);

	MadData Data;
};

struct MadUserData {
	CSample *Sample;
	MadData *Data;
};

/*----------------------------------------------------------------------------
-- Functions
----------------------------------------------------------------------------*/

/**
**  MAD read callback.
**
**  @param user    Our user pointer.
**  @param stream  MP3 stream.
**
**  @return        MAP_FLOW_STOP if eof, MAD_FLOW_CONTINUE otherwise.
*/
static enum mad_flow MAD_read(void *user, struct mad_stream *stream)
{
	CSample *sample;
	MadData *data;
	int i;

	sample = ((MadUserData *)user)->Sample;
	data = ((MadUserData *)user)->Data;

	if (stream->next_frame) {
		memmove(data->Buffer, stream->next_frame, data->BufferLen =
			&data->Buffer[data->BufferLen] - stream->next_frame);
	}

	i = data->MadFile->read(data->Buffer + data->BufferLen, MAD_INBUF_SIZE - data->BufferLen);
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
static enum mad_flow MAD_write(void *user,
	struct mad_header const *header, struct mad_pcm *pcm)
{
	CSample *sample;
	int i;
	int j;
	int n;
	short *buf;
	int s;
	int comp;

	sample = ((MadUserData *)user)->Sample;

	n = pcm->length;

	if (!sample->SampleSize) {
		sample->Frequency = pcm->samplerate;
		sample->Channels = pcm->channels;
		sample->SampleSize = 16;
	}

	comp = n * pcm->channels * 2;

	buf = (short *)(sample->Buffer + sample->Pos + sample->Len);

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
static enum mad_flow MAD_error(void *user,
	struct mad_stream *stream, struct mad_frame *frame)
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
**  @param data    Mad data
**  @param buf     Buffer to write data to
**  @param len     Length of the buffer
**
**  @return        Number of bytes read
*/
static int MadRead(CSample *sample, MadData *data, unsigned char *buf, int len)
{
	struct mad_decoder *decoder;
	struct mad_stream *stream;
	struct mad_frame *frame;
	struct mad_synth *synth;

	decoder = &data->MadDecoder;

	DebugPrint("%p %p %d\n" _C_ decoder _C_ buf _C_ len);

	stream = &decoder->sync->stream;
	frame = &decoder->sync->frame;
	synth = &decoder->sync->synth;
	DebugPrint("Error: %d\n" _C_ stream->error);

	MadUserData d = { sample, data };
	MAD_read(&d, stream);

	if (mad_frame_decode(frame, stream) == -1) {
		Assert(0);
	}
	mad_synth_frame(synth, frame);

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
**  @param buf  Buffer to write data to
**  @param len  Length of the buffer
**
**  @return     Number of bytes read
*/
int CSampleMadStream::Read(void *buf, int len)
{
	int i;
	int n;
	int divide;
	unsigned char sndbuf[SOUND_BUFFER_SIZE];

	DebugPrint("%p %d\n" _C_ buf _C_ len);

	if (this->Pos > SOUND_BUFFER_SIZE / 2) {
		memcpy(this->Buffer, this->Buffer + this->Pos, this->Len);
		this->Pos = 0;
	}

	divide = 176400 / (this->Frequency * 2 * this->Channels);

	while (this->Len < SOUND_BUFFER_SIZE / 4) {
		// not enough in buffer, read more
		n = (SOUND_BUFFER_SIZE - this->Len) / divide;

		i = MadRead(this, &this->Data, sndbuf, n);
		if (i <= 0) {
			break;
		}

		this->Len += i;
	}

	if (this->Len < len) {
		len = this->Len;
	}

	memcpy(buf, this->Buffer + this->Pos, len);
	this->Pos += len;
	this->Len -= len;

	return len;

}

/**
**  Type member function to free an mp3 file
*/
CSampleMadStream::~CSampleMadStream()
{
	// release the decoder
	mad_synth_finish(this->Data.MadDecoder.sync->synth);
	mad_frame_finish(&this->Data.MadDecoder.sync->frame);
	mad_stream_finish(&this->Data.MadDecoder.sync->stream);

// delete this->Data.MadDecoder.sync;
	mad_decoder_finish(&this->Data.MadDecoder);

	this->Data.MadFile->close();
	delete this->Data.MadFile;
}

/**
**  Type member function to read from the mp3 file
**
**  @param buf  Buffer to write data to
**  @param len  Length of the buffer
**
**  @return     Number of bytes read
*/
int CSampleMad::Read(void *buf, int len)
{
	if (len > this->Len) {
		len = this->Len;
	}

	memcpy(buf, this->Buffer + this->Pos, len);
	this->Pos += len;
	this->Len -= len;

	return len;
}

/**
**  Type member function to free an mp3 file
*/
CSampleMad::~CSampleMad()
{
	delete[] this->Buffer;
}


/**
**  Load mp3.
**
**  @param name   File name.
**  @param flags  Load flags.
**
**  @return       Returns the loaded sample.
*/
CSample *LoadMp3(const char *name, int flags)
{
	CFile *f;
	unsigned char magic[2];
	CSample *sample;
	MadData *data;

	f = new CFile;
	if (f->open(name, CL_OPEN_READ) == -1) {
		fprintf(stderr, "Can't open file `%s'\n", name);
		delete f;
		return NULL;
	}
	f->read(magic, sizeof(magic));
	// 0xFF 0xE? for mp3 stream
	if (magic[0] != 0xFF || (magic[1]&0xE0) != 0xE0) {
		f->close();
		delete f;
		return NULL;
	}

	f->seek(0, SEEK_SET);

	if (0 && flags & PlayAudioStream) {
		sample = new CSampleMadStream;
		data = &((CSampleMadStream *)sample)->Data;
	} else {
		sample = new CSampleMad;
		data = &((CSampleMad *)sample)->Data;
	}
	data->MadFile = f;
	data->BufferLen = 0;
	sample->Len = 0;
	sample->Pos = 0;
	sample->SampleSize = 0;

	// streaming currently broken
	if (0 && flags & PlayAudioStream) {
#if 0
		sample->SampleSize = 0;

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

		MadRead(sample, &sample->Data, sample->Buffer, SOUND_BUFFER_SIZE);
#endif
	} else {
		// FIXME: surely there's a better way to do this
		sample->Buffer = new unsigned char[55000000];
		Assert(sample->Buffer);

		// configure input, output, and error functions

		MadUserData d = { sample, data };
		mad_decoder_init(&data->MadDecoder, &d,
			MAD_read, NULL /* header */, NULL /* filter */, MAD_write,
			MAD_error, NULL /* message */);

		mad_decoder_run(&data->MadDecoder, MAD_DECODER_MODE_SYNC);

		// release the decoder
		mad_decoder_finish(&data->MadDecoder);
		f->close();
		delete f;

		DebugPrint(" %d\n" _C_ sample->Len);
	}

	return sample;
}

#endif // USE_MAD

//@}
