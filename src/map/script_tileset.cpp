//       _________ __                 __                               
//      /   _____//  |_____________ _/  |______     ____  __ __  ______
//      \_____  \\   __\_  __ \__  \\   __\__  \   / ___\|  |  \/  ___/
//      /        \|  |  |  | \// __ \|  |  / __ \_/ /_/  >  |  /\___ |
//     /_______  /|__|  |__|  (____  /__| (____  /\___  /|____//____  >
//             \/                  \/          \//_____/            \/ 
//  ______________________                           ______________________
//			  T H E   W A R   B E G I N S
//	   Stratagus - A free fantasy real time strategy game engine
//
/**@name ccl_tileset.c	-	The tileset ccl functions. */
//
//	(c) Copyright 2000-2003 by Lutz Sammer and Jimmy Salmon
//
//      This program is free software; you can redistribute it and/or modify
//      it under the terms of the GNU General Public License as published by
//      the Free Software Foundation; version 2 dated June, 1991.
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
//	$Id$

//@{

/*----------------------------------------------------------------------------
--	Includes
----------------------------------------------------------------------------*/

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "stratagus.h"
#include "ccl.h"
#include "tileset.h"
#include "map.h"

/*----------------------------------------------------------------------------
--	Functions
----------------------------------------------------------------------------*/

/**
**	Define tileset mapping from original number to internal symbol
**
**	@param list	List of all names.
*/
#if defined(USE_GUILE) || defined(USE_SIOD)
local SCM CclDefineTilesetWcNames(SCM list)
{
    int i;
    char** cp;

    if ((cp = TilesetWcNames)) {	// Free all old names
	while (*cp) {
	    free(*cp++);
	}
	free(TilesetWcNames);
    }

    //
    //	Get new table.
    //
    i = gh_length(list);
    TilesetWcNames = cp = malloc((i + 1) * sizeof(char*));
    if (!cp) {
	fprintf(stderr, "out of memory.\n");
	ExitFatal(-1);
    }
    while (i--) {
	*cp++ = gh_scm2newstr(gh_car(list), NULL);
	list = gh_cdr(list);
    }
    *cp = NULL;

    return SCM_UNSPECIFIED;
}
#elif defined(USE_LUA)
local int CclDefineTilesetWcNames(lua_State* l)
{
    int i;
    int j;
    char** cp;

    if ((cp = TilesetWcNames)) {	// Free all old names
	while (*cp) {
	    free(*cp++);
	}
	free(TilesetWcNames);
    }

    //
    //	Get new table.
    //
    i = lua_gettop(l);
    TilesetWcNames = cp = malloc((i + 1) * sizeof(char*));
    if (!cp) {
	fprintf(stderr, "out of memory.\n");
	ExitFatal(-1);
    }

    for (j = 0; j < i; ++j) {
	if (!lua_isstring(l, j + 1)) {
	    lua_pushstring(l, "incorrect argument");
	    lua_error(l);
	}
	*cp++ = strdup(lua_tostring(l, j + 1));
    }
    *cp = NULL;

    return 0;
}
#endif

/**
**	Extend tables of the tileset.
**
**	@param tileset	Tileset to be extended.
**	@param tiles	Number of tiles.
*/
local void ExtendTilesetTables(Tileset* tileset, int tiles)
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
**	Parse the name field in tileset definition.
**
**	@param tileset	Tileset currently parsed.
**	@param list	List with name.
*/
#if defined(USE_GUILE) || defined(USE_SIOD)
local int TilesetParseName(Tileset* tileset, SCM list)
{
    char* ident;
    int i;

    ident = gh_scm2newstr(gh_car(list), NULL);
    for (i = 0; i < tileset->NumTerrainTypes; ++i) {
	if (!strcmp(ident, tileset->SolidTerrainTypes[i].TerrainName)) {
	    free(ident);
	    return i;
	}
    }

    //  Can't find it, then we add another solid terrain type.
    tileset->SolidTerrainTypes = realloc(tileset->SolidTerrainTypes,
	++tileset->NumTerrainTypes * sizeof(*tileset->SolidTerrainTypes));
    tileset->SolidTerrainTypes[i].TerrainName = ident;

    return i;
}
#elif defined(USE_LUA)
local int TilesetParseName(lua_State* l, Tileset* tileset)
{
    char* ident;
    int i;

    if (!lua_isstring(l, -1)) {
	lua_pushstring(l, "incorrect argument");
	lua_error(l);
    }
    ident = strdup(lua_tostring(l, -1));
    for (i = 0; i < tileset->NumTerrainTypes; ++i) {
	if (!strcmp(ident, tileset->SolidTerrainTypes[i].TerrainName)) {
	    free(ident);
	    return i;
	}
    }

    //  Can't find it, then we add another solid terrain type.
    tileset->SolidTerrainTypes = realloc(tileset->SolidTerrainTypes,
	++tileset->NumTerrainTypes * sizeof(*tileset->SolidTerrainTypes));
    tileset->SolidTerrainTypes[i].TerrainName = ident;

    return i;
}
#endif

/**
**	Parse the flag section of a tile definition.
**
**	@param list	list of flags.
**	@param back	pointer for the flags (return).
**
**	@return		remaining list
*/
#if defined(USE_GUILE) || defined(USE_SIOD)
local SCM ParseTilesetTileFlags(SCM list, int* back)
{
    int flags;

    //
    //  Parse the list: flags of the slot
    //
    flags = 0;
    while (!gh_null_p(list)) {
	SCM value;

	value = gh_car(list);

	if (!gh_symbol_p(value)) {
	    break;
	}
	list = gh_cdr(list);

	//
	//      Flags are only needed for the editor
	//
	if (gh_eq_p(value, gh_symbol2scm("water"))) {
	    flags |= MapFieldWaterAllowed;
	} else if (gh_eq_p(value, gh_symbol2scm("land"))) {
	    flags |= MapFieldLandAllowed;
	} else if (gh_eq_p(value, gh_symbol2scm("coast"))) {
	    flags |= MapFieldCoastAllowed;
	} else if (gh_eq_p(value, gh_symbol2scm("no-building"))) {
	    flags |= MapFieldNoBuilding;
	} else if (gh_eq_p(value, gh_symbol2scm("unpassable"))) {
	    flags |= MapFieldUnpassable;
	} else if (gh_eq_p(value, gh_symbol2scm("wall"))) {
	    flags |= MapFieldWall;
	} else if (gh_eq_p(value, gh_symbol2scm("rock"))) {
	    flags |= MapFieldRocks;
	} else if (gh_eq_p(value, gh_symbol2scm("forest"))) {
	    flags |= MapFieldForest;
	} else if (gh_eq_p(value, gh_symbol2scm("land-unit"))) {
	    flags |= MapFieldLandUnit;
	} else if (gh_eq_p(value, gh_symbol2scm("air-unit"))) {
	    flags |= MapFieldAirUnit;
	} else if (gh_eq_p(value, gh_symbol2scm("sea-unit"))) {
	    flags |= MapFieldSeaUnit;
	} else if (gh_eq_p(value, gh_symbol2scm("building"))) {
	    flags |= MapFieldBuilding;
	} else if (gh_eq_p(value, gh_symbol2scm("human"))) {
	    flags |= MapFieldHuman;
	} else {
	    errl("solid: unsupported tag", value);
	}
    }
    *back = flags;
    return list;
}
#elif defined(USE_LUA)
local void ParseTilesetTileFlags(lua_State* l, int* back, int* j)
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
	value = lua_tostring(l, -1);
	lua_pop(l, 1);

	//
	//      Flags are only needed for the editor
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
	    lua_pushfstring(l, "solid: unsupported tag: %s", value);
	    lua_error(l);
	}
    }
    *back = flags;
}
#endif

/**
**	Parse the special slot part of a tileset definition
**
**	@param tileset	Tileset to be filled.
**	@param list	Tagged list defining a special slot.
*/
#if defined(USE_GUILE) || defined(USE_SIOD)
local void DefineTilesetParseSpecial(Tileset* tileset, SCM list)
{
    SCM value;
    SCM data;
    int i;

    //
    //	Parse the list:	(still everything could be changed!)
    //
    while (!gh_null_p(list)) {
	value = gh_car(list);
	list = gh_cdr(list);
	data = gh_car(list);
	list = gh_cdr(list);

	//
	//	top-one-tree, mid-one-tree, bot-one-tree
	//
	if (gh_eq_p(value, gh_symbol2scm("top-one-tree"))) {
	    tileset->TopOneTree = gh_scm2int(data);
	} else if (gh_eq_p(value, gh_symbol2scm("mid-one-tree"))) {
	    tileset->MidOneTree = gh_scm2int(data);
	} else if (gh_eq_p(value, gh_symbol2scm("bot-one-tree"))) {
	    tileset->BotOneTree = gh_scm2int(data);
	//
	//	removed-tree
	//
	} else if (gh_eq_p(value, gh_symbol2scm("removed-tree"))) {
	    tileset->RemovedTree = gh_scm2int(data);
	//
	//	growing-tree
	//
	} else if (gh_eq_p(value, gh_symbol2scm("growing-tree"))) {
	    if (gh_vector_length(data) != 2) {
		errl("growing-tree: Wrong vector length", data);
	    }
	    for (i = 0; i < 2; ++i) {
		value = gh_vector_ref(data, gh_int2scm(i));
		tileset->GrowingTree[i] = gh_scm2int(value);
	    }

	//
	//	top-one-rock, mid-one-rock, bot-one-rock
	//
	} else if (gh_eq_p(value, gh_symbol2scm("top-one-rock"))) {
	    tileset->TopOneRock = gh_scm2int(data);
	} else if (gh_eq_p(value, gh_symbol2scm("mid-one-rock"))) {
	    tileset->MidOneRock = gh_scm2int(data);
	} else if (gh_eq_p(value, gh_symbol2scm("bot-one-rock"))) {
	    tileset->BotOneRock = gh_scm2int(data);
	//
	//	removed-rock
	//
	} else if (gh_eq_p(value, gh_symbol2scm("removed-rock"))) {
	    tileset->RemovedRock = gh_scm2int(data);
	} else {
	    errl("special: unsupported tag", value);
	}
    }
}
#elif defined(USE_LUA)
local void DefineTilesetParseSpecial(lua_State* l, Tileset* tileset)
{
    const char* value;
    int i;
    int args;
    int j;

    if (!lua_istable(l, -1)) {
	lua_pushstring(l, "incorrect argument");
	lua_error(l);
    }
    args = luaL_getn(l, -1);

    //
    //	Parse the list:	(still everything could be changed!)
    //
    for (j = 0; j < args; ++j) {
	lua_rawgeti(l, -1, j + 1);
	if (!lua_isstring(l, -1)) {
	    lua_pushstring(l, "incorrect argument");
	    lua_error(l);
	}
	value = lua_tostring(l, -1);
	lua_pop(l, 1);

	//
	//	top-one-tree, mid-one-tree, bot-one-tree
	//
	if (!strcmp(value, "top-one-tree")) {
	    ++j;
	    lua_rawgeti(l, -1, j + 1);
	    if (!lua_isnumber(l, -1)) {
		lua_pushstring(l, "incorrect argument");
		lua_error(l);
	    }
	    tileset->TopOneTree = lua_tonumber(l, -1);
	    lua_pop(l, 1);
	} else if (!strcmp(value, "mid-one-tree")) {
	    ++j;
	    lua_rawgeti(l, -1, j + 1);
	    if (!lua_isnumber(l, -1)) {
		lua_pushstring(l, "incorrect argument");
		lua_error(l);
	    }
	    tileset->MidOneTree = lua_tonumber(l, -1);
	    lua_pop(l, 1);
	} else if (!strcmp(value, "bot-one-tree")) {
	    ++j;
	    lua_rawgeti(l, -1, j + 1);
	    if (!lua_isnumber(l, -1)) {
		lua_pushstring(l, "incorrect argument");
		lua_error(l);
	    }
	    tileset->BotOneTree = lua_tonumber(l, -1);
	    lua_pop(l, 1);
	//
	//	removed-tree
	//
	} else if (!strcmp(value, "removed-tree")) {
	    ++j;
	    lua_rawgeti(l, -1, j + 1);
	    if (!lua_isnumber(l, -1)) {
		lua_pushstring(l, "incorrect argument");
		lua_error(l);
	    }
	    tileset->RemovedTree = lua_tonumber(l, -1);
	    lua_pop(l, 1);
	//
	//	growing-tree
	//
	} else if (!strcmp(value, "growing-tree")) {
	    ++j;
	    lua_rawgeti(l, -1, j + 1);
	    if (!lua_istable(l, -1)) {
		lua_pushstring(l, "incorrect argument");
		lua_error(l);
	    }
	    if (luaL_getn(l, -1) != 2) {
		lua_pushstring(l, "growing-tree: Wrong table length");
		lua_error(l);
	    }
	    for (i = 0; i < 2; ++i) {
		lua_rawgeti(l, -1, i + 1);
		if (!lua_isnumber(l, -1)) {
		    lua_pushstring(l, "incorrect argument");
		    lua_error(l);
		}
		tileset->GrowingTree[i] = lua_tonumber(l, -1);
		lua_pop(l, 1);
	    }
	    lua_pop(l, 1);

	//
	//	top-one-rock, mid-one-rock, bot-one-rock
	//
	} else if (!strcmp(value, "top-one-rock")) {
	    ++j;
	    lua_rawgeti(l, -1, j + 1);
	    if (!lua_isnumber(l, -1)) {
		lua_pushstring(l, "incorrect argument");
		lua_error(l);
	    }
	    tileset->TopOneRock = lua_tonumber(l, -1);
	    lua_pop(l, 1);
	} else if (!strcmp(value, "mid-one-rock")) {
	    ++j;
	    lua_rawgeti(l, -1, j + 1);
	    if (!lua_isnumber(l, -1)) {
		lua_pushstring(l, "incorrect argument");
		lua_error(l);
	    }
	    tileset->MidOneRock = lua_tonumber(l, -1);
	    lua_pop(l, 1);
	} else if (!strcmp(value, "bot-one-rock")) {
	    ++j;
	    lua_rawgeti(l, -1, j + 1);
	    if (!lua_isnumber(l, -1)) {
		lua_pushstring(l, "incorrect argument");
		lua_error(l);
	    }
	    tileset->BotOneRock = lua_tonumber(l, -1);
	    lua_pop(l, 1);
	//
	//	removed-rock
	//
	} else if (!strcmp(value, "removed-rock")) {
	    ++j;
	    lua_rawgeti(l, -1, j + 1);
	    if (!lua_isnumber(l, -1)) {
		lua_pushstring(l, "incorrect argument");
		lua_error(l);
	    }
	    tileset->RemovedRock = lua_tonumber(l, -1);
	    lua_pop(l, 1);
	} else {
	    lua_pushfstring(l, "special: unsupported tag: %s", value);
	    lua_error(l);
	}
    }
}
#endif

/**
**	Parse the solid slot part of a tileset definition
**
**	@param tileset	Tileset to be filled.
**	@param index	Current table index.
**	@param list	Tagged list defining a solid slot.
*/
#if defined(USE_GUILE) || defined(USE_SIOD)
local int DefineTilesetParseSolid(Tileset* tileset, int index, SCM list)
{
    SCM value;
    SCM data;
    int i;
    int f;
    int l;
    int basic_name;
    SolidTerrainInfo* tt;// short for terrain type.

    ExtendTilesetTables(tileset, index + 16);

    basic_name = TilesetParseName(tileset, list);	// base name
    tt = tileset->SolidTerrainTypes + basic_name;
    list = gh_cdr(list);

    list = ParseTilesetTileFlags(list, &f);

    //
    //	Vector: the tiles.
    //
    value = gh_car(list);
    tt->NumSolidTiles = l = gh_vector_length(value);

    // hack for sc tilesets, remove when fixed
    if (l > 16) {
	ExtendTilesetTables(tileset, index + l);
    }

    for (i = 0; i < l; ++i) {
	data = gh_vector_ref(value, gh_int2scm(i));
//	tt->SolidTiles[i] = tileset->Table[index + i] = gh_scm2int(data);
	tileset->Table[index + i] = gh_scm2int(data);
	tileset->FlagsTable[index + i] = f;
	tileset->Tiles[index + i].BaseTerrain = basic_name;
	tileset->Tiles[index + i].MixTerrain = 0;
    }
    while (i < 16) {
	tileset->Table[index + i] = 0;
	tileset->FlagsTable[index + i] = 0;
	tileset->Tiles[index + i].BaseTerrain = 0;
	tileset->Tiles[index + i].MixTerrain = 0;
	++i;
    }

    if (l < 16) {
	return index + 16;
    }
    return index + l;
}
#elif defined(USE_LUA)
local int DefineTilesetParseSolid(lua_State* l, Tileset* tileset, int index)
{
    int i;
    int f;
    int len;
    int basic_name;
    SolidTerrainInfo* tt;// short for terrain type.
    int j;

    ExtendTilesetTables(tileset, index + 16);

    if (!lua_istable(l, -1)) {
	lua_pushstring(l, "incorrect argument");
	lua_error(l);
    }
    j = 0;
    lua_rawgeti(l, -1, j + 1);
    ++j;
    basic_name = TilesetParseName(l, tileset);	// base name
    lua_pop(l, 1);
    tt = tileset->SolidTerrainTypes + basic_name;

    ParseTilesetTileFlags(l, &f, &j);

    //
    //	Vector: the tiles.
    //
    lua_rawgeti(l, -1, j + 1);
    if (!lua_istable(l, -1)) {
	lua_pushstring(l, "incorrect argument");
	lua_error(l);
    }
    tt->NumSolidTiles = len = luaL_getn(l, -1);

    // hack for sc tilesets, remove when fixed
    if (len > 16) {
	ExtendTilesetTables(tileset, index + len);
    }

    for (i = 0; i < len; ++i) {
	lua_rawgeti(l, -1, i + 1);
	if (!lua_isnumber(l, -1)) {
	    lua_pushstring(l, "incorrect argument");
	    lua_error(l);
	}
	tileset->Table[index + i] = lua_tonumber(l, -1);
//	tt->SolidTiles[i] = tileset->Table[index + i] = lua_tonumber(l, -1);
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
#endif

/**
**	Parse the mixed slot part of a tileset definition
**
**	@param tileset	Tileset to be filled.
**	@param index	Current table index.
**	@param list	Tagged list defining a mixed slot.
*/
#if defined(USE_GUILE) || defined(USE_SIOD)
local int DefineTilesetParseMixed(Tileset* tileset, int index, SCM list)
{
    SCM value;
    SCM data;
    int i;
    int l;
    int f;
    int basic_name;
    int mixed_name;
    int new_index;

    new_index = index + 256;
    ExtendTilesetTables(tileset, new_index);

    basic_name = TilesetParseName(tileset, list);	// base name
    list = gh_cdr(list);
    mixed_name = TilesetParseName(tileset, list);	// mixed name
    list = gh_cdr(list);

    list = ParseTilesetTileFlags(list, &f);

    //
    //	Parse the list:	slots FIXME: no error checking number of slots
    //
    while (!gh_null_p(list)) {
	value = gh_car(list);
	list = gh_cdr(list);

	//
	//	Vector: the tiles.
	//
	l = gh_vector_length(value);
	for (i = 0; i < l; ++i) {
	    data = gh_vector_ref(value, gh_int2scm(i));
	    tileset->Table[index + i] = gh_scm2int(data);
	    tileset->FlagsTable[index + i] = f;
	    tileset->Tiles[index + i].BaseTerrain = basic_name;
	    tileset->Tiles[index + i].MixTerrain = mixed_name;
	}
	while (i < 16) {			// Fill missing slots
	    tileset->Table[index + i] = 0;
	    tileset->FlagsTable[index + i] = 0;
	    tileset->Tiles[index + i].BaseTerrain = 0;
	    tileset->Tiles[index + i].MixTerrain = 0;
	    ++i;
	}
	index += 16;
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
#elif defined(USE_LUA)
local int DefineTilesetParseMixed(lua_State* l, Tileset* tileset, int index)
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
	lua_pushstring(l, "incorrect argument");
	lua_error(l);
    }
    j = 0;
    args = luaL_getn(l, -1);
    lua_rawgeti(l, -1, j + 1);
    ++j;
    basic_name = TilesetParseName(l, tileset);	// base name
    lua_pop(l, 1);
    lua_rawgeti(l, -1, j + 1);
    ++j;
    mixed_name = TilesetParseName(l, tileset);	// mixed name
    lua_pop(l, 1);

    ParseTilesetTileFlags(l, &f, &j);

    for (; j < args; ++j) {
	lua_rawgeti(l, -1, j + 1);
	if (!lua_istable(l, -1)) {
	    lua_pushstring(l, "incorrect argument");
	    lua_error(l);
	}
	//
	//	Vector: the tiles.
	//
	len = luaL_getn(l, -1);
	for (i = 0; i < len; ++i) {
	    lua_rawgeti(l, -1, i + 1);
	    if (!lua_isnumber(l, -1)) {
		lua_pushstring(l, "incorrect argument");
		lua_error(l);
	    }
	    tileset->Table[index + i] = lua_tonumber(l, -1);
	    tileset->FlagsTable[index + i] = f;
	    tileset->Tiles[index + i].BaseTerrain = basic_name;
	    tileset->Tiles[index + i].MixTerrain = mixed_name;
	    lua_pop(l, 1);
	}
	while (i < 16) {			// Fill missing slots
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
#endif

/**
**	Parse the slot part of a tileset definition
**
**	@param tileset	Tileset to be filled.
**	@param list	Tagged list defining a slot.
*/
#if defined(USE_GUILE) || defined(USE_SIOD)
local void DefineTilesetParseSlot(Tileset* tileset, SCM list)
{
    SCM value;
    SCM data;
    int index;

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
    //	Parse the list:	(still everything could be changed!)
    //
    while (!gh_null_p(list)) {
	value = gh_car(list);
	list = gh_cdr(list);
	data = gh_car(list);
	list = gh_cdr(list);

	//
	//	special part
	//
	if (gh_eq_p(value, gh_symbol2scm("special"))) {
	    DefineTilesetParseSpecial(tileset, data);
	//
	//	solid part
	//
	} else if (gh_eq_p(value, gh_symbol2scm("solid"))) {
	    index = DefineTilesetParseSolid(tileset, index, data);
	//
	//	mixed part
	//
	} else if (gh_eq_p(value, gh_symbol2scm("mixed"))) {
	    index = DefineTilesetParseMixed(tileset, index, data);
	} else {
	    errl("slots: unsupported tag", value);
	}
    }
    tileset->NumTiles = index;
}
#elif defined(USE_LUA)
local void DefineTilesetParseSlot(lua_State* l, Tileset* tileset, int t)
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
    //	Parse the list:	(still everything could be changed!)
    //
    args = luaL_getn(l, t);
    for (j = 0; j < args; ++j) {
	lua_rawgeti(l, t, j + 1);
	if (!lua_isstring(l, -1)) {
	    lua_pushstring(l, "incorrect argument");
	    lua_error(l);
	}
	value = lua_tostring(l, -1);
	lua_pop(l, 1);
	++j;

	//
	//	special part
	//
	if (!strcmp(value, "special")) {
	    lua_rawgeti(l, t, j + 1);
	    DefineTilesetParseSpecial(l, tileset);
	    lua_pop(l, 1);
	//
	//	solid part
	//
	} else if (!strcmp(value, "solid")) {
	    lua_rawgeti(l, t, j + 1);
	    index = DefineTilesetParseSolid(l, tileset, index);
	    lua_pop(l, 1);
	//
	//	mixed part
	//
	} else if (!strcmp(value, "mixed")) {
	    lua_rawgeti(l, t, j + 1);
	    index = DefineTilesetParseMixed(l, tileset, index);
	    lua_pop(l, 1);
	} else {
	    lua_pushfstring(l, "slots: unsupported tag: %s", value);
	    lua_error(l);
	}
    }
    tileset->NumTiles = index;
}
#endif

/**
**	Parse the item mapping part of a tileset definition
**
**	@param tileset	Tileset to be filled.
**	@param list	List defining item mapping.
*/
#if defined(USE_GUILE) || defined(USE_SIOD)
local void DefineTilesetParseItemMapping(Tileset* tileset, SCM list)
{
    SCM value;
    int num;
    char* unit;
    char buf[30];
    char** h;

    while (!gh_null_p(list)) {
	value = gh_car(list);
	list = gh_cdr(list);
	num = gh_scm2int(value);
	value = gh_car(list);
	list = gh_cdr(list);
	unit = gh_scm2newstr(value, 0);
	sprintf(buf, "%d", num);
	if ((h = (char**)hash_find(tileset->ItemsHash, buf)) != NULL) {
	    free(*h);
	}
	*(char**)hash_add(tileset->ItemsHash, buf) = unit;
    }
}
#elif defined(USE_LUA)
local void DefineTilesetParseItemMapping(lua_State* l, Tileset* tileset, int t)
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
	if (!lua_isnumber(l, -1)) {
	    lua_pushstring(l, "incorrect argument");
	    lua_error(l);
	}
	num = lua_tonumber(l, -1);
	lua_pop(l, 1);
	++j;
	lua_rawgeti(l, -1, j + 1);
	if (!lua_isstring(l, -1)) {
	    lua_pushstring(l, "incorrect argument");
	    lua_error(l);
	}
	unit = strdup(lua_tostring(l, -1));
	lua_pop(l, 1);
	sprintf(buf, "%d", num);
	if ((h = (char**)hash_find(tileset->ItemsHash, buf)) != NULL) {
	    free(*h);
	}
	*(char**)hash_add(tileset->ItemsHash, buf) = unit;
    }
}
#endif

/**
**	Define tileset
**
**	@param list	Tagged list defining a tileset.
*/
#if defined(USE_GUILE) || defined(USE_SIOD)
local SCM CclDefineTileset(SCM list)
{
    SCM value;
    SCM data;
    int type;
    Tileset* tileset;
    char* ident;

    value = gh_car(list);
    list = gh_cdr(list);

    if (!gh_symbol_p(value)) {
	errl("illegal tileset slot name", value);
    }
    ident = gh_scm2newstr(value, NULL);

    //
    //	Find the tile set.
    //
    if (Tilesets) {
	for (type = 0; type < NumTilesets; ++type) {
	    if(!strcmp(Tilesets[type]->Ident, ident)) {
		free(Tilesets[type]->Ident);
		free(Tilesets[type]->File);
		free(Tilesets[type]->Class);
		free(Tilesets[type]->Name);
		free(Tilesets[type]->ImageFile);
		free(Tilesets[type]->PaletteFile);
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
    //	Parse the list:	(still everything could be changed!)
    //
    while (!gh_null_p(list)) {

	value = gh_car(list);
	list = gh_cdr(list);
	data = gh_car(list);
	list = gh_cdr(list);

	if (gh_eq_p(value, gh_symbol2scm("file"))) {
	    tileset->File = gh_scm2newstr(data, NULL);
	} else if (gh_eq_p(value, gh_symbol2scm("class"))) {
	    tileset->Class = gh_scm2newstr(data, NULL);
	} else if (gh_eq_p(value, gh_symbol2scm("name"))) {
	    tileset->Name = gh_scm2newstr(data, NULL);
	} else if (gh_eq_p(value, gh_symbol2scm("image"))) {
	    tileset->ImageFile = gh_scm2newstr(data, NULL);
	} else if (gh_eq_p(value, gh_symbol2scm("palette"))) {
	    tileset->PaletteFile = gh_scm2newstr(data, NULL);
	} else if (gh_eq_p(value, gh_symbol2scm("size"))) {
	    tileset->TileSizeX = gh_scm2int(gh_car(data));
	    data = gh_cdr(data);
	    tileset->TileSizeY = gh_scm2int(gh_car(data));
	} else if (gh_eq_p(value, gh_symbol2scm("slots"))) {
	    DefineTilesetParseSlot(tileset, data);
	} else if (gh_eq_p(value, gh_symbol2scm("animations"))) {
	    DebugLevel0Fn("Animations not supported.\n");
	} else if (gh_eq_p(value, gh_symbol2scm("objects"))) {
	    DebugLevel0Fn("Objects not supported.\n");
	} else if (gh_eq_p(value, gh_symbol2scm("item-mapping"))) {
	    DefineTilesetParseItemMapping(tileset, data);
	} else {
	    errl("Unsupported tag", value);
	}
    }
    return list;
}
#elif defined(USE_LUA)
local int CclDefineTileset(lua_State* l)
{
    const char* value;
    int type;
    Tileset* tileset;
    char* ident;
    int args;
    int j;

    if (!lua_isstring(l, 1)) {
	lua_pushstring(l, "incorrect argument");
	lua_error(l);
    }
    ident = strdup(lua_tostring(l, 1));

    //
    //	Find the tile set.
    //
    if (Tilesets) {
	for (type = 0; type < NumTilesets; ++type) {
	    if(!strcmp(Tilesets[type]->Ident, ident)) {
		free(Tilesets[type]->Ident);
		free(Tilesets[type]->File);
		free(Tilesets[type]->Class);
		free(Tilesets[type]->Name);
		free(Tilesets[type]->ImageFile);
		free(Tilesets[type]->PaletteFile);
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
    //	Parse the list:	(still everything could be changed!)
    //
    args = lua_gettop(l);
    for (j = 1; j < args; ++j) {
	if (!lua_isstring(l, j + 1)) {
	    lua_pushstring(l, "incorrect argument");
	    lua_error(l);
	}
	value = lua_tostring(l, j + 1);
	++j;

	if (!strcmp(value, "file")) {
	    if (!lua_isstring(l, j + 1)) {
		lua_pushstring(l, "incorrect argument");
		lua_error(l);
	    }
	    tileset->File = strdup(lua_tostring(l, j + 1));
	} else if (!strcmp(value, "class")) {
	    if (!lua_isstring(l, j + 1)) {
		lua_pushstring(l, "incorrect argument");
		lua_error(l);
	    }
	    tileset->Class = strdup(lua_tostring(l, j + 1));
	} else if (!strcmp(value, "name")) {
	    if (!lua_isstring(l, j + 1)) {
		lua_pushstring(l, "incorrect argument");
		lua_error(l);
	    }
	    tileset->Name = strdup(lua_tostring(l, j + 1));
	} else if (!strcmp(value, "image")) {
	    if (!lua_isstring(l, j + 1)) {
		lua_pushstring(l, "incorrect argument");
		lua_error(l);
	    }
	    tileset->ImageFile = strdup(lua_tostring(l, j + 1));
	} else if (!strcmp(value, "palette")) {
	    if (!lua_isstring(l, j + 1)) {
		lua_pushstring(l, "incorrect argument");
		lua_error(l);
	    }
	    tileset->PaletteFile = strdup(lua_tostring(l, j + 1));
	} else if (!strcmp(value, "size")) {
	    if (!lua_istable(l, j + 1)) {
		lua_pushstring(l, "incorrect argument");
		lua_error(l);
	    }
	    lua_rawgeti(l, j + 1, 1);
	    if (!lua_isnumber(l, -1)) {
		lua_pushstring(l, "incorrect argument");
		lua_error(l);
	    }
	    tileset->TileSizeX = lua_tonumber(l, -1);
	    lua_pop(l, 1);
	    lua_rawgeti(l, j + 1, 2);
	    if (!lua_isnumber(l, -1)) {
		lua_pushstring(l, "incorrect argument");
		lua_error(l);
	    }
	    tileset->TileSizeY = lua_tonumber(l, -1);
	    lua_pop(l, 1);
	} else if (!strcmp(value, "slots")) {
	    if (!lua_istable(l, j + 1)) {
		lua_pushstring(l, "incorrect argument");
		lua_error(l);
	    }
	    DefineTilesetParseSlot(l, tileset, j + 1);
	} else if (!strcmp(value, "animations")) {
	    DebugLevel0Fn("Animations not supported.\n");
	} else if (!strcmp(value, "objects")) {
	    DebugLevel0Fn("Objects not supported.\n");
	} else if (!strcmp(value, "item-mapping")) {
	    if (!lua_istable(l, j + 1)) {
		lua_pushstring(l, "incorrect argument");
		lua_error(l);
	    }
	    DefineTilesetParseItemMapping(l, tileset, j + 1);
	} else {
	    lua_pushfstring(l, "Unsupported tag: %s", value);
	    lua_error(l);
	}
    }
    return 0;
}
#endif

/**
**	Register CCL features for tileset.
*/
global void TilesetCclRegister(void)
{
#if defined(USE_GUILE) || defined(USE_SIOD)
    gh_new_procedureN("define-tileset-wc-names", CclDefineTilesetWcNames);
    gh_new_procedureN("define-tileset", CclDefineTileset);
#elif defined(USE_LUA)
    lua_register(Lua, "DefineTilesetWcNames", CclDefineTilesetWcNames);
    lua_register(Lua, "DefineTileset", CclDefineTileset);
#endif
}

//@}
