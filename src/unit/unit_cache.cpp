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
void CMap::Insert(CUnit &unit)
{
	Assert(!unit.Removed);
	unsigned int index = unit.Offset;
	const int w = unit.Type->TileWidth;
	const int h = unit.Type->TileHeight;
	int j, i = h;

	do {
		CMapField *mf = Field(index);
		j = w;
		do {
			mf->UnitCache.Insert(&unit);
			++mf;
		} while (--j && unit.tilePos.x + (j - w) < Info.MapWidth);
		index += Info.MapWidth;
	} while (--i && unit.tilePos.y + (i - h) < Info.MapHeight);
}

/**
**  Remove unit from cache.
**
**  @param unit  Unit pointer to remove from cache.
*/
void CMap::Remove(CUnit &unit)
{
	Assert(!unit.Removed);
	unsigned int index = unit.Offset;
	const int w = unit.Type->TileWidth;
	const int h = unit.Type->TileHeight;
	int j, i = h;

	do {
		CMapField *mf = Field(index);
		j = w;
		do {
			mf->UnitCache.Remove(&unit);
			++mf;
		} while (--j && unit.tilePos.x + (j - w) < Info.MapWidth);
		index += Info.MapWidth;
	} while (--i && unit.tilePos.y + (i - h) < Info.MapHeight);
}


void CMap::Clamp(Vec2i &pos) const
{
	clamp<short int>(&pos.x, 0, this->Info.MapWidth - 1);
	clamp<short int>(&pos.y, 0, this->Info.MapHeight - 1);
}

class NoFilter
{
public:
	bool operator()(const CUnit *) const { return true; }
};

void CMap::Select(const Vec2i &ltPos, const Vec2i &rbPos, std::vector<CUnit *> &units)
{
	Select(ltPos, rbPos, units, NoFilter());
}

void CMap::SelectFixed(const Vec2i &ltPos, const Vec2i &rbPos, std::vector<CUnit *> &units)
{
	Select(ltPos, rbPos, units, NoFilter());
}

void CMap::SelectAroundUnit(const CUnit &unit, int range, std::vector<CUnit *> &around)
{
	SelectAroundUnit(unit, range, around, NoFilter());
}

