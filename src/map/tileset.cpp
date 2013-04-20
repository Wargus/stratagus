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

const std::string &CTileset::getTerrainName(int solidTerrainIndex) const
{
	return SolidTerrainTypes[solidTerrainIndex].TerrainName;
}

unsigned int CTileset::getSolidTerrainCount() const
{
	return SolidTerrainTypes.size();
}

int CTileset::findTileIndex(unsigned char baseTerrain, unsigned char mixTerrain) const
{
	const TileInfo tileInfo = {baseTerrain, mixTerrain};

	for (int i = 0; i != NumTiles;) {
		if (Tiles[i] == tileInfo) {
			return i;
		}
		// Advance solid or mixed.
		if (!Tiles[i].MixTerrain) {
			i += 16;
		} else {
			i += 256;
		}
	}
	return -1;
}

int CTileset::getTileIndex(unsigned char baseTerrain, unsigned char mixTerrain, unsigned int quad) const
{
	int tileIndex = findTileIndex(baseTerrain, mixTerrain);
	if (tileIndex == -1) {
		tileIndex = findTileIndex(mixTerrain, baseTerrain);
		if (tileIndex == -1) {
			return -1;
		}
		std::swap(baseTerrain, mixTerrain);
	}
	int base = tileIndex;

	int direction = 0;
	for (int i = 0; i != 4; ++i) {
		if (((quad >> (8 * i)) & 0xFF) == baseTerrain) {
			direction |= 1 << i;
		}
	}
	//                       0  1  2  3   4  5  6  7   8  9  A   B  C   D  E  F
	const char table[16] = { 0, 7, 3, 11, 1, 9, 5, 13, 0, 8, 4, 12, 2, 10, 6, 0 };
	return base | (table[direction] << 4);
}

int CTileset::getTileIndexBySurrounding(unsigned short type,
										unsigned int ttup, unsigned int ttright,
										unsigned int ttdown, unsigned int ttleft) const
{
	//  Check each of the corners to ensure it has both connecting
	//  ?**?
	//  *mm*
	//  *mm*
	//  ?**?
	//
	//   *  type asterixs must match for wood to be present

	int tile = 0;
	tile += ((ttup & 0x01) && (ttleft & 0x04)) * 8;
	tile += ((ttup & 0x02) && (ttright & 0x08)) * 4;
	tile += ((ttright & 0x01) && (ttdown & 0x04)) * 2;
	tile += ((ttleft & 0x02) && (ttdown & 0x08)) * 1;

	//Test if we have top tree, or bottom tree, they are special
	if ((ttdown & 0x10) != 0) {
		tile |= (ttleft & 0x06) != 0 ? 1 : 0;
		tile |= (ttright & 0x09) != 0 ? 2 : 0;
	}

	if ((ttup & 0x20) != 0) {
		tile |= (ttleft & 0x06) != 0 ? 8 : 0;
		tile |= (ttright & 0x09) != 0 ? 4 : 0;
	}

	Assert(type == MapFieldForest || type == MapFieldRocks);
	const int (&lookuptable)[20] = (type == MapFieldForest) ? WoodTable : RockTable;
	tile = lookuptable[tile];

	//If tile is -1, then we should check if we are to draw just one tree
	//Check for tile about, or below or both...
	if (tile == -1) {
		tile = 16;
		tile += ((ttup & 0x01) || (ttup & 0x02)) * 1;
		tile += ((ttdown & 0x04) || (ttdown & 0x08)) * 2;
		tile = lookuptable[tile];
	}
	return tile;
}

int CTileset::findTileIndexByTile(unsigned int tile) const
{
	for (int i = 0; i != NumTiles; ++i) {
		if (tile == Table[i]) {
			return i;
		}
	}
	return -1;
}

/**
**  Get tile number.
**
**  @param basic   Basic tile number
**  @param random  Return random tile
**  @param filler  Get a decorated tile.
**
**  @return        Tile number.
**
**  @todo  FIXME: Solid tiles are here still hardcoded.
*/
unsigned int CTileset::getTileNumber(int basic, bool random, bool filler) const
{
	int tile = basic;
	if (random) {
		int n = 0;
		for (int i = 0; i < 16; ++i) {
			if (!Table[tile + i]) {
				if (!filler) {
					break;
				}
			} else {
				++n;
			}
		}
		n = MyRand() % n;
		int i = -1;
		do {
			while (++i < 16 && !Table[tile + i]) {
			}
		} while (i < 16 && n--);
		Assert(i != 16);
		return Table[tile + i];
	}
	if (filler) {
		int i = 0;
		for (; i < 16 && Table[tile + i]; ++i) {
		}
		for (; i < 16 && !Table[tile + i]; ++i) {
		}
		if (i != 16) {
			return Table[tile + i];
		}
	}
	return Table[tile];
}

/**
**  Get quad from tile.
**
**  A quad is a 32 bit value defining the content of the tile.
**
**  A tile is split into 4 parts, the basic tile type of this part
**    is stored as 8bit value in the quad.
**
**  ab
**  cd -> abcd
**
**  If the tile is 100% light grass(0x05) the value is 0x05050505.
**  If the tile is 3/4 light grass and dark grass(0x06) in upper left corner
**    the value is 0x06050505.
*/
unsigned CTileset::getQuadFromTile(unsigned int tile) const
{
	const int tileIndex = findTileIndexByTile(tile);
	Assert(tileIndex != -1);

	unsigned base = Tiles[tileIndex].BaseTerrain;
	unsigned mix = Tiles[tileIndex].MixTerrain;

	if (mix == 0) { // a solid tile
		return base | (base << 8) | (base << 16) | (base << 24);
	}
	// Mixed tiles, mix together
	switch ((tileIndex & 0x00F0) >> 4) {
		case 0: return (base << 24) | (mix << 16) | (mix << 8) | mix;
		case 1: return (mix << 24) | (base << 16) | (mix << 8) | mix;
		case 2: return (base << 24) | (base << 16) | (mix << 8) | mix;
		case 3: return (mix << 24) | (mix << 16) | (base << 8) | mix;
		case 4: return (base << 24) | (mix << 16) | (base << 8) | mix;
		case 5: return (mix << 24) | (base << 16) | (base << 8) | mix;
		case 6: return (base << 24) | (base << 16) | (base << 8) | mix;
		case 7: return (mix << 24) | (mix << 16) | (mix << 8) | base;
		case 8: return (base << 24) | (mix << 16) | (mix << 8) | base;
		case 9: return (mix << 24) | (base << 16) | (mix << 8) | base;
		case 10: return (base << 24) | (base << 16) | (mix << 8) | base;
		case 11: return (mix << 24) | (mix << 16) | (base << 8) | base;
		case 12: return (base << 24) | (mix << 16) | (base << 8) | base;
		case 13: return (mix << 24) | (base << 16) | (base << 8) | base;
	}
	Assert(0);
	return base | (base << 8) | (base << 16) | (base << 24);
}

void CTileset::fillSolidTiles(std::vector<unsigned int> *tiles) const
{
	for (int i = 16; i < Map.Tileset->NumTiles; i += 16) {
		const TileInfo &info = Map.Tileset->Tiles[i];

		if (info.BaseTerrain && info.MixTerrain == 0) {
			tiles->push_back(Map.Tileset->Table[i]);
		}
	}
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
