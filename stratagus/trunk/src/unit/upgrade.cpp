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
/**@name upgrade.c	-	The upgrade/allow functions. */
//
//	(c) Copyright 1999-2001 by Vladi Belperchinov-Shabanski
//
//	FreeCraft is free software; you can redistribute it and/or modify
//	it under the terms of the GNU General Public License as published
//	by the Free Software Foundation; either version 2 of the License,
//	or (at your option) any later version.
//
//	FreeCraft is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//	GNU General Public License for more details.
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
#include "player.h"
#include "interface.h"

#include "myendian.h"

#include "etlib/hash.h"

local int AddUpgradeModifierBase(int,int,int,int,int,int,int,int,int*,
	const char*,const char*,const char*,UnitType*);
local int AddUpgradeModifier(int,int,int,int,int,int,int,int,int*,
	const char*,const char*,const char*);

/*----------------------------------------------------------------------------
--	Variables
----------------------------------------------------------------------------*/

/**
**	upgrade type definition
*/
global const char UpgradeType[] = "upgrade";

global Upgrade Upgrades[UpgradeMax];	/// The main user useable upgrades
local int UpgradesCount;		/// Upgrades used

    /// How many upgrades modifiers supported
#define UPGRADE_MODIFIERS_MAX	1024
    /// Upgrades modifiers
local UpgradeModifier* UpgradeModifiers[UPGRADE_MODIFIERS_MAX];
    /// Upgrades modifiers used
local int UpgradeModifiersCount;

#ifdef DOXYGEN				// no real code, only for document

local Upgrade* UpgradeHash[61];		/// lookup table for upgrade names

#else

local hashtable(Upgrade*,61) UpgradeHash;/// lookup table for upgrade names

#endif

local int AllowDone;			/// allow already setup.

/**
**	W*rCr*ft number to internal upgrade name.
*/
local char** UpgradeWcNames;

/*----------------------------------------------------------------------------
--	Functions
----------------------------------------------------------------------------*/

/**
**	Add an upgrade.
**
**	@param ident	upgrade identifier.
**	@param icon	icon displayed for this upgrade,
**			NULL for generated name (icon-<ident>).
**	@param costs	costs to upgrade.
**
**	@return		upgrade id or -1 for error
*/
local Upgrade* AddUpgrade(const char* ident,const char* icon,const int* costs)
{
    char buf[256];
    Upgrade* upgrade;
    Upgrade** tmp;
    int i;

    //	Check for free slot.

    if ( UpgradesCount == UpgradeMax ) {
	DebugLevel0Fn("Upgrades limit reached.\n");
	return NULL;
    }

    //	Fill upgrade structure

    if( (tmp=(Upgrade**)hash_find(UpgradeHash,(char*)ident)) && *tmp ) {
	// FIXME: memory loose!
	DebugLevel0Fn("Already defined upgrade `%s'\n",ident);
	upgrade=*tmp;
    } else {
	upgrade=Upgrades+UpgradesCount++;
	upgrade->OType = UpgradeType;
	upgrade->Ident = strdup( ident );
	*(Upgrade**)hash_add(UpgradeHash,upgrade->Ident)=upgrade;
    }

    if( icon ) {
	upgrade->Icon.Name = strdup(icon);
    } else {				// automatic generated icon-name
	sprintf(buf,"icon-%s",ident+8);
	upgrade->Icon.Name = strdup(buf);
    }

    for( i=0; i<MaxCosts; ++i ) {
	upgrade->Costs[i]=costs[i];
    }

    return upgrade;
}

/**
**	Upgrade by identifier.
**
**	@param ident	The upgrade identifier.
**	@return		Upgrade pointer or NULL if not found.
*/
global Upgrade* UpgradeByIdent(const char* ident)
{
    Upgrade** upgrade;

    if( (upgrade=(Upgrade**)hash_find(UpgradeHash,(char*)ident)) ) {
	return *upgrade;
    }

    DebugLevel0Fn(" upgrade %s not found\n",ident);

    return NULL;
}

/**
**	Init upgrade/allow structures
*/
global void InitUpgrades(void)
{
    int i;

    //
    //	Resolve the icons.
    //
    for( i=0; i<UpgradesCount; ++i ) {
	Upgrades[i].Icon.Icon=IconByIdent(Upgrades[i].Icon.Name);
    }
}

/**
**	Cleanup the upgrade module.
*/
global void CleanUpgrades(void)
{
    int i;

    DebugLevel0Fn("FIXME: not complete written\n");

    //
    //	Free the upgrades.
    //
    for( i=0; i<UpgradesCount; ++i ) {
	// FIXME: hash_del not supported
	*(Upgrade**)hash_add(UpgradeHash,Upgrades[i].Ident)=NULL;
	free(Upgrades[i].Ident);
	free(Upgrades[i].Icon.Name);
    }
    UpgradesCount=0;

    //
    //	Free the upgrade modifiers.
    //
    for( i=0; i<UpgradeModifiersCount; ++i ) {
	free(UpgradeModifiers[i]);
    }

    UpgradeModifiersCount = 0;
}

/**
**	Parse ALOW area from puds.
**
**	@param alow	Pointer to alow area.
**	@param length	length of alow area.
*/
global void ParsePudALOW(const char* alow,int length)
{
    // FIXME: must loaded from config files
    // units allow bits -> internal names.
    static char* units[] = {
	"unit-footman",		"unit-grunt",
	"unit-peasant",		"unit-peon",
	"unit-ballista",	"unit-catapult",
	"unit-knight",		"unit-ogre",
	"unit-archer",		"unit-axethrower",
	"unit-mage",		"unit-death-knight",
	"unit-human-oil-tanker","unit-orc-oil-tanker",
	"unit-elven-destroyer",	"unit-troll-destroyer",
	"unit-human-transport",	"unit-orc-transport",
	"unit-battleship",	"unit-ogre-juggernaught",
	"unit-gnomish-submarine","unit-giant-turtle",
	"unit-gnomish-flying-machine", "unit-goblin-zeppelin",
	"unit-gryphon-rider",	"unit-dragon",
	NULL,			NULL,
	"unit-dwarves",		"unit-goblin-sappers",
	"unit-gryphon-aviary",	"unit-dragon-roost",
	"unit-farm",		"unit-pig-farm",
	"unit-human-barracks",	"unit-orc-barracks",
	"unit-gryphon-aviary",	"unit-dragon-roost",
	"unit-elven-lumber-mill","unit-troll-lumber-mill",
	"unit-stables",		"unit-ogre-mound",
	"unit-mage-tower",	"unit-temple-of-the-damned",
	"unit-human-foundry",	"unit-orc-foundry",
	"unit-human-refinery",	"unit-orc-refinery",
	"unit-gnomish-inventor","unit-goblin-alchemist",
	"unit-church",		"unit-altar-of-storms",
	"unit-human-watch-tower","unit-orc-watch-tower",
	"unit-town-hall",	"unit-great-hall",
	"unit-keep",		"unit-stronghold",
	"unit-castle",		"unit-fortress",
	"unit-human-blacksmith","unit-orc-blacksmith",
	"unit-human-shipyard",	"unit-orc-shipyard",
	"unit-human-wall",	"unit-orc-wall",
    };
    // spell allow bits -> internal names.
    static char* spells[] = {
	"upgrade-holy-vision",
	"upgrade-healing",
	NULL,
	"upgrade-exorcism",
	"upgrade-flame-shield",
	"upgrade-fireball",
	"upgrade-slow",
	"upgrade-invisibility",
	"upgrade-polymorph",
	"upgrade-blizzard",
	"upgrade-eye-of-kilrogg",
	"upgrade-bloodlust",
	NULL,
	"upgrade-raise-dead",
	"upgrade-death-coil",
	"upgrade-whirlwind",
	"upgrade-haste",
	"upgrade-unholy-armor",
	"upgrade-runes",
	"upgrade-death-and-decay",
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
    };
    // upgrades allow bits -> internal names.
    static char* upgrades[] = {
	"upgrade-arrow1",		"upgrade-throwing-axe1",
	"upgrade-arrow2",		"upgrade-throwing-axe2",
	"upgrade-sword1",		"upgrade-battle-axe1",
	"upgrade-sword2"	,	"upgrade-battle-axe2",
	"upgrade-human-shield1",	"upgrade-orc-shield1",
	"upgrade-human-shield2",	"upgrade-orc-shield2",
	"upgrade-human-ship-cannon1",	"upgrade-orc-ship-cannon1",
	"upgrade-human-ship-cannon2",	"upgrade-orc-ship-cannon2",
	"upgrade-human-ship-armor1",	"upgrade-orc-ship-armor1",
	"upgrade-human-ship-armor2",	"upgrade-orc-ship-armor2",
	NULL,				NULL,
	NULL,				NULL,
	"upgrade-catapult1",		"upgrade-ballista1",
	"upgrade-catapult2",		"upgrade-ballista2",
	NULL,				NULL,
	NULL,				NULL,
	"upgrade-ranger",		"upgrade-berserker",
	"upgrade-longbow",		"upgrade-light-axes",
	"upgrade-ranger-scouting",	"upgrade-berserker-scouting",
	"upgrade-ranger-marksmanship",	"upgrade-berserker-regeneration",
	"upgrade-paladin",		"upgrade-ogre-mage",
	NULL,				NULL,
	NULL,				NULL,
	NULL,				NULL,
	NULL,				NULL,
	NULL,				NULL,
	NULL,				NULL,
	NULL,				NULL,
	NULL,				NULL,
	NULL,				NULL,
	NULL,				NULL,
	NULL,				NULL,
    };
    int i;
    int b;
    Player* player;

    DebugLevel0Fn(" Length %d FIXME: constant must be moved to ccl\n",length);

    InitUpgrades();

    //
    //	Allow units
    //
    for( player=Players; player<Players+16; ++player ) {
	for( i=0; i<4; ++i ) {
	    int v;

	    v=*alow++;
	    DebugLevel3Fn(" %x\n",v);
	    for( b=0; b<8; ++b ) {
		if( v&(1<<b) ) {
		    if( units[i*16+0+b*2] ) {
			DebugLevel3Fn(" %s +\n",
				units[i*16+0+b*2]);

			AllowUnitByIdent(player,units[i*16+0+b*2],'A');
			AllowUnitByIdent(player,units[i*16+1+b*2],'A');
		    }
		} else {
		    if( units[i*16+0+b*2] ) {
			DebugLevel3Fn(" %s -\n",
				units[i*16+0+b*2]);

			AllowUnitByIdent(player,units[i*16+0+b*2],'F');
			AllowUnitByIdent(player,units[i*16+1+b*2],'F');
		    }
		}
	    }
	}
    }

    //
    //	Spells start with
    //
    for( player=Players; player<Players+16; ++player ) {
	for( i=0; i<4; ++i ) {
	    int v;

	    v=*alow++;
	    DebugLevel3Fn(" %x\n",v);
	    for( b=0; b<8; ++b ) {
		if( v&(1<<b) ) {
		    if( spells[i*8+b] ) {
			DebugLevel0Fn(" %d %s +R\n",
				player->Player,spells[i*8+b]);

			AllowUpgradeByIdent(player,spells[i*8+b],'R');
		    }
		} else {
		    if( spells[i*8+b] ) {
			DebugLevel0Fn(" %d %s -F\n",
				player->Player,spells[i*8+b]);

			AllowUpgradeByIdent(player,spells[i*8+b],'F');
		    }
		}
	    }
	}
    }

    //
    //	Spells allowed
    //
    for( player=Players; player<Players+16; ++player ) {
	for( i=0; i<4; ++i ) {
	    int v;

	    v=*alow++;
	    DebugLevel3Fn(" %x\n",v);
	    for( b=0; b<8; ++b ) {
		if( v&(1<<b) ) {
		    if( spells[i*8+b] ) {
			DebugLevel0Fn(" %d %s +A\n",
				player->Player,spells[i*8+b]);

			AllowUpgradeByIdent(player,spells[i*8+b],'A');
		    }
		}
	    }
	}
    }

    //
    //	Spells researched
    //
    for( player=Players; player<Players+16; ++player ) {
	for( i=0; i<4; ++i ) {
	    int v;

	    v=*alow++;
	    DebugLevel3Fn(" %x\n",v);
	    for( b=0; b<8; ++b ) {
		if( v&(1<<b) ) {
		    if( spells[i*8+b] ) {
			DebugLevel0Fn(" %d %s +U\n",
				player->Player,spells[i*8+b]);

			AllowUpgradeByIdent(player,spells[i*8+b],'U');
		    }
		}
	    }
	}
    }

    //
    // Upgrades allowed
    //
    for( player=Players; player<Players+16; ++player ) {
	for( i=0; i<4; ++i ) {
	    int v;

	    v=*alow++;
	    DebugLevel3Fn(" %x\n",v);
	    for( b=0; b<8; ++b ) {
		if( v&(1<<b) ) {
		    if( upgrades[i*16+b*2+0] ) {
			DebugLevel0Fn(" %d %s +A\n",
				player->Player,upgrades[i*16+b*2]);

			AllowUpgradeByIdent(player,upgrades[i*16+b*2+0],'A');
			AllowUpgradeByIdent(player,upgrades[i*16+b*2+1],'A');
		    }
		}
	    }
	}
    }

    //
    // Upgrades acquired
    //
    for( player=Players; player<Players+16; ++player ) {
	for( i=0; i<4; ++i ) {
	    int v;

	    v=*alow++;
	    DebugLevel3Fn(" %x\n",v);
	    for( b=0; b<8; ++b ) {
		if( v&(1<<b) ) {
		    if( upgrades[i*16+b*2+0] ) {
			DebugLevel0Fn(" %d %s +U\n",
				player->Player,upgrades[i*16+b*2]);

			AllowUpgradeByIdent(player,upgrades[i*16+b*2+0],'U');
			AllowUpgradeByIdent(player,upgrades[i*16+b*2+1],'U');
		    }
		}
	    }
	}
    }

}

/**
**	Parse UGRD area from puds.
**
**	@param ugrd	Pointer to ugrd area.
**	@param length	length of ugrd area.
*/
global void ParsePudUGRD(const char* ugrd,int length)
{
    int i;
    int time;
    int gold;
    int lumber;
    int oil;
    int icon;
    int group;
    int flags;
    int costs[MaxCosts];

    DebugLevel3Fn(" Length %d\n",length);
    DebugCheck( length!=780 );

    for( i=0; i<52; ++i ) {
	time=((unsigned char*)ugrd)[i];
	gold=AccessLE16(	ugrd+52+(i)*2);
	lumber=AccessLE16(	ugrd+52+(i+52)*2);
	oil=AccessLE16(		ugrd+52+(i+52+52)*2);
	icon=AccessLE16(	ugrd+52+(i+52+52+52)*2);
	group=AccessLE16(	ugrd+52+(i+52+52+52+52)*2);
	flags=AccessLE16(	ugrd+52+(i+52+52+52+52+52)*2);
	DebugLevel3Fn(" (%d)%s %d,%d,%d,%d (%d)%s %d %08X\n"
		,i,UpgradeWcNames[i]
		,time,gold,lumber,oil
		,icon,IconWcNames[icon],group,flags);

	memset(costs,0,sizeof(costs));
	costs[TimeCost]=time;
	costs[GoldCost]=gold;
	costs[WoodCost]=lumber;
	costs[OilCost]=oil;
	AddUpgrade(UpgradeWcNames[i],IconWcNames[icon],costs);

	// group+flags are to mystic to be implemented
    }
}

/**
**	save state of the dependencies to file.
**
**	@param file	Output file.
*/
global void SaveUpgrades(FILE* file)
{
    int i;
    int j;
    int p;

    fprintf(file,"\n;;; -----------------------------------------\n");
    fprintf(file,";;; MODULE: upgrades $Id$\n\n");

    //
    //	Save all upgrades
    //
    for( i=0; i<UpgradesCount; ++i ) {
	fprintf(file,"(define-upgrade '%s 'icon '%s\n"
		,Upgrades[i].Ident,Upgrades[i].Icon.Name);
	fprintf(file,"  'costs #(");
	for( j=0; j<MaxCosts; ++j ) {
	    fprintf(file," %5d",Upgrades[i].Costs[j]);
	}

	fprintf(file,"))\n");
    }
    fprintf(file,"\n");

    //
    //	Save all upgrade modifiers.
    //
    for( i=0; i<UpgradeModifiersCount; ++i ) {
	fprintf(file,"(define-modifier '%s",
		Upgrades[UpgradeModifiers[i]->UpgradeId].Ident);

	if( UpgradeModifiers[i]->Modifier.AttackRange ) {
	    fprintf(file,"\n  '(attack-range %d)"
		    ,UpgradeModifiers[i]->Modifier.AttackRange );
	}
	if( UpgradeModifiers[i]->Modifier.SightRange ) {
	    fprintf(file,"\n  '(sight-range %d)"
		    ,UpgradeModifiers[i]->Modifier.SightRange );
	}
	if( UpgradeModifiers[i]->Modifier.BasicDamage ) {
	    fprintf(file,"\n  '(basic-damage %d)"
		    ,UpgradeModifiers[i]->Modifier.BasicDamage );
	}
	if( UpgradeModifiers[i]->Modifier.PiercingDamage ) {
	    fprintf(file,"\n  '(piercing-damage %d)"
		    ,UpgradeModifiers[i]->Modifier.PiercingDamage );
	}
	if( UpgradeModifiers[i]->Modifier.Armor ) {
	    fprintf(file,"\n  '(armor %d)"
		    ,UpgradeModifiers[i]->Modifier.Armor );
	}
	if( UpgradeModifiers[i]->Modifier.Speed ) {
	    fprintf(file,"\n  '(speed %d)"
		    ,UpgradeModifiers[i]->Modifier.Speed );
	}
	if( UpgradeModifiers[i]->Modifier.HitPoints ) {
	    fprintf(file,"\n  '(hit-points %d)"
		    ,UpgradeModifiers[i]->Modifier.HitPoints );
	}

	for( j=0; j<MaxCosts; ++j ) {
	    if( UpgradeModifiers[i]->Modifier.Costs[j] ) {
		fprintf(file,"\n  '(%s-cost %d)"
		    ,DEFAULT_NAMES[j],UpgradeModifiers[i]->Modifier.Costs[j]);
	    }
	}

	for( j=0; j<UnitTypeMax; ++j ) {	// allow/forbid units
	    if( UpgradeModifiers[i]->ChangeUnits[j]!='?' ) {
		fprintf(file,"\n  '(allow %s %d)",
			UnitTypes[j].Ident,
			UpgradeModifiers[i]->ChangeUnits[j]);
	    }
	}

	for( j=0; j<UpgradeMax; ++j ) {		// allow/forbid upgrades
	    if( UpgradeModifiers[i]->ChangeUpgrades[j]!='?' ) {
		fprintf(file,"\n  '(allow %s %c)",Upgrades[j].Ident,
			UpgradeModifiers[i]->ChangeUpgrades[j]);
	    }
	}

	for( j=0; j<UnitTypeMax; ++j ) {	// apply to units
	    if( UpgradeModifiers[i]->ApplyTo[j]!='?' ) {
		fprintf(file,"\n  '(apply-to %s)",UnitTypes[j].Ident);
	    }
	}

	if( UpgradeModifiers[i]->ConvertTo ) {
	    fprintf(file,"\n  '(convert-to %s)",
		    ((UnitType*)UpgradeModifiers[i]->ConvertTo)->Ident);
	}

	fprintf(file,")\n\n");
    }

    //
    //	Save the allow
    //
    for( i=0; UnitTypes[i].OType; ++i ) {
	fprintf(file,"(define-allow '%s\t",UnitTypes[i].Ident);
	if( strlen(UnitTypes[i].Ident)<9 ) {
	    fprintf(file,"\t\t\t\"");
	} else if( strlen(UnitTypes[i].Ident)<17 ) {
	    fprintf(file,"\t\t\"");
	} else if( strlen(UnitTypes[i].Ident)<25 ) {
	    fprintf(file,"\t\"");
	} else {
	    fprintf(file,"\"");
	}
	for( p=0; p<PlayerMax; ++p ) {
	    fprintf(file,"%c",Players[p].Allow.Units[i]);
	}
	fprintf(file,"\")\n");
    }
    fprintf(file,"\n");

    //
    //	Save the upgrades
    //
    for( i=0; i<UpgradesCount; ++i ) {
	fprintf(file,"(define-allow '%s\t",Upgrades[i].Ident);
	if( strlen(Upgrades[i].Ident)<9 ) {
	    fprintf(file,"\t\t\t\"");
	} else if( strlen(Upgrades[i].Ident)<17 ) {
	    fprintf(file,"\t\t\"");
	} else if( strlen(Upgrades[i].Ident)<25 ) {
	    fprintf(file,"\t\"");
	} else {
	    fprintf(file,"\"");
	}
	for( p=0; p<PlayerMax; ++p ) {
	    fprintf(file,"%c",Players[p].Allow.Upgrades[i]);
	}
	fprintf(file,"\"");
	fprintf(file,")\n");
    }
}

/*----------------------------------------------------------------------------
--	Ccl part of upgrades
----------------------------------------------------------------------------*/

#include "ccl.h"

/**
**	Define a new upgrade modifier.
**
**	@param list	List of modifiers.
*/
local SCM CclDefineModifier(SCM list)
{
    SCM temp;
    SCM value;
    int uid;
    char* str;
    int attack_range;
    int sight_range;
    int basic_damage;
    int piercing_damage;
    int armor;
    int speed;
    int hit_points;
    int costs[MaxCosts];
    char units[UnitTypeMax];
    char upgrades[UpgradeMax];
    char apply_to[UnitTypeMax];
    UnitType* convert_to;

    attack_range=0;
    sight_range=0;
    basic_damage=0;
    piercing_damage=0;
    armor=0;
    speed=0;
    hit_points=0;
    memset(costs,0,sizeof(costs));
    memset(units,'?',sizeof(units));
    memset(upgrades,'?',sizeof(upgrades));
    memset(apply_to,'?',sizeof(apply_to));
    convert_to=NULL;

    value=gh_car(list);
    list=gh_cdr(list);

    str=gh_scm2newstr(value,NULL);
    uid=UpgradeIdByIdent(str);
    free(str);

    while( !gh_null_p(list) ) {
	value=gh_car(list);
	list=gh_cdr(list);
	if( !gh_list_p(value) ) {
	    errl("wrong tag",value);
	    return SCM_UNSPECIFIED;
	}
	temp=gh_car(value);
	if( gh_eq_p(temp,gh_symbol2scm("attack-range")) ) {
	    attack_range=gh_scm2int(gh_cadr(value));
	} else if( gh_eq_p(temp,gh_symbol2scm("sight-range")) ) {
	    sight_range=gh_scm2int(gh_cadr(value));
	} else if( gh_eq_p(temp,gh_symbol2scm("attack-range")) ) {
	    attack_range=gh_scm2int(gh_cadr(value));
	} else if( gh_eq_p(temp,gh_symbol2scm("basic-damage")) ) {
	    basic_damage=gh_scm2int(gh_cadr(value));
	} else if( gh_eq_p(temp,gh_symbol2scm("piercing-damage")) ) {
	    piercing_damage=gh_scm2int(gh_cadr(value));
	} else if( gh_eq_p(temp,gh_symbol2scm("armor")) ) {
	    armor=gh_scm2int(gh_cadr(value));
	} else if( gh_eq_p(temp,gh_symbol2scm("speed")) ) {
	    speed=gh_scm2int(gh_cadr(value));
	} else if( gh_eq_p(temp,gh_symbol2scm("hit-points")) ) {
	    hit_points=gh_scm2int(gh_cadr(value));
	} else if( gh_eq_p(temp,gh_symbol2scm("time-cost")) ) {
	    costs[0]=gh_scm2int(gh_cadr(value));
	} else if( gh_eq_p(temp,gh_symbol2scm("gold-cost")) ) {
	    costs[1]=gh_scm2int(gh_cadr(value));
	} else if( gh_eq_p(temp,gh_symbol2scm("wood-cost")) ) {
	    costs[2]=gh_scm2int(gh_cadr(value));
	} else if( gh_eq_p(temp,gh_symbol2scm("oil-cost")) ) {
	    costs[3]=gh_scm2int(gh_cadr(value));
	} else if( gh_eq_p(temp,gh_symbol2scm("ore-cost")) ) {
	    costs[4]=gh_scm2int(gh_cadr(value));
	} else if( gh_eq_p(temp,gh_symbol2scm("stone-cost")) ) {
	    costs[5]=gh_scm2int(gh_cadr(value));
	} else if( gh_eq_p(temp,gh_symbol2scm("coal-cost")) ) {
	    costs[6]=gh_scm2int(gh_cadr(value));
	} else if( gh_eq_p(temp,gh_symbol2scm("allow")) ) {
	    value=gh_cdr(value);
	    str=gh_scm2newstr(gh_car(value),NULL);
	    value=gh_cdr(value);
	    DebugLevel3Fn("%s\n",str);
	    if( !strncmp(str,"upgrade-",8) ) {
		upgrades[UpgradeIdByIdent(str)]=gh_scm2int(gh_car(value));
	    } else if( !strncmp(str,"unit-",5) ) {
		units[UnitTypeIdByIdent(str)]=gh_scm2int(gh_car(value));
	    } else {
		free(str);
		errl("upgrade or unit expected",NIL);
	    }
	    free(str);
	} else if( gh_eq_p(temp,gh_symbol2scm("apply-to")) ) {
	    value=gh_cdr(value);
	    str=gh_scm2newstr(gh_car(value),NULL);
	    apply_to[UnitTypeIdByIdent(str)]='X';
	    free(str);
	} else if( gh_eq_p(temp,gh_symbol2scm("convert-to")) ) {
	    value=gh_cdr(value);
	    str=gh_scm2newstr(gh_car(value),NULL);
	    convert_to=UnitTypeByIdent(str);
	    free(str);
	} else {
	    errl("wrong tag",temp);
	    return SCM_UNSPECIFIED;
	}
    }

    AddUpgradeModifierBase(uid,attack_range,sight_range,
	    basic_damage,piercing_damage,armor,speed,hit_points,costs,
	    units,upgrades,apply_to,convert_to);

    return SCM_UNSPECIFIED;
}

/**
**	Define a new upgrade.
**
**	@param list	List defining the upgrade.
*/
local SCM CclDefineUpgrade(SCM list)
{
    SCM value;
    char* str;
    char* icon;
    char* ident;
    int costs[MaxCosts];
    int n;
    int j;

    //	Identifier

    ident=gh_scm2newstr(gh_car(list),NULL);
    list=gh_cdr(list);

    icon=NULL;
    memset(costs,0,sizeof(costs));

    while( !gh_null_p(list) ) {
	value=gh_car(list);
	list=gh_cdr(list);
	if( gh_eq_p(value,gh_symbol2scm("icon")) ) {
	    //	Icon

	    if( icon ) {
		free(icon);
	    }
	    icon=gh_scm2newstr(gh_car(list),NULL);
	    list=gh_cdr(list);
	} else if( gh_eq_p(value,gh_symbol2scm("costs")) ) {
	    //	Costs

	    value=gh_car(list);
	    list=gh_cdr(list);
	    n=gh_vector_length(value);
	    if( n<4 || n>MaxCosts ) {
		fprintf(stderr,"%s: Wrong vector length\n",ident);
		if( n>MaxCosts ) {
		    n=MaxCosts;
		}
	    }
	    for( j=0; j<n; ++j ) {
		costs[j]=gh_scm2int(gh_vector_ref(value,gh_int2scm(j)));
	    }
	    while( j<MaxCosts ) {
		costs[j++]=0;
	    }
	} else {
	    str=gh_scm2newstr(value,NULL);
	    fprintf(stderr,"%s: Wrong tag `%s'\n",ident,str);
	    free(str);
	}
    }

    AddUpgrade(ident,icon,costs);
    free(ident);
    free(icon);

    return SCM_UNSPECIFIED;
}

/**
**	Define which units/upgrades are allowed.
*/
local SCM CclDefineAllow(SCM list)
{
    SCM value;
    char* str;
    char* ids;
    int i;
    int n;

    while( !gh_null_p(list) ) {
	value=gh_car(list);
	list=gh_cdr(list);
	str=gh_scm2newstr(value,NULL);
	value=gh_car(list);
	list=gh_cdr(list);
	ids=gh_scm2newstr(value,NULL);

	n=strlen(ids);
	if( n>16 ) {
	    fprintf(stderr,"%s: Allow string too long %d\n",str,n);
	    n=16;
	}

	for( i=0; i<n; ++i ) {
	    AllowByIdent(&Players[i],str,ids[i]);
	}

	free(str);
	free(ids);
    }

    return SCM_UNSPECIFIED;
}

/**
**	Define upgrade mapping from original number to internal symbol
**
**	@param list	List of all names.
*/
local SCM CclDefineUpgradeWcNames(SCM list)
{
    int i;
    char** cp;

    if( (cp=UpgradeWcNames) ) {		// Free all old names
	while( *cp ) {
	    free(*cp++);
	}
	free(UpgradeWcNames);
    }

    //
    //	Get new table.
    //
    i=gh_length(list);
    UpgradeWcNames=cp=malloc((i+1)*sizeof(char*));
    while( i-- ) {
	*cp++=gh_scm2newstr(gh_car(list),NULL);
	list=gh_cdr(list);
    }
    *cp=NULL;

    return SCM_UNSPECIFIED;
}

/**
**	Register CCL features for upgrades.
*/
global void UpgradesCclRegister(void)
{
    gh_new_procedureN("define-modifier",CclDefineModifier);
    gh_new_procedureN("define-upgrade",CclDefineUpgrade);
    gh_new_procedureN("define-allow",CclDefineAllow);

    gh_new_procedureN("define-upgrade-wc-names",CclDefineUpgradeWcNames);
}


































// FIXME: Johns stops here

/*----------------------------------------------------------------------------
--	Init/Done/Add functions
----------------------------------------------------------------------------*/

// returns upgrade modifier id or -1 for error ( actually this id is useless, just error checking )
local int AddUpgradeModifierBase(int uid,int attack_range,int sight_range,
    int basic_damage,int piercing_damage,int armor,int speed,
    int hit_points,int* costs,const char* af_units,
    const char* af_upgrades,const char* apply_to,UnitType* convert_to)
{
    int i;
    UpgradeModifier *um;

    um=(UpgradeModifier*)malloc(sizeof(UpgradeModifier));
    if( !um ) {
	return -1;
    }

    um->UpgradeId = uid;

    // get/save stats modifiers
    um->Modifier.AttackRange	= attack_range;
    um->Modifier.SightRange	= sight_range;
    um->Modifier.BasicDamage	= basic_damage;
    um->Modifier.PiercingDamage	= piercing_damage;
    um->Modifier.Armor		= armor;
    um->Modifier.Speed		= speed;
    um->Modifier.HitPoints	= hit_points;

    for( i=0; i<MaxCosts; ++i ) {
	um->Modifier.Costs[i]	= costs[i];
    }

    memcpy(um->ChangeUnits,af_units,sizeof(um->ChangeUnits));
    memcpy(um->ChangeUpgrades,af_upgrades,sizeof(um->ChangeUpgrades));
    memcpy(um->ApplyTo,apply_to,sizeof(um->ApplyTo));

    um->ConvertTo=convert_to;

    UpgradeModifiers[UpgradeModifiersCount] = um;

    return UpgradeModifiersCount++;
}


// returns upgrade modifier id or -1 for error ( actually this id is useless, just error checking )
local int AddUpgradeModifier( int uid,
    int attack_range,
    int sight_range,
    int basic_damage,
    int piercing_damage,
    int armor,
    int speed,
    int hit_points,

    int* costs,

    // following are comma separated list of required string id's

    const char* af_units,    // "A:unit-mage,F:unit-grunt" -- allow mages, forbid grunts
    const char* af_upgrades, // "F:upgrade-Shield1,R:upgrade-ShieldTotal" -- :)
    const char* apply_to	    // "unit-Peon,unit-Peasant"

    )
{
    char *s1;
    char *s2;
    int i;
    UpgradeModifier *um;

    um=(UpgradeModifier*)malloc(sizeof(UpgradeModifier));
    if( !um ) {
	return -1;
    }

    um->UpgradeId = uid;

    // get/save stats modifiers
    um->Modifier.AttackRange	= attack_range;
    um->Modifier.SightRange	= sight_range;
    um->Modifier.BasicDamage	= basic_damage;
    um->Modifier.PiercingDamage	= piercing_damage;
    um->Modifier.Armor		= armor;
    um->Modifier.Speed		= speed;
    um->Modifier.HitPoints	= hit_points;

    for( i=0; i<MaxCosts; ++i ) {
	um->Modifier.Costs[i]	= costs[i];
    }

    // FIXME: all the thing below is sensitive to the format of the string!
    // FIXME: it will be good if things are checked for errors better!
    // FIXME: perhaps the function `strtok()' should be replaced with local one?

    memset( um->ChangeUnits,    '?', sizeof(um->ChangeUnits)   );
    memset( um->ChangeUpgrades, '?', sizeof(um->ChangeUpgrades));
    memset( um->ApplyTo,        '?', sizeof(um->ApplyTo)       );

    //
    // get allow/forbid's for units
    //
    s1 = strdup( af_units );
    DebugCheck(!s1);
    for( s2 = strtok( s1, "," ); s2; s2=strtok( NULL, "," ) ) {
	int id;
	DebugCheck(! (s2[0] == 'A' || s2[0] == 'F' ));
	DebugCheck(! (s2[1] == ':' ));
	id = UnitTypeIdByIdent( s2+2 );
	if ( id == -1 ) {
	    continue;		// should we cancel all and return error?!
	}
	um->ChangeUnits[id] = s2[0];
    }
    free(s1);

    //
    // get allow/forbid's for upgrades
    //
    s1 = strdup( af_upgrades );
    DebugCheck(!s1);
    for( s2 = strtok( s1, "," ); s2; s2=strtok( NULL, "," ) ) {
	int id;
	DebugCheck(!( s2[0] == 'A' || s2[0] == 'F' || s2[0] == 'R' ));
	DebugCheck(!( s2[1] == ':' ));
	id = UpgradeIdByIdent( s2+2 );
	if ( id == -1 ) {
	    continue;		// should we cancel all and return error?!
	}
	um->ChangeUpgrades[id] = s2[0];
    }
    free(s1);

    //
    // get units that are affected by this upgrade
    //
    s1 = strdup( apply_to );
    DebugCheck(!s1);
    for( s2 = strtok( s1, "," ); s2; s2=strtok( NULL, "," ) ) {
	int id;

	DebugLevel3Fn(" %s\n",s2);
	id = UnitTypeIdByIdent( s2 );
	if ( id == -1 ) {
	    break;		// cade: should we cancel all and return error?!
	}
	um->ApplyTo[id] = 'X';	// something other than '?'
    }
    free(s1);

    UpgradeModifiers[UpgradeModifiersCount] = um;
    UpgradeModifiersCount++;

    return UpgradeModifiersCount-1;
}

// this function is used for define `simple' upgrades
// with only one modifier
global void AddSimpleUpgrade( const char* ident,
    const char* icon,
    // upgrade costs
    int* costs,
    // upgrade modifiers
    int attack_range,
    int sight_range,
    int basic_damage,
    int piercing_damage,
    int armor,
    int speed,
    int hit_points,

    int* mcosts,

    const char* apply_to		// "unit-Peon,unit-Peasant"
    )
{
    Upgrade* up;

    up = AddUpgrade(ident,icon,costs);
    if ( !up )  {
	return;
    }
    AddUpgradeModifier(up-Upgrades,attack_range,sight_range,basic_damage,
	    piercing_damage,armor,speed,hit_points,
	    mcosts,
	    "","", // no allow/forbid maps
	    apply_to);
}

/*----------------------------------------------------------------------------
--	General/Map functions
----------------------------------------------------------------------------*/

// AllowStruct and UpgradeTimers will be static in the player so will be
// load/saved with the player struct

/**
**	UnitType ID by identifier.
**
**	@param ident	The unit-type identifier.
**	@return		Unit-type ID (int) or -1 if not found.
*/
global int UnitTypeIdByIdent(const char* sid)
{
    UnitType* type;

    if( (type=UnitTypeByIdent(sid)) ) {
	return type->Type;
    }
    DebugLevel0Fn(" fix this %s\n",sid);
    return -1;
}

/**
**	Upgrade ID by identifier.
**
**	@param ident	The upgrade identifier.
**	@return		Upgrade ID (int) or -1 if not found.
*/
global int UpgradeIdByIdent(const char* sid)
{
    Upgrade* upgrade;

    upgrade=UpgradeByIdent(sid);
    if( upgrade ) {
	return upgrade-Upgrades;
    }
    DebugLevel0Fn(" fix this %s\n",sid);
    return -1;
}

/*----------------------------------------------------------------------------
--	Upgrades
----------------------------------------------------------------------------*/

/**
**	Increment the counter of an upgrade.
**
**	Amount==-1 to cancel upgrade, could happen when building destroyed
**	during upgrade. Using this we could have one upgrade research in two
**	buildings, so we can have this upgrade faster.
**
**	@param player	Player pointer of the incremented upgrade.
**	@param id	Upgrade id number.
**	@param amount	Value to add to timer. -1 to cancel upgrade
*/
global void UpgradeIncTime(Player * player, int id, int amount)
{
    player->UpgradeTimers.Upgrades[id] += amount;
    if (player->UpgradeTimers.Upgrades[id] >= Upgrades[id].Costs[TimeCost]) {
	player->UpgradeTimers.Upgrades[id] = Upgrades[id].Costs[TimeCost];
	UpgradeAcquire(player, &Upgrades[id]);
    }
}

/**
**	Convert unit-type to.
**
**	@param player	For this player.
**	@param src	From this unit-type.
**	@param dst	To this unit-type.
*/
local void ConvertUnitTypeTo(Player* player,const UnitType* src,UnitType* dst)
{
    Unit* unit;
    int i;
    int j;

    for( i=0; i<player->TotalNumUnits; ++i ) {
	unit=player->Units[i];
	//
	//	Convert already existing units to this type.
	//
	if( unit->Type==src ) {
	    unit->HP+=dst->Stats[player->Player].HitPoints
		    -unit->Stats->HitPoints;
	    // don't have such unit now
	    player->UnitTypesCount[src->Type]--;
	    unit->Type=dst;
	    unit->Stats=&dst->Stats[player->Player];
	    // and we have new one...
	    player->UnitTypesCount[dst->Type]++;
	    UpdateForNewUnit(unit,1);
	    if( dst->CanCastSpell ) {
		unit->Mana=MAGIC_FOR_NEW_UNITS;
	    }
	    CheckUnitToBeDrawn(unit);
	//
	//	Convert trained units to this type.
	//	FIXME: what about buildings?
	//
	} else {
	    if( unit->Orders[0].Action==UnitActionTrain ) {
		for( j=0; j<unit->Data.Train.Count; ++j ) {
		     if( unit->Data.Train.What[j]==src ) {
			unit->Data.Train.What[j]=dst; 
			if( IsOnlySelected(unit) ) {
			    MustRedraw|=RedrawInfoPanel;
			}
		     }
		}
	    }
	    for( j=1; j<unit->OrderCount; ++j ) {
		if( unit->Orders[j].Action==UnitActionTrain	
			&& unit->Orders[j].Type==src ) {
		    unit->Orders[j].Type=dst; 
		}
	    }
	}
    }
}

/**
**	Apply the modifiers of an upgrade.
**
**	This function will mark upgrade done and do all required modifications
**	to unit types and will modify allow/forbid maps
**
**	@param player	Player that get all the upgrades.
**	@param um	Upgrade modifier that do the effects
*/
local void ApplyUpgradeModifier(Player * player, const UpgradeModifier * um)
{
    int z;
    int j;
    int pn;

    pn = player->Player;		// player number
    for (z = 0; z < UpgradeMax; z++) {
	// allow/forbid upgrades for player.  only if upgrade is not acquired
	if (player->Allow.Upgrades[z] != 'R') {
	    if (um->ChangeUpgrades[z] == 'A') {
		player->Allow.Upgrades[z] = 'A';
	    }
	    if (um->ChangeUpgrades[z] == 'F') {
		player->Allow.Upgrades[z] = 'F';
	    }
	    // we can even have upgrade acquired w/o costs
	    if (um->ChangeUpgrades[z] == 'R') {
		player->Allow.Upgrades[z] = 'R';
	    }
	}
    }

    for (z = 0; z < UnitTypeMax; z++) {
	// allow/forbid unit types for player
	if (um->ChangeUnits[z] == 'A') {
	    player->Allow.Units[z] = 'A';
	}
	if (um->ChangeUnits[z] == 'F') {
	    player->Allow.Units[z] = 'F';
	}

	DebugCheck(!(um->ApplyTo[z] == '?' || um->ApplyTo[z] == 'X'));

	// this modifier should be applied to unittype id == z
	if (um->ApplyTo[z] == 'X') {

	    DebugLevel3Fn(" applied to %d\n", z);
	    // upgrade stats
	    UnitTypes[z].Stats[pn].AttackRange += um->Modifier.AttackRange;
	    UnitTypes[z].Stats[pn].SightRange += um->Modifier.SightRange;
	    UnitTypes[z].Stats[pn].BasicDamage += um->Modifier.BasicDamage;
	    UnitTypes[z].Stats[pn].PiercingDamage
		    += um->Modifier.PiercingDamage;
	    UnitTypes[z].Stats[pn].Armor += um->Modifier.Armor;
	    UnitTypes[z].Stats[pn].Speed += um->Modifier.Speed;
	    UnitTypes[z].Stats[pn].HitPoints += um->Modifier.HitPoints;

	    // upgrade costs :)
	    for (j = 0; j < MaxCosts; ++j) {
		UnitTypes[z].Stats[pn].Costs[j] += um->Modifier.Costs[j];
	    }

	    UnitTypes[z].Stats[pn].Level++;

	    if( um->ConvertTo ) {
		((UnitType*)um->ConvertTo)->Stats[pn].Level++;
		ConvertUnitTypeTo(player,&UnitTypes[z],um->ConvertTo);
	    }
	}
    }
}

/**
**	Handle that an upgrade was acquired.
**	Called by UpgradeIncTime() when timer reached
**
**	@param player	Player researching the upgrade.
**	@param upgrade	Upgrade ready researched.
*/
global void UpgradeAcquire( Player* player, const Upgrade* upgrade )
{
    int z;
    int id;

    id=upgrade-Upgrades;
    player->UpgradeTimers.Upgrades[id] = upgrade->Costs[TimeCost];
    AllowUpgradeId( player, id, 'R' );		// research done

    for ( z = 0; z < UpgradeModifiersCount; z++ ) {
	if ( UpgradeModifiers[z]->UpgradeId == id ) {
	    ApplyUpgradeModifier( player, UpgradeModifiers[z] );
	}
    }

    //
    //	Upgrades could change the buttons displayed.
    //
    if( player==ThisPlayer ) {
	UpdateButtonPanel();
	MustRedraw|=RedrawInfoPanel;
    }
}

// for now it will be empty?
// perhaps acquired upgrade can be lost if ( for example ) a building is lost
// ( lumber mill? stronghold? )
// this function will apply all modifiers in reverse way
global void UpgradeLost( Player* player, int id )
{
  return; //FIXME: remove this if implemented below

  player->UpgradeTimers.Upgrades[id] = 0;
  AllowUpgradeId( player, id, 'A' ); // research is lost i.e. available
  // FIXME: here we should reverse apply upgrade...
}

/*----------------------------------------------------------------------------
--	Allow(s)
----------------------------------------------------------------------------*/

// all the following functions are just map handlers, no specific notes

/**
**	FIXME: docu
*/
global void AllowUnitId( Player* player, int id, char af ) // id -- unit type id, af -- `A'llow/`F'orbid
{
  DebugCheck(!( af == 'A' || af == 'F' ));
  player->Allow.Units[id] = af;
}

/**
**	FIXME: docu
*/
global void AllowUpgradeId( Player* player,  int id, char af )
{
  DebugCheck(!( af == 'A' || af == 'F' || af == 'R' ));
  player->Allow.Upgrades[id] = af;
}

/**
**	FIXME: docu
*/
global char UnitIdAllowed(const Player* player,  int id )
{
  if ( id < 0 || id >= UpgradeMax ) return 'F';
  return player->Allow.Units[id];
}

/**
**	FIXME: docu
*/
global char UpgradeIdAllowed(const Player* player,  int id )
{
    // JOHNS: Don't be kind, the people should code correct!
    DebugCheck( id < 0 || id >= UpgradeMax );

    return player->Allow.Upgrades[id];
}

// ***************by sid's

/**
**	FIXME: docu
*/
global void UpgradeIncTime2(Player * player, char *sid, int amount)	// by ident string
{
    UpgradeIncTime(player, UpgradeIdByIdent(sid), amount);
}

/**
**	FIXME: docu
*/
global void UpgradeLost2(Player * player, char *sid)	// by ident string
{
    UpgradeLost(player, UpgradeIdByIdent(sid));
}

/**
**	FIXME: docu
*/
global void AllowUnitByIdent(Player * player, const char *sid, char af)
{
    AllowUnitId(player, UnitTypeIdByIdent(sid), af);
}

/**
**	FIXME: docu
*/
global void AllowUpgradeByIdent(Player * player, const char *sid, char af)
{
    AllowUpgradeId(player, UpgradeIdByIdent(sid), af);
}

/**
**	FIXME: docu
*/
global void AllowByIdent(Player* player,  const char* sid, char af )
{
    if( !strncmp(sid,"unit-",5) ) {
	AllowUnitByIdent(player,sid,af);
    } else if( !strncmp(sid,"upgrade-",8) ) {
	AllowUpgradeByIdent(player,sid,af);
    } else {
	DebugLevel0Fn(" wrong sid %s\n",sid);
    }
}

/**
**	FIXME: docu
*/
global char UnitIdentAllowed(const Player * player, const char *sid)
{
    return UnitIdAllowed(player, UnitTypeIdByIdent(sid));
}

/**
**	FIXME: docu
*/
global char UpgradeIdentAllowed(const Player * player, const char *sid)
{
    return UpgradeIdAllowed(player, UpgradeIdByIdent(sid));
}

//@}
