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
/**@name clone.c	-	The main file. */
//
//	(c) Copyright 1998-2002 by Lutz Sammer
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

/**
**	@mainpage
**
**	@section Introduction Introduction
**
**	Welcome to the source code documentation of the FreeCraft engine.
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
**	Visit the http://FreeCraft.Org web page for the latest news and
**	../doc/readme.html for other documentations.
**
**	@section Modules Modules
**
**	This are the main modules of the FreeCraft engine.
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
**		@see unit.h @see unit.c
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
**		@todo ogg and mp3 file format support for background music
**		@todo flac and adpcm file format support for sound effects
**		@todo better separation of low and high level, assembler mixing
**			support.
**
**		@see sound.h @see sound.c
**		@see ccl_sound.c @see sound_id.c @see sound_server.c
**		@see unitsound.c
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
**		The new is very flexible, but very basic. It includes none
**		optimations.
**
**		@see new_ai.c ai_local.h
**		@see ai.h @see ai.c
**
**	@subsection CCL CCL
**
**		CCL is Craft Configuration Language, which is used to
**		configure and customize FreeCraft.
**
**		@see ccl.h @see ccl.c
**
**	@subsection Icon Icon
**
**		@see icons.h @see icons.c
**
*/

/*----------------------------------------------------------------------------
--	Includes
----------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#ifdef USE_BEOS
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
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
#endif

#ifdef __MINGW32__
#include <SDL.h>
extern int opterr;
extern int optind;
extern int optopt;
extern char *optarg;

extern int getopt(int argc, char *const*argv, const char *opt);
#endif

#include "freecraft.h"
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

#ifdef DEBUG
extern SCM CclUnits(void);
#endif

/*----------------------------------------------------------------------------
--	Variables
----------------------------------------------------------------------------*/

global char* TitleScreen;		/// titlescreen to show at startup
global char* MenuBackground;		/// file for menu background
global char* MenuBackgroundWithTitle;	/// file for menu with title
global char* TitleMusic;		/// file for title music
global char* MenuMusic;			/// file for menu music
global char* FreeCraftLibPath;		/// path for data directory

    /// Name, Version, Copyright
global char NameLine[] =
    "FreeCraft V" VERSION ", (c) 1998-2002 by The FreeCraft Project.";

    /// Filename of the map to load
local char* MapName;

//FIXME: all game global options should be moved in structure like `TheUI'
global int OptionUseDepletedMines=0;    /// use depleted mines or destroy them

/*----------------------------------------------------------------------------
--	Speedups
----------------------------------------------------------------------------*/

global int SpeedMine=SPEED_MINE;	/// speed factor for mine gold
global int SpeedGold=SPEED_GOLD;	/// speed factor for getting gold
global int SpeedChop=SPEED_CHOP;	/// speed factor for chop
global int SpeedWood=SPEED_WOOD;	/// speed factor for getting wood
global int SpeedHaul=SPEED_HAUL;	/// speed factor for haul oil
global int SpeedOil=SPEED_OIL;		/// speed factor for getting oil
global int SpeedBuild=SPEED_BUILD;	/// speed factor for building
global int SpeedTrain=SPEED_TRAIN;	/// speed factor for training
global int SpeedUpgrade=SPEED_UPGRADE;	/// speed factor for upgrading
global int SpeedResearch=SPEED_RESEARCH;/// speed factor for researching

/*============================================================================
==	DISPLAY
============================================================================*/

// FIXME: move to video header file
global int VideoWidth;			/// window width in pixels
global int VideoHeight;			/// window height in pixels

global unsigned long NextFrameTicks;	/// Ticks of begin of the next frame
global int FrameCounter;		/// current frame number
global int SlowFrameCounter;		/// profile, frames out of sync

// FIXME: not the correct place
global enum MustRedraw_e MustRedraw=RedrawEverything;	/// redraw flags

global unsigned long GameCycle;		/// Game simulation cycle counter

/*----------------------------------------------------------------------------
--	Random
----------------------------------------------------------------------------*/

global unsigned SyncRandSeed = 0x87654321;	/// sync random seed value.

/**
**	Syncron rand.
*/
global int SyncRand(void)
{
    int val;

    val=SyncRandSeed>>16;

    SyncRandSeed=SyncRandSeed*(0x12345678 * 4 + 1) + 1;

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

/*============================================================================
==	MAIN
============================================================================*/

local int WaitNoEvent;			/// Flag got an event.

/**
**	Callback for input.
*/
local void WaitCallbackKey(unsigned dummy __attribute__((unused)))
{
    DebugLevel3Fn("Pressed %8x %8x\n",MouseButtons,dummy);
    WaitNoEvent=0;
}

/**
**	Callback for input.
*/
local void WaitCallbackKey2(unsigned dummy1 __attribute__((unused)),
	unsigned dummy2 __attribute__((unused)))
{
    DebugLevel3Fn("Pressed %8x %8x %8x\n",MouseButtons,dummy1,dummy2);
    WaitNoEvent=0;
}

/**
**	Callback for input.
*/
local void WaitCallbackMouse(int dummy_x __attribute__((unused)),
	int dummy_y __attribute__((unused)))
{
    DebugLevel3Fn("Moved %d,%d\n",dummy_x,dummy_y);
}

/**
**	Callback for exit.
*/
local void WaitCallbackExit(void)
{
    DebugLevel3Fn("Exit\n");
}

/**
**	Wait for any input.
**
**	@param timeout	Time in seconds to wait.
*/
local void WaitForInput(int timeout)
{
    EventCallback callbacks;
#ifdef linux
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

    callbacks.NetworkEvent=NetworkEvent;
    callbacks.SoundReady=WriteSound;

    //
    //	FIXME: more work needed, scrolling credits, animations, ...
    VideoLockScreen();
    //VideoDrawTextCentered(VideoWidth/2,5,LargeTitleFont,"Press SPACE to continue.");
    VideoDrawTextCentered(VideoWidth/2,5,LargeFont,"Press SPACE to continue.");
#ifdef linux
    ddfile=popen("`which ddate`","r");
    fgets(ddate,72,ddfile);
    pclose(ddfile);
    VideoDrawTextCentered(VideoWidth/2,20,LargeFont,ddate);
#endif
    VideoUnlockScreen();
    Invalidate();
    RealizeVideoMemory();

    WaitNoEvent=1;
    timeout*=CYCLES_PER_SECOND;
    while( timeout-- && WaitNoEvent ) {
	WaitEventsOneFrame(&callbacks);
    }

    VideoLockScreen();
    VideoDrawTextCentered(VideoWidth/2,5,LargeFont,"----------------------------");
    VideoUnlockScreen();
    Invalidate();
    RealizeVideoMemory();
}

//----------------------------------------------------------------------------

/**
**	Pre menu setup.
*/
local void PreMenuSetup(void)
{
    char* s;

    //
    //  Inital menues require some gfx.
    //
    // FIXME: must search tileset by identifier or use a gui palette?
    LoadRGB(GlobalPalette, s=strdcat3(FreeCraftLibPath,
	    "/graphics/",Tilesets[TilesetSummer]->PaletteFile));
    free(s);
    VideoCreatePalette(GlobalPalette);
    LoadFonts();

    InitVideoCursors();

    // All pre-start menues are orcish - may need to be switched later..
    SetDefaultTextColors(FontYellow,FontWhite);
    InitMenus(PlayerRaceOrc);
    LoadCursors(RaceWcNames ? RaceWcNames[1] : "oops");
    InitSettings();

    InitUserInterface(RaceWcNames ? RaceWcNames[1] : "oops");
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
	VideoFillRectangle(ColorBlack,0,0,VideoWidth,VideoHeight);
	VideoUnlockScreen();
	Invalidate();
	RealizeVideoMemory();
	//
	//	Network part 1 (port set-up)
	//	FIXME: JOHNS: -> ARI can this be called multiple?
	//
	InitNetwork1();
	//
	// Don't leak when called multiple times
	//	- FIXME: not the ideal place for this..
	//
	FreeMapInfo(map->Info);
	map->Info = NULL;
	//
	//	No filename given, choose with the menus
	//
	if ( !filename ) {
	    // Start new music for menus?
	    // FIXME: If second loop?
	    if( strcmp(TitleMusic,MenuMusic) ) {
		PlayMusic(MenuMusic);
	    }
	    ProcessMenu(MENU_PRG_START, 1);
	    if( NetworkFildes!=-1 && NetPlayers<2 ) {
		ExitNetwork1();
	    }
	}
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

	CleanModules();
	CleanFonts();

	LoadCcl();			// Reload the main config file

	PreMenuSetup();

	filename=NextChapter();
	DebugLevel0Fn("Next chapter %s\n",filename);
    }
}

//----------------------------------------------------------------------------

/**
**	Print headerline, copyright, ...
*/
local void PrintHeader(void)
{
    // vvv---- looks wired, but is needed for GNU brain damage
    fprintf(stdout,"%s\n  written by Lutz Sammer, Fabrice Rossi, Vladi Shabanski, Patrice Fortier,\n  Jon Gabrielson, Andreas Arens and others. (http://FreeCraft.Org)"
    "\n  SIOD Copyright by George J. Carrette."
    "\n  libmodplug Copyright by Kenton Varda & Olivier Lapique."
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
#ifdef NEW_FOW
    "NEW-FOW "
#endif
#ifdef NEW_AI
    "NEW-AI "
#endif
#ifdef NEW_SHIPS
    "NEW-SHIPS "
#endif
#ifdef NEW_NETMENUS
    "NEW-NETMENUS "
#endif
#ifdef HIERARCHIC_PATHFINDER
    "HIERARCHIC-PATHFINDER "
#endif
#ifdef SLOW_INPUT
    "SLOW-INPUT "
#endif
#ifdef HAVE_EXPANSION
    "EXPANSION "
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
#ifndef DEBUG
    srand(time(NULL));
#endif

    PrintHeader();
    printf(
    "\n\nFreeCraft may be copied only under the terms of the GNU General Public License\
\nwhich may be found in the FreeCraft source kit."
    "\n\nDISCLAIMER:\n\
This software is provided as-is.  The author(s) can not be held liable for any\
\ndamage that might arise from the use of this software.\n\
Use it at your own risk.\n\n");

    //
    //	Hardware drivers setup
    //
    InitVideo();			// setup video display
#ifdef WITH_SOUND
    if( InitSound() ) {			// setup sound card
	SoundOff=1;
	SoundFildes=-1;
    }
#endif

    //
    //	Show title screen.
    //
    SetClipping(0,0,VideoWidth-1,VideoHeight-1);
    if( TitleScreen ) {
	DisplayPicture(TitleScreen);
    }
    Invalidate();

    InitUnitsMemory();		// Units memory management
    PreMenuSetup();		// Load everything needed for menus
    WaitForInput(15);		// Show game intro
    NetworkSetupArgs();		// Evaluate optional command line parameters

    MenuLoop(MapName,&TheMap);	// Enter the menu loop

    return 0;
}

/**
**	Exit clone.
**
**	Called from ALT-'X' key or exit game menus.
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
    NetworkQuit();

    ExitNetwork1();
    IfDebug(
	DebugLevel0( "Frames %d, Slow frames %d = %d%%\n"
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
    fprintf(stderr,"Thanks for playing FreeCraft.\n");
    exit(err);
}

/**
**	Display the usage.
*/
local void Usage(void)
{
    PrintHeader();
    printf(
"\n\nUsage: freecraft [OPTIONS] [map.pud|map.pud.gz|map.cm|map.cm.gz]\n\
\t-d datapath\tpath to freecraft data\n\
\t-c file.ccl\tccl start file\n\
\t-f factor\tComputer units cost factor\n\
\t-h\t\tHelp shows this page\n\
\t-l\t\tEnable command log to \"command.log\"\n\
\t-p players\tNumber of players\n\
\t-P port\t\tNetwork port to use (menu code only)\n\
\t-n [localport:]host[:port]\tNetwork argument (port default 6660)\n\
\t-L lag\t\tNetwork lag in # frames (default 5 = 165ms)\n\
\t-U update\tNetwork update rate in # frames (default 5=6x pro s)\n\
\t-N name\t\tName of the player\n\
\t-s sleep\tNumber of frames for the AI to sleep before it starts\n\
\t-t factor\tComputer units built time factor\n\
\t-v mode\t\tVideo mode (0=default,1=640x480,2=800x600,\n\
\t-w\t\tWait for sound device (OSS sound driver only)\n\
\t\t\t\t3=1024x768,4=1600x1200)\n\
\t-D\t\tVideo mode depth = pixel per point (for Win32/TNT)\n\
\t-F\t\tFull screen video mode (only supported with SDL)\n\
\t-S\t\tSync speed (100 = 30 frames/s)\n\
\t-W\t\tWindowed video mode (only supported with SDL)\n\
map is relative to FreeCraftLibPath=datapath, use ./map for relative to cwd\n\
");
}

/**
**	The main program: initialise, parse options and arguments.
**
**	@param	argc	Number of arguments.
**	@param	argv	Vector of arguments.
*/
#if defined(__MINGW32__) || defined(__CYGWIN__)
global int mymain(int argc,char** argv)
#else
global int main(int argc,char** argv)
#endif
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
    FreeCraftLibPath=FREECRAFT_LIB_PATH;
    CclStartFile="ccl/freecraft.ccl";

    memset(NetworkName, 0, 16);
    strcpy(NetworkName, "Anonymous");

    // FIXME: Parse options before or after ccl?

    //
    //	Parse commandline
    //
    for( ;; ) {
	switch( getopt(argc,argv,"c:d:f:hln:p:P:s:t:v:wD:N:FL:S:U:W?") ) {
	    case 'c':
		CclStartFile=optarg;
		continue;
            case 'd':
                FreeCraftLibPath=optarg;
                continue;
	    case 'f':
		AiCostFactor=atoi(optarg);
		continue;
	    case 'l':
		CommandLogEnabled=1;
		continue;
	    case 'p':
		NetPlayers=atoi(optarg);
		continue;
	    case 'P':
		NetworkPort=atoi(optarg);
		continue;
	    case 'n':
		NetworkArg=strdup(optarg);
		continue;
	    case 'N':
		memset(NetworkName, 0, 16);
		strncpy(NetworkName, optarg, 16);
		continue;
	    case 's':
		AiSleep=atoi(optarg);
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
			VideoWidth=1600;
			VideoHeight=1200;
			continue;
		    default:
			Usage();
			exit(-1);
		}
		continue;

	    case 'w':
		WaitForSoundDevice=1;
		continue;

	    case 'L':
		NetworkLag=atoi(optarg);
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
		exit(-1);
	}
	break;
    }

    if( argc-optind>1 ) {
	fprintf(stderr,"too many files\n");
	Usage();
	exit(-1);
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
