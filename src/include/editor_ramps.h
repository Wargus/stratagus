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
/**@name editor-ramps.h - Assistant for drawing ramps in the editor. */
//
//      (c) Copyright 2023-2025 by Alyokhin
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

#pragma once

//@{

/*----------------------------------------------------------------------------
--  Declarations
----------------------------------------------------------------------------*/
#include "tileset.h"


/*----------------------------------------------------------------------------
--  Declarations
----------------------------------------------------------------------------*/
enum class RampDirections
{
	North,
	NorthEast,
	SouthEast,
	South,
	SouthWest,
	NorthWest
};
enum class RampSections
{
	Complete,
	SideOne,
	Middle,
	SideTwo
};

class CRampSection
{
public:
	tile_index getTile(RampDirections direction, uint16_t col, uint16_t row, lGround_type, hGround_type) const;
	uint16_t getWidth() const { return width;  }
	uint16_t getHeight() const { return height;  }

private:
	RampDirections direction {RampDirections::North};
	RampSections sectionType {RampSections::Complete};
	uint16_t width = 0;
	uint16_t height = 0;

	std::vector<tile_index> tiles;
};

class CRamps
{
public:
private:
	
};
