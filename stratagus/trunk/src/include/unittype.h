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
/**@name unittype.h	-	The unit-types headerfile. */
//
//	(c) Copyright 1998-2001 by Lutz Sammer
//
//	$Id$

#ifndef __UNITTYPE_H__
#define __UNITTYPE_H__

//@{

/*----------------------------------------------------------------------------
--	Includes
----------------------------------------------------------------------------*/

#include "video.h"
#include "icons.h"
#include "button.h"
#include "sound_id.h"
#include "unitsound.h"
#include "upgrade_structs.h"

/*----------------------------------------------------------------------------
--	Declarations
----------------------------------------------------------------------------*/

/**
**	Defines the animation for different actions.
*/
typedef struct _animation_ {
    char	Flags;			/// Flags for actions.
    char	Pixel;			/// Change the position in pixels
    char	Sleep;			/// Wait for next animation
    char	Frame;			/// Sprite-frame to display
} Animation;

#define AnimationRestart	1	/// restart animation
#define AnimationReset		2	/// animation could here be aborted
#define AnimationSound		4	/// play sound
#define AnimationMissile	8	/// fire projectil
#define AnimationEnd		0x80	/// animation end in memory

/**
**	Define all animations scripts of an unittype.
*/
typedef struct __animations__ {
    Animation*	Still;			/// Standing still
    Animation*	Move;			/// Unit moving
    Animation*	Attack;			/// Unit attacking/working
    Animation*	Die;			/// Unit dieing
    Animation**	Extend;			/// For future extensions
} Animations;

/**
**      Missile type definition (used in config tables)
*/
typedef struct _missile_config_ {
    char*	Name;			/// config missile name
    // FIXME: void* is needed,because of recursive headers :(
    void*	Missile;		/// identifier to use to run time
} MissileConfig;

/**
**	Typedef of base structure of unit-type
*/
typedef struct _unit_type_ UnitType;

/**
**	Base structure of unit-type
**
**	Contains all informations for a special unit-type.
**	Appearance, features...
*/
struct _unit_type_ {
    const void*	OType;			/// Object type (future extensions)

    char*	Ident;			/// identifier
    char*	Name;			/// unit name shown from the engine
    char*	SameSprite;		/// unittype shared sprites
    char*	File[4/*TilesetMax*/];	/// sprite files

    int		Width;			/// sprite width
    int		Height;			/// sprite height

    Animations*	Animations;		/// animation scripts

    IconConfig	Icon;			/// icon to display for this unit
    MissileConfig Missile;		/// missile weapon

    char*	CorpseName;		/// corpse type name
    UnitType*	CorpseType;		/// corpse unit-type
    int		CorpseScript;		/// corpse script start

    int		_Speed;			/// movement speed

// this is taken from the UDTA section
    int		OverlapFrame;		/// what is shown in construction phase
    int		_SightRange;		/// sight range
    unsigned	_HitPoints;		/// maximum hit points
    // FIXME: only flag
    int		Magic;			/// Unit can cast spells

    int		_Costs[MaxCosts];	/// how many resources needed

    int		TileWidth;		/// tile size on map width
    int		TileHeight;		/// tile size on map height
    int		BoxWidth;		/// selected box size width
    int		BoxHeight;		/// selected box size height
    int		MinAttackRange;		/// minimal attack range
    int		_AttackRange;		/// how far can the unit attack
    int		ReactRangeComputer;	/// reacts on enemy for computer
    int		ReactRangeHuman;	/// reacts on enemy for human player
    int		_Armor;			/// amount of armor this unit has
    int		Priority;		/// Priority value / AI Treatment
    int		_BasicDamage;		/// Basic damage dealt
    int		_PiercingDamage;	/// Piercing damage dealt
    int		WeaponsUpgradable;	/// Weapons could be upgraded
    int		ArmorUpgradable;	/// Armor could be upgraded
    //int	MissileWeapon;		/// Missile type used when it attacks
    int		UnitType;		/// land / fly / naval
    // FIXME: original only visual effect, we do more with this!
#define UnitTypeLand	0			/// Unit lives on land
#define UnitTypeFly	1			/// Unit lives in air
#define UnitTypeNaval	2			/// Unit lives on water
    int		DecayRate;		/// Decay rate in 1/6 seconds
    int		AnnoyComputerFactor;	/// How much this annoys the computer
    int		MouseAction;		/// Right click action
#define MouseActionNone		0		/// Nothing
#define MouseActionAttack	1		/// Attack
#define MouseActionMove		2		/// Move
#define MouseActionHarvest	3		/// Harvest or mine gold
#define MouseActionHaulOil	4		/// Haul oil
#define MouseActionDemolish	5		/// Demolish
#define MouseActionSail		6		/// Sail
    int		Points;			/// How many points you get for unit
    int		CanTarget;		/// which units can it attack
#define CanTargetLand	1			/// can attack land units
#define CanTargetSea	2			/// can attack sea units
#define CanTargetAir	4			/// can attack air units

    unsigned LandUnit : 1;		/// Land animated
    unsigned AirUnit : 1;		/// Air animated
    unsigned SeaUnit : 1;		/// Sea animated
    unsigned ExplodeWhenKilled : 1;	/// Death explosion animated
    unsigned Critter : 1;		/// Unit is controlled by nobody
    unsigned Building : 1;		/// Building
    unsigned Submarine : 1;		/// Is only visible by CanSeeSubmarine
    unsigned CanSeeSubmarine : 1;	/// Only this units can see Submarine
    unsigned CowerWorker : 1;		/// Is a worker, runs away if attcked
    unsigned Tanker : 1;		/// FIXME: used? Can transport oil
    unsigned Transporter : 1;		/// can transport units
    unsigned GivesOil : 1;		/// We get here oil
    unsigned StoresGold : 1;		/// We can store gold/wood here
    unsigned Vanishes : 1;		/// Corpes & destroyed places.
    unsigned GroundAttack : 1;		/// Can do command ground attack
    unsigned IsUndead : 1;		///
    unsigned ShoreBuilding : 1;		///
    unsigned CanCastSpell : 1;		///
    unsigned StoresWood : 1;		///
    unsigned CanAttack : 1;		///
    unsigned Tower : 1;			///
    unsigned OilPatch : 1;		///
    unsigned GoldMine : 1;		///
    unsigned Hero : 1;			///
    unsigned StoresOil : 1;		///
    unsigned Volatile : 1;		/// invisiblity/unholy armor kills unit
    unsigned CowerMage : 1;		///
    unsigned Organic : 1;		/// organic

    unsigned SelectableByRectangle : 1;	/// selectable with mouse rectangle

    //ButtonConfig* Buttons;		/// buttons of this unit-type
    UnitSound Sound;			/// sounds for events
    // FIXME: temporary solution
    WeaponSound Weapon;                 /// currently sound for weapon

// --- FILLED UP ---

	// FIXME: This stats should? be moved into the player struct
    UnitStats Stats[PlayerMax];		/// Unit status for each player

	// FIXME: Should us a general name f.e. Slot here?
    unsigned	Type;			/// Type as number

    void*	Property;		/// CCL property storage

    Graphic*	Sprite;			/// sprite images
};

    // FIXME: ARI: should be dynamic (ccl..)
    /// How many unit-types are currently supported.
#define UnitTypeMax	0xFF

/*----------------------------------------------------------------------------
--	Variables
----------------------------------------------------------------------------*/

extern const char UnitTypeType[];	/// unit-type type
extern UnitType* UnitTypes;		/// all unit-types
extern int NumUnitTypes;		/// number of unit-types made.

extern UnitType*UnitTypeGoldMine;	/// Gold-mine unit-type pointer.
extern UnitType*UnitTypeHumanTanker;	/// orc tanker unit-type pointer.
extern UnitType*UnitTypeOrcTanker;	/// human tanker unit-type pointer.
extern UnitType*UnitTypeHumanTankerFull;/// orc tanker full unit-type pointer.
extern UnitType*UnitTypeOrcTankerFull;	/// human tanker full unit-type pointer.
extern UnitType*UnitTypeHumanWorker;	/// Human worker.
extern UnitType*UnitTypeOrcWorker;	/// Orc worker.
extern UnitType*UnitTypeHumanWorkerWithGold;	/// Human worker with gold.
extern UnitType*UnitTypeOrcWorkerWithGold;	/// Orc worker with gold.
extern UnitType*UnitTypeHumanWorkerWithWood;	/// Human worker with wood.
extern UnitType*UnitTypeOrcWorkerWithWood;	/// Orc worker with wood.
extern UnitType*UnitTypeHumanFarm;	/// Human farm
extern UnitType*UnitTypeOrcFarm;	/// Orc farm
extern UnitType*UnitTypeCritter;	/// Critter unit-type pointer

extern char** UnitTypeWcNames;		/// Mapping wc-number 2 symbol

/*----------------------------------------------------------------------------
--	Functions
----------------------------------------------------------------------------*/

extern void UnitTypeCclRegister(void);	/// register ccl features

extern void PrintUnitTypeTable(void);	/// generate c-table
extern void UpdateStats(void);		/// update unit stats
extern void ParsePudUDTA(const char*,int); /// parse pud udta table
extern UnitType* UnitTypeByIdent(const char*);	/// get unit-type by ident
extern UnitType* UnitTypeByWcNum(unsigned);	/// get unit-type by wc number

    /// Draw the sprite frame of unit-type
extern void DrawUnitType(const UnitType* type,unsigned frame,int x,int y);
extern void LoadUnitTypes(FILE* file);	/// load the unit-type table
extern void SaveUnitTypes(FILE* file);	/// save the unit-type table
extern UnitType* NewUnitTypeSlot(char*);/// allocate an empty unit-type slot
extern void InitUnitTypes(void);	/// Init unit-type table
extern void LoadUnitSprites(void);	/// Load all unit-type sprites

//@}

#endif	// !__UNITTYPE_H__
