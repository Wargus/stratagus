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
/**@name flac.c - flac support */
//
//      (c) Copyright 2002-2004 by Lutz Sammer, Fabrice Rossi and Nehal Mistry
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

#ifdef USE_FLAC  // {

#include <stdlib.h>
#include <string.h>
#include "FLAC/stream_decoder.h"

#include "myendian.h"
#include "iolib.h"
#include "sound_server.h"

/*----------------------------------------------------------------------------
--  Declaration
----------------------------------------------------------------------------*/

/**
**  Private flac data structure to handle flac streaming.
*/
typedef struct _flac_data_ {
	FLAC__StreamDecoder *FlacStream;	/// Decoder stream
	CLFile *FlacFile;			/// File handle
} FlacData;

/*----------------------------------------------------------------------------
--  Functions
----------------------------------------------------------------------------*/

/**
**  Read callback from FLAC stream decoder.
**
**  @param stream  Decoder stream.
**  @param status  Error state.
**  @param user    User data.
*/
static void FLAC_error_callback(const FLAC__StreamDecoder *stream,
	FLAC__StreamDecoderErrorStatus status, void *user)
{
	DebugLevel0Fn(" %s\n" _C_ FLAC__StreamDecoderErrorStatusString[status]);
}

/**
**  Read callback from FLAC stream decoder.
**
**  @param stream  Decoder stream.
**  @param buffer  Buffer to be filled.
**  @param bytes   Number of bytes to be filled.
**  @param user    User data.
**
**  @return        Error status.
*/
static FLAC__StreamDecoderReadStatus FLAC_read_callback(
	const FLAC__StreamDecoder *stream, FLAC__byte buffer[],
	unsigned int *bytes, void *user)
{
	Sample *sample;
	FlacData *data;
	unsigned int i;

	DebugLevel3Fn("Read callback %d\n" _C_ *bytes);

	sample = user;
	data = sample->User;

	if ((i = CLread(data->FlacFile, buffer, *bytes)) != *bytes) {
		*bytes = i;
		if (!i) {
			return FLAC__STREAM_DECODER_READ_STATUS_END_OF_STREAM;
		}
	}
	return FLAC__STREAM_DECODER_READ_STATUS_CONTINUE;
}

/**
**  Write callback from FLAC stream decoder.
**
**  @param stream    Decoder stream.
**  @param metadata  metadata block
**  @param user      User data.
*/
static void FLAC_metadata_callback(const FLAC__StreamDecoder *stream,
	const FLAC__StreamMetadata *metadata, void *user)
{
	Sample *sample;

	if (metadata->type == FLAC__METADATA_TYPE_STREAMINFO) {
		sample = user;

		sample->Channels = metadata->data.stream_info.channels;
		sample->Frequency = metadata->data.stream_info.sample_rate;
		sample->SampleSize = metadata->data.stream_info.bits_per_sample;

		if (!sample->Buffer) {
			DebugCheck(!metadata->data.stream_info.total_samples);
			sample->Buffer = malloc(metadata->data.stream_info.total_samples * 4);
		}

		DebugLevel3Fn("Stream %d Channels, %d frequency, %d bits\n" _C_
			sample->Channels _C_ sample->Frequency _C_ sample->SampleSize);
	}
}

/**
**  Write callback from FLAC stream decoder.
**
**  @param stream  Decoder stream.
**  @param frame   Frame to decode.
**  @param buffer  Buffer to be filled.
**  @param user    User data.
**
**  @return        Error status.
*/
static FLAC__StreamDecoderWriteStatus FLAC_write_callback(
	const FLAC__StreamDecoder *stream, const FLAC__Frame *frame,
	const FLAC__int32* const buffer[], void *user)
{
	Sample *sample;
	FlacData *data;
	unsigned int i;
	int j;
	char *dest;
	char *buf;
	int ssize;

	DebugLevel3Fn("Write callback %d bits, %d channel, %d bytes\n" _C_
		frame->header.bits_per_sample _C_ frame->header.channels _C_
		frame->header.blocksize);

	sample = user;
	data = sample->User;

	DebugCheck(!sample->Buffer);
	DebugCheck(frame->header.bits_per_sample != sample->SampleSize);

	ssize = (frame->header.bits_per_sample / 8);
	buf = malloc(frame->header.blocksize * sample->Channels * ssize);

	// FLAC splits it up the channels, we need to sew it back together
	for (i = 0; i < frame->header.blocksize; ++i) {
		for (j = 0; j < sample->Channels; ++j) {
			if (ssize == 1) {
				buf[i * sample->Channels + j] = ((const char*)buffer[j])[i * sample->Channels];
			} else {
				buf[i * 2 * sample->Channels + j * 2] = ((const char*)buffer[j])[i * 2 * sample->Channels];
				buf[i * 2 * sample->Channels + j * 2 + 1] = ((const char*)buffer[j])[i * 2 * sample->Channels + 1];
			}
		}
	}

	dest = sample->Buffer + sample->Pos + sample->Len;
	i = ConvertToStereo32(buf, dest, sample->Frequency,
		sample->SampleSize / 8, sample->Channels,
		frame->header.blocksize * sample->Channels * 2);
	sample->Len += i;

	free(buf);

	return FLAC__STREAM_DECODER_WRITE_STATUS_CONTINUE;
}

/**
**  Type member function to read from the flac file
**
**  @param sample  Sample reading from
**  @param buf     Buffer to write data to
**  @param len     Length of the buffer
**
**  @return        Number of bytes read
*/
static int FlacStreamRead(Sample *sample, void *buf, int len)
{
	FlacData *data;

	data = sample->User;

	if (sample->Pos > SOUND_BUFFER_SIZE / 2) {
		memcpy(sample->Buffer, sample->Buffer + sample->Pos, sample->Len);
		sample->Pos = 0;
	}

	while (sample->Len < SOUND_BUFFER_SIZE / 4 &&
			FLAC__stream_decoder_get_state(data->FlacStream) != FLAC__STREAM_DECODER_END_OF_STREAM) {
		// need to read new data
		FLAC__stream_decoder_process_single(data->FlacStream);
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
**  Type member function to free an flac file
**
**  @param sample  Sample to free
*/
local void FlacStreamFree(Sample *sample)
{
	FlacData *data;

	data = sample->User;

	CLclose(data->FlacFile);
	FLAC__stream_decoder_finish(data->FlacStream);
	FLAC__stream_decoder_delete(data->FlacStream);
	free(data);
	free(sample->Buffer);
	free(sample);
}

/**
**  Flac stream type structure.
*/
local const SampleType FlacStreamSampleType = {
	FlacStreamRead,
	FlacStreamFree,
};

/**
**  Type member function to read from the flac file
**
**  @param sample  Sample reading from
**  @param buf     Buffer to write data to
**  @param len     Length of the buffer
**
**  @return        Number of bytes read
*/
local int FlacRead(Sample *sample, void *buf, int len)
{
	if (len > sample->Len) {
		len = sample->Len;
	}

	memcpy(buf, sample->Buffer + sample->Pos, len);
	sample->Pos += len;
	sample->Len -= len;

	return len;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

/**
**  Type member function to free an flac file
**
**  @param sample  Sample to free
*/
local void FlacFree(Sample *sample)
{
	free(sample->User);
	free(sample->Buffer);
	free(sample);
}

/**
**  Flac object type structure.
*/
local const SampleType FlacSampleType = {
	FlacRead,
	FlacFree,
};

/**
**  Load flac.
**
**  @param name   File name.
**  @param flags  Load flags.
**
**  @return       Returns the loaded sample.
*/
global Sample* LoadFlac(const char *name, int flags)
{
	Sample *sample;
	FlacData *data;
	CLFile *f;
	unsigned int magic[1];
	FLAC__StreamDecoder *stream;


	if (!(f = CLopen(name, CL_OPEN_READ))) {
		fprintf(stderr, "Can't open file `%s'\n", name);
		return NULL;
	}

	CLread(f, magic, sizeof(magic));
	if (AccessLE32(magic) != 0x43614C66) {		// "fLaC" in ASCII
		CLclose(f);
		return NULL;
	}

	DebugLevel2Fn("Loading flac file: %s\n" _C_ name);

	CLseek(f, 0, SEEK_SET);

	if (!(stream = FLAC__stream_decoder_new())) {
		fprintf(stderr, "Can't initialize flac decoder\n");
		CLclose(f);
		return NULL;
	}

	data = malloc(sizeof(FlacData));
	data->FlacFile = f;
	data->FlacStream = stream;

	sample = malloc(sizeof(Sample));
	sample->Len = 0;
	sample->Pos = 0;
	sample->User = data;

	FLAC__stream_decoder_set_read_callback(stream, FLAC_read_callback);
	FLAC__stream_decoder_set_write_callback(stream, FLAC_write_callback);
	FLAC__stream_decoder_set_metadata_callback(stream, FLAC_metadata_callback);
	FLAC__stream_decoder_set_error_callback(stream, FLAC_error_callback);
	FLAC__stream_decoder_set_client_data(stream, sample);
	FLAC__stream_decoder_init(stream);

	if (flags & PlayAudioStream) {
		sample->Buffer = malloc(SOUND_BUFFER_SIZE);
		sample->Type = &FlacStreamSampleType;

		FLAC__stream_decoder_process_until_end_of_metadata(stream);
	} else {
		// Buffer will be malloc'ed from metadata callback
		sample->Buffer = NULL;
		sample->Type = &FlacSampleType;

		DebugCheck(FLAC__stream_decoder_get_state(stream) !=
			FLAC__STREAM_DECODER_SEARCH_FOR_METADATA);
		FLAC__stream_decoder_process_until_end_of_stream(stream);
		DebugCheck(FLAC__stream_decoder_get_state(stream) !=
			FLAC__STREAM_DECODER_END_OF_STREAM);

		FLAC__stream_decoder_finish(stream);
		FLAC__stream_decoder_delete(stream);
		CLclose(f);
	}
	return sample;
}

#endif  // USE_FLAC

//@}
