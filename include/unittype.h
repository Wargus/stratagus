//     ____                _       __               
//    / __ )____  _____   | |     / /___ ___________
//   / __  / __ \/ ___/   | | /| / / __ `/ ___/ ___/
//  / /_/ / /_/ (__  )    | |/ |/ / /_/ / /  (__  ) 
// /_____/\____/____/     |__/|__/\__,_/_/  /____/  
//                                              
//       A futuristic real-time strategy game.
//          This file is part of Bos Wars.
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
//      $Id$

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
**  CUnitType::Sound
**
**    Sounds for events
**
**  CUnitType::Weapon
**
**    Currently sound for weapon
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
*/

/*----------------------------------------------------------------------------
--  Includes
----------------------------------------------------------------------------*/

#include "SDL.h"

#include <vector>
#include "upgrade_structs.h"
#include "util.h"
#include "unitsound.h"
#include "icons.h"

/*----------------------------------------------------------------------------
--  Declarations
----------------------------------------------------------------------------*/

class CPlayerColorGraphic;
class CConstruction;
class CAnimations;
class MissileType;
class CFile;
struct lua_State;

CUnitType *UnitTypeByIdent(const std::string &ident);

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
	ResourceInfo() : HarvestFromOutside(0), WaitAtResource(0), ResourceStep(0),
		ResourceCapacity(0), WaitAtDepot(0), ResourceId(0), FinalResource(0),
		LoseResources(0), SpriteWhenLoaded(NULL), SpriteWhenEmpty(NULL)
	{}

	std::string FileWhenLoaded;     /// Change the graphic when the unit is loaded.
	std::string FileWhenEmpty;      /// Change the graphic when the unit is empty.
	unsigned HarvestFromOutside;    /// Unit harvests without entering the building.
	unsigned WaitAtResource;        /// Cycles the unit waits while mining.
	unsigned ResourceStep;          /// Resources the unit gains per mining cycle.
	int      ResourceCapacity;      /// Max amount of resources to carry.
	unsigned WaitAtDepot;           /// Cycles the unit waits while returning.
	unsigned ResourceId;            /// Id of the resource harvested. Redundant.
	unsigned FinalResource;         /// Convert resource when delivered.
	unsigned LoseResources;         /// The unit will lose it's resource when distracted.
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
	COWARD_INDEX,
	BUILDING_INDEX,
	FLIP_INDEX,
	REVEALER_INDEX,
	LANDUNIT_INDEX,
	AIRUNIT_INDEX,
	SEAUNIT_INDEX,
	EXPLODEWHENKILLED_INDEX,
	VISIBLEUNDERFOG_INDEX,
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
};

// Index for variable already defined.
enum {
	HP_INDEX,
	BUILD_INDEX,
	MANA_INDEX,
	TRANSPORT_INDEX,
	TRAINING_INDEX,
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
	NVARALREADYDEFINED,
};

class CUnit;
class CUnitType;


/**
**  Decoration for userdefined variable.
**
**    It is used to show variables graphicly.
*/
class CDecoVar {
public:

	CDecoVar() {};
	virtual ~CDecoVar() {};

	/// function to draw the decorations.
	virtual void Draw(int x, int y, const CUnit *unit) const = 0;

	int Index;                  /// Index of the variable. @see DefineVariables

	int OffsetX;                /// Offset in X coord.
	int OffsetY;                /// Offset in Y coord.

	int OffsetXPercent;         /// Percent offset (TileWidth) in X coord.
	int OffsetYPercent;         /// Percent offset (TileHeight) in Y coord.

	char IsCenteredInX;         /// if true, use center of deco instead of left border
	char IsCenteredInY;         /// if true, use center of deco instead of upper border

	char ShowIfNotEnable;       /// if false, Show only if var is enable
	char ShowWhenNull;          /// if false, don't show if var is null (F.E poison)
	char HideHalf;              /// if true, don't show when 0 < var < max.
	char ShowWhenMax;           /// if false, don't show if var is to max. (Like mana)
	bool ShowOnlySelected;      /// if true, show only for selected units.

	bool HideNeutral;           /// if true, don't show for neutral unit.
	bool HideAllied;            /// if true, don't show for allied unit. (but show own units)
	bool ShowOpponent;          /// if true, show for opponent unit.
};

class CDecoVarBar : public CDecoVar
{
public:
	/// function to draw the decorations.
	virtual void Draw(int x, int y, const CUnit *unit) const;

	char IsVertical;            /// if true, vertical bar, else horizontal.
	char SEToNW;                /// (SouthEastToNorthWest), if false value 0 is on the left or up of the bar.
	int Height;                 /// Height of the bar.
	int Width;                  /// Width of the bar.
	char ShowFullBackground;    /// if true, show background like value equal to max.
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
	virtual void Draw(int x, int y, const CUnit *unit) const;

	CFont *Font;                   /// Font to use to display value.
// FIXME : Add Color, format
};

/// Sprite contains frame from full (left)to empty state (right).
class CDecoVarSpriteBar : public CDecoVar
{
public:
	CDecoVarSpriteBar() : NSprite(-1) {};
	/// function to draw the decorations.
	virtual void Draw(int x, int y, const CUnit *unit) const;

	char NSprite; /// Index of number. (@see DefineSprites and @see GetSpriteIndex)
// FIXME Sprite info. better way ?
};

/// use to show specific frame in a sprite.
class CDecoVarStaticSprite : public CDecoVar
{
public:
	CDecoVarStaticSprite() : NSprite(-1), n(0) {}
	/// function to draw the decorations.
	virtual void Draw(int x, int y, const CUnit *unit) const;

// FIXME Sprite info. and Replace n with more appropriate var.
	char NSprite;               /// Index of sprite. (@see DefineSprites and @see GetSpriteIndex)
	int n;                      /// identifiant in SpellSprite
};

typedef enum {
	UnitTypeLand,               /// Unit lives on land
	UnitTypeFly,                /// Unit lives in air
	UnitTypeNaval,              /// Unit lives on water
} UnitTypeType;

enum DistanceTypeType {
	Equal,
	NotEqual,
	LessThan,
	LessThanEqual,
	GreaterThan,
	GreaterThanEqual,
};


class CBuildRestriction {
public:

	virtual ~CBuildRestriction() {} ;
	virtual void Init() {};
	virtual bool Check(const CUnitType *type, int x, int y, CUnit *&ontoptarget) const = 0;
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
	virtual bool Check(const CUnitType *type, int x, int y, CUnit *&ontoptarget) const;

	void push_back(CBuildRestriction *restriction) {
		_or_list.push_back(restriction);
	}

	std::vector<CBuildRestriction*> _or_list;
};



class CBuildRestrictionAddOn : public CBuildRestriction {
public:
	CBuildRestrictionAddOn() : OffsetX(0), OffsetY(0), ParentName(NULL), Parent(NULL) {};
	virtual ~CBuildRestrictionAddOn() {delete[] this->ParentName;};
	virtual void Init() {this->Parent = UnitTypeByIdent(this->ParentName);};
	virtual bool Check(const CUnitType *type, int x, int y, CUnit *&ontoptarget) const;

	int OffsetX;         /// offset from the main building to place this
	int OffsetY;         /// offset from the main building to place this
	char *ParentName;    /// building that is unit is an addon too.
	CUnitType *Parent;   /// building that is unit is an addon too.
};

class CBuildRestrictionOnTop : public CBuildRestriction {
public:
	CBuildRestrictionOnTop() : ParentName(NULL), Parent(NULL), ReplaceOnDie(0), ReplaceOnBuild(0) {};
	virtual ~CBuildRestrictionOnTop() {delete[] this->ParentName;};
	virtual void Init() {this->Parent = UnitTypeByIdent(this->ParentName);};
	virtual bool Check(const CUnitType *type, int x, int y, CUnit *&ontoptarget) const;

	char *ParentName;    /// building that is unit is an addon too.
	CUnitType *Parent;   /// building that is unit is an addon too.
	int ReplaceOnDie;    /// recreate the parent on destruction
	int ReplaceOnBuild;  /// remove the parent, or just build over it.
};

class CBuildRestrictionDistance : public CBuildRestriction {
public:
	CBuildRestrictionDistance() : Distance(0), RestrictTypeName(NULL), RestrictType(NULL) {};
	virtual ~CBuildRestrictionDistance() {delete [] this->RestrictTypeName;};
	virtual void Init() {this->RestrictType = UnitTypeByIdent(this->RestrictTypeName);};
	virtual bool Check(const CUnitType *type, int x, int y, CUnit *&ontoptarget) const;

	int Distance;        /// distance to build (circle)
	DistanceTypeType DistanceType;
	char *RestrictTypeName;
	CUnitType *RestrictType;
};

	/// Base structure of unit-type
class CUnitType {
public:
	CUnitType() : Slot(0), Width(0), Height(0), OffsetX(0), OffsetY(0), DrawLevel(0),
		ShadowWidth(0), ShadowHeight(0), ShadowOffsetX(0), ShadowOffsetY(0),
		Animations(NULL), StillFrame(0), CorpseType(NULL),
		Construction(NULL),  RepairHP(0), TileWidth(0), TileHeight(0),
		BoxWidth(0), BoxHeight(0), NumDirections(0), MinAttackRange(0),
		ReactRangeComputer(0), ReactRangePerson(0), Priority(0),
		BurnPercent(0), BurnDamageRate(0), RepairRange(0),
		CanCastSpell(NULL), AutoCastActive(NULL), AutoBuildRate(0),
		RandomMovementProbability(0), ClicksToExplode(0),
		CanTransport(NULL), MaxOnBoard(0), StartingResources(0),
		UnitType(UnitTypeLand), DecayRate(0), AnnoyComputerFactor(0),
		MouseAction(0), Points(0), CanTarget(0),
		Flip(0), Revealer(0), LandUnit(0), AirUnit(0), SeaUnit(0),
		ExplodeWhenKilled(0), Building(0), VisibleUnderFog(0),
		Coward(0), AttackFromTransporter(0),
		Vanishes(0), GroundAttack(0), ShoreBuilding(0), CanAttack(0),
		BuilderOutside(0), BuilderLost(0), CanHarvest(0), Harvester(0),
		BoolFlag(NULL), Variable(NULL), CanTargetFlag(NULL),
		SelectableByRectangle(0), IsNotSelectable(0), Decoration(0),
		Indestructible(0), Teleporter(0),
		GivesResource(0), Supply(0), Demand(0), FieldFlags(0), MovementMask(0),
		Sprite(NULL), ShadowSprite(NULL)
	{
		memset(_Costs, 0, sizeof(_Costs));
		memset(RepairCosts, 0, sizeof(RepairCosts));
		memset(CanStore, 0, sizeof(CanStore));
		memset(ResInfo, 0, sizeof(ResInfo));
		memset(&NeutralMinimapColorRGB, 0, sizeof(NeutralMinimapColorRGB));
		memset(ImproveIncomes, 0, sizeof(ImproveIncomes));
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

	MissileConfig Missile;          /// Missile weapon
	MissileConfig Explosion;        /// Missile for unit explosion

	std::string CorpseName;         /// Corpse type name
	CUnitType *CorpseType;          /// Corpse unit-type

	// this is taken from the UDTA section
	CConstruction *Construction;    /// What is shown in construction phase

	int _Costs[MaxCosts];           /// How many resources needed
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
	char *CanTransport;             /// Can transport units with this flag.
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
	int Points;                     /// How many points you get for unit
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
	unsigned char *BoolFlag;        /// User defined flag. Used for (dis)allow target.
	CVariable *Variable;            /// Array of user defined variables.
	unsigned char *CanTargetFlag;   /// Flag needed to target with missile.

	unsigned SelectableByRectangle : 1; /// Selectable with mouse rectangle.
	unsigned IsNotSelectable : 1;       /// Unit should not be selected during game.
	unsigned Decoration : 1;            /// Unit is a decoration (act as tile).
	unsigned Indestructible : 1;        /// Unit is indestructible (take no damage).
	unsigned Teleporter : 1;            /// Can teleport other units.

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
};

	/// @todo ARI: should be dynamic (lua..).
	/// How many unit-types are currently supported
#define UnitTypeMax 257

/*----------------------------------------------------------------------------
--  Variables
----------------------------------------------------------------------------*/

extern std::vector<CUnitType *> UnitTypes;   /// All unit-types

/**
**  Variable info for unit and unittype.
*/
class CUnitTypeVar {
public:
	CUnitTypeVar() : BoolFlagName(NULL), NumberBoolFlag(0),
		VariableName(NULL), Variable(NULL), NumberVariable(0) {}

	char **BoolFlagName;                /// Array of name of user defined bool flag.
	int NumberBoolFlag;                 /// Number of user defined bool flag.

	char **VariableName;                /// Array of names of user defined variables.
	CVariable *Variable;                /// Array of user defined variables (default value for unittype).
//	EventType *Event;                   /// Array of functions sets to call when en event occurs.
	int NumberVariable;                 /// Number of defined variables.

	std::vector<CDecoVar *> DecoVar;    /// Array to describe how showing variable.
};

extern CUnitTypeVar UnitTypeVar;

/*----------------------------------------------------------------------------
--  Functions
----------------------------------------------------------------------------*/

extern CUnitType *CclGetUnitType(lua_State *l);      /// Access unit-type object
extern void UnitTypeCclRegister(void);               /// Register ccl features

extern void UpdateStats(int reset_to_default);       /// Update unit stats
extern CUnitType *UnitTypeByIdent(const std::string &ident);/// Get unit-type by ident
extern int GetVariableIndex(const char *VarName);    /// Get index of the variable

extern void SaveUnitTypes(CFile *file);              /// Save the unit-type table
extern CUnitType *NewUnitTypeSlot(const std::string &ident);/// Allocate an empty unit-type slot
	/// Draw the sprite frame of unit-type
extern void DrawUnitType(const CUnitType *type, CPlayerColorGraphic *sprite,
	int player, int frame, int x, int y);

extern void InitUnitTypes(int reset_player_stats);   /// Init unit-type table
extern void LoadUnitTypeSprite(CUnitType *unittype); /// Load the sprite for a unittype
extern void LoadUnitTypes(void);                     /// Load the unit-type data
extern void CleanUnitTypes(void);                    /// Cleanup unit-type module

// in script_unittype.c

	/// Parse User Variables field.
extern void DefineVariableField(lua_State *l, CVariable *var, int lua_index);

	/// Update custom Variables with other variable (like Hp, ...)
extern void UpdateUnitVariables(const CUnit *unit);

//@}

#endif // !__UNITTYPE_H__
