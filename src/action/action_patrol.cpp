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
/**@name action_patrol.cpp - The patrol action. */
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

#include "stratagus.h"

#include "action/action_patrol.h"

#include "iolib.h"
#include "map.h"
#include "pathfinder.h"
#include "script.h"
#include "ui.h"
#include "unit.h"
#include "unittype.h"
#include "video.h"

/*----------------------------------------------------------------------------
--  Functions
----------------------------------------------------------------------------*/

/* static */ COrder *COrder::NewActionPatrol(const Vec2i &currentPos, const Vec2i &dest)
{
	Assert(Map.Info.IsPointOnMap(currentPos));
	Assert(Map.Info.IsPointOnMap(dest));

	COrder_Patrol *order = new COrder_Patrol();

	order->goalPos = dest;
	order->WayPoint = currentPos;
	return order;
}


/* virtual */ void COrder_Patrol::Save(CFile &file, const CUnit &unit) const
{
	file.printf("{\"action-patrol\",");

	if (this->Finished) {
		file.printf(" \"finished\", ");
	}
	file.printf(" \"tile\", {%d, %d},", this->goalPos.x, this->goalPos.y);
	file.printf(" \"range\", %d,", this->Range);

	if (this->WaitingCycle != 0) {
		file.printf(" \"waiting-cycle\", %d,", this->WaitingCycle);
	}
	file.printf(" \"patrol\", {%d, %d}", this->WayPoint.x, this->WayPoint.y);
	file.printf("}");
}

/* virtual */ bool COrder_Patrol::ParseSpecificData(lua_State *l, int &j, const char *value, const CUnit &unit)
{
	if (!strcmp(value, "patrol")) {
		++j;
		lua_rawgeti(l, -1, j + 1);
		CclGetPos(l, &this->WayPoint.x , &this->WayPoint.y);
		lua_pop(l, 1);
	} else if (!strcmp(value, "waiting-cycle")) {
		++j;
		lua_rawgeti(l, -1, j + 1);
		this->WaitingCycle = LuaToNumber(l, -1);
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

/* virtual */ bool COrder_Patrol::IsValid() const
{
	return true;
}

/* virtual */ PixelPos COrder_Patrol::Show(const CViewport &vp, const PixelPos &lastScreenPos) const
{
	const PixelPos pos1 = vp.TilePosToScreen_Center(this->goalPos);
	const PixelPos pos2 = vp.TilePosToScreen_Center(this->WayPoint);

	Video.DrawLineClip(ColorGreen, lastScreenPos, pos1);
	Video.FillCircleClip(ColorBlue, pos1, 2);
	Video.DrawLineClip(ColorBlue, pos1, pos2);
	Video.FillCircleClip(ColorBlue, pos2, 3);
	return pos2;
}

/* virtual */ void COrder_Patrol::UpdatePathFinderData(PathFinderInput &input)
{
	input.SetMinRange(0);
	input.SetMaxRange(this->Range);
	const Vec2i tileSize(0, 0);
	input.SetGoal(this->goalPos, tileSize);
}


/* virtual */ void COrder_Patrol::Execute(CUnit &unit)
{
	if (unit.Wait) {
		unit.Wait--;
		return ;
	}

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
			this->WaitingCycle = 1;
			this->Range = 0;
			std::swap(this->WayPoint, this->goalPos);

			break;
		case PF_WAIT:
			// Wait for a while then give up
			this->WaitingCycle++;
			if (this->WaitingCycle == 5) {
				this->WaitingCycle = 0;
				this->Range = 0;
				std::swap(this->WayPoint, this->goalPos);

				unit.pathFinderData->output.Cycles = 0; //moving counter
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

//@}
