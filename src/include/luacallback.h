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

#include <map>
#include <string>
#include <string_view>
#include <tuple>
#include <type_traits>
#include <utility>
#include <vector>
#include <variant>

using lua_Object = int; // from tolua++.h
struct lua_State;

class LuaCallback
{
public:
	LuaCallback(lua_State *lua, lua_Object luaref);
	~LuaCallback();
	void pushPreamble();
	void pushInteger(int value);
	void pushIntegers(const std::vector<int> &values);
	void pushString(std::string_view s);
	void pushTable(std::initializer_list<std::pair<std::string, std::variant<std::string, int>>> list);
	void pushTable(std::map<std::string, std::variant<std::string, int>> map);
	void run(int results = 0);
	bool popBoolean();
	int popInteger();

	template <typename T>
	void pushT(const T &arg)
	{
		if constexpr (std::is_integral_v<T>) {
			return pushInteger(arg);
		} else if constexpr (std::is_same_v<T, std::vector<int>>) {
			return pushIntegers(arg);
		} else if constexpr (std::is_convertible_v<T, std::string_view>) {
			return pushString(arg);
		} else {
			return pushTable(arg);
		}
	}

	template<typename T>
	T popT()
	{
		if constexpr (std::is_same_v<T, bool>) {
			return popBoolean();
		} else if constexpr (std::is_same_v<T, int>) {
			return popInteger();
		}
	}

	template <typename... Res, typename... Args>
	auto call(const Args &...args)
	{
		pushPreamble();
		(pushT(args), ...);
		run(sizeof...(Res));

		if constexpr (sizeof...(Res) <= 1) {
			return (popT<Res>(), ...);
		} else {
			return std::tuple<Res...>{popT<Res>()...};
		}
	}

private:
	lua_State *luastate;
	int luaref;
	int arguments;
	int rescount;
	int base;
};

#endif
