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
/*
**	(c) Copyright 2000 by Vladi Belperchinov-Shabanski
**
**	$Id$
*/

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

/**
**	Buildin (default) dependencies.
**
**	Counter -1 starts a new dependency.
*/
local struct _default_depend_ {
    const char* Name;			/// operation name
    int Count;				/// counter
} Dependencies[] = {
    ///////////////////////////////////////////////////////////////////////////
    //	* Race human:

    //	- human naval forces
    { "unit-human-transport",	-1 },{ "unit-elven-lumber-mill",	1 },
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
	DebugLevel0(__FUNCTION__": wrong dependency target %s\n",target);
	return;
    }
    hash=(int)(long)rule.Kind.UnitType%(sizeof(DependHash)/sizeof(*DependHash));
    DebugLevel3("Hash %d\n",hash);

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
	DebugLevel0(__FUNCTION__": wrong count range\n");
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
	DebugLevel0(__FUNCTION__": wrong dependency required %s\n",required);
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
    DependRule* node;
    DependRule* temp;
    int i;

    //
    //	first have to check if target is allowed itself
    //
    if ( !strncmp( target, "unit-", 5 ) ) {
	// target string refers to unit-xxx
	rule.Kind.UnitType = UnitTypeByIdent( target );
	if ( UnitIdAllowed( player, rule.Kind.UnitType->Type ) != 'A' ) {
	    return 0;
	}
	rule.Type = DependRuleUnitType;
    } else if ( !strncmp( target, "upgrade-", 8 ) ) {
	// target string refers to upgrade-XXX
	rule.Kind.Upgrade = UpgradeIdByIdent( target );
	// z=UpgradeIdAllowed( player, upgrade );
	// FIXME: ?? if ( z != 'R' && z != 'A' ) return 0;
	if( UpgradeIdAllowed( player, rule.Kind.Upgrade ) != 'A' ) {
	    return 0;
	}
	rule.Type = DependRuleUpgrade;
    } else {
	// FIXME: should this be an error?!
	// FIXME: this target string is neither UnitType nor Upgrade
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
**
**	FIXME:	Some problems:
**		ballista should be possible if *any* lumber mill is available
**		also an "or" function whould be usefull.
*/
global void InitDependencies(void)
{
    int i;
    int or_flag;
    const char* target;

    // FIXME: don't add default if ccl is already loaded!

    //
    //	Add all default dependencies
    //
    target=NULL;
    or_flag=0;
    for( i=0; i<sizeof(Dependencies)/sizeof(*Dependencies); ++i ) {
	if( Dependencies[i].Count==-1 ) {
	    // Or rule
	    if( target && !strcmp(target,Dependencies[i].Name) ) {
		DebugLevel3(__FUNCTION__": or rule\n");
		or_flag=1;
	    }
	    target=Dependencies[i].Name;
	    continue;
	}
	AddDependency(target,Dependencies[i].Name,Dependencies[i].Count
		    ,or_flag);
	or_flag=0;
    }
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
    fprintf(file,";;; MODULE: dependencies $Id$\n");

    // Save all dependencies

    for( i=0; i<sizeof(DependHash)/sizeof(*DependHash); ++i ) {
	node=DependHash[i];
	while( node ) {			// all hash links
	    fprintf(file,"(define-dependency \"");
	    switch( node->Type ) {
		case DependRuleUnitType:
		    fprintf(file,"%s\"",node->Kind.UnitType->Ident);
		    break;
		case DependRuleUpgrade:
		    fprintf(file,"%s\"",Upgrades[node->Kind.Upgrade].Ident);
		    break;
	    }
	    // All or cases
	    rule=node->Rule;
	    do {
		fprintf(file,"\n  '( ");
		temp=rule;
		while( temp ) {
		    switch( temp->Type ) {
		    case DependRuleUnitType:
			fprintf(file,"\"%s\" "
				,temp->Kind.UnitType->Ident);
			break;
		    case DependRuleUpgrade:
			fprintf(file,"\"%s\" "
				,Upgrades[temp->Kind.Upgrade].Ident);
			break;
		    }
		    temp=temp->Rule;
		}
		fprintf(file,")");
	    } while( (rule=rule->Next) );

	    fprintf(file,")\n");

	    node=node->Next;
	}
    }
}

/*----------------------------------------------------------------------------
--	Ccl part of dependencies
----------------------------------------------------------------------------*/

#if defined(USE_CCL) || defined(USE_CCL2)

#include "ccl.h"

/**
**	Define a new dependency.
*/
local SCM CclDefineDependency(SCM list)
{
    char* target;
    char* required;
    int count;
    SCM value;
    SCM temp;

    value=gh_car(list);
    target=gh_scm2newstr(value,NULL);
    DebugLevel3("Target: %s\n",target);

    while( !gh_null_p(list) ) {
	list=gh_cdr(list);
	value=gh_car(list);
	while( !gh_null_p(value) ) {
	    temp=gh_car(value);
	    required=gh_scm2newstr(temp,NULL);
	    value=gh_cdr(value);
	    count=0;
	    if( !gh_null_p(value) ) {
		temp=gh_car(value);
		count=gh_scm2int(temp);
		value=gh_cdr(value);
	    }
	    DebugLevel3("Target: %s %d \n",required,count);
	    //AddDependency(target,required,count,0);

	    // FIXME: first upgrade must be ready than I can continue here
	}
    }

    return SCM_UNSPECIFIED;
}

/**
**	Get the dependency.
**
**	@param target	Unit type or upgrade.
*/
local SCM CclGetDependency(SCM target)
{
    // FIXME: write this

    return SCM_UNSPECIFIED;
}

/**
**	Register CCL features for dependencies.
*/
global void DependenciesCclRegister(void)
{
    gh_new_procedureN("define-dependency",CclDefineDependency);
    gh_new_procedure1_0("get-dependency",CclGetDependency);
}

#endif	// defined(USE_CCL) || defined(USE_CCL2)

//@}
