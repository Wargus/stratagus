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
/**@name stratagus.cpp - The main file. */
//
//      (c) Copyright 1998-2006 by Lutz Sammer, Francois Beerten, and
//                                 Jimmy Salmon
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

/**
** @mainpage
**
** @section Introduction Introduction
**
** Welcome to the source code documentation of the Stratagus engine.
** Extract the source documentation with doxygen (http://www.doxygen.org)
** or doc++ (http://www.zib.de/Visual/software/doc++/index.html) tools.
**
** Any help to improve this documention is welcome. If you didn't
** understand something or you found an error or a wrong spelling
** or wrong grammer please write an email (including a patch :).
**
** @section Informations Informations
**
** Visit the http://stratagus.org web page for the latest news and
** ../doc/readme.html for other documentations.
**
** @section Modules Modules
**
** This are the main modules of the Stratagus engine.
**
** @subsection Map Map
**
** Handles the map. A map is made from tiles.
**
** @see map.h @see map.cpp @see tileset.h @see tileset.cpp
**
** @subsection Unit Unit
**
** Handles units. Units are ships, flyers, buildings, creatures,
** machines.
**
** @see unit.h @see unit.cpp @see unittype.h @see unittype.cpp
**
** @subsection Missile Missile
**
** Handles missiles. Missiles are all other sprites on map
** which are no unit.
**
** @see missile.h @see missile.cpp
**
** @subsection Player Player
**
** Handles players, all units are owned by a player. A player
** could be controlled by a human or a computer.
**
** @see player.h @see player.cpp @see ::CPlayer
**
** @subsection Sound Sound
**
** Handles the high and low level of the sound. There are the
** background music support, voices and sound effects.
** Following low level backends are supported: OSS and SDL.
**
** @todo adpcm file format support for sound effects
** @todo better separation of low and high level, assembler mixing
** support.
** @todo Streaming support of ogg/mp3 files.
**
** @see sound.h @see sound.cpp
** @see script_sound.cpp @see sound_id.cpp @see sound_server.cpp
** @see unitsound.cpp
** @see sdl_audio.cpp
** @see ogg.cpp @see wav.cpp
**
** @subsection Video Video
**
** Handles the high and low level of the graphics.
** This also contains the sprite and linedrawing routines.
**
** See page @ref VideoModule for more information upon supported
** features and video platforms.
**
** @see video.h @see video.cpp
**
** @subsection Network Network
**
** Handles the high and low level of the network protocol.
** The network protocol is needed for multiplayer games.
**
** See page @ref NetworkModule for more information upon supported
** features and API.
**
** @see network.h @see network.cpp
**
** @subsection Pathfinder Pathfinder
**
** @see pathfinder.h @see pathfinder.cpp
**
** @subsection AI AI
**
** There are currently two AI's. The old one is very hardcoded,
** but does things like placing buildings better than the new.
** The old AI shouldn't be used.  The new is very flexible, but
** very basic. It includes none optimations.
**
** See page @ref AiModule for more information upon supported
** features and API.
**
** @see ai_local.h
** @see ai.h @see ai.cpp
**
** @subsection CCL CCL
**
** CCL is Craft Configuration Language, which is used to
** configure and customize Stratagus.
**
** @see script.h @see script.cpp
**
** @subsection Icon Icon
**
** @see icons.h @see icons.cpp
**
** @subsection Editor Editor
**
** This is the integrated editor, it shouldn't be a perfect
** editor. It is used to test new features of the engine.
**
** See page @ref EditorModule for more information upon supported
** features and API.
**
** @see editor.h @see editor.cpp
*/

/*----------------------------------------------------------------------------
--  Includes
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

extern void beos_init(int argc, char **argv);

#endif

#ifndef _MSC_VER
#include <unistd.h>
#endif
#ifdef __CYGWIN__
#include <getopt.h>
#endif
#if defined(_MSC_VER) || defined(__MINGW32__)
extern char *optarg;
extern int optind;
extern int getopt(int argc, char *const *argv, const char *opt);
#endif

#ifdef MAC_BUNDLE
#define Button ButtonOSX
#include <Carbon/Carbon.h>
#undef Button
#endif

#include "SDL.h"

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
#include "script.h"
#include "network.h"
#include "netconnect.h"
#include "ai.h"
#include "commands.h"
#include "campaign.h"
#include "editor.h"
#include "movie.h"
#include "pathfinder.h"
#include "widgets.h"
#include "iolib.h"

#ifdef DEBUG
extern int CclUnits(lua_State *l);
#endif

/*----------------------------------------------------------------------------
--  Variables
----------------------------------------------------------------------------*/

TitleScreen **TitleScreens;          /// Title screens to show at startup
char *StratagusLibPath;              /// Path for data directory
char LocalPlayerName[16];            /// Name of local player

	/// Name, Version, Copyright
char NameLine[] =
	"Stratagus V" VERSION ", (c) 1998-2006 by The Stratagus Project.";

static char *MapName;                /// Filename of the map to load
char *CompileOptions;                /// Compile options.

/*----------------------------------------------------------------------------
--  Speedups FIXME: Move to some other more logic place
----------------------------------------------------------------------------*/

int SpeedResourcesHarvest[MaxCosts]; /// speed factor for harvesting resources
int SpeedResourcesReturn[MaxCosts];  /// speed factor for returning resources
int SpeedBuild = 1;                  /// speed factor for building
int SpeedTrain = 1;                  /// speed factor for training
int SpeedUpgrade = 1;                /// speed factor for upgrading
int SpeedResearch = 1;               /// speed factor for researching

/*============================================================================
==  DISPLAY
============================================================================*/

unsigned long GameCycle;             /// Game simulation cycle counter
unsigned long FastForwardCycle;      /// Cycle to fastforward to in a replay

/*============================================================================
==  MAIN
============================================================================*/

static bool WaitNoEvent;                     /// Flag got an event

/**
**  Callback for input.
*/
static void WaitCallbackButtonPressed(unsigned dummy)
{
	WaitNoEvent = false;
}

/**
**  Callback for input.
*/
static void WaitCallbackButtonReleased(unsigned dummy)
{
}

/**
**  Callback for input.
*/
static void WaitCallbackKeyPressed(unsigned dummy1, unsigned dummy2)
{
	WaitNoEvent = false;
}

/**
**  Callback for input.
*/
static void WaitCallbackKeyReleased(unsigned dummy1, unsigned dummy2)
{
}

/**
**  Callback for input.
*/
static void WaitCallbackKeyRepeated(unsigned dummy1, unsigned dummy2)
{
}

/**
**  Callback for input.
*/
static void WaitCallbackMouse(int x, int y)
{
}

/**
**  Callback for exit.
*/
static void WaitCallbackExit(void)
{
}

/**
**  Show a title image
*/
static void ShowTitleImage(TitleScreen *t)
{
	EventCallback callbacks;
	CGraphic *g;

	WaitNoEvent = true;

	callbacks.ButtonPressed = WaitCallbackButtonPressed;
	callbacks.ButtonReleased = WaitCallbackButtonReleased;
	callbacks.MouseMoved = WaitCallbackMouse;
	callbacks.MouseExit = WaitCallbackExit;
	callbacks.KeyPressed = WaitCallbackKeyPressed;
	callbacks.KeyReleased = WaitCallbackKeyReleased;
	callbacks.KeyRepeated = WaitCallbackKeyRepeated;
	callbacks.NetworkEvent = NetworkEvent;

	g = CGraphic::New(t->File);
	g->Load();
	g->Resize(Video.Width, Video.Height);

	int timeout = t->Timeout * CYCLES_PER_SECOND;
	if (!timeout) {
		timeout = -1;
	}

	while (timeout-- && WaitNoEvent) {
		g->DrawSubClip(0, 0, g->Width, g->Height,
			(Video.Width - g->Width) / 2, (Video.Height - g->Height) / 2);
		TitleScreenLabel **labels = t->Labels;
		if (labels && labels[0] && labels[0]->Font &&
				labels[0]->Font->IsLoaded()) {
			for (int j = 0; labels[j]; ++j) {
				// offsets are for 640x480, scale up to actual resolution
				int x = labels[j]->Xofs * Video.Width / 640;
				int y = labels[j]->Yofs * Video.Width / 640;
				if (labels[j]->Flags & TitleFlagCenter) {
					x -= labels[j]->Font->Width(labels[j]->Text) / 2;
				}
				VideoDrawText(x, y, labels[j]->Font, labels[j]->Text);
			}
		}

		Invalidate();
		RealizeVideoMemory();
		WaitEventsOneFrame(&callbacks);
	}

	CGraphic::Free(g);
}

/**
**  Show the title screens
*/
static void ShowTitleScreens(void)
{
	if (!TitleScreens) {
		return;
	}

	SetVideoSync();

	for (int i = 0; TitleScreens[i]; ++i) {
		if (TitleScreens[i]->Music) {
			if (!strcmp(TitleScreens[i]->Music, "none") ||
					PlayMusic(TitleScreens[i]->Music) == -1) {
				StopMusic();
			}
		}

		if (PlayMovie(TitleScreens[i]->File)) {
			ShowTitleImage(TitleScreens[i]);
		}

		Video.ClearScreen();
	}
	Invalidate();
}

/**
**  Show load progress.
**
**  @param fmt  printf format string.
*/
void ShowLoadProgress(const char *fmt, ...)
{
	va_list va;
	char temp[4096];

	va_start(va, fmt);
	vsnprintf(temp, sizeof(temp) - 1, fmt, va);
	temp[sizeof(temp) - 1] = '\0';
	va_end(va);

	if (Video.Depth && GameFont && GameFont->IsLoaded()) {
		// Remove non printable chars
		for (char *s = temp; *s; ++s) {
			if (*s < 32) {
				*s = ' ';
			}
		}
		Video.FillRectangle(ColorBlack, 5, Video.Height - 18, Video.Width - 10, 18);
		VideoDrawTextCentered(Video.Width / 2, Video.Height - 16, GameFont, temp);
		InvalidateArea(5, Video.Height - 18, Video.Width - 10, 18);
		RealizeVideoMemory();
	} else {
		DebugPrint("!!!!%s\n" _C_ temp);
	}
}

//----------------------------------------------------------------------------

/**
**  Pre menu setup.
*/
void PreMenuSetup(void)
{
	//
	//  Initial menus require some gfx.
	//
	SetDefaultTextColors(FontYellow, FontWhite);

	LoadFonts();

	InitVideoCursors();

	LoadCursors(PlayerRaces.Name[0]);
	InitSettings();

	InitUserInterface();
	UI.Load();
}

#if 0
/**
**  Menu loop.
**
**  Show the menus, start game, return back.
**
**  @param filename  map filename
**  @param map       map loaded
*/
void MenuLoop(const char *filename, CMap *map)
{
	oldMenusRunning = true;
	for (;;) {
		//
		//  Clear screen
		//
		Video.ClearScreen();
		Invalidate();

		//
		//  Network part 1 (port set-up)
		//
		if (IsNetworkGame()) {
			ExitNetwork1();
		}
		InitNetwork1();

		//
		//  Don't leak when called multiple times
		//    - FIXME: not the ideal place for this..
		//
		DebugPrint("Freeing map info, wrong place\n");
		FreeMapInfo(&map->Info);

		//
		//  No filename given, choose with the menus
		//
		if (!filename) {
			NetPlayers = 0;

			// Start new music for menus
			if (IsMusicPlaying() && MenuMusic != NULL &&
					strcmp(CurrentMusicFile, MenuMusic)) {
				StopMusic();
			}

			if (!IsMusicPlaying() && MenuMusic) {
				PlayMusic(MenuMusic);
			}

			GuiGameStarted = 0;
			while (GuiGameStarted == 0) {
				int old_video_sync;

				old_video_sync = VideoSyncSpeed;
				VideoSyncSpeed = 100;
				SetVideoSync();
				if (Editor.Running == EditorCommandLine) {
					SetupEditor();
				}
				if (Editor.Running) {
					ProcessMenu("menu-editor-select", 1);
				} else {
					ProcessMenu("menu-program-start", 1);
				}
				VideoSyncSpeed = old_video_sync;
				SetVideoSync();
			}

			DebugPrint("Menu start: NetPlayers %d\n" _C_ NetPlayers);
			filename = CurrentMapPath;
		} else {
			if (Editor.Running) {
				SetupEditor();
			}
			strcpy(CurrentMapPath, filename);
		}
		if (IsNetworkGame() && NetPlayers < 2) {
			GameSettings.Presets[0].Race = GameSettings.Presets[Hosts[0].PlyNr].Race;
			ExitNetwork1();
		}

		//
		//  Start editor or game.
		//
		if (Editor.Running) {
			EditorMainLoop();
		} else {
			//
			//  Create the game.
			//
			DebugPrint("Creating game with map: %s\n" _C_ filename);
			CreateGame(filename, map);

			UI.StatusLine.Set(NameLine);
			SetMessage(_("Do it! Do it now!"));
			//
			//  Play the game.
			//
			GameMainLoop();
		}

		CleanModules();

		// Reload the main config file
		LoadCcl();

		PreMenuSetup();

		filename = NextChapter();
		if (filename && filename != CurrentMapPath) {
			sprintf(CurrentMapPath, "%s/%s", StratagusLibPath, filename);
			filename = CurrentMapPath;
			DebugPrint("Next chapter %s\n" _C_ CurrentMapPath);
		}
	}
}
#endif

/**
**  Run the guichan main menus loop.
**
**  @param filename  map filename
**  @param map       map loaded
**
**  @return      0 in success, else exit.
*/
static int MenuLoop(const char *filename, CMap *map)
{
	char buf[1024];
	int status;

	initGuichan(Video.Width, Video.Height);
	InterfaceState = IfaceStateMenu;
	//  Clear screen
	Video.ClearScreen();
	Invalidate();

	ButtonUnderCursor = -1;
	CursorState = CursorStatePoint;
	GameCursor = UI.Point.Cursor;

	// FIXME delete this when switching to full guichan GUI
	LibraryFileName("scripts/guichan.lua", buf);
	status = LuaLoadFile(buf);
	if (status == 0) {
		CleanModules();
	}

	freeGuichan();
	return status;
}

extern void CleanMissiles();
extern void InitDefinedVariables();
extern void CleanTriggers();
/**
**  Cleanup game.
**
**  Call each module to clean up.
**  Contrary to CleanModules, maps can be restarted
**  without reloading all lua files.
*/
void CleanGame(void)
{
	EndReplayLog();
	CleanMessages();

	CleanTriggers();
	CleanUnits();
	CleanSelections();
	CleanGroups();
	CleanMissiles();
	CleanTilesets();
	Map.Clean();
	CleanReplayLog();
	FreeVisionTable();
	FreeAStar();
}

static void ExpandPath(char *newpath, const char *path)
{
#ifndef WIN32
	char *s;
#endif

	if (*path == '~') {
		++path;
#ifndef WIN32
		if ((s = getenv("HOME")) && GameName) {
			sprintf(newpath, "%s/%s/%s/%s",
				s, STRATAGUS_HOME_PATH, GameName, path);
		} else
#endif
		{
			sprintf(newpath, "%s/%s", GameName, path);
		}
	} else {
		sprintf(newpath, "%s/%s", StratagusLibPath, path);
	}
}

void StartMap(const char *filename, bool clean = true) 
{
	GuichanActive = false;
	NetConnectRunning = 0;
	InterfaceState = IfaceStateNormal;

	//  Create the game.
	DebugPrint("Creating game with map: %s\n" _C_ filename);
	if (clean) {
		CleanPlayers();
	}
	CreateGame(filename, &Map);

	UI.StatusLine.Set(NameLine);
	SetMessage(_("Do it! Do it now!"));

	//  Play the game.
	GameMainLoop();

	//  Clear screen
	Video.ClearScreen();
	Invalidate();
	GuichanActive = true;

	CleanGame();
	InterfaceState = IfaceStateMenu;
}

void StartSavedGame(const char *filename) 
{
	char path[512];

	GuichanActive = false;
	SaveGameLoading = 1;
	CleanPlayers();
	ExpandPath(path, filename);
	LoadGame(path);

	StartMap(filename, false);
}
	
void StartReplay(const char *filename, bool reveal)
{
	char replay[PATH_MAX];

	CleanPlayers();
	ExpandPath(replay, filename);
	LoadReplay(replay);

	// FIXME why is this needed ?
	for (int i = 0; i < MAX_OBJECTIVES; i++) {
		delete[] GameIntro.Objectives[i];
		GameIntro.Objectives[i] = NULL;
	}
	GameIntro.Objectives[0] = new_strdup(DefaultObjective);

	ReplayRevealMap = reveal;

	StartMap(CurrentMapPath, false);
}

//----------------------------------------------------------------------------

/**
**  Print headerline, copyright, ...
*/
static void PrintHeader(void)
{
	fprintf(stdout, "%s\n  written by Lutz Sammer, Fabrice Rossi, Vladi Shabanski, Patrice Fortier,\n"
		"Jon Gabrielson, Andreas Arens, Nehal Mistry, Jimmy Salmon, and others.\n"
		"\t(http://stratagus.org)"
		"\nCompile options %s", NameLine, CompileOptions);
}

/**
**  Main1, called from main.
**
**  @param argc  Number of arguments.
**  @param argv  Vector of arguments.
*/
static int main1(int argc, char **argv)
{
	PrintHeader();
	printf(
	"\n"
	"\n"
	"Stratagus may be copied only under the terms of the GNU General Public License\n"
	"which may be found in the Stratagus source kit.\n"
	"\n"
	"DISCLAIMER:\n"
	"This software is provided as-is.  The author(s) can not be held liable for any\n"
	"damage that might arise from the use of this software.\n"
	"Use it at your own risk.\n"
	"\n");

	// Setup video display
	InitVideo();

	// Setup sound card
	if (!InitSound()) {
		InitMusic();
	}

#ifndef DEBUG           // For debug it's better not to have:
	srand(time(NULL));  // Random counter = random each start
#endif

	//
	//  Show title screens.
	//
	SetDefaultTextColors(FontYellow, FontWhite);
	LoadFonts();
	SetClipping(0, 0, Video.Width - 1, Video.Height - 1);
	Video.ClearScreen();
	ShowTitleScreens();

	// Init player data
	ThisPlayer = NULL;
	//Don't clear the Players strucure as it would erase the allowed units.
	// memset(Players, 0, sizeof(Players));
	NumPlayers = 0;

	InitUnitsMemory();  // Units memory management
	PreMenuSetup();     // Load everything needed for menus

	MenuLoop(MapName, &Map);

	return 0;
}

/**
**  Exit the game.
**
**  @param err  Error code to pass to shell.
*/
void Exit(int err)
{
	StopMusic();
	QuitSound();
	NetworkQuit();

	ExitNetwork1();
#ifdef DEBUG
	DebugPrint("Frames %lu, Slow frames %d = %ld%%\n" _C_
		FrameCounter _C_ SlowFrameCounter _C_
		(SlowFrameCounter * 100) / (FrameCounter ? FrameCounter : 1));
	lua_settop(Lua, 0);
	CclUnits(Lua);
	CleanModules();
	lua_close(Lua);
#endif

	fprintf(stdout, _("Thanks for playing Stratagus.\n"));
	exit(err);
}

/**
**  Do a fatal exit.
**  Called on out of memory or crash.
**
**  @param err  Error code to pass to shell.
*/
void ExitFatal(int err)
{
	exit(err);
}

/**
**  Display the usage.
*/
static void Usage(void)
{
	PrintHeader();
	printf(
"\n\nUsage: stratagus [OPTIONS] [map.smp|map.smp.gz]\n\
\t-c file.lua\tconfiguration start file (default stratagus.lua)\n\
\t-d datapath\tpath to stratagus data\n\
\t-e\t\tStart editor\n\
\t-h\t\tHelp shows this page\n\
\t-l\t\tDisable command log\n\
\t-P port\t\tNetwork port to use\n\
\t-n server\tNetwork server host preset\n\
\t-L lag\t\tNetwork lag in # frames (default 10 = 333ms)\n\
\t-U update\tNetwork update rate in # frames (default 5=6x per s)\n\
\t-N name\t\tName of the player\n\
\t-s sleep\tNumber of frames for the AI to sleep before it starts\n\
\t-v mode\t\tVideo mode (0=default,1=640x480,2=800x600,\n\
\t\t\t\t3=1024x768,4=1280x960,5=1600x1200)\n\
\t-D\t\tVideo mode depth = pixel per point (for Win32/TNT)\n\
\t-F\t\tFull screen video mode\n\
\t-S\t\tSync speed (100 = 30 frames/s)\n\
\t-W\t\tWindowed video mode\n\
map is relative to StratagusLibPath=datapath, use ./map for relative to cwd\n\
");
}

/**
**  The main program: initialise, parse options and arguments.
**
**  @param argc  Number of arguments.
**  @param argv  Vector of arguments.
*/
int main(int argc, char **argv)
{
	char *p;

	CompileOptions =
#ifdef DEBUG
		"DEBUG "
#endif
#ifdef USE_ZLIB
		"ZLIB "
#endif
#ifdef USE_BZ2LIB
		"BZ2LIB "
#endif
#ifdef USE_VORBIS
		"VORBIS "
#endif
#ifdef USE_THEORA
		"THEORA "
#endif
#ifdef USE_MIKMOD
		"MIKMOD "
#endif
#ifdef USE_OPENGL
		"OPENGL "
#endif
#ifdef USE_MNG
		"MNG "
#endif
	;

#ifdef USE_BEOS
	//
	//  Parse arguments for BeOS
	//
	beos_init(argc, argv);
#endif

	//
	//  Setup some defaults.
	//
#ifndef MAC_BUNDLE
	StratagusLibPath = STRATAGUS_LIB_PATH;
#else
	freopen("/tmp/stdout.txt", "w", stdout);
	freopen("/tmp/stderr.txt", "w", stderr);
	// Look for the specified data set inside the application bundle
	// This should be a subdir of the Resources directory
	CFURLRef pluginRef = CFBundleCopyResourceURL(CFBundleGetMainBundle(),
		CFSTR(MAC_BUNDLE_DATADIR), NULL, NULL);
	CFStringRef macPath = CFURLCopyFileSystemPath(pluginRef,
		 kCFURLPOSIXPathStyle);
	const char *pathPtr = CFStringGetCStringPtr(macPath,
		CFStringGetSystemEncoding());
	Assert(pathPtr);
	StratagusLibPath = new_strdup(pathPtr);
#endif
	CclStartFile = "scripts/stratagus.lua";
	EditorStartFile = "scripts/editor.lua";

	//  Default player name to username on unix systems.
	memset(LocalPlayerName, 0, sizeof(LocalPlayerName));
#ifdef USE_WIN32
	strcpy(LocalPlayerName, "Anonymous");
#else
	if (getenv("USER")) {
		strncpy(LocalPlayerName, getenv("USER"), sizeof(LocalPlayerName) - 1);
	} else {
		strcpy(LocalPlayerName, "Anonymous");
	}
#endif

	// FIXME: Parse options before or after scripts?

	//
	//  Parse commandline
	//
	for (;;) {
		switch (getopt(argc, argv, "c:d:ef:hln:P:s:t:v:wD:N:E:FL:S:U:W?")) {
			case 'c':
				CclStartFile = optarg;
				continue;
			case 'd':
				StratagusLibPath = optarg;
				for (p = StratagusLibPath; *p; ++p) {
					if (*p == '\\') {
						*p = '/';
					}
				}
				continue;
			case 'e':
				Editor.Running = EditorCommandLine;
				continue;
			case 'E':
				EditorStartFile = optarg;
				continue;
			case 'l':
				CommandLogDisabled = 1;
				continue;
			case 'P':
				NetworkPort = atoi(optarg);
				continue;
			case 'n':
				NetworkArg = new_strdup(optarg);
				continue;
			case 'N':
				memset(LocalPlayerName, 0, sizeof(LocalPlayerName));
				strncpy(LocalPlayerName, optarg, sizeof(LocalPlayerName) - 1);
				continue;
			case 's':
				AiSleepCycles = atoi(optarg);
				continue;
			case 'v':
				switch (atoi(optarg)) {
					case 0:
						continue;
					case 1:
						Video.Width = 640;
						Video.Height = 480;
						continue;
					case 2:
						Video.Width = 800;
						Video.Height = 600;
						continue;
					case 3:
						Video.Width = 1024;
						Video.Height = 768;
						continue;
					case 4:
						Video.Width = 1280;
						Video.Height = 960;
						continue;
					case 5:
						Video.Width = 1600;
						Video.Height = 1200;
						continue;
					default:
						Usage();
						ExitFatal(-1);
				}
				continue;

			case 'L':
				NetworkLag = atoi(optarg);
				if (!NetworkLag) {
					fprintf(stderr, "FIXME: zero lag not supported\n");
					Usage();
					ExitFatal(-1);
				}
				continue;
			case 'U':
				NetworkUpdates = atoi(optarg);
				continue;

			case 'F':
				VideoForceFullScreen = 1;
				Video.FullScreen = 1;
				continue;
			case 'W':
				VideoForceFullScreen = 1;
				Video.FullScreen = 0;
				continue;
			case 'D':
				Video.Depth = atoi(optarg);
				continue;
			case 'S':
				VideoSyncSpeed = atoi(optarg);
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

	if (argc - optind > 1) {
		fprintf(stderr, "too many files\n");
		Usage();
		ExitFatal(-1);
	}

	if (argc - optind) {
		MapName = argv[optind];
		for (p = MapName; *p; ++p) {
			if (*p == '\\') {
				*p = '/';
			}
		}
		--argc;
	}

	// Init the random number generator.
	InitSyncRand();

	// Init CCL and load configurations!
	InitCcl();

	// Initialise AI module
	InitAiModule();

	LoadCcl();

	main1(argc, argv);

	return 0;
}

//@}
