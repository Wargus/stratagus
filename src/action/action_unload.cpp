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

//@{

/*----------------------------------------------------------------------------
--  Includes
----------------------------------------------------------------------------*/

#include "stratagus.h"

#include "action/action_unload.h"

#include "iolib.h"
#include "map.h"
#include "pathfinder.h"
#include "player.h"
#include "script.h"
#include "ui.h"
#include "unit.h"
#include "unittype.h"
#include "video.h"

/*----------------------------------------------------------------------------
--  Functions
----------------------------------------------------------------------------*/

/* static */ COrder *COrder::NewActionUnload(const Vec2i &pos, CUnit *what)
{
	COrder_Unload *order = new COrder_Unload;

	order->goalPos = pos;
	if (what && !what->Destroyed) {
		order->SetGoal(what);
	}
	return order;
}

/* virtual */ void COrder_Unload::Save(CFile &file, const CUnit &unit) const
{
	file.printf("{\"action-unload\",");
	if (this->Finished) {
		file.printf(" \"finished\", ");
	}
	if (this->HasGoal()) {
		file.printf(" \"goal\", \"%s\",", UnitReference(this->GetGoal()).c_str());
	}
	file.printf(" \"tile\", {%d, %d}, ", this->goalPos.x, this->goalPos.y);
	file.printf("\"state\", %d", this->State);
	file.printf("}");
}

/* virtual */ bool COrder_Unload::ParseSpecificData(lua_State *l, int &j, const char *value, const CUnit &unit)
{
	if (!strcmp("state", value)) {
		++j;
		lua_rawgeti(l, -1, j + 1);
		this->State = LuaToNumber(l, -1);
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

/* virtual */ bool COrder_Unload::IsValid() const
{
	return true;
}

/* virtual */ PixelPos COrder_Unload::Show(const CViewport &vp, const PixelPos &lastScreenPos) const
{
	const PixelPos targetPos = vp.TilePosToScreen_Center(this->goalPos);

	Video.FillCircleClip(ColorGreen, lastScreenPos, 2);
	Video.DrawLineClip(ColorGreen, lastScreenPos, targetPos);
	Video.FillCircleClip(ColorGreen, targetPos, 3);
	return targetPos;
}

/* virtual */ void COrder_Unload::UpdatePathFinderData(PathFinderInput &input)
{
	input.SetMinRange(0);
	input.SetMaxRange(0);

	const Vec2i tileSize(0, 0);

	input.SetGoal(this->goalPos, tileSize);
}


/**
**  Find a free position close to startPos
**
**  @param transporter
**  @param unit         Unit to unload.
**  @param startPos     Original search position
**  @param maxrange     maximal range to unload.
**  @param res          Unload position.
**
**  @return      True if a position was found, False otherwise.
**  @note        res is undefined if a position is not found.
**
**  @bug         FIXME: Place unit only on fields reachable from the transporter
*/
static bool FindUnloadPosition(const CUnit &transporter, const CUnit &unit, const Vec2i startPos, int maxRange, Vec2i *res)
{
	Vec2i pos = startPos;

	pos.x -= unit.Type->TileWidth - 1;
	pos.y -= unit.Type->TileHeight - 1;
	int addx = transporter.Type->TileWidth + unit.Type->TileWidth - 1;
	int addy = transporter.Type->TileHeight + unit.Type->TileHeight - 1;

	--pos.x;
	for (int range = 0; range < maxRange; ++range) {
		for (int i = addy; i--; ++pos.y) {
			if (UnitCanBeAt(unit, pos)) {
				*res = pos;
				return true;
			}
		}
		++addx;

		for (int i = addx; i--; ++pos.x) {
			if (UnitCanBeAt(unit, pos)) {
				*res = pos;
				return true;
			}
		}
		++addy;

		for (int i = addy; i--; --pos.y) {
			if (UnitCanBeAt(unit, pos)) {
				*res = pos;
				return true;
			}
		}
		++addx;

		for (int i = addx; i--; --pos.x) {
			if (UnitCanBeAt(unit, pos)) {
				*res = pos;
				return true;
			}
		}
		++addy;
	}
	return false;
}

/**
**  Reappear unit on map.
**
**  @param unit  Unit to drop out.
**
**  @return      True if unit is unloaded.
**
**  @bug         FIXME: Place unit only on fields reachable from the transporter
*/
static int UnloadUnit(CUnit &transporter, CUnit &unit)
{
	const int maxRange = 1;
	Vec2i pos;

	Assert(unit.Removed);
	if (!FindUnloadPosition(transporter, unit, transporter.tilePos, maxRange, &pos)) {
		return false;
	}
	unit.Boarded = 0;
	unit.Place(pos);
	transporter.BoardCount--;
	return true;
}

/**
**  Return true is possition is a correct place to drop out units.
**
**  @param transporter  Transporter unit.
**  @param pos          position to drop out units.
*/
static bool IsDropZonePossible(const CUnit &transporter, const Vec2i &pos)
{
	const int maxUnloadRange = 1;

	if (!UnitCanBeAt(transporter, pos)) {
		return false;
	}
	Vec2i dummyPos;
	CUnit *unit = transporter.UnitInside;
	for (int i = 0; i < transporter.InsideCount; ++i, unit = unit->NextContained) {
		if (FindUnloadPosition(transporter, *unit, pos, maxUnloadRange, &dummyPos)) {
			return true;
		}
	}
	// Check unit can be droped from here.
	return false;
}


/**
**  Find the closest available drop zone for a transporter.
**  Fail if transporter don't transport any unit..
**
**  @param  transporter  the transporter
**  @param  startPos     start location for the search
**	@param  maxRange     The maximum distance from initial position to search...
**  @param  resPos       drop zone position
**
**  @return              true if a location was found, false otherwise
**  @note to be called only from ClosestFreeDropZone.
*/
static bool ClosestFreeDropZone_internal(const CUnit &transporter, const Vec2i &startPos, int maxRange, Vec2i *resPos)
{
	int addx = 0;
	int addy = 1;
	Vec2i pos = startPos;

	for (int range = 0; range < maxRange; ++range) {
		for (int i = addy; i--; ++pos.y) {
			if (IsDropZonePossible(transporter, pos)) {
				*resPos = pos;
				return true;
			}
		}
		++addx;
		for (int i = addx; i--; ++pos.x) {
			if (IsDropZonePossible(transporter, pos)) {
				*resPos = pos;
				return true;
			}
		}
		++addy;
		for (int i = addy; i--; --pos.y) {
			if (IsDropZonePossible(transporter, pos)) {
				*resPos = pos;
				return true;
			}
		}
		++addx;
		for (int i = addx; i--; --pos.x) {
			if (IsDropZonePossible(transporter, pos)) {
				*resPos = pos;
				return true;
			}
		}
		++addy;
	}
	DebugPrint("Try clicking closer to an actual coast.\n");
	return false;
}

/**
**  Find the closest available drop zone for a transporter.
**  Fail if transporter don't transport any unit..
**
**  @param  transporter  the transporter
**  @param  startPos     start location for the search
**	@param  maxRange     The maximum distance from initial position to search...
**  @param  resPos       drop zone position
**
**  @return              1 if a location was found, 0 otherwise
*/
static int ClosestFreeDropZone(CUnit &transporter, const Vec2i &startPos, int maxRange, Vec2i *resPos)
{
	// Check there are units onboard
	if (!transporter.UnitInside) {
		return 0;
	}
	const bool isTransporterRemoved = transporter.Removed;

	if (!isTransporterRemoved) {
		// Remove transporter to avoid "collision" with itself.
		transporter.Remove(NULL);
	}
	const bool res = ClosestFreeDropZone_internal(transporter, startPos, maxRange, resPos);
	if (!isTransporterRemoved) {
		transporter.Place(transporter.tilePos);
	}
	return res;
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
**  @return false if action should continue
*/
bool COrder_Unload::LeaveTransporter(CUnit &transporter)
{
	int stillonboard = 0;

	// Goal is the specific unit unit that you want to unload.
	// This can be NULL, in case you want to unload everything.
	if (this->HasGoal()) {
		CUnit &goal = *this->GetGoal();

		if (goal.Destroyed) {
			DebugPrint("destroyed unit unloading?\n");
			this->ClearGoal();
			return true;
		}
		transporter.CurrentOrder()->ClearGoal();
		// Try to unload the unit. If it doesn't work there is no problem.
		if (UnloadUnit(transporter, goal)) {
			this->ClearGoal();
		} else {
			++stillonboard;
		}
	} else {
		// Unload all units.
		CUnit *goal = transporter.UnitInside;
		for (int i = transporter.InsideCount; i; --i, goal = goal->NextContained) {
			if (goal->Boarded) {
				if (!UnloadUnit(transporter, *goal)) {
					++stillonboard;
				}
			}
		}
	}
	if (IsOnlySelected(transporter)) {
		SelectedUnitChanged();
	}

	// We still have some units to unload, find a piece of free coast.
	if (stillonboard) {
		// We tell it to unload at it's current position. This can't be done,
		// so it will search for a piece of free coast nearby.
		this->State = 0;
		return false;
	} else {
		return true;
	}
}

/* virtual */ void COrder_Unload::Execute(CUnit &unit)
{
	const int maxSearchRange = 20;

	if (!unit.CanMove()) {
		this->State = 2;
	}
	switch (this->State) {
		case 0: // Choose destination
			if (!this->HasGoal()) {
				Vec2i pos;

				if (!ClosestFreeDropZone(unit, this->goalPos, maxSearchRange, &pos)) {
					this->Finished = true;
					return ;
				}
				this->goalPos = pos;
			}

			this->State = 1;
			// follow on next case
		case 1: // Move unit to destination
			// The Goal is the unit that we have to unload.
			if (!this->HasGoal()) {
				const int moveResult = MoveToDropZone(unit);

				// We have to unload everything
				if (moveResult) {
					if (moveResult == PF_REACHED) {
						if (++this->State == 1) {
							this->Finished = true;
							return ;
						}
					} else {
						this->State = 2;
					}
				}
				return ;
			}
		case 2: { // Leave the transporter
			// FIXME: show still animations ?
			if (LeaveTransporter(unit)) {
				this->Finished = true;
				return ;
			}
			return ;
		}
		default:
			return ;
	}
}

//@}
