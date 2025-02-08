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

#include "animation.h"
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
--  Constants
----------------------------------------------------------------------------*/

constexpr int MAX_SEARCH_RANGE = 20;
constexpr int MAX_RETRIES = 20;
constexpr int FIND_DROPZONE_STATE = 0;
constexpr int MOVE_TO_DROPZONE_STATE = 1;
constexpr int UNLOAD_STATE = 2;

/*----------------------------------------------------------------------------
--  Functions
----------------------------------------------------------------------------*/

/* static */ std::unique_ptr<COrder> COrder::NewActionUnload(const Vec2i &pos, CUnit *what)
{
	auto order = std::make_unique<COrder_Unload>();

	order->goalPos = pos;
	if (what && !what->Destroyed) {
		order->SetGoal(what);
	}
	return order;
}

void COrder_Unload::Save(CFile &file, const CUnit &unit) const /* override */
{
	file.printf("{\"action-unload\",");
	if (this->Finished) {
		file.printf(" \"finished\", ");
	}
	file.printf(" \"retries\", %d,", this->Retries);
	if (this->HasGoal()) {
		file.printf(" \"goal\", \"%s\",", UnitReference(*this->GetGoal()).c_str());
	}
	file.printf(" \"tile\", {%d, %d}, ", this->goalPos.x, this->goalPos.y);
	file.printf("\"state\", %d", this->State);
	file.printf("}");
}

bool COrder_Unload::ParseSpecificData(lua_State *l, int &j, std::string_view value, const CUnit &unit)
{
	if (value == "state") {
		++j;
		this->State = LuaToNumber(l, -1, j + 1);
	} else if (value == "retries") {
		++j;
		this->Retries = LuaToNumber(l, -1, j + 1);
	} else if (value == "tile") {
		++j;
		lua_rawgeti(l, -1, j + 1);
		CclGetPos(l, &this->goalPos);
		lua_pop(l, 1);
	} else {
		return false;
	}
	return true;
}

bool COrder_Unload::IsValid() const /* override */
{
	return true;
}

PixelPos COrder_Unload::Show(const CViewport &vp,
                             const PixelPos &lastScreenPos) const /* override */
{
	const PixelPos targetPos = vp.TilePosToScreen_Center(this->goalPos);

	Video.FillCircleClip(ColorGreen, lastScreenPos, 2);
	Video.DrawLineClip(ColorGreen, lastScreenPos, targetPos);
	Video.FillCircleClip(ColorGreen, targetPos, 3);
	return targetPos;
}

void COrder_Unload::UpdatePathFinderData(PathFinderInput &input) /* override */
{
	input.SetMinRange(0);
	input.SetMaxRange(this->State == FIND_DROPZONE_STATE ? 1 : 0);

	const Vec2i tileSize(0, 0);

	input.SetGoal(this->goalPos, tileSize);
}


/**
**  Find a free position close to startPos
**
**  @param transporter
**  @param unit         Unit to unload.
**  @param startPos     Original search position
**  @param maxrange     maximum range to unload.
**  @param res          Unload position.
**
**  @return      found position, std::nullopt otherwise.
**
**  @bug         FIXME: Place unit only on fields reachable from the transporter
*/
static std::optional<Vec2i>
FindUnloadPosition(const CUnit &transporter, const CUnit &unit, const Vec2i startPos, int maxRange)
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
				return pos;
			}
		}
		++addx;

		for (int i = addx; i--; ++pos.x) {
			if (UnitCanBeAt(unit, pos)) {
				return pos;
			}
		}
		++addy;

		for (int i = addy; i--; --pos.y) {
			if (UnitCanBeAt(unit, pos)) {
				return pos;
			}
		}
		++addx;

		for (int i = addx; i--; --pos.x) {
			if (UnitCanBeAt(unit, pos)) {
				return pos;
			}
		}
		++addy;
	}
	return std::nullopt;
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
static bool UnloadUnit(CUnit &transporter, CUnit &unit)
{
	const int maxRange = 1;

	Assert(unit.Removed);
	if (auto pos = FindUnloadPosition(transporter, unit, transporter.tilePos, maxRange)) {
		unit.Boarded = 0;
		unit.Place(*pos);
		transporter.BoardCount -= unit.Type->BoardSize;
		return true;
	}
	return false;
}

/**
**  Return true if position is a correct place to drop out units.
**
**  @param transporter  Transporter unit.
**  @param pos          position to drop out units.
*/
static bool IsDropZonePossible(const CUnit &transporter, const Vec2i &pos)
{
	constexpr int maxUnloadRange = 1;

	if (!UnitCanBeAt(transporter, pos)) {
		return false;
	}
	return ranges::any_of(transporter.InsideUnits, [&](CUnit *unit) {
		return FindUnloadPosition(transporter, *unit, pos, maxUnloadRange);
	});
}


/**
**  Find the closest available drop zone for a transporter.
**  Fail if transporter don't transport any unit..
**
**  @param  transporter  the transporter
**  @param  startPos     start location for the search
**  @param  maxRange     The maximum distance from initial position to search...
**
**  @return              drop zone position if a location was found, std::nullopt otherwise
**  @note to be called only from ClosestFreeDropZone.
*/
static std::optional<Vec2i>
ClosestFreeDropZone_internal(const CUnit &transporter, const Vec2i &startPos, int maxRange)
{
	int addx = 0;
	int addy = 1;
	Vec2i pos = startPos;

	for (int range = 0; range < maxRange; ++range) {
		for (int i = addy; i--; ++pos.y) {
			if (IsDropZonePossible(transporter, pos)) {
				return pos;
			}
		}
		++addx;
		for (int i = addx; i--; ++pos.x) {
			if (IsDropZonePossible(transporter, pos)) {
				return pos;
			}
		}
		++addy;
		for (int i = addy; i--; --pos.y) {
			if (IsDropZonePossible(transporter, pos)) {
				return pos;
			}
		}
		++addx;
		for (int i = addx; i--; --pos.x) {
			if (IsDropZonePossible(transporter, pos)) {
				return pos;
			}
		}
		++addy;
	}
	DebugPrint("Try clicking closer to an actual coast.\n");
	return std::nullopt;
}

/**
**  Find the closest available drop zone for a transporter.
**  Fail if transporter don't transport any unit..
**
**  @param  transporter  the transporter
**  @param  startPos     start location for the search
**  @param  maxRange     The maximum distance from initial position to search...
**
**  @return              Drop zone position if a location was found, std::nullopt otherwise
*/
static std::optional<Vec2i>
ClosestFreeDropZone(CUnit &transporter, const Vec2i &startPos, int maxRange)
{
	// Check there are units onboard
	if (transporter.InsideUnits.empty()) {
		return std::nullopt;
	}
	const bool isTransporterRemoved = transporter.Removed;
	const bool selected = transporter.Selected;

	if (!isTransporterRemoved) {
		// Remove transporter to avoid "collision" with itself.
		transporter.Remove(nullptr);
	}
	const auto resPos = ClosestFreeDropZone_internal(transporter, startPos, maxRange);
	if (!isTransporterRemoved) {
		transporter.Place(transporter.tilePos);
		if (selected) {
			SelectUnit(transporter);
			SelectionChanged();
		}
	}
	return resPos;
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
	// This can be nullptr, in case you want to unload everything.
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
		auto insideUnits = transporter.InsideUnits; // UnloadUnit modifies transporter.InsideUnits;

		for (CUnit *goal : insideUnits) {
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
		return false;
	} else {
		return true;
	}
}

void COrder_Unload::Execute(CUnit &unit) /* override */
{
	if (!unit.CanMove()) {
		this->State = UNLOAD_STATE;
	}

	if (IsWaiting(unit)) {
		return;
	}
	StopWaiting(unit);
	if (this->Retries >= MAX_RETRIES) {
		// failed to reach the goal
		Assert(!unit.Moving && !unit.Anim.Unbreakable);
		this->Finished = true;
		return;
	}

	switch (this->State) {
		case FIND_DROPZONE_STATE:
		{
			if (auto pos = ClosestFreeDropZone(unit, this->goalPos, MAX_SEARCH_RANGE)) {
				this->goalPos = *pos;
				this->Retries = 0;
				this->State = MOVE_TO_DROPZONE_STATE;
			} else {
				this->Retries = MAX_RETRIES;
				return;
			}
		}
			[[fallthrough]]; // fallthrough and move immediately
		case MOVE_TO_DROPZONE_STATE:
			switch (DoActionMove(unit)) {
				case PF_UNREACHABLE:
					unit.Wait = 5;
					this->Retries++;
					return;
				case PF_REACHED:
					this->Retries = 0;
					this->State = 2;
					break;
				default:
					// still moving or waiting to move
					return;
			}
			[[fallthrough]];
		case UNLOAD_STATE:
			// Leave the transporter
			// FIXME: show still animations ?
			if (LeaveTransporter(unit)) {
				this->Retries = MAX_RETRIES;
			} else {
				// We tell try to unload from the beginning at it's current position.
				// Since this didn't work just now, it will search for a piece of free
				// coast nearby.
				this->State = FIND_DROPZONE_STATE;
				this->Retries++;
			}
			break;
		default:
			Assert(false);
	}
}

//@}
