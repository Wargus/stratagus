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
/**@name minimap.c - The minimap. */
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

/*----------------------------------------------------------------------------
--  Variables
----------------------------------------------------------------------------*/

#ifdef USE_OPENGL
static GLuint MinimapTexture;
static unsigned char* MinimapSurface;
static unsigned char* MinimapTerrainSurface;
static int MinimapTextureWidth;
static int MinimapTextureHeight;
#else
static SDL_Surface* MinimapSurface;        /// generated minimap
static SDL_Surface* MinimapTerrainSurface; /// generated minimap terrain
#endif
static int* Minimap2MapX;                  /// fast conversion table
static int* Minimap2MapY;                  /// fast conversion table
static int Map2MinimapX[MaxMapWidth];      /// fast conversion table
static int Map2MinimapY[MaxMapHeight];     /// fast conversion table

// MinimapScale:
// 32x32 64x64 96x96 128x128 256x256 512x512 ...
// *4 *2 *4/3   *1 *1/2 *1/4
static int MinimapScaleX;                  /// Minimap scale to fit into window
static int MinimapScaleY;                  /// Minimap scale to fit into window
int MinimapX;                              /// Minimap drawing position x offset
int MinimapY;                              /// Minimap drawing position y offset

int MinimapWithTerrain = 1;                /// display minimap with terrain
int MinimapFriendly = 1;                   /// switch colors of friendly units
int MinimapShowSelected = 1;               /// highlight selected units


#define MAX_MINIMAP_EVENTS 8

struct _minimap_events_ {
	int X;
	int Y;
	int Size;
} MinimapEvents[MAX_MINIMAP_EVENTS];
int NumMinimapEvents;


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
void CreateMinimap(void)
{
	int n;
#ifndef USE_OPENGL
	SDL_PixelFormat* f;
#endif

	if (TheMap.Info.MapWidth > TheMap.Info.MapHeight) { // Scale to biggest value.
		n = TheMap.Info.MapWidth;
	} else {
		n = TheMap.Info.MapHeight;
	}
	MinimapScaleX = (TheUI.MinimapW * MINIMAP_FAC + n - 1) / n;
	MinimapScaleY = (TheUI.MinimapH * MINIMAP_FAC + n - 1) / n;

	MinimapX = ((TheUI.MinimapW * MINIMAP_FAC) / MinimapScaleX - TheMap.Info.MapWidth) / 2;
	MinimapY = ((TheUI.MinimapH * MINIMAP_FAC) / MinimapScaleY - TheMap.Info.MapHeight) / 2;
	MinimapX = (TheUI.MinimapW - (TheMap.Info.MapWidth * MinimapScaleX) / MINIMAP_FAC) / 2;
	MinimapY = (TheUI.MinimapH - (TheMap.Info.MapHeight * MinimapScaleY) / MINIMAP_FAC) / 2;

	DebugPrint("MinimapScale %d %d (%d %d), X off %d, Y off %d\n" _C_
		MinimapScaleX / MINIMAP_FAC _C_ MinimapScaleY / MINIMAP_FAC _C_
		MinimapScaleX _C_ MinimapScaleY _C_
		MinimapX _C_ MinimapY);

	//
	// Calculate minimap fast lookup tables.
	//
	// FIXME: this needs to be recalculated during map load - the map size
	// might have changed!
	Minimap2MapX = calloc(sizeof(int), TheUI.MinimapW * TheUI.MinimapH);
	Minimap2MapY = calloc(sizeof(int), TheUI.MinimapW * TheUI.MinimapH);
	for (n = MinimapX; n < TheUI.MinimapW - MinimapX; ++n) {
		Minimap2MapX[n] = ((n - MinimapX) * MINIMAP_FAC) / MinimapScaleX;
	}
	for (n = MinimapY; n < TheUI.MinimapH - MinimapY; ++n) {
		Minimap2MapY[n] = (((n - MinimapY) * MINIMAP_FAC) / MinimapScaleY) * TheMap.Info.MapWidth;
	}
	for (n = 0; n < TheMap.Info.MapWidth; ++n) {
		Map2MinimapX[n] = (n * MinimapScaleX) / MINIMAP_FAC;
	}
	for (n = 0; n < TheMap.Info.MapHeight; ++n) {
		Map2MinimapY[n] = (n * MinimapScaleY) / MINIMAP_FAC;
	}

	// Palette updated from UpdateMinimapTerrain()
#ifndef USE_OPENGL
	f = TheMap.TileGraphic->Surface->format;
	MinimapTerrainSurface = SDL_CreateRGBSurface(SDL_SWSURFACE,
		TheUI.MinimapW, TheUI.MinimapH, f->BitsPerPixel,
		f->Rmask, f->Gmask, f->Bmask, f->Amask);
	MinimapSurface = SDL_CreateRGBSurface(SDL_SWSURFACE,
		TheUI.MinimapW, TheUI.MinimapH,
		f->BitsPerPixel, f->Rmask, f->Gmask, f->Bmask, f->Amask);
#else
	for (MinimapTextureWidth = 1; MinimapTextureWidth < TheUI.MinimapW; MinimapTextureWidth <<= 1) {
	}
	for (MinimapTextureHeight = 1; MinimapTextureHeight < TheUI.MinimapH; MinimapTextureHeight <<= 1) {
	}
	MinimapTerrainSurface = malloc(MinimapTextureWidth * MinimapTextureHeight * 4);
	MinimapSurface = calloc(MinimapTextureWidth * MinimapTextureHeight * 4, sizeof(*MinimapSurface));
	CreateMinimapTexture();
#endif

#ifndef USE_OPENGL
	if (!TheUI.MinimapTransparent) {
		VideoFillRectangle(ColorBlack, MinimapX, MinimapY,
			TheUI.MinimapW, TheUI.MinimapH);
	}
#endif

	UpdateMinimapTerrain();

	NumMinimapEvents = 0;
}

#ifdef USE_OPENGL
/**
**  Reload OpenGL minimap
*/
void ReloadMinimap(void)
{
	CreateMinimapTexture();
}
#endif

/**
**  Update a mini-map from the tiles of the map.
**
**  FIXME: this can surely be sped up??
*/
void UpdateMinimapTerrain(void)
{
	int mx;
	int my;
	int scalex;
	int scaley;
	int tilepitch;
	int xofs;
	int yofs;
	int bpp;

	if (!(scalex = (MinimapScaleX / MINIMAP_FAC))) {
		scalex = 1;
	}
	if (!(scaley = (MinimapScaleY / MINIMAP_FAC))) {
		scaley = 1;
	}
	bpp = TheMap.TileGraphic->Surface->format->BytesPerPixel;

#ifndef USE_OPENGL
	if (bpp == 1) {
		SDL_SetPalette(MinimapTerrainSurface, SDL_LOGPAL,
			TheMap.TileGraphic->Surface->format->palette->colors, 0, 256);
		SDL_SetPalette(MinimapSurface, SDL_LOGPAL,
			TheMap.TileGraphic->Surface->format->palette->colors, 0, 256);
	}
#endif

	tilepitch = TheMap.TileGraphic->Surface->w / TileSizeX;

#ifndef USE_OPENGL
	SDL_LockSurface(MinimapTerrainSurface);
#endif
	SDL_LockSurface(TheMap.TileGraphic->Surface);
	//
	//  Pixel 7,6 7,14, 15,6 15,14 are taken for the minimap picture.
	//
	for (my = MinimapY; my < TheUI.MinimapH - MinimapY; ++my) {
		for (mx = MinimapX; mx < TheUI.MinimapW - MinimapX; ++mx) {
			int tile;
#ifdef USE_OPENGL
			Uint32 c;
#endif

			tile = TheMap.Fields[Minimap2MapX[mx] + Minimap2MapY[my]].Tile;

			xofs = TileSizeX * (tile % tilepitch);
			yofs = TileSizeY * (tile / tilepitch);

#ifndef USE_OPENGL
			if (bpp == 1) {
				((Uint8*)MinimapTerrainSurface->pixels)[mx + my * MinimapTerrainSurface->pitch] =
					((Uint8*)TheMap.TileGraphic->Surface->pixels)[
						xofs + 7 + (mx % scalex) * 8 + (yofs + 6 + (my % scaley) * 8) *
						TheMap.TileGraphic->Surface->pitch];
			} else if (bpp == 3) {
				char* d;
				char* s;

				d = &((Uint8*)MinimapTerrainSurface->pixels)[mx * bpp + my * MinimapTerrainSurface->pitch];
				s = &((Uint8*)TheMap.TileGraphic->Surface->pixels)[
						(xofs + 7 + (mx % scalex) * 8) * bpp + (yofs + 6 + (my % scaley) * 8) *
						TheMap.TileGraphic->Surface->pitch];
				*d++ = *s++;
				*d++ = *s++;
				*d++ = *s++;
			} else {
				*(Uint32*)&((Uint8*)MinimapTerrainSurface->pixels)[mx * bpp + my * MinimapTerrainSurface->pitch] =
					*(Uint32*)&((Uint8*)TheMap.TileGraphic->Surface->pixels)[
						(xofs + 7 + (mx % scalex) * 8) * bpp + (yofs + 6 + (my % scaley) * 8) *
						TheMap.TileGraphic->Surface->pitch];
			}
#else
			if (bpp == 1) {
				SDL_Color color;

				color = TheMap.TileGraphic->Surface->format->palette->colors[
					((Uint8*)TheMap.TileGraphic->Surface->pixels)[
						xofs + 7 + (mx % scalex) * 8 + (yofs + 6 + (my % scaley) * 8) *
						TheMap.TileGraphic->Surface->pitch]];
				c = VideoMapRGB(0, color.r, color.g, color.b);
			} else {
				SDL_PixelFormat* f;

				f = TheMap.TileGraphic->Surface->format;
				c = *(Uint32*)&((Uint8*)TheMap.TileGraphic->Surface->pixels)[
					(xofs + 7 + (mx % scalex) * 8) * bpp + (yofs + 6 + (my % scaley) * 8) *
					TheMap.TileGraphic->Surface->pitch];
				c = VideoMapRGB(0,
					((c & f->Rmask) >> f->Rshift),
					((c & f->Gmask) >> f->Gshift),
					((c & f->Bmask) >> f->Bshift));
			}
			*(Uint32*)&(MinimapTerrainSurface[(mx + my * MinimapTextureWidth) * 4]) = c;
#endif
		}
	}

#ifndef USE_OPENGL
	SDL_UnlockSurface(MinimapTerrainSurface);
#endif
	SDL_UnlockSurface(TheMap.TileGraphic->Surface);
}

/**
**  Update a single minimap tile after a change
**
**  @param tx  The X map position to update in the minimap
**  @param ty  The Y map position to update in the minimap
*/
void UpdateMinimapXY(int tx, int ty)
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
	int bpp;

	if (!MinimapTerrainSurface) {
		return;
	}

	scalex = MinimapScaleX / MINIMAP_FAC;
	if (scalex == 0) {
		scalex = 1;
	}
	scaley = MinimapScaleY / MINIMAP_FAC;
	if (scaley == 0) {
		scaley = 1;
	}

	tilepitch = TheMap.TileGraphic->Surface->w / TileSizeX;
	bpp = TheMap.TileGraphic->Surface->format->BytesPerPixel;

	//
	//  Pixel 7,6 7,14, 15,6 15,14 are taken for the minimap picture.
	//
#ifndef USE_OPENGL
	SDL_LockSurface(MinimapTerrainSurface);
#endif
	SDL_LockSurface(TheMap.TileGraphic->Surface);

	ty *= TheMap.Info.MapWidth;
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
#ifdef USE_OPENGL
			Uint32 c;
#endif

			x = Minimap2MapX[mx];
			if (x < tx) {
				continue;
			}
			if (x > tx) {
				break;
			}

			tile = TheMap.Fields[x + y].SeenTile;
			if (!tile) {
				tile = TheMap.Fields[x + y].Tile;
			}

			xofs = TileSizeX * (tile % tilepitch);
			yofs = TileSizeY * (tile / tilepitch);

#ifndef USE_OPENGL
			if (bpp == 1) {
				((Uint8*)MinimapTerrainSurface->pixels)[mx + my * MinimapTerrainSurface->pitch] =
					((Uint8*)TheMap.TileGraphic->Surface->pixels)[
						xofs + 7 + (mx % scalex) * 8 + (yofs + 6 + (my % scaley) * 8) *
						TheMap.TileGraphic->Surface->pitch];
			} else if (bpp == 3) {
				char* d;
				char* s;

				d = &((Uint8*)MinimapTerrainSurface->pixels)[mx * bpp + my * MinimapTerrainSurface->pitch];
				s = &((Uint8*)TheMap.TileGraphic->Surface->pixels)[
						(xofs + 7 + (mx % scalex) * 8) * bpp + (yofs + 6 + (my % scaley) * 8) *
						TheMap.TileGraphic->Surface->pitch];
				*d++ = *s++;
				*d++ = *s++;
				*d++ = *s++;
			} else {
				*(Uint32*)&((Uint8*)MinimapTerrainSurface->pixels)[mx * bpp + my * MinimapTerrainSurface->pitch] =
					*(Uint32*)&((Uint8*)TheMap.TileGraphic->Surface->pixels)[
						(xofs + 7 + (mx % scalex) * 8) * bpp + (yofs + 6 + (my % scaley) * 8) *
						TheMap.TileGraphic->Surface->pitch];
			}
#else
			if (bpp == 1) {
				SDL_Color color;

				color = TheMap.TileGraphic->Surface->format->palette->colors[
					((Uint8*)TheMap.TileGraphic->Surface->pixels)[
						xofs + 7 + (mx % scalex) * 8 + (yofs + 6 + (my % scaley) * 8) *
						TheMap.TileGraphic->Surface->pitch]];
				c = VideoMapRGB(0, color.r, color.g, color.b);
			} else {
				SDL_PixelFormat* f;

				f = TheMap.TileGraphic->Surface->format;
				c = *(Uint32*)&((Uint8*)TheMap.TileGraphic->Surface->pixels)[
					(xofs + 7 + (mx % scalex) * 8) * 4 + (yofs + 6 + (my % scaley) * 8) *
					TheMap.TileGraphic->Surface->pitch];
				c = VideoMapRGB(0,
					((c & f->Rmask) >> f->Rshift),
					((c & f->Gmask) >> f->Gshift),
					((c & f->Bmask) >> f->Bshift));
			}
			*(Uint32*)&(MinimapTerrainSurface[(mx + my * MinimapTextureWidth) * 4]) = c;
#endif
		}
	}

#ifndef USE_OPENGL
	SDL_UnlockSurface(MinimapTerrainSurface);
#endif
	SDL_UnlockSurface(TheMap.TileGraphic->Surface);
}

/**
**  Draw an unit on the minimap.
*/
static void DrawUnitOnMinimap(Unit* unit, int red_phase)
{
	UnitType* type;
	int mx;
	int my;
	int w;
	int h;
	int h0;
	Uint32 color;
#ifndef USE_OPENGL
	SDL_Color c;
	int bpp;
#endif

	if (EditorRunning || ReplayRevealMap || UnitVisible(unit, ThisPlayer)) {
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
	//  FIXME: We should force unittypes to have a certain color on the minimap.
	//
	if (unit->Player->Index == PlayerNumNeutral) {
		color = VideoMapRGB(TheScreen->format,
			type->NeutralMinimapColorRGB.r,
			type->NeutralMinimapColorRGB.g,
			type->NeutralMinimapColorRGB.b);
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
	if (mx + w >= TheUI.MinimapW) { // clip right side
		w = TheUI.MinimapW - mx;
	}
	h0 = Map2MinimapY[type->TileHeight];
	if (my + h0 >= TheUI.MinimapH) { // clip bottom side
		h0 = TheUI.MinimapH - my;
	}
#ifndef USE_OPENGL
	bpp = MinimapSurface->format->BytesPerPixel;
	SDL_GetRGB(color, TheScreen->format, &c.r, &c.g, &c.b);
#endif
	while (w-- >= 0) {
		h = h0;
		while (h-- >= 0) {
#ifndef USE_OPENGL
			if (bpp == 1) {
				((Uint8*)MinimapSurface->pixels)[mx + w + (my + h) * MinimapSurface->pitch] =
					VideoMapRGB(MinimapSurface->format, c.r, c.g, c.b);
			} else if (bpp == 3) {
				char* d;

				d = &((Uint8*)MinimapSurface->pixels)[(mx + w) * bpp + (my + h) * MinimapSurface->pitch];
				*(d + MinimapSurface->format->Rshift / 8) = c.r;
				*(d + MinimapSurface->format->Gshift / 8) = c.g;
				*(d + MinimapSurface->format->Bshift / 8) = c.b;
			} else {
				*(Uint32*)&((Uint8*)MinimapSurface->pixels)[(mx + w) * bpp + (my + h) * MinimapSurface->pitch] =
					VideoMapRGB(MinimapSurface->format, c.r, c.g, c.b);
			}
#else
			*(Uint32*)&(MinimapSurface[((mx + w) + (my + h) * MinimapTextureWidth) * 4]) = color;
#endif
		}
	}
}

/**
**  Update the minimap with the current game information
*/
void UpdateMinimap(void)
{
	static int red_phase;
	int red_phase_changed;
	int mx;
	int my;
	int n;
	int visiontype; // 0 unexplored, 1 explored, >1 visible.
#ifndef USE_OPENGL
	int bpp;
#endif

	red_phase_changed = red_phase != (int)((FrameCounter / FRAMES_PER_SECOND) & 1);
	if (red_phase_changed) {
		red_phase = !red_phase;
	}

	// Clear Minimap background if not transparent
	if (!TheUI.MinimapTransparent) {
#ifndef USE_OPENGL
		SDL_FillRect(MinimapSurface, NULL, 0);
#else
		memset(MinimapSurface, 0, MinimapTextureWidth * MinimapTextureHeight * 4);
#endif
	}

#ifndef USE_OPENGL
	SDL_LockSurface(MinimapSurface);
	SDL_LockSurface(MinimapTerrainSurface);
	bpp = MinimapSurface->format->BytesPerPixel;
#endif
	//
	// Draw the terrain
	//
	for (my = 0; my < TheUI.MinimapH; ++my) {
		for (mx = 0; mx < TheUI.MinimapW; ++mx) {
			if (ReplayRevealMap) {
				visiontype = 2;
			} else {
				visiontype = IsTileVisible(ThisPlayer, Minimap2MapX[mx], Minimap2MapY[my] / TheMap.Info.MapWidth);
			}

			if (MinimapWithTerrain && (visiontype > 1 || (visiontype == 1 && ((mx & 1) == (my & 1))))) {
#ifndef USE_OPENGL
				if (bpp == 1) {
					((Uint8*)MinimapSurface->pixels)[mx + my * MinimapSurface->pitch] =
						((Uint8*)MinimapTerrainSurface->pixels)[mx + my * MinimapTerrainSurface->pitch];
				} else if (bpp == 3) {
					char* d;
					char* s;

					d = &((Uint8*)MinimapSurface->pixels)[mx * bpp + my * MinimapSurface->pitch];
					s = &((Uint8*)MinimapTerrainSurface->pixels)[mx * bpp + my * MinimapTerrainSurface->pitch];
					*d++ = *s++;
					*d++ = *s++;
					*d++ = *s++;
				} else {
					*(Uint32*)&((Uint8*)MinimapSurface->pixels)[mx * bpp + my * MinimapSurface->pitch] =
						*(Uint32*)&((Uint8*)MinimapTerrainSurface->pixels)[mx * bpp + my * MinimapTerrainSurface->pitch];
				}
#else
				*(Uint32*)&(MinimapSurface[(mx + my * MinimapTextureWidth) * 4]) =
					*(Uint32*)&(MinimapTerrainSurface[(mx + my * MinimapTextureWidth) * 4]);
#endif
			} else if (visiontype > 0) {
#ifndef USE_OPENGL
				if (bpp == 1) {
					((Uint8*)MinimapSurface->pixels)[mx + my * MinimapSurface->pitch] =
						VideoMapRGB(MinimapSurface->format, 0, 0, 0);
				} else if (bpp == 3) {
					char* d;

					d = &((Uint8*)MinimapSurface->pixels)[mx * bpp + my * MinimapSurface->pitch];
					*(d + MinimapSurface->format->Rshift / 8) = 0;
					*(d + MinimapSurface->format->Gshift / 8) = 0;
					*(d + MinimapSurface->format->Bshift / 8) = 0;
				} else {
					*(Uint32*)&((Uint8*)MinimapSurface->pixels)[mx * bpp + my * MinimapSurface->pitch] =
						VideoMapRGB(MinimapSurface->format, 0, 0, 0);
				}
#else
				*(Uint32*)&(MinimapSurface[(mx + my * MinimapTextureWidth) * 4]) =
					VideoMapRGB(0, 0, 0, 0);
#endif
			}
		}
	}
#ifndef USE_OPENGL
	SDL_UnlockSurface(MinimapTerrainSurface);
#endif

	//
	// Draw units on map
	// FIXME: We should rewrite this completely
	//
	for (n = 0; n < NumUnits; ++n) {
		if (UnitVisibleOnMinimap(Units[n])) {
			DrawUnitOnMinimap(Units[n], red_phase);
		}
	}

#ifndef USE_OPENGL
	SDL_UnlockSurface(MinimapSurface);
#endif
}

/**
**  Draw the minimap events
*/
static void DrawMinimapEvents(void)
{
	int i;

	for (i = 0; i < NumMinimapEvents; ++i) {
		VideoDrawTransCircleClip(ColorWhite,
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
void DrawMinimap(int vx __attribute__((unused)),
	int vy __attribute__((unused)))
{
#ifndef USE_OPENGL
	SDL_Rect drect;

	drect.x = TheUI.MinimapPosX;
	drect.y = TheUI.MinimapPosY;

	SDL_BlitSurface(MinimapSurface, NULL, TheScreen, &drect);
#else
	glBindTexture(GL_TEXTURE_2D, MinimapTexture);
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, MinimapTextureWidth, MinimapTextureHeight,
		GL_RGBA, GL_UNSIGNED_BYTE, MinimapSurface);

	glBegin(GL_QUADS);
	glTexCoord2f(0.0f, 0.0f);
	glVertex2i(TheUI.MinimapPosX, TheUI.MinimapPosY);
	glTexCoord2f(0.0f, (float)TheUI.MinimapH / MinimapTextureHeight);
	glVertex2i(TheUI.MinimapPosX, TheUI.MinimapPosY + TheUI.MinimapH);
	glTexCoord2f((float)TheUI.MinimapW / MinimapTextureWidth, (float)TheUI.MinimapH / MinimapTextureHeight);
	glVertex2i(TheUI.MinimapPosX + TheUI.MinimapW, TheUI.MinimapPosY + TheUI.MinimapH);
	glTexCoord2f((float)TheUI.MinimapW / MinimapTextureWidth, 0.0f);
	glVertex2i(TheUI.MinimapPosX + TheUI.MinimapW, TheUI.MinimapPosY);
	glEnd();
#endif

	DrawMinimapEvents();
}


/**
**  Convert minimap cursor X position to tile map coordinate.
**
**  @param x  Screen X pixel coordinate.
**
**  @return   Tile X coordinate.
*/
int ScreenMinimap2MapX(int x)
{
	int tx;

	tx = (((x - TheUI.MinimapPosX - MinimapX) * MINIMAP_FAC) / MinimapScaleX);
	if (tx < 0) {
		return 0;
	}
	return tx < TheMap.Info.MapWidth ? tx : TheMap.Info.MapWidth - 1;
}

/**
**  Convert minimap cursor Y position to tile map coordinate.
**
**  @param y  Screen Y pixel coordinate.
**
**  @return   Tile Y coordinate.
*/
int ScreenMinimap2MapY(int y)
{
	int ty;

	ty = (((y - TheUI.MinimapPosY - MinimapY) * MINIMAP_FAC) / MinimapScaleY);
	if (ty < 0) {
		return 0;
	}
	return ty < TheMap.Info.MapHeight ? ty : TheMap.Info.MapHeight - 1;
}

/**
**  Destroy mini-map.
*/
void DestroyMinimap(void)
{
#ifndef USE_OPENGL
	SDL_FreeSurface(MinimapTerrainSurface);
#else
	free(MinimapTerrainSurface);
#endif
	MinimapTerrainSurface = NULL;
	if (MinimapSurface) {
#ifndef USE_OPENGL
		SDL_FreeSurface(MinimapSurface);
#else
		glDeleteTextures(1, &MinimapTexture);
		free(MinimapSurface);
#endif
		MinimapSurface = NULL;
	}
	free(Minimap2MapX);
	Minimap2MapX = NULL;
	free(Minimap2MapY);
	Minimap2MapY = NULL;
}

/**
**  Draw minimap cursor.
**
**  @param vx  View point X position.
**  @param vy  View point Y position.
*/
void DrawMinimapCursor(int vx, int vy)
{
	int x;
	int y;
	int w;
	int h;
	int i;

	// Determine and save region below minimap cursor
	x = TheUI.MinimapPosX + MinimapX + (vx * MinimapScaleX) / MINIMAP_FAC;
	y = TheUI.MinimapPosY + MinimapY + (vy * MinimapScaleY) / MINIMAP_FAC;
	w = (TheUI.SelectedViewport->MapWidth * MinimapScaleX) / MINIMAP_FAC;
	h = (TheUI.SelectedViewport->MapHeight * MinimapScaleY) / MINIMAP_FAC;

	i = (w + 1 + h) * 2 * TheScreen->format->BytesPerPixel;

	// Draw cursor as rectangle (Note: unclipped, as it is always visible)
	VideoDrawTransRectangle(TheUI.ViewportCursorColor, x, y, w, h, 128);
}

/**
**  Add a minimap event
**
**  @param x  Map X tile position
**  @param y  Map Y tile position
*/
void AddMinimapEvent(int x, int y)
{
	if (NumMinimapEvents == MAX_MINIMAP_EVENTS) {
		return;
	}

	MinimapEvents[NumMinimapEvents].X = TheUI.MinimapPosX + MinimapX + (x * MinimapScaleX) / MINIMAP_FAC;
	MinimapEvents[NumMinimapEvents].Y = TheUI.MinimapPosY + MinimapY + (y * MinimapScaleY) / MINIMAP_FAC;
	MinimapEvents[NumMinimapEvents].Size =
		(TheUI.MinimapW < TheUI.MinimapH) ? TheUI.MinimapW / 3 : TheUI.MinimapH / 3;

	++NumMinimapEvents;
}

//@}
