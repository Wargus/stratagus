//     ____                _       __               
//    / __ )____  _____   | |     / /___ ___________
//   / __  / __ \/ ___/   | | /| / / __ `/ ___/ ___/
//  / /_/ / /_/ (__  )    | |/ |/ / /_/ / /  (__  ) 
// /_____/\____/____/     |__/|__/\__,_/_/  /____/  
//                                              
//       A futuristic real-time strategy game.
//          This file is part of Bos Wars.
//
/**@name luacallback.h. */
//
//      (c) Copyright 2008 by Francois Beerten
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

#ifndef LUA_CALLBACK_HEADER_FILE
#define LUA_CALLBACK_HEADER_FILE

#include <string>

typedef int lua_Object; // from tolua++.h
struct lua_State;

class LuaCallback
{
	lua_State *luastate;

	// Integer key of the callback function in the Lua registry.
	int luaref;

	// Number of arguments pushed for the function.
	int arguments;

	// The top of the Lua stack before pushPreamble().
	int base;

public:
	LuaCallback(lua_State *lua, lua_Object luaref);
	virtual ~LuaCallback();
	virtual void pushPreamble();
	virtual void pushInteger(int value);
	virtual void pushString(const std::string &eventId);
	virtual void run();
};

#endif

