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
//      (c) Copyright 1998-2011 by Lutz Sammer, Francois Beerten,
//                                 Jimmy Salmon and Pali Rohár
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

//@{

/**
** @mainpage
**
** @section Introduction Introduction
**
** Welcome to the source code documentation of the Stratagus engine.
** Extract the source documentation with doxygen (http://www.doxygen.org) tool.
**
** Any help to improve this documention is welcome. If you didn't
** understand something or you found an error or a wrong spelling
** or wrong grammer please write an email (including a patch :).
**
** @section Informations Informations
**
** Visit the https://launchpad.net/stratagus web page for the latest news and
** <A HREF="../index.html">Stratagus Info</A> for other documentations.
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
#include <sstream>

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
#include "unit_manager.h"
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
#include "replay.h"
#include "results.h"
#include "editor.h"
#include "movie.h"
#include "pathfinder.h"
#include "widgets.h"
#include "iolib.h"
#include "util.h"
#include "guichan.h"
#include "title.h"
#include "map.h"

#ifdef DEBUG
#include "missile.h" //for FreeBurningBuildingFrames
#endif

#if defined(USE_WIN32) && ! defined(NO_STDIO_REDIRECT)
#define REDIRECT_OUTPUT
#endif


void Parameters::SetDefaultValues()
{
	applicationName = "stratagus";
	luaStartFilename = "scripts/stratagus.lua";
	luaEditorStartFilename = "scripts/editor.lua";
	SetUserDirectory();
}

void Parameters::SetUserDirectory()
{
#ifdef USE_WIN32
	UserDirectory = getenv("APPDATA");
#else
	UserDirectory = getenv("HOME");
#endif

	if (!UserDirectory.empty()) {
		UserDirectory += "/";
	}

#ifdef USE_WIN32
	UserDirectory += "Stratagus";
#elif defined(USE_MAC)
	UserDirectory += "Library/Stratagus";
#else
	UserDirectory += ".stratagus";
#endif
}


/* static */ Parameters Parameters::Instance;


/*----------------------------------------------------------------------------
--  Variables
----------------------------------------------------------------------------*/

std::string StratagusLibPath;        /// Path for data directory

/// Name, Version, Copyright
const char NameLine[] = NAME " V" VERSION ", " COPYRIGHT;

std::string CliMapName;          /// Filename of the map given on the command line
static std::vector<gcn::Container *> Containers;
std::string MenuRace;

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
unsigned long ResultGameCycle;       /// Used in game result
unsigned long FastForwardCycle;      /// Cycle to fastforward to in a replay

/*============================================================================
==  MAIN
============================================================================*/

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

	if (Video.Depth && GetGameFont() && GetGameFont()->IsLoaded()) {
		// Remove non printable chars
		for (char *s = temp; *s; ++s) {
			if (*s < 32) {
				*s = ' ';
			}
		}
		Video.FillRectangle(ColorBlack, 5, Video.Height - 18, Video.Width - 10, 18);
		CLabel(GetGameFont()).DrawCentered(Video.Width / 2, Video.Height - 16, temp);
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
void PreMenuSetup()
{
	//
	//  Initial menus require some gfx.
	//
	SetDefaultTextColors(FontYellow, FontWhite);

	LoadFonts();

	InitVideoCursors();

	if (MenuRace.empty()) {
		LoadCursors(PlayerRaces.Name[0]);
	} else {
		LoadCursors(MenuRace);
	}

	InitSettings();

	InitUserInterface();
	UI.Load();
}

/**
**  Run the guichan main menus loop.
**
**  @return          0 for success, else exit.
*/
static int MenuLoop()
{
	char buf[1024];
	int status;

	initGuichan();
	InterfaceState = IfaceStateMenu;
	//  Clear screen
	Video.ClearScreen();
	Invalidate();

	ButtonUnderCursor = -1;
	CursorState = CursorStatePoint;
	GameCursor = UI.Point.Cursor;

	// FIXME delete this when switching to full guichan GUI
	LibraryFileName("scripts/guichan.lua", buf, sizeof(buf));
	status = LuaLoadFile(buf);

	// We clean up later in Exit
	return status;
}

extern void CleanMissiles();
extern void CleanTriggers();

/**
**  Cleanup game.
**
**  Call each module to clean up.
**  Contrary to CleanModules, maps can be restarted
**  without reloading all lua files.
*/
void CleanGame()
{
	EndReplayLog();
	CleanMessages();

	CleanGame_Lua();
	CleanTriggers();
	CleanAi();
	CleanGroups();
	CleanMissiles();
	CleanUnits();
	CleanSelections();
	CleanTilesets();
	Map.Clean();
	CleanReplayLog();
	FreePathfinder();
	CursorBuilding = NULL;
	UnitUnderCursor = NULL;
}

static void ExpandPath(std::string &newpath, const std::string &path)
{
	if (path[0] == '~') {
		newpath = Parameters::Instance.GetUserDirectory();
		if (!GameName.empty()) {
			newpath += "/";
			newpath += GameName;
		}
		newpath += "/" + path.substr(1);
	} else {
		newpath = StratagusLibPath + "/" + path;
	}
}

extern gcn::Gui *Gui;

void StartMap(const std::string &filename, bool clean)
{
	std::string nc, rc;

	gcn::Widget *oldTop = Gui->getTop();
	gcn::Container *container = new gcn::Container();
	Containers.push_back(container);
	container->setDimension(gcn::Rectangle(0, 0, Video.Width, Video.Height));
	container->setOpaque(false);
	Gui->setTop(container);

	NetConnectRunning = 0;
	InterfaceState = IfaceStateNormal;

	//  Create the game.
	DebugPrint("Creating game with map: %s\n" _C_ filename.c_str());
	if (clean) {
		CleanPlayers();
	}
	GetDefaultTextColors(nc, rc);

	CreateGame(filename.c_str(), &Map);

	UI.StatusLine.Set(NameLine);
	SetMessage("%s", _("Do it! Do it now!"));

	//  Play the game.
	GameMainLoop();

	//  Clear screen
	Video.ClearScreen();
	Invalidate();

	CleanGame();
	InterfaceState = IfaceStateMenu;
	SetDefaultTextColors(nc, rc);

	Gui->setTop(oldTop);
	Containers.erase(std::find(Containers.begin(), Containers.end(), container));
	delete container;
}

void StartSavedGame(const std::string &filename)
{
	std::string path;

	SaveGameLoading = true;
	CleanPlayers();
	ExpandPath(path, filename);
	LoadGame(path);

	StartMap(filename, false);
	//SetDefaultTextColors(nc, rc);
}

void StartReplay(const std::string &filename, bool reveal)
{
	std::string replay;

	CleanPlayers();
	ExpandPath(replay, filename);
	LoadReplay(replay);

	ReplayRevealMap = reveal;

	StartMap(CurrentMapPath, false);
}

/**
**  Save the replay
**
**  @param filename  Name of the file to save to
**
**  @return          0 for success, -1 for failure
*/
int SaveReplay(const std::string &filename)
{
	FILE *fd;
	char *buf;
	std::ostringstream logfile;
	std::string destination;
	struct stat sb;
	size_t size;

	if (filename.find_first_of("\\/") != std::string::npos) {
		fprintf(stderr, "\\ or / not allowed in SaveReplay filename\n");
		return -1;
	}

	destination = Parameters::Instance.GetUserDirectory() + "/logs/" + filename;

	logfile << Parameters::Instance.GetUserDirectory() << "/logs/log_of_stratagus_" << ThisPlayer->Index << ".log";

	if (stat(logfile.str().c_str(), &sb)) {
		fprintf(stderr, "stat failed\n");
		return -1;
	}
	buf = new char[sb.st_size];
	if (!buf) {
		fprintf(stderr, "Out of memory\n");
		return -1;
	}
	fd = fopen(logfile.str().c_str(), "rb");
	if (!fd) {
		fprintf(stderr, "fopen failed\n");
		delete[] buf;
		return -1;
	}
	size = fread(buf, sb.st_size, 1, fd);
	fclose(fd);

	fd = fopen(destination.c_str(), "wb");
	if (!fd) {
		fprintf(stderr, "Can't save to `%s'\n", destination.c_str());
		delete[] buf;
		return -1;
	}
	fwrite(buf, size, 1, fd);
	fclose(fd);

	delete[] buf;

	return 0;
}

//----------------------------------------------------------------------------

/**
**  Print headerline, copyright, ...
*/
static void PrintHeader()
{
	std::string CompileOptions =
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
#ifdef USE_MNG
		"MNG "
#endif
#ifdef USE_OPENGL
		"OPENGL "
#endif
#ifdef USE_GLES
		"GLES "
#endif
#ifdef USE_WIN32
		"WIN32 "
#endif
#ifdef USE_BSD
		"BSD "
#endif
#ifdef USE_BEOS
		"BEOS "
#endif
#ifdef USE_MAC
		"MAC "
#endif
#ifdef USE_MAEMO
		"MAEMO "
#endif
#ifdef USE_TOUCHSCREEN
		"TOUCHSCREEN "
#endif
		"";

	fprintf(stdout,
			"%s\n  written by Lutz Sammer, Fabrice Rossi, Vladi Shabanski, Patrice Fortier,\n"
			"  Jon Gabrielson, Andreas Arens, Nehal Mistry, Jimmy Salmon, Pali Rohar,\n"
			"  and others.\n"
			"\t" HOMEPAGE "\n"
			"Compile options %s",
			NameLine, CompileOptions.c_str());
}

void PrintLicense()
{
	printf("\n"
		   "\n"
		   "Stratagus may be copied only under the terms of the GNU General Public License\n"
		   "which may be found in the Stratagus source kit.\n"
		   "\n"
		   "DISCLAIMER:\n"
		   "This software is provided as-is.  The author(s) can not be held liable for any\n"
		   "damage that might arise from the use of this software.\n"
		   "Use it at your own risk.\n"
		   "\n");
}


/**
**  Exit the game.
**
**  @param err  Error code to pass to shell.
*/
void Exit(int err)
{
	if (GameRunning) {
		StopGame(GameExit);
		return;
	}

	StopMusic();
	QuitSound();
	NetworkQuit();

	ExitNetwork1();
#ifdef DEBUG
	CleanModules();
	FreeBurningBuildingFrames();
	FreeSounds();
	FreeGraphics();
	FreePlayerColors();
	FreeButtonStyles();
	for (size_t i = 0; i < Containers.size(); ++i) {
		delete Containers[i];
	}
	freeGuichan();
	DebugPrint("Frames %lu, Slow frames %d = %ld%%\n" _C_
			   FrameCounter _C_ SlowFrameCounter _C_
			   (SlowFrameCounter * 100) / (FrameCounter ? FrameCounter : 1));
	lua_settop(Lua, 0);
	lua_close(Lua);
	DeInitVideo();
#endif

	fprintf(stdout, "%s", _("Thanks for playing Stratagus.\n"));
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
static void Usage()
{
	PrintHeader();
	printf(
		"\n\nUsage: %s [OPTIONS] [map.smp|map.smp.gz]\n"
		"\t-c file.lua\tConfiguration start file (default stratagus.lua)\n"
		"\t-d datapath\tPath to stratagus data (default current directory)\n"
		"\t-D depth\tVideo mode depth = pixel per point\n"
		"\t-e\t\tStart editor (instead of game)\n"
		"\t-E file.lua\tEditor configuration start file (default editor.lua)\n"
		"\t-F\t\tFull screen video mode\n"
		"\t-h\t\tHelp shows this page\n"
		"\t-I addr\t\tNetwork address to use\n"
		"\t-l\t\tDisable command log\n"
		"\t-L lag\t\tNetwork lag in # frames (default 10 = 333ms)\n"
		"\t-n server\tNetwork server host preset\n"
		"\t-N name\t\tName of the player\n"
		"\t-o\t\tDo not use OpenGL or OpenGL ES 1.1\n"
		"\t-O\t\tUse OpenGL or OpenGL ES 1.1\n"
		"\t-P port\t\tNetwork port to use\n"
		"\t-s sleep\tNumber of frames for the AI to sleep before it starts\n"
		"\t-S speed\tSync speed (100 = 30 frames/s)\n"
		"\t-U update\tNetwork update rate in # frames (default 5=6x per s)\n"
		"\t-v mode\t\tVideo mode resolution in format <xres>x<yres>\n"
		"\t-W\t\tWindowed video mode\n"
		"map is relative to StratagusLibPath=datapath, use ./map for relative to cwd\n",
		Parameters::Instance.applicationName.c_str());
}

#ifdef REDIRECT_OUTPUT

static std::string stdoutFile;
static std::string stderrFile;

static void CleanupOutput()
{
	fclose(stdout);
	fclose(stderr);

	struct stat st;
	if (stat(stdoutFile.c_str(), &st) == 0 && st.st_size == 0) {
		unlink(stdoutFile.c_str());
	}
	if (stat(stderrFile.c_str(), &st) == 0 && st.st_size == 0) {
		unlink(stderrFile.c_str());
	}
}

static void RedirectOutput()
{
	char path[MAX_PATH];
	int pathlen;

	pathlen = GetModuleFileName(NULL, path, sizeof(path));
	while (pathlen > 0 && path[pathlen] != '\\') {
		--pathlen;
	}
	path[pathlen] = '\0';

	stdoutFile = std::string(path) + "\\stdout.txt";
	stderrFile = std::string(path) + "\\stderr.txt";

	if (!freopen(stdoutFile.c_str(), "w", stdout)) {
		printf("freopen stdout failed");
	}
	if (!freopen(stderrFile.c_str(), "w", stderr)) {
		printf("freopen stderr failed");
	}
	atexit(CleanupOutput);
}
#endif

void ParseCommandLine(int argc, char **argv, Parameters &parameters)
{
	for (;;) {
		switch (getopt(argc, argv, "c:d:D:eE:FhI:lL:n:N:oOP:s:S:U:v:W?")) {
			case 'c':
				parameters.luaStartFilename = optarg;
				continue;
			case 'd': {
				StratagusLibPath = optarg;
				size_t index;
				while ((index = StratagusLibPath.find('\\')) != std::string::npos) {
					StratagusLibPath[index] = '/';
				}
				continue;
			}
			case 'D':
				Video.Depth = atoi(optarg);
				continue;
			case 'e':
				Editor.Running = EditorCommandLine;
				continue;
			case 'E':
				parameters.luaEditorStartFilename = optarg;
				continue;
			case 'F':
				VideoForceFullScreen = 1;
				Video.FullScreen = 1;
				continue;
			case 'I':
				NetworkAddr = optarg;
				continue;
			case 'l':
				CommandLogDisabled = true;
				continue;
			case 'L':
				NetworkLag = atoi(optarg);
				if (!NetworkLag) {
					fprintf(stderr, "%s: zero lag not supported\n", argv[0]);
					Usage();
					ExitFatal(-1);
				}
				continue;
			case 'n':
				NetworkArg = optarg;
				continue;
			case 'N':
				parameters.LocalPlayerName = optarg;
				continue;
			case 'o':
				ForceUseOpenGL = 1;
				UseOpenGL = 0;
				continue;
			case 'O':
				ForceUseOpenGL = 1;
				UseOpenGL = 1;
				continue;
			case 'P':
				NetworkPort = atoi(optarg);
				continue;
			case 's':
				AiSleepCycles = atoi(optarg);
				continue;
			case 'S':
				VideoSyncSpeed = atoi(optarg);
				continue;
			case 'U':
				NetworkUpdates = atoi(optarg);
				continue;
			case 'v': {
				char *sep = strchr(optarg, 'x');
				if (!sep || !*(sep + 1)) {
					fprintf(stderr, "%s: incorrect format of video mode resolution -- '%s'\n", argv[0], optarg);
					Usage();
					ExitFatal(-1);
				}
				Video.Height = atoi(sep + 1);
				*sep = 0;
				Video.Width = atoi(optarg);
				if (!Video.Height || !Video.Width) {
					fprintf(stderr, "%s: incorrect format of video mode resolution -- '%sx%s'\n", argv[0], optarg, sep + 1);
					Usage();
					ExitFatal(-1);
				}
				continue;
			}
			case 'W':
				VideoForceFullScreen = 1;
				Video.FullScreen = 0;
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
		size_t index;
		CliMapName = argv[optind];
		while ((index = CliMapName.find('\\')) != std::string::npos) {
			CliMapName[index] = '/';
		}
		--argc;
	}
}

std::string GetLocalPlayerNameFromEnv()
{
	//  Default player name to username on unix systems.
#if defined(USE_WIN32) || defined(USE_MAEMO)
	return "Anonymous";
#else
	const char *userName = getenv("USER");

	if (userName) {
		return userName;
	} else {
		return "Anonymous";
	}
#endif
}

/**
**  The main program: initialise, parse options and arguments.
**
**  @param argc  Number of arguments.
**  @param argv  Vector of arguments.
*/
int main(int argc, char **argv)
{
#ifdef REDIRECT_OUTPUT
	RedirectOutput();
#endif

#ifdef USE_BEOS
	//  Parse arguments for BeOS
	beos_init(argc, argv);
#endif

	//  Setup some defaults.
#ifndef MAC_BUNDLE
	StratagusLibPath = ".";
#else
	freopen("/tmp/stdout.txt", "w", stdout);
	freopen("/tmp/stderr.txt", "w", stderr);
	// Look for the specified data set inside the application bundle
	// This should be a subdir of the Resources directory
	CFURLRef pluginRef = CFBundleCopyResourceURL(CFBundleGetMainBundle(),
												 CFSTR(MAC_BUNDLE_DATADIR), NULL, NULL);
	CFStringRef macPath = CFURLCopyFileSystemPath(pluginRef,  kCFURLPOSIXPathStyle);
	const char *pathPtr = CFStringGetCStringPtr(macPath, CFStringGetSystemEncoding());
	Assert(pathPtr);
	StratagusLibPath = pathPtr;
#endif

	Parameters &parameters = Parameters::Instance;
	parameters.SetDefaultValues();
	parameters.applicationName = argv[0];
	parameters.LocalPlayerName = GetLocalPlayerNameFromEnv();

	// FIXME: Parse options before or after scripts?
	ParseCommandLine(argc, argv, parameters);
	// Init the random number generator.
	InitSyncRand();

	makedir(parameters.GetUserDirectory().c_str(), 0777);

	// Init Lua and register lua functions!
	InitCcl();

	// Initialise AI module
	InitAiModule();

	LoadCcl(parameters.luaStartFilename);

	PrintHeader();
	PrintLicense();

	// Setup video display
	InitVideo();

	// Setup sound card
	if (!InitSound()) {
		InitMusic();
	}

#ifndef DEBUG           // For debug it's better not to have:
	srand(time(NULL));  // Random counter = random each start
#endif

	//  Show title screens.
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

	UnitManager.Init(); // Units memory management
	PreMenuSetup();     // Load everything needed for menus

	MenuLoop();

	Exit(0);
	return 0;
}

//@}
