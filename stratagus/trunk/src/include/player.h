//   ___________		     _________		      _____  __
//   \_	  _____/______   ____   ____ \_   ___ \____________ _/ ____\/  |_
//    |    __) \_  __ \_/ __ \_/ __ \/    \  \/\_  __ \__  \\   __\\   __|
//    |     \   |  | \/\  ___/\  ___/\     \____|  | \// __ \|  |   |  |
//    \___  /   |__|    \___  >\___  >\______  /|__|  (____  /__|   |__|
//	  \/		    \/	   \/	     \/		   \/
//  ______________________                           ______________________
//			  T H E   W A R   B E G I N S
//	   FreeCraft - A free fantasy real time strategy game engine
//
/**@name player.h	-	The player headerfile. */
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

#ifndef __PLAYER_H__
#define __PLAYER_H__

//@{

/*----------------------------------------------------------------------------
--	Documentation
----------------------------------------------------------------------------*/

/**
**	@struct _player_ player.h
**
**	\#include "player.h"
**
**	typedef struct _player_ Player;
**
**	This structure contains all informations about a player in game.
**
**	The player structure members:
**
**	Player::Player
**
**		This is the unique slot number. It is not possible that two
**		players have the same slot number at the same time. The slot
**		numbers are reused in the future. This means if a player is
**		defeated, a new player can join using this slot. Currently
**		#PlayerMax (16) players are supported. This member is used to
**		access bit fields.
**		Slot #PlayerNumNeutral (15) is reserved for the neutral units
**		like gold-mines or critters.
**
**		@note Should call this member Slot?
**
**	Player::Name
**
**		Name of the player used for displays and network game.
**		It is restricted to 15 characters plus final zero.
**
**	Player::Type
**
**		Type of the player. This field is setup from the level (PUD).
**		We support currently #PlayerNeutral,
**		#PlayerNobody, #PlayerComputer, #PlayerPerson,
**		#PlayerRescuePassive and #PlayerRescueActive.
**		@see #PlayerTypes.
**
**	Player::RaceName
**
**		Name of the race to which the player belongs, used to select
**		the user interface and the AI.
**		We have 'orc', 'human', 'alliance' or 'mythical'. Should
**		only be used during configuration and not during runtime.
**
**	Player::Race
**
**		Race number of the player. This field is setup from the level
**		(PUD). This number is mapped with #RaceWcNames to the symbolic
**		name Player::RaceName.
**		@see #PlayerRaces.
**
**	Player::AiNum
**
**		AI number for computer (See #PlayerAis). This field is setup
**		from the level (PUD). Used to select the AI for the computer
**		player.
**
**	Player::Team
**
**		Team of player. Selected during network game setup. All players
**		of the same team are allied and enemy to all other teams.
**		@note It is planned to show the team on the map.
**
**	Player::Enemy
**
**		A bit field which contains the enemies of this player.
**		If Player::Enemy & (1<<Player::Player) != 0 its an enemy.
**		Setup during startup using the Player::Team, can later be
**		changed with diplomacy. Player::Enemy and Player::Allied
**		are combined, if none bit is set, the player is neutral.
**		@note You can be allied to a player, which sees you as enemy.
**
**	Player::Allied
**
**		A bit field which contains the allies of this player.
**		If Player::Allied & (1<<Player::Player) != 0 its an allied.
**		Setup during startup using the Player:Team, can later be
**		changed with diplomacy. Player::Enemy and Player::Allied
**		are combined, if none bit is set, the player is neutral.
**		@note You can be allied to a player, which sees you as enemy.
**
**	Player::StartX Player::StartY
**
**		The tile map coordinates of the player start position. 0,0 is
**		the upper left on the map. This members are setup from the
**		level (PUD) and only important for the game start.
**		Ignored if game starts with level settings. Used to place
**		the initial workers if you play with 1 or 3 workers.
**
**	Player::Resources[::MaxCosts]
**
**		How many resources the player owns. Needed for building
**		units and structures.
**		@see _costs_, TimeCost, GoldCost, WoodCost, OilCost, MaxCosts.
**
**	Player::Incomes[::MaxCosts]
**
**		Income of the resources, when they are delivered at a store.
**		@see _costs_, TimeCost, GoldCost, WoodCost, OilCost, MaxCosts.
**
**	Player::LastResources[::MaxCosts]
**
**		Keeps track of resources in time (used for calculating
**		Player::Revenue, see below)
**
**	Player::Revenue[::MaxCosts]
**
**		Production of resources per minute (or estimates)
**              Used just as information (statistics) for the player...
**
**	Player::UnitTypesCount[::UnitTypeMax]
**
**		Total count for each different unit type. Used by the AI and
**		for dependencies checks. The addition of all counts should
**		be Player::TotalNumUnits.
**		@note Should not use the maximum number of unit-types here,
**		only the real number of unit-types used.
**
**	Player::AiEnabled
**
**		If the player is controlled by the computer and this flag is
**		true, than the player is handled by the AI on this local
**		computer.
**
**		@note Currently the AI is calculated parallel on all computers
**		in a network play. It is planned to change this.
**
**	Player::Ai
**
**		AI structure pointer. Please look at #PlayerAi for more
**		informations.
**
**	Player::Units
**
**		A table of all (Player::TotalNumUnits) units of the player.
**
**	Player::TotalNumUnits
**
**		Total number of units (incl. buildings) in the Player::Units
**		table.
**
**	Player::NumFoodUnits
**
**		Total number of units that need food, used to check food limit.
**		A player can only build up to Player::Food units and not more
**		than Player::FoodUnitLimit units.
**
**		@note that Player::NumFoodUnits > Player::Food, when enough
**		farms are destroyed.
**
**	Player::NumBuildings
**
**		Total number buildings, units that don't need food.
**
**	Player::Food
**
**		Number of food available/produced. Player can't train more
**		Player::NumFoodUnits than this.
**		@note that all limits are always checked.
**
**	Player::FoodUnitLimit
**
**		Number of food units allowed. Player can't train more
**		Player::NumFoodUnits than this.
**		@note that all limits are always checked.
**
**	Player::BuildingLimit
**
**		Number of buildings allowed.  Player can't build more
**		Player::NumBuildings than this.
**		@note that all limits are always checked.
**
**	Player::TotalUnitLimit
**
**		Number of total units allowed. Player can't have more
**		Player::NumFoodUnits+Player::NumBuildings=Player::TotalNumUnits
**		this.
**		@note that all limits are always checked.
**
**	Player::LastRepairResource
**
**		Last resource used for repair cycles. Was introduced to
**		reduce the repair speed.  @see RepairUnit.
**
**	Player::Score
**
**		Total number of points. You can get points for killing units,
**		destroying buildings ...
**
**	Player::TotalUnits
**
**		Total number of units made.
**
**	Player::TotalBuildings
**
**		Total number of buildings made.
**
**	Player::TotalResources[::MaxCosts]
**
**		Total number of resources collected.
**		@see _costs_, TimeCost, GoldCost, WoodCost, OilCost, MaxCosts.
**
**	Player::TotalRazings
**
**		Total number of buildings destroyed.
**
**	Player::TotalKills
**
**		Total number of kills.
**
**	Player::Color
**
**		Color of units of this player on the minimap. Index number
**		into the global palette.
**
**	Player::UnitColors
**
**		Unit colors of this player. Contains the hardware dependent
**		pixel values for the player colors (palette index #208-#211).
**		Setup from the global palette.
**		@note Index #208-#211 are various SHADES of the team color
**		(#208 is brightest shade, #211 is darkest shade) .... these
**		numbers are NOT red=#208, blue=#209, etc
**
**	Player::Allow
**
**		Contains which unit-types and upgrades are allowed for the
**		player. Possible values are:
**		@li	`A' -- allowed,
**		@li	`F' -- forbidden,
**		@li	`R' -- acquired, perhaps other values
**		@li	`Q' -- acquired but forbidden (does it make sense?:))
**		@li	`E' -- enabled, allowed by level but currently forbidden
**		@see _allow_
**
**	Player::UpgradeTimers
**
**		Timer for the upgrades. One timer for all possible upgrades.
**		Initial 0 counted up by the upgrade action, until it reaches
**		the upgrade time.
**		@see _upgrade_timers_
**		@note it is planned to combine research for faster upgrades.
*/

/*----------------------------------------------------------------------------
--	Includes
----------------------------------------------------------------------------*/

#include "upgrade_structs.h"
#include "unittype.h"
#include "unit.h"
#include "video.h"

/*----------------------------------------------------------------------------
--	Player type
----------------------------------------------------------------------------*/

#ifndef __STRUCT_PLAYER__
#define __STRUCT_PLAYER__		/// protect duplicate player typedef
typedef struct _player_ Player;		/// player typedef
#endif

    ///	Player structure
struct _player_ {
    int		Player;			/// player as number
    char*	Name;			/// name of non computer

    int		Type;			/// type of player (human,computer,...)
    char*	RaceName;		/// name of race
    int		Race;			/// race of player (orc,human,...)
    int		AiNum;			/// AI for computer

    // friend enemy detection
    int		Team;			/// team of player
    unsigned	Enemy;			/// enemy bit field for this player
    unsigned	Allied;			/// allied bit field for this player
    unsigned	SharedVision;		/// shared vision bit field

    int		StartX;			/// map tile start X position
    int		StartY;			/// map tile start Y position

    int		Resources[MaxCosts];	/// resources in store
    int		LastResources[MaxCosts];	/// last values for revenue
    int		Incomes[MaxCosts];	/// income of the resources
    int		Revenue[MaxCosts];	/// income rate of the resources

//  FIXME: needed again? if not remove
//    unsigned	UnitFlags[
//	(UnitTypeMax+BitsOf(unsigned)-1)
//	    /BitsOf(unsigned)];		/// flags what units are available
    // FIXME: shouldn't use the constant
    int		UnitTypesCount[UnitTypeMax];	/// total units of unit-type

    int		AiEnabled;		/// handle AI on local computer
    void*	Ai;			/// Ai structure pointer

    Unit**	Units;			/// units of this player
    int		TotalNumUnits;		/// total # units for units' list

    int		NumFoodUnits;		/// # units (need food)
    int		NumBuildings;		/// # buildings (don't need food)

    int		Food;			/// food available/produced
    int		FoodUnitLimit;		/// # food units allowed
    int		BuildingLimit;		/// # buildings allowed
    int		TotalUnitLimit;		/// # total unit number allowed
    int		LastRepairResource;	/// last resource for repair cycles

    int		Score;			/// Points for killing ...
    int		TotalUnits;
    int		TotalBuildings;
    int		TotalResources[MaxCosts];
    int		TotalRazings;
    int		TotalKills;		/// How many unit killed

// Display video
    unsigned	Color;			/// color of units on minimap

    UnitColors	UnitColors;		/// Unit colors for new units

//  Upgrades/Allows:
    Allow		Allow;		/// Allowed for player
    UpgradeTimers	UpgradeTimers;	/// Timer for the upgrades
};

/**
**	Races for the player (must fit to PUD!)
**	Mapped with #RaceWcNames to a symbolic name, which will be used in
**	the future.
**
**	@note FIXME: This and the use MUST be removed to allow more races.
*/
enum PlayerRaces {
    PlayerRaceHuman	=0,		/// belongs to human
    PlayerRaceOrc	=1,		/// belongs to orc
    PlayerRaceNeutral	=2,		/// belongs to none

    PlayerMaxRaces	=2		/// maximal races supported
};

/**
**	Types for the player (must fit to PUD!)
**
**	#PlayerNeutral
**
**		This player is controlled by the computer doing nothing.
**
**	#PlayerNobody
**
**		This player is unused. Nobody controlls this player.
**
**	#PlayerComputer
**
**		This player is controlled by the computer. Player::AiNum
**		selects the AI strategy.
**
**	#PlayerPerson
**
**		This player is contolled by a person. This can be the player
**		sitting on the local computer or player playing over the
**		network.
**
**	#PlayerRescuePassive
**
**		This player does nothing, the game pieces just sit in the game
**		(being passive)... when a person player moves next to a
**		PassiveRescue unit/building, then it is "rescued" and becomes
**		part of that persons team. If the city center is rescued, than
**		all units of this player are rescued.
**
**	#PlayerRescueActive
**
**		This player is controlled by the computer. Player::AiNum
**		selects the AI strategy. Until it is rescued it plays like
**		an ally. The first person which reaches units of this player,
**		can rescue them. If the city center is rescued, than all units
**		of this player are rescued.
*/
enum PlayerTypes {
    PlayerNeutral	=2,		/// neutral
    PlayerNobody	=3,		/// unused slot
    PlayerComputer	=4,		/// computer player
    PlayerPerson	=5,		/// human player
    PlayerRescuePassive	=6,		/// rescued passive
    PlayerRescueActive	=7,		/// rescued  active
};

/**
**	Ai types for the player (must fit to PUD!)
**
**	Mapped with #AiTypeWcNames to internal symbols.
*/
enum PlayerAis {
    PlayerAiLand	=0x00,		/// attack at land
    PlayerAiPassive	=0x01,		/// passive does nothing
    PlayerAiSea		=0x19,		/// attack at sea
    PlayerAiAir		=0x1A,		/// attack at air
    PlayerAiUniversal	=0xFF,		/// attack best possible
};

#define PlayerNumNeutral	15	/// this is the neutral player slot
//#define PlayerMax		16	/// maximal players supported

/**
**	Notify types. Noties are send to the player.
*/
enum _notify_type_ {
    NotifyRed,				/// Red alram
    NotifyYellow,			/// Yellow alarm
    NotifyGreen,			/// Green alarm
};

/*----------------------------------------------------------------------------
--	Variables
----------------------------------------------------------------------------*/

extern int NumPlayers;			/// How many player slots used
extern Player Players[PlayerMax];	/// All players
extern Player* ThisPlayer;		/// Player on local computer
extern int NoRescueCheck;		/// Disable rescue check
extern int PlayerColors[PlayerMax];	/// Player colors

extern char** RaceWcNames;		/// pud original -> internal

/*----------------------------------------------------------------------------
--	Functions
----------------------------------------------------------------------------*/

    /// Init players
extern void InitPlayers(void);
    /// Clean up players
extern void CleanPlayers(void);
    /// Save players
extern void SavePlayers(FILE*);

    /// Create a new player
extern void CreatePlayer(int type);

    /// Change player side
extern void PlayerSetSide(Player* player,int side);
    /// Change player name
extern void PlayerSetName(Player* player,const char* name);
    /// Change player AI
extern void PlayerSetAiNum(Player* player,int ai);

    /// Set a resource of the player
extern void PlayerSetResource(Player* player,int resource,int value);

    /// Check if the unit-type didn't break any unit limits
extern int PlayerCheckLimits(const Player* player,const UnitType* type);

    /// Check if enough food is available for unit-type
extern int PlayerCheckFood(const Player* player,const UnitType* type);

    /// Check if enough resources are available for costs
extern int PlayerCheckCosts(const Player* player,const int* costs);
    /// Check if enough resources are available for a new unit-type
extern int PlayerCheckUnitType(const Player* player,const UnitType* type);

    /// Add costs to the resources
extern void PlayerAddCosts(Player* player,const int* costs);
    /// Add costs for an unit-type to the resources
extern void PlayerAddUnitType(Player* player,const UnitType* type);
    /// Add a factor of costs to the resources
extern void PlayerAddCostsFactor(Player* player,const int* costs,int factor);
    /// Remove costs from the resources
extern void PlayerSubCosts(Player* player,const int* costs);
    /// Remove costs for an unit-type from the resources
extern void PlayerSubUnitType(Player* player,const UnitType* type);
    /// Remove a factor of costs from the resources
extern void PlayerSubCostsFactor(Player* player,const int* costs,int factor);

    /// Has the player units of that type
extern int HaveUnitTypeByType(const Player* player,const UnitType* type);
    /// Has the player units of that type
extern int HaveUnitTypeByIdent(const Player* player,const char* ident);

    /// Initialize the computer opponent AI
extern void PlayersInitAi(void);
    /// Called each game cycle for player handlers (AI)
extern void PlayersEachCycle(void);
    /// Called each second for player handlers (AI)
extern void PlayersEachSecond(void);

    /// Change current color set to new player
extern void PlayerPixels(const Player* player);

    /// Change current color set to new player of the sprite
extern void GraphicPlayerPixels(const Player* player, const Graphic * sprite);

    /// Output debug informations for players
extern void DebugPlayers(void);

    /// Notify player about a problem
extern void NotifyPlayer(const Player*,int,int,int,const char*,...);

    /// register ccl features
extern void PlayerCclRegister(void);

//@}

#endif // !__PLAYER_H__
