//   ___________		     _________		      _____  __
//   \_	  _____/______   ____   ____ \_   ___ \____________ _/ ____\/  |_
//    |    __) \_  __ \_/ __ \_/ __ \/    \  \/\_  __ \__  \\   __\\   __\ 
//    |     \   |  | \/\  ___/\  ___/\     \____|  | \// __ \|  |   |  |
//    \___  /   |__|    \___  >\___  >\______  /|__|  (____  /__|   |__|
//        \/		    \/	   \/	     \/		   \/
//  ______________________                           ______________________
//                        T H E   W A R   B E G I N S
//	   FreeCraft - A free fantasy real time strategy game engine
//
/**@name spells.h	-	The Spells. */
//
//	(c) Copyright 1999-2001 by Vladi Belperchinov-Shabanski
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

/*----------------------------------------------------------------------------
--	Definitons
----------------------------------------------------------------------------*/

/**
**	Different spell actions.
*/
typedef enum _spell_action_type_ {
    SpellActionNone,			/// FIXME: Comments
//	---human paladins---
    SpellActionHolyVision,
    SpellActionHealing,
    SpellActionExorcism,
//	---human mages---
    SpellActionFireball,
    SpellActionSlow,
    SpellActionFlameShield,
    SpellActionInvisibility,
    SpellActionPolymorph,
    SpellActionBlizzard,
//	---orc ogres---
    SpellActionEyeOfKilrogg,
    SpellActionBloodlust,
    SpellActionRunes,
//	---orc death knights---
    SpellActionDeathCoil,
    SpellActionHaste,
    SpellActionRaiseDead,
    SpellActionWhirlwind,
    SpellActionUnholyArmor,
    SpellActionDeathAndDecay
} SpellActionType;

/**
**	Base structure of a spell type
*/
typedef struct _spell_type_ {

    int  Id;			/// index in the table (set by InitSpells())
    char Ident[64];		/// spell name (ident)

    int  Range;			/// spell range
    int  ManaCost;		/// required mana for each cast
    int  TTL;			/// time to live (ticks)

    SpellActionType  Action;	/// SpellAction*

    SoundConfig Casted;		/// sound played if casted
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
extern const SpellType* SpellTypeByIdent( const char* Ident );

    /// return spell type by spell id
extern SpellType* SpellTypeById( int Id );

/*    // returns != 0 if spell can be casted by this unit, enough mana?
extern int CanCastSpell( const Unit* unit, int SpellId );
*/
    /// fire spell on target unit or place at x,y
extern int SpellCast( int SpellId, Unit* unit, Unit* target, int x, int y );

//@}

#endif	// !__BUTTON_H__
