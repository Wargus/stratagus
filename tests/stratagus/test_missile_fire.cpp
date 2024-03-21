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
	MissileType* dummyMissile(uintptr_t m)
	{
		return reinterpret_cast<MissileType*>(0x10000 + m);
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
	MissileType *m0 = dummyMissile(0);
	MissileType *m1 = dummyMissile(1);
	MissileType *m2 = dummyMissile(2);
	MissileType *m3 = dummyMissile(3);
	MissileType *m4 = dummyMissile(4);
	MissileType *m5 = dummyMissile(5);
	MissileType *m6 = dummyMissile(6);
	MissileType *m7 = dummyMissile(7);
	MissileType *m8 = dummyMissile(8);
	MissileType *m9 = dummyMissile(9);

	BurningBuildingFrames.clear();

	SUBCASE("size=0") {
		setFrames({});
		CHECK(IsBurningBuildingFramesValid());

		CHECK(MissileBurningBuilding(  0) == nullptr);
	}

	SUBCASE("size=1") {
		setFrames({
			{  0, m0},
		});
		CHECK(IsBurningBuildingFramesValid());

		CHECK(MissileBurningBuilding(  0) == m0);
		CHECK(MissileBurningBuilding(  1) == m0);
		CHECK(MissileBurningBuilding(100) == m0);
	}

	SUBCASE("large steps") {
		setFrames({
			{  0, m0},
			{ 50, m1},
		});
		CHECK(IsBurningBuildingFramesValid());

		SUBCASE("normal") {
			CHECK(MissileBurningBuilding(  0) == m0);
			CHECK(MissileBurningBuilding(  1) == m0);
			CHECK(MissileBurningBuilding( 49) == m0);
			CHECK(MissileBurningBuilding( 50) == m1);
			CHECK(MissileBurningBuilding( 51) == m1);
			CHECK(MissileBurningBuilding(100) == m1);
		}

		SUBCASE("out of bounds") {
			CHECK(MissileBurningBuilding( -1) == nullptr);
			CHECK(MissileBurningBuilding(101) == m1);
		}
	}

	SUBCASE("partially dense")
	{
		setFrames({
			{  0, m0},
			{  1, m1},
			{  2, m2},
			{ 65, m3},
			{ 66, m4},
			{ 67, m5},
			{ 99, m6},
			{100, m7},
		});
		CHECK(IsBurningBuildingFramesValid());

		CHECK(MissileBurningBuilding(  0) == m0);
		CHECK(MissileBurningBuilding(  1) == m1);
		CHECK(MissileBurningBuilding(  2) == m2);
		CHECK(MissileBurningBuilding(  3) == m2);

		CHECK(MissileBurningBuilding( 50) == m2);

		CHECK(MissileBurningBuilding( 64) == m2);
		CHECK(MissileBurningBuilding( 65) == m3);
		CHECK(MissileBurningBuilding( 66) == m4);
		CHECK(MissileBurningBuilding( 67) == m5);
		CHECK(MissileBurningBuilding( 68) == m5);

		CHECK(MissileBurningBuilding( 98) == m5);
		CHECK(MissileBurningBuilding( 99) == m6);
		CHECK(MissileBurningBuilding(100) == m7);
	}

	SUBCASE("repeated values and null")
	{
		setFrames({
			{  0, m8},
			{ 80, nullptr},
			{ 90, m8},
		});
		CHECK(IsBurningBuildingFramesValid());

		CHECK(MissileBurningBuilding( 79) == m8);
		CHECK(MissileBurningBuilding( 80) == nullptr);
		CHECK(MissileBurningBuilding( 89) == nullptr);
		CHECK(MissileBurningBuilding( 90) == m8);
	}

	SUBCASE("first key larger") {
		setFrames({
			{ 10, m5},
		});
		CHECK(IsBurningBuildingFramesValid());

		CHECK(MissileBurningBuilding( 9) == nullptr);
		CHECK(MissileBurningBuilding(10) == m5);
	}

	SUBCASE("duplicate keys")
	{
		setFrames({
			{  0, m0},
			{ 20, m1},
			{ 20, m2},
			{ 60, m3},
		});
		CHECK(IsBurningBuildingFramesValid());

		CHECK(MissileBurningBuilding(19) == m0);
		CHECK(MissileBurningBuilding(20) == m2);
		CHECK(MissileBurningBuilding(21) == m2);
	}

	SUBCASE("wrong key order") {
		setFrames({
			{ 75, m9},
			{ 50, m8},
			{  0, m7},
		});

		CHECK_FALSE(IsBurningBuildingFramesValid());
	}

	SUBCASE("too small key value") {
		setFrames({
			{ -1, m7},
			{100, m8},
		});

		CHECK_FALSE(IsBurningBuildingFramesValid());
	}

	SUBCASE("too large key value") {
		setFrames({
			{  0, m7},
			{101, m8},
		});

		CHECK_FALSE(IsBurningBuildingFramesValid());
	}

	BurningBuildingFrames.clear();
}
