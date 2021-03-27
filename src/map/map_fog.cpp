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
//      (c) Copyright 1999-2015 by Lutz Sammer, Vladi Shabanski,
//		Russell Smith, Jimmy Salmon, Pali Rohár and Andrettin
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
#include <queue>
#include <omp.h>

#include "stratagus.h"

#include "map.h"

#include "actions.h"
#include "fov.h"
#include "minimap.h"
#include "player.h"
#include "tileset.h"
#include "ui.h"
#include "unit.h"
#include "unit_manager.h"
#include "video.h"
#include "../video/intern_video.h"


/*----------------------------------------------------------------------------
--  Variables
----------------------------------------------------------------------------*/
CFieldOfView FieldOfView;

int FogOfWarOpacity;				/// Fog of war Opacity.
Uint32 FogOfWarColorSDL;
CColor FogOfWarColor;

CGraphic *CMap::FogGraphic;

/**
**  Mapping for fog of war tiles.
*/
static const int FogTable[16] = {
	0, 11, 10, 2,  13, 6, 14, 3,  12, 15, 4, 1,  8, 9, 7, 0,
};

static std::vector<unsigned short> VisibleTable;

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

	void operator()(const CUnit *const unit) const
	{
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
**  @param index   map location
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
	if (Map.Info.IsPointOnMap(pos)) {
		return MapFogFilterFlags(player, Map.getIndex(pos), mask);
	}
	return mask;
}

template<bool MARK>
class _TileSeen
{
public:
	_TileSeen(const CPlayer &p , int c) : player(&p), cloak(c)
	{}

	void operator()(CUnit *const unit) const
	{
		if (cloak != (int)unit->Type->BoolFlag[PERMANENTCLOAK_INDEX].value) {
			return ;
		}
		const uint8_t p = this->player->Index;
		if (MARK) {
			//  If the unit goes out of fog, this can happen for any player that
			//  this player shares vision with, and can't YET see the unit.
			//  It will be able to see the unit after the Unit->VisCount ++
			if (!unit->VisCount[p]) {
				UnitGoesOutOfFog(*unit, *this->player);
				for (const uint8_t pi : this->player->GetGaveVisionTo()) {
					if (!unit->IsVisible(Players[pi])) {
						UnitGoesOutOfFog(*unit, Players[pi]);
					} 
				}
			}
			unit->VisCount[p/*player->Index*/]++;
		} else {
			/*
			 * HACK: UGLY !!!
			 * There is bug in Seen code conneded with
			 * UnitActionDie and Cloaked units.
			 */
			if (!unit->VisCount[p] && unit->CurrentAction() == UnitActionDie) {
				return;
			}

			/// This could happen if shadow caster type of field of view is enabled, 
			/// because of multiple calls for tiles in vertical/horizontal/diagonal lines
			if(!unit->VisCount[p]) {
				return;
			}

			unit->VisCount[p]--;
			//  If the unit goes under of fog, this can happen for any player that
			//  this player shares vision to. First of all, before unmarking,
			//  every player that this player shares vision to can see the unit.
			//  Now we have to check who can't see the unit anymore.
			if (!unit->VisCount[p]) {
				UnitGoesUnderFog(*unit, *this->player);
				for (const uint8_t pi : this->player->GetGaveVisionTo()) {
					if (!unit->IsVisible(Players[pi])) {
						UnitGoesUnderFog(*unit, Players[pi]);
					} 
				}
			}
		}
	}
private:
	const CPlayer *player;
	int cloak;
};

/**
**  Mark all units on a tile as now visible.
**
**  @param player  The player this is for.
**  @param mf      field location to check
**  @param cloak   If we mark cloaked units too.
*/
static void UnitsOnTileMarkSeen(const CPlayer &player, CMapField &mf, int cloak)
{
	_TileSeen<true> seen(player, cloak);
	mf.UnitCache.for_each(seen);
}

/**
**  This function unmarks units on x, y as seen. It uses a reference count.
**
**  @param player    The player to mark for.
**  @param mf        field to check if building is on, and mark as seen
**  @param cloak     If this is for cloaked units.
*/
static void UnitsOnTileUnmarkSeen(const CPlayer &player, CMapField &mf, int cloak)
{
	_TileSeen<false> seen(player, cloak);
	mf.UnitCache.for_each(seen);
}


/**
**  Mark a tile's sight. (Explore and make visible.)
**
**  @param player  Player to mark sight.
**  @param index   tile to mark.
*/
void MapMarkTileSight(const CPlayer &player, const unsigned int index)
{
	CMapField &mf = *Map.Field(index);
	unsigned short *v = &(mf.playerInfo.Visible[player.Index]);
	if (*v == 0 || *v == 1) { // Unexplored or unseen
		// When there is no fog only unexplored tiles are marked.
		if (!Map.NoFogOfWar || *v == 0) {
			UnitsOnTileMarkSeen(player, mf, 0);
		}
		*v = 2;
		if (mf.playerInfo.IsTeamVisible(*ThisPlayer)) {
			Map.MarkSeenTile(mf);
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
**  @param indexx  tile to mark.
*/
void MapUnmarkTileSight(const CPlayer &player, const unsigned int index)
{
	CMapField &mf = *Map.Field(index);
	unsigned short *v = &mf.playerInfo.Visible[player.Index];
	switch (*v) {
		case 0:  // Unexplored
		case 1:
			// This happens when we unmark everything in CommandSharedVision
			break;
		case 2:
			// When there is NoFogOfWar units never get unmarked.
			if (!Map.NoFogOfWar) {
				UnitsOnTileUnmarkSeen(player, mf, 0);
			}
			// Check visible Tile, then deduct...
			if (mf.playerInfo.IsTeamVisible(*ThisPlayer)) {
				Map.MarkSeenTile(mf);
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
**  @param index   Tile to mark.
*/
void MapMarkTileDetectCloak(const CPlayer &player, const unsigned int index)
{
	CMapField &mf = *Map.Field(index);
	unsigned char *v = &mf.playerInfo.VisCloak[player.Index];
	if (*v == 0) {
		UnitsOnTileMarkSeen(player, mf, 1);
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
**  @param index   tile to mark.
*/
void MapUnmarkTileDetectCloak(const CPlayer &player, const unsigned int index)
{
	CMapField &mf = *Map.Field(index);
	unsigned char *v = &mf.playerInfo.VisCloak[player.Index];
	Assert(*v != 0);
	if (*v == 1) {
		UnitsOnTileUnmarkSeen(player, mf, 1);
	}
	--*v;
}

void MapUnmarkTileDetectCloak(const CPlayer &player, const Vec2i &pos)
{
	MapUnmarkTileDetectCloak(player, Map.getIndex(pos));
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
void MapSight(const CPlayer &player, const CUnit &unit, const Vec2i &pos, int w, int h, int range, MapMarkerFunc *marker)
{
	FieldOfView.Refresh(player, unit, pos, w, h, range, marker);
}

/**
**  Update fog of war.
*/
void UpdateFogOfWarChange()
{
	DebugPrint("::UpdateFogOfWarChange\n");
	//  Mark all explored fields as visible again.
	if (Map.NoFogOfWar) {
		const unsigned int w = Map.Info.MapHeight * Map.Info.MapWidth;
		for (unsigned int index = 0; index != w; ++index) {
			CMapField &mf = *Map.Field(index);
			if (mf.playerInfo.IsExplored(*ThisPlayer)) {
				Map.MarkSeenTile(mf);
			}
		}
	}
	//  Global seen recount.
	for (CUnitManager::Iterator it = UnitManager.begin(); it != UnitManager.end(); ++it) {
		CUnit &unit = **it;
		UnitCountSeen(unit);
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

/*----------------------------------------------------------------------------
--  Old version correct working but not 100% original
----------------------------------------------------------------------------*/

static void GetFogOfWarTile(int sx, int sy, int *fogTile, int *blackFogTile)
{
#define IsMapFieldExploredTable(index) (VisibleTable[(index)])
#define IsMapFieldVisibleTable(index) (VisibleTable[(index)] > 1)

	int w = Map.Info.MapWidth;
	int fogTileIndex = 0;
	int blackFogTileIndex = 0;
	int x = sx - sy;

	if (ReplayRevealMap) {
		*fogTile = 0;
		*blackFogTile = 0;
		return;
	}

	//
	//  Which Tile to draw for fog
	//
	// Investigate tiles around current tile
	// 1 2 3
	// 4 * 5
	// 6 7 8

	//    2  3 1
	//   10 ** 5
	//    8 12 4

	if (sy) {
		unsigned int index = sy - Map.Info.MapWidth;//(y-1) * Map.Info.MapWidth;
		if (sx != sy) {
			//if (!IsMapFieldExploredTable(x - 1, y - 1)) {
			if (!IsMapFieldExploredTable(x - 1 + index)) {
				blackFogTileIndex |= 2;
				fogTileIndex |= 2;
				//} else if (!IsMapFieldVisibleTable(x - 1, y - 1)) {
			} else if (!IsMapFieldVisibleTable(x - 1 + index)) {
				fogTileIndex |= 2;
			}
		}
		//if (!IsMapFieldExploredTable(x, y - 1)) {
		if (!IsMapFieldExploredTable(x + index)) {
			blackFogTileIndex |= 3;
			fogTileIndex |= 3;
			//} else if (!IsMapFieldVisibleTable(x, y - 1)) {
		} else if (!IsMapFieldVisibleTable(x + index)) {
			fogTileIndex |= 3;
		}
		if (sx != sy + w - 1) {
			//if (!IsMapFieldExploredTable(x + 1, y - 1)) {
			if (!IsMapFieldExploredTable(x + 1 + index)) {
				blackFogTileIndex |= 1;
				fogTileIndex |= 1;
				//} else if (!IsMapFieldVisibleTable(x + 1, y - 1)) {
			} else if (!IsMapFieldVisibleTable(x + 1 + index)) {
				fogTileIndex |= 1;
			}
		}
	}

	if (sx != sy) {
		unsigned int index = sy;//(y) * Map.Info.MapWidth;
		//if (!IsMapFieldExploredTable(x - 1, y)) {
		if (!IsMapFieldExploredTable(x - 1 + index)) {
			blackFogTileIndex |= 10;
			fogTileIndex |= 10;
			//} else if (!IsMapFieldVisibleTable(x - 1, y)) {
		} else if (!IsMapFieldVisibleTable(x - 1 + index)) {
			fogTileIndex |= 10;
		}
	}
	if (sx != sy + w - 1) {
		unsigned int index = sy;//(y) * Map.Info.MapWidth;
		//if (!IsMapFieldExploredTable(x + 1, y)) {
		if (!IsMapFieldExploredTable(x + 1 + index)) {
			blackFogTileIndex |= 5;
			fogTileIndex |= 5;
			//} else if (!IsMapFieldVisibleTable(x + 1, y)) {
		} else if (!IsMapFieldVisibleTable(x + 1 + index)) {
			fogTileIndex |= 5;
		}
	}

	if (sy + w < Map.Info.MapHeight * w) {
		unsigned int index = sy + Map.Info.MapWidth;//(y+1) * Map.Info.MapWidth;
		if (sx != sy) {
			//if (!IsMapFieldExploredTable(x - 1, y + 1)) {
			if (!IsMapFieldExploredTable(x - 1 + index)) {
				blackFogTileIndex |= 8;
				fogTileIndex |= 8;
				//} else if (!IsMapFieldVisibleTable(x - 1, y + 1)) {
			} else if (!IsMapFieldVisibleTable(x - 1 + index)) {
				fogTileIndex |= 8;
			}
		}
		//if (!IsMapFieldExploredTable(x, y + 1)) {
		if (!IsMapFieldExploredTable(x + index)) {
			blackFogTileIndex |= 12;
			fogTileIndex |= 12;
			//} else if (!IsMapFieldVisibleTable(x, y + 1)) {
		} else if (!IsMapFieldVisibleTable(x + index)) {
			fogTileIndex |= 12;
		}
		if (sx != sy + w - 1) {
			//if (!IsMapFieldExploredTable(x + 1, y + 1)) {
			if (!IsMapFieldExploredTable(x + 1 + index)) {
				blackFogTileIndex |= 4;
				fogTileIndex |= 4;
				//} else if (!IsMapFieldVisibleTable(x + 1, y + 1)) {
			} else if (!IsMapFieldVisibleTable(x + 1 + index)) {
				fogTileIndex |= 4;
			}
		}
	}

	*fogTile = FogTable[fogTileIndex];
	*blackFogTile = FogTable[blackFogTileIndex];
}

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
	int fogTile = 0;
	int blackFogTile = 0;

	GetFogOfWarTile(sx, sy, &fogTile, &blackFogTile);

	if (IsMapFieldVisibleTable(sx) || ReplayRevealMap) {
		if (fogTile && fogTile != blackFogTile) {
			AlphaFogG->DrawFrameClip(fogTile, dx, dy);
		}
	} else {
		VideoDrawOnlyFog(dx, dy);
	}
	if (blackFogTile) {
		Map.FogGraphic->DrawFrameClip(blackFogTile, dx, dy);
	}

#undef IsMapFieldExploredTable
#undef IsMapFieldVisibleTable
}

/**
**  Draw the map fog of war.
*/
void CViewport::DrawMapFogOfWar()
{
	// flags must redraw or not
	if (ReplayRevealMap) {
		return;
	}

	switch (FogOfWar.GetType()) {
		case FogOfWarTypes::cLegacy:
		
			this->DrawLegacyFogOfWar();
			break;
		
		case FogOfWarTypes::cEnhanced: 

			if (!FogSurface || ( ((this->BottomRightPos.x - this->TopLeftPos.x) 
								  / PixelTileSize.x + 2) 
								  * PixelTileSize.x != FogSurface->w 
								|| ((this->BottomRightPos.y - this->TopLeftPos.y) 
									/ PixelTileSize.y + 2) 
									* PixelTileSize.y != FogSurface->h) ) {
          		this->AdjustFogSurface();
    		}
			this->DrawEnhancedFogOfWar(); 
			break;
		default: 
		break;
	}
}


/**
**  Draw the map fog of war (legacy type).
*/
void CViewport::DrawLegacyFogOfWar() const
{
	
	int sx = std::max<int>(MapPos.x - 1, 0);
	int ex = std::min<int>(MapPos.x + MapWidth + 1, Map.Info.MapWidth);
	int my = std::max<int>(MapPos.y - 1, 0);
	int ey = std::min<int>(MapPos.y + MapHeight + 1, Map.Info.MapHeight);

	// Update for visibility all tile in viewport
	// and 1 tile around viewport (for fog-of-war connection display)

	unsigned int my_index = my * Map.Info.MapWidth;
	for (; my < ey; ++my) {
		for (int mx = sx; mx < ex; ++mx) {
			VisibleTable[my_index + mx] = Map.Field(mx + my_index)->playerInfo.TeamVisibilityState(*ThisPlayer);
		}
		my_index += Map.Info.MapWidth;
	}
	ex = this->BottomRightPos.x;
	int sy = MapPos.y * Map.Info.MapWidth;
	int dy = this->TopLeftPos.y - Offset.y;
	ey = this->BottomRightPos.y;

	while (dy <= ey) {
		sx = MapPos.x + sy;
		int dx = this->TopLeftPos.x - Offset.x;
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
**  Draw the map fog of war (enhanced type).
*/
void CViewport::DrawEnhancedFogOfWar(const bool isSoftwareRender /* = true */ )
{

 	FogOfWar.GetFogForViewport(*this, FogSurface);

	if (isSoftwareRender) {
		SDL_Rect screenRect;
		screenRect.x = this->TopLeftPos.x;
		screenRect.y = this->TopLeftPos.y;
		screenRect.w = this->BottomRightPos.x - this->TopLeftPos.x + 1;
		screenRect.h = this->BottomRightPos.y - this->TopLeftPos.y + 1;

		SDL_Rect fogRect;
		fogRect.x = this->Offset.x;
		fogRect.y = this->Offset.y;
		fogRect.w = screenRect.w;
		fogRect.h = screenRect.h;

		/// As a single-threaded alternative:
		// SDL_BlitSurface(FogSurface, &fogRect, TheScreen, &screenRect);

		/// Alpha blending of the fog texture into the screen
		uint32_t *const fog    = static_cast<uint32_t *>(FogSurface->pixels);
		uint32_t *const screen = static_cast<uint32_t *>(TheScreen->pixels);
		const CColor fogColor = FogOfWar.GetFogColor();
		const uint8_t fogR = fogColor.R;
		const uint8_t fogG = fogColor.G;
		const uint8_t fogB = fogColor.B;
		
		#pragma omp parallel
		{    
			const uint16_t thisThread   = omp_get_thread_num();
			const uint16_t numOfThreads = omp_get_num_threads();
			
			const uint16_t lBound = (thisThread    ) * screenRect.h / numOfThreads; 
			const uint16_t uBound = (thisThread + 1) * screenRect.h / numOfThreads; 

			size_t fogIndex = (fogRect.y    + lBound) * FogSurface->w + fogRect.x;
			size_t scrIndex = (screenRect.y + lBound) * TheScreen->w + screenRect.x;
			
			for (uint16_t y = lBound; y < uBound; y++) {
				for (uint16_t x = 0; x < screenRect.w; x++) {

					const uint32_t fogPixel = fog[fogIndex + x];
					const uint8_t  alpha    = 0xFF & (fogPixel >> ASHIFT);

					uint32_t &scrPixel = screen[scrIndex + x];

					if (alpha == 0xFE) { scrPixel = fogPixel; }
					else  {
						const uint8_t scrR = 0xFF & (scrPixel >> RSHIFT);
						const uint8_t scrG = 0xFF & (scrPixel >> GSHIFT);
						const uint8_t scrB = 0xFF & (scrPixel >> BSHIFT);

						const uint32_t resR = ((fogR * alpha) + (scrR * (0xFF - alpha))) >> 8;
						const uint32_t resG = ((fogG * alpha) + (scrG * (0xFF - alpha))) >> 8;
						const uint32_t resB = ((fogB * alpha) + (scrB * (0xFF - alpha))) >> 8;

						scrPixel = (resR << RSHIFT) | (resG << GSHIFT) | (resB << BSHIFT);
					}
				}
				fogIndex += FogSurface->w;
				scrIndex += TheScreen->w;
			}
		} /// pragma omp parallel

	}

}


/**
**  Adjust fog of war surface to viewport
**
*/
void CViewport::AdjustFogSurface()
{
 	SDL_FreeSurface(FogSurface); /// It is safe to pass NULL to this function.
	FogSurface = nullptr;
	
    const uint16_t surfaceWidth  = ((this->BottomRightPos.x - this->TopLeftPos.x) 
									/ PixelTileSize.x + 2) 
									* PixelTileSize.x;  /// +2 because of Offset.x
    const uint16_t surfaceHeight = ((this->BottomRightPos.y - this->TopLeftPos.y) 
									/ PixelTileSize.y + 2) 
									* PixelTileSize.y; /// +2 because of Offset.y
    
    FogSurface = SDL_CreateRGBSurface(SDL_SWSURFACE, surfaceWidth, 
                                                     surfaceHeight,
                                                     32, RMASK, GMASK, BMASK, AMASK);
    SDL_SetSurfaceBlendMode(FogSurface, SDL_BLENDMODE_BLEND);
	
	const uint32_t fogColorSolid = FogOfWar.GetFogColorSDL() | ((uint32_t)0xFF << ASHIFT);

	SDL_FillRect(FogSurface, NULL, fogColorSolid);
}


void CViewport::Clean()
{
	SDL_FreeSurface(this->FogSurface);
	this->FogSurface = nullptr;
}

/**
**  Initialize the fog of war.
**  Build tables, setup functions.
*/
void CMap::InitFogOfWar()
{
	//calculate this once from the settings and store it
	FogOfWarColorSDL = Video.MapRGB(TheScreen->format, FogOfWarColor);

	Uint8 r, g, b;
	SDL_Surface *s;

	FogGraphic->Load();

	//
	// Generate Only Fog surface.
	//
	OnlyFogSurface = SDL_CreateRGBSurface(SDL_SWSURFACE, PixelTileSize.x, PixelTileSize.y,
										  32, RMASK, GMASK, BMASK, AMASK);

	SDL_GetRGB(FogOfWarColorSDL, TheScreen->format, &r, &g, &b);
	Uint32 color = Video.MapRGBA(OnlyFogSurface->format, r, g, b, FogOfWarOpacity);

	SDL_FillRect(OnlyFogSurface, NULL, color);

	//
	// Generate Alpha Fog surface.
	//
	if (FogGraphic->Surface->format->BytesPerPixel == 1) {
		s = SDL_ConvertSurfaceFormat(FogGraphic->Surface, SDL_PIXELFORMAT_RGBA8888, 0);
		SDL_SetSurfaceAlphaMod(s, FogOfWarOpacity);
	} else {
		// Copy the top row to a new surface
		SDL_PixelFormat *f = FogGraphic->Surface->format;
		s = SDL_CreateRGBSurface(SDL_SWSURFACE, FogGraphic->Surface->w, PixelTileSize.y,
								 f->BitsPerPixel, f->Rmask, f->Gmask, f->Bmask, f->Amask);
		SDL_LockSurface(s);
		SDL_LockSurface(FogGraphic->Surface);
		for (int i = 0; i < s->h; ++i) {
			memcpy(reinterpret_cast<Uint8 *>(s->pixels) + i * s->pitch,
				   reinterpret_cast<Uint8 *>(FogGraphic->Surface->pixels) + i * FogGraphic->Surface->pitch,
				   FogGraphic->Surface->w * f->BytesPerPixel);
		}
		SDL_UnlockSurface(s);
		SDL_UnlockSurface(FogGraphic->Surface);

		// Convert any non-transparent pixels to use FogOfWarOpacity as alpha
		SDL_LockSurface(s);
		for (int j = 0; j < s->h; ++j) {
			for (int i = 0; i < s->w; ++i) {
				Uint32 c = *reinterpret_cast<Uint32 *>(&reinterpret_cast<Uint8 *>(s->pixels)[i * 4 + j * s->pitch]);
				Uint8 a;

				Video.GetRGBA(c, s->format, &r, &g, &b, &a);
				if (a) {
					c = Video.MapRGBA(s->format, r, g, b, FogOfWarOpacity);
					*reinterpret_cast<Uint32 *>(&reinterpret_cast<Uint8 *>(s->pixels)[i * 4 + j * s->pitch]) = c;
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

	VisibleTable.clear();
	VisibleTable.resize(Info.MapWidth * Info.MapHeight);
}

/**
**  Cleanup the fog of war.
**  Note: If current type of FOW is cEnhanced it has to be called too in case of FOW type was changed during game
**  It's safe to call this for both types of FOW. 
*/
void CMap::CleanFogOfWar()
{
	VisibleTable.clear();

	CGraphic::Free(Map.FogGraphic);
	FogGraphic = NULL;

	if (OnlyFogSurface) {
		VideoPaletteListRemove(OnlyFogSurface);
		SDL_FreeSurface(OnlyFogSurface);
		OnlyFogSurface = NULL;
	}
	CGraphic::Free(AlphaFogG);
	AlphaFogG = NULL;
}

//@}
