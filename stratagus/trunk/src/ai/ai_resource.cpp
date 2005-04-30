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
/**@name ai_resource.c - AI resource manager. */
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
//      $Id$

//@{

/*----------------------------------------------------------------------------
--  Includes
----------------------------------------------------------------------------*/

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "stratagus.h"
#include "unit.h"
#include "unittype.h"
#include "upgrade.h"
#include "map.h"
#include "pathfinder.h"
#include "ai_local.h"
#include "actions.h"
#include "player.h"

static int AiMakeUnit(UnitType* type);

/*----------------------------------------------------------------------------
--  Variables
----------------------------------------------------------------------------*/

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
static int AiCheckCosts(const int* costs)
{
	int i;
	int j;
	int k;
	int err;
	const int* resources;
	const int* reserve;
	int* used;
	int nunits;
	Unit** units;
	const int* building_costs;


	// FIXME: the used costs shouldn't be calculated here
	used = AiPlayer->Used;
	for (j = 1; j < MaxCosts; ++j) {
		used[j] = 0;
	}

	nunits = AiPlayer->Player->TotalNumUnits;
	units = AiPlayer->Player->Units;
	for (i = 0; i < nunits; ++i) {
		for (k = 0; k < units[i]->OrderCount; ++k) {
			if (units[i]->Orders[k].Action == UnitActionBuild) {
				building_costs =
					units[i]->Orders[k].Type->Stats[AiPlayer->Player->Player].Costs;
				for (j = 1; j < MaxCosts; ++j) {
					used[j] += building_costs[j];
				}
			}
		}
	}


	err = 0;
	resources = AiPlayer->Player->Resources;
	reserve = AiPlayer->Reserve;
	for (i = 1; i < MaxCosts; ++i) {
		if (resources[i] - used[i] < costs[i] - reserve[i]) {
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
static int AiCheckSupply(const PlayerAi* pai, const UnitType* type)
{
	int remaining;
	const AiBuildQueue* queue;

	//
	// Count food supplies under construction.
	//
	remaining = 0;
	for (queue = pai->UnitTypeBuilt; queue; queue = queue->Next) {
		if (queue->Type->Supply) {
			remaining += queue->Made * queue->Type->Supply;
		}
	}

	//
	// We are already out of food.
	//
	remaining += pai->Player->Supply - pai->Player->Demand - type->Demand;
	if (remaining < 0) {
		return 0;
	}
	//
	// Count what we train.
	//
	for (queue = pai->UnitTypeBuilt; queue; queue = queue->Next) {
		if ((remaining -= queue->Made * queue->Type->Demand) < 0) {
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
static int AiCheckUnitTypeCosts(const UnitType* type)
{
	return AiCheckCosts(type->Stats[AiPlayer->Player->Player].Costs);
}

/**
**  Enemy units in distance.
**
**  @param unit  Find in distance for this unit.
**  @param range Distance range to look.
**
**  @return      Number of enemy units.
*/
int EnemyUnitsInDistance(const Unit* unit, unsigned range)
{
	const Unit* dest;
	const UnitType* type;
	Unit* table[UnitMax];
	unsigned x;
	unsigned y;
	unsigned n;
	unsigned i;
	int e;
	const Player* player;

	//
	// Select all units in range.
	//
	x = unit->X;
	y = unit->Y;
	n = UnitCacheSelect(x - range, y - range, x + range + unit->Type->TileWidth,
		y + range + unit->Type->TileHeight, table);

	player = unit->Player;
	type = unit->Type;

	//
	// Find the enemy units which can attack
	//
	for (e = i = 0; i < n; ++i) {
		dest = table[i];
		//
		// Those can't attack anyway.
		//
		if (dest->Removed || dest->Variable[INVISIBLE_INDEX].Value ||
			dest->Orders[0].Action == UnitActionDie) {
			continue;
		}

		if (!IsEnemy(player, dest)) { // a friend or neutral
			continue;
		}
		//
		// Unit can attack back?
		//
		if (CanTarget(dest->Type, type)) {
			++e;
		}
	}

	return e;
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
static int AiBuildBuilding(const UnitType* type, UnitType* building)
{
	Unit* table[UnitMax];
	Unit* unit;
	int nunits;
	int i;
	int num;
	int x;
	int y;

#ifdef DEBUG
	unit = NoUnitP;
#endif
	//
	// Remove all workers on the way building something
	//
	nunits = FindPlayerUnitsByType(AiPlayer->Player, type, table);
	for (num = i = 0; i < nunits; ++i) {
		unit = table[i];
		for (x = 0; x < unit->OrderCount; ++x) {
			if (unit->Orders[x].Action == UnitActionBuild
				|| unit->Orders[x].Action == UnitActionRepair) {
				break;
			}
		}
		if (x == unit->OrderCount) {
			table[num++] = unit;
		}
	}

	for (i = 0; i < num; ++i) {

		unit = table[i];

		//
		// Find place on that could be build.
		//
		if (!AiFindBuildingPlace(unit, building, &x, &y)) {
			continue;
		}

		CommandBuildBuilding(unit, x, y, building, FlushCommands);

		return 1;
	}

	return 0;
}

/**
**  Build new units to reduce the food shortage.
*/
static void AiRequestSupply(void)
{
	int i;
	int n;
	int c;
	UnitType* type;
	AiBuildQueue* queue;
	int counter[UnitTypeMax];

	Assert(AiHelpers.UnitLimit);

	//
	// Count the already made build requests.
	//
	memset(counter, 0, sizeof(counter));
	for (queue = AiPlayer->UnitTypeBuilt; queue; queue = queue->Next) {
		counter[queue->Type->Slot] += queue->Want;
	}

	//
	// Check if we can build this?
	//
	n = AiHelpers.UnitLimit[0]->Count;
	for (i = 0; i < n; ++i) {
		type = AiHelpers.UnitLimit[0]->Table[i];
		if (counter[type->Slot]) { // Already ordered.
			return;
		}

		//
		// Check if resources available.
		//
		if ((c = AiCheckUnitTypeCosts(type))) {
			AiPlayer->NeededMask |= c;
			return;
		} else {
			if (AiMakeUnit(type)) {
				queue = malloc(sizeof (*AiPlayer->UnitTypeBuilt));
				queue->Next = AiPlayer->UnitTypeBuilt;
				queue->Type = type;
				queue->Want = 1;
				queue->Made = 1;
				AiPlayer->UnitTypeBuilt = queue;
			}
		}
	}
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
static int AiTrainUnit(const UnitType* type, UnitType* what)
{
	Unit* table[UnitMax];
	Unit* unit;
	int nunits;
	int i;
	int num;

#ifdef DEBUG
	unit = NoUnitP;
#endif

	//
	// Remove all units already doing something.
	//
	nunits = FindPlayerUnitsByType(AiPlayer->Player, type, table);
	for (num = i = 0; i < nunits; ++i) {
		unit = table[i];
		if (UnitIdle(unit)) {
			table[num++] = unit;
		}
	}

	for (i = 0; i < num; ++i) {
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
int AiCountUnitBuilders(UnitType* type)
{
	int result;
	int i;
	int n;
	const int* unit_count;
	AiUnitTypeTable* const* tablep;
	const AiUnitTypeTable* table;

	if (UnitIdAllowed(AiPlayer->Player, type->Slot) == 0) {
		DebugPrint("Can't build `%s' now\n" _C_ type->Ident);
		return 0;
	}
	//
	// Check if we have a place for building or an unit to build.
	//
	if (type->Building) {
		n = AiHelpers.BuildCount;
		tablep = AiHelpers.Build;
	} else {
		n = AiHelpers.TrainCount;
		tablep = AiHelpers.Train;
	}
	if (type->Slot > n) { // Oops not known.
		DebugPrint("Nothing known about `%s'\n" _C_ type->Ident);
		return 0;
	}
	table = tablep[type->Slot];
	if (!table) { // Oops not known.
		DebugPrint("Nothing known about `%s'\n" _C_ type->Ident);
		return 0;
	}
	n = table->Count;

	unit_count = AiPlayer->Player->UnitTypesCount;
	result = 0;
	for (i = 0; i < n; ++i) {
		//
		// The type for builder/trainer is available
		//
		result += unit_count[table->Table[i]->Slot];
	}
	return result;
}

/**
**  Check if we can make an unit-type.
**
**  @param type  Unit-type that must be made.
**
**  @return      True if made, false if can't be made.
**
**  @note        We must check if the dependencies are fulfilled.
*/
static int AiMakeUnit(UnitType* type)
{
	int i;
	int n;
	const int* unit_count;
	AiUnitTypeTable* const* tablep;
	const AiUnitTypeTable* table;

	int usableTypes[UnitTypeMax + 1];
	int usableTypesCount;
	int currentType;

	// Find equivalents unittypes.
	usableTypesCount = AiFindAvailableUnitTypeEquiv(type, usableTypes);

	// Iterate them
	for (currentType = 0; currentType < usableTypesCount; ++currentType) {

		type = UnitTypes[usableTypes[currentType]];

		//
		// Check if we have a place for building or an unit to build.
		//
		if (type->Building) {
			n = AiHelpers.BuildCount;
			tablep = AiHelpers.Build;
		} else {
			n = AiHelpers.TrainCount;
			tablep = AiHelpers.Train;
		}
		if (type->Slot > n) { // Oops not known.
			DebugPrint("Nothing known about `%s'\n" _C_ type->Ident);
			continue;
		}
		table = tablep[type->Slot];
		if (!table) { // Oops not known.
			DebugPrint("Nothing known about `%s'\n" _C_ type->Ident);
			continue;
		}
		n = table->Count;

		unit_count = AiPlayer->Player->UnitTypesCount;
		for (i = 0; i < n; ++i) {
			//
			// The type for builder/trainer is available
			//
			if (unit_count[table->Table[i]->Slot]) {
				if (type->Building) {
					if (AiBuildBuilding(table->Table[i], type)) {
						return 1;
					}
				} else {
					if (AiTrainUnit(table->Table[i], type)) {
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
static int AiResearchUpgrade(const UnitType* type, Upgrade* what)
{
	Unit* table[UnitMax];
	Unit* unit;
	int nunits;
	int i;
	int num;

#ifdef DEBUG
	unit = NoUnitP;
#endif
	// Remove all units already doing something.
	//
	nunits = FindPlayerUnitsByType(AiPlayer->Player, type, table);
	for (num = i = 0; i < nunits; ++i) {
		unit = table[i];
		if (UnitIdle(unit)) {
			table[num++] = unit;
		}
	}

	for (i = 0; i < num; ++i) {
		unit = table[i];

		CommandResearch(unit, what, FlushCommands);

		return 1;
	}

	return 0;
}

/**
**  Check if the research can be done.
**
**  @param upgrade  Upgrade to research
*/
void AiAddResearchRequest(Upgrade* upgrade)
{
	int i;
	int n;
	const int* unit_count;
	AiUnitTypeTable* const* tablep;
	const AiUnitTypeTable* table;

	//
	// Check if resources are available.
	//
	if ((i = AiCheckCosts(upgrade->Costs))) {
		AiPlayer->NeededMask |= i;
		return;
	}
	//
	// Check if we have a place for the upgrade to research
	//
	n = AiHelpers.ResearchCount;
	tablep = AiHelpers.Research;

	if (upgrade - Upgrades > n) { // Oops not known.
		DebugPrint("Nothing known about `%s'\n" _C_ upgrade->Ident);
		return;
	}
	table = tablep[upgrade - Upgrades];
	if (!table) { // Oops not known.
		DebugPrint("Nothing known about `%s'\n" _C_ upgrade->Ident);
		return;
	}
	n = table->Count;

	unit_count = AiPlayer->Player->UnitTypesCount;
	for (i = 0; i < n; ++i) {
		//
		// The type is available
		//
		if (unit_count[table->Table[i]->Slot]) {
			if (AiResearchUpgrade(table->Table[i], upgrade)) {
				return;
			}
		}
	}

	return;
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
static int AiUpgradeTo(const UnitType* type, UnitType* what)
{
	Unit* table[UnitMax];
	Unit* unit;
	int nunits;
	int i;
	int num;

#ifdef DEBUG
	unit = NoUnitP;
#endif
	// Remove all units already doing something.
	//
	nunits = FindPlayerUnitsByType(AiPlayer->Player, type, table);
	for (num = i = 0; i < nunits; ++i) {
		unit = table[i];
		if (UnitIdle(unit)) {
			table[num++] = unit;
		}
	}

	for (i = 0; i < num; ++i) {
		unit = table[i];

		CommandUpgradeTo(unit, what, FlushCommands);

		return 1;
	}

	return 0;
}

/**
**  Check if the upgrade-to can be done.
**
**  @param type  FIXME: docu
*/
void AiAddUpgradeToRequest(UnitType* type)
{
	int i;
	int n;
	const int* unit_count;
	AiUnitTypeTable* const* tablep;
	const AiUnitTypeTable* table;

	//
	// Check if resources are available.
	//
	if ((i = AiCheckUnitTypeCosts(type))) {
		AiPlayer->NeededMask |= i;
		return;
	}
	//
	// Check if we have a place for the upgrade to.
	//
	n = AiHelpers.UpgradeCount;
	tablep = AiHelpers.Upgrade;

	if (type->Slot > n) { // Oops not known.
		DebugPrint("Nothing known about `%s'\n" _C_ type->Ident);
		return;
	}
	table = tablep[type->Slot];
	if (!table) { // Oops not known.
		DebugPrint("Nothing known about `%s'\n" _C_ type->Ident);
		return;
	}
	n = table->Count;

	unit_count = AiPlayer->Player->UnitTypesCount;
	for (i = 0; i < n; ++i) {
		//
		// The type is available
		//
		if (unit_count[table->Table[i]->Slot]) {
			if (AiUpgradeTo(table->Table[i], type)) {
				return;
			}
		}
	}

	return;
}

/**
**  Check what must be built / trained.
*/
static void AiCheckingWork(void)
{
	int c;
	UnitType *type;
	AiBuildQueue *queue;

	// Suppy has the highest priority
	if (AiPlayer->NeedSupply) {
		if (!(AiPlayer->UnitTypeBuilt && AiPlayer->UnitTypeBuilt->Type->Supply)) {
			AiPlayer->NeedSupply = 0;
			AiRequestSupply();
		}
	}
	//
	// Look to the build requests, what can be done.
	//
	for (queue = AiPlayer->UnitTypeBuilt; queue; queue = queue->Next) {
		if (queue->Want > queue->Made) {
			type = queue->Type;

			//
			// FIXME: must check if requirements are fulfilled.
			// Buildings can be destroyed.

			//
			// Check if we have enough food.
			//
			if (type->Demand && !AiCheckSupply(AiPlayer, type)) {
				AiPlayer->NeedSupply = 1;
				AiRequestSupply();
			}
			//
			// Check limits, AI should be broken if reached.
			//
			if (PlayerCheckLimits(AiPlayer->Player, type) < 0) {
				continue;
			}
			//
			// Check if resources available.
			//
			if ((c = AiCheckUnitTypeCosts(type))) {
				AiPlayer->NeededMask |= c;
				//
				// NOTE: we can continue and build things with lesser
				//  resource or other resource need!
				continue;
			} else {
				if (AiMakeUnit(type)) {
					++queue->Made;
				}
			}
		}
	}
}

/*----------------------------------------------------------------------------
--  WORKERS/RESOURCES
----------------------------------------------------------------------------*/

/**
**  Assign worker to gather a certain resource.
**
**  @param unit      pointer to the unit.
**  @param resource  resource identification.
**
**  @return          1 if the worker was assigned, 0 otherwise.
*/
static int AiAssignHarvester(Unit* unit, int resource)
{
	ResourceInfo* resinfo;
	// These will hold the coordinates of the forest.
	int forestx;
	int foresty;
	int i;
	int exploremask;
	//  This will hold the resulting gather destination.
	Unit* dest;

	// It can't.
	if (unit->Removed) {
		return 0;
	}

	resinfo = unit->Type->ResInfo[resource];
	Assert(resinfo);
	if (resinfo->TerrainHarvester) {
		//
		// Code for terrain harvesters. Search for piece of terrain to mine.
		//
		if (FindTerrainType(UnitMovementMask(unit), MapFieldForest, 0, 1000,
				unit->Player, unit->X, unit->Y, &forestx, &foresty)) {
			CommandResourceLoc(unit, forestx, foresty, FlushCommands);
			return 1;
		}
		// Ask the AI to explore...
		AiExplore(unit->X, unit->Y, MapFieldLandUnit);
	} else {
		//
		// Find a resource to harvest from.
		//
		if ((dest = FindResource(unit, unit->X, unit->Y, 1000, resource))) {
			CommandResource(unit, dest, FlushCommands);
			return 1;
		}
		exploremask = 0;
		for (i = 0; i < UnitTypeMax; ++i) {
			if (UnitTypes[i] && UnitTypes[i]->GivesResource == resource) {
				switch (UnitTypes[i]->UnitType) {
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
		AiExplore(unit->X, unit->Y, exploremask);
	}

	// Failed.
	return 0;
}

/**
**  Assign workers to collect resources.
**
**  If we have a shortage of a resource, let many workers collecting this.
**  If no shortage, split workers to all resources.
*/
static void AiCollectResources(void)
{
	Unit* units_with_resource[UnitMax][MaxCosts]; // Worker with resource
	Unit* units_assigned[UnitMax][MaxCosts]; // Worker assigned to resource
	Unit* units_unassigned[UnitMax][MaxCosts]; // Unassigned workers
	int num_units_with_resource[MaxCosts];
	int num_units_assigned[MaxCosts];
	int num_units_unassigned[MaxCosts];
	int total; // Total of workers
	int c;
	int src_c;
	int i;
	int j;
	int k;
	int n;
	Unit** units;
	Unit* unit;
	int percent[MaxCosts];
	int percent_total;

	int priority_resource[MaxCosts];
	int priority_needed[MaxCosts];
	int wanted[MaxCosts];
	int total_harvester;

	//
	// Collect statistics about the current assignment
	//
	percent_total = 100;
	for (c = 0; c < MaxCosts; ++c) {
		num_units_with_resource[c] = 0;
		num_units_assigned[c] = 0;
		num_units_unassigned[c] = 0;
		percent[c] = AiPlayer->Collect[c];
		if ((AiPlayer->NeededMask & (1 << c))) { // Double percent if needed
			percent_total += percent[c];
			percent[c] <<= 1;
		}
	}

	total_harvester = 0;

	n = AiPlayer->Player->TotalNumUnits;
	units = AiPlayer->Player->Units;
	for (i = 0; i < n; ++i) {
		unit = units[i];
		if (!unit->Type->Harvester) {
			continue;
		}

		c = unit->CurrentResource;

		//
		// See if it's assigned already
		//
		if (unit->Orders[0].Action == UnitActionResource && unit->OrderCount == 1 && c) {
			units_assigned[num_units_assigned[c]++][c] = unit;
			total_harvester++;
			continue;
		}

		//
		// Ignore busy units. ( building, fighting, ... )
		//
		if (!UnitIdle(unit)) {
			continue;
		}

		//
		// Send workers with resources back home.
		//
		if (unit->ResourcesHeld && c) {
			units_with_resource[num_units_with_resource[c]++][c] = unit;
			CommandReturnGoods(unit, 0, FlushCommands);
			total_harvester++;
			continue;
		}

		//
		// Look what the unit can do
		//
		for (c = 0; c < MaxCosts; ++c) {
			if (unit->Type->ResInfo[c]) {
				units_unassigned[num_units_unassigned[c]++][c] = unit;
			}
		}
		++total_harvester;
	}

	total = 0;
	for (c = 0; c < MaxCosts; ++c) {
		total += num_units_assigned[c] + num_units_with_resource[c];
	}

	//
	// Turn percent values into harvester numbers.
	//

	// Wanted needs to be representative.
	if (total_harvester < 5) {
		total_harvester = 5;
	}

	for (c = 0; c < MaxCosts; ++c ) {
		if (percent[c]) {
			wanted[c] = 1 + (percent[c] * total_harvester) / percent_total;
		} else {
			wanted[c] = 0;
		}
	}

	//
	// Initialise priority & mapping
	//
	for (c = 0; c < MaxCosts; ++c) {
		priority_resource[c] = c;
		priority_needed[c] = wanted[c] - num_units_assigned[c] - num_units_with_resource[c];
	}

	do {
		//
		// sort resources by priority
		//
		for (i = 0; i < MaxCosts; ++i) {
			for (j = i + 1; j < MaxCosts; ++j) {
				if (priority_needed[j] > priority_needed[i]) {
					c = priority_needed[j];
					priority_needed[j] = priority_needed[i];
					priority_needed[i] = c;
					c = priority_resource[j];
					priority_resource[j] = priority_resource[i];
					priority_resource[i] = c;
				}
			}
		}

		//
		// Try to complete each ressource in the priority order
		//
		unit = 0;
		for (i = 0; i < MaxCosts; ++i) {
			c = priority_resource[i];

			//
			// If there is a free worker for c, take it.
			//
			if (num_units_unassigned[c]) {
				// Take the unit.
				j = 0;
				while (j < num_units_unassigned[c] && !AiAssignHarvester(units_unassigned[j][c], c)) {
					// can't assign to c => remove from units_unassigned !
					units_unassigned[j][c] = units_unassigned[--num_units_unassigned[c]][c];
				}

				// unit is assigned
				if (j < num_units_unassigned[c]) {
					unit = units_unassigned[j][c];
					units_unassigned[j][c] = units_unassigned[--num_units_unassigned[c]][c];

					// remove it from other ressources
					for (j = 0; j < MaxCosts; ++j) {
						if (j == c || !unit->Type->ResInfo[j]) {
							continue;
						}
						for (k = 0; k < num_units_unassigned[j]; ++k) {
							if (units_unassigned[k][j] == unit) {
								units_unassigned[k][j] = units_unassigned[--num_units_unassigned[j]][j];
								break;
							}
						}
					}
				}
			}

			//
			// Else : Take from already assigned worker with lower priority.
			//
			if (!unit) {
				// Take from lower priority only (i+1).
				for (j = i + 1; j < MaxCosts; ++j) {
					// Try to move worker from src_c to c
					src_c = priority_resource[j];

					// Don't complete with lower priority ones...
					if (wanted[src_c] >= wanted[c]) {
						continue;
					}

					for (k = num_units_assigned[src_c]-1; k >= 0 ; --k) {
						unit = units_assigned[k][src_c];

						// unit can't harvest : next one
						if (!unit->Type->ResInfo[c] || !AiAssignHarvester(unit, c)) {
							unit = 0;
							continue;
						}

						// Remove from src_c
						units_assigned[k][src_c] = units_assigned[--num_units_assigned[src_c]][src_c];

						// j need one more
						priority_needed[j]++;
					}
				}
			}

			//
			// We just moved an unit. Adjust priority & retry
			//
			if (unit) {
				// i got a new unit.
				priority_needed[i]--;

				// Add to the assigned
				units_assigned[num_units_assigned[c]++][c] = unit;

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
static int AiRepairBuilding(const UnitType* type, Unit* building)
{
	Unit* table[UnitMax];
	Unit* unit;
	Unit* unit_temp;
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

#ifdef DEBUG
	unit = NoUnitP;
#endif
	//
	// Remove all workers not mining. on the way building something
	// FIXME: It is not clever to use workers with gold
	// Idea: Antonis: Put the rest of the workers in a table in case
	// miners can't reach but others can. This will be useful if AI becomes
	// more flexible (e.g.: transports workers to an island)
	// FIXME: too hardcoded, not nice, needs improvement.
	// FIXME: too many workers repair the same building!

	// Selection of mining workers.
	nunits = FindPlayerUnitsByType(AiPlayer->Player, type, table);
	for (num = i = 0; i < nunits; ++i) {
		unit = table[i];
		if (unit->Type->RepairRange &&
			(unit->Orders[0].Action == UnitActionResource ||
				unit->Orders[0].Action == UnitActionStill) && unit->OrderCount == 1) {
			table[num++] = unit;
		}
	}

	// Sort by distance loops -Antonis-
	for (i = 0; i < num; ++i) {
		unit = table[i];
		// FIXME: Probably calculated from top left corner of building
		if ((rX = unit->X - building->X) < 0) {
			rX = -rX;
		}
		if ((rY = unit->Y - building->Y) < 0) {
			rY = -rY;
		}
		if (rX < rY) {
			distance[i] = rX;
		} else {
			distance[i] = rY;
		}
	}
	for (i = 0; i < num; ++i) {
		r_temp = distance[i];
		index_temp = i;
		for (j = i; j < num; ++j) {
			if (distance[j] < r_temp) {
				r_temp = distance[j];
				index_temp = j;
			}
		}
		if (index_temp > i) {
			unit_temp = table[index_temp];
			table[index_temp] = table[i];
			distance[index_temp] = distance[i];
			table[i] = unit_temp;
			distance[i] = r_temp; // May be omitted, here for completence
		}
	}

	// Check if building is reachable and try next trio of workers

	if ((j = AiPlayer->TriedRepairWorkers[UnitNumber(building)]) > num) {
		j = AiPlayer->TriedRepairWorkers[UnitNumber(building)] = 0;
	}

	for (k = i = j; i < num && i < j + 3; ++i) {

		unit = table[i];

		if (UnitReachable(unit, building, unit->Type->RepairRange)) {
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
static int AiRepairUnit(Unit* unit)
{
	int i;
	int n;
	const UnitType* type;
	const int* unit_count;
	AiUnitTypeTable* const* tablep;
	const AiUnitTypeTable* table;

	n = AiHelpers.RepairCount;
	tablep = AiHelpers.Repair;
	type = unit->Type;
	if (type->Slot > n) { // Oops not known.
		DebugPrint("Nothing known about `%s'\n" _C_ type->Ident);
		return 0;
	}
	table = tablep[type->Slot];
	if (!table) { // Oops not known.
		DebugPrint("Nothing known about `%s'\n" _C_ type->Ident);
		return 0;
	}

	n = table->Count;
	unit_count = AiPlayer->Player->UnitTypesCount;
	for (i = 0; i < n; ++i) {
		//
		// The type is available
		//
		if (unit_count[table->Table[i]->Slot]) {
			if (AiRepairBuilding(table->Table[i], unit)) {
				return 1;
			}
		}
	}

	return 0;
}

/**
**  Look through the units, if an unit must be repaired.
*/
static void AiCheckRepair(void)
{
	int i;
	int j;
	int k;
	int n;
	int repair_flag;
	Unit* unit;

	n = AiPlayer->Player->TotalNumUnits;
	k = 0;
	// Selector for next unit
	for (i = n; (i > 0); --i) {
		unit = AiPlayer->Player->Units[i];
		if (unit) {
			if (UnitNumber(unit) == AiPlayer->LastRepairBuilding) {
				k = i + 1;
			}
		}
	}

	for (i = k; i < n; ++i) {
		unit = AiPlayer->Player->Units[i];
		repair_flag = 1;
		// Unit damaged?
		// Don't repair attacked unit ( wait 5 sec before repairing )
		if (unit->Type->Building
			&& unit->Orders[0].Action != UnitActionBuilt
			&& unit->Orders[0].Action != UnitActionUpgradeTo
			&& unit->Variable[HP_INDEX].Value < unit->Variable[HP_INDEX].Max
			&& unit->Attacked + 5 * CYCLES_PER_SECOND < GameCycle) {

			//
			// FIXME: Repair only buildings under control
			//
			if (EnemyUnitsInDistance(unit, unit->Stats->Variables[SIGHTRANGE_INDEX].Max)) {
				continue;
			}
			//
			// Must check, if there are enough resources
			//
			for (j = 1; j < MaxCosts; ++j) {
				if (unit->Stats->Costs[j]
					&& AiPlayer->Player->Resources[j] < 99) {
					repair_flag = 0;
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
void AiAddUnitTypeRequest(UnitType* type, int count)
{
	AiBuildQueue** queue;

	//
	// Find end of the list.
	//
	for (queue = &AiPlayer->UnitTypeBuilt; *queue; queue = &(*queue)->Next) {
	}

	*queue = malloc(sizeof (*AiPlayer->UnitTypeBuilt));
	(*queue)->Next = NULL;
	(*queue)->Type = type;
	(*queue)->Want = count;
	(*queue)->Made = 0;
}

/**
**  Mark that a zone is requiring exploration.
**
**  @param x     X pos of the zone
**  @param y     Y pos of the zone
**  @param mask  Mask to explore ( land/sea/air )
*/
void AiExplore(int x, int y, int mask)
{
	AiExplorationRequest* req;

	// Alloc a new struct,
	req = malloc(sizeof(AiExplorationRequest));

	// Link into the exploration requests list
	req->X = x;
	req->Y = y;
	req->Mask = mask;

	req->Next = AiPlayer->FirstExplorationRequest;
	AiPlayer->FirstExplorationRequest = req;
}

/**
**  Entry point of resource manager, periodically called.
*/
void AiResourceManager(void)
{
	//
	// Check if something needs to be build / trained.
	//
	AiCheckingWork();
	//
	// Look if we can build a farm in advance.
	//
	if (!AiPlayer->NeedSupply && AiPlayer->Player->Supply == AiPlayer->Player->Demand) {
		AiRequestSupply();
	}
	//
	// Collect resources.
	//
	AiCollectResources();

	//
	// Check repair.
	//
	AiCheckRepair();

	AiPlayer->NeededMask = 0;
}

//@}
