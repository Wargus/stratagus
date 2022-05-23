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
/**@name stratagus.h - The main header file. */
//
//      (c) Copyright 1998-2007 by Lutz Sammer and Jimmy Salmon
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

#ifndef __STRATAGUS_H__
#define __STRATAGUS_H__

#ifdef HAVE_COZ_PROFILER
#include <coz.h>
#endif

#include <string>
#include <vector>

#ifdef USE_OPENMP
#include <omp.h>
#else
#define omp_get_thread_num() 0
#define omp_get_num_threads() 1
#endif

//@{

/*============================================================================
==  Config definitions
============================================================================*/

// Dynamic loading.
//#define DYNAMIC_LOAD

/*============================================================================
==  Compiler repairs
============================================================================*/

#ifdef _MSC_VER

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <Shlobj.h>

#if _MSC_VER >= 1800
// From VS2013 onwards, std::min/max are only defined if algorithm is included
#include <algorithm>
#endif

#pragma warning(disable:4244)               /// Conversion from double to uchar
#pragma warning(disable:4761)               /// Integral size mismatch
#pragma warning(disable:4786)               /// Truncated to 255 chars
#pragma warning(disable:4996)               /// Warning about POSIX names

#ifndef __func__
#define __func__ __FUNCTION__
#endif

#define snprintf _snprintf
#if !(_MSC_VER >= 1500 && _MSC_VER < 1600)
#define vsnprintf _vsnprintf
#endif
#define unlink _unlink
#define strdup _strdup
#define strcasecmp _stricmp
#define strncasecmp _strnicmp

#endif  // } _MSC_VER


/*============================================================================
==  Macro
============================================================================*/

// To remove warning for unused variable.
#ifdef __GNUC__
#define UNUSED(var) do {__typeof__ (&var) __attribute__ ((unused)) __tmp = &var; } while(0)
#else
#define UNUSED(var) (var)
#endif

#ifdef __GNUC__
#define PRINTF_VAARG_ATTRIBUTE(a, b) __attribute__((format (printf, a, b)))
#else
#define PRINTF_VAARG_ATTRIBUTE(a, b)
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

extern void PrintLocation(const char *file, int line, const char *funcName);

/// Print function in debug macros
#define PrintFunction() PrintLocation(__FILE__, __LINE__, __func__);

extern bool EnableDebugPrint;
extern bool EnableAssert;
extern bool EnableUnitDebug;
extern bool IsRestart;
extern bool IsDebugEnabled;
extern bool EnableWallsInSinglePlayer;

extern std::vector<std::string> OriginalArgv;

extern void AbortAt(const char *file, int line, const char *funcName, const char *conditionStr);
extern void PrintOnStdOut(const char *format, ...);

/**
**  Assert a condition. If cond is not true abort with file,line.
*/
#define Assert(cond) \
	do { if (EnableAssert && !(cond)) { AbortAt(__FILE__, __LINE__, __func__, #cond); }} while (0)

/**
**  Print debug information with function name.
*/
#define DebugPrint(args) \
	do { if (EnableDebugPrint) { PrintFunction(); PrintOnStdOut(args); } } while (0)

/*============================================================================
==  Definitions
============================================================================*/

#include "util.h"
#include "settings.h"

inline char *new_strdup(const char *str)
{
	int len = strlen(str) + 1;
	char *newstr = new char[len];
	strcpy_s(newstr, len, str);
	return newstr;
}

/*----------------------------------------------------------------------------
--  General
----------------------------------------------------------------------------*/

/// Text string: Name, Version, Copyright
extern const char NameLine[];

/*----------------------------------------------------------------------------
--  stratagus.cpp
----------------------------------------------------------------------------*/

extern std::string StratagusLibPath;        /// Location of stratagus data
extern std::string MenuRace;

extern unsigned long GameCycle;             /// Game simulation cycle counter
extern unsigned long FastForwardCycle;      /// Game Replay Fast Forward Counter

extern void Exit(int err);                  /// Exit
extern void ExitFatal(int err);             /// Exit with fatal error

extern void UpdateDisplay();            /// Game display update
extern void DrawMapArea();              /// Draw the map area
extern void GameMainLoop();             /// Game main loop
extern int stratagusMain(int argc, char **argv); /// main entry

//@}

#endif // !__STRATAGUS_H__
