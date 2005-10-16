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
/**@name unit_cache.cpp - The unit cache. */
//
//      Cache to find units faster from position.
//      Sort of trivial implementation, since most queries are on a single tile.
//      Unit is just inserted in a double linked list for every tile it's on.
//
//      (c) Copyright 2004 by Crestez Leonard
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
//     along with this program; if not, write to the Free Software
//     Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
//     02111-1307, USA.
//
// $Id$

//@{

/*----------------------------------------------------------------------------
-- Includes
----------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "stratagus.h"
#include "unit.h"
#include "unittype.h"
#include "map.h"

/**
**  Insert new unit into cache.
**
**  @param unit  Unit pointer to place in cache.
*/
void UnitCacheInsert(CUnit *unit)
{
	int i;
	int j;
	MapField *mf;

	Assert(!unit->Removed);

	for (i = 0; i < unit->Type->TileHeight; ++i) {
		mf = Map.Fields + (i + unit->Y) * Map.Info.MapWidth + unit->X;
		for (j = 0; j < unit->Type->TileWidth; ++j) {
			mf[j].UnitCache.push_back(unit);
		}
	}
}

/**
**  Remove unit from cache.
**
**  @param unit  Unit pointer to remove from cache.
*/
void UnitCacheRemove(CUnit *unit)
{
	int i;
	int j;
	MapField *mf;

	Assert(!unit->Removed);
	for (i = 0; i < unit->Type->TileHeight; ++i) {
		mf = Map.Fields + (i + unit->Y) * Map.Info.MapWidth + unit->X;
		for (j = 0; j < unit->Type->TileWidth; ++j) {
			for (std::vector<CUnit *>::iterator k = mf[j].UnitCache.begin(); k != mf[j].UnitCache.end(); ++k) {
				if (*k == unit) {
					mf[j].UnitCache.erase(k);
					break;
				}
			}
		}
	}
}

/**
**  Select units in rectangle range.
**
**  @param x1     Left column of selection rectangle
**  @param y1     Top row of selection rectangle
**  @param x2     Right column of selection rectangle
**  @param y2     Bottom row of selection rectangle
**  @param table  All units in the selection rectangle
**
**  @return       Returns the number of units found
*/
int UnitCacheSelect(int x1, int y1, int x2, int y2, CUnit **table)
{
	int i;
	int j;
	int n;
	MapField *mf;

	// Optimize small searches.
	if (x1 >= x2 - 1 && y1 >= y2 - 1) {
		return UnitCacheOnTile(x1, y1, table);
	}

	//
	//  Reduce to map limits. FIXME: should the caller check?
	//
	if (x1 < 0) {
		x1 = 0;
	}
	if (y1 < 0) {
		y1 = 0;
	}
	if (x2 > Map.Info.MapWidth) {
		x2 = Map.Info.MapWidth;
	}
	if (y2 > Map.Info.MapHeight) {
		y2 = Map.Info.MapHeight;
	}

	n = 0;
	for (i = y1; i < y2; ++i) {
		for (j = x1; j < x2; ++j) {
			mf = &Map.Fields[i * Map.Info.MapWidth + j];
			for (std::vector<CUnit *>::iterator k = mf->UnitCache.begin();
				k != mf->UnitCache.end(); ++k) {
				//
				// To avoid getting a unit in multiple times we use a cache lock.
				// It should only be used in here, unless you somehow want the unit
				// to be out of cache.
				//
				if (!(*k)->CacheLock && !(*k)->Type->Revealer) {
					Assert(!(*k)->Removed);
					(*k)->CacheLock = 1;
					table[n++] = *k;
				}
			}
		}
	}

	//
	// Clean the cache locks, restore to original situation.
	//
	for (i = 0; i < n; ++i) {
		table[i]->CacheLock = 0;
	}

	return n;
}

/**
**  Select units on map tile.
**
**  @param x      Map X tile position
**  @param y      Map Y tile position
**  @param table  All units in the selection rectangle
**
**  @return       Returns the number of units found
*/
int UnitCacheOnTile(int x, int y, CUnit **table)
{
	int n;
	MapField *mf;

	mf = &Map.Fields[y * Map.Info.MapWidth + x];
	//
	// Unlike in UnitCacheSelect, there's no way an unit can show up twice,
	// so there is no need for Cache Locks.
	//
	n = 0;
	for (std::vector<CUnit *>::iterator i = mf->UnitCache.begin();
		i != mf->UnitCache.end(); ++i) {
			Assert(!(*i)->Removed);
			table[n++] = *i;
	}

	return n;
}

/**
**  Initialize unit-cache.
*/
void InitUnitCache(void)
{
}
