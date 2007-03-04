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
//      $Id$

#ifndef __STRATAGUS_H__
#define __STRATAGUS_H__

//@{

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

/*============================================================================
==  Config definitions
============================================================================*/

// Dynamic loading.
//#define DYNAMIC_LOAD

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
#define strncasecmp _strnicmp
#include <io.h>
#define access _access
#define write _write
#include <direct.h>
#define mkdir(dir, parmissions) _mkdir(dir)

#endif  // } _MSC_VER

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

#ifdef REFS_DEBUG  // {

	/**
	**  Assert a condition for references
	*/
#define RefsAssert(cond)  do { if (!(cond)) { \
	fprintf(stderr, "Assertion failed at %s:%d\n", __FILE__, __LINE__); \
	abort(); } } while (0)

#else  // }{ REFS_DEBUG

#define RefsAssert(cond)      /* disabled */

#endif  // } !REFS_DEBUG

/*============================================================================
==  Definitions
============================================================================*/

#include <string.h>
#include "util.h"

inline char *new_strdup(const char *str)
{
	int len = strlen(str) + 1;
	char *newstr = new char[len];
	strcpy_s(newstr, len, str);
	return newstr;
}

/*----------------------------------------------------------------------------
--  Translate
----------------------------------------------------------------------------*/

#include "translate.h"

#define _(str) Translate(str)
#define N_(str) str

/*----------------------------------------------------------------------------
--  General
----------------------------------------------------------------------------*/

#define VERSION  "2.3.0"                 /// Engine version shown

#ifndef StratagusMajorVerion
	/// Stratagus major version
#define StratagusMajorVersion  2
	/// Stratagus minor version (maximal 99)
#define StratagusMinorVersion  3
	/// Stratagus patch level (maximal 99)
#define StratagusPatchLevel    0
	/// Stratagus version (1,2,3) -> 10203
#define StratagusVersion \
	(StratagusMajorVersion * 10000 + StratagusMinorVersion * 100 \
		+ StratagusPatchLevel)

	/// Stratagus printf format string
#define StratagusFormatString   "%d.%d.%d"
	/// Stratagus printf format arguments
#define StratagusFormatArgs(v)  (v) / 10000, ((v) / 100) % 100, (v) % 100
#endif

	/// Text string: Name, Version, Copyright
extern char NameLine[];

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

#define MAGIC_FOR_NEW_UNITS  33         /// How many percent of max mana for new units

/*----------------------------------------------------------------------------
--  Some limits
----------------------------------------------------------------------------*/

#define PlayerMax    8                  /// How many players are supported
#define UnitTypeMax  257                /// How many unit types supported
#define UpgradeMax   8                  /// How many upgrades supported
#define UnitMax      2048               /// How many units supported

/*----------------------------------------------------------------------------
--  Screen
----------------------------------------------------------------------------*/

	/// Scrolling area (<= 15 y)
#define SCROLL_UP     15
	/// Scrolling area (>= VideoHeight - 16 y)
#define SCROLL_DOWN   (Video.Height - 16)
	/// Scrolling area (<= 15 y)
#define SCROLL_LEFT   15
	/// Scrolling area (>= VideoWidth - 16 x)
#define SCROLL_RIGHT  (Video.Width - 16)

	/// Frames per second to display (original 30-40)
#define FRAMES_PER_SECOND  30  // 1/30s
	/// Game cycles per second to simulate (original 30-40)
#define CYCLES_PER_SECOND  30  // 1/30s 0.33ms

/*----------------------------------------------------------------------------
--  stratagus.c
----------------------------------------------------------------------------*/

class CFont;

enum {
	TitleFlagCenter = 1 << 0,  /// Center Text
};

class TitleScreenLabel {
public:
	TitleScreenLabel() : Font(0), Xofs(0), Yofs(0), Flags(0) {}

	std::string Text;
	CFont *Font;
	int Xofs;
	int Yofs;
	int Flags;
};

class TitleScreen {
public:
	TitleScreen() : Timeout(0), Iterations(0), Labels(NULL) {}
	~TitleScreen() {
		if (this->Labels) {
			for (int i = 0; this->Labels[i]; ++i) {
				delete this->Labels[i];
			}
			delete[] this->Labels;
		}
	}

	std::string File;
	std::string Music;
	int Timeout;
	int Iterations;
	TitleScreenLabel **Labels;
};

extern TitleScreen **TitleScreens;          /// File for title screen
extern std::string UserDirectory;           /// Directory containing user settings and data
extern std::string ClickMissile;            /// Missile to show when you click
extern std::string DamageMissile;           /// Missile to show damage caused
extern std::string StratagusLibPath;        /// Location of stratagus data

extern int SpeedBuild;                      /// Speed factor for building
extern int SpeedTrain;                      /// Speed factor for training
extern int SpeedUpgrade;                    /// Speed factor for upgrading
extern int SpeedResearch;                   /// Speed factor for researching

extern bool UseHPForXp;                     /// true if gain XP by dealing damage, false if by killing.

extern unsigned long GameCycle;             /// Game simulation cycle counter
extern unsigned long FastForwardCycle;      /// Game Replay Fast Forward Counter

extern void LoadGame(const std::string &filename); /// Load saved game
extern void SaveGame(const std::string &filename); /// Save game
extern int SaveGameLoading;                 /// Save game is in progress of loading
struct lua_State;
extern char *SaveGlobal(lua_State *l, bool is_root); /// For saving lua state

extern void Exit(int err);                  /// Exit stratagus
extern void ExitFatal(int err);             /// Exit stratagus with fatal error

extern void UpdateDisplay(void);            /// Game display update
extern void InitModules(void);              /// Initinalize all modules
extern void LoadModules(void);              /// Load all modules
extern void CleanModules(void);             /// Cleanup all modules
extern void DrawMapArea(void);              /// Draw the map area
extern void GameMainLoop(void);             /// Game main loop

	/// Show load progress
extern void ShowLoadProgress(const char *fmt, ...);

/*============================================================================
==  Misc
============================================================================*/

// @todo configurable. maybe we could move it into one big global
// @todo settings struct?
	/// How many resources the player gets back if canceling building
#define CancelBuildingCostsFactor  75
	/// How many resources the player gets back if canceling training
#define CancelTrainingCostsFactor  100
	/// How many resources the player gets back if canceling research
#define CancelResearchCostsFactor  100
	/// How many resources the player gets back if canceling upgrade
#define CancelUpgradeCostsFactor   100

extern std::string CompileOptions;
//@}

#endif // !__STRATAGUS_H__
