//     ____                _       __               
//    / __ )____  _____   | |     / /___ ___________
//   / __  / __ \/ ___/   | | /| / / __ `/ ___/ ___/
//  / /_/ / /_/ (__  )    | |/ |/ / /_/ / /  (__  ) 
// /_____/\____/____/     |__/|__/\__,_/_/  /____/  
//                                              
//       A futuristic real-time strategy game.
//          This file is part of Bos Wars.
//
/**@name action_move.cpp - The move action. */
//
//      (c) Copyright 1998-2008 by Lutz Sammer and Jimmy Salmon
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
**  Test if unit can move.
**  For the moment only check for move animation.
**
**  @param unit unit to test if it can move.
**
**  @return 0 if unit cannot move.
*/
bool CanMove(const CUnit *unit)
{
	Assert(unit);
	Assert(unit->Type);
	return unit->Type->Animations && unit->Type->Animations->Move;
}


/**
**  Unit moves! Generic function called from other actions.
**
**  @param unit  Pointer to unit.
**
**  @return      >0 remaining path length, 0 wait for path, -1
**               reached goal, -2 can't reach the goal.
*/
int DoActionMove(CUnit *unit)
{
	int xd;     // X movement in tile.
	int yd;     // Y movement in tile.
	int d;
	int x;      // Unit->X
	int y;      // Unit->Y
	int move;

	Assert(CanMove(unit));
	if (!unit->Moving &&
			(unit->Type->Animations->Move != unit->Anim.CurrAnim || !unit->Anim.Wait)) {
		Assert(!unit->Anim.Unbreakable);

		// FIXME: So units flying up and down are not affected.
		unit->IX = unit->IY = 0;

		UnmarkUnitFieldFlags(unit);
		d = NextPathElement(unit, &xd, &yd);
		MarkUnitFieldFlags(unit);
		switch (d) {
			case PF_UNREACHABLE: // Can't reach, stop
				if (unit->Player->AiEnabled) {
					AiCanNotMove(unit);
				}
				unit->Moving = 0;
				return d;
			case PF_REACHED: // Reached goal, stop
				unit->Moving = 0;
				return d;
			case PF_WAIT: // No path, wait
				// Reset frame to still frame while we wait
				// FIXME: Unit doesn't animate.
				unit->Frame = unit->Type->StillFrame;
				UnitUpdateHeading(unit);
				unit->Wait = 10;

				unit->Moving = 0;
				return d;
			default: // On the way moving
				unit->Moving = 1;
				break;
		}
		x = unit->X;
		y = unit->Y;
		//
		// Transporter (un)docking?
		//
		// FIXME: This is an ugly hack
		if (unit->Type->CanTransport &&
				((Map.WaterOnMap(x, y) && Map.CoastOnMap(x + xd, y + yd)) ||
				(Map.CoastOnMap(x, y) && Map.WaterOnMap(x + xd, y + yd)))) {
			PlayUnitSound(unit, VoiceDocking);
		}

		x = unit->X + xd;
		y = unit->Y + yd;
		unit->MoveToXY(x, y);

		// Remove unit from the current selection
		if (unit->Selected && !Map.IsFieldVisible(ThisPlayer, x, y)) {
			if (NumSelected == 1) { //  Remove building cursor
				CancelBuildingMode();
			}
			if (!ReplayRevealMap) {
				UnSelectUnit(unit);
				SelectionChanged();
			}
		}

		unit->IX = -xd * TileSizeX;
		unit->IY = -yd * TileSizeY;
		unit->Frame = unit->Type->StillFrame;
		UnitHeadingFromDeltaXY(unit, xd, yd);
	} else {
		xd = Heading2X[unit->Direction / NextDirection];
		yd = Heading2Y[unit->Direction / NextDirection];
		d = unit->Data.Move.Length + 1;
	}

	move = UnitShowAnimationScaled(unit, unit->Type->Animations->Move,
		(unit->Type->UnitType == UnitTypeFly)
		? MapFieldNormalCost : Map.Field(unit->X, unit->Y)->Cost);

	unit->IX += xd * move;
	unit->IY += yd * move;

	// Finished move animation, set Moving to 0 so we recalculate the path
	// next frame
	// FIXME: this is broken for subtile movement
	if (!unit->Anim.Unbreakable && !unit->IX && !unit->IY) {
		unit->Moving = 0;
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
void HandleActionMove(CUnit *unit)
{
	CUnit *goal;

	Assert(unit);
	Assert(CanMove(unit));

	if (unit->Wait) {
		unit->Wait--;
		return;
	}

	if (!unit->SubAction) { // first entry
		unit->SubAction = 1;
		NewResetPath(unit);

		Assert(unit->State == 0);
	}

	// FIXME: (mr-russ) Make a reachable goal here with GoalReachable ...

	switch (DoActionMove(unit)) { // reached end-point?
		case PF_UNREACHABLE:
			//
			// Some tries to reach the goal
			//
			if (unit->Orders[0]->Range <= Map.Info.MapWidth ||
					unit->Orders[0]->Range <= Map.Info.MapHeight) {
				unit->Orders[0]->Range++;
				break;
			}
			// FALL THROUGH
		case PF_REACHED:
			// Release target, if any.
			if ((goal = unit->Orders[0]->Goal)) {
				goal->RefsDecrease();
				unit->Orders[0]->Goal = NoUnitP;
			}
			unit->ClearAction();
			return;

		default:
			break;
	}

	//
	// Target destroyed?
	//
	if ((goal = unit->Orders[0]->Goal) && goal->Destroyed) {
		DebugPrint("Goal dead\n");
		unit->Orders[0]->X = goal->X + goal->Type->TileWidth / 2;
		unit->Orders[0]->Y = goal->Y + goal->Type->TileHeight / 2;
		unit->Orders[0]->Goal = NoUnitP;
		goal->RefsDecrease();
		NewResetPath(unit);
	}
}

//@}
