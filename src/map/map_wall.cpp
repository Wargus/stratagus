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
/**@name map_wall.cpp - The map wall handling. */
//
//      (c) Copyright 1999-2005 by Vladi Shabanski
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

#include <stdio.h>

#include "stratagus.h"
#include "map.h"
#include "tileset.h"
#include "ui.h"
#include "player.h"
#include "unittype.h"

/*----------------------------------------------------------------------------
-- Declarations
----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
-- Variables
----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
-- Functions
----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
-- Fix walls (connections)
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
** Check if the seen tile-type is wall.
**
** @param x Map X tile-position.
** @param y Map Y tile-position.
** @param walltype Walltype to check. (-1 any kind)
*/
static int MapIsSeenTileWall(int x, int y, int walltype)
{
	int t;

	t = Map.Tileset.TileTypeTable[Map.Field(x, y)->SeenTile];
	if (walltype == -1) {
		return t == TileTypeHumanWall || t == TileTypeOrcWall;
	}
	return t == walltype;
}

/**
** Correct the seen wall field, depending on the surrounding.
**
** @param x Map X tile-position.
** @param y Map Y tile-position.
*/
void MapFixSeenWallTile(const Vec2i &pos)
{
	int t;
	int tile;
	CMapField *mf;

	//  Outside of map or no wall.
	if (!Map.Info.IsPointOnMap(pos)) {
		return;
	}
	mf = Map.Field(pos);
	t = Map.Tileset.TileTypeTable[mf->SeenTile];
	if (t != TileTypeHumanWall && t != TileTypeOrcWall) {
		return;
	}

	//
	//  Calculate the correct tile. Depends on the surrounding.
	//
	tile = 0;
	if ((pos.y - 1) < 0 || MapIsSeenTileWall(pos.x, pos.y - 1, t)) {
		tile |= 1 << 0;
	}
	if ((pos.x + 1) >= Map.Info.MapWidth || MapIsSeenTileWall(pos.x + 1, pos.y, t)) {
		tile |= 1 << 1;
	}
	if ((pos.y + 1) >= Map.Info.MapHeight || MapIsSeenTileWall(pos.x, pos.y + 1, t)) {
		tile |= 1 << 2;
	}
	if ((pos.x - 1) < 0 || MapIsSeenTileWall(pos.x - 1, pos.y, t)) {
		tile |= 1 << 3;
	}

	if (t == TileTypeHumanWall) {
		tile = Map.Tileset.HumanWallTable[tile];
		if (UnitTypeHumanWall && mf->Value <= UnitTypeHumanWall->Variable[HP_INDEX].Max / 2) {
			while (Map.Tileset.Table[tile]) { // Skip good tiles
				++tile;
			}
			while (!Map.Tileset.Table[tile]) { // Skip separator
				++tile;
			}
		}
	} else {
		tile = Map.Tileset.OrcWallTable[tile];
		if (UnitTypeOrcWall && mf->Value <= UnitTypeOrcWall->Variable[HP_INDEX].Max / 2) {
			while (Map.Tileset.Table[tile]) { // Skip good tiles
				++tile;
			}
			while (!Map.Tileset.Table[tile]) { // Skip separator
				++tile;
			}
		}
	}
	if (mf->Value == 0) {
		while (Map.Tileset.Table[tile]) { // Skip good tiles
			++tile;
		}
		while (!Map.Tileset.Table[tile]) { // Skip separator
			++tile;
		}
	}
	tile = Map.Tileset.Table[tile];

	if (mf->SeenTile != tile) { // Already there!
		mf->SeenTile = tile;

		// FIXME: can this only happen if seen?
		if (Map.IsFieldVisible(*ThisPlayer, pos)) {
			UI.Minimap.UpdateSeenXY(pos);
		}
	}
}

/**
** Correct the surrounding seen wall fields.
**
** @param pos Map tile-position.
*/
void MapFixSeenWallNeighbors(const Vec2i &pos)
{
	const Vec2i offset[] = {{1, 0}, {-1, 0}, {0, 1}, {0, -1}};

	for (unsigned int i = 0; i < 4; ++i)
	{
		MapFixSeenWallTile(pos + offset[i]);
	}
}

/**
** Correct the real wall field, depending on the surrounding.
**
** @param x Map X tile-position.
** @param y Map Y tile-position.
*/
void MapFixWallTile(const Vec2i &pos)
{
	//  Outside of map or no wall.
	if (!Map.Info.IsPointOnMap(pos)) {
		return;
	}
	CMapField *mf = Map.Field(pos);
	if (!(mf->Flags & MapFieldWall)) {
		return;
	}

	int t = mf->Flags & (MapFieldHuman | MapFieldWall);
	//
	//  Calculate the correct tile. Depends on the surrounding.
	//
	int tile = 0;
	if ((pos.y - 1) < 0 || (Map.Field(pos.x, (pos.y - 1))->
			Flags & (MapFieldHuman | MapFieldWall)) == t) {
		tile |= 1 << 0;
	}
	if ((pos.x + 1) >= Map.Info.MapWidth || (Map.Field(pos.x + 1, pos.y)->
			Flags & (MapFieldHuman | MapFieldWall)) == t) {
		tile |= 1 << 1;
	}
	if ((pos.y + 1) >= Map.Info.MapHeight || (Map.Field(pos.x, pos.y + 1)->
			Flags & (MapFieldHuman | MapFieldWall)) == t) {
		tile |= 1 << 2;
	}
	if ((pos.x - 1) < 0 || (Map.Field(pos.x - 1, pos.y)->
			Flags & (MapFieldHuman | MapFieldWall)) == t) {
		tile |= 1 << 3;
	}

	if (t & MapFieldHuman) {
		tile = Map.Tileset.HumanWallTable[tile];
		if (UnitTypeHumanWall && mf->Value <= UnitTypeHumanWall->Variable[HP_INDEX].Max / 2) {
			while (Map.Tileset.Table[tile]) { // Skip good tiles
				++tile;
			}
			while (!Map.Tileset.Table[tile]) { // Skip separator
				++tile;
			}
		}
	} else {
		tile = Map.Tileset.OrcWallTable[tile];
		if (UnitTypeOrcWall && mf->Value <= UnitTypeOrcWall->Variable[HP_INDEX].Max / 2) {
			while (Map.Tileset.Table[tile]) { // Skip good tiles
				++tile;
			}
			while (!Map.Tileset.Table[tile]) { // Skip separator
				++tile;
			}
		}
	}
	if (mf->Value == 0) {
		while (Map.Tileset.Table[tile]) { // Skip good tiles
			++tile;
		}
		while (!Map.Tileset.Table[tile]) { // Skip separator
			++tile;
		}
	}
	tile = Map.Tileset.Table[tile];

	if (mf->Tile != tile) {
		mf->Tile = tile;
		UI.Minimap.UpdateXY(pos);

		if (Map.IsFieldVisible(*ThisPlayer, pos)) {
			UI.Minimap.UpdateSeenXY(pos);
			Map.MarkSeenTile(pos);
		}
	}
}

/**
** Correct the surrounding real wall fields.
**
** @param pos Map tile-position.
*/
static void MapFixWallNeighbors(const Vec2i &pos)
{
	const Vec2i offset[] = {{1, 0}, {-1, 0}, {0, 1}, {0, -1}};

	for (unsigned int i = 0; i < sizeof (offset) / sizeof (*offset); ++i)
	{
		MapFixWallTile(pos + offset[i]);
	}
}

/**
** Remove wall from the map.
**
** @param pos  Map position.
*/
void CMap::RemoveWall(const Vec2i &pos)
{
	CMapField *mf = Field(pos);

	mf->Value = 0;
	// FIXME: support more walls of different races.
	mf->Flags &= ~(MapFieldHuman | MapFieldWall | MapFieldUnpassable);

	UI.Minimap.UpdateXY(pos);
	MapFixWallTile(pos);
	MapFixWallNeighbors(pos);

	if (Map.IsFieldVisible(*ThisPlayer, pos)) {
		UI.Minimap.UpdateSeenXY(pos);
		this->MarkSeenTile(pos);
	}
}

/**
** Set wall onto the map.
**
** @param pos  Map position.
** @param humanwall Flag, if true set a human wall.
**
** @todo FIXME: support for more races.
*/
void CMap::SetWall(const Vec2i &pos, int humanwall)
{
	CMapField *mf = Field(pos);

	// FIXME: support more walls of different races.
	if (humanwall) {
		// FIXME: Set random walls
		mf->Tile = this->Tileset.Table[this->Tileset.HumanWallTable[0]];
		mf->Flags |= MapFieldWall | MapFieldUnpassable | MapFieldHuman;
		mf->Value = UnitTypeHumanWall->Variable[HP_INDEX].Max;
	} else {
		// FIXME: Set random walls
		mf->Tile = this->Tileset.Table[this->Tileset.OrcWallTable[0]];
		mf->Flags |= MapFieldWall | MapFieldUnpassable;
		mf->Value = UnitTypeOrcWall->Variable[HP_INDEX].Max;
	}

	UI.Minimap.UpdateXY(pos);
	MapFixWallTile(pos);
	MapFixWallNeighbors(pos);

	if (Map.IsFieldVisible(*ThisPlayer, pos)) {
		UI.Minimap.UpdateSeenXY(pos);
		this->MarkSeenTile(pos);
	}
}

/**
** Wall is hit with damage.
**
** @param pos     Map tile-position of wall.
** @param damage  Damage done to wall.
*/
void CMap::HitWall(const Vec2i &pos, unsigned damage)
{
	unsigned v;

	v = this->Field(pos)->Value;
	if (v <= damage) {
		RemoveWall(pos);
	} else {
		this->Field(pos)->Value = v - damage;
		MapFixWallTile(pos);
	}
}

//@}
