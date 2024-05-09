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
/**@name player.h - The player headerfile. */
//
//      (c) Copyright 1998-2005 by Lutz Sammer and Jimmy Salmon
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

#ifndef __PLAYER_H__
#define __PLAYER_H__

//@{

/*----------------------------------------------------------------------------
--  Includes
----------------------------------------------------------------------------*/

#include <optional>
#include <set>
#include <string>
#include <vector>

#include "../ai/ai_local.h"
#include "color.h"
#include "settings.h"
#include "upgrade_structs.h"
#include "vec2i.h"


class CGraphic;

/*----------------------------------------------------------------------------
--  Definitons
----------------------------------------------------------------------------*/

enum class EStoreType
{
	Overall = 0,
	Building = 1,
	Both = 2,
};

#define SPEEDUP_FACTOR 100
/*----------------------------------------------------------------------------
--  Declarations
----------------------------------------------------------------------------*/

class CUnit;
class CUnitType;
class PlayerAi;
class CFile;
struct lua_State;

constexpr char DEFAULT_PASSIVE_AI[] = "ai-passive";
constexpr char DEFAULT_ACTIVE_AI[] = "ai-active";

/*----------------------------------------------------------------------------
--  Player type
----------------------------------------------------------------------------*/

enum class EDiplomacy {
	Allied,   /// Ally with opponent
	Neutral,  /// Don't attack be neutral
	Enemy,    /// Attack opponent
	Crazy     /// Ally and attack opponent
}; /// Diplomacy states for CommandDiplomacy

std::optional<EDiplomacy> DiplomacyFromString(std::string_view);
std::string_view ToString(EDiplomacy);

enum class ECheckLimit
{
	BuildingLimitReached = -1,
	UnitLimitReached = -2,
	InsufficientSupply = -3,
	TotalUnitLimitReached = -4,
	SpecificUnitLimitReached = -6,
	Ok = 1
};


///  Player structure
class CPlayer
{
	friend void CleanPlayers();
public:
	static inline RevealTypes RevelationFor { RevealTypes::cNoRevelation }; /// type of revelation (when player lost their last main facility)

public:
	/// Check if relevation enabled
	static const bool IsRevelationEnabled()
	{
		// By default there is no revelation. Can be changed in lua-script
		return CPlayer::RevelationFor != RevealTypes::cNoRevelation;
	}
	/// Change revelation type
	static void SetRevelationType(const RevealTypes type);
	/// Get revealed players list
	static const std::vector<const CPlayer *> &GetRevealedPlayers()
	{
		return CPlayer::RevealedPlayers;
	}

public:

	/// Change player name
	void SetName(const std::string &name);

	/// Clear turn related player data
	void Clear();

	const std::vector<CUnit *> &GetUnits() const { return this->Units; }
	CUnit &GetUnit(int index) const;
	int GetUnitCount() const;

	void AddUnit(CUnit &unit);
	void RemoveUnit(CUnit &unit);

	const std::vector<CUnit *> &GetFreeWorkers() const { return FreeWorkers; }
	void UpdateFreeWorkers();

	void ClearUnitColors();
	void SetUnitColors(std::vector<CColor> &colors);

	/// Get a resource of the player
	int GetResource(const int resource, const EStoreType type);
	/// Adds/subtracts some resources to/from the player store
	void ChangeResource(const int resource, const int value, const bool store = false);
	/// Set a resource of the player
	void SetResource(int resource, int value, EStoreType type = EStoreType::Overall);
	/// Check, if there enough resources for action.
	bool CheckResource(const int resource, const int value);

	/// Returns count of specified unittype
	int GetUnitTotalCount(const CUnitType &type) const;
	/// Check if the unit-type didn't break any unit limits and supply/demand
	ECheckLimit CheckLimits(const CUnitType &type) const;

	/// Check if enough resources are available for costs
	int CheckCosts(const int (&costs)[MaxCosts], bool notify = true) const;
	/// Check if enough resources are available for a new unit-type
	int CheckUnitType(const CUnitType &type) const;

	/// Add costs to the resources
	void AddCosts(const int (&costs)[MaxCosts]);
	/// Add costs for an unit-type to the resources
	void AddUnitType(const CUnitType &type);
	/// Add a factor of costs to the resources
	void AddCostsFactor(const int (&costs)[MaxCosts], int factor);
	/// Remove costs from the resources
	void SubCosts(const int (&costs)[MaxCosts]);
	/// Remove costs for an unit-type from the resources
	void SubUnitType(const CUnitType &type);
	/// Remove a factor of costs from the resources
	void SubCostsFactor(const int (&costs)[MaxCosts], int factor);

	/// Does the player have units of that type
	int HaveUnitTypeByType(const CUnitType &type) const;
	/// Does the player have units of that type
	int HaveUnitTypeByIdent(std::string_view ident) const;

	/// Notify player about a problem
	void Notify(IntColor, const Vec2i &pos, const char *fmt, ...) const PRINTF_VAARG_ATTRIBUTE(4, 5); // Don't forget to count this
	/// Notify player about a problem
	void Notify(const char *fmt, ...) const PRINTF_VAARG_ATTRIBUTE(2, 3); // Don't forget to count this

	/**
	**  Check if the player index is an enemy
	*/
	bool IsEnemy(const int index) const { return (Index != index && (Enemy & (1 << index)) != 0); }

	/**
	**  Check if the player index is an enemy
	*/
	bool IsAllied(const int index) const
	{
		return (Index != index && (Allied & (1 << index)) != 0);
	}

	bool IsEnemy(const CPlayer &player) const;
	bool IsEnemy(const CUnit &unit) const;
	bool IsAllied(const CPlayer &player) const;
	bool IsAllied(const CUnit &unit) const;
	bool IsVisionSharing() const;
	const std::set<uint8_t> &GetSharedVision() const { return this->HasVisionFrom; }
	const std::set<uint8_t> &GetGaveVisionTo() const { return this->GaveVisionTo; }
	bool HasSharedVisionWith(const CPlayer &player) const
	{
		return (this->GaveVisionTo.find(player.Index) != this->GaveVisionTo.end());
	}
	bool IsTeamed(const CPlayer &player) const;
	bool IsTeamed(const CUnit &unit) const;

	void SetDiplomacyNeutralWith(const CPlayer &player);
	void SetDiplomacyAlliedWith(const CPlayer &player);
	void SetDiplomacyEnemyWith(const CPlayer &player);
	void SetDiplomacyCrazyWith(const CPlayer &player);

	void ShareVisionWith(CPlayer &player);
	void EnableSharedVisionFrom(const CPlayer &player);
	void UnshareVisionWith(CPlayer &player);
	void DisableSharedVisionFrom(const CPlayer &player);

	void Init(PlayerTypes type);
	void Save(CFile &file) const;
	void Load(lua_State *l);

	bool IsRevealed() const { return this->isRevealed; }
	void SetRevealed(const bool revealed);

public:
	int Index = 0;      /// player as number
	std::string Name;   /// name of non computer

	PlayerTypes Type;   /// type of player (human,computer,...)
	int Race = 0;         /// race of player (orc,human,...)
	std::string AiName; /// AI for computer

	// friend enemy detection
	int Team = 0;          /// team of player

	Vec2i StartPos{-1, -1}; /// map tile start position

	void SetStartView(const Vec2i &pos) { StartPos = pos; }

	int Resources[MaxCosts]{};       /// resources in overall store
	int MaxResources[MaxCosts]{};    /// max resources can be stored
	int StoredResources[MaxCosts]{}; /// resources in store buildings (can't exceed MaxResources)
	int LastResources[MaxCosts]{};   /// last values for revenue
	int Incomes[MaxCosts]{};         /// income of the resources
	int Revenue[MaxCosts]{};         /// income rate of the resources

	int SpeedResourcesHarvest[MaxCosts]{}; /// speed factor for harvesting resources
	int SpeedResourcesReturn[MaxCosts]{};  /// speed factor for returning resources
	int SpeedBuild = 0;                  /// speed factor for building
	int SpeedTrain = 0;                  /// speed factor for training
	int SpeedUpgrade = 0;                /// speed factor for upgrading
	int SpeedResearch = 0;               /// speed factor for researching

	// FIXME: shouldn't use the constant
	int UnitTypesCount[UnitTypeMax]{}; /// total units of unit-type
	int UnitTypesAiActiveCount [UnitTypeMax]{}; /// total units of unit-type that have their AI set to active

	bool AiEnabled = false;        /// handle AI on local computer
	std::unique_ptr<PlayerAi> Ai; /// Ai structure pointer

	int NumBuildings = 0;   /// # buildings
	int Supply = 0;         /// supply available/produced
	int Demand = 0;         /// demand of player

	int UnitLimit = 0;       /// # food units allowed
	int BuildingLimit = 0;   /// # buildings allowed
	int TotalUnitLimit = 0;  /// # total unit number allowed

	int Score = 0;           /// Points for killing ...
	int TotalUnits = 0;
	int TotalBuildings = 0;
	int TotalResources[MaxCosts]{};
	int TotalRazings = 0;
	int TotalKills = 0;      /// How many unit killed

	int LostMainFacilityTimer { 0 };/// The timer for when the player lost the last town hall
									/// (to make the player's units be revealed)

	IntColor Color;           /// color of units on minimap

	// Upgrades/Allows:
	CAllow Allow;                 /// Allowed for player
	CUpgradeTimers UpgradeTimers; /// Timer for the upgrades

private:
	CUnitColors UnitColors;            /// Unit colors for new units
	std::vector<CUnit *> Units;        /// units of this player
	std::vector<CUnit *> FreeWorkers;  /// Container for free workers
	unsigned int Enemy = 0;            /// enemy bit field for this player
	unsigned int Allied = 0;           /// allied bit field for this player
	std::set<uint8_t> HasVisionFrom;   /// set of player indexes that have shared their vision with this player
	std::set<uint8_t> GaveVisionTo;    /// set of player indexes that this player has shared vision with

	bool isRevealed { false }; 	/// whether the player has been revealed (i.e. after losing the last Town Hall)

private:
	/// List of players revealed after losing their last Town Hall
	static inline std::vector<const CPlayer *> RevealedPlayers;
};

/**
**  Races for the player
**  Mapped with #PlayerRaces to a symbolic name.
*/
class PlayerRace
{
public:
	PlayerRace() = default;

	void Clean();
	int GetRaceIndexByName(std::string_view raceName) const;

public:
	bool Visible[MAX_RACES]{};      /// race should be visible in pulldown
	std::string Name[MAX_RACES];    /// race names
	std::string Display[MAX_RACES]; /// text to display in pulldown
	unsigned int Count = 0;         /// number of races
};

/*----------------------------------------------------------------------------
--  Variables
----------------------------------------------------------------------------*/

extern int NumPlayers;             /// How many player slots used
extern CPlayer Players[PlayerMax];  /// All players
extern CPlayer *ThisPlayer;         /// Player on local computer
extern bool NoRescueCheck;          /// Disable rescue check
extern std::vector<std::vector<CColor>> PlayerColorsRGB; /// Player colors
extern std::vector<std::vector<SDL_Color>> PlayerColorsSDL; /// Player colors
extern std::vector<std::string> PlayerColorNames;  /// Player color names

extern PlayerRace PlayerRaces;  /// Player races

/**
**  Which indexes to replace with player color
*/
extern int PlayerColorIndexStart;
extern int PlayerColorIndexCount;

/*----------------------------------------------------------------------------
--  Functions
----------------------------------------------------------------------------*/

/// Init players
extern void InitPlayers();
/// Clean up players
extern void CleanPlayers();
/// Save players
extern void SavePlayers(CFile &file);

/// Create a new player
extern void CreatePlayer(PlayerTypes type);


/// Initialize the computer opponent AI
extern void PlayersInitAi();
/// Called each game cycle for player handlers (AI)
extern void PlayersEachCycle();
/// Called each second for a given player handler (AI)
extern void PlayersEachSecond(int player);

/// Change current color set to the player color of the sprite
extern void GraphicPlayerPixels(int colorIndex, const CGraphic &sprite);

/// Output debug information for players
extern void DebugPlayers();

void FreePlayerColors();


CPlayer *CclGetPlayer(lua_State *l);

/// register ccl features
extern void PlayerCclRegister();

/// Allowed to select multiple units, maybe not mine
inline bool CanSelectMultipleUnits(const CPlayer &player) { return &player == ThisPlayer || ThisPlayer->IsTeamed(player); }

//@}

#endif // !__PLAYER_H__
