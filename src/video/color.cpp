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

extern SDL_Surface *TheScreen;

CColor::operator IntColor() const
{
	return SDL_MapRGB(TheScreen->format, R, G, B);
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
	this->A = 255;
}

void CUnitColors::Clear()
{
	Colors.clear();
}

void CUnitColors::Set(std::vector<CColor> &colors)
{
	Colors = colors;
}

PaletteSwap::PaletteSwap(unsigned int variable, unsigned char colorStart, unsigned char colorCount, unsigned char steps, unsigned char alternatives, std::vector<CColor> &colors)
{
	this->UnitVariableIndex = variable;
	this->ColorIndexStart = colorStart;
	this->ColorCount = colorCount;
	this->Steps = steps;
	this->AlternativesCount = alternatives;
	this->Colors = new SDL_Color[colorCount * steps * alternatives];
	Assert(colors.size() == colorCount * steps * alternatives);
	for (unsigned int i = 0; i < colors.size(); i++) {
		this->Colors[i] = colors[i];
	}
}

PaletteSwap::~PaletteSwap()
{
	if (Colors) {
		// delete[] Colors;
	}
}

SDL_Color *PaletteSwap::GetColorsForPercentAndAlternative(unsigned int value, unsigned int max, unsigned int alt)
{
	if (max == 0) {
		return Colors;
	}
	unsigned int step = (this->Steps - 1) * value / max;
	unsigned char alternative = alt % AlternativesCount;

	return &Colors[step * ColorCount * AlternativesCount + alternative * ColorCount];
}

IntColor InterpolateColor(IntColor color1, IntColor color2, float fraction)
{
        unsigned char r1 = (color1 >> 16) & 0xff;
        unsigned char r2 = (color2 >> 16) & 0xff;
        unsigned char g1 = (color1 >> 8) & 0xff;
        unsigned char g2 = (color2 >> 8) & 0xff;
        unsigned char b1 = color1 & 0xff;
        unsigned char b2 = color2 & 0xff;

        return (int) ((r2 - r1) * fraction + r1) << 16 |
                (int) ((g2 - g1) * fraction + g1) << 8 |
                (int) ((b2 - b1) * fraction + b1);
}

//@}
