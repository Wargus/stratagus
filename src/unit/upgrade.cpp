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
/*
**	(c) Copyright 1999-2000 by Vladi Belperchinov-Shabanski
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
#include "player.h"

#include "myendian.h"

#include "etlib/hash.h"

/*----------------------------------------------------------------------------
--	Variables
----------------------------------------------------------------------------*/

global Upgrade Upgrades[MAXUACOUNT];	/// The main user useable upgrades
local int UpgradesCount;		/// Upgrades used

    /// How many upgrades modifiers supported
#define UPGRADE_MODIFIERS_MAX	1024
    /// Upgrades modifiers
local UpgradeModifier* UpgradeModifiers[UPGRADE_MODIFIERS_MAX];
    /// Upgrades modifiers used
local int UpgradeModifiersCount;

local hashtable(int,61) UpgradeHash;	/// lookup table for upgrade names

local int AllowDone;			/// allow already setup.

/**
**	Builtin (default) upgrades.
*/
local struct _wc_upgrades_ {
    const char*	Ident;			/// upgrade ident
    const char*	Icon;			/// Icon ident
    int		Costs[MaxCosts];	/// Costs of the upgrade

    int		AttackRange;		/// attack range modifier
    int		SightRange;		/// sight range modifier
    int		BasicDamage;		/// basic damage modifier
    int		PiercingDamage;		/// piercing damage modifier
    int		Armor;			/// armor modifier
    int		Speed;			/// speed modifier
    int		HitPoints;		/// hit points modifier

    int		CostsModifier[MaxCosts];/// costs modifier

    const char*	Units;			/// units affected
} WcUpgrades[] = {
//			Name			Icon
//  Time  Gold Wood  Oil         At Si BD PD Ar Sp HP  Time Gold Wood  Oil
//		ApplyToUnitsList
{ "upgrade-sword1",			"icon-sword2",
  {  200,  800,   0,   0 },	 0, 0, 0, 2, 0, 0, 0, {   0,   0,   0,   0 },
  "unit-footman,unit-knight,unit-paladin" },
{ "upgrade-sword2",			"icon-sword3",
  {  250, 2400,   0,   0 },	 0, 0, 0, 2, 0, 0, 0, {   0,   0,   0,   0 },
  "unit-footman,unit-knight,unit-paladin" },
{ "upgrade-battle-axe1",		"icon-battle-axe2",
  {  200,  500, 100,   0 },	 0, 0, 0, 2, 0, 0, 0, {   0,   0,   0,   0 },
  "unit-grunt,unit-ogre,unit-ogre-mage" },
{ "upgrade-battle-axe2",		"icon-battle-axe3",
  {  250, 1500, 300,   0 },	 0, 0, 0, 2, 0, 0, 0, {   0,   0,   0,   0 },
  "unit-grunt,unit-ogre,unit-ogre-mage" },
{ "upgrade-arrow1",			"icon-arrow2",
  {  200,  300, 300,   0 },	 0, 0, 0, 1, 0, 0, 0, {   0,   0,   0,   0 },
  "unit-archer,unit-ranger" },
{ "upgrade-arrow2",			"icon-arrow3",
  {  250,  900, 500,   0 },	 0, 0, 0, 1, 0, 0, 0, {   0,   0,   0,   0 },
  "unit-archer,unit-ranger" },
{ "upgrade-throwing-axe1",		"icon-throwing-axe2",
  {  200,  300, 300,   0 },	 0, 0, 0, 1, 0, 0, 0, {   0,   0,   0,   0 },
  "unit-axethrower,unit-berserker" },
{ "upgrade-throwing-axe2",		"icon-throwing-axe3",
  {  250,  900, 500,   0 },	 0, 0, 0, 1, 0, 0, 0, {   0,   0,   0,   0 },
  "unit-axethrower,unit-berserker" },
{ "upgrade-human-shield1",		"icon-human-shield2",
  {  200,  300, 300,   0 },	 0, 0, 0, 0, 2, 0, 0, {   0,   0,   0,   0 },
  "unit-footman,unit-knight,unit-paladin" },
{ "upgrade-human-shield2",		"icon-human-shield3",
  {  250,  900, 500,   0 },	 0, 0, 0, 0, 2, 0, 0, {   0,   0,   0,   0 },
  "unit-footman,unit-knight,unit-paladin" },
{ "upgrade-orc-shield1",		"icon-orc-shield2",
  {  200,  300, 300,   0 },	 0, 0, 0, 0, 2, 0, 0, {   0,   0,   0,   0 },
  "unit-grunt,unit-ogre,unit-ogre-mage" },
{ "upgrade-orc-shield2",		"icon-orc-shield3",
  {  250,  900, 500,   0 },	 0, 0, 0, 0, 2, 0, 0, {   0,   0,   0,   0 },
  "unit-grunt,unit-ogre,unit-ogre-mage" },
{ "upgrade-human-ship-cannon1",		"icon-human-ship-cannon2",
  {  200,  700, 100,1000 },	 0, 0, 0, 5, 0, 0, 0, {   0,   0,   0,   0 },
  "unit-elven-destroyer,unit-battleship,unit-gnomish-submarine" },
{ "upgrade-human-ship-cannon2",		"icon-human-ship-cannon3",
  {  250, 2000, 250,3000 },	 0, 0, 0, 5, 0, 0, 0, {   0,   0,   0,   0 },
  "unit-elven-destroyer,unit-battleship,unit-gnomish-submarine" },
{ "upgrade-orc-ship-cannon1",		"icon-orc-ship-cannon2",
  {  200,  700, 100,1000 },	 0, 0, 0, 5, 0, 0, 0, {   0,   0,   0,   0 },
  "unit-giant-turtle,unit-troll-destroyer,unit-ogre-juggernaught" },
{ "upgrade-orc-ship-cannon2",		"icon-orc-ship-cannon3",
  {  250, 2000, 250,3000 },	 0, 0, 0, 5, 0, 0, 0, {   0,   0,   0,   0 },
  "unit-giant-turtle,unit-troll-destroyer,unit-ogre-juggernaught" },
{ "upgrade-human-ship-armor1",		"icon-human-ship-armor2",
  {  200,  500, 500,   0 },	 0, 0, 0, 0, 5, 0, 0, {   0,   0,   0,   0 },
  "unit-elven-destroyer,unit-battleship,unit-gnomish-submarine" },
{ "upgrade-human-ship-armor2",		"icon-human-ship-armor3",
  {  250, 1500, 900,   0 },	 0, 0, 0, 0, 5, 0, 0, {   0,   0,   0,   0 },
  "unit-elven-destroyer,unit-battleship,unit-gnomish-submarine" },
{ "upgrade-orc-ship-armor1",		"icon-orc-ship-armor2",
  {  200,  500, 500,   0 },	 0, 0, 0, 0, 5, 0, 0, {   0,   0,   0,   0 },
  "unit-giant-turtle,unit-troll-destroyer,unit-ogre-juggernaught" },
{ "upgrade-orc-ship-armor2",		"icon-orc-ship-armor3",
  {  250, 1500, 900,   0 },	 0, 0, 0, 0, 5, 0, 0, {   0,   0,   0,   0 },
  "unit-giant-turtle,unit-troll-destroyer,unit-ogre-juggernaught" },
{ "upgrade-catapult1",			"icon-catapult1",
  {  250, 1500,   0,   0 },	 0, 0, 0,15, 0, 0, 0, {   0,   0,   0,   0 },
  "unit-catapult" },
// FIXME: JOHNS: anybody sure about this +1 range
{ "upgrade-catapult2",			"icon-catapult2",
  {  250, 4000,   0,   0 },	 1, 0, 0,15, 0, 0, 0, {   0,   0,   0,   0 },
  "unit-catapult" },
{ "upgrade-ballista1",			"icon-ballista1",
  {  250, 1500,   0,   0 },	 0, 0, 0,15, 0, 0, 0, {   0,   0,   0,   0 },
  "unit-ballista" },
// FIXME: JOHNS: anybody sure about this +1 range
{ "upgrade-ballista2",			"icon-ballista2",
  {  250, 4000,   0,   0 },	 1, 0, 0,15, 0, 0, 0, {   0,   0,   0,   0 },
  "unit-ballista" },
{ "upgrade-ranger",			NULL,
  {  250, 1500,   0,   0 },	 0, 0, 0, 0, 1, 0, 0, {   0,   0,   0,   0 },
  "unit-archer,unit-ranger" },
{ "upgrade-longbow",			NULL,
  {  250, 2000,   0,   0 },	 0, 1, 0, 0, 0, 0, 0, {   0,   0,   0,   0 },
  "unit-archer,unit-ranger" },
{ "upgrade-ranger-scouting",		NULL,
  {  250, 1500,   0,   0 },	 3, 0, 0, 0, 0, 0, 0, {   0,   0,   0,   0 },
  "unit-archer,unit-ranger" },
{ "upgrade-ranger-marksmanship",	NULL,
  {  250, 2500,   0,   0 },	 0, 0, 0, 3, 0, 0, 0, {   0,   0,   0,   0 },
  "unit-archer,unit-ranger" },
{ "upgrade-berserker",			NULL,
  {  250, 1500,   0,   0 },	 0, 0, 0, 0, 1, 0, 0, {   0,   0,   0,   0 },
  "unit-axethrower,unit-berserker" },
{ "upgrade-light-axes",			NULL,
  {  250, 2000,   0,   0 },	 0, 1, 0, 0, 0, 0, 0, {   0,   0,   0,   0 },
  "unit-axethrower,unit-berserker" },
{ "upgrade-berserker-scouting",		NULL,
  {  250, 1500,   0,   0 },	 3, 0, 0, 0, 0, 0, 0, {   0,   0,   0,   0 },
  "unit-axethrower,unit-berserker" },
{ "upgrade-berserker-regeneration",	NULL,
  {  250, 3000,   0,   0 },	 0, 0, 0, 0, 0, 0, 0, {   0,   0,   0,   0 },
  "unit-axethrower,unit-berserker" },
{ "upgrade-ogre-mage",			NULL,
  {  250, 1000,   0,   0 },	 0, 0, 0, 0, 0, 0, 0, {   0,   0,   0,   0 },
  "" },
{ "upgrade-paladin",			NULL,
  {  250, 1000,   0,   0 },	 0, 0, 0, 0, 0, 0, 0, {   0,   0,   0,   0 },
  "" },
{ "upgrade-holy-vision",		NULL,
  {  0,    0,   0,   0 },	 0, 0, 0, 0, 0, 0, 0, {   0,   0,   0,   0 },
  "" },
{ "upgrade-healing",			"icon-heal",
  {  200, 1000,   0,   0 },	 0, 0, 0, 0, 0, 0, 0, {   0,   0,   0,   0 },
  "" },
{ "upgrade-exorcism",			NULL,
  {  200, 2000,   0,   0 },	 0, 0, 0, 0, 0, 0, 0, {   0,   0,   0,   0 },
  "" },
{ "upgrade-flame-shield",		NULL,
  {  100, 1000,   0,   0 },	 0, 0, 0, 0, 0, 0, 0, {   0,   0,   0,   0 },
  "" },
{ "upgrade-fireball",			NULL,
  {  0,    0,   0,   0 },	 0, 0, 0, 0, 0, 0, 0, {   0,   0,   0,   0 },
  "" },
{ "upgrade-slow",			NULL,
  {  100,  500,   0,   0 },	 0, 0, 0, 0, 0, 0, 0, {   0,   0,   0,   0 },
  "" },
{ "upgrade-invisibility",		NULL,
  {  200, 2500,   0,   0 },	 0, 0, 0, 0, 0, 0, 0, {   0,   0,   0,   0 },
  "" },
{ "upgrade-polymorph",			NULL,
  {  200, 2000,   0,   0 },	 0, 0, 0, 0, 0, 0, 0, {   0,   0,   0,   0 },
  "" },
{ "upgrade-blizzard",			NULL,
  {  200, 2000,   0,   0 },	 0, 0, 0, 0, 0, 0, 0, {   0,   0,   0,   0 },
  "" },
{ "upgrade-eye-of-kilrogg",		NULL,
  {  0,    0,   0,   0 },	 0, 0, 0, 0, 0, 0, 0, {   0,   0,   0,   0 },
  "" },
{ "upgrade-bloodlust",			NULL,
  {  100, 1000,   0,   0 },	 0, 0, 0, 0, 0, 0, 0, {   0,   0,   0,   0 },
  "" },
{ "upgrade-raise-dead",			NULL,
  {  100, 1500,   0,   0 },	 0, 0, 0, 0, 0, 0, 0, {   0,   0,   0,   0 },
  "" },
{ "upgrade-death-coil",			NULL,
  {  100,    0,   0,   0 },	 0, 0, 0, 0, 0, 0, 0, {   0,   0,   0,   0 },
  "" },
{ "upgrade-whirlwind",			NULL,
  {  150, 1500,   0,   0 },	 0, 0, 0, 0, 0, 0, 0, {   0,   0,   0,   0 },
  "" },
{ "upgrade-haste",			NULL,
  {  100,  500,   0,   0 },	 0, 0, 0, 0, 0, 0, 0, {   0,   0,   0,   0 },
  "" },
{ "upgrade-unholy-armor",		NULL,
  {  200, 2500,   0,   0 },	 0, 0, 0, 0, 0, 0, 0, {   0,   0,   0,   0 },
  "" },
{ "upgrade-runes",			NULL,
  {  150, 1000,   0,   0 },	 0, 0, 0, 0, 0, 0, 0, {   0,   0,   0,   0 },
  "" },
{ "upgrade-death-and-decay",		NULL,
  {  200, 2000,   0,   0 },	 0, 0, 0, 0, 0, 0, 0, {   0,   0,   0,   0 },
  "" },
};

/**
**	W*rCr*ft number to internal upgrade name.
*/
local const char* UpgradeWcNames[] = {
    "upgrade-sword1",
    "upgrade-sword2",
    "upgrade-battle-axe1",
    "upgrade-battle-axe2",
    "upgrade-arrow1",
    "upgrade-arrow2",
    "upgrade-throwing-axe1",
    "upgrade-throwing-axe2",
    "upgrade-human-shield1",
    "upgrade-human-shield2",
    "upgrade-orc-shield1",
    "upgrade-orc-shield2",
    "upgrade-human-ship-cannon1",
    "upgrade-human-ship-cannon2",
    "upgrade-orc-ship-cannon1",
    "upgrade-orc-ship-cannon2",
    "upgrade-human-ship-armor1",
    "upgrade-human-ship-armor2",
    "upgrade-orc-ship-armor1",
    "upgrade-orc-ship-armor2",
    "upgrade-catapult1",
    "upgrade-catapult2",
    "upgrade-ballista1",
    "upgrade-ballista2",
    "upgrade-ranger",
    "upgrade-longbow",
    "upgrade-ranger-scouting",
    "upgrade-ranger-marksmanship",
    "upgrade-berserker",
    "upgrade-light-axes",
    "upgrade-berserker-scouting",
    "upgrade-berserker-regeneration",
    "upgrade-ogre-mage",
    "upgrade-paladin",
    "upgrade-holy-vision",
    "upgrade-healing",
    "upgrade-exorcism",
    "upgrade-flame-shield",
    "upgrade-fireball",
    "upgrade-slow",
    "upgrade-invisibility",
    "upgrade-polymorph",
    "upgrade-blizzard",
    "upgrade-eye-of-kilrogg",
    "upgrade-bloodlust",
    "upgrade-raise-dead",
    "upgrade-death-coil",
    "upgrade-whirlwind",
    "upgrade-haste",
    "upgrade-unholy-armor",
    "upgrade-runes",
    "upgrade-death-and-decay",
};

/*----------------------------------------------------------------------------
--	Functions
----------------------------------------------------------------------------*/

/**
**	Add an upgrade.
**
**	@param aIdent	upgrade identifier.
**
**	FIXME: docu
**
**	@returns upgrade id or -1 for error
*/
local Upgrade* AddUpgrade(const char* aIdent,const char* aIcon,int* aCosts)
{
    char buf[256];
    int i;

    //	Check for free slot.

    if ( UpgradesCount == MAXUACOUNT ) {
	return NULL;
    }

    //	Fill upgrade structure

    Upgrades[UpgradesCount].Ident = strdup( aIdent );

    for( i=0; i<MaxCosts; ++i ) {
	Upgrades[UpgradesCount].Costs[i]=aCosts[i];
    }

    if( aIcon ) {
	Upgrades[UpgradesCount].Icon = IconByIdent(aIcon);
    } else {
	sprintf(buf,"icon-%s",aIdent+8);
	Upgrades[UpgradesCount].Icon = IconByIdent(buf);
    }

    *(Upgrade**)hash_add(UpgradeHash,Upgrades[UpgradesCount].Ident)
	    =&Upgrades[UpgradesCount];

    return &Upgrades[UpgradesCount++];
}

/**
**	Setup allow.
*/
local void SetupAllow(void)
{
    int z;

    if( !AllowDone ) {
	// First we have to allow everything

	for ( z = 0; z < PlayerMax; z++ ) {
	    memset(Players[z].Allow.Upgrades,'A'
		    ,sizeof(Players[z].Allow.Upgrades));
	    memset(Players[z].Allow.Units,'A'
		    ,sizeof(Players[z].Allow.Units));
	    memset(Players[z].Allow.Actions,'A'
		    ,sizeof(Players[z].Allow.Actions));
	}

	// Give some upgrades as default.

	for ( z = 0; z < PlayerMax; z++ ) {
	    AllowUpgradeByIdent(&Players[z],"upgrade-death-coil",'R');
	    AllowUpgradeByIdent(&Players[z],"upgrade-fireball",'R');
	    AllowUpgradeByIdent(&Players[z],"upgrade-holy-vision",'R');
	    AllowUpgradeByIdent(&Players[z],"upgrade-eye-of-kilrogg",'R');
	}

	AllowDone=1;
    }
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

    upgrade=(Upgrade**)hash_find(UpgradeHash,(char*)ident);

    if( upgrade ) { 
	return *upgrade;
    }

    DebugLevel0(__FUNCTION__": upgrade %s not found\n",ident);

    return NULL;
}

/**
**	Init upgrade/allow structures
*/
global void InitUpgrades(void)
{
    int z;

    DebugLevel3(__FUNCTION__": ---------------------------------------\n");
    if( !UpgradesCount ) {
	InitIcons();			// wired, but I need them here

	// Setup the default upgrades
	for( z = 0; z <sizeof(WcUpgrades)/sizeof(*WcUpgrades); z++ ) {
	    // FIXME: perhaps we should parse some structures.
	    AddSimpleUpgrade(
		WcUpgrades[z].Ident,
		WcUpgrades[z].Icon,

		WcUpgrades[z].Costs,

		WcUpgrades[z].AttackRange,
		WcUpgrades[z].SightRange,
		WcUpgrades[z].BasicDamage,
		WcUpgrades[z].PiercingDamage,
		WcUpgrades[z].Armor,
		WcUpgrades[z].Speed,
		WcUpgrades[z].HitPoints,

		WcUpgrades[z].CostsModifier,

		WcUpgrades[z].Units);
	}
    }

    SetupAllow();
}

/**
**	Parse ALOW area from puds.
**
**	@param alow	Pointer to alow area.
**	@param length	length of alow area.
*/
global void ParsePudALOW(const char* alow,int length)
{
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

    DebugLevel0(__FUNCTION__": Length %d\n",length);
    //SetupAllow();
    InitUpgrades();

    //
    //	Allow units
    //
    for( player=Players; player<Players+16; ++player ) {
	for( i=0; i<4; ++i ) {
	    int v;

	    v=*alow++;
	    DebugLevel3(__FUNCTION__": %x\n",v);
	    for( b=0; b<8; ++b ) {
		if( v&(1<<b) ) {
		    if( units[i*16+0+b*2] ) {
			DebugLevel3(__FUNCTION__": %s +\n",
				units[i*16+0+b*2]);
				
			AllowUnitByIdent(player,units[i*16+0+b*2],'A');
			AllowUnitByIdent(player,units[i*16+1+b*2],'A');
		    }
		} else {
		    if( units[i*16+0+b*2] ) {
			DebugLevel3(__FUNCTION__": %s -\n",
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
	    DebugLevel0(__FUNCTION__": %x\n",v);
	    for( b=0; b<8; ++b ) {
		if( v&(1<<b) ) {
		    if( spells[i*8+b] ) {
			DebugLevel0(__FUNCTION__": %s +R\n",spells[i*8+b]);
				
			AllowUpgradeByIdent(player,spells[i*8+b],'R');
		    }
		} else {
		    if( spells[i*8+b] ) {
			DebugLevel0(__FUNCTION__": %s -F\n",spells[i*8+b]);

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
	    DebugLevel0(__FUNCTION__": %x\n",v);
	    for( b=0; b<8; ++b ) {
		if( v&(1<<b) ) {
		    if( spells[i*8+b] ) {
			DebugLevel0(__FUNCTION__": %s +A\n",spells[i*8+b]);
				
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
	    DebugLevel0(__FUNCTION__": %x\n",v);
	    for( b=0; b<8; ++b ) {
		if( v&(1<<b) ) {
		    if( spells[i*8+b] ) {
			DebugLevel0(__FUNCTION__": %s +U\n",spells[i*8+b]);
				
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
	    DebugLevel0(__FUNCTION__": %x\n",v);
	    for( b=0; b<8; ++b ) {
		if( v&(1<<b) ) {
		    if( upgrades[i*16+b*2+0] ) {
			DebugLevel0(__FUNCTION__": %s +A\n",upgrades[i*16+b*2]);
				
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
	    DebugLevel0(__FUNCTION__": %x\n",v);
	    for( b=0; b<8; ++b ) {
		if( v&(1<<b) ) {
		    if( upgrades[i*16+b*2+0] ) {
			DebugLevel0(__FUNCTION__": %s +U\n",upgrades[i*16+b*2]);
				
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
    int j;
    int time;
    int gold;
    int lumber;
    int oil;
    int icon;
    int group;
    int flags;

    DebugLevel3(__FUNCTION__": Length %d\n",length);
    DebugCheck( length!=780 );
    DebugLevel3(__FUNCTION__": Upgrades %d\n",UpgradesCount);

    for( i=0; i<52; ++i ) {
	time=((unsigned char*)ugrd)[i];
	gold=AccessLE16(	ugrd+52+(i)*2);
	lumber=AccessLE16(	ugrd+52+(i+52)*2);
	oil=AccessLE16(		ugrd+52+(i+52+52)*2);
	icon=AccessLE16(	ugrd+52+(i+52+52+52)*2);
	group=AccessLE16(	ugrd+52+(i+52+52+52+52)*2);
	flags=AccessLE16(	ugrd+52+(i+52+52+52+52+52)*2);
	DebugLevel3(__FUNCTION__": %s %d,%d,%d,%d %d %d %08X\n"
		,UpgradeWcNames[i] 
		,time,gold,lumber,oil
		,icon,group,flags);
	if( UpgradesCount ) {
	    printf("// FIXME: no bock to write this better\n");
	}
	WcUpgrades[i].Costs[TimeCost]=time;
	WcUpgrades[i].Costs[GoldCost]=gold;
	WcUpgrades[i].Costs[WoodCost]=lumber;
	WcUpgrades[i].Costs[OilCost]=oil;
	for( j=OilCost+1; j<MaxCosts; ++j ) {
	    WcUpgrades[i].Costs[j]=0;
	}
	WcUpgrades[i].Icon=IdentOfIcon(icon);

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
    fprintf(file,";;; MODULE: upgrades $Id$\n");

    //
    //	Save all upgrade modifiers.
    //
    for( i=0; i<UpgradeModifiersCount; ++i ) {
	fprintf(file,"(define-modifier \"%s\"\n",
		Upgrades[UpgradeModifiers[i]->uid].Ident);

	if( UpgradeModifiers[i]->mods.AttackRange ) {
	    fprintf(file,"  '('attack-range %d)\n"
		    ,UpgradeModifiers[i]->mods.AttackRange );
	}
	if( UpgradeModifiers[i]->mods.SightRange ) {
	    fprintf(file,"  '('sight-range %d)\n"
		    ,UpgradeModifiers[i]->mods.SightRange );
	}
	if( UpgradeModifiers[i]->mods.SightRange ) {
	    fprintf(file,"  '('attack-range %d)\n"
		    ,UpgradeModifiers[i]->mods.SightRange );
	}
	if( UpgradeModifiers[i]->mods.BasicDamage ) {
	    fprintf(file,"  '('basic-damage %d)\n"
		    ,UpgradeModifiers[i]->mods.BasicDamage );
	}
	if( UpgradeModifiers[i]->mods.PiercingDamage ) {
	    fprintf(file,"  '('piercing-damage %d)\n"
		    ,UpgradeModifiers[i]->mods.PiercingDamage );
	}
	if( UpgradeModifiers[i]->mods.Armor ) {
	    fprintf(file,"  '('armor %d)\n"
		    ,UpgradeModifiers[i]->mods.Armor );
	}
	if( UpgradeModifiers[i]->mods.Speed ) {
	    fprintf(file,"  '('speed %d)\n"
		    ,UpgradeModifiers[i]->mods.Speed );
	}
	if( UpgradeModifiers[i]->mods.HitPoints ) {
	    fprintf(file,"  '('hit-points %d)\n"
		    ,UpgradeModifiers[i]->mods.HitPoints );
	}

	for( j=0; j<MaxCosts; ++j ) {
	    if( UpgradeModifiers[i]->mods.Costs[j] ) {
		fprintf(file,"  '('%s-cost %d)\n"
			,DEFAULT_NAMES[j],UpgradeModifiers[i]->mods.Costs[j]);
	    }
	}


	fprintf(file,"  )\n");
    }

#if 0
  // allow/forbid bitmaps -- used as chars for example:
  // `?' -- leave as is, `F' -- forbid, `A' -- allow
  char af_units[MAXUNITTYPES];   // allow/forbid units
  char af_actions[MAXACTIONS]; // allow/forbid actions
  char af_upgrades[MAXUPGRADES]; // allow/forbid upgrades
  char apply_to[MAXUNITTYPES]; // which unit types are affected
#endif

    //
    //	Save all upgrades
    //
    for( i=0; i<UpgradesCount; ++i ) {
	fprintf(file,"(define-upgrade \"%s\" \"%s\"\n"
		,Upgrades[i].Ident,IdentOfIcon(Upgrades[i].Icon));
	fprintf(file,"  #(");
	for( j=0; j<MaxCosts; ++j ) {
	    fprintf(file," %5d",Upgrades[i].Costs[j]);
	}

	fprintf(file,"))\n");
    }

    fprintf(file,"\n");

    // Save the allow 
    fprintf(file,"(define-allow\n");
    for( i=0; i<sizeof(UnitTypes)/sizeof(*UnitTypes); ++i ) {
	fprintf(file,"  \"%s\"\t",UnitTypes[i].Ident);
	if( strlen(UnitTypes[i].Ident)<12 ) {
	    fprintf(file,"\t\t\t\"");
	} else if( strlen(UnitTypes[i].Ident)<20 ) {
	    fprintf(file,"\t\t\"");
	} else if( strlen(UnitTypes[i].Ident)<28 ) {
	    fprintf(file,"\t\"");
	} else {
	    fprintf(file,"\"");
	}
	for( p=0; p<PlayerMax; ++p ) {
	    fprintf(file,"%c",Players[p].Allow.Units[i]);
	}
	fprintf(file,"\"\n");
    }

    fprintf(file,"\n");

#if 0
    // Save the actions 
    for( i=0; i<20; ++i ) {
	for( p=0; p<PlayerMax; ++p ) {
	    fprintf(file,"(allow-action %d \"%s\" \"%c\")\n"
		    ,p,UnitTypes[i].Ident,Players[p].Allow.Actions[i]);
	}
    }

    fprintf(file,"\n");
#endif

    // Save the upgrades 
    for( i=0; i<UpgradesCount; ++i ) {
	fprintf(file,"  \"%s\"\t",Upgrades[i].Ident);
	if( strlen(Upgrades[i].Ident)<12 ) {
	    fprintf(file,"\t\t\t\"");
	} else if( strlen(Upgrades[i].Ident)<20 ) {
	    fprintf(file,"\t\t\"");
	} else if( strlen(Upgrades[i].Ident)<28 ) {
	    fprintf(file,"\t\"");
	} else {
	    fprintf(file,"\"");
	}
	for( p=0; p<PlayerMax; ++p ) {
	    fprintf(file,"%c",Players[p].Allow.Upgrades[i]);
	}
	fprintf(file,"\"\n");
    }
    fprintf(file,")\n");
}

/*----------------------------------------------------------------------------
--	Ccl part of upgrades
----------------------------------------------------------------------------*/

#if defined(USE_CCL) || defined(USE_CCL2)

#include "ccl.h"

/**
**	Define a new upgrade modifier.
*/
local SCM CclDefineModifier(SCM list)
{
    SCM value;
    const char* str;

    value=gh_car(list);
    list=gh_cdr(list);

    str=gh_scm2newstr(value,NULL);
    DebugLevel2(__FUNCTION__"\tName: %s\n",str);

    //CclFree(type->Name);
    //type->Name=str;

    DebugLevel0(__FUNCTION__": not written\n");

    return SCM_UNSPECIFIED;
}

/**
**	Define a new upgrade.
*/
local SCM CclDefineUpgrade(SCM list)
{
    SCM value;
    const char* str;

    value=gh_car(list);
    list=gh_cdr(list);

    str=gh_scm2newstr(value,NULL);
    DebugLevel2(__FUNCTION__"\tName: %s\n",str);

    //CclFree(type->Name);
    //type->Name=str;

    DebugLevel0(__FUNCTION__": not written\n");

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

	DebugLevel3(__FUNCTION__"\tName: %s - %s\n",str,ids);

	n=strlen(ids);
	if( n>16 ) {
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
**	Register CCL features for upgrades.
*/
global void UpgradesCclRegister(void)
{
    gh_new_procedureN("define-modifier",CclDefineModifier);
    gh_new_procedureN("define-upgrade",CclDefineUpgrade);
    gh_new_procedureN("define-allow",CclDefineAllow);
}

#endif	// defined(USE_CCL) || defined(USE_CCL2)


































// FIXME: Johns stops here

/*----------------------------------------------------------------------------
--	Init/Done/Add functions
----------------------------------------------------------------------------*/

#if 0
void UpgradesDone(void) // free upgrade/allow structures
{
  int z;

  memset( &Upgrades, 0, sizeof(Upgrades) );
  UpgradesCount = 0;

  for ( z = 0; z < UpgradeModifiersCount; z++ )
    free( UpgradeModifiers[z] );
  UpgradeModifiersCount = 0;
}
#endif


// returns upgrade modifier id or -1 for error ( actually this id is useless, just error checking )
global int AddUpgradeModifier( int aUid,
    int aattack_range,
    int asight_range,
    int abasic_damage,
    int apiercing_damage,
    int aarmor,
    int aspeed,
    int ahit_points,

    int* acosts,

    // following are comma separated list of required string id's

    const char* aaf_units,    // "A:unit-mage,F:unit-grunt" -- allow mages, forbid grunts
    const char* aaf_actions,  // "A:PeonAttack"
    const char* aaf_upgrades, // "F:upgrade-Shield1,R:upgrade-ShieldTotal" -- :)
    const char* aapply_to	    // "unit-Peon,unit-Peasant"

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

    um->uid = aUid;

    // get/save stats modifiers
    um->mods.AttackRange	= aattack_range;
    um->mods.SightRange		= asight_range;
    um->mods.BasicDamage	= abasic_damage;
    um->mods.PiercingDamage	= apiercing_damage;
    um->mods.Armor		= aarmor;
    um->mods.Speed		= aspeed;
    um->mods.HitPoints		= ahit_points;

    for( i=0; i<MaxCosts; ++i ) {
	um->mods.Costs[i]	= acosts[i];
    }

    // FIXME: all the thing below is sensitive to the format of the string!
    // FIXME: it will be good if things are checked for errors better!
    // FIXME: perhaps the function `strtok()' should be replaced with local one?

    memset( um->af_units,    '?', sizeof(um->af_units)    );
    memset( um->af_actions,  '?', sizeof(um->af_actions)  );
    memset( um->af_upgrades, '?', sizeof(um->af_upgrades) );
    memset( um->apply_to,    '?', sizeof(um->apply_to)    );

    //
    // get allow/forbid's for units
    //
    s1 = strdup( aaf_units );
    DebugCheck(!s1);
    for( s2 = strtok( s1, "," ); s2; s2=strtok( NULL, "," ) ) {
	int id;
	DebugCheck(! (s2[0] == 'A' || s2[0] == 'F' ));
	DebugCheck(! (s2[1] == ':' ));
	id = UnitTypeIdByIdent( s2+2 );
	if ( id == -1 ) {
	    continue;		// should we cancel all and return error?!
	}
	um->af_units[id] = s2[0];
    }
    free(s1);

    //
    // get allow/forbid's for actions
    //
    s1 = strdup( aaf_actions );
    DebugCheck(!s1);
    for( s2 = strtok( s1, "," ); s2; s2=strtok( NULL, "," ) ) {
	int id;
	DebugCheck(!( s2[0] == 'A' || s2[0] == 'F' ));
	DebugCheck(!( s2[1] == ':' ));
	id = ActionIdByIdent( s2+2 );
	if ( id == -1 ) {
	    continue;		// should we cancel all and return error?!
	}
	um->af_actions[id] = s2[0];
    }
    free(s1);

    //
    // get allow/forbid's for upgrades
    //
    s1 = strdup( aaf_upgrades );
    DebugCheck(!s1);
    for( s2 = strtok( s1, "," ); s2; s2=strtok( NULL, "," ) ) {
	int id;
	DebugCheck(!( s2[0] == 'A' || s2[0] == 'F' || s2[0] == 'R' ));
	DebugCheck(!( s2[1] == ':' ));
	id = UpgradeIdByIdent( s2+2 );
	if ( id == -1 ) {
	    continue;		// should we cancel all and return error?!
	}
	um->af_upgrades[id] = s2[0];
    }
    free(s1);

    //
    // get units that are affected by this upgrade
    //
    s1 = strdup( aapply_to );
    DebugCheck(!s1);
    for( s2 = strtok( s1, "," ); s2; s2=strtok( NULL, "," ) ) {
	int id;

	DebugLevel3(__FUNCTION__": %s\n",s2);
	id = UnitTypeIdByIdent( s2 );
	if ( id == -1 ) {
	    break;		// cade: should we cancel all and return error?!
	}
	um->apply_to[id] = 'X'; // something other than '?'
    }
    free(s1);

    UpgradeModifiers[UpgradeModifiersCount] = um;
    UpgradeModifiersCount++;

    return UpgradeModifiersCount-1;
}

// this function is used for define `simple' upgrades
// with only one modifier
global void AddSimpleUpgrade( const char* aIdent,
    const char* aIcon,
    // upgrade costs
    int* aCosts,
    // upgrade modifiers
    int aattack_range,
    int asight_range,
    int abasic_damage,
    int apiercing_damage,
    int aarmor,
    int aspeed,
    int ahit_points,

    int* mcosts,

    const char* aapply_to		// "unit-Peon,unit-Peasant"
    )
{
    Upgrade* up;

    up = AddUpgrade(aIdent,aIcon,aCosts);
    if ( !up )  {
	return;
    }
    AddUpgradeModifier(up-Upgrades,aattack_range,asight_range,abasic_damage,
	    apiercing_damage,aarmor,aspeed,ahit_points,
	    mcosts,
	    "","","", // no allow/forbid maps
	    aapply_to);
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
    DebugLevel0(__FUNCTION__": fix this %s\n",sid);
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
    DebugLevel0(__FUNCTION__": fix this %s\n",sid);
    return -1;
}

// FIXME: Docu
global int ActionIdByIdent( const char* sid )
{
  // FIXME: there's no actions table yet
  DebugLevel0(__FUNCTION__": fix this %s\n",sid);
  return -1;
}

/*----------------------------------------------------------------------------
--	Upgrades
----------------------------------------------------------------------------*/

// amount==-1 to cancel upgrade, could happen when building destroyed during upgrade
// using this we could have one upgrade research in two buildings, so we can have
// this upgrade faster.
void UpgradeIncTime( Player* player, int id, int amount )
{
    player->UTimers.upgrades[id] += amount;
    if ( player->UTimers.upgrades[id] >= Upgrades[id].Costs[TimeCost] )
    {
	player->UTimers.upgrades[id] = Upgrades[id].Costs[TimeCost];
	UpgradeAcquire( player, &Upgrades[id] );
    }
}

// this function will mark upgrade done and do all required modifications to
// unit types and will modify allow/forbid maps
void ApplyUpgradeModifier( Player* player, UpgradeModifier* um )
{
    int z;
    int j;
    int pn = player-Players; // player number

    for ( z = 0; z < MAXUACOUNT; z++ )
    {
	// allow/forbid unit types for player
	if ( um->af_units[z] == 'A' ) player->Allow.Units[z] = 'A';
	if ( um->af_units[z] == 'F' ) player->Allow.Units[z] = 'F';

	// allow/forbid actions for player
	if ( um->af_actions[z] == 'A' ) player->Allow.Actions[z] = 'A';
	if ( um->af_actions[z] == 'F' ) player->Allow.Actions[z] = 'F';

	// allow/forbid upgrades for player
	if ( player->Allow.Upgrades[z] != 'R' )
	{ // only if upgrade is not acquired
	    if ( um->af_upgrades[z] == 'A' ) player->Allow.Upgrades[z] = 'A';
	    if ( um->af_upgrades[z] == 'F' ) player->Allow.Upgrades[z] = 'F';
	    // we can even have upgrade acquired w/o costs
	    if ( um->af_upgrades[z] == 'R' ) player->Allow.Upgrades[z] = 'R';
	}

	DebugCheck(!( um->apply_to[z] == '?' || um->apply_to[z] == 'X' ));
	if ( um->apply_to[z] == 'X' )
	{ // this modifier should be applied to unittype id == z

	    DebugLevel3(__FUNCTION__": applied to %d\n",z);
	    // upgrade stats
	    UnitTypes[z].Stats[pn].AttackRange	+= um->mods.AttackRange;
	    UnitTypes[z].Stats[pn].SightRange	+= um->mods.SightRange;
	    UnitTypes[z].Stats[pn].BasicDamage	+= um->mods.BasicDamage;
	    UnitTypes[z].Stats[pn].PiercingDamage += um->mods.PiercingDamage;
	    UnitTypes[z].Stats[pn].Armor	+= um->mods.Armor;
	    UnitTypes[z].Stats[pn].Speed	+= um->mods.Speed;
	    UnitTypes[z].Stats[pn].HitPoints	+= um->mods.HitPoints;
		

	    // upgrade costs :)
	    for( j=0; j<MaxCosts; ++j ) {
		UnitTypes[z].Stats[pn].Costs[j]	+= um->mods.Costs[j];
	    }

	    UnitTypes[z].Stats[pn].Level++;
	}
    }
}

 // called by UpgradeIncTime() when timer reached
global void UpgradeAcquire( Player* player, Upgrade* upgrade )
{
    int z;
    int id;

    id=upgrade-Upgrades;
    player->UTimers.upgrades[id] = upgrade->Costs[TimeCost];
    AllowUpgradeId( player, id, 'R' );		// research done

    for ( z = 0; z < UpgradeModifiersCount; z++ ) {
	if ( UpgradeModifiers[z]->uid == id ) {
	    ApplyUpgradeModifier( player, UpgradeModifiers[z] );
	}
    }
}

// for now it will be empty?
// perhaps acquired upgrade can be lost if ( for example ) a building is lost
// ( lumber mill? stronghold? )
// this function will apply all modifiers in reverse way
void UpgradeLost( Player* player, int id )
{
  return; //FIXME: remove this if implemented below

  player->UTimers.upgrades[id] = 0;
  AllowUpgradeId( player, id, 'A' ); // research is lost i.e. available
  // FIXME: here we should reverse apply upgrade...
}

/*----------------------------------------------------------------------------
--	Allow(s)
----------------------------------------------------------------------------*/

// all the following functions are just map handlers, no specific notes
void AllowUnitId( Player* player, int id, char af ) // id -- unit type id, af -- `A'llow/`F'orbid
{
  DebugCheck(!( af == 'A' || af == 'F' ));
  player->Allow.Units[id] = af;
}

void AllowActionId( Player* player,  int id, char af )
{
  DebugCheck(!( af == 'A' || af == 'F' ));
  player->Allow.Actions[id] = af;
}

void AllowUpgradeId( Player* player,  int id, char af )
{
  DebugCheck(!( af == 'A' || af == 'F' || af == 'R' ));
  player->Allow.Upgrades[id] = af;
}

char UnitIdAllowed(const Player* player,  int id )
{
  if ( id < 0 || id >= MAXUACOUNT ) return 'F';
  return player->Allow.Units[id];
}

char ActionIdAllowed(const Player* player,  int id )
{
  if ( id < 0 || id >= MAXUACOUNT ) return 'F';
  return player->Allow.Actions[id];
}

global char UpgradeIdAllowed(const Player* player,  int id )
{
    // JOHNS: Don't be kind, the people should code correct!
    DebugCheck( id < 0 || id >= MAXUACOUNT );

    return player->Allow.Upgrades[id];
}

// ***************by sid's
void UpgradeIncTime2( Player* player, char* sid, int amount ) // by ident string
  { UpgradeIncTime( player, UpgradeIdByIdent(sid), amount ); }
void UpgradeLost2( Player* player, char* sid ) // by ident string
  { UpgradeLost( player, UpgradeIdByIdent(sid) ); }

void AllowUnitByIdent( Player* player,  const char* sid, char af )
     { AllowUnitId( player,  UnitTypeIdByIdent(sid), af ); };
void AllowActionByIdent( Player* player,  const char* sid, char af )
     { AllowActionId( player,  ActionIdByIdent(sid), af ); };
void AllowUpgradeByIdent( Player* player,  const char* sid, char af )
     { AllowUpgradeId( player,  UpgradeIdByIdent(sid), af ); };

void AllowByIdent(Player* player,  const char* sid, char af )
{
    if( !strncmp(sid,"unit-",5) ) {
	AllowUnitByIdent(player,sid,af);
    } else if( !strncmp(sid,"upgrade-",8) ) {
	AllowUpgradeByIdent(player,sid,af);
    } else {
	DebugLevel0(__FUNCTION__": wrong sid %s\n",sid);
    }
}

char UnitIdentAllowed(const Player* player,const char* sid )
     { return UnitIdAllowed( player,  UnitTypeIdByIdent(sid) ); };
char ActionIdentAllowed(const Player* player,const char* sid )
     { return ActionIdAllowed( player,  ActionIdByIdent(sid) ); };
char UpgradeIdentAllowed(const Player* player,const char* sid )
     { return UpgradeIdAllowed( player,  UpgradeIdByIdent(sid) ); };

//@}
