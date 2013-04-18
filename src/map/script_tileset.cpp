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
			TopOneTree = LuaToNumber(l, -1);
			lua_pop(l, 1);
		} else if (!strcmp(value, "mid-one-tree")) {
			++j;
			lua_rawgeti(l, -1, j + 1);
			MidOneTree = LuaToNumber(l, -1);
			lua_pop(l, 1);
		} else if (!strcmp(value, "bot-one-tree")) {
			++j;
			lua_rawgeti(l, -1, j + 1);
			BotOneTree = LuaToNumber(l, -1);
			lua_pop(l, 1);
		} else if (!strcmp(value, "removed-tree")) {
			++j;
			lua_rawgeti(l, -1, j + 1);
			RemovedTree = LuaToNumber(l, -1);
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
				GrowingTree[i] = LuaToNumber(l, -1);
				lua_pop(l, 1);
			}
			lua_pop(l, 1);
		} else if (!strcmp(value, "top-one-rock")) {
			++j;
			lua_rawgeti(l, -1, j + 1);
			TopOneRock = LuaToNumber(l, -1);
			lua_pop(l, 1);
		} else if (!strcmp(value, "mid-one-rock")) {
			++j;
			lua_rawgeti(l, -1, j + 1);
			MidOneRock = LuaToNumber(l, -1);
			lua_pop(l, 1);
		} else if (!strcmp(value, "bot-one-rock")) {
			++j;
			lua_rawgeti(l, -1, j + 1);
			BotOneRock = LuaToNumber(l, -1);
			lua_pop(l, 1);
		} else if (!strcmp(value, "removed-rock")) {
			++j;
			lua_rawgeti(l, -1, j + 1);
			RemovedRock = LuaToNumber(l, -1);
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
	PixelTileSize = Map.Tileset->PixelTileSize;

	ShowLoadProgress("Tileset `%s'", Map.Tileset->ImageFile.c_str());
	Map.TileGraphic = CGraphic::New(Map.Tileset->ImageFile, PixelTileSize.x, PixelTileSize.y);
	Map.TileGraphic->Load();
	return 0;
}


void CTileset::parse(lua_State *l)
{
	Clear();

	this->PixelTileSize.x = 32;
	this->PixelTileSize.y = 32;

	const int args = lua_gettop(l);
	for (int j = 1; j < args; ++j) {
		const char *value = LuaToString(l, j);
		++j;

		if (!strcmp(value, "name")) {
			this->Name = LuaToString(l, j);
		} else if (!strcmp(value, "image")) {
			this->ImageFile = LuaToString(l, j);
		} else if (!strcmp(value, "size")) {
			CclGetPos(l, &this->PixelTileSize.x, &this->PixelTileSize.x, j);
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
** Build tileset tables like HumanWallTable or MixedLookupTable
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

	MixedLookupTable.resize(n, 0);
	//  Build the TileTypeTable
	TileTypeTable.resize(n, 0);

	const std::vector<unsigned short> &table = Table;
	for (int i = 0; i < n; ++i) {
		const int tile = table[i];
		if (tile == 0) {
			continue;
		}
		//Initialize all Lookup Items to zero
		MixedLookupTable[tile] = 0;

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
	if (TopOneTree) {
		TileTypeTable[TopOneTree] = TileTypeWood;
	}
	if (MidOneTree) {
		TileTypeTable[MidOneTree] = TileTypeWood;
	}
	if (BotOneTree) {
		TileTypeTable[BotOneTree] = TileTypeWood;
	}
	if (TopOneRock) {
		TileTypeTable[TopOneRock] = TileTypeRock;
	}
	if (MidOneRock) {
		TileTypeTable[MidOneRock] = TileTypeRock;
	}
	if (BotOneRock) {
		TileTypeTable[BotOneRock] = TileTypeRock;
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
	WoodTable[ 0] = -1;
	WoodTable[ 1] = table[mixed + 0x30];
	WoodTable[ 2] = table[mixed + 0x70];
	WoodTable[ 3] = table[mixed + 0xB0];
	WoodTable[ 4] = table[mixed + 0x10];
	WoodTable[ 5] = table[mixed + 0x50];
	WoodTable[ 6] = table[mixed + 0x90];
	WoodTable[ 7] = table[mixed + 0xD0];
	WoodTable[ 8] = table[mixed + 0x00];
	WoodTable[ 9] = table[mixed + 0x40];
	WoodTable[10] = table[mixed + 0x80];
	WoodTable[11] = table[mixed + 0xC0];
	WoodTable[12] = table[mixed + 0x20];
	WoodTable[13] = table[mixed + 0x60];
	WoodTable[14] = table[mixed + 0xA0];
	WoodTable[15] = table[solid];
	WoodTable[16] = -1;
	WoodTable[17] = BotOneTree;
	WoodTable[18] = TopOneTree;
	WoodTable[19] = MidOneTree;

	//Mark which corners of each tile has tree in it.
	//All corners for solid tiles. (Same for rocks)
	//1 Bottom Left
	//2 Bottom Right
	//4 Top Right
	//8 Top Left
	//16 Bottom Tree Tile
	//32 Top Tree Tile
	for (int i = solid; i < solid + 16; ++i) {
		MixedLookupTable[table[i]] = 15;
	}
	for (int i = mixed; i < mixed + 256; ++i) {
		int check = (int)((i - mixed) / 16);

		switch (check) {
			case 0: MixedLookupTable[table[i]] = 8; break;
			case 1: MixedLookupTable[table[i]] = 4; break;
			case 2: MixedLookupTable[table[i]] = 8 + 4; break;
			case 3: MixedLookupTable[table[i]] = 1; break;
			case 4: MixedLookupTable[table[i]] = 8 + 1; break;
			case 5: MixedLookupTable[table[i]] = 4 + 1; break;
			case 6: MixedLookupTable[table[i]] = 8 + 4 + 1; break;
			case 7: MixedLookupTable[table[i]] = 2; break;
			case 8: MixedLookupTable[table[i]] = 8 + 2; break;
			case 9: MixedLookupTable[table[i]] = 4 + 2; break;
			case 10: MixedLookupTable[table[i]] = 8 + 4 + 2; break;
			case 11: MixedLookupTable[table[i]] = 2 + 1; break;
			case 12: MixedLookupTable[table[i]] = 8 + 2 + 1; break;
			case 13:  MixedLookupTable[table[i]] = 4 + 2 + 1; break;
			default: MixedLookupTable[table[i]] = 0; break;
		}
	}
	//16 Bottom Tree Special
	//32 Top Tree Special
	//64 Mid tree special - differentiate with mixed tiles.
	MixedLookupTable[BotOneTree] = 12 + 16;
	MixedLookupTable[TopOneTree] = 3 + 32;
	MixedLookupTable[MidOneTree] = 15 + 48;

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
		MixedLookupTable[table[i]] = 15;
	}
	for (int i = mixed; i < mixed + 256; ++i) {
		int check = (int)((i - mixed) / 16);
		switch (check) {
			case 0: MixedLookupTable[table[i]] = 8; break;
			case 1: MixedLookupTable[table[i]] = 4; break;
			case 2: MixedLookupTable[table[i]] = 8 + 4; break;
			case 3: MixedLookupTable[table[i]] = 1; break;
			case 4: MixedLookupTable[table[i]] = 8 + 1; break;
			case 5: MixedLookupTable[table[i]] = 4 + 1; break;
			case 6: MixedLookupTable[table[i]] = 8 + 4 + 1; break;
			case 7: MixedLookupTable[table[i]] = 2; break;
			case 8: MixedLookupTable[table[i]] = 8 + 2; break;
			case 9: MixedLookupTable[table[i]] = 4 + 2; break;
			case 10: MixedLookupTable[table[i]] = 8 + 4 + 2; break;
			case 11: MixedLookupTable[table[i]] = 2 + 1; break;
			case 12: MixedLookupTable[table[i]] = 8 + 2 + 1; break;
			case 13: MixedLookupTable[table[i]] = 4 + 2 + 1; break;
			default: MixedLookupTable[table[i]] = 0; break;
		}
	}

	MixedLookupTable[BotOneRock] = 12 + 16;
	MixedLookupTable[TopOneRock] = 3 + 32;
	MixedLookupTable[MidOneRock] = 15 + 48;

	RockTable[ 0] = -1;
	RockTable[ 1] = table[mixed + 0x30];
	RockTable[ 2] = table[mixed + 0x70];
	RockTable[ 3] = table[mixed + 0xB0];
	RockTable[ 4] = table[mixed + 0x10];
	RockTable[ 5] = table[mixed + 0x50];
	RockTable[ 6] = table[mixed + 0x90];
	RockTable[ 7] = table[mixed + 0xD0];
	RockTable[ 8] = table[mixed + 0x00];
	RockTable[ 9] = table[mixed + 0x40];
	RockTable[10] = table[mixed + 0x80];
	RockTable[11] = table[mixed + 0xC0];
	RockTable[12] = table[mixed + 0x20];
	RockTable[13] = table[mixed + 0x60];
	RockTable[14] = table[mixed + 0xA0];
	RockTable[15] = table[solid];
	RockTable[16] = -1;
	RockTable[17] = BotOneRock;
	RockTable[18] = TopOneRock;
	RockTable[19] = MidOneRock;

	buildWallReplacementTable();
}

void CTileset::buildWallReplacementTable()
{
	// FIXME: Build wall replacement tables
	HumanWallTable[ 0] = 0x090;
	HumanWallTable[ 1] = 0x830;
	HumanWallTable[ 2] = 0x810;
	HumanWallTable[ 3] = 0x850;
	HumanWallTable[ 4] = 0x800;
	HumanWallTable[ 5] = 0x840;
	HumanWallTable[ 6] = 0x820;
	HumanWallTable[ 7] = 0x860;
	HumanWallTable[ 8] = 0x870;
	HumanWallTable[ 9] = 0x8B0;
	HumanWallTable[10] = 0x890;
	HumanWallTable[11] = 0x8D0;
	HumanWallTable[12] = 0x880;
	HumanWallTable[13] = 0x8C0;
	HumanWallTable[14] = 0x8A0;
	HumanWallTable[15] = 0x0B0;

	OrcWallTable[ 0] = 0x0A0;
	OrcWallTable[ 1] = 0x930;
	OrcWallTable[ 2] = 0x910;
	OrcWallTable[ 3] = 0x950;
	OrcWallTable[ 4] = 0x900;
	OrcWallTable[ 5] = 0x940;
	OrcWallTable[ 6] = 0x920;
	OrcWallTable[ 7] = 0x960;
	OrcWallTable[ 8] = 0x970;
	OrcWallTable[ 9] = 0x9B0;
	OrcWallTable[10] = 0x990;
	OrcWallTable[11] = 0x9D0;
	OrcWallTable[12] = 0x980;
	OrcWallTable[13] = 0x9C0;
	OrcWallTable[14] = 0x9A0;
	OrcWallTable[15] = 0x0C0;

	// Set destroyed walls to TileTypeUnknown
	for (int i = 0; i < 16; ++i) {
		int n = 0;
		unsigned int tile = HumanWallTable[i];
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
