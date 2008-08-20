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
//      (c) Copyright 1999-2006 by Lutz Sammer, Vladi Shabanski,
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
CGraphic *CMap::FogGraphic;

/**
**  Mapping for fog of war tiles.
*/
static const int FogTable[16] = {
	 0,11,10, 2,  13, 6, 14, 3,  12, 15, 4, 1,  8, 9, 7, 0,
};

unsigned char *VisionTable[3];
int *VisionLookup;

static unsigned short *VisibleTable;

#ifndef USE_OPENGL
static SDL_Surface *OnlyFogSurface;
static CGraphic *AlphaFogG;
#endif

/*----------------------------------------------------------------------------
--  Functions
----------------------------------------------------------------------------*/


struct _filter_flags {
	const CPlayer *player;
	int fogmask;
	_filter_flags(const CPlayer *p, int m) : player(p), fogmask(m) {}
	inline bool operator() (const CUnit *const unit) {
		if (!unit->IsVisibleAsGoal(player)) {
			fogmask &= ~unit->Type->FieldFlags;
		}
		return true;
	}
};

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
int MapFogFilterFlags(CPlayer *player, const unsigned int index, int mask)
{
	_filter_flags filter(player, -1);
	Map.Field(index)->UnitCache.for_each(filter);
	return mask & filter.fogmask;
	
}

int MapFogFilterFlags(CPlayer *player, int x, int y, int mask)
{
	if(Map.Info.IsPointOnMap(x,y))
	{
		return MapFogFilterFlags(player, Map.getIndex(x,y), mask);
	}
	return mask;
}

/**
**  Mark a tile's sight. (Explore and make visible.)
**
**  @param player  Player to mark sight.
**  @param x       X tile to mark.
**  @param y       Y tile to mark.
*/
void MapMarkTileSight(const CPlayer *player, 
	const unsigned int index)
{
	//v = &Map.Field(x, y)->Visible[player->Index];
	unsigned short *v = &(Map.Field(index)->Visible[player->Index]);
	if (*v == 0 || *v == 1) { // Unexplored or unseen
		// When there is no fog only unexplored tiles are marked.
		if (!Map.NoFogOfWar || *v == 0) {
			UnitsOnTileMarkSeen(player, index, 0);
		}
		*v = 2;
		if (Map.IsTileVisible(ThisPlayer, index) > 1) {
			Map.MarkSeenTile(index);
		}
		return;
	}
	Assert(*v != 65535);
	++*v;
}

void MapMarkTileSight(const CPlayer *player, int x, int y)
{
	Assert(0 <= x && x < Map.Info.MapWidth);
	Assert(0 <= y && y < Map.Info.MapHeight);
#ifdef MARKER_ON_INDEX	
	MapMarkTileSight(player, Map.getIndex(x,y));
#else
	const unsigned int index = Map.getIndex(x,y);
	unsigned short *v = &(Map.Field(index)->Visible[player->Index]);
	if (*v == 0 || *v == 1) { // Unexplored or unseen
		// When there is no fog only unexplored tiles are marked.
		if (!Map.NoFogOfWar || *v == 0) {
			UnitsOnTileMarkSeen(player, index, 0);
		}
		*v = 2;
		//if (Map.IsTileVisible(ThisPlayer, x, y) > 1) {
		if (Map.IsTileVisible(ThisPlayer, index) > 1) {
			Map.MarkSeenTile(x, y);
		}
		return;
	}
	Assert(*v != 65535);
	++*v;
#endif
}


/**
**  Unmark a tile's sight. (Explore and make visible.)
**
**  @param player  Player to mark sight.
**  @param x       X tile to mark.
**  @param y       Y tile to mark.
*/
void MapUnmarkTileSight(const CPlayer *player, const unsigned int index)
{
	unsigned short *v = &(Map.Field(index)->Visible[player->Index]);
	switch (*v) {
		case 0:  // Unexplored
		case 1:
			// We are at minimum, don't do anything shouldn't happen.
			Assert(0);
			break;
		case 2:
			// When there is NoFogOfWar units never get unmarked.
			if (!Map.NoFogOfWar) {
				UnitsOnTileUnmarkSeen(player, index, 0);
			}
			// Check visible Tile, then deduct...
			if (Map.IsTileVisible(ThisPlayer, index) > 1) {
				Map.MarkSeenTile(index);
			}
		default:  // seen -> seen
			--*v;
			break;
	}
}

void MapUnmarkTileSight(const CPlayer *player, int x, int y)
{
	Assert(0 <= x && x < Map.Info.MapWidth);
	Assert(0 <= y && y < Map.Info.MapHeight);
#ifdef MARKER_ON_INDEX	
	MapUnmarkTileSight(player, Map.getIndex(x,y));
#else
	const unsigned int index = Map.getIndex(x,y);
	unsigned short *v = &(Map.Field(index)->Visible[player->Index]);
	switch (*v) {
		case 0:  // Unexplored
		case 1:
			// We are at minimum, don't do anything shouldn't happen.
			Assert(0);
			break;
		case 2:
			// When there is NoFogOfWar units never get unmarked.
			if (!Map.NoFogOfWar) {
				//UnitsOnTileUnmarkSeen(player, x, y, 0);
				UnitsOnTileUnmarkSeen(player, index, 0);
			}
			// Check visible Tile, then deduct...
			//if (Map.IsTileVisible(ThisPlayer, x, y) > 1) {
			if (Map.IsTileVisible(ThisPlayer, index) > 1) {
				Map.MarkSeenTile(x, y);
			}
		default:  // seen -> seen
			--*v;
			break;
	}
#endif
}


/**
**  Mark a tile for cloak detection.
**
**  @param player  Player to mark sight.
**  @param x       X tile to mark.
**  @param y       Y tile to mark.
*/
void MapMarkTileDetectCloak(const CPlayer *player, const unsigned int index)
{
	unsigned char *v = &(Map.Field(index)->VisCloak[player->Index]);
	if (*v == 0) {
		UnitsOnTileMarkSeen(player, index, 1);
	}
	Assert(*v != 255);
	++*v;
}

void MapMarkTileDetectCloak(const CPlayer *player, int x, int y)
{
	MapMarkTileDetectCloak(player, Map.getIndex(x,y));
}


/**
**  Unmark a tile for cloak detection.
**
**  @param player  Player to mark sight.
**  @param x       X tile to mark.
**  @param y       Y tile to mark.
*/
void
MapUnmarkTileDetectCloak(const CPlayer *player, const unsigned int index)
{
	unsigned char *v = &(Map.Field(index)->VisCloak[player->Index]);
	Assert(*v != 0);
	if (*v == 1) {
		UnitsOnTileUnmarkSeen(player, index, 1);
	}
	--*v;
}

void MapUnmarkTileDetectCloak(const CPlayer *player, int x, int y)
{
	MapUnmarkTileDetectCloak(player, Map.getIndex(x,y));
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
	MapMarkerFunc *marker)
{
	int mx;
	int my;
	int cx[4];
	int cy[4];
	int steps;
	int cycle;
	int p;

	// Units under construction have no sight range.
	if (!range) {
		return;
	}

	p = player->Index;

#ifdef MARKER_ON_INDEX
	unsigned int index = y * Map.Info.MapWidth;
#endif
	for (my = y; my < y + h; ++my) {
		for (mx = x - range; mx < x + range + w; ++mx) {
			if (mx >= 0 && mx < Map.Info.MapWidth) {
#ifdef MARKER_ON_INDEX
				marker(player, mx + index);
#else			
				marker(player, mx, my);
#endif
			}
		}
#ifdef MARKER_ON_INDEX
	index += Map.Info.MapWidth;
#endif
	}

	// Mark vertical sight for unit (don't remark self) (above unit)
#ifdef MARKER_ON_INDEX
	index = (y - range) * Map.Info.MapWidth;
#endif
	for (my = y - range; my < y; ++my) {
		if (my >= 0 && my < Map.Info.MapHeight) {	
			for (mx = x; mx < x + w; ++mx) {
#ifdef MARKER_ON_INDEX
				marker(player, mx + index);
#else			
				marker(player, mx, my);
#endif
			}
		}
#ifdef MARKER_ON_INDEX
	index += Map.Info.MapWidth;
#endif		
	}

	// Mark vertical sight for unit (don't remark self) (below unit)
#ifdef MARKER_ON_INDEX
	index = (y + h) * Map.Info.MapWidth;
#endif
	for (my = y + h; my < y + range + h; ++my) {
		if (my >= 0 && my < Map.Info.MapHeight) {
			for (mx = x; mx < x + w; ++mx) {
#ifdef MARKER_ON_INDEX
				marker(player, mx + index);
#else			
				marker(player, mx, my);
#endif
			}
		}
#ifdef MARKER_ON_INDEX
	index += Map.Info.MapWidth;
#endif
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
				if (cx[0] < Map.Info.MapWidth && cy[0] >= 0) {
#ifdef MARKER_ON_INDEX
					marker(player, Map.getIndex(cx[0], cy[0]));
#else
					marker(player, cx[0], cy[0]);
#endif
				}
				if (cx[1] >= 0 && cy[1] >= 0) {
#ifdef MARKER_ON_INDEX
					marker(player, Map.getIndex(cx[1], cy[1]));
#else
					marker(player, cx[1], cy[1]);
#endif
				}
				if (cx[2] >= 0 && cy[2] < Map.Info.MapHeight) {
#ifdef MARKER_ON_INDEX
					marker(player, Map.getIndex(cx[2], cy[2]));
#else
					marker(player, cx[2], cy[2]);
#endif
				}
				if (cx[3] < Map.Info.MapWidth && cy[3] < Map.Info.MapHeight) {
#ifdef MARKER_ON_INDEX
					marker(player, Map.getIndex(cx[3], cy[3]));
#else
					marker(player, cx[3], cy[3]);
#endif
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

	DebugPrint("::UpdateFogOfWarChange\n");
	//
	//  Mark all explored fields as visible again.
	//
	if (Map.NoFogOfWar) {
		unsigned int index = 0;
		int w = Map.Info.MapHeight * Map.Info.MapWidth;
		do {
			if (Map.IsFieldExplored(ThisPlayer, index)) {
					Map.MarkSeenTile(index);
			}
			index++;
		} while(--w);
	}
	//
	//  Global seen recount.
	//
	for (int x = 0; x < NumUnits; ++x) {
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

#define IsMapFieldExploredTable(index) \
	(VisibleTable[(index)])
#define IsMapFieldVisibleTable(index) \
	(VisibleTable[(index)] > 1)


	int w = Map.Info.MapWidth;
	int tile = 0,tile2 = 0;
	int x = sx - sy;
	//int y = sy / Map.Info.MapWidth;

	//
	//  Which Tile to draw for fog
	//
	// Investigate tiles around current tile
	// 1 2 3
	// 4 * 5
	// 6 7 8

	if (sy) {
		unsigned int index = sy - Map.Info.MapWidth;//(y-1) * Map.Info.MapWidth;	
		if (sx != sy) {
			//if (!IsMapFieldExploredTable(x - 1, y - 1)) {
			if (!IsMapFieldExploredTable(x - 1 + index)) {
				tile2 |= 2;
				tile |= 2;
			//} else if (!IsMapFieldVisibleTable(x - 1, y - 1)) {
			} else if (!IsMapFieldVisibleTable(x - 1 + index)) {
				tile |= 2;
			}
		}
		//if (!IsMapFieldExploredTable(x, y - 1)) {
		if (!IsMapFieldExploredTable(x + index)) {
			tile2 |= 3;
			tile |= 3;
		//} else if (!IsMapFieldVisibleTable(x, y - 1)) {
		} else if (!IsMapFieldVisibleTable(x + index)) {
			tile |= 3;
		}
		if (sx != sy + w - 1) {
			//if (!IsMapFieldExploredTable(x + 1, y - 1)) {
			if (!IsMapFieldExploredTable(x + 1 + index)) {
				tile2 |= 1;
				tile |= 1;
			//} else if (!IsMapFieldVisibleTable(x + 1, y - 1)) {
			} else if (!IsMapFieldVisibleTable(x + 1 + index)) {
				tile |= 1;
			}
		}
	}

	if (sx != sy) {
		unsigned int index = sy;//(y) * Map.Info.MapWidth;	
		//if (!IsMapFieldExploredTable(x - 1, y)) {
		if (!IsMapFieldExploredTable(x - 1 + index)) {
			tile2 |= 10;
			tile |= 10;
		//} else if (!IsMapFieldVisibleTable(x - 1, y)) {
		} else if (!IsMapFieldVisibleTable(x - 1 + index)) {
			tile |= 10;
		}
	}
	if (sx != sy + w - 1) {
		unsigned int index = sy;//(y) * Map.Info.MapWidth;	
		//if (!IsMapFieldExploredTable(x + 1, y)) {
		if (!IsMapFieldExploredTable(x + 1 + index)) {
			tile2 |= 5;
			tile |= 5;
		//} else if (!IsMapFieldVisibleTable(x + 1, y)) {
		} else if (!IsMapFieldVisibleTable(x + 1 + index)) {
			tile |= 5;
		}
	}

	if (sy + w < Map.Info.MapHeight * w) {
		unsigned int index = sy + Map.Info.MapWidth;//(y+1) * Map.Info.MapWidth;	
		if (sx != sy) {
			//if (!IsMapFieldExploredTable(x - 1, y + 1)) {
			if (!IsMapFieldExploredTable(x - 1 + index)) {
				tile2 |= 8;
				tile |= 8;
			//} else if (!IsMapFieldVisibleTable(x - 1, y + 1)) {
			} else if (!IsMapFieldVisibleTable(x - 1 + index)) {
				tile |= 8;
			}
		}
		//if (!IsMapFieldExploredTable(x, y + 1)) {
		if (!IsMapFieldExploredTable(x + index)) {
			tile2 |= 12;
			tile |= 12;
		//} else if (!IsMapFieldVisibleTable(x, y + 1)) {
		} else if (!IsMapFieldVisibleTable(x + index)) {
			tile |= 12;
		}
		if (sx != sy + w - 1) {
			//if (!IsMapFieldExploredTable(x + 1, y + 1)) {
			if (!IsMapFieldExploredTable(x + 1 + index)) {
				tile2 |= 4;
				tile |= 4;
			//} else if (!IsMapFieldVisibleTable(x + 1, y + 1)) {
			} else if (!IsMapFieldVisibleTable(x + 1 + index)) {
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

	//if (IsMapFieldVisibleTable(x, y) || ReplayRevealMap) {
	if (IsMapFieldVisibleTable(sx) || ReplayRevealMap) {
		if (tile && tile != tile2) {
#ifdef USE_OPENGL
			Map.FogGraphic->DrawFrameClipTrans(tile, dx, dy, FogOfWarOpacity);
#else
			AlphaFogG->DrawFrameClip(tile, dx, dy);
#endif
		}
	} else {
		VideoDrawOnlyFog(dx, dy);
	}
	if (tile2) {
		Map.FogGraphic->DrawFrameClip(tile2, dx, dy);
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

    sx = std::max(MapX - 1, 0);
    ex = std::min(MapX + MapWidth + 1, Map.Info.MapWidth);
    my = std::max(MapY - 1, 0);
    ey = std::min(MapY + MapHeight + 1, Map.Info.MapHeight);

    // Update for visibility all tile in viewport
	// and 1 tile around viewport (for fog-of-war connection display)

	unsigned int my_index = my * Map.Info.MapWidth;
	for (; my < ey; ++my) {
		for (mx = sx; mx < ex; ++mx) {
			VisibleTable[my_index + mx] = Map.IsTileVisible(ThisPlayer, mx + my_index);
		}
		my_index += Map.Info.MapWidth;
	}
	ex = EndX;
	sy = MapY * Map.Info.MapWidth;
	dy = Y - OffsetY;
	ey = EndY;

	while (dy <= ey) {
		sx = MapX + sy;
		dx = X - OffsetX;
		while (dx <= ex) {
			if (VisibleTable[sx]) {
				DrawFogOfWarTile(sx, sy, dx, dy);
			} else {
				Video.FillRectangleClip(ColorBlack, dx, dy, TileSizeX, TileSizeY);
			}
			++sx;
			dx += TileSizeX;
		}
		sy += Map.Info.MapWidth;
		dy += TileSizeY;
	}
}

/**
**  Initialize the fog of war.
**  Build tables, setup functions.
*/
void CMap::InitFogOfWar(void)
{
#ifndef USE_OPENGL
	Uint8 r, g, b;
	Uint32 color;
	SDL_Surface *s;
#endif

	FogGraphic->Load();

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
	if (FogGraphic->Surface->format->BytesPerPixel == 1) {
		s = SDL_DisplayFormat(FogGraphic->Surface);
		SDL_SetAlpha(s, SDL_SRCALPHA | SDL_RLEACCEL, FogOfWarOpacity);
	} else {
		int i;
		int j;
		Uint32 c;
		Uint8 a;
		SDL_PixelFormat *f;

		// Copy the top row to a new surface
		f = FogGraphic->Surface->format;
		s = SDL_CreateRGBSurface(SDL_SWSURFACE, FogGraphic->Surface->w, TileSizeY,
			f->BitsPerPixel, f->Rmask, f->Gmask, f->Bmask, f->Amask);
		SDL_LockSurface(s);
		SDL_LockSurface(FogGraphic->Surface);
		for (i = 0; i < s->h; ++i) {
			memcpy((Uint8 *)s->pixels + i * s->pitch,
				(Uint8 *)FogGraphic->Surface->pixels + i * FogGraphic->Surface->pitch,
				FogGraphic->Surface->w * f->BytesPerPixel);
		}
		SDL_UnlockSurface(s);
		SDL_UnlockSurface(FogGraphic->Surface);

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
	AlphaFogG = CGraphic::New("");
	AlphaFogG->Surface = s;
	AlphaFogG->Width = TileSizeX;
	AlphaFogG->Height = TileSizeY;
	AlphaFogG->GraphicWidth = s->w;
	AlphaFogG->GraphicHeight = s->h;
	AlphaFogG->NumFrames = 16;//1;
	AlphaFogG->GenFramesMap();
	AlphaFogG->UseDisplayFormat();
#endif

	delete[] VisibleTable;
	VisibleTable = new unsigned short[Info.MapWidth * Info.MapHeight];
}

/**
**  Cleanup the fog of war.
*/
void CMap::CleanFogOfWar(void)
{
	delete[] VisibleTable;
	VisibleTable = NULL;

	CGraphic::Free(Map.FogGraphic);
	FogGraphic = NULL;

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

			   // search right
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
