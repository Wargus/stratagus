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
/**@name unittype.c	-	The unit types. */
/*
**	(c) Copyright 1998-2000 by Lutz Sammer
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
#include <ctype.h>

#include "freecraft.h"
#include "video.h"
#include "tileset.h"
#include "map.h"
#include "sound_id.h"
#include "unitsound.h"
#include "unittype.h"
#include "player.h"
#include "missile.h"

#include "etlib/hash.h"

#include "myendian.h"

/*----------------------------------------------------------------------------
--	Variables
----------------------------------------------------------------------------*/

/**
**	Lookup table for unit-type names
*/
local hashtable(UnitType*,61) UnitTypeHash;

/**
**	W*rCr*ft number to internal unit-type name.
*/
local const char* UnitTypeWcNames[] = {
    "unit-footman",
    "unit-grunt",
    "unit-peasant",
    "unit-peon",
    "unit-ballista",
    "unit-catapult",
    "unit-knight",
    "unit-ogre",
    "unit-archer",
    "unit-axethrower",
    "unit-mage",
    "unit-death-knight",
    "unit-paladin",
    "unit-ogre-mage",
    "unit-dwarves",
    "unit-goblin-sappers",
    "unit-attack-peasant",
    "unit-attack-peon",
    "unit-ranger",
    "unit-berserker",
    "unit-alleria",
    "unit-teron-gorefiend",
    "unit-kurdan-and-sky'ree",
    "unit-dentarg",
    "unit-khadgar",
    "unit-grom-hellscream",
    "unit-human-oil-tanker",
    "unit-orc-oil-tanker",
    "unit-human-transport",
    "unit-orc-transport",
    "unit-elven-destroyer",
    "unit-troll-destroyer",
    "unit-battleship",
    "unit-ogre-juggernaught",
    "unit-nothing-22",
    "unit-deathwing",
    "unit-nothing-24",
    "unit-nothing-25",
    "unit-gnomish-submarine",
    "unit-giant-turtle",
    "unit-gnomish-flying-machine",
    "unit-goblin-zeppelin",
    "unit-gryphon-rider",
    "unit-dragon",
    "unit-turalyon",
    "unit-eye-of-kilrogg",
    "unit-danath",
    "unit-korgath-bladefist",
    "unit-nothing-30",
    "unit-cho'gall",
    "unit-lothar",
    "unit-gul'dan",
    "unit-uther-lightbringer",
    "unit-zuljin",
    "unit-nothing-36",
    "unit-skeleton",
    "unit-daemon",
    "unit-critter",
    "unit-farm",
    "unit-pig-farm",
    "unit-human-barracks",
    "unit-orc-barracks",
    "unit-church",
    "unit-altar-of-storms",
    "unit-human-watch-tower",
    "unit-orc-watch-tower",
    "unit-stables",
    "unit-ogre-mound",
    "unit-gnomish-inventor",
    "unit-goblin-alchemist",
    "unit-gryphon-aviary",
    "unit-dragon-roost",
    "unit-human-shipyard",
    "unit-orc-shipyard",
    "unit-town-hall",
    "unit-great-hall",
    "unit-elven-lumber-mill",
    "unit-troll-lumber-mill",
    "unit-human-foundry",
    "unit-orc-foundry",
    "unit-mage-tower",
    "unit-temple-of-the-damned",
    "unit-human-blacksmith",
    "unit-orc-blacksmith",
    "unit-human-refinery",
    "unit-orc-refinery",
    "unit-human-oil-platform",
    "unit-orc-oil-platform",
    "unit-keep",
    "unit-stronghold",
    "unit-castle",
    "unit-fortress",
    "unit-gold-mine",
    "unit-oil-patch",
    "unit-human-start-location",
    "unit-orc-start-location",
    "unit-human-guard-tower",
    "unit-orc-guard-tower",
    "unit-human-cannon-tower",
    "unit-orc-cannon-tower",
    "unit-circle-of-power",
    "unit-dark-portal",
    "unit-runestone",
    "unit-human-wall",
    "unit-orc-wall",
    "unit-dead-body",
    "unit-destroyed-1x1-place",
    "unit-destroyed-2x2-place",
    "unit-destroyed-3x3-place",
    "unit-destroyed-4x4-place",
    "unit-peasant-with-gold",
    "unit-peon-with-gold",
    "unit-peasant-with-wood",
    "unit-peon-with-wood",
    "unit-human-oil-tanker-full",
    "unit-orc-oil-tanker-full",
};

#ifdef DEBUG	// {

/**
**	Table unit-type enums -> string.
**
**	Used to build C tables.
*/
local const char* UnitTypeNames[] = {
    "Footman",
    "Grunt",
    "Peasant",
    "Peon",
    "Ballista",
    "Catapult",
    "Knight",
    "Ogre",
    "Archer",
    "Axethrower",
    "Mage",
    "DeathKnight",
    "Paladin",
    "OgreMage",
    "Dwarves",
    "GoblinSappers",
    "AttackPeasant",
    "AttackPeon",
    "Ranger",
    "Berserker",
    "Alleria",
    "TeronGorefiend",
    "KurdanAndSky_ree",
    "Dentarg",
    "Khadgar",
    "GnomHellscream",
    "TankerHuman",
    "TankerOrc",
    "TransportHuman",
    "TransportOrc",
    "ElvenDestroyer",
    "TrollDestroyer",
    "Battleship",
    "Juggernaught",
    "Nothing",
    "Deathwing",
    "Nothing1",
    "Nothing2",
    "GnomishSubmarine",
    "GiantTurtle",
    "GnomishFlyingMachine",
    "GoblinZeppelin",
    "GryphonRider",
    "Dragon",
    "Turalyon",
    "EyeOfKilrogg",
    "Danath",
    "KorgathBladefist",
    "Nothing3",
    "Cho_gall",
    "Lothar",
    "Gul_dan",
    "UtherLightbringer",
    "Zuljin",
    "Nothing4",
    "Skeleton",
    "Daemon",
    "Critter",
    "Farm",
    "PigFarm",
    "BarracksHuman",
    "BarracksOrc",
    "Church",
    "AltarOfStorms",
    "ScoutTowerHuman",
    "ScoutTowerOrc",
    "Stables",
    "OgreMound",
    "GnomishInventor",
    "GoblinAlchemist",
    "GryphonAviary",
    "DragonRoost",
    "ShipyardHuman",
    "ShipyardOrc",
    "TownHall",
    "GreatHall",
    "ElvenLumberMill",
    "TrollLumberMill",
    "FoundryHuman",
    "FoundryOrc",
    "MageTower",
    "TempleOfTheDamned",
    "BlacksmithHuman",
    "BlacksmithOrc",
    "RefineryHuman",
    "RefineryOrc",
    "OilPlatformHuman",
    "OilPlatformOrc",
    "Keep",
    "Stronghold",
    "Castle",
    "Fortress",
    "GoldMine",
    "OilPatch",
    "StartLocationHuman",
    "StartLocationOrc",
    "GuardTowerHuman",
    "GuardTowerOrc",
    "CannonTowerHuman",
    "CannonTowerOrc",
    "CircleofPower",
    "DarkPortal",
    "Runestone",
    "WallHuman",
    "WallOrc",
    "DeadBody",
    "Destroyed1x1Place",
    "Destroyed2x2Place",
    "Destroyed3x3Place",
    "Destroyed4x4Place",
    "PeasantWithGold",
    "PeonWithGold",
    "PeasantWithWood",
    "PeonWithWood",
    "TankerHumanFull",
    "TankerOrcFull",
    NULL
};

/*----------------------------------------------------------------------------
--	Functions
----------------------------------------------------------------------------*/

#ifndef USE_CCL2
extern char UnitCorpse[UnitTypeInternalMax];
#endif

/**
**	Generate C - table for UnitTypes.
*/
global void PrintUnitTypeTable(void)
{
    int i;
    UnitType* type;

    for( i=0; i<sizeof(UnitTypes)/sizeof(*UnitTypes); ++i ) {
	type=&UnitTypes[i];
	printf("\n{   UnitTypeType, \"%s\"",type->Ident);
	printf("\n    ,\"%s\"\n    ",type->Name);
	if( type->SameSprite ) {
	    printf(",\"%s\"",type->SameSprite);
	} else {
	    printf(",NULL");
	}
	printf(", {\n");
	if( type->File[0] ) {
	    printf("\t\"%s\"\n",type->File[0]);
	} else {
	    printf("\tDEFAULT\n");
	}
	if( type->File[1] ) {
	    printf("\t,\"%s\"\n",type->File[1]);
	} else {
	    printf("\t,DEFAULT\n");
	}
	if( type->File[2] ) {
	    printf("\t,\"%s\"\n",type->File[2]);
	} else {
	    printf("\t,DEFAULT\n");
	}
	if( type->File[3] ) {
	    printf("\t,\"%s\" }\n",type->File[3]);
	} else {
	    printf("\t,DEFAULT }\n");
	}

	printf("    ,%3d,%3d\t\t\t// graphic size\n"
		,type->Width,type->Height);

	printf("   ,_%sAnimations\t// animations\n",UnitTypeNames[i]);
	printf("   ,{ \"%s\" }\n",IdentOfIcon(type->Icon.Icon));

//	printf("   ,{ \"%s\" }\t\t// Missile\n"
//		,MissileTypes[type->MissileWeapon].Ident);
	printf("   ,{ \"%s\" }\t\t// Missile\n",type->Missile.Name);

//	switch( UnitCorpse[type->Type] ) {
	switch( type->Corpse ) {
	    case 0:
		printf("   ,CorpseNone\n");
		break;
	    case 1:
		printf("   ,CorpseHuman\n");
		break;
	    case 2:
		printf("   ,CorpseOrc\n");
		break;
	    case 3:
		printf("   ,CorpseShip\n");
		break;
	    case 4:
		printf("   ,CorpseLandSite\n");
		break;
	    case 5:
		printf("   ,CorpseWaterSite\n");
		break;
	}
	printf("\t//Speed\tOvFrame\tSightR\tHitpnt\tMagic\tBTime\tGold\tWood\tOil\n");
	printf("\t,%6d,%7d,%6d,%7d,%6d, {%5d,%6d,%7d,%6d }\n"
	    ,type->_Speed
	    ,type->OverlapFrame
	    ,type->_SightRange
	    ,type->_HitPoints
	    ,type->Magic
	    ,type->_Costs[TimeCost]
	    ,type->_Costs[GoldCost]
	    ,type->_Costs[WoodCost]
	    ,type->_Costs[OilCost]);
	printf("\t//TileW\tTileH\tBoxW\tBoxH\tAttack\tReactC\tReactH\n");
	printf("\t,%6d,%5d,%6d,%7d,%9d,%7d,%7d\n"
	    ,type->TileWidth
	    ,type->TileHeight
	    ,type->BoxWidth
	    ,type->BoxHeight
	    ,type->_AttackRange
	    ,type->ReactRangeComputer
	    ,type->ReactRangeHuman);

	printf("\t//Armor\tPrior\tDamage\tPierc\tWUpgr\tAUpgr\n");
	printf("\t,%6d,%5d,%8d,%6d,%7d,%7d\n"
	    ,type->_Armor
	    ,type->Priority
	    ,type->_BasicDamage
	    ,type->_PiercingDamage
	    ,type->WeaponsUpgradable
	    ,type->ArmorUpgradable);

	printf("\t//Type\tDecay\tAnnoy\tMouse\tPoints\n");
	printf("\t,%4d,%8d,%7d,%7d,%8d\n"
	    ,type->UnitType
	    ,type->DecayRate
	    ,type->AnnoyComputerFactor
	    ,type->MouseAction
	    ,type->Points);

	printf("\t//Targ\tLand\tAir\tSea\tExplode\tCritter\tBuild\tSubmarin\n");
	printf("\t,%5d,%5d,%6d,%7d,%11d,%7d,%5d,%11d\n"
	    ,type->CanTarget
	    ,type->LandUnit
	    ,type->AirUnit
	    ,type->SeaUnit
	    ,type->ExplodeWhenKilled
	    ,type->Critter
	    ,type->Building
	    ,type->Submarine);

	printf("\t//SeeSu\tCowerP\tTanker\tTrans\tGOil\tSOil\tVanish\tGrAtt\n");
	printf("\t,%6d,%6d,%7d,%6d,%6d,%7d,%9d,%8d\n"
	    ,type->CanSeeSubmarine
	    ,type->CowerPeon
	    ,type->Tanker
	    ,type->Transporter
	    ,type->GivesOil
	    ,type->StoresGold
	    ,type->Vanishes
	    ,type->GroundAttack);

	printf("\t//Udead\tShore\tSpell\tSWood\tCanAtt\tTower\tOilPtch\tGoldmine\n");
	printf("\t,%6d,%5d,%7d,%7d,%8d,%6d,%9d,%8d\n"
	    ,type->IsUndead
	    ,type->ShoreBuilding
	    ,type->CanCastSpell
	    ,type->StoresWood
	    ,type->CanAttack
	    ,type->Tower
	    ,type->OilPatch
	    ,type->GoldMine);

	printf("\t//Hero\tSOil\tExplode\tCowerM\tOrganic\tSelect\n");
	printf("\t,%5d,%5d,%10d,%6d,%8d,%6d\n"
	    ,type->Hero
	    ,type->StoresOil
	    ,type->Explodes
	    ,type->CowerMage
	    ,type->Organic
	    ,type->SelectableByRectangle);


#if 0
	if( !type->Buttons && 0 ) {
	    printf("   ,NULL\t\t// buttons\n");
	} else {
	    printf("   ,_%sButtons\t// buttons\n"
		,UnitTypeNames[i]);
	}
#endif

	printf("   ,{\t\t// sound\n");
	printf("\t { \"%s\" }\n",type->Sound.Selected.Name);
	printf("\t,{ \"%s\" }\n",type->Sound.Acknowledgement.Name);
	printf("\t,{ \"%s\" }\n",type->Sound.Ready.Name);
	printf("\t,{ \"%s\" }\n",type->Sound.Help.Name);
	printf("\t,{ \"%s\" }\n",type->Sound.Dead.Name);
#if 0
	printf("\t { \"%s-selected\" }\n",type->Ident);
	printf("\t,{ \"%s-acknowledge\" }\n",type->Ident);
	printf("\t,{ \"%s-ready\" }\n",type->Ident);
	printf("\t,{ \"%s-help\" }\n",type->Ident);
	printf("\t,{ \"%s-dead\" }\n",type->Ident);
	//FIXME: add here autogeneration of attack information (at least sound)
#endif
	printf("   },");
	printf("   {");
	printf("\t { \"%s\" }\n",type->Weapon.Attack.Name);
	printf("   }");
	printf(" },\n");
    }
    fflush(stdout);
}

#endif	// } DEBUG

/**
**	Update the player stats for changed unit types.
*/
global void UpdateStats(void)
{
    UnitType* type;
    UnitStats* stats;
    unsigned player;
    unsigned i;

    //
    //	Update players stats
    //
    for( type=UnitTypes;
	    type<&UnitTypes[sizeof(UnitTypes)/sizeof(*UnitTypes)]; ++type ) {
	for( player=0; player<PlayerMax; ++player ) {
	    stats=&type->Stats[player];
	    stats->AttackRange=type->_AttackRange;
	    stats->SightRange=type->_SightRange;
	    stats->Armor=type->_Armor;
	    stats->BasicDamage=type->_BasicDamage;
	    stats->PiercingDamage=type->_PiercingDamage;
	    stats->Speed=type->_Speed;
	    stats->HitPoints=type->_HitPoints;
	    for( i=0; i<MaxCosts; ++i ) {
		stats->Costs[i]=type->_Costs[i];
	    }
	    stats->Level=1;
	}
    }
}

#define ReadByte()   (*((unsigned char*)udta)++)

#if defined(__BYTE_ORDER) && __BYTE_ORDER == __BIG_ENDIAN
#define ReadWord()   bswap_16(*((unsigned short*)udta)++)
#define ReadLong()   bswap_32(*((unsigned long*)udta)++)
#else
#define ReadWord()   (*((unsigned short*)udta)++)
#define ReadLong()   (*((unsigned long*)udta)++)
#endif

/**
**	Parse UDTA area from puds.
**
**	@param udta	Pointer to udta area.
**	@param length	length of udta area.
*/
global void ParsePudUDTA(const char* udta,int length)
{
    int i;
    int v;
    const char* start;
    UnitType* unittype;

    // FIXME: not the fastest, remove UnitTypeByWcNum from loops!
    IfDebug(
	if( length!=5694 && length!=5948 ) {
	    DebugLevel0("\n"__FUNCTION__": ***\n"__FUNCTION__": %d\n",length);
	    DebugLevel0(__FUNCTION__": ***\n\n");
	}
    )
    start=udta;

    for( i=0; i<110; ++i ) {		// overlap frames
	unittype=UnitTypeByWcNum(i);
	v=ReadWord();
	unittype->OverlapFrame=v;
    }
    for( i=0; i<508; ++i ) {		// skip obselete data
	v=ReadWord();
    }
    for( i=0; i<110; ++i ) {		// sight range
	unittype=UnitTypeByWcNum(i);
	v=ReadLong();
	unittype->_SightRange=v;
    }
    for( i=0; i<110; ++i ) {		// hit points
	unittype=UnitTypeByWcNum(i);
	v=ReadWord();
	unittype->_HitPoints=v;
    }
    for( i=0; i<110; ++i ) {		// Flag if unit is magic
	unittype=UnitTypeByWcNum(i);
	v=ReadByte();
	unittype->Magic=v;
    }
    for( i=0; i<110; ++i ) {		// Build time * 6 = one second FRAMES
	unittype=UnitTypeByWcNum(i);
	v=ReadByte();
	unittype->_Costs[TimeCost]=v;
    }
    for( i=0; i<110; ++i ) {		// Gold cost / 10
	unittype=UnitTypeByWcNum(i);
	v=ReadByte();
	unittype->_Costs[GoldCost]=v*10;
    }
    for( i=0; i<110; ++i ) {		// Lumber cost / 10
	unittype=UnitTypeByWcNum(i);
	v=ReadByte();
	unittype->_Costs[WoodCost]=v*10;
    }
    for( i=0; i<110; ++i ) {		// Oil cost / 10
	unittype=UnitTypeByWcNum(i);
	v=ReadByte();
	unittype->_Costs[OilCost]=v*10;
    }
    for( i=0; i<110; ++i ) {		// Unit size in tiles
	unittype=UnitTypeByWcNum(i);
	v=ReadWord();
	unittype->TileWidth=v;
	v=ReadWord();
	unittype->TileHeight=v;
    }
    for( i=0; i<110; ++i ) {		// Box size in pixel
	unittype=UnitTypeByWcNum(i);
	v=ReadWord();
	unittype->BoxWidth=v;
	v=ReadWord();
	unittype->BoxHeight=v;
    }

    for( i=0; i<110; ++i ) {		// Attack range
	unittype=UnitTypeByWcNum(i);
	v=ReadByte();
	unittype->_AttackRange=v;
    }
    for( i=0; i<110; ++i ) {		// React range
	unittype=UnitTypeByWcNum(i);
	v=ReadByte();
	unittype->ReactRangeComputer=v;
    }
    for( i=0; i<110; ++i ) {		// React range
	unittype=UnitTypeByWcNum(i);
	v=ReadByte();
	unittype->ReactRangeHuman=v;
    }
    for( i=0; i<110; ++i ) {		// Armor
	unittype=UnitTypeByWcNum(i);
	v=ReadByte();
	unittype->_Armor=v;
    }
    for( i=0; i<110; ++i ) {		// Selectable via rectangle
	unittype=UnitTypeByWcNum(i);
	v=ReadByte();
	unittype->SelectableByRectangle=v!=0;
    }
    for( i=0; i<110; ++i ) {		// Priority
	unittype=UnitTypeByWcNum(i);
	v=ReadByte();
	unittype->Priority=v;
    }
    for( i=0; i<110; ++i ) {		// Basic damage
	unittype=UnitTypeByWcNum(i);
	v=ReadByte();
	unittype->_BasicDamage=v;
    }
    for( i=0; i<110; ++i ) {		// Piercing damage
	unittype=UnitTypeByWcNum(i);
	v=ReadByte();
	unittype->_PiercingDamage=v;
    }
    for( i=0; i<110; ++i ) {		// Weapons upgradable
	unittype=UnitTypeByWcNum(i);
	v=ReadByte();
	unittype->WeaponsUpgradable=v;
    }
    for( i=0; i<110; ++i ) {		// Armor upgradable
	unittype=UnitTypeByWcNum(i);
	v=ReadByte();
	unittype->ArmorUpgradable=v;
    }
    for( i=0; i<110; ++i ) {		// Missile Weapon
	unittype=UnitTypeByWcNum(i);
	v=ReadByte();
	//unittype->MissileWeapon=v;
	unittype->Missile.Name=MissileTypes[v].Ident;
	if( unittype->Missile.Missile ) abort();
	// FIXME: convert wc weapon number to internal name
    }
    for( i=0; i<110; ++i ) {		// Unit type
	unittype=UnitTypeByWcNum(i);
	v=ReadByte();
	unittype->UnitType=v;
    }
    for( i=0; i<110; ++i ) {		// Decay rate * 6 = secs
	unittype=UnitTypeByWcNum(i);
	v=ReadByte();
	unittype->DecayRate=v;
    }
    for( i=0; i<110; ++i ) {		// Annoy computer factor
	unittype=UnitTypeByWcNum(i);
	v=ReadByte();
	unittype->AnnoyComputerFactor=v;
    }
    for( i=0; i<58; ++i ) {		// 2nd mouse button action
	unittype=UnitTypeByWcNum(i);
	v=ReadByte();
	unittype->MouseAction=v;
    }
    for( ; i<110; ++i ) {		// 2nd mouse button action
	unittype=UnitTypeByWcNum(i);
	unittype->MouseAction=0;
    }
    for( i=0; i<110; ++i ) {		// Point value for killing unit
	unittype=UnitTypeByWcNum(i);
	v=ReadWord();
	unittype->Points=v;
    }
    for( i=0; i<110; ++i ) {		// Can target (1 land, 2 sea, 4 air)
	unittype=UnitTypeByWcNum(i);
	v=ReadByte();
	unittype->CanTarget=v;
    }

    for( i=0; i<110; ++i ) {		// Flags
	unittype=UnitTypeByWcNum(i);
	v=ReadLong();
	// unittype->Flags=v;
#define BIT(b,v)	(((v>>b))&1)
	unittype->LandUnit=BIT(0,v);
	unittype->AirUnit=BIT(1,v);
	unittype->ExplodeWhenKilled=BIT(2,v);
	unittype->SeaUnit=BIT(3,v);
	unittype->Critter=BIT(4,v);
	unittype->Building=BIT(5,v);
	unittype->Submarine=BIT(6,v);
	unittype->CanSeeSubmarine=BIT(7,v);
	unittype->CowerPeon=BIT(8,v);
	unittype->Tanker=BIT(9,v);
	unittype->Transporter=BIT(10,v);
	unittype->GivesOil=BIT(11,v);
	unittype->StoresGold=BIT(12,v);
	unittype->Vanishes=BIT(13,v);
	unittype->GroundAttack=BIT(14,v);
	unittype->IsUndead=BIT(15,v);
	unittype->ShoreBuilding=BIT(16,v);
	unittype->CanCastSpell=BIT(17,v);
	unittype->StoresWood=BIT(18,v);
	unittype->CanAttack=BIT(19,v);
	unittype->Tower=BIT(20,v);
	unittype->OilPatch=BIT(21,v);
	unittype->GoldMine=BIT(22,v);
	unittype->Hero=BIT(23,v);
	unittype->StoresOil=BIT(24,v);
	unittype->Explodes=BIT(25,v);
	UnitTypes[i].CowerMage=BIT(26,v);
	UnitTypes[i].Organic=BIT(27,v);
	if( BIT(28,v) )	DebugLevel0("Unused bit 28 used in %d\n",i);
	if( BIT(29,v) )	DebugLevel0("Unused bit 29 used in %d\n",i);
	if( BIT(30,v) )	DebugLevel0("Unused bit 30 used in %d\n",i);
	if( BIT(31,v) )	DebugLevel0("Unused bit 31 used in %d\n",i);
#undef BIT
    }

    // FIXME: peon applies also to peon-with-gold and peon-with-wood
    // FIXME: oil-tanker applies also to oil-tanker-full

    DebugLevel0("\tUDTA used %Zd bytes\n",udta-start);

    UpdateStats();
}

/**
**	Save state of an unit-type to file.
**
**	@param file	Output file.
*/
global void SaveUnitType(const UnitType* type,FILE* file)
{
    int i;

    fprintf(file,"(define-unit-type \"%s\"",type->Ident);
    if( strlen(type->Ident)<12 ) {
	fputs("\t\t",file);
    } else if( strlen(type->Ident)<20 ) {
	fputc('\t',file);
    }
    fprintf(file,"\t\"%s\"\n",type->Name);
    fputs("  ;; graphic data\n  ",file);
    if( type->SameSprite ) {
	fprintf(file,"\"%s\"",type->SameSprite);
    } else {
	fprintf(file,"#(");
	for( i=0; i<4; ++i ) {
	    if( i ) {
		fputs("\n    ",file);
	    }
	    if( type->File[i] ) {
		fprintf(file,"\"%s\"",type->File[i]);
	    } else {
		fprintf(file,"()");
	    }
	}
	fprintf(file," )");
    }
    fprintf(file,"\n  '( %3d %3d )\t\t\t;; graphic size\n"
		,type->Width,type->Height);
    fprintf(file,"  \"animations-%s\"\t;; animations\n",type->Ident+5);
    fprintf(file,"  \"%s\"\n",IdentOfIcon(type->Icon.Icon));

    fprintf(file,"  ;;Speed OvFrame SightR Hitpnt Magic  BTime\tGold\tWood\tOil\n");
    fprintf(file,"  %6d %7d %6d %6d %6d #(%5d %6d %7d %6d )\n"
	,type->_Speed
	,type->OverlapFrame
	,type->_SightRange
	,type->_HitPoints
	,type->Magic
	,type->_Costs[TimeCost]
	,type->_Costs[GoldCost]
	,type->_Costs[WoodCost]
	,type->_Costs[OilCost]);
    fprintf(file,"  ;;Tile    Box Size    Attack\tReactC\tReactH\n");
    fprintf(file,"  '( %d %d ) '( %3d %3d ) %5d %7d %7d\n"
	,type->TileWidth
	,type->TileHeight
	,type->BoxWidth
	,type->BoxHeight
	,type->_AttackRange
	,type->ReactRangeComputer
	,type->ReactRangeHuman);

    fprintf(file,"  ;;Armor Prior\tDamage\tPierc\tWUpgr\tAUpgr\n");
    fprintf(file,"  %6d %5d %6d %6d %7d %7d\n"
	,type->_Armor
	,type->Priority
	,type->_BasicDamage
	,type->_PiercingDamage
	,type->WeaponsUpgradable
	,type->ArmorUpgradable);

    fprintf(file,"  ;;Decay Annoy\tPoints\n");
    fprintf(file,"  %5d %6d %7d\n"
	,type->DecayRate
	,type->AnnoyComputerFactor
	,type->Points);

    //fprintf(file,"  \"%s\"\n",MissileTypes[type->MissileWeapon].Ident);
    fprintf(file,"  \"%s\"\n",type->Missile.Name);
    fprintf(file,"  ");
    switch( type->Corpse ) {
	case 0:
	    fprintf(file,"'corpse-none");
	    break;
	case 1:
	    fprintf(file,"'corpse-human");
	    break;
	case 2:
	    fprintf(file,"'corpse-orc");
	    break;
	case 3:
	    fprintf(file,"'corpse-ship");
	    break;
	case 4:
	    fprintf(file,"'corpse-land-site");
	    break;
	case 5:
	    fprintf(file,"'corpse-water-site");
	    break;
    }
    fprintf(file,"\n");

    fprintf(file,"  ");
    switch( type->UnitType ) {
	case UnitTypeLand:
	    fprintf(file,"'type-land");
	    break;
	case UnitTypeFly:
	    fprintf(file,"'type-fly");
	    break;
	case UnitTypeNaval:
	    fprintf(file,"'type-naval");
	    break;
	default:
	    fprintf(file,"'type-unknown");
	    break;
    }
    fprintf(file,"\n");

    fprintf(file,"  ");
    switch( type->MouseAction ) {
	case MouseActionNone:
	    fprintf(file,"'right-none");
	    break;
	case MouseActionAttack:
	    fprintf(file,"'right-attack");
	    break;
	case MouseActionMove:
	    fprintf(file,"'right-move");
	    break;
	case MouseActionHarvest:
	    fprintf(file,"'right-harvest");
	    break;
	case MouseActionHaulOil:
	    fprintf(file,"'right-haul-oil");
	    break;
	case MouseActionDemolish:
	    fprintf(file,"'right-demolish");
	    break;
	case MouseActionSail:
	    fprintf(file,"'right-sail");
	    break;
	default:
	    fprintf(file,"'right-unknown");
	    break;
    }
    fprintf(file,"\n");

    if( type->CanTarget ) {
	fprintf(file,"  ");
	if( type->CanTarget&CanTargetLand ) {
	    fprintf(file,"'can-target-land ");
	}
	if( type->CanTarget&CanTargetSea ) {
	    fprintf(file,"'can-target-sea ");
	}
	if( type->CanTarget&CanTargetAir ) {
	    fprintf(file,"'can-target-air ");
	}
	if( type->CanTarget&~7 ) {
	    fprintf(file,"'can-target-other ");
	}
	fprintf(file,"\n");
    }

    fprintf(file,"  ;; flags\n");
    if( type->LandUnit ) {
	fprintf(file,"  'land-unit\n");
    }
    if( type->AirUnit ) {
	fprintf(file,"  'air-unit\n");
    }
    if( type->SeaUnit ) {
	fprintf(file,"  'sea-unit\n");
    }
    if( type->ExplodeWhenKilled ) {
	fprintf(file,"  'explode-when-killed\n");
    }
    if( type->Critter ) {
	fprintf(file,"  'critter\n");
    }
    if( type->Building ) {
	fprintf(file,"  'building\n");
    }
    if( type->Submarine ) {
	fprintf(file,"  'submarine\n");
    }
    if( type->CanSeeSubmarine ) {
	fprintf(file,"  'can-see-submarine\n");
    }
    if( type->CowerPeon ) {
	fprintf(file,"  'cower-peon\n");
    }
    if( type->Tanker ) {
	fprintf(file,"  'tanker\n");
    }
    if( type->Transporter ) {
	fprintf(file,"  'transporter\n");
    }
    if( type->GivesOil ) {
	fprintf(file,"  'gives-oil\n");
    }
    if( type->StoresGold ) {
	fprintf(file,"  'stores-gold\n");
    }
    if( type->Vanishes ) {
	fprintf(file,"  'vanishes\n");
    }
    if( type->GroundAttack ) {
	fprintf(file,"  'can-ground-attack\n");
    }
    if( type->IsUndead ) {
	fprintf(file,"  'isundead\n");
    }
    if( type->ShoreBuilding ) {
	fprintf(file,"  'shore-building\n");
    }
    if( type->CanCastSpell ) {
	fprintf(file,"  'can-cast-spell\n");
    }
    if( type->StoresWood ) {
	fprintf(file,"  'stores-wood\n");
    }
    if( type->CanAttack ) {
	fprintf(file,"  'can-attack\n");
    }
    if( type->Tower ) {
	fprintf(file,"  'tower\n");
    }
    if( type->OilPatch ) {
	fprintf(file,"  'oil-patch\n");
    }
    if( type->GoldMine ) {
	fprintf(file,"  'gives-gold\n");
    }
    if( type->Hero ) {
	fprintf(file,"  'hero\n");
    }
    if( type->StoresOil ) {
	fprintf(file,"  'stores-oil\n");
    }
    if( type->Explodes ) {
	fprintf(file,"  'volatile\n");
    }
    if( type->CowerMage ) {
	fprintf(file,"  'cower-mage\n");
    }
    if( type->Organic ) {
	fprintf(file,"  'organic\n");
    }
    if( type->SelectableByRectangle ) {
	fprintf(file,"  'selectable-by-rectangle\n");
    }

    fprintf(file,"  ;; sounds\n");
    fprintf(file,"  #(\"%s\"\n",type->Sound.Selected.Name);
    fprintf(file,"    \"%s\"\n",type->Sound.Acknowledgement.Name);
    fprintf(file,"    \"%s\"\n",type->Sound.Ready.Name);
    fprintf(file,"    \"%s\"\n",type->Sound.Help.Name);
    fprintf(file,"    \"%s\" )\n",type->Sound.Dead.Name);
    fprintf(file,"  \"%s\" )\n",type->Weapon.Attack.Name);
}

/**
**	Save state of the unit-type table to file.
**
**	@param file	Output file.
*/
global void SaveUnitTypes(FILE* file)
{
    int i;
    const UnitType* type;

    fprintf(file,"\n;;; -----------------------------------------\n");
    fprintf(file,";;; MODULE: unittypes $Id$\n");

    // Save all types

    for( i=0; i<sizeof(UnitTypes)/sizeof(*UnitTypes); ++i ) {
	type=&UnitTypes[i];
	fputc('\n',file);
	SaveUnitType(type,file);
    }
}

/**
**	Find unit-type by identifier.
**
**	@param ident	The unit-type identifier.
**	@return		Unit-type pointer.
*/
global UnitType* UnitTypeByIdent(const char* ident)
{
    UnitType** type;

    type=(UnitType**)hash_find(UnitTypeHash,(char*)ident);
    if( type ) {
	return *type;
    }

    DebugLevel0(__FUNCTION__": Name %s not found\n",ident);

    return NULL;
}

/**
**	Find unit-type by wc number.
**
**	@param num	The unit-type number used in f.e. puds.
**	@return		Unit-type pointer.
*/
global UnitType* UnitTypeByWcNum(unsigned num)
{
    return UnitTypeByIdent(UnitTypeWcNames[num]);
}

/**
**	Init unit types.
*/
global void InitUnitTypes(void)
{
    unsigned type;

    if( UnitTypes[2].Type==2 ) {	// FIXME: trick 17, double entered
	return;
    }

    for( type=0; type<sizeof(UnitTypes)/sizeof(*UnitTypes); ++type ) {
	//
	//	Initialize:
	//
	UnitTypes[type].Type=type;
	//
	//	Add idents to hash.
	//
	*(UnitType**)hash_add(UnitTypeHash,UnitTypes[type].Ident)
		=&UnitTypes[type];
    }
}

/**
**	Load the graphics for the units.
*/
global void LoadUnitSprites(void)
{
    UnitType* unittype;
    unsigned type;
    const char* file;

    for( type=0; type<sizeof(UnitTypes)/sizeof(*UnitTypes); ++type ) {
	//
	//	Unit-type uses the same sprite as an other.
	//
	if( UnitTypes[type].SameSprite ) {
	    continue;
	}

	file=UnitTypes[type].File[TheMap.Terrain];
	if( !file ) {			// default one
	    file=UnitTypes[type].File[0];
	}
	if( file ) {
	    char* buf;

	    buf=alloca(strlen(file)+9+1);
	    file=strcat(strcpy(buf,"graphic/"),file);
	    ShowLoadProgress("\tUnit `%s'\n",file);
	    UnitTypes[type].RleSprite=LoadRleSprite(file
		    ,UnitTypes[type].Width,UnitTypes[type].Height);
	}
    }

    for( type=0; type<sizeof(UnitTypes)/sizeof(*UnitTypes); ++type ) {
	//
	//	Unit-type uses the same sprite as an other.
	//
	if( UnitTypes[type].SameSprite ) {
	    unittype=UnitTypeByIdent(UnitTypes[type].SameSprite);
	    if( !unittype ) {
		fprintf(stderr,__FUNCTION__": unit-type %s not found\n"
			,UnitTypes[type].SameSprite);
		exit(-1);
	    }
	    UnitTypes[type].RleSprite=unittype->RleSprite;
	}

	//
	//	Lookup icons.
	//
	UnitTypes[type].Icon.Icon=IconByIdent(UnitTypes[type].Icon.Name);
	//
	//	Lookup missiles.
	//
	UnitTypes[type].Missile.Missile=MissileTypeByIdent(
		UnitTypes[type].Missile.Name);
    }

    // FIXME: must copy unit data from peon/peasant to with gold/wood
    // FIXME: must copy unit data from tanker to tanker full
}

/**
**	Draw unit-type on map.
**
**	@param type	Unit-type pointer.
**	@param frame	Animation frame of unit-type.
**	@param x	Display X postion to draw unit-type.
**	@param Y	Display Y postion to draw unit-type.
*/
global void DrawUnitType(const UnitType* type,unsigned frame,int x,int y)
{
    DebugLevel3("%s\n",type->Name);

    // FIXME: move this calculation to high level.
    x-=(type->Width-type->TileWidth*TileSizeX)/2;
    y-=(type->Height-type->TileHeight*TileSizeY)/2;

    // FIXME: This is a hack for mirrored sprites
    if( frame&128 ) {
	DrawRleSpriteClippedX(type->RleSprite,frame&127,x,y);
    } else {
	DrawRleSpriteClipped(type->RleSprite,frame,x,y);
    }
}

//@}
