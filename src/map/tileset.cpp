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
/**@name tileset.cpp - The tileset. */
//
//      (c) Copyright 1998-2005 by Lutz Sammer and Jimmy Salmon
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
#include "tileset.h"
#include "script.h"
#include "map.h"
#include "iolib.h"
#include "video.h"

/*----------------------------------------------------------------------------
-- Variables
----------------------------------------------------------------------------*/

/**
** Size of a tile in pixel
*/
PixelSize PixelTileSize = {32, 32};


/*----------------------------------------------------------------------------
-- Functions
----------------------------------------------------------------------------*/

/**
** Load tileset and setup ::Map for this tileset.
**
** @see Map @see Map.Tileset.
*/
void LoadTileset()
{
	//  Load and prepare the tileset
	PixelTileSize = Map.Tileset.PixelTileSize;

	ShowLoadProgress("Tileset `%s'", Map.Tileset.ImageFile.c_str());
	//Map.TileGraphic = CGraphic::New(Map.Tileset.ImageFile);
	Map.TileGraphic = CGraphic::New(Map.Tileset.ImageFile, PixelTileSize.x, PixelTileSize.y);
	Map.TileGraphic->Load();
}


/**
** Cleanup the tileset module.
**
** @note this didn't frees the configuration memory.
*/
void CleanTilesets()
{
	Map.Tileset.Clear();

	//
	// Should this be done by the map?
	//
	CGraphic::Free(Map.TileGraphic);
	Map.TileGraphic = NULL;
}

//@}
