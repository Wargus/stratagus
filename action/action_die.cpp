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
/**@name action_die.c	-	The die action. */
/*
**	(c) Copyright 1998,2000 by Lutz Sammer
**
**	$Id$
*/

//@{

#include <stdio.h>
#include <stdlib.h>

#include "freecraft.h"
#include "video.h"
#include "sound_id.h"
#include "unitsound.h"
#include "unittype.h"
#include "player.h"
#include "unit.h"
#include "actions.h"

#ifndef USE_CCL2
/*----------------------------------------------------------------------------
--	Die
----------------------------------------------------------------------------*/

///	Footman,Grunt,Grom Hellscream,Danath,Korgath Bladefist
local Animation GruntDie[] = {
    { 0, 0, 3, 45},{ 0, 0, 3,  5},{ 0, 0,100,  5},{ 3, 0, 1,  0}
};

///	Peon, Peasant, Attacking Peon, Attacking Peasant.
local Animation PeonDie[] = {
    { 0, 0, 3, 50},{ 0, 0, 3,  5},{ 0, 0,100,  5},{ 3, 0, 1,  0}
};

///	Knight, Paladin, Turalyon, Lothar, Uther Lightbringer
local Animation KnightDie[] = {
    { 0, 0, 3, 45},{ 0, 0, 3,  5},{ 0, 0,100, 5},{ 0, 0,200, 5},
    { 0, 0,200, 5},{ 3, 0, 1,  0}
};

///	Ogre, Ogre-mage, Dentarg, Cho'gall
local Animation OgreDie[] = {
    { 0, 0, 3, 45},{ 0, 0, 3,  5},{ 0, 0,100, 5},{ 0, 0,200, 5},
    { 0, 0,200, 5},{ 3, 0, 1,  0}
};

///	Archer, Ranger, Alleria
local Animation ArcherDie[] = {
    { 0, 0, 3, 35},{ 0, 0, 3,  5},{ 0, 0,100,  5},{ 3, 0, 1,  0}
};

///	Axethrower, Berserker, Zuljin
local Animation AxethrowerDie[] = {
    { 0, 0, 3, 45},{ 0, 0, 3,  5},{ 0, 0,100,  5},{ 3, 0, 1,  0}
};

///	Mage, Khadar
local Animation MageDie[] = {
    { 0, 0, 5, 45},{ 0, 0, 5,  5},{ 0, 0, 5,  5},{ 0, 0, 5,  5},
    { 0, 0, 5,  5},{ 0, 0, 5,  5},{ 0, 0, 5,  5},{ 3, 0, 1,  0}
};

///	Death Knight, Teron Gorefiend, Gul'dan
local Animation DeathKnightDie[] = {
    { 0, 0, 5, 45},{ 0, 0, 5,  5},{ 0, 0, 5,  5},{ 0, 0, 5,  5},
    { 3, 0, 1,  0}
};

///	Dwarves
local Animation DwarvesDie[] = {
    { 0, 0, 3,  5},{ 0, 0, 3, 15},{ 0, 0, 3, 15},{ 0, 0, 3, 15},
    { 0, 0, 3, 10},{ 3, 0, 1,  0}
};

///	Goblin Sappers
local Animation GoblinSappersDie[] = {
    { 0, 0, 3,  5},{ 0, 0, 3, 15},{ 0, 0, 3, 15},{ 0, 0, 3, 15},
    { 0, 0, 3, 10},{ 0, 0, 3, 10},{ 3, 0, 1,  0}
};

///	Gryphon Rider, Kurdan and Sky'ree:
local Animation GryphonRiderDie[] = {
    { 0, 0, 5, 35},{ 0, 0, 5,  5},{ 0, 0, 5,  5},{ 0, 0, 5,  5},
    { 0, 0, 5,  5},{ 0, 0, 5,  5},{ 3, 0, 1,  0}
};

///	Dragon, Deathwing
local Animation DragonDie[] = {
    { 0, 0, 5, 25},{ 0, 0, 5,  5},{ 0, 0, 5,  5},{ 0, 0, 5,  5},
    { 0, 0, 5,  5},{ 3, 0, 1,  0}
};

///	Human tanker, orc tanker:
local Animation TankerDie[] = {
    { 0, 0,50,  5},{ 0, 0,50,  5},{ 3, 0, 1,  0}
};

///	Human transporter, orc transporter:
local Animation TransportDie[] = {
    { 0, 0,50,  5},{ 0, 0,50,  5},{ 3, 0, 1,  0}
};

///	Elven destroyer, Troll destroyer:
local Animation DestroyerDie[] = {
    { 0, 0,50,  5},{ 0, 0,50,  5},{ 3, 0, 1,  0}
};

///	Battleship, Juggernaught
local Animation BattleshipDie[] = {
    { 0, 0,50,  5},{ 0, 0,50,  5},{ 3, 0, 1,  0}
};

///	Gnomish submarine, giant turtle
local Animation SubmarineDie[] = {
    { 3, 0, 1,  0}
};

///	Gnomish flying machine
local Animation GnomishFlyingMachineDie[] = {
    { 3, 0, 1,  0}
};

///	Critter
local Animation CritterDie[] = {
    { 0, 0,200,  5},{ 3, 0, 1,  0}
};

///	Skeleton
local Animation SkeletonDie[] = {
    { 0, 0, 3,  5},{ 0, 0, 3, 15},{ 0, 0, 3, 15},{ 0, 0, 3, 15},
    { 0, 0, 3, 15},{ 3, 0, 1,  0}
};

///	Daemon
local Animation DaemonDie[] = {
    { 0, 0, 5, 50},{ 0, 0, 5,  5},{ 0, 0, 5,  5},{ 0, 0, 5,  5},
    { 3, 0, 1,  0}
};

///	Corpse:		Orcish
local Animation CorpseOrcishDie[] = {
    {0, 0,200, 5}, {0, 0,200,  5}, {0, 0,200, 5}, {0, 0,200, 5},
    {0, 0,200, 5}, {3, 0,  1,-25}
};

///	Corpse:		Human
local Animation CorpseHumanDie[] = {
    {0, 0,200, 0}, {0, 0,200, 10}, {0, 0,200, 5}, {0, 0,200, 5},
    {0, 0,200, 5}, {3, 0,  1,-25}
};

///	Corpse:		Ships
local Animation CorpseShipsDie[] = {
    {0, 0,100,30}, {0, 0,100, 0}, {3, 0,  1,  0}
};

///	Destroyed site:
local Animation DestroyedSiteDie[] = {
    {0, 0,200,0}, {0, 0,200,1}, {3, 0,  1,0}
};

///	Destroyed water site:
local Animation DestroyedWaterSiteDie[] = {
    {0, 0,200,2}, {0, 0,200,1}, {3, 0,  1,0}
};

/**
**	The animation for a unit die.
**	FIXME: move this to unit-type. CCL configurable
*/
local Animation* UnitDie[UnitTypeInternalMax] = {
    GruntDie,			// UnitFootman
    GruntDie,			// UnitGrunt
    PeonDie,			// UnitPeasant
    PeonDie,			// UnitPeon
    0,				// UnitBallista
    0,				// UnitCatapult
    KnightDie,			// UnitKnight
    OgreDie,			// UnitOgre
    ArcherDie,			// UnitArcher
    AxethrowerDie,		// UnitAxethrower
    MageDie,			// UnitMage
    DeathKnightDie,		// UnitDeathKnight
    KnightDie,			// UnitPaladin
    OgreDie,			// UnitOgreMage
    DwarvesDie,			// UnitDwarves
    GoblinSappersDie,		// UnitGoblinSappers
    PeonDie,			// UnitAttackPeasant
    PeonDie,			// UnitAttackPeon
    ArcherDie,			// UnitRanger
    AxethrowerDie,		// UnitBerserker
    ArcherDie,			// UnitAlleria
    DeathKnightDie,		// UnitTeronGorefiend
    GryphonRiderDie,		// UnitKurdanAndSky_ree
    OgreDie,			// UnitDentarg
    MageDie,			// UnitKhadgar
    GruntDie,			// UnitGromHellscream
    TankerDie,			// UnitTankerHuman
    TankerDie,			// UnitTankerOrc
    TransportDie,		// UnitTransportHuman
    TransportDie,		// UnitTransportOrc
    DestroyerDie,		// UnitElvenDestroyer
    DestroyerDie,		// UnitTrollDestroyer
    BattleshipDie,		// UnitBattleship
    BattleshipDie,		// UnitJuggernaught
    0,				// UnitNothing
    DragonDie,			// UnitDeathwing
    0,				// UnitNothing1
    0,				// UnitNothing2
    SubmarineDie,		// UnitGnomishSubmarine
    SubmarineDie,		// UnitGiantTurtle
    GnomishFlyingMachineDie,	// UnitGnomishFlyingMachine
    0,				// UnitGoblinZeppelin
    GryphonRiderDie,		// UnitGryphonRider
    DragonDie,			// UnitDragon
    KnightDie,			// UnitTuralyon
    0,				// UnitEyeOfKilrogg
    GruntDie,			// UnitDanath
    GruntDie,			// UnitKorgathBladefist
    0,				// UnitNothing3
    OgreDie,			// UnitCho_gall
    KnightDie,			// UnitLothar
    DeathKnightDie,		// UnitGul_dan
    KnightDie,			// UnitUtherLightbringer
    AxethrowerDie,		// UnitZuljin
    0,				// UnitNothing4
    SkeletonDie,		// UnitSkeleton
    DaemonDie,			// UnitDaemon
    CritterDie,			// UnitCritter
    0,				// UnitFarm
    0,				// UnitPigFarm
    0,				// UnitBarracksHuman
    0,				// UnitBarracksOrc
    0,				// UnitChurch
    0,				// UnitAltarOfStorms
    0,				// UnitScoutTowerHuman
    0,				// UnitScoutTowerOrc
    0,				// UnitStables
    0,				// UnitOgreMound
    0,				// UnitGnomishInventor
    0,				// UnitGoblinAlchemist
    0,				// UnitGryphonAviary
    0,				// UnitDragonRoost
    0,				// UnitShipyardHuman
    0,				// UnitShipyardOrc
    0,				// UnitTownHall
    0,				// UnitGreatHall
    0,				// UnitElvenLumberMill
    0,				// UnitTrollLumberMill
    0,				// UnitFoundryHuman
    0,				// UnitFoundryOrc
    0,				// UnitMageTower
    0,				// UnitTempleOfTheDamned
    0,				// UnitBlacksmithHuman
    0,				// UnitBlacksmithOrc
    0,				// UnitRefineryHuman
    0,				// UnitRefineryOrc
    0,				// UnitOilPlatformHuman
    0,				// UnitOilPlatformOrc
    0,				// UnitKeep
    0,				// UnitStronghold
    0,				// UnitCastle
    0,				// UnitFortress
    0,				// UnitGoldMine
    0,				// UnitOilPatch
    0,				// UnitStartLocationHuman
    0,				// UnitStartLocationOrc
    0,				// UnitGuardTowerHuman
    0,				// UnitGuardTowerOrc
    0,				// UnitCannonTowerHuman
    0,				// UnitCannonTowerOrc
    0,				// UnitCircleofPower
    0,				// UnitDarkPortal
    0,				// UnitRunestone
    0,				// UnitWallHuman
    0,				// UnitWallOrc
    0,				// UnitDeadBody
    0,				// Unit1x1DestroyedPlace
    0,				// Unit2x2DestroyedPlace
    0,				// Unit3x3DestroyedPlace
    0,				// Unit4x4DestroyedPlace
    0,				// UnitPeasantWithGold
    0,				// UnitPeonWithGold
    0,				// UnitPeasantWithWood
    0,				// UnitPeonWithWood
    TankerDie,			// UnitTankerHumanFull
    TankerDie,			// UnitTankerOrcFull
};

/**
**	The corpse type. FIXME: move this to unit-type. CCL configurable
*/
global char UnitCorpse[UnitTypeInternalMax] = {
    CorpseHuman,		// UnitFootman
    CorpseOrc,			// UnitGrunt
    CorpseHuman,		// UnitPeasant
    CorpseOrc,			// UnitPeon
    CorpseNone,			// UnitBallista
    CorpseNone,			// UnitCatapult
    CorpseHuman,		// UnitKnight
    CorpseOrc,			// UnitOgre
    CorpseHuman,		// UnitArcher
    CorpseOrc,			// UnitAxethrower
    CorpseNone,			// UnitMage
    CorpseNone,			// UnitDeathKnight
    CorpseHuman,		// UnitPaladin
    CorpseOrc,			// UnitOgreMage
    CorpseNone,			// UnitDwarves
    CorpseNone,			// UnitGoblinSappers
    CorpseHuman,		// UnitAttackPeasant
    CorpseOrc,			// UnitAttackPeon
    CorpseHuman,		// UnitRanger
    CorpseOrc,			// UnitBerserker
    CorpseHuman,		// UnitAlleria
    CorpseOrc,			// UnitTeronGorefiend
    CorpseNone,			// UnitKurdanAndSky_ree
    CorpseOrc,			// UnitDentarg
    CorpseNone,			// UnitKhadgar
    CorpseHuman,		// UnitGromHellscream
    CorpseShip,			// UnitTankerHuman
    CorpseShip,			// UnitTankerOrc
    CorpseShip,			// UnitTransportHuman
    CorpseShip,			// UnitTransportOrc
    CorpseShip,			// UnitElvenDestroyer
    CorpseShip,			// UnitTrollDestroyer
    CorpseShip,			// UnitBattleship
    CorpseShip,			// UnitJuggernaught
    CorpseNone,			// UnitNothing
    CorpseNone,			// UnitDeathwing
    CorpseNone,			// UnitNothing1
    CorpseNone,			// UnitNothing2
    CorpseShip,			// UnitGnomishSubmarine
    CorpseShip,			// UnitGiantTurtle
    CorpseNone,			// UnitGnomishFlyingMachine
    CorpseNone,			// UnitGoblinZeppelin
    CorpseNone,			// UnitGryphonRider
    CorpseNone,			// UnitDragon
    CorpseHuman,		// UnitTuralyon
    CorpseNone,			// UnitEyeOfKilrogg
    CorpseHuman,		// UnitDanath
    CorpseHuman,		// UnitKorgathBladefist
    CorpseNone,			// UnitNothing3
    CorpseOrc,			// UnitCho_gall
    CorpseHuman,		// UnitLothar
    CorpseNone,			// UnitGul_dan
    CorpseHuman,		// UnitUtherLightbringer
    CorpseOrc,			// UnitZuljin
    CorpseNone,			// UnitNothing4
    CorpseNone,			// UnitSkeleton
    CorpseNone,			// UnitDaemon
    CorpseNone,			// UnitCritter
    CorpseLandSite,		// UnitFarm
    CorpseLandSite,		// UnitPigFarm
    CorpseLandSite,		// UnitBarracksHuman
    CorpseLandSite,		// UnitBarracksOrc
    CorpseLandSite,		// UnitChurch
    CorpseLandSite,		// UnitAltarOfStorms
    CorpseLandSite,		// UnitScoutTowerHuman
    CorpseLandSite,		// UnitScoutTowerOrc
    CorpseLandSite,		// UnitStables
    CorpseLandSite,		// UnitOgreMound
    CorpseLandSite,		// UnitGnomishInventor
    CorpseLandSite,		// UnitGoblinAlchemist
    CorpseLandSite,		// UnitGryphonAviary
    CorpseLandSite,		// UnitDragonRoost
    CorpseWaterSite,		// UnitShipyardHuman
    CorpseWaterSite,		// UnitShipyardOrc
    CorpseLandSite,		// UnitTownHall
    CorpseLandSite,		// UnitGreatHall
    CorpseLandSite,		// UnitElvenLumberMill
    CorpseLandSite,		// UnitTrollLumberMill
    CorpseWaterSite,		// UnitFoundryHuman
    CorpseWaterSite,		// UnitFoundryOrc
    CorpseLandSite,		// UnitMageTower
    CorpseLandSite,		// UnitTempleOfTheDamned
    CorpseLandSite,		// UnitBlacksmithHuman
    CorpseLandSite,		// UnitBlacksmithOrc
    CorpseWaterSite,		// UnitRefineryHuman
    CorpseWaterSite,		// UnitRefineryOrc
    CorpseWaterSite,		// UnitOilPlatformHuman
    CorpseWaterSite,		// UnitOilPlatformOrc
    CorpseLandSite,		// UnitKeep
    CorpseLandSite,		// UnitStronghold
    CorpseLandSite,		// UnitCastle
    CorpseLandSite,		// UnitFortress
    CorpseLandSite,		// UnitGoldMine
    CorpseLandSite,		// UnitOilPatch
    0,				// UnitStartLocationHuman
    0,				// UnitStartLocationOrc
    CorpseLandSite,		// UnitGuardTowerHuman
    CorpseLandSite,		// UnitGuardTowerOrc
    CorpseLandSite,		// UnitCannonTowerHuman
    CorpseLandSite,		// UnitCannonTowerOrc
    CorpseLandSite,		// UnitCircleofPower
    CorpseLandSite,		// UnitDarkPortal
    CorpseLandSite,		// UnitRunestone
    CorpseLandSite,		// UnitWallHuman
    CorpseLandSite,		// UnitWallOrc
    CorpseNone,			// UnitDeadBody
    CorpseNone,			// Unit1x1DestroyedPlace
    CorpseNone,			// Unit2x2DestroyedPlace
    CorpseNone,			// Unit3x3DestroyedPlace
    CorpseNone,			// Unit4x4DestroyedPlace
    CorpseHuman,		// UnitPeasantWithGold
    CorpseOrc,			// UnitPeonWithGold
    CorpseHuman,		// UnitPeasantWithWood
    CorpseOrc,			// UnitPeonWithWood
    CorpseShip,			// UnitTankerHumanFull
    CorpseShip,			// UnitTankerOrcFull
};
#endif //not defined USE_CCL2

/**
**	Unit dies!
**
**	@param unit	The unit which dies.
**
**	@return		True the unit has died.
*/
global int HandleActionDie(Unit* unit)
{
    int type;

    type=unit->Type->Type;
    //
    //	Die animations:
    //		Corpse have own animations,
    //			Orc, Human, Ships, Sites.
    //
    switch( type ){
    case UnitPeonWithGold:
    case UnitPeonWithWood:
      type=UnitPeon;
      unit->Type=&UnitTypes[type];
      break;
      
    case UnitPeasantWithGold:
    case UnitPeasantWithWood:
      type=UnitPeasant;
      unit->Type=&UnitTypes[type];
      break;
    default:
      break;
    }


    switch( type ) {
	case Unit1x1DestroyedPlace:
	case Unit2x2DestroyedPlace:
	case Unit3x3DestroyedPlace:
	case Unit4x4DestroyedPlace:
	case UnitDeadBody:
	  if(UnitCorpse[unit->SubAction]){
	    UnitShowAnimation(unit,UnitCorpse[unit->SubAction]);
	  }
	  break;

	default:
	    if( type<UnitTypeInternalMax && UnitDie[type] ) {
	      UnitShowAnimation(unit,UnitDie[type]);	      
	    } else {
		DebugLevel3("RESET\n");
		unit->Reset=1;
		unit->Wait=1;
	    }
	    break;
    }


    //
    //	Die sequence terminated, generate corpse.
    //
    if( unit->Reset ) {
      DebugLevel3("Die complete %Zd\n",UnitNumber(unit));
      if(!UnitCorpse[type]){
	FreeUnitMemory(unit);
	return 1;
      }
      unit->SubAction=type;
      unit->Type=&UnitTypes[UnitDeadBody];
      unit->Frame=0;
      unit->State=0;
      unit->Reset=0;
      UnitNewHeading(unit);
      ChangeUnitOwner(unit,unit->Player,&Players[PlayerNumNeutral]);
    }

    return 0;
}

//@}
