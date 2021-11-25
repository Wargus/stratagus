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

using tile_index 		= uint16_t;
using graphic_index		= uint16_t;
using tile_flags 		= uint64_t;
using terrain_typeIdx 	= uint8_t;

// Not used until now: 
constexpr tile_flags MapFieldSpeedMask		{0x0000'0000'0000'0003};	/// Move faster on this tile

constexpr tile_flags MapFieldOpaque			{0x0000'0000'0000'0004};	/// Units can't see through this field (FoV/FoW)

constexpr tile_flags MapFieldHuman			{0x0000'0000'0000'0008};	/// Human is owner of the field (walls)

constexpr tile_flags MapFieldLandAllowed	{0x0000'0000'0000'0010};	/// Land units allowed
constexpr tile_flags MapFieldCoastAllowed	{0x0000'0000'0000'0020};	/// Coast (transporter) units allowed
constexpr tile_flags MapFieldWaterAllowed	{0x0000'0000'0000'0040};	/// Water units allowed
constexpr tile_flags MapFieldNoBuilding		{0x0000'0000'0000'0080};	/// No buildings allowed

constexpr tile_flags MapFieldUnpassable		{0x0000'0000'0000'0100};	/// Field is movement blocked
constexpr tile_flags MapFieldWall			{0x0000'0000'0000'0200};	/// Field contains wall
constexpr tile_flags MapFieldRocks			{0x0000'0000'0000'0400};	/// Field contains rocks
constexpr tile_flags MapFieldForest			{0x0000'0000'0000'0800};	/// Field contains forest

constexpr tile_flags MapFieldLandUnit		{0x0000'0000'0000'1000};	/// Land unit on field
constexpr tile_flags MapFieldAirUnit		{0x0000'0000'0000'2000};	/// Air unit on field
constexpr tile_flags MapFieldSeaUnit		{0x0000'0000'0000'4000};	/// Water unit on field
constexpr tile_flags MapFieldBuilding		{0x0000'0000'0000'8000};	/// Building on field

constexpr tile_flags MapFieldDecorative		{0x0000'0000'0001'0000};	/// A field that needs no mixing with the surroundings, for the editor

constexpr tile_flags MapFieldCost4			{0x0000'0000'0002'0000 | MapFieldForest};	/// This field is terrain harvestable, but gives Cost4 instead of wood
constexpr tile_flags MapFieldCost5			{0x0000'0000'0004'0000 | MapFieldForest};	/// This field is terrain harvestable, but gives Cost5 instead of wood
constexpr tile_flags MapFieldCost6			{0x0000'0000'0008'0000 | MapFieldForest};	/// This field is terrain harvestable, but gives Cost6 instead of wood

constexpr tile_flags MapFieldNonMixing		{0x0000'0000'8000'0000};	/// special flag - this isn't it's own name, but it doesn't mix

constexpr uint8_t MapFieldSubtilesMax				{16};
constexpr uint8_t MapFieldSubtilesUnpassableShift	{48};
constexpr tile_flags MapFieldSubtilesUnpassableMask	{0xFFFFL << MapFieldSubtilesUnpassableShift}; /// Up to 16 unpassable subtiles, never used in MapField, only in CTile


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
	CTileInfo(terrain_typeIdx base, terrain_typeIdx mix) : BaseTerrain(base), MixTerrain(mix)
	{}

	bool operator ==(const CTileInfo &rhs) const
	{
		return BaseTerrain == rhs.BaseTerrain && MixTerrain == rhs.MixTerrain;
	}
	bool operator !=(const CTileInfo &rhs) const { return !(*this == rhs); }

public:
	terrain_typeIdx BaseTerrain; /// Basic terrain of the tile
	terrain_typeIdx MixTerrain;  /// Terrain mixed with this
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
	graphic_index	tile;		/// graphical pos
	tile_flags		flag;		/// Flag
	CTileInfo		tileinfo;	/// Tile descriptions
};

/// Tileset definition
class CTileset
{
public:
	void clear();

	size_t getTileCount() const { return tiles.size(); }
	bool setTileCount(const size_t newCount);
	bool increaseTileCountBy(const size_t increaseBy) { return setTileCount(tiles.size() + increaseBy); }

	tile_index getDefaultTileIndex() const;
	tile_index getDefaultWoodTileIndex() const;

	bool isAWallTile(tile_index tile) const;
	bool isARaceWallTile(tile_index tile, bool human) const;
	bool isAWoodTile(tile_index tile) const;
	bool isARockTile(tile_index tile) const;

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

	graphic_index getRemovedRockTile() const { return removedRockTile; }
	graphic_index getRemovedTreeTile() const { return removedTreeTile; }
	graphic_index getBottomOneTreeTile() const { return botOneTreeTile; }
	graphic_index getTopOneTreeTile() const { return topOneTreeTile; }
	graphic_index getMidOneTreeTile() const { return midOneTreeTile; }

	unsigned getWallDirection(tile_index tileIndex, bool human) const;

	tile_index getHumanWallTileIndex(int dirFlag) const;
	tile_index getOrcWallTileIndex(int dirFlag) const;
	tile_index getHumanWallTileIndex_broken(int dirFlag) const;
	tile_index getOrcWallTileIndex_broken(int dirFlag) const;
	tile_index getHumanWallTileIndex_destroyed(int dirFlag) const;
	tile_index getOrcWallTileIndex_destroyed(int dirFlag) const;

	terrain_typeIdx getSolidTerrainCount() const;
	
	const std::string &getTerrainName(terrain_typeIdx solidTerrainIndex) const;

	int32_t findTileIndexByTile(graphic_index tile) const;
	tile_index getTileNumber(tile_index basic, bool random, bool filler) const;
	void fillSolidTiles(std::vector<unsigned int> *solidTiles) const;

	uint32_t getQuadFromTile(graphic_index tile) const;
	int getTileBySurrounding(tile_flags type,
							 int up, int right,
							 int bottom, int left) const;
	tile_index tileFromQuad(uint32_t fixed, uint32_t quad) const;
	bool isEquivalentTile(unsigned int tile1, unsigned int tile2) const;

	void parse(lua_State *l);
	void buildTable(lua_State *l);
	static bool ModifyFlag(const char *flagName, tile_flags *flag, const int subtileCount);
	static tile_flags parseTilesetTileFlags(lua_State *l, int *j);
	
	terrain_typeIdx getOrAddSolidTileIndexByName(const std::string &name);
	
	/// FIXME: Check if it realy needed
	terrain_typeIdx addDecoTerrainType() 
	{
		return getOrAddSolidTileIndexByName(std::to_string(solidTerrainTypes.size()));
	}

private:
	int32_t findTileIndex(terrain_typeIdx baseTerrain, terrain_typeIdx mixTerrain = 0) const;
	int32_t getTileIndex(terrain_typeIdx baseTerrain, terrain_typeIdx mixTerrain, uint32_t quad) const;
	void buildWallReplacementTable();
	void parseSlots(lua_State *l, int t);
	void parseSpecial(lua_State *l);
	void parseSolid(lua_State *l);
	void parseMixed(lua_State *l);
	int32_t findTilePath(int base, int goal, int length, std::vector<char> &marks, tile_index *tileIndex) const;

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
	std::vector<int> mixedLookupTable;	/// Lookup for what part of tile used
	graphic_index topOneTreeTile;		/// Tile for one tree top
	graphic_index midOneTreeTile;		/// Tile for one tree middle
	graphic_index botOneTreeTile;		/// Tile for one tree bottom
	graphic_index removedTreeTile;		/// Tile placed where trees are gone
	int woodTable[20];					/// Table for tree removable
	graphic_index topOneRockTile;		/// Tile for one rock top
	graphic_index midOneRockTile;		/// Tile for one rock middle
	graphic_index botOneRockTile;		/// Tile for one rock bottom
	graphic_index removedRockTile;		/// Tile placed where rocks are gone
	int rockTable[20];					/// Removed rock placement table
	tile_index humanWallTable[16];		/// Human wall placement table
	tile_index orcWallTable[16];			/// Orc wall placement table
#endif
};

/*----------------------------------------------------------------------------
--  Functions
----------------------------------------------------------------------------*/
//@}

#endif // !TILESET_H
