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
/**@name action_unload.cpp - The unload action. */
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
	MapFieldWall | \
	MapFieldRocks | \
	MapFieldForest | \
	MapFieldCoastAllowed | \
	MapFieldWaterAllowed | \
	MapFieldUnpassable)

#define NavalUnitMask ( \
	MapFieldLandUnit | \
	MapFieldBuilding | \
	MapFieldWall | \
	MapFieldRocks | \
	MapFieldForest | \
	MapFieldCoastAllowed | \
	MapFieldLandAllowed | \
	MapFieldUnpassable)


/**
**  Find a free position close to startPos
**
**  @param startPos     Original search position
**  @param res  Unload position.
**  @param mask  Movement mask for the unit to be droped.
**
**  @return      True if a position was found, False otherwise.
**  @note        resx and resy are undefined if a position is not found.
**
**  @bug         FIXME: Place unit only on fields reachable from the transporter
**  @bug         FIXME: This function fails for units larger than 1x1.
*/
static int FindUnloadPosition(const Vec2i startPos, Vec2i *res, int mask)
{
	int i;
	int n;
	int addx;
	int addy;
	Vec2i pos = startPos;
	addx = addy = 1;
	--pos.x;
	for (n = 0; n < 2; ++n) {
		// Nobody: There was some code here to check for unloading units that can
		// only go on even tiles. It's useless, since we can only unload land units.
		for (i = addy; i--; ++pos.y) {
			if (CheckedCanMoveToMask(pos, mask)) {
				*res = pos;
				return 1;
			}
		}
		++addx;
		for (i = addx; i--; ++pos.x) {
			if (CheckedCanMoveToMask(pos, mask)) {
				*res = pos;
				return 1;
			}
		}
		++addy;
		for (i = addy; i--; --pos.y) {
			if (CheckedCanMoveToMask(pos, mask)) {
				*res = pos;
				return 1;
			}
		}
		++addx;
		for (i = addx; i--; --pos.x) {
			if (CheckedCanMoveToMask(pos, mask)) {
				*res = pos;
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
int UnloadUnit(CUnit &unit)
{
	Vec2i pos;

	Assert(unit.Removed);
	if (!FindUnloadPosition(unit.tilePos, &pos, unit.Type->MovementMask)) {
		return 0;
	}
	unit.Boarded = 0;
	unit.Place(pos.x, pos.y);
	return 1;
}

/**
**  Find the closest piece of coast you can unload units on
**
**  @param  x     start location for the search
**  @param  y     start location for the search
**  @param  resPos  coast position
**
**  @return       1 if a location was found, 0 otherwise
*/
static int ClosestFreeCoast(const Vec2i &startPos, Vec2i *resPos)
{
	int i;
	int addx;
	int addy;
	Vec2i nullpos;
	Vec2i pos = startPos;
	int n;

	addx = addy = 1;
	if (Map.CoastOnMap(pos) &&
			FindUnloadPosition(pos, &nullpos, LandUnitMask)) {
		*resPos = pos;
		return 1;
	}
	--pos.x;
	// The maximum distance to the coast. We have to stop somewhere...
	n = 20;
	while (n--) {
		for (i = addy; i--; ++pos.y) {
			if (Map.Info.IsPointOnMap(pos.x, pos.y) &&
					Map.CoastOnMap(pos) && !UnitOnMapTile(pos.x, pos.y, -1) &&
					FindUnloadPosition(pos, &nullpos, LandUnitMask)) {
				*resPos = pos;
				return 1;
			}
		}
		++addx;
		for (i = addx; i--; ++pos.x) {
			if (Map.Info.IsPointOnMap(pos.x, pos.y) &&
					Map.CoastOnMap(pos) && !UnitOnMapTile(pos.x, pos.y, -1) &&
					FindUnloadPosition(pos, &nullpos, LandUnitMask)) {
				*resPos = pos;
				return 1;
			}
		}
		++addy;
		for (i = addy; i--; --pos.y) {
			if (Map.Info.IsPointOnMap(pos.x, pos.y) &&
					Map.CoastOnMap(pos) && !UnitOnMapTile(pos.x, pos.y, -1) &&
					FindUnloadPosition(pos, &nullpos, LandUnitMask)) {
				*resPos = pos;
				return 1;
			}
		}
		++addx;
		for (i = addx; i--; --pos.x) {
			if (Map.Info.IsPointOnMap(pos.x, pos.y) &&
					Map.CoastOnMap(pos) && !UnitOnMapTile(pos.x, pos.y, -1) &&
					FindUnloadPosition(pos, &nullpos, LandUnitMask)) {
				*resPos = pos;
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
**  @param  startPos     start location for the search
**  @param  resPos       coast position
**
**  @return              1 if a location was found, 0 otherwise
**
*/
static int ClosestFreeDropZone(CUnit &transporter, const Vec2i& startPos, Vec2i *resPos)
{
	// Type (land/fly/naval) of the transporter
	int transporterType;
	// Type (land/fly/naval) of the units to unload
	int loadedType;

	// Check there are units onboard
	if (!transporter.UnitInside) {
		return 0;
	}

	transporterType = transporter.Type->UnitType;
	// Take the type of the onboard unit
	loadedType = transporter.UnitInside->Type->UnitType;

	// Don't move in thoses cases
	if ((transporterType == loadedType) || (loadedType == UnitTypeFly)) {
		*resPos = startPos;
		return 1;
	}

	switch (transporterType) {
		case UnitTypeLand:
			// in this case, loadedType == UnitTypeSea
			return ClosestFreeCoast(startPos, resPos);
		case UnitTypeNaval:
			// Same ( but reversed... )
			return ClosestFreeCoast(startPos, resPos);
		case UnitTypeFly:
			// Here we have loadedType in [ UnitTypeLand,UnitTypeNaval ]
			if (loadedType == UnitTypeLand) {
				return FindUnloadPosition(startPos, resPos, LandUnitMask);
			} else {
				return FindUnloadPosition(startPos, resPos, NavalUnitMask);
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
static int MoveToDropZone(CUnit &unit)
{
	switch (DoActionMove(unit)) { // reached end-point?
		case PF_UNREACHABLE:
			return -1;
		case PF_REACHED:
			break;
		default:
			return 0;
	}

	Assert(unit.CurrentAction() == UnitActionUnload);
	return 1;
}

/**
**  Make one or more unit leave the transporter.
**
**  @param unit  Pointer to unit.
*/
static void LeaveTransporter(CUnit &unit)
{
	int i;
	int stillonboard;
	CUnit *goal;

	stillonboard = 0;
	goal = unit.CurrentOrder()->GetGoal();
	//
	// Goal is the specific unit unit that you want to unload.
	// This can be NULL, in case you want to unload everything.
	//
	if (goal) {
		if (goal->Destroyed) {
			DebugPrint("destroyed unit unloading?\n");
			unit.CurrentOrder()->ClearGoal();
			return;
		}
		unit.CurrentOrder()->ClearGoal();
		goal->tilePos = unit.tilePos;
		// Try to unload the unit. If it doesn't work there is no problem.
		if (UnloadUnit(*goal)) {
			unit.BoardCount--;
		}
	} else {
		// Unload all units.
		goal = unit.UnitInside;
		for (i = unit.InsideCount; i; --i, goal = goal->NextContained) {
			if (goal->Boarded) {
				goal->tilePos = unit.tilePos;
				if (!UnloadUnit(*goal)) {
					++stillonboard;
				} else {
					unit.BoardCount--;
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
		unit.CurrentOrder()->Action = UnitActionUnload;
		unit.CurrentOrder()->ClearGoal();
		unit.CurrentOrder()->goalPos = unit.tilePos;
		unit.SubAction = 0;
	} else {
		unit.ClearAction();
	}
}

/**
**  The transporter unloads a unit.
**
**  @param unit  Pointer to unit.
*/
void HandleActionUnload(CUnit &unit)
{
	int i;
	Vec2i pos;

	if (!unit.CanMove()) {
		unit.SubAction = 2;
	}
	switch (unit.SubAction) {
		//
		// Move the transporter
		//
		case 0:
			if (!unit.CurrentOrder()->HasGoal()) {
				if (!ClosestFreeDropZone(unit, unit.CurrentOrder()->goalPos, &pos)) {
					// Sorry... I give up.
					unit.ClearAction();
					return;
				}
				unit.CurrentOrder()->goalPos = pos;
			}

			NewResetPath(unit);
			unit.SubAction = 1;
		case 1:
			// The Goal is the unit that we have to unload.
			if (!unit.CurrentOrder()->HasGoal()) {
				// We have to unload everything
				if ((i = MoveToDropZone(unit))) {
					if (i == PF_REACHED) {
						if (++unit.SubAction == 1) {
							unit.ClearAction();
						}
					} else {
						unit.SubAction = 2;
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
			if (unit.CanMove() && unit.CurrentAction() != UnitActionStill) {
				HandleActionUnload(unit);
			}
			break;
	}
}

//@}
