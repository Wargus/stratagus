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
/**@name script_tileset.cpp - The tileset ccl functions. */
//
//      (c) Copyright 2000-2007 by Lutz Sammer, Francois Beerten and Jimmy Salmon
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

#include "tileset.h"

#include "script.h"

/*----------------------------------------------------------------------------
--  Functions
----------------------------------------------------------------------------*/

static bool ModifyFlag(const char *flagName, unsigned int *flag)
{
	const struct {
		const char *name;
		unsigned int flag;
	} flags[] = {
		{"water", MapFieldWaterAllowed},
		{"land", MapFieldLandAllowed},
		{"coast", MapFieldCoastAllowed},
		{"no-building", MapFieldNoBuilding},
		{"unpassable", MapFieldUnpassable},
		{"wall", MapFieldWall},
		{"rock", MapFieldRocks},
		{"forest", MapFieldForest},
		{"land-unit", MapFieldLandUnit},
		{"air-unit", MapFieldAirUnit},
		{"sea-unit", MapFieldSeaUnit},
		{"building", MapFieldBuilding},
		{"human", MapFieldHuman}
	};

	for (unsigned int i = 0; i != sizeof(flags) / sizeof(*flags); ++i) {
		if (!strcmp(flagName, flags[i].name)) {
			*flag |= flags[i].flag;
			return true;
		}
	}

	const struct {
		const char *name;
		unsigned int speed;
	} speeds[] = {
		{"fastest", 0},
		{"fast", 1},
		{"slow", 4},
		{"slower", 5},
		{"slowest", 7},
	};

	for (unsigned int i = 0; i != sizeof(speeds) / sizeof(*speeds); ++i) {
		if (!strcmp(flagName, speeds[i].name)) {
			*flag = (*flag & ~MapFieldSpeedMask) | speeds[i].speed;
			return true;
		}
	}
	return false;
}

/**
**  Parse the flag section of a tile definition.
**
**  @param l     Lua state.
**  @param back  pointer for the flags (return).
**  @param j     pointer for the location in the array. in and out
**
*/
void ParseTilesetTileFlags(lua_State *l, int *back, int *j)
{
	unsigned int flags = 3;

	//  Parse the list: flags of the slot
	while (1) {
		lua_rawgeti(l, -1, *j + 1);
		if (!lua_isstring(l, -1)) {
			lua_pop(l, 1);
			break;
		}
		++(*j);
		const char *value = LuaToString(l, -1);
		lua_pop(l, 1);

		//  Flags are only needed for the editor
		if (ModifyFlag(value, &flags) == false) {
			LuaError(l, "solid: unsupported tag: %s" _C_ value);
		}
	}
	*back = flags;
}

/**
**  Parse the special slot part of a tileset definition
**
**  @param l        Lua state.
*/
void CTileset::parseSpecial(lua_State *l)
{
	if (!lua_istable(l, -1)) {
		LuaError(l, "incorrect argument");
	}
	const int args = lua_rawlen(l, -1);

	for (int j = 0; j < args; ++j) {
		const char *value = LuaToString(l, -1, j + 1);

		if (!strcmp(value, "top-one-tree")) {
			++j;
			topOneTreeTile = LuaToNumber(l, -1, j + 1);
		} else if (!strcmp(value, "mid-one-tree")) {
			++j;
			midOneTreeTile = LuaToNumber(l, -1, j + 1);
		} else if (!strcmp(value, "bot-one-tree")) {
			++j;
			botOneTreeTile = LuaToNumber(l, -1, j + 1);
		} else if (!strcmp(value, "removed-tree")) {
			++j;
			removedTreeTile = LuaToNumber(l, -1, j + 1);
		} else if (!strcmp(value, "growing-tree")) {
			++j;
			// keep for retro compatibility.
			// TODO : remove when game data are updated.
		} else if (!strcmp(value, "top-one-rock")) {
			++j;
			topOneRockTile = LuaToNumber(l, -1, j + 1);
		} else if (!strcmp(value, "mid-one-rock")) {
			++j;
			midOneRockTile = LuaToNumber(l, -1, j + 1);
		} else if (!strcmp(value, "bot-one-rock")) {
			++j;
			botOneRockTile = LuaToNumber(l, -1, j + 1);
		} else if (!strcmp(value, "removed-rock")) {
			++j;
			removedRockTile = LuaToNumber(l, -1, j + 1);
		} else {
			LuaError(l, "special: unsupported tag: %s" _C_ value);
		}
	}
}

/**
**  Parse the solid slot part of a tileset definition
**
**  @param l        Lua state.
*/
void CTileset::parseSolid(lua_State *l)
{
	const int index = tiles.size();

	this->tiles.resize(index + 16);
	if (!lua_istable(l, -1)) {
		LuaError(l, "incorrect argument");
	}

	int j = 0;
	const int basic_name = getOrAddSolidTileIndexByName(LuaToString(l, -1, j + 1));
	++j;

	int f = 0;
	ParseTilesetTileFlags(l, &f, &j);
	//  Vector: the tiles.
	lua_rawgeti(l, -1, j + 1);
	if (!lua_istable(l, -1)) {
		LuaError(l, "incorrect argument");
	}
	const int len = lua_rawlen(l, -1);

	j = 0;
	for (int i = 0; i < len; ++i, ++j) {
		lua_rawgeti(l, -1, i + 1);
		if (lua_istable(l, -1)) {
			int k = 0;
			int tile_flag = 0;
			ParseTilesetTileFlags(l, &tile_flag, &k);
			--j;
			lua_pop(l, 1);
			tiles[index + j].flag = tile_flag;
			continue;
		}
		const int pud = LuaToNumber(l, -1);
		lua_pop(l, 1);

		// ugly hack for sc tilesets, remove when fixed
		if (j > 15) {
			this->tiles.resize(index + j);
		}
		CTile &tile = tiles[index + j];

		tile.tile = pud;
		tile.flag = f;
		tile.tileinfo.BaseTerrain = basic_name;
		tile.tileinfo.MixTerrain = 0;
	}
	lua_pop(l, 1);
}

/**
**  Parse the mixed slot part of a tileset definition
**
**  @param l        Lua state.
*/
void CTileset::parseMixed(lua_State *l)
{
	int index = tiles.size();
	tiles.resize(index + 256);

	if (!lua_istable(l, -1)) {
		LuaError(l, "incorrect argument");
	}
	int j = 0;
	const int args = lua_rawlen(l, -1);
	const int basic_name = getOrAddSolidTileIndexByName(LuaToString(l, -1, j + 1));
	++j;
	const int mixed_name = getOrAddSolidTileIndexByName(LuaToString(l, -1, j + 1));
	++j;

	int f = 0;
	ParseTilesetTileFlags(l, &f, &j);

	for (; j < args; ++j) {
		lua_rawgeti(l, -1, j + 1);
		if (!lua_istable(l, -1)) {
			LuaError(l, "incorrect argument");
		}
		//  Vector: the tiles.
		const int len = lua_rawlen(l, -1);
		for (int i = 0; i < len; ++i) {
			const int pud = LuaToNumber(l, -1, i + 1);
			CTile &tile = tiles[index + i];

			tile.tile = pud;
			tile.flag = f;
			tile.tileinfo.BaseTerrain = basic_name;
			tile.tileinfo.MixTerrain = mixed_name;
		}
		index += 16;
		lua_pop(l, 1);
	}
}

/**
**  Parse the slot part of a tileset definition
**
**  @param l        Lua state.
**  @param t        FIXME: docu
*/
void CTileset::parseSlots(lua_State *l, int t)
{
	tiles.clear();

	//  Parse the list: (still everything could be changed!)
	const int args = lua_rawlen(l, t);
	for (int j = 0; j < args; ++j) {
		const char *value = LuaToString(l, t, j + 1);
		++j;

		if (!strcmp(value, "special")) {
			lua_rawgeti(l, t, j + 1);
			parseSpecial(l);
			lua_pop(l, 1);
		} else if (!strcmp(value, "solid")) {
			lua_rawgeti(l, t, j + 1);
			parseSolid(l);
			lua_pop(l, 1);
		} else if (!strcmp(value, "mixed")) {
			lua_rawgeti(l, t, j + 1);
			parseMixed(l);
			lua_pop(l, 1);
		} else {
			LuaError(l, "slots: unsupported tag: %s" _C_ value);
		}
	}
}

void CTileset::parse(lua_State *l)
{
	clear();

	this->pixelTileSize.x = 32;
	this->pixelTileSize.y = 32;

	const int args = lua_gettop(l);
	for (int j = 1; j < args; ++j) {
		const char *value = LuaToString(l, j);
		++j;

		if (!strcmp(value, "name")) {
			this->Name = LuaToString(l, j);
		} else if (!strcmp(value, "image")) {
			this->ImageFile = LuaToString(l, j);
		} else if (!strcmp(value, "size")) {
			CclGetPos(l, &this->pixelTileSize.x, &this->pixelTileSize.x, j);
		} else if (!strcmp(value, "slots")) {
			if (!lua_istable(l, j)) {
				LuaError(l, "incorrect argument");
			}
			parseSlots(l, j);
		} else {
			LuaError(l, "Unsupported tag: %s" _C_ value);
		}
	}
}

void CTileset::buildTable(lua_State *l)
{
	//  Calculate number of tiles in graphic tile
	const int n = tiles.size();

	mixedLookupTable.clear();
	mixedLookupTable.resize(n, 0);
	//  Build the TileTypeTable
	TileTypeTable.resize(n, 0);

	for (int i = 0; i < n; ++i) {
		const int tile = tiles[i].tile;
		if (tile == 0) {
			continue;
		}
		const unsigned flag = tiles[i].flag;
		if (flag & MapFieldWaterAllowed) {
			TileTypeTable[tile] = TileTypeWater;
		} else if (flag & MapFieldCoastAllowed) {
			TileTypeTable[tile] = TileTypeCoast;
		} else if (flag & MapFieldWall) {
			if (flag & MapFieldHuman) {
				TileTypeTable[tile] = TileTypeHumanWall;
			} else {
				TileTypeTable[tile] = TileTypeOrcWall;
			}
		} else if (flag & MapFieldRocks) {
			TileTypeTable[tile] = TileTypeRock;
		} else if (flag & MapFieldForest) {
			TileTypeTable[tile] = TileTypeWood;
		}
	}
	//  mark the special tiles
	if (topOneTreeTile) {
		TileTypeTable[topOneTreeTile] = TileTypeWood;
	}
	if (midOneTreeTile) {
		TileTypeTable[midOneTreeTile] = TileTypeWood;
	}
	if (botOneTreeTile) {
		TileTypeTable[botOneTreeTile] = TileTypeWood;
	}
	if (topOneRockTile) {
		TileTypeTable[topOneRockTile] = TileTypeRock;
	}
	if (midOneRockTile) {
		TileTypeTable[midOneRockTile] = TileTypeRock;
	}
	if (botOneRockTile) {
		TileTypeTable[botOneRockTile] = TileTypeRock;
	}

	//  Build wood removement table.
	int solid = 0;
	int mixed = 0;
	for (int i = 0; i < n;) {
		const CTile &tile = tiles[i];
		const CTileInfo &tileinfo = tile.tileinfo;
		if (tileinfo.BaseTerrain && tileinfo.MixTerrain) {
			if (tile.flag & MapFieldForest) {
				mixed = i;
			}
			i += 256;
		} else {
			if (tileinfo.BaseTerrain != 0 && tileinfo.MixTerrain == 0) {
				if (tile.flag & MapFieldForest) {
					solid = i;
				}
			}
			i += 16;
		}
	}
	woodTable[ 0] = -1;
	woodTable[ 1] = tiles[mixed + 0x30].tile;
	woodTable[ 2] = tiles[mixed + 0x70].tile;
	woodTable[ 3] = tiles[mixed + 0xB0].tile;
	woodTable[ 4] = tiles[mixed + 0x10].tile;
	woodTable[ 5] = tiles[mixed + 0x50].tile;
	woodTable[ 6] = tiles[mixed + 0x90].tile;
	woodTable[ 7] = tiles[mixed + 0xD0].tile;
	woodTable[ 8] = tiles[mixed + 0x00].tile;
	woodTable[ 9] = tiles[mixed + 0x40].tile;
	woodTable[10] = tiles[mixed + 0x80].tile;
	woodTable[11] = tiles[mixed + 0xC0].tile;
	woodTable[12] = tiles[mixed + 0x20].tile;
	woodTable[13] = tiles[mixed + 0x60].tile;
	woodTable[14] = tiles[mixed + 0xA0].tile;
	woodTable[15] = tiles[solid].tile;
	woodTable[16] = -1;
	woodTable[17] = botOneTreeTile;
	woodTable[18] = topOneTreeTile;
	woodTable[19] = midOneTreeTile;

	//Mark which corners of each tile has tree in it.
	//All corners for solid tiles. (Same for rocks)
	//1 Bottom Left
	//2 Bottom Right
	//4 Top Right
	//8 Top Left
	//16 Bottom Tree Tile
	//32 Top Tree Tile
	for (int i = solid; i < solid + 16; ++i) {
		mixedLookupTable[tiles[i].tile] = 15;
	}
	for (int i = mixed; i < mixed + 256; ++i) {
		int check = (int)((i - mixed) / 16);

		switch (check) {
			case 0: mixedLookupTable[tiles[i].tile] = 8; break;
			case 1: mixedLookupTable[tiles[i].tile] = 4; break;
			case 2: mixedLookupTable[tiles[i].tile] = 8 + 4; break;
			case 3: mixedLookupTable[tiles[i].tile] = 1; break;
			case 4: mixedLookupTable[tiles[i].tile] = 8 + 1; break;
			case 5: mixedLookupTable[tiles[i].tile] = 4 + 1; break;
			case 6: mixedLookupTable[tiles[i].tile] = 8 + 4 + 1; break;
			case 7: mixedLookupTable[tiles[i].tile] = 2; break;
			case 8: mixedLookupTable[tiles[i].tile] = 8 + 2; break;
			case 9: mixedLookupTable[tiles[i].tile] = 4 + 2; break;
			case 10: mixedLookupTable[tiles[i].tile] = 8 + 4 + 2; break;
			case 11: mixedLookupTable[tiles[i].tile] = 2 + 1; break;
			case 12: mixedLookupTable[tiles[i].tile] = 8 + 2 + 1; break;
			case 13:  mixedLookupTable[tiles[i].tile] = 4 + 2 + 1; break;
			default: mixedLookupTable[tiles[i].tile] = 0; break;
		}
	}
	//16 Bottom Tree Special
	//32 Top Tree Special
	//64 Mid tree special - differentiate with mixed tiles.
	mixedLookupTable[botOneTreeTile] = 12 + 16;
	mixedLookupTable[topOneTreeTile] = 3 + 32;
	mixedLookupTable[midOneTreeTile] = 15 + 48;

	//  Build rock removement table.
	mixed = 0;
	solid = 0;
	for (int i = 0; i < n;) {
		const CTile &tile = tiles[i];
		const CTileInfo &tileinfo = tile.tileinfo;
		if (tileinfo.BaseTerrain && tileinfo.MixTerrain) {
			if (tile.flag & MapFieldRocks) {
				mixed = i;
			}
			i += 256;
		} else {
			if (tileinfo.BaseTerrain != 0 && tileinfo.MixTerrain == 0) {
				if (tile.flag & MapFieldRocks) {
					solid = i;
				}
			}
			i += 16;
		}
	}

	//Mark which corners of each tile has rock in it.
	//All corners for solid tiles.
	//1 Bottom Left
	//2 Bottom Right
	//4 Top Right
	//8 Top Left
	for (int i = solid; i < solid + 16; ++i) {
		mixedLookupTable[tiles[i].tile] = 15;
	}
	for (int i = mixed; i < mixed + 256; ++i) {
		int check = (int)((i - mixed) / 16);
		switch (check) {
			case 0: mixedLookupTable[tiles[i].tile] = 8; break;
			case 1: mixedLookupTable[tiles[i].tile] = 4; break;
			case 2: mixedLookupTable[tiles[i].tile] = 8 + 4; break;
			case 3: mixedLookupTable[tiles[i].tile] = 1; break;
			case 4: mixedLookupTable[tiles[i].tile] = 8 + 1; break;
			case 5: mixedLookupTable[tiles[i].tile] = 4 + 1; break;
			case 6: mixedLookupTable[tiles[i].tile] = 8 + 4 + 1; break;
			case 7: mixedLookupTable[tiles[i].tile] = 2; break;
			case 8: mixedLookupTable[tiles[i].tile] = 8 + 2; break;
			case 9: mixedLookupTable[tiles[i].tile] = 4 + 2; break;
			case 10: mixedLookupTable[tiles[i].tile] = 8 + 4 + 2; break;
			case 11: mixedLookupTable[tiles[i].tile] = 2 + 1; break;
			case 12: mixedLookupTable[tiles[i].tile] = 8 + 2 + 1; break;
			case 13: mixedLookupTable[tiles[i].tile] = 4 + 2 + 1; break;
			default: mixedLookupTable[tiles[i].tile] = 0; break;
		}
	}

	mixedLookupTable[botOneRockTile] = 12 + 16;
	mixedLookupTable[topOneRockTile] = 3 + 32;
	mixedLookupTable[midOneRockTile] = 15 + 48;

	rockTable[ 0] = -1;
	rockTable[ 1] = tiles[mixed + 0x30].tile;
	rockTable[ 2] = tiles[mixed + 0x70].tile;
	rockTable[ 3] = tiles[mixed + 0xB0].tile;
	rockTable[ 4] = tiles[mixed + 0x10].tile;
	rockTable[ 5] = tiles[mixed + 0x50].tile;
	rockTable[ 6] = tiles[mixed + 0x90].tile;
	rockTable[ 7] = tiles[mixed + 0xD0].tile;
	rockTable[ 8] = tiles[mixed + 0x00].tile;
	rockTable[ 9] = tiles[mixed + 0x40].tile;
	rockTable[10] = tiles[mixed + 0x80].tile;
	rockTable[11] = tiles[mixed + 0xC0].tile;
	rockTable[12] = tiles[mixed + 0x20].tile;
	rockTable[13] = tiles[mixed + 0x60].tile;
	rockTable[14] = tiles[mixed + 0xA0].tile;
	rockTable[15] = tiles[solid].tile;
	rockTable[16] = -1;
	rockTable[17] = botOneRockTile;
	rockTable[18] = topOneRockTile;
	rockTable[19] = midOneRockTile;

	buildWallReplacementTable();
}

void CTileset::buildWallReplacementTable()
{
	// FIXME: Build wall replacement tables
	humanWallTable[ 0] = 0x090;
	humanWallTable[ 1] = 0x830;
	humanWallTable[ 2] = 0x810;
	humanWallTable[ 3] = 0x850;
	humanWallTable[ 4] = 0x800;
	humanWallTable[ 5] = 0x840;
	humanWallTable[ 6] = 0x820;
	humanWallTable[ 7] = 0x860;
	humanWallTable[ 8] = 0x870;
	humanWallTable[ 9] = 0x8B0;
	humanWallTable[10] = 0x890;
	humanWallTable[11] = 0x8D0;
	humanWallTable[12] = 0x880;
	humanWallTable[13] = 0x8C0;
	humanWallTable[14] = 0x8A0;
	humanWallTable[15] = 0x0B0;

	orcWallTable[ 0] = 0x0A0;
	orcWallTable[ 1] = 0x930;
	orcWallTable[ 2] = 0x910;
	orcWallTable[ 3] = 0x950;
	orcWallTable[ 4] = 0x900;
	orcWallTable[ 5] = 0x940;
	orcWallTable[ 6] = 0x920;
	orcWallTable[ 7] = 0x960;
	orcWallTable[ 8] = 0x970;
	orcWallTable[ 9] = 0x9B0;
	orcWallTable[10] = 0x990;
	orcWallTable[11] = 0x9D0;
	orcWallTable[12] = 0x980;
	orcWallTable[13] = 0x9C0;
	orcWallTable[14] = 0x9A0;
	orcWallTable[15] = 0x0C0;

	// Set destroyed walls to TileTypeUnknown
	for (int i = 0; i < 16; ++i) {
		int n = 0;
		unsigned int tileIndex = humanWallTable[i];
		while (tiles[tileIndex].tile) { // Skip good tiles
			++tileIndex;
			++n;
		}
		while (!tiles[tileIndex].tile) { // Skip separator
			++tileIndex;
			++n;
		}
		while (tiles[tileIndex].tile) { // Skip good tiles
			++tileIndex;
			++n;
		}
		while (!tiles[tileIndex].tile) { // Skip separator
			++tileIndex;
			++n;
		}
		while (n < 16 && tiles[tileIndex].tile) {
			TileTypeTable[tiles[tileIndex].tile] = TileTypeUnknown;
			++tileIndex;
			++n;
		}
	}
}

//@}
