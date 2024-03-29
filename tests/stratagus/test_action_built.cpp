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
/**@name test_action_built.cpp - The test file for action_built.cpp. */
//
//      (c) Copyright 2024 by Matthias Schwarzott
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

#include <doctest.h>

#include "stratagus.h"
#include "unit.h"
#include "action/action_built.h"
#include "construct.h"

class tested_COrder_Built : public COrder_Built
{
public:
	using COrder_Built::ProgressCounter;

	int testUpdateConstrFrame_Percent(CUnit &unit, int percent, int initialFrame = 9999)
	{
		const int progressOnePercent = unit.Type->Stats[0].Costs[TimeCost] * 6;
		return testUpdateConstrFrame_Progress(unit, percent * progressOnePercent, initialFrame);
		}

	int testUpdateConstrFrame_Progress(CUnit &unit, int progress, int initialFrame = 9999)
	{
		unit.Frame = initialFrame; // to see if it was set
		Frame = -5; // force re-calculation
		ProgressCounter = progress;
		UpdateConstructionFrame(unit);
		return unit.Frame;
	}
};

namespace {
	void setFrames(std::vector<CConstructionFrame> &cframes,
				   const std::vector<std::pair<int, int>>& vec)
	{
		const std::size_t size = vec.size();
		cframes.resize(size);
		for (int i = 0; i < size; ++i) {
			cframes[i].Percent = vec[i].first;
			cframes[i].Frame = vec[i].second;
		}
	}
};

TEST_CASE("COrder_Built Frame")
{
	CPlayer player;
	player.Index = 0;
	CUnitType type;
	type.Stats[0].Costs[TimeCost] = 1;
	CConstruction construction;
	type.Construction = &construction;
	auto &frames = construction.Frames;
	CUnit unit;
	unit.Type = &type;
	unit.Player = &player;
	tested_COrder_Built order;

	SUBCASE("init values") {
		CHECK(unit.Frame == 0);
		CHECK(order.ProgressCounter == 0);
	}

	SUBCASE("size=0") {
		setFrames(frames, {});

		// check unit.Frame is not modified
		CHECK(order.testUpdateConstrFrame_Percent(unit, 0, 123) == 123);
	}

	SUBCASE("size=1") {
		setFrames(frames, {
			{0, 0},
		});

		CHECK(order.testUpdateConstrFrame_Percent(unit,   0) == 0);
		CHECK(order.testUpdateConstrFrame_Percent(unit,   1) == 0);
		CHECK(order.testUpdateConstrFrame_Percent(unit, 100) == 0);
	}

	SUBCASE("large steps") {
		setFrames(frames, {
			{ 0, 100},
			{50, 110},
		});

		SUBCASE("normal") {
			CHECK(order.testUpdateConstrFrame_Percent(unit,   0) == 100);
			CHECK(order.testUpdateConstrFrame_Percent(unit,   1) == 100);
			CHECK(order.testUpdateConstrFrame_Percent(unit,  49) == 100);
			CHECK(order.testUpdateConstrFrame_Percent(unit,  50) == 110);
			CHECK(order.testUpdateConstrFrame_Percent(unit,  51) == 110);
			CHECK(order.testUpdateConstrFrame_Percent(unit, 100) == 110);
		}

		SUBCASE("mirrored") {
			CHECK(order.testUpdateConstrFrame_Percent(unit, 49, -1) == -101);
			CHECK(order.testUpdateConstrFrame_Percent(unit, 50, -1) == -111);
		}

		SUBCASE("out of bounds") {
			// return frame of first entry
			CHECK(order.testUpdateConstrFrame_Percent(unit,  -1) == 100);
			CHECK(order.testUpdateConstrFrame_Percent(unit, 101) == 110);
		}
	}

	SUBCASE("partially dense")
	{
		setFrames(frames, {
			{  0, 0},
			{  1, 1},
			{  2, 2},
			{ 65, 3},
			{ 66, 4},
			{ 67, 5},
			{ 99, 6},
			{100, 7},
		});

		CHECK(order.testUpdateConstrFrame_Percent(unit,   0) == 0);
		CHECK(order.testUpdateConstrFrame_Percent(unit,   1) == 1);
		CHECK(order.testUpdateConstrFrame_Percent(unit,   2) == 2);
		CHECK(order.testUpdateConstrFrame_Percent(unit,   3) == 2);

		CHECK(order.testUpdateConstrFrame_Percent(unit,  50) == 2);

		CHECK(order.testUpdateConstrFrame_Percent(unit,  64) == 2);
		CHECK(order.testUpdateConstrFrame_Percent(unit,  65) == 3);
		CHECK(order.testUpdateConstrFrame_Percent(unit,  66) == 4);
		CHECK(order.testUpdateConstrFrame_Percent(unit,  67) == 5);
		CHECK(order.testUpdateConstrFrame_Percent(unit,  68) == 5);

		CHECK(order.testUpdateConstrFrame_Percent(unit,  98) == 5);
		CHECK(order.testUpdateConstrFrame_Percent(unit,  99) == 6);
		CHECK(order.testUpdateConstrFrame_Percent(unit, 100) == 7);

		SUBCASE("by progress")
		{
			CHECK(order.testUpdateConstrFrame_Progress(unit,   0) == 0);
			CHECK(order.testUpdateConstrFrame_Progress(unit,   5) == 0);
			CHECK(order.testUpdateConstrFrame_Progress(unit,   6) == 1);
			CHECK(order.testUpdateConstrFrame_Progress(unit, 594) == 6);
			CHECK(order.testUpdateConstrFrame_Progress(unit, 595) == 6);
			CHECK(order.testUpdateConstrFrame_Progress(unit, 600) == 7);
		}
	}

	SUBCASE("repeated values")
	{
		setFrames(frames, {
			{  0, 8},
			{ 80, 1},
			{ 90, 8},
		});

		CHECK(order.testUpdateConstrFrame_Percent(unit, 79) == 8);
		CHECK(order.testUpdateConstrFrame_Percent(unit, 80) == 1);
		CHECK(order.testUpdateConstrFrame_Percent(unit, 89) == 1);
		CHECK(order.testUpdateConstrFrame_Percent(unit, 90) == 8);
	}

	SUBCASE("first key larger") {
		setFrames(frames, {
			{ 10, 5},
		});

		CHECK(order.testUpdateConstrFrame_Percent(unit,  9) == 5);
		CHECK(order.testUpdateConstrFrame_Percent(unit, 10) == 5);
	}

	SUBCASE("duplicate keys")
	{
		setFrames(frames, {
			{  0, 0},
			{ 20, 1},
			{ 20, 2},
			{ 60, 3},
		});

		CHECK(order.testUpdateConstrFrame_Percent(unit, 19) == 0);
		CHECK(order.testUpdateConstrFrame_Percent(unit, 20) == 2);
		CHECK(order.testUpdateConstrFrame_Percent(unit, 21) == 2);
	}

	SUBCASE("wrong key order") {
		setFrames(frames, {
			{ 75, 9},
			{ 50, 8},
			{  0, 7},
		});

		CHECK_FALSE(ranges::is_sorted(frames, std::less<>{}, &CConstructionFrame::Percent));
		const bool allgood =
			order.testUpdateConstrFrame_Percent(unit,   0) == 7 &&
			order.testUpdateConstrFrame_Percent(unit,  49) == 7 &&
			order.testUpdateConstrFrame_Percent(unit,  50) == 8 &&
			order.testUpdateConstrFrame_Percent(unit,  74) == 8 &&
			order.testUpdateConstrFrame_Percent(unit,  75) == 9 &&
			order.testUpdateConstrFrame_Percent(unit,  76) == 9;
		CHECK_FALSE(allgood);
	}

	SUBCASE("too small key value") {
		setFrames(frames, {
			{ -1, 7},
			{100, 8},
		});

		CHECK(order.testUpdateConstrFrame_Percent(unit,   0) == 7);
		CHECK(order.testUpdateConstrFrame_Percent(unit,   1) == 7);
		CHECK(order.testUpdateConstrFrame_Percent(unit,  99) == 7);
		CHECK(order.testUpdateConstrFrame_Percent(unit, 100) == 8);
	}

	SUBCASE("too large key value") {
		setFrames(frames, {
			{  0, 7},
			{101, 8},
		});

		CHECK(order.testUpdateConstrFrame_Percent(unit,   0) == 7);
		CHECK(order.testUpdateConstrFrame_Percent(unit, 100) == 7);
	}
}
