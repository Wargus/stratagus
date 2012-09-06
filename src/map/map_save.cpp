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
/**@name map_save.cpp - Saving the map. */
//
//      (c) Copyright 2001-2005 by Lutz Sammer and Jimmy Salmon
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
-- Includes
----------------------------------------------------------------------------*/

#include "stratagus.h"
#include "map.h"

#include "iolib.h"
#include "version.h"

/*----------------------------------------------------------------------------
-- Variables
----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
-- Functions
----------------------------------------------------------------------------*/

void CMapField::Save(CFile &file) const
{
	file.printf("  {%3d, %3d, %2d, %2d,", Tile, playerInfo.SeenTile, Value, Cost);
	for (int i = 0; i != PlayerMax; ++i) {
		if (playerInfo.Visible[i] == 1) {
			file.printf(" \"explored\", %d,", i);
		}
	}
	if (Flags & MapFieldHuman) {
		file.printf(" \"human\",");
	}
	if (Flags & MapFieldLandAllowed) {
		file.printf(" \"land\",");
	}
	if (Flags & MapFieldCoastAllowed) {
		file.printf(" \"coast\",");
	}
	if (Flags & MapFieldWaterAllowed) {
		file.printf(" \"water\",");
	}
	if (Flags & MapFieldNoBuilding) {
		file.printf(" \"mud\",");
	}
	if (Flags & MapFieldUnpassable) {
		file.printf(" \"block\",");
	}
	if (Flags & MapFieldWall) {
		file.printf(" \"wall\",");
	}
	if (Flags & MapFieldRocks) {
		file.printf(" \"rock\",");
	}
	if (Flags & MapFieldForest) {
		file.printf(" \"wood\",");
	}
#if 1
	// Not Required for save
	// These are required for now, UnitType::FieldFlags is 0 until
	// UpdateStats is called which is after the game is loaded
	if (Flags & MapFieldLandUnit) {
		file.printf(" \"ground\",");
	}
	if (Flags & MapFieldAirUnit) {
		file.printf(" \"air\",");
	}
	if (Flags & MapFieldSeaUnit) {
		file.printf(" \"sea\",");
	}
	if (Flags & MapFieldBuilding) {
		file.printf(" \"building\",");
	}
#endif
	file.printf("}");
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

	file.printf("  \"version\", \"" StratagusFormatString "\",\n",
				StratagusFormatArgs(StratagusVersion));
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

//@}
