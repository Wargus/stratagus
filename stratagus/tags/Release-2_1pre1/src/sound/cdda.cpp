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
/**@name cdda.c			-	cdda support */
//
//	(c) Copyright 2002-2003 by Nehal Mistry
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
//	$Id: wav.c,v 1.7 2002/07/20 00:09:05 johns Exp $

//@{

/*----------------------------------------------------------------------------
--		Includes
----------------------------------------------------------------------------*/

#include "stratagus.h"

#if defined(USE_CDDA) // {

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "sound.h"
#include "sound_server.h"
#include "cdaudio.h"

/*----------------------------------------------------------------------------
--		Declarations
----------------------------------------------------------------------------*/

typedef struct _cdda_data {
	int PosInCd;						// Offset on CD to read from
	struct cdrom_read_audio Readdata;		// Structure for IOCTL
	char* PointerInBuffer;				// Position in buffer
	char* Buffer;						// Buffer start
} CddaData;

#define FRAME_SIZE 2352
#define CDDA_BUFFER_SIZE (12 * FRAME_SIZE)

/*----------------------------------------------------------------------------
--		Functions
----------------------------------------------------------------------------*/

/**
**		Type member function to read from the cd
**
**		@param sample			Sample reading from
**		@param buf			Buffer to write data to
**		@param len			Length of the buffer
**
**		@return					Number of bytes read
*/
static int CDRead(Sample* sample, void* buf, int len)
{
	CddaData* data;
	int n;

	data = (CddaData*)sample->User;

	data->Readdata.addr.lba = CDtocentry[CDTrack].cdte_addr.lba + data->PosInCd / FRAME_SIZE;
	data->Readdata.addr_format = CDROM_LBA;

	// end of track
	if (FRAME_SIZE * (CDtocentry[CDTrack + 1].cdte_addr.lba -
			CDtocentry[CDTrack].cdte_addr.lba) - data->PosInCd < len) {
		len = FRAME_SIZE * (CDtocentry[CDTrack+1].cdte_addr.lba -
			CDtocentry[CDTrack].cdte_addr.lba) - data->PosInCd;
		data->PosInCd = 0;
	}

	if (sample->Length - (data->PointerInBuffer - data->Buffer) < len) {
		// need to read more data
		sample->Length -= data->PointerInBuffer - data->Buffer;
		memcpy(data->Buffer, data->PointerInBuffer, sample->Length);
		data->PointerInBuffer = data->Buffer;

		n = CDDA_BUFFER_SIZE - sample->Length;

		data->Readdata.nframes = n / FRAME_SIZE;
		data->Readdata.buf = data->PointerInBuffer + sample->Length;
		ioctl(CDDrive, CDROMREADAUDIO, &data->Readdata);

		sample->Length += data->Readdata.nframes * FRAME_SIZE;
		data->PosInCd += data->Readdata.nframes * FRAME_SIZE;
	}

	memcpy(buf, data->PointerInBuffer, len);
	data->PointerInBuffer += len;
	return len;
}

/**
**		Type member function to free CDDA sample
**
**		@param sample			Sample to free
*/
static void CDFree(Sample* sample)
{
	free(sample);
}

/**
**		CDDA object type structure.
*/
static const SampleType CDStreamSampleType = {
	CDRead,
	CDFree,
};

/**
**		Load CD.
**
**		@param name		Unused.
**		@param flags		Unused.
**
**		@return				Returns the loaded sample.
**
*/
Sample* LoadCD(const char* name __attribute__((unused)),
		int flags __attribute__((unused)))
{
	Sample* sample;
	CddaData* data;

	sample = malloc(sizeof(*sample));
	sample->Channels = 2;
	sample->SampleSize = 16;
	sample->Frequency = 44100;
	sample->Type = &CDStreamSampleType;
	sample->Length = 0;

	data = malloc(sizeof(CddaData));
	data->PosInCd = 0;
	data->Buffer = malloc(CDDA_BUFFER_SIZE);
	data->PointerInBuffer = data->Buffer;
	sample->User = data;

	return sample;
}

#endif		// } USE_CDDA

//@}
