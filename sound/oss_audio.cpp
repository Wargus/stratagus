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
/**@name oss_audio.c		-	Oss hardware support */
//
//	(c) Copyright 2002-2003 by Lutz Sammer and Fabrice Rossi
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
#include <string.h>
#include "stratagus.h"

#if defined(WITH_SOUND) && !defined(USE_SDLA)		// {

#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

#include "sound_server.h"

#ifdef __linux__
#   include <sys/ioctl.h>
#   include <linux/soundcard.h>
#else
#   include <sys/ioctl.h>
#   include <machine/soundcard.h>
#endif

/*----------------------------------------------------------------------------
--		Functions
----------------------------------------------------------------------------*/

/**
**		Initialize sound card hardware part for oss compatible drivers.
**
**		@param dev		Device name ("/dev/dsp").
**		@param freq		Sample frequenz (44100,22050,11025 hz).
**		@param size		Sample size (8bit, 16bit)
**		@param wait		Flag, if true wait for sound device to open.
**
**		@return				True if failure, false if everything ok.
*/
global int InitOssSound(const char* dev, int freq, int size, int wait)
{
	int dummy;

	//
	//		Open dsp device, 8bit samples, stereo.
	//
	if (wait) {
		SoundFildes = open(dev, O_WRONLY);
	} else {
		SoundFildes = open(dev, O_WRONLY | O_NDELAY);
	}
	if (SoundFildes == -1) {
		fprintf(stderr, "Can't open audio device `%s'\n", dev);
		return 1;
	}
	dummy = size;
	if (ioctl(SoundFildes, SNDCTL_DSP_SAMPLESIZE, &dummy) == -1) {
		PrintFunction();
		fprintf(stdout, "%s - ioctl(SNDCTL_DSP_SAMPLESIZE)", strerror(errno));
		close(SoundFildes);
		SoundFildes = -1;
		return 1;
	}
	dummy = 1;
	if (ioctl(SoundFildes, SNDCTL_DSP_STEREO, &dummy) == -1) {
		PrintFunction();
		fprintf(stdout, "%s - ioctl(SNDCTL_DSP_STEREO)", strerror(errno));
		close(SoundFildes);
		SoundFildes = -1;
		return 1;
	}
	dummy = freq;
	if (ioctl(SoundFildes, SNDCTL_DSP_SPEED, &dummy) == -1) {
		PrintFunction();
		fprintf(stdout, "%s - ioctl(SNDCTL_DSP_SPEED)", strerror(errno));
		close(SoundFildes);
		SoundFildes = -1;
		return 1;
	}
	//
	//		33ms buffer minimum.
	//
	// FIXME: higher speed more buffers!!

	switch (freq) {
		case 11025:
			dummy=((8 << 16) |  8);   // 8 Buffers of  256 Bytes
			break;
		case 22050:
			dummy=((8 << 16) |  9);   // 8 Buffers of  512 Bytes
			break;
		default:
			DebugLevel0Fn("Unexpected sample frequency %d\n" _C_ freq);
			// FALL THROUGH
		case 44100:
			dummy=((8 << 16) | 10);   // 8 Buffers of 1024 Bytes
			break;
	}
	if (size == 16) {						//  8 bit
		++dummy;						// double buffer size
	}

	DebugLevel0Fn("%d bytes %d ms buffer\n" _C_ freq*size/8 _C_
		((dummy >> 16) * (1 << (dummy & 0xFFFF)) * 1000) / (freq * size / 8));

	if (ioctl(SoundFildes, SNDCTL_DSP_SETFRAGMENT, &dummy) == -1) {
		PrintFunction();
		fprintf(stdout, "%s - ioctl(SNDCTL_DSP_SETFRAGMENT)", strerror(errno));
		close(SoundFildes);
		SoundFildes = -1;
		return 1;
	}

#if 0
	dummy = 4;
	if (ioctl(SoundFildes, SNDCTL_DSP_SUBDIVIDE, &dummy) == -1) {
		PrintFunction();
		fprintf(stdout, "%s - ioctl(SNDCTL_DSP_SUBDIVIDE)", strerror(errno));
		close(SoundFildes);
		SoundFildes = -1;
		return;
	}
#endif

#ifdef DEBUG
	if (ioctl(SoundFildes, SNDCTL_DSP_GETBLKSIZE, &dummy) == -1) {
		PrintFunction();
		fprintf(stdout, "%s - ioctl(SNDCTL_DSP_GETBLKSIZE)", strerror(errno));
		close(SoundFildes);
		SoundFildes = -1;
		return 1;
	}

	DebugLevel2Fn("DSP block size %d\n" _C_ dummy);
	DebugLevel2Fn("DSP sample speed %d\n" _C_ freq);
#endif

	return 0;
}

#endif		// } WITH_SOUND && !USE_SDLA

//@}
