//     ____                _       __               
//    / __ )____  _____   | |     / /___ ___________
//   / __  / __ \/ ___/   | | /| / / __ `/ ___/ ___/
//  / /_/ / /_/ (__  )    | |/ |/ / /_/ / /  (__  ) 
// /_____/\____/____/     |__/|__/\__,_/_/  /____/  
//                                              
//       A futuristic real-time strategy game.
//          This file is part of Bos Wars.
//
/**@name map_save.cpp - Saving the map. */
//
//      (c) Copyright 2001-2007 by Lutz Sammer and Jimmy Salmon
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
#include "patch_manager.h"
#include "iolib.h"
#include "version.h"

/*----------------------------------------------------------------------------
-- Variables
----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
-- Functions
----------------------------------------------------------------------------*/

/**
**  Save the complete map.
**
**  @param file  Output file.
*/
void CMap::Save(CFile *file) const
{
	file->printf("\n--- -----------------------------------------\n");
	file->printf("--- MODULE: map\n\n");

	file->printf("StratagusMap(\n");

	file->printf("  \"version\", \"" StratagusFormatString "\",\n",
		StratagusFormatArgs(StratagusVersion));
	file->printf("  \"description\", \"%s\",\n", this->Info.Description.c_str());

	file->printf("  \"the-map\", {\n");

	file->printf("  \"size\", {%d, %d},\n", this->Info.MapWidth, this->Info.MapHeight);
	file->printf("  \"%s\",\n", this->NoFogOfWar ? "no-fog-of-war" : "fog-of-war");
	file->printf("  \"filename\", \"%s\",\n", this->Info.Filename.c_str());

	file->printf("  \"map-fields\", {\n");
	for (int h = 0; h < this->Info.MapHeight; ++h) {
		file->printf("  -- %d\n", h);
		for (int w = 0; w < this->Info.MapWidth; ++w) {
			CMapField* mf;

			mf = this->Field(w, h);
			file->printf("  {");
			for (int i = 0; i < PlayerMax; ++i) {
				if (mf->Visible[i] == 1) {
					file->printf(" \"explored\", %d,", i);
				}
			}
			if (mf->Flags & MapFieldLandAllowed) {
				file->printf(" \"land\",");
			}
			if (mf->Flags & MapFieldCoastAllowed) {
				file->printf(" \"coast\",");
			}
			if (mf->Flags & MapFieldShallowWater) {
				file->printf(" \"pond\",");
			}
			if (mf->Flags & MapFieldDeepWater) {
				file->printf(" \"water\",");
			}
			if (mf->Flags & MapFieldNoBuilding) {
				file->printf(" \"mud\",");
			}
			if (mf->Flags & MapFieldUnpassable) {
				file->printf(" \"block\",");
			}
#if 1
			// Not Required for save
			// These are required for now, UnitType::FieldFlags is 0 until
			// UpdateStats is called which is after the game is loaded
			if (mf->Flags & MapFieldLandUnit) {
				file->printf(" \"ground\",");
			}
			if (mf->Flags & MapFieldAirUnit) {
				file->printf(" \"air\",");
			}
			if (mf->Flags & MapFieldSeaUnit) {
				file->printf(" \"sea\",");
			}
			if (mf->Flags & MapFieldBuilding) {
				file->printf(" \"building\",");
			}
#endif
			if (w & 1) {
				file->printf("},\n");
			} else {
				file->printf("}, ");
			}
		}
	}
	file->printf("}})\n");
}

//@}
