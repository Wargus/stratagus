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
//      (c) Copyright 1998-2011 by Lutz Sammer and Jimmy Salmon and Pali Rohár
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

#include "minimap.h"

#include "editor.h"
#include "map.h"
#include "player.h"
#include "unit.h"
#include "unit_manager.h"
#include "ui.h"
#include "unittype.h"
#include "video.h"

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

unsigned char *MinimapSurfaceGL;
unsigned char *MinimapTerrainSurfaceGL;
SDL_Surface *MinimapSurface;        /// generated minimap
SDL_Surface *MinimapTerrainSurface; /// generated minimap terrain

static GLuint MinimapTexture;
static int MinimapTextureWidth;
static int MinimapTextureHeight;

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
	PixelPos pos;
	int Size;
	Uint32 Color;
} MinimapEvents[MAX_MINIMAP_EVENTS];
int NumMinimapEvents;


/*----------------------------------------------------------------------------
-- Functions
----------------------------------------------------------------------------*/


/**
**  Create the minimap texture
*/
static void CreateMinimapTexture()
{
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glGenTextures(1, &MinimapTexture);
	glBindTexture(GL_TEXTURE_2D, MinimapTexture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
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
void CMinimap::Create()
{
	// Scale to biggest value.
	const int n = std::max(Map.Info.MapWidth, Map.Info.MapHeight);

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
	Minimap2MapX = new int[W * H];
	memset(Minimap2MapX, 0, W * H * sizeof(int));
	Minimap2MapY = new int[W * H];
	memset(Minimap2MapY, 0, W * H * sizeof(int));
	for (int i = XOffset; i < W - XOffset; ++i) {
		Minimap2MapX[i] = ((i - XOffset) * MINIMAP_FAC) / MinimapScaleX;
	}
	for (int i = YOffset; i < H - YOffset; ++i) {
		Minimap2MapY[i] = (((i - YOffset) * MINIMAP_FAC) / MinimapScaleY) * Map.Info.MapWidth;
	}
	for (int i = 0; i < Map.Info.MapWidth; ++i) {
		Map2MinimapX[i] = (i * MinimapScaleX) / MINIMAP_FAC;
	}
	for (int i = 0; i < Map.Info.MapHeight; ++i) {
		Map2MinimapY[i] = (i * MinimapScaleY) / MINIMAP_FAC;
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
**  Calculate the tile graphic pixel
*/
static inline Uint8 *GetTileGraphicPixel(int xofs, int yofs, int mx, int my, int scalex, int scaley, int bpp)
{
	Uint8 *pixels = (Uint8 *)Map.TileGraphic->Surface->pixels;
	int x = (xofs + 7 + ((mx * SCALE_PRECISION) % scalex) / SCALE_PRECISION * 8);
	int y = (yofs + 6 + ((my * SCALE_PRECISION) % scaley) / SCALE_PRECISION * 8);
	return &pixels[x * bpp + y * Map.TileGraphic->Surface->pitch];
}

/**
**  Update a mini-map from the tiles of the map.
*/
void CMinimap::UpdateTerrain()
{
	int scalex = MinimapScaleX * SCALE_PRECISION / MINIMAP_FAC;
	if (!scalex) {
		scalex = 1;
	}
	int scaley = MinimapScaleY * SCALE_PRECISION / MINIMAP_FAC;
	if (!scaley) {
		scaley = 1;
	}
	const int bpp = Map.TileGraphic->Surface->format->BytesPerPixel;

	if (!UseOpenGL) {
		if (bpp == 1) {
			SDL_SetPalette(MinimapTerrainSurface, SDL_LOGPAL,
						   Map.TileGraphic->Surface->format->palette->colors, 0, 256);
		}
	}

	const int tilepitch = Map.TileGraphic->Surface->w / PixelTileSize.x;

	if (!UseOpenGL) {
		SDL_LockSurface(MinimapTerrainSurface);
	} else {
		SDL_LockSurface(Map.TileGraphic->Surface);
	}

	//
	//  Pixel 7,6 7,14, 15,6 15,14 are taken for the minimap picture.
	//
	for (int my = YOffset; my < H - YOffset; ++my) {
		for (int mx = XOffset; mx < W - XOffset; ++mx) {
			const int tile = Map.Fields[Minimap2MapX[mx] + Minimap2MapY[my]].Tile;
			const int xofs = PixelTileSize.x * (tile % tilepitch);
			const int yofs = PixelTileSize.y * (tile / tilepitch);

			if (!UseOpenGL) {
				if (bpp == 1) {
					((Uint8 *)MinimapTerrainSurface->pixels)[mx + my * MinimapTerrainSurface->pitch] =
						*GetTileGraphicPixel(xofs, yofs, mx, my, scalex, scaley, bpp);
				} else if (bpp == 3) {
					Uint8 *d = &((Uint8 *)MinimapTerrainSurface->pixels)[mx * bpp + my * MinimapTerrainSurface->pitch];
					Uint8 *s = GetTileGraphicPixel(xofs, yofs, mx, my, scalex, scaley, bpp);
					*d++ = *s++;
					*d++ = *s++;
					*d++ = *s++;
				} else {
					*(Uint32 *)&((Uint8 *)MinimapTerrainSurface->pixels)[mx * bpp + my * MinimapTerrainSurface->pitch] =
						*(Uint32 *)GetTileGraphicPixel(xofs, yofs, mx, my, scalex, scaley, bpp);
				}
			} else {
				Uint32 c;

				if (bpp == 1) {
					SDL_Color color = Map.TileGraphic->Surface->format->palette->colors[
										  *GetTileGraphicPixel(xofs, yofs, mx, my, scalex, scaley, bpp)];
					c = Video.MapRGB(0, color.r, color.g, color.b);
				} else {
					SDL_PixelFormat *f = Map.TileGraphic->Surface->format;
					c = *(Uint32 *)GetTileGraphicPixel(xofs, yofs, mx, my, scalex, scaley, bpp);
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
**  @param pos  The map position to update in the minimap
*/
void CMinimap::UpdateXY(const Vec2i &pos)
{
	if (!UseOpenGL) {
		if (!MinimapTerrainSurface) {
			return;
		}
	} else {
		if (!MinimapTerrainSurfaceGL) {
			return;
		}
	}

	int scalex = MinimapScaleX * SCALE_PRECISION / MINIMAP_FAC;
	if (scalex == 0) {
		scalex = 1;
	}
	int scaley = MinimapScaleY * SCALE_PRECISION / MINIMAP_FAC;
	if (scaley == 0) {
		scaley = 1;
	}

	const int tilepitch = Map.TileGraphic->Surface->w / PixelTileSize.x;
	const int bpp = Map.TileGraphic->Surface->format->BytesPerPixel;

	//
	//  Pixel 7,6 7,14, 15,6 15,14 are taken for the minimap picture.
	//
	if (!UseOpenGL) {
		SDL_LockSurface(MinimapTerrainSurface);
	}
	SDL_LockSurface(Map.TileGraphic->Surface);

	const int ty = pos.y * Map.Info.MapWidth;
	const int tx = pos.x;
	for (int my = YOffset; my < H - YOffset; ++my) {
		const int y = Minimap2MapY[my];
		if (y < ty) {
			continue;
		}
		if (y > ty) {
			break;
		}

		for (int mx = XOffset; mx < W - XOffset; ++mx) {
			const int x = Minimap2MapX[mx];

			if (x < tx) {
				continue;
			}
			if (x > tx) {
				break;
			}

			int tile = Map.Fields[x + y].playerInfo.SeenTile;
			if (!tile) {
				tile = Map.Fields[x + y].Tile;
			}

			const int xofs = PixelTileSize.x * (tile % tilepitch);
			const int yofs = PixelTileSize.y * (tile / tilepitch);

			if (!UseOpenGL) {
				const int index = mx * bpp + my * MinimapTerrainSurface->pitch;
				Uint8 *s = GetTileGraphicPixel(xofs, yofs, mx, my, scalex, scaley, bpp);
				if (bpp == 1) {
					((Uint8 *)MinimapTerrainSurface->pixels)[index] = *s;
				} else if (bpp == 3) {
					Uint8 *d = &((Uint8 *)MinimapTerrainSurface->pixels)[index];

					*d++ = *s++;
					*d++ = *s++;
					*d++ = *s++;
				} else {
					*(Uint32 *)&((Uint8 *)MinimapTerrainSurface->pixels)[index] = *(Uint32 *)s;
				}
			} else {
				Uint32 c;

				if (bpp == 1) {
					const int colorIndex = *GetTileGraphicPixel(xofs, yofs, mx, my, scalex, scaley, bpp);
					const SDL_Color color = Map.TileGraphic->Surface->format->palette->colors[colorIndex];

					c = Video.MapRGB(0, color.r, color.g, color.b);
				} else {
					SDL_PixelFormat *f = Map.TileGraphic->Surface->format;

					c = *(Uint32 *)GetTileGraphicPixel(xofs, yofs, mx, my, scalex, scaley, bpp);
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
static void DrawUnitOn(CUnit &unit, int red_phase)
{
	const CUnitType *type;

	if (Editor.Running || ReplayRevealMap || unit.IsVisible(*ThisPlayer)) {
		type = unit.Type;
	} else {
		type = unit.Seen.Type;
		// This will happen for radar if the unit has not been seen and we
		// have it on radar.
		if (!type) {
			type = unit.Type;
		}
	}

	Uint32 color;
	if (unit.Player->Index == PlayerNumNeutral) {
		color = Video.MapRGB(TheScreen->format, type->NeutralMinimapColorRGB);
	} else if (unit.Player == ThisPlayer && !Editor.Running) {
		if (unit.Attacked && unit.Attacked + ATTACK_BLINK_DURATION > GameCycle &&
			(red_phase || unit.Attacked + ATTACK_RED_DURATION > GameCycle)) {
			color = ColorRed;
		} else if (UI.Minimap.ShowSelected && unit.Selected) {
			color = ColorWhite;
		} else {
			color = ColorGreen;
		}
	} else {
		color = unit.Player->Color;
	}

	int mx = 1 + UI.Minimap.XOffset + Map2MinimapX[unit.tilePos.x];
	int my = 1 + UI.Minimap.YOffset + Map2MinimapY[unit.tilePos.y];
	int w = Map2MinimapX[type->TileWidth];
	if (mx + w >= UI.Minimap.W) { // clip right side
		w = UI.Minimap.W - mx;
	}
	int h0 = Map2MinimapY[type->TileHeight];
	if (my + h0 >= UI.Minimap.H) { // clip bottom side
		h0 = UI.Minimap.H - my;
	}
	int bpp = 0;
	if (!UseOpenGL) {
		SDL_Color c;
		bpp = MinimapSurface->format->BytesPerPixel;
		SDL_GetRGB(color, TheScreen->format, &c.r, &c.g, &c.b);
	}
	while (w-- >= 0) {
		int h = h0;
		while (h-- >= 0) {
			if (!UseOpenGL) {
				const unsigned int index = (mx + w) * bpp + (my + h) * MinimapSurface->pitch;
				if (bpp == 2) {
					*(Uint16 *)&((Uint8 *)MinimapSurface->pixels)[index] = color;
				} else {
					*(Uint32 *)&((Uint8 *)MinimapSurface->pixels)[index] = color;
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
void CMinimap::Update()
{
	static int red_phase;

	int red_phase_changed = red_phase != (int)((FrameCounter / FRAMES_PER_SECOND) & 1);
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
	const int bpp = (!UseOpenGL) ? MinimapSurface->format->BytesPerPixel : 0;

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

	for (int my = 0; my < H; ++my) {
		for (int mx = 0; mx < W; ++mx) {
			int visiontype; // 0 unexplored, 1 explored, >1 visible.

			if (ReplayRevealMap) {
				visiontype = 2;
			} else {
				const Vec2i tilePos(Minimap2MapX[mx], Minimap2MapY[my] / Map.Info.MapWidth);
				visiontype = Map.Field(tilePos)->playerInfo.TeamVisibilityState(*ThisPlayer);
			}

			if (visiontype == 0 || (visiontype == 1 && ((mx & 1) != (my & 1)))) {
				if (!UseOpenGL) {
					const int index = mx * bpp + my * MinimapSurface->pitch;
					if (bpp == 2) {
						*(Uint16 *)&((Uint8 *)MinimapSurface->pixels)[index] = ColorBlack;
					} else {
						*(Uint32 *)&((Uint8 *)MinimapSurface->pixels)[index] = ColorBlack;
					}
				} else {
					*(Uint32 *)&(MinimapSurfaceGL[(mx + my * MinimapTextureWidth) * 4]) = Video.MapRGB(0, 0, 0, 0);
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
	for (CUnitManager::Iterator it = UnitManager.begin(); it != UnitManager.end(); ++it) {
		CUnit &unit = **it;
		if (unit.IsVisibleOnMinimap()) {
			DrawUnitOn(unit, red_phase);
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
	const unsigned char alpha = 192;

	for (int i = 0; i < NumMinimapEvents; ++i) {
		Video.DrawTransCircleClip(MinimapEvents[i].Color,
								  MinimapEvents[i].pos.x, MinimapEvents[i].pos.y,
								  MinimapEvents[i].Size, alpha);
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
void CMinimap::Draw() const
{
	if (!UseOpenGL) {
		SDL_Rect drect = {X, Y, 0, 0};
		SDL_BlitSurface(MinimapSurface, NULL, TheScreen, &drect);
	} else {
		glBindTexture(GL_TEXTURE_2D, MinimapTexture);
		glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, MinimapTextureWidth, MinimapTextureHeight,
						GL_RGBA, GL_UNSIGNED_BYTE, MinimapSurfaceGL);

#ifdef USE_GLES
		float texCoord[] = {
			0.0f, 0.0f,
			(float)W / MinimapTextureWidth, 0.0f,
			0.0f, (float)H / MinimapTextureHeight,
			(float)W / MinimapTextureWidth, (float)H / MinimapTextureHeight
		};

		float vertex[] = {
			2.0f / (GLfloat)Video.Width *X - 1.0f, -2.0f / (GLfloat)Video.Height *Y + 1.0f,
			2.0f / (GLfloat)Video.Width *(X + W) - 1.0f, -2.0f / (GLfloat)Video.Height *Y + 1.0f,
			2.0f / (GLfloat)Video.Width *X - 1.0f, -2.0f / (GLfloat)Video.Height *(Y + H) + 1.0f,
			2.0f / (GLfloat)Video.Width *(X + W) - 1.0f, -2.0f / (GLfloat)Video.Height *(Y + H) + 1.0f
		};

		glEnableClientState(GL_TEXTURE_COORD_ARRAY);
		glEnableClientState(GL_VERTEX_ARRAY);

		glTexCoordPointer(2, GL_FLOAT, 0, texCoord);
		glVertexPointer(2, GL_FLOAT, 0, vertex);
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

		glDisableClientState(GL_TEXTURE_COORD_ARRAY);
		glDisableClientState(GL_VERTEX_ARRAY);
#else
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
	}

	DrawEvents();
}

/**
**  Convert screen position to tile map coordinate.
**
**  @param screenPos  Screen pixel coordinate.
**
**  @return   Tile coordinate.
*/
Vec2i CMinimap::ScreenToTilePos(const PixelPos &screenPos) const
{
	Vec2i tilePos((((screenPos.x - X - XOffset) * MINIMAP_FAC) / MinimapScaleX),
				  (((screenPos.y - Y - YOffset) * MINIMAP_FAC) / MinimapScaleY));

	Map.Clamp(tilePos);
	return tilePos;
}

/**
**  Convert tile map coordinate to screen position.
**
**  @param tilePos  Tile coordinate.
**
**  @return   Screen pixel coordinate.
*/
PixelPos CMinimap::TilePosToScreenPos(const Vec2i &tilePos) const
{
	const PixelPos screenPos(X + XOffset + (tilePos.x * MinimapScaleX) / MINIMAP_FAC,
							 Y + YOffset + (tilePos.y * MinimapScaleY) / MINIMAP_FAC);
	return screenPos;
}

/**
**  Destroy mini-map.
*/
void CMinimap::Destroy()
{
	if (!UseOpenGL) {
		VideoPaletteListRemove(MinimapTerrainSurface);
		SDL_FreeSurface(MinimapTerrainSurface);
		MinimapTerrainSurface = NULL;
	} else {
		delete[] MinimapTerrainSurfaceGL;
		MinimapTerrainSurfaceGL = NULL;
	}
	if (!UseOpenGL) {
		if (MinimapSurface) {
			VideoPaletteListRemove(MinimapSurface);
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
**  Draw viewport area contour.
*/
void CMinimap::DrawViewportArea(const CViewport &viewport) const
{
	// Determine and save region below minimap cursor
	const PixelPos screenPos = TilePosToScreenPos(viewport.MapPos);
	int w = (viewport.MapWidth * MinimapScaleX) / MINIMAP_FAC;
	int h = (viewport.MapHeight * MinimapScaleY) / MINIMAP_FAC;

	// Draw cursor as rectangle (Note: unclipped, as it is always visible)
	Video.DrawTransRectangle(UI.ViewportCursorColor, screenPos.x, screenPos.y, w, h, 128);
}

/**
**  Add a minimap event
**
**  @param pos  Map tile position
*/
void CMinimap::AddEvent(const Vec2i &pos, Uint32 color)
{
	if (NumMinimapEvents == MAX_MINIMAP_EVENTS) {
		return;
	}
	MinimapEvents[NumMinimapEvents].pos = TilePosToScreenPos(pos);
	MinimapEvents[NumMinimapEvents].Size = (W < H) ? W / 3 : H / 3;
	MinimapEvents[NumMinimapEvents].Color = color;
	++NumMinimapEvents;
}

bool CMinimap::Contains(const PixelPos &screenPos) const
{
	return this->X <= screenPos.x && screenPos.x < this->X + this->W
		   && this->Y <= screenPos.y && screenPos.y < this->Y + this->H;
}

//@}
