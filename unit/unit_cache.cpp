//     ____                _       __               
//    / __ )____  _____   | |     / /___ ___________
//   / __  / __ \/ ___/   | | /| / / __ `/ ___/ ___/
//  / /_/ / /_/ (__  )    | |/ |/ / /_/ / /  (__  ) 
// /_____/\____/____/     |__/|__/\__,_/_/  /____/  
//                                              
//       A futuristic real-time strategy game.
//          This file is part of Bos Wars.
//
/**@name unit_cache.cpp - The unit cache. */
//
//      Cache to find units faster from position.
//      Sort of trivial implementation, since most queries are on a single tile.
//      Unit is just inserted in a double linked list for every tile it's on.
//
//      (c) Copyright 2004-2007 by Crestez Leonard and Jimmy Salmon
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
	Assert(!unit->Removed);

	for (int i = 0; i < unit->Type->TileHeight; ++i) {
		CMapField *mf = Map.Field(unit->X, unit->Y + i);
		for (int j = 0; j < unit->Type->TileWidth; ++j) {
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
	Assert(!unit->Removed);

	for (int i = 0; i < unit->Type->TileHeight; ++i) {
		CMapField *mf = Map.Field(unit->X, unit->Y + i);
		for (int j = 0; j < unit->Type->TileWidth; ++j) {
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
**  @param x1         Left column of selection rectangle
**  @param y1         Top row of selection rectangle
**  @param x2         Right column of selection rectangle
**  @param y2         Bottom row of selection rectangle
**  @param table      All units in the selection rectangle
**  @param tablesize  Size of table array
**
**  @return           Returns the number of units found
*/
int UnitCacheSelect(int x1, int y1, int x2, int y2, CUnit **table, int tablesize)
{
	int i;
	int j;
	int n;
	CMapField *mf;
	CUnit *unit;

	// Optimize small searches.
	if (x1 >= x2 - 1 && y1 >= y2 - 1) {
		return UnitCacheOnTile(x1, y1, table, tablesize);
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
	for (i = y1; i < y2 && n < tablesize; ++i) {
		mf = Map.Field(x1, i);
		for (j = x1; j < x2 && n < tablesize; ++j) {
			for (size_t k = 0, end = mf->UnitCache.size(); k < end && n < tablesize; ++k) {
				//
				// To avoid getting a unit in multiple times we use a cache lock.
				// It should only be used in here, unless you somehow want the unit
				// to be out of cache.
				//
				unit = mf->UnitCache[k];
				if (!unit->CacheLock && !unit->Type->Revealer) {
					Assert(!unit->Removed);
					unit->CacheLock = 1;
					table[n++] = unit;
				}
			}
			++mf;
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
**  @param x          Map X tile position
**  @param y          Map Y tile position
**  @param table      All units in the selection rectangle
**  @param tablesize  Size of table array
**
**  @return       Returns the number of units found
*/
int UnitCacheOnTile(int x, int y, CUnit **table, int tablesize)
{
	//
	// Unlike in UnitCacheSelect, there's no way a unit can show up twice,
	// so there is no need for Cache Locks.
	//
	int n = 0;
	CMapField *mf = Map.Field(x, y);
	std::vector<CUnit *>::iterator i, end;

	for (i = mf->UnitCache.begin(), end = mf->UnitCache.end(); i != end && n < tablesize; ++i) {
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
