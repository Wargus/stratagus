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
/**@name mikmod.c - MikMod support */
//
//      (c) Copyright 2004 by Nehal Mistry
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

#define DrawIcon WinDrawIcon
#include <mikmod.h>
#undef DrawIcon

/*----------------------------------------------------------------------------
--  Declarations
----------------------------------------------------------------------------*/

typedef struct _mikmod_data_ {
	SAMPLE* MikModSample;
	MODULE* MikModModule;
	int Pos;                    /// Position in buffer
	char* Buffer;               /// Buffer start
	int Len;                    /// Length of filled buffer
} MikModData;

#define MIKMOD_BUFFER_SIZE (1024 * 32)

static CLFile *ModFile;

/*----------------------------------------------------------------------------
--  Functions
----------------------------------------------------------------------------*/

static BOOL Seek(struct MREADER* mreader, long off, int whence)
{
	return CLseek(ModFile, off, whence);
}

static long Tell(struct MREADER* mreader)
{
	return CLseek(ModFile, 0, SEEK_CUR);
}

static BOOL Read(struct MREADER* mreader, void *buf, size_t len)
{
	return CLread(ModFile, buf, len);
}

static int Get(struct MREADER* mreader)
{
	char c;
	CLread(ModFile, &c, 1);
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
local int MikModRead(Sample* sample, void* buf, int len)
{
	MikModData* data;
	int read;

	data = (MikModData*)sample->User;

	// fill up the buffer
	read = 0;
	if (data->Len < MIKMOD_BUFFER_SIZE / 2 && Player_Active()) {
		memcpy(data->Buffer, data->Buffer + data->Pos, data->Len);
		data->Pos = 0;
		read = VC_WriteBytes(data->Buffer + data->Pos,
			MIKMOD_BUFFER_SIZE - (data->Pos + data->Len));
		data->Len += read;
	}

	if (data->Len < len) {
		// EOF
		len = data->Len;
	}

	memcpy(buf, data->Buffer + data->Pos, len);
	data->Len -= len;
	data->Pos += len;

	return len;
}

/**
**  Type member function to free sample
**
**  @param sample  Sample to free
*/
local void MikModFree(Sample* sample)
{
	MikModData *data;

	data = (MikModData*)sample->User;

	CLclose(ModFile);
	Player_Stop();
	Player_Free(data->MikModModule);
	MikMod_Exit();
	free(data->Buffer);
	free(data);
	free(sample);
}

/**
**  MikMod object type structure.
*/
local const SampleType MikModStreamSampleType = {
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
global Sample* LoadMikMod(const char* name, int flags __attribute__((unused)))
{
	Sample* sample;
	MikModData* data;
	char s[256];
	static int registered = 0;

	data = malloc(sizeof(MikModData));
	data->Pos = 0;
	data->Buffer = malloc(MIKMOD_BUFFER_SIZE);
	data->Len = 0;

	MikMod_RegisterDriver(&drv_nos);
	if (!registered) {
		MikMod_RegisterAllLoaders();
		registered = 1;
	}
	MikMod_Init("");

	strcpy(s, name);
	ModFile = CLopen(name, CL_OPEN_READ);
	data->MikModModule = Player_LoadGeneric(&MReader, 64, 0);
	if (!data->MikModModule) {
		MikMod_Exit();
		free(data->Buffer);
		free(data);
		return NULL;
	}

	sample = malloc(sizeof(*sample));
	sample->Channels = 2;
	sample->SampleSize = 16;
	sample->Frequency = 44100;
	sample->Type = &MikModStreamSampleType;
	sample->Length = 0;
	sample->User = data;

	Player_Start(data->MikModModule);

	return sample;
}

#endif  // } USE_MIKMOD

//@}
