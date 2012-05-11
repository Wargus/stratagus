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
/**@name action_move.cpp - The move action. */
//
//      (c) Copyright 1998-2006 by Lutz Sammer and Jimmy Salmon
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

#include "action/action_move.h"

#include "ai.h"
#include "animation.h"
#include "interface.h"
#include "iolib.h"
#include "map.h"
#include "pathfinder.h"
#include "script.h"
#include "sound.h"
#include "ui.h"
#include "unit.h"
#include "unittype.h"

/*----------------------------------------------------------------------------
--  Functions
----------------------------------------------------------------------------*/

/* static */ COrder *COrder::NewActionMove(const Vec2i &pos)
{
	Assert(Map.Info.IsPointOnMap(pos));

	COrder_Move *order = new COrder_Move;

	order->goalPos = pos;
	return order;
}

/* virtual */ void COrder_Move::Save(CFile &file, const CUnit &unit) const
{
	file.printf("{\"action-move\",");

	if (this->Finished) {
		file.printf(" \"finished\", ");
	}
	file.printf(" \"range\", %d,", this->Range);
	file.printf(" \"tile\", {%d, %d}", this->goalPos.x, this->goalPos.y);

	file.printf("}");
}

/* virtual */ bool COrder_Move::ParseSpecificData(lua_State *l, int &j, const char *value, const CUnit &unit)
{
	if (!strcmp(value, "range")) {
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

/* virtual */ bool COrder_Move::IsValid() const
{
	return true;
}

/* virtual */ PixelPos COrder_Move::Show(const CViewport &vp, const PixelPos &lastScreenPos) const
{
	const PixelPos targetPos = vp.TilePosToScreen_Center(this->goalPos);

	Video.FillCircleClip(ColorGreen, lastScreenPos, 2);
	Video.DrawLineClip(ColorGreen, lastScreenPos, targetPos);
	Video.FillCircleClip(ColorGreen, targetPos, 3);
	return targetPos;
}

/* virtual */ void COrder_Move::UpdatePathFinderData(PathFinderInput &input)
{
	input.SetMinRange(0);
	input.SetMaxRange(this->Range);
	const Vec2i tileSize = {0, 0};

	input.SetGoal(this->goalPos, tileSize);
}

/**
**  Unit moves! Generic function called from other actions.
**
**  @param unit  Pointer to unit.
**
**  @return      >0 remaining path length, 0 wait for path, -1
**               reached goal, -2 can't reach the goal.
*/
int DoActionMove(CUnit &unit)
{
	Vec2i posd; // movement in tile.
	int d;
	Vec2i pos;

	Assert(unit.CanMove());

	if (!unit.Moving && (unit.Type->Animations->Move != unit.Anim.CurrAnim || !unit.Anim.Wait)) {
		Assert(!unit.Anim.Unbreakable);

		// FIXME: So units flying up and down are not affected.
		unit.IX = 0;
		unit.IY = 0;

		UnmarkUnitFieldFlags(unit);
		d = NextPathElement(unit, &posd.x, &posd.y);
		MarkUnitFieldFlags(unit);
		switch (d) {
			case PF_UNREACHABLE: // Can't reach, stop
				if (unit.Player->AiEnabled) {
					AiCanNotMove(unit);
				}
				unit.Moving = 0;
				return d;
			case PF_REACHED: // Reached goal, stop
				unit.Moving = 0;
				return d;
			case PF_WAIT: // No path, wait
				// Reset frame to still frame while we wait
				// FIXME: Unit doesn't animate.
				unit.Frame = unit.Type->StillFrame;
				UnitUpdateHeading(unit);
				unit.Wait = 10;

				unit.Moving = 0;
				return d;
			default: // On the way moving
				unit.Moving = 1;
				break;
		}
		pos = unit.tilePos;
		int off = unit.Offset;
		//
		// Transporter (un)docking?
		//
		// FIXME: This is an ugly hack
		if (unit.Type->CanTransport()
			&& ((Map.WaterOnMap(off) && Map.CoastOnMap(pos + posd))
				|| (Map.CoastOnMap(off) && Map.WaterOnMap(pos + posd)))) {
			PlayUnitSound(unit, VoiceDocking);
		}

		pos = unit.tilePos + posd;
		unit.MoveToXY(pos);

		// Remove unit from the current selection
		if (unit.Selected && !Map.IsFieldVisible(*ThisPlayer, pos)) {
			if (NumSelected == 1) { //  Remove building cursor
				CancelBuildingMode();
			}
			if (!ReplayRevealMap) {
				UnSelectUnit(unit);
				SelectionChanged();
			}
		}

		unit.IX = -posd.x * PixelTileSize.x;
		unit.IY = -posd.y * PixelTileSize.y;
		unit.Frame = unit.Type->StillFrame;
		UnitHeadingFromDeltaXY(unit, posd);
	} else {
		posd.x = Heading2X[unit.Direction / NextDirection];
		posd.y = Heading2Y[unit.Direction / NextDirection];
		d = unit.pathFinderData->output.Length + 1;
	}

	unit.pathFinderData->output.Cycles++;//reset have to be manualy controled by caller.
	int move = UnitShowAnimationScaled(unit, unit.Type->Animations->Move, Map.Field(unit.Offset)->Cost);

	unit.IX += posd.x * move;
	unit.IY += posd.y * move;

	// Finished move animation, set Moving to 0 so we recalculate the path
	// next frame
	// FIXME: this is broken for subtile movement
	if (!unit.Anim.Unbreakable && !unit.IX && !unit.IY) {
		unit.Moving = 0;
	}
	return d;
}


/* virtual */ void COrder_Move::Execute(CUnit &unit)
{
	Assert(unit.CanMove());

	if (unit.Wait) {
		unit.Wait--;
		return ;
	}
	// FIXME: (mr-russ) Make a reachable goal here with GoalReachable ...

	switch (DoActionMove(unit)) { // reached end-point?
		case PF_UNREACHABLE:
			// Some tries to reach the goal
			this->Range++;
			break;

		case PF_REACHED:
			this->Finished = true;
			break;
		default:
			break;
	}
}

//@}
