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
/**@name settings.h	-	The game settings headerfile. */
//
//	(c) Copyright 2000-2002 by Andreas Arens
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

#ifndef __SETTINGS_H__
#define __SETTINGS_H__

//@{

/*----------------------------------------------------------------------------
--	Includes
----------------------------------------------------------------------------*/

#ifndef __STRUCT_SETTINGS__
#define __STRUCT_SETTINGS__
typedef struct _settings_ Settings;
#endif

#include "player.h"
#include "map.h"

/*----------------------------------------------------------------------------
--	Settings  __WIP__
----------------------------------------------------------------------------*/

/**
**	Settings structure
**
**	This structure one day should contain all common game settings,
**	in-game, or pre-start, and the individual (per player) presets.
**	This allows central maintainance, easy (network-)negotiation,
**	simplifies load/save/reinitialization, etc...
**
*/
struct _settings_ {
//  Individual presets:
//  For single-player game only Presets[0] will be used..
    struct {
	unsigned	Race;		/// Race of the player
	unsigned	Team;		/// Team of player -- NOT SELECTABLE YET
	unsigned	Type;		/// Type of player (for network games)
    }	Presets[PlayerMax];

//  Common settings:
    unsigned	Resources;		/// Preset resource factor
    unsigned	NumUnits;		/// Preset # of units
    unsigned	Opponents;		/// Preset # of ai-opponents
    unsigned	Terrain;		/// Terrain type (summer,winter,...)
    unsigned	GameType;		/// Game type (melee, free for all,...)
};

#define SettingsPresetMapDefault	(~0ul)	/// Special: Use pud/cm supplied

/*
**	Resource-Preset factor
*/
#define SettingsResourcesMapDefault	SettingsPresetMapDefault
#define SettingsResourcesLow		0
#define SettingsResourcesMedium		1
#define SettingsResourcesHigh		2

/*
**	NumUnits start settings
*/
#define SettingsNumUnitsMapDefault	SettingsPresetMapDefault
#define SettingsNumUnits1		0

/*
**	GameType settings
*/
enum {
    SettingsGameTypeMapDefault=SettingsPresetMapDefault,
    SettingsGameTypeMelee=0,
    SettingsGameTypeFreeForAll,
    SettingsGameTypeOneOnOne,
    SettingsGameTypeCaptureTheFlag,
    SettingsGameTypeGreed,
    SettingsGameTypeSlaughter,
    SettingsGameTypeSuddenDeath,
    SettingsGameTypeTeamMelee,
    SettingsGameTypeTeamCaptureTheFlag,
    SettingsGameTypeTopVsBottom,
    SettingsGameTypeLeftVsRight,
} GameTypes;


// ----------------------------------------------------------------------------

#define MAX_BRIEFING_VOICES	2	/// How many intro voices supported
#define MAX_OBJECTIVES		9	/// How many objectives supported

typedef struct _intro_ {
    char*	Title;				/// Intro title
    char*	Background;			/// Background picture
    char*	TextFile;			/// Intro text file
    char*	VoiceFile[MAX_BRIEFING_VOICES];	/// Intro voice file
    char*	Objectives[MAX_OBJECTIVES];	/// Objectives text
} Intro;					/// Intro definition


// ----------------------------------------------------------------------------

typedef struct _credits_ {
    char*	Background;		/// Background picture
    char*	Names;			/// Names
} Credits;

// ----------------------------------------------------------------------------

#define MAX_TIPS	50		/// How many tips supported

extern char*	Tips[MAX_TIPS+1];	/// Array of tips
extern int	ShowTips;		/// Show tips at start of level
extern int	CurrentTip;		/// Current tip to display

/*----------------------------------------------------------------------------
--	Variables
----------------------------------------------------------------------------*/

extern Settings GameSettings;		/// Game settings
extern Intro	GameIntro;		/// Game intro
extern Credits	GameCredits;		/// Game credits

/*----------------------------------------------------------------------------
--	Functions
----------------------------------------------------------------------------*/

    /// Show level intro
extern void ShowIntro(const Intro* intro);
    /// Show game credits
extern void ShowCredits();
    /// Show picture
extern void ShowPicture(const char* act,const char* title,const char* picture);
    /// Show stats
extern void ShowStats();
    /// Register Ccl
extern void CreditsCclRegister(void);
    /// Create a game
extern void CreateGame(char* filename,WorldMap* map);
    /// Init Setting to default values
extern void InitSettings(void);

//@}

#endif // !__SETTINGS_H__
