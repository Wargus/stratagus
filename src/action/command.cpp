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
/**@name command.cpp - Give units a command. */
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

//@{

/*----------------------------------------------------------------------------
--  Includes
----------------------------------------------------------------------------*/

#include "stratagus.h"

#include "actions.h"
#include "action/action_built.h"
#include "action/action_build.h"
#include "action/action_research.h"
#include "action/action_train.h"
#include "action/action_upgradeto.h"
#include "commands.h"
#include "map.h"
#include "pathfinder.h"
#include "player.h"
#include "spells.h"
#include "translate.h"
#include "upgrade.h"
#include "ui.h"
#include "unit.h"
#include "unit_manager.h"
#include "unittype.h"

/*----------------------------------------------------------------------------
--  Functions
----------------------------------------------------------------------------*/

/**
**  Release all orders of a unit.
**
**  @param unit  Pointer to unit.
*/
static void ReleaseOrders(CUnit &unit)
{
	Assert(unit.Orders.empty() == false);

	// Order 0 must be stopped in the action loop.
	for (size_t i = 0; i != unit.Orders.size(); ++i) {
		if (unit.Orders[i]->Action == UnitAction::Built) {
			(dynamic_cast<COrder_Built *>(unit.Orders[i].get()))->Cancel(unit);
		} if (unit.Orders[i]->Action == UnitAction::Build) {
			(dynamic_cast<COrder_Build *>(unit.Orders[i].get()))->Cancel(unit);
		} else if (unit.Orders[i]->Action == UnitAction::Research) {
			(dynamic_cast<COrder_Research *>(unit.Orders[i].get()))->Cancel(unit);
		} else if (unit.Orders[i]->Action == UnitAction::Train) {
			(dynamic_cast<COrder_Train *>(unit.Orders[i].get()))->Cancel(unit);
		} else if (unit.Orders[i]->Action == UnitAction::UpgradeTo) {
			(dynamic_cast<COrder_UpgradeTo *>(unit.Orders[i].get()))->Cancel(unit);
		}
	}
	unit.Orders.resize(1);
	unit.Orders[0]->Finished = true;
}

/**
**  Get next free order slot.
**
**  @param unit   pointer to unit.
**  @param flush  if On, flush order queue.
**
**  @return       Pointer to next free order slot.
*/
static std::unique_ptr<COrder> *GetNextOrder(CUnit &unit, EFlushMode flush)
{
	if (flush == EFlushMode::On) {
		// empty command queue
		ReleaseOrders(unit);
	}
	// FIXME : Remove Hardcoded value.
	const unsigned int maxOrderCount = 0x7F;

	if (unit.Orders.size() == maxOrderCount) {
		return nullptr;
	}
	unit.Orders.push_back(nullptr);
	return &unit.Orders.back();
}

/**
**  Remove an order from the list of orders pending
**
**  @param unit   pointer to unit
**  @param order  number of the order to remove
*/
static void RemoveOrder(CUnit &unit, unsigned int order)
{
	Assert(order < unit.Orders.size());

	unit.Orders.erase(unit.Orders.begin() + order);
	if (unit.Orders.empty()) {
		unit.Orders.push_back(COrder::NewActionStill());
	}
}

static void ClearNewAction(CUnit &unit)
{
	unit.NewOrder = nullptr;
}

/**
**  Clear the saved action.
**
**  @param unit  Unit pointer, that get the saved action cleared.
**
**  @note        If we make a new order, we must clear any saved actions.
**  @note        Internal functions, must protect it, if needed.
*/
static void ClearSavedAction(CUnit &unit)
{
	unit.SavedOrder = nullptr;
}

static bool IsUnitValidForNetwork(const CUnit &unit)
{
	return !unit.Removed && unit.CurrentAction() != UnitAction::Die;
}


/*----------------------------------------------------------------------------
--  Commands
----------------------------------------------------------------------------*/

/**
**  Stop unit.
**
**  @param unit  pointer to unit.
*/
void CommandStopUnit(CUnit &unit)
{
	// Ignore that the unit could be removed.
	auto *order = GetNextOrder(unit, EFlushMode::On); // Flush them.
	Assert(order);
	*order = COrder::NewActionStill();

	ClearSavedAction(unit);
	ClearNewAction(unit);
}

/**
**  Stand ground.
**
**  @param unit   pointer to unit.
**  @param flush  If On, flush command queue.
*/
void CommandStandGround(CUnit &unit, EFlushMode flush)
{
	std::unique_ptr<COrder> *order = nullptr;

	if (unit.Type->Building) {
		ClearNewAction(unit);
		order = &unit.NewOrder;
	} else {
		order = GetNextOrder(unit, flush);
		if (order == nullptr) {
			return;
		}
	}
	*order = COrder::NewActionStandGround();
	ClearSavedAction(unit);
}

/**
**  Follow unit and defend it
**
**  @param unit   pointer to unit.
**  @param dest   unit to follow
**  @param flush  If On, flush command queue.
*/
void CommandDefend(CUnit &unit, CUnit &dest, EFlushMode flush)
{
	if (IsUnitValidForNetwork(unit) == false) {
		return ;
	}
	std::unique_ptr<COrder> *order = nullptr;

	if (!unit.CanMove()) {
		ClearNewAction(unit);
		order = &unit.NewOrder;
	} else {
		order = GetNextOrder(unit, flush);
		if (order == nullptr) {
			return;
		}
	}
	*order = COrder::NewActionDefend(dest);
	ClearSavedAction(unit);
}

/**
**  Follow unit to new position
**
**  @param unit   pointer to unit.
**  @param dest   unit to be followed
**  @param flush  If On, flush command queue.
*/
void CommandFollow(CUnit &unit, CUnit &dest, EFlushMode flush)
{
	if (IsUnitValidForNetwork(unit) == false) {
		return ;
	}
	std::unique_ptr<COrder> *order = nullptr;

	if (!unit.CanMove()) {
		ClearNewAction(unit);
		order = &unit.NewOrder;
	} else {
		order = GetNextOrder(unit, flush);
		if (order == nullptr) {
			return;
		}
	}
	*order = COrder::NewActionFollow(dest);
	ClearSavedAction(unit);
}

/**
**  Move unit to new position
**
**  @param unit   pointer to unit.
**  @param pos    map position to move to.
**  @param flush  If On, flush command queue.
*/
void CommandMove(CUnit &unit, const Vec2i &pos, EFlushMode flush)
{
	Assert(Map.Info.IsPointOnMap(pos));

	if (IsUnitValidForNetwork(unit) == false) {
		return ;
	}
	std::unique_ptr<COrder> *order = nullptr;

	if (!unit.CanMove()) {
		ClearNewAction(unit);
		order = &unit.NewOrder;
	} else {
		order = GetNextOrder(unit, flush);
		if (order == nullptr) {
			return;
		}
	}
	*order = COrder::NewActionMove(pos);
	ClearSavedAction(unit);
}

/**
**  Repair unit
**
**  @param unit   pointer to unit.
**  @param pos    map position to repair.
**  @param dest   or unit to be repaired. FIXME: not supported
**  @param flush  If On, flush command queue.
*/
void CommandRepair(CUnit &unit, const Vec2i &pos, CUnit *dest, EFlushMode flush)
{
	if (IsUnitValidForNetwork(unit) == false) {
		return ;
	}
	std::unique_ptr<COrder> *order = nullptr;

	if (unit.Type->Building) {
		ClearNewAction(unit);
		order = &unit.NewOrder;
	} else {
		order = GetNextOrder(unit, flush);
		if (order == nullptr) {
			return;
		}
	}
	if (dest) {
		*order = COrder::NewActionRepair(unit, *dest);
	} else {
		*order = COrder::NewActionRepair(pos);
	}
	ClearSavedAction(unit);
}

/**
**  Auto repair.
**
**  @param unit     pointer to unit.
**  @param on       1 for auto repair on, 0 for off.
*/
void CommandAutoRepair(CUnit &unit, int on)
{
	if (IsUnitValidForNetwork(unit) == false) {
		return ;
	}
	unit.AutoRepair = on;
}

/**
**  Attack with unit at new position
**
**  @param unit    pointer to unit.
**  @param pos     map position to attack.
**  @param target  or unit to be attacked.
**  @param flush   If On, flush command queue.
*/
void CommandAttack(CUnit &unit, const Vec2i &pos, CUnit *target, EFlushMode flush)
{
	Assert(Map.Info.IsPointOnMap(pos));
	if (IsUnitValidForNetwork(unit) == false) {
		return ;
	}

	std::unique_ptr<COrder> *order = nullptr;

	if (!unit.Type->CanAttack) {
		ClearNewAction(unit);
		order = &unit.NewOrder;
	} else {
		order = GetNextOrder(unit, flush);
		if (order == nullptr) {
			return;
		}
	}
	if (target && target->IsAlive()) {
		*order = COrder::NewActionAttack(unit, *target);
	} else {
		*order = COrder::NewActionAttack(unit, pos);
	}
	ClearSavedAction(unit);
}

/**
**  Attack ground with unit.
**
**  @param unit   pointer to unit.
**  @param pos    map position to fire on.
**  @param flush  If On, flush command queue.
*/
void CommandAttackGround(CUnit &unit, const Vec2i &pos, EFlushMode flush)
{
	Assert(Map.Info.IsPointOnMap(pos));

	if (IsUnitValidForNetwork(unit) == false) {
		return ;
	}
	std::unique_ptr<COrder> *order = nullptr;

	if (!unit.Type->CanAttack) {
		ClearNewAction(unit);
		order = &unit.NewOrder;
	} else {
		order = GetNextOrder(unit, flush);
		if (order == nullptr) {
			return;
		}
	}
	*order = COrder::NewActionAttackGround(unit, pos);
	ClearSavedAction(unit);
}

/**
**  Let a unit patrol from current to new position
**
**  FIXME: want to support patrolling between units.
**
**  @param unit   pointer to unit.
**  @param pos    map position to patrol between.
**  @param flush  If On, flush command queue.
*/
void CommandPatrolUnit(CUnit &unit, const Vec2i &pos, EFlushMode flush)
{
	Assert(Map.Info.IsPointOnMap(pos));

	if (IsUnitValidForNetwork(unit) == false) {
		return ;
	}

	const Vec2i invalidPos(-1, -1);

	Vec2i startPos = unit.tilePos;
	auto *prevOrder = &unit.Orders.back();

	if(*prevOrder != nullptr) {
		Vec2i prevGoalPos = (*prevOrder)->GetGoalPos();
		if(prevGoalPos != invalidPos) {
			startPos = prevGoalPos;
		}
	}

	std::unique_ptr<COrder> *order = nullptr;

	if (!unit.CanMove()) {
		ClearNewAction(unit);
		order = &unit.NewOrder;
	} else {
		order = GetNextOrder(unit, flush);
		if (order == nullptr) {
			return;
		}
	}
	*order = COrder::NewActionPatrol(startPos, pos);

	ClearSavedAction(unit);
}

/**
**  Board a transporter with unit.
**
**  @param unit   pointer to unit.
**  @param dest   unit to be boarded.
**  @param flush  If On, flush command queue.
*/
void CommandBoard(CUnit &unit, CUnit &dest, EFlushMode flush)
{
	if (IsUnitValidForNetwork(unit) == false) {
		return ;
	}
	if (dest.Destroyed) {
		return ;
	}
	std::unique_ptr<COrder> *order = nullptr;

	if (unit.Type->Building) {
		ClearNewAction(unit);
		order = &unit.NewOrder;
	} else {
		order = GetNextOrder(unit, flush);
		if (order == nullptr) {
			return;
		}
	}
	*order = COrder::NewActionBoard(dest);
	ClearSavedAction(unit);
}

/**
**  Unload a transporter.
**
**  @param unit   pointer to unit.
**  @param pos    map position to unload.
**  @param what   unit to be unloaded, nullptr for all.
**  @param flush  If On, flush command queue.
*/
void CommandUnload(CUnit &unit, const Vec2i &pos, CUnit *what, EFlushMode flush)
{
	if (IsUnitValidForNetwork(unit) == false) {
		return ;
	}
	auto *order = GetNextOrder(unit, flush);

	if (order == nullptr) {
		return;
	}
	*order = COrder::NewActionUnload(pos, what);
	ClearSavedAction(unit);
}

/**
**  Send a unit building
**
**  @param unit   pointer to unit.
**  @param pos    map position to build.
**  @param what   Unit type to build.
**  @param flush  If On, flush command queue.
*/
void CommandBuildBuilding(CUnit &unit, const Vec2i &pos, CUnitType &what, EFlushMode flush)
{
	if (IsUnitValidForNetwork(unit) == false) {
		return ;
	}
	std::unique_ptr<COrder> *order = nullptr;

	if (unit.Type->Building && !what.BoolFlag[BUILDEROUTSIDE_INDEX].value && unit.MapDistanceTo(pos) > unit.Type->RepairRange) {
		ClearNewAction(unit);
		order = &unit.NewOrder;
	} else {
		order = GetNextOrder(unit, flush);
		if (order == nullptr) {
			return;
		}
	}
	*order = COrder::NewActionBuild(unit, pos, what);
	ClearSavedAction(unit);
}

/**
**  Send a unit exploring
**
**  @param unit   pointer to unit.
**  @param flush  If On, flush command queue.
*/
void CommandExplore(CUnit &unit, EFlushMode flush)
{
	if (IsUnitValidForNetwork(unit) == false) {
		return ;
	}
	std::unique_ptr<COrder> *order = nullptr;

	if (!unit.CanMove()) {
		ClearNewAction(unit);
		order = &unit.NewOrder;
	} else {
		order = GetNextOrder(unit, flush);
		if (order == nullptr) {
			return;
		}
	}
	*order = COrder::NewActionExplore(unit);

	ClearSavedAction(unit);
}

/**
**  Cancel the building construction, or kill a unit.
**
**  @param unit  pointer to unit.
*/
void CommandDismiss(CUnit &unit)
{
	// Check if building is still under construction? (NETWORK!)
	if (unit.CurrentAction() == UnitAction::Built) {
		unit.CurrentOrder()->Cancel(unit);
	} else {
		DebugPrint("Suicide unit ... \n");
		LetUnitDie(unit, true);
	}
	ClearSavedAction(unit);
}

/**
**  Send unit harvest a location
**
**  @param unit   pointer to unit.
**  @param pos    map position for harvest.
**  @param flush  If On, flush command queue.
*/
void CommandResourceLoc(CUnit &unit, const Vec2i &pos, EFlushMode flush)
{
	if (IsUnitValidForNetwork(unit) == false) {
		return ;
	}
	if (!unit.Type->Building && !unit.Type->BoolFlag[HARVESTER_INDEX].value) {
		ClearSavedAction(unit);
		return ;
	}
	std::unique_ptr<COrder> *order = nullptr;

	if (unit.Type->Building) {
		ClearNewAction(unit);
		order = &unit.NewOrder;
	} else {
		order = GetNextOrder(unit, flush);
		if (order == nullptr) {
			return;
		}
	}
	*order = COrder::NewActionResource(unit, pos);
	ClearSavedAction(unit);
}

/**
**  Send unit to harvest resources
**
**  @param unit   pointer to unit.
**  @param dest   destination unit.
**  @param flush  If On, flush command queue.
*/
void CommandResource(CUnit &unit, CUnit &dest, EFlushMode flush)
{
	if (IsUnitValidForNetwork(unit) == false) {
		return ;
	}
	if (dest.Destroyed) {
		return ;
	}
	if (!unit.Type->Building && !unit.Type->BoolFlag[HARVESTER_INDEX].value) {
		ClearSavedAction(unit);
		return ;
	}
	std::unique_ptr<COrder> *order = nullptr;

	if (unit.Type->Building) {
		ClearNewAction(unit);
		order = &unit.NewOrder;
	} else {
		order = GetNextOrder(unit, flush);
		if (order == nullptr) {
			return;
		}
	}
	*order = COrder::NewActionResource(unit, dest);
	ClearSavedAction(unit);
}

/**
**  Let unit returning goods.
**
**  @param unit   pointer to unit.
**  @param depot  bring goods to this depot.
**  @param flush  If On, flush command queue.
*/
void CommandReturnGoods(CUnit &unit, CUnit *depot, EFlushMode flush)
{
	if (IsUnitValidForNetwork(unit) == false) {
		return ;
	}
	if ((unit.Type->BoolFlag[HARVESTER_INDEX].value && unit.ResourcesHeld == 0)
		|| (!unit.Type->Building && !unit.Type->BoolFlag[HARVESTER_INDEX].value)) {
		ClearSavedAction(unit);
		return ;
	}
	std::unique_ptr<COrder> *order = nullptr;

	if (unit.Type->Building) {
		ClearNewAction(unit);
		order = &unit.NewOrder;
	} else {
		order = GetNextOrder(unit, flush);
		if (order == nullptr) {
			return;
		}
	}
	*order = COrder::NewActionReturnGoods(unit, depot);
	ClearSavedAction(unit);
}

/**
**  Building starts training an unit.
**
**  @param unit   pointer to unit.
**  @param type   unit type to train.
**  @param flush  If On, flush command queue.
*/
void CommandTrainUnit(CUnit &unit, CUnitType &type, EFlushMode)
{
	if (IsUnitValidForNetwork(unit) == false) {
		return ;
	}
	// Check if enough resources remains? (NETWORK!)
	// FIXME: wrong if append to message queue!!!
	if (unit.Player->CheckLimits(type) != ECheckLimit::Ok
		|| unit.Player->CheckUnitType(type)) {
		return;
	}
	// Not already training?
	if (!EnableTrainingQueue && unit.CurrentAction() == UnitAction::Train) {
		DebugPrint("Unit queue disabled!\n");
		return;
	}

	auto *order = GetNextOrder(unit, EFlushMode::Off);

	if (order == nullptr) {
		return;
	}
	*order = COrder::NewActionTrain(unit, type);
	ClearSavedAction(unit);
}

/**
**  Cancel the training of an unit.
**
**  @param unit  pointer to unit.
**  @param slot  slot number to cancel.
**  @param type  Unit-type to cancel.
*/
void CommandCancelTraining(CUnit &unit, int slot, const CUnitType *type)
{
	DebugPrint("Cancel %d type: %s\n", slot, type ? type->Ident.c_str() : "-any-");

	ClearSavedAction(unit);

	// Check if unit is still training 'slot'? (NETWORK!)
	if (slot == -1) {
		// Cancel All training
		while (unit.CurrentAction() == UnitAction::Train) {
			unit.CurrentOrder()->Cancel(unit);
			RemoveOrder(unit, 0);
		}
		if (unit.Player == ThisPlayer && unit.Selected) {
			SelectedUnitChanged();
		}
	} else if (unit.Orders.size() <= static_cast<size_t>(slot)) {
		// Order has moved
		return;
	} else if (unit.Orders[slot]->Action != UnitAction::Train) {
		// Order has moved, we are not training
		return;
	} else if (unit.Orders[slot]->Action == UnitAction::Train) {
		COrder_Train &order = *static_cast<COrder_Train *>(unit.Orders[slot].get());
		// Still training this order, same unit?
		if (type && &order.GetUnitType() != type) {
			// Different unit being trained
			return;
		}
		order.Cancel(unit);
		RemoveOrder(unit, slot);

		// Update interface.
		if (unit.Player == ThisPlayer && unit.Selected) {
			SelectedUnitChanged();
		}
	}
}

/**
**  Building starts upgrading to.
**
**  @param unit   pointer to unit.
**  @param type   upgrade to type
**  @param flush  If On, flush command queue.
*/
void CommandUpgradeTo(CUnit &unit, CUnitType &type, EFlushMode flush, bool instant)
{
	if (IsUnitValidForNetwork(unit) == false) {
		return ;
	}

	// Check if enough resources remains? (NETWORK!)
	if (unit.Player->CheckUnitType(type)) {
		return;
	}

	auto *order = GetNextOrder(unit, flush);

	if (order == nullptr) {
		return;
	}
	*order = COrder::NewActionUpgradeTo(unit, type, instant);
	ClearSavedAction(unit);
}

/**
**  Immediate transforming unit into type.
**
**  @param unit   pointer to unit.
**  @param type   upgrade to type
*/
void CommandTransformIntoType(CUnit &unit, CUnitType &type)
{
	if (unit.CriticalOrder && unit.CriticalOrder->Action == UnitAction::TransformInto) {
		return;
	}
	Assert(unit.CriticalOrder == nullptr);

	unit.CriticalOrder = COrder::NewActionTransformInto(type);
}

/**
**  Cancel building upgrading to.
**
**  @param unit  pointer to unit.
*/
void CommandCancelUpgradeTo(CUnit &unit)
{
	// Check if unit is still upgrading? (NETWORK!)
	if (unit.CurrentAction() == UnitAction::UpgradeTo) {
		unit.CurrentOrder()->Cancel(unit);
		RemoveOrder(unit, 0);
		if (!Selected.empty()) {
			SelectedUnitChanged();
		}
	}
	ClearSavedAction(unit);
}

/**
**  Building starts researching.
**
**  @param unit   pointer to unit.
**  @param what   what to research.
**  @param flush  If On, flush command queue.
*/
void CommandResearch(CUnit &unit, CUpgrade &what, EFlushMode flush)
{
	if (IsUnitValidForNetwork(unit) == false) {
		return ;
	}
	// Check if enough resources remains? (NETWORK!)
	if (unit.Player->CheckCosts(what.Costs)) {
		return;
	}
	auto *order = GetNextOrder(unit, flush);
	if (order == nullptr) {
		return;
	}
	*order = COrder::NewActionResearch(unit, what);
	ClearSavedAction(unit);
}

/**
**  Cancel Building researching.
**
**  @param unit  Pointer to unit.
*/
void CommandCancelResearch(CUnit &unit)
{
	// Check if unit is still researching? (NETWORK!)
	if (unit.CurrentAction() == UnitAction::Research) {
		unit.CurrentOrder()->Cancel(unit);
		RemoveOrder(unit, 0);
		if (!Selected.empty()) {
			SelectedUnitChanged();
		}
	}
	ClearSavedAction(unit);
}

/**
**  Cast a spell at position or unit.
**
**  @param unit   Pointer to unit.
**  @param pos    map position to spell cast on.
**  @param dest   Spell cast on unit (if exist).
**  @param spell  Spell type pointer.
**  @param flush  If On, flush command queue.
*/
void CommandSpellCast(CUnit &unit, const Vec2i &pos, CUnit *dest, const SpellType &spell, EFlushMode flush, bool isAutocast)
{
	DebugPrint(": %d casts %s at %d %d on %d\n",
	           UnitNumber(unit),
	           spell.Ident.c_str(),
	           pos.x,
	           pos.y,
	           dest ? UnitNumber(*dest) : 0);
	Assert(unit.Type->CanCastSpell[spell.Slot]);
	Assert(Map.Info.IsPointOnMap(pos));

	if (IsUnitValidForNetwork(unit) == false) {
		return ;
	}
	auto *order = GetNextOrder(unit, flush);

	if (order == nullptr) {
		return;
	}

	*order = COrder::NewActionSpellCast(spell, pos, dest, true);
	ClearSavedAction(unit);
}

/**
**  Auto spell cast.
**
**  @param unit     pointer to unit.
**  @param spellid  Spell id.
**  @param on       1 for auto cast on, 0 for off.
*/
void CommandAutoSpellCast(CUnit &unit, int spellid, int on)
{
	if (IsUnitValidForNetwork(unit) == false) {
		return ;
	}
	unit.AutoCastSpell[spellid] = on;
}

/**
**  Diplomacy changed.
**
**  @param player    Player which changes his state.
**  @param state     New diplomacy state.
**  @param opponent  Opponent.
*/
void CommandDiplomacy(int player, EDiplomacy state, int opponent)
{
	switch (state) {
		case EDiplomacy::Neutral:
			Players[player].SetDiplomacyNeutralWith(Players[opponent]);
			break;
		case EDiplomacy::Allied:
			Players[player].SetDiplomacyAlliedWith(Players[opponent]);
			break;
		case EDiplomacy::Enemy:
			Players[player].SetDiplomacyEnemyWith(Players[opponent]);
			break;
		case EDiplomacy::Crazy:
			Players[player].SetDiplomacyCrazyWith(Players[opponent]);
			break;
	}
}

/**
**  Shared vision changed.
**
**  @param playerIndex   Player which changes his state.
**  @param state         New shared vision state.
**  @param opponentIndex Opponent.
*/
void CommandSharedVision(int playerIndex, bool state, int opponentIndex)
{
	CPlayer *player = &Players[playerIndex];
	CPlayer *opponent = &Players[opponentIndex];

	if (state == player->HasSharedVisionWith(*opponent)
	    || opponent->Type == PlayerTypes::PlayerNobody
	    || player->Type == PlayerTypes::PlayerNobody) {
		return;
	}

	// Do a real hardcore seen recount. First we unmark sight for all units of the player.
	for (CUnit *const unit : player->GetUnits()) {
		if (!unit->Destroyed) {
			MapUnmarkUnitSight(*unit);
		}
	}

	if (state == false) {
		player->UnshareVisionWith(*opponent);

		// Don't share vision anymore. Give explored terrain for good-bye.
		const size_t fieldsNum = Map.Info.MapWidth * Map.Info.MapHeight;
		for (size_t i = 0; i != fieldsNum; ++i) {
			CMapField &mf = *Map.Field(i);
			CMapFieldPlayerInfo &mfp = mf.playerInfo;

			if (mfp.Visible[playerIndex] && !mfp.Visible[opponentIndex]) {
				mfp.Visible[opponentIndex] = 1;
				/// TODO: change ThisPlayer to currently rendered player/players #RenderTargets
				if (opponent == ThisPlayer) {
					Map.MarkSeenTile(mf);
				}
			}
		}
	} else {
		player->ShareVisionWith(*opponent);
	}

	// Do a real hardcore seen recount. Now we remark sight for all units of the player
	for (CUnit *const unit : player->GetUnits()) {
		if (!unit->Destroyed) {
			MapMarkUnitSight(*unit);
		}
	}
}

/**
**  Player quit.
**
**  @param player  Player number that quit.
*/
void CommandQuit(int player)
{
	// Set player to neutral, remove allied/enemy/shared vision status
	// If the player doesn't have any units then this is pointless?
	Players[player].Type = PlayerTypes::PlayerNeutral;
	for (int i = 0; i < NumPlayers; ++i) {
		if (i != player && Players[i].Team != Players[player].Team) {
			Players[i].SetDiplomacyNeutralWith(Players[player]);
			Players[player].SetDiplomacyNeutralWith(Players[i]);
			//  We clear Shared vision by sending fake shared vision commands.
			//  We do this because Shared vision is a bit complex.
			CommandSharedVision(i, 0, player);
			CommandSharedVision(player, 0, i);
			// Remove Selection from Quit Player
			std::vector<CUnit *> empty;
			ChangeTeamSelectedUnits(Players[player], empty);
		}
	}

	if (Players[player].GetUnits().empty()) {
		SetMessage(_("Player \"%s\" has been killed"), Players[player].Name.c_str());
	} else {
		SetMessage(_("Player \"%s\" has left the game"), Players[player].Name.c_str());
	}
}

//@}
