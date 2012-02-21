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
/**@name missile.cpp - The missiles. */
//
//      (c) Copyright 1998-2007 by Lutz Sammer and Jimmy Salmon
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
#include "tileset.h"
#include "map.h"
#include "unitsound.h"
#include "unittype.h"
#include "player.h"
#include "unit.h"
#include "missile.h"
#include "sound.h"
#include "ui.h"
#include "actions.h"
#include "iolib.h"

#include "util.h"
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

extern NumberDesc *Damage;                   /// Damage calculation for missile.

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
void LoadMissileSprites()
{
#ifndef DYNAMIC_LOAD
	for (std::vector<MissileType*>::iterator i = MissileTypes.begin(); i != MissileTypes.end(); ++i) {
		(*i)->LoadMissileSprite();
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
	MissileType *mtype = new MissileType(ident);

	MissileTypeMap[ident] = mtype;
	MissileTypes.push_back(mtype);
	return mtype;
}

/**
**  Constructor
*/
Missile::Missile() :
	Type(NULL), SpriteFrame(0), State(0), AnimWait(0), Wait(0),
	Delay(0), SourceUnit(NULL), TargetUnit(NULL), Damage(0),
	TTL(-1), Hidden(0),
	CurrentStep(0), TotalStep(0),
	Local(0)
{
	position.x = 0;
	position.y = 0;
	destination.x = 0;
	destination.y = 0;
	source.x = 0;
	source.y = 0;
	this->Slot = Missile::Count++;
}

/**
**  Initialize a new made missile.
**
**  @param mtype      Type pointer of missile.
**  @param sourcePos  Missile start point in pixel.
**  @param destPos    Missile destination point in pixel.
**
**  @return       created missile.
*/
/* static */ Missile *Missile::Init(const MissileType &mtype, const PixelPos &startPos, const PixelPos &destPos)
{
	Missile *missile = NULL;

	switch (mtype.Class) {
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
		case MissileClassLandMine :
			missile = new MissileLandMine;
			break;
		case MissileClassWhirlwind :
			missile = new MissileWhirlwind;
			break;
		case MissileClassFlameShield :
			missile = new MissileFlameShield;
			break;
		case MissileClassDeathCoil :
			missile = new MissileDeathCoil;
			break;
		case MissileClassTracer :
			missile = new MissileTracer;
			break;
		case MissileClassClipToTarget :
			missile = new MissileClipToTarget;
			break;
	}
	const PixelPos halfSize = mtype.size / 2;
	missile->position = startPos - halfSize;
	missile->destination = destPos - halfSize;
	missile->source = missile->position;
	missile->Type = &mtype;
	missile->Wait = mtype.Sleep;
	missile->Delay = mtype.StartDelay;

	return missile;
}

/**
**  Create a new global missile at (x,y).
**
**  @param mtype    Type pointer of missile.
**  @param startPos Missile start point in pixel.
**  @param destPos  Missile destination point in pixel.
**
**  @return       created missile.
*/
Missile *MakeMissile(const MissileType &mtype, const PixelPos &startPos, const PixelPos &destPos)
{
	Missile *missile = Missile::Init(mtype, startPos, destPos);

	GlobalMissiles.push_back(missile);
	return missile;
}

/**
**  Create a new local missile at (x,y).
**
**  @param mtype     Type pointer of missile.
**  @param startPos  Missile start point in pixel.
**  @param destPos   Missile destination point in pixel.
**
**  @return       created missile.
*/
Missile *MakeLocalMissile(const MissileType &mtype, const PixelPos &startPos, const PixelPos &destPos)
{
	Missile *missile = Missile::Init(mtype, startPos, destPos);

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
static void FreeMissile(std::vector<Missile *> &missiles, std::vector<Missile*>::size_type i)
{
	CUnit *unit;
	Missile *missile = missiles[i];
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
**  @param attacker_stats  Attacker attributes.
**  @param goal_stats      Goal attributes.
**  @param bloodlust       If attacker has bloodlust
**  @param xp              Experience of attacker.
**
**  @return                damage inflicted to goal.
*/
static int CalculateDamageStats(const CUnitStats &attacker_stats,
	const CUnitStats &goal_stats, int bloodlust)
{
	int basic_damage = attacker_stats.Variables[BASICDAMAGE_INDEX].Value;
	int piercing_damage = attacker_stats.Variables[PIERCINGDAMAGE_INDEX].Value;

	if (bloodlust) {
		basic_damage *= 2;
		piercing_damage *= 2;
	}

	int damage = std::max<int>(basic_damage - goal_stats.Variables[ARMOR_INDEX].Value, 1);
	damage += piercing_damage;
	damage -= SyncRand() % ((damage + 2) / 2);
	Assert(damage >= 0);

	return damage;
}

/**
**  Calculate damage.
**
**  @param attacker  Attacker.
**  @param goal      Goal unit.
**
**  @return          damage produces on goal.
*/
static int CalculateDamage(const CUnit &attacker, const CUnit &goal)
{
	if (!Damage) { // Use old method.
		return CalculateDamageStats(*attacker.Stats, *goal.Stats,
			attacker.Variable[BLOODLUST_INDEX].Value);
	}
	Assert(Damage);

	UpdateUnitVariables(const_cast<CUnit &>(attacker));
	UpdateUnitVariables(const_cast<CUnit &>(goal));
	TriggerData.Attacker = const_cast<CUnit *>(&attacker);
	TriggerData.Defender = const_cast<CUnit *>(&goal);
	const int res = EvalNumber(Damage);
	TriggerData.Attacker = NULL;
	TriggerData.Defender = NULL;
	return res;
}


/**
**  Get pixel position of the center of the specified tilePos.
*/
static PixelPos GetPixelPosFromCenterTile(const Vec2i& tilePos)
{
	PixelPos pixelPos = {tilePos.x * PixelTileSize.x + PixelTileSize.x / 2,
						tilePos.y * PixelTileSize.y + PixelTileSize.y / 2};

	return pixelPos;
}


/**
**  Fire missile.
**
**  @param unit  Unit that fires the missile.
*/
void FireMissile(CUnit &unit)
{
	CUnit *goal = unit.CurrentOrder()->GetGoal();

	// Goal dead?
	if (goal) {
		// Better let the caller/action handle this.
		if (goal->Destroyed) {
			DebugPrint("destroyed unit\n");
			return;
		}
		if (goal->Removed || goal->CurrentAction() == UnitActionDie) {
			return;
		}
		// FIXME: Some missile hit the field of the target and all units on it.
		// FIXME: goal is already dead, but missile could hit others?
	}

	// No missile hits immediately!
	if (unit.Type->Missile.Missile->Class == MissileClassNone) {
		// No goal, take target coordinates
		if (!goal) {
			const Vec2i& goalPos = unit.CurrentOrder()->goalPos;

			if (Map.WallOnMap(goalPos)) {
				if (Map.HumanWallOnMap(goalPos)) {
					Map.HitWall(goalPos,
						CalculateDamageStats(*unit.Stats,
							*UnitTypeHumanWall->Stats, unit.Variable[BLOODLUST_INDEX].Value));
				} else {
					Map.HitWall(goalPos,
						CalculateDamageStats(*unit.Stats,
							*UnitTypeOrcWall->Stats, unit.Variable[BLOODLUST_INDEX].Value));
				}
				return;
			}
			DebugPrint("Missile-none hits no unit, shouldn't happen!\n");
			return;
		}
		HitUnit(&unit, *goal, CalculateDamage(unit, *goal));
		return;
	}

	// If Firing from inside a Bunker
	CUnit* from = GetFirstContainer(unit);
	const PixelPos startPixelPos = GetPixelPosFromCenterTile(from->tilePos);

	Vec2i dpos;
	if (goal) {
		Assert(goal->Type);  // Target invalid?
		// Moved out of attack range?

		if (unit.MapDistanceTo(*goal) < unit.Type->MinAttackRange) {
			DebugPrint("Missile target too near %d,%d\n" _C_
				unit.MapDistanceTo(*goal) _C_ unit.Type->MinAttackRange);
			// FIXME: do something other?
			return;
		}
		// Fire to nearest point of the unit!
		// If Firing from inside a Bunker
		NearestOfUnit(*goal, GetFirstContainer(unit)->tilePos, &dpos);
	} else {
		dpos = unit.CurrentOrder()->goalPos;
		// FIXME: Can this be too near??
	}

	const PixelPos destPixelPos = GetPixelPosFromCenterTile(dpos);
	Missile *missile = MakeMissile(*unit.Type->Missile.Missile, startPixelPos, destPixelPos);
	//
	// Damage of missile
	//
	if (goal) {
		missile->TargetUnit = goal;
		goal->RefsIncrease();
	}
	missile->SourceUnit = &unit;
	unit.RefsIncrease();
}

/**
**  Get area of tiles covered by missile
**
**  @param missile  Missile to be checked and set.
**  @param boxMin       OUT: Pointer to top left corner in map tiles.
**  @param boxMax       OUT: Pointer to bottom right corner in map tiles.
**
**  @return         sx,sy,ex,ey defining area in Map
*/
static void GetMissileMapArea(const Missile &missile, Vec2i& boxMin, Vec2i &boxMax)
{
#define BoundX(x) std::min<int>(std::max<int>(0, x), Map.Info.MapWidth - 1)
#define BoundY(y) std::min<int>(std::max<int>(0, y), Map.Info.MapHeight - 1)

	boxMin.x = BoundX(missile.position.x / PixelTileSize.x);
	boxMin.y = BoundY(missile.position.y / PixelTileSize.y);
	boxMax.x = BoundX((missile.position.x + missile.Type->Width() + PixelTileSize.x - 1) / PixelTileSize.x);
	boxMax.y = BoundY((missile.position.y + missile.Type->Height() + PixelTileSize.y - 1) / PixelTileSize.y);

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
static int MissileVisibleInViewport(const CViewport &vp, const Missile &missile)
{
	Vec2i boxmin;
	Vec2i boxmax;

	GetMissileMapArea(missile, boxmin, boxmax);
	if (!vp.AnyMapAreaVisibleInViewport(boxmin, boxmax)) {
		return 0;
	}
	Vec2i pos;
	for (pos.x = boxmin.x; pos.x <= boxmax.x; ++pos.x) {
		for (pos.y = boxmin.y; pos.y <= boxmax.y; ++pos.y) {
			if (ReplayRevealMap || Map.IsFieldVisible(*ThisPlayer, pos)) {
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
**  @param pos    Screen pixel position
*/
void MissileType::DrawMissileType(int frame, const PixelPos &pos) const
{
#ifdef DYNAMIC_LOAD
	if (!this->G->IsLoaded()) {
		LoadMissileSprite(this);
	}
#endif

	if (this->Flip) {
		if (frame < 0) {
			if (this->Transparency > 0) {
				this->G->DrawFrameClipTransX(-frame - 1, pos.x, pos.y, int(256-2.56*Transparency));
			} else {
				this->G->DrawFrameClipX(-frame - 1, pos.x, pos.y);
			}
		} else {
			if (this->Transparency > 0) {
				this->G->DrawFrameClipTrans(frame, pos.x, pos.y, int(256-2.56*Transparency));
			} else {
				this->G->DrawFrameClip(frame, pos.x, pos.y);
			}
		}
	} else {
		const int row = this->NumDirections / 2 + 1;

		if (frame < 0) {
			frame = ((-frame - 1) / row) * this->NumDirections + this->NumDirections - (-frame - 1) % row;
		} else {
			frame = (frame / row) * this->NumDirections + frame % row;
		}
		if (this->Transparency > 0) {
			this->G->DrawFrameClipTrans(frame, pos.x, pos.y, int(256-2.56*Transparency));
		} else {
			this->G->DrawFrameClip(frame, pos.x, pos.y);
		}
	}
}

void MissileDrawProxy::DrawMissile(const CViewport &vp) const
{
	const PixelPos sceenPixelPos = vp.MapToScreenPixelPos(this->pixelPos);

	switch (this->Type->Class) {
		case MissileClassHit:
			CLabel(GetGameFont()).DrawClip(sceenPixelPos.x, sceenPixelPos.y, this->data.Damage);
			break;
		default:
			this->Type->DrawMissileType(this->data.SpriteFrame, sceenPixelPos);
			break;
	}
}

void MissileDrawProxy::operator=(const Missile* missile) {
	this->Type = missile->Type;
	this->pixelPos = missile->position;
	if (missile->Type->Class == MissileClassHit) {
		this->data.Damage = missile->Damage;
	} else {
		this->data.SpriteFrame = missile->SpriteFrame;
	}
}

/**
**  Draw missile.
*/
void Missile::DrawMissile(const CViewport &vp) const
{
	Assert(this->Type);

	// FIXME: I should copy SourcePlayer for second level missiles.
	if (this->SourceUnit && this->SourceUnit->Player) {
#ifdef DYNAMIC_LOAD
		if (!this->Type->Sprite) {
			LoadMissileSprite(this->Type);
		}
#endif
	}
	const PixelPos screenPixelPos = vp.MapToScreenPixelPos(this->position);

	switch (this->Type->Class) {
		case MissileClassHit:
			CLabel(GetGameFont()).DrawClip(screenPixelPos.x, screenPixelPos.y, this->Damage);
			break;
		default:
			this->Type->DrawMissileType(this->SpriteFrame, screenPixelPos);
			break;
	}
}

static bool MissileDrawLevelCompare(const Missile*const l, const Missile*const r)
{
	if (l->Type->DrawLevel == r->Type->DrawLevel) {
		return l->Slot < r->Slot;
	} else {
		return l->Type->DrawLevel < r->Type->DrawLevel;
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
int FindAndSortMissiles(const CViewport &vp, Missile *table[], const int tablesize)
{
	int nmissiles = 0;
	std::vector<Missile *>::const_iterator i;

	// Loop through global missiles, then through locals.
	for (i = GlobalMissiles.begin(); i != GlobalMissiles.end() && nmissiles < tablesize; ++i) {
		Missile &missile = *(*i);
		if (missile.Delay || missile.Hidden) {
			continue;  // delayed or hidden -> aren't shown
		}
		// Draw only visible missiles
		if (MissileVisibleInViewport(vp, missile)) {
			table[nmissiles++] = &missile;
		}
	}

	for (i = LocalMissiles.begin(); i != LocalMissiles.end() && nmissiles < tablesize; ++i) {
		Missile &missile = *(*i);
		if (missile.Delay || missile.Hidden) {
			continue;  // delayed or hidden -> aren't shown
		}
		// Local missile are visible.
		table[nmissiles++] = &missile;
	}
	if (nmissiles > 1) {
		std::sort(table, table + nmissiles, MissileDrawLevelCompare);
	}
	return nmissiles;
}

int FindAndSortMissiles(const CViewport &vp, MissileDrawProxy table[], const int tablesize)
{
	Assert(tablesize <= MAX_MISSILES * 9);

	Missile *buffer[MAX_MISSILES * 9];
	const int n = FindAndSortMissiles(vp, buffer, MAX_MISSILES * 9);

	for (int i = 0; i < n; ++i) {
		table[i] = buffer[i];
	}
	return n;
}


/**
**  Change missile heading from x,y.
**
**  @param missile  Missile.
**  @param delta    Delta movement
**
**  @internal We have : SpriteFrame / (2 * (Numdirection - 1)) == DirectionToHeading / 256.
*/
static void MissileNewHeadingFromXY(Missile &missile, const PixelPos &delta)
{
	if (missile.Type->NumDirections == 1 || (delta.x == 0 && delta.y == 0)) {
		return;
	}

	if (missile.SpriteFrame < 0) {
		missile.SpriteFrame = -missile.SpriteFrame - 1;
	}
	missile.SpriteFrame /= missile.Type->NumDirections / 2 + 1;
	missile.SpriteFrame *= missile.Type->NumDirections / 2 + 1;

	const int nextdir = 256 / missile.Type->NumDirections;
	Assert(nextdir != 0);
	const int dir = ((DirectionToHeading(delta) + nextdir / 2) & 0xFF) / nextdir;
	if (dir <= LookingS / nextdir) { // north->east->south
		missile.SpriteFrame += dir;
	} else {
		missile.SpriteFrame += 256 / nextdir - dir;
		missile.SpriteFrame = -missile.SpriteFrame - 1;
	}
}

/**
**  Init the move.
**
**  @param missile  missile to initialise for movement.
**
**  @return         1 if goal is reached, 0 else.
*/
static int MissileInitMove(Missile &missile)
{
	const PixelPos heading = missile.destination - missile.position;

	MissileNewHeadingFromXY(missile, heading);
	if (!(missile.State & 1)) {
		missile.CurrentStep = 0;
		missile.TotalStep = 0;
		if (heading.x == 0 && heading.y == 0) {
			return 1;
		}
		// initialize
		missile.TotalStep = MapDistance(missile.source, missile.destination);
		missile.State++;
		return 0;
	}
	Assert(missile.TotalStep != 0);
	missile.CurrentStep += missile.Type->Speed;
	if (missile.CurrentStep >= missile.TotalStep) {
		missile.position = missile.destination;
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
static int PointToPointMissile(Missile &missile)
{
	if (MissileInitMove(missile) == 1) {
		return 1;
	}

	Assert(missile.Type != NULL);
	Assert(missile.TotalStep != 0);

	const PixelPos diff = (missile.destination - missile.source);
	missile.position = missile.source + diff * missile.CurrentStep / missile.TotalStep;

	if (missile.Type->SmokeMissile && missile.CurrentStep) {
		const PixelPos position =  missile.position + missile.Type->size / 2;
		MakeMissile(*missile.Type->SmokeMissile, position, position);
	}
	return 0;
}

/**
**  Handle tracer missile.
**
**  @param missile  Missile pointer.
**
**  @return         1 if goal is reached, 0 else.
*/
static int TracerMissile(Missile &missile)
{
	if (MissileInitMove(missile) == 1) {
		return 1;
	}

	Assert(missile.Type != NULL);
	Assert(missile.TargetUnit != NULL);
	Assert(missile.TotalStep != 0);

	missile.destination.x = missile.TargetUnit->tilePos.x * PixelTileSize.x + missile.TargetUnit->IX;
	missile.destination.y = missile.TargetUnit->tilePos.y * PixelTileSize.y + missile.TargetUnit->IY;

	const PixelPos diff = (missile.destination - missile.source);
	missile.position = missile.source + diff * missile.CurrentStep / missile.TotalStep;

	if (missile.Type->SmokeMissile && missile.CurrentStep) {
		const PixelPos position =  missile.position + missile.Type->size / 2;
		MakeMissile(*missile.Type->SmokeMissile, position, position);
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
static int ParabolicMissile(Missile &missile)
{
	int k;        // Coefficient of the parabol.
	int zprojToX; // Projection of Z axis on axis X.
	int zprojToY; // Projection of Z axis on axis Y.
	int z;        // should be missile.Z later.

	k = -2048; //-1024; // Should be initialised by an other method (computed with distance...)
	zprojToX = 4;
	zprojToY = 1024;
	if (MissileInitMove(missile) == 1) {
		return 1;
	}
	Assert(missile.Type != NULL);
	const PixelPos orig_pos = missile.position;
	Assert(missile.TotalStep != 0);
	const PixelPos diff = (missile.destination - missile.source);
	missile.position = missile.source + diff * missile.CurrentStep / missile.TotalStep;

	Assert(k != 0);
	z = missile.CurrentStep * (missile.TotalStep - missile.CurrentStep) / k;
	// Until Z is used for drawing, modify X and Y.
	missile.position.x += z * zprojToX / 64;
	missile.position.y += z * zprojToY / 64;
	MissileNewHeadingFromXY(missile, missile.position - orig_pos);
	if (missile.Type->SmokeMissile && missile.CurrentStep) {
		const PixelPos position = missile.position + missile.Type->size / 2;
		MakeMissile(*missile.Type->SmokeMissile, position, position);
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
static void MissileHitsGoal(const Missile &missile, CUnit &goal, int splash)
{
	if (!missile.Type->CanHitOwner && &goal == missile.SourceUnit) {
		return;
	}

	if (goal.CurrentAction() != UnitActionDie) {
		if (missile.Damage) {  // direct damage, spells mostly
			HitUnit(missile.SourceUnit, goal, missile.Damage / splash);
		} else {
			Assert(missile.SourceUnit != NULL);
			HitUnit(missile.SourceUnit, goal,
				CalculateDamage(*missile.SourceUnit, goal) / splash);
		}
	}
}

/**
**  Missile hits wall.
**
**  @param missile  Missile hitting the goal.
**  @param tilePos  Wall map tile position.
**  @param splash   Splash damage divisor.
**
**  @todo FIXME: Support for more races.
*/
static void MissileHitsWall(const Missile &missile, const Vec2i &tilePos, int splash)
{
	CUnitStats *stats; // stat of the wall.

	if (!Map.WallOnMap(tilePos)) {
		return;
	}
	if (missile.Damage) {  // direct damage, spells mostly
		Map.HitWall(tilePos, missile.Damage / splash);
		return;
	}

	Assert(missile.SourceUnit != NULL);
	if (Map.HumanWallOnMap(tilePos)) {
		stats = UnitTypeHumanWall->Stats;
	} else {
		Assert(Map.OrcWallOnMap(tilePos));
		stats = UnitTypeOrcWall->Stats;
	}
	Map.HitWall(tilePos, CalculateDamageStats(*missile.SourceUnit->Stats, *stats, 0) / splash);
}

/**
**  Work for missile hit.
**
**  @param missile  Missile reaching end-point.
*/
static void MissileHit(Missile &missile)
{
	const MissileType &mtype = *missile.Type;

	if (mtype.ImpactSound.Sound) {
		PlayMissileSound(&missile, mtype.ImpactSound.Sound);
	}
	const PixelPos pixelPos = missile.position + missile.Type->size / 2;

	//
	// The impact generates a new missile.
	//
	if (mtype.ImpactMissile) {
		MakeMissile(*mtype.ImpactMissile, pixelPos, pixelPos);
	}
	if (mtype.ImpactParticle) {
		mtype.ImpactParticle->pushPreamble();
		mtype.ImpactParticle->pushInteger(pixelPos.x);
		mtype.ImpactParticle->pushInteger(pixelPos.y);
		mtype.ImpactParticle->run();
	}

	if (!missile.SourceUnit) {  // no owner - green-cross ...
		return;
	}

	const Vec2i pos = {pixelPos.x / PixelTileSize.x, pixelPos.y / PixelTileSize.y};

	if (!Map.Info.IsPointOnMap(pos)) {
		// FIXME: this should handled by caller?
		DebugPrint("Missile gone outside of map!\n");
		return;  // outside the map.
	}

	//
	// Choose correct goal.
	//
	if (!mtype.Range) {
		if (missile.TargetUnit && (mtype.FriendlyFire == false
				|| missile.TargetUnit->Player->Index != missile.SourceUnit->Player->Index)) {
			//
			// Missiles without range only hits the goal always.
			//
			CUnit &goal = *missile.TargetUnit;
			if (goal.Destroyed) {  // Destroyed
				goal.RefsDecrease();
				missile.TargetUnit = NoUnitP;
				return;
			}
			MissileHitsGoal(missile, goal, 1);
			return;
		}
		MissileHitsWall(missile, pos, 1);
		return;
	}

	{
		//
		// Hits all units in range.
		//
		const int range = mtype.Range;
		CUnit *table[UnitMax];
		const int n = Map.Select(pos.x - range + 1, pos.y - range + 1, pos.x + range, pos.y + range, table);
		Assert(missile.SourceUnit != NULL);
		for (int i = 0; i < n; ++i) {
			CUnit &goal = *table[i];
			//
			// Can the unit attack the this unit-type?
			// NOTE: perhaps this should be come a property of the missile.
			// Also check CorrectSphashDamage so land explosions can't hit the air units
			//
			if (CanTarget(missile.SourceUnit->Type, goal.Type)
				&& (mtype.CorrectSphashDamage == false
					|| goal.Type->UnitType == missile.TargetUnit->Type->UnitType)
				&& (mtype.FriendlyFire == false
					|| (missile.TargetUnit->Player->Index != missile.SourceUnit->Player->Index))) {
				int splash = goal.MapDistanceTo(pos.x, pos.y);

				if (splash) {
					splash *= mtype.SplashFactor;
				} else {
					splash = 1;
				}
				MissileHitsGoal(missile, goal, splash);
			}
		}
	}

	// Missile hits ground.
	const Vec2i offset = { mtype.Range, mtype.Range};
	const Vec2i posmin = pos - offset;
	for (int i = mtype.Range * 2; --i;) {
		for (int j = mtype.Range * 2; --j;) {
			const Vec2i posIt = {posmin.x + i, posmin.y + j};

			if (Map.Info.IsPointOnMap(posIt)) {
				int d = MapDistance(pos, posIt);
				d *= mtype.SplashFactor;
				if (d == 0) {
					d = 1;
				}
				MissileHitsWall(missile, posIt, d);
			}
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
static int NextMissileFrame(Missile &missile, char sign, char longAnimation)
{
	int neg;                 // True for mirroring sprite.
	int animationIsFinished; // returned value.
	int numDirections;       // Number of direction of the missile.

	//
	// Animate missile, cycle through frames
	//
	neg = 0;
	animationIsFinished = 0;
	numDirections = missile.Type->NumDirections / 2 + 1;
	if (missile.SpriteFrame < 0) {
		neg = 1;
		missile.SpriteFrame = -missile.SpriteFrame - 1;
	}
	if (longAnimation) {
		int totalf;   // Total number of frame (for one direction).
		int df;       // Current frame (for one direction).
		int totalx;   // Total distance to cover.
		int dx;       // Covered distance.

		totalx = MapDistance(missile.destination, missile.source);
		dx = MapDistance(missile.position, missile.source);
		totalf = missile.Type->SpriteFrames / numDirections;
		df = missile.SpriteFrame / numDirections;
		if ((sign == 1 && dx * totalf <= df * totalx) ||
				(sign == -1 && dx * totalf > df * totalx)) {
			return animationIsFinished;
		}
	}
	missile.SpriteFrame += sign * numDirections;
	if (sign > 0) {
		if (missile.SpriteFrame >= missile.Type->SpriteFrames) {
			missile.SpriteFrame -= missile.Type->SpriteFrames;
			animationIsFinished = 1;
		}
	} else {
		if (missile.SpriteFrame < 0) {
			missile.SpriteFrame += missile.Type->SpriteFrames;
			animationIsFinished = 1;
		}
	}
	if (neg) {
		missile.SpriteFrame = -missile.SpriteFrame - 1;
	}

	return animationIsFinished;
}

/**
**  Pass the next frame of the animation.
**  This animation goes from start to finish ONCE on the way
**
**  @param missile  Missile pointer.
*/
static void NextMissileFrameCycle(Missile &missile)
{
	int neg = 0;

	if (missile.SpriteFrame < 0) {
		neg = 1;
		missile.SpriteFrame = -missile.SpriteFrame - 1;
	}
	const int totalx = abs(missile.destination.x - missile.source.x);
	const int dx = abs(missile.position.x - missile.source.x);
	int f = missile.Type->SpriteFrames / (missile.Type->NumDirections / 2 + 1);
	f = 2 * f - 1;
	for (int i = 1, j = 1; i <= f; ++i) {
		if (dx * f / i < totalx) {
			if ((i - 1) * 2 < f) {
				j = i - 1;
			} else {
				j = f - i;
			}
			missile.SpriteFrame = missile.SpriteFrame % (missile.Type->NumDirections / 2 + 1) +
				j * (missile.Type->NumDirections / 2 + 1);
			break;
		}
	}
	if (neg) {
		missile.SpriteFrame = -missile.SpriteFrame - 1;
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
	for (std::vector<Missile *>::size_type i = 0;
			i != missiles.size();) {
		Missile &missile = *missiles[i];

		if (missile.Delay) {
			missile.Delay--;
			++i;
			continue;  // delay start of missile
		}

		if (missile.TTL > 0) {
			missile.TTL--;  // overall time to live if specified
		}

		if (!missile.TTL) {
			FreeMissile(missiles, i);
			continue;
		}

		Assert(missile.Wait);
		if (--missile.Wait) {  // wait until time is over
			++i;
			continue;
		}

		missile.Action();

		if (!missile.TTL) {
			FreeMissile(missiles, i);
			continue;
		}
		++i;
	}
}

/**
**  Handle all missile actions.
*/
void MissileActions()
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
int ViewPointDistanceToMissile(const Missile &missile)
{
	const PixelPos pixelPos = missile.position + missile.Type->size / 2;
	const Vec2i tilePos = { pixelPos.x / PixelTileSize.x, pixelPos.y / PixelTileSize.y };

	return ViewPointDistance(tilePos);
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
**  Save a specific pos.
*/
static void SavePixelPos(CFile &file, const PixelPos &pos)
{
	file.printf("{%d, %d}", pos.x, pos.y);
}


/**
**  Save the state of a missile to file.
**
**  @param file  Output file.
*/
void Missile::SaveMissile(CFile &file) const
{
	file.printf("Missile(\"type\", \"%s\",", this->Type->Ident.c_str());
	file.printf(" \"%s\",", this->Local ? "local" : "global");
	file.printf(" \"pos\", ");
	SavePixelPos(file, this->position);
	file.printf(", \"origin-pos\", ");
	SavePixelPos(file, this->source);
	file.printf(", \"goal\", ");
	SavePixelPos(file, this->destination);
	file.printf(",\n  \"frame\", %d, \"state\", %d, \"anim-wait\", %d, \"wait\", %d, \"delay\", %d,\n ",
		this->SpriteFrame, this->State, this->AnimWait, this->Wait, this->Delay);
	if (this->SourceUnit) {
		file.printf(" \"source\", \"%s\",", UnitReference(*this->SourceUnit).c_str());
	}
	if (this->TargetUnit) {
		file.printf(" \"target\", \"%s\",", UnitReference(*this->TargetUnit).c_str());
	}
	file.printf(" \"damage\", %d,", this->Damage);
	file.printf(" \"ttl\", %d,", this->TTL);
	if (this->Hidden) {
		file.printf(" \"hidden\", ");
	}
	file.printf(" \"step\", {%d, %d}", this->CurrentStep, this->TotalStep);

	// Slot filled in during init
	file.printf(")\n");
}

/**
**  Save the state missiles to file.
**
**  @param file  Output file.
*/
void SaveMissiles(CFile &file)
{
	std::vector<Missile *>::const_iterator i;

	file.printf("\n--- -----------------------------------------\n");
	file.printf("--- MODULE: missiles\n\n");

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
void MissileType::Init()
{
	// Resolve impact missiles and sounds.
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
void InitMissileTypes()
{
#if 0
	// Rehash.
	for (std::vector<MissileType*>::iterator i = MissileTypes.begin(); i != MissileTypes.end(); ++i) {
		MissileTypeMap[(*i)->Ident] = *i;
	}
#endif
	for (std::vector<MissileType*>::iterator i = MissileTypes.begin(); i != MissileTypes.end(); ++i) {
		(*i)->Init();
	}
}

/**
**  Constructor.
*/
MissileType::MissileType(const std::string &ident) :
	Ident(ident), Transparency(0),
	DrawLevel(0), SpriteFrames(0), NumDirections(0),
	CorrectSphashDamage(false), Flip(false), CanHitOwner(false), FriendlyFire(false),
	Class(), NumBounces(0), StartDelay(0), Sleep(0), Speed(0),
	Range(0), SplashFactor(0), ImpactMissile(NULL),
	SmokeMissile(NULL), ImpactParticle(NULL), G(NULL)
{
	size.x = 0;
	size.y = 0;
	FiredSound.Sound = NULL;
	ImpactSound.Sound = NULL;
}

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
void CleanMissileTypes()
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
void InitMissiles()
{
}

/**
**  Clean up missiles.
*/
void CleanMissiles()
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

#ifdef DEBUG
void FreeBurningBuildingFrames()
{
	for (std::vector<BurningBuildingFrame *>::iterator i = BurningBuildingFrames.begin();
			i != BurningBuildingFrames.end(); ++i) {
		delete *i;
	}
	BurningBuildingFrames.clear();
}
#endif

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
	if (PointToPointMissile(*this)) {
		MissileHit(*this);
		this->TTL = 0;
	} else {
		NextMissileFrame(*this, 1, 0);
	}
}

/**
**  Missile flies from x,y to x1,y1 showing the first frame
**  and then shows a hit animation.
*/
void MissilePointToPointWithHit::Action()
{
	this->Wait = this->Type->Sleep;
	if (PointToPointMissile(*this)) {
		if (NextMissileFrame(*this, 1, 0)) {
			MissileHit(*this);
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
	if (PointToPointMissile(*this)) {
		MissileHit(*this);
		this->TTL = 0;
	} else {
		NextMissileFrameCycle(*this);
	}
}

/**
**  Missile don't move, than disappears
*/
void MissileStay::Action()
{
	this->Wait = this->Type->Sleep;
	if (NextMissileFrame(*this, 1, 0)) {
		MissileHit(*this);
		this->TTL = 0;
	}
}

/**
**  Missile flies from x,y to x1,y1 than bounces NumBounces times
*/
void MissilePointToPointBounce::Action()
{
	this->Wait = this->Type->Sleep;
	if (PointToPointMissile(*this)) {
		if (this->State < 2 * this->Type->NumBounces - 1 && this->TotalStep) {
			const PixelPos step = (this->destination - this->source);

			this->destination += step * ((PixelTileSize.x + PixelTileSize.y) * 3) / 4 / this->TotalStep;
			this->State++; // !(State & 1) to initialise
			this->source = this->position;
			PointToPointMissile(*this);
			//this->State++;
			MissileHit(*this);
			// FIXME: hits to left and right
			// FIXME: reduce damage effects on later impacts
		} else {
			MissileHit(*this);
			this->TTL = 0;
		}
	} else {
		NextMissileFrame(*this, 1, 0);
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
			if (NextMissileFrame(*this, 1, 0)) {
				++this->State;
			}
			break;
		case 3:
			if (NextMissileFrame(*this, -1, 0)) {
				MissileHit(*this);
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
	CUnit &unit = *this->SourceUnit;

	this->Wait = this->Type->Sleep;
	if (unit.Destroyed || unit.CurrentAction() == UnitActionDie) {
		this->TTL = 0;
		return;
	}
	if (NextMissileFrame(*this, 1, 0)) {
		this->SpriteFrame = 0;
		const int f = (100 * unit.Variable[HP_INDEX].Value) / unit.Variable[HP_INDEX].Max;
		MissileType *fire = MissileBurningBuilding(f);

		if (!fire) {
			this->TTL = 0;
			unit.Burning = 0;
		} else {
			if (this->Type != fire) {
				this->position += this->Type->size / 2;
				this->Type = fire;
				this->position -= this->Type->size / 2;
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
	if (PointToPointMissile(*this)) {
		::MissileHit(*this);
		this->TTL = 0;
	}
}

/**
**  Missile flies from x,y to x1,y1 using a parabolic path
*/
void MissileParabolic::Action()
{
	this->Wait = this->Type->Sleep;
	if (ParabolicMissile(*this)) {
		MissileHit(*this);
		this->TTL = 0;
	} else {
		NextMissileFrameCycle(*this);
	}
}

/**
**  FlameShield controller
*/
void MissileFlameShield::Action()
{
	static int fs_dc[] = {
		0, 32, 5, 31, 10, 30, 16, 27, 20, 24, 24, 20, 27, 15, 30, 10, 31,
		5, 32, 0, 31, -5, 30, -10, 27, -16, 24, -20, 20, -24, 15, -27, 10,
		-30, 5, -31, 0, -32, -5, -31, -10, -30, -16, -27, -20, -24, -24, -20,
		-27, -15, -30, -10, -31, -5, -32, 0, -31, 5, -30, 10, -27, 16, -24,
		20, -20, 24, -15, 27, -10, 30, -5, 31, 0, 32};

	this->Wait = this->Type->Sleep;
	const int index = this->TTL % 36;  // 36 positions on the circle
	const int dx = fs_dc[index * 2];
	const int dy = fs_dc[index * 2 + 1];
	CUnit *unit = this->TargetUnit;
	//
	// Show around the top most unit.
	// FIXME: conf, do we hide if the unit is contained or not?
	//
	while (unit->Container) {
		unit = unit->Container;
	}
	const Vec2i upos = unit->tilePos;
	const int ix = unit->IX;
	const int iy = unit->IY;
	const int uw = unit->Type->TileWidth;
	const int uh = unit->Type->TileHeight;
	this->position.x = upos.x * PixelTileSize.x + ix + uw * PixelTileSize.x / 2 + dx - 16;
	this->position.y = upos.y * PixelTileSize.y + iy + uh * PixelTileSize.y / 2 + dy - 32;
	if (unit->CurrentAction() == UnitActionDie) {
		this->TTL = index;
	}

	if (unit->Container) {
		this->Hidden = 1;
		return;  // Hidden missile don't do damage.
	} else {
		this->Hidden = 0;
	}

	// Only hit 1 out of 8 frames
	if (this->TTL & 7) {
		return;
	}

	CUnit* table[UnitMax];
	const int n = Map.Select(upos.x - 1, upos.y - 1, upos.x + 1 + 1, upos.y + 1 + 1, table);
	for (int i = 0; i < n; ++i) {
		if (table[i] == unit) {
			// cannot hit target unit
			continue;
		}
		if (table[i]->CurrentAction() != UnitActionDie) {
			HitUnit(this->SourceUnit, *table[i], this->Damage);
		}
	}
}

struct LandMineTargetFinder {
	const CUnit *const source;
	int CanHitOwner;
	LandMineTargetFinder(const CUnit *unit, int hit):
		 source(unit), CanHitOwner(hit) {}
	inline bool operator() (const CUnit *const unit) const
	{
		return (
				!(unit == source && !CanHitOwner) &&
				unit->Type->UnitType != UnitTypeFly &&
				unit->CurrentAction() != UnitActionDie
				);
	}
	inline CUnit *FindOnTile(const CMapField *const mf) const
	{
		return mf->UnitCache.find(*this);
	}
};

/**
**  Land mine controller.
**  @todo start-finish-start cyclic animation.(anim scripts!)
**  @todo missile should dissapear for a while.
*/
void MissileLandMine::Action()
{
	const Vec2i pos = {this->position.x / PixelTileSize.x, this->position.y / PixelTileSize.y};

	if (LandMineTargetFinder(this->SourceUnit,
			this->Type->CanHitOwner).FindOnTile(Map.Field(pos)) != NULL) {
		DebugPrint("Landmine explosion at %d,%d.\n" _C_ pos.x _C_ pos.y);
		MissileHit(*this);
		this->TTL = 0;
		return;
	}
	if (!this->AnimWait--) {
		NextMissileFrame(*this, 1, 0);
		this->AnimWait = this->Type->Sleep;
	}
	this->Wait = 1;
}

/**
**  Whirlwind controller
**
**  @todo do it more configurable.
*/
void MissileWhirlwind::Action()
{
	//
	// Animate, move.
	//
	if (!this->AnimWait--) {
		if (NextMissileFrame(*this, 1, 0)) {
			this->SpriteFrame = 0;
			PointToPointMissile(*this);
		}
		this->AnimWait = this->Type->Sleep;
	}
	this->Wait = 1;
	//
	// Center of the tornado
	//
	PixelPos center = this->position + this->Type->size / 2;
	center.x = (center.x + PixelTileSize.x / 2) / PixelTileSize.x;
	center.y = (center.y + PixelTileSize.y) / PixelTileSize.y;

#if 0
	CUnit *table[UnitMax];
	int i;
	int n;

	//
	// Every 4 cycles 4 points damage in tornado center
	//
	if (!(this->TTL % 4)) {
		n = SelectUnitsOnTile(x, y, table);
		for (i = 0; i < n; ++i) {
			if (table[i]->CurrentAction() != UnitActionDie) {
				// should be missile damage ?
				HitUnit(this->SourceUnit, table[i], WHIRLWIND_DAMAGE1);
			}
		}
	}
	//
	// Every 1/10s 1 points damage on tornado periphery
	//
	if (!(this->TTL % (CYCLES_PER_SECOND/10))) {
		// we should parameter this
		n = SelectUnits(center.x - 1, center.y - 1, center.x + 1, center.y + 1, table);
		for (i = 0; i < n; ++i) {
			if ((table[i]->X != center.x || table[i]->Y != center.y) && table[i]->CurrentAction() != UnitActionDie) {
				// should be in missile
				HitUnit(this->SourceUnit, table[i], WHIRLWIND_DAMAGE2);
			}
		}
	}
	DebugPrint("Whirlwind: %d, %d, TTL: %d state: %d\n" _C_
			missile->X _C_ missile->Y _C_ missile->TTL _C_ missile->State);
#else
	if (!(this->TTL % CYCLES_PER_SECOND / 10)) {
		MissileHit(*this);
	}

#endif
	//
	// Changes direction every 3 seconds (approx.)
	//
	if (!(this->TTL % 100)) { // missile has reached target unit/spot
		int nx;
		int ny;

		do {
			// find new destination in the map
			nx = center.x + SyncRand() % 5 - 2;
			ny = center.y + SyncRand() % 5 - 2;
		} while (!Map.Info.IsPointOnMap(nx, ny));
		this->destination.x = nx * PixelTileSize.x + PixelTileSize.x / 2;
		this->destination.y = ny * PixelTileSize.y + PixelTileSize.y / 2;
		this->source = this->position;
		this->State = 0;
		DebugPrint("Whirlwind new direction: %d, %d, TTL: %d\n" _C_
			this->destination.x _C_ this->destination.y _C_ this->TTL);
	}
}

/**
**  Death-Coil class. Damages organic units and gives to the caster.
**
**  @todo  do it configurable.
*/
void MissileDeathCoil::Action()
{
	this->Wait = this->Type->Sleep;
	if (PointToPointMissile(*this)) {
		Assert(this->SourceUnit != NULL);
		CUnit &source = *this->SourceUnit;

		if (source.Destroyed) {
			return;
		}
		// source unit still exists
		//
		// Target unit still exists and casted on a special target
		//
		if (this->TargetUnit && !this->TargetUnit->Destroyed
			&& this->TargetUnit->CurrentAction() == UnitActionDie) {
			HitUnit(&source, *this->TargetUnit, this->Damage);
			if (source.CurrentAction() != UnitActionDie) {
				source.Variable[HP_INDEX].Value += this->Damage;
				if (source.Variable[HP_INDEX].Value > source.Variable[HP_INDEX].Max) {
					source.Variable[HP_INDEX].Value = source.Variable[HP_INDEX].Max;
				}
			}
		} else {
			//
			// No target unit -- try enemies in range 5x5 // Must be parametrable
			//
			int ec = 0;  // enemy count
			CUnit* table[UnitMax];
			const int x = this->destination.x / PixelTileSize.x;
			const int y = this->destination.y / PixelTileSize.y;
			const int n = Map.Select(x - 2, y - 2, x + 2 + 1, y + 2 + 1, table);

			if (n == 0) {
				return;
			}
			// calculate organic enemy count
			for (int i = 0; i < n; ++i) {
				ec += (source.IsEnemy(*table[i])
				/*&& table[i]->Type->Organic != 0*/);
			}
			if (ec > 0)  {
				// yes organic enemies found
				for (int i = 0; i < n; ++i) {
					if (source.IsEnemy(*table[i])/* && table[i]->Type->Organic != 0*/) {
						// disperse damage between them
						// NOTE: 1 is the minimal damage
						HitUnit(&source, *table[i], this->Damage / ec);
					}
				}
				if (source.CurrentAction() != UnitActionDie) {
					source.Variable[HP_INDEX].Value += this->Damage;
					if (source.Variable[HP_INDEX].Value > source.Variable[HP_INDEX].Max) {
						source.Variable[HP_INDEX].Value = source.Variable[HP_INDEX].Max;
					}
				}
			}
		}
		this->TTL = 0;
	}
}

/**
**  Missile flies from x,y to the target position, changing direction on the way
*/
void MissileTracer::Action()
{
	this->Wait = this->Type->Sleep;
	if (TracerMissile(*this)) {
		MissileHit(*this);
		this->TTL = 0;
	} else {
		NextMissileFrame(*this, 1, 0);
	}
}

/**
**  Missile remains clipped to target's current goal and plays his animation once
*/
void MissileClipToTarget::Action()
{
	Assert(this->TargetUnit != NULL);

	this->Wait = this->Type->Sleep;
	this->position.x = this->TargetUnit->tilePos.x * PixelTileSize.x + this->TargetUnit->IX;
	this->position.y = this->TargetUnit->tilePos.y * PixelTileSize.y + this->TargetUnit->IY;

	if (NextMissileFrame(*this, 1, 0)) {
		MissileHit(*this);
		this->TTL = 0;
	}
}

//@}
