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
/**@name video.c	-	The universal video functions. */
/*
**	(c) Copyright 1999,2000 by Lutz Sammer
**
**	$Id$
*/

//@{

/*----------------------------------------------------------------------------
--	Includes
----------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>

#include "freecraft.h"
#include "video.h"

#include "map.h"
#include "ui.h"

#ifdef USE_SDL
#include <SDL/SDL.h>
#endif

/*----------------------------------------------------------------------------
--	Declarations
----------------------------------------------------------------------------*/

// JOHNS: This is needed, because later I want to support it all with the same
//	  executable, choosable at runtime.
#ifdef USE_X11
#define UseX11		1
#define UseSdl		0
#define UseSVGALib	0
#define UseWin32	0
#endif

#ifdef USE_SDL
#define UseX11		0
#define UseSdl		1
#define UseSVGALib	0
#define UseWin32	0
#endif

#ifdef USE_SVGALIB
#define UseX11		0
#define UseSdl		0
#define UseSVGALib	1
#define UseWin32	0
#endif

#ifdef noUSE_WIN32
#define UseX11		0
#define UseSdl		0
#define UseSVGALib	0
#define UseWin32	1
#endif

/*----------------------------------------------------------------------------
--	Externals
----------------------------------------------------------------------------*/

extern void InitVideoSdl(void);
extern void InitVideoX11(void);
extern void InitVideoSVGA(void);
extern void InitVideoWin32(void);

/*----------------------------------------------------------------------------
--	Variables
----------------------------------------------------------------------------*/

global int VideoFullScreen;		/// true fullscreen wanted

/*----------------------------------------------------------------------------
--	Functions
----------------------------------------------------------------------------*/

/**
**	Load a picture and display it on the screen (full screen),
**	changing the colormap and so on..
**
**	@param name name of the picture (file) to display
*/
global void DisplayPicture(const char *name)
{
    Graphic* title;

    title=LoadGraphic(name);
    memcpy(GlobalPalette,title->Palette,sizeof(GlobalPalette));
    VideoCreatePalette(GlobalPalette);
    // FIXME: Make some function? SetPalette(palette);

#ifdef USE_SDL
    { extern SDL_Surface *Screen;		/// internal screen
    SDL_LockSurface(Screen);

    DebugCheck( VideoMemory!=Screen->pixels );
#endif

    // FIXME: bigger window ?
    VideoDrawSubClip(title,0,0
	,title->Width,title->Height
	,(VideoWidth-title->Width)/2,(VideoHeight-title->Height)/2);

#ifdef USE_SDL
    SDL_UnlockSurface(Screen); }
#endif
    VideoFree(title);
}

/**
**	Video initialize.
*/
global void InitVideo(void)
{
    if( UseSdl ) {
	InitVideoSdl();
    } else if( UseX11 ) {
	InitVideoX11();
    } else if( UseSVGALib ) {
	InitVideoSVGA();
    } else if( UseWin32 ) {
	InitVideoWin32();
    } else {
	IfDebug( abort(); );
    }

    //
    //	Init video sub modules
    //
    InitGraphic();
    InitLineDraw();

    DebugLevel3(__FUNCTION__": %d %d\n",MapWidth,MapHeight);
    fflush(stdout);
}

//@}
