//   ___________		     _________		      _____  __
//   \_	  _____/______   ____   ____ \_   ___ \____________ _/ ____\/  |_
//    |    __) \_  __ \_/ __ \_/ __ \/    \  \/\_  __ \__  \\   __\\   __\ 
//    |     \   |  | \/\  ___/\  ___/\     \____|  | \// __ \|  |   |  |
//    \___  /   |__|    \___  >\___  >\______  /|__|  (____  /__|   |__|
//	  \/		    \/	   \/	     \/		   \/
//  ______________________                           ______________________
//			  T H E   W A R   B E G I N S
//	   FreeCraft - A free fantasy real time strategy game engine
//
/**@name tileset.h	-	The tileset headerfile. */
//
//	(c) Copyright 1998-2000 by Lutz Sammer
//
//	$Id$

#ifndef __TILESET_H__
#define __TILESET_H__

//@{

/*----------------------------------------------------------------------------
--	Declarations
----------------------------------------------------------------------------*/

#define TileSizeX	32		/// Size of a tile in X
#define TileSizeY	32		/// Size of a tile in Y

//#define MaxTilesInTileset	1024	/// Current limit of tiles in tileset
#define MaxTilesInTileset	3072	/// Current limit of tiles in tileset

#if 0

#define TILE_PER_ROW	16		/// tiles stored in an image row
#define TILE_ROWS	24		/// tiles rows in the image
#define TILE_COUNT	(TILE_PER_ROW*TILE_ROWS)

#endif


/**
**   These are used for lookup tiles types
**   mainly used for the FOW implementation of the seen woods/rocks
*/
enum _tile_type_ {
    TileTypeUnknown,			/// unknown tile type
    TileTypeNoWood,			/// UNUSED: no wood tile
    TileTypeWood,			/// any wood tile
    TileTypeGrass,			/// any grass tile
    TileTypeNoRock,			/// UNUSED: no rock tile
    TileTypeRock,			/// any rock tile
    TileTypeCoast,			/// any coast tile
    TileTypeHumanWall,			/// any human wall tile
    TileTypeOrcWall,			/// any orc wall tile
    TileTypeNoWall,			/// UNUSED: no wall tile
    TileTypeWater,			/// any water tile
};

/**
**	Tileset definition.
*/
typedef struct _tileset_ {
    char*	Ident;			/// tileset identifier
    char*	Name;			/// name for future extensions
    char*	File;			/// file containing image data

    const unsigned short* Table;	/// pud to internal conversion table
    unsigned char* TileTypeTable;	/// for fast lookup of tile type
    unsigned short* AnimationTable;	/// Tile Animation sequences

#if 1
    // FIXME: old code should be complete removed.
    unsigned	HumanWall100Tile;	/// 100% wall
    unsigned	HumanWall50Tile;	/// 100% wall
    unsigned	HumanNoWallTile;	/// tile placed where walls are gone
    unsigned	OrcWall100Tile;		/// 100% wall
    unsigned	OrcWall50Tile;		/// 100% wall
    unsigned	OrcNoWallTile;		/// tile placed where walls are gone
#endif

    unsigned	ExtraTrees[6];		/// extra tree tiles for removed
    unsigned	TopOneTree;		/// tile for one tree top
    unsigned	MidOneTree;		/// tile for one tree middle
    unsigned	BotOneTree;		/// tile for one tree bottom
    unsigned	RemovedTree;		/// tile placed where trees are gone
    unsigned	GrowingTree[2];		/// Growing tree tiles

    unsigned	ExtraRocks[6];		/// extra rock tiles for removed
    unsigned	TopOneRock;		/// tile for one rock top
    unsigned	MidOneRock;		/// tile for one rock middle
    unsigned	BotOneRock;		/// tile for one rock bottom
    unsigned	RemovedRock;		/// tile placed where rocks are gone
} Tileset;

// FIXME: this #define's should be removed

#define TilesetSummer		0	/// Reference number
#define TilesetWinter		1	/// Reference number
#define TilesetWasteland	2	/// Reference number
#define TilesetSwamp		3	/// Reference number

// FIXME: allow more tilesets

#define TilesetMax		4	/// Biggest supported tilset number

/*----------------------------------------------------------------------------
--	Variables
----------------------------------------------------------------------------*/

extern char** TilesetWcNames;		/// Mapping wc-number 2 symbol
extern Tileset Tilesets[TilesetMax];	/// Tileset information

/*----------------------------------------------------------------------------
--	Functions
----------------------------------------------------------------------------*/

extern void LoadTileset(void);		/// Load tileset definition
extern void SaveTileset(FILE*);		/// Save the tileset configuration
extern void CleanTileset(void);		/// Cleanup the tileset module

extern void TilesetCclRegister(void);	/// Register CCL features for tileset

/*----------------------------------------------------------------------------
--	Predicates
----------------------------------------------------------------------------*/

// True if this is the fog color
#define COLOR_FOG_P(x) ((x) == 239 || (x) == 228)
#define COLOR_FOG (0)

//@}

#endif // !__TILESET_H__
