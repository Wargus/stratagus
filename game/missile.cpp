//     ____                _       __               
//    / __ )____  _____   | |     / /___ ___________
//   / __  / __ \/ ___/   | | /| / / __ `/ ___/ ___/
//  / /_/ / /_/ (__  )    | |/ |/ / /_/ / /  (__  ) 
// /_____/\____/____/     |__/|__/\__,_/_/  /____/  
//                                              
//       A futuristic real-time strategy game.
//          This file is part of Bos Wars.
//
/**@name missile.cpp - The missiles. */
//
//      (c) Copyright 1998-2008 by Lutz Sammer and Jimmy Salmon
//
//      This program is free software; you can redistribute it and/or modify
//      it under the terms of the GNU General Public License as published by
//      the Free Software Foundation; only version 2 of the License.
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

//@{

/*----------------------------------------------------------------------------
--  Includes
----------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>


#include "stratagus.h"

#include <vector>
#include <string>
#include <map>

#include "video.h"
#include "font.h"
#include "map.h"
#include "unitsound.h"
#include "unittype.h"
#include "unit_cache.h"
#include "player.h"
#include "unit.h"
#include "missile.h"
#include "sound.h"
#include "ui.h"
#include "iolib.h"
#include "particle.h"
#include "trigger.h"
#include "luacallback.h"

/*----------------------------------------------------------------------------
--  Declarations
----------------------------------------------------------------------------*/

unsigned int Missile::Count = 0;

static std::vector<MissileType *>MissileTypes;  /// Missile types.

static std::vector<Missile*> GlobalMissiles;     /// all global missiles on map
static std::vector<Missile*> LocalMissiles;      /// all local missiles on map

	/// lookup table for missile names
static std::map<std::string, MissileType *> MissileTypeMap;

std::vector<BurningBuildingFrame *> BurningBuildingFrames; /// Burning building frames

/*----------------------------------------------------------------------------
--  Functions
----------------------------------------------------------------------------*/

/**
**  Load the graphics for a missile type
*/
void MissileType::LoadMissileSprite()
{
	if (this->G && !this->G->IsLoaded()) {
		this->G->Load();
		if (this->Flip) {
			this->G->Flip();
		}

		// Correct the number of frames in graphic
		Assert(this->G->NumFrames >= this->SpriteFrames);
		this->G->NumFrames = this->SpriteFrames;
		// FIXME: Don't use NumFrames as number of frames.
	}
}

/**
**  Load the graphics for all missiles types
*/
void LoadMissileSprites(void)
{
	for (std::vector<MissileType*>::iterator i = MissileTypes.begin(); i != MissileTypes.end(); ++i) {
		(*i)->LoadMissileSprite();
	}
}
/**
**  Get Missile type by identifier.
**
**  @param ident  Identifier.
**
**  @return       Missile type pointer.
*/
MissileType *MissileTypeByIdent(const std::string &ident)
{
	return MissileTypeMap[ident];
}

/**
**  Allocate an empty missile-type slot.
**
**  @param ident  Identifier to identify the slot.
**
**  @return       New allocated (zeroed) missile-type pointer.
*/
MissileType *NewMissileTypeSlot(const std::string &ident)
{
	MissileType *mtype;

	mtype = new MissileType(ident);
	MissileTypeMap[ident] = mtype;
	MissileTypes.push_back(mtype);
	return mtype;
}

/**
**  Constructor
*/
Missile::Missile() :
	SourceX(0), SourceY(0), X(0), Y(0),	DX(0), DY(0),
	Type(NULL), SpriteFrame(0), State(0), AnimWait(0), Wait(0),
	Delay(0), SourceUnit(NULL), TargetUnit(NULL), Damage(0),
	TTL(-1), Hidden(0),
	CurrentStep(0), TotalStep(0),
	Local(0)
{
	this->Slot = Missile::Count++;
}

/**
**  Initialize a new made missile.
**
**  @param mtype  Type pointer of missile.
**  @param sx     Missile x start point in pixel.
**  @param sy     Missile y start point in pixel.
**  @param dx     Missile x destination point in pixel.
**  @param dy     Missile y destination point in pixel.
**
**  @return       created missile.
*/
Missile *Missile::Init(MissileType *mtype, int sx, int sy, int dx, int dy)
{
	Missile *missile = NULL;

	switch (mtype->Class) {
		case MissileClassNone :
			missile = new MissileNone;
			break;
		case MissileClassPointToPoint :
			missile = new MissilePointToPoint;
			break;
		case MissileClassPointToPointWithHit :
			missile = new MissilePointToPointWithHit;
			break;
		case MissileClassPointToPointCycleOnce :
			missile = new MissilePointToPointCycleOnce;
			break;
		case MissileClassPointToPointBounce :
			missile = new MissilePointToPointBounce;
			break;
		case MissileClassStay :
			missile = new MissileStay;
			break;
		case MissileClassCycleOnce :
			missile = new MissileCycleOnce;
			break;
		case MissileClassFire :
			missile = new MissileFire;
			break;
		case MissileClassHit :
			missile = new MissileHit;
			break;
		case MissileClassParabolic :
			missile = new MissileParabolic;
			break;
	}

	missile->X = sx - mtype->Width / 2;
	missile->Y = sy - mtype->Height / 2;
	missile->DX = dx - mtype->Width / 2;
	missile->DY = dy - mtype->Height / 2;
	missile->SourceX = missile->X;
	missile->SourceY = missile->Y;
	missile->Type = mtype;
	missile->Wait = mtype->Sleep;
	missile->Delay = mtype->StartDelay;

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
Missile *MakeMissile(MissileType *mtype, int sx, int sy, int dx, int dy)
{
	Missile *missile;

	missile = Missile::Init(mtype, sx, sy, dx, dy);
	GlobalMissiles.push_back(missile);
	return missile;
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
Missile *MakeLocalMissile(MissileType *mtype, int sx, int sy, int dx, int dy)
{
	Missile *missile;

	missile = Missile::Init(mtype, sx, sy, dx, dy);
	missile->Local = 1;
	LocalMissiles.push_back(missile);
	return missile;
}

/**
**  Free a missile.
**
**  @param missiles  Missile pointer.
**  @param i         Index in missiles of missile to free
*/
static void FreeMissile(std::vector<Missile *> &missiles, size_t i)
{
	Missile *missile;
	CUnit *unit;

	missile = missiles[i];
	//
	// Release all unit references.
	//
	if ((unit = missile->SourceUnit)) {
		unit->RefsDecrease();
	}
	if ((unit = missile->TargetUnit)) {
		unit->RefsDecrease();
	}
	for (std::vector<Missile*>::iterator j = missiles.begin(); j != missiles.end(); ++j) {
		if (*j == missile) {
			missiles.erase(j);
			break;
		}
	}
	delete missile;
}

/**
**  Calculate damage.
**
**  @todo NOTE: different targets (big are hit by some missiles better)
**  @todo NOTE: lower damage for hidden targets.
**  @todo NOTE: lower damage for targets on higher ground.
**
**  @param attacker  Attacker.
**  @param goal      Goal unit.
**
**  @return          damage inflicted on goal.
*/
static int CalculateDamage(const CUnit *attacker, const CUnit *goal)
{
	int damage;
	int basic_damage;
	int piercing_damage;

	basic_damage = attacker->Stats->Variables[BASICDAMAGE_INDEX].Value;
	piercing_damage = attacker->Stats->Variables[PIERCINGDAMAGE_INDEX].Value;

	damage = std::max(basic_damage - goal->Stats->Variables[ARMOR_INDEX].Value, 1);
	damage += piercing_damage;
	damage -= SyncRand() % ((damage + 2) / 2);
	Assert(damage >= 0);

	return damage;
}

/**
**  Fire missile.
**
**  @param unit  Unit that fires the missile.
*/
void FireMissile(CUnit *unit)
{
	int x;
	int y;
	int dx;
	int dy;
	CUnit *goal;
	Missile *missile;

	//
	// Goal dead?
	//
	goal = unit->Orders[0]->Goal;
	if (goal) {

		// Better let the caller/action handle this.

		if (goal->Destroyed) {
			DebugPrint("destroyed unit\n");
			return;
		}
		if (goal->Removed || goal->Orders[0]->Action == UnitActionDie) {
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
			DebugPrint("Missile-none hits no unit, shouldn't happen!\n");
			return;
		}

		HitUnit(unit, goal, CalculateDamage(unit, goal));

		return;
	}

	// If Firing from inside a Bunker
	if (unit->Container) {
		x = unit->Container->X * TileSizeX + TileSizeX / 2;  // missile starts in tile middle
		y = unit->Container->Y * TileSizeY + TileSizeY / 2;
	} else {
		x = unit->X * TileSizeX + TileSizeX / 2;  // missile starts in tile middle
		y = unit->Y * TileSizeY + TileSizeY / 2;
	}

	if (goal) {
		Assert(goal->Type);  // Target invalid?
		//
		// Moved out of attack range?
		//
		if (MapDistanceBetweenUnits(unit, goal) < unit->Type->MinAttackRange) {
			DebugPrint("Missile target too near %d,%d\n" _C_
				MapDistanceBetweenUnits(unit, goal) _C_ unit->Type->MinAttackRange);
			// FIXME: do something other?
			return;
		}
		// Fire to nearest point of the unit!
		// If Firing from inside a Bunker
		if (unit->Container) {
			NearestOfUnit(goal, unit->Container->X, unit->Container->Y, &dx, &dy);
		} else {
			NearestOfUnit(goal, unit->X, unit->Y, &dx, &dy);
		}
	} else {
		dx = unit->Orders[0]->X;
		dy = unit->Orders[0]->Y;
		// FIXME: Can this be too near??
	}

	// Fire to the tile center of the destination.
	dx = dx * TileSizeX + TileSizeX / 2;
	dy = dy * TileSizeY + TileSizeY / 2;
	missile = MakeMissile(unit->Type->Missile.Missile, x, y, dx, dy);
	//
	// Damage of missile
	//
	if (goal) {
		missile->TargetUnit = goal;
		goal->RefsIncrease();
	}
	missile->SourceUnit = unit;
	unit->RefsIncrease();
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
static void GetMissileMapArea(const Missile *missile, int *sx, int *sy,
	int *ex, int *ey)
{
#define BoundX(x) std::min(std::max(0, x), Map.Info.MapWidth - 1)
#define BoundY(y) std::min(std::max(0, y), Map.Info.MapHeight - 1)

	*sx = BoundX(missile->X / TileSizeX);
	*sy = BoundY(missile->Y / TileSizeY);
	*ex = BoundX((missile->X + missile->Type->Width + TileSizeX - 1) / TileSizeX);
	*ey = BoundY((missile->Y + missile->Type->Height + TileSizeY - 1) / TileSizeY);

#undef BoundX
#undef BoundY
}

/**
**  Check missile visibility in a given viewport.
**
**  @param vp       Viewport to be checked.
**  @param missile  Missile pointer to check if visible.
**
**  @return         Returns true if visible, false otherwise.
*/
static int MissileVisibleInViewport(const CViewport *vp, const Missile *missile)
{
	int min_x;
	int max_x;
	int min_y;
	int max_y;

	GetMissileMapArea(missile, &min_x, &min_y, &max_x, &max_y);
	if (!vp->AnyMapAreaVisibleInViewport(min_x, min_y, max_x, max_y)) {
		return 0;
	}

	for (int x = min_x; x <= max_x; ++x) {
		for (int y = min_y; y <= max_y; ++y) {
			if (ReplayRevealMap || Map.IsFieldVisible(ThisPlayer, x, y)) {
				return 1;
			}
		}
	}
	return 0;
}

/**
**  Draw missile.
**
**  @param frame  Animation frame
**  @param x      Screen pixel X position
**  @param y      Screen pixel Y position
*/
void MissileType::DrawMissileType(int frame, int x, int y) const
{
	if (this->Flip) {
		if (frame < 0) {
			if (this->Transparency == 50) {
				this->G->DrawFrameClipTransX(-frame - 1, x, y, 128);
			} else {
				this->G->DrawFrameClipX(-frame - 1, x, y);
			}
		} else {
			if (this->Transparency == 50) {
				this->G->DrawFrameClipTrans(frame, x, y, 128);
			} else {
				this->G->DrawFrameClip(frame, x, y);
			}
		}
	} else {
		int row;

		row = this->NumDirections / 2 + 1;
		if (frame < 0) {
			frame = ((-frame - 1) / row) * this->NumDirections + this->NumDirections - (-frame - 1) % row;
		} else {
			frame = (frame / row) * this->NumDirections + frame % row;
		}
		if (this->Transparency == 50) {
			this->G->DrawFrameClipTrans(frame, x, y, 128);
		} else {
			this->G->DrawFrameClip(frame, x, y);
		}
	}
}

/**
**  Draw missile.
*/
void Missile::DrawMissile() const
{
	int x;
	int y;
	const CViewport *vp;

	Assert(this->Type);
	Assert(CurrentViewport);

	vp = CurrentViewport;
	x = this->X - vp->MapX * TileSizeX + vp->X - vp->OffsetX;
	y = this->Y - vp->MapY * TileSizeY + vp->Y - vp->OffsetY;
	switch (this->Type->Class) {
		case MissileClassHit:
			VideoDrawNumberClip(x, y, GameFont, this->Damage);
			break;
		default:
			this->Type->DrawMissileType(this->SpriteFrame, x, y);
			break;
	}
}

/**
**  Compare Draw level. Used by qsort.
**
**  @param v1  adress of a missile pointer.
**  @param v2  adress of a missile pointer.
**
**  @return    -1 if v1 < v2, 1 else.
*/
static int MissileDrawLevelCompare(const void *v1, const void *v2)
{
	const Missile *c1;
	const Missile *c2;

	c1 = *(Missile **)v1;
	c2 = *(Missile **)v2;

	if (c1->Type->DrawLevel == c2->Type->DrawLevel) {
		return c1->Slot < c2->Slot ? -1 : 1;
	} else {
		return c1->Type->DrawLevel <= c2->Type->DrawLevel ? -1 : 1;
	}
}
/**
**  Sort visible missiles on map for display.
**
**  @param vp         Viewport pointer.
**  @param table      OUT : array of missile to display sorted by DrawLevel.
**  @param tablesize  Size of table array
**
**  @return           number of missiles returned in table
*/
int FindAndSortMissiles(const CViewport *vp, Missile **table, int tablesize)
{
	int nmissiles;
	std::vector<Missile *>::const_iterator i;

	//
	// Loop through global missiles, then through locals.
	//
	nmissiles = 0;
	for (i = GlobalMissiles.begin(); i != GlobalMissiles.end() && nmissiles < tablesize; ++i) {
		if ((*i)->Delay || (*i)->Hidden) {
			continue;  // delayed or hidden -> aren't shown
		}
		// Draw only visible missiles
		if (MissileVisibleInViewport(vp, *i)) {
			table[nmissiles++] = *i;
		}
	}

	for (i = LocalMissiles.begin(); i != LocalMissiles.end() && nmissiles < tablesize; ++i) {
		if ((*i)->Delay || (*i)->Hidden) {
			continue;  // delayed or hidden -> aren't shown
		}
		// Local missile are visible.
		table[nmissiles++] = *i;
	}
	if (nmissiles) {
		qsort((void *)table, nmissiles, sizeof(Missile *), MissileDrawLevelCompare);
	}
	return nmissiles;
}

/**
**  Change missile heading from x,y.
**  Do not change the animation step.
**
**  @param missile  Missile pointer.
**  @param dx       Delta in x.
**  @param dy       Delta in y.
**  @internal We have : SpriteFrame / (2 * (Numdirection - 1)) == DirectionToHeading / 256.
*/
static void MissileNewHeadingFromXY(Missile *missile, int dx, int dy)
{
	int dir;
	int nextdir;

	// If the heading cannot be changed, return right away.
	if (missile->Type->NumDirections == 1 || (dx == 0 && dy == 0)) {
		return;
	}

	// Compute the number of the first frame of the current
	// animation step, discarding the previous heading.
	if (missile->SpriteFrame < 0) {
		missile->SpriteFrame = -missile->SpriteFrame - 1;
	}
	missile->SpriteFrame /= missile->Type->NumDirections / 2 + 1;
	missile->SpriteFrame *= missile->Type->NumDirections / 2 + 1;

	// How many heading units there are per frame.
	// This assumes NumDirections is a power of 2; otherwise,
	// nextdir is truncated and dir can get out of range.
	nextdir = 256 / missile->Type->NumDirections;
	Assert(nextdir != 0);

	// Compute the frame number relative to the current animation
	// step, as if flipping were not used.  Add nextdir/2 so the
	// quotient is rounded, not truncated.
	dir = ((DirectionToHeading(10 * dx, 10 * dy) + nextdir / 2) & 0xFF) / nextdir;

	// Compute the absolute frame number, with both the animation
	// step and the heading.
	if (dir <= LookingS / nextdir) { // north->east->south
		missile->SpriteFrame += dir;
	} else {
		missile->SpriteFrame += 256 / nextdir - dir;
		missile->SpriteFrame = -missile->SpriteFrame - 1;
	}
}

/**
**  Init the move.
**
**  @param missile  missile to initialise for movement.
**
**  @return         1 if goal is reached, 0 else.
*/
static int MissileInitMove(Missile *missile)
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
	Assert(missile->TotalStep != 0);
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
static int PointToPointMissile(Missile *missile)
{
	int xstep;
	int ystep;
	int x;
	int y;

	if (MissileInitMove(missile) == 1) {
		return 1;
	}

	Assert(missile->Type != NULL);
	Assert(missile->TotalStep != 0);
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
static int ParabolicMissile(Missile *missile)
{
	int orig_x;   // position before moving.
	int orig_y;   // position before moving.
	int xstep;
	int ystep;
	int k;        // Coefficient of the parabol.
	int zprojToX; // Projection of Z axis on axis X.
	int zprojToY; // Projection of Z axis on axis Y.
	int z;        // should be missile->Z later.
	int x;
	int y;

	k = -2048; //-1024; // Should be initialised by an other method (computed with distance...)
	zprojToX = 4;
	zprojToY = 1024;
	if (MissileInitMove(missile) == 1) {
		return 1;
	}
	Assert(missile->Type != NULL);
	orig_x = missile->X;
	orig_y = missile->Y;
	xstep = missile->DX - missile->SourceX;
	ystep = missile->DY - missile->SourceY;
	Assert(missile->TotalStep != 0);
	xstep = xstep * 1000 / missile->TotalStep;
	ystep = ystep * 1000 / missile->TotalStep;
	missile->X = missile->SourceX + xstep * missile->CurrentStep / 1000;
	missile->Y = missile->SourceY + ystep * missile->CurrentStep / 1000;
	Assert(k != 0);
	z = missile->CurrentStep * (missile->TotalStep - missile->CurrentStep) / k;
	// Until Z is used for drawing, modify X and Y.
	missile->X += z * zprojToX / 64;
	missile->Y += z * zprojToY / 64;
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
static void MissileHitsGoal(const Missile *missile, CUnit *goal, int splash)
{
	if (!missile->Type->CanHitOwner && goal == missile->SourceUnit) {
		return;
	}

	if (goal->Orders[0]->Action != UnitActionDie) {
		if (missile->Damage) {  // direct damage, spells mostly
			HitUnit(missile->SourceUnit, goal, missile->Damage / splash);
		} else {
			Assert(missile->SourceUnit != NULL);
			HitUnit(missile->SourceUnit, goal,
				CalculateDamage(missile->SourceUnit, goal) / splash);
		}
	}
}

/**
**  Work for missile hit.
**
**  @param missile  Missile reaching end-point.
*/
void MissileHit(Missile *missile)
{
	CUnit *goal;
	int x;
	int y;
	CUnit *table[UnitMax];
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
		MakeMissile(missile->Type->ImpactMissile, x, y, x, y);
	}
	if (missile->Type->ImpactParticle) {
		missile->Type->ImpactParticle->pushPreamble();
		missile->Type->ImpactParticle->pushInteger(x);
		missile->Type->ImpactParticle->pushInteger(y);
		missile->Type->ImpactParticle->run();
	}

	if (!missile->SourceUnit) {  // no owner - green-cross ...
		return;
	}

	x /= TileSizeX;
	y /= TileSizeY;

	if (x < 0 || y < 0 || x >= Map.Info.MapWidth || y >= Map.Info.MapHeight) {
		// FIXME: this should handled by caller?
		DebugPrint("Missile gone outside of map!\n");
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
				goal->RefsDecrease();
				missile->TargetUnit = NoUnitP;
				return;
			}
			MissileHitsGoal(missile, goal, 1);
			return;
		}
		return;
	}

	//
	// Hits all units in range.
	//
	i = missile->Type->Range;
	n = UnitCache.Select(x - i + 1, y - i + 1, x + i, y + i, table, UnitMax);
	Assert(missile->SourceUnit != NULL);
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
}

/**
**  Pass to the next frame for animation.
**
**  @param missile        missile to animate.
**  @param sign           1 for next frame, -1 for previous frame.
**  @param longAnimation  1 if Frame is conditionned by covered distance, 0 else.
**
**  @return               1 if animation is finished, 0 else.
*/
static int NextMissileFrame(Missile *missile, char sign, char longAnimation)
{
	int neg;                 // True for mirroring sprite.
	int animationIsFinished; // returned value.
	int numDirections;       // Number of direction of the missile.

	//
	// Animate missile, cycle through frames
	//
	neg = 0;
	animationIsFinished = 0;
	numDirections = missile->Type->NumDirections / 2 + 1;
	if (missile->SpriteFrame < 0) {
		neg = 1;
		missile->SpriteFrame = -missile->SpriteFrame - 1;
	}
	if (longAnimation) {
		int totalf;   // Total number of frame (for one direction).
		int df;       // Current frame (for one direction).
		int totalx;   // Total distance to cover.
		int dx;       // Covered distance.

		totalx = MapDistance(missile->DX, missile->DY, missile->SourceX, missile->SourceY);
		dx = MapDistance(missile->X, missile->Y, missile->SourceX, missile->SourceY);
		totalf = missile->Type->SpriteFrames / numDirections;
		df = missile->SpriteFrame / numDirections;
		if ((sign == 1 && dx * totalf <= df * totalx) ||
				(sign == -1 && dx * totalf > df * totalx)) {
			return animationIsFinished;
		}
	}
	missile->SpriteFrame += sign * numDirections;
	if (sign > 0) {
		if (missile->SpriteFrame >= missile->Type->SpriteFrames) {
			missile->SpriteFrame -= missile->Type->SpriteFrames;
			animationIsFinished = 1;
		}
	} else {
		if (missile->SpriteFrame < 0) {
			missile->SpriteFrame += missile->Type->SpriteFrames;
			animationIsFinished = 1;
		}
	}
	if (neg) {
		missile->SpriteFrame = -missile->SpriteFrame - 1;
	}

	return animationIsFinished;
}

/**
**  Pass the next frame of the animation.
**  This animation goes from start to finish ONCE on the way
**
**  @param missile  Missile pointer.
*/
static void NextMissileFrameCycle(Missile *missile)
{
	int neg;
	int totalx;
	int dx;
	int f;

	neg = 0;
	if (missile->SpriteFrame < 0) {
		neg = 1;
		missile->SpriteFrame = -missile->SpriteFrame - 1;
	}
	totalx = abs(missile->DX - missile->SourceX);
	dx = abs(missile->X - missile->SourceX);
	f = missile->Type->SpriteFrames / (missile->Type->NumDirections / 2 + 1);
	f = 2 * f - 1;
	for (int i = 1, j = 1; i <= f; ++i) {
		if (dx * f / i < totalx) {
			if ((i - 1) * 2 < f) {
				j = i - 1;
			} else {
				j = f - i;
			}
			missile->SpriteFrame = missile->SpriteFrame % (missile->Type->NumDirections / 2 + 1) +
				j * (missile->Type->NumDirections / 2 + 1);
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
static void MissilesActionLoop(std::vector<Missile *> &missiles)
{
	//
	// NOTE: missiles[??] could be modified!!! Yes (freed)
	//
	for (size_t i = 0; i != missiles.size(); ) {

		if (missiles[i]->Delay) {
			missiles[i]->Delay--;
			++i;
			continue;  // delay start of missile
		}

		if (missiles[i]->TTL > 0) {
			missiles[i]->TTL--;  // overall time to live if specified
		}

		if (!missiles[i]->TTL) {
			FreeMissile(missiles, i);
			continue;
		}

		Assert(missiles[i]->Wait);
		if (--missiles[i]->Wait) {  // wait until time is over
			++i;
			continue;
		}

		missiles[i]->Action();

		if (!missiles[i]->TTL) {
			FreeMissile(missiles, i);
			continue;
		}
		++i;
	}
}

/**
**  Handle all missile actions.
*/
void MissileActions(void)
{
	MissilesActionLoop(GlobalMissiles);
	MissilesActionLoop(LocalMissiles);
}

/**
**  Calculate distance from view-point to missle.
**
**  @param missile  Missile pointer for distance.
**
**  @return the computed value.
*/
int ViewPointDistanceToMissile(const Missile *missile)
{
	int x = (missile->X + missile->Type->Width / 2) / TileSizeX;
	int y = (missile->Y + missile->Type->Height / 2) / TileSizeY;  // pixel -> tile

	return ViewPointDistance(x, y);
}

/**
**  Get the burning building missile based on hp percent.
**
**  @param percent  HP percent
**
**  @return  the missile used for burning.
*/
MissileType *MissileBurningBuilding(int percent)
{
	for (std::vector<BurningBuildingFrame *>::iterator i = BurningBuildingFrames.begin();
		i != BurningBuildingFrames.end(); ++i) {
		if (percent > (*i)->Percent) {
			return (*i)->Missile;
		}
	}
	return NULL;
}

/**
**  Save the state of a missile to file.
**
**  @param file  Output file.
*/
void Missile::SaveMissile(CFile *file) const
{
	file->printf("Missile(\"type\", \"%s\",", this->Type->Ident.c_str());
	file->printf(" \"%s\",", this->Local ? "local" : "global");
	file->printf(" \"pos\", {%d, %d}, \"origin-pos\", {%d, %d}, \"goal\", {%d, %d},",
		this->X, this->Y, this->SourceX, this->SourceY, this->DX, this->DY);
	file->printf("\n  \"frame\", %d, \"state\", %d, \"anim-wait\", %d, \"wait\", %d, \"delay\", %d,\n ",
		this->SpriteFrame, this->State, this->AnimWait, this->Wait, this->Delay);

	if (this->SourceUnit) {
		file->printf(" \"source\", \"%s\",", UnitReference(this->SourceUnit).c_str());
	}
	if (this->TargetUnit) {
		file->printf(" \"target\", \"%s\",", UnitReference(this->TargetUnit).c_str());
	}

	file->printf(" \"damage\", %d,", this->Damage);

	file->printf(" \"ttl\", %d,", this->TTL);
	if (this->Hidden) {
		file->printf(" \"hidden\", ");
	}

	file->printf(" \"step\", {%d, %d}", this->CurrentStep, this->TotalStep);

	// Slot filled in during init

	file->printf(")\n");
}

/**
**  Save the state missiles to file.
**
**  @param file  Output file.
*/
void SaveMissiles(CFile *file)
{
	std::vector<Missile *>::const_iterator i;

	file->printf("\n--- -----------------------------------------\n");
	file->printf("--- MODULE: missiles\n\n");

	for (i = GlobalMissiles.begin(); i != GlobalMissiles.end(); ++i) {
		(*i)->SaveMissile(file);
	}
	for (i = LocalMissiles.begin(); i != LocalMissiles.end(); ++i) {
		(*i)->SaveMissile(file);
	}
}

/**
**  Initialize missile type.
*/
void MissileType::Init(void)
{
	//
	// Resolve impact missiles and sounds.
	//
	if (!this->FiredSound.Name.empty()) {
		this->FiredSound.Sound = SoundForName(this->FiredSound.Name);
	}
	if (!this->ImpactSound.Name.empty()) {
		this->ImpactSound.Sound = SoundForName(this->ImpactSound.Name);
	}
	this->ImpactMissile = MissileTypeByIdent(this->ImpactName);
	this->SmokeMissile = MissileTypeByIdent(this->SmokeName);
}

/**
**  Initialize missile-types.
*/
void InitMissileTypes(void)
{
	for (std::vector<MissileType*>::iterator i = MissileTypes.begin(); i != MissileTypes.end(); ++i) {
		(*i)->Init();
	}
}

/**
**  Constructor.
*/
MissileType::MissileType(const std::string &ident) :
	Ident(ident), Transparency(0), Width(0), Height(0),
	DrawLevel(0), SpriteFrames(0), NumDirections(0),
	Flip(false), CanHitOwner(false), FriendlyFire(false),
	Class(), NumBounces(0), StartDelay(0), Sleep(0), Speed(0),
	Range(0), SplashFactor(0), ImpactMissile(NULL),
	SmokeMissile(NULL), ImpactParticle(NULL), G(NULL)
{
};

/**
**  Destructor.
*/
MissileType::~MissileType()
{
	CGraphic::Free(this->G);
	delete ImpactParticle;
}

/**
**  Clean up missile-types.
*/
void CleanMissileTypes(void)
{
	for (std::vector<MissileType*>::iterator i = MissileTypes.begin(); i != MissileTypes.end(); ++i) {
		delete *i;
	}
	MissileTypes.clear();
	MissileTypeMap.clear();
}

/**
**  Initialize missiles.
*/
void InitMissiles(void)
{
}

/**
**  Clean up missiles.
*/
void CleanMissiles(void)
{
	std::vector<Missile*>::const_iterator i;

	for (i = GlobalMissiles.begin(); i != GlobalMissiles.end(); ++i) {
		delete *i;
	}
	GlobalMissiles.clear();
	for (i = LocalMissiles.begin(); i != LocalMissiles.end(); ++i) {
		delete *i;
	}
	LocalMissiles.clear();
}

void FreeBurningBuildingFrames()
{
	for (std::vector<BurningBuildingFrame *>::iterator i = BurningBuildingFrames.begin();
			i != BurningBuildingFrames.end(); ++i) {
		delete *i;
	}
	BurningBuildingFrames.clear();
}

/*----------------------------------------------------------------------------
--    Functions (Spells Controllers/Callbacks) TODO: move to another file?
----------------------------------------------------------------------------*/

// ****************************************************************************
// Actions for the missiles
// ****************************************************************************

/*
**  Missile controllers
**
**  To cancel a missile set it's TTL to 0, it will be handled right after
**  the controller call and missile will be down.
*/

/**
**  Missile does nothing
*/
void MissileNone::Action()
{
	this->Wait = this->Type->Sleep;
	// Busy doing nothing.
}

/**
**  Missile flies from x,y to x1,y1 animation on the way
*/
void MissilePointToPoint::Action()
{
	this->Wait = this->Type->Sleep;
	if (PointToPointMissile(this)) {
		MissileHit(this);
		this->TTL = 0;
	} else {
		NextMissileFrame(this, 1, 0);
	}
}

/**
**  Missile flies from x,y to x1,y1 showing the first frame
**  and then shows a hit animation.
*/
void MissilePointToPointWithHit::Action()
{
	this->Wait = this->Type->Sleep;
	if (PointToPointMissile(this)) {
		if (NextMissileFrame(this, 1, 0)) {
			MissileHit(this);
			this->TTL = 0;
		}
	}
}

/**
**  Missile flies from x,y to x1,y1 and stays there for a moment
*/
void MissilePointToPointCycleOnce::Action()
{
	this->Wait = this->Type->Sleep;
	if (PointToPointMissile(this)) {
		MissileHit(this);
		this->TTL = 0;
	} else {
		NextMissileFrameCycle(this);
	}
}

/**
**  Missile don't move, than disappears
*/
void MissileStay::Action()
{
	this->Wait = this->Type->Sleep;
	if (NextMissileFrame(this, 1, 0)) {
		MissileHit(this);
		this->TTL = 0;
	}
}

/**
**  Missile flies from x,y to x1,y1 than bounces NumBounces times
*/
void MissilePointToPointBounce::Action()
{
	this->Wait = this->Type->Sleep;
	if (PointToPointMissile(this)) {
		if (this->State < 2 * this->Type->NumBounces - 1 && this->TotalStep) {
			int xstep;
			int ystep;

			xstep = (this->DX - this->SourceX) * 1024 / this->TotalStep;
			ystep = (this->DY - this->SourceY) * 1024 / this->TotalStep;
			this->DX += xstep * (TileSizeX + TileSizeY) * 3 / 4 / 1024;
			this->DY += ystep * (TileSizeX + TileSizeY) * 3 / 4 / 1024;

			this->State++; // !(State & 1) to initialise
			this->SourceX = this->X;
			this->SourceY = this->Y;
			PointToPointMissile(this);
			//this->State++;
			MissileHit(this);
			// FIXME: hits to left and right
			// FIXME: reduce damage effects on later impacts
		} else {
			MissileHit(this);
			this->TTL = 0;
		}
	} else {
		NextMissileFrame(this, 1, 0);
	}
}

/**
**  Missile doesn't move, it will just cycle once and vanish.
**  Used for ui missiles (cross shown when you give and order)
*/
void MissileCycleOnce::Action()
{
	this->Wait = this->Type->Sleep;
	switch (this->State) {
		case 0:
		case 2:
			++this->State;
			break;
		case 1:
			if (NextMissileFrame(this, 1, 0)) {
				++this->State;
			}
			break;
		case 3:
			if (NextMissileFrame(this, -1, 0)) {
				MissileHit(this);
				this->TTL = 0;
			}
			break;
	}
}

/**
**  Missile don't move, than checks the source unit for HP.
*/
void MissileFire::Action()
{
	CUnit *unit;

	unit = this->SourceUnit;
	this->Wait = this->Type->Sleep;
	if (unit->Destroyed || unit->Orders[0]->Action == UnitActionDie) {
		this->TTL = 0;
		return;
	}
	if (NextMissileFrame(this, 1, 0)) {
		int f;
		MissileType *fire;

		this->SpriteFrame = 0;
		f = (100 * unit->Variable[HP_INDEX].Value) / unit->Variable[HP_INDEX].Max;
		fire = MissileBurningBuilding(f);
		if (!fire) {
			this->TTL = 0;
			unit->Burning = 0;
		} else {
			if (this->Type != fire) {
				this->X += this->Type->Width / 2;
				this->Y += this->Type->Height / 2;
				this->Type = fire;
				this->X -= this->Type->Width / 2;
				this->Y -= this->Type->Height / 2;
			}
		}
	}
}

/**
**  Missile shows hit points?
*/
void MissileHit::Action()
{
	this->Wait = this->Type->Sleep;
	if (PointToPointMissile(this)) {
		::MissileHit(this);
		this->TTL = 0;
	}
}

/**
**  Missile flies from x,y to x1,y1 using a parabolic path
*/
void MissileParabolic::Action()
{
	this->Wait = this->Type->Sleep;
	if (ParabolicMissile(this)) {
		MissileHit(this);
		this->TTL = 0;
	} else {
		NextMissileFrameCycle(this);
	}
}

//@}
