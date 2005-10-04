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
/**@name map_fog.cpp - The map fog of war handling. */
//
//      (c) Copyright 1999-2005 by Lutz Sammer, Vladi Shabanski,
//                                 Russell Smith, and Jimmy Salmon
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
#include "player.h"
#include "unittype.h"
#include "unit.h"
#include "map.h"
#include "tileset.h"
#include "minimap.h"
#include "font.h"
#include "ui.h"
#include "../video/intern_video.h"

/*----------------------------------------------------------------------------
--  Declarations
----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
--  Variables
----------------------------------------------------------------------------*/

int FogOfWarOpacity;                 /// Fog of war Opacity.

/**
**  Mapping for fog of war tiles.
*/
static const int FogTable[16] = {
	 0,11,10, 2,  13, 6, 14, 3,  12, 15, 4, 1,  8, 9, 7, 0,
};

unsigned char *VisionTable[3];
int *VisionLookup;

static unsigned char *VisibleTable;

#ifndef USE_OPENGL
static SDL_Surface *OnlyFogSurface;
static CGraphic *AlphaFogG;
#endif

/*----------------------------------------------------------------------------
--  Functions
----------------------------------------------------------------------------*/

/**
**  Find the Number of Units that can see this square using a long
**  lookup. So when a 225 Viewed square can be calculated properly.
**
**  @param player  Player to mark sight.
**  @param tx      X center position.
**  @param ty      Y center position.
**
**  @return        Number of units that can see this square.
*/
static int LookupSight(const CPlayer *player, int tx, int ty)
{
	int i;
	int visiblecount;
	int range;
	int mapdistance;
	CUnit *unit;

	visiblecount = 0;
	for (i = 0; i < player->TotalNumUnits; ++i) {
		unit = player->Units[i];
		range = unit->CurrentSightRange;
		mapdistance = MapDistanceToUnit(tx, ty, unit);
		if (mapdistance <= range) {
			++visiblecount;
		}
		if ((tx >= unit->X && tx < unit->X + unit->Type->TileWidth &&
				mapdistance == range + 1) || (ty >= unit->Y &&
				ty < unit->Y + unit->Type->TileHeight &&
				mapdistance == range + 1)) {
			--visiblecount;
		}
		if (visiblecount >= 255) {
			return 255;
		}
	}
	return visiblecount;
}

/**
**  Find out if a field is seen (By player, or by shared vision)
**  This function will return > 1 with no fog of war.
**
**  @param player  Player to check for.
**  @param x       X tile to check.
**  @param y       Y tile to check.
**
**  @return        0 unexplored, 1 explored, > 1 visible.
*/
unsigned char IsTileVisible(const CPlayer *player, int x, int y)
{
	int i;
	unsigned char visiontype;
	unsigned char *visible;

	visible = TheMap.Fields[y * TheMap.Info.MapWidth + x].Visible;
	visiontype = visible[player->Index];

	if (visiontype > 1) {
		return visiontype;
	}
	if (!player->SharedVision) {
		if (visiontype) {
			return visiontype + (TheMap.NoFogOfWar ? 1 : 0);
		}
		return 0;
	}

	for (i = 0; i < PlayerMax ; ++i) {
		if (player->SharedVision & (1 << i) &&
				(Players[i].SharedVision & (1 << player->Index))) {
			if (visible[i] > 1) {
				return 2;
			}
			visiontype |= visible[i];
		}
	}
	if (visiontype) {
		return visiontype + (TheMap.NoFogOfWar ? 1 : 0);
	}
	return 0;
}

/**
**  Find out what the tile flags are a tile is covered by fog
**
**  @param player  player who is doing operation
**  @param x       X map location
**  @param y       Y map location
**  @param mask    input mask to filter
**
**  @return        Filtered mask after taking fog into account
*/
int MapFogFilterFlags(CPlayer *player, int x, int y, int mask)
{
	int nunits;
	int unitcount;
	int fogmask;
	CUnit *table[UnitMax];

	// Calculate Mask for tile with fog
	if (x < 0 || y < 0 || x >= TheMap.Info.MapWidth || y >= TheMap.Info.MapHeight) {
		return mask;
	}

	nunits = UnitCacheOnTile(x, y, table);
	fogmask = -1;
	unitcount = 0;
	while (unitcount < nunits) {
		if (!table[unitcount]->IsVisibleAsGoal(player)) {
			fogmask &= ~table[unitcount]->Type->FieldFlags;
		}
		++unitcount;
	}
	return mask & fogmask;
}

/**
**  Mark a tile's sight. (Explore and make visible.)
**
**  @param player  Player to mark sight.
**  @param x       X tile to mark.
**  @param y       Y tile to mark.
*/
void MapMarkTileSight(const CPlayer *player, int x, int y)
{
	unsigned char v;

	Assert(0 <= x && x < TheMap.Info.MapWidth);
	Assert(0 <= y && y < TheMap.Info.MapHeight);
	v = TheMap.Fields[x + y * TheMap.Info.MapWidth].Visible[player->Index];
	switch (v) {
		case 0:  // Unexplored
		case 1:  // Unseen
			// When there is NoFogOfWar only unexplored tiles are marked.
			if (!TheMap.NoFogOfWar || v == 0) {
				UnitsOnTileMarkSeen(player, x, y, 0);
			}
			v = 2;
			TheMap.Fields[x + y * TheMap.Info.MapWidth].Visible[player->Index] = v;
			if (IsTileVisible(ThisPlayer, x, y) > 1) {
				MapMarkSeenTile(x, y);
			}
			return;
		case 255:  // Overflow
			DebugPrint("Visible overflow (Player): %d\n" _C_ player->Index);
			break;
		default:  // seen -> seen
			++v;
			break;
	}
	TheMap.Fields[x + y * TheMap.Info.MapWidth].Visible[player->Index] = v;
}

/**
**  Unmark a tile's sight. (Explore and make visible.)
**
**  @param player  Player to mark sight.
**  @param x       X tile to mark.
**  @param y       Y tile to mark.
*/
void MapUnmarkTileSight(const CPlayer *player, int x, int y)
{
	unsigned char v;

	Assert(0 <= x && x < TheMap.Info.MapWidth);
	Assert(0 <= y && y < TheMap.Info.MapHeight);
	v = TheMap.Fields[x + y * TheMap.Info.MapWidth].Visible[player->Index];
	switch (v) {
		case 255:
			// FIXME: (mr-russ) Lookupsight is broken :(
			Assert(0);
			v = LookupSight(player, x, y);
			Assert(v >= 254);
			break;
		case 0:  // Unexplored
		case 1:
			// We are at minimum, don't do anything shouldn't happen.
			Assert(0);
			break;
		case 2:
			// When there is NoFogOfWar units never get unmarked.
			if (!TheMap.NoFogOfWar) {
				UnitsOnTileUnmarkSeen(player, x, y, 0);
			}
			// Check visible Tile, then deduct...
			if (IsTileVisible(ThisPlayer, x, y) > 1) {
				MapMarkSeenTile(x, y);
			}
		default:  // seen -> seen
			v--;
			break;
	}
	TheMap.Fields[x + y * TheMap.Info.MapWidth].Visible[player->Index] = v;
}

/**
** Mark a tile for cloak detection.
**
** @param player  Player to mark sight.
** @param x       X tile to mark.
** @param y       Y tile to mark.
*/
void MapMarkTileDetectCloak(const CPlayer *player, int x, int y)
{
	unsigned char v;

	v = TheMap.Fields[x + y * TheMap.Info.MapWidth].VisCloak[player->Index];
	if (v == 0) {
		UnitsOnTileMarkSeen(player, x, y, 1);
	}
	Assert(v != 255);
	++v;
	TheMap.Fields[x + y * TheMap.Info.MapWidth].VisCloak[player->Index] = v;
}

/**
** Unmark a tile for cloak detection.
**
** @param player  Player to mark sight.
** @param x       X tile to mark.
** @param y       Y tile to mark.
*/
void MapUnmarkTileDetectCloak(const CPlayer *player, int x, int y)
{
	unsigned char v;

	v = TheMap.Fields[x + y * TheMap.Info.MapWidth].VisCloak[player->Index];
	Assert(v != 0);
	if (v == 1) {
		UnitsOnTileUnmarkSeen(player, x, y, 1);
	}
	--v;
	TheMap.Fields[x + y * TheMap.Info.MapWidth].VisCloak[player->Index] = v;
}

/**
**  Mark the sight of unit. (Explore and make visible.)
**
**  @param player  player to mark the sight for (not unit owner)
**  @param x       x location to mark
**  @param y       y location to mark
**  @param w       width to mark, in square
**  @param h       height to mark, in square
**  @param range   Radius to mark.
**  @param marker  Function to mark or unmark sight
*/
void MapSight(const CPlayer *player, int x, int y, int w, int h, int range,
	void (*marker)(const CPlayer *, int, int))
{
	int mx;
	int my;
	int cx[4];
	int cy[4];
	int steps;
	int cycle;
	int p;

	// Mark as seen
	if (!range) {
		DebugPrint("Zero sight range\n");
		return;
	}

	p = player->Index;

	// Mark Horizontal sight for unit
	for (mx = x - range; mx < x + range + w; ++mx) {
		for (my = y; my < y + h; ++my) {
			if (mx >= 0 && mx < TheMap.Info.MapWidth) {
				marker(player, mx, my);
			}
		}
	}

	// Mark vertical sight for unit (don't remark self) (above unit)
	for (my = y - range; my < y; ++my) {
		for (mx = x; mx < x + w; ++mx) {
			if (my >= 0 && my < TheMap.Info.MapWidth) {
				marker(player, mx, my);
			}
		}
	}

	// Mark vertical sight for unit (don't remark self) (below unit)
	for (my = y + h; my < y + range + h; ++my) {
		for (mx = x; mx < x + w; ++mx) {
			if (my >= 0 && my < TheMap.Info.MapWidth) {
				marker(player, mx, my);
			}
		}
	}

	// Now the cross has been marked, need to use loop to mark in circle
	steps = 0;
	while (VisionTable[0][steps] <= range) {
		// 0 - Top right Quadrant
		cx[0] = x + w - 1;
		cy[0] = y - VisionTable[0][steps];
		// 1 - Top left Quadrant
		cx[1] = x;
		cy[1] = y - VisionTable[0][steps];
		// 2 - Bottom Left Quadrant
		cx[2] = x;
		cy[2] = y + VisionTable[0][steps] + h - 1;
		// 3 - Bottom Right Quadrant
		cx[3] = x + w - 1;
		cy[3] = y + VisionTable[0][steps] + h - 1;
		// loop for steps
		++steps;  // Increment past info pointer
		while (VisionTable[1][steps] != 0 || VisionTable[2][steps] != 0) {
			// Loop through for repeat cycle
			cycle = 0;
			while (cycle++ < VisionTable[0][steps]) {
				cx[0] += VisionTable[1][steps];
				cy[0] += VisionTable[2][steps];
				cx[1] -= VisionTable[1][steps];
				cy[1] += VisionTable[2][steps];
				cx[2] -= VisionTable[1][steps];
				cy[2] -= VisionTable[2][steps];
				cx[3] += VisionTable[1][steps];
				cy[3] -= VisionTable[2][steps];
				if (cx[0] < TheMap.Info.MapWidth && cy[0] >= 0) {
					marker(player, cx[0], cy[0]);
				}
				if (cx[1] >= 0 && cy[1] >= 0) {
					marker(player, cx[1], cy[1]);
				}
				if (cx[2] >= 0 && cy[2] < TheMap.Info.MapHeight) {
					marker(player, cx[2], cy[2]);
				}
				if (cx[3] < TheMap.Info.MapWidth && cy[3] < TheMap.Info.MapHeight) {
					marker(player, cx[3], cy[3]);
				}
			}
			++steps;
		}
	}
}

/**
**  Update fog of war.
*/
void UpdateFogOfWarChange(void)
{
	int x;
	int y;
	int w;

	DebugPrint("\n");
	//
	//  Mark all explored fields as visible again.
	//
	if (TheMap.NoFogOfWar) {
		w = TheMap.Info.MapWidth;
		for (y = 0; y < TheMap.Info.MapHeight; ++y) {
			for (x = 0; x < TheMap.Info.MapWidth; ++x) {
				if (IsMapFieldExplored(ThisPlayer, x, y)) {
					MapMarkSeenTile(x, y);
				}
			}
		}
	}
	//
	//  Global seen recount.
	//
	for (x = 0; x < NumUnits; ++x) {
		UnitCountSeen(Units[x]);
	}
}

/*----------------------------------------------------------------------------
--  Draw fog solid
----------------------------------------------------------------------------*/

/**
**  Draw only fog of war
**
**  @param x  X position into video memory
**  @param y  Y position into video memory
*/
#ifndef USE_OPENGL
void VideoDrawOnlyFog(int x, int y)
{
	int oldx;
	int oldy;
	SDL_Rect srect;
	SDL_Rect drect;

	srect.x = 0;
	srect.y = 0;
	srect.w = OnlyFogSurface->w;
	srect.h = OnlyFogSurface->h;

	oldx = x;
	oldy = y;
	CLIP_RECTANGLE(x, y, srect.w, srect.h);
	srect.x += x - oldx;
	srect.y += y - oldy;

	drect.x = x;
	drect.y = y;

	SDL_BlitSurface(OnlyFogSurface, &srect, TheScreen, &drect);
}
#else
void VideoDrawOnlyFog(int x, int y)
{
	Video.FillRectangleClip(Video.MapRGBA(0, 0, 0, 0, FogOfWarOpacity),
		x, y, TileSizeX, TileSizeY);
}
#endif

/*----------------------------------------------------------------------------
--  Old version correct working but not 100% original
----------------------------------------------------------------------------*/

/**
**  Draw fog of war tile.
**
**  @param sx  Offset into fields to current tile.
**  @param sy  Start of the current row.
**  @param dx  X position into video memory.
**  @param dy  Y position into video memory.
*/
static void DrawFogOfWarTile(int sx, int sy, int dx, int dy)
{
	int w;
	int tile;
	int tile2;
	int x;
	int y;

#define IsMapFieldExploredTable(x, y) \
	(VisibleTable[(y) * TheMap.Info.MapWidth + (x)])
#define IsMapFieldVisibleTable(x, y) \
	(VisibleTable[(y) * TheMap.Info.MapWidth + (x)] > 1)

	w = TheMap.Info.MapWidth;
	tile = tile2 = 0;
	x = sx - sy;
	y = sy / TheMap.Info.MapWidth;

	//
	//  Which Tile to draw for fog
	//
	// Investigate tiles around current tile
	// 1 2 3
	// 4 * 5
	// 6 7 8

	if (sy) {
		if (sx != sy) {
			if (!IsMapFieldExploredTable(x - 1, y - 1)) {
				tile2 |= 2;
				tile |= 2;
			} else if (!IsMapFieldVisibleTable(x - 1, y - 1)) {
				tile |= 2;
			}
		}
		if (!IsMapFieldExploredTable(x, y - 1)) {
			tile2 |= 3;
			tile |= 3;
		} else if (!IsMapFieldVisibleTable(x, y - 1)) {
			tile |= 3;
		}
		if (sx != sy + w - 1) {
			if (!IsMapFieldExploredTable(x + 1, y - 1)) {
				tile2 |= 1;
				tile |= 1;
			} else if (!IsMapFieldVisibleTable(x + 1, y - 1)) {
				tile |= 1;
			}
		}
	}

	if (sx != sy) {
		if (!IsMapFieldExploredTable(x - 1, y)) {
			tile2 |= 10;
			tile |= 10;
		} else if (!IsMapFieldVisibleTable(x - 1, y)) {
			tile |= 10;
		}
	}
	if (sx != sy + w - 1) {
		if (!IsMapFieldExploredTable(x + 1, y)) {
			tile2 |= 5;
			tile |= 5;
		} else if (!IsMapFieldVisibleTable(x + 1, y)) {
			tile |= 5;
		}
	}

	if (sy + w < TheMap.Info.MapHeight * w) {
		if (sx != sy) {
			if (!IsMapFieldExploredTable(x - 1, y + 1)) {
				tile2 |= 8;
				tile |= 8;
			} else if (!IsMapFieldVisibleTable(x - 1, y + 1)) {
				tile |= 8;
			}
		}
		if (!IsMapFieldExploredTable(x, y + 1)) {
			tile2 |= 12;
			tile |= 12;
		} else if (!IsMapFieldVisibleTable(x, y + 1)) {
			tile |= 12;
		}
		if (sx != sy + w - 1) {
			if (!IsMapFieldExploredTable(x + 1, y + 1)) {
				tile2 |= 4;
				tile |= 4;
			} else if (!IsMapFieldVisibleTable(x + 1, y + 1)) {
				tile |= 4;
			}
		}
	}

	tile = FogTable[tile];
	tile2 = FogTable[tile2];

	if (ReplayRevealMap) {
		tile2 = 0;
		tile = 0;
	}

	if (IsMapFieldVisibleTable(x, y) || ReplayRevealMap) {
		if (tile && tile != tile2) {
#ifdef USE_OPENGL
			TheMap.TileGraphic->DrawFrameClipTrans(tile, dx, dy, FogOfWarOpacity);
#else
			AlphaFogG->DrawFrameClip(tile, dx, dy);
#endif
		}
	} else {
		VideoDrawOnlyFog(dx, dy);
	}
	if (tile2) {
		TheMap.FogGraphic->DrawFrameClip(tile2, dx, dy);
	}

#undef IsMapFieldExploredTable
#undef IsMapFieldVisibleTable
}

/**
**  Draw the map fog of war.
*/
void CViewport::DrawMapFogOfWar() const
{
	int sx;
	int sy;
	int dx;
	int ex;
	int dy;
	int ey;
	int p;
	int my;
	int mx;

	// flags must redraw or not
	if (ReplayRevealMap) {
		return;
	}
	p = ThisPlayer->Index;

	sx = this->MapX - 1;
	if (sx < 0) {
		sx = 0;
	}
	ex = this->MapX + this->MapWidth + 1;
	if (ex > TheMap.Info.MapWidth) {
		ex = TheMap.Info.MapWidth;
	}
	my = this->MapY - 1;
	if (my < 0) {
		my = 0;
	}
	ey = this->MapY + this->MapHeight + 1;
	if (ey > TheMap.Info.MapHeight) {
		ey = TheMap.Info.MapHeight;
	}
	// Update for visibility all tile in viewport
	// and 1 tile around viewport (for fog-of-war connection display)
	for (; my < ey; ++my) {
		for (mx = sx; mx < ex; ++mx) {
			VisibleTable[my * TheMap.Info.MapWidth + mx] = IsTileVisible(ThisPlayer, mx, my);
		}
	}
	ex = this->EndX;
	sy = this->MapY * TheMap.Info.MapWidth;
	dy = this->Y - this->OffsetY;
	ey = this->EndY;

	while (dy <= ey) {
		sx = this->MapX + sy;
		dx = this->X - this->OffsetX;
		while (dx <= ex) {
			mx = (dx - this->X + this->OffsetX) / TileSizeX + this->MapX;
			my = (dy - this->Y + this->OffsetY) / TileSizeY + this->MapY;
			if (VisibleTable[my * TheMap.Info.MapWidth + mx]) {
				DrawFogOfWarTile(sx, sy, dx, dy);
			} else {
				Video.FillRectangleClip(ColorBlack, dx, dy, TileSizeX, TileSizeY);
			}
			++sx;
			dx += TileSizeX;
		}
		sy += TheMap.Info.MapWidth;
		dy += TileSizeY;
	}
}

/**
**  Initialize the fog of war.
**  Build tables, setup functions.
*/
void InitMapFogOfWar(void)
{
#ifndef USE_OPENGL
	unsigned char r;
	unsigned char g;
	unsigned char b;
	Uint32 color;
	SDL_Surface *s;
#endif

	TheMap.FogGraphic->Load();

#ifndef USE_OPENGL
	//
	// Generate Only Fog surface.
	//
	s = SDL_CreateRGBSurface(SDL_SWSURFACE, TileSizeX, TileSizeY,
		32, RMASK, GMASK, BMASK, AMASK);

	// FIXME: Make the color configurable
	SDL_GetRGB(ColorBlack, TheScreen->format, &r, &g, &b);
	color = Video.MapRGB(s->format, r, g, b);

	SDL_FillRect(s, NULL, color);
	OnlyFogSurface = SDL_DisplayFormat(s);
	SDL_SetAlpha(OnlyFogSurface, SDL_SRCALPHA | SDL_RLEACCEL, FogOfWarOpacity);
	SDL_FreeSurface(s);

	//
	// Generate Alpha Fog surface.
	//
	if (TheMap.FogGraphic->Surface->format->BytesPerPixel == 1) {
		s = SDL_DisplayFormat(TheMap.FogGraphic->Surface);
		SDL_SetAlpha(s, SDL_SRCALPHA | SDL_RLEACCEL, FogOfWarOpacity);
	} else {
		int i;
		int j;
		Uint32 c;
		unsigned char a;
		SDL_PixelFormat *f;

		// Copy the top row to a new surface
		f = TheMap.FogGraphic->Surface->format;
		s = SDL_CreateRGBSurface(SDL_SWSURFACE, TheMap.FogGraphic->Surface->w, TileSizeY,
			f->BitsPerPixel, f->Rmask, f->Gmask, f->Bmask, f->Amask);
		SDL_LockSurface(s);
		SDL_LockSurface(TheMap.FogGraphic->Surface);
		for (i = 0; i < s->h; ++i) {
			memcpy((Uint8 *)s->pixels + i * s->pitch,
				(Uint8 *)TheMap.FogGraphic->Surface->pixels + i * TheMap.FogGraphic->Surface->pitch,
				TheMap.FogGraphic->Surface->w * f->BytesPerPixel);
		}
		SDL_UnlockSurface(s);
		SDL_UnlockSurface(TheMap.FogGraphic->Surface);

		// Convert any non-transparent pixels to use FogOfWarOpacity as alpha
		SDL_LockSurface(s);
		for (j = 0; j < s->h; ++j) {
			for (i = 0; i < s->w; ++i) {
				c = *(Uint32 *)&((Uint8*)s->pixels)[i * 4 + j * s->pitch];
				Video.GetRGBA(c, s->format, &r, &g, &b, &a);
				if (a) {
					c = Video.MapRGBA(s->format, r, g, b, FogOfWarOpacity);
					*(Uint32 *)&((Uint8*)s->pixels)[i * 4 + j * s->pitch] = c;
				}

			}
		}
		SDL_UnlockSurface(s);
	}
	AlphaFogG = CGraphic::New(NULL);
	AlphaFogG->Surface = s;
	AlphaFogG->Width = TileSizeX;
	AlphaFogG->Height = TileSizeY;
	AlphaFogG->GraphicWidth = s->w;
	AlphaFogG->GraphicHeight = s->h;
	AlphaFogG->NumFrames = 1;
#endif

	VisibleTable = new unsigned char[TheMap.Info.MapWidth * TheMap.Info.MapHeight];
}

/**
**  Cleanup the fog of war.
*/
void CleanMapFogOfWar(void)
{
	delete[] VisibleTable;
	VisibleTable = NULL;

	CGraphic::Free(TheMap.FogGraphic);
	TheMap.FogGraphic = NULL;

#ifndef USE_OPENGL
	if (OnlyFogSurface) {
		SDL_FreeSurface(OnlyFogSurface);
		OnlyFogSurface = NULL;
	}
	CGraphic::Free(AlphaFogG);
	AlphaFogG = NULL;
#endif
}

/**
**  Initialize Vision and Goal Tables.
*/
void InitVisionTable(void)
{
	int *visionlist;
	int maxsize;
	int sizex;
	int sizey;
	int maxsearchsize;
	int i;
	int VisionTablePosition;
	int marker;
	int direction;
	int right;
	int up;
	int repeat;

	// Initialize Visiontable to large size, can't be more entries than tiles.
	VisionTable[0] = new unsigned char[MaxMapWidth * MaxMapWidth];
	VisionTable[1] = new unsigned char[MaxMapWidth * MaxMapWidth];
	VisionTable[2] = new unsigned char[MaxMapWidth * MaxMapWidth];

	VisionLookup = new int[MaxMapWidth + 2];
#ifndef SQUAREVISION
	visionlist = new int[MaxMapWidth * MaxMapWidth];
	//*2 as diagonal distance is longer

	maxsize = MaxMapWidth;
	maxsearchsize = MaxMapWidth;
	// Fill in table of map size
	for (sizex = 0; sizex < maxsize; ++sizex) {
		for (sizey = 0; sizey < maxsize; ++sizey) {
			visionlist[sizey * maxsize + sizex] = isqrt(sizex * sizex + sizey * sizey);
		}
	}

	VisionLookup[0] = 0;
	i = 1;
	VisionTablePosition = 0;
	while (i < maxsearchsize) {
		// Set Lookup Table
		VisionLookup[i] = VisionTablePosition;
		// Put in Null Marker
		VisionTable[0][VisionTablePosition] = i;
		VisionTable[1][VisionTablePosition] = 0;
		VisionTable[2][VisionTablePosition] = 0;
		++VisionTablePosition;


		// find i in left column
		marker = maxsize * i;
		direction = 0;
		right = 0;
		up = 0;

		// If not on top row, continue
		do {
			repeat = 0;
			do {
				// search for repeating
				// Test Right
				if ((repeat == 0 || direction == 1) && visionlist[marker + 1] == i) {
					right = 1;
					up = 0;
					++repeat;
					direction = 1;
					++marker;
				} else if ((repeat == 0 || direction == 2) && visionlist[marker - maxsize] == i) {
					up = 1;
					right = 0;
					++repeat;
					direction = 2;
					marker = marker - maxsize;
				} else if ((repeat == 0 || direction == 3) && visionlist[marker + 1 - maxsize] == i &&
						visionlist[marker - maxsize] != i && visionlist[marker + 1] != i) {
					up = 1;
					right = 1;
					++repeat;
					direction = 3;
					marker = marker + 1 - maxsize;
				} else {
					direction = 0;
					break;
				}

			   // search rigth
			   // search up - store as down.
			   // search diagonal
			}  while (direction && marker > (maxsize * 2));
			if (right || up) {
				VisionTable[0][VisionTablePosition] = repeat;
				VisionTable[1][VisionTablePosition] = right;
				VisionTable[2][VisionTablePosition] = up;
				++VisionTablePosition;
			}
		} while (marker > (maxsize * 2));
		++i;
	}

	delete[] visionlist;
#else
	// Find maximum distance in corner of map.
	maxsize = MaxMapWidth;
	maxsearchsize = isqrt(MaxMapWidth / 2);
	// Mark 1, It's a special case
	// Only Horizontal is marked
	VisionTable[0][0] = 1;
	VisionTable[1][0] = 0;
	VisionTable[2][0] = 0;
	VisionTable[0][1] = 1;
	VisionTable[1][1] = 1;
	VisionTable[2][1] = 0;

	// Mark VisionLookup
	VisionLookup[0] = 0;
	VisionLookup[1] = 0;
	i = 2;
	VisionTablePosition = 1;
	while (i <= maxsearchsize) {
		++VisionTablePosition;
		VisionLookup[i] = VisionTablePosition;
		// Setup Vision Start
		VisionTable[0][VisionTablePosition] = i;
		VisionTable[1][VisionTablePosition] = 0;
		VisionTable[2][VisionTablePosition] = 0;
		// Do Horizontal
		++VisionTablePosition;
		VisionTable[0][VisionTablePosition] = i;
		VisionTable[1][VisionTablePosition] = 1;
		VisionTable[2][VisionTablePosition] = 0;
		// Do Vertical
		++VisionTablePosition;
		VisionTable[0][VisionTablePosition] = i - 1;
		VisionTable[1][VisionTablePosition] = 0;
		VisionTable[2][VisionTablePosition] = 1;
		i++;
	}
#endif
}

/**
**  Clean Up Generated Vision and Goal Tables.
*/
void FreeVisionTable(void)
{
	// Free Vision Data
	delete[] VisionTable[0];
	VisionTable[0] = NULL;
	delete[] VisionTable[1];
	VisionTable[1] = NULL;
	delete[] VisionTable[2];
	VisionTable[2] = NULL;
	delete[] VisionLookup;
	VisionLookup = NULL;
}
//@}
