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
/**@name test_missile_fire.cpp - The test file for missile_fire.cpp / missile.cpp. */
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
#include "missile.h"

namespace {
	MissileType* M(uintptr_t m)
	{
		return reinterpret_cast<MissileType*>(m);
	}

	void setFrames(const std::vector<std::pair<int, MissileType*>>& frames)
	{
		const std::size_t size = frames.size();
		BurningBuildingFrames.resize(size);
		for (int i = 0; i < size; ++i) {
			// reverse order as in CclDefineBurningBuilding
			BurningBuildingFrames[size - 1 - i].Percent = frames[i].first;
			BurningBuildingFrames[size - 1 - i].Missile = frames[i].second;
		}
	}

} // anonymous namespace

TEST_CASE("missile_fire")
{
	BurningBuildingFrames.clear();

	SUBCASE("size=0") {
		setFrames({});

		CHECK(MissileBurningBuilding(  0) == nullptr);
	}

	SUBCASE("size=1") {
		setFrames({
			{  0, M(10)},
		});

		CHECK(MissileBurningBuilding(  0) == M(10));
		CHECK(MissileBurningBuilding(  1) == M(10));
		CHECK(MissileBurningBuilding(100) == M(10));
	}

	SUBCASE("large steps") {
		setFrames({
			{  0, M(10)},
			{ 50, M(11)},
		});

		SUBCASE("normal") {
			CHECK(MissileBurningBuilding(  0) == M(10));
			CHECK(MissileBurningBuilding(  1) == M(10));
			CHECK(MissileBurningBuilding( 49) == M(10));
			CHECK(MissileBurningBuilding( 50) == M(11));
			CHECK(MissileBurningBuilding( 51) == M(11));
			CHECK(MissileBurningBuilding(100) == M(11));
		}

		SUBCASE("out of bounds") {
			CHECK(MissileBurningBuilding( -1) == nullptr);
			CHECK(MissileBurningBuilding(101) == M(11));
		}
	}

	SUBCASE("partially dense")
	{
		setFrames({
			{  0, M(10)},
			{  1, M(11)},
			{  2, M(12)},
			{ 65, M(13)},
			{ 66, M(14)},
			{ 67, M(15)},
			{ 99, M(16)},
			{100, M(17)},
		});

		CHECK(MissileBurningBuilding(  0) == M(10));
		CHECK(MissileBurningBuilding(  1) == M(11));
		CHECK(MissileBurningBuilding(  2) == M(12));
		CHECK(MissileBurningBuilding(  3) == M(12));

		CHECK(MissileBurningBuilding( 50) == M(12));

		CHECK(MissileBurningBuilding( 64) == M(12));
		CHECK(MissileBurningBuilding( 65) == M(13));
		CHECK(MissileBurningBuilding( 66) == M(14));
		CHECK(MissileBurningBuilding( 67) == M(15));
		CHECK(MissileBurningBuilding( 68) == M(15));

		CHECK(MissileBurningBuilding( 98) == M(15));
		CHECK(MissileBurningBuilding( 99) == M(16));
		CHECK(MissileBurningBuilding(100) == M(17));
	}

	SUBCASE("repeated values and null")
	{
		setFrames({
			{  0, M(10)},
			{ 80, nullptr},
			{ 90, M(10)},
		});

		CHECK(MissileBurningBuilding( 79) == M(10));
		CHECK(MissileBurningBuilding( 80) == nullptr);
		CHECK(MissileBurningBuilding( 89) == nullptr);
		CHECK(MissileBurningBuilding( 90) == M(10));
	}

	SUBCASE("first key larger") {
		setFrames({
			{ 10, M(1000)},
		});

		CHECK(MissileBurningBuilding( 9) == nullptr);
		CHECK(MissileBurningBuilding(10) == M(1000));
	}

	SUBCASE("duplicate keys")
	{
		setFrames({
			{  0, M(10)},
			{ 20, M(11)},
			{ 20, M(12)},
			{ 60, M(13)},
		});

		CHECK(MissileBurningBuilding(19) == M(10));
		CHECK(MissileBurningBuilding(20) == M(12));
		CHECK(MissileBurningBuilding(21) == M(12));
	}

	SUBCASE("wrong key order") {
		setFrames({
			{ 75, M(12)},
			{ 50, M(11)},
			{  0, M(10)},
		});

		CHECK(MissileBurningBuilding(  0) == M(10));
		CHECK(MissileBurningBuilding( 20) == M(10));
		CHECK(MissileBurningBuilding( 50) == M(10));
		CHECK(MissileBurningBuilding( 60) == M(10));
		CHECK(MissileBurningBuilding( 75) == M(10));
		CHECK(MissileBurningBuilding( 90) == M(10));
	}

	BurningBuildingFrames.clear();
}
