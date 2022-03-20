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
/**@name mapfield.cpp - The map field. */
//
//      (c) Copyright 2013 by Joris Dauphin
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

//@{

/*----------------------------------------------------------------------------
--  Includes
----------------------------------------------------------------------------*/

#include "stratagus.h"

#include "tile.h"

#include "fov.h"
#include "iolib.h"
#include "map.h"
#include "player.h"
#include "script.h"
#include "tileset.h"
#include "unit.h"
#include "unit_manager.h"

CMapField::CMapField() :
#ifdef DEBUG
	tilesetTile(0),
#endif
	tile(0),
	Flags(0),
	cost(0),
	Value(0),
	UnitCache()
{}

bool CMapField::IsTerrainResourceOnMap(int resource) const
{
	switch (resource) {
		case WoodCost:
			return ForestOnMap();
		case Cost4:
			return Cost4OnMap();
		case Cost5:
			return Cost5OnMap();
		case Cost6:
			return Cost6OnMap();
	}
	return false;
}

bool CMapField::IsTerrainResourceOnMap() const
{
	for (int i = 0; i != MaxCosts; ++i) {
		if (IsTerrainResourceOnMap(i)) {
			return true;
		}
	}
	return false;
}

void CMapField::setTileIndex(const CTileset &tileset, unsigned int tileIndex, int value)
{
	const CTile &tile = tileset.tiles[tileIndex];
	this->tile = tile.tile;
	this->Value = value;
#if 0
	this->Flags = tile.flag;
#else
	this->Flags &= ~(MapFieldOpaque | MapFieldHuman | MapFieldLandAllowed | MapFieldCoastAllowed |
					 MapFieldWaterAllowed | MapFieldNoBuilding | MapFieldUnpassable |
					 MapFieldWall | MapFieldRocks | MapFieldForest);
	this->Flags |= tile.flag;
	if (!value && (tile.flag & MapFieldForest)) {
		if ((tile.flag & MapFieldCost4) == MapFieldCost4) {
			this->Value = DefaultResourceAmounts[Cost4];
		} else if ((tile.flag & MapFieldCost5) == MapFieldCost5) {
			this->Value = DefaultResourceAmounts[Cost4];
		} else if ((tile.flag & MapFieldCost6) == MapFieldCost6) {
			this->Value = DefaultResourceAmounts[Cost4];
		} else if (tile.flag & MapFieldForest) {
			this->Value = 100; // TODO: should be DefaultResourceAmounts[WoodCost] once all games are migrated
		}
	}
#endif
	this->cost = 1 << (tile.flag & MapFieldSpeedMask);
#ifdef DEBUG
	this->tilesetTile = tileIndex;
#endif
}

void CMapField::Save(CFile &file) const
{
	file.printf("  {%3d, %3d, %2d, %2d", tile, playerInfo.SeenTile, Value, cost);
	for (int i = 0; i != PlayerMax; ++i) {
		if (playerInfo.Visible[i] == 1) {
			file.printf(", \"explored\", %d", i);
		}
	}
	if (Flags & MapFieldOpaque) {
		file.printf(", \"opaque\"");
	}
	if (Flags & MapFieldHuman) {
		file.printf(", \"human\"");
	}
	if (Flags & MapFieldLandAllowed) {
		file.printf(", \"land\"");
	}
	if (Flags & MapFieldCoastAllowed) {
		file.printf(", \"coast\"");
	}
	if (Flags & MapFieldWaterAllowed) {
		file.printf(", \"water\"");
	}
	if (Flags & MapFieldNoBuilding) {
		file.printf(", \"mud\"");
	}
	if (Flags & MapFieldUnpassable) {
		file.printf(", \"block\"");
	}
	if (Flags & MapFieldWall) {
		file.printf(", \"wall\"");
	}
	if (Flags & MapFieldRocks) {
		file.printf(", \"rock\"");
	}
	if (Flags & MapFieldForest) {
		file.printf(", \"wood\"");
	}
	if (Cost4OnMap()) {
		file.printf(", \"cost4\"");
	}
	if (Cost5OnMap()) {
		file.printf(", \"cost5\"");
	}
	if (Cost6OnMap()) {
		file.printf(", \"cost6\"");
	}
#if 1
	// Not Required for save
	// These are required for now, UnitType::FieldFlags is 0 until
	// UpdateStats is called which is after the game is loaded
	if (Flags & MapFieldLandUnit) {
		file.printf(", \"ground\"");
	}
	if (Flags & MapFieldAirUnit) {
		file.printf(", \"air\"");
	}
	if (Flags & MapFieldSeaUnit) {
		file.printf(", \"sea\"");
	}
	if (Flags & MapFieldBuilding) {
		file.printf(", \"building\"");
	}
#endif
	file.printf("}");
}


void CMapField::parse(lua_State *l)
{
	if (!lua_istable(l, -1)) {
		LuaError(l, "incorrect argument");
	}
	const int len = lua_rawlen(l, -1);
	if (len < 4) {
		LuaError(l, "incorrect argument");
	}

	this->tile = LuaToNumber(l, -1, 1);
	this->playerInfo.SeenTile = LuaToNumber(l, -1, 2);
	this->Value = LuaToNumber(l, -1, 3);
	this->cost = LuaToNumber(l, -1, 4);

	for (int j = 4; j < len; ++j) {
		const char *value = LuaToString(l, -1, j + 1);

		if (!strcmp(value, "explored")) {
			++j;
			this->playerInfo.Visible[LuaToNumber(l, -1, j + 1)] = 1;
		} else if (!strcmp(value, "opaque")) {
			this->Flags |= MapFieldOpaque;
		} else if (!strcmp(value, "human")) {
			this->Flags |= MapFieldHuman;
		} else if (!strcmp(value, "land")) {
			this->Flags |= MapFieldLandAllowed;
		} else if (!strcmp(value, "coast")) {
			this->Flags |= MapFieldCoastAllowed;
		} else if (!strcmp(value, "water")) {
			this->Flags |= MapFieldWaterAllowed;
		} else if (!strcmp(value, "mud")) {
			this->Flags |= MapFieldNoBuilding;
		} else if (!strcmp(value, "block")) {
			this->Flags |= MapFieldUnpassable;
		} else if (!strcmp(value, "wall")) {
			this->Flags |= MapFieldWall;
		} else if (!strcmp(value, "rock")) {
			this->Flags |= MapFieldRocks;
		} else if (!strcmp(value, "wood")) {
			this->Flags |= MapFieldForest;
		} else if (!strcmp(value, "cost4")) {
			this->Flags |= MapFieldCost4;
		} else if (!strcmp(value, "cost5")) {
			this->Flags |= MapFieldCost5;
		} else if (!strcmp(value, "cost6")) {
			this->Flags |= MapFieldCost6;
		} else if (!strcmp(value, "ground")) {
			this->Flags |= MapFieldLandUnit;
		} else if (!strcmp(value, "air")) {
			this->Flags |= MapFieldAirUnit;
		} else if (!strcmp(value, "sea")) {
			this->Flags |= MapFieldSeaUnit;
		} else if (!strcmp(value, "building")) {
			this->Flags |= MapFieldBuilding;
		} else {
			LuaError(l, "Unsupported tag: %s" _C_ value);
		}
	}
}

/**
** Check if a field is opaque
** We check not only MapFieldOpaque flag because some field types (f.e. forest/rock/wall) 
** may be set in the FieldOfView as opaque as well.
*/
bool CMapField::isOpaque() const
{
	return (FieldOfView.GetType() == FieldOfViewTypes::cShadowCasting 
			&& FieldOfView.GetOpaqueFields() & this->Flags);
}

/// Check if a field flags.
bool CMapField::CheckMask(int mask) const
{
	return (this->Flags & mask) != 0;
}

/// Returns true, if water on the map tile field
bool CMapField::WaterOnMap() const
{
	return CheckMask(MapFieldWaterAllowed);
}

/// Returns true, if coast on the map tile field
bool CMapField::CoastOnMap() const
{
	return CheckMask(MapFieldCoastAllowed);
}

/// Returns true, if forest on the map tile field
bool CMapField::ForestOnMap() const
{
	return CheckMask(MapFieldForest);
}

/// Returns true, if any terrain resource giving Cost4 on the map tile field
/// Cannot use CheckMask, since two bits need to be tested.
bool CMapField::Cost4OnMap() const
{
	return (this->Flags & MapFieldCost4) == MapFieldCost4;
}

/// Returns true, if any terrain resource giving Cost5 on the map tile field
/// Cannot use CheckMask, since two bits need to be tested.
bool CMapField::Cost5OnMap() const
{
	return (this->Flags & MapFieldCost5) == MapFieldCost5;
}

/// Returns true, if any terrain resource giving Cost6 on the map tile field
/// Cannot use CheckMask, since two bits need to be tested.
bool CMapField::Cost6OnMap() const
{
	return (this->Flags & MapFieldCost6) == MapFieldCost6;
}

/// Returns true, if coast on the map tile field
bool CMapField::RockOnMap() const
{
	return CheckMask(MapFieldRocks);
}

/// Returns true if the field should not need mixing with the surroundings
bool CMapField::isDecorative() const
{
	return CheckMask(MapFieldDecorative);
}

bool CMapField::isAWall() const
{
	return Flags & MapFieldWall;
}
bool CMapField::isHuman() const
{
	return Flags & MapFieldHuman;
}

bool CMapField::isAHumanWall() const
{
	const unsigned int humanWallFlag = (MapFieldWall | MapFieldHuman);
	return (Flags & humanWallFlag) == humanWallFlag;
}
bool CMapField::isAOrcWall() const
{
	const unsigned int humanWallFlag = (MapFieldWall | MapFieldHuman);
	return (Flags & humanWallFlag) == MapFieldWall;
}

//
//  CMapFieldPlayerInfo
//

unsigned char CMapFieldPlayerInfo::TeamVisibilityState(const CPlayer &player) const
{
	if (this->IsVisible(player)) {
		return 2;
	}
	unsigned char maxVision = 0;
	if (this->IsExplored(player)) {
		maxVision = 1;
	}
	
	for (const uint8_t p : player.GetSharedVision()) {
		maxVision = std::max<uint8_t>(maxVision, this->Visible[p]);
		if (maxVision >= 2) {
			return 2;
		}
	}

	if (maxVision == 1 && Map.NoFogOfWar) {
		return 2;
	}
	return maxVision;
}

bool CMapFieldPlayerInfo::IsExplored(const CPlayer &player) const
{
	return this->Visible[player.Index] != 0;
}

bool CMapFieldPlayerInfo::IsVisible(const CPlayer &player) const
{
	const bool fogOfWar = !Map.NoFogOfWar;
	return this->Visible[player.Index] >= 2 || (!fogOfWar && IsExplored(player));
}

bool CMapFieldPlayerInfo::IsTeamVisible(const CPlayer &player) const
{
	return this->TeamVisibilityState(player) == 2;
}

//@}
