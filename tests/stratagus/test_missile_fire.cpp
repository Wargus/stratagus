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

	void setFrames(const std::vector<std::pair<float, MissileType*>>& frames)
	{
		const std::size_t size = frames.size();
		BurningBuildingFrames.resize(size);
		for (int i = 0; i < size; ++i) {
			BurningBuildingFrames[i].Percent = frames[i].first;
			BurningBuildingFrames[i].Missile = frames[i].second;
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

		CHECK(MissileBurningBuilding(  0.F) == nullptr);
	}

	SUBCASE("size=1") {
		setFrames({
			{  0.F, m0},
		});
		CHECK(IsBurningBuildingFramesValid());

		CHECK(MissileBurningBuilding(  0.F) == m0);
		CHECK(MissileBurningBuilding(  1.F) == m0);
		CHECK(MissileBurningBuilding(100.F) == m0);
	}

	SUBCASE("large steps") {
		setFrames({
			{  0.F, m0},
			{ 50.F, m1},
		});
		CHECK(IsBurningBuildingFramesValid());

		SUBCASE("normal") {
			CHECK(MissileBurningBuilding(  0.F) == m0);
			CHECK(MissileBurningBuilding(  1.F) == m0);
			CHECK(MissileBurningBuilding( 49.F) == m0);
			CHECK(MissileBurningBuilding( 50.F) == m1);
			CHECK(MissileBurningBuilding( 51.F) == m1);
			CHECK(MissileBurningBuilding(100.F) == m1);
		}

		SUBCASE("out of bounds") {
			CHECK(MissileBurningBuilding( -1.F) == nullptr);
			CHECK(MissileBurningBuilding(101.F) == m1);
		}
	}

	SUBCASE("partially dense")
	{
		setFrames({
			{  0.F, m0},
			{ 0.5F, m1},
			{  1.F, m2},
			{ 65.F, m3},
			{ 66.F, m4},
			{ 67.F, m5},
			{ 99.F, m6},
			{100.F, m7},
		});
		CHECK(IsBurningBuildingFramesValid());

		CHECK(MissileBurningBuilding(  0.F) == m0);
		CHECK(MissileBurningBuilding( 0.5F) == m1);
		CHECK(MissileBurningBuilding(  1.F) == m2);
		CHECK(MissileBurningBuilding(  2.F) == m2);

		CHECK(MissileBurningBuilding( 50.F) == m2);

		CHECK(MissileBurningBuilding( 64.F) == m2);
		CHECK(MissileBurningBuilding( 65.F) == m3);
		CHECK(MissileBurningBuilding( 66.F) == m4);
		CHECK(MissileBurningBuilding( 67.F) == m5);
		CHECK(MissileBurningBuilding( 68.F) == m5);

		CHECK(MissileBurningBuilding( 98.F) == m5);
		CHECK(MissileBurningBuilding( 99.F) == m6);
		CHECK(MissileBurningBuilding(100.F) == m7);
	}

	SUBCASE("repeated values and null")
	{
		setFrames({
			{  0.F, m8},
			{ 80.F, nullptr},
			{ 90.F, m8},
		});
		CHECK(IsBurningBuildingFramesValid());

		CHECK(MissileBurningBuilding( 79.F) == m8);
		CHECK(MissileBurningBuilding( 80.F) == nullptr);
		CHECK(MissileBurningBuilding( 89.F) == nullptr);
		CHECK(MissileBurningBuilding( 90.F) == m8);
	}

	SUBCASE("first key larger") {
		setFrames({
			{ 10.F, m5},
		});
		CHECK(IsBurningBuildingFramesValid());

		CHECK(MissileBurningBuilding( 9.F) == nullptr);
		CHECK(MissileBurningBuilding(10.F) == m5);
	}

	SUBCASE("duplicate keys")
	{
		setFrames({
			{  0.F, m0},
			{ 20.F, m1},
			{ 20.F, m2},
			{ 60.F, m3},
		});
		CHECK(IsBurningBuildingFramesValid());

		CHECK(MissileBurningBuilding(19.F) == m0);
		CHECK(MissileBurningBuilding(20.F) == m2);
		CHECK(MissileBurningBuilding(21.F) == m2);
	}

	SUBCASE("wrong key order") {
		setFrames({
			{ 75.F, m9},
			{ 50.F, m8},
			{  0.F, m7},
		});

		CHECK_FALSE(IsBurningBuildingFramesValid());
	}

	SUBCASE("too small key value") {
		setFrames({
			{ -1.F, m7},
			{100.F, m8},
		});

		CHECK_FALSE(IsBurningBuildingFramesValid());
	}

	SUBCASE("too large key value") {
		setFrames({
			{  0.F, m7},
			{101.F, m8},
		});

		CHECK_FALSE(IsBurningBuildingFramesValid());
	}

	BurningBuildingFrames.clear();
}
