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
/**@name action_still.c	-	The stand still action. */
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
#include "tileset.h"
#include "map.h"
#include "sound_server.h"
#include "missile.h"

#ifndef USE_CCL2
/*----------------------------------------------------------------------------
--	Still
----------------------------------------------------------------------------*/

//	Gryphon rider, Kurdan and Sky'ree:
local Animation GryphonRiderStill[] = {
    {2, 0, 6, 0},  {2, 0, 6, 5},  {2, 0, 6, 5},  {3, 0, 6, 5}
};

//	Dragon, Deathwing:
local Animation DragonStill[] = {
    {2, 0, 6, 0},  {2, 0, 6, 5},  {2, 0, 6, 5},  {3, 0, 6, 5}
};

//	GnomishFlyingMachine:
local Animation GnomishFlyingMachineStill[] = {
    {2, 0, 1, 0},  {2, 0, 1, 5},  {2, 0, 1, 0},  {3, 0, 1,-5}
};

//	Daemon:
local Animation DaemonStill[] = {
    {2, 0, 4, 0},  {2, 0, 4, 5},  {2, 0, 4, 5},  {3, 0, 4, 5}
};

//	Default:
local Animation DefaultStill[] = {
// FIXME: Reset frame 0, wait 1, than endless wait 5
    {0, 0, 4, 0}, {3, 0, 1, 0}
};

/*
**	Still animation. FIXME: move this to unit-type, CCL configurable
*/
global Animation* UnitStillAnimation[UnitTypeInternalMax] = {
    DefaultStill,		// UnitFootman
    DefaultStill,		// UnitGrunt
    DefaultStill,		// UnitPeasant
    DefaultStill,		// UnitPeon
    DefaultStill,		// UnitBallista
    DefaultStill,		// UnitCatapult
    DefaultStill,		// UnitKnight
    DefaultStill,		// UnitOgre
    DefaultStill,		// UnitArcher
    DefaultStill,		// UnitAxethrower
    DefaultStill,		// UnitMage
    DefaultStill,		// UnitDeathKnight
    DefaultStill,		// UnitPaladin
    DefaultStill,		// UnitOgreMage
    DefaultStill,		// UnitDwarves
    DefaultStill,		// UnitGoblinSappers
    DefaultStill,		// UnitAttackPeasant
    DefaultStill,		// UnitAttackPeon
    DefaultStill,		// UnitRanger
    DefaultStill,		// UnitBerserker
    DefaultStill,		// UnitAlleria
    DefaultStill,		// UnitTeronGorefiend
    GryphonRiderStill,		// UnitKurdanAndSky_ree
    DefaultStill,		// UnitDentarg
    DefaultStill,		// UnitKhadgar
    DefaultStill,		// UnitGromHellscream
    DefaultStill,		// UnitTankerHuman
    DefaultStill,		// UnitTankerOrc
    DefaultStill,		// UnitTransportHuman
    DefaultStill,		// UnitTransportOrc
    DefaultStill,		// UnitElvenDestroyer
    DefaultStill,		// UnitTrollDestroyer
    DefaultStill,		// UnitBattleship
    DefaultStill,		// UnitJuggernaught
    DefaultStill,		// UnitNothing
    DragonStill,		// UnitDeathwing
    DefaultStill,		// UnitNothing1
    DefaultStill,		// UnitNothing2
    DefaultStill,		// UnitGnomishSubmarine
    DefaultStill,		// UnitGiantTurtle
    GnomishFlyingMachineStill,	// UnitGnomishFlyingMachine
    DefaultStill,		// UnitGoblinZeppelin
    GryphonRiderStill,		// UnitGryphonRider
    DragonStill,		// UnitDragon
    DefaultStill,		// UnitTuralyon
    DefaultStill,		// UnitEyeOfKilrogg
    DefaultStill,		// UnitDanath
    DefaultStill,		// UnitKorgathBladefist
    DefaultStill,		// UnitNothing3
    DefaultStill,		// UnitCho_gall
    DefaultStill,		// UnitLothar
    DefaultStill,		// UnitGul_dan
    DefaultStill,		// UnitUtherLightbringer
    DefaultStill,		// UnitZuljin
    DefaultStill,		// UnitNothing4
    DefaultStill,		// UnitSkeleton
    DaemonStill,		// UnitDaemon
    DefaultStill,		// UnitCritter
    DefaultStill,		// UnitFarm
    DefaultStill,		// UnitPigFarm
    DefaultStill,		// UnitBarracksHuman
    DefaultStill,		// UnitBarracksOrc
    DefaultStill,		// UnitChurch
    DefaultStill,		// UnitAltarOfStorms
    DefaultStill,		// UnitScoutTowerHuman
    DefaultStill,		// UnitScoutTowerOrc
    DefaultStill,		// UnitStables
    DefaultStill,		// UnitOgreMound
    DefaultStill,		// UnitGnomishInventor
    DefaultStill,		// UnitGoblinAlchemist
    DefaultStill,		// UnitGryphonAviary
    DefaultStill,		// UnitDragonRoost
    DefaultStill,		// UnitShipyardHuman
    DefaultStill,		// UnitShipyardOrc
    DefaultStill,		// UnitTownHall
    DefaultStill,		// UnitGreatHall
    DefaultStill,		// UnitElvenLumberMill
    DefaultStill,		// UnitTrollLumberMill
    DefaultStill,		// UnitFoundryHuman
    DefaultStill,		// UnitFoundryOrc
    DefaultStill,		// UnitMageTower
    DefaultStill,		// UnitTempleOfTheDamned
    DefaultStill,		// UnitBlacksmithHuman
    DefaultStill,		// UnitBlacksmithOrc
    DefaultStill,		// UnitRefineryHuman
    DefaultStill,		// UnitRefineryOrc
    DefaultStill,		// UnitOilPlatformHuman
    DefaultStill,		// UnitOilPlatformOrc
    DefaultStill,		// UnitKeep
    DefaultStill,		// UnitStronghold
    DefaultStill,		// UnitCastle
    DefaultStill,		// UnitFortress
    DefaultStill,		// UnitGoldMine
    DefaultStill,		// UnitOilPatch
    DefaultStill,		// UnitStartLocationHuman
    DefaultStill,		// UnitStartLocationOrc
    DefaultStill,		// UnitGuardTowerHuman
    DefaultStill,		// UnitGuardTowerOrc
    DefaultStill,		// UnitCannonTowerHuman
    DefaultStill,		// UnitCannonTowerOrc
    DefaultStill,		// UnitCircleofPower
    DefaultStill,		// UnitDarkPortal
    DefaultStill,		// UnitRunestone
    DefaultStill,		// UnitWallHuman
    DefaultStill,		// UnitWallOrc
    DefaultStill,		// UnitDeadBody
    DefaultStill,		// Unit1x1DestroyedPlace
    DefaultStill,		// Unit2x2DestroyedPlace
    DefaultStill,		// Unit3x3DestroyedPlace
    DefaultStill,		// Unit4x4DestroyedPlace
    DefaultStill,		// UnitPeonWithGold
    DefaultStill,		// UnitPeasantWithGold
    DefaultStill,		// UnitPeonWithWood
    DefaultStill,		// UnitPeasantWithWood
    DefaultStill,		// UnitTankerHumanFull
    DefaultStill,		// UnitTankerOrcFull
};
#endif

/**
**	Unit stands still!
*/
global void HandleActionStill(Unit* unit)
{
    UnitType* type;
    Unit* goal;

    DebugLevel3(__FUNCTION__": %Zd\n",UnitNumber(unit));

    type=unit->Type;

    if( unit->SubAction ) {
	//
	//	Attacking unit in attack range.
	//
	AnimateActionAttack(unit);
    } else {
	//
	//	Still animation
	//
        DebugCheck( type->Type>=UnitTypeInternalMax
            || !UnitStillAnimation[type->Type] );

	UnitShowAnimation(unit,UnitStillAnimation[type->Type]);

	//
	//	FIXME: this a workaround of a bad code.
	//		UnitShowAnimation resets frame.
	//
	if( unit->State==1 && type->GoldMine ) {
	    unit->Frame=!!unit->Command.Data.GoldMine.Active;
	}
	if( unit->State==1 && type->GivesOil ) {
	    unit->Frame=unit->Command.Data.OilWell.Active ? 2 : 0;
	}

    }

    if( !unit->Reset ) {		// animation can't be aborted
	return;
    }

    //
    //	Building:	burning
    //
    if( type->Building ) {
	if( unit->HP ) {
	    int f;

	    f=(100*unit->HP)/unit->Stats->HitPoints;
	    if( f>75) {
		; // No fire for this
	    } else if( f>50 ) {
		MakeMissile(MissileSmallFire
			,unit->X*TileSizeX
				+(type->TileWidth*TileSizeX)/2
			,unit->Y*TileSizeY
				+(type->TileHeight*TileSizeY)/2
				-TileSizeY
			,0,0);
	    } else {
		MakeMissile(MissileBigFire
			,unit->X*TileSizeX
				+(type->TileWidth*TileSizeX)/2
			,unit->Y*TileSizeY
				+(type->TileHeight*TileSizeY)/2
				-TileSizeY
			,0,0);
	    }
	}
    }

#if 0  // a unit with type->Vanishes is _dying_.
    //
    //	Corpse:		vanishes
    //
    if( type->Vanishes ) {
	UnitCacheRemove(unit);
	FreeUnitMemory(unit);
	return;
    }
#endif

    //
    //	Critters:	are moving random around.
    //
    if( type->Critter ) {
	// FIXME: critters: skeleton and daemon are also critters??????
	if( type->Type==UnitCritter ) {
	    int x;
	    int y;
	
	    x=unit->X;
	    y=unit->Y;
	    switch( (SyncRand()>>12)&15 ) {
		case 0:	x++;		break;
		case 1:	y++;		break;
		case 2:	x--;		break;
		case 3:	y--;		break;
		case 4:	x++; y++;	break;
		case 5:	x--; y++;	break;
		case 6:	y--; x++;	break;
		case 7:	x--; y--;	break;
		default:
			break;
	    }
	    if( x<0 ) {
		x=0;
	    } else if( x>=TheMap.Width ) {
		x=TheMap.Width-1;
	    } 
	    if( y<0 ) {
		y=0;
	    } else if( y>=TheMap.Height ) {
		y=TheMap.Height-1;
	    }
	    if( x!=unit->X || y!=unit->Y ) {
		// FIXME: Don't use pathfinder for this.
		unit->Command.Action=UnitActionMove;
		unit->Command.Data.Move.Fast=1;
		unit->Command.Data.Move.Goal=NoUnitP;
		unit->Command.Data.Move.Range=0;
		unit->Command.Data.Move.SX=unit->X;
		unit->Command.Data.Move.SY=unit->Y;
		unit->Command.Data.Move.DX=x;
		unit->Command.Data.Move.DY=y;
		return;
	    }

	}
    }

    //
    //	Workers and mage didn't attack automatic
    //
    if( type->CanAttack && !type->CowerPeon && !type->CowerMage ) {
	//
	// JOHNS: removed Human controlled units attacks in attacking range.
	// JOHNS: use stand ground for old behavior.
	//	Computer controlled units react in reaction range.
	//
	if( /*unit->Player->Type!=PlayerHuman &&*/ !type->Tower ) {
	    if( (goal=AttackUnitsInReactRange(unit)) ) {
		CommandAttack(unit,goal->X,goal->Y,NULL,0);
		unit->SubAction|=2;
	    }
	} else if( (goal=AttackUnitsInRange(unit)) ) {
	    // FIXME: johns, looks wired what I have written here
	    // FIXME: Why have I written such a chaos? (johns)
	    if( !unit->SubAction || unit->Command.Data.Move.Goal!=goal ) {
		unit->Command.Data.Move.Goal=goal;
		unit->State=0;
		unit->SubAction=1;
		// Turn to target
		if( !type->Tower ) {
		    UnitNewHeadingFromXY(unit,goal->X-unit->X,goal->Y-unit->Y);
		    AnimateActionAttack(unit);
		}
	    }
	    return;
	}
    }

    if( unit->SubAction ) {
	unit->SubAction=unit->State=0;
    }

    //
    //	Land units:	are turning left/right.
    //
    if( type->LandUnit ) {
	switch( (MyRand()>>8)&0x0FF ) {
	    case 0:			// Turn clockwise
		unit->Heading=(unit->Heading+1)&7;
		UnitNewHeading(unit);
		if( UnitVisible(unit) ) {
		    MustRedraw|=RedrawMap;
		}
		break;
	    case 1:			// Turn counter clockwise
		unit->Heading=(unit->Heading-1)&7;
		UnitNewHeading(unit);
		if( UnitVisible(unit) ) {
		    MustRedraw|=RedrawMap;
		}
		break;
	    default:			// does nothing
		break;
	}
	return;
    }

    //
    //	Sea units:	are floating up/down.
    //
    if( type->SeaUnit ) {
	unit->IY=(MyRand()>>15)&1;
	return;
    }

    //
    //	Air units:	are floating up/down.
    //
    if( type->AirUnit ) {
	unit->IY=(MyRand()>>15)&1;
	return;
    }
}

//@}
