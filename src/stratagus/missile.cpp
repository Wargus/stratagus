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
//	FreeCraft is free software; you can redistribute it and/or modify
//	it under the terms of the GNU General Public License as published
//	by the Free Software Foundation; either version 2 of the License,
//	or (at your option) any later version.
//
//	FreeCraft is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//	GNU General Public License for more details.
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


/**
**	W*rCr*ft number to internal missile-type name.
*/
global char** MissileTypeWcNames;

/**
**	Defined missile-types.
*/
global MissileType* MissileTypes;

global int NumMissileTypes;		/// number of missile-types made.

/*
**	Next missile-types are used hardcoded in the source.
*/
global MissileType* MissileTypeSmallFire;	/// Small fire missile-type
global MissileType* MissileTypeBigFire;		/// Big fire missile-type
global MissileType* MissileTypeGreenCross;	/// Green cross missile-type
    /// missile-type for the explosion missile
global MissileType* MissileTypeExplosion;

IfDebug(
global int NoWarningMissileType;		/// quiet ident lookup.
);

#define MAX_MISSILES	1800		/// maximum number of missiles

local Missile Missiles[MAX_MISSILES];	/// all missiles on map
local int NumMissiles;			/// currently used missiles

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
	    file=strcat(strcpy(buf,"graphics/"),file);
	    ShowLoadProgress("\tMissile %s\n",file);
	    MissileTypes[i].Sprite=LoadSprite(
		    file,MissileTypes[i].Width,MissileTypes[i].Height);

	    // Correct the number of frames in graphic
	    DebugCheck( MissileTypes[i].Sprite->NumFrames
			<MissileTypes[i].Frames );
	    MissileTypes[i].Sprite->NumFrames=MissileTypes[i].Frames;
	}
    }
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
    MissileType* const* mtype;

    mtype=(MissileType**)hash_find(MissileTypeHash,(char*)ident);

    if( mtype ) {
	return *mtype;
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
    MissileType* mtype;
    unsigned i;

    //
    //	Allocate new memory. (+2 for start end empty last entry.)
    //
    mtype=calloc(NumMissileTypes+2,sizeof(MissileType));
    if( !mtype ) {
	fprintf(stderr,"Out of memory\n");
	exit(-1);
    }
    memcpy(mtype,MissileTypes,sizeof(MissileType)*NumMissileTypes);
    if( MissileTypes ) {
	free(MissileTypes);
    }
    MissileTypes=mtype;
    mtype=MissileTypes+NumMissileTypes++;
    mtype->OType=MissileTypeType;
    mtype->Ident=ident;
    //
    //	Rehash.
    //
    for( i=0; i<NumMissileTypes; ++i ) {
	*(MissileType**)hash_add(MissileTypeHash,MissileTypes[i].Ident)
		=&MissileTypes[i];
    }
    return mtype;
}

/**
**	Create a new missile at (x,y).
**
**	@param mtype	Type pointer of missile.
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
global Missile* MakeMissile(MissileType* mtype,int sx,int sy,int dx,int dy)
{
    Missile* missile;

    DebugLevel3Fn("type %d(%s) at %d,%d to %d,%d\n"
	    ,mtype-MissileTypes,mtype->Ident,sx,sy,dx,dy);

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
    missile->X=sx-mtype->Width/2;
    missile->Y=sy-mtype->Height/2;
    missile->DX=dx-mtype->Width/2;
    missile->DY=dy-mtype->Height/2;
    missile->Type=mtype;
    missile->Frame=0;
    missile->State=0;
    missile->Wait=mtype->Sleep;		// initial wait = sleep
    missile->Delay=mtype->Delay;		// initial delay

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
    DebugLevel3Fn("\nDamage done [%d] %d %d ->%d\n",goal_stats->Armor,
	    basic_damage,piercing_damage, damage);

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

	// FIXME: Some missile hit the field of the target and all units on it.
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
		    HitWall(dx,dy,CalculateDamageStats(unit->Stats,
			    UnitTypeHumanWall->Stats,unit->Bloodlust));
		} else {
		    HitWall(dx,dy,CalculateDamageStats(unit->Stats,
			    UnitTypeOrcWall->Stats,unit->Bloodlust));
		}
		return;
	    }

	    DebugLevel1Fn("Missile-none hits no unit, shouldn't happen!\n");
	    return;
	}

	HitUnit(unit,goal,CalculateDamage(unit->Stats,goal,unit->Bloodlust));

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
    if ( !AnyMapAreaVisibleOnScreen(tileMinX,tileMinY,tileMaxX,tileMaxY) ) {
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
global void DrawMissile(const MissileType* mtype,unsigned frame,int x,int y)
{
    // FIXME: This is a hack for mirrored sprites
    if( frame&128 ) {
	VideoDrawClipX(mtype->Sprite,frame&127,x,y);
    } else {
	VideoDrawClip(mtype->Sprite,frame,x,y);
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
	    continue;	// custom missiles are handled by Controller() only
	}
	if( missile->Delay ) {
	    continue;	// delayed missiles aren't shown
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
	if( missile->Type->Class == MissileClassWhirlwind
		|| missile->Type->Class == MissileClassFlameShield ) {
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
**	Missile hits the goal.
**
**	@param missile	Missile hitting the goal.
**	@param goal	Goal of the missile.
**	@param splash	Splash damage divisor.
*/
local void MissileHitsGoal(const Missile* missile,Unit* goal,int splash)
{
    if ( BlizzardMissileHit && goal == missile->SourceUnit ) {
	return;		// blizzard cannot hit owner unit
    }

    if( goal->HP && goal->Orders[0].Action!=UnitActionDie ) {
	if ( missile->Damage ) {	// direct damage, spells mostly
	    HitUnit(missile->SourceUnit,goal,missile->Damage/splash);
	} else {
	    HitUnit(missile->SourceUnit,goal,
		    CalculateDamage(missile->SourceUnit->Stats,goal,
			missile->SourceUnit->Bloodlust)/splash);
	}
    }
}

/**
**	Missile hits wall.
**
**	@param missile	Missile hitting the goal.
**	@param X	Wall X position.
**	@param Y	Wall Y position.
**	@param splash	Splash damage divisor.
*/
local void MissileHitsWall(const Missile* missile,int x,int y,int splash)
{
    if( WallOnMap(x,y) ) {
	DebugLevel3Fn("Missile on wall?\n");
	if( HumanWallOnMap(x,y) ) {
	    if ( missile->Damage ) {	// direct damage, spells mostly
		HitWall(x,y,missile->Damage/splash);
	    } else {
		HitWall(x,y,CalculateDamageStats(missile->SourceUnit->Stats,
		    UnitTypeHumanWall->Stats,0)/splash);
	    }
	} else {
	    if ( missile->Damage ) {	// direct damage, spells mostly
		HitWall(x,y,missile->Damage/splash);
	    } else {
		HitWall(x,y,CalculateDamageStats(missile->SourceUnit->Stats,
		    UnitTypeOrcWall->Stats,0)/splash);
	    }
	}
	return;
    }
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
    Unit* table[UnitMax];
    int n;
    int i;

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

    x/=TileSizeX;
    y/=TileSizeY;

    if( x<0 || y<0 || x>=TheMap.Width || y>=TheMap.Height ) {
	// FIXME: this should handled by caller?
	DebugLevel0Fn("Missile gone outside of map!\n");
	return;				// outside the map.
    }

    //
    //	Choose correct goal.
    //
    if( !missile->Type->Range ) {
	if( missile->TargetUnit ) {
	    //
	    //	Missiles without range only hits the goal always.
	    //
	    goal=missile->TargetUnit;
	    if( goal->Destroyed ) {			// Destroyed
		RefsDebugCheck( !goal->Refs );
		if( !--goal->Refs ) {
		    ReleaseUnit(goal);
		}
		missile->TargetUnit=NoUnitP;
		return;
	    }
	    MissileHitsGoal(missile,goal,1);
	    return;
	}
	MissileHitsWall(missile,x,y,1);
	return;
    }

    //
    //	Hits all units in range.
    //
    i=missile->Type->Range;
    n=SelectUnits(x-i+1,y-i+1,x+i,y+i,table);
    for( i=0; i<n; ++i ) {
	goal=table[i];
	//
	//	Can the unit attack the this unit-type?
	//	NOTE: perhaps this should be come a property of the missile.
	//
	if( CanTarget(missile->SourceUnit->Type,goal->Type) ) {
	    // We are attacking the nearest field of the unit
	    if( x<goal->X || y<goal->Y
		    || x>=goal->X+goal->Type->TileWidth
		    || y>=goal->Y+goal->Type->TileHeight ) {
		MissileHitsGoal(missile,goal,2);
	    } else {
		MissileHitsGoal(missile,goal,1);
	    }
	}
    }
    //
    //	Missile hits ground.
    //
    // FIXME: no bock writting it correct.
    x-=missile->Type->Range;
    y-=missile->Type->Range;
    for( i=missile->Type->Range*2; --i; ) {
	for( n=missile->Type->Range*2; --n; ) {
	    if( x+i>=0 && x+i<TheMap.Width && y+n>=0 && y+n<TheMap.Height ) {
		if( i==0 && n==0 ) {
		    MissileHitsWall(missile,x+i,y+n,1);
		} else {
		    MissileHitsWall(missile,x+i,y+n,2);
		}
	    }
	}
    }
}

/**
**	Handle all missile actions.
**
**	@todo	Split this function, into small functions, than it is
**		better readable and easier to extend.
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
	if( missile->Delay && missile->Delay-- ) {
	    continue;		// delay start of missile
	}

	if ( missile->TTL > 0 ) {
	    missile->TTL--;	// overall time to live if specified
	}

	if ( missile->Controller ) {
	    missile->Controller( missile );
	}

	if ( !missile->TTL ) {
	    FreeMissile(missile);
	    continue;
	}

	if( --missile->Wait ) {	// wait until time is over
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

	    case MissileClassFlameShield:
		missile->Wait=missile->Type->Sleep;
		if( ++missile->Frame
			==VideoGraphicFrames(missile->Type->Sprite) ) {
		    missile->Frame = 0;
		    if( PointToPointMissile(missile) ) {
			// Must set new goal.
		    }
		}
		//missile->X-=unit->TargetUnit->X*TileSizeX+unit->TargetUnit->IY;
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
			BlizzardMissileHit = 0;
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
		    BlizzardMissileHit = 0;
		    FreeMissile(missile);
		}
		break;

	    case MissileClassWhirlwind:
		missile->Wait=missile->Type->Sleep;
		if( ++missile->Frame
			==VideoGraphicFrames(missile->Type->Sprite) ) {
		    missile->Frame = 0;
		    PointToPointMissile(missile);
		}
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

	    case MissileClassFire: {
		Unit* unit;

		unit=missile->SourceUnit;
		if( unit->Destroyed || !unit->HP ) {
		    FreeMissile(missile);
		    break;
		}
		missile->Wait=missile->Type->Sleep;
		if( ++missile->Frame
			==VideoGraphicFrames(missile->Type->Sprite) ) {
		    int f;

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
    MissileType* mtype;
    char** sp;
    int i;

    fprintf(file,"\n;;; -----------------------------------------\n");
    fprintf(file,";;; MODULE: missile-types $Id$\n\n");

    //
    //	Original number to internal missile-type name.
    //
    i=fprintf(file,"(define-missiletype-wc-names");
    for( sp=MissileTypeWcNames; *sp; ++sp ) {
	if( i+strlen(*sp)>79 ) {
	    i=fprintf(file,"\n ");
	}
	i+=fprintf(file," '%s",*sp);
    }
    fprintf(file,")\n\n");

    //
    //	Missile types
    //
    for( mtype=MissileTypes; mtype<&MissileTypes[NumMissileTypes]; ++mtype ) {
	fprintf(file,"(define-missile-type '%s\n ",mtype->Ident);
	if( mtype->File ) {
	    fprintf(file," 'file \"%s\"",mtype->File);
	}
	fprintf(file," 'size '(%d %d)",mtype->Width,mtype->Height);
	if( mtype->Sprite ) {
	    fprintf(file," 'frames %d",mtype->Frames);
	}
	fprintf(file,"\n ");
	if( mtype->FiredSound.Name ) {
	    fprintf(file," 'fired-sound \"%s\"",mtype->FiredSound.Name);
	}
	if( mtype->ImpactSound.Name ) {
	    fprintf(file," 'impact-sound \"%s\"",mtype->ImpactSound.Name);
	}
	if( mtype->FiredSound.Name || mtype->ImpactSound.Name ) {
	    fprintf(file,"\n ");
	}
	fprintf(file," 'class '%s",MissileClassNames[mtype->Class]);
	if( mtype->Delay ) {
	    fprintf(file," 'delay %d",mtype->Delay);
	}
	fprintf(file," 'sleep %d",mtype->Sleep);
	fprintf(file," 'speed %d",mtype->Speed);
	fprintf(file," 'range %d",mtype->Range);
	if( mtype->ImpactMissile ) {
	    fprintf(file,"\n  'impact-missile '%s",mtype->ImpactMissile->Ident);
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

    fprintf(file,"(missile 'type '%s",missile->Type->Ident);
    fprintf(file," 'pos '(%d %d) 'goal '(%d %d)",
	missile->X,missile->Y,missile->DX,missile->DY);
    fprintf(file,"\n  'frame %d 'state %d 'wait %d 'delay %d\n ",
	missile->Frame,missile->State,missile->Wait,missile->Delay);
    if( missile->SourceUnit ) {
	fprintf(file," 'source %s",s1=UnitReference(missile->SourceUnit));
	free(s1);
    }
    if( missile->TargetUnit ) {
	fprintf(file," 'target %s",s1=UnitReference(missile->TargetUnit));
	free(s1);
    }
    fprintf(file," 'damage %d",missile->Damage);
    // FIXME: need symbolic names for controller
    fprintf(file," 'ttl %d 'controller %ld",
	missile->TTL,(long)missile->Controller);
    fprintf(file," 'data '(%d %d %d %d %d)",
	missile->D,missile->Dx,missile->Dy,missile->Xstep,missile->Ystep);
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

/**
**	Initialize missile-types.
*/
global void InitMissileTypes(void)
{
    MissileType* mtype;

    for( mtype=MissileTypes; mtype->OType; ++mtype ) {
	//
	//	Add missile names to hash table
	//
	*(MissileType**)hash_add(MissileTypeHash,mtype->Ident)=mtype;

	//
	//	Resolve impact missiles and sounds.
	//
	if( mtype->FiredSound.Name ) {
	    mtype->FiredSound.Sound=SoundIdForName(mtype->FiredSound.Name);
	}
	if( mtype->ImpactSound.Name ) {
	    mtype->ImpactSound.Sound=SoundIdForName(mtype->ImpactSound.Name);
	}
	if( mtype->ImpactName ) {
	    mtype->ImpactMissile=MissileTypeByIdent(mtype->ImpactName);
	}
    }

    MissileTypeSmallFire=MissileTypeByIdent("missile-small-fire");
    MissileTypeBigFire=MissileTypeByIdent("missile-big-fire");
    MissileTypeGreenCross=MissileTypeByIdent("missile-green-cross");
    MissileTypeExplosion = MissileTypeByIdent("missile-explosion");

#if 0
    // FIXME: FIXME: FIXME: very dirty hacks
    DebugCheck( MissileTypeSmallFire->Sprite->NumFrames!=6 );
    MissileTypeSmallFire->Sprite->NumFrames=6;
    DebugCheck( MissileTypeByIdent("missile-death-and-decay")
		->Sprite->NumFrames!=8 );
    MissileTypeByIdent("missile-death-and-decay")->Sprite->NumFrames=8;
    DebugCheck( MissileTypeByIdent("missile-normal-spell")
		->Sprite->NumFrames!=6 );
    MissileTypeByIdent("missile-normal-spell")->Sprite->NumFrames=6;
    DebugCheck( MissileTypeByIdent("missile-flame-shield")
		->Sprite->NumFrames!=6 );
    MissileTypeByIdent("missile-flame-shield")->Sprite->NumFrames=6;
#endif
}

/**
**	Clean up missile-types.
*/
global void CleanMissileTypes(void)
{
    MissileType* mtype;

    for( mtype=MissileTypes; mtype->OType; ++mtype ) {
	hash_del(MissileTypeHash,mtype->Ident);

	free(mtype->Ident);
	free(mtype->File);
	free(mtype->FiredSound.Name);
	free(mtype->ImpactSound.Name);
	free(mtype->ImpactName);

	VideoSaveFree(mtype->Sprite);
    }
    free(MissileTypes);
    MissileTypes=NULL;
    NumMissileTypes=0;

    MissileTypeSmallFire=NULL;
    MissileTypeBigFire=NULL;
    MissileTypeGreenCross=NULL;
    MissileTypeExplosion=NULL;
}

/**
**	Initialize missiles.
*/
global void InitMissiles(void)
{
}

/**
**	Clean up missiles.
*/
global void CleanMissiles(void)
{
    NumMissiles=0;
}

//@}
