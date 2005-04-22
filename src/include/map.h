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
//      (c) Copyright 1998-2005 by Vladi Shabanski, Lutz Sammer,
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
**  WorldMap::Tiles
**
**    Pointer into the tile graphic data. Used to find fast the start
**    of different tiles.
**
**  WorldMap::TileData
**
**    Tiles graphic for the map, loaded from WorldMap::Tileset::File.
**
**  WorldMap::Info
**
**    Descriptive information of the map.
**    @see ::_map_info_
**    @todo This structure contains duplicate informations of the map.
*/

/*----------------------------------------------------------------------------
--  Declarations
----------------------------------------------------------------------------*/

#include "upgrade_structs.h"

/*----------------------------------------------------------------------------
--  Declarations
----------------------------------------------------------------------------*/

struct _tileset_;
struct _graphic_;
struct _player_;
struct _CL_File_;
struct _unit_;
struct _unit_type_;
struct _unit_list_item_;

/*----------------------------------------------------------------------------
--  Map
----------------------------------------------------------------------------*/

// JOHNS: only limited by computer memory
//            OLD    NEW Code
// 512x512:     2 MB   3 MB
// 1024x1024:   8 MB  12 MB
// 2048*2048:  32 MB  48 MB
// 4096*4096: 128 MB 192 MB
#define MaxMapWidth  256  ///  maximal map width supported
#define MaxMapHeight 256  /// maximal map height supported

/*----------------------------------------------------------------------------
--  Map - field
----------------------------------------------------------------------------*/

	/// Describes a field of the map
typedef struct _map_field_ {
	unsigned short Tile;      /// graphic tile number
	unsigned short SeenTile;  /// last seen tile (FOW)
	unsigned short Flags;     /// field flags
	// FIXME: Value can be removed, walls and regeneration can be handled
	//        different.
	unsigned char Value;               /// HP for walls/ Wood Regeneration
	unsigned char Visible[PlayerMax];  /// Seen counter 0 unexplored
	unsigned char VisCloak[PlayerMax]; /// Visiblity for cloaking.
	unsigned char Radar[PlayerMax];    /// Visiblity for radar.
	unsigned char RadarJammer[PlayerMax]; /// Jamming Capabilities.
	struct _unit_list_item_* UnitCache;/// A unit on the map field
} MapField;

// Not used until now:
#if 0
#define MapFieldArray 0x0004  /// More than one unit on the field
#endif

#define MapFieldHuman 0x0008  /// Human is owner of the field (walls)

#define MapFieldLandAllowed  0x0010  /// Land units allowed
#define MapFieldCoastAllowed 0x0020  /// Coast (transporter) units allowed
#define MapFieldWaterAllowed 0x0040  /// Water units allowed
#define MapFieldNoBuilding   0x0080  /// No buildings allowed

#define MapFieldUnpassable 0x0100  /// Field is movement blocked
#define MapFieldWall       0x0200  /// Field contains wall
#define MapFieldRocks      0x0400  /// Field contains rocks
#define MapFieldForest     0x0800  /// Field contains forest

#define MapFieldLandUnit 0x1000  /// Land unit on field
#define MapFieldAirUnit  0x2000  /// Air unit on field
#define MapFieldSeaUnit  0x4000  /// Water unit on field
#define MapFieldBuilding 0x8000  /// Building on field

/*----------------------------------------------------------------------------
--  Map info structure
----------------------------------------------------------------------------*/

/**
**  Get info about a map.
*/
typedef struct _map_info_ {
	char*  Description;     /// Map description
	char*  MapTerrainName;  /// Map terrain name
	char*  Filename;        /// Map filename
	// TODO: Map Terrain Nr. should be removed.
	int MapTerrain;  /// Map terrain
	int MapWidth;    /// Map width
	int MapHeight;   /// Map height
	int PlayerType[PlayerMax];  /// Same player->Type
	int PlayerSide[PlayerMax];  /// Same player->Side
	int PlayerResources[PlayerMax][MaxCosts];  /// Same player->Gold
	int PlayerAi[PlayerMax];  /// Same player->Ai
	unsigned int MapUID;  /// Unique Map ID (hash)
} MapInfo;

/*----------------------------------------------------------------------------
--  Map itself
----------------------------------------------------------------------------*/

	/// Describes the wold map
typedef struct _world_map_ {
	MapField* Fields;              /// fields on map
	unsigned* Visible[PlayerMax];  /// visible bit-field

	unsigned char NoFogOfWar;  /// fog of war disabled

	char* TerrainName;         /// terrain as name
	// TODO: terrain nr. should be removed?
	int      Terrain;          /// terrain type (summer,winter,...)
	struct _tileset_* Tileset; /// tileset data

	struct _graphic_* TileGraphic; /// graphic for all the tiles
	struct _graphic_* FogGraphic; /// graphic for fog of war

	MapInfo Info;  /// descriptive information
} WorldMap;

/*----------------------------------------------------------------------------
--  Variables
----------------------------------------------------------------------------*/

extern WorldMap TheMap;  /// The current map

	/// Vision Table to see where to locate goals and vision
extern unsigned char* VisionTable[3];
	/// Companion table for fast lookups
extern int* VisionLookup;

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
	/// Called when the color cycles
extern void MapColorCycle(void);

	/// Draw the map background
extern void DrawMapBackgroundInViewport(const Viewport* vp);
	/// Build tables for map
extern void InitMap(void);

	/// Denote wether area in map is overlapping with viewport on screen
extern int MapAreaVisibleInViewport(const Viewport* vp, int sx, int sy,
	int ex, int ey);
	/// Check if any part of an area is visible in viewport
extern int AnyMapAreaVisibleInViewport(const Viewport* vp, int sx, int sy,
	int ex, int ey);

//
// in map_fog.c
//
/// Function to (un)mark the vision table.
typedef void MapMarkerFunc(const struct _player_* player, int x, int y);

	/// Filter map flags through fog
extern int MapFogFilterFlags(struct _player_* player, int x, int y, int mask);
	/// Mark a tile for normal sight
extern MapMarkerFunc MapMarkTileSight;
	/// Unmark a tile for normal sight
extern MapMarkerFunc MapUnmarkTileSight;
	/// Mark a tile for cloak detection
extern MapMarkerFunc MapMarkTileDetectCloak;
	/// Unmark a tile for cloak detection
extern MapMarkerFunc MapUnmarkTileDetectCloak;

	/// Mark sight changes
extern void MapSight(const struct _player_* player, int x, int y, int w,
	int h, int range, MapMarkerFunc* marker);
	/// Find if a tile is visible (With shared vision)
extern unsigned char IsTileVisible(const struct _player_* player, int x,
	int y);
	/// Mark tiles with fog of war to be redrawn
extern void MapUpdateFogOfWar(int x, int y);
	/// Update fog of war
extern void UpdateFogOfWarChange(void);

	/// Draw the map fog of war
extern void DrawMapFogOfWar(Viewport* vp);
	/// Build tables for fog of war
extern void InitMapFogOfWar(void);
	/// Cleanup memory for fog of war tables
extern void CleanMapFogOfWar(void);
	/// Builds Vision and Goal Tables
extern void InitVisionTable(void);
	/// Cleans up Vision and Goal Tables
extern void FreeVisionTable(void);

//
// in map_radar.c
//

	/// Check if a unit is visible on radar
extern unsigned char UnitVisibleOnRadar(const struct _player_* pradar, const struct _unit_* punit);
	/// Check if a tile is visible on radar
extern unsigned char IsTileRadarVisible(const struct _player_* pradar, const struct _player_* punit, int x, int y);
	/// Mark a tile as radar visible, or incrase radar vision
extern void MapMarkTileRadar(const struct _player_* player, int x, int y);
	/// Unmark a tile as radar visible, decrease is visible by other radar
extern void MapUnmarkTileRadar(const struct _player_* player, int x, int y);
	/// Mark a tile as radar jammed, or incrase radar jamming'ness
extern void MapMarkTileRadarJammer(const struct _player_* player, int x, int y);
	/// Unmark a tile as jammed, decrease is jamming'ness
extern void MapUnmarkTileRadarJammer(const struct _player_* player, int x, int y);

//
// in map_wall.c
//
	/// Correct the seen wall field, depending on the surrounding
extern void MapFixSeenWallTile(int x, int y);
	/// Correct the surrounding seen wall fields
extern void MapFixSeenWallNeighbors(int x, int y);
	/// Correct the real wall field, depending on the surrounding
extern void MapFixWallTile(int x, int y);
	/// Remove wall on tile
extern void MapRemoveWall(unsigned x, unsigned y);
	/// Wall is hit
extern void HitWall(unsigned x, unsigned y, unsigned damage);


//
// in ccl_map.c
//
	/// register ccl features
extern void MapCclRegister(void);

//
// mixed sources
//
	/// Alocate and initialise map table
extern void CreateMap(int width, int height);
	/// Save the map
extern void SaveMap(struct _CL_File_* file);
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
extern int UnitTypeCanBeAt(const struct _unit_type_* type, int x, int y);
	/// Returns true, if the unit can enter the field
extern int UnitCanBeAt(const struct _unit_* unit, int x, int y);

	/// Preprocess map, for internal use.
extern void PreprocessMap(void);

	/// Set wall on field
extern void MapSetWall(unsigned x, unsigned y, int humanwall);

	/// Check if the seen tile-type is wood
extern int MapIsSeenTile(unsigned short type, int x, int y);
	/// Correct the seen wood field, depending on the surrounding
extern void MapFixTile(unsigned short type, int seen, int x, int y);
	/// Correct the surrounding seen wood fields
extern void MapFixNeighbors(unsigned short type, int seen, int x, int y);
	/// Remove wood from the map
extern void MapClearTile(unsigned short type, unsigned x, unsigned y);
	/// Regenerate the forest
extern void RegenerateForest(void);

// in unit.c

/// Mark on vision table the Sight of the unit.
void MapMarkUnitSight(struct _unit_* unit);
/// Unmark on vision table the Sight of the unit.
void MapUnmarkUnitSight(struct _unit_* unit);

/*----------------------------------------------------------------------------
--  Defines
----------------------------------------------------------------------------*/

	/// Can a unit with 'mask' enter the field
#define CanMoveToMask(x, y, mask) \
	!(TheMap.Fields[(x) + (y) * TheMap.Info.MapWidth].Flags & (mask))

#define MapMarkSight(player, x, y, w, h, range) \
	MapSight((player), (x), (y), (w), (h), (range), MapMarkTileSight)
#define MapUnmarkSight(player, x, y, w, h, range) \
	MapSight((player), (x), (y), (w), (h), (range), MapUnmarkTileSight)

	/// Handle Marking and Unmarking of radar vision
#define MapMarkRadar(player, x, y, w, h, range) \
	MapSight((player), (x), (y), (w), (h), (range), MapMarkTileRadar)
#define MapUnmarkRadar(player, x, y, w, h, range) \
	MapSight((player), (x), (y), (w), (h), (range), MapUnmarkTileRadar)
	/// Handle Marking and Unmarking of radar vision
#define MapMarkRadarJammer(player, x, y, w, h, range) \
	MapSight((player), (x), (y), (w), (h), (range), MapMarkTileRadarJammer)
#define MapUnmarkRadarJammer(player, x, y, w, h, range) \
	MapSight((player), (x), (y), (w), (h), (range), MapUnmarkTileRadarJammer)

	/// Check if a field for the user is explored
#define IsMapFieldExplored(player, x, y) \
	(IsTileVisible((player), (x), (y)))

	/// Check if a field for the user is visibile
#define IsMapFieldVisible(player, x, y) \
	(IsTileVisible((player), (x), (y)) > 1)

//@}

#endif // !__MAP_H__
