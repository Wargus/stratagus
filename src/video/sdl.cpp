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
/**@name sdl.c - SDL video support. */
//
//      (c) Copyright 1999-2004 by Lutz Sammer, Jimmy Salmon, Nehal Mistry
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
//      $Id$

//@{

/*----------------------------------------------------------------------------
--		Includes
----------------------------------------------------------------------------*/

#include "stratagus.h"

#ifdef USE_SDL		// {

#ifdef DEBUG
#include <signal.h>
#endif

#include <stdlib.h>
#include <string.h>

#include <limits.h>
#ifndef _MSC_VER
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#endif
#include "SDL.h"
#ifdef USE_OPENGL
#define DrawIcon WinDrawIcon
#define EndMenu WinEndMenu
#include "SDL_opengl.h"
#undef EndMenu
#undef DrawIcon
#endif

#ifdef USE_BEOS
#include <sys/socket.h>
#endif

#ifdef USE_WIN32
#include "net_lowlevel.h"
#endif

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
--  Declarations
----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
--  Variables
----------------------------------------------------------------------------*/

global SDL_Surface* TheScreen;				/// Internal screen

local SDL_Rect Rects[100];
local int NumRects;

local int FrameTicks;						/// Frame length in ms
local int FrameRemainder;				/// Frame remainder 0.1 ms
local int FrameFraction;				/// Frame fractional term

/*----------------------------------------------------------------------------
--  Functions
----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
--  Sync
----------------------------------------------------------------------------*/

/**
**  Initialise video sync.
**  Calculate the length of video frame and any simulation skips.
**
**  @see VideoSyncSpeed @see SkipFrames @see FrameTicks @see FrameRemainder
*/
global void SetVideoSync(void)
{
	int ms;

	if (VideoSyncSpeed) {
		ms = (1000 * 1000 / CYCLES_PER_SECOND) / VideoSyncSpeed;
	} else {
		ms = INT_MAX;
	}
	SkipFrames = ms / 400;
	while (SkipFrames && ms / SkipFrames < 200) {
		--SkipFrames;
	}
	ms /= SkipFrames + 1;

	FrameTicks = ms / 10;
	FrameRemainder = ms % 10;
	DebugPrint("frames %d - %d.%dms\n" _C_ SkipFrames _C_ ms / 10 _C_ ms % 10);
}

/*----------------------------------------------------------------------------
--  Video
----------------------------------------------------------------------------*/

#ifdef USE_OPENGL
/**
**  Initialize open gl for doing 2d with 3d.
*/
local void InitOpenGL(void)
{
	glViewport(0, 0, (GLsizei)VideoWidth, (GLsizei)VideoHeight);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, VideoWidth, VideoHeight, 0, -1, 1);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glTranslatef(0.375, 0.375, 0.);

	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClearDepth(1.0f);
	glShadeModel(GL_FLAT);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_BLEND);
	glEnable(GL_TEXTURE_2D);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}
#endif

#ifdef DEBUG
local void CleanExit(int signum)
{
	// Clean SDL
	SDL_Quit();
	// Reestablish normal behaviour for next abort call
	signal(SIGABRT, SIG_DFL);
	// Generates a core dump
	abort();
}
#endif


/**
**  Initialize the video part for SDL.
*/
global void InitVideoSdl(void)
{
	Uint32 flags;

	if (SDL_WasInit(SDL_INIT_VIDEO) == 0) {
		if (SDL_Init(
#ifdef DEBUG
				SDL_INIT_NOPARACHUTE |
#endif
				SDL_INIT_AUDIO | SDL_INIT_VIDEO |
				SDL_INIT_TIMER) < 0 ) {
			fprintf(stderr, "Couldn't initialize SDL: %s\n", SDL_GetError());
			exit(1);
		}

		// Clean up on exit

		atexit(SDL_Quit);

		// If debug is enabled, Stratagus disable SDL Parachute.
		// So we need gracefully handle segfaults and aborts.
#if defined(DEBUG) && !defined(USE_WIN32)
		signal(SIGSEGV, CleanExit);
		signal(SIGABRT, CleanExit);
#endif
		// Set WindowManager Title
		SDL_WM_SetCaption("Stratagus", "Stratagus");
	}

	// Initialize the display

	if (!VideoWidth) {
		VideoWidth = DEFAULT_VIDEO_WIDTH;
		VideoHeight = DEFAULT_VIDEO_HEIGHT;
	}

	flags = 0;
	// Sam said: better for windows.
	/* SDL_HWSURFACE|SDL_HWPALETTE | */
	if (VideoFullScreen) {
		flags |= SDL_FULLSCREEN;
	}
#ifdef USE_OPENGL
	flags |= SDL_OPENGL;
#endif

	TheScreen = SDL_SetVideoMode(VideoWidth, VideoHeight, VideoDepth, flags);
	if (TheScreen && (TheScreen->format->BitsPerPixel != 16 &&
			TheScreen->format->BitsPerPixel != 32)) {
		// Only support 16 and 32 bpp, default to 16
		TheScreen = SDL_SetVideoMode(VideoWidth, VideoHeight, 16, flags);
	}
	if (TheScreen == NULL) {
		fprintf(stderr, "Couldn't set %dx%dx%d video mode: %s\n",
			VideoWidth, VideoHeight, VideoDepth, SDL_GetError());
		exit(1);
	}

	VideoFullScreen = (TheScreen->flags & SDL_FULLSCREEN) ? 1 : 0;
	VideoDepth = TheScreen->format->BitsPerPixel;

	// Turn cursor off, we use our own.
	SDL_ShowCursor(0);

	// Make default character translation easier
	SDL_EnableUNICODE(1);

#ifdef USE_OPENGL
	InitOpenGL();
#endif

	ColorBlack = VideoMapRGB(TheScreen->format, 0, 0, 0);
	ColorDarkGreen = VideoMapRGB(TheScreen->format, 48, 100, 4);
	ColorBlue = VideoMapRGB(TheScreen->format, 0, 0, 252);
	ColorOrange = VideoMapRGB(TheScreen->format, 248, 140, 20);
	ColorWhite = VideoMapRGB(TheScreen->format, 252, 248, 240);
	ColorGray = VideoMapRGB(TheScreen->format, 128, 128, 128);
	ColorRed = VideoMapRGB(TheScreen->format, 252, 0, 0);
	ColorGreen = VideoMapRGB(TheScreen->format, 0, 252, 0);
	ColorYellow = VideoMapRGB(TheScreen->format, 252, 252, 0);

	TheUI.MouseWarpX = TheUI.MouseWarpY = -1;
}

/**
**  Invalidate some area
**
**  @param x  screen pixel X position.
**  @param y  screen pixel Y position.
**  @param w  width of rectangle in pixels.
**  @param h  height of rectangle in pixels.
*/
global void InvalidateArea(int x, int y, int w, int h)
{
#ifndef USE_OPENGL
	Assert(NumRects != sizeof(Rects) / sizeof(*Rects));
	Assert(x >= 0 && y >= 0 && x + w <= VideoWidth && y + h <= VideoHeight);
	Rects[NumRects].x = x;
	Rects[NumRects].y = y;
	Rects[NumRects].w = w;
	Rects[NumRects].h = h;
	++NumRects;
#endif
}

/**
**  Invalidate whole window
*/
global void Invalidate(void)
{
#ifndef USE_OPENGL
	Rects[0].x = 0;
	Rects[0].y = 0;
	Rects[0].w = VideoWidth;
	Rects[0].h = VideoHeight;
	NumRects = 1;
#endif
}

/**
**  Convert SDL keysym into internal keycode.
**
**  @param code     SDL keysym structure pointer.
**  @param keychar  Internal keycode.
**
**  @return         ASCII code or internal keycode.
*/
local int Sdl2InternalKeycode(const SDL_keysym* code, int* keychar)
{
	int icode;

	//
	//  Convert SDL keycodes into internal keycodes.
	//
	*keychar = 0;
	switch ((icode = code->sym)) {
		case SDLK_ESCAPE:
			*keychar = icode = '\033';
			break;
		case SDLK_RETURN:
		case SDLK_KP_ENTER:
			*keychar = icode = '\r';
			break;
		case SDLK_BACKSPACE:
			*keychar = icode = '\b';
			break;
		case SDLK_TAB:
			*keychar = icode = '\t';
			break;
		case SDLK_UP:
			icode = KeyCodeUp;
			break;
		case SDLK_DOWN:
			icode = KeyCodeDown;
			break;
		case SDLK_LEFT:
			icode = KeyCodeLeft;
			break;
		case SDLK_RIGHT:
			icode = KeyCodeRight;
			break;
		case SDLK_PAUSE:
			icode = KeyCodePause;
			break;
		case SDLK_F1:
			icode = KeyCodeF1;
			break;
		case SDLK_F2:
			icode = KeyCodeF2;
			break;
		case SDLK_F3:
			icode = KeyCodeF3;
			break;
		case SDLK_F4:
			icode = KeyCodeF4;
			break;
		case SDLK_F5:
			icode = KeyCodeF5;
			break;
		case SDLK_F6:
			icode = KeyCodeF6;
			break;
		case SDLK_F7:
			icode = KeyCodeF7;
			break;
		case SDLK_F8:
			icode = KeyCodeF8;
			break;
		case SDLK_F9:
			icode = KeyCodeF9;
			break;
		case SDLK_F10:
			icode = KeyCodeF10;
			break;
		case SDLK_F11:
			icode = KeyCodeF11;
			break;
		case SDLK_F12:
			icode = KeyCodeF12;
			break;
		case SDLK_KP0:
			icode = KeyCodeKP0;
			break;
		case SDLK_KP1:
			icode = KeyCodeKP1;
			break;
		case SDLK_KP2:
			icode = KeyCodeKP2;
			break;
		case SDLK_KP3:
			icode = KeyCodeKP3;
			break;
		case SDLK_KP4:
			icode = KeyCodeKP4;
			break;
		case SDLK_KP5:
			icode = KeyCodeKP5;
			break;
		case SDLK_KP6:
			icode = KeyCodeKP6;
			break;
		case SDLK_KP7:
			icode = KeyCodeKP7;
			break;
		case SDLK_KP8:
			icode = KeyCodeKP8;
			break;
		case SDLK_KP9:
			icode = KeyCodeKP9;
			break;
		case SDLK_KP_PLUS:
			icode = KeyCodeKPPlus;
			break;
		case SDLK_KP_MINUS:
			icode = KeyCodeKPMinus;
			break;
		case SDLK_KP_PERIOD:
			icode = KeyCodeKPPeriod;
			break;
		case SDLK_SYSREQ:
		case SDLK_PRINT:
			icode = KeyCodePrint;
			break;
		case SDLK_DELETE:
			icode = KeyCodeDelete;
			break;

			// We need these because if you only hit a modifier key,
			// the *ots from SDL don't report correct modifiers
		case SDLK_LSHIFT:
		case SDLK_RSHIFT:
			icode = KeyCodeShift;
			break;
		case SDLK_LCTRL:
		case SDLK_RCTRL:
			icode = KeyCodeControl;
			break;
		case SDLK_LALT:
		case SDLK_RALT:
		case SDLK_LMETA:
		case SDLK_RMETA:
			icode = KeyCodeAlt;
			break;
		case SDLK_LSUPER:
		case SDLK_RSUPER:
			icode = KeyCodeSuper;
			break;
		default:
			if ((code->unicode & 0xFF80) == 0) {
				*keychar = code->unicode & 0x7F;
			} else {
				// An international character..
				// let's asume latin 1 for now
				*keychar = code->unicode & 0xFF;
			}
			break;
	}

	return icode;
}

/**
**  Handle keyboard key press!
**
**  @param callbacks  Callback funktion for key down.
**  @param code       SDL keysym structure pointer.
*/
local void SdlHandleKeyPress(const EventCallback* callbacks,
	const SDL_keysym* code)
{
	int icode;
	int keychar;

	icode = Sdl2InternalKeycode(code, &keychar);
	InputKeyButtonPress(callbacks, SDL_GetTicks(), icode, keychar);
}

/**
**  Handle keyboard key release!
**
**  @param callbacks  Callback funktion for key up.
**  @param code       SDL keysym structure pointer.
*/
local void SdlHandleKeyRelease(const EventCallback* callbacks,
	const SDL_keysym* code)
{
	int icode;
	int keychar;

	icode = Sdl2InternalKeycode(code, &keychar);
	InputKeyButtonRelease(callbacks, SDL_GetTicks(), icode, keychar);
}

/**
**  Handle interactive input event.
**
**  @param callbacks  Callback structure for events.
**  @param event      SDL event structure pointer.
*/
local void SdlDoEvent(const EventCallback* callbacks, const SDL_Event* event)
{
	switch (event->type) {
		case SDL_MOUSEBUTTONDOWN:
			//
			//  SDL has already a good order of the buttons.
			//
			InputMouseButtonPress(callbacks, SDL_GetTicks(),
				event->button.button);
			break;

		case SDL_MOUSEBUTTONUP:
			//
			//  SDL has already a good order of the buttons.
			//
			InputMouseButtonRelease(callbacks, SDL_GetTicks(),
				event->button.button);
			break;

			// FIXME: check if this is only useful for the cursor
			// FIXME: if this is the case we don't need this.
		case SDL_MOUSEMOTION:
			InputMouseMove(callbacks, SDL_GetTicks(),
				event->motion.x, event->motion.y);
			// FIXME: Same bug fix from X11
			if ((TheUI.MouseWarpX != -1 || TheUI.MouseWarpY != -1) &&
					(event->motion.x != TheUI.MouseWarpX ||
						event->motion.y != TheUI.MouseWarpY)) {
				int xw;
				int yw;

				xw = TheUI.MouseWarpX;
				yw = TheUI.MouseWarpY;
				TheUI.MouseWarpX = -1;
				TheUI.MouseWarpY = -1;
				SDL_WarpMouse(xw, yw);
			}
			break;

		case SDL_ACTIVEEVENT:
			if (event->active.state & SDL_APPMOUSEFOCUS) {
				static int InMainWindow = 1;

				if (InMainWindow && !event->active.gain) {
					InputMouseExit(callbacks, SDL_GetTicks());
				}
				InMainWindow = event->active.gain;
			}
			if (event->active.state & SDL_APPACTIVE) {
				static int IsVisible = 1;

				if (IsVisible && !event->active.gain) {
					IsVisible = 0;
					UiTogglePause();
				} else if (!IsVisible && event->active.gain) {
					IsVisible = 1;
					UiTogglePause();
				}
			}
			break;

		case SDL_KEYDOWN:
			SdlHandleKeyPress(callbacks, &event->key.keysym);
			break;

		case SDL_KEYUP:
			SdlHandleKeyRelease(callbacks, &event->key.keysym);
			break;

		case SDL_QUIT:
			Exit(0);
	}
}

/**
**		Wait for interactive input event for one frame.
**
**		Handles system events, joystick, keyboard, mouse.
**		Handles the network messages.
**		Handles the sound queue.
**
**		All events available are fetched. Sound and network only if available.
**		Returns if the time for one frame is over.
**
**		@param callbacks		Call backs that handle the events.
**
**		FIXME:		the initialition could be moved out of the loop
*/
global void WaitEventsOneFrame(const EventCallback* callbacks)
{
	struct timeval tv;
	fd_set rfds;
	fd_set wfds;
	Socket maxfd;
	int i;
	int s;
	SDL_Event event[1];
	Uint32 ticks;
	int interrupts;

	if (!++FrameCounter) {
		// FIXME: tests with frame counter now fails :(
		// FIXME: Should happen in 68 years :)
		fprintf(stderr, "FIXME: *** round robin ***\n");
		fprintf(stderr, "FIXME: *** round robin ***\n");
		fprintf(stderr, "FIXME: *** round robin ***\n");
		fprintf(stderr, "FIXME: *** round robin ***\n");
	}

	ticks = SDL_GetTicks();
	if (ticks > NextFrameTicks) {		// We are too slow :(
#ifdef DEBUG
		// FIXME: need locking!
		// if (InterfaceState == IfaceStateNormal) {
		// VideoDrawText(TheUI.MapX+10,TheUI.MapY+10,GameFont,"SLOW FRAME!!");
		// }
#endif
		++SlowFrameCounter;
	}

	InputMouseTimeout(callbacks, ticks);
	InputKeyTimeout(callbacks, ticks);
	CursorAnimate(ticks);

	interrupts = 0;

	for (;;) {
		//
		//		Time of frame over? This makes the CPU happy. :(
		//
		ticks = SDL_GetTicks();
		if (!interrupts && ticks + 11 < NextFrameTicks) {
			SDL_Delay(10);
		}
		while (ticks >= NextFrameTicks) {
			++interrupts;
			FrameFraction += FrameRemainder;
			if (FrameFraction > 10) {
				FrameFraction -= 10;
				++NextFrameTicks;
			}
			NextFrameTicks += FrameTicks;
		}

		//
		//		Prepare select
		//
		maxfd = 0;
		tv.tv_sec = tv.tv_usec = 0;
		FD_ZERO(&rfds);
		FD_ZERO(&wfds);

		//
		//		Network
		//
		if (IsNetworkGame()) {
			if (NetworkFildes > maxfd) {
				maxfd = NetworkFildes;
			}
			FD_SET(NetworkFildes, &rfds);
		}

#if 0
		s = select(maxfd + 1, &rfds, &wfds, NULL,
			(i = SDL_PollEvent(event)) ? &tv : NULL);
#else
		// QUICK HACK to fix the event/timer problem
		//		The timer code didn't interrupt the select call.
		//		Perhaps I could send a signal to the process
		// Not very nice, but this is the problem if you use other libraries
		// The event handling of SDL is wrong designed = polling only.
		// There is hope on SDL 1.3 which will have this fixed.

		s = select(maxfd + 1, &rfds, &wfds, NULL, &tv);
		i = SDL_PollEvent(event);
#endif

		if (i) {						// Handle SDL event
			SdlDoEvent(callbacks, event);
		}

		if (s > 0) {
			//
			//		Network
			//
			if (IsNetworkGame() && FD_ISSET(NetworkFildes, &rfds) ) {
				callbacks->NetworkEvent();
			}
		}

		//
		//		No more input and time for frame over: return
		//
		if (!i && s <= 0 && interrupts) {
			break;
		}
	}

	if (!SkipGameCycle--) {
		SkipGameCycle = SkipFrames;
	}

#ifndef USE_OPENGL
	VideoClearScreen();
#endif
}

/**
**  Realize video memory.
*/
global void RealizeVideoMemory(void)
{
#ifdef USE_OPENGL
	SDL_GL_SwapBuffers();
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
#else
	if (NumRects) {
		SDL_UpdateRects(TheScreen, NumRects, Rects);
		NumRects = 0;
	}
#endif
}

/**
**  Lock the screen for write access.
*/
global void SdlLockScreen(void)
{
#ifndef USE_OPENGL
	if (SDL_MUSTLOCK(TheScreen)) {
		SDL_LockSurface(TheScreen);
	}
#endif
}

/**
**  Unlock the screen for write access.
*/
global void SdlUnlockScreen(void)
{
#ifndef USE_OPENGL
	if (SDL_MUSTLOCK(TheScreen)) {
		SDL_UnlockSurface(TheScreen);
	}
#endif
}

/**
**  Toggle grab mouse.
**
**  @param mode  Wanted mode, 1 grab, -1 not grab, 0 toggle.
*/
global void ToggleGrabMouse(int mode)
{
	static int grabbed;

	if (mode <= 0 && grabbed) {
		SDL_WM_GrabInput(SDL_GRAB_OFF);
		grabbed = 0;
	} else if (mode >= 0 && !grabbed) {
		if (SDL_WM_GrabInput(SDL_GRAB_ON) == SDL_GRAB_ON) {
			grabbed = 1;
		}
	}
}

/**
**  Toggle full screen mode.
*/
global void ToggleFullScreen(void)
{
#ifdef USE_WIN32
	long framesize;
	SDL_Rect clip;
	Uint32 flags;
	int w;
	int h;
	int bpp;
#ifndef USE_OPENGL
	void* pixels;
	SDL_Color* palette;
	int ncolors;
#endif

	if (!TheScreen) {						// don't bother if there's no surface.
		return;
	}

	flags = TheScreen->flags;
	w = TheScreen->w;
	h = TheScreen->h;
	bpp = TheScreen->format->BitsPerPixel;

	SDL_GetClipRect(TheScreen, &clip);

	// save the contents of the screen.
	framesize = w * h * TheScreen->format->BytesPerPixel;

#ifndef USE_OPENGL
	if (!(pixels = malloc(framesize))) {		// out of memory
		return;
	}
	SDL_LockSurface(TheScreen);
	memcpy(pixels, TheScreen->pixels, framesize);

#ifdef DEBUG
	// shut up compiler
	palette = NULL;
	ncolors=0;
#endif
	if (TheScreen->format->palette) {
		ncolors = TheScreen->format->palette->ncolors;
		if (!(palette = malloc(ncolors * sizeof(SDL_Color)))) {
			free(pixels);
			return;
		}
		memcpy(palette, TheScreen->format->palette->colors,
			ncolors * sizeof(SDL_Color));
	}
	SDL_UnlockSurface(TheScreen);
#endif

	TheScreen = SDL_SetVideoMode(w, h, bpp, flags ^ SDL_FULLSCREEN);
	if (!TheScreen) {
		TheScreen = SDL_SetVideoMode(w, h, bpp, flags);
		if (!TheScreen) {				// completely screwed.
#ifndef USE_OPENGL
			free(pixels);
			if (TheScreen->format->palette) {
				free(palette);
			}
#endif
			fprintf(stderr, "Toggle to fullscreen, crashed all\n");
			Exit(-1);
		}
	}

	// Windows shows the SDL cursor when starting in fullscreen mode
	// then switching to window mode.  This hides the cursor again.
	SDL_ShowCursor(SDL_ENABLE);
	SDL_ShowCursor(SDL_DISABLE);

#ifdef USE_OPENGL
	InitOpenGL();
#else
	SDL_LockSurface(TheScreen);
	memcpy(TheScreen->pixels, pixels, framesize);
	free(pixels);

	if (TheScreen->format->palette) {
		// !!! FIXME : No idea if that flags param is right.
		SDL_SetPalette(TheScreen, SDL_LOGPAL, palette, 0, ncolors);
		free(palette);
	}
	SDL_UnlockSurface(TheScreen);
#endif

	SDL_SetClipRect(TheScreen, &clip);

	Invalidate();						// Update display
#else		// !USE_WIN32
	SDL_WM_ToggleFullScreen(TheScreen);
#endif

	VideoFullScreen = (TheScreen->flags & SDL_FULLSCREEN) ? 1 : 0;
}

#endif // } USE_SDL

//@}
