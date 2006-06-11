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
//      (c) Copyright 1998-2006 by Vladi Shabanski, Lutz Sammer, and
//                                 Jimmy Salmon
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
**  @class CMapField map.h
**
**  \#include "map.h"
**
**  This class contains all information about a field on map.
**  It contains its look, properties and content.
**
**  The map-field class members:
**
**  CMapField::Tile
**
**    Tile is number defining the graphic image display for the
**    map-field. 65535 different tiles are supported. A tile is
**    currently 32x32 pixels. In the future is planned to support
**    animated tiles.
**
**  CMapField::SeenTile
**
**    This is the tile number, that the player sitting on the computer
**    currently knows. Idea: Can be uses for illusions.
**
**  CMapField::Flags
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
**  CMapField::Cost
**
**    Unit cost to move in this tile.
**
**  CMapField::Value
**
**    Extra value for each tile. This currently only used for
**    walls, contains the remaining hit points of the wall and
**    for forest, contains the frames until they grow.
**
**  CMapField::Visible[]
**
**    Counter how many units of the player can see this field. 0 the
**    field is not explored, 1 explored, n-1 unit see it. Currently
**    no more than 253 units can see a field.
**
**  CMapField::VisCloak[]
**
**    Visiblity for cloaking.
**
**  CMapField::Radar[]
**
**    Visiblity for radar.
**
**  CMapField::RadarJammer[]
**
**    Jamming capabilities.
**
**  CMapField::UnitCache
**
**    Contains a vector of all units currently on this field.
**    Note: currently units are only inserted at the insert point.
**    This means units of the size of 2x2 fields are inserted at the
**    top and right most map coordinate.
*/

/**
**  @class CMap map.h
**
**  \#include "map.h"
**
**  This class contains all information about a Stratagus map.
**  A map is a rectangle of any size.
**
**  The map class members:
**
**  CMap::Fields
**
**    An array CMap::Info::Width * CMap::Info::Height of all fields
**    belonging to this map.
**
**  CMap::NoFogOfWar
**
**    Flag if true, the fog of war is disabled.
**
**  CMap::Tileset
**
**    Tileset data for the map. See ::CTileset. This contains all
**    information about the tile.
**
**  CMap::TileModelsFileName[]
**
**    Lua filename that loads all tilemodels
**
**  CMap::TileGraphic
**
**    Graphic for all the tiles
**
**  CMap::FogGraphic
**
**    Graphic for fog of war
**
**  CMap::Info
**
**    Descriptive information of the map. See ::CMapInfo.
*/

/*----------------------------------------------------------------------------
--  Includes
----------------------------------------------------------------------------*/

#include "iocompat.h"
#include "tileset.h"
#include "upgrade_structs.h"

/*----------------------------------------------------------------------------
--  Declarations
----------------------------------------------------------------------------*/

class CGraphic;
class CPlayer;
class CFile;
class CUnit;
class CUnitType;

/*----------------------------------------------------------------------------
--  Map
----------------------------------------------------------------------------*/

#define MaxMapWidth  256  /// max map width supported
#define MaxMapHeight 256  /// max map height supported

/*----------------------------------------------------------------------------
--  Map - field
----------------------------------------------------------------------------*/

	/// Describes a field of the map
class CMapField {
public:
	CMapField() : Tile(0), SeenTile(0), Flags(0), Cost(0), Value(0),
		UnitCache()
	{
		memset(Visible, 0, sizeof(Visible));
		memset(VisCloak, 0, sizeof(VisCloak));
		memset(Radar, 0, sizeof(Radar));
		memset(RadarJammer, 0, sizeof(RadarJammer));
	}

	unsigned short Tile;      /// graphic tile number
	unsigned short SeenTile;  /// last seen tile (FOW)
	unsigned short Flags;     /// field flags
	unsigned char Cost;       /// unit cost to move in this tile
	// FIXME: Value can be removed, walls and regeneration can be handled
	//        different.
	unsigned char Value;                  /// HP for walls/ Wood Regeneration
	unsigned short Visible[PlayerMax];    /// Seen counter 0 unexplored
	unsigned char VisCloak[PlayerMax];    /// Visiblity for cloaking.
	unsigned char Radar[PlayerMax];       /// Visiblity for radar.
	unsigned char RadarJammer[PlayerMax]; /// Jamming capabilities.
	std::vector<CUnit *> UnitCache;       /// A unit on the map field.
};

// Not used until now:
#define MapFieldSpeedMask 0x0007  /// Move faster on this tile

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
class CMapInfo {
public:
	char *Description;     /// Map description
	char *Filename;        /// Map filename
	int MapWidth;          /// Map width
	int MapHeight;         /// Map height
	int PlayerType[PlayerMax];  /// Same player->Type
	int PlayerSide[PlayerMax];  /// Same player->Side
	unsigned int MapUID;   /// Unique Map ID (hash)
};

/*----------------------------------------------------------------------------
--  Map itself
----------------------------------------------------------------------------*/

	/// Describes the world map
class CMap {
public:

	/// Alocate and initialise map table.
	void Create();
	/// Build tables for map
	void Init(void);
	/// Clean the map
	void Clean();
	/// Cleanup memory for fog of war tables
	void CleanFogOfWar(void);
	/// Remove wood/rock from the map.
	void ClearTile(unsigned short type, unsigned x, unsigned y);

	/// Find if a tile is visible (with shared vision).
	unsigned short IsTileVisible(const CPlayer *player, int x, int y) const;

	/// Check if a field for the user is explored.
	bool IsFieldExplored(const CPlayer *player, int x, int y) const
	{
		return IsTileVisible(player, x, y) > 0;
	}
	/// Check if a field for the user is visible.
	bool IsFieldVisible(const CPlayer *player, int x, int y) const
	{
		return IsTileVisible(player, x, y) > 1;
	}
	/// Mark a tile as seen by the player.
	void MarkSeenTile(int x, int y);

	/// Reveal the complete map, make everything known.
	void Reveal(void);
	/// Save the map.
	void Save(CFile *file) const;

//
// Wall
//
	/// Wall is hit.
	void HitWall(unsigned x, unsigned y, unsigned damage);
	/// Set wall on field.
	void RemoveWall(unsigned x, unsigned y);
	/// Set wall on field.
	void SetWall(unsigned x, unsigned y, int humanwall);

	/// Returns true, if wall on the map tile field
	bool WallOnMap(int x, int y) const;
	/// Returns true, if human wall on the map tile field
	bool HumanWallOnMap(int x, int y) const;
	/// Returns true, if orc wall on the map tile field
	bool OrcWallOnMap(int x, int y) const;


//
//  Tile type.
//

	/// Returns true, if water on the map tile field
	bool WaterOnMap(int x, int y) const;
	/// Returns true, if coast on the map tile field
	bool CoastOnMap(int x, int y) const;

	/// Returns true, if forest on the map tile field
	bool ForestOnMap(int x, int y) const;

	/// Returns true, if rock on the map tile field
	bool RockOnMap(int x, int y) const;



private:
	/// Build tables for fog of war
	void InitFogOfWar(void);

	/// Check if the seen tile-type is wood
	bool IsSeenTile(unsigned short type, int x, int y) const;
	/// Correct the surrounding seen wood fields
	void FixNeighbors(unsigned short type, int seen, int x, int y);
	/// Correct the seen wood field, depending on the surrounding
	void FixTile(unsigned short type, int seen, int x, int y);

public:
	CMapField *Fields;              /// fields on map
	unsigned *Visible[PlayerMax];  /// visible bit-field

	bool NoFogOfWar;           /// fog of war disabled

	CTileset Tileset;          /// tileset data
	char TileModelsFileName[PATH_MAX]; /// lua filename that loads all tilemodels
	CGraphic *TileGraphic;     /// graphic for all the tiles
	static CGraphic *FogGraphic;      /// graphic for fog of war

	CMapInfo Info;             /// descriptive information
};

/*----------------------------------------------------------------------------
--  Variables
----------------------------------------------------------------------------*/

extern CMap Map;  /// The current map
extern char CurrentMapPath[]; /// Path to the current map

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
// in map_fog.c
//
/// Function to (un)mark the vision table.
typedef void MapMarkerFunc(const CPlayer *player, int x, int y);

	/// Filter map flags through fog
extern int MapFogFilterFlags(CPlayer *player, int x, int y, int mask);
	/// Mark a tile for normal sight
extern MapMarkerFunc MapMarkTileSight;
	/// Unmark a tile for normal sight
extern MapMarkerFunc MapUnmarkTileSight;
	/// Mark a tile for cloak detection
extern MapMarkerFunc MapMarkTileDetectCloak;
	/// Unmark a tile for cloak detection
extern MapMarkerFunc MapUnmarkTileDetectCloak;

	/// Mark sight changes
extern void MapSight(const CPlayer *player, int x, int y, int w,
	int h, int range, MapMarkerFunc *marker);
	/// Mark tiles with fog of war to be redrawn
extern void MapUpdateFogOfWar(int x, int y);
	/// Update fog of war
extern void UpdateFogOfWarChange(void);

	/// Builds Vision and Goal Tables
extern void InitVisionTable(void);
	/// Cleans up Vision and Goal Tables
extern void FreeVisionTable(void);

//
// in map_radar.c
//

	/// Check if a unit is visible on radar
extern bool UnitVisibleOnRadar(const CPlayer *pradar, const CUnit *punit);
	/// Check if a tile is visible on radar
extern unsigned char IsTileRadarVisible(const CPlayer *pradar, const CPlayer *punit, int x, int y);
	/// Mark a tile as radar visible, or incrase radar vision
extern void MapMarkTileRadar(const CPlayer *player, int x, int y);
	/// Unmark a tile as radar visible, decrease is visible by other radar
extern void MapUnmarkTileRadar(const CPlayer *player, int x, int y);
	/// Mark a tile as radar jammed, or incrase radar jamming'ness
extern void MapMarkTileRadarJammer(const CPlayer *player, int x, int y);
	/// Unmark a tile as jammed, decrease is jamming'ness
extern void MapUnmarkTileRadarJammer(const CPlayer *player, int x, int y);

//
// in map_wall.c
//
	/// Correct the seen wall field, depending on the surrounding
extern void MapFixSeenWallTile(int x, int y);
	/// Correct the surrounding seen wall fields
extern void MapFixSeenWallNeighbors(int x, int y);
	/// Correct the real wall field, depending on the surrounding
extern void MapFixWallTile(int x, int y);

//
// in ccl_map.c
//
	/// register ccl features
extern void MapCclRegister(void);

//
// mixed sources
//
	/// Save a stratagus map (smp format)
extern int SaveStratagusMap(const char *filename, CMap *map, int writeTerrain);


	/// Load map presentation
extern void LoadStratagusMapInfo(const char *mapname);
	/// Release info for a map
extern void FreeMapInfo(CMapInfo *info);

	/// Returns true, if the unit-type(mask can enter field with bounds check
extern int CheckedCanMoveToMask(int x, int y, int mask);
	/// Returns true, if the unit-type can enter the field
extern int UnitTypeCanBeAt(const CUnitType *type, int x, int y);
	/// Returns true, if the unit can enter the field
extern int UnitCanBeAt(const CUnit *unit, int x, int y);

	/// Preprocess map, for internal use.
extern void PreprocessMap(void);

// in unit.c

/// Mark on vision table the Sight of the unit.
void MapMarkUnitSight(CUnit *unit);
/// Unmark on vision table the Sight of the unit.
void MapUnmarkUnitSight(CUnit *unit);

/*----------------------------------------------------------------------------
--  Defines
----------------------------------------------------------------------------*/

	/// Can a unit with 'mask' enter the field
#define CanMoveToMask(x, y, mask) \
	!(Map.Fields[(x) + (y) * Map.Info.MapWidth].Flags & (mask))

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

//@}

#endif // !__MAP_H__
