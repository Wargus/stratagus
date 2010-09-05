//     ____                _       __               
//    / __ )____  _____   | |     / /___ ___________
//   / __  / __ \/ ___/   | | /| / / __ `/ ___/ ___/
//  / /_/ / /_/ (__  )    | |/ |/ / /_/ / /  (__  ) 
// /_____/\____/____/     |__/|__/\__,_/_/  /____/  
//                                              
//       A futuristic real-time strategy game.
//          This file is part of Bos Wars.
//
/**@name command.cpp - Give units a command. */
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
//

//@{

/*----------------------------------------------------------------------------
--  Includes
----------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "stratagus.h"
#include "unittype.h"
#include "player.h"
#include "unit.h"
#include "actions.h"
#include "map.h"
#include "upgrade.h"
#include "pathfinder.h"
#include "spells.h"
#include "interface.h"
#include "ui.h"

/*----------------------------------------------------------------------------
--  Functions
----------------------------------------------------------------------------*/

/**
**  Release an order.
**
**  @param order  Pointer to order.
*/
static void ReleaseOrder(COrder *order)
{
	if (order->Goal) {
		order->Goal->RefsDecrease();
		order->Goal = NoUnitP;
	}
}

/**
**  Release all orders of a unit.
**
**  @param unit  Pointer to unit.
*/
static void ReleaseOrders(CUnit *unit)
{
	int n;

	if ((n = unit->OrderCount) > 1) {
		while (--n) {
			ReleaseOrder(unit->Orders[n]);
			delete unit->Orders[n];
			unit->Orders.pop_back();
		}
		unit->OrderCount = 1;
	}
	unit->OrderFlush = 1;
	// Order 0 must be stopped in the action loop.
}

/**
**  Get next free order slot.
**
**  @param unit   pointer to unit.
**  @param flush  if true, flush order queue.
**
**  @return       Pointer to next free order slot.
*/
static COrder *GetNextOrder(CUnit *unit, int flush)
{
	if (flush) {
		// empty command queue
		ReleaseOrders(unit);
	}
	if (unit->OrderCount == 0x7F) {
		return NULL;
	}

	unit->Orders.push_back(new COrder);

	return unit->Orders[(int)unit->OrderCount++];
}

/**
**  Remove an order from the list of orders pending
**
**  @param unit   pointer to unit
**  @param order  number of the order to remove
*/
static void RemoveOrder(CUnit *unit, int order)
{
	int i;
	
	Assert(0 <= order && order < unit->OrderCount);
	if (order != 0) {
		delete unit->Orders[order];
	}
	i = order;
	while (i < unit->OrderCount - 1) {
		unit->Orders[i] = unit->Orders[i + 1];
		++i;
	}

	if (unit->OrderCount > 1) {
		unit->Orders.pop_back();
		--unit->OrderCount;
		if (order == 0) {
			unit->SubAction = 0;
		}
	} else {
		Assert(i == 0);
		unit->Orders[0]->Init();
		unit->Orders[0]->Action = UnitActionStill;
		unit->SubAction = 0;
	}
}

/**
**  Clear the saved action.
**
**  @param unit  Unit pointer, that get the saved action cleared.
**
**  @note        If we make a new order, we must clear any saved actions.
**  @note        Internal functions, must protect it, if needed.
*/
static void ClearSavedAction(CUnit *unit)
{
	ReleaseOrder(&unit->SavedOrder);

	unit->SavedOrder.Init();
	unit->SavedOrder.Action = UnitActionStill; // clear saved action
}

/*----------------------------------------------------------------------------
--  Commands
----------------------------------------------------------------------------*/

/**
**  Stop unit.
**
**  @param unit  pointer to unit.
*/
void CommandStopUnit(CUnit *unit)
{
	COrder *order;

	// Ignore that the unit could be removed.

	order = GetNextOrder(unit, FlushCommands); // Flush them.
	Assert(order);
	order->Init();

	order->Action = UnitActionStill;
	ReleaseOrder(&unit->SavedOrder);
	ReleaseOrder(&unit->NewOrder);
	unit->SavedOrder = unit->NewOrder = *order;
}

/**
**  Order an already formed Order structure
**
**  @param unit      pointer to unit
**  @param cpyorder  pointer to valid order
**  @param flush     if true, flush command queue.
*/
void CommandAnyOrder(CUnit *unit, COrder *cpyorder, int flush)
{
	COrder *order;

	if (!(order = GetNextOrder(unit, flush))) {
		return;
	}

	*order = *cpyorder;
	if (order->Goal) {
		order->Goal->RefsIncrease();
	}
	ClearSavedAction(unit);
}

/**
**  Move an order in the order queue.
**  ( Cannot move the order 0 ! )
**
**  @param unit  pointer to unit
**  @param src   the order to move
**  @param dst   the new position of the order
*/
void CommandMoveOrder(CUnit *unit, int src, int dst)
{
	COrder *tmp;
	int i;

	Assert(src != 0 && dst != 0 && src < unit->OrderCount && dst < unit->OrderCount);

	if (src == dst) {
		return;
	}

	if (src < dst) {
		tmp = unit->Orders[src];
		for (i = src; i < dst; ++i) {
			unit->Orders[i] = unit->Orders[i+1];
		}
		unit->Orders[dst] = tmp;
	} else {
		// dst < src
		tmp = unit->Orders[src];
		for (i = src - 1 ; i >= dst; --i) {
			unit->Orders[i + 1] = unit->Orders[i];
		}
		unit->Orders[dst] = tmp;
	}
}

/**
**  Stand ground.
**
**  @param unit   pointer to unit.
**  @param flush  if true, flush command queue.
*/
void CommandStandGround(CUnit *unit, int flush)
{
	COrder *order;

	// Ignore that the unit could be removed.

	if (unit->Type->Building) {
		// FIXME: should find a better way for pending orders.
		order = &unit->NewOrder;
		ReleaseOrder(order);
	} else if (!(order = GetNextOrder(unit, flush))) {
		return;
	}
	order->Init();
	order->Action = UnitActionStandGround;
	ClearSavedAction(unit);
}

/**
**  Follow unit to new position
**
**  @param unit   pointer to unit.
**  @param dest   unit to be followed
**  @param flush  if true, flush command queue.
*/
void CommandFollow(CUnit *unit, CUnit *dest, int flush)
{
	COrder *order;

	//
	// Check if unit is still valid? (NETWORK!)
	//
	if (!unit->Removed && unit->Orders[0]->Action != UnitActionDie) {
		if (!CanMove(unit)) {
			// FIXME: should find a better way for pending orders.
			order = &unit->NewOrder;
			ReleaseOrder(order);
		} else if (!(order = GetNextOrder(unit, flush))) {
			return;
		}
		order->Init();

		order->Action = UnitActionFollow;
		//
		// Destination could be killed.
		// Should be handled in action, but is not possible!
		// Unit::Refs is used as timeout counter.
		//
		if (dest->Destroyed) {
			order->X = dest->X + dest->Type->TileWidth / 2;
			order->Y = dest->Y + dest->Type->TileHeight / 2;
		} else {
			order->Goal = dest;
			dest->RefsIncrease();
			order->Range = 1;
		}
	}
	ClearSavedAction(unit);
}

/**
**  Move unit to new position
**
**  @param unit   pointer to unit.
**  @param x      X map position to move to.
**  @param y      Y map position to move to.
**  @param flush  if true, flush command queue.
*/
void CommandMove(CUnit *unit, int x, int y, int flush)
{
	COrder *order;

	Assert(x >= 0 && y >= 0 && x < Map.Info.MapWidth && y < Map.Info.MapHeight);

	//
	//  Check if unit is still valid? (NETWORK!)
	//
	if (!unit->Removed && unit->Orders[0]->Action != UnitActionDie) {
		if (!CanMove(unit)) {
			// FIXME: should find a better way for pending orders.
			order = &unit->NewOrder;
			ReleaseOrder(order);
		} else if (!(order = GetNextOrder(unit, flush))) {
			return;
		}
		order->Init();

		order->Action = UnitActionMove;
		order->X = x;
		order->Y = y;
	}
	ClearSavedAction(unit);
}

/**
**  Repair unit
**
**  @param unit   pointer to unit.
**  @param x      X map position to repair.
**  @param y      Y map position to repair.
**  @param dest   or unit to be repaired. FIXME: not supported
**  @param flush  if true, flush command queue.
*/
void CommandRepair(CUnit *unit, int x, int y, CUnit *dest, int flush)
{
	COrder *order;

	//
	//  Check if unit is still valid? (NETWORK!)
	//
	if (!unit->Removed && unit->Orders[0]->Action != UnitActionDie) {
		if (unit->Type->Building) {
			// FIXME: should find a better way for pending orders.
			order = &unit->NewOrder;
			ReleaseOrder(order);
		} else if (!(order = GetNextOrder(unit, flush))) {
			return;
		}
		order->Init();

		order->Action = UnitActionRepair;
		//
		//  Destination could be killed.
		//  Should be handled in action, but is not possible!
		//  Unit::Refs is used as timeout counter.
		//
		if (dest) {
			if (dest->Destroyed) {
				order->X = dest->X + dest->Type->TileWidth / 2;
				order->Y = dest->Y + dest->Type->TileHeight / 2;
			} else {
				order->Goal = dest;
				dest->RefsIncrease();
				order->Range = unit->Type->RepairRange;
			}
		} else {
			order->X = x;
			order->Y = y;
		}
	}
	ClearSavedAction(unit);
}

/**
**  Auto repair.
**
**  @param unit     pointer to unit.
**  @param on       1 for auto repair on, 0 for off.
*/
void CommandAutoRepair(CUnit *unit, int on)
{
	//
	// Check if unit is still valid? (NETWORK!)
	//
	if (!unit->Removed && unit->Orders[0]->Action != UnitActionDie) {
		unit->AutoRepair = on;
	}
}

/**
**  Attack with unit at new position
**
**  @param unit    pointer to unit.
**  @param x       X map position to attack.
**  @param y       Y map position to attack.
**  @param attack  or unit to be attacked.
**  @param flush   if true, flush command queue.
*/
void CommandAttack(CUnit *unit, int x, int y, CUnit *attack, int flush)
{
	COrder *order;

	Assert(x >= 0 && y >= 0 && x < Map.Info.MapWidth && y < Map.Info.MapHeight);

	//
	// Check if unit is still valid? (NETWORK!)
	//
	if (!unit->Removed && unit->Orders[0]->Action != UnitActionDie) {
		if (!unit->Type->CanAttack) {
			// FIXME: should find a better way for pending orders.
			order = &unit->NewOrder;
			ReleaseOrder(order);
		} else if (!(order = GetNextOrder(unit, flush))) {
			return;
		}
		order->Init();

		order->Action = UnitActionAttack;
		if (attack) {
			//
			// Destination could be killed.
			// Should be handled in action, but is not possible!
			// Unit::Refs is used as timeout counter.
			//
			if (attack->Destroyed) {
				order->X = attack->X + attack->Type->TileWidth / 2;
				order->Y = attack->Y + attack->Type->TileHeight / 2;
			} else {
				// Removed, Dying handled by action routine.
				order->Goal = attack;
				attack->RefsIncrease();
				order->Range = unit->Stats->Variables[ATTACKRANGE_INDEX].Max;
				order->MinRange = unit->Type->MinAttackRange;
			}
		} else {
			order->X = x;
			order->Y = y;
		}
	}
	ClearSavedAction(unit);
}

/**
**  Attack ground with unit.
**
**  @param unit   pointer to unit.
**  @param x      X map position to fire on.
**  @param y      Y map position to fire on.
**  @param flush  if true, flush command queue.
*/
void CommandAttackGround(CUnit *unit, int x, int y, int flush)
{
	COrder *order;

	Assert(x >= 0 && y >= 0 && x < Map.Info.MapWidth && y < Map.Info.MapHeight);

	//
	// Check if unit is still valid? (NETWORK!)
	//
	if (!unit->Removed && unit->Orders[0]->Action != UnitActionDie) {
		if (unit->Type->Building) {
			// FIXME: should find a better way for pending orders.
			order = &unit->NewOrder;
			ReleaseOrder(order);
		} else if (!(order = GetNextOrder(unit, flush))) {
			return;
		}
		order->Init();

		order->Action = UnitActionAttackGround;
		order->X = x;
		order->Y = y;
		order->Range = unit->Stats->Variables[ATTACKRANGE_INDEX].Max;
		order->MinRange = unit->Type->MinAttackRange;

		DebugPrint("FIXME this next\n");
	}
	ClearSavedAction(unit);
}

/**
**  Let a unit patrol from current to new position
**
**  FIXME: want to support patroling between units.
**
**  @param unit   pointer to unit.
**  @param x      X map position to patrol between.
**  @param y      Y map position to patrol between.
**  @param flush  if true, flush command queue.
*/
void CommandPatrolUnit(CUnit *unit, int x, int y, int flush)
{
	COrder *order;

	Assert(x >= 0 && y >= 0 && x < Map.Info.MapWidth && y < Map.Info.MapHeight);

	//
	// Check if unit is still valid? (NETWORK!)
	//
	if (!unit->Removed && unit->Orders[0]->Action != UnitActionDie) {
		if (!CanMove(unit)) {
			// FIXME: should find a better way for pending orders.
			order = &unit->NewOrder;
			ReleaseOrder(order);
		} else if (!(order = GetNextOrder(unit, flush))) {
			return;
		}
		order->Init();

		order->Action = UnitActionPatrol;
		order->X = x;
		order->Y = y;
		Assert(!(unit->X & ~0xFFFF) && !(unit->Y & ~0xFFFF));
		order->Arg1.Patrol.X = unit->X;
		order->Arg1.Patrol.Y = unit->Y;
	}
	ClearSavedAction(unit);
}

/**
**  Board a transporter with unit.
**
**  @param unit   pointer to unit.
**  @param dest   unit to be boarded.
**  @param flush  if true, flush command queue.
*/
void CommandBoard(CUnit *unit, CUnit *dest, int flush)
{
	COrder *order;

	//
	// Check if unit is still valid? (NETWORK!)
	//
	if (!unit->Removed && unit->Orders[0]->Action != UnitActionDie) {
		//
		// Destination could be killed.
		// Should be handled in action, but is not possible!
		// Unit::Refs is used as timeout counter.
		//
		if (dest->Destroyed) {
			return;
		}

		if (unit->Type->Building) {
			// FIXME: should find a better way for pending orders.
			order = &unit->NewOrder;
			ReleaseOrder(order);
		} else if (!(order = GetNextOrder(unit, flush))) {
			return;
		}
		order->Init();

		order->Action = UnitActionBoard;
		order->Goal = dest;
		dest->RefsIncrease();
		order->Range = 1;
	}
	ClearSavedAction(unit);
}

/**
**  Unload a transporter.
**
**  @param unit   pointer to unit.
**  @param x      X map position to unload.
**  @param y      Y map position to unload.
**  @param what   unit to be unloaded, NoUnitP all.
**  @param flush  if true, flush command queue.
*/
void CommandUnload(CUnit *unit, int x, int y, CUnit *what, int flush)
{
	COrder *order;

	//
	// Check if unit is still valid? (NETWORK!)
	//
	if (!unit->Removed && unit->Orders[0]->Action != UnitActionDie) {
		if (!(order = GetNextOrder(unit, flush))) {
			return;
		}
		order->Init();

		order->Action = UnitActionUnload;
		order->X = x;
		order->Y = y;
		//
		// Destination could be killed.
		// Should be handled in action, but is not possible!
		// Unit::Refs is used as timeout counter.
		//
		if (what && !what->Destroyed) {
			order->Goal = what;
			what->RefsIncrease();
		}
	}
	ClearSavedAction(unit);
}

/**
**  Send a unit building
**
**  @param unit   pointer to unit.
**  @param x      X map position to build.
**  @param y      Y map position to build.
**  @param what   Unit type to build.
**  @param flush  if true, flush command queue.
*/
void CommandBuildBuilding(CUnit *unit, int x, int y,
	CUnitType *what, int flush)
{
	COrder *order;

	//
	// Check if unit is still valid? (NETWORK!)
	//
	if (!unit->Removed && unit->Orders[0]->Action != UnitActionDie) {
		if (unit->Type->Building) {
			// FIXME: should find a better way for pending orders.
			order = &unit->NewOrder;
			ReleaseOrder(order);
		} else if (!(order = GetNextOrder(unit, flush))) {
			return;
		}
		order->Init();

		order->Action = UnitActionBuild;
		order->X = x;
		order->Y = y;
		order->Width = what->TileWidth;
		order->Height = what->TileHeight;
		order->Range = unit->Type->RepairRange;
		order->Type = what;
		order->MinRange = 1;
	}
	ClearSavedAction(unit);
}

/**
**  Cancel the building construction, or kill a unit.
**
**  @param unit  pointer to unit.
*/
void CommandDismiss(CUnit *unit)
{
	//
	// Check if building is still under construction? (NETWORK!)
	//
	if (unit->Orders[0]->Action == UnitActionBuilt) {
		unit->Data.Built.Cancel = 1;
	} else {
		DebugPrint("Suicide unit ... \n");
		LetUnitDie(unit);
	}
	ClearSavedAction(unit);
}

/**
**  Send unit to harvest resources
**
**  @param unit   pointer to unit.
**  @param dest   destination unit.
**  @param flush  if true, flush command queue.
*/
void CommandResource(CUnit *unit, CUnit *dest, int flush)
{
	COrder *order;

	//
	// Check if unit is still valid and Goal still alive? (NETWORK!)
	//
	if (!unit->Removed && unit->Orders[0]->Action != UnitActionDie &&
			!dest->Destroyed) {
		if (!unit->Type->Building && !unit->Type->Harvester) {
			ClearSavedAction(unit);
			return;
		}

		// FIXME: if low-level supports searching, pass NoUnitP down.

		if (unit->Type->Building) {
			// FIXME: should find a better way for pending orders.
			order = &unit->NewOrder;
			ReleaseOrder(order);
		} else if (!(order = GetNextOrder(unit, flush))) {
			return;
		}
		order->Init();
		order->Action = UnitActionResource;
		order->Goal = dest;
		dest->RefsIncrease();
		order->Range = 1;
	}
	ClearSavedAction(unit);
}

/**
**  Check whether the terrain is suitable for training a unit of a
**  given type.  For example, you cannot train battleships at a
**  shipyard if there is no deep water anywhere near it.
**
**  @param trainer      A unit that would train another unit.
**                      Typically, this is a building.
**  @param traineeType  What type of unit it would train.
**
**  @return true if there is at least one suitable map field near
**  the proposed trainer unit, false if not.  Unexplored map fields
**  are never suitable.
**
**  This function only checks that training the unit would look at
**  least somewhat sensible.  It does not check whether there are
**  other units blocking the way, because the situation could change
**  before the training would end.  It also ignores the size of the
**  trained unit.
*/
bool TerrainAllowsTraining(const CUnit *trainer, const CUnitType *traineeType)
{
	// Make this nonzero if you also want to check tiles around
	// the trainer unit.
	const int around = 0;

	const int xMin = std::max(0, trainer->X - around);
	const int yMin = std::max(0, trainer->Y - around);
	const int xEnd = std::min(Map.Info.MapWidth,
		trainer->X + trainer->Type->TileWidth + around);
	const int yEnd = std::min(Map.Info.MapHeight,
		trainer->Y + trainer->Type->TileHeight + around);

	// Ignore any units in the way.  Especially ignore buildings
	// because the trainer unit itself is probably a building.
	const unsigned obstacles = traineeType->MovementMask & MapFieldPatchMask;
	
	for (int y = yMin; y < yEnd; y++) {
		for (int x = xMin; x < xEnd; x++) {
			if (Map.IsFieldExplored(trainer->Player, x, y)
			    && !(Map.Field(x, y)->Flags & obstacles)) {
				return true;
			}
		}
	}

	return false;
}

/**
**  Building starts training a unit.
**
**  @param unit   pointer to unit.
**  @param type   unit type to train.
**  @param flush  if true, flush command queue.
*/
void CommandTrainUnit(CUnit *unit, CUnitType *type, int flush)
{
	COrder *order;

	//
	// Check if unit is still valid? (NETWORK!)
	//
	if (!unit->Removed && unit->Orders[0]->Action != UnitActionDie) {
		//
		// Check if enough resources remains? (NETWORK!)
		// FIXME: wrong if append to message queue!!!
		//
		if (unit->Player->CheckLimits(type) < 0 ) {
			return;
		}

		if (!(order = GetNextOrder(unit, 0))) {
			return;
		}
		order->Init();

		order->Action = UnitActionTrain;
		order->Type = type;
	}
	ClearSavedAction(unit);
}

/**
**  Cancel the training of a unit.
**
**  @param unit  pointer to unit.
**  @param slot  slot number to cancel.
**  @param type  Unit-type to cancel.
*/
void CommandCancelTraining(CUnit *unit, int slot, const CUnitType *type)
{
	DebugPrint("Cancel %d type: %s\n" _C_ slot _C_
		type ? type->Ident.c_str() : "-any-");

	ClearSavedAction(unit);

	//
	// Check if unit is still training 'slot'? (NETWORK!)
	//

	if (slot == -1) {
		if (unit->Orders[0]->Action == UnitActionTrain) {
			unit->Player->RemoveFromUnitsConsumingResources(unit);
		}

		// Cancel All training
		while (unit->Orders[0]->Action == UnitActionTrain) {
			RemoveOrder(unit, 0);
		}
		unit->Data.Train.Ticks = 0;
		if (unit->Player == ThisPlayer && unit->Selected) {
			SelectedUnitChanged();
		}
	} else if (unit->OrderCount <= slot) {
		// Order has moved
		return;
	} else if (unit->Orders[slot]->Action != UnitActionTrain) {
		// Order has moved, we are not training
		return;
	} else if (unit->Orders[slot]->Action == UnitActionTrain) {
		// Still training this order, same unit?
		if (type && unit->Orders[slot]->Type != type) {
			// Different unit being trained
			return;
		}

		DebugPrint("Cancel training\n");

		if (!slot) { // Canceled in work slot
			unit->Data.Train.Ticks = 0;
			unit->Player->RemoveFromUnitsConsumingResources(unit);
		}
		RemoveOrder(unit, slot);

		//
		// Update interface.
		//
		if (unit->Player == ThisPlayer && unit->Selected) {
			SelectedUnitChanged();
		}
	}
}

/**
**  Cast a spell at position or unit.
**
**  @param unit   Pointer to unit.
**  @param x      X map position to spell cast on.
**  @param y      Y map position to spell cast on.
**  @param dest   Spell cast on unit (if exist).
**  @param spell  Spell type pointer.
**  @param flush  If true, flush command queue.
*/
void CommandSpellCast(CUnit *unit, int x, int y, CUnit *dest,
	SpellType *spell, int flush)
{
	COrder *order;

	Assert(x >= 0 && y >= 0 && x < Map.Info.MapWidth && y < Map.Info.MapHeight);

	DebugPrint(": %d casts %s at %d %d on %d\n" _C_
		UnitNumber(unit) _C_ spell->Ident.c_str() _C_ x _C_ y _C_ dest ? UnitNumber(dest) : 0);
	Assert(unit->Type->CanCastSpell[spell->Slot]);

	//
	// Check if unit is still valid? (NETWORK!)
	//
	if (!unit->Removed && unit->Orders[0]->Action != UnitActionDie) {
		// FIXME: should I check here, if there is still enough mana?

		if (!(order = GetNextOrder(unit, flush))) {
			return;
		}
		order->Init();

		order->Action = UnitActionSpellCast;
		order->Range = spell->Range;
		if (dest) {
			//
			// Destination could be killed.
			// Should be handled in action, but is not possible!
			// Unit::Refs is used as timeout counter.
			//
			if (dest->Destroyed) {
				// FIXME: where check if spell needs a unit as destination?
				// FIXME: dest->Type is now set to 0. maybe we shouldn't bother.
				order->X = dest->X /*+ dest->Type->TileWidth / 2*/  - order->Range;
				order->Y = dest->Y /*+ dest->Type->TileHeight / 2*/ - order->Range;
				order->Range <<= 1;
			} else {
				order->Goal = dest;
				dest->RefsIncrease();
			}
		} else {
			order->Range = 1;
			order->X = x;
			order->Y = y;
		}
		order->Arg1.Spell = spell;
	}
	ClearSavedAction(unit);
}

/**
**  Auto spell cast.
**
**  @param unit     pointer to unit.
**  @param spellid  Spell id.
**  @param on       1 for auto cast on, 0 for off.
*/
void CommandAutoSpellCast(CUnit *unit, int spellid, int on)
{
	//
	// Check if unit is still valid? (NETWORK!)
	//
	if (!unit->Removed && unit->Orders[0]->Action != UnitActionDie) {
		unit->AutoCastSpell[spellid] = on;
	}
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
			Players[player].Enemy &= ~(1 << opponent);
			Players[player].Allied &= ~(1 << opponent);
			break;
		case DiplomacyAllied:
			Players[player].Enemy &= ~(1 << opponent);
			Players[player].Allied |= 1 << opponent;
			break;
		case DiplomacyEnemy:
			Players[player].Enemy |= 1 << opponent;
			Players[player].Allied &= ~(1 << opponent);
			break;
		case DiplomacyCrazy:
			Players[player].Enemy |= 1 << opponent;
			Players[player].Allied |= 1 << opponent;
			break;
	}
	// FIXME: Should we display a message?
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
	int before;
	int after;
	int x;
	int y;
	int i;
	CMapField *mf;

	//
	// Do a real hardcore seen recount. First we unmark EVERYTHING.
	//
	for (i = 0; i < NumUnits; ++i) {
		if (!Units[i]->Destroyed) {
			MapUnmarkUnitSight(Units[i]);
		}
	}

	//
	// Compute Before and after.
	//
	before = Players[player].IsBothSharedVision(&Players[opponent]);
	if (state == false) {
		Players[player].SharedVision &= ~(1 << opponent);
	} else {
		Players[player].SharedVision |= (1 << opponent);
	}
	after = Players[player].IsBothSharedVision(&Players[opponent]);

	if (before && !after) {
		//
		// Don't share vision anymore. Give each other explored terrain for good-bye.
		//
		for (x = 0; x < Map.Info.MapWidth; ++x) {
			for (y = 0; y < Map.Info.MapHeight; ++y) {
				mf = Map.Field(x, y);
				if (mf->Visible[player] && !mf->Visible[opponent]) {
					mf->Visible[opponent] = 1;
				}
				if (mf->Visible[opponent] && !mf->Visible[player]) {
					mf->Visible[player] = 1;
				}
			}
		}
	}

	//
	// Do a real hardcore seen recount. Now we remark EVERYTHING
	//
	for (i = 0; i < NumUnits; ++i) {
		if (!Units[i]->Destroyed) {
			MapMarkUnitSight(Units[i]);
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
	int i;

	// Set player to neutral, remove allied/enemy/shared vision status
	// If the player doesn't have any units then this is pointless?
	Players[player].Type = PlayerNeutral;
	for (i = 0; i < NumPlayers; ++i) {
		if (i != player && Players[i].Team != Players[player].Team) {
			Players[i].Allied &= ~(1 << player);
			Players[i].Enemy &= ~(1 << player);
			Players[player].Enemy &= ~(1 << i);
			Players[player].Allied &= ~(1 << i);
			//  We clear Shared vision by sending fake shared vision commands.
			//  We do this because Shared vision is a bit complex.
			CommandSharedVision(i, 0, player);
			CommandSharedVision(player, 0, i);
			// Remove Selection from Quit Player
			ChangeTeamSelectedUnits(&Players[player], NULL, 0, 0);
		}
	}

	if (Players[player].TotalNumUnits != 0) {
		SetMessage(_("Player \"%s\" has left the game"), Players[player].Name.c_str());
	} else {
		SetMessage(_("Player \"%s\" has been killed"), Players[player].Name.c_str());
	}
}

//@}
