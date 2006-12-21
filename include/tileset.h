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
//      $Id$

#ifndef __TILESET_H__
#define __TILESET_H__

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
--  Declarations
----------------------------------------------------------------------------*/


extern int TileSizeX; /// Size of a tile in X
extern int TileSizeY; /// Size of a tile in Y

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
	TileTypeCoast,      /// Any coast tile
	TileTypeWater,      /// Any water tile
};

	/// Single tile definition
struct TileInfo {
	unsigned char BaseTerrain; /// Basic terrain of the tile
	unsigned char MixTerrain;  /// Terrain mixed with this
};

	/// Definition for a terrain type
struct SolidTerrainInfo {
	char *TerrainName;  /// Name of the terrain
	// TODO: When drawing with the editor add some kind fo probabilities for every tile.
};

	/// Tileset definition
class CTileset {
public:
	void Clear() {
		Name.clear();
		ImageFile.clear();
		NumTiles = 0;
		TileSizeX = 0;
		TileSizeY = 0;
		delete[] Table;
		Table = NULL;
		delete[] FlagsTable;
		FlagsTable = NULL;
		delete[] Tiles;
		Tiles = NULL;
		delete[] TileTypeTable;
		TileTypeTable = NULL;
		for (int i = 0; i < NumTerrainTypes; ++i) {
			delete[] SolidTerrainTypes[i].TerrainName;
		}
		delete[] SolidTerrainTypes;
		SolidTerrainTypes = NULL;
		NumTerrainTypes = 0;
	}
	std::string Name;           /// Nice name to display
	std::string ImageFile;      /// File containing image data

	int NumTiles;               /// Number of tiles in the tables
	int TileSizeX;              /// Size of a tile in X
	int TileSizeY;              /// Size of a tile in Y
	unsigned short *Table;      /// Pud to internal conversion table
	unsigned short *FlagsTable; /// Flag table for editor

	TileInfo *Tiles; /// Tile descriptions

	// TODO: currently hardcoded
	unsigned char *TileTypeTable;   /// For fast lookup of tile type

	int NumTerrainTypes;                 /// Number of different terrain types
	SolidTerrainInfo *SolidTerrainTypes; /// Information about solid terrains.
};

/*----------------------------------------------------------------------------
--  Functions
----------------------------------------------------------------------------*/

extern void LoadTileset(void);   /// Load tileset definition
extern void CleanTilesets(void); /// Cleanup the tileset module

extern void TilesetCclRegister(void); /// Register CCL features for tileset

//@}

#endif // !__TILESET_H__
