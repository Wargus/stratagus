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
/**@name script_editor.c - Editor CCL functions. */
//
//      (c) Copyright 2002-2004 by Lutz Sammer
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
//      $Id$

//@{

/*----------------------------------------------------------------------------
--  Includes
----------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "stratagus.h"
#include "editor.h"
#include "script.h"

/*----------------------------------------------------------------------------
--  Defines
----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
--  Variables
----------------------------------------------------------------------------*/

global char* EditorSelectIcon;
global char* EditorUnitsIcon;

/*----------------------------------------------------------------------------
--  Functions
----------------------------------------------------------------------------*/

/**
**  Define an editor unit-type list.
**
**  @param l  Lua state.
*/
local int CclDefineEditorUnitTypes(lua_State* l)
{
	char** cp;
	int args;
	int j;

	if ((cp = EditorUnitTypes)) { // Free all old names
		while (*cp) {
			free(*cp++);
		}
		free(EditorUnitTypes);
	}

	//
	// Get new table.
	//
	args = lua_gettop(l);
	EditorUnitTypes = cp = malloc((args + 1) * sizeof(char*));
	MaxUnitIndex = args;
	for (j = 0; j < args; ++j) {
		*cp++ = strdup(LuaToString(l, j + 1));
	}
	*cp = NULL;

	return 0;
}

/**
**  Set the editor's select icon
**
**  @param l  Lua state.
*/
local int CclSetEditorSelectIcon(lua_State* l)
{
	if (lua_gettop(l) != 1) {
		LuaError(l, "incorrect argument");
	}
	free(EditorSelectIcon);
	EditorSelectIcon = strdup(LuaToString(l, 1));
	return 0;
}

/**
**  Set the editor's units icon
**
**  @param l  Lua state.
*/
local int CclSetEditorUnitsIcon(lua_State* l)
{
	if (lua_gettop(l) != 1) {
		LuaError(l, "incorrect argument");
	}
	free(EditorUnitsIcon);
	EditorUnitsIcon = strdup(LuaToString(l, 1));
	return 0;
}

/**
**  Register CCL features for the editor.
*/
global void EditorCclRegister(void)
{
	lua_register(Lua, "DefineEditorUnitTypes", CclDefineEditorUnitTypes);
	lua_register(Lua, "SetEditorSelectIcon", CclSetEditorSelectIcon);
	lua_register(Lua, "SetEditorUnitsIcon", CclSetEditorUnitsIcon);
}

//@}
