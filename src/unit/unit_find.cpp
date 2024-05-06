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

#include <climits>

/*----------------------------------------------------------------------------
  -- Finding units
  ----------------------------------------------------------------------------*/

std::vector<CUnit *> Select(const Vec2i &ltPos, const Vec2i &rbPos)
{
	return Select(ltPos, rbPos, NoFilter());
}

std::vector<CUnit *> SelectFixed(const Vec2i &ltPos, const Vec2i &rbPos)
{
	return Select(ltPos, rbPos, NoFilter());
}

std::vector<CUnit *> SelectAroundUnit(const CUnit &unit, int range)
{
	return SelectAroundUnit(unit, range, NoFilter());
}

/* static */ CUnit *UnitFinder::find(const std::vector<CUnit *> &candidates,
                                     int maxDist,
                                     CUnit &target)
{
	if (candidates.empty()) {
		return nullptr;
	}
	const auto& type = *candidates.front()->Type;
	const CPlayer &player = *candidates.front()->Player;
	TerrainTraversal terrainTraversal;

	terrainTraversal.SetSize(Map.Info.MapWidth, Map.Info.MapHeight);
	terrainTraversal.Init();

	terrainTraversal.PushUnitPosAndNeighboor(target);

	const int movemask =
		type.MovementMask & ~(MapFieldLandUnit | MapFieldAirUnit | MapFieldSeaUnit);
	UnitFinder unitFinder(player, candidates, maxDist, movemask);
	return terrainTraversal.Run(unitFinder) ? unitFinder.resUnit : nullptr;
}

CUnit *UnitFinder::FindUnitAtPos(const Vec2i &pos) const
{
	for (CUnit *unit : Map.Field(pos)->UnitCache) {
		if (ranges::contains(units, unit)) {
			return unit;
		}
	}
	return nullptr;
}

VisitResult UnitFinder::Visit(TerrainTraversal &terrainTraversal, const Vec2i &pos, const Vec2i &from)
{
	if (!player.AiEnabled && !Map.Field(pos)->playerInfo.IsExplored(player)) {
		return VisitResult::DeadEnd;
	}
	// Look if found what was required.
	CUnit *unit = FindUnitAtPos(pos);
	if (unit) {
		resUnit = unit;
		return VisitResult::Finished;
	}
	if (CanMoveToMask(pos, movemask)) { // reachable
		if (terrainTraversal.Get(pos) <= maxDist) {
			return VisitResult::Ok;
		} else {
			return VisitResult::DeadEnd;
		}
	} else { // unreachable
		return VisitResult::DeadEnd;
	}
}

class TerrainFinder
{
	friend std::optional<Vec2i>
	FindTerrainType(int movemask, int resmask, int range, const CPlayer &, const Vec2i &startPos);

public:

	TerrainFinder(const CPlayer &player, int maxDist, int movemask, int resmask) :
		player(player), maxDist(maxDist), movemask(movemask), resmask(resmask) {}
	VisitResult Visit(TerrainTraversal &terrainTraversal, const Vec2i &pos, const Vec2i &from);
private:
	const CPlayer &player;
	int maxDist;
	int movemask;
	int resmask;
	Vec2i resPos;
};

VisitResult TerrainFinder::Visit(TerrainTraversal &terrainTraversal, const Vec2i &pos, const Vec2i &from)
{
	if (!player.AiEnabled && !Map.Field(pos)->playerInfo.IsExplored(player)) {
		return VisitResult::DeadEnd;
	}
	// Look if found what was required.
	if (Map.Field(pos)->CheckMask(resmask)) {
		resPos = pos;
		return VisitResult::Finished;
	}
	if (CanMoveToMask(pos, movemask)) { // reachable
		if (terrainTraversal.Get(pos) <= maxDist) {
			return VisitResult::Ok;
		} else {
			return VisitResult::DeadEnd;
		}
	} else { // unreachable
		return VisitResult::DeadEnd;
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
**  @note Movement mask can be 0xFFFFFFFF to have no effect
**  Range is not circular, but square.
**  Player is ignored if nil(search the entire map)
**
**  @return            wood position if found, else std::nullopt.
*/
std::optional<Vec2i>
FindTerrainType(int movemask, int resmask, int range, const CPlayer &player, const Vec2i &startPos)
{
	TerrainTraversal terrainTraversal;

	terrainTraversal.SetSize(Map.Info.MapWidth, Map.Info.MapHeight);
	terrainTraversal.Init();

	terrainTraversal.PushPos(startPos);

	TerrainFinder terrainFinder(player, range, movemask & ~(MapFieldLandUnit | MapFieldAirUnit | MapFieldSeaUnit), resmask);

	return terrainTraversal.Run(terrainFinder) ? std::make_optional(terrainFinder.resPos) : std::nullopt;
}

template <const bool NEARLOCATION>
class BestDepotFinder
{
	void operator()(CUnit *const dest)
	{
		/* Only resource depots */
		if (dest->Type->CanStore[resource]
			&& dest->IsAliveOnMap()
			&& dest->CurrentAction() != UnitAction::Built) {
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
	std::vector<CUnit *> table = p.GetUnits();
	for (int i = 0; i < PlayerMax - 1; ++i) {
		if (Players[i].IsAllied(p) && p.IsAllied(Players[i])) {
			table.insert(table.end(), Players[i].GetUnits().begin(), Players[i].GetUnits().end());
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
		*resultMine = nullptr;
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
	const auto &field = *Map.Field(pos);
	if (!worker.Player->AiEnabled && !field.playerInfo.IsExplored(*worker.Player)) {
		return VisitResult::DeadEnd;
	}
	auto it = ranges::find_if(field.UnitCache, res_finder);
	CUnit *mine = it != field.UnitCache.end() ? *it : nullptr;

	if (mine && mine != *resultMine && MineIsUsable(*mine)) {
		ResourceUnitFinder::ResourceUnitFinder_Cost cost;

		cost.SetFrom(*mine, deposit, check_usage);
		if (cost < bestCost) {
			*resultMine = mine;

			if (cost.IsMin()) {
				return VisitResult::Finished;
			}
			bestCost = cost;
		}
	}
	if (CanMoveToMask(pos, movemask)) { // reachable
		if (terrainTraversal.Get(pos) < maxRange) {
			return VisitResult::Ok;
		} else {
			return VisitResult::DeadEnd;
		}
	} else { // unreachable
		return VisitResult::DeadEnd;
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
**  @return            nullptr or resource unit
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

	CUnit *resultMine = nullptr;

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
**  @return            nullptr or deposit unit
*/
CUnit *FindDeposit(const CUnit &unit, int range, int resource)
{
	BestDepotFinder<false> finder(unit, resource, range);
	std::vector<CUnit *> table = unit.Player->GetUnits();
	if (GameSettings.AllyDepositsAllowed) {
		for (int i = 0; i < PlayerMax - 1; ++i) {
			if (Players[i].IsAllied(*unit.Player) && unit.Player->IsAllied(Players[i])) {
				table.insert(
					table.end(), Players[i].GetUnits().begin(), Players[i].GetUnits().end());
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
**  @return nullptr or next idle worker
*/
CUnit *FindIdleWorker(const CPlayer &player, const CUnit *last)
{
	CUnit *FirstUnitFound = nullptr;
	int SelectNextUnit = (last == nullptr) ? 1 : 0;

	for (CUnit *unit : player.GetUnits()) {
		if (unit->Type->BoolFlag[HARVESTER_INDEX].value && !unit->Removed) {
			if (unit->CurrentAction() == UnitAction::Still) {
				if (SelectNextUnit && !IsOnlySelected(*unit)) {
					return unit;
				}
				if (FirstUnitFound == nullptr) {
					FirstUnitFound = unit;
				}
			}
		}
		if (unit == last) {
			SelectNextUnit = 1;
		}
	}
	if (FirstUnitFound != nullptr && !IsOnlySelected(*FirstUnitFound)) {
		return FirstUnitFound;
	}
	return nullptr;
}

/**
**  Find all units of type.
**
**  @param type       type of unit requested
**  @param everybody  if true, include all units
**  @return           all units of type
*/
std::vector<CUnit *> FindUnitsByType(const CUnitType &type, bool everybody)
{
	std::vector<CUnit *> units;
	for (CUnit *unit : UnitManager->GetUnits()) {
		if (unit->Type == &type && !unit->IsUnusable(everybody)) {
			units.push_back(unit);
		}
	}
	return units;
}

/**
**  Find all units of type.
**
**  @param player  we're looking for the units of this player
**  @param type    type of unit requested
**  @return the units
*/
std::vector<CUnit *> FindPlayerUnitsByType(const CPlayer &player, const CUnitType &type, bool ai_active)
{
	int typecount = player.UnitTypesCount[type.Slot];

	if (ai_active) {
		typecount = player.UnitTypesAiActiveCount[type.Slot];
	}

	if (typecount < 0) { // if unit type count is negative, something wrong happened
		ErrorPrint("Player %d has a negative '%s' unit type count of %d.\n",
		           player.Index,
		           type.Ident.c_str(),
		           typecount);
	}

	if (typecount == 0) {
		return {};
	}
	std::vector<CUnit *> table;

	for (CUnit *unit : player.GetUnits()) {
		if (unit->Type != &type) {
			continue;
		}
		if (!unit->IsUnusable()) {
			table.push_back(unit);
		}
		--typecount;
		if (typecount == 0) {
			return table;
		}
	}
	return table;
}

/**
**  Unit on map tile.
**
**  @param index flat index position on map, tile-based.
**  @param type  moveType, std::nullopt for any moveType.
**
**  @return      Returns first found unit on tile.
*/
static CUnit *UnitOnMapTile(const unsigned int index, std::optional<EMovement> moveType)
{
	const auto &field = *Map.Field(index);
	auto it = ranges::find_if(field.UnitCache, CUnitTypeFinder(moveType));
	return it != field.UnitCache.end() ? *it : nullptr;
}

/**
**  Unit on map tile.
**
**  @param pos   position on map, tile-based.
**  @param type  moveType, std::nullopt for any type.
**
**  @return      Returns first found unit on tile.
*/
CUnit *UnitOnMapTile(const Vec2i &pos, std::optional<EMovement> moveType)
{
	return UnitOnMapTile(Map.getIndex(pos), moveType);
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
	std::vector<CUnit *> table = Select(pos1, pos2);

	CUnit *best = nullptr;
	for (CUnit *unit : table) {
		if (!unit->IsVisibleAsGoal(*source.Player)) {
			continue;
		}
		if (!CanTarget(*source.Type, *unit->Type)) {
			continue;
		}

		// Choose the best target.
		if (!best || best->Variable[PRIORITY_INDEX].Value < unit->Variable[PRIORITY_INDEX].Value) {
			best = unit;
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
**  @return          Returns the deposit if found, or nullptr.
*/
CUnit *ResourceOnMap(const Vec2i &pos, int resource, bool mine_on_top)
{
	const auto &field = *Map.Field(pos);
	auto it = ranges::find_if(field.UnitCache, CResourceFinder(resource, mine_on_top));
	return it != field.UnitCache.end() ? *it : nullptr;
}

/**
**  Resource deposit on map tile
**
**  @param pos       position on map, tile-based.
**  @param resource  resource type.
**
**  @return          Returns the deposit if found, or nullptr.
*/
CUnit *ResourceDepositOnMap(const Vec2i &pos, int resource)
{
	const auto isADeposit = [=](const CUnit *unit) {
		return (unit->Type->CanStore[resource] && !unit->IsUnusable());
	};
	const auto &field = *Map.Field(pos);
	auto it = ranges::find_if(field.UnitCache, isADeposit);
	return it != field.UnitCache.end() ? *it : nullptr;
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

private:
	template <typename Iterator>
	CUnit *Find(Iterator begin, Iterator end) const
	{
		CUnit *enemy = nullptr;
		int best_cost = GameSettings.SimplifiedAutoTargeting ? INT_MIN : INT_MAX;

		for (Iterator it = begin; it != end; ++it) {
			int cost = GameSettings.SimplifiedAutoTargeting ? TargetPriorityCalculate(*attacker, **it) : ComputeCost(*it);

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

		if (dtype.MoveType == EMovement::Fly && dest->IsAgressive() == false) {
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
			if (type.BoolFlag[i].AiPriorityTarget != ECondition::Ignore) {
				if ((type.BoolFlag[i].AiPriorityTarget == ECondition::ShouldBeTrue)
				    & (dtype.BoolFlag[i].value)) {
					cost -= AIPRIORITY_BONUS;
				}
				if ((type.BoolFlag[i].AiPriorityTarget == ECondition::ShouldBeFalse)
				    & (dtype.BoolFlag[i].value)) {
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
	BestRangeTargetFinder(const CUnit &a, const int r) :
		attacker(&a),
		range(r),
		size((a.Type->Missile.Missile->Range + r) * 2),
		good(size * size, 0),
		bad(size * size, 0)
	{
	}

	class FillBadGood
	{
	public:
		FillBadGood(const CUnit &a, int r, std::vector<int> *g, std::vector<int> *b, int s):
			attacker(&a), range(r), size(s),
			enemy_count(0), good(g), bad(b)
		{
		}

		template <typename Container>
		int Fill(Container &&container)
		{
			for (auto *elem : container) {
				Compute(*elem);
			}
			return enemy_count;
		}
	private:

		void Compute(CUnit &dest)
		{
			const CPlayer &player = *attacker->Player;

			if (!dest.IsVisibleAsGoal(player)) {
				dest.CacheLock = 1;
				return;
			}

			const CUnitType &type = *attacker->Type;
			const CUnitType &dtype = *dest.Type;
			// won't be a target...
			if (!CanTarget(type, dtype)) { // can't be attacked.
				dest.CacheLock = 1;
				return;
			}
			// Don't attack invulnerable units
			if (dtype.BoolFlag[INDESTRUCTIBLE_INDEX].value || dest.Variable[UNHOLYARMOR_INDEX].Value) {
				dest.CacheLock = 1;
				return;
			}

			//  Calculate the costs to attack the unit.
			//  Unit with the smallest attack costs will be taken.

			int cost = 0;
			int hp_damage_evaluate;
			if (Damage) {
				hp_damage_evaluate = CalculateDamage(*attacker, dest, Damage.get());
			} else {
				hp_damage_evaluate = attacker->Stats->Variables[BASICDAMAGE_INDEX].Value
									 + attacker->Stats->Variables[PIERCINGDAMAGE_INDEX].Value;
			}
			if (!player.IsEnemy(dest)) { // a friend or neutral
				dest.CacheLock = 1;

				// Calc a negative cost
				// The gost is more important when the unit would be killed
				// by our fire.

				// It costs (is positive) if hp_damage_evaluate>dest->HP ...)
				// FIXME : assume that PRIORITY_FACTOR>HEALTH_FACTOR
				cost = HEALTH_FACTOR * (2 * hp_damage_evaluate - dest.Variable[HP_INDEX].Value)
				     / (dtype.TileWidth * dtype.TileWidth);
				cost = std::max(cost, 1);
				cost = -cost;
			} else {
				//  Priority 0-255
				cost += dtype.DefaultStat.Variables[PRIORITY_INDEX].Value * PRIORITY_FACTOR;

				for (unsigned int i = 0; i < UnitTypeVar.GetNumberBoolFlag(); i++) {
					if (type.BoolFlag[i].AiPriorityTarget != ECondition::Ignore) {
						if ((type.BoolFlag[i].AiPriorityTarget == ECondition::ShouldBeTrue)
						    & (dtype.BoolFlag[i].value)) {
							cost -= AIPRIORITY_BONUS;
						} else if ((type.BoolFlag[i].AiPriorityTarget == ECondition::ShouldBeFalse)
						           & (dtype.BoolFlag[i].value)) {
							cost += AIPRIORITY_BONUS;
						}
					}
				}

				//  Remaining HP (Health) 0-65535
				// Give a boost to unit we can kill in one shoot only

				// calculate HP which will remain in the enemy unit, after hit
				int effective_hp = (dest.Variable[HP_INDEX].Value - 2 * hp_damage_evaluate);

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
					d = attacker->Container->MapDistanceTo(dest);
				} else {
					d = attacker->MapDistanceTo(dest);
				}

				int attackrange = attacker->Stats->Variables[ATTACKRANGE_INDEX].Max;
				if (d <= attackrange ||
					(d <= range && UnitReachable(*attacker, dest, attackrange, false))) {
					++enemy_count;
				} else {
					dest.CacheLock = 1;
				}
				// Attack walls only if we are stuck in them
				if (dtype.BoolFlag[WALL_INDEX].value && d > 1) {
					dest.CacheLock = 1;
				}
			}

			// cost map is relative to attacker position
			const int x = dest.tilePos.x - attacker->tilePos.x + (size / 2);
			const int y = dest.tilePos.y - attacker->tilePos.y + (size / 2);

			// Mark the good/bad array...
			for (int yy = 0; yy < dtype.TileHeight; ++yy) {
				for (int xx = 0; xx < dtype.TileWidth; ++xx) {
					int pos = (y + yy) * (size / 2) + (x + xx);
					if (pos < 0 || static_cast<unsigned int>(pos) >= good->size()) {
						ErrorPrint("BUG: RangeTargetFinder::FillBadGood.Compute out of range. "
						           "size: %d, pos: %d, x: %d, xx: %d, y: %d, yy: %d\n",
						           size,
						           pos,
						           x,
						           xx,
						           y,
						           yy);
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
		const int size;
		int enemy_count;
		std::vector<int> *good;
		std::vector<int> *bad;
	};

	CUnit *Find(std::vector<CUnit *> &table)
	{
		if (!GameSettings.SimplifiedAutoTargeting) {
			FillBadGood(*attacker, range, &good, &bad, size).Fill(table);
		}
		for (auto* unit : table) {
			Compute(*unit);
		}
		return best_unit;
	}

private:

	void Compute(CUnit &dest)
	{
		if (dest.CacheLock) {
			dest.CacheLock = 0;
			return;
		}

		if (GameSettings.SimplifiedAutoTargeting) {
			const int cost = TargetPriorityCalculate(*attacker, dest);
			if (cost > best_cost) {
				best_unit = &dest;
				best_cost = cost;
			}
			return;
		}

		const CUnitType &type = *attacker->Type;
		const CUnitType &dtype = *dest.Type;
		int x = attacker->tilePos.x;
		int y = attacker->tilePos.y;

		// put in x-y the real point which will be hit...
		// (only meaningful when dtype->TileWidth > 1)
		clamp<int>(&x, dest.tilePos.x, dest.tilePos.x + dtype.TileWidth - 1);
		clamp<int>(&y, dest.tilePos.y, dest.tilePos.y + dtype.TileHeight - 1);

		int sbad = 0;
		int sgood = 0;

		// cost map is relative to attacker position
		x = dest.tilePos.x - attacker->tilePos.x + (size / 2);
		y = dest.tilePos.y - attacker->tilePos.y + (size / 2);

		// calculate the costs:
		// costs are the full costs at the target and the splash-factor
		// adjusted costs of the tiles immediately around the target
		int splashFactor = type.Missile.Missile->SplashFactor;
		for (int yy = -1; yy <= 1; ++yy) {
			for (int xx = -1; xx <= 1; ++xx) {
				int pos = (y + yy) * (size / 2) + (x + xx);
				int localFactor = (!xx && !yy) ? 1 : splashFactor;
				if (pos < 0 || static_cast<unsigned int>(pos) >= good.size()) {
					ErrorPrint("BUG: RangeTargetFinder.Compute out of range. "
					           "size: %d, pos: %d, x: %d, xx: %d, y: %d, yy: %d \n",
					           size,
					           pos,
					           x,
					           xx,
					           y,
					           yy);
					break;
				}
				sbad += bad.at(pos) / localFactor;
				sgood += good.at(pos) / localFactor;
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
			best_unit = &dest;
		}
	}

private:
	const CUnit *attacker;
	const int range;
	CUnit *best_unit = nullptr;
	int best_cost = INT_MIN;
	const int size;
	std::vector<int> good;
	std::vector<int> bad;
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
		std::vector<CUnit *> table =
			SelectAroundUnit(*firstContainer,
		                     missile_range,
		                     MakeAndPredicate(HasNotSamePlayerAs(Players[PlayerNumNeutral]), pred));

		if (table.empty() == false) {
			return BestRangeTargetFinder(unit, range).Find(table);
		}
		return nullptr;
	} else {
		// If unit is removed, use containers x and y
		const CUnit *firstContainer = unit.Container ? unit.Container : &unit;
		std::vector<CUnit *> table =
			SelectAroundUnit(*firstContainer,
		                     range,
		                     MakeAndPredicate(HasNotSamePlayerAs(Players[PlayerNumNeutral]), pred));

		if (range > 25 && table.size() > 9) {
			ranges::sort(table, CompareUnitDistance(unit));
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
