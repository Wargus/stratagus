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
/**@name missile.c	-	The missiles. */
//
//	(c) Copyright 1998-2001 by Lutz Sammer
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
#include "video.h"
#include "tileset.h"
#include "map.h"
#include "sound_id.h"
#include "unitsound.h"
#include "unittype.h"
#include "player.h"
#include "unit.h"
#include "missile.h"
#include "sound.h"
#include "ui.h"

#include "etlib/hash.h"

/*----------------------------------------------------------------------------
--	Declarations
----------------------------------------------------------------------------*/

/**
**	Missile-class this defines how a missile-type reacts.
**
**	@todo
**		Here is something double defined, the whirlwind is
**		ClassWhirlwind and also handled by controler.
**
**	FIXME:	We need no class or no controller.
*/
enum _missile_class_ {
	/**
	**	Missile does nothing
	*/
    MissileClassNone,
	/**
	**	Missile flies from x,y to x1,y1
	*/
    MissileClassPointToPoint,
	/**
	**	Missile flies from x,y to x1,y1 and stays there for a moment
	*/
    MissileClassPointToPointWithDelay,
	/**
	**	Missile don't move, than disappears
	*/
    MissileClassStayWithDelay,
	/**
	**	Missile flies from x,y to x1,y1 than bounces three times.
	*/
    MissileClassPointToPoint3Bounces,
	/**
	**	Missile flies from x,y to x1,y1 than changes into flame shield
	*/
    MissileClassFireball,
	/**
	**	Missile surround x,y
	*/
    MissileClassFlameShield,
	/**
	**	Missile appears at x,y, is blizzard
	*/
    MissileClassBlizzard,
	/**
	**	Missile appears at x,y, is death and decay
	*/
    MissileClassDeathDecay,
	/**
	**	Missile appears at x,y, is whirlwind
	*/
    MissileClassWhirlwind,
	/**
	**	Missile appears at x,y, than cycle through the frames up and
	**	down.
	*/
    MissileClassCycleOnce,
	/**
	**	Missile flies from x,y to x1,y1 than shows hit animation.
	*/
    MissileClassPointToPointWithHit,
	/**
	**	Missile don't move, than checks the source unit for HP.
	*/
    MissileClassFire,
	/**
	**	Missile is controlled completely by Controller() function.
	*/
    MissileClassCustom,
};

/*----------------------------------------------------------------------------
--	Variables
----------------------------------------------------------------------------*/

/**
**	Missile class names, used to load/save the missiles.
*/
global const char* MissileClassNames[] = {
    "missile-class-none",
    "missile-class-point-to-point",
    "missile-class-point-to-point-with-delay",
    "missile-class-stay-with-delay",
    "missile-class-point-to-point-3bounces",
    "missile-class-fireball",
    "missile-class-flame-shield",
    "missile-class-blizzard",
    "missile-class-death-decay",
    "missile-class-whirlwind",
    "missile-class-cycle-once",
    "missile-class-point-to-point-with-hit",
    "missile-class-fire",
    "missile-class-custom",
    NULL
};

/**
**	Missile type type definition
*/
global const char MissileTypeType[] = "missile-type";

#ifndef USE_CCL

/**
**	Mapping of missile numbers in original puds to our internal symbols.
**	Default without CCL support.
*/
local char* DefaultMissileTypeWcNames[] = {
    "missile-lightning",
    "missile-griffon-hammer",
    "missile-dragon-breath",
    "missile-fireball",
    "missile-flame-shield",
    "missile-blizzard",
    "missile-death-and-decay",
    "missile-big-cannon",
    "missile-exorcism",
    "missile-heal-effect",
    "missile-touch-of-death",
    "missile-rune",
    "missile-whirlwind",
    "missile-catapult-rock",
    "missile-ballista-bolt",
    "missile-arrow",
    "missile-axe",
    "missile-submarine-missile",
    "missile-turtle-missile",
    "missile-small-fire",
    "missile-big-fire",
    "missile-impact",
    "missile-normal-spell",
    "missile-explosion",
    "missile-small-cannon",
    "missile-cannon-explosion",
    "missile-cannon-tower-explosion",
    "missile-daemon-fire",
    "missile-green-cross",
    "missile-none",
    "missile-blizzard-hit",
    "missile-death-coil",
    "missile-none",
    NULL
};

/**
**	Define our missile-types.
**	Default without CCL support.
*/
global MissileType DefaultMissileTypes[] = {
{ MissileTypeType,
    "missile-lightning",
    "lightning.png",
    32,32,
    { NULL },
    { NULL },
    MissileClassPointToPointWithHit,
    1,
    16,
    0,
    },
{ MissileTypeType,
    "missile-griffon-hammer",
    "gryphon hammer.png",
    32,32,
    { NULL },
    { "fireball hit" },
    MissileClassPointToPoint3Bounces,
    1,
    "missile-explosion",	NULL,
    },
{ MissileTypeType,
    "missile-dragon-breath",
    "dragon breath.png",
    32,32,
    { NULL },
    { "fireball hit" },
    MissileClassPointToPoint3Bounces,
    1,
    16,
    1,
    "missile-explosion",	NULL,
    },
{ MissileTypeType,
    "missile-fireball",
    "fireball.png",
    32,32,
    { NULL },
    { "fireball hit" },
    MissileClassPointToPoint,
    1,
    16,
    1,
    },
{ MissileTypeType,
    "missile-flame-shield",
    "flame shield.png",
    32,32,
    { NULL },
    { NULL },
    MissileClassFlameShield,
    1,
    16,
    1,
    },
{ MissileTypeType,
    "missile-blizzard",
    "blizzard.png",
    32,32,
    { NULL },
    { NULL },
    MissileClassBlizzard,
    1,
    16,
    1,
    "missile-blizzard-hit", NULL
    },
{ MissileTypeType,
    "missile-death-and-decay",
    "death and decay.png",
    32,32,
    { NULL },
    { NULL },
    MissileClassDeathDecay,
    1,
    16,
    1,
    },
{ MissileTypeType,
    "missile-big-cannon",
    "big cannon.png",
    16,16,
    { NULL },
    { "explosion" },
    MissileClassPointToPoint,
    1,
    16,
    1,
    "missile-cannon-tower-explosion",	NULL,
    },
{ MissileTypeType,
    "missile-exorcism",
    "exorcism.png",
    48,48,
    { NULL },
    { NULL },
    MissileClassPointToPoint,
    1,
    16,
    1,
    },
{ MissileTypeType,
    "missile-heal-effect",
    "heal effect.png",
    48,48,
    { NULL },
    { NULL },
    MissileClassStayWithDelay,
    1,
    16,
    1,
    },
{ MissileTypeType,
    "missile-touch-of-death",
    "touch of death.png",
    32,32,
    { NULL },
    { NULL },
    MissileClassPointToPointWithHit,
    1,
    16,
    .,
    },
{ MissileTypeType,
    "missile-rune",
    "rune.png",
    16,16,
    { NULL },
    { NULL },
    MissileClassStayWithDelay,
    5,
    16,
    1,
    },
{ MissileTypeType,
    "missile-whirlwind",
    "tornado.png",
    56,56,
    { NULL },
    { NULL },
    MissileClassWhirlwind,
    1,
    16,
    1,
    },
{ MissileTypeType,
    "missile-catapult-rock",
    "catapult rock.png",
    32,32,
    { NULL },
    { "explosion" },
    MissileClassPointToPointWithDelay,
    1,
    16,
    1,
    "missile-impact",	NULL,
    },
{ MissileTypeType,
    "missile-ballista-bolt",
    "ballista bolt.png",
    64,64,
    { NULL },
    { "explosion" },
    MissileClassPointToPoint,
    1,
    16,
    1,
    "missile-impact",	NULL,
    },
{ MissileTypeType,
    "missile-arrow",
    "arrow.png",
    40,40,
    { NULL },
    { "bow hit" },
    MissileClassPointToPoint,
    1,
    16,
    0,
    },
{ MissileTypeType,
    "missile-axe",
    "axe.png",
    32,32,
    { NULL },
    { "bow hit" },
    MissileClassPointToPoint,
    1,
    16,
    0,
    },
{ MissileTypeType,
    "missile-submarine-missile",
    "submarine missile.png",
    40,40,
    { NULL },
    { "explosion" },
    MissileClassPointToPoint,
    1,
    16,
    1,
    "missile-impact",	NULL,
    },
{ MissileTypeType,
    "missile-turtle-missile",
    "turtle missile.png",
    40,40,
    { NULL },
    { "explosion" },
    MissileClassPointToPoint,
    1,
    16,
    1,
    "missile-impact",	NULL,
    },
{ MissileTypeType,
    "missile-small-fire",
    "small fire.png",
    32,48,
    { NULL },
    { NULL },
    MissileClassFire,
    8,
    16,
    1,
    },
{ MissileTypeType,
    "missile-big-fire",
    "big fire.png",
    48,48,
    { NULL },
    { NULL },
    MissileClassFire,
    8,
    16,
    1,
    },
{ MissileTypeType,
    "missile-impact",
    "ballista-catapult impact.png",
    48,48,
    { NULL },
    { NULL },
    MissileClassStayWithDelay,
    1,
    16,
    1,
    },
{ MissileTypeType,
    "missile-normal-spell",
    "normal spell.png",
    32,32,
    { NULL },
    { NULL },
    MissileClassStayWithDelay,
    1,
    16,
    1,
    },
{ MissileTypeType,
    "missile-explosion",
    "explosion.png",
    64,64,
    { NULL },
    { NULL },
    MissileClassStayWithDelay,
    1,
    16,
    1,
    },
{ MissileTypeType,
    "missile-small-cannon",
    "cannon.png",
    32,32,
    { NULL },
    { "explosion" },
    MissileClassPointToPointWithDelay,
    1,
    16,
    1,
    "missile-cannon-explosion",	NULL,
    },
{ MissileTypeType,
    "missile-cannon-explosion",
    "cannon explosion.png",
    32,32,
    { NULL },
    { NULL },
    MissileClassStayWithDelay,
    1,
    16,
    1,
    },
{ MissileTypeType,
    "missile-cannon-tower-explosion",
    "cannon-tower explosion.png",
    32,32,
    { NULL },
    { NULL },
    MissileClassStayWithDelay,
    1,
    16,
    1,
    },
{ MissileTypeType,
    "missile-daemon-fire",
    "daemon fire.png",
    32,32,
    { NULL },
    { NULL },
    MissileClassPointToPoint,
    1,
    16,
    1,
    },
{ MissileTypeType,
    "missile-green-cross",
    "green cross.png",
    32,32,
    { NULL },
    { NULL },
    MissileClassCycleOnce,
    FRAMES_PER_SECOND/30
    16,
    1,
    },
{ MissileTypeType,
    "missile-none",
    NULL,
    32,32,
    { NULL },
    { NULL },
    MissileClassNone,
    1,
    16,
    1,
    },
{ MissileTypeType,
    "missile-blizzard-hit",
    "blizzard.png",
    32,32,
    { NULL },
    { NULL },
    MissileClassStayWithDelay,
    1,
    16,
    1,
    },
{ MissileTypeType,
    "missile-death-coil",
    "touch of death.png",
    32,32,
    { NULL },
    { NULL },
    MissileClassPointToPoint,
    1,
    16,
    1,
    },
{ MissileTypeType,
    "missile-custom",
    NULL,
    32,32,
    { NULL },
    { NULL },
    MissileClassCustom,
    1,
    16,
    1,
    },
{ }
};

#endif

/**
**	W*rCr*ft number to internal missile-type name.
*/
global char** MissileTypeWcNames
#ifndef USE_CCL
    =DefaultMissileTypeWcNames
#endif
    ;

/**
**	Defined missile-types.
*/
global MissileType* MissileTypes
#ifndef USE_CCL
    = DefaultMissileTypes
#endif
    ;

global int NumMissileTypes;		/// number of missile-types made.

/*
**	Next missile-types are used hardcoded in the source.
*/
global MissileType* MissileTypeSmallFire;	/// Small fire missile-type
global MissileType* MissileTypeBigFire;		/// Big fire missile-type
global MissileType* MissileTypeGreenCross;	/// Green cross missile-type

IfDebug(
global int NoWarningMissileType;		/// quiet ident lookup.
);

#define MAX_MISSILES	1800		/// maximum number of missiles

local int NumMissiles;			/// currently used missiles
local Missile Missiles[MAX_MISSILES];	/// all missiles on map

    /// lookup table for missile names
local hashtable(MissileType*,65) MissileTypeHash;

local int BlizzardMissileHit;		/// Flag for blizzards.

/*----------------------------------------------------------------------------
--	Functions
----------------------------------------------------------------------------*/

/**
**	Load the graphics for the missiles.
*/
global void LoadMissileSprites(void)
{
    int i;
    const char* file;

    for( i=0; MissileTypes[i].OType; ++i ) {
	if( (file=MissileTypes[i].File) ) {
	    char* buf;

	    buf=alloca(strlen(file)+9+1);
#ifdef NEW_NAMES
	    file=strcat(strcpy(buf,"graphics/"),file);
#else
	    file=strcat(strcpy(buf,"graphic/"),file);
#endif
	    ShowLoadProgress("\tMissile %s\n",file);
	    MissileTypes[i].Sprite=LoadSprite(
		    file,MissileTypes[i].Width,MissileTypes[i].Height);
	}
    }
    NumMissileTypes=i;

    //
    //	Add missile names to hash table
    //
    for( i=0; i<NumMissileTypes; ++i ) {
	*(MissileType**)hash_add(MissileTypeHash,MissileTypes[i].Ident)
		=&MissileTypes[i];
    }

    //
    //	Resolve impact missiles and sounds.
    //
    for( i=0; i<NumMissileTypes; ++i ) {
	if( MissileTypes[i].FiredSound.Name ) {
	    MissileTypes[i].FiredSound.Sound
		    =SoundIdForName(MissileTypes[i].FiredSound.Name);
	}
	if( MissileTypes[i].ImpactSound.Name ) {
	    MissileTypes[i].ImpactSound.Sound
		    =SoundIdForName(MissileTypes[i].ImpactSound.Name);
	}
	if( MissileTypes[i].ImpactName ) {
	    MissileTypes[i].ImpactMissile
		    =MissileTypeByIdent(MissileTypes[i].ImpactName);
	}
    }

    MissileTypeSmallFire=MissileTypeByIdent("missile-small-fire");
    // FIXME: FIXME: FIXME: very diry hack
    MissileTypeSmallFire->Sprite->NumFrames=6;
    MissileTypeBigFire=MissileTypeByIdent("missile-big-fire");
    MissileTypeGreenCross=MissileTypeByIdent("missile-green-cross");
}

/**
**	Get Missile type by identifier.
**
**	@param ident	Identifier.
**
**	@return		Missile type pointer.
*/
global MissileType* MissileTypeByIdent(const char* ident)
{
    MissileType** type;

    type=(MissileType**)hash_find(MissileTypeHash,(char*)ident);

    if( type ) {
	return *type;
    }

    IfDebug( 
	if( !NoWarningMissileType ) {
	    DebugLevel0Fn("Missile %s not found\n",ident);
	}
    );
    return NULL;
}

/**
**	Allocate an empty missile-type slot.
**
**	@todo	Don't allocate an array of missile-types, allocate an array
**		of pointers.
**
**	@param ident	Identifier to identify the slot (malloced by caller!).
**
**	@return		New allocated (zeroed) missile-type pointer.
*/
global MissileType* NewMissileTypeSlot(char* ident)
{
    MissileType* type;
    unsigned i;

    //
    //	Allocate new memory. (+2 for start end empty last entry.)
    //
    type=calloc(NumMissileTypes+2,sizeof(MissileType));
    if( !type ) {
	fprintf(stderr,"Out of memory\n");
	exit(-1);
    }
    memcpy(type,MissileTypes,sizeof(MissileType)*NumMissileTypes);
    if( MissileTypes ) {
	free(MissileTypes);
    }
    MissileTypes=type;
    type=MissileTypes+NumMissileTypes++;
    type->OType=MissileTypeType;
    type->Ident=ident;
    //
    //	Rehash.
    //
    for( i=0; i<NumMissileTypes; ++i ) {
	*(MissileType**)hash_add(MissileTypeHash,MissileTypes[i].Ident)
		=&MissileTypes[i];
    }
    return type;
}

/**
**	Create a new missile at (x,y).
**
**	@param type	Type pointer of missile.
**	@param sx	Missile x start point in pixel.
**	@param sy	Missile y start point in pixel.
**	@param dx	Missile x destination point in pixel.
**	@param dy	Missile y destination point in pixel.
**
**	@return		created missile.
**
**	@todo
**		Need a better memory management for missiles.
*/
global Missile* MakeMissile(MissileType* type,int sx,int sy,int dx,int dy)
{
    Missile* missile;

    DebugLevel3Fn("type %Zd(%s) at %d,%d to %d,%d\n"
	    ,type-MissileTypes,type->Ident,sx,sy,dx,dy);

    //
    //	Find free slot, FIXME: see MakeUnit for better code
    //
    for( missile=Missiles; missile<Missiles+NumMissiles; ++missile ) {
	if( missile->Type==MissileFree ) {
	    goto found;
	}
    }

    //	Check maximum missiles!
    if( NumMissiles==MAX_MISSILES ) {
	printf("Maximum of missiles reached\n");
	abort();
	return NULL;
    }

    missile=Missiles+NumMissiles++;

found:
    missile->X=sx-type->Width/2;
    missile->Y=sy-type->Height/2;
    missile->DX=dx-type->Width/2;
    missile->DY=dy-type->Height/2;
    missile->Type=type;
    missile->Frame=0;
    missile->State=0;
    missile->Wait=1;

    missile->SourceUnit=NULL;

    missile->Damage = 0;
    missile->TargetUnit = NULL;
    missile->TTL = -1;
    missile->Controller = NULL;

    return missile;
}

/**
**	Free a missile. FIXME: could be done better memory management.
**
**	@todo
**		Need a better memory management for missiles.
**
**	@param missile	Missile pointer.
*/
local void FreeMissile(Missile* missile)
{
    missile->Type=MissileFree;
    if( missile->SourceUnit ) {
	RefsDebugCheck( !missile->SourceUnit->Refs );
	if( missile->SourceUnit->Destroyed ) {
	    if( !--missile->SourceUnit->Refs ) {
		ReleaseUnit(missile->SourceUnit);
	    }
	} else {
	    --missile->SourceUnit->Refs;
	    RefsDebugCheck( !missile->SourceUnit->Refs );
	}
	missile->SourceUnit=NoUnitP;
    }
    if( missile->TargetUnit ) {
	RefsDebugCheck( !missile->TargetUnit->Refs );
	if( missile->TargetUnit->Destroyed ) {
	    if( !--missile->TargetUnit->Refs ) {
		ReleaseUnit(missile->TargetUnit);
	    }
	} else {
	    --missile->TargetUnit->Refs;
	    RefsDebugCheck( !missile->TargetUnit->Refs );
	}
	missile->TargetUnit=NoUnitP;
    }
}

/**
**	Calculate damage.
**
**	Damage calculation:
**		(BasicDamage-Armor)+PiercingDamage
**	damage =----------------------------------
**				    2
**	damage is multiplied by random 1 or 2.
**
**	NOTE: different targets (big are hit by some missiles better)
**	NOTE: hidden targets are hit worser.
**	NOTE: targets higher are hit worser.
**
**	@param attack_stats	Attacker attributes.
**	@param goal_stats	Goal attributes.
**	@param bloodlust	If attacker has bloodlust
**
**	@return			damage produces on goal.
*/
local int CalculateDamageStats(const UnitStats * attacker_stats,
	const UnitStats * goal_stats, int bloodlust)
{
    int damage;
    int basic_damage;
    int piercing_damage;

    basic_damage = attacker_stats->BasicDamage;
    piercing_damage = attacker_stats->PiercingDamage;
    if (bloodlust) {
	basic_damage *= 2;
	piercing_damage *= 2;
	DebugLevel0Fn("bloodlust\n");
    }

    damage = -goal_stats->Armor;
    damage += basic_damage;
    if (damage < 0) {
	damage = 0;
    }
    damage += piercing_damage + 1;	// round up
    damage /= 2;
    damage *= ((SyncRand() >> 15) & 1) + 1;
    DebugLevel3Fn("Damage done %d\n", damage);

    return damage;
}

/**
**	Calculate damage.
**
**	@param attack_stats	Attacker attributes.
**	@param goal		Goal unit.
**	@param bloodlust	If attacker has bloodlust
**	@return			damage produces on goal.
*/
local int CalculateDamage(const UnitStats* attacker_stats,
    const Unit* goal,int bloodlust)
{
    return CalculateDamageStats(attacker_stats,goal->Stats,bloodlust);
}

/**
**	Fire missile.
**
**	@param unit	Unit that fires the missile.
*/
global void FireMissile(Unit* unit)
{
    int x;
    int y;
    int dx;
    int dy;
    Unit* goal;
    Missile* missile;

    DebugLevel3Fn("\n");

    //
    //	Goal dead?
    //
    goal=unit->Orders[0].Goal;
    if( goal ) {

	// Better let the caller/action handle this.

	if( goal->Destroyed ) {
	    DebugLevel0Fn("destroyed unit\n");
	    return;
	}
	if( goal->Removed ) {
	    DebugLevel3Fn("Missile-none hits removed unit!\n");
	    return;
	}
	if( !goal->HP || goal->Orders[0].Action==UnitActionDie ) {
	    DebugLevel3Fn("Missile hits dead unit!\n");
	    return;
	}

	// FIXME: this is wrong some missiles didn't hit the target
	// FIXME: they hit the field of the target and all units on it.
	// FIXME: goal is already dead, but missile could hit others?
    }

    //
    //	None missile hits immediately!
    //
    if( ((MissileType*)unit->Type->Missile.Missile)->Class==MissileClassNone ) {
	// No goal, take target coordinates
	if( !goal ) {
	    dx=unit->Orders[0].X;
	    dy=unit->Orders[0].Y;
	    if( WallOnMap(dx,dy) ) {
		if( HumanWallOnMap(dx,dy) ) {
		    // FIXME: don't use UnitTypeByIdent here, this is slow!
		    HitWall(dx,dy,CalculateDamageStats(unit->Stats,
			    UnitTypeByIdent("unit-human-wall")->Stats,0));
		} else {
		    // FIXME: don't use UnitTypeByIdent here, this is slow!
		    HitWall(dx,dy,CalculateDamageStats(unit->Stats,
			    UnitTypeByIdent("unit-orc-wall")->Stats,0));
		}
		return;
	    }

	    DebugLevel1Fn("Missile-none hits no unit, shouldn't happen!\n");
	    return;
	}

	HitUnit(goal,CalculateDamage(unit->Stats,goal,unit->Bloodlust));

	return;
    }

    x=unit->X*TileSizeX+TileSizeX/2;	// missile starts in tile middle
    y=unit->Y*TileSizeY+TileSizeY/2;

    if( goal ) {
	DebugCheck( !goal->Type );	// Target invalid?
	// Fire to nearest point of the unit!
	NearestOfUnit(goal,unit->X,unit->Y,&dx,&dy);
	DebugLevel3Fn("Fire to unit at %d,%d\n",dx,dy);

	//
	//	Moved out of attack range?
	//
	if( MapDistanceBetweenUnits(unit,goal)<unit->Type->MinAttackRange ) {
	    DebugLevel0Fn("Missile target too near %d,%d\n"
		,MapDistanceBetweenUnits(unit,goal),unit->Type->MinAttackRange);
	    // FIXME: do something other?
	    return;
	}

    } else {
	dx=unit->Orders[0].X;
	dy=unit->Orders[0].Y;
	// FIXME: Can this be too near??
    }

    // Fire to the tile center of the destination.
    dx=dx*TileSizeX+TileSizeX/2;
    dy=dy*TileSizeY+TileSizeY/2;
    missile=MakeMissile(unit->Type->Missile.Missile,x,y,dx,dy);
    //
    //	Damage of missile
    //
    if( goal ) {
	missile->TargetUnit=goal;
	RefsDebugCheck( !goal->Refs || goal->Destroyed );
	goal->Refs++;
    }
    missile->SourceUnit=unit;
    RefsDebugCheck( !unit->Refs || unit->Destroyed );
    unit->Refs++;
}

/**
**      Get area of tiles covered by missile
**
**      @param missile  Missile to be checked and set.
**      @return         sx,sy,ex,ey defining area in Map
*/
local void GetMissileMapArea( const Missile* missile,
			      int *sx, int *sy, int *ex, int *ey )
{
    *sx=missile->X/TileSizeX;
    *sy=missile->Y/TileSizeY;
    *ex=(missile->X+missile->Type->Width)/TileSizeX;
    *ey=(missile->Y+missile->Type->Height)/TileSizeY;
}

/**
**      Check missile visibility.
**
**      @param missile  Missile pointer to check if visible.
**
**      @return         Returns true if visibile, false otherwise.
*/
local int MissileVisible(const Missile* missile)
{
    int tileMinX;
    int tileMaxX;
    int tileMinY;
    int tileMaxY;

    GetMissileMapArea(missile,&tileMinX,&tileMinY,&tileMaxX,&tileMaxY);
    if ( !AnyAreaVisibleInMap(tileMinX,tileMinY,tileMaxX,tileMaxY) ) {
	return 0;
    }
    DebugLevel3Fn("Missile bounding box %d %d %d %d (Map %d %d %d %d)\n",
		tileMinX,tileMaxX,tileMinY,tileMaxY,
		MapX,MapX+MapWidth,MapY,MapY+MapHeight);
    return 1;
}

/**
**      Check and sets if missile must be drawn on screen-map
**
**      @param missile  Missile to be checked.
**      @return         True if map marked to be drawn, false otherwise.
*/
local int CheckMissileToBeDrawn(const Missile* missile)
{
    int sx,sy,ex,ey;

    GetMissileMapArea( missile, &sx, &sy, &ex, &ey );
    return MarkDrawAreaMap( sx, sy, ex, ey );
}

/**
**	Draw missile.
*/
global void DrawMissile(const MissileType* type,unsigned frame,int x,int y)
{
    // FIXME: This is a hack for mirrored sprites
    if( frame&128 ) {
	VideoDrawClipX(type->Sprite,frame&127,x,y);
    } else {
	VideoDrawClip(type->Sprite,frame,x,y);
    }
}

/**
**	Draw all missiles on map.
*/
global void DrawMissiles(void)
{
    const Missile* missile;
    const Missile* missiles_end;
    int x;
    int y;

    missiles_end=Missiles+NumMissiles;
    for( missile=Missiles; missile<missiles_end; ++missile ) {
	// FIXME: make table of used slots!
	if( missile->Type==MissileFree ) {
	    continue;
	}
	if( missile->Type->Class == MissileClassCustom ) {
	    continue; // custom missiles are handled by Controller() only
	}
	// Draw only visibile missiles
	if (MissileVisible(missile)) {
	    x=missile->X-MapX*TileSizeX+TheUI.MapX;
	    y=missile->Y-MapY*TileSizeY+TheUI.MapY;
	    // FIXME: I should copy SourcePlayer for second level missiles.
	    if( missile->SourceUnit && missile->SourceUnit->Player ) {
		GraphicPlayerPixels(missile->SourceUnit->Player
			,missile->Type->Sprite);
	    }
	    DrawMissile(missile->Type,missile->Frame,x,y);
	}
    }
}

/**
**	Change missile heading from x,y.
**
**	@param missile	Missile pointer.
**	@param dx	Delta in x.
**	@param dy	Delta in y.
*/
local void MissileNewHeadingFromXY(Missile* missile,int dx,int dy)
{
    int dir;

    // FIXME: depends on the missile directions wc 8, sc 32
    missile->Frame&=127;
    missile->Frame/=5;
    missile->Frame*=5;

    dir=((DirectionToHeading(dx,dy)+NextDirection/2)&0xFF)/NextDirection;
    if( dir<=LookingS/NextDirection ) {	// north->east->south
	missile->Frame+=dir;
    } else {
	// Note: 128 is the flag for flip graphic in X.
	missile->Frame+=128+256/NextDirection-dir;
    }
}

/**
**	Handle point to point missile.
*/
local int PointToPointMissile(Missile* missile)
{
    int dx;
    int dy;
    int xstep;
    int ystep;
    int i;

    if( !(missile->State&1) ) {
	// initialize
	dy=missile->DY-missile->Y;
	ystep=1;
	if( dy<0 ) {
	    dy=-dy;
	    ystep=-1;
	}
	dx=missile->DX-missile->X;
	xstep=1;
	if( dx<0 ) {
	    dx=-dx;
	    xstep=-1;
	}

	// FIXME: could be better written
	if( missile->Type->Class == MissileClassWhirlwind ) {
	    // must not call MissileNewHeading nor frame change
	} else if( missile->Type->Class == MissileClassBlizzard ) {
	    missile->Frame = 0;
	} else {
	    MissileNewHeadingFromXY(missile,dx*xstep,dy*ystep);
	}

	if( dy==0 ) {		// horizontal line
	    if( dx==0 ) {
		return 1;
	    }
	} else if( dx==0 ) {	// vertical line
	} else if( dx<dy ) {	// step in vertical direction
	    missile->D=dy-1;
	    dx+=dx;
	    dy+=dy;
	} else if( dx>dy ) {	// step in horizontal direction
	    missile->D=dx-1;
	    dx+=dx;
	    dy+=dy;
	}

	missile->Dx=dx;
	missile->Dy=dy;
	missile->Xstep=xstep;
	missile->Ystep=ystep;
	++missile->State;
	DebugLevel3Fn("Init: %d,%d, %d,%d, =%d\n"
		,dx,dy,xstep,ystep,missile->D);
	return 0;
    } else {
	// on the way
	dx=missile->Dx;
	dy=missile->Dy;
	xstep=missile->Xstep;
	ystep=missile->Ystep;
    }

    //
    //	Move missile
    //
    if( dy==0 ) {		// horizontal line
	for( i=0; i<missile->Type->Speed; ++i ) {
	    if( missile->X==missile->DX ) {
		return 1;
	    }
	    missile->X+=xstep;
	}
	return 0;
    }

    if( dx==0 ) {		// vertical line
	for( i=0; i<missile->Type->Speed; ++i ) {
	    if( missile->Y==missile->DY ) {
		return 1;
	    }
	    missile->Y+=ystep;
	}
	return 0;
    }

    if( dx<dy ) {		// step in vertical direction
	for( i=0; i<missile->Type->Speed; ++i ) {
	    if( missile->Y==missile->DY ) {
		return 1;
	    }
	    missile->Y+=ystep;
	    missile->D-=dx;
	    if( missile->D<0 ) {
		missile->D+=dy;
		missile->X+=xstep;
	    }
	}
	return 0;
    }

    if( dx>dy ) {		// step in horizontal direction
	for( i=0; i<missile->Type->Speed; ++i ) {
	    if( missile->X==missile->DX ) {
		return 1;
	    }
	    missile->X+=xstep;
	    missile->D-=dy;
	    if( missile->D<0 ) {
		missile->D+=dx;
		missile->Y+=ystep;
	    }
	}
	return 0;
    }
				// diagonal line
    for( i=0; i<missile->Type->Speed; ++i ) {
	if( missile->Y==missile->DY ) {
	    return 1;
	}
	missile->X+=xstep;
	missile->Y+=ystep;
    }
    return 0;
}

/**
**	Work for missile hit.
**
**	@param missile	Missile reaching end-point.
*/
global void MissileHit(Missile* missile)
{
    Unit* goal;
    int x;
    int y;

    // FIXME: should I move the PlayMissileSound to here?

    if( missile->Type->ImpactSound.Sound ) {
	PlayMissileSound(missile,missile->Type->ImpactSound.Sound);
    }

    x=missile->X+missile->Type->Width/2;
    y=missile->Y+missile->Type->Height/2;

    //
    //	The impact generates a new missile.
    //
    if( missile->Type->ImpactMissile ) {
	Missile* mis;

	mis = MakeMissile(missile->Type->ImpactMissile,x,y,0,0);
	// Impact missiles didn't generate any damage now.
#if 0
	mis->Damage = missile->Damage; // direct damage, spells mostly
	mis->SourceUnit = missile->SourceUnit;
	// FIXME: should copy target also?
	if( mis->SourceUnit ) {
	    // RefsDebugCheck( mis->SourceUnit->Destroyed );
	    if( mis->SourceUnit->Destroyed ) {
		DebugLevel0Fn("Referencing a destroyed unit, I think it is good here\n");
	    }
	    RefsDebugCheck( !mis->SourceUnit->Refs );
	    mis->SourceUnit->Refs++;
	}
#endif
    }

    if( !missile->SourceUnit ) {	// no owner - green-cross ...
	DebugLevel3Fn("Missile has no owner!\n");
	return;
    }

    // FIXME: what can the missile hit?
    // FIXME: "missile-catapult-rock", "missile-ballista-bolt", have area effect

    x/=TileSizeX;
    y/=TileSizeY;

    if( x<0 || y<0 || x>=TheMap.Width || y>=TheMap.Height ) {
	// FIXME: this should handled by caller?
	DebugLevel0Fn("Missile gone outside of map!\n");
	return;				// outside the map.
    }

    //
    //	Chooce correct goal.
    //
    if( !missile->Type->Range && missile->TargetUnit ) {
	goal=missile->TargetUnit;
	if( goal->Destroyed ) {			// Destroyed
	    RefsDebugCheck( !goal->Refs );
	    if( !--goal->Refs ) {
		ReleaseUnit(goal);
	    }
	    missile->TargetUnit=NoUnitP;
	    return;
	}
    } else {
	// FIXME: hits all or only enemies?
	goal=UnitOnMapTile(x,y);
    }

    if( !goal || !goal->HP ) {
	if( WallOnMap(x,y) ) {
	    DebugLevel3Fn("Missile on wall?\n");
	    // FIXME: don't use UnitTypeByIdent here, this is slow!
	    if( HumanWallOnMap(x,y) ) {
		if ( missile->Damage ) {	// direct damage, spells mostly
		    HitWall(x,y,missile->Damage);
		} else {
		    HitWall(x,y,CalculateDamageStats(missile->SourceUnit->Stats,
			UnitTypeByIdent("unit-human-wall")->Stats,0));
		}
	    } else {
		if ( missile->Damage ) {	// direct damage, spells mostly
		    HitWall(x,y,missile->Damage);
		} else {
		    HitWall(x,y,CalculateDamageStats(missile->SourceUnit->Stats,
			UnitTypeByIdent("unit-orc-wall")->Stats,0));
		}
	    }
	    return;
	}
	DebugLevel3Fn("Oops nothing to hit (%d,%d)?\n",x,y);
	return;
    }

    if ( BlizzardMissileHit && goal == missile->SourceUnit ) {
	return;		// blizzard cannot hit owner unit
    }
    BlizzardMissileHit = 0;

    if ( missile->Damage ) {	// direct damage, spells mostly
	HitUnit(goal,missile->Damage);
    } else {
	HitUnit(goal,CalculateDamage(missile->SourceUnit->Stats,goal,
	    missile->SourceUnit->Bloodlust));
    }
}

/**
**	Handle all missile actions.
*/
global void MissileActions(void)
{
    Missile* missile;
    const Missile* missiles_end;

    missiles_end=Missiles+NumMissiles;
    for( missile=Missiles; missile<missiles_end; ++missile ) {
	if( missile->Type==MissileFree ) {
	    continue;
	}
	if( missile->Wait-- ) {
	    continue;
	}

	if ( missile->TTL != -1 ) {
	    missile->TTL--;	// overall time to live if specified
	}

	if ( missile->Controller ) {
	    missile->Controller( missile );
	}

	if ( missile->TTL == 0 ) {
	    FreeMissile(missile);
	    continue;
	}

	if ( missile->Type->Class == MissileClassCustom ) {
	    missile->Wait=missile->Type->Sleep;
	    continue;	// custom missiles are handled by Controller() only
	}

	CheckMissileToBeDrawn(missile); //StephanR FIXME:needed here?

	switch( missile->Type->Class ) {
	    //
	    //	Missile flies from x,y to x1,y1
	    //
	    case MissileClassPointToPoint:
		missile->Wait=missile->Type->Sleep;
		if( PointToPointMissile(missile) ) {
		    MissileHit(missile);
		    FreeMissile(missile);
		} else {
		    //
		    //	Animate missile, cycle through frames
		    //
		    missile->Frame+=5;		// FIXME: frames pro row
		    if( (missile->Frame&127)
			    >=VideoGraphicFrames(missile->Type->Sprite) ) {
			missile->Frame-=
				VideoGraphicFrames(missile->Type->Sprite);
		    }
		    DebugLevel3Fn("Frame %d of %d\n"
			    ,missile->Frame
			    ,VideoGraphicFrames(missile->Type->Sprite));
		}
		break;

	    case MissileClassPointToPointWithDelay:
		missile->Wait=missile->Type->Sleep;
		if( PointToPointMissile(missile) ) {
		    MissileHit(missile);
		    FreeMissile(missile);
		} else {
		    //
		    //	Animate missile, depends on the way.
		    //		FIXME: becomes bigger than smaller.
		    // FIXME: how?
		}
		break;

	    case MissileClassPointToPoint3Bounces:
		missile->Wait=missile->Type->Sleep;
		if( PointToPointMissile(missile) ) {
		    //
		    //	3 Bounces.
		    //
		    switch( missile->State ) {
			case 1:
			case 3:
			case 5:
			    missile->State+=2;
			    missile->DX+=missile->Xstep*TileSizeX*2;
			    missile->DY+=missile->Ystep*TileSizeY*2;
			    MissileHit(missile);
			    // FIXME: hits to left and right
			    // FIXME: reduce damage effects on later impacts
			    break;
			default:
			    FreeMissile(missile);
			    break;
		    }
		} else {
		    //
		    //	Animate missile, cycle through frames
		    //
		    missile->Frame+=5;		// FIXME: frames pro row
		    if( (missile->Frame&127)
			    >=VideoGraphicFrames(missile->Type->Sprite) ) {
			missile->Frame-=
				VideoGraphicFrames(missile->Type->Sprite);
		    }
		    DebugLevel3Fn("Frame %d of %d\n"
			    ,missile->Frame
			    ,VideoGraphicFrames(missile->Type->Sprite));
		}
		break;

	    case MissileClassPointToPointWithHit:
		missile->Wait=missile->Type->Sleep;
		if( PointToPointMissile(missile) ) {
		    //
		    //	Animate hit
		    //
		    missile->Frame+=5;	// FIXME: frames pro row
		    if( (missile->Frame&127)
			    >=VideoGraphicFrames(missile->Type->Sprite) ) {
			MissileHit(missile);
			FreeMissile(missile);
		    }
		}
		break;

	    case MissileClassBlizzard:
		missile->Wait=missile->Type->Sleep;
		if( PointToPointMissile(missile) ) {
		    //
		    //	Animate hit
		    //
		    missile->Frame+=4;	// FIXME: frames pro row
		    if( (missile->Frame&127)
			    >=VideoGraphicFrames(missile->Type->Sprite) ) {
			//NOTE: vladi: blizzard cannot hit owner...
			BlizzardMissileHit = 1;
			MissileHit(missile);
			FreeMissile(missile);
		    }
		}
		break;

	    case MissileClassDeathDecay:
		//NOTE: vladi: this is exact copy of MissileClassStayWithDelay
		// but with check for blizzard-type hit (friendly fire:))
		missile->Wait=missile->Type->Sleep;
		if( ++missile->Frame
			==VideoGraphicFrames(missile->Type->Sprite) ) {
		    BlizzardMissileHit = 1;
		    MissileHit(missile);
		    FreeMissile(missile);
		}
		break;

	    case MissileClassWhirlwind:
		missile->Wait=missile->Type->Sleep;
		missile->Frame++;
		if ( missile->Frame > 3 )
		  missile->Frame = 0;
		//NOTE: vladi: whirlwind moves slowly, i.e. it stays
		//      5 ticks at the same pixels...
		if ( missile->TTL < 1 || missile->TTL % 10 == 0 )
		  PointToPointMissile(missile);
		break;

	    case MissileClassStayWithDelay:
		missile->Wait=missile->Type->Sleep;
		if( ++missile->Frame
			==VideoGraphicFrames(missile->Type->Sprite) ) {
		    MissileHit(missile);
		    FreeMissile(missile);
		    // FIXME: should MissileHitAndFree();
		}
		break;

	    case MissileClassCycleOnce:
		missile->Wait=missile->Type->Sleep;
		switch( missile->State ) {
		    case 0:
		    case 2:
			++missile->State;
			break;
		    case 1:
			if( ++missile->Frame
				==VideoGraphicFrames(missile->Type->Sprite) ) {
			    --missile->Frame;
			    ++missile->State;
			}
			break;
		    case 3:
			if( !missile->Frame-- ) {
			    MissileHit(missile);
			    FreeMissile(missile);
			}
			break;
		}
		break;

	    case MissileClassFire:
		missile->Wait=missile->Type->Sleep;
		if( ++missile->Frame
			==VideoGraphicFrames(missile->Type->Sprite) ) {
		    int f;
		    Unit* unit;

		    unit=missile->SourceUnit;
		    if( unit->Destroyed || !unit->HP ) {
			FreeMissile(missile);
			break;
		    }
		    missile->Frame=0;
		    f=(100*unit->HP)/unit->Stats->HitPoints;
		    if( f>75) {
			FreeMissile(missile);
			unit->Burning=0;
		    } else if( f>50 ) {
			if( missile->Type!=MissileTypeSmallFire ) {
			    missile->X+=missile->Type->Width/2;
			    missile->Y+=missile->Type->Height/2;
			    missile->Type=MissileTypeSmallFire;
			    missile->X-=missile->Type->Width/2;
			    missile->Y-=missile->Type->Height/2;
			}
		    } else {
			if( missile->Type!=MissileTypeBigFire ) {
			    missile->X+=missile->Type->Width/2;
			    missile->Y+=missile->Type->Height/2;
			    missile->Type=MissileTypeBigFire;
			    missile->X-=missile->Type->Width/2;
			    missile->Y-=missile->Type->Height/2;
			}
		    }
		}
		break;

	}

	if (missile->Type!=MissileFree) {
	    // check after movement
	    CheckMissileToBeDrawn(missile);
	}
    }
}

/**
**	Calculate distance from view-point to missle.
**
**	@param missile	Missile pointer for distance.
*/
global int ViewPointDistanceToMissile(const Missile* missile)
{
    int x;
    int y;

    x=(missile->X+missile->Type->Width/2)/TileSizeX;
    y=(missile->Y+missile->Type->Height/2)/TileSizeY;	// pixel -> tile

    DebugLevel3Fn("Missile %p at %d %d\n",missile,x,y);

    return ViewPointDistance(x,y);
}

/**
**	Save the missile-types to file.
**
**	@param file	Output file.
*/
global void SaveMissileTypes(FILE* file)
{
    MissileType* mt;
    char** sp;
    int i;

    fprintf(file,"\n;;; -----------------------------------------\n");
    fprintf(file,";;; MODULE: missile-types $Id$\n\n");

    i=fprintf(file,"(define-missiletype-wc-names");
    for( sp=MissileTypeWcNames; *sp; ++sp ) {
	if( i+strlen(*sp)>79 ) {
	    i=fprintf(file,"\n ");
	}
	i+=fprintf(file," '%s",*sp);
    }
    fprintf(file,")\n\n");


    for( mt=MissileTypes; mt<&MissileTypes[NumMissileTypes]; ++mt ) {
	fprintf(file,"(define-missile-type '%s\n ",mt->Ident);
	if( mt->File ) {
	    fprintf(file," 'file \"%s\"",mt->File);
	}
	fprintf(file," 'size '(%d %d)",mt->Width,mt->Height);
	if( mt->Sprite ) {
	    fprintf(file," 'frames %d",VideoGraphicFrames(mt->Sprite));
	}
	fprintf(file,"\n ");
	if( mt->FiredSound.Name ) {
	    fprintf(file," 'fired-sound \"%s\"",mt->FiredSound.Name);
	}
	if( mt->ImpactSound.Name ) {
	    fprintf(file," 'impact-sound \"%s\"",mt->ImpactSound.Name);
	}
	if( mt->FiredSound.Name || mt->ImpactSound.Name ) {
	    fprintf(file,"\n ");
	}
	fprintf(file," 'class '%s",MissileClassNames[mt->Class]);
	fprintf(file," 'sleep %d",mt->Sleep);
	fprintf(file," 'speed %d",mt->Speed);
	fprintf(file," 'range %d",mt->Range);
	if( mt->ImpactMissile ) {
	    fprintf(file,"\n  'impact-missile '%s",mt->ImpactMissile->Ident);
	}
	fprintf(file,")\n");
    }
}

/**
**	Save the state of a missile to file.
*/
local void SaveMissile(const Missile* missile,FILE* file)
{
    char* s1;
    extern char* UnitReference(const Unit*);

    fprintf(file,"(missile 'type '%s",
	missile->Type->Ident);
    fprintf(file," 'pos (%d %d) 'goal (%d %d)",
	missile->X,missile->Y,missile->DX,missile->DY);
    fprintf(file,"\n  'frame %d 'state %d 'wait %d\n ",
	missile->Frame,missile->State,missile->Wait);
    if( missile->SourceUnit ) {
	fprintf(file," 'source %s",s1=UnitReference(missile->SourceUnit));
	free(s1);
    }
    if( missile->TargetUnit ) {
	fprintf(file," 'target %s",s1=UnitReference(missile->TargetUnit));
	free(s1);
    }
    fprintf(file," 'damage %d",missile->Damage);
    fprintf(file,"\n  'data (%d %d %d %d %d)",
	missile->D,missile->Dx,missile->Dy,missile->Xstep,missile->Ystep);
    // FIXME: need symbolic names for controller
    fprintf(file," 'ttl %d 'controller %ld",
	missile->TTL,(long)missile->Controller);
    fprintf(file,")\n");
}

/**
**	Save the state missiles to file.
**
**	@param file	Output file.
*/
global void SaveMissiles(FILE* file)
{
    Missile* missile;

    fprintf(file,"\n;;; -----------------------------------------\n");
    fprintf(file,";;; MODULE: missiles $Id$\n\n");

    for( missile=Missiles; missile<&Missiles[NumMissiles]; ++missile ) {
	if( missile->Type!=MissileFree ) {
	    SaveMissile(missile,file);
	}
    }
}

//@}
