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
/**@name wince.c	-	WinCE video support. */
//
//	(c) Copyright 2001 by
//
//	FreeCraft is free software; you can redistribute it and/or modify
//	it under the terms of the GNU General Public License as published
//	by the Free Software Foundation; either version 2 of the License,
//	or (at your option) any later version.
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

#include "freecraft.h"

#ifdef USE_WINCE	// {

#include <stdlib.h>

#include "video.h"
#include "font.h"
#include "map.h"
#include "interface.h"
#include "network.h"
#include "ui.h"
#include "sound_server.h"
#include "sound.h"
#include "interface.h"

/*----------------------------------------------------------------------------
--	Declarations
----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
--	Variables
----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
--	Functions
----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
--	Sync
----------------------------------------------------------------------------*/

/**
**	Initialise video sync.
**
**	@note	SDL has only a maximum resolution of 10 ms.
**
**	@see VideoSyncSpeed
*/
global void SetVideoSync(void)
{
    DebugLevel0Fn("%d\n",(100*1000/FRAMES_PER_SECOND)/VideoSyncSpeed);
}

/*----------------------------------------------------------------------------
--	Video
----------------------------------------------------------------------------*/

/**
**	Initialze the video part for WinCE.
*/
global void InitVideoSdl(void)
{
    // Initialize the WinCE library

    // Initialize the display

    if( !VideoWidth ) {
	VideoWidth = DEFAULT_VIDEO_WIDTH;
	VideoHeight = DEFAULT_VIDEO_HEIGHT;
    }

    VideoBpp=16;
    VideoDepth=16;

    DebugLevel3Fn("Video init ready %d %d\n",VideoDepth,VideoBpp);
}

/**
**	Invalidate some area
**
**	@param x	screen pixel X position.
**	@param y	screen pixel Y position.
**	@param w	width of rectangle in pixels.
**	@param h	height of rectangle in pixels.
*/
global void InvalidateArea(int x,int y,int w,int h)
{
}

/**
**	Invalidate whole window
*/
global void Invalidate(void)
{
}

/**
**	Wait for interactive input event for one frame.
**
**	Handles system events, joystick, keyboard, mouse.
**	Handles the network messages.
**	Handles the sound queue.
**
**	All events available are fetched. Sound and network only if available.
**	Returns if the time for one frame is over.
**
**	@param callbacks	Call backs that handle the events.
*/
global void WaitEventsOneFrame(const EventCallback* callbacks)
{
    struct timeval tv;
    fd_set rfds;
    fd_set wfds;
    int maxfd;
    Uint32 i;
    SDL_Event event[1];

    if( SoundFildes==-1 ) {
	SoundOff=1;
    }

    InputMouseTimeout(callbacks,SDL_GetTicks());
    for(;;) {
#if 1
	static Uint32 LastTick;

	//
	//	Time of frame over? This makes the CPU happy. :(
	//
	i=WinCE_GetTicks();
	while( i>=LastTick ) {
	    ++VideoInterrupts;
	    LastTick+=(100*1000/FRAMES_PER_SECOND)/VideoSyncSpeed;
	}
#endif

	//
	//	Prepare select
	//
	maxfd=0;
	tv.tv_sec=tv.tv_usec=0;
	FD_ZERO(&rfds);
	FD_ZERO(&wfds);

	//
	//	Network
	//
	if( NetworkFildes!=-1 ) {
	    if( NetworkFildes>maxfd ) {
		maxfd=NetworkFildes;
	    }
	    FD_SET(NetworkFildes,&rfds);
	}

	//
	//	Sound
	//
	if( !SoundOff && !SoundThreadRunning ) {
	    if( SoundFildes>maxfd ) {
		maxfd=SoundFildes;
	    }
	    FD_SET(SoundFildes,&wfds);
	}

#if 0
	maxfd=select(maxfd+1,&rfds,&wfds,NULL
		,(i=SDL_PollEvent(event)) ? &tv : NULL);
#else
	// QUICK HACK to fix the event/timer problem
	//	The timer code didn't interrupt the select call.
	//	Perhaps I could send a signal to the process
	// Not very nice, but this is the problem if you use other libraries
	// The event handling of SDL is wrong designed = polling only.
	// There is hope on SDL 1.3 which will have this fixed.

	maxfd=select(maxfd+1,&rfds,&wfds,NULL,&tv);
	i=SDL_PollEvent(event);
#endif

	if ( i ) {			// Handle SDL event
	    SdlDoEvent(callbacks,event);
	}

	if( maxfd>0 ) {
	    //
	    //	Sound
	    //
	    if( !SoundOff && !SoundThreadRunning
		    && FD_ISSET(SoundFildes,&wfds) ) {
		callbacks->SoundReady();
	    }

	    //
	    //	Not more input and network in syn and time for frame over
	    //
	    if( !i && NetworkInSync && VideoInterrupts ) {
		break;
	    }

	    //
	    //	Network
	    //
	    if( NetworkFildes!=-1 && FD_ISSET(NetworkFildes,&rfds) ) {
		callbacks->NetworkEvent();
	    }
	}

	//
	//	Not more input and time for frame over: return
	//
	if( !i && VideoInterrupts ) {
	    break;
	}
    }

    //
    //	Prepare return, time for one frame is over.
    //
    VideoInterrupts=0;
}

/**
**	Wait for interactive input event.
**
**	Handles system events, keyboard, mouse.
**	Video interrupt for sync.
**	Network messages.
**	Sound queue.
**
**	We must handle atlast one X11 event
**
**	FIXME:	the initialition could be moved out of the loop
*/
global void WaitEventsAndKeepSync(void)
{
    EventCallback callbacks;

    struct timeval tv;
    fd_set rfds;
    fd_set wfds;
    int maxfd;
    int i;

    SDL_Event event[1];
    static Uint32 LastTick;

    callbacks.ButtonPressed=(void*)HandleButtonDown;
    callbacks.ButtonReleased=(void*)HandleButtonUp;
    callbacks.MouseMoved=(void*)HandleMouseMove;
    callbacks.KeyPressed=HandleKeyDown;
    callbacks.KeyReleased=HandleKeyUp;

    callbacks.NetworkEvent=NetworkEvent;
    callbacks.SoundReady=WriteSound;

    if( SoundFildes==-1 ) {
	SoundOff=1;
    }
    InputMouseTimeout(&callbacks,SDL_GetTicks());
    for(;;) {
	//
	//	Time of frame over? This makes the CPU happy. :(
	//
	i=WinCE_GetTicks();
	while( i>=LastTick ) {
	    ++VideoInterrupts;
	    LastTick+=(100*1000/FRAMES_PER_SECOND)/VideoSyncSpeed;
	}

	//
	//	Prepare select
	//
	maxfd=0;
	tv.tv_sec=tv.tv_usec=0;
	FD_ZERO(&rfds);
	FD_ZERO(&wfds);

	//
	//	Network
	//
	if( NetworkFildes!=-1 ) {
	    if( NetworkFildes>maxfd ) {
		maxfd=NetworkFildes;
	    }
	    FD_SET(NetworkFildes,&rfds);
	    if( !NetworkInSync ) {
		NetworkRecover();	// recover network
	    }
	}

	//
	//	Sound
	//
	if( !SoundOff && !SoundThreadRunning ) {
	    if( SoundFildes>maxfd ) {
		maxfd=SoundFildes;
	    }
	    FD_SET(SoundFildes,&wfds);
	}

#if 0
	maxfd=select(maxfd+1,&rfds,&wfds,NULL
		,(i=SDL_PollEvent(event)) ? &tv : NULL);
#else
	// Not very nice, but this is the problem if you use other libraries
	// The event handling of SDL is wrong designed = polling only.
	// QUICK HACK to fix the event/timer problem
	maxfd=select(maxfd+1,&rfds,&wfds,NULL,&tv);
	i=SDL_PollEvent(event);
#endif

	if ( i ) {			// Handle SDL event
	    SdlDoEvent(&callbacks,event);
	}

	if( maxfd>0 ) {
	    //
	    //	Sound
	    //
	    if( !SoundOff && !SoundThreadRunning
			&& FD_ISSET(SoundFildes,&wfds) ) {
		callbacks.SoundReady();
	    }

	    //
	    //	Network in sync and time for frame over: return
	    //
	    if( !i && NetworkInSync && VideoInterrupts ) {
		break;
	    }

	    //
	    //	Network
	    //
	    if( NetworkFildes!=-1 && FD_ISSET(NetworkFildes,&rfds) ) {
		callbacks.NetworkEvent();
	    }
	}

	//
	//	Network in sync and time for frame over: return
	//
	if( !i && NetworkInSync && VideoInterrupts ) {
	    break;
	}
    }

    //
    //	Prepare return, time for one frame is over.
    //
    VideoInterrupts=0;
}

/**
**	Create a new hardware dependend palette palette.
**
**	@param palette	Hardware independend palette.
**
**	@return		A hardware dependend pixel table.
*/
global VMemType *VideoCreateNewPalette(const Palette * palette)
{
    int i;
    void *pixels;

    if (!Screen) {			// no init
	return NULL;
    }

    switch (VideoBpp) {
	case 8:
	    pixels = malloc(256 * sizeof(VMemType8));
	    break;
	case 15:
	case 16:
	    pixels = malloc(256 * sizeof(VMemType16));
	    break;
	case 24:
	    pixels = malloc(256 * sizeof(VMemType24));
	    break;
	case 32:
	    pixels = malloc(256 * sizeof(VMemType32));
	    break;
	default:
	    DebugLevel0Fn("Unknown depth\n");
	    return NULL;
    }

    //
    //  Convert each palette entry into hardware format.
    //
    for (i = 0; i < 256; ++i) {
	int r;
	int g;
	int b;
	int v;
	char *vp;

	r = (palette[i].r) & 0xFF;
	g = (palette[i].g) & 0xFF;
	b = (palette[i].b) & 0xFF;
	v = r + g + b;

	// Apply global saturation,contrast and brightness
	r = ((((r * 3 - v) * TheUI.Saturation + v * 100)
		* TheUI.Contrast)
	    + TheUI.Brightness * 25600 * 3) / 30000;
	g = ((((g * 3 - v) * TheUI.Saturation + v * 100)
		* TheUI.Contrast)
	    + TheUI.Brightness * 25600 * 3) / 30000;
	b = ((((b * 3 - v) * TheUI.Saturation + v * 100)
		* TheUI.Contrast)
	    + TheUI.Brightness * 25600 * 3) / 30000;

	// Boundings
	r = r < 0 ? 0 : r > 255 ? 255 : r;
	g = g < 0 ? 0 : g > 255 ? 255 : g;
	b = b < 0 ? 0 : b > 255 ? 255 : b;

	// -> Video
	switch (VideoBpp) {
	    case 8:
		((VMemType8 *) pixels)[i] =
		    SDL_MapRGB(Screen->format, r, g, b);
		break;
	    case 15:
	    case 16:
		((VMemType16 *) pixels)[i] =
		    SDL_MapRGB(Screen->format, r, g, b);
		break;
	    case 24:
		v = SDL_MapRGB(Screen->format, r, g, b);
		vp = (char *)(&v);
		((VMemType24 *) pixels)[i].a = vp[0];	// endian safe ?
		((VMemType24 *) pixels)[i].b = vp[1];
		((VMemType24 *) pixels)[i].c = vp[2];
		break;
	    case 32:
		((VMemType32 *) pixels)[i] =
		    SDL_MapRGB(Screen->format, r, g, b);
		break;
	}
    }

    return pixels;
}

/**
**	Check video interrupt.
**
**	Display and count too slow frames.
*/
global void CheckVideoInterrupts(void)
{
    if( VideoInterrupts ) {
        //DebugLevel1("Slow frame\n");
	IfDebug(
	    DrawText(TheUI.MapX+10,TheUI.MapY+10,GameFont,"SLOW FRAME!!");
	);
        ++SlowFrameCounter;
    }
}

/**
**	Realize video memory.
*/
global void RealizeVideoMemory(void)
{
}

/**
**	Toggle grab mouse.
*/
global void ToggleGrabMouse(void)
{
}

#endif // } USE_WINCE

//@}
