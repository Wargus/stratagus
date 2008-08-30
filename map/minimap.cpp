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
//      (c) Copyright 1998-2008 by Lutz Sammer and Jimmy Salmon
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

#include <string.h>

#include "stratagus.h"
#include "video.h"
#include "map.h"
#include "minimap.h"
#include "unittype.h"
#include "player.h"
#include "unit.h"
#include "ui.h"
#include "editor.h"
#include "patch.h"
#include "patch_type.h"

/*----------------------------------------------------------------------------
--  Defines
----------------------------------------------------------------------------*/

#define MINIMAP_FAC (16 * 3)  /// integer scale factor

	/// unit attacked are shown red for at least this amount of cycles
#define ATTACK_RED_DURATION (1 * CYCLES_PER_SECOND)
	/// unit attacked are shown blinking for this amount of cycles
#define ATTACK_BLINK_DURATION (7 * CYCLES_PER_SECOND)

#define SCALE_PRECISION 100


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
	int maxSize = std::max(Map.Info.MapWidth, Map.Info.MapHeight);

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
**  Get the color from the patch to be used in the minimap
*/
CColor GetColor(CPatch *patch, int xoffset, int yoffset, int mx, int my, int scalex, int scaley)
{
	//
	//  Pixel 7,6 7,14, 15,6 15,14 are taken for the minimap picture.
	//
	int x = (xoffset * TileSizeX + 7 + ((mx * SCALE_PRECISION) % scalex) / SCALE_PRECISION * 8);
	int y = (yoffset * TileSizeY + 6 + ((my * SCALE_PRECISION) % scaley) / SCALE_PRECISION * 8);

	Uint8 r, g, b, a;
	SDL_Surface *s = patch->getType()->getGraphic()->Surface;
	SDL_PixelFormat *f = s->format;

	//
	// Convert the pixel to r,g,b,a
	//
	Uint8 *pixel = &((Uint8 *)s->pixels)[x * f->BytesPerPixel + y * s->pitch];
	switch (f->BytesPerPixel) {
		case 1:
			SDL_GetRGBA(*pixel, f, &r, &g, &b, &a);
			break;
		case 2:
			SDL_GetRGBA(*(Uint16 *)pixel, f, &r, &g, &b, &a);
			break;
		case 3:
			r = pixel[f->Rshift >> 3];
			g = pixel[f->Gshift >> 3];
			b = pixel[f->Bshift >> 3];
			a = 0;
			break;
		case 4:
			SDL_GetRGBA(*(Uint32 *)pixel, f, &r, &g, &b, &a);
			break;
	}

	return CColor(r, g, b, a);
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

/**
**  Update a mini-map from the tiles of the map.
*/
void CMinimap::UpdateTerrain()
{
	int scalex = MinimapScaleX * SCALE_PRECISION / MINIMAP_FAC;
	if (scalex == 0) {
		scalex = 1;
	}
	int scaley = MinimapScaleY * SCALE_PRECISION / MINIMAP_FAC;
	if (scaley == 0) {
		scaley = 1;
	}

	if (!UseOpenGL) {
		SDL_LockSurface(MinimapTerrainSurface);
	}

	for (int my = YOffset; my < H - YOffset; ++my) {
		for (int mx = XOffset; mx < W - XOffset; ++mx) {
			CPatch *patch;
			int xoffset, yoffset;
			CColor color = ColorBlack;

			patch = Map.PatchManager.getPatch(Minimap2MapX[mx], Minimap2MapY[my], &xoffset, &yoffset);
			if (patch) {
				color = GetColor(patch, xoffset, yoffset, mx, my, scalex, scaley);
			}
			SetMinimapTerrainPixel(mx, my, color);
		}
	}

	if (!UseOpenGL) {
		SDL_UnlockSurface(MinimapTerrainSurface);
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

	// Draw the terrain
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
