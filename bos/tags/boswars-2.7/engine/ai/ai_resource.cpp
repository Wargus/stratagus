//     ____                _       __               
//    / __ )____  _____   | |     / /___ ___________
//   / __  / __ \/ ___/   | | /| / / __ `/ ___/ ___/
//  / /_/ / /_/ (__  )    | |/ |/ / /_/ / /  (__  ) 
// /_____/\____/____/     |__/|__/\__,_/_/  /____/  
//                                              
//       A futuristic real-time strategy game.
//          This file is part of Bos Wars.
//
/**@name ai_resource.cpp - AI resource manager. */
//
//      (c) Copyright 2000-2009 by Lutz Sammer and Antonis Chaniotis.
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
#include "unit.h"
#include "unit_cache.h"
#include "unittype.h"
#include "upgrade.h"
#include "map.h"
#include "pathfinder.h"
#include "ai_local.h"
#include "actions.h"
#include "player.h"

/*----------------------------------------------------------------------------
--  Defines
----------------------------------------------------------------------------*/

#define COLLECT_RESOURCES_INTERVAL 4

/*----------------------------------------------------------------------------
--  Functions
----------------------------------------------------------------------------*/

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
	int err = 0;

	for (int i = 0; i < MaxCosts; ++i)
	{
		if (costs[i] &&
			AiPlayer->Player->ProductionRate[i] == 0 &&
			AiPlayer->Player->StoredResources[i] == 0)
		{
			err |= (1 << i);
		}
	}

	return err;
}

/**
**  Check if the costs for a unit-type are available for the AI.
**
**  Take reserve and already used resources into account.
**
**  @param type  Unit-type to check the costs for.
**
**  @return      A bit field of the missing costs.
*/
static int AiCheckUnitTypeCosts(const CUnitType *type)
{
	return AiCheckCosts(type->ProductionCosts);
}

/**
**  Enemy units in distance.
**
**  @param player  Find enemies of this player
**  @param type    Optional unit type to check if enemy can target this
**  @param x       X location
**  @param y       Y location
**  @param range   Distance range to look.
**
**  @return       Number of enemy units.
*/
int AiEnemyUnitsInDistance(const CPlayer *player, const CUnitType *type, int x, int y, unsigned range)
{
	const CUnit *dest;
	CUnit *table[UnitMax];
	unsigned n;
	unsigned i;
	int e;

	//
	// Select all units in range.
	//
	n = UnitCache.Select(x - range, y - range, x + range, y + range, table, UnitMax);

	//
	// Find the enemy units which can attack
	//
	for (e = i = 0; i < n; ++i)
	{
		dest = table[i];
		//
		// Those can't attack anyway.
		//
		if (dest->Removed || dest->Orders[0]->Action == UnitActionDie)
		{
			continue;
		}

		if (!player->IsEnemy(dest))
		{
			// a friend or neutral
			continue;
		}
		//
		// Unit can attack back?
		//
		if (!type || CanTarget(dest->Type, type))
		{
			++e;
		}
	}

	return e;
}

/**
**  Enemy units in distance.
**
**  @param unit   Find in distance for this unit.
**  @param range  Distance range to look.
**
**  @return       Number of enemy units.
*/
int AiEnemyUnitsInDistance(const CUnit *unit, unsigned range)
{
	range += std::max(unit->Type->TileWidth, unit->Type->TileHeight);
	return AiEnemyUnitsInDistance(unit->Player, unit->Type, unit->X, unit->Y, range);
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
static int AiBuildBuilding(const CUnitType *type, CUnitType *building)
{
	CUnit *table[UnitMax];
	CUnit *unit;
	int nunits;
	int i;
	int num;
	int x;
	int y;

	//
	// Remove all workers on the way building something
	//
	nunits = FindPlayerUnitsByType(AiPlayer->Player, type, table, UnitMax);
	for (num = i = 0; i < nunits; ++i)
	{
		unit = table[i];
		for (x = 0; x < unit->OrderCount; ++x)
		{
			if (unit->Orders[x]->Action == UnitActionBuild ||
				unit->Orders[x]->Action == UnitActionRepair)
			{
				break;
			}
		}
		if (x == unit->OrderCount)
		{
			table[num++] = unit;
		}
	}

	if (num == 0)
	{
		// No workers available to build
		return 0;
	}

	// Try one worker at random to save cpu
	unit = table[SyncRand() % num];

	// Find a place to build.
	if (AiFindBuildingPlace(unit, building, &x, &y))
	{
		CommandBuildBuilding(unit, x, y, building, FlushCommands);
		return 1;
	}
	return 0;
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
static int AiTrainUnit(const CUnitType *type, CUnitType *what)
{
	CUnit *table[UnitMax];
	CUnit *unit;
	int nunits;
	int i;
	int num;

	//
	// Remove all units already doing something
	// or located in an unsuitable terrain.
	//
	nunits = FindPlayerUnitsByType(AiPlayer->Player, type, table, UnitMax);
	for (num = i = 0; i < nunits; ++i)
	{
		unit = table[i];
		if (unit->IsIdle() && TerrainAllowsTraining(unit, what))
		{
			table[num++] = unit;
		}
	}

	for (i = 0; i < num; ++i)
	{
		unit = table[i];
		CommandTrainUnit(unit, what, FlushCommands);

		return 1;
	}

	return 0;
}

/**
**  Return the number of unit which can produce the given unittype.
**
**  @param type  The unittype we wan't to build
*/
int AiCountUnitBuilders(CUnitType *type)
{
	int result;
	int n;
	const int *unit_count;
	std::vector<std::vector<CUnitType *> > *tablep;
	const std::vector<CUnitType *> *table;

	if (UnitIdAllowed(AiPlayer->Player, type->Slot) == 0)
	{
		DebugPrint("Can't build `%s' now\n" _C_ type->Ident.c_str());
		return 0;
	}
	//
	// Check if we have a place for building or a unit to build.
	//
	if (type->Building)
	{
		n = AiHelpers.Build.size();
		tablep = &AiHelpers.Build;
	}
	else
	{
		n = AiHelpers.Train.size();
		tablep = &AiHelpers.Train;
	}
	if (type->Slot > n)
	{
		// Oops not known.
		DebugPrint("Nothing known about `%s'\n" _C_ type->Ident.c_str());
		return 0;
	}
	table = &(*tablep)[type->Slot];
	if (!table->size())
	{
		// Oops not known.
		DebugPrint("Nothing known about `%s'\n" _C_ type->Ident.c_str());
		return 0;
	}

	unit_count = AiPlayer->Player->UnitTypesCount;
	result = 0;
	for (size_t i = 0; i < table->size(); ++i)
	{
		//
		// The type for builder/trainer is available
		//
		result += unit_count[(*table)[i]->Slot];
	}
	return result;
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
static int AiMakeUnit(CUnitType *type)
{
	int i;
	int n;
	const int *unit_count;
	std::vector<std::vector<CUnitType *> > *tablep;
	std::vector<CUnitType *> *table;

	int usableTypes[UnitTypeMax + 1];
	int usableTypesCount;
	int currentType;

	// Find equivalents unittypes.
	usableTypesCount = AiFindAvailableUnitTypeEquiv(type, usableTypes);

	// Iterate them
	for (currentType = 0; currentType < usableTypesCount; ++currentType)
	{
		type = UnitTypes[usableTypes[currentType]];

		//
		// Check if we have a place for building or a unit to build.
		//
		if (type->Building)
		{
			n = AiHelpers.Build.size();
			tablep = &AiHelpers.Build;
		}
		else
		{
			n = AiHelpers.Train.size();
			tablep = &AiHelpers.Train;
		}
		if (type->Slot > n)
		{
			// Oops not known.
			DebugPrint("Nothing known about `%s'\n" _C_ type->Ident.c_str());
			continue;
		}
		table = &(*tablep)[type->Slot];
		if (!table->size())
		{
			// Oops not known.
			DebugPrint("Nothing known about `%s'\n" _C_ type->Ident.c_str());
			continue;
		}

		unit_count = AiPlayer->Player->UnitTypesCount;
		for (i = 0; i < (int)table->size(); ++i)
		{
			//
			// The type for builder/trainer is available
			//
			if (unit_count[(*table)[i]->Slot])
			{
				if (type->Building)
				{
					if (AiBuildBuilding((*table)[i], type))
					{
						return 1;
					}
				}
				else
				{
					if (AiTrainUnit((*table)[i], type))
					{
						return 1;
					}
				}
			}
		}
	}
	return 0;
}

/**
**  Check what must be built / trained.
*/
static void AiCheckingWork()
{
	int c;
	CUnitType *type;
	AiBuildQueue *queue;

	//
	// Look to the build requests, what can be done.
	//
	int sz = AiPlayer->UnitTypeBuilt.size();
	for (int i = 0; i < sz; ++i)
	{
		queue = &AiPlayer->UnitTypeBuilt[AiPlayer->UnitTypeBuilt.size() - sz + i];
		type = queue->Type;

		//
		// FIXME: must check if requirements are fulfilled.
		// Buildings can be destroyed.

		//
		// Check limits, AI should be broken if reached.
		//
		if (queue->Want > queue->Made && AiPlayer->Player->CheckLimits(type) < 0)
		{
			continue;
		}
		//
		// Check if resources available.
		//
		if ((c = AiCheckUnitTypeCosts(type)))
		{
			AiPlayer->NeededMask |= c;
			//
			// NOTE: we can continue and build things with lesser
			//  resource or other resource need!
			continue;
		}
		else if (queue->Want > queue->Made && queue->Wait <= GameCycle)
		{
			if (AiMakeUnit(type))
			{
				// AiRequestSupply can change UnitTypeBuilt so recalculate queue
				queue = &AiPlayer->UnitTypeBuilt[AiPlayer->UnitTypeBuilt.size() - sz + i];
				++queue->Made;
				queue->Wait = 0;
			}
			else if (queue->Type->Building)
			{
				// Finding a building place is costly, don't try again for a while
				if (queue->Wait == 0)
				{
					queue->Wait = GameCycle + 150;
				}
				else
				{
					queue->Wait = GameCycle + 450;
				}
			}
		}
	}
}

/*----------------------------------------------------------------------------
--  WORKERS/RESOURCES
----------------------------------------------------------------------------*/

/**
**  Assign worker to gather a resource.
**
**  @param unit      Unit to be assigned.
**  @param resource  Resource to find, -1 for any
**
**  @return          1 if the worker was assigned, 0 otherwise.
*/
static int AiAssignHarvester(CUnit *unit, int resource)
{
	CUnit *dest;
	int res = resource;

	// It can't.
	if (unit->Removed)
	{
		return 0;
	}

	if (res == -1)
	{
		// No resource specified, see which resource we need most
		for (int i = 0; i < MaxCosts; ++i)
		{
			if (unit->Player->ProductionRate[i] == 0)
			{
				res = i;
				if (unit->Player->StoredResources[i] == 0)
				{
					break;
				}
			}
		}
	}

	//
	// Find a resource to harvest from.
	//
	dest = UnitFindResource(unit, unit->X, unit->Y, 1000, res);
	if (!dest && res != -1)
	{
		dest = UnitFindResource(unit, unit->X, unit->Y, 1000, -1);
	}
	if (dest)
	{
		CommandResource(unit, dest, FlushCommands);
		return 1;
	}

	// Failed.
	return 0;
}

/**
**  Assign workers to collect resources.
*/
static void AiCollectResources()
{
	int i;
	int needed = -1;
	std::vector<CUnit *> harvesters;

	// Check if we need a resource
	if (AiPlayer->NeededMask)
	{
		int n = AiPlayer->NeededMask;
		while (n != 0)
		{
			n >>= 1;
			++needed;
		}
	}

	// Build a list of all harvesters
	for (i = 0; i < AiPlayer->Player->TotalNumUnits; ++i)
	{
		CUnit *unit = AiPlayer->Player->Units[i];

		// Ignore non-harvesters
		if (!unit->Type->Harvester)
		{
			continue;
		}

		harvesters.push_back(unit);
	}

	// If we need a resource, pick one unit to harvest it
	if (needed != -1 && harvesters.size() != 0)
	{
		int best = -1;
		for (i = 0; i < (int)harvesters.size(); ++i)
		{
			// Idle units are always best
			if (harvesters[i]->IsIdle())
			{
				best = i;
				break;
			}
			// Unit harvesting the wrong resource
			if (harvesters[i]->Orders[0]->Action == UnitActionResource &&
				harvesters[i]->Data.Harvest.CurrentProduction[needed] == 0)
			{
				best = i;
			}
		}

		if (best == -1)
		{
			// No best unit found, pick one at random
			best = SyncRand() % harvesters.size();
		}

		if (AiAssignHarvester(harvesters[best], needed))
		{
			harvesters.erase(harvesters.begin() + best);
		}
	}

	// Assign one random idle harvester at a time
	std::vector<CUnit *> idleHarvesters;
	for (i = 0; i < (int)harvesters.size(); ++i)
	{
		if (harvesters[i]->IsIdle())
		{
			idleHarvesters.push_back(harvesters[i]);
		}
	}
	if (idleHarvesters.size() > 0)
	{
		AiAssignHarvester(idleHarvesters[SyncRand() % idleHarvesters.size()], -1);
	}
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
static int AiRepairBuilding(const CUnitType *type, CUnit *building)
{
	CUnit *table[UnitMax];
	CUnit *unit;
	CUnit *unit_temp;
	int distance[UnitMax];
	int rX;
	int rY;
	int r_temp;
	int index_temp;
	int nunits;
	int i;
	int j;
	int k;
	int num;

	unit = NoUnitP;

	//
	// Remove all workers not mining. on the way building something
	// FIXME: It is not clever to use workers with gold
	// Idea: Antonis: Put the rest of the workers in a table in case
	// miners can't reach but others can. This will be useful if AI becomes
	// more flexible (e.g.: transports workers to an island)
	// FIXME: too hardcoded, not nice, needs improvement.
	// FIXME: too many workers repair the same building!

	// Selection of mining workers.
	nunits = FindPlayerUnitsByType(AiPlayer->Player, type, table, UnitMax);
	for (num = i = 0; i < nunits; ++i)
	{
		unit = table[i];
		if (unit->Type->RepairRange &&
			(unit->Orders[0]->Action == UnitActionResource ||
			 unit->Orders[0]->Action == UnitActionStill) &&
			unit->OrderCount == 1)
		{
			table[num++] = unit;
		}
	}

	// Sort by distance loops -Antonis-
	for (i = 0; i < num; ++i)
	{
		unit = table[i];
		// FIXME: Probably calculated from top left corner of building
		if ((rX = unit->X - building->X) < 0)
		{
			rX = -rX;
		}
		if ((rY = unit->Y - building->Y) < 0)
		{
			rY = -rY;
		}
		if (rX < rY)
		{
			distance[i] = rX;
		}
		else
		{
			distance[i] = rY;
		}
	}
	for (i = 0; i < num; ++i)
	{
		r_temp = distance[i];
		index_temp = i;
		for (j = i; j < num; ++j)
		{
			if (distance[j] < r_temp)
			{
				r_temp = distance[j];
				index_temp = j;
			}
		}
		if (index_temp > i)
		{
			unit_temp = table[index_temp];
			table[index_temp] = table[i];
			distance[index_temp] = distance[i];
			table[i] = unit_temp;
			distance[i] = r_temp; // May be omitted, here for completence
		}
	}

	// Check if building is reachable and try next trio of workers

	if ((j = AiPlayer->TriedRepairWorkers[UnitNumber(building)]) > num)
	{
		j = AiPlayer->TriedRepairWorkers[UnitNumber(building)] = 0;
	}

	for (k = i = j; i < num && i < j + 3; ++i)
	{
		unit = table[i];

		if (UnitReachable(unit, building, unit->Type->RepairRange))
		{
			CommandRepair(unit, 0, 0, building, FlushCommands);
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
static int AiRepairUnit(CUnit *unit)
{
	int n;
	const CUnitType *type;
	const int *unit_count;
	std::vector<std::vector<CUnitType *> > *tablep;
	std::vector<CUnitType *> *table;

	n = AiHelpers.Repair.size();
	tablep = &AiHelpers.Repair;
	type = unit->Type;
	if (type->Slot > n)
	{
		// Oops not known.
		DebugPrint("Nothing known about `%s'\n" _C_ type->Ident.c_str());
		return 0;
	}
	table = &(*tablep)[type->Slot];
	if (!table->size())
	{
		// Oops not known.
		DebugPrint("Nothing known about `%s'\n" _C_ type->Ident.c_str());
		return 0;
	}

	unit_count = AiPlayer->Player->UnitTypesCount;
	for (size_t i = 0; i < table->size(); ++i)
	{
		// The type is available
		if (unit_count[(*table)[i]->Slot])
		{
			if (AiRepairBuilding((*table)[i], unit))
			{
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
	int i;
	int k;
	int n;
	bool repair_flag;
	CUnit *unit;

	n = AiPlayer->Player->TotalNumUnits;
	k = 0;
	// Selector for next unit
	for (i = n - 1; i >= 0; --i)
	{
		unit = AiPlayer->Player->Units[i];
		if (UnitNumber(unit) == AiPlayer->LastRepairBuilding)
		{
			k = i + 1;
		}
	}


	for (i = k; i < n; ++i)
	{
		unit = AiPlayer->Player->Units[i];
		repair_flag = true;
		// Unit damaged?
		// Don't repair attacked unit (wait 5 sec before repairing)
		if (unit->Type->RepairHP &&
			unit->Orders[0]->Action != UnitActionBuilt &&
			unit->Variable[HP_INDEX].Value < unit->Variable[HP_INDEX].Max &&
			unit->Attacked + 5 * CYCLES_PER_SECOND < GameCycle)
		{

			// FIXME: Repair only units under control
			if (AiEnemyUnitsInDistance(unit, unit->Stats->Variables[SIGHTRANGE_INDEX].Max))
			{
				continue;
			}
			//
			// Must check if there are enough resources
			//
			for (int j = 0; j < MaxCosts; ++j)
			{
				if (unit->Type->ProductionCosts[j] &&
					AiPlayer->Player->ProductionRate[j] == 0 &&
					AiPlayer->Player->StoredResources[j] == 0)
				{
					repair_flag = false;
					break;
				}
			}

			//
			// Find a free worker, who can build this building can repair it?
			//
			if (repair_flag)
			{
				AiRepairUnit(unit);
				AiPlayer->LastRepairBuilding = UnitNumber(unit);
				return;
			}
		}

		// Building under construction but no worker
		if (unit->Orders[0]->Action == UnitActionBuilt)
		{
			int j;
			for (j = 0; j < AiPlayer->Player->TotalNumUnits; ++j)
			{
				COrder *order = AiPlayer->Player->Units[j]->Orders[0];
				if (order->Action == UnitActionRepair && order->Goal == unit)
				{
					break;
				}
			}
			if (j == AiPlayer->Player->TotalNumUnits)
			{
				// Make sure we have enough resources first
				for (j = 0; j < MaxCosts; ++j)
				{
					// FIXME: the resources don't necessarily have to be in storage
					if (AiPlayer->Player->StoredResources[j] < unit->Type->ProductionCosts[j])
					{
						break;
					}
				}
				if (j == MaxCosts)
				{
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
void AiAddUnitTypeRequest(CUnitType *type, int count)
{
	AiBuildQueue queue;

	queue.Type = type;
	queue.Want = count;
	queue.Made = 0;
	AiPlayer->UnitTypeBuilt.push_back(queue);
}

/**
**  Entry point of resource manager, periodically called.
*/
void AiResourceManager()
{
	//
	// Check if something needs to be build / trained.
	//
	AiCheckingWork();

	//
	// Collect resources.
	//
	if ((GameCycle / CYCLES_PER_SECOND) % COLLECT_RESOURCES_INTERVAL ==
			(unsigned long)AiPlayer->Player->Index % COLLECT_RESOURCES_INTERVAL)
	{
		AiCollectResources();
	}

	//
	// Check repair.
	//
	AiCheckRepair();

	AiPlayer->NeededMask = 0;
}

//@}
