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
/*
**	(c) Copyright 1999-2000 by Lutz Sammer, Vladi Belperchinov-Shabanski
**
**	$Id$
*/

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
    "unit-alleria,unit-kurdan-and-sky'ree,unit-danath,unit-turalyon," \
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
    "unit-dentarg,unit-cho'gall," \
    "unit-zuljin,unit-grom-hellscream,unit-korgath-bladefist"
#define ORC_HEROS \
    ORC_HEROS1 ",unit-gul'dan" ",unit-teron-gorefiend"
#define ORC_LAND_FORCES2 \
    ORC_LAND_FORCES "," WORKERS_O
#define ORC_LAND_FORCES3 \
    ORC_LAND_FORCES2 "," ORC_HEROS ",unit-skeleton"
#define HALLS_O \
    "unit-great-hall,unit-stronghold,unit-fortress"
#define ORC_AIR_FORCES \
    "unit-eye-of-kilrogg,unit-dragon,unit-goblin-zeppelin"

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
**	Check for button enabled, if upgrade is ready.
**
**	@param unit	Pointer to unit for button.
**	@param button	Pointer to button to check/enable.
**	@return		True if enabled.
*/
local int CheckUpgrade(const Unit* unit,const ButtonAction* button)
{
    DebugLevel3("%s\n",button->AllowStr);
    return UpgradeIdentAllowed(unit->Player,button->AllowStr)=='R';
}

/**
**	Check for button enabled, if unit is available.
**
**	@param unit	Pointer to unit for button.
**	@param button	Pointer to button to check/enable.
**	@return		True if enabled.
*/
local int CheckUnit(const Unit* unit,const ButtonAction* button)
{
    DebugLevel3("%s\n",button->AllowStr);
    return HaveUnitTypeByIdent(unit->Player,button->AllowStr);
}

/**
**	Check for button enabled, if all units are available.
**
**	@param unit	Pointer to unit for button.
**	@param button	Pointer to button to check/enable.
**	@return		True if enabled.
*/
local int CheckUnits(const Unit* unit,const ButtonAction* button)
{
    char* buf;
    const char* s;
    Player* player;

    DebugLevel3("%s\n",button->AllowStr);
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
**	Check for button enabled, if an upgrade to unit is available.
**
**	@param unit	Pointer to unit for button.
**	@param button	Pointer to button to check/enable.
**	@return		True if enabled.
*/
local int CheckUpgradeTo(const Unit* unit,const ButtonAction* button)
{
    if ( unit->Command.Action != UnitActionStill ) {
	return 0;
    }
    DebugLevel3("%s\n",button->ValueStr);
    return CheckDependByIdent(unit->Player,button->ValueStr);
}

/**
**	Check for button enabled, always true.
**	This needed to overwrite the internal tests.
**
**	@param unit	Pointer to unit for button.
**	@param button	Pointer to button to check/enable.
**	@return		True if enabled.
*/
local int CheckTrue(const Unit* unit,const ButtonAction* button)
{
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
local int CheckNetwork(const Unit* unit,const ButtonAction* button)
{
    return NetworkFildes!=-1;
}

/**
**	Check for button enabled, if the unit isn't working.
**
**	@param unit	Pointer to unit for button.
**	@param button	Pointer to button to check/enable.
**	@return		True if enabled.
*/
local int CheckNoWork(const Unit* unit,const ButtonAction* button)
{
    return unit->Type->Building
	    && unit->Command.Action != UnitActionTrain
	    && unit->Command.Action != UnitActionUpgradeTo
	    && unit->Command.Action != UnitActionResearch;
}

/**
**	Check for button enabled, if the unit isn't researching.
**
**	@param unit	Pointer to unit for button.
**	@param button	Pointer to button to check/enable.
**	@return		True if enabled.
*/
local int CheckNoResearch(const Unit* unit,const ButtonAction* button)
{
    return unit->Type->Building
	    && unit->Command.Action != UnitActionUpgradeTo
	    && unit->Command.Action != UnitActionResearch;
}

/**
**	Check for button enabled, if the unit is training.
**
**	@param unit	Pointer to unit for button.
**	@param button	Pointer to button to check/enable.
**	@return		True if enabled.
*/
local int CheckTraining(const Unit* unit,const ButtonAction* button)
{
    return unit->Type->Building && unit->Command.Action == UnitActionTrain;
}

/**
**	Check if all requirements for upgrade research are meet.
**
**	@param unit	Pointer to unit for button.
**	@param button	Pointer to button to check/enable.
**	@return		True if enabled.
*/
local int CheckResearch(const Unit* unit,const ButtonAction* button)
{
    if ( !CheckNoWork( unit, button ) ) {	// don't show any if working
	return 0;
    }

    DebugLevel3("%s\n",button->ValueStr);
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

/**
**	All possible buttons.
**
**	Sorted by races
*/
global ButtonAction AllButtons[] = {
// HUMAN ---------------------------------------------------------------------
// general commands -- almost all units have it ------------------------------
{   1, 0, { "icon-move-peasant" },
    B_Move,		0, NULL,
    NULL,		NULL,
    'm', "~!MOVE",
    HUMAN_LAND_FORCES3 "," HUMAN_AIR_FORCES ",unit-attack-peasant"
    ",unit-mage" ",unit-critter" ",human-group"
},
{   2, 0, { "icon-human-shield1" },
    B_Stop,		0, NULL,
    NULL,		NULL,
    's', "~!STOP",
    HUMAN_LAND_FORCES3 "," HUMAN_AIR_FORCES ",unit-attack-peasant"
    ",unit-mage" ",unit-critter" ",human-group"
},
{   2, 0, { "icon-human-shield2" },
    B_Stop,		0, NULL,
    CheckUpgrade,	"upgrade-human-shield1",
    's', "~!STOP",
    HUMAN_LAND_FORCES
},
{   2, 0, { "icon-human-shield3" },
    B_Stop,		0, NULL,
    CheckUpgrade,	"upgrade-human-shield2",
    's', "~!STOP",
    HUMAN_LAND_FORCES
},
{   3, 0, { "icon-sword1" },
    B_Attack,		0, NULL,
    NULL,		NULL,
    'a', "~!ATTACK",
    HUMAN_LAND_FORCES3 ",unit-gryphon-rider" ",human-group"
},
{   3, 0, { "icon-sword2" },
    B_Attack,		0, NULL,
    CheckUpgrade,	"upgrade-sword1",
    'a', "~!ATTACK",
    HUMAN_LAND_FORCES
},
{   3, 0, { "icon-sword3" },
    B_Attack,		0, NULL,
    CheckUpgrade,	"upgrade-sword2",
    'a', "~!ATTACK",
    HUMAN_LAND_FORCES
},
{   3, 0, { "icon-arrow1" },
    B_Attack,		0, NULL,
    NULL,		NULL,
    'a', "~!ATTACK",
    "unit-archer,unit-ranger,unit-alleria"
},
{   3, 0, { "icon-arrow2" },
    B_Attack,		0, NULL,
    CheckUpgrade,	"upgrade-arrow1",
    'a', "~!ATTACK",
    "unit-archer,unit-ranger"
},
{   3, 0, { "icon-arrow3" },
    B_Attack,		0, NULL,
    CheckUpgrade,	"upgrade-arrow2",
    'a', "~!ATTACK",
    "unit-archer,unit-ranger"
},
// NOTE: this isn't compatible dwarves didn't have this button
{   4, 0, { "icon-human-patrol-land" },
    B_Patrol,		0, NULL,
    NULL,		NULL,
    'p', "~!PATROL",
    HUMAN_LAND_FORCES "," HUMAN_HEROS1 ",unit-gryphon-rider" ",human-group"
},
// NOTE: this isn't compatible dwarves didn't have this button
{   5, 0, { "icon-human-stand-ground" },
    B_StandGround,	0, NULL,
    NULL,		NULL,
    't', "S~!TAND GROUND",
    HUMAN_LAND_FORCES "," HUMAN_HEROS1 ",unit-gryphon-rider" ",human-group"
    ",unit-gnomish-submarine,unit-battleship,unit-elven-destroyer"
},
{   6, 0, { "icon-human-attack-ground" },
    B_AttackGround,	0, NULL,
    NULL,		NULL,
    'g', "ATTACK ~!GROUND",
    "unit-ballista,unit-battleship" ",human-group"
},
{   9, 0, { "icon-human-demolish" },
    B_Demolish,	0, NULL,
    NULL,		NULL,
    'd', "~!DEMOLISH",
    HUMAN_LAND_FORCES ",unit-gryphon-rider" ",human-group"
},

// paladin specific actions --------------------------------------------
{   7, 0, { "icon-holy-vision" },
    B_SpellCast,		0, "spell-holy-vision",
    CheckTrue,		NULL,
    'v', "HOLY ~!VISION",
    "unit-paladin"
},
{   8, 0, { "icon-heal" },
    B_SpellCast,		0, "spell-healing",
    CheckUpgrade,		"upgrade-healing",
    'h', "~!HEALING (per 1 HP)",
    "unit-paladin"
},
{   9, 0, { "icon-exorcism" },
    B_SpellCast,		0, "spell-exorcism",
    CheckUpgrade,		"upgrade-exorcism",
    'e', "~!EXORCISM",
    "unit-paladin"
},
// mage specific actions --------------------------------------------
{   3, 0, { "icon-lightning" },
    B_Attack,		0, NULL,
    NULL,		NULL,
    'a', "LIGHTNING ~!ATTACK",
    "unit-mage,unit-khadgar"
},
{   4, 0, { "icon-fireball" },
    B_SpellCast,		0, "spell-fireball",
    CheckTrue,		NULL,
    'f', "~!FIREBALL",
    "unit-mage,unit-khadgar"
},
{   5, 0, { "icon-slow" },
    B_SpellCast,		0, "spell-slow",
    CheckUpgrade,		"upgrade-slow",
    'o', "SL~!OW",
    "unit-mage"
},
{   6, 0, { "icon-flame-shield" },
    B_SpellCast,		0, "spell-flame-shield",
    CheckUpgrade,		"upgrade-flame-shield",
    'l', "F~!LAME SHIELD",
    "unit-mage"
},
{   7, 0, { "icon-invisibility" },
    B_SpellCast,		0, "spell-invisibility",
    CheckUpgrade,		"upgrade-invisibility",
    'i', "~!INVISIBILITY",
    "unit-mage"
},
{   8, 0, { "icon-polymorph" },
    B_SpellCast,		0, "spell-polymorph",
    CheckUpgrade,		"upgrade-polymorph",
    'p', "~!POLYMORPH",
    "unit-mage"
},
{   9, 0, { "icon-blizzard" },
    B_SpellCast,		0, "spell-blizzard",
    CheckUpgrade,		"upgrade-blizzard",
    'b', "~!BLIZZARD",
    "unit-mage"
},

// peasant specific actions ---------------------------------------------------
{   4, 0, { "icon-repair" },
    B_Repair,		0, NULL,
    NULL,		NULL,
    'r', "~!REPAIR",
    WORKERS_H
},
{   5, 0, { "icon-harvest" },
    B_Harvest,		0, NULL,
    NULL,		NULL,
    'h', "~!HARVEST LUMBER/MINE GOLD",
    "unit-peasant"
},
{   6, 0, { "icon-return-goods-peasant" },
    B_Return,		0, NULL,
    NULL,		NULL,
    'g', "RETURN WITH ~!GOODS",
    "unit-peasant-with-gold,unit-peasant-with-wood"
},

// build basic/advanced structs -----------------------------------------------
{   7, 0, { "icon-build-basic" },
    B_Button, 		1, "1",
    NULL,		NULL,
    'b', "~!BUILD BASIC STRUCTURE",
    WORKERS_H
},
{   8, 0, { "icon-build-advanced" },
    B_Button,		2, "2",
    CheckUnit,		"unit-elven-lumber-mill",
    'v', "BUILD AD~!VANCED STRUCTURE",
    WORKERS_H
},
#ifdef USE_EXTENSIONS
{   9, 0, { "icon-build-advanced" },
    B_Button,		3, "3",
    CheckUnits,		"unit-castle,unit-mage-tower,unit-church",
    'e', "BUILD SP~!ECIAL STRUCTURE",
    WORKERS_H
},
#endif
// simple buildings human -----------------------------------------------------
{   1, 1, { "icon-farm" },
    B_Build,		0, "unit-farm",
    NULL,		NULL,
    'f', "BUILD ~!FARM",
    WORKERS_H
},
{   2, 1, { "icon-human-barracks" },
    B_Build,		0, "unit-human-barracks",
    NULL,		NULL,
    'b', "BUILD ~!BARRACKS",
    WORKERS_H
},
{   3, 1, { "icon-town-hall" },
    B_Build,		0, "unit-town-hall",
    NULL,		NULL,
    'h', "BUILD TOWN ~!HALL",
    WORKERS_H
},
{   4, 1, { "icon-elven-lumber-mill" },
    B_Build,		0, "unit-elven-lumber-mill",
    NULL,		NULL,
    'l', "BUILD ELVEN ~!LUMBER MILL",
    WORKERS_H
},
{   5, 1, { "icon-human-blacksmith" },
    B_Build,		0, "unit-human-blacksmith",
    NULL,		NULL,
    's', "BUILD BLACK~!SMITH",
    WORKERS_H
},
{   7, 1, { "icon-human-watch-tower" },
    B_Build,		0, "unit-human-watch-tower",
    NULL,		NULL,
    't', "BUILD ~!TOWER",
    WORKERS_H
},
{   8, 1, { "icon-human-wall" },
    B_Build,		0, "unit-human-wall",
    CheckNetwork,	NULL,
    'w', "BUILD ~!WALL",
    WORKERS_H
},
{   9, 1, { "icon-cancel" },
    B_Button,		0, NULL,
    NULL,		NULL,
    '\e', "~<ESC~> CANCEL",
    WORKERS_H
},
// human advanced buildings ---------------------------------------------------
{   1, 2, { "icon-human-shipyard" },
    B_Build,		0, "unit-human-shipyard",
    NULL,		NULL,
    's', "BUILD ~!SHIPYARD",
    WORKERS_H 
},
{   2, 2, { "icon-human-foundry" },
    B_Build,		0, "unit-human-foundry",
    NULL,		NULL,
    'f', "BUILD ~!FOUNDRY",
    WORKERS_H 
},
{   3, 2, { "icon-human-refinery" },
    B_Build,		0, "unit-human-refinery",
    NULL,		NULL,
    'r', "BUILD ~!REFINERY",
    WORKERS_H 
},
{   4, 2, { "icon-gnomish-inventor" },
    B_Build,		0, "unit-gnomish-inventor",
    NULL,		NULL,
    'i', "BUILD GNOMISH ~!INVENTOR",
    WORKERS_H 
},
{   5, 2, { "icon-stables" },
    B_Build,		0, "unit-stables",
    NULL,		NULL,
    'a', "BUILD ST~!ABLES",
    WORKERS_H 
},
{   6, 2, { "icon-mage-tower" },
    B_Build,		0, "unit-mage-tower",
    NULL,		NULL,
    'm', "BUILD ~!MAGE TOWER",
    WORKERS_H 
},
{   7, 2, { "icon-church" },
    B_Build,		0, "unit-church",
    NULL,		NULL,
    'c', "BUILD ~!CHURCH",
    WORKERS_H 
},
{   8, 2, { "icon-gryphon-aviary" },
    B_Build,		0, "unit-gryphon-aviary",
    NULL,		NULL,
    'g', "BUILD ~!GRYPHON AVIARY",
    WORKERS_H 
},
{   9, 2, { "icon-cancel" },
    B_Button,		0, NULL,
    NULL,		NULL,
    '\e', "~<ESC~> CANCEL",
    WORKERS_H
},
// human special buildings ----------------------------------------------------
#ifdef USE_EXTENSIONS
{   1, 3, { "icon-dark-portal" },
    B_Build,		0, "unit-dark-portal",
    NULL,		NULL,
    'p', "BUILD DARK ~!PORTAL",
    WORKERS_H
},
{   2, 3, { "icon-runestone" },
    B_Build,		0, "unit-runestone",
    NULL,		NULL,
    'r', "BUILD ~!RUNESTONE",
    WORKERS_H
},
{   9, 3, { "icon-cancel" },
    B_Button,		0, NULL,
    NULL,		NULL,
    '\e', "~<ESC~> CANCEL",
    WORKERS_H
},
#endif
// buildings commands ---------------------------------------------------------
#ifdef USE_EXTENSIONS
{   1, 0, { "icon-critter" },
    B_Train,		0, "unit-critter",
    NULL,		NULL,
    'c', "TRAIN ~!CRITTER",
    "unit-farm"
},
#endif
{   1, 0, { "icon-peasant" },
    B_Train,		0, "unit-peasant",
    CheckNoResearch,	NULL,
    'p', "TRAIN ~!PEASANT",
    HALLS_H
},
// town hall upgrades
{   2, 0, { "icon-keep" },
    B_UpgradeTo,	0, "unit-keep",
    CheckUpgradeTo,	NULL,
    'k', "UPGRADE TO ~!KEEP",
    "unit-town-hall"
},
{   2, 0, { "icon-castle-upgrade" },
    B_UpgradeTo,	0, "unit-castle",
    CheckUpgradeTo,	NULL,
    'c', "UPGRADE TO ~!CASTLE",
    "unit-keep"
},
#ifdef USE_EXTENSIONS
{   5, 0, { "icon-harvest" },
    B_Harvest,		0, NULL,
    CheckNoWork,	NULL,
    'h', "SET ~!HARVEST LUMBER/MINE GOLD",
    HALLS_H
},
{   7, 0, { "icon-move-peasant" },
    B_Move,		0, NULL,
    CheckNoWork,	NULL,
    'm', "SET ~!MOVE",
    HALLS_H ",unit-human-barracks,unit-mage-tower,unit-gryphon-aviary"
    ",unit-gnomish-inventor"
},
{   8, 0, { "icon-human-shield1" },
    B_Stop,		0, NULL,
    CheckNoWork,	NULL,
    's', "SET ~!STOP",
    HALLS_H ",unit-human-barracks,unit-mage-tower,unit-gryphon-aviary"
    ",unit-gnomish-inventor"
},
{   9, 0, { "icon-sword1" },
    B_Attack,		0, NULL,
    CheckNoWork,	NULL,
    'a', "SET ~!ATTACK",
    HALLS_H ",unit-human-barracks,unit-mage-tower,unit-gryphon-aviary"
    ",unit-gnomish-inventor"
},
#endif
{   1, 0, { "icon-footman" },
    B_Train,		0, "unit-footman",
    NULL,		NULL,
    'f', "TRAIN ~!FOOTMAN",
    "unit-human-barracks"
},
{   2, 0, { "icon-archer" },
    B_Train,		0, "unit-archer",
    NULL,		NULL,
    'a', "TRAIN ~!ARCHER",
    "unit-human-barracks"
},
{   2, 0, { "icon-ranger" },
    B_Train,		0, "unit-ranger",
    NULL,		NULL,
    'r', "TRAIN ~!RANGER",
    "unit-human-barracks"
},
{   3, 0, { "icon-ballista" },
    B_Train,		0, "unit-ballista",
    NULL,		NULL,
    'b', "BUILD ~!BALLISTA",
    "unit-human-barracks"
},
{   4, 0, { "icon-knight" },
    B_Train,		0, "unit-knight",
    NULL,		NULL,
    'k', "TRAIN ~!KNIGHT",
    "unit-human-barracks"
},
{   4, 0, { "icon-paladin" },
    B_Train,		0, "unit-paladin",
    NULL,		NULL,
    'p', "TRAIN ~!PALADIN",
    "unit-human-barracks"
},
{   1, 0, { "icon-gnomish-flying-machine" },
    B_Train,		0, "unit-gnomish-flying-machine",
    NULL,		NULL,
    'f', "BUILD GNOMISH ~!FLYING MACHINE",
    "unit-gnomish-inventor"
},
{   2, 0, { "icon-dwarves" },
    B_Train,		0, "unit-dwarves",
    NULL,		NULL,
    'd', "TRAIN ~!DWARVEN DEMOLITION SQUAD",
    "unit-gnomish-inventor"
},
{   1, 0, { "icon-mage" },
    B_Train,		0, "unit-mage",
    NULL,		NULL,
    't', "~!TRAIN MAGE",
    "unit-mage-tower"
},
{   1, 0, { "icon-gryphon-rider" },
    B_Train,		0, "unit-gryphon-rider",
    NULL,		NULL,
    't', "~!TRAIN GRYPHON RIDER",
    "unit-gryphon-aviary"
},
{   1, 0, { "icon-human-oil-tanker" },
    B_Train,		0, "unit-human-oil-tanker",
    NULL,		NULL,
    'o', "BUILD ~!OIL TANKER",
    "unit-human-shipyard"
},
{   2, 0, { "icon-elven-destroyer" },
    B_Train,		0, "unit-elven-destroyer",
    NULL,		NULL,
    'd', "BUILD ~!DESTROYER",
    "unit-human-shipyard"
},
{   3, 0, { "icon-human-transport" },
    B_Train,		0, "unit-human-transport",
    NULL,		NULL,
    't', "BUILD ~!TRANSPORT",
    "unit-human-shipyard"
},
{   4, 0, { "icon-gnomish-submarine" },
    B_Train,		0, "unit-gnomish-submarine",
    NULL,		NULL,
    's', "BUILD GNOMISH ~!SUBMARINE",
    "unit-human-shipyard"
},
{   5, 0, { "icon-battleship" },
    B_Train,		0, "unit-battleship",
    NULL,		NULL,
    'b', "BUILD ~!BATTLESHIP",
    "unit-human-shipyard"
},
{   1, 0, { "icon-human-guard-tower" },
    B_UpgradeTo,	0, "unit-human-guard-tower",
    NULL,		NULL,
    'g', "UPGRADE TO ~!GUARD TOWER",
    "unit-human-watch-tower"
},
{   2, 0, { "icon-human-cannon-tower" },
    B_UpgradeTo,	0, "unit-human-cannon-tower",
    NULL,		NULL,
    'c', "UPGRADE TO ~!CANNON TOWER",
    "unit-human-watch-tower"
},

// Ships --------------------------------------------------------------------
{   1, 0, { "icon-human-ship-move" },
    B_Move,		0, NULL,
    NULL,		NULL,
    'm', "~!MOVE",
    "unit-human-oil-tanker,unit-human-oil-tanker-full,unit-gnomish-submarine"
    ",unit-battleship,unit-elven-destroyer,unit-human-transport"
},
{   2, 0, { "icon-human-ship-armor1" },
    B_Stop, 		0, NULL,
    NULL,		NULL,
    's', "~!STOP",
    "unit-human-oil-tanker,unit-human-oil-tanker-full,unit-gnomish-submarine"
    ",unit-battleship,unit-elven-destroyer,unit-human-transport"
},
{   3, 0, { "icon-human-unload" },
    B_Unload, 		0, NULL,
    NULL,		NULL,
    'u', "~!UNLOAD",
    "unit-human-transport"
},
{   4, 0, { "icon-human-oil-platform" },
    B_Build,		0, "unit-human-oil-platform",
    NULL,		NULL,
    'b', "~!BUILD OIL PLATFORM",
    "unit-human-oil-tanker"
},
{   5, 0, { "icon-human-ship-haul-oil" },
    B_Harvest,		0, NULL,
    NULL,		NULL,
    'h', "~!HAUL OIL",
    "unit-human-oil-tanker"
},
{   6, 0, { "icon-human-ship-return-oil" },
    B_Return,		0, NULL,
    NULL,		NULL,
    'g', "RETURN WITH ~!GOODS",
    "unit-human-oil-tanker-full"
},
{   3, 0, { "icon-human-ship-cannon1" },
    B_Attack,		0, NULL,
    NULL,		NULL,
    'a', "~!ATTACK",
    "unit-gnomish-submarine,unit-battleship,unit-elven-destroyer"
},
{   3, 0, { "icon-human-ship-cannon2" },
    B_Attack,		0, NULL,
    CheckUpgrade,	"upgrade-human-ship-cannon1",
    'a', "~!ATTACK",
    "unit-gnomish-submarine,unit-battleship,unit-elven-destroyer"
},
{   3, 0, { "icon-human-ship-cannon3" },
    B_Attack,		0, NULL,
    CheckUpgrade,	"upgrade-human-ship-cannon2",
    'a', "~!ATTACK",
    "unit-gnomish-submarine,unit-battleship,unit-elven-destroyer"
},
{   4, 0, { "icon-human-patrol-naval" },
    B_Patrol,		0, NULL,
    NULL,		NULL,
    'p', "~!PATROL",
    "unit-gnomish-submarine,unit-battleship,unit-elven-destroyer"
},
#ifdef USE_EXTENSIONS
{   7, 0, { "icon-human-ship-move" },
    B_Move,		0, NULL,
    CheckNoWork,	NULL,
    'm', "SET ~!MOVE",
    "unit-human-shipyard"
},
{   8, 0, { "icon-human-ship-armor1" },
    B_Stop,		0, NULL,
    CheckNoWork,	NULL,
    's', "SET ~!STOP",
    "unit-human-shipyard"
},
{   9, 0, { "icon-human-ship-cannon1" },
    B_Attack,		0, NULL,
    CheckNoWork,	NULL,
    'a', "SET ~!ATTACK",
    "unit-human-shipyard"
},
#endif

// upgrades
{   1, 0, { "icon-sword2" },
    B_Research,		0, "upgrade-sword1",
    CheckResearch,	NULL,
    'w', "UPGRADE S~!WORDS (Damage +2)",
    "unit-human-blacksmith"
},
{   1, 0, { "icon-sword3" },
    B_Research,		0, "upgrade-sword2",
    CheckResearch,	NULL,
    'w', "UPGRADE S~!WORDS (Damage +2)",
    "unit-human-blacksmith"
},
{   2, 0, { "icon-human-shield2" },
    B_Research,		0, "upgrade-human-shield1",
    CheckResearch,	NULL,
    's', "UPGRADE ~!SHIELDS (Armor +2)",
    "unit-human-blacksmith"
},
{   2, 0, { "icon-human-shield3" },
    B_Research,		0, "upgrade-human-shield2",
    CheckResearch,	NULL,
    's', "UPGRADE ~!SHIELDS (Armor +2)",
    "unit-human-blacksmith"
},
{   3, 0, { "icon-ballista1" },
    B_Research,		0, "upgrade-ballista1",
    CheckResearch,	NULL,
    'b', "UPGRADE ~!BALLISTA (Damage +15)",
    "unit-human-blacksmith"
},
{   3, 0, { "icon-ballista2" },
    B_Research,		0, "upgrade-ballista2",
    CheckResearch,	NULL,
    'b', "UPGRADE ~!BALLISTA (Damage +15)",
    "unit-human-blacksmith"
},
{   1, 0, { "icon-arrow2" },
    B_Research,		0, "upgrade-arrow1",
    CheckResearch,	NULL,
    'u', "~!UPGRADE ARROWS (Damage +1)",
    "unit-elven-lumber-mill"
},
{   1, 0, { "icon-arrow3" },
    B_Research,		0, "upgrade-arrow2",
    CheckResearch,	NULL,
    'u', "~!UPGRADE ARROWS (Damage +1)",
    "unit-elven-lumber-mill"
},
{   4, 0, { "icon-ranger" },
    B_Research,		0, "upgrade-ranger",
    CheckResearch,	NULL,
    'r', "ELVEN ~!RANGER TRAINING",
    "unit-elven-lumber-mill"
},
{   4, 0, { "icon-ranger-scouting" },
    B_Research,		0, "upgrade-ranger-scouting",
    CheckResearch,	NULL,
    's', "RANGER ~!SCOUTING (Sight:9)",
    "unit-elven-lumber-mill"
},
{   5, 0, { "icon-longbow" },
    B_Research,		0, "upgrade-longbow",
    CheckResearch,	NULL,
    'l', "RESEARCH ~!LONGBOW (Range +1)",
    "unit-elven-lumber-mill"
},
{   6, 0, { "icon-ranger-marksmanship" },
    B_Research,		0, "upgrade-ranger-marksmanship",
    CheckResearch,	NULL,
    'm', "RANGER ~!MARKSMANSHIP (Damage +3)",
    "unit-elven-lumber-mill"
},
{   1, 0, { "icon-paladin" },
    B_Research,		0, "upgrade-paladin",
    CheckResearch,	NULL,
    'p', "UPGRADES KNIGHTS TO ~!PALADINS",
    "unit-church"
},
{   2, 0, { "icon-heal" },
    B_Research,		0, "upgrade-healing",
    CheckResearch,	NULL,
    'h', "RESEARCH ~!HEALING",
    "unit-church"
},
{   3, 0, { "icon-exorcism" },
    B_Research,		0, "upgrade-exorcism",
    CheckResearch,	NULL,
    'e', "RESEARCH ~!EXORCISM",
    "unit-church"
},
{   2, 0, { "icon-slow" },
    B_Research,		0, "upgrade-slow",
    CheckResearch,	NULL,
    'o', "RESEARCH SL~!OW",
    "unit-mage-tower"
},
{   3, 0, { "icon-flame-shield" },
    B_Research,		0, "upgrade-flame-shield",
    CheckResearch,	NULL,
    'l', "RESEARCH F~!LAME SHIELD",
    "unit-mage-tower"
},
{   4, 0, { "icon-invisibility" },
    B_Research,		0, "upgrade-invisibility",
    CheckResearch,	NULL,
    'i', "RESEARCH ~!INVISIBILITY",
    "unit-mage-tower"
},
{   5, 0, { "icon-polymorph" },
    B_Research,		0, "upgrade-polymorph",
    CheckResearch,	NULL,
    'p', "RESEARCH ~!POLYMORPH",
    "unit-mage-tower"
},
{   6, 0, { "icon-blizzard" },
    B_Research,		0, "upgrade-blizzard",
    CheckResearch,	NULL,
    'b', "RESEARCH ~!BLIZZARD",
    "unit-mage-tower"
},
{   1, 0, { "icon-human-ship-cannon2" },
    B_Research,		0, "upgrade-human-ship-cannon1",
    CheckResearch,	NULL,
    'c', "UPGRADE ~!CANNONS (Damage +5)",
    "unit-human-foundry"
},
{   1, 0, { "icon-human-ship-cannon3" },
    B_Research,		0, "upgrade-human-ship-cannon2",
    CheckResearch,	NULL,
    'c', "UPGRADE ~!CANNONS (Damage +5)",
    "unit-human-foundry"
},
{   2, 0, { "icon-human-ship-armor2" },
    B_Research,		0, "upgrade-human-ship-armor1",
    CheckResearch,	NULL,
    'a', "UPGRADE SHIP ~!ARMOR (Armor +5)",
    "unit-human-foundry"
},
{   2, 0, { "icon-human-ship-armor3" },
    B_Research,		0, "upgrade-human-ship-armor2",
    CheckResearch,	NULL,
    'a', "UPGRADE SHIP ~!ARMOR (Armor +5)",
    "unit-human-foundry"
},

// ============================================================================
// ORCS -----------------------------------------------------------------------
// general commands -- almost all units have it -------------------------------
{   1, 0, { "icon-move-peon" },
    B_Move,		0, NULL,
    NULL,		NULL,
    'm', "~!MOVE",
    ORC_LAND_FORCES3 "," ORC_AIR_FORCES ",unit-attack-peon"
    ",unit-death-knight" ",unit-deathwing" ",orc-group"
},
{   2, 0, { "icon-orc-shield1" },
    B_Stop, 		0, NULL,
    NULL,		NULL,
    's', "~!STOP",
    ORC_LAND_FORCES3 "," ORC_AIR_FORCES ",unit-attack-peon"
    ",unit-death-knight" ",unit-deathwing" ",orc-group"
},
{   2, 0, { "icon-orc-shield2" },
    B_Stop, 		0, NULL,
    CheckUpgrade,	"upgrade-orc-shield1",
    's', "~!STOP",
    ORC_LAND_FORCES
},
{   2, 0, { "icon-orc-shield3" },
    B_Stop, 		0, NULL,
    CheckUpgrade,	"upgrade-orc-shield2",
    's', "~!STOP",
    ORC_LAND_FORCES
},
{   3, 0, { "icon-battle-axe1" },
    B_Attack,		0, NULL,
    NULL,		NULL,
    'a', "~!ATTACK",
    ORC_LAND_FORCES3 ",unit-dragon" ",unit-deathwing" ",orc-group"
},
{   3, 0, { "icon-battle-axe2" },
    B_Attack,		0, NULL,
    CheckUpgrade,	"upgrade-battle-axe1",
    'a', "~!ATTACK",
    ORC_LAND_FORCES
},
{   3, 0, { "icon-battle-axe3" },
    B_Attack,		0, NULL,
    CheckUpgrade,	"upgrade-battle-axe2",
    'a', "~!ATTACK",
    ORC_LAND_FORCES
},
{   3, 0, { "icon-throwing-axe1" },
    B_Attack,		0, NULL,
    NULL,		NULL,
    'a', "~!ATTACK",
    "unit-axethrower,unit-berserker,unit-zuljin"
},
{   3, 0, { "icon-throwing-axe2" },
    B_Attack,		0, NULL,
    CheckUpgrade,	"upgrade-throwing-axe1",
    'a', "~!ATTACK",
    "unit-axethrower,unit-berserker"
},
{   3, 0, { "icon-throwing-axe3" },
    B_Attack,		0, NULL,
    CheckUpgrade,	"upgrade-throwing-axe2",
    'a', "~!ATTACK",
    "unit-axethrower,unit-berserker"
},
{   3, 0, { "icon-catapult1" },
    B_Attack,		0, NULL,
    CheckUpgrade,	"upgrade-catapult1",
    'a', "~!ATTACK",
    "unit-catapult"
},
{   3, 0, { "icon-catapult2" },
    B_Attack,		0, NULL,
    CheckUpgrade,	"upgrade-catapult2",
    'a', "~!ATTACK",
    "unit-catapult"
},

// NOTE: this isn't compatible goblin-sapper didn't have this button
{   4, 0, { "icon-orc-patrol-land" },
    B_Patrol,		0, NULL,
    NULL,		NULL,
    'p', "~!PATROL",
    ORC_LAND_FORCES "," ORC_HEROS1 ",unit-skeleton"
    ",unit-dragon" ",unit-deathwing" ",orc-group"
},
// NOTE: this isn't compatible goblin-sapper didn't have this button
{   5, 0, { "icon-orc-stand-ground" },
    B_StandGround,	0, NULL,
    NULL,		NULL,
    't', "S~!TAND GROUND",
    ORC_LAND_FORCES  "," ORC_HEROS1 ",unit-dragon" ",unit-deathwing"
    ",unit-giant-turtle,unit-ogre-juggernaught,unit-troll-destroyer"
    ",orc-group"
},
{   6, 0, { "icon-orc-attack-ground" },
    B_AttackGround,	0, NULL,
    NULL,		NULL,
    'g', "ATTACK ~!GROUND",
    "unit-catapult,unit-ogre-juggernaught" ",orc-group"
},
{   9, 0, { "icon-orc-demolish" },
    B_Demolish,	0, NULL,
    NULL,		NULL,
    'd', "~!DEMOLISH",
    ORC_LAND_FORCES3 "," ORC_AIR_FORCES ",orc-group"
},

// ogre-mage specific actions --------------------------------------------
{   7, 0, { "icon-eye-of-kilrogg" },
    B_SpellCast,		0, "spell-eye-of-kilrogg",
    CheckTrue,		NULL,
    'k', "EYE OF ~!KILROGG",
    "unit-ogre-mage"
},
{   8, 0, { "icon-bloodlust" },
    B_SpellCast,		0, "spell-bloodlust",
    CheckUpgrade,		"upgrade-bloodlust",
    'b', "~!BLOODLUST",
    "unit-ogre-mage"
},
{   9, 0, { "icon-runes" },
    B_SpellCast,		0, "spell-runes",
    CheckTrue,		"upgrade-runes",
    'r', "~!RUNES",
    "unit-ogre-mage"
},
// cho'gall specific actions --- same as ogre mage but it has them always --
{   7, 0, { "icon-eye-of-kilrogg" },
    B_SpellCast,		0, "spell-eye-of-kilrogg",
    CheckTrue,		NULL,
    'k', "EYE OF ~!KILROGG",
    "unit-cho'gall"
},
{   8, 0, { "icon-bloodlust" },
    B_SpellCast,		0, "spell-bloodlust",
    CheckTrue,		NULL,
    'b', "~!BLOODLUST",
    "unit-cho'gall"
},
{   9, 0, { "icon-runes" },
    B_SpellCast,		0, "spell-runes",
    CheckTrue,		NULL,
    'r', "~!RUNES",
    "unit-cho'gall"
},
// death-knight specific actions --------------------------------------------
{   3, 0, { "icon-touch-of-darkness" },
    B_Attack,		0, NULL,
    NULL,		NULL,
    'a', "TOUCH OF D~!ARKNESS",
    "unit-death-knight" ",unit-gul'dan" ",unit-teron-gorefiend"
},
{   4, 0, { "icon-death-coil" },
    B_SpellCast,		0, "spell-death-coil",
    CheckTrue,		NULL,
    'c', "DEATH ~!COIL",
    "unit-death-knight" ",unit-gul'dan" ",unit-teron-gorefiend"
},
{   5, 0, { "icon-haste" },
    B_SpellCast,		0, "spell-haste",
    CheckUpgrade,		"upgrade-haste",
    'h', "~!HASTE",
    "unit-death-knight"
},
{   6, 0, { "icon-raise-dead" },
    B_SpellCast,		0, "spell-raise-dead",
    CheckUpgrade,		"upgrade-raise-dead",
    'r', "~!RAISE DEAD",
    "unit-death-knight"
},
{   7, 0, { "icon-whirlwind" },
    B_SpellCast,		0, "spell-whirlwind",
    CheckUpgrade,		"upgrade-whirlwind",
    'w', "~!WHIRLWIND",
    "unit-death-knight"
},
{   8, 0, { "icon-unholy-armor" },
    B_SpellCast,		0, "spell-unholy-armor",
    CheckUpgrade,		"upgrade-unholy-armor",
    'u', "~!UNHOLY ARMOR",
    "unit-death-knight"
},
{   9, 0, { "icon-death-and-decay" },
    B_SpellCast,		0, "spell-death-and-decay",
    CheckUpgrade,		"upgrade-death-and-decay",
    'd', "~!DEATH AND DECAY",
    "unit-death-knight"
},
// peon specific actions ------------------------------------------------------
{   4, 0, { "icon-repair" },
    B_Repair,		0, NULL,
    NULL,		NULL,
    'r', "~!REPAIR",
    WORKERS_O
},
{   5, 0, { "icon-harvest" },
    B_Harvest,		0, NULL,
    NULL,		NULL,
    'h', "~!HARVEST LUMBER/MINE GOLD",
    "unit-peon"
},
{   6, 0, { "icon-return-goods-peon" },
    B_Return,		0, NULL,
    NULL,		NULL,
    'g', "RETURN WITH ~!GOODS",
    "unit-peon-with-gold,unit-peon-with-wood"
},
// build basic/advanced structs -----------------------------------------------
{   7, 0, { "icon-build-basic" },
    B_Button,		1, "1",
    NULL,		NULL,
    'b', "~!BUILD BASIC STRUCTURE",
    WORKERS_O
},
{   8, 0, { "icon-build-advanced" },
    B_Button,		2, "2",
    CheckUnit,		"unit-troll-lumber-mill",
    'v', "BUILD AD~!VANCED STRUCTURE",
    WORKERS_O
},
#ifdef USE_EXTENSIONS
{   9, 0, { "icon-build-advanced" },
    B_Button,		3, "3",
    CheckUnits, 	"unit-fortress,unit-temple-of-the-damned"
			",unit-altar-of-storms",
    'e', "BUILD SP~!ECIAL STRUCTURE",
    WORKERS_O
},
#endif
// simple buildings orc -------------------------------------------------------
{   1, 1, { "icon-pig-farm" },
    B_Build,		0, "unit-pig-farm",
    NULL,		NULL,
    'f', "BUILD PIG ~!FARM",
    WORKERS_O  
},
{   2, 1, { "icon-orc-barracks" },
    B_Build,		0, "unit-orc-barracks",
    NULL,		NULL,
    'b', "BUILD ~!BARRACKS",
    WORKERS_O  
},
{   3, 1, { "icon-great-hall" },
    B_Build,		0, "unit-great-hall",
    NULL,		NULL,
    'h', "BUILD GREAT ~!HALL",
    WORKERS_O
},
{   4, 1, { "icon-troll-lumber-mill" },
    B_Build,		0, "unit-troll-lumber-mill",
    NULL,		NULL,
    'l', "BUILD TROLL ~!LUMBER MILL",
    WORKERS_O
},
{   5, 1, { "icon-orc-blacksmith" },
    B_Build,		0, "unit-orc-blacksmith",
    NULL,		NULL,
    's', "BUILD BLACK~!SMITH",
    WORKERS_O 
},
{   7, 1, { "icon-orc-watch-tower" },
    B_Build,		0, "unit-orc-watch-tower",
    NULL,		NULL,
    't', "BUILD ~!TOWER",
    WORKERS_O 
},
{   8, 1, { "icon-orc-wall" },
    B_Build,		0, "unit-orc-wall",
    CheckNetwork,	NULL,
    'w', "BUILD ~!WALL",
    WORKERS_O 
},
{   9, 1, { "icon-cancel" },
    B_Button,		0, NULL,
    NULL,		NULL,
    '\e', "~<ESC~> CANCEL",
    WORKERS_O
},
// orc advanced buildings -----------------------------------------------------
{   1, 2, { "icon-orc-shipyard" },
    B_Build,		0, "unit-orc-shipyard",
    NULL,		NULL,
    's', "BUILD ~!SHIPYARD",
    WORKERS_O 
},
{   2, 2, { "icon-orc-foundry" },
    B_Build,		0, "unit-orc-foundry",
    NULL,		NULL,
    'f', "BUILD ~!FOUNDRY",
    WORKERS_O 
},
{   3, 2, { "icon-orc-refinery" },
    B_Build,		0, "unit-orc-refinery",
    NULL,		NULL,
    'r', "BUILD ~!REFINERY",
    WORKERS_O 
},
{   4, 2, { "icon-goblin-alchemist" },
    B_Build,		0, "unit-goblin-alchemist",
    NULL,		NULL,
    'a', "BUILD GOBLIN ~!ALCHEMIST",
    WORKERS_O 
},
{   5, 2, { "icon-ogre-mound" },
    B_Build,		0, "unit-ogre-mound",
    NULL,		NULL,
    'o', "BUILD ~!OGRE MOUND",
    WORKERS_O 
},
{   6, 2, { "icon-temple-of-the-damned" },
    B_Build,		0, "unit-temple-of-the-damned",
    NULL,		NULL,
    't', "BUILD ~!TEMPLE OF THE DAMNED",
    WORKERS_O 
},
{   7, 2, { "icon-altar-of-storms" },
    B_Build,		0, "unit-altar-of-storms",
    NULL,		NULL,
    'l', "BUILD ~!ALTAR OF STORMS",
    WORKERS_O 
},
{   8, 2, { "icon-dragon-roost" },
    B_Build,		0, "unit-dragon-roost",
    NULL,		NULL,
    'd', "BUILD ~!DRAGON ROOST",
    WORKERS_O 
},
{   9, 2, { "icon-cancel" },
    B_Button,		0, NULL,
    NULL,		NULL,
    '\e', "~<ESC~> CANCEL",
    WORKERS_O
},
// orc special buildings ------------------------------------------------------
#ifdef USE_EXTENSIONS
{   1, 3, { "icon-dark-portal" },
    B_Build,		0, "unit-dark-portal",
    NULL,		NULL,
    'p', "BUILD DARK ~!PORTAL",
    WORKERS_O
},
{   2, 3, { "icon-runestone" },
    B_Build,		0, "unit-runestone",
    NULL,		NULL,
    'r', "BUILD ~!RUNESTONE",
    WORKERS_O
},
{   9, 3, { "icon-cancel" },
    B_Button,		0, NULL,
    NULL,		NULL,
    '\e', "~<ESC~> CANCEL",
    WORKERS_O
},
#endif
// orc buildings commands -----------------------------------------------------
#ifdef USE_EXTENSIONS
{   1, 0, { "icon-critter" },
    B_Train,		0, "unit-critter",
    NULL,		NULL,
    'c', "TRAIN ~!CRITTER",
    "unit-pig-farm"
},
#endif
{   1, 0, { "icon-peon" },
    B_Train,		0, "unit-peon",
    CheckNoResearch,	NULL,
    'p', "TRAIN ~!PEON",
    HALLS_O
},
{   2, 0, { "icon-stronghold" },
    B_UpgradeTo,	0, "unit-stronghold",
    CheckUpgradeTo,	NULL,
    's', "UPGRADE TO ~!STRONGHOLD",
    "unit-great-hall"
},
{   2, 0, { "icon-fortress-upgrade" },
    B_UpgradeTo,	0, "unit-fortress",
    CheckUpgradeTo,	NULL,
    'f', "UPGRADE TO ~!FORTRESS",
    "unit-stronghold"
},
#ifdef USE_EXTENSIONS
{   5, 0, { "icon-harvest" },
    B_Harvest,		0, NULL,
    CheckNoWork,	NULL,
    'h', "SET ~!HARVEST LUMBER/MINE GOLD",
    HALLS_O
},
{   7, 0, { "icon-move-peon" },
    B_Move,		0, NULL,
    CheckNoWork,	NULL,
    'm', "SET ~!MOVE",
    HALLS_O ",unit-orc-barracks" ",unit-temple-of-the-damned"
    ",unit-dragon-roost" ",unit-goblin-alchemist"
},
{   8, 0, { "icon-orc-shield1" },
    B_Stop,		0, NULL,
    CheckNoWork,	NULL,
    's', "SET ~!STOP",
    HALLS_O ",unit-orc-barracks" ",unit-temple-of-the-damned"
    ",unit-dragon-roost" ",unit-goblin-alchemist"
},
{   9, 0, { "icon-battle-axe1" },
    B_Attack,		0, NULL,
    CheckNoWork,	NULL,
    'a', "SET ~!ATTACK",
    HALLS_O ",unit-orc-barracks" ",unit-temple-of-the-damned"
    ",unit-dragon-roost" ",unit-goblin-alchemist"
},
#endif
{   1, 0, { "icon-grunt" },
    B_Train,		0, "unit-grunt",
    NULL,		NULL,
    'g', "TRAIN ~!GRUNT",
    "unit-orc-barracks"
},
{   2, 0, { "icon-axethrower" },
    B_Train,		0, "unit-axethrower",
    NULL,		NULL,
    'a', "TRAIN ~!AXETHROWER",
    "unit-orc-barracks"
},
{   2, 0, { "icon-berserker" },
    B_Train,		0, "unit-berserker",
    NULL,		NULL,
    'b', "TRAIN ~!BERSERKER",
    "unit-orc-barracks"
},
{   3, 0, { "icon-catapult" },
    B_Train,		0, "unit-catapult",
    NULL,		NULL,
    'c', "BUILD ~!CATAPULT",
    "unit-orc-barracks"
},
{   4, 0, { "icon-ogre" },
    B_Train,		0, "unit-ogre",
    NULL,		NULL,
    'o', "TRAIN TWO-HEADED ~!OGRE",
    "unit-orc-barracks"
},
{   4, 0, { "icon-ogre-mage" },
    B_Train,		0, "unit-ogre-mage",
    NULL,		NULL,
    'o', "TRAIN ~!OGRE MAGE",
    "unit-orc-barracks"
},
{   1, 0, { "icon-goblin-zeppelin" },
    B_Train,		0, "unit-goblin-zeppelin",
    NULL,		NULL,
    'z', "BUILD GOBLIN ~!ZEPPELIN",
    "unit-goblin-alchemist"
},
{   2, 0, { "icon-goblin-sappers" },
    B_Train,		0, "unit-goblin-sappers",
    NULL,		NULL,
    's', "TRAIN GOBLIN ~!SAPPERS",
    "unit-goblin-alchemist"
},
{   1, 0, { "icon-death-knight" },
    B_Train,		0, "unit-death-knight",
    NULL,		NULL,
    't', "~!TRAIN DEATH KNIGHT",
    "unit-temple-of-the-damned"
},
{   1, 0, { "icon-dragon" },
    B_Train,		0, "unit-dragon",
    NULL,		NULL,
    'd', "BUILD ~!DRAGON",
    "unit-dragon-roost"
},
{   1, 0, { "icon-orc-oil-tanker" },
    B_Train,		0, "unit-orc-oil-tanker",
    NULL,		NULL,
    'o', "BUILD ~!OIL TANKER",
    "unit-orc-shipyard"
},
{   2, 0, { "icon-troll-destroyer" },
    B_Train,		0, "unit-troll-destroyer",
    NULL,		NULL,
    'd', "BUILD ~!DESTROYER",
    "unit-orc-shipyard"
},
{   3, 0, { "icon-orc-transport" },
    B_Train,		0, "unit-orc-transport",
    NULL,		NULL,
    't', "BUILD ~!TRANSPORT",
    "unit-orc-shipyard"
},
{   4, 0, { "icon-giant-turtle" },
    B_Train,		0, "unit-giant-turtle",
    NULL,		NULL,
    'g', "BUILD ~!GIANT TURTLE",
    "unit-orc-shipyard"
},
{   5, 0, { "icon-ogre-juggernaught" },
    B_Train,		0, "unit-ogre-juggernaught",
    NULL,		NULL,
    'j', "BUILD ~!JUGGERNAUHGT",
    "unit-orc-shipyard"
},
{   1, 0, { "icon-orc-guard-tower" },
    B_UpgradeTo,	0, "unit-orc-guard-tower",
    NULL,		NULL,
    'g', "UPGRADE TO ~!GUARD TOWER",
    "unit-orc-watch-tower"
},
{   2, 0, { "icon-orc-cannon-tower" },
    B_UpgradeTo,	0, "unit-orc-cannon-tower",
    NULL,		NULL,
    'c', "UPGRADE TO ~!CANNON TOWER",
    "unit-orc-watch-tower"
},

// Ships --------------------------------------------------------------------
{   1, 0, { "icon-orc-ship-move" },
    B_Move,		0, NULL,
    NULL,		NULL,
    'm', "~!MOVE",
    "unit-orc-oil-tanker,unit-orc-oil-tanker-full,unit-giant-turtle"
    ",unit-ogre-juggernaught,unit-troll-destroyer,unit-orc-transport"
},
{   2, 0, { "icon-orc-ship-armor1" },
    B_Stop, 		0, NULL,
    NULL,		NULL,
    's', "~!STOP",
    "unit-orc-oil-tanker,unit-orc-oil-tanker-full,unit-giant-turtle"
    ",unit-ogre-juggernaught,unit-troll-destroyer,unit-orc-transport"
},
{   3, 0, { "icon-orc-unload" },
    B_Unload, 		0, NULL,
    NULL,		NULL,
    'u', "~!UNLOAD",
    "unit-orc-transport"
},
{   4, 0, { "icon-orc-oil-platform" },
    B_Build,		0, "unit-orc-oil-platform",
    NULL,		NULL,
    'b', "~!BUILD OIL PLATFORM",
    "unit-orc-oil-tanker"
},
{   5, 0, { "icon-orc-ship-haul-oil" },
    B_Harvest,		0, NULL,
    NULL,		NULL,
    'h', "~!HAUL OIL",
    "unit-orc-oil-tanker"
},
{   6, 0, { "icon-orc-ship-return-oil" },
    B_Return,		0, NULL,
    NULL,		NULL,
    'g', "RETURN WITH ~!GOODS",
    "unit-orc-oil-tanker-full"
},
{   3, 0, { "icon-orc-ship-cannon1" },
    B_Attack,		0, NULL,
    NULL,		NULL,
    'a', "~!ATTACK",
    "unit-giant-turtle,unit-ogre-juggernaught,unit-troll-destroyer"
},
{   3, 0, { "icon-orc-ship-cannon2" },
    B_Attack,		0, NULL,
    CheckUpgrade,	"upgrade-orc-ship-cannon1",
    'a', "~!ATTACK",
    "unit-giant-turtle,unit-ogre-juggernaught,unit-troll-destroyer"
},
{   3, 0, { "icon-orc-ship-cannon3" },
    B_Attack,		0, NULL,
    CheckUpgrade,	"upgrade-orc-ship-cannon2",
    'a', "~!ATTACK",
    "unit-giant-turtle,unit-ogre-juggernaught,unit-troll-destroyer"
},
{   4, 0, { "icon-orc-patrol-naval" },
    B_Patrol,		0, NULL,
    NULL,		NULL,
    'p', "~!PATROL",
    "unit-giant-turtle,unit-ogre-juggernaught,unit-troll-destroyer"
},
#ifdef USE_EXTENSIONS
{   7, 0, { "icon-orc-ship-move" },
    B_Move,		0, NULL,
    CheckNoWork,	NULL,
    'm', "SET ~!MOVE",
    "unit-orc-shipyard"
},
{   8, 0, { "icon-orc-ship-armor1" },
    B_Stop,		0, NULL,
    CheckNoWork,	NULL,
    's', "SET ~!STOP",
    "unit-orc-shipyard"
},
{   9, 0, { "icon-orc-ship-cannon1" },
    B_Attack,		0, NULL,
    CheckNoWork,	NULL,
    'a', "SET ~!ATTACK",
    "unit-orc-shipyard"
},
#endif
// Updates ------------------------------------------------------------------
{   1, 0, { "icon-battle-axe2" },
    B_Research,		0, "upgrade-battle-axe1",
    CheckResearch,	NULL,
    'w', "UPGRADE ~!WEAPONS (Damage +2)",
    "unit-orc-blacksmith"
},
{   1, 0, { "icon-battle-axe3" },
    B_Research,		0, "upgrade-battle-axe2",
    CheckResearch,	NULL,
    'w', "UPGRADE ~!WEAPONS (Damage +2)",
    "unit-orc-blacksmith"
},
{   2, 0, { "icon-orc-shield2" },
    B_Research,		0, "upgrade-orc-shield1",
    CheckResearch,	NULL,
    's', "UPGRADE ~!SHIELDS (Armor +2)",
    "unit-orc-blacksmith"
},
{   2, 0, { "icon-orc-shield3" },
    B_Research,		0, "upgrade-orc-shield2",
    CheckResearch,	NULL,
    's', "UPGRADE ~!SHIELDS (Armor +2)",
    "unit-orc-blacksmith"
},
{   3, 0, { "icon-catapult1" },
    B_Research,		0, "upgrade-catapult1",
    CheckResearch,	NULL,
    'c', "UPGRADE ~!CATAPULT (Damage +15)",
    "unit-orc-blacksmith"
},
{   3, 0, { "icon-catapult2" },
    B_Research,		0, "upgrade-catapult2",
    CheckResearch,	NULL,
    'c', "UPGRADE ~!CATAPULT (Damage +15)",
    "unit-orc-blacksmith"
},
{   1, 0, { "icon-throwing-axe2" },
    B_Research,		0, "upgrade-throwing-axe1",
    CheckResearch,	NULL,
    'u', "~!UPGRADE THROWING AXE (Damage +1)",
    "unit-troll-lumber-mill"
},
{   1, 0, { "icon-throwing-axe3" },
    B_Research,		0, "upgrade-throwing-axe2",
    CheckResearch,	NULL,
    'u', "~!UPGRADE THROWING AXE (Damage +1)",
    "unit-troll-lumber-mill"
},
{   4, 0, { "icon-berserker" },
    B_Research,		0, "upgrade-berserker",
    CheckResearch,	NULL,
    'b', "TROLL ~!BERSERKER TRAINING",
    "unit-troll-lumber-mill"
},
{   4, 0, { "icon-berserker-scouting" },
    B_Research,		0, "upgrade-berserker-scouting",
    CheckResearch,	NULL,
    's', "BERSERKER ~!SCOUTING (Sight:9)",
    "unit-troll-lumber-mill"
},
{   5, 0, { "icon-light-axes" },
    B_Research,		0, "upgrade-light-axes",
    CheckResearch,	NULL,
    'a', "RESEARCH LIGHTER ~!AXES (Range +1)",
    "unit-troll-lumber-mill"
},
{   6, 0, { "icon-berserker-regeneration" },
    B_Research,		0, "upgrade-berserker-regeneration",
    CheckResearch,	NULL,
    'r', "BERSERKER ~!REGENERATION",
    "unit-troll-lumber-mill"
},
{   1, 0, { "icon-ogre-mage" },
    B_Research,		0, "upgrade-ogre-mage",
    CheckResearch,	NULL,
    'm', "UPGRADES OGRES TO ~!MAGES",
    "unit-altar-of-storms"
},
{   2, 0, { "icon-bloodlust" },
    B_Research,		0, "upgrade-bloodlust",
    CheckResearch,	NULL,
    'b', "RESEARCH ~!BLOODLUST",
    "unit-altar-of-storms"
},
{   3, 0, { "icon-runes" },
    B_Research,		0, "upgrade-runes",
    CheckResearch,	NULL,
    'r', "RESEARCH ~!RUNES",
    "unit-altar-of-storms"
},
{   2, 0, { "icon-haste" },
    B_Research,		0, "upgrade-haste",
    CheckResearch,	NULL,
    'h', "RESEARCH ~!HASTE",
    "unit-temple-of-the-damned"
},
{   3, 0, { "icon-raise-dead" },
    B_Research,		0, "upgrade-raise-dead",
    CheckResearch,	NULL,
    'r', "RESEARCH ~!RAISE DEAD",
    "unit-temple-of-the-damned"
},
{   4, 0, { "icon-whirlwind" },
    B_Research,		0, "upgrade-whirlwind",
    CheckResearch,	NULL,
    'w', "RESEARCH ~!WHIRLWIND",
    "unit-temple-of-the-damned"
},
{   5, 0, { "icon-unholy-armor" },
    B_Research,		0, "upgrade-unholy-armor",
    CheckResearch,	NULL,
    'u', "RESEARCH ~!UNHOLY ARMOR",
    "unit-temple-of-the-damned"
},
{   6, 0, { "icon-death-and-decay" },
    B_Research,		0, "upgrade-death-and-decay",
    CheckResearch,	NULL,
    'd', "RESEARCH ~!DEATH AND DECAY",
    "unit-temple-of-the-damned"
},
{   1, 0, { "icon-orc-ship-cannon2" },
    B_Research,		0, "upgrade-orc-ship-cannon1",
    CheckResearch,	NULL,
    'c', "UPGRADE ~!CANNONS (Damage +5)",
    "unit-orc-foundry"
},
{   1, 0, { "icon-orc-ship-cannon3" },
    B_Research,		0, "upgrade-orc-ship-cannon2",
    CheckResearch,	NULL,
    'c', "UPGRADE ~!CANNONS (Damage +5)",
    "unit-orc-foundry"
},
{   2, 0, { "icon-orc-ship-armor2" },
    B_Research,		0, "upgrade-orc-ship-armor1",
    CheckResearch,	NULL,
    'a', "UPGRADE SHIP ~!ARMOR (Armor +5)",
    "unit-orc-foundry"
},
{   2, 0, { "icon-orc-ship-armor3" },
    B_Research,		0, "upgrade-orc-ship-armor2",
    CheckResearch,	NULL,
    'a', "UPGRADE SHIP ~!ARMOR (Armor +5)",
    "unit-orc-foundry"
},

// Neutral --------------------------------------------------------------------
{   1, 0, { "icon-circle-of-power" },
    B_Build,		0, "unit-circle-of-power",
    NULL,		NULL,
    'p', "~!PLACE EXIT/DESTINATION POINT",
    "unit-dark-portal"
},
// general cancel button
{   9, 9, { "icon-cancel" },
    B_Cancel,		0, NULL,
    NULL,		NULL,
    '\e', "~<ESC~> CANCEL",
    "*"
},
{   9, 0, { "icon-cancel" },
    B_Cancel,		0, NULL,
    NULL,		NULL,
    '\e', "~<ESC~> CANCEL UPGRADE",
    "cancel-upgrade"
},
{   9, 0, { "icon-cancel" },
    B_CancelTrain,	0, NULL,
    CheckTraining,	NULL,
    '\e', "~<ESC~> CANCEL UNIT TRAINING",
    "*"
},
{   9, 0, { "icon-cancel" },
    B_CancelBuild,	0, NULL,
    NULL,		NULL,
    '\e', "~<ESC~> CANCEL CONSTRUCTION",
    "cancel-build"
},

{ },
};

//@}
