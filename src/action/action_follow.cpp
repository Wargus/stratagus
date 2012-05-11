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

#include "action/action_follow.h"

#include "iolib.h"
#include "pathfinder.h"
#include "script.h"
#include "ui.h"
#include "unit.h"
#include "unittype.h"

enum {
	State_Init = 0,
	State_Initialized = 1,

	State_TargetReached = 128,
};



/*----------------------------------------------------------------------------
--  Functions
----------------------------------------------------------------------------*/

/* static */ COrder *COrder::NewActionFollow(CUnit &dest)
{
	COrder_Follow *order = new COrder_Follow;

	// Destination could be killed.
	// Should be handled in action, but is not possible!
	// Unit::Refs is used as timeout counter.
	if (dest.Destroyed) {
		order->goalPos = dest.tilePos + dest.Type->GetHalfTileSize();
	} else {
		order->SetGoal(&dest);
		order->Range = 1;
	}
	return order;
}

/* virtual */ void COrder_Follow::Save(CFile &file, const CUnit &unit) const
{
	file.printf("{\"action-follow\",");

	if (this->Finished) {
		file.printf(" \"finished\", ");
	}
	file.printf(" \"range\", %d,", this->Range);
	if (this->HasGoal()) {
		file.printf(" \"goal\", \"%s\",", UnitReference(this->GetGoal()).c_str());
	}
	file.printf(" \"tile\", {%d, %d},", this->goalPos.x, this->goalPos.y);

	file.printf(" \"state\", %d", this->State);

	file.printf("}");
}

/* virtual */ bool COrder_Follow::ParseSpecificData(lua_State *l, int &j, const char *value, const CUnit &unit)
{
	if (!strcmp(value, "state")) {
		++j;
		lua_rawgeti(l, -1, j + 1);
		this->State = LuaToNumber(l, -1);
		lua_pop(l, 1);
	} else if (!strcmp(value, "range")) {
		++j;
		lua_rawgeti(l, -1, j + 1);
		this->Range = LuaToNumber(l, -1);
		lua_pop(l, 1);
	} else if (!strcmp(value, "tile")) {
		++j;
		lua_rawgeti(l, -1, j + 1);
		CclGetPos(l, &this->goalPos.x , &this->goalPos.y);
		lua_pop(l, 1);
	} else {
		return false;
	}
	return true;
}

/* virtual */ bool COrder_Follow::IsValid() const
{
	return true;
}

/* virtual */ PixelPos COrder_Follow::Show(const CViewport &vp, const PixelPos &lastScreenPos) const
{
	PixelPos targetPos;

	if (this->HasGoal()) {
		targetPos = vp.MapToScreenPixelPos(this->GetGoal()->GetMapPixelPosCenter());
	} else {
		targetPos = vp.TilePosToScreen_Center(this->goalPos);
	}
	Video.FillCircleClip(ColorGreen, lastScreenPos, 2);
	Video.DrawLineClip(ColorGreen, lastScreenPos, targetPos);
	Video.FillCircleClip(ColorGreen, targetPos, 3);
	return targetPos;
}

/* virtual */ void COrder_Follow::UpdatePathFinderData(PathFinderInput &input)
{
	input.SetMinRange(0);
	input.SetMaxRange(this->Range);

	Vec2i tileSize;
	if (this->HasGoal()) {
		CUnit *goal = this->GetGoal();
		tileSize.x = goal->Type->TileWidth;
		tileSize.y = goal->Type->TileHeight;
		input.SetGoal(goal->tilePos, tileSize);
	} else {
		tileSize.x = 0;
		tileSize.y = 0;
		input.SetGoal(this->goalPos, tileSize);
	}
}


/* virtual */ void COrder_Follow::Execute(CUnit &unit)
{
	if (unit.Wait) {
		unit.Wait--;
		return ;
	}
	CUnit *goal = this->GetGoal();

	// Reached target
	if (this->State == State_TargetReached) {

		if (!goal || !goal->IsVisibleAsGoal(*unit.Player)) {
			DebugPrint("Goal gone\n");
			this->Finished = true;
			return ;
		}

		if (goal->tilePos == this->goalPos) {
			// Move to the next order
			if (unit.Orders.size() > 1) {
				this->Finished = true;
				return ;
			}

			// Reset frame to still frame while we wait
			// FIXME: Unit doesn't animate.
			unit.Frame = unit.Type->StillFrame;
			UnitUpdateHeading(unit);
			unit.Wait = 10;
			if (this->Range > 1) {
				this->Range = 1;
				this->State = State_Init;
			}
			return ;
		}
		this->State = State_Init;
	}
	if (this->State == State_Init) { // first entry
		this->State = State_Initialized;
	}
	switch (DoActionMove(unit)) { // reached end-point?
		case PF_UNREACHABLE:
			// Some tries to reach the goal
			this->Range++;
			break;
		case PF_REACHED: {
			if (!goal) { // goal has died
				this->Finished = true;
				return ;
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
							unit.GetMapPixelPosCenter(),
							unit.GetMapPixelPosCenter());
#endif
				// FIXME: we must check if the units supports the new order.
				CUnit &dest = *goal->Goal;

				if (dest.NewOrder == NULL
					|| (dest.NewOrder->Action == UnitActionResource && !unit.Type->Harvester)
					|| (dest.NewOrder->Action == UnitActionAttack && !unit.Type->CanAttack)
					|| (dest.NewOrder->Action == UnitActionBoard && unit.Type->UnitType != UnitTypeLand)) {
					this->Finished = true;
					return ;
				} else {
					if (dest.NewOrder->HasGoal()) {
						if (dest.NewOrder->GetGoal()->Destroyed) {
							delete dest.NewOrder;
							dest.NewOrder = NULL;
							this->Finished = true;
							return ;
						}
					}
					unit.Orders.insert(unit.Orders.begin() + 1, dest.NewOrder->Clone());
					this->Finished = true;
					return ;
				}
			}
			this->goalPos = goal->tilePos;
			this->State = State_TargetReached;
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
	}

	if (unit.Anim.Unbreakable) {
		return ;
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

			this->Finished = true;
			unit.Orders.insert(unit.Orders.begin() + 1, COrder::NewActionAttack(unit, target->tilePos));

			if (unit.StoreOrder(savedOrder) == false) {
				delete savedOrder;
				savedOrder = NULL;
			}
		}
	}
}

//@}
