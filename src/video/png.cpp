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
/**@name png.cpp - The png graphic file loader. */
//
//      (c) Copyright 1998-2011 by Lutz Sammer, Jimmy Salmon and Pali Rohár
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

#include "SDL_image.h"

#include "stratagus.h"
#include "map.h"
#include "video.h"
#include "iolib.h"

/*----------------------------------------------------------------------------
--  Variables
----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
--  Functions
----------------------------------------------------------------------------*/

/**
**  Save a screenshot to a PNG file.
**
**  @param name  PNG filename to save.
*/
void SaveScreenshotPNG(const char *name)
{
	IMG_SavePNG(TheScreen, name);
}

/**
**  Save a whole map to a PNG file.
**
**  @param name  PNG filename to save.
*/
void SaveMapPNG(const char *name)
{
	FILE *fp = fopen(name, "wb");
	if (fp == nullptr) {
		return;
	}

	const size_t imageWidth = Map.Info.MapWidth * PixelTileSize.x;
	const size_t imageHeight = Map.Info.MapHeight * PixelTileSize.y;

	SDL_Surface *mapImage = SDL_CreateRGBSurface(SDL_SWSURFACE,
		imageWidth, imageHeight, 32, RMASK, GMASK, BMASK, 0);

	for (int i = 0; i < Map.Info.MapHeight; ++i) {
		for (int j = 0; j < Map.Info.MapWidth; ++j) {
			const CMapField &mf = *Map.Field(i, j);
			SDL_Rect srcRect, dstRect;
			unsigned short int tile = mf.getGraphicTile();

			srcRect.x = Map.TileGraphic->frame_map[tile].x;
			srcRect.y = Map.TileGraphic->frame_map[tile].y;
			dstRect.x = i * PixelTileSize.x;
			dstRect.y = j * PixelTileSize.y;
			srcRect.w = dstRect.w = PixelTileSize.x;
			srcRect.h = dstRect.h = PixelTileSize.y;
			SDL_BlitSurface(Map.TileGraphic->Surface, &srcRect, mapImage, &dstRect);
		}
	}

	IMG_SavePNG(mapImage, name);
	SDL_FreeSurface(mapImage);
	fclose(fp);
}

//@}
