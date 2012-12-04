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
/**@name wav.cpp - wav support */
//
//      (c) Copyright 2003-2005 by Lutz Sammer, Fabrice Rossi and Nehal Mistry
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

//@{

/*----------------------------------------------------------------------------
-- Includes
----------------------------------------------------------------------------*/

#include "stratagus.h"

#include "SDL.h"
#include "SDL_endian.h"

#include "iolib.h"
#include "sound_server.h"
#include "wav.h"

/*----------------------------------------------------------------------------
--  Declaration
----------------------------------------------------------------------------*/

/**
**  Private wav data structure to handle wav streaming.
*/
struct WavData {
	CFile *WavFile;       /// Wav file handle
	int ChunkRem;         /// Bytes remaining in chunk
};

class CSampleWav : public CSample
{
public:
	~CSampleWav();
	int Read(void *buf, int len);

	WavData Data;
};

class CSampleWavStream : public CSample
{
public:
	~CSampleWavStream();
	int Read(void *buf, int len);

	WavData Data;
};

/*----------------------------------------------------------------------------
--  Functions
----------------------------------------------------------------------------*/

static void swapEndianness(WavHeader *wavHeader)
{
	wavHeader->MagicRiff = SDL_SwapLE32(wavHeader->MagicRiff);
	wavHeader->Length = SDL_SwapLE32(wavHeader->Length);
	wavHeader->MagicWave = SDL_SwapLE32(wavHeader->MagicWave);
}

static bool Check(const WavHeader &wavHeader)
{
	if (wavHeader.MagicRiff != RIFF) {
		return false;
	}
	if (wavHeader.MagicWave != WAVE) {
		printf("Wrong magic %x (not %x)\n", wavHeader.MagicWave, WAVE);
		return false;
	}
	return true;
}

static void swapEndianness(WavChunk *chunk)
{
	chunk->Magic = SDL_SwapLE32(chunk->Magic);
	chunk->Length = SDL_SwapLE32(chunk->Length);
}

static void swapEndianness(WavFMT *wavfmt)
{
	wavfmt->Encoding = SDL_SwapLE16(wavfmt->Encoding);
	wavfmt->Channels = SDL_SwapLE16(wavfmt->Channels);
	wavfmt->Frequency = SDL_SwapLE32(wavfmt->Frequency);
	wavfmt->ByteRate = SDL_SwapLE32(wavfmt->ByteRate);
	wavfmt->SampleSize = SDL_SwapLE16(wavfmt->SampleSize);
	wavfmt->BitsPerSample = SDL_SwapLE16(wavfmt->BitsPerSample);
}

static bool IsWavFormatSupported(const WavFMT &wavfmt)
{
	if (wavfmt.Encoding != WAV_PCM_CODE) {
		printf("Unsupported encoding %d\n", wavfmt.Encoding);
		return false;
	}
	if (wavfmt.Channels != WAV_MONO && wavfmt.Channels != WAV_STEREO) {
		printf("Unsupported channels %d\n", wavfmt.Channels);
		return false;
	}
	if (wavfmt.SampleSize != 1 && wavfmt.SampleSize != 2 && wavfmt.SampleSize != 4) {
		printf("Unsupported sample size %d\n", wavfmt.SampleSize);
		return false;
	}
	if (wavfmt.BitsPerSample != 8 && wavfmt.BitsPerSample != 16) {
		printf("Unsupported bits per sample %d\n", wavfmt.BitsPerSample);
		return false;
	}
	Assert(wavfmt.Frequency == 44100 || wavfmt.Frequency == 22050 || wavfmt.Frequency == 11025);
	return true;
}

int CSampleWavStream::Read(void *buf, int len)
{
	WavChunk chunk;
	unsigned char *sndbuf;
	int comp; // number of compressed bytes actually read
	int i;
	int read;
	int bufrem;

	if (this->Pos > SOUND_BUFFER_SIZE / 2) {
		memcpy(this->Buffer, this->Buffer + this->Pos, this->Len);
		this->Pos = 0;
	}

	while (this->Len < SOUND_BUFFER_SIZE / 4) {
		if (!this->Data.ChunkRem) {
			// read next chunk
			comp = this->Data.WavFile->read(&chunk, sizeof(chunk));

			if (!comp) {
				// EOF
				this->Data.ChunkRem = 0;
				break;
			}

			swapEndianness(&chunk);
			if (chunk.Magic != DATA) {
				this->Data.WavFile->seek(chunk.Length, SEEK_CUR);
				continue;
			}
			this->Data.ChunkRem = chunk.Length;
		}

		bufrem = SOUND_BUFFER_SIZE - (this->Pos + this->Len);
		if (this->Data.ChunkRem > bufrem) {
			read = bufrem;
		} else {
			read = this->Data.ChunkRem;
		}
		this->Data.ChunkRem -= read;

		sndbuf = this->Buffer + this->Pos + this->Len;

		comp = this->Data.WavFile->read(sndbuf, read);
		if (!comp) {
			break;
		}

		read >>= 1;
		for (i = 0; i < read; ++i) {
			((unsigned short *)sndbuf)[i] = SDL_SwapLE16(((unsigned short *)sndbuf)[i]);
		}

		this->Len += comp;
	}

	if (this->Len < len) {
		len = this->Len;
	}

	memcpy(buf, this->Buffer + this->Pos, len);
	this->Pos += len;
	this->Len -= len;

	return len;
}

CSampleWavStream::~CSampleWavStream()
{
	this->Data.WavFile->close();
	delete this->Data.WavFile;
	delete[] this->Buffer;
}

int CSampleWav::Read(void *buf, int len)
{
	if (len > this->Len) {
		len = this->Len;
	}

	memcpy(buf, this->Buffer + this->Pos, len);
	this->Pos += len;
	this->Len -= len;

	return len;
}

CSampleWav::~CSampleWav()
{
	delete[] this->Buffer;
}

/**
**  Load wav.
**
**  @param name   File name.
**  @param flags  Load flags.
**
**  @return       Returns the loaded sample.
**
**  @todo         Add ADPCM loading support!
*/
CSample *LoadWav(const char *name, int flags)
{
	CFile *f = new CFile;

	if (f->open(name, CL_OPEN_READ) == -1) {
		printf("Can't open file `%s'\n", name);
		delete f;
		return NULL;
	}
	WavHeader wavHeader;
	if (f->read(&wavHeader, sizeof(wavHeader)) != sizeof(wavHeader)) {
		f->close();
		delete f;
		return NULL;
	}
	// Convert to native format
	swapEndianness(&wavHeader);

	if (Check(wavHeader) == false) {
		f->close();
		delete f;
		return NULL;
	}

	WavChunk chunk;
	if (f->read(&chunk, sizeof(chunk)) != sizeof(chunk)) {
		f->close();
		delete f;
		return NULL;
	}
	// Convert to native format
	swapEndianness(&chunk);

	while (chunk.Magic != FMT) {
		printf("Discard wavChunk '%x'\n", chunk.Magic);
		std::vector<char> buffer;

		buffer.resize(chunk.Length);
		if (f->read(&buffer[0], chunk.Length) != static_cast<int>(chunk.Length)
			|| f->read(&chunk, sizeof(chunk)) != sizeof(chunk)) {
			f->close();
			delete f;
			return NULL;
		}
		// Convert to native format
		swapEndianness(&chunk);
	}
	if (chunk.Length < 16) {
		printf("Wrong length %d (not %d)\n", chunk.Length, 16);
		f->close();
		delete f;
		return NULL;
	}
	WavFMT wavfmt;

	if (f->read(&wavfmt, sizeof(wavfmt)) != sizeof(wavfmt)) {
		f->close();
		delete f;
		return NULL;
	}
	// Convert to native format
	swapEndianness(&wavfmt);

	if (chunk.Length != 16) {
		std::vector<char> buffer;
		const int extraSize = chunk.Length - 16;

		buffer.resize(extraSize);
		if (f->read(&buffer[0], extraSize) != extraSize) {
			f->close();
			delete f;
			return NULL;
		}
	}

	//  Check if supported
	if (IsWavFormatSupported(wavfmt) == false) {
		f->close();
		delete f;
		return NULL;
	}

	CSample *sample;
	WavData *data;
	//
	//  Read sample
	//
	if (flags & PlayAudioStream) {
		CSampleWavStream *sampleWavStream = new CSampleWavStream;
		sample = sampleWavStream;
		sampleWavStream->Data.WavFile = f;
		data = &sampleWavStream->Data;
	} else {
		CSampleWav *sampleWav = new CSampleWav;
		sample = sampleWav;
		sampleWav->Data.WavFile = f;
		data = &sampleWav->Data;
	}
	sample->Channels = wavfmt.Channels;
	sample->SampleSize = wavfmt.SampleSize * 8 / sample->Channels;
	sample->Frequency = wavfmt.Frequency;
	sample->BitsPerSample = wavfmt.BitsPerSample;
	sample->Len = 0;
	sample->Pos = 0;

	if (flags & PlayAudioStream) {
		data->ChunkRem = 0;
		sample->Buffer = new unsigned char[SOUND_BUFFER_SIZE];
	} else {
		char sndbuf[SOUND_BUFFER_SIZE];

		sample->Buffer = NULL;
		int read = 0;
		int rem = 0;
		while (1) {
			if (!rem) {
				// read next chunk
				const int comp = f->read(&chunk, sizeof(chunk));

				if (!comp) {
					// EOF
					break;
				}
				swapEndianness(&chunk);
				if (chunk.Magic != DATA) {
					f->seek(chunk.Length, SEEK_CUR);
					continue;
				}
				rem = chunk.Length;
			}

			const int bufrem = SOUND_BUFFER_SIZE;
			if (rem > bufrem) {
				read = bufrem;
			} else {
				read = rem;
			}
			rem -= read;

			unsigned char *b = new unsigned char[sample->Len + read];
			Assert(b);
			memcpy(b, sample->Buffer, sample->Len);
			delete[] sample->Buffer;
			sample->Buffer = b;

			const int comp = data->WavFile->read(sndbuf, read);
			Assert(comp == read);

			if (sample->SampleSize == 16) {
				read >>= 1;
				for (int i = 0; i < read; ++i) {
					((unsigned short *)(sample->Buffer + sample->Pos + sample->Len))[i] =
						SDL_SwapLE16(((unsigned short *)sndbuf)[i]);
				}
			} else {
				memcpy((sample->Buffer + sample->Pos + sample->Len), sndbuf, comp);
			}

			sample->Len += comp;
		}

		data->WavFile->close();
		delete data->WavFile;
	}

	return sample;
}

//@}
