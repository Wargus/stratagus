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
/**@name settings.h - The game settings headerfile. */
//
//      (c) Copyright 2000-2006 by Andreas Arens and Jimmy Salmon
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

#ifndef __SETTINGS_H__
#define __SETTINGS_H__

//@{

#include <string>
#include <vector>

/*----------------------------------------------------------------------------
--  Declarations
----------------------------------------------------------------------------*/

class CFile;
class CMap;


/*----------------------------------------------------------------------------
--  Settings
----------------------------------------------------------------------------*/

struct SettingsPresets {
	int PlayerColor;          /// Color of a player
	std::string AIScript;     /// AI script for computer to use
	int Race;                 /// Race of the player
	int Team;                 /// Team of player
	int Type;                 /// Type of player (for network games)
};

/**
**  Settings structure
**
**  This structure one day should contain all common game settings,
**  in-game, or pre-start, and the individual (per player) presets.
**  This allows central maintenance, easy (network-)negotiation,
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
	bool Inside;     /// If game uses interior tileset
	int RevealMap;   /// Reveal map { cHidden = 0, cKnown,	cExplored };
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
	SettingsGameTypeMachineVsMachine,
	SettingsGameTypeMachineVsMachineTraining

	// Future game type ideas
#if 0
	SettingsGameTypeOneOnOne,
	SettingsGameTypeCaptureTheFlag,
	SettingsGameTypeGreed,
	SettingsGameTypeSlaughter,
	SettingsGameTypeSuddenDeath,
	SettingsGameTypeTeamMelee,
	SettingsGameTypeTeamCaptureTheFlag
#endif
};
/// Map revealing modes: cHidden - unrevealed, cKnown - you can see unexplored tiles, but they are darker than explored
/// cExplored - all tiles became explored, and covered only by the fog of war if it's enabled.
enum MapRevealModes { cHidden = 0, cKnown, cExplored, cNumOfModes };

/*----------------------------------------------------------------------------
--  Variables
----------------------------------------------------------------------------*/

extern Settings GameSettings;  /// Game settings

/*----------------------------------------------------------------------------
--  Functions
----------------------------------------------------------------------------*/

/// Show stats
extern void ShowStats();
/// Create a game
extern void CreateGame(const std::string &filename, CMap *map);
/// Init Setting to default values
extern void InitSettings();

//@}

#endif // !__SETTINGS_H__
