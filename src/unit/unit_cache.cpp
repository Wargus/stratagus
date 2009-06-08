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
#include "unit.h"
#include "unittype.h"
#include "map.h"

/**
**  Insert new unit into cache.
**
**  @param unit  Unit pointer to place in cache.
*/
void CMap::Insert(CUnit *unit)
{
	Assert(!unit->Removed);
	unsigned int index = unit->Offset;
	const int w = unit->Type->TileWidth;
	const int h = unit->Type->TileHeight;
	int j,i = h;
	CMapField *mf;
	do {	
		mf = Field(index);
		j = w;
		do {
			mf->UnitCache.Insert(unit);
			++mf;
		} while( --j && unit->X + (j - w) < Info.MapWidth);
		index += Info.MapWidth;
	} while( --i && unit->Y + (i - h) < Info.MapHeight);
}

/**
**  Remove unit from cache.
**
**  @param unit  Unit pointer to remove from cache.
*/
void CMap::Remove(CUnit *unit)
{
	Assert(!unit->Removed);
	unsigned int index = unit->Offset;
	const int w = unit->Type->TileWidth;
	const int h = unit->Type->TileHeight;
	int j,i = h;
	CMapField *mf;
	do {	
		mf = Field(index);
		j = w;
		do {
			mf->UnitCache.Remove(unit);
			++mf;
		} while( --j && unit->X + (j - w) < Info.MapWidth);
		index += Info.MapWidth;
	} while( --i && unit->Y + (i - h) < Info.MapHeight);
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
int CMap::Select(int x, int y, CUnit *table[],  
							const int tablesize)
{
	int n = 0;
	CUnitCache &cache = Field(x,y)->UnitCache;
	const size_t size = cache.size();
	for(unsigned int i = 0; n < tablesize && i < size; ++i) {
		CUnit *unit = cache.Units[i];
		Assert(!unit->Removed);
		table[n++] = unit;
	}
	return n;
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
int CMap::SelectFixed(int x1, int y1,  
		int x2, int y2, CUnit *table[], const int tablesize)
{

	// Optimize small searches.
	if (x1 >= x2 - 1 && y1 >= y2 - 1) {
		return Select(x1, y1, table, tablesize);		
	}

	int i;
	int n = 0;
	CUnit *unit;
	const CMapField *mf;
	unsigned int index = getIndex(x1, y1);
	int j = y2 - y1 + 1;
	do {
		mf = Field(index);
		i = x2 - x1 + 1;
		do {
#if __GNUC__ >  3
			//GCC version only, since std::vector::data() is not in STL		
			size_t count = mf->UnitCache.size();
			if(count) {
				CUnit **cache = (CUnit **)mf->UnitCache.Units.data();
				do {
					unit = *cache;
					//
					// To avoid getting a unit in multiple times we use a cache lock.
					// It should only be used in here, unless you somehow want the unit
					// to be out of cache.
					//
					if (!unit->CacheLock && !unit->Type->Revealer) {
						Assert(!unit->Removed);
						unit->CacheLock = 1;
						table[n++] = unit;
					}				
					++cache;
				} while(--count && n < tablesize);
			}
#else
			const size_t count = mf->UnitCache.size();
			if(count) {	
				unsigned int k = 0;
				const CUnitCache &cache = mf->UnitCache;
				do {
					unit = cache[k];
					//
					// To avoid getting a unit in multiple times we use a cache lock.
					// It should only be used in here, unless you somehow want the unit
					// to be out of cache.
					//
					if (!unit->CacheLock && !unit->Type->Revealer) {
						Assert(!unit->Removed);
						unit->CacheLock = 1;
						table[n++] = unit;
					}				
				} while(++k < count && n < tablesize);
			}
#endif			
			++mf;
		} while(--i && n < tablesize);
		index += Info.MapWidth;
	} while(--j && n < tablesize);

	if(!n) return 0;

	//
	// Clean the cache locks, restore to original situation.
	//
#ifndef __GNUG__
	for (i = 0; i < n; ++i) {
		table[i]->CacheLock = 0;
	}
#else
	i = 0;
	j = (n+3)/4;
	switch (n & 3) {
		case 0: do { 
						table[i++]->CacheLock = 0;
		case 3:			table[i++]->CacheLock = 0;
		case 2:			table[i++]->CacheLock = 0;
		case 1:			table[i++]->CacheLock = 0;
			} while ( --j > 0 );
	}

#endif


	return n;
}

int CMap::Select(int x1, int y1,  
		int x2, int y2, CUnit *table[], const int tablesize)
{

	//
	//  Reduce to map limits.
	//
	x1 = std::max<int>(x1, 0);
	y1 = std::max<int>(y1, 0);
	x2 = std::min<int>(x2, Info.MapWidth - 1);
	y2 = std::min<int>(y2, Info.MapHeight - 1);

	return SelectFixed(x1,y1,x2,y2,table, tablesize);
}

