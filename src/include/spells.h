//   ___________		     _________		      _____  __
//   \_	  _____/______   ____   ____ \_   ___ \____________ _/ ____\/  |_
//    |    __) \_  __ \_/ __ \_/ __ \/    \  \/\_  __ \__  \\   __\\   __|
//    |     \   |  | \/\  ___/\  ___/\     \____|  | \// __ \|  |   |  |
//    \___  /   |__|    \___  >\___  >\______  /|__|  (____  /__|   |__|
//        \/		    \/	   \/	     \/		   \/
//  ______________________                           ______________________
//                        T H E   W A R   B E G I N S
//	   FreeCraft - A free fantasy real time strategy game engine
//
/**@name spells.h	-	The Spells. */
//
//	(c) Copyright 1999-2002 by Vladi Belperchinov-Shabanski
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

#ifndef __SPELLS_H__
#define __SPELLS_H__

//@{

/*----------------------------------------------------------------------------
--	Includes
----------------------------------------------------------------------------*/

#include <stdio.h>

#include "freecraft.h"
#include "sound_id.h"
#include "sound.h"
#include "unittype.h"
#include "unit.h"
#include "missile.h"

/*----------------------------------------------------------------------------
--	Definitons
----------------------------------------------------------------------------*/

/**
**	Different spell actions, defines the behavior of the spells.
*/
typedef enum _spell_action_type_ {
    SpellActionNone,			/// No spell, shouldn't happen
//	---human paladins---
    SpellActionHolyVision,		/// Holy vision
    SpellActionHealing,			/// Healing
    SpellActionExorcism,		/// Exorcism
//	---human mages---
    SpellActionFireball,		/// Fireball
    SpellActionSlow,			/// Slow down
    SpellActionFlameShield,		/// Flame shield
    SpellActionInvisibility,		/// Invisibility
    SpellActionPolymorph,		/// Polymorph
    SpellActionBlizzard,		/// Blizzard
//	---orc ogres---
    SpellActionEyeOfKilrogg,		/// Eye
    SpellActionBloodlust,		/// Blood lust
    SpellActionRunes,			/// Runes
//	---orc death knights---
    SpellActionDeathCoil,		/// Death coil
    SpellActionHaste,			/// Haste
    SpellActionRaiseDead,		/// Raise dead
    SpellActionWhirlwind,		/// Whirlwind
    SpellActionUnholyArmor,		/// Unholy armor
    SpellActionDeathAndDecay,		/// Death and decay

//	---new spells---
    SpellActionCircleOfPower,		/// Circle of power

    // Here you can add new spell actions

} SpellActionType;

/**
**	Base structure of a spell type
**
**	@todo	Move more parameters into this structure.
*/
typedef struct _spell_type_ {

    char* Ident;		/// spell identifier
    char* Name;			/// spell name shown by the engine

    int  Range;			/// spell range
    unsigned  ManaCost : 8;	/// required mana for each cast
    int  TTL;			/// time to live (ticks)

    SpellActionType  Action;	/// SpellAction...

    SoundConfig Casted;		/// sound played if casted
    MissileConfig Missile;	/// missile fired on cast
} SpellType;

/*----------------------------------------------------------------------------
--	Variables
----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
--	Functions
----------------------------------------------------------------------------*/

    /// init spell tables
extern void InitSpells(void);

    /// done spell tables
extern void DoneSpells(void);

    /// return spell id by ident string
extern int SpellIdByIdent( const char* Ident );

    /// return spell type by ident string
extern SpellType* SpellTypeByIdent( const char* Ident );

    /// return spell type by spell id
extern SpellType* SpellTypeById( int Id );

    /// returns != 0 if spell can be casted by this unit, enough mana?
extern int CanCastSpell( const Unit*, const SpellType*, const Unit*, int, int );

    /// fire spell on target unit or place at x,y
extern int SpellCast( Unit*, const SpellType* , Unit* , int , int );

    /// auto cast the spell if possible
extern int AutoCastSpell(Unit* unit, SpellType* spell);

//@}

#endif	// !__SPELLS_H__
