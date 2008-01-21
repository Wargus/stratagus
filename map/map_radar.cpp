//     ____                _       __               
//    / __ )____  _____   | |     / /___ ___________
//   / __  / __ \/ ___/   | | /| / / __ `/ ___/ ___/
//  / /_/ / /_/ (__  )    | |/ |/ / /_/ / /  (__  ) 
// /_____/\____/____/     |__/|__/\__,_/_/  /____/  
//                                              
//       A futuristic real-time strategy game.
//          This file is part of Bos Wars.
//
/**@name map_radar.cpp - The map radar handling. */
//
//      (c) Copyright 2004-2007 by Russell Smith and Jimmy Salmon.
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
#include "player.h"
#include "unittype.h"
#include "unit.h"
#include "map.h"

/*----------------------------------------------------------------------------
--  Functions
----------------------------------------------------------------------------*/

/**
**  Find out if a unit is visible under radar (By player, or by shared vision)
**
**  @param pradar  Player to who has radar.
**  @param punit   Unit to check for.
**
**  @return        true if radar visible, false if jammed or not radar visible.
*/
bool UnitVisibleOnRadar(const CPlayer *pradar, const CUnit *punit)
{
	for (int i = punit->X; i < punit->X + punit->Type->TileWidth; ++i) {
		for (int j = punit->Y; j < punit->Y + punit->Type->TileHeight; ++j) {
			if (IsTileRadarVisible(pradar, punit->Player, i, j) != 0) {
				return true;
			}
		}
	}

	// Can't exit till the end, as we might be be able to see a different tile
	return false;
}

/**
**  Find out if a unit is visible under radar (By player, or by shared vision)
**
**  @param pradar  Player who has radar.
**  @param punit   Player who is being check.
**  @param x       x tile location to check.
**  @param y       y tile location to check.
**
**  @return        0 jammed or not radar visible, >0 radar visible.
*/
unsigned char IsTileRadarVisible(const CPlayer *pradar, const CPlayer *punit, int x, int y)
{
	unsigned char radarvision;
	unsigned char *radar;
	unsigned char *jamming;

	jamming = Map.Field(x, y)->RadarJammer;
	if (jamming[punit->Index]) {
		return 0;
	}

	radar = Map.Field(x, y)->Radar;
	radarvision = radar[pradar->Index];

	if (!pradar->SharedVision) {
		return radarvision;
	}

	// Check jamming first, if we are jammed, exit
	for (int i = 0; i < PlayerMax; ++i) {
		if (jamming[i] > 0 && (punit->SharedVision & (1 << i)) &&
				(Players[i].SharedVision & (1 << punit->Index))) {
			// We are jammed, return nothing
			return 0;
		}
		if (pradar->SharedVision & (1 << i) &&
				(Players[i].SharedVision & (1 << pradar->Index))) {
			radarvision |= radar[i];
		}
	}

	// Can't exit until the end, as we might be jammed
	return radarvision;
}

/**
**  Mark Radar Vision for a tile
**
**  @param player  The player you are marking for
**  @param x       the X tile to mark.
**  @param y       the Y tile to mark.
*/
void MapMarkTileRadar(const CPlayer *player, int x, int y)
{
	Assert(0 <= x && x < Map.Info.MapWidth);
	Assert(0 <= y && y < Map.Info.MapHeight);
	Assert(Map.Field(x, y)->Radar[player->Index] != 255);

	Map.Field(x, y)->Radar[player->Index]++;
}

/**
**  Unmark Radar Vision for a tile
**
**  @param player  The player you are marking for
**  @param x       the X tile to mark.
**  @param y       the Y tile to mark.
*/
void MapUnmarkTileRadar(const CPlayer *player, int x, int y)
{
	unsigned char *v;

	Assert(0 <= x && x < Map.Info.MapWidth);
	Assert(0 <= y && y < Map.Info.MapHeight);

	// Reduce radar coverage if it exists.
	v = &Map.Field(x, y)->Radar[player->Index];
	if (*v) {
		--*v;
	}
}

/**
**  Mark Radar Jamming Vision for a tile
**
**  @param player  The player you are marking for
**  @param x       the X tile to mark.
**  @param y       the Y tile to mark.
*/
void MapMarkTileRadarJammer(const CPlayer *player, int x, int y)
{
	Assert(0 <= x && x < Map.Info.MapWidth);
	Assert(0 <= y && y < Map.Info.MapHeight);
	Assert(Map.Field(x, y)->RadarJammer[player->Index] != 255);

	Map.Field(x, y)->RadarJammer[player->Index]++;
}

/**
**  Unmark Radar Vision for a tile
**
**  @param player  The player you are marking for
**  @param x       the X tile to mark.
**  @param y       the Y tile to mark.
*/
void MapUnmarkTileRadarJammer(const CPlayer *player, int x, int y)
{
	unsigned char *v;

	Assert(0 <= x && x < Map.Info.MapWidth);
	Assert(0 <= y && y < Map.Info.MapHeight);

	// Reduce radar coverage if it exists.
	v = &Map.Field(x, y)->RadarJammer[player->Index];
	if (*v) {
		--*v;
	}
}
