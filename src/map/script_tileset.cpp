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

#include "map.h"
#include "script.h"

/*----------------------------------------------------------------------------
--  Functions
----------------------------------------------------------------------------*/

/**
**  Extend tables of the tileset.
**
**  @param tileset   Tileset to be extended.
**  @param oldtiles  Number of old tiles.
**  @param newtiles  Number of new tiles.
*/
static void ExtendTilesetTables(CTileset *tileset, unsigned int oldtiles, unsigned int newtiles)
{
	tileset->Table.resize(oldtiles + newtiles);
	tileset->FlagsTable.resize(oldtiles + newtiles);
	tileset->Tiles.resize(oldtiles + newtiles);
}

/**
**  Parse the name field in tileset definition.
**
**  @param l        Lua state.
**  @param tileset  Tileset currently parsed.
*/
static int TilesetParseName(lua_State *l, CTileset *tileset)
{
	const char *ident = LuaToString(l, -1);
	for (size_t i = 0; i != tileset->SolidTerrainTypes.size(); ++i) {
		if (tileset->SolidTerrainTypes[i].TerrainName == ident) {
			return i;
		}
	}
	// Can't find it, then we add another solid terrain type.
	SolidTerrainInfo s;
	s.TerrainName = ident;
	tileset->SolidTerrainTypes.push_back(s);
	return tileset->SolidTerrainTypes.size() - 1;
}

/**
**  Parse the flag section of a tile definition.
**
**  @param l     Lua state.
**  @param back  pointer for the flags (return).
**  @param j     pointer for the location in the array. in and out
**
*/
static void ParseTilesetTileFlags(lua_State *l, int *back, int *j)
{
	int flags = 3;

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
		if (!strcmp(value, "water")) {
			flags |= MapFieldWaterAllowed;
		} else if (!strcmp(value, "land")) {
			flags |= MapFieldLandAllowed;
		} else if (!strcmp(value, "coast")) {
			flags |= MapFieldCoastAllowed;
		} else if (!strcmp(value, "no-building")) {
			flags |= MapFieldNoBuilding;
		} else if (!strcmp(value, "unpassable")) {
			flags |= MapFieldUnpassable;
		} else if (!strcmp(value, "wall")) {
			flags |= MapFieldWall;
		} else if (!strcmp(value, "rock")) {
			flags |= MapFieldRocks;
		} else if (!strcmp(value, "forest")) {
			flags |= MapFieldForest;
		} else if (!strcmp(value, "land-unit")) {
			flags |= MapFieldLandUnit;
		} else if (!strcmp(value, "air-unit")) {
			flags |= MapFieldAirUnit;
		} else if (!strcmp(value, "sea-unit")) {
			flags |= MapFieldSeaUnit;
		} else if (!strcmp(value, "building")) {
			flags |= MapFieldBuilding;
		} else if (!strcmp(value, "human")) {
			flags |= MapFieldHuman;
		} else if (!strcmp(value, "fastest")) {
			flags = (flags & ~MapFieldSpeedMask);
		} else if (!strcmp(value, "fast")) {
			flags = (flags & ~MapFieldSpeedMask) | 1;
		} else if (!strcmp(value, "slow")) {
			flags = (flags & ~MapFieldSpeedMask) | 4;
		} else if (!strcmp(value, "slower")) {
			flags = (flags & ~MapFieldSpeedMask) | 5;
		} else if (!strcmp(value, "slowest")) {
			flags = (flags & ~MapFieldSpeedMask) | 7;
		} else {
			LuaError(l, "solid: unsupported tag: %s" _C_ value);
		}
	}
	*back = flags;
}

/**
**  Parse the special slot part of a tileset definition
**
**  @param l        Lua state.
**  @param tileset  Tileset to be filled.
*/
static void DefineTilesetParseSpecial(lua_State *l, CTileset *tileset)
{
	if (!lua_istable(l, -1)) {
		LuaError(l, "incorrect argument");
	}
	const int args = lua_rawlen(l, -1);

	for (int j = 0; j < args; ++j) {
		lua_rawgeti(l, -1, j + 1);
		const char *value = LuaToString(l, -1);
		lua_pop(l, 1);

		if (!strcmp(value, "top-one-tree")) {
			++j;
			lua_rawgeti(l, -1, j + 1);
			tileset->TopOneTree = LuaToNumber(l, -1);
			lua_pop(l, 1);
		} else if (!strcmp(value, "mid-one-tree")) {
			++j;
			lua_rawgeti(l, -1, j + 1);
			tileset->MidOneTree = LuaToNumber(l, -1);
			lua_pop(l, 1);
		} else if (!strcmp(value, "bot-one-tree")) {
			++j;
			lua_rawgeti(l, -1, j + 1);
			tileset->BotOneTree = LuaToNumber(l, -1);
			lua_pop(l, 1);
		} else if (!strcmp(value, "removed-tree")) {
			++j;
			lua_rawgeti(l, -1, j + 1);
			tileset->RemovedTree = LuaToNumber(l, -1);
			lua_pop(l, 1);
		} else if (!strcmp(value, "growing-tree")) {
			++j;
			lua_rawgeti(l, -1, j + 1);
			if (!lua_istable(l, -1)) {
				LuaError(l, "incorrect argument");
			}
			if (lua_rawlen(l, -1) != 2) {
				LuaError(l, "growing-tree: Wrong table length");
			}
			for (int i = 0; i < 2; ++i) {
				lua_rawgeti(l, -1, i + 1);
				tileset->GrowingTree[i] = LuaToNumber(l, -1);
				lua_pop(l, 1);
			}
			lua_pop(l, 1);
		} else if (!strcmp(value, "top-one-rock")) {
			++j;
			lua_rawgeti(l, -1, j + 1);
			tileset->TopOneRock = LuaToNumber(l, -1);
			lua_pop(l, 1);
		} else if (!strcmp(value, "mid-one-rock")) {
			++j;
			lua_rawgeti(l, -1, j + 1);
			tileset->MidOneRock = LuaToNumber(l, -1);
			lua_pop(l, 1);
		} else if (!strcmp(value, "bot-one-rock")) {
			++j;
			lua_rawgeti(l, -1, j + 1);
			tileset->BotOneRock = LuaToNumber(l, -1);
			lua_pop(l, 1);
		} else if (!strcmp(value, "removed-rock")) {
			++j;
			lua_rawgeti(l, -1, j + 1);
			tileset->RemovedRock = LuaToNumber(l, -1);
			lua_pop(l, 1);
		} else {
			LuaError(l, "special: unsupported tag: %s" _C_ value);
		}
	}
}

/**
**  Parse the solid slot part of a tileset definition
**
**  @param l        Lua state.
**  @param tileset  Tileset to be filled.
**  @param index    Current table index.
*/
static int DefineTilesetParseSolid(lua_State *l, CTileset *tileset, int index)
{
	int f = 0;
	int j = 0;

	ExtendTilesetTables(tileset, index, 16);

	if (!lua_istable(l, -1)) {
		LuaError(l, "incorrect argument");
	}

	lua_rawgeti(l, -1, j + 1);
	++j;
	const int basic_name = TilesetParseName(l, tileset);
	lua_pop(l, 1);

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
			tileset->FlagsTable[index + j] = tile_flag;
			continue;
		}

		const int pud = LuaToNumber(l, -1);
		lua_pop(l, 1);

		// ugly hack for sc tilesets, remove when fixed
		if (j > 15) {
			ExtendTilesetTables(tileset, index, j);
		}

		tileset->Table[index + j] = pud;
		tileset->FlagsTable[index + j] = f;
		tileset->Tiles[index + j].BaseTerrain = basic_name;
		tileset->Tiles[index + j].MixTerrain = 0;
	}
	lua_pop(l, 1);

	for (int i = j; i < 16; ++i) {
		tileset->Table[index + i] = 0;
		tileset->FlagsTable[index + i] = 0;
		tileset->Tiles[index + i].BaseTerrain = 0;
		tileset->Tiles[index + i].MixTerrain = 0;
	}

	if (j < 16) {
		return index + 16;
	}
	return index + j;
}

/**
**  Parse the mixed slot part of a tileset definition
**
**  @param l        Lua state.
**  @param tileset  Tileset to be filled.
**  @param index    Current table index.
*/
static int DefineTilesetParseMixed(lua_State *l, CTileset *tileset, int index)
{
	int f = 0;
	const int new_index = index + 256;
	ExtendTilesetTables(tileset, index, 256);

	if (!lua_istable(l, -1)) {
		LuaError(l, "incorrect argument");
	}
	int j = 0;
	const int args = lua_rawlen(l, -1);
	lua_rawgeti(l, -1, j + 1);
	++j;
	const int basic_name = TilesetParseName(l, tileset);
	lua_pop(l, 1);
	lua_rawgeti(l, -1, j + 1);
	++j;
	const int mixed_name = TilesetParseName(l, tileset);
	lua_pop(l, 1);

	ParseTilesetTileFlags(l, &f, &j);

	for (; j < args; ++j) {
		lua_rawgeti(l, -1, j + 1);
		if (!lua_istable(l, -1)) {
			LuaError(l, "incorrect argument");
		}
		//  Vector: the tiles.
		const int len = lua_rawlen(l, -1);
		for (int i = 0; i < len; ++i) {
			lua_rawgeti(l, -1, i + 1);
			const int pud = LuaToNumber(l, -1);
			lua_pop(l, 1);
			tileset->Table[index + i] = pud;
			tileset->FlagsTable[index + i] = f;
			tileset->Tiles[index + i].BaseTerrain = basic_name;
			tileset->Tiles[index + i].MixTerrain = mixed_name;
		}
		// Fill missing slots
		for (int i = len; i < 16; ++i) {
			tileset->Table[index + i] = 0;
			tileset->FlagsTable[index + i] = 0;
			tileset->Tiles[index + i].BaseTerrain = 0;
			tileset->Tiles[index + i].MixTerrain = 0;
		}
		index += 16;
		lua_pop(l, 1);
	}
	while (index < new_index) {
		tileset->Table[index] = 0;
		tileset->FlagsTable[index] = 0;
		tileset->Tiles[index].BaseTerrain = 0;
		tileset->Tiles[index].MixTerrain = 0;
		++index;
	}
	return new_index;
}

/**
**  Parse the slot part of a tileset definition
**
**  @param l        Lua state.
**  @param tileset  Tileset to be filled.
**  @param t        FIXME: docu
*/
static void DefineTilesetParseSlot(lua_State *l, CTileset *tileset, int t)
{
	int index = 0;

	tileset->Table.resize(16);
	tileset->FlagsTable.resize(16);
	tileset->Tiles.resize(16);
	SolidTerrainInfo solidTerrainInfo;
	solidTerrainInfo.TerrainName = "unused";
	tileset->SolidTerrainTypes.push_back(solidTerrainInfo);

	//  Parse the list: (still everything could be changed!)
	const int args = lua_rawlen(l, t);
	for (int j = 0; j < args; ++j) {
		lua_rawgeti(l, t, j + 1);
		const char *value = LuaToString(l, -1);
		lua_pop(l, 1);
		++j;

		if (!strcmp(value, "special")) {
			lua_rawgeti(l, t, j + 1);
			DefineTilesetParseSpecial(l, tileset);
			lua_pop(l, 1);
		} else if (!strcmp(value, "solid")) {
			lua_rawgeti(l, t, j + 1);
			index = DefineTilesetParseSolid(l, tileset, index);
			lua_pop(l, 1);
		} else if (!strcmp(value, "mixed")) {
			lua_rawgeti(l, t, j + 1);
			index = DefineTilesetParseMixed(l, tileset, index);
			lua_pop(l, 1);
		} else {
			LuaError(l, "slots: unsupported tag: %s" _C_ value);
		}
	}
	tileset->NumTiles = index;
}

/**
**  Define tileset
**
**  @param l  Lua state.
*/
static int CclDefineTileset(lua_State *l)
{
	Map.Tileset->Clear();

	Map.Tileset->PixelTileSize.x = 32;
	Map.Tileset->PixelTileSize.y = 32;

	const int args = lua_gettop(l);
	for (int j = 1; j < args; ++j) {
		const char *value = LuaToString(l, j);
		++j;

		if (!strcmp(value, "name")) {
			Map.Tileset->Name = LuaToString(l, j);
		} else if (!strcmp(value, "image")) {
			Map.Tileset->ImageFile = LuaToString(l, j);
		} else if (!strcmp(value, "size")) {
			if (!lua_istable(l, j)) {
				LuaError(l, "incorrect argument");
			}
			lua_rawgeti(l, j, 1);
			Map.Tileset->PixelTileSize.x = LuaToNumber(l, -1);
			lua_pop(l, 1);
			lua_rawgeti(l, j, 2);
			Map.Tileset->PixelTileSize.y = LuaToNumber(l, -1);
			lua_pop(l, 1);
		} else if (!strcmp(value, "slots")) {
			if (!lua_istable(l, j)) {
				LuaError(l, "incorrect argument");
			}
			DefineTilesetParseSlot(l, Map.Tileset, j);
		} else {
			LuaError(l, "Unsupported tag: %s" _C_ value);
		}
	}
	LoadTileset();
	return 0;
}

/**
** Build tileset tables like HumanWallTable or MixedLookupTable
**
** Called after LoadTileset and only for tilesets that have wall,
** trees and rocks. This function will be deleted when removing
** support of walls and alike in the tileset.
*/
static int CclBuildTilesetTables(lua_State *l)
{
	LuaCheckArgs(l, 0);

	CTileset &tileset = *Map.Tileset;
	//  Calculate number of tiles in graphic tile
	int n = tileset.NumTiles;

	tileset.MixedLookupTable.resize(n, 0);

	//  Build the TileTypeTable
	tileset.TileTypeTable.resize(n, 0);

	const std::vector<unsigned short> &table = tileset.Table;
	int tile;
	for (int i = 0; i < n; ++i) {
		if ((tile = table[i])) {
			unsigned flags;

			//Initialize all Lookup Items to zero
			tileset.MixedLookupTable[table[i]] = 0;

			flags = tileset.FlagsTable[i];
			if (flags & MapFieldWaterAllowed) {
				tileset.TileTypeTable[tile] = TileTypeWater;
			} else if (flags & MapFieldCoastAllowed) {
				tileset.TileTypeTable[tile] = TileTypeCoast;
			} else if (flags & MapFieldWall) {
				if (flags & MapFieldHuman) {
					tileset.TileTypeTable[tile] = TileTypeHumanWall;
				} else {
					tileset.TileTypeTable[tile] = TileTypeOrcWall;
				}
			} else if (flags & MapFieldRocks) {
				tileset.TileTypeTable[tile] = TileTypeRock;
			} else if (flags & MapFieldForest) {
				tileset.TileTypeTable[tile] = TileTypeWood;
			}
		}
	}

	//  mark the special tiles
	if ((tile = tileset.TopOneTree)) {
		tileset.TileTypeTable[tile] = TileTypeWood;
	}
	if ((tile = tileset.MidOneTree)) {
		tileset.TileTypeTable[tile] = TileTypeWood;
	}
	if ((tile = tileset.BotOneTree)) {
		tileset.TileTypeTable[tile] = TileTypeWood;
	}
	if ((tile = tileset.TopOneRock)) {
		tileset.TileTypeTable[tile] = TileTypeRock;
	}
	if ((tile = tileset.MidOneRock)) {
		tileset.TileTypeTable[tile] = TileTypeRock;
	}
	if ((tile = tileset.BotOneRock)) {
		tileset.TileTypeTable[tile] = TileTypeRock;
	}

	//  Build wood removement table.
	int solid = 0;
	int mixed = 0;
	n = tileset.NumTiles;
	for (int i = 0; i < n;) {
		if (tileset.Tiles[i].BaseTerrain
			&& tileset.Tiles[i].MixTerrain) {
			if (tileset.FlagsTable[i] & MapFieldForest) {
				mixed = i;
			}
			i += 256;
		} else {
			if (tileset.Tiles[i].BaseTerrain != 0 &&
				tileset.Tiles[i].MixTerrain == 0) {
				if (tileset.FlagsTable[i] & MapFieldForest) {
					solid = i;
				}
			}
			i += 16;
		}
	}
	tileset.WoodTable[ 0] = -1;
	tileset.WoodTable[ 1] = table[mixed + 0x30];
	tileset.WoodTable[ 2] = table[mixed + 0x70];
	tileset.WoodTable[ 3] = table[mixed + 0xB0];
	tileset.WoodTable[ 4] = table[mixed + 0x10];
	tileset.WoodTable[ 5] = table[mixed + 0x50];
	tileset.WoodTable[ 6] = table[mixed + 0x90];
	tileset.WoodTable[ 7] = table[mixed + 0xD0];
	tileset.WoodTable[ 8] = table[mixed + 0x00];
	tileset.WoodTable[ 9] = table[mixed + 0x40];
	tileset.WoodTable[10] = table[mixed + 0x80];
	tileset.WoodTable[11] = table[mixed + 0xC0];
	tileset.WoodTable[12] = table[mixed + 0x20];
	tileset.WoodTable[13] = table[mixed + 0x60];
	tileset.WoodTable[14] = table[mixed + 0xA0];
	tileset.WoodTable[15] = table[solid];
	tileset.WoodTable[16] = -1;
	tileset.WoodTable[17] = tileset.BotOneTree;
	tileset.WoodTable[18] = tileset.TopOneTree;
	tileset.WoodTable[19] = tileset.MidOneTree;

	//Mark which corners of each tile has tree in it.
	//All corners for solid tiles. (Same for rocks)
	//1 Bottom Left
	//2 Bottom Right
	//4 Top Right
	//8 Top Left
	//16 Bottom Tree Tile
	//32 Top Tree Tile
	for (int i = solid; i < solid + 16; ++i) {
		tileset.MixedLookupTable[table[i]] = 15;
	}
	for (int i = mixed; i < mixed + 256; ++i) {
		int check;

		check = (int)((i - mixed) / 16);
		switch (check) {
			case 0:
				tileset.MixedLookupTable[table[i]] = 8;
				break;
			case 1:
				tileset.MixedLookupTable[table[i]] = 4;
				break;
			case 2:
				tileset.MixedLookupTable[table[i]] = 8 + 4;
				break;
			case 3:
				tileset.MixedLookupTable[table[i]] = 1;
				break;
			case 4:
				tileset.MixedLookupTable[table[i]] = 8 + 1;
				break;
			case 5:
				tileset.MixedLookupTable[table[i]] = 4 + 1;
				break;
			case 6:
				tileset.MixedLookupTable[table[i]] = 8 + 4 + 1;
				break;
			case 7:
				tileset.MixedLookupTable[table[i]] = 2;
				break;
			case 8:
				tileset.MixedLookupTable[table[i]] = 8 + 2;
				break;
			case 9:
				tileset.MixedLookupTable[table[i]] = 4 + 2;
				break;
			case 10:
				tileset.MixedLookupTable[table[i]] = 8 + 4 + 2;
				break;
			case 11:
				tileset.MixedLookupTable[table[i]] = 2 + 1;
				break;
			case 12:
				tileset.MixedLookupTable[table[i]] = 8 + 2 + 1;
				break;
			case 13:
				tileset.MixedLookupTable[table[i]] = 4 + 2 + 1;
				break;
			default:
				tileset.MixedLookupTable[table[i]] = 0;
				break;
		}
	}
	//16 Bottom Tree Special
	//32 Top Tree Special
	//64 Mid tree special - differentiate with mixed tiles.
	tileset.MixedLookupTable[tileset.BotOneTree] = 12 + 16;
	tileset.MixedLookupTable[tileset.TopOneTree] = 3 + 32;
	tileset.MixedLookupTable[tileset.MidOneTree] = 15 + 48;

	//  Build rock removement table.
	mixed = 0;
	solid = 0;
	for (int i = 0; i < n;) {
		if (tileset.Tiles[i].BaseTerrain
			&& tileset.Tiles[i].MixTerrain) {
			if (tileset.FlagsTable[i] & MapFieldRocks) {
				mixed = i;
			}
			i += 256;
		} else {
			if (tileset.Tiles[i].BaseTerrain != 0 &&
				tileset.Tiles[i].MixTerrain == 0) {
				if (tileset.FlagsTable[i] & MapFieldRocks) {
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
		tileset.MixedLookupTable[table[i]] = 15;
	}
	for (int i = mixed; i < mixed + 256; ++i) {
		int check = (int)((i - mixed) / 16);
		switch (check) {
			case 0:
				tileset.MixedLookupTable[table[i]] = 8;
				break;
			case 1:
				tileset.MixedLookupTable[table[i]] = 4;
				break;
			case 2:
				tileset.MixedLookupTable[table[i]] = 8 + 4;
				break;
			case 3:
				tileset.MixedLookupTable[table[i]] = 1;
				break;
			case 4:
				tileset.MixedLookupTable[table[i]] = 8 + 1;
				break;
			case 5:
				tileset.MixedLookupTable[table[i]] = 4 + 1;
				break;
			case 6:
				tileset.MixedLookupTable[table[i]] = 8 + 4 + 1;
				break;
			case 7:
				tileset.MixedLookupTable[table[i]] = 2;
				break;
			case 8:
				tileset.MixedLookupTable[table[i]] = 8 + 2;
				break;
			case 9:
				tileset.MixedLookupTable[table[i]] = 4 + 2;
				break;
			case 10:
				tileset.MixedLookupTable[table[i]] = 8 + 4 + 2;
				break;
			case 11:
				tileset.MixedLookupTable[table[i]] = 2 + 1;
				break;
			case 12:
				tileset.MixedLookupTable[table[i]] = 8 + 2 + 1;
				break;
			case 13:
				tileset.MixedLookupTable[table[i]] = 4 + 2 + 1;
				break;
			default:
				tileset.MixedLookupTable[table[i]] = 0;
				break;
		}
	}

	tileset.MixedLookupTable[tileset.BotOneRock] = 12 + 16;
	tileset.MixedLookupTable[tileset.TopOneRock] = 3 + 32;
	tileset.MixedLookupTable[tileset.MidOneRock] = 15 + 48;

	tileset.RockTable[ 0] = -1;
	tileset.RockTable[ 1] = table[mixed + 0x30];
	tileset.RockTable[ 2] = table[mixed + 0x70];
	tileset.RockTable[ 3] = table[mixed + 0xB0];
	tileset.RockTable[ 4] = table[mixed + 0x10];
	tileset.RockTable[ 5] = table[mixed + 0x50];
	tileset.RockTable[ 6] = table[mixed + 0x90];
	tileset.RockTable[ 7] = table[mixed + 0xD0];
	tileset.RockTable[ 8] = table[mixed + 0x00];
	tileset.RockTable[ 9] = table[mixed + 0x40];
	tileset.RockTable[10] = table[mixed + 0x80];
	tileset.RockTable[11] = table[mixed + 0xC0];
	tileset.RockTable[12] = table[mixed + 0x20];
	tileset.RockTable[13] = table[mixed + 0x60];
	tileset.RockTable[14] = table[mixed + 0xA0];
	tileset.RockTable[15] = table[solid];
	tileset.RockTable[16] = -1;
	tileset.RockTable[17] = tileset.BotOneRock;
	tileset.RockTable[18] = tileset.TopOneRock;
	tileset.RockTable[19] = tileset.MidOneRock;

	// FIXME: Build wall replacement tables
	tileset.HumanWallTable[ 0] = 0x090;
	tileset.HumanWallTable[ 1] = 0x830;
	tileset.HumanWallTable[ 2] = 0x810;
	tileset.HumanWallTable[ 3] = 0x850;
	tileset.HumanWallTable[ 4] = 0x800;
	tileset.HumanWallTable[ 5] = 0x840;
	tileset.HumanWallTable[ 6] = 0x820;
	tileset.HumanWallTable[ 7] = 0x860;
	tileset.HumanWallTable[ 8] = 0x870;
	tileset.HumanWallTable[ 9] = 0x8B0;
	tileset.HumanWallTable[10] = 0x890;
	tileset.HumanWallTable[11] = 0x8D0;
	tileset.HumanWallTable[12] = 0x880;
	tileset.HumanWallTable[13] = 0x8C0;
	tileset.HumanWallTable[14] = 0x8A0;
	tileset.HumanWallTable[15] = 0x0B0;

	tileset.OrcWallTable[ 0] = 0x0A0;
	tileset.OrcWallTable[ 1] = 0x930;
	tileset.OrcWallTable[ 2] = 0x910;
	tileset.OrcWallTable[ 3] = 0x950;
	tileset.OrcWallTable[ 4] = 0x900;
	tileset.OrcWallTable[ 5] = 0x940;
	tileset.OrcWallTable[ 6] = 0x920;
	tileset.OrcWallTable[ 7] = 0x960;
	tileset.OrcWallTable[ 8] = 0x970;
	tileset.OrcWallTable[ 9] = 0x9B0;
	tileset.OrcWallTable[10] = 0x990;
	tileset.OrcWallTable[11] = 0x9D0;
	tileset.OrcWallTable[12] = 0x980;
	tileset.OrcWallTable[13] = 0x9C0;
	tileset.OrcWallTable[14] = 0x9A0;
	tileset.OrcWallTable[15] = 0x0C0;

	// Set destroyed walls to TileTypeUnknown
	for (int i = 0; i < 16; ++i) {
		n = 0;
		tile = tileset.HumanWallTable[i];
		while (tileset.Table[tile]) { // Skip good tiles
			++tile;
			++n;
		}
		while (!tileset.Table[tile]) { // Skip separator
			++tile;
			++n;
		}
		while (tileset.Table[tile]) { // Skip good tiles
			++tile;
			++n;
		}
		while (!tileset.Table[tile]) { // Skip separator
			++tile;
			++n;
		}
		while (n < 16 && tileset.Table[tile]) {
			tileset.TileTypeTable[tileset.Table[tile]] = TileTypeUnknown;
			++tile;
			++n;
		}
	}
	return 0;
}

/**
**  Set the flags like "water" for a tile of a tileset
**
**  @param l  Lua state.
*/
static int CclSetTileFlags(lua_State *l)
{
	if (lua_gettop(l) < 2) {
		LuaError(l, "No flags defined");
	}
	const int tilenumber = LuaToNumber(l, 1);

	if (tilenumber >= Map.Tileset->NumTiles) {
		LuaError(l, "Accessed a tile that's not defined");
	}
	int j = 0;
	int flags = 0;

	ParseTilesetTileFlags(l, &flags, &j);
	Map.Tileset->FlagsTable[tilenumber] = flags;
	return 0;
}

/**
**  Register CCL features for tileset.
*/
void TilesetCclRegister()
{
	lua_register(Lua, "DefineTileset", CclDefineTileset);
	lua_register(Lua, "SetTileFlags", CclSetTileFlags);
	lua_register(Lua, "BuildTilesetTables", CclBuildTilesetTables);
}

//@}
