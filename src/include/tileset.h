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
/**@name tileset.h - The tileset headerfile. */
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

#ifndef TILESET_H
#define TILESET_H

//@{

/*----------------------------------------------------------------------------
--  Declarations
----------------------------------------------------------------------------*/

#include "vec2i.h"
#include <vector>

struct lua_State;

/**
**  These are used for lookup tiles types
**  mainly used for the FOW implementation of the seen woods/rocks
**
**  @todo I think this can be removed, we can use the flags?
**  I'm not sure, if we have seen and real time to considere.
*/
enum TileType {
	TileTypeUnknown,    /// Unknown tile type
	TileTypeWood,       /// Any wood tile
	TileTypeRock,       /// Any rock tile
	TileTypeCoast,      /// Any coast tile
	TileTypeHumanWall,  /// Any human wall tile
	TileTypeOrcWall,    /// Any orc wall tile
	TileTypeWater       /// Any water tile
};

/// Single tile definition
struct TileInfo {
public:
	bool operator ==(const TileInfo &rhs) const
	{
		return BaseTerrain == rhs.BaseTerrain && MixTerrain == rhs.MixTerrain;
	}
	bool operator !=(const TileInfo &rhs) const { return !(*this == rhs); }

public:
	unsigned char BaseTerrain; /// Basic terrain of the tile
	unsigned char MixTerrain;  /// Terrain mixed with this
};

/// Definition for a terrain type
struct SolidTerrainInfo {
	std::string TerrainName;  /// Name of the terrain
	// TODO: When drawing with the editor add some kind fo probabilities for every tile.
};

/// Tileset definition
class CTileset
{
public:
	void Clear();
	bool IsSeenTile(unsigned short type, unsigned short seen) const;

	unsigned getRemovedRockTile() const { return RemovedRock; }
	unsigned getRemovedTreeTile() const { return RemovedTree; }
	unsigned getBottomOneTreeTile() const { return BotOneTree; }
	unsigned getTopOneTreeTile() const { return TopOneTree; }

	unsigned getHumanWallTile(int index) const;
	unsigned getOrcWallTile(int index) const;
	unsigned getHumanWallTile_broken(int index) const;
	unsigned getOrcWallTile_broken(int index) const;
	unsigned getHumanWallTile_destroyed(int index) const;
	unsigned getOrcWallTile_destroyed(int index) const;

public:
	unsigned int getSolidTerrainCount() const;

	const std::string &getTerrainName(int solidTerrainIndex) const;
	int findTileIndex(unsigned char baseTerrain, unsigned char mixTerrain = 0) const;
	int getTileIndex(unsigned char baseTerrain, unsigned char mixTerrain, unsigned int quad) const;

	int findTileIndexByTile(unsigned int tile) const;
	unsigned int getTileNumber(int basic, bool random, bool filler) const;
	void fillSolidTiles(std::vector<unsigned int> *tiles) const;

	unsigned getQuadFromTile(unsigned int tile) const;
	int getTileIndexBySurrounding(unsigned short type,
								  unsigned int up, unsigned int right,
								  unsigned int bottom, unsigned int left) const;
	int tileFromQuad(unsigned fixed, unsigned quad) const;

public:
	void parse(lua_State *l);
	void buildTable(lua_State *l);

private:
	unsigned int getOrAddSolidTileIndexByName(const std::string &name);
	void buildWallReplacementTable();
	void parseSlots(lua_State *l, int t);
	void parseSpecial(lua_State *l);
	void parseSolid(lua_State *l);
	void parseMixed(lua_State *l);
	int findTilePath(int base, int goal, int length, std::vector<char> &marks, int *tileIndex) const;
public:
	std::string Name;           /// Nice name to display
	std::string ImageFile;      /// File containing image data

	int NumTiles;               /// Number of tiles in the tables
	PixelSize PixelTileSize;    /// Size of a tile in pixel
	std::vector<unsigned short> Table;      /// Pud to Internal conversion table
	std::vector<unsigned short> FlagsTable; /// Flag table for editor
	std::vector<TileInfo> Tiles; /// Tile descriptions

	// TODO: currently hardcoded
	std::vector<unsigned char> TileTypeTable;  /// For fast lookup of tile type
private:
	std::vector<SolidTerrainInfo> SolidTerrainTypes; /// Information about solid terrains.
public:
	std::vector<int> MixedLookupTable;  /// Lookup for what part of tile used
private:
	unsigned TopOneTree;   /// Tile for one tree top
	unsigned MidOneTree;   /// Tile for one tree middle
	unsigned BotOneTree;   /// Tile for one tree bottom
	unsigned RemovedTree;  /// Tile placed where trees are gone
	int WoodTable[20];     /// Table for tree removable
	unsigned TopOneRock;   /// Tile for one rock top
	unsigned MidOneRock;   /// Tile for one rock middle
	unsigned BotOneRock;   /// Tile for one rock bottom
	unsigned RemovedRock;  /// Tile placed where rocks are gone
	int RockTable[20];     /// Removed rock placement table
	unsigned HumanWallTable[16];  /// Human wall placement table
	unsigned OrcWallTable[16];    /// Orc wall placement table
};

/*----------------------------------------------------------------------------
--  Functions
----------------------------------------------------------------------------*/

extern void TilesetCclRegister(); /// Register CCL features for tileset

//@}

#endif // !TILESET_H
