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
#include "ui.h"
#include "video.h"

/*----------------------------------------------------------------------------
--  Functions
----------------------------------------------------------------------------*/

/**
**  Extend tables of the tileset.
**
**  @param tileset  Tileset to be extended.
**  @param newsize  New total number of tiles.
*/
static void ExtendTilesetTables(CTileset *tileset, unsigned int newsize)
{
	tileset->Table.resize(newsize);
	tileset->FlagsTable.resize(newsize);
	tileset->Tiles.resize(newsize);
}

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
static void ParseTilesetTileFlags(lua_State *l, int *back, int *j)
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
		lua_rawgeti(l, -1, j + 1);
		const char *value = LuaToString(l, -1);
		lua_pop(l, 1);

		if (!strcmp(value, "top-one-tree")) {
			++j;
			lua_rawgeti(l, -1, j + 1);
			topOneTreeTile = LuaToNumber(l, -1);
			lua_pop(l, 1);
		} else if (!strcmp(value, "mid-one-tree")) {
			++j;
			lua_rawgeti(l, -1, j + 1);
			midOneTreeTile = LuaToNumber(l, -1);
			lua_pop(l, 1);
		} else if (!strcmp(value, "bot-one-tree")) {
			++j;
			lua_rawgeti(l, -1, j + 1);
			botOneTreeTile = LuaToNumber(l, -1);
			lua_pop(l, 1);
		} else if (!strcmp(value, "removed-tree")) {
			++j;
			lua_rawgeti(l, -1, j + 1);
			removedTreeTile = LuaToNumber(l, -1);
			lua_pop(l, 1);
		} else if (!strcmp(value, "growing-tree")) {
			++j;
			// keep for retro compatibility.
			// TODO : remove when game data are updated.
		} else if (!strcmp(value, "top-one-rock")) {
			++j;
			lua_rawgeti(l, -1, j + 1);
			topOneRockTile = LuaToNumber(l, -1);
			lua_pop(l, 1);
		} else if (!strcmp(value, "mid-one-rock")) {
			++j;
			lua_rawgeti(l, -1, j + 1);
			midOneRockTile = LuaToNumber(l, -1);
			lua_pop(l, 1);
		} else if (!strcmp(value, "bot-one-rock")) {
			++j;
			lua_rawgeti(l, -1, j + 1);
			botOneRockTile = LuaToNumber(l, -1);
			lua_pop(l, 1);
		} else if (!strcmp(value, "removed-rock")) {
			++j;
			lua_rawgeti(l, -1, j + 1);
			removedRockTile = LuaToNumber(l, -1);
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
*/
void CTileset::parseSolid(lua_State *l)
{
	const int index = NumTiles;
	int j = 0;

	ExtendTilesetTables(this, index + 16);
	if (!lua_istable(l, -1)) {
		LuaError(l, "incorrect argument");
	}

	lua_rawgeti(l, -1, j + 1);
	++j;
	const int basic_name = getOrAddSolidTileIndexByName(LuaToString(l, -1));
	lua_pop(l, 1);

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
			FlagsTable[index + j] = tile_flag;
			continue;
		}
		const int pud = LuaToNumber(l, -1);
		lua_pop(l, 1);

		// ugly hack for sc tilesets, remove when fixed
		if (j > 15) {
			ExtendTilesetTables(this, index + j);
		}
		Table[index + j] = pud;
		FlagsTable[index + j] = f;
		Tiles[index + j].BaseTerrain = basic_name;
		Tiles[index + j].MixTerrain = 0;
	}
	lua_pop(l, 1);

	for (int i = j; i < 16; ++i) {
		Table[index + i] = 0;
		FlagsTable[index + i] = 0;
		Tiles[index + i].BaseTerrain = 0;
		Tiles[index + i].MixTerrain = 0;
	}
	NumTiles = index + std::max(16, j);
}

/**
**  Parse the mixed slot part of a tileset definition
**
**  @param l        Lua state.
*/
void CTileset::parseMixed(lua_State *l)
{
	int index = NumTiles;
	const int new_index = index + 256;
	NumTiles = new_index;
	ExtendTilesetTables(this, index + 256);

	if (!lua_istable(l, -1)) {
		LuaError(l, "incorrect argument");
	}
	int j = 0;
	const int args = lua_rawlen(l, -1);
	lua_rawgeti(l, -1, j + 1);
	++j;
	const int basic_name = getOrAddSolidTileIndexByName(LuaToString(l, -1));;
	lua_pop(l, 1);
	lua_rawgeti(l, -1, j + 1);
	++j;
	const int mixed_name = getOrAddSolidTileIndexByName(LuaToString(l, -1));;
	lua_pop(l, 1);

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
			lua_rawgeti(l, -1, i + 1);
			const int pud = LuaToNumber(l, -1);
			lua_pop(l, 1);
			Table[index + i] = pud;
			FlagsTable[index + i] = f;
			Tiles[index + i].BaseTerrain = basic_name;
			Tiles[index + i].MixTerrain = mixed_name;
		}
		// Fill missing slots
		for (int i = len; i < 16; ++i) {
			Table[index + i] = 0;
			FlagsTable[index + i] = 0;
			Tiles[index + i].BaseTerrain = 0;
			Tiles[index + i].MixTerrain = 0;
		}
		index += 16;
		lua_pop(l, 1);
	}
	while (index < new_index) {
		Table[index] = 0;
		FlagsTable[index] = 0;
		Tiles[index].BaseTerrain = 0;
		Tiles[index].MixTerrain = 0;
		++index;
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
	NumTiles = 0;

	//  Parse the list: (still everything could be changed!)
	const int args = lua_rawlen(l, t);
	for (int j = 0; j < args; ++j) {
		lua_rawgeti(l, t, j + 1);
		const char *value = LuaToString(l, -1);
		lua_pop(l, 1);
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

/**
**  Define tileset
**
**  @param l  Lua state.
*/
static int CclDefineTileset(lua_State *l)
{
	Map.Tileset->parse(l);

	//  Load and prepare the tileset
	PixelTileSize = Map.Tileset->getPixelTileSize();

	ShowLoadProgress("Tileset `%s'", Map.Tileset->ImageFile.c_str());
	Map.TileGraphic = CGraphic::New(Map.Tileset->ImageFile, PixelTileSize.x, PixelTileSize.y);
	Map.TileGraphic->Load();
	return 0;
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

/**
** Build tileset tables like humanWallTable or mixedLookupTable
**
** Called after DefineTileset and only for tilesets that have wall,
** trees and rocks. This function will be deleted when removing
** support of walls and alike in the tileset.
*/
static int CclBuildTilesetTables(lua_State *l)
{
	LuaCheckArgs(l, 0);

	Map.Tileset->buildTable(l);
	return 0;
}

void CTileset::buildTable(lua_State *l)
{
	//  Calculate number of tiles in graphic tile
	const int n = NumTiles;

	mixedLookupTable.clear();
	mixedLookupTable.resize(n, 0);
	//  Build the TileTypeTable
	TileTypeTable.resize(n, 0);

	const std::vector<unsigned short> &table = Table;
	for (int i = 0; i < n; ++i) {
		const int tile = table[i];
		if (tile == 0) {
			continue;
		}
		unsigned flags = FlagsTable[i];
		if (flags & MapFieldWaterAllowed) {
			TileTypeTable[tile] = TileTypeWater;
		} else if (flags & MapFieldCoastAllowed) {
			TileTypeTable[tile] = TileTypeCoast;
		} else if (flags & MapFieldWall) {
			if (flags & MapFieldHuman) {
				TileTypeTable[tile] = TileTypeHumanWall;
			} else {
				TileTypeTable[tile] = TileTypeOrcWall;
			}
		} else if (flags & MapFieldRocks) {
			TileTypeTable[tile] = TileTypeRock;
		} else if (flags & MapFieldForest) {
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
		if (Tiles[i].BaseTerrain && Tiles[i].MixTerrain) {
			if (FlagsTable[i] & MapFieldForest) {
				mixed = i;
			}
			i += 256;
		} else {
			if (Tiles[i].BaseTerrain != 0 && Tiles[i].MixTerrain == 0) {
				if (FlagsTable[i] & MapFieldForest) {
					solid = i;
				}
			}
			i += 16;
		}
	}
	woodTable[ 0] = -1;
	woodTable[ 1] = table[mixed + 0x30];
	woodTable[ 2] = table[mixed + 0x70];
	woodTable[ 3] = table[mixed + 0xB0];
	woodTable[ 4] = table[mixed + 0x10];
	woodTable[ 5] = table[mixed + 0x50];
	woodTable[ 6] = table[mixed + 0x90];
	woodTable[ 7] = table[mixed + 0xD0];
	woodTable[ 8] = table[mixed + 0x00];
	woodTable[ 9] = table[mixed + 0x40];
	woodTable[10] = table[mixed + 0x80];
	woodTable[11] = table[mixed + 0xC0];
	woodTable[12] = table[mixed + 0x20];
	woodTable[13] = table[mixed + 0x60];
	woodTable[14] = table[mixed + 0xA0];
	woodTable[15] = table[solid];
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
		mixedLookupTable[table[i]] = 15;
	}
	for (int i = mixed; i < mixed + 256; ++i) {
		int check = (int)((i - mixed) / 16);

		switch (check) {
			case 0: mixedLookupTable[table[i]] = 8; break;
			case 1: mixedLookupTable[table[i]] = 4; break;
			case 2: mixedLookupTable[table[i]] = 8 + 4; break;
			case 3: mixedLookupTable[table[i]] = 1; break;
			case 4: mixedLookupTable[table[i]] = 8 + 1; break;
			case 5: mixedLookupTable[table[i]] = 4 + 1; break;
			case 6: mixedLookupTable[table[i]] = 8 + 4 + 1; break;
			case 7: mixedLookupTable[table[i]] = 2; break;
			case 8: mixedLookupTable[table[i]] = 8 + 2; break;
			case 9: mixedLookupTable[table[i]] = 4 + 2; break;
			case 10: mixedLookupTable[table[i]] = 8 + 4 + 2; break;
			case 11: mixedLookupTable[table[i]] = 2 + 1; break;
			case 12: mixedLookupTable[table[i]] = 8 + 2 + 1; break;
			case 13:  mixedLookupTable[table[i]] = 4 + 2 + 1; break;
			default: mixedLookupTable[table[i]] = 0; break;
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
		if (Tiles[i].BaseTerrain && Tiles[i].MixTerrain) {
			if (FlagsTable[i] & MapFieldRocks) {
				mixed = i;
			}
			i += 256;
		} else {
			if (Tiles[i].BaseTerrain != 0 && Tiles[i].MixTerrain == 0) {
				if (FlagsTable[i] & MapFieldRocks) {
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
		mixedLookupTable[table[i]] = 15;
	}
	for (int i = mixed; i < mixed + 256; ++i) {
		int check = (int)((i - mixed) / 16);
		switch (check) {
			case 0: mixedLookupTable[table[i]] = 8; break;
			case 1: mixedLookupTable[table[i]] = 4; break;
			case 2: mixedLookupTable[table[i]] = 8 + 4; break;
			case 3: mixedLookupTable[table[i]] = 1; break;
			case 4: mixedLookupTable[table[i]] = 8 + 1; break;
			case 5: mixedLookupTable[table[i]] = 4 + 1; break;
			case 6: mixedLookupTable[table[i]] = 8 + 4 + 1; break;
			case 7: mixedLookupTable[table[i]] = 2; break;
			case 8: mixedLookupTable[table[i]] = 8 + 2; break;
			case 9: mixedLookupTable[table[i]] = 4 + 2; break;
			case 10: mixedLookupTable[table[i]] = 8 + 4 + 2; break;
			case 11: mixedLookupTable[table[i]] = 2 + 1; break;
			case 12: mixedLookupTable[table[i]] = 8 + 2 + 1; break;
			case 13: mixedLookupTable[table[i]] = 4 + 2 + 1; break;
			default: mixedLookupTable[table[i]] = 0; break;
		}
	}

	mixedLookupTable[botOneRockTile] = 12 + 16;
	mixedLookupTable[topOneRockTile] = 3 + 32;
	mixedLookupTable[midOneRockTile] = 15 + 48;

	rockTable[ 0] = -1;
	rockTable[ 1] = table[mixed + 0x30];
	rockTable[ 2] = table[mixed + 0x70];
	rockTable[ 3] = table[mixed + 0xB0];
	rockTable[ 4] = table[mixed + 0x10];
	rockTable[ 5] = table[mixed + 0x50];
	rockTable[ 6] = table[mixed + 0x90];
	rockTable[ 7] = table[mixed + 0xD0];
	rockTable[ 8] = table[mixed + 0x00];
	rockTable[ 9] = table[mixed + 0x40];
	rockTable[10] = table[mixed + 0x80];
	rockTable[11] = table[mixed + 0xC0];
	rockTable[12] = table[mixed + 0x20];
	rockTable[13] = table[mixed + 0x60];
	rockTable[14] = table[mixed + 0xA0];
	rockTable[15] = table[solid];
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
		unsigned int tile = humanWallTable[i];
		while (Table[tile]) { // Skip good tiles
			++tile;
			++n;
		}
		while (!Table[tile]) { // Skip separator
			++tile;
			++n;
		}
		while (Table[tile]) { // Skip good tiles
			++tile;
			++n;
		}
		while (!Table[tile]) { // Skip separator
			++tile;
			++n;
		}
		while (n < 16 && Table[tile]) {
			TileTypeTable[Table[tile]] = TileTypeUnknown;
			++tile;
			++n;
		}
	}
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
