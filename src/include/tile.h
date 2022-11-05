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
/**@name tile.h - The map tile headerfile. */
//
//      (c) Copyright 1998-2008 by Vladi Shabanski, Lutz Sammer,
//                                 Jimmy Salmon and Rafal Bursig
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

#ifndef __MAP_TILE_H__
#define __MAP_TILE_H__

//@{


/*----------------------------------------------------------------------------
--  Documentation
----------------------------------------------------------------------------*/

/**
**  @class CMapField tile.h
**
**  \#include "tile.h"
**
**  CMapFieldPlayerInfo::SeenTile
**
**    This is the tile number, that the player sitting on the computer
**    currently knows. Idea: Can be uses for illusions.
**
**  CMapFieldPlayerInfo::Visible[]
**
**    Counter how many units of the player can see this field. 0 the
**    field is not explored, 1 explored, n-1 unit see it. Currently
**    no more than 253 units can see a field.
**
**  CMapFieldPlayerInfo::VisCloak[]
**
**    Visiblity for cloaking.
**
**  CMapFieldPlayerInfo::Radar[]
**
**    Visiblity for radar.
**
**  CMapFieldPlayerInfo::RadarJammer[]
**
**    Jamming capabilities.
*/

/**
**  @class CMapField tile.h
**
**  \#include "tile.h"
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
**  CMapField::Flags
**
**    Contains special information of that tile. What units are
**    on this field, what units could be placed on this field.
**
**    This is the list of all flags currently used:
**
**    ::MapFieldVisible field is visible.
**    ::MapFieldExplored field is explored.
**    ::MapFieldHuman human player is the owner of the field used for walls.
**    ::MapFieldLandAllowed land units are allowed.
**    ::MapFieldCoastAllowed coast units (transporter) and coast buildings (shipyard) are allowed.
**    ::MapFieldWaterAllowed water units allowed.
**    ::MapFieldNoBuilding no buildings allowed.
**    ::MapFieldUnpassable field is movement blocked.
**    ::MapFieldWall field contains wall.
**    ::MapFieldRocks field contains rocks.
**    ::MapFieldForest field contains forest or other harvestable resource
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
**  CMapField::UnitCache
**
**    Contains a vector of all units currently on this field.
**    Note: currently units are only inserted at the insert point.
**    This means units of the size of 2x2 fields are inserted at the
**    top and right most map coordinate.
*/


/*----------------------------------------------------------------------------
--  Includes
----------------------------------------------------------------------------*/

#ifndef __UNIT_CACHE_H__
#include "unit_cache.h"
#endif

#include <tileset.h>
#include <vec2i.h>

class CFile;
class CPlayer;
class CTileset;
struct lua_State;

/*----------------------------------------------------------------------------
--  Map - field
----------------------------------------------------------------------------*/

class CMapFieldPlayerInfo
{
public:
	CMapFieldPlayerInfo() : SeenTile(0)
	{
		memset(Visible, 0, sizeof(Visible));
		memset(VisCloak, 0, sizeof(VisCloak));
		memset(Radar, 0, sizeof(Radar));
		memset(RadarJammer, 0, sizeof(RadarJammer));
	}

	/// Check if a field for the user is explored.
	bool IsExplored(const CPlayer &player) const;

	/// @note Manage Map.NoFogOfWar
	bool IsVisible(const CPlayer &player) const;
	bool IsTeamVisible(const CPlayer &player) const;
	/**
	**  Find out how a field is seen (By player, or by shared vision)
	**
	**  @param player   Player to check for.
	**  @note manage fogOfWar (using Map.NoFogOfWar)
	**
	**  @return        0 unexplored, 1 explored, 2 visible.
	*/
	unsigned char TeamVisibilityState(const CPlayer &player) const;

public:
	unsigned short SeenTile;              /// last seen tile (FOW)
	unsigned short Visible[PlayerMax];    /// Seen counter 0 unexplored
	unsigned char VisCloak[PlayerMax];    /// Visiblity for cloaking.
	unsigned char Radar[PlayerMax];       /// Visiblity for radar.
	unsigned char RadarJammer[PlayerMax]; /// Jamming capabilities.
};

/// Describes a field of the map
class CMapField
{
public:
	CMapField();

	void Save(CFile &file) const;
	void parse(lua_State *l);

	void setTileIndex(const CTileset &tileset, const tile_index tileIndex, const int value, const uint8_t elevation, const int subtile = -1);

	graphic_index getGraphicTile() const { return tile; }

	/// Check if a field is opaque for field of view
	bool isOpaque() const;

	/// Check if a field flags.
	bool CheckMask(int mask) const;

	/// Returns true, if water on the map tile field
	bool WaterOnMap() const;

	/// Returns true, if coast on the map tile field
	bool CoastOnMap() const;

	/// Returns true, if water on the map tile field
	bool ForestOnMap() const;

	/// Returns true, if any terrain resource giving Cost4 on the map tile field
	bool Cost4OnMap() const;

	/// Returns true, if any terrain resource giving Cost5 on the map tile field
	bool Cost5OnMap() const;

	/// Returns true, if any terrain resource giving Cost6 on the map tile field
	bool Cost6OnMap() const;

	/// Returns true, if coast on the map tile field
	bool RockOnMap() const;

	/// Returns true if the field should not need mixing with the surroundings
	bool isDecorative() const;

	bool isAWall() const;
	bool isHuman() const;
	bool isAHumanWall() const;
	bool isAOrcWall() const;

	bool IsTerrainResourceOnMap(int resource) const;
	bool IsTerrainResourceOnMap() const;

	unsigned char getCost() const { return cost; }
	unsigned int getFlag() const { return Flags; }
	void setGraphicTile(unsigned int tile) { this->tile = tile; }
#ifdef DEBUG
	int64_t lastAStarCost;    /// debugging pathfinder
#endif

	uint8_t getElevation() const { return this->ElevationLevel; }
	void 	setElevation(const uint8_t newLevel) { this->ElevationLevel = newLevel; }

private:
#ifdef DEBUG
	tile_index tilesetTile;  /// tileset tile number
#endif
	graphic_index tile;       /// graphic tile number
public:
	tile_flags Flags;        /// field flags
private:
	unsigned char cost;        /// unit cost to move in this tile
public:
	unsigned int Value;        /// HP for walls/Wood Regeneration, value of stored resource for forest or harvestable terrain
	CUnitCache UnitCache;      /// A unit on the map field.

	CMapFieldPlayerInfo playerInfo; /// stuff related to player

private:
	uint8_t ElevationLevel {0};		/// highground elevation level

};

extern PixelSize PixelTileSize; /// Size of a tile in pixels

//@}

#endif // !__MAP_TILE_H__

