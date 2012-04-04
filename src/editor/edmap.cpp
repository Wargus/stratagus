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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

#include "stratagus.h"
#include "editor.h"
#include "map.h"
#include "tileset.h"
#include "ui.h"
#include "player.h"
#include "unit.h"
#include "unittype.h"

/*----------------------------------------------------------------------------
--  Defines
----------------------------------------------------------------------------*/

#define TH_QUAD_M 0xFFFF0000 /// Top half quad mask
#define BH_QUAD_M 0x0000FFFF /// Bottom half quad mask
#define LH_QUAD_M 0xFF00FF00 /// Left half quad mask
#define RH_QUAD_M 0x00FF00FF /// Right half quad mask

/// Callback for changed tile (with direction mask)
static void EditorTileChanged2(const Vec2i &pos, int d);

/*----------------------------------------------------------------------------
--  Variables
----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
--  Functions
----------------------------------------------------------------------------*/

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
**
**  If the tile is 100% light grass the value is 0x5555.
**  If the tile is 3/4 light grass and dark grass in upper left corner
**    the value is 0x6555.
**
**  @param pos  map tile position
**
**  @return   the 'quad' of the tile.
**
**  @todo     Make a lookup table to speed up the things.
*/
static unsigned QuadFromTile(const Vec2i &pos)
{
	int i;

	//
	// find the abstact tile number
	//
	const int tile = Map.Field(pos)->Tile;
	for (i = 0; i < Map.Tileset.NumTiles; ++i) {
		if (tile == Map.Tileset.Table[i]) {
			break;
		}
	}
	Assert(i != Map.Tileset.NumTiles);

	unsigned base = Map.Tileset.Tiles[i].BaseTerrain;
	unsigned mix = Map.Tileset.Tiles[i].MixTerrain;

	if (!mix) { // a solid tile
		return base | (base << 8) | (base << 16) | (base << 24);
	}
	//
	// Mixed tiles, mix together
	//
	switch ((i & 0x00F0) >> 4) {
		case 0:
			return (base << 24) | (mix << 16) | (mix << 8) | mix;
		case 1:
			return (mix << 24) | (base << 16) | (mix << 8) | mix;
		case 2:
			return (base << 24) | (base << 16) | (mix << 8) | mix;
		case 3:
			return (mix << 24) | (mix << 16) | (base << 8) | mix;
		case 4:
			return (base << 24) | (mix << 16) | (base << 8) | mix;
		case 5:
			return (base << 24) | (base << 16) | (base << 8) | mix;
		case 6:
			return (base << 24) | (base << 16) | (base << 8) | mix;
		case 7:
			return (mix << 24) | (mix << 16) | (mix << 8) | base;
		case 8:
			return (base << 24) | (mix << 16) | (mix << 8) | base;
		case 9:
			return (mix << 24) | (base << 16) | (mix << 8) | base;
		case 10:
			return (base << 24) | (base << 16) | (mix << 8) | base;
		case 11:
			return (mix << 24) | (mix << 16) | (base << 8) | base;
		case 12:
			return (base << 24) | (mix << 16) | (base << 8) | base;
		case 13:
			return (mix << 24) | (base << 16) | (base << 8) | base;
	}

	Assert(0);

	return base | (base << 8) | (base << 16) | (base << 24);
}

/**
**  Find a tile path.
**
**  @param base    Start tile type.
**  @param goal    Goal tile type.
**  @param length  Best found path length.
**  @param marks   Already visited tile types.
**  @param tile    Tile pointer.
*/
static int FindTilePath(int base, int goal, int length, char *marks, int *tile)
{
	int n;
	//
	// Find any mixed tile
	//
	int l = INT_MAX;
	for (int i = 0; i < Map.Tileset.NumTiles;) {
		// goal found.
		if (base == Map.Tileset.Tiles[i].BaseTerrain
			&& goal == Map.Tileset.Tiles[i].MixTerrain) {
			*tile = i;
			return length;
		}
		// goal found.
		if (goal == Map.Tileset.Tiles[i].BaseTerrain
			&& base == Map.Tileset.Tiles[i].MixTerrain) {
			*tile = i;
			return length;
		}

		// possible path found
		if (base == Map.Tileset.Tiles[i].BaseTerrain
			&& Map.Tileset.Tiles[i].MixTerrain) {
			const int j = Map.Tileset.Tiles[i].MixTerrain;
			if (!marks[j]) {
				marks[j] = j;
				n = FindTilePath(j, goal, length + 1, marks, &n);
				marks[j] = 0;
				if (n < l) {
					*tile = i;
					l = n;
				}
			}
			// possible path found
		} else if (Map.Tileset.Tiles[i].BaseTerrain
				   && base == Map.Tileset.Tiles[i].MixTerrain) {
			const int j = Map.Tileset.Tiles[i].BaseTerrain;
			if (!marks[j]) {
				marks[j] = j;
				n = FindTilePath(j, goal, length + 1, marks, &n);
				marks[j] = 0;
				if (n < l) {
					*tile = i;
					l = n;
				}
			}
		}
		// Advance solid or mixed.
		if (!Map.Tileset.Tiles[i].MixTerrain) {
			i += 16;
		} else {
			i += 256;
		}
	}
	return l;
}

/**
**  Get tile from quad.
**
**  @param fixed  Part can't be changed.
**  @param quad   Quad of the tile type.
**  @return       Best matching tile.
*/
static int TileFromQuad(unsigned fixed, unsigned quad)
{
	int i;
	unsigned type1;
	unsigned type2;

	//
	// Get tile type from fixed.
	//
	while (!(type1 = (fixed & 0xFF))) {
		fixed >>= 8;
		if (!fixed) {
			abort();
		}
	}
	fixed >>= 8;
	while (!(type2 = (fixed & 0xFF)) && fixed) {
		fixed >>= 8;
	}
	//
	// Need an second type.
	//
	if (!type2 || type2 == type1) {
		fixed = quad;
		while ((type2 = (fixed & 0xFF)) == type1 && fixed) {
			fixed >>= 8;
		}
		if (type1 == type2) { // Oooh a solid tile.
find_solid:
			//
			// Find the solid tile
			//
			for (i = 0; i < Map.Tileset.NumTiles;) {
				if (type1 == Map.Tileset.Tiles[i].BaseTerrain &&
					!Map.Tileset.Tiles[i].MixTerrain) {
					break;
				}
				// Advance solid or mixed.
				if (!Map.Tileset.Tiles[i].MixTerrain) {
					i += 16;
				} else {
					i += 256;
				}
			}
			Assert(i < Map.Tileset.NumTiles);
			return i;
		}
	} else {
		char *marks = new char[Map.Tileset.NumTerrainTypes];

		memset(marks, 0, Map.Tileset.NumTerrainTypes);
		marks[type1] = type1;
		marks[type2] = type2;

		//
		// What fixed tile-type should replace the non useable tile-types.
		// FIXME: write a loop.
		//
		fixed = (quad >> 0) & 0xFF;
		if (fixed != type1 && fixed != type2) {
			quad &= 0xFFFFFF00;
			if (FindTilePath(type1, fixed, 0, marks, &i) < FindTilePath(type2, fixed, 0, marks, &i)) {
				quad |= type1 << 0;
			} else {
				quad |= type2 << 0;
			}
		}
		fixed = (quad >> 8) & 0xFF;
		if (fixed != type1 && fixed != type2) {
			quad &= 0xFFFF00FF;
			if (FindTilePath(type1, fixed, 0, marks, &i) < FindTilePath(type2, fixed, 0, marks, &i)) {
				quad |= type1 << 8;
			} else {
				quad |= type2 << 8;
			}
		}
		fixed = (quad >> 16) & 0xFF;
		if (fixed != type1 && fixed != type2) {
			quad &= 0xFF00FFFF;
			if (FindTilePath(type1, fixed, 0, marks, &i) < FindTilePath(type2, fixed, 0, marks, &i)) {
				quad |= type1 << 16;
			} else {
				quad |= type2 << 16;
			}
		}
		fixed = (quad >> 24) & 0xFF;
		if (fixed != type1 && fixed != type2) {
			quad &= 0x00FFFFFF;
			if (FindTilePath(type1, fixed, 0, marks, &i) < FindTilePath(type2, fixed, 0, marks, &i)) {
				quad |= type1 << 24;
			} else {
				quad |= type2 << 24;
			}
		}
		delete[] marks;
	}

	//
	// Need a mixed tile
	//
	for (i = 0; i < Map.Tileset.NumTiles;) {
		if (type1 == Map.Tileset.Tiles[i].BaseTerrain && type2 == Map.Tileset.Tiles[i].MixTerrain) {
			break;
		}
		if (type2 == Map.Tileset.Tiles[i].BaseTerrain && type1 == Map.Tileset.Tiles[i].MixTerrain) {
			// Other mixed
			type1 ^= type2;
			type2 ^= type1;
			type1 ^= type2;
			break;
		}
		// Advance solid or mixed.
		if (!Map.Tileset.Tiles[i].MixTerrain) {
			i += 16;
		} else {
			i += 256;
		}
	}

	if (i >= Map.Tileset.NumTiles) {
		//
		// Find the best tile path.
		//
		char *marks = new char[Map.Tileset.NumTerrainTypes];
		memset(marks, 0, Map.Tileset.NumTerrainTypes);
		marks[type1] = type1;
		if (FindTilePath(type1, type2, 0, marks, &i) == INT_MAX) {
			DebugPrint("Huch, no mix found!!!!!!!!!!!\n");
			delete[] marks;
			goto find_solid;
		}
		if (type1 == Map.Tileset.Tiles[i].MixTerrain) {
			// Other mixed
			type1 ^= type2;
			type2 ^= type1;
			type1 ^= type2;
		}

		delete[] marks;
	}

	int base = i;

	int direction = 0;
	if (((quad >> 24) & 0xFF) == type1) {
		direction |= 8;
	}
	if (((quad >> 16) & 0xFF) == type1) {
		direction |= 4;
	}
	if (((quad >> 8) & 0xFF) == type1) {
		direction |= 2;
	}
	if (((quad >> 0) & 0xFF) == type1) {
		direction |= 1;
	}
	//                       0  1  2  3   4  5  6  7   8  9  A   B  C   D  E  F
	const char table[16] = { 0, 7, 3, 11, 1, 9, 5, 13, 0, 8, 4, 12, 2, 10, 6, 0 };
	return base | (table[direction] << 4);
}

/**
**  Change tile from abstract tile-type.
**
**  @param pos   map tile coordinate.
**  @param tile  Abstract tile type to edit.
**
**  @note  this is a rather dumb function, doesn't do any tile fixing.
*/
void ChangeTile(const Vec2i &pos, int tile)
{
	Assert(Map.Info.IsPointOnMap(pos));
	Assert(tile >= 0 && tile < Map.Tileset.NumTiles);

	CMapField *mf = Map.Field(pos);
	mf->Tile = mf->SeenTile = Map.Tileset.Table[tile];
}

#define DIR_UP     8 /// Go up allowed
#define DIR_DOWN   4 /// Go down allowed
#define DIR_LEFT   2 /// Go left allowed
#define DIR_RIGHT  1 /// Go right allowed

/**
**  Editor change tile.
**
**  @param pos   map tile coordinate.
**  @param tile  Tile type to edit.
**  @param d     Fix direction flag 8 up, 4 down, 2 left, 1 right.
*/
static void EditorChangeTile(const Vec2i &pos, int tile, int d)
{
	Assert(Map.Info.IsPointOnMap(pos));

	ChangeTile(pos, tile);

	//
	// Change the flags
	//
	CMapField *mf = Map.Field(pos);
	mf->Flags &= ~(MapFieldHuman | MapFieldLandAllowed | MapFieldCoastAllowed
				   | MapFieldWaterAllowed | MapFieldNoBuilding | MapFieldUnpassable
				   | MapFieldWall | MapFieldRocks | MapFieldForest);

	mf->Flags |= Map.Tileset.FlagsTable[tile];

	UI.Minimap.UpdateSeenXY(pos);
	UI.Minimap.UpdateXY(pos);

	EditorTileChanged2(pos, d);
}

/**
**  Update surroundings for tile changes.
**
**  @param pos  Map X tile position of change.
**  @param y  Map Y tile position of change.
**  @param d  Fix direction flag 8 up, 4 down, 2 left, 1 right.
*/
static void EditorTileChanged2(const Vec2i &pos, int d)
{
	unsigned quad = QuadFromTile(pos);

	//
	// Change the surrounding
	//

	//
	// Special case 1) Walls.
	//
	CMapField *mf = Map.Field(pos);
	if (mf->Flags & MapFieldWall) {
		Map.SetWall(pos, mf->Flags & MapFieldHuman);
		return;
	}

	//
	// How this works:
	//  first get the quad of the neighbouring tile, then
	//  check if the margin matches. otherwise, call
	//  EditorChangeTile again.
	//
	if (d & DIR_UP && pos.y) {
		const Vec2i offset = {0, -1};
		//
		// Insert into the bottom the new tile.
		//
		unsigned q2 = QuadFromTile(pos + offset);
		unsigned u = (q2 & TH_QUAD_M) | ((quad >> 16) & BH_QUAD_M);
		if (u != q2) {
			int tile = TileFromQuad(u & BH_QUAD_M, u);
			EditorChangeTile(pos + offset, tile, d & ~DIR_DOWN);
		}
	}
	if (d & DIR_DOWN && pos.y < Map.Info.MapHeight - 1) {
		const Vec2i offset = {0, 1};
		//
		// Insert into the top the new tile.
		//
		unsigned q2 = QuadFromTile(pos + offset);
		unsigned u = (q2 & BH_QUAD_M) | ((quad << 16) & TH_QUAD_M);
		if (u != q2) {
			int tile = TileFromQuad(u & TH_QUAD_M, u);
			EditorChangeTile(pos + offset, tile, d & ~DIR_UP);
		}
	}
	if (d & DIR_LEFT && pos.x) {
		const Vec2i offset = { -1, 0};
		//
		// Insert into the left the new tile.
		//
		unsigned q2 = QuadFromTile(pos + offset);
		unsigned u = (q2 & LH_QUAD_M) | ((quad >> 8) & RH_QUAD_M);
		if (u != q2) {
			int tile = TileFromQuad(u & RH_QUAD_M, u);
			EditorChangeTile(pos + offset, tile, d & ~DIR_RIGHT);
		}
	}
	if (d & DIR_RIGHT && pos.x < Map.Info.MapWidth - 1) {
		const Vec2i offset = {1, 0};
		//
		// Insert into the right the new tile.
		//
		unsigned q2 = QuadFromTile(pos + offset);
		unsigned u = (q2 & RH_QUAD_M) | ((quad << 8) & LH_QUAD_M);
		if (u != q2) {
			int tile = TileFromQuad(u & LH_QUAD_M, u);
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
	EditorTileChanged2(pos, 0xF);
}

/**
**  Make random map
**  FIXME: vladi: we should have parameters control here...
*/

/**
**  TileFill
**
**  @param pos   map tile coordinate for area center.
**  @param y     Y map tile coordinate for area center.
**  @param tile  Tile type to edit.
**  @param size  Size of surrounding rectangle.
**
**  TileFill(centerx, centery, tile_type_water, map_width)
**  will fill map with water...
*/
static void TileFill(const Vec2i &pos, int tile, int size)
{
	const Vec2i diag = {size / 2, size / 2};
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

#define WATER_TILE  0x10
#define COAST_TILE  0x30
#define GRASS_TILE  0x50
#define WOOD_TILE   0x70
#define ROCK_TILE   0x80

/**
**  Randomize tiles and fill in map
**
**  @param tile      tile number to use
**  @param count     number of times to apply randomization
**  @param max_size  maximum size of the fill rectangle
*/
static void EditorRandomizeTile(int tile, int count, int max_size)
{
	const Vec2i mpos = { Map.Info.MapWidth - 1, Map.Info.MapHeight - 1};

	for (int i = 0; i < count; ++i) {
		const Vec2i rpos = {rand() % ((1 + mpos.x) / 2), rand() % ((1 + mpos.y) / 2)};
		const Vec2i mirror = mpos - rpos;
		const Vec2i mirrorh = {rpos.x, mirror.y};
		const Vec2i mirrorv = {mirror.x, rpos.y};
		const int rz = rand() % max_size + 1;

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
static void EditorRandomizeUnit(const char *unit_type, int count, int value)
{
	const Vec2i mpos = {Map.Info.MapWidth, Map.Info.MapHeight};
	CUnitType *typeptr = UnitTypeByIdent(unit_type);

	if (!typeptr) { // Error
		return;
	}
	CUnitType &type = *typeptr;
	const Vec2i tpos = {type.TileWidth, type.TileHeight};

	for (int i = 0; i < count; ++i) {
		const Vec2i rpos = {rand() % (mpos.x / 2 - tpos.x + 1), rand() % (mpos.y / 2 - tpos.y + 1)};
		const Vec2i mirror = {mpos.x - rpos.x - 1, mpos.y - rpos.y - 1};
		const Vec2i mirrorh = {rpos.x, mirror.y};
		const Vec2i mirrorv = {mirror.x, rpos.y};
		const Vec2i tmirror = {mpos.x - rpos.x - tpos.x, mpos.y - rpos.y - tpos.y};
		const Vec2i tmirrorh = {rpos.x, tmirror.y};
		const Vec2i tmirrorv = {tmirror.x, rpos.y};
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
		if (unit != NoUnitP) {
			DebugPrint("Unable to allocate Unit");
		} else {
			unit->ResourcesHeld = value;
		}

		unit = MakeUnitAndPlace(tmirrorh, type, &Players[PlayerNumNeutral]);
		if (unit != NoUnitP) {
			DebugPrint("Unable to allocate Unit");
		} else {
			unit->ResourcesHeld = value;
		}

		unit = MakeUnitAndPlace(tmirrorv, type, &Players[PlayerNumNeutral]);
		if (unit != NoUnitP) {
			DebugPrint("Unable to allocate Unit");
		} else {
			unit->ResourcesHeld = value;
		}

		unit = MakeUnitAndPlace(tmirror, type, &Players[PlayerNumNeutral]);
		if (unit != NoUnitP) {
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
	while (NumUnits != 0) {
		CUnit &unit = *Units[0];

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
	int mz;

	mz = Map.Info.MapWidth > Map.Info.MapHeight ? Map.Info.MapWidth : Map.Info.MapHeight;

	// make water-base
	const Vec2i zeros = {0, 0};
	TileFill(zeros, WATER_TILE, mz * 3);
	// remove all units
	EditorDestroyAllUnits();

	EditorRandomizeTile(COAST_TILE, 10, 16);
	EditorRandomizeTile(GRASS_TILE, 20, 16);
	EditorRandomizeTile(WOOD_TILE,  60,  4);
	EditorRandomizeTile(ROCK_TILE,  30,  2);

	EditorRandomizeUnit("unit-gold-mine",  5,  50000);
}

//@}
