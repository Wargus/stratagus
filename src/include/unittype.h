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

#ifndef __UNITTYPE_H__
#define __UNITTYPE_H__

//@{

/*----------------------------------------------------------------------------
-- Documentation
----------------------------------------------------------------------------*/

/**
**  @class CUnitType unittype.h
**
**  \#include "unittype.h"
**
**  This class contains the information that is shared between all
**  units of the same type and determins if a unit is a building,
**  a person, ...
**
**  The unit-type class members:
**
**  CUnitType::Ident
**
**    Unique identifier of the unit-type, used to reference it in
**    config files and during startup. As convention they start with
**    "unit-" fe. "unit-farm".
**  @note Don't use this member in game, use instead the pointer
**  to this structure. See UnitTypeByIdent().
**
**  CUnitType::Name
**
**    Pretty name shown by the engine. The name should be shorter
**    than 17 characters and no word can be longer than 8 characters.
**
**  CUnitType::File
**
**    Path file name of the sprite file.
**
**  CUnitType::ShadowFile
**
**    Path file name of shadow sprite file.
**
**  CUnitType::DrawLevel
**
**    The Level/Order to draw this type of unit in. 0-255 usually.
**
**  CUnitType::Width CUnitType::Height
**
**    Size of a sprite frame in pixels. All frames of a sprite have
**    the same size. Also all sprites (tilesets) must have the same
**    size.
**
**  CUnitType::ShadowWidth CUnitType::ShadowHeight
**
**    Size of a shadow sprite frame in pixels. All frames of a sprite
**    have the same size. Also all sprites (tilesets) must have the
**    same size.
**
**  CUnitType::ShadowOffsetX CUnitType::ShadowOffsetY
**
**    Vertical offset to draw the shadow in pixels.
**
**  CUnitType::Animations
**
**    Animation scripts for the different actions. Currently the
**    animations still, move, attack and die are supported.
**  @see CAnimations
**  @see CAnimation
**
**  CUnitType::Icon
**
**    Icon to display for this unit-type. Contains configuration and
**    run time variable.
**  @note This icon can be used for training, but isn't used.
**
**  CUnitType::Missile
**
**    Configuration and run time variable of the missile weapon.
**  @note It is planned to support more than one weapons.
**  And the sound of the missile should be used as fire sound.
**
**  CUnitType::Explosion
**
**    Configuration and run time variable of the missile explosion.
**    This is the explosion that happens if unit is set to
**    ExplodeWhenKilled
**
**  CUnitType::CorpseName
**
**    Corpse unit-type name, should only be used during setup.
**
**  CUnitType::CorpseType
**
**    Corpse unit-type pointer, only this should be used during run
**    time. Many unit-types can share the same corpse.
**
**
**  @todo continue this documentation
**
**  CUnitType::Construction
**
**    What is shown in construction phase.
**
**  CUnitType::SightRange
**
**    Sight range
**
**  CUnitType::_HitPoints
**
**    Maximum hit points
**
**
**  CUnitType::_Costs[::MaxCosts]
**
**    How many resources needed
**
**  CUnitType::RepairHP
**
**    The HP given to a unit each cycle it's repaired.
**    If zero, unit cannot be repaired
**
**    CUnitType::RepairCosts[::MaxCosts]
**
**    Costs per repair cycle to fix a unit.
**
**  CUnitType::TileWidth
**
**    Tile size on map width
**
**  CUnitType::TileHeight
**
**    Tile size on map height
**
**  CUnitType::BoxWidth
**
**    Selected box size width
**
**  CUnitType::BoxHeight
**
**    Selected box size height
**
**  CUnitType::NumDirections
**
**    Number of directions the unit can face
**
**  CUnitType::MinAttackRange
**
**    Minimal attack range
**
**  CUnitType::ReactRangeComputer
**
**    Reacts on enemy for computer
**
**  CUnitType::ReactRangePerson
**
**    Reacts on enemy for person player
**
**  CUnitType::Priority
**
**    Priority value / AI Treatment
**
**  CUnitType::BurnPercent
**
**    The burning limit in percents. If the unit has lees than
**    this it will start to burn.
**
**  CUnitType::BurnDamageRate
**
**    Burn rate in HP per second
**
**  CUnitType::UnitType
**
**    Land / fly / naval
**
**  @note original only visual effect, we do more with this!
**
**  CUnitType::DecayRate
**
**    Decay rate in 1/6 seconds
**
**  CUnitType::AnnoyComputerFactor
**
**    How much this annoys the computer
**
**  @todo not used
**
**  CUnitType::MouseAction
**
**    Right click action
**
**  CUnitType::Points
**
**    How many points you get for unit. Used in the final score table.
**
**  CUnitType::CanTarget
**
**    Which units can it attack
**
**  Unit::Revealer
**
**    A special unit used to reveal the map for a time. This unit
**    has active sight even when Removed. It's used for Reveal map
**    type of spells.
**
**  CUnitType::LandUnit
**
**    Land animated
**
**  CUnitType::AirUnit
**
**    Air animated
**
**  CUnitType::SeaUnit
**
**    Sea animated
**
**  CUnitType::ExplodeWhenKilled
**
**    Death explosion animated
**
**  CUnitType::RandomMovementProbability
**
**    When the unit is idle this is the probability that it will
**    take a step in a random direction, in percents.
**
**  CUnitType::ClicksToExplode
**
**    If this is non-zero, then after that many clicks the unit will
**    commit suicide. Doesn't work with resource workers/resources.
**
**  CUnitType::Building
**
**    Unit is a Building
**
**  CUnitType::VisibleUnderFog
**
**    Unit is visible under fog of war.
**
**  CUnitType::PermanentCloak
**
**    Unit is permanently cloaked.
**
**  CUnitType::DetectCloak
**
**    These units can detect Cloaked units.
**
**  CUnitType::Coward
**
**    Unit is a coward, and acts defensively. it will not attack
**    at will and auto-casters will not buff it(bloodlust).
**
**  CUnitType::Transporter
**
**    Can transport units
**
**  CUnitType::AttackFromTransporter
**
**    Units inside this transporter can attack with missiles.
**
**  CUnitType::MaxOnBoard
**
**    Maximum units on board (for transporters), and resources
**
**  CUnitType::StartingResources
**    Amount of Resources a unit has when It's Built
**
**  CUnitType::DamageType
**    Unit's missile damage type (used for extra death animations)
**
**  CUnitType::GivesResource
**
**    This equals to the resource Id of the resource given
**    or 0 (TimeCost) for other buildings.
**
**  CUnitType::CanHarvest
**
**    Resource can be harvested. It's false for things like
**    oil patches.
**  @todo crappy name.
**
**  CUnitType::Harvester
**
**    Unit is a resource worker. Faster than examining ResInfo
**
**  CUnitType::ResInfo[::MaxCosts]
**
**    Information about resource harvesting. If NULL, it can't
**    harvest it.
**
**  CUnitType::NeutralMinimapColorRGB
**
**    Says what color a unit will have when it's neutral and
**    is displayed on the minimap.
**
**  CUnitType::CanStore[::MaxCosts]
**
**    What resource types we can store here.
**
**  CUnitType::Vanishes
**
**    Corpes & destroyed places
**
**  CUnitType::GroundAttack
**
**    Can do command ground attack
**
**  CUnitType::ShoreBuilding
**
**    Building must be build on coast
**
**  CUnitType::CanCastSpell
**
**    Unit is able to use spells
**
**  CUnitType::CanAttack
**
**    Unit is able to attack.
**
**  CUnitType::RepairRange
**
**    Unit can repair buildings. It will use the actack animation.
**    It will heal 4 points for every repair cycle, and cost 1 of
**    each resource, alternatively(1 cycle wood, 1 cycle gold)
**  @todo The above should be more configurable.
**    If units have a repair range, they can repair, and this is the
**    distance.
**
**  CUnitType::BuilderOutside
**
**    Only valid for buildings. When building the worker will
**    remain outside inside the building.
**
**  @warning Workers that can build buildings with the
**  @warning BuilderOutside flag must have the CanRepair flag.
**
**  CUnitType::BuilderLost
**
**    Only valid for buildings without the BuilderOutside flag.
**    The worker is lost when the building is completed.
**
**  CUnitType::SelectableByRectangle
**
**    Selectable with mouse rectangle
**
**    CUnitType::Teleporter
**
**    Can teleport other units.
**
**    CUnitType::ShieldPiercing
**
**    Can directly damage shield-protected units, without shield damaging.
**
**    CUnitType::SaveCargo
**
**    Unit unloads his passengers after death.
**
**  CUnitType::Sound
**
**    Sounds for events
**
**  CUnitType::Weapon
**
**    Current sound for weapon
**
**  @todo temporary solution
**
**  CUnitType::Supply
**
**    How much food does this unit supply.
**
**  CUnitType::Demand
**
**    Food demand
**
**  CUnitType::ImproveIncomes[::MaxCosts]
**
**    Gives the player an improved income.
**
**  CUnitType::FieldFlags
**
**    Flags that are set, if a unit enters a map field or cleared, if
**    a unit leaves a map field.
**
**  CUnitType::MovementMask
**
**    Movement mask, this value is and'ed to the map field flags, to
**    see if a unit can enter or placed on the map field.
**
**  CUnitType::Stats[::PlayerMax]
**
**    Unit status for each player
**  @todo This stats should? be moved into the player struct
**
**  CUnitType::Type
**
**    Type as number
**  @todo Should us a general name f.e. Slot here?
**
**  CUnitType::Sprite
**
**    Sprite images
**
**  CUnitType::ShadowSprite
**
**    Shadow sprite images
**
**  CUnitType::PlayerColorSprite
**
**    Sprite images of the player colors.  This image is drawn
**    over CUnitType::Sprite.  Used with OpenGL only.
**
**
*/
/**
**
**  @class ResourceInfo unittype.h
**
** \#include "unittype.h"
**
**    This class contains information about how a unit will harvest a resource.
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

#include <SDL.h>
#include <vector>
#include <algorithm>

#ifndef __UPGRADE_STRUCTS_H__
#include "upgrade_structs.h"
#endif

#ifndef __UTIL_H__
#include "util.h"
#endif

#ifndef __UNITSOUND_H__
#include "unitsound.h"
#endif

#ifndef __ICONS_H__
#include "icons.h"
#endif

#ifndef __ANIMATIONS_H__
#include "animation.h"
#endif

#include "vec2i.h"

/*----------------------------------------------------------------------------
--  Declarations
----------------------------------------------------------------------------*/

class CPlayerColorGraphic;
class CConstruction;
class MissileType;
class CFile;
struct lua_State;
#ifdef USE_MNG
class Mng;
#endif
class LuaCallback;

CUnitType *UnitTypeByIdent(const std::string &ident);

enum GroupSelectionMode {
	SELECTABLE_BY_RECTANGLE_ONLY = 0,
	NON_SELECTABLE_BY_RECTANGLE_ONLY,
	SELECT_ALL
};

/**
**  Missile type definition (used in config tables)
**
**  @todo Move this to missle.h?
*/
class MissileConfig {
public:
	MissileConfig() : Missile(NULL) {}

	std::string Name;        /// Config missile name
	MissileType *Missile;    /// Identifier to use to run time
};

class ResourceInfo {
public:
	ResourceInfo() : WaitAtResource(0), ResourceStep(0),
		ResourceCapacity(0), WaitAtDepot(0), ResourceId(0), FinalResource(0),
		TerrainHarvester(0), LoseResources(0), HarvestFromOutside(0),
		SpriteWhenLoaded(NULL), SpriteWhenEmpty(NULL)
	{}

	std::string FileWhenLoaded;     /// Change the graphic when the unit is loaded.
	std::string FileWhenEmpty;      /// Change the graphic when the unit is empty.
	unsigned WaitAtResource;        /// Cycles the unit waits while mining.
	unsigned ResourceStep;          /// Resources the unit gains per mining cycle.
	int      ResourceCapacity;      /// Max amount of resources to carry.
	unsigned WaitAtDepot;           /// Cycles the unit waits while returning.
	unsigned ResourceId;            /// Id of the resource harvested. Redundant.
	unsigned FinalResource;         /// Convert resource when delivered.
	unsigned char TerrainHarvester;      /// Unit will harvest terrain(wood only for now).
	unsigned char LoseResources;         /// The unit will lose it's resource when distracted.
	unsigned char HarvestFromOutside;    /// Unit harvests without entering the building.
	unsigned char RefineryHarvester;    /// Unit have to build Refinery buildings for harvesting.
	//  Runtime info:
	CPlayerColorGraphic *SpriteWhenLoaded; /// The graphic corresponding to FileWhenLoaded.
	CPlayerColorGraphic *SpriteWhenEmpty;  /// The graphic corresponding to FileWhenEmpty
};

/**
**  User defined variable type.
**
**  It is used to define variables and use it after
**  to manage magic, energy, shield or other stuff.
*/
class CVariable {
public:
	CVariable() : Max(0), Value(0), Increase(0), Enable(0) {}

	int Max;           /// Maximum for the variable. (Assume min is 0.)
	int Value;         /// Current (or initial) value of the variable (or initial value).
	char Increase;     /// Number to increase(decrease) Value by second.
	char Enable;       /// True if the unit doesn't have this variable. (f.e shield)
};

// Index for boolflag aready defined
enum {
	COWARD_INDEX = 0,
	BUILDING_INDEX,
	FLIP_INDEX,
	REVEALER_INDEX,
	LANDUNIT_INDEX,
	AIRUNIT_INDEX,
	SEAUNIT_INDEX,
	EXPLODEWHENKILLED_INDEX,
	VISIBLEUNDERFOG_INDEX,
	PERMANENTCLOAK_INDEX,
	DETECTCLOAK_INDEX,
	ATTACKFROMTRANSPORTER_INDEX,
	VANISHES_INDEX,
	GROUNDATTACK_INDEX,
	SHOREBUILDING_INDEX,
	CANATTACK_INDEX,
	BUILDEROUTSIDE_INDEX,
	BUILDERLOST_INDEX,
	CANHARVEST_INDEX,
	HARVESTER_INDEX,
	SELECTABLEBYRECTANGLE_INDEX,
	ISNOTSELECTABLE_INDEX,
	DECORATION_INDEX,
	INDESTRUCTIBLE_INDEX,
	TELEPORTER_INDEX,
	SHIELDPIERCE_INDEX,
	SAVECARGO_INDEX,
	NONSOLID_INDEX,
	WALL_INDEX,
	NBARALREADYDEFINED
};

// Index for variable already defined.
enum {
	HP_INDEX = 0,
	BUILD_INDEX,
	MANA_INDEX,
	TRANSPORT_INDEX,
	RESEARCH_INDEX,
	TRAINING_INDEX,
	UPGRADINGTO_INDEX,
	GIVERESOURCE_INDEX,
	CARRYRESOURCE_INDEX,
	XP_INDEX,
	KILL_INDEX,
	SUPPLY_INDEX,
	DEMAND_INDEX,
	ARMOR_INDEX,
	SIGHTRANGE_INDEX,
	ATTACKRANGE_INDEX,
	PIERCINGDAMAGE_INDEX,
	BASICDAMAGE_INDEX,
	POSX_INDEX,
	POSY_INDEX,
	RADAR_INDEX,
	RADARJAMMER_INDEX,
	AUTOREPAIRRANGE_INDEX,
	BLOODLUST_INDEX,
	HASTE_INDEX,
	SLOW_INDEX,
	INVISIBLE_INDEX,
	UNHOLYARMOR_INDEX,
	SLOT_INDEX,
	SHIELD_INDEX,
	POINTS_INDEX,
	MAXHARVESTERS_INDEX,
	NVARALREADYDEFINED
};

class CUnit;
class CUnitType;
class CFont;

/**
**  Decoration for user defined variable.
**
**    It is used to show variables graphicly.
**  @todo add more stuff in this struct.
*/
class CDecoVar {
public:

	CDecoVar() {};
	virtual ~CDecoVar() {
	};

	/// function to draw the decorations.
	virtual void Draw(int x, int y, const CUnitType *Type, const CVariable &var) const = 0;

	unsigned int Index;         /// Index of the variable. @see DefineVariables

	int OffsetX;                /// Offset in X coord.
	int OffsetY;                /// Offset in Y coord.

	int OffsetXPercent;         /// Percent offset (TileWidth) in X coord.
	int OffsetYPercent;         /// Percent offset (TileHeight) in Y coord.

	bool IsCenteredInX;         /// if true, use center of deco instead of left border
	bool IsCenteredInY;         /// if true, use center of deco instead of upper border

	bool ShowIfNotEnable;       /// if false, Show only if var is enable
	bool ShowWhenNull;          /// if false, don't show if var is null (F.E poison)
	bool HideHalf;              /// if true, don't show when 0 < var < max.
	bool ShowWhenMax;           /// if false, don't show if var is to max. (Like mana)
	bool ShowOnlySelected;      /// if true, show only for selected units.

	bool HideNeutral;           /// if true, don't show for neutral unit.
	bool HideAllied;            /// if true, don't show for allied unit. (but show own units)
	bool ShowOpponent;          /// if true, show for opponent unit.
};

class CDecoVarBar : public CDecoVar
{
public:
	/// function to draw the decorations.
	virtual void Draw(int x, int y, const CUnitType *type, const CVariable &var) const;

	bool IsVertical;            /// if true, vertical bar, else horizontal.
	bool SEToNW;                /// (SouthEastToNorthWest), if false value 0 is on the left or up of the bar.
	int Height;                 /// Height of the bar.
	int Width;                  /// Width of the bar.
	bool ShowFullBackground;    /// if true, show background like value equal to max.
	char BorderSize;            /// Size of the border, 0 for no border.
// FIXME color depend of percent (red, Orange, Yellow, Green...)
	Uint32 Color;               /// Color of bar.
	Uint32 BColor;              /// Color of background.
};

class CDecoVarText : public CDecoVar
{
public:
	CDecoVarText() : Font(NULL) {};
	/// function to draw the decorations.
	virtual void Draw(int x, int y, const CUnitType *type, const CVariable &var) const;

	CFont *Font;                   /// Font to use to display value.
// FIXME : Add Color, format
};

/// Sprite contains frame from full (left)to empty state (right).
class CDecoVarSpriteBar : public CDecoVar
{
public:
	CDecoVarSpriteBar() : NSprite(-1) {};
	/// function to draw the decorations.
	virtual void Draw(int x, int y,
		const CUnitType *Type, const CVariable &Variable) const;

	char NSprite; /// Index of number. (@see DefineSprites and @see GetSpriteIndex)
// FIXME Sprite info. better way ?
};

/// use to show specific frame in a sprite.
class CDecoVarStaticSprite : public CDecoVar
{
public:
	CDecoVarStaticSprite() : NSprite(-1), n(0) {}
	/// function to draw the decorations.
	virtual void Draw(int x, int y, const CUnitType *type, const CVariable &var) const;

// FIXME Sprite info. and Replace n with more appropriate var.
	char NSprite;               /// Index of sprite. (@see DefineSprites and @see GetSpriteIndex)
	int n;                      /// identifiant in SpellSprite
};

enum UnitTypeType {
	UnitTypeLand,               /// Unit lives on land
	UnitTypeFly,                /// Unit lives in air
	UnitTypeNaval               /// Unit lives on water
};

enum DistanceTypeType {
	Equal,
	NotEqual,
	LessThan,
	LessThanEqual,
	GreaterThan,
	GreaterThanEqual
};


class CBuildRestriction {
public:

	virtual ~CBuildRestriction() {} ;
	virtual void Init() {};
	virtual bool Check(const CUnitType &type, int x, int y, CUnit *&ontoptarget) const = 0;
};


class CBuildRestrictionAnd : public CBuildRestriction {
public:

	virtual ~CBuildRestrictionAnd() {
		for (std::vector<CBuildRestriction*>::const_iterator i = _or_list.begin();
				i != _or_list.end(); ++i) {
			delete *i;
		}
		_or_list.clear();
	} ;
	virtual void Init() {
		for (std::vector<CBuildRestriction*>::const_iterator i = _or_list.begin();
				i != _or_list.end(); ++i) {
			(*i)->Init();
		}
	};
	virtual bool Check(const CUnitType &type, int x, int y, CUnit *&ontoptarget) const;

	void push_back(CBuildRestriction *restriction) {
		_or_list.push_back(restriction);
	}

	std::vector<CBuildRestriction*> _or_list;
};



class CBuildRestrictionAddOn : public CBuildRestriction {
	struct functor {
		const CUnitType *const Parent;   /// building that is unit is an addon too.
		const int x,y;	//functor work position
		functor(const CUnitType *type, int _x, int _y): Parent(type), x(_x), y(_y) {}
		inline bool operator() (const CUnit *const unit) const;
	};
public:
	CBuildRestrictionAddOn() : OffsetX(0), OffsetY(0), Parent(NULL) {};
	virtual ~CBuildRestrictionAddOn() {};
	virtual void Init() {this->Parent = UnitTypeByIdent(this->ParentName);};
	virtual bool Check(const CUnitType &type, int x, int y, CUnit *&ontoptarget) const;

	int OffsetX;         /// offset from the main building to place this
	int OffsetY;         /// offset from the main building to place this
	std::string ParentName; /// building that is unit is an addon too.
	CUnitType *Parent;   /// building that is unit is an addon too.
};

class CBuildRestrictionOnTop : public CBuildRestriction {
	struct functor {
		CUnit *ontop;   /// building that is unit is an addon too.
		const CUnitType *const Parent;   /// building that is unit is an addon too.
		const int x,y;	//functor work position
		functor(const CUnitType *type, int _x, int _y): ontop(0), Parent(type), x(_x), y(_y) {}
		inline bool operator() (CUnit *const unit);
	};
public:
	CBuildRestrictionOnTop() : Parent(NULL), ReplaceOnDie(0), ReplaceOnBuild(0) {};
	virtual ~CBuildRestrictionOnTop() {};
	virtual void Init() {this->Parent = UnitTypeByIdent(this->ParentName);};
	virtual bool Check(const CUnitType &type, int x, int y, CUnit *&ontoptarget) const;

	std::string ParentName;  /// building that is unit is an addon too.
	CUnitType *Parent;   /// building that is unit is an addon too.
	int ReplaceOnDie:1;    /// recreate the parent on destruction
	int ReplaceOnBuild:1;  /// remove the parent, or just build over it.
};

class CBuildRestrictionDistance : public CBuildRestriction {
public:
	CBuildRestrictionDistance() : Distance(0), RestrictType(NULL) {};
	virtual ~CBuildRestrictionDistance() {};
	virtual void Init() {this->RestrictType = UnitTypeByIdent(this->RestrictTypeName);};
	virtual bool Check(const CUnitType &type, int x, int y, CUnit *&ontoptarget) const;

	int Distance;        /// distance to build (circle)
	DistanceTypeType DistanceType;
	std::string RestrictTypeName;
	CUnitType *RestrictType;
};

	/// Base structure of unit-type
	/// @todo n0body: AutoBuildRate not implemented.
class CUnitType {
public:
	CUnitType();
	~CUnitType();

	Vec2i GetHalfTileSize() const {
		Vec2i res = {TileWidth / 2, TileHeight / 2};

		return res;
	}

	std::string Ident;              /// Identifier
	std::string Name;               /// Pretty name shown from the engine
	int Slot;                       /// Type as number
	std::string File;               /// Sprite files
	std::string ShadowFile;         /// Shadow file

	int Width;                      /// Sprite width
	int Height;                     /// Sprite height
	int OffsetX;                    /// Sprite horizontal offset
	int OffsetY;                    /// Sprite vertical offset
	int DrawLevel;                  /// Level to Draw UnitType at
	int ShadowWidth;                /// Shadow sprite width
	int ShadowHeight;               /// Shadow sprite height
	int ShadowOffsetX;              /// Shadow horizontal offset
	int ShadowOffsetY;              /// Shadow vertical offset

	CAnimations *Animations;        /// Animation scripts
	int StillFrame;                 /// Still frame

	IconConfig Icon;                /// Icon to display for this unit
#ifdef USE_MNG
	struct _portrait_ {
		std::string *Files;
		int Num;
		Mng **Mngs;
		int CurrMng;
		int NumIterations;
	} Portrait;
#endif
	MissileConfig Missile;                           /// Missile weapon
	MissileConfig Explosion;                         /// Missile for unit explosion
	MissileConfig Impact[ANIMATIONS_DEATHTYPES + 2]; /// Missiles spawned if unit is hit(+shield)

	LuaCallback *DeathExplosion;
	LuaCallback *OnHit;				/// lua function called whel unit is hit

	std::string DamageType;         /// DamageType (used for extra death animations and impacts)

	std::string CorpseName;         /// Corpse type name
	CUnitType *CorpseType;          /// Corpse unit-type

	CConstruction *Construction;    /// What is shown in construction phase

	int _Costs[MaxCosts];           /// How many resources needed
	int _Storing[MaxCosts];         /// How many resources the unit can store 
	int RepairHP;                   /// Amount of HP per repair
	int RepairCosts[MaxCosts];      /// How much it costs to repair

	int TileWidth;                  /// Tile size on map width
	int TileHeight;                 /// Tile size on map height
	int BoxWidth;                   /// Selected box size width
	int BoxHeight;                  /// Selected box size height
	int NumDirections;              /// Number of directions unit can face
	int MinAttackRange;             /// Minimal attack range
	int ReactRangeComputer;         /// Reacts on enemy for computer
	int ReactRangePerson;           /// Reacts on enemy for person player
	int Priority;                   /// Priority value / AI Treatment
	int BurnPercent;                /// Burning percent.
	int BurnDamageRate;             /// HP burn rate per sec
	int RepairRange;                /// Units repair range.
	char *CanCastSpell;             /// Unit is able to use spells.
	char *AutoCastActive;           /// Default value for autocast.
	int AutoBuildRate;              /// The rate at which the building builds itself
	int RandomMovementProbability;  /// Probability to move randomly.
	int ClicksToExplode;            /// Number of consecutive clicks until unit suicides.
	int MaxOnBoard;                 /// Number of Transporter slots.
	int StartingResources;          /// Amount of Resources on build
	/// originally only visual effect, we do more with this!
	UnitTypeType UnitType;          /// Land / fly / naval
	int DecayRate;                  /// Decay rate in 1/6 seconds
	// TODO: not used
	int AnnoyComputerFactor;        /// How much this annoys the computer
	int MouseAction;                /// Right click action
#define MouseActionNone      0      /// Nothing
#define MouseActionAttack    1      /// Attack
#define MouseActionMove      2      /// Move
#define MouseActionHarvest   3      /// Harvest resources
#define MouseActionSpellCast 5      /// Cast the first spell known
#define MouseActionSail      6      /// Sail
	int CanTarget;                  /// Which units can it attack
#define CanTargetLand 1             /// Can attack land units
#define CanTargetSea  2             /// Can attack sea units
#define CanTargetAir  4             /// Can attack air units

	unsigned Flip : 1;              /// Flip image when facing left
	unsigned Revealer : 1;          /// reveal the fog of war
	unsigned LandUnit : 1;          /// Land animated
	unsigned AirUnit : 1;           /// Air animated
	unsigned SeaUnit : 1;           /// Sea animated
	unsigned ExplodeWhenKilled : 1; /// Death explosion animated
	unsigned Building : 1;          /// Building
	unsigned VisibleUnderFog : 1;   /// Unit is visible under fog of war.
	unsigned PermanentCloak : 1;    /// Is only visible by CloakDetectors.
	unsigned DetectCloak : 1;       /// Can see Cloaked units.
	unsigned Coward : 1;            /// Unit will only attack if instructed.
	unsigned AttackFromTransporter : 1;  /// Can attack from transporter
	unsigned Vanishes : 1;          /// Corpes & destroyed places.
	unsigned GroundAttack : 1;      /// Can do command ground attack.
	unsigned ShoreBuilding : 1;     /// Building must be build on coast.
	unsigned CanAttack : 1;         /// Unit can attack.
	unsigned BuilderOutside : 1;    /// The builder stays outside during the build.
	unsigned BuilderLost : 1;       /// The builder is lost after the build.
	unsigned CanHarvest : 1;        /// Resource can be harvested.
	unsigned Harvester : 1;         /// unit is a resource harvester.
	unsigned Neutral : 1;           /// Unit is neutral, used by the editor

	unsigned SelectableByRectangle : 1; /// Selectable with mouse rectangle.
	unsigned IsNotSelectable : 1;       /// Unit should not be selected during game.
	unsigned Decoration : 1;            /// Unit is a decoration (act as tile).
	unsigned Indestructible : 1;        /// Unit is indestructible (take no damage).
	unsigned Teleporter : 1;            /// Can teleport other units.
	unsigned ShieldPiercing : 1;        /// Can directly damage shield-protected units, without shield damaging.
	unsigned SaveCargo : 1;             /// Unit unloads his passengers after death.
	unsigned NonSolid : 1;              /// Unit can be entered by other units.
	unsigned Wall : 1;                  /// Use special logic for Direction field.

	CVariable *Variable;            /// Array of user defined variables.
	struct BoolFlags {
		bool value;					/// User defined flag. Used for (dis)allow target.
		char CanTransport;			/// Can transport units with this flag.
		char CanTargetFlag;			/// Flag needed to target with missile.
	};
	std::vector<BoolFlags> BoolFlag;

	int CanStore[MaxCosts];             /// Resources that we can store here.
	int GivesResource;                  /// The resource this unit gives.
	ResourceInfo *ResInfo[MaxCosts];    /// Resource information.
	std::vector<CBuildRestriction *> BuildingRules;   /// Rules list for building a building.
	SDL_Color NeutralMinimapColorRGB;   /// Minimap Color for Neutral Units.

	CUnitSound Sound;               /// Sounds for events

	int Supply;                     /// Food supply
	int Demand;                     /// Food demand

// --- FILLED UP ---

	int ImproveIncomes[MaxCosts];   /// Gives player an improved income

	unsigned FieldFlags;            /// Unit map field flags
	unsigned MovementMask;          /// Unit check this map flags for move

	/// @todo This stats should? be moved into the player struct
	CUnitStats Stats[PlayerMax];     /// Unit status for each player

	CPlayerColorGraphic *Sprite;     /// Sprite images
	CGraphic *ShadowSprite;          /// Shadow sprite image

	/* API */

	bool CheckUserBoolFlags(char *BoolFlags);
	bool CanTransport()  const
	{
		return MaxOnBoard > 0 && !GivesResource;
	}
	bool CanMove() const
	{
		return Animations && Animations->Move;
	}

	bool CanSelect(GroupSelectionMode mode = SELECTABLE_BY_RECTANGLE_ONLY) const
	{
		if(!IsNotSelectable) {
			switch(mode) {
				case SELECTABLE_BY_RECTANGLE_ONLY:
					return SelectableByRectangle;
				case NON_SELECTABLE_BY_RECTANGLE_ONLY:
					return !SelectableByRectangle;
				default:
					return true;
			}
		}
		return false;
	}

};

/*----------------------------------------------------------------------------
--  Variables
----------------------------------------------------------------------------*/

extern std::vector<CUnitType *> UnitTypes;   /// All unit-types

/// @todo this hardcoded unit-types must be removed!!
extern CUnitType *UnitTypeHumanWall;          /// Human wall
extern CUnitType *UnitTypeOrcWall;            /// Orc wall

/**
**  Variable info for unit and unittype.
*/
class CUnitTypeVar {
public:

	template <const unsigned int SIZE>
	struct CKeys {

		struct DataKey
		{
			static bool key_pred(const DataKey& lhs,
									const DataKey& rhs)
			{
				return ((lhs.keylen == rhs.keylen) ?
					(strcmp(lhs.key, rhs.key) < 0) : (lhs.keylen < rhs.keylen));
			}
			int offset;
			unsigned int keylen;
			const char* key;
		};

		CKeys(): TotalKeys(SIZE) {}

		DataKey buildin[SIZE];
		std::map<std::string, int> user;
		unsigned int TotalKeys;

		void Init() {
			std::sort(buildin, buildin+SIZE, DataKey::key_pred);
		}

		const char *operator[](int index) {
			for (unsigned int i = 0; i < SIZE; ++i) {
				if (buildin[i].offset == index) {
					return buildin[i].key;
				}
			}
			for (std::map<std::string, int>::iterator
				it(user.begin()), end(user.end());
				it != end; ++it) {
				if ((*it).second == index) {
					return ((*it).first).c_str();
				}
			}
			return NULL;
		}

		/**
		**  Return the index of the external storage array/vector.
		**
		**  @param varname  Name of the variable.
		**
		**  @return         Index of the variable, -1 if not found.
		*/
		int operator[](const char*const key) {
			DataKey k;
			k.key = key;
			k.keylen = strlen(key);
			const DataKey* p = std::lower_bound(buildin, buildin + SIZE,
				k, DataKey::key_pred);
			if ((p != buildin + SIZE) && p->keylen == k.keylen &&
				0 == strcmp(p->key, key)) {
				return p->offset;
			} else {
				std::map<std::string, int>::iterator
						ret(user.find(key));
				if (ret != user.end()) {
					return  (*ret).second;
				}
			}
			return -1;
		}

		int AddKey(const char*const key)
		{
			int index = this->operator[](key);
			if (index != -1) {
				DebugPrint("Warning, Key '%s' already defined\n" _C_ key);
				return index;
			}
			user[key] = TotalKeys++;
			return TotalKeys - 1;
		}

	};

	struct CBoolKeys : public CKeys<NBARALREADYDEFINED> {
		CBoolKeys();
	};

	struct CVariableKeys : public CKeys<NVARALREADYDEFINED> {
		CVariableKeys();
	};

	CUnitTypeVar() {}

	void Init();
	void Clear();

	CBoolKeys BoolFlagNameLookup;		/// Container of name of user defined bool flag.
	CVariableKeys VariableNameLookup;	/// Container of names of user defined variables.

//	EventType *Event;                   /// Array of functions sets to call when en event occurs.
	std::vector<CVariable> Variable;	/// Array of user defined variables (default value for unittype).
	std::vector<CDecoVar *> DecoVar;    /// Array to describe how showing variable.

	unsigned int GetNumberBoolFlag() const {
		return BoolFlagNameLookup.TotalKeys;
	}

	unsigned int GetNumberVariable() const {
		return VariableNameLookup.TotalKeys;
	}
};

extern CUnitTypeVar UnitTypeVar;

/*----------------------------------------------------------------------------
--  Functions
----------------------------------------------------------------------------*/
extern CUnitType *CclGetUnitType(lua_State *l);      /// Access unit-type object
extern void UnitTypeCclRegister();               /// Register ccl features

extern void UpdateStats(int reset_to_default);       /// Update unit stats
extern CUnitType *UnitTypeByIdent(const std::string &ident);/// Get unit-type by ident

extern void SaveUnitTypes(CFile &file);              /// Save the unit-type table
extern CUnitType *NewUnitTypeSlot(const std::string &ident);/// Allocate an empty unit-type slot
	/// Draw the sprite frame of unit-type
extern void DrawUnitType(const CUnitType &type, CPlayerColorGraphic *sprite,
	int player, int frame, int x, int y);

extern void InitUnitTypes(int reset_player_stats);   /// Init unit-type table
extern void LoadUnitTypeSprite(CUnitType &unittype); /// Load the sprite for a unittype
extern void LoadUnitTypes();                     /// Load the unit-type data
extern void CleanUnitTypes();                    /// Cleanup unit-type module

// in script_unittype.c

	/// Parse User Variables field.
extern void DefineVariableField(lua_State *l, CVariable *var, int lua_index);

	/// Update custom Variables with other variable (like Hp, ...)
extern void UpdateUnitVariables(CUnit &unit);

//@}

#endif // !__UNITTYPE_H__
