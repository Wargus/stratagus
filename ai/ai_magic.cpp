//       _________ __                 __                               
//      /   _____//  |_____________ _/  |______     ____  __ __  ______
//      \_____  \\   __\_  __ \__  \\   __\__  \   / ___\|  |  \/  ___/
//      /        \|  |  |  | \// __ \|  |  / __ \_/ /_/  >  |  /\___ | 
//     /_______  /|__|  |__|  (____  /__| (____  /\___  /|____//____  >
//             \/                  \/          \//_____/            \/ 
//  ______________________                           ______________________
//			  T H E   W A R   B E G I N S
//	   Stratagus - A free fantasy real time strategy game engine
//
/**@name ai_magic.c	-	AI magic functions. */
//
//      (c) Copyright 2002 by Lutz Sammer
//
//      This program is free software; you can redistribute it and/or modify
//      it under the terms of the GNU General Public License as published by
//      the Free Software Foundation; version 2 dated June, 1991.
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
//      $Id$

#ifdef NEW_AI	// {

//@{

/*----------------------------------------------------------------------------
--	Includes
----------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>

#include "stratagus.h"

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
local UnitType* AiMage;
local SpellType* AiSlow;
local SpellType* AiInvisibility;
local UnitType* AiOgreMage;
local SpellType* AiEyeOfVision;
local SpellType* AiBloodlust;
local UnitType* AiUnitTypeEyeOfVision;
local UnitType* AiDeathKnight;
local SpellType* AiHaste;
local SpellType* AiUnholyArmor;

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
    AiMage=UnitTypeByIdent("unit-mage");
    AiSlow=SpellTypeByIdent("spell-slow");
    AiInvisibility=SpellTypeByIdent("spell-invisibility");
    AiOgreMage=UnitTypeByIdent("unit-ogre-mage");
    AiEyeOfVision=SpellTypeByIdent("spell-eye-of-vision");
    AiBloodlust=SpellTypeByIdent("spell-bloodlust");
    AiUnitTypeEyeOfVision=UnitTypeByIdent("unit-eye-of-vision");
    AiDeathKnight=UnitTypeByIdent("unit-death-knight");
    AiHaste=SpellTypeByIdent("spell-haste");
    AiUnholyArmor=SpellTypeByIdent("spell-unholy-armor");
/*
	"spell-exorcism"
	"spell-fireball"
	"spell-flame-shield"
	"spell-polymorph"
	"spell-blizzard"

	"spell-runes"
	"spell-death-coil"
	"spell-raise-dead"
	"spell-whirlwind"
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
*/
local int AiBloodlustSpell(Unit* unit)
{
    if( UpgradeIdentAvailable(AiPlayer->Player,"upgrade-bloodlust") ) {
	if( AutoCastSpell(unit,AiBloodlust) ) {
	    return 1;
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
*/
local int AiEyeOfVisionSpell(Unit* unit)
{
    if( unit->Orders[0].Action==UnitActionStill
	    && UpgradeIdentAvailable(AiPlayer->Player,"upgrade-eye-of-kilrogg")
	    && UpgradeIdentAvailable(AiPlayer->Player,"upgrade-ogre-mage") ) {
	// s0m3body: each unit can have different MaxMana, the original
	//		condition is testing MaxMana-10, so let's take unit's
	//		maxmana * 245/255 as a new threshold
	if( unit->Mana>(unit->Type->_MaxMana*245)/255 && !SyncRand()%32 ) {
	    if( AutoCastSpell(unit,AiEyeOfVision) ) {
		return 1;
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
**	Check if the unit should cast the "slow" spell.
**
**	If the spell is available and the unit has enough mana, the surrounding
**	of the unit is checked if any enemy units are in sight. If enemy units
**	are in sight the spell is casted on the units in range.
**
**	@param unit	Magic unit.
**
**	@return		True, if a spell is casted.
*/
local int AiSlowSpell(Unit* unit)
{
    if( UpgradeIdentAvailable(AiPlayer->Player,"upgrade-slow") ) {
	if( !SyncRand()%4 && AutoCastSpell(unit,AiSlow) ) {
	    return 1;
	}
    }
    return 0;
}

/**
**	Check if the unit should cast the "invisibility" spell.
**
**	If the spell is available and the unit has enough mana, the surrounding
**	of the unit is checked if any enemy units are in sight. If enemy units
**	are in sight the spell is casted on own units in range.
**
**	@param unit	Magic unit.
**
**	@return		True, if a spell is casted.
*/
local int AiInvisibilitySpell(Unit* unit)
{
    if( UpgradeIdentAvailable(AiPlayer->Player,"upgrade-invisibility") ) {
	if( !SyncRand()%4 && AutoCastSpell(unit,AiInvisibility) ) {
	    return 1;
	}
    }
    return 0;
}

/**
**	Do magic for mage.
*/
local void AiDoMage(Unit* unit)
{
    if (AiSlowSpell(unit)) {
	return;
    }
    if (AiInvisibilitySpell(unit)) {
	return;
    }
}

/**
**	Check if the unit should cast the "healing" spell.
**
**	If the spell is available and the unit has enough mana, the surrounding
**	of the unit is checked if any ally damaged units are in sight. If ally
**	damaged units are in sight the spell is casted on own units in range.
**
**	@param unit	Magic unit.
**
**	@return		True, if a spell is casted.
*/
local int AiHealingSpell(Unit* unit)
{
    if (UpgradeIdentAvailable(AiPlayer->Player, "upgrade-healing") ) {
	if( AutoCastSpell(unit,AiHealing) ) {
	    return 1;
	}
    }
    return 0;
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
*/
local int AiHolyVisionSpell(Unit* unit)
{
    if( unit->Orders[0].Action==UnitActionStill
	    && UpgradeIdentAvailable(AiPlayer->Player,"upgrade-holy-vision")
	    && UpgradeIdentAvailable(AiPlayer->Player,"upgrade-paladin") ) {
	// s0m3body: each unit can have different MaxMana, the original
	//		condition is testing MaxMana-10, so let's take unit's
	//		maxmana * 245/255 as a new threshold
	if( unit->Mana>(unit->Type->_MaxMana*245)/255 && !SyncRand()%32 ) {
	    if( AutoCastSpell(unit,AiHolyVision) ) {
		return 1;
	    }
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
**	Check if the unit should cast the "haste" spell.
**
**	If the spell is available and the unit has enough mana, the surrounding
**	of the unit is checked if any ally units are in sight. If ally units
**	are in sight the spell is casted on the units in range.
**
**	@param unit	Magic unit.
**
**	@return		True, if a spell is casted.
*/
local int AiHasteSpell(Unit* unit)
{
    if( UpgradeIdentAvailable(AiPlayer->Player,"upgrade-haste") ) {
	if( !SyncRand()%4 && AutoCastSpell(unit,AiHaste) ) {
	    return 1;
	}
    }
    return 0;
}

/**
**	Check if the unit should cast the "unholy armor" spell.
**
**	If the spell is available and the unit has enough mana, the surrounding
**	of the unit is checked if any enemy units are in sight. If enemy units
**	are in sight the spell is casted on own units in range.
**
**	@param unit	Magic unit.
**
**	@return		True, if a spell is casted.
*/
local int AiUnholyArmorSpell(Unit* unit)
{
    if( UpgradeIdentAvailable(AiPlayer->Player,"upgrade-unholy-armor") ) {
	if( !SyncRand()%4 && AutoCastSpell(unit,AiUnholyArmor) ) {
	    return 1;
	}
    }
    return 0;
}

/**
**	Do magic for death knight.
*/
local void AiDoDeathKnight(Unit* unit)
{
    if (AiHasteSpell(unit)) {
	return;
    }
    if (AiUnholyArmorSpell(unit)) {
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
	    } else if( unit->Type==AiMage ) {
		AiDoMage(unit);
	    } else if( unit->Type==AiPaladin ) {
		AiDoPaladin(unit);
	    } else if( unit->Type==AiDeathKnight ) {
		AiDoDeathKnight(unit);
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
