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
//      $Id$

//@{

/*----------------------------------------------------------------------------
--  Includes
----------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "stratagus.h"
#include "unittype.h"
#include "map.h"
#include "tileset.h"
#include "minimap.h"
#include "player.h"
#include "unit.h"
#include "pathfinder.h"
#include "ui.h"
#include "editor.h"
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
void CMap::MarkSeenTile(int x, int y)
{
	int tile;
	int seentile;
	CMapField *mf;

	mf = this->Fields + x + y * this->Info.MapWidth;
	//
	//  Nothing changed? Seeing already the correct tile.
	//
	if ((tile = mf->Tile) == (seentile = mf->SeenTile)) {
		return;
	}
	mf->SeenTile = tile;

	if (this->Tileset.TileTypeTable) {
		//  Handle wood changes. FIXME: check if for growing wood correct?
		if (seentile != this->Tileset.RemovedTree &&
				tile == this->Tileset.RemovedTree) {
			FixNeighbors(MapFieldForest, 1, x, y);
		} else if (seentile == this->Tileset.RemovedTree &&
				tile != this->Tileset.RemovedTree) {
			FixTile(MapFieldForest, 1, x, y);
		} else if (ForestOnMap(x, y)) {
			FixTile(MapFieldForest, 1, x, y);
			FixNeighbors(MapFieldForest, 1, x, y);

		// Handle rock changes.
		} else if (seentile != this->Tileset.RemovedRock &&
				tile == Map.Tileset.RemovedRock) {
			FixNeighbors(MapFieldRocks, 1, x, y);
		} else if (seentile == this->Tileset.RemovedRock &&
				tile != Map.Tileset.RemovedRock) {
			FixTile(MapFieldRocks, 1, x, y);
		} else if (RockOnMap(x, y)) {
			FixTile(MapFieldRocks, 1, x, y);
			FixNeighbors(MapFieldRocks, 1, x, y);

		//  Handle Walls changes.
		} else if (this->Tileset.TileTypeTable[tile] == TileTypeHumanWall ||
				this->Tileset.TileTypeTable[tile] == TileTypeOrcWall ||
				this->Tileset.TileTypeTable[seentile] == TileTypeHumanWall ||
				this->Tileset.TileTypeTable[seentile] == TileTypeOrcWall) {
			MapFixSeenWallTile(x, y);
			MapFixSeenWallNeighbors(x, y);
		}
	}

	UI.Minimap.UpdateXY(x, y);
}

/**
**  Reveal the entire map.
*/
void CMap::Reveal(void)
{
	int x;
	int y;
	int p;  // iterator on player.

	//
	//  Mark every explored tile as visible. 1 turns into 2.
	//
	for (x = 0; x < this->Info.MapWidth; ++x) {
		for (y = 0; y < this->Info.MapHeight; ++y) {
			for (p = 0; p < PlayerMax; ++p) {
				if (!this->Fields[x + y * this->Info.MapWidth].Visible[p]) {
					this->Fields[x + y * this->Info.MapWidth].Visible[p] = 1;
				}
			}
			MarkSeenTile(x, y);
		}
	}
	//
	//  Global seen recount. Simple and effective.
	//
	for (x = 0; x < NumUnits; ++x) {
		//
		//  Reveal neutral buildings. Gold mines:)
		//
		if (Units[x]->Player->Type == PlayerNeutral) {
			for (p = 0; p < PlayerMax; ++p) {
				if (Players[p].Type != PlayerNobody &&
						(!(Units[x]->Seen.ByPlayer & (1 << p)))) {
					UnitGoesOutOfFog(Units[x], Players + p);
					UnitGoesUnderFog(Units[x], Players + p);
				}
			}
		}
		UnitCountSeen(Units[x]);
	}
}

/*----------------------------------------------------------------------------
--  Map queries
----------------------------------------------------------------------------*/

/**
**  Water on map tile.
**
**  @param tx  X map tile position.
**  @param ty  Y map tile position.
**
**  @return    True if water, false otherwise.
*/
bool CMap::WaterOnMap(int tx, int ty) const
{
	Assert(tx >= 0 && ty >= 0 && tx < Info.MapWidth && ty < Info.MapHeight);
	return (Fields[tx + ty * Info.MapWidth].Flags & MapFieldWaterAllowed) != 0;
}

/**
**  Coast on map tile.
**
**  @param tx  X map tile position.
**  @param ty  Y map tile position.
**
**  @return    True if coast, false otherwise.
*/
bool CMap::CoastOnMap(int tx, int ty) const
{
	Assert(tx >= 0 && ty >= 0 && tx < Info.MapWidth && ty < Info.MapHeight);
	return (Fields[tx + ty * Info.MapWidth].Flags & MapFieldCoastAllowed) != 0;

}

/**
**  Wall on map tile.
**
**  @param tx  X map tile position.
**  @param ty  Y map tile position.
**
**  @return    True if wall, false otherwise.
*/
bool CMap::WallOnMap(int tx, int ty) const
{
	Assert(tx >= 0 && ty >= 0 && tx < Info.MapWidth && ty < Info.MapHeight);
	return (Fields[tx + ty * Info.MapWidth].Flags & MapFieldWall) != 0;

}

/**
**  Human wall on map tile.
**
**  @param tx  X map tile position.
**  @param ty  Y map tile position.
**
**  @return    True if human wall, false otherwise.
*/
bool CMap::HumanWallOnMap(int tx, int ty) const
{
	Assert(tx >= 0 && ty >= 0 && tx < Info.MapWidth && ty < Info.MapHeight);
	return (Fields[tx + ty * Info.MapWidth].Flags &
		(MapFieldWall | MapFieldHuman)) == (MapFieldWall | MapFieldHuman);
}

/**
**  Orc wall on map tile.
**
**  @param tx  X map tile position.
**  @param ty  Y map tile position.
**
**  @return    True if orcish wall, false otherwise.
*/
bool CMap::OrcWallOnMap(int tx, int ty) const
{
	Assert(tx >= 0 && ty >= 0 && tx < Info.MapWidth && ty < Info.MapHeight);
	return (Fields[tx + ty * Info.MapWidth].Flags &
		(MapFieldWall | MapFieldHuman)) == MapFieldWall;
}

/**
**  Forest on map tile.
**
**  @param tx  X map tile position.
**  @param ty  Y map tile position.
**
**  @return    True if forest, false otherwise.
*/
bool CMap::ForestOnMap(int tx, int ty) const
{
	Assert(tx >= 0 && ty >= 0 && tx < Info.MapWidth && ty < Info.MapHeight);

	return (Fields[tx + ty * Info.MapWidth].Flags & MapFieldForest) != 0;
}

/**
**  Rock on map tile.
**
**  @param tx  X map tile position.
**  @param ty  Y map tile position.
**
**  @return    True if rock, false otherwise.
*/
bool CMap::RockOnMap(int tx, int ty) const
{
	Assert(tx >= 0 && ty >= 0 && tx < Info.MapWidth && ty < Info.MapHeight);

	return (Fields[tx + ty * Info.MapWidth].Flags & MapFieldRocks) != 0;
}

/**
**  Can move to this point, applying mask.
**
**  @param x     X map tile position.
**  @param y     Y map tile position.
**  @param mask  Mask for movement to apply.
**
**  @return      True if could be entered, false otherwise.
*/
int CheckedCanMoveToMask(int x, int y, int mask)
{
	if (x < 0 || y < 0 || x >= Map.Info.MapWidth || y >= Map.Info.MapHeight) {
		return 0;
	}

	return !(Map.Fields[x + y * Map.Info.MapWidth].Flags & mask);
}

/**
**  Can an unit of unit-type be placed at this point.
**
**  @param type  unit-type to be checked.
**  @param x     X map tile position.
**  @param y     Y map tile position.
**
**  @return      True if could be entered, false otherwise.
*/
int UnitTypeCanBeAt(const CUnitType *type, int x, int y)
{
	int addx;  // iterator
	int addy;  // iterator
	int mask;  // movement mask of the unit.

	Assert(type);
	mask = type->MovementMask;
	for (addx = 0; addx < type->TileWidth; addx++) {
		for (addy = 0; addy < type->TileHeight; addy++) {
			if (!CheckedCanMoveToMask(x + addx, y + addy, mask)) {
				return 0;
			}
		}
	}
	return 1;
}

/**
**  Can an unit be placed to this point.
**
**  @param unit  unit to be checked.
**  @param x     X map tile position.
**  @param y     Y map tile position.
**
**  @return      True if could be placeded, false otherwise.
*/
int UnitCanBeAt(const CUnit *unit, int x, int y)
{
	Assert(unit);
	return UnitTypeCanBeAt(unit->Type, x, y);
}

/**
**  Fixes initially the wood and seen tiles.
*/
void PreprocessMap(void)
{
	int ix;
	int iy;
	CMapField *mf;

	for (ix = 0; ix < Map.Info.MapWidth; ++ix) {
		for (iy = 0; iy < Map.Info.MapHeight; ++iy) {
			mf = Map.Fields + ix + iy * Map.Info.MapWidth;
			mf->SeenTile = mf->Tile;
		}
	}

	// it is required for fixing the wood that all tiles are marked as seen!
	if (Map.Tileset.TileTypeTable) {
		for (ix = 0; ix < Map.Info.MapWidth; ++ix) {
			for (iy = 0; iy < Map.Info.MapHeight; ++iy) {
				MapFixWallTile(ix, iy);
				MapFixSeenWallTile(ix, iy);
			}
		}
	}
}

/**
**  Release info about a map.
**
**  @param info  CMapInfo pointer.
*/
void FreeMapInfo(CMapInfo *info)
{
	if (info) {
		delete[] info->Description;
		delete[] info->Filename;
		memset(info, 0, sizeof(CMapInfo));
	}
}

/**
**  Alocate and initialise map table
*/
void CMap::Create()
{
	Assert(!this->Fields);

	this->Fields = new CMapField[this->Info.MapWidth * this->Info.MapHeight];
	this->Visible[0] = new unsigned[this->Info.MapWidth * this->Info.MapHeight / 2];
	memset(this->Visible[0], 0, this->Info.MapWidth * this->Info.MapHeight / 2 * sizeof(unsigned));
	InitUnitCache();
}

/**
**  Cleanup the map module.
*/
void CMap::Clean(void)
{
	delete[] this->Fields;
	delete[] this->Visible[0];

	// Tileset freed by Tileset?

	FreeMapInfo(&this->Info);
	memset(this, 0, sizeof(*this));
	FlagRevealMap = 0;
	ReplayRevealMap = 0;

	UI.Minimap.Destroy();
}


/*----------------------------------------------------------------------------
-- Map Tile Update Functions
----------------------------------------------------------------------------*/

/**
**  Check if the seen tile-type is wood or rock.
**
**  @param type  Tile type
**  @param x     Map X tile-position.
**  @param y     Map Y tile-position.
*/
bool CMap::IsSeenTile(unsigned short type, int x, int y) const
{
	switch (type) {
		case MapFieldForest:
			return this->Tileset.TileTypeTable[
				this->Fields[x + y * this->Info.MapWidth].SeenTile] == TileTypeWood;
		case MapFieldRocks:
			return this->Tileset.TileTypeTable[
				this->Fields[x + y * this->Info.MapWidth].SeenTile] == TileTypeRock;
		default:
			return false;
	}
}

/**
**  Correct the seen wood field, depending on the surrounding.
**
**  @param type  type fo tile to update
**  @param seen  1 if updating seen value, 0 for real
**  @param x     Map X tile-position.
**  @param y     Map Y tile-position.
*/
void CMap::FixTile(unsigned short type, int seen, int x, int y)
{
	int tile;
	int ttup;
	int ttdown;
	int ttleft;
	int ttright;
	int *lookuptable;
	int removedtile;
	int flags;
	CMapField *mf;


	//  Outside of map or no wood.
	if (x < 0 || y < 0 || x >= this->Info.MapWidth || y >= this->Info.MapHeight) {
		return;
	}

	mf = this->Fields + x + y * this->Info.MapWidth;

	if (seen && !IsSeenTile(type, x, y)) {
		return;
	} else if (!seen && !(mf->Flags & MapFieldForest)) {
		return;
	}

	// Select Table to lookup
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
	tile = 0;
	if (y - 1 < 0) {
		ttup = 15; //Assign trees in all directions
	} else {
		if (seen) {
			ttup = this->Tileset.MixedLookupTable[
				this->Fields[x + (y - 1) * this->Info.MapWidth].SeenTile];
		} else {
			ttup = this->Tileset.MixedLookupTable[
				this->Fields[x + (y - 1) * this->Info.MapWidth].Tile];
		}
	}
	if (x + 1 >= this->Info.MapWidth) {
		ttright = 15; //Assign trees in all directions
	} else {
		if (seen) {
			ttright = this->Tileset.MixedLookupTable[
				this->Fields[x + 1 + y  * this->Info.MapWidth].SeenTile];
		} else {
			ttright = this->Tileset.MixedLookupTable[
				this->Fields[x + 1 + y  * this->Info.MapWidth].Tile];
		}
	}
	if (y + 1 >= this->Info.MapHeight) {
		ttdown = 15; //Assign trees in all directions
	} else {
		if (seen) {
			ttdown = this->Tileset.MixedLookupTable[
				this->Fields[x + (y + 1) * this->Info.MapWidth].SeenTile];
		} else {
			ttdown = this->Tileset.MixedLookupTable[
				this->Fields[x + (y + 1) * this->Info.MapWidth].Tile];
		}
	}
	if (x - 1 < 0) {
		ttleft = 15; //Assign trees in all directions
	} else {
		if (seen) {
			ttleft = this->Tileset.MixedLookupTable[
				this->Fields[x -1 + y * this->Info.MapWidth].SeenTile];
		} else {
			ttleft = this->Tileset.MixedLookupTable[
				this->Fields[x -1 + y * this->Info.MapWidth].Tile];
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
		tile |= ((ttleft & 0x06) && 1)* 1;
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
			this->FixNeighbors(type, seen, x, y);
		} else {
			mf->Tile = removedtile;
			mf->Flags &= ~flags;
			mf->Value = 0;
			UI.Minimap.UpdateXY(x, y);
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

	if (Map.IsFieldVisible(ThisPlayer, x, y)) {
		UI.Minimap.UpdateSeenXY(x, y);
		if (!seen) {
			MarkSeenTile(x, y);
		}
	}
}

/**
**  Correct the surrounding fields.
**
**  @param type  Tiletype of tile to adjust
**  @param seen  1 if updating seen value, 0 for real
**  @param x     Map X tile-position.
**  @param y     Map Y tile-position.
*/
void CMap::FixNeighbors(unsigned short type, int seen, int x, int y)
{
	FixTile(type, seen, x + 1, y); // side neighbors
	FixTile(type, seen, x - 1, y);
	FixTile(type, seen, x, y + 1);
	FixTile(type, seen, x, y - 1);
	FixTile(type, seen, x + 1, y - 1); // side neighbors
	FixTile(type, seen, x - 1, y - 1);
	FixTile(type, seen, x - 1, y + 1);
	FixTile(type, seen, x + 1, y + 1);
}

/**
**  Remove wood from the map.
**
**  @param type  TileType to clear
**  @param x     Map X tile-position.
**  @param y     Map Y tile-position.
*/
void CMap::ClearTile(unsigned short type, unsigned x, unsigned y)
{
	CMapField *mf;
	int removedtile;
	int flags;

	mf = this->Fields + x + y * this->Info.MapWidth;

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
	//
	mf->Tile = removedtile;
	mf->Flags &= ~flags;
	mf->Value = 0;

	UI.Minimap.UpdateXY(x, y);
	FixNeighbors(type, 0, x, y);

	if (Map.IsFieldVisible(ThisPlayer, x, y)) {
		UI.Minimap.UpdateSeenXY(x, y);
		MarkSeenTile(x, y);
	}
}

/**
**  Load the map presentation
**
**  @param mapname    map filename
**/
void LoadStratagusMapInfo(const char *mapname) 
{
	// Set the default map setup by replacing .smp with .sms
	delete[] Map.Info.Filename;
	Map.Info.Filename = new_strdup(mapname);
	memcpy(strstr(Map.Info.Filename, ".smp"), ".sms", 4);
	
	LuaLoadFile(mapname);
}

//@}
