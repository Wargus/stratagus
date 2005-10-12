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
/**@name flac.cpp - flac support */
//
//      (c) Copyright 2002-2005 by Lutz Sammer, Fabrice Rossi and Nehal Mistry
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
struct FlacData {
	FLAC__StreamDecoder *FlacStream;  /// Decoder stream
	CFile *FlacFile;                  /// File handle
};

class CSampleFlac : public CSample
{
public:
	int Read(void *buf, int len);
	void Free();

	FlacData Data;
};

class CSampleFlacStream : public CSample
{
public:
	int Read(void *buf, int len);
	void Free();

	FlacData Data;
};

struct FlacUserData {
	CSample *Sample;
	FlacData *Data;
};

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
	DebugPrint(" %s\n" _C_ FLAC__StreamDecoderErrorStatusString[status]);
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
	CSample *sample;
	FlacData *data;
	unsigned int i;

	sample = ((FlacUserData *)user)->Sample;
	data = ((FlacUserData *)user)->Data;

	if ((i = data->FlacFile->read(buffer, *bytes)) != *bytes) {
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
	CSample *sample;

	if (metadata->type == FLAC__METADATA_TYPE_STREAMINFO) {
		sample = ((FlacUserData *)user)->Sample;

		sample->Channels = metadata->data.stream_info.channels;
		sample->Frequency = metadata->data.stream_info.sample_rate;
		sample->SampleSize = metadata->data.stream_info.bits_per_sample;

		if (!sample->Buffer) {
			Assert(metadata->data.stream_info.total_samples);
			sample->Buffer = new unsigned char[metadata->data.stream_info.total_samples * 4];
		}
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
	const FLAC__int32 *const buffer[], void *user)
{
	CSample *sample;
	FlacData *data;
	unsigned int i;
	int j;
	char *buf;
	int ssize;

	sample = ((FlacUserData *)user)->Sample;
	data = ((FlacUserData *)user)->Data;

	Assert(sample->Buffer);
	Assert(frame->header.bits_per_sample == sample->SampleSize);

	ssize = (frame->header.bits_per_sample / 8);
	buf = new char[frame->header.blocksize * sample->Channels * ssize];

	// FIXME: mono flac files don't play correctly

	// FLAC splits it up the channels, we need to sew it back together
	for (i = 0; i < frame->header.blocksize; ++i) {
		for (j = 0; j < sample->Channels; ++j) {
			if (ssize == 1) {
				buf[i * sample->Channels + j] = ((const char *)buffer[j])[i];
			} else {
				buf[i * 2 * sample->Channels + j * 2] = ((const char *)buffer[j])[i * 2];
				buf[i * 2 * sample->Channels + j * 2 + 1] = ((const char *)buffer[j])[i * 2 + 1];

			}
		}
	}

	memcpy(sample->Buffer + sample->Pos + sample->Len, buf,
		frame->header.blocksize * sample->Channels * ssize);
	sample->Len += frame->header.blocksize * sample->Channels * ssize;

	delete[] buf;

	return FLAC__STREAM_DECODER_WRITE_STATUS_CONTINUE;
}


/**
**  Type member function to read from the flac file
**
**  @param buf     Buffer to write data to
**  @param len     Length of the buffer
**
**  @return        Number of bytes read
*/
int CSampleFlacStream::Read(void *buf, int len)
{
	if (this->Pos > SOUND_BUFFER_SIZE / 2) {
		memcpy(this->Buffer, this->Buffer + this->Pos, this->Len);
		this->Pos = 0;
	}

	while (this->Len < SOUND_BUFFER_SIZE / 4 &&
			FLAC__stream_decoder_get_state(this->Data.FlacStream) != FLAC__STREAM_DECODER_END_OF_STREAM) {
		// need to read new data
		FLAC__stream_decoder_process_single(this->Data.FlacStream);
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
**  Type member function to free an flac file
*/
void CSampleFlacStream::Free()
{
	this->Data.FlacFile->close();
	delete this->Data.FlacFile;
	FLAC__stream_decoder_finish(this->Data.FlacStream);
	FLAC__stream_decoder_delete(this->Data.FlacStream);
	delete[] this->Buffer;
}

/**
**  Type member function to read from the flac file
**
**  @param buf     Buffer to write data to
**  @param len     Length of the buffer
**
**  @return        Number of bytes read
*/
int CSampleFlac::Read(void *buf, int len)
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
**  Type member function to free an flac file
*/
void CSampleFlac::Free()
{
	delete[] this->Buffer;
}


/**
**  Load flac.
**
**  @param name   File name.
**  @param flags  Load flags.
**
**  @return       Returns the loaded sample.
*/
CSample *LoadFlac(const char *name, int flags)
{
	CSample *sample;
	FlacData *data;
	CFile *f;
	unsigned int magic[1];
	FLAC__StreamDecoder* stream;

	f = new CFile;
	if (f->open(name, CL_OPEN_READ) == -1) {
		fprintf(stderr, "Can't open file `%s'\n", name);
		delete f;
		return NULL;
	}

	f->read(magic, sizeof(magic));
	if (AccessLE32(magic) != 0x43614C66) { // "fLaC" in ASCII
		f->close();
		delete f;
		return NULL;
	}

	f->seek(0, SEEK_SET);

	if (!(stream = FLAC__stream_decoder_new())) {
		fprintf(stderr, "Can't initialize flac decoder\n");
		f->close();
		delete f;
		return NULL;
	}

	if (flags & PlayAudioStream) {
		sample = new CSampleFlacStream;
		data = &((CSampleFlacStream *)sample)->Data;
	} else {
		sample = new CSampleFlac;
		data = &((CSampleFlac *)sample)->Data;
	}
	data->FlacFile = f;
	data->FlacStream = stream;
	sample->Len = 0;
	sample->Pos = 0;

	FLAC__stream_decoder_set_read_callback(stream, FLAC_read_callback);
	FLAC__stream_decoder_set_write_callback(stream, FLAC_write_callback);
	FLAC__stream_decoder_set_metadata_callback(stream, FLAC_metadata_callback);
	FLAC__stream_decoder_set_error_callback(stream, FLAC_error_callback);
	FlacUserData d = { sample, data };
	FLAC__stream_decoder_set_client_data(stream, &d);
	FLAC__stream_decoder_init(stream);

	if (flags & PlayAudioStream) {
		sample->Buffer = new unsigned char[SOUND_BUFFER_SIZE];
		FLAC__stream_decoder_process_until_end_of_metadata(stream);
	} else {
		// Buffer will be new'ed from metadata callback
		sample->Buffer = NULL;

		Assert(FLAC__stream_decoder_get_state(stream) ==
			FLAC__STREAM_DECODER_SEARCH_FOR_METADATA);
		FLAC__stream_decoder_process_until_end_of_stream(stream);
		Assert(FLAC__stream_decoder_get_state(stream) ==
			FLAC__STREAM_DECODER_END_OF_STREAM);

		FLAC__stream_decoder_finish(stream);
		FLAC__stream_decoder_delete(stream);
		f->close();
		delete f;
	}
	return sample;
}

#endif  // USE_FLAC

//@}
