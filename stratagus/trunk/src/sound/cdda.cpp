//   ___________		     _________		      _____  __
//   \_	  _____/______   ____   ____ \_   ___ \____________ _/ ____\/  |_
//    |    __) \_  __ \_/ __ \_/ __ \/    \  \/\_  __ \__  \\   __\\   __\ 
//    |     \   |  | \/\  ___/\  ___/\     \____|  | \// __ \|  |   |  |
//    \___  /   |__|    \___  >\___  >\______  /|__|  (____  /__|   |__|
//	  \/		    \/	   \/	     \/		   \/
//  ______________________                           ______________________
//			  T H E   W A R   B E G I N S
//	   FreeCraft - A free fantasy real time strategy game engine
//
/**@name wav.c			-	wav support */
//
//	(c) Copyright 2002 by Lutz Sammer and Fabrice Rossi
//
//	FreeCraft is free software; you can redistribute it and/or modify
//	it under the terms of the GNU General Public License as published
//	by the Free Software Foundation; only version 2 of the License.
//
//	FreeCraft is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//	GNU General Public License for more details.
//
//	$Id: wav.c,v 1.7 2002/07/20 00:09:05 johns Exp $

//@{

/*----------------------------------------------------------------------------
--	Includes
----------------------------------------------------------------------------*/

#include "freecraft.h"

#if defined(USE_CDDA) // {

#include "sound_server.h"
#include <stdlib.h>
#include <stdio.h>

global struct cdrom_read_audio data;
local void *bufstart;

/*----------------------------------------------------------------------------
--	Functions
----------------------------------------------------------------------------*/

local int CDRead(Sample *sample, void *buf, int len)
{
    static int pos = 0;
    static int count = 0;

    ++count;

    data.addr.lba = CDtocentry[CDTrack].cdte_addr.lba + pos / 2352;
    data.addr_format = CDROM_LBA;
    data.nframes = 14;

    if (count == 4) {
	data.buf = bufstart;
	ioctl(CDDrive, CDROMREADAUDIO, &data);
    } else if (count == 8) {
	count = 0;
	sample->User = bufstart;
	data.buf = sample->User + 2352 * 14;
	ioctl(CDDrive, CDROMREADAUDIO, &data);
    } 
    
    if (count)
	sample->User += len;

    pos += len;

    memcpy(buf, sample->User, len);

    return len;
}

local void CDFree(Sample *sample)
{
    free(sample);
}

local const SampleType CDStreamSampleType = {
    CDRead,
    CDFree,
};

/**
**	Load CD.
**
**	@param name	Unused.
**	@param flags	Track number.
**
**	@return		Returns the loaded sample.
**
*/
global Sample* LoadCD(const char* name __attribute__((unused)),
	int flags __attribute__((unused)))
{
    Sample* sample;

    sample = malloc(sizeof(*sample));
    sample->Channels = 2;
    sample->SampleSize = 16;
    sample->Frequency = 44100;
    sample->User = malloc(8192 * 10);
    sample->Type = &CDStreamSampleType;
    sample->Length = 0;
    
    bufstart = sample->User;

    return sample;
}

#endif	// } USE_CDDA

//@}
