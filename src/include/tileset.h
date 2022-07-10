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
#include <string>
#include <vector>

struct lua_State;

// Not used until now:    
#define MapFieldSpeedMask               0x0000'0000'0000'0003  /// Move faster on this tile

#define MapFieldOpaque                  0x0000'0000'0000'0004  /// Units can't see through this field (FoW)

#define MapFieldHuman                   0x0000'0000'0000'0008  /// Human is owner of the field (walls)

#define MapFieldLandAllowed             0x0000'0000'0000'0010  /// Land units allowed
#define MapFieldCoastAllowed            0x0000'0000'0000'0020  /// Coast (transporter) units allowed
#define MapFieldWaterAllowed            0x0000'0000'0000'0040  /// Water units allowed
#define MapFieldNoBuilding              0x0000'0000'0000'0080  /// No buildings allowed

#define MapFieldUnpassable              0x0000'0000'0000'0100  /// Field is movement blocked
#define MapFieldWall                    0x0000'0000'0000'0200  /// Field contains wall
#define MapFieldRocks                   0x0000'0000'0000'0400  /// Field contains rocks
#define MapFieldForest                  0x0000'0000'0000'0800  /// Field contains forest or other harvestable resource

#define MapFieldLandUnit                0x0000'0000'0000'1000  /// Land unit on field
#define MapFieldAirUnit                 0x0000'0000'0000'2000  /// Air unit on field
#define MapFieldSeaUnit                 0x0000'0000'0000'4000  /// Water unit on field
#define MapFieldBuilding                0x0000'0000'0000'8000  /// Building on field

#define MapFieldDecorative              0x0000'0000'0001'0000  /// A field that needs no mixing with the surroundings, for the editor
#define MapFieldCost4                  (0x0000'0000'0002'0000 | MapFieldForest) /// This field is terrain harvestable, but gives Cost4 instead of wood
#define MapFieldCost5                  (0x0000'0000'0004'0000 | MapFieldForest) /// This field is terrain harvestable, but gives Cost5 instead of wood
#define MapFieldCost6                  (0x0000'0000'0008'0000 | MapFieldForest) /// This field is terrain harvestable, but gives Cost6 instead of wood

#define MapFieldSubtilesMax             16
#define MapFieldSubtilesUnpassableShift 48
#define MapFieldSubtilesUnpassableMask  0xffffL << MapFieldSubtilesUnpassableShift /// Up to 16 unpassable subtiles, never used in MapField, only in CTile

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
struct CTileInfo {
public:
	CTileInfo() : BaseTerrain(0), MixTerrain(0)
	{}
	CTileInfo(unsigned char base, unsigned char mix) : BaseTerrain(base), MixTerrain(mix)
	{}

	bool operator ==(const CTileInfo &rhs) const
	{
		return BaseTerrain == rhs.BaseTerrain && MixTerrain == rhs.MixTerrain;
	}
	bool operator !=(const CTileInfo &rhs) const { return !(*this == rhs); }

public:
	unsigned char BaseTerrain; /// Basic terrain of the tile
	unsigned char MixTerrain;  /// Terrain mixed with this
};

/// Definition for a terrain type
struct SolidTerrainInfo {
	std::string TerrainName;  /// Name of the terrain
	// TODO: When drawing with the editor add some kind fo probabilities for every tile.
};

class CTile
{
public:
	CTile() : tile(0), flag(0) {}

public:
	unsigned short tile;  /// graphical pos
	uint64_t flag;        /// tile flags
	CTileInfo tileinfo;   /// Tile descriptions
};

/// Tileset definition
class CTileset
{
public:
	void clear();

	unsigned int getTileCount() const { return tiles.size(); }

	unsigned int getDefaultTileIndex() const;
	unsigned int getDefaultWoodTileIndex() const;

	bool isAWallTile(unsigned tile) const;
	bool isARaceWallTile(unsigned tile, bool human) const;
	bool isAWoodTile(unsigned tile) const;
	bool isARockTile(unsigned tile) const;

	/**
	 * Size of a graphical (not logical!) tile in pixels.
	 * 
	 * @return const PixelSize& 
	 */
	const PixelSize &getPixelTileSize() const { return pixelTileSize; }
	const int getLogicalToGraphicalTileSizeMultiplier() const { return logicalTileToGraphicalTileMultiplier; }
	const int getLogicalToGraphicalTileSizeShift() const { return logicalTileToGraphicalTileShift; }
	const int getGraphicalTileSizeShiftX() const { return graphicalTileSizeShiftX; }
	const int getGraphicalTileSizeShiftY() const { return graphicalTileSizeShiftY; }

	unsigned getRemovedRockTile() const { return removedRockTile; }
	unsigned getRemovedTreeTile() const { return removedTreeTile; }
	unsigned getBottomOneTreeTile() const { return botOneTreeTile; }
	unsigned getTopOneTreeTile() const { return topOneTreeTile; }
	unsigned getMidOneTreeTile() const { return midOneTreeTile; }

	unsigned getWallDirection(int tileIndex, bool human) const;

	unsigned getHumanWallTileIndex(int dirFlag) const;
	unsigned getOrcWallTileIndex(int dirFlag) const;
	unsigned getHumanWallTileIndex_broken(int dirFlag) const;
	unsigned getOrcWallTileIndex_broken(int dirFlag) const;
	unsigned getHumanWallTileIndex_destroyed(int dirFlag) const;
	unsigned getOrcWallTileIndex_destroyed(int dirFlag) const;

	unsigned int getSolidTerrainCount() const;

	const std::string &getTerrainName(int solidTerrainIndex) const;

	int findTileIndexByTile(unsigned int tile) const;
	unsigned int getTileNumber(int basic, bool random, bool filler) const;
	void fillSolidTiles(std::vector<unsigned int> *solidTiles) const;

	unsigned getQuadFromTile(unsigned int tile) const;
	int getTileBySurrounding(unsigned short type,
							 int up, int right,
							 int bottom, int left) const;
	int tileFromQuad(unsigned fixed, unsigned quad) const;
	bool isEquivalentTile(unsigned int tile1, unsigned int tile2) const;

	void parse(lua_State *l);
	void buildTable(lua_State *l);
	int parseTilesetTileFlags(lua_State *l, uint64_t *back, int *j);
	int findTileIndex(unsigned char baseTerrain, unsigned char mixTerrain = 0) const;

private:
	unsigned int getOrAddSolidTileIndexByName(const std::string &name);
	int getTileIndex(unsigned char baseTerrain, unsigned char mixTerrain, unsigned int quad) const;
	void buildWallReplacementTable();
	void parseSlots(lua_State *l, int t);
	void parseSpecial(lua_State *l);
	void parseSolid(lua_State *l);
	void parseMixed(lua_State *l);
	int findTilePath(int base, int goal, int length, std::vector<char> &marks, int *tileIndex) const;
public:
	std::string Name;           /// Nice name to display
	std::string ImageFile;      /// File containing image data

public:
	std::vector<CTile> tiles;

	// TODO: currently hardcoded
	std::vector<unsigned char> TileTypeTable;  /// For fast lookup of tile type
private:
	PixelSize pixelTileSize;    /// Size of a tile in pixel

	// some cached values based on pixelTileSize and the logical tile size in the game
	uint8_t logicalTileToGraphicalTileMultiplier; /// By what to multiply logical tile coordinates to get graphical tile coordinates
	uint8_t logicalTileToGraphicalTileShift;      /// By what to shift logical tile coordinates to get graphical tile coordinates
	uint8_t graphicalTileSizeShiftX; /// 1<<shift size for graphical tiles in X direction
	uint8_t graphicalTileSizeShiftY; /// 1<<shift size for graphical tiles in Y direction
	
	std::vector<SolidTerrainInfo> solidTerrainTypes; /// Information about solid terrains.
#if 1
	std::vector<int> mixedLookupTable;  /// Lookup for what part of tile used
	unsigned topOneTreeTile;   /// Tile for one tree top
	unsigned midOneTreeTile;   /// Tile for one tree middle
	unsigned botOneTreeTile;   /// Tile for one tree bottom
	unsigned removedTreeTile;  /// Tile placed where trees are gone
	int woodTable[20];     /// Table for tree removable
	unsigned topOneRockTile;   /// Tile for one rock top
	unsigned midOneRockTile;   /// Tile for one rock middle
	unsigned botOneRockTile;   /// Tile for one rock bottom
	unsigned removedRockTile;  /// Tile placed where rocks are gone
	int rockTable[20];     /// Removed rock placement table
	unsigned humanWallTable[16];  /// Human wall placement table
	unsigned orcWallTable[16];    /// Orc wall placement table
#endif
};

/*----------------------------------------------------------------------------
--  Functions
----------------------------------------------------------------------------*/

extern void ParseTilesetTileFlags(lua_State *l, int *back, int *j);

//@}

#endif // !TILESET_H
