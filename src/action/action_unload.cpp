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
/**@name action_unload.c - The unload action. */
//
//      (c) Copyright 1998-2004 by Lutz Sammer
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
#include "unittype.h"
#include "player.h"
#include "unit.h"
#include "actions.h"
#include "map.h"
#include "interface.h"
#include "pathfinder.h"

/*----------------------------------------------------------------------------
--  Functions
----------------------------------------------------------------------------*/

/**
**  Find a free position close to x, y for unit.
**
**  @unit        Unit to unload.
**  @param x     Original x search position
**  @param y     Original y search position
**  @param resx  Unload x position.
**  @param resy  Unload y position.
**
**  @return      True if a position was found, False otherwise.
**  @note        resx and resy are undefined if a position is not found.
**
**  @bug         FIXME: Place unit only on fields reachable from the transporter
*/
static int FindUnloadPositionForUnit(const Unit* unit, int x, int y, int* resx, int* resy)
{
	int i;
	int n;
	int addx;
	int addy;

	addx = addy = 1;
	--x;
	for (n = 0; n < 2; ++n) {
		// Nobody: There was some code here to check for unloading units that can
		// only go on even tiles. It's useless, since we can only unload land units.
		for (i = addy; i--; ++y) {
			if (UnitCanMoveTo(unit, x, y)) {
				*resx = x;
				*resy = y;
				return 1;
			}
		}
		++addx;
		for (i = addx; i--; ++x) {
			if (UnitCanMoveTo(unit, x, y)) {
				*resx = x;
				*resy = y;
				return 1;
			}
		}
		++addy;
		for (i = addy; i--; --y) {
			if (UnitCanMoveTo(unit, x, y)) {
				*resx = x;
				*resy = y;
				return 1;
			}
		}
		++addx;
		for (i = addx; i--; --x) {
			if (UnitCanMoveTo(unit, x, y)) {
				*resx = x;
				*resy = y;
				return 1;
			}
		}
		++addy;
	}
	return 0;
}


/**
**  Reappear unit on map.
**
**  @param unit  Unit to drop out.
**
**  @return      True if unit can be unloaded.
**
**  @bug         FIXME: Place unit only on fields reachable from the transporter
*/
int UnloadUnit(Unit* unit)
{
	int x;
	int y;

	Assert(unit->Removed);
	if (!FindUnloadPositionForUnit(unit, unit->X, unit->Y, &x, &y)) {
		return 0;
	}
	unit->Wait = 1; // should be correct unit has still action
	unit->Boarded = 0;
	PlaceUnit(unit, x, y);
	return 1;
}

/**
**  Find the closest piece of coast you can unload units on
**
**  @param  x     start location for the search
**  @param  y     start location for the search
**  @param  resx  coast x position
**  @param  resy  coast y position
**
**  @return       1 if a location was found, 0 otherwise
**  @todo         Only know area.
*/
static int ClosestFreeCoast(const Unit* unit, int x, int y, int* resx, int* resy, int mask)
{
	int i;
	int addx;
	int addy;
	int nullx;
	int nully;
	int n;

	addx = addy = 1;
	if ((~TheMap.Fields[x + y * TheMap.Width].Flags & mask) &&
			FindUnloadPositionForUnit(unit, x, y, &nullx, &nully)) {
		*resx = x;
		*resy = y;
		return 1;
	}
#ifdef shortalias
#warning // rename this define.
#endif
#define shortalias \
	if (x >= 0 && y >= 0 && x < TheMap.Width && y < TheMap.Height && \
	(~TheMap.Fields[x + y * TheMap.Width].Flags & mask) && !UnitOnMapTile(x, y) && \
		FindUnloadPositionForUnit(unit, x, y, &nullx, &nully)) { \
		*resx = x; \
		*resy = y; \
		return 1; \
	}

	--x;
	// The maximum distance to the coast. We have to stop somewhere...
	n = 20;
	while (n--) {
		for (i = addy; i--; ++y) {
			shortalias;
		}
		++addx;
		for (i = addx; i--; ++x) {
			shortalias;
		}
		++addy;
		for (i = addy; i--; --y) {
			shortalias;
		}
		++addx;
		for (i = addx; i--; --x) {
			shortalias;
		}
		++addy;
	}
	DebugPrint("Try clicking closer to an actual coast.\n");
	return 0;
}

/**
**  Find the closest available drop zone for a transporter.
**  Fail if transporter don't transport any unit..
**
**  @param  transporter  the transporter
**  @param  x            start location for the search
**  @param  y            start location for the search
**  @param  resx         coast x position
**  @param  resy         coast y position
**
**  @return              1 if a location was found, 0 otherwise
**
*/
static int ClosestFreeDropZone(Unit* transporter, int x, int y, int* resx, int* resy)
{
	// Type (land/fly/naval) of the transporter
	int transporterType;
	// Type (land/fly/naval) of the units to unload
	int loadedType;

	// Check there are units onboard
	if (!transporter->UnitInside) {
		return 0;
	}

	transporterType = transporter->Type->UnitType;
	// Take the type of the onboard unit
	loadedType = transporter->UnitInside->Type->UnitType;

	// Don't move in thoses cases
	if ((transporterType == loadedType) || (loadedType == UnitTypeFly)) {
		*resx = x;
		*resy = y;
		return 1;
	}

	switch (transporterType) {
		case UnitTypeLand:
			// in this case, loadedType == UnitTypeSea
			return ClosestFreeCoast(transporter->UnitInside, x, y, resx, resy, MapFieldCoastAllowed);
		case UnitTypeNaval:
			// Same ( but reversed... )
			return ClosestFreeCoast(transporter, x, y, resx, resy, MapFieldCoastAllowed);
		case UnitTypeFly:
			// Now, depend of unit inside.
			return ClosestFreeCoast(transporter->UnitInside, x, y, resx, resy,
				MapFieldCoastAllowed | (loadedType == UnitTypeNaval) ?
				MapFieldWaterAllowed : MapFieldLandAllowed);
		default :
			DebugPrint("unknow type for transporter\n");
			abort();
			return 0;
	}
}

/**
**  Move to dropzone.
**
**  @param unit  Pointer to unit.
**
**  @return      -1 if unreachable, True if reached, False otherwise.
*/
static int MoveToDropZone(Unit* unit)
{
	switch (DoActionMove(unit)) { // reached end-point?
		case PF_UNREACHABLE:
			return -1;
		case PF_REACHED:
			break;
		default:
			return 0;
	}

	Assert(unit->Orders[0].Action == UnitActionUnload);
	return 1;
}

/**
**  Make one or more unit leave the transporter.
**
**  @param unit  Pointer to unit.
*/
static void LeaveTransporter(Unit* unit)
{
	int i;
	int stillonboard;
	Unit* goal;

	stillonboard = 0;
	goal = unit->Orders[0].Goal;
	//
	// Goal is the specific unit unit that you want to unload.
	// This can be NULL, in case you want to unload everything.
	//
	if (goal) {
		unit->Orders[0].Goal = NoUnitP;
		if (goal->Destroyed) {
			DebugPrint("destroyed unit unloading?\n");
			RefsDecrease(goal);
			return;
		}
		RefsDecrease(goal);
		goal->X = unit->X;
		goal->Y = unit->Y;
		// Try to unload the unit. If it doesn't work there is no problem.
		if (UnloadUnit(goal)) {
			unit->BoardCount--;
		}
	} else {
		// Unload all units.
		goal = unit->UnitInside;
		for (i = unit->InsideCount; i; --i, goal = goal->NextContained) {
			if (goal->Boarded) {
				goal->X = unit->X;
				goal->Y = unit->Y;
				if (!UnloadUnit(goal)) {
					++stillonboard;
				} else {
					unit->BoardCount--;
				}
			}
		}
	}
	if (IsOnlySelected(unit)) {
		SelectedUnitChanged();
	}

	// We still have some units to unload, find a piece of free coast.
	if (stillonboard) {
		// We tell it to unload at it's current position. This can't be done,
		// so it will search for a piece of free coast nearby.
		unit->Orders[0].Action = UnitActionUnload;
		unit->Orders[0].Goal = NoUnitP;
		unit->Orders[0].X = unit->X;
		unit->Orders[0].Y = unit->Y;
		unit->SubAction = 0;
		unit->Reset = 0;
		unit->Wait = 0;
	} else {
		unit->Wait = 1;
		unit->Orders[0].Action = UnitActionStill;
		unit->SubAction = 0;
	}
}

/**
**  The transporter unloads an unit.
**
**  @param unit  Pointer to unit.
*/
void HandleActionUnload(Unit* unit)
{
	int i;
	int x;
	int y;

	switch (unit->SubAction) {
		//
		// Move the transporter
		//
		case 0:
			if (!unit->Orders[0].Goal) {
				if (!ClosestFreeDropZone(unit,unit->Orders[0].X,unit->Orders[0].Y,
						&x, &y)) {
					// Sorry... I give up.
					unit->Orders[0].Action = UnitActionStill;
					unit->SubAction = 0;
					return;
				}
				unit->Orders[0].X = x;
				unit->Orders[0].Y = y;
			}

			NewResetPath(unit);
			unit->SubAction = 1;
		case 1:
			// The Goal is the unit that we have to unload.
			if (!unit->Orders[0].Goal) {
				// We have to unload everything
				if ((i = MoveToDropZone(unit))) {
					if (i == PF_REACHED) {
						if (++unit->SubAction == 1) {
							unit->Orders[0].Action = UnitActionStill;
							unit->SubAction = 0;
						}
					} else {
						unit->SubAction = 2;
					}
				}
				break;
			}
		//
		// Leave the transporter
		//
		case 2:
			// FIXME: show still animations ?
			LeaveTransporter(unit);
			if (unit->Orders[0].Action != UnitActionStill) {
				HandleActionUnload(unit);
			}
			break;
	}
}

//@}
