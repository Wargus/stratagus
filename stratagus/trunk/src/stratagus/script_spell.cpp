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

// **************************************************************************
//		Generic condition parsers for autocast for spell.
// **************************************************************************

/**
**	To pushback the condition \p cond to the list \p begin.
**	@param	begin : begin of the list.
**	@param	cond : condition to add.
*/
local void pushback_condition(t_Conditions **begin, t_Conditions *cond)
{
    t_Conditions *next;

    DebugCheck(!begin);
    DebugCheck(!cond);

    if (*begin == NULL) {
	*begin = cond;
	return ;
    }
    for(next = *begin; next->next != NULL; next = next->next) {
	next->next = cond;
    }
}

/**
**	Pointer on function to parse 'condition.
**	@param	spellName : name of spell modified.
**	@param	id : last keyword recognized.
**	@param	list : args for \p id.
**	@param	generic : list for generic conditions.
**	@param	specific : list for specific conditions.
*/
typedef void f_ccl_condition(const char *spellname, const char *id, SCM list,
    t_Conditions **generic, t_Conditions **specific);

/**
**	Parsing true or false.
**	factorisation of code.
**	@param	SpellName : Name of spell.
**	@param	id : last keywork.
**	@param	expectvalue : OUTPUT <- 1 if true 0 else
*/
local int ccl_true(const char *spellname, const char *id, int *expectvalue,
    SCM value)
{
    DebugCheck(!spellname);
    DebugCheck(!id);
    DebugCheck(!expectvalue);
    
    *expectvalue = 0;
    if (gh_eq_p(value, gh_symbol2scm("true"))) {
	*expectvalue = 1;
	return 1;
    }
    if (gh_eq_p(value, gh_symbol2scm("false"))) {
	return 1;
    }
// Warning user : Unknown flag
    DebugLevel0Fn("In spell-type %s : %s : %s\n"
	    _C_ spellname _C_ id _C_ "Argument must be true or false");
    return 0;
}

/**
**	Parse the Condition for spell.
**	list = #t #flagtype
*/
local void ccl_UnitTypeFlag(const char *spellname, const char *id, SCM list,
    t_Conditions **generic, t_Conditions **specific)
{
    SCM value;
    t_Conditions *cond;
    struct {
	const char *id;
	int flag;
    } parser[] = {
	{"building", flag_building},
	{"coward", flag_coward},
	{"canattack", flag_canattack},
	{"organic", flag_organic},
	{"undead", flag_isundead},
	{NULL, 0}
    };
    int i;

    DebugCheck(!id);
    DebugCheck(!spellname);
    DebugCheck(!generic);
    DebugCheck(!specific);


    cond = malloc(sizeof(*cond));
    memset(cond, 0, sizeof(*cond));
    value = gh_car(list);
    if (!ccl_true(spellname, id, &cond->expectvalue, value)) {
	free(cond);
	return ;
    }
    list = gh_cdr(list);
    value = gh_car(list);
    for (i = 0; parser[i].id != NULL; i++) {
	if (gh_eq_p(value, gh_symbol2scm((char *)parser[i].id))) {
	    cond->u.flag = parser[i].flag;
	    break;
	}
    }
    if (parser[i].id == NULL) {
	DebugLevel0Fn("FIXME : WARNING : unknow flag");
	free(cond);
	return ;
    }
    cond->f.specific = CheckUnitTypeFlag;
    pushback_condition(specific, cond);
}

/**
**	Parse the Condition for spell.
**	list = #t #f_flag #n
*/
local void ccl_DurationEffect(const char *spellname, const char *id, SCM list,
    t_Conditions **generic, t_Conditions **specific)
{
    SCM value;
    t_Conditions *cond;
    struct {
	const char *id;
	int flag;
    } parser[] = {
	{"bloodlust", flag_bloodlust},
	{"flameshield", flag_flameshield},
	{"haste", flag_haste},
	{"invisibility", flag_invisibility},
	{"slow", flag_slow},
	{"unholyarmor", flag_unholyarmor},
	{"HP", flag_HP},
	{"mana", flag_Mana},
	{"HP_percent", flag_HP_percent},
	{"mana_percent", flag_Mana_percent},
	{NULL, 0}
    };
    int i;

    DebugCheck(!id);
    DebugCheck(!spellname);
    DebugCheck(!generic);
    DebugCheck(!specific);

    cond = malloc(sizeof(*cond));
    memset(cond, 0, sizeof(*cond));
    value = gh_car(list);
    if (!ccl_true(spellname, id, &cond->expectvalue, value)) {
	free(cond);
	return ;
    }
    list = gh_cdr(list);
    value = gh_car(list);
    if (!gh_eq_p(value, gh_symbol2scm("flag"))) {
	DebugLevel0Fn("FIXME : WARNING : unknow flag");
	free(cond);
	return ;
    }
    list = gh_cdr(list);
    value = gh_car(list);
    for (i = 0; parser[i].id != NULL; i++) {
	if (gh_eq_p(value, gh_symbol2scm((char *)parser[i].id))) {
	    cond->u.durationeffect.flag = parser[i].flag;
	    break;
	}
    }
    if (parser[i].id == NULL) {
	DebugLevel0Fn("FIXME : WARNING : unknow flag");
	// FIXME : WARNING.
	free(cond);
	return ;
    }
    list = gh_cdr(list);
    value = gh_car(list);
    if (!gh_eq_p(value, gh_symbol2scm("value"))) {
	DebugLevel0Fn("FIXME : WARNING : unknow flag");
	free(cond);
	return ;
    }
    list = gh_cdr(list);
    value = gh_car(list);
    cond->u.durationeffect.ttl = gh_scm2int(value);
    cond->f.specific = CheckUnitDurationEffect;
    pushback_condition(specific, cond);
}

/**
**	For enemy presence
**	list = #t range #n
*/
void ccl_enemy_presence(const char *spellname, const char *id, SCM list,
    t_Conditions **generic, t_Conditions **specific)
{
    SCM value;
    t_Conditions *cond;

    DebugCheck(!id);
    DebugCheck(!spellname);
    DebugCheck(!generic);
    DebugCheck(!specific);

    cond = malloc(sizeof(*cond));
    memset(cond, 0, sizeof(*cond));
    value = gh_car(list);
    if (!ccl_true(spellname, id, &cond->expectvalue, value)) {
	free(cond);
	return ;
    }
    list = gh_cdr(list);
    value = gh_car(list);
    if (!gh_eq_p(value, gh_symbol2scm("range"))) {
	DebugLevel0Fn("FIXME : WARNING : unknow flag");
	free(cond);
	return ;
    }
    list = gh_cdr(list);
    value = gh_car(list);
    cond->f.generic = CheckEnemyPresence; // In spell.c
    cond->u.range = gh_scm2int(value);
    if (cond->u.range <= 0 && cond->u.range != -1/*no limit*/) {
	DebugLevel0Fn("FIXME : WARNING : range <= 0");
	// Warn : range <= 0 have no sens, must be strict positive. or = -1
	free(cond);
	return ;
    }
    pushback_condition(generic, cond);
}

/**
**	For enemy presence
**	list = #t
*/
void ccl_alliance(const char *spellname, const char *id, SCM list,
    t_Conditions **generic, t_Conditions **specific)
{
    SCM value;
    t_Conditions *cond;

    DebugCheck(!id);
    DebugCheck(!spellname);
    DebugCheck(!generic);
    DebugCheck(!specific);

    cond = malloc(sizeof(*cond));
    memset(cond, 0, sizeof(*cond));
    value = /*gh_car(*/list/*)*/;
    if (!ccl_true(spellname, id, &cond->expectvalue, value)) {//	warn user
	free(cond);
	return ;
    }
    cond->f.specific = CheckAllied;
    pushback_condition(specific, cond);
}

/**
**	Parse the Condition for spell.
*/
local void ccl_spell_all_condition(const char *spellname, const char *id,
    SCM list, t_Conditions **generic, t_Conditions **specific)
{
    int i;
    struct {
	const char *id;
	f_ccl_condition *f;
    } parser[] = {
	{"UnitTypeflag", ccl_UnitTypeFlag},
	{"DurationEffect", ccl_DurationEffect},
	{"Enemypresence", ccl_enemy_presence},
	{"Alliance", ccl_alliance},
	{ NULL, 0}	
    };
    SCM value;

    DebugCheck(!id);
    DebugCheck(!spellname);
    DebugCheck(!generic);
    DebugCheck(!specific);

    for (; !gh_null_p(list); list = gh_cdr(list)) {
	value = gh_car(list);
	for (i = 0; parser[i].id != NULL; i++) {
	    if (gh_eq_p(value, gh_symbol2scm((char *) parser[i].id))) {
		list = gh_cdr(list);
		parser[i].f(spellname, parser[i].id, gh_car(list), generic, specific);
		break;
	    }
	}
	DebugCheck(!parser[i].id);
    }
}

/**
**	Parse the Condition for spell.
*/
local void ccl_spell_condition(const char *id, SCM list, SpellType *spell)
{
    DebugCheck(!id);
    DebugCheck(!spell);

    ccl_spell_all_condition(spell->IdentName, id, list,
	&spell->Condition_generic,&spell->Condition_specific);
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
    ccl_spell_all_condition(spell->IdentName, "Autocast conditions", value,
	    &spell->AutoCast->Condition_generic,&spell->AutoCast->Condition_specific);
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
	    spell->Range=gh_scm2int(gh_car(list));
	    list=gh_cdr(list);
	} else if (gh_eq_p(value,gh_symbol2scm("target"))) {
	    value=gh_car(list);
	    if (gh_eq_p(value,gh_symbol2scm("none"))) {
		spell->Target=TargetNone;
	    } else if (gh_eq_p(value,gh_symbol2scm("self"))) {
		spell->Target=TargetSelf;
	    } else if (gh_eq_p(value,gh_symbol2scm("unit"))) {
		spell->Target=TargetSelf;
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
	    ccl_spell_condition("condition",gh_car(list),spell);
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
