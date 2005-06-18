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
/**@name action_move.c - The move action. */
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
//      $Id$

//@{

/*----------------------------------------------------------------------------
--  Includes
----------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>

#include "stratagus.h"
#include "video.h"
#include "unittype.h"
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
**  Test if unit can move.
**  For the moment only check for move animation.
**
**  @param unit unit to test if it can move.
**
**  @return 0 if unit cannot move.
*/
int CanMove(const Unit* unit)
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
int DoActionMove(Unit* unit)
{
	int xd;     // X movement in tile.
	int yd;     // Y movement in tile.
	int d;
	int x;      // Unit->X
	int y;      // Unit->Y
	int move;

	Assert(CanMove(unit));
	if (!unit->Moving) {
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
				((WaterOnMap(x, y) && CoastOnMap(x + xd, y + yd)) ||
				(CoastOnMap(x, y) && WaterOnMap(x + xd, y + yd)))) {
			PlayUnitSound(unit, VoiceDocking);
		}

		x = unit->X + xd;
		y = unit->Y + yd;
		MoveUnitToXY(unit, x, y);

		// Remove unit from the current selection
		if (unit->Selected && !IsMapFieldVisible(ThisPlayer, x, y)) {
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
		d = 0;
	}

	move = UnitShowAnimation(unit, unit->Type->Animations->Move);

	unit->IX += xd * move;
	unit->IY += yd * move;

	// Finished move animation, set Moving to 0 so we recalculate the path
	// next frame
	// FIXME: this is broken for subtile movement
	if (!unit->IX && !unit->IY) {
		unit->Moving = 0;
	}

	return d;
}

/**
**  Unit move action:
**
**  Move to a place or to an unit (can move).
**  Tries 10x to reach the target, note this are the complete tries.
**  If the target entered another unit, move to it's position.
**  If the target unit is destroyed, continue to move to it's last position.
**
**  @param unit  Pointer to unit.
*/
void HandleActionMove(Unit* unit)
{
	Unit* goal;

	Assert(unit);
	Assert(CanMove(unit));
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
			if (unit->Orders[0].Range <= TheMap.Info.MapWidth ||
					unit->Orders[0].Range <= TheMap.Info.MapHeight) {
				unit->Orders[0].Range++;
				break;
			}
			// FALL THROUGH
		case PF_REACHED:
			unit->SubAction = 0;
			// Release target, if any.
			if ((goal = unit->Orders[0].Goal)) {
				RefsDecrease(goal);
				unit->Orders[0].Goal = NoUnitP;
			}
			unit->Orders[0].Action = UnitActionStill;
			if (unit->Selected) { // update display for new action
				SelectedUnitChanged();
			}
			return;

		default:
			break;
	}

	//
	// Target destroyed?
	//
	if ((goal = unit->Orders[0].Goal) && goal->Destroyed) {
		DebugPrint("Goal dead\n");
		unit->Orders[0].X = goal->X + goal->Type->TileWidth / 2;
		unit->Orders[0].Y = goal->Y + goal->Type->TileHeight / 2;
		unit->Orders[0].Goal = NoUnitP;
		RefsDecrease(goal);
		NewResetPath(unit);
	}
}

//@}
