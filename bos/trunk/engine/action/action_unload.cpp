//     ____                _       __               
//    / __ )____  _____   | |     / /___ ___________
//   / __  / __ \/ ___/   | | /| / / __ `/ ___/ ___/
//  / /_/ / /_/ (__  )    | |/ |/ / /_/ / /  (__  ) 
// /_____/\____/____/     |__/|__/\__,_/_/  /____/  
//                                              
//       A futuristic real-time strategy game.
//          This file is part of Bos Wars.
//
/**@name action_unload.cpp - The unload action. */
//
//      (c) Copyright 1998-2007 by Lutz Sammer and Jimmy Salmon
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


// Flag for searching a valid tileset for unloading
#define LandUnitMask ( \
	MapFieldLandUnit | \
	MapFieldBuilding | \
	MapFieldCoastAllowed | \
	MapFieldShallowWater | \
	MapFieldDeepWater | \
	MapFieldUnpassable)

#define NavalUnitMask ( \
	MapFieldLandUnit | \
	MapFieldBuilding | \
	MapFieldCoastAllowed | \
	MapFieldLandAllowed | \
	MapFieldUnpassable)


/**
**  Find a free position close to x, y
**
**  @param x     Original x search position
**  @param y     Original y search position
**  @param resx  Unload x position.
**  @param resy  Unload y position.
**  @param mask  Movement mask for the unit to be droped.
**
**  @return      True if a position was found, False otherwise.
**  @note        resx and resy are undefined if a position is not found.
**
**  @bug         FIXME: Place unit only on fields reachable from the transporter
**  @bug         FIXME: This function fails for units larger than 1x1.
*/
static int FindUnloadPosition(int x, int y, int *resx, int *resy, int mask)
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
			if (CheckedCanMoveToMask(x, y, mask)) {
				*resx = x;
				*resy = y;
				return 1;
			}
		}
		++addx;
		for (i = addx; i--; ++x) {
			if (CheckedCanMoveToMask(x, y, mask)) {
				*resx = x;
				*resy = y;
				return 1;
			}
		}
		++addy;
		for (i = addy; i--; --y) {
			if (CheckedCanMoveToMask(x, y, mask)) {
				*resx = x;
				*resy = y;
				return 1;
			}
		}
		++addx;
		for (i = addx; i--; --x) {
			if (CheckedCanMoveToMask(x, y, mask)) {
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
int UnloadUnit(CUnit *unit)
{
	int x;
	int y;

	Assert(unit->Removed);
	if (!FindUnloadPosition(unit->X, unit->Y, &x, &y, unit->Type->MovementMask)) {
		return 0;
	}
	unit->Boarded = 0;
	unit->Place(x, y);
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
*/
static int ClosestFreeCoast(int x, int y, int *resx, int *resy)
{
	int i;
	int addx;
	int addy;
	int nullx;
	int nully;
	int n;

	addx = addy = 1;
	if (Map.CoastOnMap(x, y) &&
			FindUnloadPosition(x, y, &nullx, &nully, LandUnitMask)) {
		*resx = x;
		*resy = y;
		return 1;
	}
	--x;
	// The maximum distance to the coast. We have to stop somewhere...
	n = 20;
	while (n--) {
		for (i = addy; i--; ++y) {
			if (x >= 0 && y >= 0 && x < Map.Info.MapWidth && y < Map.Info.MapHeight &&
					Map.CoastOnMap(x, y) && !UnitOnMapTile(x, y) &&
					FindUnloadPosition(x, y, &nullx, &nully, LandUnitMask)) {
				*resx = x;
				*resy = y;
				return 1;
			}
		}
		++addx;
		for (i = addx; i--; ++x) {
			if (x >= 0 && y >= 0 && x < Map.Info.MapWidth && y < Map.Info.MapHeight &&
					Map.CoastOnMap(x, y) && !UnitOnMapTile(x ,y) &&
					FindUnloadPosition(x, y, &nullx, &nully, LandUnitMask)) {
				*resx = x;
				*resy = y;
				return 1;
			}
		}
		++addy;
		for (i = addy; i--; --y) {
			if (x >= 0 && y >= 0 && x < Map.Info.MapWidth && y < Map.Info.MapHeight &&
					Map.CoastOnMap(x, y) && !UnitOnMapTile(x, y) &&
					FindUnloadPosition(x, y, &nullx, &nully, LandUnitMask)) {
				*resx = x;
				*resy = y;
				return 1;
			}
		}
		++addx;
		for (i = addx; i--; --x) {
			if (x >= 0 && y >= 0 && x < Map.Info.MapWidth && y < Map.Info.MapHeight &&
					Map.CoastOnMap(x, y) && !UnitOnMapTile(x, y) &&
					FindUnloadPosition(x, y, &nullx, &nully, LandUnitMask)) {
				*resx = x;
				*resy = y;
				return 1;
			}
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
static int ClosestFreeDropZone(CUnit *transporter, int x, int y, int *resx, int *resy)
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
			return ClosestFreeCoast(x, y, resx, resy);
		case UnitTypeNaval:
			// Same ( but reversed... )
			return ClosestFreeCoast(x, y, resx, resy);
		case UnitTypeFly:
			// Here we have loadedType in [ UnitTypeLand,UnitTypeNaval ]
			if (loadedType == UnitTypeLand) {
				return FindUnloadPosition(x, y, resx, resy, LandUnitMask);
			} else {
				return FindUnloadPosition(x, y, resx, resy, NavalUnitMask);
			}
	}
	// Just to avoid a warning
	return 0;
}

/**
**  Move to dropzone.
**
**  @param unit  Pointer to unit.
**
**  @return      -1 if unreachable, True if reached, False otherwise.
*/
static int MoveToDropZone(CUnit *unit)
{
	switch (DoActionMove(unit)) { // reached end-point?
		case PF_UNREACHABLE:
			return -1;
		case PF_REACHED:
			break;
		default:
			return 0;
	}

	Assert(unit->Orders[0]->Action == UnitActionUnload);
	return 1;
}

/**
**  Make one or more unit leave the transporter.
**
**  @param unit  Pointer to unit.
*/
static void LeaveTransporter(CUnit *unit)
{
	int i;
	int stillonboard;
	CUnit *goal;

	stillonboard = 0;
	goal = unit->Orders[0]->Goal;
	//
	// Goal is the specific unit unit that you want to unload.
	// This can be NULL, in case you want to unload everything.
	//
	if (goal) {
		unit->Orders[0]->Goal = NoUnitP;
		if (goal->Destroyed) {
			DebugPrint("destroyed unit unloading?\n");
			goal->RefsDecrease();
			return;
		}
		goal->RefsDecrease();
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
		unit->Orders[0]->Action = UnitActionUnload;
		unit->Orders[0]->Goal = NoUnitP;
		unit->Orders[0]->X = unit->X;
		unit->Orders[0]->Y = unit->Y;
		unit->SubAction = 0;
	} else {
		unit->ClearAction();
	}
}

/**
**  The transporter unloads a unit.
**
**  @param unit  Pointer to unit.
*/
void HandleActionUnload(CUnit *unit)
{
	int i;
	int x;
	int y;

	if (!CanMove(unit)) {
		unit->SubAction = 2;
	}
	switch (unit->SubAction) {
		//
		// Move the transporter
		//
		case 0:
			if (!unit->Orders[0]->Goal) {
				if (!ClosestFreeDropZone(unit, unit->Orders[0]->X, unit->Orders[0]->Y,
						&x, &y)) {
					// Sorry... I give up.
					unit->ClearAction();
					return;
				}
				unit->Orders[0]->X = x;
				unit->Orders[0]->Y = y;
			}

			NewResetPath(unit);
			unit->SubAction = 1;
		case 1:
			// The Goal is the unit that we have to unload.
			if (!unit->Orders[0]->Goal) {
				// We have to unload everything
				if ((i = MoveToDropZone(unit))) {
					if (i == PF_REACHED) {
						if (++unit->SubAction == 1) {
							unit->ClearAction();
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
			if (CanMove(unit) && unit->Orders[0]->Action != UnitActionStill) {
				HandleActionUnload(unit);
			}
			break;
	}
}

//@}
