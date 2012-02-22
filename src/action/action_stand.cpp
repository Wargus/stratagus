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
/**@name action_stand.cpp - The stand ground action. */
//
//      (c) Copyright 2000-2006 by Lutz Sammer and Jimmy Salmon
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

#include "stratagus.h"
#include "unit.h"
#include "actions.h"
#include "iolib.h"

/*----------------------------------------------------------------------------
--  Functions
----------------------------------------------------------------------------*/


/* virtual */ void COrder_StandGround::Save(CFile &file, const CUnit &unit) const
{
	file.printf("{\"action-stand-ground\"");
	if (this->HasGoal()) {
		CUnit &goal = *this->GetGoal();
		if (goal.Destroyed) {
			/* this unit is destroyed so it's not in the global unit
			 * array - this means it won't be saved!!! */
			printf ("FIXME: storing destroyed Goal - loading will fail.\n");
		}
		file.printf(", \"goal\", \"%s\"", UnitReference(goal).c_str());
	}
	file.printf("}");
}

/* virtual */ bool COrder_StandGround::ParseSpecificData(lua_State *l, int &j, const char *value, const CUnit &unit)
{
	return false;
}

/* virtual */ bool COrder_StandGround::Execute(CUnit &unit)
{
	ActionStillGeneric(unit, true);

	return false;
}


/**
**  Unit stands ground!
**
**  @param unit  Action handled for this unit pointer.
*/
void HandleActionStandGround(COrder& order, CUnit &unit)
{
	Assert(order.Action == UnitActionStandGround);

	if (order.Execute(unit)) {
		unit.ClearAction();
	}
}

//@}
