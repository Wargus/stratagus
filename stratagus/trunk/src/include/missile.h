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
/**@name missile.h	-	The missile headerfile. */
/*
**	(c) Copyright 1998-2000 by Lutz Sammer
**
**	$Id$
*/

#ifndef __MISSILE_H__
#define __MISSILE_H__

//@{

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
--	Missile - type
----------------------------------------------------------------------------*/

/**
**	Base structure of missile types
*/
typedef struct _missile_type_ {
    void*	OType;			/// Object type (future extensions)

    char*	Ident;			/// missile name
    char*	File;			/// missile sprite file

    unsigned	Width;			/// missile width in pixels
    unsigned	Height;			/// missile height in pixels

//  SoundConfig FiredSound;		/// fired sound (FIXME: must write this)
    SoundConfig	ImpactSound;		/// impact sound for this missile type

    int		Class;			/// missile class 
    int		Speed;			/// missile speed

// --- FILLED UP ---
    unsigned	Type;			/// missile type as number

    RleSprite*	RleSprite;		/// missile sprite image
} MissileType;

#if 1
/*
**	Missile types	(FIXME: should be removed!!)
*/
#define MissileLightning		0x00
#define MissileGriffonHammer		0x01
#define MissileDragonBreath		0x02
#define MissileFireball			0x03
#define MissileFlameShield		0x04
#define MissileBlizzard			0x05
#define MissileDeathDecay		0x06
#define MissileBigCannon		0x07
#define MissileExorcism			0x08
#define MissileHealEffect		0x09
#define MissileTouchOfDeath		0x0A
#define MissileRune			0x0B
#define MissileWhirlwind		0x0C
#define MissileCatapultRock		0x0D
#define MissileBallistaBolt		0x0E
#define MissileArrow			0x0F
#define MissileAxe			0x10
#define MissileSubmarineMissile		0x11
#define MissileTurtleMissile		0x12
#define MissileSmallFire		0x13
#define MissileBigFire			0x14
#define MissileImpact			0x15
#define MissileNormalSpell		0x16
#define MissileExplosion		0x17
#define MissileSmallCannon		0x18
#define MissileCannonExplosion		0x19
#define MissileCannonTowerExplosion	0x1A
#define MissileDaemonFire		0x1B
#define MissileGreenCross		0x1C
#define MissileNone			0x1D

#endif

#define MissileTypeMax			0x1E

#define MissileFree			(MissileType*)0	// free missile slot

/*----------------------------------------------------------------------------
--	Missile
----------------------------------------------------------------------------*/

/**
**	Missile on the map.
*/
typedef struct _missile_ {
    int		X;			/// missile pixel position
    int		Y;			/// missile pixel position
    int		DX;			/// missile pixel destination
    int		DY;			/// missile pixel destination
    MissileType*Type;			/// missile type pointer
    int		Frame;			/// frame counter
    int		State;			/// state
    int		Wait;			/// delay

    Unit*	SourceUnit;		/// unit that fires (could be killed)
    UnitType*	SourceType;		/// type of unit that fires
    UnitStats*	SourceStats;		/// stats of unit that fires
    Player*	SourcePlayer;		/// player of unit that fires

    int		D;			/// for point to point missiles
    int		Dx;			/// delta x
    int		Dy;			/// delta y
    int		Xstep;			/// X step
    int		Ystep;			/// Y step
} Missile;

/*----------------------------------------------------------------------------
--	Variables
----------------------------------------------------------------------------*/

extern MissileType MissileTypes[];	/// all missile types

/*----------------------------------------------------------------------------
--	Functions
----------------------------------------------------------------------------*/

    /// load the graphics for the missiles 
extern void LoadMissileSprites(void);
    /// Get missile type by ident.
extern MissileType* MissileTypeByIdent(const char*);
    /// create a missile
extern Missile* MakeMissile(int missile_type,int sx,int sy,int dx,int dy);
			// FIXME: ^ use pointer
    /// fire a missile
extern void FireMissile(Unit* unit);
    /// draw all missiles
extern void DrawMissiles(void);
    /// handle all missiles
extern void MissileActions(void);
    /// distance from view point to missile
extern int ViewPointDistanceToMissile(const Missile* dest);

//@}

#endif	// !__MISSILE_H__
