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
/**@name test_unitptr.cpp - The test file for unitptr.cpp. */
//
//      (c) Copyright 2026 by The Stratagus Project
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
#include "unitptr.h"

#include <utility>

namespace {

void MakeAlive(CUnit &unit)
{
	unit.Refs = 1;
	unit.Destroyed = 0;
}

} // namespace

TEST_CASE("CUnitPtr copy assignment keeps reference count balanced")
{
	CUnit unit;
	MakeAlive(unit);

	{
		CUnitPtr first(&unit);
		CHECK(unit.Refs == 2);

		CUnitPtr second;
		second = first;
		CHECK(second.get() == &unit);
		CHECK(unit.Refs == 3);

		CUnitPtr third(first);
		CHECK(third.get() == &unit);
		CHECK(unit.Refs == 4);

		third = first;
		CHECK(unit.Refs == 4);

		second = nullptr;
		CHECK(unit.Refs == 3);
	}

	CHECK(unit.Refs == 1);
}

TEST_CASE("CUnitPtr move operations transfer references")
{
	CUnit unit;
	MakeAlive(unit);

	{
		CUnitPtr first(&unit);
		CHECK(unit.Refs == 2);

		CUnitPtr second(std::move(first));
		CHECK(first.get() == nullptr);
		CHECK(second.get() == &unit);
		CHECK(unit.Refs == 2);

		CUnitPtr third(&unit);
		CHECK(unit.Refs == 3);

		third = std::move(second);
		CHECK(second.get() == nullptr);
		CHECK(third.get() == &unit);
		CHECK(unit.Refs == 2);
	}

	CHECK(unit.Refs == 1);
}
