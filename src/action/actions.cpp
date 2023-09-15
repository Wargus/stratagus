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
/**@name actions.cpp - The actions. */
//
//      (c) Copyright 1998-2005 by Lutz Sammer, Russell Smith, and Jimmy Salmon
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

//@{

/*----------------------------------------------------------------------------
--  Includes
----------------------------------------------------------------------------*/

#include <time.h>

#include "stratagus.h"
#include "version.h"

#include "actions.h"

#include "action/action_attack.h"
#include "action/action_board.h"
#include "action/action_build.h"
#include "action/action_built.h"
#include "action/action_defend.h"
#include "action/action_die.h"
#include "action/action_explore.h"
#include "action/action_follow.h"
#include "action/action_move.h"
#include "action/action_patrol.h"
#include "action/action_repair.h"
#include "action/action_research.h"
#include "action/action_resource.h"
#include "action/action_spellcast.h"
#include "action/action_still.h"
#include "action/action_train.h"
#include "action/action_unload.h"
#include "action/action_upgradeto.h"

#include "animation/animation_die.h"
#include "commands.h"
#include "game.h"
#include "interface.h"
#include "luacallback.h"
#include "map.h"
#include "missile.h"
#include "parameters.h"
#include "pathfinder.h"
#include "player.h"
#include "script.h"
#include "spells.h"
#include "unit.h"
#include "unit_find.h"
#include "unit_manager.h"
#include "unittype.h"

#ifdef USE_STACKTRACE
#include <stdexcept>
#include <stacktrace/call_stack.hpp>
#include <stacktrace/stack_exception.hpp>
#else
#include "st_backtrace.h"
#endif

/*----------------------------------------------------------------------------
--  Variables
----------------------------------------------------------------------------*/

unsigned SyncHash; /// Hash calculated to find sync failures


/*----------------------------------------------------------------------------
--  Functions
----------------------------------------------------------------------------*/

COrder::~COrder()
{
	Goal.Reset();
}

void COrder::SetGoal(CUnit *const new_goal)
{
	Goal = new_goal;
}

void COrder::ClearGoal()
{
	Goal.Reset();
}

bool COrder::IsWaiting(CUnit &unit)
{
	if (unit.Wait) {
		if (!unit.Waiting) {
			unit.Waiting = 1;
			unit.WaitBackup = unit.Anim;
		}
		UnitShowAnimation(unit, unit.Type->Animations->Still);
		unit.Wait--;
		return true;
	}
	return false;
}

void COrder::StopWaiting(CUnit &unit)
{
	if (unit.Waiting) {
		unit.Anim = unit.WaitBackup;
		unit.Waiting = 0;
	}
}

void COrder::UpdatePathFinderData_NotCalled(PathFinderInput &input)
{
	Assert(false); // should not be called.

	// Don't move
	input.SetMinRange(0);
	input.SetMaxRange(0);
	const Vec2i tileSize(0, 0);
	input.SetGoal(input.GetUnit()->tilePos, tileSize);

}

/* virtual */ void COrder::FillSeenValues(CUnit &unit) const
{
	unit.Seen.State = ((Action == UnitAction::UpgradeTo) << 1);
	if (unit.CurrentAction() == UnitAction::Die) {
		unit.Seen.State = 3;
	}
	unit.Seen.CFrame = nullptr;
}

/* virtual */ bool COrder::OnAiHitUnit(CUnit &unit, CUnit *attacker, int /*damage*/)
{
	return false;
}


/** Called when unit is killed.
**  warn the AI module.
*/
/* virtual */ void COrder::AiUnitKilled(CUnit &unit)
{
	switch (Action) {
		case UnitAction::Still:
		case UnitAction::Attack:
		case UnitAction::Move:
			break;
		default:
			DebugPrint("FIXME: %d: %d(%s) killed, with order %d!\n",
			           unit.Player->Index,
			           UnitNumber(unit),
			           unit.Type->Ident.c_str(),
			           Action);
			break;
	}
}

/**
**  Call when animation step is "attack"
*/
/* virtual */ void COrder::OnAnimationAttack(CUnit &unit)
{
	if (unit.Type->CanAttack == false) {
		return;
	}
	CUnit *goal = AttackUnitsInRange(unit);

	if (goal != nullptr) {
		const Vec2i invalidPos(-1, -1);

		FireMissile(unit, goal, invalidPos);
		UnHideUnit(unit); // unit is invisible until attacks
	}
	// Fixme : Auto select position to attack ?
}

/**
**  Get goal position
*/
/* virtual */ const Vec2i COrder::GetGoalPos() const
{
	const Vec2i invalidPos(-1, -1);
	if (this->HasGoal()) {
		return this->GetGoal()->tilePos;
	}
	return invalidPos;
}

/**
**  Parse order
**
**  @param l      Lua state.
**  @param order  OUT: resulting order.
*/
void CclParseOrder(lua_State *l, CUnit &unit, COrderPtr *orderPtr)
{
	const int args = lua_rawlen(l, -1);
	const std::string_view actiontype = LuaToString(l, -1, 1);

	if (actiontype == "action-attack") {
		*orderPtr = new COrder_Attack(false);
	} else if (actiontype == "action-attack-ground") {
		*orderPtr = new COrder_Attack(true);
	} else if (actiontype == "action-board") {
		*orderPtr = new COrder_Board;
	} else if (actiontype == "action-build") {
		*orderPtr = new COrder_Build;
	} else if (actiontype == "action-built") {
		*orderPtr = new COrder_Built;
	} else if (actiontype == "action-defend") {
		*orderPtr = new COrder_Defend;
	} else if (actiontype == "action-die") {
		*orderPtr = new COrder_Die;
	} else if (actiontype == "action-explore") {
		*orderPtr = new COrder_Explore;
	} else if (actiontype == "action-follow") {
		*orderPtr = new COrder_Follow;
	} else if (actiontype == "action-move") {
		*orderPtr = new COrder_Move;
	} else if (actiontype == "action-patrol") {
		*orderPtr = new COrder_Patrol;
	} else if (actiontype == "action-repair") {
		*orderPtr = new COrder_Repair;
	} else if (actiontype == "action-research") {
		*orderPtr = new COrder_Research;
	} else if (actiontype == "action-resource") {
		*orderPtr = new COrder_Resource(unit);
	} else if (actiontype == "action-spell-cast") {
		*orderPtr = new COrder_SpellCast;
	} else if (actiontype == "action-stand-ground") {
		*orderPtr = new COrder_Still(true);
	} else if (actiontype == "action-still") {
		*orderPtr = new COrder_Still(false);
	} else if (actiontype == "action-train") {
		*orderPtr = new COrder_Train;
	} else if (actiontype == "action-transform-into") {
		*orderPtr = new COrder_TransformInto;
	} else if (actiontype == "action-upgrade-to") {
		*orderPtr = new COrder_UpgradeTo;
	} else if (actiontype == "action-unload") {
		*orderPtr = new COrder_Unload;
	} else {
		LuaError(l, "ParseOrder: Unsupported type: %s", actiontype.data());
	}

	COrder &order = **orderPtr;

	for (int j = 1; j < args; ++j) {
		const std::string_view value = LuaToString(l, -1, j + 1);

		if (order.ParseGenericData(l, j, value)) {
			continue;
		} else if (order.ParseSpecificData(l, j, value, unit)) {
			continue;
		} else {
			// This leaves a half initialized unit
			LuaError(l, "ParseOrder: Unsupported tag: %s", value.data());
		}
	}
}


/*----------------------------------------------------------------------------
--  Actions
----------------------------------------------------------------------------*/

static inline void IncreaseVariable(CUnit &unit, int index)
{
	unit.Variable[index].Value += unit.Variable[index].Increase;
	clamp(&unit.Variable[index].Value, 0, unit.Variable[index].Max);
	
	//if variable is HP and increase is negative, unit dies if HP reached 0
	if (index == HP_INDEX && unit.Variable[HP_INDEX].Value <= 0) {
		LetUnitDie(unit);
	}
}

/**
**  Handle things about the unit that decay over time each cycle
**
**  @param unit    The unit that the decay is handled for
*/
static void HandleBuffsEachCycle(CUnit &unit)
{
	// Look if the time to live is over.
	if (unit.TTL && unit.IsAlive() && unit.TTL < GameCycle) {
		DebugPrint("Unit must die %lu %lu!\n", unit.TTL, GameCycle);

		// Hit unit does some funky stuff...
		--unit.Variable[HP_INDEX].Value;
		if (unit.Variable[HP_INDEX].Value <= 0) {
			LetUnitDie(unit);
			return;
		}
	}

	if (--unit.Threshold < 0) {
		unit.Threshold = 0;
	}
	if (--unit.UnderAttack < 0) {
		unit.UnderAttack = 0;
	}

	if (!unit.Type->CanCastSpell.empty()) {
		// decrease spell countdown timers
		for (unsigned int i = 0; i < SpellTypeTable.size(); ++i) {
			if (unit.SpellCoolDownTimers[i] > 0) {
				--unit.SpellCoolDownTimers[i];
			}
		}
	}

	const int SpellEffects[] = {BLOODLUST_INDEX, HASTE_INDEX, SLOW_INDEX, INVISIBLE_INDEX, UNHOLYARMOR_INDEX, POISON_INDEX};
	//  decrease spells effects time.
	for (unsigned int i = 0; i < sizeof(SpellEffects) / sizeof(int); ++i) {
		unit.Variable[SpellEffects[i]].Increase = -1;
		IncreaseVariable(unit, SpellEffects[i]);
	}
}

/**
**  Modify unit's health according to burn and poison
**
**  @param unit  the unit to operate on
*/
static bool HandleBurnAndPoison(CUnit &unit)
{
	if (unit.Removed || unit.Destroyed || unit.Variable[HP_INDEX].Max == 0
		|| unit.CurrentAction() == UnitAction::Built
		|| unit.CurrentAction() == UnitAction::Die) {
		return false;
	}
	// Burn & poison
	const int hpPercent = (100 * unit.Variable[HP_INDEX].Value) / unit.Variable[HP_INDEX].Max;
	if (hpPercent <= unit.Type->BurnPercent && unit.Type->BurnDamageRate) {
		HitUnit(NoUnitP, unit, unit.Type->BurnDamageRate);
		return true;
	}
	if (unit.Variable[POISON_INDEX].Value && unit.Type->PoisonDrain) {
		HitUnit(NoUnitP, unit, unit.Type->PoisonDrain);
		return true;
	}
	return false;
}

/**
**  Handle things about the unit that decay over time each second
**
**  @param unit    The unit that the decay is handled for
*/
static void HandleBuffsEachSecond(CUnit &unit)
{
	// User defined variables
	for (unsigned int i = 0; i < UnitTypeVar.GetNumberVariable(); i++) {
		if (i == BLOODLUST_INDEX || i == HASTE_INDEX || i == SLOW_INDEX
			|| i == INVISIBLE_INDEX || i == UNHOLYARMOR_INDEX || i == POISON_INDEX) {
			continue;
		}
		if (i == HP_INDEX && HandleBurnAndPoison(unit)) {
			continue;
		}
		unsigned char freq = unit.Variable[i].IncreaseFrequency;
		if (unit.Variable[i].Enable && unit.Variable[i].Increase &&
				(freq == 0 || freq == 1 || (GameCycle / CYCLES_PER_SECOND % freq) == 0)) {
			IncreaseVariable(unit, i);
		}
	}
}

/**
**  Handle the action of a unit.
**
**  @param unit  Pointer to handled unit.
*/
static void HandleUnitAction(CUnit &unit)
{
	// If current action is breakable proceed with next one.
	if (!unit.Anim.Unbreakable) {
		if (unit.CriticalOrder != nullptr) {
			unit.CriticalOrder->Execute(unit);
			delete unit.CriticalOrder;
			unit.CriticalOrder = nullptr;
		}

		if (unit.Orders[0]->Finished && unit.Orders[0]->Action != UnitAction::Still
			&& unit.Orders.size() == 1) {

			delete unit.Orders[0];
			unit.Orders[0] = COrder::NewActionStill();
			if (IsOnlySelected(unit)) { // update display for new action
				SelectedUnitChanged();
			}
		}

		// o Look if we have a new order and old finished.
		// o Or the order queue should be flushed.
		if ((unit.CurrentAction() == UnitAction::StandGround || unit.CurrentOrder()->Finished)
			&& unit.Orders.size() > 1) {
			if (unit.Removed && unit.CurrentAction() != UnitAction::Board) { // FIXME: johns I see this as an error
				if (unit.CurrentAction() == UnitAction::Still) {
					// timfel: I observed this exactly once and could never reproduce it, so I don't know why this
					// can happen, but in case it happens again, bring the unit back
					DebugPrint("Dropping out stuck unit\n");
					DropOutOnSide(unit, LookingW, nullptr);
				} else {
					DebugPrint("Flushing removed unit\n");
					// This happens, if building with ALT+SHIFT.
					return;
				}
			}

			delete unit.Orders[0];
			unit.Orders.erase(unit.Orders.begin());

			unit.Wait = 0;
			if (IsOnlySelected(unit)) { // update display for new action
				SelectedUnitChanged();
			}
		}
		if (unit.CurrentResource) {
			const ResourceInfo &resinfo = *unit.Type->ResInfo[unit.CurrentResource];
			if (resinfo.LoseResources && unit.Orders[0]->Action != UnitAction::Resource) {
				unit.CurrentResource = 0;
				unit.ResourcesHeld = 0;
			}
		}
	}
	unit.Orders[0]->Execute(unit);
}

template <typename UNITP_ITERATOR>
static void UnitActionsEachSecond(UNITP_ITERATOR begin, UNITP_ITERATOR end)
{
	for (UNITP_ITERATOR it = begin; it != end; ++it) {
		CUnit &unit = **it;

		if (unit.Destroyed) {
			continue;
		}

		// OnEachSecond callback
		if (unit.Type->OnEachSecond  && unit.IsUnusable(false) == false) {
			unit.Type->OnEachSecond->call(UnitNumber(unit));
		}

		// 1) Blink flag.
		if (unit.Blink) {
			--unit.Blink;
		}
		// 2) Buffs...
		HandleBuffsEachSecond(unit);
	}
}

static const char* toCStr(UnitAction action)
{
	switch (action) {
		case UnitAction::None: return "None";
		case UnitAction::Still: return "Still";
		case UnitAction::StandGround: return "StandGround";
		case UnitAction::Follow: return "Follow";
		case UnitAction::Defend: return "Defend";
		case UnitAction::Move: return "Move";
		case UnitAction::Attack: return "Attack";
		case UnitAction::AttackGround: return "AttackGround";
		case UnitAction::Die: return "Die";
		case UnitAction::SpellCast: return "SpellCast";
		case UnitAction::Train: return "Train";
		case UnitAction::UpgradeTo: return "UpgradeTo";
		case UnitAction::Research: return "Research";
		case UnitAction::Built: return "Built";
		case UnitAction::Board: return "Board";
		case UnitAction::Unload: return "Unload";
		case UnitAction::Patrol: return "Patrol";
		case UnitAction::Build: return "Build";
		case UnitAction::Explore: return "Explore";
		case UnitAction::Repair: return "Repair";
		case UnitAction::Resource: return "Resource";
		case UnitAction::TransformInto: return "TransformInto";
	}
	return "";
}

static void DumpUnitInfo(CUnit &unit)
{
	// Dump the unit to find the network sync bugs.
	static FILE *logf = nullptr;

	if (!logf) {
		time_t now;
		time(&now);

		fs::path path(Parameters::Instance.GetUserDirectory());
		if (!GameName.empty()) {
			path /= GameName;
		}
		path /= "logs";
		fs::create_directories(path);

		path /= "unit_log_of_stratagus_" + std::to_string(ThisPlayer->Index) + "_"
		      + std::to_string((intmax_t) now) + ".log";

		logf = fopen(path.string().c_str(), "wb");
		if (!logf) {
			DebugPrint("Could not open %s for writing\n", path.u8string().c_str());
			return ;
		}
		fprintf(logf, "; Log file generated by Stratagus Version " VERSION "\n");
		fprintf(logf, ";\tDate: %s", ctime(&now));
		fprintf(logf, ";\tMap: %s\n\n", Map.Info.Description.c_str());
	}

	fprintf(logf, "%lu: ", GameCycle);

	const char *currentAction = unit.Orders.empty() ? "No Orders" : toCStr(unit.CurrentAction());

	fprintf(logf, "%d %s %s P%d Refs %d: Seed %X Hash %X %d@%d %d@%d\n",
			UnitNumber(unit), unit.Type ? unit.Type->Ident.c_str() : "unit-killed",
			currentAction, unit.Player ? unit.Player->Index : -1, unit.Refs,
			SyncRandSeed, SyncHash, unit.tilePos.x, unit.tilePos.y, unit.IX, unit.IY);
#if 0
	SaveUnit(unit, logf);
#endif
	fflush(nullptr);
}

template <typename UNITP_ITERATOR>
static void UnitActionsEachCycle(UNITP_ITERATOR begin, UNITP_ITERATOR end)
{
	for (UNITP_ITERATOR it = begin; it != end; ++it) {
		CUnit &unit = **it;

		if (unit.Destroyed) {
			continue;
		}

		if (!ReplayRevealMap && unit.Selected && !unit.IsVisible(*ThisPlayer)) {
			UnSelectUnit(unit);
			SelectionChanged();
		}

		// OnEachCycle callback
		if (unit.Type->OnEachCycle && unit.IsUnusable(false) == false) {
			unit.Type->OnEachCycle->call(UnitNumber(unit));
		}

		// Handle each cycle buffs
		HandleBuffsEachCycle(unit);
		// Unit could be dead after TTL kill
		if (unit.Destroyed) {
			continue;
		}

		try {
			HandleUnitAction(unit);
		} catch (AnimationDie_Exception &) {
			AnimationDie_OnCatch(unit);
		}

		if (EnableUnitDebug) {
			DumpUnitInfo(unit);
		}

		// Calculate some hash.
		SyncHash = (SyncHash << 5) | (SyncHash >> 27);
		SyncHash ^= unit.Orders.empty() == false
		              ? static_cast<std::underlying_type_t<UnitAction>>(unit.CurrentAction()) << 18
		              : 0;
		SyncHash ^= unit.Refs << 3;

		if (EnableUnitDebug) {
			const char *currentAction =
				unit.Orders.empty() ? "No Orders" : toCStr(unit.CurrentAction());

			fprintf(stderr,
			        "GameCycle: %lud, new SyncHash: %x (unit: %d:%s, order: %s, refs: %d)\n",
			        GameCycle,
			        SyncHash,
			        UnitNumber(unit),
			        unit.Type->Ident.c_str(),
			        currentAction,
			        unit.Refs);
			print_backtrace(8);
			fflush(stderr);
		}
	}
}


/**
**  Update the actions of all units each game cycle/second.
*/
void UnitActions()
{
	const bool isASecondCycle = !(GameCycle % CYCLES_PER_SECOND);
	// Unit list may be modified during loop... so make a copy
	std::vector<CUnit *> table(UnitManager->begin(), UnitManager->end());

	// Check for things that only happen every second
	if (isASecondCycle) {
		UnitActionsEachSecond(table.begin(), table.end());
	}
	// Do all actions
	UnitActionsEachCycle(table.begin(), table.end());
}

//@}
