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

/*----------------------------------------------------------------------------
--	Variables
----------------------------------------------------------------------------*/

/**
**	W*rCr*ft number to internal missile-type name.
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
global MissileType MissileTypes[] = {
{ MissileTypeType,
    "missile-lightning",
    "lightning.png",
    32,32,
    { NULL },
    MissileClassPointToPointWithHit,	},
{ MissileTypeType,
    "missile-griffon-hammer",
    "gryphon hammer.png",
    32,32,
    { "fireball hit" },
    MissileClassPointToPoint3Bounces,	},
{ MissileTypeType,
    "missile-dragon-breath",
    "dragon breath.png",
    32,32,
    { "fireball hit" },
    MissileClassPointToPoint3Bounces,	},
{ MissileTypeType,
    "missile-fireball",
    "fireball.png",
    32,32,
    { "fireball hit" },
    MissileClassFireball,		},
{ MissileTypeType,
    "missile-flame-shield",
    "flame shield.png",
    32,32,
    { NULL },
    MissileClassFlameShield,		},
{ MissileTypeType,
    "missile-blizzard",
    "blizzard.png",
    32,32,
    { NULL },
    MissileClassBlizzard,		},
{ MissileTypeType,
    "missile-death-and-decay",
    "death and decay.png",
    32,32,
    { NULL },
    MissileClassDeathDecay,		},
{ MissileTypeType,
    "missile-big-cannon",
    "big cannon.png",
    16,16,
    { "explosion" },
    MissileClassPointToPoint,		},
{ MissileTypeType,
    "missile-exorcism",
    "exorcism.png",
    32,32,
    { NULL },
    MissileClassPointToPoint,		},
{ MissileTypeType,
    "missile-heal-effect",
    "heal effect.png",
    32,32,
    { NULL },
    MissileClassPointToPoint,		},
{ MissileTypeType,
    "missile-touch-of-death",
    "touch of death.png",
    32,32,
    { NULL },
    MissileClassPointToPointWithHit,	},
{ MissileTypeType,
    "missile-rune",
    "rune.png",
    32,32,
    { NULL },
    MissileClassStayWithDelay,		},
{ MissileTypeType,
    "missile-whirlwind",
    "tornado.png",
    56,56,
    { NULL },
    MissileClassWhirlwind,		},
{ MissileTypeType,
    "missile-catapult-rock",
    "catapult rock.png",
    32,32,
    { "explosion" },
    MissileClassPointToPointWithDelay,	},
{ MissileTypeType,
    "missile-ballista-bolt",
    "ballista bolt.png",
    64,64,
    { "explosion" },
    MissileClassPointToPoint,		},
{ MissileTypeType,
    "missile-arrow",
    "arrow.png",
    40,40,
    { "bow-hit" },
    MissileClassPointToPoint,		},
{ MissileTypeType,
    "missile-axe",
    "axe.png",
    32,32,
    { "bow-hit" },
    MissileClassPointToPoint,		},
{ MissileTypeType,
    "missile-submarine-missile",
    "submarine missile.png",
    40,40,
    { "explosion" },
    MissileClassPointToPoint,		},
{ MissileTypeType,
    "missile-turtle-missile",
    "turtle missile.png",
    40,40,
    { "explosion" },
    MissileClassPointToPoint,		},
{ MissileTypeType,
    "missile-small-fire",
    "small fire.png",
    32,48,
    { NULL },
    MissileClassStayWithDelay,		},
{ MissileTypeType,
    "missile-big-fire",
    "big fire.png",
    48,48,
    { NULL },
    MissileClassStayWithDelay,		},
{ MissileTypeType,
    "missile-impact",
    "ballista-catapult impact.png",
    48,48,
    { NULL },
    MissileClassStayWithDelay,		},
{ MissileTypeType,
    "missile-normal-spell",
    "normal spell.png",
    32,32,
    { NULL },
    MissileClassStayWithDelay,		},
{ MissileTypeType,
    "missile-explosion",
    "explosion.png",
    64,64,
    { NULL },
    MissileClassStayWithDelay,		},
{ MissileTypeType,
    "missile-small-cannon",
    "cannon.png",
    32,32,
    { "explosion" },
    MissileClassPointToPointWithDelay,	},
{ MissileTypeType,
    "missile-cannon-explosion",
    "cannon explosion.png",
    32,32,
    { NULL },
    MissileClassStayWithDelay,		},
{ MissileTypeType,
    "missile-cannon-tower-explosion",
    "cannon-tower explosion.png",
    32,32,
    { NULL },
    MissileClassStayWithDelay,		},
{ MissileTypeType,
    "missile-daemon-fire",
    "daemon fire.png",
    32,32,
    { NULL },
    MissileClassPointToPoint,		},
{ MissileTypeType,
    "missile-green-cross",
    "green cross.png",
    32,32,
    { NULL },
    MissileClassCycleOnce,			FRAMES_PER_SECOND/30	},
{ MissileTypeType,
    "missile-none",
    NULL,
    32,32,
    { NULL },
    MissileClassNone,			},
};

#define MAX_MISSILES	1800		/// maximum number of missiles

local int NumMissiles;			/// currently used missiles

local Missile Missiles[MAX_MISSILES];	/// all missiles on map

    /// lookup table for missile names
local hashtable(MissileType*,61) MissileHash;

/*============================================================================
==	MISSILE FUNCTIONS
============================================================================*/

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
	    MissileTypes[i].RleSprite=LoadRleSprite(
		    file,MissileTypes[i].Width,MissileTypes[i].Height);
	}
	MissileTypes[i].Type=i;
    }

    //
    //	Add missile names to hash table
    //
    for( i=0; i<sizeof(MissileTypes)/sizeof(*MissileTypes); ++i ) {
	*(MissileType**)hash_add(MissileHash,MissileTypes[i].Ident)
		=&MissileTypes[i];
    }
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

    DebugLevel0(__FUNCTION__": Missile %s not found\n",ident);
    return NULL;
}

/**
**	Create a new missile at (x,y).
**
**	@param missle_type	Type of missile.
**	@param sx		Missile x start point in pixel.
**	@param sy		Missile y start point in pixel.
**	@param dx		Missile x destination point in pixel.
**	@param dy		Missile y destination point in pixel.
**
**	@returns	created missile.
*/
global Missile* MakeMissile(int missile_type,int sx,int sy,int dx,int dy)
{
    int missile;

    DebugLevel3(__FUNCTION__": type %d(%s) at %d,%d to %d,%d\n"
	    ,missile_type,MissileTypes[missile_type].Name
	    ,sx,sy,dx,dy);

    if( missile_type>sizeof(MissileTypes)/sizeof(*MissileTypes) ) {
	DebugLevel0(__FUNCTION__": INTERNAL: Illegal type\n");
	return NULL;
    }

    //
    //	Find free slot, FIXME: see MakeUnit for better code
    //
    for( missile=0; missile<NumMissiles; ++missile ) {
	if( Missiles[missile].Type==MissileFree ) {
	    goto found;
	}
    }

    //	Check maximum missiles!
    if( NumMissiles==MAX_MISSILES ) {
	printf("Maximum of missiles reached\n");
	abort();
	return NULL;
    }

    missile=NumMissiles++;

found:
    Missiles[missile].X=sx;
    Missiles[missile].Y=sy;
    Missiles[missile].DX=dx;
    Missiles[missile].DY=dy;
    Missiles[missile].Type=&MissileTypes[missile_type];
    Missiles[missile].Frame=0;
    Missiles[missile].State=0;
    Missiles[missile].Wait=1;

    Missiles[missile].SourceUnit=NULL;
    Missiles[missile].SourceType=NULL;
    Missiles[missile].SourceStats=NULL;
    Missiles[missile].SourcePlayer=NULL;

    return &Missiles[missile];
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
    DebugLevel3("Damage done %d\n",damage);

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

    DebugLevel3(__FUNCTION__":\n");

    if( ((MissileType*)unit->Type->Missile.Missile)->Class==MissileClassNone ) {
	// FIXME: must hit now!!!
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

	    DebugLevel1("Missile-none hits no unit, shouldn't happen!\n");
	    return;
	}

	// FIXME: make sure thats the correct unit.
#ifdef NEW_UNIT
	// Check if goal is correct unit.

#endif

	if( goal->Removed ) {
	    DebugLevel1("Missile-none hits removed unit!\n");
	    return;
	}
	if( !goal->HP || goal->Command.Action==UnitActionDie ) {
	    DebugLevel1("Missile-none hits dead unit!\n");
	    return;
	}

	HitUnit(goal,CalculateDamage(unit->Stats,goal));

	return;
    }

    x=unit->X*TileSizeX+TileSizeX/2;
    y=unit->Y*TileSizeY+TileSizeY/2;
    if( (goal=unit->Command.Data.Move.Goal) ) {
	// Fire to nearest point of unit!
	if( goal->Type ) {
	    NearestOfUnit(goal,unit->X,unit->Y,&dx,&dy);
	} else {
	    // FIXME: unit killed?
	    dx=goal->X;
	    dy=goal->Y;
	}
	DebugLevel3("Fire to unit at %d,%d\n",dx,dy);
	dx=dx*TileSizeX+TileSizeX/2;
	dy=dy*TileSizeY+TileSizeY/2;
    } else {
	dx=unit->Command.Data.Move.DX*TileSizeX+TileSizeX/2;
	dy=unit->Command.Data.Move.DY*TileSizeY+TileSizeY/2;
    }

    missile=MakeMissile((MissileType*)unit->Type->Missile.Missile-MissileTypes
	    ,x,y,dx,dy);
    //
    //	Damage of missile
    //
    missile->SourceUnit=unit;
    missile->SourceType=unit->Type;
    missile->SourceStats=unit->Stats;
    missile->SourcePlayer=unit->Player;
}

/**
**	Draw missile.
*/
global void DrawMissile(MissileType* missile_type,int frame,int x,int y)
{
    // FIXME: remove this here, move to higher functions
    x-=missile_type->Width/2;
    y-=missile_type->Height/2;

    // FIXME: This is a hack for mirrored sprites
    if( frame&128 ) {
	DrawRleSpriteClippedX(missile_type->RleSprite,frame&127,x,y);
    } else {
	DrawRleSpriteClipped(missile_type->RleSprite,frame,x,y);
    }
}

/**
**	Draw all missiles on map.
*/
global void DrawMissiles(void)
{
    int missile;
    int x;
    int y;

    for( missile=0; missile<NumMissiles; ++missile ) {
	if( Missiles[missile].Type==MissileFree ) {
	    continue;
	}
	// FIXME: is this correct???
	x=Missiles[missile].X-MapX*TileSizeX+TheUI.MapX;
	y=Missiles[missile].Y-MapY*TileSizeY+TheUI.MapY;
	if( x<TheUI.MapX || x>=TheUI.MapWidth
		|| y<TheUI.MapY || y>=TheUI.MapHeight ) {
	    continue;
	}
	DrawMissile(Missiles[missile].Type,Missiles[missile].Frame,x,y);
    }
}

/**
**	Change missile heading from x,y.
*/
local void MissileNewHeadingFromXY(int missile,int x,int y)
{
    int heading;
    int frame;

    // Set new heading:
    if( x==0 ) {
	if( y<0 ) {
	    heading=HeadingN;
	} else {
	    heading=HeadingS;
	}
	frame=heading;
    } else if( x>0 ) {
	if( y<0 ) {
	    heading=HeadingNE;
	} else if( y==0 ) {
	    heading=HeadingE;
	} else {
	    heading=HeadingSE;
	}
	frame=heading;
    } else {
	if( y<0 ) {
	    heading=HeadingNW;
	} else if( y==0 ) {
	    heading=HeadingW;
	} else {
	    heading=HeadingSW;
	}
	frame=128+1+HeadingNW-heading;
    }
    Missiles[missile].Frame=frame;
}

#define MISSILE_STEPS	16		// How much did a missile move??

/**
**	Handle point to point missile.
*/
local int PointToPointMissile(int missile)
{
    int dx;
    int dy;
    int xstep;
    int ystep;
    int i;

    if( !(Missiles[missile].State&1) ) {
	// initialize
	dy=Missiles[missile].DY-Missiles[missile].Y;
	ystep=1;
	if( dy<0 ) {
	    dy=-dy;
	    ystep=-1;
	}
	dx=Missiles[missile].DX-Missiles[missile].X;
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
	    Missiles[missile].D=dy-1;
	    dx+=dx;
	    dy+=dy;
        } else if( dx>dy ) {	// step in horizontal direction
	    Missiles[missile].D=dx-1;
	    dx+=dx;
	    dy+=dy;
	}

	Missiles[missile].Dx=dx;
	Missiles[missile].Dy=dy;
	Missiles[missile].Xstep=xstep;
	Missiles[missile].Ystep=ystep;
	++Missiles[missile].State;
	DebugLevel3("Init: %d,%d, %d,%d, =%d\n"
		,dx,dy,xstep,ystep,Missiles[missile].D);
	return 0;
    } else {
	dx=Missiles[missile].Dx;
	dy=Missiles[missile].Dy;
	xstep=Missiles[missile].Xstep;
	ystep=Missiles[missile].Ystep;
    }


    //
    //	Move missile
    //
    if( dy==0 ) {		// horizontal line
	for( i=0; i<MISSILE_STEPS; ++i ) {
	    if( Missiles[missile].X==Missiles[missile].DX ) {
		return 1;
	    }
	    Missiles[missile].X+=xstep;
	}
	return 0;
    }

    if( dx==0 ) {		// vertical line
	for( i=0; i<MISSILE_STEPS; ++i ) {
	    if( Missiles[missile].Y==Missiles[missile].DY ) {
		return 1;
	    }
	    Missiles[missile].Y+=ystep;
	}
	return 0;
    }

    if( dx<dy ) {		// step in vertical direction
	for( i=0; i<MISSILE_STEPS; ++i ) {
	    if( Missiles[missile].Y==Missiles[missile].DY ) {
		return 1;
	    }
	    Missiles[missile].Y+=ystep;
	    Missiles[missile].D-=dx;
	    if( Missiles[missile].D<0 ) {
		Missiles[missile].D+=dy;
		Missiles[missile].X+=xstep;
	    }
	}
	return 0;
    }

    if( dx>dy ) {		// step in horizontal direction
	for( i=0; i<MISSILE_STEPS; ++i ) {
	    if( Missiles[missile].X==Missiles[missile].DX ) {
		return 1;
	    }
	    Missiles[missile].X+=xstep;
	    Missiles[missile].D-=dy;
	    if( Missiles[missile].D<0 ) {
		Missiles[missile].D+=dx;
		Missiles[missile].Y+=ystep;
	    }
	}
	return 0;
    }
				// diagonal line
    for( i=0; i<MISSILE_STEPS; ++i ) {
	if( Missiles[missile].Y==Missiles[missile].DY ) {
	    return 1;
	}
	Missiles[missile].X+=xstep;
	Missiles[missile].Y+=ystep;
    }
    return 0;
}

/**
**	Work for missile hit.
*/
global void MissileHit(int missile)
{
    Unit* goal;

    // FIXME: should I move the PlayMissileSound to here?
    // FIXME: And should the the connected missile be defined in the missile
    // FIXME: structure

    switch( Missiles[missile].Type->Type ) {
	case MissileArrow:
	case MissileAxe:
	    PlayMissileSound(Missiles+missile,
			     Missiles[missile].Type->ImpactSound.Sound);
	    break;
	case MissileBallistaBolt:
	case MissileBigCannon:
	    PlayMissileSound(Missiles+missile,
			     Missiles[missile].Type->ImpactSound.Sound);
	    MakeMissile(MissileImpact
		,Missiles[missile].X
		,Missiles[missile].Y
		,0,0);
	    break;

	case MissileSubmarineMissile:
	case MissileTurtleMissile:
	    PlayMissileSound(Missiles+missile,
			     Missiles[missile].Type->ImpactSound.Sound);
	    MakeMissile(MissileImpact
		,Missiles[missile].X
		,Missiles[missile].Y
		,0,0);
	    break;

	case MissileGreenCross:
	    break;
    }

    if( !Missiles[missile].SourceType ) {
	return;
    }

    // FIXME: must choose better goal!
    // FIXME: what can the missile hit?
    goal=UnitOnMapTile(Missiles[missile].X/TileSizeX
		,Missiles[missile].Y/TileSizeY);
    if( !goal || !goal->HP ) {
	int dx;
	int dy;

	dx=Missiles[missile].X/TileSizeX;
	dy=Missiles[missile].Y/TileSizeY;
	if( WallOnMap(dx,dy) ) {
	    DebugLevel3("Missile on wall?\n");
	    // FIXME: don't use UnitTypeByIdent here, this is slow!
	    HitWall(dx,dy,CalculateDamageStats(Missiles[missile].SourceStats,
		    UnitTypeByIdent("unit-human-wall")->Stats));
	    return;
	}
	return;
    }

    HitUnit(goal,CalculateDamage(Missiles[missile].SourceStats,goal));
}

/**
 ** Check missile visibility.
 */
//FIXME: is this correct?
//FIXME: should be exported?
local int MissileVisible(int missile)
{
    int tileMinX,tileMaxX;
    int tileMinY,tileMaxY;

    tileMinX=Missiles[missile].X/TileSizeX;
    tileMinY=Missiles[missile].Y/TileSizeY;
    tileMaxX=(Missiles[missile].X+Missiles[missile].Type->Width)/TileSizeX;
    tileMaxY=(Missiles[missile].Y+Missiles[missile].Type->Height)/TileSizeY;
    if ( (tileMinX>(MapX+MapWidth)) || (tileMaxX<MapX)
	 || (tileMinY>MapY+MapHeight) || (tileMaxY<MapY)) {
	return 0;
    }
    DebugLevel3("Missile bounding box %d %d %d %d (Map %d %d %d %d)\n",
		tileMinX,tileMaxX,tileMinY,tileMaxY,
		MapX,MapX+MapWidth,MapY,MapY+MapHeight);
    return 1;
}

/**
**	Handle all missile actions.
*/
//FIXME: (Fabrice) I don't know if my update for missile visibility is fully
//correct.
global void MissileActions(void)
{
    int missile;

    for( missile=0; missile<NumMissiles; ++missile ) {
	if( Missiles[missile].Type==MissileFree ) {
	    continue;
	}
	if( Missiles[missile].Wait-- ) {
	    continue;
	}
	if (MissileVisible(missile)) {
	    // check before movement
	    MustRedraw|=RedrawMap;
	}
	switch( Missiles[missile].Type->Class ) {

	    case MissileClassPointToPoint:
		Missiles[missile].Wait=1;
		if( PointToPointMissile(missile) ) {
		    MissileHit(missile);
		    Missiles[missile].Type=MissileFree;
		} else {
		    //
		    //	Animate missile, cycle through frames
		    //
		    Missiles[missile].Frame+=5;
		    if( (Missiles[missile].Frame&127)
			    >=Missiles[missile].Type->RleSprite->NumFrames ) {
			Missiles[missile].Frame=
			    // (Missiles[missile].Frame&128)|
				(Missiles[missile].Frame
				    -Missiles[missile].Type->RleSprite
					->NumFrames);
		    }
		    DebugLevel3("Frame %d of %d\n"
			,Missiles[missile].Frame
			,Missiles[missile].Type->RleSprite->NumFrames);

		}
		break;

	    case MissileClassPointToPointWithDelay:
		Missiles[missile].Wait=1;
		if( PointToPointMissile(missile) ) {
		    switch( Missiles[missile].State++ ) {
			case 1:
			    // FIXME: bounces up.
			    PlayMissileSound(Missiles+missile,
				     Missiles[missile].Type->ImpactSound.Sound);
			    // FIXME: make this configurable!!
			    switch( Missiles[missile].Type->Type ) {
				case MissileSmallCannon:
				    MakeMissile(MissileCannonExplosion
					,Missiles[missile].X
					,Missiles[missile].Y
					,0,0);
				    break;
				case MissileBigCannon:
				    MakeMissile(MissileCannonTowerExplosion
					,Missiles[missile].X
					,Missiles[missile].Y
					,0,0);
				    break;
				case MissileCatapultRock:
				case MissileBallistaBolt:
				    MakeMissile(MissileImpact
					,Missiles[missile].X
					,Missiles[missile].Y
					,0,0);
				    break;
			    }
			    break;
			default:
			    MissileHit(missile);
			    Missiles[missile].Type=MissileFree;
			    break;
		    }
		} else {
		    //
		    //	Animate missile, depends on the way.
		    //
		    // FIXME: how?
		}
		break;

	    case MissileClassPointToPoint3Bounces:
		Missiles[missile].Wait=1;
		if( PointToPointMissile(missile) ) {
		    //
		    //	3 Bounces.
		    //
		    switch( Missiles[missile].State ) {
			case 1:
			case 3:
			case 5:
			    Missiles[missile].State+=2;
			    Missiles[missile].DX+=
				Missiles[missile].Xstep*TileSizeX*2;
			    Missiles[missile].DY+=
				Missiles[missile].Ystep*TileSizeY*2;
			    PlayMissileSound(Missiles+missile,
				    Missiles[missile].Type->ImpactSound.Sound);
			    MakeMissile(MissileExplosion
				,Missiles[missile].X
				,Missiles[missile].Y
				,0,0);
			    MissileHit(missile);
			    // FIXME: hits to left and right
			    // FIXME: reduce damage
			    break;
			default:
			    Missiles[missile].Type=MissileFree;
			    break;
		    }
		} else {
		    //
		    //	Animate missile, cycle through frames
		    //
		    Missiles[missile].Frame+=5;
		    if( (Missiles[missile].Frame&127)
			    >=Missiles[missile].Type->RleSprite->NumFrames ) {
			Missiles[missile].Frame=
			    // (Missiles[missile].Frame&128)|
				(Missiles[missile].Frame
				    -Missiles[missile].Type->RleSprite
					->NumFrames);
		    }
		    DebugLevel3("Frame %d of %d\n"
			,Missiles[missile].Frame
			,Missiles[missile].Type->RleSprite->NumFrames);

		}
		break;

	    case MissileClassPointToPointWithHit:
		Missiles[missile].Wait=1;
		if( PointToPointMissile(missile) ) {
		    //
		    //	Animate hit
		    //
		    Missiles[missile].Frame+=5;
		    if( (Missiles[missile].Frame&127)
			    >=Missiles[missile].Type->RleSprite->NumFrames ) {
			MissileHit(missile);
			Missiles[missile].Type=MissileFree;
		    }
		}
		break;

	    case MissileClassStayWithDelay:
		Missiles[missile].Wait=1;
		if( ++Missiles[missile].Frame
			==Missiles[missile].Type->RleSprite
			    ->NumFrames ) {
		    MissileHit(missile);
		    Missiles[missile].Type=MissileFree;
		}
		break;

	    case MissileClassCycleOnce:
		Missiles[missile].Wait=Missiles[missile].Type->Speed;
		switch( Missiles[missile].State ) {
		    case 0:
		    case 2:
			++Missiles[missile].State;
			break;
		    case 1:
			if( ++Missiles[missile].Frame
				==Missiles[missile].Type->RleSprite
				    ->NumFrames ) {
			    --Missiles[missile].Frame;
			    ++Missiles[missile].State;
			}
			break;
		    case 3:
			if( !Missiles[missile].Frame-- ) {
			    MissileHit(missile);
			    Missiles[missile].Type=MissileFree;
			}
			break;
		}
		break;
	}
	if (Missiles[missile].Type!=MissileFree && MissileVisible(missile)) {
	    // check after movement
	    MustRedraw|=RedrawMap;
	}
    }
}

/**
**	Calculate distance from view-point to missle.
*/
global int ViewPointDistanceToMissile(Missile* dest)
{
    int x;
    int y;

    //FIXME: is it the correct formula?
    x=dest->X/TileSizeX;
    y=dest->Y/TileSizeY;
    DebugLevel3("Missile %p at %d %d\n",dest,x,y);
    return ViewPointDistance(x,y);
}

//@}
