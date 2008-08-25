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
	unsigned int index = unit->Y * Info.MapWidth;
	const int w = unit->Type->TileWidth;
	const int h = unit->Type->TileHeight;
	int j,i = h;
	CMapField *mf;
	do {	
		mf = Field(index + unit->X);
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
	unsigned int index = unit->Y * Info.MapWidth;
	const int w = unit->Type->TileWidth;
	const int h = unit->Type->TileHeight;
	int j,i = h;
	CMapField *mf;
	do {	
		mf = Field(index + unit->X);
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
	CMapField *mf = Field(x, y);
	const size_t size = mf->UnitCache.size();
	for(unsigned int i = 0; n < tablesize && i < size; ++i) {
		CUnit *unit = mf->UnitCache.Units[i];
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
int CMap::Select(int x1, int y1,  
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
	bool unitAdded[UnitMax];
	bool *m;
	//
	//  Reduce to map limits.
	//
	x1 = std::max(x1, 0);
	y1 = std::max(y1, 0);
	x2 = std::min(x2, Info.MapWidth - 1);
	y2 = std::min(y2, Info.MapHeight - 1);

	memset(unitAdded, 0, sizeof(unitAdded));
	
	unsigned int index = x1 + y1 * Info.MapWidth;
	int j = y2 - y1 + 1;
	do {
		mf = Field(index);
		i = x2 - x1 + 1;
		do {
			const size_t count = mf->UnitCache.size();
			if(count) {
				unsigned int k = 0;
				do {
					unit = mf->UnitCache.Units[k];
					m = &(unitAdded[unit->Slot]);
					if (!(*m) && !unit->Type->Revealer) {
						Assert(!unit->Removed);
						*m = true;
						table[n++] = unit;
					}				
				} while(++k < count && n < tablesize);
			}
			++mf;
		} while(--i && n < tablesize);
		index += Info.MapWidth;
	} while(--j && n < tablesize);
	
	return n;
}

