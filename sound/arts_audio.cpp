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
/**@name arts_audio.c		-	Arts daemon sound support */
//
//	(c) Copyright 2002 by Andreas Arens
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

#if defined(WITH_ARTSC)		// {

/*----------------------------------------------------------------------------
--		Includes
----------------------------------------------------------------------------*/

#include <artsc.h>
#include <stdio.h>
#include "stratagus.h"

#include "sound_server.h"

/*----------------------------------------------------------------------------
--		Variables
----------------------------------------------------------------------------*/

static arts_stream_t stream;

/*----------------------------------------------------------------------------
--		Functions
----------------------------------------------------------------------------*/

/**
**		Initialize Arts daemon part for sound drivers.
**
**		@param freq		Sample frequenz (44100,22050,11025 hz).
**		@param size		Sample size (8bit, 16bit)
**
**		@return				True if failure, false if everything ok.
*/
global int InitArtsSound(int freq, int size)
{
	int err;
	int frags;

	SoundFildes = -1;
	//
	//		Connect to the daemon.
	//
	if ((err = arts_init())) {
		fprintf(stderr, "Sound [arts]: %s\n", arts_error_text(err));
		return 1;
	}

	//
	//		Open daemon stream, size bit samples, stereo.
	//
	stream = arts_play_stream(freq, size, 2, "Stratagus");
	if (stream == NULL) {
		fprintf(stderr, "Sound [arts]: Unable to open a stream\n");
		arts_free();
		return 1;
	}

	//
	// Set the stream to blocking: it will not block anyway, but it seems
	// to be working better
	//
	arts_stream_set(stream, ARTS_P_BLOCKING, 1);

	switch (freq) {
		case 11025:
			frags = ((8 << 16) |  8);   // 8 Buffers of  256 Bytes
			break;
		case 22050:
			frags = ((8 << 16) |  9);   // 8 Buffers of  512 Bytes
			break;
		default:
			DebugLevel0Fn("Unexpected sample frequency %d\n" _C_ freq);
			// FALL THROUGH
		case 44100:
			frags = ((8 << 16) | 10);   // 8 Buffers of 1024 Bytes
			break;
	}
	if (size == 16) {						//  8 bit
		++frags;						// double buffer size
	}
	arts_stream_set(stream, ARTS_P_PACKET_SETTINGS, frags);

#ifdef DEBUG
		frags = arts_stream_get(stream, ARTS_P_BUFFER_SIZE);
		DebugLevel0Fn("frequency %d, buffer size %d\n" _C_ freq _C_ frags);
#endif
	SoundFildes = 0;
	return 0;
}

/**
**		Uninit Arts daemon part for sound drivers.
*/
global void ExitArtsSound(void)
{
	if (SoundFildes == 0) {
		arts_close_stream(stream);
		arts_free();
		SoundFildes = -1;
	}
}

/**
**		Write out sound data to arts daemon.
**
**		@param data		Pointer to data (sample) buffer
**		@param len		length of buffer
**
**		@return				Number of written bytes on success or error code
*/
global int WriteArtsSound(void* data,int len)
{
	return arts_write(stream, data, len);
}

/**
**		Query available sample buffer space from arts daemon.
**
**		@return				Available sample buffer space
*/
global int ArtsGetSpace(void)
{
	return arts_stream_get(stream, ARTS_P_BUFFER_SPACE);
}

#endif		// } WITH_ARTSC

//@}

