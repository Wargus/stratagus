//     ____                _       __               
//    / __ )____  _____   | |     / /___ ___________
//   / __  / __ \/ ___/   | | /| / / __ `/ ___/ ___/
//  / /_/ / /_/ (__  )    | |/ |/ / /_/ / /  (__  ) 
// /_____/\____/____/     |__/|__/\__,_/_/  /____/  
//                                              
//       A futuristic real-time strategy game.
//          This file is part of Bos Wars.
//
/**@name script.h - The configuration language headerfile. */
//
//      (c) Copyright 1998-2008 by Lutz Sammer and Jimmy Salmon
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

#ifndef __SCRIPT_H__
#define __SCRIPT_H__

//@{

/*----------------------------------------------------------------------------
--  Includes
----------------------------------------------------------------------------*/

#include <string>

#ifdef __cplusplus
extern "C" {
#endif
#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"
#ifdef __cplusplus
}
#endif

/*----------------------------------------------------------------------------
--  Declarations
----------------------------------------------------------------------------*/

class CFile;

typedef struct _lua_user_data_ {
	int Type;
	void *Data;
} LuaUserData;

enum {
	LuaUnitType = 100,
	LuaSoundType,
};

extern lua_State *Lua;

extern int LuaLoadFile(const std::string &file);
extern int LuaCall(int narg, int clear, bool exitOnError = true);

#define LuaError(l, args) \
	do { \
		fprintf(stdout, "%s:%d: ", __FILE__, __LINE__); \
		fprintf(stdout, args); \
		fprintf(stdout, "\n"); \
		lua_pushfstring(l, args); lua_error(l); \
	} while (0)

#define LuaCheckArgs(l, args) \
	do { \
		if (lua_gettop(l) != args) { \
			LuaError(l, "incorrect argument"); \
		} \
	} while (0)

#define LuaCheckTable(l, index) \
	do { \
		if (!lua_istable((l), (index))) { \
			LuaError(l, "incorrect argument"); \
		} \
	} while (0)

#define LuaCheckTableSize(l, index, size) \
	do { \
		if (!lua_istable((l), (index)) || lua_objlen((l), (index)) != (size)) { \
			LuaError(l, "incorrect argument"); \
		} \
	} while (0)

/*----------------------------------------------------------------------------
--  Variables
----------------------------------------------------------------------------*/

extern std::string CclStartFile;   /// CCL start file
extern int CclInConfigFile;        /// True while config file parsing

/*----------------------------------------------------------------------------
--  Functions
----------------------------------------------------------------------------*/

extern const char *LuaToString(lua_State *l, int index);
extern const char *LuaToString(lua_State *l, int tableIndex, int stringIndex);
extern int LuaToNumber(lua_State *l, int index);
extern int LuaToNumber(lua_State *l, int tableIndex, int numberIndex);
extern bool LuaToBoolean(lua_State *l, int index);
extern int luatraceback(lua_State *L);

extern void CclGarbageCollect(int fast);    /// Perform garbage collection
extern void InitCcl(void);                  /// Initialise ccl
extern void LoadCcl(void);                  /// Load ccl config file
extern void SaveCcl(CFile *file);           /// Save CCL module
extern void SavePreferences(void);          /// Save user preferences
extern char *SaveGlobal(lua_State *l, bool is_root); /// For saving lua state
extern int CclCommand(const std::string &command, bool exitOnError = true);   /// Execute a ccl command

//@}

#endif // !__SCRIPT_H__
