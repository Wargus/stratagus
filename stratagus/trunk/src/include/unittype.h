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
/**@name unittype.h - The unit-types headerfile. */
//
//      (c) Copyright 1998-2004 by Lutz Sammer and Jimmy Salmon
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

#ifndef __UNITTYPE_H__
#define __UNITTYPE_H__

//@{

/*----------------------------------------------------------------------------
-- Documentation
----------------------------------------------------------------------------*/

struct _CL_File_;
struct lua_State;

/**
**  @struct _unit_type_ unittype.h
**
**  \#include "unittype.h"
**
**  typedef struct _unit_type_ UnitType;
**
**  This structure contains the informations that are shared between all
**  units of the same type and determins if an unit is a building,
**  a person, ...
**
**  The unit-type structure members:
**
**  UnitType::Ident
**
**    Unique identifier of the unit-type, used to reference it in
**    config files and during startup. As convention they start with
**    "unit-" fe. "unit-farm".
**  @note Don't use this member in game, use instead the pointer
**  to this structure. See UnitTypeByIdent().
**
**  UnitType::Name
**
**    Pretty name shown by the engine. The name should be shorter
**    than 17 characters and no word can be longer than 8 characters.
**
**  UnitType::File[::TilesetMax]
**
**    Path file name of sprite files for the different tilesets.
**  @note It is planned to change this to support more and
**  better tilesets.
**
**  UnitType::SpriteFile
**
**    Path file name of shadow sprite file for the different tilesets.
**
**  UnitType::DrawLevel
**
**    The Level/Order to draw this type of unit in. 0-255 usually.
**
**  UnitType::Width UnitType::Height
**
**    Size of a sprite frame in pixels. All frames of a sprite have
**    the same size. Also all sprites (tilesets) must have the same
**    size.
**
**  UnitType::ShadowWidth UnitType::ShadowHeight
**
**    Size of a shadow sprite frame in pixels. All frames of a sprite
**    have the same size. Also all sprites (tilesets) must have the
**    same size.
**
**  UnitType::ShadowOffset
**
**    Vertical offset to draw the shadow in pixels.
**
**  UnitType::Animations
**
**    Animation scripts for the different actions. Currently the
**    animations still, move, attack and die are supported.
**  @see Animations @see _animations_
**  @see Animation @see _animation_
**
**  UnitType::Icon
**
**    Icon to display for this unit-type. Contains configuration and
**    run time variable.
**  @note This icon can be used for training, but isn't used.
**
**  UnitType::Missile
**
**    Configuration and run time variable of the missile weapon.
**  @note It is planned to support more than one weapons.
**  And the sound of the missile should be used as fire sound.
**
**  UnitType::Explosion
**
**    Configuration and run time variable of the missile explosion.
**    This is the explosion that happens if unit is set to
**    ExplodeWhenKilled
**
**  UnitType::CorpseName
**
**    Corpse unit-type name, should only be used during setup.
**
**  UnitType::CorpseType
**
**    Corpse unit-type pointer, only this should be used during run
**    time. Many unit-types can share the same corpse.
**
**  UnitType::CorpseScript
**
**    Index into corpse animation script. Used if unit-types share
**    the same corpse but have different animations.
**
**
**  @todo continue this documentation
**
**  UnitType::Construction
**
**    What is shown in construction phase.
**
**  UnitType::SightRange
**
**    Sight range
**
**  UnitType::_HitPoints
**
**    Maximum hit points
**
**  UnitType::_MaxMana
**
**    Maximum mana points
**
**  UnitType::_Costs[::MaxCosts]
**
**    How many resources needed
**
**  UnitType::RepairHP
**
**    The HP given to a unit each cycle it's repaired.
**    If zero, unit cannot be repaired
**
**    UnitType::RepairCosts[::MaxCosts]
**
**    Costs per repair cycle to fix a unit.
**
**  UnitType::TileWidth
**
**    Tile size on map width
**
**  UnitType::TileHeight
**
**    Tile size on map height
**
**  UnitType::BoxWidth
**
**    Selected box size width
**
**  UnitType::BoxHeight
**
**    Selected box size height
**
**  UnitType::NumDirections
**
**    Number of directions the unit can face
**
**  UnitType::MinAttackRange
**
**    Minimal attack range
**
**
**  UnitType::_AttackRange
**
**    How far can the unit attack
**
**  UnitType::ReactRangeComputer
**
**    Reacts on enemy for computer
**
**  UnitType::ReactRangePerson
**
**    Reacts on enemy for person player
**
**  UnitType::_Armor
**
**    Amount of armor this unit has
**
**  UnitType::Priority
**
**    Priority value / AI Treatment
**
**  UnitType::_BasicDamage
**
**    Basic damage dealt
**
**  UnitType::_PiercingDamage
**
**    Piercing damage dealt
**
**  UnitType::_RegenerationRate
**
**    Regeneration rate in HP per second
**
**  UnitType::BurnPercent
**
**    The burning limit in percents. If the unit has lees than
**    this it will start to burn.
**
**  UnitType::BurnDamageRate
**
**    Burn rate in HP per second
**
**  UnitType::UnitType
**
**    Land / fly / naval
**
**  @note original only visual effect, we do more with this!
**
**  UnitType::DecayRate
**
**    Decay rate in 1/6 seconds
**
**  UnitType::AnnoyComputerFactor
**
**    How much this annoys the computer
**
**  @todo not used
**
**  UnitType::MouseAction
**
**    Right click action
**
**  UnitType::Points
**
**    How many points you get for unit. Used in the final score table.
**
**  UnitType::CanTarget
**
**    Which units can it attack
**
**  Unit::Revealer
**
**    A special unit used to reveal the map for a time. This unit
**    has active sight even when Removed. It's used for Reveal map
**    type of spells.
**
**  UnitType::LandUnit
**
**    Land animated
**
**  UnitType::AirUnit
**
**    Air animated
**
**  UnitType::SeaUnit
**
**    Sea animated
**
**  UnitType::ExplodeWhenKilled
**
**    Death explosion animated
**
**  UnitType::RandomMovementProbability
**
**    When the unit is idle this is the probability that it will
**    take a step in a random direction, in percents.
**
**  UnitType::ClicksToExplode
**
**    If this is non-zero, then after that many clicks the unit will
**    commit suicide. Doesn't work with resource workers/resources.
**
**  UnitType::Building
**
**    Unit is a Building
**
**  UnitType::VisibleUnderFog
**
**    Unit is visible under fog of war.
**
**  UnitType::PermanentCloak
**
**    Unit is permanently cloaked.
**
**  UnitType::DetectCloak
**
**    These units can detect Cloaked units.
**
**  UnitType::Coward
**
**    Unit is a coward, and acts defensively. it will not attack
**    at will and auto-casters will not buff it(bloodlust).
**
**  UnitType::Transporter
**
**    Can transport units
**
**  UnitType::AttackFromTransporter
**
**    Units inside this transporter can attack with missiles.
**
**  UnitType::MaxOnBoard
**
**    Maximum units on board (for transporters), and resources
**
**  UnitType::StartingResources
**    Amount of Resources a unit has when It's Built
**
**  UnitType::GivesResource
**
**    This equals to the resource Id of the resource given
**    or 0 (TimeCost) for other buildings.
**
**  UnitType::CanHarvest
**
**    Resource can be harvested. It's false for things like
**    oil patches.
**  @todo crappy name.
**
**  UnitType::Harvester
**
**    Unit is a resource worker. Faster than examining ResInfo
**
**  UnitType::ResInfo[::MaxCosts]
**
**    Information about resource harvesting. If NULL, it can't
**    harvest it.
**
**  UnitType::NeutralMinimapColorRGB
**
**    Says what color a unit will have when it's neutral and
**    is displayed on the minimap.
**
**  UnitType::CanStore[::MaxCosts]
**
**    What resource types we can store here.
**
**  UnitType::Vanishes
**
**    Corpes & destroyed places
**
**  UnitType::GroundAttack
**
**    Can do command ground attack
**
**  UnitType::ShoreBuilding
**
**    Building must be build on coast
**
**  UnitType::CanCastSpell
**
**    Unit is able to use spells
**
**  UnitType::CanAttack
**
**    Unit is able to attack.
**
**  UnitType::RepairRange
**
**    Unit can repair buildings. It will use the actack animation.
**    It will heal 4 points for every repair cycle, and cost 1 of
**    each resource, alternatively(1 cycle wood, 1 cycle gold)
**  @todo The above should be more configurable.
**    If units have a repair range, they can repair, and this is the
**    distance.
**
**  UnitType::BuilderOutside
**
**    Only valid for buildings. When building the worker will
**    remain outside inside the building.
**
**  @warning Workers that can build buildings with the
**  @warning BuilderOutside flag must have the CanRepair flag.
**
**  UnitType::BuilderLost
**
**    Only valid for buildings without the BuilderOutside flag.
**    The worker is lost when the building is completed.
**
**  UnitType::SelectableByRectangle
**
**    Selectable with mouse rectangle
**
**    UnitType::Teleporter
**
**    Can teleport other units.
**
**  UnitType::Sound
**
**    Sounds for events
**
**  UnitType::Weapon
**
**    Currently sound for weapon
**
**  @todo temporary solution
**
**  UnitType::Supply
**
**    How much food does this unit supply.
**
**  UnitType::Demand
**
**    Food demand
**
**  UnitType::ImproveIncomes[::MaxCosts]
**
**    Gives the player an improved income.
**
**  UnitType::FieldFlags
**
**    Flags that are set, if an unit enters a map field or cleared, if
**    an unit leaves a map field.
**
**  UnitType::MovementMask
**
**    Movement mask, this value is and'ed to the map field flags, to
**    see if an unit can enter or placed on the map field.
**
**  UnitType::Stats[::PlayerMax]
**
**    Unit status for each player
**  @todo This stats should? be moved into the player struct
**
**  UnitType::Type
**
**    Type as number
**  @todo Should us a general name f.e. Slot here?
**
**  UnitType::Sprite
**
**    Sprite images
**
**  UnitType::ShadowSprite
**
**    Shadow sprite images
**
**  UnitType::PlayerColorSprite
**
**    Sprite images of the player colors.  This image is drawn
**    over UnitType::Sprite.  Used with OpenGL only.
**
**
*/
/**
**
**  @struct _resource_info_ unittype.h
**
** \#include "unittype.h"
**
**  typedef struct _resource_info_ ResourceInfo;
**
**    This struct contains information about how a unit will harvest a resource.
**
**  ResourceInfo::FileWhenLoaded
**
**    The harvester's animation file will change when it's loaded.
**
**  ResourceInfo::FileWhenEmpty;
**
**    The harvester's animation file will change when it's empty.
**    The standard animation is used only when building/repairing.
**
**
**  ResourceInfo::HarvestFromOutside
**
**    Unit will harvest from the outside. The unit will use it's
**    Attack animation (seems it turned into a generic Action anim.)
**
**  ResourceInfo::ResourceId
**
**    The resource this is for. Mostly redundant.
**
**  ResourceInfo::FinalResource
**
**    The resource is converted to this at the depot. Usefull for
**    a fisherman who harvests fish, but it all turns to food at the
**    depot.
**
**  ResourceInfo::WaitAtResource
**
**    Cycles the unit waits while inside a resource.
**
**  ResourceInfo::ResourceStep
**
**    The unit makes so-caled mining cycles. Each mining cycle
**    it does some sort of animation and gains ResourceStep
**    resources. You can stop after any number of steps.
**    when the quantity in the harvester reaches the maximum
**    (ResourceCapacity) it will return home. I this is 0 then
**    it's considered infinity, and ResourceCapacity will now
**    be the limit.
**
**  ResourceInfo::ResourceCapacity
**
**    Maximum amount of resources a harvester can carry. The
**    actual amount can be modified while unloading.
**
**  ResourceInfo::LoseResources
**
**    Special lossy behaviour for loaded harvesters. Harvesters
**    with loads other than 0 and ResourceCapacity will lose their
**    cargo on any new order.
**
**  ResourceInfo::WaitAtDepot
**
**    Cycles the unit waits while inside the depot to unload.
**
**  ResourceInfo::TerrainHarvester
**
**    The unit will harvest terrain. For now this only works
**    for wood. maybe it could be made to work for rocks, but
**    more than that requires a tileset rewrite.
**  @todo more configurable.
**
*/

/*----------------------------------------------------------------------------
--  Includes
----------------------------------------------------------------------------*/

#include "upgrade_structs.h"
#include "util.h"
#include "unitsound.h"

/*----------------------------------------------------------------------------
--  Declarations
----------------------------------------------------------------------------*/

struct _graphic_;
struct _construction_;

/**
**  Defines the animation for different actions.
*/
typedef struct _animation_ {
	unsigned char Flags;            ///< Flags for actions
	signed char   Pixel;            ///< Change the position in pixels
	unsigned char Sleep;            ///< Wait for next animation
	int           Frame;            ///< Sprite-frame to display
} Animation;

#define AnimationRestart  1         ///< Restart animation
#define AnimationReset    2         ///< Animation could here be aborted
#define AnimationSound    4         ///< Play sound
#define AnimationMissile  8         ///< Fire projectil
#define AnimationEnd      0x80      ///< Animation end in memory

/**
**  Define all animations scripts of an unittype.
*/
typedef struct __animations__ {
	Animation*  Still;              ///< Standing still
	Animation*  Move;               ///< Unit moving
	Animation*  Attack;             ///< Unit attacking/working
	Animation*  Repair;             ///< Unit repairing
	Animation*  Harvest[MaxCosts];  ///< Unit harvesting
	Animation*  Die;                ///< Unit dying
} Animations;

#define ANIMATIONS_MAXANIM 1024

	/// Hash table of all the animations
typedef hashtable(Animations*, ANIMATIONS_MAXANIM) _AnimationsHash;
extern _AnimationsHash AnimationsHash;

/**
**  Missile type definition (used in config tables)
**
**  @todo Move this to missle.h?
*/
typedef struct _missile_config_ {
	char*        Name;              ///< Config missile name
	MissileType* Missile;           ///< Identifier to use to run time
} MissileConfig;

typedef struct _resource_info_ {
	char*    FileWhenLoaded;        ///< Change the graphic when the unit is loaded.
	char*    FileWhenEmpty;         ///< Change the graphic when the unit is empty.
	unsigned HarvestFromOutside;    ///< Unit harvests without entering the building.
	unsigned WaitAtResource;        ///< Cycles the unit waits while mining.
	unsigned ResourceStep;          ///< Resources the unit gains per mining cycle.
	int      ResourceCapacity;      ///< Max amount of resources to carry.
	unsigned WaitAtDepot;           ///< Cycles the unit waits while returning.
	unsigned ResourceId;            ///< Id of the resource harvested. Redundant.
	unsigned FinalResource;         ///< Convert resource when delivered.
	unsigned TerrainHarvester;      ///< Unit will harvest terrain(wood only for now).
	unsigned LoseResources;         ///< The unit will lose it's resource when distracted.
	//  Runtime info:
	struct _graphic_* SpriteWhenLoaded; ///< The graphic corresponding to FileWhenLoaded.
	struct _graphic_* SpriteWhenEmpty;  ///< The graphic corresponding to FileWhenEmpty
#ifdef USE_OPENGL
	struct _graphic_* PlayerColorSpriteWhenLoaded[PlayerMax]; ///< Sprites with player colors
	struct _graphic_* PlayerColorSpriteWhenEmpty[PlayerMax];  ///< Sprites with player colors
#endif
} ResourceInfo;

/**
**  User defined variable type.
**
**  It is used to define variables and use it after
**  to manage magic, energy, shield or other stuff.
*/
typedef struct _variable_type_ {
	int Max;           ///< Maximum for the variable. (Assume min is 0.)
	int Value;         ///< Current (or initial) value of the variable (or initial value).
	char Increase;     ///< Number to increase(decrease) Value by second.
	char Enable;       ///< True if the unit doesn't have this variable. (f.e shield)
} VariableType;

// Index for variable already defined.
#define HP_INDEX              0
#define BUILD_INDEX           1
#define MANA_INDEX            2
#define TRANSPORT_INDEX       3
#define RESEARCH_INDEX        4
#define TRAINING_INDEX        5
#define UPGRADINGTO_INDEX     6
#define GIVERESOURCE_INDEX    7
#define CARRYRESOURCE_INDEX   8
#define XP_INDEX              9
#define LEVEL_INDEX          10
#define KILL_INDEX           11
#define SUPPLY_INDEX         12
#define DEMAND_INDEX         13
#define ARMOR_INDEX          14
#define SIGHTRANGE_INDEX     15
#define ATTACKRANGE_INDEX    16
#define PIERCINGDAMAGE_INDEX 17
#define BASICDAMAGE_INDEX    18
#define DAMAGE_INDEX         19
#define EXTRADAMAGE_INDEX    20
#define POSX_INDEX           21
#define POSY_INDEX           22
#define SLOT_INDEX           23

#define NVARALREADYDEFINED SLOT_INDEX + 1 // Hardcoded variables

struct _decovartype_;
struct _unit_;

typedef void DrawDecoFunc(int x, int y, const struct _unit_* unit,
	const struct _decovartype_* Deco);

/**
**  Decoration for userdefined variable.
**
**    It is used to show variables graphicly.
**  @todo add more stuff in this struct.
*/
typedef struct _decovartype_ {
	int Index;                  ///< Index of the variable. @see DefineVariables

	int OffsetX;                ///< Offset in X coord.
	int OffsetY;                ///< Offset in Y coord.

	int OffsetXPercent;         ///< Percent offset (TileWidth) in X coord.
	int OffsetYPercent;         ///< Percent offset (TileHeight) in Y coord.

	char IsCenteredInX;         ///< if true, use center of deco instead of left border
	char IsCenteredInY;         ///< if true, use center of deco instead of upper border

	char ShowIfNotEnable;       ///< if false, Show only if var is enable
	char ShowWhenNull;          ///< if false, don't show if var is null (F.E poison)
	char HideHalf;              ///< if true, don't show when 0 < var < max.
	char ShowWhenMax;           ///< if false, don't show if var is to max. (Like mana)
	char ShowOnlySelected;      ///< if true, show only for selected units.

	char HideNeutral;           ///< if true, don't show for neutral unit.
	char HideAllied;            ///< if true, don't show for allied unit. (but show own units)
	char ShowOpponent;          ///< if true, show for opponent unit.

	DrawDecoFunc* f;            ///< function to draw the decorations.
	union {
		struct {
			char IsVertical;            ///< if true, vertical bar, else horizontal.
			char SEToNW;                ///< (SouthEastToNorthWest), if false value 0 is on the left or up of the bar.
			int Height;                 ///< Height of the bar.
			int Width;                  ///< Width of the bar.
			char ShowFullBackground;    ///< if true, show background like value equal to max.
			char BorderSize;            ///< Size of the border, 0 for no border.
// FIXME color depend of percent (red, Orange, Yellow, Green...)
			Uint32 Color;               ///< Color of bar.
			Uint32 BColor;              ///< Color of background.
		} Bar; ///< Use for Horizontal and vertical Bar.

		struct {
			int Font;                   ///< Font to use to display value.
// FIXME : Add Color, format
		} Text; ///< Use for info text.

		struct {
			char NSprite;                ///< Index of nuber. (@see DefineSprites and @see GetSpriteIndex)
// FIXME Sprite info. better way ?
		} SpriteBar; ///< Use for extra info for sprite. Sprite contains frame from full (left)to empty state (right).

		struct {
// FIXME Sprite info. and Replace n with more appropriate var.
			int n;                      ///< identifiant in SpellSprite
		} StaticSprite; ///< use to show specific frame in a sprite.
// FIXME : other method here.
	} Data;         ///< More Datas, depend of showing method

} DecoVarType;

/**
**  Typedef of base structure of unit-type
*/
typedef struct _unit_type_ UnitType;

typedef struct _building_restrictions_ BuildRestriction;

struct _building_restrictions_ {
	enum {
		RestrictAddOn,          ///< We are refereing to the following restriction
		RestrictOnTop,
		RestrictDistance,
		RestrictDirection,
		RestrictTiles,
	} RestrictType;

	union {
		struct {
			int OffsetX;         ///< offset from the main building to place this
			int OffsetY;         ///< offset from the main building to place this
			char* ParentName;    ///< building that is unit is an addon too.
			UnitType* Parent;    ///< building that is unit is an addon too.
		} AddOn;

		struct {
			UnitType* Parent;
			char* ParentName;    ///< building that is unit is an addon too.
			int ReplaceOnDie;    ///< recreate the parent on destruction
			int ReplaceOnBuild;  ///< remove the parent, or just build over it.
		} OnTop;
		
		struct {
			int Distance;        ///< distance to build (circle)
			enum {
				Equal,
				NotEqual,
				LessThan,
				LessThanEqual,
				GreaterThan,
				GreaterThanEqual,
			} DistanceType;
			int Except;          ///< all except this building type.
			char* RestrictTypeName; 
			UnitType* RestrictType; 
		} Distance;

		int Direction;           ///< Direction mask up,down,left,right,up-left...

		struct {
			int Number;          ///< All tiles, or just 1-n,  0 is all as you don't have a rule otherwise
			int Mask;            ///< Tile mask required
		} Tiles;
	} Data;
	BuildRestriction* Next;
};
	

/**
** Base structure of unit-type
** @todo n0body: AutoBuildRate not implemented.
*/
struct _unit_type_ {
	char* Ident;                    ///< Identifier
	char* Name;                     ///< Pretty name shown from the engine
	int Slot;                       ///< Type as number
	char* File[TilesetMax];         ///< Sprite files
	char* ShadowFile;               ///< Shadow file

	int Width;                      ///< Sprite width
	int Height;                     ///< Sprite height
	int DrawLevel;                  ///< Level to Draw UnitType at
	int ShadowWidth;                ///< Shadow sprite width
	int ShadowHeight;               ///< Shadow sprite height
	int ShadowOffsetX;              ///< Shadow horizontal offset
	int ShadowOffsetY;              ///< Shadow vertical offset

	Animations* Animations;         ///< Animation scripts

	IconConfig Icon;                ///< Icon to display for this unit
	MissileConfig Missile;          ///< Missile weapon
	MissileConfig Explosion;        ///< Missile for unit explosion

	char* CorpseName;               ///< Corpse type name
	UnitType* CorpseType;           ///< Corpse unit-type
	int CorpseScript;               ///< Corpse script start

	// this is taken from the UDTA section
	struct _construction_* Construction; ///< What is shown in construction phase
	int _SightRange;                ///< Sight range
	int _HitPoints;                 ///< Maximum hit points
	int _MaxMana;                   ///< Maximum mana points

	int _Costs[MaxCosts];           ///< How many resources needed
	int RepairHP;                   ///< Amount of HP per repair
	int RepairCosts[MaxCosts];      ///< How much it costs to repair

	int TileWidth;                  ///< Tile size on map width
	int TileHeight;                 ///< Tile size on map height
	int BoxWidth;                   ///< Selected box size width
	int BoxHeight;                  ///< Selected box size height
	int NumDirections;              ///< Number of directions unit can face
	int MinAttackRange;             ///< Minimal attack range
	int _AttackRange;               ///< How far can the unit attack
	int ReactRangeComputer;         ///< Reacts on enemy for computer
	int ReactRangePerson;           ///< Reacts on enemy for person player
	int _Armor;                     ///< Amount of armor this unit has
	int Priority;                   ///< Priority value / AI Treatment
	int _BasicDamage;               ///< Basic damage dealt
	int _PiercingDamage;            ///< Piercing damage dealt
	int _RegenerationRate;          ///< HP regeneration rate per sec
	int BurnPercent;                ///< Burning percent.
	int BurnDamageRate;             ///< HP burn rate per sec
	int RepairRange;                ///< Units repair range.
	char* CanCastSpell;             ///< Unit is able to use spells.
	char* AutoCastActive;           ///< Default value for autocast.
	int AutoBuildRate;              ///< The rate at which the building builds itself
	int RandomMovementProbability;  ///< Probability to move randomly.
	int ClicksToExplode;            ///< Number of consecutive clicks until unit suicides.
	char* CanTransport;             ///< Can transport units with this flag.
	int MaxOnBoard;                 ///< Number of Transporter slots.
	int StartingResources;          ///< Amount of Resources on build
	/// @note original only visual effect, we do more with this!
	enum {
		UnitTypeLand,               ///< Unit lives on land
		UnitTypeFly,                ///< Unit lives in air
		UnitTypeNaval,              ///< Unit lives on water
	} UnitType;                     ///< Land / fly / naval
	int DecayRate;                  ///< Decay rate in 1/6 seconds
	// TODO: not used
	int AnnoyComputerFactor;        ///< How much this annoys the computer
	int MouseAction;                ///< Right click action
#define MouseActionNone      0      ///< Nothing
#define MouseActionAttack    1      ///< Attack
#define MouseActionMove      2      ///< Move
#define MouseActionHarvest   3      ///< Harvest resources
#define MouseActionSpellCast 5      ///< Cast the first spell known
#define MouseActionSail      6      ///< Sail
	int Points;                     ///< How many points you get for unit
	int CanTarget;                  ///< Which units can it attack
#define CanTargetLand 1             ///< Can attack land units
#define CanTargetSea  2             ///< Can attack sea units
#define CanTargetAir  4             ///< Can attack air units

	unsigned Flip : 1;              ///< Flip image when facing left
	unsigned Revealer : 1;          ///< reveal the fog of war
	unsigned LandUnit : 1;          ///< Land animated
	unsigned AirUnit : 1;           ///< Air animated
	unsigned SeaUnit : 1;           ///< Sea animated
	unsigned ExplodeWhenKilled : 1; ///< Death explosion animated
	unsigned Building : 1;          ///< Building
	unsigned VisibleUnderFog : 1;   ///< Unit is visible under fog of war.
	unsigned PermanentCloak : 1;    ///< Is only visible by CloakDetectors.
	unsigned DetectCloak : 1;       ///< Can see Cloaked units.
	unsigned Coward : 1;            ///< Unit will only attack if instructed.
	unsigned AttackFromTransporter : 1;  ///< Can attack from transporter
	unsigned Vanishes : 1;          ///< Corpes & destroyed places.
	unsigned GroundAttack : 1;      ///< Can do command ground attack.
	unsigned ShoreBuilding : 1;     ///< Building must be build on coast.
	unsigned CanAttack : 1;         ///< Unit can attack.
	unsigned BuilderOutside : 1;    ///< The builder stays outside during the build.
	unsigned BuilderLost : 1;       ///< The builder is lost after the build.
	unsigned CanHarvest : 1;        ///< Resource can be harvested.
	unsigned Harvester : 1;         ///< unit is a resource harvester.
	unsigned char* BoolFlag;        ///< User defined flag. Used for (dis)allow target.
	VariableType* Variable;         ///< Array of user defined variables.
	unsigned char* CanTargetFlag;   ///< Flag needed to target with missile.

	unsigned SelectableByRectangle : 1; ///< Selectable with mouse rectangle.
	unsigned Decoration : 1;            ///< Unit Is a decoration.
	unsigned Teleporter : 1;            ///< Can teleport other units.

	int CanStore[MaxCosts];             ///< Resources that we can store here.
	int GivesResource;                  ///< The resource this unit gives.
	ResourceInfo* ResInfo[MaxCosts];    ///< Resource information.
	BuildRestriction** BuildingRules;   ///< Rules list for building a building.
	SDL_Color NeutralMinimapColorRGB;   ///< Minimap Color for Neutral Units.

	UnitSound Sound;                ///< Sounds for events
	// TODO: temporary solution
	WeaponSound Weapon;             ///< Currently sound for weapon

	int Supply;                     ///< Food supply
	int Demand;                     ///< Food demand

// --- FILLED UP ---

	int ImproveIncomes[MaxCosts];   ///< Gives player an improved income

	unsigned FieldFlags;            ///< Unit map field flags
	unsigned MovementMask;          ///< Unit check this map flags for move

	/// @todo This stats should? be moved into the player struct
	UnitStats Stats[PlayerMax];     ///< Unit status for each player

	struct _graphic_* Sprite;                ///< Sprite images
	struct _graphic_* ShadowSprite;          ///< Shadow sprite image
};

	/// @todo ARI: should be dynamic (lua..), JOHNS: Pud only supports 255.
	/// How many unit-types are currently supported
#define UnitTypeMax 257

/*----------------------------------------------------------------------------
--  Variables
----------------------------------------------------------------------------*/

extern UnitType* UnitTypes[UnitTypeMax];    ///< All unit-types
extern int NumUnitTypes;                    ///< Number of unit-types made

/// @todo this hardcoded unit-types must be removed!!
extern UnitType*UnitTypeHumanWall;          ///< Human wall
extern UnitType*UnitTypeOrcWall;            ///< Orc wall

extern char** UnitTypeWcNames;              ///< Mapping wc-number 2 symbol

/**
**  Variable info for unit and unittype.
*/
extern struct _UnitTypeVar_{
	char** BoolFlagName;                ///< Array of name of user defined bool flag.
	int NumberBoolFlag;                 ///< Number of user defined bool flag.

	char** VariableName;                ///< Array of names of user defined variables.
	VariableType *Variable;             ///< Array of user defined variables (default value for unittype).
// EventType* Event;                   ///< Array of functions sets to call when en event occurs.
	int NumberVariable;                 ///< Number of defined variables.

	DecoVarType *DecoVar;               ///< Array to describe how showing variable.
	int NumberDeco;                     ///< Size of DecoVar.
} UnitTypeVar;
/*----------------------------------------------------------------------------
--  Functions
----------------------------------------------------------------------------*/

extern UnitType* CclGetUnitType(struct lua_State* l);  ///< Access unit-type object
extern void UnitTypeCclRegister(void);          ///< Register ccl features

extern void UpdateStats(int reset_to_default);  ///< Update unit stats
extern void ParsePudUDTA(const char*,int);      ///< Parse pud udta table
extern UnitType* UnitTypeByIdent(const char*);  ///< Get unit-type by ident
extern UnitType* UnitTypeByWcNum(unsigned);     ///< Get unit-type by wc number
extern int GetVariableIndex(const char *VarName); ///< Get index of the variable

	/// Get the animations structure by ident
extern Animations* AnimationsByIdent(const char* ident);

extern void SaveUnitTypes(struct _CL_File_* file);  ///< Save the unit-type table
extern UnitType* NewUnitTypeSlot(char*);            ///< Allocate an empty unit-type slot
	/// Draw the sprite frame of unit-type
extern void DrawUnitType(const UnitType* type, struct _graphic_* sprite, int player,
	int frame, int x, int y);

extern void InitUnitTypes(int reset_player_stats);  ///< Init unit-type table
extern void LoadUnitTypeSprite(UnitType* unittype); ///< Load the sprite for a unittype
extern void LoadUnitTypes(void);                    ///< Load the unit-type data
extern void CleanUnitTypes(void);                   ///< Cleanup unit-type module

// in script_unittype.c
	/// Update custom Variables with other variable (like Hp, ...)
extern void UpdateUnitVariables(const struct _unit_* unit);

//@}

#endif // !__UNITTYPE_H__
