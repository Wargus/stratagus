//       _________ __                 __
//      /   _____//  |_____________ _/  |______     ____  __ __  ______
//      \_____  \\   __\_  __ \__  \\   __\__  \   / ___\|  |  \/  ___/
//      /        \|  |  |  | \// __ \|  |  / __ \_/ /_/  >  |  /\___ |
//     /_______  /|__|  |__|  (____  /__| (____  /\___  /|____//____  >
//             \/                  \/          \//_____/            \/
//  ______________________                           ______________________
//                        T H E   W A R   B E G I N S
//         Stratagus - A free fantasy real time strategy game engine
//
/**@name missile.c - The missiles. */
//
//      (c) Copyright 1998-2004 by Lutz Sammer and Jimmy Salmon
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
//      $Id$

//@{

/*----------------------------------------------------------------------------
--  Includes
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
--  Declarations
----------------------------------------------------------------------------*/

/**
**  Missile class names, used to load/save the missiles.
*/
global const char* MissileClassNames[] = {
	"missile-class-none",
	"missile-class-point-to-point",
	"missile-class-point-to-point-with-hit",
	"missile-class-point-to-point-cycle-once",
	"missile-class-point-to-point-bounce",
	"missile-class-stay",
	"missile-class-cycle-once",
	"missile-class-fire",
	"missile-class-hit",
	"missile-class-parabolic",
	"missile-class-land-mine",
	"missile-class-whirlwind",
	"missile-class-flame-shield",
	"missile-class-death-coil",
	NULL
};

local FuncController *MissileClassFunctions[] = {
	MissileActionNone,
	MissileActionPointToPoint,
	MissileActionPointToPointWithHit,
	MissileActionPointToPointCycleOnce,
	MissileActionPointToPointBounce,
	MissileActionStay,
	MissileActionCycleOnce,
	MissileActionFire,
	MissileActionHit,
	MissileActionParabolic,
	MissileActionLandMine,
	MissileActionWhirlwind,
	MissileActionFlameShield,
	MissileActionDeathCoil
};

/**
**  Missile type type definition
*/
global const char MissileTypeType[] = "missile-type";

/**
**  W*rCr*ft number to internal missile-type name.
*/
global char** MissileTypeWcNames;

global MissileType** MissileTypes;              /// Missile types.
global int NumMissileTypes;                     /// number of missile-types made.

#ifdef DEBUG
global int NoWarningMissileType;                /// quiet ident lookup.
#endif

local Missile* GlobalMissiles[MAX_MISSILES];    /// all global missiles on map
local int NumGlobalMissiles;                    /// currently used missiles

local Missile* LocalMissiles[MAX_MISSILES * 8]; /// all local missiles on map
local int NumLocalMissiles;                     /// currently used missiles

#ifdef DOXYGEN  // no real code, only for document

	/// lookup table for missile names
local MissileType* MissileTypeHash[61];

#else

	/// lookup table for missile names
local hashtable(MissileType*, 61) MissileTypeHash;

#endif

global BurningBuildingFrame* BurningBuildingFrames; /// Burning building frames

/*----------------------------------------------------------------------------
--  Functions
----------------------------------------------------------------------------*/

/**
**  Load the graphics for a missile type
**
**  @param missile  Type of missile to Load
*/
global void LoadMissileSprite(MissileType* mtype)
{
	const char* file;

	if ((file = mtype->File)) {
		char* buf;

		buf = alloca(strlen(file) + 9 + 1);
		file = strcat(strcpy(buf, "graphics/"), file);
		mtype->Sprite = LoadSprite(
			file, mtype->Width, mtype->Height);
		FlipGraphic(mtype->Sprite);

		// Correct the number of frames in graphic
		DebugCheck(mtype->Sprite->NumFrames < mtype->SpriteFrames);
		mtype->Sprite->NumFrames = mtype->SpriteFrames;
		// FIXME: Don't use NumFrames as number of frames.
	}
}

/**
**  Load the graphics for all missiles types
*/
global void LoadMissileSprites(void)
{
#ifndef DYNAMIC_LOAD
	int i;

	for (i = 0; i < NumMissileTypes; ++i) {
		LoadMissileSprite(MissileTypes[i]);
	}
#endif
}
/**
**  Get Missile type by identifier.
**
**  @param ident  Identifier.
**
**  @return       Missile type pointer.
*/
global MissileType* MissileTypeByIdent(const char* ident)
{
	MissileType* const* mtype;

	mtype = (MissileType**)hash_find(MissileTypeHash, (char*)ident);
	return mtype ? *mtype : 0;
}

/**
**  Allocate an empty missile-type slot.
**
**  @param ident  Identifier to identify the slot.
**
**  @return       New allocated (zeroed) missile-type pointer.
*/
global MissileType* NewMissileTypeSlot(char* ident)
{
	MissileType* mtype;
	int i;

	MissileTypes = realloc(MissileTypes, (NumMissileTypes + 1) * sizeof(MissileType *));
	mtype = MissileTypes[NumMissileTypes++] = (MissileType *)malloc(sizeof(MissileType));
	memset(mtype, 0, sizeof(MissileType));
	mtype->Ident = ident;

	// Rehash.
	for (i = 0; i < NumMissileTypes; ++i) {
		*(MissileType**)hash_add(MissileTypeHash, MissileTypes[i]->Ident) = MissileTypes[i];
	}

	return mtype;
}

/**
**  Allocate memory for a new global missile.
*/
local Missile* NewGlobalMissile(void)
{
	Missile* missile;

	// Check maximum missiles!
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
**  Allocate memory for a new local missile.
*/
local Missile* NewLocalMissile(void)
{
	Missile* missile;

	// Check maximum missiles!
	if (NumLocalMissiles == MAX_MISSILES * 8) {
		fprintf(stderr, "Maximum of local missiles reached\n");
		abort();
		return NULL;
	}

	missile = calloc(1, sizeof(Missile));
	memset(missile, 0, sizeof(*missile));
	missile->MissileSlot = LocalMissiles + NumLocalMissiles;
	LocalMissiles[NumLocalMissiles++] = missile;
	missile->Local = 1;

	return missile;
}

/**
**  Initialize a new made missile.
**
**  @param missile  Pointer to new uninitialized missile.
**  @param mtype    Type pointer of missile.
**  @param sx       Missile x start point in pixel.
**  @param sy       Missile y start point in pixel.
**  @param dx       Missile x destination point in pixel.
**  @param dy       Missile y destination point in pixel.
**
**  @return         created missile.
*/
local Missile* InitMissile(Missile* missile, MissileType* mtype, int sx,
	int sy, int dx, int dy)
{
	missile->X = sx - mtype->Width / 2;
	missile->Y = sy - mtype->Height / 2;
	missile->DX = dx - mtype->Width / 2;
	missile->DY = dy - mtype->Height / 2;
	missile->SourceX = missile->X;
	missile->SourceY = missile->Y;
	missile->Type = mtype;
	missile->SpriteFrame = 0;
	missile->State = 0;
	missile->AnimWait = 0;
	missile->Wait = mtype->Sleep;        // initial wait = sleep
	missile->Delay = mtype->StartDelay;  // initial delay

	missile->SourceUnit = NULL;

	missile->Damage = 0;
	missile->Hidden = 0;
	missile->TargetUnit = NULL;
	missile->TTL = -1;

	return missile;
}

/**
**  Create a new global missile at (x,y).
**
**  @param mtype  Type pointer of missile.
**  @param sx     Missile x start point in pixel.
**  @param sy     Missile y start point in pixel.
**  @param dx     Missile x destination point in pixel.
**  @param dy     Missile y destination point in pixel.
**
**  @return       created missile.
*/
global Missile* MakeMissile(MissileType* mtype, int sx, int sy, int dx, int dy)
{
	Missile* missile;

	DebugLevel3Fn("type %d(%s) at %d,%d to %d,%d\n" _C_
		mtype - MissileTypes _C_ mtype->Ident _C_ sx _C_ sy _C_ dx _C_ dy);

	if (!(missile = NewGlobalMissile())) {
		return missile;
	}

	return InitMissile(missile, mtype, sx, sy, dx, dy);
}

/**
**  Create a new local missile at (x,y).
**
**  @param mtype  Type pointer of missile.
**  @param sx     Missile x start point in pixel.
**  @param sy     Missile y start point in pixel.
**  @param dx     Missile x destination point in pixel.
**  @param dy     Missile y destination point in pixel.
**
**  @return       created missile.
*/
global Missile* MakeLocalMissile(MissileType* mtype, int sx, int sy, int dx, int dy)
{
	Missile* missile;

	DebugLevel3Fn("type %d(%s) at %d,%d to %d,%d\n" _C_
		mtype - MissileTypes _C_ mtype->Ident _C_ sx _C_ sy _C_ dx _C_ dy);

	if (!(missile = NewLocalMissile())) {
		return NULL;
	}

	return InitMissile(missile, mtype, sx, sy, dx, dy);
}

/**
**  Free a missile.
**
**  @param missile  Missile pointer.
*/
local void FreeMissile(Missile* missile)
{
	Missile* temp;
	Unit* unit;

	//
	// Release all unit references.
	//
	if ((unit = missile->SourceUnit)) {
		RefsDecrease(unit);
	}
	if((unit = missile->TargetUnit)) {
		RefsDecrease(unit);
	}

	//
	// Free the missile memory
	// Note: removing the last missile works.
	//
	if (missile->Local) {
		DebugCheck(*missile->MissileSlot != missile);
		temp = LocalMissiles[--NumLocalMissiles];
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
**  Calculate damage.
**
**  @todo NOTE: different targets (big are hit by some missiles better)
**  @todo NOTE: lower damage for hidden targets.
**  @todo NOTE: lower damage for targets on higher ground.
**
**  @param attacker_stats  Attacker attributes.
**  @param goal_stats      Goal attributes.
**  @param bloodlust       If attacker has bloodlust
**  @param xp              Experience of attacker.
**
**  @return                damage inflicted to goal.
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

	damage = max(basic_damage - goal_stats->Armor, 1) + piercing_damage;
	damage -= SyncRand() % ((damage + 2) / 2);
	DebugCheck(damage < 0);

	DebugLevel3Fn("\nDamage done [%d] %d %d ->%d\n" _C_ goal_stats->Armor _C_
		basic_damage _C_ piercing_damage _C_ damage);

	return damage;
}

/**
**  Calculate damage.
**
**  @param attacker_stats  Attacker attributes.
**  @param goal            Goal unit.
**  @param bloodlust       If attacker has bloodlust
**  @param xp              Experience of attack.
**
**  @return                damage produces on goal.
*/
local int CalculateDamage(const UnitStats* attacker_stats,
	const Unit* goal, int bloodlust, int xp)
{
	return CalculateDamageStats(attacker_stats, goal->Stats, bloodlust, xp);
}

/**
**  Fire missile.
**
**  @param unit  Unit that fires the missile.
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
	// Goal dead?
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
	// No missile hits immediately!
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

	x = unit->X * TileSizeX + TileSizeX / 2;  // missile starts in tile middle
	y = unit->Y * TileSizeY + TileSizeY / 2;

	if (goal) {
		DebugCheck(!goal->Type);  // Target invalid?
		//
		// Moved out of attack range?
		//
		if (MapDistanceBetweenUnits(unit, goal) < unit->Type->MinAttackRange) {
			DebugLevel0Fn("Missile target too near %d,%d\n" _C_
				MapDistanceBetweenUnits(unit,goal) _C_ unit->Type->MinAttackRange);
			// FIXME: do something other?
			return;
		}
		// Fire to nearest point of the unit!
		NearestOfUnit(goal, unit->X, unit->Y, &dx, &dy);
		DebugLevel3Fn("Fire to unit at %d,%d\n" _C_ dx _C_ dy);
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
	//		Damage of missile
	//
	if (goal) {
		missile->TargetUnit = goal;
		RefsIncrease(goal);
	}
	missile->SourceUnit = unit;
	RefsIncrease(unit);
}

/**
**  Get area of tiles covered by missile
**
**  @param missile  Missile to be checked and set.
**  @param sx       OUT: Pointer to X of top left corner in map tiles.
**  @param sy       OUT: Pointer to Y of top left corner in map tiles.
**  @param ex       OUT: Pointer to X of bottom right corner in map tiles.
**  @param ey       OUT: Pointer to Y of bottom right corner in map tiles.
**
**  @return         sx,sy,ex,ey defining area in Map
*/
local void GetMissileMapArea(const Missile* missile, int* sx, int* sy,
	int* ex, int* ey)
{
#define Bound(x, y) (x) < 0 ? 0 : ((x) > (y) ? (y) : (x))
	*sx = Bound(missile->X / TileSizeX, TheMap.Width - 1);
	*sy = Bound(missile->Y / TileSizeY, TheMap.Height - 1);
	*ex = Bound((missile->X + missile->Type->Width) / TileSizeX, TheMap.Width - 1);
	*ey = Bound((missile->Y + missile->Type->Height) / TileSizeY, TheMap.Height - 1);
#undef Bound
}

/**
**  Check missile visibility in a given viewport.
**
**  @param vp       Viewport to be checked.
**  @param missile  Missile pointer to check if visible.
**
**  @return         Returns true if visible, false otherwise.
*/
local int MissileVisibleInViewport(const Viewport* vp, const Missile* missile)
{
	int min_x;
	int max_x;
	int min_y;
	int max_y;
	int x;
	int y;

	GetMissileMapArea(missile, &min_x, &min_y, &max_x, &max_y);
	if (!AnyMapAreaVisibleInViewport(vp, min_x, min_y, max_x, max_y)) {
		return 0;
	}
	DebugLevel3Fn("Missile bounding box %d %d %d %d\n" _C_ min_x _C_ max_x _C_
		min_y _C_ max_y);

	for (x = min_x; x <= max_x; ++x) {
		for ( y = min_y; y <= max_y; ++y) {
			if (ReplayRevealMap || IsMapFieldVisible(ThisPlayer, x, y)) {
				return 1;
			}
		}
	}
	return 0;
}

/**
**  Check and sets if missile must be drawn on screen-map
**
**  @param missile  Missile to be checked.
**  @return         True if map marked to be drawn, false otherwise.
*/
global int CheckMissileToBeDrawn(const Missile* missile)
{
	int sx;
	int sy;
	int ex;
	int ey;

	GetMissileMapArea(missile, &sx, &sy, &ex, &ey);
	return MarkDrawAreaMap(sx, sy, ex, ey);
}

/**
**  Draw missile.
**
**  @param mtype  Missile type
**  @param frame  Animation frame
**  @param x      Screen pixel X position
**  @param y      Screen pixel Y position
*/
global void DrawMissile(MissileType* mtype, int frame, int x, int y)
{
	if (!mtype->Sprite) {
		LoadMissileSprite(mtype);
	}

	if (mtype->Flip) {
		if (frame < 0) {
			if (mtype->Transparency == 50) {
				VideoDrawClipXTrans50(mtype->Sprite, -frame - 1, x, y);
			} else {
				VideoDrawClipX(mtype->Sprite, -frame - 1, x, y);
			}
		} else {
			if (mtype->Transparency == 50) {
				VideoDrawClipTrans50(mtype->Sprite, frame, x, y);
			} else {
				VideoDrawClip(mtype->Sprite, frame, x, y);
			}
		}
	} else {
		int row;

		row = mtype->NumDirections / 2 + 1;
		if (frame < 0) {
			frame = ((-frame - 1) / row) * mtype->NumDirections + mtype->NumDirections - (-frame - 1) % row;
		} else {
			frame = (frame / row) * mtype->NumDirections + frame % row;
		}
		if (mtype->Transparency == 50) {
			VideoDrawClipTrans50(mtype->Sprite, frame, x, y);
		} else {
			VideoDrawClip(mtype->Sprite, frame, x, y);
		}
	}
}

/**
**  Compare Draw level. Used by qsort.
**
**  @param v1  adress of a missile pointer.
**  @param v2  adress of a missile pointer.
**  @return    -1 if v1 < v2, 1 else.
*/
local int MissileDrawLevelCompare(const void* v1, const void* v2)
{
	const Missile* c1;
	const Missile* c2;

	c1 = *(Missile**)v1;
	c2 = *(Missile**)v2;

	if (c1->Type->DrawLevel == c2->Type->DrawLevel) {
		return c1->MissileSlot < c2->MissileSlot ? -1 : 1;
	} else {
		return c1->Type->DrawLevel <= c2->Type->DrawLevel ? -1 : 1;
	}
}
/**
**  Draw all missiles on map.
**
**  @param vp     Viewport pointer.
**  @param table  FIXME: docu
**
**  @return       FIXME: docu
*/
global int FindAndSortMissiles(const Viewport* vp, Missile** table)
{
	Missile* missile;
	Missile* const* missiles;
	Missile* const* missiles_end;
	int flag;
	int nmissiles;

	//
	// Loop through global missiles, then through locals.
	//
	flag = 1;
	missiles = GlobalMissiles;
	missiles_end = missiles + NumGlobalMissiles;
	nmissiles = 0;
	do {
		for (; missiles < missiles_end; ++missiles) {
			missile = *missiles;
			if (missile->Delay || missile->Hidden) {
				continue;  // delayed or hidden -> aren't shown
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
		qsort((void*)table, nmissiles, sizeof(Missile*), MissileDrawLevelCompare);
	}

	return nmissiles;
}

/**
**  Change missile heading from x,y.
**
**  @param missile  Missile pointer.
**  @param dx       Delta in x.
**  @param dy       Delta in y.
**  @internal We have : SpriteFrame / (2 * (Numdirection - 1)) == DirectionToHeading / 256.
*/
local void MissileNewHeadingFromXY(Missile* missile, int dx, int dy)
{
	int dir;
	int nextdir;

	if (missile->Type->NumDirections == 1 || (dx == 0 && dy == 0)) {
		return;
	}
	// reinitialise the direction but with skipping Animation step.
	if (missile->SpriteFrame < 0) {
		missile->SpriteFrame = -missile->SpriteFrame - 1;
	}
	missile->SpriteFrame /= missile->Type->NumDirections / 2 + 1;
	missile->SpriteFrame *= missile->Type->NumDirections / 2 + 1;

	nextdir = 128 / (missile->Type->NumDirections - 1);
	DebugCheck(nextdir == 0);
	dir = ((DirectionToHeading(10 * dx, 10 * dy) + nextdir / 2) & 0xFF) / nextdir;
	if (dir >= missile->Type->NumDirections) {
		dir -= (missile->Type->NumDirections - 1) * 2;
	}
	DebugCheck(dir >= missile->Type->NumDirections);
	DebugCheck(dir < -missile->Type->NumDirections + 1);
	missile->SpriteFrame = dir;
	if (missile->SpriteFrame < 0) {
		missile->SpriteFrame = -missile->SpriteFrame - 1;
	}
}

/**
**  Init the move.
**
**  @param missile  missile to initialise for movement.
**  @return         1 if goal is reached, 0 else.
*/
local int MissileInitMove(Missile* missile)
{
	int dx;
	int dy;

	dx = missile->DX - missile->X;
	dy = missile->DY - missile->Y;
	MissileNewHeadingFromXY(missile, dx, dy);
	if (!(missile->State & 1)) {
		missile->CurrentStep = 0;
		missile->TotalStep = 0;
		if (dx == 0 && dy == 0) {
			return 1;
		}
		// initialize
		missile->TotalStep = MapDistance(missile->SourceX, missile->SourceY, missile->DX, missile->DY);
		missile->State++;
		return 0;
	}
	DebugCheck(missile->TotalStep == 0);
	missile->CurrentStep += missile->Type->Speed;
	if (missile->CurrentStep >= missile->TotalStep) {
		missile->X = missile->DX;
		missile->Y = missile->DY;
		return 1;
	}
	return 0;
}

/**
**  Handle point to point missile.
**
**  @param missile  Missile pointer.
**
**  @return         1 if goal is reached, 0 else.
*/
local int PointToPointMissile(Missile* missile)
{
	int xstep;
	int ystep;
	int x;
	int y;

	if (MissileInitMove(missile) == 1) {
		return 1;
	}

	DebugCheck(missile->Type == NULL);
	DebugCheck(missile->TotalStep == 0);
	xstep = (missile->DX - missile->SourceX) * 1024 / missile->TotalStep;
	ystep = (missile->DY - missile->SourceY) * 1024 / missile->TotalStep;
	missile->X = missile->SourceX + xstep * missile->CurrentStep / 1024;
	missile->Y = missile->SourceY + ystep * missile->CurrentStep / 1024;
	if (missile->Type->SmokeMissile && missile->CurrentStep) {
		x = missile->X + missile->Type->Width / 2;
		y = missile->Y + missile->Type->Height / 2;
		MakeMissile(missile->Type->SmokeMissile, x, y, x, y);
	}
	return 0;
}

/**
**  Calculate parabolic trajectories.
**
**  @param missile  Missile pointer.
**
**  @return         1 if target is reached, 0 otherwise
**
**  @todo Find good values for ZprojToX and Y
*/
local int ParabolicMissile(Missile* missile)
{
	int orig_x;   // position before moving.
	int orig_y;   // position before moving.
	int xstep;
	int ystep;
	int K;        // Coefficient of the parabol.
	int ZprojToX; // Projection of Z axis on axis X.
	int ZprojToY; // Projection of Z axis on axis Y.
	int Z;        // should be missile->Z later.
	int x;
	int y;

	K = -2048; //-1024; // Should be initialised by an other method (computed with distance...)
	ZprojToX = 4;
	ZprojToY = 1024;
	if (MissileInitMove(missile) == 1) {
		return 1;
	}
	DebugCheck(missile->Type == NULL);
	orig_x = missile->X;
	orig_y = missile->Y;
	xstep = missile->DX - missile->SourceX;
	ystep = missile->DY - missile->SourceY;
	DebugCheck(missile->TotalStep == 0);
	xstep = xstep * 1000 / missile->TotalStep;
	ystep = ystep * 1000 / missile->TotalStep;
	missile->X = missile->SourceX + xstep * missile->CurrentStep / 1000;
	missile->Y = missile->SourceY + ystep * missile->CurrentStep / 1000;
	DebugCheck(K == 0);
	Z = missile->CurrentStep * (missile->TotalStep - missile->CurrentStep) / K;
	// Until Z is used for drawing, modify X and Y.
	missile->X += Z * ZprojToX / 64;
	missile->Y += Z * ZprojToY / 64;
	MissileNewHeadingFromXY(missile, missile->X - orig_x, missile->Y - orig_y);
	if (missile->Type->SmokeMissile && missile->CurrentStep) {
		x = missile->X + missile->Type->Width / 2;
		y = missile->Y + missile->Type->Height / 2;
		MakeMissile(missile->Type->SmokeMissile, x, y, x, y);
	}
	return 0;
}

/**
**  Missile hits the goal.
**
**  @param missile  Missile hitting the goal.
**  @param goal     Goal of the missile.
**  @param splash   Splash damage divisor.
*/
local void MissileHitsGoal(const Missile* missile, Unit* goal, int splash)
{
	if (!missile->Type->CanHitOwner && goal == missile->SourceUnit) {
		return;  // blizzard cannot hit owner unit
	}

	if (goal->HP && goal->Orders[0].Action != UnitActionDie) {
		if (missile->Damage) {  // direct damage, spells mostly
			HitUnit(missile->SourceUnit, goal, missile->Damage / splash);
		} else {
			DebugCheck(missile->SourceUnit == NULL);
			HitUnit(missile->SourceUnit, goal,
				CalculateDamage(missile->SourceUnit->Stats, goal,
					missile->SourceUnit->Bloodlust, 0) / splash);
		}
	}
}

/**
**  Missile hits wall.
**
**  @param missile  Missile hitting the goal.
**  @param x        Wall X map tile position.
**  @param y        Wall Y map tile position.
**  @param splash   Splash damage divisor.
**
**  @todo FIXME: Support for more races.
*/
local void MissileHitsWall(const Missile* missile, int x, int y, int splash)
{
	if (WallOnMap(x, y)) {
		DebugLevel3Fn("Missile on wall?\n");
		if (HumanWallOnMap(x, y)) {
			if (missile->Damage) {  // direct damage, spells mostly
				HitWall(x, y, missile->Damage / splash);
			} else {
				HitWall(x, y,
					CalculateDamageStats(missile->SourceUnit->Stats,
						UnitTypeHumanWall->Stats, 0, 0) / splash);
			}
		} else {
			if (missile->Damage) {  // direct damage, spells mostly
				HitWall(x, y, missile->Damage / splash);
			} else {
				DebugCheck(missile->SourceUnit == NULL);
				HitWall(x, y,
					CalculateDamageStats(missile->SourceUnit->Stats,
						UnitTypeOrcWall->Stats, 0, 0) / splash);
			}
		}
		return;
	}
}

/**
**  Work for missile hit.
**
**  @param missile  Missile reaching end-point.
*/
global void MissileHit(Missile* missile)
{
	Unit* goal;
	int x;
	int y;
	Unit* table[UnitMax];
	int n;
	int i;
	int splash;

	if (missile->Type->ImpactSound.Sound) {
		PlayMissileSound(missile, missile->Type->ImpactSound.Sound);
	}

	x = missile->X + missile->Type->Width / 2;
	y = missile->Y + missile->Type->Height / 2;

	//
	// The impact generates a new missile.
	//
	if (missile->Type->ImpactMissile) {
//		Missile* mis;

//		mis =
		MakeMissile(missile->Type->ImpactMissile, x, y, x, y);
		// Impact missiles didn't generate any damage now.
#if 0
		mis->Damage = missile->Damage; // direct damage, spells mostly
		mis->SourceUnit = missile->SourceUnit;
		// FIXME: should copy target also?
		if (mis->SourceUnit) {
			RefsIncrease(mis->SourceUnit);
		}
#endif
	}

	if (!missile->SourceUnit) {  // no owner - green-cross ...
		DebugLevel3Fn("Missile has no owner!\n");
		return;
	}

	x /= TileSizeX;
	y /= TileSizeY;

	if (x < 0 || y < 0 || x >= TheMap.Width || y >= TheMap.Height) {
		// FIXME: this should handled by caller?
		DebugLevel0Fn("Missile gone outside of map!\n");
		return;  // outside the map.
	}

	//
	// Choose correct goal.
	//
	if (!missile->Type->Range) {
		if (missile->TargetUnit) {
			//
			// Missiles without range only hits the goal always.
			//
			goal = missile->TargetUnit;
			if (goal->Destroyed) {  // Destroyed
				RefsDecrease(goal);
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
	// Hits all units in range.
	//
	i = missile->Type->Range;
	n = SelectUnits(x - i + 1, y - i + 1, x + i, y + i, table);
	DebugCheck(missile->SourceUnit == NULL);
	for (i = 0; i < n; ++i) {
		goal = table[i];
		//
		// Can the unit attack the this unit-type?
		// NOTE: perhaps this should be come a property of the missile.
		//
		if (CanTarget(missile->SourceUnit->Type, goal->Type)) {
			splash = MapDistanceToUnit(x, y, goal);
			if (splash) {
				splash *= missile->Type->SplashFactor;
			} else {
				splash = 1;
			}
			MissileHitsGoal(missile, goal, splash);
		}
	}
	//
	// Missile hits ground.
	//
	// FIXME: no bock writing it correct.
	x -= missile->Type->Range;
	y -= missile->Type->Range;
	for (i = missile->Type->Range * 2; --i;) {
		for (n = missile->Type->Range * 2; --n;) {
			if (x + i >= 0 && x + i < TheMap.Width && y + n >= 0 && y + n < TheMap.Height) {
				if (i == 0 && n == 0) {
					MissileHitsWall(missile, x + i, y + n, 1);
				} else {
					MissileHitsWall(missile, x + i, y + n,
						MapDistance(x, y, i, n) * missile->Type->SplashFactor);
				}
			}
		}
	}
}

/**
**  Pass to the next frame for animation.
**
**  @param missile        missile to animate.
**  @param sign           1 for next frame, -1 for previous frame.
**  @param LongAnimation  1 if Frame is conditionned by covered distance, 0 else.
**
**  @return               1 if animation is finished, 0 else.
*/
local int NextMissileFrame(Missile* missile, char sign, char LongAnimation)
{
	int neg;                 // True for mirroring sprite.
	int AnimationIsFinished; // returned value.
	int NumDirections;       // Number of direction of the missile.

	//
	// Animate missile, cycle through frames
	//
	neg = 0;
	AnimationIsFinished = 0;
	NumDirections = missile->Type->NumDirections;
	if (missile->SpriteFrame < 0) {
		neg = 1;
		missile->SpriteFrame = -missile->SpriteFrame - 1;
	}
	if (LongAnimation) {
		int totalf;   // Total number of frame (for one direction).
		int df;       // Current frame (for one direction).
		int totalx;   // Total distance to cover.
		int dx;       // Covered distance.

		totalx = MapDistance(missile->DX, missile->DY, missile->SourceX, missile->SourceY);
		dx = MapDistance(missile->X, missile->Y, missile->SourceX, missile->SourceY);
		totalf = missile->Type->SpriteFrames / NumDirections;
		df = missile->SpriteFrame / NumDirections;
		if ((sign == 1 && dx * totalf <= df * totalx) ||
				(sign == -1 && dx * totalf > df * totalx)) {
			return AnimationIsFinished;
		}
	}
	missile->SpriteFrame += sign * NumDirections;
	if (sign > 0) {
		if (missile->SpriteFrame >= missile->Type->SpriteFrames) {
			missile->SpriteFrame -= missile->Type->SpriteFrames;
			AnimationIsFinished = 1;
		}
	} else {
		if (missile->SpriteFrame < 0) {
			missile->SpriteFrame += missile->Type->SpriteFrames;
			AnimationIsFinished = 1;
		}
	}
	if (neg) {
		missile->SpriteFrame = -missile->SpriteFrame - 1;
	}
	DebugLevel3Fn("Frame %d of %d\n" _C_
		missile->SpriteFrame _C_ missile->Type->SpriteFrames);
	return AnimationIsFinished;
}

/**
**  Pass the next frame of the animation.
**  This animation goes from start to finish ONCE on the way
**
**  @param missile  Missile pointer.
*/
local void NextMissileFrameCycle(Missile* missile)
{
	int neg;
	int totalx;
	int dx;
	int f;
	int i;
	int j;

	neg = 0;
	if (missile->SpriteFrame < 0) {
		neg = 1;
		missile->SpriteFrame = -missile->SpriteFrame - 1;
	}
	totalx = abs(missile->DX - missile->SourceX);
	dx = abs(missile->X - missile->SourceX);
	f = missile->Type->SpriteFrames / missile->Type->NumDirections;
	f = 2 * f - 1;
	for (i = 1, j = 1; i <= f; ++i) {
		if (dx * f / i < totalx) {
			if ((i - 1) * 2 < f) {
				j = i - 1;
			} else {
				j = f - i;
			}
			missile->SpriteFrame = missile->SpriteFrame % missile->Type->NumDirections +
				j * missile->Type->NumDirections;
			break;
		}
	}
	if (neg) {
		missile->SpriteFrame = -missile->SpriteFrame - 1;
	}
}

/**
**  Handle all missile actions of global/local missiles.
**
**  @param missiles  Table of missiles.
*/
local void MissilesActionLoop(Missile** missiles)
{
	Missile* missile;

	//
	// NOTE: missiles[??] could be modified!!! Yes (freed)
	//
	while ((missile = *missiles)) {
		DebugLevel3Fn("Missile %s ttl %d at %d, %d\n" _C_ missile->Type->Ident
				_C_ missile->TTL _C_ missile->X _C_ missile->Y);
		if (missile->Delay && missile->Delay--) {
			++missiles;
			continue;  // delay start of missile
		}

		if (missile->TTL > 0) {
			missile->TTL--;  // overall time to live if specified
		}

		if (!missile->TTL) {
			FreeMissile(missile);
			continue;
		}

		if (--missile->Wait) {  // wait until time is over
			++missiles;
			continue;
		}

		// Mark missile area on screen to be drawn, if missile moves or disappears.
		CheckMissileToBeDrawn(missile);

		MissileClassFunctions[missile->Type->Class](missile);

		if (!missile->TTL) {
			FreeMissile(missile);
			continue;
		}

		if (*missiles == missile) {  // Missile not destroyed
			++missiles;
		}
	}
}

/**
**  Handle all missile actions.
*/
global void MissileActions(void)
{
	MissilesActionLoop(GlobalMissiles);
	MissilesActionLoop(LocalMissiles);
}

/**
**  Calculate distance from view-point to missle.
**
**  @param missile  Missile pointer for distance.
*/
global int ViewPointDistanceToMissile(const Missile* missile)
{
	int x;
	int y;

	x = (missile->X + missile->Type->Width / 2) / TileSizeX;
	y = (missile->Y + missile->Type->Height / 2) / TileSizeY;  // pixel -> tile

	DebugLevel3Fn("Missile %p at %d %d\n" _C_ missile _C_ x _C_ y);

	return ViewPointDistance(x, y);
}

/**
**  Get the burning building missile based on hp percent.
**
**  @param percent  HP percent
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
	DebugCheck(frame == NULL);
	return frame->Missile;
}

/**
**  Save the missile-types to file.
**
**  @param file  Output file.
**
**  @todo FIXME: CanHitOwner and FriendlyFire not supported!
*/
global void SaveMissileTypes(CLFile* file)
{
#if 0
	MissileType* mtype;
	char** sp;
	int i;

	DebugCheck(file == NULL);
	CLprintf(file, "\n;;; -----------------------------------------\n");
	CLprintf(file, ";;; MODULE: missile-types $Id$\n\n");

	//
	// Original number to internal missile-type name.
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
	// Missile types
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
		if (mtype->Transparency) {
			CLprintf(file, "\n 'transparency %d", mtype->Transparency);
		}
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
		if (mtype->Class == MissileClassPointToPointBounce) {
			CLprintf(file," 'num-bounces %d",mtype->NumBounces);
		}
		CLprintf(file, " 'class '%s", MissileClassNames[mtype->Class]);
		CLprintf(file, " 'draw-level %d ", mtype->DrawLevel);
		if (mtype->StartDelay) {
			CLprintf(file, " 'delay %d", mtype->StartDelay);
		}
		CLprintf(file, " 'sleep %d", mtype->Sleep);
		CLprintf(file, " 'speed %d", mtype->Speed);
		CLprintf(file, " 'range %d", mtype->Range);
		CLprintf(file, " 'splash-factor %d", mtype->SplashFactor);
		if (mtype->ImpactMissile) {
			CLprintf(file, "\n  'impact-missile '%s", mtype->ImpactMissile->Ident);
		}
		if (mtype->SmokeMissile) {
			CLprintf(file, "\n  'smoke-missile '%s", mtype->SmokeMissile->Ident);
		}
		CLprintf(file, "\n ");
		if (mtype->CanHitOwner) {
			CLprintf(file, " 'can-hit-owner ");
		}
		if (mtype->FriendlyFire) {
			CLprintf(file, " 'friendly-fire");
		}
		CLprintf(file, ")\n");
	}
#endif
}

/**
**  Save the state of a missile to file.
**
**  @param missile  Missile object to save.
**  @param file     Output file.
*/
local void SaveMissile(const Missile* missile, CLFile* file)
{
	char* s1;

	CLprintf(file, "Missile(\"type\", \"%s\",", missile->Type->Ident);
	CLprintf(file, " \"%s\",", missile->Local ? "local" : "global");
	CLprintf(file, " \"pos\", {%d, %d}, \"origin-pos\", {%d, %d}, \"goal\", {%d, %d},",
		missile->X, missile->Y, missile->SourceX, missile->SourceY, missile->DX, missile->DY);
	CLprintf(file, "\n  \"frame\", %d, \"state\", %d, \"anim-wait\", %d, \"wait\", %d, \"delay\", %d,\n ",
		missile->SpriteFrame, missile->State, missile->AnimWait, missile->Wait, missile->Delay);

	if (missile->SourceUnit) {
		CLprintf(file, " \"source\", \"%s\",", s1 = UnitReference(missile->SourceUnit));
		free(s1);
	}
	if (missile->TargetUnit) {
		CLprintf(file, " \"target\", \"%s\",", s1 = UnitReference(missile->TargetUnit));
		free(s1);
	}

	CLprintf(file, " \"damage\", %d,", missile->Damage);

	CLprintf(file, " \"ttl\", %d,",		missile->TTL);
	if (missile->Hidden) {
		CLprintf(file, " \"hidden\", ");
	}

	CLprintf(file, " \"step\", {%d, %d},",
		missile->CurrentStep, missile->TotalStep);

	// MissileSlot filled in during init

	CLprintf(file, ")\n");
}

/**
**  Save the state missiles to file.
**
**  @param file  Output file.
*/
global void SaveMissiles(CLFile* file)
{
	Missile* const* missiles;

	CLprintf(file,"\n--- -----------------------------------------\n");
	CLprintf(file,"--- MODULE: missiles $Id$\n\n");

	for (missiles = GlobalMissiles; *missiles; ++missiles) {
		SaveMissile(*missiles, file);
	}
	for (missiles = LocalMissiles; *missiles; ++missiles) {
		SaveMissile(*missiles, file);
	}
}

/**
**  Initialize missile-types.
*/
global void InitMissileTypes(void)
{
	int i;
	MissileType* mtype;

	if (!MissileTypes) {
		return;
	}
	for (i = 0; i < NumMissileTypes; ++i) {
		mtype = MissileTypes[i];

		//
		// Add missile names to hash table
		//
		*(MissileType**)hash_add(MissileTypeHash, mtype->Ident) = mtype;

		//
		// Resolve impact missiles and sounds.
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
		if (mtype->SmokeName) {
			mtype->SmokeMissile = MissileTypeByIdent(mtype->SmokeName);
		}
	}
}

/**
**  Clean up missile-types.
*/
global void CleanMissileTypes(void)
{
	int i;
	MissileType* mtype;

	if (!MissileTypes) {
		return;
	}

	for (i = 0; i < NumMissileTypes; ++i) {
		mtype = MissileTypes[i];
		hash_del(MissileTypeHash, mtype->Ident);

		free(mtype->Ident);
		free(mtype->File);
		free(mtype->FiredSound.Name);
		free(mtype->ImpactSound.Name);
		free(mtype->ImpactName);
		free(mtype->SmokeName);
		VideoSafeFree(mtype->Sprite);
	}
	free(MissileTypes);
	MissileTypes = NULL;
	NumMissileTypes = 0;
}

/**
**  Initialize missiles.
*/
global void InitMissiles(void)
{
}

/**
**  Clean up missiles.
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
--    Functions (Spells Controllers/Callbacks) TODO: move to another file?
----------------------------------------------------------------------------*/

// ****************************************************************************
// Actions for the missiles
// ****************************************************************************

/**
**  Missile controllers
**
**  To cancel a missile set it's TTL to 0, it will be handled right after
**  the controller call and missile will be down.
*/

/**
**  Missile does nothing
**
**  @param missile  pointer to missile
*/
void MissileActionNone(Missile* missile)
{
	missile->Wait = missile->Type->Sleep;
	return;  //  Busy doing nothing.
}

/**
**  Missile flies from x,y to x1,y1 animation on the way
**
**  @param missile  pointer to missile
*/
void MissileActionPointToPoint(Missile* missile)
{
	missile->Wait = missile->Type->Sleep;
	if (PointToPointMissile(missile)) {
		MissileHit(missile);
		missile->TTL = 0;
	} else {
		NextMissileFrame(missile, 1, 0);
	}
}

/**
**  Missile flies from x,y to x1,y1 showing the first frame
**  and then shows a hit animation.
**
**  @param missile  pointer to missile
*/
void MissileActionPointToPointWithHit(Missile* missile)
{
	missile->Wait = missile->Type->Sleep;
	if (PointToPointMissile(missile)) {
		if (NextMissileFrame(missile, 1, 0)) {
			MissileHit(missile);
			missile->TTL = 0;
		}
	}
}

/**
**  Missile flies from x,y to x1,y1 and stays there for a moment
**
**  @param missile  pointer to missile
*/
void MissileActionPointToPointCycleOnce(Missile* missile)
{
	missile->Wait = missile->Type->Sleep;
	if (PointToPointMissile(missile)) {
		MissileHit(missile);
		missile->TTL = 0;
	} else {
		NextMissileFrameCycle(missile);
	}
}

/**
**  Missile don't move, than disappears
**
**  @param missile  pointer to missile
*/
void MissileActionStay(Missile* missile)
{
	missile->Wait = missile->Type->Sleep;
	if (NextMissileFrame(missile, 1, 0)) {
		MissileHit(missile);
		missile->TTL = 0;
	}
}

/**
**  Missile flies from x,y to x1,y1 than bounces NumBounces times
**
**  @param missile  pointer to missile
*/
void MissileActionPointToPointBounce(Missile* missile)
{
	missile->Wait = missile->Type->Sleep;
	if (PointToPointMissile(missile)) {
		if (missile->State < 2 * missile->Type->NumBounces - 1 && missile->TotalStep) {
			int xstep;
			int ystep;

			xstep = (missile->DX - missile->SourceX) * 1024 / missile->TotalStep;
			ystep = (missile->DY - missile->SourceY) * 1024 / missile->TotalStep;
			missile->DX += xstep * (TileSizeX + TileSizeY) * 3 / 4 / 1024;
			missile->DY += ystep * (TileSizeX + TileSizeY) * 3 / 4 / 1024;

			missile->State++; // !(State & 1) to initialise
			missile->SourceX = missile->X;
			missile->SourceY = missile->Y;
			PointToPointMissile(missile);
			//missile->State++;
			DebugLevel3("HIT %d!\n" _C_ missile->State);
			MissileHit(missile);
			// FIXME: hits to left and right
			// FIXME: reduce damage effects on later impacts
		} else {
			MissileHit(missile);
			missile->TTL = 0;
		}
	} else {
		NextMissileFrame(missile, 1, 0);
	}
}

/**
**  Missile doesn't move, it will just cycle once and vanish.
**  Used for ui missiles (cross shown when you give and order)
**
**  @param missile  pointer to missile
*/
void MissileActionCycleOnce(Missile* missile)
{
	missile->Wait = missile->Type->Sleep;
	switch (missile->State) {
		case 0:
		case 2:
			++missile->State;
			break;
		case 1:
			if (NextMissileFrame(missile, 1, 0)) {
				++missile->State;
			}
			break;
		case 3:
			if (NextMissileFrame(missile, -1, 0)) {
				MissileHit(missile);
				missile->TTL = 0;
			}
			break;
	}
}

/**
**  Missile don't move, than checks the source unit for HP.
**
**  @param missile  pointer to missile
*/
void MissileActionFire(Missile* missile)
{
	Unit* unit;

	unit = missile->SourceUnit;
	missile->Wait = missile->Type->Sleep;
	if (unit->Destroyed || !unit->HP) {
		missile->TTL = 0;
		return;
	}
	if (NextMissileFrame(missile, 1, 0)) {
		int f;
		MissileType* fire;

		missile->SpriteFrame = 0;
		f = (100 * unit->HP) / unit->Stats->HitPoints;
		fire = MissileBurningBuilding(f);
		if (!fire) {
			missile->TTL = 0;
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
**  Missile shows hit points?
**
**  @param missile  pointer to missile
*/
void MissileActionHit(Missile* missile)
{
	missile->Wait = missile->Type->Sleep;
	if (PointToPointMissile(missile)) {
		MissileHit(missile);
		missile->TTL = 0;
	}
}

/**
**  Missile flies from x,y to x1,y1 using a parabolic path
**
**  @param missile  pointer to missile
*/
void MissileActionParabolic(Missile* missile)
{
	missile->Wait = missile->Type->Sleep;
	if (ParabolicMissile(missile)) {
		MissileHit(missile);
		missile->TTL = 0;
	} else {
		NextMissileFrameCycle(missile);
	}
}

/**
**  FlameShield controller
**
**  @param missile  Controlled missile
*/
global void MissileActionFlameShield(Missile* missile)
{
	static int fs_dc[] = {
		0, 32, 5, 31, 10, 30, 16, 27, 20, 24, 24, 20, 27, 15, 30, 10, 31,
		5, 32, 0, 31, -5, 30, -10, 27, -16, 24, -20, 20, -24, 15, -27, 10,
		-30, 5, -31, 0, -32, -5, -31, -10, -30, -16, -27, -20, -24, -24, -20,
		-27, -15, -30, -10, -31, -5, -32, 0, -31, 5, -30, 10, -27, 16, -24,
		20, -20, 24, -15, 27, -10, 30, -5, 31, 0, 32};
	Unit* table[UnitMax];
	Unit* unit;
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

	missile->Wait = missile->Type->Sleep;
	i = missile->TTL % 36;  // 36 positions on the circle
	dx = fs_dc[i * 2];
	dy = fs_dc[i * 2 + 1];
	unit = missile->TargetUnit;
	//
	// Show around the top most unit.
	// FIXME: conf, do we hide if the unit is contained or not?
	//
	while (unit->Container) {
		unit=unit->Container;
	}
	ux = unit->X;
	uy = unit->Y;
	ix = unit->IX;
	iy = unit->IY;
	uw = unit->Type->TileWidth;
	uh = unit->Type->TileHeight;
	missile->X = ux * TileSizeX + ix + uw * TileSizeX / 2 + dx - 16;
	missile->Y = uy * TileSizeY + iy + uh * TileSizeY / 2 + dy - 32;
	if (unit->Orders[0].Action == UnitActionDie) {
		missile->TTL = i;
	}
	if (missile->TTL == 0) {
		unit->FlameShield = 0;
	}

	if (unit->Container) {
		missile->Hidden = 1;
		return;  // Hidden missile don't do damage.
	} else {
		missile->Hidden = 0;
	}

	// Only hit 1 out of 8 frames
	if (missile->TTL & 7) {
		return;
	}
	n = SelectUnits(ux - 1, uy - 1, ux + 1 + 1, uy + 1 + 1, table);
	for (i = 0; i < n; ++i) {
		if (table[i] == unit) {
			// cannot hit target unit
			continue;
		}
		if (table[i]->HP) {
			HitUnit(missile->SourceUnit, table[i], missile->Damage);
		}
	}
}

/**
**  Land mine controller.
**  @todo start-finish-start cyclic animation.(anim scripts!)
**  @todo missile should dissapear for a while.
**
**  @param missile  Controlled missile
*/
global void MissileActionLandMine(Missile* missile)
{
	Unit* table[UnitMax];
	int i;
	int n;
	int x;
	int y;

	x = missile->X / TileSizeX;
	y = missile->Y / TileSizeY;

	n = SelectUnitsOnTile(x, y, table);
	for (i = 0; i < n; ++i) {
		if (table[i]->Type->UnitType != UnitTypeFly &&
				table[i]->HP &&
				!(table[i] == missile->SourceUnit && !missile->Type->CanHitOwner)) {
			DebugLevel0("Landmine explosion at %d,%d.\n" _C_ x _C_ y);
			MissileHit(missile);
			missile->TTL = 0;
			return;
		}
	}

	if (!missile->AnimWait--) {
		NextMissileFrame(missile, 1, 0);
		missile->AnimWait = missile->Type->Sleep;
	}
	missile->Wait = 1;
}

/**
**  Whirlwind controller
**
**  @param missile  Controlled missile
*/
global void MissileActionWhirlwind(Missile* missile)
{
	int x;
	int y;

	//
	// Animate, move.
	//
	if (!missile->AnimWait--) {
		if (NextMissileFrame(missile, 1, 0)) {
			missile->SpriteFrame = 0;
			PointToPointMissile(missile);
		}
		missile->AnimWait = missile->Type->Sleep;
	}
	missile->Wait = 1;
	//
	// Center of the tornado
	//
	x = (missile->X + TileSizeX / 2 + missile->Type->Width / 2) / TileSizeX;
	y = (missile->Y + TileSizeY + missile->Type->Height / 2) / TileSizeY;

#if 0
	Unit* table[UnitMax];
	int i;
	int n;

	//
	// Every 4 cycles 4 points damage in tornado center
	//
	if (!(missile->TTL % 4)) {
		n = SelectUnitsOnTile(x, y, table);
		for (i = 0; i < n; ++i)		{
			if (table[i]->HP) {
				// should be missile damage ?
				HitUnit(missile->SourceUnit,table[i], WHIRLWIND_DAMAGE1);
			}
		}
	}
	//
	// Every 1/10s 1 points damage on tornado periphery
	//
	if (!(missile->TTL % (CYCLES_PER_SECOND/10))) {
		// we should parameter this
		n = SelectUnits(x - 1, y - 1, x + 1, y + 1, table);
		DebugLevel3Fn("Damage on %d,%d-%d,%d = %d\n" _C_ x-1 _C_ y-1 _C_ x+1 _C_ y+1 _C_ n);
		for (i = 0; i < n; ++i) {
			if( (table[i]->X != x || table[i]->Y != y) && table[i]->HP) {
				// should be in missile
				HitUnit(missile->SourceUnit,table[i], WHIRLWIND_DAMAGE2);
			}
		}
	}
	DebugLevel0Fn( "Whirlwind: %d, %d, TTL: %d state: %d\n" _C_
			missile->X _C_ missile->Y _C_ missile->TTL _C_ missile->State);
#else
	if (!(missile->TTL % CYCLES_PER_SECOND / 10)) {
		MissileHit(missile);
	}

#endif
	//
	// Changes direction every 3 seconds (approx.)
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
		missile->SourceX = missile->X;
		missile->SourceY = missile->Y;
		missile->State = 0;
		DebugLevel0Fn( "Whirlwind new direction: %d, %d, TTL: %d\n" _C_
			missile->DX _C_ missile->DY _C_ missile->TTL );
	}
}

/**
**  Death-Coil class. Damages organic units and gives to the caster.
**
**  @param missile  Controlled missile
*/
global void MissileActionDeathCoil(Missile* missile)
{
	Unit* table[UnitMax];
	int i;
	int n;
	Unit* source;

	missile->Wait = missile->Type->Sleep;
	if (PointToPointMissile(missile)) {
		source = missile->SourceUnit;
		DebugCheck(source == NULL);
		if (source->Destroyed) {
			return;
		}
		// source unit still exists
		//
		// Target unit still exists and casted on a special target
		//
		if (missile->TargetUnit && !missile->TargetUnit->Destroyed
				&& missile->TargetUnit->HP)  {
			HitUnit(source,missile->TargetUnit,missile->Damage);
			if (source->Orders[0].Action != UnitActionDie) {
				source->HP += missile->Damage;
				if (source->HP > source->Stats->HitPoints) {
					source->HP = source->Stats->HitPoints;
				}
			}
		} else {
			//
			// No target unit -- try enemies in range 5x5 // Must be parametrable
			//
			int ec;  // enemy count
			int x;
			int y;

			ec = 0;
			x = missile->DX / TileSizeX;
			y = missile->DY / TileSizeY;

			n = SelectUnits(x - 2, y - 2, x + 2, y + 2, table);
			if (n == 0) {
				return;
			}
			// calculate organic enemy count
			for (i = 0; i < n; ++i) {
				ec += (IsEnemy(source->Player, table[i])
				/*&& table[i]->Type->Organic != 0*/);
			}
			if (ec > 0)  {
				// yes organic enemies found
				for (i = 0; i < n; ++i) {
					if (IsEnemy(source->Player, table[i])/* && table[i]->Type->Organic != 0*/) {
						// disperse damage between them
						// NOTE: 1 is the minimal damage
						HitUnit(source,table[i], missile->Damage / ec);
					}
				}
				if (source->Orders[0].Action != UnitActionDie) {
					source->HP += missile->Damage;
					if (source->HP > source->Stats->HitPoints) {
						source->HP = source->Stats->HitPoints;
					}
				}
			}
		}
		missile->TTL = 0;
	}
}

//@}
