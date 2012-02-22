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

#include <stdio.h>
#include <stdlib.h>

#include "stratagus.h"
#include "unit.h"
#include "unittype.h"
#include "actions.h"
#include "pathfinder.h"
#include "map.h"
#include "iolib.h"
#include "script.h"

/*----------------------------------------------------------------------------
--  Functions
----------------------------------------------------------------------------*/


/* virtual */ void COrder_Patrol::Save(CFile &file, const CUnit &unit) const
{
	file.printf("{\"action-patrol\",");

	file.printf(" \"tile\", {%d, %d},", this->goalPos.x, this->goalPos.y);
	file.printf(" \"range\", %d,", this->Range);

	file.printf(" \"patrol\", {%d, %d},\n  ", this->WayPoint.x, this->WayPoint.y);
	SaveDataMove(file);
	file.printf("}");
}

/* virtual */ bool COrder_Patrol::ParseSpecificData(lua_State *l, int &j, const char *value, const CUnit &unit)
{
	if (ParseMoveData(l, j, value)) {
		return true;
	} else if (!strcmp(value, "patrol")) {
		++j;
		lua_rawgeti(l, -1, j + 1);
		CclGetPos(l, &this->WayPoint.x , &this->WayPoint.y);
		lua_pop(l, 1);
	} else {
		return false;
	}
	return true;
}

/* virtual */ bool COrder_Patrol::Execute(CUnit &unit)
{
	if (unit.Wait) {
		unit.Wait--;
		return false;
	}

	if (!unit.SubAction) { // first entry.
		this->Data.Move.Cycles = 0; //moving counter
		this->NewResetPath();
		unit.SubAction = 1;
	}

	switch (DoActionMove(unit)) {
		case PF_FAILED:
			unit.SubAction = 1;
			break;
		case PF_UNREACHABLE:
			// Increase range and try again
			unit.SubAction = 1;
			if (this->CheckRange()) {
				this->Range++;
				break;
			}
			// FALL THROUGH
		case PF_REACHED:
			unit.SubAction = 1;
			this->Range = 0;
			std::swap(this->WayPoint, this->goalPos);

			this->Data.Move.Cycles = 0; //moving counter
			this->NewResetPath();
			break;
		case PF_WAIT:
			// Wait for a while then give up
			unit.SubAction++;
			if (unit.SubAction == 5) {
				unit.SubAction = 1;
				this->Range = 0;
				std::swap(this->WayPoint, this->goalPos);

				this->Data.Move.Cycles = 0; //moving counter
				this->NewResetPath();
			}
			break;
		default: // moving
			unit.SubAction = 1;
			break;
	}

	if (!unit.Anim.Unbreakable) {
		if (AutoAttack(unit, false) || AutoRepair(unit) || AutoCast(unit)) {
			return true;
		}
	}
	return false;
}

/**
**  Unit Patrol:
**    The unit patrols between two points.
**    Any enemy unit in reaction range is attacked.
**  @todo FIXME:
**    Should do some tries to reach the end-points.
**    Should support patrol between more points!
**    Patrol between units.
**
**  @param unit  Patroling unit pointer.
*/
void HandleActionPatrol(COrder& order, CUnit &unit)
{
	Assert(order.Action == UnitActionPatrol);

	if (order.Execute(unit)) {
		unit.ClearAction();
	}
}

//@}
