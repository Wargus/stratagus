//       _________ __                 __                               
//      /   _____//  |_____________ _/  |______     ____  __ __  ______
//      \_____  \\   __\_  __ \__  \\   __\__  \   / ___\|  |  \/  ___/
//      /        \|  |  |  | \// __ \|  |  / __ \_/ /_/  >  |  /\___ |
//     /_______  /|__|  |__|  (____  /__| (____  /\___  /|____//____  >
//             \/                  \/          \//_____/            \/ 
//  ______________________                           ______________________
//			  T H E   W A R   B E G I N S
//	   Stratagus - A free fantasy real time strategy game engine
//
/**@name missile.c	-	The missiles. */
//
//	(c) Copyright 1998-2003 by Lutz Sammer and Jimmy Salmon
//
//      This program is free software; you can redistribute it and/or modify
//      it under the terms of the GNU General Public License as published by
//      the Free Software Foundation; version 2 dated June, 1991.
//
//      This program is distributed in the hope that it will be useful,
//      but WITHOUT ANY WARRANTY; without even the implied warranty of
//      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//      GNU General Public License for more details.
//
//      You should have received a copy of the GNU General Public License
//      along with this program; if not, write to the Free Software
//      Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
//      02111-1307, USA.
//
//	$Id$

//@{

/*----------------------------------------------------------------------------
--	Includes
----------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "stratagus.h"
#include "video.h"
#include "font.h"
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

//	TODO : Remove, configure in ccl.
//	#define FIREBALL_DAMAGE		20	/// Damage of center fireball
#define WHIRLWIND_DAMAGE1	 4	/// the center of the whirlwind
#define WHIRLWIND_DAMAGE2	 1	/// the periphery of the whirlwind
//#define RUNE_DAMAGE		50	/// Rune damage


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
    "missile-class-hit",
    "missile-class-parabolic",
    "missile-class-land-mine",
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

#ifdef DEBUG
global int NoWarningMissileType;		/// quiet ident lookup.
#endif

local Missile* GlobalMissiles[MAX_MISSILES];	/// all global missiles on map
local int NumGlobalMissiles;			/// currently used missiles

local Missile* LocalMissiles[MAX_MISSILES * 8];	/// all local missiles on map
local int NumLocalMissiles;			/// currently used missiles

#ifdef DOXYGEN                          // no real code, only for document

    /// lookup table for missile names
local MissileType* MissileTypeHash[61];

#else

    /// lookup table for missile names
local hashtable(MissileType*, 61) MissileTypeHash;

#endif

global BurningBuildingFrame* BurningBuildingFrames;  /// Burning building frames

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

    for (i = 0; MissileTypes[i].OType; ++i) {
	if ((file = MissileTypes[i].File)) {
	    char* buf;

	    buf = alloca(strlen(file) + 9 + 1);
	    file = strcat(strcpy(buf, "graphics/"), file);
	    ShowLoadProgress("Missile %s", file);
	    MissileTypes[i].Sprite = LoadSprite(
		    file, MissileTypes[i].Width, MissileTypes[i].Height);

	    // Correct the number of frames in graphic
	    DebugCheck(MissileTypes[i].Sprite->NumFrames < MissileTypes[i].SpriteFrames);
	    MissileTypes[i].Sprite->NumFrames = MissileTypes[i].SpriteFrames;
	    // FIXME: Don't use NumFrames as number of frames.
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

    DebugCheck(ident == NULL);
    mtype = (MissileType**)hash_find(MissileTypeHash, (char*)ident);
    if (mtype) {
	return *mtype;
    }

#ifdef DEBUG
    if (!NoWarningMissileType) {
	DebugLevel0Fn("Missile %s not found\n" _C_ ident);
    }
#endif
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
    int i;

    DebugCheck(ident == NULL);
    //
    //	Allocate new memory. (+2 for start end empty last entry.)
    //
    mtype = calloc(NumMissileTypes + 2, sizeof(MissileType));
    if (!mtype) {
	fprintf(stderr, "Out of memory\n");
	ExitFatal(-1);
    }
    memcpy(mtype, MissileTypes, sizeof(MissileType) * NumMissileTypes);
    if (MissileTypes) {
	free(MissileTypes);
    }
    MissileTypes = mtype;
    mtype = MissileTypes + NumMissileTypes++;
    mtype->OType = MissileTypeType;
    mtype->Ident = ident;
    //
    //	Rehash.
    //
    for (i = 0; i < NumMissileTypes; ++i) {
	*(MissileType**)hash_add(MissileTypeHash, MissileTypes[i].Ident) = &MissileTypes[i];
    }

    mtype->CanHitOwner = 0;		// defaults
    mtype->FriendlyFire = 0;

    return mtype;
}

/**
**	Allocate memory for a new global missile.
*/
local Missile* NewGlobalMissile(void)
{
    Missile* missile;

    //	Check maximum missiles!
    if (NumGlobalMissiles == MAX_MISSILES) {
	fprintf(stderr, "Maximum of global missiles reached\n");
	abort();
	return NULL;
    }

    missile = calloc(1, sizeof(Missile));
    memset(missile, 0, sizeof (*missile));
    missile->MissileSlot = GlobalMissiles + NumGlobalMissiles;
    GlobalMissiles[NumGlobalMissiles++] = missile;

    return missile;
}

/**
**	Allocate memory for a new local missile.
*/
local Missile* NewLocalMissile(void)
{
    Missile* missile;

    //	Check maximum missiles!
    if (NumLocalMissiles == MAX_MISSILES * 8) {
	fprintf(stderr, "Maximum of local missiles reached\n");
	abort();
	return NULL;
    }

    missile = calloc(1, sizeof(Missile));
    memset(missile, 0, sizeof (*missile));
    missile->MissileSlot = LocalMissiles + NumLocalMissiles;
    LocalMissiles[NumLocalMissiles++] = missile;
    missile->Local = 1;

    return missile;
}

/**
**	Initialize a new made missile.
**
**	@param missile	Pointer to new uninitialized missile.
**	@param mtype	Type pointer of missile.
**	@param sx	Missile x start point in pixel.
**	@param sy	Missile y start point in pixel.
**	@param dx	Missile x destination point in pixel.
**	@param dy	Missile y destination point in pixel.
**
**	@return		created missile.
*/
local Missile* InitMissile(Missile* missile, MissileType* mtype, int sx,
    int sy, int dx, int dy)
{
    DebugCheck(!mtype);
    DebugCheck(!missile);
    missile->X = sx - mtype->Width / 2;
    missile->Y = sy - mtype->Height / 2;
    missile->DX = dx - mtype->Width / 2;
    missile->DY = dy - mtype->Height / 2;
    missile->SourceX = missile->X;
    missile->SourceY = missile->Y;
    missile->Type = mtype;
    missile->SpriteFrame = 0;
    missile->State = 0;
    missile->Wait = mtype->Sleep;	// initial wait = sleep
    missile->Delay = mtype->StartDelay;	// initial delay

    missile->SourceUnit = NULL;

    missile->Damage = 0;
    missile->TargetUnit = NULL;
    missile->TTL = -1;
    missile->Controller = NULL;

    return missile;
}

/**
**	Create a new global missile at (x,y).
**
**	@param mtype	Type pointer of missile.
**	@param sx	Missile x start point in pixel.
**	@param sy	Missile y start point in pixel.
**	@param dx	Missile x destination point in pixel.
**	@param dy	Missile y destination point in pixel.
**
**	@return		created missile.
*/
global Missile* MakeMissile(MissileType* mtype, int sx, int sy, int dx, int dy)
{
    Missile* missile;

    DebugCheck(!mtype);
    DebugLevel3Fn("type %d(%s) at %d,%d to %d,%d\n" _C_
	mtype - MissileTypes _C_ mtype->Ident _C_ sx _C_ sy _C_ dx _C_ dy);


    if (!(missile = NewGlobalMissile())) {
	return missile;
    }

    return InitMissile(missile, mtype, sx, sy, dx, dy);
}

/**
**	Create a new local missile at (x,y).
**
**	@param mtype	Type pointer of missile.
**	@param sx	Missile x start point in pixel.
**	@param sy	Missile y start point in pixel.
**	@param dx	Missile x destination point in pixel.
**	@param dy	Missile y destination point in pixel.
**
**	@return		created missile.
*/
global Missile* MakeLocalMissile(MissileType* mtype, int sx, int sy, int dx, int dy)
{
    Missile* missile;

    DebugCheck(!mtype);
    DebugLevel3Fn("type %d(%s) at %d,%d to %d,%d\n" _C_
	mtype - MissileTypes _C_ mtype->Ident _C_ sx _C_ sy _C_ dx _C_ dy);

    if (!(missile = NewLocalMissile())) {
	return missile;
    }

    return InitMissile(missile, mtype, sx, sy, dx, dy);
}

/**
**	Free a missile.
**
**	@param missile	Missile pointer.
*/
local void FreeMissile(Missile* missile)
{
    Missile* temp;
    Unit* unit;

    DebugCheck(missile == NULL);
    //
    //	Release all unit references.
    //
    if ((unit = missile->SourceUnit)) {
	RefsDebugCheck(!unit->Refs);
	if (unit->Destroyed) {
	    if (!--unit->Refs) {
		ReleaseUnit(unit);
	    }
	} else {
	    --unit->Refs;
	    RefsDebugCheck(!unit->Refs);
	}
    }
    if((unit = missile->TargetUnit)) {
	RefsDebugCheck(!unit->Refs);
	if (unit->Destroyed) {
	    if (!--unit->Refs) {
		ReleaseUnit(unit);
	    }
	} else {
	    --unit->Refs;
	    RefsDebugCheck(!unit->Refs);
	}
    }

    //
    //	Free the missile memory
    //		Note: removing the last missile works.
    //
    if (missile->Local) {
	DebugCheck(*missile->MissileSlot != missile);
	temp=LocalMissiles[--NumLocalMissiles];
	DebugCheck(*temp->MissileSlot != temp);
	temp->MissileSlot = missile->MissileSlot;
	*missile->MissileSlot = temp;
	LocalMissiles[NumLocalMissiles] = NULL;
    } else {
	DebugCheck(*missile->MissileSlot != missile);
	temp = GlobalMissiles[--NumGlobalMissiles];
	DebugCheck(*temp->MissileSlot != temp);
	temp->MissileSlot = missile->MissileSlot;
	*missile->MissileSlot = temp;
	GlobalMissiles[NumGlobalMissiles] = NULL;
    }

    free(missile);
}

/**
**	Calculate damage.
**
**	Damage calculation:
**		(BasicDamage-Armor)+PiercingDamage
**	damage =----------------------------------
**				    2
**	damage is multiplied by random between 1 and 2.
**
**	@todo NOTE: different targets (big are hit by some missiles better)
**	@todo NOTE: lower damage for hidden targets.
**	@todo NOTE: lower damage for targets on higher ground.
**
**	@param attacker_stats	Attacker attributes.
**	@param goal_stats	Goal attributes.
**	@param bloodlust	If attacker has bloodlust
**	@param xp		Experience of attacker.
**
**	@return			damage inflicted to goal.
*/
local int CalculateDamageStats(const UnitStats* attacker_stats,
    const UnitStats* goal_stats, int bloodlust, int xp)
{
    int damage;
    int basic_damage;
    int piercing_damage;

    basic_damage = attacker_stats->BasicDamage + isqrt(xp / 100) * XpDamage;
    piercing_damage = attacker_stats->PiercingDamage;
    if (bloodlust) {
	basic_damage *= 2;
	piercing_damage *= 2;
	DebugLevel3Fn("bloodlust\n");
    }

#if 0
    damage = basic_damage - goal_stats->Armor;
    if (damage < 0) {
	// Use minimum damage
	if (piercing_damage < 30 && basic_damage < 30) {
	    damage = (piercing_damage + 1) / 2;
	} else {
	    damage = (piercing_damage + basic_damage - 30) / 2;
	}
    } else {
	damage += piercing_damage;
	damage -= SyncRand() % ((damage + 2) / 2);
    }
#else
    damage = max(basic_damage - goal_stats->Armor, 1) + piercing_damage;
    damage -= SyncRand() % ((damage + 2) / 2);
    DebugCheck(damage < 0);
#endif
	
    DebugLevel3Fn("\nDamage done [%d] %d %d ->%d\n" _C_ goal_stats->Armor _C_
	basic_damage _C_ piercing_damage _C_ damage);

    return damage;
}

/**
**	Calculate damage.
**
**	@param attacker_stats	Attacker attributes.
**	@param goal		Goal unit.
**	@param bloodlust	If attacker has bloodlust
**	@param xp		Experience of attack.
**
**	@return			damage produces on goal.
*/
local int CalculateDamage(const UnitStats* attacker_stats,
    const Unit* goal, int bloodlust, int xp)
{
    DebugCheck(attacker_stats == NULL);
    DebugCheck(goal == NULL);
    return CalculateDamageStats(attacker_stats, goal->Stats, bloodlust, xp);
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

    DebugCheck(unit == NULL);
    //
    //	Goal dead?
    //
    goal = unit->Orders[0].Goal;
    if (goal) {

	// Better let the caller/action handle this.

	if (goal->Destroyed) {
	    DebugLevel0Fn("destroyed unit\n");
	    return;
	}
	if (goal->Removed) {
	    DebugLevel3Fn("Missile-none hits removed unit!\n");
	    return;
	}
	if (!goal->HP || goal->Orders[0].Action == UnitActionDie) {
	    DebugLevel3Fn("Missile hits dead unit!\n");
	    return;
	}

	// FIXME: Some missile hit the field of the target and all units on it.
	// FIXME: goal is already dead, but missile could hit others?
    }

    //
    //	No missile hits immediately!
    //
    if (unit->Type->Missile.Missile->Class == MissileClassNone) {
	// No goal, take target coordinates
	if (!goal) {
	    dx = unit->Orders[0].X;
	    dy = unit->Orders[0].Y;
	    if (WallOnMap(dx, dy)) {
		if (HumanWallOnMap(dx, dy)) {
		    HitWall(dx, dy,
			CalculateDamageStats(unit->Stats,
			    UnitTypeHumanWall->Stats, unit->Bloodlust, unit->XP));
		} else {
		    HitWall(dx, dy,
			CalculateDamageStats(unit->Stats,
			    UnitTypeOrcWall->Stats, unit->Bloodlust, unit->XP));
		}
		return;
	    }

	    DebugLevel1Fn("Missile-none hits no unit, shouldn't happen!\n");
	    return;
	}

	HitUnit(unit, goal,
	    CalculateDamage(unit->Stats, goal, unit->Bloodlust, unit->XP));

	return;
    }

    x = unit->X * TileSizeX + TileSizeX / 2;	// missile starts in tile middle
    y = unit->Y * TileSizeY + TileSizeY / 2;

    if (goal) {
	DebugCheck(!goal->Type);	// Target invalid?
	// Fire to nearest point of the unit!
	NearestOfUnit(goal, unit->X, unit->Y, &dx, &dy);
	DebugLevel3Fn("Fire to unit at %d,%d\n" _C_ dx _C_ dy);

	//
	//	Moved out of attack range?
	//
	if (MapDistanceBetweenUnits(unit, goal) < unit->Type->MinAttackRange) {
	    DebugLevel0Fn("Missile target too near %d,%d\n" _C_
		MapDistanceBetweenUnits(unit,goal) _C_ unit->Type->MinAttackRange);
	    // FIXME: do something other?
	    return;
	}

    } else {
	dx = unit->Orders[0].X;
	dy = unit->Orders[0].Y;
	// FIXME: Can this be too near??
    }

    // Fire to the tile center of the destination.
    dx = dx * TileSizeX + TileSizeX / 2;
    dy = dy * TileSizeY + TileSizeY / 2;
    missile = MakeMissile(unit->Type->Missile.Missile, x, y, dx, dy);
    //
    //	Damage of missile
    //
    if (goal) {
	missile->TargetUnit = goal;
	RefsDebugCheck(!goal->Refs || goal->Destroyed);
	goal->Refs++;
    }
    missile->SourceUnit = unit;
    RefsDebugCheck(!unit->Refs || unit->Destroyed);
    unit->Refs++;
}

/**
**      Get area of tiles covered by missile
**
**      @param missile  Missile to be checked and set.
**	@param sx	OUT: Pointer to X of top left corner in map tiles.
**	@param sy	OUT: Pointer to Y of top left corner in map tiles.
**	@param ex	OUT: Pointer to X of bottom right corner in map tiles.
**	@param ey	OUT: Pointer to Y of bottom right corner in map tiles.
**      @return         sx,sy,ex,ey defining area in Map
*/
local void GetMissileMapArea(const Missile* missile, int* sx, int* sy,
    int* ex, int* ey)
{
    DebugCheck(missile == NULL);
    DebugCheck(sx == NULL);
    DebugCheck(sy == NULL);
    DebugCheck(ex == NULL);
    DebugCheck(ey == NULL);
    DebugCheck(TileSizeX <= 0);
    DebugCheck(TileSizeY <= 0);
    DebugCheck(missile->Type == NULL);
    *sx = missile->X / TileSizeX;
    *sy = missile->Y / TileSizeY;
    *ex = (missile->X + missile->Type->Width) / TileSizeX;
    *ey = (missile->Y + missile->Type->Height) / TileSizeY;
}

/**
**      Check missile visibility in a given viewport.
**
**	@param vp	Viewport to be checked.
**      @param missile  Missile pointer to check if visible.
**
**      @return         Returns true if visible, false otherwise.
*/
local int MissileVisibleInViewport(const Viewport* vp, const Missile* missile)
{
    int min_x;
    int max_x;
    int min_y;
    int max_y;

    DebugCheck(vp == NULL);
    DebugCheck(missile == NULL);
    GetMissileMapArea(missile, &min_x, &min_y, &max_x, &max_y);
    if (!AnyMapAreaVisibleInViewport(vp, min_x, min_y, max_x, max_y)) {
	return 0;
    }
    DebugLevel3Fn("Missile bounding box %d %d %d %d\n" _C_ min_x _C_ max_x _C_
	min_y _C_ max_y);
    if (!IsMapFieldVisible(ThisPlayer, (missile->X - TileSizeX / 2) / TileSizeX,
	    (missile->Y - TileSizeY / 2) / TileSizeY) && !ReplayRevealMap) {
	return 0;
    }
    return 1;
}

/**
**      Check and sets if missile must be drawn on screen-map
**
**      @param missile  Missile to be checked.
**      @return         True if map marked to be drawn, false otherwise.
*/
global int CheckMissileToBeDrawn(const Missile* missile)
{
    int sx;
    int sy;
    int ex;
    int ey;

    DebugCheck(missile == NULL);
    GetMissileMapArea(missile, &sx, &sy, &ex, &ey);
    return MarkDrawAreaMap(sx, sy, ex, ey);
}

/**
**	Draw missile.
**
**	@param mtype	Missile type
**	@param frame	Animation frame
**	@param x	Screen pixel X position
**	@param y	Screen pixel Y position
*/
global void DrawMissile(const MissileType* mtype, int frame, int x, int y)
{
    DebugCheck(mtype == NULL);
    // FIXME: This is a hack for mirrored sprites
    if (frame < 0) {
	VideoDrawClipX(mtype->Sprite, -frame, x, y);
    } else {
	VideoDrawClip(mtype->Sprite, frame, x, y);
    }
}

/**
**	FIXME: docu
*/
local int MissileDrawLevelCompare(const void* v1, const void* v2)
{
    const Missile* c1;
    const Missile* c2;

    DebugCheck(v1 == NULL);
    DebugCheck(v2 == NULL);

    c1 = *(Missile**)v1;
    c2 = *(Missile**)v2;

    if (c1->Type->DrawLevel == c2->Type->DrawLevel) {
	return c1->MissileSlot < c2->MissileSlot ? -1 : 1;
    } else {
	return c1->Type->DrawLevel <= c2->Type->DrawLevel ? -1 : 1;
    }
}
/**
**	Draw all missiles on map.
**
**	@param vp	Viewport pointer.
**	@param table
*/
global int FindAndSortMissiles(const Viewport* vp, Missile** table)
{
    Missile* missile;
    Missile* const* missiles;
    Missile* const* missiles_end;
    int flag;
    int nmissiles;

    DebugCheck(vp == NULL);
    DebugCheck(table == NULL);
    //
    //	Loop through global missiles, than through locals.
    //
    flag = 1;
    missiles = GlobalMissiles;
    missiles_end = missiles + NumGlobalMissiles;
    nmissiles = 0;
    do {
	for (; missiles < missiles_end; ++missiles) {
	    missile = *missiles;
	    if (missile->Type->Class == MissileClassCustom) {
		continue;	// custom are handled by Controller() only
	    }
	    if (missile->Delay) {
		continue;	// delayed aren't shown
	    }
	    // Draw only visible missiles
	    if (!flag || MissileVisibleInViewport(vp, missile)) {
	        table[nmissiles++] = missile;
	    }
	}
	missiles = LocalMissiles;
	missiles_end = missiles + NumLocalMissiles;
    } while (flag--);
    if (nmissiles) {
	qsort((void *)table, nmissiles, sizeof(Missile*), MissileDrawLevelCompare);
    }

    return nmissiles;
}

/**
**	Change missile heading from x,y.
**
**	@param missile	Missile pointer.
**	@param dx	Delta in x.
**	@param dy	Delta in y.
*/
local void MissileNewHeadingFromXY(Missile* missile, int dx, int dy)
{
    int dir;
    int nextdir;

    DebugCheck(missile == NULL);
    if (missile->SpriteFrame < 0) {
	missile->SpriteFrame = -missile->SpriteFrame;
    }
    missile->SpriteFrame /= missile->Type->NumDirections / 2 + 1;
    missile->SpriteFrame *= missile->Type->NumDirections / 2 + 1;

    nextdir = 256 / missile->Type->NumDirections;
    dir = ((DirectionToHeading(dx, dy) + nextdir / 2) & 0xFF) / nextdir;
    if (dir <= LookingS / nextdir) {	// north->east->south
	missile->SpriteFrame += dir;
    } else {
	missile->SpriteFrame += 256 / nextdir - dir;
	missile->SpriteFrame = -missile->SpriteFrame;
    }
}

/**
**	Handle point to point missile.
**
**	@param missile	Missile pointer.
**
**	@return 1 if goal is reached, 0 else.
*/
local int PointToPointMissile(Missile* missile)
{
    int dx;
    int dy;
    int xstep;
    int ystep;
    int i;

    if (!(missile->State & 1)) {
	// initialize
	dy = missile->DY - missile->Y;
	ystep = 1;
	if (dy < 0) {
	    dy = -dy;
	    ystep = -1;
	}
	dx = missile->DX - missile->X;
	xstep = 1;
	if (dx < 0) {
	    dx = -dx;
	    xstep = -1;
	}

	// FIXME: could be better written
	if (missile->Type->Class == MissileClassWhirlwind ||
		missile->Type->Class == MissileClassFlameShield) {
	    // must not call MissileNewHeading nor frame change
	} else if (missile->Type->Class == MissileClassBlizzard) {
	    missile->SpriteFrame = 0;
	} else if (missile->Type->Class == MissileClassPointToPoint3Bounces) {
	    missile->DX -= xstep * TileSizeX / 2;
	    missile->DY -= ystep * TileSizeY / 2;
	} else {
	    MissileNewHeadingFromXY(missile, dx * xstep, dy * ystep);
	}

	if (dy == 0) {		// horizontal line
	    if (dx == 0) {
		return 1;
	    }
	} else if (dx == 0) {	// vertical line
	} else if (dx < dy) {	// step in vertical direction
	    missile->D = dy - 1;
	    dx += dx;
	    dy += dy;
	} else if (dx > dy) {	// step in horizontal direction
	    missile->D = dx - 1;
	    dx += dx;
	    dy += dy;
	}

	missile->Dx = dx;
	missile->Dy = dy;
	missile->Xstep = xstep;
	missile->Ystep = ystep;
	missile->State++;
	DebugLevel3Fn("Init: %d,%d, %d,%d, =%d\n" _C_ dx _C_ dy _C_
	    xstep _C_ ystep _C_ missile->D);
	return 0;
    } else {
	// on the way
	dx = missile->Dx;
	dy = missile->Dy;
	xstep = missile->Xstep;
	ystep = missile->Ystep;
    }

    //
    //	Move missile
    //
    if (dy == 0) {		// horizontal line
	for (i = 0; i < missile->Type->Speed; ++i) {
	    if (missile->X == missile->DX) {
		return 1;
	    }
	    missile->X += xstep;
	}
	return 0;
    }

    if (dx == 0) {		// vertical line
	for (i = 0; i < missile->Type->Speed; ++i) {
	    if (missile->Y == missile->DY) {
		return 1;
	    }
	    missile->Y += ystep;
	}
	return 0;
    }

    if (dx < dy) {		// step in vertical direction
	for (i = 0; i < missile->Type->Speed; ++i) {
	    if (missile->Y == missile->DY) {
		return 1;
	    }
	    missile->Y += ystep;
	    missile->D -= dx;
	    if (missile->D < 0) {
		missile->D += dy;
		missile->X += xstep;
	    }
	}
	return 0;
    }

    if (dx > dy) {		// step in horizontal direction
	for (i = 0; i < missile->Type->Speed; ++i) {
	    if (missile->X == missile->DX) {
		return 1;
	    }
	    missile->X += xstep;
	    missile->D -= dy;
	    if (missile->D < 0) {
		missile->D += dx;
		missile->Y += ystep;
	    }
	}
	return 0;
    }
				// diagonal line
    for (i = 0; i < missile->Type->Speed; ++i) {
	if (missile->Y == missile->DY) {
	    return 1;
	}
	missile->X += xstep;
	missile->Y += ystep;
    }
    return 0;
}

/**
**	Calculate parabolic trajectories.
**
**	@param missile	Missile pointer.
**	@param amplitude    How high can the missile go. This value depends
**			    on the missile direction and game perspective.
*/
local int ParabolicCalc(Missile* missile, int amplitude)
{
    int xmid;
    long sinu;
    int thetha;

    missile->Xl -= missile->Xstep;
    missile->X = (missile->Xl +500) / 1000;

    xmid = (missile->SourceX + missile->DX) / 2;
    sinu = (missile->X - xmid) * (missile->X - xmid);
    thetha = missile->SourceX - xmid;
    missile->Y = ((missile->Angle * (missile->X - missile->SourceX)) -
	amplitude * isqrt(-sinu + thetha * thetha) + missile->SourceY * 1000 + 500 ) / 1000;

    return 0;
}

/**
**	Calculate parabolic trajectories.
**
**	@param missile	Missile pointer.
*/
local int ParabolicMissile(Missile* missile)
{
    int i;
    int sx;
    int sy;

    if (!(missile->State & 1)) {
	int dx;
	int dy;
	int xstep;
	int ystep;

	// initialize
	dy = missile->DY - missile->Y;
	ystep = 1;
	if (dy < 0) {
	    dy = -dy;
	    ystep = -1;
	}
	dx = missile->DX - missile->X;
	xstep = 1;
	if (dx < 0) {
	    dx = -dx;
	    xstep = -1;
	}
	if (missile->SourceX - missile->DX != 0) {
	    missile->Angle = (1000 * (missile->SourceY - missile->DY)) / 
		(missile->SourceX - missile->DX);
	} else {
	    missile->Angle = 1;
	}
	missile->Xl = missile->SourceX * 1000;

	MissileNewHeadingFromXY(missile, dx * xstep, dy * ystep);

	if (dx == 0 && dy == 0) {
	    return 1;
	}

	missile->Dx = dx;
	missile->Dy = dy;
	dx = missile->SourceX - missile->DX;
	dy = missile->SourceY - missile->DY;
	missile->Xstep = (1000 * dx) / isqrt(dx * dx + dy * dy);
	missile->Ystep = ystep;
	++missile->State;
	DebugLevel3Fn("Init: %d,%d\n" _C_ dx _C_ dy);
	return 0;
    }

    sx = missile->X;
    sy = missile->Y;

    //
    //	Move missile
    //
    if (missile->Dy == 0) {		// horizontal line
	for (i = 0; i < missile->Type->Speed; ++i) {
	    if (missile->X == missile->DX) {
		return 1;
	    }
	    ParabolicCalc(missile, 500);
	}
	MissileNewHeadingFromXY(missile, missile->X - sx, missile->Y - sy);
	return 0;
    }

    if (missile->Dx == 0) {		// vertical line
	for (i = 0; i < missile->Type->Speed; ++i) {
	    if (missile->Y == missile->DY) {
		return 1;
	    }					
	    missile->Y += missile->Ystep; //no parabolic missile there.
	}
	return 0;
    }

    for (i = 0; i < missile->Type->Speed; ++i) {
	if (abs(missile->X - missile->DX) <= 1 &&
		abs(missile->Y - missile->DY) <= 1) {
	    return 1;
	}
	ParabolicCalc(missile, 1000);
	MissileNewHeadingFromXY(missile, missile->X - sx, missile->Y - sy);
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
local void MissileHitsGoal(const Missile* missile, Unit* goal, int splash)
{
    if (!missile->Type->CanHitOwner && goal == missile->SourceUnit) {
	return;				// blizzard cannot hit owner unit
    }

    if (goal->HP && goal->Orders[0].Action != UnitActionDie) {
	if (missile->Damage) {		// direct damage, spells mostly
	    HitUnit(missile->SourceUnit, goal, missile->Damage / splash);
	} else {
	    HitUnit(missile->SourceUnit, goal,
		CalculateDamage(missile->SourceUnit->Stats, goal,
		    missile->SourceUnit->Bloodlust, 0) / splash);
	}
    }
}

/**
**	Missile hits wall.
**
**	@param missile	Missile hitting the goal.
**	@param x	Wall X map tile position.
**	@param y	Wall Y map tile position.
**	@param splash	Splash damage divisor.
**
**	@todo	FIXME: Support for more races.
*/
local void MissileHitsWall(const Missile* missile, int x, int y, int splash)
{
    if (WallOnMap(x, y)) {
	DebugLevel3Fn("Missile on wall?\n");
	if (HumanWallOnMap(x, y)) {
	    if (missile->Damage) {	// direct damage, spells mostly
		HitWall(x, y, missile->Damage / splash);
	    } else {
		HitWall(x, y,
		    CalculateDamageStats(missile->SourceUnit->Stats, 
			UnitTypeHumanWall->Stats, 0, 0) / splash);
	    }
	} else {
	    if (missile->Damage) {	// direct damage, spells mostly
		HitWall(x, y, missile->Damage / splash);
	    } else {
		HitWall(x, y,
		    CalculateDamageStats(missile->SourceUnit->Stats,
			UnitTypeOrcWall->Stats, 0, 0) / splash);
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

    if (missile->Type->ImpactSound.Sound) {
	PlayMissileSound(missile, missile->Type->ImpactSound.Sound);
    }

    x = missile->X + missile->Type->Width / 2;
    y = missile->Y + missile->Type->Height / 2;

    //
    //	The impact generates a new missile.
    //
    if (missile->Type->ImpactMissile) {
//	Missile* mis;

//	mis = 
	MakeMissile(missile->Type->ImpactMissile, x, y, x, y);
	// Impact missiles didn't generate any damage now.
#if 0
	mis->Damage = missile->Damage; // direct damage, spells mostly
	mis->SourceUnit = missile->SourceUnit;
	// FIXME: should copy target also?
	if (mis->SourceUnit) {
	    RefsDebugCheck(!mis->SourceUnit->Refs);
	    mis->SourceUnit->Refs++;
	}
#endif
    }

    if (!missile->SourceUnit) {		// no owner - green-cross ...
	DebugLevel3Fn("Missile has no owner!\n");
	return;
    }

    x /= TileSizeX;
    y /= TileSizeY;

    if (x < 0 || y < 0 || x >= TheMap.Width || y >= TheMap.Height) {
	// FIXME: this should handled by caller?
	DebugLevel0Fn("Missile gone outside of map!\n");
	return;				// outside the map.
    }

    //
    //	Choose correct goal.
    //
    if (!missile->Type->Range) {
	if (missile->TargetUnit) {
	    //
	    //	Missiles without range only hits the goal always.
	    //
	    goal = missile->TargetUnit;
	    if (goal->Destroyed) {			// Destroyed
		RefsDebugCheck(!goal->Refs);
		if (!--goal->Refs) {
		    ReleaseUnit(goal);
		}
		missile->TargetUnit = NoUnitP;
		return;
	    }
	    MissileHitsGoal(missile, goal, 1);
	    return;
	}
	MissileHitsWall(missile, x, y, 1);
	return;
    }

    //
    //	Hits all units in range.
    //
    i = missile->Type->Range;
    n = SelectUnits(x - i + 1, y - i + 1, x + i, y + i, table);
    for (i = 0; i < n; ++i) {
	goal = table[i];
	//
	//	Can the unit attack the this unit-type?
	//	NOTE: perhaps this should be come a property of the missile.
	//
	if (CanTarget(missile->SourceUnit->Type, goal->Type)) {
	    // We are attacking the nearest field of the unit
	    if (x < goal->X || y < goal->Y ||
		    x >= goal->X + goal->Type->TileWidth ||
		    y >= goal->Y + goal->Type->TileHeight) {
		MissileHitsGoal(missile, goal, 2);
	    } else {
		MissileHitsGoal(missile, goal, 1);
	    }
	}
    }
    //
    //	Missile hits ground.
    //
    // FIXME: no bock writting it correct.
    x -= missile->Type->Range;
    y -= missile->Type->Range;
    for (i = missile->Type->Range * 2; --i;) {
	for (n = missile->Type->Range * 2; --n;) {
	    if (x + i >= 0 && x + i < TheMap.Width && y + n >= 0 && y + n < TheMap.Height) {
		if (i == 0 && n == 0) {
		    MissileHitsWall(missile, x + i, y + n, 1);
		} else {
		    MissileHitsWall(missile, x + i, y + n, 2);
		}
	    }
	}
    }
}


/**
**	Pass to the next frame for animation.
**
**	@param missile : missile to animate.
**	@param SpriteFrame : number of frame for a row : must be remove
**	@return	1 if animation is finished, 0 else.
**	@todo remove SpriteFrame, and use number of frames per row (frames pro row)
*/
local int NextMissileFrame(Missile* missile, int SpriteFrame)
{
    int neg;
    int AnimationIsFinished;

    DebugCheck(missile == NULL);
//
//	Animate missile, cycle through frames
//
    neg = 0;
    AnimationIsFinished = 0;
    if (missile->SpriteFrame < 0) {
	neg = 1;
	missile->SpriteFrame = -missile->SpriteFrame;
    }
    missile->SpriteFrame += SpriteFrame;	// FIXME: frames pro row
    if (missile->SpriteFrame >= VideoGraphicFrames(missile->Type->Sprite)) {
	missile->SpriteFrame -= VideoGraphicFrames(missile->Type->Sprite);
	AnimationIsFinished = 1;
    }
    if (neg) {
	missile->SpriteFrame = -missile->SpriteFrame;
    }
    DebugLevel3Fn("Frame %d of %d\n" _C_
	missile->SpriteFrame _C_ VideoGraphicFrames(missile->Type->Sprite));
    return AnimationIsFinished;
}


/**
**	Handle action for a missile.
**
**	@param missile	Missile pointer.
*/
local void MissileAction(Missile* missile)
{
    DebugCheck(missile == NULL);
    // Mark missile area on screen to be drawn, if missile moves or disappears.
    CheckMissileToBeDrawn(missile);
    missile->Wait = missile->Type->Sleep;

    switch (missile->Type->Class) {
	//
	//	Missile flies from x,y to x1,y1
	//
	case MissileClassPointToPoint:
	    MissileActionPointToPoint(missile);
	    break;
	case MissileClassPointToPointWithDelay:
	    MissileActionPointToPointWithDelay(missile);
	    break;
	case MissileClassPointToPoint3Bounces:
	    MissileActionPointToPoint3Bounces(missile);
	    break;
	case MissileClassPointToPointWithHit:
	    MissileActionPointToPointWithHit(missile);
	case MissileClassBlizzard:
	    MissileActionPointToPointWithHit(missile);
	    break;
	case MissileClassDeathDecay:
	    MissileActionStayWithDelay(missile);
	    break;
	case MissileClassWhirlwind:
	    if (NextMissileFrame(missile, 1)) {
		missile->SpriteFrame = 0;
		PointToPointMissile(missile);
	    }
	    break;
	case MissileClassStayWithDelay:
	    MissileActionStayWithDelay(missile);
	    break;
	case MissileClassCycleOnce:
	    MissileActionCycleOnce(missile);
	    break;
	case MissileClassFire:
	    MissileActionFire(missile);
	    break;
	case MissileClassHit:
	    MissileActionHit(missile);
	    break;
	case MissileClassParabolic:
	    MissileActionParabolic(missile);
	    break;
	case MissileClassLandMine:
	    MissileActionLandMine(missile);
	    break;
    }
}

/**
**	Handle all missile actions of global/local missiles.
**
**	@param missiles		Table of missiles.
*/
local void MissilesActionLoop(Missile** missiles)
{
    Missile* missile;

    //
    //	NOTE: missiles[??] could be modified!!!
    //
    while ((missile = *missiles)) {
	DebugLevel3Fn("Missile %s ttl %d at %d, %d\n" _C_ missile->Type->Ident
		_C_ missile->TTL _C_ missile->X _C_ missile->Y);
	if (missile->Delay && missile->Delay--) {
	    ++missiles;
	    continue;		// delay start of missile
	}

	if (missile->TTL > 0) {
	    missile->TTL--;	// overall time to live if specified
	}

	if (missile->Controller) {
	    missile->Controller(missile);
	    if (*missiles != missile) {	// Missile is destroyed
		continue;
	    }
	}

	if (!missile->TTL) {
	    FreeMissile(missile);
	    continue;
	}

	if (--missile->Wait) {	// wait until time is over
	    ++missiles;
	    continue;
	}

	if (missile->Type->Class == MissileClassCustom) {
	    missile->Wait = missile->Type->Sleep;
	    ++missiles;
	    continue;	// custom missiles are handled by Controller() only
	}

	MissileAction(missile);

	if (*missiles == missile) {	// Missile not destroyed
	    ++missiles;
	}
    }
}

/**
**	Handle all missile actions.
*/
global void MissileActions(void)
{
    MissilesActionLoop(GlobalMissiles);
    MissilesActionLoop(LocalMissiles);
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

    DebugCheck(missile == NULL);
    x = (missile->X+missile->Type->Width / 2) / TileSizeX;
    y = (missile->Y+missile->Type->Height / 2) / TileSizeY;	// pixel -> tile

    DebugLevel3Fn("Missile %p at %d %d\n" _C_ missile _C_ x _C_ y);

    return ViewPointDistance(x, y);
}

/**
**	Get the burning building missile based on hp percent.
**
**	@param percent	HP percent
*/
global MissileType* MissileBurningBuilding(int percent)
{
    BurningBuildingFrame* frame;
    BurningBuildingFrame* tmp;

    frame = tmp = BurningBuildingFrames;
    while (tmp) {
	if (percent < tmp->Percent) {
	    break;
	}
	frame = tmp;
	tmp = tmp->Next;
    }
    return frame->Missile;
}

/**
**	Save the missile-types to file.
**
**	@param file	Output file.
**
**	@todo FIXME: CanHitOwner and FriendlyFire not supported!
*/
global void SaveMissileTypes(CLFile* file)
{
    MissileType* mtype;
    char** sp;
    int i;

    CLprintf(file, "\n;;; -----------------------------------------\n");
    CLprintf(file, ";;; MODULE: missile-types $Id$\n\n");

    //
    //	Original number to internal missile-type name.
    //
    i = CLprintf(file, "(define-missiletype-wc-names");
    for (sp = MissileTypeWcNames; *sp; ++sp) {
	if (i + strlen(*sp) > 79) {
	    i = CLprintf(file, "\n ");
	}
	i += CLprintf(file, " '%s", *sp);
    }
    CLprintf(file, ")\n\n");

    //
    //	Missile types
    //
    for (mtype = MissileTypes; mtype < &MissileTypes[NumMissileTypes]; ++mtype) {
	CLprintf(file, "(define-missile-type '%s\n ", mtype->Ident);
	if (mtype->File) {
	    CLprintf(file, " 'file \"%s\"", mtype->File);
	}
	CLprintf(file, " 'size '(%d %d)", mtype->Width, mtype->Height);
	if (mtype->Sprite) {
	    CLprintf(file, " 'frames %d", mtype->SpriteFrames);
	}
	CLprintf(file, "\n  'num-directions %d", mtype->NumDirections);
	CLprintf(file, "\n ");
	if (mtype->FiredSound.Name) {
	    CLprintf(file, " 'fired-sound \"%s\"", mtype->FiredSound.Name);
	}
	if (mtype->ImpactSound.Name) {
	    CLprintf(file, " 'impact-sound \"%s\"", mtype->ImpactSound.Name);
	}
	if (mtype->FiredSound.Name || mtype->ImpactSound.Name) {
	    CLprintf(file, "\n ");
	}
	CLprintf(file, " 'class '%s", MissileClassNames[mtype->Class]);
	CLprintf(file, " 'draw-level %d ", mtype->DrawLevel);
	if (mtype->StartDelay) {
	    CLprintf(file, " 'delay %d", mtype->StartDelay);
	}
	CLprintf(file, " 'sleep %d", mtype->Sleep);
	CLprintf(file, " 'speed %d", mtype->Speed);
	CLprintf(file, " 'range %d", mtype->Range);
	if (mtype->ImpactMissile) {
	    CLprintf(file, "\n  'impact-missile '%s", mtype->ImpactMissile->Ident);
	}
	CLprintf(file, "\n ");
	CLprintf(file, " 'can-hit-owner #%c", mtype->CanHitOwner ? 't' : 'f');
	CLprintf(file, " 'friendly-fire #%c", mtype->FriendlyFire ? 't' : 'f');
	CLprintf(file, ")\n");
    }
}

/**
**	Save the state of a missile to file.
*/
local void SaveMissile(const Missile* missile,CLFile* file)
{
    char* s1;

    CLprintf(file, "(missile 'type '%s",missile->Type->Ident);
    CLprintf(file, " 'pos '(%d %d) 'goal '(%d %d)",
	missile->X, missile->Y, missile->DX, missile->DY);
    CLprintf(file, " '%s", missile->Local ? "local" : "global");
    CLprintf(file, "\n  'frame %d 'state %d 'wait %d 'delay %d\n ",
	missile->SpriteFrame, missile->State, missile->Wait, missile->Delay);
    if (missile->SourceUnit) {
	CLprintf(file, " 'source '%s", s1 = UnitReference(missile->SourceUnit));
	free(s1);
    }
    if (missile->TargetUnit) {
	CLprintf(file, " 'target '%s", s1 = UnitReference(missile->TargetUnit));
	free(s1);
    }
    CLprintf(file, " 'damage %d", missile->Damage);
    // FIXME: need symbolic names for controller
    CLprintf(file, " 'ttl %d 'controller %ld",
	missile->TTL, (long)missile->Controller);
    CLprintf(file, " 'data '(%d %d %d %d %d)",
	missile->D, missile->Dx, missile->Dy, missile->Xstep, missile->Ystep);
    CLprintf(file, ")\n");
}

/**
**	Save the state missiles to file.
**
**	@param file	Output file.
*/
global void SaveMissiles(CLFile* file)
{
    Missile* const* missiles;

    CLprintf(file, "\n;;; -----------------------------------------\n");
    CLprintf(file, ";;; MODULE: missiles $Id$\n\n");

    for (missiles = GlobalMissiles; *missiles; ++missiles) {
	SaveMissile(*missiles, file);
    }
    for (missiles = LocalMissiles; *missiles; ++missiles) {
	SaveMissile(*missiles, file);
    }
}

/**
**	Initialize missile-types.
*/
global void InitMissileTypes(void)
{
    MissileType* mtype;

    for (mtype = MissileTypes; mtype->OType; ++mtype) {
	//
	//	Add missile names to hash table
	//
	*(MissileType**)hash_add(MissileTypeHash, mtype->Ident) = mtype;

	//
	//	Resolve impact missiles and sounds.
	//
	if (mtype->FiredSound.Name) {
	    mtype->FiredSound.Sound = SoundIdForName(mtype->FiredSound.Name);
	}
	if (mtype->ImpactSound.Name) {
	    mtype->ImpactSound.Sound = SoundIdForName(mtype->ImpactSound.Name);
	}
	if (mtype->ImpactName) {
	    mtype->ImpactMissile = MissileTypeByIdent(mtype->ImpactName);
	}
    }
}

/**
**	Clean up missile-types.
*/
global void CleanMissileTypes(void)
{
    MissileType* mtype;

    for (mtype = MissileTypes; mtype->OType; ++mtype) {
	hash_del(MissileTypeHash, mtype->Ident);

	free(mtype->Ident);
	free(mtype->File);
	free(mtype->FiredSound.Name);
	free(mtype->ImpactSound.Name);
	free(mtype->ImpactName);

	VideoSaveFree(mtype->Sprite);
    }
    free(MissileTypes);
    MissileTypes = NULL;
    NumMissileTypes = 0;
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
    Missile** missiles;
    Missile* missile;
    char** sp;

    for (missiles = GlobalMissiles; (missile = *missiles); ++missiles) {
	free(missile);
	*missiles = NULL;
    }
    NumGlobalMissiles = 0;
    for (missiles = LocalMissiles; (missile = *missiles); ++missiles) {
	free(missile);
	*missiles = NULL;
    }
    NumLocalMissiles = 0;

    if (MissileTypeWcNames) {
	for (sp = MissileTypeWcNames; *sp; ++sp) {
	    free(*sp);
	}
	free(MissileTypeWcNames);
	MissileTypeWcNames = NULL;
    }
}

/*----------------------------------------------------------------------------
--	Functions (Spells Controllers/Callbacks) TODO move to anoher file?
----------------------------------------------------------------------------*/

// ****************************************************************************
// Actions for the missiles
// ****************************************************************************

/*
** Missile controllers
**
** To cancel a missile set it's TTL to 0, it will be handled right after
** the controller call and missile will be down.
**
*/

/*
**      Missile does nothing
**	
**	@param missile	pointer to missile
*/
void MissileActionNone(Missile* missile)
{
    return;//  Busy doing nothing.
}

/*
**      Missile flies from x,y to x1,y1
**	
**	@param missile	pointer to missile
*/
void MissileActionPointToPoint(Missile* missile)
{
    if (PointToPointMissile(missile)) {
	MissileHit(missile);
	FreeMissile(missile);
    } else {
	NextMissileFrame(missile, 5);
    }
}

/*
**      Missile flies from x,y to x1,y1 and stays there for a moment
**	
**	@param missile	pointer to missile
*/
void MissileActionPointToPointWithDelay(Missile* missile)
{
    int neg;
    int totalx;
    int dx;
    int f;
    int i;
    int j;

    if (PointToPointMissile(missile)) {
	MissileHit(missile);
	FreeMissile(missile);
    } else {
	neg = 0;
	if (missile->SpriteFrame < 0) {
	    neg = 1;
	    missile->SpriteFrame = -missile->SpriteFrame;
	}
	totalx = abs(missile->DX - missile->SourceX);
	dx = abs(missile->X - missile->SourceX);
	f = VideoGraphicFrames(missile->Type->Sprite) / 5; // FIXME: frames per row
	f = 2 * f - 1;
	for (i = 1, j = 1; i <= f; ++i) {
	    if (dx * f / i < totalx) {
		if ((i - 1) * 2 < f) {
		    j = i - 1;
		} else {
		    j = f - i;
		}
		missile->SpriteFrame = missile->SpriteFrame % 5 +
		    j * 5; // FIXME: frames per row
		break;
	    }
	}
	if (neg) {
	    missile->SpriteFrame = -missile->SpriteFrame;
	}
    }
}

/*
**      Missile don't move, than disappears
**	
**	@param missile	pointer to missile
*/
void MissileActionStayWithDelay(Missile* missile)
{
    if (NextMissileFrame(missile, 1)) {
	MissileHit(missile);
	FreeMissile(missile);
    }
}

/**
**      Missile flies from x,y to x1,y1 than bounces three times.
**	
**	@param missile	pointer to missile
*/
void MissileActionPointToPoint3Bounces(Missile* missile)
{
    if (PointToPointMissile(missile)) {
	//	3 Bounces.
	switch (missile->State) {
	    case 1:
	    case 3:
	    case 5:
		missile->State += 2;
		missile->DX += missile->Xstep * TileSizeX * 3 / 2;
		missile->DY += missile->Ystep * TileSizeY * 3 / 2;
		MissileHit(missile);
		// FIXME: hits to left and right
		// FIXME: reduce damage effects on later impacts
		break;
	    default:
		FreeMissile(missile);
		missile = NULL;
		break;
	}
    } else {
	NextMissileFrame(missile, 5);
    }
}

/**
**	FIXME: docu
**
**	@param missile	pointer to missile
*/
void MissileActionCycleOnce(Missile* missile)
{
    int neg;

    neg = 0;
    if (missile->SpriteFrame < 0) {
	neg = 1;
	missile->SpriteFrame = -missile->SpriteFrame;
    }
    switch (missile->State) {
	case 0:
	case 2:
	    ++missile->State;
	    break;
	case 1:
	    if (++missile->SpriteFrame == VideoGraphicFrames(missile->Type->Sprite)) {
		--missile->SpriteFrame;
		++missile->State;
	    }
	    break;
	case 3:
	    if (!missile->SpriteFrame--) {
		MissileHit(missile);
		FreeMissile(missile);
		missile = NULL;
	    }
	    break;
    }
    if (neg && missile) {
	missile->SpriteFrame = -missile->SpriteFrame;
    }
}

/*
**      Missile flies from x,y to x1,y1 than shows hit animation.
**
**	@param missile	pointer to missile
*/
void MissileActionPointToPointWithHit(Missile* missile)
{
    if (PointToPointMissile(missile)) {
	if (NextMissileFrame(missile, 4)) {
	    MissileHit(missile);
	    FreeMissile(missile);
	}
    }
}
	
/*
**      Missile don't move, than checks the source unit for HP.
**	
**	@param missile	pointer to missile
*/
void MissileActionFire(Missile* missile)
{
    Unit* unit;

    unit = missile->SourceUnit;
    if (unit->Destroyed || !unit->HP) {
	FreeMissile(missile);
	return;
    }
    if (NextMissileFrame(missile, 1)) {
	int f;
	MissileType* fire;

	missile->SpriteFrame = 0;
	f = (100 * unit->HP) / unit->Stats->HitPoints;
	fire = MissileBurningBuilding(f);
	if (!fire) {
	    FreeMissile(missile);
	    missile = NULL;
	    unit->Burning = 0;
	} else {
	    if (missile->Type != fire) {
		missile->X += missile->Type->Width / 2;
		missile->Y += missile->Type->Height / 2;
		missile->Type = fire;
		missile->X -= missile->Type->Width / 2;
		missile->Y -= missile->Type->Height / 2;
	    }
	}
    }
}

/**
**	FIXME: docu
**
**	@param missile	pointer to missile
*/
void MissileActionHit(Missile* missile)
{
    if (PointToPointMissile(missile)) {
	MissileHit(missile);
	FreeMissile(missile);
    }
}

/*
**	Missile flies from x,y to x1,y1 using a parabolic path
**	
**	@param missile	pointer to missile
*/
void MissileActionParabolic(Missile* missile)
{
    int neg;

    if (ParabolicMissile(missile)) {
	MissileHit(missile);
	FreeMissile(missile);
	missile = NULL;
    } else {
	int totalx;
	int dx;
	int f;
	int i;
	int j;

	neg = 0;
	if (missile->SpriteFrame < 0) {
	    neg = 1;
	    missile->SpriteFrame = -missile->SpriteFrame;
	}
	totalx = abs(missile->DX - missile->SourceX);
	dx = abs(missile->X - missile->SourceX);
	f = VideoGraphicFrames(missile->Type->Sprite) / 5; // FIXME: frames per row
	f = 2 * f - 1;
	for (i = 1, j = 1; i <= f; ++i) {
	    if (dx * f / i < totalx) {
		if ((i - 1) * 2 < f) {
		    j = i - 1;
		} else {
		    j = f - i;
		}
		missile->SpriteFrame = missile->SpriteFrame % 5 +
		    j * 5; // FIXME: frames per row
		break;
	    }
	}
	if (neg) {
	    missile->SpriteFrame = -missile->SpriteFrame;
	}
    }
}

/**
**	Death-Coil controller
**
**	@param missile	Controlled missile
*/
global void SpellDeathCoilController(Missile *missile)
{
    Unit *table[UnitMax];
    int	i;
    int	n;
    Unit *source;

    DebugCheck(missile == NULL);
    //
    //  missile has not reached target unit/spot
    //
    if (!(missile->X == missile->DX && missile->Y == missile->DY)) {
	return ;
    }
    source = missile->SourceUnit;
    if (source->Destroyed) {
	return ;
    }
    // source unit still exists
    //
    //	Target unit still exists and casted on a special target
    //
    if (missile->TargetUnit && !missile->TargetUnit->Destroyed
	    && missile->TargetUnit->HP)  {
	if (missile->TargetUnit->HP <= 50) {// 50 should be parametrable
	    source->Player->Score += missile->TargetUnit->Type->Points;
	    if (missile->TargetUnit->Type->Building) {
		source->Player->TotalRazings++;
	    } else {
		source->Player->TotalKills++;
	    }
#ifdef USE_HP_FOR_XP
	    source->XP += missile->TargetUnit->HP;
#else
	    source->XP += missile->TargetUnit->Type->Points;
#endif
	    ++source->Kills;
	    missile->TargetUnit->HP = 0;
	    LetUnitDie(missile->TargetUnit);
	} else {
#ifdef USE_HP_FOR_XP
	    source->XP += 50;
#endif
	    missile->TargetUnit->HP -= 50;
	}
	if (source->Orders[0].Action != UnitActionDie) {
	    source->HP += 50;
	    if (source->HP > source->Stats->HitPoints) {
		source->HP = source->Stats->HitPoints;
	    }
	}
    } else {
	//
	//  No target unit -- try enemies in range 5x5 // Must be parametrable
	//
	int ec;		// enemy count
	int x;
	int y;

	ec = 0;
	x = missile->DX / TileSizeX;
	y = missile->DY / TileSizeY;

	n = SelectUnits(x - 2, y - 2, x + 2, y + 2, table);
	if (n == 0) {
	    return ;
	}
	// calculate organic enemy count
	for (i = 0; i < n; ++i) {
	    ec += (IsEnemy(source->Player, table[i])
	    && table[i]->Type->Organic != 0);
	}
	if (ec > 0)  {
	    // yes organic enemies found
	    for (i = 0; i < n; ++i) {
		if (IsEnemy(source->Player, table[i]) && table[i]->Type->Organic != 0) {
		    // disperse damage between them
		    //NOTE: 1 is the minimal damage
		    if (table[i]->HP <= 50 / ec) {
			source->Player->Score += table[i]->Type->Points;
			if (table[i]->Type->Building) {
			    source->Player->TotalRazings++;
			} else {
			    source->Player->TotalKills++;
			}
#ifdef USE_HP_FOR_XP
			source->XP += table[i]->HP;
#else
			source->XP += table[i]->Type->Points;
#endif
			++source->Kills;
			table[i]->HP = 0;
			LetUnitDie(table[i]); // too much damage
		    } else {
#ifdef USE_HP_FOR_XP
			source->XP += 50/ec;
#endif
			table[i]->HP -= 50 / ec;
		    }
		}
	    }
	    if (source->Orders[0].Action!=UnitActionDie) {
		source->HP += 50;
		if (source->HP > source->Stats->HitPoints) {
		    source->HP = source->Stats->HitPoints;
		}
	    }
	}
    }
}

/**
**	Fireball controller
**
**	@param missile	Controlled missile
*/
global void SpellFireballController(Missile *missile)
{
    DebugCheck(missile == NULL);
    //NOTE: vladi: TTL is used as counter for explosions
    // explosions start at target and continue (10 tiles) beyond
    // explosions are on each tile on the way

    // approx
    if (missile->TTL <= missile->State && missile->TTL % 4 == 0) {
	MissileHit(missile);
    }
}

/**
**	FlameShield controller
**
**	@param missile	Controlled missile
*/
global void SpellFlameShieldController(Missile *missile)
{
    static int fs_dc[] = {
	0, 32, 5, 31, 10, 30, 16, 27, 20, 24, 24, 20, 27, 15, 30, 10, 31,
	5, 32, 0, 31, -5, 30, -10, 27, -16, 24, -20, 20, -24, 15, -27, 10,
	-30, 5, -31, 0, -32, -5, -31, -10, -30, -16, -27, -20, -24, -24, -20,
	-27, -15, -30, -10, -31, -5, -32, 0, -31, 5, -30, 10, -27, 16, -24,
	20, -20, 24, -15, 27, -10, 30, -5, 31, 0, 32};
    Unit *table[UnitMax];
    int n;
    int i;
    int dx;
    int dy;
    int ux;
    int uy;
    int ix;
    int iy;
    int uw;
    int uh;

    DebugCheck(missile == NULL);
    i = missile->TTL % 36;		// 36 positions on the circle
    dx = fs_dc[i * 2];
    dy = fs_dc[i * 2 + 1];
    ux = missile->TargetUnit->X;
    uy = missile->TargetUnit->Y;
    ix = missile->TargetUnit->IX;
    iy = missile->TargetUnit->IY;
    uw = missile->TargetUnit->Type->Width;
    uh = missile->TargetUnit->Type->Height;
    missile->X = ux * TileSizeX + ix + uw / 2 + dx - 32;
    missile->Y = uy * TileSizeY + iy + uh / 2 + dy - 32 - 16;
    if (missile->TargetUnit->Orders[0].Action == UnitActionDie) {
	missile->TTL = i;
    }
    if (missile->TTL == 0) {
	missile->TargetUnit->FlameShield = 0;
    }
    //vladi: still no have clear idea what is this about :)
    CheckMissileToBeDrawn(missile);

    // Only hit 1 out of 8 frames
    if (missile->TTL & 7) {
	return;
    }
    n = SelectUnits(ux - 1, uy - 1, ux + 1 + 1, uy + 1 + 1, table);
    for (i = 0; i < n; ++i) {
	if (table[i] == missile->TargetUnit) {
	    // cannot hit target unit
	    continue;
	}
	if (table[i]->HP) {
	    HitUnit(missile->SourceUnit, table[i], missile->Damage);
	}
    }
}

/**
**	Runes controller
**
**	@param missile	Controlled missile
*/
global void MissileActionLandMine(Missile *missile)
{
    Unit *table[UnitMax];
    int i;
    int n;
    int x;
    int y;

    DebugCheck(missile == NULL);
    DebugCheck(TileSizeX == 0);
    DebugCheck(TileSizeY == 0);

    x = missile->X / TileSizeX;
    y = missile->Y / TileSizeY;

    n = SelectUnitsOnTile(x, y, table);
    for (i = 0; i < n; ++i) {
	if (table[i]->Type->UnitType != UnitTypeFly &&
		table[i]->HP &&
		table[i]!=missile->SourceUnit) {
	    DebugLevel0("Landmine explosion.\n");
	    MissileHit(missile);
	    missile->TTL = 0;
	    return;
	}
    }

    NextMissileFrame(missile, 1);
}

/**
**	Whirlwind controller
**
**	@param missile	Controlled missile
*/
global void SpellWhirlwindController(Missile *missile)
{
    Unit *table[UnitMax];
    int i;
    int n;
    int x;
    int y;

    DebugCheck(missile == NULL);
    //
    //	Center of the tornado
    //
    x = (missile->X+TileSizeX/2+missile->Type->Width/2) / TileSizeX;
    y = (missile->Y+TileSizeY+missile->Type->Height/2) / TileSizeY;
    //
    //	Every 4 cycles 4 points damage in tornado center
    //
    if (!(missile->TTL % 4)) {
	n = SelectUnitsOnTile(x, y, table);
	for (i = 0; i < n; ++i)	{
	    if (table[i]->HP) {
		HitUnit(missile->SourceUnit,table[i], WHIRLWIND_DAMAGE1);// should be missile damage ?
	    }
	}
    }
    //
    //	Every 1/10s 1 points damage on tornado periphery
    //
    if (!(missile->TTL % (CYCLES_PER_SECOND/10))) {
    	// we should parameter this
	n = SelectUnits(x - 1, y - 1, x + 1, y + 1, table);
	DebugLevel3Fn("Damage on %d,%d-%d,%d = %d\n" _C_ x-1 _C_ y-1 _C_ x+1 _C_ y+1 _C_ n);
	for (i = 0; i < n; ++i) {
	    if ((table[i]->X != x || table[i]->Y != y) && table[i]->HP) {
		HitUnit(missile->SourceUnit,table[i], WHIRLWIND_DAMAGE2); // should be in missile
	    }
	}
    }
    DebugLevel3Fn("Whirlwind: %d, %d, TTL: %d\n" _C_
	missile->X _C_ missile->Y _C_ missile->TTL);

    //
    //	Changes direction every 3 seconds (approx.)
    //
    if (!(missile->TTL % 100)) { // missile has reached target unit/spot
	int nx;
	int ny;

	do {
	    // find new destination in the map
	    nx = x + SyncRand() % 5 - 2;
	    ny = y + SyncRand() % 5 - 2;
	} while (nx < 0 && ny < 0 && nx >= TheMap.Width && ny >= TheMap.Height);
	missile->DX = nx * TileSizeX + TileSizeX / 2;
	missile->DY = ny * TileSizeY + TileSizeY / 2;
	missile->State=0;
	DebugLevel3Fn("Whirlwind new direction: %d, %d, TTL: %d\n" _C_
	    missile->X _C_ missile->Y _C_ missile->TTL);
    }
}

//@}
