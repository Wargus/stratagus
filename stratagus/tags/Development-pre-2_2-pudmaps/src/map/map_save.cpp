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
/**@name map_save.c - Saving the map. */
//
//      (c) Copyright 2001-2004 by Lutz Sammer and Jimmy Salmon
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
-- Includes
----------------------------------------------------------------------------*/

#include <stdio.h>
#include <string.h>

#include "stratagus.h"
#include "map.h"
#include "tileset.h"
#include "minimap.h"
#include "player.h"
#include "unit.h"
#include "pathfinder.h"
#include "pud.h"
#include "ui.h"

#include "script.h"

/*----------------------------------------------------------------------------
-- Variables
----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
-- Functions
----------------------------------------------------------------------------*/

/**
** Save the complete map.
**
** @param file Output file.
*/
void SaveMap(CLFile* file)
{
	int w;
	int h;
	int i;

	CLprintf(file, "\n--- -----------------------------------------\n");
	CLprintf(file, "--- MODULE: map $Id$\n");

	CLprintf(file, "StratagusMap(\n");

	CLprintf(file, "  \"version\", \"" StratagusFormatString "\",\n",
		StratagusFormatArgs(StratagusVersion));
	CLprintf(file, "  \"description\", \"%s\",\n", TheMap.Info.Description);

	CLprintf(file, "  \"the-map\", {\n");

	// FIXME: Why terrain? TheMap->Tileset->Class should be correct
	CLprintf(file, "  \"terrain\", {\"%s\", \"%s\"},\n",
		TheMap.TerrainName, Tilesets[TheMap.Terrain]->Class);

	CLprintf(file, "  \"size\", {%d, %d},\n", TheMap.Info.MapWidth, TheMap.Info.MapHeight);
	CLprintf(file, "  \"%s\",\n", TheMap.NoFogOfWar ? "no-fog-of-war" : "fog-of-war");
	CLprintf(file, "  \"filename\", \"%s\",\n", TheMap.Info.Filename);

	CLprintf(file, "  \"map-fields\", {\n");
	for (h = 0; h < TheMap.Info.MapHeight; ++h) {
		CLprintf(file, "  -- %d\n", h);
		for (w = 0; w < TheMap.Info.MapWidth; ++w) {
			MapField* mf;

			mf = &TheMap.Fields[h * TheMap.Info.MapWidth + w];
			CLprintf(file, "  {%3d, %3d,", mf->Tile, mf->SeenTile);
			if (mf->Value) {
				CLprintf(file, " %d,", mf->Value);
			}
			for (i = 0; i < PlayerMax; ++i) {
				if (mf->Visible[i] == 1) {
					CLprintf(file, " \"explored\", %d,", i);
				}
			}
			if (mf->Flags & MapFieldHuman) {
				CLprintf(file, " \"human\",");
			}
			if (mf->Flags & MapFieldLandAllowed) {
				CLprintf(file, " \"land\",");
			}
			if (mf->Flags & MapFieldCoastAllowed) {
				CLprintf(file, " \"coast\",");
			}
			if (mf->Flags & MapFieldWaterAllowed) {
				CLprintf(file, " \"water\",");
			}
			if (mf->Flags & MapFieldNoBuilding) {
				CLprintf(file, " \"mud\",");
			}
			if (mf->Flags & MapFieldUnpassable) {
				CLprintf(file, " \"block\",");
			}
			if (mf->Flags & MapFieldWall) {
				CLprintf(file, " \"wall\",");
			}
			if (mf->Flags & MapFieldRocks) {
				CLprintf(file, " \"rock\",");
			}
			if (mf->Flags & MapFieldForest) {
				CLprintf(file, " \"wood\",");
			}
#if 1
			// Not Required for save
			// These are required for now, UnitType::FieldFlags is 0 until
			// UpdateStats is called which is after the game is loaded
			if (mf->Flags & MapFieldLandUnit) {
				CLprintf(file, " \"ground\",");
			}
			if (mf->Flags & MapFieldAirUnit) {
				CLprintf(file, " \"air\",");
			}
			if (mf->Flags & MapFieldSeaUnit) {
				CLprintf(file, " \"sea\",");
			}
			if (mf->Flags & MapFieldBuilding) {
				CLprintf(file, " \"building\",");
			}
#endif
			if (w & 1) {
				CLprintf(file, "},\n");
			} else {
				CLprintf(file, "}, ");
			}
		}
	}
	CLprintf(file, "}})\n");
}

//@}
