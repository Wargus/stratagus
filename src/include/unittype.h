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
//      (c) Copyright 1998-2015 by Lutz Sammer, Jimmy Salmon and Andrettin
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
--  Includes
----------------------------------------------------------------------------*/

#ifndef __UPGRADE_STRUCTS_H__
#include "upgrade_structs.h"
#endif

#ifndef __UNITSOUND_H__
#include "unitsound.h"
#endif

#ifndef __ICONS_H__
#include "icons.h"
#endif

#include "color.h"
#include "luacallback.h"
#include "missileconfig.h"
#include "spells.h"
#include "util.h"
#include "vec2i.h"

#include <climits>
#include <vector>
#include <algorithm>
#include <map>

// Fix problems with defined None in X.h included though SDL.h
#undef None

/*----------------------------------------------------------------------------
--  Declarations
----------------------------------------------------------------------------*/

class CAnimations;
class CPlayerColorGraphic;
class CConstruction;
class MissileType;
class CFile;
struct lua_State;
#ifdef USE_MNG
class Mng;
#endif
class LuaCallback;

#define UnitSides 8
#define MaxAttackPos 5

CUnitType &UnitTypeByIdent(std::string_view ident); /// Get unit-type by ident

enum class EGroupSelectionMode {
	SelectableByRectangleOnly = 0,
	NonSelectableByRectangleOnly,
	SelectAll
};

class ResourceInfo
{
public:
	ResourceInfo() = default;

	std::string FileWhenLoaded;     /// Change the graphic when the unit is loaded.
	std::string FileWhenEmpty;      /// Change the graphic when the unit is empty.
	unsigned WaitAtResource = 0;      /// Cycles the unit waits while mining.
	unsigned ResourceStep = 0;        /// Resources the unit gains per mining cycle.
	int ResourceCapacity = 0;         /// Max amount of resources to carry.
	unsigned WaitAtDepot = 0;         /// Cycles the unit waits while returning.
	unsigned ResourceId = 0;          /// Id of the resource harvested. Redundant.
	unsigned FinalResource = 0;       /// Convert resource when delivered.
	bool TerrainHarvester = false;    /// Unit will harvest terrain.
	bool LoseResources = false;       /// The unit will lose it's resource when distracted.
	bool HarvestFromOutside = false;  /// Unit harvests without entering the building.
	bool RefineryHarvester = false;   /// Unit have to build Refinery buildings for harvesting.
	//  Runtime info:
	CPlayerColorGraphic *SpriteWhenLoaded = nullptr; /// The graphic corresponding to FileWhenLoaded.
	CPlayerColorGraphic *SpriteWhenEmpty = nullptr;  /// The graphic corresponding to FileWhenEmpty
};

/**
**  User defined variable type.
**
**  It is used to define variables and use it after
**  to manage magic, energy, shield or other stuff.
*/
class CVariable
{
public:
	CVariable() = default;

	bool operator ==(const CVariable &rhs) const
	{
		return this->Max == rhs.Max
			   && this->Value == rhs.Value
			   && this->Increase == rhs.Increase
			   && this->IncreaseFrequency == rhs.IncreaseFrequency
			   && this->Enable == rhs.Enable;
	}
	bool operator !=(const CVariable &rhs) const { return !(*this == rhs); }

public:
	int Max = 0;        /// Maximum for the variable. (Assume min is 0.)
	int Value = 0;      /// Current (or initial) value of the variable (or initial value).
	char Increase = 0;  /// Number to increase(decrease) Value by second.
	unsigned char IncreaseFrequency = 0; /// Every how many seconds we should apply the increase
	bool Enable = false; /// True if the unit doesn't have this variable. (f.e shield)
};

// Index for boolflag already defined
enum {
	COWARD_INDEX = 0,				/// Unit will only attack if instructed.
	BUILDING_INDEX,
	FLIP_INDEX,
	REVEALER_INDEX,					/// reveal the fog of war
	LANDUNIT_INDEX,
	AIRUNIT_INDEX,
	SEAUNIT_INDEX,
	EXPLODEWHENKILLED_INDEX,
	VISIBLEUNDERFOG_INDEX,			/// Unit is visible under fog of war.
	PERMANENTCLOAK_INDEX,			/// Is only visible by CloakDetectors.
	DETECTCLOAK_INDEX,				/// Can see Cloaked units.
	ATTACKFROMTRANSPORTER_INDEX,	/// Can attack from transporter
	VANISHES_INDEX,					/// Corpses & destroyed places.
	GROUNDATTACK_INDEX,				/// Can do ground attack command.
	SHOREBUILDING_INDEX,			/// Building must be built on coast.
	CANATTACK_INDEX,
	BUILDEROUTSIDE_INDEX,			/// The builder stays outside during the construction.
	BUILDERLOST_INDEX,				/// The builder is lost after the construction.
	CANHARVEST_INDEX,				/// Resource can be harvested.
	HARVESTER_INDEX,				/// Unit is a resource harvester.
	SELECTABLEBYRECTANGLE_INDEX,	/// Selectable with mouse rectangle.
	ISNOTSELECTABLE_INDEX,
	DECORATION_INDEX,				/// Unit is a decoration (act as tile).
	INDESTRUCTIBLE_INDEX,			/// Unit is indestructible (take no damage).
	TELEPORTER_INDEX,				/// Can teleport other units.
	SHIELDPIERCE_INDEX,
	SAVECARGO_INDEX,				/// Unit unloads his passengers after death.
	NONSOLID_INDEX,					/// Unit can be entered by other units.
	WALL_INDEX,						/// Use special logic for Direction field.
	NORANDOMPLACING_INDEX,			/// Don't use random frame rotation
	ORGANIC_INDEX,					/// Organic unit (used for death coil spell)
	SIDEATTACK_INDEX,               /// Unit turns sideways to attack (like e.g. a galley would before firing a broadside)
	SURROUND_ATTACK_INDEX,          /// Unit doesn't turn towards the attack (it can shoot in any direction)
	SKIRMISHER_INDEX,
	ALWAYSTHREAT_INDEX,				/// Unit always considered as threat for auto targeting algorihm, useful for unit without main attack ability, but which can cast spells (f.e. defiler in SC:BW)
	ELEVATED_INDEX,					/// Unit is elevated and can see over opaque tiles placed in the same ground level with the unit.
	NOFRIENDLYFIRE_INDEX,           /// Unit accepts friendly fire for splash attacks
	MAINFACILITY_INDEX,				/// Unit is a main building (Town Hall f. ex.)
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
	SUPPLY_INDEX,					/// Food supply
	DEMAND_INDEX,					/// Food demand
	ARMOR_INDEX,
	SIGHTRANGE_INDEX,
	ATTACKRANGE_INDEX,
	PIERCINGDAMAGE_INDEX,
	BASICDAMAGE_INDEX,
	POSX_INDEX,
	POSY_INDEX,
	POS_RIGHT_INDEX,
	POS_BOTTOM_INDEX,
	TARGETPOSX_INDEX,
	TARGETPOSY_INDEX,
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
	POISON_INDEX,
	SHIELDPERMEABILITY_INDEX,
	SHIELDPIERCING_INDEX,
	ISALIVE_INDEX,
	PLAYER_INDEX,
	PRIORITY_INDEX,
	NVARALREADYDEFINED
};

class CUnit;
class CUnitType;
class CFont;

/**
**  Decoration for user defined variable.
**
**  It is used to show variables graphicly.
**  @todo add more stuff in this struct.
*/
class CDecoVar
{
public:

	CDecoVar() = default;
	virtual ~CDecoVar() = default;

	/// function to draw the decorations.
	virtual void Draw(int x, int y, const CUnitType &type, const CVariable &var) const = 0;

	bool BoolFlagMatches(const CUnitType &type) const;

	unsigned int Index = -1;    /// Index of the variable. @see DefineVariables

	PixelPos Offset{0, 0}; /// Offset
	Vec2i OffsetPercent{0, 0}; /// Percent offset (Tile size).

	bool IsCenteredInX = false;     /// if true, use center of deco instead of left border
	bool IsCenteredInY = false;     /// if true, use center of deco instead of upper border

	bool ShowIfNotEnable = false;   /// if false, Show only if var is enable
	bool ShowWhenNull = false;      /// if false, don't show if var is null (F.E poison)
	bool HideHalf = false;          /// if true, don't show when 0 < var < max.
	bool ShowWhenMax = false;       /// if false, don't show if var is to max. (Like mana)
	bool ShowOnlySelected = false;  /// if true, show only for selected units.

	bool HideNeutral = false;       /// if true, don't show for neutral unit.
	bool HideAllied = false;        /// if true, don't show for allied unit. (but show own units)
	bool ShowOpponent = false;      /// if true, show for opponent unit.

	bool BoolFlagInvert = false;    /// if 1, invert the bool flag check
	int BoolFlag = -1;              /// if !=-1, show only for units with this flag
};

class CDecoVarBar : public CDecoVar
{
public:
	CDecoVarBar() = default;

	void Draw(int x, int y, const CUnitType &type, const CVariable &var) const override;

	bool IsVertical = false;         /// if true, vertical bar, else horizontal.
	bool SEToNW = false;             /// (SouthEastToNorthWest), if false value 0 is on the left or up of the bar.
	int Height = 0;                  /// Height of the bar.
	int Width = 0;                   /// Width of the bar.
	bool ShowFullBackground = false; /// if true, show background like value equal to max.
	bool Invert = false;             /// if true, invert length
	int MinValue = 0;                /// show only above percent
	int MaxValue = 100;              /// show only below percent
	char BorderSize = 0;             /// Size of the border, 0 for no border.
	// FIXME color depend of percent (red, Orange, Yellow, Green...)
	IntColor Color;             /// Color of bar.
	IntColor BColor;            /// Color of background.
};

class CDecoVarFrame : public CDecoVar
{
public:
	CDecoVarFrame() = default;

	void Draw(int x, int y, const CUnitType &type, const CVariable &var) const override;

	int Thickness = 0;
	int ColorIndex = -1;
};

class CDecoVarText : public CDecoVar
{
public:
	CDecoVarText() = default;

	/// function to draw the decorations.
	void Draw(int x, int y, const CUnitType &type, const CVariable &var) const override;

	CFont *Font = nullptr;  /// Font to use to display value.
	// FIXME : Add Color, format
};

/// Sprite contains frame from full (left)to empty state (right).
class CDecoVarSpriteBar : public CDecoVar
{
public:
	CDecoVarSpriteBar() = default;

	/// function to draw the decorations.
	void Draw(int x, int y, const CUnitType &type, const CVariable &var) const override;

	char NSprite = -1; /// Index of number. (@see DefineSprites and @see GetSpriteIndex)
	// FIXME Sprite info. better way ?
};

/// use to show specific frame in a sprite.
class CDecoVarStaticSprite : public CDecoVar
{
public:
	CDecoVarStaticSprite() = default;

	/// function to draw the decorations.
	void Draw(int x, int y, const CUnitType &type, const CVariable &var) const override;

	// FIXME Sprite info. and Replace n with more appropriate var.
	char NSprite = -1;  /// Index of sprite. (@see DefineSprites and @see GetSpriteIndex)
	int n = 0;          /// identifiant in SpellSprite
	int FadeValue = 0;  /// if variable's value is below than FadeValue, it drawn transparent.
};

/// use to show specific frame in a sprite.
class CDecoVarAnimatedSprite : public CDecoVar
{
public:
	CDecoVarAnimatedSprite() = default;

	/// function to draw the decorations.
	void Draw(int x, int y, const CUnitType &type, const CVariable &var) const override;

	char NSprite = -1;   /// Index of sprite. (@see DefineSprites and @see GetSpriteIndex)
	char WaitFrames = 0; /// Frames to wait between each sprite animation step
private:
	char lastFrame = 0;  /// last update
	int n = 0  ;         /// identifiant in SpellSprite
};

enum UnitTypeType {
	UnitTypeLand,  /// Unit lives on land
	UnitTypeFly,   /// Unit lives in air
	UnitTypeNaval  /// Unit lives on water
};

enum DistanceTypeType {
	Equal,
	NotEqual,
	LessThan,
	LessThanEqual,
	GreaterThan,
	GreaterThanEqual
};

class CBuildRestriction
{
public:
	virtual ~CBuildRestriction() = default;

	virtual void Init() {}
	virtual bool Check(const CUnit *builder, const CUnitType &type, const Vec2i &pos, CUnit *&ontoptarget) const = 0;
};

class CBuildRestrictionAnd : public CBuildRestriction
{
public:
	void Init() override
	{
		for (auto &restriction : _or_list) {
			restriction->Init();
		}
	}
	bool Check(const CUnit *builder, const CUnitType &type, const Vec2i &pos, CUnit *&ontoptarget) const override;

	void push_back(std::unique_ptr<CBuildRestriction>restriction) { _or_list.push_back(std::move(restriction)); }
public:
	std::vector<std::unique_ptr<CBuildRestriction>> _or_list;
};

class CBuildRestrictionAddOn : public CBuildRestriction
{
	class functor
	{
	public:
		functor(const CUnitType *type, const Vec2i &_pos): Parent(type), pos(_pos) {}
		inline bool operator()(const CUnit *const unit) const;
	private:
		const CUnitType *const Parent;   /// building that is unit is an addon too.
		const Vec2i pos; //functor work position
	};
public:
	CBuildRestrictionAddOn() = default;
	void Init() override { this->Parent = &UnitTypeByIdent(this->ParentName); }
	bool Check(const CUnit *builder, const CUnitType &type, const Vec2i &pos, CUnit *&ontoptarget) const override;

	Vec2i Offset{0, 0}; /// offset from the main building to place this
	std::string ParentName; /// building that is unit is an addon too.
	CUnitType *Parent = nullptr;      /// building that is unit is an addon too.
};

class CBuildRestrictionOnTop : public CBuildRestriction
{
	class functor
	{
	public:
		functor(const CUnitType *type, const Vec2i &_pos): ontop(nullptr), Parent(type), pos(_pos) {}
		inline bool operator()(CUnit *const unit);
		CUnit *ontop;   /// building that is unit is an addon too.
	private:
		const CUnitType *const Parent;  /// building that is unit is an addon too.
		const Vec2i pos;  //functor work position
	};
public:
	CBuildRestrictionOnTop() = default;
	void Init() override { this->Parent = &UnitTypeByIdent(this->ParentName); }
	bool Check(const CUnit *builder, const CUnitType &type, const Vec2i &pos, CUnit *&ontoptarget) const override;

	std::string ParentName;  /// building that is unit is an addon too.
	CUnitType *Parent = nullptr;   /// building that is unit is an addon too.
	bool ReplaceOnDie = false;     /// recreate the parent on destruction
	bool ReplaceOnBuild = false;   /// remove the parent, or just build over it.
};

class CBuildRestrictionDistance : public CBuildRestriction
{
public:
	CBuildRestrictionDistance() = default;

	void Init() override {this->RestrictType = &UnitTypeByIdent(this->RestrictTypeName);}
	bool Check(const CUnit *builder, const CUnitType &type, const Vec2i &pos, CUnit *&ontoptarget) const override;

	int Distance = 0;        /// distance to build (circle)
	DistanceTypeType DistanceType = DistanceTypeType::Equal;
	std::string RestrictTypeName;
	std::string RestrictTypeOwner;
	CUnitType *RestrictType = nullptr;
	bool CheckBuilder = false;
	bool Diagonal = true;
};

class CBuildRestrictionHasUnit : public CBuildRestriction
{
public:
	CBuildRestrictionHasUnit() = default;

	void Init() override { this->RestrictType = &UnitTypeByIdent(this->RestrictTypeName); }
	bool Check(const CUnit *builder, const CUnitType &type, const Vec2i &pos, CUnit *&ontoptarget) const override;

	int Count = 0;
	DistanceTypeType CountType = DistanceTypeType::Equal;
	std::string RestrictTypeName;
	CUnitType *RestrictType = nullptr;
	std::string RestrictTypeOwner;
};

class CBuildRestrictionSurroundedBy : public CBuildRestriction
{
public:
	CBuildRestrictionSurroundedBy() = default;

	void Init() override { this->RestrictType = &UnitTypeByIdent(this->RestrictTypeName); }
	bool Check(const CUnit *builder, const CUnitType &type, const Vec2i &pos, CUnit *&ontoptarget) const override;

	int Distance = 0;
	DistanceTypeType DistanceType = DistanceTypeType::Equal;
	int Count = 0;
	DistanceTypeType CountType = DistanceTypeType::Equal;
	std::string RestrictTypeName;
	std::string RestrictTypeOwner;
	CUnitType *RestrictType = nullptr;
	bool CheckBuilder = false;
};

class CBuildRestrictionLuaCallback : public CBuildRestriction
{
public:
	explicit CBuildRestrictionLuaCallback(std::unique_ptr<LuaCallback> callback) :
		Func(std::move(callback))
	{}

	void Init() override {}
	bool Check(const CUnit *builder, const CUnitType &type, const Vec2i &pos, CUnit *&ontoptarget) const override;

private:
	std::unique_ptr<LuaCallback> Func;
};

enum class EMouseAction
{
	None = 0, /// Nothing
	Attack = 1, /// Attack
	Move = 2, /// Move
	Harvest = 3, /// Harvest resources
	SpellCast = 5, /// Cast the first spell known
	Sail = 6 /// Sail
};

enum class ECanTargetFlag
{
	None = 0,
	Land = 1, /// Can attack land units
	Sea = 2, /// Can attack sea units
	Air = 4, /// Can attack air units
};

inline ECanTargetFlag operator~(ECanTargetFlag e)
{
	return ECanTargetFlag(~unsigned(e));
}

inline ECanTargetFlag operator|(ECanTargetFlag lhs, ECanTargetFlag rhs)
{
	return ECanTargetFlag(unsigned(lhs) | unsigned(rhs));
}

inline ECanTargetFlag operator&(ECanTargetFlag lhs, ECanTargetFlag rhs)
{
	return ECanTargetFlag(unsigned(lhs) & unsigned(rhs));
}

inline ECanTargetFlag& operator|=(ECanTargetFlag& lhs, ECanTargetFlag rhs)
{
	return lhs = ECanTargetFlag(unsigned(lhs) | unsigned(rhs));
}

inline ECanTargetFlag &operator&=(ECanTargetFlag &lhs, ECanTargetFlag rhs)
{
	return lhs = ECanTargetFlag(unsigned(lhs) & unsigned(rhs));
}

/// Base structure of unit-type
/// @todo n0body: AutoBuildRate not implemented.
class CUnitType
{
public:
	CUnitType() = default;
	~CUnitType();

	Vec2i GetHalfTileSize() const { return Vec2i(TileWidth / 2, TileHeight / 2); }
	PixelSize GetPixelSize() const;

	bool CheckUserBoolFlags(const std::vector<ECondition> &BoolFlags) const;
	bool CanTransport() const { return MaxOnBoard > 0 && !GivesResource; }
	bool CanMove() const;

	bool CanSelect(EGroupSelectionMode mode = EGroupSelectionMode::SelectableByRectangleOnly) const;

public:
	std::string Ident;              /// Identifier
	std::string Name;               /// Pretty name shown from the engine
	int Slot = 0;                   /// Type as number
	std::string File;               /// Sprite files
	std::string AltFile;            /// Alternative sprite files
	std::string ShadowFile;         /// Shadow file

	int Width = 0;                                        /// Sprite width
	int Height = 0;                                       /// Sprite height
	PixelPos Offset{0, 0};                                /// Sprite offset
	int DrawLevel = 0;                                    /// Level to Draw UnitType at
	int ShadowWidth = 0;                                  /// Shadow sprite width
	int ShadowHeight = 0;                                 /// Shadow sprite height
	PixelPos ShadowOffset{0, 0};                          /// Shadow offset
	char ShadowScale = 1;                                 /// Shadow scale-down factor
	char ShadowSpriteFrame = 0;                           /// If > 0, the shadow is a simple sprite without
	                                                      /// directions and this selects which frame to use
	PixelPos MissileOffsets[UnitSides][MaxAttackPos]{};   /// Attack offsets for missiles

	CAnimations *Animations = nullptr;  /// Animation scripts
	int StillFrame = 0;                 /// Still frame

	IconConfig Icon;                /// Icon to display for this unit
#ifdef USE_MNG
	struct _portrait_ {
		std::vector<std::string> Files;
		std::vector<Mng *> Mngs;
		int Talking = 0; /// offset into portraits for talking portraits
		mutable int CurrMng = 0;
		mutable int NumIterations = 0;
	} Portrait;
#endif
	MissileConfig Missile;                           /// Missile weapon
	MissileConfig Explosion;                         /// Missile for unit explosion
	MissileConfig Impact[ANIMATIONS_DEATHTYPES + 2]; /// Missiles spawned if unit is hit(+shield)

	LuaCallback *OnDeath = nullptr;           /// lua function called when unit is about to die, receives x,y,unit
	LuaCallback *OnHit = nullptr;             /// lua function called when unit is hit
	LuaCallback *OnEachCycle = nullptr;       /// lua function called every cycle
	LuaCallback *OnEachSecond = nullptr;      /// lua function called every second
	LuaCallback *OnInit = nullptr;            /// lua function called on unit init
	LuaCallback *OnReady = nullptr;           /// lua function called when unit ready/built

	int TeleportCost = 0;                     /// mana used for teleportation
	LuaCallback *TeleportEffectIn = nullptr;  /// lua function to create effects before teleportation
	LuaCallback *TeleportEffectOut = nullptr; /// lua function to create effects after teleportation

	mutable std::string DamageType; /// DamageType (used for extra death animations and impacts)

	std::string CorpseName;         /// Corpse type name
	CUnitType *CorpseType = nullptr;/// Corpse unit-type

	CConstruction *Construction = nullptr;    /// What is shown in construction phase

	int RepairHP = 0;                   /// Amount of HP per repair
	int RepairCosts[MaxCosts]{};        /// How much it costs to repair

	short TileWidth = 0;                /// Tile size on map width
	short TileHeight = 0;               /// Tile size on map height
	short PersonalSpaceWidth = 0;       /// How much "personal space" the unit tries to leave in X direction
	short PersonalSpaceHeight = 0;      /// How much "personal space" the unit tries to leave in Y direction
	int BoxWidth = 0;                   /// Selected box size width
	int BoxHeight = 0;                  /// Selected box size height
	PixelPos BoxOffset{0, 0};           /// Selected box size offset
	int NumDirections = 0;              /// Number of directions unit can face
	int MinAttackRange = 0;             /// Minimal attack range
	int ReactRangeComputer = 0;         /// Reacts on enemy for computer
	int ReactRangePerson = 0;           /// Reacts on enemy for person player
	int BurnPercent = 0;                /// Burning percent.
	int BurnDamageRate = 0;             /// HP burn rate per sec
	int RepairRange = 0;                /// Units repair range.
#define InfiniteRepairRange INT_MAX
	std::vector<char> CanCastSpell;     /// Unit is able to use spells.
	std::vector<bool> AutoCastActive;   /// Default value for autocast.
	int AutoBuildRate = 0;              /// The rate at which the building builds itself
	int RandomMovementProbability = 0;  /// Probability to move randomly.
	int RandomMovementDistance = 1;     /// Quantity of tiles to move randomly.
	int ClicksToExplode = 0;            /// Number of consecutive clicks until unit suicides.
	int MaxOnBoard = 0;                 /// Number of Transporter slots.
	int BoardSize = 1;                  /// How many "cells" unit occupies inside transporter
	int ButtonLevelForTransporter = 0;  /// On which button level game will show units inside transporter
	int StartingResources = 0;          /// Amount of Resources on build
	/// originally only visual effect, we do more with this!
	UnitTypeType UnitType = UnitTypeLand; /// Land / fly / naval
	int DecayRate = 0;                  /// Decay rate in 1/6 seconds
	// TODO: not used
	int AnnoyComputerFactor = 0;        /// How much this annoys the computer
	int AiAdjacentRange = -1;           /// Min radius for AI build surroundings checking
	EMouseAction MouseAction = EMouseAction::None; /// Right click action
	uint8_t RotationSpeed = 128;    /// Max unit.Direction change per frame. 128 is maximum
	ECanTargetFlag CanTarget = ECanTargetFlag::None; /// Which units can it attack

	bool Flip = false;              /// Flip image when facing left
	bool LandUnit = false;          /// Land animated
	bool AirUnit = false;           /// Air animated
	bool SeaUnit = false;           /// Sea animated
	bool ExplodeWhenKilled = false; /// Death explosion animated
	bool Building = false;          /// Building
	bool CanAttack = false;         /// Unit can attack.
	bool Neutral = false;           /// Unit is neutral, used by the editor

	bool SideAttack = false;        /// Unit turns for attack (used for ships)
	bool Skirmisher = false;        /// Unit will try to shoot from max range

	CUnitStats DefaultStat;
	CUnitStats MapDefaultStat;
	struct BoolFlags {
		bool value = false;             /// User defined flag. Used for (dis)allow target.
		ECondition CanTransport = ECondition::Ignore; /// Can transport units with this flag.
		ECondition CanTargetFlag = ECondition::Ignore; /// Flag needed to target with missile.
		ECondition AiPriorityTarget = ECondition::Ignore; /// Attack this units first.
	};
	std::vector<BoolFlags> BoolFlag;

	int CanStore[MaxCosts]{};           /// Resources that we can store here.
	int GivesResource = 0;              /// The resource this unit gives.
	ResourceInfo *ResInfo[MaxCosts]{};  /// Resource information.
	std::vector<std::unique_ptr<CBuildRestriction>>
		BuildingRules; /// Rules list for building a building.
	std::vector<std::unique_ptr<CBuildRestriction>>
		AiBuildingRules; /// Rules list for AI to build a building.
	CColor NeutralMinimapColorRGB;   /// Minimap Color for Neutral Units.

	CUnitSound Sound;               /// Sounds for events
	CUnitSound MapSound;            /// Sounds for events, map-specific

	int PoisonDrain = 0;                /// How much health is drained every second when poisoned

	// --- FILLED UP ---

	unsigned FieldFlags = 0;            /// Unit map field flags
	unsigned MovementMask = 0;          /// Unit check this map flags for move

	/// @todo This stats should? be moved into the player struct
	CUnitStats Stats[PlayerMax];     /// Unit status for each player

	CPlayerColorGraphic *Sprite = nullptr;     /// Sprite images
	CPlayerColorGraphic *AltSprite = nullptr;  /// Alternative sprite images
	CGraphic *ShadowSprite = nullptr;          /// Shadow sprite image
};

/*----------------------------------------------------------------------------
--  Variables
----------------------------------------------------------------------------*/

extern std::vector<CUnitType *> UnitTypes;   /// All unit-types

/// @todo this hardcoded unit-types must be removed!!
extern CUnitType *UnitTypeHumanWall;  /// Human wall
extern CUnitType *UnitTypeOrcWall;    /// Orc wall

/**
**  Variable info for unit and unittype.
*/
class CUnitTypeVar
{
public:

	template <const unsigned int SIZE>
	struct CKeys {

		struct DataKey {
			static bool key_pred(const DataKey &lhs, const DataKey &rhs)
			{
				return lhs.key < rhs.key;
			}
			std::string_view key;
			int offset;
		};

		CKeys(): TotalKeys(SIZE) {}

		DataKey buildin[SIZE];
		std::map<std::string, int, std::less<>> user;
		unsigned int TotalKeys;

		void Init()
		{
			ranges::sort(buildin, DataKey::key_pred);
		}

		std::string_view operator[](int index)
		{
			if (auto it = ranges::find(buildin, index, &DataKey::offset); it != std::end(buildin)) {
				return it->key;
			}
			if (auto it = ranges::find(user, index, &std::pair<const std::string, int>::second); it != user.end()) {
				return it->first;
			}
			return ""; // Not found
		}

		/**
		**  Return the index of the external storage array/vector.
		**
		**  @param varname  Name of the variable.
		**
		**  @return Index of the variable, -1 if not found.
		*/
		int operator[](std::string_view key)
		{
			DataKey k;
			k.key = key;
			const DataKey *p = std::lower_bound(buildin, buildin + SIZE,
												k, DataKey::key_pred);
			if ((p != buildin + SIZE) && p->key == key) {
				return p->offset;
			} else {
				auto it = user.find(key);
				if (it != user.end()) {
					return it->second;
				}
			}
			return -1;
		}

		int AddKey(const std::string& key)
		{
			int index = this->operator[](key);
			if (index != -1) {
				DebugPrint("Warning, Key '%s' already defined\n", key.c_str());
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

	unsigned int GetNumberBoolFlag() const { return BoolFlagNameLookup.TotalKeys; }
	unsigned int GetNumberVariable() const { return VariableNameLookup.TotalKeys; }

	CBoolKeys BoolFlagNameLookup;      /// Container of name of user defined bool flag.
	CVariableKeys VariableNameLookup;  /// Container of names of user defined variables.

	//EventType *Event;                  /// Array of functions sets to call when en event occurs.
	std::vector<CVariable> Variable;   /// Array of user defined variables (default value for unittype).
	std::vector<std::unique_ptr<CDecoVar>> DecoVar;   /// Array to describe how showing variable.
};

extern CUnitTypeVar UnitTypeVar;

/*----------------------------------------------------------------------------
--  Functions
----------------------------------------------------------------------------*/
extern CUnitType *CclGetUnitType(lua_State *l);  /// Access unit-type object
extern void UnitTypeCclRegister();               /// Register ccl features

extern void UpdateUnitStats(CUnitType &type, int reset_to_default);       /// Update unit stats
extern void UpdateStats(int reset_to_default);       /// Update unit stats

extern void SaveUnitTypes(CFile &file);              /// Save the unit-type table
extern std::pair<CUnitType *, bool /*redefined*/>
NewUnitTypeSlot(std::string_view ident); /// Allocate an empty unit-type slot
/// Draw the sprite frame of unit-type
extern void DrawUnitType(const CUnitType &type, CPlayerColorGraphic *sprite,
						 int colorIndex, int frame, const PixelPos &screenPos);

extern void InitUnitTypes(int reset_player_stats);   /// Init unit-type table
extern void LoadUnitTypeSprite(CUnitType &unittype); /// Load the sprite for a unittype
extern void LoadUnitTypes();                     /// Load the unit-type data
extern void CleanUnitTypes();                    /// Cleanup unit-type module

// in script_unittype.c

/// Parse User Variables field.
extern void DefineVariableField(lua_State *l, CVariable *var, int lua_index);

/// Update custom Variables with other variable (like Hp, ...)
extern void UpdateUnitVariables(CUnit &unit);

extern void SetMapStat(std::string ident, std::string variable_key, int value, std::string variable_type);
extern void SetMapSound(std::string ident, std::string sound, std::string sound_type, std::string sound_subtype = "");
//@}

#endif // !__UNITTYPE_H__
