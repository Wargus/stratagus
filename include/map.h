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
/**@name map.h - The map headerfile. */
//
//      (c) Copyright 1998-2003 by Vladi Shabanski, Lutz Sammer,
//                              and Jimmy Salmon
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

#ifndef __MAP_H__
#define __MAP_H__

//@{

/*----------------------------------------------------------------------------
--  Documentation
----------------------------------------------------------------------------*/

/**
**  @struct _map_field_ map.h
**
**  \#include "map.h"
**
**  typedef struct _map_field_ MapField;
**
**  This structure contains all informations about a field on map.
**  It contains it look, properties and content.
**
**  The map-field structure members:
**
**  MapField::Tile
**
**    Tile is number defining the graphic image display for the
**    map-field. 65535 different tiles are supported. A tile is
**    currently 32x32 pixels. In the future is planned to support
**    animated tiles.
**
**  MapField::SeenTile
**
**    This is the tile number, that the player sitting on the computer
**    currently knows. Idea: Can be uses for illusions.
**
**  MapField::Flags
**
**    Contains special information of that tile. What units are
**    on this field, what units could be placed on this field.
**
**    This is the list of all flags currently used:
**
**    ::MapFieldVisible field is visible.
**    ::MapFieldExplored field is explored.
**    ::MapFieldHuman human player is the owner of the field used for
**      walls.
**    ::MapFieldLandAllowed land units are allowed.
**    ::MapFieldCoastAllowed coast units (transporter) and coast
**      buildings (shipyard) are allowed.
**    ::MapFieldWaterAllowed water units allowed.
**    ::MapFieldNoBuilding no buildings allowed.
**    ::MapFieldUnpassable field is movement blocked.
**    ::MapFieldWall field contains wall.
**    ::MapFieldRocks field contains rocks.
**    ::MapFieldForest field contains forest.
**    ::MapFieldLandUnit land unit on field.
**    ::MapFieldAirUnit air unit on field.
**    ::MapFieldSeaUnit water unit on field.
**    ::MapFieldBuilding building on field.
**
**    Note: We want to add support for more unit-types like under
**      ground units.
**
**  MapField::Value
**
**    Extra value for each tile. This currently only used for
**    walls, contains the remaining hit points of the wall and
**    for forest, contains the frames until they grow.
**
**  MapField::Visible[]
**
**    Counter how many units of the player can see this field. 0 the
**    field is not explored, 1 explored, n-1 unit see it. Currently
**    no more than 253 units can see a field.
**
**  MapField::Here MapField::Here::Units
**
**    Contains a list of all units currently on this field.
**    Note: currently units are only inserted at the insert point.
**    This means units of the size of 2x2 fields are inserted at the
**    top and right most map coordinate.
**
**  MapField::Region
**
**    Number of the region to that the tile belongs.
*/

/**
**  @struct _world_map_ map.h
**
**  \#include "map.h"
**
**  typedef struct _world_map_ WorldMap;
**
**  This structure contains all informations about a stratagus world.
**  A world is a rectangle of any size. In the future it is planned to
**  support multiple worlds.
**
**  The world-map structure members:
**
**  WorldMap::Width WorldMap::Height
**
**    The map size in tiles.
**
**  WorldMap::Fields
**
**    An array WorldMap::Width*WorldMap::Height of all fields
**    belonging to this map.
**
**  WorldMap::NoFogOfWar
**
**    Flag if true, the fog of war is disabled.
**
**  WorldMap::TerrainName
**
**    Terrain as name. Used for unit-type look changes depending on
**    the current terrain type. In the future we want to support
**    multiple terrains pro map.
**
**  WorldMap::Terrain
**
**    The terrain as number, this should be removed.
**
**  WorldMap::Tileset
**
**    Tileset data for the map. See ::Tileset. This contains all
**    information about the tile.
**
**  WorldMap::TileCount
**
**    How many graphic tiles are available.
**
**  WorldMap::Tiles
**
**    Pointer into the tile graphic data. Used to find fast the start
**    of different tiles.
**
**  WorldMap::TileData
**
**    Tiles graphic for the map, loaded from WorldMap::Tileset::File.
**
**  WorldMap::Description[32]
**
**    Short description of the map.
**
**  WorldMap::Info
**
**    Descriptive information of the map.
**    @see ::_map_info_
**    @todo This structure contains duplicate informations of the map.
*/

/*----------------------------------------------------------------------------
--  Includes
----------------------------------------------------------------------------*/

#include "unit.h"
#include "video.h"
#include "tileset.h"

/*----------------------------------------------------------------------------
--  Map
----------------------------------------------------------------------------*/

// JOHNS: only limited by computer memory
//            OLD    NEW Code
// 512x512:     2 MB   3 MB
// 1024x1024:   8 MB  12 MB
// 2048*2048:  32 MB  48 MB
// 4096*4096: 128 MB 192 MB
#define MaxMapWidth  256  ///<  maximal map width supported
#define MaxMapHeight 256  ///< maximal map height supported

/*----------------------------------------------------------------------------
--  Map - field
----------------------------------------------------------------------------*/

	/// Describes a field of the map
typedef struct _map_field_ {
	unsigned short Tile;      ///< graphic tile number
	unsigned short SeenTile;  ///< last seen tile (FOW)
	unsigned short Flags;     ///< field flags
	// FIXME: Value can be removed, walls and regeneration can be handled
	//        different.
	unsigned char Value;               ///< HP for walls/ Wood Regeneration
	unsigned char Visible[PlayerMax];  ///< Seen counter 0 unexplored
	unsigned char VisCloak[PlayerMax]; ///< Visiblity for cloaking.
#ifndef NEW_UNIT_CACHE
	Unit*      UnitCache;  ///< An unit on the map field
#else
	UnitListItem*      UnitCache;  ///< An unit on the map field
#endif
} MapField;

// Not used until now:
#if 0
#define MapFieldArray 0x0004  ///< More than one unit on the field
#endif

#define MapFieldHuman 0x0008  ///< Human is owner of the field (walls)

#define MapFieldLandAllowed  0x0010  ///< Land units allowed
#define MapFieldCoastAllowed 0x0020  ///< Coast (transporter) units allowed
#define MapFieldWaterAllowed 0x0040  ///< Water units allowed
#define MapFieldNoBuilding   0x0080  ///< No buildings allowed

#define MapFieldUnpassable 0x0100  ///< Field is movement blocked
#define MapFieldWall       0x0200  ///< Field contains wall
#define MapFieldRocks      0x0400  ///< Field contains rocks
#define MapFieldForest     0x0800  ///< Field contains forest

#define MapFieldLandUnit 0x1000  ///< Land unit on field
#define MapFieldAirUnit  0x2000  ///< Air unit on field
#define MapFieldSeaUnit  0x4000  ///< Water unit on field
#define MapFieldBuilding 0x8000  ///< Building on field

/*----------------------------------------------------------------------------
--  Map info structure
----------------------------------------------------------------------------*/

/**
**  Get info about a map.
*/
typedef struct _map_info_ {
	char*  Description;     ///< Map description
	char*  MapTerrainName;  ///< Map terrain name
	char*  Filename;        ///< Map filename
	// TODO: Map Terrain Nr. should be removed.
	int MapTerrain;  ///< Map terrain
	int MapWidth;    ///< Map width
	int MapHeight;   ///< Map height
	int PlayerType[PlayerMax];  ///< Same player->Type
	int PlayerSide[PlayerMax];  ///< Same player->Side
	int PlayerResources[PlayerMax][MaxCosts];  ///< Same player->Gold
	int PlayerAi[PlayerMax];  ///< Same player->Ai
	unsigned int MapUID;  ///< Unique Map ID (hash)
} MapInfo;

/*----------------------------------------------------------------------------
--  Map itself
----------------------------------------------------------------------------*/

	/// Describes the wold map
typedef struct _world_map_ {
	int Width;   ///< the map width
	int Height;  ///< the map height

	MapField* Fields;              ///< fields on map
	unsigned* Visible[PlayerMax];  ///< visible bit-field

	unsigned char NoFogOfWar;  ///< fog of war disabled

	char* TerrainName;  ///< terrain as name
	// TODO: terrain nr. should be removed?
	int      Terrain; ///< terrain type (summer,winter,...)
	Tileset* Tileset; ///< tileset data

	unsigned TileCount; ///< how many tiles, (== TileGraphic->NFrames)
	Graphic* TileGraphic; ///< graphic for all the tiles

	char Description[32];///< map description short

	MapInfo* Info;  ///< descriptive information
	// TODO: (MapInfo* Info DUPLICATES!)
} WorldMap;

/*----------------------------------------------------------------------------
--  Variables
----------------------------------------------------------------------------*/

extern WorldMap TheMap;  ///< The current map

	/// Fast draw tile, display and video mode independ
extern void VideoDrawTile(const int, int, int);
	/// Draws tiles display and video mode independ
extern void MapDrawTile(int, int, int);
	/// Vision Table to see where to locate goals and vision
extern unsigned char *VisionTable[3];
	/// Companion table for fast lookups
extern int *VisionLookup;

	/// Contrast of fog of war
extern int FogOfWarOpacity;
	/// Forest regeneration
extern int ForestRegeneration;
	/// Flag must reveal the map
extern int FlagRevealMap;
	/// Flag must reveal map when in replay
extern int ReplayRevealMap;

/*----------------------------------------------------------------------------
--  Functions
----------------------------------------------------------------------------*/

//
// in map_draw.c
//
#if 0
	/// Fast draw 32x32 tile for 32 bpp video modes
extern void VideoDraw32Tile32(const unsigned char* data,int x,int y);
	/// Fast draw 32x32 tile for 16 bpp video modes
extern void VideoDraw16Tile32(const unsigned char* data,int x,int y);
	/// Fast draw 32x32 tile for  8 bpp video modes
extern void VideoDraw8Tile32(const unsigned char* data,int x,int y);
#endif

	/// Called when the color cycles
extern void MapColorCycle(void);

	/// Draw the map background
extern void DrawMapBackgroundInViewport(const Viewport*, int x, int y);
	/// Build tables for map
extern void InitMap(void);

	/// Denote wether area in map is overlapping with viewport on screen
extern int MapAreaVisibleInViewport(const Viewport*, int , int , int , int);
	/// Check if any part of an area is visible in viewport
extern int AnyMapAreaVisibleInViewport(const Viewport*, int , int , int , int);

//
// in map_fog.c
//
	/// Filter map flags through fog
extern int MapFogFilterFlags(Player* player, int x, int y, int mask);
	/// Mark a tile for normal sight
extern void MapMarkTileSight(const Player* player, int x, int y);
	/// Unmark a tile for normal sight
extern void MapUnmarkTileSight(const Player* player, int x, int y);
	/// Mark a tile for cloak detection
extern void MapMarkTileDetectCloak(const Player* player,int x,int y);
	/// Unmark a tile for cloak detection
extern void MapUnmarkTileDetectCloak(const Player* player,int x,int y);

	/// Mark sight changes
extern void MapSight(const Player* player, int x, int y, int w, int h, int range, void (*marker)(const Player*, int, int));
	/// Find if a tile is visible (With shared vision)
extern unsigned char IsTileVisible(const Player* player, int x, int y);
	/// Mark tiles with fog of war to be redrawn
extern void MapUpdateFogOfWar(int x, int y);
	/// Update fog of war
extern void UpdateFogOfWarChange(void);

	/// Draw the map fog of war
extern void DrawMapFogOfWar(Viewport* vp, int x, int y);
	/// Build tables for fog of war
extern void InitMapFogOfWar(void);
	/// Cleanup memory for fog of war tables
extern void CleanMapFogOfWar(void);
	/// Builds Vision and Goal Tables
extern void InitVisionTable(void);
	/// Cleans up Vision and Goal Tables
extern void FreeVisionTable(void);

//
// in map_wall.c
//
	/// Check if the seen tile-type is wall
extern int MapIsSeenTileWall(int x, int y,int walltype);
	/// Correct the seen wall field, depending on the surrounding
extern void MapFixSeenWallTile(int x, int y);
	/// Correct the surrounding seen wall fields
extern void MapFixSeenWallNeighbors(int x, int y);
	/// Correct the real wall field, depending on the surrounding
extern void MapFixWallTile(int x, int y);
	/// Remove wall on tile
extern void MapRemoveWall(unsigned x,unsigned y);
	/// Wall is hit
extern void HitWall(unsigned x,unsigned y,unsigned damage);

//
// in map_wood.c
//
	/// Check if the seen tile-type is wood
extern int MapIsSeenTileWood(int x, int y);
	/// Correct the seen wood field, depending on the surrounding
extern void MapFixSeenWoodTile(int x, int y);
	/// Correct the surrounding seen wood fields
extern void MapFixSeenWoodNeighbors(int x, int y);
	/// Correct the real wood field, depending on the surrounding
extern void MapFixWoodTile(int x, int y);
	/// Remove wood from the map
extern void MapRemoveWood(unsigned x, unsigned y);
	/// Regenerate the forest
extern void RegenerateForest(void);

//
// in map_rock.c
//
	/// Check if the seen tile-type is rock
extern int MapIsSeenTileRock(int x, int y);
	/// Correct the seen rock field, depending on the surrounding
extern void MapFixSeenRockTile(int x, int y);
	/// Correct the surrounding seen rock fields
extern void MapFixSeenRockNeighbors(int x, int y);
	/// Correct the real rock field, depending on the surrounding
extern void MapFixRockTile(int x, int y);
	/// Remove rock from the map
extern void MapRemoveRock(unsigned x,unsigned y);

//
// in ccl_map.c
//
	/// register ccl features
extern void MapCclRegister(void);

//
// mixed sources
//
	/// Load a map
#if 0
extern void LoadMap(const char* file, WorldMap* map);
#endif
	/// Save the map
extern void SaveMap(CLFile* file);
	/// Clean the map
extern void CleanMap(void);

	/// Release info for a map
extern void FreeMapInfo(MapInfo* info);

	/// Mark a tile as seen by the player
extern void MapMarkSeenTile(int x, int y);
	/// Reveal the complete map, make everything known
extern void RevealMap(void);

	/// Returns true, if the tile field is empty
extern int IsMapFieldEmpty(int x, int y);
	/// Returns true, if water on the map tile field
extern int WaterOnMap(int x, int y);
	/// Returns true, if coast on the map tile field
extern int CoastOnMap(int x, int y);

	/// Returns true, if wall on the map tile field
extern int WallOnMap(int x, int y);
	/// Returns true, if human wall on the map tile field
extern int HumanWallOnMap(int x, int y);
	/// Returns true, if orc wall on the map tile field
extern int OrcWallOnMap(int x, int y);

	/// Returns true, if forest on the map tile field with bounds check
extern int CheckedForestOnMap(int x, int y);
	/// Returns true, if forest on the map tile field
extern int ForestOnMap(int x, int y);

	/// Returns true, if rock on the map tile field
extern int RockOnMap(int x, int y);

	/// Returns true, if the unit-type(mask can enter field with bounds check
extern int CheckedCanMoveToMask(int x, int y, int mask);
	/// Returns true, if the unit-type can enter the field
extern int UnitTypeCanMoveTo(int x, int y, const UnitType* type);
	/// Returns true, if the unit can enter the field
extern int UnitCanMoveTo(int x, int y, const Unit* unit);

	/// Preprocess map, for internal use.
extern void PreprocessMap(void);

	/// Set wall on field
extern void MapSetWall(unsigned x, unsigned y, int humanwall);

/*----------------------------------------------------------------------------
--  Defines
----------------------------------------------------------------------------*/

	/// Can a unit with 'mask' enter the field
#define CanMoveToMask(x, y, mask) \
	!(TheMap.Fields[(x) + (y) * TheMap.Width].Flags & (mask))

#define MapMarkSight(player,x,y,w,h,range) MapSight((player),(x),(y),(w),(h),(range),MapMarkTileSight)
#define MapUnmarkSight(player,x,y,w,h,range) MapSight((player),(x),(y),(w),(h),(range),MapUnmarkTileSight)

#define MapMarkUnitSight(unit) \
{ \
	MapSight((unit)->Player, (unit)->X,(unit)->Y, (unit)->Type->TileWidth,\
			(unit)->Type->TileHeight, (unit)->CurrentSightRange, MapMarkTileSight); \
	if (unit->Type->DetectCloak) { \
		MapSight((unit)->Player, (unit)->X,(unit)->Y, (unit)->Type->TileWidth,\
				(unit)->Type->TileHeight, (unit)->CurrentSightRange, MapMarkTileDetectCloak); \
	}\
}

#define MapUnmarkUnitSight(unit) \
{ \
	MapSight((unit)->Player,(unit)->X,(unit)->Y, (unit)->Type->TileWidth,\
			(unit)->Type->TileHeight,(unit)->CurrentSightRange,MapUnmarkTileSight); \
	if (unit->Type->DetectCloak) { \
		MapSight((unit)->Player, (unit)->X,(unit)->Y, (unit)->Type->TileWidth,\
				(unit)->Type->TileHeight, (unit)->CurrentSightRange, MapUnmarkTileDetectCloak); \
	}\
}

#define MapMarkUnitOnBoardSight(unit,host) MapSight((unit)->Player,(host)->X,(host)->Y, \
	(host)->Type->TileWidth,(host)->Type->TileHeight,(unit)->CurrentSightRange,MapMarkTileSight)
#define MapUnmarkUnitOnBoardSight(unit,host) MapSight((unit)->Player,(host)->X,(host)->Y, \
	(host)->Type->TileWidth,(host)->Type->TileHeight,(unit)->CurrentSightRange,MapUnmarkTileSight)
	/// Check if a field for the user is explored
#define IsMapFieldExplored(player,x,y) \
	(IsTileVisible((player),(x),(y)))

	/// Check if a field for the user is visibile
#define IsMapFieldVisible(player,x,y) \
	(IsTileVisible((player),(x),(y))>1)

//@}

#endif // !__MAP_H__
