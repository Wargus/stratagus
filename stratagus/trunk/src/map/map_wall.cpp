//       _________ __                 __
//      /   _____//  |_____________ _/  |______     ____  __ __  ______
//      \_____  \\   __\_  __ \__  \\   __\__  \   / ___\|  |  \/  ___/
//      /        \|  |  |  | \// __ \|  |  / __ \_/ /_/  >  |  /\___ |
//     /_______  /|__|  |__|  (____  /__| (____  /\___  /|____//____  >
//             \/                  \/          \//_____/            \/
//  ______________________                           ______________________
//			  T H E   W A R   B E G I N S
//	   Stratagus - A free fantasy real time strategy game engine
//
/**@name map_wall.c	-	The map wall handling. */
//
//	(c) Copyright 1999-2002 by Vladi Shabanski
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
//	$Id$

//@{

/*----------------------------------------------------------------------------
--		Includes
----------------------------------------------------------------------------*/

#include <stdio.h>

#include "stratagus.h"
#include "map.h"
#include "minimap.h"
#include "player.h"
#if defined(MAP_REGIONS)
#include "pathfinder.h"
#endif

/*----------------------------------------------------------------------------
--		Declarations
----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
--		Variables
----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
--		Functions
----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
--		Fix walls (connections)
----------------------------------------------------------------------------*/

/*
  Vladi:
  NOTE: this is not the original behaviour of the wall demolishing,
  instead I'm replacing tiles just as the wood fixing, so if part of
  a wall is down side neighbours are fixed just as current tile is
  empty one. It is still nice... :)

  For the connecting new walls -- all's fine.
*/

/**
**		Check if the seen tile-type is wall.
**
**		@param x		Map X tile-position.
**		@param y		Map Y tile-position.
**		@param walltype		Walltype to check. (-1 any kind)
*/
global int MapIsSeenTileWall(int x, int y, int walltype)
{
	int t;

	t = TheMap.Tileset->TileTypeTable[
		TheMap.Fields[x + y * TheMap.Width].SeenTile];
	if (walltype == -1) {
		return t == TileTypeHumanWall || t == TileTypeOrcWall;
	}
	return t == walltype;
}

/**
**		Correct the seen wall field, depending on the surrounding.
**
**		@param x		Map X tile-position.
**		@param y		Map Y tile-position.
*/
global void MapFixSeenWallTile(int x, int y)
{
	int t;
	int tile;
	MapField* mf;

	//  Outside of map or no wall.
	if (x < 0 || y < 0 || x >= TheMap.Width || y >= TheMap.Height) {
		return;
	}
	mf = TheMap.Fields + x + y * TheMap.Width;
	t = TheMap.Tileset->TileTypeTable[mf->SeenTile];
	if (t != TileTypeHumanWall && t != TileTypeOrcWall) {
		return;
	}

	//
	//  Calculate the correct tile. Depends on the surrounding.
	//
	tile = 0;
	if ((y - 1) < 0 || MapIsSeenTileWall(x, y - 1, t)) {
		tile |= 1 << 0;
	}
	if ((x + 1) >= TheMap.Width || MapIsSeenTileWall(x + 1, y, t)) {
		tile |= 1 << 1;
	}
	if ((y + 1) >= TheMap.Height || MapIsSeenTileWall(x, y + 1, t)) {
		tile |= 1 << 2;
	}
	if ((x - 1) < 0 || MapIsSeenTileWall(x - 1, y, t)) {
		tile |= 1 << 3;
	}

	if (t == TileTypeHumanWall) {
		tile = TheMap.Tileset->HumanWallTable[tile];
		if (UnitTypeHumanWall && mf->Value <= UnitTypeHumanWall->_HitPoints / 2) {
			while (TheMap.Tileset->Table[tile]){		// Skip good tiles
				++tile;
			}
			while (!TheMap.Tileset->Table[tile]) {		// Skip separator
				++tile;
			}
		}
	} else {
		tile = TheMap.Tileset->OrcWallTable[tile];
		if (UnitTypeOrcWall && mf->Value <= UnitTypeOrcWall->_HitPoints / 2) {
			while (TheMap.Tileset->Table[tile]) {		// Skip good tiles
				++tile;
			}
			while (!TheMap.Tileset->Table[tile]) {		// Skip separator
				++tile;
			}
		}
	}
	if (mf->Value == 0) {
		while (TheMap.Tileset->Table[tile]) {		// Skip good tiles
			++tile;
		}
		while (!TheMap.Tileset->Table[tile]) {		// Skip separator
			++tile;
		}
	}
	tile = TheMap.Tileset->Table[tile];

	if (mf->SeenTile != tile) {				// Already there!
		mf->SeenTile = tile;

		// FIXME: can this only happen if seen?
		if (IsMapFieldVisible(ThisPlayer, x, y)) {
			UpdateMinimapSeenXY(x, y);
			MustRedraw |= RedrawMinimap;
		}
	}
}

/**
**		Correct the surrounding seen wall fields.
**
**		@param x		Map X tile-position.
**		@param y		Map Y tile-position.
*/
global void MapFixSeenWallNeighbors(int x, int y)
{
	MapFixSeenWallTile(x + 1, y);				// side neighbors
	MapFixSeenWallTile(x - 1, y);
	MapFixSeenWallTile(x, y + 1);
	MapFixSeenWallTile(x, y - 1);
}

/**
**		Correct the real wall field, depending on the surrounding.
**
**		@param x		Map X tile-position.
**		@param y		Map Y tile-position.
*/
global void MapFixWallTile(int x, int y)
{
	int tile;
	MapField* mf;
	int t;

	//  Outside of map or no wall.
	if (x < 0 || y < 0 || x >= TheMap.Width || y >= TheMap.Height) {
		return;
	}
	mf = TheMap.Fields + x + y * TheMap.Width;
	if (!(mf->Flags & MapFieldWall)) {
		return;
	}

	t = mf->Flags & (MapFieldHuman | MapFieldWall);
	//
	//  Calculate the correct tile. Depends on the surrounding.
	//
	tile = 0;
	if ((y - 1) < 0 || (TheMap.Fields[x + (y - 1) * TheMap.Width].
			Flags & t) == t) {
		tile |= 1 << 0;
	}
	if ((x + 1) >= TheMap.Width || (TheMap.Fields[x + 1 + y * TheMap.Width].
			Flags & t) == t) {
		tile |= 1 << 1;
	}
	if ((y + 1) >= TheMap.Height || (TheMap.Fields[x + (y + 1) * TheMap.Width].
			Flags & t) == t) {
		tile |= 1 << 2;
	}
	if ((x - 1) < 0 || (TheMap.Fields[x - 1 + y * TheMap.Width].
			Flags & t) == t) {
		tile |= 1 << 3;
	}

	if (t & MapFieldHuman) {
		tile = TheMap.Tileset->HumanWallTable[tile];
		if (UnitTypeHumanWall && mf->Value <= UnitTypeHumanWall->_HitPoints / 2) {
			while (TheMap.Tileset->Table[tile]) {		// Skip good tiles
				++tile;
			}
			while (!TheMap.Tileset->Table[tile]) {		// Skip separator
				++tile;
			}
		}
	} else {
		tile = TheMap.Tileset->OrcWallTable[tile];
		if (UnitTypeOrcWall && mf->Value <= UnitTypeOrcWall->_HitPoints / 2) {
			while (TheMap.Tileset->Table[tile]) {		// Skip good tiles
				++tile;
			}
			while (!TheMap.Tileset->Table[tile]) {		// Skip separator
				++tile;
			}
		}
	}
	if (mf->Value == 0) {
		while (TheMap.Tileset->Table[tile]) {		// Skip good tiles
			++tile;
		}
		while (!TheMap.Tileset->Table[tile]) {		// Skip separator
			++tile;
		}
	}
	tile = TheMap.Tileset->Table[tile];

	if( mf->Tile != tile ) {
		mf->Tile = tile;
		UpdateMinimapXY(x, y);

		if (IsMapFieldVisible(ThisPlayer, x, y)) {
			UpdateMinimapSeenXY(x, y);
			MapMarkSeenTile(x, y);
			MustRedraw |= RedrawMinimap;
		}
	}
}

/**
**		Correct the surrounding real wall fields.
**
**		@param x		Map X tile-position.
**		@param y		Map Y tile-position.
*/
local void MapFixWallNeighbors(int x, int y)
{
	MapFixWallTile(x + 1, y);				// side neighbors
	MapFixWallTile(x - 1, y);
	MapFixWallTile(x, y + 1);
	MapFixWallTile(x, y - 1);
}

/**
**		Remove wall from the map.
**
**		@param x		Map X position.
**		@param y		Map Y position.
*/
global void MapRemoveWall(unsigned x, unsigned y)
{
	MapField* mf;

	mf = TheMap.Fields + x + y * TheMap.Width;
	// FIXME: support more walls of different races.
	mf->Flags &= ~(MapFieldHuman | MapFieldWall | MapFieldUnpassable);

	UpdateMinimapXY(x, y);
	MapFixWallTile(x, y);
	MapFixWallNeighbors(x, y);

	if (IsMapFieldVisible(ThisPlayer, x, y)) {
		UpdateMinimapSeenXY(x, y);
		MapMarkSeenTile(x, y);
		MustRedraw |= RedrawMinimap;
	}
#ifdef MAP_REGIONS
	MapSplitterTilesCleared(x, y, x, y);
#endif
}

/**
**		Set wall onto the map.
**
**		@param x		Map X position.
**		@param y		Map Y position.
**		@param humanwall		Flag, if true set a human wall.
**
**		@todo		FIXME: support for more races.
*/
global void MapSetWall(unsigned x, unsigned y, int humanwall)
{
	MapField* mf;

	mf = TheMap.Fields + x + y * TheMap.Width;

	// FIXME: support more walls of different races.
	if (humanwall) {
		// FIXME: Set random walls
		mf->Tile = TheMap.Tileset->Table[TheMap.Tileset->HumanWallTable[0]];
		mf->Flags |= MapFieldWall | MapFieldUnpassable | MapFieldHuman;
		mf->Value = UnitTypeHumanWall->_HitPoints;
	} else {
		// FIXME: Set random walls
		mf->Tile = TheMap.Tileset->Table[TheMap.Tileset->OrcWallTable[0]];
		mf->Flags |= MapFieldWall | MapFieldUnpassable;
		mf->Value = UnitTypeOrcWall->_HitPoints;
	}

	UpdateMinimapXY(x, y);
	MapFixWallTile(x, y);
	MapFixWallNeighbors(x, y);

	if (IsMapFieldVisible(ThisPlayer, x, y)) {
		UpdateMinimapSeenXY(x, y);
		MapMarkSeenTile(x, y);
		MustRedraw |= RedrawMinimap;
	}
#ifdef MAP_REGIONS
	MapSplitterTilesOccuped(x, y, x, y);
#endif
}

/**
**		Wall is hit with damage.
**
**		@param x		Map X tile-position of wall.
**		@param y		Map Y tile-position of wall.
**		@param damage		Damage done to wall.
*/
global void HitWall(unsigned x, unsigned y, unsigned damage)
{
	unsigned v;

	v = TheMap.Fields[x + y * TheMap.Width].Value;
	if( v <= damage ) {
		TheMap.Fields[x + y * TheMap.Width].Value = 0;
		MapRemoveWall(x, y);
	} else {
		TheMap.Fields[x + y * TheMap.Width].Value = v - damage;
		MapFixWallTile(x, y);
	}
}

//@}
