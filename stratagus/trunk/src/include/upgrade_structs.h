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
//
//	(c) Copyright 1999-2001 by Vladi Belperchinov-Shabanski
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

/*----------------------------------------------------------------------------
--	Declarations
----------------------------------------------------------------------------*/

/**
**	Default resources for a new player.
*/
#define DEFAULT_RESOURCES \
	((int[]){ 0,  2000, 1000, 1000,  1000, 1000, 1000 })
#define DEFAULT_RESOURCES_LOW \
	((int[]){ 0,  2000, 1000, 1000,  1000, 1000, 1000 })
#define DEFAULT_RESOURCES_MEDIUM \
	((int[]){ 0,  5000, 2000, 2000,  2000, 2000, 2000 })
#define DEFAULT_RESOURCES_HIGH \
	((int[]){ 0,  10000, 5000, 5000,  5000, 5000, 5000 })

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
    GoldCost,				/// gold  resource
    WoodCost,				/// wood  resource
    OilCost,				/// oil   resource
// extensions
    OreCost,				/// ore   resource
    StoneCost,				/// stone resource
    CoalCost,				/// coal  resource

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
    const void*	OType;			/// Object type (future extensions)
    char*	Ident;			/// identifier
    int		Costs[MaxCosts];	/// costs for the upgrade
	// FIXME: not used by buttons
    IconConfig	Icon;			/// icon to display to the user
} Upgrade;

/*----------------------------------------------------------------------------
--	upgrades and modifiers
----------------------------------------------------------------------------*/

/**
**	Modifiers of the unit stats.
**	All the following are modifiers not values!
**	@see UnitStats
*/
typedef struct _modifiers_ {
    int	AttackRange;			/// attack range modifier
    int	SightRange;			/// sight range modifier
    int	BasicDamage;			/// basic damage modifier
    int	PiercingDamage;			/// piercing damage modifier
    int	Armor;				/// armor modifier
    int	Speed;				/// speed modifier (FIXME: not working)
    int	HitPoints;			/// hit points modifier

    int	Costs[MaxCosts];		/// costs modifier
} Modifiers;

/**
**	This is the modifier of an upgrade.
**	This do the real action of an upgrade, an upgrade can have multiple
**	modifiers.
*/
typedef struct _upgrade_modifier_ {

  int		UpgradeId;		/// used to filter required modifier

  Modifiers	Modifier;		/// modifier of unit stats

  // allow/forbid bitmaps -- used as chars for example:
  // `?' -- leave as is, `F' -- forbid, `A' -- allow
  // FIXME: see below allow more semantics?
  // FIXME: pointers or ids would be faster and less memory use
  char	ChangeUnits[UnitTypeMax];	/// allow/forbid units
  char	ChangeUpgrades[UpgradeMax];	/// allow/forbid upgrades
  char	ApplyTo[UnitTypeMax];		/// which unit types are affected

  // FIXME: UnitType*
  void*		ConvertTo;		/// convert to this unit-type.

} UpgradeModifier;

/**
**	Allow what a player can do. Every player has an own Allow struct.
**
**	This could allow/disallow units, actions or upgrades.
**
**	Values are:
**		`A' -- allowed,
**		`F' -- forbidden,
**		`R' -- acquired, perhaps other values
**		`Q' -- acquired but forbidden (does it make sense?:))
**		`E' -- enabled, allowed by level but currently forbidden
*/
typedef struct _allow_ {
    char	Units[UnitTypeMax];	/// Units allowed/disallowed
    char	Upgrades[UpgradeMax];	/// Upgrades allowed/disallowed
} Allow;

/**
**	Upgrade timer used in the player structure.
**	Every player has an own UpgradeTimers struct.
*/
typedef struct _upgrade_timers_ {

    /**
    **	all 0 at the beginning, all upgrade actions do increment values in
    **	this struct.
    */
    int	Upgrades[UpgradeMax];		/// Counter for each upgrade

} UpgradeTimers;

/*----------------------------------------------------------------------------
--	Variables
----------------------------------------------------------------------------*/

extern const char UpgradeType[];	/// upgrade type
extern Upgrade Upgrades[UpgradeMax];	/// The main user useable upgrades

//@}

#endif // !__UPGRADE_STRUCTS_H__
