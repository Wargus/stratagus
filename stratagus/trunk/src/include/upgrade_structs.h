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
/**@name upgrade_structs.h	-	The upgrade/allow headerfile. */
/*
**	(c) Copyright 1999-2000 by Vladi Belperchinov-Shabanski
**
**	$Id$
*/

#ifndef __UPGRADE_STRUCTS_H__
#define __UPGRADE_STRUCTS_H__

//@{

/*----------------------------------------------------------------------------
--	Includes
----------------------------------------------------------------------------*/

#include "icons.h"

/*----------------------------------------------------------------------------
--	Defines
----------------------------------------------------------------------------*/

    /// max upgrade/allow item/types count
#define MAXUACOUNT	256

/*----------------------------------------------------------------------------
--	Declarations
----------------------------------------------------------------------------*/

/**
**	Default resources for a new player.
*/
#define DEFAULT_RESOURCES \
	((int[]){ 0,  3000, 1000, 1000,  1000, 1000, 1000 })

/**
**	Default incomes for a new player.
*/
#define DEFAULT_INCOMES \
	((int[]){ 0,   100,  100,  100,   100,  100,  100 })

/**
**	Default action for the resources.
*/
#define DEFAULT_ACTIONS \
	((char*[]){"stop","mine","chop","drill","mine","mine","mine"})

/**
**	Default names for the resources.
*/
#define DEFAULT_NAMES \
	((char*[]){"time","gold","wood","oil","ore","stone","coal"})

/**
**	Indices into costs/resource/income array. (A litte future here :)
*/
enum _costs_ {
    TimeCost,				/// time in frames

// standard
    GoldCost,				/// gold resource
    WoodCost,				/// wood resource
    OilCost,				/// oil  resource
// extensions
    OreCost,				/// ore  resource
    StoneCost,				/// stone resource
    CoalCost,				/// coal resource

    MaxCosts				/// how many different costs
};

/**
**	This are the current stats of an unit. Upgraded or downgraded.
*/
typedef struct _unit_stats_ {
    int		AttackRange;		/// how far can the unit attack
    int		SightRange;		/// how far can the unit see
    int		Armor;			/// armor strength
    int		BasicDamage;		/// weapon basic damage
    int		PiercingDamage;		/// weapon piercing damage
    int		Speed;			/// movement speed
    int		HitPoints;		/// hit points
    int		Costs[MaxCosts];	/// Current costs of the unit.
    int		Level;			/// unit level (upgrades)
} UnitStats;

/**
**	The main useable upgrades.
*/
typedef struct _upgrade_ {
    char*	Ident;			/// identifier
    int		Costs[MaxCosts];	/// costs for the upgrade
    IconId	Icon;			/// icon to display to the user
} Upgrade;

/*----------------------------------------------------------------------------
--	Variables
----------------------------------------------------------------------------*/

extern Upgrade Upgrades[MAXUACOUNT];	/// The main user useable upgrades




//Cleaning this
//vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
//vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
//vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
//vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
//vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv

/*----------------------------------------------------------------------------
--	upgrades and modifiers
----------------------------------------------------------------------------*/

// should keep with the same count ( even in reserve ) just to keep
// compatibility if new units added.
#define MAXUNITTYPES	MAXUACOUNT
#define MAXUPGRADES	MAXUACOUNT
#define MAXACTIONS	MAXUACOUNT // include spells

typedef struct _modifiers_
{
  //int level; // unit level (upgrades) used not in the same context -- ignore

  // all the following are modifiers not values!
  int	AttackRange;			/// attack range modifier
  int	SightRange;			/// sight range modifier
  int	BasicDamage;			/// basic damage modifier
  int	PiercingDamage;			/// piercing damage modifier
  int	Armor;				/// armor modifier
  int	Speed;				/// speed modifier (FIXME: not working)
  int	HitPoints;			/// hit points modifier

  int	Costs[MaxCosts];		/// costs modifier
} Modifiers;

typedef struct _upgrade_modifier_
{

  int	uid; // used to filter required by upgrade modifiers

  Modifiers mods;

  // allow/forbid bitmaps -- used as chars for example:
  // `?' -- leave as is, `F' -- forbid, `A' -- allow
  // FIXME: pointers or ids would be faster and less memory use
  char af_units[MAXUNITTYPES];   // allow/forbid units
  char af_actions[MAXACTIONS]; // allow/forbid actions
  char af_upgrades[MAXUPGRADES]; // allow/forbid upgrades
  char apply_to[MAXUNITTYPES]; // which unit types are affected

} UpgradeModifier;

typedef struct _upgrade_timers_
{

  // all 0 at the beginning, all upgrade actions do increment values in
  // this struct, every player has own UpgradeTimers struct
  int upgrades[MAXUPGRADES];

} UpgradeTimers;

/*----------------------------------------------------------------------------
--	allow
----------------------------------------------------------------------------*/

typedef struct _allow_
{

  // values are `A' -- allowed, `F' -- forbidden, `R' -- acquired, perhaps
  // other value: `Q' -- acquired but forbidden (does it make sense?:))
  char Units[MAXUNITTYPES];
  char Actions[MAXACTIONS];
  char Upgrades[MAXUPGRADES];
  // every player has own Allow struct

} Allow;

//@}

#endif // !__UPGRADE_STRUCTS_H__
