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

	/// Script get/set function prototype with string key. The value is on the lua stack.
typedef int ScriptGetSetStrFunction(void* object, const char* key, lua_State* l);
	/// Script get/set function prototype with int index. The value is on the lua stack.
typedef int ScriptGetSetIntFunction(void* object, int index, lua_State* l);
	/// Script garbage collector function prototype.
typedef int ScriptCollectFunction(void* object);

	/// Structure for a script proxy type. Make one of those for every scriptable struct.
typedef struct {
	ScriptGetSetStrFunction* GetStr;        /// Get function with strings.
	ScriptGetSetStrFunction* SetStr;        /// Set function with strings.
	ScriptGetSetIntFunction* GetInt;        /// Get function with int index.
	ScriptGetSetIntFunction* SetInt;        /// Set function with int index.
	ScriptCollectFunction* Collect;         /// Garbage collection function.
} ScriptProxyType;

	/// Structure for a script proxy. Don't mess with this outside of scripting
typedef struct {
	void* Object;                                   /// The actual Object
	ScriptProxyType* Type;                          /// Type information
} ScriptProxy;

	/// Userdata Constructor. Push userdata on the stack.
extern void ScriptCreateUserdata(lua_State* l, void* object, ScriptProxyType* type);
	/// Init ScriptProxyType with all blockers.
extern void ScriptProxyTypeInitBlock(ScriptProxyType* type);
	/// Really dumb set function that always goes into an error, with string key
extern int ScriptGetSetStrBlock(void* object, const char* key, lua_State* l);
	/// Really dumb set function that always goes into an error, with int index
extern int ScriptGetSetIntBlock(void* object, int index, lua_State* l);

/*----------------------------------------------------------------------------
--  Quick macros for meta lua. Use them in well-formed get/set functions.
----------------------------------------------------------------------------*/

    /// Quick way to fail in a function. You can use _C_ like in DebugLevelx
#define LuaError(l, args) \
	{ lua_pushfstring(l, args); lua_error(l); return 0; }

    /// Quick way to check the number of arguments
#define LuaCheckArgCount(l, argcount) \
{ \
	if (lua_gettop(l) != (argcount)) { \
		LuaError(l, "Wrong number of arguments, expected %d got %d" \
				_C_ (argcount) _C_ lua_gettop(l)); \
	} \
}

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

#define META_GET_FUNC(keyval, v) \
{ \
	if (!strcmp(key, keyval)) { \
		lua_pushcfunction(l, v); \
		return 1; \
	} \
}

/*
#define META_SET_FUNC(keyval, v) \
{ \
	if (!strcmp(key, keyval)) { \
		luaL_checktype(l, -1, LUA_TBOOLEAN); \
		v = lua_toboolean(l, -1); \
		return 0; \
	} \
}*/

#define META_GET_USERDATA(keyval, obj, type) \
{ \
	if (!strcmp(key, keyval)) { \
		ScriptCreateUserdata(l, obj, type); \
		return 1; \
	} \
}

#endif // META_LUA

//@}

#endif // !__CCL_H__
