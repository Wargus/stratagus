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
/**@name spells.cpp - The spell cast action. */
//
//      (c) Copyright 1998-2012 by Vladi Belperchinov-Shabanski, Lutz Sammer,
//                                 Jimmy Salmon, and Joris DAUPHIN
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

/*
** And when we cast our final spell
** And we meet in our dreams
** A place that no one else can go
** Don't ever let your love die
** Don't ever go breaking this spell
*/

//@{

/*----------------------------------------------------------------------------
-- Includes
----------------------------------------------------------------------------*/

#include "stratagus.h"

#include "spells.h"

#include "commands.h"
#include "map.h"
#include "sound.h"
#include "unit.h"
#include "unit_find.h"
#include "upgrade.h"

/*----------------------------------------------------------------------------
-- Variables
----------------------------------------------------------------------------*/

/**
** Define the names and effects of all im play available spells.
*/
std::vector<SpellType *> SpellTypeTable;


/*----------------------------------------------------------------------------
-- Functions
----------------------------------------------------------------------------*/

// ****************************************************************************
// Target constructor
// ****************************************************************************

/**
**  Target constructor for unit.
**
**  @param unit  Target unit.
**
**  @return the new target.
*/
static Target *NewTargetUnit(CUnit &unit)
{
	return new Target(TargetUnit, &unit, unit.tilePos);
}

// ****************************************************************************
// Main local functions
// ****************************************************************************

/**
**  Check the condition.
**
**  @param caster      Pointer to caster unit.
**  @param spell       Pointer to the spell to cast.
**  @param target      Pointer to target unit, or 0 if it is a position spell.
**  @param goalPos     position, or {-1, -1} if it is a unit spell.
**  @param condition   Pointer to condition info.
**
**  @return            true if passed, false otherwise.
*/
static bool PassCondition(const CUnit &caster, const SpellType &spell, const CUnit *target,
						  const Vec2i &/*goalPos*/, const ConditionInfo *condition)
{
	if (caster.Variable[MANA_INDEX].Value < spell.ManaCost) { // Check caster mana.
		return false;
	}
	if (spell.Target == TargetUnit) { // Casting a unit spell without a target.
		if ((!target) || target->IsAlive() == false) {
			return false;
		}
	}
	if (!condition) { // no condition, pass.
		return true;
	}
	for (unsigned int i = 0; i < UnitTypeVar.GetNumberVariable(); i++) { // for custom variables
		const CUnit *unit;

		if (!condition->Variable[i].Check) {
			continue;
		}

		unit = (condition->Variable[i].ConditionApplyOnCaster) ? &caster : target;
		//  Spell should target location and have unit condition.
		if (unit == NULL) {
			continue;
		}
		if (condition->Variable[i].Enable != CONDITION_TRUE) {
			if ((condition->Variable[i].Enable == CONDITION_ONLY) ^ (unit->Variable[i].Enable)) {
				return false;
			}
		}
		// Value and Max
		if (condition->Variable[i].MinValue >= unit->Variable[i].Value) {
			return false;
		}
		if (condition->Variable[i].MaxValue != -1 &&
			condition->Variable[i].MaxValue <= unit->Variable[i].Value) {
			return false;
		}

		if (condition->Variable[i].MinMax >= unit->Variable[i].Max) {
			return false;
		}

		if (!unit->Variable[i].Max) {
			continue;
		}
		// Percent
		if (condition->Variable[i].MinValuePercent * unit->Variable[i].Max
			>= 100 * unit->Variable[i].Value) {
			return false;
		}
		if (condition->Variable[i].MaxValuePercent * unit->Variable[i].Max
			<= 100 * unit->Variable[i].Value) {
			return false;
		}
	}

	if (!target) {
		return true;
	}
	if (!target->Type->CheckUserBoolFlags(condition->BoolFlag)) {
		return false;
	}
	if (condition->Alliance != CONDITION_TRUE) {
		if ((condition->Alliance == CONDITION_ONLY) ^
			// own units could be not allied ?
			(caster.IsAllied(*target) || target->Player == caster.Player)) {
			return false;
		}
	}
	if (condition->Opponent != CONDITION_TRUE) {
		if ((condition->Opponent == CONDITION_ONLY) ^
			(caster.IsEnemy(*target) && 1)) {
			return false;
		}
	}
	if (condition->TargetSelf != CONDITION_TRUE) {
		if ((condition->TargetSelf == CONDITION_ONLY) ^ (&caster == target)) {
			return false;
		}
	}
	return true;
}

/**
**  Select the target for the autocast.
**
**  @param caster    Unit who would cast the spell.
**  @param spell     Spell-type pointer.
**
**  @return          Target* chosen target or Null if spell can't be cast.
**  @todo FIXME: should be global (for AI) ???
**  @todo FIXME: write for position target.
*/
static Target *SelectTargetUnitsOfAutoCast(CUnit &caster, const SpellType &spell)
{
	AutoCastInfo *autocast;

	// Ai cast should be a lot better. Use autocast if not found.
	if (caster.Player->AiEnabled && spell.AICast) {
		autocast = spell.AICast;
	} else {
		autocast = spell.AutoCast;
	}
	Assert(autocast);
	const Vec2i &pos = caster.tilePos;
	int range = autocast->Range;

	// Select all units aroung the caster
	std::vector<CUnit *> table;
	SelectAroundUnit(caster, range, table);

	// Check generic conditions. FIXME: a better way to do this?
	if (autocast->Combat != CONDITION_TRUE) {
		// Check each unit if it is hostile.
		bool inCombat = false;
		for (size_t i = 0; i < table.size(); ++i) {
			const CUnit &target = *table[i];

			// Note that CanTarget doesn't take into account (offensive) spells...
			if (target.IsVisibleAsGoal(*caster.Player) && caster.IsEnemy(target)
				&& (CanTarget(*caster.Type, *target.Type) || CanTarget(*target.Type, *caster.Type))) {
				inCombat = true;
				break;
			}
		}
		if ((autocast->Combat == CONDITION_ONLY) ^ (inCombat)) {
			return NULL;
		}
	}

	switch (spell.Target) {
		case TargetSelf :
			if (PassCondition(caster, spell, &caster, pos, spell.Condition)
				&& PassCondition(caster, spell, &caster, pos, autocast->Condition)) {
				return NewTargetUnit(caster);
			}
			return NULL;
		case TargetPosition:
			return 0;
			//  Autocast with a position? That's hard
			//  Possibilities: cast reveal-map on a dark region
			//  Cast raise dead on a bunch of corpses. That would rule.
			//  Cast summon until out of mana in the heat of battle. Trivial?
			//  Find a tight group of units and cast area-damage spells. HARD,
			//  but it is a must-have for AI. What about area-heal?
		case TargetUnit: {
			// The units are already selected.
			//  Check every unit if it is a possible target

			int n = 0;
			for (size_t i = 0; i != table.size(); ++i) {
				//  FIXME: autocast conditions should include normal conditions.
				//  FIXME: no, really, they should.
				if (PassCondition(caster, spell, table[i], pos, spell.Condition)
					&& PassCondition(caster, spell, table[i], pos, autocast->Condition)) {
					table[n++] = table[i];
				}
			}
			// Now select the best unit to target.
			// FIXME: Some really smart way to do this.
			// FIXME: Heal the unit with the lowest hit-points
			// FIXME: Bloodlust the unit with the highest hit-point
			// FIMXE: it will survive more
			if (n != 0) {
#if 0
				// For the best target???
				sort(table, n, spell.autocast->f_order);
				return NewTargetUnit(*table[0]);
#else
				// Best unit, random unit, oh well, same stuff.
				int index = SyncRand() % n;
				return NewTargetUnit(*table[index]);
#endif
			}
			break;
		}
		default:
			// Something is wrong
			DebugPrint("Spell is screwed up, unknown target type\n");
			Assert(0);
			return NULL;
			break;
	}
	return NULL; // Can't spell the auto-cast.
}

// ****************************************************************************
// Public spell functions
// ****************************************************************************

// ****************************************************************************
// Constructor and destructor
// ****************************************************************************

/**
** Spells constructor, inits spell id's and sounds
*/
void InitSpells()
{
}

/**
**  Get spell-type struct pointer by string identifier.
**
**  @param ident  Spell identifier.
**
**  @return       spell-type struct pointer.
*/
SpellType *SpellTypeByIdent(const std::string &ident)
{
	for (std::vector<SpellType *>::iterator i = SpellTypeTable.begin(); i < SpellTypeTable.end(); ++i) {
		if ((*i)->Ident == ident) {
			return *i;
		}
	}
	return NULL;
}

// ****************************************************************************
// CanAutoCastSpell, CanCastSpell, AutoCastSpell, CastSpell.
// ****************************************************************************

/**
**  Check if spell is research for player \p player.
**  @param player    player for who we want to know if he knows the spell.
**  @param spellid   id of the spell to check.
**
**  @return          0 if spell is not available, else no null.
*/
bool SpellIsAvailable(const CPlayer &player, int spellid)
{
	const int dependencyId = SpellTypeTable[spellid]->DependencyId;

	return dependencyId == -1 || UpgradeIdAllowed(player, dependencyId) == 'R';
}

/**
**  Check if unit can cast the spell.
**
**  @param caster    Unit that casts the spell
**  @param spell     Spell-type pointer
**  @param target    Target unit that spell is addressed to
**  @param goalPos   coord of target spot when/if target does not exist
**
**  @return          =!0 if spell should/can casted, 0 if not
**  @note caster must know the spell, and spell must be researched.
*/
bool CanCastSpell(const CUnit &caster, const SpellType &spell,
				  const CUnit *target, const Vec2i &goalPos)
{
	if (spell.Target == TargetUnit && target == NULL) {
		return false;
	}
	return PassCondition(caster, spell, target, goalPos, spell.Condition);
}

/**
**  Check if the spell can be auto cast and cast it.
**
**  @param caster    Unit who can cast the spell.
**  @param spell     Spell-type pointer.
**
**  @return          1 if spell is casted, 0 if not.
*/
int AutoCastSpell(CUnit &caster, const SpellType &spell)
{
	//  Check for mana, trivial optimization.
	if (!SpellIsAvailable(*caster.Player, spell.Slot)
		|| caster.Variable[MANA_INDEX].Value < spell.ManaCost) {
		return 0;
	}
	Target *target = SelectTargetUnitsOfAutoCast(caster, spell);
	if (target == NULL) {
		return 0;
	} else {
		// Must move before ?
		CommandSpellCast(caster, target->targetPos, target->Unit, spell, FlushCommands);
		delete target;
	}
	return 1;
}

/**
** Spell cast!
**
** @param caster    Unit that casts the spell
** @param spell     Spell-type pointer
** @param target    Target unit that spell is addressed to
** @param goalPos   coord of target spot when/if target does not exist
**
** @return          !=0 if spell should/can continue or 0 to stop
*/
int SpellCast(CUnit &caster, const SpellType &spell, CUnit *target, const Vec2i &goalPos)
{
	Vec2i pos = goalPos;

	caster.Variable[INVISIBLE_INDEX].Value = 0;// unit is invisible until attacks // FIXME: Must be configurable
	if (target) {
		pos = target->tilePos;
	}
	//
	// For TargetSelf, you target.... YOURSELF
	//
	if (spell.Target == TargetSelf) {
		pos = caster.tilePos;
		target = &caster;
	}
	DebugPrint("Spell cast: (%s), %s -> %s (%d,%d)\n" _C_ spell.Ident.c_str() _C_
			   caster.Type->Name.c_str() _C_ target ? target->Type->Name.c_str() : "none" _C_ pos.x _C_ pos.y);
	if (CanCastSpell(caster, spell, target, pos)) {
		int cont = 1; // Should we recast the spell.
		bool mustSubtractMana = true; // false if action which have their own calculation is present.
		//
		//  Ugly hack, CastAdjustVitals makes it's own mana calculation.
		//
		PlayGameSound(spell.SoundWhenCast.Sound, MaxSampleVolume);
		for (std::vector<SpellActionType *>::const_iterator act = spell.Action.begin();
			 act != spell.Action.end(); ++act) {
			if ((*act)->ModifyManaCaster) {
				mustSubtractMana = false;
			}
			cont = cont & (*act)->Cast(caster, spell, target, pos);
		}
		if (mustSubtractMana) {
			caster.Variable[MANA_INDEX].Value -= spell.ManaCost;
		}
		//
		// Spells like blizzard are casted again.
		// This is sort of confusing, we do the test again, to
		// check if it will be possible to cast again. Otherwise,
		// when you're out of mana the caster will try again ( do the
		// anim but fail in this proc.
		//
		if (spell.RepeatCast && cont) {
			return CanCastSpell(caster, spell, target, pos);
		}
	}
	//
	// Can't cast, STOP.
	//
	return 0;
}


/**
**  SpellType constructor.
*/
SpellType::SpellType(int slot, const std::string &ident) :
	Ident(ident), Slot(slot), Target(), Action(),
	Range(0), ManaCost(0), RepeatCast(0),
	DependencyId(-1), Condition(NULL),
	AutoCast(NULL), AICast(NULL)
{
}

/**
**  SpellType destructor.
*/
SpellType::~SpellType()
{
	for (std::vector<SpellActionType *>::iterator act = Action.begin(); act != Action.end(); ++act) {
		delete *act;
	}
	Action.clear();

	delete Condition;
	//
	// Free Autocast.
	//
	delete AutoCast;
	delete AICast;
}


/**
** Cleanup the spell subsystem.
*/
void CleanSpells()
{
	DebugPrint("Cleaning spells.\n");
	for (std::vector<SpellType *>::iterator i = SpellTypeTable.begin(); i < SpellTypeTable.end(); ++i) {
		delete *i;
	}
	SpellTypeTable.clear();
}

//@}
