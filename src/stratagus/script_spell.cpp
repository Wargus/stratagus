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
#include <assert.h>

#include "stratagus.h"
/*
#include "video.h"
#include "tileset.h"
#include "unittype.h"
*/
#include "spells.h"
#include "ccl_sound.h"
#include "ccl.h"

/**
**	pointer on function.
**	@param	id		: last keyword recognized.
**	@param	list	: list to be parsed. (just the args).
**	@param	spell	: spelltype to modify.
**	FIXME: remove all this cruft
*/
typedef void f_ccl_spell(const char *id, SCM list, SpellType	*spell);

// **************************************************************************
//		Direct affectation for spell
// **************************************************************************

/**
**      Parse the dependency of spell.
**      list = (upgrade "upgradename")
*/
local void ccl_spell_dependency(const char *id, SCM list, SpellType *spell)
{
    assert (id != NULL);
    assert (spell != NULL);

    char *dependencyName = NULL;
    SCM  value;
    int  dependencyId = -1;

    value = gh_car(list);

    if (!gh_eq_p(value, gh_symbol2scm("upgrade"))) {
	return;
    }
    list = gh_cdr(list);
    value = gh_car(list);

    dependencyName = gh_scm2newstr(value, NULL);
    dependencyId = UpgradeIdByIdent(dependencyName);
    if (dependencyId == -1)
    {// warn user
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
**	For blizzard and DeathAndDecay.
**	list = fields #n shards #n damage #n
*/
local char CclSpellParseActionAreaBombardment(const char *SpellName, SCM list, SpellActionType *spellaction)
{
    SCM	value;

    assert(SpellName);
    assert(spellaction != NULL);

    memset(spellaction,sizeof(spellaction),0);
    for (; !gh_null_p(list); list = gh_cdr(list)) {
	value = gh_car(list);
	list = gh_cdr(list);
	if (gh_eq_p(value, gh_symbol2scm("fields"))) {
	    spellaction->AreaBombardment.Fields = gh_scm2int(gh_car(list));
	    continue;
	} else if (gh_eq_p(value, gh_symbol2scm("shards"))) {
	    spellaction->AreaBombardment.Shards = gh_scm2int(gh_car(list));
	    continue;
	} else if (gh_eq_p(value, gh_symbol2scm("damage"))) {
	    spellaction->AreaBombardment.Damage = gh_scm2int(gh_car(list));
	    continue;
	} else if (gh_eq_p(value, gh_symbol2scm("start-offset-x"))) {
	    spellaction->AreaBombardment.StartOffsetX = gh_scm2int(gh_car(list));
	    continue;
	} else if (gh_eq_p(value, gh_symbol2scm("start-offset-y"))) {
	    spellaction->AreaBombardment.StartOffsetY = gh_scm2int(gh_car(list));
	    continue;
	}
	// warning user : unknow tag
	DebugLevel0Fn("FIXME : better WARNING : unknow tag");
    }
    return 1;
}

/**
**	For fireball and Runes.
**	list = 'ttl #n 'damage #n
*/
local char CclSpellParseActionFireball(const char *SpellName, SCM list, SpellActionType *spellaction)
{
    int	ttl;
    int	damage;
    SCM	value;

    assert(SpellName);
    assert(spellaction != NULL);

    ttl = 0;
    damage = 0;
    for (; !gh_null_p(list); list = gh_cdr(list)) {
	value = gh_car(list);
	list = gh_cdr(list);
	// Todo, Warn for redefinition ???
	if (gh_eq_p(value, gh_symbol2scm("ttl"))) {
	    ttl = gh_scm2int(gh_car(list));
	    continue;
	}
	if (gh_eq_p(value, gh_symbol2scm("damage"))) {
	    damage = gh_scm2int(gh_car(list));
	    continue;
	}
	// warning user : unknow tag
	DebugLevel0Fn("FIXME : better WARNING : unknow tag");
    }
    if (damage == 0) {
	DebugLevel0Fn("in spell-type %s : %s" _C_ SpellName _C_
		"damage == 0 have no sense : Positive to deal damage, negative for healing.");
	return 0;
    }
    if (ttl <= 0)  {
	//  TTL must be positive.
	DebugLevel0Fn("in spell-type %s : %s" _C_ SpellName _C_
		"TTL <= 0 have no sense");
	return 0;
    }
    spellaction->Fireball.TTL = ttl;
    spellaction->Fireball.Damage = damage;
    return 1;
}

/**
**	For flameshield and whirlwind.
**	list = 'ttl #n
*/
local char CclSpellParseActionFlameShield(const char *SpellName, SCM list, SpellActionType *spellaction)
{
    int ttl;
    SCM	value;

    assert(SpellName);
    assert(spellaction != NULL);

    ttl = 0;
    for (; !gh_null_p(list); list = gh_cdr(list)) {
	value = gh_car(list);
	list = gh_cdr(list);
	// Todo, Warn for redefinition ???
	if (gh_eq_p(value, gh_symbol2scm("ttl")))
	{
		ttl = gh_scm2int(gh_car(list));
		continue;
	}
	// Warning user : unknow tag
	DebugLevel0Fn("FIXME : better WARNING : unknow tag");
    }
    if (ttl <= 0) {
	DebugLevel0Fn("in spell-type %s : %s" _C_ SpellName _C_
		"ttl <= 0 have no sense");//  ttl must be positive.
	return 0;
    }
    spellaction->FlameShield.TTL = ttl;
    return 1;
}

/**
**	For haste, slow and bloodlust.
**	list = *haste #n *slow #n *bloodlust #n
**	One or more.
**	@todo Free when an error occurs. Do a function to do this.
*/
local char CclSpellParseActionHaste(const char *SpellName, SCM list, SpellActionType	*spellaction)
{
    struct {
	const char *id;
	int flag;
    } parser[] = {
	{"haste", flag_haste},
	{"slow", flag_slow},
	{"bloodlust", flag_bloodlust},
	{"HP", flag_HP},
	{"mana", flag_Mana},
	{"HP_percent", flag_HP_percent},
	{"mana_percent", flag_Mana_percent},
	{NULL, 0}
    };

    int	i;
    SCM	value;
    struct s_haste	*lasthaste;
    struct s_haste	*newhaste;

    assert(SpellName);
    assert(spellaction != NULL);

    lasthaste = NULL;
    newhaste = &spellaction->haste;
    while(!gh_null_p(list)) {
	if (lasthaste != NULL) {
	    newhaste = malloc(sizeof(*newhaste));
	    memset(newhaste, 0, sizeof(*newhaste));
	}
	value = gh_car(list);
	list = gh_cdr(list);
	for (i = 0; parser[i].id != NULL; i++) {
	    if (gh_eq_p(value, gh_symbol2scm((char *) parser[i].id))) {
		newhaste->flag = parser[i].flag;
		newhaste->value = gh_scm2int(gh_car(list));
		list = gh_cdr(list);
		newhaste->next = NULL;
		break;
	    }
	}
	if (parser[i].id == NULL) {
	    DebugLevel0Fn("FIXME : better WARNING : unknow tag");
	    // FIXME : free correctly.
	    return 0;
	}
	if (spellaction->haste.value < 0) {
	    DebugLevel0Fn("in spell-type %s : %s" _C_ SpellName _C_
		    "value < 0 have no sense : "
		    "nul to desactivate, positive te activate.");// value == 0 have no sense.
	    // FIXME : free correctly.
	    return 0;
	}
	if (lasthaste != NULL) {
	    lasthaste->next = newhaste;
	}
	lasthaste = newhaste;
    }
    return 1;
}

/**
**	For healing or dealing damage.
**	HP positive for healing, negative for dealing damage.
**	list = (HP #n)
*/
local char CclSpellParseActionHealing(const char *SpellName, SCM list, SpellActionType *spellaction)
{
   

    SCM	value;
    int HP = 0;
	assert(SpellName);
    assert(spellaction != NULL);
    for (; !gh_null_p(list); list = gh_cdr(list)) {
	value = gh_car(list);
	list = gh_cdr(list);
	// Todo, Warn for redefinition ???
	if (gh_eq_p(value, gh_symbol2scm("HP"))) {
	    HP = gh_scm2int(gh_car(list));
	    continue;
	}
	// Warning user : unknow tag
	DebugLevel0Fn("FIXME : better WARNING : unknow tag");
    }
    if (spellaction->healing.HP != 0 && spellaction->healing.HP != HP) {
	DebugLevel3Fn("Redefinition in spell-type '%s' : %s : '%d' -> '%d'\n"
		_C_ SpellName _C_ "HP" _C_ spellaction->healing.HP _C_ HP);
    }
    spellaction->healing.HP = HP;
    if (HP == 0) {
	DebugLevel0Fn("in spell-type %s : %s" _C_ SpellName _C_
	"HP == 0 have no sense : "
	"Positive for healing, negative to deal damages.");
	return 0;
    }
    return 1;
}

/**
**	For invisibility and unholyarmor.
**	list = flag #f_inv value #n missile "missile-name"
*/
local char CclSpellParseActionInvisibility(const char *SpellName, SCM list, SpellActionType	*spellaction)
{
    const struct {
	const char *id;
	int flag;
    } parser[] = {
	{"unholyarmor", flag_unholyarmor},
	{"invisibility", flag_invisibility},
	{NULL, 0}
    };
    SCM	value;
    int	i;
    int flag;
    int sec;
    char *missilename;
    flag = -1;
    sec = 0;
    missilename=NULL;

    assert(SpellName);
    assert(spellaction != NULL);

    for (; !gh_null_p(list); list = gh_cdr(list)) {
	value = gh_car(list);
	list = gh_cdr(list);
	// Todo, Warn for redefinition ???
	if (gh_eq_p(value, gh_symbol2scm("flag"))) {
	    value = gh_car(list);
	    flag = -1;
	    for (i = 0; parser[i].id != NULL; i++) {
		if (gh_eq_p(value, gh_symbol2scm((char *) parser[i].id))) {
		    flag = parser[i].flag;
		    break;
		}
	    }
	    continue;
	}
	if (gh_eq_p(value, gh_symbol2scm("value"))) {
		sec = gh_scm2int(gh_car(list));
		continue;
	}
	if (gh_eq_p(value, gh_symbol2scm("missile"))) {
	    if (missilename != NULL) {
		free(missilename);
	    }
	    missilename = gh_scm2newstr(gh_car(list), NULL);
	    continue;
	}
	// Warning user : unknow tag
	DebugLevel0Fn("FIXME : better WARNING : unknow tag");
    }
    if (missilename == NULL) {
	DebugLevel0Fn("FIXME : better WARNING : must define (correctly)missilename");
	return 0;
    }
    if (flag == -1) {
	DebugLevel0Fn("FIXME : better WARNING : must define (correctly) missilename");
	free(missilename);
	return 0;
    }
    if (sec <= 0) {
	DebugLevel0Fn("FIXME : better WARNING : must define (correctly) missilename");
	free(missilename);
	return 0;
    }
    spellaction->invisibility.flag = flag;
    spellaction->invisibility.value = sec;
    spellaction->invisibility.missile = MissileTypeByIdent(missilename);
    if (spellaction->invisibility.missile == NULL) {
	// Warning user :  Missile doesn't exist (yet).
	DebugLevel0Fn("in spell-type %s : '%s' %s " _C_ SpellName _C_ missilename
		_C_ "does not exist.");
	free(missilename);
	return 0;
    }
    free(missilename);
    return 1;
}

/**
**	For summoning
**	list = ("unittypename")
**	@note	WARNING, use for other functions than summon, see ccl_spell_action.
*/
local char CclSpellParseActionSummon(const char *SpellName, SCM list, SpellActionType *spellaction)
{
    char *str;
    UnitType *unittype;

    assert(SpellName);
    assert(spellaction != NULL);

    str = gh_scm2newstr(list, NULL);
    unittype = UnitTypeByIdent(str);
    spellaction->Summon.UnitType = unittype;
    if (unittype == NULL) {
	DebugLevel0Fn("in spell-type %s : Unittype '%s'doesn't exist" _C_ SpellName _C_ str);
	free(str);
	return 0;
    }
    free(str);
    return 1;
}

// **************************************************************************
//		main Action parsers for spellAction
// **************************************************************************

/*
**	return false if a problem occurs (unit not found...)
**	@param	spellname : name of spell to display errors and warning.
**	@param	list	: argument for spell.
**	@param	spellaction : What we want modify.
*/
typedef char	f_ccl_action(const char *spellname, SCM list, SpellActionType	*spellaction);

/*
**		Parse the action for spell.
**	list = ('Spell '(parameter))
*/
local void ccl_spell_action(const char *id, SCM list, SpellType *spell)
{
    int		i;
    struct {
	    SpellFunc *fspell;
	    const char *id;
	    f_ccl_action *f;
    }	parser[] = {
	{CastAreaBombardment, "area-bombardment", CclSpellParseActionAreaBombardment},
	{CastSpawnPortal, "spawn-portal", CclSpellParseActionSummon/*circleofpower*/},
	{CastDeathCoil, "death-coil", NULL},
	{CastFireball, "fireball", CclSpellParseActionFireball},
	{CastFlameShield, "flame-shield", CclSpellParseActionFlameShield},
	{CastHaste, "haste", CclSpellParseActionHaste},
	{CastHealing, "healing", CclSpellParseActionHealing},
	{CastHolyVision, "HolyVision", CclSpellParseActionSummon/*holyvision*/},
	{CastInvisibility, "Invisibility", CclSpellParseActionInvisibility},
	{CastPolymorph, "Polymorph", CclSpellParseActionSummon/*polymorph*/},
	{CastRaiseDead, "RaiseDead", CclSpellParseActionSummon/*raisedead*/},
	{CastRunes, "Runes", CclSpellParseActionFireball/*runes*/},
	{CastSummon, "Summon", CclSpellParseActionSummon},
	{CastWhirlwind, "Whirlwind", CclSpellParseActionFlameShield/*whirlwind*/},
	{0, NULL, NULL}
    };
    
    SCM	value = list;
    value = gh_car(value);

    assert (id != NULL);
    assert (spell != NULL);

    for (i = 0; parser[i].id != NULL; i++) {
	if (gh_eq_p(value, gh_symbol2scm((char *) parser[i].id))) {
	    spell->f = parser[i].fspell;
	    if (parser[i].f) {
		DebugLevel3Fn("%s %d\n" _C_ parser[i].id _C_ sizeof(*spell->SpellAction));
		list = gh_cdr(list);
		free(spell->SpellAction);// FIXME : Use a destructor : free pointer, list..
		spell->SpellAction = (SpellActionType *) malloc(sizeof (*spell->SpellAction));
		memset(spell->SpellAction, 0, sizeof(*spell->SpellAction));
		if (parser[i].f(spell->IdentName, gh_car(list), spell->SpellAction) == 0) {
		    // Error in function : it is to the function to warn..
		    free(spell->SpellAction);// FIXME : Use a destructor : free pointer, list..
		    spell->SpellAction = NULL;
		    spell->f = NULL;
		}
	    }
	    break;
	}
    }
    if (parser[i].id == NULL) {
	// Warning user : bad flag.
	free(spell->SpellAction);// FIXME : Use a destructor : free pointer, list..
	spell->SpellAction = NULL;
	spell->f = NULL;
	errl("Unsupported tag", value);
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

	assert(begin != NULL);
    assert(cond != NULL);

    if (*begin == NULL) {
	*begin = cond;
	return ;
    }
    for(next = *begin; next->next != NULL; next = next->next) {
	next->next = cond;
    }
}

/*
**	Pointer on function to parse 'condition.
**	@param	spellName : name of spell modified.
**	@param	id : last keyword recognized.
**	@param	list : args for \p id.
**	@param	generic : list for generic conditions.
**	@param	specific : list for specific conditions.
*/
typedef	void f_ccl_condition(const char *spellName, const char *id, SCM list,
	t_Conditions **generic, t_Conditions **specific);

/**
**	Parsing true or false.
**	factorisation of code.
**	@param	SpellName : Name of spell.
**	@param	id : last keywork.
**	@param	expectvalue : OUTPUT <- 1 if true 0 else
*/
local int ccl_true(const char *SpellName, const char *id, int *expectvalue, SCM value)
{
    assert(SpellName);
    assert(id);
    assert(expectvalue);
    
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
	    _C_ SpellName _C_ id _C_ "Argument must be true or false");
    return 0;
}

/*
**		Parse the Condition for spell.
**	list = #t #flagtype
*/
local void ccl_UnitTypeFlag(const char *SpellName, const char *id, SCM list,
		t_Conditions **generic, t_Conditions **specific)
{
   

    SCM	value;
    t_Conditions *cond;

	

    struct {
	const char *id;
	int	flag;
    }	parser[] = {
	{"building", flag_building},
	{"coward", flag_coward},
	{"canattack", flag_canattack},
	{"organic", flag_organic},
	{"undead", flag_isundead},
	{NULL, 0}
    };
    int	i;

	assert(id != NULL);
    assert(SpellName != NULL);
    assert(generic != NULL);
    assert(specific != NULL);


    cond = malloc(sizeof(*cond));
    memset(cond, 0, sizeof(*cond));
    value = gh_car(list);
    if (!ccl_true(SpellName, id, &cond->expectvalue, value)) {
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
local void	ccl_DurationEffect(const char *SpellName, const char *id, SCM list,
		t_Conditions **generic, t_Conditions **specific)
{
    

    SCM	value;
    t_Conditions *cond;
    struct {
	const char *id;
	int	flag;
    }	parser[] = {
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
    int	i;

	assert(id != NULL);
    assert(SpellName != NULL);
    assert(generic != NULL);
    assert(specific != NULL);

    cond = malloc(sizeof(*cond));
    memset(cond, 0, sizeof(*cond));
    value = gh_car(list);
    if (!ccl_true(SpellName, id, &cond->expectvalue, value)) {
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

/*
**	For enemy presence
**	list = #t range #n
*/
void ccl_enemy_presence(const char *SpellName, const char *id, SCM list,
		t_Conditions **generic, t_Conditions **specific)
{
    
    SCM	value;
    t_Conditions *cond;
	
	assert(id != NULL);
    assert(SpellName != NULL);
    assert(generic != NULL);
    assert(specific != NULL);

    cond = malloc(sizeof(*cond));
    memset(cond, 0, sizeof(*cond));
    value = gh_car(list);
    if (!ccl_true(SpellName, id, &cond->expectvalue, value)) {
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

/*
**	For enemy presence
**	list = #t
*/
void ccl_alliance(const char *SpellName, const char *id, SCM list,
		    t_Conditions **generic, t_Conditions **specific)
{
	

	SCM	value;
	t_Conditions *cond;

	assert(id != NULL);
	assert(SpellName != NULL);
	assert(generic != NULL);
	assert(specific != NULL);

	cond = malloc(sizeof(*cond));
	memset(cond, 0, sizeof(*cond));
	value = /*gh_car(*/list/*)*/;
	if (!ccl_true(SpellName, id, &cond->expectvalue, value)) {//	warn user
	    free(cond);
	    return ;
	}
	cond->f.specific = CheckAllied;
	pushback_condition(specific, cond);
}

/*
**		Parse the Condition for spell.
*/
local void ccl_spell_all_condition(const char*SpellName, const char *id, SCM list,
		t_Conditions **generic, t_Conditions **specific)
{
   

    int	i;
    struct {
	const char *id;
	f_ccl_condition	*f;
    } parser[] = {
	{"UnitTypeflag", ccl_UnitTypeFlag},
	{"DurationEffect", ccl_DurationEffect},
	{"Enemypresence", ccl_enemy_presence},
	{"Alliance", ccl_alliance},
	{ NULL, 0}	
    };

    SCM value;

	assert(id != NULL);
    assert(SpellName != NULL);
    assert(generic != NULL);
    assert(specific != NULL);


    for (; !gh_null_p(list); list = gh_cdr(list)) {
	value = gh_car(list);
	for (i = 0; parser[i].id != NULL; i++) {
	    if (gh_eq_p(value, gh_symbol2scm((char *) parser[i].id))) {
		list = gh_cdr(list);
		parser[i].f(SpellName, parser[i].id, gh_car(list), generic, specific);
		break;
	    }
	}
	assert(parser[i].id);
    }
}

/*
**		Parse the Condition for spell.
*/
local void ccl_spell_condition(const char *id, SCM list, SpellType *spell)
{
    assert(id != NULL);
    assert(spell != NULL);

    ccl_spell_all_condition(spell->IdentName, id, list,
	    &spell->Condition_generic,&spell->Condition_specific);
}

local void ccl_spell_autocast(const char *id, SCM list, SpellType *spell)
{
   

    SCM	value;
    int range;

	assert(id != NULL);
    assert(spell != NULL);

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
    SCM	value;

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
	    ccl_spell_action("action",gh_car(list),spell);
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
global void SaveSpells(CLFile* file)
{
    SpellType* spell;
    struct s_haste *hinfo;

    assert(file);
    
    for (spell=SpellTypeTable;spell<SpellTypeTable+SpellTypeCount;++spell) {
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
	if (spell->f==CastAreaBombardment) {
	    CLprintf(file," '(area-bombardment (fields %d shards %d damage %d start-offset-x %d start-offset-y %d) )\n",
		    spell->SpellAction->AreaBombardment.Fields,
		    spell->SpellAction->AreaBombardment.Shards,
		    spell->SpellAction->AreaBombardment.Damage,
		    spell->SpellAction->AreaBombardment.StartOffsetX,
		    spell->SpellAction->AreaBombardment.StartOffsetY);
	} else if (spell->f==CastFireball) {
	    CLprintf(file," '(fireball (ttl %d damage %d) )\n",
		    spell->SpellAction->Fireball.TTL,
		    spell->SpellAction->Fireball.Damage);
	} else if (spell->f==CastHolyVision) {
	    CLprintf(file," '(HolyVision \"%s\" )\n",spell->SpellAction->holyvision.revealer->Ident);
	} else if (spell->f==CastHealing) {
	    CLprintf(file," '(healing (HP %d) )\n",spell->SpellAction->healing.HP);
	} else if (spell->f==CastSummon) {
	    CLprintf(file," '(Summon \"%s\")\n",spell->SpellAction->Summon.UnitType->Ident);
	} else if (spell->f==CastHaste) {
	    CLprintf(file," '(haste ( ");
	    hinfo=&spell->SpellAction->haste;
	    while (hinfo) {
		switch (hinfo->flag) {
		    case flag_haste:
			CLprintf(file,"haste");
			break;
		    case flag_slow:
			CLprintf(file,"slow");
			break;
		    case flag_bloodlust:
			CLprintf(file,"bloodlust");
			break;
		    case flag_HP:
			CLprintf(file,"HP");
			break;
		    case flag_Mana:
			CLprintf(file,"mana");
			break;
		    case flag_HP_percent:
			CLprintf(file,"HP_percent");
			break;
		    case flag_Mana_percent:
			CLprintf(file,"mana_percent");
			break;
		    default:
			DebugLevel0("wrong haste flags?\n");
			DebugCheck(1);
		}
		CLprintf(file," %d ",hinfo->value);
		hinfo=hinfo->next;
	    }
	    CLprintf(file,") )\n");
	} else if (spell->f==CastInvisibility) {
	    CLprintf(file," '(Invisibility ( flag ");
	    if (spell->SpellAction->invisibility.flag==flag_invisibility) {
		CLprintf(file,"invisibility");
	    } else if (spell->SpellAction->invisibility.flag==flag_unholyarmor) {
		CLprintf(file,"unholyarmor");
	    } else {
		DebugLevel0("Wrong flags?\n");
		DebugCheck(1);
	    }
	    CLprintf(file," value %d missile \"%s\") )\n",
		    spell->SpellAction->invisibility.value,
		    spell->SpellAction->invisibility.missile->Ident);
	} else if (spell->f==CastPolymorph) {
	    CLprintf(file," '(Polymorph \"%s\")\n",
		    spell->SpellAction->Polymorph.NewForm->Ident);
	} else if (spell->f==CastRaiseDead) {
	    CLprintf(file," '(RaiseDead \"%s\")\n",
		    spell->SpellAction->RaiseDead.Skeleton->Ident);
	} else if (spell->f==CastFlameShield) {
	    CLprintf(file," '(flame-shield (ttl %d) )\n",
		    spell->SpellAction->FlameShield.TTL);
	} else if (spell->f==CastRunes) {
	    CLprintf(file," '(Runes (ttl %d damage %d) )\n",
		    spell->SpellAction->runes.TTL,
		    spell->SpellAction->runes.Damage);
	} else if (spell->f==CastSpawnPortal) {
	    CLprintf(file," '(spawn-portal \"%s\")\n",
		    spell->SpellAction->SpawnPortal.PortalType->Ident);
	} else if (spell->f==CastDeathCoil) {
	    CLprintf(file," '(death-coil)\n");
	    // FIXME: more?
	} else if (spell->f==CastWhirlwind) {
	    CLprintf(file," '(Whirlwind (ttl %d) )\n",
		    spell->SpellAction->whirlwind.TTL);
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
