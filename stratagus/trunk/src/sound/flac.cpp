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
/**@name flac.c			-	flac support */
//
//	(c) Copyright 2002-2003 by Lutz Sammer, Fabrice Rossi and Nehal Mistry
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

//@{

/*----------------------------------------------------------------------------
--		Includes
----------------------------------------------------------------------------*/

#include <stdio.h>
#include "stratagus.h"

#ifdef USE_FLAC		// {

#include <stdlib.h>
#include <string.h>
#include "FLAC/stream_decoder.h"

#include "myendian.h"
#include "iolib.h"
#include "sound_server.h"

/*----------------------------------------------------------------------------
--		Declaration
----------------------------------------------------------------------------*/

/**
**	  Private flac data structure to handle flac streaming.
*/
typedef struct _flac_data_ {
	char* PointerInBuffer;			/// Pointer into buffer
	CLFile* FlacFile;			/// File handle
	Sample* Sample;				/// Sample buffer
	int Bytes;				/// Amount of data to read
	FLAC__StreamDecoder* Stream;		/// Decoder stream
} FlacData;

#define FLAC_BUFFER_SIZE  (12 * 1024)		/// Buffer size to fill

local const SampleType FlacSampleType;

/*----------------------------------------------------------------------------
--		Functions
----------------------------------------------------------------------------*/

/**
**		Read callback from FLAC stream decoder.
**
**		@param stream		Decoder stream.
**		@param status		Error state.
**		@param user		User data.
*/
local void FLAC_error_callback(const FLAC__StreamDecoder* stream,
	FLAC__StreamDecoderErrorStatus status, void* user)
{
	DebugLevel0Fn(" %s\n" _C_ FLAC__StreamDecoderErrorStatusString[status]);
}

/**
**		Read callback from FLAC stream decoder.
**
**		@param stream		Decoder stream.
**		@param buffer		Buffer to be filled.
**		@param bytes		Number of bytes to be filled.
**		@param user		User data.
**
**		@return			Error status.
*/
local FLAC__StreamDecoderReadStatus FLAC_read_callback(
	const FLAC__StreamDecoder * stream ,
	FLAC__byte buffer[], unsigned int *bytes, void *user)
{
	unsigned i;
	CLFile* f;
	FlacData* data;

	DebugLevel3Fn("Read callback %d\n" _C_ *bytes);

	data = (FlacData*)user;
	f = data->FlacFile;

	if ((i = CLread(f, buffer, *bytes)) != *bytes) {
		*bytes = i;
		if (!i) {
			return FLAC__STREAM_DECODER_READ_STATUS_END_OF_STREAM;
		}
	}
	return FLAC__STREAM_DECODER_READ_STATUS_CONTINUE;
}

/**
**		Write callback from FLAC stream decoder.
**
**		@param stream		Decoder stream.
**		@param metadata		metadata block
**		@param user		User data.
*/
local void FLAC_metadata_callback(const FLAC__StreamDecoder* stream,
	const FLAC__StreamMetadata* metadata, void *user)
{
	Sample* sample;
	int rate;

	if (metadata->type == FLAC__METADATA_TYPE_STREAMINFO) {
		sample = ((FlacData*)user)->Sample;

		sample->Channels = metadata->data.stream_info.channels;
		sample->Frequency = metadata->data.stream_info.sample_rate;
		sample->SampleSize = metadata->data.stream_info.bits_per_sample;

		rate = 44100 / sample->Frequency;
		// will overbuffer, so double the amount to allocate
		sample = realloc(sample, sizeof(*sample) + 2 * rate * FLAC_BUFFER_SIZE);
		((FlacData*)(sample->User))->Sample = sample;
		((FlacData*)(sample->User))->PointerInBuffer = sample->Data;

		DebugLevel3Fn("Stream %d Channels, %d frequency, %d bits\n" _C_
			sample->Channels _C_ sample->Frequency _C_ sample->SampleSize);
	}
}

/**
**		Write callback from FLAC stream decoder.
**
**		@param stream		Decoder stream.
**		@param frame		Frame to decode.
**		@param buffer		Buffer to be filled.
**		@param user		User data.
**
**		@return			Error status.
*/
local FLAC__StreamDecoderWriteStatus FLAC_write_callback(
	const FLAC__StreamDecoder* stream,
	const FLAC__Frame* frame, const FLAC__int32* const buffer[], void* user)
{
	FlacData* data;
	Sample* sample;
	unsigned i;
	unsigned channel;
	void* p;
	int rate;
	int y;

	DebugLevel3Fn("Write callback %d bits, %d channel, %d bytes\n" _C_
		frame->header.bits_per_sample _C_ frame->header.channels _C_
		frame->header.blocksize);

	data = (FlacData*)user;

	sample = data->Sample;
	DebugCheck(frame->header.bits_per_sample != sample->SampleSize);

	i = frame->header.channels * frame->header.blocksize *
		frame->header.bits_per_sample / 8;

	rate = 44100 / sample->Frequency;

	if (sample->Type == &FlacSampleType) {
		// not streaming
		sample = realloc(sample, sizeof(*sample) + sample->Length + i * rate);
		if (!sample) {
			fprintf(stderr, "Out of memory!\n");
			CLclose(data->FlacFile);
			ExitFatal(-1);
		}
		data->Sample = sample;
		data->PointerInBuffer = sample->Data;
	}

	p = sample->Data + sample->Length;
	sample->Length += i * rate;
	data->Bytes -= i * rate;

	switch (sample->SampleSize) {
		case 8:
			for (i = 0; i < frame->header.blocksize; ++i) {
				for (y = 0; y < rate; ++y) {
					for (channel = 0; channel < frame->header.channels; channel++) {
						*((unsigned char*)p)++ = buffer[channel][i] + 128;
					}
				}
			}
			break;
		case 16:
			for (i = 0; i < frame->header.blocksize; ++i) {
				for (y = 0; y < rate; ++y) {
					for (channel = 0; channel < frame->header.channels; channel++) {
						*((short*)p)++ = buffer[channel][i];
					}
				}
			}
			break;
		default:
			fprintf(stderr, "Unsupported sample depth!\n");
			CLclose(data->FlacFile);
			ExitFatal(-1);
	}

	return FLAC__STREAM_DECODER_WRITE_STATUS_CONTINUE;
}

/**
**		Type member function to read from the flac file
**
**		@param sample		Sample reading from
**		@param buf		Buffer to write data to
**		@param len		Length of the buffer
**
**		@return			Number of bytes read
*/
local int FlacRead(Sample* sample, void* buf, int len)
{
	char* pos;

	pos = ((FlacData*)sample->User)->PointerInBuffer;

	if ((pos - sample->Data) + len > sample->Length) {
		len = sample->Length - (pos - sample->Data);
	}
	memcpy(buf, ((FlacData*)sample->User)->PointerInBuffer, len);
	((FlacData*)sample->User)->PointerInBuffer += len;

	return len;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

/**
**		Type member function to free an flac file
**
**		@param sample		Sample to free
*/
local void FlacFree(Sample* sample)
{
#ifdef DEBUG
	AllocatedSoundMemory -= sample->Length;
#endif

	free(sample);
}

/**
**		Flac object type structure.
*/
local const SampleType FlacSampleType = {
	FlacRead,
	FlacFree,
};

/**
**		Type member function to read from the flac file
**
**		@param sample		Sample reading from
**		@param buf		Buffer to write data to
**		@param len		Length of the buffer
**
**		@return			Number of bytes read
*/
local int FlacStreamRead(Sample* sample, void* buf, int len)
{
	FlacData* data;

	data = (FlacData*)sample->User;

	while (FLAC__stream_decoder_get_state(data->Stream) != FLAC__STREAM_DECODER_SEARCH_FOR_FRAME_SYNC) {
		// read metadata
		FLAC__stream_decoder_process_single(data->Stream);
	}

	if (data->PointerInBuffer - sample->Data + len > sample->Length) {
		// need to read new data
		sample->Length -= data->PointerInBuffer - sample->Data;
		memcpy(sample->Data, data->PointerInBuffer, sample->Length);
		data->PointerInBuffer = sample->Data;

		data->Bytes = FLAC_BUFFER_SIZE - sample->Length;

		while (data->Bytes > 0 && FLAC__stream_decoder_get_state(data->Stream) != FLAC__STREAM_DECODER_END_OF_STREAM) {
			FLAC__stream_decoder_process_single(data->Stream);
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
**		Type member function to free an flac file
**
**		@param sample		Sample to free
*/
local void FlacStreamFree(Sample* sample)
{
	FlacData* data;

#ifdef DEBUG
	AllocatedSoundMemory -= sizeof(*sample) + FLAC_BUFFER_SIZE;
#endif

	data = (FlacData*)sample->User;
	CLclose(data->FlacFile);
	FLAC__stream_decoder_finish(data->Stream);
	FLAC__stream_decoder_delete(data->Stream);
	free(data);
	free(sample);
}

/**
**		Flac stream type structure.
*/
local const SampleType FlacStreamSampleType = {
	FlacStreamRead,
	FlacStreamFree,
};

/**
**		Load flac.
**
**		@param name		File name.
**		@param flags		Load flags.
**
**		@return			Returns the loaded sample.
*/
global Sample* LoadFlac(const char* name, int flags)
{
	CLFile* f;
	Sample* sample;
	unsigned int magic[1];
	FLAC__StreamDecoder* stream;
	FlacData* data;

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

	sample = malloc(sizeof(*sample) + 2 * FLAC_BUFFER_SIZE);

	data = malloc(sizeof(FlacData));
	data->FlacFile = f;
	data->PointerInBuffer = sample->Data;
	data->Sample = sample;
	data->Stream = stream;
	data->Bytes = 0;

	sample->User = data;
	sample->Length = 0;

	FLAC__stream_decoder_set_read_callback(stream, FLAC_read_callback);
	FLAC__stream_decoder_set_write_callback(stream, FLAC_write_callback);
	FLAC__stream_decoder_set_metadata_callback(stream, FLAC_metadata_callback);
	FLAC__stream_decoder_set_error_callback(stream, FLAC_error_callback);
	FLAC__stream_decoder_set_client_data(stream, data);
	FLAC__stream_decoder_init(stream);

	if (flags & PlayAudioStream) {
		sample->Type = &FlacStreamSampleType;

		FLAC__stream_decoder_process_until_end_of_metadata(stream);
	} else {
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
	return data->Sample;
}

#endif		// USE_FLAC

//@}
