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
/**@name command.c - Give units a command. */
//
//      (c) Copyright 1998-2004 by Lutz Sammer
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
//      $Id$

//@{

/*----------------------------------------------------------------------------
--  Includes
----------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>

#include "stratagus.h"
#include "unittype.h"
#include "player.h"
#include "unit.h"
#include "actions.h"
#include "tileset.h"
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
local void ReleaseOrder(Order* order)
{
	if (order->Goal) {
		RefsDecrease(order->Goal);
		order->Goal = NoUnitP;
	}
}

/**
**  Release all orders of an unit.
**
**  @param unit  Pointer to unit.
*/
local void ReleaseOrders(Unit* unit)
{
	int n;

	if ((n = unit->OrderCount) > 1) {
		while (--n) {
			ReleaseOrder(&unit->Orders[n]);
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
local Order* GetNextOrder(Unit* unit, int flush)
{
	if (flush) {			// empty command queue
		ReleaseOrders(unit);
	} else if (unit->OrderCount == MAX_ORDERS) {
		// FIXME: johns: wrong place for an error message.
		// FIXME: johns: should be checked by AI or the user interface
		// NOTE: But must still be checked here.
		NotifyPlayer(unit->Player, NotifyYellow, unit->X, unit->Y,
			"Unit order list is full");
		return NULL;
	}

	return &unit->Orders[(int)unit->OrderCount++];
}

/**
**  Clear the saved action.
**
**  @param unit  Unit pointer, that get the saved action cleared.
**
**  @note        If we make an new order, we must clear any saved actions.
**  @note        Internal functions, must protect it, if needed.
*/
local void ClearSavedAction(Unit* unit)
{
	ReleaseOrder(&unit->SavedOrder);

	unit->SavedOrder.Action = UnitActionStill;		// clear saved action
	unit->SavedOrder.X = unit->SavedOrder.Y = -1;
	unit->SavedOrder.Type = NULL;
	unit->SavedOrder.Arg1 = NULL;
}

/*----------------------------------------------------------------------------
--  Commands
----------------------------------------------------------------------------*/

/**
**  Stop unit.
**
**  @param unit  pointer to unit.
*/
global void CommandStopUnit(Unit* unit)
{
	Order* order;

	// Ignore that the unit could be removed.

	order = GetNextOrder(unit, FlushCommands);		// Flush them.
	order->Action = UnitActionStill;
	order->X = order->Y = -1;
	order->Goal = NoUnitP;
	order->Type = NULL;
	order->Arg1 = NULL;
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
global void CommandAnyOrder(Unit* unit, Order* cpyorder, int flush)
{
	Order* order;
	if (!(order = GetNextOrder(unit, flush))) {
		return;
	}

	*order = *cpyorder;
	if (order->Goal) {
		RefsIncrease(order->Goal);
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
global void CommandMoveOrder(Unit* unit, int src, int dst)
{
	Order tmp;
	int i;
	Assert(src != 0 && dst != 0 && src < unit->OrderCount && dst < unit->OrderCount);

	if (src == dst) {
		return;
	}

	if (src < dst) {
		tmp = unit->Orders[src];
		for(i = src; i < dst; i++) {
			unit->Orders[i] = unit->Orders[i+1];
		}
		unit->Orders[dst] = tmp;
	} else {
		// dst < src
		tmp = unit->Orders[src];
		for (i = src - 1 ; i >= dst; i--){
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
global void CommandStandGround(Unit* unit, int flush)
{
	Order* order;

	// Ignore that the unit could be removed.

	if (unit->Type->Building) {
		// FIXME: should find a better way for pending orders.
		order = &unit->NewOrder;
		ReleaseOrder(order);
	} else if (!(order = GetNextOrder(unit, flush))) {
		return;
	}
	order->Action = UnitActionStandGround;
	order->X = order->Y = -1;
	order->Goal = NoUnitP;
	order->Type = NULL;
	order->Arg1 = NULL;
	ClearSavedAction(unit);
}

/**
**  Follow unit to new position
**
**  @param unit   pointer to unit.
**  @param dest   unit to be followed
**  @param flush  if true, flush command queue.
*/
global void CommandFollow(Unit* unit, Unit* dest, int flush)
{
	Order* order;

	//
	// Check if unit is still valid? (NETWORK!)
	//
	if (!unit->Removed && unit->Orders[0].Action != UnitActionDie) {
		if (unit->Type->Building) {
			// FIXME: should find a better way for pending orders.
			order = &unit->NewOrder;
			ReleaseOrder(order);
		} else if (!(order = GetNextOrder(unit, flush))) {
			return;
		}

		order->Action = UnitActionFollow;
		//
		// Destination could be killed.
		// Should be handled in action, but is not possible!
		// Unit::Refs is used as timeout counter.
		//
		if (dest->Destroyed) {
			order->X = dest->X + dest->Type->TileWidth / 2;
			order->Y = dest->Y + dest->Type->TileHeight / 2;
			order->Goal = NoUnitP;
			order->Range = 0;
		} else {
			order->X = order->Y = -1;
			order->Goal = dest;
			RefsIncrease(dest);
			order->Range = 1;
		}
		order->Type = NULL;
		order->Arg1 = NULL;
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
global void CommandMove(Unit* unit, int x, int y, int flush)
{
	Order* order;

#ifdef DEBUG
	if (x < 0 || y < 0 || x >= TheMap.Width || y >= TheMap.Height) {
		DebugPrint("Internal movement error\n");
		return;
	}
#endif

	//
	//  Check if unit is still valid? (NETWORK!)
	//
	if (!unit->Removed && unit->Orders[0].Action != UnitActionDie) {
		if (unit->Type->Building) {
			// FIXME: should find a better way for pending orders.
			order = &unit->NewOrder;
			ReleaseOrder(order);
		} else if (!(order = GetNextOrder(unit, flush))) {
			return;
		}

		order->Action = UnitActionMove;
		order->Goal = NoUnitP;
		order->X = x;
		order->Y = y;
		order->Range = 0;
		order->Type = NULL;
		order->Arg1 = NULL;
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
global void CommandRepair(Unit* unit, int x, int y, Unit* dest, int flush)
{
	Order* order;

	//
	//  Check if unit is still valid? (NETWORK!)
	//
	if (!unit->Removed && unit->Orders[0].Action != UnitActionDie) {
		if (unit->Type->Building) {
			// FIXME: should find a better way for pending orders.
			order = &unit->NewOrder;
			ReleaseOrder(order);
		} else if (!(order = GetNextOrder(unit, flush))) {
			return;
		}

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
				order->Goal = NoUnitP;
				order->Range = 0;
				order->Width = order->Height = 0;
			} else {
				order->X = order->Y = -1;
				order->Width = order->Height = 0;
				order->Goal = dest;
				RefsIncrease(dest);
				order->Range = unit->Type->RepairRange;
			}
		} else {
			order->X = x;
			order->Y = y;
			order->Goal = NoUnitP;
			order->Range = 0;
		}
		order->Type = NULL;
		order->Arg1 = NULL;
	}
	ClearSavedAction(unit);
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
global void CommandAttack(Unit* unit, int x, int y, Unit* attack, int flush)
{
	Order* order;

#ifdef DEBUG
	if (x < 0 || y < 0 || x >= TheMap.Width || y >= TheMap.Height) {
		DebugPrint("Internal movement error\n");
		return;
	}
#endif

	//
	// Check if unit is still valid? (NETWORK!)
	//
	if (!unit->Removed && unit->Orders[0].Action != UnitActionDie) {
		if (unit->Type->Building) {
			// FIXME: should find a better way for pending orders.
			order = &unit->NewOrder;
			ReleaseOrder(order);
		} else if (!(order = GetNextOrder(unit, flush))) {
			return;
		}

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
				order->Goal = NoUnitP;
				order->Range = 0;
			} else {
				// Removed, Dying handled by action routine.
				order->X = order->Y = -1;
				order->Goal = attack;
				RefsIncrease(attack);
				order->Range = unit->Stats->AttackRange;
				order->MinRange = unit->Type->MinAttackRange;
			}
		} else if (WallOnMap(x,y)) {
			// FIXME: look into action_attack.c about this ugly problem
			order->X = x;
			order->Y = y;
			order->Range = unit->Stats->AttackRange;
			order->MinRange = unit->Type->MinAttackRange;
			order->Goal = NoUnitP;
		} else {
			order->X = x;
			order->Y = y;
			order->Range = 0;
			order->Goal = NoUnitP;
		}
		order->Type = NULL;
		order->Arg1 = NULL;

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
global void CommandAttackGround(Unit* unit, int x, int y, int flush)
{
	Order* order;

#ifdef DEBUG
	if (x < 0 || y < 0 || x >= TheMap.Width || y >= TheMap.Height) {
		DebugPrint("Internal movement error\n");
		return;
	}
#endif

	//
	// Check if unit is still valid? (NETWORK!)
	//
	if (!unit->Removed && unit->Orders[0].Action != UnitActionDie) {
		if (unit->Type->Building) {
			// FIXME: should find a better way for pending orders.
			order = &unit->NewOrder;
			ReleaseOrder(order);
		} else if (!(order = GetNextOrder(unit, flush))) {
			return;
		}

		order->Action = UnitActionAttackGround;
		order->X = x;
		order->Y = y;
		order->Range = unit->Stats->AttackRange;
		order->MinRange = unit->Type->MinAttackRange;
		order->Goal = NoUnitP;
		order->Type = NULL;
		order->Arg1 = NULL;

		DebugPrint("FIXME this next\n");
	}
	ClearSavedAction(unit);
}

/**
**  Let an unit patrol from current to new position
**
**  FIXME: want to support patroling between units.
**
**  @param unit   pointer to unit.
**  @param x      X map position to patrol between.
**  @param y      Y map position to patrol between.
**  @param flush  if true, flush command queue.
*/
global void CommandPatrolUnit(Unit* unit, int x, int y, int flush)
{
	Order* order;

#ifdef DEBUG
	if (x < 0 || y < 0 || x >= TheMap.Width || y >= TheMap.Height) {
		DebugPrint("Internal movement error\n");
		return;
	}
#endif

	//
	// Check if unit is still valid? (NETWORK!)
	//
	if (!unit->Removed && unit->Orders[0].Action != UnitActionDie) {
		if (unit->Type->Building) {
			// FIXME: should find a better way for pending orders.
			order = &unit->NewOrder;
			ReleaseOrder(order);
		} else if (!(order = GetNextOrder(unit, flush))) {
			return;
		}

		order->Action = UnitActionPatrol;
		order->Goal = NoUnitP;
		order->X = x;
		order->Y = y;
		order->Range = 0;
		order->Type = NULL;
		Assert(!(unit->X & ~0xFFFF) && !(unit->Y & ~0xFFFF));
		// FIXME: BUG-ALERT: encode source into arg1 as two 16 bit values!
		order->Arg1 = (void*)((unit->X << 16) | unit->Y);
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
global void CommandBoard(Unit* unit, Unit* dest, int flush)
{
	Order* order;

	//
	// Check if unit is still valid? (NETWORK!)
	//
	if (!unit->Removed && unit->Orders[0].Action != UnitActionDie) {
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

		order->Action = UnitActionBoard;
		order->X = order->Y = -1;
		order->Goal = dest;
		RefsIncrease(dest);
		order->Range = 1;
		order->Type = NULL;
		order->Arg1 = NULL;
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
global void CommandUnload(Unit* unit, int x, int y, Unit* what, int flush)
{
	Order* order;

	//
	// Check if unit is still valid? (NETWORK!)
	//
	if (!unit->Removed && unit->Orders[0].Action != UnitActionDie) {
		//
		//	For bunkers, don't go into an action. Just drop everything here and now.
		//
		if (unit->Type->Building) {
			int i;
			Unit* uins;

			// Unload all units.
			uins = unit->UnitInside;
			for (i = unit->InsideCount; i; --i, uins = uins->NextContained) {
				if (uins->Boarded) {
					uins->X = unit->X;
					uins->Y = unit->Y;
					if (UnloadUnit(uins)) {
						unit->BoardCount--;
					}
				}
			}
			return;
		}

		if (!(order = GetNextOrder(unit, flush))) {
			return;
		}

		order->Action = UnitActionUnload;
		order->X = x;
		order->Y = y;
		//
		// Destination could be killed.
		// Should be handled in action, but is not possible!
		// Unit::Refs is used as timeout counter.
		//
		order->Goal = NoUnitP;
		if (what && !what->Destroyed) {
			order->Goal = what;
			RefsIncrease(what);
		}
		order->Range = 0;
		order->Type = NULL;
		order->Arg1 = NULL;
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
global void CommandBuildBuilding(Unit* unit, int x, int y,
	UnitType* what, int flush)
{
	Order* order;

	//
	// Check if unit is still valid? (NETWORK!)
	//
	if (!unit->Removed && unit->Orders[0].Action != UnitActionDie) {
		if (unit->Type->Building) {
			// FIXME: should find a better way for pending orders.
			order = &unit->NewOrder;
			ReleaseOrder(order);
		} else if (!(order = GetNextOrder(unit, flush))) {
			return;
		}

		order->Action = UnitActionBuild;
		order->Goal = NoUnitP;
		order->X = x;
		order->Y = y;
		order->Width = what->TileWidth;
		order->Height = what->TileHeight;
		if (what->BuilderOutside) {
			order->Range = unit->Type->RepairRange;
		} else {
			// If building inside, but be next to stop
			if (what->ShoreBuilding && unit->Type->UnitType == UnitTypeLand) {
					// Peon won't dive :-)
				order->Range = 1;
			} else {
				order->Range = 0;
			}
		}
		order->Type = what;
		if (what->BuilderOutside) {
			order->MinRange = 1;
		} else {
			order->MinRange = 0;
		}
		order->Arg1 = NULL;
	}
	ClearSavedAction(unit);
}

/**
**  Cancel the building construction, or kill an unit.
**
**  @param unit    pointer to unit.
*/
global void CommandDismiss(Unit* unit)
{
	//
	// Check if building is still under construction? (NETWORK!)
	//
	if (unit->Orders[0].Action == UnitActionBuilded) {
		unit->Data.Builded.Cancel = 1;
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
**  @param x      X map position for harvest.
**  @param y      Y map position for harvest.
**  @param flush  if true, flush command queue.
*/
global void CommandResourceLoc(Unit* unit, int x, int y, int flush)
{
	Order* order;
	int nx;
	int ny;

	//
	// Check if unit is still valid? (NETWORK!)
	//
	if (!unit->Removed && unit->Orders[0].Action != UnitActionDie) {
		if (unit->Type->Building) {
			// FIXME: should find a better way for pending orders.
			order = &unit->NewOrder;
			ReleaseOrder(order);
		} else if (!(order = GetNextOrder(unit, flush))) {
			return;
		}

		order->Action = UnitActionResource;

		//  Find the closest piece of wood next to a tile where the unit can move
		if (!FindTerrainType(0, (unit->Type->MovementMask), 1, 20,
				unit->Player, x, y, &nx, &ny)) {
			DebugPrint("FIXME: Give up???\n");
		}

		// Max Value > 1
		if ((abs(nx - x) | abs(ny - y)) > 1) {
			if (!FindTerrainType(0, MapFieldForest, 0, 20, unit->Player,
					nx, ny, &nx, &ny)) {
				DebugPrint("FIXME: Give up???\n");
			}
		} else {
			// The destination is next to a reacahble tile.
			nx = x;
			ny = y;
		}
		order->X = nx;
		order->Y = ny;

		order->Range = 1;
		order->Goal = NoUnitP;
		order->Type = NULL;
		order->Arg1 = NULL;
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
global void CommandResource(Unit* unit, Unit* dest, int flush)
{
	Order* order;

	//
	// Check if unit is still valid and Goal still alive? (NETWORK!)
	//
	if (!unit->Removed && unit->Orders[0].Action != UnitActionDie &&
			!dest->Destroyed) {
		// FIXME: more races, could happen with many orders in queue.
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

		order->Action = UnitActionResource;
		order->X = order->Y = -1;
		order->Goal = dest;
		RefsIncrease(dest);
		order->Range = 1;
		order->Type = NULL;
		order->Arg1 = NULL;
	}
	ClearSavedAction(unit);
}

/**
**  Let unit returning goods.
**
**  @param unit   pointer to unit.
**  @param goal   bring goods to this depot.
**  @param flush  if true, flush command queue.
*/
global void CommandReturnGoods(Unit* unit, Unit* goal, int flush)
{
	Order* order;

	//
	// Check if unit is still valid and Goal still alive? (NETWORK!)
	//
	if (!unit->Removed && unit->Orders[0].Action != UnitActionDie) {
		// FIXME: more races, could happen with many orders in queue.
		if (!unit->Type->Building && !unit->Type->Harvester && !unit->Value) {
			ClearSavedAction(unit);
			return;
		}

		if (unit->Type->Building) {
			// FIXME: should find a better way for pending orders.
			order = &unit->NewOrder;
			ReleaseOrder(order);
		} else if (!(order = GetNextOrder(unit, flush))) {
			return;
		}

		order->Action = UnitActionReturnGoods;
		order->X = order->Y = -1;
		order->Goal = NoUnitP;
		//
		//		Destination could be killed. NETWORK!
		//
		if (goal && !goal->Destroyed) {
			order->Goal = goal;
			RefsIncrease(goal);
		}
		order->Range = 1;
		order->Type = NULL;
		order->Arg1 = NULL;
	}
	ClearSavedAction(unit);
}

/**
**  Building starts training an unit.
**
**  @param unit   pointer to unit.
**  @param type   unit type to train.
**  @param flush  if true, flush command queue.
*/
global void CommandTrainUnit(Unit* unit, UnitType* type,
	int flush __attribute__((unused)))
{
	//
	// Check if unit is still valid? (NETWORK!)
	//
	if (!unit->Removed && unit->Orders[0].Action != UnitActionDie) {
		//
		//		Check if enough resources remains? (NETWORK!)
		//		FIXME: wrong if append to message queue!!!
		//
		if (PlayerCheckLimits(unit->Player, type) < 0 ||
				PlayerCheckUnitType(unit->Player, type)) {
			return;
		}

		//
		// Not already training?
		//
		if (unit->Orders[0].Action != UnitActionTrain) {
			if (unit->OrderCount == 2 && unit->Orders[1].Action == UnitActionTrain) {
				DebugPrint("FIXME: not supported. Unit queue full!\n");
				return;
			} else {
				ReleaseOrders(unit);
				unit->Orders[1].Action = UnitActionTrain;
			}
			Assert(unit->OrderCount == 1 && unit->OrderFlush == 1);

			unit->OrderCount = 2;
			unit->Orders[1].Type = type;
			unit->Orders[1].X = unit->Orders[1].Y = -1;
			unit->Orders[1].Goal = NoUnitP;
			unit->Orders[1].Arg1 = NULL;
		} else {
			//
			// Training slots are all already full. (NETWORK!)
			//
			if (!EnableTrainingQueue || unit->Data.Train.Count >= MAX_UNIT_TRAIN) {
				DebugPrint("Unit queue full!\n");
				return;
			}

			unit->Data.Train.What[unit->Data.Train.Count++] = type;
		}
		// FIXME: if you give quick an other order, the resources are lost!
		PlayerSubUnitType(unit->Player, type);
	}
	ClearSavedAction(unit);
}

/**
**  Cancel the training of an unit.
**
**  @param unit  pointer to unit.
**  @param slot  slot number to cancel.
**  @param type  Unit-type to cancel.
*/
global void CommandCancelTraining(Unit* unit, int slot, const UnitType* type)
{
	int i;
	int n;

	DebugPrint("Cancel %d type: %s\n" _C_ slot _C_
		type ? type->Ident : "-any-");

	ClearSavedAction(unit);

	//
	// Check if unit is still training 'slot'? (NETWORK!)
	//
	if (unit->Orders[0].Action == UnitActionTrain) {
		n = unit->Data.Train.Count;
		Assert(n >= 1);
		if (slot == -1) {				// default last slot!
			slot += n;
		}
		//
		// Check if slot and unit-type is still trained? (NETWORK!)
		//
		if (slot >= n || (type && unit->Data.Train.What[slot] != type)) {
			// FIXME: we can look if this is now in an earlier slot.
			return;
		}

		DebugPrint	("Cancel training\n");

		PlayerAddCostsFactor(unit->Player,
			unit->Data.Train.What[slot]->Stats[unit->Player->Player].Costs,
			CancelTrainingCostsFactor);

		if (--n) {
			// Copy the other slots down
			for (i = slot; i < n; ++i) {
				unit->Data.Train.What[i] = unit->Data.Train.What[i + 1];
			}
			if (!slot) {				// Canceled in work slot
				unit->Data.Train.Ticks = 0;
				unit->Wait = unit->Reset = 1;		// immediately start next training
			}
			unit->Data.Train.Count = n;
		} else {
			DebugPrint("Last slot\n");
			unit->Orders[0].Action = UnitActionStill;
			unit->SubAction = 0;
			unit->Wait = unit->Reset = 1;
		}

		//
		// Update interface.
		//
		if (unit->Player == ThisPlayer && unit->Selected) {
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
global void CommandUpgradeTo(Unit* unit, UnitType* type, int flush)
{
	Order* order;

	//
	// Check if unit is still valid and Goal still alive? (NETWORK!)
	//
	if (!unit->Removed && unit->Orders[0].Action != UnitActionDie) {
		//
		// Check if enough resources remains? (NETWORK!)
		//
		if (PlayerCheckUnitType(unit->Player, type)) {
			return;
		}

		if (!flush) {
			DebugPrint("FIXME: must support order queing!!");
		}
		if (!(order = GetNextOrder(unit, flush))) {
			return;
		}

		// FIXME: if you give quick an other order, the resources are lost!
		PlayerSubUnitType(unit->Player, type);

		order->Action = UnitActionUpgradeTo;
		order->X = order->Y = -1;
		order->Goal = NoUnitP;
		order->Type = type;
		order->Arg1 = NULL;
	}
	ClearSavedAction(unit);
}

/**
**  Cancel building upgrading to.
**
**  @param unit  pointer to unit.
*/
global void CommandCancelUpgradeTo(Unit* unit)
{
	ReleaseOrders(unit);				// empty command queue

	//
	// Check if unit is still upgrading? (NETWORK!)
	//
	if (unit->Orders[0].Action == UnitActionUpgradeTo) {

		PlayerAddCostsFactor(unit->Player,
			unit->Orders[0].Type->Stats[unit->Player->Player].Costs,
			CancelUpgradeCostsFactor);

		unit->Orders[0].Action = UnitActionStill;
		unit->Orders[0].X = unit->Orders[0].Y = -1;
		unit->Orders[0].Goal = NoUnitP;
		unit->Orders[0].Type = NULL;
		unit->Orders[0].Arg1 = NULL;

		unit->SubAction = 0;

		//
		// Update interface.
		//
		if (unit->Player == ThisPlayer && unit->Selected) {
			SelectedUnitChanged();
		}

		unit->Wait = unit->Reset = 1;		// immediately start next command.
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
global void CommandResearch(Unit* unit, Upgrade* what, int flush)
{
	Order* order;

	//
	// Check if unit is still valid and Goal still alive? (NETWORK!)
	//
	if (!unit->Removed && unit->Orders[0].Action != UnitActionDie) {
		//
		// Check if enough resources remains? (NETWORK!)
		//
		if (PlayerCheckCosts(unit->Player, what->Costs)) {
			return;
		}

		if (!flush) {
			DebugPrint("FIXME: must support order queing!!");
		} else {
			if (unit->Orders[0].Action == UnitActionResearch) {
				const Upgrade* upgrade;

				// Cancel current research
				upgrade = unit->Data.Research.Upgrade;
				unit->Player->UpgradeTimers.Upgrades[upgrade-Upgrades] = 0;
				PlayerAddCostsFactor(unit->Player,upgrade->Costs,
					CancelResearchCostsFactor);
				unit->SubAction = 0;
				unit->Wait = unit->Reset = 1;	// immediately start next command.
			}
		}

		if (!(order = GetNextOrder(unit, flush))) {
			return;
		}

		// FIXME: if you give quick an other order, the resources are lost!
		PlayerSubCosts(unit->Player, what->Costs);

		order->Action = UnitActionResearch;
		order->X = order->Y = -1;
		order->Goal = NoUnitP;
		order->Type = NULL;
		order->Arg1 = what;
	}
	ClearSavedAction(unit);
}

/**
**  Cancel Building researching.
**
**  @param unit  Pointer to unit.
*/
global void CommandCancelResearch(Unit* unit)
{
	ReleaseOrders(unit);				// empty command queue

	//
	// Check if unit is still researching? (NETWORK!)
	//
	if (unit->Orders[0].Action == UnitActionResearch) {
		const Upgrade* upgrade;

		upgrade = unit->Data.Research.Upgrade;
		unit->Player->UpgradeTimers.Upgrades[upgrade-Upgrades] = 0;

		PlayerAddCostsFactor(unit->Player,upgrade->Costs,
			CancelResearchCostsFactor);

		unit->Orders[0].Action = UnitActionStill;
		unit->Orders[0].X = unit->Orders[0].Y = -1;
		unit->Orders[0].Goal = NoUnitP;
		unit->Orders[0].Type = NULL;
		unit->Orders[0].Arg1 = NULL;

		unit->SubAction = 0;

		//
		// Update interface.
		//
		if (unit->Player == ThisPlayer && unit->Selected) {
			SelectedUnitChanged();
		}

		unit->Wait = unit->Reset = 1;		// immediately start next command.
	}
	ClearSavedAction(unit);
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
global void CommandSpellCast(Unit* unit, int x, int y, Unit* dest,
	SpellType* spell, int flush)
{
	Order* order;

#ifdef DEBUG
	if (x < 0 || y < 0 || x >= TheMap.Width || y >= TheMap.Height) {
		DebugPrint("Internal movement error\n");
		return;
	}
#endif

	DebugPrint(": %d casts %s at %d %d on %d\n" _C_
		UnitNumber(unit) _C_ spell->Ident _C_ x _C_ y _C_ dest ? UnitNumber(dest) : 0);
	Assert(unit->Type->CanCastSpell[spell->Slot]);

	//
	// Check if unit is still valid? (NETWORK!)
	//
	if (!unit->Removed && unit->Orders[0].Action != UnitActionDie) {
		// FIXME: should I check here, if there is still enough mana?

		if (!(order = GetNextOrder(unit, flush))) {
			return;
		}

		order->Action = UnitActionSpellCast;
		order->Range = spell->Range;
		if (dest) {
			//
			// Destination could be killed.
			// Should be handled in action, but is not possible!
			// Unit::Refs is used as timeout counter.
			//
			if (dest->Destroyed) {
				// FIXME: where check if spell needs an unit as destination?
				// FIXME: dest->Type is now set to 0. maybe we shouldn't bother.
				order->X = dest->X /*+ dest->Type->TileWidth / 2*/  - order->Range;
				order->Y = dest->Y /*+ dest->Type->TileHeight / 2*/ - order->Range;
				order->Goal = NoUnitP;
				order->Range <<= 1;
				order->Range <<= 1;
			} else {
				order->X = order->Y = -1;
				order->Goal = dest;
				RefsIncrease(dest);
			}
		} else {
			order->X = x;
			order->Y = y;
			order->Range <<= 1;
		}
		order->Type = NULL;
		order->Arg1 = spell;
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
global void CommandAutoSpellCast(Unit* unit, int spellid, int on)
{
	//
	// Check if unit is still valid? (NETWORK!)
	//
	if (!unit->Removed && unit->Orders[0].Action != UnitActionDie) {
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
global void CommandDiplomacy(int player, int state, int opponent)
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
global void CommandSharedVision(int player, int state, int opponent)
{
	int before;
	int after;
	int x;
	int y;
	int i;

	//
	//	Do a real hardcore seen recount. First we unmark EVERYTHING.
	//
	for (i = 0; i < NumUnits; ++i) {
		if (!Units[i]->Destroyed) {
			MapUnmarkUnitSight(Units[i]);
		}
	}

	//
	//	Compute Before and after.
	//
	before = PlayersShareVision(player, opponent);
	if (state == 0) {
		Players[player].SharedVision &= ~(1 << opponent);
	} else {
		Players[player].SharedVision |= (1 << opponent);
	}
	after = PlayersShareVision(player, opponent);

	if (before && !after) {
		//
		//	Don't share vision anymore. Give each other explored terrain for good-bye.
		//
		for (x = 0; x < TheMap.Width; ++x) {
			for (y = 0; y < TheMap.Height; ++y) {
				i = x + y * TheMap.Width;
				if (TheMap.Fields[i].Visible[player] && !TheMap.Fields[i].Visible[opponent]) {
					TheMap.Fields[i].Visible[opponent] = 1;
					if (opponent == ThisPlayer->Player) {
						MapMarkSeenTile(x, y);
					}
				}
				if (TheMap.Fields[i].Visible[opponent] && !TheMap.Fields[i].Visible[player]) {
					TheMap.Fields[i].Visible[player] = 1;
					if (player == ThisPlayer->Player) {
						MapMarkSeenTile(x, y);
					}
				}
			}
		}
	}

	//
	//	Do a real hardcore seen recount. Now we remark EVERYTHING
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
global void CommandQuit(int player)
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
		SetMessage("Player \"%s\" has left the game", Players[player].Name);
	} else {
		SetMessage("Player \"%s\" has been killed", Players[player].Name);
	}
}

//@}
