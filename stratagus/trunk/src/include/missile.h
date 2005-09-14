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
//      $Id$

#ifndef __MISSILE_H__
#define __MISSILE_H__

//@{

/*----------------------------------------------------------------------------
--  Documentation
----------------------------------------------------------------------------*/

/**
**  @struct _missile_type_ missile.h
**
**  \#include "missile.h"
**
**  typedef struct _missile_type_ MissileType;
**
**  This structure defines the base type information of all missiles. It
**  contains all information that all missiles of the same type shares.
**  The fields are filled from the configuration files (CCL). See
**  (define-missile-type).
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
**  MissileType::File
**
**    File containing the image (sprite) graphics of the missile.
**    The file can contain multiple sprite frames.  The sprite frames
**    for the different directions are placed in the row.
**    The different animations steps are placed in the column. But
**    the correct order depends on MissileType::Class (
**    ::_missile_class_, ...). Missiles like fire have no directions,
**    missiles like arrows have a direction.
**    @note Note that only 8 directions are currently supported and
**    only 5 are stored in the image (N NW W SW S) and 4 are mirrored.
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
**    Set a missile transparency. Current supported value is 50 only.
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
**    informations (::_missile_class_, ::MissileClassNone, ...).
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
*/

/**
**  @struct _missile_ missile.h
**
**  \#include "missile.h"
**
**  typedef struct _missile_ Missile;
**
**  This structure contains all informations about a missile in game.
**  A missile could have different effects, based on its missile-type.
**  Missiles could be saved and stored with CCL. See (missile).
**  Currently only a tile, an unit or a missile could be placed on the map.
**
**
**  The missile structure members:
**
**  Missile::X Missile::Y
**
**    Missile current map position in pixels.  To convert a map tile
**    position to pixel position use: (mapx*::TileSizeX+::TileSizeX/2)
**    and (mapy*::TileSizeY+::TileSizeY/2)
**    @note ::TileSizeX%2==0 && ::TileSizeY%2==0 and ::TileSizeX,
**    ::TileSizeY are currently fixed 32 pixels.
**
**  Missile::SourceX Missile::SourceY
**
**    Missile original map position in pixels.  To convert a map tile
**    position to pixel position use: (mapx*::TileSizeX+::TileSizeX/2)
**    and (mapy*::TileSizeY+::TileSizeY/2)
**    @note ::TileSizeX%2==0 && ::TileSizeY%2==0 and ::TileSizeX,
**    ::TileSizeY are currently fixed 32 pixels.
**
**  Missile::DX Missile::DY
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
**    when field MissileType::CanHitOwner==1. Also used for kill
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

/*----------------------------------------------------------------------------
--  Declarations
----------------------------------------------------------------------------*/

class Graphic;
struct _unit_;
struct _viewport_;
class CLFile;

/*----------------------------------------------------------------------------
--  Missile-type
----------------------------------------------------------------------------*/

#ifndef __STRUCT_MISSILETYPE__
#define __STRUCT_MISSILETYPE__  /// protect duplicate missile typedef

/**
**  Missile-type typedef
*/
typedef struct _missile_type_ MissileType;

#endif

#define MAX_MISSILES 2048        /// maximum number of missiles
#define MAX_LOCAL_MISSILES 4096  /// maximum number of local missiles

/**
**  Missile-type-class typedef
*/
typedef int MissileClass;

/**
**  Missile-class this defines how a missile-type reacts.
**
*/
enum _missile_class_ {
	/// Missile does nothing
	MissileClassNone,
	/// Missile flies from x,y to x1,y1
	MissileClassPointToPoint,
	/// Missile flies from x,y to x1,y1 than shows hit animation.
	MissileClassPointToPointWithHit,
	/// Missile flies from x,y to x1,y1 and animates ONCE from start to finish and back
	MissileClassPointToPointCycleOnce,
	/// Missile flies from x,y to x1,y1 than bounces three times.
	MissileClassPointToPointBounce,
	/// Missile appears at x,y, does it's anim and vanishes.
	MissileClassStay,
	/// Missile appears at x,y, then cycle through the frames once.
	MissileClassCycleOnce,
	/// Missile doesn't move, than checks the source unit for HP.
	MissileClassFire,
	/// Missile shows the hit points.
	MissileClassHit,
	/// Missile flies from x,y to x1,y1 using a parabolic path
	MissileClassParabolic,
	/// Missile wait on x,y until a non-air unit comes by, the explodes.
	MissileClassLandMine,
	/// Missile appears at x,y, is whirlwind
	MissileClassWhirlwind,
	/// Missile surround x,y
	MissileClassFlameShield,
	/// Missile is death coil.
	MissileClassDeathCoil
};

	/// Base structure of missile-types
struct _missile_type_ {
	char* Ident;          /// missile name
	int   Transparency;   /// missile transparency possible value is 50 (later 25 and 75)
	int   Width;          /// missile width in pixels
	int   Height;         /// missile height in pixels
	int   DrawLevel;      /// Level to draw missile at
	int   SpriteFrames;   /// number of sprite frames in graphic
	int   NumDirections;  /// number of directions missile can face

	/// @todo FireSound defined but not used!
	SoundConfig FiredSound;   /// fired sound
	SoundConfig ImpactSound;  /// impact sound for this missile-type

	unsigned Flip : 1;        /// flip image when facing left
	unsigned CanHitOwner : 1; /// missile can hit the owner
	unsigned FriendlyFire : 1;/// missile can't hit own units

	MissileClass Class;       /// missile class
	int          NumBounces;  /// number of bounces
	int          StartDelay;  /// missile start delay
	int          Sleep;       /// missile sleep
	int          Speed;       /// missile speed

	int          Range;          /// missile damage range
	int          SplashFactor;   /// missile splash divisor
	char*        ImpactName;     /// impact missile-type name
	MissileType* ImpactMissile;  /// missile produces an impact
	char*        SmokeName;      /// impact missile-type name
	MissileType* SmokeMissile;   /// Trailling missile

// --- FILLED UP ---
	Graphic *G;         /// missile graphic
};

/*----------------------------------------------------------------------------
--  Missile
----------------------------------------------------------------------------*/

/**
**  Missile typedef.
*/
typedef struct _missile_ Missile;
typedef void FuncController(Missile*);

	/// Missile on the map
struct _missile_ {
	int SourceX;  /// Missile Source X
	int SourceY;  /// Missile Source Y
	int X;        /// missile pixel position
	int Y;        /// missile pixel position
	int DX;       /// missile pixel destination
	int DY;       /// missile pixel destination
	MissileType* Type;  /// missile-type pointer
	int SpriteFrame;  /// sprite frame counter
	int State;        /// state
	int AnimWait;     /// Animation wait.
	int Wait;         /// delay between frames
	int Delay;        /// delay to showup

	struct _unit_* SourceUnit;  /// unit that fires (could be killed)
	struct _unit_* TargetUnit;  /// target unit, used for spells

	int Damage;  /// direct damage that missile applies

	int TTL;     /// time to live (ticks) used for spells
	int Hidden;  /// If this is 1 then the missile is invisible

// Internal use:
	int CurrentStep;  /// Current step (0 <= x < TotalStep).
	int TotalStep;    /// Total step.

	unsigned  Local:1;      /// missile is a local missile
	Missile** MissileSlot;  /// pointer to missile slot
};

typedef struct _burning_building_frame_ {
	int          Percent;  /// HP percent
	MissileType* Missile;  /// Missile to draw
	struct _burning_building_frame_* Next;  /// Next pointer
} BurningBuildingFrame;

/*----------------------------------------------------------------------------
--  Variables
----------------------------------------------------------------------------*/

extern MissileType** MissileTypes;  /// All missile-types
extern int NumMissileTypes;        /// Number of missile-types

extern const char* MissileClassNames[];  /// Missile class names

extern BurningBuildingFrame* BurningBuildingFrames;  /// Burning building frames

/*----------------------------------------------------------------------------
--  Functions
----------------------------------------------------------------------------*/

// In ccl_missile.c

	/// register ccl features
extern void MissileCclRegister(void);

// In missile.c

	/// load the graphics for a missile type
extern void LoadMissileSprite(MissileType* mtype);
	/// load all missile sprites
extern void LoadMissileSprites();
	/// allocate an empty missile-type slot
extern MissileType* NewMissileTypeSlot(char* ident);
	/// Get missile-type by ident
extern MissileType* MissileTypeByIdent(const char* ident);
	/// create a missile
extern Missile* MakeMissile(MissileType* mtype, int sx, int sy, int dx,
	int dy);
	/// create a local missile
extern Missile* MakeLocalMissile(MissileType* mtype, int sx, int sy, int dx,
	int dy);
	/// fire a missile
extern void FireMissile(struct _unit_* unit);

	/// Draw all missiles
extern void DrawMissile(const Missile* missile);
extern int FindAndSortMissiles(const struct _viewport_* vp, Missile** table);

	/// handle all missiles
extern void MissileActions(void);
	/// distance from view point to missile
extern int ViewPointDistanceToMissile(const Missile* missile);

	/// Get the burning building missile based on hp percent
extern MissileType* MissileBurningBuilding(int percent);

	/// Save missiles
extern void SaveMissiles(CLFile *file);

	/// Initialize missile-types
extern void InitMissileTypes(void);
	/// Clean missile-types
extern void CleanMissileTypes(void);
	/// Initialize missiles
extern void InitMissiles(void);
	/// Clean missiles
extern void CleanMissiles(void);

FuncController MissileActionNone;
FuncController MissileActionPointToPoint;
FuncController MissileActionPointToPointWithHit;
FuncController MissileActionPointToPointCycleOnce;
FuncController MissileActionPointToPointBounce;
FuncController MissileActionStay;
FuncController MissileActionCycleOnce;
FuncController MissileActionFire;
FuncController MissileActionHit;
FuncController MissileActionParabolic;
FuncController MissileActionLandMine;
FuncController MissileActionWhirlwind;
FuncController MissileActionFlameShield;
FuncController MissileActionDeathCoil;

//@}

#endif // !__MISSILE_H__
