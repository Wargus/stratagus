//       _________ __                 __
//      /   _____//  |_____________ _/  |______     ____  __ __  ______
//      \_____  \\   __\_  __ \__  \\   __\__  \   / ___\|  |  \/  ___/
//      /        \|  |  |  | \// __ \|  |  / __ \_/ /_/  >  |  /\___ |
//     /_______  /|__|  |__|  (____  /__| (____  /\___  /|____//____  >
//             \/                  \/          \//_____/            \/
//  ______________________                           ______________________
//			  T H E   W A R   B E G I N S
//	   Stratagus - A free fantasy real time strategy game engine
//
/**@name minimap.c	-	The minimap. */
//
//	(c) Copyright 1998-2003 by Lutz Sammer and Jimmy Salmon
//
//      This program is free software; you can redistribute it and/or modify
//      it under the terms of the GNU General Public License as published by
//      the Free Software Foundation; version 2 dated June, 1991.
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
//	$Id$

//@{

/*----------------------------------------------------------------------------
--		Includes
----------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "stratagus.h"
#include "video.h"
#include "tileset.h"
#include "map.h"
#include "minimap.h"
#include "unittype.h"
#include "player.h"
#include "unit.h"
#include "ui.h"

/*----------------------------------------------------------------------------
--		Variables
----------------------------------------------------------------------------*/

local SDL_Surface* MinimapSurface;		/// generated minimap
local SDL_Surface* MinimapTerrainSurface;		/// generated minimap terrain
local int* Minimap2MapX;				/// fast conversion table
local int* Minimap2MapY;				/// fast conversion table
local int Map2MinimapX[MaxMapWidth];		/// fast conversion table
local int Map2MinimapY[MaxMapHeight];		/// fast conversion table

//		MinimapScale:
//		32x32 64x64 96x96 128x128 256x256 512x512 ...
//		  *4	*2	*4/3   *1			 *1/2	*1/4
local int MinimapScaleX;				/// Minimap scale to fit into window
local int MinimapScaleY;				/// Minimap scale to fit into window
global int MinimapX;						/// Minimap drawing position x offset
global int MinimapY;						/// Minimap drawing position y offset

global int MinimapWithTerrain = 1;		/// display minimap with terrain
global int MinimapFriendly = 1;				/// switch colors of friendly units
global int MinimapShowSelected = 1;		/// highlight selected units

local int OldMinimapCursorX;				/// Save MinimapCursorX
local int OldMinimapCursorY;				/// Save MinimapCursorY
local int OldMinimapCursorW;				/// Save MinimapCursorW
local int OldMinimapCursorH;				/// Save MinimapCursorH
local int OldMinimapCursorSize;				/// Saved image size

local SDL_Surface* OldMinimapCursorImage;		/// Saved image behind cursor

/**
**		Create a mini-map from the tiles of the map.
**
**		@todo 		Scaling and scrolling the minmap is currently not supported.
*/
global void CreateMinimap(void)
{
	int n;
	SDL_Rect srect;

	if (TheMap.Width > TheMap.Height) {		// Scale to biggest value.
		n = TheMap.Width;
	} else {
		n = TheMap.Height;
	}
	MinimapScaleX = (TheUI.MinimapW * MINIMAP_FAC + n - 1) / n;
	MinimapScaleY = (TheUI.MinimapH * MINIMAP_FAC + n - 1) / n;

	MinimapX = ((TheUI.MinimapW * MINIMAP_FAC) / MinimapScaleX - TheMap.Width) / 2;
	MinimapY = ((TheUI.MinimapH * MINIMAP_FAC) / MinimapScaleY - TheMap.Height) / 2;
	MinimapX = (TheUI.MinimapW - (TheMap.Width * MinimapScaleX) / MINIMAP_FAC) / 2;
	MinimapY = (TheUI.MinimapH - (TheMap.Height * MinimapScaleY) / MINIMAP_FAC) / 2;

	DebugLevel0Fn("MinimapScale %d %d (%d %d), X off %d, Y off %d\n" _C_
		MinimapScaleX / MINIMAP_FAC _C_ MinimapScaleY / MINIMAP_FAC _C_
		MinimapScaleX _C_ MinimapScaleY _C_
		MinimapX _C_ MinimapY);

	//
	//		Calculate minimap fast lookup tables.
	//
	// FIXME: this needs to be recalculated during map load - the map size
	// might have changed!
	Minimap2MapX = calloc(sizeof(int), TheUI.MinimapW * TheUI.MinimapH);
	Minimap2MapY = calloc(sizeof(int), TheUI.MinimapW * TheUI.MinimapH);
	for (n = MinimapX; n < TheUI.MinimapW - MinimapX; ++n) {
		Minimap2MapX[n] = ((n - MinimapX) * MINIMAP_FAC) / MinimapScaleX;
	}
	for (n = MinimapY; n < TheUI.MinimapH - MinimapY; ++n) {
		Minimap2MapY[n] = (((n - MinimapY) * MINIMAP_FAC) / MinimapScaleY) * TheMap.Width;
	}
	for (n = 0; n < TheMap.Width; ++n) {
		Map2MinimapX[n] = (n * MinimapScaleX) / MINIMAP_FAC;
	}
	for (n = 0; n < TheMap.Height; ++n) {
		Map2MinimapY[n] = (n * MinimapScaleY) / MINIMAP_FAC;
	}

	// Palette updated from UpdateMinimapTerrain()
	MinimapTerrainSurface = SDL_CreateRGBSurface(SDL_SWSURFACE, TheUI.MinimapW,
		TheUI.MinimapH, 8, 0, 0, 0, 0);
	MinimapSurface = SDL_CreateRGBSurface(SDL_SWSURFACE, TheUI.MinimapW,
		TheUI.MinimapH, 8, 0, 0, 0, 0);

	srect.x = TheUI.MinimapPosX - TheUI.MinimapPanelX;
	srect.y = TheUI.MinimapPosY - TheUI.MinimapPanelY;
	srect.w = TheUI.MinimapW;
	srect.h = TheUI.MinimapH;
	if (TheUI.MinimapPanel.Graphic) {
		SDL_BlitSurface(TheUI.MinimapPanel.Graphic->Surface, &srect,
			MinimapSurface, NULL);
	}

	if (!TheUI.MinimapTransparent) {
		VideoFillRectangle(ColorBlack, MinimapX, MinimapY,
			TheUI.MinimapW, TheUI.MinimapH);
	}

	UpdateMinimapTerrain();
}

/**
**		Update a mini-map from the tiles of the map.
**
**		@todo		FIXME: this is not correct should use SeenTile.
**
**		FIXME: this can surely be sped up??
*/
global void UpdateMinimapTerrain(void)
{
	int mx;
	int my;
	int scalex;
	int scaley;
	int tilepitch;
	int xofs;
	int yofs;

	if (!(scalex = (MinimapScaleX / MINIMAP_FAC))) {
		scalex = 1;
	}
	if (!(scaley = (MinimapScaleY / MINIMAP_FAC))) {
		scaley = 1;
	}

	SDL_SetPalette(MinimapTerrainSurface, SDL_LOGPAL,
		TheMap.TileGraphic->Surface->format->palette->colors, 0, 256);
	SDL_SetPalette(MinimapSurface, SDL_LOGPAL,
		TheMap.TileGraphic->Surface->format->palette->colors, 0, 256);

	tilepitch = TheMap.TileGraphic->Surface->w / TileSizeX;

	SDL_LockSurface(MinimapTerrainSurface);
	SDL_LockSurface(TheMap.TileGraphic->Surface);
	//
	//		Pixel 7,6 7,14, 15,6 15,14 are taken for the minimap picture.
	//
	for (my = MinimapY; my < TheUI.MinimapH - MinimapY; ++my) {
		for (mx = MinimapX; mx < TheUI.MinimapW - MinimapX; ++mx) {
			int tile;

			tile = TheMap.Fields[Minimap2MapX[mx] + Minimap2MapY[my]].Tile;

			xofs = TileSizeX * (tile % tilepitch);
			yofs = TileSizeY * (tile / tilepitch);

			((Uint8*)MinimapTerrainSurface->pixels)[mx + my * MinimapTerrainSurface->pitch] =
				((Uint8*)TheMap.TileGraphic->Surface->pixels)
					[xofs + 7 + (mx % scalex) * 8 + (yofs + 6 + (my % scaley) * 8)
					* TheMap.TileGraphic->Surface->pitch];
		}
	}
	SDL_UnlockSurface(MinimapTerrainSurface);
	SDL_UnlockSurface(TheMap.TileGraphic->Surface);
}

// FIXME: todo
global void UpdateMinimapXY(int tx, int ty)
{
	int mx;
	int my;
	int x;
	int y;
	int scalex;
	int scaley;
	int xofs;
	int yofs;
	int tilepitch;

	scalex = MinimapScaleX / MINIMAP_FAC;
	if (scalex == 0) {
		scalex = 1;
	}
	scaley = MinimapScaleY / MINIMAP_FAC;
	if (scaley == 0) {
		scaley = 1;
	}

	tilepitch = TheMap.TileGraphic->Surface->w / TileSizeX;

	//
	//		Pixel 7,6 7,14, 15,6 15,14 are taken for the minimap picture.
	//
	SDL_LockSurface(TheMap.TileGraphic->Surface);
	SDL_LockSurface(MinimapTerrainSurface);

	ty *= TheMap.Width;
	for (my = MinimapY; my < TheUI.MinimapH - MinimapY; ++my) {
		y = Minimap2MapY[my];
		if (y < ty) {
			continue;
		}
		if (y > ty) {
			break;
		}

		for (mx = MinimapX; mx < TheUI.MinimapW - MinimapX; ++mx) {
			int tile;

			x = Minimap2MapX[mx];
			if (x < tx) {
				continue;
			}
			if (x > tx) {
				break;
			}

			tile = TheMap.Fields[x + y].Tile;

			xofs = TileSizeX * (tile % tilepitch);
			yofs = TileSizeY * (tile / tilepitch);

			((Uint8*)MinimapTerrainSurface->pixels)[mx + my * MinimapTerrainSurface->pitch] =
				((Uint8*)TheMap.TileGraphic->Surface->pixels)
					[xofs + 7 + (mx % scalex) * 8 + (yofs + 6 + (my % scaley) * 8)
					* TheMap.TileGraphic->Surface->pitch];
		}
	}
	SDL_UnlockSurface(TheMap.TileGraphic->Surface);
	SDL_UnlockSurface(MinimapTerrainSurface);
}

/**
**      Draw an unit on the minimap. 
*/
local void DrawUnitOnMinimap(Unit* unit, int red_phase)
{
	UnitType* type;
	int mx;
	int my;
	int w;
	int h;
	int h0;
	Uint32 color;
	SDL_Color c;

	if (!UnitVisibleOnMinimap(unit)) {
		return ;
	}
	type = unit->Type;
	//
	//  FIXME: We should force unittypes to have a certain color on the minimap.
	//
	if (unit->Player->Player == PlayerNumNeutral) {
		color = VideoMapRGB(TheScreen->format,
			unit->Type->NeutralMinimapColorRGB.r,
			unit->Type->NeutralMinimapColorRGB.g,
			unit->Type->NeutralMinimapColorRGB.b);
	} else if (unit->Player == ThisPlayer) {
		if (unit->Attacked && unit->Attacked + ATTACK_BLINK_DURATION > GameCycle &&
				(red_phase || unit->Attacked + ATTACK_RED_DURATION > GameCycle)) {
			color = ColorRed;
		} else if (MinimapShowSelected && unit->Selected) {
			color = ColorWhite;
		} else {
			color = ColorGreen;
		}
	} else {
		color = unit->Player->Color;
	}

	mx = 1 + MinimapX + Map2MinimapX[unit->X];
	my = 1 + MinimapY + Map2MinimapY[unit->Y];
	w = Map2MinimapX[type->TileWidth];
	if (mx + w >= TheUI.MinimapW) {				// clip right side
		w = TheUI.MinimapW - mx;
	}
	h0 = Map2MinimapY[type->TileHeight];
	if (my + h0 >= TheUI.MinimapH) {		// clip bottom side
		h0 = TheUI.MinimapH - my;
	}
	while (w-- >= 0) {
		h = h0;
		while (h-- >= 0) {
			SDL_GetRGB(color, TheScreen->format, &c.r, &c.g, &c.b);
			((Uint8*)MinimapSurface->pixels)[mx + w + (my + h) * MinimapSurface->pitch] =
				VideoMapRGB(MinimapSurface->format, c.r, c.g, c.b);
		}
	}
}

global void UpdateMinimap(void)
{
	static int red_phase;
	int red_phase_changed;
	int mx;
	int my;
	int n;
	Unit* table[UnitMax];
	int visiontype; // 0 unexplored, 1 explored, >1 visible.

	red_phase_changed = red_phase != (int)((FrameCounter / FRAMES_PER_SECOND) & 1);
	if (red_phase_changed) {
		red_phase = !red_phase;
	}

	SDL_LockSurface(MinimapSurface);
	SDL_LockSurface(MinimapTerrainSurface);
	//
	//		Draw the terrain
	//
	for (my = 0; my < TheUI.MinimapH; ++my) {
		for (mx = 0; mx < TheUI.MinimapW; ++mx) {
			if (ReplayRevealMap) {
				visiontype = 2;
			} else {
				visiontype = IsTileVisible(ThisPlayer, Minimap2MapX[mx], Minimap2MapY[my] / TheMap.Width);
			}

			if (MinimapWithTerrain && (visiontype > 1 || (visiontype == 1 && ((mx & 1) == (my & 1))))) {
				((Uint8*)MinimapSurface->pixels)[mx + my * MinimapSurface->pitch] =
					((Uint8*)MinimapTerrainSurface->pixels)[mx + my * MinimapTerrainSurface->pitch];
			} else if (visiontype > 0) {
				((Uint8*)MinimapSurface->pixels)[mx + my * MinimapSurface->pitch] =
					VideoMapRGB(MinimapSurface->format, 0, 0, 0);
			}
		}
	}
	SDL_UnlockSurface(MinimapTerrainSurface);

	//
	//		Draw units on map
	//		FIXME: We should rewrite this completely
	//
	n = UnitCacheSelect(0, 0, TheMap.Height, TheMap.Width, table);
	while (n--) {
		DrawUnitOnMinimap(table[n], red_phase);
	}

	SDL_UnlockSurface(MinimapSurface);
}

global void DrawMinimap(int vx __attribute__((unused)),
		int vy __attribute__((unused)))
{
	SDL_Rect drect;

	drect.x = TheUI.MinimapPosX;
	drect.y = TheUI.MinimapPosY;

	SDL_BlitSurface(MinimapSurface, NULL, TheScreen, &drect);
}


/**
**		Convert minimap cursor X position to tile map coordinate.
**
**		@param x		Screen X pixel coordinate.
**		@return				Tile X coordinate.
*/
global int ScreenMinimap2MapX(int x)
{
	int tx;

	tx = (((x - TheUI.MinimapPosX - MinimapX) * MINIMAP_FAC) / MinimapScaleX);
	if (tx < 0) {
		return 0;
	}
	return tx < TheMap.Width ? tx : TheMap.Width - 1;
}

/**
**		Convert minimap cursor Y position to tile map coordinate.
**
**		@param y		Screen Y pixel coordinate.
**		@return				Tile Y coordinate.
*/
global int ScreenMinimap2MapY(int y)
{
	int ty;

	ty = (((y - TheUI.MinimapPosY - MinimapY) * MINIMAP_FAC) / MinimapScaleY);
	if (ty < 0) {
		return 0;
	}
	return ty < TheMap.Height ? ty : TheMap.Height - 1;
}

/**
**		Destroy mini-map.
*/
global void DestroyMinimap(void)
{
	SDL_FreeSurface(MinimapTerrainSurface);
	MinimapTerrainSurface = NULL;
	if (MinimapSurface) {
		SDL_FreeSurface(MinimapSurface);
		MinimapSurface = NULL;
	}
	free(Minimap2MapX);
	Minimap2MapX = NULL;
	free(Minimap2MapY);
	Minimap2MapY = NULL;
}

/**
**		Hide minimap cursor.
*/
global void HideMinimapCursor(void)
{
	if (OldMinimapCursorW) {
		LoadCursorRectangle(OldMinimapCursorImage,
			OldMinimapCursorX, OldMinimapCursorY,
			OldMinimapCursorW, OldMinimapCursorH);
		OldMinimapCursorW = 0;
	}
}

/**
**		Draw minimap cursor.
**
**		@param vx		View point X position.
**		@param vy		View point Y position.
*/
global void DrawMinimapCursor(int vx, int vy)
{
	int x;
	int y;
	int w;
	int h;
	int i;

	// Determine and save region below minimap cursor
	OldMinimapCursorX = x =
		TheUI.MinimapPosX + MinimapX + (vx * MinimapScaleX) / MINIMAP_FAC;
	OldMinimapCursorY = y =
		TheUI.MinimapPosY + MinimapY + (vy * MinimapScaleY) / MINIMAP_FAC;
	OldMinimapCursorW = w =
		(TheUI.SelectedViewport->MapWidth * MinimapScaleX) / MINIMAP_FAC;
	OldMinimapCursorH = h =
		(TheUI.SelectedViewport->MapHeight * MinimapScaleY) / MINIMAP_FAC;

	i = (w + 1 + h) * 2 * TheScreen->format->BytesPerPixel;

	if (OldMinimapCursorSize < i) {
		if (OldMinimapCursorImage) {
			OldMinimapCursorImage = realloc(OldMinimapCursorImage, i);
		} else {
			OldMinimapCursorImage = malloc(i);
		}
		DebugLevel3("Cursor memory %d\n" _C_ i);
		OldMinimapCursorSize = i;
	}
	SaveCursorRectangle(OldMinimapCursorImage, x, y, w, h);

	// Draw cursor as rectangle (Note: unclipped, as it is always visible)
	VideoDrawTransRectangle(TheUI.ViewportCursorColor, x, y, w, h, 128);
}

//@}
