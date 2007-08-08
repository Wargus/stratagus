//     ____                _       __               
//    / __ )____  _____   | |     / /___ ___________
//   / __  / __ \/ ___/   | | /| / / __ `/ ___/ ___/
//  / /_/ / /_/ (__  )    | |/ |/ / /_/ / /  (__  ) 
// /_____/\____/____/     |__/|__/\__,_/_/  /____/  
//                                              
//       A futuristic real-time strategy game.
//          This file is part of Bos Wars.
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
//      $Id$

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
	delete[] Editor.StartUnitName;
	Editor.StartUnitName = new_strdup(LuaToString(l, 1));
	return 0;
}

/**
**  Register CCL features for the editor.
*/
void EditorCclRegister(void)
{
	lua_register(Lua, "SetEditorSelectIcon", CclSetEditorSelectIcon);
	lua_register(Lua, "SetEditorUnitsIcon", CclSetEditorUnitsIcon);
	lua_register(Lua, "SetEditorStartUnit", CclSetEditorStartUnit);
}

//@}
