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
//	(c) Copyright 2000 by Andreas Arens
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
	unsigned	Race;		/// race of the player
	unsigned	Team;		/// team of player -- NOT SELECTABLE YET
    }	Presets[PlayerMax];

//  Common settings:
    unsigned	Resources;		/// preset resource factor
    unsigned	NumUnits;		/// preset # of units
    unsigned	Opponents;		/// preset # of ai-opponents
    unsigned	Terrain;		/// terrain type (summer,winter,...)
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

/*----------------------------------------------------------------------------

--	Variables
----------------------------------------------------------------------------*/

extern Settings GameSettings;		/// Game Settings

/*----------------------------------------------------------------------------
--	Functions
----------------------------------------------------------------------------*/

    /// Create a game
extern void CreateGame(char* filename,WorldMap* map);
    /// Init Setting to default values
extern void InitSettings(void);

//@}

#endif // !__SETTINGS_H__
