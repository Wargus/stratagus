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
//      (c) Copyright 2000-2004 by Lutz Sammer and Jimmy Salmon
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
**  Define tileset mapping from original number to internal symbol
**
**  @param l  Lua state.
*/
static int CclDefineTilesetWcNames(lua_State* l)
{
	int i;
	int j;
	char** cp;

	// Free all old names
	if ((cp = TilesetWcNames)) {
		while (*cp) {
			free(*cp++);
		}
		free(TilesetWcNames);
	}

	//
	//  Get new table.
	//
	i = lua_gettop(l);
	TilesetWcNames = cp = malloc((i + 1) * sizeof(char*));
	if (!cp) {
		fprintf(stderr, "out of memory.\n");
		ExitFatal(-1);
	}

	for (j = 0; j < i; ++j) {
		*cp++ = strdup(LuaToString(l, j + 1));
	}
	*cp = NULL;

	return 0;
}

/**
**  Extend tables of the tileset.
**
**  @param tileset  Tileset to be extended.
**  @param tiles    Number of tiles.
*/
static void ExtendTilesetTables(Tileset* tileset, int tiles)
{
	tileset->Table = realloc(tileset->Table, tiles * sizeof(*tileset->Table));
	if (!tileset->Table) {
		fprintf(stderr, "out of memory.\n");
		ExitFatal(-1);
	}
	tileset->FlagsTable =
		realloc(tileset->FlagsTable, tiles * sizeof(*tileset->FlagsTable));
	if (!tileset->FlagsTable) {
		fprintf(stderr, "out of memory.\n");
		ExitFatal(-1);
	}
	tileset->Tiles = realloc(tileset->Tiles,
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
	tileset->SolidTerrainTypes = realloc(tileset->SolidTerrainTypes,
		++tileset->NumTerrainTypes * sizeof(*tileset->SolidTerrainTypes));
	tileset->SolidTerrainTypes[i].TerrainName = ident;

	return i;
}

/**
**  Parse the flag section of a tile definition.
**
**  @param l     Lua state.
**  @param back  pointer for the flags (return).
**  @param j     FIXME: docu
**
**  @return      remaining list
*/
static void ParseTilesetTileFlags(lua_State* l, int* back, int* j)
{
	int flags;
	const char* value;

	//
	//  Parse the list: flags of the slot
	//
	flags = 0;
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
	SolidTerrainInfo* tt; // short for terrain type.
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
	tt = tileset->SolidTerrainTypes + basic_name;

	ParseTilesetTileFlags(l, &f, &j);

	//
	//  Vector: the tiles.
	//
	lua_rawgeti(l, -1, j + 1);
	if (!lua_istable(l, -1)) {
		LuaError(l, "incorrect argument");
	}
	tt->NumSolidTiles = len = luaL_getn(l, -1);

	// hack for sc tilesets, remove when fixed
	if (len > 16) {
		ExtendTilesetTables(tileset, index + len);
	}

	for (i = 0; i < len; ++i) {
		lua_rawgeti(l, -1, i + 1);
		tileset->Table[index + i] = LuaToNumber(l, -1);
//		tt->SolidTiles[i] = tileset->Table[index + i] = LuaToNumber(l, -1);
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
	tileset->Table = malloc(16 * sizeof(*tileset->Table));
	if (!tileset->Table) {
		fprintf(stderr, "out of memory.\n");
		ExitFatal(-1);
	}
	tileset->FlagsTable =
		malloc(16 * sizeof(*tileset->FlagsTable));
	if (!tileset->FlagsTable) {
		fprintf(stderr, "out of memory.\n");
		ExitFatal(-1);
	}
	tileset->Tiles = malloc(16 * sizeof(TileInfo));
	if (!tileset->Tiles) {
		fprintf(stderr, "out of memory.\n");
		ExitFatal(-1);
	}
	tileset->SolidTerrainTypes = malloc(sizeof(SolidTerrainInfo));
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
**  Parse the item mapping part of a tileset definition
**
**  @param l        Lua state.
**  @param tileset  Tileset to be filled.
**  @param t        FIXME: docu
*/
static void DefineTilesetParseItemMapping(lua_State* l, Tileset* tileset, int t)
{
	int num;
	char* unit;
	char buf[30];
	char** h;
	int args;
	int j;

	args = luaL_getn(l, t);
	for (j = 0; j < args; ++j) {
		lua_rawgeti(l, -1, j + 1);
		num = LuaToNumber(l, -1);
		lua_pop(l, 1);
		++j;
		lua_rawgeti(l, -1, j + 1);
		unit = strdup(LuaToString(l, -1));
		lua_pop(l, 1);
		sprintf(buf, "%d", num);
		if ((h = (char**)hash_find(tileset->ItemsHash, buf)) != NULL) {
			free(*h);
		}
		*(char**)hash_add(tileset->ItemsHash, buf) = unit;
	}
}

/**
**  Define tileset
**
**  @param l  Lua state.
*/
static int CclDefineTileset(lua_State* l)
{
	const char* value;
	int type;
	Tileset* tileset;
	char* ident;
	int args;
	int j;

	ident = strdup(LuaToString(l, 1));

	//
	//  Find the tile set.
	//
	if (Tilesets) {
		for (type = 0; type < NumTilesets; ++type) {
			if(!strcmp(Tilesets[type]->Ident, ident)) {
				free(Tilesets[type]->Ident);
				free(Tilesets[type]->File);
				free(Tilesets[type]->Class);
				free(Tilesets[type]->Name);
				free(Tilesets[type]->ImageFile);
				free(Tilesets[type]->Table);
				free(Tilesets[type]->Tiles);
				free(Tilesets[type]->TileTypeTable);
				free(Tilesets[type]->AnimationTable);
				free(Tilesets[type]);
				break;
			}
		}
		if (type == NumTilesets) {
			Tilesets = realloc(Tilesets, ++NumTilesets * sizeof(*Tilesets));
		}
	} else {
		Tilesets = malloc(sizeof(*Tilesets));
		type = 0;
		++NumTilesets;
	}
	if (!Tilesets) {
		fprintf(stderr, "out of memory.\n");
		ExitFatal(-1);
	}
	Tilesets[type] = tileset = calloc(sizeof(Tileset), 1);
	if (!tileset) {
		fprintf(stderr, "out of memory.\n");
		ExitFatal(-1);
	}
	Tilesets[type]->Ident = ident;
	Tilesets[type]->TileSizeX = 32;
	Tilesets[type]->TileSizeY = 32;

	//
	//  Parse the list: (still everything could be changed!)
	//
	args = lua_gettop(l);
	for (j = 1; j < args; ++j) {
		value = LuaToString(l, j + 1);
		++j;

		if (!strcmp(value, "file")) {
			tileset->File = strdup(LuaToString(l, j + 1));
		} else if (!strcmp(value, "class")) {
			tileset->Class = strdup(LuaToString(l, j + 1));
		} else if (!strcmp(value, "name")) {
			tileset->Name = strdup(LuaToString(l, j + 1));
		} else if (!strcmp(value, "image")) {
			tileset->ImageFile = strdup(LuaToString(l, j + 1));
		} else if (!strcmp(value, "size")) {
			if (!lua_istable(l, j + 1)) {
				LuaError(l, "incorrect argument");
			}
			lua_rawgeti(l, j + 1, 1);
			tileset->TileSizeX = LuaToNumber(l, -1);
			lua_pop(l, 1);
			lua_rawgeti(l, j + 1, 2);
			tileset->TileSizeY = LuaToNumber(l, -1);
			lua_pop(l, 1);
		} else if (!strcmp(value, "slots")) {
			if (!lua_istable(l, j + 1)) {
				LuaError(l, "incorrect argument");
			}
			DefineTilesetParseSlot(l, tileset, j + 1);
		} else if (!strcmp(value, "animations")) {
			DebugPrint("Animations not supported.\n");
		} else if (!strcmp(value, "objects")) {
			DebugPrint("Objects not supported.\n");
		} else if (!strcmp(value, "item-mapping")) {
			if (!lua_istable(l, j + 1)) {
				LuaError(l, "incorrect argument");
			}
			DefineTilesetParseItemMapping(l, tileset, j + 1);
		} else {
			LuaError(l, "Unsupported tag: %s" _C_ value);
		}
	}
	return 0;
}

/**
**  Register CCL features for tileset.
*/
void TilesetCclRegister(void)
{
	lua_register(Lua, "DefineTilesetWcNames", CclDefineTilesetWcNames);
	lua_register(Lua, "DefineTileset", CclDefineTileset);
}

//@}
