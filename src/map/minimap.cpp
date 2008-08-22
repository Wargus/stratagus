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
/**@name minimap.cpp - The minimap. */
//
//      (c) Copyright 1998-2006 by Lutz Sammer and Jimmy Salmon
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
#include "video.h"
#include "tileset.h"
#include "map.h"
#include "minimap.h"
#include "unittype.h"
#include "player.h"
#include "unit.h"
#include "ui.h"
#include "editor.h"

#include "../video/renderer.h"

/*----------------------------------------------------------------------------
--  Defines
----------------------------------------------------------------------------*/

#define MINIMAP_FAC (16 * 3)  /// integer scale factor

	/// unit attacked are shown red for at least this amount of cycles
#define ATTACK_RED_DURATION (1 * CYCLES_PER_SECOND)
	/// unit attacked are shown blinking for this amount of cycles
#define ATTACK_BLINK_DURATION (7 * CYCLES_PER_SECOND)


/*----------------------------------------------------------------------------
--  Variables
----------------------------------------------------------------------------*/

#ifdef USE_OPENGL
static GLuint MinimapTexture;
static unsigned char *MinimapSurface;
static unsigned char *MinimapTerrainSurface;
static int MinimapTextureWidth;
static int MinimapTextureHeight;
#else
static SDL_Surface *MinimapSurface;        /// generated minimap
static SDL_Surface *MinimapTerrainSurface; /// generated minimap terrain
#endif
static int *Minimap2MapX;                  /// fast conversion table
static int *Minimap2MapY;                  /// fast conversion table
static int Map2MinimapX[MaxMapWidth];      /// fast conversion table
static int Map2MinimapY[MaxMapHeight];     /// fast conversion table

// MinimapScale:
// 32x32 64x64 96x96 128x128 256x256 512x512 ...
// *4 *2 *4/3   *1 *1/2 *1/4
static int MinimapScaleX;                  /// Minimap scale to fit into window
static int MinimapScaleY;                  /// Minimap scale to fit into window

#define MAX_MINIMAP_EVENTS 8

struct MinimapEvent {
	int X;
	int Y;
	int Size;
} MinimapEvents[MAX_MINIMAP_EVENTS];
int NumMinimapEvents;

//#define TERRAIN_IN_DISPLAY_FORMAT

/*----------------------------------------------------------------------------
-- Functions
----------------------------------------------------------------------------*/


#ifdef USE_OPENGL
/**
**  Create the minimap texture
*/
static void CreateMinimapTexture(void)
{
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glGenTextures(1, &MinimapTexture);
	glBindTexture(GL_TEXTURE_2D, MinimapTexture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, MinimapTextureWidth,
		MinimapTextureHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE,
		MinimapSurface);
}
#endif

/**
**  Create a mini-map from the tiles of the map.
**
**  @todo Scaling and scrolling the minmap is currently not supported.
*/
void CMinimap::Create(void)
{
	int n;
	int maxSize = std::max(Map.Info.MapWidth, Map.Info.MapHeight);

	MinimapScaleX = (W * MINIMAP_FAC + maxSize - 1) / maxSize;
	MinimapScaleY = (H * MINIMAP_FAC + maxSize - 1) / maxSize;

	XOffset = (W - (Map.Info.MapWidth * MinimapScaleX) / MINIMAP_FAC + 1) / 2;
	YOffset = (H - (Map.Info.MapHeight * MinimapScaleY) / MINIMAP_FAC + 1) / 2;

	DebugPrint("MinimapScale %d %d (%d %d), X off %d, Y off %d\n" _C_
		MinimapScaleX / MINIMAP_FAC _C_ MinimapScaleY / MINIMAP_FAC _C_
		MinimapScaleX _C_ MinimapScaleY _C_
		XOffset _C_ YOffset);

	//
	// Calculate minimap fast lookup tables.
	//
	// FIXME: this needs to be recalculated during map load - the map size
	// might have changed!
	Minimap2MapX = new int[W];
	memset(Minimap2MapX, 0, W * sizeof(int));
	Minimap2MapY = new int[H];
	memset(Minimap2MapY, 0, H * sizeof(int));
	for (n = XOffset; n < W - XOffset; ++n) {
		Minimap2MapX[n] = ((n - XOffset) * MINIMAP_FAC) / MinimapScaleX;
	}
	for (n = YOffset; n < H - YOffset; ++n) {
		Minimap2MapY[n] = (((n - YOffset) * MINIMAP_FAC) / MinimapScaleY) * Map.Info.MapWidth;
	}
	for (n = 0; n < Map.Info.MapWidth; ++n) {
		Map2MinimapX[n] = (n * MinimapScaleX) / MINIMAP_FAC;
	}
	for (n = 0; n < Map.Info.MapHeight; ++n) {
		Map2MinimapY[n] = (n * MinimapScaleY) / MINIMAP_FAC;
	}

	// Palette updated from UpdateMinimapTerrain()
#ifndef USE_OPENGL
	SDL_PixelFormat *f = Map.TileGraphic->Surface->format;
	MinimapTerrainSurface = SDL_CreateRGBSurface(SDL_SWSURFACE,
		W, H, f->BitsPerPixel, f->Rmask, f->Gmask, f->Bmask, f->Amask);
	MinimapSurface = SDL_CreateRGBSurface(SDL_SWSURFACE,
		W, H, TheScreen->format->BitsPerPixel, 
		TheScreen->format->Rmask, TheScreen->format->Gmask,
		TheScreen->format->Bmask, 0);
#else
	for (MinimapTextureWidth = 1; MinimapTextureWidth < W; MinimapTextureWidth <<= 1) {
	}
	for (MinimapTextureHeight = 1; MinimapTextureHeight < H; MinimapTextureHeight <<= 1) {
	}
	MinimapTerrainSurface = new unsigned char[MinimapTextureWidth * MinimapTextureHeight * 4];
	MinimapSurface = new unsigned char[MinimapTextureWidth * MinimapTextureHeight * 4];
	memset(MinimapSurface, 0, MinimapTextureWidth * MinimapTextureHeight * 4);
	CreateMinimapTexture();
#endif

	UpdateTerrain();

	NumMinimapEvents = 0;
}

#ifdef USE_OPENGL
/**
**  Free OpenGL minimap
*/
void CMinimap::FreeOpenGL(void)
{
	glDeleteTextures(1, &MinimapTexture);
}

/**
**  Reload OpenGL minimap
*/
void CMinimap::Reload(void)
{
	CreateMinimapTexture();
}
#endif

/**
**  Clear the minimap
*/
static void ClearMinimap()
{
#ifndef USE_OPENGL
		SDL_FillRect(MinimapSurface, NULL, 
				SDL_MapRGB(MinimapSurface->format, 0, 0, 0));
#else
		memset(MinimapSurface, 0, MinimapTextureWidth * MinimapTextureHeight * 4);
#endif
}

/**
**  Copy the minimap terrain to the minimap
*/
static void CopyMinimapTerrain()
{
#ifndef USE_OPENGL
		SDL_BlitSurface(MinimapTerrainSurface, NULL, MinimapSurface, NULL);
#else
		memcpy(MinimapSurface, MinimapTerrainSurface,
			 MinimapTextureWidth * MinimapTextureHeight * 4);
#endif
}


template <const int BPP>
void CMinimap::UpdateMapTerrain(
			void *const mpixels, const int mpitch,
			const void *const tpixels, const int tpitch)
{
	int tile, mx, my, xofs, yofs;
	int scalex = MinimapScaleX / MINIMAP_FAC;
	int scaley = MinimapScaleY / MINIMAP_FAC;

	if (scalex == 0) {
		scalex = 1;
	}
	if (scaley == 0) {
		scaley = 1;
	}

	//
	//  Pixel 7,6 7,14, 15,6 15,14 are taken for the minimap picture.
	//
	unsigned int mindex = YOffset * mpitch;
	for (my = YOffset; my < H - YOffset; ++my) {
		unsigned int tindex = (6 + (my % scaley) * 8) * tpitch;
		for (mx = XOffset; mx < W - XOffset; ++mx) {

			tile = Map.Fields[Minimap2MapX[mx] + Minimap2MapY[my]].Tile;

			xofs = Map.TileGraphic->frame_map[tile].x + 7 + (mx % scalex) * 8;
			yofs = Map.TileGraphic->frame_map[tile].y * tpitch + tindex;

			if(BPP == 1) {
#ifndef USE_OPENGL
					DRAW::PutPixel<1>(mpixels, mx + mindex, 
						DRAW::GetPixel<1>(tpixels, xofs + yofs));
#else
					SDL_Color color =
						 Map.TileGraphic->Surface->format->palette->colors[
								DRAW::GetPixel<1>(tpixels, xofs + yofs)];
					DRAW::PutPixel<4>(mpixels, mx + mindex, 
						Video.MapRGB(0, color.r, color.g, color.b));			
#endif
				} else if (BPP == 2) {
#ifndef USE_OPENGL
					DRAW::PutPixel<2>(mpixels, mx + mindex, 
						DRAW::GetPixel<2>(tpixels, xofs + yofs));
#else
					SDL_PixelFormat *f = Map.TileGraphic->Surface->format;
					Uint32 c = DRAW::GetPixel<2>(tpixels, xofs + yofs);
					DRAW::PutPixel<4>(mpixels, mx + mindex, 
						Video.MapRGB(0,
							((c & f->Rmask) >> f->Rshift),
							((c & f->Gmask) >> f->Gshift),
							((c & f->Bmask) >> f->Bshift)));	
#endif
				} else if (BPP == 4) {
					DRAW::PutPixel<4>(mpixels, mx + mindex, 
						DRAW::GetPixel<4>(tpixels, xofs + yofs));
				} else	Assert(0);
		}
		mindex += mpitch;
	}
}

/**
**  Update a mini-map from the tiles of the map.
**
**  FIXME: this can surely be sped up??
*/
void CMinimap::UpdateTerrain(void)
{
	const int bpp = Map.TileGraphic->Surface->format->BytesPerPixel;
	const int tpitch = Map.TileGraphic->Surface->pitch / bpp;
	
#ifndef USE_OPENGL
	if (bpp == 1) {
		SDL_SetPalette(MinimapTerrainSurface, SDL_LOGPAL,
			Map.TileGraphic->Surface->format->palette->colors, 0, 256);
	}

	Assert(bpp == MinimapTerrainSurface->format->BytesPerPixel);

	SDL_LockSurface(MinimapTerrainSurface);
	
	void *const mpixels = MinimapTerrainSurface->pixels;
	const int mpitch = MinimapTerrainSurface->pitch / bpp;
#else
	void *const mpixels = (void *)MinimapTerrainSurface;
	const int mpitch = MinimapTextureWidth;
#endif

	SDL_LockSurface(Map.TileGraphic->Surface);
	
	const void *const tpixels = Map.TileGraphic->Surface->pixels;
	

	switch (bpp) {
		case 1:
			UpdateMapTerrain<1>(mpixels, mpitch, tpixels, tpitch);
		break;
		case 2:
			UpdateMapTerrain<2>(mpixels, mpitch, tpixels, tpitch);
		break;
		case 4:
			UpdateMapTerrain<4>(mpixels, mpitch, tpixels, tpitch);
		break;
		default:
			Assert(0);
		break;	
	}

	SDL_UnlockSurface(Map.TileGraphic->Surface);
#ifndef USE_OPENGL
	SDL_UnlockSurface(MinimapTerrainSurface);
	
#ifdef TERRAIN_IN_DISPLAY_FORMAT	
	SDL_Surface *s = MinimapTerrainSurface;
	if (s->format->Amask != 0) {
		MinimapTerrainSurface = SDL_DisplayFormatAlpha(s);
	} else {
		MinimapTerrainSurface = SDL_DisplayFormat(s);
	}
	SDL_FreeSurface(s);
#endif

#endif
}

/**
**  Update a single minimap tile after a change
**
**  @param tx  The X map position to update in the minimap
**  @param ty  The Y map position to update in the minimap
*/
void CMinimap::UpdateXY(int tx, int ty)
{

	if (!MinimapTerrainSurface) {
		return;
	}
	//DebugPrint("Ran Update Minimap on [%d,%d], beware.\n" _C_ tx _C_ ty);

	int tile;
	int mx,x;
	int my,y;
	int xofs;
	int yofs;
	int tbpp = Map.TileGraphic->Surface->format->BytesPerPixel;
	int tpitch = Map.TileGraphic->Surface->pitch / tbpp;
#ifdef TERRAIN_IN_DISPLAY_FORMAT	
	Uint32 c;
#endif
	int scalex = MinimapScaleX / MINIMAP_FAC;
	int scaley = MinimapScaleY / MINIMAP_FAC;
	if (scalex == 0) {
		scalex = 1;
	}
	if (scaley == 0) {
		scaley = 1;
	}

#ifndef USE_OPENGL
	SDL_LockSurface(MinimapTerrainSurface);
	void *mpixels = MinimapTerrainSurface->pixels;	
#ifdef TERRAIN_IN_DISPLAY_FORMAT
	int mbpp = MinimapTerrainSurface->format->BytesPerPixel;
	int mpitch = MinimapTerrainSurface->pitch / mbpp;
#else
	Assert(tbpp == MinimapTerrainSurface->format->BytesPerPixel);
	int mpitch = MinimapTerrainSurface->pitch / tbpp;
#endif

#else

	void *mpixels = (void *)MinimapTerrainSurface;
	int mpitch = MinimapTextureWidth;
	
#endif

	SDL_LockSurface(Map.TileGraphic->Surface);
	void *tpixels = Map.TileGraphic->Surface->pixels;
	
	//
	//  Pixel 7,6 7,14, 15,6 15,14 are taken for the minimap picture.
	//
	ty *= Map.Info.MapWidth;
	unsigned int mindex = YOffset * mpitch;
	for (my = YOffset; my < H - YOffset; ++my) {
	
		y = Minimap2MapY[my];
		if (y < ty) {
			mindex += mpitch;
			continue;
		}
		if (y > ty) {
			break;
		}
		unsigned int tindex = (6 + (my % scaley) * 8) * tpitch;
		for (mx = XOffset; mx < W - XOffset; ++mx) {

			x = Minimap2MapX[mx];
			if (x < tx) {
				continue;
			}
			if (x > tx) {
				break;
			}

			tile = Map.Fields[x + y].SeenTile;
			if (!tile) {
				tile = Map.Fields[x + y].Tile;
			}

			xofs = Map.TileGraphic->frame_map[tile].x + 7 + (mx % scalex) * 8;
			yofs = Map.TileGraphic->frame_map[tile].y * tpitch + tindex;

#ifdef TERRAIN_IN_DISPLAY_FORMAT
			switch (tbpp) {
				case 1:
				{
					SDL_Color color =
						 Map.TileGraphic->Surface->format->palette->colors[
								DRAW::GetPixel<1>(tpixels, xofs + yofs)];
#ifdef USE_OPENGL
					c = Video.MapRGB(0, color.r, color.g, color.b);
#else
					c = Video.MapRGB(MinimapTerrainSurface->format,
											 color.r, color.g, color.b);
#endif
				}
				break;
				case 2:
				{
					SDL_PixelFormat *f = Map.TileGraphic->Surface->format;
					c = DRAW::GetPixel<2>(tpixels, xofs + yofs);
#ifdef USE_OPENGL
					c = Video.MapRGB(0,
							((c & f->Rmask) >> f->Rshift),
							((c & f->Gmask) >> f->Gshift),
							((c & f->Bmask) >> f->Bshift));
#else
					c = Video.MapRGB(MinimapTerrainSurface->format,
							((c & f->Rmask) >> f->Rshift),
							((c & f->Gmask) >> f->Gshift),
							((c & f->Bmask) >> f->Bshift));
#endif					
				}
				break;
				case 4:
					c = DRAW::GetPixel<4>(tpixels, xofs + yofs);
#ifndef USE_OPENGL
					if(mbpp != 4) {
						SDL_PixelFormat *f = Map.TileGraphic->Surface->format;
						c = Video.MapRGB(MinimapTerrainSurface->format,
							((c & f->Rmask) >> f->Rshift),
							((c & f->Gmask) >> f->Gshift),
							((c & f->Bmask) >> f->Bshift));
					}
#endif
				break;
				default:
					Assert(0);
				break;	
			}
#ifndef USE_OPENGL
			switch (mbpp) {
				case 1:
					//should we support this ?
					//palete should be already set or not ????
					DRAW::PutPixel<1>(mpixels, mx + mindex, c);
				break;
				case 2:
					DRAW::PutPixel<2>(mpixels, mx + mindex, c);
				break;
				case 4:
#endif
					DRAW::PutPixel<4>(mpixels, mx + mindex, c);
#ifndef USE_OPENGL				
				break;
				default:
					Assert(0);
				break;	
			}		 
#endif		
				

#else
			switch (tbpp) {
				case 1:
#ifndef USE_OPENGL
					DRAW::PutPixel<1>(mpixels, mx + mindex, 
						DRAW::GetPixel<1>(tpixels, xofs + yofs));
#else
				{
					SDL_Color color =
						 Map.TileGraphic->Surface->format->palette->colors[
								DRAW::GetPixel<1>(tpixels, xofs + yofs)];
					DRAW::PutPixel<4>(mpixels, mx + mindex, 
						Video.MapRGB(0, color.r, color.g, color.b));			
				}		
#endif
				break;
				case 2:
#ifndef USE_OPENGL
					DRAW::PutPixel<2>(mpixels, mx + mindex, 
						DRAW::GetPixel<2>(tpixels, xofs + yofs));
#else
				{
					SDL_PixelFormat *f = Map.TileGraphic->Surface->format;
					Uint32 c = DRAW::GetPixel<2>(tpixels, xofs + yofs);
					DRAW::PutPixel<4>(mpixels, mx + mindex, 
						Video.MapRGB(0,
							((c & f->Rmask) >> f->Rshift),
							((c & f->Gmask) >> f->Gshift),
							((c & f->Bmask) >> f->Bshift)));
				}
#endif
				break;
				case 4:
					DRAW::PutPixel<4>(mpixels, mx + mindex, 
						DRAW::GetPixel<4>(tpixels, xofs + yofs));
				break;
				default:		
					Assert(0);
				break;
			}	

#endif

		}
		mindex += mpitch;
	}

#ifndef USE_OPENGL
	SDL_UnlockSurface(MinimapTerrainSurface);
#endif
	SDL_UnlockSurface(Map.TileGraphic->Surface);

}

/**
**  Draw an unit on the minimap.
*/
static void DrawUnitOn(CUnit *unit, int red_phase)
{
	CUnitType *type;
	int mx;
	int my;
	int w;
	int h;
	Uint32 color;

	if (Editor.Running || ReplayRevealMap || unit->IsVisible(ThisPlayer)) {
		type = unit->Type;
	} else {
		type = unit->Seen.Type;
		// This will happen for radar if the unit has not been seen and we
		// have it on radar.
		if (!type) {
			type = unit->Type;
		}
	}

	if (unit->Player->Index == PlayerNumNeutral) {
		color = Video.MapRGB(TheScreen->format,
			type->NeutralMinimapColorRGB.r,
			type->NeutralMinimapColorRGB.g,
			type->NeutralMinimapColorRGB.b);
	} else if (unit->Player == ThisPlayer && !Editor.Running) {
		if (unit->Attacked && unit->Attacked + ATTACK_BLINK_DURATION > GameCycle &&
				(red_phase || unit->Attacked + ATTACK_RED_DURATION > GameCycle)) {
			color = ColorRed;
		} else if (UI.Minimap.ShowSelected && unit->Selected) {
			color = ColorWhite;
		} else {
			color = ColorGreen;
		}
	} else {
		color = unit->Player->Color;
	}

	mx = 1 + UI.Minimap.XOffset + Map2MinimapX[unit->X];
	my = 1 + UI.Minimap.YOffset + Map2MinimapY[unit->Y];
	w = Map2MinimapX[type->TileWidth];
	if (mx + w >= UI.Minimap.W) { // clip right side
		w = UI.Minimap.W - mx;
	}
	
	h = Map2MinimapY[type->TileHeight];
	if (my + h >= UI.Minimap.H) { // clip bottom side
		h = UI.Minimap.H - my;
	}
	

	{
#ifdef USE_OPENGL
		const int pitch = MinimapTextureWidth;
		void *pixels = (void *)MinimapSurface;
#else
		void *pixels = MinimapSurface->pixels;
		const int bpp = MinimapSurface->format->BytesPerPixel;
		const int pitch = MinimapSurface->pitch / bpp; 
#endif
		if(w == 1 && h == 1) {
#ifndef USE_OPENGL		
			switch(bpp) {
				case 2:
					DRAW::PutPixel<2>(pixels, mx + my * pitch, color);
				break;
				case 4:
#endif
					DRAW::PutPixel<4>(pixels, mx + my * pitch, color);
#ifndef USE_OPENGL		
				break;
				default:
					Assert(0);
				break;
			}			
#endif		
		} else {
			unsigned int index = my * pitch;
#ifndef USE_OPENGL					
			switch(bpp) {
				case 2:
					do {
						DRAW::DrawHLine<2>(pixels, mx + index, w, color);
						index += pitch;
					} while(--h);
				break;
				case 4:
#endif			
					do {
						DRAW::DrawHLine<4>(pixels, mx + index, w, color);
						index += pitch;
					} while(--h);
#ifndef USE_OPENGL		
				break;
				default:
					Assert(0);
				break;
			}			
#endif
		}
	}
}

template <const int BPP>
void CMinimap::UpdateSeen(void *const pixels, const int pitch)
{
	int mx;
	int my;
	int visiontype; // 0 unexplored, 1 explored, >1 visible.
	unsigned int index = 0;
#ifndef USE_OPENGL
	Uint32 color = ColorBlack;
#else
	Uint32 color = Video.MapRGB(0, 0, 0, 0);
#endif
	
	for (my = 0; my < H; ++my) {
		for (mx = 0; mx < W; ++mx) {
			visiontype = Map.IsTileVisible(ThisPlayer,
								 Minimap2MapX[mx] + Minimap2MapY[my]);
			if ( visiontype == 0 || (visiontype == 1 && ((mx & 1) != (my & 1))))  {
				DRAW::PutPixel<BPP>(pixels, mx + index, color);
			}			
		}
		index += pitch;
	}
}

/**
**  Update the minimap with the current game information
*/
void CMinimap::Update(void)
{
	static int red_phase;

	// red phase occurs every other second
	if (red_phase != (int)((FrameCounter / FRAMES_PER_SECOND) & 1)) {
		red_phase = !red_phase;
	}

	/*
	 * Clear Minimap background if not transparent and 
	 * we won't copy terrain as entire background.
	 */
	if (!Transparent && !WithTerrain) {
		ClearMinimap();
	}

	//
	// Draw the terrain
	//
	if (WithTerrain) {
		CopyMinimapTerrain();
	}

#ifndef USE_OPENGL
	SDL_LockSurface(MinimapSurface);
	
	const int bpp = MinimapSurface->format->BytesPerPixel;
	const int pitch = MinimapSurface->pitch / bpp;
	void *const pixels = MinimapSurface->pixels;
	
#else
	void *const pixels = (void *)MinimapSurface;
	const int pitch = MinimapTextureWidth;
#endif

	if(!ReplayRevealMap) {
#ifndef USE_OPENGL
		switch(bpp) {
			case 2:
				UpdateSeen<2>(pixels, pitch);
			break;
			case 4:
#endif
				UpdateSeen<4>(pixels, pitch);
#ifndef USE_OPENGL
			break;
			default:
				Assert(0);
			break;
		}	
#endif
	}

	//
	// Draw units on map
	// FIXME: We should rewrite this completely
	//
	for (int n = 0; n < NumUnits; ++n) {
		if (Units[n]->IsVisibleOnMinimap()) {
			DrawUnitOn(Units[n], red_phase);
		}
	}

#ifndef USE_OPENGL
	SDL_UnlockSurface(MinimapSurface);
#endif
}

/**
**  Draw the minimap events
*/
static void DrawEvents(void)
{
	for (int i = 0; i < NumMinimapEvents; ++i) {
		Video.DrawTransCircleClip(ColorWhite,
			MinimapEvents[i].X, MinimapEvents[i].Y,
			MinimapEvents[i].Size, 192);
		MinimapEvents[i].Size -= 1;
		if (MinimapEvents[i].Size < 2) {
			MinimapEvents[i] = MinimapEvents[--NumMinimapEvents];
			--i;
		}
	}
}

/**
**  Draw the minimap on the screen
*/
void CMinimap::Draw(int vx, int vy)
{
#ifndef USE_OPENGL
	SDL_Rect drect = {X, Y, 0, 0};
	SDL_BlitSurface(MinimapSurface, NULL, TheScreen, &drect);
#else
	glBindTexture(GL_TEXTURE_2D, MinimapTexture);
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, MinimapTextureWidth, MinimapTextureHeight,
		GL_RGBA, GL_UNSIGNED_BYTE, MinimapSurface);

	glBegin(GL_QUADS);
	glTexCoord2f(0.0f, 0.0f);
	glVertex2i(X, Y);
	glTexCoord2f(0.0f, (float)H / MinimapTextureHeight);
	glVertex2i(X, Y + H);
	glTexCoord2f((float)W / MinimapTextureWidth, (float)H / MinimapTextureHeight);
	glVertex2i(X + W, Y + H);
	glTexCoord2f((float)W / MinimapTextureWidth, 0.0f);
	glVertex2i(X + W, Y);
	glEnd();
#endif

	DrawEvents();
}


/**
**  Convert minimap cursor X position to tile map coordinate.
**
**  @param x  Screen X pixel coordinate.
**
**  @return   Tile X coordinate.
*/
int CMinimap::Screen2MapX(int x)
{
	int tx = (((x - X - XOffset) * MINIMAP_FAC) / MinimapScaleX);
	tx = std::max(tx, 0);
	tx = std::min(tx, Map.Info.MapWidth - 1);
	return tx;
}

/**
**  Convert minimap cursor Y position to tile map coordinate.
**
**  @param y  Screen Y pixel coordinate.
**
**  @return   Tile Y coordinate.
*/
int CMinimap::Screen2MapY(int y)
{
	int ty = (((y - Y - YOffset) * MINIMAP_FAC) / MinimapScaleY);
	ty = std::max(ty, 0);
	ty = std::min(ty, Map.Info.MapHeight - 1);
	return ty;
}

/**
**  Destroy mini-map.
*/
void CMinimap::Destroy(void)
{
#ifndef USE_OPENGL
	SDL_FreeSurface(MinimapTerrainSurface);
#else
	delete[] MinimapTerrainSurface;
#endif
	MinimapTerrainSurface = NULL;
	if (MinimapSurface) {
#ifndef USE_OPENGL
		SDL_FreeSurface(MinimapSurface);
#else
		glDeleteTextures(1, &MinimapTexture);
		delete[] MinimapSurface;
#endif
		MinimapSurface = NULL;
	}
	delete[] Minimap2MapX;
	Minimap2MapX = NULL;
	delete[] Minimap2MapY;
	Minimap2MapY = NULL;
}

/**
**  Draw minimap cursor.
**
**  @param vx  View point X position.
**  @param vy  View point Y position.
*/
void CMinimap::DrawCursor(int vx, int vy)
{
	// Determine and save region below minimap cursor
	int x = X + XOffset + (vx * MinimapScaleX) / MINIMAP_FAC;
	int y = Y + YOffset + (vy * MinimapScaleY) / MINIMAP_FAC;
	int w = (UI.SelectedViewport->MapWidth * MinimapScaleX) / MINIMAP_FAC;
	int h = (UI.SelectedViewport->MapHeight * MinimapScaleY) / MINIMAP_FAC;

	// Draw cursor as rectangle (Note: unclipped, as it is always visible)
	Video.DrawTransRectangle(UI.ViewportCursorColor, x, y, w, h, 128);
}

/**
**  Add a minimap event
**
**  @param x  Map X tile position
**  @param y  Map Y tile position
*/
void CMinimap::AddEvent(int x, int y)
{
	if (NumMinimapEvents == MAX_MINIMAP_EVENTS) {
		return;
	}

	MinimapEvent *minimapEvent = &MinimapEvents[NumMinimapEvents++];

	minimapEvent->X = X + XOffset + (x * MinimapScaleX) / MINIMAP_FAC;
	minimapEvent->Y = Y + YOffset + (y * MinimapScaleY) / MINIMAP_FAC;
	minimapEvent->Size = (W < H) ? W / 3 : H / 3;
}

//@}
