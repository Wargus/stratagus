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

#endif

extern Animation ** UnitCorpse;

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

    switch( type ) {
	case Unit1x1DestroyedPlace:
	case Unit2x2DestroyedPlace:
	case Unit3x3DestroyedPlace:
	case Unit4x4DestroyedPlace:
	case UnitDeadBody:
	    if( UnitCorpse[unit->SubAction] ) {
		UnitShowAnimation(unit,UnitCorpse[unit->SubAction]);
	    }
	    break;

	default:
	    if( unit->Type->Animations ) {
		UnitShowAnimation(unit,unit->Type->Animations->Die);
	    } else {
		DebugLevel0("FIXME: die animation missing\n");
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
	if( !UnitCorpse[type] ){
	    FreeUnitMemory(unit);
	    return 1;
	}
	unit->SubAction=type;
	unit->Type=UnitTypeByIdent("unit-dead-body");
	unit->Frame=0;
	unit->State=0;
	unit->Reset=0;
	UnitNewHeading(unit);
	ChangeUnitOwner(unit,unit->Player,&Players[PlayerNumNeutral]);
    }

    return 0;
}

//@}
