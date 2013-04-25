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

#include "iolib.h"
#include "map.h"
#include "player.h"
#include "script.h"
#include "tileset.h"
#include "unit.h"
#include "unit_manager.h"

CMapField::CMapField() :
	Tile(0),
	Flags(0),
	Cost(0),
	Value(0),
	UnitCache()
#ifdef DEBUG
	, TilesetTile(0)
#endif
{}

bool CMapField::IsTerrainResourceOnMap(int resource) const
{
	// TODO: Hard coded stuff.
	if (resource == WoodCost) {
		return this->ForestOnMap();
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
	this->Tile = tile.tile;
	this->Value = value;
#if 0
	this->Flags = tile.flag;
#else
	this->Flags &= ~(MapFieldHuman | MapFieldLandAllowed | MapFieldCoastAllowed |
					 MapFieldWaterAllowed | MapFieldNoBuilding | MapFieldUnpassable |
					 MapFieldWall | MapFieldRocks | MapFieldForest);
	this->Flags |= tile.flag;
#endif
	this->Cost = 1 << (tile.flag & MapFieldSpeedMask);
#ifdef DEBUG
	this->TilesetTile = tileIndex;
#endif
}

void CMapField::Save(CFile &file) const
{
	file.printf("  {%3d, %3d, %2d, %2d", Tile, playerInfo.SeenTile, Value, Cost);
	for (int i = 0; i != PlayerMax; ++i) {
		if (playerInfo.Visible[i] == 1) {
			file.printf(", \"explored\", %d", i);
		}
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

	this->Tile = LuaToNumber(l, -1, 1);
	this->playerInfo.SeenTile = LuaToNumber(l, -1, 2);
	this->Value = LuaToNumber(l, -1, 3);
	this->Cost = LuaToNumber(l, -1, 4);

	for (int j = 4; j < len; ++j) {
		const char *value = LuaToString(l, -1, j + 1);

		if (!strcmp(value, "explored")) {
			++j;
			this->playerInfo.Visible[LuaToNumber(l, -1, j + 1)] = 1;
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

//
//  CMapFieldPlayerInfo
//

unsigned char CMapFieldPlayerInfo::TeamVisibilityState(const CPlayer &player) const
{
	if (IsVisible(player)) {
		return 2;
	}
	unsigned char maxVision = 0;
	if (IsExplored(player)) {
		maxVision = 1;
	}
	for (int i = 0; i != PlayerMax ; ++i) {
		if (player.IsBothSharedVision(Players[i])) {
			maxVision = std::max<unsigned char>(maxVision, Visible[i]);
			if (maxVision >= 2) {
				return 2;
			}
		}
	}
	if (maxVision == 1 && Map.NoFogOfWar) {
		return 2;
	}
	return maxVision;
}

bool CMapFieldPlayerInfo::IsExplored(const CPlayer &player) const
{
	return Visible[player.Index] != 0;
}

bool CMapFieldPlayerInfo::IsVisible(const CPlayer &player) const
{
	const bool fogOfWar = !Map.NoFogOfWar;
	return Visible[player.Index] >= 2 || (!fogOfWar && IsExplored(player));
}

bool CMapFieldPlayerInfo::IsTeamVisible(const CPlayer &player) const
{
	return TeamVisibilityState(player) == 2;
}

//@}
