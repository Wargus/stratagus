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
//      (c) Copyright 1998-2008 by Lutz Sammer and Jimmy Salmon
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
**  CUnitType::RepairHP
**
**    The HP given to a unit each cycle it's repaired.
**    If zero, unit cannot be repaired
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
**  CUnitType::ExplodeWhenKilled
**
**    Death explosion animated
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
**  CUnitType::CanHarvestFrom
**
**    Resource can be harvested from.
**
**  CUnitType::Harvester
**
**    Unit is a resource worker.
**
**  CUnitType::NeutralMinimapColorRGB
**
**    Says what color a unit will have when it's neutral and
**    is displayed on the minimap.
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
**  CUnitType::SelectableByRectangle
**
**    Selectable with mouse rectangle
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
**  CUnitType::ExplicitAllowTerrainMask
**
**    Flags that the script explicitly wants to clear in MovementMask,
**    so that units of this type are allowed in those terrains.
**    This is used only for computing MovementMask, not in any actual
**    movement checks.
**
**  CUnitType::ExplicitForbidTerrainMask
**
**    Flags that the script explicitly wants to set in MovementMask,
**    so that units of this type are forbidden in those terrains.
**    This is used only for computing MovementMask, not in any actual
**    movement checks.
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
*/

/*----------------------------------------------------------------------------
--  Includes
----------------------------------------------------------------------------*/

#include "SDL.h"

#include <vector>
#include "upgrade_structs.h"
#include "unitsound.h"
#include "icons.h"

/*----------------------------------------------------------------------------
--  Declarations
----------------------------------------------------------------------------*/

class CUnit;
class CUnitType;
class CPlayerColorGraphic;
class CConstruction;
class CAnimations;
class MissileType;
class CFile;
struct lua_State;
class LuaCallback;

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

// Index for variable already defined.
enum {
	HP_INDEX,
	BUILD_INDEX,
	MANA_INDEX,
	TRANSPORT_INDEX,
	TRAINING_INDEX,
	GIVERESOURCE_INDEX,
	KILL_INDEX,
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
	SLOT_INDEX,
	NVARALREADYDEFINED,
};


/**
**  Decoration for user defined variable.
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

	bool IsCenteredInX;         /// if true, use center of deco instead of left border
	bool IsCenteredInY;         /// if true, use center of deco instead of upper border

	bool ShowIfNotEnable;       /// if false, Show only if var is enable
	bool ShowWhenMax;           /// if false, don't show if var is to max. (Like mana)
	bool ShowOnlySelected;      /// if true, show only for selected units.

	bool HideNeutral;           /// if true, don't show for neutral unit.
	bool HideAllied;            /// if true, don't show for allied unit. (but show own units)
	bool ShowOpponent;          /// if true, show for opponent unit.
};

/// Sprite contains frame from full (left)to empty state (right).
class CDecoVarSpriteBar : public CDecoVar
{
public:
	CDecoVarSpriteBar() : SpriteIndex(-1) {};
	/// function to draw the decorations.
	virtual void Draw(int x, int y, const CUnit *unit) const;

	char SpriteIndex; /// Index of number. (@see DefineSprites and @see GetSpriteIndex)
// FIXME Sprite info. better way ?
};

enum UnitTypeType {
	UnitTypeLand,               /// Unit lives on land
	UnitTypeFly,                /// Unit lives in air
	UnitTypeNaval,              /// Unit lives on water
};

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
	CBuildRestrictionAddOn() : OffsetX(0), OffsetY(0), Parent(NULL) {};
	virtual ~CBuildRestrictionAddOn() {};
	virtual void Init() {this->Parent = UnitTypeByIdent(this->ParentName);};
	virtual bool Check(const CUnitType *type, int x, int y, CUnit *&ontoptarget) const;

	int OffsetX;            /// offset from the main building to place this
	int OffsetY;            /// offset from the main building to place this
	std::string ParentName; /// building that is unit is an addon too.
	CUnitType *Parent;      /// building that is unit is an addon too.
};

class CBuildRestrictionOnTop : public CBuildRestriction {
public:
	CBuildRestrictionOnTop() : Parent(NULL), ReplaceOnDie(0), ReplaceOnBuild(0) {};
	virtual ~CBuildRestrictionOnTop() {};
	virtual void Init() {this->Parent = UnitTypeByIdent(this->ParentName);};
	virtual bool Check(const CUnitType *type, int x, int y, CUnit *&ontoptarget) const;

	std::string ParentName; /// building that is unit is an addon too.
	CUnitType *Parent;      /// building that is unit is an addon too.
	int ReplaceOnDie;       /// recreate the parent on destruction
	int ReplaceOnBuild;     /// remove the parent, or just build over it.
};

class CBuildRestrictionDistance : public CBuildRestriction {
public:
	CBuildRestrictionDistance() : Distance(0), RestrictType(NULL) {};
	virtual ~CBuildRestrictionDistance() {};
	virtual void Init() {this->RestrictType = UnitTypeByIdent(this->RestrictTypeName);};
	virtual bool Check(const CUnitType *type, int x, int y, CUnit *&ontoptarget) const;

	int Distance;        /// distance to build (circle)
	DistanceTypeType DistanceType;
	std::string RestrictTypeName;
	CUnitType *RestrictType;
};

class CBuildRestrictionTerrain : public CBuildRestriction {
public:
	CBuildRestrictionTerrain(unsigned fieldFlags, int min, int max)
		: FieldFlags(fieldFlags), Min(min), Max(max) {}
	virtual bool Check(const CUnitType *type, int x, int y, CUnit *&ontoptarget) const;

private:
	unsigned FieldFlags;
	int Min;
	int Max;
};

	/// Base structure of unit-type
class CUnitType {
public:
	CUnitType();
	~CUnitType();

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

	LuaCallback *DeathExplosion;

	std::string CorpseName;         /// Corpse type name
	CUnitType *CorpseType;          /// Corpse unit-type

	CConstruction *Construction;    /// What is shown in construction phase

	int RepairHP;                   /// Amount of HP per repair

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
	bool CanTransport;              /// Can transport units with this flag.
	int MaxOnBoard;                 /// Number of Transporter slots.
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
	int Points;                     /// How many points you get for unit
	int CanTarget;                  /// Which units can it attack
#define CanTargetLand 1             /// Can attack land units
#define CanTargetSea  2             /// Can attack sea units
#define CanTargetAir  4             /// Can attack air units

	unsigned Flip : 1;              /// Flip image when facing left
	unsigned Revealer : 1;          /// reveal the fog of war
	unsigned ExplodeWhenKilled : 1; /// Death explosion animated
	unsigned Building : 1;          /// Building
	unsigned VisibleUnderFog : 1;   /// Unit is visible under fog of war.
	unsigned Coward : 1;            /// Unit will only attack if instructed.
	unsigned AttackFromTransporter : 1;  /// Can attack from transporter
	unsigned Vanishes : 1;          /// Corpes & destroyed places.
	unsigned GroundAttack : 1;      /// Can do command ground attack.
	unsigned ShoreBuilding : 1;     /// Building must be build on coast.
	unsigned CanAttack : 1;         /// Unit can attack.
	unsigned CanHarvestFrom : 1;    /// Resource can be harvested.
	unsigned Harvester : 1;         /// unit is a resource harvester.
	unsigned Neutral : 1;           /// Unit is neutral, used by the editor

	unsigned SelectableByRectangle : 1; /// Selectable with mouse rectangle.
	unsigned IsNotSelectable : 1;       /// Unit should not be selected during game.
	unsigned Decoration : 1;            /// Unit is a decoration (act as tile).
	unsigned Indestructible : 1;        /// Unit is indestructible (take no damage).
	unsigned Organic : 1;               /// Can be transported or healed by a medic

	CVariable *Variable;            /// Array of user defined variables.

	std::vector<CBuildRestriction *> BuildingRules;/// Rules list for building a building.
	SDL_Color NeutralMinimapColorRGB;   /// Minimap Color for Neutral Units.

	CUnitSound Sound;                   /// Sounds for events

	int ProductionRate[MaxCosts];       /// Rate that resources are produced
	int MaxUtilizationRate[MaxCosts];   /// Max resource rate that can be used
	int ProductionCosts[MaxCosts];      /// Total cost to produce this type
	int StorageCapacity[MaxCosts];      /// Storage capacity of resources
	unsigned ProductionEfficiency : 8;  /// Production efficiency

	inline void SetEnergyProductionRate(int v) { ProductionRate[EnergyCost] = v; }
	inline int GetEnergyProductionRate() { return ProductionRate[EnergyCost]; }
	inline void SetMagmaProductionRate(int v) { ProductionRate[MagmaCost] = v; }
	inline int GetMagmaProductionRate() { return ProductionRate[MagmaCost]; }

	inline void SetMaxEnergyUtilizationRate(int v) { MaxUtilizationRate[EnergyCost] = v; }
	inline int GetMaxEnergyUtilizationRate() { return MaxUtilizationRate[EnergyCost]; }
	inline void SetMaxMagmaUtilizationRate(int v) { MaxUtilizationRate[MagmaCost] = v; }
	inline int GetMaxMagmaUtilizationRate() { return MaxUtilizationRate[MagmaCost]; }

	inline void SetEnergyValue(int v) { ProductionCosts[EnergyCost] = CYCLES_PER_SECOND * v; }
	inline int GetEnergyValue() { return ProductionCosts[EnergyCost] / CYCLES_PER_SECOND; }
	inline void SetMagmaValue(int v) { ProductionCosts[MagmaCost] = CYCLES_PER_SECOND * v; }
	inline int GetMagmaValue() { return ProductionCosts[MagmaCost] / CYCLES_PER_SECOND; }

	inline void SetEnergyStorageCapacity(int v) { StorageCapacity[EnergyCost] = CYCLES_PER_SECOND * v; }
	inline int GetEnergyStorageCapacity() { return StorageCapacity[EnergyCost] / CYCLES_PER_SECOND; }
	inline void SetMagmaStorageCapacity(int v) { StorageCapacity[MagmaCost] = CYCLES_PER_SECOND * v; }
	inline int GetMagmaStorageCapacity() { return StorageCapacity[MagmaCost] / CYCLES_PER_SECOND; }

// --- FILLED UP ---

	unsigned FieldFlags;            /// Unit map field flags
	unsigned MovementMask;          /// Unit check this map flags for move
	unsigned ExplicitAllowTerrainMask; /// Flags to clear from MovementMask
	unsigned ExplicitForbidTerrainMask; /// Flags to set in MovementMask

	/// @todo This stats should? be moved into the player struct
	CUnitStats Stats[PlayerMax];     /// Unit status for each player

	CPlayerColorGraphic *Sprite;     /// Sprite images
	CGraphic *ShadowSprite;          /// Shadow sprite image
};

/*----------------------------------------------------------------------------
--  Variables
----------------------------------------------------------------------------*/

extern std::vector<CUnitType *> UnitTypes;   /// All unit-types

/**
**  Variable info for unit and unittype.
*/
class CUnitTypeVar {
public:
	CUnitTypeVar() : VariableName(NULL), Variable(NULL), NumberVariable(0) {}

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
