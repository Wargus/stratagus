//   ___________		     _________		      _____  __
//   \_	  _____/______	 ____	____ \_	  ___ \____________ _/ ____\/  |_
//    |	   __) \_  __ \_/ __ \_/ __ \/	  \  \/\_  __ \__  \\	__\\   __\ 
//    |	    \	|  | \/\  ___/\	 ___/\	   \____|  | \// __ \|	|   |  |
//    \___  /	|__|	\___  >\___  >\______  /|__|  (____  /__|   |__|
//	  \/		    \/	   \/	     \/		   \/
//  ______________________			     ______________________
//			  T H E	  W A R	  B E G I N S
//	   FreeCraft - A free fantasy real time strategy game engine
//
/**@name missile.h	-	The missile headerfile. */
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

#ifndef __MISSILE_H__
#define __MISSILE_H__

//@{

/*----------------------------------------------------------------------------
--	Documentation
----------------------------------------------------------------------------*/

/**
**	@struct _missile_type_ missile.h
**
**	\#include "missile.h"
**
**	typedef struct _missile_type_ MissileType;
**
**	This structure defines the base type informations of all missiles.
**	It contains all informations that all missiles of the same type shares.
**
**	The missile-type structure members:
**
**	MissileType::OType
**
**		Object type (future extensions).
**
**	MissileType::Ident
**
**		Unique identifier of the missile-type, used to reference it in
**		config files and during startup. Don't use this in game, use
**		instead the pointer to this structure.
**
**	MissileType::File
**
**		File containing the image graphics of the missile. The file
**		can contain multiple frames.
**		The frames for the different directions are placed in the row.
**		The different animations steps are placed in the column.
**
**	MissileType::Width MissileType::Height
**
**		Size (width and height) of a frame in the image. All frames
**		of the missile-type must have the same size.
**
**	MissleType::Frames
**
**		Number of frames in the graphic image.
**
**	MissileType::FiredSound
**
**		Sound of the missile, if fired. @note currently not used.
**
**	MissileType::ImpactSound
**
**		Impact sound for this missile.
**
**	MissileType::CanHitOwner
**
**		Can hit the unit that have fired the missile.
**
**	MissileType::FriendlyFire
**
**		Can't hit the units of the same player, that has the
**		missile fired.
**
**	MissileType::Class
**
**		Class of the missile-type, defines the basic effects of the
**		missile. Look at the different class identifiers for more
**		informations (::_missile_class_ ::MissileClassNone ...).
**
**	MissileType::Delay
**
**		Delay after the missile generation, until the missile animation
**		starts.
**
**	MissileType::Sleep
**
**		This are the number of frames to wait for the next animation.
**		This value is for all animations steps the same.  0 is the
**		fastest and 255 the slowest animation.
**		@note Perhaps we should later allow animation scripts for
**		more complex animations.
**
**	MissileType::Speed
**
**		The speed how fast the missile moves. 0 the missile didn't
**		move, 1 is the slowest speed and 32 the fastest supported
**		speed. This is how many pixels the missiles moves with each
**		animation step.
**
**	MissileType::Range
**
**		Determines the range that a projectile will deal its damage.
**		A range of 0 will mean that the damage will be limited to only
**		where the missile was directed towards. So if you shot a
**		missile at a unit, it would only damage that unit. A value of
**		1 only effects the field on that the missile is shot. A value
**		of 2  would mean that for a range of 1 around the impact spot,
**		the damage for that particular missile would be dealt.
**		All fields that aren't the center get only 50% of the damage.
**
**	MissileType::ImpactName
**
**		The name of the missile-type generated, if the missile
**		reaches its end point. Can be used to generate chains of
**		missiles. Used only during configuration and startup.
**
**	MissileType::ImpactMissile
**
**		Pointer to the impact missile-type. Used during run time.
**
**	MissileType::Sprite
**
**		Missile sprite loaded from MissileType::File
*/

/**
**	@struct _missile_ missile.h
**
**	\#include "missile.h"
**
**	typedef struct _missile_ Missile;
**
**	This structure contains all informations about a missile in game.
**	A missile could have different effects, based on its missile-type.
**	Currently only a tile, an unit or a missile could be placed on the map.
**
**	The missile structure members:
**
**	Missile::X Missile::Y
**
**		Missile current map position in pixels. To convert a map tile
**		position to pixel position use: (mapx*::TileSizeX+::TileSizeX/2)
**		and (mapy*::TileSizeY+::TileSizeY/2)
**
**	Missile::DX Missile::DY
**
**		Missile destination on the map in pixels. If
**		Missile::X==MissileDX and Missile::Y==Missile::DY the missile
**		stays at its position.
**
**	Missile::Type
**
**		::MissileType pointer of the missile, contains the shared
**		informations of all missiles of the same type.
**
**	Missile::Frame
**
**		Current animation frame of the missile. Animation scripts
**		aren't currently supported for missiles, everything is
**		handled by the MissileType::Class. If wanted we can add
**		animation scripts support to the engine.
**
**	Missile::State
**
**		Current state of the missile.
**
**	Missile::Wait
**
**		Wait this number of frames until the next state or animation
**		of this missile is handled.
**
**	Missile::Delay
**
**		Number of frames the missile isn't shown on the map.
**
**	Missile::SourceUnit
**
**		The owner of the missile. Normally the unit which has fired
**		this missile. Some missiles didn't hurt the owner.
**
**	Missile::TargetUnit
**
**		The target of the missile. Normally the unit which should be
**		hit by the missile.
**
**	Missile::Damage
**
**		Damage done by missile.
**
**	Missile::TTL
**
**	FIXME: not written documentation
**
**	Missile::Controller
**
**	FIXME: not written documentation
**
**	Missile::D
**
**	FIXME: not written documentation
**
**	Missile::Dx Missile::Dy
**
**	FIXME: not written documentation
**
**	Missile::Xstep Missile::Ystep
**
**	FIXME: not written documentation
**
*/

/*----------------------------------------------------------------------------
--	Includes
----------------------------------------------------------------------------*/

#include "unitsound.h"
#include "unittype.h"
#include "upgrade_structs.h"
#include "player.h"
#include "video.h"

/*----------------------------------------------------------------------------
--	Declarations
----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
--	Missile-type
----------------------------------------------------------------------------*/

/**
**	Missile-type typedef
*/
typedef struct _missile_type_ MissileType;

    ///		Base structure of missile-types
struct _missile_type_ {
    const void* OType;			/// Object type (future extensions)

    char*	Ident;			/// missile name
    char*	File;			/// missile sprite file

    unsigned	Width;			/// missile width in pixels
    unsigned	Height;			/// missile height in pixels
    unsigned	Frames;			/// number of frames in graphic

	// FIXME: FireSound defined but not used!
    SoundConfig FiredSound;		/// fired sound
    SoundConfig ImpactSound;		/// impact sound for this missile-type

    unsigned	CanHitOwner : 1;	/// missile can hit the owner
    unsigned	FriendlyFire : 1;	/// missile can't hit own units

    int		Class;			/// missile class
    int		Delay;			/// missile delay
    int		Sleep;			/// missile sleep
    int		Speed;			/// missile speed

    int		Range;			/// missile damage range
    char*	ImpactName;		/// Impact missile-type name
    MissileType*ImpactMissile;		/// Missile produces an impact

// --- FILLED UP ---
    Graphic*	Sprite;			/// missile sprite image
};

    /// mark a free missile slot
#define MissileFree			(MissileType*)0

/*----------------------------------------------------------------------------
--	Missile
----------------------------------------------------------------------------*/

/**
**	Missile typedef.
*/
typedef struct _missile_ Missile;

    /// Missile on the map
struct _missile_ {
    int		X;			/// missile pixel position
    int		Y;			/// missile pixel position
    int		DX;			/// missile pixel destination
    int		DY;			/// missile pixel destination
    MissileType*Type;			/// missile-type pointer
    int short	Frame;			/// frame counter
    int short	State;			/// state
    int short	Wait;			/// delay between frames
    int short	Delay;			/// delay to showup

    Unit*	SourceUnit;		/// unit that fires (could be killed)
    Unit*	TargetUnit;		/// target unit, used for spells

    int		Damage;			/// direct damage that missile applies

    int		TTL;			/// time to live (ticks) used for spells
    void (*Controller)( Missile* );	/// used to controll spells

// Internal use:
    int		D;			/// for point to point missiles
    int		Dx;			/// delta x
    int		Dy;			/// delta y
    int		Xstep;			/// X step
    int		Ystep;			/// Y step
};

/*----------------------------------------------------------------------------
--	Variables
----------------------------------------------------------------------------*/

extern char** MissileTypeWcNames;	/// Mapping wc-number 2 symbol

extern MissileType* MissileTypes;		/// all missile-types
extern MissileType* MissileTypeSmallFire;	/// Small fire missile-type
extern MissileType* MissileTypeBigFire;		/// Big fire missile-type
extern MissileType* MissileTypeGreenCross;	/// Green cross missile-type
extern MissileType* MissileTypeExplosion;	/// Explosion missile-type

extern const char* MissileClassNames[];		/// Missile class names

/*----------------------------------------------------------------------------
--	Functions
----------------------------------------------------------------------------*/

//	In ccl_missile.c

    /// register ccl features
extern void MissileCclRegister(void);

//	In missile.c

    /// load the graphics for the missiles
extern void LoadMissileSprites(void);
    /// allocate an empty missile-type slot
extern MissileType* NewMissileTypeSlot(char*);
    /// Get missile-type by ident
extern MissileType* MissileTypeByIdent(const char*);
    /// create a missile
extern Missile* MakeMissile(MissileType*,int,int,int,int);
    /// fire a missile
extern void FireMissile(Unit*);
    /// draw all missiles
extern void DrawMissiles(void);
    /// handle all missiles
extern void MissileActions(void);
    /// distance from view point to missile
extern int ViewPointDistanceToMissile(const Missile*);

    /// Save missile-types
extern void SaveMissileTypes(FILE*);
    /// Save missiles
extern void SaveMissiles(FILE*);

    /// Initialize missile-types
extern void InitMissileTypes(void);
    /// Clean missile-types
extern void CleanMissileTypes(void);
    /// Initialize missiles
extern void InitMissiles(void);
    /// Clean missiles
extern void CleanMissiles(void);

//@}

#endif	// !__MISSILE_H__
