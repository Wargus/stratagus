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
//	(c) Copyright 1998-2002 by Lutz Sammer
//
//	FreeCraft is free software; you can redistribute it and/or modify
//	it under the terms of the GNU General Public License as published
//	by the Free Software Foundation; only version 2 of the License.
//
//	FreeCraft is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//	GNU General Public License for more details.
//
//	$Id$

#ifndef __UNITTYPE_H__
#define __UNITTYPE_H__

//@{

/*----------------------------------------------------------------------------
--	Documentation
----------------------------------------------------------------------------*/

/**
**	@struct _unit_type_ unittype.h
**
**	\#include "unittype.h"
**
**	typedef struct _unit_type_ UnitType;
**
**	This structure contains the informations that are shared between all
**	units of the same type and determins if an unit is a building,
**	a person, ...
**
**	The unit-type structure members:
**
**	UnitType::OType
**
**		Object type (future extensions).
**
**	UnitType::Ident
**
**		Unique identifier of the unit-type, used to reference it in
**		config files and during startup. As convention they start with
**		"unit-" fe. "unit-farm".
**		@note Don't use this member in game, use instead the pointer
**		to this structure. See UnitTypeByIdent().
**
**	UnitType::Name
**
**		Pretty name shown by the engine. The name should be shorter
**		than 17 characters and no word can be longer than 8 characters. 
**
**	UnitType::SameSprite
**
**		Identifier of an unit-type with this are the sprites shared.
**
**	UnitType::File[::TilesetMax]
**
**		Path file name of sprite files for the different tilesets.
**		@note It is planned to change this to support more and 
**		better tilesets.
**
**	UnitType::SpriteFile
**
**		Path file name of shadow sprite file for the different tilesets.
**
**	UnitType::Width UnitType::Height
**
**		Size of a sprite frame in pixels. All frames of a sprite have
**		the same size. Also all sprites (tilesets) must have the same
**		size.
**
**	UnitType::ShadowWidth UnitType::ShadowHeight
**
**		Size of a shadow sprite frame in pixels. All frames of a sprite
**		have the same size. Also all sprites (tilesets) must have the
**		same size.
**
**	UnitType::Animations
**
**		Animation scripts for the different actions. Currently the
**		animations still, move, attack and die are supported.
**		@see Animations @see _animations_
**		@see Animation @see _animation_
**
**	UnitType::Icon
**
**		Icon to display for this unit-type. Contains configuration and
**		run time variable.
**		@note This icon can be used for training, but isn't used.
**
**	UnitType::Missile
**
**		Configuration and run time variable of the missile weapon.
**		@note It is planned to support more than one weapons.
**		And the sound of the missile should be used as fire sound.
**
**	UnitType::CorpseName
**
**		Corpse unit-type name, should only be used during setup.
**
**	UnitType::CorpseType
**
**		Corpse unit-type pointer, only this should be used during run
**		time. Many unit-types can share the same corpse.
**
**	UnitType::CorpseScript
**
**		Index into corpse animation script. Used if unit-types share
**		the same corpse but have different animations.
**
**	UnitType::_Speed
**
**		Non upgraded movement speed.
**		@note Until now we didn't support speed upgrades.
**
**	FIXME: continue this documentation
**
**	UnitType::Construction
**
**		What is shown in construction phase.
**
**	UnitType::SightRange
**
**		Sight range
**
**	UnitType::_HitPoints
**
**		Maximum hit points
**
**	UnitType::_MaxMana
**
**		Maximum mana points
**
**	UnitType::Magic
**
**		Unit is a mage
**
**	UnitType::_Costs[::MaxCosts]
**
**		How many resources needed
**
**	UnitType::TileWidth
**
**		Tile size on map width
**
**	UnitType::TileHeight
**
**		Tile size on map height
**
**	UnitType::BoxWidth
**
**		Selected box size width
**
**	UnitType::BoxHeight
**
**		Selected box size height
**
**	UnitType::NumDirections
**
**		Number of directions the unit can face
**
**	UnitType::MinAttackRange
**
**		Minimal attack range
**
**
**	UnitType::_AttackRange
**
**		How far can the unit attack
**
**	UnitType::ReactRangeComputer
**
**		Reacts on enemy for computer
**
**	UnitType::ReactRangePerson
**
**		Reacts on enemy for person player
**
**	UnitType::_Armor
**
**		Amount of armor this unit has
**
**	UnitType::Priority
**
**		Priority value / AI Treatment
**
**	UnitType::_BasicDamage
**
**		Basic damage dealt
**
**	UnitType::_PiercingDamage
**
**		Piercing damage dealt
**
**	UnitType::WeaponsUpgradable
**
**		Weapons could be upgraded
**
**	UnitType::ArmorUpgradable
**
**		Armor could be upgraded
**
**	UnitType::UnitType
**
**		Land / fly / naval
**
**		FIXME: original only visual effect, we do more with this!
**
**	UnitType::DecayRate
**
**		Decay rate in 1/6 seconds
**
**	UnitType::AnnoyComputerFactor
**
**		How much this annoys the computer
**
**		FIXME: not used
**
**	UnitType::MouseAction
**
**		Right click action
**
**	UnitType::Points
**
**		How many points you get for unit
**
**	UnitType::CanTarget
**
**		Which units can it attack
**
**	UnitType::LandUnit
**
**		Land animated
**
**	UnitType::AirUnit
**
**		Air animated
**
**	UnitType::SeaUnit
**
**		Sea animated
**
**	UnitType::ExplodeWhenKilled
**
**		Death explosion animated
**
**	UnitType::Critter
**
**		Unit is controlled by nobody
**
**	UnitType::Building
**
**		Building
**
**	UnitType::Submarine
**
**		Is only visible by CanSeeSubmarine
**
**	UnitType::CanSeeSubmarine
**
**		Only this units can see Submarine
**
**	UnitType::CowerWorker
**
**		Is a worker, runs away if attcked
**
**	UnitType::Tanker
**
**		FIXME: used? Can transport oil
**
**	UnitType::Transporter
**
**		Can transport units
**
**	UnitType::GivesOil
**
**		We get here oil
**
**	UnitType::StoresGold
**
**		We can store oil/gold/wood here
**
**	UnitType::Vanishes
**
**		Corpes & destroyed places
**
**	UnitType::GroundAttack
**
**		Can do command ground attack
**
**	UnitType::IsUndead
**
**		Unit is already dead
**
**	UnitType::ShoreBuilding
**
**		Building must be build on coast
**
**	UnitType::CanCastSpell
**
**		Unit is able to use spells
**
**	UnitType::StoresWood
**
**		We can store wood here
**
**	UnitType::CanAttack
**
**		FIXME: docu
**
**	UnitType::Tower
**
**		FIXME: docu
**
**	UnitType::OilPatch
**
**		FIXME: docu
**
**	UnitType::GoldMine
**
**		FIXME: docu
**
**	UnitType::Hero
**
**		FIXME: docu
**
**	UnitType::StoresOil
**
**		We can store oil here
**
**	UnitType::Volatile
**
**		Invisiblity/unholy armor kills unit
**
**	UnitType::CowerMage
**
**		FIXME: docu
**
**	UnitType::Organic
**
**		Organic can be healed
**
**	UnitType::SelectableByRectangle
**
**		Selectable with mouse rectangle
**
**	UnitType::Teleporter
**
**		Can teleport other units.
**
**	UnitType::Sound
**
**		Sounds for events
**
**	UnitType::Weapon
**
**		Currently sound for weapon
**
**	FIXME: temporary solution
**
**	UnitType::Supply
**
**		Food supply
**
**	UnitType::Demand
**
**		Food demand
**
**	UnitType::ImproveIncomes[::MaxCosts]
**
**		Gives the player an improved income.
**
**	UnitType::FieldFlags
**
**		Flags that are set, if an unit enters a map field or cleared, if
**		an unit leaves a map field.
**
**	UnitType::MovementMask
**
**		Movement mask, this value is and'ed to the map field flags, to
**		see if an unit can enter or placed on the map field.
**
**	UnitType::Stats[::PlayerMax]
**
**		Unit status for each player
**		FIXME: This stats should? be moved into the player struct
**
**	UnitType::Type
**
**		Type as number
**		FIXME: Should us a general name f.e. Slot here?
**
**	UnitType::Property
**
**		CCL property storage
**
**	UnitType::Sprite
**
**		Sprite images
**
**	UnitType::ShadowSprite
**
**		Shadow sprite images
*/

/*----------------------------------------------------------------------------
--	Includes
----------------------------------------------------------------------------*/

#include "video.h"
#include "icons.h"
#include "sound_id.h"
#include "unitsound.h"
#include "upgrade_structs.h"
#include "construct.h"

#ifndef __STRUCT_MISSILETYPE__
#define __STRUCT_MISSILETYPE__
typedef struct _missile_type_ MissileType;         /// Missile-type typedef
#endif

/*----------------------------------------------------------------------------
--	Declarations
----------------------------------------------------------------------------*/

/**
**	Defines the animation for different actions.
*/
typedef struct _animation_ {
    unsigned char	Flags;		/// Flags for actions
    signed char		Pixel;		/// Change the position in pixels
    unsigned char	Sleep;		/// Wait for next animation
    int			Frame;		/// Sprite-frame to display
} Animation;

#define AnimationRestart	1	/// Restart animation
#define AnimationReset		2	/// Animation could here be aborted
#define AnimationSound		4	/// Play sound
#define AnimationMissile	8	/// Fire projectil
#define AnimationEnd		0x80	/// Animation end in memory

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
**
**	@todo Shouldn't I move this into missle.h?
*/
typedef struct _missile_config_ {
    char*	Name;			/// Config missile name
    MissileType*Missile;		/// Identifier to use to run time
} MissileConfig;

/**
**	Typedef of base structure of unit-type
*/
typedef struct _unit_type_ UnitType;

    /// Base structure of unit-type
struct _unit_type_ {
    const void*	OType;			/// Object type (future extensions)

    char*	Ident;			/// Identifier
    char*	Name;			/// Pretty name shown from the engine
    char*	SameSprite;		/// Unit-type shared sprites
    char*	File[TilesetMax];	/// Sprite files
    char*	ShadowFile;		/// Shadow file

    int		Width;			/// Sprite width
    int		Height;			/// Sprite height
    int		ShadowWidth;		/// Shadow sprite width
    int		ShadowHeight;		/// Shadow sprite height

    Animations*	Animations;		/// Animation scripts

    IconConfig	Icon;			/// Icon to display for this unit
    MissileConfig Missile;		/// Missile weapon

    char*	CorpseName;		/// Corpse type name
    UnitType*	CorpseType;		/// Corpse unit-type
    int		CorpseScript;		/// Corpse script start

    int		_Speed;			/// Movement speed

// this is taken from the UDTA section
    Construction*Construction;		/// What is shown in construction phase
    int		_SightRange;		/// Sight range
    unsigned	_HitPoints;		/// Maximum hit points
    int		_MaxMana;		/// Maximum mana points
    // FIXME: only flag
    int		Magic;			/// Unit can cast spells

    int		_Costs[MaxCosts];	/// How many resources needed

    int		TileWidth;		/// Tile size on map width
    int		TileHeight;		/// Tile size on map height
    int		BoxWidth;		/// Selected box size width
    int		BoxHeight;		/// Selected box size height
    int		NumDirections;		/// Number of directions unit can face
    int		MinAttackRange;		/// Minimal attack range
    int		_AttackRange;		/// How far can the unit attack
    int		ReactRangeComputer;	/// Reacts on enemy for computer
    int		ReactRangePerson;	/// Reacts on enemy for person player
    int		_Armor;			/// Amount of armor this unit has
    int		Priority;		/// Priority value / AI Treatment
    int		_BasicDamage;		/// Basic damage dealt
    int		_PiercingDamage;	/// Piercing damage dealt
    int		WeaponsUpgradable;	/// Weapons could be upgraded
    int		ArmorUpgradable;	/// Armor could be upgraded
    // FIXME: original only visual effect, we do more with this!
    enum {
	UnitTypeLand,			/// Unit lives on land
	UnitTypeFly,			/// Unit lives in air
	UnitTypeNaval,			/// Unit lives on water
    }		UnitType;		/// Land / fly / naval
    int		DecayRate;		/// Decay rate in 1/6 seconds
    // FIXME: not used
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
    int		CanTarget;		/// Which units can it attack
#define CanTargetLand	1			/// Can attack land units
#define CanTargetSea	2			/// Can attack sea units
#define CanTargetAir	4			/// Can attack air units

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
    unsigned Transporter : 1;		/// Can transport units
    unsigned GivesOil : 1;		/// We get here oil
    unsigned StoresGold : 1;		/// We can store oil/gold/wood here
    unsigned Vanishes : 1;		/// Corpes & destroyed places
    unsigned GroundAttack : 1;		/// Can do command ground attack
    unsigned IsUndead : 1;		/// Unit is already dead
    unsigned ShoreBuilding : 1;		/// Building must be build on coast
    unsigned CanCastSpell : 1;		/// Unit is able to use spells
    unsigned StoresWood : 1;		/// We can store wood here
    unsigned CanAttack : 1;		/// Unit can attack
    unsigned Tower : 1;			/// Unit can attack, but not move
    unsigned OilPatch : 1;		/// Platform can be build here
    unsigned GoldMine : 1;		/// Gold can be collected here
    unsigned Hero : 1;			/// Is hero only used for triggers 
    unsigned StoresOil : 1;		/// We can store oil here
    unsigned Volatile : 1;		/// Invisiblity/unholy armor kills unit
    unsigned CowerMage : 1;		/// FIXME: docu
    unsigned Organic : 1;		/// Organic can be healed

    unsigned SelectableByRectangle : 1;	/// Selectable with mouse rectangle
    unsigned Teleporter : 1;		/// Can teleport other units.

    UnitSound Sound;			/// Sounds for events
    // FIXME: temporary solution
    WeaponSound Weapon;                 /// Currently sound for weapon

    unsigned	Supply;			/// Food supply
    unsigned	Demand;			/// Food demand

// --- FILLED UP ---

    unsigned	ImproveIncomes[MaxCosts];/// Gives player an improved income

    unsigned	FieldFlags;		/// Unit map field flags
    unsigned	MovementMask;		/// Unit check this map flags for move

	// FIXME: This stats should? be moved into the player struct
    UnitStats Stats[PlayerMax];		/// Unit status for each player

	// FIXME: Should us a general name f.e. Slot here?
    unsigned	Type;			/// Type as number

    void*	Property;		/// CCL property storage

    Graphic*	Sprite;			/// Sprite images
    Graphic*	ShadowSprite;		/// Shadow sprite image
};

    // FIXME: ARI: should be dynamic (ccl..), JOHNS: Pud only supports 255.
    /// How many unit-types are currently supported
#define UnitTypeMax	0xFF

/*----------------------------------------------------------------------------
--	Variables
----------------------------------------------------------------------------*/

extern const char UnitTypeType[];	/// Unit-type type
extern UnitType* UnitTypes;		/// All unit-types
extern int NumUnitTypes;		/// Number of unit-types made

// FIXME: this hardcoded unit-types must be removed!!
extern UnitType*UnitTypeOilPatch;	/// Oil patch unit-type pointer
extern UnitType*UnitTypeGoldMine;	/// Gold-mine unit-type pointer
extern UnitType*UnitTypeHumanTanker;	/// Orc tanker unit-type pointer
extern UnitType*UnitTypeOrcTanker;	/// Human tanker unit-type pointer
extern UnitType*UnitTypeHumanTankerFull;/// Orc tanker full unit-type pointer
extern UnitType*UnitTypeOrcTankerFull;	/// Human tanker full unit-type pointer
extern UnitType*UnitTypeHumanWorker;	/// Human worker
extern UnitType*UnitTypeOrcWorker;	/// Orc worker
extern UnitType*UnitTypeHumanWorkerWithGold;	/// Human worker with gold
extern UnitType*UnitTypeOrcWorkerWithGold;	/// Orc worker with gold
extern UnitType*UnitTypeHumanWorkerWithWood;	/// Human worker with wood
extern UnitType*UnitTypeOrcWorkerWithWood;	/// Orc worker with wood
extern UnitType*UnitTypeHumanWall;	/// Human wall
extern UnitType*UnitTypeOrcWall;	/// Orc wall
extern UnitType*UnitTypeCritter;	/// Critter unit-type pointer
extern UnitType*UnitTypeBerserker;	/// Berserker for berserker regeneration

extern char** UnitTypeWcNames;		/// Mapping wc-number 2 symbol

/*----------------------------------------------------------------------------
--	Functions
----------------------------------------------------------------------------*/

extern void UnitTypeCclRegister(void);	/// Register ccl features

extern void UpdateStats(void);		/// Update unit stats
extern void ParsePudUDTA(const char*,int); /// Parse pud udta table
extern UnitType* UnitTypeByIdent(const char*);	/// Get unit-type by ident
extern UnitType* UnitTypeByWcNum(unsigned);	/// Get unit-type by wc number

    /// Get the animations structure by ident
extern Animations* AnimationsByIdent(const char* ident);

extern void SaveUnitTypes(FILE* file);	/// Save the unit-type table
extern UnitType* NewUnitTypeSlot(char*);/// Allocate an empty unit-type slot
    /// Draw the sprite frame of unit-type
extern void DrawUnitType(const UnitType* type,int frame,int x,int y);

extern void InitUnitTypes(void);	/// Init unit-type table
extern void LoadUnitTypes(void);	/// Load the unit-type data
extern void CleanUnitTypes(void);	/// Cleanup unit-type module

//@}

#endif	// !__UNITTYPE_H__
