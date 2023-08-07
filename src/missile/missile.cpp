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

#include <math.h>

#include "stratagus.h"

#include "missile.h"

#include "action/action_spellcast.h"
#include "actions.h"
#include "animation.h"
#include "font.h"
#include "iolib.h"
#include "luacallback.h"
#include "map.h"
#include "player.h"
#include "sound.h"
#include "spells.h"
#include "trigger.h"
#include "ui.h"
#include "unit.h"
#include "unit_find.h"
#include "unitsound.h"
#include "unittype.h"
#include "video.h"

/*----------------------------------------------------------------------------
--  Declarations
----------------------------------------------------------------------------*/

unsigned int Missile::Count = 0;

static std::vector<Missile *> GlobalMissiles;    /// all global missiles on map
static std::vector<Missile *> LocalMissiles;     /// all local missiles on map

/// lookup table for missile names
using MissileTypeMap = std::map<std::string, MissileType *>;
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
	if (this->G && !this->G->IsLoaded(this->Flip)) {
		this->G->Load();
		if (this->Flip) {
			this->G->Flip();
		}

		// Correct the number of frames in graphic
		DebugPrint("%s - %d>=%d\n" _C_ this->G->File.c_str() _C_ this->G->NumFrames _C_ this->SpriteFrames);
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
		return nullptr;
	}
	MissileTypeMap::iterator it = MissileTypes.find(ident);
	if (it != MissileTypes.end()) {
		return it->second;
	}
	return nullptr;
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
	Type(nullptr), SpriteFrame(0), State(0), AnimWait(0), Wait(0),
	Delay(0), SourceUnit(), TargetUnit(), Damage(0),
	TTL(-1), Hidden(0), DestroyMissile(0),
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
	Missile *missile = nullptr;

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
		case MissileClassContinious :
			missile = new MissileContinious;
			break;
		case MissileClassStraightFly :
			missile = new MissileStraightFly;
			break;
	}
	const PixelPos halfSize = mtype.size / 2;
	missile->position = startPos - halfSize;
	missile->destination = destPos - halfSize;
	missile->source = missile->position;
	missile->Type = &mtype;
	missile->Wait = mtype.Sleep;
	missile->Delay = mtype.StartDelay;
	missile->TTL = mtype.TTL;
	if (mtype.FiredSound.Sound) {
		PlayMissileSound(*missile, mtype.FiredSound.Sound);
	}

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
**  @param formula   Formula used to calculate damage.
**
**  @return          damage produces on goal.
*/
int CalculateDamage(const CUnit &attacker, const CUnit &goal, const NumberDesc *formula)
{
	if (!formula) { // Use old method.
		return CalculateDamageStats(*attacker.Stats, *goal.Stats,
									attacker.Variable[BLOODLUST_INDEX].Value);
	}
	Assert(formula);

	UpdateUnitVariables(const_cast<CUnit &>(attacker));
	UpdateUnitVariables(const_cast<CUnit &>(goal));
	TriggerData.Attacker = const_cast<CUnit *>(&attacker);
	TriggerData.Defender = const_cast<CUnit *>(&goal);
	const int res = EvalNumber(formula);
	TriggerData.Attacker = nullptr;
	TriggerData.Defender = nullptr;
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
				goal = nullptr;
			} else {
				return;
			}
		}
	}

	// No missile hits immediately!
	if (
		unit.Type->Missile.Missile->Class == MissileClassNone
		|| (unit.Type->Animations && unit.Type->Animations->Attack && unit.Type->Animations->RangedAttack && !unit.IsAttackRanged(goal, goalPos)) // treat melee attacks from units that have both attack and ranged attack animations as having missile class none
	) {
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
		HitUnit(&unit, *goal, CalculateDamage(unit, *goal, Damage));
		return;
	}

	// If Firing from inside a Bunker
	CUnit *from = GetFirstContainer(unit);
	const int dir = ((unit.Direction + NextDirection / 2) & 0xFF) / NextDirection;
	const PixelPos startPixelPos = from->GetMapPixelPosCenter()
								   + unit.Type->MissileOffsets[dir][0];

	Vec2i dpos;
	bool divisionOffset = false;
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
		if (unit.Container) {
			NearestOfUnit(*goal, GetFirstContainer(unit)->tilePos, &dpos);
		} else {
			if (goal->Type->TileWidth % 2 == 0)
				divisionOffset = true;
			dpos = goal->tilePos + goal->Type->GetHalfTileSize();
		}
	} else {
		dpos = newgoalPos;
		// FIXME: Can this be too near??
	}

	PixelPos destPixelPos = Map.TilePosToMapPixelPos_Center(dpos);
	if (divisionOffset)
		destPixelPos -= PixelTileSize/2;
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
	PixelSize missileSize(missile.Type->Width(), missile.Type->Height());
	PixelDiff margin(PixelTileSize.x - 1, PixelTileSize.y - 1);
	boxMin = Map.MapPixelPosToTilePos(missile.position);
	boxMax = Map.MapPixelPosToTilePos(missile.position + missileSize + margin);
	Map.Clamp(boxMin);
	Map.Clamp(boxMax);
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
			if (ReplayRevealMap || Map.Field(pos)->playerInfo.IsTeamVisible(*ThisPlayer)) {
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
	if (!this->G->IsLoaded(this->Flip)) {
		((MissileType*)this)->LoadMissileSprite();
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
		if (!this->Type->G->IsLoaded(this->Type->Flip)) {
			((MissileType*)this->Type)->LoadMissileSprite();
		}
#endif
	}
	const PixelPos screenPixelPos = vp.MapToScreenPixelPos(this->position);

	switch (this->Type->Class) {
		case MissileClassHit:
			CLabel(GetGameFont()).DrawClip(screenPixelPos.x, screenPixelPos.y, this->Damage);
			break;
		default:
			if (Type->G) {
				this->Type->DrawMissileType(this->SpriteFrame, screenPixelPos);
			}
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
	// Loop through global missiles, then through locals.
	for (auto* missilePtr : GlobalMissiles) {
		Missile &missile = *missilePtr;
		if (missile.Delay || missile.Hidden) {
			continue;  // delayed or hidden -> aren't shown
		}
		// Draw only visible missiles
		if (MissileVisibleInViewport(vp, missile)) {
			table.push_back(&missile);
		}
	}

	for (auto* missilePtr : LocalMissiles) {
		Missile &missile = *missilePtr;
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
bool MissileInitMove(Missile &missile, bool pointToPoint)
{
	const PixelPos source = pointToPoint ? missile.source : missile.position;
	const PixelPos heading = missile.destination - source;

	missile.MissileNewHeadingFromXY(heading);
	if (!(missile.State & 1)) {
		missile.CurrentStep = 0;
		missile.TotalStep = 0;
		if (heading.x == 0 && heading.y == 0) {
			return true;
		}
		// initialize
		missile.TotalStep = Distance(missile.source, missile.destination);
		missile.State++;
		return false;
	}
	Assert(missile.TotalStep != 0);
	missile.CurrentStep += missile.Type->Speed;
	if (missile.CurrentStep >= missile.TotalStep) {
		missile.CurrentStep = missile.TotalStep;
		return true;
	}
	return false;
}

void MissileHandlePierce(Missile &missile, const Vec2i &pos)
{
	if (Map.Info.IsPointOnMap(pos) == false) {
		return;
	}
	std::vector<CUnit *> units;
	Select(pos, pos, units);
	for (std::vector<CUnit *>::iterator it = units.begin(); it != units.end(); ++it) {
		CUnit &unit = **it;

		if (unit.IsAliveOnMap()
			&& (missile.Type->FriendlyFire == false || unit.IsEnemy(*missile.SourceUnit->Player))) {
			missile.MissileHit(&unit);
		}
	}
}

bool MissileHandleBlocking(Missile &missile, const PixelPos &position)
{
	const MissileType &mtype = *missile.Type;
	if (missile.SourceUnit) {
		bool shouldHit = false;
		if (missile.TargetUnit && missile.SourceUnit->Type->UnitType == missile.TargetUnit->Type->UnitType) {
			shouldHit = true;
		}
		if (mtype.Range && mtype.CorrectSphashDamage) {
			shouldHit = true;
		}
		if (shouldHit) {
			// search for blocking units
			std::vector<CUnit *> blockingUnits;
			const Vec2i missilePos = Map.MapPixelPosToTilePos(position);
			Select(missilePos, missilePos, blockingUnits);
			for (std::vector<CUnit *>::iterator it = blockingUnits.begin();	it != blockingUnits.end(); ++it) {
				CUnit &unit = **it;
				// If land unit shoots at land unit, missile can be blocked by Wall units
				if (!missile.Type->IgnoreWalls && missile.SourceUnit->Type->UnitType == UnitTypeLand) {
					if (!missile.TargetUnit || missile.TargetUnit->Type->UnitType == UnitTypeLand) {
						if (&unit != missile.SourceUnit && unit.Type->BoolFlag[WALL_INDEX].value
							&& unit.Player != missile.SourceUnit->Player && unit.IsAllied(*missile.SourceUnit) == false) {
							if (missile.TargetUnit) {
								missile.TargetUnit = &unit;
								if (unit.Type->TileWidth == 1 || unit.Type->TileHeight == 1) {
									missile.position = Map.TilePosToMapPixelPos_TopLeft(unit.tilePos);
								}
							} else {
								missile.position = position;
							}
							missile.DestroyMissile = 1;
							return true;
						}
					}
				}
				// missile can kill any unit on it's way
				if (missile.Type->KillFirstUnit && &unit != missile.SourceUnit) {
					// can't kill non-solid or dead units
					if (unit.IsAliveOnMap() == false || unit.Type->BoolFlag[NONSOLID_INDEX].value) {
						continue;
					}
					if (missile.Type->FriendlyFire == false || unit.IsEnemy(*missile.SourceUnit->Player)) {
						missile.TargetUnit = &unit;
						if (unit.Type->TileWidth == 1 || unit.Type->TileHeight == 1) {
							missile.position = Map.TilePosToMapPixelPos_TopLeft(unit.tilePos);
						}
						missile.DestroyMissile = 1;
						return true;
					}
				}
			}
		}
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
	MissileInitMove(missile, true);
	if (missile.TotalStep == 0) {
		return true;
	}
	Assert(missile.Type != nullptr);
	Assert(missile.TotalStep != 0);

	const PixelPos diff = (missile.destination - missile.source);
	const PixelPrecise sign(diff.x >= 0 ? 1.0 : -1.0, diff.y >= 0 ? 1.0 : -1.0); // Remember sign to move into correct direction
	const PixelPrecise oldPos((double)missile.position.x, (double)missile.position.y); // Remember old position
	PixelPrecise pos(oldPos);
	missile.position = missile.source + diff * missile.CurrentStep / missile.TotalStep;

	for (; pos.x * sign.x <= missile.position.x * sign.x
		 && pos.y * sign.y <= missile.position.y * sign.y;
		 pos.x += (double)diff.x * missile.Type->SmokePrecision / missile.TotalStep,
		 pos.y += (double)diff.y * missile.Type->SmokePrecision / missile.TotalStep) {
		const PixelPos position((int)pos.x + missile.Type->size.x / 2,
								(int)pos.y + missile.Type->size.y / 2);

		if (missile.Type->Smoke.Missile && (missile.CurrentStep || missile.State > 1)) {
			Missile *smoke = MakeMissile(*missile.Type->Smoke.Missile, position, position);
			if (smoke && smoke->Type->NumDirections > 1) {
				smoke->MissileNewHeadingFromXY(diff);
			}
		}

		if (missile.Type->SmokeParticle && (missile.CurrentStep || missile.State > 1)) {
			missile.Type->SmokeParticle->pushPreamble();
			missile.Type->SmokeParticle->pushInteger(position.x);
			missile.Type->SmokeParticle->pushInteger(position.y);
			missile.Type->SmokeParticle->run();
		}

		if (missile.Type->Pierce) {
			const PixelPos posInt((int)pos.x, (int)pos.y);
			MissileHandlePierce(missile, Map.MapPixelPosToTilePos(posInt));
		}
	}

	// Handle wall blocking and kill first enemy
	for (pos = oldPos; pos.x * sign.x <= missile.position.x * sign.x
		 && pos.y * sign.y <= missile.position.y * sign.y;
		 pos.x += (double)diff.x / missile.TotalStep,
		 pos.y += (double)diff.y / missile.TotalStep) {
		const PixelPos position((int)pos.x + missile.Type->size.x / 2,
								(int)pos.y + missile.Type->size.y / 2);
		const Vec2i tilePos(Map.MapPixelPosToTilePos(position));

		if (Map.Info.IsPointOnMap(tilePos) && MissileHandleBlocking(missile, position)) {
			return true;
		}
		if (missile.Type->MissileStopFlags) {
			if (!Map.Info.IsPointOnMap(tilePos)) { // gone outside
				missile.TTL = 0;
				return false;
			}
			const CMapField &mf = *Map.Field(tilePos);
			if (missile.Type->MissileStopFlags & mf.Flags) { // incompatible terrain
				missile.position = position;
				missile.MissileHit();
				missile.TTL = 0;
				return false;
			}
		}
	}

	if (missile.CurrentStep == missile.TotalStep) {
		missile.position = missile.destination;
		return true;
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
		int damage;

		if (missile.Type->Damage) {   // custom formula
			Assert(missile.SourceUnit != nullptr);
			damage = CalculateDamage(*missile.SourceUnit, goal, missile.Type->Damage) / splash;
		} else if (missile.Damage) {  // direct damage, spells mostly
			damage = missile.Damage / splash;
		} else {
			Assert(missile.SourceUnit != nullptr);
			damage = CalculateDamage(*missile.SourceUnit, goal, Damage) / splash;
		}
		if (missile.Type->Pierce) {  // Handle pierce factor
			for (size_t i = 0; i < (missile.PiercedUnits.size() - 1); ++i) {
				damage *= (double)missile.Type->ReduceFactor / 100;
			}
		}

		HitUnit(missile.SourceUnit, goal, damage, &missile);
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

	Assert(missile.SourceUnit != nullptr);
	if (Map.HumanWallOnMap(tilePos)) {
		stats = UnitTypeHumanWall->Stats;
	} else {
		Assert(Map.OrcWallOnMap(tilePos));
		stats = UnitTypeOrcWall->Stats;
	}
	Map.HitWall(tilePos, CalculateDamageStats(*missile.SourceUnit->Stats, *stats, 0) / splash);
}

/**
**  Check if missile has already pierced that unit
**
**  @param missile  Current missile.
**  @param unit     Target unit.
**
**  @return         true if goal is pierced, false else.
*/

static bool IsPiercedUnit(const Missile &missile, const CUnit &unit)
{
	for (std::vector<CUnit *>::const_iterator it = missile.PiercedUnits.begin();
		 it != missile.PiercedUnits.end(); ++it) {
		CUnit &punit = **it;
		if (UnitNumber(unit) == UnitNumber(punit)) {
			return true;
		}
	}
	return false;
}

/**
**  Work for missile hit.
*/
void Missile::MissileHit(CUnit *unit)
{
	const MissileType &mtype = *this->Type;

	if (mtype.ImpactSound.Sound) {
		PlayMissileSound(*this, mtype.ImpactSound.Sound);
	}
	const PixelPos pixelPos = this->position + this->Type->size / 2;

	//
	// The impact generates a new missile.
	//
	if (mtype.Impact.empty() == false) {
		for (std::vector<MissileConfig *>::const_iterator it = mtype.Impact.begin(); it != mtype.Impact.end(); ++it) {
			const MissileConfig &mc = **it;
			Missile *impact = MakeMissile(*mc.Missile, pixelPos, pixelPos);
			if (impact && impact->Type->Damage) {
				impact->SourceUnit = this->SourceUnit;
			}
		}
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
	if (unit) {
		if (unit->Destroyed) {
			return;
		}
		if (mtype.Pierce && mtype.PierceOnce) {
			if (IsPiercedUnit(*this, *unit)) {
				return;
			} else {
				PiercedUnits.insert(this->PiercedUnits.begin(), unit);
			}
		}
		MissileHitsGoal(*this, *unit, 1);
		return;
	}
	if (!mtype.Range) {
		if (this->TargetUnit && (mtype.FriendlyFire == false
								 || this->TargetUnit->Player->Index != this->SourceUnit->Player->Index)) {
			//
			// Missiles without range only hits the goal always.
			//
			CUnit &goal = *this->TargetUnit;
			if (mtype.Pierce && mtype.PierceOnce) {
				if (IsPiercedUnit(*this, goal)) {
					return;
				} else {
					PiercedUnits.insert(this->PiercedUnits.begin(), &goal);
				}
			}
			if (goal.Destroyed) {
				this->TargetUnit = nullptr;
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
		const Vec2i range(mtype.Range - 1, mtype.Range - 1);
		std::vector<CUnit *> table;
		Select(pos - range, pos + range, table);
		Assert(this->SourceUnit != nullptr);
		for (size_t i = 0; i != table.size(); ++i) {
			CUnit &goal = *table[i];
			//
			// Can the unit attack this unit-type?
			// NOTE: perhaps this should be come a property of the missile.
			// Also check CorrectSphashDamage so land explosions can't hit the air units
			//
			if (CanTarget(*this->SourceUnit->Type, *goal.Type)
				&& (mtype.FriendlyFire == false || goal.Player->Index != this->SourceUnit->Player->Index)) {
				bool shouldHit = true;

				if (mtype.Pierce && mtype.PierceOnce) {
					if (IsPiercedUnit(*this, goal)) {
						shouldHit = false;
					} else {
						PiercedUnits.insert(this->PiercedUnits.begin(), &goal);
					}
				}

				if (mtype.CorrectSphashDamage == true) {
					bool isPosition = false;
					if (this->TargetUnit == nullptr) {
						if (this->SourceUnit->CurrentAction() == UnitActionSpellCast) {
							const COrder_SpellCast &order = *static_cast<COrder_SpellCast *>(this->SourceUnit->CurrentOrder());
							if (order.GetSpell().Target == TargetPosition) {
								isPosition = true;
							}
						} else {
							isPosition = true;
						}
					}
					if (isPosition || this->SourceUnit->CurrentAction() == UnitActionAttackGround) {
						if (goal.Type->UnitType != this->SourceUnit->Type->UnitType) {
							shouldHit = false;
						}
					} else {
						if (this->TargetUnit == nullptr || goal.Type->UnitType != this->TargetUnit->Type->UnitType) {
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
	const Vec2i offset(mtype.Range, mtype.Range);
	const Vec2i posmin = pos - offset;
	for (int i = mtype.Range * 2; --i;) {
		for (int j = mtype.Range * 2; --j;) {
			const Vec2i posIt(posmin.x + i, posmin.y + j);

			if (Map.Info.IsPointOnMap(posIt)) {
				int d = Distance(pos, posIt);
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
		const int totalx = Distance(this->destination, this->source);
		// Covered distance.
		const int dx = Distance(this->position, this->source);
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
**  Calculate distance from view-point to missile.
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
	return nullptr;
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
	if (this->SourceUnit != nullptr) {
		file.printf(" \"source\", \"%s\",", UnitReference(this->SourceUnit).c_str());
	}
	if (this->TargetUnit != nullptr) {
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
	for (std::vector<MissileConfig *>::iterator it = this->Impact.begin(); it != this->Impact.end(); ++it) {
		MissileConfig &mc = **it;

		mc.MapMissile();
	}
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
	Ident(ident), Transparency(0), DrawLevel(0),
	SpriteFrames(0), NumDirections(0), ChangeVariable(-1), ChangeAmount(0), ChangeMax(false),
	CorrectSphashDamage(false), Flip(false), CanHitOwner(false), FriendlyFire(false),
	AlwaysFire(false), Pierce(false), PierceOnce(false), IgnoreWalls(true), KillFirstUnit(false),
	Class(), NumBounces(0),	ParabolCoefficient(2048), StartDelay(0),
	Sleep(0), Speed(0), BlizzardSpeed(0), TTL(-1), ReduceFactor(100), SmokePrecision(0),
	MissileStopFlags(0), Damage(nullptr), Range(0), SplashFactor(0),
	ImpactParticle(nullptr), SmokeParticle(nullptr), OnImpact(nullptr),
	G(nullptr)
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
	Impact.clear();
	delete ImpactParticle;
	delete SmokeParticle;
	delete OnImpact;
	FreeNumberDesc(this->Damage);
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
**  Missile destructior.
*/
Missile::~Missile()
{
	PiercedUnits.clear();
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

void FreeBurningBuildingFrames()
{
	for (std::vector<BurningBuildingFrame *>::iterator i = BurningBuildingFrames.begin();
		 i != BurningBuildingFrames.end(); ++i) {
		delete *i;
	}
	BurningBuildingFrames.clear();
}

//@}
