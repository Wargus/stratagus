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
/**@name animation_rotate.cpp - The animation Rotate. */
//
//      (c) Copyright 2012 by Joris Dauphin
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

#include "animation/animation_rotate.h"

#include "actions.h"
#include "action/action_build.h"
#include "tile.h"
#include "unit.h"

/**
**  Rotate a unit
**
**  @param unit    Unit to rotate
**  @param rotate  Number of frames to rotate (>0 clockwise, <0 counterclockwise)
*/
void UnitRotate(CUnit &unit, int rotate)
{
	unit.Direction += rotate * 256 / unit.Type->NumDirections;
	UnitUpdateHeading(unit);
}

/* virtual */ void CAnimation_Rotate::Action(CUnit &unit, int &/*move*/, int /*scale*/) const
{
	Assert(unit.Anim.Anim == this);

	if (this->rotateStr == "target") {
		COrder *order = unit.CurrentOrder();
		CUnit *target;
		if (order->HasGoal()) {
			target = order->GetGoal();
			if (target->Destroyed) {
				order->ClearGoal();
				return;	
			}
		} else if (unit.CurrentOrder()->Action == UnitAction::Build) {
			target = static_cast<const COrder_Build *>(order)->GetBuildingUnit();
		}
		Vec2i dpos = target->tilePos + target->Type->GetHalfTileSize() - unit.tilePos;
		Vec2i doff = Vec2i(target->IX / 2, target->IY / 2) - Vec2i(unit.IX, unit.IY);
		dpos.x += doff.x / PixelTileSize.x;
		dpos.y += doff.y / PixelTileSize.y;
		UnitHeadingFromDeltaXY(unit, dpos);
	} else {
		UnitRotate(unit, ParseAnimInt(unit, this->rotateStr.c_str()));
	}
}

/* virtual */ void CAnimation_Rotate::Init(const char *s, lua_State *)
{
	this->rotateStr = s;
}

//@}
