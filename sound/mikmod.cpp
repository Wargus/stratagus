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
/**@name mikmod.cpp - MikMod support */
//
//      (c) Copyright 2004-2005 by Nehal Mistry
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

#include "stratagus.h"

#ifdef USE_MIKMOD // {

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "sound.h"
#include "sound_server.h"
#include "iolib.h"

#define DrawIcon WinDrawIcon
#include <mikmod.h>
#undef DrawIcon

/*----------------------------------------------------------------------------
--  Declarations
----------------------------------------------------------------------------*/

typedef struct _mikmod_data_ {
	MODULE* MikModModule;
	CLFile *MikModFile;
} MikModData;

static CLFile *CurrentFile;

/*----------------------------------------------------------------------------
--  Functions
----------------------------------------------------------------------------*/

static BOOL Seek(struct MREADER* mreader, long off, int whence)
{
	return CurrentFile->seek(off, whence);
}

static long Tell(struct MREADER* mreader)
{
	return CurrentFile->tell();
}

static BOOL Read(struct MREADER* mreader, void *buf, size_t len)
{
	return CurrentFile->read(buf, len);
}

static int Get(struct MREADER* mreader)
{
	char c;
	CurrentFile->read(&c, 1);
	return c;
}

static BOOL Eof(struct MREADER* mreader)
{
	return 0;
}

static MREADER MReader = { Seek, Tell, Read, Get, Eof };

/**
**  Type member function to read from the module
**
**  @param sample  Sample reading from
**  @param buf     Buffer to write data to
**  @param len     Length of the buffer
**
**  @return        Number of bytes read
*/
static int MikModStreamRead(Sample* sample, void* buf, int len)
{
	MikModData* data;
	int read;

	data = (MikModData*)sample->User;

	// fill up the buffer
	read = 0;
	while (sample->Len < SOUND_BUFFER_SIZE / 2 && Player_Active()) {
		memcpy(sample->Buffer, sample->Buffer + sample->Pos, sample->Len);
		sample->Pos = 0;
		CurrentFile = data->MikModFile;
		read = VC_WriteBytes((SBYTE*)sample->Buffer + sample->Pos,
			SOUND_BUFFER_SIZE - (sample->Pos + sample->Len));
		sample->Len += read;
	}

	if (sample->Len < len) {
		// EOF
		len = sample->Len;
	}

	memcpy(buf, sample->Buffer + sample->Pos, len);
	sample->Len -= len;
	sample->Pos += len;

	return len;
}

/**
**  Type member function to free sample
**
**  @param sample  Sample to free
*/
static void MikModStreamFree(Sample* sample)
{
	MikModData *data;
	data = (MikModData*)sample->User;

	CurrentFile = data->MikModFile;

	Player_Stop();
	Player_Free(data->MikModModule);
	MikMod_Exit();
	data->MikModFile->close();
	delete data->MikModFile;
	free(sample->User);
	free(sample->Buffer);
	free(sample);
}

/**
**  MikMod object type structure.
*/
static const SampleType MikModStreamSampleType = {
	MikModStreamRead,
	MikModStreamFree,
};

/**
**  Type member function to read from the module
**
**  @param sample  Sample reading from
**  @param buf     Buffer to write data to
**  @param len     Length of the buffer
**
**  @return        Number of bytes read
*/
static int MikModRead(Sample* sample, void* buf, int len)
{
	if (sample->Len < len) {
		len = sample->Len;
	}

	memcpy(buf, sample->Buffer + sample->Pos, len);
	sample->Pos += len;
	sample->Len -= len;

	return len;
}

/**
**  Type member function to free sample
**
**  @param sample  Sample to free
*/
static void MikModFree(Sample* sample)
{
	free(sample->User);
	free(sample->Buffer);
	free(sample);
}

/**
**  MikMod object type structure.
*/
static const SampleType MikModSampleType = {
	MikModRead,
	MikModFree,
};

/**
**  Load MikMod.
**
**  @param name   Filename of the module.
**  @param flags  Unused.
**
**  @return       Returns the loaded sample.
**
*/
Sample* LoadMikMod(const char* name, int flags)
{
	Sample* sample;
	MikModData* data;
	char s[256];
	static int registered = 0;

	data = (MikModData*)malloc(sizeof(MikModData));

	md_mode |= DMODE_STEREO | DMODE_INTERP | DMODE_SURROUND | DMODE_HQMIXER;
	MikMod_RegisterDriver(&drv_nos);
	if (!registered) {
		MikMod_RegisterAllLoaders();
		registered = 1;
	}
	MikMod_Init("");

	strcpy(s, name);
	data->MikModFile = new CLFile;
	if (data->MikModFile->open(name, CL_OPEN_READ) == -1) {
		MikMod_Exit();
		delete data->MikModFile;
		free(data);
		return NULL;
	}
	CurrentFile = data->MikModFile;

	data->MikModModule = Player_LoadGeneric(&MReader, 64, 0);
	if (!data->MikModModule) {
		MikMod_Exit();
		data->MikModFile->close();
		delete data->MikModFile;
		free(data);
		return NULL;
	}

	sample = (Sample*)malloc(sizeof(*sample));
	sample->Channels = 2;
	sample->SampleSize = 16;
	sample->Frequency = 44100;
	sample->Pos = 0;
	sample->User = data;

	if (flags & PlayAudioStream) {
		sample->Len = 0;
		sample->Buffer = (unsigned char*)malloc(SOUND_BUFFER_SIZE);
		sample->Type = &MikModStreamSampleType;

		Player_Start(data->MikModModule);
	} else {
		int read;
		int pos;

		// FIXME: need to find the correct length
		sample->Len = 55000000;
		sample->Buffer = (unsigned char*)malloc(sample->Len);
		sample->Type = &MikModSampleType;

		pos = 0;
		Player_Start(data->MikModModule);
		while (Player_Active()) {
			read = VC_WriteBytes((SBYTE*)sample->Buffer + pos,
				 sample->Len - pos);
			pos += read;
		}

		Player_Stop();
		Player_Free(data->MikModModule);
		MikMod_Exit();

		data->MikModFile->close();
		delete data->MikModFile;
	}

	return sample;
}

#endif  // } USE_MIKMOD

//@}
