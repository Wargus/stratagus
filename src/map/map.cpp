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
/**@name map.cpp - The map. */
//
//      (c) Copyright 1998-2006 by Lutz Sammer, Vladi Shabanski and
//                                 Francois Beerten
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "stratagus.h"
#include "map.h"
#include "tileset.h"
#include "minimap.h"
#include "ui.h"
#include "script.h"

/*----------------------------------------------------------------------------
--  Variables
----------------------------------------------------------------------------*/

CMap Map;                        /// The current map
int FlagRevealMap;               /// Flag must reveal the map
int ReplayRevealMap;             /// Reveal Map is replay
int ForestRegeneration;          /// Forest regeneration
char CurrentMapPath[1024];       /// Path of the current map

/*----------------------------------------------------------------------------
--  Visible and explored handling
----------------------------------------------------------------------------*/

/**
**  Marks seen tile -- used mainly for the Fog Of War
**
**  @param x  Map X tile-position.
**  @param y  Map Y tile-position.
*/
void CMap::MarkSeenTile(const unsigned int index)
{
	CMapField &mf = *this->Field(index);
	const int tile = mf.Tile;
	const int seentile = mf.SeenTile;

	//
	//  Nothing changed? Seeing already the correct tile.
	//
	if (tile == seentile) {
		return;
	}
	mf.SeenTile = tile;

#ifdef MINIMAP_UPDATE
	//rb - GRRRRRRRRRRRR
	const int y = index / Info.MapWidth;
	const int x = index - (y * Info.MapWidth);
	const Vec2i pos = {x, y}
#endif

	if (this->Tileset.TileTypeTable) {
#ifndef MINIMAP_UPDATE
		//rb - GRRRRRRRRRRRR
		const int y = index / Info.MapWidth;
		const int x = index - (y * Info.MapWidth);
		const Vec2i pos = {x, y};
#endif

		//  Handle wood changes. FIXME: check if for growing wood correct?
		if (seentile != this->Tileset.RemovedTree && tile == this->Tileset.RemovedTree) {
			FixNeighbors(MapFieldForest, 1, pos);
		} else if (seentile == this->Tileset.RemovedTree && tile != this->Tileset.RemovedTree) {
			FixTile(MapFieldForest, 1, pos);
		} else if (ForestOnMap(index)) {
			FixTile(MapFieldForest, 1, pos);
			FixNeighbors(MapFieldForest, 1, pos);

			// Handle rock changes.
		} else if (seentile != this->Tileset.RemovedRock && tile == Map.Tileset.RemovedRock) {
			FixNeighbors(MapFieldRocks, 1, pos);
		} else if (seentile == this->Tileset.RemovedRock && tile != Map.Tileset.RemovedRock) {
			FixTile(MapFieldRocks, 1, pos);
		} else if (RockOnMap(index)) {
			FixTile(MapFieldRocks, 1, pos);
			FixNeighbors(MapFieldRocks, 1, pos);

			//  Handle Walls changes.
		} else if (this->Tileset.TileTypeTable[tile] == TileTypeHumanWall
				   || this->Tileset.TileTypeTable[tile] == TileTypeOrcWall
				   || this->Tileset.TileTypeTable[seentile] == TileTypeHumanWall
				   || this->Tileset.TileTypeTable[seentile] == TileTypeOrcWall) {
			MapFixSeenWallTile(pos);
			MapFixSeenWallNeighbors(pos);
		}
	}

#ifdef MINIMAP_UPDATE
	UI.Minimap.UpdateXY(pos);
#endif
}

/**
**  Reveal the entire map.
*/
void CMap::Reveal()
{
	//  Mark every explored tile as visible. 1 turns into 2.
	Vec2i pos;
	for (pos.x = 0; pos.x < this->Info.MapWidth; ++pos.x) {
		for (pos.y = 0; pos.y < this->Info.MapHeight; ++pos.y) {
			for (int p = 0; p < PlayerMax; ++p) {
				if (!this->Field(pos)->Visible[p]) {
					this->Field(pos)->Visible[p] = 1;
				}
			}
			MarkSeenTile(pos);
		}
	}
	//  Global seen recount. Simple and effective.
	for (int i = 0; i < NumUnits; ++i) {
		//  Reveal neutral buildings. Gold mines:)
		if (Units[i]->Player->Type == PlayerNeutral) {
			for (int p = 0; p < PlayerMax; ++p) {
				if (Players[p].Type != PlayerNobody && (!(Units[i]->Seen.ByPlayer & (1 << p)))) {
					UnitGoesOutOfFog(*Units[i], Players[p]);
					UnitGoesUnderFog(*Units[i], Players[p]);
				}
			}
		}
		UnitCountSeen(*Units[i]);
	}
}

/*----------------------------------------------------------------------------
--  Map queries
----------------------------------------------------------------------------*/

Vec2i CMap::MapPixelPosToTilePos(const PixelPos &mapPos) const
{
	const Vec2i tilePos = {mapPos.x / PixelTileSize.x, mapPos.y / PixelTileSize.y};

	return tilePos;
}

PixelPos CMap::TilePosToMapPixelPos_TopLeft(const Vec2i &tilePos) const
{
	PixelPos mapPixelPos = {tilePos.x * PixelTileSize.x, tilePos.y * PixelTileSize.y};

	return mapPixelPos;
}

PixelPos CMap::TilePosToMapPixelPos_Center(const Vec2i &tilePos) const
{
	return TilePosToMapPixelPos_TopLeft(tilePos) + PixelTileSize / 2;
}

/**
**  Wall on map tile.
**
**  @param pos  map tile position.
**
**  @return    True if wall, false otherwise.
*/
bool CMap::WallOnMap(const Vec2i &pos) const
{
	Assert(Map.Info.IsPointOnMap(pos));
	return (Field(pos)->Flags & MapFieldWall) != 0;

}

/**
**  Human wall on map tile.
**
**  @param pos  map tile position.
**
**  @return    True if human wall, false otherwise.
*/
bool CMap::HumanWallOnMap(const Vec2i &pos) const
{
	Assert(Map.Info.IsPointOnMap(pos));
	return (Field(pos)->Flags & (MapFieldWall | MapFieldHuman)) == (MapFieldWall | MapFieldHuman);
}

/**
**  Orc wall on map tile.
**
**  @param pos  map tile position.
**
**  @return    True if orcish wall, false otherwise.
*/
bool CMap::OrcWallOnMap(const Vec2i &pos) const
{
	Assert(Map.Info.IsPointOnMap(pos));
	return (Field(pos)->Flags & (MapFieldWall | MapFieldHuman)) == MapFieldWall;
}

/**
**  Can move to this point, applying mask.
**
**  @param pos   map tile position.
**  @param mask  Mask for movement to apply.
**
**  @return      True if could be entered, false otherwise.
*/
bool CheckedCanMoveToMask(const Vec2i &pos, int mask)
{
	return Map.Info.IsPointOnMap(pos) && !Map.CheckMask(pos, mask);
}

/**
**  Can a unit of unit-type be placed at this point.
**
**  @param type  unit-type to be checked.
**  @param pos   map tile position.
**
**  @return      True if could be entered, false otherwise.
*/
bool UnitTypeCanBeAt(const CUnitType &type, const Vec2i &pos)
{
	const int mask = type.MovementMask;
	unsigned int index = pos.y * Map.Info.MapWidth;

	for (int addy = 0; addy < type.TileHeight; ++addy) {
		for (int addx = 0; addx < type.TileWidth; ++addx) {
			if (Map.Info.IsPointOnMap(pos.x + addx, pos.y + addy) == false
				|| Map.CheckMask(pos.x + addx + index, mask) == true) {
				return false;
			}
		}
		index += Map.Info.MapWidth;
	}
	return true;
}

/**
**  Can a unit be placed to this point.
**
**  @param unit  unit to be checked.
**  @param pos   map tile position.
**
**  @return      True if could be placeded, false otherwise.
*/
bool UnitCanBeAt(const CUnit &unit, const Vec2i &pos)
{
	Assert(unit.Type);

	return UnitTypeCanBeAt(*unit.Type, pos);
}

/**
**  Fixes initially the wood and seen tiles.
*/
void PreprocessMap()
{
	for (int ix = 0; ix < Map.Info.MapWidth; ++ix) {
		for (int iy = 0; iy < Map.Info.MapHeight; ++iy) {
			CMapField *mf = Map.Field(ix, iy);
			mf->SeenTile = mf->Tile;
		}
	}
	// it is required for fixing the wood that all tiles are marked as seen!
	if (Map.Tileset.TileTypeTable) {
		Vec2i pos;
		for (pos.x = 0; pos.x < Map.Info.MapWidth; ++pos.x) {
			for (pos.y = 0; pos.y < Map.Info.MapHeight; ++pos.y) {
				MapFixWallTile(pos);
				MapFixSeenWallTile(pos);
			}
		}
	}
}

/**
**  Clear CMapInfo.
*/
void CMapInfo::Clear()
{
	this->Description.clear();
	this->Filename.clear();
	this->MapWidth = this->MapHeight = 0;
	memset(this->PlayerSide, 0, sizeof(this->PlayerSide));
	memset(this->PlayerType, 0, sizeof(this->PlayerType));
	this->MapUID = 0;
}

/**
**  Alocate and initialise map table
*/
void CMap::Create()
{
	Assert(!this->Fields);

	this->Fields = new CMapField[this->Info.MapWidth * this->Info.MapHeight];
}

/**
**  Initialize the fog of war.
**  Build tables, setup functions.
*/
void CMap::Init()
{
	InitFogOfWar();
}


/**
**  Cleanup the map module.
*/
void CMap::Clean()
{
	delete[] this->Fields;

	// Tileset freed by Tileset?

	this->Info.Clear();
	this->Fields = NULL;
	this->NoFogOfWar = false;
	this->Tileset.Clear();
	memset(this->TileModelsFileName, 0, sizeof(this->TileModelsFileName));
	this->TileGraphic = NULL;

	FlagRevealMap = 0;
	ReplayRevealMap = 0;

	UI.Minimap.Destroy();
}


/*----------------------------------------------------------------------------
-- Map Tile Update Functions
----------------------------------------------------------------------------*/

/**
**  Correct the seen wood field, depending on the surrounding.
**
**  @param type  type fo tile to update
**  @param seen  1 if updating seen value, 0 for real
**  @param pos   Map tile-position.
*/
void CMap::FixTile(unsigned short type, int seen, const Vec2i &pos)
{
	//  Outside of map or no wood.
	if (!Info.IsPointOnMap(pos)) {
		return;
	}
	unsigned int index = getIndex(pos);
	CMapField *mf = this->Field(index);

	if (seen && !Tileset.IsSeenTile(type, mf->SeenTile)) {
		return;
	} else if (!seen && !(mf->Flags & type)) {
		return;
	}

	// Select Table to lookup
	int *lookuptable;
	int removedtile;
	int flags;
	switch (type) {
		case MapFieldForest:
			lookuptable = this->Tileset.WoodTable;
			removedtile = this->Tileset.RemovedTree;
			flags = (MapFieldForest | MapFieldUnpassable);
			break;
		case MapFieldRocks:
			lookuptable = this->Tileset.RockTable;
			removedtile = this->Tileset.RemovedRock;
			flags = (MapFieldRocks | MapFieldUnpassable);
			break;
		default:
			lookuptable = NULL;
			removedtile = 0;
			flags = 0;
	}
	//
	//  Find out what each tile has with respect to wood, or grass.
	//
	int tile = 0;
	int ttup;
	int ttdown;
	int ttleft;
	int ttright;

	if (pos.y - 1 < 0) {
		ttup = 15; //Assign trees in all directions
	} else {
		CMapField *new_mf = (mf - this->Info.MapWidth);
		if (seen) {
			ttup = this->Tileset.MixedLookupTable[new_mf->SeenTile];
		} else {
			ttup = this->Tileset.MixedLookupTable[new_mf->Tile];
		}
	}
	if (pos.x + 1 >= this->Info.MapWidth) {
		ttright = 15; //Assign trees in all directions
	} else {
		CMapField *new_mf = (mf + 1);
		if (seen) {
			ttright = this->Tileset.MixedLookupTable[new_mf->SeenTile];
		} else {
			ttright = this->Tileset.MixedLookupTable[new_mf->Tile];
		}
	}
	if (pos.y + 1 >= this->Info.MapHeight) {
		ttdown = 15; //Assign trees in all directions
	} else {
		CMapField *new_mf = (mf + this->Info.MapWidth);
		if (seen) {
			ttdown = this->Tileset.MixedLookupTable[new_mf->SeenTile];
		} else {
			ttdown = this->Tileset.MixedLookupTable[new_mf->Tile];
		}
	}
	if (pos.x - 1 < 0) {
		ttleft = 15; //Assign trees in all directions
	} else {
		CMapField *new_mf = (mf - 1);
		if (seen) {
			ttleft = this->Tileset.MixedLookupTable[new_mf->SeenTile];
		} else {
			ttleft = this->Tileset.MixedLookupTable[new_mf->Tile];
		}
	}

	//
	//  Check each of the corners to ensure it has both connecting
	//  ?**?
	//  *mm*
	//  *mm*
	//  ?**?
	//  *
	//   *  type asterixs must match for wood to be present

	tile += ((ttup & 0x01) && (ttleft & 0x04)) * 8;
	tile += ((ttup & 0x02) && (ttright & 0x08)) * 4;
	tile += ((ttright & 0x01) && (ttdown & 0x04)) * 2;
	tile += ((ttleft & 0x02) && (ttdown & 0x08)) * 1;

	//Test if we have top tree, or bottom tree, they are special
	if ((ttdown & 0x10) && 1) {
		tile |= ((ttleft & 0x06) && 1) * 1;
		tile |= ((ttright & 0x09) && 1) * 2;
	}

	if ((ttup & 0x20) && 1) {
		tile |= ((ttleft & 0x06) && 1) * 8;
		tile |= ((ttright & 0x09) && 1) * 4;
	}

	tile = lookuptable[tile];

	//If tile is -1, then we should check if we are to draw just one tree
	//Check for tile about, or below or both...
	if (tile == -1) {
		tile = 16;
		tile += ((ttup & 0x01) || (ttup & 0x02)) * 1;
		tile += ((ttdown & 0x04) || (ttdown & 0x08)) * 2;
		tile = lookuptable[tile];
	}

	//Update seen tile.
	if (tile == -1) { // No valid wood remove it.
		if (seen) {
			mf->SeenTile = removedtile;
			this->FixNeighbors(type, seen, pos);
		} else {
			mf->Tile = removedtile;
			mf->Flags &= ~flags;
			mf->Value = 0;
			UI.Minimap.UpdateXY(pos);
		}
	} else if (seen && this->Tileset.MixedLookupTable[mf->SeenTile] ==
			   this->Tileset.MixedLookupTable[tile]) { //Same Type
		return;
	} else {
		if (seen) {
			mf->SeenTile = tile;
		} else {
			mf->Tile = tile;
		}
	}

	//maybe isExplored
	if (IsTileVisible(*ThisPlayer, index) > 0) {
		UI.Minimap.UpdateSeenXY(pos);
		if (!seen) {
			MarkSeenTile(pos);
		}
	}
}

/**
**  Correct the surrounding fields.
**
**  @param type  Tiletype of tile to adjust
**  @param seen  1 if updating seen value, 0 for real
**  @param pos   Map tile-position.
*/
void CMap::FixNeighbors(unsigned short type, int seen, const Vec2i &pos)
{
	const Vec2i offset[] = {{1, 0}, { -1, 0}, {0, 1}, {0, -1}, { -1, -1}, { -1, 1}, {1, -1}, {1, 1}};

	for (unsigned int i = 0; i < sizeof(offset) / sizeof(*offset); ++i) {
		FixTile(type, seen, pos + offset[i]);
	}
}

/**
**  Remove wood from the map.
**
**  @param type  TileType to clear
**  @param pos   Map tile-position.
*/
void CMap::ClearTile(unsigned short type, const Vec2i &pos)
{
	int removedtile;
	int flags;

	unsigned int index = getIndex(pos);
	CMapField &mf = *this->Field(index);

	// Select Table to lookup
	switch (type) {
		case MapFieldForest:
			removedtile = this->Tileset.RemovedTree;
			flags = (MapFieldForest | MapFieldUnpassable);
			break;
		case MapFieldRocks:
			removedtile = this->Tileset.RemovedRock;
			flags = (MapFieldRocks | MapFieldUnpassable);
			break;
		default:
			removedtile = flags = 0;
	}
	mf.Tile = removedtile;
	mf.Flags &= ~flags;
	mf.Value = 0;

	UI.Minimap.UpdateXY(pos);
	FixNeighbors(type, 0, pos);

	//maybe isExplored
	if (IsTileVisible(*ThisPlayer, index) > 0) {
		UI.Minimap.UpdateSeenXY(pos);
		MarkSeenTile(pos);
	}
}

/**
**  Regenerate forest.
**
**  @param pos  Map tile pos
*/
void CMap::RegenerateForestTile(const Vec2i &pos)
{
	Assert(Map.Info.IsPointOnMap(pos));
	CMapField &mf = *this->Field(pos);

	if (mf.Tile != this->Tileset.RemovedTree) {
		return;
	}

	//  Increment each value of no wood.
	//  If grown up, place new wood.
	//  FIXME: a better looking result would be fine
	//    Allow general updates to any tiletype that regrows

	const unsigned int occupedFlag = (MapFieldWall | MapFieldUnpassable | MapFieldLandUnit | MapFieldBuilding);
	++mf.Value;
	if (mf.Value < ForestRegeneration) {
		return;
	}
	mf.Value = ForestRegeneration;
	if ((mf.Flags & occupedFlag) || pos.y == 0) {
		return;
	}
	CMapField &topMf = *(&mf - this->Info.MapWidth);
	if (topMf.Tile == this->Tileset.RemovedTree
		&& topMf.Value >= ForestRegeneration
		&& !(topMf.Flags & occupedFlag)) {
		DebugPrint("Real place wood\n");
		topMf.Tile = this->Tileset.TopOneTree;
		topMf.Value = 0;
		topMf.Flags |= MapFieldForest | MapFieldUnpassable;

		mf.Tile = this->Tileset.BotOneTree;
		mf.Value = 0;
		mf.Flags |= MapFieldForest | MapFieldUnpassable;
		if (Map.IsFieldVisible(*ThisPlayer, pos)) {
			MarkSeenTile(pos);
		}
		const Vec2i offset = {0, -1};
		if (Map.IsFieldVisible(*ThisPlayer, pos + offset)) {
			MarkSeenTile(pos);
		}
	}
}

/**
**  Regenerate forest.
*/
void CMap::RegenerateForest()
{
	if (!ForestRegeneration) {
		return;
	}
	Vec2i pos;
	for (pos.y = 0; pos.y < Info.MapHeight; ++pos.y) {
		for (pos.x = 0; pos.x < Info.MapWidth; ++pos.x) {
			RegenerateForestTile(pos);
		}
	}
}


/**
**  Load the map presentation
**
**  @param mapname  map filename
*/
void LoadStratagusMapInfo(const std::string &mapname)
{
	// Set the default map setup by replacing .smp with .sms
	size_t loc = mapname.find(".smp");
	if (loc != std::string::npos) {
		Map.Info.Filename = mapname;
		Map.Info.Filename.replace(loc, 4, ".sms");
	}

	LuaLoadFile(mapname);
}

//@}
