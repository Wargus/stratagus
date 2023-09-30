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
/**@name map_radar.cpp - The map radar handling. */
//
//      (c) Copyright 2004-2006 by Russell Smith and Jimmy Salmon.
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

#include "stratagus.h"

#include "map.h"

#include "player.h"
#include "unittype.h"
#include "unit.h"


/*----------------------------------------------------------------------------
--  Functions
----------------------------------------------------------------------------*/

static inline unsigned char
IsTileRadarVisible(const CPlayer &pradar, const CPlayer &punit, const CMapFieldPlayerInfo &mfp)
{
	if (mfp.RadarJammer[punit.Index]) {
		return 0;
	}

	if (pradar.IsVisionSharing()) {
		const uint8_t *const radar = mfp.Radar;
		const uint8_t *const jamming = mfp.RadarJammer;
		uint8_t radarvision = 0;
		// Check jamming first, if we are jammed, exit
		for (const uint8_t p : punit.GetSharedVision()) {
			if (p == pradar.Index) {
				continue;
			}
			if (jamming[p] > 0) {
				return 0;
			}
		}

		for (const uint8_t p : pradar.GetSharedVision()) {
			if (p == pradar.Index) {
				continue;
			}
			if (radar[p] > 0) {
				radarvision |= radar[p];
			}
		}

		// Can't exit until the end, as we might be jammed
		return (radarvision | mfp.Radar[pradar.Index]);
	}
	return mfp.Radar[pradar.Index];
}


bool CUnit::IsVisibleOnRadar(const CPlayer &pradar) const
{
	const int x_max = Type->TileWidth;
	unsigned int index = Offset;
	int j = Type->TileHeight;
	do {
		const CMapField *mf = Map.Field(index);
		int i = x_max;
		do {
			if (IsTileRadarVisible(pradar, *Player, mf->playerInfo) != 0) {
				return true;
			}
			++mf;
		} while (--i);
		index += Map.Info.MapWidth;
	} while (--j);

	// Can't exit till the end, as we might be be able to see a different tile
	return false;
}


/**
**  Mark Radar Vision for a tile
**
**  @param player  The player you are marking for
**  @param x       the X tile to mark.
**  @param y       the Y tile to mark.
*/
void MapMarkTileRadar(const CPlayer &player, const unsigned int index)
{
	Assert(Map.Field(index)->playerInfo.Radar[player.Index] != 255);
	Map.Field(index)->playerInfo.Radar[player.Index]++;
}

void MapMarkTileRadar(const CPlayer &player, int x, int y)
{
	Assert(Map.Info.IsPointOnMap(x, y));
	MapMarkTileRadar(player, Map.getIndex(x, y));
}


/**
**  Unmark Radar Vision for a tile
**
**  @param player  The player you are marking for
**  @param x       the X tile to mark.
**  @param y       the Y tile to mark.
*/
void MapUnmarkTileRadar(const CPlayer &player, const unsigned int index)
{
	// Reduce radar coverage if it exists.
	unsigned char *v = &(Map.Field(index)->playerInfo.Radar[player.Index]);
	if (*v) {
		--*v;
	}
}

void MapUnmarkTileRadar(const CPlayer &player, int x, int y)
{
	Assert(Map.Info.IsPointOnMap(x, y));
	MapUnmarkTileRadar(player, Map.getIndex(x, y));
}


/**
**  Mark Radar Jamming Vision for a tile
**
**  @param player  The player you are marking for
**  @param x       the X tile to mark.
**  @param y       the Y tile to mark.
*/
void MapMarkTileRadarJammer(const CPlayer &player, const unsigned int index)
{
	Assert(Map.Field(index)->playerInfo.RadarJammer[player.Index] != 255);
	Map.Field(index)->playerInfo.RadarJammer[player.Index]++;
}

void MapMarkTileRadarJammer(const CPlayer &player, int x, int y)
{
	Assert(Map.Info.IsPointOnMap(x, y));
	MapMarkTileRadarJammer(player, Map.getIndex(x, y));
}

/**
**  Unmark Radar Vision for a tile
**
**  @param player  The player you are marking for
**  @param x       the X tile to mark.
**  @param y       the Y tile to mark.
*/
void MapUnmarkTileRadarJammer(const CPlayer &player, const unsigned int index)
{
	// Reduce radar coverage if it exists.
	unsigned char *v = &(Map.Field(index)->playerInfo.RadarJammer[player.Index]);
	if (*v) {
		--*v;
	}
}

void MapUnmarkTileRadarJammer(const CPlayer &player, int x, int y)
{
	Assert(Map.Info.IsPointOnMap(x, y));
	MapUnmarkTileRadarJammer(player, Map.getIndex(x, y));
}

