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
/*
**	(c) Copyright 1998-2000 by Lutz Sammer
**
**	$Id$
*/

#ifndef __PLAYER_H__
#define __PLAYER_H__

//@{

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

//    unsigned	UnitFlags[
//	(UnitTypeInternalMax+BitsOf(unsigned)-1)
//	    /BitsOf(unsigned)];		/// flags what units are available
    unsigned    UnitTypesCount[UnitTypeInternalMax];       /// each type unit count

    unsigned	AiEnabled;		/// handle ai on this computer
    void*	Ai;			/// Ai structure pointer

    Unit**	Units;			/// units of this player
    unsigned	TotalNumUnits;		/// total # units for units' list.

    unsigned	Food;			/// food available/produced
    unsigned	NumFoodUnits;		/// # units (need food)
    unsigned	NumBuildings;		/// # buildings (don't need food)

    unsigned	Score;			/// points for killing ...

// Display video
    unsigned	Color;			/// color of units on minimap

    // FIXME: not used
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

    // FIXME: this should be removed, use UnitColors insteed
    //unsigned	UnitColor1;		/// Unit color 1 on map and icons
    //unsigned	UnitColor2;		/// Unit color 2 on map and icons
    //unsigned	UnitColor3;		/// Unit color 3 on map and icons
    //unsigned	UnitColor4;		/// Unit color 4 on map and icons

//  Upgrades/Allows:

    Allow		Allow;		/// Allowed for player
    UpgradeTimers	UTimers;	/// Timer for the upgrades
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
extern int NetPlayers;			/// Network players
extern Player Players[PlayerMax];	/// All players
extern Player* ThisPlayer;		/// Player on this computer

/*----------------------------------------------------------------------------
--	Functions
----------------------------------------------------------------------------*/

    /// Create a new player
extern void CreatePlayer(char* name,int type);
    /// Change player side.
extern void PlayerSetSide(Player* player,int side);
    /// Change player AI.
extern void PlayerSetAiNum(Player* player,int ai);

    /// Set a resource of the player.
extern void PlayerSetResource(Player* player,int resource,int value);

    /// Check if enough food is available for unit type
extern int PlayerCheckFood(const Player* player,const UnitType* type);

    /// Check if enough resources are available for costs
extern int PlayerCheckCosts(const Player* player,const int* costs);
    /// Check if enough resources are available for a new unit type
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

    /// FIXME: write short docu
extern void PlayersInitAi(void);
    /// FIXME: write short docu
extern void PlayersEachFrame(void);
    /// FIXME: write short docu
extern void PlayersEachSecond(void);

    /// Change current color set to new player.
extern void PlayerPixels(const Player* player);

    /// Change current color set to new player of the sprite
#ifdef NEW_VIDEO
extern void GraphicPlayerPixels(const Player* player, const Graphic * sprite);
#else
extern void RLEPlayerPixels(const Player* player, const RleSprite * sprite);
#endif

    /// Output debug informations for players
extern void DebugPlayers(void);

//@}

#endif // !__PLAYER_H__
