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
/**@name movie.c	-	Movie playback functions. */
//
//	(c) Copyright 2002 by Lutz Sammer
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
#include <stdlib.h>

#include "freecraft.h"
#include "video.h"
#include "network.h"
#include "sound.h"
#include "sound_server.h"
#include "avi.h"
#include "movie.h"
#include "iocompat.h"

#include "vfw_PB_Interface.h"
#include "pbdll.h"
extern void VPInitLibrary(void);
extern void VPDeInitLibrary(void);

#ifdef USE_SDL
#include "SDL.h"
#endif

#ifdef USE_SDL				/// Only supported with SDL for now

extern SDL_Surface *Screen;		/// internal screen

/*----------------------------------------------------------------------------
--	Defines
----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
--	Variables
----------------------------------------------------------------------------*/

local char MovieKeyPressed;		/// Flag key is pressed to stop

/*----------------------------------------------------------------------------
--	Functions
----------------------------------------------------------------------------*/

/**
**	Open a movie file.
**
**	@param name	Filename of movie file.
**
**	@return		Avi file handle on success.
*/
global AviFile* MovieOpen(const char* name)
{
    AviFile* avi;
    char buffer[PATH_MAX];

    name = LibraryFileName(name, buffer);
    if (!(avi = AviOpen(name))) {
	return NULL;
    }

    return avi;
}

/**
**	Close a movie file.
**
**	@param avi	Avi file handle.
*/
global void MovieClose(AviFile* avi)
{
    AviClose(avi);
}

/**
**	Display the next movie frame.
**
**	@param avi	Avi file handle.
**	@param pbi	VP3 decoder.
**	@param overlay	Output overlay.
**	@param rect	Destination rectangle.
**
**	@return		True if eof.
*/
global int MovieDisplayFrame(AviFile* avi, PB_INSTANCE* pbi,
    SDL_Overlay* overlay, SDL_Rect rect)
{
    unsigned char *frame;
    int length;
    int i;
    YUV_BUFFER_CONFIG yuv;

    length = AviReadNextVideoFrame(avi, &frame);
    if (length < 0) {			// EOF
	return 1;
    }
    if (length) {
	if (DecodeFrameToYUV(pbi, frame, length, -1, -1)) {
	    fprintf(stderr, "DecodeFrameToYUV error\n");
	    exit(-1);
	}

	SDL_LockYUVOverlay(overlay);
	GetYUVConfig(pbi, &yuv);
	for (i = 0; i < yuv.YHeight; ++i) {	// copy Y
	    memcpy(overlay->pixels[0] + i * overlay->pitches[0],
		yuv.YBuffer + (yuv.YHeight - i - 1) * yuv.YStride, yuv.YWidth);
	}
	for (i = 0; i < yuv.UVHeight; ++i) {	// copy UV
	    memcpy(overlay->pixels[1] + i * overlay->pitches[1],
		yuv.VBuffer + (yuv.UVHeight - i - 1) * yuv.UVStride,
		yuv.UVWidth);
	    memcpy(overlay->pixels[2] + i * overlay->pitches[2],
		yuv.UBuffer + (yuv.UVHeight - i - 1) * yuv.UVStride,
		yuv.UVWidth);
	}
	SDL_UnlockYUVOverlay(overlay);
    }

    SDL_DisplayYUVOverlay(overlay, &rect);

    return 0;
}

/*--------------------------------------------------------------------------*/

/**
**	Callback for input.
*/
local void MovieCallbackKey(unsigned dummy __attribute__((unused)))
{
    MovieKeyPressed=0;
}

/**
**	Callback for input.
*/
local void MovieCallbackKey2(unsigned dummy1 __attribute__((unused)),
	unsigned dummy2 __attribute__((unused)))
{
    MovieKeyPressed=0;
}

/**
**	Callback for input.
*/
local void MovieCallbackKey3(unsigned dummy1 __attribute__((unused)),
	unsigned dummy2 __attribute__((unused)))
{
    MovieKeyPressed=0;
}

/**
**	Callback for input.
*/
local void MovieCallbackMouse(int dummy_x __attribute__((unused)),
	int dummy_y __attribute__((unused)))
{
}

/**
**	Callback for exit.
*/
local void MovieCallbackExit(void)
{
}

/**
**	Play a video file.
**
**	@param name	Filename of movie file.
**	@param flags	Flags for movie playback.
**
**	@return		True if file isn't a supported movie.
**
**	@todo Support full screen and resolution changes.
*/
global int PlayMovie(const char *name, int flags)
{
    AviFile *avi;
    SDL_Overlay *overlay;
    PB_INSTANCE *pbi;
    EventCallback callbacks;
    SDL_Rect rect;

    // FIXME: Should split this into lowlevel parts.

    if (!(avi = MovieOpen(name))) {
	return 1;
    }

    if( avi->AudioStream != -1 ) {	// Only if audio available
	StopMusic();
    }

    //
    //  Prepare video
    //
    if (flags & PlayMovieFullScreen) {
	DebugLevel0Fn("FIXME: full screen switch not supported\n");
    }
    overlay =
	SDL_CreateYUVOverlay(avi->Width, avi->Height, SDL_YV12_OVERLAY,
	Screen);
    if (!overlay) {
	fprintf(stderr, "Couldn't create overlay: %s\n", SDL_GetError());
	exit(1);
    }
    if (overlay->hw_overlay) {
	DebugLevel0Fn("Got a hardware overlay.\n");
    }
    VideoSyncSpeed = avi->FPS100 / FRAMES_PER_SECOND;
    SetVideoSync();

    StartDecoder(&pbi, avi->Width, avi->Height);

#ifdef USE_OGG
    if( avi->AudioStream != -1 ) {	// Only if audio available
	PlayAviOgg(avi);
    }
#endif

    callbacks.ButtonPressed=MovieCallbackKey;
    callbacks.ButtonReleased=MovieCallbackKey;
    callbacks.MouseMoved=MovieCallbackMouse;
    callbacks.MouseExit=MovieCallbackExit;
    callbacks.KeyPressed=MovieCallbackKey2;
    callbacks.KeyReleased=MovieCallbackKey2;
    callbacks.KeyRepeated=MovieCallbackKey3;
    callbacks.NetworkEvent=NetworkEvent;
    callbacks.SoundReady=WriteSound;

    if (flags & PlayMovieZoomScreen) {
	rect.x = 0;
	rect.y = 0;
	rect.w = VideoWidth;
	rect.h = VideoHeight;
    } else {
	rect.x = (VideoWidth - avi->Width) / 2;
	rect.y = (VideoHeight - avi->Height) / 2;
	rect.w = avi->Width;
	rect.h = avi->Height;
    }

    MovieKeyPressed = 1;
    while (MovieKeyPressed) {

	if (MovieDisplayFrame(avi, pbi, overlay, rect)) {
	    break;
	}

	WaitEventsOneFrame(&callbacks);
    }

    if( avi->AudioStream != -1 ) {	// Only if audio available
	StopMusic();
    }

    StopDecoder(&pbi);
    SDL_FreeYUVOverlay(overlay);

    MovieClose(avi);

    return 0;
}

#else

/**
**	Play a video file.
**
**	@param file	Filename of movie file.
**	@param flags	Flags for movie playback.
**
**	@return		True if file isn't a supported movie.
**
**	@todo Support full screen and resolution changes.
*/
global int PlayMovie(const char* file, int flags)
{
    printf("FIXME: PlayMovie(\"%s\",%x) not supported.\n", file, flags);

    return 1;
}

#endif

/**
**	Initialize the movie module.
*/
global void InitMovie(void)
{
    VPInitLibrary();
}

/**
**	Cleanup the movie module.
*/
global void CleanMovie(void)
{
    VPDeInitLibrary();
}

//@}
