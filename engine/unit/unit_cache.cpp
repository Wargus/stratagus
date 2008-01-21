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

//@{

/*----------------------------------------------------------------------------
-- Includes
----------------------------------------------------------------------------*/

#include <string.h>

#include "stratagus.h"
#include "unit_cache.h"
#include "unit.h"
#include "unittype.h"


CUnitCache UnitCache;


/**
**  Initialize unit-cache.
**
**  @param mapWidth   Map width
**  @param mapHeight  Map height
*/
void CUnitCache::Init(int mapWidth, int mapHeight)
{
	width = mapWidth;
	height = mapHeight;

	cache.clear();
	cache.resize(width);
	for (int i = 0; i < width; ++i) {
		cache[i].resize(height);
	}
}

/**
**  Insert new unit into cache.
**
**  @param unit  Unit pointer to place in cache.
*/
void CUnitCache::Insert(CUnit *unit)
{
	Assert(!unit->Removed);

	for (int i = 0; i < unit->Type->TileWidth; ++i) {
		for (int j = 0; j < unit->Type->TileHeight; ++j) {
			cache[unit->X + i][unit->Y + j].push_back(unit);
		}
	}
}

/**
**  Remove unit from cache.
**
**  @param unit  Unit pointer to remove from cache.
*/
void CUnitCache::Remove(CUnit *unit)
{
	Assert(!unit->Removed);

	for (int i = 0; i < unit->Type->TileWidth; ++i) {
		for (int j = 0; j < unit->Type->TileHeight; ++j) {
			for (std::vector<CUnit *>::iterator k = cache[unit->X + i][unit->Y + j].begin();
					k != cache[unit->X + i][unit->Y + j].end(); ++k) {
				if (*k == unit) {
					cache[unit->X + i][unit->Y + j].erase(k);
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
int CUnitCache::Select(int x1, int y1, int x2, int y2, CUnit **table, int tablesize)
{
	int i;
	int j;
	int n;
	CUnit *unit;
	bool unitAdded[UnitMax];

	// Optimize small searches.
	if (x1 >= x2 - 1 && y1 >= y2 - 1) {
		return UnitCache.Select(x1, y1, table, tablesize);
	}

	//
	//  Reduce to map limits.
	//
	x1 = std::max(x1, 0);
	y1 = std::max(y1, 0);
	x2 = std::min(x2, width - 1);
	y2 = std::min(y2, height - 1);

	memset(unitAdded, 0, sizeof(unitAdded));
	n = 0;

	for (i = x1; i <= x2 && n < tablesize; ++i) {
		for (j = y1; j <= y2 && n < tablesize; ++j) {
			for (size_t k = 0, end = cache[i][j].size(); k < end && n < tablesize; ++k) {
				//
				// To avoid getting a unit in multiple times we use a cache lock.
				// It should only be used in here, unless you somehow want the unit
				// to be out of cache.
				//
				unit = cache[i][j][k];
				if (!unitAdded[unit->Slot] && !unit->Type->Revealer) {
					Assert(!unit->Removed);
					unitAdded[unit->Slot] = true;
					table[n++] = unit;
				}
			}
		}
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
**  @return           Returns the number of units found
*/
int CUnitCache::Select(int x, int y, CUnit **table, int tablesize)
{
	//
	// Unlike in UnitCacheSelect, there's no way a unit can show up twice,
	// so there is no need for Cache Locks.
	//
	int n = 0;
	std::vector<CUnit *>::iterator i, end;

	for (i = cache[x][y].begin(), end = cache[x][y].end(); i != end && n < tablesize; ++i) {
		Assert(!(*i)->Removed);
		table[n++] = *i;
	}

	return n;
}

