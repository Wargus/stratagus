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
/**@name ai_magic.c	-	AI magic functions. */
//
//      (c) Copyright 2002 by Lutz Sammer
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
//      $Id$

#ifdef NEW_AI	// {

//@{

/*----------------------------------------------------------------------------
--	Includes
----------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>

#include "freecraft.h"

#include "unittype.h"
#include "unit.h"
#include "spells.h"
#include "actions.h"
#include "map.h"
#include "ai_local.h"

/*----------------------------------------------------------------------------
--	Variables
----------------------------------------------------------------------------*/

local UnitType* AiPaladin;
local SpellType* AiHolyVision;
local SpellType* AiHealing;
local UnitType* AiOgreMage;
local SpellType* AiEyeOfVision;
local SpellType* AiBloodlust;
local UnitType* AiUnitTypeEyeOfVision;

/*----------------------------------------------------------------------------
--	Functions
----------------------------------------------------------------------------*/

/**
**	Init the magic.
**
**	@note I will remove the hardcoded stuff, if tested.
*/
local void AiInitMagic(void)
{
    AiPaladin=UnitTypeByIdent("unit-paladin");
    AiHolyVision=SpellTypeByIdent("spell-holy-vision");
    AiHealing=SpellTypeByIdent("spell-healing");
    AiOgreMage=UnitTypeByIdent("unit-ogre-mage");
    AiEyeOfVision=SpellTypeByIdent("spell-eye-of-vision");
    AiBloodlust=SpellTypeByIdent("spell-bloodlust");
    AiUnitTypeEyeOfVision=UnitTypeByIdent("unit-eye-of-vision");
/*
	"spell-exorcism"
	"spell-fireball"
	"spell-slow"
	"spell-flame-shield"
	"spell-invisibility"
	"spell-polymorph"
	"spell-blizzard"

	"spell-runes"
	"spell-death-coil"
	"spell-haste"
	"spell-raise-dead"
	"spell-whirlwind"
	"spell-unholy-armor"
	"spell-death-and-decay"
*/
}

/**
**	Check if the unit should cast the "bloodlust" spell.
**
**	If the spell is available and the unit has enough mana, the surrounding
**	of the unit is checked if any enemy units are in sight. If enemy units
**	are in sight the spell is casted on own units in range.
**
**	@param unit	Magic unit.
**
**	@return		True, if a spell is casted.
**
**	@note This function can also be used for auto bloodlust.
*/
local int AiBloodlustSpell(Unit* unit)
{
    Unit* best;
    Unit* table[UnitMax];
    int r;
    int i;
    int n;

    if (UpgradeIdentAvailable(AiPlayer->Player, "upgrade-bloodlust")
	    && unit->Mana > AiBloodlust->ManaCost) {

	r = unit->Type->ReactRangeComputer;
	n = SelectUnits(unit->X - r, unit->Y - r, unit->X + r + 1,
	    unit->Y + r + 1, table);

	for (i = 0; i < n; ++i) {

	    // an enemy which can attack
	    if (!IsEnemy(unit->Player, table[i])
		    || (!table[i]->Type->CanAttack)) {
		continue;
	    }
	    //
	    //      We have an enemy in range.
	    //
	    best = NoUnitP;
	    for (i = 0; i < n; ++i) {
		// FIXME: Spell castable checks must be done global by spells.
		// not self, not already bloodlust and can attack
		if (table[i] == unit || table[i]->Bloodlust
			|| !table[i]->Type->CanAttack) {
		    continue;
		}
		// Allied unit
		// FIXME: should ally to self
		if (unit->Player != table[i]->Player
			&& !IsAllied(unit->Player, table[i])) {
		    continue;
		}
		r = MapDistanceBetweenUnits(unit, table[i]);
		DebugLevel0Fn("Distance %d\n" _C_ r);
		if (r <= 1) {
		    DebugLevel0Fn("`%s' cast bloodlust\n" _C_ unit->Type->
			Ident);
		    CommandSpellCast(unit, 0, 0, table[i], AiBloodlust,
			FlushCommands);
		    return 1;
		}
		if (r == 2) {
		    best = table[i];
		}
	    }
	    if (best) {
		CommandSpellCast(unit, 0, 0, best, AiBloodlust,
		    FlushCommands);
		return 1;
	    }
	    break;
	}
    }
    return 0;
}

/**
**	Check if the unit should cast the "eye of vision" spell.
**
**	If the unit has nothing to do and the spell is available and the unit
**	has full mana cast with a change of 1/32 the spell. The spells does
**	nothing, because the AI cheats and already knows the surroundings.
**
**	@param unit	Magic unit.
**
**	@return		True, if a spell is casted.
**
**	@note This function can also be used for auto eye of vision.
*/
local int AiEyeOfVisionSpell(Unit* unit)
{
    int r;

    if( unit->Orders[0].Action==UnitActionStill ) {
	if( UpgradeIdentAvailable(AiPlayer->Player,"upgrade-eye-of-kilrogg")
		&& UpgradeIdentAvailable(AiPlayer->Player,
		    "upgrade-ogre-mage") ) {
	    r=SyncRand();
	    /* s0m3body: each unit can have different MaxMana, the original condition is testing MaxMana-10,
	     * so let's take unit's maxmana * 245/255 as a new treshold */
	    if( unit->Mana>((unit->Type->_MaxMana * 245 ) / 255) && !(r%32) ) {
		if( unit->Mana>AiEyeOfVision->ManaCost ) {
		    DebugLevel0Fn("`%s' cast eye of vision\n"
			_C_ unit->Type->Ident);

		    CommandSpellCast(unit,unit->X,unit->Y,NoUnitP,
			AiEyeOfVision,FlushCommands);
		    return 1;
		}
	    }
	}
    }
    return 0;
}

/**
**	Do magic for ogre-mage.
*/
local void AiDoOgreMage(Unit* unit)
{
    if( AiBloodlustSpell(unit) ) {
	return;
    }
    if( AiEyeOfVisionSpell(unit) ) {
	return;
    }
}

/**
**	Check if the unit should cast the "holy vision" spell.
**
**	If the unit has nothing to do and the spell is available and the unit
**	has full mana cast with a change of 1/32 the spell. The spells does
**	nothing, because the AI cheats and already knows the surroundings.
**
**	@param unit	Magic unit.
**
**	@return		True, if a spell is casted.
**
**	@note This function can also be used for auto holy vision.
*/
local int AiHolyVisionSpell(Unit* unit)
{
    int r;

    if( unit->Orders[0].Action==UnitActionStill ) {
	if( UpgradeIdentAvailable(AiPlayer->Player,"upgrade-holy-vision")
		&& UpgradeIdentAvailable(AiPlayer->Player,
		    "upgrade-paladin") ) {
	    r=SyncRand();
	    // s0m3body: each unit can have different MaxMana, the original
	    //		condition is testing MaxMana-10, so let's take unit's
	    //		maxmana * 245/255 as a new treshold
	    if( unit->Mana>((unit->Type->_MaxMana * 245)/255) && !(r%32) ) {
		if( unit->Mana>AiHolyVision->ManaCost ) {
		    int x;
		    int y;

		    DebugLevel0Fn("`%s' cast holy vision\n"
			_C_ unit->Type->Ident);
		    // Look around randomly
		    r>>=5;
		    x=r%TheMap.Width;
		    y=SyncRand()%TheMap.Height;
		    CommandSpellCast(unit,x,y,NoUnitP,
			AiHolyVision,FlushCommands);
		    return 1;
		}
	    }
	}
    }
    return 0;
}

/**
**	Check if the unit should cast the "heal" spell.
**
**	If the spell is available and the unit has enough mana, the surrounding
**	of the unit is checked if any ally damaged units are in sight. If ally
**	damaged units are in sight the spell is casted on own units in range.
**
**	@param unit	Magic unit.
**
**	@return		True, if a spell is casted.
**
**	@note This function can also be used for auto heal.
*/
local int AiHealingSpell(Unit* unit)
{
    Unit* best;
    Unit* table[UnitMax];
    int r;
    int i;
    int n;

    // FIXME : minimum Mana for Ai Healing should be configurable.
    if (UpgradeIdentAvailable(AiPlayer->Player, "upgrade-healing")
	    && unit->Mana > AiHealing->ManaCost+20) {

	r = unit->Type->ReactRangeComputer;
	n = SelectUnits(unit->X - r, unit->Y - r, unit->X + r + 1,
	    unit->Y + r + 1, table);

	for (i = 0; i < n; ++i) {

	    // an ally
	    if (IsEnemy(unit->Player, table[i])) {
		continue;
	    }
	    //
	    //      We have an ally in range...
	    //
	    best = NoUnitP;
	    for (i = 0; i < n; ++i) {
		// not self, organic and somewhat in need with healing
		//	(HP<8/10*max), ...
		// FIXME: Spell castable checks must be done global by spells.
		if (table[i] == unit || !table[i]->Type->Organic
			|| table[i]->HP>=((table[i]->Type->_HitPoints*9)/10)) {
		    continue;
		}

		// Allied unit
		// FIXME: should ally to self
		if (unit->Player != table[i]->Player
			&& !IsAllied(unit->Player, table[i])) {
		    continue;
		}
		r = MapDistanceBetweenUnits(unit, table[i]);
		DebugLevel0Fn("Distance %d\n" _C_ r);
		if (r <= 1) {
		    DebugLevel0Fn("`%s' cast healing\n" _C_ unit->Type->
			Ident);
		    CommandSpellCast(unit, 0, 0, table[i], AiHealing,
			FlushCommands);
		    return 1;
		}
		if (r == 2) {
		    best = table[i];
		}
	    }
	    if (best) {
		CommandSpellCast(unit, 0, 0, best, AiHealing,
		    FlushCommands);
		return 1;
	    }
	    break;
	}
    }
    return 0;
}

/**
**	Do magic for paladin.
*/
local void AiDoPaladin(Unit* unit)
{
    if (AiHealingSpell(unit)) {
	return;
    }
    if (AiHolyVisionSpell(unit)) {
	return;
    }
}

/**
**	Check what computer units can do with magic.
*/
global void AiCheckMagic(void)
{
    int i;
    int n;
    Unit** units;
    Unit* unit;

    AiInitMagic();

    n=AiPlayer->Player->TotalNumUnits;
    units=AiPlayer->Player->Units;
    for( i=0; i<n; ++i ) {
	unit=units[i];
	if( unit->Type->CanCastSpell ) {	// Its a magic unit
	    DebugLevel3Fn("Have mage `%s'\n" _C_ unit->Type->Ident);
	    // FIXME: I hardcode the reactions now
	    if( unit->Type==AiOgreMage ) {
		AiDoOgreMage(unit);
	    } else if( unit->Type==AiPaladin ) {
		AiDoPaladin(unit);
	    }
	}
	//
	//	Handle casted eyes of vision.
	//
	if( unit->Type==AiUnitTypeEyeOfVision ) {
	    if( unit->Orders[0].Action==UnitActionStill ) {
		int x;
		int y;
		int r;

		// Let it move around randomly
		r=SyncRand()>>4;
		if( r&0x20 ) {
		    if( unit->X<(r&0x1F) ) {
			x=0;
		    } else {
			x=unit->X-(r&0x1F);
		    }
		} else {
		    x=unit->X+(r&0x1F);
		    if( x>=TheMap.Width ) {
			x=TheMap.Width-1;
		    }
		}
		r>>=6;
		if( r&0x20 ) {
		    if( unit->Y<(r&0x1F) ) {
			y=0;
		    } else {
			y=unit->Y-(r&0x1F);
		    }
		} else {
		    y=unit->Y+(r&0x1F);
		    if( y>=TheMap.Height ) {
			y=TheMap.Height-1;
		    }
		}
		CommandMove(unit,x,y,FlushCommands);
	    }
	}
    }
}

//@}

#endif // } NEW_AI
