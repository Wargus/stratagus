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
**	Do magic for ogre-mage.
*/
local void AiDoOgreMage(Unit* unit)
{
    int r;

    if( unit->Orders[0].Action==UnitActionStill ) {
	if( UpgradeIdentAvailable(AiPlayer->Player,"upgrade-eye-of-kilrogg")
		&& UpgradeIdentAvailable(AiPlayer->Player,
		    "upgrade-ogre-mage") ) {
	    r=SyncRand();
	    if( unit->Mana>MaxMana-10 && !(r%32) ) {
		if( unit->Mana>AiEyeOfVision->ManaCost ) {
		    DebugLevel0Fn("`%s' cast eye of vision\n"
			_C_ unit->Type->Ident);

		    CommandSpellCast(unit,unit->X,unit->Y,NoUnitP,
			AiEyeOfVision,FlushCommands);
		}
	    }
	}
    }
}

/**
**	Do magic for paladin.
*/
local void AiDoPaladin(Unit* unit)
{
    int r;

    if( unit->Orders[0].Action==UnitActionStill ) {
	if( UpgradeIdentAvailable(AiPlayer->Player,"upgrade-holy-vision")
		&& UpgradeIdentAvailable(AiPlayer->Player,
		    "upgrade-paladin") ) {
	    r=SyncRand();
	    if( unit->Mana>MaxMana-10 && !(r%32) ) {
		if( unit->Mana>AiHolyVision->ManaCost ) {
		    int x;
		    int y;

		    DebugLevel0Fn("`%s' cast holy vision\n"
			_C_ unit->Type->Ident);
		    // Look around randomly
		    r>>=5;
		    x=r&0xFFFF;
		    if( x>=TheMap.Width ) {
			x=TheMap.Width-1;
		    }
		    y=SyncRand();
		    if( y>=TheMap.Height ) {
			y=TheMap.Height-1;
		    }
		    CommandSpellCast(unit,x,y,NoUnitP,
			AiHolyVision,FlushCommands);
		}
	    }
	}
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
		x=unit->X+(r&0x20) ? -(r&0x1F) : (r&0x1F);
		if( x>=TheMap.Width ) {
		    x=TheMap.Width-1;
		}
		r>>=6;
		y=unit->Y+(r&0x20) ? -(r&0x1F) : (r&0x1F);
		if( y>=TheMap.Height ) {
		    y=TheMap.Height-1;
		}
		CommandMove(unit,x,y,FlushCommands);
	    }
	}
    }
}

//@}

#endif // } NEW_AI
