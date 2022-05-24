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

#include <chrono>
#include <random>
#include <cmath>

#include "stratagus.h"
#include "editor.h"
#include "map.h"
#include "tileset.h"
#include "ui.h"
#include "player.h"
#include "unit.h"
#include "unit_manager.h"
#include "unittype.h"


/// Callback for changed tile (with locked position)
static void EditorChangeSurrounding(const Vec2i &pos, const Vec2i &lock_pos);

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
**  @param lock_pos   map tile coordinate, that should not be changed in callback.
*/
void EditorChangeTile(const Vec2i &pos, int tileIndex, const Vec2i &lock_pos, bool changeSurroundings)
{
	Assert(Map.Info.IsPointOnMap(pos));

	// Change the flags
	CMapField &mf = *Map.Field(pos);
	int tile = tileIndex;
	if (TileToolRandom) {
		int n = 0;
		for (int i = 0; i < 16; ++i) {
			if (!Map.Tileset->tiles[tile + i].tile) {
				break;
			} else {
				++n;
			}
		}
		n = MyRand() % n;
		int i = -1;
		do {
			while (++i < 16 && !Map.Tileset->tiles[tile + i].tile) {
			}
		} while (i < 16 && n--);
		if (i < 16) {
			tile += i;
		}
	}
	mf.setTileIndex(*Map.Tileset, tile, 0);
	mf.playerInfo.SeenTile = mf.getGraphicTile();

	UI.Minimap.UpdateSeenXY(pos);
	UI.Minimap.UpdateXY(pos);

	if (!mf.isDecorative() && changeSurroundings) {
		if (TileToolNoFixup) {
			mf.Flags |= MapFieldDecorative;
		} else {
			EditorChangeSurrounding(pos, lock_pos);
		}
	}
}

/**
**  Update surroundings for tile changes.
**
**  @param pos  Map tile position of change.
**  @param lock_pos  Original change position, that should not be altered.
*/
static void EditorChangeSurrounding(const Vec2i &pos, const Vec2i &lock_pos)
{
	if (TileToolNoFixup) {
		CMapField &mf = *Map.Field(pos);
		mf.Flags |= MapFieldDecorative;
		return;
	}

	// Special case 1) Walls.
	CMapField &mf = *Map.Field(pos);
	if (mf.isAWall()) {
		Map.SetWall(pos, mf.isHuman());
		return;
	}

	if (mf.isDecorative()) {
		return;
	}

	const unsigned int quad = QuadFromTile(pos);
	const unsigned int TH_QUAD_M = 0xFFFF0000; // Top half quad mask
	const unsigned int BH_QUAD_M = 0x0000FFFF; // Bottom half quad mask
	const unsigned int LH_QUAD_M = 0xFF00FF00; // Left half quad mask
	const unsigned int RH_QUAD_M = 0x00FF00FF; // Right half quad mask

	bool did_change = false; // if we changed any tiles

	// How this works:
	//  first get the quad of the neighbouring tile,
	//  then check if the margin matches.
	//  Otherwise, call EditorChangeTile again.
	if (pos.y) {
		const Vec2i offset(0, -1);
		// Insert into the bottom the new tile.
		CMapField *f = Map.Field(pos + offset);
		if (f->isAWall()) {
			EditorChangeSurrounding(pos + offset, pos);
		} else if (!f->isDecorative()) {
			unsigned q2 = QuadFromTile(pos + offset);
			unsigned u = (q2 & TH_QUAD_M) | ((quad >> 16) & BH_QUAD_M);
			if (u != q2 && (pos + offset) != lock_pos) {
				int tile = Map.Tileset->tileFromQuad(u & BH_QUAD_M, u);
				if (tile) {
					did_change = true;
					EditorChangeTile(pos + offset, tile, lock_pos, true);
				}
			}
		}
	}
	if (pos.y < Map.Info.MapHeight - 1) {
		const Vec2i offset(0, 1);
		// Insert into the top the new tile.
		CMapField *f = Map.Field(pos + offset);
		if (f->isAWall()) {
			EditorChangeSurrounding(pos + offset, pos);
		} else if (!f->isDecorative()) {
			unsigned q2 = QuadFromTile(pos + offset);
			unsigned u = (q2 & BH_QUAD_M) | ((quad << 16) & TH_QUAD_M);
			if (u != q2 && (pos + offset) != lock_pos) {
				int tile = Map.Tileset->tileFromQuad(u & TH_QUAD_M, u);
				if (tile) {
					did_change = true;
					EditorChangeTile(pos + offset, tile, lock_pos, true);
				}
			}
		}
	}
	if (pos.x) {
		const Vec2i offset(-1, 0);
		// Insert into the left the new tile.
		CMapField *f = Map.Field(pos + offset);
		if (f->isAWall()) {
			EditorChangeSurrounding(pos + offset, pos);
		} else if (!f->isDecorative()) {
			unsigned q2 = QuadFromTile(pos + offset);
			unsigned u = (q2 & LH_QUAD_M) | ((quad >> 8) & RH_QUAD_M);
			if (u != q2 && (pos + offset) != lock_pos) {
				int tile = Map.Tileset->tileFromQuad(u & RH_QUAD_M, u);
				if (tile) {
					did_change = true;
					EditorChangeTile(pos + offset, tile, lock_pos, true);
				}
			}
		}
	}
	if (pos.x < Map.Info.MapWidth - 1) {
		const Vec2i offset(1, 0);
		// Insert into the right the new tile.
		CMapField *f = Map.Field(pos + offset);
		if (f->isAWall()) {
			EditorChangeSurrounding(pos + offset, pos);
		} else if (!f->isDecorative()) {
			unsigned q2 = QuadFromTile(pos + offset);
			unsigned u = (q2 & RH_QUAD_M) | ((quad << 8) & LH_QUAD_M);
			if (u != q2 && (pos + offset) != lock_pos) {
				int tile = Map.Tileset->tileFromQuad(u & LH_QUAD_M, u);
				if (tile) {
					did_change = true;
					EditorChangeTile(pos + offset, tile, lock_pos, true);
				}
			}
		}
	}

	if (did_change) {
		EditorChangeSurrounding(pos, lock_pos);
	}
}

/**
**  Update surroundings for tile changes.
**
**  @param pos  Map tile position of change.
*/
void EditorTileChanged(const Vec2i &pos)
{
	EditorChangeSurrounding(pos, pos);
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

	// change surroundings unless the fill covers the entire map
	bool changeSurroundings = (ipos.x > 0 || ipos.y > 0 || 
			Map.Info.MapWidth - 1 > apos.x || Map.Info.MapHeight - 1 > apos.y);

	Vec2i itPos;
	for (itPos.x = ipos.x; itPos.x <= apos.x; ++itPos.x) {
		for (itPos.y = ipos.y; itPos.y <= apos.y; ++itPos.y) {
			EditorChangeTile(itPos, tile, itPos, changeSurroundings);
		}
	}
}

static std::mt19937 MersenneTwister(std::chrono::steady_clock::now().time_since_epoch().count());
static int rng() {
	return std::abs((int)MersenneTwister());
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
		const Vec2i rpos(rng() % ((1 + mpos.x) / 2), rng() % ((1 + mpos.y) / 2));
		const Vec2i mirror = mpos - rpos;
		const Vec2i mirrorh(rpos.x, mirror.y);
		const Vec2i mirrorv(mirror.x, rpos.y);
		const int rz = rng() % max_size + 1;

		TileFill(rpos, tile, rz);
		TileFill(mirrorh, tile, rz);
		TileFill(mirrorv, tile, rz);
		TileFill(mirror, tile, rz);
	}
}

/**
**  Add a unit to random locations on the map, unit will be neutral
**
**  @param unit_type  unit type to add to map as a character string
**  @param count      the number of times to add the unit
**  @param value      resources to be stored in that unit
*/
static void EditorRandomizeUnit(const char *unit_type, int count, int value, int tileIndexUnderUnit)
{
	const Vec2i mpos(Map.Info.MapWidth, Map.Info.MapHeight);
	CUnitType *typeptr = UnitTypeByIdent(unit_type);

	if (!typeptr) { // Error
		return;
	}
	CUnitType &type = *typeptr;
	const Vec2i tpos(type.TileWidth, type.TileHeight);

	for (int i = 0; i < count; ++i) {
		const Vec2i rpos(rng() % (mpos.x / 2 - tpos.x + 1), rng() % (mpos.y / 2 - tpos.y + 1));
		const Vec2i mirror(mpos.x - rpos.x - 1, mpos.y - rpos.y - 1);
		const Vec2i mirrorh(rpos.x, mirror.y);
		const Vec2i mirrorv(mirror.x, rpos.y);
		const Vec2i tmirror(mpos.x - rpos.x - tpos.x, mpos.y - rpos.y - tpos.y);
		const Vec2i tmirrorh(rpos.x, tmirror.y);
		const Vec2i tmirrorv(tmirror.x, rpos.y);
		int tile = tileIndexUnderUnit;
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
	while (UnitManager->empty() == false) {
		CUnit &unit = **UnitManager->begin();

		unit.Remove(NULL);
		UnitLost(unit);
		UnitClearOrders(unit);
		unit.Release();
	}
}

static void RandomizeTransition(int x, int y)
{
	CMapField &mf = *Map.Field(x, y);
	const CTileset &tileset = *Map.Tileset;
	int baseTileIndex = tileset.tiles[tileset.findTileIndexByTile(mf.getGraphicTile())].tileinfo.BaseTerrain;
	int mixTerrainIdx = tileset.tiles[tileset.findTileIndexByTile(mf.getGraphicTile())].tileinfo.MixTerrain;
	if (mixTerrainIdx != 0) {
		if (rng() % 8 == 0) {
			// change only in ~12% of cases
			const int tileIdx = tileset.findTileIndex(rng() % 2 ? baseTileIndex : mixTerrainIdx, 0);
			EditorChangeTile(Vec2i(x, y), tileIdx, Vec2i(x, y), true);
		}
	}
}

/**
**  Create a random map
*/
void CEditor::CreateRandomMap(bool shuffleTranslitions) const
{
	const int mz = std::max(Map.Info.MapHeight, Map.Info.MapWidth);

	// remove all units
	EditorDestroyAllUnits();
	// make water-base
	const Vec2i zeros(0, 0);
	TileFill(zeros, BaseTileIndex, mz * 3);
	UI.Minimap.Update();
	EditorUpdateDisplay();


	const char oldRandom = TileToolRandom;
	TileToolRandom = 1;
	for (std::tuple<int, int, int> t : RandomTiles) {
		EditorRandomizeTile(std::get<0>(t), mz / 64 * std::get<1>(t), std::get<2>(t));
		UI.Minimap.Update();
		EditorUpdateDisplay();
	}

	if (shuffleTranslitions) {
		// shuffle transitions in all directions
		// from top left to bottom right
		for (int x = 0; x < Map.Info.MapWidth; x++) {
			for (int y = 0; y < Map.Info.MapHeight; y++) {
				RandomizeTransition(x, y);
			}
		}
		UI.Minimap.Update();
		EditorUpdateDisplay();
		// from bottom right to top left
		for (int x = Map.Info.MapWidth - 1; x >= 0; x--) {
			for (int y = Map.Info.MapHeight - 1; y >= 0; y--) {
				RandomizeTransition(x, y);
			}
		}
		UI.Minimap.Update();
		EditorUpdateDisplay();
	}

	TileToolRandom = oldRandom;

	for (std::tuple<std::string, int, int, int> t : RandomUnits) {
		EditorRandomizeUnit(std::get<0>(t).c_str(), mz / 64 * std::get<1>(t), std::get<2>(t), std::get<3>(t));
		UI.Minimap.Update();
		EditorUpdateDisplay();
	}
}

//@}
