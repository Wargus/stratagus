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
/**@name freecraft.h	-	The main header file. */
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

#ifndef __FREECRAFT_H__
#define __FREECRAFT_H__

//@{

/*============================================================================
==	Config definitions
============================================================================*/

#define noDEBUG				/// Define to include debug code
#define noFLAG_DEBUG			/// Define to include map flag debug

#define noUSE_THREAD			/// Remove no for version with thread

#define noUSE_SDL			/// Remove no for sdl support
#define noUSE_SDLA			/// Remove no for sdl audio support
#define noUSE_X11			/// Remove no for x11 support
#define noUSE_SVGALIB			/// Remove no for svgalib support
#define noUSE_WINCE			/// Remove no for win-ce video support

    /**
    **	Define this to support load of compressed (gzip) pud files
    **	and other data files. (If defined you need libz)
    **	Comment if you have problems with gzseek, ... and other gz functions.
    */
#define noUSE_ZLIB

    /**
    **	Define this to support load of compressed (libbz2) pud files
    **	and other data files. (If defined you need libbz2)
    */
#define noUSE_BZ2LIB

    /**
    **	Define this to support data files stored in a single zip archive or
    **	multiple archives. (If defined you need libzziplib)
    */
#define noUSE_ZZIPLIB

    //
    //	Default speed for many things, set it higher for faster actions.
    //
#define SPEED_MINE	1		/// Speed factor for mine gold
#define SPEED_GOLD	1		/// Speed factor for getting gold
#define SPEED_CHOP	1		/// Speed factor for chop
#define SPEED_WOOD	1		/// Speed factor for getting wood
#define SPEED_HAUL	1		/// Speed factor for haul oil
#define SPEED_OIL	1		/// Speed factor for getting oil
#define SPEED_BUILD	1		/// Speed factor for building
#define SPEED_TRAIN	1		/// Speed factor for training
#define SPEED_UPGRADE	1		/// Speed factor for upgrading
#define SPEED_RESEARCH	1		/// Speed factor for researching

/*============================================================================
==	Compiler repairs
============================================================================*/

#ifdef __GNUC__	// {

#if __GNUC__==2 && __GNUC_MINOR__==96

#if !defined(__I_KNOW_THAT_GNUC_296_IS_BUGGY__)
#warning "GCC 2.96 is not supported and buggy, downgrade to GCC 2.95"
#endif

#endif

#if __GNUC__>=3

#if !defined(__I_KNOW_THAT_GNUC_3_IS_UNSUPPORTED__)
#warning "GCC 3.XX is not supported, downgrade to GCC 2.95"
#endif

//	It looks that GCC 3.xx is becoming nutty:
//	__FUNCTION__	can't be concated in the future.
//	__func__	Is defined by ISO C99 as
//		static const char __func__[] = "function-name";
#define __FUNCTION__ "Wrong compiler:"__FILE__

#endif

#if defined(__MINGW32__) && defined(DEBUG)
// GDB + MINGW doesn't like free(0)
#include <stdlib.h>
#define free(x) do { void* __x; __x=(x); if( __x ) free( __x ); } while(0)
#endif

#endif	// } __GNUC__

#ifndef __GNUC__	// { disable GNU C Compiler features

#define __attribute__(args)		/// Does nothing for non GNU CC

#endif	// }

#ifdef _MSC_VER	// { m$ auto detection

#define WIN32_LEAN_AND_MEAN
#define NOUSER

#define inline __inline			/// Fix m$ brain damage
#define alloca _alloca			/// I hope this works with all VC..

#ifndef _WIN32_WCE
#pragma warning(disable:4244)		// Conversion from double to uchar
#pragma warning(disable:4761)		// Integral size mismatch
#define snprintf _snprintf		/// Unix -> dumm
#include <string.h>
#define strdup _strdup
#define strncasecmp strnicmp
#endif


#ifndef __FUNCTION__

#define __FUNCTION__ __FILE__  /* ":" __LINE__ */

    /// Print function in debug macros
#define PrintFunction() \
    do { fprintf(stdout,"%s:%d: ",__FILE__,__LINE__); } while(0)

#endif

#endif	// } m$

#ifndef PrintFunction
    /// Print function in debug macros
#define PrintFunction() do { fprintf(stdout,__FUNCTION__": "); } while(0)
#endif

/*============================================================================
==	Includes
============================================================================*/

#include "fc_types.h"

/*============================================================================
==	Debug definitions
============================================================================*/

    /**
    **	This simulates vararg macros.
    **	@example
    **		DebugLevel0("Test %d\n" _C_ 1);
    **		DebugLevel0("Test %d %d\n" _C_ 1 _C_ 2);
    */
#define _C_	,			/// Debug , to simulate vararg macros

#ifdef DEBUG	// {

    /**
    **	Include code only if debugging.
    */
#define IfDebug(code)	code

    /**
    **	Debug check condition. If cond is true abort with file,line.
    */
#define DebugCheck(cond)	do{ if( cond ) { \
	fprintf(stderr,"DebugCheck at %s:%d\n",__FILE__,__LINE__); \
	abort(); } }while( 0 )

    /**
    **	Print debug information of level 0.
    */
#define DebugLevel0(args) \
	do { fprintf(stdout,args); } while(0)

    /**
    **	Print debug information of level 1.
    */
#define DebugLevel1(args)\
	do { fprintf(stdout,args); } while(0)

    /**
    **	Print debug information of level 2.
    */
#define DebugLevel2(args)\
	do { fprintf(stdout,args); } while(0)

    /**
    **	Print debug information of level 3. (normal = disable)
    */
#define DebugLevel3(args) \
	/* TURNED OFF: do { fprintf(stdout,args); } while(0) */

    /**
    **	Print debug information of level 0 with function name.
    */
#define DebugLevel0Fn(args) \
	do { PrintFunction(); fprintf(stdout,args); } while(0)

    /**
    **	Print debug information of level 1 with function name.
    */
#define DebugLevel1Fn(args) \
	do { PrintFunction(); fprintf(stdout,args); } while(0)

    /**
    **	Print debug information of level 2 with function name.
    */
#define DebugLevel2Fn(args) \
	do { PrintFunction(); fprintf(stdout,args); } while(0)

    /**
    **	Print debug information of level 3 with function name.
    */
#define DebugLevel3Fn(args) \
	/* TURNED OFF: do { fprintf(stdout,__FUNCTION__": " args); } while(0) */

#else	// }{ DEBUG

#define IfDebug(code)		/* disabled */
#define DebugCheck(cond)	/* disabled */

#define DebugLevel0(args)	/* disabled */
#define DebugLevel1(args)	/* disabled */
#define DebugLevel2(args)	/* disabled */
#define DebugLevel3(args)	/* disabled */
#define DebugLevel0Fn(args)	/* disabled */
#define DebugLevel1Fn(args)	/* disabled */
#define DebugLevel2Fn(args)	/* disabled */
#define DebugLevel3Fn(args)	/* disabled */

#endif	// } !DEBUG

#ifdef REFS_DEBUG	// {

    /**
    **	Debug check condition for references
    */
#define RefsDebugCheck(cond)	do{ if( cond ) { \
	fprintf(stderr,"DebugCheck at %s:%d\n",__FILE__,__LINE__); \
	abort(); } }while( 0 )

#else	// }{ REFS_DEBUG

#define RefsDebugCheck(cond)	/* disabled */

#endif	// } !REFS_DEBUG

/*============================================================================
==	Storage types
============================================================================*/

#define global				/// Defines global visible names

#ifdef DEBUG
#define local				/// Defines local visible names
#else
#define local static
#endif

/*============================================================================
==	Definitions
============================================================================*/

/*----------------------------------------------------------------------------
--	General
----------------------------------------------------------------------------*/

#ifndef VERSION
#define VERSION	"1.18pre1"		/// Engine version shown
#endif

#ifndef FreeCraftMajorVerion
    /// FreeCraft major version
#define FreeCraftMajorVersion	1
    /// FreeCraft minor version (maximal 99)
#define FreeCraftMinorVersion	17
    /// FreeCraft patch level (maximal 99)
#define FreeCraftPatchLevel	99
    /// FreeCraft version (1,2,3) -> 10203
#define FreeCraftVersion \
	(FreeCraftMajorVersion*10000+FreeCraftMinorVersion*100 \
	+FreeCraftPatchLevel)

    /// FreeCraft printf format string
#define FreeCraftFormatString	"%d.%d.%d"
    /// FreeCraft printf format arguments
#define FreeCraftFormatArgs(v)	(v)/10000,((v)/100)%100,(v)%100
#endif

#ifndef FREECRAFT_LIB_PATH
#define FREECRAFT_LIB_PATH "data"	/// Where to find the data files
#endif
#ifndef FREECRAFT_HOME_PATH
#define FREECRAFT_HOME_PATH ".freecraft"/// Data files in user home dir
#endif

#define MAGIC_FOR_NEW_UNITS	33	/// How many percent of max mana for new units
#define DEMOLISH_DAMAGE		400	/// Damage for demolish attack

/*----------------------------------------------------------------------------
--	Some limits
----------------------------------------------------------------------------*/

#define TilesetMax	8		/// How many tilesets are supported
#define PlayerMax	16		/// How many players are supported
#define UnitTypeMax	0xFF		/// How many unit types supported
#define UpgradeMax	256		/// How many upgrades supported
#define UnitMax		2048		/// How many units supported

/*----------------------------------------------------------------------------
--	Screen
----------------------------------------------------------------------------*/

    // FIXME: this values should go into a general ui structure

#define noGRID		1		/// Map is shown with a grid, if 1

#define DEFAULT_VIDEO_WIDTH	640	/// Default video width
#define DEFAULT_VIDEO_HEIGHT	480	/// Default video height

    // This is for 1600x1200
#define MAXMAP_W	50		/// Maximum map width in tiles on screen
#define MAXMAP_H	40		/// Maximum map height in tiles

#define MINIMAP_W	128		/// Minimap width in pixels
#define MINIMAP_H	128		/// Minimap height in pixels

    /// Scrolling area (<= 15 y)
#define SCROLL_UP	15
    /// Scrolling area (>= VideoHeight-16 y)
#define SCROLL_DOWN	(VideoHeight-16)
    /// Scrolling area (<= 15 y)
#define SCROLL_LEFT	15
    /// Scrolling area (>= VideoWidth-16 x)
#define SCROLL_RIGHT	(VideoWidth-16)

    /// Mouse scrolling magnify
#define MOUSE_SCROLL_SPEED	3

    /// Keyboard scrolling magnify
#define KEY_SCROLL_SPEED	3

    /// Frames per second to display (original 30-40)
#define FRAMES_PER_SECOND	30	// 1/30s
    /// Game cycles per second to simulate (original 30-40)
#define CYCLES_PER_SECOND	30	// 1/30s 0.33ms

    /// Must redraw flags
enum _must_redraw_flags_ {
    RedrawNothing	= 1<< 0,	/// Nothing to do
    RedrawMinimap	= 1<< 1,	/// Minimap area
    RedrawMap		= 1<< 2,	/// Map area
    RedrawCursor	= 1<< 3,	/// Cursor changed
    RedrawResources	= 1<< 4,	/// Resources

/* FIXME: Next is planned to reduce the area of redraws
    RedrawGold		= 1<< 5,	/// Resources 1 gold
    RedrawWood		= 1<< 6,	/// Resources 2 wood
    RedrawOil		= 1<< 7,	/// Resources 3 oil
    RedrawOre		= 1<< 8,	/// Resources 4 ore
    RedrawStone		= 1<< 9,	/// Resources 5 stone
    RedrawCoal		= 1<<10,	/// Resources 6 coal
    RedrawFood		= 1<<11,	/// Resources F food supply / demand
    RedrawPoints	= 1<<12,	/// Resources S score
*/
    RedrawMessage	= 1<<13,	/// Message
    RedrawStatusLine	= 1<<14,	/// Statusline
    RedrawInfoPanel	= 1<<15,	/// Unit description
    RedrawButtonPanel	= 1<<16,	/// Unit buttons
    RedrawFiller1	= 1<<17,	/// Filler1: Border on right side
    RedrawMinimapBorder	= 1<<18,	/// Area around minimap
    RedrawCosts		= 1<<19,	/// Costs in status line
    RedrawMenuButton	= 1<<20,	/// Area above minimap
    RedrawMinimapCursor	= 1<<21,	/// Minimap cursor changed
    RedrawMenu		= 1<<22,	/// Menu
    RedrawTimer		= 1<<23,	/// Timer

    // Bits 23-29 are unused.

    RedrawAll		= 1<<30,	/// All flag set by RedrawEverything
    RedrawEverything	= -1,		/// Must redraw everything
};

    /// Must redraw all maps
#define RedrawMaps		(RedrawMinimap|RedrawMap)
    /// Must redraw all cursors
#define RedrawCursors		(RedrawMinimapCursor|RedrawCursor)
    /// Must redraw all panels
#define RedrawPanels		(RedrawInfoPanel|RedrawButtonPanel)
    /// Must redraw after color cycle
#define RedrawColorCycle	(RedrawMap | RedrawInfoPanel | RedrawButtonPanel | RedrawResources)

    /// Mainscreen pitch (default VideoWidth)
extern int VideoPitch;

    /// Mainscreen width (default 640)
extern int VideoWidth;

    /// Mainscreen height (default 480)
extern int VideoHeight;

    /// Invalidated redraw flags
extern int MustRedraw;

    /// Enable redraw flags
extern int EnableRedraw;

    /// Next frame ticks
extern unsigned long NextFrameTicks;

    /// Counts frames
extern unsigned long FrameCounter;

    /// Counts quantity of slow frames
extern int SlowFrameCounter;

/*----------------------------------------------------------------------------
--	clone.c
----------------------------------------------------------------------------*/

    /**
    **	MyRand():	rand only used on this computer.
    */
#define MyRand()	rand()

extern char* TitleScreen;		/// File for title screen
extern char* MenuBackground;		/// File for menu background
extern char* MenuBackgroundWithTitle;	/// File for menu with title
extern char* TitleMusic;		/// File for title music
extern char* MenuMusic;			/// File for menu music
extern char* FreeCraftLibPath;		/// Location of freecraft data

extern int SpeedMine;			/// Speed factor for mine gold
extern int SpeedGold;			/// Speed factor for getting gold
extern int SpeedChop;			/// Speed factor for chop
extern int SpeedWood;			/// Speed factor for getting wood
extern int SpeedHaul;			/// Speed factor for haul oil
extern int SpeedOil;			/// Speed factor for getting oil
extern int SpeedBuild;			/// Speed factor for building
extern int SpeedTrain;			/// Speed factor for training
extern int SpeedUpgrade;		/// Speed factor for upgrading
extern int SpeedResearch;		/// Speed factor for researching

    //FIXME: all game global options should be moved in structure like `TheUI'
extern int OptionUseDepletedMines;      /// Use depleted mines or destroy them

extern unsigned SyncRandSeed;		/// Sync random seed value

extern unsigned long GameCycle;		/// Game simulation cycle counter

extern void LoadGame(char*);		/// Load saved game back
extern void SaveGame(const char*);	/// Save game for later load

extern void LoadAll(void);		/// Load all data back

extern void InitSyncRand(void);		/// Initialize the syncron rand
extern int SyncRand(void);		/// Syncron rand

extern int main1(int argc,char* argv[]);/// Init freecraft
extern volatile void Exit(int err);	/// Exit freecraft
extern volatile void ExitFatal(int err);/// Exit freecraft with fatal error

extern void UpdateDisplay(void);	/// Game display update
extern void InitModules(void);		/// Initinalize all modules
extern void LoadModules(void);		/// Load all modules
extern void CleanModules(void);		/// Cleanup all modules
extern void DrawMapArea(void);		/// Draw the map area
extern void GameMainLoop(void);		/// Game main loop

    ///	Show load progress
extern void ShowLoadProgress(const char* fmt,...);

    /// strdup + strcat
extern char* strdcat(const char* l, const char* r);
    /// strdup + strcat + strcat
extern char* strdcat3(const char* l, const char *m, const char* r);

#if !defined(BSD) || defined(__APPLE__)
    /// case insensitive strstr
extern char* strcasestr(const char* str, const char* substr);
#endif // !BSD || APPLE

/*============================================================================
==	Misc
============================================================================*/

#if !defined(_MSC_VER) || defined(_WIN32_WCE)
#ifndef max
    /// max macro
#define max(n1,n2)	(((n1)<(n2)) ? (n2) : (n1))
#endif
#endif

    /// bits macro
#define BitsOf(n)	(sizeof(n)*8)

    // FIXME: more config stuff which needs a better place

    /// How long stay in a gold-mine
#define MINE_FOR_GOLD	(UnitTypeGoldMine->_Costs[TimeCost]/SpeedMine)
    /// How long stay in a gold-deposit
#define WAIT_FOR_GOLD	(UnitTypeGoldMine->_Costs[TimeCost]/SpeedGold)
    /// How much I must chop for 1 wood
#define CHOP_FOR_WOOD	(52/SpeedChop)
    /// How long stay in a wood-deposit
#define WAIT_FOR_WOOD	(100/SpeedWood)
    /// How long stay in a oil-well
#define HAUL_FOR_OIL	(100/SpeedHaul)
    /// How long stay in a oil-deposit
#define WAIT_FOR_OIL	(100/SpeedOil)

    /// How many resource get the player back if canceling building
#define CancelBuildingCostsFactor	75
    /// How many resource get the player back if canceling training
#define CancelTrainingCostsFactor	100
    /// How many resource get the player back if canceling research
#define CancelResearchCostsFactor	100
    /// How many resource get the player back if canceling upgrade
#define CancelUpgradeCostsFactor	100

    /// How near could a hall or gold-depot be build to a goldmine
#define GOLDMINE_DISTANCE	3
    /// How near could a oil-depot be build to a oil-patch
#define OILPATCH_DISTANCE	3

    /// How near we could repair an unit
#define REPAIR_RANGE		1

//@}

#endif	// !__FREECRAFT_H__
