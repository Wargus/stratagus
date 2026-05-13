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

#include <doctest.h>

#include "action/action_resource.h"

#include "actions.h"
#include "stratagus.h"
#include "unit.h"

struct ResourceOrderTestAccess
{
	static void DropResource(COrder_Resource &order, CUnit &unit)
	{
		order.DropResource(unit);
	}

	static CUnit *Mine(COrder_Resource &order)
	{
		return order.Resource.Mine;
	}
};

namespace {

void PrepareUnit(CUnit &unit, CUnitType &type, CPlayer &player)
{
	unit.Type = &type;
	unit.Player = &player;
	unit.Refs = 1;
	unit.Orders.push_back(COrder::NewActionStill());
}

}

TEST_CASE("Resource order destructor deassigns live mine workers")
{
	CPlayer player;
	player.Index = 0;
	CUnitType workerType;
	CUnitType mineType;
	mineType.TileWidth = 1;
	mineType.TileHeight = 1;
	mineType.GivesResource = GoldCost;
	CUnit worker;
	CUnit mine;
	PrepareUnit(worker, workerType, player);
	PrepareUnit(mine, mineType, player);

	{
		auto order = COrder::NewActionResource(worker, mine);
		worker.AssignWorkerToMine(mine);

		REQUIRE(mine.Resource.AssignedWorkers.size() == 1);
		CHECK(mine.Resource.AssignedWorkers.front() == &worker);
		CHECK(worker.Refs == 3);
		CHECK(mine.Refs == 3);
	}

	CHECK(mine.Resource.AssignedWorkers.empty());
	CHECK(worker.Refs == 1);
	CHECK(mine.Refs == 1);
}

TEST_CASE("Dropping carried resources clears mine assignment")
{
	CPlayer player;
	player.Index = 0;
	CUnitType workerType;
	CUnitType mineType;
	auto goldInfo = std::make_unique<ResourceInfo>();
	goldInfo->TerrainHarvester = false;
	workerType.ResInfo[GoldCost] = std::move(goldInfo);
	mineType.TileWidth = 1;
	mineType.TileHeight = 1;
	mineType.GivesResource = GoldCost;
	CUnit worker;
	CUnit mine;
	PrepareUnit(worker, workerType, player);
	PrepareUnit(mine, mineType, player);

	auto baseOrder = COrder::NewActionResource(worker, mine);
	auto &order = static_cast<COrder_Resource &>(*baseOrder);
	worker.AssignWorkerToMine(mine);
	worker.CurrentResource = GoldCost;
	worker.ResourcesHeld = 100;

	ResourceOrderTestAccess::DropResource(order, worker);

	CHECK(worker.CurrentResource == 0);
	CHECK(worker.ResourcesHeld == 0);
	CHECK(mine.Resource.AssignedWorkers.empty());
	CHECK(ResourceOrderTestAccess::Mine(order) == nullptr);
	CHECK(worker.Refs == 2);
	CHECK(mine.Refs == 2);
}
