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
/**@name player.h	-	The player headerfile. */
//
//	(c) Copyright 1998-2001 by Lutz Sammer
//
//	FreeCraft is free software; you can redistribute it and/or modify
//	it under the terms of the GNU General Public License as published
//	by the Free Software Foundation; either version 2 of the License,
//	or (at your option) any later version.
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
**	FIXME: not written documentation
*/

/*----------------------------------------------------------------------------
--	Includes
----------------------------------------------------------------------------*/

#ifndef __STRUCT_PLAYER__
#define __STRUCT_PLAYER__
typedef struct _player_ Player;
#endif

#include "upgrade_structs.h"
#include "unittype.h"
#include "unit.h"
#include "video.h"

/*----------------------------------------------------------------------------
--	Player type
----------------------------------------------------------------------------*/

/**
**	Player structure
*/
struct _player_ {
    unsigned	Player;			/// player as number
    char*	Name;			/// name of non computer

    unsigned	Type;			/// type of player (human,computer,...)
    char*	RaceName;		/// name of race.
    unsigned	Race;			/// race of player (orc,human,...)
    unsigned	AiNum;			/// AI for computer

    // friend enemy detection
    unsigned	Team;			/// team of player
    unsigned	Enemy;			/// enemy bit field for this player
    unsigned	Allied;			/// allied bit field for this player

    unsigned	X;			/// map tile start X position
    unsigned	Y;			/// map tile start Y position

    unsigned	Resources[MaxCosts];	/// resources in store
    int		Incomes[MaxCosts];	/// income of the resources.

//  FIXME: needed again? if not remove
//    unsigned	UnitFlags[
//	(UnitTypeMax+BitsOf(unsigned)-1)
//	    /BitsOf(unsigned)];		/// flags what units are available
    // FIXME: shouldn't use the constant
    unsigned    UnitTypesCount[UnitTypeMax];       /// each type unit count

    unsigned	AiEnabled;		/// handle ai on this computer
    void*	Ai;			/// Ai structure pointer

    Unit**	Units;			/// units of this player
    unsigned	TotalNumUnits;		/// total # units for units' list.

    unsigned	NumFoodUnits;		/// # units (need food)
    unsigned	NumBuildings;		/// # buildings (don't need food)

    unsigned	Food;			/// food available/produced
    unsigned	FoodUnitLimit;		/// # food units allowed
    unsigned	BuildingLimit;		/// # buildings allowed
    unsigned	TotalUnitLimit;		/// # total unit number allowed

    unsigned	Score;			/// points for killing ...

// Display video
    unsigned	Color;			/// color of units on minimap

    union {
	struct __4pixel8__ {
	    VMemType8	Pixels[4];	/// palette colors #0 ... #3
	}	Depth8;			/// player colors for 8bpp
	struct __4pixel16__ {
	    VMemType16	Pixels[4];	/// palette colors #0 ... #3
	}	Depth16;		/// player colors for 16bpp
	struct __4pixel24__ {
	    VMemType24	Pixels[4];	/// palette colors #0 ... #3
	}	Depth24;		/// player colors for 24bpp
	struct __4pixel32__ {
	    VMemType32	Pixels[4];	/// palette colors #0 ... #3
	}	Depth32;		/// player colors for 32bpp
    }		UnitColors;		/// Unit colors for faster setup

//  Upgrades/Allows:
    Allow		Allow;		/// Allowed for player
    UpgradeTimers	UpgradeTimers;	/// Timer for the upgrades
};

/*
**	Races for the player
*/
#define PlayerRaceHuman		0	/// belongs to human
#define PlayerRaceOrc		1	/// belongs to orc
#define PlayerRaceNeutral	2	/// belongs to none

#define PlayerMaxRaces		2	/// maximal races supported

/*
**	Types for the player
*/
#define PlayerNeutral		2	/// neutral
#define PlayerNobody		3	/// unused slot
#define PlayerComputer		4	/// computer player
#define PlayerHuman		5	/// human player
#define PlayerRescuePassive	6	/// rescued passive
#define PlayerRescueActive	7	/// rescued  active

/*
**	Ai types for the player
*/
#define PlayerAiLand		0	/// attack at land
#define PlayerAiPassive		1
#define PlayerAiSea		0x19	/// attack at sea
#define PlayerAiAir		0x1A	/// attack at air

#define PlayerAiUniversal	0xFF	/// attack best

#define PlayerNumNeutral	15	/// this is the neutral player slot

#define PlayerMax		16	/// maximal players supported

/*----------------------------------------------------------------------------
--	Variables
----------------------------------------------------------------------------*/

extern int NumPlayers;			/// Player in play
extern Player Players[PlayerMax];	/// All players
extern Player* ThisPlayer;		/// Player on this computer

/*----------------------------------------------------------------------------
--	Functions
----------------------------------------------------------------------------*/

    /// Create a new player
extern void CreatePlayer(int type);
    /// Change player side.
extern void PlayerSetSide(Player* player,int side);
    /// Change player name.
extern void PlayerSetName(Player* player,char* name);
    /// Change player AI.
extern void PlayerSetAiNum(Player* player,int ai);

    /// Set a resource of the player.
extern void PlayerSetResource(Player* player,int resource,int value);

    /// Check if the unit-type didn't break any unit limits.
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
    /// Called each frame for player handlers (AI)
extern void PlayersEachFrame(void);
    /// Called each second for player handlers (AI)
extern void PlayersEachSecond(void);

    /// Change current color set to new player.
extern void PlayerPixels(const Player* player);

    /// Change current color set to new player of the sprite
extern void GraphicPlayerPixels(const Player* player, const Graphic * sprite);

    /// Output debug informations for players
extern void DebugPlayers(void);

    /// register ccl features
extern void PlayerCclRegister(void);

//@}

#endif // !__PLAYER_H__
