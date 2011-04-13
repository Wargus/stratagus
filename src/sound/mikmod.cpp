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

#include <mikmod.h>

/*----------------------------------------------------------------------------
--  Declarations
----------------------------------------------------------------------------*/

struct MikModData {
	MODULE *MikModModule;
	CFile  *MikModFile;
};

static CFile *CurrentFile;

class CSampleMikMod : public CSample
{
public:
	~CSampleMikMod();
	int Read(void *buf, int len);

	MikModData Data;
};

class CSampleMikModStream : public CSample
{
public:
	~CSampleMikModStream();
	int Read(void *buf, int len);

	MikModData Data;
};

/*----------------------------------------------------------------------------
--  Functions
----------------------------------------------------------------------------*/

static BOOL Seek(struct MREADER *, long off, int whence)
{
	return CurrentFile->seek(off, whence);
}

static long Tell(struct MREADER *)
{
	return CurrentFile->tell();
}

static BOOL Read(struct MREADER *, void *buf, size_t len)
{
	return CurrentFile->read(buf, len);
}

static int Get(struct MREADER *)
{
	char c;
	CurrentFile->read(&c, 1);
	return c;
}

static BOOL Eof(struct MREADER *)
{
	return 0;
}

static MREADER MReader = { Seek, Tell, Read, Get, Eof };

/**
**  Type member function to read from the module
**
**  @param buf     Buffer to write data to
**  @param len     Length of the buffer
**
**  @return        Number of bytes read
*/
int CSampleMikModStream::Read(void *buf, int len)
{
	int read;

	// fill up the buffer
	read = 0;
	while (this->Len < SOUND_BUFFER_SIZE / 2 && Player_Active()) {
		memcpy(this->Buffer, this->Buffer + this->Pos, this->Len);
		this->Pos = 0;
		CurrentFile = this->Data.MikModFile;
		read = VC_WriteBytes((SBYTE *)this->Buffer + this->Pos,
			SOUND_BUFFER_SIZE - (this->Pos + this->Len));
		this->Len += read;
	}

	if (this->Len < len) {
		// EOF
		len = this->Len;
	}

	memcpy(buf, this->Buffer + this->Pos, len);
	this->Len -= len;
	this->Pos += len;

	return len;
}

/**
**  Type member function to free sample
*/
CSampleMikModStream::~CSampleMikModStream()
{
	CurrentFile = this->Data.MikModFile;

	Player_Stop();
	Player_Free(this->Data.MikModModule);
	MikMod_Exit();
	this->Data.MikModFile->close();
	delete this->Data.MikModFile;
	delete[] this->Buffer;
}

/**
**  Type member function to read from the module
**
**  @param buf     Buffer to write data to
**  @param len     Length of the buffer
**
**  @return        Number of bytes read
*/
int CSampleMikMod::Read(void *buf, int len)
{
	if (this->Len < len) {
		len = this->Len;
	}

	memcpy(buf, this->Buffer + this->Pos, len);
	this->Pos += len;
	this->Len -= len;

	return len;
}

/**
**  Type member function to free sample
*/
CSampleMikMod::~CSampleMikMod()
{
	delete[] this->Buffer;
}


/**
**  Load MikMod.
**
**  @param name   Filename of the module.
**  @param flags  Unused.
**
**  @return       Returns the loaded sample.
*/
CSample *LoadMikMod(const char *name, int flags)
{
	CSample *sample;
	MikModData *data;
	MODULE *module;
	CFile *f;
	char s[256];
	static int registered = 0;

	md_mode |= DMODE_STEREO | DMODE_INTERP | DMODE_SURROUND | DMODE_HQMIXER;
	MikMod_RegisterDriver(&drv_nos);
	if (!registered) {
		MikMod_RegisterAllLoaders();
		registered = 1;
	}

	strcpy_s(s, sizeof(s), name);
	f = new CFile;
	if (f->open(name, CL_OPEN_READ) == -1) {
		delete f;
		return NULL;
	}
	CurrentFile = f;

	MikMod_Init((char*)"");
	module = Player_LoadGeneric(&MReader, 64, 0);
	if (!module) {
		MikMod_Exit();
		f->close();
		delete f;
		return NULL;
	}

	if (flags & PlayAudioStream) {
		sample = new CSampleMikModStream;
		data = &((CSampleMikModStream *)sample)->Data;
	} else {
		sample = new CSampleMikMod;
		data = &((CSampleMikMod *)sample)->Data;
	}
	data->MikModFile = f;
	data->MikModModule = module;
	sample->Channels = 2;
	sample->SampleSize = 16;
	sample->Frequency = 44100;
	sample->Pos = 0;

	if (flags & PlayAudioStream) {
		sample->Len = 0;
		sample->Buffer = new unsigned char[SOUND_BUFFER_SIZE];

		Player_Start(data->MikModModule);
	} else {
		int read;
		int pos;

		// FIXME: need to find the correct length
		sample->Len = 55000000;
		sample->Buffer = new unsigned char[sample->Len];

		pos = 0;
		Player_Start(data->MikModModule);
		while (Player_Active()) {
			read = VC_WriteBytes((SBYTE *)sample->Buffer + pos,
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
