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
/**@name action_follow.cpp - The follow action. */
//
//      (c) Copyright 2001-2005 by Lutz Sammer and Jimmy Salmon
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
#include "unit.h"
#include "unittype.h"
#include "pathfinder.h"
#include "actions.h"
#include "iolib.h"
#include "script.h"

/*----------------------------------------------------------------------------
--  Functions
----------------------------------------------------------------------------*/


/* virtual */ void COrder_Follow::Save(CFile &file, const CUnit &unit) const
{
	file.printf("{\"action-follow\",");

	file.printf(" \"range\", %d,", this->Range);
	if (this->HasGoal()) {
		CUnit &goal = *this->GetGoal();
		if (goal.Destroyed) {
			/* this unit is destroyed so it's not in the global unit
			 * array - this means it won't be saved!!! */
			printf ("FIXME: storing destroyed Goal - loading will fail.\n");
		}
		file.printf(" \"goal\", \"%s\",", UnitReference(goal).c_str());
	}
	file.printf(" \"tile\", {%d, %d},", this->goalPos.x, this->goalPos.y);

	file.printf(" \"state\", %d,", this->State);
	SaveDataMove(file);

	file.printf("}");
}

/* virtual */ bool COrder_Follow::ParseSpecificData(lua_State *l, int &j, const char *value, const CUnit &unit)
{
	if (ParseMoveData(l, j, value)) {
		return true;
	} else if (!strcmp(value, "state")) {
		++j;
		lua_rawgeti(l, -1, j + 1);
		this->State = LuaToNumber(l, -1);
		lua_pop(l, 1);
	} else {
		return false;
	}
	return true;
}

/* virtual */ bool COrder_Follow::Execute(CUnit &unit)
{
	if (unit.Wait) {
		unit.Wait--;
		return false;
	}
	CUnit *goal = this->GetGoal();

	// Reached target
	if (this->State == 128) {

		if (!goal || !goal->IsVisibleAsGoal(*unit.Player)) {
			DebugPrint("Goal gone\n");
			return true;
		}

		if (goal->tilePos == this->goalPos) {
			// Move to the next order
			if (unit.Orders.size() > 1) {
				return true;
			}

			// Reset frame to still frame while we wait
			// FIXME: Unit doesn't animate.
			unit.Frame = unit.Type->StillFrame;
			UnitUpdateHeading(unit);
			unit.Wait = 10;
			if (this->Range > 1) {
				this->Range = 1;
				this->State = 0;
			}
			return false;
		}
		this->State = 0;
	}
	if (!this->State) { // first entry
		this->State = 1;
		this->NewResetPath();
	}
	switch (DoActionMove(unit)) { // reached end-point?
		case PF_UNREACHABLE:
			// Some tries to reach the goal
			if (this->CheckRange()) {
				this->Range++;
				break;
			}
			// FALL THROUGH
		case PF_REACHED:
		{
			if (!goal) { // goal has died
				return true;
			}
			// Handle Teleporter Units
			// FIXME: BAD HACK
			if (goal->Type->Teleporter && goal->Goal && unit.MapDistanceTo(*goal) <= 1) {
				// Teleport the unit
				unit.Remove(NULL);
				unit.tilePos = goal->Goal->tilePos;
				DropOutOnSide(unit, unit.Direction, NULL);
#if 0
				// FIXME: SoundForName() should be called once
				PlayGameSound(SoundForName("invisibility"), MaxSampleVolume);
				// FIXME: MissileTypeByIdent() should be called once
				MakeMissile(MissileTypeByIdent("missile-normal-spell"),
					unit.tilePos.x * PixelTileSize.x + PixelTileSize.x / 2,
					unit.tilePos.y * PixelTileSize.y + PixelTileSize.y / 2,
					unit.tilePos.x * PixelTileSize.x + PixelTileSize.x / 2,
					unit.tilePos.y * PixelTileSize.y + PixelTileSize.y / 2);
#endif
				// FIXME: we must check if the units supports the new order.
				CUnit &dest = *goal->Goal;

				if (dest.NewOrder == NULL
					|| (dest.NewOrder->Action == UnitActionResource && !unit.Type->Harvester)
					|| (dest.NewOrder->Action == UnitActionAttack && !unit.Type->CanAttack)
					|| (dest.NewOrder->Action == UnitActionBoard && unit.Type->UnitType != UnitTypeLand)) {
					return true;
				} else {
					if (dest.NewOrder->HasGoal()) {
						if (dest.NewOrder->GetGoal()->Destroyed) {
							delete dest.NewOrder;
							dest.NewOrder = NULL;
							return true;
						}
					}

					delete unit.CurrentOrder();
					unit.Orders[0] = dest.NewOrder->Clone();
					unit.CurrentResource = dest.CurrentResource;
					return false;
				}
			}
			this->goalPos = goal->tilePos;
			this->State = 128;
		}
			// FALL THROUGH
		default:
			break;
	}

	// Target destroyed?
	if (goal && !goal->IsVisibleAsGoal(*unit.Player)) {
		DebugPrint("Goal gone\n");
		this->goalPos = goal->tilePos + goal->Type->GetHalfTileSize();
		this->ClearGoal();
		goal = NoUnitP;
		this->NewResetPath();
	}

	if (unit.Anim.Unbreakable) {
		return false;
	}
	// If our leader is dead or stops or attacks:
	// Attack any enemy in reaction range.
	// If don't set the goal, the unit can than choose a
	//  better goal if moving nearer to enemy.
	if (unit.Type->CanAttack
		&& (!goal || goal->CurrentAction() == UnitActionAttack || goal->CurrentAction() == UnitActionStill)) {
		CUnit *target = AttackUnitsInReactRange(unit);
		if (target) {
			// Save current command to come back.
			COrder *savedOrder = this->Clone();

			CommandAttack(unit, target->tilePos, NULL, FlushCommands);

			if (unit.StoreOrder(savedOrder) == false) {
				delete savedOrder;
				savedOrder = NULL;
			}
			return true;
		}
	}
	return false;
}

void HandleActionFollow(COrder& order, CUnit &unit)
{
	if (order.Execute(unit)) {
		unit.ClearAction();
	}
}

//@}
