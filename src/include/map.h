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

#ifndef __MAP_H__
#define __MAP_H__

//@{

/*----------------------------------------------------------------------------
--  Documentation
----------------------------------------------------------------------------*/

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
**  CMap::TileModelsFileName
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

#include <string>

#ifndef __MAP_TILE_H__
#include "tile.h"
#endif

#include "color.h"
#include "vec2i.h"

/*----------------------------------------------------------------------------
--  Declarations
----------------------------------------------------------------------------*/

class CGraphic;
class CPlayer;
class CFile;
class CTileset;
class CUnit;
class CUnitType;

/*----------------------------------------------------------------------------
--  Map
----------------------------------------------------------------------------*/

#define MaxMapWidth  256  /// max map width supported
#define MaxMapHeight 256  /// max map height supported

/*----------------------------------------------------------------------------
--  Map info structure
----------------------------------------------------------------------------*/

/**
**  Get info about a map.
*/
class CMapInfo
{
public:
	bool IsPointOnMap(int x, int y) const {
		return (x >= 0 && y >= 0 && x < MapWidth && y < MapHeight);
	}

	bool IsPointOnMap(const Vec2i &pos) const {
		return IsPointOnMap(pos.x, pos.y);
	}

	void Clear();

public:
	std::string Description;    /// Map description
	std::string Filename;       /// Map filename
	int MapWidth;               /// Map width
	int MapHeight;              /// Map height
	int PlayerType[PlayerMax];  /// Same player->Type
	int PlayerSide[PlayerMax];  /// Same player->Side
	unsigned int MapUID;        /// Unique Map ID (hash)
};

/*----------------------------------------------------------------------------
--  Map itself
----------------------------------------------------------------------------*/

/// Describes the world map
class CMap
{
public:
	CMap();
	~CMap();

	unsigned int getIndex(int x, int y) const {
		return x + y * this->Info.MapWidth;
	}
	unsigned int getIndex(const Vec2i &pos) const {
		return getIndex(pos.x, pos.y);
	}

	CMapField *Field(unsigned int index) const {
		return &this->Fields[index];
	}
	/// Get the MapField at location x,y
	CMapField *Field(int x, int y) const {
		return &this->Fields[x + y * this->Info.MapWidth];
	}
	CMapField *Field(const Vec2i &pos) const {
		return Field(pos.x, pos.y);
	}

	/// Alocate and initialise map table.
	void Create();
	/// Build tables for map
	void Init();
	/// Clean the map
	void Clean();
	/// Cleanup memory for fog of war tables
	void CleanFogOfWar();

	/// Remove wood from the map.
	void ClearWoodTile(const Vec2i &pos);
	/// Remove rock from the map.
	void ClearRockTile(const Vec2i &pos);

	/// convert map pixelpos coordonates into tilepos
	Vec2i MapPixelPosToTilePos(const PixelPos &mapPos) const;
	/// convert tilepos coordonates into map pixel pos (take the top left of the tile)
	PixelPos TilePosToMapPixelPos_TopLeft(const Vec2i &tilePos) const;
	/// convert tilepos coordonates into map pixel pos (take the center of the tile)
	PixelPos TilePosToMapPixelPos_Center(const Vec2i &tilePos) const;

	/// Mark a tile as seen by the player.
	void MarkSeenTile(CMapField &mf);

	/// Regenerate the forest.
	void RegenerateForest();
	/// Reveal the complete map, make everything known.
	void Reveal();
	/// Save the map.
	void Save(CFile &file) const;

	//
	// Wall
	//
	/// Wall is hit.
	void HitWall(const Vec2i &pos, unsigned damage);
	/// Set wall on field.
	void RemoveWall(const Vec2i &pos);
	/// Set wall on field.
	void SetWall(const Vec2i &pos, bool humanwall);

	/// Returns true, if wall on the map tile field
	bool WallOnMap(const Vec2i &pos) const;
	/// Returns true, if human wall on the map tile field
	bool HumanWallOnMap(const Vec2i &pos) const;
	/// Returns true, if orc wall on the map tile field
	bool OrcWallOnMap(const Vec2i &pos) const;

	//UnitCache

	/// Insert new unit into cache
	void Insert(CUnit &unit);

	/// Remove unit from cache
	void Remove(CUnit &unit);

	void Clamp(Vec2i &pos) const;

	//Warning: we expect typical usage as xmin = x - range
	void FixSelectionArea(Vec2i &minpos, Vec2i &maxpos) {
		minpos.x = std::max<short>(0, minpos.x);
		minpos.y = std::max<short>(0, minpos.y);

		maxpos.x = std::min<short>(maxpos.x, Info.MapWidth - 1);
		maxpos.y = std::min<short>(maxpos.y, Info.MapHeight - 1);
	}

private:
	/// Build tables for fog of war
	void InitFogOfWar();

	/// Correct the surrounding seen wood fields
	void FixNeighbors(unsigned short type, int seen, const Vec2i &pos);
	/// Correct the seen wood field, depending on the surrounding
	void FixTile(unsigned short type, int seen, const Vec2i &pos);

	/// Regenerate the forest.
	void RegenerateForestTile(const Vec2i &pos);

public:
	CMapField *Fields;              /// fields on map
	bool NoFogOfWar;           /// fog of war disabled

	CTileset *Tileset;          /// tileset data
	std::string TileModelsFileName; /// lua filename that loads all tilemodels
	CGraphic *TileGraphic;     /// graphic for all the tiles
	static CGraphic *FogGraphic;      /// graphic for fog of war

	CMapInfo Info;             /// descriptive information
};


/*----------------------------------------------------------------------------
--  Variables
----------------------------------------------------------------------------*/

extern CMap Map;  /// The current map
extern char CurrentMapPath[1024]; /// Path to the current map

/// Contrast of fog of war
extern int FogOfWarOpacity;
/// fog of war color
extern CColor FogOfWarColor;
/// Forest regeneration
extern int ForestRegeneration;
/// Flag must reveal the map
extern int FlagRevealMap;
/// Flag must reveal map when in replay
extern int ReplayRevealMap;

/*----------------------------------------------------------------------------
--  Functions
----------------------------------------------------------------------------*/
#define MARKER_ON_INDEX
//
// in map_fog.c
//
/// Function to (un)mark the vision table.
#ifndef MARKER_ON_INDEX
typedef void MapMarkerFunc(const CPlayer &player, const Vec2i &pos);
#else
typedef void MapMarkerFunc(const CPlayer &player, const unsigned int index);
#endif

/// Filter map flags through fog
extern int MapFogFilterFlags(CPlayer &player, const Vec2i &pos, int mask);
extern int MapFogFilterFlags(CPlayer &player, const unsigned int index, int mask);
/// Mark a tile for normal sight
extern MapMarkerFunc MapMarkTileSight;
/// Unmark a tile for normal sight
extern MapMarkerFunc MapUnmarkTileSight;
/// Mark a tile for cloak detection
extern MapMarkerFunc MapMarkTileDetectCloak;
/// Unmark a tile for cloak detection
extern MapMarkerFunc MapUnmarkTileDetectCloak;

/// Mark sight changes
extern void MapSight(const CPlayer &player, const Vec2i &pos, int w,
					 int h, int range, MapMarkerFunc *marker);
/// Update fog of war
extern void UpdateFogOfWarChange();

//
// in map_radar.c
//

/// Mark a tile as radar visible, or incrase radar vision
extern MapMarkerFunc MapMarkTileRadar;

/// Unmark a tile as radar visible, decrease is visible by other radar
extern MapMarkerFunc MapUnmarkTileRadar;

/// Mark a tile as radar jammed, or incrase radar jamming'ness
extern MapMarkerFunc MapMarkTileRadarJammer;

/// Unmark a tile as jammed, decrease is jamming'ness
extern MapMarkerFunc MapUnmarkTileRadarJammer;


//
// in map_wall.c
//
/// Correct the seen wall field, depending on the surrounding
extern void MapFixSeenWallTile(const Vec2i &pos);
/// Correct the surrounding seen wall fields
extern void MapFixSeenWallNeighbors(const Vec2i &pos);
/// Correct the real wall field, depending on the surrounding
extern void MapFixWallTile(const Vec2i &pos);

//
// in script_map.cpp
//
/// Set a tile
extern void SetTile(unsigned int tile, const Vec2i &pos, int value = 0);
inline void SetTile(unsigned int tile, int x, int y, int value = 0)
{
	const Vec2i pos(x, y);
	SetTile(tile, pos, value);
}

/// register ccl features
extern void MapCclRegister();

//
// mixed sources
//
/// Save a stratagus map (smp format)
extern int SaveStratagusMap(const std::string &filename, CMap &map, int writeTerrain);


/// Load map presentation
extern void LoadStratagusMapInfo(const std::string &mapname);

/// Returns true, if the unit-type(mask can enter field with bounds check
extern bool CheckedCanMoveToMask(const Vec2i &pos, int mask);
/// Returns true, if the unit-type can enter the field
extern bool UnitTypeCanBeAt(const CUnitType &type, const Vec2i &pos);
/// Returns true, if the unit can enter the field
extern bool UnitCanBeAt(const CUnit &unit, const Vec2i &pos);

/// Preprocess map, for internal use.
extern void PreprocessMap();

// in unit.c

/// Mark on vision table the Sight of the unit.
void MapMarkUnitSight(CUnit &unit);
/// Unmark on vision table the Sight of the unit.
void MapUnmarkUnitSight(CUnit &unit);

/*----------------------------------------------------------------------------
--  Defines
----------------------------------------------------------------------------*/

/// Can a unit with 'mask' enter the field
inline bool CanMoveToMask(const Vec2i &pos, int mask)
{
	return !Map.Field(pos)->CheckMask(mask);
}

/// Handle Marking and Unmarking of radar vision
inline void MapMarkRadar(const CPlayer &player, const Vec2i &pos, int w, int h, int range)
{
	MapSight(player, pos, w, h, range, MapMarkTileRadar);
}
inline void MapUnmarkRadar(const CPlayer &player, const Vec2i &pos, int w, int h, int range)
{
	MapSight(player, pos, w, h, range, MapUnmarkTileRadar);
}
/// Handle Marking and Unmarking of radar vision
inline void MapMarkRadarJammer(const CPlayer &player, const Vec2i &pos, int w, int h, int range)
{
	MapSight(player, pos, w, h, range, MapMarkTileRadarJammer);
}
inline void MapUnmarkRadarJammer(const CPlayer &player, const Vec2i &pos, int w, int h, int range)
{
	MapSight(player, pos, w, h, range, MapUnmarkTileRadarJammer);
}

//@}

#endif // !__MAP_H__
