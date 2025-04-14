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
/**@name minimap.cpp. Bindings for minimap related code to lua */
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
#include "minimap.h"
#include "script.h"
#include "script_sol.h"

/*----------------------------------------------------------------------------
--  Functions
----------------------------------------------------------------------------*/
/**

class CMinimap
{
	int X;
	int Y;
	int W;
	int H;
	bool WithTerrain;
	bool ShowSelected;
	bool Transparent;
};

**/

void ToLuaBind_Minimap()
{
	sol::state_view luaSol(Lua);

	/// CMinimap
	sol::usertype<CMinimap> Lua_CMinimap = luaSol.new_usertype<CMinimap>("CMinimap");
	Lua_CMinimap["X"] = &CMinimap::X;
	Lua_CMinimap["Y"] = &CMinimap::Y;
	Lua_CMinimap["W"] = &CMinimap::W;
	Lua_CMinimap["H"] = &CMinimap::H;
	Lua_CMinimap["WithTerrain"] = &CMinimap::WithTerrain;
	Lua_CMinimap["ShowSelected"] = &CMinimap::ShowSelected;
	Lua_CMinimap["Transparent"] = &CMinimap::Transparent;
}
//@}
