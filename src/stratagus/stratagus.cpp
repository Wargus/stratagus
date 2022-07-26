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
//      (c) Copyright 1998-2015 by Lutz Sammer, Francois Beerten,
//                                 Jimmy Salmon, Pali Rohár and cybermind
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
** or wrong grammar please write an email (including a patch :).
**
** @section Information Information
**
** Visit the https://github.com/Wargus/stratagus web page for the latest news and
** <A HREF="../index.html">Stratagus Info</A> for other documentation.
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

#include <ctype.h>

#ifdef USE_BEOS
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

extern void beos_init(int argc, char **argv);

#endif

#ifdef MAC_BUNDLE
#define Button ButtonOSX
#include <Carbon/Carbon.h>
#undef Button
#endif

#include "SDL.h"

#include "stratagus.h"

#include "ai.h"
#include "editor.h"
#include "game.h"
#include "guichan.h"
#include "interface.h"
#include "iocompat.h"
#include "iolib.h"
#include "map.h"
#include "netconnect.h"
#include "network.h"
#include "parameters.h"
#include "player.h"
#include "replay.h"
#include "results.h"
#include "settings.h"
#include "sound_server.h"
#include "title.h"
#include "translate.h"
#include "ui.h"
#include "unit_manager.h"
#include "version.h"
#include "video.h"
#include "widgets.h"
#include "util.h"

#include "missile.h" //for FreeBurningBuildingFrames

#ifdef USE_STACKTRACE
#include <stdexcept>
#include <stacktrace/call_stack.hpp>
#include <stacktrace/stack_exception.hpp>
#else
#include "st_backtrace.h"
#endif

#include <stdlib.h>
#include <stdio.h>

#ifdef USE_WIN32
#include <windows.h>
#include <dbghelp.h>
#endif

#if defined(USE_WIN32) && ! defined(NO_STDIO_REDIRECT)
#include "windows.h"
#define REDIRECT_OUTPUT
#endif

#if defined(USE_WIN32) && ! defined(REDIRECT_OUTPUT)
#include "SetupConsole_win32.h"
#endif

/*----------------------------------------------------------------------------
--  Variables
----------------------------------------------------------------------------*/

std::string StratagusLibPath;        /// Path for data directory

/// Name, Version, Copyright
const char NameLine[] = NAME " v" VERSION ", " COPYRIGHT;

std::string CliMapName;          /// Filename of the map given on the command line
std::string MenuRace;

bool EnableDebugPrint;           /// if enabled, print the debug messages
bool EnableAssert;               /// if enabled, halt on assertion failures
bool EnableUnitDebug;            /// if enabled, a unit info dump will be created
bool IsRestart;                  /// if true, the game skips some things like title screens

std::vector<std::string> OriginalArgv;

#ifdef DEBUG
bool IsDebugEnabled {true};      /// Is debug enabled? Flag to pass into lua code. 
#else
bool IsDebugEnabled {false};     /// Is debug enabled? Flag to pass into lua code.
#endif
bool EnableWallsInSinglePlayer {false}; /// Enables ability to build walls in the single player games
										/// used for debug purposes

/*============================================================================
==  MAIN
============================================================================*/

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
	int status;

	initGuichan();
	InterfaceState = IfaceStateMenu;
	//  Clear screen
	Video.ClearScreen();
	Invalidate();

	ButtonUnderCursor = -1;
	OldButtonUnderCursor = -1;
	CursorState = CursorStatePoint;
	GameCursor = UI.Point.Cursor;

	// FIXME delete this when switching to full guichan GUI
	const std::string filename = LibraryFileName("scripts/guichan.lua");
	status = LuaLoadFile(filename);

	// We clean up later in Exit
	return status;
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
#ifdef USE_WIN32
		"WIN32 "
#endif
#ifdef USE_LINUX
		"LINUX "
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
		"";

	fprintf(stdout,
			"%s\n  written by Lutz Sammer, Fabrice Rossi, Vladi Shabanski, Patrice Fortier,\n"
			"  Jon Gabrielson, Andreas Arens, Nehal Mistry, Jimmy Salmon, Pali Rohar,\n"
			"  cybermind and others.\n"
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
	NetworkQuitGame();

	ExitNetwork1();
	CleanModules();
	FreeBurningBuildingFrames();
	FreeSounds();
	FreeGraphics();
	FreePlayerColors();
	FreeButtonStyles();
	FreeAllContainers();
	freeGuichan();
	fprintf(stdout, "Frames %lu, Slow frames %ld = %ld%%\n",
			   FrameCounter, SlowFrameCounter,
			   (SlowFrameCounter * 100) / (FrameCounter ? FrameCounter : 1));
	lua_settop(Lua, 0);
	lua_close(Lua);
	DeInitVideo();
	DeInitImageLoaders();

	if (UnitManager) {
		delete UnitManager;
	}
	if (FogOfWar) {
		delete FogOfWar;
	}

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
#ifdef USE_STACKTRACE
	throw stacktrace::stack_runtime_error((const char*)err);
#else
	print_backtrace();
#endif
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
		"\t-a\t\tEnables asserts check in engine code (for debugging)\n"
		"\t-b\t\tBenchmark mode. Runs as fast as possible and reports FPS.\n"
		"\t-c file.lua\tConfiguration start file (default stratagus.lua)\n"
		"\t-d datapath\tPath to stratagus data (default current directory)\n"
		"\t-D depth\tVideo mode depth = pixel per point\n"
		"\t-e\t\tStart editor (instead of game)\n"
		"\t-E file.lua\tEditor configuration start file (default editor.lua)\n"
		"\t-F\t\tFull screen video mode\n"
		"\t-g\t\tForce software rendering (implies no shaders)\n"
		"\t-G \"options\"\tGame options (passed to game scripts)\n"
		"\t-h\t\tHelp shows this page\n"
		"\t-i\t\tEnables unit info dumping into log (for debugging)\n"
		"\t-I addr\t\tNetwork address to use\n"
		"\t-l\t\tDisable command log\n"
		"\t-N name\t\tName of the player\n"
		"\t-p\t\tEnables debug messages printing in console\n"
		"\t-P port\t\tNetwork port to use\n"
		"\t-r\t\tIndicate a rapid start. Skips a few things like title screens\n"
		"\t-s sleep\tNumber of frames for the AI to sleep before it starts\n"
		"\t-S speed\tSync speed (100 = 30 frames/s)\n"
		"\t-u userpath\tPath where stratagus saves preferences, log and savegame. Use 'userhome' to force platform-default userhome directory.\n"
		"\t-v mode\t\tVideo mode resolution in format <xres>x<yres>\n"
		"\t-W\t\tWindowed video mode. Optionally takes a window size in <xres>x<yres>\n"
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
	std::string path = Parameters::Instance.GetUserDirectory();

	makedir(path.c_str(), 0777);

	stdoutFile = path + "\\stdout.txt";
	stderrFile = path + "\\stderr.txt";

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
#ifdef DEBUG
	fprintf(stderr, "optind(%d), argc(%d) at startup\n", optind, argc);
	for (int i = 0; i < argc; i++) {
		fprintf(stderr, "%s ", argv[i]);
	}
	fprintf(stderr, "\n");
#endif
	char *sep;
	for (;;) {
		switch (getopt(argc, argv, "abc:d:D:eE:FgG:hiI:lN:oOP:prs:S:u:v:W?-")) {
			case 'a':
				EnableAssert = true;
				continue;
			case 'b':
				parameters.benchmark = true;
				continue;
			case 'c':
				parameters.luaStartFilename = optarg;
				if (strlen(optarg) > 4 &&
				    !(strstr(optarg, ".lua") == optarg + strlen(optarg) - 4)) {
					parameters.luaStartFilename += ".lua";
				}
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
			case 'g':
				SDL_SetHintWithPriority(SDL_HINT_RENDER_DRIVER, "software", SDL_HINT_OVERRIDE);
				continue;
			case 'G':
				parameters.luaScriptArguments = optarg;
				continue;
			case 'i':
				EnableUnitDebug = true;
				continue;
			case 'I':
				CNetworkParameter::Instance.localHost = optarg;
				continue;
			case 'l':
				CommandLogDisabled = true;
				continue;
			case 'N':
				parameters.LocalPlayerName = optarg;
				continue;
			case 'P':
				CNetworkParameter::Instance.localPort = atoi(optarg);
				continue;
			case 'p':
				EnableDebugPrint = true;
				continue;
			case 'r':
				IsRestart = true;
				continue;
			case 's':
				AiSleepCycles = atoi(optarg);
				continue;
			case 'S':
				VideoSyncSpeed = atoi(optarg);
				continue;
			case 'u':
				if (!strcmp(optarg, "userhome")) {
					Parameters::Instance.SetUserDirectory("");
				} else {
					Parameters::Instance.SetUserDirectory(optarg);
				}
				continue;
			case 'v': {
				sep = strchr(optarg, 'x');
				if (!sep || !*(sep + 1)) {
					fprintf(stderr, "%s: incorrect format of video mode resolution -- '%s'\n", argv[0], optarg);
					Usage();
					exit(-1);
				}
				Video.Height = atoi(sep + 1);
				*sep = 0;
				Video.Width = atoi(optarg);
				if (!Video.Height || !Video.Width) {
					fprintf(stderr, "%s: incorrect format of video mode resolution -- '%sx%s'\n", argv[0], optarg, sep + 1);
					Usage();
					exit(-1);
				}
				continue;
			}
			case 'W':
				if (optind < argc && argv[optind] && argv[optind][0] != '-') {
					// allow -W to take an optional argument in a POSIX compliant way
					optarg = argv[optind];
					sep = strchr(optarg, 'x');
					if (!sep || !*(sep + 1)) {
						fprintf(stderr, "%s: incorrect window size -- '%s'\n", argv[0], optarg);
						Usage();
						exit(-1);
					}
					Video.WindowHeight = atoi(sep + 1);
					*sep = 0;
					Video.WindowWidth = atoi(optarg);
					if (!Video.WindowHeight || !Video.WindowWidth) {
						fprintf(stderr, "%s: incorrect window size -- '%sx%s'\n", argv[0], optarg, sep + 1);
						Usage();
						exit(-1);
					}
					optind += 1; // skip the optional window size argument
				}
				VideoForceFullScreen = 1;
				Video.FullScreen = 0;
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

	if (argc - optind > 1) {
		fprintf(stderr, "too many map files. if you meant to pass game arguments, these go after '--'\n");
		Usage();
		ExitFatal(-1);
	}

	if (argc - optind) {
		size_t index;
		CliMapName = argv[optind];
		while ((index = CliMapName.find('\\')) != std::string::npos) {
			CliMapName[index] = '/';
		}
	}
}

#ifdef USE_WIN32
static LONG WINAPI CreateDumpFile(EXCEPTION_POINTERS *ExceptionInfo)
{
	HANDLE hFile = CreateFile("crash.dmp", GENERIC_READ | GENERIC_WRITE,	FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE,
		NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL,	NULL);
	MINIDUMP_EXCEPTION_INFORMATION mei;
	mei.ThreadId = GetCurrentThreadId();
	mei.ClientPointers = TRUE;
	mei.ExceptionPointers = ExceptionInfo;
	MiniDumpWriteDump(GetCurrentProcess(), GetCurrentProcessId(), hFile, MiniDumpNormal, &mei, NULL, NULL);
	fprintf(stderr, "Stratagus crashed!\n");
	fprintf(stderr, "A mini dump file \"crash.dmp\" has been created in the Stratagus folder.\n");
	fprintf(stderr, "Please send it to our bug tracker: https://github.com/Wargus/stratagus/issues\n");
	fprintf(stderr, "and tell us what caused this bug to occur.\n");
	return EXCEPTION_EXECUTE_HANDLER;
}
#endif

/**
**  The main program: initialise, parse options and arguments.
**
**  @param argc  Number of arguments.
**  @param argv  Vector of arguments.
*/
int stratagusMain(int argc, char **argv)
{
	for (int i = 0; i < argc; i++) {
		OriginalArgv.push_back(std::string(argv[i]));
	}
#ifdef USE_BEOS
	//  Parse arguments for BeOS
	beos_init(argc, argv);
#endif
#ifdef USE_WIN32
	SetUnhandledExceptionFilter(CreateDumpFile);
#endif
#if defined(USE_WIN32) && ! defined(REDIRECT_OUTPUT)
	SetupConsole();
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
	try {
		Parameters &parameters = Parameters::Instance;
		parameters.SetDefaultValues();
		parameters.SetLocalPlayerNameFromEnv();

#ifdef REDIRECT_OUTPUT
		RedirectOutput();
#endif

		if (argc > 0) {
			parameters.applicationName = argv[0];
		}

		// FIXME: Parse options before or after scripts?
		ParseCommandLine(argc, argv, parameters);
		// Init the random number generator.
		InitSyncRand();

		makedir(parameters.GetUserDirectory().c_str(), 0777);

		// Init Lua and register lua functions!
		InitLua();
		LuaRegisterModules();

		// Initialise AI module
		InitAiModule();

		// Setup sound card, must be done before loading sounds, so that
		// SDL_mixer can auto-convert to the target format
		if (!InitSound()) {
			InitMusic();
		}

		// init globals
		Map.AllocateTileset();
		UnitManager = new CUnitManager();
		FogOfWar = new CFogOfWar();

		LoadCcl(parameters.luaStartFilename, parameters.luaScriptArguments);

		// Setup video display
		InitVideo();

		PrintHeader();
		PrintLicense();

#ifndef DEBUG           // For debug it's better not to have:
		srand(time(NULL));  // Random counter = random each start
#endif

		//  Show title screens.
		SetDefaultTextColors(FontYellow, FontWhite);
		LoadFonts();
		SetClipping(0, 0, Video.Width - 1, Video.Height - 1);
		Video.ClearScreen();
		if (!IsRestart) {
			ShowTitleScreens();
		}

		// Init player data
		ThisPlayer = NULL;
		//Don't clear the Players structure as it would erase the allowed units.
		// memset(Players, 0, sizeof(Players));
		NumPlayers = 0;

		UnitManager->Init(); // Units memory management
		PreMenuSetup();     // Load everything needed for menus

		MenuLoop();

		Exit(0);
	} catch (const std::exception &e) {
		fprintf(stderr, "Stratagus crashed!\n");
		fprintf(stderr, "Please send this call stack to our bug tracker: https://github.com/Wargus/stratagus/issues\n");
		fprintf(stderr, "and tell us what caused this bug to occur.\n");
		fprintf(stderr, " === exception state traceback === \n");
		fprintf(stderr, "%s", e.what());
		exit(1);
	}
	return 0;
}

//@}
