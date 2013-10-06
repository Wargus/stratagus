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

#include "stratagus.h"

#include "map.h"

#include "iolib.h"
#include "player.h"
#include "tileset.h"
#include "unit.h"
#include "unit_manager.h"
#include "ui.h"
#include "version.h"
#include "video.h"

/*----------------------------------------------------------------------------
--  Variables
----------------------------------------------------------------------------*/

CMap Map;                   /// The current map
int FlagRevealMap;          /// Flag must reveal the map
int ReplayRevealMap;        /// Reveal Map is replay
int ForestRegeneration;     /// Forest regeneration
char CurrentMapPath[1024];  /// Path of the current map

/*----------------------------------------------------------------------------
--  Visible and explored handling
----------------------------------------------------------------------------*/

/**
**  Marks seen tile -- used mainly for the Fog Of War
**
**  @param mf  MapField-position.
*/
void CMap::MarkSeenTile(CMapField &mf)
{
	const unsigned int tile = mf.getGraphicTile();
	const unsigned int seentile = mf.playerInfo.SeenTile;

	//  Nothing changed? Seeing already the correct tile.
	if (tile == seentile) {
		return;
	}
	mf.playerInfo.SeenTile = tile;

#ifdef MINIMAP_UPDATE
	//rb - GRRRRRRRRRRRR
	const unsigned int index = &mf - Map.Fields;
	const int y = index / Info.MapWidth;
	const int x = index - (y * Info.MapWidth);
	const Vec2i pos = {x, y}
#endif

	if (this->Tileset->TileTypeTable.empty() == false) {
#ifndef MINIMAP_UPDATE
		//rb - GRRRRRRRRRRRR
		const unsigned int index = &mf - Map.Fields;
		const int y = index / Info.MapWidth;
		const int x = index - (y * Info.MapWidth);
		const Vec2i pos(x, y);
#endif

		//  Handle wood changes. FIXME: check if for growing wood correct?
		if (tile == this->Tileset->getRemovedTreeTile()) {
			FixNeighbors(MapFieldForest, 1, pos);
		} else if (seentile == this->Tileset->getRemovedTreeTile()) {
			FixTile(MapFieldForest, 1, pos);
		} else if (mf.ForestOnMap()) {
			FixTile(MapFieldForest, 1, pos);
			FixNeighbors(MapFieldForest, 1, pos);

			// Handle rock changes.
		} else if (tile == Tileset->getRemovedRockTile()) {
			FixNeighbors(MapFieldRocks, 1, pos);
		} else if (seentile == Tileset->getRemovedRockTile()) {
			FixTile(MapFieldRocks, 1, pos);
		} else if (mf.RockOnMap()) {
			FixTile(MapFieldRocks, 1, pos);
			FixNeighbors(MapFieldRocks, 1, pos);

			//  Handle Walls changes.
		} else if (this->Tileset->isAWallTile(tile)
				   || this->Tileset->isAWallTile(seentile)) {
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
	for (int i = 0; i != this->Info.MapWidth * this->Info.MapHeight; ++i) {
		CMapField &mf = *this->Field(i);
		CMapFieldPlayerInfo &playerInfo = mf.playerInfo;
		for (int p = 0; p < PlayerMax; ++p) {
			playerInfo.Visible[p] = std::max<unsigned short>(1, playerInfo.Visible[p]);
		}
		MarkSeenTile(mf);
	}
	//  Global seen recount. Simple and effective.
	for (CUnitManager::Iterator it = UnitManager.begin(); it != UnitManager.end(); ++it) {
		CUnit &unit = **it;
		//  Reveal neutral buildings. Gold mines:)
		if (unit.Player->Type == PlayerNeutral) {
			for (int p = 0; p < PlayerMax; ++p) {
				if (Players[p].Type != PlayerNobody && (!(unit.Seen.ByPlayer & (1 << p)))) {
					UnitGoesOutOfFog(unit, Players[p]);
					UnitGoesUnderFog(unit, Players[p]);
				}
			}
		}
		UnitCountSeen(unit);
	}
}

/*----------------------------------------------------------------------------
--  Map queries
----------------------------------------------------------------------------*/

Vec2i CMap::MapPixelPosToTilePos(const PixelPos &mapPos) const
{
	const Vec2i tilePos(mapPos.x / PixelTileSize.x, mapPos.y / PixelTileSize.y);

	return tilePos;
}

PixelPos CMap::TilePosToMapPixelPos_TopLeft(const Vec2i &tilePos) const
{
	PixelPos mapPixelPos(tilePos.x * PixelTileSize.x, tilePos.y * PixelTileSize.y);

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
	return Field(pos)->isAWall();
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
	return Field(pos)->isAHumanWall();
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
	return Field(pos)->isAOrcWall();
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
	return Map.Info.IsPointOnMap(pos) && CanMoveToMask(pos, mask);
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
				|| Map.Field(pos.x + addx + index)->CheckMask(mask) == true) {
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
	if (unit.Type->BoolFlag[NONSOLID_INDEX].value) {
		return true;
	}
	return UnitTypeCanBeAt(*unit.Type, pos);
}

/**
**  Fixes initially the wood and seen tiles.
*/
void PreprocessMap()
{
	for (int ix = 0; ix < Map.Info.MapWidth; ++ix) {
		for (int iy = 0; iy < Map.Info.MapHeight; ++iy) {
			CMapField &mf = *Map.Field(ix, iy);
			mf.playerInfo.SeenTile = mf.getGraphicTile();
		}
	}
	// it is required for fixing the wood that all tiles are marked as seen!
	if (Map.Tileset->TileTypeTable.empty() == false) {
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

CMap::CMap() : Fields(NULL), NoFogOfWar(false), TileGraphic(NULL)
{
	Tileset = new CTileset;
}

CMap::~CMap()
{
	delete Tileset;
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
	this->Tileset->clear();
	this->TileModelsFileName.clear();
	CGraphic::Free(this->TileGraphic);
	this->TileGraphic = NULL;

	FlagRevealMap = 0;
	ReplayRevealMap = 0;

	UI.Minimap.Destroy();
}

/**
** Save the complete map.
**
** @param file Output file.
*/
void CMap::Save(CFile &file) const
{
	file.printf("\n--- -----------------------------------------\n");
	file.printf("--- MODULE: map\n");
	file.printf("LoadTileModels(\"%s\")\n\n", this->TileModelsFileName.c_str());
	file.printf("StratagusMap(\n");
	file.printf("  \"version\", \"%s\",\n", VERSION);
	file.printf("  \"description\", \"%s\",\n", this->Info.Description.c_str());
	file.printf("  \"the-map\", {\n");
	file.printf("  \"size\", {%d, %d},\n", this->Info.MapWidth, this->Info.MapHeight);
	file.printf("  \"%s\",\n", this->NoFogOfWar ? "no-fog-of-war" : "fog-of-war");
	file.printf("  \"filename\", \"%s\",\n", this->Info.Filename.c_str());
	file.printf("  \"map-fields\", {\n");
	for (int h = 0; h < this->Info.MapHeight; ++h) {
		file.printf("  -- %d\n", h);
		for (int w = 0; w < this->Info.MapWidth; ++w) {
			const CMapField &mf = *this->Field(w, h);

			mf.Save(file);
			if (w & 1) {
				file.printf(",\n");
			} else {
				file.printf(", ");
			}
		}
	}
	file.printf("}})\n");
}

/*----------------------------------------------------------------------------
-- Map Tile Update Functions
----------------------------------------------------------------------------*/

/**
**  Correct the seen wood field, depending on the surrounding.
**
**  @param type  type of tile to update
**  @param seen  1 if updating seen value, 0 for real
**  @param pos   Map tile-position.
*/
void CMap::FixTile(unsigned short type, int seen, const Vec2i &pos)
{
	Assert(type == MapFieldForest || type == MapFieldRocks);

	//  Outside of map or no wood.
	if (!Info.IsPointOnMap(pos)) {
		return;
	}
	unsigned int index = getIndex(pos);
	CMapField &mf = *this->Field(index);

	if (!((type == MapFieldForest && Tileset->isAWoodTile(mf.playerInfo.SeenTile))
		  || (type == MapFieldRocks && Tileset->isARockTile(mf.playerInfo.SeenTile)))) {
		if (seen) {
			return;
		}
	}
	if (!seen && !(mf.getFlag() & type)) {
		return;
	}

	// Select Table to lookup
	int removedtile;
	int flags;
	if (type == MapFieldForest) {
		removedtile = this->Tileset->getRemovedTreeTile();
		flags = (MapFieldForest | MapFieldUnpassable);
	} else { // (type == MapFieldRocks)
		removedtile = this->Tileset->getRemovedRockTile();
		flags = (MapFieldRocks | MapFieldUnpassable);
	}
	//  Find out what each tile has with respect to wood, or grass.
	int ttup;
	int ttdown;
	int ttleft;
	int ttright;

	if (pos.y - 1 < 0) {
		ttup = -1; //Assign trees in all directions
	} else {
		const CMapField &new_mf = *(&mf - this->Info.MapWidth);
		ttup = seen ? new_mf.playerInfo.SeenTile : new_mf.getGraphicTile();
	}
	if (pos.x + 1 >= this->Info.MapWidth) {
		ttright = -1; //Assign trees in all directions
	} else {
		const CMapField &new_mf = *(&mf + 1);
		ttright = seen ? new_mf.playerInfo.SeenTile : new_mf.getGraphicTile();
	}
	if (pos.y + 1 >= this->Info.MapHeight) {
		ttdown = -1; //Assign trees in all directions
	} else {
		const CMapField &new_mf = *(&mf + this->Info.MapWidth);
		ttdown = seen ? new_mf.playerInfo.SeenTile : new_mf.getGraphicTile();
	}
	if (pos.x - 1 < 0) {
		ttleft = -1; //Assign trees in all directions
	} else {
		const CMapField &new_mf = *(&mf - 1);
		ttleft = seen ? new_mf.playerInfo.SeenTile : new_mf.getGraphicTile();
	}
	int tile = this->Tileset->getTileBySurrounding(type, ttup, ttright, ttdown, ttleft);

	//Update seen tile.
	if (tile == -1) { // No valid wood remove it.
		if (seen) {
			mf.playerInfo.SeenTile = removedtile;
			this->FixNeighbors(type, seen, pos);
		} else {
			mf.setGraphicTile(removedtile);
			mf.Flags &= ~flags;
			mf.Value = 0;
			UI.Minimap.UpdateXY(pos);
		}
	} else if (seen && this->Tileset->isEquivalentTile(tile, mf.playerInfo.SeenTile)) { //Same Type
		return;
	} else {
		if (seen) {
			mf.playerInfo.SeenTile = tile;
		} else {
			mf.setGraphicTile(tile);
		}
	}

	//maybe isExplored
	if (mf.playerInfo.IsExplored(*ThisPlayer)) {
		UI.Minimap.UpdateSeenXY(pos);
		if (!seen) {
			MarkSeenTile(mf);
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
	const Vec2i offset[] = {Vec2i(1, 0), Vec2i(-1, 0), Vec2i(0, 1), Vec2i(0, -1),
							Vec2i(-1, -1), Vec2i(-1, 1), Vec2i(1, -1), Vec2i(1, 1)
						   };

	for (unsigned int i = 0; i < sizeof(offset) / sizeof(*offset); ++i) {
		FixTile(type, seen, pos + offset[i]);
	}
}

/// Remove wood from the map.
void CMap::ClearWoodTile(const Vec2i &pos)
{
	CMapField &mf = *this->Field(pos);

	mf.setGraphicTile(this->Tileset->getRemovedTreeTile());
	mf.Flags &= ~(MapFieldForest | MapFieldUnpassable);
	mf.Value = 0;

	UI.Minimap.UpdateXY(pos);
	FixNeighbors(MapFieldForest, 0, pos);

	//maybe isExplored
	if (mf.playerInfo.IsExplored(*ThisPlayer)) {
		UI.Minimap.UpdateSeenXY(pos);
		MarkSeenTile(mf);
	}
}
/// Remove rock from the map.
void CMap::ClearRockTile(const Vec2i &pos)
{
	CMapField &mf = *this->Field(pos);

	mf.setGraphicTile(this->Tileset->getRemovedRockTile());
	mf.Flags &= ~(MapFieldRocks | MapFieldUnpassable);
	mf.Value = 0;

	UI.Minimap.UpdateXY(pos);
	FixNeighbors(MapFieldRocks, 0, pos);

	//maybe isExplored
	if (mf.playerInfo.IsExplored(*ThisPlayer)) {
		UI.Minimap.UpdateSeenXY(pos);
		MarkSeenTile(mf);
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

	if (mf.getGraphicTile() != this->Tileset->getRemovedTreeTile()) {
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
	if (topMf.getGraphicTile() == this->Tileset->getRemovedTreeTile()
		&& topMf.Value >= ForestRegeneration
		&& !(topMf.Flags & occupedFlag)) {
		DebugPrint("Real place wood\n");
		topMf.setGraphicTile(this->Tileset->getTopOneTreeTile());
		topMf.Value = 0;
		topMf.Flags |= MapFieldForest | MapFieldUnpassable;

		mf.setGraphicTile(this->Tileset->getBottomOneTreeTile());
		mf.Value = 0;
		mf.Flags |= MapFieldForest | MapFieldUnpassable;
		if (mf.playerInfo.IsTeamVisible(*ThisPlayer)) {
			MarkSeenTile(mf);
		}
		const Vec2i offset(0, -1);
		if (Map.Field(pos + offset)->playerInfo.IsTeamVisible(*ThisPlayer)) {
			MarkSeenTile(mf);
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
