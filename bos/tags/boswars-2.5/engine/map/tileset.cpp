//     ____                _       __               
//    / __ )____  _____   | |     / /___ ___________
//   / __  / __ \/ ___/   | | /| / / __ `/ ___/ ___/
//  / /_/ / /_/ (__  )    | |/ |/ / /_/ / /  (__  ) 
// /_____/\____/____/     |__/|__/\__,_/_/  /____/  
//                                              
//       A futuristic real-time strategy game.
//          This file is part of Bos Wars.
//
/**@name tileset.cpp - The tileset. */
//
//      (c) Copyright 1998-2007 by Lutz Sammer and Jimmy Salmon
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
#include "tileset.h"
#include "map.h"
#include "video.h"

/*----------------------------------------------------------------------------
-- Variables
----------------------------------------------------------------------------*/

/**
** Size of a tile in X
*/
int TileSizeX = 32;

/**
** Size of a tile in Y
*/
int TileSizeY = 32;

/*----------------------------------------------------------------------------
-- Functions
----------------------------------------------------------------------------*/

/**
** Load tileset and setup ::Map for this tileset.
**
** @see Map @see Map.Tileset.
*/
void LoadTileset(void)
{
	//  Load and prepare the tileset
	TileSizeX = Map.Tileset.TileSizeX;
	TileSizeY = Map.Tileset.TileSizeY;

	ShowLoadProgress("Tileset `%s'", Map.Tileset.ImageFile.c_str());
	//Map.TileGraphic = CGraphic::New(Map.Tileset.ImageFile);
	Map.TileGraphic = CGraphic::New(Map.Tileset.ImageFile, TileSizeX, TileSizeY);
	Map.TileGraphic->Load();
}


/**
** Cleanup the tileset module.
**
** @note this didn't frees the configuration memory.
*/
void CleanTilesets(void)
{
	Map.Tileset.Clear();

	//
	// Should this be done by the map?
	//
	CGraphic::Free(Map.TileGraphic);
	Map.TileGraphic = NULL;
}

//@}
