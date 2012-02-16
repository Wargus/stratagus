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
//      (c) Copyright 1998-2005 by Lutz Sammer and Jimmy Salmon
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

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <limits.h>

#include "stratagus.h"
#include "map.h"
#include "video.h"
#include "sound.h"
#include "unitsound.h"
#include "unittype.h"
#include "actions.h"
#include "player.h"
#include "missile.h"
#include "unit.h"
#include "interface.h"
#include "tileset.h"
#include "pathfinder.h"

/*----------------------------------------------------------------------------
--  Defines
----------------------------------------------------------------------------*/

/*
** Configuration of the small (unit) AI.
*/
#define PRIORITY_FACTOR   0x00010000
#define HEALTH_FACTOR     0x00000001
#define DISTANCE_FACTOR   0x00100000
#define INRANGE_FACTOR    0x00010000
#define INRANGE_BONUS     0x01000000
#define CANATTACK_BONUS   0x00100000

/*----------------------------------------------------------------------------
--  Local Data
----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
--  Functions
----------------------------------------------------------------------------*/

/**
**  Find all units of type.
**
**  @param type   type of unit requested
**  @param table  table in which we have to store the units
**
**  @return       Returns the number of units found.
*/
int FindUnitsByType(const CUnitType &type, CUnit **table)
{
	int num = 0;

	for (int i = 0; i < NumUnits; ++i) {
		CUnit *unit = Units[i];

		if (unit->Type == &type && !unit->IsUnusable()) {
			table[num++] = unit;
		}
	}
	return num;
}

/**
**  Find all units of type.
**
**  @param player  we're looking for the units of this player
**  @param type    type of unit requested
**  @param table   table in which we have to store the units
**
**  @return        Returns the number of units found.
*/
int FindPlayerUnitsByType(const CPlayer &player, const CUnitType &type, CUnit **table)
{
	const int nunits = player.TotalNumUnits;
	int typecount = player.UnitTypesCount[type.Slot];
	int num = 0;

	for (int i = 0; i < nunits && typecount; ++i) {
		CUnit *unit = player.Units[i];

		if (unit->Type == &type) {
			if (!unit->IsUnusable()) {
				table[num++] = unit;
			}
			--typecount;
		}
	}
	return num;
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
	return CUnitTypeFinder((UnitTypeType)type).Find(Map.Field(index));
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
**  @param x1      X position on map, tile-based.
**  @param y1      Y position on map, tile-based.
**  @param x2      X position on map, tile-based.
**  @param y2      Y position on map, tile-based.
**
**  @return        Returns ideal target on map tile.
*/
CUnit *TargetOnMap(const CUnit &source, int x1, int y1, int x2, int y2)
{
	CUnit *table[UnitMax];
	CUnit *unit;
	CUnit *best = NoUnitP;
	const CUnitType *type;

	int n = Map.Select(x1, y1, x2, y2, table);
	for (int i = 0; i < n; ++i) {
		unit = table[i];
		if (!unit->IsVisibleAsGoal(*source.Player)) {
			continue;
		}
		type = unit->Type;
		if (x2 < unit->tilePos.x || x1 >= unit->tilePos.x + type->TileWidth ||
				y2 < unit->tilePos.y || y1 >= unit->tilePos.y + type->TileHeight) {
			continue;
		}
		if (!CanTarget(source.Type, unit->Type)) {
			continue;
		}

		//
		// Choose the best target.
		//
		if (!best || best->Type->Priority < unit->Type->Priority) {
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
**  @return          Returns the deposit if found, or NoUnitP.
*/
CUnit *ResourceOnMap(const Vec2i &pos, int resource, bool mine_on_top)
{
	return CResourceFinder(resource, mine_on_top).Find(Map.Field(pos));
}

/**
**  Resource deposit on map tile
**
**  @param pos       position on map, tile-based.
**  @param resource  resource type.
**
**  @return          Returns the deposit if found, or NoUnitP.
*/
CUnit *ResourceDepositOnMap(const Vec2i &pos, int resource)
{
	return CResourceDepositFinder(resource).Find(Map.Field(pos));
}

/*----------------------------------------------------------------------------
--  Finding units for attack
----------------------------------------------------------------------------*/

class BestTargetFinder
{
public:
	BestTargetFinder(const CUnit &a, int r) :
		attacker(&a), range(r)
	{}
	
	CUnit *Find(CUnit* table[], const int table_size) const
	{
		return Find(table, table + table_size);
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
		int best_cost = INT_MAX;

		for (Iterator it = begin; it != end; ++it) {
			const int cost = ComputeCost(*it);

			if (cost < best_cost) {
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
			|| !CanTarget(&type, &dtype)) {
			return INT_MAX;
		}
		// Unit in range ?
		const int d = attacker->MapDistanceTo(*dest);

		if (d > range || !UnitReachable(*attacker, *dest, attackrange)) {
			return INT_MAX;
		}

		// Calculate the costs to attack the unit.
		// Unit with the smallest attack costs will be taken.
		int cost = 0;

		// Priority 0-255
		cost -= dtype.Priority * PRIORITY_FACTOR;
		// Remaining HP (Health) 0-65535
		cost += dest->Variable[HP_INDEX].Value * HEALTH_FACTOR;

		if (d <= attackrange && d >= type.MinAttackRange) {
			cost += d * INRANGE_FACTOR;
			cost -= INRANGE_BONUS;
		} else {
			cost += d * DISTANCE_FACTOR;
		}

		// Unit can attack back.
		if (CanTarget(&dtype, &type)) {
			cost -= CANATTACK_BONUS;
		}
		return cost;
	}

private:
	const CUnit *attacker;
	const int range;
};

/**
**  Attack units in distance, with large missile
**
**  Choose the best target, that can be attacked. It takes into
**  account allied unit which could be hit by the missile
**
**
**  @note This could be improved, for better performance / better trade.
**  @note   Limited to attack range smaller than 16.
**  @note Will be moved to unit_ai.c soon.
*/
class BestRangeTargetFinder {
public:
	/**
	**  @param a      Find in distance for this unit.
	**  @param range  Distance range to look.
	**
	*/
	BestRangeTargetFinder(const CUnit &a, const int r) : attacker(&a), range(r),
		 best_unit(0), best_cost(INT_MIN) {
		memset(good, 0 , sizeof(int) * 32 * 32);
		memset(bad, 0 , sizeof(int) * 32 * 32);
	};

	class FillBadGood
	{
	public:
		FillBadGood(const CUnit &a, int r, int *g, int *b):
			attacker(&a), range(r),
			enemy_count(0), good(g), bad(b)
		{
		}

		int Fill(CUnit *table[], const int table_size)
		{
			return Fill(table, table + table_size);
		}

		int Fill(CUnitCache &cache)
		{
			return Fill(cache.begin(), cache.end());
		}
	private:
		template <typename Iterator>
		int Fill(Iterator begin, Iterator end)
		{
			for (Iterator it = begin; it != end; ++it) {
				Compute(*it);
			}
			return enemy_count;
		}

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
			if (!CanTarget(&type, &dtype)) { // can't be attacked.
				dest->CacheLock = 1;
				return;
			}

			//  Calculate the costs to attack the unit.
			//  Unit with the smallest attack costs will be taken.

			int cost = 0;
			const int hp_damage_evaluate =
				attacker->Stats->Variables[BASICDAMAGE_INDEX].Value
						+ attacker->Stats->Variables[PIERCINGDAMAGE_INDEX].Value;

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
				if (cost < 1) {
					cost = 1;
				}
				cost = (-cost);
			} else {
				//  Priority 0-255
				cost += dtype.Priority * PRIORITY_FACTOR;
				//  Remaining HP (Health) 0-65535
				// Give a boost to unit we can kill in one shoot only

				// calculate HP which will remain in the enemy unit, after hit
				int effective_hp = (dest->Variable[HP_INDEX].Value - 2 * hp_damage_evaluate);

				// Unit we won't kill are evaluated the same
				if (effective_hp > 0) {
					effective_hp = 0;
				}

				// Unit we are sure to kill are all evaluated the same (except PRIORITY)
				if (effective_hp < -hp_damage_evaluate) {
					effective_hp = -hp_damage_evaluate;
				}

				// Here, effective_hp vary from -hp_damage_evaluate (unit will be killed) to 0 (unit can't be killed)
				// => we prefer killing rather than only hitting...
				cost += -effective_hp * HEALTH_FACTOR;

				//  Unit can attack back.
				if (CanTarget(&dtype, &type)) {
					cost += CANATTACK_BONUS;
				}

				// the cost may be divided accros multiple cells
				cost = cost / (dtype.TileWidth * dtype.TileWidth);
				if (cost < 1) {
					cost = 1;
				}

				// Removed Unit's are in bunkers
				int d;
				if (attacker->Removed) {
					d = attacker->Container->MapDistanceTo(*dest);
				} else {
					d = attacker->MapDistanceTo(*dest);
				}

				int attackrange = attacker->Stats->Variables[ATTACKRANGE_INDEX].Max;
				if (d <= attackrange ||
					(d <= range && UnitReachable(*attacker, *dest, attackrange))) {
					++enemy_count;
				} else {
					dest->CacheLock = 1;
				}
			}

			const int missile_range = type.Missile.Missile->Range + range - 1;
			const int x = dest->tilePos.x - attacker->tilePos.x + missile_range + 1;
			const int y = dest->tilePos.y - attacker->tilePos.y + missile_range + 1;

			// Mark the good/bad array...
			int yy_offset = x + y * 32;
			for (int yy = 0; yy < dtype.TileHeight; ++yy) {
				if ((y + yy >= 0) && (y + yy < 2 * missile_range + 1)) {
					for (int xx = 0; xx < dtype.TileWidth; ++xx) {
						if ((x + xx >= 0) && (x + xx < 2 * missile_range + 1)) {
							if (cost < 0) {
								good[yy_offset + xx] -= cost;
							} else {
								bad[yy_offset + xx] += cost;
							}
						}
					}
				}
				yy_offset += 32;
			}
		}


	private:
		const CUnit *attacker;
		const int range;
		int enemy_count;
		int *good;
		int *bad;
	};

	CUnit *Find(CUnit* table[], const int table_size) {
		FillBadGood(*attacker, range, good, bad).Fill(table, table_size);
		return Find(table, table + table_size);

	}

	CUnit *Find(CUnitCache &cache) {
		FillBadGood(*attacker, range, good, bad).Fill(cache);
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
		const CUnitType &type =  *attacker->Type;
		const CUnitType &dtype = *dest->Type;
		const int missile_range = type.Missile.Missile->Range + range - 1;
		int x,y;

		// put in x-y the real point which will be hit...
		// (only meaningful when dtype->TileWidth > 1)
		if (attacker->tilePos.x < dest->tilePos.x) {
			x = dest->tilePos.x;
		} else if (attacker->tilePos.x > dest->tilePos.x + dtype.TileWidth - 1) {
			x = dest->tilePos.x + dtype.TileWidth - 1;
		} else {
			x = attacker->tilePos.x;
		}

		if (attacker->tilePos.y < dest->tilePos.y) {
			y = dest->tilePos.y;
		} else if (attacker->tilePos.y > dest->tilePos.y + dtype.TileHeight - 1) {
			y = dest->tilePos.y + dtype.TileHeight - 1;
		} else {
			y = attacker->tilePos.y;
		}

		// Make x,y relative to u->x...

		x = x - attacker->tilePos.x + missile_range + 1;
		y = y - attacker->tilePos.x + missile_range + 1;

		int sbad = 0;
		int sgood = 0;
		int yy = -(type.Missile.Missile->Range - 1);
		int yy_offset = x + yy * 32;
		for (;yy <= type.Missile.Missile->Range - 1; ++yy) {
			if ((y + yy >= 0) && ((y + yy) < 2 * missile_range + 1)) {
				for (int xx = -(type.Missile.Missile->Range - 1);
					xx <= type.Missile.Missile->Range - 1; ++xx) {
					if ((x + xx >= 0) && ((x + xx) < 2 * missile_range + 1)) {
						sbad += bad[yy_offset + xx];
						sgood += good[yy_offset + xx];
						if (!yy && !xx) {
							sbad += bad[yy_offset + xx];
							sgood += good[yy_offset + xx];
						}
					}
				}
			}
			yy_offset += 32;
		}

		// don't consider small damages...
		if (sgood < 20) {
			sgood = 20;
		}

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
	int good[32*32];
	int bad[32*32];
};

struct CompareUnitDistance {
	const CUnit *referenceunit;
	CompareUnitDistance(const CUnit &unit): referenceunit(&unit) {}
	bool operator() (const CUnit*c1, const CUnit*c2)
	{
		int d1 = c1->MapDistanceTo(*referenceunit);
		int d2 = c2->MapDistanceTo(*referenceunit);
		if (d1 == d2) {
			return c1->Slot < c2->Slot;
		} else {
			return d1 < d2;
		}
	}
};

/**
**  AutoAttack units in distance.
**
**  If the unit can attack must be handled by caller.
**  Choose the best target, that can be attacked.
**
**  @param unit   Find in distance for this unit.
**  @param range  Distance range to look.
**  @param autotargets  Know enemy targets to chose in range.
**
**  @return       Unit to be attacked.
**
*/
CUnit *AutoAttackUnitsInDistance(const CUnit &unit, int range,
		CUnitCache &autotargets)
{
	// if necessary, take possible damage on allied units into account...
	if (unit.Type->Missile.Missile->Range > 1
		&& (range + unit.Type->Missile.Missile->Range < 15)) {
		return BestRangeTargetFinder(unit, range).Find(autotargets);
	} else {
		// Find the best unit to auto attack
		return BestTargetFinder(unit, range).Find(autotargets);
	}
}

/**
**  Attack units in distance.
**
**  If the unit can attack must be handled by caller.
**  Choose the best target, that can be attacked.
**
**  @param unit   Find in distance for this unit.
**  @param range  Distance range to look.
**
**  @return       Unit to be attacked.
**
*/
CUnit *AttackUnitsInDistance(const CUnit &unit, int range)
{
	CUnit *table[UnitMax];
	int n;
	// if necessary, take possible damage on allied units into account...
	if (unit.Type->Missile.Missile->Range > 1 &&
			(range + unit.Type->Missile.Missile->Range < 15)) {

		//  If catapult, count units near the target...
		//   FIXME : make it configurable
		//

		int missile_range = unit.Type->Missile.Missile->Range + range - 1;

		Assert(2 * missile_range + 1 < 32);
		//
		// If unit is removed, use containers x and y
		if (unit.Removed) {
			int x = unit.Container->tilePos.x;
			int y = unit.Container->tilePos.y;
			n = Map.Select(x - missile_range, y - missile_range,
				x + missile_range + unit.Container->Type->TileWidth,
				y + missile_range + unit.Container->Type->TileHeight, table);
		} else {
			int x = unit.tilePos.x;
			int y = unit.tilePos.y;
			n = Map.Select(x - missile_range, y - missile_range,
				x + missile_range + unit.Type->TileWidth,
				y + missile_range + unit.Type->TileHeight, table);
		}

		if (n) {
			return BestRangeTargetFinder(unit, range).Find(table, n);
		}
		return NoUnitP;

	} else {
		n = Map.Select(unit.tilePos.x - range, unit.tilePos.y - range,
			unit.tilePos.x + range + unit.Type->TileWidth,
			unit.tilePos.y + range + unit.Type->TileHeight, table);

		if (range > 25 && n > 9) {
			std::sort(table, table + n, CompareUnitDistance(unit));
		}

		//
		// Find the best unit to attack
		//
		return BestTargetFinder	(unit, range).Find(table, n);
	}
}

/**
**  Attack units in attack range.
**
**  @param unit  Find unit in attack range for this unit.
**
**  @return      Pointer to unit which should be attacked.
*/
CUnit *AttackUnitsInRange(const CUnit &unit)
{
	Assert(unit.Type->CanAttack);
	return AttackUnitsInDistance(unit, unit.Stats->Variables[ATTACKRANGE_INDEX].Max);
}

/**
**  Attack units in reaction range.
**
**  @param unit  Find unit in reaction range for this unit.
**
**  @return      Pointer to unit which should be attacked.
*/
CUnit *AttackUnitsInReactRange(const CUnit &unit)
{
	Assert(unit.Type->CanAttack);
	return AttackUnitsInDistance(unit, unit.GetReactRange());
}

//@}
