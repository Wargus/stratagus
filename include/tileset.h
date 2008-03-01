//     ____                _       __               
//    / __ )____  _____   | |     / /___ ___________
//   / __  / __ \/ ___/   | | /| / / __ `/ ___/ ___/
//  / /_/ / /_/ (__  )    | |/ |/ / /_/ / /  (__  ) 
// /_____/\____/____/     |__/|__/\__,_/_/  /____/  
//                                              
//       A futuristic real-time strategy game.
//          This file is part of Bos Wars.
//
/**@name tileset.h - The tileset headerfile. */
//
//      (c) Copyright 1998-2008 by Lutz Sammer and Jimmy Salmon
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
		NumTiles = 0;
		TileSizeX = 0;
		TileSizeY = 0;
		delete[] Table;
		Table = NULL;
		delete[] FlagsTable;
		FlagsTable = NULL;
		delete[] Tiles;
		Tiles = NULL;
		for (int i = 0; i < NumTerrainTypes; ++i) {
			delete[] SolidTerrainTypes[i].TerrainName;
		}
		delete[] SolidTerrainTypes;
		SolidTerrainTypes = NULL;
		NumTerrainTypes = 0;
	}
	std::string Name;           /// Nice name to display

	int NumTiles;               /// Number of tiles in the tables
	int TileSizeX;              /// Size of a tile in X
	int TileSizeY;              /// Size of a tile in Y
	unsigned short *Table;      /// Pud to internal conversion table
	unsigned short *FlagsTable; /// Flag table for editor

	TileInfo *Tiles; /// Tile descriptions

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
