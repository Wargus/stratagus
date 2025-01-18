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
#include "widgets.h"

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

	using brushApplyFn = std::function<void(const TilePos&, tile_index, bool, bool)>; // type alias

	struct Properties;

public:
	explicit CBrush(std::string name,
					CBrush::Properties properties)
	{
		this->name = std::move(name);
		this->properties = std::move(properties);
		rndEnabled = this->properties.randomizeAllowed;
		fixNeighborsEnabled = this->properties.fixNeighborsAllowed;
		setSize(this->properties.minSize.width, this->properties.minSize.height);
	}
	explicit CBrush(std::string name,
					CBrush::Properties properties,
					const std::vector<tile_index> &tilesSrc)
	{
		this->name = std::move(name);
		this->properties = std::move(properties);
		rndEnabled = this->properties.randomizeAllowed;
		fixNeighborsEnabled = this->properties.fixNeighborsAllowed;
		setSize(this->properties.minSize.width, this->properties.minSize.height);
		fillWith(tilesSrc);
	}

	~CBrush() = default;

	void applyAt(const TilePos &pos, brushApplyFn applyFn, bool forbidRandomization = false) const;
	
	uint8_t getWidth() const { return width; }
	uint8_t getHeight() const { return height; }

	BrushTypes getType() const { return properties.type; }

	graphic_index getGraphicTile(uint8_t col, uint8_t row) const;

	tile_index getTile(uint8_t col, uint8_t row) const;

	void setTile(tile_index tile, uint8_t col = 0, uint8_t row = 0);
	void fillWith(tile_index tile, bool init = false);
	void fillWith(const std::vector<tile_index> &tilesSrc);
	void randomize();

	void setAllign(BrushAllign allignTo) { properties.allign = allignTo; }
	TilePos getAllignOffset() const;
	BrushAllign getAllign() const { return properties.allign; }

	bool isCentered() const { return properties.allign == BrushAllign::Center; }

	void resizeW(uint8_t newWidth);
	void resizeH(uint8_t newHeight);
	void resize(uint8_t newWidth, uint8_t newHeight);

	uint8_t getWidth() { return width; }
	uint8_t getHeight() { return height; }

	Vec2i getResizeSteps() const { return { properties.resizeSteps.width, properties.resizeSteps.height}; }
	Vec2i getMaxSize() const { return { properties.maxSize.width, properties.maxSize.height}; }
	Vec2i getMinSize() const { return { properties.minSize.width, properties.minSize.height}; }

	bool isSymmetric() const { return properties.symmetric; }
	bool isResizable() const { return properties.resizable; }
	bool isRandomizeAllowed() const { return properties.randomizeAllowed; }
	bool isNeighborsFixAllowed() const { return properties.fixNeighborsAllowed; }

	void enableRandomization(bool enable = true)
	{
		rndEnabled = properties.randomizeAllowed ? enable : false;
	}
	bool isRandomizationEnabled() const { return rndEnabled; }
	void enableFixNeighbors(bool enable = true)
	{ 
		fixNeighborsEnabled = properties.fixNeighborsAllowed ? enable : false;
	}
	bool isFixNeighborsEnabled() const { return fixNeighborsEnabled; }
	bool isDecorative() const { return decorative; }
	void setDecorative(bool value) {
		decorative = properties.fixNeighborsAllowed ? value : true;
	}

	std::string getName() const { return name; }
	void setName(const std::string &name) { this->name = name; }

protected:
	void setSize(uint8_t newWidth, uint8_t newHeight);
	bool withinBounds(uint8_t col, uint8_t row) const { return col < width && row < height; }

	void drawCircle(int16_t xCenter,
	                int16_t yCenter,
	                int16_t diameter,
	                tile_index tile,
	                std::vector<tile_index> &canvas);

	tile_index randomizeTile(tile_index tileIdx) const;
	tile_index getCurrentTile() const;
	
public:
	struct Properties { // with default settings
		BrushTypes type = BrushTypes::SingleTile;
		BrushShapes shape = BrushShapes::Rectangular;
		BrushAllign allign = BrushAllign::UpperLeft;
		bool symmetric = false;
		bool resizable = true;
		struct
		{
			uint8_t width;
			uint8_t height;
		} resizeSteps{1, 1}, minSize{1, 1}, maxSize{20, 20};

		bool randomizeAllowed = true;
		bool fixNeighborsAllowed = true;
	};

protected:
	Properties properties;

	std::string name;
	
	bool rndEnabled = false; /// Edit mode: place an the selected tile or a random tile of the same type
	bool fixNeighborsEnabled = false; /// Edit mode: enabled fix up for neighbors with tile to be placed
	bool decorative = false;

	bool isInit = false;

	uint8_t width = 1;
	uint8_t height = 1;
	std::vector<tile_index> tiles;
};

class CBrushesSet
{
public:
	CBrushesSet()
	{
		loadBrushes();
	}

	CBrushesSet(std::string_view brushesSrc)
	{ 
		loadBrushes(brushesSrc);
	}
	~CBrushesSet() { brushes.clear(); }

public:
	void loadBrushes(std::string_view brushesSrc = {});
	bool isLoaded() const { return !brushes.empty(); }

	CBrush &getCurrentBrush() { return currentBrush; }
	
	bool setCurrentBrush(std::string_view name);
	void addBrush(CBrush brush) { brushes.push_back(std::move(brush)); }

	std::vector<std::string> getBrushesNames() const;

private:
	std::string brushesSrc;
	std::list<CBrush> brushes;
	CBrush currentBrush {std::string("Default"), CBrush::Properties()};
};