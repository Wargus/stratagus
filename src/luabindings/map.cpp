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
/**@name map.cpp. Bindings for map related code to lua */
//
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

//@{

/*----------------------------------------------------------------------------
--  Includes
----------------------------------------------------------------------------*/
#include "map.h"
#include "script.h"
#include "script_sol.h"

/*----------------------------------------------------------------------------
--  Functions
----------------------------------------------------------------------------*/
/**

class CMapInfo
{
	string Description;
	string Filename;
        string Preamble;
        string Postamble;
	int MapWidth;
	int MapHeight;
	PlayerTypes PlayerType[PlayerMax];
};

class CTileset
{
	string Name;
};


class CMap
{
	tolua_readonly CMapInfo Info;
	tolua_readonly CTileset Tileset;
};

extern CMap Map;

void SetTile(int tile, int w, int h, int value = 0, int elevation = 0);

**/

void ToLuaBind_Map()
{
	sol::state_view luaSol(Lua);

	/// CMapInfo
	sol::usertype<CMapInfo> Lua_CMapInfo = luaSol.new_usertype<CMapInfo>("CMapInfo");
	Lua_CMapInfo["Description"] = &CMapInfo::Description;
	Lua_CMapInfo["Filename"] = &CMapInfo::Filename;
	Lua_CMapInfo["Preamble"] = &CMapInfo::Preamble;
	Lua_CMapInfo["Postamble"] = &CMapInfo::Postamble;
	Lua_CMapInfo["MapWidth"] = &CMapInfo::MapWidth;
	Lua_CMapInfo["MapHeight"] = &CMapInfo::MapHeight;
	Lua_CMapInfo["PlayerType"] = &CMapInfo::PlayerType;	

	/// CTileset
	sol::usertype<CTileset> Lua_CTileset = luaSol.new_usertype<CTileset>("CTileset");
	Lua_CTileset["Name"] = &CTileset::Name;

	/// CMap
	sol::usertype<CMap> Lua_CMap = luaSol.new_usertype<CMap>("CMap");
	Lua_CMap.set("Info", sol::readonly(&CMap::Info));
	Lua_CMap.set("Tileset", sol::readonly(&CMap::Tileset));

	luaSol["Map"] = &Map;

	/// lua do not supply default args for c++ functions,
	/// so we should wrap them with set of overloaded functions;
	luaSol.set_function("SetTile",
						sol::overload( 
							[](unsigned int tile, int x, int y)
								{ SetTile(tile, x, y); },
							[](unsigned int tile, int x, int y, int value)
								{ SetTile(tile, x, y, value); },
							[](unsigned int tile, int x, int y, int value, int elevation)
								{ SetTile(tile, x, y, value, elevation); }));
}
//@}
