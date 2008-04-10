//     ____                _       __               
//    / __ )____  _____   | |     / /___ ___________
//   / __  / __ \/ ___/   | | /| / / __ `/ ___/ ___/
//  / /_/ / /_/ (__  )    | |/ |/ / /_/ / /  (__  ) 
// /_____/\____/____/     |__/|__/\__,_/_/  /____/  
//                                              
//       A futuristic real-time strategy game.
//          This file is part of Bos Wars.
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

#include <string.h>
#include <stdio.h>

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
	const char *ident;
	
	ident = LuaToString(l, -1);
	for (int i = 0; i < tileset->NumTerrainTypes; ++i) {
		if (!strcmp(ident, tileset->SolidTerrainTypes[i].TerrainName)) {
			return i;
		}
	}

	// Can't find it, then we add another solid terrain type.
	SolidTerrainInfo *s = new SolidTerrainInfo[tileset->NumTerrainTypes + 1];
	memcpy(s, tileset->SolidTerrainTypes, tileset->NumTerrainTypes * sizeof(SolidTerrainInfo));
	delete[] tileset->SolidTerrainTypes;
	tileset->SolidTerrainTypes = s;
	tileset->SolidTerrainTypes[tileset->NumTerrainTypes].TerrainName = new_strdup(ident);
	++tileset->NumTerrainTypes;
	
	return tileset->NumTerrainTypes - 1;
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
	for (int j = 0; j < args; ++j) {
		lua_rawgeti(l, t, j + 1);
		value = LuaToString(l, -1);
		lua_pop(l, 1);
		++j;

		//
		//  solid part
		//
		if (!strcmp(value, "solid")) {
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
	int args;

	Map.Tileset.Clear();

	Map.Tileset.TileSizeX = 32;
	Map.Tileset.TileSizeY = 32;

	//
	//  Parse the list: (still everything could be changed!)
	//
	args = lua_gettop(l);
	for (int j = 1; j < args; ++j) {
		const char *value = LuaToString(l, j);
		++j;

		if (!strcmp(value, "name")) {
			Map.Tileset.Name = LuaToString(l, j);
		} else if (!strcmp(value, "image")) {
			Map.Tileset.ImageFile = LuaToString(l, j);
		} else if (!strcmp(value, "imagemap")) {
			Map.Tileset.ImageMap = LuaToBoolean(l, j);
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
}

//@}
