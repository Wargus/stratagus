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

#include "stratagus.h"

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
 **
 **  This structure contains all settings that must be synchronized across
 **  clients in a network game or saved+restored to the same values during a
 **  replay to avoid desyncs.
 */
typedef struct _SharedGameSettings {
	int8_t Resources;   /// Preset resource factor
	int8_t NumUnits;    /// Preset # of units
	int8_t Opponents;   /// Preset # of ai-opponents
	int8_t Difficulty;

	int8_t GameType;    /// Game type (melee, free for all,...)
	int8_t NoFogOfWar; /// No fog of war
	int8_t Inside;     /// If game uses interior tileset
	int8_t RevealMap;   /// Reveal map

	int8_t MapRichness; /// Map richness
	int8_t TilesetSelection;  /// Tileset select option
	int8_t AiExplores; /// If true, AI sends explorers to search for resources (almost useless thing)
	int8_t SimplifiedAutoTargeting; /// Use alternate target choosing algorithm for auto attack mode (idle, attack-move, patrol, etc.)

	int8_t AiChecksDependencies; /// If false, the AI can do upgrades even if the dependencies are not met. This can be desirable to simplify AI scripting.
	int8_t AllyDepositsAllowed;  /// If true, allies can deposit resources in each others buildings

	// Game-specific settings that may cause desyncs can be stored in the following fields
	int8_t GameSetting01;
	int8_t GameSetting02;
	int8_t GameSetting03;
	int8_t GameSetting04;
	int8_t GameSetting05;
	int8_t GameSetting06;
} SharedGameSettings;

/**
**  Settings structure
**
**  This structure one day should contain all common game settings, in-game, or
**  pre-start, and the individual (per player) presets.  This allows central
**  maintenance, easy (network-)negotiation, simplifies
**  load/save/reinitialization, etc...
**
*/
struct Settings {
	int NetGameType;   /// Multiplayer or single player

	//  Individual presets:
	//  For single-player game only Presets[0] will be used..
	SettingsPresets Presets[PlayerMax];

	SharedGameSettings SharedSettings;
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
