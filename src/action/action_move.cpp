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
#include "video.h"
#include "unittype.h"
#include "animation.h"
#include "player.h"
#include "unit.h"
#include "tileset.h"
#include "map.h"
#include "actions.h"
#include "pathfinder.h"
#include "sound.h"
#include "interface.h"
#include "map.h"
#include "ai.h"

/*----------------------------------------------------------------------------
--  Variables
----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
--  Function
----------------------------------------------------------------------------*/

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
	int move;
	int off;

	Assert(unit.CanMove());
	if (!unit.Moving &&
			(unit.Type->Animations->Move != unit.Anim.CurrAnim || !unit.Anim.Wait)) {
		Assert(!unit.Anim.Unbreakable);

		// FIXME: So units flying up and down are not affected.
		unit.IX = unit.IY = 0;

		MapUnmarkUnitGuard(unit);
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
		off = unit.Offset;
		//
		// Transporter (un)docking?
		//
		// FIXME: This is an ugly hack
		if (unit.Type->CanTransport() &&
				((Map.WaterOnMap(off) && Map.CoastOnMap(pos + posd)) ||
				(Map.CoastOnMap(off) && Map.WaterOnMap(pos + posd)))) {
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
		d = unit.CurrentOrder()->Data.Move.Length + 1;
	}

	unit.CurrentOrder()->Data.Move.Cycles++;//reset have to be manualy controled by caller.
	move = UnitShowAnimationScaled(unit, unit.Type->Animations->Move,
			Map.Field(unit.Offset)->Cost);

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

/**
**  Unit move action:
**
**  Move to a place or to a unit (can move).
**  Tries 10x to reach the target, note this are the complete tries.
**  If the target entered another unit, move to it's position.
**  If the target unit is destroyed, continue to move to it's last position.
**
**  @param unit  Pointer to unit.
*/
void HandleActionMove(COrder& order, CUnit &unit)
{
	CUnit *goal;

	Assert(unit.CanMove());

	if (unit.Wait) {
		unit.Wait--;
		return;
	}

	if (!unit.SubAction) { // first entry
		unit.SubAction = 1;
		unit.CurrentOrder()->NewResetPath();
		order.Data.Move.Cycles = 0;
		Assert(unit.State == 0);
	}

	// FIXME: (mr-russ) Make a reachable goal here with GoalReachable ...

	switch (DoActionMove(unit)) { // reached end-point?
		case PF_UNREACHABLE:
			//
			// Some tries to reach the goal
			//
			if (order.CheckRange()) {
				order.Range++;
				break;
			}
			// FALL THROUGH
		case PF_REACHED:
			// Release target, if any.
			order.ClearGoal();
			unit.ClearAction();
			return;

		default:
			break;
	}

	//
	// Target destroyed?
	//
	goal = order.GetGoal();
	if (goal && goal->Destroyed) {
		DebugPrint("Goal dead\n");
		order.goalPos = goal->tilePos + goal->Type->GetHalfTileSize();
		order.ClearGoal();
		order.NewResetPath();
	}
}

//@}
