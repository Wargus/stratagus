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
/**@name script_editor.cpp - Editor CCL functions. */
//
//      (c) Copyright 2002-2006 by Lutz Sammer and Jimmy Salmon
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

#include "editor.h"
#include "script_sol.h"
#include "stratagus.h"
/*----------------------------------------------------------------------------
--  Variables
----------------------------------------------------------------------------*/

CEditor Editor;

/*----------------------------------------------------------------------------
--  Functions
----------------------------------------------------------------------------*/

/**
**  Set the editor's select icon
**
**  @param l  Lua state.
*/
static int CclSetEditorSelectIcon(lua_State *l)
{
	LuaCheckArgs(l, 1);
	Editor.Select.Name = LuaToString(l, 1);
	return 0;
}

/**
**  Set the editor's units icon
**
**  @param l  Lua state.
*/
static int CclSetEditorUnitsIcon(lua_State *l)
{
	LuaCheckArgs(l, 1);
	Editor.Units.Name = LuaToString(l, 1);
	return 0;
}

/**
**  Set the editor's start location unit
**
**  @param l  Lua state.
*/
static int CclSetEditorStartUnit(lua_State *l)
{
	LuaCheckArgs(l, 1);
	Editor.StartUnitName = LuaToString(l, 1);
	return 0;
}

static int CclEditorResizeMap(lua_State *l)
{
	LuaCheckArgs(l, 5);
	int res = EditorSaveMapWithResize(LuaToString(l, 1),
							{(short)LuaToNumber(l, 2), (short)LuaToNumber(l, 3)},
							{(short)LuaToNumber(l, 4), (short)LuaToNumber(l, 5)});
	lua_pushnumber(l, res);
	return 1;
}

/*
  Name = "Default",
  Type = "SingleTile", -- or Decoration
  Shape = "Rectangular", -- or Round
  Symmetric = false,
  Align = "UpperLeft",  -- or Center
  Resizable = true,
  ResizeSteps = {1, 1},
  MinSize = {1, 1},
  MaxSize = {20, 20},
  RandomizeAllowed = true,
  FixNeighborsAllowed = true,
  TileIconsPaletteRequired = true,
  ExtendedTilesetRequired = true,
  Generator = { -- decoration generator options
    ["source"] = "scripts/editor/brushes/_generator_.lua", -- path to generator file
    ["option"] = {"value1", "value2" ...} -- option : {possible values} pairs, values type is string
  }
*/

static int CclEditorAddBrush(lua_State *l)
{

	LuaCheckArgs(l, 1);
	if (!lua_istable(l, 1)) {
		LuaError(l, "incorrect argument");
	}

	CBrush::Properties properties;
	std::string name("Default");

	for (lua_pushnil(l); lua_next(l, 1); lua_pop(l, 1)) {
		const std::string_view key = LuaToString(l, -2);
		if (key == "Name") {
			name = std::string(LuaToString(l, -1));

		} else if (key == "Type") {
			properties.type = CBrush::convertToEnumTypes(LuaToString(l, -1));

			if (properties.type == CBrush::EBrushTypes::NotSet) {
				ErrorPrint("Incorrect brush type '%s'\n", LuaToString(l, -1).data());
				return 0;
			}
		} else if (key == "Shape") {
			properties.shape = CBrush::convertToEnumShapes(LuaToString(l, -1));

			if (properties.shape == CBrush::EBrushShapes::NotSet) {
				ErrorPrint("Incorrect brush shape '%s'\n", LuaToString(l, -1).data());
				return 0;
			}
		} else if (key == "Symmetric") {
			properties.symmetric = LuaToBoolean(l, -1);

		} else if (key == "Align") {
			properties.align = CBrush::convertToEnumAlign(LuaToString(l, -1));

			if (properties.align == CBrush::EBrushAlign::NotSet) {
				ErrorPrint("Incorrect brush align '%s'\n", LuaToString(l, -1).data());
				return 0;
			}
		} else if (key == "Resizable") {
			properties.resizable = LuaToBoolean(l, -1);

		} else if (key == "ResizeSteps" || key == "MinSize" || key == "MaxSize") {
			if (!lua_istable(l, -1)) {
				ErrorPrint("Incorrect argument type: table expected. ['%s']\n",
							LuaToString(l, -1).data());
				return 0;
			}
			if (const int args = lua_rawlen(l, -1); args != 2) {
				ErrorPrint("Incorrect table size: {width, height} expected. ['%s']\n",
							LuaToString(l, -1).data());
				return 0;
			}
			auto &property = key == "ResizeSteps" ? properties.resizeSteps
						   : key == "MinSize"	  ? properties.minSize
												  : properties.maxSize;

			property = {uint8_t(LuaToUnsignedNumber(l, -1, 1)),
						uint8_t(LuaToUnsignedNumber(l, -1, 2))};

		} else if (key == "RandomizeAllowed") {
			properties.randomizeAllowed = LuaToBoolean(l, -1);
		} else if (key == "FixNeighborsAllowed") {
			properties.fixNeighborsAllowed = LuaToBoolean(l, -1);
		} else if (key == "TileIconsPaletteRequired") {
			properties.tileIconsPaletteRequired = LuaToBoolean(l, -1);
		} else if (key == "ExtendedTilesetRequired") {
			properties.extendedTilesetRequired = LuaToBoolean(l, -1);
			if(properties.extendedTilesetRequired && !Map.Info.IsHighgroundsEnabled()) {
				ErrorPrint("Unable to load brush: extended tileset required, but loaded base.\n");
				return 0;
			}
		} else if (key == "Generator") {
			if (!lua_istable(l, -1)) {
				ErrorPrint("Incorrect argument type: table expected. ['%s']\n",
							LuaToString(l, -1).data());
				return 0;
			}
			for (lua_pushnil(l); lua_next(l, -2); lua_pop(l, 1)) {

				const std::string_view option = LuaToString(l, -2);

				if (option == "source") {
					properties.decorationGenerator.source = LuaToString(l, -1);
				} else {
					std::vector <CBrush::TDecorationOptionValue> values;
					if (lua_istable(l, -1)) {
						const int valuesCount = lua_rawlen(l, -1);
						for (int i = 1; i <= valuesCount; i++) {
							values.emplace_back(LuaToString(l, -1, i));
						}
					} else {
						values.emplace_back(LuaToString(l, -1));
					}
					if (values.size() == 0) {
						ErrorPrint("Unable to parse possible values for generator option \"%s\".\n",
						           option);
						return 0;
					}
					properties.decorationGenerator.options.emplace(option, values);
				}
			}
		}
	}
	Editor.brushes.addBrush(CBrush(name, properties));

	return 0;
}

int CclEditorBrush_GetGeneratorOption(lua_State *l)
{
	LuaCheckArgs(l, 1);
	const std::string option {LuaToString(l, 1)};
	lua_pushstring(l, Editor.brushes.getCurrentBrush().getDecorationOption(option).c_str());
	return 1;
}

int CclEditorBrush_LoadDecorationTiles(lua_State *l)
{
	auto parseTiles = [&l]() { // at (l, -1) should be a table
		std::vector<tile_index> result;

		const int rows = lua_rawlen(l, -1);
		for (int i = 1; i <= rows; i++) {
			lua_rawgeti(l, -1, i);
			if (!lua_istable(l, -1)) {
				LuaError(l, "Incorrect argument type: table expected.");
				break;
			}
			const int cols = lua_rawlen(l, -1);
			for (int col = 1; col <= cols; col++) {
				const tile_index tile = LuaToUnsignedNumber(l, -1, col);
				result.push_back(tile);
			}
			lua_pop(l, 1);
		}
		return result;
	};

	LuaCheckArgs(l, 1);
	if (!lua_istable(l, 1)) {
		LuaError(l, "incorrect argument");
	}

	uint16_t width = 0;
	uint16_t height = 0;
	std::vector<tile_index> tiles;

	for (lua_pushnil(l); lua_next(l, 1); lua_pop(l, 1)) {
		const std::string_view key = LuaToString(l, -2);
		if (key == "width") {
			width = LuaToNumber(l, -1);
		} else if (key == "height") {
			height = LuaToNumber(l, -1);
		} else if (key == "tiles") {
			if (lua_istable(l, -1)) {
				tiles = parseTiles();
			} else {
				LuaError(l, "Incorrect argument type for ['%s']: table expected.", key.data());
			}
		}  else {
			LuaError(l, "Unsupported tag: %s", key.data());
		}
	}
	if (tiles.size() == width * height) {
		Editor.brushes.getCurrentBrush().pushDecorationTiles(width, height, tiles);
	}

	return 0;
}
/**
**  Configure the randomize map feature of the editor.
**
**  @param l  Lua state.
*/
static int CclEditorRandomizeProperties(lua_State *l)
{
	LuaCheckArgs(l, 1);
	if (!lua_istable(l, 1)) {
		LuaError(l, "incorrect argument");
	}

	Editor.RandomTiles.clear();
	Editor.RandomUnits.clear();

	for (lua_pushnil(l); lua_next(l, 1); lua_pop(l, 1)) {
		const std::string_view value = LuaToString(l, -2);
		if (value == "BaseTile") {
			Editor.BaseTileIndex = LuaToNumber(l, -1);
		} else if (value == "RandomTiles") {
			if (!lua_istable(l, -1)) {
				LuaError(l, "incorrect argument");
			}
			int subargs = lua_rawlen(l, -1);
			for (int j = 0; j < subargs; j++) {
				lua_rawgeti(l, -1, j + 1);
				if (!lua_istable(l, -1)) {
					LuaError(l, "incorrect argument");
				}
				if (lua_rawlen(l, -1) != 3) {
					LuaError(l, "incorrect RandomTiles entry length, need 3 integers");
				}
				Editor.RandomTiles.push_back(std::make_tuple(LuaToNumber(l, -1, 1),
															 LuaToNumber(l, -1, 2),
															 LuaToNumber(l, -1, 3)));
				lua_pop(l, 1);
			}
		} else if (value == "RandomUnits") {
			if (!lua_istable(l, -1)) {
				LuaError(l, "incorrect argument");
			}
			int subargs = lua_rawlen(l, -1);
			for (int j = 0; j < subargs; j++) {
				lua_rawgeti(l, -1, j + 1);
				if (!lua_istable(l, -1)) {
					LuaError(l, "incorrect argument");
				}
				if (lua_rawlen(l, -1) != 4) {
					LuaError(l, "incorrect RandomUnits entry length, need 1 string followed by 3 integers");
				}
				Editor.RandomUnits.push_back(std::make_tuple(std::string(LuaToString(l, -1, 1)),
																		 LuaToNumber(l, -1, 2),
																		 LuaToNumber(l, -1, 3),
																		 LuaToNumber(l, -1, 4)));
				lua_pop(l, 1);
			}
		}
	}
	return 0;
}

/**
**  Register CCL features for the editor.
*/
void EditorCclRegister()
{
	sol::state_view luaSol(Lua);

	luaSol["SetEditorSelectIcon"] = CclSetEditorSelectIcon;
	luaSol["SetEditorUnitsIcon"] = CclSetEditorUnitsIcon;
	luaSol["SetEditorStartUnit"] = CclSetEditorStartUnit;
	luaSol["EditorResizeMap"] = CclEditorResizeMap;

	luaSol["EditorAddBrush"] = CclEditorAddBrush;
	luaSol["EditorBrush_GetGeneratorOption"] = CclEditorBrush_GetGeneratorOption;
	luaSol["EditorBrush_LoadDecorationTiles"] = CclEditorBrush_LoadDecorationTiles;

	luaSol["SetEditorRandomizeProperties"] = CclEditorRandomizeProperties;
}

//@}
