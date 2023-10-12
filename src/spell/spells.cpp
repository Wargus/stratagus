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

#include "actions.h"
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
std::vector<std::unique_ptr<SpellType>> SpellTypeTable;


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
static std::unique_ptr<Target> NewTargetUnit(CUnit &unit)
{
	return std::make_unique<Target>(TargetUnit, &unit, unit.tilePos);
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
						  const Vec2i &goalPos, const ConditionInfo *condition)
{
	if (caster.Variable[MANA_INDEX].Value < spell.ManaCost) { // Check caster mana.
		return false;
	}
	// check countdown timer
	if (caster.SpellCoolDownTimers[spell.Slot]) { // Check caster mana.
		return false;
	}
	// Check caster's resources
	if (caster.Player->CheckCosts(spell.Costs, false)) {
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
		if (!condition->Variable[i].Check) {
			continue;
		}

		const CUnit *unit = (condition->Variable[i].ConditionApplyOnCaster) ? &caster : target;
		//  Spell should target location and have unit condition.
		if (unit == nullptr) {
			continue;
		}
		if (condition->Variable[i].Enable != ECondition::Ignore) {
			if ((condition->Variable[i].Enable == ECondition::ShouldBeTrue)
			    ^ (unit->Variable[i].Enable)) {
				return false;
			}
		}
		// Value and Max
		if (condition->Variable[i].ExactValue != -1 &&
			condition->Variable[i].ExactValue != unit->Variable[i].Value) {
			return false;
		}
		if (condition->Variable[i].ExceptValue != -1 &&
			condition->Variable[i].ExceptValue == unit->Variable[i].Value) {
			return false;
		}
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
	if (target && !target->Type->CheckUserBoolFlags(condition->BoolFlag)) {
		return false;
	}

	if (condition->CheckFunc) {
		auto res = condition->CheckFunc->call<bool>(spell.Ident, UnitNumber(caster), goalPos.x, goalPos.y, (target && target->IsAlive()) ? UnitNumber(*target) : -1);
		if (res == false) {
			return false;
		}
	}
	if (!target) {
		return true;
	}

	if (condition->Alliance != ECondition::Ignore) {
		// own units could be not allied ?
		if ((condition->Alliance == ECondition::ShouldBeTrue)
		    ^ (caster.IsAllied(*target) || target->Player == caster.Player)) {
			return false;
		}
	}
	if (condition->Opponent != ECondition::Ignore) {
		if ((condition->Opponent == ECondition::ShouldBeTrue) ^ caster.IsEnemy(*target)) {
			return false;
		}
	}
	if (condition->TargetSelf != ECondition::Ignore) {
		if ((condition->TargetSelf == ECondition::ShouldBeTrue) ^ (&caster == target)) {
			return false;
		}
	}
	return true;
}

class AutoCastPrioritySort
{
public:
	AutoCastPrioritySort(const CUnit &caster, const int var, const bool reverse) :
		caster(caster), variable(var), reverse(reverse) {}
	bool operator()(const CUnit *lhs, const CUnit *rhs) const
	{
		if (variable == ACP_DISTANCE) {
			if (reverse) {
				return lhs->MapDistanceTo(caster) > rhs->MapDistanceTo(caster);
			} else {
				return lhs->MapDistanceTo(caster) < rhs->MapDistanceTo(caster);
			}
		} else {
			if (reverse) {
				return lhs->Variable[variable].Value > rhs->Variable[variable].Value;
			} else {
				return lhs->Variable[variable].Value < rhs->Variable[variable].Value;
			}
		}
	}
private:
	const CUnit &caster;
	const int variable;
	const bool reverse;
};

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
static std::unique_ptr<Target> SelectTargetUnitsOfAutoCast(CUnit &caster, const SpellType &spell)
{
	AutoCastInfo *autocast;

	// Ai cast should be a lot better. Use autocast if not found.
	if (caster.Player->AiEnabled && spell.AICast) {
		autocast = spell.AICast.get();
	} else {
		autocast = spell.AutoCast.get();
	}
	Assert(autocast);
	const Vec2i &pos = caster.tilePos;
	int range = autocast->Range;
	int minRange = autocast->MinRange;

	// Select all units around the caster
	std::vector<CUnit *> table = SelectAroundUnit(caster, range, OutOfMinRange(minRange, caster.tilePos));
	if (minRange == 0)
		table.push_back(&caster); // Allow self as target (we check conditions later)

	// Check generic conditions. FIXME: a better way to do this?
	if (autocast->Combat != ECondition::Ignore) {
		// Check each unit if it is hostile.
		const bool inCombat =
			ranges::find_if(table,
		                    [&](const auto *target) {
								// Note that CanTarget doesn't take into account (offensive) spells...
								return target->IsVisibleAsGoal(*caster.Player)
			                        && caster.IsEnemy(*target)
			                        && (CanTarget(*caster.Type, *target->Type)
			                            || CanTarget(*target->Type, *caster.Type));
							})
			!= table.end();
		if ((autocast->Combat == ECondition::ShouldBeTrue) ^ (inCombat)) {
			return nullptr;
		}
	}

	switch (spell.Target) {
		case TargetSelf :
			if (PassCondition(caster, spell, &caster, pos, spell.Condition.get())
				&& PassCondition(caster, spell, &caster, pos, autocast->Condition.get())) {
				return NewTargetUnit(caster);
			}
			return nullptr;
		case TargetPosition: {
			if (autocast->PositionAutoCast && table.empty() == false) {
				size_t count = 0;
				for (size_t i = 0; i != table.size(); ++i) {
					// Check for corpse
					if (autocast->Corpse == ECondition::ShouldBeTrue) {
						if (table[i]->CurrentAction() != UnitAction::Die) {
							continue;
						}
					} else if (autocast->Corpse == ECondition::ShouldBeFalse) {
						if (table[i]->CurrentAction() == UnitAction::Die || table[i]->IsAlive() == false) {
							continue;
						}
					}
					if (PassCondition(caster, spell, table[i], pos, spell.Condition.get())
						&& PassCondition(caster, spell, table[i], pos, autocast->Condition.get())) {
							table[count++] = table[i];
					}
				}
				if (count > 0) {
					if (autocast->PriorytyVar != ACP_NOVALUE) {
						std::sort(table.begin(), table.begin() + count,
							AutoCastPrioritySort(caster, autocast->PriorytyVar, autocast->ReverseSort));
					}
					std::vector<int> array(count + 1);
					for (size_t i = 1; i != count + 1; ++i) {
						array[i] = UnitNumber(*table[i - 1]);
					}
					array[0] = UnitNumber(caster);
					const auto [x, y] = autocast->PositionAutoCast->call<int, int>(array);
					Vec2i resPos(x, y);
					if (Map.Info.IsPointOnMap(resPos)) {
						return std::make_unique<Target>(TargetPosition, nullptr, resPos);
					}
				}
			}
			return nullptr;
		}
		case TargetUnit: {
			// The units are already selected.
			//  Check every unit if it is a possible target

			int n = 0;
			for (size_t i = 0; i != table.size(); ++i) {
				// Check if unit in battle
				if (autocast->Attacker == ECondition::ShouldBeTrue) {
					const int range = table[i]->Player->Type == PlayerTypes::PlayerPerson ? table[i]->Type->ReactRangePerson : table[i]->Type->ReactRangeComputer;
					if ((table[i]->CurrentAction() != UnitAction::Attack
						 && table[i]->CurrentAction() != UnitAction::AttackGround
						 && table[i]->CurrentAction() != UnitAction::SpellCast)
						|| table[i]->CurrentOrder()->HasGoal() == false
						|| table[i]->MapDistanceTo(table[i]->CurrentOrder()->GetGoalPos()) > range) {
						continue;
					}
				}
				// Check for corpse
				if (autocast->Corpse == ECondition::ShouldBeTrue) {
					if (table[i]->CurrentAction() != UnitAction::Die) {
						continue;
					}
				} else if (autocast->Corpse == ECondition::ShouldBeFalse) {
					if (table[i]->CurrentAction() == UnitAction::Die) {
						continue;
					}
				}
				if (PassCondition(caster, spell, table[i], pos, spell.Condition.get())
					&& PassCondition(caster, spell, table[i], pos, autocast->Condition.get())) {
					table[n++] = table[i];
				}
			}
			// Now select the best unit to target.
			if (n != 0) {
				// For the best target???
				if (autocast->PriorytyVar != ACP_NOVALUE) {
					std::sort(table.begin(), table.begin() + n,
							  AutoCastPrioritySort(caster, autocast->PriorytyVar, autocast->ReverseSort));
					return NewTargetUnit(*table[0]);
				} else { // Use the old behavior
					return NewTargetUnit(*table[SyncRand() % n]);
				}
			}
			break;
		}
		default:
			// Something is wrong
			DebugPrint("Spell is screwed up, unknown target type\n");
			Assert(0);
			return nullptr;
	}
	return nullptr; // Can't spell the auto-cast.
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
SpellType &SpellTypeByIdent(const std::string_view &ident)
{
	auto it = ranges::find(SpellTypeTable, ident, &SpellType::Ident);
	if (it != SpellTypeTable.end()) {
		return **it;
	}
	DebugPrint("Unknown spellType '%s'\n", ident.data());
	ExitFatal(1);
}

// ****************************************************************************
// CanAutoCastSpell, CanCastSpell, AutoCastSpell, CastSpell.
// ****************************************************************************

/**
**  Check if spell is research for player \p player.
**  @param player    player for who we want to know if he knows the spell.
**  @param spellid   id of the spell to check.
**
**  @return          false if spell is not available, else true.
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
**  @return          true if spell should/can casted, false if not
**  @note caster must know the spell, and spell must be researched.
*/
bool CanCastSpell(const CUnit &caster, const SpellType &spell,
				  const CUnit *target, const Vec2i &goalPos)
{
	if (spell.Target == TargetUnit && target == nullptr) {
		return false;
	}
	return PassCondition(caster, spell, target, goalPos, spell.Condition.get());
}

/**
**  Check if the spell can be auto cast and cast it.
**
**  @param caster    Unit who can cast the spell.
**  @param spell     Spell-type pointer.
**
**  @return          true if spell is casted, false if not.
*/
bool AutoCastSpell(CUnit &caster, const SpellType &spell)
{
	//  Check for mana and cooldown time, trivial optimization.
	if (!SpellIsAvailable(*caster.Player, spell.Slot)
		|| caster.Variable[MANA_INDEX].Value < spell.ManaCost
		|| caster.SpellCoolDownTimers[spell.Slot]) {
		return false;
	}
	auto target = SelectTargetUnitsOfAutoCast(caster, spell);
	if (target == nullptr) {
		return false;
	} else {
		// Save previous order
		std::unique_ptr<COrder> savedOrder;
		if (caster.CurrentAction() != UnitAction::Still && caster.CanStoreOrder(caster.CurrentOrder())) {
			savedOrder = caster.CurrentOrder()->Clone();
		}
		// Must move before?
		CommandSpellCast(caster, target->targetPos, target->Unit, spell, FlushCommands, true);
		if (savedOrder != nullptr) {
			caster.SavedOrder = std::move(savedOrder);
		}
	}
	return true;
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
	DebugPrint("Spell cast: (%s), %s -> %s (%d,%d)\n",
	           spell.Ident.c_str(),
	           caster.Type->Name.c_str(),
	           target ? target->Type->Name.c_str() : "none",
	           pos.x,
	           pos.y);
	if (CanCastSpell(caster, spell, target, pos)) {
		int cont = 1; // Should we recast the spell.
		bool mustSubtractMana = true; // false if action which have their own calculation is present.
		//
		//  Ugly hack, CastAdjustVitals makes it's own mana calculation.
		//
		if (spell.SoundWhenCast.Sound) {
			if (spell.Target == TargetSelf) {
				PlayUnitSound(caster, spell.SoundWhenCast.Sound);
			} else {
				PlayGameSound(spell.SoundWhenCast.Sound, CalculateVolume(false, ViewPointDistance(target ? target->tilePos : goalPos), spell.SoundWhenCast.Sound->Range));
			}
		}
		for (auto& act : spell.Action) {
			if (act->ModifyManaCaster) {
				mustSubtractMana = false;
			}
			cont = cont & act->Cast(caster, spell, target, pos);
		}
		if (mustSubtractMana) {
			caster.Variable[MANA_INDEX].Value -= spell.ManaCost;
		}
		caster.Player->SubCosts(spell.Costs);
		caster.SpellCoolDownTimers[spell.Slot] = spell.CoolDown;
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
** Cleanup the spell subsystem.
*/
void CleanSpells()
{
	DebugPrint("Cleaning spells.\n");
	SpellTypeTable.clear();
}

//@}
