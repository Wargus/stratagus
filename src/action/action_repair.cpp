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
/**@name action_repair.cpp - The repair action. */
//
//      (c) Copyright 1999-2007 by Vladi Shabanski and Jimmy Salmon
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
#include "unittype.h"
#include "animation.h"
#include "player.h"
#include "unit.h"
#include "missile.h"
#include "actions.h"
#include "sound.h"
#include "tileset.h"
#include "map.h"
#include "pathfinder.h"
#include "interface.h"

/*----------------------------------------------------------------------------
--  Functions
----------------------------------------------------------------------------*/


/**
**  Repair a unit.
**
**  @param unit  unit repairing
**  @param goal  unit being repaired
*/
static void RepairUnit(CUnit &unit, CUnit &goal)
{
	if (goal.CurrentAction() == UnitActionBuilt) {
		COrder_Built &order = *static_cast<COrder_Built *>(goal.CurrentOrder());

		order.ProgressHp(goal, 100 * unit.CurrentOrder()->Data.Repair.Cycles);
		unit.CurrentOrder()->Data.Repair.Cycles = 0;
		return ;
	}
	CPlayer *player = unit.Player;
	char buf[100];

	// Calculate the repair costs.
	Assert(goal.Stats->Variables[HP_INDEX].Max);

	// Check if enough resources are available
	for (int i = 1; i < MaxCosts; ++i) {
		if (player->Resources[i] < goal.Type->RepairCosts[i]) {
			snprintf(buf, 100, _("We need more %s for repair!"),
				DefaultResourceNames[i].c_str());
			player->Notify(NotifyYellow, unit.tilePos.x, unit.tilePos.y, buf);
			if (player->AiEnabled) {
				// FIXME: call back to AI?
				unit.CurrentOrder()->ClearGoal();
				if (!unit.RestoreOrder()) {
					unit.ClearAction();
					unit.State = 0;
				}
			}
			// FIXME: We shouldn't animate if no resources are available.
			return;
		}
	}
	//
	// Subtract the resources
	//
	player->SubCosts(goal.Type->RepairCosts);

	goal.Variable[HP_INDEX].Value += goal.Type->RepairHP;
	if (goal.Variable[HP_INDEX].Value > goal.Variable[HP_INDEX].Max) {
		goal.Variable[HP_INDEX].Value = goal.Variable[HP_INDEX].Max;
	}
}

/**
**  Animate unit repair
**
**  @param unit  Unit, for that the repair animation is played.
*/
static int AnimateActionRepair(CUnit &unit)
{
	UnitShowAnimation(unit, unit.Type->Animations->Repair);
	return 0;
}

/**
**  Unit repairs
**
**  @param unit  Unit, for that the attack is handled.
*/
void HandleActionRepair(COrder& order, CUnit &unit)
{
	CUnit *goal;
	int err;

	switch (unit.SubAction) {
		case 0:
			order.NewResetPath();
			unit.SubAction = 1;
			// FALL THROUGH
		case 1:// Move near to target.
			// FIXME: RESET FIRST!! Why? We move first and than check if
			// something is in sight.
			err = DoActionMove(unit);
			if (!unit.Anim.Unbreakable) {
				// No goal: if meeting damaged building repair it.
				goal = order.GetGoal();

				// Target is dead, choose new one.
				//
				// Check if goal is correct unit.
				if (goal) {
					if (!goal->IsVisibleAsGoal(*unit.Player)) {
						DebugPrint("repair target gone.\n");
						order.goalPos = goal->tilePos;
						// FIXME: should I clear this here?
						order.ClearGoal();
						goal = NULL;
						order.NewResetPath();
					}
				} else if (unit.Player->AiEnabled) {
					// Ai players workers should stop if target is killed
					err = -1;
				}

				// Have reached target? FIXME: could use return value
				if (goal && unit.MapDistanceTo(*goal) <= unit.Type->RepairRange &&
						goal->Variable[HP_INDEX].Value < goal->Variable[HP_INDEX].Max) {
					unit.State = 0;
					unit.SubAction = 2;
					order.Data.Repair.Cycles = 0;
					const Vec2i dir = goal->tilePos + goal->Type->GetHalfTileSize() - unit.tilePos;
					UnitHeadingFromDeltaXY(unit, dir);
				} else if (err < 0) {
					order.ClearGoal();
					if (!unit.RestoreOrder()) {
						unit.ClearAction();
						unit.State = 0;
					}
					return;
				}

				// FIXME: Should be it already?
				Assert(unit.CurrentAction() == UnitActionRepair);
			}
			break;

		case 2:// Repair the target.
			AnimateActionRepair(unit);
			order.Data.Repair.Cycles++;
			if (!unit.Anim.Unbreakable) {
				goal = unit.CurrentOrder()->GetGoal();

				// Target is dead, choose new one.
				//
				// Check if goal is correct unit.
				// FIXME: should I do a function for this?
				if (goal) {
					if (!goal->IsVisibleAsGoal(*unit.Player)) {
						DebugPrint("repair goal is gone\n");
						order.goalPos = goal->tilePos;
						// FIXME: should I clear this here?
						order.ClearGoal();
						goal = NULL;
						order.NewResetPath();
					} else {
						int dist = unit.MapDistanceTo(*goal);
						if (dist <= unit.Type->RepairRange) {
							RepairUnit(unit, *goal);
							goal = order.GetGoal();
						} else if (dist > unit.Type->RepairRange) {
							// If goal has move, chase after it
							unit.State = 0;
							unit.SubAction = 0;
						}
					}
				}

				// Target is fine, choose new one.
				if (!goal || goal->Variable[HP_INDEX].Value >= goal->Variable[HP_INDEX].Max) {
					order.ClearGoal();
					if (!unit.RestoreOrder()) {
						unit.ClearAction();
						unit.State = 0;
					}
					return;
				}
				// FIXME: automatic repair
			}
			break;
	}
}

//@}
