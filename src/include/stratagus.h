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

#define WIN32_LEAN_AND_MEAN
#define NOUSER

#define NOMINMAX // do not use min, max as macro

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

#ifndef __func__
#define __func__ __FUNCTION__
#endif

#endif  // } _MSC_VER

#ifdef __GNUC__
#ifdef USE_WIN32
#define makedir(dir, permissions) mkdir(dir)
#else
#define makedir(dir, permissions) mkdir(dir, permissions)
#endif
#endif

/*============================================================================
==  Macro
============================================================================*/

// To remove warning for unused variable.
#ifdef __GNUC__
#define UNUSED(var) do {__typeof__ (&var) __attribute__ ((unused)) __tmp = &var; } while(0)
#else
#define UNUSED(var) (var)
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
	do { fprintf(stdout, "%s:%d: %s: ", __FILE__, __LINE__, __func__); } while (0)


#ifdef DEBUG  // {

	/**
	**  Assert a condition. If cond is not true abort with file,line.
	*/
#define Assert(cond)  do { if (!(cond)) { \
	fprintf(stderr, "Assertion failed at %s:%d: %s: %s\n", __FILE__, __LINE__, __func__, #cond); \
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
	fprintf(stderr, "Assertion failed at %s:%d: %s\n", __FILE__, __LINE__, __func__); \
	abort(); } } while (0)

#else  // }{ REFS_DEBUG

#define RefsAssert(cond)      /* disabled */

#endif  // } !REFS_DEBUG

/*============================================================================
==  Definitions
============================================================================*/

#include <string.h>

#ifndef __UTIL_H__
#include "util.h"
#endif

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

#include "version.h"

	/// Text string: Name, Version, Copyright
extern const char NameLine[];

/*----------------------------------------------------------------------------
--  Some limits
----------------------------------------------------------------------------*/

#define PlayerMax    16                 /// How many players are supported
#define UnitTypeMax  2048                /// How many unit types supported
#define UpgradeMax   2048                /// How many upgrades supported
#define UnitMax      65536               /// How many units supported

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

class Parameters
{
public:
	void SetDefaultValues();

	const std::string& GetUserDirectory() { return UserDirectory; }

private:
	void SetUserDirectory();

public:
	std::string applicationName;
	std::string luaStartFilename;
	std::string luaEditorStartFilename;
	std::string LocalPlayerName;        /// Name of local player
private:
	std::string UserDirectory;          /// Directory containing user settings and data
public:
	static Parameters Instance;
};



extern std::string StratagusLibPath;        /// Location of stratagus data
extern std::string GameName;                /// Name of the game
extern std::string FullGameName;            /// Full Name of the game
extern std::string ClickMissile;            /// Missile to show when you click
extern std::string DamageMissile;           /// Missile to show damage caused
extern std::string MenuRace;

extern int SpeedBuild;                      /// Speed factor for building
extern int SpeedTrain;                      /// Speed factor for training
extern int SpeedUpgrade;                    /// Speed factor for upgrading
extern int SpeedResearch;                   /// Speed factor for researching

extern bool UseHPForXp;                     /// true if gain XP by dealing damage, false if by killing.

extern unsigned long GameCycle;             /// Game simulation cycle counter
extern unsigned long ResultGameCycle;       /// Used in game result
extern unsigned long FastForwardCycle;      /// Game Replay Fast Forward Counter

extern void LoadGame(const std::string &filename); /// Load saved game
extern int SaveGame(const std::string &filename); /// Save game
extern void DeleteSaveGame(const std::string &filename); /// Delete save game
extern bool SaveGameLoading;                 /// Save game is in progress of loading
struct lua_State;
extern std::string SaveGlobal(lua_State *l, bool is_root); /// For saving lua state

extern void Exit(int err);                  /// Exit
extern void ExitFatal(int err);             /// Exit with fatal error

extern void UpdateDisplay();            /// Game display update
extern void InitModules();              /// Initialize all modules
extern void LoadModules();              /// Load all modules
extern void CleanModules();             /// Cleanup all modules
extern void DrawMapArea();              /// Draw the map area
extern void GameMainLoop();             /// Game main loop

	/// Show load progress
extern void ShowLoadProgress(const char *fmt, ...);

struct DisplayAutoLocker {
	DisplayAutoLocker();
	~DisplayAutoLocker();
};

extern const int CPU_NUM;
extern bool CanAccessFile(const char *filename);

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

//@}

#endif // !__STRATAGUS_H__
