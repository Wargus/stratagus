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

/*----------------------------------------------------------------------------
--	Functions
----------------------------------------------------------------------------*/

local int CDRead(Sample *sample, void *buf, int len)
{
    int pos;
    
    pos = (int)sample->User;

    data.addr.lba = CDtocentry[CDTrack].cdte_addr;
    data.addr_format = CDROM_LBA;
    data.nframes = 50;
    data.buf = sample->User;
    ioctl(CDDrive, CDROMREADAUDIO, &data);

    memcpy(buf, data.buf + pos, data.nframes * 2352);
    sample->User = (void*)(pos + len * 2352);

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
global Sample* LoadCD(const char* name, int flags)
{
    Sample* sample = NULL;
    int i;

    sample = malloc(sizeof(*sample));
    sample->Channels = 2;
    sample->SampleSize = 16;
    sample->Frequency = 44100;
    sample->Length = 0;
    sample->Type = &CDStreamSampleType;
    sample->User = malloc(50 * 2352);

    data.addr.lba = CDtocentry[flags].cdte_addr.lba;
    data.addr_format = CDROM_LBA;
    data.nframes = 50;
    data.buf = sample->User;
    ioctl(CDDrive, CDROMREADAUDIO, &data);

    return sample;
}

#endif	// } USE_CDDA

//@}
