//     ____                _       __
//    / __ )____  _____   | |     / /___ ___________
//   / __  / __ \/ ___/   | | /| / / __ `/ ___/ ___/
//  / /_/ / /_/ (__  )    | |/ |/ / /_/ / /  (__  )
// /_____/\____/____/     |__/|__/\__,_/_/  /____/
//
//       A futuristic real-time strategy game.
//          This file is part of Bos Wars.
//
/**@name minimap.cpp - The minimap. */
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
**  Create a mini-map from the tiles of the map.
**
**  @todo Scaling and scrolling the minmap is currently not supported.
*/
void CMinimap::Create(void)
{
	int n;

	if (Map.Info.MapWidth > Map.Info.MapHeight) { // Scale to biggest value.
		n = Map.Info.MapWidth;
	} else {
		n = Map.Info.MapHeight;
	}
	MinimapScaleX = (W * MINIMAP_FAC + n - 1) / n;
	MinimapScaleY = (H * MINIMAP_FAC + n - 1) / n;

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
	Minimap2MapX = new int[W * H];
	memset(Minimap2MapX, 0, W * H * sizeof(int));
	Minimap2MapY = new int[W * H];
	memset(Minimap2MapY, 0, W * H * sizeof(int));
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
	if (!UseOpenGL) {
		SDL_PixelFormat *f = Map.TileGraphic->Surface->format;
		MinimapTerrainSurface = SDL_CreateRGBSurface(SDL_SWSURFACE,
			W, H, f->BitsPerPixel, f->Rmask, f->Gmask, f->Bmask, f->Amask);
		MinimapSurface = SDL_CreateRGBSurface(SDL_SWSURFACE,
			W, H, 32, TheScreen->format->Rmask, TheScreen->format->Gmask,
			TheScreen->format->Bmask, 0);
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
**  Reload OpenGL minimap
*/
void CMinimap::Reload(void)
{
	CreateMinimapTexture();
}

/**
**  Update a mini-map from the tiles of the map.
**
**  FIXME: this can surely be sped up??
*/
void CMinimap::UpdateTerrain(void)
{
	int mx;
	int my;
	int scalex;
	int scaley;
	int tilepitch;
	int xofs;
	int yofs;
	int bpp;
	const int scalePrecision = 100;

	if (!(scalex = (MinimapScaleX * scalePrecision / MINIMAP_FAC))) {
		scalex = 1;
	}
	if (!(scaley = (MinimapScaleY * scalePrecision / MINIMAP_FAC))) {
		scaley = 1;
	}
	bpp = Map.TileGraphic->Surface->format->BytesPerPixel;

	if (!UseOpenGL) {
		if (bpp == 1) {
			SDL_SetPalette(MinimapTerrainSurface, SDL_LOGPAL,
				Map.TileGraphic->Surface->format->palette->colors, 0, 256);
		}
	}

	tilepitch = Map.TileGraphic->Surface->w / TileSizeX;

	if (!UseOpenGL) {
		SDL_LockSurface(MinimapTerrainSurface);
	} else {
		SDL_LockSurface(Map.TileGraphic->Surface);
	}

	//
	//  Pixel 7,6 7,14, 15,6 15,14 are taken for the minimap picture.
	//
	for (my = YOffset; my < H - YOffset; ++my) {
		for (mx = XOffset; mx < W - XOffset; ++mx) {
			int tile;
			Uint32 c;

			tile = Map.Fields[Minimap2MapX[mx] + Minimap2MapY[my]].Tile;

			xofs = TileSizeX * (tile % tilepitch);
			yofs = TileSizeY * (tile / tilepitch);

			if (!UseOpenGL) {
				if (bpp == 1) {
					((Uint8 *)MinimapTerrainSurface->pixels)[mx + my * MinimapTerrainSurface->pitch] =
						((Uint8 *)Map.TileGraphic->Surface->pixels)[
							xofs + 7 + ((mx * scalePrecision) % scalex) / scalePrecision * 8 +
							(yofs + 6 + ((my * scalePrecision) % scaley) / scalePrecision * 8) *
							Map.TileGraphic->Surface->pitch];
				} else if (bpp == 3) {
					Uint8 *d;
					Uint8 *s;

					d = &((Uint8 *)MinimapTerrainSurface->pixels)[mx * bpp + my * MinimapTerrainSurface->pitch];
					s = &((Uint8 *)Map.TileGraphic->Surface->pixels)[
							(xofs + 7 + ((mx * scalePrecision) % scalex) / scalePrecision * 8) * bpp +
							(yofs + 6 + ((my * scalePrecision) % scaley) / scalePrecision * 8) *
							Map.TileGraphic->Surface->pitch];
					*d++ = *s++;
					*d++ = *s++;
					*d++ = *s++;
				} else {
					*(Uint32 *)&((Uint8 *)MinimapTerrainSurface->pixels)[mx * bpp + my * MinimapTerrainSurface->pitch] =
						*(Uint32 *)&((Uint8 *)Map.TileGraphic->Surface->pixels)[
							(xofs + 7 + ((mx * scalePrecision) % scalex) / scalePrecision * 8) * bpp +
							(yofs + 6 + ((my * scalePrecision) % scaley) / scalePrecision * 8) *
							Map.TileGraphic->Surface->pitch];
				}
			} else {
				if (bpp == 1) {
					SDL_Color color;

					color = Map.TileGraphic->Surface->format->palette->colors[
						((Uint8 *)Map.TileGraphic->Surface->pixels)[
							xofs + 7 + ((mx * scalePrecision) % scalex) / scalePrecision * 8 +
							(yofs + 6 + ((my * scalePrecision) % scaley) / scalePrecision * 8) *
							Map.TileGraphic->Surface->pitch]];
					c = Video.MapRGB(0, color.r, color.g, color.b);
				} else {
					SDL_PixelFormat *f;

					f = Map.TileGraphic->Surface->format;
					c = *(Uint32 *)&((Uint8 *)Map.TileGraphic->Surface->pixels)[
						(xofs + 7 + ((mx * scalePrecision) % scalex) / scalePrecision * 8) * bpp +
						(yofs + 6 + ((my * scalePrecision) % scaley) / scalePrecision * 8) *
						Map.TileGraphic->Surface->pitch];
					c = Video.MapRGB(0,
						((c & f->Rmask) >> f->Rshift),
						((c & f->Gmask) >> f->Gshift),
						((c & f->Bmask) >> f->Bshift));
				}
				*(Uint32 *)&(MinimapTerrainSurfaceGL[(mx + my * MinimapTextureWidth) * 4]) = c;
			}
		}
	}

	if (!UseOpenGL) {
		SDL_UnlockSurface(MinimapTerrainSurface);
	}
	SDL_UnlockSurface(Map.TileGraphic->Surface);
}

/**
**  Update a single minimap tile after a change
**
**  @param tx  The X map position to update in the minimap
**  @param ty  The Y map position to update in the minimap
*/
void CMinimap::UpdateXY(int tx, int ty)
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
	const int scalePrecision = 100;

	if (!UseOpenGL) {
		if (!MinimapTerrainSurface) {
			return;
		}
	} else {
		if (!MinimapTerrainSurfaceGL) {
			return;
		}
	}

	scalex = MinimapScaleX * scalePrecision / MINIMAP_FAC;
	if (scalex == 0) {
		scalex = 1;
	}
	scaley = MinimapScaleY * scalePrecision / MINIMAP_FAC;
	if (scaley == 0) {
		scaley = 1;
	}

	tilepitch = Map.TileGraphic->Surface->w / TileSizeX;
	bpp = Map.TileGraphic->Surface->format->BytesPerPixel;

	//
	//  Pixel 7,6 7,14, 15,6 15,14 are taken for the minimap picture.
	//
	if (!UseOpenGL) {
		SDL_LockSurface(MinimapTerrainSurface);
	}
	SDL_LockSurface(Map.TileGraphic->Surface);

	ty *= Map.Info.MapWidth;
	for (my = YOffset; my < H - YOffset; ++my) {
		y = Minimap2MapY[my];
		if (y < ty) {
			continue;
		}
		if (y > ty) {
			break;
		}

		for (mx = XOffset; mx < W - XOffset; ++mx) {
			int tile;
			Uint32 c;

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

			xofs = TileSizeX * (tile % tilepitch);
			yofs = TileSizeY * (tile / tilepitch);

			if (!UseOpenGL) {
				if (bpp == 1) {
					((Uint8 *)MinimapTerrainSurface->pixels)[mx + my * MinimapTerrainSurface->pitch] =
						((Uint8 *)Map.TileGraphic->Surface->pixels)[
							xofs + 7 + ((mx * scalePrecision) % scalex) / scalePrecision * 8 +
							(yofs + 6 + ((my * scalePrecision) % scaley) / scalePrecision * 8) *
							Map.TileGraphic->Surface->pitch];
				} else if (bpp == 3) {
					Uint8 *d;
					Uint8 *s;

					d = &((Uint8 *)MinimapTerrainSurface->pixels)[mx * bpp + my * MinimapTerrainSurface->pitch];
					s = &((Uint8 *)Map.TileGraphic->Surface->pixels)[
							(xofs + 7 + ((mx * scalePrecision) % scalex) / scalePrecision * 8) * bpp +
							(yofs + 6 + ((my * scalePrecision) % scaley) / scalePrecision * 8) *
							Map.TileGraphic->Surface->pitch];
					*d++ = *s++;
					*d++ = *s++;
					*d++ = *s++;
				} else {
					*(Uint32 *)&((Uint8 *)MinimapTerrainSurface->pixels)[mx * bpp + my * MinimapTerrainSurface->pitch] =
						*(Uint32 *)&((Uint8 *)Map.TileGraphic->Surface->pixels)[
							(xofs + 7 + ((mx * scalePrecision) % scalex) / scalePrecision * 8) * bpp +
							(yofs + 6 + ((my * scalePrecision) % scaley) / scalePrecision * 8) *
							Map.TileGraphic->Surface->pitch];
				}
			} else {
				if (bpp == 1) {
					SDL_Color color;

					color = Map.TileGraphic->Surface->format->palette->colors[
						((Uint8 *)Map.TileGraphic->Surface->pixels)[
							xofs + 7 + ((mx * scalePrecision) % scalex) / scalePrecision * 8 +
							(yofs + 6 + ((my * scalePrecision) % scaley) / scalePrecision * 8) *
							Map.TileGraphic->Surface->pitch]];
					c = Video.MapRGB(0, color.r, color.g, color.b);
				} else {
					SDL_PixelFormat *f;

					f = Map.TileGraphic->Surface->format;
					c = *(Uint32 *)&((Uint8 *)Map.TileGraphic->Surface->pixels)[
						(xofs + 7 + ((mx * scalePrecision) % scalex) / scalePrecision * 8) * 4 +
						(yofs + 6 + ((my * scalePrecision) % scaley) / scalePrecision * 8) *
						Map.TileGraphic->Surface->pitch];
					c = Video.MapRGB(0,
						((c & f->Rmask) >> f->Rshift),
						((c & f->Gmask) >> f->Gshift),
						((c & f->Bmask) >> f->Bshift));
				}
				*(Uint32 *)&(MinimapTerrainSurfaceGL[(mx + my * MinimapTextureWidth) * 4]) = c;
			}
		}
	}

	if (!UseOpenGL) {
		SDL_UnlockSurface(MinimapTerrainSurface);
	}
	SDL_UnlockSurface(Map.TileGraphic->Surface);
}

/**
**  Draw a unit on the minimap.
*/
static void DrawUnitOn(CUnit *unit, int red_phase)
{
	CUnitType *type;
	int mx;
	int my;
	int w;
	int h;
	int h0;
	Uint32 color;
	SDL_Color c;
	int bpp;

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
	h0 = Map2MinimapY[type->TileHeight];
	if (my + h0 >= UI.Minimap.H) { // clip bottom side
		h0 = UI.Minimap.H - my;
	}
	if (!UseOpenGL) {
		bpp = MinimapSurface->format->BytesPerPixel;
		SDL_GetRGB(color, TheScreen->format, &c.r, &c.g, &c.b);
	}
	while (w-- >= 0) {
		h = h0;
		while (h-- >= 0) {
			if (!UseOpenGL) {
				if (bpp == 2) {
					*(Uint16 *)&((Uint8*)MinimapSurface->pixels)[(mx + w) * bpp + (my + h) * MinimapSurface->pitch] =
						color;
				} else {
					*(Uint32 *)&((Uint8*)MinimapSurface->pixels)[(mx + w) * bpp + (my + h) * MinimapSurface->pitch] =
						color;
				}
			} else {
				*(Uint32 *)&(MinimapSurfaceGL[((mx + w) + (my + h) * MinimapTextureWidth) * 4]) = color;
			}
		}
	}
}

/**
**  Update the minimap with the current game information
*/
void CMinimap::Update(void)
{
	static int red_phase;
	int red_phase_changed;
	int mx;
	int my;
	int n;
	int visiontype; // 0 unexplored, 1 explored, >1 visible.
	int bpp;

	red_phase_changed = red_phase != (int)((FrameCounter / FRAMES_PER_SECOND) & 1);
	if (red_phase_changed) {
		red_phase = !red_phase;
	}

	// Clear Minimap background if not transparent
	if (!Transparent) {
		if (!UseOpenGL) {
			SDL_FillRect(MinimapSurface, NULL, SDL_MapRGB(MinimapSurface->format, 0, 0, 0));
		} else {
			memset(MinimapSurfaceGL, 0, MinimapTextureWidth * MinimapTextureHeight * 4);
		}
	}

	if (!UseOpenGL) {
		bpp = MinimapSurface->format->BytesPerPixel;
	}

	//
	// Draw the terrain
	//
	if (WithTerrain) {
		if (!UseOpenGL) {
			SDL_BlitSurface(MinimapTerrainSurface, NULL, MinimapSurface, NULL);
		} else {
			memcpy(MinimapSurfaceGL, MinimapTerrainSurfaceGL, MinimapTextureWidth * MinimapTextureHeight * 4);
		}
	}

	if (!UseOpenGL) {
		SDL_LockSurface(MinimapSurface);
		SDL_LockSurface(MinimapTerrainSurface);
	}
	for (my = 0; my < H; ++my) {
		for (mx = 0; mx < W; ++mx) {
			if (ReplayRevealMap) {
				visiontype = 2;
			} else {
				visiontype = Map.IsTileVisible(ThisPlayer, Minimap2MapX[mx], Minimap2MapY[my] / Map.Info.MapWidth);
			}

			if ( visiontype == 0 || (visiontype == 1 && ((mx & 1) != (my & 1))))  {
				if (!UseOpenGL) {
					if (bpp == 2) {
						*(Uint16 *)&((Uint8 *)MinimapSurface->pixels)[mx * bpp + my * MinimapSurface->pitch] =
							ColorBlack;
					} else {
						*(Uint32 *)&((Uint8 *)MinimapSurface->pixels)[mx * bpp + my * MinimapSurface->pitch] =
							ColorBlack;
					}
				} else {
					*(Uint32 *)&(MinimapSurfaceGL[(mx + my * MinimapTextureWidth) * 4]) =
						Video.MapRGB(0, 0, 0, 0);
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
	for (n = 0; n < NumUnits; ++n) {
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
	int tx;

	tx = (((x - X - XOffset) * MINIMAP_FAC) / MinimapScaleX);
	if (tx < 0) {
		return 0;
	}
	return std::min(tx, Map.Info.MapWidth - 1);
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
	int ty;

	ty = (((y - Y - YOffset) * MINIMAP_FAC) / MinimapScaleY);
	if (ty < 0) {
		return 0;
	}
	return std::min(ty, Map.Info.MapHeight - 1);
}

/**
**  Destroy mini-map.
*/
void CMinimap::Destroy(void)
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

	MinimapEvents[NumMinimapEvents].X = X + XOffset + (x * MinimapScaleX) / MINIMAP_FAC;
	MinimapEvents[NumMinimapEvents].Y = Y + YOffset + (y * MinimapScaleY) / MINIMAP_FAC;
	MinimapEvents[NumMinimapEvents].Size = (W < H) ? W / 3 : H / 3;

	++NumMinimapEvents;
}

//@}
