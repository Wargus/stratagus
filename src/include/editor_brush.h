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
/**@name editor_brush.h - Assistant for brushes in the editor. */
//
//      (c) Copyright 2023-2024 by Alyokhin
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
#include <cstdint>
#include "tileset.h"
#include "vec2i.h"

/*----------------------------------------------------------------------------
--  Declarations
----------------------------------------------------------------------------*/

class CBrush
{
public:
	enum class BrushTypes
	{
		SingleTile,
		MultiTile,
		Ramp
	};

	enum class BrushShapes
	{
		Round,
		Rectangular
	};

	enum class BrushAllign
	{
		UpperLeft,
		Center
	};

	using brushApplyFn = std::function<void(const TilePos&, tile_index)>; // type alias

public:
	explicit CBrush(BrushTypes type,
					uint8_t w,
					uint8_t h,
					BrushShapes shape = BrushShapes::Rectangular,
					BrushAllign allignTo = BrushAllign::UpperLeft)
	{
		this->type = type;
		this->shape = shape;
		setAllign(allignTo);
		setSize(w, h);
	}
	explicit CBrush(BrushTypes type,
					uint8_t w,
					uint8_t h,
					tile_index tile,
					BrushShapes shape = BrushShapes::Rectangular,
					BrushAllign allignTo = BrushAllign::UpperLeft)
	{
		this->type = type;
		this->shape = shape;
		setAllign(allignTo);
		setSize(w, h);
		fillWith(tile, true);
	}
	explicit CBrush(BrushTypes type,
					uint8_t w,
					uint8_t h,
					const std::vector<tile_index> &tilesSrc,
					BrushShapes shape = BrushShapes::Rectangular,
					BrushAllign allignTo = BrushAllign::UpperLeft)
	{
		this->type = type;
		this->shape = shape;
		setAllign(allignTo);
		setSize(w, h);
		fillWith(tilesSrc);
	}

	~CBrush() = default;

	void applyBrushAt(const TilePos &pos, brushApplyFn applyFn) const;

	uint8_t getWidth() const { return width; }
	uint8_t getHeight() const { return height; }

	BrushTypes getType() const { return type; }

	graphic_index getGraphicTile(uint8_t col, uint8_t row) const;

	tile_index getTile(uint8_t col, uint8_t row) const;

	void setTile(tile_index tile, uint8_t col = 0, uint8_t row = 0);
	void fillWith(tile_index tile, bool init = false);
	void fillWith(const std::vector<tile_index> &tilesSrc);

	void setAllign(BrushAllign allignTo) { allign = allignTo; }
	TilePos getAllignOffset() const;
	BrushAllign getAllign() const { return allign; }

	bool isCentered() const { return allign == BrushAllign::Center; }

	void setSize(uint8_t newWidth, uint8_t newHeight);


protected:
	bool withinBounds(uint8_t col, uint8_t row) const { return col < width && row < height; }
	void drawCircle(int16_t xCenter,
	                int16_t yCenter,
	                int16_t diameter,
	                tile_index tile,
	                std::vector<tile_index> &canvas);

protected:
	BrushTypes type = BrushTypes::SingleTile;
	BrushShapes shape = BrushShapes::Rectangular;
	BrushAllign allign = BrushAllign::UpperLeft;

	bool isInit = false;

	uint8_t width = 0;
	uint8_t height = 0;
	std::vector<tile_index> tiles;
};
