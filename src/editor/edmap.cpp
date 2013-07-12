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
/**@name edmap.cpp - Editor map functions. */
//
//      (c) Copyright 2002-2005 by Lutz Sammer
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
#include "editor.h"
#include "map.h"
#include "tileset.h"
#include "ui.h"
#include "player.h"
#include "unit.h"
#include "unit_manager.h"
#include "unittype.h"


/// Callback for changed tile (with direction mask)
static void EditorChangeSurrounding(const Vec2i &pos, int d);

/*----------------------------------------------------------------------------
--  Functions
----------------------------------------------------------------------------*/

/**
**  Change tile from abstract tile-type.
**
**  @param pos   map tile coordinate.
**  @param tile  tile type to edit.
**
**  @note  this is a rather dumb function, doesn't do any tile fixing.
*/
void ChangeTile(const Vec2i &pos, int tile)
{
	Assert(Map.Info.IsPointOnMap(pos));

	CMapField &mf = *Map.Field(pos);
	mf.setGraphicTile(tile);
	mf.playerInfo.SeenTile = tile;
}


/**
**  Get quad from tile.
**
**  A quad is a 32 bit value defining the content of the tile.
**
**  A tile is split into 4 parts, the basic tile type of this part
**    is stored as 8bit value in the quad.
**
**  ab
**  cd -> abcd
*/
static unsigned QuadFromTile(const Vec2i &pos)
{
	// find the abstact tile number
	const int tile = Map.Field(pos)->getGraphicTile();
	return Map.Tileset->getQuadFromTile(tile);
}

/**
**  Editor change tile.
**
**  @param pos   map tile coordinate.
**  @param tileIndex  Tile type to edit.
**  @param d     Fix direction flag 8 up, 4 down, 2 left, 1 right.
*/
static void EditorChangeTile(const Vec2i &pos, int tileIndex, int d)
{
	Assert(Map.Info.IsPointOnMap(pos));

	// Change the flags
	CMapField &mf = *Map.Field(pos);
	mf.setTileIndex(*Map.Tileset, tileIndex, 0);
	mf.playerInfo.SeenTile = mf.getGraphicTile();

	UI.Minimap.UpdateSeenXY(pos);
	UI.Minimap.UpdateXY(pos);

	EditorChangeSurrounding(pos, d);
}

/**
**  Update surroundings for tile changes.
**
**  @param pos  Map tile position of change.
**  @param d  Fix direction flag 8 up, 4 down, 2 left, 1 right.
*/
static void EditorChangeSurrounding(const Vec2i &pos, int d)
{
	// Special case 1) Walls.
	CMapField &mf = *Map.Field(pos);
	if (mf.isAWall()) {
		Map.SetWall(pos, mf.isHuman());
		return;
	}

	const unsigned int quad = QuadFromTile(pos);
	const unsigned int TH_QUAD_M = 0xFFFF0000; // Top half quad mask
	const unsigned int BH_QUAD_M = 0x0000FFFF; // Bottom half quad mask
	const unsigned int LH_QUAD_M = 0xFF00FF00; // Left half quad mask
	const unsigned int RH_QUAD_M = 0x00FF00FF; // Right half quad mask
	const unsigned int DIR_UP =    8; // Go up allowed
	const unsigned int DIR_DOWN =  4; // Go down allowed
	const unsigned int DIR_LEFT =  2; // Go left allowed
	const unsigned int DIR_RIGHT = 1; // Go right allowed

	// How this works:
	//  first get the quad of the neighbouring tile,
	//  then check if the margin matches.
	//  Otherwise, call EditorChangeTile again.
	if ((d & DIR_UP) && pos.y) {
		const Vec2i offset(0, -1);
		// Insert into the bottom the new tile.
		unsigned q2 = QuadFromTile(pos + offset);
		unsigned u = (q2 & TH_QUAD_M) | ((quad >> 16) & BH_QUAD_M);
		if (u != q2) {
			int tile = Map.Tileset->tileFromQuad(u & BH_QUAD_M, u);
			EditorChangeTile(pos + offset, tile, d & ~DIR_DOWN);
		}
	}
	if ((d & DIR_DOWN) && pos.y < Map.Info.MapHeight - 1) {
		const Vec2i offset(0, 1);
		// Insert into the top the new tile.
		unsigned q2 = QuadFromTile(pos + offset);
		unsigned u = (q2 & BH_QUAD_M) | ((quad << 16) & TH_QUAD_M);
		if (u != q2) {
			int tile = Map.Tileset->tileFromQuad(u & TH_QUAD_M, u);
			EditorChangeTile(pos + offset, tile, d & ~DIR_UP);
		}
	}
	if ((d & DIR_LEFT) && pos.x) {
		const Vec2i offset(-1, 0);
		// Insert into the left the new tile.
		unsigned q2 = QuadFromTile(pos + offset);
		unsigned u = (q2 & LH_QUAD_M) | ((quad >> 8) & RH_QUAD_M);
		if (u != q2) {
			int tile = Map.Tileset->tileFromQuad(u & RH_QUAD_M, u);
			EditorChangeTile(pos + offset, tile, d & ~DIR_RIGHT);
		}
	}
	if ((d & DIR_RIGHT) && pos.x < Map.Info.MapWidth - 1) {
		const Vec2i offset(1, 0);
		// Insert into the right the new tile.
		unsigned q2 = QuadFromTile(pos + offset);
		unsigned u = (q2 & RH_QUAD_M) | ((quad << 8) & LH_QUAD_M);
		if (u != q2) {
			int tile = Map.Tileset->tileFromQuad(u & LH_QUAD_M, u);
			EditorChangeTile(pos + offset, tile, d & ~DIR_LEFT);
		}
	}
}

/**
**  Update surroundings for tile changes.
**
**  @param pos  Map tile position of change.
*/
void EditorTileChanged(const Vec2i &pos)
{
	EditorChangeSurrounding(pos, 0x0F);
}

/**
**  Make random map
**  FIXME: vladi: we should have parameters control here...
*/

/**
**  TileFill
**
**  @param pos   map tile coordinate for area center.
**  @param tile  Tile type to edit.
**  @param size  Size of surrounding rectangle.
**
**  TileFill(centerx, centery, tile_type_water, map_width)
**  will fill map with water...
*/
static void TileFill(const Vec2i &pos, int tile, int size)
{
	const Vec2i diag(size / 2, size / 2);
	Vec2i ipos = pos - diag;
	Vec2i apos = pos + diag;

	Map.FixSelectionArea(ipos, apos);

	Vec2i itPos;
	for (itPos.x = ipos.x; itPos.x <= apos.x; ++itPos.x) {
		for (itPos.y = ipos.y; itPos.y <= apos.y; ++itPos.y) {
			EditorChangeTile(itPos, tile, 15);
		}
	}
}

/**
**  Randomize tiles and fill in map
**
**  @param tile      tile number to use
**  @param count     number of times to apply randomization
**  @param max_size  maximum size of the fill rectangle
*/
static void EditorRandomizeTile(int tile, int count, int max_size)
{
	const Vec2i mpos(Map.Info.MapWidth - 1, Map.Info.MapHeight - 1);

	for (int i = 0; i < count; ++i) {
		const Vec2i rpos(rand() % ((1 + mpos.x) / 2), rand() % ((1 + mpos.y) / 2));
		const Vec2i mirror = mpos - rpos;
		const Vec2i mirrorh(rpos.x, mirror.y);
		const Vec2i mirrorv(mirror.x, rpos.y);
		const int rz = rand() % max_size + 1;

		TileFill(rpos, tile, rz);
		TileFill(mirrorh, tile, rz);
		TileFill(mirrorv, tile, rz);
		TileFill(mirror, tile, rz);
	}
}

#define WATER_TILE  0x10
#define COAST_TILE  0x30
#define GRASS_TILE  0x50
#define WOOD_TILE   0x70
#define ROCK_TILE   0x80

/**
**  Add a unit to random locations on the map, unit will be neutral
**
**  @param unit_type  unit type to add to map as a character string
**  @param count      the number of times to add the unit
**  @param value      resources to be stored in that unit
*/
static void EditorRandomizeUnit(const char *unit_type, int count, int value)
{
	const Vec2i mpos(Map.Info.MapWidth, Map.Info.MapHeight);
	CUnitType *typeptr = UnitTypeByIdent(unit_type);

	if (!typeptr) { // Error
		return;
	}
	CUnitType &type = *typeptr;
	const Vec2i tpos(type.TileWidth, type.TileHeight);

	for (int i = 0; i < count; ++i) {
		const Vec2i rpos(rand() % (mpos.x / 2 - tpos.x + 1), rand() % (mpos.y / 2 - tpos.y + 1));
		const Vec2i mirror(mpos.x - rpos.x - 1, mpos.y - rpos.y - 1);
		const Vec2i mirrorh(rpos.x, mirror.y);
		const Vec2i mirrorv(mirror.x, rpos.y);
		const Vec2i tmirror(mpos.x - rpos.x - tpos.x, mpos.y - rpos.y - tpos.y);
		const Vec2i tmirrorh(rpos.x, tmirror.y);
		const Vec2i tmirrorv(tmirror.x, rpos.y);
		int tile = GRASS_TILE;
		const int z = type.TileHeight;

		// FIXME: vladi: the idea is simple: make proper land for unit(s) :)
		// FIXME: handle units larger than 1 square
		TileFill(rpos, tile, z * 2);
		TileFill(mirrorh, tile, z * 2);
		TileFill(mirrorv, tile, z * 2);
		TileFill(mirror, tile, z * 2);

		// FIXME: can overlap units
		CUnit *unit = MakeUnitAndPlace(rpos, type, &Players[PlayerNumNeutral]);
		if (unit == NULL) {
			DebugPrint("Unable to allocate Unit");
		} else {
			unit->ResourcesHeld = value;
		}

		unit = MakeUnitAndPlace(tmirrorh, type, &Players[PlayerNumNeutral]);
		if (unit == NULL) {
			DebugPrint("Unable to allocate Unit");
		} else {
			unit->ResourcesHeld = value;
		}

		unit = MakeUnitAndPlace(tmirrorv, type, &Players[PlayerNumNeutral]);
		if (unit == NULL) {
			DebugPrint("Unable to allocate Unit");
		} else {
			unit->ResourcesHeld = value;
		}

		unit = MakeUnitAndPlace(tmirror, type, &Players[PlayerNumNeutral]);
		if (unit == NULL) {
			DebugPrint("Unable to allocate Unit");
		} else {
			unit->ResourcesHeld = value;
		}
	}
}

/**
**  Destroy all units
*/
static void EditorDestroyAllUnits()
{
	while (UnitManager.empty() == false) {
		CUnit &unit = **UnitManager.begin();

		unit.Remove(NULL);
		UnitLost(unit);
		UnitClearOrders(unit);
		unit.Release();
	}
}

/**
**  Create a random map
*/
void CEditor::CreateRandomMap() const
{
	const int mz = std::max(Map.Info.MapHeight, Map.Info.MapWidth);

	// make water-base
	const Vec2i zeros(0, 0);
	TileFill(zeros, WATER_TILE, mz * 3);
	// remove all units
	EditorDestroyAllUnits();

	EditorRandomizeTile(COAST_TILE, 10, 16);
	EditorRandomizeTile(GRASS_TILE, 20, 16);
	EditorRandomizeTile(WOOD_TILE,  60,  4);
	EditorRandomizeTile(ROCK_TILE,  30,  2);

	EditorRandomizeUnit("unit-gold-mine", 5, 50000);
}

//@}
