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
/**@name svgalib.c	-	svgalib support. */
//
//	(c) Copyright 1999-2001 by Jarek Sobieszek
//
//	$Id$

//@{

/*----------------------------------------------------------------------------
--	Includes
----------------------------------------------------------------------------*/

#include <stdio.h>

#include "freecraft.h"

#ifdef USE_SVGALIB

#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#include <vga.h>
#include <vgamouse.h>
#include <vgakeyboard.h>

#include "video.h"
#include "tileset.h"
#include "sound_id.h"
#include "unittype.h"
#include "player.h"
#include "unit.h"
#include "map.h"
#include "minimap.h"
#include "font.h"
#include "image.h"
#include "sound_server.h"
#include "missile.h"
#include "sound.h"
#include "cursor.h"
#include "interface.h"
#include "network.h"
#include "ui.h"
#include "video.h"

/*----------------------------------------------------------------------------
--	Variables
----------------------------------------------------------------------------*/

local int old_button;			/// Svgalib old button state
local int mouse_x;			/// Svgalib mouse x position
local int mouse_y;			/// Svgalib mouse y position

local struct timeval SVGALibTicksStart;	/// My counter start

local const EventCallback* SVGALibCallbacks;	/// My event call back

/*----------------------------------------------------------------------------
--	Forwards
----------------------------------------------------------------------------*/

    /// FIXME: docu?
local void MouseEvent(int button,int dx,int dy,int dz,int drx,int dry,int drz);
    /// FIXME: docu?
local void KeyboardEvent(int scancode,int press);

/*----------------------------------------------------------------------------
--	Functions
----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
--	Sync
----------------------------------------------------------------------------*/

/*
**	The timer resolution is 10ms, which make the timer useless for us.
*/

/**
**	Called from SIGALRM.
*/
local void VideoSyncHandler(int unused)
{
    DebugLevel3Fn("Interrupt\n");
    ++VideoInterrupts;
}

/**
**	Initialise video sync.
*/
global void SetVideoSync(void)
{
    struct sigaction sa;
    struct itimerval itv;

    if( !VideoSyncSpeed ) {
	return;
    }

    sa.sa_handler=VideoSyncHandler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags=SA_RESTART;
    if( sigaction(SIGALRM,&sa,NULL) ) {
	fprintf(stderr,"Can't set signal\n");
    }

    itv.it_interval.tv_sec=itv.it_value.tv_sec=
	(100/FRAMES_PER_SECOND)/VideoSyncSpeed;
    itv.it_interval.tv_usec=itv.it_value.tv_usec=
	(100000000/FRAMES_PER_SECOND)/VideoSyncSpeed-
	itv.it_value.tv_sec*100000;
    if( setitimer(ITIMER_REAL,&itv,NULL) ) {
	fprintf(stderr,"Can't set itimer\n");
    }

    DebugLevel3Fn("Timer installed\n");
}

/**
**	SVGALib get ticks in ms.
*/
long SVGALibGetTicks(void)
{
    struct timeval now;
    long ticks;
 
    gettimeofday(&now,NULL);

    ticks=(now.tv_sec-SVGALibTicksStart.tv_sec)*1000
	    +(now.tv_usec-SVGALibTicksStart.tv_usec)/1000;

    return ticks;
}

/**
**	Close the display.
*/
local void CloseDisplay(void)
{
    free(VideoMemory);
    vga_setmode(TEXT);
}

/**
**	Initialze the video part for SVGAlib.
*/
global void InitVideoSVGA(void)
{
    int real_uid;
    int mode;
    vga_modeinfo *vga_info;

    gettimeofday(&SVGALibTicksStart,NULL);

    if( !VideoDepth ) {
	VideoDepth = 16;
    }

    if( !VideoWidth ) {
	VideoWidth = DEFAULT_VIDEO_WIDTH;
	VideoHeight = DEFAULT_VIDEO_HEIGHT;
    }

    real_uid = getuid();
    setuid(geteuid());

    if(vga_init() == -1) {
	fprintf(stderr, "Cannot initialize svgalib.\n");
	exit(-1);
    }
    VideoMemory =malloc(VideoWidth * VideoHeight * ((VideoDepth+7) >> 3));
    if(VideoMemory == NULL) {
	fprintf(stderr, "Cannot allocate virtual screen.\n");
	exit(-1);
    }
    if(atexit(CloseDisplay) == -1) {
	fprintf(stderr, "Cannot register CloseDisplay.\n");
	free(VideoMemory);
	exit(-1);
    }

    mode=G640x480x64K;
    switch( VideoDepth ) {
	case 8:
	    switch( VideoWidth ) {
		case 640:
		    mode=G640x480x256;
		    break;
		case 800:
		    mode=G800x600x256;
		    break;
		case 1024:
		    mode=G1024x768x256;
		    break;
		case 1600:
		    mode=G1600x1200x256;
		    break;
	    }
	    break;
	case 15:
	    switch( VideoWidth ) {
		case 640:
		    mode=G640x480x32K;
		    break;
		case 800:
		    mode=G800x600x32K;
		    break;
		case 1024:
		    mode=G1024x768x32K;
		    break;
		case 1600:
		    mode=G1600x1200x32K;
		    break;
	    }
	    break;
	case 16:
	    switch( VideoWidth ) {
		case 640:
		    mode=G640x480x64K;
		    break;
		case 800:
		    mode=G800x600x64K;
		    break;
		case 1024:
		    mode=G1024x768x64K;
		    break;
		case 1600:
		    mode=G1600x1200x64K;
		    break;
	    }
	    break;
	case 24:
	    switch( VideoWidth ) {
		case 640:
		    mode=G640x480x16M;
		    break;
		case 800:
		    mode=G800x600x16M;
		    break;
		case 1024:
		    mode=G1024x768x16M;
		    break;
		case 1600:
		    mode=G1600x1200x16M;
		    break;
	    }
	    break;
	case 32:
	    switch( VideoWidth ) {
		case 640:
		    mode=G640x480x16M32;
		    break;
		case 800:
		    mode=G800x600x16M32;
		    break;
		case 1024:
		    mode=G1024x768x16M32;
		    break;
		case 1600:
		    mode=G1600x1200x16M32;
		    break;
	    }
	    break;
    }

    if(vga_setmode(mode) == -1) {
	fprintf(stderr, "%dbpp %dx%d mode is not available.\n"
		,VideoDepth,VideoWidth,VideoHeight);
	exit(-1);
    }

    vga_info =vga_getmodeinfo(mode);
    if(vga_info) {
	if(vga_info->flags && CAPABLE_LINEAR) {
	    vga_setlinearaddressing();
	}
	VideoBpp=vga_info->bytesperpixel*8;
    } else {
	VideoBpp=((VideoDepth+7)/8)*8;
    }

    if(mouse_init("/dev/mouse", vga_getmousetype(),
		MOUSE_DEFAULTSAMPLERATE) == -1) {
	fprintf(stderr, "Cannot enable mouse.\n");
	exit(-1);
    }
    if(atexit(mouse_close) == -1) {
	fprintf(stderr, "Cannot register mouse_close.\n");
	exit(-1);
    }
    if(keyboard_init() == -1) {
	fprintf(stderr, "Cannot switch keyboard to raw mode.\n");
	exit(-1);
    }
    if(atexit(keyboard_close) == -1) {
	keyboard_close();
	fprintf(stderr, "Cannot register keyboard_close.\n");
	exit(-1);
    }

    setuid(real_uid);

    mouse_setposition(VideoWidth/2 , VideoHeight/2);
    mouse_seteventhandler(MouseEvent);
    mouse_setscale(TheUI.MouseScale);
    old_button = 0;
    mouse_x = VideoWidth/2;
    mouse_y = VideoHeight/2;
    HandleMouseMove(mouse_x, mouse_y);
    keyboard_seteventhandler(KeyboardEvent);
}

/**
**	Change video mode to new width.
*/
global int SetVideoMode(int width)
{
    if (width == 640) return 1;
    return 0;
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
**	Handle interactive input events.
*/
local void MouseEvent(int button, int dx, int dy, int dz, int drx, int dry, int drz) {
    if((old_button == 0) && (button == MOUSE_LEFTBUTTON)) {
	DebugLevel3Fn("first down\n");
	InputMouseButtonPress(SVGALibCallbacks,SVGALibGetTicks(),1);
    }
    if((old_button == 0) && (button == (MOUSE_LEFTBUTTON + MOUSE_RIGHTBUTTON))) {
	DebugLevel3Fn("second down\n");
	InputMouseButtonPress(SVGALibCallbacks,SVGALibGetTicks(),2);
    }
    if((old_button == 0) && (button == MOUSE_RIGHTBUTTON)) {
	DebugLevel3Fn("third down\n");
	InputMouseButtonPress(SVGALibCallbacks,SVGALibGetTicks(),3);
    }
    if((old_button == MOUSE_LEFTBUTTON) && (button == 0)) {
	DebugLevel3Fn("first up\n");
	InputMouseButtonRelease(SVGALibCallbacks,SVGALibGetTicks(),1);
    }
    if((old_button == (MOUSE_LEFTBUTTON + MOUSE_RIGHTBUTTON)) && (button == 0)) {
	DebugLevel3Fn("second up\n");
	InputMouseButtonRelease(SVGALibCallbacks,SVGALibGetTicks(),2);
    }
    if((old_button == MOUSE_RIGHTBUTTON) && (button == 0)) {
	DebugLevel3Fn("third up\n");
	InputMouseButtonRelease(SVGALibCallbacks,SVGALibGetTicks(),3);
    }
    old_button = button;

    if(dx != 0 || dy != 0) {
        if(mouse_x + dx/TheUI.MouseAdjust >= 0
		&& mouse_x + dx/TheUI.MouseAdjust <= VideoWidth)
	    mouse_x += dx/TheUI.MouseAdjust;
	if(mouse_y + dy/TheUI.MouseAdjust >= 0
		&& mouse_y + dy/TheUI.MouseAdjust <= VideoHeight)
	    mouse_y += dy/TheUI.MouseAdjust;
	InputMouseMove(SVGALibCallbacks,SVGALibGetTicks(),mouse_x,mouse_y);
	MustRedraw |= RedrawCursor;
    }
}

/**
**	Process keyboard event.
**
**	@param scancode	SVGAlib scancode
**	@param prees	True if key was pressed
**
**	@todo	International scancode support.
*/
local void KeyboardEvent(int scancode, int press)
{
    int icode;
    int keychar;

    IfDebug( icode = 0; );		// keeps the compiler happy
    keychar = 0;

    if (press == KEY_EVENTPRESS) {
	// FIXME: combine scancode -> internal keycode of press and release
	switch (scancode) {
	    case SCANCODE_1:
		keychar = icode = '1';
		if (KeyModifiers & ModifierShift) {
		    icode = '!';
		}
		break;
	    case SCANCODE_2:
		keychar = icode = '2';
		if (KeyModifiers & ModifierShift) {
		    icode = '@';
		}
		break;
	    case SCANCODE_3:
		keychar = icode = '3';
		if (KeyModifiers & ModifierShift) {
		    icode = '3';
		}
		break;
	    case SCANCODE_4:
		keychar = icode = '4';
		if (KeyModifiers & ModifierShift) {
		    icode = '$';
		}
		break;
	    case SCANCODE_5:
		keychar = icode = '5';
		if (KeyModifiers & ModifierShift) {
		    icode = '%';
		}
		break;
	    case SCANCODE_6:
		keychar = icode = '6';
		if (KeyModifiers & ModifierShift) {
		    icode = '^';
		}
		break;
	    case SCANCODE_7:
		keychar = icode = '7';
		if (KeyModifiers & ModifierShift) {
		    icode = '&';
		}
		break;
	    case SCANCODE_8:
		keychar = icode = '8';
		if (KeyModifiers & ModifierShift) {
		    icode = '*';
		}
		break;
	    case SCANCODE_9:
		keychar = icode = '9';
		if (KeyModifiers & ModifierShift) {
		    icode = '(';
		}
		break;
	    case SCANCODE_0:
		keychar = icode = '0';
		if (KeyModifiers & ModifierShift) {
		    icode = ')';
		}
		break;
	    case SCANCODE_A:
		keychar = icode = 'a';
		break;
	    case SCANCODE_B:
		keychar = icode = 'b';
		break;
	    case SCANCODE_C:
		keychar = icode = 'c';
		break;
	    case SCANCODE_D:
		keychar = icode = 'd';
		break;
	    case SCANCODE_E:
		keychar = icode = 'e';
		break;
	    case SCANCODE_F:
		keychar = icode = 'f';
		break;
	    case SCANCODE_G:
		keychar = icode = 'g';
		break;
	    case SCANCODE_H:
		keychar = icode = 'h';
		break;
	    case SCANCODE_I:
		keychar = icode = 'i';
		break;
	    case SCANCODE_J:
		keychar = icode = 'j';
		break;
	    case SCANCODE_K:
		keychar = icode = 'k';
		break;
	    case SCANCODE_L:
		keychar = icode = 'l';
		break;
	    case SCANCODE_M:
		keychar = icode = 'm';
		break;
	    case SCANCODE_N:
		keychar = icode = 'n';
		break;
	    case SCANCODE_O:
		keychar = icode = 'o';
		break;
	    case SCANCODE_P:
		keychar = icode = 'p';
		break;
	    case SCANCODE_Q:
		keychar = icode = 'q';
		break;
	    case SCANCODE_R:
		keychar = icode = 'r';
		break;
	    case SCANCODE_S:
		keychar = icode = 's';
		break;
	    case SCANCODE_T:
		keychar = icode = 't';
		break;
	    case SCANCODE_U:
		keychar = icode = 'u';
		break;
	    case SCANCODE_V:
		keychar = icode = 'v';
		break;
	    case SCANCODE_W:
		keychar = icode = 'w';
		break;
	    case SCANCODE_X:
		keychar = icode = 'x';
		break;
	    case SCANCODE_Y:
		keychar = icode = 'y';
		break;
	    case SCANCODE_Z:
		keychar = icode = 'z';
		break;
	    case SCANCODE_SPACE:
		keychar = icode = ' ';
		break;
	    case SCANCODE_MINUS:
		keychar = icode = '-';
		if (KeyModifiers & ModifierShift) {
		    icode = '_';
		}
		break;
	    case SCANCODE_EQUAL:
		keychar = icode = '=';
		if (KeyModifiers & ModifierShift) {
		    icode = '+';
		}
		break;
	    case SCANCODE_ESCAPE:
		keychar = icode = '\e';
		break;
	    case SCANCODE_ENTER:
		keychar = icode = '\r';
		break;
	    case SCANCODE_BACKSPACE:
		keychar = icode = '\b';
		break;
	    case SCANCODE_TAB:
		keychar = icode = '\t';
		break;

	    case SCANCODE_COMMA:
		keychar = icode = ',';
		break;
	    case SCANCODE_PERIOD:
		keychar = icode = '.';
		break;
	    case SCANCODE_SLASH:
		keychar = icode = '/';
		break;

	    case SCANCODE_CURSORBLOCKUP:
		icode = KeyCodeUp;
		break;
	    case SCANCODE_CURSORBLOCKDOWN:
		icode = KeyCodeDown;
		break;
	    case SCANCODE_CURSORBLOCKLEFT:
		icode = KeyCodeLeft;
		break;
	    case SCANCODE_CURSORBLOCKRIGHT:
		icode = KeyCodeRight;
		break;
	    case SCANCODE_F1:
		icode = KeyCodeF1;
		break;
	    case SCANCODE_F2:
		icode = KeyCodeF2;
		break;
	    case SCANCODE_F3:
		icode = KeyCodeF3;
		break;
	    case SCANCODE_F4:
		icode = KeyCodeF4;
		break;
	    case SCANCODE_F5:
		icode = KeyCodeF5;
		break;
	    case SCANCODE_F6:
		icode = KeyCodeF6;
		break;
	    case SCANCODE_F7:
		icode = KeyCodeF7;
		break;
	    case SCANCODE_F8:
		icode = KeyCodeF8;
		break;
	    case SCANCODE_F9:
		icode = KeyCodeF9;
		break;
	    case SCANCODE_F10:
		icode = KeyCodeF10;
		break;
	    case SCANCODE_F11:
		icode = KeyCodeF11;
		break;
	    case SCANCODE_F12:
		icode = KeyCodeF12;
		break;

	    case SCANCODE_KEYPAD0:
		icode = KeyCodeKP0;
		break;
	    case SCANCODE_KEYPAD1:
		icode = KeyCodeKP1;
		break;
	    case SCANCODE_KEYPAD2:
		icode = KeyCodeKP2;
		break;
	    case SCANCODE_KEYPAD3:
		icode = KeyCodeKP3;
		break;
	    case SCANCODE_KEYPAD4:
		icode = KeyCodeKP4;
		break;
	    case SCANCODE_KEYPAD5:
		icode = KeyCodeKP5;
		break;
	    case SCANCODE_KEYPAD6:
		icode = KeyCodeKP6;
		break;
	    case SCANCODE_KEYPAD7:
		icode = KeyCodeKP7;
		break;
	    case SCANCODE_KEYPAD8:
		icode = KeyCodeKP8;
		break;
	    case SCANCODE_KEYPAD9:
		icode = KeyCodeKP9;
		break;

		// KeyCodePause

	    case SCANCODE_LEFTSHIFT:
	    case SCANCODE_RIGHTSHIFT:
		KeyModifiers |= ModifierShift;
		break;
	    case SCANCODE_LEFTCONTROL:
	    case SCANCODE_RIGHTCONTROL:
		KeyModifiers |= ModifierControl;
		break;
	    case SCANCODE_LEFTALT:
	    case SCANCODE_RIGHTALT:
		KeyModifiers |= ModifierAlt;
		// Super, Hyper
	}
	if (KeyModifiers & ModifierShift) {
	    if (icode <= 'z' && icode >= 'a') {
		icode -= 32;
	    }
	}
	SVGALibCallbacks->KeyPressed(icode, keychar);
    } else if (press == KEY_EVENTRELEASE) {
	// FIXME: combine scancode -> internal keycode of press and release
	switch (scancode) {
	    case SCANCODE_1:
		keychar = icode = '1';
		if (KeyModifiers & ModifierShift) {
		    icode = '!';
		}
		break;
	    case SCANCODE_2:
		keychar = icode = '2';
		if (KeyModifiers & ModifierShift) {
		    icode = '@';
		}
		break;
	    case SCANCODE_3:
		keychar = icode = '3';
		if (KeyModifiers & ModifierShift) {
		    icode = '3';
		}
		break;
	    case SCANCODE_4:
		keychar = icode = '4';
		if (KeyModifiers & ModifierShift) {
		    icode = '$';
		}
		break;
	    case SCANCODE_5:
		keychar = icode = '5';
		if (KeyModifiers & ModifierShift) {
		    icode = '%';
		}
		break;
	    case SCANCODE_6:
		keychar = icode = '6';
		if (KeyModifiers & ModifierShift) {
		    icode = '^';
		}
		break;
	    case SCANCODE_7:
		keychar = icode = '7';
		if (KeyModifiers & ModifierShift) {
		    icode = '&';
		}
		break;
	    case SCANCODE_8:
		keychar = icode = '8';
		if (KeyModifiers & ModifierShift) {
		    icode = '*';
		}
		break;
	    case SCANCODE_9:
		keychar = icode = '9';
		if (KeyModifiers & ModifierShift) {
		    icode = '(';
		}
		break;
	    case SCANCODE_0:
		keychar = icode = '0';
		if (KeyModifiers & ModifierShift) {
		    icode = ')';
		}
		break;
	    case SCANCODE_A:
		keychar = icode = 'a';
		break;
	    case SCANCODE_B:
		keychar = icode = 'b';
		break;
	    case SCANCODE_C:
		keychar = icode = 'c';
		break;
	    case SCANCODE_D:
		keychar = icode = 'd';
		break;
	    case SCANCODE_E:
		keychar = icode = 'e';
		break;
	    case SCANCODE_F:
		keychar = icode = 'f';
		break;
	    case SCANCODE_G:
		keychar = icode = 'g';
		break;
	    case SCANCODE_H:
		keychar = icode = 'h';
		break;
	    case SCANCODE_I:
		keychar = icode = 'i';
		break;
	    case SCANCODE_J:
		keychar = icode = 'j';
		break;
	    case SCANCODE_K:
		keychar = icode = 'k';
		break;
	    case SCANCODE_L:
		keychar = icode = 'l';
		break;
	    case SCANCODE_M:
		keychar = icode = 'm';
		break;
	    case SCANCODE_N:
		keychar = icode = 'n';
		break;
	    case SCANCODE_O:
		keychar = icode = 'o';
		break;
	    case SCANCODE_P:
		keychar = icode = 'p';
		break;
	    case SCANCODE_Q:
		keychar = icode = 'q';
		break;
	    case SCANCODE_R:
		keychar = icode = 'r';
		break;
	    case SCANCODE_S:
		keychar = icode = 's';
		break;
	    case SCANCODE_T:
		keychar = icode = 't';
		break;
	    case SCANCODE_U:
		keychar = icode = 'u';
		break;
	    case SCANCODE_V:
		keychar = icode = 'v';
		break;
	    case SCANCODE_W:
		keychar = icode = 'w';
		break;
	    case SCANCODE_X:
		keychar = icode = 'x';
		break;
	    case SCANCODE_Y:
		keychar = icode = 'y';
		break;
	    case SCANCODE_Z:
		keychar = icode = 'z';
		break;
	    case SCANCODE_SPACE:
		keychar = icode = ' ';
		break;
	    case SCANCODE_MINUS:
		keychar = icode = '-';
		if (KeyModifiers & ModifierShift) {
		    icode = '_';
		}
		break;
	    case SCANCODE_EQUAL:
		keychar = icode = '=';
		if (KeyModifiers & ModifierShift) {
		    icode = '+';
		}
		break;
	    case SCANCODE_ESCAPE:
		keychar = icode = '\e';
		break;
	    case SCANCODE_ENTER:
		keychar = icode = '\r';
		break;
	    case SCANCODE_BACKSPACE:
		keychar = icode = '\b';
		break;
	    case SCANCODE_TAB:
		keychar = icode = '\t';
		break;

	    case SCANCODE_COMMA:
		keychar = icode = ',';
		break;
	    case SCANCODE_PERIOD:
		keychar = icode = '.';
		break;
	    case SCANCODE_SLASH:
		keychar = icode = '/';
		break;

	    case SCANCODE_CURSORBLOCKUP:
		icode = KeyCodeUp;
		break;
	    case SCANCODE_CURSORBLOCKDOWN:
		icode = KeyCodeDown;
		break;
	    case SCANCODE_CURSORBLOCKLEFT:
		icode = KeyCodeLeft;
		break;
	    case SCANCODE_CURSORBLOCKRIGHT:
		icode = KeyCodeRight;
		break;
	    case SCANCODE_F1:
		icode = KeyCodeF1;
		break;
	    case SCANCODE_F2:
		icode = KeyCodeF2;
		break;
	    case SCANCODE_F3:
		icode = KeyCodeF3;
		break;
	    case SCANCODE_F4:
		icode = KeyCodeF4;
		break;
	    case SCANCODE_F5:
		icode = KeyCodeF5;
		break;
	    case SCANCODE_F6:
		icode = KeyCodeF6;
		break;
	    case SCANCODE_F7:
		icode = KeyCodeF7;
		break;
	    case SCANCODE_F8:
		icode = KeyCodeF8;
		break;
	    case SCANCODE_F9:
		icode = KeyCodeF9;
		break;
	    case SCANCODE_F10:
		icode = KeyCodeF10;
		break;
	    case SCANCODE_F11:
		icode = KeyCodeF11;
		break;
	    case SCANCODE_F12:
		icode = KeyCodeF12;
		break;

	    case SCANCODE_KEYPAD0:
		icode = KeyCodeKP0;
		break;
	    case SCANCODE_KEYPAD1:
		icode = KeyCodeKP1;
		break;
	    case SCANCODE_KEYPAD2:
		icode = KeyCodeKP2;
		break;
	    case SCANCODE_KEYPAD3:
		icode = KeyCodeKP3;
		break;
	    case SCANCODE_KEYPAD4:
		icode = KeyCodeKP4;
		break;
	    case SCANCODE_KEYPAD5:
		icode = KeyCodeKP5;
		break;
	    case SCANCODE_KEYPAD6:
		icode = KeyCodeKP6;
		break;
	    case SCANCODE_KEYPAD7:
		icode = KeyCodeKP7;
		break;
	    case SCANCODE_KEYPAD8:
		icode = KeyCodeKP8;
		break;
	    case SCANCODE_KEYPAD9:
		icode = KeyCodeKP9;
		break;

	    case SCANCODE_LEFTSHIFT:
	    case SCANCODE_RIGHTSHIFT:
		KeyModifiers &= ~ModifierShift;
		icode = 0;
		break;
	    case SCANCODE_LEFTCONTROL:
	    case SCANCODE_RIGHTCONTROL:
		KeyModifiers &= ~ModifierControl;
		icode = 0;
		break;
	    case SCANCODE_LEFTALT:
	    case SCANCODE_RIGHTALT:
		KeyModifiers &= ~ModifierAlt;
		icode = 0;
		break;
		// Super ???
	}
	SVGALibCallbacks->KeyReleased(icode, keychar);
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
    int ret;

    SVGALibCallbacks=callbacks;

    InputMouseTimeout(callbacks,SVGALibGetTicks());
    for(;;) {
	//
	//	Prepare select
	//
	tv.tv_sec = tv.tv_usec = 0;

	FD_ZERO(&rfds);
	FD_ZERO(&wfds);

	//
	//	Network
	//
	if(NetworkFildes != -1) {
	    FD_SET(NetworkFildes, &rfds);
	}

	//
	//	Sound
	//
	if( !SoundOff && !SoundThreadRunning && SoundFildes!=-1 ) {
	    FD_SET(SoundFildes, &wfds);
	}

	ret = vga_waitevent(VGA_MOUSEEVENT | VGA_KEYEVENT, &rfds, &wfds, NULL, &tv);

	if(ret >= 0) {
	    //
	    //	Sound
	    //
	    if(!SoundOff && !SoundThreadRunning && SoundFildes!=-1 
			&& FD_ISSET(SoundFildes, &wfds)) {
		callbacks->SoundReady();
	    }

	    //
	    //	Network in sync and time for frame over: return
	    //
	    if(VideoInterrupts) {
		break;
	    }

	    //
	    //	Network
	    //
	    if(NetworkFildes != -1 && FD_ISSET(NetworkFildes, &rfds)) {
		callbacks->NetworkEvent();
	    }
	}

	//
	//	Network in sync and time for frame over: return
	//
	if(VideoInterrupts) {
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
**	Handles SVGALib events, keyboard, mouse.
**	Video interrupt for sync.
**	Network messages.
**	Sound queue.
**
**	We must handle atlast one SVGALib event
**
**	FIXME:	the initialition could be moved out of the loop
*/
global void WaitEventsAndKeepSync(void)
{
    EventCallback callbacks;
    struct timeval tv;
    fd_set rfds;
    fd_set wfds;
    int ret;

    callbacks.ButtonPressed=(void*)HandleButtonDown;
    callbacks.ButtonReleased=(void*)HandleButtonUp;
    callbacks.MouseMoved=(void*)HandleMouseMove;
    callbacks.KeyPressed=HandleKeyDown;
    callbacks.KeyReleased=HandleKeyUp;

    callbacks.NetworkEvent=NetworkEvent;
    callbacks.SoundReady=WriteSound;
    SVGALibCallbacks=&callbacks;

    InputMouseTimeout(&callbacks,SVGALibGetTicks());
    for(;;) {
	//
	//	Prepare select
	//
	tv.tv_sec = tv.tv_usec = 0;

	FD_ZERO(&rfds);
	FD_ZERO(&wfds);

	//
	//	Network
	//
	if(NetworkFildes != -1) {
	    FD_SET(NetworkFildes, &rfds);
	    if( !NetworkInSync ) {
		NetworkRecover();	// recover network
	    }
	}

	//
	//	Sound
	//
	if( !SoundOff && !SoundThreadRunning && SoundFildes!=-1 ) {
	    FD_SET(SoundFildes, &wfds);
	}

	ret = vga_waitevent(VGA_MOUSEEVENT | VGA_KEYEVENT, &rfds, &wfds, NULL, &tv);

	if(ret >= 0) {
	    //
	    //	Sound
	    //
	    if(!SoundOff && !SoundThreadRunning && SoundFildes!=-1 
			&& FD_ISSET(SoundFildes, &wfds)) {
		callbacks.SoundReady();
	    }

	    //
	    //	Network in sync and time for frame over: return
	    //
	    if(NetworkInSync && VideoInterrupts) {
		break;
	    }

	    //
	    //	Network
	    //
	    if(NetworkFildes != -1 && FD_ISSET(NetworkFildes, &rfds)) {
		callbacks.NetworkEvent();
	    }
	}

	//
	//	Network in sync and time for frame over: return
	//
	if(NetworkInSync && VideoInterrupts) {
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
global VMemType* VideoCreateNewPalette(const Palette *palette)
{
    int i;
    void* pixels;

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

	r=(palette[i].r>>2)&0x3F;
	g=(palette[i].g>>2)&0x3F;
	b=(palette[i].b>>2)&0x3F;
	v=r+g+b;

	// Apply global saturation,contrast and brightness
	r= ((((r*3-v)*TheUI.Saturation + v*100)
	    *TheUI.Contrast)
	    +TheUI.Brightness*6400*3)/30000;
	g= ((((g*3-v)*TheUI.Saturation + v*100)
	    *TheUI.Contrast)
	    +TheUI.Brightness*6400*3)/30000;
	b= ((((b*3-v)*TheUI.Saturation + v*100)
	    *TheUI.Contrast)
	    +TheUI.Brightness*6400*3)/30000;

	// Boundings
	r= r<0 ? 0 : r>63 ? 63 : r;
	g= g<0 ? 0 : g>63 ? 63 : g;
	b= b<0 ? 0 : b>63 ? 63 : b;

	// -> Video
	switch( VideoDepth ) {
	case 15:
	    ((VMemType16*)pixels)[i] = ((r >> 1) << 10)
		    + ((g >>1) << 5)
		    + (b >> 1);
	    break;
	case 16:
	    ((VMemType16*)pixels)[i] = ((r >> 1) << 11)
		    + (g << 5)
		    + (b >> 1);
	    break;
	case 8:
	case 24:
	case 32:
	    // FIXME: write this please
	default:
	    DebugLevel0Fn("Depth not written\n");
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
        //DebugLevel1Fn("Slow frame\n");
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
    int i;

    vga_waitretrace();
    for(i = 0; i < 480; i++) {
	vga_drawscansegment((void*)&VideoMemory16[i * 640], 0, i, 1280);
    }
}

/**
**	Toggle grab mouse.
*/
global void ToggleGrabMouse(void)
{
}

#endif	// USE_SVGALIB

//@}
