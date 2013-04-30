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
/**@name color.h - The A platform independent color. */
//
//      (c) Copyright 2012 by Joris Dauphin
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

#include "stratagus.h"

#include "color.h"

#include "script.h"

#include "SDL.h"

CColor::operator SDL_Color() const
{
	SDL_Color c = { R, G, B, A };
	return c;
}

void CColor::Parse(lua_State *l, const int offset)
{
	if (!lua_istable(l, offset) || lua_rawlen(l, offset) != 3) {
		LuaError(l, "incorrect argument");
	}
	const int r = LuaToNumber(l, offset, 1);
	const int g = LuaToNumber(l, offset, 2);
	const int b = LuaToNumber(l, offset, 3);

	if (!(0 <= r && r <= 255
		  && 0 <= g && g <= 255
		  && 0 <= b && b <= 255)) {
		LuaError(l, "Arguments must be in the range 0-255");
	}
	this->R = r;
	this->G = g;
	this->B = b;
	this->A = 0;
}

//@}
