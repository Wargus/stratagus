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
//
//	(c) Copyright 1998-2001 by Lutz Sammer
//
//	$Id$

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
**	Missile type typedef.
*/
typedef struct _missile_type_ MissileType;

/**
**	Base structure of missile types
*/
struct _missile_type_ {
    const void*	OType;			/// Object type (future extensions)

    char*	Ident;			/// missile name
    char*	File;			/// missile sprite file

    unsigned	Width;			/// missile width in pixels
    unsigned	Height;			/// missile height in pixels

//  SoundConfig FiredSound;		/// fired sound (FIXME: must write this)
    SoundConfig	ImpactSound;		/// impact sound for this missile type

    int		Class;			/// missile class
    int		Speed;			/// missile speed

    char*	ImpactName;		/// Impact missile type name
    MissileType*ImpactMissile;		/// Missile produces an impact

// --- FILLED UP ---
    Graphic*	Sprite;			/// missile sprite image
};

    /// how many missile type are maximal supported
#define MissileTypeMax			0x22

    /// mark a free missile slot
#define MissileFree			(MissileType*)0

/*----------------------------------------------------------------------------
--	Missile
----------------------------------------------------------------------------*/

/**
**	Missile typedef.
*/
typedef struct _missile_ Missile;


/**
**	Missile on the map.
*/
struct _missile_ {
    int		X;			/// missile pixel position
    int		Y;			/// missile pixel position
    int		DX;			/// missile pixel destination
    int		DY;			/// missile pixel destination
    MissileType*Type;			/// missile type pointer
    int		Frame;			/// frame counter
    int		State;			/// state
    int		Wait;			/// delay

    Unit*	SourceUnit;		/// unit that fires (could be killed)
    Unit*	TargetUnit;             /// target unit, used for spells

    int		Damage;                 /// direct damage that missile applies

    int		D;			/// for point to point missiles
    int		Dx;			/// delta x
    int		Dy;			/// delta y
    int		Xstep;			/// X step
    int		Ystep;			/// Y step

    int TTL;				/// time to live (ticks) used for spells
    int (*Controller)( Missile* this_missile );    /// used to controll spells
};

/*----------------------------------------------------------------------------
--	Variables
----------------------------------------------------------------------------*/

extern MissileType MissileTypes[];	/// all missile types
extern MissileType* MissileTypeSmallFire;	/// Small fire missile type
extern MissileType* MissileTypeBigFire;		/// Big fire missile type
extern MissileType* MissileTypeGreenCross;	/// Green cross missile type

extern char** MissileTypeWcNames;	/// Mapping wc-number 2 symbol

/*----------------------------------------------------------------------------
--	Functions
----------------------------------------------------------------------------*/

    /// register ccl features
extern void MissileCclRegister(void);

    /// load the graphics for the missiles
extern void LoadMissileSprites(void);
    /// Get missile type by ident.
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

//@}

#endif	// !__MISSILE_H__
