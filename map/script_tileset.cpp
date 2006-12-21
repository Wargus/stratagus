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
//      (c) Copyright 2000-2006 by Lutz Sammer, Francois Beerten and Jimmy Salmon
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
//      $Id$

//@{

/*----------------------------------------------------------------------------
--  Includes
----------------------------------------------------------------------------*/

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "stratagus.h"
#include "script.h"
#include "tileset.h"
#include "map.h"

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
static void ExtendTilesetTables(CTileset *tileset, int oldtiles, int newtiles)
{
	unsigned short *newtable;
	TileInfo *newtileinfo;

	newtable = new unsigned short[oldtiles + newtiles];
	if (!newtable) {
		fprintf(stderr, "out of memory.\n");
		ExitFatal(-1);
	}
	memcpy(newtable, tileset->Table, oldtiles * sizeof(short));
	delete[] tileset->Table;
	tileset->Table = newtable;

	newtable = new unsigned short[oldtiles + newtiles];
	if (!newtable) {
		fprintf(stderr, "out of memory.\n");
		ExitFatal(-1);
	}
	memcpy(newtable, tileset->FlagsTable, oldtiles * sizeof(short));
	delete[] tileset->FlagsTable;
	tileset->FlagsTable = newtable;

	newtileinfo = new TileInfo[oldtiles + newtiles];
	if (!newtileinfo) {
		fprintf(stderr, "out of memory.\n");
		ExitFatal(-1);
	}
	memcpy(newtileinfo, tileset->Tiles, oldtiles * sizeof(TileInfo));
	delete[] tileset->Tiles;
	tileset->Tiles = newtileinfo;
}

/**
**  Parse the name field in tileset definition.
**
**  @param l        Lua state.
**  @param tileset  Tileset currently parsed.
*/
static int TilesetParseName(lua_State *l, CTileset *tileset)
{
	char *ident;
	int i;
	
	ident = new_strdup(LuaToString(l, -1));
	for (i = 0; i < tileset->NumTerrainTypes; ++i) {
		if (!strcmp(ident, tileset->SolidTerrainTypes[i].TerrainName)) {
			delete[] ident;
			return i;
		}
	}

	// Can't find it, then we add another solid terrain type.
	SolidTerrainInfo *s = new SolidTerrainInfo[tileset->NumTerrainTypes + 1];
	memcpy(s, tileset->SolidTerrainTypes, tileset->NumTerrainTypes * sizeof(SolidTerrainInfo));
	delete[] tileset->SolidTerrainTypes;
	tileset->SolidTerrainTypes = s;
	tileset->SolidTerrainTypes[tileset->NumTerrainTypes].TerrainName = ident;
	++tileset->NumTerrainTypes;
	
	return i;
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
	int flags;
	const char *value;

	//
	//  Parse the list: flags of the slot
	//
	flags = 3;
	while (1) {
		lua_rawgeti(l, -1, *j + 1);
		if (!lua_isstring(l, -1)) {
			lua_pop(l, 1);
			break;
		}
		++(*j);
		value = LuaToString(l, -1);
		lua_pop(l, 1);

		//
		//  Flags are only needed for the editor
		//
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
	const char *value;
	int i;
	int args;
	int j;

	if (!lua_istable(l, -1)) {
		LuaError(l, "incorrect argument");
	}
	args = luaL_getn(l, -1);

	//
	//  Parse the list: (still everything could be changed!)
	//
	for (j = 0; j < args; ++j) {
		lua_rawgeti(l, -1, j + 1);
		value = LuaToString(l, -1);
		lua_pop(l, 1);

		//
		//  top-one-tree, mid-one-tree, bot-one-tree
		//
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
		//
		//  removed-tree
		//
		} else if (!strcmp(value, "removed-tree")) {
			++j;
			lua_rawgeti(l, -1, j + 1);
			tileset->RemovedTree = LuaToNumber(l, -1);
			lua_pop(l, 1);
		//
		//  growing-tree
		//
		} else if (!strcmp(value, "growing-tree")) {
			++j;
			lua_rawgeti(l, -1, j + 1);
			if (!lua_istable(l, -1)) {
				LuaError(l, "incorrect argument");
			}
			if (luaL_getn(l, -1) != 2) {
				LuaError(l, "growing-tree: Wrong table length");
			}
			for (i = 0; i < 2; ++i) {
				lua_rawgeti(l, -1, i + 1);
				tileset->GrowingTree[i] = LuaToNumber(l, -1);
				lua_pop(l, 1);
			}
			lua_pop(l, 1);

		//
		//  top-one-rock, mid-one-rock, bot-one-rock
		//
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
		//
		//  removed-rock
		//
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
	int i;
	int f;
	int len;
	int basic_name;
	int j;

	ExtendTilesetTables(tileset, index, 16);

	if (!lua_istable(l, -1)) {
		LuaError(l, "incorrect argument");
	}
	j = 0;
	lua_rawgeti(l, -1, j + 1);
	++j;
	basic_name = TilesetParseName(l, tileset);
	lua_pop(l, 1);

	ParseTilesetTileFlags(l, &f, &j);

	//
	//  Vector: the tiles.
	//
	lua_rawgeti(l, -1, j + 1);
	if (!lua_istable(l, -1)) {
		LuaError(l, "incorrect argument");
	}
	len = luaL_getn(l, -1);

	// hack for sc tilesets, remove when fixed
	if (len > 16) {
		ExtendTilesetTables(tileset, index, len);
	}

	for (i = 0; i < len; ++i) {
		lua_rawgeti(l, -1, i + 1);
		tileset->Table[index + i] = LuaToNumber(l, -1);
		lua_pop(l, 1);
		tileset->FlagsTable[index + i] = f;
		tileset->Tiles[index + i].BaseTerrain = basic_name;
		tileset->Tiles[index + i].MixTerrain = 0;
	}
	lua_pop(l, 1);
	while (i < 16) {
		tileset->Table[index + i] = 0;
		tileset->FlagsTable[index + i] = 0;
		tileset->Tiles[index + i].BaseTerrain = 0;
		tileset->Tiles[index + i].MixTerrain = 0;
		++i;
	}

	if (len < 16) {
		return index + 16;
	}
	return index + len;
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
	int i;
	int len;
	int f;
	int basic_name;
	int mixed_name;
	int new_index;
	int j;
	int args;

	new_index = index + 256;
	ExtendTilesetTables(tileset, index, 256);

	if (!lua_istable(l, -1)) {
		LuaError(l, "incorrect argument");
	}
	j = 0;
	args = luaL_getn(l, -1);
	lua_rawgeti(l, -1, j + 1);
	++j;
	basic_name = TilesetParseName(l, tileset);
	lua_pop(l, 1);
	lua_rawgeti(l, -1, j + 1);
	++j;
	mixed_name = TilesetParseName(l, tileset);
	lua_pop(l, 1);

	ParseTilesetTileFlags(l, &f, &j);

	for (; j < args; ++j) {
		lua_rawgeti(l, -1, j + 1);
		if (!lua_istable(l, -1)) {
			LuaError(l, "incorrect argument");
		}
		//
		//  Vector: the tiles.
		//
		len = luaL_getn(l, -1);
		for (i = 0; i < len; ++i) {
			lua_rawgeti(l, -1, i + 1);
			tileset->Table[index + i] = LuaToNumber(l, -1);
			tileset->FlagsTable[index + i] = f;
			tileset->Tiles[index + i].BaseTerrain = basic_name;
			tileset->Tiles[index + i].MixTerrain = mixed_name;
			lua_pop(l, 1);
		}
		// Fill missing slots
		while (i < 16) {
			tileset->Table[index + i] = 0;
			tileset->FlagsTable[index + i] = 0;
			tileset->Tiles[index + i].BaseTerrain = 0;
			tileset->Tiles[index + i].MixTerrain = 0;
			++i;
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
	const char *value;
	int index;
	int args;
	int j;

	index = 0;
	tileset->Table = new unsigned short[16];
	if (!tileset->Table) {
		fprintf(stderr, "out of memory.\n");
		ExitFatal(-1);
	}
	tileset->FlagsTable = new unsigned short[16];
	if (!tileset->FlagsTable) {
		fprintf(stderr, "out of memory.\n");
		ExitFatal(-1);
	}
	tileset->Tiles = new TileInfo[16];
	if (!tileset->Tiles) {
		fprintf(stderr, "out of memory.\n");
		ExitFatal(-1);
	}
	tileset->SolidTerrainTypes = new SolidTerrainInfo[1];
	if (!tileset->SolidTerrainTypes) {
		fprintf(stderr, "out of memory.\n");
		ExitFatal(-1);
	}
	tileset->SolidTerrainTypes[0].TerrainName = new_strdup("unused");
	tileset->NumTerrainTypes = 1;

	//
	//  Parse the list: (still everything could be changed!)
	//
	args = luaL_getn(l, t);
	for (j = 0; j < args; ++j) {
		lua_rawgeti(l, t, j + 1);
		value = LuaToString(l, -1);
		lua_pop(l, 1);
		++j;

		//
		//  special part
		//
		if (!strcmp(value, "special")) {
			lua_rawgeti(l, t, j + 1);
			DefineTilesetParseSpecial(l, tileset);
			lua_pop(l, 1);
		//
		//  solid part
		//
		} else if (!strcmp(value, "solid")) {
			lua_rawgeti(l, t, j + 1);
			index = DefineTilesetParseSolid(l, tileset, index);
			lua_pop(l, 1);
		//
		//  mixed part
		//
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
	const char *value;
	int args;
	int j;

	Map.Tileset.Clear();

	Map.Tileset.TileSizeX = 32;
	Map.Tileset.TileSizeY = 32;

	//
	//  Parse the list: (still everything could be changed!)
	//
	args = lua_gettop(l);
	for (j = 1; j < args; ++j) {
		value = LuaToString(l, j);
		++j;

		if (!strcmp(value, "name")) {
			Map.Tileset.Name = LuaToString(l, j);
		} else if (!strcmp(value, "image")) {
			Map.Tileset.ImageFile = LuaToString(l, j);
		} else if (!strcmp(value, "size")) {
			if (!lua_istable(l, j)) {
				LuaError(l, "incorrect argument");
			}
			lua_rawgeti(l, j, 1);
			Map.Tileset.TileSizeX = LuaToNumber(l, -1);
			lua_pop(l, 1);
			lua_rawgeti(l, j, 2);
			Map.Tileset.TileSizeY = LuaToNumber(l, -1);
			lua_pop(l, 1);
		} else if (!strcmp(value, "slots")) {
			if (!lua_istable(l, j)) {
				LuaError(l, "incorrect argument");
			}
			DefineTilesetParseSlot(l, &Map.Tileset, j);
		} else {
			LuaError(l, "Unsupported tag: %s" _C_ value);
		}
	}
	LoadTileset();
	return 0;
}

/**
** Build tileset tables like MixedLookupTable
**
** Called after LoadTileset and only for tilesets that have 
** trees and rocks. This function will be deleted when removing 
** support of trees and alike in the tileset.
*/
static int CclBuildTilesetTables(lua_State *l)
{
	int n;
	int tile;
	int solid;
	int mixed;
	const unsigned short *table;
	int i;

	LuaCheckArgs(l, 0);

	//  Calculate number of tiles in graphic tile
	n = Map.Tileset.NumTiles;

	Map.Tileset.MixedLookupTable = new int[n];
	memset(Map.Tileset.MixedLookupTable, 0, n * sizeof(int));

	//  Build the TileTypeTable
	Map.Tileset.TileTypeTable = new unsigned char[n];
	memset(Map.Tileset.TileTypeTable, 0, n * sizeof(unsigned char));

	table = Map.Tileset.Table;
	for (i = 0; i < n; ++i) {
		if ((tile = table[i])) {
			unsigned flags;

			//Initialize all Lookup Items to zero
			Map.Tileset.MixedLookupTable[table[i]] = 0;

			flags = Map.Tileset.FlagsTable[i];
			if (flags & MapFieldWaterAllowed) {
				Map.Tileset.TileTypeTable[tile] = TileTypeWater;
			} else if (flags & MapFieldCoastAllowed) {
				Map.Tileset.TileTypeTable[tile] = TileTypeCoast;
			} else if (flags & MapFieldRocks) {
				Map.Tileset.TileTypeTable[tile] = TileTypeRock;
			} else if (flags & MapFieldForest) {
				Map.Tileset.TileTypeTable[tile] = TileTypeWood;
			}
		}
	}

	//  mark the special tiles
	if ((tile = Map.Tileset.TopOneTree)) {
		Map.Tileset.TileTypeTable[tile] = TileTypeWood;
	}
	if ((tile = Map.Tileset.MidOneTree)) {
		Map.Tileset.TileTypeTable[tile] = TileTypeWood;
	}
	if ((tile = Map.Tileset.BotOneTree)) {
		Map.Tileset.TileTypeTable[tile] = TileTypeWood;
	}
	if ((tile = Map.Tileset.TopOneRock)) {
		Map.Tileset.TileTypeTable[tile] = TileTypeRock;
	}
	if ((tile = Map.Tileset.MidOneRock)) {
		Map.Tileset.TileTypeTable[tile] = TileTypeRock;
	}
	if ((tile = Map.Tileset.BotOneRock)) {
		Map.Tileset.TileTypeTable[tile] = TileTypeRock;
	}

	//  Build wood removement table.
	n = Map.Tileset.NumTiles;
	for (mixed = solid = i = 0; i < n;) {
		if (Map.Tileset.Tiles[i].BaseTerrain
			&& Map.Tileset.Tiles[i].MixTerrain) {
			if (Map.Tileset.FlagsTable[i] & MapFieldForest) {
				mixed = i;
			}
			i += 256;
		} else {
			if (Map.Tileset.Tiles[i].BaseTerrain != 0 &&
				Map.Tileset.Tiles[i].MixTerrain == 0) {
					if (Map.Tileset.FlagsTable[i] & MapFieldForest) {
						solid = i;
				}
			}
			i += 16;
		}
	}
	Map.Tileset.WoodTable[ 0] = -1;
	Map.Tileset.WoodTable[ 1] = table[mixed + 0x30];
	Map.Tileset.WoodTable[ 2] = table[mixed + 0x70];
	Map.Tileset.WoodTable[ 3] = table[mixed + 0xB0];
	Map.Tileset.WoodTable[ 4] = table[mixed + 0x10];
	Map.Tileset.WoodTable[ 5] = table[mixed + 0x50];
	Map.Tileset.WoodTable[ 6] = table[mixed + 0x90];
	Map.Tileset.WoodTable[ 7] = table[mixed + 0xD0];
	Map.Tileset.WoodTable[ 8] = table[mixed + 0x00];
	Map.Tileset.WoodTable[ 9] = table[mixed + 0x40];
	Map.Tileset.WoodTable[10] = table[mixed + 0x80];
	Map.Tileset.WoodTable[11] = table[mixed + 0xC0];
	Map.Tileset.WoodTable[12] = table[mixed + 0x20];
	Map.Tileset.WoodTable[13] = table[mixed + 0x60];
	Map.Tileset.WoodTable[14] = table[mixed + 0xA0];
	Map.Tileset.WoodTable[15] = table[solid];
	Map.Tileset.WoodTable[16] = -1;
	Map.Tileset.WoodTable[17] = Map.Tileset.BotOneTree;
	Map.Tileset.WoodTable[18] = Map.Tileset.TopOneTree;
	Map.Tileset.WoodTable[19] = Map.Tileset.MidOneTree;

	//Mark which corners of each tile has tree in it.
	//All corners for solid tiles. (Same for rocks)
	//1 Bottom Left
	//2 Bottom Right
	//4 Top Right
	//8 Top Left
	//16 Bottom Tree Tile
	//32 Top Tree Tile
	for (i = solid; i < solid + 16; ++i) {
		Map.Tileset.MixedLookupTable[table[i]] = 15;
	}
	for (i = mixed; i < mixed + 256; ++i) {
		int check;

		check = (int)((i - mixed) / 16);
		switch (check) {
			case 0:
				Map.Tileset.MixedLookupTable[table[i]] = 8;
				break;
			case 1:
				Map.Tileset.MixedLookupTable[table[i]] = 4;
				break;
			case 2:
				Map.Tileset.MixedLookupTable[table[i]] = 8 + 4;
				break;
			case 3:
				Map.Tileset.MixedLookupTable[table[i]] = 1;
				break;
			case 4:
				Map.Tileset.MixedLookupTable[table[i]] = 8 + 1;
				break;
			case 5:
				Map.Tileset.MixedLookupTable[table[i]] = 4 + 1;
				break;
			case 6:
				Map.Tileset.MixedLookupTable[table[i]] = 8 + 4 + 1;
				break;
			case 7:
				Map.Tileset.MixedLookupTable[table[i]] = 2;
				break;
			case 8:
				Map.Tileset.MixedLookupTable[table[i]] = 8 + 2;
				break;
			case 9:
				Map.Tileset.MixedLookupTable[table[i]] = 4 + 2;
				break;
			case 10:
				Map.Tileset.MixedLookupTable[table[i]] = 8 + 4 + 2;
				break;
			case 11:
				Map.Tileset.MixedLookupTable[table[i]] = 2 + 1;
				break;
			case 12:
				Map.Tileset.MixedLookupTable[table[i]] = 8 + 2 + 1;
				break;
			case 13:
				Map.Tileset.MixedLookupTable[table[i]] = 4 + 2 + 1;
				break;
			default:
				Map.Tileset.MixedLookupTable[table[i]] = 0;
				break;
		}
	}
	//16 Bottom Tree Special
	//32 Top Tree Special
	//64 Mid tree special - differentiate with mixed tiles.
	Map.Tileset.MixedLookupTable[Map.Tileset.BotOneTree] = 12 + 16;
	Map.Tileset.MixedLookupTable[Map.Tileset.TopOneTree] = 3 + 32;
	Map.Tileset.MixedLookupTable[Map.Tileset.MidOneTree] = 15 + 48;

	//  Build rock removement table.
	for (mixed = solid = i = 0; i < n;) {
		if (Map.Tileset.Tiles[i].BaseTerrain
			&& Map.Tileset.Tiles[i].MixTerrain) {
			if (Map.Tileset.FlagsTable[i] & MapFieldRocks) {
				mixed = i;
			}
			i += 256;
		} else {
			if (Map.Tileset.Tiles[i].BaseTerrain != 0 &&
				Map.Tileset.Tiles[i].MixTerrain == 0) {
					  if (Map.Tileset.FlagsTable[i] & MapFieldRocks) {
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
	for (i = solid; i < solid + 16; ++i) {
		Map.Tileset.MixedLookupTable[table[i]] = 15;
	}
	for (i = mixed; i < mixed + 256; ++i) {
		int check;

		check = (int)((i - mixed) / 16);
		switch (check) {
			case 0:
				Map.Tileset.MixedLookupTable[table[i]] = 8;
				break;
			case 1:
				Map.Tileset.MixedLookupTable[table[i]] = 4;
				break;
			case 2:
				Map.Tileset.MixedLookupTable[table[i]] = 8 + 4;
				break;
			case 3:
				Map.Tileset.MixedLookupTable[table[i]] = 1;
				break;
			case 4:
				Map.Tileset.MixedLookupTable[table[i]] = 8 + 1;
				break;
			case 5:
				Map.Tileset.MixedLookupTable[table[i]] = 4 + 1;
				break;
			case 6:
				Map.Tileset.MixedLookupTable[table[i]] = 8 + 4 + 1;
				break;
			case 7:
				Map.Tileset.MixedLookupTable[table[i]] = 2;
				break;
			case 8:
				Map.Tileset.MixedLookupTable[table[i]] = 8 + 2;
				break;
			case 9:
				Map.Tileset.MixedLookupTable[table[i]] = 4 + 2;
				break;
			case 10:
				Map.Tileset.MixedLookupTable[table[i]] = 8 + 4 + 2;
				break;
			case 11:
				Map.Tileset.MixedLookupTable[table[i]] = 2 + 1;
				break;
			case 12:
				Map.Tileset.MixedLookupTable[table[i]] = 8 + 2 + 1;
				break;
			case 13:
				Map.Tileset.MixedLookupTable[table[i]] = 4 + 2 + 1;
				break;
			default:
				Map.Tileset.MixedLookupTable[table[i]] = 0;
				break;
		}
	}

	Map.Tileset.MixedLookupTable[Map.Tileset.BotOneRock] = 12 + 16;
	Map.Tileset.MixedLookupTable[Map.Tileset.TopOneRock] = 3 + 32;
	Map.Tileset.MixedLookupTable[Map.Tileset.MidOneRock] = 15 + 48;

	Map.Tileset.RockTable[ 0] = -1;
	Map.Tileset.RockTable[ 1] = table[mixed + 0x30];
	Map.Tileset.RockTable[ 2] = table[mixed + 0x70];
	Map.Tileset.RockTable[ 3] = table[mixed + 0xB0];
	Map.Tileset.RockTable[ 4] = table[mixed + 0x10];
	Map.Tileset.RockTable[ 5] = table[mixed + 0x50];
	Map.Tileset.RockTable[ 6] = table[mixed + 0x90];
	Map.Tileset.RockTable[ 7] = table[mixed + 0xD0];
	Map.Tileset.RockTable[ 8] = table[mixed + 0x00];
	Map.Tileset.RockTable[ 9] = table[mixed + 0x40];
	Map.Tileset.RockTable[10] = table[mixed + 0x80];
	Map.Tileset.RockTable[11] = table[mixed + 0xC0];
	Map.Tileset.RockTable[12] = table[mixed + 0x20];
	Map.Tileset.RockTable[13] = table[mixed + 0x60];
	Map.Tileset.RockTable[14] = table[mixed + 0xA0];
	Map.Tileset.RockTable[15] = table[solid];
	Map.Tileset.RockTable[16] = -1;
	Map.Tileset.RockTable[17] = Map.Tileset.BotOneRock;
	Map.Tileset.RockTable[18] = Map.Tileset.TopOneRock;
	Map.Tileset.RockTable[19] = Map.Tileset.MidOneRock;

	return 0;
}

/**
**  Set the flags like "water" for a tile of a tileset
**
**  @param l  Lua state.
*/
static int CclSetTileFlags(lua_State *l)
{
	int j;
	int tilenumber;
	int flags;

	if (lua_gettop(l) < 2) {
		LuaError(l, "No flags defined");
	}

	tilenumber = LuaToNumber(l, 1);
	
	if (tilenumber >= Map.Tileset.NumTiles) {
		LuaError(l, "Accessed a tile that's not defined");
	}
	
	j = 0;
	flags = 0;
	ParseTilesetTileFlags(l, &flags, &j);
	Map.Tileset.FlagsTable[tilenumber] = flags;

	return 0;
}

/**
**  Register CCL features for tileset.
*/
void TilesetCclRegister(void)
{
	lua_register(Lua, "DefineTileset", CclDefineTileset);
	lua_register(Lua, "SetTileFlags", CclSetTileFlags);
	lua_register(Lua, "BuildTilesetTables", CclBuildTilesetTables);
}

//@}
