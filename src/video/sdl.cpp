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
**	Called from SIGALRM.
*/
local Uint32 VideoSyncHandler(Uint32 unused)
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
*/
global void SetVideoSync(void)
{
    if( !VideoSyncSpeed ) {
	return;
    }

#ifdef __linux__
    // FIXME: doesn't work with SDL/SVGAlib 1.0
    {
	// ARI: kick svgalib's butt - WE handled SIGALRM, no bombing any more!
	// JOHNS: I think this will no longer work with SDL 1.1, yes it doesn't
	//extern void SDL_TimerInit();
	//SDL_TimerInit();
    }
#endif
    if( SDL_SetTimer(
		(100*1000/FRAMES_PER_SECOND)/VideoSyncSpeed,
		VideoSyncHandler) ) {
	fprintf(stderr, "Can't set timer or you use SDL 1.1.X %d\n"
		,(100*1000/FRAMES_PER_SECOND)/VideoSyncSpeed);
    }

    // DebugLevel1("Timer installed\n");
}

/*----------------------------------------------------------------------------
--	Video
----------------------------------------------------------------------------*/

global SDL_Surface *Screen;		/// internal screen

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

#if 0
    // FIXME: remove this, if it works everywhere, new freecraft didn't need
    // FIXME: repeat, it works better without it.
    // Enable keyboard repeat (with autodetection of SDL version :)
#ifdef SDL_DEFAULT_REPEAT_DELAY
    SDL_EnableKeyRepeat(SDL_DEFAULT_REPEAT_DELAY,SDL_DEFAULT_REPEAT_INTERVAL);
#endif
#endif

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
local int Sdl2InternalKeycode(const SDL_keysym* code)
{
    int icode;

    /*
    **	Convert SDL keycodes into internal keycodes.
    */
    switch( (icode=code->sym) ) {
	case SDLK_ESCAPE:
	    icode='\e';
	    break;
	case SDLK_RETURN:
	    icode='\r';
	    break;
	case SDLK_BACKSPACE:
	    icode='\b';
	    break;
	case SDLK_TAB:
	    icode='\t';
	    break;
	case SDLK_UP:
	    icode=KeyCodeUp;
	    break;
	case SDLK_DOWN:
	    icode=KeyCodeDown;
	    break;
	case SDLK_LEFT:
	    icode=KeyCodeLeft;
	    break;
	case SDLK_RIGHT:
	    icode=KeyCodeRight;
	    break;
	case SDLK_PAUSE:
	    icode=KeyCodePause;
	    break;
	case SDLK_F1:
	    icode=KeyCodeF1;
	    break;
	case SDLK_F2:
	    icode=KeyCodeF2;
	    break;
	case SDLK_F3:
	    icode=KeyCodeF3;
	    break;
	case SDLK_F4:
	    icode=KeyCodeF4;
	    break;
	case SDLK_F5:
	    icode=KeyCodeF5;
	    break;
	case SDLK_F6:
	    icode=KeyCodeF6;
	    break;
	case SDLK_F7:
	    icode=KeyCodeF7;
	    break;
	case SDLK_F8:
	    icode=KeyCodeF8;
	    break;
	case SDLK_F9:
	    icode=KeyCodeF9;
	    break;
	case SDLK_F10:
	    icode=KeyCodeF10;
	    break;
	case SDLK_F11:
	    icode=KeyCodeF11;
	    break;
	case SDLK_F12:
	    icode=KeyCodeF12;
	    break;
	case SDLK_KP0:
	    icode=KeyCodeKP0;
	    break;
	case SDLK_KP1:
	    icode=KeyCodeKP1;
	    break;
	case SDLK_KP2:
	    icode=KeyCodeKP2;
	    break;
	case SDLK_KP3:
	    icode=KeyCodeKP3;
	    break;
	case SDLK_KP4:
	    icode=KeyCodeKP4;
	    break;
	case SDLK_KP5:
	    icode=KeyCodeKP5;
	    break;
	case SDLK_KP6:
	    icode=KeyCodeKP6;
	    break;
	case SDLK_KP7:
	    icode=KeyCodeKP7;
	    break;
	case SDLK_KP8:
	    icode=KeyCodeKP8;
	    break;
	case SDLK_KP9:
	    icode=KeyCodeKP9;
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
	    // Note to X11-users: SDL (versions before 1.0.3) is buggy here.
	    // Shifted keys may fail to be converted correctly.
	    // Use SDL 1.0.3 or higher, which works ok.
	    if (1) {
		if (icode >= '0' && icode <= '9') {
		    if( code->mod&(KMOD_CTRL|KMOD_ALT|KMOD_META) ) {
			// Do not translate these to support grouping!
			break;
		    }
		}
		if ( (code->unicode & 0xFF80) == 0 ) {
		    icode = code->unicode & 0x7F;
		} else {
		    // An international character..
		    // let's asume latin 1 for now
		    icode = code->unicode & 0xFF;
		}
	    } else if( code->mod&(KMOD_SHIFT) ) {
		// FIXME: only letters handled here - implement shift keymap (punctuation!)
		if(icode <= 'z' && icode >= 'a') {
		    icode -= 32;
		}
	    }
	    break;
    }

    return icode;
}

/**
**	Handle keyboard!
**
**	@param code	SDL keysym structure pointer.
*/
local void SdlHandleKey(const SDL_keysym* code)
{
    int icode;

    icode=Sdl2InternalKeycode(code);

    if( HandleKeyDown(icode) ) {
	return;
    }
    // FIXME: Should come first, move this into low level!!!!
    DoButtonPanelKey(icode);
}

/**
**	Handle keyboard!
**
**	@param code	SDL keysym structure pointer.
*/
local void SdlHandleKeyUp(const SDL_keysym* code)
{
    int icode;

    icode=Sdl2InternalKeycode(code);

    HandleKeyUp(icode);
}

/**
**	Handle interactive input event.
*/
local void DoEvent(SDL_Event* event)
{
    switch( event->type ) {
	case SDL_MOUSEBUTTONDOWN:
	    DebugLevel3("\tbutton press %d\n",event->button.button);
	    HandleButtonDown(event->button.button);
	    break;

	case SDL_MOUSEBUTTONUP:
	    DebugLevel3("\tbutton release %d\n",event->button.button);
	    HandleButtonUp(event->button.button);
	    break;

	    // FIXME SDL: check if this is only usefull for the cursor
	    //            if this is the case we don't need this.
	case SDL_MOUSEMOTION:
	    DebugLevel3("\tmotion notify %d,%d\n"
		    ,event->motion.x,event->motion.y);
	    HandleMouseMove(event->motion.x,event->motion.y);
	    // FIXME: Same bug fix from X11
	    if ( (TheUI.WarpX != -1 || TheUI.WarpY != -1)
		    && (event->motion.x!=TheUI.WarpX
			|| event->motion.y!=TheUI.WarpY) ) {
		int xw;
		int yw;

		xw = TheUI.WarpX;
		yw = TheUI.WarpY;
		TheUI.WarpX = -1;
		TheUI.WarpY = -1;
		SDL_WarpMouse(xw,yw);
	    }
	    MustRedraw|=RedrawCursor;
	    break;

	case SDL_ACTIVEEVENT:
	    DebugLevel3("\tFocus changed\n");
	    if( !event->active.state ) {
		CursorOn=-1;
	    }
	    break;

	case SDL_KEYDOWN:
	    DebugLevel3("\tKey press\n");
	    SdlHandleKey(&event->key.keysym);
	    break;

	case SDL_KEYUP:
	    DebugLevel3("\tKey release\n");
	    SdlHandleKeyUp(&event->key.keysym);
	    break;

	case SDL_QUIT:
	    Exit(0);
    }
}

/**
**	Wait for interactive input event.
**
**	Handles X11 events, keyboard, mouse.
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
    struct timeval tv;
    fd_set rfds;
    fd_set wfds;
    int maxfd;
    int i;

    SDL_Event event[1];

    for(;;) {
	// Not very nice, but this is the problem if you use other libraries
	// The event handling of SDL is wrong designed = polling only.
	while( SDL_PollEvent(event) ) {
	    // Handle SDL event
	    DoEvent(event);
	}

	//
	//	Prepare select
	//
	tv.tv_sec=0;
	tv.tv_usec=0;
	FD_ZERO(&rfds);
	FD_ZERO(&wfds);
	maxfd=0;

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
	if( !SoundOff && !SoundThreadRunning && SoundFildes!=-1 ) {
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
	maxfd=select(maxfd+1,&rfds,&wfds,NULL,&tv);
	i=SDL_PollEvent(event);
#endif

	if ( i ) {
	    // Handle SDL event
	    DoEvent(event);
	}

	if( maxfd>0 ) {
#ifndef USE_SDLA
	    //
	    //	Sound
	    //
	    if( !SoundOff && !SoundThreadRunning && SoundFildes!=-1 
			&& FD_ISSET(SoundFildes,&wfds) ) {
		WriteSound();
	    }
#endif

	    //
	    //	Network in sync and time for frame over: return
	    //
	    if( !i && NetworkInSync && VideoInterrupts ) {
		return;
	    }

	    //
	    //	Network
	    //
	    if( NetworkFildes!=-1 && FD_ISSET(NetworkFildes,&rfds) ) {
		NetworkEvent();
	    }
	}

	//
	//	Network in sync and time for frame over: return
	//
	if( NetworkInSync && VideoInterrupts ) {
	    return;
	}
    }
}

/**
**	Create a new hardware dependend palette palette.
**
**	@param palette	Hardware independend palette.
**
**	@return		A hardware dependend pixel table.
*/
global VMemType* VideoCreateNewPalette(const Palette *palette)
{
    int i;
    void* pixels;

    if( !Screen ) {			// no init
	return NULL;
    }

    switch( VideoBpp ) {
    case 8:
	pixels=malloc(256*sizeof(VMemType8));
	break;
    case 15:
    case 16:
	pixels=malloc(256*sizeof(VMemType16));
	break;
    case 24:
	pixels=malloc(256*sizeof(VMemType24));
	break;
    case 32:
	pixels=malloc(256*sizeof(VMemType32));
	break;
    default:
	DebugLevel0Fn("Unknown depth\n");
	return NULL;
    }

    //
    //	Convert each palette entry into hardware format.
    //
    for( i=0; i<256; ++i ) {
	int r;
	int g;
	int b;
	int v;
	char *vp;

	r=(palette[i].r)&0xFF;
	g=(palette[i].g)&0xFF;
	b=(palette[i].b)&0xFF;
	v=r+g+b;

	// Apply global saturation,contrast and brightness
	r= ((((r*3-v)*TheUI.Saturation + v*100)
	    *TheUI.Contrast)
	    +TheUI.Brightness*25600*3)/30000;
	g= ((((g*3-v)*TheUI.Saturation + v*100)
	    *TheUI.Contrast)
	    +TheUI.Brightness*25600*3)/30000;
	b= ((((b*3-v)*TheUI.Saturation + v*100)
	    *TheUI.Contrast)
	    +TheUI.Brightness*25600*3)/30000;

	// Boundings
	r= r<0 ? 0 : r>255 ? 255 : r;
	g= g<0 ? 0 : g>255 ? 255 : g;
	b= b<0 ? 0 : b>255 ? 255 : b;

	// -> Video
	switch( VideoBpp ) {
	case 8:
	    ((VMemType8*)pixels)[i]=SDL_MapRGB(Screen->format,r,g,b);
	    break;
	case 15:
	case 16:
	    ((VMemType16*)pixels)[i]=SDL_MapRGB(Screen->format,r,g,b);
	    break;
	case 24:
	    v=SDL_MapRGB(Screen->format,r,g,b);
	    vp = (char *)(&v);
	    ((VMemType24*)pixels)[i].a=vp[0];	// endian safe ?
	    ((VMemType24*)pixels)[i].b=vp[1];
	    ((VMemType24*)pixels)[i].c=vp[2];
	    break;
	case 32:
	    ((VMemType32*)pixels)[i]=SDL_MapRGB(Screen->format,r,g,b);
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
#ifdef SDL_GRAB_ON
    static int grabbed;

    if( grabbed ) {
	SDL_WM_GrabInput(SDL_GRAB_OFF);
	grabbed=0;
    } else {
	if( SDL_WM_GrabInput(SDL_GRAB_ON)==SDL_GRAB_ON ) {
	    grabbed=1;
	}
    }
#endif	// SDL_GRAB_ON
}

#endif // } USE_SDL

//@}
