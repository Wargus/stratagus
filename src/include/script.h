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
/**@name script.h - The clone configuration language headerfile. */
//
//      (c) Copyright 1998-2004 by Lutz Sammer and Jimmy Salmon
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

#ifndef __SCRIPT_H__
#define __SCRIPT_H__

//@{

/*----------------------------------------------------------------------------
--  Includes
----------------------------------------------------------------------------*/

#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"

typedef struct _lua_user_data_ {
	int Type;
	void* Data;
} LuaUserData;

enum {
	LuaUnitType = 100,
	LuaSoundType,
};

extern lua_State* Lua;

extern int LuaLoadFile(const char* file);
extern int LuaCall(int narg, int clear);



#include "iolib.h"

/*----------------------------------------------------------------------------
--  Variables
----------------------------------------------------------------------------*/

extern char* CclStartFile;   /// CCL start file
extern int CclInConfigFile;  /// True while config file parsing

/*----------------------------------------------------------------------------
--  Functions
----------------------------------------------------------------------------*/

extern const char* LuaToString(lua_State* l, int narg);
extern lua_Number LuaToNumber(lua_State* l, int narg);
extern int LuaToBoolean(lua_State* l, int narg);

extern void CclGarbageCollect(int fast);  /// Perform garbage collection
extern void InitCcl(void);                /// Initialise ccl
extern void LoadCcl(void);                /// Load ccl config file
extern void SaveCcl(CLFile* file);        /// Save CCL module
extern void SavePreferences(void);        /// Save user preferences
extern int CclCommand(const char*);      /// Execute a ccl command
extern void CclFree(void*);               /// Save free
extern void CleanCclCredits();            /// Free Ccl Credits Memory

#ifdef META_LUA

/*----------------------------------------------------------------------------
--  Functions and data structures.
----------------------------------------------------------------------------*/

	/// Script get/set function prototype. The values is on the lua stack.
typedef int ScriptGetSetFunction(void* object, const char* key, lua_State* l);

	/// Structure for a script proxy. Don't mess with this outside of scripting
typedef struct {
	void* Object;                          /// The actual Object
	ScriptGetSetFunction* GetFunction;        /// Get function
	ScriptGetSetFunction* SetFunction;        /// Set function
} ScriptProxy;

	/// Userdata Constructor.
extern void ScriptDoCreateUserdata(lua_State* l, void* Object,
		ScriptGetSetFunction* GetFunction, ScriptGetSetFunction* SetFunction);
	/// Really dumb set function that always goes into an error.
extern int ScriptSetValueBlock(lua_State* l);

/*----------------------------------------------------------------------------
--  Quick macros for meta lua. Use them in well-formed get/set functions.
----------------------------------------------------------------------------*/

	/// Macro to call ScriptDoCreateUserdata w/o casts.
#define ScriptCreateUserdata(l, o, g, s) (ScriptDoCreateUserdata(l, o, \
		(ScriptGetSetFunction*)(g), (ScriptGetSetFunction*)(s)))

    /// Quick way to fail in a function. You can use _C_ like in DebugLevelx
#ifdef DEBUG
#define LuaError(l, args) \
	{ lua_pushfstring(l, args); lua_error(l); return 0; }
#else
	/// Save on memory.
#define LuaError(l, args) \
	{ lua_pushstring(l, "Lua error"); lua_error(l); return 0; }
#endif

//
//	Pushing 0 as a string to lua is ok. strdup-ing 0 is not.
//
#define META_GET_STRING(keyval, v) \
{ \
	if (!strcmp(key, keyval)) { \
		if (v) { \
			lua_pushstring(l, strdup(v)); \
		} else { \
			lua_pushstring(l, 0); \
		} \
		return 1; \
	} \
}

#define META_SET_STRING(keyval, v) \
{ \
	if (!strcmp(key, keyval)) { \
		luaL_checktype(l, -1, LUA_TSTRING); \
		v = strdup(lua_tostring(l, -1)); \
		return 0; \
	} \
}

#define META_GET_INT(keyval, v) \
{ \
	if (!strcmp(key, keyval)) { \
		lua_pushnumber(l, v); \
		return 1; \
	} \
}

#define META_SET_INT(keyval, v) \
{ \
	if (!strcmp(key, keyval)) { \
		luaL_checktype(l, -1, LUA_TNUMBER); \
		v = lua_tonumber(l, -1); \
		return 0; \
	} \
}

#define META_GET_BOOL(keyval, v) \
{ \
	if (!strcmp(key, keyval)) { \
		lua_pushboolean(l, v); \
		return 1; \
	} \
}

#define META_SET_BOOL(keyval, v) \
{ \
	if (!strcmp(key, keyval)) { \
		luaL_checktype(l, -1, LUA_TBOOLEAN); \
		v = lua_toboolean(l, -1); \
		return 0; \
	} \
}

#endif // META_LUA

//@}

#endif // !__CCL_H__
