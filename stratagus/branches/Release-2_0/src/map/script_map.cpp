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
/**@name ccl_map.c - The map ccl functions. */
//
//      (c) Copyright 1999-2004 by Lutz Sammer and Jimmy Salmon
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
#include "map.h"
#include "minimap.h"
#include "actions.h"
#include "campaign.h"
#include "ui.h"

/*----------------------------------------------------------------------------
--  Functions
----------------------------------------------------------------------------*/

/**
**  Parse a stratagus map.
**
**  @param list  list of tuples keyword data
*/
local int CclStratagusMap(lua_State* l)
{
	const char* value;
	int args;
	int j;
	int subargs;
	int k;

	//
	//  Parse the list: (still everything could be changed!)
	//

	if (!TheMap.Info) {
		TheMap.Info = calloc(1, sizeof(MapInfo));
	}

	args = lua_gettop(l);
	for (j = 0; j < args; ++j) {
		value = LuaToString(l, j + 1);
		++j;

		if (!strcmp(value, "version")) {
			char buf[32];

			value = LuaToString(l, j + 1);
			sprintf(buf, StratagusFormatString, StratagusFormatArgs(StratagusVersion));
			if (strcmp(buf, value)) {
				fprintf(stderr, "Warning not saved with this version.\n");
			}
		} else if (!strcmp(value, "uid")) {
			TheMap.Info->MapUID = LuaToNumber(l, j + 1);
		} else if (!strcmp(value, "description")) {
			value = LuaToString(l, j + 1);
			strncpy(TheMap.Description, value, sizeof(TheMap.Description));
			TheMap.Info->Description = strdup(value);
		} else if (!strcmp(value, "the-map")) {
			if (!lua_istable(l, j + 1)) {
				lua_pushstring(l, "incorrect argument");
				lua_error(l);
			}
			subargs = luaL_getn(l, j + 1);
			for (k = 0; k < subargs; ++k) {
				lua_rawgeti(l, j + 1, k + 1);
				value = LuaToString(l, -1);
				lua_pop(l, 1);
				++k;

				if (!strcmp(value, "terrain")) {
					int i;

					lua_rawgeti(l, j + 1, k + 1);
					if (!lua_istable(l, -1)) {
						lua_pushstring(l, "incorrect argument");
						lua_error(l);
					}
					lua_rawgeti(l, -1, 1);
					value = LuaToString(l, -1);
					lua_pop(l, 1);
					// ignore (l, -1, 2)
					lua_pop(l, 1);

					free(TheMap.TerrainName);
					TheMap.TerrainName = strdup(value);

					// Lookup the index of this tileset.
					for (i = 0; TilesetWcNames[i] &&
						strcmp(value, TilesetWcNames[i]); ++i) {
					}
					TheMap.Terrain = i;
					LoadTileset();
				} else if (!strcmp(value, "size")) {
					lua_rawgeti(l, j + 1, k + 1);
					if (!lua_istable(l, -1)) {
						lua_pushstring(l, "incorrect argument");
						lua_error(l);
					}
					lua_rawgeti(l, -1, 1);
					TheMap.Width = LuaToNumber(l, -1);
					lua_pop(l, 1);
					lua_rawgeti(l, -1, 2);
					TheMap.Height = LuaToNumber(l, -1);
					lua_pop(l, 1);
					lua_pop(l, 1);

					free(TheMap.Fields);
					TheMap.Fields = calloc(TheMap.Width * TheMap.Height,
						sizeof(*TheMap.Fields));
					TheMap.Visible[0] = calloc(TheMap.Width * TheMap.Height / 8, 1);
					InitUnitCache();
					// FIXME: this should be CreateMap or InitMap?
				} else if (!strcmp(value, "fog-of-war")) {
					TheMap.NoFogOfWar = 0;
					--k;
				} else if (!strcmp(value, "no-fog-of-war")) {
					TheMap.NoFogOfWar = 1;
					--k;
				} else if (!strcmp(value, "filename")) {
					 lua_rawgeti(l, j + 1, k + 1);
					TheMap.Info->Filename = strdup(LuaToString(l, -1));
					lua_pop(l, 1);
				} else if (!strcmp(value, "map-fields")) {
					int i;
					int subsubargs;
					int subk;

					lua_rawgeti(l, j + 1, k + 1);
					if (!lua_istable(l, -1)) {
						lua_pushstring(l, "incorrect argument");
						lua_error(l);
					}

					subsubargs = luaL_getn(l, -1);
					if (subsubargs != TheMap.Width * TheMap.Height) {
						fprintf(stderr, "Wrong tile table length: %d\n", subsubargs);
					}
					i = 0;
					for (subk = 0; subk < subsubargs; ++subk) {
						int args2;
						int j2;

						lua_rawgeti(l, -1, subk + 1);
						if (!lua_istable(l, -1)) {
							lua_pushstring(l, "incorrect argument");
							lua_error(l);
						}
						args2 = luaL_getn(l, -1);
						j2 = 0;

						lua_rawgeti(l, -1, j2 + 1);
						TheMap.Fields[i].Tile = LuaToNumber(l, -1);
						lua_pop(l, 1);
						++j2;
						lua_rawgeti(l, -1, j2 + 1);
						TheMap.Fields[i].SeenTile = LuaToNumber(l, -1);
						lua_pop(l, 1);
						++j2;
						for (; j2 < args2; ++j2) {
							lua_rawgeti(l, -1, j2 + 1);
							if (lua_isnumber(l, -1)) {
								TheMap.Fields[i].Value = LuaToNumber(l, -1);
								lua_pop(l, 1);
								continue;
							}
							value = LuaToString(l, -1);
							lua_pop(l, 1);
							if (!strcmp(value, "explored")) {
								++j2;
								lua_rawgeti(l, -1, j2 + 1);
								TheMap.Fields[i].Visible[(int)LuaToNumber(l, -1)] = 1;
								lua_pop(l, 1);
							} else if (!strcmp(value, "human")) {
								TheMap.Fields[i].Flags |= MapFieldHuman;

							} else if (!strcmp(value, "land")) {
								TheMap.Fields[i].Flags |= MapFieldLandAllowed;
							} else if (!strcmp(value, "coast")) {
								TheMap.Fields[i].Flags |= MapFieldCoastAllowed;
							} else if (!strcmp(value, "water")) {
								TheMap.Fields[i].Flags |= MapFieldWaterAllowed;

							} else if (!strcmp(value, "mud")) {
								TheMap.Fields[i].Flags |= MapFieldNoBuilding;
							} else if (!strcmp(value, "block")) {
								TheMap.Fields[i].Flags |= MapFieldUnpassable;

							} else if (!strcmp(value, "wall")) {
								TheMap.Fields[i].Flags |= MapFieldWall;
							} else if (!strcmp(value, "rock")) {
								TheMap.Fields[i].Flags |= MapFieldRocks;
							} else if (!strcmp(value, "wood")) {
								TheMap.Fields[i].Flags |= MapFieldForest;

							} else if (!strcmp(value, "ground")) {
								TheMap.Fields[i].Flags |= MapFieldLandUnit;
							} else if (!strcmp(value, "air")) {
								TheMap.Fields[i].Flags |= MapFieldAirUnit;
							} else if (!strcmp(value, "sea")) {
								TheMap.Fields[i].Flags |= MapFieldSeaUnit;
							} else if (!strcmp(value, "building")) {
								TheMap.Fields[i].Flags |= MapFieldBuilding;

							} else {
							   lua_pushfstring(l, "Unsupported tag: %s", value);
							   lua_error(l);
							}
						}
						lua_pop(l, 1);
						++i;
					}
					lua_pop(l, 1);
				} else {
				   lua_pushfstring(l, "Unsupported tag: %s", value);
				   lua_error(l);
				}
			}

		} else {
		   lua_pushfstring(l, "Unsupported tag: %s", value);
		   lua_error(l);
		}
	}

	return 0;
}

/**
**  Reveal the complete map.
*/
local int CclRevealMap(lua_State* l)
{
	if (lua_gettop(l) != 0) {
		lua_pushstring(l, "incorrect argument");
		lua_error(l);
	}
	if (CclInConfigFile) {
		FlagRevealMap = 1;
	} else {
		RevealMap();
	}

	return 0;
}

/**
**  Center the map.
**
**  @param x  X tile location.
**  @param y  Y tile location.
*/
local int CclCenterMap(lua_State* l)
{
	if (lua_gettop(l) != 2) {
		lua_pushstring(l, "incorrect argument");
		lua_error(l);
	}
	ViewportCenterViewpoint(TheUI.SelectedViewport,
		LuaToNumber(l, 1), LuaToNumber(l, 2));

	return 0;
}

/**
**  Show Map Location
**
**  @param x       X tile location.
**  @param y       Y tile location.
**  @param radius  radius of view.
**  @param cycle   cycles show vision for.
**  @param unit    name of unit to use for showing map
*/
local int CclShowMapLocation(lua_State* l)
{
	Unit* target;
	const char* unitname;

	// Put a unit on map, use its properties, except for
	// what is listed below

	if (lua_gettop(l) != 4) {
		lua_pushstring(l, "incorrect argument");
		lua_error(l);
	}
	unitname = LuaToString(l, 5);
	target = MakeUnit(UnitTypeByIdent(unitname), ThisPlayer);
	target->Orders[0].Action = UnitActionStill;
	target->HP = 0;
	target->X = LuaToNumber(l, 1);
	target->Y = LuaToNumber(l, 2);
	target->TTL = GameCycle + LuaToNumber(l, 4);
	target->CurrentSightRange = LuaToNumber(l, 3);
	MapMarkUnitSight(target);
	return 0;
}

/**
**  Set the default map.
**
**  @param map  Path to the default map.
**
**  @return     The old default map.
*/
local int CclSetDefaultMap(lua_State* l)
{
	char* old;

	if (lua_gettop(l) != 1) {
		lua_pushstring(l, "incorrect argument");
		lua_error(l);
	}
	old = strdup(DefaultMap);
	strcpy(DefaultMap, LuaToString(l, 1));

	lua_pushstring(l, old);
	free(old);
	return 1;
}

/**
**  Set fog of war on/off.
**
**  @param flag  True = turning fog of war on, false = off.
**
**  @return      The old state of fog of war.
*/
local int CclSetFogOfWar(lua_State* l)
{
	int old;

	if (lua_gettop(l) != 1) {
		lua_pushstring(l, "incorrect argument");
		lua_error(l);
	}
	old = !TheMap.NoFogOfWar;
	TheMap.NoFogOfWar = !LuaToBoolean(l, 1);
	if (!CclInConfigFile) {
		UpdateFogOfWarChange();
	}

	lua_pushboolean(l, old);
	return 1;
}

/**
**  Enable display of terrain in minimap.
**
**  @param flag  true = show minimap with terrain, false = show no terrain.
**
**  @return      The old state of the minimap with terrain.
*/
local int CclSetMinimapTerrain(lua_State* l)
{
	int old;

	if (lua_gettop(l) != 1) {
		lua_pushstring(l, "incorrect argument");
		lua_error(l);
	}
	old = MinimapWithTerrain;
	MinimapWithTerrain = LuaToBoolean(l, 1);

	lua_pushboolean(l, old);
	return 1;
}

/**
**  Original fog of war.
*/
local int CclOriginalFogOfWar(lua_State* l)
{
	if (lua_gettop(l) != 0) {
		lua_pushstring(l, "incorrect argument");
		lua_error(l);
	}
	OriginalFogOfWar = 1;

	if (!CclInConfigFile) {
		InitMapFogOfWar();
	}

	return 0;
}

/**
**  Alpha style fog of war.
*/
local int CclAlphaFogOfWar(lua_State* l)
{
	if (lua_gettop(l) != 0) {
		lua_pushstring(l, "incorrect argument");
		lua_error(l);
	}
	OriginalFogOfWar = 0;

	if (!CclInConfigFile) {
		InitMapFogOfWar();
	}

	return 0;
}

/**
**  Gray style fog of war brightness.
*/
local int CclSetFogOfWarOpacity(lua_State* l)
{
	int i;
	int old;

	if (lua_gettop(l) != 1) {
		lua_pushstring(l, "incorrect argument");
		lua_error(l);
	}
	i = LuaToNumber(l, 1);
	if (i < 0 || i > 255) {
		PrintFunction();
		fprintf(stdout, "Opacity should be 0 - 256\n");
		i = 100;
	}
	old = FogOfWarOpacity;
	FogOfWarOpacity = i;

	if (!CclInConfigFile) {
		InitMapFogOfWar();
	}

	lua_pushnumber(l, old);
	return 1;
}

/**
**  Set forest regeneration speed.
**
**  @param speed  New regeneration speed (0 disabled)
**
**  @return       Old speed
*/
local int CclSetForestRegeneration(lua_State* l)
{
	int i;
	int old;

	if (lua_gettop(l) != 1) {
		lua_pushstring(l, "incorrect argument");
		lua_error(l);
	}
	i = LuaToNumber(l, 1);
	if (i < 0 || i > 255) {
		PrintFunction();
		fprintf(stdout, "Regneration speed should be 0 - 255\n");
		i = 100;
	}
	old = ForestRegeneration;
	ForestRegeneration = i;

	if (!CclInConfigFile) {
		InitMapFogOfWar();
	}

	lua_pushnumber(l, old);
	return 1;
}

/**
**  Register CCL features for map.
*/
global void MapCclRegister(void)
{
	lua_register(Lua, "StratagusMap", CclStratagusMap);
	lua_register(Lua, "RevealMap", CclRevealMap);
	lua_register(Lua, "CenterMap", CclCenterMap);
	lua_register(Lua, "ShowMapLocation", CclShowMapLocation);

	lua_register(Lua, "SetDefaultMap", CclSetDefaultMap);
	lua_register(Lua, "SetFogOfWar", CclSetFogOfWar);
	lua_register(Lua, "SetMinimapTerrain", CclSetMinimapTerrain);

	lua_register(Lua, "OriginalFogOfWar", CclOriginalFogOfWar);
	lua_register(Lua, "AlphaFogOfWar", CclAlphaFogOfWar);

	lua_register(Lua, "SetFogOfWarOpacity", CclSetFogOfWarOpacity);

	lua_register(Lua, "SetForestRegeneration",CclSetForestRegeneration);
}

//@}
