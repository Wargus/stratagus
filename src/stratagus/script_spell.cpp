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
//		Direct affectation for spell
// **************************************************************************

/**
**      Parse the dependency of spell.
**      list = (upgrade "upgradename")
*/
local void ccl_spell_dependency(const char *id, SCM list, SpellType *spell)
{
    char *dependencyName;
    SCM value;
    int dependencyId;

    DebugCheck(!id);
    DebugCheck(!spell);

    dependencyName = NULL;
    dependencyId = -1;

    value = gh_car(list);

    if (!gh_eq_p(value, gh_symbol2scm("upgrade"))) {
	return;
    }
    list = gh_cdr(list);
    value = gh_car(list);

    dependencyName = gh_scm2newstr(value, NULL);
    dependencyId = UpgradeIdByIdent(dependencyName);
    if (dependencyId == -1) {
	// warn user
	DebugLevel0Fn("Bad upgrade-name '%s'\n" _C_ dependencyName);
	free(dependencyName);
	return ;
    }
    spell->DependencyId = dependencyId;
    free(dependencyName);
}


// **************************************************************************
//		Action parsers for spellAction
// **************************************************************************

/**
**	Parse the action for spell.
**	list = (action-type lots-of-parameters).
*/
local void CclParseSpellAction(SCM list, SpellType* spell, SpellActionType *spellaction)
{
    char* str;
    SCM	value = list;

    DebugCheck (spellaction == NULL);

    value = gh_car(value);

    value=gh_car(list);
    list=gh_cdr(list);

    memset(spellaction, 0, sizeof(*spellaction));
    if (gh_eq_p(value,gh_symbol2scm("area-bombardment"))) {
	spell->CastFunction=CastAreaBombardment;
	while (!gh_null_p(list)) {
	    value = gh_car(list);
	    list=gh_cdr(list);
	    if (gh_eq_p(value, gh_symbol2scm("fields"))) {
		spellaction->AreaBombardment.Fields = gh_scm2int(gh_car(list));
		list = gh_cdr(list);
	    } else if (gh_eq_p(value, gh_symbol2scm("shards"))) {
		spellaction->AreaBombardment.Shards = gh_scm2int(gh_car(list));
		list = gh_cdr(list);
	    } else if (gh_eq_p(value, gh_symbol2scm("damage"))) {
		spellaction->AreaBombardment.Damage = gh_scm2int(gh_car(list));
		list = gh_cdr(list);
	    } else if (gh_eq_p(value, gh_symbol2scm("start-offset-x"))) {
		spellaction->AreaBombardment.StartOffsetX = gh_scm2int(gh_car(list));
		list = gh_cdr(list);
	    } else if (gh_eq_p(value, gh_symbol2scm("start-offset-y"))) {
		spellaction->AreaBombardment.StartOffsetY = gh_scm2int(gh_car(list));
		list = gh_cdr(list);
	    } else {
		errl("Unsupported area-bombardment tag",value);
	    }
	}
    } else if (gh_eq_p(value,gh_symbol2scm("flame-shield"))) {
	spell->CastFunction=CastFlameShield;
	while (!gh_null_p(list)) {
	    value = gh_car(list);
	    list=gh_cdr(list);
	    if (gh_eq_p(value, gh_symbol2scm("duration"))) {
		spellaction->FlameShield.TTL = gh_scm2int(gh_car(list));
		list = gh_cdr(list);
		/// FIXME:damage, missiles, rotation speed?
	    } else {
		errl("Unsupported flame-shield tag",value);
	    }
	}
    } else if (gh_eq_p(value,gh_symbol2scm("fireball"))) {
	spell->CastFunction=CastFireball;
	while (!gh_null_p(list)) {
	    value = gh_car(list);
	    list=gh_cdr(list);
	    if (gh_eq_p(value, gh_symbol2scm("damage"))) {
		spellaction->Fireball.Damage = gh_scm2int(gh_car(list));
		list = gh_cdr(list);
	    } else if (gh_eq_p(value, gh_symbol2scm("ttl"))) {
		spellaction->Fireball.TTL = gh_scm2int(gh_car(list));
		list = gh_cdr(list);
	    } else {
		errl("Unsupported fireball tag",value);
	    }
	}
    } else if (gh_eq_p(value,gh_symbol2scm("runes"))) {
	spell->CastFunction=CastRunes;
	while (!gh_null_p(list)) {
	    value = gh_car(list);
	    list=gh_cdr(list);
	    if (gh_eq_p(value, gh_symbol2scm("damage"))) {
		spellaction->Fireball.Damage = gh_scm2int(gh_car(list));
		list = gh_cdr(list);
	    } else if (gh_eq_p(value, gh_symbol2scm("ttl"))) {
		spellaction->Fireball.TTL = gh_scm2int(gh_car(list));
		list = gh_cdr(list);
	    } else {
		errl("Unsupported runes tag",value);
	    }
	}
    } else if (gh_eq_p(value,gh_symbol2scm("whirlwind"))) {
	spell->CastFunction=CastWhirlwind;
	while (!gh_null_p(list)) {
	    value = gh_car(list);
	    list=gh_cdr(list);
	    if (gh_eq_p(value, gh_symbol2scm("duration"))) {
		spellaction->Whirlwind.TTL = gh_scm2int(gh_car(list));
		list = gh_cdr(list);
	    } else {
		errl("Unsupported runes tag",value);
	    }
	}
    } else if (gh_eq_p(value,gh_symbol2scm("adjust-buffs"))) {
	spell->CastFunction=CastAdjustBuffs;
	spellaction->AdjustBuffs.HasteTicks=BUFF_NOT_AFFECTED;
	spellaction->AdjustBuffs.SlowTicks=BUFF_NOT_AFFECTED;
	spellaction->AdjustBuffs.BloodlustTicks=BUFF_NOT_AFFECTED;
	spellaction->AdjustBuffs.InvisibilityTicks=BUFF_NOT_AFFECTED;
	spellaction->AdjustBuffs.InvincibilityTicks=BUFF_NOT_AFFECTED;
	while (!gh_null_p(list)) {
	    value = gh_car(list);
	    list=gh_cdr(list);
	    if (gh_eq_p(value, gh_symbol2scm("haste-ticks"))) {
		spellaction->AdjustBuffs.HasteTicks = gh_scm2int(gh_car(list));
		list = gh_cdr(list);
	    } else if (gh_eq_p(value, gh_symbol2scm("slow-ticks"))) {
		spellaction->AdjustBuffs.SlowTicks  = gh_scm2int(gh_car(list));
		list = gh_cdr(list);
	    } else if (gh_eq_p(value, gh_symbol2scm("bloodlust-ticks"))) {
		spellaction->AdjustBuffs.BloodlustTicks  = gh_scm2int(gh_car(list));
		list = gh_cdr(list);
	    } else if (gh_eq_p(value, gh_symbol2scm("invisibility-ticks"))) {
		spellaction->AdjustBuffs.InvisibilityTicks  = gh_scm2int(gh_car(list));
		list = gh_cdr(list);
	    } else if (gh_eq_p(value, gh_symbol2scm("invincibility-ticks"))) {
		spellaction->AdjustBuffs.InvincibilityTicks  = gh_scm2int(gh_car(list));
		list = gh_cdr(list);
	    } else {
		errl("Unsupported adjust-buffs tag",value);
	    }
	}
    } else if (gh_eq_p(value,gh_symbol2scm("summon"))) {
	spell->CastFunction=CastSummon;
	while (!gh_null_p(list)) {
	    value = gh_car(list);
	    list = gh_cdr(list);
	    if (gh_eq_p(value, gh_symbol2scm("unit-type"))) {
		str = gh_scm2newstr(gh_car(list),0);
		spellaction->Summon.UnitType = UnitTypeByIdent(str);
		if (!spellaction->Summon.UnitType) {
		    spellaction->Summon.UnitType = 0;
		    DebugLevel0("unit type \"%s\" not found for summon spell.\n" _C_ str);
		}
		free(str);
		list = gh_cdr(list);
	    } else if (gh_eq_p(value, gh_symbol2scm("time-to-live"))) {
		spellaction->Summon.TTL = gh_scm2int(gh_car(list));
		list = gh_cdr(list);
	    } else {
		errl("Unsupported summon tag",value);
	    }
	}
    } else if (gh_eq_p(value,gh_symbol2scm("spawn-portal"))) {
	spell->CastFunction=CastSpawnPortal;
	while (!gh_null_p(list)) {
	    value = gh_car(list);
	    list = gh_cdr(list);
	    if (gh_eq_p(value, gh_symbol2scm("portal-type"))) {
		str = gh_scm2newstr(gh_car(list),0);
		spellaction->SpawnPortal.PortalType = UnitTypeByIdent(str);
		if (!spellaction->SpawnPortal.PortalType) {
		    spellaction->SpawnPortal.PortalType = 0;
		    DebugLevel0("unit type \"%s\" not found for spawn-portal.\n" _C_ str);
		}
		free(str);
		list = gh_cdr(list);
	    } else {
		errl("Unsupported spawn-portal tag",value);
	    }
	}
    } else if (gh_eq_p(value,gh_symbol2scm("raise-dead"))) {
	spell->CastFunction=CastRaiseDead;
	while (!gh_null_p(list)) {
	    value = gh_car(list);
	    list = gh_cdr(list);
	    if (gh_eq_p(value, gh_symbol2scm("unit-raised"))) {
		str = gh_scm2newstr(gh_car(list),0);
		spellaction->RaiseDead.UnitRaised = UnitTypeByIdent(str);
		if (!spellaction->RaiseDead.UnitRaised) {
		    spellaction->RaiseDead.UnitRaised = 0;
		    DebugLevel0("unit type \"%s\" not found for summon spell.\n" _C_ str);
		}
		free(str);
		list = gh_cdr(list);
	    } else if (gh_eq_p(value, gh_symbol2scm("time-to-live"))) {
		spellaction->RaiseDead.TTL = gh_scm2int(gh_car(list));
		list = gh_cdr(list);
	    } else {
		errl("Unsupported raise-dead tag",value);
	    }
	}
    } else if (gh_eq_p(value,gh_symbol2scm("polymorph"))) {
	spell->CastFunction=CastPolymorph;
	while (!gh_null_p(list)) {
	    value = gh_car(list);
	    list = gh_cdr(list);
	    if (gh_eq_p(value, gh_symbol2scm("new-form"))) {
		str = gh_scm2newstr(gh_car(list),0);
		spellaction->Summon.UnitType = UnitTypeByIdent(str);
		if (!spellaction->Summon.UnitType) {
		    spellaction->Summon.UnitType = 0;
		    DebugLevel0("unit type \"%s\" not found for summon spell.\n" _C_ str);
		}
		free(str);
		list = gh_cdr(list);
		//FIXME : temp polymorphs? hard to do.
	    } else {
		errl("Unsupported polymorph tag",value);
	    }
	}
    } else if (gh_eq_p(value,gh_symbol2scm("adjust-vitals"))) {
	spell->CastFunction=CastAdjustVitals;
	while (!gh_null_p(list)) {
	    value = gh_car(list);
	    list=gh_cdr(list);
	    if (gh_eq_p(value, gh_symbol2scm("hit-points"))) {
		spellaction->AdjustVitals.HP = gh_scm2int(gh_car(list));
		list = gh_cdr(list);
	    } else if (gh_eq_p(value, gh_symbol2scm("mana-points"))) {
		spellaction->AdjustVitals.Mana = gh_scm2int(gh_car(list));
		list = gh_cdr(list);
	    } else if (gh_eq_p(value, gh_symbol2scm("max-multi-cast"))) {
		spellaction->AdjustVitals.MaxMultiCast = gh_scm2int(gh_car(list));
		list = gh_cdr(list);
	    } else {
		errl("Unsupported adjust-vitals tag",value);
	    }
	}
    } else {
	errl("Unsupported action type", value);
    }
}

/**
**	Get a condition value from a scm object.
**	
**	@param 	value		scm value to convert.
**
**	@return CONDITION_TRUE, CONDITION_FALSE, CONDITION_ONLY or -1 on error.
*/
local char Scm2Condition(SCM value)
{
    if (gh_eq_p(value,gh_symbol2scm("true"))) {
	return CONDITION_TRUE;
    } else if (gh_eq_p(value,gh_symbol2scm("false"))) {
	return CONDITION_FALSE;
    } else if (gh_eq_p(value,gh_symbol2scm("only"))) {
	return CONDITION_ONLY;
    } else {
	errl("Bad condition result",value);
	return -1;
    }
}

/**
**	Parse the Condition for spell.
**	
**	@param list		SCM object to parse
**	@param condition	pointer to condition to fill with data.
**
**	@notes: conditions must be allocated. All data already in is LOST.
*/
local void CclSpellParseCondition(SCM list, ConditionInfo* condition)
{
    SCM value;

    //
    //	Initializations:
    //

    //	Set everything to 0:
    memset(condition,0,sizeof(ConditionInfo));
    //	Flags are defaulted to 0(CONDITION_TRUE)

    //	Initialize min/max stuff to values with no effect.
    condition->MinHpPercent=-10;
    condition->MaxHpPercent=1000;
    condition->MinManaPercent=-10;
    condition->MaxManaPercent=1000;
    //  Buffs too.
    condition->MaxHasteTicks=0xFFFFFFF;
    condition->MaxSlowTicks=0xFFFFFFF;
    condition->MaxBloodlustTicks=0xFFFFFFF;
    condition->MaxInvisibilityTicks=0xFFFFFFF;
    condition->MaxInvincibilityTicks=0xFFFFFFF;
    //  Now parse the list and set values.
    while (!gh_null_p(list)) {
	value = gh_car(list);
	list = gh_cdr(list);
	if (gh_eq_p(value,gh_symbol2scm("undead"))) {
	    condition->Undead=Scm2Condition(gh_car(list));
	    list=gh_cdr(list);
	} else if (gh_eq_p(value,gh_symbol2scm("organic"))) {
	    condition->Organic=Scm2Condition(gh_car(list));
	    list=gh_cdr(list);
	} else if (gh_eq_p(value,gh_symbol2scm("hero"))) {
	    condition->Hero=Scm2Condition(gh_car(list));
	    list=gh_cdr(list);
	} else if (gh_eq_p(value,gh_symbol2scm("coward"))) {
	    condition->Coward=Scm2Condition(gh_car(list));
	    list=gh_cdr(list);
	} else if (gh_eq_p(value,gh_symbol2scm("alliance"))) {
	    condition->Alliance=Scm2Condition(gh_car(list));
	    list=gh_cdr(list);
	} else if (gh_eq_p(value,gh_symbol2scm("building"))) {
	    condition->Building=Scm2Condition(gh_car(list));
	    list=gh_cdr(list);
	} else if (gh_eq_p(value,gh_symbol2scm("self"))) {
	    condition->TargetSelf=Scm2Condition(gh_car(list));
	    list=gh_cdr(list);
	} else if (gh_eq_p(value,gh_symbol2scm("min-hp-percent"))) {
	    condition->MaxHpPercent=gh_scm2int(gh_car(list));
	    list=gh_cdr(list);
	} else if (gh_eq_p(value,gh_symbol2scm("max-hp-percent"))) {
	    condition->MaxHpPercent=gh_scm2int(gh_car(list));
	    list=gh_cdr(list);
	} else if (gh_eq_p(value,gh_symbol2scm("min-mana-percent"))) {
	    condition->MinManaPercent=gh_scm2int(gh_car(list));
	    list=gh_cdr(list);
	} else if (gh_eq_p(value,gh_symbol2scm("max-mana-percent"))) {
	    condition->MaxManaPercent=gh_scm2int(gh_car(list));
	    list=gh_cdr(list);
	} else if (gh_eq_p(value,gh_symbol2scm("max-slow-ticks"))) {
	    condition->MaxSlowTicks=gh_scm2int(gh_car(list));
	    list=gh_cdr(list);
	} else if (gh_eq_p(value,gh_symbol2scm("max-haste-ticks"))) {
	    condition->MaxHasteTicks=gh_scm2int(gh_car(list));
	    list=gh_cdr(list);
	} else if (gh_eq_p(value,gh_symbol2scm("max-bloodlust-ticks"))) {
	    condition->MaxBloodlustTicks=gh_scm2int(gh_car(list));
	    list=gh_cdr(list);
	} else if (gh_eq_p(value,gh_symbol2scm("max-invisibility-ticks"))) {
	    condition->MaxInvisibilityTicks=gh_scm2int(gh_car(list));
	    list=gh_cdr(list);
	} else if (gh_eq_p(value,gh_symbol2scm("max-invincibility-ticks"))) {
	    condition->MaxInvincibilityTicks=gh_scm2int(gh_car(list));
	    list=gh_cdr(list);
	} else {
	    errl("Unsuported condition tag",value);
	}
    }
}

/**
**	FIXME: docu
*/
local void ccl_spell_autocast(const char *id, SCM list, SpellType *spell)
{
    SCM value;
    int range;

    DebugCheck(!id);
    DebugCheck(!spell);

    value = gh_car(list);
    if (!gh_eq_p(value, gh_symbol2scm("range"))) {
	return ;
    }

    list = gh_cdr(list);
    value = gh_car(list);
    range = gh_scm2int(value);
    if (range <= 0 && range != -1/*no limit*/) {
	// Warn : range <= 0 have no sens, must be strict positive. or = -1
	return ;
    }
    spell->AutoCast = malloc(sizeof(*spell->AutoCast));
    memset(spell->AutoCast, 0, sizeof(*spell->AutoCast));
    spell->AutoCast->Range = range;
    list = gh_cdr(list);
    value = gh_car(list);
    if (!gh_eq_p(value, gh_symbol2scm("condition"))) {
	return ;
    }
    list = gh_cdr(list);
    value = gh_car(list);
    spell->AutoCast->Condition=(ConditionInfo*)malloc(sizeof(ConditionInfo));
    CclSpellParseCondition(value,spell->AutoCast->Condition);
}

/**
**	Parse Spell.
**
**	@param list	List describing Spell.
*/
local SCM CclDefineSpell(SCM list)
{
    char *identname;
    char *str;
    SpellType *spell;
    SCM value;

    identname = gh_scm2newstr(gh_car(list), NULL);
    list = gh_cdr(list);
    spell = SpellTypeByIdent(identname);
    if (spell != NULL) {
    	DebugLevel0Fn("Redefining spell-type `%s'\n" _C_ identname);
	free(identname);
    } else {
	SpellTypeTable = realloc(SpellTypeTable,(1+SpellTypeCount)*sizeof(SpellType));
	spell = &SpellTypeTable[SpellTypeCount++];
	memset(spell,0,sizeof(SpellType));
	spell->Ident=SpellTypeCount-1;
	spell->IdentName=identname;
	spell->DependencyId = -1;
    }
    while (!gh_null_p(list)) {
	value = gh_car(list);
	list = gh_cdr(list);
	if (gh_eq_p(value,gh_symbol2scm("showname"))) {
	    if (spell->Name) { 
	    	free(spell->Name);
	    }
	    spell->Name=gh_scm2newstr(gh_car(list), NULL);
	    list=gh_cdr(list);
	} else if (gh_eq_p(value,gh_symbol2scm("manacost"))) {
	    spell->ManaCost=gh_scm2int(gh_car(list));
	    list=gh_cdr(list);
	} else if (gh_eq_p(value,gh_symbol2scm("range"))) {
	    if (gh_eq_p(gh_car(list),gh_symbol2scm("infinite"))) {
		spell->Range=INFINITE_RANGE;
	    } else {
		spell->Range=gh_scm2int(gh_car(list));
	    }
	    list=gh_cdr(list);
	} else if (gh_eq_p(value,gh_symbol2scm("target"))) {
	    value=gh_car(list);
	    if (gh_eq_p(value,gh_symbol2scm("none"))) {
		spell->Target=TargetNone;
	    } else if (gh_eq_p(value,gh_symbol2scm("self"))) {
		spell->Target=TargetSelf;
	    } else if (gh_eq_p(value,gh_symbol2scm("unit"))) {
		spell->Target=TargetUnit;
	    } else if (gh_eq_p(value,gh_symbol2scm("position"))) {
		spell->Target=TargetPosition;
	    } else {
		errl("Unsupported spell target type tag",value);
	    }
	    list=gh_cdr(list);
	} else if (gh_eq_p(value,gh_symbol2scm("action"))) {
	    spell->SpellAction=(SpellActionType*)malloc(sizeof(SpellActionType));
	    CclParseSpellAction(gh_car(list),spell,spell->SpellAction);
	    list=gh_cdr(list);
	} else if (gh_eq_p(value,gh_symbol2scm("condition"))) {
	    spell->Conditions=(ConditionInfo*)malloc(sizeof(ConditionInfo));
	    CclSpellParseCondition(gh_car(list),spell->Conditions);
	    list=gh_cdr(list);
	} else if (gh_eq_p(value,gh_symbol2scm("autocast"))) {
	    ccl_spell_autocast("autocast",gh_car(list),spell);
	    list=gh_cdr(list);
	} else if (gh_eq_p(value,gh_symbol2scm("sound-when-cast"))) {
	    //  Free the old name, get the new one
	    free(spell->SoundWhenCast.Name);
	    spell->SoundWhenCast.Name=gh_scm2newstr(gh_car(list),0);
	    spell->SoundWhenCast.Sound=SoundIdForName(spell->SoundWhenCast.Name);
	    //  Check for sound.
	    if (!spell->SoundWhenCast.Sound) {
		free(spell->SoundWhenCast.Name);
		spell->SoundWhenCast.Name=0;
	    }
	    list=gh_cdr(list);
	} else if (gh_eq_p(value,gh_symbol2scm("missile-when-cast"))) {
	    str = gh_scm2newstr(gh_car(list), NULL);
	    spell->Missile = MissileTypeByIdent(str);
	    if (spell->Missile == NULL) {
		DebugLevel0Fn("in spell-type '%s' : missile %s does not exist\n"
			_C_ spell->Name _C_ str);
	    }
	    free(str);
	    list=gh_cdr(list);
	} else if (gh_eq_p(value,gh_symbol2scm("depend"))) {
	    ccl_spell_dependency("depend", gh_car(list), spell);
	    list = gh_cdr(list);
	} else 
	{
	    errl("Unsupported tag", value);
	}
    }
    return SCM_UNSPECIFIED;
}

/**
**	Register CCL features for Spell.
*/
global void SpellCclRegister(void)
{
    gh_new_procedureN("define-spell", CclDefineSpell);
}

/**
**	Save spells to a CCL file.
**	
**	@param file	The file to save to.
*/
global void SaveSpells(CLFile *file)
{
    SpellType *spell;

    DebugCheck(!file);
    
    for (spell = SpellTypeTable; spell < SpellTypeTable + SpellTypeCount; ++spell) {
	CLprintf(file,"(define-spell\n");
	//
	//  Misc small stuff.
	//
	CLprintf(file,"    \"%s\"\n",spell->IdentName);
	CLprintf(file,"    'showname \"%s\"\n",spell->Name);
	CLprintf(file,"    'manacost %d\n",spell->ManaCost);
	CLprintf(file,"    'range %d\n",spell->Range);
	if (spell->SoundWhenCast.Name) { 
	    CLprintf(file,"    'sound-when-cast \"%s\"\n",spell->SoundWhenCast.Name);
	}
	if (spell->Missile) {
	    CLprintf(file,"    'missile-when-cast \"%s\"\n",spell->Missile->Ident);
	}
	//
	//  Target type.
	//
	CLprintf(file,"    'target '");
	switch (spell->Target) {
	    case TargetSelf:
		CLprintf(file,"self\n");
		break;
	    case TargetNone:
		CLprintf(file,"none\n");
		break;
	    case TargetPosition:
		CLprintf(file,"position\n");
		break;
	    case TargetUnit:
		CLprintf(file,"unit\n");
		break;
	    default:
		DebugLevel0("Bad spell, can't save.\n");
		DebugCheck(1);
	}
	//
	//  Save the action(effect of the spell)
	//
	CLprintf(file,"    'action");
	if (spell->CastFunction==CastAreaBombardment) {
	    CLprintf(file," '(area-bombardment fields %d shards %d damage %d start-offset-x %d start-offset-y %d)\n",
		    spell->SpellAction->AreaBombardment.Fields,
		    spell->SpellAction->AreaBombardment.Shards,
		    spell->SpellAction->AreaBombardment.Damage,
		    spell->SpellAction->AreaBombardment.StartOffsetX,
		    spell->SpellAction->AreaBombardment.StartOffsetY);
	} else if (spell->CastFunction==CastFireball) {
	    CLprintf(file," '(fireball ttl %d damage %d)\n",
		    spell->SpellAction->Fireball.TTL,
		    spell->SpellAction->Fireball.Damage);
	} else if (spell->CastFunction==CastAdjustVitals) {
	    CLprintf(file," '(adjust-vitals");
	    if (spell->SpellAction->AdjustVitals.HP) {
		CLprintf(file," hit-points %d",spell->SpellAction->AdjustVitals.HP);
	    }
	    if (spell->SpellAction->AdjustVitals.Mana) {
		CLprintf(file," mana-points %d",spell->SpellAction->AdjustVitals.Mana);
	    }
	    if (spell->SpellAction->AdjustVitals.MaxMultiCast) {
		CLprintf(file," max-multi-cast %d",spell->SpellAction->AdjustVitals.MaxMultiCast);
	    }
	    CLprintf(file,")\n");
	} else if (spell->CastFunction==CastSummon) {
	    CLprintf(file," '(summon unit-type %s time-to-live %d)\n",
		    spell->SpellAction->Summon.UnitType->Ident,
		    spell->SpellAction->Summon.TTL);
	} else if (spell->CastFunction==CastAdjustBuffs) {
	    CLprintf(file," '(adjust-buffs");
	    if (spell->SpellAction->AdjustBuffs.HasteTicks!=BUFF_NOT_AFFECTED) {
		CLprintf(file," haste-ticks %d",spell->SpellAction->AdjustBuffs.HasteTicks);
	    }
	    if (spell->SpellAction->AdjustBuffs.SlowTicks!=BUFF_NOT_AFFECTED) {
		CLprintf(file," slow-ticks %d",spell->SpellAction->AdjustBuffs.SlowTicks);
	    }
	    if (spell->SpellAction->AdjustBuffs.BloodlustTicks!=BUFF_NOT_AFFECTED) {
		CLprintf(file," bloodlust-ticks %d",spell->SpellAction->AdjustBuffs.BloodlustTicks);
	    }
	    if (spell->SpellAction->AdjustBuffs.InvisibilityTicks!=BUFF_NOT_AFFECTED) {
		CLprintf(file," invisibility-ticks %d",spell->SpellAction->AdjustBuffs.InvisibilityTicks);
	    }
	    if (spell->SpellAction->AdjustBuffs.InvincibilityTicks!=BUFF_NOT_AFFECTED) {
		CLprintf(file," invincibility-ticks %d",spell->SpellAction->AdjustBuffs.InvincibilityTicks);
	    }
	    CLprintf(file,")\n");
	} else if (spell->CastFunction==CastPolymorph) {
	    CLprintf(file," '(polymorph new-form %s)\n",
		    spell->SpellAction->Polymorph.NewForm->Ident);
	} else if (spell->CastFunction==CastRaiseDead) {
	    CLprintf(file," '(raise-dead unit-raised %s time-to-live %d)\n",
		    spell->SpellAction->RaiseDead.UnitRaised->Ident,
		    spell->SpellAction->RaiseDead.TTL);
	} else if (spell->CastFunction==CastFlameShield) {
	    CLprintf(file," '(flame-shield duration %d)\n",
		    spell->SpellAction->FlameShield.TTL);
	} else if (spell->CastFunction==CastRunes) {
	    CLprintf(file," '(runes ttl %d damage %d)\n",
		    spell->SpellAction->Runes.TTL,
		    spell->SpellAction->Runes.Damage);
	} else if (spell->CastFunction==CastSpawnPortal) {
	    CLprintf(file," '(spawn-portal portal-type %s)\n",
		    spell->SpellAction->SpawnPortal.PortalType->Ident);
	} else if (spell->CastFunction==CastDeathCoil) {
	    CLprintf(file," '(death-coil)\n");
	    // FIXME: more?
	} else if (spell->CastFunction==CastWhirlwind) {
	    CLprintf(file," '(whirlwind duration %d)\n",
		    spell->SpellAction->Whirlwind.TTL);
	    // FIXME: more?
	} 
	//
	//  FIXME: Save conditions
	//
	
	//
	//  FIXME: Save autocast and AI info
	//
	CLprintf(file,")\n");
    }
}

//@}
