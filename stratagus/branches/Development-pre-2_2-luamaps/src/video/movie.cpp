//       _________ __                 __
//      /   _____//  |_____________ _/  |______     ____  __ __  ______
//      \_____  \\   __\_  __ \__  \\   __\__  \   / ___\|  |  \/  ___/
//      /        \|  |  |  | \// __ \|  |  / __ \_/ /_/  >  |  /\___ |
//     /_______  /|__|  |__|  (____  /__| (____  /\___  /|____//____  >
//             \/                  \/          \//_____/            \/
//  ______________________                           ______________________
// T H E   W A R   B E G I N S
// Stratagus - A free fantasy real time strategy game engine
//
/**@name movie.c - Movie playback functions. */
//
// (c) Copyright 2002-2004 by Lutz Sammer
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
// $Id$

//@{

/*----------------------------------------------------------------------------
-- Includes
----------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>

#include "stratagus.h"
#include "video.h"
#include "network.h"
#include "sound.h"
#include "sound_server.h"
#include "avi.h"
#include "movie.h"
#include "iocompat.h"
#include "iolib.h"

#include "vfw_PB_Interface.h"
#include "pbdll.h"
extern void VPInitLibrary(void);
extern void VPDeInitLibrary(void);

#ifdef USE_SDL
#include "SDL.h"
#endif

#if defined(USE_SDL) && !defined(USE_OPENGL)  /// Only supported with SDL for now

extern SDL_Surface* TheScreen; ///< internal screen

/*----------------------------------------------------------------------------
--  Defines
----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
--  Variables
----------------------------------------------------------------------------*/

static char MovieKeyPressed;         /// Flag key is pressed to stop
static int MovieInitialized;         /// Movie module initialized

/*----------------------------------------------------------------------------
--  Functions
----------------------------------------------------------------------------*/

/**
**  Open a movie file.
**
**  @param name  Filename of movie file.
**
**  @return      Avi file handle on success.
*/
AviFile* MovieOpen(const char* name)
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
**  Close a movie file.
**
**  @param avi  Avi file handle.
*/
void MovieClose(AviFile* avi)
{
	AviClose(avi);
}

/**
**  Display the next movie frame.
**
**  @param avi      Avi file handle.
**  @param pbi      VP3 decoder.
**  @param overlay  Output overlay.
**  @param rect     Destination rectangle.
**
**  @return         True if eof.
*/
int MovieDisplayFrame(AviFile* avi, PB_INSTANCE* pbi,
	SDL_Overlay* overlay, SDL_Rect rect)
{
	unsigned char* frame;
	int length;
	int i;
	YUV_BUFFER_CONFIG yuv;

	length = AviReadNextVideoFrame(avi, &frame);
	if (length < 0) {  // EOF
		return 1;
	}
	if (length) {
		if (DecodeFrameToYUV(pbi, frame, length, -1, -1)) {
			fprintf(stderr, "DecodeFrameToYUV error\n");
			exit(-1);
		}

		SDL_LockYUVOverlay(overlay);
		GetYUVConfig(pbi, &yuv);
		for (i = 0; i < yuv.YHeight; ++i) {  // copy Y
			memcpy(overlay->pixels[0] + i * overlay->pitches[0],
				yuv.YBuffer + (yuv.YHeight - i - 1) * yuv.YStride, yuv.YWidth);
		}
		for (i = 0; i < yuv.UVHeight; ++i) {  // copy UV
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
**  Callback for input.
*/
static void MovieCallbackKey(unsigned dummy __attribute__((unused)))
{
	MovieKeyPressed = 0;
}

/**
**  Callback for input.
*/
static void MovieCallbackKey1(unsigned dummy __attribute__((unused)))
{
}

/**
**  Callback for input.
*/
static void MovieCallbackKey2(unsigned dummy1 __attribute__((unused)),
	unsigned dummy2 __attribute__((unused)))
{
	MovieKeyPressed = 0;
}

/**
**  Callback for input.
*/
static void MovieCallbackKey3(unsigned dummy1 __attribute__((unused)),
	unsigned dummy2 __attribute__((unused)))
{
}

/**
**  Callback for input.
*/
static void MovieCallbackKey4(unsigned dummy1 __attribute__((unused)),
	unsigned dummy2 __attribute__((unused)))
{
	MovieKeyPressed = 0;
}

/**
**  Callback for input.
*/
static void MovieCallbackMouse(int dummy_x __attribute__((unused)),
	int dummy_y __attribute__((unused)))
{
}

/**
** Callback for exit.
*/
static void MovieCallbackExit(void)
{
}

/**
**  Play a video file.
**
**  @param name   Filename of movie file.
**  @param flags  Flags for movie playback.
**
**  @return       True if file isn't a supported movie.
**
**  @todo Support full screen and resolution changes.
*/
int PlayMovie(const char* name, int flags)
{
	AviFile* avi;
	SDL_Overlay* overlay;
	PB_INSTANCE* pbi;
	EventCallback callbacks;
	SDL_Rect rect;
	int oldspeed;

	InitMovie();

	// FIXME: Should split this into lowlevel parts.

	if (!(avi = MovieOpen(name))) {
		return 1;
	}

	if (avi->AudioStream != -1) {  // Only if audio available
		StopMusic();
	}

	//
	//  Prepare video
	//
	if (flags & PlayMovieFullScreen) {
		DebugPrint("FIXME: full screen switch not supported\n");
	}
	overlay = SDL_CreateYUVOverlay(avi->Width, avi->Height,
		SDL_YV12_OVERLAY, TheScreen);
	if (!overlay) {
		fprintf(stderr, "Couldn't create overlay: %s\n", SDL_GetError());
		exit(1);
	}
	if (overlay->hw_overlay) {
		DebugPrint("Got a hardware overlay.\n");
	}
	oldspeed = VideoSyncSpeed;
	VideoSyncSpeed = avi->FPS100 / FRAMES_PER_SECOND;
	SetVideoSync();

	StartDecoder(&pbi, avi->Width, avi->Height);

#ifdef USE_OGG
	if (avi->AudioStream != -1) {  // Only if audio available
		PlayAviOgg(avi);
	}
#endif

	callbacks.ButtonPressed = MovieCallbackKey;
	callbacks.ButtonReleased = MovieCallbackKey1;
	callbacks.MouseMoved = MovieCallbackMouse;
	callbacks.MouseExit = MovieCallbackExit;
	callbacks.KeyPressed = MovieCallbackKey2;
	callbacks.KeyReleased = MovieCallbackKey3;
	callbacks.KeyRepeated = MovieCallbackKey4;
	callbacks.NetworkEvent = NetworkEvent;

	if (flags & PlayMovieZoomScreen) {
		if (flags & PlayMovieKeepAspect) {
			int wa;
			int ha;

			wa = VideoWidth * 100 / avi->Width;
			ha = VideoHeight * 100 / avi->Height;
			DebugPrint(" %d x %d\n" _C_ wa _C_ ha);
			if (wa < ha) {  // Keep the aspect ratio
				rect.w = VideoWidth;
				rect.h = avi->Height * wa / 100;
			} else {
				rect.w = avi->Width * ha / 100;
				rect.h = VideoHeight;
			}
		} else {  // Just zoom to max
			rect.w = VideoWidth;
			rect.h = VideoHeight;
		}
	} else {
		rect.w = avi->Width;
		rect.h = avi->Height;
	}
	rect.x = (VideoWidth - rect.w) / 2;
	rect.y = (VideoHeight - rect.h) / 2;

	if (1) {
		int i;

		GetPbParam(pbi, PBC_SET_POSTPROC, &i);
		DebugPrint("Postprocess level %d\n" _C_ i);
		SetPbParam(pbi, PBC_SET_POSTPROC, 6);
	}

	MovieKeyPressed = 1;
	while (MovieKeyPressed) {

		if (MovieDisplayFrame(avi, pbi, overlay, rect)) {
			break;
		}

		WaitEventsOneFrame(&callbacks);
	}

	if (avi->AudioStream != -1) {  // Only if audio available
		StopMusic();
	}

	StopDecoder(&pbi);
	SDL_FreeYUVOverlay(overlay);

	MovieClose(avi);

	VideoSyncSpeed = oldspeed;
	SetVideoSync();

	return 0;
}

/**
**  Initialize the movie module.
*/
void InitMovie(void)
{
	if (!MovieInitialized) {
		VPInitLibrary();
		MovieInitialized = 1;
	}
}

/**
**  Cleanup the movie module.
*/
void CleanMovie(void)
{
	if (MovieInitialized) {
		VPDeInitLibrary();
		MovieInitialized = 0;
	}
}

#else

/**
**  Play a video file.
**
**  @param file   Filename of movie file.
**  @param flags  Flags for movie playback.
**
**  @return       True if file isn't a supported movie.
**
**  @todo Support full screen and resolution changes.
*/
int PlayMovie(const char* file, int flags)
{
	printf("FIXME: PlayMovie(\"%s\",%x) not supported.\n", file, flags);

	if (strcasestr(file, ".avi\0")) {
		return 0;
	}
	return 1;
}

/**
**  Initialize the movie module.
*/
void InitMovie(void)
{
}

/**
**  Cleanup the movie module.
*/
void CleanMovie(void)
{
}

#endif

//@}
