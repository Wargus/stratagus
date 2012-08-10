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

#include "unitsound.h"
#include "vec2i.h"

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
	SpellActionType(int mod = 0) : ModifyManaCaster(mod) {};
	virtual ~SpellActionType() {};

	virtual int Cast(CUnit &caster, const SpellType &spell,
					 CUnit *target, const Vec2i &goalPos) = 0;
	virtual void Parse(lua_State *l, int startIndex, int endIndex) = 0;

	const int ModifyManaCaster;
};


/**
**  Different targets.
*/
enum TargetType {
	TargetSelf,
	TargetPosition,
	TargetUnit
};

/*
** *******************
** Target definition.
** *******************
*/

class Target
{
public:
	Target(TargetType type, CUnit *unit, const Vec2i &pos) :
		Type(type), Unit(unit), targetPos(pos) {}

	TargetType Type;                  /// type of target.
	CUnit *Unit;                      /// Unit target.
	Vec2i targetPos;
};

/*
** *******************
** Conditions definition.
** *******************
*/

class ConditionInfoVariable
{
public:
	ConditionInfoVariable() : Enable(0), Check(false), MinValue(0), MaxValue(0),
		MinMax(0), MinValuePercent(0), MaxValuePercent(0),
		ConditionApplyOnCaster(0) {};

	char Enable;                /// Target is 'user defined variable'.
	bool Check;                 /// True if need to check that variable.

	int MinValue;               /// Target must have more Value than that.
	int MaxValue;               /// Target must have less Value than that.
	int MinMax;                 /// Target must have more Max than that.
	int MinValuePercent;        /// Target must have more (100 * Value / Max) than that.
	int MaxValuePercent;        /// Target must have less (100 * Value / Max) than that.

	char ConditionApplyOnCaster; /// true if these condition are for caster.
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
	ConditionInfo() : Alliance(0), Opponent(0), TargetSelf(0),
		BoolFlag(NULL), Variable(NULL) {};
	~ConditionInfo() {
		delete[] BoolFlag;
		delete[] Variable;
	};
	//
	//  Conditions that check specific flags. Possible values are the defines below.
	//
#define CONDITION_FALSE 1
#define CONDITION_TRUE  0
#define CONDITION_ONLY  2
	char Alliance;          /// Target is allied. (neutral is neither allied, nor opponent)
	char Opponent;          /// Target is opponent. (neutral is neither allied, nor opponent)
	char TargetSelf;        /// Target is the same as the caster.

	char *BoolFlag;         /// User defined boolean flag.

	ConditionInfoVariable *Variable;
	//
	//  @todo more? feel free to add, here and to
	//  @todo PassCondition, CclSpellParseCondition, SaveSpells
	//
};

/**
**  Informations about the autocasting mode.
*/
class AutoCastInfo
{
public:
	AutoCastInfo() : Range(0), Condition(0), Combat(0) {};
	~AutoCastInfo() { delete Condition; };
	/// @todo this below is SQUARE!!!
	int Range;                   /// Max range of the target.

	ConditionInfo *Condition;    /// Conditions to cast the spell.

	/// Detalied generic conditions (not per-target, where Condition is evaluated.)
	/// Combat mode is when there are hostile non-coward units around
	int Combat;                  /// If it should be casted in combat

	/// @todo Add stuff here for target preference.
	/// @todo Heal units with the lowest hit points first.
};

/**
**  Base structure of a spell type.
*/
class SpellType
{
public:
	SpellType(int slot, const std::string &ident);
	~SpellType();

	// Identification stuff
	std::string Ident;    /// Spell unique identifier (spell-holy-vision)
	std::string Name;     /// Spell name shown by the engine
	int Slot;             /// Spell numeric identifier

	// Spell Specifications
	TargetType Target;          /// Targetting information. See TargetType.
	std::vector<SpellActionType *> Action; /// More arguments for spell (damage, delay, additional sounds...).

	int Range;                  /// Max range of the target.
#define INFINITE_RANGE 0xFFFFFFF
	int ManaCost;               /// Required mana for each cast.
	int RepeatCast;             /// If the spell will be cast again until out of targets.

	int DependencyId;           /// Id of upgrade, -1 if no upgrade needed for cast the spell.
	ConditionInfo *Condition;   /// Conditions to cast the spell. (generic (no test for each target))

	// Autocast informations. No AICast means the AI use AutoCast.
	AutoCastInfo *AutoCast;     /// AutoCast information for your own units
	AutoCastInfo *AICast;       /// AutoCast information for ai. More detalied.

	// Graphics and sounds. Add something else here?
	SoundConfig SoundWhenCast;  /// Sound played if cast

	bool IsCasterOnly() const {
		return !Range && Target == TargetSelf;
	}

};

/*----------------------------------------------------------------------------
--  Variables
----------------------------------------------------------------------------*/

/**
**  Define the names and effects of all available spells.
*/
extern std::vector<SpellType *> SpellTypeTable;


/*----------------------------------------------------------------------------
--  Functions
----------------------------------------------------------------------------*/

/// register fonction.
extern void SpellCclRegister();

/// init spell tables
extern void InitSpells();

/// done spell tables
extern void CleanSpells();

/// return 1 if spell is availible, 0 if not (must upgrade)
extern bool SpellIsAvailable(const CPlayer &player, int SpellId);

/// returns true if spell can be casted (enough mana, valid target)
extern bool CanCastSpell(const CUnit &caster, const SpellType &spell,
						 const CUnit *target, const Vec2i &goalPos);

/// cast spell on target unit or place at x,y
extern int SpellCast(CUnit &caster, const SpellType &spell,
					 CUnit *target, const Vec2i &goalPos);

/// auto cast the spell if possible
extern int AutoCastSpell(CUnit &caster, const SpellType &spell);

/// return spell type by ident string
extern SpellType *SpellTypeByIdent(const std::string &ident);

/// return 0, 1, 2 for true, only, false.
extern char Ccl2Condition(lua_State *l, const char *value);

//@}

#endif // !__SPELLS_H__
