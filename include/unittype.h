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
/**@name unittype.h	-	The unit types headerfile. */
/*
**	(c) Copyright 1998-2000 by Lutz Sammer
**
**	$Id$
*/

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

//#define MaxUnitTypes	300		/// maximal number of unit types

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

#if 0
#define CorpseNone	0	/// Unit has no corpse
#define CorpseHuman	1	/// Unit has a human corpse
#define CorpseOrc	2	/// Unit has a orc corpse
#define CorpseShip	3	/// Unit has a ship corpse
#define CorpseLandSite	4	/// Unit has a land corpse
#define CorpseWaterSite	5	/// Unit has a water corpse
#endif

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
    void*	OType;			/// Object type (future extensions)

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
    int		UnitType;		/// land / fly / naval (visual effect)
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
    unsigned Critter : 1;		///
    unsigned Building : 1;		///
    unsigned Submarine : 1;		///
    unsigned CanSeeSubmarine : 1;	///
    unsigned CowerPeon : 1;		///
    unsigned Tanker : 1;		///
    unsigned Transporter : 1;		///
    unsigned GivesOil : 1;		///
    unsigned StoresGold : 1;		///
    unsigned Vanishes : 1;		/// Corpes & destroyed places.
    unsigned GroundAttack : 1;		///
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
    unsigned Explodes : 1;		/// invisiblity/unholy armor kills unit
    unsigned CowerMage : 1;		///
    unsigned Organic : 1;		/// organic

    unsigned SelectableByRectangle : 1;	/// selectable with mouse rectangle

    //ButtonConfig* Buttons;		/// buttons of this unit-type
    UnitSound Sound;			/// sounds for events
    // FIXME: temporary solution
    WeaponSound Weapon;                 /// currently sound for weapon

// --- FILLED UP ---

	// FIXME: Should be included from "player.h", but recursive includes
#define PlayerMax		16	/// maximal players supported
	// FIXME: This stats should? be moved into the player struct
    UnitStats Stats[PlayerMax];		/// Unit status for each player

    unsigned Type;			/// Type as number

    void*	Property;		/// CCL property storage

    RleSprite*	RleSprite;		/// sprite images
};

#if 1
/*
**	Names for the unit-type table slots as used in puds.
**
**	NOTE: Would be soon global removed.
*/
#define	UnitFootman		0x00
#define	UnitGrunt		0x01
#define UnitPeasant		0x02
#define UnitPeon		0x03
#define UnitBallista		0x04
#define UnitCatapult		0x05
#define UnitKnight		0x06
#define UnitOgre		0x07
#define UnitArcher		0x08
#define UnitAxethrower		0x09
#define UnitMage		0x0A
#define UnitDeathKnight		0x0B
#define UnitPaladin		0x0C
#define UnitOgreMage		0x0D
#define UnitDwarves		0x0E
#define UnitGoblinSappers	0x0F
#define UnitAttackPeasant	0x10
#define UnitAttackPeon		0x11
#define UnitRanger		0x12
#define UnitBerserker		0x13
#define UnitAlleria		0x14
#define UnitTeronGorefiend	0x15
#define UnitKurdanAndSky_ree	0x16
#define UnitDentarg		0x17
#define UnitKhadgar		0x18
#define UnitGromHellscream	0x19
#define UnitTankerHuman		0x1A
#define UnitTankerOrc		0x1B
#define UnitTransportHuman	0x1C
#define UnitTransportOrc	0x1D
#define UnitElvenDestroyer	0x1E
#define UnitTrollDestroyer	0x1F
#define UnitBattleship		0x20
#define UnitJuggernaught	0x21
#define UnitNothing		0x22
#define UnitDeathwing		0x23
#define UnitNothing1		0x24
#define UnitNothing2		0x25
#define UnitGnomishSubmarine	0x26
#define UnitGiantTurtle		0x27
#define UnitGnomishFlyingMachine 0x28
#define UnitGoblinZeppelin	0x29
#define UnitGryphonRider	0x2A
#define UnitDragon		0x2B
#define UnitTuralyon		0x2C
#define UnitEyeOfKilrogg	0x2D
#define UnitDanath		0x2E
#define UnitKorgathBladefist	0x2F
#define UnitNothing3		0x30
#define UnitCho_gall		0x31
#define UnitLothar		0x32
#define UnitGul_dan		0x33
#define UnitUtherLightbringer	0x34
#define UnitZuljin		0x35
#define UnitNothing4		0x36
#define UnitSkeleton		0x37
#define UnitDaemon		0x38
#define UnitCritter		0x39
#define UnitFarm		0x3A
#define UnitPigFarm		0x3B
#define UnitBarracksHuman	0x3C
#define UnitBarracksOrc		0x3D
#define UnitChurch		0x3E
#define UnitAltarOfStorms	0x3F
#define UnitScoutTowerHuman	0x40
#define UnitScoutTowerOrc	0x41
#define UnitStables		0x42
#define UnitOgreMound		0x43
#define UnitGnomishInventor	0x44
#define UnitGoblinAlchemist	0x45
#define UnitGryphonAviary	0x46
#define UnitDragonRoost		0x47
#define UnitShipyardHuman	0x48
#define UnitShipyardOrc		0x49
#define UnitTownHall		0x4A
#define UnitGreatHall		0x4B
#define UnitElvenLumberMill	0x4C
#define UnitTrollLumberMill	0x4D
#define UnitFoundryHuman	0x4E
#define UnitFoundryOrc		0x4F
#define UnitMageTower		0x50
#define UnitTempleOfTheDamned	0x51
#define UnitBlacksmithHuman	0x52
#define UnitBlacksmithOrc	0x53
#define UnitRefineryHuman	0x54
#define UnitRefineryOrc		0x55
#define UnitOilPlatformHuman	0x56
#define UnitOilPlatformOrc	0x57
#define UnitKeep		0x58
#define UnitStronghold		0x59
#define UnitCastle		0x5A
#define UnitFortress		0x5B
#define UnitGoldMine		0x5C
#define UnitOilPatch		0x5D
#define UnitStartLocationHuman	0x5E
#define UnitStartLocationOrc	0x5F
#define UnitGuardTowerHuman	0x60
#define UnitGuardTowerOrc	0x61
#define UnitCannonTowerHuman	0x62
#define UnitCannonTowerOrc	0x63
#define UnitCircleofPower	0x64
#define UnitDarkPortal		0x65
#define UnitRunestone		0x66
#define UnitWallHuman		0x67
#define UnitWallOrc		0x68
#define UnitDeadBody		0x69
#define Unit1x1DestroyedPlace	0x6A
#define Unit2x2DestroyedPlace	0x6B
#define Unit3x3DestroyedPlace	0x6C
#define Unit4x4DestroyedPlace	0x6D

#define UnitTypeMax		0x6E

//	This are internal used unit-types:

#define UnitPeasantWithGold	0x6E
#define UnitPeonWithGold	0x6F
#define UnitPeasantWithWood	0x70
#define UnitPeonWithWood	0x71
#define UnitTankerHumanFull	0x72
#define UnitTankerOrcFull	0x73

#endif

#define UnitTypeInternalMax	0x74

#define WC_StartLocationHuman	0x5E
#define WC_StartLocationOrc	0x5F

/*----------------------------------------------------------------------------
--	Variables
----------------------------------------------------------------------------*/

extern char	UnitTypeType[];			/// unit type type
// FIXME: this limit must be removed!
extern UnitType UnitTypes[UnitTypeInternalMax];	/// all unit types

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
extern void InitUnitTypes(void);	/// Init unit-type table
extern void LoadUnitSprites(void);	/// Load all unit-type sprites

//@}

#endif	// !__UNITTYPE_H__
