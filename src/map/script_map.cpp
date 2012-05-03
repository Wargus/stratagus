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
/**@name script_map.cpp - The map ccl functions. */
//
//      (c) Copyright 1999-2005 by Lutz Sammer and Jimmy Salmon
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
#include <stdlib.h>

#include "stratagus.h"
#include "unit.h"
#include "unittype.h"
#include "script.h"
#include "map.h"
#include "tileset.h"
#include "minimap.h"
#include "results.h"
#include "ui.h"
#include "player.h"
#include "iolib.h"

/*----------------------------------------------------------------------------
--  Functions
----------------------------------------------------------------------------*/

/**
**  Parse a map.
**
**  @param l  Lua state.
*/
static int CclStratagusMap(lua_State *l)
{
	const char *value;
	int args;
	int j;
	int subargs;
	int k;

	//
	//  Parse the list: (still everything could be changed!)
	//

	args = lua_gettop(l);
	for (j = 0; j < args; ++j) {
		value = LuaToString(l, j + 1);
		++j;

		if (!strcmp(value, "version")) {
			char buf[32];

			value = LuaToString(l, j + 1);
			snprintf(buf, sizeof(buf), StratagusFormatString, StratagusFormatArgs(StratagusVersion));
			if (strcmp(buf, value)) {
				fprintf(stderr, "Warning not saved with this version.\n");
			}
		} else if (!strcmp(value, "uid")) {
			Map.Info.MapUID = LuaToNumber(l, j + 1);
		} else if (!strcmp(value, "description")) {
			Map.Info.Description = LuaToString(l, j + 1);
		} else if (!strcmp(value, "the-map")) {
			if (!lua_istable(l, j + 1)) {
				LuaError(l, "incorrect argument");
			}
			subargs = lua_rawlen(l, j + 1);
			for (k = 0; k < subargs; ++k) {
				lua_rawgeti(l, j + 1, k + 1);
				value = LuaToString(l, -1);
				lua_pop(l, 1);
				++k;

				if (!strcmp(value, "size")) {
					lua_rawgeti(l, j + 1, k + 1);
					if (!lua_istable(l, -1)) {
						LuaError(l, "incorrect argument");
					}
					lua_rawgeti(l, -1, 1);
					Map.Info.MapWidth = LuaToNumber(l, -1);
					lua_pop(l, 1);
					lua_rawgeti(l, -1, 2);
					Map.Info.MapHeight = LuaToNumber(l, -1);
					lua_pop(l, 1);
					lua_pop(l, 1);

					delete[] Map.Fields;
					Map.Fields = new CMapField[Map.Info.MapWidth * Map.Info.MapHeight];
					// FIXME: this should be CreateMap or InitMap?
				} else if (!strcmp(value, "fog-of-war")) {
					Map.NoFogOfWar = false;
					--k;
				} else if (!strcmp(value, "no-fog-of-war")) {
					Map.NoFogOfWar = true;
					--k;
				} else if (!strcmp(value, "filename")) {
					lua_rawgeti(l, j + 1, k + 1);
					Map.Info.Filename = LuaToString(l, -1);
					lua_pop(l, 1);
				} else if (!strcmp(value, "map-fields")) {
					int i;
					int subsubargs;
					int subk;

					lua_rawgeti(l, j + 1, k + 1);
					if (!lua_istable(l, -1)) {
						LuaError(l, "incorrect argument");
					}

					subsubargs = lua_rawlen(l, -1);
					if (subsubargs != Map.Info.MapWidth * Map.Info.MapHeight) {
						fprintf(stderr, "Wrong tile table length: %d\n", subsubargs);
					}
					i = 0;
					for (subk = 0; subk < subsubargs; ++subk) {
						int args2;
						int j2;

						lua_rawgeti(l, -1, subk + 1);
						if (!lua_istable(l, -1)) {
							LuaError(l, "incorrect argument");
						}
						args2 = lua_rawlen(l, -1);
						j2 = 0;

						lua_rawgeti(l, -1, j2 + 1);
						Map.Fields[i].Tile = LuaToNumber(l, -1);
						lua_pop(l, 1);
						++j2;
						lua_rawgeti(l, -1, j2 + 1);
						Map.Fields[i].SeenTile = LuaToNumber(l, -1);
						lua_pop(l, 1);
						++j2;
						for (; j2 < args2; ++j2) {
							lua_rawgeti(l, -1, j2 + 1);
							if (lua_isnumber(l, -1)) {
								Map.Fields[i].Value = LuaToNumber(l, -1);
								lua_pop(l, 1);
								continue;
							}
							value = LuaToString(l, -1);
							lua_pop(l, 1);
							if (!strcmp(value, "explored")) {
								++j2;
								lua_rawgeti(l, -1, j2 + 1);
								Map.Fields[i].Visible[(int)LuaToNumber(l, -1)] = 1;
								lua_pop(l, 1);
							} else if (!strcmp(value, "human")) {
								Map.Fields[i].Flags |= MapFieldHuman;

							} else if (!strcmp(value, "land")) {
								Map.Fields[i].Flags |= MapFieldLandAllowed;
							} else if (!strcmp(value, "coast")) {
								Map.Fields[i].Flags |= MapFieldCoastAllowed;
							} else if (!strcmp(value, "water")) {
								Map.Fields[i].Flags |= MapFieldWaterAllowed;

							} else if (!strcmp(value, "mud")) {
								Map.Fields[i].Flags |= MapFieldNoBuilding;
							} else if (!strcmp(value, "block")) {
								Map.Fields[i].Flags |= MapFieldUnpassable;

							} else if (!strcmp(value, "wall")) {
								Map.Fields[i].Flags |= MapFieldWall;
							} else if (!strcmp(value, "rock")) {
								Map.Fields[i].Flags |= MapFieldRocks;
							} else if (!strcmp(value, "wood")) {
								Map.Fields[i].Flags |= MapFieldForest;

							} else if (!strcmp(value, "ground")) {
								Map.Fields[i].Flags |= MapFieldLandUnit;
							} else if (!strcmp(value, "air")) {
								Map.Fields[i].Flags |= MapFieldAirUnit;
							} else if (!strcmp(value, "sea")) {
								Map.Fields[i].Flags |= MapFieldSeaUnit;
							} else if (!strcmp(value, "building")) {
								Map.Fields[i].Flags |= MapFieldBuilding;

							} else {
								LuaError(l, "Unsupported tag: %s" _C_ value);
							}
						}
						lua_pop(l, 1);
						++i;
					}
					lua_pop(l, 1);
				} else {
					LuaError(l, "Unsupported tag: %s" _C_ value);
				}
			}

		} else {
			LuaError(l, "Unsupported tag: %s" _C_ value);
		}
	}

	return 0;
}

/**
**  Reveal the complete map.
**
**  @param l  Lua state.
*/
static int CclRevealMap(lua_State *l)
{
	LuaCheckArgs(l, 0);
	if (CclInConfigFile || !Map.Fields) {
		FlagRevealMap = 1;
	} else {
		Map.Reveal();
	}

	return 0;
}

/**
**  Center the map.
**
**  @param l  Lua state.
*/
static int CclCenterMap(lua_State *l)
{
	LuaCheckArgs(l, 2);
	const Vec2i pos = {LuaToNumber(l, 1), LuaToNumber(l, 2)};

	UI.SelectedViewport->Center(Map.TilePosToMapPixelPos_Center(pos));
	return 0;
}

/**
**  Define the starting viewpoint for a given player.
**
**  @param l  Lua state.
*/
static int CclSetStartView(lua_State *l)
{
	LuaCheckArgs(l, 3);

	const int p = LuaToNumber(l, 1);
	Players[p].StartPos.x = LuaToNumber(l, 2);
	Players[p].StartPos.y = LuaToNumber(l, 3);

	return 0;
}

/**
**  Show Map Location
**
**  @param l  Lua state.
*/
static int CclShowMapLocation(lua_State *l)
{
	// Put a unit on map, use its properties, except for
	// what is listed below

	LuaCheckArgs(l, 4);
	const char *unitname = LuaToString(l, 5);
	CUnitType *unitType = UnitTypeByIdent(unitname);
	if (!unitType) {
		DebugPrint("Unable to find UnitType '%s'" _C_ unitname);
		return 0;
	}
	CUnit *target = MakeUnit(*unitType, ThisPlayer);
	if (target != NoUnitP) {
		target->Variable[HP_INDEX].Value = 0;
		target->tilePos.x = LuaToNumber(l, 1);
		target->tilePos.y = LuaToNumber(l, 2);
		target->TTL = GameCycle + LuaToNumber(l, 4);
		target->CurrentSightRange = LuaToNumber(l, 3);
		MapMarkUnitSight(*target);
	} else {
		DebugPrint("Unable to allocate Unit");
	}
	return 0;
}

/**
**  Set fog of war on/off.
**
**  @param l  Lua state.
*/
static int CclSetFogOfWar(lua_State *l)
{
	LuaCheckArgs(l, 1);
	Map.NoFogOfWar = !LuaToBoolean(l, 1);
	if (!CclInConfigFile && Map.Fields) {
		UpdateFogOfWarChange();
		// FIXME: save setting in replay log
		//CommandLog("input", NoUnitP, FlushCommands, -1, -1, NoUnitP, "fow off", -1);
	}
	return 0;
}

static int CclGetFogOfWar(lua_State *l)
{
	LuaCheckArgs(l, 0);
	lua_pushboolean(l, !Map.NoFogOfWar);
	return 1;
}

/**
**  Enable display of terrain in minimap.
**
**  @param l  Lua state.
*/
static int CclSetMinimapTerrain(lua_State *l)
{
	LuaCheckArgs(l, 1);
	UI.Minimap.WithTerrain = LuaToBoolean(l, 1);
	return 0;
}

/**
**  Fog of war opacity.
**
**  @param l  Lua state.
*/
static int CclSetFogOfWarOpacity(lua_State *l)
{
	int i;

	LuaCheckArgs(l, 1);
	i = LuaToNumber(l, 1);
	if (i < 0 || i > 255) {
		PrintFunction();
		fprintf(stdout, "Opacity should be 0 - 256\n");
		i = 100;
	}
	FogOfWarOpacity = i;

	if (!CclInConfigFile) {
		Map.Init();
	}

	return 0;
}

/**
**  Set forest regeneration speed.
**
**  @param l  Lua state.
**
**  @return   Old speed
*/
static int CclSetForestRegeneration(lua_State *l)
{
	int i;
	int old;

	LuaCheckArgs(l, 1);
	i = LuaToNumber(l, 1);
	if (i < 0 || i > 255) {
		PrintFunction();
		fprintf(stdout, "Regeneration speed should be 0 - 255\n");
		i = 100;
	}
	old = ForestRegeneration;
	ForestRegeneration = i;

	lua_pushnumber(l, old);
	return 1;
}

/**
**  Set Fog color.
**
**  @param l  Lua state.
*/
static int CclSetFogOfWarColor(lua_State *l)
{
	int r, g, b;

	LuaCheckArgs(l, 3);
	r = LuaToNumber(l, 1);
	g = LuaToNumber(l, 2);
	b = LuaToNumber(l, 3);

	if ((r < 0 || r > 255) ||
		(g < 0 || g > 255) ||
		(b < 0 || b > 255)) {
		LuaError(l, "Arguments must be in the range 0-255");
	}

	FogOfWarColor[0] = r;
	FogOfWarColor[1] = g;
	FogOfWarColor[2] = b;

	return 0;
}

/**
**  Define Fog graphics
**
**  @param l  Lua state.
*/
static int CclSetFogOfWarGraphics(lua_State *l)
{
	std::string FogGraphicFile;

	LuaCheckArgs(l, 1);
	FogGraphicFile = LuaToString(l, 1);
	if (CMap::FogGraphic) {
		CGraphic::Free(CMap::FogGraphic);
	}
	CMap::FogGraphic = CGraphic::New(FogGraphicFile, PixelTileSize.x, PixelTileSize.y);

	return 0;
}

/**
**  Set a tile
**
**  @param tile   Tile number
**  @param pos    coordinate
**  @param value  Value of the tile
*/
void SetTile(int tile, const Vec2i &pos, int value)
{
	if (!Map.Info.IsPointOnMap(pos)) {
		fprintf(stderr, "Invalid map coordonate : (%d, %d)\n", pos.x, pos.y);
		return;
	}
	if (tile < 0 || tile >= Map.Tileset.NumTiles) {
		fprintf(stderr, "Invalid tile number: %d\n", tile);
		return;
	}
	if (value < 0 || value >= 256) {
		fprintf(stderr, "Invalid tile number: %d\n", tile);
		return;
	}

	if (Map.Fields) {
		CMapField &mf = *Map.Field(pos);

		mf.Tile = Map.Tileset.Table[tile];
		mf.Value = value;
		mf.Flags = Map.Tileset.FlagsTable[tile];
		mf.Cost = 1 << (Map.Tileset.FlagsTable[tile] & MapFieldSpeedMask);
#ifdef DEBUG
		mf.TilesetTile = tile;
#endif
	}
}

/**
**  Define the type of each player available for the map
**
**  @param l  Lua state.
*/
static int CclDefinePlayerTypes(lua_State *l)
{
	const char *type;
	int numplayers;
	int i;

	numplayers = lua_gettop(l); /* Number of players == number of arguments */
	if (numplayers < 2) {
		LuaError(l, "Not enough players");
	}

	for (i = 0; i < numplayers && i < PlayerMax; ++i) {
		if (lua_isnil(l, i + 1)) {
			numplayers = i;
			break;
		}
		type = LuaToString(l, i + 1);
		if (!strcmp(type, "neutral")) {
			Map.Info.PlayerType[i] = PlayerNeutral;
		} else if (!strcmp(type, "nobody")) {
			Map.Info.PlayerType[i] = PlayerNobody;
		} else if (!strcmp(type, "computer")) {
			Map.Info.PlayerType[i] = PlayerComputer;
		} else if (!strcmp(type, "person")) {
			Map.Info.PlayerType[i] = PlayerPerson;
		} else if (!strcmp(type, "rescue-passive")) {
			Map.Info.PlayerType[i] = PlayerRescuePassive;
		} else if (!strcmp(type, "rescue-active")) {
			Map.Info.PlayerType[i] = PlayerRescueActive;
		} else {
			LuaError(l, "Unsupported tag: %s" _C_ type);
		}
	}
	for (i = numplayers; i < PlayerMax - 1; ++i) {
		Map.Info.PlayerType[i] = PlayerNobody;
	}
	if (numplayers < PlayerMax) {
		Map.Info.PlayerType[PlayerMax - 1] = PlayerNeutral;
	}
	return 0;
}

/**
** Load the lua file which will define the tile models
**
**  @param l  Lua state.
*/
static int CclLoadTileModels(lua_State *l)
{
	char buf[1024];

	if (lua_gettop(l) != 1) {
		LuaError(l, "incorrect argument");
	}
	strcpy_s(Map.TileModelsFileName, sizeof(Map.TileModelsFileName), LuaToString(l, 1));
	LibraryFileName(Map.TileModelsFileName, buf, sizeof(buf));
	if (LuaLoadFile(buf) == -1) {
		DebugPrint("Load failed: %s\n" _C_ LuaToString(l, 1));
	}
	return 0;
}

/**
**  Register CCL features for map.
*/
void MapCclRegister()
{
	lua_register(Lua, "StratagusMap", CclStratagusMap);
	lua_register(Lua, "RevealMap", CclRevealMap);
	lua_register(Lua, "CenterMap", CclCenterMap);
	lua_register(Lua, "SetStartView", CclSetStartView);
	lua_register(Lua, "ShowMapLocation", CclShowMapLocation);

	lua_register(Lua, "SetFogOfWar", CclSetFogOfWar);
	lua_register(Lua, "GetFogOfWar", CclGetFogOfWar);
	lua_register(Lua, "SetMinimapTerrain", CclSetMinimapTerrain);

	lua_register(Lua, "SetFogOfWarGraphics", CclSetFogOfWarGraphics);
	lua_register(Lua, "SetFogOfWarOpacity", CclSetFogOfWarOpacity);
	lua_register(Lua, "SetFogOfWarColor", CclSetFogOfWarColor);

	lua_register(Lua, "SetForestRegeneration", CclSetForestRegeneration);

	lua_register(Lua, "LoadTileModels", CclLoadTileModels);
	lua_register(Lua, "DefinePlayerTypes", CclDefinePlayerTypes);
}

//@}
