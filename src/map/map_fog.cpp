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
//      (c) Copyright 1999-2011 by Lutz Sammer, Vladi Shabanski,
//                                 Russell Smith, Jimmy Salmon and
//                                 Pali Rohár
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
Uint32 FogOfWarColorSDL;
int FogOfWarColor[3];

CGraphic *CMap::FogGraphic;

/**
**  Mapping for fog of war tiles.
*/
static const int FogTable[16] = {
	 0,11,10, 2,  13, 6, 14, 3,  12, 15, 4, 1,  8, 9, 7, 0,
};

static unsigned short *VisibleTable;

static SDL_Surface *OnlyFogSurface;
static CGraphic *AlphaFogG;

/*----------------------------------------------------------------------------
--  Functions
----------------------------------------------------------------------------*/


class _filter_flags
{
public:
	_filter_flags(const CPlayer &p, int *fogmask) : player(&p), fogmask(fogmask)
	{
		Assert(fogmask != NULL);
	}

	void operator() (const CUnit *const unit) const {
		if (!unit->IsVisibleAsGoal(*player)) {
			*fogmask &= ~unit->Type->FieldFlags;
		}
	}
private:
	const CPlayer *player;
	int *fogmask;
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
int MapFogFilterFlags(CPlayer &player, const unsigned int index, int mask)
{
	int fogMask = mask;

	_filter_flags filter(player, &fogMask);
	Map.Field(index)->UnitCache.for_each(filter);
	return fogMask;
}

int MapFogFilterFlags(CPlayer &player, const Vec2i &pos, int mask)
{
	if (Map.Info.IsPointOnMap(pos))
	{
		return MapFogFilterFlags(player, Map.getIndex(pos), mask);
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
void MapMarkTileSight(const CPlayer &player, const unsigned int index)
{
	//v = &Map.Field(x, y)->Visible[player.Index];
	unsigned short *v = &(Map.Field(index)->Visible[player.Index]);
	if (*v == 0 || *v == 1) { // Unexplored or unseen
		// When there is no fog only unexplored tiles are marked.
		if (!Map.NoFogOfWar || *v == 0) {
			UnitsOnTileMarkSeen(player, index, 0);
		}
		*v = 2;
		if (Map.IsTileVisible(*ThisPlayer, index) > 1) {
			Map.MarkSeenTile(index);
		}
		return;
	}
	Assert(*v != 65535);
	++*v;
}

void MapMarkTileSight(const CPlayer &player, const Vec2i &pos)
{
	Assert(Map.Info.IsPointOnMap(pos));
	MapMarkTileSight(player, Map.getIndex(pos));
}


/**
**  Unmark a tile's sight. (Explore and make visible.)
**
**  @param player  Player to mark sight.
**  @param x       X tile to mark.
**  @param y       Y tile to mark.
*/
void MapUnmarkTileSight(const CPlayer &player, const unsigned int index)
{
	unsigned short *v = &(Map.Field(index)->Visible[player.Index]);
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
			if (Map.IsTileVisible(*ThisPlayer, index) > 1) {
				Map.MarkSeenTile(index);
			}
		default:  // seen -> seen
			--*v;
			break;
	}
}

void MapUnmarkTileSight(const CPlayer &player, const Vec2i &pos)
{
	Assert(Map.Info.IsPointOnMap(pos));
	MapUnmarkTileSight(player, Map.getIndex(pos));
}


/**
**  Mark a tile for cloak detection.
**
**  @param player  Player to mark sight.
**  @param x       X tile to mark.
**  @param y       Y tile to mark.
*/
void MapMarkTileDetectCloak(const CPlayer &player, const unsigned int index)
{
	unsigned char *v = &(Map.Field(index)->VisCloak[player.Index]);
	if (*v == 0) {
		UnitsOnTileMarkSeen(player, index, 1);
	}
	Assert(*v != 255);
	++*v;
}

void MapMarkTileDetectCloak(const CPlayer &player, const Vec2i &pos)
{
	MapMarkTileDetectCloak(player, Map.getIndex(pos));
}


/**
**  Unmark a tile for cloak detection.
**
**  @param player  Player to mark sight.
**  @param x       X tile to mark.
**  @param y       Y tile to mark.
*/
void
MapUnmarkTileDetectCloak(const CPlayer &player, const unsigned int index)
{
	unsigned char *v = &(Map.Field(index)->VisCloak[player.Index]);
	Assert(*v != 0);
	if (*v == 1) {
		UnitsOnTileUnmarkSeen(player, index, 1);
	}
	--*v;
}

void MapUnmarkTileDetectCloak(const CPlayer &player, const Vec2i &pos)
{
	MapUnmarkTileDetectCloak(player, Map.getIndex(pos));
}

inline int square(int v)
{
	return v * v;
}

/**
**  Mark the sight of unit. (Explore and make visible.)
**
**  @param player  player to mark the sight for (not unit owner)
**  @param pos     location to mark
**  @param w       width to mark, in square
**  @param h       height to mark, in square
**  @param range   Radius to mark.
**  @param marker  Function to mark or unmark sight
*/
void MapSight(const CPlayer &player, const Vec2i &pos, int w, int h, int range, MapMarkerFunc *marker)
{
	// Units under construction have no sight range.
	if (!range) {
		return;
	}
// Up hemi-cyle
	const int miny = std::max(-range, 0 - pos.y);
	for (int offsety = miny; offsety != 0; ++offsety) {
		const int offsetx = isqrt(square(range + 1) - square(-offsety) - 1);
		const int minx = std::max(0, pos.x - offsetx);
		const int maxx = std::min(Map.Info.MapWidth, pos.x + w + offsetx);
		Vec2i mpos = {minx, pos.y + offsety};
		const unsigned int index = mpos.y * Map.Info.MapWidth;

		for (mpos.x = minx; mpos.x < maxx; ++mpos.x) {
#ifdef MARKER_ON_INDEX
				marker(player, mpos.x + index);
#else
				marker(player, mpos);
#endif
		}
	}
	for (int offsety = 0; offsety < h; ++offsety) {
		const int minx = std::max(0, pos.x - range);
		const int maxx = std::min(Map.Info.MapWidth, pos.x + w + range);
		Vec2i mpos = {minx, pos.y + offsety};
		const unsigned int index = mpos.y * Map.Info.MapWidth;

		for (mpos.x = minx; mpos.x < maxx; ++mpos.x) {
#ifdef MARKER_ON_INDEX
				marker(player, mpos.x + index);
#else
				marker(player, mpos);
#endif
		}
	}
// bottom hemi-cycle
	const int maxy = std::min(range, Map.Info.MapHeight - pos.y - h);
	for (int offsety = 0; offsety < maxy; ++offsety) {
		const int offsetx = isqrt(square(range + 1) - square(offsety) - 1);
		const int minx = std::max(0, pos.x - offsetx);
		const int maxx = std::min(Map.Info.MapWidth, pos.x + w + offsetx);
		Vec2i mpos = {minx, pos.y + h + offsety};
		const unsigned int index = mpos.y * Map.Info.MapWidth;

		for (mpos.x = minx; mpos.x < maxx; ++mpos.x) {
#ifdef MARKER_ON_INDEX
				marker(player, mpos.x + index);
#else
				marker(player, mpos);
#endif
		}
	}
}

/**
**  Update fog of war.
*/
void UpdateFogOfWarChange()
{

	DebugPrint("::UpdateFogOfWarChange\n");
	//
	//  Mark all explored fields as visible again.
	//
	if (Map.NoFogOfWar) {
		unsigned int index = 0;
		int w = Map.Info.MapHeight * Map.Info.MapWidth;
		do {
			if (Map.IsFieldExplored(*ThisPlayer, index)) {
					Map.MarkSeenTile(index);
			}
			index++;
		} while(--w);
	}
	//
	//  Global seen recount.
	//
	for (int x = 0; x < NumUnits; ++x) {
		UnitCountSeen(*Units[x]);
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
void VideoDrawOnlyFog(int x, int y)
{
	if (!UseOpenGL) {
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
	} else {
		Video.FillRectangleClip(Video.MapRGBA(0, 0, 0, 0, FogOfWarOpacity),
			x, y, PixelTileSize.x, PixelTileSize.y);
	}
}

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
			if (UseOpenGL) {
				Map.FogGraphic->DrawFrameClipTrans(tile, dx, dy, FogOfWarOpacity);
			} else {
				AlphaFogG->DrawFrameClip(tile, dx, dy);
			}
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
	int my;
	int mx;

	// flags must redraw or not
	if (ReplayRevealMap) {
		return;
	}

    sx = std::max<int>(MapX - 1, 0);
    ex = std::min<int>(MapX + MapWidth + 1, Map.Info.MapWidth);
    my = std::max<int>(MapY - 1, 0);
    ey = std::min<int>(MapY + MapHeight + 1, Map.Info.MapHeight);

    // Update for visibility all tile in viewport
	// and 1 tile around viewport (for fog-of-war connection display)

	unsigned int my_index = my * Map.Info.MapWidth;
	for (; my < ey; ++my) {
		for (mx = sx; mx < ex; ++mx) {
			VisibleTable[my_index + mx] = Map.IsTileVisible(*ThisPlayer, mx + my_index);
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
				Video.FillRectangleClip(FogOfWarColorSDL, dx, dy, PixelTileSize.x, PixelTileSize.y);
			}
			++sx;
			dx += PixelTileSize.x;
		}
		sy += Map.Info.MapWidth;
		dy += PixelTileSize.y;
	}
}

/**
**  Initialize the fog of war.
**  Build tables, setup functions.
*/
void CMap::InitFogOfWar()
{
	//calculate this once from the settings and store it
	FogOfWarColorSDL = Video.MapRGB(TheScreen->format, FogOfWarColor[0], FogOfWarColor[1], FogOfWarColor[2]);

	Uint8 r, g, b;
	Uint32 color;
	SDL_Surface *s;

	FogGraphic->Load();

	if (!UseOpenGL) {
		//
		// Generate Only Fog surface.
		//
		s = SDL_CreateRGBSurface(SDL_SWSURFACE, PixelTileSize.x, PixelTileSize.y,
			32, RMASK, GMASK, BMASK, AMASK);

		SDL_GetRGB(FogOfWarColorSDL, TheScreen->format, &r, &g, &b);
		color = Video.MapRGB(s->format, r, g, b);

		SDL_FillRect(s, NULL, color);
		OnlyFogSurface = SDL_DisplayFormat(s);
		SDL_SetAlpha(OnlyFogSurface, SDL_SRCALPHA | SDL_RLEACCEL, FogOfWarOpacity);
		VideoPaletteListRemove(s);
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
			s = SDL_CreateRGBSurface(SDL_SWSURFACE, FogGraphic->Surface->w, PixelTileSize.y,
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
		AlphaFogG->Width = PixelTileSize.x;
		AlphaFogG->Height = PixelTileSize.y;
		AlphaFogG->GraphicWidth = s->w;
		AlphaFogG->GraphicHeight = s->h;
		AlphaFogG->NumFrames = 16;//1;
		AlphaFogG->GenFramesMap();
		AlphaFogG->UseDisplayFormat();
	}

	delete[] VisibleTable;
	VisibleTable = new unsigned short[Info.MapWidth * Info.MapHeight];
}

/**
**  Cleanup the fog of war.
*/
void CMap::CleanFogOfWar()
{
	delete[] VisibleTable;
	VisibleTable = NULL;

	CGraphic::Free(Map.FogGraphic);
	FogGraphic = NULL;

	if (!UseOpenGL) {
		if (OnlyFogSurface) {
			VideoPaletteListRemove(OnlyFogSurface);
			SDL_FreeSurface(OnlyFogSurface);
			OnlyFogSurface = NULL;
		}
		CGraphic::Free(AlphaFogG);
		AlphaFogG = NULL;
	}
}

//@}
