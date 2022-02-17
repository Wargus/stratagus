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

#include "stratagus.h"
#include "editor.h"
#include "script.h"

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

/**
**  Confgure the randomize map feature of the editor.
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
		const char *value = LuaToString(l, -2);
		if (!strcmp(value, "BaseTile")) {
			Editor.BaseTileIndex = LuaToNumber(l, -1);
		} else if (!strcmp(value, "RandomTiles")) {
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
		} else if (!strcmp(value, "RandomUnits")) {
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

	lua_register(Lua, "SetEditorRandomizeProperties", CclEditorRandomizeProperties);
}

//@}
