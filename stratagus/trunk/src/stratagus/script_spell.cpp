//       _________ __                 __                               
//      /   _____//  |_____________ _/  |______     ____  __ __  ______
//      \_____  \\   __\_  __ \__  \\   __\__  \   / ___\|  |  \/  ___/
//      /        \|  |  |  | \// __ \|  |  / __ \_/ /_/  >  |  /\___ |
//     /_______  /|__|  |__|  (____  /__| (____  /\___  /|____//____  >
//             \/                  \/          \//_____/            \/ 
//  ______________________                           ______________________
//			  T H E	  W A R	  B E G I N S
//	   Stratagus - A free fantasy real time strategy game engine
//
/**@name ccl_spells.c	-	The spell ccl functions.. */
//
//	(c) Copyright 1998-2003 by Joris DAUPHIN
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
//	$Id$
//@{

/*----------------------------------------------------------------------------
--	Includes
----------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "stratagus.h"
/*
#include "video.h"
#include "tileset.h"
#include "unittype.h"
*/
#include "spells.h"
#include "ccl_sound.h"
#include "ccl.h"

// **************************************************************************
//		Action parsers for spellAction
// **************************************************************************

/**
** 	Parse the missile location description for a spell action.
**
**	@param list		SCM list object, with the description.
**	@param location		Pointer to missile location description.
**
**	@note	This is only here to avoid code duplication. You don't have
**	any reason to USE this:)
*/
#if defined(USE_GUILE) || defined(USE_SIOD)
local void CclSpellMissileLocation(SCM list, SpellActionMissileLocation* location)
{
    SCM value;
 
    DebugCheck(location == NULL);
    memset(location, 0, sizeof(*location));
    //list = gh_cdr(list);

    while (!gh_null_p(list)) {
	value = gh_car(list);
	list = gh_cdr(list);
	if (gh_eq_p(value, gh_symbol2scm("base"))) {
	    if (gh_eq_p(gh_car(list), gh_symbol2scm("caster"))) {
		location->Base = LocBaseCaster;
	    } else if (gh_eq_p(gh_car(list), gh_symbol2scm("target"))) {
		location->Base = LocBaseTarget;
	    } else {
		errl("Unsupported missile location base flag.\n",gh_car(list));
	    }
	    list = gh_cdr(list);
	} else if (gh_eq_p(value, gh_symbol2scm("add-x"))) {
	    location->AddX = gh_scm2int(gh_car(list));
	    list = gh_cdr(list);
	} else if (gh_eq_p(value, gh_symbol2scm("add-y"))) {
	    location->AddY = gh_scm2int(gh_car(list));
	    list = gh_cdr(list);
	} else if (gh_eq_p(value, gh_symbol2scm("add-rand-x"))) {
	    location->AddRandX = gh_scm2int(gh_car(list));
	    list = gh_cdr(list);
	} else if (gh_eq_p(value, gh_symbol2scm("add-rand-y"))) {
	    location->AddRandY = gh_scm2int(gh_car(list));
	    list = gh_cdr(list);
	} else {
	    errl("Unsupported missile location description flag.\n",value);
	}
    }
}
#elif defined(USE_LUA)
#endif

/**
**	Parse the action for spell.
**
**	@param list		SCM list object, with something like (action-type params).
**	@param spellaction	Pointer to spellactopm.
*/
#if defined(USE_GUILE) || defined(USE_SIOD)
local void CclSpellAction(SCM list, SpellActionType* spellaction)
{
    char* str;
    SCM	value;
 
    DebugCheck(spellaction == NULL);

    value = gh_car(list);
    list = gh_cdr(list);

    if (gh_eq_p(value, gh_symbol2scm("spawn-missile"))) {
	spellaction->CastFunction = CastSpawnMissile;
	spellaction->Data.SpawnMissile.StartPoint.Base=LocBaseCaster;
	spellaction->Data.SpawnMissile.EndPoint.Base=LocBaseTarget;
	spellaction->Data.SpawnMissile.TTL=-1;
	while (!gh_null_p(list)) {
	    value = gh_car(list);
	    list = gh_cdr(list);
	    if (gh_eq_p(value, gh_symbol2scm("damage"))) {
		spellaction->Data.SpawnMissile.Damage = gh_scm2int(gh_car(list));
		list = gh_cdr(list);
	    } else if (gh_eq_p(value, gh_symbol2scm("delay"))) {
		spellaction->Data.SpawnMissile.Delay = gh_scm2int(gh_car(list));
		list = gh_cdr(list);
	    } else if (gh_eq_p(value, gh_symbol2scm("ttl"))) {
		spellaction->Data.SpawnMissile.TTL = gh_scm2int(gh_car(list));
		list = gh_cdr(list);
	    } else if (gh_eq_p(value, gh_symbol2scm("start-point"))) {
		CclSpellMissileLocation(gh_car(list),&spellaction->Data.SpawnMissile.StartPoint);
		list = gh_cdr(list);
	    } else if (gh_eq_p(value, gh_symbol2scm("end-point"))) {
		CclSpellMissileLocation(gh_car(list),&spellaction->Data.SpawnMissile.EndPoint);
		list = gh_cdr(list);
	    } else {
		errl("Unsupported area-bombardment tag", value);
	    }
	}
    } else if (gh_eq_p(value, gh_symbol2scm("area-adjust-vitals"))) {
	spellaction->CastFunction = CastAreaAdjustVitals;
	while (!gh_null_p(list)) {
	    value = gh_car(list);
	    list = gh_cdr(list);
	    if (gh_eq_p(value, gh_symbol2scm("hit-points"))) {
		spellaction->Data.AreaAdjustVitals.HP = gh_scm2int(gh_car(list));
		list = gh_cdr(list);
	    } else if (gh_eq_p(value, gh_symbol2scm("mana-points"))) {
		spellaction->Data.AreaAdjustVitals.Mana = gh_scm2int(gh_car(list));
		list = gh_cdr(list);
	    } else {
		errl("Unsupported area-adjust-vitals tag", value);
	    }
	}
    } else if (gh_eq_p(value, gh_symbol2scm("area-bombardment"))) {
	spellaction->CastFunction = CastAreaBombardment;
	while (!gh_null_p(list)) {
	    value = gh_car(list);
	    list = gh_cdr(list);
	    if (gh_eq_p(value, gh_symbol2scm("fields"))) {
		spellaction->Data.AreaBombardment.Fields = gh_scm2int(gh_car(list));
		list = gh_cdr(list);
	    } else if (gh_eq_p(value, gh_symbol2scm("shards"))) {
		spellaction->Data.AreaBombardment.Shards = gh_scm2int(gh_car(list));
		list = gh_cdr(list);
	    } else if (gh_eq_p(value, gh_symbol2scm("damage"))) {
		spellaction->Data.AreaBombardment.Damage = gh_scm2int(gh_car(list));
		list = gh_cdr(list);
	    } else if (gh_eq_p(value, gh_symbol2scm("start-offset-x"))) {
		spellaction->Data.AreaBombardment.StartOffsetX = gh_scm2int(gh_car(list));
		list = gh_cdr(list);
	    } else if (gh_eq_p(value, gh_symbol2scm("start-offset-y"))) {
		spellaction->Data.AreaBombardment.StartOffsetY = gh_scm2int(gh_car(list));
		list = gh_cdr(list);
	    } else {
		errl("Unsupported area-bombardment tag", value);
	    }
	}
    } else if (gh_eq_p(value, gh_symbol2scm("demolish"))) {
	spellaction->CastFunction = CastDemolish;
	while (!gh_null_p(list)) {
	    value = gh_car(list);
	    list = gh_cdr(list);
	    if (gh_eq_p(value, gh_symbol2scm("range"))) {
		spellaction->Data.Demolish.Range = gh_scm2int(gh_car(list));
		list = gh_cdr(list);
	    } else if (gh_eq_p(value, gh_symbol2scm("damage"))) {
		spellaction->Data.Demolish.Damage = gh_scm2int(gh_car(list));
		list = gh_cdr(list);
	    } else {
		errl("Unsupported demolish tag", value);
	    }
	}
    } else if (gh_eq_p(value, gh_symbol2scm("adjust-buffs"))) {
	spellaction->CastFunction = CastAdjustBuffs;
	spellaction->Data.AdjustBuffs.HasteTicks = BUFF_NOT_AFFECTED;
	spellaction->Data.AdjustBuffs.SlowTicks = BUFF_NOT_AFFECTED;
	spellaction->Data.AdjustBuffs.BloodlustTicks = BUFF_NOT_AFFECTED;
	spellaction->Data.AdjustBuffs.InvisibilityTicks = BUFF_NOT_AFFECTED;
	spellaction->Data.AdjustBuffs.InvincibilityTicks = BUFF_NOT_AFFECTED;
	while (!gh_null_p(list)) {
	    value = gh_car(list);
	    list = gh_cdr(list);
	    if (gh_eq_p(value, gh_symbol2scm("haste-ticks"))) {
		spellaction->Data.AdjustBuffs.HasteTicks = gh_scm2int(gh_car(list));
		list = gh_cdr(list);
	    } else if (gh_eq_p(value, gh_symbol2scm("slow-ticks"))) {
		spellaction->Data.AdjustBuffs.SlowTicks  = gh_scm2int(gh_car(list));
		list = gh_cdr(list);
	    } else if (gh_eq_p(value, gh_symbol2scm("bloodlust-ticks"))) {
		spellaction->Data.AdjustBuffs.BloodlustTicks  = gh_scm2int(gh_car(list));
		list = gh_cdr(list);
	    } else if (gh_eq_p(value, gh_symbol2scm("invisibility-ticks"))) {
		spellaction->Data.AdjustBuffs.InvisibilityTicks  = gh_scm2int(gh_car(list));
		list = gh_cdr(list);
	    } else if (gh_eq_p(value, gh_symbol2scm("invincibility-ticks"))) {
		spellaction->Data.AdjustBuffs.InvincibilityTicks  = gh_scm2int(gh_car(list));
		list = gh_cdr(list);
	    } else {
		errl("Unsupported adjust-buffs tag", value);
	    }
	}
    } else if (gh_eq_p(value, gh_symbol2scm("summon"))) {
	spellaction->CastFunction = CastSummon;
	while (!gh_null_p(list)) {
	    value = gh_car(list);
	    list = gh_cdr(list);
	    if (gh_eq_p(value, gh_symbol2scm("unit-type"))) {
		str = gh_scm2newstr(gh_car(list), 0);
		spellaction->Data.Summon.UnitType = UnitTypeByIdent(str);
		if (!spellaction->Data.Summon.UnitType) {
		    spellaction->Data.Summon.UnitType = 0;
		    DebugLevel0("unit type \"%s\" not found for summon spell.\n" _C_ str);
		}
		free(str);
		list = gh_cdr(list);
	    } else if (gh_eq_p(value, gh_symbol2scm("time-to-live"))) {
		spellaction->Data.Summon.TTL = gh_scm2int(gh_car(list));
		list = gh_cdr(list);
	    } else if (gh_eq_p(value, gh_symbol2scm("require-corpse"))) {
		spellaction->Data.Summon.RequireCorpse = 1;
	    } else {
		errl("Unsupported summon tag", value);
	    }
	}
    } else if (gh_eq_p(value, gh_symbol2scm("spawn-portal"))) {
	spellaction->CastFunction = CastSpawnPortal;
	while (!gh_null_p(list)) {
	    value = gh_car(list);
	    list = gh_cdr(list);
	    if (gh_eq_p(value, gh_symbol2scm("portal-type"))) {
		str = gh_scm2newstr(gh_car(list), 0);
		spellaction->Data.SpawnPortal.PortalType = UnitTypeByIdent(str);
		if (!spellaction->Data.SpawnPortal.PortalType) {
		    spellaction->Data.SpawnPortal.PortalType = 0;
		    DebugLevel0("unit type \"%s\" not found for spawn-portal.\n" _C_ str);
		}
		free(str);
		list = gh_cdr(list);
	    } else {
		errl("Unsupported spawn-portal tag", value);
	    }
	}
    } else if (gh_eq_p(value, gh_symbol2scm("polymorph"))) {
	spellaction->CastFunction = CastPolymorph;
	while (!gh_null_p(list)) {
	    value = gh_car(list);
	    list = gh_cdr(list);
	    if (gh_eq_p(value, gh_symbol2scm("new-form"))) {
		str = gh_scm2newstr(gh_car(list),0);
		spellaction->Data.Summon.UnitType = UnitTypeByIdent(str);
		if (!spellaction->Data.Summon.UnitType) {
		    spellaction->Data.Summon.UnitType = 0;
		    DebugLevel0("unit type \"%s\" not found for polymorph spell.\n" _C_ str);
		}
		free(str);
		list = gh_cdr(list);
		//FIXME : temp polymorphs? hard to do.
	    } else {
		errl("Unsupported polymorph tag", value);
	    }
	}
    } else if (gh_eq_p(value, gh_symbol2scm("adjust-vitals"))) {
	spellaction->CastFunction = CastAdjustVitals;
	while (!gh_null_p(list)) {
	    value = gh_car(list);
	    list = gh_cdr(list);
	    if (gh_eq_p(value, gh_symbol2scm("hit-points"))) {
		spellaction->Data.AdjustVitals.HP = gh_scm2int(gh_car(list));
		list = gh_cdr(list);
	    } else if (gh_eq_p(value, gh_symbol2scm("mana-points"))) {
		spellaction->Data.AdjustVitals.Mana = gh_scm2int(gh_car(list));
		list = gh_cdr(list);
	    } else if (gh_eq_p(value, gh_symbol2scm("max-multi-cast"))) {
		spellaction->Data.AdjustVitals.MaxMultiCast = gh_scm2int(gh_car(list));
		list = gh_cdr(list);
	    } else {
		errl("Unsupported adjust-vitals tag", value);
	    }
	}
    } else {
	errl("Unsupported action type", value);
    }
}
#elif defined(USE_LUA)
#endif

/**
**	Get a condition value from a scm object.
**	
**	@param 	value		scm value to convert.
**
**	@return CONDITION_TRUE, CONDITION_FALSE, CONDITION_ONLY or -1 on error.
**	@note 	This is a helper function to make CclSpellCondition shorter
**		and easier to understand.
*/
#if defined(USE_GUILE) || defined(USE_SIOD)
global char Scm2Condition(SCM value)
{
    if (gh_eq_p(value, gh_symbol2scm("true"))) {
	return CONDITION_TRUE;
    } else if (gh_eq_p(value, gh_symbol2scm("false"))) {
	return CONDITION_FALSE;
    } else if (gh_eq_p(value, gh_symbol2scm("only"))) {
	return CONDITION_ONLY;
    } else {
	errl("Bad condition result", value);
	return -1;
    }
}
#elif defined(USE_LUA)
#endif

/**
**	Parse the Condition for spell.
**	
**	@param list		SCM object to parse
**	@param condition	pointer to condition to fill with data.
**
**	@notes: conditions must be allocated. All data already in is LOST.
*/
#if defined(USE_GUILE) || defined(USE_SIOD)
local void CclSpellCondition(SCM list, ConditionInfo* condition)
{
    SCM value;
    int i;
    //
    //	Initializations:
    //

    //	Set everything to 0:
    memset(condition, 0, sizeof(ConditionInfo));
    //	Flags are defaulted to 0(CONDITION_TRUE)
    condition->BoolFlag = malloc(NumberBoolFlag * sizeof (*condition->BoolFlag));
    memset(condition->BoolFlag, 0, NumberBoolFlag * sizeof (*condition->BoolFlag));
    //	Initialize min/max stuff to values with no effect.
    condition->MinHpPercent = -10;
    condition->MaxHpPercent = 1000;
    condition->MinManaPercent = -10;
    condition->MaxManaPercent = 1000;
    //  Buffs too.
    condition->MaxHasteTicks = 0xFFFFFFF;
    condition->MaxSlowTicks = 0xFFFFFFF;
    condition->MaxBloodlustTicks = 0xFFFFFFF;
    condition->MaxInvisibilityTicks = 0xFFFFFFF;
    condition->MaxInvincibilityTicks = 0xFFFFFFF;
    //  Now parse the list and set values.
    while (!gh_null_p(list)) {
	value = gh_car(list);
	list = gh_cdr(list);
	if (gh_eq_p(value, gh_symbol2scm("coward"))) {
	    condition->Coward = Scm2Condition(gh_car(list));
	    list = gh_cdr(list);
	} else if (gh_eq_p(value, gh_symbol2scm("alliance"))) {
	    condition->Alliance = Scm2Condition(gh_car(list));
	    list = gh_cdr(list);
	} else if (gh_eq_p(value, gh_symbol2scm("building"))) {
	    condition->Building = Scm2Condition(gh_car(list));
	    list = gh_cdr(list);
	} else if (gh_eq_p(value, gh_symbol2scm("self"))) {
	    condition->TargetSelf = Scm2Condition(gh_car(list));
	    list = gh_cdr(list);
	} else if (gh_eq_p(value, gh_symbol2scm("min-hp-percent"))) {
	    condition->MinHpPercent = gh_scm2int(gh_car(list));
	    list = gh_cdr(list);
	} else if (gh_eq_p(value, gh_symbol2scm("max-hp-percent"))) {
	    condition->MaxHpPercent = gh_scm2int(gh_car(list));
	    list = gh_cdr(list);
	} else if (gh_eq_p(value, gh_symbol2scm("min-mana-percent"))) {
	    condition->MinManaPercent = gh_scm2int(gh_car(list));
	    list = gh_cdr(list);
	} else if (gh_eq_p(value, gh_symbol2scm("max-mana-percent"))) {
	    condition->MaxManaPercent = gh_scm2int(gh_car(list));
	    list = gh_cdr(list);
	} else if (gh_eq_p(value, gh_symbol2scm("max-slow-ticks"))) {
	    condition->MaxSlowTicks = gh_scm2int(gh_car(list));
	    list = gh_cdr(list);
	} else if (gh_eq_p(value, gh_symbol2scm("max-haste-ticks"))) {
	    condition->MaxHasteTicks = gh_scm2int(gh_car(list));
	    list = gh_cdr(list);
	} else if (gh_eq_p(value, gh_symbol2scm("max-bloodlust-ticks"))) {
	    condition->MaxBloodlustTicks = gh_scm2int(gh_car(list));
	    list = gh_cdr(list);
	} else if (gh_eq_p(value, gh_symbol2scm("max-invisibility-ticks"))) {
	    condition->MaxInvisibilityTicks = gh_scm2int(gh_car(list));
	    list = gh_cdr(list);
	} else if (gh_eq_p(value, gh_symbol2scm("max-invincibility-ticks"))) {
	    condition->MaxInvincibilityTicks = gh_scm2int(gh_car(list));
	    list = gh_cdr(list);
	} else {
	    for (i = 0; i < NumberBoolFlag; i++) { // User defined flags
	        if (gh_eq_p(value, gh_symbol2scm(BoolFlagName[i]))) {
	            condition->BoolFlag[i] = Scm2Condition(gh_car(list));
                    list = gh_cdr(list);
	            break;
                }
	    }
	    if (i != NumberBoolFlag) {
	        continue;
	    }
	    errl("Unsuported condition tag", value);
	}
    }
}
#elif defined(USE_LUA)
#endif

/*
**	Parse the Condition for spell.
**	
**	@param list		SCM object to parse
**	@param autocast		pointer to autocast to fill with data.
**
**	@notes: autocast must be allocated. All data already in is LOST.
*/
#if defined(USE_GUILE) || defined(USE_SIOD)
local void CclSpellAutocast(SCM list, AutoCastInfo* autocast)
{
    SCM value;

    DebugCheck(!list);
    DebugCheck(!autocast);

    while (!gh_null_p(list)) {
	value = gh_car(list);
	list = gh_cdr(list);
	if (gh_eq_p(value, gh_symbol2scm("range"))) {
	    autocast->Range = gh_scm2int(gh_car(list));
	    list = gh_cdr(list);
	} else if (gh_eq_p(value, gh_symbol2scm("combat"))) {
	    autocast->Combat = Scm2Condition(gh_car(list));
	    list = gh_cdr(list);
	} else if (gh_eq_p(value, gh_symbol2scm("condition"))) {
	    if (!autocast->Condition) {
		autocast->Condition = (ConditionInfo*)malloc(sizeof(ConditionInfo));
	    }
	    CclSpellCondition(gh_car(list), autocast->Condition);
	    list = gh_cdr(list);
	} else {
	    errl("Unsupported autocast tag", value);
	}
    }
}
#elif defined(USE_LUA)
#endif

/**
**	Parse Spell.
**
**	@param list	List describing Spell.
*/
#if defined(USE_GUILE) || defined(USE_SIOD)
local SCM CclDefineSpell(SCM list)
{
    char* identname;
    char* str;
    SpellType* spell;
    SCM value;
    SCM sublist;
    SpellActionType* act;

    identname = gh_scm2newstr(gh_car(list), NULL);
    list = gh_cdr(list);
    spell = SpellTypeByIdent(identname);
    if (spell != NULL) {
    	DebugLevel0Fn("Redefining spell-type `%s'\n" _C_ identname);
	free(identname);
    } else {
	SpellTypeTable = realloc(SpellTypeTable, (1 + SpellTypeCount) * sizeof(SpellType));
	spell = &SpellTypeTable[SpellTypeCount++];
	memset(spell, 0, sizeof(SpellType));
	spell->Ident = SpellTypeCount - 1;
	spell->IdentName = identname;
	spell->DependencyId = -1;
    }
    while (!gh_null_p(list)) {
	value = gh_car(list);
	list = gh_cdr(list);
	if (gh_eq_p(value, gh_symbol2scm("showname"))) {
	    if (spell->Name) { 
	    	free(spell->Name);
	    }
	    spell->Name = gh_scm2newstr(gh_car(list), NULL);
	    list = gh_cdr(list);
	} else if (gh_eq_p(value, gh_symbol2scm("manacost"))) {
	    spell->ManaCost = gh_scm2int(gh_car(list));
	    list = gh_cdr(list);
	} else if (gh_eq_p(value, gh_symbol2scm("range"))) {
	    if (gh_eq_p(gh_car(list), gh_symbol2scm("infinite"))) {
		spell->Range = INFINITE_RANGE;
	    } else {
		spell->Range = gh_scm2int(gh_car(list));
	    }
	    list = gh_cdr(list);
	} else if (gh_eq_p(value, gh_symbol2scm("repeat-cast"))) {
	    spell->RepeatCast=1;
	} else if (gh_eq_p(value, gh_symbol2scm("target"))) {
	    value = gh_car(list);
	    if (gh_eq_p(value, gh_symbol2scm("self"))) {
		spell->Target = TargetSelf;
	    } else if (gh_eq_p(value, gh_symbol2scm("unit"))) {
		spell->Target = TargetUnit;
	    } else if (gh_eq_p(value, gh_symbol2scm("position"))) {
		spell->Target = TargetPosition;
	    } else {
		errl("Unsupported spell target type tag", value);
	    }
	    list = gh_cdr(list);
	} else if (gh_eq_p(value, gh_symbol2scm("action"))) {
	    spell->Action = (SpellActionType*)malloc(sizeof(SpellActionType));
	    act=spell->Action;
	    memset(act, 0, sizeof(SpellActionType));
	    sublist=gh_car(list);
	    CclSpellAction(gh_car(sublist), act);
	    sublist=gh_cdr(sublist);
	    while (!gh_null_p(sublist)) {
		act->Next = (SpellActionType*)malloc(sizeof(SpellActionType));
		act=act->Next;
		memset(act, 0, sizeof(SpellActionType));
		CclSpellAction(gh_car(sublist), act);
		sublist=gh_cdr(sublist);
	    }
	    list = gh_cdr(list);
	} else if (gh_eq_p(value, gh_symbol2scm("condition"))) {
	    if (!spell->Condition) {
		spell->Condition = (ConditionInfo*)malloc(sizeof(ConditionInfo));
	    }
	    CclSpellCondition(gh_car(list), spell->Condition);
	    list = gh_cdr(list);
	} else if (gh_eq_p(value, gh_symbol2scm("autocast"))) {
	    if (!spell->AutoCast) {
		spell->AutoCast = (AutoCastInfo*)malloc(sizeof(AutoCastInfo));
		memset(spell->AutoCast, 0, sizeof(AutoCastInfo));
	    }
	    CclSpellAutocast(gh_car(list), spell->AutoCast);
	    list = gh_cdr(list);
	} else if (gh_eq_p(value, gh_symbol2scm("ai-cast"))) {
	    if (!spell->AICast) {
		spell->AICast = (AutoCastInfo*)malloc(sizeof(AutoCastInfo));
		memset(spell->AICast, 0, sizeof(AutoCastInfo));
	    }
	    CclSpellAutocast(gh_car(list), spell->AICast);
	    list = gh_cdr(list);
	} else if (gh_eq_p(value, gh_symbol2scm("sound-when-cast"))) {
	    //  Free the old name, get the new one
	    free(spell->SoundWhenCast.Name);
	    spell->SoundWhenCast.Name = gh_scm2newstr(gh_car(list), 0);
	    spell->SoundWhenCast.Sound = SoundIdForName(spell->SoundWhenCast.Name);
	    //  Check for sound.
	    if (!spell->SoundWhenCast.Sound) {
		free(spell->SoundWhenCast.Name);
		spell->SoundWhenCast.Name = 0;
	    }
	    list = gh_cdr(list);
	} else if (gh_eq_p(value, gh_symbol2scm("missile-when-cast"))) {
	    str = gh_scm2newstr(gh_car(list), NULL);
	    spell->Missile = MissileTypeByIdent(str);
	    if (spell->Missile == NULL) {
		DebugLevel0Fn("in spell-type '%s' : missile %s does not exist\n" _C_
		    spell->Name _C_ str);
	    }
	    free(str);
	    list = gh_cdr(list);
	} else if (gh_eq_p(value, gh_symbol2scm("depend-upgrade"))) {
	    str = gh_scm2newstr(gh_car(list), NULL);
	    spell->DependencyId = UpgradeIdByIdent(str);
	    free(str);
	    if (spell->DependencyId == -1) {
		errl("Bad upgrade name", gh_car(list));
	    }
	    list = gh_cdr(list);
	} else {
	    errl("Unsupported tag", value);
	}
    }
    return SCM_UNSPECIFIED;
}
#elif defined(USE_LUA)
#endif

/**
**	Register CCL features for Spell.
*/
global void SpellCclRegister(void)
{
#if defined(USE_GUILE) || defined(USE_SIOD)
    gh_new_procedureN("define-spell", CclDefineSpell);
#endif
}

/*
**	Save a spell action to a file.
**
** 	@param file	File pointer to save to
**	@param action	Pointer to action to save.
*/
local void SaveSpellAction(CLFile *file,SpellActionType* action)
{
    SpellActionMissileLocation * loc;
    if (action->CastFunction == CastAreaBombardment) {
	CLprintf(file, "(area-bombardment fields %d shards %d damage %d start-offset-x %d start-offset-y %d)",
		action->Data.AreaBombardment.Fields,
		action->Data.AreaBombardment.Shards,
		action->Data.AreaBombardment.Damage,
		action->Data.AreaBombardment.StartOffsetX,
		action->Data.AreaBombardment.StartOffsetY);
    } else if (action->CastFunction == CastAreaAdjustVitals) {
	CLprintf(file, "(area-adjust-vitals");
	if (action->Data.AreaAdjustVitals.HP) {
	    CLprintf(file, " hit-points %d", action->Data.AdjustVitals.HP);
	}
	if (action->Data.AreaAdjustVitals.Mana) {
	    CLprintf(file, " mana-points %d", action->Data.AdjustVitals.Mana);
	}
	CLprintf(file, ")\n");
    } else if (action->CastFunction == CastSpawnMissile) {
	CLprintf(file, "(spawn-missile delay %d ttl %d damage %d ",
		action->Data.SpawnMissile.Delay,
		action->Data.SpawnMissile.TTL,
		action->Data.SpawnMissile.Damage);
	//
	//	Save start-point
	//
	loc=&action->Data.SpawnMissile.StartPoint;
	CLprintf(file, "start-point (base ");
	if (loc->Base==LocBaseCaster) {
	    CLprintf(file, "caster");
	} else {
	    CLprintf(file, "target");
	}
	CLprintf(file, " add-x %d add-y %d add-rand-x %d add-rand-y %d) ",
		loc->AddX,loc->AddY,loc->AddRandX,loc->AddRandY);
	//
	//	Save end-point
	//
	loc=&action->Data.SpawnMissile.EndPoint;
	CLprintf(file, "end-point (base ");
	if (loc->Base==LocBaseCaster) {
	    CLprintf(file, "caster");
	} else {
	    CLprintf(file, "target");
	}
	CLprintf(file, " add-x %d add-y %d add-rand-x %d add-rand-y %d)",
		loc->AddX,loc->AddY,loc->AddRandX,loc->AddRandY);
	CLprintf(file, ")");
    } else if (action->CastFunction == CastAdjustVitals) {
	CLprintf(file, "(adjust-vitals");
	if (action->Data.AdjustVitals.HP) {
	    CLprintf(file, " hit-points %d", action->Data.AdjustVitals.HP);
	}
	if (action->Data.AdjustVitals.Mana) {
	    CLprintf(file, " mana-points %d", action->Data.AdjustVitals.Mana);
	}
	if (action->Data.AdjustVitals.MaxMultiCast) {
	    CLprintf(file, " max-multi-cast %d", action->Data.AdjustVitals.MaxMultiCast);
	}
	CLprintf(file, ")\n");
    } else if (action->CastFunction == CastSummon) {
	CLprintf(file, "(summon unit-type %s time-to-live %d",
		action->Data.Summon.UnitType->Ident,
		action->Data.Summon.TTL);
	if (action->Data.Summon.RequireCorpse) {
	    CLprintf(file, " require-corpse ");
	}
	CLprintf(file, ")\n");
    } else if (action->CastFunction == CastDemolish) {
	CLprintf(file, "(demolish range %d damage %d)\n",
		action->Data.Demolish.Range,
		action->Data.Demolish.Damage);
    } else if (action->CastFunction == CastAdjustBuffs) {
	CLprintf(file, "(adjust-buffs");
	if (action->Data.AdjustBuffs.HasteTicks != BUFF_NOT_AFFECTED) {
	    CLprintf(file, " haste-ticks %d", action->Data.AdjustBuffs.HasteTicks);
	}
	if (action->Data.AdjustBuffs.SlowTicks != BUFF_NOT_AFFECTED) {
	    CLprintf(file, " slow-ticks %d", action->Data.AdjustBuffs.SlowTicks);
	}
	if (action->Data.AdjustBuffs.BloodlustTicks != BUFF_NOT_AFFECTED) {
	    CLprintf(file, " bloodlust-ticks %d", action->Data.AdjustBuffs.BloodlustTicks);
	}
	if (action->Data.AdjustBuffs.InvisibilityTicks != BUFF_NOT_AFFECTED) {
	    CLprintf(file, " invisibility-ticks %d", action->Data.AdjustBuffs.InvisibilityTicks);
	}
	if (action->Data.AdjustBuffs.InvincibilityTicks != BUFF_NOT_AFFECTED) {
	    CLprintf(file, " invincibility-ticks %d", action->Data.AdjustBuffs.InvincibilityTicks);
	}
	CLprintf(file, ")");
    } else if (action->CastFunction == CastPolymorph) {
	CLprintf(file, "(polymorph new-form %s)",
		action->Data.Polymorph.NewForm->Ident);
    } else if (action->CastFunction == CastSpawnPortal) {
	CLprintf(file, "(spawn-portal portal-type %s)",
		action->Data.SpawnPortal.PortalType->Ident);
    } 
}

/*
**	Save a spell action to a file.
**
** 	@param file	File pointer to save to
**	@param action	Pointer to action to save.
*/
local void SaveSpellCondition(CLFile *file, ConditionInfo* condition)
{
    char condstrings [3][10] = {
	"true",			/// CONDITION_TRUE
	"false",		/// CONDITION_FALSE
	"only"			/// CONDITION_ONLY
    };
    int i;
    DebugCheck(!file);
    DebugCheck(!condition);

    CLprintf(file, "( ");
    //
    //	First save data related to flags.
    //	NOTE: (int) is there to keep compilers happy.
    //
    if (condition->Coward != CONDITION_TRUE) {
	CLprintf(file, "coward %s ", condstrings[(int)condition->Coward]);
    }
    if (condition->Alliance != CONDITION_TRUE) {
	CLprintf(file, "alliance %s ", condstrings[(int)condition->Alliance]);
    }
    if (condition->Building != CONDITION_TRUE) {
	CLprintf(file, "building %s ", condstrings[(int)condition->Building]);
    }
    if (condition->TargetSelf != CONDITION_TRUE) {
	CLprintf(file, "self %s ", condstrings[(int)condition->TargetSelf]);
    }
    for (i = 0; i < NumberBoolFlag; i++) { // User defined flags
        if (condition->BoolFlag[i] != CONDITION_TRUE) {
            CLprintf(file, "%s %s ",
                BoolFlagName[i], condstrings[(int)condition->BoolFlag[i]]);
        }
    }
    //
    //	Min/Max vital percents
    //
    CLprintf(file, "min-hp-percent %d ", condition->MinHpPercent);
    CLprintf(file, "max-hp-percent %d ", condition->MaxHpPercent);
    CLprintf(file, "min-mana-percent %d ", condition->MinManaPercent);
    CLprintf(file, "max-mana-percent %d ", condition->MaxManaPercent);
    //
    //	Max buff ticks stuff
    //
    CLprintf(file, "max-slow-ticks %d ", condition->MaxSlowTicks);
    CLprintf(file, "max-haste-ticks %d ", condition->MaxHasteTicks);
    CLprintf(file, "max-bloodlust-ticks %d ", condition->MaxBloodlustTicks);
    CLprintf(file, "max-invisibility-ticks %d ", condition->MaxInvisibilityTicks);
    CLprintf(file, "max-invincibility-ticks %d ", condition->MaxInvincibilityTicks);
    //
    //	The end.
    //
    CLprintf(file, ")\n");
}

/*
**	Save autocast info to a CCL file	 
**
**	@param file	The file to save to.
**	@param autocast	Auocastinfo to save.
*/
void SaveSpellAutoCast(CLFile* file, AutoCastInfo* autocast)
{
    char condstrings [3][10] = {
	"true",			/// CONDITION_TRUE
	"false",		/// CONDITION_FALSE
	"only"			/// CONDITION_ONLY
    };
    
    CLprintf(file, "( range %d ", autocast->Range);
    if (autocast->Combat != CONDITION_TRUE) {
	CLprintf(file, "combat %s ", condstrings[(int)autocast->Combat]);
    }
    if (autocast->Condition) {
	CLprintf(file, " condition ");
	SaveSpellCondition(file, autocast->Condition);
    }
    CLprintf(file, " )\n");
}

/*
**	Save spells to a CCL file.
**	
**	@param file	The file to save to.
*/
global void SaveSpells(CLFile* file)
{
    SpellType* spell;
    SpellActionType* act;

    DebugCheck(!file);
    
    for (spell = SpellTypeTable; spell < SpellTypeTable + SpellTypeCount; ++spell) {
	CLprintf(file, "(define-spell\n");
	//
	//  Misc small stuff.
	//
	CLprintf(file, "    \"%s\"\n", spell->IdentName);
	CLprintf(file, "    'showname \"%s\"\n", spell->Name);
	CLprintf(file, "    'manacost %d\n", spell->ManaCost);
	if (spell->RepeatCast) {
	    CLprintf(file, "    'repeat-cast\n");
	}
	CLprintf(file, "    'manacost %d\n", spell->ManaCost);
	CLprintf(file, "    'range %d\n", spell->Range);
	if (spell->SoundWhenCast.Name) { 
	    CLprintf(file, "    'sound-when-cast \"%s\"\n", spell->SoundWhenCast.Name);
	}
	if (spell->Missile) {
	    CLprintf(file, "    'missile-when-cast \"%s\"\n", spell->Missile->Ident);
	}
	//
	//  Target type.
	//
	CLprintf(file, "    'target '");
	switch (spell->Target) {
	    case TargetSelf:
		CLprintf(file, "self\n");
		break;
	    case TargetPosition:
		CLprintf(file, "position\n");
		break;
	    case TargetUnit:
		CLprintf(file, "unit\n");
		break;
	    default:
		DebugLevel0("Bad spell, can't save.\n");
		DebugCheck(1);
	}
	//
	//  Save the action(effect of the spell)
	//
	CLprintf(file, "    'action '(\n");
	act=spell->Action;
	while (act) {
	    CLprintf(file,"        ");
	    SaveSpellAction(file, act);
	    CLprintf(file,"\n");
	    act=act->Next;
	}
	CLprintf(file, ")\n");
	//
	//  Save conditions
	//
	if (spell->Condition) {
	    CLprintf(file, "    'condition '");
	    SaveSpellCondition(file, spell->Condition);
	    CLprintf(file, "\n");
	}
	//
	//  Save own unit autocast
	//
	if (spell->AutoCast) {
	    CLprintf(file, "    'autocast '");
	    SaveSpellAutoCast(file, spell->AutoCast);
	    CLprintf(file, "\n");
	}
	//
	//  Save AI autocast.
	//
	if (spell->AICast) {
	    CLprintf(file, "    'ai-cast '");
	    SaveSpellAutoCast(file, spell->AICast);
	    CLprintf(file, "\n");
	}
	CLprintf(file, ")\n");
    }
}

//@}
