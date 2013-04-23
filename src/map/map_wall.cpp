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

static unsigned int getWallTile(const CTileset &tileset, bool humanWall, int dirFlag, int value)
{
	if (humanWall) {
		if (value == 0) {
			return tileset.getHumanWallTile_destroyed(dirFlag);
		} else if (UnitTypeHumanWall && value <= UnitTypeHumanWall->DefaultStat.Variables[HP_INDEX].Max / 2) {
			return tileset.getHumanWallTile_broken(dirFlag);
		} else {
			return tileset.getHumanWallTile(dirFlag);
		}
	} else { // orcWall
		if (value == 0) {
			return tileset.getOrcWallTile_destroyed(dirFlag);
		} else if (UnitTypeOrcWall && value <= UnitTypeOrcWall->DefaultStat.Variables[HP_INDEX].Max / 2) {
			return tileset.getOrcWallTile_broken(dirFlag);
		} else {
			return tileset.getOrcWallTile(dirFlag);
		}
	}
}




//  Calculate the correct tile. Depends on the surrounding.
static int GetDirectionFromSurrounding(const Vec2i &pos, bool human, bool seen)
{
	const Vec2i offsets[4] = {Vec2i(0, -1), Vec2i(1, 0), Vec2i(0, 1), Vec2i(-1, 0)};
	int dirFlag = 0;

	for (int i = 0; i != 4; ++i) {
		const Vec2i newpos = pos + offsets[i];

		if (!Map.Info.IsPointOnMap(newpos)) {
			dirFlag |= 1 << i;
		} else {
			const CMapField &mf = *Map.Field(newpos);
			const unsigned int tile = seen ? mf.playerInfo.SeenTile : mf.Tile;

			if (Map.Tileset->isARaceWallTile(tile, human)) {
				dirFlag |= 1 << i;
			}
		}
	}
	return dirFlag;
}

/**
** Correct the seen wall field, depending on the surrounding.
**
** @param pos Map tile-position.
*/
void MapFixSeenWallTile(const Vec2i &pos)
{
	//  Outside of map or no wall.
	if (!Map.Info.IsPointOnMap(pos)) {
		return;
	}
	CMapField &mf = *Map.Field(pos);
	const CTileset &tileset = *Map.Tileset;
	const unsigned tile = mf.playerInfo.SeenTile;
	if (!tileset.isAWallTile(tile)) {
		return;
	}
	const bool human = tileset.isARaceWallTile(tile, true);
	const int dirFlag = GetDirectionFromSurrounding(pos, human, true);
	const int wallTile = getWallTile(tileset, human, dirFlag, mf.Value);

	if (mf.playerInfo.SeenTile != wallTile) { // Already there!
		mf.playerInfo.SeenTile = wallTile;
		// FIXME: can this only happen if seen?
		if (mf.playerInfo.IsTeamVisible(*ThisPlayer)) {
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
	const Vec2i offset[] = {Vec2i(1, 0), Vec2i(-1, 0), Vec2i(0, 1), Vec2i(0, -1)};

	for (unsigned int i = 0; i < 4; ++i) {
		MapFixSeenWallTile(pos + offset[i]);
	}
}

/**
** Correct the real wall field, depending on the surrounding.
**
** @param pos Map tile-position.
*/
void MapFixWallTile(const Vec2i &pos)
{
	//  Outside of map or no wall.
	if (!Map.Info.IsPointOnMap(pos)) {
		return;
	}
	CMapField &mf = *Map.Field(pos);
	const CTileset &tileset = *Map.Tileset;
	const int tile = mf.Tile;
	if (!tileset.isAWallTile(tile)) {
		return;
	}
	const bool human = tileset.isARaceWallTile(tile, true);
	const int dirFlag = GetDirectionFromSurrounding(pos, human, false);
	const int wallTile = getWallTile(tileset, human, dirFlag, mf.Value);

	if (mf.Tile != wallTile) {
		mf.Tile = wallTile;
		UI.Minimap.UpdateXY(pos);

		if (mf.playerInfo.IsTeamVisible(*ThisPlayer)) {
			UI.Minimap.UpdateSeenXY(pos);
			Map.MarkSeenTile(mf);
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
	const Vec2i offset[] = {Vec2i(1, 0), Vec2i(-1, 0), Vec2i(0, 1), Vec2i(0, -1)};

	for (unsigned int i = 0; i < sizeof(offset) / sizeof(*offset); ++i) {
		MapFixWallTile(pos + offset[i]);
	}
}

/**
** Remove wall from the map.
**
** @param pos  Map position.
**
** FIXME: support more walls of different races.
*/
void CMap::RemoveWall(const Vec2i &pos)
{
	CMapField &mf = *Field(pos);

	mf.Value = 0;

	MapFixWallTile(pos);
	mf.Flags &= ~(MapFieldHuman | MapFieldWall | MapFieldUnpassable);
	MapFixWallNeighbors(pos);
	UI.Minimap.UpdateXY(pos);

	if (mf.playerInfo.IsTeamVisible(*ThisPlayer)) {
		UI.Minimap.UpdateSeenXY(pos);
		this->MarkSeenTile(mf);
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
void CMap::SetWall(const Vec2i &pos, bool humanwall)
{
	CMapField &mf = *Field(pos);

	if (humanwall) {
		const int value = UnitTypeHumanWall->DefaultStat.Variables[HP_INDEX].Max;
		mf.setTileIndex(*Tileset, Tileset->getHumanWallTile(0), value);
	} else {
		const int value = UnitTypeOrcWall->DefaultStat.Variables[HP_INDEX].Max;
		mf.setTileIndex(*Tileset, Tileset->getOrcWallTile(0), value);
	}

	UI.Minimap.UpdateXY(pos);
	MapFixWallTile(pos);
	MapFixWallNeighbors(pos);

	if (mf.playerInfo.IsTeamVisible(*ThisPlayer)) {
		UI.Minimap.UpdateSeenXY(pos);
		this->MarkSeenTile(mf);
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
	const unsigned v = this->Field(pos)->Value;

	if (v <= damage) {
		RemoveWall(pos);
	} else {
		this->Field(pos)->Value = v - damage;
		MapFixWallTile(pos);
	}
}

//@}
