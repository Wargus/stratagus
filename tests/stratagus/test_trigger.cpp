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
/**@name test_trigger.cpp - The test file for trigger.cpp. */
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

#include "trigger.h"
#include "script.h"

namespace
{
[[nodiscard]] auto InitLuaTrigger(std::string content)
{
	InitLua();
	CHECK(Lua);
	CleanTriggers();
	TriggerCclRegister();

	const std::string_view preamble = R"(
		log = ""
		function l(s) log = log .. s .. " " end
)";

	const std::string content2 = preamble.data() + content;

	const int status = luaL_loadbuffer(Lua, content2.data(), content2.size(), "test");
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

std::string_view test_getLuaGlobalStr(const char* varname)
{
	lua_getglobal(Lua, varname);
	std::string_view value = LuaToString(Lua, -1);
	lua_pop(Lua, 1);
	return value;
}

void test_setLuaGlobalStr(const char* varname, const char* value)
{
	lua_pushstring(Lua, value);
	lua_setglobal(Lua, varname);
}

int test_getLuaTableSize(const char* varname)
{
	lua_getglobal(Lua, varname);
	REQUIRE(lua_istable(Lua, -1));
	int size = lua_rawlen(Lua, -1);
	lua_pop(Lua, 1);
	return size;
}

} // namespace


TEST_CASE("Trigger None")
{
	const auto raii = InitLuaTrigger(R"()");

	TriggersEachCycle();
}

TEST_CASE("Trigger One")
{
	const auto raii = InitLuaTrigger(R"(
		AddTrigger(function() return false end, function() return false end)
	)");
	CHECK(test_getLuaTableSize("_triggers_") == 2);

	TriggersEachCycle();
}

TEST_CASE("Trigger Basic")
{
	const auto raii = InitLuaTrigger(R"(
		AddTrigger(function() l("C1") return false end, function() l("A1") return false end)
		AddTrigger(function() l("C2") return true end, function() l("A2") return false end)
		AddTrigger(function() l("C3") return true end, function() l("A3") return true end)
		AddTrigger(function() l("C4") return false end, function() l("A4") return true end)
	)");
	CHECK(test_getLuaTableSize("_triggers_") == 4 * 2);

	test_setLuaGlobalStr("log", "");
	TriggersEachCycle();
	CHECK(test_getLuaGlobalStr("log") == "C1 C2 A2 C3 A3 C4 ");

	test_setLuaGlobalStr("log", "");
	TriggersEachCycle();
	CHECK(test_getLuaGlobalStr("log") == "C1 C3 A3 C4 ");

	test_setLuaGlobalStr("log", "");
	TriggersEachCycle();
	CHECK(test_getLuaGlobalStr("log") == "C1 C3 A3 C4 ");
}

TEST_CASE("Trigger SetActiveTriggers")
{
	const auto raii = InitLuaTrigger(R"(
		AddTrigger(function() l("C1") return false end, function() l("A1") return false end)
		AddTrigger(function() l("C2") return false end, function() l("A2") return false end)
		SetActiveTriggers(false, true, false, true)
		AddTrigger(function() l("C3") return false end, function() l("A3") return false end)
		AddTrigger(function() l("C4") return false end, function() l("A4") return false end)
		AddTrigger(function() l("C5") return false end, function() l("A5") return false end)
		AddTrigger(function() l("C6") return false end, function() l("A6") return false end)
	)");
	CHECK(test_getLuaTableSize("_triggers_") == 6 * 2);

	test_setLuaGlobalStr("log", "");
	TriggersEachCycle();
	CHECK(test_getLuaGlobalStr("log") == "C2 C4 C5 C6 ");
}

TEST_CASE("Trigger Pause")
{
	const auto raii = InitLuaTrigger(R"(
		AddTrigger(function() l("C1") return false end, function() l("A1") return ActionVictory() end)
		AddTrigger(function() l("C2") return true end, function() l("A2") return ActionVictory() end)
		AddTrigger(function() l("C3") return false end, function() l("A3") return ActionVictory() end)
	)");
	CHECK(test_getLuaTableSize("_triggers_") == 3 * 2);

	test_setLuaGlobalStr("log", "");
	TriggersEachCycle();
	// do not run check C3
	CHECK(test_getLuaGlobalStr("log") == "C1 C2 A2 ");
}
