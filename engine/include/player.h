//     ____                _       __               
//    / __ )____  _____   | |     / /___ ___________
//   / __  / __ \/ ___/   | | /| / / __ `/ ___/ ___/
//  / /_/ / /_/ (__  )    | |/ |/ / /_/ / /  (__  ) 
// /_____/\____/____/     |__/|__/\__,_/_/  /____/  
//                                              
//       A futuristic real-time strategy game.
//          This file is part of Bos Wars.
//
/**@name player.h - The player headerfile. */
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
**  CPlayer::Index
**
**    This is the unique slot number. It is not possible that two
**    players have the same slot number at the same time. The slot
**    numbers are reused in the future. This means if a player is
**    defeated, a new player can join using this slot. Currently
**    #PlayerMax players are supported. This member is used to
**    access bit fields.
**    Slot #PlayerNumNeutral is reserved for the neutral units
**    like gold-mines or critters.
**
**    @note Should call this member Slot?
**
**  CPlayer::Name
**
**    Name of the player, encoded in UTF-8.
**    In local games, the name is used for displays, and there is no
**    strict restriction on the length, although there is limited
**    space on the screen.
**    In network games, the name is also used for chatting,
**    and it is restricted to NetPlayerNameSize bytes, including
**    the final zero.
**
**  CPlayer::Type
**
**    Type of the player. This field is setup from the level (map).
**    We support currently #PlayerNeutral,
**    #PlayerNobody, #PlayerComputer, #PlayerPerson,
**    #PlayerRescuePassive and #PlayerRescueActive.
**    @see #PlayerTypes.
**
**  CPlayer::AiName
**
**    AI name for computer. This field is setup
**    from the map. Used to select the AI for the computer
**    player. Might differ from CAiType::Name of the AI
**    that was actually selected.
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
**    If CPlayer::Enemy & (1<<CPlayer::Index) != 0 its an enemy.
**    Setup during startup using the CPlayer::Team, can later be
**    changed with diplomacy. CPlayer::Enemy and CPlayer::Allied
**    are combined, if none bit is set, the player is neutral.
**    @note You can be allied to a player, which sees you as enemy.
**
**  CPlayer::Allied
**
**    A bit field which contains the allies of this player.
**    If CPlayer::Allied & (1<<CPlayer::Index) != 0 its an allied.
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
**    They control which part of the map is initially scrolled to the view,
**    and in some game types (e.g. ::SettingsGameTypeTopVsBottom),
**    they also control which players are initially allied.
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
**  CPlayer::NumBuildings
**
**    Total number of buildings in the CPlayer::Units table.
**
**  CPlayer::UnitLimit
**
**    Number of non-building units allowed.
**    When CPlayer::TotalNumUnit - CPlayer::NumBuildings >= CPlayer::UnitLimit,
**    the player cannot train any more non-building units;
**    but may still be able to build buildings.
**    @note that all limits are always checked.
**
**  CPlayer::BuildingLimit
**
**    Number of buildings allowed.
**    When CPlayer::NumBuildings >= CPlayer::BuildingLimit,
**    the player cannot build any more buildings;
**    but may still be able to train other units.
**    @note that all limits are always checked.
**
**  CPlayer::TotalUnitLimit
**
**    Number of total units allowed.
**    When CPlayer::TotalNumUnits >= CPlayer::TotalUnitLimit,
**    the player cannot train or build any more units of any kind.
**    @note that all limits are always checked.
**
**  CPlayer::Score
**
**    Total number of points. You can get points for killing units,
**    destroying buildings ...
**
**  CPlayer::TotalUnits
**
**    Total number of non-building units made or captured
**    by the player, including any that have since been killed.
**    This counter never decreases during a game.
**
**  CPlayer::TotalBuildings
**
**    Total number of buildings made or captured by the player,
**    including any that have since been razed.
**    This counter never decreases during a game.
**
**  CPlayer::TotalResources[::MaxCosts]
**
**    Total number of resources collected.
**    This counter never decreases during a game, unless some
**    unit type has been strangely configured with a negative
**    CUnitType::ProductionRate.
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
**    Contains which unit-types are allowed for the
**    player. Possible values are:
**    @li  `A' -- allowed,
**    @li  `F' -- forbidden,
**    @li  `R' -- acquired, perhaps other values
**    @li  `Q' -- acquired but forbidden (does it make sense?:))
**    @li  `E' -- enabled, allowed by level but currently forbidden
**    @see CAllow
*/

/*----------------------------------------------------------------------------
--  Includes
----------------------------------------------------------------------------*/

#include <map>
#include <string>
#include "upgrade_structs.h"
#include "video.h"

/*----------------------------------------------------------------------------
--  Declarations
----------------------------------------------------------------------------*/

typedef int lua_Object; // from tolua++.h
struct lua_State;

class CUnit;
class CUnitType;
class PlayerAi;
class CFile;

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
	PlayerRescueActive = 7,   /// rescued  active
};

#define PlayerNumNeutral (PlayerMax - 1)  /// this is the neutral player slot

/**
**  Notify types. Noties are send to the player.
*/
enum NotifyType {
	NotifyRed,     /// Red alram
	NotifyYellow,  /// Yellow alarm
	NotifyGreen,   /// Green alarm
};

/*----------------------------------------------------------------------------
--  Player type
----------------------------------------------------------------------------*/

	///  Player structure
class CPlayer
{
public:
	int Index;          /// player as number
	std::string Name;   /// name of non computer

	PlayerTypes Type;   /// type of player (human,computer,...)
	std::string AiName; /// AI for computer

	// friend enemy detection
	int      Team;          /// team of player
	unsigned Enemy;         /// enemy bit field for this player
	unsigned Allied;        /// allied bit field for this player
	unsigned SharedVision;  /// shared vision bit field

	int StartX;  /// map tile start X position
	int StartY;  /// map tile start Y position
	inline void SetStartView(int x, int y) { StartX = x; StartY = y; }

	std::map<CUnit *, int *> UnitsConsumingResourcesActual;
	std::map<CUnit *, int *> UnitsConsumingResourcesRequested;
	int ProductionRate[MaxCosts];           /// Rate that resources are produced
	int ActualUtilizationRate[MaxCosts];    /// Rate that resources are used
	int RequestedUtilizationRate[MaxCosts]; /// Rate that resources are used
	int StoredResources[MaxCosts];          /// Amount of resources in storage
	int StorageCapacity[MaxCosts];          /// Storage capacity of resources

	inline void SetEnergyProductionRate(int v) { ProductionRate[EnergyCost] = v; }
	inline int GetEnergyProductionRate() { return ProductionRate[EnergyCost]; }
	inline void SetMagmaProductionRate(int v) { ProductionRate[MagmaCost] = v; }
	inline int GetMagmaProductionRate() { return ProductionRate[MagmaCost]; }

	inline void SetEnergyStored(int v) { StoredResources[EnergyCost] = CYCLES_PER_SECOND * v; }
	inline int GetEnergyStored() { return StoredResources[EnergyCost] / CYCLES_PER_SECOND; }
	inline void SetMagmaStored(int v) { StoredResources[MagmaCost] = CYCLES_PER_SECOND * v; }
	inline int GetMagmaStored() { return StoredResources[MagmaCost] / CYCLES_PER_SECOND; }

	inline void SetEnergyStorageCapacity(int v) { StorageCapacity[EnergyCost] = CYCLES_PER_SECOND * v; }
	inline int GetEnergyStorageCapacity() { return StorageCapacity[EnergyCost] / CYCLES_PER_SECOND; }
	inline void SetMagmaStorageCapacity(int v) { StorageCapacity[MagmaCost] = CYCLES_PER_SECOND * v; }
	inline int GetMagmaStorageCapacity() { return StorageCapacity[MagmaCost] / CYCLES_PER_SECOND; }

	void AddToUnitsConsumingResources(CUnit *unit, int costs[MaxCosts]);
	void RemoveFromUnitsConsumingResources(CUnit *unit);
	void UpdateUnitsConsumingResources(CUnit *unit, int costs[MaxCosts]);
	void RebuildUnitsConsumingResourcesList();
	void ClearResourceVariables();

	// FIXME: shouldn't use the constant
	int UnitTypesCount[UnitTypeMax];  /// total units of unit-type

	int   AiEnabled;       /// handle AI on local computer
	PlayerAi *Ai;          /// Ai structure pointer

	CUnit *Units[UnitMax]; /// units of this player
	int    TotalNumUnits;  /// total # units for units' list
	int    NumBuildings;   /// # buildings

	int    UnitLimit;       /// # food units allowed
	int    BuildingLimit;   /// # buildings allowed
	int    TotalUnitLimit;  /// # total unit number allowed

	int    Score;           /// Points for killing ...
	int    TotalUnits;
	int    TotalBuildings;
	int    TotalResources[MaxCosts];
	int    TotalRazings;
	int    TotalKills;      /// How many unit killed

	inline void SetTotalEnergy(int v) { TotalResources[EnergyCost] = CYCLES_PER_SECOND * v; }
	inline int GetTotalEnergy() { return TotalResources[EnergyCost] / CYCLES_PER_SECOND; }
	inline void SetTotalMagma(int v) { TotalResources[MagmaCost] = CYCLES_PER_SECOND * v; }
	inline int GetTotalMagma() { return TotalResources[MagmaCost] / CYCLES_PER_SECOND; }

	Uint32 Color;           /// color of units on minimap

	CUnitColors UnitColors; /// Unit colors for new units

	// Allows:
	CAllow Allow;           /// Allowed for player


	/// Change player name
	void SetName(const std::string &name);

	/// Clear turn related player data
	void Clear();

	/// Set a resource of the player
	void SetResource(int resource, int value);

	/// Check if the unit-type didn't break any unit limits and supply/demand
	int CheckLimits(const CUnitType *type) const;

	/// Does the player have units of that type
	int HaveUnitTypeByType(const CUnitType *type) const;
	/// Does the player have units of that type
	int HaveUnitTypeByIdent(const std::string &ident) const;

	/// Notify player about a problem
	void Notify(int type, int x, int y, const char *fmt, ...) const;

	bool IsEnemy(const CPlayer *x) const;
	bool IsEnemy(const CUnit *x) const;
	bool IsAllied(const CPlayer *x) const;
	bool IsAllied(const CUnit *x) const;
	bool IsSharedVision(const CPlayer *x) const;
	bool IsSharedVision(const CUnit *x) const;
	bool IsBothSharedVision(const CPlayer *x) const;
	bool IsBothSharedVision(const CUnit *x) const;
	bool IsTeamed(const CPlayer *x) const;
	bool IsTeamed(const CUnit *x) const;
};

/*----------------------------------------------------------------------------
--  Variables
----------------------------------------------------------------------------*/

extern int NumPlayers;             /// How many player slots used
extern CPlayer Players[PlayerMax];  /// All players
extern CPlayer *ThisPlayer;         /// Player on local computer
extern int NoRescueCheck;          /// Disable rescue check
extern SDL_Color *PlayerColorsRGB[PlayerMax]; /// Player colors
extern Uint32 *PlayerColors[PlayerMax];       /// Player colors
extern std::string PlayerColorNames[PlayerMax];  /// Player color names

/**
**  Which indexes to replace with player color
*/
extern int PlayerColorIndexStart;
extern int PlayerColorIndexCount;

/*----------------------------------------------------------------------------
--  Functions
----------------------------------------------------------------------------*/

	/// Init players
extern void InitPlayers(void);
	/// Clean up players
extern void CleanPlayers(void);
#ifdef DEBUG
extern void FreePlayerColors();
#endif
	/// Save players
extern void SavePlayers(CFile *file);

	/// Create a new player
extern void CreatePlayer(PlayerTypes type);


	/// Initialize the computer opponent AI
extern void PlayersInitAi(void);
	/// Called each game cycle for player handlers (AI)
extern void PlayersEachCycle(void);
	/// Called each second for a given player handler (AI)
extern void PlayersEachSecond(int player);

	/// Change current color set to new player of the sprite
extern void GraphicPlayerPixels(CPlayer *player, const CGraphic *sprite);

	/// Calculate how many resources the unit needs to request
void CalculateRequestedAmount(CUnitType *utype, int bcosts[MaxCosts], int costs[MaxCosts]);

	/// Output debug informations for players
extern void DebugPlayers(void);

	/// register ccl features
extern void PlayerCclRegister(void);

extern void CclChangeUnitsOwner(
	const int topLeft[2],
	const int bottomRight[2],
	int oldPlayer,
	int newPlayer,
	lua_Object unitTypeLua,
	lua_State *l);

	/// Allowed to select multiple units, maybe not mine
#define CanSelectMultipleUnits(player) \
	((player) == ThisPlayer || ThisPlayer->IsTeamed((player)))

//@}

#endif // !__PLAYER_H__
