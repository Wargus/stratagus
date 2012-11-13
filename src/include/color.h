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
/**@name color.h - The A platform independent color headerfile. */
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

#ifndef COLOR_H
#define COLOR_H

//@{

struct SDL_Color;
struct lua_State;

/// A platform independent color
class CColor
{
public:
	CColor() : R(0), G(0), B(0), A(0) {}
	CColor(unsigned char r, unsigned char g, unsigned char b,
		   unsigned char a = 0) : R(r), G(g), B(b), A(a) {}

	void Parse(lua_State *l, int index = -1);

	/// Cast to a SDL_Color
	operator SDL_Color() const;

public:
	unsigned char R;       /// Red
	unsigned char G;       /// Green
	unsigned char B;       /// Blue
	unsigned char A;       /// Alpha
};


#include <vector>

class CUnitColors
{
public:
	CUnitColors() {}

public:
	std::vector<CColor> Colors;
};


#include <stdint.h>

typedef uint32_t IntColor; // Uint32 in SDL


//@}

#endif
