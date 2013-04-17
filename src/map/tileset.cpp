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
/**@name tileset.cpp - The tileset. */
//
//      (c) Copyright 1998-2005 by Lutz Sammer and Jimmy Salmon
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
--  Documentation
----------------------------------------------------------------------------*/

/**
**  @class CTileset tileset.h
**
**  \#include "tileset.h"
**
**  This structure contains information about the tileset of the map.
**  It defines the look and properties of the tiles. Currently only one
**  tileset per map is supported. In the future it is planned to support
**  multiple tilesets on the same map. Also it is planned to support animated
**  tiles.
**
**  The tileset structure members:
**
**  CTileset::Name
**
**      Long name of the tileset. Can be used by the level editor.
**
**  CTileset::ImageFile
**
**      Name of the graphic file, containing all tiles.
**
**  CTileset::NumTiles
**
**      The number of different tiles in the tables.
**
**  CTileset::Table
**
**      Table to map the abstract level (PUD) tile numbers, to tile
**      numbers in the graphic file (CTileset::File).
**      FE. 16 (solid light water) in pud to 328 in png.
**
**  CTileset::FlagsTable
**
**      Table of the tile flags used by the editor.
**      @see CMapField::Flags
**
**  CTileset::BasicNameTable
**
**      Index to name of the basic tile type. FE. "light-water".
**      If the index is 0, the tile is not used.
**      @see CTileset::TileNames
**
**  CTileset::MixedNameTable
**
**      Index to name of the mixed tile type. FE. "light-water".
**      If this index is 0, the tile is a solid tile.
**      @see CTileset::TileNames
**
**  CTileset::TileTypeTable
**
**      Lookup table of the tile type. Maps the graphic file tile
**      number back to a tile type (::TileTypeWood, ::TileTypeWater,
**      ...)
**
**      @note The creation of this table is currently hardcoded in
**      the engine. It should be calculated from the flags in the
**      tileset configuration (CCL). And it is created for the map
**      and not for the tileset.
**
**      @note I'm not sure if this table is needed in the future.
**
**      @see TileType.
**
**  CTileset::NumNames
**
**      Number of different tile names.
**
**  CTileset::TileNames
**
**      The different tile names. FE "light-grass", "dark-water".
**
**  CTileset::TopOneTree
**
**      The tile number of tile only containing the top part of a tree.
**      Is created on the map by lumber chopping.
**
**  CTileset::MidOneTree
**
**      The tile number of tile only containing the connection of
**      the top part to the bottom part of tree.
**      Is created on the map by lumber chopping.
**
**  CTileset::BotOneTree
**
**      The tile number of tile only containing the bottom part of a
**      tree. Is created on the map by lumber chopping.
**
**  CTileset::RemovedTree
**
**      The tile number of the tile placed where trees are removed.
**      Is created on the map by lumber chopping.
**
**  CTileset::GrowingTree[2]
**
**      Contains the tile numbers of a growing tree from small to big.
**      @note Not yet used.
**
**  CTilset::WoodTable[20]
**
**      Table for wood removable. This table contains the tile which
**      is placed after a tree removement, depending on the surrounding.
**
**  CTileset::MixedLookupTable[]
**      Table for finding what part of the tile contains wood/rock,
**      and which part is grass or bare ground.
**
**  CTileset::TopOneRock
**
**      The tile number of tile only containing the top part of a rock.
**      Is created on the map by destroying rocks.
**
**  CTileset::MidOneRock
**
**      The tile number of tile only containing the connection of
**      the top part to the bottom part of a rock.
**      Is created on the map by destroying rocks.
**
**  CTileset::BotOneRock
**
**      The tile number of tile only containing the bottom part of a
**      rock. Is created on the map by destroying rocks.
**
**  CTileset::RemovedRock
**
**      The tile number of the tile placed where rocks are removed.
**      Is created on the map by destroying rocks.
**
**  CTileset::RockTable[20]
**
**      Table for rock removable. Depending on the surrinding this
**      table contains the new tile to be placed.
**
**      @todo Johns: I don't think this table or routines look correct.
**      But they work correct.
**
**  CTileset::HumanWallTable
**
**      Table of human wall tiles, index depends on the surroundings.
**
**  CTileset::OrcWallTable
**
**      Table of orc wall tiles, index depends on the surroundings.
**
**
**  @struct TileInfo tileset.h
**
**  \#include "tileset.h"
**
**  This structure includes everything about a specific tile from the tileset.
**
**  TileInfo::BaseTerrain
**
**      This is the base terrain type of a tile. Only 15 of those
**      are currently supported.
**
**  TileInfo::MixTerrain
**
**  @todo This is the terrain the tile is mixed with. This is 0 for
**    a solid tile, we should make it equal to BaseTerrain
*/

/*----------------------------------------------------------------------------
--  Includes
----------------------------------------------------------------------------*/

#include "stratagus.h"

#include "tileset.h"

#include "map.h"
#include "video.h"

/*----------------------------------------------------------------------------
-- Variables
----------------------------------------------------------------------------*/

/**
** Size of a tile in pixel
*/
PixelSize PixelTileSize(32, 32);


/*----------------------------------------------------------------------------
-- Functions
----------------------------------------------------------------------------*/

/**
** Cleanup the tileset module.
**
** @note this didn't frees the configuration memory.
*/
void CleanTilesets()
{
	Map.Tileset->Clear();

	// Should this be done by the map?
	CGraphic::Free(Map.TileGraphic);
	Map.TileGraphic = NULL;
}

void CTileset::Clear()
{
	Name.clear();
	ImageFile.clear();
	NumTiles = 0;
	PixelTileSize.x = PixelTileSize.y = 0;
	Table.clear();
	FlagsTable.clear();
	Tiles.clear();
	TileTypeTable.clear();
	SolidTerrainTypes.clear();
	TopOneTree = 0;
	MidOneTree = 0;
	BotOneTree = 0;
	RemovedTree = 0;
	memset(GrowingTree, 0, sizeof(GrowingTree));
	memset(WoodTable, 0, sizeof(WoodTable));
	MixedLookupTable.clear();
	TopOneRock = 0;
	MidOneRock = 0;
	BotOneRock = 0;
	RemovedRock = 0;
	memset(RockTable, 0, sizeof(RockTable));
	memset(HumanWallTable, 0, sizeof(HumanWallTable));
	memset(OrcWallTable, 0, sizeof(OrcWallTable));
}

bool CTileset::IsSeenTile(unsigned short type, unsigned short seen) const
{
	if (TileTypeTable.empty() == false) {
		switch (type) {
			case MapFieldForest:
				return TileTypeTable[seen] == TileTypeWood;
			case MapFieldRocks:
				return TileTypeTable[seen] == TileTypeRock;
			default:
				return false;
		}
	}
	return false;
}

unsigned int CTileset::getOrAddSolidTileIndexByName(const std::string &name)
{
	for (size_t i = 0; i != SolidTerrainTypes.size(); ++i) {
		if (SolidTerrainTypes[i].TerrainName == name) {
			return i;
		}
	}
	// Can't find it, then we add another solid terrain type.
	SolidTerrainInfo s;
	s.TerrainName = name;
	SolidTerrainTypes.push_back(s);
	return SolidTerrainTypes.size() - 1;
}

unsigned CTileset::getHumanWallTile(int index) const
{
	unsigned tile = HumanWallTable[index];
	tile = Table[tile];
	return tile;
}
unsigned CTileset::getOrcWallTile(int index) const
{
	unsigned tile = OrcWallTable[index];
	tile = Table[tile];
	return tile;
}


static unsigned int NextSection(const CTileset &tileset, unsigned int tile)
{
	while (tileset.Table[tile]) { // Skip good tiles
		++tile;
	}
	while (!tileset.Table[tile]) { // Skip separator
		++tile;
	}
	return tile;
}

unsigned CTileset::getHumanWallTile_broken(int index) const
{
	unsigned tile = HumanWallTable[index];
	tile = NextSection(*this, tile);
	tile = Table[tile];
	return tile;
}
unsigned CTileset::getOrcWallTile_broken(int index) const
{
	unsigned tile = OrcWallTable[index];
	tile = NextSection(*this, tile);
	tile = Table[tile];
	return tile;
}
unsigned CTileset::getHumanWallTile_destroyed(int index) const
{
	unsigned tile = HumanWallTable[index];
	tile = NextSection(*this, tile);
	tile = NextSection(*this, tile);
	tile = Table[tile];
	return tile;
}
unsigned CTileset::getOrcWallTile_destroyed(int index) const
{
	unsigned tile = OrcWallTable[index];
	tile = NextSection(*this, tile);
	tile = NextSection(*this, tile);
	tile = Table[tile];
	return tile;
}

//@}
