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
/**@name test_util.cpp - The test file for util.cpp. */
//
//      (c) Copyright 2013 by Joris Dauphin
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
#include "util.h"

TEST_CASE("square")
{
	CHECK(4 == square(2));
	CHECK(0 == square(0));
	CHECK(36 == square(6));
}

TEST_CASE("clamp max")
{
	int x = 42;

	clamp(&x, 0, 10);
	CHECK(10 == x);
}

TEST_CASE("clamp min")
{
	int x = -42;

	clamp(&x, 0, 10);
	CHECK(0 == x);
}

TEST_CASE("clamp in")
{
	int x = 4;

	clamp(&x, 0, 10);
	CHECK(4 == x);
}

TEST_CASE("strcpy_s")
{
	char buffer[42];

	// CHECK(EINVAL == strcpy_s(nullptr, 42, "NULL dest"));
	// CHECK(EINVAL == strcpy_s(buffer, 42, nullptr));
	// CHECK(ERANGE == strcpy_s(buffer, 10, "longer than given size"));
	CHECK(0 == strcpy_s(buffer, 10, "correct"));
	CHECK(std::string_view{"correct"} == buffer);
}

TEST_CASE("strncpy_s")
{
	char buffer[42];

	//CHECK(EINVAL == strncpy_s(nullptr, 42, "NULL dest", 5));
	//CHECK(EINVAL == strncpy_s(buffer, 42, nullptr, 5));
	//CHECK(EINVAL == strncpy_s(buffer, 10, "longer than given size", 12));
	CHECK(0 == strncpy_s(buffer, 10, "correct", 8));
	CHECK(std::string_view{"correct"} == buffer);
}

TEST_CASE("strcat_s")
{
	char buffer[42];

	buffer[0] = '\0';

	//CHECK(EINVAL == strcat_s(nullptr, 42, "NULL dest"));
	//CHECK(EINVAL == strcat_s(buffer, 42, nullptr));
	//CHECK(ERANGE == strcat_s(buffer, 10, "longer than given size"));
	CHECK(0 == strcat_s(buffer, 42, "hello"));
	CHECK(0 == strcat_s(buffer, 42, " world"));
	CHECK(std::string_view{"hello world"} == buffer);
}

TEST_CASE("strnlen")
{
	CHECK(2u == strnlen("hello", 2));
	CHECK(5u == strnlen("hello", 5));
	CHECK(5u == strnlen("hello", 10));
}

TEST_CASE("fletcher32")
{
	CHECK(4031760169 == fletcher32("abcde"));    // 0xF04FC729
	CHECK(1448095018 == fletcher32("abcdef"));   // 0x56502D2A
	CHECK(3957429649 == fletcher32("abcdefgh")); // 0xEBE19591
}

// TODO: int getopt(int argc, char *const argv[], const char *optstring);
// TODO: std::optional<std::string> GetClipboard();
// TODO: int UTF8GetNext(const std::string &text, int curpos);
// TODO: int UTF8GetPrev(const std::string &text, int curpos);
