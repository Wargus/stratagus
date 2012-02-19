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
/**@name missile.h - The missile headerfile. */
//
//      (c) Copyright 1998-2005 by Lutz Sammer
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

#ifndef __MISSILE_H__
#define __MISSILE_H__

//@{

/*----------------------------------------------------------------------------
--  Documentation
----------------------------------------------------------------------------*/

/**
**  @class MissileType missile.h
**
**  \#include "missile.h"
**
**  This structure defines the base type information of all missiles. It
**  contains all information that all missiles of the same type shares.
**  The fields are filled from the configuration files.
**
**
**  The missile-type structure members:
**
**  MissileType::Ident
**
**    Unique identifier of the missile-type, used to reference it in
**    config files and during startup.
**    @note Don't use this member in game, use instead the pointer
**    to this structure. See MissileTypeByIdent().
**
**  MissileType::DrawLevel
**
**    The Level/Order to draw the missile in, usually 0-255
**
**  MissileType::Width MissileType::Height
**
**    Size (width and height) of a frame in the image. All sprite
**    frames of the missile-type must have the same size.
**
**  MissileType::SpriteFrames
**
**    Total number of sprite frames in the graphic image.
**    @note If the image is smaller than the number of directions,
**    width/height and/or framecount suggest the engine crashes.
**    @note There is currently a limit of 127 sprite frames, which
**    can be lifted if needed.
**
**  MissileType::NumDirections
**
**    Number of directions missile can face.
**
**  MissileType::Transparency
**
**    Set a missile transparency.
**
**  MissileType::FiredSound
**
**    Sound of the missile, if fired. @note currently not used.
**
**  MissileType::ImpactSound
**
**    Impact sound for this missile.
**
**  MissileType::CanHitOwner
**
**    Can hit the unit that have fired the missile.
**    @note Currently no missile can hurt its owner.
**
**  MissileType::FriendlyFire
**
**    Can't hit the units of the same player, that has the
**    missile fired.
**
**  MissileType::Class
**
**    Class of the missile-type, defines the basic effects of the
**    missile. Look at the different class identifiers for more
**    informations (::MissileClassNone, ...).
**
**  MissileType::NumBounces
**
**    This is the number of bounces, and it is only valid with
**    MissileClassBounce. The missile will hit this many times in
**    a row.
**
**  MissileType::StartDelay
**
**    Delay in game cycles after the missile generation, until the
**    missile animation and effects starts. Delay denotes the number
**    of display cycles to skip before drawing the first sprite frame
**    and only happens once at start.
**
**  MissileType::Sleep
**
**    This are the number of game cycles to wait for the next
**    animation or the sleeping between the animation steps.
**    All animations steps use the same delay.  0 is the
**    fastest and 255 the slowest animation.
**    @note Perhaps we should later allow animation scripts for
**    more complex animations.
**
**  MissileType::Speed
**
**    The speed how fast the missile moves. 0 the missile didn't
**    move, 1 is the slowest speed and 32 s the fastest supported
**    speed. This is how many pixels the missiles moves with each
**    animation step.  The real use of this member depends on the
**    MissileType::Class
**    @note This is currently only used by the point-to-point
**    missiles (::MissileClassPointToPoint, ...).  Perhaps we should
**    later allow animation scripts for more complex animations.
**
**  MissileType::Range
**
**    Determines the range in which a projectile will deal its damage.
**    A range of 0 will mean that the damage will be limited to the
**    targetted unit only.  So if you shot a missile at a unit, it
**    would only damage that unit.  A value of 1 only affects the
**    field where the missile hits.  A value of 2  would mean that
**    the damage for that particular missile would be dealt for a range
**    of 1 around the impact spot. All fields that aren't the center
**    get only 1/SpashFactor of the damage. Fields 2 away get
**    1/(SplashFactor*2), and following...
**
**  MissileType::SplashFactor
**
**    Determines The Splash damage divisor, see Range
**
**  MissileType::ImpactName
**
**    The name of the next (other) missile to generate, when this
**    missile reached its end point or its life time is over.  So it
**    can be used to generate a chain of missiles.
**    @note Used and should only be used during configuration and
**    startup.
**
**  MissileType::ImpactMissile
**
**    Pointer to the impact missile-type. Used during run time.
**
**  MissileType::SmokeName
**
**    The name of the next (other) missile to generate a trailing smoke.  So it
**    can be used to generate a chain of missiles.
**    @note Used and should only be used during configuration and
**    startup.
**
**  MissileType::SmokeMissile
**
**    Pointer to the smoke missile-type. Used during run time.
**
**  MissileType::Sprite
**
**    Missile sprite image loaded from MissileType::File
**
**  MissileType::G
**
**    File containing the image (sprite) graphics of the missile.
**    The file can contain multiple sprite frames.  The sprite frames
**    for the different directions are placed in the row.
**    The different animations steps are placed in the column. But
**    the correct order depends on MissileType::Class. Missiles like fire
**    have no directions, missiles like arrows have a direction.
*/

/**
**  @class Missile missile.h
**
**  \#include "missile.h"
**
**  This structure contains all information about a missile in game.
**  A missile could have different effects, based on its missile-type.
**  Missiles could be saved and stored with CCL. See (missile).
**  Currently only a tile, a unit or a missile could be placed on the map.
**
**
**  The missile structure members:
**
**  Missile::position
**
**    Missile current map position in pixels.  To convert a map tile
**    position to pixel position use: (mapx * ::PixelTileSize.x + ::PixelTileSize.x / 2)
**    and (mapy * ::PixelTileSize.y + ::PixelTileSize.y / 2)
**    @note ::PixelTileSize.x % 2 == 0 && ::PixelTileSize.y % 2 == 0
**
**  Missile::source
**
**    Missile original map position in pixels.  To convert a map tile
**    position to pixel position use: (mapx*::PixelTileSize.x+::PixelTileSize.x/2)
**    and (mapy*::PixelTileSize.y+::PixelTileSize.y/2)
**    @note ::PixelTileSize.x%2==0 && ::PixelTileSize.y%2==0 and ::PixelTileSize.x,
**    ::PixelTileSize.y are currently fixed 32 pixels.
**
**  Missile::destination
**
**    Missile destination on the map in pixels.  If
**    Missile::X==Missile::DX and Missile::Y==Missile::DY the missile
**    stays at its position.  But the movement also depends on
**    MissileType::Class.
**
**  Missile::Type
**
**    ::MissileType pointer of the missile, contains the shared
**    informations of all missiles of the same type.
**
**  Missile::SpriteFrame
**
**    Current sprite frame of the missile.  The range is from 0
**    to MissileType::SpriteFrames-1.  The topmost bit (128) is
**    used as flag to mirror the sprites in X direction.
**    Animation scripts aren't currently supported for missiles,
**    everything is handled by MissileType::Class
**    @note If wanted, we can add animation scripts support to the
**    engine.
**
**  Missile::State
**
**    Current state of the missile.  Used for a simple state machine.
**
**  Missile::AnimWait
**
**    Animation wait. Used internally by missile actions, to run the
**    animation in parallel with the rest.
**
**  Missile::Wait
**
**    Wait this number of game cycles until the next state or
**    animation of this missile is handled. This counts down from
**    MissileType::Sleep to 0.
**
**  Missile::Delay
**
**    Number of game cycles the missile isn't shown on the map.
**    This counts down from MissileType::StartDelay to 0, before this
**    happens the missile isn't shown and has no effects.
**    @note This can also be used by MissileType::Class
**    for temporary removement of the missile.
**
**  Missile::SourceUnit
**
**    The owner of the missile. Normally the one who fired the
**    missile.  Used to check units, to prevent hitting the owner
**    when field MissileType::CanHitOwner==true. Also used for kill
**    and experience points.
**
**  Missile::TargetUnit
**
**    The target of the missile.  Normally the unit which should be
**    hit by the missile.
**
**  Missile::Damage
**
**    Damage done by missile. See also MissileType::Range, which
**    denoted the 100% damage in center.
**
**  Missile::TTL
**
**    Time to live in game cycles of the missile, if it reaches zero
**    the missile is automatic removed from the map. If -1 the
**    missile lives for ever and the lifetime is handled by
**    Missile::Type:MissileType::Class
**
**  Missile::Hidden
**
**    When you set this to 1 the unit becomes hidden for a while.
**
**  Missile::CurrentStep
**
**    Movement step. Used for the different trajectories.
**
**  Missile::TotalStep
**
**    Maximum number of step. When CurrentStep >= TotalStep, the movement is finished.
**
**  Missile::Local
**
**    This is a local missile, which can be different on all
**    computer in play. Used for the user interface (fe the green
**    cross).
**
**  Missile::MissileSlot
**
**    Pointer to the slot of this missile. Used for faster freeing.
*/

/*----------------------------------------------------------------------------
--  Includes
----------------------------------------------------------------------------*/

#include "unitsound.h"
#include "vec2i.h"

/*----------------------------------------------------------------------------
--  Declarations
----------------------------------------------------------------------------*/

class CGraphic;
class CUnit;
class CViewport;
class CFile;
class LuaCallback;

/*----------------------------------------------------------------------------
--  Missile-type
----------------------------------------------------------------------------*/

#define MAX_MISSILES 2048        /// maximum number of missiles
#define MAX_LOCAL_MISSILES 4096  /// maximum number of local missiles

/**
**  Missile-class this defines how a missile-type reacts.
**
*/
enum {
	MissileClassNone, /// Missile does nothing
	MissileClassPointToPoint, /// Missile flies from x,y to x1,y1
	MissileClassPointToPointWithHit, /// Missile flies from x,y to x1,y1 than shows hit animation.
	MissileClassPointToPointCycleOnce, /// Missile flies from x,y to x1,y1 and animates ONCE from start to finish and back
	MissileClassPointToPointBounce, /// Missile flies from x,y to x1,y1 than bounces three times.
	MissileClassStay, /// Missile appears at x,y, does it's anim and vanishes.
	MissileClassCycleOnce, /// Missile appears at x,y, then cycle through the frames once.
	MissileClassFire, /// Missile doesn't move, than checks the source unit for HP.
	MissileClassHit, /// Missile shows the hit points.
	MissileClassParabolic, /// Missile flies from x,y to x1,y1 using a parabolic path
	MissileClassLandMine, /// Missile wait on x,y until a non-air unit comes by, the explodes.
	MissileClassWhirlwind, /// Missile appears at x,y, is whirlwind
	MissileClassFlameShield, /// Missile surround x,y
	MissileClassDeathCoil, /// Missile is death coil.
	MissileClassTracer, /// Missile seeks towards to target unit
	MissileClassClipToTarget /// Missile remains clipped to target's current goal and plays his animation once
};

	/// Base structure of missile-types
class MissileType {
public:
	explicit MissileType(const std::string &ident);
	~MissileType();

	/// load the graphics for a missile type
	void LoadMissileSprite();
	void Init();
	void DrawMissileType(int frame, const PixelPos &pos) const;

	int Width() const { return size.x; }
	int Height() const { return size.y; }

//private:
	std::string Ident;         /// missile name
	int Transparency;          /// missile transparency
	PixelSize size;            /// missile size in pixels
	int DrawLevel;             /// Level to draw missile at
	int SpriteFrames;          /// number of sprite frames in graphic
	int NumDirections;         /// number of directions missile can face

	/// @todo FiredSound defined but not used!
	SoundConfig FiredSound;    /// fired sound
	SoundConfig ImpactSound;   /// impact sound for this missile-type

	bool CorrectSphashDamage;  /// restricts the radius damage depending on land, air, naval
	bool Flip;                 /// flip image when facing left
	bool CanHitOwner;          /// missile can hit the owner
	bool FriendlyFire;         /// missile can't hit own units

	int Class;                 /// missile class
	int NumBounces;            /// number of bounces
	int StartDelay;            /// missile start delay
	int Sleep;                 /// missile sleep
	int Speed;                 /// missile speed

	int Range;                 /// missile damage range
	int SplashFactor;          /// missile splash divisor
	std::string ImpactName;    /// impact missile-type name
	MissileType *ImpactMissile;/// missile produces an impact
	std::string SmokeName;     /// impact missile-type name
	MissileType *SmokeMissile; /// Trailling missile
	LuaCallback *ImpactParticle; /// impact particle

// --- FILLED UP ---
	CGraphic *G;         /// missile graphic
};

/*----------------------------------------------------------------------------
--  Missile
----------------------------------------------------------------------------*/

	/// Missile on the map
class Missile
{
protected:
	Missile();

public:
	virtual ~Missile() {};

	static Missile *Init(const MissileType &mtype, const PixelPos &startPos, const PixelPos &destPos);

	virtual void Action() = 0;

	void DrawMissile(const CViewport &vp) const;
	void SaveMissile(CFile &file) const;

	PixelPos source; /// Missile source position
	PixelPos position;   /// missile pixel position
	PixelPos destination;  /// missile pixel destination
	const MissileType *Type;  /// missile-type pointer
	int SpriteFrame;  /// sprite frame counter
	int State;        /// state
	int AnimWait;     /// Animation wait.
	int Wait;         /// delay between frames
	int Delay;        /// delay to showup

	CUnit *SourceUnit;  /// unit that fires (could be killed)
	CUnit *TargetUnit;  /// target unit, used for spells

	int Damage;  /// direct damage that missile applies

	int TTL;     /// time to live (ticks) used for spells
	int Hidden;  /// If this is 1 then the missile is invisible

// Internal use:
	int CurrentStep;  /// Current step (0 <= x < TotalStep).
	int TotalStep;    /// Total step.

	unsigned  Local:1;      /// missile is a local missile
	unsigned int Slot;      /// unique number for draw level.

	static unsigned int Count; /// slot number generator.
};

class MissileDrawProxy
{
public:
	void DrawMissile(const CViewport &vp) const;

	void operator=(const Missile* missile);
public:
	const MissileType *Type;  /// missile-type pointer
	union {
		int Damage;  /// direct damage that missile applies
		int SpriteFrame; /// sprite frame counter
	} data;
	PixelPos pixelPos;
};

class MissileNone : public Missile {
public:
	virtual void Action();
};
class MissilePointToPoint : public Missile {
public:
	virtual void Action();
};
class MissilePointToPointWithHit : public Missile {
public:
	virtual void Action();
};
class MissilePointToPointCycleOnce : public Missile {
public:
	virtual void Action();
};
class MissilePointToPointBounce : public Missile {
public:
	virtual void Action();
};
class MissileStay : public Missile {
public:
	virtual void Action();
};
class MissileCycleOnce : public Missile {
public:
	virtual void Action();
};
class MissileFire : public Missile {
public:
	virtual void Action();
};
class MissileHit : public Missile {
public:
	virtual void Action();
};
class MissileParabolic : public Missile {
public:
	virtual void Action();
};
class MissileLandMine : public Missile {
public:
	virtual void Action();
};
class MissileWhirlwind : public Missile {
public:
	virtual void Action();
};
class MissileFlameShield : public Missile {
public:
	virtual void Action();
};
class MissileDeathCoil : public Missile {
public:
	virtual void Action();
};

class MissileTracer : public Missile {
public:
	virtual void Action();
};

class MissileClipToTarget : public Missile {
public:
	virtual void Action();
};

class BurningBuildingFrame {
public:
	BurningBuildingFrame() : Percent(0), Missile(NULL) {};

	int          Percent;  /// HP percent
	MissileType *Missile;  /// Missile to draw
} ;

/*----------------------------------------------------------------------------
--  Variables
----------------------------------------------------------------------------*/

extern std::vector<BurningBuildingFrame *> BurningBuildingFrames;  /// Burning building frames

/*----------------------------------------------------------------------------
--  Functions
----------------------------------------------------------------------------*/

// In ccl_missile.c

	/// register ccl features
extern void MissileCclRegister();

// In missile.c

	/// load all missile sprites
extern void LoadMissileSprites();
	/// allocate an empty missile-type slot
extern MissileType *NewMissileTypeSlot(const std::string& ident);
	/// Get missile-type by ident
extern MissileType *MissileTypeByIdent(const std::string& ident);
	/// create a missile
extern Missile *MakeMissile(const MissileType &mtype, const PixelPos &startPos, const PixelPos &destPos);
	/// create a local missile
extern Missile *MakeLocalMissile(const MissileType &mtype, const PixelPos &startPos, const PixelPos &destPos);

	/// fire a missile
extern void FireMissile(CUnit &unit);

extern int FindAndSortMissiles(const CViewport &vp, Missile *table[], const int tablesize);
extern int FindAndSortMissiles(const CViewport &vp, MissileDrawProxy table[], const int tablesize);

	/// handle all missiles
extern void MissileActions();
	/// distance from view point to missile
extern int ViewPointDistanceToMissile(const Missile &missile);

	/// Get the burning building missile based on hp percent
extern MissileType *MissileBurningBuilding(int percent);

	/// Save missiles
extern void SaveMissiles(CFile &file);

	/// Initialize missile-types
extern void InitMissileTypes();
	/// Clean missile-types
extern void CleanMissileTypes();
	/// Initialize missiles
extern void InitMissiles();
	/// Clean missiles
extern void CleanMissiles();

#ifdef DEBUG
extern void FreeBurningBuildingFrames();
#endif

//@}

#endif // !__MISSILE_H__
