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

#define LuaError(l, args) \
	do { \
		fprintf(stdout, "%s:%d: ", __FILE__, __LINE__); \
		fprintf(stdout, args); \
		fprintf(stdout, "\n"); \
		lua_pushfstring(l, args); lua_error(l); \
	} while (0)


#include "iolib.h"

/*----------------------------------------------------------------------------
--  Variables
----------------------------------------------------------------------------*/

extern char* CclStartFile;   ///< CCL start file
extern int CclInConfigFile;  ///< True while config file parsing

/*----------------------------------------------------------------------------
--  Functions
----------------------------------------------------------------------------*/

extern const char* LuaToString(lua_State* l, int narg);
extern lua_Number LuaToNumber(lua_State* l, int narg);
extern int LuaToBoolean(lua_State* l, int narg);

extern void CclGarbageCollect(int fast);  ///< Perform garbage collection
extern void InitCcl(void);                ///< Initialise ccl
extern void LoadCcl(void);                ///< Load ccl config file
extern void SaveCcl(CLFile* file);        ///< Save CCL module
extern void SavePreferences(void);        ///< Save user preferences
extern int CclCommand(const char*);       ///< Execute a ccl command
extern void CclFree(void*);               ///< Save free
extern void CleanCclCredits();            ///< Free Ccl Credits Memory

//@}

#endif // !__CCL_H__
