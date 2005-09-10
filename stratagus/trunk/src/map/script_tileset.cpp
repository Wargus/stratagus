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
/**@name script_tileset.c - The tileset ccl functions. */
//
//      (c) Copyright 2000-2005 by Lutz Sammer, François Beerten and Jimmy Salmon
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
**  @param tileset  Tileset to be extended.
**  @param tiles    Number of tiles.
*/
static void ExtendTilesetTables(Tileset* tileset, int tiles)
{
	tileset->Table = (unsigned short*)realloc(tileset->Table, tiles * sizeof(*tileset->Table));
	if (!tileset->Table) {
		fprintf(stderr, "out of memory.\n");
		ExitFatal(-1);
	}
	tileset->FlagsTable =
		(unsigned short*)realloc(tileset->FlagsTable, tiles * sizeof(*tileset->FlagsTable));
	if (!tileset->FlagsTable) {
		fprintf(stderr, "out of memory.\n");
		ExitFatal(-1);
	}
	tileset->Tiles = (TileInfo*)realloc(tileset->Tiles,
		tiles * sizeof(*tileset->Tiles));
	if (!tileset->Tiles) {
		fprintf(stderr, "out of memory.\n");
		ExitFatal(-1);
	}
}

/**
**  Parse the name field in tileset definition.
**
**  @param l        Lua state.
**  @param tileset  Tileset currently parsed.
*/
static int TilesetParseName(lua_State* l, Tileset* tileset)
{
	char* ident;
	int i;
	
	ident = strdup(LuaToString(l, -1));
	for (i = 0; i < tileset->NumTerrainTypes; ++i) {
		if (!strcmp(ident, tileset->SolidTerrainTypes[i].TerrainName)) {
			free(ident);
			return i;
		}
	}

	// Can't find it, then we add another solid terrain type.
	tileset->SolidTerrainTypes = (SolidTerrainInfo*)realloc(tileset->SolidTerrainTypes,
		++tileset->NumTerrainTypes * sizeof(*tileset->SolidTerrainTypes));
	tileset->SolidTerrainTypes[i].TerrainName = ident;
	
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
static void ParseTilesetTileFlags(lua_State* l, int* back, int* j)
{
	int flags;
	const char* value;

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
static void DefineTilesetParseSpecial(lua_State* l, Tileset* tileset)
{
	const char* value;
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
static int DefineTilesetParseSolid(lua_State* l, Tileset* tileset, int index)
{
	int i;
	int f;
	int len;
	int basic_name;
	int j;

	ExtendTilesetTables(tileset, index + 16);

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
		ExtendTilesetTables(tileset, index + len);
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
static int DefineTilesetParseMixed(lua_State* l, Tileset* tileset, int index)
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
	ExtendTilesetTables(tileset, new_index);

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
static void DefineTilesetParseSlot(lua_State* l, Tileset* tileset, int t)
{
	const char* value;
	int index;
	int args;
	int j;

	index = 0;
	tileset->Table = (unsigned short*)malloc(16 * sizeof(*tileset->Table));
	if (!tileset->Table) {
		fprintf(stderr, "out of memory.\n");
		ExitFatal(-1);
	}
	tileset->FlagsTable =
		(unsigned short*)malloc(16 * sizeof(*tileset->FlagsTable));
	if (!tileset->FlagsTable) {
		fprintf(stderr, "out of memory.\n");
		ExitFatal(-1);
	}
	tileset->Tiles = (TileInfo*)malloc(16 * sizeof(TileInfo));
	if (!tileset->Tiles) {
		fprintf(stderr, "out of memory.\n");
		ExitFatal(-1);
	}
	tileset->SolidTerrainTypes = (SolidTerrainInfo*)malloc(sizeof(SolidTerrainInfo));
	if (!tileset->SolidTerrainTypes) {
		fprintf(stderr, "out of memory.\n");
		ExitFatal(-1);
	}
	tileset->SolidTerrainTypes[0].TerrainName = strdup("unused");
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
static int CclDefineTileset(lua_State* l)
{
	const char* value;
	int args;
	int j;

	free(TheMap.Tileset.Name);
	free(TheMap.Tileset.ImageFile);
	free(TheMap.Tileset.Table);
	free(TheMap.Tileset.Tiles);
	free(TheMap.Tileset.TileTypeTable);
	memset(&TheMap.Tileset, 0, sizeof(Tileset));

	TheMap.Tileset.TileSizeX = 32;
	TheMap.Tileset.TileSizeY = 32;

	//
	//  Parse the list: (still everything could be changed!)
	//
	args = lua_gettop(l);
	for (j = 1; j < args; ++j) {
		value = LuaToString(l, j);
		++j;

		if (!strcmp(value, "name")) {
			TheMap.Tileset.Name = strdup(LuaToString(l, j));
		} else if (!strcmp(value, "image")) {
			TheMap.Tileset.ImageFile = strdup(LuaToString(l, j));
		} else if (!strcmp(value, "size")) {
			if (!lua_istable(l, j)) {
				LuaError(l, "incorrect argument");
			}
			lua_rawgeti(l, j, 1);
			TheMap.Tileset.TileSizeX = LuaToNumber(l, -1);
			lua_pop(l, 1);
			lua_rawgeti(l, j, 2);
			TheMap.Tileset.TileSizeY = LuaToNumber(l, -1);
			lua_pop(l, 1);
		} else if (!strcmp(value, "slots")) {
			if (!lua_istable(l, j)) {
				LuaError(l, "incorrect argument");
			}
			DefineTilesetParseSlot(l, &TheMap.Tileset, j);
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
static int CclBuildTilesetTables(lua_State* l)
{
	int n;
	int tile;
	int solid;
	int mixed;
	const unsigned short* table;
	int i;

	LuaCheckArgs(l, 0);

	//  Calculate number of tiles in graphic tile
	n = TheMap.Tileset.NumTiles;

	TheMap.Tileset.MixedLookupTable = (int*)calloc(n, sizeof(int));

	//  Build the TileTypeTable
	TheMap.Tileset.TileTypeTable =
		(unsigned char*)calloc(n, sizeof(*TheMap.Tileset.TileTypeTable));

	table = TheMap.Tileset.Table;
	for (i = 0; i < n; ++i) {
		if ((tile = table[i])) {
			unsigned flags;

			//Initialize all Lookup Items to zero
			TheMap.Tileset.MixedLookupTable[table[i]] = 0;

			flags = TheMap.Tileset.FlagsTable[i];
			if (flags & MapFieldWaterAllowed) {
				TheMap.Tileset.TileTypeTable[tile] = TileTypeWater;
			} else if (flags & MapFieldCoastAllowed) {
				TheMap.Tileset.TileTypeTable[tile] = TileTypeCoast;
			} else if (flags & MapFieldWall) {
				if (flags & MapFieldHuman) {
					TheMap.Tileset.TileTypeTable[tile] = TileTypeHumanWall;
				} else {
					TheMap.Tileset.TileTypeTable[tile] = TileTypeOrcWall;
				}
			} else if (flags & MapFieldRocks) {
				TheMap.Tileset.TileTypeTable[tile] = TileTypeRock;
			} else if (flags & MapFieldForest) {
				TheMap.Tileset.TileTypeTable[tile] = TileTypeWood;
			}
		}
	}

	//  mark the special tiles
	if ((tile = TheMap.Tileset.TopOneTree)) {
		TheMap.Tileset.TileTypeTable[tile] = TileTypeWood;
	}
	if ((tile = TheMap.Tileset.MidOneTree)) {
		TheMap.Tileset.TileTypeTable[tile] = TileTypeWood;
	}
	if ((tile = TheMap.Tileset.BotOneTree)) {
		TheMap.Tileset.TileTypeTable[tile] = TileTypeWood;
	}
	if ((tile = TheMap.Tileset.TopOneRock)) {
		TheMap.Tileset.TileTypeTable[tile] = TileTypeRock;
	}
	if ((tile = TheMap.Tileset.MidOneRock)) {
		TheMap.Tileset.TileTypeTable[tile] = TileTypeRock;
	}
	if ((tile = TheMap.Tileset.BotOneRock)) {
		TheMap.Tileset.TileTypeTable[tile] = TileTypeRock;
	}

	//  Build wood removement table.
	n = TheMap.Tileset.NumTiles;
	for (mixed = solid = i = 0; i < n;) {
		if (TheMap.Tileset.Tiles[i].BaseTerrain
			&& TheMap.Tileset.Tiles[i].MixTerrain) {
			if (TheMap.Tileset.FlagsTable[i] & MapFieldForest) {
				mixed = i;
			}
			i += 256;
		} else {
			if (TheMap.Tileset.Tiles[i].BaseTerrain != 0 &&
				TheMap.Tileset.Tiles[i].MixTerrain == 0) {
					if (TheMap.Tileset.FlagsTable[i] & MapFieldForest) {
						solid = i;
				}
			}
			i += 16;
		}
	}
	TheMap.Tileset.WoodTable[ 0] = -1;
	TheMap.Tileset.WoodTable[ 1] = table[mixed + 0x30];
	TheMap.Tileset.WoodTable[ 2] = table[mixed + 0x70];
	TheMap.Tileset.WoodTable[ 3] = table[mixed + 0xB0];
	TheMap.Tileset.WoodTable[ 4] = table[mixed + 0x10];
	TheMap.Tileset.WoodTable[ 5] = table[mixed + 0x50];
	TheMap.Tileset.WoodTable[ 6] = table[mixed + 0x90];
	TheMap.Tileset.WoodTable[ 7] = table[mixed + 0xD0];
	TheMap.Tileset.WoodTable[ 8] = table[mixed + 0x00];
	TheMap.Tileset.WoodTable[ 9] = table[mixed + 0x40];
	TheMap.Tileset.WoodTable[10] = table[mixed + 0x80];
	TheMap.Tileset.WoodTable[11] = table[mixed + 0xC0];
	TheMap.Tileset.WoodTable[12] = table[mixed + 0x20];
	TheMap.Tileset.WoodTable[13] = table[mixed + 0x60];
	TheMap.Tileset.WoodTable[14] = table[mixed + 0xA0];
	TheMap.Tileset.WoodTable[15] = table[solid];
	TheMap.Tileset.WoodTable[16] = -1;
	TheMap.Tileset.WoodTable[17] = TheMap.Tileset.BotOneTree;
	TheMap.Tileset.WoodTable[18] = TheMap.Tileset.TopOneTree;
	TheMap.Tileset.WoodTable[19] = TheMap.Tileset.MidOneTree;

	//Mark which corners of each tile has tree in it.
	//All corners for solid tiles. (Same for rocks)
	//1 Bottom Left
	//2 Bottom Right
	//4 Top Right
	//8 Top Left
	//16 Bottom Tree Tile
	//32 Top Tree Tile
	for (i = solid; i < solid + 16; ++i) {
		TheMap.Tileset.MixedLookupTable[table[i]] = 15;
	}
	for (i = mixed; i < mixed + 256; ++i) {
		int check;

		check = (int)((i - mixed) / 16);
		switch (check) {
			case 0:
				TheMap.Tileset.MixedLookupTable[table[i]] = 8;
				break;
			case 1:
				TheMap.Tileset.MixedLookupTable[table[i]] = 4;
				break;
			case 2:
				TheMap.Tileset.MixedLookupTable[table[i]] = 8 + 4;
				break;
			case 3:
				TheMap.Tileset.MixedLookupTable[table[i]] = 1;
				break;
			case 4:
				TheMap.Tileset.MixedLookupTable[table[i]] = 8 + 1;
				break;
			case 5:
				TheMap.Tileset.MixedLookupTable[table[i]] = 4 + 1;
				break;
			case 6:
				TheMap.Tileset.MixedLookupTable[table[i]] = 8 + 4 + 1;
				break;
			case 7:
				TheMap.Tileset.MixedLookupTable[table[i]] = 2;
				break;
			case 8:
				TheMap.Tileset.MixedLookupTable[table[i]] = 8 + 2;
				break;
			case 9:
				TheMap.Tileset.MixedLookupTable[table[i]] = 4 + 2;
				break;
			case 10:
				TheMap.Tileset.MixedLookupTable[table[i]] = 8 + 4 + 2;
				break;
			case 11:
				TheMap.Tileset.MixedLookupTable[table[i]] = 2 + 1;
				break;
			case 12:
				TheMap.Tileset.MixedLookupTable[table[i]] = 8 + 2 + 1;
				break;
			case 13:
				TheMap.Tileset.MixedLookupTable[table[i]] = 4 + 2 + 1;
				break;
			default:
				TheMap.Tileset.MixedLookupTable[table[i]] = 0;
				break;
		}
	}
	//16 Bottom Tree Special
	//32 Top Tree Special
	//64 Mid tree special - differentiate with mixed tiles.
	TheMap.Tileset.MixedLookupTable[TheMap.Tileset.BotOneTree] = 12 + 16;
	TheMap.Tileset.MixedLookupTable[TheMap.Tileset.TopOneTree] = 3 + 32;
	TheMap.Tileset.MixedLookupTable[TheMap.Tileset.MidOneTree] = 15 + 48;

	//  Build rock removement table.
	for (mixed = solid = i = 0; i < n;) {
		if (TheMap.Tileset.Tiles[i].BaseTerrain
			&& TheMap.Tileset.Tiles[i].MixTerrain) {
			if (TheMap.Tileset.FlagsTable[i] & MapFieldRocks) {
				mixed = i;
			}
			i += 256;
		} else {
			if (TheMap.Tileset.Tiles[i].BaseTerrain != 0 &&
				TheMap.Tileset.Tiles[i].MixTerrain == 0) {
					  if (TheMap.Tileset.FlagsTable[i] & MapFieldRocks) {
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
		TheMap.Tileset.MixedLookupTable[table[i]] = 15;
	}
	for (i = mixed; i < mixed + 256; ++i) {
		int check;

		check = (int)((i - mixed) / 16);
		switch (check) {
			case 0:
				TheMap.Tileset.MixedLookupTable[table[i]] = 8;
				break;
			case 1:
				TheMap.Tileset.MixedLookupTable[table[i]] = 4;
				break;
			case 2:
				TheMap.Tileset.MixedLookupTable[table[i]] = 8 + 4;
				break;
			case 3:
				TheMap.Tileset.MixedLookupTable[table[i]] = 1;
				break;
			case 4:
				TheMap.Tileset.MixedLookupTable[table[i]] = 8 + 1;
				break;
			case 5:
				TheMap.Tileset.MixedLookupTable[table[i]] = 4 + 1;
				break;
			case 6:
				TheMap.Tileset.MixedLookupTable[table[i]] = 8 + 4 + 1;
				break;
			case 7:
				TheMap.Tileset.MixedLookupTable[table[i]] = 2;
				break;
			case 8:
				TheMap.Tileset.MixedLookupTable[table[i]] = 8 + 2;
				break;
			case 9:
				TheMap.Tileset.MixedLookupTable[table[i]] = 4 + 2;
				break;
			case 10:
				TheMap.Tileset.MixedLookupTable[table[i]] = 8 + 4 + 2;
				break;
			case 11:
				TheMap.Tileset.MixedLookupTable[table[i]] = 2 + 1;
				break;
			case 12:
				TheMap.Tileset.MixedLookupTable[table[i]] = 8 + 2 + 1;
				break;
			case 13:
				TheMap.Tileset.MixedLookupTable[table[i]] = 4 + 2 + 1;
				break;
			default:
				TheMap.Tileset.MixedLookupTable[table[i]] = 0;
				break;
		}
	}

	TheMap.Tileset.MixedLookupTable[TheMap.Tileset.BotOneRock] = 12 + 16;
	TheMap.Tileset.MixedLookupTable[TheMap.Tileset.TopOneRock] = 3 + 32;
	TheMap.Tileset.MixedLookupTable[TheMap.Tileset.MidOneRock] = 15 + 48;

	TheMap.Tileset.RockTable[ 0] = -1;
	TheMap.Tileset.RockTable[ 1] = table[mixed + 0x30];
	TheMap.Tileset.RockTable[ 2] = table[mixed + 0x70];
	TheMap.Tileset.RockTable[ 3] = table[mixed + 0xB0];
	TheMap.Tileset.RockTable[ 4] = table[mixed + 0x10];
	TheMap.Tileset.RockTable[ 5] = table[mixed + 0x50];
	TheMap.Tileset.RockTable[ 6] = table[mixed + 0x90];
	TheMap.Tileset.RockTable[ 7] = table[mixed + 0xD0];
	TheMap.Tileset.RockTable[ 8] = table[mixed + 0x00];
	TheMap.Tileset.RockTable[ 9] = table[mixed + 0x40];
	TheMap.Tileset.RockTable[10] = table[mixed + 0x80];
	TheMap.Tileset.RockTable[11] = table[mixed + 0xC0];
	TheMap.Tileset.RockTable[12] = table[mixed + 0x20];
	TheMap.Tileset.RockTable[13] = table[mixed + 0x60];
	TheMap.Tileset.RockTable[14] = table[mixed + 0xA0];
	TheMap.Tileset.RockTable[15] = table[solid];
	TheMap.Tileset.RockTable[16] = -1;
	TheMap.Tileset.RockTable[17] = TheMap.Tileset.BotOneRock;
	TheMap.Tileset.RockTable[18] = TheMap.Tileset.TopOneRock;
	TheMap.Tileset.RockTable[19] = TheMap.Tileset.MidOneRock;

	// FIXME: Build wall replacement tables
	TheMap.Tileset.HumanWallTable[ 0] = 0x090;
	TheMap.Tileset.HumanWallTable[ 1] = 0x830;
	TheMap.Tileset.HumanWallTable[ 2] = 0x810;
	TheMap.Tileset.HumanWallTable[ 3] = 0x850;
	TheMap.Tileset.HumanWallTable[ 4] = 0x800;
	TheMap.Tileset.HumanWallTable[ 5] = 0x840;
	TheMap.Tileset.HumanWallTable[ 6] = 0x820;
	TheMap.Tileset.HumanWallTable[ 7] = 0x860;
	TheMap.Tileset.HumanWallTable[ 8] = 0x870;
	TheMap.Tileset.HumanWallTable[ 9] = 0x8B0;
	TheMap.Tileset.HumanWallTable[10] = 0x890;
	TheMap.Tileset.HumanWallTable[11] = 0x8D0;
	TheMap.Tileset.HumanWallTable[12] = 0x880;
	TheMap.Tileset.HumanWallTable[13] = 0x8C0;
	TheMap.Tileset.HumanWallTable[14] = 0x8A0;
	TheMap.Tileset.HumanWallTable[15] = 0x0B0;

	TheMap.Tileset.OrcWallTable[ 0] = 0x0A0;
	TheMap.Tileset.OrcWallTable[ 1] = 0x930;
	TheMap.Tileset.OrcWallTable[ 2] = 0x910;
	TheMap.Tileset.OrcWallTable[ 3] = 0x950;
	TheMap.Tileset.OrcWallTable[ 4] = 0x900;
	TheMap.Tileset.OrcWallTable[ 5] = 0x940;
	TheMap.Tileset.OrcWallTable[ 6] = 0x920;
	TheMap.Tileset.OrcWallTable[ 7] = 0x960;
	TheMap.Tileset.OrcWallTable[ 8] = 0x970;
	TheMap.Tileset.OrcWallTable[ 9] = 0x9B0;
	TheMap.Tileset.OrcWallTable[10] = 0x990;
	TheMap.Tileset.OrcWallTable[11] = 0x9D0;
	TheMap.Tileset.OrcWallTable[12] = 0x980;
	TheMap.Tileset.OrcWallTable[13] = 0x9C0;
	TheMap.Tileset.OrcWallTable[14] = 0x9A0;
	TheMap.Tileset.OrcWallTable[15] = 0x0C0;

	// Set destroyed walls to TileTypeUnknown
	for (i = 0; i < 16; ++i) {
		n = 0;
		tile = TheMap.Tileset.HumanWallTable[i];
		while (TheMap.Tileset.Table[tile]) { // Skip good tiles
			++tile;
			++n;
		}
		while (!TheMap.Tileset.Table[tile]) { // Skip separator
			++tile;
			++n;
		}
		while (TheMap.Tileset.Table[tile]) { // Skip good tiles
			++tile;
			++n;
		}
		while (!TheMap.Tileset.Table[tile]) { // Skip separator
			++tile;
			++n;
		}
		while (n < 16 && TheMap.Tileset.Table[tile]) {
			TheMap.Tileset.TileTypeTable[
				TheMap.Tileset.Table[tile]] = TileTypeUnknown;
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
static int CclSetTileFlags(lua_State* l)
{
	int j;
	int tilenumber;
	int flags;

	if (lua_gettop(l) < 2) {
		LuaError(l, "No flags defined");
	}

	tilenumber = LuaToNumber(l, 1);
	j = 0;
	flags = 0;
	ParseTilesetTileFlags(l, &flags, &j);
	TheMap.Tileset.FlagsTable[tilenumber] = flags;

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
