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

#include <UnitTest++.h>

#include "stratagus.h"
#include "util.h"

TEST(SQUARE)
{
	CHECK_EQUAL(4, square(2));
	CHECK_EQUAL(0, square(0));
	CHECK_EQUAL(36, square(6));
}

TEST(CLAMP_MAX)
{
	int x = 42;

	clamp(&x, 0, 10);
	CHECK_EQUAL(10, x);
}

TEST(CLAMP_MIN)
{
	int x = -42;

	clamp(&x, 0, 10);
	CHECK_EQUAL(0, x);
}

TEST(CLAMP_IN)
{
	int x = 4;

	clamp(&x, 0, 10);
	CHECK_EQUAL(4, x);
}

TEST(STRCPY_S)
{
	char buffer[42];

	CHECK_EQUAL(EINVAL, strcpy_s(NULL, 42, "NULL dest"));
	CHECK_EQUAL(EINVAL, strcpy_s(buffer, 42, NULL));
	CHECK_EQUAL(ERANGE, strcpy_s(buffer, 10, "longer than given size"));
	CHECK_EQUAL(0, strcpy_s(buffer, 10, "correct"));
	CHECK_EQUAL("correct", buffer);
}

TEST(STRNCPY_S)
{
	char buffer[42];

	CHECK_EQUAL(EINVAL, strncpy_s(NULL, 42, "NULL dest", 5));
	CHECK_EQUAL(EINVAL, strncpy_s(buffer, 42, NULL, 5));
	CHECK_EQUAL(EINVAL, strncpy_s(buffer, 10, "longer than given size", 12));
	CHECK_EQUAL(0, strncpy_s(buffer, 10, "correct", 8));
	CHECK_EQUAL("correct", buffer);
}

TEST(STRCAT_S)
{
	char buffer[42];

	buffer[0] = '\0';

	CHECK_EQUAL(EINVAL, strcat_s(NULL, 42, "NULL dest"));
	CHECK_EQUAL(EINVAL, strcat_s(buffer, 42, NULL));
	CHECK_EQUAL(ERANGE, strcat_s(buffer, 10, "longer than given size"));
	CHECK_EQUAL(0, strcat_s(buffer, 42, "hello"));
	CHECK_EQUAL(0, strcat_s(buffer, 42, " world"));
	CHECK_EQUAL("hello world", buffer);
}

TEST(STRCASESTR)
{
	const char *text = "HELLO world";

	CHECK(NULL == strcasestr(text, "not found"));
	CHECK(text == strcasestr(text, "HelLo"));
	CHECK(text + 6 == strcasestr(text, "WoRlD"));
	CHECK(text + 4 == strcasestr(text, "o"));
}

TEST(STRNLEN)
{
	CHECK_EQUAL(2u, strnlen("hello", 2));
	CHECK_EQUAL(5u, strnlen("hello", 5));
	CHECK_EQUAL(5u, strnlen("hello", 10));
}

// TODO: int getopt(int argc, char *const argv[], const char *optstring);
// TODO: int GetClipboard(std::string &str);
// TODO: int UTF8GetNext(const std::string &text, int curpos);
// TODO: int UTF8GetPrev(const std::string &text, int curpos);
