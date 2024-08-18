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
/**@name test_pathfinder.cpp - The test file for pathfinder.cpp. */
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

#include "action/action_move.h"
#include "actions.h"
#include "map.h"
#include "pathfinder.h"
#include "stratagus.h"
#include "unit.h"
#include "unittype.h"

namespace doctest
{
template <typename T>
struct StringMaker<Vec2T<T>>
{
	static String convert(const Vec2T<T> &value)
	{
		String res = "{";
		res += toString(value.x);
		res += ", ";
		res += toString(value.y);
		res += "}";
		return res;
	}
};
} // namespace doctest

namespace
{
Vec2i FollowedPath(const Vec2i &origin, const PathFinderOutput &data)
{
	Vec2i res = origin;
	for (int i = 0; i != data.Length; ++i) {
		const auto direction = data.Path[data.Length - 1 - i];
		res.x += Heading2X[direction];
		res.y += Heading2Y[direction];
	}
	return res;
}
} // namespace

TEST_CASE("PathFinding on clear map 128x128")
{
	CPlayer player;
	player.Index = 0;
	CUnitType type;
	type.TileWidth = 2;
	type.TileHeight = 2;
	type.BoolFlag.resize(UnitTypeVar.GetNumberBoolFlag()); // SOLID_INDEX
	CUnit unit;
	unit.Player = &player;
	unit.Type = &type;
	unit.tilePos = {42, 1};

	Map.Info.MapWidth = 128;
	Map.Info.MapHeight = 128;

	Map.Create();

	extern void InitAStar(int mapWidth, int mapHeight);
	InitAStar(Map.Info.MapWidth, Map.Info.MapHeight);

	SUBCASE("Already reached")
	{
		const auto dest = unit.tilePos;
		unit.Orders.push_back(COrder::NewActionMove(dest));

		const auto [d, dir] = NextPathElement(unit);

		CHECK(d == PF_REACHED);
		CHECK(unit.pathFinderData->output.Length == 0);
		CHECK(dir == Vec2i(0, 0));
		CHECK(dest == FollowedPath(unit.tilePos + dir, unit.pathFinderData->output));

		unit.Orders.clear();
	}

	SUBCASE("short path (10)")
	{
		const short dist = 10;
		REQUIRE(dist < std::size(unit.pathFinderData->output.Path));
		const auto dest = unit.tilePos + Vec2i{0, dist};
		unit.Orders.push_back(COrder::NewActionMove(dest));

		const auto [d, dir] = NextPathElement(unit);

		CHECK(d == dist);

		CHECK(unit.pathFinderData->output.Length == dist);
		CHECK(dest == FollowedPath(unit.tilePos, unit.pathFinderData->output));

		unit.Orders.clear();
	}

	SUBCASE("long path (30)")
	{
		const short dist = 30;
		REQUIRE(30 > std::size(unit.pathFinderData->output.Path));
		const auto dest = unit.tilePos + Vec2i{0, dist};
		unit.Orders.push_back(COrder::NewActionMove(dest));

		const auto [d, dir] = NextPathElement(unit);

		CHECK(0 < d);
		CHECK(d <= std::size(unit.pathFinderData->output.Path));
		CHECK(unit.pathFinderData->output.Length + unit.pathFinderData->output.OverflowLength == dist);

		CHECK(unit.pathFinderData->output.Length == d);
		CHECK(unit.tilePos + Vec2i(0, d) == FollowedPath(unit.tilePos, unit.pathFinderData->output));

		unit.Orders.clear();
	}

	Map.Fields.clear();

	extern void FreeAStar(); // free the a* data structures
	FreeAStar();
}
