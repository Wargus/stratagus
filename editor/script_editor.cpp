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
/**@name ccl_editor.c - Editor CCL functions. */
//
//      (c) Copyright 2002-2004 by Lutz Sammer
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "stratagus.h"
#include "editor.h"
#include "ccl.h"

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
**  @param list  List of all names.
*/
#if defined(USE_GUILE) || defined(USE_SIOD)
local SCM CclDefineEditorUnitTypes(SCM list)
{
	int i;
	char** cp;

	if ((cp = EditorUnitTypes)) { // Free all old names
		while (*cp) {
			free(*cp++);
		}
		free(EditorUnitTypes);
	}

	//
	// Get new table.
	//
	i = gh_length(list);
	EditorUnitTypes = cp = malloc((i + 1) * sizeof(char*));
	MaxUnitIndex = i;
	while (i--) {
		*cp++ = gh_scm2newstr(gh_car(list), NULL);
		list = gh_cdr(list);
	}
	*cp = NULL;

	return SCM_UNSPECIFIED;
}
#elif defined(USE_LUA)
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
#endif

/**
**  Set the editor's select icon
*/
#if defined(USE_GUILE) || defined(USE_SIOD)
local SCM CclSetEditorSelectIcon(SCM icon)
{
	free(EditorSelectIcon);
	EditorSelectIcon = gh_scm2newstr(icon, NULL);
	return SCM_UNSPECIFIED;
}
#elif defined(USE_LUA)
local int CclSetEditorSelectIcon(lua_State* l)
{
	if (lua_gettop(l) != 1) {
		lua_pushstring(l, "incorrect argument");
		lua_error(l);
	}
	free(EditorSelectIcon);
	EditorSelectIcon = strdup(LuaToString(l, 1));
	return 0;
}
#endif

/**
**  Set the editor's units icon
*/
#if defined(USE_GUILE) || defined(USE_SIOD)
local SCM CclSetEditorUnitsIcon(SCM icon)
{
	free(EditorUnitsIcon);
	EditorUnitsIcon = gh_scm2newstr(icon, NULL);
	return SCM_UNSPECIFIED;
}
#elif defined(USE_LUA)
local int CclSetEditorUnitsIcon(lua_State* l)
{
	if (lua_gettop(l) != 1) {
		lua_pushstring(l, "incorrect argument");
		lua_error(l);
	}
	free(EditorUnitsIcon);
	EditorUnitsIcon = strdup(LuaToString(l, 1));
	return 0;
}
#endif

/**
**  Register CCL features for the editor.
*/
global void EditorCclRegister(void)
{
#if defined(USE_GUILE) || defined(USE_SIOD)
	gh_new_procedureN("define-editor-unittypes", CclDefineEditorUnitTypes);
	gh_new_procedure1_0("set-editor-select-icon!", CclSetEditorSelectIcon);
	gh_new_procedure1_0("set-editor-units-icon!", CclSetEditorUnitsIcon);
#elif defined(USE_LUA)
	lua_register(Lua, "DefineEditorUnitTypes", CclDefineEditorUnitTypes);
	lua_register(Lua, "SetEditorSelectIcon", CclSetEditorSelectIcon);
	lua_register(Lua, "SetEditorUnitsIcon", CclSetEditorUnitsIcon);
#endif
}

//@}
