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
/**@name button_table.c	-	The button table. */
//
//	(c) Copyright 1999-2001 by Lutz Sammer, Vladi Belperchinov-Shabanski
//
//	$Id$

//@{

/*----------------------------------------------------------------------------
--	Includes
----------------------------------------------------------------------------*/

#include <stdio.h>
#include <string.h>

#include "freecraft.h"
#include "upgrade.h"
#include "depend.h"
#include "interface.h"
#include "network.h"

/*----------------------------------------------------------------------------
--      Defines
----------------------------------------------------------------------------*/

#ifndef USE_CCL

#define USE_EXTENSIONS		/// Enable our extensions

// Unit lists
// FIXME: Write some comments, for the groups.

/**
**	All human workers.
*/
#define WORKERS_H \
    "unit-peasant,unit-peasant-with-gold,unit-peasant-with-wood"
#define HUMAN_LAND_FORCES \
    "unit-footman,unit-knight,unit-archer,unit-paladin," \
    "unit-dwarves,unit-ranger,unit-ballista"
#define HUMAN_HEROS1 \
    "unit-alleria,unit-kurdan-and-sky-ree,unit-danath,unit-turalyon," \
    "unit-lothar,unit-uther-lightbringer"
#define HUMAN_HEROS \
    HUMAN_HEROS1 ",unit-khadgar"
#define HUMAN_LAND_FORCES2 \
    HUMAN_LAND_FORCES "," WORKERS_H
#define HUMAN_LAND_FORCES3 \
    HUMAN_LAND_FORCES2 "," HUMAN_HEROS
#define HALLS_H \
    "unit-town-hall,unit-keep,unit-castle"
#define HUMAN_AIR_FORCES \
    "unit-gnomish-flying-machine,unit-gryphon-rider"

#define WORKERS_O \
    "unit-peon,unit-peon-with-gold,unit-peon-with-wood"
#define ORC_LAND_FORCES \
    "unit-grunt,unit-ogre,unit-axethrower,unit-ogre-mage," \
    "unit-goblin-sappers,unit-berserker,unit-catapult"
#define ORC_HEROS1 \
    "unit-dentarg,unit-cho-gall," \
    "unit-zuljin,unit-grom-hellscream,unit-korgath-bladefist"
#define ORC_HEROS \
    ORC_HEROS1 ",unit-gul-dan" ",unit-teron-gorefiend"
#define ORC_LAND_FORCES2 \
    ORC_LAND_FORCES "," WORKERS_O
#define ORC_LAND_FORCES3 \
    ORC_LAND_FORCES2 "," ORC_HEROS ",unit-skeleton"
#define HALLS_O \
    "unit-great-hall,unit-stronghold,unit-fortress"
#define ORC_AIR_FORCES \
    "unit-eye-of-kilrogg,unit-dragon,unit-goblin-zeppelin"

#endif

/*----------------------------------------------------------------------------
--      Declaration
----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
--      Variables
----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
--      Function
----------------------------------------------------------------------------*/

/**
**	ButtonCheck for button enabled, always true.
**	This needed to overwrite the internal tests.
**
**	@param unit	Pointer to unit for button.
**	@param button	Pointer to button to check/enable.
**	@return		True if enabled.
*/
global int ButtonCheckTrue(const Unit* unit,const ButtonAction* button)
{
    return 1;
}

/**
**	Check for button enabled, always false.
**	This needed to overwrite the internal tests.
**
**	@param unit	Pointer to unit for button.
**	@param button	Pointer to button to check/enable.
**	@return		True if enabled.
*/
global int ButtonCheckFalse(const Unit* unit,const ButtonAction* button)
{
    return 0;
}

/**
**	Check for button enabled, if upgrade is ready.
**
**	@param unit	Pointer to unit for button.
**	@param button	Pointer to button to check/enable.
**	@return		True if enabled.
*/
global int ButtonCheckUpgrade(const Unit* unit,const ButtonAction* button)
{
    return UpgradeIdentAllowed(unit->Player,button->AllowStr)=='R';
}

/**
**	Check for button enabled, if unit is available.
**
**	@param unit	Pointer to unit for button.
**	@param button	Pointer to button to check/enable.
**	@return		True if enabled.
*/
global int ButtonCheckUnit(const Unit* unit,const ButtonAction* button)
{
    return HaveUnitTypeByIdent(unit->Player,button->AllowStr);
}

/**
**	Check for button enabled, if all units are available.
**
**	@param unit	Pointer to unit for button.
**	@param button	Pointer to button to check/enable.
**	@return		True if enabled.
*/
global int ButtonCheckUnits(const Unit* unit,const ButtonAction* button)
{
    char* buf;
    const char* s;
    Player* player;

    player=unit->Player;
    buf=alloca(strlen(button->AllowStr)+1);
    strcpy(buf,button->AllowStr);
    for( s=strtok(buf,","); s; s=strtok(NULL,",") ) {
	if( !HaveUnitTypeByIdent(player,s) ) {
	    return 0;
	}
    }
    return 1;
}

/**
**	Check if network play is enabled.
**	Needed for walls, which could only be build in network play.
**
**	@param unit	Pointer to unit for button.
**	@param button	Pointer to button to check/enable.
**	@return		True if enabled.
**
**	NOTE: this check could also be moved into intialisation.
*/
global int ButtonCheckNetwork(const Unit* unit,const ButtonAction* button)
{
    return NetworkFildes!=-1;
}

/**
**	Check for button enabled, if the unit isn't working.
**		Working is training, upgrading, researching.
**
**	@param unit	Pointer to unit for button.
**	@param button	Pointer to button to check/enable.
**	@return		True if enabled.
*/
global int ButtonCheckNoWork(const Unit* unit,const ButtonAction* button)
{
    return unit->Type->Building
	    && unit->Orders[0].Action != UnitActionTrain
	    && unit->Orders[0].Action != UnitActionUpgradeTo
	    && unit->Orders[0].Action != UnitActionResearch;
}

/**
**	Check for button enabled, if the unit isn't researching.
**
**	@param unit	Pointer to unit for button.
**	@param button	Pointer to button to check/enable.
**	@return		True if enabled.
*/
global int ButtonCheckNoResearch(const Unit* unit,const ButtonAction* button)
{
    return unit->Type->Building
	    && unit->Orders[0].Action != UnitActionUpgradeTo
	    && unit->Orders[0].Action != UnitActionResearch;
}

/**
**	Check for button enabled, if all requirements for an upgrade to unit
**	are meet.
**
**	@param unit	Pointer to unit for button.
**	@param button	Pointer to button to check/enable.
**	@return		True if enabled.
*/
global int ButtonCheckUpgradeTo(const Unit* unit,const ButtonAction* button)
{
    if ( unit->Orders[0].Action != UnitActionStill ) {
	return 0;
    }
    return CheckDependByIdent(unit->Player,button->ValueStr);
}

/**
**	Check if all requirements for an attack are meet.
**
**	@param unit	Pointer to unit for button.
**	@param button	Pointer to button to check/enable.
**	@return		True if enabled.
*/
global int ButtonCheckAttack(const Unit* unit,const ButtonAction* button)
{
    return unit->Type->CanAttack;
}

/**
**	Check if all requirements for upgrade research are meet.
**
**	@param unit	Pointer to unit for button.
**	@param button	Pointer to button to check/enable.
**	@return		True if enabled.
*/
global int ButtonCheckResearch(const Unit* unit,const ButtonAction* button)
{
    if ( !ButtonCheckNoWork( unit, button ) ) {	// don't show any if working
	return 0;
    }

    // check if allowed
    if ( !CheckDependByIdent( ThisPlayer, button->ValueStr ) ) {
	return 0;
    }
    if ( !strncmp( button->ValueStr,"upgrade-", 8 ) &&
		UpgradeIdentAllowed( ThisPlayer,button->ValueStr )!='A' ) {
	return 0;
    }
    return 1;
}

/*----------------------------------------------------------------------------
--      Table
----------------------------------------------------------------------------*/

#ifndef USE_CCL

/**
**	All possible buttons.
**
**	Sorted by races
*/
global ButtonAction AllButtons[] = {
// HUMAN ---------------------------------------------------------------------
// general commands -- almost all units have it ------------------------------
{   1, 0, { "icon-move-peasant" },
    ButtonMove,		0, NULL,
    NULL,		NULL,
    'm', "~!MOVE",
    HUMAN_LAND_FORCES3 "," HUMAN_AIR_FORCES ",unit-attack-peasant"
    ",unit-mage" ",unit-critter" ",human-group"
},
{   2, 0, { "icon-human-shield1" },
    ButtonStop,		0, NULL,
    NULL,		NULL,
    's', "~!STOP",
    HUMAN_LAND_FORCES3 "," HUMAN_AIR_FORCES ",unit-attack-peasant"
    ",unit-mage" ",unit-critter" ",human-group"
},
{   2, 0, { "icon-human-shield2" },
    ButtonStop,		0, NULL,
    ButtonCheckUpgrade,	"upgrade-human-shield1",
    's', "~!STOP",
    HUMAN_LAND_FORCES
},
{   2, 0, { "icon-human-shield3" },
    ButtonStop,		0, NULL,
    ButtonCheckUpgrade,	"upgrade-human-shield2",
    's', "~!STOP",
    HUMAN_LAND_FORCES
},
{   3, 0, { "icon-sword1" },
    ButtonAttack,	0, NULL,
    NULL,		NULL,
    'a', "~!ATTACK",
    HUMAN_LAND_FORCES3 ",unit-gryphon-rider" ",human-group"
},
{   3, 0, { "icon-sword2" },
    ButtonAttack,	0, NULL,
    ButtonCheckUpgrade,	"upgrade-sword1",
    'a', "~!ATTACK",
    HUMAN_LAND_FORCES
},
{   3, 0, { "icon-sword3" },
    ButtonAttack,	0, NULL,
    ButtonCheckUpgrade,	"upgrade-sword2",
    'a', "~!ATTACK",
    HUMAN_LAND_FORCES
},
{   3, 0, { "icon-arrow1" },
    ButtonAttack,	0, NULL,
    NULL,		NULL,
    'a', "~!ATTACK",
    "unit-archer,unit-ranger,unit-alleria"
},
{   3, 0, { "icon-arrow2" },
    ButtonAttack,	0, NULL,
    ButtonCheckUpgrade,	"upgrade-arrow1",
    'a', "~!ATTACK",
    "unit-archer,unit-ranger"
},
{   3, 0, { "icon-arrow3" },
    ButtonAttack,	0, NULL,
    ButtonCheckUpgrade,	"upgrade-arrow2",
    'a', "~!ATTACK",
    "unit-archer,unit-ranger"
},
// NOTE: this isn't compatible dwarves didn't have this button
{   4, 0, { "icon-human-patrol-land" },
    ButtonPatrol,	0, NULL,
    NULL,		NULL,
    'p', "~!PATROL",
    HUMAN_LAND_FORCES "," HUMAN_HEROS1 ",unit-gryphon-rider" ",human-group"
},
// NOTE: this isn't compatible dwarves didn't have this button
{   5, 0, { "icon-human-stand-ground" },
    ButtonStandGround,	0, NULL,
    NULL,		NULL,
    't', "S~!TAND GROUND",
    HUMAN_LAND_FORCES "," HUMAN_HEROS1 ",unit-gryphon-rider" ",human-group"
    ",unit-gnomish-submarine,unit-battleship,unit-elven-destroyer"
},
{   6, 0, { "icon-human-attack-ground" },
    ButtonAttack,	0, NULL,
    NULL,		NULL,
    'g', "ATTACK ~!GROUND",
    "unit-ballista,unit-battleship" ",human-group"
},
{   9, 0, { "icon-human-demolish" },
    ButtonDemolish,	0, NULL,
    NULL,		NULL,
    'd', "~!DEMOLISH",
    HUMAN_LAND_FORCES ",unit-gryphon-rider" ",human-group"
},

// paladin specific actions --------------------------------------------
{   7, 0, { "icon-holy-vision" },
    ButtonSpellCast,	0, "spell-holy-vision",
    ButtonCheckTrue,	NULL,
    'v', "HOLY ~!VISION",
    "unit-paladin"
},
{   8, 0, { "icon-heal" },
    ButtonSpellCast,	0, "spell-healing",
    ButtonCheckUpgrade,	"upgrade-healing",
    'h', "~!HEALING (per 1 HP)",
    "unit-paladin"
},
{   9, 0, { "icon-exorcism" },
    ButtonSpellCast,	0, "spell-exorcism",
    ButtonCheckUpgrade,	"upgrade-exorcism",
    'e', "~!EXORCISM",
    "unit-paladin"
},
// mage specific actions --------------------------------------------
{   3, 0, { "icon-lightning" },
    ButtonAttack,	0, NULL,
    NULL,		NULL,
    'a', "LIGHTNING ~!ATTACK",
    "unit-mage,unit-khadgar"
},
{   4, 0, { "icon-fireball" },
    ButtonSpellCast,	0, "spell-fireball",
    ButtonCheckTrue,	NULL,
    'f', "~!FIREBALL",
    "unit-mage,unit-khadgar"
},
{   5, 0, { "icon-slow" },
    ButtonSpellCast,	0, "spell-slow",
    ButtonCheckUpgrade,	"upgrade-slow",
    'o', "SL~!OW",
    "unit-mage"
},
{   6, 0, { "icon-flame-shield" },
    ButtonSpellCast,	0, "spell-flame-shield",
    ButtonCheckUpgrade,	"upgrade-flame-shield",
    'l', "F~!LAME SHIELD",
    "unit-mage"
},
{   7, 0, { "icon-invisibility" },
    ButtonSpellCast,	0, "spell-invisibility",
    ButtonCheckUpgrade,	"upgrade-invisibility",
    'i', "~!INVISIBILITY",
    "unit-mage"
},
{   8, 0, { "icon-polymorph" },
    ButtonSpellCast,	0, "spell-polymorph",
    ButtonCheckUpgrade,	"upgrade-polymorph",
    'p', "~!POLYMORPH",
    "unit-mage"
},
{   9, 0, { "icon-blizzard" },
    ButtonSpellCast,	0, "spell-blizzard",
    ButtonCheckUpgrade,	"upgrade-blizzard",
    'b', "~!BLIZZARD",
    "unit-mage"
},

// peasant specific actions ---------------------------------------------------
{   4, 0, { "icon-repair" },
    ButtonRepair,	0, NULL,
    NULL,		NULL,
    'r', "~!REPAIR",
    WORKERS_H
},
{   5, 0, { "icon-harvest" },
    ButtonHarvest,	0, NULL,
    NULL,		NULL,
    'h', "~!HARVEST LUMBER/MINE GOLD",
    "unit-peasant"
},
{   6, 0, { "icon-return-goods-peasant" },
    ButtonReturn,	0, NULL,
    NULL,		NULL,
    'g', "RETURN WITH ~!GOODS",
    "unit-peasant-with-gold,unit-peasant-with-wood"
},

// build basic/advanced structs -----------------------------------------------
{   7, 0, { "icon-build-basic" },
    ButtonButton,	1, "1",
    NULL,		NULL,
    'b', "~!BUILD BASIC STRUCTURE",
    WORKERS_H
},
{   8, 0, { "icon-build-advanced" },
    ButtonButton,	2, "2",
    ButtonCheckUnit,	"unit-elven-lumber-mill",
    'v', "BUILD AD~!VANCED STRUCTURE",
    WORKERS_H
},
#ifdef USE_EXTENSIONS
{   9, 0, { "icon-build-advanced" },
    ButtonButton,	3, "3",
    ButtonCheckUnits,	"unit-castle,unit-mage-tower,unit-church",
    'e', "BUILD SP~!ECIAL STRUCTURE",
    WORKERS_H
},
#endif
// simple buildings human -----------------------------------------------------
{   1, 1, { "icon-farm" },
    ButtonBuild,	0, "unit-farm",
    NULL,		NULL,
    'f', "BUILD ~!FARM",
    WORKERS_H
},
{   2, 1, { "icon-human-barracks" },
    ButtonBuild,	0, "unit-human-barracks",
    NULL,		NULL,
    'b', "BUILD ~!BARRACKS",
    WORKERS_H
},
{   3, 1, { "icon-town-hall" },
    ButtonBuild,	0, "unit-town-hall",
    NULL,		NULL,
    'h', "BUILD TOWN ~!HALL",
    WORKERS_H
},
{   4, 1, { "icon-elven-lumber-mill" },
    ButtonBuild,	0, "unit-elven-lumber-mill",
    NULL,		NULL,
    'l', "BUILD ELVEN ~!LUMBER MILL",
    WORKERS_H
},
{   5, 1, { "icon-human-blacksmith" },
    ButtonBuild,	0, "unit-human-blacksmith",
    NULL,		NULL,
    's', "BUILD BLACK~!SMITH",
    WORKERS_H
},
{   7, 1, { "icon-human-watch-tower" },
    ButtonBuild,	0, "unit-human-watch-tower",
    NULL,		NULL,
    't', "BUILD ~!TOWER",
    WORKERS_H
},
{   8, 1, { "icon-human-wall" },
    ButtonBuild,	0, "unit-human-wall",
    ButtonCheckNetwork,	NULL,
    'w', "BUILD ~!WALL",
    WORKERS_H
},
{   9, 1, { "icon-cancel" },
    ButtonButton,	0, "0",
    NULL,		NULL,
    '\e', "~<ESC~> CANCEL",
    WORKERS_H
},
// human advanced buildings ---------------------------------------------------
{   1, 2, { "icon-human-shipyard" },
    ButtonBuild,	0, "unit-human-shipyard",
    NULL,		NULL,
    's', "BUILD ~!SHIPYARD",
    WORKERS_H
},
{   2, 2, { "icon-human-foundry" },
    ButtonBuild,	0, "unit-human-foundry",
    NULL,		NULL,
    'f', "BUILD ~!FOUNDRY",
    WORKERS_H
},
{   3, 2, { "icon-human-refinery" },
    ButtonBuild,	0, "unit-human-refinery",
    NULL,		NULL,
    'r', "BUILD ~!REFINERY",
    WORKERS_H
},
{   4, 2, { "icon-gnomish-inventor" },
    ButtonBuild,	0, "unit-gnomish-inventor",
    NULL,		NULL,
    'i', "BUILD GNOMISH ~!INVENTOR",
    WORKERS_H
},
{   5, 2, { "icon-stables" },
    ButtonBuild,	0, "unit-stables",
    NULL,		NULL,
    'a', "BUILD ST~!ABLES",
    WORKERS_H
},
{   6, 2, { "icon-mage-tower" },
    ButtonBuild,	0, "unit-mage-tower",
    NULL,		NULL,
    'm', "BUILD ~!MAGE TOWER",
    WORKERS_H
},
{   7, 2, { "icon-church" },
    ButtonBuild,	0, "unit-church",
    NULL,		NULL,
    'c', "BUILD ~!CHURCH",
    WORKERS_H
},
{   8, 2, { "icon-gryphon-aviary" },
    ButtonBuild,	0, "unit-gryphon-aviary",
    NULL,		NULL,
    'g', "BUILD ~!GRYPHON AVIARY",
    WORKERS_H
},
{   9, 2, { "icon-cancel" },
    ButtonButton,	0, "0",
    NULL,		NULL,
    '\e', "~<ESC~> CANCEL",
    WORKERS_H
},
// human special buildings ----------------------------------------------------
#ifdef USE_EXTENSIONS
{   1, 3, { "icon-dark-portal" },
    ButtonBuild,	0, "unit-dark-portal",
    NULL,		NULL,
    'p', "BUILD DARK ~!PORTAL",
    WORKERS_H
},
{   2, 3, { "icon-runestone" },
    ButtonBuild,	0, "unit-runestone",
    NULL,		NULL,
    'r', "BUILD ~!RUNESTONE",
    WORKERS_H
},
{   9, 3, { "icon-cancel" },
    ButtonButton,	0, "0",
    NULL,		NULL,
    '\e', "~<ESC~> CANCEL",
    WORKERS_H
},
#endif
// buildings commands ---------------------------------------------------------
#ifdef USE_EXTENSIONS
{   1, 0, { "icon-critter" },
    ButtonTrain,	0, "unit-critter",
    NULL,		NULL,
    'c', "TRAIN ~!CRITTER",
    "unit-farm"
},
#endif
{   1, 0, { "icon-peasant" },
    ButtonTrain,	0, "unit-peasant",
    ButtonCheckNoResearch,	NULL,
    'p', "TRAIN ~!PEASANT",
    HALLS_H
},
// town hall upgrades
{   2, 0, { "icon-keep" },
    ButtonUpgradeTo,	0, "unit-keep",
    ButtonCheckUpgradeTo,	NULL,
    'k', "UPGRADE TO ~!KEEP",
    "unit-town-hall"
},
{   2, 0, { "icon-castle-upgrade" },
    ButtonUpgradeTo,	0, "unit-castle",
    ButtonCheckUpgradeTo,	NULL,
    'c', "UPGRADE TO ~!CASTLE",
    "unit-keep"
},
#ifdef USE_EXTENSIONS
{   5, 0, { "icon-harvest" },
    ButtonHarvest,	0, NULL,
    ButtonCheckNoWork,	NULL,
    'h', "SET ~!HARVEST LUMBER/MINE GOLD",
    HALLS_H
},
{   7, 0, { "icon-move-peasant" },
    ButtonMove,		0, NULL,
    ButtonCheckNoWork,	NULL,
    'm', "SET ~!MOVE",
    HALLS_H ",unit-human-barracks,unit-mage-tower,unit-gryphon-aviary"
    ",unit-gnomish-inventor"
},
{   8, 0, { "icon-human-shield1" },
    ButtonStop,		0, NULL,
    ButtonCheckNoWork,	NULL,
    's', "SET ~!STOP",
    HALLS_H ",unit-human-barracks,unit-mage-tower,unit-gryphon-aviary"
    ",unit-gnomish-inventor"
},
{   9, 0, { "icon-sword1" },
    ButtonAttack,	0, NULL,
    ButtonCheckNoWork,	NULL,
    'a', "SET ~!ATTACK",
    HALLS_H ",unit-human-barracks,unit-mage-tower,unit-gryphon-aviary"
    ",unit-gnomish-inventor"
},
#endif
{   1, 0, { "icon-footman" },
    ButtonTrain,	0, "unit-footman",
    NULL,		NULL,
    'f', "TRAIN ~!FOOTMAN",
    "unit-human-barracks"
},
{   2, 0, { "icon-archer" },
    ButtonTrain,	0, "unit-archer",
    NULL,		NULL,
    'a', "TRAIN ~!ARCHER",
    "unit-human-barracks"
},
{   2, 0, { "icon-ranger" },
    ButtonTrain,	0, "unit-ranger",
    NULL,		NULL,
    'r', "TRAIN ~!RANGER",
    "unit-human-barracks"
},
{   3, 0, { "icon-ballista" },
    ButtonTrain,	0, "unit-ballista",
    NULL,		NULL,
    'b', "BUILD ~!BALLISTA",
    "unit-human-barracks"
},
{   4, 0, { "icon-knight" },
    ButtonTrain,	0, "unit-knight",
    NULL,		NULL,
    'k', "TRAIN ~!KNIGHT",
    "unit-human-barracks"
},
{   4, 0, { "icon-paladin" },
    ButtonTrain,	0, "unit-paladin",
    NULL,		NULL,
    'p', "TRAIN ~!PALADIN",
    "unit-human-barracks"
},
{   1, 0, { "icon-gnomish-flying-machine" },
    ButtonTrain,	0, "unit-gnomish-flying-machine",
    NULL,		NULL,
    'f', "BUILD GNOMISH ~!FLYING MACHINE",
    "unit-gnomish-inventor"
},
{   2, 0, { "icon-dwarves" },
    ButtonTrain,	0, "unit-dwarves",
    NULL,		NULL,
    'd', "TRAIN ~!DWARVEN DEMOLITION SQUAD",
    "unit-gnomish-inventor"
},
{   1, 0, { "icon-mage" },
    ButtonTrain,	0, "unit-mage",
    NULL,		NULL,
    't', "~!TRAIN MAGE",
    "unit-mage-tower"
},
{   1, 0, { "icon-gryphon-rider" },
    ButtonTrain,	0, "unit-gryphon-rider",
    NULL,		NULL,
    't', "~!TRAIN GRYPHON RIDER",
    "unit-gryphon-aviary"
},
{   1, 0, { "icon-human-oil-tanker" },
    ButtonTrain,	0, "unit-human-oil-tanker",
    NULL,		NULL,
    'o', "BUILD ~!OIL TANKER",
    "unit-human-shipyard"
},
{   2, 0, { "icon-elven-destroyer" },
    ButtonTrain,	0, "unit-elven-destroyer",
    NULL,		NULL,
    'd', "BUILD ~!DESTROYER",
    "unit-human-shipyard"
},
{   3, 0, { "icon-human-transport" },
    ButtonTrain,	0, "unit-human-transport",
    NULL,		NULL,
    't', "BUILD ~!TRANSPORT",
    "unit-human-shipyard"
},
{   4, 0, { "icon-gnomish-submarine" },
    ButtonTrain,	0, "unit-gnomish-submarine",
    NULL,		NULL,
    's', "BUILD GNOMISH ~!SUBMARINE",
    "unit-human-shipyard"
},
{   5, 0, { "icon-battleship" },
    ButtonTrain,	0, "unit-battleship",
    NULL,		NULL,
    'b', "BUILD ~!BATTLESHIP",
    "unit-human-shipyard"
},
{   1, 0, { "icon-human-guard-tower" },
    ButtonUpgradeTo,	0, "unit-human-guard-tower",
    NULL,		NULL,
    'g', "UPGRADE TO ~!GUARD TOWER",
    "unit-human-watch-tower"
},
{   2, 0, { "icon-human-cannon-tower" },
    ButtonUpgradeTo,	0, "unit-human-cannon-tower",
    NULL,		NULL,
    'c', "UPGRADE TO ~!CANNON TOWER",
    "unit-human-watch-tower"
},

// Ships --------------------------------------------------------------------
{   1, 0, { "icon-human-ship-move" },
    ButtonMove,		0, NULL,
    NULL,		NULL,
    'm', "~!MOVE",
    "unit-human-oil-tanker,unit-human-oil-tanker-full,unit-gnomish-submarine"
    ",unit-battleship,unit-elven-destroyer,unit-human-transport"
},
{   2, 0, { "icon-human-ship-armor1" },
    ButtonStop,		0, NULL,
    NULL,		NULL,
    's', "~!STOP",
    "unit-human-oil-tanker,unit-human-oil-tanker-full,unit-gnomish-submarine"
    ",unit-battleship,unit-elven-destroyer,unit-human-transport"
},
{   3, 0, { "icon-human-unload" },
    ButtonUnload,	0, NULL,
    NULL,		NULL,
    'u', "~!UNLOAD",
    "unit-human-transport"
},
{   4, 0, { "icon-human-oil-platform" },
    ButtonBuild,	0, "unit-human-oil-platform",
    NULL,		NULL,
    'b', "~!BUILD OIL PLATFORM",
    "unit-human-oil-tanker"
},
{   5, 0, { "icon-human-ship-haul-oil" },
    ButtonHarvest,	0, NULL,
    NULL,		NULL,
    'h', "~!HAUL OIL",
    "unit-human-oil-tanker"
},
{   6, 0, { "icon-human-ship-return-oil" },
    ButtonReturn,	0, NULL,
    NULL,		NULL,
    'g', "RETURN WITH ~!GOODS",
    "unit-human-oil-tanker-full"
},
{   3, 0, { "icon-human-ship-cannon1" },
    ButtonAttack,	0, NULL,
    NULL,		NULL,
    'a', "~!ATTACK",
    "unit-gnomish-submarine,unit-battleship,unit-elven-destroyer"
},
{   3, 0, { "icon-human-ship-cannon2" },
    ButtonAttack,	0, NULL,
    ButtonCheckUpgrade,	"upgrade-human-ship-cannon1",
    'a', "~!ATTACK",
    "unit-gnomish-submarine,unit-battleship,unit-elven-destroyer"
},
{   3, 0, { "icon-human-ship-cannon3" },
    ButtonAttack,	0, NULL,
    ButtonCheckUpgrade,	"upgrade-human-ship-cannon2",
    'a', "~!ATTACK",
    "unit-gnomish-submarine,unit-battleship,unit-elven-destroyer"
},
{   4, 0, { "icon-human-patrol-naval" },
    ButtonPatrol,	0, NULL,
    NULL,		NULL,
    'p', "~!PATROL",
    "unit-gnomish-submarine,unit-battleship,unit-elven-destroyer"
},
#ifdef USE_EXTENSIONS
{   7, 0, { "icon-human-ship-move" },
    ButtonMove,		0, NULL,
    ButtonCheckNoWork,	NULL,
    'm', "SET ~!MOVE",
    "unit-human-shipyard"
},
{   8, 0, { "icon-human-ship-armor1" },
    ButtonStop,		0, NULL,
    ButtonCheckNoWork,	NULL,
    's', "SET ~!STOP",
    "unit-human-shipyard"
},
{   9, 0, { "icon-human-ship-cannon1" },
    ButtonAttack,	0, NULL,
    ButtonCheckNoWork,	NULL,
    'a', "SET ~!ATTACK",
    "unit-human-shipyard"
},
#endif

// upgrades
{   1, 0, { "icon-sword2" },
    ButtonResearch,	0, "upgrade-sword1",
    ButtonCheckResearch,	NULL,
    'w', "UPGRADE S~!WORDS (Damage +2)",
    "unit-human-blacksmith"
},
{   1, 0, { "icon-sword3" },
    ButtonResearch,	0, "upgrade-sword2",
    ButtonCheckResearch,	NULL,
    'w', "UPGRADE S~!WORDS (Damage +2)",
    "unit-human-blacksmith"
},
{   2, 0, { "icon-human-shield2" },
    ButtonResearch,	0, "upgrade-human-shield1",
    ButtonCheckResearch,	NULL,
    's', "UPGRADE ~!SHIELDS (Armor +2)",
    "unit-human-blacksmith"
},
{   2, 0, { "icon-human-shield3" },
    ButtonResearch,	0, "upgrade-human-shield2",
    ButtonCheckResearch,	NULL,
    's', "UPGRADE ~!SHIELDS (Armor +2)",
    "unit-human-blacksmith"
},
{   3, 0, { "icon-ballista1" },
    ButtonResearch,	0, "upgrade-ballista1",
    ButtonCheckResearch,	NULL,
    'b', "UPGRADE ~!BALLISTA (Damage +15)",
    "unit-human-blacksmith"
},
{   3, 0, { "icon-ballista2" },
    ButtonResearch,	0, "upgrade-ballista2",
    ButtonCheckResearch,	NULL,
    'b', "UPGRADE ~!BALLISTA (Damage +15)",
    "unit-human-blacksmith"
},
{   1, 0, { "icon-arrow2" },
    ButtonResearch,	0, "upgrade-arrow1",
    ButtonCheckResearch,	NULL,
    'u', "~!UPGRADE ARROWS (Damage +1)",
    "unit-elven-lumber-mill"
},
{   1, 0, { "icon-arrow3" },
    ButtonResearch,	0, "upgrade-arrow2",
    ButtonCheckResearch,	NULL,
    'u', "~!UPGRADE ARROWS (Damage +1)",
    "unit-elven-lumber-mill"
},
{   4, 0, { "icon-ranger" },
    ButtonResearch,	0, "upgrade-ranger",
    ButtonCheckResearch,	NULL,
    'r', "ELVEN ~!RANGER TRAINING",
    "unit-elven-lumber-mill"
},
{   4, 0, { "icon-ranger-scouting" },
    ButtonResearch,	0, "upgrade-ranger-scouting",
    ButtonCheckResearch,	NULL,
    's', "RANGER ~!SCOUTING (Sight:9)",
    "unit-elven-lumber-mill"
},
{   5, 0, { "icon-longbow" },
    ButtonResearch,	0, "upgrade-longbow",
    ButtonCheckResearch,	NULL,
    'l', "RESEARCH ~!LONGBOW (Range +1)",
    "unit-elven-lumber-mill"
},
{   6, 0, { "icon-ranger-marksmanship" },
    ButtonResearch,	0, "upgrade-ranger-marksmanship",
    ButtonCheckResearch,	NULL,
    'm', "RANGER ~!MARKSMANSHIP (Damage +3)",
    "unit-elven-lumber-mill"
},
{   1, 0, { "icon-paladin" },
    ButtonResearch,	0, "upgrade-paladin",
    ButtonCheckResearch,	NULL,
    'p', "UPGRADES KNIGHTS TO ~!PALADINS",
    "unit-church"
},
{   2, 0, { "icon-heal" },
    ButtonResearch,	0, "upgrade-healing",
    ButtonCheckResearch,	NULL,
    'h', "RESEARCH ~!HEALING",
    "unit-church"
},
{   3, 0, { "icon-exorcism" },
    ButtonResearch,	0, "upgrade-exorcism",
    ButtonCheckResearch,	NULL,
    'e', "RESEARCH ~!EXORCISM",
    "unit-church"
},
{   2, 0, { "icon-slow" },
    ButtonResearch,	0, "upgrade-slow",
    ButtonCheckResearch,	NULL,
    'o', "RESEARCH SL~!OW",
    "unit-mage-tower"
},
{   3, 0, { "icon-flame-shield" },
    ButtonResearch,	0, "upgrade-flame-shield",
    ButtonCheckResearch,	NULL,
    'l', "RESEARCH F~!LAME SHIELD",
    "unit-mage-tower"
},
{   4, 0, { "icon-invisibility" },
    ButtonResearch,	0, "upgrade-invisibility",
    ButtonCheckResearch,	NULL,
    'i', "RESEARCH ~!INVISIBILITY",
    "unit-mage-tower"
},
{   5, 0, { "icon-polymorph" },
    ButtonResearch,	0, "upgrade-polymorph",
    ButtonCheckResearch,	NULL,
    'p', "RESEARCH ~!POLYMORPH",
    "unit-mage-tower"
},
{   6, 0, { "icon-blizzard" },
    ButtonResearch,	0, "upgrade-blizzard",
    ButtonCheckResearch,	NULL,
    'b', "RESEARCH ~!BLIZZARD",
    "unit-mage-tower"
},
{   1, 0, { "icon-human-ship-cannon2" },
    ButtonResearch,	0, "upgrade-human-ship-cannon1",
    ButtonCheckResearch,	NULL,
    'c', "UPGRADE ~!CANNONS (Damage +5)",
    "unit-human-foundry"
},
{   1, 0, { "icon-human-ship-cannon3" },
    ButtonResearch,	0, "upgrade-human-ship-cannon2",
    ButtonCheckResearch,	NULL,
    'c', "UPGRADE ~!CANNONS (Damage +5)",
    "unit-human-foundry"
},
{   2, 0, { "icon-human-ship-armor2" },
    ButtonResearch,	0, "upgrade-human-ship-armor1",
    ButtonCheckResearch,	NULL,
    'a', "UPGRADE SHIP ~!ARMOR (Armor +5)",
    "unit-human-foundry"
},
{   2, 0, { "icon-human-ship-armor3" },
    ButtonResearch,	0, "upgrade-human-ship-armor2",
    ButtonCheckResearch,	NULL,
    'a', "UPGRADE SHIP ~!ARMOR (Armor +5)",
    "unit-human-foundry"
},

// ============================================================================
// ORCS -----------------------------------------------------------------------
// general commands -- almost all units have it -------------------------------
{   1, 0, { "icon-move-peon" },
    ButtonMove,		0, NULL,
    NULL,		NULL,
    'm', "~!MOVE",
    ORC_LAND_FORCES3 "," ORC_AIR_FORCES ",unit-attack-peon"
    ",unit-death-knight" ",unit-deathwing" ",orc-group"
},
{   2, 0, { "icon-orc-shield1" },
    ButtonStop,		0, NULL,
    NULL,		NULL,
    's', "~!STOP",
    ORC_LAND_FORCES3 "," ORC_AIR_FORCES ",unit-attack-peon"
    ",unit-death-knight" ",unit-deathwing" ",orc-group"
},
{   2, 0, { "icon-orc-shield2" },
    ButtonStop,		0, NULL,
    ButtonCheckUpgrade,	"upgrade-orc-shield1",
    's', "~!STOP",
    ORC_LAND_FORCES
},
{   2, 0, { "icon-orc-shield3" },
    ButtonStop,		0, NULL,
    ButtonCheckUpgrade,	"upgrade-orc-shield2",
    's', "~!STOP",
    ORC_LAND_FORCES
},
{   3, 0, { "icon-battle-axe1" },
    ButtonAttack,	0, NULL,
    NULL,		NULL,
    'a', "~!ATTACK",
    ORC_LAND_FORCES3 ",unit-dragon" ",unit-deathwing" ",orc-group"
},
{   3, 0, { "icon-battle-axe2" },
    ButtonAttack,	0, NULL,
    ButtonCheckUpgrade,	"upgrade-battle-axe1",
    'a', "~!ATTACK",
    ORC_LAND_FORCES
},
{   3, 0, { "icon-battle-axe3" },
    ButtonAttack,	0, NULL,
    ButtonCheckUpgrade,	"upgrade-battle-axe2",
    'a', "~!ATTACK",
    ORC_LAND_FORCES
},
{   3, 0, { "icon-throwing-axe1" },
    ButtonAttack,	0, NULL,
    NULL,		NULL,
    'a', "~!ATTACK",
    "unit-axethrower,unit-berserker,unit-zuljin"
},
{   3, 0, { "icon-throwing-axe2" },
    ButtonAttack,	0, NULL,
    ButtonCheckUpgrade,	"upgrade-throwing-axe1",
    'a', "~!ATTACK",
    "unit-axethrower,unit-berserker"
},
{   3, 0, { "icon-throwing-axe3" },
    ButtonAttack,	0, NULL,
    ButtonCheckUpgrade,	"upgrade-throwing-axe2",
    'a', "~!ATTACK",
    "unit-axethrower,unit-berserker"
},
{   3, 0, { "icon-catapult1" },
    ButtonAttack,	0, NULL,
    ButtonCheckUpgrade,	"upgrade-catapult1",
    'a', "~!ATTACK",
    "unit-catapult"
},
{   3, 0, { "icon-catapult2" },
    ButtonAttack,	0, NULL,
    ButtonCheckUpgrade,	"upgrade-catapult2",
    'a', "~!ATTACK",
    "unit-catapult"
},

// NOTE: this isn't compatible goblin-sapper didn't have this button
{   4, 0, { "icon-orc-patrol-land" },
    ButtonPatrol,	0, NULL,
    NULL,		NULL,
    'p', "~!PATROL",
    ORC_LAND_FORCES "," ORC_HEROS1 ",unit-skeleton"
    ",unit-dragon" ",unit-deathwing" ",orc-group"
},
// NOTE: this isn't compatible goblin-sapper didn't have this button
{   5, 0, { "icon-orc-stand-ground" },
    ButtonStandGround,	0, NULL,
    NULL,		NULL,
    't', "S~!TAND GROUND",
    ORC_LAND_FORCES  "," ORC_HEROS1 ",unit-dragon" ",unit-deathwing"
    ",unit-giant-turtle,unit-ogre-juggernaught,unit-troll-destroyer"
    ",orc-group"
},
{   6, 0, { "icon-orc-attack-ground" },
    ButtonAttack,	0, NULL,
    NULL,		NULL,
    'g', "ATTACK ~!GROUND",
    "unit-catapult,unit-ogre-juggernaught" ",orc-group"
},
{   9, 0, { "icon-orc-demolish" },
    ButtonDemolish,	0, NULL,
    NULL,		NULL,
    'd', "~!DEMOLISH",
    ORC_LAND_FORCES3 "," ORC_AIR_FORCES ",orc-group"
},

// ogre-mage specific actions --------------------------------------------
{   7, 0, { "icon-eye-of-kilrogg" },
    ButtonSpellCast,	0, "spell-eye-of-kilrogg",
    ButtonCheckUpgrade,	"upgrade-eye-of-kilrogg",
    'k', "EYE OF ~!KILROGG",
    "unit-ogre-mage"
},
{   8, 0, { "icon-bloodlust" },
    ButtonSpellCast,	0, "spell-bloodlust",
    ButtonCheckUpgrade,	"upgrade-bloodlust",
    'b', "~!BLOODLUST",
    "unit-ogre-mage"
},
{   9, 0, { "icon-runes" },
    ButtonSpellCast,	0, "spell-runes",
    ButtonCheckUpgrade,	"upgrade-runes",
    'r', "~!RUNES",
    "unit-ogre-mage"
},
// cho'gall specific actions --- same as ogre mage but it has them always --
{   7, 0, { "icon-eye-of-kilrogg" },
    ButtonSpellCast,	0, "spell-eye-of-kilrogg",
    ButtonCheckTrue,	NULL,
    'k', "EYE OF ~!KILROGG",
    "unit-cho-gall"
},
{   8, 0, { "icon-bloodlust" },
    ButtonSpellCast,	0, "spell-bloodlust",
    ButtonCheckTrue,	NULL,
    'b', "~!BLOODLUST",
    "unit-cho-gall"
},
{   9, 0, { "icon-runes" },
    ButtonSpellCast,	0, "spell-runes",
    ButtonCheckTrue,	NULL,
    'r', "~!RUNES",
    "unit-cho-gall"
},
// death-knight specific actions --------------------------------------------
{   3, 0, { "icon-touch-of-darkness" },
    ButtonAttack,	0, NULL,
    NULL,		NULL,
    'a', "TOUCH OF D~!ARKNESS",
    "unit-death-knight" ",unit-gul-dan" ",unit-teron-gorefiend"
},
{   4, 0, { "icon-death-coil" },
    ButtonSpellCast,	0, "spell-death-coil",
    ButtonCheckUpgrade,	"upgrade-death-coil",
    'c', "DEATH ~!COIL",
    "unit-death-knight" ",unit-gul-dan" ",unit-teron-gorefiend"
},
{   5, 0, { "icon-haste" },
    ButtonSpellCast,	0, "spell-haste",
    ButtonCheckUpgrade,	"upgrade-haste",
    'h', "~!HASTE",
    "unit-death-knight"
},
{   6, 0, { "icon-raise-dead" },
    ButtonSpellCast,	0, "spell-raise-dead",
    ButtonCheckUpgrade,	"upgrade-raise-dead",
    'r', "~!RAISE DEAD",
    "unit-death-knight"
},
{   7, 0, { "icon-whirlwind" },
    ButtonSpellCast,	0, "spell-whirlwind",
    ButtonCheckUpgrade,	"upgrade-whirlwind",
    'w', "~!WHIRLWIND",
    "unit-death-knight"
},
{   8, 0, { "icon-unholy-armor" },
    ButtonSpellCast,	0, "spell-unholy-armor",
    ButtonCheckUpgrade,	"upgrade-unholy-armor",
    'u', "~!UNHOLY ARMOR",
    "unit-death-knight"
},
{   9, 0, { "icon-death-and-decay" },
    ButtonSpellCast,	0, "spell-death-and-decay",
    ButtonCheckUpgrade,	"upgrade-death-and-decay",
    'd', "~!DEATH AND DECAY",
    "unit-death-knight"
},
// peon specific actions ------------------------------------------------------
{   4, 0, { "icon-repair" },
    ButtonRepair,	0, NULL,
    NULL,		NULL,
    'r', "~!REPAIR",
    WORKERS_O
},
{   5, 0, { "icon-harvest" },
    ButtonHarvest,	0, NULL,
    NULL,		NULL,
    'h', "~!HARVEST LUMBER/MINE GOLD",
    "unit-peon"
},
{   6, 0, { "icon-return-goods-peon" },
    ButtonReturn,	0, NULL,
    NULL,		NULL,
    'g', "RETURN WITH ~!GOODS",
    "unit-peon-with-gold,unit-peon-with-wood"
},
// build basic/advanced structs -----------------------------------------------
{   7, 0, { "icon-build-basic" },
    ButtonButton,	1, "1",
    NULL,		NULL,
    'b', "~!BUILD BASIC STRUCTURE",
    WORKERS_O
},
{   8, 0, { "icon-build-advanced" },
    ButtonButton,	2, "2",
    ButtonCheckUnit,		"unit-troll-lumber-mill",
    'v', "BUILD AD~!VANCED STRUCTURE",
    WORKERS_O
},
#ifdef USE_EXTENSIONS
{   9, 0, { "icon-build-advanced" },
    ButtonButton,	3, "3",
    ButtonCheckUnits,		"unit-fortress,unit-temple-of-the-damned"
			",unit-altar-of-storms",
    'e', "BUILD SP~!ECIAL STRUCTURE",
    WORKERS_O
},
#endif
// simple buildings orc -------------------------------------------------------
{   1, 1, { "icon-pig-farm" },
    ButtonBuild,	0, "unit-pig-farm",
    NULL,		NULL,
    'f', "BUILD PIG ~!FARM",
    WORKERS_O
},
{   2, 1, { "icon-orc-barracks" },
    ButtonBuild,	0, "unit-orc-barracks",
    NULL,		NULL,
    'b', "BUILD ~!BARRACKS",
    WORKERS_O
},
{   3, 1, { "icon-great-hall" },
    ButtonBuild,	0, "unit-great-hall",
    NULL,		NULL,
    'h', "BUILD GREAT ~!HALL",
    WORKERS_O
},
{   4, 1, { "icon-troll-lumber-mill" },
    ButtonBuild,	0, "unit-troll-lumber-mill",
    NULL,		NULL,
    'l', "BUILD TROLL ~!LUMBER MILL",
    WORKERS_O
},
{   5, 1, { "icon-orc-blacksmith" },
    ButtonBuild,	0, "unit-orc-blacksmith",
    NULL,		NULL,
    's', "BUILD BLACK~!SMITH",
    WORKERS_O
},
{   7, 1, { "icon-orc-watch-tower" },
    ButtonBuild,	0, "unit-orc-watch-tower",
    NULL,		NULL,
    't', "BUILD ~!TOWER",
    WORKERS_O
},
{   8, 1, { "icon-orc-wall" },
    ButtonBuild,	0, "unit-orc-wall",
    ButtonCheckNetwork,	NULL,
    'w', "BUILD ~!WALL",
    WORKERS_O
},
{   9, 1, { "icon-cancel" },
    ButtonButton,	0, "0",
    NULL,		NULL,
    '\e', "~<ESC~> CANCEL",
    WORKERS_O
},
// orc advanced buildings -----------------------------------------------------
{   1, 2, { "icon-orc-shipyard" },
    ButtonBuild,	0, "unit-orc-shipyard",
    NULL,		NULL,
    's', "BUILD ~!SHIPYARD",
    WORKERS_O
},
{   2, 2, { "icon-orc-foundry" },
    ButtonBuild,	0, "unit-orc-foundry",
    NULL,		NULL,
    'f', "BUILD ~!FOUNDRY",
    WORKERS_O
},
{   3, 2, { "icon-orc-refinery" },
    ButtonBuild,	0, "unit-orc-refinery",
    NULL,		NULL,
    'r', "BUILD ~!REFINERY",
    WORKERS_O
},
{   4, 2, { "icon-goblin-alchemist" },
    ButtonBuild,	0, "unit-goblin-alchemist",
    NULL,		NULL,
    'a', "BUILD GOBLIN ~!ALCHEMIST",
    WORKERS_O
},
{   5, 2, { "icon-ogre-mound" },
    ButtonBuild,	0, "unit-ogre-mound",
    NULL,		NULL,
    'o', "BUILD ~!OGRE MOUND",
    WORKERS_O
},
{   6, 2, { "icon-temple-of-the-damned" },
    ButtonBuild,	0, "unit-temple-of-the-damned",
    NULL,		NULL,
    't', "BUILD ~!TEMPLE OF THE DAMNED",
    WORKERS_O
},
{   7, 2, { "icon-altar-of-storms" },
    ButtonBuild,	0, "unit-altar-of-storms",
    NULL,		NULL,
    'l', "BUILD ~!ALTAR OF STORMS",
    WORKERS_O
},
{   8, 2, { "icon-dragon-roost" },
    ButtonBuild,	0, "unit-dragon-roost",
    NULL,		NULL,
    'd', "BUILD ~!DRAGON ROOST",
    WORKERS_O
},
{   9, 2, { "icon-cancel" },
    ButtonButton,	0, "0",
    NULL,		NULL,
    '\e', "~<ESC~> CANCEL",
    WORKERS_O
},
// orc special buildings ------------------------------------------------------
#ifdef USE_EXTENSIONS
{   1, 3, { "icon-dark-portal" },
    ButtonBuild,	0, "unit-dark-portal",
    NULL,		NULL,
    'p', "BUILD DARK ~!PORTAL",
    WORKERS_O
},
{   2, 3, { "icon-runestone" },
    ButtonBuild,	0, "unit-runestone",
    NULL,		NULL,
    'r', "BUILD ~!RUNESTONE",
    WORKERS_O
},
{   9, 3, { "icon-cancel" },
    ButtonButton,	0, "0",
    NULL,		NULL,
    '\e', "~<ESC~> CANCEL",
    WORKERS_O
},
#endif
// orc buildings commands -----------------------------------------------------
#ifdef USE_EXTENSIONS
{   1, 0, { "icon-critter" },
    ButtonTrain,	0, "unit-critter",
    NULL,		NULL,
    'c', "TRAIN ~!CRITTER",
    "unit-pig-farm"
},
#endif
{   1, 0, { "icon-peon" },
    ButtonTrain,	0, "unit-peon",
    ButtonCheckNoResearch,	NULL,
    'p', "TRAIN ~!PEON",
    HALLS_O
},
{   2, 0, { "icon-stronghold" },
    ButtonUpgradeTo,	0, "unit-stronghold",
    ButtonCheckUpgradeTo,	NULL,
    's', "UPGRADE TO ~!STRONGHOLD",
    "unit-great-hall"
},
{   2, 0, { "icon-fortress-upgrade" },
    ButtonUpgradeTo,	0, "unit-fortress",
    ButtonCheckUpgradeTo,	NULL,
    'f', "UPGRADE TO ~!FORTRESS",
    "unit-stronghold"
},
#ifdef USE_EXTENSIONS
{   5, 0, { "icon-harvest" },
    ButtonHarvest,	0, NULL,
    ButtonCheckNoWork,	NULL,
    'h', "SET ~!HARVEST LUMBER/MINE GOLD",
    HALLS_O
},
{   7, 0, { "icon-move-peon" },
    ButtonMove,		0, NULL,
    ButtonCheckNoWork,	NULL,
    'm', "SET ~!MOVE",
    HALLS_O ",unit-orc-barracks" ",unit-temple-of-the-damned"
    ",unit-dragon-roost" ",unit-goblin-alchemist"
},
{   8, 0, { "icon-orc-shield1" },
    ButtonStop,		0, NULL,
    ButtonCheckNoWork,	NULL,
    's', "SET ~!STOP",
    HALLS_O ",unit-orc-barracks" ",unit-temple-of-the-damned"
    ",unit-dragon-roost" ",unit-goblin-alchemist"
},
{   9, 0, { "icon-battle-axe1" },
    ButtonAttack,	0, NULL,
    ButtonCheckNoWork,	NULL,
    'a', "SET ~!ATTACK",
    HALLS_O ",unit-orc-barracks" ",unit-temple-of-the-damned"
    ",unit-dragon-roost" ",unit-goblin-alchemist"
},
#endif
{   1, 0, { "icon-grunt" },
    ButtonTrain,	0, "unit-grunt",
    NULL,		NULL,
    'g', "TRAIN ~!GRUNT",
    "unit-orc-barracks"
},
{   2, 0, { "icon-axethrower" },
    ButtonTrain,	0, "unit-axethrower",
    NULL,		NULL,
    'a', "TRAIN ~!AXETHROWER",
    "unit-orc-barracks"
},
{   2, 0, { "icon-berserker" },
    ButtonTrain,	0, "unit-berserker",
    NULL,		NULL,
    'b', "TRAIN ~!BERSERKER",
    "unit-orc-barracks"
},
{   3, 0, { "icon-catapult" },
    ButtonTrain,	0, "unit-catapult",
    NULL,		NULL,
    'c', "BUILD ~!CATAPULT",
    "unit-orc-barracks"
},
{   4, 0, { "icon-ogre" },
    ButtonTrain,	0, "unit-ogre",
    NULL,		NULL,
    'o', "TRAIN TWO-HEADED ~!OGRE",
    "unit-orc-barracks"
},
{   4, 0, { "icon-ogre-mage" },
    ButtonTrain,	0, "unit-ogre-mage",
    NULL,		NULL,
    'o', "TRAIN ~!OGRE MAGE",
    "unit-orc-barracks"
},
{   1, 0, { "icon-goblin-zeppelin" },
    ButtonTrain,	0, "unit-goblin-zeppelin",
    NULL,		NULL,
    'z', "BUILD GOBLIN ~!ZEPPELIN",
    "unit-goblin-alchemist"
},
{   2, 0, { "icon-goblin-sappers" },
    ButtonTrain,	0, "unit-goblin-sappers",
    NULL,		NULL,
    's', "TRAIN GOBLIN ~!SAPPERS",
    "unit-goblin-alchemist"
},
{   1, 0, { "icon-death-knight" },
    ButtonTrain,	0, "unit-death-knight",
    NULL,		NULL,
    't', "~!TRAIN DEATH KNIGHT",
    "unit-temple-of-the-damned"
},
{   1, 0, { "icon-dragon" },
    ButtonTrain,	0, "unit-dragon",
    NULL,		NULL,
    'd', "BUILD ~!DRAGON",
    "unit-dragon-roost"
},
{   1, 0, { "icon-orc-oil-tanker" },
    ButtonTrain,	0, "unit-orc-oil-tanker",
    NULL,		NULL,
    'o', "BUILD ~!OIL TANKER",
    "unit-orc-shipyard"
},
{   2, 0, { "icon-troll-destroyer" },
    ButtonTrain,	0, "unit-troll-destroyer",
    NULL,		NULL,
    'd', "BUILD ~!DESTROYER",
    "unit-orc-shipyard"
},
{   3, 0, { "icon-orc-transport" },
    ButtonTrain,	0, "unit-orc-transport",
    NULL,		NULL,
    't', "BUILD ~!TRANSPORT",
    "unit-orc-shipyard"
},
{   4, 0, { "icon-giant-turtle" },
    ButtonTrain,	0, "unit-giant-turtle",
    NULL,		NULL,
    'g', "BUILD ~!GIANT TURTLE",
    "unit-orc-shipyard"
},
{   5, 0, { "icon-ogre-juggernaught" },
    ButtonTrain,	0, "unit-ogre-juggernaught",
    NULL,		NULL,
    'j', "BUILD ~!JUGGERNAUHGT",
    "unit-orc-shipyard"
},
{   1, 0, { "icon-orc-guard-tower" },
    ButtonUpgradeTo,	0, "unit-orc-guard-tower",
    NULL,		NULL,
    'g', "UPGRADE TO ~!GUARD TOWER",
    "unit-orc-watch-tower"
},
{   2, 0, { "icon-orc-cannon-tower" },
    ButtonUpgradeTo,	0, "unit-orc-cannon-tower",
    NULL,		NULL,
    'c', "UPGRADE TO ~!CANNON TOWER",
    "unit-orc-watch-tower"
},

// Ships --------------------------------------------------------------------
{   1, 0, { "icon-orc-ship-move" },
    ButtonMove,		0, NULL,
    NULL,		NULL,
    'm', "~!MOVE",
    "unit-orc-oil-tanker,unit-orc-oil-tanker-full,unit-giant-turtle"
    ",unit-ogre-juggernaught,unit-troll-destroyer,unit-orc-transport"
},
{   2, 0, { "icon-orc-ship-armor1" },
    ButtonStop,		0, NULL,
    NULL,		NULL,
    's', "~!STOP",
    "unit-orc-oil-tanker,unit-orc-oil-tanker-full,unit-giant-turtle"
    ",unit-ogre-juggernaught,unit-troll-destroyer,unit-orc-transport"
},
{   3, 0, { "icon-orc-unload" },
    ButtonUnload,	0, NULL,
    NULL,		NULL,
    'u', "~!UNLOAD",
    "unit-orc-transport"
},
{   4, 0, { "icon-orc-oil-platform" },
    ButtonBuild,	0, "unit-orc-oil-platform",
    NULL,		NULL,
    'b', "~!BUILD OIL PLATFORM",
    "unit-orc-oil-tanker"
},
{   5, 0, { "icon-orc-ship-haul-oil" },
    ButtonHarvest,	0, NULL,
    NULL,		NULL,
    'h', "~!HAUL OIL",
    "unit-orc-oil-tanker"
},
{   6, 0, { "icon-orc-ship-return-oil" },
    ButtonReturn,	0, NULL,
    NULL,		NULL,
    'g', "RETURN WITH ~!GOODS",
    "unit-orc-oil-tanker-full"
},
{   3, 0, { "icon-orc-ship-cannon1" },
    ButtonAttack,	0, NULL,
    NULL,		NULL,
    'a', "~!ATTACK",
    "unit-giant-turtle,unit-ogre-juggernaught,unit-troll-destroyer"
},
{   3, 0, { "icon-orc-ship-cannon2" },
    ButtonAttack,	0, NULL,
    ButtonCheckUpgrade,	"upgrade-orc-ship-cannon1",
    'a', "~!ATTACK",
    "unit-giant-turtle,unit-ogre-juggernaught,unit-troll-destroyer"
},
{   3, 0, { "icon-orc-ship-cannon3" },
    ButtonAttack,	0, NULL,
    ButtonCheckUpgrade,	"upgrade-orc-ship-cannon2",
    'a', "~!ATTACK",
    "unit-giant-turtle,unit-ogre-juggernaught,unit-troll-destroyer"
},
{   4, 0, { "icon-orc-patrol-naval" },
    ButtonPatrol,	0, NULL,
    NULL,		NULL,
    'p', "~!PATROL",
    "unit-giant-turtle,unit-ogre-juggernaught,unit-troll-destroyer"
},
#ifdef USE_EXTENSIONS
{   7, 0, { "icon-orc-ship-move" },
    ButtonMove,		0, NULL,
    ButtonCheckNoWork,	NULL,
    'm', "SET ~!MOVE",
    "unit-orc-shipyard"
},
{   8, 0, { "icon-orc-ship-armor1" },
    ButtonStop,		0, NULL,
    ButtonCheckNoWork,	NULL,
    's', "SET ~!STOP",
    "unit-orc-shipyard"
},
{   9, 0, { "icon-orc-ship-cannon1" },
    ButtonAttack,	0, NULL,
    ButtonCheckNoWork,	NULL,
    'a', "SET ~!ATTACK",
    "unit-orc-shipyard"
},
#endif
// Updates ------------------------------------------------------------------
{   1, 0, { "icon-battle-axe2" },
    ButtonResearch,	0, "upgrade-battle-axe1",
    ButtonCheckResearch,	NULL,
    'w', "UPGRADE ~!WEAPONS (Damage +2)",
    "unit-orc-blacksmith"
},
{   1, 0, { "icon-battle-axe3" },
    ButtonResearch,	0, "upgrade-battle-axe2",
    ButtonCheckResearch,	NULL,
    'w', "UPGRADE ~!WEAPONS (Damage +2)",
    "unit-orc-blacksmith"
},
{   2, 0, { "icon-orc-shield2" },
    ButtonResearch,	0, "upgrade-orc-shield1",
    ButtonCheckResearch,	NULL,
    's', "UPGRADE ~!SHIELDS (Armor +2)",
    "unit-orc-blacksmith"
},
{   2, 0, { "icon-orc-shield3" },
    ButtonResearch,	0, "upgrade-orc-shield2",
    ButtonCheckResearch,	NULL,
    's', "UPGRADE ~!SHIELDS (Armor +2)",
    "unit-orc-blacksmith"
},
{   3, 0, { "icon-catapult1" },
    ButtonResearch,	0, "upgrade-catapult1",
    ButtonCheckResearch,	NULL,
    'c', "UPGRADE ~!CATAPULT (Damage +15)",
    "unit-orc-blacksmith"
},
{   3, 0, { "icon-catapult2" },
    ButtonResearch,	0, "upgrade-catapult2",
    ButtonCheckResearch,	NULL,
    'c', "UPGRADE ~!CATAPULT (Damage +15)",
    "unit-orc-blacksmith"
},
{   1, 0, { "icon-throwing-axe2" },
    ButtonResearch,	0, "upgrade-throwing-axe1",
    ButtonCheckResearch,	NULL,
    'u', "~!UPGRADE THROWING AXE (Damage +1)",
    "unit-troll-lumber-mill"
},
{   1, 0, { "icon-throwing-axe3" },
    ButtonResearch,	0, "upgrade-throwing-axe2",
    ButtonCheckResearch,	NULL,
    'u', "~!UPGRADE THROWING AXE (Damage +1)",
    "unit-troll-lumber-mill"
},
{   4, 0, { "icon-berserker" },
    ButtonResearch,	0, "upgrade-berserker",
    ButtonCheckResearch,	NULL,
    'b', "TROLL ~!BERSERKER TRAINING",
    "unit-troll-lumber-mill"
},
{   4, 0, { "icon-berserker-scouting" },
    ButtonResearch,	0, "upgrade-berserker-scouting",
    ButtonCheckResearch,	NULL,
    's', "BERSERKER ~!SCOUTING (Sight:9)",
    "unit-troll-lumber-mill"
},
{   5, 0, { "icon-light-axes" },
    ButtonResearch,	0, "upgrade-light-axes",
    ButtonCheckResearch,	NULL,
    'a', "RESEARCH LIGHTER ~!AXES (Range +1)",
    "unit-troll-lumber-mill"
},
{   6, 0, { "icon-berserker-regeneration" },
    ButtonResearch,	0, "upgrade-berserker-regeneration",
    ButtonCheckResearch,	NULL,
    'r', "BERSERKER ~!REGENERATION",
    "unit-troll-lumber-mill"
},
{   1, 0, { "icon-ogre-mage" },
    ButtonResearch,	0, "upgrade-ogre-mage",
    ButtonCheckResearch,	NULL,
    'm', "UPGRADES OGRES TO ~!MAGES",
    "unit-altar-of-storms"
},
{   2, 0, { "icon-bloodlust" },
    ButtonResearch,	0, "upgrade-bloodlust",
    ButtonCheckResearch,	NULL,
    'b', "RESEARCH ~!BLOODLUST",
    "unit-altar-of-storms"
},
{   3, 0, { "icon-runes" },
    ButtonResearch,	0, "upgrade-runes",
    ButtonCheckResearch,	NULL,
    'r', "RESEARCH ~!RUNES",
    "unit-altar-of-storms"
},
{   2, 0, { "icon-haste" },
    ButtonResearch,	0, "upgrade-haste",
    ButtonCheckResearch,	NULL,
    'h', "RESEARCH ~!HASTE",
    "unit-temple-of-the-damned"
},
{   3, 0, { "icon-raise-dead" },
    ButtonResearch,	0, "upgrade-raise-dead",
    ButtonCheckResearch,	NULL,
    'r', "RESEARCH ~!RAISE DEAD",
    "unit-temple-of-the-damned"
},
{   4, 0, { "icon-whirlwind" },
    ButtonResearch,	0, "upgrade-whirlwind",
    ButtonCheckResearch,	NULL,
    'w', "RESEARCH ~!WHIRLWIND",
    "unit-temple-of-the-damned"
},
{   5, 0, { "icon-unholy-armor" },
    ButtonResearch,	0, "upgrade-unholy-armor",
    ButtonCheckResearch,	NULL,
    'u', "RESEARCH ~!UNHOLY ARMOR",
    "unit-temple-of-the-damned"
},
{   6, 0, { "icon-death-and-decay" },
    ButtonResearch,	0, "upgrade-death-and-decay",
    ButtonCheckResearch,	NULL,
    'd', "RESEARCH ~!DEATH AND DECAY",
    "unit-temple-of-the-damned"
},
{   1, 0, { "icon-orc-ship-cannon2" },
    ButtonResearch,	0, "upgrade-orc-ship-cannon1",
    ButtonCheckResearch,	NULL,
    'c', "UPGRADE ~!CANNONS (Damage +5)",
    "unit-orc-foundry"
},
{   1, 0, { "icon-orc-ship-cannon3" },
    ButtonResearch,	0, "upgrade-orc-ship-cannon2",
    ButtonCheckResearch,	NULL,
    'c', "UPGRADE ~!CANNONS (Damage +5)",
    "unit-orc-foundry"
},
{   2, 0, { "icon-orc-ship-armor2" },
    ButtonResearch,	0, "upgrade-orc-ship-armor1",
    ButtonCheckResearch,	NULL,
    'a', "UPGRADE SHIP ~!ARMOR (Armor +5)",
    "unit-orc-foundry"
},
{   2, 0, { "icon-orc-ship-armor3" },
    ButtonResearch,	0, "upgrade-orc-ship-armor2",
    ButtonCheckResearch,	NULL,
    'a', "UPGRADE SHIP ~!ARMOR (Armor +5)",
    "unit-orc-foundry"
},

// Neutral --------------------------------------------------------------------
{   1, 0, { "icon-circle-of-power" },
    ButtonBuild,	0, "unit-circle-of-power",
    NULL,		NULL,
    'p', "~!PLACE EXIT/DESTINATION POINT",
    "unit-dark-portal"
},
// general cancel button
{   9, 9, { "icon-cancel" },
    ButtonCancel,	0, NULL,
    NULL,		NULL,
    '\e', "~<ESC~> CANCEL",
    "*"
},
{   9, 0, { "icon-cancel" },
    ButtonCancelUpgrade,0, NULL,
    NULL,		NULL,
    '\e', "~<ESC~> CANCEL UPGRADE",
    "cancel-upgrade"
},
{   9, 0, { "icon-cancel" },
    ButtonCancelTrain,	0, NULL,
    NULL,		NULL,
    '\e', "~<ESC~> CANCEL UNIT TRAINING",
    "*"
},
{   9, 0, { "icon-cancel" },
    ButtonCancelBuild,	0, NULL,
    NULL,		NULL,
    '\e', "~<ESC~> CANCEL CONSTRUCTION",
    "cancel-build"
},

{ },
};

#endif

//@}
