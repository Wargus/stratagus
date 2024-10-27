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
/**@name test_format.cpp - Test file for function Format. */
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

TEST_CASE("Format")
{
	CHECK("" == Format(""));
	CHECK("1" == Format("1"));
	CHECK("12" == Format("12"));
	CHECK("12\n" == Format("12\n"));
	CHECK("abc\n" == Format("%s", "abc\n"));
	CHECK("abc\n" == Format("%s\n", "abc"));
	CHECK("12345" == Format("%d", 12345));
	CHECK("         x" == Format("%*s", 10, "x"));
	std::string result = Format("%*s", 100, "x");
	REQUIRE(result.size() == 100);
	CHECK(result[98] == ' ');
	CHECK(result[99] == 'x');
}
