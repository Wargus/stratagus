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

#include "missile.h"

#include "actions.h"
#include "font.h"
#include "iolib.h"
#include "luacallback.h"
#include "map.h"
#include "player.h"
#include "sound.h"
#include "trigger.h"
#include "ui.h"
#include "unit.h"
#include "unitsound.h"
#include "unittype.h"

/*----------------------------------------------------------------------------
--  Declarations
----------------------------------------------------------------------------*/

unsigned int Missile::Count = 0;

static std::vector<Missile *> GlobalMissiles;    /// all global missiles on map
static std::vector<Missile *> LocalMissiles;     /// all local missiles on map

/// lookup table for missile names
typedef std::map<std::string, MissileType *> MissileTypeMap;
static MissileTypeMap MissileTypes;

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
	for (MissileTypeMap::iterator it = MissileTypes.begin(); it != MissileTypes.end(); ++it) {
		(*it).second->LoadMissileSprite();
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
	if (ident.empty()) {
		return NULL;
	}
	MissileTypeMap::iterator it = MissileTypes.find(ident);
	if (it != MissileTypes.end()) {
		return it->second;
	}
	return NULL;
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

	MissileTypes[ident] = mtype;
	return mtype;
}

/**
**  Constructor
*/
Missile::Missile() :
	Type(NULL), SpriteFrame(0), State(0), AnimWait(0), Wait(0),
	Delay(0), SourceUnit(), TargetUnit(), Damage(0),
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
			missile = new ::MissileHit;
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
**  Fire missile.
**
**  @param unit  Unit that fires the missile.
*/
void FireMissile(CUnit &unit, CUnit *goal, const Vec2i &goalPos)
{
	Vec2i newgoalPos = goalPos;
	// Goal dead?
	if (goal) {
		Assert(!unit.Type->Missile.Missile->AlwaysFire || unit.Type->Missile.Missile->Range);
		if (goal->Destroyed) {
			DebugPrint("destroyed unit\n");
			return;
		}
		if (goal->Removed) {
			return;
		}
		if (goal->CurrentAction() == UnitActionDie) {
			if (unit.Type->Missile.Missile->AlwaysFire) {
				newgoalPos = goal->tilePos;
				goal = NoUnitP;
			} else {
				return;
			}
		}
	}

	// No missile hits immediately!
	if (unit.Type->Missile.Missile->Class == MissileClassNone) {
		// No goal, take target coordinates
		if (!goal) {
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
	CUnit *from = GetFirstContainer(unit);
	const PixelPos startPixelPos = Map.TilePosToMapPixelPos_Center(from->tilePos);

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
		dpos = newgoalPos;
		// FIXME: Can this be too near??
	}

	const PixelPos destPixelPos = Map.TilePosToMapPixelPos_Center(dpos);
	Missile *missile = MakeMissile(*unit.Type->Missile.Missile, startPixelPos, destPixelPos);
	//
	// Damage of missile
	//
	if (goal) {
		missile->TargetUnit = goal;
	}
	missile->SourceUnit = &unit;
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
static void GetMissileMapArea(const Missile &missile, Vec2i &boxMin, Vec2i &boxMax)
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
				this->G->DrawFrameClipTransX(-frame - 1, pos.x, pos.y, int(256 - 2.56 * Transparency));
			} else {
				this->G->DrawFrameClipX(-frame - 1, pos.x, pos.y);
			}
		} else {
			if (this->Transparency > 0) {
				this->G->DrawFrameClipTrans(frame, pos.x, pos.y, int(256 - 2.56 * Transparency));
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
			this->G->DrawFrameClipTrans(frame, pos.x, pos.y, int(256 - 2.56 * Transparency));
		} else {
			this->G->DrawFrameClip(frame, pos.x, pos.y);
		}
	}
}

/**
**  Draw missile.
*/
void Missile::DrawMissile(const CViewport &vp) const
{
	Assert(this->Type);
	CUnit *sunit = this->SourceUnit;
	// FIXME: I should copy SourcePlayer for second level missiles.
	if (sunit && sunit->Player) {
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

static bool MissileDrawLevelCompare(const Missile *const l, const Missile *const r)
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
*/
void FindAndSortMissiles(const CViewport &vp, std::vector<Missile *> &table)
{
	typedef std::vector<Missile *>::const_iterator MissilePtrConstiterator;

	// Loop through global missiles, then through locals.
	for (MissilePtrConstiterator i = GlobalMissiles.begin(); i != GlobalMissiles.end(); ++i) {
		Missile &missile = *(*i);
		if (missile.Delay || missile.Hidden) {
			continue;  // delayed or hidden -> aren't shown
		}
		// Draw only visible missiles
		if (MissileVisibleInViewport(vp, missile)) {
			table.push_back(&missile);
		}
	}

	for (MissilePtrConstiterator i = LocalMissiles.begin(); i != LocalMissiles.end(); ++i) {
		Missile &missile = *(*i);
		if (missile.Delay || missile.Hidden) {
			continue;  // delayed or hidden -> aren't shown
		}
		// Local missile are visible.
		table.push_back(&missile);
	}
	std::sort(table.begin(), table.end(), MissileDrawLevelCompare);
}

/**
**  Change missile heading from x,y.
**
**  @param delta    Delta movement
**
**  @internal We have : SpriteFrame / (2 * (Numdirection - 1)) == DirectionToHeading / 256.
*/
void Missile::MissileNewHeadingFromXY(const PixelPos &delta)
{
	if (this->Type->NumDirections == 1 || (delta.x == 0 && delta.y == 0)) {
		return;
	}

	if (this->SpriteFrame < 0) {
		this->SpriteFrame = -this->SpriteFrame - 1;
	}
	this->SpriteFrame /= this->Type->NumDirections / 2 + 1;
	this->SpriteFrame *= this->Type->NumDirections / 2 + 1;

	const int nextdir = 256 / this->Type->NumDirections;
	Assert(nextdir != 0);
	const int dir = ((DirectionToHeading(delta) + nextdir / 2) & 0xFF) / nextdir;
	if (dir <= LookingS / nextdir) { // north->east->south
		this->SpriteFrame += dir;
	} else {
		this->SpriteFrame += 256 / nextdir - dir;
		this->SpriteFrame = -this->SpriteFrame - 1;
	}
}

/**
**  Init the move.
**
**  @param missile  missile to initialise for movement.
**
**  @return         true if goal is reached, false else.
*/
bool MissileInitMove(Missile &missile)
{
	const PixelPos heading = missile.destination - missile.position;

	missile.MissileNewHeadingFromXY(heading);
	if (!(missile.State & 1)) {
		missile.CurrentStep = 0;
		missile.TotalStep = 0;
		if (heading.x == 0 && heading.y == 0) {
			return true;
		}
		// initialize
		missile.TotalStep = MapDistance(missile.source, missile.destination);
		missile.State++;
		return false;
	}
	Assert(missile.TotalStep != 0);
	missile.CurrentStep += missile.Type->Speed;
	if (missile.CurrentStep >= missile.TotalStep) {
		missile.position = missile.destination;
		return true;
	}
	return false;
}

/**
**  Handle point to point missile.
**
**  @param missile  Missile pointer.
**
**  @return         true if goal is reached, false else.
*/
bool PointToPointMissile(Missile &missile)
{
	if (MissileInitMove(missile) == true) {
		return true;
	}

	Assert(missile.Type != NULL);
	Assert(missile.TotalStep != 0);

	const PixelPos diff = (missile.destination - missile.source);
	missile.position = missile.source + diff * missile.CurrentStep / missile.TotalStep;

	if (missile.Type->Smoke.Missile && missile.CurrentStep) {
		const PixelPos position = missile.position + missile.Type->size / 2;
		MakeMissile(*missile.Type->Smoke.Missile, position, position);
	}
	return false;
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
	if (!missile.Type->CanHitOwner && missile.SourceUnit == &goal) {
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
*/
void Missile::MissileHit()
{
	const MissileType &mtype = *this->Type;

	if (mtype.ImpactSound.Sound) {
		PlayMissileSound(this, mtype.ImpactSound.Sound);
	}
	const PixelPos pixelPos = this->position + this->Type->size / 2;

	//
	// The impact generates a new missile.
	//
	if (mtype.Impact.Missile) {
		MakeMissile(*mtype.Impact.Missile, pixelPos, pixelPos);
	}
	if (mtype.ImpactParticle) {
		mtype.ImpactParticle->pushPreamble();
		mtype.ImpactParticle->pushInteger(pixelPos.x);
		mtype.ImpactParticle->pushInteger(pixelPos.y);
		mtype.ImpactParticle->run();
	}

	if (!this->SourceUnit) {  // no owner - green-cross ...
		return;
	}

	const Vec2i pos = Map.MapPixelPosToTilePos(pixelPos);

	if (!Map.Info.IsPointOnMap(pos)) {
		// FIXME: this should handled by caller?
		DebugPrint("Missile gone outside of map!\n");
		return;  // outside the map.
	}

	//
	// Choose correct goal.
	//
	if (!mtype.Range) {
		if (this->TargetUnit && (mtype.FriendlyFire == false
								 || this->TargetUnit->Player->Index != this->SourceUnit->Player->Index)) {
			//
			// Missiles without range only hits the goal always.
			//
			CUnit &goal = *this->TargetUnit;
			if (goal.Destroyed) {
				this->TargetUnit = NoUnitP;
				return;
			}
			MissileHitsGoal(*this, goal, 1);
			return;
		}
		MissileHitsWall(*this, pos, 1);
		return;
	}

	{
		//
		// Hits all units in range.
		//
		const Vec2i range = {mtype.Range - 1, mtype.Range - 1};
		std::vector<CUnit *> table;
		Map.Select(pos - range, pos + range, table);
		Assert(this->SourceUnit != NULL);
		for (size_t i = 0; i != table.size(); ++i) {
			CUnit &goal = *table[i];
			//
			// Can the unit attack the this unit-type?
			// NOTE: perhaps this should be come a property of the missile.
			// Also check CorrectSphashDamage so land explosions can't hit the air units
			//
			if (CanTarget(this->SourceUnit->Type, goal.Type)
				&& (mtype.FriendlyFire == false || goal.Player->Index != this->SourceUnit->Player->Index)) {
				bool shouldHit = true;

				if (mtype.CorrectSphashDamage == true) {
					if (this->SourceUnit->CurrentAction() == UnitActionAttackGround) {
						if (goal.Type->UnitType != this->SourceUnit->Type->UnitType) {
							shouldHit = false;
						}
					} else {
						if (this->TargetUnit == NULL || goal.Type->UnitType != this->TargetUnit->Type->UnitType) {
							shouldHit = false;
						}
					}
				}
				if (shouldHit) {
					int splash = goal.MapDistanceTo(pos);

					if (splash) {
						splash *= mtype.SplashFactor;
					} else {
						splash = 1;
					}
					MissileHitsGoal(*this, goal, splash);
				}
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
				MissileHitsWall(*this, posIt, d);
			}
		}
	}
}

/**
**  Pass to the next frame for animation.
**
**  @param sign           1 for next frame, -1 for previous frame.
**  @param longAnimation  1 if Frame is conditioned by covered distance, 0 else.
**
**  @return               true if animation is finished, false else.
*/
bool Missile::NextMissileFrame(char sign, char longAnimation)
{
	int neg = 0; // True for mirroring sprite.
	bool animationIsFinished = false;
	int numDirections = this->Type->NumDirections / 2 + 1;
	if (this->SpriteFrame < 0) {
		neg = 1;
		this->SpriteFrame = -this->SpriteFrame - 1;
	}
	if (longAnimation) {
		// Total distance to cover.
		const int totalx = MapDistance(this->destination, this->source);
		// Covered distance.
		const int dx = MapDistance(this->position, this->source);
		// Total number of frame (for one direction).
		const int totalf = this->Type->SpriteFrames / numDirections;
		// Current frame (for one direction).
		const int df = this->SpriteFrame / numDirections;

		if ((sign == 1 && dx * totalf <= df * totalx)
			|| (sign == -1 && dx * totalf > df * totalx)) {
			return animationIsFinished;
		}
	}
	this->SpriteFrame += sign * numDirections;
	if (sign > 0) {
		if (this->SpriteFrame >= this->Type->SpriteFrames) {
			this->SpriteFrame -= this->Type->SpriteFrames;
			animationIsFinished = true;
		}
	} else {
		if (this->SpriteFrame < 0) {
			this->SpriteFrame += this->Type->SpriteFrames;
			animationIsFinished = true;
		}
	}
	if (neg) {
		this->SpriteFrame = -this->SpriteFrame - 1;
	}
	return animationIsFinished;
}

/**
**  Pass the next frame of the animation.
**  This animation goes from start to finish ONCE on the way
*/
void Missile::NextMissileFrameCycle()
{
	int neg = 0;

	if (this->SpriteFrame < 0) {
		neg = 1;
		this->SpriteFrame = -this->SpriteFrame - 1;
	}
	const int totalx = abs(this->destination.x - this->source.x);
	const int dx = abs(this->position.x - this->source.x);
	int f = this->Type->SpriteFrames / (this->Type->NumDirections / 2 + 1);
	f = 2 * f - 1;
	for (int i = 1, j = 1; i <= f; ++i) {
		if (dx * f / i < totalx) {
			if ((i - 1) * 2 < f) {
				j = i - 1;
			} else {
				j = f - i;
			}
			this->SpriteFrame = this->SpriteFrame % (this->Type->NumDirections / 2 + 1) +
								j * (this->Type->NumDirections / 2 + 1);
			break;
		}
	}
	if (neg) {
		this->SpriteFrame = -this->SpriteFrame - 1;
	}
}

/**
**  Handle all missile actions of global/local missiles.
**
**  @param missiles  Table of missiles.
*/
static void MissilesActionLoop(std::vector<Missile *> &missiles)
{
	for (size_t i = 0; i != missiles.size(); /* empty */) {
		Missile &missile = *missiles[i];

		if (missile.Delay) {
			missile.Delay--;
			++i;
			continue;  // delay start of missile
		}
		if (missile.TTL > 0) {
			missile.TTL--;  // overall time to live if specified
		}
		if (missile.TTL == 0) {
			delete &missile;
			missiles.erase(missiles.begin() + i);
			continue;
		}
		Assert(missile.Wait);
		if (--missile.Wait) {  // wait until time is over
			++i;
			continue;
		}
		missile.Action(); // may create other missiles, and so modifies the array
		if (missile.TTL == 0) {
			delete &missile;
			missiles.erase(missiles.begin() + i);
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
	const Vec2i tilePos = Map.MapPixelPosToTilePos(pixelPos);

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
	if (this->SourceUnit != NULL) {
		file.printf(" \"source\", \"%s\",", UnitReference(this->SourceUnit).c_str());
	}
	if (this->TargetUnit != NULL) {
		file.printf(" \"target\", \"%s\",", UnitReference(this->TargetUnit).c_str());
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
	file.printf("\n--- -----------------------------------------\n");
	file.printf("--- MODULE: missiles\n\n");

	std::vector<Missile *>::const_iterator i;

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
	this->FiredSound.MapSound();
	this->ImpactSound.MapSound();
	this->Impact.MapMissile();
	this->Smoke.MapMissile();
}

/**
**  Initialize missile-types.
*/
void InitMissileTypes()
{
	for (MissileTypeMap::iterator it = MissileTypes.begin(); it != MissileTypes.end(); ++it) {
		(*it).second->Init();
	}
}

/**
**  Constructor.
*/
MissileType::MissileType(const std::string &ident) :
	Ident(ident), Transparency(0),
	DrawLevel(0), SpriteFrames(0), NumDirections(0),
	CorrectSphashDamage(false), Flip(false), CanHitOwner(false), FriendlyFire(false),
	AlwaysFire(false), Class(), NumBounces(0), StartDelay(0), Sleep(0), Speed(0),
	Range(0), SplashFactor(0), ImpactParticle(NULL), G(NULL)
{
	size.x = 0;
	size.y = 0;
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
	for (MissileTypeMap::iterator it = MissileTypes.begin(); it != MissileTypes.end(); ++it) {
		delete it->second;
	}
	MissileTypes.clear();
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
	std::vector<Missile *>::const_iterator i;

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

//@}
