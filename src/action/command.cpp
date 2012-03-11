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
//      (c) Copyright 1998-2005 by Lutz Sammer and Jimmy Salmon
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "stratagus.h"

#include "actions.h"
#include "action/action_train.h"
#include "map.h"
#include "pathfinder.h"
#include "player.h"
#include "spells.h"
#include "tileset.h"
#include "upgrade.h"
#include "ui.h"
#include "unit.h"
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
	for (size_t i = 1; i != unit.Orders.size(); ++i) {
		delete unit.Orders[i];
	}
	unit.Orders.resize(1);
	unit.Orders[0]->Finished = true;
}

/**
**  Get next free order slot.
**
**  @param unit   pointer to unit.
**  @param flush  if true, flush order queue.
**
**  @return       Pointer to next free order slot.
*/
static COrderPtr *GetNextOrder(CUnit &unit, int flush)
{
	if (flush) {
		// empty command queue
		ReleaseOrders(unit);
	}
	// FIXME : Remove Hardcoded value.
	const unsigned int maxOrderCount = 0x7F;

	if (unit.Orders.size() == maxOrderCount) {
		return NULL;
	}
	unit.Orders.push_back(NULL);
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

	delete unit.Orders[order];
	unit.Orders.erase(unit.Orders.begin() + order);
	if (unit.Orders.empty()) {
		unit.Orders.push_back(COrder::NewActionStill());
	}
}

static void ClearNewAction(CUnit &unit)
{
	delete unit.NewOrder;
	unit.NewOrder = NULL;
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
	delete unit.SavedOrder;
	unit.SavedOrder = NULL;
}

static bool IsUnitValidForNetwork(const CUnit &unit)
{
	return !unit.Removed && unit.CurrentAction() != UnitActionDie;
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
	COrderPtr *order = GetNextOrder(unit, FlushCommands); // Flush them.
	Assert(order);
	Assert(*order == NULL);
	*order = COrder::NewActionStill();

	ClearSavedAction(unit);
	ClearNewAction(unit);
}

/**
**  Stand ground.
**
**  @param unit   pointer to unit.
**  @param flush  if true, flush command queue.
*/
void CommandStandGround(CUnit &unit, int flush)
{
	COrderPtr* order;

	if (unit.Type->Building) {
		ClearNewAction(unit);
		order = &unit.NewOrder;
	} else {
		order = GetNextOrder(unit, flush);
		if (order == NULL) {
			return;
		}
	}
	*order = COrder::NewActionStandGround();
	ClearSavedAction(unit);
}

/**
**  Follow unit to new position
**
**  @param unit   pointer to unit.
**  @param dest   unit to be followed
**  @param flush  if true, flush command queue.
*/
void CommandFollow(CUnit &unit, CUnit &dest, int flush)
{
	if (IsUnitValidForNetwork(unit) == false) {
		return ;
	}
	COrderPtr *order;

	if (!unit.CanMove()) {
		ClearNewAction(unit);
		order = &unit.NewOrder;
	} else {
		order = GetNextOrder(unit, flush);
		if (order == NULL) {
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
**  @param flush  if true, flush command queue.
*/
void CommandMove(CUnit &unit, const Vec2i &pos, int flush)
{
	Assert(Map.Info.IsPointOnMap(pos));

	if (IsUnitValidForNetwork(unit) == false) {
		return ;
	}
	COrderPtr *order;

	if (!unit.CanMove()) {
		ClearNewAction(unit);
		order = &unit.NewOrder;
	} else {
		order = GetNextOrder(unit, flush);
		if (order == NULL) {
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
**  @param flush  if true, flush command queue.
*/
void CommandRepair(CUnit &unit, const Vec2i &pos, CUnit *dest, int flush)
{
	if (IsUnitValidForNetwork(unit) == false) {
		return ;
	}
	COrderPtr *order;

	if (unit.Type->Building) {
		ClearNewAction(unit);
		order = &unit.NewOrder;
	} else {
		order = GetNextOrder(unit, flush);
		if (order == NULL) {
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
**  @param flush   if true, flush command queue.
*/
void CommandAttack(CUnit &unit, const Vec2i &pos, CUnit *target, int flush)
{
	Assert(Map.Info.IsPointOnMap(pos));
	if (IsUnitValidForNetwork(unit) == false) {
		return ;
	}

	COrderPtr *order;

	if (!unit.Type->CanAttack) {
		ClearNewAction(unit);
		order = &unit.NewOrder;
	} else {
		order = GetNextOrder(unit, flush);
		if (order == NULL) {
			return;
		}
	}
	if (target) {
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
**  @param flush  if true, flush command queue.
*/
void CommandAttackGround(CUnit &unit, const Vec2i &pos, int flush)
{
	Assert(Map.Info.IsPointOnMap(pos));

	if (IsUnitValidForNetwork(unit) == false) {
		return ;
	}
	COrderPtr *order;

	if (!unit.Type->CanAttack) {
		ClearNewAction(unit);
		order = &unit.NewOrder;
	} else {
		order = GetNextOrder(unit, flush);
		if (order == NULL) {
			return;
		}
	}
	*order = COrder::NewActionAttackGround(unit, pos);
	ClearSavedAction(unit);
}

/**
**  Let a unit patrol from current to new position
**
**  FIXME: want to support patroling between units.
**
**  @param unit   pointer to unit.
**  @param pos    map position to patrol between.
**  @param flush  if true, flush command queue.
*/
void CommandPatrolUnit(CUnit &unit, const Vec2i &pos, int flush)
{
	Assert(Map.Info.IsPointOnMap(pos));

	if (IsUnitValidForNetwork(unit) == false) {
		return ;
	}
	COrderPtr *order;

	if (!unit.CanMove()) {
		ClearNewAction(unit);
		order = &unit.NewOrder;
	} else {
		order = GetNextOrder(unit, flush);
		if (order == NULL) {
			return;
		}
	}
	*order = COrder::NewActionPatrol(unit.tilePos, pos);

	ClearSavedAction(unit);
}

/**
**  Board a transporter with unit.
**
**  @param unit   pointer to unit.
**  @param dest   unit to be boarded.
**  @param flush  if true, flush command queue.
*/
void CommandBoard(CUnit &unit, CUnit &dest, int flush)
{
	if (IsUnitValidForNetwork(unit) == false) {
		return ;
	}
	if (dest.Destroyed) {
		return ;
	}
	COrderPtr *order;

	if (unit.Type->Building) {
		ClearNewAction(unit);
		order = &unit.NewOrder;
	} else {
		order = GetNextOrder(unit, flush);
		if (order == NULL) {
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
**  @param what   unit to be unloaded, NoUnitP all.
**  @param flush  if true, flush command queue.
*/
void CommandUnload(CUnit &unit, const Vec2i &pos, CUnit *what, int flush)
{
	if (IsUnitValidForNetwork(unit) == false) {
		return ;
	}
	COrderPtr *order = GetNextOrder(unit, flush);

	if (order == NULL) {
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
**  @param flush  if true, flush command queue.
*/
void CommandBuildBuilding(CUnit &unit, const Vec2i &pos, CUnitType &what, int flush)
{
	if (IsUnitValidForNetwork(unit) == false) {
		return ;
	}
	COrderPtr *order;

	if (unit.Type->Building) {
		ClearNewAction(unit);
		order = &unit.NewOrder;
	} else {
		order = GetNextOrder(unit, flush);
		if (order == NULL) {
			return;
		}
	}
	*order = COrder::NewActionBuild(unit, pos, what);
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
	if (unit.CurrentAction() == UnitActionBuilt) {
		unit.CurrentOrder()->Cancel(unit);
	} else {
		DebugPrint("Suicide unit ... \n");
		LetUnitDie(unit);
	}
	ClearSavedAction(unit);
}

/**
**  Send unit harvest a location
**
**  @param unit   pointer to unit.
**  @param pos    map position for harvest.
**  @param flush  if true, flush command queue.
*/
void CommandResourceLoc(CUnit &unit, const Vec2i &pos, int flush)
{
	if (IsUnitValidForNetwork(unit) == false) {
		return ;
	}
	if (!unit.Type->Building && !unit.Type->Harvester) {
		ClearSavedAction(unit);
		return ;
	}
	COrderPtr *order;

	if (unit.Type->Building) {
		ClearNewAction(unit);
		order = &unit.NewOrder;
	} else {
		order = GetNextOrder(unit, flush);
		if (order == NULL) {
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
**  @param flush  if true, flush command queue.
*/
void CommandResource(CUnit &unit, CUnit &dest, int flush)
{
	if (IsUnitValidForNetwork(unit) == false) {
		return ;
	}
	if (dest.Destroyed) {
		return ;
	}
	if (!unit.Type->Building && !unit.Type->Harvester) {
		ClearSavedAction(unit);
		return ;
	}
	COrderPtr *order;

	if (unit.Type->Building) {
		ClearNewAction(unit);
		order = &unit.NewOrder;
	} else {
		order = GetNextOrder(unit, flush);
		if (order == NULL) {
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
**  @param goal   bring goods to this depot.
**  @param flush  if true, flush command queue.
*/
void CommandReturnGoods(CUnit &unit, CUnit *goal, int flush)
{
	if (IsUnitValidForNetwork(unit) == false) {
		return ;
	}
	if ((unit.Type->Harvester && unit.ResourcesHeld == 0)
		|| (!unit.Type->Building && !unit.Type->Harvester)) {
		ClearSavedAction(unit);
		return ;
	}
	COrderPtr *order;

	if (unit.Type->Building) {
		ClearNewAction(unit);
		order = &unit.NewOrder;
	} else {
		order = GetNextOrder(unit, flush);
		if (order == NULL) {
			return;
		}
	}
	*order = COrder::NewActionReturnGoods(unit, goal);
	ClearSavedAction(unit);
}

/**
**  Building starts training an unit.
**
**  @param unit   pointer to unit.
**  @param type   unit type to train.
**  @param flush  if true, flush command queue.
*/
void CommandTrainUnit(CUnit &unit, CUnitType &type, int)
{
	if (IsUnitValidForNetwork(unit) == false) {
		return ;
	}
	// Check if enough resources remains? (NETWORK!)
	// FIXME: wrong if append to message queue!!!
	if (unit.Player->CheckLimits(type) < 0
		|| unit.Player->CheckUnitType(type)) {
		return;
	}
	// Not already training?
	if (!EnableTrainingQueue && unit.CurrentAction() == UnitActionTrain) {
		DebugPrint("Unit queue disabled!\n");
		return;
	}

	const int noFlushCommands = 0;
	COrderPtr *order = GetNextOrder(unit, noFlushCommands);

	if (order == NULL) {
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
	DebugPrint("Cancel %d type: %s\n" _C_ slot _C_
		type ? type->Ident.c_str() : "-any-");

	ClearSavedAction(unit);

	// Check if unit is still training 'slot'? (NETWORK!)
	if (slot == -1) {
		// Cancel All training
		while (unit.CurrentAction() == UnitActionTrain) {
			unit.CurrentOrder()->Cancel(unit);
			RemoveOrder(unit, 0);
		}
		if (unit.Player == ThisPlayer && unit.Selected) {
			SelectedUnitChanged();
		}
	} else if (unit.Orders.size() <= static_cast<size_t>(slot)) {
		// Order has moved
		return;
	} else if (unit.Orders[slot]->Action != UnitActionTrain) {
		// Order has moved, we are not training
		return;
	} else if (unit.Orders[slot]->Action == UnitActionTrain) {
		COrder_Train& order = *static_cast<COrder_Train*>(unit.Orders[slot]);
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
**  @param flush  if true, flush command queue.
*/
void CommandUpgradeTo(CUnit &unit, CUnitType &type, int flush)
{
	if (IsUnitValidForNetwork(unit) == false) {
		return ;
	}

	// Check if enough resources remains? (NETWORK!)
	if (unit.Player->CheckUnitType(type)) {
		return;
	}

	COrderPtr *order = GetNextOrder(unit, flush);

	if (order == NULL) {
		return;
	}
	*order = COrder::NewActionUpgradeTo(unit, type);
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
	Assert(unit.CriticalOrder == NULL);

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
	if (unit.CurrentAction() == UnitActionUpgradeTo) {
		unit.CurrentOrder()->Cancel(unit);
		RemoveOrder(unit, 0);
		if (Selected) {
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
**  @param flush  if true, flush command queue.
*/
void CommandResearch(CUnit &unit, CUpgrade *what, int flush)
{
	if (IsUnitValidForNetwork(unit) == false) {
		return ;
	}
	// Check if enough resources remains? (NETWORK!)
	if (unit.Player->CheckCosts(what->Costs)) {
		return;
	}
	COrderPtr *order = GetNextOrder(unit, flush);
	if (order == NULL) {
		return;
	}
	*order = COrder::NewActionResearch(unit, *what);
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
	if (unit.CurrentAction() == UnitActionResearch) {
		unit.CurrentOrder()->Cancel(unit);
		RemoveOrder(unit, 0);
		if (Selected) {
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
**  @param flush  If true, flush command queue.
*/
void CommandSpellCast(CUnit &unit, const Vec2i &pos, CUnit *dest, SpellType *spell, int flush)
{
	DebugPrint(": %d casts %s at %d %d on %d\n" _C_
		UnitNumber(unit) _C_ spell->Ident.c_str() _C_ pos.x _C_ pos.y _C_ dest ? UnitNumber(*dest) : 0);
	Assert(unit.Type->CanCastSpell[spell->Slot]);
	Assert(Map.Info.IsPointOnMap(pos));

	if (IsUnitValidForNetwork(unit) == false) {
		return ;
	}
	COrderPtr *order = GetNextOrder(unit, flush);

	if (order == NULL) {
		return;
	}

	*order = COrder::NewActionSpellCast(*spell, pos, dest);
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
void CommandDiplomacy(int player, int state, int opponent)
{
	switch (state) {
		case DiplomacyNeutral:
			Players[player].SetDiplomacyNeutralWith(Players[opponent]);
			break;
		case DiplomacyAllied:
			Players[player].SetDiplomacyAlliedWith(Players[opponent]);
			break;
		case DiplomacyEnemy:
			Players[player].SetDiplomacyEnemyWith(Players[opponent]);
			break;
		case DiplomacyCrazy:
			Players[player].SetDiplomacyCrazyWith(Players[opponent]);
			break;
	}
}

/**
**  Shared vision changed.
**
**  @param player    Player which changes his state.
**  @param state     New shared vision state.
**  @param opponent  Opponent.
*/
void CommandSharedVision(int player, bool state, int opponent)
{
	// Do a real hardcore seen recount. First we unmark EVERYTHING.
	for (int i = 0; i < NumUnits; ++i) {
		if (!Units[i]->Destroyed) {
			MapUnmarkUnitSight(*Units[i]);
		}
	}

	// Compute Before and after.
	const int before = Players[player].IsBothSharedVision(Players[opponent]);
	if (state == false) {
		Players[player].UnshareVisionWith(Players[opponent]);
	} else {
		Players[player].ShareVisionWith(Players[opponent]);
	}
	const int after = Players[player].IsBothSharedVision(Players[opponent]);

	if (before && !after) {
		// Don't share vision anymore. Give each other explored terrain for good-bye.
		Vec2i pos;
		for (pos.x = 0; pos.x < Map.Info.MapWidth; ++pos.x) {
			for (pos.y = 0; pos.y < Map.Info.MapHeight; ++pos.y) {
				CMapField &mf = *Map.Field(pos);

				if (mf.Visible[player] && !mf.Visible[opponent]) {
					mf.Visible[opponent] = 1;
					if (opponent == ThisPlayer->Index) {
						Map.MarkSeenTile(pos);
					}
				}
				if (mf.Visible[opponent] && !mf.Visible[player]) {
					mf.Visible[player] = 1;
					if (player == ThisPlayer->Index) {
						Map.MarkSeenTile(pos);
					}
				}
			}
		}
	}

	// Do a real hardcore seen recount. Now we remark EVERYTHING
	for (int i = 0; i < NumUnits; ++i) {
		if (!Units[i]->Destroyed) {
			MapMarkUnitSight(*Units[i]);
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
	Players[player].Type = PlayerNeutral;
	for (int i = 0; i < NumPlayers; ++i) {
		if (i != player && Players[i].Team != Players[player].Team) {
			Players[i].SetDiplomacyNeutralWith(Players[player]);
			Players[player].SetDiplomacyNeutralWith(Players[i]);
			//  We clear Shared vision by sending fake shared vision commands.
			//  We do this because Shared vision is a bit complex.
			CommandSharedVision(i, 0, player);
			CommandSharedVision(player, 0, i);
			// Remove Selection from Quit Player
			std::vector<CUnit*> empty;
			ChangeTeamSelectedUnits(Players[player], empty, 0);
		}
	}

	if (Players[player].TotalNumUnits != 0) {
		SetMessage(_("Player \"%s\" has left the game"), Players[player].Name.c_str());
	} else {
		SetMessage(_("Player \"%s\" has been killed"), Players[player].Name.c_str());
	}
}

//@}
