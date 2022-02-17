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
/**@name editor.cpp - Editor functions. */
//
//      (c) Copyright 2002-2005 by Lutz Sammer
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

//----------------------------------------------------------------------------
//  Documentation
//----------------------------------------------------------------------------

/**
**  @page EditorModule Module - Editor
**
**  This is a very simple editor for the Stratagus engine.
**
**  @section Missing Missing features
**
**    @li Edit upgrade section
**    @li Edit allow section
**    @li Edit .cm files
**    @li Upgraded unit-types should be shown different on map
**    @li Good keyboard bindings
**    @li Script support
**    @li Commandline support
**    @li Cut&Paste
**    @li More random map functions.
*/

/*----------------------------------------------------------------------------
--  Includes
----------------------------------------------------------------------------*/

#include "stratagus.h"

#include "editor.h"

#include "player.h"

/*----------------------------------------------------------------------------
--  Defines
----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
--  Variables
----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
--  Functions
----------------------------------------------------------------------------*/

CEditor::CEditor() :
	TerrainEditable(true),
	StartUnit(NULL),
	UnitIndex(0), CursorUnitIndex(-1), SelectedUnitIndex(-1),
	TileIndex(0), CursorTileIndex(-1), SelectedTileIndex(-1),
	CursorPlayer(-1), SelectedPlayer(PlayerNumNeutral),
	MapLoaded(false), WriteCompressedMaps(true), PopUpX(-1), PopUpY(-1)
{
	
#define WATER_TILE  0x10
#define COAST_TILE  0x30
#define GRASS_TILE  0x50
#define WOOD_TILE   0x70
#define ROCK_TILE   0x80
	BaseTileIndex = WATER_TILE;

	RandomTiles.push_back(std::make_tuple(COAST_TILE, 2, 16));
	RandomTiles.push_back(std::make_tuple(GRASS_TILE, 4, 16));
	RandomTiles.push_back(std::make_tuple(WOOD_TILE, 12, 4));
	RandomTiles.push_back(std::make_tuple(ROCK_TILE, 4, 2));

	RandomUnits.push_back(std::make_tuple("unit-gold-mine", 1, 50000, GRASS_TILE));
	RandomUnits.push_back(std::make_tuple("unit-oil-patch", 1, 20000, WATER_TILE));
}

//@}
