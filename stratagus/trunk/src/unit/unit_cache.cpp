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
/**@name unit_cache.c - The unit cache. */
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
//	    along with this program; if not, write to the Free Software
//	    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
//	    02111-1307, USA.
//
//		$Id$

//@{

/*----------------------------------------------------------------------------
--		Includes
----------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "stratagus.h"
#include "unit.h"
#include "map.h"

/**
**  Insert new unit into cache.
**
**  @param unit  Unit pointer to place in cache.
*/
global void UnitCacheInsert(Unit* unit)
{
	int i;
	int j;
	MapField* mf;
	UnitListItem* listitem;

	for (i = 0; i < unit->Type->TileHeight; ++i) {
		for (j = 0; j < unit->Type->TileWidth; ++j) {
			mf = TheMap.Fields + (i + unit->Y) * TheMap.Width + j + unit->X;
			listitem = unit->CacheLinks + i * unit->Type->TileWidth + j;
			Assert(!listitem->Next && !listitem->Prev);

			// Always add at the start of the list.
			listitem->Next = mf->UnitCache;
			listitem->Prev = 0;
			// update Prev link if cache on tile is no empty
			if (mf->UnitCache) {
				mf->UnitCache->Prev = listitem;
			}
			mf->UnitCache = listitem;
		}
	}
}

/**
**  Remove unit from cache.
**
**  @param unit  Unit pointer to remove from cache.
*/
global void UnitCacheRemove(Unit* unit)
{
	int i;
	int j;
	MapField* mf;
	UnitListItem* listitem;

	for (i = 0; i < unit->Type->TileHeight; ++i) {
		for (j = 0; j < unit->Type->TileWidth; ++j) {
			mf = TheMap.Fields + (i + unit->Y) * TheMap.Width + j + unit->X;
			listitem = unit->CacheLinks + i * unit->Type->TileWidth + j;

			if (listitem->Next) {
				listitem->Next->Prev = listitem->Prev;
			}
			if (listitem->Prev) {
				listitem->Prev->Next = listitem->Next;
			} else {
				if (mf->UnitCache != listitem) {
					DebugPrint("Try to remove unit not in cache. (%d)\n" _C_ unit->Slot);
				} else {
					// item is head of the list.
					mf->UnitCache = listitem->Next;
					Assert(!mf->UnitCache || !mf->UnitCache->Prev);
				}
			}

			listitem->Next = listitem->Prev = 0;
		}
	}
}

/**
**  Change unit in cache.
**  FIXME: optimize, add destination to parameters
**
**  @param unit  Unit pointer to change in cache.
*/
global void UnitCacheChange(Unit* unit)
{
	UnitCacheRemove(unit);
	UnitCacheInsert(unit);
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
global int UnitCacheSelect(int x1, int y1, int x2, int y2, Unit** table)
{
	int i;
	int j;
	int n;
	UnitListItem* listitem;

	// Optimize small searches.
	if (x1 == x2 && y1 == y2) {
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
	if (x2 > TheMap.Width) {
		x2 = TheMap.Width;
	}
	if (y2 > TheMap.Height) {
		y2 = TheMap.Height;
	}

	n = 0;
	for (i = y1; i < y2; ++i) {
		for (j = x1; j < x2; ++j) {
			listitem = TheMap.Fields[i * TheMap.Width + j].UnitCache;
			for (; listitem; listitem = listitem->Next) {
				//
				//	To avoid getting an unit in multiple times we use a cache lock.
				//	It should only be used in here, unless you somehow want the unit
				//	to be out of cache.
				//
				if (!listitem->Unit->CacheLock) {
					listitem->Unit->CacheLock = 1;
					table[n++] = listitem->Unit;
				}
			}
		}
	}

	//
	//	Clean the cache locks, restore to original situation.
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
global int UnitCacheOnTile(int x, int y, Unit** table)
{
	UnitListItem* listitem;
	int n;

	//
	//	Unlike in UnitCacheSelect, there's no way an unit can show up twice,
	//	so there is no need for Cache Locks.
	//
	n = 0;
	listitem = TheMap.Fields[y * TheMap.Width + x].UnitCache;
	for (; listitem; listitem = listitem->Next) {
		if (!listitem->Unit->CacheLock) {
			table[n++] = listitem->Unit;
		}
	}

	return n;
}

/**
**  Initialize unit-cache.
*/
global void InitUnitCache(void)
{
}
