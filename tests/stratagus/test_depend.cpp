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
/**@name test_depend.cpp - The test file for depend.cpp. */
//
//      (c) Copyright 2023 by Joris Dauphin
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

#include "depend.h"
#include "interface.h"
#include "player.h"
#include "script.h"
#include "translate.h"
#include "unittype.h"
#include "unit_manager.h"
#include "unit.h"

namespace
{
void InitLua_Depend(std::string defineDependency)
{
	InitLua();
	CHECK(Lua);
	UnitTypeCclRegister();
	DependenciesCclRegister();

	const std::string_view preamble = R"(
DefineUnitType("unit-main", {Name="unitMain"})
CUpgrade:New("upgrade-main").Name = "upgradeMain"

DefineUnitType("unit-dep1", {Name="unit1"})
CUpgrade:New("upgrade-dep1").Name = "upgrade1"
)";
	const std::string content = preamble.data() + defineDependency;
	const int status = luaL_loadbuffer(Lua, content.data(), content.size(), "test");
	CHECK(status == 0);
	LuaCall(Lua, 0, 0, lua_gettop(Lua), false);
	lua_close(Lua);
	Lua = nullptr;
}
}

TEST_CASE("Unit depends")
{
	const auto requirements = std::string(_("Requirements:\n"));
	CPlayer player{};
	ButtonAction button;
	button.ValueStr = "unit-main";

	SUBCASE("with unit")
	{
		InitLua_Depend(R"(DefineDependency("unit-main", {"unit-dep1"}))");
		const auto main_id = UnitTypeByIdent("unit-main").Slot;
		const auto dep_id = UnitTypeByIdent("unit-dep1").Slot;
		REQUIRE(player.UnitTypesCount[dep_id] == 0);
		player.Allow.Units[main_id] = 42;

		CHECK(PrintDependencies(player, button) == requirements + "-unit1\n");
		CHECK(!CheckDependByIdent(player, "unit-main"));

		player.UnitTypesCount[dep_id] = 1;

		CHECK(PrintDependencies(player, button) == "");
		CHECK(CheckDependByIdent(player, "unit-main"));

		player.Allow.Units[main_id] = 0;
		CHECK(!CheckDependByIdent(player, "unit-main"));
	}
	SUBCASE("with 4 units")
	{
		InitLua_Depend(R"(DefineDependency("unit-main", {"unit-dep1", 4}))");
		const auto main_id = UnitTypeByIdent("unit-main").Slot;
		const auto dep_id = UnitTypeByIdent("unit-dep1").Slot;
		player.Allow.Units[main_id] = 42;
		player.UnitTypesCount[dep_id] = 3;

		CHECK(PrintDependencies(player, button) == requirements + "-unit1\n");
		CHECK(!CheckDependByIdent(player, "unit-main"));

		player.UnitTypesCount[dep_id] = 4;

		CHECK(PrintDependencies(player, button) == "");
		CHECK(CheckDependByIdent(player, "unit-main"));

		player.Allow.Units[main_id] = 0;
		CHECK(!CheckDependByIdent(player, "unit-main"));
	}
	SUBCASE("with upgrade")
	{
		InitLua_Depend(R"(DefineDependency("unit-main", {"upgrade-dep1"}))");
		const auto main_id = UnitTypeByIdent("unit-main").Slot;
		player.Allow.Units[main_id] = 42;

		CHECK(PrintDependencies(player, button) == requirements + "-upgrade1\n");
		CHECK(!CheckDependByIdent(player, "unit-main"));

		player.Allow.Upgrades[CUpgrade::Get("upgrade-dep1")->ID] = 'R';

		CHECK(PrintDependencies(player, button) == "");
		CHECK(CheckDependByIdent(player, "unit-main"));
	}

	SUBCASE("without upgrade")
	{
		InitLua_Depend(R"(DefineDependency("unit-main", {"upgrade-dep1", 0}))");
		const auto main_id = UnitTypeByIdent("unit-main").Slot;
		player.Allow.Units[main_id] = 42;

		CHECK(PrintDependencies(player, button) == "");
		CHECK(CheckDependByIdent(player, "unit-main"));

		player.Allow.Upgrades[CUpgrade::Get("upgrade-dep1")->ID] = 'R';

		CHECK(PrintDependencies(player, button) == requirements + "-upgrade1\n"); // Strange display
		CHECK(!CheckDependByIdent(player, "unit-main"));
	}

	SUBCASE("with unit and upgrade")
	{
		InitLua_Depend(R"(DefineDependency("unit-main", {"unit-dep1", "upgrade-dep1"}))");
		const auto main_id = UnitTypeByIdent("unit-main").Slot;
		const auto dep_id = UnitTypeByIdent("unit-dep1").Slot;
		player.Allow.Units[main_id] = 42;

		const auto both_required = PrintDependencies(player, button);
		CHECK_MESSAGE((both_required == requirements + "-upgrade1\n" + "-unit1\n"
		               || both_required == requirements + "-unit1\n" + "-upgrade1\n"),
		              "With value =",
		              both_required);
		CHECK(!CheckDependByIdent(player, "unit-main"));

		player.UnitTypesCount[dep_id] = 1;

		CHECK(PrintDependencies(player, button) == requirements + "-upgrade1\n");
		CHECK(!CheckDependByIdent(player, "unit-main"));

		player.Allow.Upgrades[CUpgrade::Get("upgrade-dep1")->ID] = 'R';

		CHECK(PrintDependencies(player, button) == "");
		CHECK(CheckDependByIdent(player, "unit-main"));

		player.UnitTypesCount[dep_id] = 0;

		CHECK(PrintDependencies(player, button) == requirements + "-unit1\n");
		CHECK(!CheckDependByIdent(player, "unit-main"));
	}

	SUBCASE("with unit or upgrade")
	{
		InitLua_Depend(R"(DefineDependency("unit-main", {"unit-dep1"}, "or", {"upgrade-dep1"}))");
		const auto main_id = UnitTypeByIdent("unit-main").Slot;
		const auto dep_id = UnitTypeByIdent("unit-dep1").Slot;
		player.Allow.Units[main_id] = 42;

		const auto either_required = PrintDependencies(player, button);
		CHECK_MESSAGE((either_required == requirements + "-upgrade1\n"
		               || either_required == requirements + "-unit1\n"),
		              "With value =",
		              either_required);
		CHECK(!CheckDependByIdent(player, "unit-main"));

		player.UnitTypesCount[dep_id] = 1;

		CHECK(PrintDependencies(player, button) == "");
		CHECK(CheckDependByIdent(player, "unit-main"));

		player.Allow.Upgrades[CUpgrade::Get("upgrade-dep1")->ID] = 'R';

		CHECK(PrintDependencies(player, button) == "");
		CHECK(CheckDependByIdent(player, "unit-main"));

		player.UnitTypesCount[dep_id] = 0;

		CHECK(PrintDependencies(player, button) == "");
		CHECK(CheckDependByIdent(player, "unit-main"));
	}


	CleanDependencies();
}

TEST_CASE("upgrade depends")
{
	const auto requirements = std::string(_("Requirements:\n"));
	CPlayer player{};
	ButtonAction button;
	button.ValueStr = "upgrade-main";

	SUBCASE("with unit")
	{
		InitLua_Depend(R"(DefineDependency("upgrade-main", {"unit-dep1"}))");
		const auto main_id = CUpgrade::Get("upgrade-main")->ID;
		player.Allow.Upgrades[main_id] = 'A';
		const auto dep_id = UnitTypeByIdent("unit-dep1").Slot;
		REQUIRE(player.UnitTypesCount[dep_id] == 0);

		CHECK(PrintDependencies(player, button) == requirements + "-unit1\n");
		CHECK(!CheckDependByIdent(player, "upgrade-main"));

		player.UnitTypesCount[dep_id] = 1;

		CHECK(PrintDependencies(player, button) == "");
		CHECK(CheckDependByIdent(player, "upgrade-main"));

		player.Allow.Upgrades[main_id] = '\0';
		CHECK(!CheckDependByIdent(player, "upgrade-main"));
	}
	SUBCASE("with 4 units")
	{
		InitLua_Depend(R"(DefineDependency("upgrade-main", {"unit-dep1", 4}))");
		const auto main_id = CUpgrade::Get("upgrade-main")->ID;
		player.Allow.Upgrades[main_id] = 'A';
		const auto dep_id = UnitTypeByIdent("unit-dep1").Slot;
		player.UnitTypesCount[dep_id] = 3;

		CHECK(PrintDependencies(player, button) == requirements + "-unit1\n");
		CHECK(!CheckDependByIdent(player, "upgrade-main"));

		player.UnitTypesCount[dep_id] = 4;

		CHECK(PrintDependencies(player, button) == "");
		CHECK(CheckDependByIdent(player, "upgrade-main"));

		player.Allow.Upgrades[main_id] = '\0';
		CHECK(!CheckDependByIdent(player, "upgrade-main"));
	}
	SUBCASE("with upgrade")
	{
		InitLua_Depend(R"(DefineDependency("upgrade-main", {"upgrade-dep1"}))");
		const auto main_id = CUpgrade::Get("upgrade-main")->ID;
		player.Allow.Upgrades[main_id] = 'A';

		CHECK(PrintDependencies(player, button) == requirements + "-upgrade1\n");
		CHECK(!CheckDependByIdent(player, "upgrade-main"));

		player.Allow.Upgrades[CUpgrade::Get("upgrade-dep1")->ID] = 'R';

		CHECK(PrintDependencies(player, button) == "");
		CHECK(CheckDependByIdent(player, "upgrade-main"));
	}

	SUBCASE("without upgrade")
	{
		InitLua_Depend(R"(DefineDependency("upgrade-main", {"upgrade-dep1", 0}))");
		const auto main_id = CUpgrade::Get("upgrade-main")->ID;
		player.Allow.Upgrades[main_id] = 'A';

		CHECK(PrintDependencies(player, button) == "");
		CHECK(CheckDependByIdent(player, "upgrade-main"));

		player.Allow.Upgrades[CUpgrade::Get("upgrade-dep1")->ID] = 'R';

		CHECK(PrintDependencies(player, button) == requirements + "-upgrade1\n"); // Strange display
		CHECK(!CheckDependByIdent(player, "upgrade-main"));
	}

	SUBCASE("with unit and upgrade")
	{
		InitLua_Depend(R"(DefineDependency("upgrade-main", {"unit-dep1", "upgrade-dep1"}))");
		const auto main_id = CUpgrade::Get("upgrade-main")->ID;
		player.Allow.Upgrades[main_id] = 'A';
		const auto dep_id = UnitTypeByIdent("unit-dep1").Slot;

		const auto both_required = PrintDependencies(player, button);
		CHECK_MESSAGE((both_required == requirements + "-upgrade1\n" + "-unit1\n"
		               || both_required == requirements + "-unit1\n" + "-upgrade1\n"),
		              "With value =",
		              both_required);
		CHECK(!CheckDependByIdent(player, "upgrade-main"));

		player.UnitTypesCount[dep_id] = 1;

		CHECK(PrintDependencies(player, button) == requirements + "-upgrade1\n");
		CHECK(!CheckDependByIdent(player, "upgrade-main"));

		player.Allow.Upgrades[CUpgrade::Get("upgrade-dep1")->ID] = 'R';

		CHECK(PrintDependencies(player, button) == "");
		CHECK(CheckDependByIdent(player, "upgrade-main"));

		player.UnitTypesCount[dep_id] = 0;

		CHECK(PrintDependencies(player, button) == requirements + "-unit1\n");
		CHECK(!CheckDependByIdent(player, "upgrade-main"));
	}

	SUBCASE("with unit or upgrade")
	{
		InitLua_Depend(R"(DefineDependency("upgrade-main", {"unit-dep1"}, "or", {"upgrade-dep1"}))");
		const auto main_id = CUpgrade::Get("upgrade-main")->ID;
		player.Allow.Upgrades[main_id] = 'A';
		const auto dep_id = UnitTypeByIdent("unit-dep1").Slot;

		const auto either_required = PrintDependencies(player, button);
		CHECK_MESSAGE((either_required == requirements + "-upgrade1\n"
		               || either_required == requirements + "-unit1\n"),
		              "With value =",
		              either_required);
		CHECK(!CheckDependByIdent(player, "upgrade-main"));

		player.UnitTypesCount[dep_id] = 1;

		CHECK(PrintDependencies(player, button) == "");
		CHECK(CheckDependByIdent(player, "upgrade-main"));

		player.Allow.Upgrades[CUpgrade::Get("upgrade-dep1")->ID] = 'R';

		CHECK(PrintDependencies(player, button) == "");
		CHECK(CheckDependByIdent(player, "upgrade-main"));

		player.UnitTypesCount[dep_id] = 0;

		CHECK(PrintDependencies(player, button) == "");
		CHECK(CheckDependByIdent(player, "upgrade-main"));
	}

	CleanDependencies();
}

TEST_CASE("spell button depend")
{
	const auto requirements = std::string(_("Requirements:\n"));
	CPlayer player{};
	ButtonAction button;

	button.ValueStr = "spell-dummy";
	button.Allowed = [](const auto &, const auto &) { return false; };
	button.AllowStr = "upgrade-main";
	CUnit unit;
	Selected.push_back(&unit);

	CHECK(PrintDependencies(player, button) == requirements + "-upgradeMain\n");

	Selected.clear();
}
