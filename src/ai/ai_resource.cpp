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
/**@name ai_resource.cpp - AI resource manager. */
//
//      (c) Copyright 2000-2005 by Lutz Sammer and Antonis Chaniotis.
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

//@{

/*----------------------------------------------------------------------------
--  Includes
----------------------------------------------------------------------------*/

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "stratagus.h"

#include "ai_local.h"

#include "action/action_build.h"
#include "action/action_repair.h"
#include "action/action_resource.h"
#include "depend.h"
#include "map.h"
#include "pathfinder.h"
#include "player.h"
#include "unit.h"
#include "unittype.h"
#include "upgrade.h"

/*----------------------------------------------------------------------------
--  Defines
----------------------------------------------------------------------------*/

#define COLLECT_RESOURCES_INTERVAL 4

/*----------------------------------------------------------------------------
--  Functions
----------------------------------------------------------------------------*/

static int AiMakeUnit(CUnitType &type, const Vec2i &nearPos);

/**
**  Check if the costs are available for the AI.
**
**  Take reserve and already used resources into account.
**
**  @param costs  Costs for something.
**
**  @return       A bit field of the missing costs.
*/
static int AiCheckCosts(const int *costs)
{
	// FIXME: the used costs shouldn't be calculated here
	int *used = AiPlayer->Used;

	for (int i = 1; i < MaxCosts; ++i) {
		used[i] = 0;
	}

	const int nunits = AiPlayer->Player->GetUnitCount();
	for (int i = 0; i < nunits; ++i) {
		CUnit &unit = AiPlayer->Player->GetUnit(i);

		for (size_t k = 0; k < unit.Orders.size(); ++k) {
			const COrder &order = *unit.Orders[k];

			if (order.Action == UnitActionBuild) {
				const COrder_Build &orderBuild = static_cast<const COrder_Build &>(order);
				const int *building_costs = orderBuild.GetUnitType().Stats[AiPlayer->Player->Index].Costs;

				for (int j = 1; j < MaxCosts; ++j) {
					used[j] += building_costs[j];
				}
			}
		}
	}

	int err = 0;
	const int *resources = AiPlayer->Player->Resources;
	const int *storedresources = AiPlayer->Player->StoredResources;
	const int *reserve = AiPlayer->Reserve;
	for (int i = 1; i < MaxCosts; ++i) {
		if (resources[i] + storedresources[i] - used[i] < costs[i] - reserve[i]) {
			err |= 1 << i;
		}
	}
	return err;
}

/**
**  Check if the AI player needs food.
**
**  It counts buildings in progress and units in training queues.
**
**  @param pai   AI player.
**  @param type  Unit-type that should be build.
**
**  @return      True if enought, false otherwise.
**
**  @todo  The number of food currently trained can be stored global
**         for faster use.
*/
static int AiCheckSupply(const PlayerAi &pai, const CUnitType &type)
{
	// Count food supplies under construction.
	int remaining = 0;
	for (unsigned int i = 0; i < pai.UnitTypeBuilt.size(); ++i) {
		const AiBuildQueue &queue = pai.UnitTypeBuilt[i];
		if (queue.Type->Supply) {
			remaining += queue.Made * queue.Type->Supply;
		}
	}

	// We are already out of food.
	remaining += pai.Player->Supply - pai.Player->Demand - type.Demand;
	if (remaining < 0) {
		return 0;
	}
	// Count what we train.
	for (unsigned int i = 0; i < pai.UnitTypeBuilt.size(); ++i) {
		const AiBuildQueue &queue = pai.UnitTypeBuilt[i];

		remaining -= queue.Made * queue.Type->Demand;
		if (remaining < 0) {
			return 0;
		}
	}
	return 1;
}

/**
**  Check if the costs for an unit-type are available for the AI.
**
**  Take reserve and already used resources into account.
**
**  @param type  Unit-type to check the costs for.
**
**  @return      A bit field of the missing costs.
*/
static int AiCheckUnitTypeCosts(const CUnitType &type)
{
	return AiCheckCosts(type.Stats[AiPlayer->Player->Index].Costs);
}

class IsAEnemyUnitOf
{
public:
	explicit IsAEnemyUnitOf(const CPlayer &_player) : player(&_player) {}
	bool operator()(const CUnit *unit) const {
		return unit->IsVisibleAsGoal(*player) && unit->IsEnemy(*player);
	}
private:
	const CPlayer *player;
};

class IsAEnemyUnitWhichCanCounterAttackOf
{
public:
	explicit IsAEnemyUnitWhichCanCounterAttackOf(const CPlayer &_player, const CUnitType &_type) :
		player(&_player), type(&_type)
	{}
	bool operator()(const CUnit *unit) const {
		return unit->IsVisibleAsGoal(*player)
			   && unit->IsEnemy(*player)
			   && CanTarget(unit->Type, type);
	}
private:
	const CPlayer *player;
	const CUnitType *type;
};

/**
**  Enemy units in distance.
**
**  @param player  Find enemies of this player
**  @param type    Optional unit type to check if enemy can target this
**  @param pos     location
**  @param range   Distance range to look.
**
**  @return       Number of enemy units.
*/
int AiEnemyUnitsInDistance(const CPlayer &player,
						   const CUnitType *type, const Vec2i &pos, unsigned range)
{
	const Vec2i offset = {range, range};
	std::vector<CUnit *> units;

	if (type == NULL) {
		Map.Select(pos - offset, pos + offset, units, IsAEnemyUnitOf(player));
		return static_cast<int>(units.size());
	} else {
		const Vec2i typeSize = {type->TileWidth - 1, type->TileHeight - 1};
		const IsAEnemyUnitWhichCanCounterAttackOf pred(player, *type);

		Map.Select(pos - offset, pos + typeSize + offset, units, pred);
		return static_cast<int>(units.size());
	}
}

/**
**  Enemy units in distance.
**
**  @param unit   Find in distance for this unit.
**  @param range  Distance range to look.
**
**  @return       Number of enemy units.
*/
int AiEnemyUnitsInDistance(const CUnit &unit, unsigned range)
{
	return AiEnemyUnitsInDistance(*unit.Player, unit.Type, unit.tilePos, range);
}

static bool IsAlreadyWorking(const CUnit &unit)
{
	for (size_t i = 0; i != unit.Orders.size(); ++i) {
		const int action = unit.Orders[i]->Action;

		if (action == UnitActionBuild || action == UnitActionRepair) {
			return true;
		}
		if (action == UnitActionResource) {
			const COrder_Resource &order = *static_cast<const COrder_Resource *>(unit.Orders[i]);

			if (order.IsGatheringStarted()) {
				return true;
			}
		}
	}
	return false;
}


/**
**  Check if we can build the building.
**
**  @param type      Unit that can build the building.
**  @param building  Building to be build.
**
**  @return          True if made, false if can't be made.
**
**  @note            We must check if the dependencies are fulfilled.
*/
static int AiBuildBuilding(const CUnitType &type, CUnitType &building, const Vec2i &nearPos)
{
	std::vector<CUnit *> table;

	FindPlayerUnitsByType(*AiPlayer->Player, type, table);

	int num = 0;

	// Remove all workers on the way building something
	for (size_t i = 0; i != table.size(); ++i) {
		CUnit &unit = *table[i];

		if (IsAlreadyWorking(unit) == false) {
			table[num++] = &unit;
		}
	}
	if (num == 0) {
		// No workers available to build
		return 0;
	}

	CUnit &unit = (num == 1) ? *table[0] : *table[SyncRand() % num];

	Vec2i pos;
	// Find a place to build.
	if (AiFindBuildingPlace(unit, building, nearPos, &pos)) {
		CommandBuildBuilding(unit, pos, building, FlushCommands);
		return 1;
	} else {
		//when first worker can't build then rest also won't be able (save CPU)
		if (Map.Info.IsPointOnMap(nearPos)) {
			//Crush CPU !!!!!
			for (int i = 0; i < num && table[i] != &unit; ++i) {
				// Find a place to build.
				if (AiFindBuildingPlace(*table[i], building, nearPos, &pos)) {
					CommandBuildBuilding(*table[i], pos, building, FlushCommands);
					return 1;
				}
			}
		}
	}
	return 0;
}

static bool AiRequestedTypeAllowed(const CPlayer &player, const CUnitType &type)
{
	const size_t size = AiHelpers.Build[type.Slot].size();
	for (size_t i = 0; i != size; ++i) {
		CUnitType &builder = *AiHelpers.Build[type.Slot][i];

		if (player.UnitTypesCount[builder.Slot] > 0
			&& CheckDependByType(player, type)) {
			return true;
		}
	}
	return false;
}

struct cnode {
	int unit_cost;
	int needmask;
	CUnitType *type;
};

static bool cnode_cmp(const cnode &lhs, const cnode &rhs)
{
	return lhs.unit_cost < rhs.unit_cost;
}

int AiGetBuildRequestsCount(const PlayerAi &pai, int (&counter)[UnitTypeMax])
{
	const int size = (int)pai.UnitTypeBuilt.size();
	memset(counter, 0, sizeof(int) * UnitTypeMax);
	for (int i = 0; i < size; ++i) {
		const AiBuildQueue &queue = pai.UnitTypeBuilt[i];

		counter[queue.Type->Slot] += queue.Want;
	}
	return size;
}

extern CUnit *FindDepositNearLoc(CPlayer &p, const Vec2i &pos, int range, int resource);


void AiNewDepotRequest(CUnit &worker)
{
#if 0
	DebugPrint("%d: Worker %d report: Resource [%d] too far from depot, returning time [%d].\n"
			   _C_ worker->Player->Index _C_ worker->Slot
			   _C_ worker->CurrentResource
			   _C_ worker->Data.Move.Cycles);
#endif
	Assert(worker.CurrentAction() == UnitActionResource);
	COrder_Resource &order = *static_cast<COrder_Resource *>(worker.CurrentOrder());

	const Vec2i pos = order.GetHarvestLocation();

	if (pos.x != -1 && NULL != FindDepositNearLoc(*worker.Player, pos, 10, worker.CurrentResource)) {
		/*
		 * New Depot has just be finished and worker just return to old depot
		 * (far away) from new Deopt.
		 */
		return;
	}
	CUnitType *best_type = NULL;
	int best_cost = 0;
	//int best_mask = 0;
	// Count the already made build requests.
	int counter[UnitTypeMax];

	AiGetBuildRequestsCount(*worker.Player->Ai, counter);

	const int n = AiHelpers.Depots[worker.CurrentResource - 1].size();

	for (int i = 0; i < n; ++i) {
		CUnitType &type = *AiHelpers.Depots[worker.CurrentResource - 1][i];

		if (counter[type.Slot]) { // Already ordered.
			return;
		}
		if (!AiRequestedTypeAllowed(*worker.Player, type)) {
			continue;
		}

		// Check if resources available.
		//int needmask = AiCheckUnitTypeCosts(type);
		int cost = 0;
		for (int c = 1; c < MaxCosts; ++c) {
			cost += type.Stats[worker.Player->Index].Costs[c];
		}

		if (best_type == NULL || (cost < best_cost)) {
			best_type = &type;
			best_cost = cost;
			//best_mask = needmask;
		}
	}

	if (best_type) {
		//if(!best_mask) {
		AiBuildQueue queue;

		queue.Type = best_type;
		queue.Want = 1;
		queue.Made = 0;
		queue.Pos = pos;

		worker.Player->Ai->UnitTypeBuilt.push_back(queue);

		DebugPrint("%d: Worker %d report: Requesting new depot near [%d,%d].\n"
				   _C_ worker.Player->Index _C_ worker.Slot
				   _C_ queue.Pos.x _C_ queue.Pos.y);
		/*
		} else {
			AiPlayer->NeededMask |= best_mask;
		}
		*/
	}
}

/**
**  Build new units to reduce the food shortage.
*/
static bool AiRequestSupply()
{
	// Don't request supply if we're sleeping.  When the script starts it may
	// request a better unit than the one we pick here.  If we only have enough
	// resources for one unit we don't want to build the wrong one.
	if (AiPlayer->SleepCycles != 0) {
		/* we still need supply */
		return true;
	}

	// Count the already made build requests.
	int counter[UnitTypeMax];

	AiGetBuildRequestsCount(*AiPlayer, counter);
	struct cnode cache[16];

	memset(cache, 0, sizeof(cache));

	//
	// Check if we can build this?
	//
	int j = 0;
	const int n = AiHelpers.UnitLimit[0].size();

	for (int i = 0; i < n; ++i) {
		CUnitType &type = *AiHelpers.UnitLimit[0][i];
		if (counter[type.Slot]) { // Already ordered.
#if defined(DEBUG) && defined(DebugRequestSupply)
			DebugPrint("%d: AiRequestSupply: Supply already build in %s\n"
					   _C_ AiPlayer->Player->Index _C_ type->Name.c_str());
#endif
			return false;
		}

		if (!AiRequestedTypeAllowed(*AiPlayer->Player, type)) {
			continue;
		}

		//
		// Check if resources available.
		//
		cache[j].needmask = AiCheckUnitTypeCosts(type);

		for (int c = 1; c < MaxCosts; ++c) {
			cache[j].unit_cost += type.Stats[AiPlayer->Player->Index].Costs[c];
		}
		cache[j].unit_cost += type.Supply - 1;
		cache[j].unit_cost /= type.Supply;
		cache[j++].type = &type;
		Assert(j < 16);
	}

	if (j > 1) {
		std::sort(&cache[0], &cache[j], cnode_cmp);
	}
	if (j) {
		if (!cache[0].needmask) {
			CUnitType &type = *cache[0].type;
			Vec2i invalidPos = { -1, -1};
			if (AiMakeUnit(type, invalidPos)) {
				AiBuildQueue newqueue;
				newqueue.Type = &type;
				newqueue.Want = 1;
				newqueue.Made = 1;
				AiPlayer->UnitTypeBuilt.insert(
					AiPlayer->UnitTypeBuilt.begin(), newqueue);
#if defined( DEBUG) && defined( DebugRequestSupply )
				DebugPrint("%d: AiRequestSupply: build Supply in %s\n"
						   _C_ AiPlayer->Player->Index _C_ type->Name.c_str());
#endif
				return false;
			}
		}
		AiPlayer->NeededMask |= cache[0].needmask;
	}


#if defined( DEBUG) && defined( DebugRequestSupply )
	std::string needed("");
	for (int i = 1; i < MaxCosts; ++i) {
		if (cache[0].needmask & (1 << i)) {
			needed += ":";
			switch (i) {
				case GoldCost:
					needed += "Gold<";
					break;
				case WoodCost:
					needed += "Wood<";
					break;
				case OilCost:
					needed += "Oil<";
					break;
				default:
					needed += "unknown<";
					break;
			}
			needed += '0' + i;
			needed += ">";
		}
	}
	DebugPrint("%d: AiRequestSupply: needed build %s with %s resource\n"
			   _C_ AiPlayer->Player->Index _C_ cache[0].type->Name.c_str() _C_ needed.c_str());
#endif
	return true;
}

/**
**  Check if we can train the unit.
**
**  @param type  Unit that can train the unit.
**  @param what  What to be trained.
**
**  @return      True if made, false if can't be made.
**
**  @note        We must check if the dependencies are fulfilled.
*/
static bool AiTrainUnit(const CUnitType &type, CUnitType &what)
{
	std::vector<CUnit *> table;

	FindPlayerUnitsByType(*AiPlayer->Player, type, table);
	for (size_t i = 0; i != table.size(); ++i) {
		CUnit &unit = *table[i];

		if (unit.IsIdle()) {
			CommandTrainUnit(unit, what, FlushCommands);
			return true;
		}
	}
	return false;
}

/**
**  Check if we can make a unit-type.
**
**  @param type  Unit-type that must be made.
**
**  @return      True if made, false if can't be made.
**
**  @note        We must check if the dependencies are fulfilled.
*/
static int AiMakeUnit(CUnitType &typeToMake, const Vec2i &nearPos)
{
	// Find equivalents unittypes.
	int usableTypes[UnitTypeMax + 1];
	const int usableTypesCount = AiFindAvailableUnitTypeEquiv(typeToMake, usableTypes);

	// Iterate them
	for (int currentType = 0; currentType < usableTypesCount; ++currentType) {
		CUnitType &type = *UnitTypes[usableTypes[currentType]];
		int n;
		std::vector<std::vector<CUnitType *> > *tablep;
		//
		// Check if we have a place for building or a unit to build.
		//
		if (type.Building) {
			n = AiHelpers.Build.size();
			tablep = &AiHelpers.Build;
		} else {
			n = AiHelpers.Train.size();
			tablep = &AiHelpers.Train;
		}
		if (type.Slot > n) { // Oops not known.
			DebugPrint("%d: AiMakeUnit I: Nothing known about `%s'\n"
					   _C_ AiPlayer->Player->Index _C_ type.Ident.c_str());
			continue;
		}
		std::vector<CUnitType *> &table = (*tablep)[type.Slot];
		if (!table.size()) { // Oops not known.
			DebugPrint("%d: AiMakeUnit II: Nothing known about `%s'\n"
					   _C_ AiPlayer->Player->Index _C_ type.Ident.c_str());
			continue;
		}

		const int *unit_count = AiPlayer->Player->UnitTypesCount;
		for (unsigned int i = 0; i < table.size(); ++i) {
			//
			// The type for builder/trainer is available
			//
			if (unit_count[table[i]->Slot]) {
				if (type.Building) {
					if (AiBuildBuilding(*table[i], type, nearPos)) {
						return 1;
					}
				} else {
					if (AiTrainUnit(*table[i], type)) {
						return 1;
					}
				}
			}
		}
	}
	return 0;
}

/**
**  Check if we can research the upgrade.
**
**  @param type  Unit that can research the upgrade.
**  @param what  What should be researched.
**
**  @return      True if made, false if can't be made.
**
**  @note        We must check if the dependencies are fulfilled.
*/
static bool AiResearchUpgrade(const CUnitType &type, CUpgrade &what)
{
	std::vector<CUnit *> table;

	FindPlayerUnitsByType(*AiPlayer->Player, type, table);
	for (size_t i = 0; i != table.size(); ++i) {
		CUnit &unit = *table[i];

		if (unit.IsIdle()) {
			CommandResearch(unit, &what, FlushCommands);
			return true;
		}
	}
	return false;
}

/**
**  Check if the research can be done.
**
**  @param upgrade  Upgrade to research
*/
void AiAddResearchRequest(CUpgrade *upgrade)
{
	// Check if resources are available.
	const int costNeeded = AiCheckCosts(upgrade->Costs);

	if (costNeeded) {
		AiPlayer->NeededMask |= costNeeded;
		return;
	}
	//
	// Check if we have a place for the upgrade to research
	//
	const int n = AiHelpers.Research.size();
	std::vector<std::vector<CUnitType *> > &tablep = AiHelpers.Research;

	if (upgrade->ID > n) { // Oops not known.
		DebugPrint("%d: AiAddResearchRequest I: Nothing known about `%s'\n"
				   _C_ AiPlayer->Player->Index _C_ upgrade->Ident.c_str());
		return;
	}
	std::vector<CUnitType *> &table = tablep[upgrade->ID];
	if (!table.size()) { // Oops not known.
		DebugPrint("%d: AiAddResearchRequest II: Nothing known about `%s'\n"
				   _C_ AiPlayer->Player->Index _C_ upgrade->Ident.c_str());
		return;
	}

	const int *unit_count = AiPlayer->Player->UnitTypesCount;
	for (unsigned int i = 0; i < table.size(); ++i) {
		// The type is available
		if (unit_count[table[i]->Slot]
			&& AiResearchUpgrade(*table[i], *upgrade)) {
			return;
		}
	}
}

/**
**  Check if we can upgrade to unit-type.
**
**  @param type  Unit that can upgrade to unit-type
**  @param what  To what should be upgraded.
**
**  @return      True if made, false if can't be made.
**
**  @note        We must check if the dependencies are fulfilled.
*/
static bool AiUpgradeTo(const CUnitType &type, CUnitType &what)
{
	std::vector<CUnit *> table;

	// Remove all units already doing something.
	FindPlayerUnitsByType(*AiPlayer->Player, type, table);
	for (size_t i = 0; i != table.size(); ++i) {
		CUnit &unit = *table[i];

		if (unit.IsIdle()) {
			CommandUpgradeTo(unit, what, FlushCommands);
			return true;
		}
	}
	return false;
}

/**
**  Check if the upgrade-to can be done.
**
**  @param type  FIXME: docu
*/
void AiAddUpgradeToRequest(CUnitType &type)
{
	// Check if resources are available.
	const int resourceNeeded = AiCheckUnitTypeCosts(type);
	if (resourceNeeded) {
		AiPlayer->NeededMask |= resourceNeeded;
		return;
	}
	//
	// Check if we have a place for the upgrade to.
	//
	const int n = AiHelpers.Upgrade.size();
	std::vector<std::vector<CUnitType *> > &tablep = AiHelpers.Upgrade;

	if (type.Slot > n) { // Oops not known.
		DebugPrint("%d: AiAddUpgradeToRequest I: Nothing known about `%s'\n"
				   _C_ AiPlayer->Player->Index _C_ type.Ident.c_str());
		return;
	}
	std::vector<CUnitType *> &table = tablep[type.Slot];
	if (!table.size()) { // Oops not known.
		DebugPrint("%d: AiAddUpgradeToRequest II: Nothing known about `%s'\n"
				   _C_ AiPlayer->Player->Index _C_ type.Ident.c_str());
		return;
	}

	const int *unit_count = AiPlayer->Player->UnitTypesCount;
	for (unsigned int i = 0; i < table.size(); ++i) {
		//
		// The type is available
		//
		if (unit_count[table[i]->Slot]) {
			if (AiUpgradeTo(*table[i], type)) {
				return;
			}
		}
	}
}

/**
**  Check what must be built / trained.
*/
static void AiCheckingWork()
{
	// Suppy has the highest priority
	if (AiPlayer->NeedSupply) {
		if (AiPlayer->UnitTypeBuilt.empty() || AiPlayer->UnitTypeBuilt[0].Type->Supply == 0) {
			AiPlayer->NeedSupply = false;
			AiRequestSupply();
		}
	}
	// Look to the build requests, what can be done.
	const int sz = AiPlayer->UnitTypeBuilt.size();
	for (int i = 0; i < sz; ++i) {
		AiBuildQueue &queue = AiPlayer->UnitTypeBuilt[AiPlayer->UnitTypeBuilt.size() - sz + i];
		CUnitType &type = *queue.Type;

		// FIXME: must check if requirements are fulfilled.
		// Buildings can be destroyed.

		// Check if we have enough food.
		if (type.Demand && !AiCheckSupply(*AiPlayer, type)) {
			AiPlayer->NeedSupply = true;
			AiRequestSupply();
		}
		// Check limits, AI should be broken if reached.
		if (queue.Want > queue.Made && AiPlayer->Player->CheckLimits(type) < 0) {
			continue;
		}
		// Check if resources available.
		const int c = AiCheckUnitTypeCosts(type);
		if (c) {
			AiPlayer->NeededMask |= c;
			// NOTE: we can continue and build things with lesser
			//  resource or other resource need!
			continue;
		} else if (queue.Want > queue.Made && queue.Wait <= GameCycle) {
			if (AiMakeUnit(type, queue.Pos)) {
				// AiRequestSupply can change UnitTypeBuilt so recalculate queue
				AiBuildQueue &queue2 = AiPlayer->UnitTypeBuilt[AiPlayer->UnitTypeBuilt.size() - sz + i];
				++queue2.Made;
				queue2.Wait = 0;
			} else if (queue.Type->Building) {
				// Finding a building place is costly, don't try again for a while
				if (queue.Wait == 0) {
					queue.Wait = GameCycle + 150;
				} else {
					queue.Wait = GameCycle + 450;
				}
			}
		}
	}
}

/*----------------------------------------------------------------------------
--  WORKERS/RESOURCES
----------------------------------------------------------------------------*/

/**
**  Assign worker to gather a certain resource from terrain.
**
**  @param unit      pointer to the unit.
**  @param resource  resource identification.
**
**  @return          1 if the worker was assigned, 0 otherwise.
*/
static int AiAssignHarvesterFromTerrain(CUnit &unit, int resource)
{
	// TODO : hardcoded forest
	Vec2i forestPos;

	// Code for terrain harvesters. Search for piece of terrain to mine.
	if (FindTerrainType(unit.Type->MovementMask, MapFieldForest, 1000, *unit.Player, unit.tilePos, &forestPos)) {
		CommandResourceLoc(unit, forestPos, FlushCommands);
		return 1;
	}
	// Ask the AI to explore...
	AiExplore(unit.tilePos, MapFieldLandUnit);

	// Failed.
	return 0;
}

/**
**  Assign worker to gather a certain resource from Unit.
**
**  @param unit      pointer to the unit.
**  @param resource  resource identification.
**
**  @return          1 if the worker was assigned, 0 otherwise.
*/
static int AiAssignHarvesterFromUnit(CUnit &unit, int resource)
{
	// Try to find the nearest depot first.
	CUnit *depot = FindDeposit(unit, 1000, resource);
	// Find a resource to harvest from.
	CUnit *mine = UnitFindResource(unit, depot ? depot->tilePos : unit.tilePos, 1000, resource, true);

	if (mine) {
		CommandResource(unit, *mine, FlushCommands);
		return 1;
	}

	int exploremask = 0;

	for (size_t i = 0; i != UnitTypes.size(); ++i) {
		const CUnitType *type = UnitTypes[i];

		if (type && type->GivesResource == resource) {
			switch (type->UnitType) {
				case UnitTypeLand:
					exploremask |= MapFieldLandUnit;
					break;
				case UnitTypeFly:
					exploremask |= MapFieldAirUnit;
					break;
				case UnitTypeNaval:
					exploremask |= MapFieldSeaUnit;
					break;
				default:
					Assert(0);
			}
		}
	}
	// Ask the AI to explore
	AiExplore(unit.tilePos, exploremask);
	// Failed.
	return 0;
}
/**
**  Assign worker to gather a certain resource.
**
**  @param unit      pointer to the unit.
**  @param resource  resource identification.
**
**  @return          1 if the worker was assigned, 0 otherwise.
*/
static int AiAssignHarvester(CUnit &unit, int resource)
{
	// It can't.
	if (unit.Removed) {
		return 0;
	}

	const ResourceInfo &resinfo = *unit.Type->ResInfo[resource];
	Assert(&resinfo);

	if (resinfo.TerrainHarvester) {
		return AiAssignHarvesterFromTerrain(unit, resource);
	} else {
		return AiAssignHarvesterFromUnit(unit, resource);
	}
}

static bool CmpWorkers(const CUnit *lhs, const CUnit *rhs)
{
	return lhs->ResourcesHeld < rhs->ResourcesHeld;
}

/**
**  Assign workers to collect resources.
**
**  If we have a shortage of a resource, let many workers collecting this.
**  If no shortage, split workers to all resources.
*/
static void AiCollectResources()
{
	std::vector<CUnit *> units_assigned[MaxCosts]; // Worker assigned to resource
	std::vector<CUnit *> units_unassigned[MaxCosts]; // Unassigned workers
	int num_units_with_resource[MaxCosts];
	int num_units_assigned[MaxCosts];
	int num_units_unassigned[MaxCosts];
	int percent[MaxCosts];
	int priority_resource[MaxCosts];
	int priority_needed[MaxCosts];
	int wanted[MaxCosts];
	int total_harvester = 0;

	memset(num_units_with_resource, 0, sizeof(num_units_with_resource));
	memset(num_units_unassigned, 0, sizeof(num_units_unassigned));
	memset(num_units_assigned, 0, sizeof(num_units_assigned));

	// Collect statistics about the current assignment
	const int n = AiPlayer->Player->GetUnitCount();
	for (int i = 0; i < n; ++i) {
		CUnit &unit = AiPlayer->Player->GetUnit(i);
		if (!unit.Type->Harvester) {
			continue;
		}

		// See if it's assigned already
		if (unit.Orders.size() == 1 &&
			unit.CurrentAction() == UnitActionResource) {
			const COrder_Resource& order = *static_cast<COrder_Resource*>(unit.CurrentOrder());
			const int c = order.GetCurrentResource();
			units_assigned[c].push_back(&unit);
			num_units_assigned[c]++;
			total_harvester++;
			continue;
		}

		// Ignore busy units. ( building, fighting, ... )
		if (!unit.IsIdle()) {
			continue;
		}

		// Send workers with resources back home.
		if (unit.ResourcesHeld) {
			const int c = unit.CurrentResource;

			num_units_with_resource[c]++;
			CommandReturnGoods(unit, 0, FlushCommands);
			total_harvester++;
			continue;
		}

		// Look what the unit can do
		for (int c = 1; c < MaxCosts; ++c) {
			if (unit.Type->ResInfo[c]) {
				units_unassigned[c].push_back(&unit);
				num_units_unassigned[c]++;
			}
		}
		++total_harvester;
	}

	if (!total_harvester) {
		return;
	}

	memset(wanted, 0, sizeof(wanted));

	int percent_total = 100;
	for (int c = 1; c < MaxCosts; ++c) {
		percent[c] = AiPlayer->Collect[c];
		if ((AiPlayer->NeededMask & (1 << c))) { // Double percent if needed
			percent_total += percent[c];
			percent[c] <<= 1;
		}
	}

	// Turn percent values into harvester numbers.
	for (int c = 1; c < MaxCosts; ++c) {
		if (percent[c]) {
			// Wanted needs to be representative.
			if (total_harvester < 5) {
				wanted[c] = 1 + (percent[c] * 5) / percent_total;
			} else {
				wanted[c] = 1 + (percent[c] * total_harvester) / percent_total;
			}
		}
	}

	// Initialise priority & mapping
	for (int c = 0; c < MaxCosts; ++c) {
		priority_resource[c] = c;
		priority_needed[c] = wanted[c] - num_units_assigned[c] - num_units_with_resource[c];

		if (c && num_units_assigned[c] > 1) {
			//first should go workers with lower ResourcesHeld value
			std::sort(units_assigned[c].begin(), units_assigned[c].end(), CmpWorkers);
		}
	}
	CUnit *unit;
	do {
		// sort resources by priority
		for (int i = 0; i < MaxCosts; ++i) {
			for (int j = i + 1; j < MaxCosts; ++j) {
				if (priority_needed[j] > priority_needed[i]) {
					std::swap(priority_needed[i], priority_needed[j]);
					std::swap(priority_resource[i], priority_resource[j]);
				}
			}
		}
		unit = NoUnitP;

		// Try to complete each ressource in the priority order
		for (int i = 0; i < MaxCosts; ++i) {
			int c = priority_resource[i];

			// If there is a free worker for c, take it.
			if (num_units_unassigned[c]) {
				// Take the unit.
				while (0 < num_units_unassigned[c] && !AiAssignHarvester(*units_unassigned[c][0], c)) {
					// can't assign to c => remove from units_unassigned !
					units_unassigned[c][0] = units_unassigned[c][--num_units_unassigned[c]];
					units_unassigned[c].pop_back();
				}

				// unit is assigned
				if (0 < num_units_unassigned[c]) {
					unit = units_unassigned[c][0];
					units_unassigned[c][0] = units_unassigned[c][--num_units_unassigned[c]];
					units_unassigned[c].pop_back();

					// remove it from other ressources
					for (int j = 0; j < MaxCosts; ++j) {
						if (j == c || !unit->Type->ResInfo[j]) {
							continue;
						}
						for (int k = 0; k < num_units_unassigned[j]; ++k) {
							if (units_unassigned[j][k] == unit) {
								units_unassigned[j][k] = units_unassigned[j][--num_units_unassigned[j]];
								units_unassigned[j].pop_back();
								break;
							}
						}
					}
				}
			}

			// Else : Take from already assigned worker with lower priority.
			if (!unit) {
				// Take from lower priority only (i+1).
				for (int j = i + 1; j < MaxCosts && !unit; ++j) {
					// Try to move worker from src_c to c
					const int src_c = priority_resource[j];

					// Don't complete with lower priority ones...
					if (wanted[src_c] > wanted[c]
						|| (wanted[src_c] == wanted[c]
							&& num_units_assigned[src_c] <= num_units_assigned[c] + 1)) {
						continue;
					}

					for (int k = num_units_assigned[src_c] - 1; k >= 0 && !unit; --k) {
						unit = units_assigned[src_c][k];

						Assert(unit->CurrentAction() == UnitActionResource);
						COrder_Resource &order = *static_cast<COrder_Resource *>(unit->CurrentOrder());

						if (order.IsGatheringFinished()) {
							//worker returning with resource
							continue;
						}

						// unit can't harvest : next one
						if (!unit->Type->ResInfo[c] || !AiAssignHarvester(*unit, c)) {
							unit = NoUnitP;
							continue;
						}

						// Remove from src_c
						units_assigned[src_c][k] = units_assigned[src_c][--num_units_assigned[src_c]];
						units_assigned[src_c].pop_back();

						// j need one more
						priority_needed[j]++;
					}
				}
			}

			// We just moved an unit. Adjust priority & retry
			if (unit) {
				// i got a new unit.
				priority_needed[i]--;
				// Recompute priority now
				break;
			}
		}
	} while (unit);

	// Unassigned units there can't be assigned ( ie : they can't move to ressource )
	// IDEA : use transporter here.
}

/*----------------------------------------------------------------------------
--  WORKERS/REPAIR
----------------------------------------------------------------------------*/

/**
**  Check if we can repair the building.
**
**  @param type      Unit that can repair the building.
**  @param building  Building to be repaired.
**
**  @return          True if can repair, false if can't repair..
*/
static int AiRepairBuilding(const CUnitType &type, CUnit &building)
{
	// Remove all workers not mining. on the way building something
	// FIXME: It is not clever to use workers with gold
	// Idea: Antonis: Put the rest of the workers in a table in case
	// miners can't reach but others can. This will be useful if AI becomes
	// more flexible (e.g.: transports workers to an island)
	// FIXME: too hardcoded, not nice, needs improvement.
	// FIXME: too many workers repair the same building!

	// Selection of mining workers.
	std::vector<CUnit *> table;
	FindPlayerUnitsByType(*AiPlayer->Player, type, table);
	int num = 0;
	for (size_t i = 0; i != table.size(); ++i) {
		CUnit &unit = *table[i];

		if (unit.Type->RepairRange && unit.Orders.size() == 1) {
			if (unit.CurrentAction() == UnitActionStill) {
				table[num++] = &unit;
			} else if (unit.CurrentAction() == UnitActionResource) {
				COrder_Resource &order = *static_cast<COrder_Resource *>(unit.CurrentOrder());

				if (order.IsGatheringStarted() == false) {
					table[num++] = &unit;
				}
			}
		}
	}
	table.resize(num);
	// Sort by distance loops -Antonis-
	std::vector<int> distance;
	distance.resize(num);
	for (int i = 0; i < num; ++i) {
		CUnit &unit = *table[i];
		int rX = unit.tilePos.x - building.tilePos.x;
		int rY = unit.tilePos.y - building.tilePos.y;

		// FIXME: Probably calculated from top left corner of building
		rX = std::max<int>(rX, -rX);
		rY = std::max<int>(rY, -rY);
		distance[i] = std::min<int>(rX, rY);
	}
	for (int i = 0; i < num; ++i) {
		int r_temp = distance[i];
		int index_temp = i;
		for (int j = i; j < num; ++j) {
			if (distance[j] < r_temp) {
				r_temp = distance[j];
				index_temp = j;
			}
		}
		if (index_temp > i) {
			std::swap(distance[i], distance[index_temp]);
			std::swap(table[i], table[index_temp]);
		}
	}

	// Check if building is reachable and try next trio of workers

	int nbworker = AiPlayer->TriedRepairWorkers[UnitNumber(building)];
	if (nbworker > num) {
		nbworker = AiPlayer->TriedRepairWorkers[UnitNumber(building)] = 0;
	}

	int k = nbworker;
	for (int i = nbworker; i < num && i < nbworker + 3; ++i) {

		CUnit &unit = *table[i];

		if (UnitReachable(unit, building, unit.Type->RepairRange)) {
			const Vec2i invalidPos = { -1, -1};
			CommandRepair(unit, invalidPos, &building, FlushCommands);
			return 1;
		}
		k = i;
	}
	AiPlayer->TriedRepairWorkers[UnitNumber(building)] = k + 1;
	return 0;
}

/**
**  Check if we can repair this unit.
**
**  @param unit  Unit that must be repaired.
**
**  @return      True if made, false if can't be made.
*/
static int AiRepairUnit(CUnit &unit)
{
	int n = AiHelpers.Repair.size();
	std::vector<std::vector<CUnitType *> > &tablep = AiHelpers.Repair;
	const CUnitType &type = *unit.Type;
	if (type.Slot > n) { // Oops not known.
		DebugPrint("%d: AiRepairUnit I: Nothing known about `%s'\n"
				   _C_ AiPlayer->Player->Index _C_ type.Ident.c_str());
		return 0;
	}
	std::vector<CUnitType *> &table = tablep[type.Slot];
	if (!table.size()) { // Oops not known.
		DebugPrint("%d: AiRepairUnit II: Nothing known about `%s'\n"
				   _C_ AiPlayer->Player->Index _C_ type.Ident.c_str());
		return 0;
	}

	const int *unit_count = AiPlayer->Player->UnitTypesCount;
	for (unsigned int i = 0; i < table.size(); ++i) {
		//
		// The type is available
		//
		if (unit_count[table[i]->Slot]) {
			if (AiRepairBuilding(*table[i], unit)) {
				return 1;
			}
		}
	}
	return 0;
}

/**
**  Check if there's a unit that should be repaired.
*/
static void AiCheckRepair()
{
	const int n = AiPlayer->Player->GetUnitCount();
	int k = 0;

	// Selector for next unit
	for (int i = n - 1; i >= 0; --i) {
		const CUnit &unit = AiPlayer->Player->GetUnit(i);
		if (UnitNumber(unit) == AiPlayer->LastRepairBuilding) {
			k = i + 1;
		}
	}

	for (int i = k; i < n; ++i) {
		CUnit &unit = AiPlayer->Player->GetUnit(i);
		bool repair_flag = true;

		if (!unit.IsAliveOnMap()) {
			continue;
		}

		// Unit damaged?
		// Don't repair attacked unit (wait 5 sec before repairing)
		if (unit.Type->RepairHP
			&& unit.CurrentAction() != UnitActionBuilt
			&& unit.CurrentAction() != UnitActionUpgradeTo
			&& unit.Variable[HP_INDEX].Value < unit.Variable[HP_INDEX].Max
			&& unit.Attacked + 5 * CYCLES_PER_SECOND < GameCycle) {
			//
			// FIXME: Repair only units under control
			//
			if (AiEnemyUnitsInDistance(unit, unit.Stats->Variables[SIGHTRANGE_INDEX].Max)) {
				continue;
			}
			//
			// Must check, if there are enough resources
			//
			for (int j = 1; j < MaxCosts; ++j) {
				if (unit.Stats->Costs[j]
					&& (AiPlayer->Player->Resources[j] + AiPlayer->Player->StoredResources[j])  < 99) {
					repair_flag = false;
					break;
				}
			}

			//
			// Find a free worker, who can build this building can repair it?
			//
			if (repair_flag) {
				AiRepairUnit(unit);
				AiPlayer->LastRepairBuilding = UnitNumber(unit);
				return;
			}
		}
		// Building under construction but no worker
		if (unit.CurrentAction() == UnitActionBuilt) {
			int j;
			for (j = 0; j < AiPlayer->Player->GetUnitCount(); ++j) {
				COrder *order = AiPlayer->Player->GetUnit(j).CurrentOrder();
				if (order->Action == UnitActionRepair) {
					COrder_Repair &orderRepair = *static_cast<COrder_Repair *>(order);

					if (orderRepair.GetReparableTarget() == &unit) {
						break;
					}
				}
			}
			if (j == AiPlayer->Player->GetUnitCount()) {
				// Make sure we have enough resources first
				for (j = 0; j < MaxCosts; ++j) {
					// FIXME: the resources don't necessarily have to be in storage
					if (AiPlayer->Player->Resources[j] + AiPlayer->Player->StoredResources[j] < unit.Stats->Costs[j]) {
						break;
					}
				}
				if (j == MaxCosts) {
					AiRepairUnit(unit);
					AiPlayer->LastRepairBuilding = UnitNumber(unit);
					return;
				}
			}
		}
	}
	AiPlayer->LastRepairBuilding = 0;
}

/**
**  Add unit-type request to resource manager.
**
**  @param type   Unit type requested.
**  @param count  How many units.
**
**  @todo         FIXME: should store the end of list and not search it.
*/
void AiAddUnitTypeRequest(CUnitType &type, int count)
{
	AiBuildQueue queue;

	queue.Type = &type;
	queue.Want = count;
	queue.Made = 0;
	AiPlayer->UnitTypeBuilt.push_back(queue);
}

/**
**  Mark that a zone is requiring exploration.
**
**  @param pos   Pos of the zone
**  @param mask  Mask to explore ( land/sea/air )
*/
void AiExplore(const Vec2i &pos, int mask)
{
	AiExplorationRequest req(pos, mask);

	// Link into the exploration requests list
	AiPlayer->FirstExplorationRequest.insert(
		AiPlayer->FirstExplorationRequest.begin(), req);
}

/**
**  Entry point of resource manager, periodically called.
*/
void AiResourceManager()
{
	// Check if something needs to be build / trained.
	AiCheckingWork();

	// Look if we can build a farm in advance.
	if (!AiPlayer->NeedSupply && AiPlayer->Player->Supply == AiPlayer->Player->Demand) {
		AiRequestSupply();
	}

	// Collect resources.
	if ((GameCycle / CYCLES_PER_SECOND) % COLLECT_RESOURCES_INTERVAL ==
		(unsigned long)AiPlayer->Player->Index % COLLECT_RESOURCES_INTERVAL) {
		AiCollectResources();
	}

	// Check repair.
	AiCheckRepair();

	AiPlayer->NeededMask = 0;
}

//@}
