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
**  CTileset::Table
**
**      Table to map the abstract level (PUD) tile numbers, to tile
**      numbers in the graphic file (CTileset::File).
**      FE. 16 (solid light water) in pud to 328 in png.
**
**  CTileset::Flags
**
**      Table of the tile flags used by the editor.
**      @see CMapField::Flags
**
**  CTileset::solidTerrainTypes
**
**      Index to name of the basic tile type. FE. "light-water".
**      If the index is 0, the tile is not used.
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
**      tileset configuration. And it is created for the map
**      and not for the tileset.
**
**      @note I'm not sure if this table is needed in the future.
**
**      @see TileType.
**
**
**  CTileset::topOneTreeTile
**
**      The tile number of tile only containing the top part of a tree.
**      Is created on the map by lumber chopping.
**
**  CTileset::midOneTreeTile
**
**      The tile number of tile only containing the connection of
**      the top part to the bottom part of tree.
**      Is created on the map by lumber chopping.
**
**  CTileset::botOneTreeTile
**
**      The tile number of tile only containing the bottom part of a
**      tree. Is created on the map by lumber chopping.
**
**  CTileset::removedTreeTile
**
**      The tile number of the tile placed where trees are removed.
**      Is created on the map by lumber chopping.
**
**  CTilset::woodTable[20]
**
**      Table for wood removable. This table contains the tile which
**      is placed after a tree removement, depending on the surrounding.
**
**  CTileset::mixedLookupTable[]
**      Table for finding what part of the tile contains wood/rock,
**      and which part is grass or bare ground.
**
**  CTileset::topOneRockTile
**
**      The tile number of tile only containing the top part of a rock.
**      Is created on the map by destroying rocks.
**
**  CTileset::midOneRockTile
**
**      The tile number of tile only containing the connection of
**      the top part to the bottom part of a rock.
**      Is created on the map by destroying rocks.
**
**  CTileset::botOneRockTile
**
**      The tile number of tile only containing the bottom part of a
**      rock. Is created on the map by destroying rocks.
**
**  CTileset::removedRockTile
**
**      The tile number of the tile placed where rocks are removed.
**      Is created on the map by destroying rocks.
**
**  CTileset::rockTable[20]
**
**      Table for rock removable. Depending on the surrinding this
**      table contains the new tile to be placed.
**
**      @todo Johns: I don't think this table or routines look correct.
**      But they work correct.
**
**  CTileset::humanWallTable
**
**      Table of human wall tiles, index depends on the surroundings.
**
**  CTileset::orcWallTable
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

#include <limits.h>

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

void CTileset::clear()
{
	Name.clear();
	ImageFile.clear();
	pixelTileSize.x = pixelTileSize.y = 0;
	tiles.clear();
	TileTypeTable.clear();
	solidTerrainTypes.clear();
	topOneTreeTile = 0;
	midOneTreeTile = 0;
	botOneTreeTile = 0;
	removedTreeTile = 0;
	memset(woodTable, 0, sizeof(woodTable));
	mixedLookupTable.clear();
	topOneRockTile = 0;
	midOneRockTile = 0;
	botOneRockTile = 0;
	removedRockTile = 0;
	memset(rockTable, 0, sizeof(rockTable));
	memset(humanWallTable, 0, sizeof(humanWallTable));
	memset(orcWallTable, 0, sizeof(orcWallTable));
}

unsigned int CTileset::getDefaultTileIndex() const
{
	// TODO: remove hardcoded value.
	return 0x50;
}

bool CTileset::isAWallTile(unsigned tile) const
{
	if (TileTypeTable.empty() == false) {
		return (TileTypeTable[tile] == TileTypeHumanWall
				|| TileTypeTable[tile] == TileTypeOrcWall);
	}
	return false;
}

bool CTileset::isARaceWallTile(unsigned tile, bool human) const
{
	if (TileTypeTable.empty() == false) {
		if (human) {
			return TileTypeTable[tile] == TileTypeHumanWall;
		} else {
			return TileTypeTable[tile] == TileTypeOrcWall;
		}
	}
	return false;
}


bool CTileset::isAWoodTile(unsigned tile) const
{
	if (TileTypeTable.empty() == false) {
		return TileTypeTable[tile] == TileTypeWood;
	}
	return false;
}
bool CTileset::isARockTile(unsigned tile) const
{
	if (TileTypeTable.empty() == false) {
		return TileTypeTable[tile] == TileTypeRock;
	}
	return false;
}

unsigned int CTileset::getOrAddSolidTileIndexByName(const std::string &name)
{
	for (size_t i = 0; i != solidTerrainTypes.size(); ++i) {
		if (solidTerrainTypes[i].TerrainName == name) {
			return i;
		}
	}
	// Can't find it, then we add another solid terrain type.
	SolidTerrainInfo s;
	s.TerrainName = name;
	solidTerrainTypes.push_back(s);
	return solidTerrainTypes.size() - 1;
}

const std::string &CTileset::getTerrainName(int solidTerrainIndex) const
{
	return solidTerrainTypes[solidTerrainIndex].TerrainName;
}

unsigned int CTileset::getSolidTerrainCount() const
{
	return solidTerrainTypes.size();
}

int CTileset::findTileIndex(unsigned char baseTerrain, unsigned char mixTerrain) const
{
	const CTileInfo tileInfo(baseTerrain, mixTerrain);

	for (size_t i = 0; i != tiles.size();) {
		if (tiles[i].tileinfo == tileInfo) {
			return i;
		}
		// Advance solid or mixed.
		if (!tiles[i].tileinfo.MixTerrain) {
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

/**
**  Find a tile path.
**
**  @param base    Start tile type.
**  @param goal    Goal tile type.
**  @param length  Best found path length.
**  @param marks   Already visited tile types.
**  @param tileIndex    Tile pointer.
*/
int CTileset::findTilePath(int base, int goal, int length, std::vector<char> &marks, int *tileIndex) const
{
	int tileres = findTileIndex(base, goal);
	if (tileres == -1) {
		tileres = findTileIndex(goal, base);
	}
	if (tileres != -1) {
		*tileIndex = tileres;
		return length;
	}
	// Find any mixed tile
	int l = INT_MAX;
	for (size_t i = 0; i != tiles.size();) {
		int j = 0;
		if (base == tiles[i].tileinfo.BaseTerrain) {
			j = tiles[i].tileinfo.MixTerrain;
		} else if (base == tiles[i].tileinfo.MixTerrain) {
			j = tiles[i].tileinfo.BaseTerrain;
		}
		if (j != 0 && marks[j] == 0) { // possible path found
			marks[j] = j;
			int dummytileIndex;
			const int n = findTilePath(j, goal, length + 1, marks, &dummytileIndex);
			marks[j] = 0;
			if (n < l) {
				*tileIndex = i;
				l = n;
			}
		}
		// Advance solid or mixed.
		if (tiles[i].tileinfo.MixTerrain == 0) {
			i += 16;
		} else {
			i += 256;
		}
	}
	return l;
}

/**
**  Get tile from quad.
**
**  @param fixed  Part can't be changed.
**  @param quad   Quad of the tile type.
**  @return       Best matching tile.
*/
int CTileset::tileFromQuad(unsigned fixed, unsigned quad) const
{
	unsigned type1;
	unsigned type2;

	// Get tile type from fixed.
	while (!(type1 = (fixed & 0xFF))) {
		fixed >>= 8;
		if (!fixed) {
			ExitFatal(-1);
		}
	}
	fixed >>= 8;
	while (!(type2 = (fixed & 0xFF)) && fixed) {
		fixed >>= 8;
	}
	// Need an second type.
	if (!type2 || type2 == type1) {
		fixed = quad;
		while ((type2 = (fixed & 0xFF)) == type1 && fixed) {
			fixed >>= 8;
		}
		if (type1 == type2) { // Oooh a solid tile.
			const int res = findTileIndex(type1);
			Assert(res != -1);
			return res;
		}
	} else {
		std::vector<char> marks;
		int dummytileIndex;

		marks.resize(getSolidTerrainCount(), 0);

		marks[type1] = type1;
		marks[type2] = type2;

		// What fixed tile-type should replace the non useable tile-types.
		for (int i = 0; i != 4; ++i) {
			unsigned int type3 = (quad >> (8 * i)) & 0xFF;
			if (type3 != type1 && type3 != type2) {
				quad &= ~(0xFF << (8 * i));
				if (findTilePath(type1, type3, 0, marks, &dummytileIndex) < findTilePath(type2, fixed, 0, marks, &dummytileIndex)) {
					quad |= type1 << (8 * i);
				} else {
					quad |= type2 << (8 * i);
				}
			}
		}
	}

	// Need a mixed tile
	int tileIndex = getTileIndex(type1, type2, quad);
	if (tileIndex != -1) {
		return tileIndex;
	}
	// Find the best tile path.
	std::vector<char> marks;
	marks.resize(getSolidTerrainCount(), 0);
	marks[type1] = type1;
	if (findTilePath(type1, type2, 0, marks, &tileIndex) == INT_MAX) {
		DebugPrint("Huch, no mix found!!!!!!!!!!!\n");
		const int res = findTileIndex(type1);
		Assert(res != -1);
		return res;
	}
	if (type1 == tiles[tileIndex].tileinfo.MixTerrain) {
		// Other mixed
		std::swap(type1, type2);
	}
	int base = tileIndex;
	int direction = 0;
	for (int i = 0; i != 4; ++i) {
		if (((quad >> (8 * i)) & 0xFF) == type1) {
			direction |= 1 << i;
		}
	}
	//                       0  1  2  3   4  5  6  7   8  9  A   B  C   D  E  F
	const char table[16] = { 0, 7, 3, 11, 1, 9, 5, 13, 0, 8, 4, 12, 2, 10, 6, 0 };
	return base | (table[direction] << 4);
}

int CTileset::getTileBySurrounding(unsigned short type,
								   int ttup, int ttright,
								   int ttdown, int ttleft) const
{
	ttup = ttup == -1 ? 15 : mixedLookupTable[ttup];
	ttright = ttright == -1 ? 15 : mixedLookupTable[ttright];
	ttdown = ttdown == -1 ? 15 : mixedLookupTable[ttdown];
	ttleft = ttleft == -1 ? 15 : mixedLookupTable[ttleft];

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
#ifdef _MSC_VER
	const int *lookuptable = (type == MapFieldForest) ? woodTable : rockTable;
#else
	const int (&lookuptable)[20] = (type == MapFieldForest) ? woodTable : rockTable;
#endif
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


bool CTileset::isEquivalentTile(unsigned int tile1, unsigned int tile2) const
{
	//Assert(type == MapFieldForest || type == MapFieldRocks);

	return mixedLookupTable[tile1] == mixedLookupTable[tile2];
}

int CTileset::findTileIndexByTile(unsigned int tile) const
{
	for (size_t i = 0; i != tiles.size(); ++i) {
		if (tile == tiles[i].tile) {
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
**  @return        Tile index number.
**
**  @todo  FIXME: Solid tiles are here still hardcoded.
*/
unsigned int CTileset::getTileNumber(int basic, bool random, bool filler) const
{
	int tile = basic;
	if (random) {
		int n = 0;
		for (int i = 0; i < 16; ++i) {
			if (!tiles[tile + i].tile) {
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
			while (++i < 16 && !tiles[tile + i].tile) {
			}
		} while (i < 16 && n--);
		Assert(i != 16);
		return tile + i;
	}
	if (filler) {
		int i = 0;
		for (; i < 16 && tiles[tile + i].tile; ++i) {
		}
		for (; i < 16 && !tiles[tile + i].tile; ++i) {
		}
		if (i != 16) {
			return tile + i;
		}
	}
	return tile;
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

	const unsigned base = tiles[tileIndex].tileinfo.BaseTerrain;
	const unsigned mix = tiles[tileIndex].tileinfo.MixTerrain;

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

void CTileset::fillSolidTiles(std::vector<unsigned int> *solidTiles) const
{
	for (size_t i = 16; i < tiles.size(); i += 16) {
		const CTileInfo &info = tiles[i].tileinfo;

		if (info.BaseTerrain && info.MixTerrain == 0) {
			solidTiles->push_back(tiles[i].tile);
		}
	}
}

unsigned CTileset::getHumanWallTileIndex(int dirFlag) const
{
	return humanWallTable[dirFlag];
}
unsigned CTileset::getOrcWallTileIndex(int dirFlag) const
{
	return orcWallTable[dirFlag];
}

static unsigned int NextSection(const CTileset &tileset, unsigned int tileIndex)
{
	while (tileset.tiles[tileIndex].tile) { // Skip good tiles
		++tileIndex;
	}
	while (!tileset.tiles[tileIndex].tile) { // Skip separator
		++tileIndex;
	}
	return tileIndex;
}

unsigned CTileset::getHumanWallTileIndex_broken(int dirFlag) const
{
	unsigned tileIndex = humanWallTable[dirFlag];
	tileIndex = NextSection(*this, tileIndex);
	return tileIndex;
}
unsigned CTileset::getOrcWallTileIndex_broken(int dirFlag) const
{
	unsigned tileIndex = orcWallTable[dirFlag];
	tileIndex = NextSection(*this, tileIndex);
	return tileIndex;
}
unsigned CTileset::getHumanWallTileIndex_destroyed(int dirFlag) const
{
	unsigned tileIndex = humanWallTable[dirFlag];
	tileIndex = NextSection(*this, tileIndex);
	tileIndex = NextSection(*this, tileIndex);
	return tileIndex;
}
unsigned CTileset::getOrcWallTileIndex_destroyed(int dirFlag) const
{
	unsigned tileIndex = orcWallTable[dirFlag];
	tileIndex = NextSection(*this, tileIndex);
	tileIndex = NextSection(*this, tileIndex);
	return tileIndex;
}
//@}
