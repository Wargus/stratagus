//     ____                _       __               
//    / __ )____  _____   | |     / /___ ___________
//   / __  / __ \/ ___/   | | /| / / __ `/ ___/ ___/
//  / /_/ / /_/ (__  )    | |/ |/ / /_/ / /  (__  ) 
// /_____/\____/____/     |__/|__/\__,_/_/  /____/  
//                                              
//       A futuristic real-time strategy game.
//          This file is part of Bos Wars.
//
/**@name action_train.cpp - The building train action. */
//
//      (c) Copyright 1998-2007 by Lutz Sammer and Jimmy Salmon
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

#include "stratagus.h"
#include "video.h"
#include "sound.h"
#include "unitsound.h"
#include "unittype.h"
#include "animation.h"
#include "player.h"
#include "unit.h"
#include "actions.h"
#include "sound.h"
#include "ai.h"
#include "interface.h"
#include "ui.h"

/*----------------------------------------------------------------------------
--  Functions
----------------------------------------------------------------------------*/

/**
**  Get the production cost of a unit type
**  Use the energy cost if it's not 0, otherwise use magma cost
*/
static int GetProductionCost(CUnitType *type)
{
	int *costs = type->ProductionCosts;
	return costs[EnergyCost] ? costs[EnergyCost] : costs[MagmaCost];
}

/**
**  Unit can handle order.
**
**  @param unit   Newly trained unit.
**  @param order  New order for the unit.
**
**  @return  1 if the the unit can do it, 0 otherwise.
*/
static int CanHandleOrder(CUnit *unit, COrder *order)
{
	if (order->Action == UnitActionResource) {
		//  Check if new unit can harvest.
		if (!unit->Type->Harvester) {
			return 0;
		}
		return 1;
	}
	if (order->Action == UnitActionAttack && !unit->Type->CanAttack) {
		return 0;
	}
	if (order->Action == UnitActionBoard && unit->Type->UnitType != UnitTypeLand) {
		return 0;
	}
	return 1;
}

/**
**  Unit trains unit!
**
**  @param unit  Unit that trains.
*/
void HandleActionTrain(CUnit *unit)
{
	CUnit *nunit;
	const CUnitType *type;
	CPlayer *player;
	int food;
	int pcost = GetProductionCost(unit->Orders[0]->Type);

	//
	// First entry
	//
	if (!unit->SubAction) {
		// Check the terrain here rather than in
		// CommandTrainUnit because, if the train command is
		// queued after a move, CommandTrainUnit might not
		// know in advance where the move will end.  Usually
		// though, units that can train are buildings and
		// unable to move.
		if (!TerrainAllowsTraining(unit, unit->Orders[0]->Type)) {
			unit->Player->Notify(NotifyYellow, unit->X, unit->Y,
				_("Cannot train %s here"),
				unit->Orders[0]->Type->Name.c_str());

			if (unit->Player->AiEnabled) {
				// There is no AiCanNotTrain but this
				// call will work.  To the AI, the
				// primary difference between training
				// and building is that trained units
				// get drafted into forces; there is
				// no such unit here because the
				// training attempt failed.
				AiCanNotBuild(unit, unit->Orders[0]->Type);
			}

			if (unit->OrderCount == 1) {
				unit->ClearAction();
			} else {
				unit->OrderFlush = 1;
				unit->SubAction = 0;
			}

			if (IsOnlySelected(unit)) {
				UI.ButtonPanel.Update();
			}

			return;
		}

		unit->Data.Train.Ticks = 0;
		unit->SubAction = 1;

		int costs[MaxCosts];
		CalculateRequestedAmount(unit->Type, unit->Orders[0]->Type->ProductionCosts, costs);
		unit->Player->AddToUnitsConsumingResources(unit, costs);
	} else {
		int *costs = unit->Player->UnitsConsumingResourcesActual[unit];
		int cost = costs[EnergyCost] ? costs[EnergyCost] : costs[MagmaCost];
		unit->Data.Train.Ticks += cost * SpeedTrain;
		if (unit->Data.Train.Ticks > pcost) {
			unit->Data.Train.Ticks = pcost;
		}
	}

	unit->Type->Animations->Train ?
		UnitShowAnimation(unit, unit->Type->Animations->Train) :
		UnitShowAnimation(unit, unit->Type->Animations->Still);
	if (unit->Wait) {
		unit->Wait--;
		return;
	}

	player = unit->Player;

	if (unit->Data.Train.Ticks >= pcost) {
		unit->Data.Train.Ticks = pcost;

		//
		// Check if there are still unit slots.
		//
		if (NumUnits >= UnitMax) {
			unit->Wait = CYCLES_PER_SECOND / 6;
			return;
		}

		//
		// Check if enough supply available.
		//
		food = player->CheckLimits(unit->Orders[0]->Type);
		if (food < 0) {
			unit->Data.Train.Ticks = pcost;
			unit->Wait = CYCLES_PER_SECOND / 6;
			return;
		}

		nunit = MakeUnit(unit->Orders[0]->Type, player);
		if (nunit != NoUnitP) {
			nunit->X = unit->X;
			nunit->Y = unit->Y;
			type = unit->Type;

			// DropOutOnSide set unit to belong to the building
			// training it. This was an ugly hack, setting X and Y is enough,
			// no need to add the unit only to be removed.
			nunit->X = unit->X;
			nunit->Y = unit->Y;

			player->RemoveFromUnitsConsumingResources(unit);

			// New unit might supply food
			UpdateForNewUnit(nunit, 0);

			DropOutOnSide(nunit, LookingW, type->TileWidth, type->TileHeight);

			// Set life span
			if (type->DecayRate) {
				nunit->TTL = GameCycle + type->DecayRate * 6 * CYCLES_PER_SECOND;
			}

			player->Notify(NotifyYellow, nunit->X, nunit->Y,
				_("New %s ready"), nunit->Type->Name.c_str());
			if (player == ThisPlayer) {
				PlayUnitSound(nunit, VoiceReady);
			}
			if (unit->Player->AiEnabled) {
				AiTrainingComplete(unit, nunit);
			}

			if (unit->OrderCount == 1) {
				unit->ClearAction();
			} else {
				unit->OrderFlush = 1;
				unit->SubAction = 0;
			}

			if (unit->NewOrder.Goal) {
				if (unit->NewOrder.Goal->Destroyed) {
					DebugPrint("Destroyed unit in train unit\n");
					unit->NewOrder.Goal->RefsDecrease();
					unit->NewOrder.Goal = NoUnitP;
					unit->NewOrder.Action = UnitActionStill;
				}
			}

			if (!CanHandleOrder(nunit, &unit->NewOrder)) {
				DebugPrint("Wrong order for unit\n");

				// Tell the unit to move instead of trying any funny stuff.
				*nunit->Orders[0] = unit->NewOrder;
				nunit->Orders[0]->Action = UnitActionMove;
				if (nunit->Orders[0]->Goal) {
					nunit->Orders[0]->Goal->RefsIncrease();
				}
			} else {
				*nunit->Orders[0] = unit->NewOrder;

				//
				// FIXME: Pending command uses any references?
				//
				if (nunit->Orders[0]->Goal) {
					nunit->Orders[0]->Goal->RefsIncrease();
				}
			}

			if (IsOnlySelected(unit)) {
				UI.ButtonPanel.Update();
			}
			return;
		} else {
			player->Notify(NotifyYellow, unit->X, unit->Y,
				_("Unable to train %s"), unit->Orders[0]->Type->Name.c_str());
		}
	}

	unit->Wait = CYCLES_PER_SECOND / 6;
}

//@}
