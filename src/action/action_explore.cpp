//       _________ __                 __
//      /   _____//  |_____________ _/  |______     ____  __ __  ______
//      \_____  \\   __\_  __ \__  \\   __\__  \   / ___\|  |  \/  ___/
//      /        \|  |  |  | \// __ \|  |  / __ \_/ /_/  >  |  /\___ |
//     /_______  /|__|  |__|  (____  /__| (____  /\___  /|____//____  >
//             \/                  \/          \//_____/            \/
//  ______________________                           ______________________
//                        T H E   W A R   B E G I N S
//           Stratagus - A free fantasy real time strategy game engine
//
/**@name action_explore.cpp - The explore action. */
//
//      (c) Copyright 1998-2019 by Lutz Sammer, Jimmy Salmon and Talas
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

#include "action/action_explore.h"

#include "animation.h"
#include "iolib.h"
#include "map.h"
#include "pathfinder.h"
#include "script.h"
#include "tile.h"
#include "ui.h"
#include "unit.h"
#include "unittype.h"
#include "video.h"

/*----------------------------------------------------------------------------
--  Functions
----------------------------------------------------------------------------*/

static void GetExplorationTarget(const CUnit &unit, Vec2i &dest)
{
	int triesLeft = Map.NoFogOfWar ? 0 : 3;
	CMapField *field;
	const CPlayer &player = *unit.Player;
	dest.x = SyncRand(Map.Info.MapWidth - 1) + 1;
	dest.y = SyncRand(Map.Info.MapHeight - 1) + 1;

	while (triesLeft > 0) {
		field = Map.Field(dest);
		if (field && !field->playerInfo.IsExplored(player))
			return; // unexplored, go here!
		dest.x = SyncRand(Map.Info.MapWidth - 1) + 1;
		dest.y = SyncRand(Map.Info.MapHeight - 1) + 1;
		--triesLeft;
	}
}

/* static */ COrder *COrder::NewActionExplore(const CUnit &unit)
{
	Vec2i dest;
	GetExplorationTarget(unit, dest);
	Assert(Map.Info.IsPointOnMap(dest));

	COrder_Explore *order = new COrder_Explore();

	order->goalPos = dest;
	return order;
}


/* virtual */ void COrder_Explore::Save(CFile &file, const CUnit &unit) const
{
	file.printf("{\"action-explore\",");

	if (this->Finished) {
		file.printf(" \"finished\", ");
	}
	file.printf(" \"tile\", {%d, %d},", this->goalPos.x, this->goalPos.y);
	file.printf(" \"range\", %d,", this->Range);

	if (this->WaitingCycle != 0) {
		file.printf(" \"waiting-cycle\", %d,", this->WaitingCycle);
	}
	file.printf("}");
}

/* virtual */ bool COrder_Explore::ParseSpecificData(lua_State *l, int &j, const char *value, const CUnit &unit)
{
	if (!strcmp(value, "waiting-cycle")) {
		++j;
		this->WaitingCycle = LuaToNumber(l, -1, j + 1);
	} else if (!strcmp(value, "range")) {
		++j;
		this->Range = LuaToNumber(l, -1, j + 1);
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

/* virtual */ bool COrder_Explore::IsValid() const
{
	return true;
}

/* virtual */ PixelPos COrder_Explore::Show(const CViewport &vp, const PixelPos &lastScreenPos) const
{
	const PixelPos pos1 = vp.TilePosToScreen_Center(this->goalPos);

	Video.DrawLineClip(ColorGreen, lastScreenPos, pos1);
	Video.FillCircleClip(ColorBlue, pos1, 2);
	return pos1;
}

/* virtual */ void COrder_Explore::UpdatePathFinderData(PathFinderInput &input)
{
	input.SetMinRange(0);
	input.SetMaxRange(this->Range);
	const Vec2i tileSize(0, 0);
	input.SetGoal(this->goalPos, tileSize);
}


/* virtual */ void COrder_Explore::Execute(CUnit &unit)
{
	if (IsWaiting(unit)) {
		return;
	}
	StopWaiting(unit);

	switch (DoActionMove(unit)) {
		case PF_FAILED:
			this->WaitingCycle = 0;
			break;
		case PF_UNREACHABLE:
			// Increase range and try again
			this->WaitingCycle = 1;
			this->Range++;
			break;

		case PF_REACHED:
		{
			this->WaitingCycle = 1;
			this->Range = 0;
			// pick a new place to explore
			GetExplorationTarget(unit, this->goalPos);
		}
			break;
		case PF_WAIT:
		{
			// Wait for a while then give up
			this->WaitingCycle++;
			if (this->WaitingCycle == 5) {
				this->WaitingCycle = 0;
				this->Range = 0;
				// pick a new place to explore
				GetExplorationTarget(unit, this->goalPos);

				unit.pathFinderData->output.Cycles = 0; //moving counter
			}
		}
			break;
		default: // moving
			this->WaitingCycle = 0;
			break;
	}

	if (!unit.Anim.Unbreakable) {
		if (AutoAttack(unit) || AutoRepair(unit) || AutoCast(unit)) {
			this->Finished = true;
		}
	}
}

/**
**  Get goal position
*/
/* virtual */ const Vec2i COrder_Explore::GetGoalPos() const
{
	const Vec2i invalidPos(-1, -1);
	if (goalPos != invalidPos) {
		return goalPos;
	}
	if (this->HasGoal()) {
		return this->GetGoal()->tilePos;
	}
	return invalidPos;
}

//@}
