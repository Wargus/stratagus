//     ____                _       __               
//    / __ )____  _____   | |     / /___ ___________
//   / __  / __ \/ ___/   | | /| / / __ `/ ___/ ___/
//  / /_/ / /_/ (__  )    | |/ |/ / /_/ / /  (__  ) 
// /_____/\____/____/     |__/|__/\__,_/_/  /____/  
//                                              
//       A futuristic real-time strategy game.
//          This file is part of Bos Wars.
//
/**@name luacallback.cpp. */
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
//
//      $Id: widgets.cpp 9391 2008-01-20 12:59:30Z feb $

//@{


#include "luacallback.h"
#include "stratagus.h"
#include "script.h"


/**
**  LuaCallback constructor
**
**  @param l  Lua state
**  @param f  Listener function
*/
LuaCallback::LuaCallback(lua_State *l, lua_Object f) :
	luastate(l)
{
	if (!lua_isfunction(l, f)) {
		LuaError(l, "Argument isn't a function");
		Assert(0);
	}
	lua_pushvalue(l, f);
	luaref = luaL_ref(l, LUA_REGISTRYINDEX);
}

/**
**  Push the preamble on the stack to call the callback.
**  Call this function before pushing the arguments on the lua stack.
*/
void LuaCallback::pushPreamble() 
{
	base = lua_gettop(luastate);
	lua_getglobal(luastate, "_TRACEBACK");
	lua_rawgeti(luastate, LUA_REGISTRYINDEX, luaref);
}


/**
**  Push a string argument for the callback on the stack.
**
**  @param value  the integer to push on the stack
*/
void LuaCallback::pushInteger(int value) 
{
	lua_pushnumber(luastate, value);
}


/**
**  Push a string argument for the callback on the stack.
**
**  @param s  the string to push on the stack
*/
void LuaCallback::pushString(const std::string &s) 
{
	lua_pushstring(luastate, s.c_str());
}


/**
**  Called when an action is recieved from a Widget. It is used
**  to be able to recieve a notification that an action has
**  occured.
*/
void LuaCallback::run() 
{
	int status;

	 //FIXME call error reporting function
	status = lua_pcall(luastate, 1, 0, base);
	if (status) {
		const char *msg;
		msg = lua_tostring(luastate, -1);
		if (msg == NULL) {
			msg = "(error with no message)";
		}
		fprintf(stderr, "%s\n", msg);
		lua_pop(luastate, 1);
	}
}

/**
**  LuaActionListener destructor
*/
LuaCallback::~LuaCallback()
{
	luaL_unref(luastate, LUA_REGISTRYINDEX, luaref);
}

//@}
