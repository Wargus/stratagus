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

#include <stdint.h>

typedef uint32_t IntColor; // Uint32 in SDL

struct SDL_Color;
struct lua_State;

/// A platform independent color
class CColor
{
public:
	CColor() : R(0), G(0), B(0), A(0) {}
	CColor(unsigned char r, unsigned char g, unsigned char b,
		   unsigned char a = 0) : R(r), G(g), B(b), A(a) {}
	CColor(const CColor &color) : R(color.R), G(color.G), B(color.B), A(color.A) {}

	void Parse(lua_State *l, int index = -1);

	/// Cast to a SDL_Color
	operator SDL_Color() const;

	/// Cast to IntColor
	operator IntColor() const;

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

	void Clear();

	void Set(std::vector<CColor> &colors);

private:
	std::vector<CColor> Colors;
};


/**
 * @brief A class to encode unit-variable dependent palette swaps.
 * 
 * This class encodes a range of colors that should be swapped based on
 * the current percentage of a unit variable, with a resolution of a
 * number of steps and a number of alternative colorings per step.
 * 
 * The colors most be laid out first by step, then alternative, then
 * color.
 */
class PaletteSwap
{
public:
	PaletteSwap(unsigned int variable, unsigned char colorStart, unsigned char colorCount, unsigned char steps, unsigned char alternatives, std::vector<CColor> &colors);
	~PaletteSwap();

	SDL_Color *GetColorsForPercentAndAlternative(unsigned int value, unsigned int max, unsigned int alt);
	
	unsigned int GetUnitVariableIndex() { return UnitVariableIndex; }
	unsigned int GetColorIndexStart() { return ColorIndexStart; }
	unsigned int GetColorCount() { return ColorCount; }

private:
	unsigned int UnitVariableIndex;
	unsigned char ColorIndexStart;
	unsigned char ColorCount;
	unsigned char Steps;
	unsigned char AlternativesCount;
	SDL_Color *Colors = nullptr;
};

/**
 * interpolate 2 RGB colors
 * @param color1    integer containing color as 0x00RRGGBB
 * @param color2    integer containing color as 0x00RRGGBB
 * @param fraction  how much interpolation (0..1)
 * - 0: full color 1
 * - 1: full color 2
 * @return the new color after interpolation
 */
IntColor InterpolateColor(IntColor color1, IntColor color2, float fraction);

//@}

#endif
