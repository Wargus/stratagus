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

#include "stratagus.h"

#include "map.h"
#include "fov.h"
#include "fow.h"
#include "iolib.h"
#include "netconnect.h"
#include "network.h"
#include "script.h"
#include "tileset.h"
#include "translate.h"
#include "ui.h"
#include "unit.h"
#include "version.h"
#include "video.h"

/*----------------------------------------------------------------------------
--  Variables
----------------------------------------------------------------------------*/

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
	int args = lua_gettop(l);
	for (int j = 0; j < args; ++j) {
		const char *value = LuaToString(l, j + 1);
		++j;

		if (!strcmp(value, "version")) {
			char buf[32];

			const char *version = LuaToString(l, j + 1);
			strncpy(buf, VERSION, sizeof(buf));
			if (strcmp(buf, version)) {
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
			int subargs = lua_rawlen(l, j + 1);
			for (int k = 0; k < subargs; ++k) {
				const char *value = LuaToString(l, j + 1, k + 1);
				++k;

				if (!strcmp(value, "size")) {
					lua_rawgeti(l, j + 1, k + 1);
					CclGetPos(l, &Map.Info.MapWidth, &Map.Info.MapHeight);
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
					Map.Info.Filename = LuaToString(l, j + 1, k + 1);
				} else if (!strcmp(value, "map-fields")) {
					lua_rawgeti(l, j + 1, k + 1);
					if (!lua_istable(l, -1)) {
						LuaError(l, "incorrect argument");
					}
					const int subsubargs = lua_rawlen(l, -1);
					if (subsubargs != Map.Info.MapWidth * Map.Info.MapHeight) {
						fprintf(stderr, "Wrong tile table length: %d\n", subsubargs);
					}
					for (int i = 0; i < subsubargs; ++i) {
						lua_rawgeti(l, -1, i + 1);
						if (!lua_istable(l, -1)) {
							LuaError(l, "incorrect argument");
						}
						Map.Fields[i].parse(l);
						lua_pop(l, 1);
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
	LuaCheckArgs(l, 1);

	MapRevealModes newMode;
	const char *revealMode = LuaToString(l, 1);
	if (!strcmp(revealMode, "hidden")) {
		newMode = MapRevealModes::cHidden;
	} else 	if (!strcmp(revealMode, "known")) {
		newMode = MapRevealModes::cKnown;
	} else if (!strcmp(revealMode, "explored")) {
		newMode = MapRevealModes::cExplored;
	} else {
		PrintFunction();
		fprintf(stdout, "Accessible reveal modes: \"hidden\", \"known\", \"explored\".\n");
		return 1;
	}

	if (CclInConfigFile || !Map.Fields) {
		FlagRevealMap = newMode;
	} else if (!IsNetworkGame()) {
		Map.Reveal(newMode);
	} else {
		NetworkSendExtendedCommand(ExtendedMessageRevealMapDB, int(newMode), 0, 0, 0, 0);
	}
	return 0;
}

/**
** <b>Description</b>
**
**  Center the map.
**
**  @param l  Lua state.
**
** Example:
**
** <div class="example"><code>-- Center the view at position x=11 and y=1.
**		  <strong>CenterMap</strong>(11, 1)</code></div>
*/
static int CclCenterMap(lua_State *l)
{
	LuaCheckArgs(l, 2);
	const Vec2i pos(LuaToNumber(l, 1), LuaToNumber(l, 2));

	UI.SelectedViewport->Center(Map.TilePosToMapPixelPos_Center(pos));
	return 0;
}

/**
** <b>Description</b>
**
**  Define the starting viewpoint for a given player.
**
**  @param l  Lua state.
**
** Example:
**
** <div class="example"><code>-- Start view for player 0.
**		  <strong>SetStartView</strong>(0, 25, 12)
**		  -- Start view for player 1.
**		  <strong>SetStartView</strong>(1, 71, 38)</code></div>
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

	LuaCheckArgs(l, 5);
	const char *unitname = LuaToString(l, 5);
	CUnitType *unitType = UnitTypeByIdent(unitname);
	if (!unitType) {
		DebugPrint("Unable to find UnitType '%s'" _C_ unitname);
		return 0;
	}
	CUnit *target = MakeUnit(*unitType, ThisPlayer);
	if (target != NULL) {
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
** <b>Description</b>
**
**  Set fog of war on/off.
**
** Example:
**
** <div class="example"><code><strong>SetFogOfWar</strong>(true)</code></div>
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

/**
** <b>Description</b>
**
**  Get if the fog of war is enabled.
**
**  @param l  Lua state.
**
** Example:
**
** <div class="example"><code><strong>GetFogOfWar</strong>()</code></div>
*/
static int CclGetFogOfWar(lua_State *l)
{
	LuaCheckArgs(l, 0);
	lua_pushboolean(l, !Map.NoFogOfWar);
	return 1;
}

/**
** <b>Description</b>
**
**  Enable display of terrain in minimap.
**
**  @param l  Lua state.
**
** Example:
**
** <div class="example"><code>-- Show the minimap terrain
**		<strong>SetMinimapTerrain</strong>(true)</code></div>
*/
static int CclSetMinimapTerrain(lua_State *l)
{
	LuaCheckArgs(l, 1);
	UI.Minimap.WithTerrain = LuaToBoolean(l, 1);
	return 0;
}

/**
**  Activate map grid  (true|false)
**
**  @param l  Lua state.
**
**  @return   0 for success, 1 for wrong type;
*/
static int CclSetEnableMapGrid(lua_State *l)
{
	LuaCheckArgs(l, 1);
	CViewport::EnableGrid(LuaToBoolean(l, 1));
	return 0;
}

/**
**  Check if map grid is enabled
*/
static int CclGetIsMapGridEnabled(lua_State *l)
{
	LuaCheckArgs(l, 0);
	lua_pushboolean(l, CViewport::isGridEnabled());
	return 1;
}

/**
**  Select unit's field of view algorithm -  ShadowCasting or SimpleRadial
**
**  @param l  Lua state.
**
**  @return   0 for success, 1 for wrong type;
*/
static int CclSetFieldOfViewType(lua_State *l)
{
	LuaCheckArgs(l, 1);

	FieldOfViewTypes new_type;
	const char *type_name = LuaToString(l, 1);
	if (!strcmp(type_name, "shadow-casting")) {
		new_type = FieldOfViewTypes::cShadowCasting;
		/// Tiled types of FOW don't work with shadow casting
		if (FogOfWar->GetType() != FogOfWarTypes::cEnhanced) {
			FogOfWar->SetType(FogOfWarTypes::cEnhanced);
		}
	} else if (!strcmp(type_name, "simple-radial")) {
		new_type = FieldOfViewTypes::cSimpleRadial;
	} else {
		PrintFunction();
		fprintf(stdout, "Accessible Field of View types: \"shadow-casting\", \"simple-radial\".\n");
		return 1;
	}
	if (!IsNetworkGame()) {
		FieldOfView.SetType(new_type);
	} else {
		NetworkSendExtendedCommand(ExtendedMessageFieldOfViewDB, int(new_type), 0, 0, 0, 0);
	}
	return 0;
}

/**
**  Get unit's field of view type -  ShadowCasting or SimpleRadial
*/
static int CclGetFieldOfViewType(lua_State *l)
{
	LuaCheckArgs(l, 0);
	lua_pushinteger(l, int(FieldOfView.GetType()));
	return 1;
}

/**
**  Set opaque for the tile's terrain.
**
**  @param l  Lua state.
**
**  @return   0 for success, 1 for wrong tile's terrain;
*/
static int CclSetOpaqueFor(lua_State *l)
{
	uint16_t new_flag = 0;
	const int args = lua_gettop(l);
	if (args < 1) {
		LuaError(l, "argument missed");
		return 1;
	}
	for (int arg = 0; arg < args; ++arg) {
		const char *flag_name = LuaToString(l, arg + 1);
		if (!strcmp(flag_name, "wall")) {
			new_flag |= MapFieldWall;
		} else if (!strcmp(flag_name, "rock")) {
			new_flag |= MapFieldRocks;
		} else if (!strcmp(flag_name, "forest")) {
			new_flag |= MapFieldForest;
		} else {
			PrintFunction();
			fprintf(stdout, "Opaque can only be set for \"wall\", \"rock\" or \"forest\". \n");
			return 1;
		}
	}
	if (!IsNetworkGame()) {
		FieldOfView.SetOpaqueFields(FieldOfView.GetOpaqueFields() | new_flag);
	} else {
		NetworkSendExtendedCommand(ExtendedMessageMapFieldsOpacityDB, 0,
								   FieldOfView.GetOpaqueFields() | new_flag, 0, 0, 0);
	}
	return 0;
}
/**
**  Check opacity for the tile's terrain.
**
**  @param l  Lua state.
**
*/
static int CclGetIsOpaqueFor(lua_State *l)
{
	LuaCheckArgs(l, 1);

	uint16_t flagToCheck = 0;
	const char *flag_name = LuaToString(l, 1);
	if (!strcmp(flag_name, "wall")) {
		flagToCheck = MapFieldWall;
	} else if (!strcmp(flag_name, "rock")) {
		flagToCheck = MapFieldRocks;
	} else if (!strcmp(flag_name, "forest")) {
		flagToCheck = MapFieldForest;
	} else {
		PrintFunction();
		fprintf(stdout, "Opaque can only be checked for \"wall\", \"rock\" or \"forest\". \n");
	}

	lua_pushboolean(l, FieldOfView.GetOpaqueFields() & flagToCheck);
	return 1;
}

static int CclRemoveOpaqueFor(lua_State *l)
{
	unsigned short new_flag = 0;
	const int args = lua_gettop(l);
	if (args < 1) {
		LuaError(l, "argument missed");
		return 1;
	}
	for (int arg = 0; arg < args; ++arg) {
		const char *flag_name = LuaToString(l, arg + 1);
		if (!strcmp(flag_name, "wall")) {
			new_flag |= MapFieldWall;
		} else if (!strcmp(flag_name, "rock")) {
			new_flag |= MapFieldRocks;
		} else if (!strcmp(flag_name, "forest")) {
			new_flag |= MapFieldForest;
		} else {
			PrintFunction();
			fprintf(stdout, "Opaque can only be removed for \"wall\", \"rock\" or \"forest\". \n");
			return 1;
		}
	}
	if (!IsNetworkGame()) {
		FieldOfView.SetOpaqueFields(FieldOfView.GetOpaqueFields() & ~new_flag);
	} else {
		NetworkSendExtendedCommand(ExtendedMessageMapFieldsOpacityDB, 0,
								   FieldOfView.GetOpaqueFields() & ~new_flag, 0, 0, 0);
	}
	return 0;
}


/**
**  Select which type of Fog of War to use
**
**  @param l  Lua state.
**
**  @return   0 for success, 1 for wrong type;
*/
static int CclSetFogOfWarType(lua_State *l)
{
	LuaCheckArgs(l, 1);
	
	FogOfWarTypes new_type;
	const std::string type_name {LuaToString(l, 1)};
	if (type_name == "tiled" || type_name == "fast") {
		new_type = type_name == "tiled" ? FogOfWarTypes::cTiled : FogOfWarTypes::cTiledLegacy;
		/// Tiled types of FOW don't work with shadow casting
		if (FieldOfView.GetType() == FieldOfViewTypes::cShadowCasting) {
			if (!IsNetworkGame()) {
				FieldOfView.SetType(FieldOfViewTypes::cSimpleRadial);
			} else {
				NetworkSendExtendedCommand(ExtendedMessageFieldOfViewDB, 
										   int(FieldOfViewTypes::cSimpleRadial), 0, 0, 0, 0);
			}
		}
	} else if (type_name == "enhanced") {
		new_type = FogOfWarTypes::cEnhanced;
	} else {
		PrintFunction();
		fprintf(stdout, "Accessible Fog of War types: \"tiled\", \"enhanced\" and \"fast\".\n");
		return 1;
	}
	FogOfWar->SetType(new_type);
	return 0;
}

/**
**  Get Fog of War type - legacy or enhanced
*/
static int CclGetFogOfWarType(lua_State *l)
{
	LuaCheckArgs(l, 0);
	lua_pushinteger(l, int(FogOfWar->GetType()));
	return 1;
}

/**
**  Set opacity (alpha) for different levels of fog of war - explored, revealed, unseen
**
**  @param l  Lua state.
**
**  @return   0 for success, 1 for wrong type;
*/
static int CclSetFogOfWarOpacityLevels(lua_State *l)
{
	LuaCheckArgs(l, 3);
	const int explored = LuaToNumber(l, 1);
	if (explored <= 0 || explored > 255) {
		PrintFunction();
		fprintf(stderr, "Invalid value (%d) of opacity for Explored tiles. Acceptable range is [0 <= Explored <= Known <= Hidden <= 255].\n", explored);
		return 1;
	}
	const int revealed = LuaToNumber(l, 2);
	if (revealed <= explored || revealed > 255) {
		PrintFunction();
		fprintf(stderr, "Invalid value (%d) of opacity for Revealed tiles. Acceptable range is [0 <= Explored <= Known <= Hidden <= 255].\n", revealed);
		return 1;
	}
	const int unseen = LuaToNumber(l, 3);
	if (unseen < revealed || unseen > 255) {
		PrintFunction();
		fprintf(stderr, "Invalid value (%d) of opacity for Unseen tiles. Acceptable range is [0 <= Explored <= Known <= Hidden <= 255].\n", unseen);
		return 1;
	}

	FogOfWar->SetOpacityLevels(explored, revealed, unseen);

	return 0;	
}

/**
**  Set parameters for FOW blurer (radiuses and number of iterations)
**
**  @param l  Lua state.
**
**  @return   0 for success, 1 for wrong type;
*/
static int CclSetFogOfWarBlur(lua_State *l)
{
	LuaCheckArgs(l, 3);

	const float radiusSimple = LuaToFloat(l, 1);
	if (radiusSimple <= 0 ) {
		PrintFunction();
		fprintf(stdout, "Radius should be a positive float number. Blur is disabled.\n");
	}

	const float radiusBilinear = LuaToFloat(l, 2);
	if (radiusBilinear <= 0 ) {
		PrintFunction();
		fprintf(stdout, "Radius should be a positive float number. Blur is disabled.\n");
	}

	const int iterations = LuaToNumber(l, 3);	
	if (iterations <= 0 ) {
		PrintFunction();
		fprintf(stdout, "Number of box blur iterations should be greater than 0. Blur is disabled.\n");
	}
	FogOfWar->InitBlurer(radiusSimple, radiusBilinear, iterations);
	return 0;
}

/**
**  Activate FOW bilinear upscaling type  (true|false)
**
**  @param l  Lua state.
**
**  @return   0 for success, 1 for wrong type;
*/
static int CclSetFogOfWarBilinear(lua_State *l)
{
	LuaCheckArgs(l, 1);
	FogOfWar->EnableBilinearUpscale(LuaToBoolean(l, 1));
	return 0;
}

/**
**  Check if FOW bilinear upscaling enabled
*/
static int CclGetIsFogOfWarBilinear(lua_State *l)
{
	LuaCheckArgs(l, 0);
	lua_pushboolean(l, FogOfWar->IsBilinearUpscaleEnabled());
	return 1;
}

/**
** <b>Description</b>
**
**  Set forest regeneration speed.
**
**  @param l  Lua state.
**
**  @return   Old speed
**
** Example:
**
** <div class="example"><code>-- No regeneration.
**		  <strong>SetForestRegeneration</strong>(0)
**		  -- Slow regeneration every 50 seconds
**		  <strong>SetForestRegeneration</strong>(50)
**		  -- Extremely slow regeneration every 1h of game time
**		  <strong>SetForestRegeneration</strong>(3600)</code></div>
*/
static int CclSetForestRegeneration(lua_State *l)
{
	LuaCheckArgs(l, 1);
	int i = LuaToNumber(l, 1);
	int frequency = 1;
	if (i < 0) {
		LuaError(l, "Regeneration speed should be >= 0\n");
	}
	while (i / frequency > 255) {
		frequency++;
	}
	i = i / frequency;
	const int old = ForestRegeneration * ForestRegenerationFrequency;
	ForestRegeneration = i;
	ForestRegenerationFrequency = frequency;

	lua_pushnumber(l, old);
	return 1;
}

/**
** <b>Description</b>
**
**  Set Fog color.
**
**  @param l  Lua state.
**
** Example:
**
** <div class="example"><code>-- Red fog of war
**		<strong>SetFogOfWarColor</strong>(128,0,0)</code></div>
*/
static int CclSetFogOfWarColor(lua_State *l)
{
	LuaCheckArgs(l, 3);
	int r = LuaToNumber(l, 1);
	int g = LuaToNumber(l, 2);
	int b = LuaToNumber(l, 3);

	if ((r < 0 || r > 255) ||
		(g < 0 || g > 255) ||
		(b < 0 || b > 255)) {
		LuaError(l, "Arguments must be in the range 0-255");
	}

	FogOfWar->SetFogColor(r, g, b);

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
	CFogOfWar::SetTiledFogGraphic(FogGraphicFile);

	return 0;
}


/**
**  Set opacity (alpha) for different levels of fog of war - explored, revealed, unexplored for mini map
**
**  @param l  Lua state.
**
**  @return   0 for success, 1 for wrong type;
*/
static int CclSetMMFogOfWarOpacityLevels(lua_State *l)
{
	LuaCheckArgs(l, 3);
	const int explored = LuaToNumber(l, 1);
	if (explored <= 0 || explored > 255) {
		PrintFunction();
		fprintf(stderr, "Invalid value (%d) of opacity for Minimap's Explored tiles. Acceptable range is [0 <= Explored <= Known <= Hidden <= 255].\n", explored);
		return 1;
	}
	const int revealed = LuaToNumber(l, 2);
	if (revealed <= explored || revealed > 255) {
		PrintFunction();
		fprintf(stderr, "Invalid value (%d) of opacity for Minimap's  Revealed tiles. Acceptable range is [0 <= Explored <= Known <= Hidden <= 255].\n", revealed);
		return 1;
	}
	const int unseen = LuaToNumber(l, 3);
	if (unseen < revealed || unseen > 255) {
		PrintFunction();
		fprintf(stderr, "Invalid value (%d) of opacity for Minimap's Unseen tiles. Acceptable range is [0 <= Explored <= Known <= Hidden <= 255].\n", unseen);
		return 1;
	}

	UI.Minimap.SetFogOpacityLevels(explored, revealed, unseen);

	return 0;	
}

/**
** <b>Description</b>
**
**  Define size in pixels (x,y) of a tile in this game
**
**  @param l  Lua state.
**
** Example:
**
** <div class="example"><code><strong>SetTileSize</strong>(32,32)</code></div>
*/
static int CclSetTileSize(lua_State *l)
{
	LuaCheckArgs(l, 2);
	PixelTileSize.x = LuaToNumber(l, 1);
	PixelTileSize.y = LuaToNumber(l, 2);
	return 0;
}

/**
**  Set a tile
**
**  @param tileIndex   Tile number
**  @param pos    coordinate
**  @param value  Value of the tile
*/
void SetTile(unsigned int tileIndex, const Vec2i &pos, int value)
{
	if (!Map.Info.IsPointOnMap(pos)) {
		fprintf(stderr, "Invalid map coordonate : (%d, %d)\n", pos.x, pos.y);
		return;
	}
	if (Map.Tileset->getTileCount() <= tileIndex) {
		fprintf(stderr, "Invalid tile number: %u\n", tileIndex);
		return;
	}

	if (Map.Fields) {
		CMapField &mf = *Map.Field(pos);

		mf.setTileIndex(*Map.Tileset, tileIndex, value);
	}
}

/**
**  Define the type of each player available for the map
**
**  @param l  Lua state.
*/
static int CclDefinePlayerTypes(lua_State *l)
{
	int numplayers = lua_gettop(l); /* Number of players == number of arguments */
	if (numplayers < 2) {
		LuaError(l, "Not enough players");
	}

	for (int i = 0; i < numplayers && i < PlayerMax; ++i) {
		if (lua_isnil(l, i + 1)) {
			numplayers = i;
			break;
		}
		const char *type = LuaToString(l, i + 1);
		if (!strcmp(type, "neutral")) {
			Map.Info.PlayerType[i] = PlayerTypes::PlayerNeutral;
		} else if (!strcmp(type, "nobody")) {
			Map.Info.PlayerType[i] = PlayerTypes::PlayerNobody;
		} else if (!strcmp(type, "computer")) {
			Map.Info.PlayerType[i] = PlayerTypes::PlayerComputer;
		} else if (!strcmp(type, "person")) {
			Map.Info.PlayerType[i] = PlayerTypes::PlayerPerson;
		} else if (!strcmp(type, "rescue-passive")) {
			Map.Info.PlayerType[i] = PlayerTypes::PlayerRescuePassive;
		} else if (!strcmp(type, "rescue-active")) {
			Map.Info.PlayerType[i] = PlayerTypes::PlayerRescueActive;
		} else {
			LuaError(l, "Unsupported tag: %s" _C_ type);
		}
	}
	for (int i = numplayers; i < PlayerMax - 1; ++i) {
		Map.Info.PlayerType[i] = PlayerTypes::PlayerNobody;
	}
	if (numplayers < PlayerMax) {
		Map.Info.PlayerType[PlayerMax - 1] = PlayerTypes::PlayerNeutral;
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
	if (lua_gettop(l) != 1) {
		LuaError(l, "incorrect argument");
	}
	Map.TileModelsFileName = LuaToString(l, 1);
	const std::string filename = LibraryFileName(Map.TileModelsFileName.c_str());
	if (LuaLoadFile(filename) == -1) {
		DebugPrint("Load failed: %s\n" _C_ filename.c_str());
	}
	return 0;
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

	ShowLoadProgress(_("Tileset '%s'"), Map.Tileset->ImageFile.c_str());
	Map.TileGraphic = CGraphic::New(Map.Tileset->ImageFile, PixelTileSize.x, PixelTileSize.y);
	Map.TileGraphic->Load();
	return 0;
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
	const unsigned int tilenumber = LuaToNumber(l, 1);

	if (tilenumber >= Map.Tileset->tiles.size()) {
		LuaError(l, "Accessed a tile that's not defined");
	}
	int j = 0;
	int flags = 0;

	unsigned char newBase = Map.Tileset->parseTilesetTileFlags(l, &flags, &j);
	Map.Tileset->tiles[tilenumber].flag = flags;
	if (newBase) {
		Map.Tileset->tiles[tilenumber].tileinfo.BaseTerrain = newBase;
	}
	return 0;
}

/**
**  Get the name of the terrain of the tile.
**
**  @param l  Lua state.
**
**  @return   The name of the terrain of the tile.
*/
static int CclGetTileTerrainName(lua_State *l)
{
	LuaCheckArgs(l, 2);

	const Vec2i pos(LuaToNumber(l, 1), LuaToNumber(l, 2));

	const CMapField &mf = *Map.Field(pos);
	const CTileset &tileset = *Map.Tileset;
	const int index = tileset.findTileIndexByTile(mf.getGraphicTile());
	Assert(index != -1);
	const int baseTerrainIdx = tileset.tiles[index].tileinfo.BaseTerrain;

	lua_pushstring(l, tileset.getTerrainName(baseTerrainIdx).c_str());
	return 1;
}

/**
**  Check if the tile's terrain has a particular flag.
**
**  @param l  Lua state.
**
**  @return   True if has the flag, false if not.
*/
static int CclGetTileTerrainHasFlag(lua_State *l)
{
	LuaCheckArgs(l, 3);

	const Vec2i pos(LuaToNumber(l, 1), LuaToNumber(l, 2));
	if (pos.x >= Map.Info.MapWidth || pos.y >= Map.Info.MapHeight || pos.x < 0 || pos.y < 0) {
		// out of bounds, doesn't have it
		lua_pushboolean(l, 0);
		return 1;
	}

	unsigned short flag = 0;
	const char *flag_name = LuaToString(l, 3);
	if (!strcmp(flag_name, "opaque")) {
		flag = MapFieldOpaque;
	} else if (!strcmp(flag_name, "water")) {
		flag = MapFieldWaterAllowed;
	} else if (!strcmp(flag_name, "land")) {
		flag = MapFieldLandAllowed;
	} else if (!strcmp(flag_name, "coast")) {
		flag = MapFieldCoastAllowed;
	} else if (!strcmp(flag_name, "no-building")) {
		flag = MapFieldNoBuilding;
	} else if (!strcmp(flag_name, "unpassable")) {
		flag = MapFieldUnpassable;
	} else if (!strcmp(flag_name, "wall")) {
		flag = MapFieldWall;
	} else if (!strcmp(flag_name, "rock")) {
		flag = MapFieldRocks;
	} else if (!strcmp(flag_name, "forest")) {
		flag = MapFieldForest;
	}

	const CMapField &mf = *Map.Field(pos);

	if (mf.getFlag() & flag) {
		lua_pushboolean(l, 1);
	} else {
		lua_pushboolean(l, 0);
	}

	return 1;
}

/**
**  Enable walls enabled for single player games (for debug purposes)
**
**  @param l  Lua state.
**
**  @return   0 for success, 1 for wrong type;
*/
static int CclSetEnableWallsForSP(lua_State *l)
{
	LuaCheckArgs(l, 1);
	EnableWallsInSinglePlayer = LuaToBoolean(l, 1);
	return 0;
}

/**
**  Check if walls enabled for single player games (for debug purposes)
*/
static int CclIsWallsEnabledForSP(lua_State *l)
{
	LuaCheckArgs(l, 0);
	lua_pushboolean(l, EnableWallsInSinglePlayer);
	return 1;
}

/**
**  Check if network game was created on this PC
*/
static int CclGetIsGameHoster(lua_State *l)
{
	LuaCheckArgs(l, 0);
	lua_pushboolean(l, (ThisPlayer->Index == Hosts[0].PlyNr) ? true : false);
	return 1;
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

	lua_register(Lua, "SetTileSize", CclSetTileSize);

	lua_register(Lua, "SetFogOfWar", CclSetFogOfWar);
	lua_register(Lua, "GetFogOfWar", CclGetFogOfWar);
	lua_register(Lua, "SetMinimapTerrain", CclSetMinimapTerrain);

	lua_register(Lua, "SetEnableMapGrid", CclSetEnableMapGrid);
	lua_register(Lua, "GetIsMapGridEnabled", CclGetIsMapGridEnabled);

	lua_register(Lua, "SetFieldOfViewType", CclSetFieldOfViewType);
	lua_register(Lua, "GetFieldOfViewType", CclGetFieldOfViewType);
	lua_register(Lua, "SetOpaqueFor", CclSetOpaqueFor);
	lua_register(Lua, "RemoveOpaqueFor", CclRemoveOpaqueFor);
	lua_register(Lua, "GetIsOpaqueFor", CclGetIsOpaqueFor);
	
	lua_register(Lua, "SetFogOfWarType", CclSetFogOfWarType);
	lua_register(Lua, "GetFogOfWarType", CclGetFogOfWarType);

	lua_register(Lua, "SetFogOfWarOpacityLevels", CclSetFogOfWarOpacityLevels);
	lua_register(Lua, "SetFogOfWarBlur", CclSetFogOfWarBlur);
	lua_register(Lua, "SetFogOfWarBilinear", CclSetFogOfWarBilinear);
	lua_register(Lua, "GetIsFogOfWarBilinear", CclGetIsFogOfWarBilinear);
	
	lua_register(Lua, "SetFogOfWarGraphics", CclSetFogOfWarGraphics);
	lua_register(Lua, "SetFogOfWarColor", CclSetFogOfWarColor);

	lua_register(Lua, "SetMMFogOfWarOpacityLevels", CclSetMMFogOfWarOpacityLevels);

	lua_register(Lua, "SetForestRegeneration", CclSetForestRegeneration);

	lua_register(Lua, "LoadTileModels", CclLoadTileModels);
	lua_register(Lua, "DefinePlayerTypes", CclDefinePlayerTypes);

	lua_register(Lua, "DefineTileset", CclDefineTileset);
	lua_register(Lua, "SetTileFlags", CclSetTileFlags);
	lua_register(Lua, "BuildTilesetTables", CclBuildTilesetTables);

	lua_register(Lua, "GetTileTerrainName", CclGetTileTerrainName);
	lua_register(Lua, "GetTileTerrainHasFlag", CclGetTileTerrainHasFlag);

	lua_register(Lua, "SetEnableWallsForSP", CclSetEnableWallsForSP);
	lua_register(Lua, "GetIsWallsEnabledForSP", CclIsWallsEnabledForSP);

	lua_register(Lua, "GetIsGameHoster", CclGetIsGameHoster);

}

//@}
