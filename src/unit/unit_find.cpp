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
/**@name unit_find.cpp - The find/select for units. */
//
//      (c) Copyright 1998-2015 by Lutz Sammer, Jimmy Salmon and Andrettin
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

#include <limits.h>

#include "stratagus.h"

#include "unit_find.h"

#include "actions.h"
#include "map.h"
#include "missile.h"
#include "pathfinder.h"
#include "player.h"
#include "spells.h"
#include "tileset.h"
#include "unit.h"
#include "unit_manager.h"
#include "unittype.h"

/*----------------------------------------------------------------------------
  -- Finding units
  ----------------------------------------------------------------------------*/

void Select(const Vec2i &ltPos, const Vec2i &rbPos, std::vector<CUnit *> &units)
{
	Select(ltPos, rbPos, units, NoFilter());
}

void SelectFixed(const Vec2i &ltPos, const Vec2i &rbPos, std::vector<CUnit *> &units)
{
	Select(ltPos, rbPos, units, NoFilter());
}

void SelectAroundUnit(const CUnit &unit, int range, std::vector<CUnit *> &around)
{
	SelectAroundUnit(unit, range, around, NoFilter());
}

CUnit *UnitFinder::FindUnitAtPos(const Vec2i &pos) const
{
	CUnitCache &cache = Map.Field(pos)->UnitCache;

	for (CUnitCache::iterator it = cache.begin(); it != cache.end(); ++it) {
		CUnit *unit = *it;

		if (std::find(units.begin(), units.end(), unit) != units.end()) {
			return unit;
		}
	}
	return NULL;
}

VisitResult UnitFinder::Visit(TerrainTraversal &terrainTraversal, const Vec2i &pos, const Vec2i &from)
{
	if (!player.AiEnabled && !Map.Field(pos)->playerInfo.IsExplored(player)) {
		return VisitResult_DeadEnd;
	}
	// Look if found what was required.
	CUnit *unit = FindUnitAtPos(pos);
	if (unit) {
		*unitP = unit;
		return VisitResult_Finished;
	}
	if (CanMoveToMask(pos, movemask)) { // reachable
		if (terrainTraversal.Get(pos) <= maxDist) {
			return VisitResult_Ok;
		} else {
			return VisitResult_DeadEnd;
		}
	} else { // unreachable
		return VisitResult_DeadEnd;
	}
}

class TerrainFinder
{
public:
	TerrainFinder(const CPlayer &player, int maxDist, int movemask, int resmask, Vec2i *resPos) :
		player(player), maxDist(maxDist), movemask(movemask), resmask(resmask), resPos(resPos) {}
	VisitResult Visit(TerrainTraversal &terrainTraversal, const Vec2i &pos, const Vec2i &from);
private:
	const CPlayer &player;
	int maxDist;
	int movemask;
	int resmask;
	Vec2i *resPos;
};

VisitResult TerrainFinder::Visit(TerrainTraversal &terrainTraversal, const Vec2i &pos, const Vec2i &from)
{
	if (!player.AiEnabled && !Map.Field(pos)->playerInfo.IsExplored(player)) {
		return VisitResult_DeadEnd;
	}
	// Look if found what was required.
	if (Map.Field(pos)->CheckMask(resmask)) {
		if (resPos) {
			*resPos = pos;
		}
		return VisitResult_Finished;
	}
	if (CanMoveToMask(pos, movemask)) { // reachable
		if (terrainTraversal.Get(pos) <= maxDist) {
			return VisitResult_Ok;
		} else {
			return VisitResult_DeadEnd;
		}
	} else { // unreachable
		return VisitResult_DeadEnd;
	}
}

/**
**  Find the closest piece of terrain with the given flags.
**
**  @param movemask    The movement mask to reach that location.
**  @param resmask     Result tile mask.
**  @param range       Maximum distance for the search.
**  @param player      Only search fields explored by player
**  @param startPos    Map start position for the search.
**
**  @param terrainPos  OUT: Map position of tile.
**
**  @note Movement mask can be 0xFFFFFFFF to have no effect
**  Range is not circular, but square.
**  Player is ignored if nil(search the entire map)
**
**  @return            True if wood was found.
*/
bool FindTerrainType(int movemask, int resmask, int range,
					 const CPlayer &player, const Vec2i &startPos, Vec2i *terrainPos)
{
	TerrainTraversal terrainTraversal;

	terrainTraversal.SetSize(Map.Info.MapWidth, Map.Info.MapHeight);
	terrainTraversal.Init();

	terrainTraversal.PushPos(startPos);

	TerrainFinder terrainFinder(player, range, movemask & ~(MapFieldLandUnit | MapFieldAirUnit | MapFieldSeaUnit), resmask, terrainPos);

	return terrainTraversal.Run(terrainFinder);
}


template <const bool NEARLOCATION>
class BestDepotFinder
{
	void operator()(CUnit *const dest)
	{
		/* Only resource depots */
		if (dest->Type->CanStore[resource]
			&& dest->IsAliveOnMap()
			&& dest->CurrentAction() != UnitActionBuilt) {
			// Unit in range?

			if constexpr (NEARLOCATION) {
				const int d = dest->MapDistanceTo(u_near.loc);

				//
				// Take this depot?
				//
				if (d <= this->range && d < this->best_dist) {
					this->best_depot = dest;
					this->best_dist = d;
				}
			} else {
				const CUnit *worker = this->u_near.worker;
				const CUnit *first_container = GetFirstContainer(*worker);

				//simple distance
				const int distance = first_container->MapDistanceTo(*dest);

				// Use Circle, not square :)
				if (distance > this->range) {
					return;
				}

				if (distance >= this->best_dist) {
					//if the depot's simple distance is greater or equal to the real travel distance of the currently-chosen depot, then it can never be closer than it, and we have no reason to actually calculate its real travel distance 
					return;
				}

				// calc real travel distance
				const int travel_distance = UnitReachable(*worker, *dest, 1, worker->Container != nullptr);
				//
				// Take this depot?
				//
				if (travel_distance && travel_distance < this->best_dist) {
					this->best_depot = dest;
					this->best_dist = travel_distance;
				}
			}
		}
	}

public:
	explicit BestDepotFinder(const CUnit &w, const int res, const int ran)
		: resource(res), range(ran)
	{
		u_near.worker = &w;
	}

	explicit BestDepotFinder(const Vec2i &pos, const int res, const int ran) :
		resource(res), range(ran)
	{
		u_near.loc = pos;
	}

	template <typename ITERATOR>
	CUnit *Find(ITERATOR begin, ITERATOR end)
	{
		for (ITERATOR it = begin; it != end; ++it) {
			this->operator()(*it);
		}
		return best_depot;
	}

	CUnit *Find(CUnitCache &cache)
	{
		cache.for_each(*this);
		return best_depot;
	}
private:
	struct {
		const CUnit *worker;
		Vec2i loc;
	} u_near;
	const int resource;
	const int range;
	int best_dist = INT_MAX;
public:
	CUnit *best_depot = nullptr;
};

CUnit *FindDepositNearLoc(CPlayer &p, const Vec2i &pos, int range, int resource)
{
	BestDepotFinder<true> finder(pos, resource, range);
	std::vector<CUnit *> table;
	for (std::vector<CUnit *>::iterator it = p.UnitBegin(); it != p.UnitEnd(); ++it) {
		table.push_back(*it);
	}
	for (int i = 0; i < PlayerMax - 1; ++i) {
		if (Players[i].IsAllied(p) && p.IsAllied(Players[i])) {
			for (std::vector<CUnit *>::iterator it = Players[i].UnitBegin(); it != Players[i].UnitEnd(); ++it) {
				table.push_back(*it);
			}
		}
	}
	return finder.Find(table.begin(), table.end());
}

class CResourceFinder
{
public:
	CResourceFinder(int r, bool on_top) : resource(r), mine_on_top(on_top) {}
	bool operator()(const CUnit *const unit) const
	{
		const CUnitType &type = *unit->Type;
		return (type.GivesResource == resource
				&& unit->ResourcesHeld != 0
				&& (mine_on_top ? type.BoolFlag[CANHARVEST_INDEX].value : !type.BoolFlag[CANHARVEST_INDEX].value)
				&& !unit->IsUnusable(true) //allow mines under construction
			   );
	}
private:
	const int resource;
	const bool mine_on_top;
};

class ResourceUnitFinder
{
public:
	ResourceUnitFinder(const CUnit &worker, const CUnit *deposit, int resource, int maxRange, bool check_usage, CUnit **resultMine) :
		worker(worker),
		resinfo(*worker.Type->ResInfo[resource]),
		deposit(deposit),
		movemask(worker.Type->MovementMask & ~(MapFieldLandUnit | MapFieldAirUnit | MapFieldSeaUnit)),
		maxRange(maxRange),
		check_usage(check_usage),
		res_finder(resource, 1),
		resultMine(resultMine)
	{
		bestCost.SetToMax();
		*resultMine = NULL;
	}
	VisitResult Visit(TerrainTraversal &terrainTraversal, const Vec2i &pos, const Vec2i &from);
private:
	bool MineIsUsable(const CUnit &mine) const;

	struct ResourceUnitFinder_Cost {
	public:
		void SetFrom(const CUnit &mine, const CUnit *deposit, bool check_usage);
		bool operator < (const ResourceUnitFinder_Cost &rhs) const
		{
			if (waiting != rhs.waiting) {
				return waiting < rhs.waiting;
			} else if (distance != rhs.distance) {
				return distance < rhs.distance;
			} else {
				return assigned < rhs.assigned;
			}
		}
		void SetToMax() { assigned = waiting = distance = UINT_MAX; }
		bool IsMin() const { return assigned == 0 && waiting == 0 && distance == 0; }

	public:
		unsigned int assigned;
		unsigned int waiting;
		unsigned int distance;
	};

private:
	const CUnit &worker;
	const ResourceInfo &resinfo;
	const CUnit *deposit;
	unsigned int movemask;
	int maxRange;
	bool check_usage;
	CResourceFinder res_finder;
	ResourceUnitFinder_Cost bestCost;
	CUnit **resultMine;
};

bool ResourceUnitFinder::MineIsUsable(const CUnit &mine) const
{
	return mine.Type->BoolFlag[CANHARVEST_INDEX].value && mine.ResourcesHeld
		   && (resinfo.HarvestFromOutside
			   || mine.Player->Index == PlayerMax - 1
			   || mine.Player == worker.Player
			   || (worker.IsAllied(mine) && mine.IsAllied(worker)));
}

void ResourceUnitFinder::ResourceUnitFinder_Cost::SetFrom(const CUnit &mine, const CUnit *deposit, bool check_usage)
{
	distance = deposit ? mine.MapDistanceTo(*deposit) : 0;
	if (check_usage) {
		assigned = mine.Resource.Assigned - mine.Type->MaxOnBoard;
		waiting = GetNumWaitingWorkers(mine);
	} else {
		assigned = 0;
		waiting = 0;
	}
}

VisitResult ResourceUnitFinder::Visit(TerrainTraversal &terrainTraversal, const Vec2i &pos, const Vec2i &from)
{
	if (!worker.Player->AiEnabled && !Map.Field(pos)->playerInfo.IsExplored(*worker.Player)) {
		return VisitResult_DeadEnd;
	}

	CUnit *mine = Map.Field(pos)->UnitCache.find(res_finder);

	if (mine && mine != *resultMine && MineIsUsable(*mine)) {
		ResourceUnitFinder::ResourceUnitFinder_Cost cost;

		cost.SetFrom(*mine, deposit, check_usage);
		if (cost < bestCost) {
			*resultMine = mine;

			if (cost.IsMin()) {
				return VisitResult_Finished;
			}
			bestCost = cost;
		}
	}
	if (CanMoveToMask(pos, movemask)) { // reachable
		if (terrainTraversal.Get(pos) < maxRange) {
			return VisitResult_Ok;
		} else {
			return VisitResult_DeadEnd;
		}
	} else { // unreachable
		return VisitResult_DeadEnd;
	}
}

/**
**  Find Resource.
**
**  @param unit        The unit that wants to find a resource.
**  @param startUnit   Find closest unit from this location
**  @param range       Maximum distance to the resource.
**  @param resource    The resource id.
**
**  @note This will return an usable resource building that doesn't
**  belong to the player or one of his allies.
**
**  @return            NULL or resource unit
*/
CUnit *UnitFindResource(const CUnit &unit, const CUnit &startUnit, int range, int resource,
						bool check_usage, const CUnit *deposit)
{
	if (!deposit) { // Find the nearest depot
		deposit = FindDepositNearLoc(*unit.Player, startUnit.tilePos, range, resource);
	}

	TerrainTraversal terrainTraversal;

	terrainTraversal.SetSize(Map.Info.MapWidth, Map.Info.MapHeight);
	terrainTraversal.Init();

	terrainTraversal.PushUnitPosAndNeighboor(startUnit);

	CUnit *resultMine = NULL;

	ResourceUnitFinder resourceUnitFinder(unit, deposit, resource, range, check_usage, &resultMine);

	terrainTraversal.Run(resourceUnitFinder);
	return resultMine;
}

/**
**  Find deposit. This will find a deposit for a resource
**
**  @param unit        The unit that wants to find a resource.
**  @param range       Maximum distance to the deposit.
**  @param resource    Resource to find deposit from.
**
**  @note This will return a reachable allied depot.
**
**  @return            NULL or deposit unit
*/
CUnit *FindDeposit(const CUnit &unit, int range, int resource)
{
	BestDepotFinder<false> finder(unit, resource, range);
	std::vector<CUnit *> table;
	for (std::vector<CUnit *>::iterator it = unit.Player->UnitBegin(); it != unit.Player->UnitEnd(); ++it) {
		table.push_back(*it);
	}
	for (int i = 0; i < PlayerMax - 1; ++i) {
		if (Players[i].IsAllied(*unit.Player) && unit.Player->IsAllied(Players[i])) {
			for (std::vector<CUnit *>::iterator it = Players[i].UnitBegin(); it != Players[i].UnitEnd(); ++it) {
				table.push_back(*it);
			}
		}
	}
	return finder.Find(table.begin(), table.end());
}

/**
**  Find the next idle worker
**
**  @param player    Player's units to search through
**  @param last      Previous idle worker selected
**
**  @return NULL or next idle worker
*/
CUnit *FindIdleWorker(const CPlayer &player, const CUnit *last)
{
	CUnit *FirstUnitFound = NULL;
	int SelectNextUnit = (last == NULL) ? 1 : 0;
	const int nunits = player.GetUnitCount();

	for (int i = 0; i < nunits; ++i) {
		CUnit &unit = player.GetUnit(i);
		if (unit.Type->BoolFlag[HARVESTER_INDEX].value && !unit.Removed) {
			if (unit.CurrentAction() == UnitActionStill) {
				if (SelectNextUnit && !IsOnlySelected(unit)) {
					return &unit;
				}
				if (FirstUnitFound == NULL) {
					FirstUnitFound = &unit;
				}
			}
		}
		if (&unit == last) {
			SelectNextUnit = 1;
		}
	}
	if (FirstUnitFound != NULL && !IsOnlySelected(*FirstUnitFound)) {
		return FirstUnitFound;
	}
	return NULL;
}

/**
**  Find all units of type.
**
**  @param type       type of unit requested
**  @param units      array in which we have to store the units
**  @param everybody  if true, include all units
*/
void FindUnitsByType(const CUnitType &type, std::vector<CUnit *> &units, bool everybody)
{
	for (CUnitManager::Iterator it = UnitManager->begin(); it != UnitManager->end(); ++it) {
		CUnit &unit = **it;

		if (unit.Type == &type && !unit.IsUnusable(everybody)) {
			units.push_back(&unit);
		}
	}
}

/**
**  Find all units of type.
**
**  @param player  we're looking for the units of this player
**  @param type    type of unit requested
**  @param table   table in which we have to store the units
*/
void FindPlayerUnitsByType(const CPlayer &player, const CUnitType &type, std::vector<CUnit *> &table, bool ai_active)
{
	const int nunits = player.GetUnitCount();
	int typecount = player.UnitTypesCount[type.Slot];

	if (ai_active) {
		typecount = player.UnitTypesAiActiveCount[type.Slot];
	}

	if (typecount < 0) { // if unit type count is negative, something wrong happened
		fprintf(stderr, "Player %d has a negative %s unit type count of %d.\n", player.Index, type.Ident.c_str(), typecount);
	}

	if (typecount == 0) {
		return;
	}

	for (int i = 0; i < nunits; ++i) {
		CUnit &unit = player.GetUnit(i);

		if (unit.Type != &type) {
			continue;
		}
		if (!unit.IsUnusable()) {
			table.push_back(&unit);
		}
		--typecount;
		if (typecount == 0) {
			return ;
		}
	}
}

/**
**  Unit on map tile.
**
**  @param index flat index position on map, tile-based.
**  @param type  UnitTypeType, (unsigned)-1 for any type.
**
**  @return      Returns first found unit on tile.
*/
CUnit *UnitOnMapTile(const unsigned int index, unsigned int type)
{
	return Map.Field(index)->UnitCache.find(CUnitTypeFinder((UnitTypeType)type));
}

/**
**  Unit on map tile.
**
**  @param pos   position on map, tile-based.
**  @param type  UnitTypeType, (unsigned)-1 for any type.
**
**  @return      Returns first found unit on tile.
*/
CUnit *UnitOnMapTile(const Vec2i &pos, unsigned int type)
{
	return UnitOnMapTile(Map.getIndex(pos), type);
}

/**
**  Choose target on map area.
**
**  @param source  Unit which want to attack.
**  @param pos1    position on map, tile-based.
**  @param pos2    position on map, tile-based.
**
**  @return        Returns ideal target on map tile.
*/
CUnit *TargetOnMap(const CUnit &source, const Vec2i &pos1, const Vec2i &pos2)
{
	std::vector<CUnit *> table;

	Select(pos1, pos2, table);
	CUnit *best = NULL;
	for (size_t i = 0; i != table.size(); ++i) {
		CUnit &unit = *table[i];

		if (!unit.IsVisibleAsGoal(*source.Player)) {
			continue;
		}
		if (!CanTarget(*source.Type, *unit.Type)) {
			continue;
		}

		// Choose the best target.
		if (!best || best->Variable[PRIORITY_INDEX].Value < unit.Variable[PRIORITY_INDEX].Value) {
			best = &unit;
		}
	}
	return best;
}

/*----------------------------------------------------------------------------
--  Finding special units
----------------------------------------------------------------------------*/

/**
**  Resource on map tile
**
**  @param pos       position on map, tile-based.
**  @param resource  resource type.
**  @param mine_on_top  return mine or mining area.
**
**  @return          Returns the deposit if found, or NULL.
*/
CUnit *ResourceOnMap(const Vec2i &pos, int resource, bool mine_on_top)
{
	return Map.Field(pos)->UnitCache.find(CResourceFinder(resource, mine_on_top));
}

class IsADepositForResource
{
public:
	explicit IsADepositForResource(const int r) : resource(r) {}
	bool operator()(const CUnit *const unit) const
	{
		return (unit->Type->CanStore[resource] && !unit->IsUnusable());
	}
private:
	const int resource;
};

/**
**  Resource deposit on map tile
**
**  @param pos       position on map, tile-based.
**  @param resource  resource type.
**
**  @return          Returns the deposit if found, or NULL.
*/
CUnit *ResourceDepositOnMap(const Vec2i &pos, int resource)
{
	return Map.Field(pos)->UnitCache.find(IsADepositForResource(resource));
}

/*----------------------------------------------------------------------------
--  Finding units for attack
----------------------------------------------------------------------------*/

class BestTargetFinder
{
public:
	BestTargetFinder(const CUnit &a) :
		attacker(&a)
	{}

	CUnit *Find(const std::vector<CUnit *> &table) const
	{
		return Find(table.begin(), table.end());
	}

	CUnit *Find(CUnitCache &cache) const
	{
		return Find(cache.begin(), cache.end());
	}

private:
	template <typename Iterator>
	CUnit *Find(Iterator begin, Iterator end) const
	{
		CUnit *enemy = NULL;
		int best_cost = GameSettings.SimplifiedAutoTargeting ? INT_MIN : INT_MAX;

		for (Iterator it = begin; it != end; ++it) {
			int cost = GameSettings.SimplifiedAutoTargeting ? TargetPriorityCalculate(attacker, *it) : ComputeCost(*it);

			if (GameSettings.SimplifiedAutoTargeting ? (cost > best_cost) : (cost < best_cost)) {
				enemy = *it;
				best_cost = cost;
			}
		}
		return enemy;
	}

	int ComputeCost(CUnit *const dest) const
	{
		const CPlayer &player = *attacker->Player;
		const CUnitType &type = *attacker->Type;
		const CUnitType &dtype = *dest->Type;
		const int attackrange = attacker->Stats->Variables[ATTACKRANGE_INDEX].Max;

		if (!player.IsEnemy(*dest) // a friend or neutral
			|| !dest->IsVisibleAsGoal(player)
			|| !CanTarget(type, dtype)) {
			return INT_MAX;
		}
		// Don't attack invulnerable units
		if (dtype.BoolFlag[INDESTRUCTIBLE_INDEX].value || dest->Variable[UNHOLYARMOR_INDEX].Value) {
			return INT_MAX;
		}
		// Unit in range ?
		const int d = attacker->MapDistanceTo(*dest);

		if (d > attackrange && !UnitReachable(*attacker, *dest, attackrange, false)) {
			return INT_MAX;
		}

		// Attack walls only if we are stuck in them
		if (dtype.BoolFlag[WALL_INDEX].value && d > 1) {
			return INT_MAX;
		}

		if (dtype.UnitType == UnitTypeFly && dest->IsAgressive() == false) {
			return INT_MAX / 2;
		}

		// Calculate the costs to attack the unit.
		// Unit with the smallest attack costs will be taken.
		int cost = 0;

		// Priority 0-255
		cost -= dtype.DefaultStat.Variables[PRIORITY_INDEX].Value * PRIORITY_FACTOR;
		// Remaining HP (Health) 0-65535
		cost += dest->Variable[HP_INDEX].Value * 100 / dest->Variable[HP_INDEX].Max * HEALTH_FACTOR;

		if (d <= attackrange && d >= type.MinAttackRange) {
			cost += d * INRANGE_FACTOR;
			cost -= INRANGE_BONUS;
		} else {
			cost += d * DISTANCE_FACTOR;
		}

		for (unsigned int i = 0; i < UnitTypeVar.GetNumberBoolFlag(); i++) {
			if (type.BoolFlag[i].AiPriorityTarget != CONDITION_TRUE) {
				if ((type.BoolFlag[i].AiPriorityTarget == CONDITION_ONLY) &
					(dtype.BoolFlag[i].value)) {
					cost -= AIPRIORITY_BONUS;
				}
				if ((type.BoolFlag[i].AiPriorityTarget == CONDITION_FALSE) &
					(dtype.BoolFlag[i].value)) {
					cost += AIPRIORITY_BONUS;
				}
			}
		}

		// Unit can attack back.
		if (CanTarget(dtype, type)) {
			cost -= CANATTACK_BONUS;
		}
		return cost;
	}

private:
	const CUnit *attacker;
};

/**
**  Attack units in distance, with large missile
**
**  Choose the best target, that can be attacked. It takes into
**  account allied unit which could be hit by the missile
**
**
**  @note This could be improved, for better performance / better trade.
**  @note Will be moved to unit_ai.c soon.
*/
class BestRangeTargetFinder
{
public:
	/**
	**  @param a      Find in distance for this unit.
	**  @param range  Distance range to look.
	**
	*/
	BestRangeTargetFinder(const CUnit &a, const int r) : attacker(&a), range(r),
		best_unit(0), best_cost(INT_MIN), size((a.Type->Missile.Missile->Range + r) * 2)
	{
		good = new std::vector<int>(size * size, 0);
		bad = new std::vector<int>(size * size, 0);
	};

	~BestRangeTargetFinder()
	{
		delete good;
		delete bad;
	};

	class FillBadGood
	{
	public:
		FillBadGood(const CUnit &a, int r, std::vector<int> *g, std::vector<int> *b, int s):
			attacker(&a), range(r), size(s),
			enemy_count(0), good(g), bad(b)
		{
		}

		int Fill(CUnitCache &cache)
		{
			return Fill(cache.begin(), cache.end());
		}

		template <typename Iterator>
		int Fill(Iterator begin, Iterator end)
		{
			for (Iterator it = begin; it != end; ++it) {
				Compute(*it);
			}
			return enemy_count;
		}
	private:

		void Compute(CUnit *const dest)
		{
			const CPlayer &player = *attacker->Player;

			if (!dest->IsVisibleAsGoal(player)) {
				dest->CacheLock = 1;
				return;
			}

			const CUnitType &type =  *attacker->Type;
			const CUnitType &dtype = *dest->Type;
			// won't be a target...
			if (!CanTarget(type, dtype)) { // can't be attacked.
				dest->CacheLock = 1;
				return;
			}
			// Don't attack invulnerable units
			if (dtype.BoolFlag[INDESTRUCTIBLE_INDEX].value || dest->Variable[UNHOLYARMOR_INDEX].Value) {
				dest->CacheLock = 1;
				return;
			}

			//  Calculate the costs to attack the unit.
			//  Unit with the smallest attack costs will be taken.

			int cost = 0;
			int hp_damage_evaluate;
			if (Damage) {
				hp_damage_evaluate = CalculateDamage(*attacker, *dest, Damage);
			} else {
				hp_damage_evaluate = attacker->Stats->Variables[BASICDAMAGE_INDEX].Value
									 + attacker->Stats->Variables[PIERCINGDAMAGE_INDEX].Value;
			}
			if (!player.IsEnemy(*dest)) { // a friend or neutral
				dest->CacheLock = 1;

				// Calc a negative cost
				// The gost is more important when the unit would be killed
				// by our fire.

				// It costs (is positive) if hp_damage_evaluate>dest->HP ...)
				// FIXME : assume that PRIORITY_FACTOR>HEALTH_FACTOR
				cost = HEALTH_FACTOR * (2 * hp_damage_evaluate -
										dest->Variable[HP_INDEX].Value) /
					   (dtype.TileWidth * dtype.TileWidth);
				cost = std::max(cost, 1);
				cost = -cost;
			} else {
				//  Priority 0-255
				cost += dtype.DefaultStat.Variables[PRIORITY_INDEX].Value * PRIORITY_FACTOR;

				for (unsigned int i = 0; i < UnitTypeVar.GetNumberBoolFlag(); i++) {
					if (type.BoolFlag[i].AiPriorityTarget != CONDITION_TRUE) {
						if ((type.BoolFlag[i].AiPriorityTarget == CONDITION_ONLY) &
							(dtype.BoolFlag[i].value)) {
							cost -= AIPRIORITY_BONUS;
						} else if ((type.BoolFlag[i].AiPriorityTarget == CONDITION_FALSE) &
								   (dtype.BoolFlag[i].value)) {
							cost += AIPRIORITY_BONUS;
						}
					}
				}

				//  Remaining HP (Health) 0-65535
				// Give a boost to unit we can kill in one shoot only

				// calculate HP which will remain in the enemy unit, after hit
				int effective_hp = (dest->Variable[HP_INDEX].Value - 2 * hp_damage_evaluate);

				// Unit we won't kill are evaluated the same
				// Unit we are sure to kill are all evaluated the same (except PRIORITY)
				clamp(&effective_hp, -hp_damage_evaluate, 0);

				// Here, effective_hp vary from -hp_damage_evaluate (unit will be killed) to 0 (unit can't be killed)
				// => we prefer killing rather than only hitting...
				cost += -effective_hp * HEALTH_FACTOR;

				//  Unit can attack back.
				if (CanTarget(dtype, type)) {
					cost += CANATTACK_BONUS;
				}

				// the cost may be divided across multiple cells
				cost = cost / (dtype.TileWidth * dtype.TileWidth);
				cost = std::max(cost, 1);

				// Removed Unit's are in bunkers
				int d;
				if (attacker->Removed) {
					d = attacker->Container->MapDistanceTo(*dest);
				} else {
					d = attacker->MapDistanceTo(*dest);
				}

				int attackrange = attacker->Stats->Variables[ATTACKRANGE_INDEX].Max;
				if (d <= attackrange ||
					(d <= range && UnitReachable(*attacker, *dest, attackrange, false))) {
					++enemy_count;
				} else {
					dest->CacheLock = 1;
				}
				// Attack walls only if we are stuck in them
				if (dtype.BoolFlag[WALL_INDEX].value && d > 1) {
					dest->CacheLock = 1;
				}
			}

			// cost map is relative to attacker position
			const int x = dest->tilePos.x - attacker->tilePos.x + (size / 2);
			const int y = dest->tilePos.y - attacker->tilePos.y + (size / 2);

			// Mark the good/bad array...
			for (int yy = 0; yy < dtype.TileHeight; ++yy) {
				for (int xx = 0; xx < dtype.TileWidth; ++xx) {
					int pos = (y + yy) * (size / 2) + (x + xx);
					if (pos < 0 || static_cast<unsigned int>(pos) >= good->size()) {
						DebugPrint("BUG: RangeTargetFinder::FillBadGood.Compute out of range. "\
								   "size: %d, pos: %d, "				\
								   "x: %d, xx: %d, y: %d, yy: %d\n" _C_
								   size _C_ pos _C_ x _C_ xx _C_ y _C_ yy);
						break;
					}
					if (cost < 0) {
						good->at(pos) -= cost;
					} else {
						bad->at(pos) += cost;
					}
				}
			}
		}


	private:
		const CUnit *attacker;
		const int range;
		int enemy_count;
		std::vector<int> *good;
		std::vector<int> *bad;
		const int size;
	};

	CUnit *Find(std::vector<CUnit *> &table)
	{
		if (!GameSettings.SimplifiedAutoTargeting) {
			FillBadGood(*attacker, range, good, bad, size).Fill(table.begin(), table.end());
		}
		return Find(table.begin(), table.end());

	}

	CUnit *Find(CUnitCache &cache)
	{
		FillBadGood(*attacker, range, good, bad, size).Fill(cache);
		return Find(cache.begin(), cache.end());
	}

private:
	template <typename Iterator>
	CUnit *Find(Iterator begin, Iterator end)
	{
		for (Iterator it = begin; it != end; ++it) {
			Compute(*it);
		}
		return best_unit;
	}

	void Compute(CUnit *const dest)
	{
		if (dest->CacheLock) {
			dest->CacheLock = 0;
			return;
		}

		if (GameSettings.SimplifiedAutoTargeting) {
			const int cost = TargetPriorityCalculate(attacker, dest);
			if (cost > best_cost) {
				best_unit = dest;
				best_cost = cost;
			}
			return;
		}

		const CUnitType &type = *attacker->Type;
		const CUnitType &dtype = *dest->Type;
		int x = attacker->tilePos.x;
		int y = attacker->tilePos.y;

		// put in x-y the real point which will be hit...
		// (only meaningful when dtype->TileWidth > 1)
		clamp<int>(&x, dest->tilePos.x, dest->tilePos.x + dtype.TileWidth - 1);
		clamp<int>(&y, dest->tilePos.y, dest->tilePos.y + dtype.TileHeight - 1);

		int sbad = 0;
		int sgood = 0;

		// cost map is relative to attacker position
		x = dest->tilePos.x - attacker->tilePos.x + (size / 2);
		y = dest->tilePos.y - attacker->tilePos.y + (size / 2);

		// calculate the costs:
		// costs are the full costs at the target and the splash-factor
		// adjusted costs of the tiles immediately around the target
		int splashFactor = type.Missile.Missile->SplashFactor;
		for (int yy = -1; yy <= 1; ++yy) {
			for (int xx = -1; xx <= 1; ++xx) {
				int pos = (y + yy) * (size / 2) + (x + xx);
				int localFactor = (!xx && !yy) ? 1 : splashFactor;
				if (pos < 0 || static_cast<unsigned int>(pos) >= good->size()) {
					DebugPrint("BUG: RangeTargetFinder.Compute out of range. " \
							   "size: %d, pos: %d, "	\
							   "x: %d, xx: %d, y: %d, yy: %d \n" _C_
							   size _C_ pos _C_ x _C_ xx _C_ y _C_ yy);
					break;
				}
				sbad += bad->at(pos) / localFactor;
				sgood += good->at(pos) / localFactor;
			}
		}

		if (sgood > 0 && attacker->Type->BoolFlag[NOFRIENDLYFIRE_INDEX].value) {
			return;
		}

		// don't consider small friendly-fire damages...
		sgood = std::max(sgood, 20);
		int cost = sbad / sgood;
		if (cost > best_cost) {
			best_cost = cost;
			best_unit = dest;
		}
	}

private:
	const CUnit *attacker;
	const int range;
	CUnit *best_unit;
	int best_cost;
	std::vector<int> *good;
	std::vector<int> *bad;
	const int size;
};

struct CompareUnitDistance {
	const CUnit *referenceunit;
	CompareUnitDistance(const CUnit &unit): referenceunit(&unit) {}
	bool operator()(const CUnit *c1, const CUnit *c2)
	{
		int d1 = c1->MapDistanceTo(*referenceunit);
		int d2 = c2->MapDistanceTo(*referenceunit);
		if (d1 == d2) {
			return UnitNumber(*c1) < UnitNumber(*c2);
		} else {
			return d1 < d2;
		}
	}
};

/**
**  Check map for obstacles in a line between 2 tiles
**
**  This function uses Bresenham's line algorithm
**
**  @param unit     First tile
**  @param goal     Second tile
**  @param flags    Terrain type to check
**
**  @return         false, if an obstacle was found, true otherwise
*/
bool CheckObstaclesBetweenTiles(const Vec2i &unitPos, const Vec2i &goalPos, unsigned short flags, int *distance)
{
	const Vec2i delta(abs(goalPos.x - unitPos.x), abs(goalPos.y - unitPos.y));
	const Vec2i sign(unitPos.x < goalPos.x ? 1 : -1, unitPos.y < goalPos.y ? 1 : -1);
	int error = delta.x - delta.y;
	Vec2i pos(unitPos), oldPos(unitPos);

	while (pos.x != goalPos.x || pos.y != goalPos.y) {
		const int error2 = error * 2;

		if (error2 > -delta.y) {
			error -= delta.y;
			pos.x += sign.x;
		}
		if (error2 < delta.x) {
			error += delta.x;
			pos.y += sign.y;
		}

		if (Map.Info.IsPointOnMap(pos) == false) {
			DebugPrint("outside of map\n");
		} else if (Map.Field(pos)->Flags & flags) {
			if (distance) {
				*distance = Distance(unitPos, pos);
			}
			return false;
		}
		oldPos = pos;
	}
	return true;
}

/**
**  Attack units in distance.
**
**  If the unit can attack must be handled by caller.
**  Choose the best target, that can be attacked.
**
**  @param unit           Find in distance for this unit.
**  @param range          Distance range to look.
**  @param onlyBuildings  Search only buildings (useful when attacking with AI force)
**
**  @return       Unit to be attacked.
*/
CUnit *AttackUnitsInDistance(const CUnit &unit, int range, CUnitFilter pred)
{
	// if necessary, take possible damage on allied units into account...
	if (unit.Type->Missile.Missile->Range > 1
		&& (range + unit.Type->Missile.Missile->Range < 15)) {
		//  If catapult, count units near the target...
		//   FIXME : make it configurable

		int missile_range = unit.Type->Missile.Missile->Range + range - 1;

		Assert(2 * missile_range + 1 < 32);

		// If unit is removed, use containers x and y
		const CUnit *firstContainer = unit.Container ? unit.Container : &unit;
		std::vector<CUnit *> table;
		SelectAroundUnit(*firstContainer, missile_range, table,
						 MakeAndPredicate(HasNotSamePlayerAs(Players[PlayerNumNeutral]), pred));

		if (table.empty() == false) {
			return BestRangeTargetFinder(unit, range).Find(table);
		}
		return NULL;
	} else {
		// If unit is removed, use containers x and y
		const CUnit *firstContainer = unit.Container ? unit.Container : &unit;
		std::vector<CUnit *> table;

		SelectAroundUnit(*firstContainer, range, table,
						 MakeAndPredicate(HasNotSamePlayerAs(Players[PlayerNumNeutral]), pred));

		const int n = static_cast<int>(table.size());
		if (range > 25 && table.size() > 9) {
			std::sort(table.begin(), table.begin() + n, CompareUnitDistance(unit));
		}

		// Find the best unit to attack
		return BestTargetFinder(unit).Find(table);
	}
}

CUnit *AttackUnitsInDistance(const CUnit &unit, int range)
{
	return AttackUnitsInDistance(unit, range, NoFilter());
}

/**
**  Attack units in attack range.
**
**  @param unit  Find unit in attack range for this unit.
**
**  @return      Pointer to unit which should be attacked.
*/
CUnit *AttackUnitsInRange(const CUnit &unit, CUnitFilter pred)
{
	Assert(unit.Type->CanAttack);
	return AttackUnitsInDistance(unit, unit.Stats->Variables[ATTACKRANGE_INDEX].Max, pred);
}

CUnit *AttackUnitsInRange(const CUnit &unit)
{
	return AttackUnitsInRange(unit, NoFilter());
}

/**
**  Attack units in reaction range.
**
**  @param unit  Find unit in reaction range for this unit.
**
**  @return      Pointer to unit which should be attacked.
*/
CUnit *AttackUnitsInReactRange(const CUnit &unit, CUnitFilter pred)
{
	Assert(unit.Type->CanAttack);
	const int range = unit.Player->Type == PlayerTypes::PlayerPerson ? unit.Type->ReactRangePerson : unit.Type->ReactRangeComputer;
	return AttackUnitsInDistance(unit, range, pred);
}

CUnit *AttackUnitsInReactRange(const CUnit &unit)
{
	return AttackUnitsInReactRange(unit, NoFilter());
}

//@}
