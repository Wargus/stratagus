//     ____                _       __               
//    / __ )____  _____   | |     / /___ ___________
//   / __  / __ \/ ___/   | | /| / / __ `/ ___/ ___/
//  / /_/ / /_/ (__  )    | |/ |/ / /_/ / /  (__  ) 
// /_____/\____/____/     |__/|__/\__,_/_/  /____/  
//                                              
//       A futuristic real-time strategy game.
//          This file is part of Bos Wars.
//
/**@name unit_find.cpp - The find/select for units. */
//
//      (c) Copyright 1998-2008 by Lutz Sammer and Jimmy Salmon
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
#include "video.h"
#include "sound.h"
#include "unitsound.h"
#include "unittype.h"
#include "actions.h"
#include "player.h"
#include "missile.h"
#include "unit.h"
#include "unit_cache.h"
#include "interface.h"
#include "map.h"
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
**  @param type       type of unit requested
**  @param table      table in which we have to store the units
**  @param tablesize  size of table array
**
**  @return       Returns the number of units found.
*/
int FindUnitsByType(const CUnitType *type, CUnit **table, int tablesize)
{
	int num = 0;

	for (int i = 0; i < NumUnits && num < tablesize; ++i) {
		CUnit *unit = Units[i];
		if (unit->Type == type && !unit->IsUnusable()) {
			table[num++] = unit;
		}
	}
	return num;
}

/**
**  Find all units of type.
**
**  @param player     we're looking for the units of this player
**  @param type       type of unit requested
**  @param table      table in which we have to store the units
**  @param tablesize  size of table array
**
**  @return        Returns the number of units found.
*/
int FindPlayerUnitsByType(const CPlayer *player, const CUnitType *type,
	CUnit **table, int tablesize)
{
	int nunits = player->TotalNumUnits;
	int typecount = player->UnitTypesCount[type->Slot];
	int num = 0;

	for (int i = 0; i < nunits && typecount && num < tablesize; ++i) {
		CUnit *unit = player->Units[i];
		if (unit->Type == type) {
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
**  @param tx    X position on map, tile-based.
**  @param ty    Y position on map, tile-based.
**  @param type  UnitTypeType, (unsigned)-1 for any type.
**
**  @return      Returns first found unit on tile.
*/
CUnit *UnitOnMapTile(int tx, int ty, unsigned type)
{
	CUnit *table[UnitMax];
	int n;

	n = UnitCache.Select(tx, ty, table, UnitMax);
	for (int i = 0; i < n; ++i) {
		// Note: this is less restrictive than UnitActionDie...
		// Is it normal?
		if (table[i]->Type->Vanishes) {
			continue;
		}
		if (type != (unsigned)-1 && (unsigned)table[i]->Type->UnitType != type) {
			continue;
		}
		return table[i];
	}

	return NoUnitP;
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
CUnit *TargetOnMap(const CUnit *source, int x1, int y1, int x2, int y2)
{
	CUnit *table[UnitMax];
	CUnit *best = NoUnitP;
	int n;

	n = UnitCache.Select(x1, y1, x2, y2, table, UnitMax);
	for (int i = 0; i < n; ++i) {
		CUnit *unit = table[i];
		if (!unit->IsVisibleAsGoal(source->Player)) {
			continue;
		}
		if (x2 < unit->X || x1 >= unit->X + unit->Type->TileWidth ||
				y2 < unit->Y || y1 >= unit->Y + unit->Type->TileHeight) {
			continue;
		}
		if (!CanTarget(source->Type, unit->Type)) {
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
**  @param tx        X position on map, tile-based.
**  @param ty        Y position on map, tile-based.
**  @param resource  Resource number or -1 for any.
**
**  @return    Returns the resource if found, or NoUnitP.
*/
CUnit *ResourceOnMap(int tx, int ty, int resource)
{
	CUnit *table[UnitMax];
	int n;

	n = UnitCache.Select(tx, ty, table, UnitMax);
	for (int i = 0; i < n; ++i) {
		if (table[i]->IsUnusable() || !table[i]->Type->CanHarvestFrom) {
			continue;
		}
		if (resource == -1) {
			if (!UnitHoldsResources(table[i])) {
				continue;
			}
		} else {
			if (table[i]->ResourcesHeld[resource] == 0) {
				continue;
			}
		}
		return table[i];
	}
	return NoUnitP;
}

/*----------------------------------------------------------------------------
--  Finding units for attack
----------------------------------------------------------------------------*/

/**
**  Attack units in distance, with large missile
**
**  Choose the best target, that can be attacked. It takes into
**  account allied unit which could be hit by the missile
**
**  @param u      Find in distance for this unit.
**  @param range  Distance range to look.
**
**  @return       Unit to be attacked.
**
**  @note This could be improved, for better performance / better trade.
**  @note   Limited to attack range smaller than 16.
**  @note Will be moved to unit_ai.c soon.
*/
static CUnit *FindRangeAttack(const CUnit *u, int range)
{
	int x;
	int y;
	int n;
	int cost;
	int d;
	int effective_hp;
	int enemy_count;
	int missile_range;
	int attackrange;
	int hp_damage_evaluate;
	int good[32][32];
	int bad[32][32];
	CUnit *table[UnitMax];
	CUnit *dest;
	const CUnitType *dtype;
	const CUnitType *type;
	const CPlayer *player;
	int xx;
	int yy;
	int best_cost;
	int i;
	int sbad;
	int sgood;
	CUnit *best;

	type = u->Type;
	player = u->Player;

	//  If catapult, count units near the target...
	//   FIXME : make it configurable
	//

	missile_range = type->Missile.Missile->Range + range - 1;
	attackrange = u->Stats->Variables[ATTACKRANGE_INDEX].Max;
	// Evaluation of possible damage...
	hp_damage_evaluate = u->Stats->Variables[BASICDAMAGE_INDEX].Value
						+ u->Stats->Variables[PIERCINGDAMAGE_INDEX].Value;

	Assert(2 * missile_range + 1 < 32);

	//
	// If unit is removed, use containers x and y
	if (u->Removed) {
		x = u->Container->X;
		y = u->Container->Y;
		n = UnitCache.Select(x - missile_range, y - missile_range,
			x + missile_range + u->Container->Type->TileWidth,
			y + missile_range + u->Container->Type->TileHeight, table, UnitMax);
	} else {
		x = u->X;
		y = u->Y;
		n = UnitCache.Select(x - missile_range, y - missile_range,
			x + missile_range + u->Type->TileWidth,
			y + missile_range + u->Type->TileHeight, table, UnitMax);
	}

	if (!n) {
		return NoUnitP;
	}

	for (y = 0; y < 2 * missile_range + 1; ++y) {
		for (x = 0; x < 2 * missile_range + 1; ++x) {
			good[y][x] = 0;
			bad[y][x] = 0;
		}
	}

	enemy_count = 0;
	// FILL good/bad...
	for (i = 0; i < n; ++i) {
		dest = table[i];
		dtype = dest->Type;
		if (!dest->IsVisibleAsGoal(u->Player)) {
			table[i] = 0;
			continue;
		}

		// won't be a target...
		if (!CanTarget(type, dtype)) { // can't be attacked.
			table[i] = 0;
			continue;
		}

		if (!player->IsEnemy(dest)) { // a friend or neutral
			table[i] = 0;

			// Calc a negative cost
			// The gost is more important when the unit would be killed
			// by our fire.

			// It costs (is positive) if hp_damage_evaluate>dest->HP ...)
			// FIXME : assume that PRIORITY_FACTOR>HEALTH_FACTOR
			cost = HEALTH_FACTOR * (2 * hp_damage_evaluate - dest->Variable[HP_INDEX].Value) /
				(dtype->TileWidth * dtype->TileWidth);
			if (cost < 1) {
				cost = 1;
			}
			cost = (-cost);
		} else {
			//
			//  Calculate the costs to attack the unit.
			//  Unit with the smallest attack costs will be taken.
			//
			cost = 0;
			//
			//  Priority 0-255
			//
			cost += dtype->Priority * PRIORITY_FACTOR;
			//
			//  Remaining HP (Health) 0-65535
			//
			// Give a boost to unit we can kill in one shoot only

			//
			// calculate HP which will remain in the enemy unit, after hit
			//
			effective_hp = (dest->Variable[HP_INDEX].Value - 2 * hp_damage_evaluate);

			//
			// Unit we won't kill are evaluated the same
			//
			if (effective_hp > 0) {
				effective_hp = 0;
			}

			//
			// Unit we are sure to kill are all evaluated the same (except PRIORITY)
			//
			if (effective_hp < -hp_damage_evaluate) {
				effective_hp = -hp_damage_evaluate;
			}

			//
			// Here, effective_hp vary from -hp_damage_evaluate (unit will be killed) to 0 (unit can't be killed)
			// => we prefer killing rather than only hitting...
			//
			cost += -effective_hp * HEALTH_FACTOR;

			//
			//  Unit can attack back.
			//
			if (CanTarget(dtype, type)) {
				cost += CANATTACK_BONUS;
			}

			//
			// the cost may be divided accros multiple cells
			//
			cost = cost / (dtype->TileWidth * dtype->TileWidth);
			if (cost < 1) {
				cost = 1;
			}

			//
			// Removed Unit's are in bunkers
			//
			if (u->Removed) {
				d = MapDistanceBetweenUnits(u->Container, dest);
			} else {
				d = MapDistanceBetweenUnits(u, dest);
			}

			if (d <= attackrange || (d <= range && UnitReachable(u, dest, attackrange))) {
				++enemy_count;
			} else {
					table[i] = 0;
			}
		}

		x = dest->X - u->X + missile_range + 1;
		y = dest->Y - u->Y + missile_range + 1;

		// Mark the good/bad array...
		for (xx = 0; xx < dtype->TileWidth; ++xx) {
			for (yy = 0; yy < dtype->TileWidth; ++yy) {
				if ((x + xx < 0) || (y + yy < 0) ||
						(x + xx >= 2 * missile_range + 1) ||
						(y + yy >= 2 * missile_range + 1)) {
					continue;
				}
				if (cost < 0) {
					good[y + yy][x + xx] -= cost;
				} else {
					bad[y + yy][x + xx] += cost;
				}
			}
		}
	}

	if (!enemy_count) {
		return NoUnitP;
	}

	// Find the best area...
	// The target which provide the best bad/good ratio is choosen...
	best_cost = -1;
	best = NoUnitP;
	for (i = 0; i < n; ++i) {
		if (!table[i]) {
			continue;
		}
		dest = table[i];
		dtype = dest->Type;

		// put in x-y the real point which will be hit...
		// (only meaningful when dtype->TileWidth > 1)
		if (u->X < dest->X) {
			x = dest->X;
		} else if (u->X > dest->X + dtype->TileWidth - 1) {
			x = dest->X + dtype->TileWidth - 1;
		} else {
			x = u->X;
		}

		if (u->Y < dest->Y) {
			y = dest->Y;
		} else if (u->Y > dest->Y + dtype->TileHeight - 1) {
			y = dest->Y + dtype->TileHeight - 1;
		} else {
			y = u->Y;
		}

		// Make x,y relative to u->x...
		x = x - u->X + missile_range + 1;
		y = y - u->Y + missile_range + 1;

		sbad = 0;
		sgood = 0;
		for (yy = -(type->Missile.Missile->Range - 1);
			yy <= type->Missile.Missile->Range - 1; ++yy) {
			for (xx = -(type->Missile.Missile->Range - 1);
				xx <= type->Missile.Missile->Range - 1; ++xx) {
				if ((x + xx < 0) || (y + yy < 0) ||
						((x + xx) >= 2 * missile_range + 1) ||
						((y + yy) >= 2 * missile_range + 1)) {
					continue;
				}

				sbad += bad[y + yy][x + xx];
				sgood += good[y + yy][x + xx];
				if (!yy && !xx) {
					sbad += bad[y + yy][x + xx];
					sgood += good[y + yy][x + xx];
				}
			}
		}

		// don't consider small damages...
		if (sgood < 20) {
			sgood = 20;
		}

		cost = sbad / sgood;
		if (cost > best_cost) {
			best_cost = cost;
			best = dest;
		}
	}
	return best;
}

/**
**  Reference unit used by CompareUnitDistance.
*/
static const CUnit *referenceunit;

/**
**  Returns a value less then 0, 0 or bigger then 0,
**  when the first unit is repectively nearer, at the same distance
**  or further away then the 2nd from the referenceunit.
*/
static int CompareUnitDistance(const void *v1, const void *v2)
{
	CUnit *c1 = *(CUnit **)v1;
	CUnit *c2  = *(CUnit **)v2;

	int d1 = MapDistanceBetweenUnits(referenceunit, c1);
	int d2 = MapDistanceBetweenUnits(referenceunit, c2);

	if (d1 - d2 != 0) {
		return d1 - d2;
	} else {
		return c1->Slot - c2->Slot;
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
CUnit *AttackUnitsInDistance(const CUnit *unit, int range)
{
	CUnit *dest;
	const CUnitType *type;
	const CUnitType *dtype;
	CUnit *table[UnitMax];
	int x;
	int y;
	int n;
	int i;
	int d;
	int attackrange;
	int cost;
	int best_cost;
	const CPlayer *player;
	CUnit *best_unit;

	// if necessary, take possible damage on allied units into account...
	if (unit->Type->Missile.Missile->Range > 1 &&
			(range + unit->Type->Missile.Missile->Range < 15)) {
		return FindRangeAttack(unit, range);
	}

	//
	// Select all units in range.
	//
	x = unit->X;
	y = unit->Y;
	type = unit->Type;
	n = UnitCache.Select(x - range, y - range, x + range + type->TileWidth,
		y + range + type->TileHeight, table, UnitMax);
	
	if (range > 25 && n > 9) {
		referenceunit = unit;
		qsort((void*)table, n, sizeof(CUnit*), &CompareUnitDistance);
	}

	best_unit = NoUnitP;
	best_cost = INT_MAX;

	player = unit->Player;
	attackrange = unit->Stats->Variables[ATTACKRANGE_INDEX].Max;

	//
	// Find the best unit to attack
	//

	for (i = 0; i < n; ++i) {
		dest = table[i];

		if (!dest->IsVisibleAsGoal(unit->Player)) {
			continue;
		}

		if (!player->IsEnemy(dest)) { // a friend or neutral
			continue;
		}

		dtype = dest->Type;
		if (!CanTarget(type, dtype)) { // can't be attacked.
			continue;
		}

		//
		// Calculate the costs to attack the unit.
		// Unit with the smallest attack costs will be taken.
		//
		cost = 0;
		//
		// Priority 0-255
		//
		cost -= dtype->Priority * PRIORITY_FACTOR;
		//
		// Remaining HP (Health) 0-65535
		//
		cost += dest->Variable[HP_INDEX].Value * HEALTH_FACTOR;
		//
		// Unit in attack range?
		//
		d = MapDistanceBetweenUnits(unit, dest);

		// Use Circle, not square :)
		if (d > range) {
			continue;
		}
		if (d <= attackrange && d >= type->MinAttackRange) {
			cost += d * INRANGE_FACTOR;
			cost -= INRANGE_BONUS;
		} else {
			cost += d * DISTANCE_FACTOR;
		}
		//
		// Unit can attack back.
		//
		if (CanTarget(dtype, type)) {
			cost -= CANATTACK_BONUS;
		}

		//
		// Take this target?
		//
		if (cost < best_cost && (d <= attackrange ||
				UnitReachable(unit, dest, attackrange))) {
			best_unit = dest;
			best_cost = cost;
		}
	}

	return best_unit;
}

/**
**  Attack units in attack range.
**
**  @param unit  Find unit in attack range for this unit.
**
**  @return      Pointer to unit which should be attacked.
*/
CUnit *AttackUnitsInRange(const CUnit *unit)
{
	Assert(unit->Type->CanAttack);
	return AttackUnitsInDistance(unit, unit->Stats->Variables[ATTACKRANGE_INDEX].Max);
}

/**
**  Attack units in reaction range.
**
**  @param unit  Find unit in reaction range for this unit.
**
**  @return      Pointer to unit which should be attacked.
*/
CUnit *AttackUnitsInReactRange(const CUnit *unit)
{
	int range;

	Assert(unit->Type->CanAttack);

	if (unit->Player->Type == PlayerPerson) {
		range = unit->Type->ReactRangePerson;
	} else {
		range = unit->Type->ReactRangeComputer;
	}

	return AttackUnitsInDistance(unit, range);
}

//@}
