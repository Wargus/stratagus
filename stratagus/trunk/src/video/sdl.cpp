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
/**@name sdl.c		-	SDL video support. */
//
//	(c) Copyright 1999-2002 by Lutz Sammer
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

#include "freecraft.h"

#ifdef USE_SDL	// {

#include <stdlib.h>
#ifdef BSD
#include <string.h>
#endif
#include <limits.h>
#ifndef _MSC_VER
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#endif
#include <SDL.h>
#ifdef USE_OPENGL
#define DrawIcon WinDrawIcon
#define EndMenu WinEndMenu
#include <SDL_opengl.h>
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
--	Declarations
----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
--	Variables
----------------------------------------------------------------------------*/

global SDL_Surface *Screen;		/// Internal screen
global int InMainWindow = 1;		/// Cursor inside freecraft window

local int FrameTicks;			/// Frame length in ms
local int FrameRemainder;		/// Frame remainder 0.1 ms
local int FrameFraction;		/// Frame fractional term
local int SkipFrames;			/// Skip this frames

/*----------------------------------------------------------------------------
--	Functions
----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
--	Sync
----------------------------------------------------------------------------*/

/**
**	Initialise video sync.
**	Calculate the length of video frame and any simulation skips.
**
**	@see VideoSyncSpeed @see SkipFrames @see FrameTicks @see FrameRemainder
*/
global void SetVideoSync(void)
{
    int ms;

    if( VideoSyncSpeed ) {
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
    DebugLevel0Fn("frames %d - %d.%dms\n" _C_ SkipFrames _C_ ms / 10 _C_ ms % 10);
}

/*----------------------------------------------------------------------------
--	Video
----------------------------------------------------------------------------*/

#ifdef USE_OPENGL
/**
**	Initialize open gl for doing 2d with 3d.
*/
local void InitOpenGL(void)
{
    glViewport(0, 0, (GLsizei)VideoWidth, (GLsizei)VideoHeight);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0,1,0,1,-1,1);

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClearDepth(1.0f);
    glShadeModel(GL_FLAT);
    glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);
    glEnable(GL_TEXTURE_2D);
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
}
#endif

/**
**	Initialze the video part for SDL.
*/
global void InitVideoSdl(void)
{
    Uint32 flags;

    //	Initialize the SDL library

    if( SDL_WasInit(SDL_INIT_VIDEO) == 0 ) {

	if ( SDL_Init(
#ifdef USE_SDLA
	    // FIXME: doesn't work with SDL SVGAlib
	    SDL_INIT_AUDIO |
#endif
#ifdef DEBUG
	    SDL_INIT_NOPARACHUTE|
#endif
	    SDL_INIT_VIDEO|SDL_INIT_TIMER) < 0 ) {
	    fprintf(stderr,"Couldn't initialize SDL: %s\n", SDL_GetError());
	    exit(1);
	}

	//	Clean up on exit

	atexit(SDL_Quit);

	// Set WindowManager Title

	SDL_WM_SetCaption("FreeCraft (formerly known as ALE Clone)","FreeCraft");
    } else {
	if( VideoBpp == 32 && VideoDepth == 24 ) {
	    VideoDepth = 0;
	}
    }

    // Initialize the display

    if( !VideoWidth ) {
	VideoWidth = DEFAULT_VIDEO_WIDTH;
	VideoHeight = DEFAULT_VIDEO_HEIGHT;
    }

    flags = 0;
    // Sam said: better for windows.
    /* SDL_HWSURFACE|SDL_HWPALETTE | */
    if( VideoFullScreen ) {
	flags |= SDL_FULLSCREEN;
    }
#ifdef USE_OPENGL
    flags |= SDL_OPENGL;
#endif
    Screen = SDL_SetVideoMode(VideoWidth, VideoHeight, VideoDepth, flags);

    if ( Screen == NULL ) {
	fprintf(stderr, "Couldn't set %dx%dx%d video mode: %s\n"
		,VideoWidth,VideoHeight,VideoDepth,SDL_GetError());
	exit(1);
    }

    IfDebug(
	if( SDL_MUSTLOCK(Screen) ) {
	    DebugLevel0Fn("Must locksurface!\n");
	}
    );

    // Turn cursor off, we use our own.
    SDL_ShowCursor(0);

    VideoBpp = Screen->format->BitsPerPixel;
    VideoFullScreen = (Screen->flags & SDL_FULLSCREEN) ? 1 : 0;

    //
    //	I need the used bits per pixel.
    //	You see it's better making all self, than using wired libaries :)
    //  And with the win32 version this also doesn't work
    //
    if( !VideoDepth ) {
	int i;
	int j;

	DebugLevel3Fn("Mask R%x G%x B%x\n"
		_C_ Screen->format->Rmask
		_C_ Screen->format->Gmask
		_C_ Screen->format->Bmask);

	if( Screen->format->BitsPerPixel>8 ) {
	    j=Screen->format->Rmask;
	    j|=Screen->format->Gmask;
	    j|=Screen->format->Bmask;


	    for( i=0; j&(1<<i); ++i ) {
	    }

	    VideoDepth=i;
	} else {
	    VideoDepth=Screen->format->BitsPerPixel;
	}
    }

    // Make default character translation easier
    SDL_EnableUNICODE(1);

#ifdef USE_OPENGL
    InitOpenGL();
#endif

    DebugLevel3Fn("Video init ready %d %d\n" _C_ VideoDepth _C_ VideoBpp);

    // FIXME: Setup InMainWindow correct.
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
#ifndef USE_OPENGL
    // FIXME: This checks should be done at higher level
    // FIXME: did SDL version >1.1, check this now also?
    if( x<0 ) {
	w+=x;
	x=0;
    }
    if( x+w>=VideoWidth ) {
	w=VideoWidth-x;
    }
    if( w<=0 ) {
	return;
    }
    if( y<0 ) {
	h+=y;
	y=0;
    }
    if( y+h>=VideoHeight ) {
	h=VideoHeight-y;
    }
    if( h<=0 ) {
	return;
    }
    SDL_UpdateRect(Screen,x,y,w,h);
#endif
}

/**
**	Invalidate whole window
*/
global void Invalidate(void)
{
#ifndef USE_OPENGL
    SDL_UpdateRect(Screen,0,0,VideoWidth,VideoHeight);
#endif
}

/**
**	Convert SDL keysym into internal keycode.
**
**	@param code	SDL keysym structure pointer.
**
**	@return		ASCII code or internal keycode.
*/
local int Sdl2InternalKeycode(const SDL_keysym * code, int *keychar)
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
**	Handle keyboard key press!
**
**	@param callback	Callback funktion for key down.
**	@param code	SDL keysym structure pointer.
*/
local void SdlHandleKeyPress(const EventCallback* callbacks,
    const SDL_keysym* code)
{
    int icode;
    int keychar;

    icode = Sdl2InternalKeycode(code,&keychar);
    InputKeyButtonPress(callbacks, SDL_GetTicks(), icode, keychar);
}

/**
**	Handle keyboard key release!
**
**	@param callback	Callback funktion for key up.
**	@param code	SDL keysym structure pointer.
*/
local void SdlHandleKeyRelease(const EventCallback* callbacks,
    const SDL_keysym* code)
{
    int icode;
    int keychar;

    icode=Sdl2InternalKeycode(code,&keychar);
    InputKeyButtonRelease(callbacks, SDL_GetTicks(), icode, keychar);
}

/**
**	Handle interactive input event.
**
**	@param callback	Callback structure for events.
**	@param event	SDL event structure pointer.
*/
local void SdlDoEvent(const EventCallback* callbacks, const SDL_Event * event)
{
    switch (event->type) {
	case SDL_MOUSEBUTTONDOWN:
	    DebugLevel3("\tbutton press %d\n" _C_ event->button.button);
	    //
	    //  SDL has already a good order of the buttons.
	    //
	    InputMouseButtonPress(callbacks, SDL_GetTicks(),
		event->button.button);
	    break;

	case SDL_MOUSEBUTTONUP:
	    DebugLevel3("\tbutton release %d\n" _C_ event->button.button);
	    //
	    //  SDL has already a good order of the buttons.
	    //
	    InputMouseButtonRelease(callbacks, SDL_GetTicks(),
		event->button.button);
	    break;

	    // FIXME SDL: check if this is only usefull for the cursor
	    //            if this is the case we don't need this.
	case SDL_MOUSEMOTION:
	    DebugLevel3("\tmotion notify %d,%d\n" _C_ event->motion.x _C_
		event->motion.y);
	    InputMouseMove(callbacks, SDL_GetTicks(),
		event->motion.x, event->motion.y);
	    // FIXME: Same bug fix from X11
	    if ((TheUI.WarpX != -1 || TheUI.WarpY != -1)
		&& (event->motion.x != TheUI.WarpX
		    || event->motion.y != TheUI.WarpY)) {
		int xw;
		int yw;

		xw = TheUI.WarpX;
		yw = TheUI.WarpY;
		TheUI.WarpX = -1;
		TheUI.WarpY = -1;
		SDL_WarpMouse(xw, yw);
	    }
	    MustRedraw |= RedrawCursor;
	    break;

	case SDL_ACTIVEEVENT:
	    DebugLevel3("\tFocus changed\n");
	    // FIXME: Johns: I think this was not correct?
	    // FIXME: InMainWindow = !InMainWindow;
	    InMainWindow = event->active.gain;
	    if (!InMainWindow) {
                InputMouseExit(callbacks,SDL_GetTicks());
	    }
	    break;

	case SDL_KEYDOWN:
	    DebugLevel3("\tKey press\n");
	    SdlHandleKeyPress(callbacks, &event->key.keysym);
	    break;

	case SDL_KEYUP:
	    DebugLevel3("\tKey release\n");
	    SdlHandleKeyRelease(callbacks, &event->key.keysym);
	    break;

	case SDL_QUIT:
	    Exit(0);
    }
}

#ifdef USE_WIN32
/**
**	Check if the user alt-tabbed away from the game and redraw
**	everyhing when the user comes back.
*/
local void CheckScreenVisible()
{
    static int IsVisible=1;
    Uint8 state;

    state=SDL_GetAppState();
    if( IsVisible && !(state&SDL_APPACTIVE) ) {
	IsVisible=0;
	UiTogglePause();
    }
    else if( !IsVisible && (state&SDL_APPACTIVE) ) {
	IsVisible=1;
	UiTogglePause();
	MustRedraw=RedrawEverything&~RedrawMinimap;
    }
}
#endif

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
**
**	FIXME:	the initialition could be moved out of the loop
*/
global void WaitEventsOneFrame(const EventCallback* callbacks)
{
    struct timeval tv;
    fd_set rfds;
    fd_set wfds;
    int maxfd;
    int i;
    SDL_Event event[1];
    Uint32 ticks;

#if defined(WITH_SOUND) && !defined(USE_SDLA)
    // FIXME: ugly hack, move into sound part!!!
    if( SoundFildes==-1 ) {
	SoundOff=1;
    }
#endif

    if( !++FrameCounter ) {
	// FIXME: tests with frame counter now fails :(
	// FIXME: Should happen in 68 years :)
	fprintf(stderr,"FIXME: *** round robin ***\n");
	fprintf(stderr,"FIXME: *** round robin ***\n");
	fprintf(stderr,"FIXME: *** round robin ***\n");
	fprintf(stderr,"FIXME: *** round robin ***\n");
    }

    ticks=SDL_GetTicks();
    if( ticks>NextFrameTicks ) {	// We are too slow :(
//	IfDebug(
	    // FIXME: need locking!
	    // if (InterfaceState == IfaceStateNormal) {
	    // VideoDrawText(TheUI.MapX+10,TheUI.MapY+10,GameFont,"SLOW FRAME!!");
	    // }
//	);
	++SlowFrameCounter;
    }

    InputMouseTimeout(callbacks,ticks);
    InputKeyTimeout(callbacks,ticks);
    CursorAnimate(ticks);

    for(;;) {
	//
	//	Time of frame over? This makes the CPU happy. :(
	//
	ticks=SDL_GetTicks();
	if( !VideoInterrupts && ticks+11<NextFrameTicks ) {
	    SDL_Delay(10);
	}
	while( ticks>=NextFrameTicks ) {
	    ++VideoInterrupts;
	    FrameFraction+=FrameRemainder;
	    if( FrameFraction>10 ) {
		FrameFraction-=10;
		++NextFrameTicks;
	    }
	    NextFrameTicks+=FrameTicks;
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
	}

#ifndef USE_SDLA
	//
	//	Sound
	//
	if( !SoundOff && !SoundThreadRunning ) {
	    if( SoundFildes>maxfd ) {
		maxfd=SoundFildes;
	    }
	    FD_SET(SoundFildes,&wfds);
	}
#endif

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
#ifndef USE_SDLA
	    //
	    //	Sound
	    //
	    if( !SoundOff && !SoundThreadRunning
		    && FD_ISSET(SoundFildes,&wfds) ) {
		callbacks->SoundReady();
	    }
#endif

	    //
	    //	Network
	    //
	    if( NetworkFildes!=-1 && FD_ISSET(NetworkFildes,&rfds) ) {
		callbacks->NetworkEvent();
	    }
	}

	//
	//	No more input and time for frame over: return
	//
	if( !i && maxfd<=0 && VideoInterrupts ) {
	    break;
	}
    }

    //
    //	Prepare return, time for one frame is over.
    //
    VideoInterrupts=0;
    
    if( !SkipGameCycle-- ) {
	SkipGameCycle=SkipFrames;
    }

#ifdef USE_WIN32
    CheckScreenVisible();
#endif
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
}

/**
**	Realize video memory.
*/
global void RealizeVideoMemory(void)
{
#ifdef USE_OPENGL
    SDL_GL_SwapBuffers();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    MustRedraw=RedrawEverything;
#endif
}

/**
**	Lock the screen for write access.
*/
global void SdlLockScreen(void)
{
#ifndef USE_OPENGL
    SDL_LockSurface(Screen);
    VideoMemory=Screen->pixels;
#endif
}

/**
**	Unlock the screen for write access.
*/
global void SdlUnlockScreen(void)
{
#ifndef USE_OPENGL
    SDL_UnlockSurface(Screen);
#ifdef DEBUG
    VideoMemory=NULL;			// Catch errors!
#else
    VideoMemory=Screen->pixels;		// Be kind
#endif
#endif
}

/**
**	Toggle grab mouse.
**
**	@param mode	Wanted mode, 1 grab, -1 not grab, 0 toggle.
*/
global void ToggleGrabMouse(int mode)
{
    static int grabbed;

    if( mode<=0 && grabbed ) {
	SDL_WM_GrabInput(SDL_GRAB_OFF);
	grabbed=0;
    } else if( mode>=0 && !grabbed ) {
	if( SDL_WM_GrabInput(SDL_GRAB_ON)==SDL_GRAB_ON ) {
	    grabbed=1;
	}
    }
}

/**
**	Toggle full screen mode.
**
**	@todo FIXME: didn't work with windows,
**		must quit video system and restart it.
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
    void *pixels;
    SDL_Color *palette;
    int ncolors;
#endif

    if ( !Screen ) {			// don't bother if there's no surface.
	return;
    }

    flags = Screen->flags;
    w = Screen->w;
    h = Screen->h;
    bpp = Screen->format->BitsPerPixel;

    SDL_GetClipRect(Screen, &clip);

    // save the contents of the screen.
    framesize = w * h * Screen->format->BytesPerPixel;

#ifndef USE_OPENGL
    if ( !(pixels = malloc(framesize)) ) {	// out of memory
	return;
    }
    SDL_LockSurface(Screen);
    memcpy(pixels, Screen->pixels, framesize);

    IfDebug( palette=NULL; ncolors=0; );	// shut up compiler
    if ( Screen->format->palette ) {
	ncolors = Screen->format->palette->ncolors;
	if ( !(palette = malloc(ncolors * sizeof(SDL_Color))) ) {
	    free(pixels);
	    return;
	}
	memcpy(palette, Screen->format->palette->colors,
	    ncolors * sizeof(SDL_Color));
    }
    SDL_UnlockSurface(Screen);
#endif

    Screen = SDL_SetVideoMode(w, h, bpp, flags ^ SDL_FULLSCREEN);
    if( !Screen ) {
	Screen = SDL_SetVideoMode(w, h, bpp, flags);
	if ( !Screen ) {		// completely screwed.
#ifndef USE_OPENGL
	    free(pixels);
	    if( Screen->format->palette ) {
		free(palette);
	    }
#endif
	    fprintf(stderr,"Toggle to fullscreen, crashed all\n");
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
    SDL_LockSurface(Screen);
    memcpy(Screen->pixels, pixels, framesize);
    free(pixels);

    if ( Screen->format->palette ) {
	// !!! FIXME : No idea if that flags param is right.
	SDL_SetPalette(Screen, SDL_LOGPAL, palette, 0, ncolors);
	free(palette);
    }
    SDL_UnlockSurface(Screen);
#endif

    SDL_SetClipRect(Screen, &clip);

    Invalidate();			// Update display
#else
    SDL_WM_ToggleFullScreen(Screen);
#endif
    VideoFullScreen = (Screen->flags & SDL_FULLSCREEN) ? 1 : 0;
}

#endif // } USE_SDL

//@}
