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
/**@name spells.h - The Spells. */
//
//      (c) Copyright 1999-2006 by Vladi Belperchinov-Shabanski,
//                                 Joris DAUPHIN, and Jimmy Salmon
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

#ifndef __SPELLS_H__
#define __SPELLS_H__

//@{

/*----------------------------------------------------------------------------
--  Includes
----------------------------------------------------------------------------*/

#include "luacallback.h"
#include "unitsound.h"
#include "vec2i.h"

#include <memory>
#include <string_view>
#include <variant>

/*----------------------------------------------------------------------------
--  Declarations
----------------------------------------------------------------------------*/

class CUnit;
class CUnitType;
class CPlayer;
struct lua_State;
class SpellType;
class MissileType;

/*----------------------------------------------------------------------------
--  Definitons
----------------------------------------------------------------------------*/

/**
**  Generic spell action virtual class.
**  Spells are sub class of this one
*/
class SpellActionType
{
public:
	SpellActionType(int mod = 0) : ModifyManaCaster(mod) {}
	virtual ~SpellActionType() {}

	virtual int Cast(CUnit &caster, const SpellType &spell,
					 CUnit* &target, const Vec2i &goalPos) = 0;
	virtual void Parse(lua_State *l, int startIndex, int endIndex) = 0;

	const int ModifyManaCaster;
};


/**
**  Different targets.
*/
enum class ETarget {
	Self,
	Position,
	Unit
};

enum class ECondition
{
	Ignore,
	ShouldBeFalse,
	ShouldBeTrue
};

/*
** *******************
** Conditions definition.
** *******************
*/

class ConditionInfoVariable
{
public:
	ConditionInfoVariable() = default;

	ECondition Enable = ECondition::Ignore; /// Target is 'user defined variable'.
	bool Check = false;             /// True if need to check that variable.

	int ExactValue = 0;             /// Target must have exactly ExactValue of it's value.
	int ExceptValue = 0;            /// Target mustn't have ExceptValue of it's value.
	int MinValue = 0;               /// Target must have more Value than that.
	int MaxValue = 0;               /// Target must have less Value than that.
	int MinMax = 0;                 /// Target must have more Max than that.
	int MinValuePercent = 0;        /// Target must have more (100 * Value / Max) than that.
	int MaxValuePercent = 0;        /// Target must have less (100 * Value / Max) than that.

	bool ConditionApplyOnCaster = false; /// true if these condition are for caster.
	// FIXME : More (increase, MaxMax) ?
};

/**
**  Conditions for a spell.
**
**  @todo  Move more parameters into this structure.
*/
class ConditionInfo
{
public:
	ConditionInfo() = default;
	//
	//  Conditions that check specific flags. Possible values are the defines below.
	//
	ECondition Alliance = ECondition::Ignore; /// Target is allied. (neutral is neither allied, nor opponent)
	ECondition Opponent = ECondition::Ignore; /// Target is opponent. (neutral is neither allied, nor opponent)
	ECondition TargetSelf = ECondition::ShouldBeFalse; /// Target is the same as the caster.

	std::vector<ECondition> BoolFlag; /// User defined boolean flag.

	std::vector<ConditionInfoVariable> Variable;
	mutable LuaCallback CheckFunc;
	//
	//  @todo more? feel free to add, here and to
	//  @todo PassCondition, CclSpellParseCondition, SaveSpells
	//
};

/**
**  information about the autocasting mode.
*/
class AutoCastInfo
{
public:
	// Special flags for priority sorting
#define ACP_NOVALUE -1
#define ACP_DISTANCE -2
	AutoCastInfo() = default;
	/// @todo this below is SQUARE!!!
	int Range = 0;                   /// Max range of the target.
	int MinRange = 0;                /// Min range of the target.

	int PriorytyVar = ACP_NOVALUE;   /// Variable to sort autocast targets by priority.
	bool ReverseSort = false;        /// If true, small values have the highest priority.

	std::unique_ptr<ConditionInfo> Condition;    /// Conditions to cast the spell.

	/// Detailed generic conditions (not per-target, where Condition is evaluated.)
	/// Combat mode is when there are hostile non-coward units around
	ECondition Combat = ECondition::Ignore; /// If it should be casted in combat
	ECondition Attacker = ECondition::Ignore; /// If it should be casted on unit which attacks
	ECondition Corpse = ECondition::ShouldBeFalse; /// If it should be casted on corpses

	// Position autocast callback
	LuaCallback PositionAutoCast;
};

/**
**  Base structure of a spell type.
*/
class SpellType
{
public:
	SpellType(int slot, const std::string &ident) : Ident(ident), Slot(slot) {}

	bool IsCasterOnly() const { return !Range && Target == ETarget::Self; }

	// Identification stuff
	std::string Ident;    /// Spell unique identifier (spell-holy-vision)
	std::string Name;     /// Spell name shown by the engine
	int Slot;             /// Spell numeric identifier

	// Spell Specifications
	ETarget Target;          /// Targeting information. See TargetType.
	std::vector<std::unique_ptr<SpellActionType>> Action; /// More arguments for spell (damage, delay, additional sounds...).

	int Range = 0;                  /// Max range of the target.
#define INFINITE_RANGE 0xFFFFFFF
	int ManaCost = 0;               /// Required mana for each cast.
	int RepeatCast = 0;             /// If the spell will be cast again until out of targets.
	int Costs[MaxCosts]{};          /// Resource costs of spell.
	int CoolDown = 0;               /// How much time spell needs to be cast again.

	int DependencyId = -1;          /// Id of upgrade, -1 if no upgrade needed for cast the spell.
	std::unique_ptr<ConditionInfo> Condition;   /// Conditions to cast the spell. (generic (no test for each target))

	// Autocast information. No AICast means the AI use AutoCast.
	std::unique_ptr<AutoCastInfo> AutoCast;     /// AutoCast information for your own units
	std::unique_ptr<AutoCastInfo> AICast;       /// AutoCast information for ai. More detailed.

	// Graphics and sounds. Add something else here?
	SoundConfig SoundWhenCast;  /// Sound played if cast

	bool ForceUseAnimation = false;
};

/*----------------------------------------------------------------------------
--  Variables
----------------------------------------------------------------------------*/

/**
**  Define the names and effects of all available spells.
*/
extern std::vector<std::unique_ptr<SpellType>> SpellTypeTable;


/*----------------------------------------------------------------------------
--  Functions
----------------------------------------------------------------------------*/

/// register fonction.
extern void SpellCclRegister();

/// init spell tables
extern void InitSpells();

/// done spell tables
extern void CleanSpells();

/// return 1 if spell is available, 0 if not (must upgrade)
extern bool SpellIsAvailable(const CPlayer &player, int SpellId);

/// returns true if spell can be casted (enough mana, valid target)
extern bool CanCastSpell(const CUnit &caster, const SpellType &spell,
						 const CUnit *target, const Vec2i &goalPos);

/// cast spell on target unit or place at x,y
extern int SpellCast(CUnit &caster, const SpellType &spell,
					 CUnit *target, const Vec2i &goalPos);

/// auto cast the spell if possible
extern bool AutoCastSpell(CUnit &caster, const SpellType &spell);

/// return spell type by ident string
extern SpellType &SpellTypeByIdent(const std::string_view &ident);

/// return ECondition.
extern ECondition Ccl2Condition(lua_State *l, std::string_view value);

std::variant<ECondition, int> Ccl2ConditionOrNumber(lua_State *l, std::string_view value);


//@}

#endif // !__SPELLS_H__
