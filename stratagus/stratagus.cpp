//       _________ __                 __                               
//      /   _____//  |_____________ _/  |______     ____  __ __  ______
//      \_____  \\   __\_  __ \__  \\   __\__  \   / ___\|  |  \/  ___/
//      /        \|  |  |  | \// __ \|  |  / __ \_/ /_/  >  |  /\___ |
//     /_______  /|__|  |__|  (____  /__| (____  /\___  /|____//____  >
//             \/                  \/          \//_____/            \/ 
//  ______________________                           ______________________
//			  T H E   W A R   B E G I N S
//	   Stratagus - A free fantasy real time strategy game engine
//
/**@name clone.c	-	The main file. */
//
//	(c) Copyright 1998-2003 by Lutz Sammer
//
//      This program is free software; you can redistribute it and/or modify
//      it under the terms of the GNU General Public License as published by
//      the Free Software Foundation; version 2 dated June, 1991.
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
//	$Id$

//@{

/**
**	@mainpage
**
**	@section Introduction Introduction
**
**	Welcome to the source code documentation of the Stratagus engine.
**	For an open source project it is very important to have a good
**	source code documentation, I have tried to do this with the help
**	of doxygen (http://www.doxygen.org) or doc++
**	(http://www.zib.de/Visual/software/doc++/index.html). Please read the
**	documentation of this nice open source programs, to see how this all
**	works.
**
**	Any help to improve this documention is welcome. If you didn't
**	understand something or you found a failure or a wrong spelling
**	or wrong grammer please write an email (including a patch :).
**
**	@section Informations Informations
**
**	Visit the http://Stratagus.Org web page for the latest news and
**	../doc/readme.html for other documentations.
**
**	@section Modules Modules
**
**	This are the main modules of the Stratagus engine.
**
**	@subsection Map Map
**
**		Handles the map. A map is made from tiles.
**
**		@see map.h @see map.c @see tileset.h @see tileset.c
**
**	@subsection Unit Unit
**
**		Handles units. Units are ships, flyers, buildings, creatures,
**		machines.
**
**		@see unit.h @see unit.c @see unittype.h @see unittype.c
**
**	@subsection Missile Missile
**
**		Handles missiles. Missiles are all other sprites on map
**		which are no unit.
**
**		@see missile.h @see missile.c
**
**	@subsection Player Player
**
**		Handles players, all units are owned by a player. A player
**		could be controlled by a human or a computer.
**
**		@see player.h @see player.c @see ::Player
**
**	@subsection Sound Sound
**
**		Handles the high and low level of the sound. There are the
**		background music support, voices and sound effects.
**		Following low level backends are supported: OSS and SDL.
**
**		@todo adpcm file format support for sound effects
**		@todo better separation of low and high level, assembler mixing
**			support.
**		@todo Streaming support of ogg/mp3 files.
**
**		@see sound.h @see sound.c
**		@see ccl_sound.c @see sound_id.c @see sound_server.c
**		@see unitsound.c
**		@see oss_audio.c @see sdl_audio.c
**		@see mad.c @see ogg.c @see flac.c @see wav.c
**
**	@subsection Video Video
**
**		Handles the high and low level of the graphics.
**		This also contains the sprite and linedrawing routines.
**
**		See page @ref VideoModule for more information upon supported
**		features and video platforms.
**
**		@see video.h @see video.c
**
**	@subsection Network Network
**
**		Handles the high and low level of the network protocol.
**		The network protocol is needed for multiplayer games.
**
**		See page @ref NetworkModule for more information upon supported
**		features and API.
**
**		@see network.h @see network.c
**
**	@subsection Pathfinder Pathfinder
**
**		@see pathfinder.h @see pathfinder.c
**
**	@subsection AI AI
**
**		There are currently two AI's. The old one is very hardcoded,
**		but does things like placing buildings better than the new.
**		The old AI shouldn't be used.  The new is very flexible, but
**		very basic. It includes none optimations.
**
**		See page @ref AiModule for more information upon supported
**		features and API.
**
**		@see new_ai.c ai_local.h
**		@see ai.h @see ai.c
**
**	@subsection CCL CCL
**
**		CCL is Craft Configuration Language, which is used to
**		configure and customize Stratagus.
**
**		@see ccl.h @see ccl.c
**
**	@subsection Icon Icon
**
**		@see icons.h @see icons.c
**
**	@subsection Editor Editor
**
**		This is the integrated editor, it shouldn't be a perfect
**		editor. It is used to test new features of the engine.
**
**		See page @ref EditorModule for more information upon supported
**		features and API.
**
**		@see editor.h @see editor.c
*/

/*----------------------------------------------------------------------------
--	Includes
----------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <time.h>
#include <ctype.h>

#ifdef USE_BEOS
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

extern void beos_init( int argc, char **argv );

#endif

#ifndef _MSC_VER
#include <unistd.h>
#endif
#if defined(__CYGWIN__)
#include <getopt.h>
#endif
#if defined(_MSC_VER)
//#include "etlib/getopt.h"
extern char* optarg;
extern int optind;
#ifndef _WIN32_WCE
extern int getopt(int argc, char *const*argv, const char *opt);
#endif
#endif

#ifdef USE_SDL
#include "SDL.h"
#endif

#ifdef __MINGW32__
extern int opterr;
extern int optind;
extern int optopt;
extern char *optarg;

extern int getopt(int argc, char *const*argv, const char *opt);
#endif

#include "stratagus.h"
#include "video.h"
#include "font.h"
#include "cursor.h"
#include "ui.h"
#include "interface.h"
#include "menus.h"
#include "sound_server.h"
#include "sound.h"
#include "settings.h"
#include "ccl.h"
#include "network.h"
#include "netconnect.h"
#include "ai.h"
#include "commands.h"
#include "campaign.h"
#include "editor.h"
#include "movie.h"
#include "cdaudio.h"
#include "pathfinder.h"

#ifdef DEBUG
extern SCM CclUnits(void);
#endif

/*----------------------------------------------------------------------------
--	Variables
----------------------------------------------------------------------------*/

global char* TitleScreen;		/// Titlescreen to show at startup
global char* MenuBackground;		/// File for menu background
global char* MenuBackgroundWithTitle;	/// File for menu with title
global char* TitleMusic;		/// File for title music
global char* MenuMusic;			/// File for menu music
global char* StratagusLibPath;		/// Path for data directory
global char LocalPlayerName[16];	/// Name of local player

    /// Name, Version, Copyright
global char NameLine[] =
    "Stratagus V" VERSION ", (c) 1998-2003 by The Stratagus Project.";

local char* MapName;			/// Filename of the map to load

/*----------------------------------------------------------------------------
--	Speedups FIXME: Move to some other more logic place
----------------------------------------------------------------------------*/

global int SpeedResourcesHarvest[MaxCosts]; /// speed factor for harvesting resources
global int SpeedResourcesReturn[MaxCosts];  /// speed factor for returning resources
global int SpeedBuild=SPEED_BUILD;	/// speed factor for building
global int SpeedTrain=SPEED_TRAIN;	/// speed factor for training
global int SpeedUpgrade=SPEED_UPGRADE;	/// speed factor for upgrading
global int SpeedResearch=SPEED_RESEARCH;/// speed factor for researching

/*============================================================================
==	DISPLAY
============================================================================*/

// FIXME: move to video header file
global int VideoPitch;			/// Offset to reach next scan line
global int VideoWidth;			/// Window width in pixels
global int VideoHeight;			/// Window height in pixels

global unsigned long NextFrameTicks;	/// Ticks of begin of the next frame
global unsigned long FrameCounter;	/// Current frame number
global int SlowFrameCounter;		/// Profile, frames out of sync

// FIXME: not the correct place
global int MustRedraw=RedrawEverything;	/// Redraw flags
global int EnableRedraw=RedrawEverything;/// Enable flags

global unsigned long GameCycle;		/// Game simulation cycle counter
global unsigned long FastForwardCycle;	/// Cycle to fastforward to in a replay

/*----------------------------------------------------------------------------
--	Random
----------------------------------------------------------------------------*/

global unsigned SyncRandSeed;		/// sync random seed value.

/**
**	Inititalize sync rand seed.
*/
global void InitSyncRand(void)
{
    SyncRandSeed = 0x87654321;
}

/**
**	Syncron rand.
**
**	@note This random value must be same on all machines in network game.
**	Very simple random generations, enough for us.
*/
global int SyncRand(void)
{
    int val;

    val = SyncRandSeed >> 16;

    SyncRandSeed = SyncRandSeed * (0x12345678 * 4 + 1) + 1;

    return val;
}

/*----------------------------------------------------------------------------
--	Utility
----------------------------------------------------------------------------*/

/**
**	String duplicate/concatenate (two arguments)
**
**	@param l	Left string
**	@param r	Right string
**
**	@return		Allocated combined string (must be freeded).
*/
global char* strdcat(const char* l, const char* r)
{
    char* res;

    res = malloc(strlen(l) + strlen(r) + 1);
    if (res) {
	strcpy(res, l);
	strcat(res, r);
    }
    return res;
}

/**
**	String duplicate/concatenate (three arguments)
**
**	@param l	Left string
**	@param m	Middle string
**	@param r	Right string
**
**	@return		Allocated combined string (must be freeded).
*/
global char* strdcat3(const char* l, const char* m, const char* r)
{
    char* res;

    res = malloc(strlen(l) + strlen(m) + strlen(r) + 1);
    if (res) {
	strcpy(res, l);
	strcat(res, m);
	strcat(res, r);
    }
    return res;
}

#if !defined(BSD) || defined(__APPLE__)
/**
**	Case insensitive version of strstr
**
**	@param a	String to search in
**	@param b	Substring to search for
**
**	@return		Pointer to first occurence of b or NULL if not found.
*/
global char* strcasestr(const char* a, const char* b)
{
    int x;

    if (!a || !*a || !b || !*b || strlen(a) < strlen(b)) {
	return NULL;
    }

    x = 0;
    while (*a) {
	if (a[x] && (tolower(a[x]) == tolower(b[x]))) {
	    x++;
	} else if (b[x]) {
	    a++;
	    x = 0;
	} else {
	    return (char *)a;
	}
    }

    return NULL;
}
#endif // BSD

/**
**	Compute a square root using ints
**
**	Uses John Halleck's method, see 
**	http://www.cc.utah.edu/~nahaj/factoring/isqrt.legalize.c.html
**
**	@param num	Calculate the square root of this number
**
**	@return		The integer square root.
*/
global long isqrt(long num)
{
    long squaredbit;
    long remainder;
    long root;

    if (num < 1) {
	return 0;
    }

    //
    //	Load the binary constant 01 00 00 ... 00, where the number
    //	of zero bits to the right of the single one bit
    //	is even, and the one bit is as far left as is consistant
    //	with that condition.)
    //
    //	This portable load replaces the loop that used to be 
    //	here, and was donated by  legalize@xmission.com 
    //
    squaredbit  = (long)((((unsigned long)~0L) >> 1) & ~(((unsigned long)~0L) >> 2));

    // Form bits of the answer.
    remainder = num;
    root = 0;
    while (squaredbit > 0) {
	if (remainder >= (squaredbit | root)) {
	    remainder -= (squaredbit | root);
	    root >>= 1;
	    root |= squaredbit;
	} else {
	    root >>= 1;
	}
	squaredbit >>= 2; 
    }

    return root;
}

/*============================================================================
==	MAIN
============================================================================*/

local int WaitNoEvent;			/// Flag got an event
local int WaitMouseX;			/// Mouse X position
local int WaitMouseY;			/// Mouse Y position

/**
**	Callback for input.
*/
local void WaitCallbackKey(unsigned dummy __attribute__((unused)))
{
    DebugLevel3Fn("Pressed %8x %8x\n" _C_ MouseButtons _C_ dummy);
    WaitNoEvent=0;
}

/**
**	Callback for input.
*/
local void WaitCallbackKey2(unsigned dummy1 __attribute__((unused)),
	unsigned dummy2 __attribute__((unused)))
{
    DebugLevel3Fn("Pressed %8x %8x %8x\n" _C_ MouseButtons _C_ dummy1 _C_ dummy2);
    WaitNoEvent=0;
}

/**
**	Callback for input.
*/
local void WaitCallbackKey3(unsigned dummy1 __attribute__((unused)),
	unsigned dummy2 __attribute__((unused)))
{
    DebugLevel3Fn("Repeated %8x %8x %8x\n" _C_ MouseButtons _C_ dummy1 _C_ dummy2);
    WaitNoEvent=0;
}

/**
**	Callback for input.
*/
local void WaitCallbackMouse(int x, int y)
{
    DebugLevel3Fn("Moved %d,%d\n" _C_ x _C_ y);
    WaitMouseX=x;
    WaitMouseY=y;
}

/**
**	Callback for exit.
*/
local void WaitCallbackExit(void)
{
    DebugLevel3Fn("Exit\n");
}

#if 0

/**
**	Test some video effects.
**
**	@param frame	Current frame.
*/
local void VideoEffect0(int frame, const EventCallback* callbacks)
{
    int i;
    static Graphic* Logo;

    //
    //	Cleanup
    //
    if( frame==-1 ) {
	VideoFree(Logo);
	Logo=NULL;
	return;
    }

    //
    //	Inititialize
    //
    if( !Logo ) {
	Logo=LoadSprite("stratagus.png",628,141);
    }
    VideoLockScreen();

    switch( VideoDepth ) {
	case 15:
	case 16:
	    for( i=0; i<VideoWidth*VideoHeight; ++i ) {
		int j;

		j=MyRand()&0x1F;
		VideoMemory16[i]=(j<<11)|(j<<6)|(j);
	    }
	    break;
    }

    VideoDrawTextCentered(VideoWidth/2,5,LargeFont,"Press SPACE to continue.");
    VideoDraw(Logo,0,(VideoWidth-VideoGraphicWidth(Logo))/2,50);

    VideoUnlockScreen();

    Invalidate();
    RealizeVideoMemory();
}

#endif

/**
**	Draw video effect circle.
**
**	@param ptr	Memory pointer
**	@param depth	depth
**	@param x	Center x coordinate on the image
**	@param y	Center y coordinate on the image
**	@param r	radius of circle
**
**	@note if you like optimize play here.
*/
global void EffectDrawCircle(int* ptr,int depth,int x,int y,int r)
{
    int cx;
    int cy;
    int df;
    int d_e;
    int d_se;

    cx=0;
    cy=r;
    df=1-r;
    d_e=3;
    d_se=-2*r+5;

#define EffectDrawPixel(depth,x,y) \
    do {								\
	if( 0<(x) && (x)<VideoWidth-1 && 0<(y) && (y)<VideoHeight-1 ) {	\
	    ptr[(x)+(y)*VideoWidth]+=depth;				\
	}								\
    } while( 0 )

    do {
	if( !cx ) {
	    EffectDrawPixel(depth,x,y+cy);
	    EffectDrawPixel(depth,x,y-cy);
	    EffectDrawPixel(depth,x+cy,y);
	    EffectDrawPixel(depth,x-cy,y);
	} else if ( cx==cy ) {
	    EffectDrawPixel(depth,x+cx,y+cy);
	    EffectDrawPixel(depth,x-cx,y+cy);
	    EffectDrawPixel(depth,x+cx,y-cy);
	    EffectDrawPixel(depth,x-cx,y-cy);
	} else if ( cx<cy ) {
	    EffectDrawPixel(depth,x+cx,y+cy);
	    EffectDrawPixel(depth,x+cx,y-cy);
	    EffectDrawPixel(depth,x+cy,y+cx);
	    EffectDrawPixel(depth,x+cy,y-cx);
	    EffectDrawPixel(depth,x-cx,y+cy);
	    EffectDrawPixel(depth,x-cx,y-cy);
	    EffectDrawPixel(depth,x-cy,y+cx);
	    EffectDrawPixel(depth,x-cy,y-cx);
	}
	if( df<0 ) {
	    df+=d_e;
	    d_se+=2;
	} else {
	    df+=d_se;
	    d_se+=4;
	    cy--;
	}
	d_e+=2;
	cx++;

    } while( cx <= cy );
}

/**
**	Test some video effects.
**
**	@param frame		Current frame.
**	@param callbacks	Call backs that handle events.
*/
#ifdef USE_OPENGL
local void VideoEffect0(int frame __attribute__((unused)),
	const EventCallback* callbacks __attribute__((unused)))
{
}

#else

local void VideoEffect0(int frame,
	const EventCallback* callbacks __attribute__((unused)))
{
    static int* buf1;
    static int* buf2;
    static void* vmem;
    int* tmp;
    int x;
    int y;

    //
    //	Cleanup
    //
    if( frame==-1 ) {
	free(buf1);
	free(buf2);
	free(vmem);
	vmem=buf1=buf2=NULL;
	return;
    }

    //
    //	Inititialize
    //
    if( !buf1 ) {
	buf1=calloc(VideoWidth*VideoHeight,sizeof(int));
	buf2=calloc(VideoWidth*VideoHeight,sizeof(int));
	VideoLockScreen();
	switch( VideoBpp ) {
	    case 15:
	    case 16:
		vmem=malloc(VideoWidth*VideoHeight*sizeof(VMemType16));
		memcpy(vmem,VideoMemory,VideoWidth*VideoHeight*sizeof(VMemType16));
		break;
	    case 24:
		vmem=malloc(VideoWidth*VideoHeight*sizeof(VMemType24));
		memcpy(vmem,VideoMemory,VideoWidth*VideoHeight*sizeof(VMemType24));
		break;
	    case 32:
		vmem=malloc(VideoWidth*VideoHeight*sizeof(VMemType32));
		memcpy(vmem,VideoMemory,VideoWidth*VideoHeight*sizeof(VMemType32));
		break;
	}
	VideoUnlockScreen();

	if( 1 ) {
	    for( y=1; y<VideoHeight-1; y+=16 ) {
		for( x=1; x<VideoWidth-1; x+=16 ) {
		    buf1[y*VideoWidth+x] = (rand()%10)*20;
		}
	    }
	}
	if( 0 ) {
	    for( x=1; x<VideoWidth-1; ++x ) {
		buf2[(VideoHeight-1)*VideoWidth+x] = !(x%20)*-100;
	    }
	}
    }

    //
    //	Generate waves
    //
    for( y=1; y<VideoHeight-1; ++y ) {
	for( x=1; x<VideoWidth-1; ++x ) {
	    int i;

	    i = ((buf1[y*VideoWidth+x-1]+
		buf1[y*VideoWidth+x+1]+
		buf1[y*VideoWidth-VideoWidth+x]+
		buf1[y*VideoWidth+VideoWidth+x])>>1) - buf2[y*VideoWidth+x];
	    buf2[y*VideoWidth+x] = i - (i >> 7);
	}
    }

    //
    //	Add mouse
    //
    if( WaitMouseY && WaitMouseX
	    && WaitMouseX!=VideoWidth-1 && WaitMouseY!=VideoHeight-1 ) {
	EffectDrawCircle(buf2,10,WaitMouseX,WaitMouseY,10);
	//buf2[WaitMouseY*VideoWidth+WaitMouseX]-=100;
    }
    //
    //	Random drops
    //
    if( 0 ) {
	EffectDrawCircle(buf2,20,rand()%(VideoWidth-1),rand()%(VideoHeight-1),
	    rand()%7);
    }

    //
    //	Draw it
    //
    VideoLockScreen();
    for( y=1; y<VideoHeight-1; ++y ) {
#ifdef WITH_ARTSC
	if (ArtsGetSpace() >= 1024) {
	    callbacks->SoundReady();
	}
#endif
	for( x=1; x<VideoWidth-1; ++x ) {
	    int xo;
	    int yo;
	    int xt;
	    int yt;
	    VMemType16 pixel16;
	    VMemType24 pixel24;
	    VMemType32 pixel32;

	    xo=buf2[y*VideoWidth+x-1] - buf2[y*VideoWidth+x+1];
	    yo=buf2[y*VideoWidth-VideoWidth+x] -
		    buf2[y*VideoWidth+VideoWidth+x];

	    xt=x+xo;
	    if( xt<0 ) {
		xt=0;
	    } else if( xt>=VideoWidth ) {
		xt=VideoWidth-1;
	    }
	    yt=y+yo;
	    if( yt<0 ) {
		yt=0;
	    } else if( yt>=VideoHeight ) {
		yt=VideoHeight-1;
	    }

	    switch( VideoDepth ) {
		case 15:
		    pixel16=((VMemType16*)vmem)[xt+yt*VideoWidth];
		    if( xo ) {		// Shading
			int r,g,b;

			r=(pixel16>>0)&0x1F;
			g=(pixel16>>5)&0x1F;
			b=(pixel16>>10)&0x1F;
			r+=xo;
			g+=xo;
			b+=xo;
			r= r<0 ? 0 : r>0x1F ? 0x1F : r;
			g= g<0 ? 0 : g>0x1F ? 0x1F : g;
			b= b<0 ? 0 : b>0x1F ? 0x1F : b;
			pixel16=r|(g<<5)|(b<<10);
		    }
		    VideoMemory16[x+VideoWidth*y]=pixel16;
		    break;
		case 16:
		    pixel16=((VMemType16*)vmem)[xt+yt*VideoWidth];
		    if( xo ) {		// Shading
			int r,g,b;

			r=(pixel16>>0)&0x1F;
			g=(pixel16>>5)&0x3F;
			b=(pixel16>>11)&0x1F;
			r+=xo;
			g+=xo*2;
			b+=xo;
			r= r<0 ? 0 : r>0x1F ? 0x1F : r;
			g= g<0 ? 0 : g>0x3F ? 0x3F : g;
			b= b<0 ? 0 : b>0x1F ? 0x1F : b;
			pixel16=r|(g<<5)|(b<<11);
		    }
		    VideoMemory16[x+VideoWidth*y]=pixel16;
		    break;
		case 24:
		    if( VideoBpp==24 ) {
			pixel24=((VMemType24*)vmem)[xt+yt*VideoWidth];
			if( xo ) {
			    int r,g,b;

			    r=(pixel24.a)&0xFF;
			    g=(pixel24.b)&0xFF;
			    b=(pixel24.c)&0xFF;
			    r+=xo<<3;
			    g+=xo<<3;
			    b+=xo<<3;
			    r= r<0 ? 0 : r>0xFF ? 0xFF : r;
			    g= g<0 ? 0 : g>0xFF ? 0xFF : g;
			    b= b<0 ? 0 : b>0xFF ? 0xFF : b;
			    pixel24.a=r;
			    pixel24.b=g;
			    pixel24.c=b;
			}
			VideoMemory24[x+VideoWidth*y]=pixel24;
			break;
		    }
		    // FALL THROUGH
		case 32:
		    pixel32=((VMemType32*)vmem)[xt+yt*VideoWidth];
		    if( xo ) {
			int r,g,b;

			r=(pixel32>>0)&0xFF;
			g=(pixel32>>8)&0xFF;
			b=(pixel32>>16)&0xFF;
			r+=xo<<3;
			g+=xo<<3;
			b+=xo<<3;
			r= r<0 ? 0 : r>0xFF ? 0xFF : r;
			g= g<0 ? 0 : g>0xFF ? 0xFF : g;
			b= b<0 ? 0 : b>0xFF ? 0xFF : b;
			pixel32=r|(g<<8)|(b<<16);
		    }
		    VideoMemory32[x+VideoWidth*y]=pixel32;
		    break;
	    }
	}
    }
    VideoUnlockScreen();

    Invalidate();
    RealizeVideoMemory();

    //
    //	Swap buffers
    //
    tmp=buf1;
    buf1=buf2;
    buf2=tmp;
}
#endif

#ifdef _DEBUG
/**
**	Draw the fonts, for screen shots.
*/
local void DebugDrawFonts(void)
{
    VideoLockScreen();
    VideoClearScreen();

    PushClipping();
    SetClipping(0,0,VideoWidth-1,VideoHeight-1);

    VideoFillRectangle(ColorWhite,0,0,40,VideoHeight-1);
    VideoDrawTextClip(8,   0+ 10,SmallFont,"Stratagus");
    VideoDrawTextClip(8,   0+ 20,SmallFont,
	"~black~0~red~1~green~2~yellow~3~blue~4~magenta~5~cyan~6~white~7"
	"~grey~8~light-red~9~light-green~A~light-yellow~B~light-blue~C"
	"~light-magenta~D~light-cyan~E~light-grey~F");
    VideoDrawTextClip(8,   0+ 30,SmallFont,"abdefgABCDEFQ");

    VideoDrawTextClip(8,  40+ 10,GameFont,"Stratagus");
    VideoDrawTextClip(8,  40+ 25,GameFont,
	"~black~0~red~1~green~2~yellow~3~blue~4~magenta~5~cyan~6~white~7"
	"~grey~8~light-red~9~light-green~A~light-yellow~B~light-blue~C"
	"~light-magenta~D~light-cyan~E~light-grey~F");
    VideoDrawTextClip(8,  40+ 40,GameFont,"abdefgABCDEFQ");

    VideoDrawTextClip(8, 100+ 10,LargeFont,"Stratagus");
    VideoDrawTextClip(8, 100+ 25,LargeFont,
	"~black~0~red~1~green~2~yellow~3~blue~4~magenta~5~cyan~6~white~7"
	"~grey~8~light-red~9~light-green~A~light-yellow~B~light-blue~C"
	"~light-magenta~D~light-cyan~E~light-grey~F");
    VideoDrawTextClip(8, 100+ 40,LargeFont,"abdefgABCDEFQ");

    VideoDrawTextClip(8, 160+ 10,SmallTitleFont,"Stratagus");
    VideoDrawTextClip(8, 160+ 35,SmallTitleFont,
	"~black~0~red~1~green~2~yellow~3~blue~4~magenta~5~cyan~6~white~7"
	"~grey~8~light-red~9~light-green~A~light-yellow~B~light-blue~C"
	"~light-magenta~D~light-cyan~E~light-grey~F");
    VideoDrawTextClip(8, 160+ 60,SmallTitleFont,"abdefgABCDEFQ");

    VideoDrawTextClip(8, 260+ 10,LargeTitleFont,"Stratagus");
    VideoDrawTextClip(8, 260+ 55,LargeTitleFont,
	"~black~0~red~1~green~2~yellow~3~blue~4~magenta~5~cyan~6~white~7"
	"~grey~8~light-red~9~light-green~A~light-yellow~B~light-blue~C"
	"~light-magenta~D~light-cyan~E~light-grey~F");
    VideoDrawTextClip(8, 260+ 100,LargeTitleFont,"abdefgABCDEFQ");

    PopClipping();

    VideoUnlockScreen();
    Invalidate();
    RealizeVideoMemory();
}
#endif

/**
**	Wait for any input.
**
**	@param timeout	Time in seconds to wait.
*/
local void WaitForInput(int timeout)
{
    EventCallback callbacks;
#ifdef linux
    char* s;
    char ddate[72+1];
    FILE* ddfile;
#endif

    SetVideoSync();

    callbacks.ButtonPressed=WaitCallbackKey;
    callbacks.ButtonReleased=WaitCallbackKey;
    callbacks.MouseMoved=WaitCallbackMouse;
    callbacks.MouseExit=WaitCallbackExit;
    callbacks.KeyPressed=WaitCallbackKey2;
    callbacks.KeyReleased=WaitCallbackKey2;
    callbacks.KeyRepeated=WaitCallbackKey3;
    callbacks.NetworkEvent=NetworkEvent;
    callbacks.SoundReady=WriteSound;

    //
    //	FIXME: more work needed, scrolling credits, animations, ...
#ifdef _DEBUG
    WaitNoEvent=1;
    while( WaitNoEvent ) {
	DebugDrawFonts();
	WaitEventsOneFrame(&callbacks);
    }
#else
    VideoLockScreen();
    //VideoDrawTextCentered(VideoWidth/2,5,LargeTitleFont,"Press SPACE to continue.");
    VideoDrawTextCentered(VideoWidth/2,5,LargeFont,"Press SPACE to continue.");
#ifdef linux
    ddate[0]='\0';
    ddfile=popen("`which ddate`","r");
    fgets(ddate,72,ddfile);
    pclose(ddfile);
    if( (s=strrchr(ddate,'\n')) ) {
	*s='\0';
    }
    VideoDrawTextCentered(VideoWidth/2,20,LargeFont,ddate);
#endif
    VideoUnlockScreen();
    Invalidate();
    RealizeVideoMemory();

    WaitNoEvent=1;
    timeout*=CYCLES_PER_SECOND;
    while( timeout-- && WaitNoEvent ) {
	VideoEffect0(timeout, &callbacks);
	WaitEventsOneFrame(&callbacks);
    }
    VideoEffect0(-1, &callbacks);
#endif

    VideoLockScreen();
    VideoDrawTextCentered(VideoWidth/2,5,LargeFont,
	"----------------------------");
    VideoUnlockScreen();
    Invalidate();
    RealizeVideoMemory();

}

/**
**	Show load progress.
**
**	@param fmt	printf format string.
*/
global void ShowLoadProgress(const char* fmt,...)
{
    va_list va;
    char temp[4096];
    char* s;

    va_start(va,fmt);
    vsnprintf(temp,sizeof(temp),fmt,va);
    va_end(va);

    if( VideoDepth && IsFontLoaded(GameFont) ) {
	VideoLockScreen();
	for( s=temp; *s; ++s ) {	// Remove non printable chars
	    if( *s<32 ) {
		*s=' ';
	    }
	}
	VideoFillRectangle(ColorBlack,5,VideoHeight-18,VideoWidth-10,18);
	VideoDrawTextCentered(VideoWidth/2,VideoHeight-16,GameFont,temp);
	VideoUnlockScreen();
	InvalidateArea(5,VideoHeight-18,VideoWidth-10,18);
	RealizeVideoMemory();
    } else {
	DebugLevel0Fn("!!!!%s" _C_ temp);
    }
}

//----------------------------------------------------------------------------

/**
**	Pre menu setup.
*/
global void PreMenuSetup(void)
{
    char* s;

    //
    //  Initial menus require some gfx.
    //
    // FIXME: must search tileset by identifier or use a gui palette?
    TheMap.TerrainName=Tilesets[0]->Ident;
    LoadTileset();
    LoadRGB(GlobalPalette, s=strdcat3(StratagusLibPath,
	    "/graphics/",Tilesets[0]->PaletteFile));
    TheMap.TerrainName=NULL;
    free(s);
    VideoCreatePalette(GlobalPalette);
    SetDefaultTextColors(FontYellow,FontWhite);

    LoadFonts();

    InitVideoCursors();

    // FIXME: make the race ui configurable
    InitMenus(PlayerRaces.Race[0]);
    LoadCursors(PlayerRaces.Name[0]);
    InitSettings();

    InitUserInterface(PlayerRaces.Name[0]);
    LoadUserInterface();
}

/**
**	Menu loop.
**
**	Show the menus, start game, return back.
**
**	@param filename	map filename
**	@param map	map loaded
*/
global void MenuLoop(char* filename, WorldMap* map)
{

    for( ;; ) {
	//
	//	Clear screen
	//
	VideoLockScreen();
	VideoClearScreen();
	VideoUnlockScreen();
	Invalidate();
	RealizeVideoMemory();

	//
	//	Network part 1 (port set-up)
	//
	if( NetworkFildes!=(Socket)-1 ) {
	    ExitNetwork1();
	}
	InitNetwork1();

	//
	// Don't leak when called multiple times
	//	- FIXME: not the ideal place for this..
	//
	DebugLevel0Fn("Freeing map info, wrong place\n");
	FreeMapInfo(map->Info);
	map->Info = NULL;

	//
	//	No filename given, choose with the menus
	//
	if ( !filename ) {
	    NetPlayers = 0;

	    // Start new music for menus
	    PlaySectionMusic(PlaySectionMainMenu);

	    EnableRedraw=RedrawMenu;

	    GuiGameStarted = 0;
	    while (GuiGameStarted == 0) {
		if( EditorRunning == 2 ) {
		    SetupEditor();
		}
		if( EditorRunning ) {
		    ProcessMenu("menu-editor-select", 1);
		} else {
		    ProcessMenu("menu-program-start", 1);
		}
	    }

	    EnableRedraw=RedrawEverything;
	    DebugLevel0Fn("Menu start: NetPlayers %d\n" _C_ NetPlayers);
	    filename = CurrentMapPath;
	} else {
	    if (EditorRunning) {
		SetupEditor();
	    }
	    strcpy(CurrentMapPath,filename);
	}
	if( NetworkFildes!=(Socket)-1 && NetPlayers<2 ) {
	    ExitNetwork1();
	}

	//
	//	Start editor or game.
	//
	if( EditorRunning ) {
	    EditorMainLoop();
	} else {
	    //
	    //	Create the game.
	    //
	    CreateGame(filename,map);

	    SetStatusLine(NameLine);
	    SetMessage("Do it! Do it now!");
	    //
	    //	Play the game.
	    //
	    GameMainLoop();
	}

	CleanModules();
	CleanFonts();

	LoadCcl();			// Reload the main config file
	PlaySectionMusic(PlaySectionMainMenu);

	PreMenuSetup();

	filename=NextChapter();
	DebugLevel0Fn("Next chapter %s\n" _C_ filename);
    }
}

//----------------------------------------------------------------------------

/**
**	Print headerline, copyright, ...
*/
local void PrintHeader(void)
{
    // vvv---- looks wired, but is needed for GNU brain damage
    fprintf(stdout,"%s\n  written by Lutz Sammer, Fabrice Rossi, Vladi Shabanski, Patrice Fortier,\n  Jon Gabrielson, Andreas Arens, Nehal Mistry, Jimmy Salmon and others.\n"
    "\t(http://Stratagus.Org)"
    "\n  SIOD Copyright by George J. Carrette."
#ifdef USE_LIBMODPLUG
    "\n  libmodplug Copyright by Kenton Varda & Olivier Lapique."
#endif
    "\n  VP3 codec Copyright by On2 Technologies Inc."
#ifdef USE_SDL
    "\n  SDL Copyright by Sam Lantinga."
#endif
    "\nCompile options "
    "CCL "
#ifdef USE_THREAD
    "THREAD "
#endif
#ifdef DEBUG
    "DEBUG "
#endif
#ifdef DEBUG_FLAGS
    "DEBUG-FLAGS "
#endif
#ifdef USE_ZLIB
    "ZLIB "
#endif
#ifdef USE_BZ2LIB
    "BZ2LIB "
#endif
#ifdef USE_ZZIPLIB
    "ZZIPLIB "
#endif
#ifdef USE_SVGALIB
    "SVGALIB "
#endif
#ifdef USE_SDL
    "SDL "
#endif
#ifdef USE_SDLA
    "SDL-AUDIO "
#endif
#ifdef USE_SDLCD
    "SDL-CD "
#endif
#ifdef USE_X11
    "X11 "
#endif
#ifdef WITH_SOUND
    "SOUND "
#endif
#ifdef USE_LIBCDA
    "LIBCDA "
#endif
#ifdef USE_FLAC
    "FLAC "
#endif
#ifdef USE_OGG
    "OGG "
#endif
#ifdef USE_MAD
    "MP3 "
#endif
    // New features:
    "\nCompile feature "
#ifdef UNIT_ON_MAP
    "UNIT-ON-MAP "
#endif
#ifdef UNITS_ON_MAP
    "UNITS-ON-MAP "
#endif
#ifdef NEW_MAPDRAW
    "NEW-MAPDRAW "
#endif
#ifdef HIERARCHIC_PATHFINDER
    "HIERARCHIC-PATHFINDER "
#endif
#ifdef SLOW_INPUT
    "SLOW-INPUT "
#endif
	,NameLine);
}

/**
**	Main1, called from main.
**
**	@param	argc	Number of arguments.
**	@param	argv	Vector of arguments.
*/
global int main1(int argc __attribute__ ((unused)),
	char** argv __attribute__ ((unused)))
{
    int i;

    PrintHeader();
    printf(
    "\n\nStratagus may be copied only under the terms of the GNU General Public License\
\nwhich may be found in the Stratagus source kit."
    "\n\nDISCLAIMER:\n\
This software is provided as-is.  The author(s) can not be held liable for any\
\ndamage that might arise from the use of this software.\n\
Use it at your own risk.\n\n");

    //
    //	Hardware drivers setup
    //
    InitVideo();			// setup video display
#ifdef WITH_SOUND
    if( !SoundOff && InitSound() ) {			// setup sound card
	SoundOff=1;
	SoundFildes=-1;
    }
#endif

#ifndef DEBUG				// For debug its better not to have:
    srand(time(NULL));			// Random counter = random each start
#endif

    InitMovie();

    //
    //	Show title screen.
    //
    i=1;
    SetClipping(0,0,VideoWidth-1,VideoHeight-1);
    if( TitleScreen ) {
	if( (i=PlayMovie(TitleScreen,
		PlayMovieZoomScreen|PlayMovieKeepAspect)) ) {
	    DisplayPicture(TitleScreen);
	    Invalidate();
	}
    }

    InitUnitsMemory();		// Units memory management
    PreMenuSetup();		// Load everything needed for menus

    if( i ) {
	WaitForInput(20);		// Show game intro
    }

    MenuLoop(MapName,&TheMap);	// Enter the menu loop

    return 0;
}

/**
**	Exit clone.
**
**	Called from ALT-'X' key or exit game menus.
**
**	@param err	Error code to parse to shell.
*/
global volatile void Exit(int err)
{
    IfDebug(
	extern unsigned PfCounterFail;
	extern unsigned PfCounterOk;
	extern unsigned PfCounterDepth;
	extern unsigned PfCounterNotReachable;
    );

    StopMusic();
    QuitSound();
    QuitCD();
    NetworkQuit();

    ExitNetwork1();
    IfDebug(
	DebugLevel0( "Frames %lu, Slow frames %d = %ld%%\n"
	    _C_ FrameCounter _C_ SlowFrameCounter
	    _C_ (SlowFrameCounter*100)/(FrameCounter ? FrameCounter : 1) );
	UnitCacheStatistic();
	DebugLevel0("Path: Error: %u Unreachable: %u OK: %u Depth: %u\n"
		_C_ PfCounterFail _C_ PfCounterNotReachable
		_C_ PfCounterOk _C_ PfCounterDepth);
    );
#ifdef DEBUG
    CclUnits();
    CleanModules();
    CleanFonts();
#endif

    CleanMovie();

    fprintf(stderr,"Thanks for playing Stratagus.\n");
    exit(err);
}

/**
**	Do a fatal exit.
**
**	Called on out of memory or crash.
**
**	@param err	Error code to parse to shell.
*/
global volatile void ExitFatal(int err)
{
    QuitCD();
    exit(err);
}

/**
**	Display the usage.
*/
local void Usage(void)
{
    PrintHeader();
    printf(
"\n\nUsage: stratagus [OPTIONS] [map.pud|map.pud.gz|map.cm|map.cm.gz]\n\
\t-c file.ccl\tccl start file (default stratagus.ccl)\n\
\t-d datapath\tpath to stratagus data\n\
\t-e\t\tStart editor\n\
\t-f factor\tComputer units cost factor\n\
\t-h\t\tHelp shows this page\n\
\t-l\t\tDisable command log\n\
\t-P port\t\tNetwork port to use\n\
\t-n server\tNetwork server host preset\n\
\t-L lag\t\tNetwork lag in # frames (default 10 = 333ms)\n\
\t-U update\tNetwork update rate in # frames (default 5=6x per s)\n\
\t-N name\t\tName of the player\n\
\t-s sleep\tNumber of frames for the AI to sleep before it starts\n\
\t-t factor\tComputer units built time factor\n\
\t-v mode\t\tVideo mode (0=default,1=640x480,2=800x600,\n\
\t\t\t\t3=1024x768,4=1280x960,5=1600x1200)\n\
\t-w\t\tWait for sound device (OSS sound driver only)\n\
\t-D\t\tVideo mode depth = pixel per point (for Win32/TNT)\n\
\t-F\t\tFull screen video mode (only supported with SDL)\n\
\t-S\t\tSync speed (100 = 30 frames/s)\n\
\t-W\t\tWindowed video mode (only supported with SDL)\n\
map is relative to StratagusLibPath=datapath, use ./map for relative to cwd\n\
");
}

/**
**	The main program: initialise, parse options and arguments.
**
**	@param	argc	Number of arguments.
**	@param	argv	Vector of arguments.
*/
global int main(int argc,char** argv)
{
#ifdef USE_BEOS
    //
    //	Parse arguments for BeOS
    //
    beos_init( argc, argv );
#endif

    //
    //	Setup some defaults.
    //
#ifndef __APPLE__
    StratagusLibPath=STRATAGUS_LIB_PATH;
#endif
    CclStartFile="ccl/stratagus.ccl";
    EditorStartFile="ccl/editor.ccl";

    memset(LocalPlayerName, 0, 16);
    strcpy(LocalPlayerName, "Anonymous");

    // FIXME: Parse options before or after ccl?

    //
    //	Parse commandline
    //
    for( ;; ) {
	switch( getopt(argc,argv,"c:d:ef:hln:P:s:t:v:wD:N:E:FL:S:U:W?") ) {
	    case 'c':
		CclStartFile=optarg;
		continue;
            case 'd':
                StratagusLibPath=optarg;
                continue;
	    case 'e':
		EditorRunning=2;
		continue;
	    case 'E':
		EditorStartFile=optarg;
		continue;
	    case 'f':
		AiCostFactor=atoi(optarg);
		continue;
	    case 'l':
		CommandLogDisabled=1;
		continue;
	    case 'P':
		NetworkPort=atoi(optarg);
		continue;
	    case 'n':
		NetworkArg=strdup(optarg);
		continue;
	    case 'N':
		memset(LocalPlayerName, 0, 16);
		strncpy(LocalPlayerName, optarg, 16);
		continue;
	    case 's':
		AiSleepCycles=atoi(optarg);
		continue;
	    case 't':
		AiTimeFactor=atoi(optarg);
		continue;
	    case 'v':
		switch( atoi(optarg) ) {
		    case 0:
			continue;
		    case 1:
			VideoWidth=640;
			VideoHeight=480;
			continue;
		    case 2:
			VideoWidth=800;
			VideoHeight=600;
			continue;
		    case 3:
			VideoWidth=1024;
			VideoHeight=768;
			continue;
		    case 4:
			VideoWidth=1280;
			VideoHeight=960;
			continue;
		    case 5:
			VideoWidth=1600;
			VideoHeight=1200;
			continue;
		    default:
			Usage();
			ExitFatal(-1);
		}
		continue;

	    case 'w':
		WaitForSoundDevice=1;
		continue;

	    case 'L':
		NetworkLag=atoi(optarg);
		if( !NetworkLag ) {
		    fprintf(stderr,"FIXME: zero lag not supported\n");
		    Usage();
		    ExitFatal(-1);
		}
		continue;
	    case 'U':
		NetworkUpdates=atoi(optarg);
		continue;

	    case 'F':
		VideoFullScreen=1;
		continue;
	    case 'W':
		VideoFullScreen=0;
		continue;
	    case 'D':
		VideoDepth=atoi(optarg);
		continue;
	    case 'S':
		VideoSyncSpeed=atoi(optarg);
		continue;

	    case -1:
		break;
	    case '?':
	    case 'h':
	    default:
		Usage();
		ExitFatal(-1);
	}
	break;
    }

    if( argc-optind>1 ) {
	fprintf(stderr,"too many files\n");
	Usage();
	ExitFatal(-1);
    }

    if( argc-optind ) {
	MapName=argv[optind];
	--argc;
    }

    InitCcl();				// init CCL and load configurations!
    LoadCcl();

    main1(argc,argv);

    return 0;
}

//@}
