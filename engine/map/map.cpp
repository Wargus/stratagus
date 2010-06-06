//     ____                _       __               
//    / __ )____  _____   | |     / /___ ___________
//   / __  / __ \/ ___/   | | /| / / __ `/ ___/ ___/
//  / /_/ / /_/ (__  )    | |/ |/ / /_/ / /  (__  ) 
// /_____/\____/____/     |__/|__/\__,_/_/  /____/  
//                                              
//       A futuristic real-time strategy game.
//          This file is part of Bos Wars.
//
/**@name map.cpp - The map. */
//
//      (c) Copyright 1998-2008 by Lutz Sammer, Vladi Shabanski and
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

#include <string.h>

#include "stratagus.h"
#include "unittype.h"
#include "map.h"
#include "minimap.h"
#include "player.h"
#include "unit.h"
#include "unit_cache.h"
#include "ui.h"
#include "script.h"

/*----------------------------------------------------------------------------
--  Variables
----------------------------------------------------------------------------*/

CMap Map;                        /// The current map
int FlagRevealMap;               /// Flag must reveal the map
int ReplayRevealMap;             /// Reveal Map is replay
char CurrentMapPath[1024];       /// Path of the current map

/**
** Size of a tile in X
*/
int TileSizeX = 32;

/**
** Size of a tile in Y
*/
int TileSizeY = 32;

/*----------------------------------------------------------------------------
--  Visible and explored handling
----------------------------------------------------------------------------*/

/**
**  Reveal the entire map.
*/
void CMap::Reveal(void)
{
	int x;

	//
	//  Mark every explored tile as visible. 1 turns into 2.
	//
	for (x = 0; x < this->Info.MapWidth; ++x) {
		for (int y = 0; y < this->Info.MapHeight; ++y) {
			for (int p = 0; p < PlayerMax; ++p) {
				if (!this->Field(x, y)->Visible[p]) {
					this->Field(x, y)->Visible[p] = 1;
				}
			}
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
			for (int p = 0; p < PlayerMax; ++p) {
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
	return (this->Field(tx, ty)->Flags
		& (MapFieldShallowWater | MapFieldDeepWater)) != 0;
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
	return (this->Field(tx, ty)->Flags & MapFieldCoastAllowed) != 0;

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
bool CheckedCanMoveToMask(int x, int y, int mask)
{
	if (x < 0 || y < 0 || x >= Map.Info.MapWidth || y >= Map.Info.MapHeight) {
		return false;
	}

	return !(Map.Field(x, y)->Flags & mask);
}

/**
**  Can a unit of unit-type be placed at this point.
**
**  @param type  unit-type to be checked.
**  @param x     X map tile position.
**  @param y     Y map tile position.
**
**  @return      True if could be entered, false otherwise.
*/
bool UnitTypeCanBeAt(const CUnitType *type, int x, int y)
{
	Assert(type);
	for (int addx = 0; addx < type->TileWidth; ++addx) {
		for (int addy = 0; addy < type->TileHeight; ++addy) {
			if (!CheckedCanMoveToMask(x + addx, y + addy, type->MovementMask)) {
				return false;
			}
		}
	}
	return true;
}

/**
**  Can a unit be placed to this point.
**
**  @param unit  unit to be checked.
**  @param x     X map tile position.
**  @param y     Y map tile position.
**
**  @return      True if could be placeded, false otherwise.
*/
bool UnitCanBeAt(const CUnit *unit, int x, int y)
{
	Assert(unit);
	return UnitTypeCanBeAt(unit->Type, x, y);
}

/**
**  Release info about a map.
**
**  @param info  CMapInfo pointer.
*/
void FreeMapInfo(CMapInfo *info)
{
	if (info) {
		info->Description.clear();
		info->Filename.clear();
		info->MapWidth = info->MapHeight = 0;
		memset(info->PlayerSide, 0, sizeof(info->PlayerSide));
		memset(info->PlayerType, 0, sizeof(info->PlayerType));
		info->MapUID = 0;
	}
}

/**
**  Initialize the map
*/
void CMap::Init()
{
	InitFogOfWar();
	Map.PatchManager.load();
}

/**
**  Alocate and initialize map table
*/
void CMap::Create()
{
	Assert(!this->Fields);

	this->Fields = new CMapField[this->Info.MapWidth * this->Info.MapHeight];
	this->Visible[0] = new unsigned[this->Info.MapWidth * this->Info.MapHeight / 2];
	memset(this->Visible[0], 0, this->Info.MapWidth * this->Info.MapHeight / 2 * sizeof(unsigned));
	UnitCache.Init(this->Info.MapWidth, this->Info.MapHeight);
}

/**
**  Cleanup the map module.
*/
void CMap::Clean(void)
{
	delete[] this->Fields;
	delete[] this->Visible[0];

	FreeMapInfo(&this->Info);
	this->Fields = NULL;
	memset(this->Visible, 0, sizeof(this->Visible));
	this->NoFogOfWar = false;

	this->PatchManager.clear();

	FlagRevealMap = 0;
	ReplayRevealMap = 0;

	UI.Minimap.Destroy();
}


/*----------------------------------------------------------------------------
-- Load Map Functions
----------------------------------------------------------------------------*/

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
