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
//	(c) Copyright 1998-2000 by Lutz Sammer
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
**	Missile does nothing
*/
#define MissileClassNone			0
/**
**	Missile flies from x,y to x1,y1
*/
#define MissileClassPointToPoint		1
/**
**	Missile flies from x,y to x1,y1 and stays there for a moment
*/
#define MissileClassPointToPointWithDelay	2
/**
**	Missile don't move, than disappears
*/
#define MissileClassStayWithDelay		3
/**
**	Missile flies from x,y to x1,y1 than bounces three times.
*/
#define MissileClassPointToPoint3Bounces	4
/**
**	Missile flies from x,y to x1,y1 than changes into flame shield
*/
#define MissileClassFireball			5
/**
**	Missile surround x,y
*/
#define MissileClassFlameShield			6
/**
**	Missile appears at x,y, is blizzard
*/
#define MissileClassBlizzard			7
/**
**	Missile appears at x,y, is death and decay
*/
#define MissileClassDeathDecay			8
/**
**	Missile appears at x,y, is whirlwind
*/
#define MissileClassWhirlwind			9
/**
**	Missile appears at x,y, than cycle through the frames up and down.
*/
#define MissileClassCycleOnce			10
/**
**	Missile flies from x,y to x1,y1 than shows hit animation.
*/
#define MissileClassPointToPointWithHit		11
/**
**	Missile don't move, than checks the source unit for HP.
*/
#define MissileClassFire			12

/*----------------------------------------------------------------------------
--	Variables
----------------------------------------------------------------------------*/

/**
**	W*rCr*ft number to internal missile-type name.
**
**	Should be made configurable.
*/
local const char* MissileTypeWcNames[] = {
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
};

/**
**	Missile type type definition
*/
global char MissileTypeType[] = "missile-type";

/**
**	Define missile types.
*/
global MissileType MissileTypes[MissileTypeMax] = {
{ MissileTypeType,
    "missile-lightning",
    "lightning.png",
    32,32,
    { NULL },
    MissileClassPointToPointWithHit,
    1,
    },
{ MissileTypeType,
    "missile-griffon-hammer",
    "gryphon hammer.png",
    32,32,
    { "fireball hit" },
    MissileClassPointToPoint3Bounces,
    1,
    "missile-explosion",	NULL,
    },
{ MissileTypeType,
    "missile-dragon-breath",
    "dragon breath.png",
    32,32,
    { "fireball hit" },
    MissileClassPointToPoint3Bounces,
    1,
    "missile-explosion",	NULL,
    },
{ MissileTypeType,
    "missile-fireball",
    "fireball.png",
    32,32,
    { "fireball hit" },
    MissileClassFireball,	
    1,
    },
{ MissileTypeType,
    "missile-flame-shield",
    "flame shield.png",
    32,32,
    { NULL },
    MissileClassFlameShield,	
    1,
    },
{ MissileTypeType,
    "missile-blizzard",
    "blizzard.png",
    32,32,
    { NULL },
    MissileClassBlizzard,	
    1,
    },
{ MissileTypeType,
    "missile-death-and-decay",
    "death and decay.png",
    32,32,
    { NULL },
    MissileClassDeathDecay,	
    1,
    },
{ MissileTypeType,
    "missile-big-cannon",
    "big cannon.png",
    16,16,
    { "explosion" },
    MissileClassPointToPoint,	
    1,
    "missile-cannon-tower-explosion",	NULL,
    },
{ MissileTypeType,
    "missile-exorcism",
    "exorcism.png",
    48,48,
    { NULL },
    MissileClassPointToPoint,	
    1,
    },
{ MissileTypeType,
    "missile-heal-effect",
    "heal effect.png",
    48,48,
    { NULL },
    MissileClassPointToPoint,	
    1,
    },
{ MissileTypeType,
    "missile-touch-of-death",
    "touch of death.png",
    32,32,
    { NULL },
    MissileClassPointToPointWithHit,
    1,
    },
{ MissileTypeType,
    "missile-rune",
    "rune.png",
    16,16,
    { NULL },
    MissileClassStayWithDelay,	
    1,
    },
{ MissileTypeType,
    "missile-whirlwind",
    "tornado.png",
    56,56,
    { NULL },
    MissileClassWhirlwind,	
    1,
    },
{ MissileTypeType,
    "missile-catapult-rock",
    "catapult rock.png",
    32,32,
    { "explosion" },
    MissileClassPointToPointWithDelay,
    1,
    "missile-impact",	NULL,
    },
{ MissileTypeType,
    "missile-ballista-bolt",
    "ballista bolt.png",
    64,64,
    { "explosion" },
    MissileClassPointToPoint,	
    1,
    "missile-impact",	NULL,
    },
{ MissileTypeType,
    "missile-arrow",
    "arrow.png",
    40,40,
    { "bow-hit" },
    MissileClassPointToPoint,	
    1,
    },
{ MissileTypeType,
    "missile-axe",
    "axe.png",
    32,32,
    { "bow-hit" },
    MissileClassPointToPoint,	
    1,
    },
{ MissileTypeType,
    "missile-submarine-missile",
    "submarine missile.png",
    40,40,
    { "explosion" },
    MissileClassPointToPoint,	
    1,
    "missile-impact",	NULL,
    },
{ MissileTypeType,
    "missile-turtle-missile",
    "turtle missile.png",
    40,40,
    { "explosion" },
    MissileClassPointToPoint,	
    1,
    "missile-impact",	NULL,
    },
{ MissileTypeType,
    "missile-small-fire",
    "small fire.png",
    32,48,
    { NULL },
    MissileClassFire,	
    8,
    },
{ MissileTypeType,
    "missile-big-fire",
    "big fire.png",
    48,48,
    { NULL },
    MissileClassFire,	
    8,
    },
{ MissileTypeType,
    "missile-impact",
    "ballista-catapult impact.png",
    48,48,
    { NULL },
    MissileClassStayWithDelay,	
    1,
    },
{ MissileTypeType,
    "missile-normal-spell",
    "normal spell.png",
    32,32,
    { NULL },
    MissileClassStayWithDelay,	
    1,
    },
{ MissileTypeType,
    "missile-explosion",
    "explosion.png",
    64,64,
    { NULL },
    MissileClassStayWithDelay,	
    1,
    },
{ MissileTypeType,
    "missile-small-cannon",
    "cannon.png",
    32,32,
    { "explosion" },
    MissileClassPointToPointWithDelay,
    1,
    "missile-cannon-explosion",	NULL,
    },
{ MissileTypeType,
    "missile-cannon-explosion",
    "cannon explosion.png",
    32,32,
    { NULL },
    MissileClassStayWithDelay,	
    1,
    },
{ MissileTypeType,
    "missile-cannon-tower-explosion",
    "cannon-tower explosion.png",
    32,32,
    { NULL },
    MissileClassStayWithDelay,	
    1,
    },
{ MissileTypeType,
    "missile-daemon-fire",
    "daemon fire.png",
    32,32,
    { NULL },
    MissileClassPointToPoint,	
    1,
    },
{ MissileTypeType,
    "missile-green-cross",
    "green cross.png",
    32,32,
    { NULL },
    MissileClassCycleOnce,
    FRAMES_PER_SECOND/30
    },
{ MissileTypeType,
    "missile-none",
    NULL,
    32,32,
    { NULL },
    MissileClassNone,		
    1,
    },
};

/*
**	Next missile types are used hardcoded in the source.
*/
global MissileType* MissileTypeSmallFire;	/// Small fire missile type
global MissileType* MissileTypeBigFire;		/// Big fire missile type

#define MAX_MISSILES	1800		/// maximum number of missiles

local int NumMissiles;			/// currently used missiles
local Missile Missiles[MAX_MISSILES];	/// all missiles on map

    /// lookup table for missile names
local hashtable(MissileType*,61) MissileHash;

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

    for( i=0; i<sizeof(MissileTypes)/sizeof(*MissileTypes); ++i ) {
	if( (file=MissileTypes[i].File) ) {
	    char* buf;

	    buf=alloca(strlen(file)+9+1);
	    file=strcat(strcpy(buf,"graphic/"),file);
	    ShowLoadProgress("\tMissile %s\n",file);
	    MissileTypes[i].Sprite=LoadSprite(
		    file,MissileTypes[i].Width,MissileTypes[i].Height);
	}
    }

    //
    //	Add missile names to hash table
    //
    for( i=0; i<sizeof(MissileTypes)/sizeof(*MissileTypes); ++i ) {
	*(MissileType**)hash_add(MissileHash,MissileTypes[i].Ident)
		=&MissileTypes[i];
    }

    //
    //	Resolve impact missiles.
    //
    for( i=0; i<sizeof(MissileTypes)/sizeof(*MissileTypes); ++i ) {
	if( MissileTypes[i].ImpactName ) {
	    MissileTypes[i].ImpactMissile
		    =MissileTypeByIdent(MissileTypes[i].ImpactName);
	}
    }

    MissileTypeSmallFire=MissileTypeByIdent("missile-small-fire");
    // FIXME: FIXME: FIXME: very diry hack
    MissileTypeSmallFire->Sprite->NumFrames=6;
    MissileTypeBigFire=MissileTypeByIdent("missile-big-fire");
}

/**
**	Get Missile type by identifier.
**
**	@param ident	Identifier.
**
**	@returns	Missile type pointer.
*/
global MissileType* MissileTypeByIdent(const char* ident)
{
    MissileType** type;

    type=(MissileType**)hash_find(MissileHash,(char*)ident);

    if( type ) { 
	return *type;
    }

    DebugLevel0Fn("Missile %s not found\n",ident);
    return NULL;
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
**	@returns	created missile.
*/
global Missile* MakeMissile(MissileType* type,int sx,int sy,int dx,int dy)
{
    Missile* missile;

    DebugLevel3Fn("type %Zd(%s) at %d,%d to %d,%d\n"
	    ,type-MissileTypes,type->Name,sx,sy,dx,dy);

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
    missile->SourceType=NULL;
    missile->SourceStats=NULL;
    missile->SourcePlayer=NULL;

    return missile;
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
**	NOTE: hiden targets are hit worser.
**	NOTE: targets higher are hit worser.
**
**	@param attack_stats	Attacker attributes.
**	@param goal_stats	Goal attributes.
**
**	@returns	damage produces on goal.
*/
local int CalculateDamageStats(const UnitStats* attacker_stats
	,const UnitStats* goal_stats)
{
    int damage;

    damage=-goal_stats->Armor;
    damage+=attacker_stats->BasicDamage;
    if( damage<0 ) {
	damage=0;
    }
    damage+=attacker_stats->PiercingDamage+1;	// round up
    damage/=2;
    damage*=((SyncRand()>>15)&1)+1;
    DebugLevel3Fn("Damage done %d\n",damage);

    return damage;
}

/**
**	Calculate damage.
**
**	@param attack_stats	Attacker attributes.
**	@param goal		Goal unit.
**	@returns	damage produces on goal.
*/
local int CalculateDamage(const UnitStats* attacker_stats,const Unit* goal)
{
    return CalculateDamageStats(attacker_stats,goal->Stats);
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
    //	None missile hits immediately!
    //
    if( ((MissileType*)unit->Type->Missile.Missile)->Class==MissileClassNone ) {
	if( !(goal=unit->Command.Data.Move.Goal) ) {
	    dx=unit->Command.Data.Move.DX;
	    dy=unit->Command.Data.Move.DY;
	    if( WallOnMap(dx,dy) ) {
		if( HumanWallOnMap(dx,dy) ) {
		    // FIXME: don't use UnitTypeByIdent here, this is slow!
		    HitWall(dx,dy,CalculateDamageStats(unit->Stats,
			    UnitTypeByIdent("unit-human-wall")->Stats));
		} else {
		    // FIXME: don't use UnitTypeByIdent here, this is slow!
		    HitWall(dx,dy,CalculateDamageStats(unit->Stats,
			    UnitTypeByIdent("unit-orc-wall")->Stats));
		}
		return;
	    }

	    DebugLevel1Fn("Missile-none hits no unit, shouldn't happen!\n");
	    return;
	}

	// FIXME: make sure thats the correct unit.

	// Check if goal is correct unit.
	if( goal->Destroyed ) {
	    DebugLevel0Fn("destroyed unit\n");
	    if( !--goal->Refs ) {
		ReleaseUnit(goal);
	    }
	    // FIXME: should I clear this here?
	    unit->Command.Data.Move.Goal=NULL;
	    return;
	}
	if( goal->Removed ) {
	    DebugLevel3Fn("Missile-none hits removed unit!\n");
	    --goal->Refs;
	    unit->Command.Data.Move.Goal=NULL;
	    return;
	}
	if( !goal->HP || goal->Command.Action==UnitActionDie ) {
	    DebugLevel3Fn("Missile-none hits dead unit!\n");
	    --goal->Refs;
	    unit->Command.Data.Move.Goal=NULL;
	    return;
	}

	HitUnit(goal,CalculateDamage(unit->Stats,goal));

	return;
    }

    // FIXME: goal is already dead, but missile could hit others?

    x=unit->X*TileSizeX+TileSizeX/2;	// missile starts in tile middle
    y=unit->Y*TileSizeY+TileSizeY/2;
    if( (goal=unit->Command.Data.Move.Goal) ) {
	// Check if goal is correct unit.
	if( goal->Destroyed ) {
	    DebugLevel0Fn("destroyed unit\n");
	    if( !--goal->Refs ) {
		ReleaseUnit(goal);
	    }
	    // FIXME: should I clear this here?
	    unit->Command.Data.Move.Goal=NULL;
	    return;
	}
	DebugCheck( !goal->Type );	// Target invalid?
	// Fire to nearest point of unit!
	NearestOfUnit(goal,unit->X,unit->Y,&dx,&dy);
	DebugLevel3Fn("Fire to unit at %d,%d\n",dx,dy);
    } else {
	dx=unit->Command.Data.Move.DX;
	dy=unit->Command.Data.Move.DY;
    }

    //
    //	Moved out of attack range?
    //
    if( MapDistance(unit->X,unit->Y,dx,dy)<=unit->Type->MinAttackRange ) {
	DebugLevel0Fn("Missile target too near %d,%d\n"
	    ,MapDistance(unit->X,unit->Y,dx,dy),unit->Type->MinAttackRange);
	// FIXME: do something other?
	return;
    }

    dx=dx*TileSizeX+TileSizeX/2;
    dy=dy*TileSizeY+TileSizeY/2;
    missile=MakeMissile(unit->Type->Missile.Missile,x,y,dx,dy);
    //
    //	Damage of missile
    //
    missile->SourceUnit=unit;
    // unit->Refs++; Reference currently not used.
    missile->SourceType=unit->Type;
    missile->SourceStats=unit->Stats;
    missile->SourcePlayer=unit->Player;
}

/**
**	Check missile visibility.
**
**	@param missile	Missile pointer to check if visible.
**
**	@return		Returns true if visibile, false otherwise.
*/
local int MissileVisible(const Missile* missile)
{
    int tileMinX;
    int tileMaxX;
    int tileMinY;
    int tileMaxY;

    tileMinX=missile->X/TileSizeX;
    tileMinY=missile->Y/TileSizeY;
    tileMaxX=(missile->X+missile->Type->Width)/TileSizeX;
    tileMaxY=(missile->Y+missile->Type->Height)/TileSizeY;
    if ( (tileMinX>(MapX+MapWidth)) || (tileMaxX<MapX)
	    || (tileMinY>MapY+MapHeight) || (tileMaxY<MapY)) {
	return 0;
    }
    DebugLevel3Fn("Missile bounding box %d %d %d %d (Map %d %d %d %d)\n",
		tileMinX,tileMaxX,tileMinY,tileMaxY,
		MapX,MapX+MapWidth,MapY,MapY+MapHeight);
    return 1;
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
    Missile* missile;
    Missile* missiles_end;
    int x;
    int y;

    missiles_end=Missiles+NumMissiles;
    for( missile=Missiles; missile<missiles_end; ++missile ) {
	// FIXME: make table of used slots!
	if( missile->Type==MissileFree ) {
	    continue;
	}
	// Draw only visibile missiles
	if (MissileVisible(missile)) {
	    x=missile->X-MapX*TileSizeX+TheUI.MapX;
	    y=missile->Y-MapY*TileSizeY+TheUI.MapY;
	    // FIXME: I should copy SourcePlayer for second level missiles.
	    if( missile->SourcePlayer ) {
		GraphicPlayerPixels(missile->SourcePlayer
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

#define MISSILE_STEPS	16		// How much did a missile move??

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
	MissileNewHeadingFromXY(missile,dx*xstep,dy*ystep);

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
	for( i=0; i<MISSILE_STEPS; ++i ) {
	    if( missile->X==missile->DX ) {
		return 1;
	    }
	    missile->X+=xstep;
	}
	return 0;
    }

    if( dx==0 ) {		// vertical line
	for( i=0; i<MISSILE_STEPS; ++i ) {
	    if( missile->Y==missile->DY ) {
		return 1;
	    }
	    missile->Y+=ystep;
	}
	return 0;
    }

    if( dx<dy ) {		// step in vertical direction
	for( i=0; i<MISSILE_STEPS; ++i ) {
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
	for( i=0; i<MISSILE_STEPS; ++i ) {
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
    for( i=0; i<MISSILE_STEPS; ++i ) {
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
*/
global void MissileHit(const Missile* missile)
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
    if( missile->Type->ImpactMissile ) {
	MakeMissile(missile->Type->ImpactMissile,x,y,0,0);
    }
    if( !missile->SourceType ) {	// no target
	return;
    }

    // FIXME: must choose better goal!
    // FIXME: what can the missile hit?
    // FIXME: "missile-catapult-rock", "missile-ballista-bolt", have are effect

    x/=TileSizeX;
    y/=TileSizeY;
    goal=UnitOnMapTile(x,y);
    if( !goal || !goal->HP ) {
	if( WallOnMap(x,y) ) {
	    DebugLevel3Fn("Missile on wall?\n");
	    // FIXME: don't use UnitTypeByIdent here, this is slow!
	    if( HumanWallOnMap(x,y) ) {
		HitWall(x,y,CalculateDamageStats(missile->SourceStats,
			UnitTypeByIdent("unit-human-wall")->Stats));
	    } else {
		HitWall(x,y,CalculateDamageStats(missile->SourceStats,
			UnitTypeByIdent("unit-orc-wall")->Stats));
	    }
	    return;
	}
	DebugLevel3Fn("Oops nothing to hit (%d,%d)?\n",x,y);
	return;
    }

    HitUnit(goal,CalculateDamage(missile->SourceStats,goal));
}

/**
**	Handle all missile actions.
*/
global void MissileActions(void)
{
    Missile* missile;
    Missile* missiles_end;

    missiles_end=Missiles+NumMissiles;
    for( missile=Missiles; missile<missiles_end; ++missile ) {
	if( missile->Type==MissileFree ) {
	    continue;
	}
	if( missile->Wait-- ) {
	    continue;
	}
	if (MissileVisible(missile)) {
	    // check before movement
	    MustRedraw|=RedrawMap;
	    // FIXME: must mark the exact tile, for more optimazation
	}

	switch( missile->Type->Class ) {
	    //
	    //	Missile flies from x,y to x1,y1
	    //
	    case MissileClassPointToPoint:
		missile->Wait=missile->Type->Speed;
		if( PointToPointMissile(missile) ) {
		    MissileHit(missile);
		    missile->Type=MissileFree;
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
		missile->Wait=missile->Type->Speed;
		if( PointToPointMissile(missile) ) {
		    MissileHit(missile);
		    missile->Type=MissileFree;
		} else {
		    //
		    //	Animate missile, depends on the way.
		    //		FIXME: becomes bigger than smaller.
		    // FIXME: how?
		}
		break;

	    case MissileClassPointToPoint3Bounces:
		missile->Wait=missile->Type->Speed;
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
			    missile->Type=MissileFree;
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
		missile->Wait=missile->Type->Speed;
		if( PointToPointMissile(missile) ) {
		    //
		    //	Animate hit
		    //
		    missile->Frame+=5;	// FIXME: frames pro row
		    if( (missile->Frame&127)
			    >=VideoGraphicFrames(missile->Type->Sprite) ) {
			MissileHit(missile);
			missile->Type=MissileFree;
		    }
		}
		break;

	    case MissileClassStayWithDelay:
		missile->Wait=missile->Type->Speed;
		if( ++missile->Frame
			==VideoGraphicFrames(missile->Type->Sprite) ) {
		    MissileHit(missile);
		    missile->Type=MissileFree;
		}
		break;

	    case MissileClassCycleOnce:
		missile->Wait=missile->Type->Speed;
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
			    missile->Type=MissileFree;
			}
			break;
		}
		break;

	    case MissileClassFire:
		missile->Wait=missile->Type->Speed;
		if( ++missile->Frame
			==VideoGraphicFrames(missile->Type->Sprite) ) {
		    int f;
		    Unit* unit;

		    unit=missile->SourceUnit;
		    if( unit->Destroyed || !unit->HP ) {
			if( !--unit->Refs ) {
			    ReleaseUnit(unit);
			}
			missile->Type=MissileFree;
			break;
		    }
		    missile->Frame=0;
		    f=(100*unit->HP)/unit->Stats->HitPoints;
		    if( f>75) {
			missile->Type=MissileFree;	// No fire for this
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

	if (missile->Type!=MissileFree && MissileVisible(missile)) {
	    // check after movement
	    MustRedraw|=RedrawMap;
	    // FIXME: must mark the exact tile, for more optimazation
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

//@}
