//     ____                _       __               
//    / __ )____  _____   | |     / /___ ___________
//   / __  / __ \/ ___/   | | /| / / __ `/ ___/ ___/
//  / /_/ / /_/ (__  )    | |/ |/ / /_/ / /  (__  ) 
// /_____/\____/____/     |__/|__/\__,_/_/  /____/  
//                                              
//       A futuristic real-time strategy game.
//          This file is part of Bos Wars.
//
/**@name settings.h - The game settings headerfile. */
//
//      (c) Copyright 2000-2009 by Andreas Arens and Jimmy Salmon
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

#ifndef __SETTINGS_H__
#define __SETTINGS_H__

//@{

#include <vector>
#include "player.h"

class CFile;
class CMap;

struct SettingsPresets {
	int Team;          /// Team of player -- NOT SELECTABLE YET
	PlayerTypes Type;  /// Type of player (for network games)
};

/**
**  Settings structure
**
**  This structure one day should contain all common game settings,
**  in-game, or pre-start, and the individual (per player) presets.
**  This allows central maintainance, easy (network-)negotiation,
**  simplifies load/save/reinitialization, etc...
**
*/
struct Settings {
	int NetGameType;   /// Multiplayer or single player

//  Individual presets:
//  For single-player game only Presets[0] will be used..
	SettingsPresets Presets[PlayerMax];

//  Common settings:
	int Resources;   /// Preset resource factor
	int NumUnits;    /// Preset # of units
	int Opponents;   /// Preset # of ai-opponents
	int Difficulty;  /// Terrain type (summer,winter,...)
	int GameType;    /// Game type (melee, free for all,...)
	bool NoFogOfWar; /// No fog of war
	int RevealMap;   /// Reveal map
	int MapRichness; /// Map richness
};

#define SettingsPresetMapDefault  -1  /// Special: Use map supplied

/**
**  Single or multiplayer settings
*/
#define SettingsSinglePlayerGame  1
#define SettingsMultiPlayerGame   2

/**
**  GameType settings
*/
enum GameTypes {
	SettingsGameTypeMapDefault = SettingsPresetMapDefault,
	SettingsGameTypeMelee = 0,
	SettingsGameTypeFreeForAll,
	SettingsGameTypeTopVsBottom,
	SettingsGameTypeLeftVsRight,
	SettingsGameTypeManVsMachine,
	SettingsGameTypeManTeamVsMachine,

	// Future game type ideas
#if 0
	SettingsGameTypeOneOnOne,
	SettingsGameTypeCaptureTheFlag,
	SettingsGameTypeGreed,
	SettingsGameTypeSlaughter,
	SettingsGameTypeSuddenDeath,
	SettingsGameTypeTeamMelee,
	SettingsGameTypeTeamCaptureTheFlag,
#endif
};

extern Settings GameSettings;  /// Game settings

	/// Show stats
extern void ShowStats();
	/// Create a game
extern void CreateGame(const std::string &filename, CMap *map);
	/// Init Setting to default values
extern void InitSettings(void);

//@}

#endif
