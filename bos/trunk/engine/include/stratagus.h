//     ____                _       __               
//    / __ )____  _____   | |     / /___ ___________
//   / __  / __ \/ ___/   | | /| / / __ `/ ___/ ___/
//  / /_/ / /_/ (__  )    | |/ |/ / /_/ / /  (__  ) 
// /_____/\____/____/     |__/|__/\__,_/_/  /____/  
//                                              
//       A futuristic real-time strategy game.
//          This file is part of Bos Wars.
//
/**@name stratagus.h - The main header file. */
//
//      (c) Copyright 1998-2008 by Lutz Sammer and Jimmy Salmon
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

#ifndef __STRATAGUS_H__
#define __STRATAGUS_H__

//@{

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

/*============================================================================
==  Compiler repairs
============================================================================*/

#ifdef _MSC_VER

#define WIN32_LEAN_AND_MEAN
#define NOUSER

#define inline __inline
#define alloca _alloca

#pragma warning(disable:4244)               /// Conversion from double to uchar
#pragma warning(disable:4761)               /// Integral size mismatch
#pragma warning(disable:4786)               /// Truncated to 255 chars
#include <stdlib.h>
#define abort() _ASSERT(0)
#include <stdio.h>
#define snprintf _snprintf
#define vsnprintf _vsnprintf
#define unlink _unlink
#include <string.h>
#define strdup _strdup
#define strcasecmp _stricmp
#define strncasecmp _strnicmp
#include <io.h>
#define access _access
#define write _write
#include <direct.h>
#define makedir(dir, permissions) _mkdir(dir)

#endif  // } _MSC_VER

#ifdef __GNUC__
#ifdef USE_WIN32
#define makedir(dir, permissions) mkdir(dir)
#else 
#define makedir(dir, permissions) mkdir(dir, permissions)
#endif
#endif

/*============================================================================
==  Debug definitions
============================================================================*/

	/**
	**  This simulates vararg macros.
	**  Example:
	**    DebugPrint("Test %d %d\n" _C_ 1 _C_ 2);
	*/
#define _C_  ,    /// Debug , to simulate vararg macros

	/// Print function in debug macros
#define PrintFunction() \
	do { fprintf(stdout, "%s:%d: ", __FILE__, __LINE__); } while (0)


#ifdef DEBUG  // {

	/**
	**  Assert a condition. If cond is not true abort with file,line.
	*/
#define Assert(cond)  do { if (!(cond)) { \
	fprintf(stderr, "Assertion failed at %s:%d: %s\n", __FILE__, __LINE__, #cond); \
	abort(); }} while (0)

	/**
	**  Print debug information with function name.
	*/
#define DebugPrint(args) \
	do { PrintFunction(); fprintf(stdout, args); } while (0)

#else  // }{ DEBUG

#define Assert(cond)        /* disabled */
#define DebugPrint(args)    /* disabled */

#endif

/*============================================================================
==  Definitions
============================================================================*/

#include <string>
#include "util.h"
#include "translate.h"

/*----------------------------------------------------------------------------
--  Translate
----------------------------------------------------------------------------*/

#define _(str) Translate(str)
#define N_(str) str

/*----------------------------------------------------------------------------
--  General
----------------------------------------------------------------------------*/
#ifndef STRATAGUS_LIB_PATH
#define STRATAGUS_LIB_PATH  "."      /// Where to find the data files
#endif

#ifndef STRATAGUS_HOME_PATH
#ifdef __APPLE__
#define STRATAGUS_HOME_PATH "Library/boswars/"
#elif USE_WIN32
#define STRATAGUS_HOME_PATH "boswars/"
#else
 #define STRATAGUS_HOME_PATH ".boswars/"
#endif
#endif

/*----------------------------------------------------------------------------
--  Some limits
----------------------------------------------------------------------------*/

#define PlayerMax    9                  /// How many players are supported
#define UnitTypeMax  257                /// How many unit types supported
#define UnitMax      2048               /// How many units supported

/*----------------------------------------------------------------------------
--  Screen
----------------------------------------------------------------------------*/

	/// Frames per second to display
#define FRAMES_PER_SECOND  30  // 1/30s
	/// Game cycles per second to simulate
#define CYCLES_PER_SECOND  30  // 1/30s 33ms

/*----------------------------------------------------------------------------
--  stratagus.cpp
----------------------------------------------------------------------------*/

extern std::string UserDirectory;           /// Directory containing user settings and data
extern std::string StratagusLibPath;        /// Location of stratagus data

extern std::string ClickMissile;            /// Missile to show when you click
extern std::string DamageMissile;           /// Missile to show damage caused

extern int SpeedBuild;                      /// Speed factor for building
extern int SpeedTrain;                      /// Speed factor for training

extern unsigned long GameCycle;             /// Game simulation cycle counter
extern unsigned long FastForwardCycle;      /// Game Replay Fast Forward Counter

extern std::string CompileOptions;          /// Compile options
extern bool SaveGameLoading;                /// Save game is in progress of loading

extern void LoadGame(const std::string &filename); /// Load saved game
extern void SaveGame(const std::string &filename); /// Save game

extern void Exit(int err);                  /// Exit
extern void ExitFatal(int err);             /// Exit with fatal error

extern void UpdateDisplay(void);            /// Game display update
extern void InitModules(void);              /// Initialize all modules
extern void LoadModules(void);              /// Load all modules
extern void CleanModules(void);             /// Cleanup all modules
extern void DrawMapArea(void);              /// Draw the map area
extern void GameMainLoop(void);             /// Game main loop

	/// Show load progress
extern void ShowLoadProgress(const char *fmt, ...);

//@}

#endif // !__STRATAGUS_H__
