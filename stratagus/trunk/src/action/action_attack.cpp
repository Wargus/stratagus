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
/**@name action_attack.c	-	The attack action. */
/*
**	(c) Copyright 1998-2000 by Lutz Sammer
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
#include "missile.h"
#include "actions.h"
#include "sound.h"
#include "tileset.h"
#include "map.h"


#ifndef USE_CCL2
/*----------------------------------------------------------------------------
--	Attack
----------------------------------------------------------------------------*/

//
//	FIXME: Attack animations sequences, should me moved into unit
//	FIXME: structure and defined with ccl.
//

///	Footman,Grunt,Grom Hellscream,Danath,Korgath Bladefist
local Animation GruntAttack[] = {
    { 0, 0, 3, 25},{ 0, 0, 3,  5},{ 0, 0, 3,  5},{12, 0, 5,  5},{ 0, 0,10,-40},
    { 3, 0, 1,  0}
};

///	Peon, Peasant, Attacking Peon, Attacking Peasant.
global Animation PeonAttack[] = {
    { 0, 0, 3, 25},{ 0, 0, 3,  5},{ 0, 0, 3,  5},{12, 0, 5,  5},{ 0, 0, 3,  5},
    { 0, 0, 7,-20},{ 3, 0, 1,  0}
};

///	Ballista
local Animation BallistaAttack[] = {
    { 0, 0,25, 10},{12, 0,25,  5},{ 0, 0,100, 0},{ 0, 0,49,-15},{ 3, 0, 1,  0}
};

///	Catapult
local Animation CatapultAttack[] = {
    {12, 0, 4, 15},{ 0, 0, 4,- 5},{ 0, 0, 3,  5},{ 0, 0, 2,- 5},{ 0, 0, 2,  5},
    { 0, 0,30,- 5},{ 0, 0, 4,  5},{ 0, 0,100, 0},{ 0, 0,50,-15},{ 3, 0, 1,  0}
};

///	Knight, Paladin, Turalyon, Lothar, Uther Lightbringer
local Animation KnightAttack[] = {
    { 0, 0, 3, 25},{ 0, 0, 3,  5},{ 0, 0, 3,  5},{12, 0, 5,  5},{ 0, 0,10,-40},
    { 3, 0, 1,  0}
};

///	Ogre, Ogre-mage, Dentarg, Cho'gall
local Animation OgreAttack[] = {
    { 0, 0, 3, 25},{ 0, 0, 3,  5},{ 0, 0, 3,  5},{12, 0, 5,  5},{ 0, 0,10,-40},
    { 3, 0, 1,  0}
};

///	Archer, Ranger, Alleria
local Animation ArcherAttack[] = {
    { 0, 0,10, 25},{12, 0,10,  5},{ 0, 0,44,-30},
    { 3, 0, 1,  0}
};

///	Axethrower, Berserker, Zuljin
local Animation AxethrowerAttack[] = {
    { 0, 0, 3, 25},{ 0, 0, 3,  5},{ 0, 0, 3,  5},{12, 0, 3,  5},{ 0, 0,52,-40},
    { 3, 0, 1,  0}
};

///	Mage, Khadar
local Animation MageAttack[] = {
    { 0, 0, 5, 25},{ 0, 0, 5,  5},{12, 0, 7,  5},{ 0, 0, 5,  5},{ 0, 0,17,-40},
    { 3, 0, 1,  0}
};

///	Death Knight, Teron Gorefiend, Gul'dan
local Animation DeathKnightAttack[] = {
    { 0, 0, 5, 25},{ 0, 0, 5,  5},{12, 0, 7,  5},{ 0, 0, 5,  5},{ 0, 0,17,-40},
    { 3, 0, 1,  0}
};

///	Dwarves
local Animation DwarvesAttack[] = {
    { 0, 0, 3, 15},{12, 0, 5, 15},{ 0, 0, 3, 15},{ 0, 0,13,-45},{ 3, 0, 1,  0}
};

///	Goblin Sappers
local Animation GoblinSappersAttack[] = {
    { 0, 0, 3, 15},{12, 0, 5, 15},{ 0, 0, 3, 15},{ 0, 0,13,-45},{ 3, 0, 1,  0}
};

///	Gryphon Rider, Kurdan and Sky'ree:
local Animation GryphonRiderAttack[] = {
    { 0, 0, 6,  0},{ 0, 0, 6,  5},{ 0, 0, 6,  5},{ 0, 0, 6,  5},{ 2, 0, 1,  0},
    { 0, 0, 6,  5},{ 0, 0, 6,  5},{12, 0, 8,  5},{ 0, 0, 6,-30},{ 0, 0, 6,  5},
    { 0, 0, 6,  5},{ 0, 0, 6,  5},{ 0, 0, 6,-15},{ 0, 0, 6,  5},{ 0, 0, 6,  5},
    { 0, 0, 6,  5},{ 0, 0, 6,-15},{ 0, 0, 6,  5},{ 0, 0, 6,  5},{ 0, 0, 6,  5},
    { 0, 0, 6,-15},{ 0, 0, 6,  5},{ 0, 0, 6,  5},{ 0, 0, 6,  5},{ 0, 0, 6,-15},
    { 0, 0, 6,  5},{ 0, 0, 6,  5},{ 0, 0, 6,  5},{ 0, 0, 6,-15},{ 0, 0, 6,  5},
    { 0, 0, 6,  5},{ 0, 0, 6,  5},{ 1, 0, 1,-15}
};

///	Dragon, Deathwing
local Animation DragonAttack[] = {
    { 0, 0, 6,  0},{ 0, 0, 6,  5},{ 0, 0, 6,  5},{ 0, 0, 6,  5},{ 2, 0, 1,  0},
    {12, 0,20,  5},{ 0, 0, 6,-20},{ 0, 0, 6,  5},{ 0, 0, 6,  5},{ 0, 0, 6,  5},
    { 0, 0, 6,-15},{ 0, 0, 6,  5},{ 0, 0, 6,  5},{ 0, 0, 6,  5},{ 0, 0, 6,-15},
    { 0, 0, 6,  5},{ 0, 0, 6,  5},{ 0, 0, 6,  5},{ 0, 0, 6,-15},{ 0, 0, 6,  5},
    { 0, 0, 6,  5},{ 0, 0, 6,  5},{ 0, 0, 6,-15},{ 0, 0, 6,  5},{ 0, 0, 6,  5},
    { 0, 0, 6,  5},{ 0, 0, 6,-15},{ 0, 0, 6,  5},{ 0, 0, 6,  5},{ 0, 0, 6,  5},
    { 1, 0, 1,-15}
};

///	Eye of kilrogg
local Animation EyeOfKilroggAttack[] = {
    { 3, 0, 1,  0}
};

///	Human tanker, orc tanker:
local Animation TankerAttack[] = {
    {12, 0,30,  0},{ 0, 0,99,  0},{ 3, 0, 1,  0}
};

///	Human transporter, orc transporter:
local Animation TransportAttack[] = {
    {12, 0,119,  0},{ 3, 0, 1,  0}
};

///	Elven destroyer, Troll destroyer:
local Animation DestroyerAttack[] = {
    {12, 0,119,  0},{ 3, 0, 1,  0}
};

///	Battleship, Juggernaught
local Animation BattleshipAttack[] = {
    {12, 0,127,  0},{ 0, 0,102,  0},{ 3, 0, 1,  0}
};

///	Gnomish submarine, giant turtle
local Animation SubmarineAttack[] = {
    { 0, 0,10,  5},{ 0, 0,25,  5},{12, 0,25,  0},{ 0, 0,25,- 5},{ 0, 0,29,- 5},
    { 3, 0, 1,  0}
};

///	Gnomish flying machine
local Animation GnomishFlyingMachineAttack[] = {
    { 3, 0, 1,  0}
};

///	Goblin zeppelin
local Animation GoblinZeppelinAttack[] = {
    { 3, 0, 1,  0}
};

///	Critter
local Animation CritterAttack[] = {
    { 3, 0, 1,  0}
};

///	Skeleton
local Animation SkeletonAttack[] = {
    { 0, 0, 4, 15},{ 0, 0, 4, 15},{12, 0, 4, 15},{ 0, 0, 4, 15},{ 0, 0,18,-60},
    { 3, 0, 1,  0}
};

///	Daemon
local Animation DaemonAttack[] = {
    { 0, 0, 4,  0},{ 0, 0, 4,  5},{ 0, 0, 4,  5},{ 0, 0, 4,  5},{ 0, 0, 4,  5},
    { 2, 0, 1,  0},{ 0, 0, 4,  5},{ 0, 0, 4,  5},{ 0, 0, 4,  5},{12, 0, 4,  5},
    { 0, 0, 4,  5},{ 1, 0, 1,-45}
};

///	Guard tower
local Animation GuardTowerAttack[] = {
    {12, 0,59,  0},{ 3, 0, 1,  0},
};

///	Cannon tower
local Animation CannonTowerAttack[] = {
    {12, 0,150,  0},{ 3, 0, 1,  0},
};

/**
**	Attack animation. FIXME: move this to unit-type. CCL configurable
*/
local Animation* UnitAttack[UnitTypeInternalMax] = {
    GruntAttack,		// UnitFootman
    GruntAttack,		// UnitGrunt
    PeonAttack,			// UnitPeasant
    PeonAttack,			// UnitPeon
    BallistaAttack,		// UnitBallista
    CatapultAttack,		// UnitCatapult
    KnightAttack,		// UnitKnight
    OgreAttack,			// UnitOgre
    ArcherAttack,		// UnitArcher
    AxethrowerAttack,		// UnitAxethrower
    MageAttack,			// UnitMage
    DeathKnightAttack,		// UnitDeathKnight
    KnightAttack,		// UnitPaladin
    OgreAttack,			// UnitOgreMage
    DwarvesAttack,		// UnitDwarves
    GoblinSappersAttack,	// UnitGoblinSappers
    PeonAttack,			// UnitAttackPeasant
    PeonAttack,			// UnitAttackPeon
    ArcherAttack,		// UnitRanger
    AxethrowerAttack,		// UnitBerserker
    ArcherAttack,		// UnitAlleria
    DeathKnightAttack,		// UnitTeronGorefiend
    GryphonRiderAttack,		// UnitKurdanAndSky_ree
    OgreAttack,			// UnitDentarg
    MageAttack,			// UnitKhadgar
    GruntAttack,		// UnitGromHellscream
    TankerAttack,		// UnitTankerHuman
    TankerAttack,		// UnitTankerOrc
    TransportAttack,		// UnitTransportHuman
    TransportAttack,		// UnitTransportOrc
    DestroyerAttack,		// UnitElvenDestroyer
    DestroyerAttack,		// UnitTrollDestroyer
    BattleshipAttack,		// UnitBattleship
    BattleshipAttack,		// UnitJuggernaught
    0,				// UnitNothing
    DragonAttack,		// UnitDeathwing
    0,				// UnitNothing1
    0,				// UnitNothing2
    SubmarineAttack,		// UnitGnomishSubmarine
    SubmarineAttack,		// UnitGiantTurtle
    GnomishFlyingMachineAttack,	// UnitGnomishFlyingMachine
    GoblinZeppelinAttack,	// UnitGoblinZeppelin
    GryphonRiderAttack,		// UnitGryphonRider
    DragonAttack,		// UnitDragon
    KnightAttack,		// UnitTuralyon
    EyeOfKilroggAttack,		// UnitEyeOfKilrogg
    GruntAttack,		// UnitDanath
    GruntAttack,		// UnitKorgathBladefist
    0,				// UnitNothing3
    OgreAttack,			// UnitCho_gall
    KnightAttack,		// UnitLothar
    DeathKnightAttack,		// UnitGul_dan
    KnightAttack,		// UnitUtherLightbringer
    AxethrowerAttack,		// UnitZuljin
    0,				// UnitNothing4
    SkeletonAttack,		// UnitSkeleton
    DaemonAttack,		// UnitDaemon
    CritterAttack,		// UnitCritter
    0,				// UnitFarm
    0,				// UnitPigFarm
    0,				// UnitBarracksHuman
    0,				// UnitBarracksOrc
    0,				// UnitChurch
    0,				// UnitAltarOfStorms
    GuardTowerAttack,		// UnitScoutTowerHuman
    GuardTowerAttack,		// UnitScoutTowerOrc
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
    GuardTowerAttack,		// UnitGuardTowerHuman
    GuardTowerAttack,		// UnitGuardTowerOrc
    CannonTowerAttack,		// UnitCannonTowerHuman
    CannonTowerAttack,		// UnitCannonTowerOrc
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
    PeonAttack,			// UnitPeasantWithGold
    PeonAttack,			// UnitPeonWithGold
    PeonAttack,			// UnitPeasantWithWood
    PeonAttack,			// UnitPeonWithWood
    TankerAttack,		// UnitTankerHumanFull
    TankerAttack,		// UnitTankerOrcFull
};
#endif

/**
**	Generic unit attacker.
**
**	@param unit	Unit, for that the attack animation is played.
**	@param attack	Attack animation.
*/
local void DoActionAttackGeneric(Unit* unit,Animation* attack)
{
    int flags;
    int oframe;

    oframe=unit->Frame;
    flags=UnitShowAnimation(unit,attack);

    IfDebug(
	if( (unit->Frame&127)>=unit->Type->RleSprite->NumFrames ) {
	    DebugLevel0("Oops what this %s %d,%d %d #%d\n"
		,unit->Type->Ident
		,oframe,oframe&127
		,unit->Frame&127
		,unit->Type->RleSprite->NumFrames);
	    SaveUnit(unit,stdout);
	    abort();
	    return;
	}
    );

    if( (flags&AnimationSound) ) {	
	PlayUnitSound(unit,VoiceAttacking);
    }

    if( flags&AnimationMissile ) {	// time to fire projectil
	FireMissile(unit);
    }
}

/**
**	Animate unit attack!
**
**	@param unit	Unit, for that the attack animation is played.
*/
global int AnimateActionAttack(Unit* unit)
{
    int type;

    type=unit->Type->Type;
    if( type<UnitTypeInternalMax && UnitAttack[type] ) {
	DoActionAttackGeneric(unit,UnitAttack[type]);
    }

    return 0;
}

/**
**	Unit attacks!
**
**	I added a little trick, if SubAction&2 is true the goal is a weak goal.
**	This means the unit AI (little AI) could choose a new better goal.
**
**	@param unit	Unit, for that the attack is handled.
*/
global int HandleActionAttack(Unit* unit)
{
    Unit* goal;
    int wall;
    int err;

    DebugLevel3(__FUNCTION__": Attack %Zd\n",UnitNumber(unit));

    switch( unit->SubAction ) {
	//
	//	Move near to target.
	//
	case 0:
	case 2:
	    // FIXME: RESET FIRST!!
	    err=HandleActionMove(unit); 
	    if( unit->Reset ) {
		//
		//	Target is dead, choose new one.
		//
		goal=unit->Command.Data.Move.Goal;
		if( goal && (!goal->HP
			|| goal->Command.Action==UnitActionDie) ) {
		    unit->Command.Data.Move.Goal=goal=NoUnitP;
		}

		//
		//	No goal: if meeting enemy attack it.
		//
		wall=0;
		if( !goal
			&& !(wall=WallOnMap(unit->Command.Data.Move.DX
			     ,unit->Command.Data.Move.DY)) ) {
		    goal=AttackUnitsInReactRange(unit);
		    if( goal ) {
			unit->Command.Data.Move.Goal=goal;
			unit->Command.Data.Move.Fast=1;
			unit->Command.Data.Move.DX=goal->X;
			unit->Command.Data.Move.DY=goal->Y;
			unit->SubAction|=2;
			DebugLevel3("Unit in react range %Zd\n",UnitNumber(goal));
		    }
		} else 

		//
		//	Have a weak target, try a better target.
		//
		if( goal && (unit->SubAction&2) ) {
		    Unit* temp;

		    temp=AttackUnitsInReactRange(unit);
		    if( temp && temp->Type->Priority>goal->Type->Priority ) {
			unit->Command.Data.Move.Goal=goal=temp;
			unit->Command.Data.Move.Fast=1;
			unit->Command.Data.Move.DX=temp->X;
			unit->Command.Data.Move.DY=temp->Y;
		    }
		}

		//
		//	Have reached target?
		//
		if( goal && MapDistanceToUnit(unit->X,unit->Y,goal)
			<=unit->Stats->AttackRange ) {
		    unit->State=0;
		    if( !unit->Type->Tower ) {
			UnitNewHeadingFromXY(unit
			    ,goal->X-unit->X,goal->Y-unit->Y);
		    }
		    unit->SubAction++;
		} else if( wall && MapDistance(unit->X,unit->Y
			    ,unit->Command.Data.Move.DX
			    ,unit->Command.Data.Move.DY)
				<=unit->Stats->AttackRange ) {
		    DebugLevel3("Attacking wall\n");
		    unit->State=0;
		    if( !unit->Type->Tower ) {
			UnitNewHeadingFromXY(unit
			    ,unit->Command.Data.Move.DX-unit->X
			    ,unit->Command.Data.Move.DY-unit->Y);
		    }
		    unit->SubAction=1;
		} else if( err ) {
		    unit->SubAction=0;
		    return 1;
		}
		unit->Command.Action=UnitActionAttack;
	    }
	    break;

	//
	//	Attack the target.
	//
	case 1:
	case 3:
	    AnimateActionAttack(unit);
	    if( unit->Reset ) {
		goal=unit->Command.Data.Move.Goal;
		//
		//	Goal is "weak" or a wall.
		//
		if( !goal && WallOnMap(unit->Command.Data.Move.DX
			     ,unit->Command.Data.Move.DY) ) {
		    DebugLevel3("attack a wall!!!!\n");
		    break;
		}

		//
		//	Target is dead, choose new one.
		//
		if( !goal || !goal->HP
			|| goal->Command.Action==UnitActionDie ) {
		    unit->State=0;
		    goal=AttackUnitsInReactRange(unit);
		    unit->Command.Data.Move.Goal=goal;
		    if( !goal ) {
			unit->SubAction=0;
			unit->Command.Action=UnitActionStill;	// cade?
			return 1;
		    }
		    unit->SubAction|=2;
		    DebugLevel3("Unit in react range %Zd\n",UnitNumber(goal));
		    unit->Command.Data.Move.DX=goal->X;
		    unit->Command.Data.Move.DY=goal->Y;
		    if( !unit->Type->Tower ) {
			UnitNewHeadingFromXY(unit
			    ,goal->X-unit->X,goal->Y-unit->Y);
		    }
		} else

		//
		//	Have a weak target, try a better target.
		//
		if( goal && (unit->SubAction&2) ) {
		    Unit* temp;

		    temp=AttackUnitsInReactRange(unit);
		    if( temp && temp->Type->Priority>goal->Type->Priority ) {
			unit->Command.Data.Move.Goal=goal=temp;
			unit->Command.Data.Move.DX=goal->X;
			unit->Command.Data.Move.DY=goal->Y;
			if( !unit->Type->Tower ) {
			    UnitNewHeadingFromXY(unit
				,goal->X-unit->X,goal->Y-unit->Y);
			}
		    }
		}

		//
		//	Still near to target, if not goto target.
		//
		if( MapDistanceToUnit(unit->X,unit->Y,goal)
			>unit->Stats->AttackRange ) {
		    unit->Command.Data.Move.Fast=1;
		    unit->Command.Data.Move.DX=goal->X;
		    unit->Command.Data.Move.DY=goal->Y;
		    unit->Frame=0;
		    unit->State=0;
		    unit->SubAction--;
		    break;
		}
	    }
	    break;
    }

    return 0;
}

//@}
