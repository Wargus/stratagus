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

#define MINIMAP_FAC (16 * 3)  /// integer scale factor

	/// unit attacked are shown red for at least this amount of cycles
#define ATTACK_RED_DURATION (1 * CYCLES_PER_SECOND)
	/// unit attacked are shown blinking for this amount of cycles
#define ATTACK_BLINK_DURATION (7 * CYCLES_PER_SECOND)


/*----------------------------------------------------------------------------
--  Variables
----------------------------------------------------------------------------*/

static GLuint MinimapTexture;
static unsigned char *MinimapSurfaceGL;
static unsigned char *MinimapTerrainSurfaceGL;
static int MinimapTextureWidth;
static int MinimapTextureHeight;
static SDL_Surface *MinimapSurface;        /// generated minimap
static SDL_Surface *MinimapTerrainSurface; /// generated minimap terrain

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


/*----------------------------------------------------------------------------
-- Functions
----------------------------------------------------------------------------*/

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
		MinimapSurfaceGL);
}

/**
**  Create a minimap from the tiles of the map.
*/
void CMinimap::Create(void)
{
	int n;
	int maxSize = std::max<int>(Map.Info.MapWidth, Map.Info.MapHeight);

	MinimapScaleX = (W * MINIMAP_FAC + maxSize - 1) / maxSize;
	MinimapScaleY = (H * MINIMAP_FAC + maxSize - 1) / maxSize;

	XOffset = (W - (Map.Info.MapWidth * MinimapScaleX) / MINIMAP_FAC + 1) / 2;
	YOffset = (H - (Map.Info.MapHeight * MinimapScaleY) / MINIMAP_FAC + 1) / 2;

	//
	// Calculate minimap fast lookup tables.
	//
	Minimap2MapX = new int[W];
	memset(Minimap2MapX, 0, W * sizeof(int));
	Minimap2MapY = new int[H];
	memset(Minimap2MapY, 0, H * sizeof(int));
	for (n = XOffset; n < W - XOffset; ++n) {
		Minimap2MapX[n] = ((n - XOffset) * MINIMAP_FAC) / MinimapScaleX;
	}
	for (n = YOffset; n < H - YOffset; ++n) {
		Minimap2MapY[n] = (((n - YOffset) * MINIMAP_FAC) / MinimapScaleY);
	}
	for (n = 0; n < Map.Info.MapWidth; ++n) {
		Map2MinimapX[n] = (n * MinimapScaleX) / MINIMAP_FAC;
	}
	for (n = 0; n < Map.Info.MapHeight; ++n) {
		Map2MinimapY[n] = (n * MinimapScaleY) / MINIMAP_FAC;
	}

	if (!UseOpenGL) {
		SDL_PixelFormat *f = TheScreen->format;
		MinimapTerrainSurface = SDL_CreateRGBSurface(SDL_SWSURFACE,
			W, H, f->BitsPerPixel, f->Rmask, f->Gmask, f->Bmask, f->Amask);
		MinimapSurface = SDL_CreateRGBSurface(SDL_SWSURFACE,
			W, H, f->BitsPerPixel, f->Rmask, f->Gmask, f->Bmask, f->Amask);
	} else {
		for (MinimapTextureWidth = 1; MinimapTextureWidth < W; MinimapTextureWidth <<= 1) {
		}
		for (MinimapTextureHeight = 1; MinimapTextureHeight < H; MinimapTextureHeight <<= 1) {
		}
		MinimapTerrainSurfaceGL = new unsigned char[MinimapTextureWidth * MinimapTextureHeight * 4];
		MinimapSurfaceGL = new unsigned char[MinimapTextureWidth * MinimapTextureHeight * 4];
		memset(MinimapSurfaceGL, 0, MinimapTextureWidth * MinimapTextureHeight * 4);
		CreateMinimapTexture();
	}

	UpdateTerrain();

	NumMinimapEvents = 0;
}

/**
**  Free OpenGL minimap
*/
void CMinimap::FreeOpenGL()
{
	glDeleteTextures(1, &MinimapTexture);
}

/**
**  Reload OpenGL minimap
*/
void CMinimap::Reload()
{
	CreateMinimapTexture();
}

/**
**  Set MinimapTerrain pixel
*/
void SetMinimapTerrainPixel(int x, int y, CColor color)
{
	if (!UseOpenGL) {
		int bpp = MinimapTerrainSurface->format->BytesPerPixel;
		Uint8 *p = &((Uint8 *)MinimapTerrainSurface->pixels)[x * bpp + y * MinimapTerrainSurface->pitch];
		Uint32 c = Video.MapRGBA(MinimapTerrainSurface->format, color.R, color.G, color.B, color.A);
		if (bpp == 2) {
			*(Uint16 *)p = c;
		} else {
			*(Uint32 *)p = c;
		}
	} else {
		*(Uint32 *)&(MinimapTerrainSurfaceGL[(x + y * MinimapTextureWidth) * 4]) = Video.MapRGBA(0, color.R, color.G, color.B, color.A);
	}
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
				if (!UseOpenGL) {
					DRAW::PutPixel<1>(mpixels, mx + mindex,
						DRAW::GetPixel<1>(tpixels, xofs + yofs));
				} else {
					SDL_Color color =
						 Map.TileGraphic->Surface->format->palette->colors[
								DRAW::GetPixel<1>(tpixels, xofs + yofs)];
					DRAW::PutPixel<4>(mpixels, mx + mindex,
						Video.MapRGB(0, color.r, color.g, color.b));
				}
			} else if (BPP == 2) {
				if (!UseOpenGL) {
					DRAW::PutPixel<2>(mpixels, mx + mindex,
						DRAW::GetPixel<2>(tpixels, xofs + yofs));
				} else {
					SDL_PixelFormat *f = Map.TileGraphic->Surface->format;
					Uint32 c = DRAW::GetPixel<2>(tpixels, xofs + yofs);
					DRAW::PutPixel<4>(mpixels, mx + mindex,
						Video.MapRGB(0,
							((c & f->Rmask) >> f->Rshift),
							((c & f->Gmask) >> f->Gshift),
							((c & f->Bmask) >> f->Bshift)));
				}
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
*/
void CMinimap::UpdateTerrain(void)
{
	const int bpp = Map.TileGraphic->Surface->format->BytesPerPixel;
	const int tpitch = Map.TileGraphic->Surface->pitch / bpp;
	void *mpixels;
	int mpitch;

	if (!UseOpenGL) {
		if (bpp == 1) {
			SDL_SetPalette(MinimapTerrainSurface, SDL_LOGPAL,
				Map.TileGraphic->Surface->format->palette->colors, 0, 256);
		}

		Assert(bpp == MinimapTerrainSurface->format->BytesPerPixel);

		SDL_LockSurface(MinimapTerrainSurface);
		mpixels = MinimapTerrainSurface->pixels;
		mpitch = MinimapTerrainSurface->pitch / bpp;
	} else {
		mpixels = (void *)MinimapTerrainSurfaceGL;
		mpitch = MinimapTextureWidth;
	}

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
	if (!UseOpenGL) {
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
	}
}

/**
**  Clear the minimap
*/
static void ClearMinimap()
{
	if (!UseOpenGL) {
		SDL_FillRect(MinimapSurface, NULL, SDL_MapRGB(MinimapSurface->format, 0, 0, 0));
	} else {
		memset(MinimapSurfaceGL, 0, MinimapTextureWidth * MinimapTextureHeight * 4);
	}
}

/**
**  Copy the minimap terrain to the minimap
*/
static void CopyMinimapTerrain()
{
	if (!UseOpenGL) {
		SDL_BlitSurface(MinimapTerrainSurface, NULL, MinimapSurface, NULL);
	} else {
		memcpy(MinimapSurfaceGL, MinimapTerrainSurfaceGL, MinimapTextureWidth * MinimapTextureHeight * 4);
	}
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
	void *mpixels;
	int mpitch;
	int mbpp;
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

	if (!UseOpenGL) {
		SDL_LockSurface(MinimapTerrainSurface);
		mpixels = MinimapTerrainSurface->pixels;
#ifdef TERRAIN_IN_DISPLAY_FORMAT
		mbpp = MinimapTerrainSurface->format->BytesPerPixel;
		mpitch = MinimapTerrainSurface->pitch / mbpp;
#else
		Assert(tbpp == MinimapTerrainSurface->format->BytesPerPixel);
		mpitch = MinimapTerrainSurface->pitch / tbpp;
#endif
	} else {
		mpixels = (void *)MinimapTerrainSurfaceGL;
		mpitch = MinimapTextureWidth;
	}


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
					if (UseOpenGL)
						c = Video.MapRGB(0, color.r, color.g, color.b);
					else
						c = Video.MapRGB(MinimapTerrainSurface->format,
											 color.r, color.g, color.b);
				}
				break;
				case 2:
				{
					SDL_PixelFormat *f = Map.TileGraphic->Surface->format;
					c = DRAW::GetPixel<2>(tpixels, xofs + yofs);
					if (UseOpenGL)
						c = Video.MapRGB(0,
							((c & f->Rmask) >> f->Rshift),
							((c & f->Gmask) >> f->Gshift),
							((c & f->Bmask) >> f->Bshift));
					else
						c = Video.MapRGB(MinimapTerrainSurface->format,
							((c & f->Rmask) >> f->Rshift),
							((c & f->Gmask) >> f->Gshift),
							((c & f->Bmask) >> f->Bshift));
				}
				break;
				case 4:
					c = DRAW::GetPixel<4>(tpixels, xofs + yofs);
					if (!UseOpenGL)
						if(mbpp != 4) {
							SDL_PixelFormat *f = Map.TileGraphic->Surface->format;
							c = Video.MapRGB(MinimapTerrainSurface->format,
								((c & f->Rmask) >> f->Rshift),
								((c & f->Gmask) >> f->Gshift),
								((c & f->Bmask) >> f->Bshift));
						}
				break;
				default:
					Assert(0);
				break;
			}
			if (!UseOpenGL) {
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
					DRAW::PutPixel<4>(mpixels, mx + mindex, c);
				break;
				default:
					Assert(0);
				break;
			}
			} else {
				DRAW::PutPixel<4>(mpixels, mx + mindex, c);
			}
#else
			switch (tbpp) {
				case 1:
					if (!UseOpenGL) {
						DRAW::PutPixel<1>(mpixels, mx + mindex,
							DRAW::GetPixel<1>(tpixels, xofs + yofs));
					} else {
						SDL_Color color =
							 Map.TileGraphic->Surface->format->palette->colors[
								DRAW::GetPixel<1>(tpixels, xofs + yofs)];
						DRAW::PutPixel<4>(mpixels, mx + mindex,
							Video.MapRGB(0, color.r, color.g, color.b));
					}
				break;
				case 2:
					if (!UseOpenGL) {
						DRAW::PutPixel<2>(mpixels, mx + mindex,
							DRAW::GetPixel<2>(tpixels, xofs + yofs));
					} else {
						SDL_PixelFormat *f = Map.TileGraphic->Surface->format;
						Uint32 c = DRAW::GetPixel<2>(tpixels, xofs + yofs);
						DRAW::PutPixel<4>(mpixels, mx + mindex,
							Video.MapRGB(0,
								((c & f->Rmask) >> f->Rshift),
								((c & f->Gmask) >> f->Gshift),
								((c & f->Bmask) >> f->Bshift)));
					}
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

	if (!UseOpenGL)
		SDL_UnlockSurface(MinimapTerrainSurface);
	else
		SDL_UnlockSurface(Map.TileGraphic->Surface);

}

/**
**  Set a pixel in the minimap
*/
static void SetMinimapPixel(int x, int y, Uint32 color)
{
	if (!UseOpenGL) {
		int bpp = MinimapSurface->format->BytesPerPixel;
		Uint8 *p = &((Uint8 *)MinimapSurface->pixels)[x * bpp + y * MinimapSurface->pitch];
		if (bpp == 2) {
			*(Uint16 *)p = color;
		} else {
			*(Uint32 *)p = color;
		}
	} else {
		*(Uint32 *)&(MinimapSurfaceGL[(x + y * MinimapTextureWidth) * 4]) = color;
	}
}

/**
**  Draw a unit on the minimap.
*/
static void DrawUnitOn(CUnit *unit, bool red_phase)
{
	const CUnitType *type;
	int mx, my;
	int w, h;
	int origh;
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

	//
	// Figure out what color to use
	//
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

	//
	// Find which pixels to draw
	//
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


	//
	// Draw the color on the minimap
	//
	origh = h;
	while (w-- >= 0) {
		while (h-- >= 0) {
			SetMinimapPixel(mx + w, my + h, color);
		}
		h = origh;
	}
}

template <const int BPP>
void CMinimap::UpdateSeen(void *const pixels, const int pitch)
{
	int mx;
	int my;
	int visiontype; // 0 unexplored, 1 explored, >1 visible.
	unsigned int index = 0;
	Uint32 color;
	if (!UseOpenGL)
		color = ColorBlack;
	else
		color = Video.MapRGB(0, 0, 0, 0);

	for (my = 0; my < H; ++my) {
		for (mx = 0; mx < W; ++mx) {
			visiontype = Map.IsTileVisible(ThisPlayer,
								 Minimap2MapX[mx] + Minimap2MapY[my]);
			if(visiontype == 0 || (visiontype == 1 && ((mx & 1) != (my & 1)))) {
				DRAW::PutPixel<BPP>(pixels, mx + index, color);
			}
		}
		index += pitch;
	}
}

/**
**  Update the minimap with the current game information
*/
void CMinimap::Update()
{
	static bool red_phase = false;
	int mx, my;
	int visiontype;

	// red phase occurs every other second
	if (red_phase != (bool)((FrameCounter / FRAMES_PER_SECOND) & 1)) {
		red_phase = !red_phase;
	}

	// Clear Minimap background if not transparent
	if (!Transparent) {
		ClearMinimap();
	}

	//
	// Draw the terrain
	//
	if (WithTerrain) {
		CopyMinimapTerrain();
	}

	if (!UseOpenGL) {
		SDL_LockSurface(MinimapSurface);
		SDL_LockSurface(MinimapTerrainSurface);
	}

	// Hide unexplored tiles and use dithering for fog
	for (my = 0; my < H; ++my) {
		for (mx = 0; mx < W; ++mx) {
			if (!ReplayRevealMap) {
				visiontype = Map.IsTileVisible(ThisPlayer, Minimap2MapX[mx], Minimap2MapY[my]);

				// visiontype: 0 unexplored, 1 explored, >1 visible.
				if (visiontype == 0) {
					SetMinimapPixel(mx, my, ColorBlack);
				} else if (visiontype == 1 && ((mx & 1) != (my & 1)))  {
					// TODO: we could do real blending here instead of dithering
					SetMinimapPixel(mx, my, ColorBlack);
				}
			}
		}
	}

	if (!UseOpenGL) {
		SDL_UnlockSurface(MinimapTerrainSurface);
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

	if (!UseOpenGL) {
		SDL_UnlockSurface(MinimapSurface);
	}
}

/**
**  Draw the minimap events
*/
static void DrawEvents()
{
	for (int i = 0; i < NumMinimapEvents; ++i) {
		Video.DrawTransCircleClip(ColorWhite,
			MinimapEvents[i].X, MinimapEvents[i].Y,
			MinimapEvents[i].Size, 192);

		// Decrease the size of the circle
		MinimapEvents[i].Size -= 1;

		// Remove the event
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
	if (!UseOpenGL) {
		SDL_Rect drect = {X, Y, 0, 0};
		SDL_BlitSurface(MinimapSurface, NULL, TheScreen, &drect);
	} else {
		glBindTexture(GL_TEXTURE_2D, MinimapTexture);
		glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, MinimapTextureWidth, MinimapTextureHeight,
			GL_RGBA, GL_UNSIGNED_BYTE, MinimapSurfaceGL);

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
	}

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
	tx = std::max<int>(tx, 0);
	tx = std::min<int>(tx, Map.Info.MapWidth - 1);
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
	ty = std::max<int>(ty, 0);
	ty = std::min<int>(ty, Map.Info.MapHeight - 1);
	return ty;
}

/**
**  Destroy mini-map.
*/
void CMinimap::Destroy()
{
	if (!UseOpenGL) {
		SDL_FreeSurface(MinimapTerrainSurface);
		MinimapTerrainSurface = NULL;
	} else {
		delete[] MinimapTerrainSurfaceGL;
		MinimapTerrainSurfaceGL = NULL;
	}
	if (!UseOpenGL) {
		if (MinimapSurface) {
			SDL_FreeSurface(MinimapSurface);
			MinimapSurface = NULL;
		}
	} else {
		if (MinimapSurfaceGL) {
			glDeleteTextures(1, &MinimapTexture);
			delete[] MinimapSurfaceGL;
			MinimapSurfaceGL = NULL;
		}
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
