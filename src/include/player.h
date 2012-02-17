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
--  Documentation
----------------------------------------------------------------------------*/

/**
**  @class CPlayer player.h
**
**  \#include "player.h"
**
**  This structure contains all informations about a player in game.
**
**  The player structure members:
**
**  CPlayer::Player
**
**    This is the unique slot number. It is not possible that two
**    players have the same slot number at the same time. The slot
**    numbers are reused in the future. This means if a player is
**    defeated, a new player can join using this slot. Currently
**    #PlayerMax (16) players are supported. This member is used to
**    access bit fields.
**    Slot #PlayerNumNeutral (15) is reserved for the neutral units
**    like gold-mines or critters.
**
**    @note Should call this member Slot?
**
**  CPlayer::Name
**
**    Name of the player used for displays and network game.
**    It is restricted to 15 characters plus final zero.
**
**  CPlayer::Type
**
**    Type of the player. This field is setup from the level (map).
**    We support currently #PlayerNeutral,
**    #PlayerNobody, #PlayerComputer, #PlayerPerson,
**    #PlayerRescuePassive and #PlayerRescueActive.
**    @see #PlayerTypes.
**
**  CPlayer::RaceName
**
**    Name of the race to which the player belongs, used to select
**    the user interface and the AI.
**    We have 'orc', 'human', 'alliance' or 'mythical'. Should
**    only be used during configuration and not during runtime.
**
**  CPlayer::Race
**
**    Race number of the player. This field is setup from the level
**    map. This number is mapped with #PlayerRaces to the symbolic
**    name CPlayer::RaceName.
**
**  CPlayer::AiName
**
**    AI name for computer. This field is setup
**    from the map. Used to select the AI for the computer
**    player.
**
**  CPlayer::Team
**
**    Team of player. Selected during network game setup. All players
**    of the same team are allied and enemy to all other teams.
**    @note It is planned to show the team on the map.
**
**  CPlayer::Enemy
**
**    A bit field which contains the enemies of this player.
**    If CPlayer::Enemy & (1<<CPlayer::Player) != 0 its an enemy.
**    Setup during startup using the CPlayer::Team, can later be
**    changed with diplomacy. CPlayer::Enemy and CPlayer::Allied
**    are combined, if none bit is set, the player is neutral.
**    @note You can be allied to a player, which sees you as enemy.
**
**  CPlayer::Allied
**
**    A bit field which contains the allies of this player.
**    If CPlayer::Allied & (1<<CPlayer::Player) != 0 its an allied.
**    Setup during startup using the Player:Team, can later be
**    changed with diplomacy. CPlayer::Enemy and CPlayer::Allied
**    are combined, if none bit is set, the player is neutral.
**    @note You can be allied to a player, which sees you as enemy.
**
**  CPlayer::SharedVision
**
**    A bit field which contains shared vision for this player.
**    Shared vision only works when it's activated both ways. Really.
**
**  CPlayer::StartX CPlayer::StartY
**
**    The tile map coordinates of the player start position. 0,0 is
**    the upper left on the map. This members are setup from the
**    map and only important for the game start.
**    Ignored if game starts with level settings. Used to place
**    the initial workers if you play with 1 or 3 workers.
**
**  CPlayer::Resources[::MaxCosts]
**
**    How many resources the player owns. Needed for building
**    units and structures.
**    @see _costs_, TimeCost, GoldCost, WoodCost, OilCost, MaxCosts.
**
**  CPlayer::MaxResources[::MaxCosts]
**
**    How many resources the player can store at the moment. 
**
**  CPlayer::Incomes[::MaxCosts]
**
**    Income of the resources, when they are delivered at a store.
**    @see _costs_, TimeCost, GoldCost, WoodCost, OilCost, MaxCosts.
**
**  CPlayer::LastResources[::MaxCosts]
**
**    Keeps track of resources in time (used for calculating
**    CPlayer::Revenue, see below)
**
**  CPlayer::Revenue[::MaxCosts]
**
**    Production of resources per minute (or estimates)
**    Used just as information (statistics) for the player...
**
**  CPlayer::UnitTypesCount[::UnitTypeMax]
**
**    Total count for each different unit type. Used by the AI and
**    for dependencies checks. The addition of all counts should
**    be CPlayer::TotalNumUnits.
**    @note Should not use the maximum number of unit-types here,
**    only the real number of unit-types used.
**
**  CPlayer::AiEnabled
**
**    If the player is controlled by the computer and this flag is
**    true, than the player is handled by the AI on this local
**    computer.
**
**    @note Currently the AI is calculated parallel on all computers
**    in a network play. It is planned to change this.
**
**  CPlayer::Ai
**
**    AI structure pointer. Please look at #PlayerAi for more
**    informations.
**
**  CPlayer::Units
**
**    A table of all (CPlayer::TotalNumUnits) units of the player.
**
**  CPlayer::TotalNumUnits
**
**    Total number of units (incl. buildings) in the CPlayer::Units
**    table.
**
**  CPlayer::Demand
**
**    Total unit demand, used to demand limit.
**    A player can only build up to CPlayer::Food units and not more
**    than CPlayer::FoodUnitLimit units.
**
**    @note that CPlayer::NumFoodUnits > CPlayer::Food, when enough
**    farms are destroyed.
**
**  CPlayer::NumBuildings
**
**    Total number buildings, units that don't need food.
**
**  CPlayer::Food
**
**    Number of food available/produced. Player can't train more
**    CPlayer::NumFoodUnits than this.
**    @note that all limits are always checked.
**
**  CPlayer::FoodUnitLimit
**
**    Number of food units allowed. Player can't train more
**    CPlayer::NumFoodUnits than this.
**    @note that all limits are always checked.
**
**  CPlayer::BuildingLimit
**
**    Number of buildings allowed.  Player can't build more
**    CPlayer::NumBuildings than this.
**    @note that all limits are always checked.
**
**  CPlayer::TotalUnitLimit
**
**    Number of total units allowed. Player can't have more
**    CPlayer::NumFoodUnits+CPlayer::NumBuildings=CPlayer::TotalNumUnits
**    this.
**    @note that all limits are always checked.
**
**  CPlayer::Score
**
**    Total number of points. You can get points for killing units,
**    destroying buildings ...
**
**  CPlayer::TotalUnits
**
**    Total number of units made.
**
**  CPlayer::TotalBuildings
**
**    Total number of buildings made.
**
**  CPlayer::TotalResources[::MaxCosts]
**
**    Total number of resources collected.
**    @see _costs_, TimeCost, GoldCost, WoodCost, OilCost, MaxCosts.
**
**  CPlayer::TotalRazings
**
**    Total number of buildings destroyed.
**
**  CPlayer::TotalKills
**
**    Total number of kills.
**
**  CPlayer::Color
**
**    Color of units of this player on the minimap. Index number
**    into the global palette.
**
**  CPlayer::UnitColors
**
**    Unit colors of this player. Contains the hardware dependent
**    pixel values for the player colors (palette index #208-#211).
**    Setup from the global palette.
**    @note Index #208-#211 are various SHADES of the team color
**    (#208 is brightest shade, #211 is darkest shade) .... these
**    numbers are NOT red=#208, blue=#209, etc
**
**  CPlayer::Allow
**
**    Contains which unit-types and upgrades are allowed for the
**    player. Possible values are:
**    @li  `A' -- allowed,
**    @li  `F' -- forbidden,
**    @li  `R' -- acquired, perhaps other values
**    @li  `Q' -- acquired but forbidden (does it make sense?:))
**    @li  `E' -- enabled, allowed by level but currently forbidden
**    @see CAllow
**
**  CPlayer::UpgradeTimers
**
**    Timer for the upgrades. One timer for all possible upgrades.
**    Initial 0 counted up by the upgrade action, until it reaches
**    the upgrade time.
**    @see _upgrade_timers_
**    @note it is planned to combine research for faster upgrades.
*/

/*----------------------------------------------------------------------------
--  Includes
----------------------------------------------------------------------------*/

#include <string>
#include "upgrade_structs.h"
#include "video.h"

#ifndef __MAP_TILE_H__
#include "tile.h"
#endif

/*----------------------------------------------------------------------------
--  Declarations
----------------------------------------------------------------------------*/

class CUnit;
class CUnitType;
class PlayerAi;
class CFile;

/*----------------------------------------------------------------------------
--  Player type
----------------------------------------------------------------------------*/

	///  Player structure
class CPlayer
{
public:
	int Index;          /// player as number
	std::string Name;   /// name of non computer

	int   Type;         /// type of player (human,computer,...)
	int   Race;         /// race of player (orc,human,...)
	std::string AiName; /// AI for computer

	// friend enemy detection
	int      Team;          /// team of player
	unsigned int Enemy;         /// enemy bit field for this player
	unsigned int Allied;        /// allied bit field for this player
	unsigned int SharedVision;  /// shared vision bit field

	int StartX;  /// map tile start X position
	int StartY;  /// map tile start Y position
	inline void SetStartView(int x, int y) { StartX = x; StartY = y; }

	int Resources[MaxCosts];      /// resources in store
	int MaxResources[MaxCosts];   /// max resources can be stored
	int LastResources[MaxCosts];  /// last values for revenue
	int Incomes[MaxCosts];        /// income of the resources
	int Revenue[MaxCosts];        /// income rate of the resources

	// FIXME: shouldn't use the constant
	int UnitTypesCount[UnitTypeMax];  /// total units of unit-type

	int   AiEnabled;       /// handle AI on local computer
	PlayerAi *Ai;          /// Ai structure pointer

	CUnit *Units[UnitMax]; /// units of this player
	int    TotalNumUnits;  /// total # units for units' list
	int    NumBuildings;   /// # buildings
	int    Supply;         /// supply available/produced
	int    Demand;         /// demand of player

	int    UnitLimit;       /// # food units allowed
	int    BuildingLimit;   /// # buildings allowed
	int    TotalUnitLimit;  /// # total unit number allowed

	int    Score;           /// Points for killing ...
	int    TotalUnits;
	int    TotalBuildings;
	int    TotalResources[MaxCosts];
	int    TotalRazings;
	int    TotalKills;      /// How many unit killed

	Uint32 Color;           /// color of units on minimap

	CUnitColors UnitColors; /// Unit colors for new units

	// Upgrades/Allows:
	CAllow Allow;                 /// Allowed for player
	CUpgradeTimers UpgradeTimers; /// Timer for the upgrades
	CUnitCache AutoAttackTargets; /// Current know autoattack targets

	/// Change player side
	void SetSide(int side);
	/// Change player name
	void SetName(const std::string &name);

	/// Clear turn related player data
	void Clear();

	/// Set a resource of the player
	void SetResource(int resource, int value);

	/// Check if the unit-type didn't break any unit limits and supply/demand
	int CheckLimits(const CUnitType &type) const;

	/// Check if enough resources are available for costs
	int CheckCosts(const int *costs) const;
	/// Check if enough resources are available for a new unit-type
	int CheckUnitType(const CUnitType &type) const;

	/// Add costs to the resources
	void AddCosts(const int *costs);
	/// Add costs for an unit-type to the resources
	void AddUnitType(const CUnitType &type);
	/// Add a factor of costs to the resources
	void AddCostsFactor(const int *costs, int factor);
	/// Remove costs from the resources
	void SubCosts(const int *costs);
	/// Remove costs for an unit-type from the resources
	void SubUnitType(const CUnitType &type);
	/// Remove a factor of costs from the resources
	void SubCostsFactor(const int *costs, int factor);

	/// Does the player have units of that type
	int HaveUnitTypeByType(const CUnitType &type) const;
	/// Does the player have units of that type
	int HaveUnitTypeByIdent(const std::string &ident) const;

	/// Notify player about a problem
	void Notify(int type, int x, int y, const char *fmt, ...) const;

	/**
	**  Check if the player index is an enemy
	*/
	bool IsEnemy(const int index) const
	{
		return (Index != index && (Enemy & (1 << index)) != 0);
	}

	bool IsEnemy(const CPlayer &player) const;
	bool IsEnemy(const CUnit &unit) const;
	bool IsAllied(const CPlayer &player) const;
	bool IsAllied(const CUnit &unit) const;
	bool IsSharedVision(const CPlayer &player) const;
	bool IsSharedVision(const CUnit &unit) const;
	bool IsBothSharedVision(const CPlayer &player) const;
	bool IsBothSharedVision(const CUnit &unit) const;
	bool IsTeamed(const CPlayer &player) const;
	bool IsTeamed(const CUnit &unit) const;
};

/**
**  Races for the player
**  Mapped with #PlayerRaces to a symbolic name.
*/
#define MAX_RACES 8
class PlayerRace {
public:
	PlayerRace() : Count(0) {
		memset(Visible, 0, sizeof(Visible));
	}

	bool Visible[MAX_RACES];        /// race should be visible in pulldown
	std::string Name[MAX_RACES];     /// race names
	std::string Display[MAX_RACES];  /// text to display in pulldown
	unsigned int   Count;            /// number of races
};


enum PlayerRacesOld {
	PlayerRaceHuman = 0,  /// belongs to human
	PlayerRaceOrc  = 1    /// belongs to orc
};

/**
**  Types for the player
**
**  #PlayerNeutral
**
**    This player is controlled by the computer doing nothing.
**
**  #PlayerNobody
**
**    This player is unused. Nobody controlls this player.
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
enum PlayerTypes {
	PlayerNeutral = 2,        /// neutral
	PlayerNobody  = 3,        /// unused slot
	PlayerComputer = 4,       /// computer player
	PlayerPerson = 5,         /// human player
	PlayerRescuePassive = 6,  /// rescued passive
	PlayerRescueActive = 7    /// rescued  active
};

#define PlayerNumNeutral (PlayerMax - 1)  /// this is the neutral player slot

/**
**  Notify types. Noties are send to the player.
*/
enum NotifyType {
	NotifyRed,     /// Red alram
	NotifyYellow,  /// Yellow alarm
	NotifyGreen    /// Green alarm
};

/*----------------------------------------------------------------------------
--  Variables
----------------------------------------------------------------------------*/

extern int NumPlayers;             /// How many player slots used
extern CPlayer Players[PlayerMax];  /// All players
extern CPlayer *ThisPlayer;         /// Player on local computer
extern bool NoRescueCheck;          /// Disable rescue check
extern SDL_Color *PlayerColorsRGB[PlayerMax]; /// Player colors
extern Uint32 *PlayerColors[PlayerMax];       /// Player colors
extern std::string PlayerColorNames[PlayerMax];  /// Player color names

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
	/// Clean up races
extern void CleanRaces();
	/// Save players
extern void SavePlayers(CFile *file);

	/// Create a new player
extern void CreatePlayer(int type);


	/// Initialize the computer opponent AI
extern void PlayersInitAi();
	/// Called each game cycle for player handlers (AI)
extern void PlayersEachCycle();
	/// Called each second for a given player handler (AI)
extern void PlayersEachSecond(int player);

	/// Change current color set to new player of the sprite
extern void GraphicPlayerPixels(CPlayer &player, const CGraphic *sprite);

	/// Output debug informations for players
extern void DebugPlayers();

#ifdef DEBUG
void FreePlayerColors();
#endif

	/// register ccl features
extern void PlayerCclRegister();

	/// Allowed to select multiple units, maybe not mine
#define CanSelectMultipleUnits(player) \
	(&(player) == ThisPlayer || ThisPlayer->IsTeamed(player))

//@}

#endif // !__PLAYER_H__
