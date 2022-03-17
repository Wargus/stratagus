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

/*
 * This header file defines all the structures that need to be consistent for replays, savegame loading, and
 * multiplayer games.
 */

//@{

#include <bitset>
#include <stdint.h>
#include <string>
#include <vector>
#include <functional>
#include <variant>

/*----------------------------------------------------------------------------
--  TODO: Remove this once we move off of tolua++ and can properly support strongly typed enums
----------------------------------------------------------------------------*/
#define USING_TOLUAPP 1
#if USING_TOLUAPP
// using tolua++
#define ENUM_CLASS enum
#else
#define ENUM_CLASS enum class
#endif

/*----------------------------------------------------------------------------
--  Some limits
----------------------------------------------------------------------------*/

constexpr unsigned char PlayerMax = 16;        /// How many players are supported
constexpr unsigned short UnitTypeMax = 2048;                /// How many unit types supported
constexpr unsigned short UpgradeMax = 2048;                /// How many upgrades supported
constexpr unsigned char MAX_RACES = 8;
constexpr unsigned char PlayerNumNeutral = PlayerMax - 1;  /// this is the neutral player slot

/// Frames per second to display (original 30-40)
constexpr unsigned char FRAMES_PER_SECOND = 30; // 1/30s
/// Game cycles per second to simulate (original 30-40)
constexpr unsigned char CYCLES_PER_SECOND = 30;  // 1/30s 0.33ms

/*----------------------------------------------------------------------------
--  Settings
----------------------------------------------------------------------------*/

constexpr int8_t SettingsPresetMapDefault = -1;  /// Special: Use map supplied

/**
**  Types for the player
**
**  #PlayerNeutral
**
**    This player is controlled by the computer doing nothing.
**
**  #PlayerNobody
**
**    This player is unused. Nobody controls this player.
**
**  #PlayerComputer
**
**    This player is controlled by the computer. CPlayer::AiNum
**    selects the AI strategy.
**
**  #PlayerPerson
**
**    This player is contolled by a person. This can be the player
**    sitting on the local computer or player playing over the
**    network.
**
**  #PlayerRescuePassive
**
**    This player does nothing, the game pieces just sit in the game
**    (being passive)... when a person player moves next to a
**    PassiveRescue unit/building, then it is "rescued" and becomes
**    part of that persons team. If the city center is rescued, than
**    all units of this player are rescued.
**
**  #PlayerRescueActive
**
**    This player is controlled by the computer. CPlayer::AiNum
**    selects the AI strategy. Until it is rescued it plays like
**    an ally. The first person which reaches units of this player,
**    can rescue them. If the city center is rescued, than all units
**    of this player are rescued.
*/
ENUM_CLASS PlayerTypes : int8_t {
	MapDefault = SettingsPresetMapDefault, /// use default
	PlayerUnset = 0,                       /// not set
	PlayerNeutral = 2,                     /// neutral
	PlayerNobody  = 3,                     /// unused slot
	PlayerComputer = 4,                    /// computer player
	PlayerPerson = 5,                      /// human player
	PlayerRescuePassive = 6,               /// rescued passive
	PlayerRescueActive = 7                 /// rescued  active
};

extern std::string PlayerTypeNames[static_cast<int>(PlayerTypes::PlayerRescueActive) + 1];

static_assert(MAX_RACES < 256, "Race selection needs to fit into 8 bits");
static_assert(PlayerMax < 256, "Team number must fit into 8 bits");

struct SettingsPresets {
	int8_t PlayerColor;       /// Color of a player
	std::string AIScript;     /// AI script for computer to use
	int8_t Race;              /// Race of the player
	int8_t Team;              /// Team of player
	PlayerTypes Type;         /// Type of player (for network games)

	void Save(const std::function <void (std::string)>& f) {
		f(std::string("PlayerColor = ") + std::to_string(PlayerColor));
		f(std::string("AIScript = \"") + AIScript + "\"");
		f(std::string("Race = ") + std::to_string(Race));
		f(std::string("Team = ") + std::to_string(Team));
		f(std::string("Type = ") + std::to_string(static_cast<int>(Type)));
	}

	void Init() {
		PlayerColor = 0;
		AIScript = "ai-passive";
		Race = SettingsPresetMapDefault;
		Team = SettingsPresetMapDefault;
		Type = PlayerTypes::MapDefault;
	}

	bool operator==(const SettingsPresets &other) const {
		return PlayerColor == other.PlayerColor &&
			AIScript == other.AIScript &&
			Race == other.Race &&
			Team == other.Team &&
			Type == other.Type;
	}
};

ENUM_CLASS RevealTypes : uint8_t { 
	cNoRevelation, 
	cAllUnits, 
	cBuildingsOnly 
}; /// Revelation types

/**
**  Single or multiplayer settings
*/
ENUM_CLASS NetGameTypes : uint8_t {
	SettingsSinglePlayerGame,
	SettingsMultiPlayerGame,
	Unset
};

/**
**  GameType settings
*/
ENUM_CLASS GameTypes : int8_t {
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
ENUM_CLASS MapRevealModes : uint8_t {
	cHidden = 0,
	cKnown,
	cExplored,
	cNumOfModes
};

ENUM_CLASS FieldOfViewTypes : uint8_t {
	cShadowCasting,
	cSimpleRadial,
	NumOfTypes
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
	NetGameTypes NetGameType;   /// Multiplayer or single player

	//  Individual presets:
	//  For single-player game only Presets[0] will be used..
	SettingsPresets Presets[PlayerMax];

	//  Common settings:
	int8_t Resources;          /// Preset resource factor
	int8_t NumUnits;           /// Preset # of units
	int8_t Opponents;          /// Preset # of ai-opponents
	int8_t Difficulty;         /// Terrain type (summer,winter,...)
	GameTypes GameType;        /// Game type (melee, free for all,...)
	FieldOfViewTypes FoV;      /// Which field of view is used - important to be shared for unit sight
	MapRevealModes RevealMap;  /// Reveal map kind
	RevealTypes DefeatReveal;
	union {
		struct {
			unsigned NoFogOfWar:1;        /// if dynamic fog of war is disabled
			unsigned Inside:1;            /// if game uses interior tileset or is generally "inside" for the purpose of obstacles
			unsigned AiExplores:1;        /// If true, AI sends explorers to search for resources (almost useless thing)
			unsigned SimplifiedAutoTargeting:1; /// Use alternate target choosing algorithm for auto attack mode (idle, attack-move, patrol, etc.)
			unsigned AiChecksDependencies:1; /// If false, the AI can do upgrades even if the dependencies are not met. This can be desirable to simplify AI scripting.
			unsigned UserGameSettings:27; /// A bitfield for use by games and their settings
		};
		uint32_t _Bitfield;
	};

	bool GetUserGameSetting(int i) {
		return std::bitset<27>(UserGameSettings).test(i);
	}

	void SetUserGameSetting(int i, bool v) {
		std::bitset<27> bs(UserGameSettings);
		bs.set(i, v);
		UserGameSettings = bs.to_ulong();
	}

	bool operator==(const Settings &other) const {
		for (int i = 0; i < PlayerMax; i++) {
			if (Presets[i] == other.Presets[i]) {
				continue;
			} else {
				return false;
			}
		}
		return NetGameType == other.NetGameType &&
			Resources == other.Resources &&
			NumUnits == other.NumUnits &&
			Opponents == other.Opponents &&
			Difficulty == other.Difficulty &&
			GameType == other.GameType &&
			FoV == other.FoV &&
			RevealMap == other.RevealMap &&
			DefeatReveal == other.DefeatReveal &&
			_Bitfield == other._Bitfield;
	}

	void Save(const std::function <void (std::string)>& f, bool withPlayers = true) {
		f(std::string("NetGameType = ") + std::to_string(static_cast<int>(NetGameType)));
		if (withPlayers) {
			for (int i = 0; i < PlayerMax; ++i) {
				Presets[i].Save([&] (std::string field) {
					f(std::string("Presets[") + std::to_string(i) + "]." + field);
				});
			}
		}
		f(std::string("Resources = ") + std::to_string(Resources));
		f(std::string("NumUnits = ") + std::to_string(NumUnits));
		f(std::string("Opponents = ") + std::to_string(Opponents));
		f(std::string("Difficulty = ") + std::to_string(Difficulty));
		f(std::string("GameType = ") + std::to_string(static_cast<int>(GameType)));
		f(std::string("FoV = ") + std::to_string(static_cast<int>(FoV)));
		f(std::string("RevealMap = ") + std::to_string(static_cast<int>(RevealMap)));
		f(std::string("DefeatReveal = ") + std::to_string(static_cast<int>(DefeatReveal)));
		f(std::string("Flags = ") + std::to_string(_Bitfield));
	}

	bool SetField(std::string field, int value) {
		if (field == "NetGameType") {
			NetGameType = static_cast<NetGameTypes>(value);
		} else if (field == "Resources") {
			Resources = value;
		} else if (field == "NumUnits") {
			NumUnits = value;
		} else if (field == "Opponents") {
			Opponents = value;
		} else if (field == "Difficulty") {
			Difficulty = value;
		} else if (field == "GameType") {
			GameType = static_cast<GameTypes>(value);
		} else if (field == "FoV") {
			FoV = static_cast<FieldOfViewTypes>(value);
		} else if (field == "RevealMap") {
			RevealMap = static_cast<MapRevealModes>(value);
		} else if (field == "DefeatReveal") {
			DefeatReveal = static_cast<RevealTypes>(value);
		} else if (field == "Flags") {
			_Bitfield = value;
		} else {
			return false;
		}
		return true;
	}

	void Init() {
		NetGameType = NetGameTypes::SettingsSinglePlayerGame;
		for (int i = 0; i < PlayerMax; ++i) {
			Presets[i].Init();
			Presets[i].PlayerColor = i;
		}
		Resources = SettingsPresetMapDefault;
		NumUnits = SettingsPresetMapDefault;
		Opponents = SettingsPresetMapDefault;
		Difficulty = SettingsPresetMapDefault;
		GameType = GameTypes::SettingsGameTypeMapDefault;
		FoV = FieldOfViewTypes::cSimpleRadial;
		RevealMap = MapRevealModes::cHidden;
		DefeatReveal = RevealTypes::cAllUnits;
		NoFogOfWar = 0;
		Inside = 0;
		AiExplores = 1;
		AiChecksDependencies = 0;
		SimplifiedAutoTargeting = 0;
		UserGameSettings = 0;
	}
};

/*----------------------------------------------------------------------------
--  Variables
----------------------------------------------------------------------------*/

extern Settings GameSettings;  /// Game settings

/*----------------------------------------------------------------------------
--  Functions
----------------------------------------------------------------------------*/

/// Init Setting to default values
extern void InitSettings();

//@}

#endif // !__SETTINGS_H__
