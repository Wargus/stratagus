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
//	(c) Copyright 1999-2001 by Lutz Sammer
//
//	$Id$

//@{

/*----------------------------------------------------------------------------
--	Includes
----------------------------------------------------------------------------*/

#include "freecraft.h"

#ifdef USE_SDL	// {

#include <stdlib.h>
#include <sys/time.h>
#include <SDL/SDL.h>

#ifdef USE_BEOS
#include <sys/socket.h>
#endif

#ifdef USE_WIN32
#include <winsock.h>
#undef DrawText
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

global SDL_Surface *Screen;		/// internal screen

/*----------------------------------------------------------------------------
--	Functions
----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
--	Sync
----------------------------------------------------------------------------*/

#if 0

/*
**	The timer resolution is 10ms, which make the timer useless for us.
*/

/**
**	Called from another thread or what ever SDL uses..
**
**	@param unused	Need by library: interval of the timer.
**	@param param	Parameter passed in by library.
*/
local Uint32 VideoSyncHandler(Uint32 unused,void* param)
{
    DebugLevel3("Interrupt %d - %d\n"
	    ,VideoInterrupts,(100*1000/FRAMES_PER_SECOND)/VideoSyncSpeed);

    ++VideoInterrupts;

    // FIXME: this solves the timer problem with WIN32
    // WSACancelBlockingCall();
    // kill(0,SIGALRM);

    return (100*1000/FRAMES_PER_SECOND)/VideoSyncSpeed;
}

/**
**	Initialise video sync.
**
**	@note	SDL has only a maximum resolution of 10 ms.
**
**	@see VideoSyncSpeed
*/
global void SetVideoSync(void)
{
    static SDL_TimerID id;

    if( id ) {				// Cancel old timer.
	SDL_RemoveTimer(id);
	id=NULL;
    }

    if( !VideoSyncSpeed ) {
	return;
    }


    id=SDL_AddTimer((100*1000/FRAMES_PER_SECOND)/VideoSyncSpeed,
	VideoSyncHandler,NULL);

    // DebugLevel1("Timer installed\n");
}

#else

/**
**	Initialise video sync.
**
**	@see VideoSyncSpeed
*/
global void SetVideoSync(void)
{
    DebugLevel0Fn("%d\n",(100*1000/FRAMES_PER_SECOND)/VideoSyncSpeed);
}

#endif

/*----------------------------------------------------------------------------
--	Video
----------------------------------------------------------------------------*/

/**
**	Initialze the video part for SDL.
*/
global void InitVideoSdl(void)
{
    //	Initialize the SDL library

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

    // Initialize the display

    if( !VideoWidth ) {
	VideoWidth = DEFAULT_VIDEO_WIDTH;
	VideoHeight = DEFAULT_VIDEO_HEIGHT;
    }

    Screen = SDL_SetVideoMode(VideoWidth, VideoHeight, VideoDepth
	    // Sam said: better for windows.
	    ,/* SDL_HWSURFACE|SDL_HWPALETTE | */
		(VideoFullScreen ? SDL_FULLSCREEN : 0));

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

    VideoBpp=Screen->format->BitsPerPixel;

    //
    //	I need the used bits per pixel.
    //	You see it's better making all self, than using wired libaries :)
    //  And with the win32 version this also doesn't work
    //
    if( !VideoDepth ) {
	int i;
	int j;

	DebugLevel3Fn("Mask R%x G%x B%x\n"
		,Screen->format->Rmask
		,Screen->format->Gmask
		,Screen->format->Bmask);

	if( Screen->format->BitsPerPixel>8 ) {
	    j=Screen->format->Rmask;
	    j|=Screen->format->Gmask;
	    j|=Screen->format->Bmask;


	    for( i=0; j&(1<<i); ++i ) {
	    }

#if 0
	// FIXME: johns I think this now works out of the box for beos.
#ifdef USE_BEOS
	    if( i==24 ) {	// beos compatibility hack
		i=32;
	    }
#endif
#endif

	    VideoDepth=i;
	} else {
	    VideoDepth=Screen->format->BitsPerPixel;
	}
    }

    // Make default character translation easier
    SDL_EnableUNICODE(1);

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
}

/**
**	Invalidate whole window
*/
global void Invalidate(void)
{
    SDL_UpdateRect(Screen,0,0,VideoWidth,VideoHeight);
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
	    *keychar = icode = '\e';
	    break;
	case SDLK_RETURN:
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
local void SdlHandleKeyPress(void (*const callback) (unsigned,unsigned),
    const SDL_keysym* code)
{
    int icode;
    int keychar;

    icode = Sdl2InternalKeycode(code,&keychar);

    callback(icode,keychar);
}

/**
**	Handle keyboard key release!
**
**	@param callback	Callback funktion for key up.
**	@param code	SDL keysym structure pointer.
*/
local void SdlHandleKeyRelease(void (*const callback) (unsigned,unsigned),
    const SDL_keysym* code)
{
    int icode;
    int keychar;

    icode=Sdl2InternalKeycode(code,&keychar);

    callback(icode,keychar);
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
	    DebugLevel3("\tbutton press %d\n", event->button.button);
	    //
	    //  SDL has already a good order of the buttons.
	    //
	    InputMouseButtonPress(callbacks, SDL_GetTicks(),
		event->button.button);
	    break;

	case SDL_MOUSEBUTTONUP:
	    DebugLevel3("\tbutton release %d\n", event->button.button);
	    //
	    //  SDL has already a good order of the buttons.
	    //
	    InputMouseButtonRelease(callbacks, SDL_GetTicks(),
		event->button.button);
	    break;

	    // FIXME SDL: check if this is only usefull for the cursor
	    //            if this is the case we don't need this.
	case SDL_MOUSEMOTION:
	    DebugLevel3("\tmotion notify %d,%d\n", event->motion.x,
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
	    if (!event->active.state) {
		CursorOn = -1;
	    }
	    break;

	case SDL_KEYDOWN:
	    DebugLevel3("\tKey press\n");
	    SdlHandleKeyPress(callbacks->KeyPressed, &event->key.keysym);
	    break;

	case SDL_KEYUP:
	    DebugLevel3("\tKey release\n");
	    SdlHandleKeyRelease(callbacks->KeyReleased, &event->key.keysym);
	    break;

	case SDL_QUIT:
	    Exit(0);
    }
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

#ifndef USE_SDLA
    if( SoundFildes==-1 ) {
	SoundOff=1;
    }
#endif
    InputMouseTimeout(callbacks,SDL_GetTicks());
    for(;;) {
#if 1
	static Uint32 LastTick;

	//
	//	Time of frame over? This makes the CPU happy. :(
	//
	i=SDL_GetTicks();
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

#ifndef USE_SDLA
    if( SoundFildes==-1 ) {
	SoundOff=1;
    }
#endif
    InputMouseTimeout(&callbacks,SDL_GetTicks());
    for(;;) {
#if 1
	//
	//	Time of frame over? This makes the CPU happy. :(
	//
	i=SDL_GetTicks();
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
	    if( !NetworkInSync ) {
		NetworkRecover();	// recover network
	    }
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
#ifndef USE_SDLA
	    //
	    //	Sound
	    //
	    if( !SoundOff && !SoundThreadRunning
			&& FD_ISSET(SoundFildes,&wfds) ) {
		callbacks.SoundReady();
	    }
#endif

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
	// FIXME: need locking!
	IfDebug(
	    //DrawText(TheUI.MapX+10,TheUI.MapY+10,GameFont,"SLOW FRAME!!");
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
    static int grabbed;

    if( grabbed ) {
	SDL_WM_GrabInput(SDL_GRAB_OFF);
	grabbed=0;
    } else {
	if( SDL_WM_GrabInput(SDL_GRAB_ON)==SDL_GRAB_ON ) {
	    grabbed=1;
	}
    }
}

#endif // } USE_SDL

//@}
