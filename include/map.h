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
/**@name map.h		-	The map headerfile. */
//
//	(c) Copyright 1998-2001 by Vladi Shabanski and Lutz Sammer
//
//	FreeCraft is free software; you can redistribute it and/or modify
//	it under the terms of the GNU General Public License as published
//	by the Free Software Foundation; either version 2 of the License,
//	or (at your option) any later version.
//
//	FreeCraft is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//	GNU General Public License for more details.
//
//	$Id$

#ifndef __MAP_H__
#define __MAP_H__

//@{

/*----------------------------------------------------------------------------
--	Documentation
----------------------------------------------------------------------------*/

/**
**	@struct _map_field_ map.h
**
**	\#include "map.h"
**
**	typedef struct _map_field_ MapField;
**
**	This structure contains all informations about a field on map.
**	It contains it look, properties and content.
**
**	The map-field structure members:
**
**	MapField::Tile
**
**		Tile is number defining the graphic image display for the
**		map-field. 65535 different tiles are supported. A tile is
**		currently 32x32 pixels. In the future is planned to support
**		animated tiles.
**
**	MapField::SeenTile
**
**		This is the tile number, that the player sitting on the computer
**		currently knows. Idea: Can be uses for illusions.
**
**	MapField::Flags
**
**		Contains special information of that tile. What units are
**		on this field, what units could be placed on this field.
**
**		This is the list of all flags currently used:
**
**		::MapFieldVisible field is visible.
**		::MapFieldExplored field is explored.
**		::MapFieldHuman	human player is the owner of the field used for
**			walls.
**		::MapFieldLandAllowed land units are allowed.
**		::MapFieldCoastAllowed coast units (transporter) and coast
**			buildings (shipyard) are allowed.
**		::MapFieldWaterAllowed water units allowed.
**		::MapFieldNoBuilding no buildings allowed.
**		::MapFieldUnpassable field is movement blocked.
**		::MapFieldWall field contains wall.
**		::MapFieldRocks	field contains rocks.
**		::MapFieldForest field contains forest.
**		::MapFieldLandUnit land unit on field.
**		::MapFieldAirUnit air unit on field.
**		::MapFieldSeaUnit water unit on field.
**		::MapFieldBuilding building on field.
**
**		Note: We want to add support for more unit-types like under
**		ground units.
**
**	MapField::Value
**
**		Extra value for each tile. This currently only used for
**		walls, contains the remaining hit points of the wall and
**		for forest, contains the frames until they grow.
**
**	MapField::Visible[]
**
**		Counter how many units of the player can see this field. 0 the
**		field is not explored, 1 explored, n-1 unit see it. Currently
**		no more than 253 units can see a field.
**
**	MapField::Here MapField::Here::Units
**
**		Contains a list of all units currently on this field.
**		Note: currently units are only inserted at the insert point.
**		This means units of the size of 2x2 fields are inserted at the
**		top and right most map coordinate.
**
**	MapField::Region
**
**		Number of the region to that the tile belongs.
*/

/**
**	@struct _world_map_ map.h
**
**	\#include "map.h"
**
**	typedef struct _world_map_ WorldMap;
**
**	This structure contains all informations about a freecraft world.
**	A world is a rectangle of any size. In the future it is planned to
**	support multiple worlds.
**
**	The world-map structure members:
**
**	WorldMap::Width WorldMap::Height
**
**		The map size in tiles.
**
**	WorldMap::Fields
**
**		An array WorldMap::Width*WorldMap::Height of all fields
**		belonging to this map.
**
**	WorldMap::NoFogOfWar
**
**		Flag if true, the fog of war is disabled.
**
**	WorldMap::TerrainName
**
**		Terrain as name. Used for unit-type look changes depending on
**		the current terrain type. In the future we want to support
**		multiple terrains pro map.
**
**	WorldMap::Terrain
**
**		The terrain as number, this should be removed.
**
**	WorldMap::Tileset
**
**		Tileset data for the map. See ::Tileset. This contains all
**		information about the tile.
**
**	WorldMap::TileCount
**
**		How many graphic tiles are available.
**
**	WorldMap::Tiles
**
**		Pointer into the tile graphic data. Used to find fast the start
**		of different tiles.
**
**	WorldMap::TileData
**
**		Tiles graphic for the map, loaded from WorldMap::Tileset::File.
**
**	WorldMap::Description[32]
**
**		Short description of the map.
**
**	WorldMap::Info
**
**		Descriptive information of the map.
*/

/*----------------------------------------------------------------------------
--	Includes
----------------------------------------------------------------------------*/

#include "unit.h"
#include "video.h"
#include "tileset.h"

/*----------------------------------------------------------------------------
--	Map
----------------------------------------------------------------------------*/

// JOHNS:	only limited by computer memory
//			OLD	NEW Code
//	 512x512:	  2 MB	 3 MB
//	1024x1024:	  8 MB  12 MB
//	2048*2048:	 32 MB  48 MB
//	4096*4096:	128 MB 192 MB
#define MaxMapWidth	1024		/// maximal map width supported
#define MaxMapHeight	1024		/// maximal map height supported

/*----------------------------------------------------------------------------
--	Map - region
----------------------------------------------------------------------------*/

#ifdef NEW_REGIONS

/**
**	Map region typedef
*/
typedef struct _map_region_  MapRegion;

/**
**	A region of the map
*/
struct _map_region_ {
    unsigned short	X;		/// X tile map center of region
    unsigned short	Y;		/// Y tile map center of region
    char	Forest;			/// Region contains forest
    int		NumNeighbors;		/// How many neighbors
    MapRegion** Neightbors;		/// Neightbors of this region
    int		MoveCosts;		/// Costs to move to this field
};

#endif

/*----------------------------------------------------------------------------
--	Map - field
----------------------------------------------------------------------------*/

#ifdef UNIT_ON_MAP
/**
**	All units on a map field, if more than one.
**	FIXME: unused
*/
typedef struct _unit_array_ {
    Unit*		Building;	/// Building or corpse
    Unit*		SeaUnit;	/// Sea unit
    Unit*		LandUnit;	/// Land unit
    Unit*		AirUnit;	/// Air unit
} UnitArray;

#endif

    /// Describes a field of the map
typedef struct _map_field_ {
    unsigned short	Tile;		/// graphic tile number
    unsigned short	SeenTile;	/// last seen tile (FOW)
    unsigned short	Flags;		/// field flags
    unsigned char	Value;		/// HP for walls/ Wood Regeneration
    // FIXME: this could be removed and encoded into visible/expored
    unsigned char	VisibleLastFrame;   /// Visible last frame
#ifdef NEW_FOW
    unsigned char	VisibleMask:4;	/// Visible mask
    unsigned char	ExploredMask:4;	/// Explored mask
    unsigned short	Visible;	/// Visible flags for all players
    unsigned short	Explored;	/// Explored flags for all players
#endif
#ifdef UNIT_ON_MAP
    union {
	Unit*		Units;		/// An unit on the map field
	UnitArray*	Array;		/// More units on the map field
    }			Here;		/// What is on the field
#endif
#ifdef UNITS_ON_MAP
// FIXME: Unused
    UnitRef		Building;	/// Building or corpse
    UnitRef		AirUnit;	/// Air unit
    UnitRef		LandUnit;	/// Land unit
    UnitRef		SeaUnit;	/// Sea unit
#endif
#ifdef NEW_REGIONS
    MapRegion*		Region;		/// Region to which the field belongs
#endif
} MapField;

// FIXME: should be removed
#define MapFieldCompletelyVisible   0x0001  /// Field completely visible
#define MapFieldPartiallyVisible    0x0002  /// Field partially visible

#ifndef NEW_FOW
// 0 Unexplored, 1 Explored, 2 PartialVisible, 3 CompleteVisible
#define MapFieldVisible		0x0001	/// Field visible
#define MapFieldExplored	0x0002	/// Field explored
#endif

//#define MapFieldArray		0x0004	/// More than one unit on the field

#define MapFieldHuman		0x0008	/// Human is owner of the field (walls)

#define MapFieldLandAllowed	0x0010	/// Land units allowed
#define MapFieldCoastAllowed	0x0020	/// Coast (transporter) units allowed
#define MapFieldWaterAllowed	0x0040	/// Water units allowed
#define MapFieldNoBuilding	0x0080	/// No buildings allowed

#define MapFieldUnpassable	0x0100	/// Field is movement blocked
#define MapFieldWall		0x0200	/// Field contains wall
#define MapFieldRocks		0x0400	/// Field contains rocks
#define MapFieldForest		0x0800	/// Field contains forest

#define MapFieldLandUnit	0x1000	/// Land unit on field
#define MapFieldAirUnit		0x2000	/// Air unit on field
#define MapFieldSeaUnit		0x4000	/// Water unit on field
#define MapFieldBuilding	0x8000	/// Building on field

/*----------------------------------------------------------------------------
--	Map info structure
----------------------------------------------------------------------------*/

/**
**	Get info about a map.
*/
typedef struct _map_info_ {
    char*	Description;		/// Map description
    char*	MapTerrainName;		/// Map terrain name
    // FIXME: Map Terrain Nr. should be removed.
    int		MapTerrain;		/// Map terrain
    int		MapWidth;		/// Map width
    int		MapHeight;		/// Map height
    int		PlayerType[16];		/// Same player->Type
    int		PlayerSide[16];		/// Same player->Side
    int		PlayerGold[16];		/// Same player->Gold
    int		PlayerWood[16];		/// Same player->Wood
    int		PlayerOil[16];		/// Same player->Oil
    // FIXME: Add NEW RESOURCES
    int		PlayerAi[16];		/// Same player->Ai
    unsigned int MapUID;		/// Unique Map ID (hash)
} MapInfo;

/*----------------------------------------------------------------------------
--	Map itself
----------------------------------------------------------------------------*/

    /// Describes the wold map
typedef struct _world_map_ {
    unsigned		Width;		/// the map width
    unsigned		Height;		/// the map height

    MapField*		Fields;		/// fields on map

#ifdef NEW_REGIONS
    MapRegion**		Regions;	/// Regions of this map
#endif

    unsigned char	NoFogOfWar;	/// fog of war disabled

    char*		TerrainName;	/// terrain as name
    // FIXME: terrain nr. should be removed?
    unsigned		Terrain;	/// terrain type (summer,winter,...)
    Tileset*		Tileset;	/// tileset data

    unsigned		TileCount;	/// how many tiles are available
    unsigned char**	Tiles;		/// pointer to tile data
    Graphic*		TileData;	/// tiles graphic for map

    char		Description[32];/// map description short

    MapInfo*		Info;		/// descriptive information (FIXME: DUPLICATES!)
} WorldMap;

/*----------------------------------------------------------------------------
--	Variables
----------------------------------------------------------------------------*/

extern WorldMap TheMap;			/// The current map

extern unsigned MapX;			/// The map X tile start on display
extern unsigned MapY;			/// The map Y tile start on display
    /// map width in tiles for current mode (14 for 640x480)
extern unsigned MapWidth;
    /// map height in tiles for current mode (14 for 640x480)
extern unsigned MapHeight;

extern char MustRedrawRow[MAXMAP_W];		/// Flags must redraw map row
extern char MustRedrawTile[MAXMAP_W*MAXMAP_H];	/// Flags must redraw tile

    ///  Fast draw tile, display and video mode independ
extern void (*VideoDrawTile)(const unsigned char*,int,int);

    /// Use original style fog of war
extern int OriginalFogOfWar;
    /// Contrast of fog of war
extern int FogOfWarContrast;
    /// Brightness of fog of war
extern int FogOfWarBrightness;
    /// Saturation of fog of war
extern int FogOfWarSaturation;
    /// Forest regeneration
extern int ForestRegeneration;
    /// Flag must reveal the map
extern int FlagRevealMap;

/*----------------------------------------------------------------------------
--	Functions
----------------------------------------------------------------------------*/

//
//	in map_draw.c
//
    /// Fast draw 32x32 tile for 32 bpp video modes
//extern void VideoDraw32Tile32(const unsigned char* data,int x,int y);
    /// Fast draw 32x32 tile for 16 bpp video modes
//extern void VideoDraw16Tile32(const unsigned char* data,int x,int y);
    /// Fast draw 32x32 tile for  8 bpp video modes
//extern void VideoDraw8Tile32(const unsigned char* data,int x,int y);

    /// Called when the color cycles
extern void MapColorCycle(void);
    /// Draw the map background
extern void DrawMapBackground(int x,int y);
    /// Build tables for map
extern void InitMap(void);

    /// Mark position inside screenmap be drawn for next display update
extern int MarkDrawPosMap( int x, int y );
    /// Denote wether area in map is overlapping
extern int MapAreaVisibleOnScreen( int sx, int sy, int ex, int ey );
    /// Check if any part of an area is visible
extern int AnyMapAreaVisibleOnScreen( int sx, int sy, int ex, int ey );
    /// Set overlapping area as entries in MustRedrawRow and MustRedrawTile
extern  int MarkDrawAreaMap( int sx, int sy, int ex, int ey );
    /// Set all entries in MustRedrawRow and MustRedrawTile
extern  void MarkDrawEntireMap(void);

//
//	in map_fog.c
//
#ifdef NEW_FOW
    /// Mark the sight in range
extern void MapMarkSight(const Player*,int,int,int);
    /// Mark the new sight in range
extern void MapMarkNewSight(const Player*,int,int,int,int,int);
#else
    /// Mark the sight in range
extern void MapMarkSight(int tx,int ty,int range);
    /// Mark the new sight in range
extern void MapMarkNewSight(int,int,int,int,int);
#endif
    /// Mark tiles with fog of war to be redrawn
extern void MapUpdateFogOfWar(int x,int y);
    ///	Update fog of war
extern void UpdateFogOfWarChange(void);
    /// Update visible areas for fog of war
extern void MapUpdateVisible(void);
    /// Draw the map fog of war
extern void DrawMapFogOfWar(int x,int y);
    /// Build tables for fog of war
extern void InitMapFogOfWar(void);

//
//	in map_wall.c
//
    ///	Check if the seen tile-type is wall
extern int MapIsSeenTileWall(int x, int y,int walltype);
    ///	Correct the seen wall field, depending on the surrounding
extern void MapFixSeenWallTile(int x, int y);
    ///	Correct the surrounding seen wall fields
extern void MapFixSeenWallNeighbors(int x, int y);
    ///	Correct the real wall field, depending on the surrounding
extern void MapFixWallTile(int x, int y);
    /// Remove wall on tile
extern void MapRemoveWall(unsigned x,unsigned y);
    /// Wall is hit
extern void HitWall(unsigned x,unsigned y,unsigned damage);

//
//	in map_wood.c
//
    ///	Check if the seen tile-type is wood
extern int MapIsSeenTileWood(int x, int y);
    ///	Correct the seen wood field, depending on the surrounding
extern void MapFixSeenWoodTile(int x, int y);
    ///	Correct the surrounding seen wood fields
extern void MapFixSeenWoodNeighbors(int x, int y);
    ///	Correct the real wood field, depending on the surrounding
extern void MapFixWoodTile(int x, int y);
    /// Remove wood from the map
extern void MapRemoveWood(unsigned x,unsigned y);
    /// Regenerate the forest
extern void RegenerateForest(void);

//
//	in map_rock.c
//
    ///	Check if the seen tile-type is rock
extern int MapIsSeenTileRock(int x, int y);
    ///	Correct the seen rock field, depending on the surrounding
extern void MapFixSeenRockTile(int x, int y);
    ///	Correct the surrounding seen rock fields
extern void MapFixSeenRockNeighbors(int x, int y);
    ///	Correct the real rock field, depending on the surrounding
extern void MapFixRockTile(int x, int y);
    /// Remove rock from the map
extern void MapRemoveRock(unsigned x,unsigned y);

//
//	in ccl_map.c
//
    /// register ccl features
extern void MapCclRegister(void);

//
//	mixed sources
//
    /// Load a map
extern void LoadMap(const char* file,WorldMap* map);
    /// Save the map
extern void SaveMap(FILE* file);
    /// Clean the map
extern void CleanMap(void);

    /// Release info for a map
extern void FreeMapInfo(MapInfo* info);

    /// Mark a tile as seen by the player
extern void MapMarkSeenTile(int x,int y);
    /// Reveal the complete map, make everything known
extern void RevealMap(void);

    /// Center map on point
extern void MapCenter(int x,int y);
    /// Set the current map view to x,y (upper,left corner)
extern void MapSetViewpoint(int x,int y);

    /// Returns true, if the tile field is empty
extern int IsMapFieldEmpty(int x,int y);
    /// Returns true, if water on the map tile field
extern int WaterOnMap(int x,int y);
    /// Returns true, if coast on the map tile field
extern int CoastOnMap(int x,int y);

    /// Returns true, if wall on the map tile field
extern int WallOnMap(int x,int y);
    /// Returns true, if human wall on the map tile field
extern int HumanWallOnMap(int x,int y);
    /// Returns true, if orc wall on the map tile field
extern int OrcWallOnMap(int x,int y);

    /// Returns true, if forest on the map tile field with bounds check
extern int CheckedForestOnMap(int x,int y);
    /// Returns true, if forest on the map tile field
extern int ForestOnMap(int x,int y);

    /// Returns true, if rock on the map tile field
extern int RockOnMap(int x,int y);

    /// Returns true, if the unit-type(mask can enter field with bounds check
extern int CheckedCanMoveToMask(int x,int y,int mask);
    /// Returns true, if the unit-type(mask) can enter the field
extern int CanMoveToMask(int x,int y,int mask);
    /// Returns true, if the unit-type can enter the field
extern int UnitTypeCanMoveTo(int x,int y,const UnitType* type);
    /// Returns true, if the unit can enter the field
extern int UnitCanMoveTo(int x,int y,const Unit* unit);

    /// Preprocess map, for internal use.
extern void PreprocessMap(void);

    /// Set wall on field
extern void MapSetWall(unsigned x,unsigned y,int humanwall);

/*----------------------------------------------------------------------------
--	Defines
----------------------------------------------------------------------------*/

    /// Tile number of the tile drawn for unexplored space
#define UNEXPLORED_TILE 0

    /// Can an unit with 'mask' can enter the field
#define CanMoveToMask(x,y,mask) \
	!(TheMap.Fields[(x)+(y)*TheMap.Width].Flags&(mask))

#ifdef NEW_FOW

    /// Check if a field for the user is explored
#define IsMapFieldExplored(x,y) \
    (TheMap.Fields[(y)*TheMap.Width+(x)].Explored&(1<<ThisPlayer->Player))

    /// Check if a field for the user is visibile
#define IsMapFieldVisible(x,y) \
    (TheMap.Fields[(y)*TheMap.Width+(x)].Visible&(1<<ThisPlayer->Player))
#else

    /// Check if a field for the user is explored
#define IsMapFieldExplored(x,y) \
    (TheMap.Fields[(y)*TheMap.Width+(x)].Flags&MapFieldExplored)

    /// Check if a field for the user is visibile
#define IsMapFieldVisible(x,y) \
    (TheMap.Fields[(y)*TheMap.Width+(x)].Flags&MapFieldVisible)

#endif

//@}

#endif	// !__MAP_H__
