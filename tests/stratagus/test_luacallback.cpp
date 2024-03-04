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
//      (c) Copyright 2024 by Joris Dauphin
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

#include "luacallback.h"
#include "script.h"

namespace doctest
{
template <typename... Ts>
struct StringMaker<std::tuple<Ts...>>
{
	static String convert(const std::tuple<Ts...> &value)
	{
		String res = "{";
		String sep = "";
		std::apply([&](const auto &...args) { ((res += sep + toString(args), sep = ", "), ...); },
		           value);
		res += "}";
		return res;
	}
};
} // namespace doctest

namespace
{
[[nodiscard]] auto InitLuaCallback(std::string content)
{
	InitLua();
	CHECK(Lua);

	const int status = luaL_loadbuffer(Lua, content.data(), content.size(), "test");
	CHECK(status == 0);
	LuaCall(Lua, 0, 0, lua_gettop(Lua), false);

	struct S
	{
		S() {}
		S(const S &) = delete;
		~S()
		{
			if (Lua) {
				lua_close(Lua);
				Lua = nullptr;
			}
		}
	};
	return S(); // copy-elision
}
} // namespace

TEST_CASE("LuaCallback_simplecall")
{
	const auto raii = InitLuaCallback(R"(function f(x) return x end)");

	lua_getglobal(Lua, "f");
	LuaCallback f(Lua, -1);

	REQUIRE(f.call<int>(42) == 42);
}

TEST_CASE("LuaCallback_complexcall")
{
	const auto raii = InitLuaCallback(R"(function f(x, s, v) return #s, x == 42, v[3], v[4] end)");

	lua_getglobal(Lua, "f");
	LuaCallback f(Lua, -1);

	auto t = f.call<int, bool, int, int>(42, "toto", std::vector{11, 12, 13, 14});
	REQUIRE(t == std::tuple{4, true, 13, 14});
}

TEST_CASE("LuaCallback_copy")
{
	const auto raii = InitLuaCallback(R"(function f(x) return x end)");

	lua_getglobal(Lua, "f");
	LuaCallback f(Lua, -1);

	{
		LuaCallback g(f);
		REQUIRE(f.call<int>(42) == 42);
		REQUIRE(g.call<int>(42) == 42);
	} // g destructor

	REQUIRE(f.call<int>(42) == 42);
}

TEST_CASE("LuaCallback_assign")
{
	const auto raii = InitLuaCallback(R"(function f(x) return x end function g(x) return x + 1 end)");

	lua_getglobal(Lua, "f");
	LuaCallback f(Lua, -1);

	{
		lua_getglobal(Lua, "g");
		LuaCallback g(Lua, -1);
		REQUIRE(f.call<int>(42) == 42);
		REQUIRE(g.call<int>(41) == 42);
		f = g;
		REQUIRE(f.call<int>(41) == 42);
		REQUIRE(g.call<int>(41) == 42);
	} // g destructor

	REQUIRE(f.call<int>(41) == 42);
}
