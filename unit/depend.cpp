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
/**@name depend.c	-	The units/upgrade dependencies */
//
//	(c) Copyright 2000,2001 by Vladi Belperchinov-Shabanski
//
//	$Id$

//@{

/*----------------------------------------------------------------------------
--	Includes
----------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "freecraft.h"
#include "upgrade_structs.h"
#include "upgrade.h"
#include "depend.h"
#include "player.h"

/*----------------------------------------------------------------------------
--	Variables
----------------------------------------------------------------------------*/

    /// All dependencies hash
local DependRule*	DependHash[101];

#ifndef USE_CCL

/**
**	Buildin (default) dependencies (only for support without CCL).
**
**	Counter -1 starts a new dependency.
*/
local const struct _default_depend_ {
    const char* Name;			/// operation name
    int Count;				/// counter
} DefaultDependencies[] = {
    ///////////////////////////////////////////////////////////////////////////
    //	* Race human:

    //	- human naval forces
    { "unit-human-transport",	-1 },{ "unit-human-foundry",		1 },
    { "unit-battleship",	-1 },{ "unit-human-foundry",		1 },
    { "unit-gnomish-submarine",	-1 },{ "unit-gnomish-inventor",		1 },

    //	- human land forces
    { "unit-archer",		-1 },{ "unit-elven-lumber-mill",	1 },
    { "unit-ranger",		-1 },{ "unit-elven-lumber-mill",	1 },
				     { "upgrade-ranger",		1 },
    { "unit-knight",		-1 },{ "unit-human-blacksmith",		1 },
			             { "unit-stables",			1 },
    { "unit-paladin",		-1 },{ "unit-human-blacksmith",		1 },
				     { "unit-stables",			1 },
				     { "upgrade-paladin",		1 },
    { "unit-ballista",		-1 },{ "unit-elven-lumber-mill",	1 },
				     { "unit-human-blacksmith",		1 },
    //	- human flying forces
    { "unit-gnomish-flying-machine",-1 },{ "unit-elven-lumber-mill",	1 },

    //	-human buildings
    { "unit-human-guard-tower",	-1 },{ "unit-elven-lumber-mill",	1 },
    { "unit-human-cannon-tower",-1 },{ "unit-human-blacksmith",		1 },

    { "unit-human-shipyard",	-1 },{ "unit-elven-lumber-mill",	1 },
    { "unit-human-foundry",	-1 },{ "unit-human-shipyard",		1 },
    { "unit-human-refinery",	-1 },{ "unit-human-shipyard",		1 },

    { "unit-stables",		-1 },{ "unit-keep",			1 },
    { "unit-stables",		-1 },{ "unit-castle",			1 },
    { "unit-gnomish-inventor",	-1 },{ "unit-keep",			1 },
    { "unit-gnomish-inventor",	-1 },{ "unit-castle",			1 },

    { "unit-mage-tower",	-1 },{ "unit-castle",			1 },
    { "unit-church",		-1 },{ "unit-castle",			1 },
    { "unit-gryphon-aviary",	-1 },{ "unit-castle",			1 },

    { "unit-keep",		-1 },{ "unit-human-barracks",		1 },

    { "unit-castle",		-1 },{ "unit-elven-lumber-mill",	1 },
				     { "unit-human-blacksmith",		1 },
				     { "unit-stables",			1 },

    //	- human upgrades
    { "upgrade-sword2",		-1 },{ "upgrade-sword1",		1 },
    { "upgrade-arrow2",		-1 },{ "upgrade-arrow1",		1 },
    { "upgrade-human-shield2",	-1 },{ "upgrade-human-shield1",		1 },
    { "upgrade-ballista2",	-1 },{ "upgrade-ballista1",		1 },

    { "upgrade-ranger",		-1 },{ "unit-keep",			1 },
    { "upgrade-ranger",		-1 },{ "unit-castle",			1 },

    { "upgrade-ranger-scouting",-1 },{ "upgrade-ranger",		1 },
				     { "unit-castle",			1 },
    { "upgrade-longbow",	-1 },{ "upgrade-ranger",		1 },
				     { "unit-castle",			1 },
    { "upgrade-ranger-marksmanship",-1 },{ "upgrade-ranger",		1 },
				     { "unit-castle",			1 },
    { "upgrade-holy-vision",	-1 },{ "upgrade-paladin",		1 },
    { "upgrade-healing",	-1 },{ "upgrade-paladin",		1 },
    { "upgrade-exorcism",	-1 },{ "upgrade-paladin",		1 },

    { "upgrade-human-ship-cannon2",-1 },{ "upgrade-human-ship-cannon1",1 },
    { "upgrade-human-ship-armor2",-1 },{ "upgrade-human-ship-armor1",	1 },

    ///////////////////////////////////////////////////////////////////////////
    //	* Race orc:
    //	- orc naval forces
    { "unit-orc-transport",	-1 },{ "unit-orc-foundry",		1 },
    { "unit-ogre-juggernaught",	-1 },{ "unit-orc-foundry",		1 },
    { "unit-giant-turtle",	-1 },{ "unit-goblin-alchemist",		1 },

    //	- orc land forces
    { "unit-ogre",		-1 },{ "unit-orc-blacksmith",		1 },
				     { "unit-ogre-mound",		1 },
    { "unit-ogre-mage",		-1 },{ "upgrade-ogre-mage",		1 },
				     { "unit-orc-blacksmith",		1 },
				     { "unit-ogre-mound",		1 },
    { "unit-axethrower",	-1 },{ "unit-troll-lumber-mill",	1 },
    { "unit-berserker",		-1 },{ "upgrade-berserker",		1 },
				     { "unit-troll-lumber-mill",	1 },
    { "unit-catapult",		-1 },{ "unit-troll-lumber-mill",	1 },
				     { "unit-orc-blacksmith",		1 },

    // orc flying forces
    { "unit-goblin-zeppelin",	-1 },{ "unit-troll-lumber-mill",	1 },

    // orc buildings
    { "unit-orc-guard-tower",	-1 },{ "unit-troll-lumber-mill",	1 },
    { "unit-orc-cannon-tower",	-1 },{ "unit-orc-blacksmith",		1 },

    { "unit-orc-shipyard",	-1 },{ "unit-troll-lumber-mill",	1 },
    { "unit-orc-foundry",	-1 },{ "unit-orc-shipyard",		1 },
    { "unit-orc-refinery",	-1 },{ "unit-orc-shipyard",		1 },

    { "unit-ogre-mound",	-1 },{ "unit-stronghold",		1 },
    { "unit-ogre-mound",	-1 },{ "unit-fortress",			1 },
    { "unit-goblin-alchemist",	-1 },{ "unit-stronghold",		1 },
    { "unit-goblin-alchemist",	-1 },{ "unit-fortress",			1 },

    { "unit-temple-of-the-damned",	-1 },{ "unit-fortress",		1 },
    { "unit-altar-of-storms",	-1 },{ "unit-fortress",			1 },
    { "unit-dragon-roost",	-1 },{ "unit-fortress",			1 },

    { "unit-stronghold",	-1 },{ "unit-orc-barracks",		1},

    { "unit-fortress",		-1 },{ "unit-troll-lumber-mill",	1},
				     { "unit-orc-blacksmith",		1},
				     { "unit-ogre-mound",		1},

    // orc upgrades
    { "upgrade-battle-axe2",	-1 },{ "upgrade-battle-axe1",		1 },
    { "upgrade-throwing-axe2",	-1 },{ "upgrade-throwing-axe1",		1 },
    { "upgrade-orc-shield2",	-1 },{ "upgrade-orc-shield1",		1 },
    { "upgrade-catapult2",	-1 },{ "upgrade-catapult1",		1 },

    { "upgrade-berserker",	-1 },{ "unit-stronghold",		1 },
    { "upgrade-berserker",	-1 },{ "unit-fortress",			1 },
    { "upgrade-berserker-scouting",-1 },{ "upgrade-berserker",	1 },
				     { "unit-fortress",			1 },
    { "upgrade-light-axes",	-1 },{ "upgrade-berserker",		1 },
				     { "unit-fortress",			1 },
    { "upgrade-berserker-regeneration",	-1 },{ "upgrade-berserker",	1 },
				     { "unit-fortress",			1 },

    { "upgrade-eye-of-kilrogg",	-1 },{ "upgrade-ogre-mage",		1 },
    { "upgrade-bloodlust",	-1 },{ "upgrade-ogre-mage",		1 },
    { "upgrade-runes",		-1 },{ "upgrade-ogre-mage",		1 },

    { "upgrade-orc-ship-cannon2",-1 },{ "upgrade-orc-ship-cannon1",	1 },
    { "upgrade-orc-ship-armor2",-1 },{ "upgrade-orc-ship-armor1",	1 },
};

#endif

/*----------------------------------------------------------------------------
--	Functions
----------------------------------------------------------------------------*/

/**
**	Add a new dependency. If already exits append to and rule.
**
**	@param target	Target of the dependency.
**	@param required	Requirement of the dependency.
**	@param count	Amount of the required needed.
**	@param or_flag	Start of or rule.
*/
global void AddDependency(const char* target,const char* required,int count
	,int or_flag)
{
    DependRule rule;
    DependRule* node;
    DependRule* temp;
    int hash;

    //
    //	Setup structure.
    //
    if ( !strncmp( target, "unit-", 5 ) ) {
	// target string refers to unit-xxx
	rule.Type = DependRuleUnitType;
	rule.Kind.UnitType = UnitTypeByIdent( target );
    } else if ( !strncmp( target, "upgrade-", 8 ) ) {
	// target string refers to upgrade-XXX
	rule.Type = DependRuleUpgrade;
	rule.Kind.Upgrade = UpgradeIdByIdent( target );
    } else {
	DebugLevel0Fn("dependency target `%s' should be unit-type or upgrade\n"
		,target);
	return;
    }
    hash=(int)(long)rule.Kind.UnitType%(sizeof(DependHash)/sizeof(*DependHash));

    //
    //	Find correct hash slot.
    //
    if( (node=DependHash[hash]) ) {	// find correct entry
	while( node->Type!=rule.Type
		    || node->Kind.Upgrade!=rule.Kind.Upgrade ) {
	    if( !node->Next ) {		// end of list
		temp=malloc(sizeof(DependRule));
		temp->Next=NULL;
		temp->Rule=NULL;
		temp->Type=rule.Type;
		temp->Kind=rule.Kind;
		node->Next=temp;
		node=temp;
		break;
	    }
	    node=node->Next;
	}
    } else {				// create new slow
	node=malloc(sizeof(DependRule));
	node->Next=NULL;
	node->Rule=NULL;
	node->Type=rule.Type;
	node->Kind=rule.Kind;
	DependHash[hash]=node;
    }

    //
    //	Adjust count.
    //
    if ( count < 0 || count > 255 ) {
	DebugLevel0Fn("wrong count `%d' range 0 .. 255\n",count);
	count = 255;
    }

    temp=malloc(sizeof(DependRule));
    temp->Rule=NULL;
    temp->Next=NULL;
    temp->Count=count;
    //
    //	Setup structure.
    //
    if ( !strncmp( required, "unit-", 5 ) ) {
	// required string refers to unit-xxx
	temp->Type = DependRuleUnitType;
	temp->Kind.UnitType = UnitTypeByIdent( required );
    } else if ( !strncmp( required, "upgrade-", 8 ) ) {
	// required string refers to upgrade-XXX
	temp->Type = DependRuleUpgrade;
	temp->Kind.Upgrade = UpgradeIdByIdent( required );
    } else {
	DebugLevel0Fn("
		dependency required `%s' should be unit-type or upgrade\n"
		,required);
	free(temp);
	return;
    }

    if( or_flag ) {
	temp->Next=node->Rule;		// insert rule
	node->Rule=temp;
    } else {
	temp->Rule=node->Rule;		// insert rule
	node->Rule=temp;
    }
}

/**
**	Check if this upgrade or unit is availalbe.
**
**	@param player	For this player available.
**	@param target	Unit or Upgrade.
**
**	@return		True if available, false otherwise.
*/
global int CheckDependByIdent(const Player* player,const char* target)
{
    DependRule rule;
    const DependRule* node;
    const DependRule* temp;
    int i;

    //
    //	first have to check, if target is allowed itself
    //
    if ( !strncmp( target, "unit-", 5 ) ) {
	// target string refers to unit-XXX
	rule.Kind.UnitType = UnitTypeByIdent( target );
	if ( UnitIdAllowed( player, rule.Kind.UnitType->Type ) != 'A' ) {
	    return 0;
	}
	rule.Type = DependRuleUnitType;
    } else if ( !strncmp( target, "upgrade-", 8 ) ) {
	// target string refers to upgrade-XXX
	rule.Kind.Upgrade = UpgradeIdByIdent( target );
	if( UpgradeIdAllowed( player, rule.Kind.Upgrade ) != 'A' ) {
	    return 0;
	}
	rule.Type = DependRuleUpgrade;
    } else {
	DebugLevel0Fn("target `%s' should be unit-type or upgrade\n",target);
	return 0;
    }

    //
    //	Find rule
    //
    i=(int)(long)rule.Kind.UnitType%(sizeof(DependHash)/sizeof(*DependHash));

    if( (node=DependHash[i]) ) {	// find correct entry
	while( node->Type!=rule.Type
		    || node->Kind.Upgrade!=rule.Kind.Upgrade ) {
	    if( !node->Next ) {		// end of list
		return 1;
	    }
	    node=node->Next;
	}
    } else {
	return 1;
    }

    //
    //	Prove the rules
    //
    node=node->Rule;

    while( node ) {
	temp=node;
	while( temp ) {
	    switch( temp->Type ) {
	    case DependRuleUnitType:
		i=HaveUnitTypeByType( player, temp->Kind.UnitType );
		if ( temp->Count ? i<temp->Count : i ) {
		    goto try_or;
		}
		break;
	    case DependRuleUpgrade:
		i=UpgradeIdAllowed( player, temp->Kind.Upgrade ) != 'R';
		if ( temp->Count ? i : !i ) {
		    goto try_or;
		}
		break;
	    }
	    temp=temp->Rule;
	}
	return 1;				// all rules matches.

try_or:
	node=node->Next;
    }

    return 0;					// no rule matches
}

/**
**	Initialize unit and upgrade dependencies.
*/
global void InitDependencies(void)
{
#ifndef USE_CCL
    int i;
    int or_flag;
    const char* target;

    //
    //	Add all default dependencies
    //
    target=NULL;
    or_flag=0;
    for( i=0; i<sizeof(DefaultDependencies)/sizeof(*DefaultDependencies);
		++i ) {
	if( DefaultDependencies[i].Count==-1 ) {
	    // Or rule
	    if( target && !strcmp(target,DefaultDependencies[i].Name) ) {
		or_flag=1;
	    }
	    target=DefaultDependencies[i].Name;
	    continue;
	}
	AddDependency(target,DefaultDependencies[i].Name,
		DefaultDependencies[i].Count,or_flag);
	or_flag=0;
    }
#endif
}

/**
**	Save state of the dependencies to file.
**
**	@param file	Output file.
*/
global void SaveDependencies(FILE* file)
{
    int i;
    const DependRule* node;
    const DependRule* rule;
    const DependRule* temp;

    fprintf(file,"\n;;; -----------------------------------------\n");
    fprintf(file,";;; MODULE: dependencies $Id$\n\n");

    // Save all dependencies

    for( i=0; i<sizeof(DependHash)/sizeof(*DependHash); ++i ) {
	node=DependHash[i];
	while( node ) {			// all hash links
	    fprintf(file,"(define-dependency '");
	    switch( node->Type ) {
		case DependRuleUnitType:
		    fprintf(file,"%s",node->Kind.UnitType->Ident);
		    break;
		case DependRuleUpgrade:
		    fprintf(file,"%s",Upgrades[node->Kind.Upgrade].Ident);
		    break;
	    }
	    // All or cases

	    fprintf(file,"\n  '(");
	    rule=node->Rule;
	    for( ;; ) {
		temp=rule;
		while( temp ) {
		    switch( temp->Type ) {
		    case DependRuleUnitType:
			fprintf(file,"%s"
				,temp->Kind.UnitType->Ident);
			break;
		    case DependRuleUpgrade:
			fprintf(file,"%s"
				,Upgrades[temp->Kind.Upgrade].Ident);
			break;
		    }
		    temp=temp->Rule;
		    if( temp ) {
			fprintf(file," ");
		    }
		}
		fprintf(file,")");
		if( !(rule=rule->Next) ) {
		    break;
		}
		fprintf(file,"\n  'or '( ");
	    }

	    fprintf(file,")\n");

	    node=node->Next;
	}
    }
}

/*----------------------------------------------------------------------------
--	Ccl part of dependencies
----------------------------------------------------------------------------*/

#ifdef USE_CCL

#include "ccl.h"

/**
**	Define a new dependency.
**
**	@param list	List of the dependency.
*/
local SCM CclDefineDependency(SCM list)
{
    char* target;
    char* required;
    int count;
    SCM value;
    SCM temp;
    int or_flag;

    value=gh_car(list);
    list=gh_cdr(list);
    target=gh_scm2newstr(value,NULL);

    //
    //	All or rules.
    //
    or_flag=0;
    while( !gh_null_p(list) ) {
	temp=gh_car(list);
	list=gh_cdr(list);

	while( !gh_null_p(temp) ) {
	    value=gh_car(temp);
	    temp=gh_cdr(temp);
	    required=gh_scm2newstr(value,NULL);
	    count=1;
	    if( !gh_null_p(temp) && gh_exact_p(temp) ) {
		value=gh_car(temp);
		count=gh_scm2int(value);
		temp=gh_cdr(temp);
	    }

	    AddDependency(target,required,count,or_flag);
	    free(required);
	    or_flag=0;
	}
	if( !gh_null_p(list) ) {
	    if( !gh_eq_p(gh_car(list),gh_symbol2scm("or")) ) {
		errl("not or symbol",gh_car(list));
		return SCM_UNSPECIFIED;
	    }
	    or_flag=1;
	    list=gh_cdr(list);
	}
    }
    free(target);

    return SCM_UNSPECIFIED;
}

/**
**	Get the dependency.
**
**	@todo not written.
**
**	@param target	Unit type or upgrade.
*/
local SCM CclGetDependency(SCM target)
{
    DebugLevel0Fn("FIXME: write this\n");

    return SCM_UNSPECIFIED;
}

/**
**	Check the dependency.
**
**	@todo not written.
**
**	@param target	Unit type or upgrade.
*/
local SCM CclCheckDependency(SCM target)
{
    DebugLevel0Fn("FIXME: write this\n");

    return SCM_UNSPECIFIED;
}

/**
**	Register CCL features for dependencies.
*/
global void DependenciesCclRegister(void)
{
    gh_new_procedureN("define-dependency",CclDefineDependency);
    gh_new_procedure1_0("get-dependency",CclGetDependency);
    gh_new_procedure1_0("check-dependency",CclCheckDependency);
}

#endif	// defined(USE_CCL)

//@}
