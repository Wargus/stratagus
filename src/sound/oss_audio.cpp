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
/**@name oss_audio.c		-	Oss hardware support */
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
//	$Id$

//@{

/*----------------------------------------------------------------------------
--	Includes
----------------------------------------------------------------------------*/

#include <stdio.h>
#include "freecraft.h"

#if defined(WITH_SOUND) && !defined(USE_SDLA)	// {

#include <unistd.h>
#include <fcntl.h>

#include "sound_server.h"

#ifdef __linux__
#   include <sys/ioctl.h>
#   include <linux/soundcard.h>
#else
#   include <sys/ioctl.h>
#   include <machine/soundcard.h>
#endif

/*----------------------------------------------------------------------------
--	Functions
----------------------------------------------------------------------------*/

/**
**	Initialize sound card hardware part for oss compatible drivers.
**
**	@param dev	Device name ("/dev/dsp").
**	@param freq	Sample frequenz (44100,22050,11025 hz).
**	@param size	Sample size (8bit, 16bit)
**	@param wait	Flag, if true wait for sound device to open.
**
**	@return		True if failure, false if everything ok.
*/
global int InitOssSound(const char* dev,int freq,int size,int wait)
{
    int dummy;

    //
    //	Open dsp device, 8bit samples, stereo.
    //
    if( wait ) {
	SoundFildes=open(dev,O_WRONLY);
    } else {
	SoundFildes=open(dev,O_WRONLY|O_NDELAY);
    }
    if( SoundFildes==-1 ) {
	fprintf(stderr,"Can't open audio device `%s'\n",dev);
	return 1;
    }
    dummy=size;
    if( ioctl(SoundFildes,SNDCTL_DSP_SAMPLESIZE,&dummy)==-1 ) {
	perror(__FUNCTION__ " ioctl(SNDCTL_DSP_SAMPLESIZE)");
	close(SoundFildes);
	SoundFildes=-1;
	return 1;
    }
    dummy=1;
    if( ioctl(SoundFildes,SNDCTL_DSP_STEREO,&dummy)==-1 ) {
	perror(__FUNCTION__ " ioctl(SNDCTL_DSP_STEREO)");
	close(SoundFildes);
	SoundFildes=-1;
	return 1;
    }
    dummy=freq;
    if( ioctl(SoundFildes,SNDCTL_DSP_SPEED,&dummy)==-1 ) {
	perror(__FUNCTION__ " ioctl(SNDCTL_DSP_SPEED)");
	close(SoundFildes);
	SoundFildes=-1;
	return 1;
    }
    //
    //	33ms buffer minimum.
    //
    // FIXME: higher speed more buffers!!

    switch( freq ) {
	case 11025:
	    dummy=((8<<16) |  8);   // 8 Buffers of  256 Bytes
	    break;
	case 22050:
	    dummy=((8<<16) |  9);   // 8 Buffers of  512 Bytes
	    break;
	default:
	    DebugLevel0Fn("Unexpected sample frequency %d\n" _C_ freq);
	    // FALL THROUGH
	case 44100:
	    dummy=((8<<16) | 10);   // 8 Buffers of 1024 Bytes
	    break;
    }
    if( size==16 ) {			//  8 bit
	++dummy;			// double buffer size
    }

    DebugLevel0Fn("%d bytes %d ms buffer\n" _C_ freq*size/8 _C_ 
	((dummy>>16)*(1<<(dummy&0xFFFF))*1000)/(freq*size/8));

    if( ioctl(SoundFildes,SNDCTL_DSP_SETFRAGMENT,&dummy)==-1 ) {
	perror(__FUNCTION__);
	close(SoundFildes);
	SoundFildes=-1;
	return 1;
    }

#if 0
    dummy=4;
    if( ioctl(SoundFildes,SNDCTL_DSP_SUBDIVIDE,&dummy)==-1 ) {
	perror(__FUNCTION__ " ioctl(SNDCTL_DSP_SUBDIVIDE)");
	close(SoundFildes);
	SoundFildes=-1;
	return;
    }
#endif

#ifdef DEBUG
    if( ioctl(SoundFildes,SNDCTL_DSP_GETBLKSIZE,&dummy)==-1 ) {
	perror(__FUNCTION__ " ioctl(SNDCTL_DSP_GETBLKSIZE)");
	close(SoundFildes);
	SoundFildes=-1;
	return 1;
    }

    DebugLevel2Fn("DSP block size %d\n" _C_ dummy);
    DebugLevel2Fn("DSP sample speed %d\n" _C_ freq);
#endif

    return 0;
}

#endif	// } WITH_SOUND && !USE_SDLA

//@}
