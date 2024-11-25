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
#include "script.h"
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
  Type = "SingleTile", -- MultiTile, Ramp 
  Shape = "Rectangular", -- Round
  Symmetric = false,
  Allign = "UpperLeft",  -- Center
  Resizable = true,
  ResizeSteps = {1, 1},
  MinSize = {1, 1},
  MaxSize = {20, 20},
  RandomizeAllowed = true,
  FixNeighborsAllowed = true
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
		const std::string_view value = LuaToString(l, -2);
		if (value == "Name") {
			name = std::string(LuaToString(l, -1));

		} else if (value == "Type") {
			const std::string_view type = LuaToString(l, -1);
			if (type == "SingleTile") {
				properties.type = CBrush::BrushTypes::SingleTile;
			} else if (type == "MultiTile") {
				properties.type = CBrush::BrushTypes::MultiTile;
			} else if (type == "Ramp") {
				properties.type = CBrush::BrushTypes::Ramp;
			} else {
				ErrorPrint("Incorrect brush type '%s'\n", LuaToString(l, -1).data());
			}

		} else if (value == "Shape") {
			const std::string_view shape = LuaToString(l, -1);
			if (shape == "Rectangular") {
				properties.shape = CBrush::BrushShapes::Rectangular;
			} else if (shape == "Round") {
				properties.shape = CBrush::BrushShapes::Round;
			} else {
				ErrorPrint("Incorrect brush shape '%s'\n", LuaToString(l, -1).data());
			}

		} else if (value == "Allign") {
			const std::string_view allign = LuaToString(l, -1);
			if (allign == "UpperLeft") {
				properties.allign = CBrush::BrushAllign::UpperLeft;
			} else if (allign == "Center") {
				properties.allign = CBrush::BrushAllign::Center;
			} else {
				ErrorPrint("Incorrect brush allign '%s'\n", LuaToString(l, -1).data());
			}

		} else if (value == "Resizable") {
			properties.resizable = LuaToBoolean(l, -1);

		} else if (value == "ResizeSteps") {
			if (!lua_istable(l, -1)) {
				ErrorPrint("Incorrect argument type: table expected. ['%s']\n", LuaToString(l, -1).data());
			}
			const int args = lua_rawlen(l, -1);
			if (args != 2) {
				ErrorPrint("Incorrect table size: {width, height} expected. ['%s']\n", LuaToString(l, -1).data());
			}
			properties.resizeSteps.width = LuaToUnsignedNumber(l, -1, 1);
			properties.resizeSteps.height = LuaToUnsignedNumber(l, -1, 2);

		} else if (value == "MinSize") {
			if (!lua_istable(l, -1)) {
				ErrorPrint("Incorrect argument type: table expected. ['%s']\n", LuaToString(l, -1).data());
			}
			const int args = lua_rawlen(l, -1);
			if (args != 2) {
				ErrorPrint("Incorrect table size: {width, height} expected. ['%s']\n", LuaToString(l, -1).data());
			}
			properties.minSize.width = LuaToUnsignedNumber(l, -1, 1);
			properties.minSize.height = LuaToUnsignedNumber(l, -1, 2);

		} else if (value == "MaxSize") {
			if (!lua_istable(l, -1)) {
				ErrorPrint("Incorrect argument type: table expected. ['%s']\n", LuaToString(l, -1).data());
			}
			const int args = lua_rawlen(l, -1);
			if (args != 2) {
				ErrorPrint("Incorrect table size: {width, height} expected. ['%s']\n", LuaToString(l, -1).data());
			}
			properties.maxSize.width = LuaToUnsignedNumber(l, -1, 1);
			properties.maxSize.height = LuaToUnsignedNumber(l, -1, 2);

		} else if (value == "RandomizeAllowed") {
			properties.randomizeAllowed = LuaToBoolean(l, -1);

		} else if (value == "FixNeighborsAllowed") {
			properties.fixNeighborsAllowed = LuaToBoolean(l, -1);
		}
	}
	Editor.brushes.addBrush(CBrush(name, properties));

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
				Editor.RandomTiles.push_back(std::make_tuple(LuaToNumber(l, -1, 1), LuaToNumber(l, -1, 2), LuaToNumber(l, -1, 3)));
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
				Editor.RandomUnits.push_back(std::make_tuple(std::string(LuaToString(l, -1, 1)), LuaToNumber(l, -1, 2), LuaToNumber(l, -1, 3), LuaToNumber(l, -1, 4)));
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
	lua_register(Lua, "SetEditorSelectIcon", CclSetEditorSelectIcon);
	lua_register(Lua, "SetEditorUnitsIcon", CclSetEditorUnitsIcon);
	lua_register(Lua, "SetEditorStartUnit", CclSetEditorStartUnit);
	lua_register(Lua, "EditorResizeMap", CclEditorResizeMap);

	lua_register(Lua, "EditorAddBrush", CclEditorAddBrush);

	lua_register(Lua, "SetEditorRandomizeProperties", CclEditorRandomizeProperties);
}

//@}
